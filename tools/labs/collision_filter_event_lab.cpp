// Bounded teaching witness for reciprocal category/mask filtering, same-group
// overrides, set-difference lifecycle events, and generation-safe event IDs.
// Compile: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
//   tools/labs/collision_filter_event_lab.cpp -o /tmp/collision_filter_event_lab
// Run: /tmp/collision_filter_event_lab
#include <algorithm>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <vector>

// These checks remain active in release builds; NDEBUG must not erase the lab.
static unsigned checks = 0;
void check(bool condition) {
    ++checks;
    if (!condition) throw std::runtime_error("failed check " + std::to_string(checks));
}

struct Filter {
    std::uint64_t category;
    std::uint64_t mask;
    std::int32_t group;
};

// This models the Box2D 3.1 category/mask/group rule only.
// It does not include body-type, same-body, joint, or custom filtering rules.
bool should_collide(const Filter& a, const Filter& b) {
    if (a.group != 0 && a.group == b.group) {
        return a.group > 0; // Equal positive forces; equal negative forbids.
    }
    return (a.category & b.mask) != 0 && (b.category & a.mask) != 0;
}

struct ShapeId {
    std::uint32_t slot;
    std::uint32_t generation;
    friend bool operator==(ShapeId, ShapeId) = default;
    friend bool operator<(ShapeId a, ShapeId b) {
        return std::tie(a.slot, a.generation) < std::tie(b.slot, b.generation);
    }
};

struct Pair {
    ShapeId a;
    ShapeId b;
    static Pair canonical(ShapeId x, ShapeId y) {
        return y < x ? Pair{y, x} : Pair{x, y};
    }
    friend bool operator<(const Pair& x, const Pair& y) {
        return std::tie(x.a, x.b) < std::tie(y.a, y.b);
    }
};

enum class Kind { Begin, Stay, End };
enum class EndReason { Separated, FilterChanged, Destroyed, DiscontinuousMove };
struct Event {
    Kind kind;
    Pair pair;
    EndReason reason = EndReason::Separated;
};

// `previous` is logical touching state. Sleep does not erase it; a wake must
// revalidate geometry and filters before this function receives `current`.
std::vector<Event> transitions(const std::set<Pair>& previous,
                               const std::set<Pair>& current,
                               const std::map<Pair, EndReason>& forcedEnds = {}) {
    std::vector<Event> out;
    for (const Pair& p : current) {
        out.push_back({previous.contains(p) ? Kind::Stay : Kind::Begin, p});
    }
    for (const Pair& p : previous) {
        if (!current.contains(p)) {
            const auto it = forcedEnds.find(p);
            out.push_back({Kind::End, p, it == forcedEnds.end()
                                               ? EndReason::Separated
                                               : it->second});
        }
    }
    return out;
}

struct Slot {
    std::uint32_t generation = 1;
    bool alive = false;
};
bool valid(const std::vector<Slot>& slots, ShapeId id) {
    return id.slot < slots.size() && slots[id.slot].alive &&
           slots[id.slot].generation == id.generation;
}

int main() {
    constexpr std::uint64_t PLAYER = 1, ENEMY = 2, PLATE = 4, DEBRIS = 8;
    const Filter player{PLAYER, ENEMY | PLATE, 0};
    const Filter enemy{ENEMY, PLAYER, 0};
    const Filter plate{PLATE, PLAYER, 0};
    const Filter debris{DEBRIS, 0, 0};
    check(should_collide(player, enemy));        // both masks agree
    check(should_collide(player, plate));        // sensor-like category still filters
    check(!should_collide(player, debris));      // neither side accepts the other
    check(!should_collide(player, Filter{ENEMY, 0, 0})); // enemy rejects player
    check(should_collide(Filter{PLAYER, 0, 7}, Filter{ENEMY, 0, 7}));
    check(!should_collide(Filter{PLAYER, ~0ULL, -7}, Filter{ENEMY, ~0ULL, -7}));
    check(should_collide(Filter{PLAYER, ENEMY, 3}, Filter{ENEMY, PLAYER, -3}));
    check(should_collide(Filter{PLAYER, ENEMY, 3}, Filter{ENEMY, PLAYER, 0}));

    // A high category bit catches accidental narrowing to a 16/32-bit field.
    constexpr std::uint64_t HIGH = std::uint64_t{1} << 63;
    check(should_collide(Filter{HIGH, PLAYER, 0}, Filter{PLAYER, HIGH, 0}));
    check(transitions({}, {}).empty()); // no spurious event before any overlap

    const ShapeId oldActor{2, 4};
    const ShapeId sensor{5, 7};
    const Pair oldPair = Pair::canonical(oldActor, sensor);
    std::set<Pair> previous;
    std::set<Pair> current{oldPair};
    auto e = transitions(previous, current);
    check(e.size() == 1 && e[0].kind == Kind::Begin);
    previous = current;

    // Persisting overlap yields one application-level Stay; Box2D's buffered
    // event API documents begin/end and hit events, not a universal Stay event.
    e = transitions(previous, current);
    check(e.size() == 1 && e[0].kind == Kind::Stay);

    // A filter edit invalidates the old overlap and records why it ended.
    current.clear();
    e = transitions(previous, current, {{oldPair, EndReason::FilterChanged}});
    check(e.size() == 1 && e[0].kind == Kind::End &&
           e[0].reason == EndReason::FilterChanged);
    previous = current;

    // Re-enter, then destroy while overlapping. Copy the event IDs before any
    // structural mutation or callback; never keep engine-owned event pointers.
    current.insert(oldPair);
    e = transitions(previous, current);
    check(e.size() == 1 && e[0].kind == Kind::Begin);
    previous = current;
    current.clear();
    e = transitions(previous, current, {{oldPair, EndReason::Destroyed}});
    check(e.size() == 1 && e[0].pair.a == oldPair.a &&
           e[0].pair.b == oldPair.b && e[0].reason == EndReason::Destroyed);

    std::vector<Slot> slots(6);
    slots[2] = {4, true};
    slots[5] = {7, true};
    check(valid(slots, oldActor));
    // Deferred destruction runs at the safe structural boundary. Reusing slot
    // 2 changes generation, so an old event cannot target its new occupant.
    slots[2].alive = false;
    ++slots[2].generation;
    const ShapeId newActor{2, 5};
    slots[2].alive = true;
    check(!valid(slots, oldActor) && valid(slots, newActor));
    check(e[0].pair.a == oldPair.a && e[0].pair.a != newActor);

    std::cout << "collision filter/event lifecycle: " << checks << " checks PASS\n";
}
