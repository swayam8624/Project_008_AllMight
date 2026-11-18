#pragma once
// Project AllMight / Book 1 / Pass 09 — CPU-only memory-contract oracle.
//
// This is a pedagogical model of resource initialization, byte-range
// visibility, conceptual execution roles, layout, and safe retirement.
// It does NOT call Vulkan/Metal, implement their memory models, emulate
// caches, define actual queue-family ownership, or perform GPU work.
// Every real backend synchronization mapping must be verified separately.
//
// Reuses Pass 08's ResourceKey, Domain and typed task graph. The same
// owner/slot/generation identity can name an imported NES texture in the
// CPU-side *plan*, without pretending that logical identity owns device
// memory or ensures device completion.
#include "work_dependency_graph.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace allmight::nes::gpu_model {

using work::ResourceKey;
using work::Domain;
using Byte = std::uint8_t;

// These are educational resource roles, not VkImageLayout constants or
// Metal resource hazard tracking modes.
enum class Layout : std::uint8_t {
    undefined, host_visible, transfer_source, transfer_destination,
    general, shader_read_only
};
enum class MemoryError : std::uint8_t {
    invalid_extent, invalid_alignment, capacity_exhausted,
    duplicate_identity, missing_identity, already_retired,
    wrong_role, wrong_layout, range_out_of_bounds, uninitialized,
    producer_not_visible, consumer_not_visible, invalid_barrier,
    epoch_exhausted, completion_pending, invalid_submission
};
struct Allocation {
    ResourceKey key{};
    std::size_t offset{};
    std::size_t size{};
    std::size_t alignment{};
};
struct Descriptor {
    Allocation allocation{};
    Layout layout{Layout::undefined};
    Domain owner{Domain::host};
    bool live{};
    std::uint64_t pending_ticket{};
};
struct BarrierDescription {
    ResourceKey resource{};
    Domain source{Domain::host};
    Domain destination{Domain::host};
    Layout expected_layout{Layout::undefined};
    Layout resulting_layout{Layout::undefined};
    std::size_t offset{};       // relative to the individual allocation
    std::size_t size{};
};
[[nodiscard]] constexpr std::size_t stage_index(Domain d) noexcept {
    switch(d) {
        case Domain::host: return 0;
        case Domain::gpu_transfer: return 1;
        case Domain::gpu_compute: return 2;
        case Domain::gpu_graphics: return 3;
    }
    return 0;
}
[[nodiscard]] constexpr bool power_of_two(std::size_t value) noexcept {
    return value != 0 && (value & (value-1U)) == 0;
}
[[nodiscard]] constexpr bool in_bounds(std::size_t capacity,
                                        std::size_t offset,
                                        std::size_t size) noexcept {
    return offset <= capacity && size <= capacity - offset;
}
[[nodiscard]] constexpr bool write_layout(Domain role,Layout layout) noexcept {
    switch(role) {
        case Domain::host:
            return layout==Layout::undefined || layout==Layout::host_visible;
        case Domain::gpu_transfer: return layout==Layout::transfer_destination;
        case Domain::gpu_compute: return layout==Layout::general;
        case Domain::gpu_graphics: return layout==Layout::general;
    }
    return false;
}
[[nodiscard]] constexpr bool read_layout(Domain role,Layout layout) noexcept {
    switch(role) {
        case Domain::host: return layout==Layout::host_visible ||
                                  layout==Layout::undefined;
        case Domain::gpu_transfer: return layout==Layout::transfer_source;
        case Domain::gpu_compute:
        case Domain::gpu_graphics:
            return layout==Layout::shader_read_only || layout==Layout::general;
    }
    return false;
}

class MemoryContractOracle {
    struct State {
        Allocation allocation;
        Layout layout{Layout::undefined};
        Domain owner{Domain::host};
        std::vector<Byte> bytes;
        std::vector<Byte> initialized; // true only after a successful write
        std::vector<std::uint64_t> write_epoch;
        // The version of each byte visible within each conceptual role.
        std::array<std::vector<std::uint64_t>,4> visible;
        std::uint64_t pending_ticket{};
        bool live{true};
    };
public:
    explicit MemoryContractOracle(std::size_t arena_bytes) noexcept
        : arena_bytes_(arena_bytes) {}

    [[nodiscard]] std::size_t committed_extent() const noexcept {
        return cursor_;
    }
    [[nodiscard]] std::size_t arena_capacity() const noexcept {
        return arena_bytes_;
    }
    [[nodiscard]] std::uint64_t completed_ticket() const noexcept {
        return completed_ticket_;
    }

    // This is a monotonic *virtual* suballocator for proofs: retirement does
    // not reclaim offsets or make physical aliasing valid. The backing bytes
    // are separately owned vectors. It is NOT a real placed GPU heap.
    [[nodiscard]] std::expected<Allocation,MemoryError>
    allocate(ResourceKey key,std::size_t size,std::size_t alignment) {
        if (size==0)
            return std::unexpected(MemoryError::invalid_extent);
        if (!power_of_two(alignment))
            return std::unexpected(MemoryError::invalid_alignment);
        if (records_.contains(key))
            return std::unexpected(MemoryError::duplicate_identity);

        const auto pad=alignment-1U;
        if (cursor_>std::numeric_limits<std::size_t>::max()-pad)
            return std::unexpected(MemoryError::capacity_exhausted);
        const auto offset=(cursor_+pad)&~pad;
        if (!in_bounds(arena_bytes_,offset,size))
            return std::unexpected(MemoryError::capacity_exhausted);

        State prepared;
        prepared.allocation={key,offset,size,alignment};
        prepared.bytes.resize(size,0);
        prepared.initialized.resize(size,0);
        prepared.write_epoch.resize(size,0);
        for (auto& v:prepared.visible) v.resize(size,0);
        // The map may allocate: the virtual allocation watermark is
        // committed only after all arrays and map insertion succeed.
        records_.emplace(key,std::move(prepared));
        cursor_=offset+size;
        return Allocation{key,offset,size,alignment};
    }

    [[nodiscard]] std::expected<Descriptor,MemoryError>
    describe(ResourceKey key) const noexcept {
        const auto it=records_.find(key);
        if (it==records_.end())
            return std::unexpected(MemoryError::missing_identity);
        const auto& r=it->second;
        return Descriptor{r.allocation,r.layout,r.owner,r.live,r.pending_ticket};
    }

    [[nodiscard]] std::expected<void,MemoryError>
    write(ResourceKey key,Domain role,std::size_t offset,
          std::span<const Byte> data) noexcept {
        auto result=active(key);
        if (!result) return std::unexpected(result.error());
        State& r=**result;
        if (r.owner!=role)
            return std::unexpected(MemoryError::wrong_role);
        if (!write_layout(role,r.layout))
            return std::unexpected(MemoryError::wrong_layout);
        if (!in_bounds(r.allocation.size,offset,data.size()))
            return std::unexpected(MemoryError::range_out_of_bounds);
        if (next_epoch_==std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(MemoryError::epoch_exhausted);
        const auto epoch=++next_epoch_;
        for (std::size_t i=0;i<data.size();++i) {
            const auto at=offset+i;
            r.bytes[at]=data[i];
            r.initialized[at]=1;
            r.write_epoch[at]=epoch;
            r.visible[stage_index(role)][at]=epoch;
        }
        return {};
    }

    [[nodiscard]] std::expected<std::vector<Byte>,MemoryError>
    read(ResourceKey key,Domain role,std::size_t offset,std::size_t size) const {
        const auto result=active(key);
        if (!result) return std::unexpected(result.error());
        const State& r=**result;
        if (r.owner!=role)
            return std::unexpected(MemoryError::wrong_role);
        if (!read_layout(role,r.layout))
            return std::unexpected(MemoryError::wrong_layout);
        if (!in_bounds(r.allocation.size,offset,size))
            return std::unexpected(MemoryError::range_out_of_bounds);
        for (std::size_t i=offset;i<offset+size;++i) {
            if (!r.initialized[i])
                return std::unexpected(MemoryError::uninitialized);
            if (r.visible[stage_index(role)][i]<r.write_epoch[i])
                return std::unexpected(MemoryError::consumer_not_visible);
        }
        return std::vector<Byte>(
            r.bytes.begin()+static_cast<std::ptrdiff_t>(offset),
            r.bytes.begin()+static_cast<std::ptrdiff_t>(offset+size));
    }

    // This explicit operation *models* the effects a real backend must
    // establish through valid pipeline/encoder/queue synchronization.
    // It neither checks real GPU stages nor issues a native barrier.
    [[nodiscard]] std::expected<void,MemoryError>
    barrier(const BarrierDescription& b) noexcept {
        auto result=active(b.resource);
        if (!result) return std::unexpected(result.error());
        State& r=**result;
        if (r.owner!=b.source)
            return std::unexpected(MemoryError::wrong_role);
        if (r.layout!=b.expected_layout)
            return std::unexpected(MemoryError::wrong_layout);
        if (!in_bounds(r.allocation.size,b.offset,b.size) || b.size==0)
            return std::unexpected(MemoryError::range_out_of_bounds);
        // In this simplified oracle both logical role ownership and image
        // layout are properties of WHOLE resources. Range visibility may
        // be narrower only when neither ownership nor layout changes.
        const bool whole=b.offset==0 && b.size==r.allocation.size;
        if (!whole && (b.source!=b.destination ||
                       b.expected_layout!=b.resulting_layout))
            return std::unexpected(MemoryError::invalid_barrier);

        for (std::size_t i=b.offset;i<b.offset+b.size;++i) {
            if (r.initialized[i] &&
                r.visible[stage_index(b.source)][i]<r.write_epoch[i])
                return std::unexpected(MemoryError::producer_not_visible);
        }
        for (std::size_t i=b.offset;i<b.offset+b.size;++i) {
            if (r.initialized[i])
                r.visible[stage_index(b.destination)][i]=r.write_epoch[i];
        }
        r.owner=b.destination;
        r.layout=b.resulting_layout;
        return {};
    }

    [[nodiscard]] std::expected<void,MemoryError>
    copy_transfer(ResourceKey from,ResourceKey to,std::size_t bytes) {
        // Source read is validated BEFORE any destination mutation.
        const auto source=read(from,Domain::gpu_transfer,0,bytes);
        if (!source) return std::unexpected(source.error());
        return write(to,Domain::gpu_transfer,0,*source);
    }

    // Simulated external completion. A ticket is NOT a Vulkan fence, Metal
    // event or an actual device-completion observation.
    [[nodiscard]] std::expected<void,MemoryError>
    mark_in_flight(ResourceKey key,std::uint64_t ticket) noexcept {
        auto result=active(key);
        if (!result) return std::unexpected(result.error());
        State& r=**result;
        if (ticket<=completed_ticket_ || ticket<r.pending_ticket ||
            ticket==0)
            return std::unexpected(MemoryError::invalid_submission);
        r.pending_ticket=ticket;
        return {};
    }
    [[nodiscard]] std::expected<void,MemoryError>
    observe_completion(std::uint64_t ticket) noexcept {
        if (ticket<completed_ticket_)
            return std::unexpected(MemoryError::invalid_submission);
        completed_ticket_=ticket;
        return {};
    }
    [[nodiscard]] std::expected<void,MemoryError>
    retire(ResourceKey key) noexcept {
        auto result=active(key);
        if (!result) return std::unexpected(result.error());
        State& r=**result;
        if (r.pending_ticket>completed_ticket_)
            return std::unexpected(MemoryError::completion_pending);
        r.live=false;
        // Tombstones prohibit resurrection of a stale handle. The virtual
        // arena does not reclaim/alias old offsets in this teaching model.
        r.bytes.clear();
        r.initialized.clear();
        r.write_epoch.clear();
        for (auto& v:r.visible) v.clear();
        return {};
    }

private:
    [[nodiscard]] std::expected<State*,MemoryError>
    active(ResourceKey key) noexcept {
        auto it=records_.find(key);
        if (it==records_.end())
            return std::unexpected(MemoryError::missing_identity);
        if (!it->second.live)
            return std::unexpected(MemoryError::already_retired);
        return &it->second;
    }
    [[nodiscard]] std::expected<const State*,MemoryError>
    active(ResourceKey key) const noexcept {
        auto it=records_.find(key);
        if (it==records_.end())
            return std::unexpected(MemoryError::missing_identity);
        if (!it->second.live)
            return std::unexpected(MemoryError::already_retired);
        return &it->second;
    }
    std::size_t arena_bytes_{};
    std::size_t cursor_{};
    std::uint64_t next_epoch_{};
    std::uint64_t completed_ticket_{};
    std::unordered_map<ResourceKey,State,work::ResourceKeyHash> records_;
};

// A deliberately simple *analytical* transaction-count proxy. It counts
// unique aligned byte segments touched by chosen lane addresses. It does not
// model GPU memory controllers, sectors, cache lines, subgroup width,
// transactions actually issued, residency, or observed timing.
enum class PatternError { invalid_segment };
[[nodiscard]] inline std::expected<std::size_t,PatternError>
segment_count(std::span<const std::uint64_t> addresses,
              std::uint64_t segment_bytes) {
    if (segment_bytes==0 || (segment_bytes&(segment_bytes-1U))!=0)
        return std::unexpected(PatternError::invalid_segment);
    std::vector<std::uint64_t> segments;
    segments.reserve(addresses.size());
    for (const auto address:addresses) segments.push_back(address/segment_bytes);
    std::sort(segments.begin(),segments.end());
    segments.erase(std::unique(segments.begin(),segments.end()),segments.end());
    return segments.size();
}

// One explicit workgroup barrier in a pedagogical algorithm must be reached
// by every intended participant. A mixed mask is structurally unsafe.
// "all false" means the barrier was not executed; that cannot synchronize.
enum class Participation { nobody, everybody, divergent };
[[nodiscard]] inline Participation barrier_participation(
    std::span<const Byte> reaches_barrier) noexcept {
    std::size_t yes{};
    for (const auto x:reaches_barrier) yes+=(x!=0);
    if (yes==0) return Participation::nobody;
    if (yes==reaches_barrier.size()) return Participation::everybody;
    return Participation::divergent;
}

} // namespace allmight::nes::gpu_model
