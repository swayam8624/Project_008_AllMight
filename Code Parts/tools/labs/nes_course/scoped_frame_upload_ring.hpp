#pragma once
// AllMight Book 01 / NES-11B: guarded CPU upload frames and borrow lifetimes.
//
// CPU-only, single-threaded REFERENCE MODEL. It is not a Vulkan/Metal mapped
// buffer, does not issue a native copy, and does not observe device completion.
// Calling observe_completion is a simulation controlled by the caller; a real
// backend must source those values from verified queue completion events.
//
// Guarantees:
// * Each writer exclusively prepares one free frame slot. Failed replacement
//   does not publish half-written bytes; abandoned writers roll back.
// * Readers hold a shared ownership token for the pool control block plus a
//   read pin on one nonrecyclable slot. The pool wrapper may be destroyed or
//   moved while a reader lives: the bytes themselves stay alive.
// * Returned span<const Byte> is valid for the active ReadLease's interval,
//   provided no unsynchronized concurrency. A caller CAN escape the span;
//   C++ span has no compile-time lifetime tracking. Never retain it beyond
//   the lease. Use copy_snapshot() for an independently owned value.
// * Slot reuse requires BOTH actual reported completion on each queue AND
//   zero active read pins. A frame number or one convenient queue counter
//   never proves all uses have finished.
// * Each handle has pool owner, slot, generation. Generation retirement at
//   uint64_t max prevents ABA through wraparound.
// * The logical frame pool is not the external AssetRegistry; asset source
//   handles must separately be validated at asset import / use boundaries.
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::nes::frames {

using Byte = std::uint8_t;

struct FrameHandle {
    std::uint64_t owner{};
    std::size_t slot{};
    std::uint64_t generation{};
    friend constexpr bool operator==(FrameHandle, FrameHandle) = default;
};

struct CompletionStamp {
    std::uint64_t transfer{};
    std::uint64_t compute{};
    std::uint64_t graphics{};
    friend constexpr bool operator==(CompletionStamp, CompletionStamp) = default;
};
[[nodiscard]] constexpr bool reached(CompletionStamp observed,
                                     CompletionStamp required) noexcept {
    return observed.transfer >= required.transfer &&
           observed.compute >= required.compute &&
           observed.graphics >= required.graphics;
}
[[nodiscard]] constexpr bool monotonic(CompletionStamp previous,
                                       CompletionStamp next) noexcept {
    return next.transfer >= previous.transfer &&
           next.compute >= previous.compute &&
           next.graphics >= previous.graphics;
}
[[nodiscard]] constexpr bool has_pending(CompletionStamp complete,
                                         CompletionStamp stamped) noexcept {
    return !reached(complete, stamped);
}
enum class FrameError {
    moved_from_pool,
    no_free_slot,
    oversized_payload,
    empty_payload,
    no_writer,
    stale_handle,
    wrong_owner,
    invalid_slot,
    not_submitted,
    completion_pending,
    readers_pinned,
    completion_regressed,
    invalid_submission_stamp
};

inline std::atomic<std::uint64_t> next_frame_pool_owner{1};
[[nodiscard]] inline std::uint64_t mint_frame_owner() {
    auto prior=next_frame_pool_owner.load(std::memory_order_relaxed);
    while (prior != std::numeric_limits<std::uint64_t>::max()) {
        if (next_frame_pool_owner.compare_exchange_weak(
            prior,prior+1,std::memory_order_relaxed,
            std::memory_order_relaxed))
            return prior;
    }
    throw std::overflow_error("frame owner namespace exhausted");
}

template<std::size_t Capacity>
class FrameUploadRing {
    static_assert(Capacity>0 && Capacity<=16,
                  "the NES course limits frames in flight to 1..16");
    enum class Phase : std::uint8_t { free, writing, in_flight, exhausted };
    struct Slot {
        std::vector<Byte> payload;
        CompletionStamp required{};
        std::uint64_t generation{1};
        std::size_t readers{};
        Phase phase{Phase::free};
    };
    struct Control {
        explicit Control(std::size_t max_size,std::uint64_t initial)
            : owner(mint_frame_owner()), max_payload(max_size) {
            for (auto& slot:slots) slot.generation=initial;
        }
        std::uint64_t owner{};
        std::size_t max_payload{};
        CompletionStamp completed{};
        std::array<Slot,Capacity> slots{};
    };
public:
    class WriteLease {
    public:
        WriteLease()=default;
        WriteLease(const WriteLease&)=delete;
        WriteLease& operator=(const WriteLease&)=delete;
        WriteLease(WriteLease&& old) noexcept
            : control_(std::move(old.control_)),
              index_(std::exchange(old.index_,0)) {}
        WriteLease& operator=(WriteLease&& other) noexcept {
            if(this!=&other) {
                cancel();
                control_=std::move(other.control_);
                index_=std::exchange(other.index_,0);
            }
            return *this;
        }
        ~WriteLease(){cancel();}

        // Copy into a temporary BEFORE replacing the published candidate.
        // A bad_alloc propagates but does not modify the old staged value.
        [[nodiscard]] std::expected<void,FrameError>
        replace(std::span<const Byte> bytes) {
            if (!control_) return std::unexpected(FrameError::no_writer);
            if (bytes.empty()) return std::unexpected(FrameError::empty_payload);
            if (bytes.size()>control_->max_payload)
                return std::unexpected(FrameError::oversized_payload);
            std::vector<Byte> prepared(bytes.begin(),bytes.end());
            control_->slots[index_].payload.swap(prepared);
            return {};
        }

        // A writer is the only mutable authority for its draft. It never
        // exposes a dangling mutable reference to a submitted slot.
        [[nodiscard]] std::size_t staged_size() const noexcept {
            return control_?control_->slots[index_].payload.size():0;
        }
        // Read-only identity accessor lets a submission queue reject
        // writers belonging to another frame pool BEFORE publication.
        [[nodiscard]] std::uint64_t owner_id() const noexcept {
            return control_?control_->owner:0;
        }
        [[nodiscard]] std::expected<FrameHandle,FrameError>
        submit(CompletionStamp required) noexcept {
            if (!control_) return std::unexpected(FrameError::no_writer);
            auto& control=*control_;
            auto& slot=control.slots[index_];
            if (slot.payload.empty())
                return std::unexpected(FrameError::empty_payload);
            // Nonzero ticket means the respective queue has a pending use.
            // Zero means that queue makes no claim for this submission.
            if ((required.transfer && required.transfer<=control.completed.transfer) ||
                (required.compute && required.compute<=control.completed.compute) ||
                (required.graphics && required.graphics<=control.completed.graphics))
                return std::unexpected(FrameError::invalid_submission_stamp);
            const FrameHandle result{
                control.owner,index_,slot.generation
            };
            slot.required=required;
            slot.phase=Phase::in_flight;
            control_.reset(); // do not auto-cancel successful submission
            return result;
        }

    private:
        friend class FrameUploadRing;
        explicit WriteLease(std::shared_ptr<Control> owner,
                            std::size_t index) noexcept
            : control_(std::move(owner)),index_(index) {}
        void cancel() noexcept {
            if (!control_) return;
            auto& slot=control_->slots[index_];
            if (slot.phase==Phase::writing) {
                slot.payload.clear();
                slot.required={};
                slot.phase=Phase::free;
            }
            control_.reset();
        }
        std::shared_ptr<Control> control_;
        std::size_t index_{};
    };

    class ReadLease {
    public:
        ReadLease()=default;
        ReadLease(const ReadLease&)=delete;
        ReadLease& operator=(const ReadLease&)=delete;
        ReadLease(ReadLease&& other) noexcept
            : control_(std::move(other.control_)),
              index_(std::exchange(other.index_,0)) {}
        ReadLease& operator=(ReadLease&& other) noexcept {
            if (this!=&other) {
                release();
                control_=std::move(other.control_);
                index_=std::exchange(other.index_,0);
            }
            return *this;
        }
        ~ReadLease(){release();}

        [[nodiscard]] std::span<const Byte> view() const noexcept {
            if (!control_) return {};
            const auto& payload=control_->slots[index_].payload;
            return {payload.data(),payload.size()};
        }
        // Borrow can escape its scope if the user stores view() beyond the
        // lease; copy_snapshot is the explicit ownership escape hatch.
        [[nodiscard]] std::vector<Byte> copy_snapshot() const {
            const auto source=view();
            return {source.begin(),source.end()};
        }
        [[nodiscard]] bool active() const noexcept {
            return static_cast<bool>(control_);
        }

    private:
        friend class FrameUploadRing;
        explicit ReadLease(std::shared_ptr<Control> control,
                           std::size_t index) noexcept
            : control_(std::move(control)),index_(index) {}
        void release() noexcept {
            if (!control_) return;
            auto& slot=control_->slots[index_];
            // No external mutation/recycling can occur while this pin lives
            // under the documented single-threaded access protocol.
            if (slot.readers>0) --slot.readers;
            control_.reset();
        }
        std::shared_ptr<Control> control_;
        std::size_t index_{};
    };

    explicit FrameUploadRing(std::size_t max_payload,
                             std::uint64_t test_initial_generation=1)
        : control_([&]() -> std::shared_ptr<Control> {
            if (max_payload==0 ||
                test_initial_generation==0)
                throw std::invalid_argument("zero payload or generation");
            return std::make_shared<Control>(
                max_payload,test_initial_generation);
        }()) {}

    FrameUploadRing(const FrameUploadRing&)=delete;
    FrameUploadRing& operator=(const FrameUploadRing&)=delete;
    FrameUploadRing(FrameUploadRing&&) noexcept=default;
    FrameUploadRing& operator=(FrameUploadRing&&) noexcept=default;

    [[nodiscard]] std::expected<WriteLease,FrameError> begin() noexcept {
        if (!control_) return std::unexpected(FrameError::moved_from_pool);
        for (std::size_t i=0;i<Capacity;++i) {
            auto& slot=control_->slots[i];
            if (slot.phase==Phase::free) {
                slot.phase=Phase::writing;
                return WriteLease{control_,i};
            }
        }
        return std::unexpected(FrameError::no_free_slot);
    }

    [[nodiscard]] std::expected<ReadLease,FrameError>
    borrow(FrameHandle id) noexcept {
        auto index=validate(id);
        if (!index) return std::unexpected(index.error());
        auto& slot=control_->slots[*index];
        if (slot.phase!=Phase::in_flight)
            return std::unexpected(FrameError::not_submitted);
        ++slot.readers; // API is single-threaded and max pins bounded by RAM
        return ReadLease{control_,*index};
    }

    [[nodiscard]] std::expected<void,FrameError>
    observe_completion(CompletionStamp completed) noexcept {
        if (!control_) return std::unexpected(FrameError::moved_from_pool);
        if (!monotonic(control_->completed,completed))
            return std::unexpected(FrameError::completion_regressed);
        control_->completed=completed;
        return {};
    }

    [[nodiscard]] std::expected<void,FrameError>
    retire(FrameHandle id) noexcept {
        const auto index=validate(id);
        if (!index) return std::unexpected(index.error());
        auto& slot=control_->slots[*index];
        if (slot.phase!=Phase::in_flight)
            return std::unexpected(FrameError::not_submitted);
        if (has_pending(control_->completed,slot.required))
            return std::unexpected(FrameError::completion_pending);
        if (slot.readers)
            return std::unexpected(FrameError::readers_pinned);
        slot.payload.clear();
        slot.required={};
        if (slot.generation==std::numeric_limits<std::uint64_t>::max()) {
            slot.phase=Phase::exhausted; // never reuse a wrapped identity
        } else {
            ++slot.generation;
            slot.phase=Phase::free;
        }
        return {};
    }

    [[nodiscard]] std::size_t available() const noexcept {
        if (!control_) return 0;
        std::size_t count{};
        for (const auto& slot:control_->slots)
            if (slot.phase==Phase::free) ++count;
        return count;
    }
    [[nodiscard]] CompletionStamp completed() const noexcept {
        return control_?control_->completed:CompletionStamp{};
    }
    [[nodiscard]] std::uint64_t owner_id() const noexcept {
        return control_?control_->owner:0;
    }

private:
    [[nodiscard]] std::expected<std::size_t,FrameError>
    validate(FrameHandle id) const noexcept {
        if (!control_) return std::unexpected(FrameError::moved_from_pool);
        if (id.owner!=control_->owner)
            return std::unexpected(FrameError::wrong_owner);
        if (id.slot>=Capacity)
            return std::unexpected(FrameError::invalid_slot);
        if (id.generation!=control_->slots[id.slot].generation)
            return std::unexpected(FrameError::stale_handle);
        return id.slot;
    }
    std::shared_ptr<Control> control_;
};
static_assert(!std::is_copy_constructible_v<FrameUploadRing<3>>);
static_assert(std::is_nothrow_move_constructible_v<FrameUploadRing<3>>);
static_assert(!std::is_copy_constructible_v<FrameUploadRing<3>::ReadLease>);
static_assert(!std::is_copy_constructible_v<FrameUploadRing<3>::WriteLease>);
static_assert(std::is_trivially_copyable_v<CompletionStamp>);

} // namespace allmight::nes::frames
