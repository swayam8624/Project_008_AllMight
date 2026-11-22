#pragma once
// Project AllMight / GPU-16P: asynchronous GPU completion ownership,
// inspectable evidence and fail-closed performance summaries.
//
// Primary clients: Maveb Metal Renderer/FrameContext, Kairo MetalBackend.mm,
// Gaussian passes, Kyoto pair shaders and XR frame/gaze measurements.
//
// PORTABLE C++23 REFERENCE. An event delivered by this model is a synthetic
// observation, not proof of actual MTLCommandBuffer status or a real GPU
// counter. Optional macOS native callback adapter independently tests a
// real Metal command buffer, shader, output and short-lived UI observer.
//
// The callback captures only shared State; never captures Renderer* or this.
// The State, slot generation, permit release and terminal event are protected
// by ONE mutex. Duplicate terminal callbacks cannot return two permits.
// Do NOT call UI/Swift/SwiftUI or long-running work while the lock is held.
#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace allmight::gpu_course::profile {
enum class Project : std::uint8_t {
    maveb, kairo, kyoto, xr, nes
};
enum class Terminal : std::uint8_t {
    completed, failed
};
enum class Origin : std::uint8_t {
    synthetic_fixture, native_metal_observation
};
enum class Error : std::uint8_t {
    invalid_capacity, exhausted, invalid_owner, invalid_slot,
    stale_generation, wrong_phase, duplicate, wrong_status,
    incomplete_provenance, mixed_environment, wrong_origin,
    insufficient_samples, incorrect_output, invalid_timing,
    unsupported_counter, invalid_counter
};
struct FrameId {
    std::uint64_t owner{};
    std::uint32_t slot{};
    std::uint64_t generation{};
    friend constexpr bool operator==(FrameId,FrameId)=default;
};
struct NativeObservation {
    Terminal terminal{Terminal::failed};
    // Terminal GPU timing must be read from completed native command buffers.
    // A synthetic caller may supply values to exercise policy only.
    std::optional<double> gpu_start_seconds;
    std::optional<double> gpu_end_seconds;
    bool output_checked{};
    bool output_correct{};
    Origin origin{Origin::synthetic_fixture};
};
struct CompletionEvent {
    FrameId frame{};
    Project project{Project::maveb};
    Terminal terminal{Terminal::failed};
    Origin origin{Origin::synthetic_fixture};
    bool correctness_checked{};
    bool correctness_pass{};
    std::optional<double> gpu_milliseconds;
};
// GPUStartTime/GPUEndTime are host-time seconds, not elapsed device cycles.
// They can be zero/unavailable on a supported native device. Never substitute
// 0 ms or a guessed value for an absent/invalid interval.
[[nodiscard]] inline std::optional<double> gpu_interval_ms(
    const NativeObservation& observation) noexcept {
    if(observation.terminal!=Terminal::completed ||
       !observation.gpu_start_seconds ||
       !observation.gpu_end_seconds)
        return std::nullopt;
    const auto a=*observation.gpu_start_seconds;
    const auto b=*observation.gpu_end_seconds;
    if(!std::isfinite(a)||!std::isfinite(b)||a<=0||b<=a)
        return std::nullopt;
    const auto ms=(b-a)*1000.0;
    if(!std::isfinite(ms)||ms<=0) return std::nullopt;
    return ms;
}

class CompletionMailbox {
    enum class Phase : std::uint8_t {
        free, recording, submitted, quarantined, exhausted
    };
    struct Slot {
        Phase phase{Phase::free};
        std::uint64_t generation{1};
        Project project{Project::maveb};
    };
    struct State {
        mutable std::mutex mutex;
        std::condition_variable changed;
        std::vector<Slot> slots;
        std::vector<CompletionEvent> events;
        std::uint64_t owner;
        explicit State(std::uint64_t identity,std::size_t capacity)
          : slots(capacity),owner(identity) {}
    };
public:
    struct TerminalCallback {
        std::shared_ptr<State> state;
        FrameId frame{};

        [[nodiscard]] std::expected<void,Error>
        operator()(const NativeObservation& observation) const {
            if(!state) return std::unexpected(Error::invalid_owner);
            std::unique_lock lock(state->mutex);
            if(frame.owner!=state->owner)
                return std::unexpected(Error::invalid_owner);
            if(frame.slot>=state->slots.size())
                return std::unexpected(Error::invalid_slot);
            auto& slot=state->slots[frame.slot];
            if(slot.generation!=frame.generation ||
               slot.phase==Phase::free || slot.phase==Phase::exhausted)
                return std::unexpected(Error::stale_generation);
            if(slot.phase==Phase::quarantined ||
               slot.phase==Phase::recording)
                return std::unexpected(Error::wrong_phase);
            if(slot.phase!=Phase::submitted)
                return std::unexpected(Error::wrong_phase);

            CompletionEvent event;
            event.frame=frame;
            event.project=slot.project;
            event.terminal=observation.terminal;
            event.origin=observation.origin;
            event.correctness_checked=observation.output_checked;
            event.correctness_pass=
                observation.terminal==Terminal::completed &&
                observation.output_checked &&
                observation.output_correct;
            event.gpu_milliseconds=gpu_interval_ms(observation);
            // Prepare event under ownership before releasing the permit.
            // Allocation failure throws; slot remains submitted and can be
            // recovered explicitly after backend terminal observation.
            state->events.push_back(std::move(event));
            if(slot.generation==std::numeric_limits<std::uint64_t>::max())
                slot.phase=Phase::exhausted;
            else {
                ++slot.generation;
                slot.phase=Phase::free;
            }
            lock.unlock();
            state->changed.notify_all();
            return {};
        }
    };
    explicit CompletionMailbox(std::size_t frame_count=3,
                               std::uint64_t identity=0) {
        if(frame_count==0||frame_count>128)
            throw std::invalid_argument("invalid frame permit count");
        // Per-process owner identity. 0 means automatic; tests may pass a
        // deterministic distinct owner. Real services must not reuse owner
        // IDs across simultaneous mailbox incarnations.
        if(identity==0) {
            static std::atomic<std::uint64_t> next{1};
            identity=next.fetch_add(1,std::memory_order_relaxed);
            if(identity==0)
                throw std::overflow_error("frame mailbox owner exhausted");
        }
        state_=std::make_shared<State>(identity,frame_count);
    }
    CompletionMailbox(const CompletionMailbox&)=delete;
    CompletionMailbox& operator=(const CompletionMailbox&)=delete;
    CompletionMailbox(CompletionMailbox&&)=delete;
    CompletionMailbox& operator=(CompletionMailbox&&)=delete;

    [[nodiscard]] std::expected<FrameId,Error>
    reserve(Project project) {
        std::lock_guard lock(state_->mutex);
        for(std::size_t i=0;i<state_->slots.size();++i) {
            auto& slot=state_->slots[i];
            if(slot.phase==Phase::free) {
                slot.phase=Phase::recording;
                slot.project=project;
                return FrameId{
                  state_->owner,static_cast<std::uint32_t>(i),slot.generation};
            }
        }
        return std::unexpected(Error::exhausted);
    }
    [[nodiscard]] std::expected<void,Error>
    publish(FrameId frame) {
        return change_phase(frame,Phase::recording,Phase::submitted);
    }
    [[nodiscard]] std::expected<void,Error>
    quarantine(FrameId frame) {
        return change_phase(frame,Phase::recording,Phase::quarantined);
    }
    [[nodiscard]] std::expected<void,Error>
    confirm_published(FrameId frame) {
        return change_phase(frame,Phase::quarantined,Phase::submitted);
    }
    // Definite no-submit failure may cancel an owned recording slot.
    // It must not drop accepted/quarantined native work.
    [[nodiscard]] std::expected<void,Error>
    abandon_unsubmitted(FrameId frame) {
        std::unique_lock lock(state_->mutex);
        auto slot=checked_locked(frame);
        if(!slot) return std::unexpected(slot.error());
        if((*slot)->phase!=Phase::recording)
            return std::unexpected(Error::wrong_phase);
        if((*slot)->generation==std::numeric_limits<std::uint64_t>::max())
            (*slot)->phase=Phase::exhausted;
        else {
            ++(*slot)->generation;
            (*slot)->phase=Phase::free;
        }
        lock.unlock();
        state_->changed.notify_all();
        return {};
    }
    [[nodiscard]] TerminalCallback callback_for(FrameId frame) const {
        // Strong shared ownership deliberately keeps internal scheduler
        // state alive even if renderer/UI owner gets destroyed. Renderer
        // object itself is NEVER captured, so no dangling 'this' pointer.
        return TerminalCallback{state_,frame};
    }
    [[nodiscard]] std::size_t available() const noexcept {
        std::lock_guard lock(state_->mutex);
        std::size_t n=0;
        for(const auto& slot:state_->slots)
            if(slot.phase==Phase::free) ++n;
        return n;
    }
    [[nodiscard]] std::size_t outstanding() const noexcept {
        std::lock_guard lock(state_->mutex);
        std::size_t n=0;
        for(const auto& slot:state_->slots)
            if(slot.phase!=Phase::free && slot.phase!=Phase::exhausted) ++n;
        return n;
    }
    [[nodiscard]] std::vector<CompletionEvent> drain() {
        std::lock_guard lock(state_->mutex);
        std::vector<CompletionEvent> out;
        out.swap(state_->events);
        return out;
    }
    [[nodiscard]] std::uint64_t owner() const noexcept {
        return state_->owner;
    }
private:
    [[nodiscard]] std::expected<Slot*,Error>
    checked_locked(FrameId frame) {
        if(frame.owner!=state_->owner)
            return std::unexpected(Error::invalid_owner);
        if(frame.slot>=state_->slots.size())
            return std::unexpected(Error::invalid_slot);
        auto& slot=state_->slots[frame.slot];
        if(slot.generation!=frame.generation ||
           slot.phase==Phase::free || slot.phase==Phase::exhausted)
            return std::unexpected(Error::stale_generation);
        return &slot;
    }
    [[nodiscard]] std::expected<void,Error>
    change_phase(FrameId frame,Phase previous,Phase next) {
        std::lock_guard lock(state_->mutex);
        auto slot=checked_locked(frame);
        if(!slot) return std::unexpected(slot.error());
        if((*slot)->phase!=previous)
            return std::unexpected(Error::wrong_phase);
        (*slot)->phase=next;
        return {};
    }
    std::shared_ptr<State> state_;
};

// A native performance report requires a complete SAME-ENVIRONMENT key.
// A source commit alone is not sufficient: shaders, scene, resolution,
// device/OS/SDK, validation and run configuration all affect measurements.
struct Configuration {
    Project project{Project::maveb};
    std::string device;
    std::string os_sdk;
    std::string source_commit;
    std::string shader_digest;
    std::string workload_digest;
    std::string capture_tool;
    std::uint32_t width{};
    std::uint32_t height{};
    bool validation_enabled{};
    friend bool operator==(const Configuration&,
                           const Configuration&)=default;
};
struct CounterValue {
    std::string identifier;
    std::string unit;
    std::optional<double> measured;
    bool supported{};
};
struct FrameSample {
    Configuration config;
    Origin origin{Origin::synthetic_fixture};
    Terminal terminal{Terminal::failed};
    bool pixels_or_pairs_checked{};
    bool output_correct{};
    std::optional<double> cpu_encode_ms;
    std::optional<double> gpu_command_ms;
    std::optional<double> presentation_latency_ms;
    std::vector<CounterValue> counters;
};
struct SeriesSummary {
    Configuration config;
    std::size_t sample_count{};
    double cpu_median_ms{};
    double cpu_p95_ms{};
    double gpu_median_ms{};
    double gpu_p95_ms{};
    // Not a FPS estimate, no display/eye-tracking latency inferred.
};
[[nodiscard]] inline bool valid_ms(const std::optional<double>& ms)
    noexcept {
    return ms && std::isfinite(*ms) && *ms>0.0;
}
[[nodiscard]] inline std::expected<void,Error>
validate_configuration(const Configuration& c) {
    if(c.device.empty() || c.os_sdk.empty() || c.source_commit.empty() ||
       c.shader_digest.empty() || c.workload_digest.empty() ||
       c.capture_tool.empty() || c.width==0 || c.height==0)
        return std::unexpected(Error::incomplete_provenance);
    return {};
}
[[nodiscard]] inline double percentile(std::vector<double> v,
                                       double p) {
    std::sort(v.begin(),v.end());
    const auto rank=static_cast<std::size_t>(
        std::ceil(static_cast<double>(v.size())*p));
    return v[std::max<std::size_t>(1,rank)-1];
}
[[nodiscard]] inline std::expected<SeriesSummary,Error>
summarize_native_verified(
    const std::vector<FrameSample>& samples,
    std::size_t minimum_samples=20) {
    if(minimum_samples==0 || samples.size()<minimum_samples)
        return std::unexpected(Error::insufficient_samples);
    const auto& key=samples.front().config;
    if(auto verified=validate_configuration(key);!verified)
        return std::unexpected(verified.error());
    std::vector<double> cpu,gpu;
    cpu.reserve(samples.size());
    gpu.reserve(samples.size());
    for(const auto& s:samples) {
        if(s.config!=key)
            return std::unexpected(Error::mixed_environment);
        if(s.origin!=Origin::native_metal_observation)
            return std::unexpected(Error::wrong_origin);
        if(s.terminal!=Terminal::completed ||
           !s.pixels_or_pairs_checked || !s.output_correct)
            return std::unexpected(Error::incorrect_output);
        if(!valid_ms(s.cpu_encode_ms) ||
           !valid_ms(s.gpu_command_ms))
            return std::unexpected(Error::invalid_timing);
        for(const auto& counter:s.counters) {
            // No device-supported GPU counter and measured sample =>
            // no counter claim. Even if a counter is optional, its omission
            // must not be silently treated as numeric zero.
            if(counter.identifier.empty() || counter.unit.empty())
                return std::unexpected(Error::invalid_counter);
            if(counter.measured &&
               (!counter.supported ||
                !std::isfinite(*counter.measured)))
                return std::unexpected(Error::unsupported_counter);
        }
        cpu.push_back(*s.cpu_encode_ms);
        gpu.push_back(*s.gpu_command_ms);
    }
    return SeriesSummary{
        key,samples.size(),
        percentile(cpu,0.50),percentile(cpu,0.95),
        percentile(gpu,0.50),percentile(gpu,0.95)
    };
}
} // namespace allmight::gpu_course::profile
