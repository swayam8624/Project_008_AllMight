// AllMight Book01 Pass13: CROSS-PROJECT GPU capability/resource contracts.
//
// This is intentionally NOT another NES-only lab. The same host-side
// capability logic must handle Kairo graphics/presentation, Maveb-style
// Gaussian raster+compute buffers, Kyoto collision pairs, XR gaze/render,
// and the NES debugger overlay. Fake snapshots are labeled test fixtures.
//
// Default CTest: portable CPU correctness cases, requiring no Vulkan SDK.
// --probe-native: optional real Vulkan probe from native_vulkan_probe.cpp.
// Exit 77 means hardware/SDK unavailable, NOT "GPU test passed".
#include "gpu_course/device_resource_contracts.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace allmight::gpu_course {
int run_native_probe();
}

namespace {
namespace g=allmight::gpu_course;

void check(bool good,std::string_view why) {
    if(!good) throw std::runtime_error(std::string(why));
}
g::Snapshot unified_memory_fixture() {
    // A synthetic UMA device that advertises independently COMBINED flags.
    // These values are not copied from Apple silicon or any actual driver.
    g::Snapshot snap;
    snap.name="Synthetic UMA";
    snap.api_major=1;snap.api_minor=3;
    snap.families={
        {g::queue_graphics|g::queue_compute|g::queue_transfer,1,true}
    };
    snap.heaps={
        {256ULL*1024ULL*1024ULL,128ULL*1024ULL*1024ULL}
    };
    snap.types={
        {g::mem_device_local|g::mem_host_visible|g::mem_host_coherent,0},
        {g::mem_host_visible|g::mem_host_cached,0},
        {g::mem_device_local,0}
    };
    snap.swapchain_extension=true;
    return snap;
}
g::Snapshot split_discrete_fixture() {
    // Synthetic discrete design with separate flags/queue capabilities;
    // not a measured physical device configuration.
    g::Snapshot snap;
    snap.name="Synthetic discrete";
    snap.api_major=1;snap.api_minor=2;
    snap.families={
        {g::queue_graphics|g::queue_compute|g::queue_transfer,1,true},
        {g::queue_transfer,1,false},
        {g::queue_compute|g::queue_transfer,1,false}
    };
    snap.heaps={{512ULL*1024ULL*1024ULL,std::nullopt},
                {128ULL*1024ULL*1024ULL,64ULL*1024ULL*1024ULL}};
    snap.types={
        {g::mem_device_local,0},
        {g::mem_host_visible|g::mem_host_coherent,1},
        {g::mem_host_visible|g::mem_host_coherent|
         g::mem_host_cached,1}
    };
    snap.swapchain_extension=true;
    return snap;
}
void test_cross_project_workload_requirements() {
    const auto uma=unified_memory_fixture();
    const auto discrete=split_discrete_fixture();
    const auto cases=g::teaching_workloads();
    check(cases.size()==5,"five distinct cross-project use cases");
    std::size_t accepted=0;
    for(const auto& workload:cases) {
        const auto required=g::needs(workload.workload);
        const auto queues=g::select_queues(uma,required);
        const auto mem=g::choose_memory(uma,workload.memory);
        check(queues.has_value() && mem.has_value(),
              "UMA fixture supports stated example workload requirements");
        check(mem->memory_type_index==0 &&
              mem->heap_index==0 && mem->known_budget,
              "combined device-local and host-visible memory is legal");
        check(g::has(uma.types[mem->memory_type_index].flags,
                     workload.memory.required),
              "selected type satisfies all REQUIRED flags");
        if(required.presentation)
            check(queues->present && queues->graphics,
                  "present support queried for actual modeled surface");
        if(required.compute)
            check(queues->compute.has_value(),
                  "compute workload requires a capable queue family");
        ++accepted;
    }
    check(accepted==5,"workload regression inventory fully traversed");

    // One device may have several physically distinct memory types:
    // choose device-local for compute storage, coherent host-visible
    // memory for a CPU upload. Never mandate mutually exclusive flags.
    const auto kairo=g::choose_memory(discrete,
        {65536,256,0b111,g::mem_device_local,g::mem_host_visible});
    const auto overlay=g::choose_memory(discrete,
        {256,256,0b111,
         g::mem_host_visible|g::mem_host_coherent,g::mem_host_cached});
    check(kairo && kairo->memory_type_index==0 &&
          overlay && overlay->memory_type_index==2,
          "discrete fixture chooses different valid type for different roles");
    const auto compute=g::select_queues(discrete,
        g::needs(g::Workload::kyoto_collision_pairs_compute));
    check(compute && compute->compute.has_value() &&
          compute->transfer.has_value() &&
          !compute->graphics && !compute->present,
          "collision-pair compute does not require a graphics window");
    const auto XR=g::select_queues(discrete,g::needs(g::Workload::xr_gaze_and_render));
    check(XR && XR->graphics && XR->compute && XR->transfer && XR->present,
          "XR rendering fixture needs graphics+compute+present");
}
void test_queue_capability_fail_closed() {
    const auto all=unified_memory_fixture();
    auto noQueues=all;
    noQueues.families.clear();
    const auto empty=g::select_queues(noQueues,
        g::needs(g::Workload::kairo_presented_scene));
    check(!empty && empty.error()==g::CapabilityError::empty_device,
          "missing physical queues never treated as graphics capable");

    auto computeOnly=all;
    computeOnly.families={{g::queue_compute|g::queue_transfer,1,std::nullopt}};
    const auto accepts=g::select_queues(computeOnly,
        g::needs(g::Workload::kyoto_collision_pairs_compute));
    check(accepts && accepts->compute && accepts->transfer,
          "headless collision kernel can run on compute-capable family");
    const auto graphics=g::select_queues(computeOnly,
        g::needs(g::Workload::kairo_presented_scene));
    check(!graphics &&
          graphics.error()==g::CapabilityError::missing_graphics,
          "headless compute does not invent graphics capability");

    auto zeroCount=all;
    zeroCount.families[0].count=0;
    const auto noGraphics=g::select_queues(zeroCount,
        g::needs(g::Workload::kairo_presented_scene));
    check(!noGraphics &&
          noGraphics.error()==g::CapabilityError::missing_graphics,
          "zero queues in family is not a selectable queue");

    auto unknownPresent=all;
    unknownPresent.families[0].present=std::nullopt;
    const auto pUnknown=g::select_queues(unknownPresent,
        g::needs(g::Workload::nes_overlay));
    check(!pUnknown &&
          pUnknown.error()==g::CapabilityError::unknown_surface_support,
          "presentation requires an actual surface-dependent query");
    const auto headless=g::select_queues(unknownPresent,
        g::needs(g::Workload::headless_compute));
    check(headless && headless->compute && !headless->present,
          "lack of a surface does not preclude headless compute");

    auto noPresent=all;
    noPresent.families[0].present=false;
    const auto pBad=g::select_queues(noPresent,
        g::needs(g::Workload::nes_overlay));
    check(!pBad &&
          pBad.error()==g::CapabilityError::missing_presentation,
          "known unsupported surface cannot be used");
    noPresent.swapchain_extension=false;
    const auto noSwapchain=g::select_queues(noPresent,
        g::needs(g::Workload::nes_overlay));
    check(!noSwapchain &&
          noSwapchain.error()==g::CapabilityError::missing_swapchain_extension,
          "surface support cannot substitute for missing swapchain extension");

    auto alternate=all;
    alternate.families={
        {g::queue_graphics|g::queue_transfer,1,false},
        {g::queue_compute|g::queue_transfer,1,false},
        {g::queue_transfer,1,true}
    };
    const auto selected=g::select_queues(alternate,
        g::needs(g::Workload::xr_gaze_and_render));
    check(selected && *selected->graphics==0 && *selected->compute==1 &&
          *selected->transfer==0 && *selected->present==2,
          "separate graphics, compute and present family selection is exact");
    // No claim made here that queue-family transfers or concurrent GPU
    // execution have actually been scheduled or synchronized.
}
void test_memory_type_masks_budgets_alignment_and_bounds() {
    const auto uma=unified_memory_fixture();
    check(g::power_of_two(256) && !g::power_of_two(0) &&
          !g::power_of_two(3),"power-of-two alignment validation");
    const auto neither=g::choose_memory(uma,
        {256,256,0,g::mem_host_visible,0});
    check(!neither &&
          neither.error()==g::CapabilityError::invalid_memory_request,
          "zero memory-type mask cannot select arbitrary memory");
    const auto zeroBytes=g::choose_memory(uma,
        {0,256,0b111,g::mem_host_visible,0});
    check(!zeroBytes &&
          zeroBytes.error()==g::CapabilityError::invalid_memory_request,
          "allocation requirement bytes cannot be zero");
    const auto badAlignment=g::choose_memory(uma,
        {1024,3,0b111,g::mem_host_visible,0});
    check(!badAlignment &&
          badAlignment.error()==g::CapabilityError::invalid_memory_request,
          "invalid Vulkan-style requirement alignment rejected");
    const auto impossible=g::choose_memory(uma,
        {4096,256,0b100,g::mem_host_visible|g::mem_host_coherent,0});
    check(!impossible &&
          impossible.error()==g::CapabilityError::missing_memory_type,
          "memoryTypeBits limits eligibility even if another heap has flags");

    // Existing heap size and optional reported budget are different.
    // Budget is an estimate at query time, NOT a reservation/guarantee.
    auto tight=uma;
    tight.heaps[0].reported_budget=1024;
    const auto denied=g::choose_memory(tight,
        {2048,256,0b111,g::mem_device_local,0});
    check(!denied &&
          denied.error()==g::CapabilityError::memory_budget_too_small,
          "known budget smaller than request is a conservative refusal");
    tight.heaps[0].reported_budget=std::nullopt;
    const auto noKnownBudget=g::choose_memory(tight,
        {2048,256,0b111,g::mem_device_local,0});
    check(noKnownBudget && !noKnownBudget->known_budget,
          "unknown budget is not zero budget and never a guaranteed allocation");

    auto invalidHeap=uma;
    invalidHeap.types[0].heap_index=99;
    const auto broken=g::choose_memory(invalidHeap,
        {256,256,0b001,g::mem_device_local,0});
    check(!broken &&
          broken.error()==g::CapabilityError::invalid_heap_index,
          "malformed memory-type heap index refused");
    auto large=uma;
    large.types.resize(33);
    const auto impossibleMask=g::choose_memory(large,
        {256,256,0b1,g::mem_device_local,0});
    check(!impossibleMask &&
          impossibleMask.error()==g::CapabilityError::invalid_memory_request,
          "memory bit masks are exactly 32-bit, no undefined shift");
    const auto tooLarge=g::choose_memory(uma,
        {std::numeric_limits<std::uint64_t>::max(),256,
         0b111,g::mem_device_local,0});
    check(!tooLarge &&
          tooLarge.error()==g::CapabilityError::memory_budget_too_small,
          "oversized requirement fails without overflow or allocation");

    // Index 31 must be legal and shift-safe; 32+ must be excluded.
    auto topBit=uma;
    topBit.types.resize(32);
    topBit.types[31]={g::mem_device_local,0};
    const auto selected=g::choose_memory(topBit,
        {128,64,0x80000000U,g::mem_device_local,0});
    check(selected && selected->memory_type_index==31,
          "high legal memoryTypeBits bit index handled correctly");
}
void test_no_empty_optional_or_fabricated_hardware() {
    // Optional real Vulkan cannot be run as a fake/pretend green CTest.
    // Exercise the exact API-only model, and inspect output of --probe-native
    // separately if a real Vulkan loader and compatible device exist.
    const g::Snapshot empty{};
    const auto no=g::select_queues(empty,{true,false,false,false});
    check(!no && no.error()==g::CapabilityError::empty_device,
          "default-initialized capability snapshot is not a device");
    const auto mem=g::choose_memory(empty,
        {256,256,0b1,g::mem_device_local,0});
    check(!mem && mem.error()==g::CapabilityError::missing_memory_type,
          "absent Vulkan memory-type query cannot be fabricated");
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc==2 && std::string_view(argv[1])=="--probe-native")
            return allmight::gpu_course::run_native_probe();
        if(argc!=1) {
            std::cerr<<"usage: gpu_device_resource_cpp23_lab [--probe-native]\n";
            return 2;
        }
        test_cross_project_workload_requirements();
        test_queue_capability_fail_closed();
        test_memory_type_masks_budgets_alignment_and_bounds();
        test_no_empty_optional_or_fabricated_hardware();
        std::cout<<"GPU-13D: multi-project queue selection, unknown presentation, "
                   "memory-type eligibility and bounds, UMA/discrete fixtures "
                   "and conservative budget policies passed (CPU model only)\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"GPU-13D FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
