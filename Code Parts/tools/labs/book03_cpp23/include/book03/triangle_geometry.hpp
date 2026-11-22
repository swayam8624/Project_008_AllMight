#pragma once

// From Geometry to Images — Chapter 03, "Shapes That Know Their Borders".
//
// P03 connects Point2 from the P02 frame laboratory to finite image pixels
// from P01. It separates an exactly correct, range-restricted integer
// orientation/containment oracle from an *uncertified* floating-point query.
//
// Coordinate contract: these are planar (x right, y up) Cartesian formulas.
// In an image (u right, v down), the numeric sign reverses for an equivalent
// physical clockwise/counterclockwise orientation. Barycentric containment
// still works if all ordered vertices and query points use one same frame.
//
// Exact oracle domain: all integer coordinates in [-1'000'000,+1'000'000].
// Each difference lies in [-2'000'000,+2'000'000]. Each product has magnitude
// <= 4e12, and their difference magnitude <= 8e12, within int64_t. Hence the
// integer predicates below are exact for ALL accepted coordinates. They are
// NOT an arbitrary-precision or generic robust floating predicate.
//
// The floating query deliberately marks near-degenerate triangles and
// near-boundary points as uncertified. Its simple scale-aware numerical guard
// is pedagogical, NOT a proven Shewchuk-style adaptive error bound.

#include "book03/frame_math.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace book03 {

struct IntegerPoint2 {
    std::int64_t x{};
    std::int64_t y{};
};

enum class ExactLocation { Interior, Edge, Exterior, Degenerate };
enum class ApproxLocation {
    Interior, Exterior, BoundaryUncertain, DegenerateOrUncertified
};

[[nodiscard]] inline std::int64_t exact_twice_area(
    IntegerPoint2 a, IntegerPoint2 b, IntegerPoint2 c) {
    constexpr std::int64_t limit = 1'000'000;
    const auto accepted = [=](IntegerPoint2 p) {
        return p.x >= -limit && p.x <= limit &&
               p.y >= -limit && p.y <= limit;
    };
    if (!accepted(a) || !accepted(b) || !accepted(c)) {
        throw std::out_of_range(
            "exact integer predicate requires coordinates in [-1e6,+1e6]");
    }
    const std::int64_t dx1 = b.x - a.x;
    const std::int64_t dy1 = b.y - a.y;
    const std::int64_t dx2 = c.x - a.x;
    const std::int64_t dy2 = c.y - a.y;
    // Bounded inputs prove both multiplications and subtraction fit int64_t.
    return dx1 * dy2 - dy1 * dx2;
}

[[nodiscard]] inline int exact_orientation(
    IntegerPoint2 a, IntegerPoint2 b, IntegerPoint2 c) {
    const std::int64_t area = exact_twice_area(a, b, c);
    return (area > 0) - (area < 0);
}

[[nodiscard]] inline ExactLocation exact_triangle_location(
    IntegerPoint2 a, IntegerPoint2 b, IntegerPoint2 c, IntegerPoint2 p) {
    const std::int64_t d = exact_twice_area(a, b, c);
    // Even a degenerate triangle must validate the query point's declared
    // exactness domain. Do not skip the p check on early-return branches.
    const std::int64_t abp = exact_twice_area(a, b, p);
    if (d == 0) {
        return ExactLocation::Degenerate;
    }
    const std::array<std::int64_t, 3> numerators{
        exact_twice_area(b, c, p),
        exact_twice_area(c, a, p),
        abp};
    bool edge = false;
    for (const std::int64_t n : numerators) {
        if ((d > 0 && n < 0) || (d < 0 && n > 0)) {
            return ExactLocation::Exterior;
        }
        edge = edge || (n == 0);
    }
    return edge ? ExactLocation::Edge : ExactLocation::Interior;
}

struct BarycentricWeights {
    double at_a{};
    double at_b{};
    double at_c{};

    [[nodiscard]] double sum() const noexcept {
        return at_a + at_b + at_c;
    }
};

struct TriangleSample {
    double twice_signed_area{};
    BarycentricWeights barycentric{};
    ApproxLocation location{ApproxLocation::DegenerateOrUncertified};

    // Weights should be treated as invalid when the triangle is marked
    // DegenerateOrUncertified. An uncertain border still has numerically
    // computed weights for visualization, but no certified edge ownership.
};

struct Triangle2 {
    Point2 a{};
    Point2 b{};
    Point2 c{};

    [[nodiscard]] static double orient(Point2 a, Point2 b, Point2 p) {
        if (!finite(a) || !finite(b) || !finite(p)) {
            throw std::invalid_argument("triangle query has nonfinite point");
        }
        const double dx1 = b.x - a.x;
        const double dy1 = b.y - a.y;
        const double dx2 = p.x - a.x;
        const double dy2 = p.y - a.y;
        const double d = dx1 * dy2 - dy1 * dx2;
        if (!std::isfinite(d)) {
            throw std::overflow_error("triangle determinant is nonfinite");
        }
        return d;
    }

    // This is only a heuristic *uncertified* guard, never a complete
    // roundoff-proof predicate. It is useful to show which decisions demand
    // a more rigorous numerical method in the later robust-predicate chapter.
    [[nodiscard]] static double numerical_guard(
        Point2 a, Point2 b, Point2 p) {
        if (!finite(a) || !finite(b) || !finite(p)) {
            throw std::invalid_argument("nonfinite point in area guard");
        }
        const double q = std::abs((b.x - a.x) * (p.y - a.y));
        const double r = std::abs((b.y - a.y) * (p.x - a.x));
        const double magnitude = q + r;
        if (!std::isfinite(magnitude)) {
            throw std::overflow_error("triangle area guard overflow");
        }
        return 64.0 * std::numeric_limits<double>::epsilon()
               * std::max(1.0, magnitude);
    }

    [[nodiscard]] double twice_signed_area() const {
        return orient(a, b, c);
    }

    [[nodiscard]] TriangleSample sample(Point2 p) const {
        const double d = orient(a, b, c);
        const double d_guard = numerical_guard(a, b, c);
        if (std::abs(d) <= d_guard) {
            return {d, {}, ApproxLocation::DegenerateOrUncertified};
        }

        const std::array<double, 3> n{
            orient(b, c, p),
            orient(c, a, p),
            orient(a, b, p)};
        const std::array<double, 3> guards{
            numerical_guard(b, c, p),
            numerical_guard(c, a, p),
            numerical_guard(a, b, p)};

        const BarycentricWeights w{n[0] / d, n[1] / d, n[2] / d};
        if (!std::isfinite(w.at_a) || !std::isfinite(w.at_b) ||
            !std::isfinite(w.at_c)) {
            throw std::overflow_error("barycentric weight overflow");
        }

        bool near_edge = false;
        for (std::size_t i = 0; i < n.size(); ++i) {
            near_edge = near_edge || (std::abs(n[i]) <= guards[i]);
            if ((d > 0 && n[i] < -guards[i]) ||
                (d < 0 && n[i] > guards[i])) {
                return {d, w, ApproxLocation::Exterior};
            }
        }
        return {d, w, near_edge ?
            ApproxLocation::BoundaryUncertain : ApproxLocation::Interior};
    }

    [[nodiscard]] static double interpolate(BarycentricWeights weights,
                                            std::array<double, 3> values) {
        if (!std::isfinite(weights.at_a) ||
            !std::isfinite(weights.at_b) ||
            !std::isfinite(weights.at_c) ||
            !std::all_of(values.begin(), values.end(),
                         [](double v) { return std::isfinite(v); })) {
            throw std::invalid_argument("interpolation requires finite values");
        }
        const double interpolated = weights.at_a * values[0] +
                                    weights.at_b * values[1] +
                                    weights.at_c * values[2];
        if (!std::isfinite(interpolated)) {
            throw std::overflow_error("interpolated value is nonfinite");
        }
        return interpolated;
    }
};

} // namespace book03
