#pragma once
// Project AllMight / Book 01 / Pass 13: portable device/resource eligibility.
//
// This module is NOT tied to the NES emulator. It models capability questions
// shared by the Kairo Vulkan renderer, Maveb Gaussian/TSDF systems, collision
// broad phases, XR frame processing and simple emulator overlays.
//
// A Snapshot is a COPY of reported capabilities, not a VkPhysicalDevice, an
// MTLDevice, a live window surface or an allocated resource. The CPU functions
// below cannot prove that vkAllocateMemory / newBuffer will succeed, that a
// surface supports presentation, or that an operation is synchronized.
//
// Bit meanings mirror a small common subset for teaching; do NOT pass these
// enum values directly as Vk*Flags or Metal resource options.
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace allmight::gpu_course {

enum QueueBits : std::uint32_t {
    queue_graphics=1U<<0U,
    queue_compute=1U<<1U,
    queue_transfer=1U<<2U
};
enum MemoryBits : std::uint32_t {
    mem_device_local=1U<<0U,
    mem_host_visible=1U<<1U,
    mem_host_coherent=1U<<2U,
    mem_host_cached=1U<<3U
};
enum class Workload {
    nes_overlay,
    kairo_presented_scene,
    maveb_gaussian_raster_and_compute,
    kyoto_collision_pairs_compute,
    xr_gaze_and_render,
    headless_compute
};
struct QueueFamily {
    std::uint32_t flags{};
    std::uint32_t count{};
    // Must be explicitly queried against THE intended native VkSurfaceKHR.
    // nullopt is UNKNOWN, not false and certainly not true.
    std::optional<bool> present{};
};
struct MemoryHeap {
    std::uint64_t bytes{};
    std::optional<std::uint64_t> reported_budget{};
};
struct MemoryType {
    std::uint32_t flags{};
    std::uint32_t heap_index{};
};
struct Snapshot {
    std::string name;
    std::uint32_t api_major{};
    std::uint32_t api_minor{};
    std::vector<QueueFamily> families;
    std::vector<MemoryHeap> heaps;
    std::vector<MemoryType> types;
    bool swapchain_extension{}; // extension presence is not surface support
};
struct Needs {
    bool graphics{};
    bool compute{};
    bool transfer{};
    bool presentation{};
    // This is deliberately not a device-address, ray-tracing, descriptor
    // indexing, float16, cooperative-matrix or storage-format guarantee.
    // Advanced features require additional queried VkPhysicalDeviceFeatures2
    // chains (and extension/version dependencies) at the backend boundary.
};
[[nodiscard]] constexpr Needs needs(Workload workload) noexcept {
    switch(workload) {
      case Workload::nes_overlay: return {true,false,true,true};
      case Workload::kairo_presented_scene: return {true,false,true,true};
      case Workload::maveb_gaussian_raster_and_compute:
        return {true,true,true,false}; // Vulkan analogue, not Metal device
      case Workload::kyoto_collision_pairs_compute:
        return {false,true,true,false};
      case Workload::xr_gaze_and_render:
        return {true,true,true,true};
      case Workload::headless_compute: return {false,true,false,false};
    }
    return {};
}
enum class CapabilityError {
    empty_device, missing_graphics, missing_compute, missing_transfer,
    unknown_surface_support, missing_presentation,
    missing_swapchain_extension,
    invalid_memory_request, missing_memory_type, invalid_heap_index,
    memory_budget_too_small
};
struct Queues {
    std::optional<std::uint32_t> graphics;
    std::optional<std::uint32_t> compute;
    std::optional<std::uint32_t> transfer;
    std::optional<std::uint32_t> present;
};
[[nodiscard]] constexpr bool has(std::uint32_t present,
                                  std::uint32_t needed) noexcept {
    return (present&needed)==needed;
}
[[nodiscard]] inline std::optional<std::uint32_t>
first_family(const Snapshot& device,std::uint32_t capability) noexcept {
    for(std::uint32_t i=0;i<device.families.size();++i)
        if(device.families[i].count!=0 &&
           has(device.families[i].flags,capability)) return i;
    return std::nullopt;
}
[[nodiscard]] inline std::expected<Queues,CapabilityError>
select_queues(const Snapshot& device,Needs request) noexcept {
    if(device.families.empty())
        return std::unexpected(CapabilityError::empty_device);
    Queues out{};
    if(request.graphics) {
        out.graphics=first_family(device,queue_graphics);
        if(!out.graphics)
            return std::unexpected(CapabilityError::missing_graphics);
    }
    if(request.compute) {
        // Prefer reusing a family that the chosen graphics work also uses
        // when it explicitly advertises compute; never assume the labels
        // alone imply a separate concurrent hardware engine.
        if(out.graphics &&
           has(device.families[*out.graphics].flags,queue_compute))
            out.compute=out.graphics;
        else out.compute=first_family(device,queue_compute);
        if(!out.compute)
            return std::unexpected(CapabilityError::missing_compute);
    }
    if(request.transfer) {
        // Reuse a selected capable queue family before selecting a
        // separate one: fewer ownership transfers for the basic lesson.
        if(out.graphics &&
           has(device.families[*out.graphics].flags,queue_transfer))
            out.transfer=out.graphics;
        else if(out.compute &&
                has(device.families[*out.compute].flags,queue_transfer))
            out.transfer=out.compute;
        else out.transfer=first_family(device,queue_transfer);
        if(!out.transfer)
            return std::unexpected(CapabilityError::missing_transfer);
    }
    if(request.presentation) {
        if(!device.swapchain_extension)
            return std::unexpected(CapabilityError::missing_swapchain_extension);
        // Present query is meaningful only for an actual VkSurfaceKHR.
        if(out.graphics && device.families[*out.graphics].present==true)
            out.present=out.graphics;
        else {
            for(std::uint32_t i=0;i<device.families.size();++i)
                if(device.families[i].count>0 &&
                   device.families[i].present==true) {
                    out.present=i;
                    break;
                }
        }
        if(!out.present) {
            for(const auto& family:device.families)
                if(family.count>0 && !family.present.has_value())
                    return std::unexpected(
                        CapabilityError::unknown_surface_support);
            return std::unexpected(CapabilityError::missing_presentation);
        }
    }
    return out;
}
struct MemoryRequest {
    // The SIZE and MEMORY TYPE BITS from vkGet*MemoryRequirements, NOT
    // merely bytes in the source asset or a guess at texture dimensions.
    std::uint64_t requirement_size{};
    std::uint64_t alignment{};
    std::uint32_t memory_type_bits{};
    std::uint32_t required{};
    std::uint32_t preferred{};
};
struct MemoryChoice {
    std::uint32_t memory_type_index{};
    std::uint32_t heap_index{};
    std::uint32_t satisfied_preference_bits{};
    bool known_budget{};
};
[[nodiscard]] constexpr bool power_of_two(std::uint64_t n) noexcept {
    return n!=0U && (n&(n-1U))==0U;
}
[[nodiscard]] inline std::expected<MemoryChoice,CapabilityError>
choose_memory(const Snapshot& device,const MemoryRequest& r) noexcept {
    if(r.requirement_size==0 || !power_of_two(r.alignment) ||
       r.memory_type_bits==0 || device.types.size()>32)
        return std::unexpected(CapabilityError::invalid_memory_request);
    std::optional<MemoryChoice> best;
    bool found_flag_eligible{};
    bool invalid_heap{};
    bool insufficient_size{};
    for(std::size_t i=0;i<device.types.size();++i) {
        if((r.memory_type_bits&(1U<<static_cast<unsigned>(i)))==0)
            continue;
        const auto& type=device.types[i];
        if(!has(type.flags,r.required)) continue;
        found_flag_eligible=true;
        if(type.heap_index>=device.heaps.size()) {
            invalid_heap=true;
            continue;
        }
        const auto& heap=device.heaps[type.heap_index];
        if(r.requirement_size>heap.bytes ||
           (heap.reported_budget &&
            r.requirement_size>*heap.reported_budget)) {
            insufficient_size=true;
            continue;
        }
        const auto prefs=std::popcount(type.flags&r.preferred);
        if(!best ||
           prefs>std::popcount(best->satisfied_preference_bits)) {
            best=MemoryChoice{
                static_cast<std::uint32_t>(i),
                type.heap_index,type.flags&r.preferred,
                heap.reported_budget.has_value()
            };
        }
        // Ties deliberately keep the lower type index. Do not claim this
        // is universally the optimal device placement policy.
    }
    if(best) return *best;
    if(!found_flag_eligible)
        return std::unexpected(CapabilityError::missing_memory_type);
    if(invalid_heap)
        return std::unexpected(CapabilityError::invalid_heap_index);
    if(insufficient_size)
        return std::unexpected(CapabilityError::memory_budget_too_small);
    return std::unexpected(CapabilityError::missing_memory_type);
}
struct Case {
    std::string_view name;
    Workload workload{};
    MemoryRequest memory{};
};
[[nodiscard]] inline std::vector<Case> teaching_workloads() {
    // Sizes are illustrative QUERY RESULTS for a teaching fixture, not
    // observed resource requirements or memory budgets of any actual device.
    return {
      {"NES CPU debugger overlay",Workload::nes_overlay,
       {256,256,0b111,mem_host_visible|mem_host_coherent,mem_device_local}},
      {"Kairo indexed mesh vertex/storage",Workload::kairo_presented_scene,
       {65536,256,0b111,mem_device_local,mem_host_visible}},
      {"Maveb Gaussian project/scan storage",Workload::maveb_gaussian_raster_and_compute,
       {1048576,256,0b111,mem_device_local,mem_host_visible}},
      {"Kyoto collision-pair append buffer",Workload::kyoto_collision_pairs_compute,
       {131072,256,0b111,mem_device_local,mem_host_visible}},
      {"XR gaze-dependent compositing",Workload::xr_gaze_and_render,
       {262144,256,0b111,mem_device_local,mem_host_visible}},
    };
}
} // namespace allmight::gpu_course
