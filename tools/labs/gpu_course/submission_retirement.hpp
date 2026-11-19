#pragma once
// AllMight Book01 GPU-14F: cross-project *CPU MODEL* of frame submission,
// native-fence reset failure, distinct swapchain-image lifetime, and deferred
// resource destruction. Applies to Kairo Vulkan, Maveb Metal, Kyoto compute,
// XR frame pipelines; nothing in here calls a graphics API or schedules work.
//
// All state transitions are single-threaded, deterministic test observations.
// In particular, "observe_gpu_completion" must be called only after a
// backend-verified VkFence/timeline or Metal command-buffer completion.
// "observe_present_release" must be backed by WSI/present completion or a
// verified image-reacquisition protocol; frame-fence signaled != present done.
//
// No real queue semaphore, stage/access dependency or GPU visibility is
// simulated. Separate native executable (GPU-14F --probe-native) is needed
// for actual vkQueueSubmit + vkWaitForFences proof when runtime permits.
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace allmight::gpu_course::submission {
enum class Phase : std::uint8_t {
    idle, image_acquired, fence_reset, pending, gpu_complete,
    failed_before_submit, quarantined
};
enum class Error : std::uint8_t {
    bad_slot, busy_slot, bad_image, image_unavailable,
    wrong_owner, stale_ticket, wrong_state, fence_not_reset,
    fence_recovery_needed, completion_unproven,
    present_not_queued, image_still_acquired,
    bad_resource, stale_resource, resource_not_retired,
    already_retiring, resource_in_use, readers_pinned,
    pending_submission, device_lost, generation_exhausted,
    invalid_recovery_proof
};
struct FrameToken {
    std::uint64_t owner{};
    std::uint32_t slot{};
    std::uint64_t generation{};
    std::uint32_t image{};
    std::uint64_t swapchain_generation{};
    friend bool operator==(FrameToken,FrameToken)=default;
};
struct ResourceToken {
    std::uint64_t owner{};
    std::uint32_t slot{};
    std::uint64_t generation{};
    friend bool operator==(ResourceToken,ResourceToken)=default;
};
struct RetireInfo {
    std::size_t frame_resources_released{};
    std::size_t resources_destroyable{};
};
// An owner token is an explicit constructor argument so tests can assert
// foreign owner rejection; applications must assign globally unique owner IDs
// per logical scheduler instance and never accidentally reuse them.
class FrameRetirementModel {
    struct Frame {
        Phase phase{Phase::idle};
        std::uint64_t generation{1};
        std::uint32_t image{};
        std::uint64_t swapchain_generation{};
        bool present_queued{};
        std::vector<ResourceToken> references;
    };
    struct Image {
        enum class State { free, acquired, presented } state{State::free};
        std::uint64_t generation{1};
    };
    struct Resource {
        std::uint64_t generation{1};
        bool live{};
        bool requested_retire{};
        std::size_t frame_holds{};
        std::size_t cpu_read_pins{};
    };
public:
    explicit FrameRetirementModel(std::uint64_t owner,
                                  std::size_t frames,std::size_t images)
        : owner_(owner),frames_(frames),images_(images) {
        if(owner==0 || frames==0 || images==0 ||
           frames>std::numeric_limits<std::uint32_t>::max() ||
           images>std::numeric_limits<std::uint32_t>::max())
            throw std::invalid_argument("FrameRetirementModel dimensions/owner");
    }
    FrameRetirementModel(const FrameRetirementModel&)=delete;
    FrameRetirementModel& operator=(const FrameRetirementModel&)=delete;

    // A VkSwapchainKHR recreation requires a verified drain / device-idle
    // contract. Call this only when no frame or image is in use.
    [[nodiscard]] std::expected<void,Error> recreate_swapchain(
        std::size_t new_image_count) {
        if(lost_) return std::unexpected(Error::device_lost);
        if(new_image_count==0 ||
           new_image_count>std::numeric_limits<std::uint32_t>::max())
            return std::unexpected(Error::bad_image);
        for(const auto& frame:frames_)
            if(frame.phase!=Phase::idle)
                return std::unexpected(Error::pending_submission);
        for(const auto& image:images_)
            if(image.state!=Image::State::free)
                return std::unexpected(Error::image_unavailable);
        if(swapchain_generation_==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(Error::generation_exhausted);
        std::vector<Image> replacement(new_image_count);
        images_.swap(replacement);
        ++swapchain_generation_;
        return {};
    }

    // Explicit argument means a *real* acquire callback reported this index;
    // the Vulkan acquire semaphore still needs a properly staged device wait.
    // CPU fixture is deliberately not proof of native image availability.
    [[nodiscard]] std::expected<FrameToken,Error> acquire(
        std::uint32_t slot,std::uint32_t image) noexcept {
        if(lost_) return std::unexpected(Error::device_lost);
        if(slot>=frames_.size()) return std::unexpected(Error::bad_slot);
        if(image>=images_.size()) return std::unexpected(Error::bad_image);
        auto& frame=frames_[slot];
        if(frame.phase!=Phase::idle)
            return std::unexpected(Error::busy_slot);
        if(images_[image].state!=Image::State::free)
            return std::unexpected(Error::image_unavailable);
        frame.phase=Phase::image_acquired;
        frame.image=image;
        frame.swapchain_generation=swapchain_generation_;
        frame.present_queued=false;
        images_[image].state=Image::State::acquired;
        return FrameToken{owner_,slot,frame.generation,image,
                          swapchain_generation_};
    }
    [[nodiscard]] std::expected<void,Error>
    reset_fence(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::image_acquired)
            return std::unexpected(Error::wrong_state);
        (*x)->phase=Phase::fence_reset;
        return {};
    }
    // Models a definite pre-submission failure after fence reset. The real
    // fence is unsignaled: no future frame must wait on it until an explicit
    // recovery operation recreates the fence/WSI synchronization as required.
    [[nodiscard]] std::expected<void,Error>
    record_or_submit_failed_before_acceptance(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::fence_reset)
            return std::unexpected(Error::wrong_state);
        (*x)->phase=Phase::failed_before_submit;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    submit(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::fence_reset)
            return std::unexpected(
                (*x)->phase==Phase::failed_before_submit ?
                Error::fence_recovery_needed : Error::fence_not_reset);
        (*x)->phase=Phase::pending;
        return {};
    }
    // An outcome whose acceptance is unknown must not be treated as
    // a definite submission failure. It retains references and image.
    [[nodiscard]] std::expected<void,Error>
    quarantine_unknown(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::fence_reset)
            return std::unexpected(Error::wrong_state);
        (*x)->phase=Phase::quarantined;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    confirm_submission(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::quarantined)
            return std::unexpected(Error::wrong_state);
        (*x)->phase=Phase::pending;
        return {};
    }
    // This represents a backend-confirmed fence wait / timeline threshold /
    // native Metal terminal-completion event, not an elapsed timeout.
    [[nodiscard]] std::expected<void,Error>
    observe_gpu_completion(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::pending)
            return std::unexpected(Error::completion_unproven);
        (*x)->phase=Phase::gpu_complete;
        return {};
    }
    // vkQueuePresentKHR accepted/present enqueued does NOT imply that image
    // or its render-finished binary semaphore is free for reuse.
    [[nodiscard]] std::expected<void,Error>
    present_enqueued(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::pending &&
           (*x)->phase!=Phase::gpu_complete)
            return std::unexpected(Error::wrong_state);
        if((*x)->present_queued)
            return std::unexpected(Error::wrong_state);
        auto& image=images_[ticket.image];
        if(image.state!=Image::State::acquired)
            return std::unexpected(Error::image_unavailable);
        (*x)->present_queued=true;
        image.state=Image::State::presented;
        return {};
    }
    // The *presentation owner* must independently prove presentation/WSI
    // no longer owns the image and its binary semaphore. Graphics fence
    // completion alone is insufficient. No native WSI proof in CPU model.
    [[nodiscard]] std::expected<void,Error>
    observe_present_release(std::uint32_t image,
                            std::uint64_t swapchain_generation) noexcept {
        if(lost_) return std::unexpected(Error::device_lost);
        if(swapchain_generation!=swapchain_generation_)
            return std::unexpected(Error::stale_ticket);
        if(image>=images_.size()) return std::unexpected(Error::bad_image);
        if(images_[image].state!=Image::State::presented)
            return std::unexpected(Error::present_not_queued);
        images_[image].state=Image::State::free;
        return {};
    }
    // This call requires an external *verified* recovery: the native fence
    // and consumed/abandoned acquire semaphore/image have been reconstructed
    // or fully quiesced, and a new usable signaled fence is installed.
    // A bare timeout, failed API result or "no commands recorded" is NOT
    // sufficient proof. Recovering the fixture is intentionally explicit.
    [[nodiscard]] std::expected<void,Error>
    recover_definite_pre_submit(FrameToken ticket,
                                bool backend_proved_quiescent) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::failed_before_submit)
            return std::unexpected(Error::wrong_state);
        if(!backend_proved_quiescent)
            return std::unexpected(Error::invalid_recovery_proof);
        if((*x)->generation==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(Error::generation_exhausted);
        if(images_[ticket.image].state!=Image::State::acquired)
            return std::unexpected(Error::image_unavailable);
        if(lost_) return std::unexpected(Error::device_lost);
        release_references(**x);
        images_[ticket.image].state=Image::State::free;
        reset_to_idle(**x);
        return {};
    }
    // The frame slot and its own upload arena can be reused after observed
    // GPU completion, but the present image has an INDEPENDENT WSI lifetime.
    [[nodiscard]] std::expected<RetireInfo,Error>
    retire_frame(FrameToken ticket) noexcept {
        auto x=checked(ticket);
        if(!x) return std::unexpected(x.error());
        if((*x)->phase!=Phase::gpu_complete)
            return std::unexpected(Error::completion_unproven);
        if(!(*x)->present_queued)
            return std::unexpected(Error::present_not_queued);
        if((*x)->generation==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(Error::generation_exhausted);
        const auto count=(*x)->references.size();
        release_references(**x);
        reset_to_idle(**x);
        return RetireInfo{count,collect_retired()};
    }
    [[nodiscard]] Phase state(std::uint32_t slot) const noexcept {
        return slot<frames_.size()?frames_[slot].phase:Phase::quarantined;
    }
    [[nodiscard]] bool image_available(std::uint32_t index) const noexcept {
        return index<images_.size() &&
               images_[index].state==Image::State::free;
    }
    [[nodiscard]] std::uint64_t swapchain_generation() const noexcept {
        return swapchain_generation_;
    }
    [[nodiscard]] std::size_t live_resources() const noexcept {
        std::size_t n=0;
        for(const auto& r:resources_) if(r.live) ++n;
        return n;
    }
    // Device lost stops all automatic transitions. Native destruction may
    // require verified reset/idle protocol; this model does not fake it.
    void declare_device_lost() noexcept { lost_=true; }
    [[nodiscard]] bool lost() const noexcept {return lost_;}

    [[nodiscard]] std::expected<ResourceToken,Error> create_resource() {
        if(lost_) return std::unexpected(Error::device_lost);
        // Stable vector slot avoids reusing an ID while any in-flight
        // command or pinned CPU accessor can still refer to its generation.
        for(std::size_t i=0;i<resources_.size();++i) {
            auto& r=resources_[i];
            if(!r.live &&
               r.generation!=std::numeric_limits<std::uint64_t>::max()) {
                ++r.generation;
                r.live=true;
                r.requested_retire=false;
                return ResourceToken{owner_,static_cast<std::uint32_t>(i),
                                     r.generation};
            }
        }
        if(resources_.size()>=std::numeric_limits<std::uint32_t>::max())
            return std::unexpected(Error::generation_exhausted);
        resources_.push_back(Resource{1,true,false,0,0});
        return ResourceToken{owner_,
             static_cast<std::uint32_t>(resources_.size()-1),1};
    }
    [[nodiscard]] std::expected<void,Error>
    retain(FrameToken ticket,ResourceToken resource) {
        auto f=checked(ticket);
        if(!f) return std::unexpected(f.error());
        if((*f)->phase!=Phase::image_acquired &&
           (*f)->phase!=Phase::fence_reset)
            return std::unexpected(Error::wrong_state);
        auto r=checked(resource);
        if(!r) return std::unexpected(r.error());
        if((*r)->requested_retire) return std::unexpected(Error::already_retiring);
        // A given command frame owns each resource once, even if multiple
        // draw/dispatch operations in that frame reference it.
        for(auto held:(*f)->references)
            if(held==resource) return {};
        (*f)->references.push_back(resource); // may allocate/throw
        ++(*r)->frame_holds;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    request_destroy(ResourceToken resource) noexcept {
        auto r=checked(resource);
        if(!r) return std::unexpected(r.error());
        if((*r)->requested_retire)
            return std::unexpected(Error::already_retiring);
        (*r)->requested_retire=true;
        return {};
    }
    // CPU diagnostic readers are explicit tokens in the *test model*.
    // Real code should use RAII read leases instead of manually unpinning.
    [[nodiscard]] std::expected<void,Error>
    pin_cpu(ResourceToken resource) noexcept {
        auto r=checked(resource);
        if(!r) return std::unexpected(r.error());
        if((*r)->requested_retire)
            return std::unexpected(Error::already_retiring);
        if((*r)->cpu_read_pins==std::numeric_limits<std::size_t>::max())
            return std::unexpected(Error::resource_in_use);
        ++(*r)->cpu_read_pins;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    unpin_cpu(ResourceToken resource) noexcept {
        auto r=checked(resource);
        if(!r) return std::unexpected(r.error());
        if((*r)->cpu_read_pins==0)
            return std::unexpected(Error::wrong_state);
        --(*r)->cpu_read_pins;
        return {};
    }
    [[nodiscard]] std::expected<void,Error>
    destroy_if_safe(ResourceToken resource) noexcept {
        auto r=checked(resource);
        if(!r) return std::unexpected(r.error());
        if(!(*r)->requested_retire)
            return std::unexpected(Error::resource_not_retired);
        if((*r)->frame_holds!=0)
            return std::unexpected(Error::resource_in_use);
        if((*r)->cpu_read_pins!=0)
            return std::unexpected(Error::readers_pinned);
        (*r)->live=false;
        (*r)->requested_retire=false;
        return {};
    }
private:
    [[nodiscard]] std::expected<Frame*,Error>
    checked(FrameToken t) noexcept {
        if(t.owner!=owner_) return std::unexpected(Error::wrong_owner);
        if(t.slot>=frames_.size()) return std::unexpected(Error::bad_slot);
        auto& f=frames_[t.slot];
        if(f.phase==Phase::idle || f.generation!=t.generation ||
           f.image!=t.image || f.swapchain_generation!=t.swapchain_generation)
            return std::unexpected(Error::stale_ticket);
        return &f;
    }
    [[nodiscard]] std::expected<Resource*,Error>
    checked(ResourceToken t) noexcept {
        if(t.owner!=owner_) return std::unexpected(Error::wrong_owner);
        if(t.slot>=resources_.size())
            return std::unexpected(Error::bad_resource);
        auto& r=resources_[t.slot];
        if(!r.live || r.generation!=t.generation)
            return std::unexpected(Error::stale_resource);
        return &r;
    }
    void release_references(Frame& f) noexcept {
        for(const auto& t:f.references)
            --resources_[t.slot].frame_holds;
        f.references.clear();
    }
    static void reset_to_idle(Frame& f) noexcept {
        f.phase=Phase::idle;
        f.present_queued=false;
        ++f.generation;
    }
    [[nodiscard]] std::size_t collect_retired() noexcept {
        std::size_t eligible=0;
        for(auto& r:resources_)
            if(r.live && r.requested_retire &&
               r.frame_holds==0 && r.cpu_read_pins==0)
                ++eligible;
        return eligible; // count eligible, never silently vkDestroy*
    }
    std::uint64_t owner_;
    std::vector<Frame> frames_;
    std::vector<Image> images_;
    std::vector<Resource> resources_;
    std::uint64_t swapchain_generation_{1};
    bool lost_{};
};
} // namespace allmight::gpu_course::submission
