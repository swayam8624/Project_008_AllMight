// A single-threaded generational registry. Handles are values, not addresses.
// The generation counter changes when a slot is freed. A stale handle cannot
// suddenly designate a different object just because the slot was reused.
//
// This teaching example permanently retires a slot if its generation is
// UINT32_MAX; it does not claim to solve concurrency, ABA, or distributed IDs.
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

struct Handle {
    std::size_t slot;
    std::uint32_t generation;
};

class Registry {
    struct Slot {
        std::uint32_t generation = 1;
        int value = 0;
        bool live = false;
        bool retired = false;
    };

    std::vector<Slot> slots_;
    std::vector<std::size_t> free_slots_;

public:
    Handle create(int value) {
        if (!free_slots_.empty()) {
            const std::size_t index = free_slots_.back();
            free_slots_.pop_back();
            Slot& item = slots_[index];
            // A recycled slot must already carry its new generation.
            if (item.live || item.retired) {
                throw std::logic_error("Invalid free-slot bookkeeping");
            }
            item.value = value;
            item.live = true;
            return {index, item.generation};
        }

        const std::size_t index = slots_.size();
        slots_.push_back(Slot{1, value, true, false});
        return {index, 1};
    }

    std::optional<int> read(Handle handle) const {
        if (handle.slot >= slots_.size()) return std::nullopt;
        const Slot& item = slots_[handle.slot];
        if (!item.live || item.retired || item.generation != handle.generation) {
            return std::nullopt;
        }
        return item.value;
    }

    bool erase(Handle handle) {
        if (!read(handle).has_value()) return false;

        Slot& item = slots_[handle.slot];
        item.live = false;
        if (item.generation == std::numeric_limits<std::uint32_t>::max()) {
            // Never wrap around to a generation previously issued.
            item.retired = true;
        } else {
            ++item.generation;
            free_slots_.push_back(handle.slot);
        }
        return true;
    }
};

int main() {
    Registry registry;
    const Handle old = registry.create(42);
    if (registry.read(old) != std::optional<int>{42}) return 1;
    if (!registry.erase(old) || registry.read(old).has_value()) return 2;
    if (registry.erase(old)) return 3; // Duplicate delete is rejected.

    const Handle reused = registry.create(99);
    if (reused.slot != old.slot || reused.generation == old.generation) return 4;
    if (registry.read(reused) != std::optional<int>{99}) return 5;
    if (registry.read(old).has_value()) return 6;

    const Handle other = registry.create(-7);
    if (!registry.erase(other) || registry.read(reused) != std::optional<int>{99}) {
        return 7;
    }

    std::cout << "generational_handles: all deterministic contract tests passed\n";
}
