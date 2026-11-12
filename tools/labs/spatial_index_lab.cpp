// PROJECTALLMIGHT Phase 14 witness: BVH construction/traversal and A* graph search with reference checks.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

[[noreturn]] void fail(const std::string& m) { throw std::runtime_error(m); }
void require(bool c, const std::string& m) { if (!c) fail(m); }

struct Vec3 {
    float x = 0, y = 0, z = 0;
    float operator[](int axis) const {
        if (axis == 0) return x;
        if (axis == 1) return y;
        return z;
    }
};

struct Aabb {
    Vec3 lo{
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::infinity()};
    Vec3 hi{
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity()};

    void expand(const Aabb& b) {
        lo.x = std::min(lo.x, b.lo.x); lo.y = std::min(lo.y, b.lo.y); lo.z = std::min(lo.z, b.lo.z);
        hi.x = std::max(hi.x, b.hi.x); hi.y = std::max(hi.y, b.hi.y); hi.z = std::max(hi.z, b.hi.z);
    }

    Vec3 centroid() const {
        return {(lo.x + hi.x) * 0.5F, (lo.y + hi.y) * 0.5F, (lo.z + hi.z) * 0.5F};
    }

    bool overlaps(const Aabb& b) const {
        return lo.x <= b.hi.x && hi.x >= b.lo.x &&
               lo.y <= b.hi.y && hi.y >= b.lo.y &&
               lo.z <= b.hi.z && hi.z >= b.lo.z;
    }
};

struct Primitive {
    Aabb bounds;
    std::uint32_t id = 0;
};

struct Node {
    Aabb bounds;
    int left = -1;
    int right = -1;
    std::uint32_t begin = 0;
    std::uint32_t count = 0;

    bool leaf() const { return count != 0; }
};

class Bvh {
public:
    explicit Bvh(std::vector<Primitive> primitives) : primitives_(std::move(primitives)) {
        indices_.resize(primitives_.size());
        for (std::size_t i = 0; i < indices_.size(); ++i) indices_[i] = static_cast<std::uint32_t>(i);
        if (!indices_.empty()) root_ = build(0, static_cast<std::uint32_t>(indices_.size()));
    }

    std::vector<std::uint32_t> query(const Aabb& q) const {
        std::vector<std::uint32_t> hits;
        if (root_ < 0) return hits;

        std::vector<int> stack{root_};
        while (!stack.empty()) {
            const int ni = stack.back();
            stack.pop_back();
            const Node& n = nodes_[static_cast<std::size_t>(ni)];
            if (!n.bounds.overlaps(q)) continue;

            if (n.leaf()) {
                for (std::uint32_t k = 0; k < n.count; ++k) {
                    const Primitive& p = primitives_[indices_[n.begin + k]];
                    if (p.bounds.overlaps(q)) hits.push_back(p.id);
                }
            } else {
                stack.push_back(n.left);
                stack.push_back(n.right);
            }
        }
        std::sort(hits.begin(), hits.end());
        return hits;
    }

private:
    std::vector<Primitive> primitives_;
    std::vector<std::uint32_t> indices_;
    std::vector<Node> nodes_;
    int root_ = -1;

    Aabb bounds_for(std::uint32_t begin, std::uint32_t end) const {
        Aabb b;
        for (std::uint32_t i = begin; i < end; ++i) b.expand(primitives_[indices_[i]].bounds);
        return b;
    }

    int build(std::uint32_t begin, std::uint32_t end) {
        require(begin < end, "empty BVH build range");
        Node node;
        node.bounds = bounds_for(begin, end);
        const std::uint32_t count = end - begin;

        const int index = static_cast<int>(nodes_.size());
        nodes_.push_back(node);

        if (count <= 4) {
            nodes_[static_cast<std::size_t>(index)].begin = begin;
            nodes_[static_cast<std::size_t>(index)].count = count;
            return index;
        }

        Aabb cb;
        for (std::uint32_t i = begin; i < end; ++i) {
            const Vec3 c = primitives_[indices_[i]].bounds.centroid();
            cb.expand(Aabb{c, c});
        }
        const Vec3 extent{cb.hi.x - cb.lo.x, cb.hi.y - cb.lo.y, cb.hi.z - cb.lo.z};
        int axis = 0;
        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;

        const std::uint32_t mid = begin + count / 2;
        std::nth_element(
            indices_.begin() + begin,
            indices_.begin() + mid,
            indices_.begin() + end,
            [&](std::uint32_t a, std::uint32_t b) {
                return primitives_[a].bounds.centroid()[axis] < primitives_[b].bounds.centroid()[axis];
            });

        nodes_[static_cast<std::size_t>(index)].left = build(begin, mid);
        nodes_[static_cast<std::size_t>(index)].right = build(mid, end);
        return index;
    }
};

std::vector<std::uint32_t> brute_query(const std::vector<Primitive>& primitives, const Aabb& q) {
    std::vector<std::uint32_t> hits;
    for (const Primitive& p : primitives) if (p.bounds.overlaps(q)) hits.push_back(p.id);
    std::sort(hits.begin(), hits.end());
    return hits;
}

struct Edge { int to = -1; float cost = 0; };
using Graph = std::vector<std::vector<Edge>>;

std::vector<int> astar(
    const Graph& graph,
    int start,
    int goal,
    const std::vector<float>& heuristic) {

    require(start >= 0 && goal >= 0 &&
            static_cast<std::size_t>(start) < graph.size() &&
            static_cast<std::size_t>(goal) < graph.size(), "A* endpoint out of range");
    require(heuristic.size() == graph.size(), "A* heuristic size mismatch");

    struct QueueItem {
        float f = 0;
        int node = -1;
        bool operator>(const QueueItem& other) const { return f > other.f; }
    };

    const float inf = std::numeric_limits<float>::infinity();
    std::vector<float> g(graph.size(), inf);
    std::vector<int> parent(graph.size(), -1);
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> open;

    g[start] = 0.0F;
    open.push({heuristic[start], start});

    while (!open.empty()) {
        const QueueItem item = open.top();
        open.pop();

        // No permanent closed set: a node can be reinserted if a better path is found.
        if (item.f > g[item.node] + heuristic[item.node] + 1.0e-6F) continue;
        if (item.node == goal) break;

        for (const Edge e : graph[item.node]) {
            require(e.to >= 0 && static_cast<std::size_t>(e.to) < graph.size(), "A* edge target out of range");
            require(e.cost >= 0.0F && std::isfinite(e.cost), "A* requires finite nonnegative edge costs");
            const float candidate = g[item.node] + e.cost;
            if (candidate < g[e.to]) {
                g[e.to] = candidate;
                parent[e.to] = item.node;
                open.push({candidate + heuristic[e.to], e.to});
            }
        }
    }

    if (!std::isfinite(g[goal])) return {};
    std::vector<int> path;
    for (int n = goal; n != -1; n = parent[n]) path.push_back(n);
    std::reverse(path.begin(), path.end());
    return path;
}

void test_bvh_against_brute_force() {
    std::mt19937 rng(12345);
    std::uniform_real_distribution<float> center(-10.0F, 10.0F);
    std::uniform_real_distribution<float> half(0.05F, 1.0F);

    std::vector<Primitive> primitives;
    for (std::uint32_t id = 0; id < 200; ++id) {
        const Vec3 c{center(rng), center(rng), center(rng)};
        const Vec3 h{half(rng), half(rng), half(rng)};
        primitives.push_back({{{c.x-h.x,c.y-h.y,c.z-h.z},{c.x+h.x,c.y+h.y,c.z+h.z}}, id});
    }

    const Bvh bvh(primitives);
    for (int qi = 0; qi < 250; ++qi) {
        const Vec3 c{center(rng), center(rng), center(rng)};
        const Vec3 h{half(rng) * 2.0F, half(rng) * 2.0F, half(rng) * 2.0F};
        const Aabb q{{c.x-h.x,c.y-h.y,c.z-h.z},{c.x+h.x,c.y+h.y,c.z+h.z}};
        require(bvh.query(q) == brute_query(primitives, q), "BVH query disagrees with brute-force oracle");
    }
}

void test_astar() {
    Graph g(6);
    auto add = [&](int a, int b, float c) { g[a].push_back({b,c}); };
    add(0,1,2); add(0,2,5);
    add(1,2,1); add(1,3,4);
    add(2,3,1); add(3,4,3);
    add(2,5,10); add(4,5,1);

    const std::vector<float> zero_h(g.size(), 0.0F);
    const auto path = astar(g, 0, 5, zero_h);
    require(path == std::vector<int>({0,1,2,3,4,5}), "A* zero-heuristic path should match Dijkstra optimum");

    Graph disconnected(2);
    require(astar(disconnected, 0, 1, {0,0}).empty(), "A* must report unreachable goal");
}

} // namespace

int main() {
    try {
        test_bvh_against_brute_force();
        test_astar();
        std::cout
            << "spatial_index_lab: PASS\n"
            << "proved: median-split BVH overlap parity against brute force over deterministic random workloads, "
               "plus A* relaxation/reinsertion and unreachable handling\n"
            << "not proved: SAH/LBVH quality, dynamic-tree updates, SIMD traversal, ray packets, navmesh construction, "
               "admissible-heuristic performance, or production KairoSpatial parity\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "spatial_index_lab: FAIL: " << e.what() << '\n';
        return 1;
    }
}
