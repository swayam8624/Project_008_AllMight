#pragma once

// Book 03 / Chapter 05 — Exact orientation of REPRESENTED binary64 points.
//
// Correctness contract:
//   For finite IEEE-754 binary64 coordinates, orient_binary64(A,B,C) returns
//   the mathematically exact sign of the determinant of the 6 STORED doubles.
//   It does not promise the sign of unrounded decimal input, measured physical
//   coordinates, or the rounded double produced by a naive determinant.
//
// Proof sketch:
//   Every finite binary64 x equals an integer Nx times 2^(-1074).
//   Normal numbers have a 53-bit significand shifted left by E-1, where E
//   is the biased binary exponent; subnormals have no leading hidden bit.
//   Decode all six inputs into signed integers on this shared fixed scale.
//   Compute two exact integer products and an exact subtraction, and return
//   the sign. There is no intermediate floating arithmetic in the oracle.
//
// The nonnegative integers use a deliberately transparent vector<uint32_t>
// base-2^32 representation. All carries are held in uint64_t. The maximum
// product accumulation is (2^32-1)^2 + (2^32-1) + (2^32-1) = 2^64-1;
// it fits exactly in uint64_t. This is a pedagogical, portable big-integer
// implementation, NOT a production-optimized Shewchuk expansion.
//
// Two paths:
//   1) If every coordinate is a small integer in [-1e6,+1e6], reuse the
//      proven exact int64 oracle from Chapter 03.
//   2) Otherwise convert each binary64 bit pattern to exact signed dyadic
//      integers and calculate the sign with unlimited *dynamic* limb length.
// No unproved floating filter is used to certify a sign. This is adaptive
// in input representation, not a claim to match Shewchuk's performance.
//
// Assumptions: 8-byte IEEE binary64, 32-bit uint32_t, 64-bit uint64_t.
// The exact bit decode uses std::bit_cast, no aliasing tricks, rounding-mode
// assumptions, fast-math, external libraries, or GMP required.

#include "book03/triangle_geometry.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace book03 {

static_assert(sizeof(double) == sizeof(std::uint64_t));
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<double>::radix == 2);
static_assert(std::numeric_limits<double>::digits == 53);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::uint64_t) == 8);

namespace exact_detail {

// Unsigned arbitrary-size positive magnitude. Little-endian 32-bit limbs.
class Magnitude {
public:
    std::vector<std::uint32_t> limb{};

    Magnitude() = default;
    explicit Magnitude(std::uint64_t value) {
        if (value != 0) {
            limb.push_back(static_cast<std::uint32_t>(value));
            const auto upper = static_cast<std::uint32_t>(value >> 32u);
            if (upper) limb.push_back(upper);
        }
    }

    void trim() {
        while (!limb.empty() && limb.back() == 0) limb.pop_back();
    }
    [[nodiscard]] bool zero() const noexcept { return limb.empty(); }

    [[nodiscard]] static int compare(const Magnitude& a,
                                     const Magnitude& b) noexcept {
        if (a.limb.size() != b.limb.size()) {
            return a.limb.size() < b.limb.size() ? -1 : +1;
        }
        for (std::size_t i = a.limb.size(); i > 0; --i) {
            if (a.limb[i-1] != b.limb[i-1])
                return a.limb[i-1] < b.limb[i-1] ? -1 : +1;
        }
        return 0;
    }

    [[nodiscard]] static Magnitude shifted(Magnitude x, unsigned bits) {
        if (x.zero() || bits == 0) return x;
        const std::size_t whole = bits / 32u;
        const unsigned partial = bits % 32u;
        std::vector<std::uint32_t> out(whole + x.limb.size() + 1, 0);
        std::uint64_t carry = 0;
        for (std::size_t i=0;i<x.limb.size();++i) {
            const std::uint64_t word =
                (static_cast<std::uint64_t>(x.limb[i]) << partial) | carry;
            out[i+whole] = static_cast<std::uint32_t>(word);
            carry = word >> 32u;
        }
        out[whole + x.limb.size()] = static_cast<std::uint32_t>(carry);
        x.limb = std::move(out);
        x.trim();
        return x;
    }

    [[nodiscard]] static Magnitude add(const Magnitude& a,
                                       const Magnitude& b) {
        Magnitude out;
        out.limb.resize(std::max(a.limb.size(),b.limb.size())+1,0);
        std::uint64_t carry=0;
        for(std::size_t i=0;i+1<out.limb.size();++i) {
            const std::uint64_t x = i<a.limb.size() ? a.limb[i] : 0;
            const std::uint64_t y = i<b.limb.size() ? b.limb[i] : 0;
            const std::uint64_t sum=x+y+carry;
            out.limb[i] = static_cast<std::uint32_t>(sum);
            carry=sum>>32u;
        }
        out.limb.back() = static_cast<std::uint32_t>(carry);
        out.trim();
        return out;
    }

    // Precondition a >= b. No uint32 underflow is permitted.
    [[nodiscard]] static Magnitude subtract(const Magnitude& a,
                                            const Magnitude& b) {
        if(compare(a,b)<0) throw std::logic_error("unsigned subtraction sign");
        Magnitude out;
        out.limb.resize(a.limb.size(),0);
        std::uint64_t borrow=0;
        for(std::size_t i=0;i<a.limb.size();++i) {
            const std::uint64_t x=a.limb[i];
            const std::uint64_t y=(i<b.limb.size()?b.limb[i]:0) + borrow;
            if(x>=y) {
                out.limb[i]=static_cast<std::uint32_t>(x-y);
                borrow=0;
            } else {
                out.limb[i]=static_cast<std::uint32_t>(
                    (std::uint64_t{1}<<32u)+x-y);
                borrow=1;
            }
        }
        if(borrow) throw std::logic_error("unsigned borrow out of range");
        out.trim();
        return out;
    }

    [[nodiscard]] static Magnitude multiply(const Magnitude& a,
                                            const Magnitude& b) {
        if(a.zero()||b.zero()) return {};
        Magnitude out;
        out.limb.resize(a.limb.size()+b.limb.size(),0);
        for(std::size_t i=0;i<a.limb.size();++i) {
            std::uint64_t carry=0;
            for(std::size_t j=0;j<b.limb.size();++j) {
                const std::size_t k=i+j;
                // The three-term maximum is 2^64-1; uint64 is sufficient.
                const std::uint64_t sum=
                    std::uint64_t{a.limb[i]}*b.limb[j]+
                    std::uint64_t{out.limb[k]}+carry;
                out.limb[k]=static_cast<std::uint32_t>(sum);
                carry=sum>>32u;
            }
            // This slot has not been touched by any earlier outer row.
            out.limb[i+b.limb.size()]=static_cast<std::uint32_t>(carry);
        }
        out.trim();
        return out;
    }
};

struct SignedInteger {
    int sign{0};
    Magnitude magnitude{};

    [[nodiscard]] static SignedInteger decoded(double value) {
        if(!std::isfinite(value)) {
            throw std::invalid_argument(
                "exact binary64 orientation requires finite coordinates");
        }
        const std::uint64_t bits=std::bit_cast<std::uint64_t>(value);
        const unsigned e=static_cast<unsigned>((bits>>52u)&0x7ffu);
        const std::uint64_t fraction=bits & ((std::uint64_t{1}<<52u)-1u);
        if(e==0 && fraction==0) return {}; // both signed zeros are exact zero
        const std::uint64_t mantissa=
            e==0 ? fraction : (std::uint64_t{1}<<52u)|fraction;
        // e==0 => exponent=-1074. e>0 => exponent=e-1075.
        // Relative to -1074, normal values need left shift (e-1).
        const unsigned shift=e==0 ? 0u : e-1u;
        const int sign=(bits>>63u) ? -1 : +1;
        return {sign, Magnitude::shifted(Magnitude(mantissa),shift)};
    }

    [[nodiscard]] static SignedInteger negated(SignedInteger a) {
        a.sign=-a.sign;
        return a;
    }

    [[nodiscard]] static SignedInteger add(const SignedInteger& a,
                                           const SignedInteger& b) {
        if(a.sign==0) return b;
        if(b.sign==0) return a;
        if(a.sign==b.sign)
            return {a.sign,Magnitude::add(a.magnitude,b.magnitude)};
        const int compare=Magnitude::compare(a.magnitude,b.magnitude);
        if(compare==0) return {};
        if(compare>0)
            return {a.sign,Magnitude::subtract(a.magnitude,b.magnitude)};
        return {b.sign,Magnitude::subtract(b.magnitude,a.magnitude)};
    }

    [[nodiscard]] static SignedInteger sub(const SignedInteger& a,
                                           const SignedInteger& b) {
        return add(a,negated(b));
    }

    [[nodiscard]] static SignedInteger multiply(const SignedInteger& a,
                                                const SignedInteger& b) {
        if(a.sign==0||b.sign==0) return {};
        return {a.sign*b.sign,Magnitude::multiply(a.magnitude,b.magnitude)};
    }
};

} // namespace exact_detail

// This is a DIAGNOSTIC for the book's naive product-subtraction failure,
// never an exact or certified predicate. Force two separate binary64-rounded
// products so an optimizing compiler cannot fuse the expression into FMA.
// Volatile is intentional ONLY for this demonstration. It is not a generally
// recommended strategy for numerical stability or an arbitrary floating
// precision specification.
[[nodiscard]] inline double naive_orientation_two_rounded_products(
    Point2 a,Point2 b,Point2 c) {
    if(!finite(a)||!finite(b)||!finite(c))
        throw std::invalid_argument("naive diagnostic requires finite inputs");
    const double abx=b.x-a.x;
    const double aby=b.y-a.y;
    const double acx=c.x-a.x;
    const double acy=c.y-a.y;
    volatile double first=abx*acy;
    volatile double second=aby*acx;
    return first-second;
}

enum class ExactOrientPath { BoundedInteger, ExactBinary64Dyadic };

struct ExactOrientResult {
    int sign{};          // -1 clockwise, 0 collinear, +1 CCW in Cartesian y-up
    ExactOrientPath path{ExactOrientPath::ExactBinary64Dyadic};
};

// Returns true only if all six doubles represent EXACT integers that fit
// inside the already-proven Chapter 03 range. No cast is done before check.
[[nodiscard]] inline bool integer_oracle_applicable(Point2 a,Point2 b,
                                                     Point2 c) noexcept {
    const auto test=[](double v) {
        return std::isfinite(v) && v>=-1'000'000.0 && v<=1'000'000.0 &&
               std::trunc(v)==v;
    };
    return test(a.x)&&test(a.y)&&test(b.x)&&test(b.y)&&
           test(c.x)&&test(c.y);
}

[[nodiscard]] inline ExactOrientResult orient_binary64(
    Point2 a,Point2 b,Point2 c) {
    if(!finite(a)||!finite(b)||!finite(c))
        throw std::invalid_argument(
            "exact binary64 orientation rejects NaN and infinity");

    if(integer_oracle_applicable(a,b,c)) {
        const auto p=[](Point2 q) {
            return IntegerPoint2{
                static_cast<std::int64_t>(q.x),
                static_cast<std::int64_t>(q.y)};
        };
        return {exact_orientation(p(a),p(b),p(c)),
                ExactOrientPath::BoundedInteger};
    }

    // Exact bit-encoded input coordinates: no floating arithmetic here.
    using S=exact_detail::SignedInteger;
    const S ax=S::decoded(a.x), ay=S::decoded(a.y);
    const S bx=S::decoded(b.x), by=S::decoded(b.y);
    const S cx=S::decoded(c.x), cy=S::decoded(c.y);
    const S abx=S::sub(bx,ax), aby=S::sub(by,ay);
    const S acx=S::sub(cx,ax), acy=S::sub(cy,ay);
    const S determinant=S::sub(S::multiply(abx,acy),
                                S::multiply(aby,acx));
    return {determinant.sign,ExactOrientPath::ExactBinary64Dyadic};
}

enum class ExactBinary64Location { Interior, Edge, Exterior, Degenerate };

[[nodiscard]] inline ExactBinary64Location triangle_location_binary64(
    Point2 a,Point2 b,Point2 c,Point2 p) {
    // Evaluate all predicate inputs, including p, before early return.
    if(!finite(a)||!finite(b)||!finite(c)||!finite(p))
        throw std::invalid_argument(
            "exact triangle containment requires finite coordinates");
    const int orientation=orient_binary64(a,b,c).sign;
    if(orientation==0) return ExactBinary64Location::Degenerate;
    const int sa=orient_binary64(b,c,p).sign;
    const int sb=orient_binary64(c,a,p).sign;
    const int sc=orient_binary64(a,b,p).sign;
    if(sa*orientation<0||sb*orientation<0||sc*orientation<0)
        return ExactBinary64Location::Exterior;
    return (sa==0||sb==0||sc==0) ?
        ExactBinary64Location::Edge : ExactBinary64Location::Interior;
}

} // namespace book03
