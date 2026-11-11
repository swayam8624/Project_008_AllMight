// Executable witnesses for four first-use teaching bridges.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror
//        tools/labs/first_use_contract_lab.cpp -o /tmp/first_use && /tmp/first_use
#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

struct Vec2 { double x, y; };
struct Coordinates { double first, second, residual; };

// Solve [b1 b2] * c = world without constructing the inverse matrix.
// This is a teaching-sized 2x2 pivoted elimination, not a general LAPACK replacement.
std::optional<Coordinates> recover_coordinates(Vec2 b1, Vec2 b2, Vec2 world) {
    if (!std::isfinite(b1.x) || !std::isfinite(b1.y) ||
        !std::isfinite(b2.x) || !std::isfinite(b2.y) ||
        !std::isfinite(world.x) || !std::isfinite(world.y)) return std::nullopt;
    double a[2][3] = {{b1.x, b2.x, world.x}, {b1.y, b2.y, world.y}};
    const double scale = std::max({std::abs(b1.x), std::abs(b1.y),
                                   std::abs(b2.x), std::abs(b2.y)});
    if (scale == 0 || !std::isfinite(scale)) return std::nullopt;
    // The threshold is relative to the input matrix, not a fixed world-space distance.
    const double threshold = 32 * std::numeric_limits<double>::epsilon() * scale;
    for (int col = 0; col != 2; ++col) {
        int pivot = (std::abs(a[1][col]) > std::abs(a[col][col])) ? 1 : col;
        // Only row 1 can replace row 0 on the first pivot. At col 1, pivot stays 1.
        if (col == 1) pivot = 1;
        if (!(std::abs(a[pivot][col]) > threshold)) return std::nullopt;
        if (pivot != col) for (int j = 0; j != 3; ++j) std::swap(a[col][j], a[pivot][j]);
        if (col == 0) {
            const double m = a[1][0] / a[0][0];
            a[1][1] -= m * a[0][1];
            a[1][2] -= m * a[0][2];
            a[1][0] = 0;
        }
    }
    const double second = a[1][2] / a[1][1];
    const double first = (a[0][2] - a[0][1] * second) / a[0][0];
    const double dx = b1.x * first + b2.x * second - world.x;
    const double dy = b1.y * first + b2.y * second - world.y;
    if (!std::isfinite(first) || !std::isfinite(second) ||
        !std::isfinite(dx) || !std::isfinite(dy)) return std::nullopt;
    return Coordinates{first, second, std::hypot(dx, dy)};
}

struct Item { std::uint32_t key; std::uint32_t identity; };

// Stable two-bit radix sort. Only pointers are swapped: no in-place scatter.
template <std::size_t N>
std::array<Item, N> stable_two_bit_radix(std::array<Item, N> initial) {
    std::array<Item, N> scratch{};
    auto* read = &initial;
    auto* write = &scratch;
    for (unsigned bit = 0; bit != 2; ++bit) {
        std::size_t zeros = 0;
        for (const Item& item : *read) zeros += ((item.key >> bit) & 1u) == 0u;
        std::size_t next_zero = 0, next_one = zeros;
        for (const Item& item : *read) {
            // Each item has one disjoint destination. Relative order within a bin survives.
            const bool one = ((item.key >> bit) & 1u) != 0u;
            (*write)[one ? next_one++ : next_zero++] = item;
        }
        std::swap(read, write);
    }
    return *read; // This also works for an odd number of passes if the loop changes.
}

struct Splat { double q, opacity; std::array<double, 3> color; };
struct Composite { std::array<double, 3> color; double transmittance; };

// Input splats must already be sorted front-to-back in one declared depth convention.
// Color is linear and premultiplied into the accumulated result by alpha at each step.
std::optional<Composite> composite_splats(const Splat* splats, std::size_t count) {
    Composite out{{0, 0, 0}, 1};
    for (std::size_t i = 0; i < count; ++i) {
        const Splat& s = splats[i];
        if (!std::isfinite(s.q) || s.q < 0 || !std::isfinite(s.opacity) ||
            s.opacity < 0 || s.opacity > 1) return std::nullopt;
        for (double component : s.color)
            if (!std::isfinite(component) || component < 0 || component > 1)
                return std::nullopt;
        if (s.q > 9) continue; // Deliberate truncated footprint, not zero Gaussian support.
        const double alpha = std::clamp(s.opacity * std::exp(-0.5 * s.q), 0.0, 0.999);
        for (int ch = 0; ch != 3; ++ch)
            out.color[ch] += out.transmittance * alpha * s.color[ch];
        out.transmittance *= (1.0 - alpha);
        if (out.transmittance < 1e-4) break; // Bounded omitted color under [0,1] assumption.
    }
    return out; // Background, if any, adds T * background in the same linear space.
}

// A deterministic quadrature of the expectation of a two-technique MIS estimator.
// Both PDFs have full support in the open interval (0,1); f(x) = x^2.
double expected_mis_integral() {
    constexpr int slices = 10000;
    double total = 0;
    for (int i = 0; i < slices; ++i) {
        const double x = (i + 0.5) / slices;
        const double pa = 2 * x, pb = 2 * (1 - x);
        const double wa = (pa * pa) / (pa * pa + pb * pb);
        const double wb = 1 - wa;
        const double f = x * x;
        // p_a * (w_a f / p_a) + p_b * (w_b f / p_b): the two expectations.
        total += (pa * (wa * f / pa) + pb * (wb * f / pb)) / slices;
    }
    return total;
}

int main() {
    const auto c = recover_coordinates({1, 0}, {1, 1}, {3, 2});
    assert(c && std::abs(c->first - 1) < 1e-12 &&
           std::abs(c->second - 2) < 1e-12 && c->residual < 1e-12);
    assert(!recover_coordinates({1, 0}, {1, 0}, {3, 2}));

    auto sorted = stable_two_bit_radix<5>({{{3, 0}, {1, 1}, {2, 2}, {0, 3}, {2, 4}}});
    const std::array<std::uint32_t, 5> expected_keys{0, 1, 2, 2, 3};
    for (std::size_t i = 0; i < sorted.size(); ++i) assert(sorted[i].key == expected_keys[i]);
    assert(sorted[2].identity == 2 && sorted[3].identity == 4); // Stability.

    const std::array<Splat, 2> front_to_back{{{0, 0.6, {1, 0, 0}},
                                              {0, 0.5, {0, 0, 1}}}};
    const auto composed = composite_splats(front_to_back.data(), front_to_back.size());
    assert(composed && std::abs(composed->color[0] - 0.6) < 1e-12 &&
           std::abs(composed->color[2] - 0.2) < 1e-12 &&
           std::abs(composed->transmittance - 0.2) < 1e-12);
    assert(std::abs(expected_mis_integral() - 1.0 / 3.0) < 1e-8);
}
