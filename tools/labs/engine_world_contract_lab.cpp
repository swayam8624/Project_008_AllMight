// C++20 reference: lifecycle, input edges, streaming publication, and world-time ownership.
// This is a deterministic CPU model. It does not perform real I/O or GPU submission.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lab {
void require(bool condition, std::string_view why) {
    if (!condition) throw std::runtime_error(std::string(why));
}

// A provider may allocate something before its start() reports failure. That
// partial allocation must be rolled back by the provider, because the manager
// is only allowed to stop providers that reached the Ready state.
struct Service {
    std::string name;
    std::vector<std::size_t> dependencies;
    bool fail_after_partial_acquisition = false;
    bool ready = false;
    bool partial = false;
    std::vector<std::string>* trace = nullptr;

    bool start() {
        require(!ready && !partial, "service started twice");
        partial = true;
        trace->push_back("acquire:" + name);
        if (fail_after_partial_acquisition) {
            trace->push_back("rollback-partial:" + name);
            partial = false;
            return false;
        }
        partial = false;
        ready = true;
        trace->push_back("ready:" + name);
        return true;
    }
    void stop() {
        require(ready && !partial, "invalid service stop");
        trace->push_back("stop:" + name);
        ready = false;
    }
};

class ServiceManager {
    std::vector<Service> services_;
    std::vector<std::size_t> started_;
public:
    explicit ServiceManager(std::vector<Service> services) : services_(std::move(services)) {}
    // Preflight all graph edges and construct a deterministic topological order
    // BEFORE starting anything. Cycles or missing services cause no side effects.
    bool start_all() {
        require(started_.empty(), "manager must be stopped before restarting");
        const std::size_t n = services_.size();
        std::vector<bool> finished(n, false);
        std::vector<std::size_t> order;
        order.reserve(n);
        for (std::size_t completed = 0; completed < n; ++completed) {
            std::optional<std::size_t> next;
            for (std::size_t i = 0; i < n && !next; ++i) {
                if (finished[i]) continue;
                bool eligible = true;
                for (std::size_t dep : services_[i].dependencies) {
                    require(dep < n && dep != i, "invalid dependency");
                    eligible = eligible && finished[dep];
                }
                if (eligible) next = i;
            }
            require(next.has_value(), "dependency cycle");
            finished[*next] = true;
            order.push_back(*next);
        }
        for (std::size_t index : order) {
            if (!services_[index].start()) {
                stop_all(); // The failed service cleaned up its own partial state.
                return false;
            }
            started_.push_back(index);
        }
        return true;
    }
    void stop_all() {
        // Reverse successful-start order protects each provider from its users.
        for (auto it = started_.rbegin(); it != started_.rend(); ++it)
            services_[*it].stop();
        started_.clear();
    }
};

struct Stick { double x = 0.0, y = 0.0; };
Stick radial_dead_zone(Stick raw, double threshold) {
    require(std::isfinite(raw.x) && std::isfinite(raw.y), "nonfinite stick");
    require(threshold >= 0.0 && threshold < 1.0, "bad dead-zone threshold");
    const double radius = std::hypot(raw.x, raw.y);
    if (radius <= threshold) return {};
    const double bounded = std::min(radius, 1.0);
    // Apply the same radial gain to both axes. The direction is unchanged.
    const double new_radius = (bounded - threshold) / (1.0 - threshold);
    return {raw.x * new_radius / radius, raw.y * new_radius / radius};
}

enum class Edge { Pressed, Released };
class TickInput {
    bool held_ = false;
    std::deque<Edge> pending_;
public:
    void on_physical_button(bool pressed, bool ui_has_capture) {
        // UI ownership suppresses gameplay presses and actively releases a
        // previously held gameplay action so focus transfer cannot stick it.
        if (ui_has_capture) {
            if (held_) lose_focus();
            return;
        }
        if (pressed == held_) return; // Ignore OS repeat, not genuine transitions.
        held_ = pressed;
        pending_.push_back(pressed ? Edge::Pressed : Edge::Released);
    }
    std::vector<Edge> sample_at_tick_boundary() {
        std::vector<Edge> delivered(pending_.begin(), pending_.end());
        pending_.clear(); // Each transition is consumed at most once.
        return delivered;
    }
    bool held() const { return held_; }
    void lose_focus() {
        if (held_) on_physical_button(false, false);
    }
};

enum class AssetState { Empty, Requested, Decoded, Resident };
struct AssetSlot {
    std::uint64_t generation = 0;
    AssetState state = AssetState::Empty;
    std::size_t reserved_bytes = 0;
    std::size_t resident_bytes = 0;
    unsigned pins = 0; // World/render users explicitly prevent eviction.
};

class AssetBudget {
    std::size_t limit_;
    std::size_t reserved_ = 0;
    std::size_t resident_ = 0;
public:
    explicit AssetBudget(std::size_t limit) : limit_(limit) {}
    // Estimated reservation is charged before work starts. This is a logical
    // memory budget, not a GPU fence or exact peak staging-memory estimator.
    std::uint64_t request(AssetSlot& slot, std::size_t estimate) {
        require(slot.state == AssetState::Empty, "asset already active");
        require(estimate <= limit_ - (reserved_ + resident_), "asset budget exceeded");
        reserved_ += estimate;
        slot.reserved_bytes = estimate;
        slot.state = AssetState::Requested;
        return ++slot.generation;
    }
    void cancel(AssetSlot& slot) {
        require(slot.state == AssetState::Requested || slot.state == AssetState::Decoded,
                "cannot cancel inactive request");
        reserved_ -= slot.reserved_bytes;
        slot.reserved_bytes = 0;
        slot.state = AssetState::Empty;
        ++slot.generation; // Invalidates completions already in flight.
    }
    bool decode_finished(AssetSlot& slot, std::uint64_t generation) {
        if (slot.generation != generation || slot.state != AssetState::Requested)
            return false;
        slot.state = AssetState::Decoded;
        return true;
    }
    bool publish(AssetSlot& slot, std::uint64_t generation, std::size_t bytes) {
        if (slot.generation != generation || slot.state != AssetState::Decoded)
            return false;
        // Reject incorrect estimates without changing accounted memory.
        if (bytes > slot.reserved_bytes) return false;
        reserved_ -= slot.reserved_bytes;
        slot.reserved_bytes = 0;
        resident_ += bytes;
        slot.resident_bytes = bytes;
        slot.state = AssetState::Resident;
        return true;
    }
    bool evict(AssetSlot& slot) {
        if (slot.state != AssetState::Resident || slot.pins != 0) return false;
        resident_ -= slot.resident_bytes;
        slot.resident_bytes = 0;
        slot.state = AssetState::Empty;
        ++slot.generation;
        return true;
    }
    std::size_t charged() const { return reserved_ + resident_; }
    std::size_t resident() const { return resident_; }
};

// Persistent authored identity is not a pointer to a resident object. A cell
// can disappear while the identity remains usable in saved quests or scripts.
struct ObjectId { std::uint32_t cell, local; };
struct Cell {
    std::uint32_t cell = 0;
    bool resident = false;
    unsigned pins = 0;
};
std::optional<ObjectId> resolve(const Cell& cell, ObjectId id) {
    if (!cell.resident || cell.cell != id.cell) return std::nullopt;
    return id;
}
bool unload(Cell& cell) {
    if (cell.pins != 0) return false;
    cell.resident = false;
    return true;
}

struct Simulation {
    double previous = 0.0;
    double current = 0.0;
    double velocity = 2.0;
    double accumulator = 0.0;
    std::uint64_t tick = 0;
    static constexpr double dt = 0.02;

    void update(double real_seconds) {
        require(std::isfinite(real_seconds) && real_seconds >= 0.0,
                "invalid monotonic delta");
        accumulator += std::min(real_seconds, 0.1); // Explicit overload policy.
        unsigned steps = 0;
        while (accumulator + 1e-12 >= dt && steps < 5) {
            previous = current;
            current += velocity * dt; // Only fixed simulation writes authority.
            accumulator -= dt;
            ++tick;
            ++steps;
        }
        if (accumulator >= dt) accumulator = std::fmod(accumulator, dt);
    }
    double render_interpolated() const {
        const double alpha = std::clamp(accumulator / dt, 0.0, 1.0);
        return previous * (1.0 - alpha) + current * alpha;
    }
};

// Root motion is a movement *request*. Collision/character movement produces
// the authoritative displacement; skinning then follows the resolved root.
struct Character {
    double world_x = 0.0;
    unsigned footstep_events = 0;
    std::optional<std::uint64_t> last_applied_tick;
    void apply_root_request(double requested, double max_allowed, std::uint64_t tick) {
        require(requested >= 0.0 && max_allowed >= 0.0, "invalid root request");
        if (last_applied_tick && *last_applied_tick == tick) return;
        last_applied_tick = tick; // Tick authority accepts at most one proposal.
        world_x += std::min(requested, max_allowed); // Collision-resolved delta.
        if (requested > 0.0) ++footstep_events; // One gameplay event per tick.
    }
};
} // namespace lab

int main() {
    using namespace lab;
    std::vector<std::string> trace;
    ServiceManager manager({
        {"renderer", {1}, true, false, false, &trace},
        {"window", {2}, false, false, false, &trace},
        {"platform", {}, false, false, false, &trace}
    });
    require(!manager.start_all(), "expected renderer startup failure");
    require(trace == std::vector<std::string>({
        "acquire:platform", "ready:platform", "acquire:window", "ready:window",
        "acquire:renderer", "rollback-partial:renderer", "stop:window", "stop:platform"
    }), "startup rollback order changed");

    // Reject dependency cycles before creating a single native resource.
    std::vector<std::string> invalid_trace;
    ServiceManager cyclic({
        {"a", {1}, false, false, false, &invalid_trace},
        {"b", {0}, false, false, false, &invalid_trace}
    });
    bool cycle_rejected = false;
    try { cyclic.start_all(); }
    catch (const std::runtime_error&) { cycle_rejected = true; }
    require(cycle_rejected && invalid_trace.empty(),
            "cycle had effects before graph validation");

    auto stick = radial_dead_zone({0.3, 0.4}, 0.2); // Radius = 0.5.
    require(std::abs(std::hypot(stick.x, stick.y) - 0.375) < 1e-12,
            "incorrect radial rescale");
    require(std::abs(stick.x / stick.y - 0.75) < 1e-12,
            "dead zone changed stick direction");
    require(radial_dead_zone({0.1, 0.1}, 0.2).x == 0.0, "inside dead zone");
    TickInput action;
    action.on_physical_button(true, false);
    action.on_physical_button(false, false); // Both edges between two ticks.
    const auto edges = action.sample_at_tick_boundary();
    require(edges.size() == 2 && edges[0] == Edge::Pressed &&
            edges[1] == Edge::Released && !action.held(), "tap was lost");
    require(action.sample_at_tick_boundary().empty(), "edge fired twice");
    TickInput captured;
    captured.on_physical_button(true, false);
    captured.on_physical_button(true, true); // Focus moved to an editor control.
    const auto capture_edges = captured.sample_at_tick_boundary();
    require(capture_edges.size() == 2 &&
            capture_edges[1] == Edge::Released && !captured.held(),
            "UI capture left a gameplay action stuck");

    AssetBudget tiny(8);
    AssetSlot first, second;
    tiny.request(first, 8);
    bool overbudget_rejected = false;
    try { tiny.request(second, 1); }
    catch (const std::runtime_error&) { overbudget_rejected = true; }
    require(overbudget_rejected && tiny.charged() == 8,
            "overbudget admission modified accounting");
    tiny.cancel(first);
    AssetBudget budget(128);
    AssetSlot door;
    const auto stale = budget.request(door, 64);
    budget.cancel(door);
    const auto current = budget.request(door, 40);
    require(!budget.decode_finished(door, stale), "stale decode accepted");
    require(budget.decode_finished(door, current), "current decode refused");
    require(!budget.publish(door, current, 41), "oversized publication accepted");
    require(budget.publish(door, current, 36) && budget.resident() == 36,
            "publication accounting incorrect");
    door.pins = 1; // A frame snapshot still references this resource.
    require(!budget.evict(door), "evicted a pinned asset");
    door.pins = 0;
    require(budget.evict(door) && budget.charged() == 0, "asset leak");

    Cell cell{7, true, 1};
    ObjectId quest_target{7, 42};
    require(resolve(cell, quest_target).has_value(), "lost resident target");
    require(!unload(cell), "unloaded pinned world cell");
    cell.pins = 0;
    require(unload(cell) && !resolve(cell, quest_target).has_value(),
            "unloaded object still resolved");

    Simulation sim;
    sim.update(0.03); // One 20 ms simulation step; 10 ms render remainder.
    require(sim.tick == 1 && std::abs(sim.current - 0.04) < 1e-12,
            "simulation advanced incorrectly");
    const double authoritative = sim.current;
    require(std::abs(sim.render_interpolated() - 0.02) < 1e-12,
            "visual interpolation incorrect");
    require(sim.current == authoritative, "render sampling wrote simulation state");
    Character actor;
    actor.apply_root_request(0.5, 0.3, 1); // A wall limits root delta.
    actor.apply_root_request(0.5, 0.5, 1); // Duplicate display evaluation ignored.
    require(actor.world_x == 0.3 && actor.footstep_events == 1,
            "root motion or per-tick side-effect duplicated");

    std::cout << "PASS startup rollback, input edges, request generations, "
                 "budgets, world IDs, interpolation, root-motion ownership\n";
}
