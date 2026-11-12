// Spatial acceleration reference lab: AABBs, median and binned-SAH BVH
// construction, overlap traversal and brute-force differential testing.

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight {

struct Vec3 {
    float x{}, y{}, z{};
    float operator[](int a) const { return a == 0 ? x : (a == 1 ? y : z); }
};

struct Aabb {
    Vec3 lo{ std::numeric_limits<float>::infinity(),
             std::numeric_limits<float>::infinity(),
             std::numeric_limits<float>::infinity() };
    Vec3 hi{ -std::numeric_limits<float>::infinity(),
             -std::numeric_limits<float>::infinity(),
             -std::numeric_limits<float>::infinity() };

    void expand(Vec3 p) {
        lo.x = std::min(lo.x, p.x); lo.y = std::min(lo.y, p.y); lo.z = std::min(lo.z, p.z);
        hi.x = std::max(hi.x, p.x); hi.y = std::max(hi.y, p.y); hi.z = std::max(hi.z, p.z);
    }
    void expand(const Aabb& b) { expand(b.lo); expand(b.hi); }

    Vec3 centroid() const {
        return {(lo.x + hi.x) * 0.5F, (lo.y + hi.y) * 0.5F, (lo.z + hi.z) * 0.5F};
    }

    float area() const {
        const float dx = std::max(0.0F, hi.x - lo.x);
        const float dy = std::max(0.0F, hi.y - lo.y);
        const float dz = std::max(0.0F, hi.z - lo.z);
        return 2.0F * (dx * dy + dx * dz + dy * dz);
    }

    bool overlaps(const Aabb& b) const {
        return lo.x <= b.hi.x && hi.x >= b.lo.x &&
               lo.y <= b.hi.y && hi.y >= b.lo.y &&
               lo.z <= b.hi.z && hi.z >= b.lo.z;
    }
};

struct Primitive {
    Aabb bounds;
    int id{};
};

struct Node {
    Aabb bounds;
    int left = -1;
    int right = -1;
    std::size_t first = 0;
    std::size_t count = 0;

    bool leaf() const { return left < 0 && right < 0; }
};

enum class SplitMode { median, binned_sah };

class Bvh {
public:
    Bvh(std::vector<Primitive> primitives, SplitMode mode)
        : primitives_(std::move(primitives)), mode_(mode) {
        order_.resize(primitives_.size());
        std::iota(order_.begin(), order_.end(), std::size_t{0});
        if (!order_.empty()) root_ = build(0, order_.size());
    }

    std::vector<int> query(const Aabb& q) const {
        std::vector<int> hits;
        if (root_ < 0) return hits;
        std::vector<int> stack{root_};
        while (!stack.empty()) {
            const int ni = stack.back();
            stack.pop_back();
            const Node& n = nodes_[static_cast<std::size_t>(ni)];
            if (!n.bounds.overlaps(q)) continue;
            if (n.leaf()) {
                for (std::size_t i = 0; i < n.count; ++i) {
                    const Primitive& p = primitives_[order_[n.first + i]];
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

    void validate() const {
        if (root_ < 0) return;
        validate_node(root_);
    }

private:
    Aabb range_bounds(std::size_t begin, std::size_t end) const {
        Aabb b;
        for (std::size_t i = begin; i < end; ++i) b.expand(primitives_[order_[i]].bounds);
        return b;
    }

    Aabb centroid_bounds(std::size_t begin, std::size_t end) const {
        Aabb b;
        for (std::size_t i = begin; i < end; ++i) b.expand(primitives_[order_[i]].bounds.centroid());
        return b;
    }

    int widest_axis(const Aabb& b) const {
        const float ex = b.hi.x - b.lo.x;
        const float ey = b.hi.y - b.lo.y;
        const float ez = b.hi.z - b.lo.z;
        if (ex >= ey && ex >= ez) return 0;
        return ey >= ez ? 1 : 2;
    }

    std::size_t median_split(std::size_t begin, std::size_t end, int axis) {
        const std::size_t mid = begin + (end - begin) / 2U;
        std::nth_element(order_.begin() + static_cast<std::ptrdiff_t>(begin),
                         order_.begin() + static_cast<std::ptrdiff_t>(mid),
                         order_.begin() + static_cast<std::ptrdiff_t>(end),
                         [&](std::size_t a, std::size_t b) {
                             return primitives_[a].bounds.centroid()[axis] <
                                    primitives_[b].bounds.centroid()[axis];
                         });
        return mid;
    }

    std::size_t sah_split(std::size_t begin, std::size_t end, int axis, const Aabb& cb) {
        constexpr int bins_n = 8;
        struct Bin { Aabb b; std::size_t n = 0; };
        std::array<Bin, bins_n> bins{};

        const float minc = cb.lo[axis];
        const float maxc = cb.hi[axis];
        const float extent = maxc - minc;
        if (!(extent > 1.0e-8F)) return median_split(begin, end, axis);

        auto bin_of = [&](float c) {
            int b = static_cast<int>(((c - minc) / extent) * static_cast<float>(bins_n));
            return std::clamp(b, 0, bins_n - 1);
        };

        for (std::size_t i = begin; i < end; ++i) {
            const Primitive& p = primitives_[order_[i]];
            Bin& bin = bins[static_cast<std::size_t>(bin_of(p.bounds.centroid()[axis]))];
            ++bin.n;
            bin.b.expand(p.bounds);
        }

        std::array<Aabb, bins_n> left_b{}, right_b{};
        std::array<std::size_t, bins_n> left_n{}, right_n{};
        Aabb acc_b;
        std::size_t acc_n = 0;
        for (int i = 0; i < bins_n; ++i) {
            if (bins[static_cast<std::size_t>(i)].n) acc_b.expand(bins[static_cast<std::size_t>(i)].b);
            acc_n += bins[static_cast<std::size_t>(i)].n;
            left_b[static_cast<std::size_t>(i)] = acc_b;
            left_n[static_cast<std::size_t>(i)] = acc_n;
        }
        acc_b = Aabb{};
        acc_n = 0;
        for (int i = bins_n - 1; i >= 0; --i) {
            if (bins[static_cast<std::size_t>(i)].n) acc_b.expand(bins[static_cast<std::size_t>(i)].b);
            acc_n += bins[static_cast<std::size_t>(i)].n;
            right_b[static_cast<std::size_t>(i)] = acc_b;
            right_n[static_cast<std::size_t>(i)] = acc_n;
        }

        float best = std::numeric_limits<float>::infinity();
        int best_boundary = -1;
        for (int i = 0; i < bins_n - 1; ++i) {
            const std::size_t ln = left_n[static_cast<std::size_t>(i)];
            const std::size_t rn = right_n[static_cast<std::size_t>(i + 1)];
            if (ln == 0 || rn == 0) continue;
            const float cost = left_b[static_cast<std::size_t>(i)].area() * static_cast<float>(ln) +
                               right_b[static_cast<std::size_t>(i + 1)].area() * static_cast<float>(rn);
            if (cost < best) { best = cost; best_boundary = i; }
        }

        if (best_boundary < 0) return median_split(begin, end, axis);
        const auto mid_it = std::partition(
            order_.begin() + static_cast<std::ptrdiff_t>(begin),
            order_.begin() + static_cast<std::ptrdiff_t>(end),
            [&](std::size_t pi) {
                return bin_of(primitives_[pi].bounds.centroid()[axis]) <= best_boundary;
            });
        const std::size_t mid = static_cast<std::size_t>(mid_it - order_.begin());
        if (mid == begin || mid == end) return median_split(begin, end, axis);
        return mid;
    }

    int build(std::size_t begin, std::size_t end) {
        const int index = static_cast<int>(nodes_.size());
        nodes_.push_back(Node{});
        nodes_[static_cast<std::size_t>(index)].bounds = range_bounds(begin, end);

        const std::size_t count = end - begin;
        if (count <= 4) {
            nodes_[static_cast<std::size_t>(index)].first = begin;
            nodes_[static_cast<std::size_t>(index)].count = count;
            return index;
        }

        const Aabb cb = centroid_bounds(begin, end);
        const int axis = widest_axis(cb);
        const std::size_t mid = mode_ == SplitMode::binned_sah
            ? sah_split(begin, end, axis, cb)
            : median_split(begin, end, axis);

        const int left = build(begin, mid);
        const int right = build(mid, end);
        nodes_[static_cast<std::size_t>(index)].left = left;
        nodes_[static_cast<std::size_t>(index)].right = right;
        return index;
    }

    void validate_node(int index) const {
        const Node& n = nodes_.at(static_cast<std::size_t>(index));
        if (n.leaf()) {
            assert(n.count > 0);
            for (std::size_t i = 0; i < n.count; ++i) {
                assert(n.bounds.overlaps(primitives_[order_[n.first + i]].bounds));
            }
            return;
        }
        assert(n.left >= 0 && n.right >= 0);
        const Node& l = nodes_[static_cast<std::size_t>(n.left)];
        const Node& r = nodes_[static_cast<std::size_t>(n.right)];
        assert(n.bounds.overlaps(l.bounds));
        assert(n.bounds.overlaps(r.bounds));
        validate_node(n.left);
        validate_node(n.right);
    }

    std::vector<Primitive> primitives_;
    std::vector<std::size_t> order_;
    std::vector<Node> nodes_;
    SplitMode mode_;
    int root_ = -1;
};

Aabb box(float x, float y, float z, float r) {
    Aabb b;
    b.expand({x-r,y-r,z-r});
    b.expand({x+r,y+r,z+r});
    return b;
}

std::vector<int> brute(const std::vector<Primitive>& p, const Aabb& q) {
    std::vector<int> hits;
    for (const Primitive& x : p) if (x.bounds.overlaps(q)) hits.push_back(x.id);
    std::sort(hits.begin(), hits.end());
    return hits;
}

void tests() {
    std::mt19937 rng(123456U);
    std::uniform_real_distribution<float> pos(-50.0F, 50.0F);
    std::uniform_real_distribution<float> radius(0.05F, 2.0F);

    std::vector<Primitive> primitives;
    for (int i = 0; i < 300; ++i) {
        primitives.push_back({box(pos(rng), pos(rng), pos(rng), radius(rng)), i});
    }

    Bvh median(primitives, SplitMode::median);
    Bvh sah(primitives, SplitMode::binned_sah);
    median.validate();
    sah.validate();

    for (int i = 0; i < 1000; ++i) {
        const Aabb q = box(pos(rng), pos(rng), pos(rng), radius(rng) * 2.0F);
        const auto expected = brute(primitives, q);
        assert(median.query(q) == expected);
        assert(sah.query(q) == expected);
    }
}

} // namespace allmight

int main() {
    allmight::tests();
    std::cout << "spatial_bvh_lab: PASS\n";
    return 0;
}
