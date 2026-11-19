// AllMight Book01 / GPU-15M — cross-project Metal frame correctness.
//
// Default CTest is CPU ONLY and works on Linux or macOS. Its command-buffer
// completion observations are deterministic fixtures, NOT a GPU execution
// report. --probe-native and --benchmark-native separately run actual Metal
// shader dispatch through a macOS Objective-C++ adapter when present.
#include "gpu_course/metal_frame_contracts.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::gpu_course {
int run_native_metal_probe(bool benchmark);
}
namespace {
namespace m=allmight::gpu_course::metal;
using m::Error;
using m::FrameKey;
using m::MetalFrameModel;
using m::Phase;
using m::Project;
using m::Terminal;

void check(bool yes,std::string_view message) {
    if(!yes) throw std::runtime_error(std::string(message));
}
std::vector<m::Byte> rgba_fixture(std::size_t n,std::uint8_t bias=0) {
    std::vector<m::Byte> bytes(n);
    for(std::size_t i=0;i<n;++i)
        bytes[i]=static_cast<m::Byte>((i+bias)&255U);
    return bytes;
}

void test_three_permits_maveb_kairo_and_collision() {
    // Three is Maveb's inspected frame permit count, not a GPU measurement.
    MetalFrameModel frames(1001,3,512);
    const auto gaussian=frames.begin(Project::maveb_gaussian);
    const auto kairo=frames.begin(Project::kairo_scene);
    const auto pairs=frames.begin(Project::kyoto_collision_pairs);
    check(gaussian && kairo && pairs &&
          gaussian->slot==0 && kairo->slot==1 && pairs->slot==2 &&
          frames.available()==0 && frames.outstanding()==3,
          "three independent frames occupy all available permits");
    const auto blocked=frames.begin(Project::xr_gaze);
    check(!blocked && blocked.error()==Error::no_free_slot,
          "the fourth frame cannot rewrite in-flight upload storage");

    const auto splats=rgba_fixture(19,1);
    const auto kmesh=rgba_fixture(15,2);
    const auto colliders=rgba_fixture(12,3);
    const auto g1=frames.stage(*gaussian,splats,16);
    const auto g2=frames.stage(*gaussian,kmesh,32);
    const auto k1=frames.stage(*kairo,kmesh,64);
    const auto c1=frames.stage(*pairs,colliders,32);
    check(g1 && g1->offset==0 && g1->length==19 &&
          g2 && g2->offset==32 && g2->length==15 &&
          k1 && k1->offset==0 && c1 && c1->offset==0,
          "per-frame monotonic aligned upload arenas have independent cursors");
    const auto copied=frames.snapshot_staged(*gaussian,*g1);
    check(copied && *copied==splats,
          "strongly owned upload bytes preserve supplied fixture");
    check(frames.seal(*gaussian) && frames.seal(*kairo) &&
          frames.seal(*pairs),
          "nonempty encoder state transitions to executable");
    check(frames.commit(*gaussian) && frames.commit(*kairo) &&
          frames.commit(*pairs),
          "three separate simulated commands submitted");
    const auto premature=frames.stage(*gaussian,splats,16);
    check(!premature &&
          premature.error()==Error::mutating_submitted_bytes,
          "CPU cannot alter published upload bytes during GPU use");
    check(frames.available()==0,"permit not returned merely by commit");

    // Error is terminal, but not a successful rendering outcome.
    const auto failed=frames.observe_terminal(
        *kairo,Terminal::error,m::RawTiming{100.0,100.002});
    check(failed && frames.available()==1,
          "terminal Metal error must release a permit, not leak all frames");
    const auto reused=frames.begin(Project::xr_gaze);
    check(reused && reused->slot==kairo->slot &&
          reused->generation==kairo->generation+1,
          "completed error frees the slot with a new generation");
    check(!frames.seal(*kairo) &&
          !frames.snapshot_staged(*kairo,*k1),
          "old frame handles immediately stale following release");
    const auto old=frames.stage(*kairo,kmesh,8);
    check(!old && old.error()==Error::stale_generation,
          "stale frame cannot overwrite the next project's arena");
    auto results=frames.drain_events();
    check(results.size()==1 &&
          results[0].frame==*kairo &&
          results[0].status==Terminal::error &&
          !results[0].measured_gpu_milliseconds &&
          !results[0].capture,
          "error event has no success pixels and no trustworthy GPU time");

    const auto gaze=rgba_fixture(11,7);
    check(frames.stage(*reused,gaze,8) &&
          frames.seal(*reused) && frames.commit(*reused),
          "recycled slot works for an entirely different XR project");
    // Deliberately complete out of submission order. GPU callbacks can
    // arrive at different times and are not tied to frame slot order.
    check(frames.observe_terminal(
        *pairs,Terminal::completed,std::nullopt),
        "Kyoto pair result completes first without invented timestamp");
    check(frames.observe_terminal(
        *gaussian,Terminal::completed,m::RawTiming{10.0,10.0015}),
        "Gaussian stage completes with test-provided timing");
    check(frames.observe_terminal(
        *reused,Terminal::completed,m::RawTiming{0.0,0.0}),
        "XR completes even when timing hardware reports no useful values");
    results=frames.drain_events();
    check(results.size()==3 &&
          results[0].project==Project::kyoto_collision_pairs &&
          results[1].project==Project::maveb_gaussian &&
          results[2].project==Project::xr_gaze &&
          results[0].status==Terminal::completed &&
          !results[0].measured_gpu_milliseconds &&
          results[1].measured_gpu_milliseconds.has_value() &&
          std::fabs(*results[1].measured_gpu_milliseconds-1.5)<1e-9 &&
          !results[2].measured_gpu_milliseconds,
          "completion order and missing/valid timing labels retained");
    check(frames.available()==3 && frames.outstanding()==0 &&
          frames.drain_events().empty(),
          "all three permits restored, at-most-once owned event drain");
}

void test_capture_pitch_crop_and_failure_cases() {
    MetalFrameModel frames(1002,1,512);
    auto frame=frames.begin(Project::maveb_gaussian);
    const std::array<m::Byte,4> pixel{4,5,6,255};
    check(frame && frames.stage(*frame,pixel,16) &&
          frames.seal(*frame) && frames.commit(*frame),
          "realistic Metal command assumed submitted in CPU model");

    // Drawable is 3x2 BGRA8. Blit uses 256-byte padded rows, but the
    // captured image must be exactly 3*2*4=24 bytes without padding.
    std::vector<m::Byte> pitched(512,0xEE);
    for(std::size_t i=0;i<12;++i) {
        pitched[i]=static_cast<m::Byte>(10+i);
        pitched[256+i]=static_cast<m::Byte>(50+i);
    }
    m::Readback source{3,2,256,pitched};
    const auto compact=m::compact_bgra8(source);
    check(compact && compact->width==3 && compact->height==2 &&
          compact->bgra8_tightly_packed.size()==24 &&
          compact->bgra8_tightly_packed[0]==10 &&
          compact->bgra8_tightly_packed[11]==21 &&
          compact->bgra8_tightly_packed[12]==50 &&
          compact->bgra8_tightly_packed[23]==61,
          "remove row pitch without changing BGRA pixel byte order");

    const auto malformed=frames.observe_terminal(
        *frame,Terminal::completed,std::nullopt,
        m::Readback{3,2,12,pitched});
    check(!malformed &&
          malformed.error()==Error::malformed_capture &&
          frames.available()==0 &&
          frames.state(0)==Phase::submitted,
          "invalid capture metadata cannot publish a successful result");
    const auto shortSource=frames.observe_terminal(
        *frame,Terminal::completed,std::nullopt,
        m::Readback{3,2,256,
                    std::span<const m::Byte>(pitched.data(),256)});
    check(!shortSource && shortSource.error()==Error::malformed_capture,
          "readback cannot overrun incomplete mapped bytes");
    check(frames.observe_terminal(
        *frame,Terminal::completed,m::RawTiming{22.0,22.004},source),
        "valid terminal completion includes checked readback");
    const auto events=frames.drain_events();
    check(events.size()==1 && events[0].capture &&
          events[0].capture->bgra8_tightly_packed==
              compact->bgra8_tightly_packed &&
          events[0].measured_gpu_milliseconds &&
          std::fabs(*events[0].measured_gpu_milliseconds-4.0)<1e-9,
          "only completed command publishes pixels plus valid GPU timestamp");
    pitched.assign(pitched.size(),0);
    check(events[0].capture->bgra8_tightly_packed[23]==61,
          "captured image has independent ownership after temporary buffer ends");
    const auto doubleTerminal=frames.observe_terminal(
        *frame,Terminal::completed);
    check(!doubleTerminal &&
          doubleTerminal.error()==Error::stale_generation,
          "duplicate completed handler cannot release same permit twice");

    check(!m::compact_bgra8({0,2,256,pitched}) &&
          !m::compact_bgra8({2,0,256,pitched}) &&
          !m::compact_bgra8({3,2,512,pitched}),
          "reject empty frame or source too short for specified pitch");
}
void test_arena_overflow_and_abort() {
    MetalFrameModel frames(1003,1,64);
    auto id=frames.begin(Project::kairo_scene);
    check(id.has_value(),"frame acquired");
    const auto staged=frames.stage(*id,rgba_fixture(8),16);
    check(staged && staged->offset==0,"base payload fits");
    const auto before=frames.snapshot_staged(*id,*staged);
    check(before && *before==rgba_fixture(8),
          "original payload remains stable before replacement attempt");
    const auto badAlign=frames.stage(*id,rgba_fixture(1),3);
    check(!badAlign && badAlign.error()==Error::invalid_alignment,
          "non-power-of-two alignment rejected before cursor change");
    const auto tooLarge=frames.stage(*id,rgba_fixture(65),8);
    check(!tooLarge && tooLarge.error()==Error::arena_full,
          "capacity refusal before copying any source bytes");
    check(frames.snapshot_staged(*id,*staged)==before,
          "rejected writes do not corrupt earlier staged data");
    check(!frames.stage(*id,std::span<const m::Byte>{},8) &&
          frames.state(0)==Phase::recording,
          "zero-length allocation refused while keeping recording state");
    const auto corner=m::checked_align_up(
        std::numeric_limits<std::size_t>::max(),8);
    check(!corner && corner.error()==Error::arithmetic_overflow,
          "arithmetic overflow caught before size_t wrap");
    check(!m::checked_align_up(4,0) &&
          !m::checked_align_up(4,6) &&
          m::checked_align_up(33,32)==64,
          "alignment precondition and arithmetic checked");
    check(frames.fail_before_commit(*id) &&
          frames.available()==1,
          "definite pre-submit failure may cancel and restore permit");
    const auto next=frames.begin(Project::maveb_gaussian);
    check(next && next->generation==id->generation+1 &&
          frames.stage(*next,rgba_fixture(64),1),
          "arena reset only after definite cancellation");
    check(frames.seal(*next) && frames.commit(*next),
          "full-capacity payload can be published");
    check(!frames.fail_before_commit(*next),
          "committed frame may not be cancelled as if never submitted");
    check(frames.observe_terminal(*next,Terminal::error) &&
          frames.available()==1,
          "terminal error recovers slot without fake success result");
}
void test_unknown_acceptance_and_device_loss() {
    MetalFrameModel frames(1004,2,128);
    auto a=frames.begin(Project::xr_gaze);
    auto b=frames.begin(Project::maveb_gaussian);
    check(a && b && frames.stage(*a,rgba_fixture(5),16) &&
          frames.seal(*a) && frames.uncertain_commit(*a),
          "ambiguous commit must quarantine its upload storage");
    check(!frames.observe_terminal(*a,Terminal::completed) &&
          !frames.fail_before_commit(*a),
          "uncertain outcome cannot masquerade as completed or unsubmitted");
    check(frames.stage(*b,rgba_fixture(12),16) &&
          frames.seal(*b) && frames.commit(*b),
          "other frame can proceed independently");
    check(frames.observe_terminal(*b,Terminal::error),
          "terminal command error releases only its own frame permit");
    check(frames.available()==1 &&
          frames.outstanding()==1,
          "quarantined frame remains outstanding");
    check(frames.confirm_accepted(*a) &&
          frames.observe_terminal(*a,Terminal::completed),
          "only independent acceptance evidence can resolve quarantine");
    check(frames.available()==2,
          "all permits returned when terminal evidence arrives");

    MetalFrameModel lost(1005,1,32);
    auto id=lost.begin(Project::kyoto_collision_pairs);
    check(id && lost.stage(*id,rgba_fixture(4),4) &&
          lost.seal(*id) && lost.commit(*id),
          "separate GPU command pending before device loss");
    lost.device_lost();
    check(lost.lost() &&
          !lost.observe_terminal(*id,Terminal::completed) &&
          !lost.begin(Project::kairo_scene) &&
          !lost.fail_before_commit(*id) &&
          lost.outstanding()==1,
          "device loss must not synthesize a successful native completion");
}
void test_timestamp_provenance_and_wrap() {
    check(!m::measured_gpu_milliseconds(std::nullopt) &&
          !m::measured_gpu_milliseconds(m::RawTiming{0,0}) &&
          !m::measured_gpu_milliseconds(m::RawTiming{4,4}) &&
          !m::measured_gpu_milliseconds(m::RawTiming{4,3}),
          "missing, zero, regressed GPU timestamp is unavailable");
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    const auto inf=std::numeric_limits<double>::infinity();
    check(!m::measured_gpu_milliseconds(m::RawTiming{nan,4}) &&
          !m::measured_gpu_milliseconds(m::RawTiming{1,inf}),
          "nonfinite native timestamps cannot become throughput numbers");
    check(m::measured_gpu_milliseconds(m::RawTiming{1,1.002}) &&
          std::fabs(*m::measured_gpu_milliseconds(
              m::RawTiming{1,1.002})-2.0)<1e-9,
          "a real positive interval is measured in milliseconds");

    MetalFrameModel nearWrap(
        1006,1,8,std::numeric_limits<std::uint64_t>::max());
    auto final=nearWrap.begin(Project::kairo_scene);
    check(final && nearWrap.stage(*final,rgba_fixture(1),1) &&
          nearWrap.seal(*final) && nearWrap.commit(*final),
          "max-generation frame can still finish once");
    check(nearWrap.observe_terminal(*final,Terminal::completed) &&
          nearWrap.state(0)==Phase::exhausted &&
          nearWrap.available()==0 &&
          !nearWrap.begin(Project::kairo_scene),
          "generation exhaustion permanently retires slot; never wraps ABA");

    MetalFrameModel other(2000,1,32);
    auto foreign=other.begin(Project::maveb_gaussian);
    MetalFrameModel home(3000,1,32);
    check(foreign && !home.seal(*foreign) &&
          !home.stage(*foreign,rgba_fixture(4),4),
          "foreign frame owner cannot mutate another renderer's ring");
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc==2 &&
           (std::string_view(argv[1])=="--probe-native" ||
            std::string_view(argv[1])=="--benchmark-native"))
            return allmight::gpu_course::run_native_metal_probe(
                std::string_view(argv[1])=="--benchmark-native");
        if(argc!=1) {
            std::cerr<<"usage: gpu_metal_frame_cpp23_lab "
                "[--probe-native|--benchmark-native]\n";
            return 2;
        }
        test_three_permits_maveb_kairo_and_collision();
        test_capture_pitch_crop_and_failure_cases();
        test_arena_overflow_and_abort();
        test_unknown_acceptance_and_device_loss();
        test_timestamp_provenance_and_wrap();
        std::cout<<"GPU-15M: Maveb/Kairo/physics/XR frame permits, Metal error "
                   "status, checked upload arena, BGRA8 row pitch, distinct "
                   "GPU timestamps, source provenance and device loss passed "
                   "(synthetic CPU correctness model ONLY)\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"GPU-15M FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
