// Project AllMight / GPU-16P — CPU-only execution + provenance oracle.
// Three distinct clients: Maveb multi-frame renderer, Kairo Objective-C++
// boundary, Kyoto collision pair work, XR gaze/render latency measurements.
// No actual Metal/Vulkan graphics call is made in this CTest by default.
//
// --probe-native invokes optional real Metal callback/compute/readback code.
// The default tests use synthetic observations, never "GPU benchmark" data.
#include "gpu_course/gpu_profile_ownership.hpp"
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace allmight::gpu_course {
int run_native_gpu_profile_callback_probe();
}
namespace {
namespace p=allmight::gpu_course::profile;
void check(bool condition,std::string_view message) {
    if(!condition) throw std::runtime_error(std::string(message));
}
p::NativeObservation observed(bool success,
                               bool checked=true,bool correct=true,
                               double start=10.0,double end=10.004) {
    return p::NativeObservation{
        success?p::Terminal::completed:p::Terminal::failed,
        start,end,checked,correct,p::Origin::synthetic_fixture};
}
p::Configuration fixture_configuration() {
    return {
        p::Project::maveb, "Synthetic-GPU-NOT-REAL",
        "Synthetic SDK", "synthetic-commit", "synthetic-MSL-hash",
        "synthetic-workload-hash", "synthetic-counter-tool",
        1920,1080,false
    };
}
void test_three_concurrent_completion_callbacks() {
    p::CompletionMailbox mailbox(3,9876);
    const auto maveb=mailbox.reserve(p::Project::maveb);
    const auto kairo=mailbox.reserve(p::Project::kairo);
    const auto kyoto=mailbox.reserve(p::Project::kyoto);
    check(maveb&&kairo&&kyoto && mailbox.available()==0 &&
          mailbox.outstanding()==3,
          "three renderer permits, no fictional fourth slot");
    check(!mailbox.reserve(p::Project::xr) &&
          !mailbox.reserve(p::Project::nes),
          "backpressure protects submitted frame arenas");

    check(mailbox.publish(*maveb) && mailbox.publish(*kairo) &&
          mailbox.publish(*kyoto),
          "recording slots become submitted only after publication");
    const auto mm=mailbox.callback_for(*maveb);
    const auto kk=mailbox.callback_for(*kairo);
    const auto cc=mailbox.callback_for(*kyoto);

    std::atomic<int> successfulCallbacks{};
    // These are actual HOST threads, testing mutex/ownership and races,
    // not GPU concurrent encoders. Thread completion order is immaterial.
    {
        std::jthread t1([&] {
            if(mm(observed(true))) successfulCallbacks.fetch_add(1);
        });
        std::jthread t2([&] {
            if(kk(observed(false))) successfulCallbacks.fetch_add(1);
        });
        std::jthread t3([&] {
            if(cc(observed(true,true,false))) successfulCallbacks.fetch_add(1);
        });
    }
    check(successfulCallbacks.load()==3 &&
          mailbox.available()==3 && mailbox.outstanding()==0,
          "all three terminal events safely return one permit each");
    const auto duplicate=kk(observed(true));
    check(!duplicate &&
          duplicate.error()==p::Error::stale_generation &&
          mailbox.available()==3,
          "duplicate callback cannot release an extra permit");
    const auto events=mailbox.drain();
    check(events.size()==3 && mailbox.drain().empty(),
          "owned completed-event drain is at most once");
    std::size_t passed{},failed{},incorrect{};
    for(const auto& event:events) {
        check(event.origin==p::Origin::synthetic_fixture,
              "host fixtures are never labeled native GPU observations");
        if(event.terminal==p::Terminal::failed) {
            ++failed;
            check(!event.gpu_milliseconds && !event.correctness_pass,
                  "error terminal cannot publish successful GPU timing");
        } else if(event.correctness_pass) {
            ++passed;
            check(event.gpu_milliseconds &&
                  std::fabs(*event.gpu_milliseconds-4.0)<1e-9,
                  "valid synthetic 4ms interval labeled as a fixture");
        } else {
            ++incorrect;
            check(!event.correctness_pass,
                  "GPU terminal completion does not imply correct pixels");
        }
    }
    check(passed==1&&failed==1&&incorrect==1,
          "terminal status and independent correctness form separate axes");
    const auto reused=mailbox.reserve(p::Project::xr);
    check(reused && reused->generation!=maveb->generation,
          "fresh owner/slot generation issued after callback retirement");
    check(!mailbox.publish(*maveb) &&
          !mailbox.callback_for(*maveb)(observed(true)),
          "stale identity rejected after the slot gets a new occupant");
    check(mailbox.abandon_unsubmitted(*reused),
          "known unsubmitted XR work can release its permit");
}
void test_callback_outlives_renderer_owner_safely() {
    // CompletionCallback owns State but NEVER has a pointer to Renderer/UI.
    // Discarding the renderer wrapper must not UAF inside an eventual
    // completion callback or keep a SwiftUI-like owner unexpectedly alive.
    struct UiOwner {int value{19};};
    auto owner=std::make_shared<UiOwner>();
    std::weak_ptr<UiOwner> weakOwner=owner;
    p::CompletionMailbox::TerminalCallback callback;
    p::FrameId id{};
    {
        auto mailbox=std::make_unique<p::CompletionMailbox>(1,4567);
        const auto reserved=mailbox->reserve(p::Project::maveb);
        check(reserved && mailbox->publish(*reserved),
              "native-style completion pending before UI destruction");
        id=*reserved;
        callback=mailbox->callback_for(id);
        // Real native callbacks should capture shared internal state, not
        // raw Renderer*; a weak UI owner can be checked only on UI thread.
        owner.reset();
        mailbox.reset();
    }
    check(weakOwner.expired(),
          "callback did not retain UI owner / renderer object");
    const auto done=callback(observed(true));
    check(done,
          "callback safely finishes after original wrapper destruction");
    const auto twice=callback(observed(true));
    check(!twice && twice.error()==p::Error::stale_generation,
          "post-destruction duplicate callback rejected");
    p::CompletionMailbox other(1,5678);
    const auto q=other.reserve(p::Project::kairo);
    check(q && !other.publish(id) &&
          other.publish(*q) &&
          other.abandon_unsubmitted(*q)==std::unexpected(p::Error::wrong_phase),
          "foreign owner and accepted/queued work fail closed");
}
void test_unknown_acceptance_and_failed_record() {
    p::CompletionMailbox mailbox(1,6001);
    auto a=mailbox.reserve(p::Project::xr);
    check(a && mailbox.quarantine(*a),"submission result unknown");
    check(!mailbox.abandon_unsubmitted(*a) &&
          !mailbox.callback_for(*a)(observed(true)) &&
          mailbox.available()==0 && mailbox.outstanding()==1,
          "unknown commit retains resources and frame permit");
    check(mailbox.confirm_published(*a) &&
          mailbox.callback_for(*a)(observed(true,false,false)) &&
          mailbox.available()==1,
          "only independent acceptance and terminal observation release");
    auto b=mailbox.reserve(p::Project::kyoto);
    check(b && mailbox.abandon_unsubmitted(*b) &&
          mailbox.available()==1,
          "known abandoned encoding releases permit without fake event");
    check(!mailbox.callback_for(*b)(observed(true)),
          "definitely unsubmitted work cannot receive successful GPU result");
}
void test_counter_and_profile_provenance_gate() {
    auto config=fixture_configuration();
    std::vector<p::FrameSample> samples;
    for(unsigned i=0;i<30;++i) {
        p::FrameSample x;
        x.config=config;
        x.origin=p::Origin::synthetic_fixture;
        x.terminal=p::Terminal::completed;
        x.pixels_or_pairs_checked=true;
        x.output_correct=true;
        x.cpu_encode_ms=0.8 + static_cast<double>(i)*0.002;
        x.gpu_command_ms=1.1 + static_cast<double>(i)*0.003;
        // Unsupported device counter is absent, NOT an invented 0.
        x.counters.push_back({"tile-memory-transactions",
                               "transactions",std::nullopt,false});
        samples.push_back(x);
    }
    const auto synthetic=p::summarize_native_verified(samples);
    check(!synthetic && synthetic.error()==p::Error::wrong_origin,
          "synthetic samples cannot earn an actual GPU report");

    // Converting this TEST FIXTURE to an Origin enum exercises the gate.
    // It does NOT constitute a real Metal run, hardware device identity
    // or independently verified GPU measurements.
    for(auto& item:samples)
        item.origin=p::Origin::native_metal_observation;
    const auto modelSummary=p::summarize_native_verified(samples);
    check(modelSummary && modelSummary->sample_count==30 &&
          modelSummary->cpu_p95_ms>=modelSummary->cpu_median_ms &&
          modelSummary->gpu_p95_ms>=modelSummary->gpu_median_ms,
          "policy accepts internally consistent native-shaped fixture");
    samples[10].config.shader_digest="different shader binary";
    const auto mixed=p::summarize_native_verified(samples);
    check(!mixed && mixed.error()==p::Error::mixed_environment,
          "mixed shader builds may not be aggregated as one benchmark");
    samples[10].config=config;
    samples[5].output_correct=false;
    check(!p::summarize_native_verified(samples) &&
          p::summarize_native_verified(samples).error()==
             p::Error::incorrect_output,
          "wrong pixels or collision pairs invalidate entire benchmark");
    samples[5].output_correct=true;
    samples[6].gpu_command_ms=0.0;
    check(!p::summarize_native_verified(samples) &&
          p::summarize_native_verified(samples).error()==
             p::Error::invalid_timing,
          "zero GPU timing is not a 0ms native measurement");
    samples[6].gpu_command_ms=1.2;
    samples[8].counters[0].measured=10.0;
    check(!p::summarize_native_verified(samples) &&
          p::summarize_native_verified(samples).error()==
             p::Error::unsupported_counter,
          "unsupported counter cannot acquire a fictional value");
    samples[8].counters[0].measured.reset();
    samples[0].config.device.clear();
    check(!p::summarize_native_verified(samples) &&
          p::summarize_native_verified(samples).error()==
             p::Error::incomplete_provenance,
          "missing physical device identifier prevents release report");
    samples[0].config=config;
    check(!p::summarize_native_verified(samples,31) &&
          p::summarize_native_verified(samples,31).error()==
             p::Error::insufficient_samples,
          "no insufficient set described as statistically sufficient");
}
void test_missing_timestamps_counter_formats_and_stage_safety() {
    const auto good=observed(true);
    const auto v=p::gpu_interval_ms(good);
    check(v && std::fabs(*v-4.0)<1e-9,
          "positive terminal GPU interval accepted as raw metric");
    auto zero=good;
    zero.gpu_start_seconds=0.0;
    check(!p::gpu_interval_ms(zero),
          "Metal not-ready zero GPUStartTime not a 0ms sample");
    auto reversed=good;
    reversed.gpu_end_seconds=3.0;
    check(!p::gpu_interval_ms(reversed),
          "negative/reversed GPU interval refused");
    auto missing=good;
    missing.gpu_start_seconds.reset();
    check(!p::gpu_interval_ms(missing),
          "missing GPU interval not silently imputed");
    auto failed=good;
    failed.terminal=p::Terminal::failed;
    check(!p::gpu_interval_ms(failed),
          "error command cannot contribute successful GPU elapsed time");

    p::CompletionMailbox m(1,7001);
    auto id=m.reserve(p::Project::kairo);
    check(id,"slot reserved");
    const auto before=m.callback_for(*id)(good);
    check(!before && before.error()==p::Error::wrong_phase,
          "completion callback cannot run before a real commit");
    check(m.publish(*id) &&
          m.callback_for(*id)(zero),
          "terminal event still allows permit release without GPU timestamp");
    const auto results=m.drain();
    check(results.size()==1 && !results.front().gpu_milliseconds,
          "unavailable GPU time is explicit optional, not zero");
}
} // namespace
int main(int argc,char** argv) {
    try {
        if(argc==2 && std::string_view(argv[1])=="--probe-native")
            return allmight::gpu_course::run_native_gpu_profile_callback_probe();
        if(argc!=1) {
            std::cerr<<"Usage: gpu_profile_ownership_cpp23_lab [--probe-native]\n";
            return 2;
        }
        test_three_concurrent_completion_callbacks();
        test_callback_outlives_renderer_owner_safely();
        test_unknown_acceptance_and_failed_record();
        test_counter_and_profile_provenance_gate();
        test_missing_timestamps_counter_formats_and_stage_safety();
        std::cout<<"GPU-16P portable model PASS: real HOST callback threads, "
                   "safe post-renderer events, stale-ID refusal, Metal "
                   "terminal/timing validity and fail-closed provenance "
                   "(NO native GPU run, no real GPU metrics).\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"GPU-16P FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
