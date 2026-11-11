#include "asset_catalog.hpp"
#include <atomic>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight::m018 {
namespace {
std::atomic<std::uint64_t> next_catalog_id{1};
std::uint64_t take_catalog_id() {
    auto value = next_catalog_id.load(std::memory_order_relaxed);
    for (;;) {
        if (value == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("catalog identity space exhausted");
        if (next_catalog_id.compare_exchange_weak(
                value, value + 1, std::memory_order_relaxed))
            return value;
    }
}
} // namespace

struct AssetCatalog::Impl {
    struct Slot {
        std::optional<MeshInfo> mesh;
        std::uint64_t generation{1};
        bool retired{false};
    };
    const std::uint64_t catalog_id = take_catalog_id();
    std::vector<Slot> slots;
    std::vector<std::uint32_t> free_slots;
    std::size_t living{};

    const MeshInfo* find(MeshHandle handle) const noexcept {
        if (handle.catalog_id != catalog_id || handle.slot >= slots.size())
            return nullptr;
        const auto& slot = slots[handle.slot];
        if (slot.retired || slot.generation != handle.generation)
            return nullptr;
        return slot.mesh ? &*slot.mesh : nullptr;
    }

    MeshHandle insert(MeshInfo mesh) {
        if (!free_slots.empty()) {
            const auto index = free_slots.back();
            auto& slot = slots[index];
            // MeshInfo's default-allocator string move is non-throwing.
            // The reusable list changes only after emplacement succeeds.
            slot.mesh.emplace(std::move(mesh));
            free_slots.pop_back();
            ++living;
            return {catalog_id, index, slot.generation};
        }
        if (slots.size() >= std::numeric_limits<std::uint32_t>::max())
            throw std::length_error("slot address space exhausted");
        const auto index = static_cast<std::uint32_t>(slots.size());
        slots.push_back(Slot{std::optional<MeshInfo>{std::move(mesh)}, 1, false});
        ++living;
        return {catalog_id, index, 1};
    }

    bool erase(MeshHandle handle) {
        if (!find(handle)) return false;
        auto& slot = slots[handle.slot];
        const bool can_reuse = slot.generation !=
            std::numeric_limits<std::uint64_t>::max();
        // Reserve before mutating the slot, so bad_alloc cannot strand it.
        if (can_reuse) free_slots.reserve(free_slots.size() + 1);
        slot.mesh.reset();
        --living;
        if (can_reuse) {
            ++slot.generation;
            free_slots.push_back(handle.slot);
        } else {
            slot.retired = true; // Do not wrap the generation.
        }
        return true;
    }
};

AssetCatalog::AssetCatalog() : impl_(std::make_unique<Impl>()) {}
AssetCatalog::~AssetCatalog() = default;
AssetCatalog::AssetCatalog(AssetCatalog&&) noexcept = default;
AssetCatalog& AssetCatalog::operator=(AssetCatalog&&) noexcept = default;

MeshHandle AssetCatalog::insert(MeshInfo mesh) {
    if (!impl_) throw std::logic_error("moved-from catalog");
    return impl_->insert(std::move(mesh));
}
bool AssetCatalog::erase(MeshHandle handle) {
    if (!impl_) throw std::logic_error("moved-from catalog");
    return impl_->erase(handle);
}
std::optional<MeshInfo> AssetCatalog::lookup(MeshHandle handle) const {
    if (!impl_) throw std::logic_error("moved-from catalog");
    if (const auto* mesh = impl_->find(handle)) return *mesh;
    return std::nullopt;
}
std::size_t AssetCatalog::live_count() const {
    if (!impl_) throw std::logic_error("moved-from catalog");
    return impl_->living;
}
} // namespace allmight::m018
