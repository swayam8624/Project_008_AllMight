// GPU transition-plan CPU oracle.
//
// Models per-subresource resource state and produces explicit transition records
// for hazards/layout/queue ownership changes. This is NOT Vulkan or Metal code;
// it is the independently testable planning layer that a native backend can
// lower to VkDependencyInfo/VkImageMemoryBarrier2 or Metal barriers/fences.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifdef assert
#undef assert
#endif
#define ALLMIGHT_REQUIRE(expr) do { if (!(expr)) throw std::runtime_error("check failed: " #expr); } while (false)
#define assert(expr) ALLMIGHT_REQUIRE(expr)

namespace allmight {

enum class Access { none, read, write, read_write };
enum class Layout { undefined, color_attachment, depth_attachment, shader_read, general, present };
enum class Queue { graphics, compute, transfer };

bool writes(Access a) { return a == Access::write || a == Access::read_write; }
bool reads(Access a) { return a == Access::read || a == Access::read_write; }

struct Subresource {
    int mip{};
    int layer{};
    int aspect{}; // 0=color, 1=depth, etc.
    auto operator<=>(const Subresource&) const = default;
};

struct State {
    Access access{Access::none};
    Layout layout{Layout::undefined};
    Queue queue{Queue::graphics};
    std::string stage{"none"};
};

struct Use {
    std::string resource;
    Subresource sub;
    Access access;
    Layout layout;
    Queue queue;
    std::string stage;
};

struct Barrier {
    std::string resource;
    Subresource sub;
    State before;
    State after;
    bool memory_hazard{};
    bool layout_transition{};
    bool queue_transfer{};
};

class Planner {
public:
    std::vector<Barrier> apply(const std::vector<Use>& uses) {
        std::vector<Barrier> out;
        for (const Use& use : uses) {
            if (use.resource.empty() || use.stage.empty()) {
                throw std::invalid_argument("resource and stage must be named");
            }
            const Key key{use.resource, use.sub};
            const auto it = states_.find(key);
            const State before = (it == states_.end()) ? State{} : it->second;
            const State after{use.access, use.layout, use.queue, use.stage};

            const bool memory =
                writes(before.access) && (reads(after.access) || writes(after.access)) ||
                reads(before.access) && writes(after.access);
            const bool layout = before.layout != after.layout;
            const bool queue = before.queue != after.queue;

            if (memory || layout || queue) {
                out.push_back({use.resource, use.sub, before, after, memory, layout, queue});
            }
            states_[key] = after;
        }
        return out;
    }

private:
    struct Key {
        std::string resource;
        Subresource sub;
        auto operator<=>(const Key&) const = default;
    };
    std::map<Key, State> states_;
};

void tests() {
    {
        Planner p;
        const auto a = p.apply({
            {"hdr", {0,0,0}, Access::write, Layout::color_attachment, Queue::graphics, "color-output"},
        });
        // Undefined->attachment needs a layout transition even without a prior memory hazard.
        assert(a.size() == 1U);
        assert(a[0].layout_transition);
        assert(!a[0].memory_hazard);

        const auto b = p.apply({
            {"hdr", {0,0,0}, Access::read, Layout::shader_read, Queue::graphics, "fragment"},
        });
        assert(b.size() == 1U);
        assert(b[0].memory_hazard);     // write -> read
        assert(b[0].layout_transition); // attachment -> shader-read
        assert(!b[0].queue_transfer);
    }

    {
        Planner p;
        (void)p.apply({
            {"history", {0,0,0}, Access::write, Layout::general, Queue::compute, "compute"},
        });
        const auto b = p.apply({
            {"history", {0,0,0}, Access::read, Layout::shader_read, Queue::graphics, "fragment"},
        });
        assert(b.size() == 1U);
        assert(b[0].memory_hazard);
        assert(b[0].layout_transition);
        assert(b[0].queue_transfer);
    }

    {
        Planner p;
        (void)p.apply({
            {"pyramid", {0,0,0}, Access::write, Layout::general, Queue::compute, "compute"},
        });
        const auto b = p.apply({
            // Different mip: independent state, so no false dependency on mip 0.
            {"pyramid", {1,0,0}, Access::read, Layout::shader_read, Queue::graphics, "fragment"},
        });
        assert(b.size() == 1U);
        assert(!b[0].memory_hazard);
        assert(b[0].layout_transition);
    }

    {
        Planner p;
        (void)p.apply({
            {"scratch", {0,0,0}, Access::read, Layout::general, Queue::compute, "compute"},
        });
        const auto b = p.apply({
            {"scratch", {0,0,0}, Access::write, Layout::general, Queue::compute, "compute"},
        });
        assert(b.size() == 1U);
        assert(b[0].memory_hazard); // WAR
        assert(!b[0].layout_transition);
    }

    {
        Planner p;
        (void)p.apply({
            {"swap", {0,0,0}, Access::write, Layout::color_attachment, Queue::graphics, "color-output"},
        });
        const auto b = p.apply({
            {"swap", {0,0,0}, Access::read, Layout::present, Queue::graphics, "present"},
        });
        assert(b.size() == 1U);
        assert(b[0].layout_transition);
    }
}

} // namespace allmight

int main() {
    allmight::tests();
    std::cout << "gpu_transition_plan_lab: PASS\n";
    return 0;
}
