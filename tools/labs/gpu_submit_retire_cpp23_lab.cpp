// AllMight Book01 GPU-14F: cross-project submission, presentation and
// retirement tests. CPU state model and deterministic synthetic backend
// observations ONLY. --probe-native separately calls actual Vulkan if present.
#include "gpu_course/submission_retirement.hpp"
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace allmight::gpu_course {
int run_native_submission_probe();
}

namespace {
namespace sub=allmight::gpu_course::submission;
using Journal=sub::FrameRetirementModel;
using Error=sub::Error;
using Phase=sub::Phase;

void check(bool condition,std::string_view why) {
    if(!condition) throw std::runtime_error(std::string(why));
}
void basic_graphics_and_present_lifetimes() {
    // Two CPU frame slots and three WSI images DO NOT map 1:1.
    Journal j(101,2,3);
    auto kairo_mesh=j.create_resource();
    auto maveb_gaussians=j.create_resource();
    auto xr_gaze=j.create_resource();
    auto kyoto_pair_buffer=j.create_resource();
    check(kairo_mesh && maveb_gaussians && xr_gaze && kyoto_pair_buffer &&
          j.live_resources()==4,"four independent project resource generations");

    const auto f0=j.acquire(0,2);
    check(f0 && f0->slot==0 && f0->image==2 &&
          f0->swapchain_generation==1,
          "acquired WSI index differs from frame slot index");
    check(j.retain(*f0,*kairo_mesh) &&
          j.retain(*f0,*maveb_gaussians) &&
          j.retain(*f0,*kairo_mesh),
          "frame can hold distinct GPU resources and idempotent references");
    check(j.request_destroy(*kairo_mesh),
          "mesh removal becomes deferred retirement while still referenced");
    const auto earlyDestroy=j.destroy_if_safe(*kairo_mesh);
    check(!earlyDestroy && earlyDestroy.error()==Error::resource_in_use,
          "no native vkDestroyBuffer while a pending frame may use it");
    const auto beforeReset=j.submit(*f0);
    check(!beforeReset && beforeReset.error()==Error::fence_not_reset,
          "cannot submit a frame without the modeled reset/recording gate");
    check(j.reset_fence(*f0) && j.submit(*f0),
          "safe modeled reset followed by acceptance transitions to pending");
    const auto repeat=j.reset_fence(*f0);
    check(!repeat && repeat.error()==Error::wrong_state,
          "pending frame cannot reset same fence again");

    // Present submission may happen BEFORE graphics fence completes.
    check(j.present_enqueued(*f0),
          "present enqueued separately from graphics completion");
    check(!j.image_available(2),
          "vkQueuePresentKHR acceptance is not presentation completion");
    const auto other=j.acquire(1,0);
    check(other && j.retain(*other,*maveb_gaussians) &&
          j.retain(*other,*xr_gaze) &&
          j.retain(*other,*kyoto_pair_buffer),
          "independent frame slot references multiple compute/graphics data");
    check(j.reset_fence(*other) && j.submit(*other) &&
          j.present_enqueued(*other),
          "second frame may be simultaneously outstanding");
    check(j.state(0)==Phase::pending && j.state(1)==Phase::pending,
          "multiple frames truly pending in CPU model");
    check(!j.acquire(0,1) && !j.acquire(1,1),
          "no premature CPU frame slot reuse");
    check(!j.recreate_swapchain(2),
          "no swapchain recreation while frame/image work is outstanding");

    const auto fakeRetire=j.retire_frame(*f0);
    check(!fakeRetire && fakeRetire.error()==Error::completion_unproven,
          "presentation enqueue not a graphics-completion proof");
    check(j.observe_gpu_completion(*f0),
          "fixture supplies externally VERIFIED graphics completion");
    const auto retired=j.retire_frame(*f0);
    check(retired && retired->frame_resources_released==2 &&
          retired->resources_destroyable==1 &&
          j.state(0)==Phase::idle,
          "frame arena reusable after fence, candidate mesh destroyable");
    check(!j.image_available(2),
          "render-complete image still owned by WSI after frame slot retirement");
    const auto imageNotFree=j.acquire(0,2);
    check(!imageNotFree && imageNotFree.error()==Error::image_unavailable,
          "graphics fence cannot release presentation image");
    check(j.destroy_if_safe(*kairo_mesh) && j.live_resources()==3,
          "resource destroyed only after last submitted use completes");
    check(!j.destroy_if_safe(*kairo_mesh),
          "old resource generation is permanently invalid");
    check(j.observe_present_release(2,1) && j.image_available(2),
          "only separate modeled WSI completion releases image/semaphore");
    const auto newFrame=j.acquire(0,2);
    check(newFrame && newFrame->generation!=f0->generation,
          "same frame slot advances generation on reuse");
    check(!j.reset_fence(*f0) && !j.retain(*f0,*maveb_gaussians),
          "stale frame token cannot mutate new slot occupant");

    // An acquired image with no queued present cannot be WSI-released.
    check(!j.observe_present_release(2,1),
          "pending-acquired image is not a presented image");
    // Recover the unused new frame only through explicit pre-submit proof.
    check(j.reset_fence(*newFrame) &&
          j.record_or_submit_failed_before_acceptance(*newFrame),
          "simulate record failure after resetting fence");
    check(j.recover_definite_pre_submit(*newFrame,true),
          "verified backend recovery can replace unused unsignaled fence");
    check(j.observe_gpu_completion(*other) &&
          j.retire_frame(*other),
          "other independently pending frame eventually completes");
    check(j.observe_present_release(0,1),
          "second present image independently freed");
    check(j.recreate_swapchain(2) &&
          j.swapchain_generation()==2,
          "swapchain recreated only after verified frame and WSI drain");
    check(!j.observe_present_release(0,1),
          "old swapchain generation cannot release a new image");
}
void kairorenderer_reset_before_submit_failure() {
    Journal j(222,1,2);
    const auto resource=j.create_resource();
    const auto frame=j.acquire(0,0);
    check(frame && resource && j.retain(*frame,*resource),
          "Kairo-like frame acquire and resource lifetime hold");
    check(j.reset_fence(*frame),
          "vkResetFences modeled after WSI acquire");
    check(j.record_or_submit_failed_before_acceptance(*frame),
          "recording or known-rejected submit leaves fence unsignaled");
    check(j.state(0)==Phase::failed_before_submit &&
          !j.acquire(0,1),
          "next DrawFrame cannot wait on a fence no queue will signal");
    const auto invalid=j.submit(*frame);
    check(!invalid && invalid.error()==Error::fence_recovery_needed,
          "submit after failed reset requires a genuine recovery branch");
    const auto noProof=j.recover_definite_pre_submit(*frame,false);
    check(!noProof && noProof.error()==Error::invalid_recovery_proof,
          "a timeout or hopeful recovery flag cannot recycle native objects");
    check(j.request_destroy(*resource) &&
          !j.destroy_if_safe(*resource),
          "aborted frame still holds its referenced resources");
    check(j.recover_definite_pre_submit(*frame,true),
          "explicit independently proven quiescence/rebuilt-fence path");
    check(j.destroy_if_safe(*resource),
          "only completed recovery drops unsubmitted resource reference");
    const auto fresh=j.acquire(0,1);
    check(fresh && fresh->generation!=frame->generation,
          "recovered frame slot gets a new generation");
}
void unknown_acceptance_quarantine_and_device_loss() {
    Journal j(333,1,1);
    auto buffer=j.create_resource();
    auto frame=j.acquire(0,0);
    check(buffer && frame && j.retain(*frame,*buffer) &&
          j.reset_fence(*frame) &&
          j.quarantine_unknown(*frame),
          "unknown driver submission outcome keeps references alive");
    check(j.request_destroy(*buffer) &&
          !j.destroy_if_safe(*buffer),
          "resource must remain live while outcome ambiguous");
    check(!j.observe_gpu_completion(*frame) &&
          !j.retire_frame(*frame) &&
          !j.recover_definite_pre_submit(*frame,true),
          "unknown acceptance cannot be mistaken for definite pre-submit failure");
    check(j.confirm_submission(*frame) &&
          j.present_enqueued(*frame) &&
          j.observe_gpu_completion(*frame),
          "separate backend proof permits completion transition");
    check(j.retire_frame(*frame) &&
          j.destroy_if_safe(*buffer) &&
          j.observe_present_release(0,1),
          "verified acceptance+completion finally frees referenced resource");
    // Distinct model: device loss freezes all mutations.
    Journal lost(444,1,1);
    auto obj=lost.create_resource();
    auto f=lost.acquire(0,0);
    check(obj && f && lost.retain(*f,*obj) &&
          lost.reset_fence(*f) && lost.submit(*f),
          "second fixture submitted work before device loss");
    lost.declare_device_lost();
    check(lost.lost() && !lost.observe_gpu_completion(*f) &&
          !lost.retire_frame(*f) && !lost.recreate_swapchain(1) &&
          !lost.request_destroy(*obj) && !lost.create_resource(),
          "no invented successful recovery after device lost");
}
void resource_generation_and_cpu_pins() {
    Journal j(777,1,1);
    Journal foreign(888,1,1);
    auto obj=j.create_resource();
    auto f=j.acquire(0,0);
    check(obj && f && j.pin_cpu(*obj) &&
          j.retain(*f,*obj),
          "CPU diagnostic pin and GPU frame hold exist independently");
    const auto alien=foreign.retain(*f,*obj);
    check(!alien && alien.error()==Error::wrong_owner,
          "resource/frame owners cannot be forged across schedulers");
    auto fakeResource=*obj;
    fakeResource.owner=888;
    const auto bad=j.request_destroy(fakeResource);
    check(!bad && bad.error()==Error::wrong_owner,
          "foreign resource identity rejected");
    check(j.request_destroy(*obj) &&
          j.reset_fence(*f) && j.submit(*f) &&
          j.present_enqueued(*f) &&
          j.observe_gpu_completion(*f) &&
          j.retire_frame(*f),
          "GPU ref removed after verified completion while CPU pin remains");
    const auto cpuPinned=j.destroy_if_safe(*obj);
    check(!cpuPinned && cpuPinned.error()==Error::readers_pinned,
          "CPU read pin independently prevents destroyed storage");
    check(j.unpin_cpu(*obj) && j.destroy_if_safe(*obj),
          "last CPU pin dropped: resource safe to retire");
    auto replacement=j.create_resource();
    check(replacement && replacement->slot==obj->slot &&
          replacement->generation!=obj->generation,
          "resource slot can be recycled without ABA generation confusion");
    check(!j.pin_cpu(*obj) && !j.request_destroy(*obj),
          "retired generation cannot pin or destroy its new occupant");
    check(j.observe_present_release(0,1) && j.recreate_swapchain(2),
          "image and swapchain lifetimes independently complete");
}
void distinct_swaps_offscreen_and_wrong_states() {
    Journal j(901,2,2);
    auto f=j.acquire(0,1);
    check(f && !j.present_enqueued(*f) &&
          !j.observe_gpu_completion(*f),
          "presentation and completion cannot precede submission");
    check(j.reset_fence(*f) && j.submit(*f),
          "valid submission");
    check(!j.retire_frame(*f),
          "frame retirement requires GPU completion");
    check(j.observe_gpu_completion(*f),
          "device fence completion witnessed");
    const auto noPresent=j.retire_frame(*f);
    check(!noPresent && noPresent.error()==Error::present_not_queued,
          "presented frame cannot skip WSI hand-off");
    check(j.present_enqueued(*f) && j.retire_frame(*f),
          "present queue call may follow GPU completion in CPU fixture");
    check(j.state(0)==Phase::idle && !j.image_available(1),
          "slot reused independently of uncompleted present image");
    auto f2=j.acquire(0,0);
    check(f2 && f2->generation!=f->generation,
          "new frame using different available WSI image");
    check(!j.recreate_swapchain(2),
          "live frame and WSI image block swapchain regeneration");
    check(j.reset_fence(*f2) &&
          j.record_or_submit_failed_before_acceptance(*f2) &&
          j.recover_definite_pre_submit(*f2,true) &&
          j.observe_present_release(1,1) &&
          j.recreate_swapchain(3),
          "explicit cleanup across frame/image/new swapchain generations");
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc==2 && std::string_view(argv[1])=="--probe-native")
            return allmight::gpu_course::run_native_submission_probe();
        if(argc!=1) {
            std::cerr<<"Usage: gpu_submit_retire_cpp23_lab [--probe-native]\n";
            return 2;
        }
        basic_graphics_and_present_lifetimes();
        kairorenderer_reset_before_submit_failure();
        unknown_acceptance_quarantine_and_device_loss();
        resource_generation_and_cpu_pins();
        distinct_swaps_offscreen_and_wrong_states();
        std::cout<<"GPU-14F: Kairo fence reset and WSI present, Maveb "
                   "frame reuse, resource holds, Kyoto/XR buffers, "
                   "quarantine and generational retirement passed "
                   "(portable CPU model only)\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"GPU-14F FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
