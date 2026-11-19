#pragma once
// Project AllMight / GPU-15M — portable Metal frame-lifetime reference model.
//
// Primary project: Maveb Metal FrameContext/Renderer, with Kairo's Metal
// runtime, Gaussian attributes, Kyoto collision pairs and XR as clients.
//
// This is a deterministic, SINGLE-THREADED C++23 model, not an MTLDevice,
// MTLCommandBuffer, CAMetalDrawable or GPU timer. Host code must supply
// completion observations from a REAL terminal Metal command-buffer status.
// In particular, an MTLCommandBufferStatusError releases a frame permit
// *without* claiming correct pixels or useful GPU timing.
//
// Raw MTLBuffer resource ownership, hardware memory visibility and actual
// callback threading belong in the platform adapter. The optional
// native_metal_compute_probe.mm provides a separate real GPU oracle.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace allmight::gpu_course::metal {
using Byte = std::uint8_t;

enum class Project : std::uint8_t {
    maveb_gaussian,
    kairo_scene,
    kyoto_collision_pairs,
    xr_gaze,
    nes_overlay
};
enum class Phase : std::uint8_t {
    free, recording, encoded, submitted, unknown_acceptance, exhausted
};
enum class Terminal : std::uint8_t { completed, error };
enum class Error : std::uint8_t {
    moved_or_foreign_frame, bad_slot, stale_generation, no_free_slot,
    invalid_alignment, zero_length, arena_full, arithmetic_overflow,
    bad_phase, mutating_submitted_bytes, no_completion_evidence,
    malformed_capture, invalid_capture_extent, max_generation,
    invalid_terminal, device_lost
};

struct FrameKey {
    std::uint64_t owner{};
    std::size_t slot{};
    std::uint64_t generation{};
    friend constexpr bool operator==(FrameKey,FrameKey)=default;
};
struct ByteRange {
    std::size_t offset{};
    std::size_t length{};
    friend constexpr bool operator==(ByteRange,ByteRange)=default;
};
struct RawTiming {
    double start_seconds{}; // value from native GPUStartTime
    double end_seconds{};   // value from native GPUEndTime
};
struct Capture {
    std::uint32_t width{};
    std::uint32_t height{};
    // Original Metal drawable texture is ordinarily BGRA8. This preserves
    // byte order, but makes NO gamma, HDR, premultiplication, orientation or
    // color-space conversion claim. Native image format must be checked.
    std::vector<Byte> bgra8_tightly_packed;
};
struct Readback {
    std::uint32_t width{};
    std::uint32_t height{};
    std::size_t gpu_bytes_per_row{};
    std::span<const Byte> gpu_bytes;
};
struct TerminalEvent {
    FrameKey frame{};
    Project project{};
    Terminal status{Terminal::error};
    std::optional<double> measured_gpu_milliseconds;
    std::optional<Capture> capture;
};
[[nodiscard]] constexpr bool power_of_two(std::size_t n) noexcept {
    return n!=0 && (n&(n-1))==0;
}
[[nodiscard]] inline std::expected<std::size_t,Error>
checked_align_up(std::size_t value,std::size_t alignment) noexcept {
    if(!power_of_two(alignment))
        return std::unexpected(Error::invalid_alignment);
    const auto padding=alignment-1;
    if(value>std::numeric_limits<std::size_t>::max()-padding)
        return std::unexpected(Error::arithmetic_overflow);
    return (value+padding)&~padding;
}
[[nodiscard]] inline std::optional<double>
measured_gpu_milliseconds(std::optional<RawTiming> reported) noexcept {
    if(!reported) return std::nullopt;
    // Apple documents 0.0 values before command completion. An unsupported
    // timing facility, missing values, nonfinite values, clock regression or
    // zero interval must not be silently transformed into a "0 ms GPU".
    if(!std::isfinite(reported->start_seconds) ||
       !std::isfinite(reported->end_seconds) ||
       reported->start_seconds<=0.0 ||
       reported->end_seconds<=reported->start_seconds)
        return std::nullopt;
    const double ms=(reported->end_seconds-reported->start_seconds)*1000.0;
    if(!std::isfinite(ms) || ms<=0.0) return std::nullopt;
    return ms;
}
// The pitch contract is an independent CPU transform, invoked ONLY after
// terminal *success* has been observed in a real native backend. A CPU
// fixture may call it on synthetic bytes but may not claim GPU data was read.
[[nodiscard]] inline std::expected<Capture,Error>
compact_bgra8(Readback input) {
    if(input.width==0 || input.height==0)
        return std::unexpected(Error::invalid_capture_extent);
    const auto width=static_cast<std::size_t>(input.width);
    const auto height=static_cast<std::size_t>(input.height);
    if(width>std::numeric_limits<std::size_t>::max()/4)
        return std::unexpected(Error::arithmetic_overflow);
    const auto compact_row=width*4;
    if(input.gpu_bytes_per_row<compact_row ||
       input.gpu_bytes_per_row%256!=0)
        return std::unexpected(Error::malformed_capture);
    if(height>std::numeric_limits<std::size_t>::max()/
               input.gpu_bytes_per_row ||
       height>std::numeric_limits<std::size_t>::max()/compact_row)
        return std::unexpected(Error::arithmetic_overflow);
    const auto padded_size=height*input.gpu_bytes_per_row;
    if(input.gpu_bytes.size()<padded_size)
        return std::unexpected(Error::malformed_capture);
    Capture out;
    out.width=input.width;
    out.height=input.height;
    out.bgra8_tightly_packed.resize(height*compact_row);
    for(std::size_t row=0;row<height;++row) {
        std::copy_n(input.gpu_bytes.data()+row*input.gpu_bytes_per_row,
                    compact_row,
                    out.bgra8_tightly_packed.data()+row*compact_row);
    }
    return out;
}
class MetalFrameModel {
    struct Slot {
        Phase phase{Phase::free};
        Project project{Project::maveb_gaussian};
        std::uint64_t generation{1};
        std::size_t cursor{};
        std::vector<Byte> upload;
    };
public:
    explicit MetalFrameModel(std::uint64_t owner,std::size_t frame_count=3,
                             std::size_t upload_capacity=8U*1024U*1024U,
                             std::uint64_t test_generation=1)
        : owner_(owner),slots_(frame_count) {
        if(owner==0 || frame_count==0 || frame_count>16 ||
           upload_capacity==0 || test_generation==0)
            throw std::invalid_argument("frame capacity/owner/generation");
        for(auto& slot:slots_) {
            slot.upload.resize(upload_capacity);
            slot.generation=test_generation;
        }
    }
    MetalFrameModel(const MetalFrameModel&)=delete;
    MetalFrameModel& operator=(const MetalFrameModel&)=delete;

    [[nodiscard]] std::expected<FrameKey,Error> begin(Project project) noexcept {
        if(lost_) return std::unexpected(Error::device_lost);
        for(std::size_t index=0;index<slots_.size();++index) {
            auto& slot=slots_[index];
            if(slot.phase==Phase::free) {
                slot.cursor=0;  // safe ONLY because prior completion freed it
                slot.project=project;
                slot.phase=Phase::recording;
                return FrameKey{owner_,index,slot.generation};
            }
        }
        return std::unexpected(Error::no_free_slot);
    }
    [[nodiscard]] std::expected<ByteRange,Error>
    stage(FrameKey key,std::span<const Byte> bytes,
          std::size_t alignment) {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        auto* slot=*checked;
        if(slot->phase!=Phase::recording)
            return std::unexpected(Error::mutating_submitted_bytes);
        if(bytes.empty()) return std::unexpected(Error::zero_length);
        auto at=checked_align_up(slot->cursor,alignment);
        if(!at) return std::unexpected(at.error());
        if(*at>slot->upload.size() ||
           bytes.size()>slot->upload.size()-*at)
            return std::unexpected(Error::arena_full);
        // Copy comes before cursor publication. In the portable model, the
        // CPU source does not alias this private upload vector. A native
        // backend must explicitly handle aliasing and mapped ranges.
        std::copy(bytes.begin(),bytes.end(),slot->upload.begin()+*at);
        slot->cursor=*at+bytes.size();
        return ByteRange{*at,bytes.size()};
    }
    [[nodiscard]] std::expected<std::vector<Byte>,Error>
    snapshot_staged(FrameKey key,ByteRange range) const {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        const auto* slot=*checked;
        if(range.offset>slot->cursor ||
           range.length>slot->cursor-range.offset)
            return std::unexpected(Error::arena_full);
        const auto b=slot->upload.begin()+range.offset;
        return std::vector<Byte>(b,b+range.length);
    }
    [[nodiscard]] std::expected<void,Error>
    seal(FrameKey key) noexcept {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        if((*checked)->phase!=Phase::recording)
            return std::unexpected(Error::bad_phase);
        if((*checked)->cursor==0)
            return std::unexpected(Error::zero_length);
        (*checked)->phase=Phase::encoded;
        return {};
    }
    // Known no-submit failures may abandon the frame; ambiguous outcomes
    // must be quarantined until an actual backend acceptance/recovery proof.
    [[nodiscard]] std::expected<void,Error>
    fail_before_commit(FrameKey key) noexcept {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        if((*checked)->phase!=Phase::recording &&
           (*checked)->phase!=Phase::encoded)
            return std::unexpected(Error::bad_phase);
        return reset(**checked);
    }
    [[nodiscard]] std::expected<void,Error>
    commit(FrameKey key) noexcept {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        if((*checked)->phase!=Phase::encoded)
            return std::unexpected(Error::bad_phase);
        (*checked)->phase=Phase::submitted;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    uncertain_commit(FrameKey key) noexcept {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        if((*checked)->phase!=Phase::encoded)
            return std::unexpected(Error::bad_phase);
        (*checked)->phase=Phase::unknown_acceptance;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    confirm_accepted(FrameKey key) noexcept {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        if((*checked)->phase!=Phase::unknown_acceptance)
            return std::unexpected(Error::bad_phase);
        (*checked)->phase=Phase::submitted;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    observe_terminal(FrameKey key,Terminal status,
                     std::optional<RawTiming> native_time=std::nullopt,
                     std::optional<Readback> optional_capture=std::nullopt) {
        auto checked=slot_for(key);
        if(!checked) return std::unexpected(checked.error());
        auto* slot=*checked;
        if(slot->phase!=Phase::submitted)
            return std::unexpected(Error::no_completion_evidence);
        if(slot->generation==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(Error::max_generation);

        TerminalEvent event;
        event.frame=key;
        event.project=slot->project;
        event.status=status;
        if(status==Terminal::completed) {
            event.measured_gpu_milliseconds =
                measured_gpu_milliseconds(native_time);
            if(optional_capture) {
                auto compact=compact_bgra8(*optional_capture);
                if(!compact) return std::unexpected(compact.error());
                event.capture.emplace(std::move(*compact));
            }
        } // Error paths never publish a successful image or GPU timing.
        events_.push_back(std::move(event));  // prepare before permit release
        return reset(*slot); // now (and only now) the frame permit is reusable
    }
    [[nodiscard]] std::vector<TerminalEvent> drain_events() {
        std::vector<TerminalEvent> result;
        result.swap(events_);
        return result;
    }
    [[nodiscard]] std::size_t outstanding() const noexcept {
        std::size_t pending=0;
        for(const auto& slot:slots_)
            if(slot.phase!=Phase::free && slot.phase!=Phase::exhausted)
                ++pending;
        return pending;
    }
    [[nodiscard]] std::size_t available() const noexcept {
        std::size_t free=0;
        for(const auto& slot:slots_)
            if(slot.phase==Phase::free) ++free;
        return free;
    }
    [[nodiscard]] std::uint64_t owner_id() const noexcept {return owner_;}
    [[nodiscard]] Phase state(std::size_t index) const noexcept {
        return index<slots_.size()?slots_[index].phase:Phase::exhausted;
    }
    void device_lost() noexcept { lost_=true; }
    [[nodiscard]] bool lost() const noexcept { return lost_; }

private:
    [[nodiscard]] std::expected<Slot*,Error>
    slot_for(FrameKey key) noexcept {
        if(lost_) return std::unexpected(Error::device_lost);
        if(key.owner!=owner_) return std::unexpected(Error::moved_or_foreign_frame);
        if(key.slot>=slots_.size()) return std::unexpected(Error::bad_slot);
        auto& slot=slots_[key.slot];
        if(slot.phase==Phase::free || slot.phase==Phase::exhausted ||
           slot.generation!=key.generation)
            return std::unexpected(Error::stale_generation);
        return &slot;
    }
    [[nodiscard]] std::expected<const Slot*,Error>
    slot_for(FrameKey key) const noexcept {
        if(lost_) return std::unexpected(Error::device_lost);
        if(key.owner!=owner_) return std::unexpected(Error::moved_or_foreign_frame);
        if(key.slot>=slots_.size()) return std::unexpected(Error::bad_slot);
        const auto& slot=slots_[key.slot];
        if(slot.phase==Phase::free || slot.phase==Phase::exhausted ||
           slot.generation!=key.generation)
            return std::unexpected(Error::stale_generation);
        return &slot;
    }
    static std::expected<void,Error> reset(Slot& slot) noexcept {
        slot.cursor=0;
        if(slot.generation==std::numeric_limits<std::uint64_t>::max()) {
            slot.phase=Phase::exhausted;
            return {};
        }
        ++slot.generation;
        slot.phase=Phase::free;
        return {};
    }
    const std::uint64_t owner_;
    std::vector<Slot> slots_;
    std::vector<TerminalEvent> events_;
    bool lost_{};
};
} // namespace allmight::gpu_course::metal
