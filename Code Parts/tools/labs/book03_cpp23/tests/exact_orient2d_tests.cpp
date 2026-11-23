// Book 03 P05 — witnesses for EXACT signs of finite represented doubles.
//
// This is a standard-C++23 integer-arithmetic oracle. The oracle never
// multiplies floating operands to decide their orientation, even if the
// educational diagnostic does. The critical failure is a represented
// binary64 triangle with determinant -4 but naive double determinant 0.
//
// Every test has a stated scope; we deliberately do NOT claim exactness of
// physical measurements or of real decimal numbers before input rounding.

#include "book03/exact_orient2d.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::Point2;
using book03::ExactOrientPath;
using book03::ExactBinary64Location;

void require(bool condition,std::string_view message) {
    if(!condition) throw std::runtime_error(std::string(message));
}

template<class Exception,class Fn>
void requires_throw(Fn&& fn,std::string_view message) {
    try { fn(); }
    catch(const Exception&) { return; }
    throw std::runtime_error(std::string(message));
}

int sgn(std::int64_t value) {
    return (value>0)-(value<0);
}

void basic_integer_and_topological_contracts() {
    const Point2 a{0,0},b{6,0},c{0,4};
    const auto result=book03::orient_binary64(a,b,c);
    require(result.sign==+1,"integer positive CCW orientation");
    require(result.path==ExactOrientPath::BoundedInteger,
            "small integer inputs use old P03 oracle");
    require(book03::orient_binary64(a,c,b).sign==-1,
            "reversing vertices reverses sign");
    require(book03::orient_binary64(a,a,c).sign==0,
            "repeated vertex is exactly collinear");
    require(book03::orient_binary64(a,b,b).sign==0,
            "zero edge has zero area");
    require(book03::triangle_location_binary64(a,b,c,{1,1}) ==
            ExactBinary64Location::Interior,"interior exact");
    require(book03::triangle_location_binary64(a,b,c,{3,0}) ==
            ExactBinary64Location::Edge,"edge exact");
    require(book03::triangle_location_binary64(a,c,b,{3,0}) ==
            ExactBinary64Location::Edge,"edge winding-independent");
    require(book03::triangle_location_binary64(a,b,c,{5,3}) ==
            ExactBinary64Location::Exterior,"outside exact");
    require(book03::triangle_location_binary64({0,0},{2,2},{4,4},{1,1}) ==
            ExactBinary64Location::Degenerate,"line cannot enclose area");
}

void catastrophic_product_cancellation() {
    // Every coordinate below is exactly representable binary64:
    // N=10^16, N+2, N+4 are multiples of its spacing (2).
    // Exact determinant for A=(0,0), B=(N,N+2), C=(N+2,N+4):
    // N(N+4) -(N+2)^2 = -4.
    // Binary64 products each round to the same large number. Naive
    // subtraction returns zero, a demonstrably wrong topological decision.
    constexpr double n=10'000'000'000'000'000.0;
    const Point2 a{0,0},b{n,n+2},c{n+2,n+4};
    require(b.y==n+2 && c.x==n+2 && c.y==n+4,
            "the fixture's distinct binary64 values must survive");
    const double naive=book03::naive_orientation_two_rounded_products(a,b,c);
    require(naive==0.0,"the deliberately fragile double oracle fails");
    const auto exact=book03::orient_binary64(a,b,c);
    require(exact.sign==-1,"exact dyadic oracle must recover negative four");
    require(exact.path==ExactOrientPath::ExactBinary64Dyadic,
            "large-integer-valued doubles need arbitrary-limb path");
    require(book03::orient_binary64(a,c,b).sign==+1,
            "reversing hard fixture sign");
    require(book03::orient_binary64(b,c,a).sign==-1,
            "cyclic permutation preserves sign");
    require(book03::triangle_location_binary64(a,b,c,a) ==
            ExactBinary64Location::Edge,
            "one vertex is exactly on the triangle boundary");

    // Input-rounding is an entirely different failure:
    // +1 disappears at 1e16 before our predicate even sees it.
    const Point2 unrecoverable{n+1,n+1};
    require(unrecoverable.x==n && unrecoverable.y==n,
            "binary64 cannot preserve original unrepresented +1");
    require(book03::orient_binary64(a,{n,n},unrecoverable).sign==0,
            "no exact oracle can recreate lost input bits");
}

void subnormals_and_exponent_extremes() {
    const double tiny=std::numeric_limits<double>::denorm_min();
    require(tiny>0.0 && tiny/2.0==0.0,
            "gradual-subnormal minimum binary64 witness");
    const Point2 a{0,0},b{tiny,0},c{0,tiny};
    const double naive=book03::naive_orientation_two_rounded_products(a,b,c);
    require(naive==0.0,"tiny positive determinant underflows in naive double");
    const auto result=book03::orient_binary64(a,b,c);
    require(result.sign==+1 &&
            result.path==ExactOrientPath::ExactBinary64Dyadic,
            "exact dyadic oracle survives subnormal product underflow");
    require(book03::orient_binary64(a,c,b).sign==-1,
            "subnormal winding reversal");

    const double huge=std::numeric_limits<double>::max();
    require(book03::orient_binary64({0,0},{huge,0},{0,huge}).sign==+1,
            "exact dyadic result even when naive product overflows infinity");
    require(book03::orient_binary64({0,0},{huge,huge},
                                    {huge,-huge}).sign==-1,
            "mixed extreme-sign products remain exact");
    require(book03::orient_binary64({0,0},{huge,huge},
                                    {huge,huge}).sign==0,
            "equal extrema remain exact zero");
    require(book03::orient_binary64(
        {-huge,-huge},{huge,huge},{huge,-huge}).sign==-1,
        "coordinate SUBTRACTION overflow does not affect integer bit oracle");

    const double half=std::numeric_limits<double>::min()/2.0;
    require(book03::orient_binary64({0,0},{half,0},{0,half}).sign==+1,
            "min normal / 2 is still exactly modeled");
    require(book03::orient_binary64({0,0},{-0.0,0.0},{0.0,+0.0}).sign==0,
            "positive and negative floating zero decode to exact zero");
}

void exact_rational_grid_comparison() {
    // Coordinates are rational halves, represented EXACTLY in binary64.
    // Multiplying them by 2 gives safely bounded integers, so the old P03
    // oracle becomes an independent reference; signs must agree exactly.
    std::uint32_t state=0x6CE1A13Fu;
    auto random_int=[&]() -> std::int64_t {
        state ^= state<<13;
        state ^= state>>17;
        state ^= state<<5;
        return static_cast<std::int64_t>(state % 200001u) - 100000;
    };
    int dyadic_count=0;
    for(int i=0;i<5000;++i) {
        // Add one half-unit to x coordinate to FORCE the dyadic path.
        const Point2 a{(2.0*random_int()+1.0)/2.0,
                       (2.0*random_int())/2.0};
        const Point2 b{(2.0*random_int())/2.0,
                       (2.0*random_int())/2.0};
        const Point2 c{(2.0*random_int())/2.0,
                       (2.0*random_int())/2.0};
        const auto exact=book03::orient_binary64(a,b,c);
        require(exact.path==ExactOrientPath::ExactBinary64Dyadic,
                "fractional input must use bit-exact path");
        ++dyadic_count;
        const auto i2=[](Point2 p) {
            return book03::IntegerPoint2{
                static_cast<std::int64_t>(p.x*2.0),
                static_cast<std::int64_t>(p.y*2.0)};
        };
        const int ref=book03::exact_orientation(i2(a),i2(b),i2(c));
        require(exact.sign==ref,
                "independent exact half-grid integer reference disagrees");
        require(book03::orient_binary64(a,c,b).sign==-ref,
                "fractional orientation antisymmetry");
        require(book03::orient_binary64(b,c,a).sign==ref,
                "fractional cyclic permutation");
    }
    require(dyadic_count==5000,
            "all 5000 deterministic comparison cases were exercised");
}

void exact_containment_with_fractional_points() {
    const Point2 a{0.5,0.5},b{6.5,0.5},c{0.5,4.5};
    require(book03::triangle_location_binary64(a,b,c,{1.5,1.5}) ==
            ExactBinary64Location::Interior,
            "fractional point strictly inside");
    require(book03::triangle_location_binary64(a,b,c,{3.5,0.5}) ==
            ExactBinary64Location::Edge,
            "fractional exact edge");
    require(book03::triangle_location_binary64(a,b,c,{5.5,3.5}) ==
            ExactBinary64Location::Exterior,
            "fractional outside");
    require(book03::triangle_location_binary64(a,c,b,{1.5,1.5}) ==
            ExactBinary64Location::Interior,
            "winding independent exact containment");
}

void invalids_rejected_including_early_return() {
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    requires_throw<std::invalid_argument>(
        [&] { (void)book03::orient_binary64({0,0},{1,0},{nan,1}); },
        "NaN rejected before bit decode");
    requires_throw<std::invalid_argument>(
        [&] { (void)book03::orient_binary64({0,0},{1,0},{inf,1}); },
        "infinity rejected");
    requires_throw<std::invalid_argument>(
        [&] { (void)book03::triangle_location_binary64(
            {0,0},{1,1},{2,2},{nan,0}); },
        "degenerate triangle must still reject NaN query");
    requires_throw<std::invalid_argument>(
        [&] { (void)book03::triangle_location_binary64(
            {0,0},{1,0},{0,1},{inf,0}); },
        "finite triangle cannot accept infinite query");
}

} // namespace

int main() {
    try {
        basic_integer_and_topological_contracts();
        catastrophic_product_cancellation();
        subnormals_and_exponent_extremes();
        exact_rational_grid_comparison();
        exact_containment_with_fractional_points();
        invalids_rejected_including_early_return();
        std::cout <<
            "Book03/P05: binary64 exact sign, overflow/underflow, "
            "5000 half-grid oracle cases: PASS\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03/P05 FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
