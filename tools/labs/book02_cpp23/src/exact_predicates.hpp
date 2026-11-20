#pragma once
// Book Two: a transparent, standards-only C++23 binary64 orientation oracle.
// Input doubles represent exact binary rational numbers, NOT exact decimal
// intentions. The finite-input path never rounds intermediate arithmetic.
// Schoolbook base-2^32 integers are intentionally instructional, not fast.
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace book02::exact {
namespace detail {
struct Natural {
    std::vector<std::uint32_t> limbs; // least significant first
    Natural()=default;
    explicit Natural(std::uint64_t v) {
        if (v) { limbs.push_back(static_cast<std::uint32_t>(v));
                 if (v>>32) limbs.push_back(static_cast<std::uint32_t>(v>>32)); }
    }
    void trim() { while (!limbs.empty() && limbs.back()==0) limbs.pop_back(); }
    bool empty() const { return limbs.empty(); }
};
inline int compare(const Natural& a,const Natural& b) {
    if (a.limbs.size()!=b.limbs.size())
        return a.limbs.size()<b.limbs.size()?-1:1;
    for (std::size_t i=a.limbs.size();i-->0;)
        if (a.limbs[i]!=b.limbs[i]) return a.limbs[i]<b.limbs[i]?-1:1;
    return 0;
}
inline Natural add(const Natural& a,const Natural& b) {
    Natural r; r.limbs.resize(std::max(a.limbs.size(),b.limbs.size())+1);
    std::uint64_t carry=0;
    for (std::size_t i=0;i+1<r.limbs.size();++i) {
        const std::uint64_t x=i<a.limbs.size()?a.limbs[i]:0;
        const std::uint64_t y=i<b.limbs.size()?b.limbs[i]:0;
        const std::uint64_t sum=x+y+carry;
        r.limbs[i]=static_cast<std::uint32_t>(sum);carry=sum>>32;
    }
    r.limbs.back()=static_cast<std::uint32_t>(carry);r.trim();return r;
}
inline Natural subtract(const Natural& a,const Natural& b) { // a >= b
    Natural r; r.limbs.resize(a.limbs.size());
    std::uint64_t borrow=0;
    for (std::size_t i=0;i<a.limbs.size();++i) {
        const std::uint64_t x=a.limbs[i],y=i<b.limbs.size()?b.limbs[i]:0;
        const std::uint64_t needed=y+borrow;
        r.limbs[i]=static_cast<std::uint32_t>(x-needed);
        borrow=(x<needed);
    }
    r.trim();return r;
}
inline Natural shift(const Natural& a,unsigned bits) {
    if (a.empty()) return Natural{};
    const std::size_t whole=bits/32;
    const unsigned part=bits%32;
    Natural r; r.limbs.assign(a.limbs.size()+whole+1,0);
    for (std::size_t i=0;i<a.limbs.size();++i) {
        const std::uint64_t v=static_cast<std::uint64_t>(a.limbs[i])<<part;
        r.limbs[i+whole]|=static_cast<std::uint32_t>(v);
        r.limbs[i+whole+1]|=static_cast<std::uint32_t>(v>>32);
    }
    r.trim();return r;
}
inline Natural multiply(const Natural& a,const Natural& b) {
    if (a.empty()||b.empty()) return Natural{};
    Natural r; r.limbs.assign(a.limbs.size()+b.limbs.size(),0);
    for (std::size_t i=0;i<a.limbs.size();++i) {
        std::uint64_t carry=0;
        for (std::size_t j=0;j<b.limbs.size();++j) {
            const std::uint64_t v=static_cast<std::uint64_t>(a.limbs[i])*b.limbs[j]
                +r.limbs[i+j]+carry;
            r.limbs[i+j]=static_cast<std::uint32_t>(v);
            carry=v>>32;
        }
        r.limbs[i+b.limbs.size()]=static_cast<std::uint32_t>(carry);
    }
    r.trim();return r;
}
struct Dyadic {
    int sign{}; // -1, 0, +1
    Natural absolute;
    int exponent{}; // value = sign * absolute * 2^exponent
};
inline std::optional<Dyadic> decode(double value) {
    if (!std::isfinite(value)) return std::nullopt;
    constexpr std::uint64_t mask=(std::uint64_t{1}<<52)-1;
    const auto bits=std::bit_cast<std::uint64_t>(value);
    const auto field=static_cast<int>((bits>>52)&0x7ff);
    const std::uint64_t mantissa=(bits&mask)|(field?std::uint64_t{1}<<52:0);
    if (!mantissa) return Dyadic{}; // including -0
    return Dyadic{(bits>>63)?-1:1,Natural(mantissa),field?field-1023-52:-1074};
}
inline Dyadic negate(Dyadic a) { a.sign=-a.sign;return a; }
inline Dyadic add(Dyadic a,Dyadic b) {
    if (!a.sign) return b;
    if (!b.sign) return a;
    const int exponent=std::min(a.exponent,b.exponent);
    const auto am=shift(a.absolute,static_cast<unsigned>(a.exponent-exponent));
    const auto bm=shift(b.absolute,static_cast<unsigned>(b.exponent-exponent));
    if (a.sign==b.sign) return Dyadic{a.sign,add(am,bm),exponent};
    const int c=compare(am,bm);
    if (c==0) return Dyadic{};
    if (c>0) return Dyadic{a.sign,subtract(am,bm),exponent};
    return Dyadic{b.sign,subtract(bm,am),exponent};
}
inline Dyadic difference(Dyadic a,Dyadic b) { return add(std::move(a),negate(std::move(b))); }
inline Dyadic multiply(const Dyadic& a,const Dyadic& b) {
    if (!a.sign||!b.sign) return Dyadic{};
    return Dyadic{a.sign*b.sign,multiply(a.absolute,b.absolute),a.exponent+b.exponent};
}
} // namespace detail
struct Point { double x{},y{}; };
enum class Sign { negative=-1,zero=0,positive=1 };
// The *sign* is exact for the six supplied finite IEEE-754 binary64 values.
// No filter or hardware floating-point arithmetic is used after bit decode.
inline std::optional<Sign> orientation(Point a,Point b,Point c) {
    const auto ax=detail::decode(a.x),ay=detail::decode(a.y);
    const auto bx=detail::decode(b.x),by=detail::decode(b.y);
    const auto cx=detail::decode(c.x),cy=detail::decode(c.y);
    if (!ax||!ay||!bx||!by||!cx||!cy) return std::nullopt;
    const auto acx=detail::difference(*ax,*cx),acy=detail::difference(*ay,*cy);
    const auto bcx=detail::difference(*bx,*cx),bcy=detail::difference(*by,*cy);
    const auto d=detail::difference(detail::multiply(acx,bcy),detail::multiply(acy,bcx));
    return d.sign>0?Sign::positive:d.sign<0?Sign::negative:Sign::zero;
}
} // namespace book02::exact
