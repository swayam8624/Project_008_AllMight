// Book 03 / P03 — independent bounded exact geometry witnesses.
//
// The exact oracle accepts integer coordinates in [-1e6,+1e6] and uses
// overflow-proven int64 arithmetic. A different shoelace formula supplies
// its reference value. The floating classification is intentionally NOT
// advertised as certified across all binary64 inputs. Near edges it can
// respond "uncertain", which is a necessary part of this teaching design.

#include "book03/triangle_geometry.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::ApproxLocation;
using book03::BarycentricWeights;
using book03::ExactLocation;
using book03::Frame2;
using book03::IntegerPoint2;
using book03::Point2;
using book03::Triangle2;

void require(bool condition, std::string_view message) {
    if (!condition) throw std::runtime_error(std::string{message});
}

void near(double actual, double expected, double tolerance,
          std::string_view message) {
    if (!(std::isfinite(actual) &&
          std::abs(actual - expected) <= tolerance)) {
        throw std::runtime_error(std::string{message});
    }
}

template <typename Exception, typename Fn>
void must_throw(Fn&& fn, std::string_view message) {
    try {
        fn();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(std::string{message});
}

// Different expression from the determinant in triangle_geometry.hpp.
// All terms remain bounded by a few trillion inside the accepted domain.
std::int64_t shoelace_reference(IntegerPoint2 a, IntegerPoint2 b,
                                IntegerPoint2 c) {
    return a.x * (b.y - c.y) +
           b.x * (c.y - a.y) +
           c.x * (a.y - b.y);
}

void exact_orientation_and_area() {
    constexpr IntegerPoint2 a{0, 0}, b{6, 0}, c{0, 4};
    require(book03::exact_twice_area(a,b,c) == 24,
            "6x4 right triangle exact twice area");
    require(book03::exact_twice_area(a,c,b) == -24,
            "winding reversal must negate area");
    require(book03::exact_orientation(a,b,c) == 1,
            "positive counterclockwise winding in y-up frame");
    require(book03::exact_orientation(a,c,b) == -1,
            "negative clockwise winding in y-up frame");
    require(book03::exact_orientation({0,0},{2,2},{4,4}) == 0,
            "collinear points are exactly zero");
    require(book03::exact_twice_area(a,b,c) == shoelace_reference(a,b,c),
            "independent shoelace expression");

    constexpr IntegerPoint2 left{-1'000'000,-1'000'000};
    constexpr IntegerPoint2 right{1'000'000,1'000'000};
    constexpr IntegerPoint2 top{1'000'000,-1'000'000};
    require(book03::exact_twice_area(left,right,top) ==
            shoelace_reference(left,right,top),
            "domain endpoints remain exact without overflow");

    must_throw<std::out_of_range>(
        [] { (void)book03::exact_twice_area(
            {0,0},{1'000'001,0},{0,1}); },
        "exactness domain must reject oversized coordinates");
    must_throw<std::out_of_range>(
        [] { (void)book03::exact_orientation(
            {0,0},{1,0},{0,-1'000'001}); },
        "negative out-of-range coordinate rejected");
}

void exact_containment() {
    constexpr IntegerPoint2 a{0, 0}, b{6, 0}, c{0, 4};
    require(book03::exact_triangle_location(a,b,c,{1,1}) ==
            ExactLocation::Interior, "interior exact integer point");
    require(book03::exact_triangle_location(a,b,c,{3,0}) ==
            ExactLocation::Edge, "point on AB belongs to boundary");
    require(book03::exact_triangle_location(a,b,c,{0,0}) ==
            ExactLocation::Edge, "triangle vertex is boundary");
    require(book03::exact_triangle_location(a,b,c,{5,3}) ==
            ExactLocation::Exterior, "exterior integer point");
    require(book03::exact_triangle_location(a,c,b,{1,1}) ==
            ExactLocation::Interior, "winding-independent interior");
    require(book03::exact_triangle_location(a,c,b,{5,3}) ==
            ExactLocation::Exterior, "winding-independent exterior");
    require(book03::exact_triangle_location(
            {0,0},{2,2},{4,4},{1,1}) ==
            ExactLocation::Degenerate, "no interior for collapsed triangle");
    must_throw<std::out_of_range>(
        [] { (void)book03::exact_triangle_location(
            {0,0},{2,2},{4,4},{1'000'001,0}); },
        "even a degenerate triangle rejects out-of-domain query point");
}

void exact_property_sweep() {
    // Deterministic bounded integer inputs, not empirical benchmark scenes.
    std::uint32_t state = 0x617EA123u;
    auto next = [&]() -> std::int64_t {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return static_cast<std::int64_t>(state % 1'900'001u) - 950'000;
    };

    for (int iteration = 0; iteration < 4000; ++iteration) {
        const IntegerPoint2 a{next(),next()};
        const IntegerPoint2 b{next(),next()};
        const IntegerPoint2 c{next(),next()};
        const std::int64_t expected = shoelace_reference(a,b,c);
        const std::int64_t actual = book03::exact_twice_area(a,b,c);
        require(actual == expected, "independent integer shoelace oracle");
        require(book03::exact_twice_area(a,c,b) == -actual,
                "orientation antisymmetry");
        // The source values are generated with 50k margin, so the translated
        // points remain inside the *same* exact-coordinate domain.
        const IntegerPoint2 a2{a.x+5000,a.y-5000};
        const IntegerPoint2 b2{b.x+5000,b.y-5000};
        const IntegerPoint2 c2{c.x+5000,c.y-5000};
        require(book03::exact_twice_area(a2,b2,c2) == actual,
                "integer oriented area is translation invariant");
        const IntegerPoint2 ar{a.x,-a.y};
        const IntegerPoint2 br{b.x,-b.y};
        const IntegerPoint2 cr{c.x,-c.y};
        require(book03::exact_twice_area(ar,br,cr) == -actual,
                "reflection negates oriented area");
    }
}

void floating_barycentric_and_interpolation() {
    const Triangle2 triangle{{0,0},{6,0},{0,4}};
    const auto sample = triangle.sample({1.5,1});
    require(sample.location == ApproxLocation::Interior,
            "numerically well-separated interior");
    near(sample.twice_signed_area,24,0.0,
         "exact binary64 simple triangle area");
    near(sample.barycentric.at_a,0.5,0.0,"weight at A");
    near(sample.barycentric.at_b,0.25,0.0,"weight at B");
    near(sample.barycentric.at_c,0.25,0.0,"weight at C");
    near(sample.barycentric.sum(),1.0,0.0,"barycentric partition of unity");
    near(Triangle2::interpolate(sample.barycentric,{10,20,30}),
         17.5,0.0,"scalar interpolation");

    const auto outside = triangle.sample({5,3});
    require(outside.location == ApproxLocation::Exterior,
            "clearly exterior sample");
    near(outside.barycentric.at_a,-7.0/12.0,1e-14,
         "exterior negative barycentric weight");
    near(outside.barycentric.sum(),1.0,1e-14,
         "exterior weights still sum to one");

    const auto edge = triangle.sample({3,0});
    require(edge.location == ApproxLocation::BoundaryUncertain,
            "floating border is not certified exact");
    near(edge.barycentric.at_c,0.0,0.0,"AB edge zero C weight");

    const Triangle2 reversed{triangle.a,triangle.c,triangle.b};
    const auto other = reversed.sample({1.5,1});
    require(other.location == ApproxLocation::Interior,
            "clockwise triangle still has interior");
    near(other.twice_signed_area,-24.0,0.0,"winding reversal");
    near(other.barycentric.at_a,sample.barycentric.at_a,0.0,
         "same weight at unchanged A vertex");
    near(other.barycentric.at_b,sample.barycentric.at_c,0.0,
         "B/C swap carries vertex weight identity");

    const Triangle2 collapsed{{0,0},{2,2},{4,4}};
    require(collapsed.sample({1,1}).location ==
            ApproxLocation::DegenerateOrUncertified,
            "collapsed triangle may not invent barycentric inverse");
}

void affine_invariance() {
    const Triangle2 source{{0,0},{6,0},{0,4}};
    const Point2 p{1.5,1};
    const Frame2 turned = Frame2::rotation(
        {3,-2},book03::degrees_to_radians(41));
    const Triangle2 moved{
        turned.local_to_world(source.a),
        turned.local_to_world(source.b),
        turned.local_to_world(source.c)};
    const auto old = source.sample(p);
    const auto changed = moved.sample(turned.local_to_world(p));
    require(changed.location == ApproxLocation::Interior,
            "transformed interior remains interior");
    near(changed.barycentric.at_a,old.barycentric.at_a,1e-12,
         "affine-invariant A weight");
    near(changed.barycentric.at_b,old.barycentric.at_b,1e-12,
         "affine-invariant B weight");
    near(changed.barycentric.at_c,old.barycentric.at_c,1e-12,
         "affine-invariant C weight");

    // The determinant of a reflection is negative; relative weights should
    // be unchanged even though the signed area switches sign.
    const Triangle2 reflected{
        {source.a.x,-source.a.y},
        {source.b.x,-source.b.y},
        {source.c.x,-source.c.y}};
    const auto flipped = reflected.sample({p.x,-p.y});
    require(flipped.location == ApproxLocation::Interior,
            "reflected triangle still has an interior");
    near(flipped.twice_signed_area,-24,0.0,
         "reflection must reverse area sign");
    near(flipped.barycentric.at_a,old.barycentric.at_a,0.0,
         "reflection preserves weights");
}

void nonfinite_and_extreme_values() {
    const double inf=std::numeric_limits<double>::infinity();
    const double nan=std::numeric_limits<double>::quiet_NaN();
    must_throw<std::invalid_argument>(
        [&] { (void)Triangle2::orient({0,0},{1,0},{nan,1}); },
        "NaN location rejected");
    must_throw<std::invalid_argument>(
        [&] { (void)Triangle2{{0,0},{inf,0},{0,4}}.sample({1,1}); },
        "nonfinite triangle vertex rejected");
    must_throw<std::overflow_error>(
        [] { (void)Triangle2::orient({-1e308,0},{1e308,0},{0,1}); },
        "subtraction overflow rejected");
    must_throw<std::invalid_argument>(
        [&] { (void)Triangle2::interpolate({1,0,0},{1,nan,3}); },
        "interpolation cannot accept nonfinite attributes");

    // 2^53 is the last power-of-two threshold before not every integer
    // is representable exactly as a binary64 coordinate.
    const double x = 9'007'199'254'740'992.0;
    require(x + 1.0 == x,
            "binary64 cannot preserve unit change at 2^53");
}

} // namespace

int main() {
    try {
        exact_orientation_and_area();
        exact_containment();
        exact_property_sweep();
        floating_barycentric_and_interpolation();
        affine_invariance();
        nonfinite_and_extreme_values();
        std::cout << "Book03/P03: integer oracle and floating triangle contracts passed\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03/P03 FAILED: " << e.what() << '\n';
        return 1;
    }
}
