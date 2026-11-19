#pragma once
// Project AllMight / Book 01 / NES-12Q: CPU-only command lifecycle oracle.
//
// The executable witness composes the real NES-08G task dependency graph
// with NES-11B's guarded frame ring. No actual GPU submission, queue, fence,
// semaphore or callback is created here. The fake completion provider is a
// deterministic *test fixture*, not evidence of native device completion.
//
// Important safety properties:
//  1. Recording/sealing is separate from queue acceptance and execution.
//  2. Rejection known BEFORE publication leaves a writer and command stream
//     available for retry or safe RAII rollback.
//  3. Indeterminate submission is QUARANTINED, not casually cancelled.
//  4. Queue owns the command-state control block for the whole in-flight
//     interval even if the client destroys/moves its CommandStream wrapper.
//  5. Completion notices are explicit owned EVENT RECORDS; polling does not
//     invoke arbitrary callbacks or simulate background GPU work.
//  6. Retirement uses BOTH the underlying frame ring's queue stamps and
//     outstanding CPU reader-pin checks.
//  7. Device loss stops acceptance; in-flight records remain quarantined.
//     A real backend needs a verified idle/reset/recovery protocol before
//     actually destroying device-referenced physical allocations.
// This is a single-threaded C++23 reference model; external synchronization
// is mandatory if used from multiple host threads.
#include "scoped_frame_upload_ring.hpp"
#include "work_dependency_graph.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::nes::commands {
namespace fr = allmight::nes::frames;
namespace dg = allmight::nes::work;

enum class State : std::uint8_t {
    initial, recording, executable, pending, unknown, completed, retired
};
enum class CommandError : std::uint8_t {
    moved_from,
    wrong_state,
    empty_recording,
    host_work_not_gpu_command,
    graph_rejected,
    driver_rejected_before_submit,
    ambiguous_submission,
    capacity_exhausted,
    no_payload,
    invalid_queue_stamp,
    frame_publication_failed,
    device_lost,
    missing_submission,
    not_completed,
    submission_indeterminate,
    completion_regressed,
    pending_reader,
    invalid_frame,
    submission_id_exhausted
};
enum class SimulatedSubmit : std::uint8_t {
    accepted, rejected_before_submit, indeterminate
};
struct Receipt {
    std::uint64_t ticket{};
    fr::FrameHandle frame{};
    fr::CompletionStamp required{};
    State state{};
};
struct CompletionEvent {
    std::uint64_t ticket{};
    fr::FrameHandle frame{};
    fr::CompletionStamp observed{};
    // This is a notification of test-driven completion observation,
    // NOT a native GPU event nor a timestamp from hardware.
    friend bool operator==(CompletionEvent,CompletionEvent)=default;
};

class CommandStream {
    struct Control {
        State state{State::initial};
        dg::DependencyGraph graph;
        std::optional<dg::Plan> compiled;
        std::optional<dg::Failure> last_graph_failure;
        std::array<bool,3> gpu_domains{};
        std::size_t encoded_count{};
    };
public:
    CommandStream() : control_(std::make_shared<Control>()) {}
    CommandStream(const CommandStream&) = delete;
    CommandStream& operator=(const CommandStream&) = delete;
    CommandStream(CommandStream&&) noexcept = default;
    CommandStream& operator=(CommandStream&&) noexcept = default;

    [[nodiscard]] State state() const noexcept {
        return control_?control_->state:State::retired;
    }
    [[nodiscard]] std::size_t command_count() const noexcept {
        return control_?control_->encoded_count:0U;
    }
    [[nodiscard]] std::optional<dg::Failure>
    last_graph_failure() const noexcept {
        return control_?control_->last_graph_failure:std::nullopt;
    }

    [[nodiscard]] std::expected<void,CommandError> begin() noexcept {
        if(!control_) return std::unexpected(CommandError::moved_from);
        if(control_->state!=State::initial)
            return std::unexpected(CommandError::wrong_state);
        control_->state=State::recording;
        return {};
    }

    // Every recorded task is a GPU *intent*, never a CPU host callback.
    // GPU ordering becomes a planned DAG; native command encoding happens
    // only after a real backend translates that plan and its memory hazards.
    [[nodiscard]] std::expected<void,CommandError> record(dg::Task task) {
        if(!control_) return std::unexpected(CommandError::moved_from);
        if(control_->state!=State::recording)
            return std::unexpected(CommandError::wrong_state);
        if(task.domain==dg::Domain::host)
            return std::unexpected(CommandError::host_work_not_gpu_command);
        // Compute the domains only after graph accepts the entire record.
        const auto domain=task.domain;
        const auto added=control_->graph.add(std::move(task));
        if(!added) {
            control_->last_graph_failure=added.error();
            return std::unexpected(CommandError::graph_rejected);
        }
        control_->last_graph_failure.reset();
        if(domain==dg::Domain::gpu_transfer) control_->gpu_domains[0]=true;
        if(domain==dg::Domain::gpu_compute) control_->gpu_domains[1]=true;
        if(domain==dg::Domain::gpu_graphics) control_->gpu_domains[2]=true;
        ++control_->encoded_count;
        return {};
    }

    [[nodiscard]] std::expected<void,CommandError> seal() {
        if(!control_) return std::unexpected(CommandError::moved_from);
        if(control_->state!=State::recording)
            return std::unexpected(CommandError::wrong_state);
        if(control_->encoded_count==0)
            return std::unexpected(CommandError::empty_recording);
        auto plan=control_->graph.plan();
        if(!plan) {
            control_->last_graph_failure=plan.error();
            return std::unexpected(CommandError::graph_rejected);
        }
        control_->last_graph_failure.reset();
        control_->compiled.emplace(std::move(*plan));
        control_->state=State::executable;
        return {};
    }

    // Cancel only work which has never been handed to a queue.
    [[nodiscard]] std::expected<void,CommandError> abandon() {
        if(!control_) return std::unexpected(CommandError::moved_from);
        if(control_->state==State::pending ||
           control_->state==State::unknown ||
           control_->state==State::completed)
            return std::unexpected(CommandError::wrong_state);
        if(control_->state!=State::recording &&
           control_->state!=State::executable)
            return std::unexpected(CommandError::wrong_state);
        control_->graph=dg::DependencyGraph{};
        control_->compiled.reset();
        control_->last_graph_failure.reset();
        control_->encoded_count=0;
        control_->gpu_domains={};
        control_->state=State::initial;
        return {};
    }

    // A retired stream may be reset, but never a pending stream. The queue
    // still owns its old Control until retirement or model destruction.
    [[nodiscard]] std::expected<void,CommandError> reset() {
        if(!control_) return std::unexpected(CommandError::moved_from);
        if(control_->state!=State::retired)
            return std::unexpected(CommandError::wrong_state);
        control_->graph=dg::DependencyGraph{};
        control_->compiled.reset();
        control_->last_graph_failure.reset();
        control_->encoded_count=0;
        control_->gpu_domains={};
        control_->state=State::initial;
        return {};
    }

    [[nodiscard]] const dg::Plan* plan() const noexcept {
        return control_ && control_->compiled ? &*control_->compiled:nullptr;
    }
private:
    template<std::size_t> friend class CommandSubmissionQueue;
    std::shared_ptr<Control> control_;
};

template<std::size_t Capacity>
class CommandSubmissionQueue {
public:
    using Ring=fr::FrameUploadRing<Capacity>;
    using Writer=typename Ring::WriteLease;

    explicit CommandSubmissionQueue(Ring& ring,std::size_t max_submissions=128)
        : ring_(ring),max_submissions_(max_submissions) {
        if(max_submissions==0)
            throw std::invalid_argument("a queue needs at least one record");
    }
    CommandSubmissionQueue(const CommandSubmissionQueue&)=delete;
    CommandSubmissionQueue& operator=(const CommandSubmissionQueue&)=delete;

    // precondition: Ring must outlive the queue. The queue does not keep a
    // pointer into mutable user-owned CommandStream: it owns shared Control.
    [[nodiscard]] std::expected<Receipt,CommandError>
    submit(CommandStream& stream,Writer& writer,
           fr::CompletionStamp required,
           SimulatedSubmit decision) {
        if(lost_) return std::unexpected(CommandError::device_lost);
        if(!stream.control_) return std::unexpected(CommandError::moved_from);
        auto& control=*stream.control_;
        if(control.state!=State::executable || !control.compiled)
            return std::unexpected(CommandError::wrong_state);
        if(writer.staged_size()==0)
            return std::unexpected(CommandError::no_payload);
        if(entries_.size()==max_submissions_)
            return std::unexpected(CommandError::capacity_exhausted);
        if(next_ticket_==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(CommandError::submission_id_exhausted);
        const auto observed=ring_.completed();
        const std::array<std::uint64_t,3> tickets{
            required.transfer,required.compute,required.graphics
        };
        const std::array<std::uint64_t,3> done{
            observed.transfer,observed.compute,observed.graphics
        };
        for(std::size_t i=0;i<3;++i) {
            if(control.gpu_domains[i] && tickets[i]==0)
                return std::unexpected(CommandError::invalid_queue_stamp);
            if(tickets[i]!=0 && tickets[i]<=done[i])
                return std::unexpected(CommandError::invalid_queue_stamp);
        }
        // If the backend definitely refused before accepting commands,
        // neither the frame slot nor command stream may be published.
        if(decision==SimulatedSubmit::rejected_before_submit)
            return std::unexpected(CommandError::driver_rejected_before_submit);

        // Reserve the host journal BEFORE publishing the frame in the ring.
        // Allocation failure is allowed to propagate; the writer remains
        // active and the command state remains executable in that case.
        entries_.reserve(entries_.size()+1);
        events_.reserve(events_.size()+1);
        Entry prepared;
        prepared.ticket=next_ticket_;
        prepared.control=stream.control_;
        prepared.required=required;
        prepared.state=(decision==SimulatedSubmit::indeterminate)
            ?State::unknown:State::pending;
        entries_.push_back(std::move(prepared));
        const auto frame=writer.submit(required);
        if(!frame) {
            entries_.pop_back();
            return std::unexpected(CommandError::frame_publication_failed);
        }
        auto& entry=entries_.back();
        entry.frame=*frame;
        entry.control->state=entry.state;
        ++next_ticket_;
        return Receipt{entry.ticket,entry.frame,entry.required,entry.state};
    }

    // Unknown outcomes remain quarantined unless the backend independently
    // proves acceptance. "probably failed" is NOT a safe release condition.
    [[nodiscard]] std::expected<void,CommandError>
    confirm_accepted(std::uint64_t ticket) noexcept {
        auto entry=find(ticket);
        if(!entry) return std::unexpected(CommandError::missing_submission);
        if((*entry)->state!=State::unknown)
            return std::unexpected(CommandError::wrong_state);
        if(lost_) return std::unexpected(CommandError::device_lost);
        (*entry)->state=State::pending;
        (*entry)->control->state=State::pending;
        return {};
    }

    [[nodiscard]] std::expected<void,CommandError>
    observe_completion(fr::CompletionStamp stamp) noexcept {
        if(lost_) return std::unexpected(CommandError::device_lost);
        if(!ring_.observe_completion(stamp))
            return std::unexpected(CommandError::completion_regressed);
        // Does not fabricate events: client separately calls poll() to
        // materialize owned notifications for completed accepted entries.
        return {};
    }

    [[nodiscard]] std::expected<std::size_t,CommandError> poll() {
        if(lost_) return std::unexpected(CommandError::device_lost);
        std::size_t published{};
        for(auto& entry:entries_) {
            if(entry.state!=State::pending) continue;
            if(!fr::reached(ring_.completed(),entry.required)) continue;
            // Reserve first for exception-safe event publication.
            events_.push_back({
                entry.ticket,entry.frame,ring_.completed()
            });
            entry.state=State::completed;
            entry.control->state=State::completed;
            ++published;
        }
        return published;
    }

    [[nodiscard]] std::vector<CompletionEvent> drain_events() {
        std::vector<CompletionEvent> result;
        result.swap(events_);
        return result;
    }

    [[nodiscard]] std::expected<void,CommandError>
    retire(std::uint64_t ticket) noexcept {
        auto entry=find(ticket);
        if(!entry) return std::unexpected(CommandError::missing_submission);
        if((*entry)->state==State::unknown)
            return std::unexpected(CommandError::submission_indeterminate);
        if((*entry)->state!=State::completed)
            return std::unexpected(CommandError::not_completed);
        const auto result=ring_.retire((*entry)->frame);
        if(!result) {
            if(result.error()==fr::FrameError::readers_pinned)
                return std::unexpected(CommandError::pending_reader);
            if(result.error()==fr::FrameError::completion_pending)
                return std::unexpected(CommandError::not_completed);
            return std::unexpected(CommandError::invalid_frame);
        }
        (*entry)->state=State::retired;
        (*entry)->control->state=State::retired;
        return {};
    }

    // Device loss prohibits new submission, polling and artificial
    // retirement. All pending/unknown resources remain quarantined in this
    // simulation. Real backends must implement explicit recovery using a
    // confirmed device-idle/reset completion proof before destroying them.
    void declare_device_lost() noexcept {lost_=true;}
    [[nodiscard]] bool lost() const noexcept {return lost_;}
    [[nodiscard]] std::size_t outstanding() const noexcept {
        std::size_t count{};
        for(const auto& entry:entries_)
            if(entry.state!=State::retired) ++count;
        return count;
    }
    // A bounded journal needs an explicit compaction policy. Only RETIRED
    // records can be discarded; their owned completion events are separately
    // delivered via drain_events(), and IDs are never recycled.
    [[nodiscard]] std::size_t prune_retired() noexcept {
        const auto before=entries_.size();
        std::erase_if(entries_,[](const Entry& entry) noexcept {
            return entry.state==State::retired;
        });
        return before-entries_.size();
    }
    [[nodiscard]] std::optional<State>
    status(std::uint64_t ticket) const noexcept {
        for(const auto& entry:entries_)
            if(entry.ticket==ticket) return entry.state;
        return std::nullopt;
    }

private:
    struct Entry {
        std::uint64_t ticket{};
        fr::FrameHandle frame{};
        fr::CompletionStamp required{};
        std::shared_ptr<CommandStream::Control> control;
        State state{State::initial};
    };
    [[nodiscard]] std::optional<Entry*>
    find(std::uint64_t ticket) noexcept {
        for(auto& entry:entries_)
            if(entry.ticket==ticket) return &entry;
        return std::nullopt;
    }

    Ring& ring_; // explicit borrowed lifetime: caller owns the ring
    std::size_t max_submissions_;
    std::uint64_t next_ticket_{1};
    std::vector<Entry> entries_;
    std::vector<CompletionEvent> events_;
    bool lost_{};
};

static_assert(!std::is_copy_constructible_v<CommandStream>);
static_assert(std::is_nothrow_move_constructible_v<CommandStream>);

} // namespace allmight::nes::commands
