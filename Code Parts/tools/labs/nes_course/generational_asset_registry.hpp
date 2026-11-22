#pragma once
// AllMight Book 1 / Pass 06: stable typed *logical* handles for a growing store.
//
// Reuses the exact previously implemented Spec<Tag>, AssetRecord and NROM
// cartridge. This is a single-threaded CPU-only experiment; it is not a GPU
// lifetime manager, concurrent hot-reload registry or persistent asset ID.
//
// Invariants:
//   - every handle contains a store-issued process-local owner ID
//   - every slot stores a nonzero generation and optional owning asset
//   - erase increments generation before slot reuse (never wraps)
//   - when generation reaches UINT64_MAX, erase retires slot permanently
//   - invalid, stale, foreign and wrong-kind handles never mutate the store
//   - borrow results do not survive registry mutations that relocate slots
//   - each import prepares one complete domain object before publishing it

#include "typed_asset_pipeline.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <exception>
#include <functional>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace allmight::nes::assets {

enum class LookupError {
    wrong_owner, missing_slot, stale_or_dead, wrong_kind
};
enum class EraseError {
    wrong_owner, missing_slot, stale_or_dead, wrong_kind, allocation_failure
};
enum class RegistryImportError {
    rejected_source, capacity_exhausted, injected_failure,
    allocation_failure, invalid_moved_from_owner
};
struct RegistryFailure {
    RegistryImportError kind{};
    DecodeError source{}; // meaningful only for rejected_source
};

template<class Tag>
struct AssetHandle {
    std::uint64_t owner{};
    std::size_t slot{};
    std::uint64_t generation{};
    friend bool operator==(const AssetHandle&, const AssetHandle&) = default;
};

// Owner namespaces must not silently alias between two registries in a
// process. Atomic ID minting protects ONLY construction of owner IDs.
// Registry mutations and borrowed reads still require external coordination.
inline std::atomic<std::uint64_t> next_registry_owner{1};

inline std::uint64_t mint_owner() {
    auto old = next_registry_owner.load(std::memory_order_relaxed);
    while (old != std::numeric_limits<std::uint64_t>::max()) {
        if (next_registry_owner.compare_exchange_weak(
                old, old + 1, std::memory_order_relaxed,
                std::memory_order_relaxed)) {
            return old;
        }
    }
    throw std::overflow_error("per-process asset registry ID space exhausted");
}

class GenerationalAssetRegistry {
    struct Slot {
        std::optional<AssetRecord> asset{};
        std::uint64_t generation{1};
        bool retired{false};
    };
    static_assert(std::is_nothrow_move_constructible_v<Slot>);
    static_assert(std::is_nothrow_move_constructible_v<AssetRecord>);
    static_assert(std::is_nothrow_destructible_v<AssetRecord>);

public:
    // Generation seed is a deterministic *test configuration* allowing the
    // near-overflow behavior to be exercised without billions of removals.
    // Ordinary clients omit it and start every slot at generation one.
    explicit GenerationalAssetRegistry(
        std::size_t max_slots,
        std::uint64_t test_initial_generation = 1)
        : max_slots_(max_slots),
          owner_(mint_owner()),
          initial_generation_(test_initial_generation) {
        if (test_initial_generation == 0)
            throw std::invalid_argument("generation zero is reserved");
    }

    GenerationalAssetRegistry(const GenerationalAssetRegistry&) = delete;
    GenerationalAssetRegistry& operator=(const GenerationalAssetRegistry&) = delete;

    // Move construction transfers the logical owner namespace and all slots.
    // Existing handles resolve against the destination, not the moved-from
    // object. Never move while another thread uses the source registry.
    GenerationalAssetRegistry(GenerationalAssetRegistry&& old) noexcept
        : max_slots_(old.max_slots_),
          slots_(std::move(old.slots_)),
          free_(std::move(old.free_)),
          owner_(std::exchange(old.owner_, 0)),
          active_(std::exchange(old.active_, 0)),
          initial_generation_(old.initial_generation_) {}
    GenerationalAssetRegistry& operator=(GenerationalAssetRegistry&&) = delete;

    [[nodiscard]] std::uint64_t owner_id() const noexcept { return owner_; }
    [[nodiscard]] std::size_t active_count() const noexcept { return active_; }
    [[nodiscard]] std::size_t allocated_slots() const noexcept { return slots_.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept { return max_slots_; }
    [[nodiscard]] std::size_t recycled_slots() const noexcept { return free_.size(); }

    template<Supported Tag>
    [[nodiscard]] std::expected<AssetHandle<Tag>, RegistryFailure>
    import(ByteView bytes, TestFault fault = TestFault::none) {
        static_assert(std::is_nothrow_move_constructible_v<AssetRecord>,
                      "prepared variant must have a non-throwing move constructor");
        if (owner_ == 0) {
            return std::unexpected(RegistryFailure{
                RegistryImportError::invalid_moved_from_owner, {}});
        }
        if (free_.empty() && slots_.size() == max_slots_) {
            return std::unexpected(RegistryFailure{
                RegistryImportError::capacity_exhausted, {}});
        }
        if (fault == TestFault::before_stage) {
            return std::unexpected(RegistryFailure{
                RegistryImportError::injected_failure, {}});
        }

        try {
            // Prepare without modifying slots or free-list. Domain validators
            // come from the SAME implementation used by Pass 05.
            auto staged = Spec<Tag>::decode(bytes);
            if (!staged) {
                return std::unexpected(RegistryFailure{
                    RegistryImportError::rejected_source, staged.error()});
            }
            if (fault == TestFault::after_stage) {
                return std::unexpected(RegistryFailure{
                    RegistryImportError::injected_failure, {}});
            }
            if (fault == TestFault::throw_bad_alloc_after_stage) {
                throw std::bad_alloc{};
            }

            // A variant's in-place constructor need not be declared noexcept,
            // even when its alternative has a noexcept move constructor.
            // Prepare the entire variant before mutating a registry slot.
            AssetRecord prepared(std::in_place_type<Asset<Tag>>,
                                 std::move(*staged));

            if (!free_.empty()) {
                const auto index = free_.back();
                Slot& slot = slots_[index];
                // A free-list element denotes an empty non-retired slot.
                // The prepared variant can be moved into an empty slot without throwing.
                if (slot.retired || slot.asset.has_value() ||
                    slot.generation == 0) {
                    std::terminate(); // internal corruption, not bad input
                }
                slot.asset.emplace(std::move(prepared));
                free_.pop_back();
                ++active_;
                return AssetHandle<Tag>{owner_, index, slot.generation};
            }

            // Appending a fresh default slot may allocate. Old slots are
            // nothrow movable, so allocation failure leaves them usable.
            const auto index = slots_.size();
            slots_.emplace_back();
            Slot& slot = slots_.back();
            slot.generation = initial_generation_;
            slot.asset.emplace(std::move(prepared));
            ++active_;
            return AssetHandle<Tag>{owner_, index, slot.generation};
        } catch (const std::bad_alloc&) {
            return std::unexpected(RegistryFailure{
                RegistryImportError::allocation_failure, {}});
        }
    }

    template<Supported Tag>
    [[nodiscard]] std::expected<std::reference_wrapper<const Asset<Tag>>, LookupError>
    lookup(AssetHandle<Tag> handle) const noexcept {
        if (owner_ == 0 || handle.owner != owner_)
            return std::unexpected(LookupError::wrong_owner);
        if (handle.slot >= slots_.size())
            return std::unexpected(LookupError::missing_slot);
        const Slot& slot = slots_[handle.slot];
        if (slot.generation != handle.generation || !slot.asset ||
            slot.retired || handle.generation == 0)
            return std::unexpected(LookupError::stale_or_dead);
        const auto* value = std::get_if<Asset<Tag>>(&*slot.asset);
        if (!value) return std::unexpected(LookupError::wrong_kind);
        // Borrowed reference only: never retain across erase/import/registry
        // move/destruction without a separate lifetime protocol.
        return std::cref(*value);
    }

    template<Supported Tag>
    [[nodiscard]] std::expected<void, EraseError>
    erase(AssetHandle<Tag> handle) {
        const auto value = lookup(handle);
        if (!value) {
            switch (value.error()) {
                case LookupError::wrong_owner:
                    return std::unexpected(EraseError::wrong_owner);
                case LookupError::missing_slot:
                    return std::unexpected(EraseError::missing_slot);
                case LookupError::stale_or_dead:
                    return std::unexpected(EraseError::stale_or_dead);
                case LookupError::wrong_kind:
                    return std::unexpected(EraseError::wrong_kind);
            }
        }

        Slot& slot = slots_[handle.slot];
        const bool exhausts_generation =
            slot.generation == std::numeric_limits<std::uint64_t>::max();

        if (!exhausts_generation) {
            // Reserve BEFORE mutating the live slot; a failed allocation then
            // cannot result in lost content or a changed generation.
            try {
                free_.reserve(slots_.size());
            } catch (const std::bad_alloc&) {
                return std::unexpected(EraseError::allocation_failure);
            }
        }

        // All following operations are nonthrowing for this concrete type.
        slot.asset.reset();
        --active_;
        if (exhausts_generation) {
            // A wrapping generation can make an ancient handle resolve to
            // a different object. Permanently retire this exhausted slot.
            slot.retired = true;
        } else {
            ++slot.generation;
            free_.push_back(handle.slot);
        }
        return {};
    }

private:
    std::size_t max_slots_{};
    std::vector<Slot> slots_;
    std::vector<std::size_t> free_; // LIFO reuse of empty non-retired slots
    std::uint64_t owner_{};
    std::size_t active_{};
    std::uint64_t initial_generation_{1};
};

static_assert(!std::is_copy_constructible_v<GenerationalAssetRegistry>);
static_assert(std::is_nothrow_move_constructible_v<GenerationalAssetRegistry>);
static_assert(!std::is_convertible_v<
    AssetHandle<CartridgeTag>, AssetHandle<TextureTag>>);

} // namespace allmight::nes::assets
