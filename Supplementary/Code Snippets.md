# Code Snippets

## Source-reported C++ project excerpts

These fragments came from M007, not from a live checkout. Reported names, ranges and APIs are **To verify**. They are separate here because they belong to projects with their own module/build dependencies. This merge does not authorize production edits. All independent C++ examples and the complete object laboratory are in [[Continuous Notes/02 - C++ - From Objects to Reliable Programs|Volume 02]].

Reported KairoMath/Vector.cppm module opening (source range 1–14):

```text
module;
// Standard-library includes in the global module fragment.
export module Kairo.Foundation.Math.Vector;
export namespace kairo::foundation::math {
    // Project definitions.
}
```

Reported Vector3 relationship (source ranges 470–472, 526–530, 546–549), intentionally not executable code:

```text
members: T x; T y; T z;
Data(): return &x;
operator[](index): assert(index < Size); return Data()[index];
```

Language analysis: separate scalar members are not an array; contiguous measured offsets cannot authorize Data()[1]. A span does not repair that representation. A project fix would need revision/caller checks and an API decision: preserve named-field syntax with explicit dispatch, or store a real array. No such change was made here.

Reported KairoECS/Entity.cppm (source ranges 10–22 and 27): Index and Generation are uint32_t fields; InvalidIndex uses numeric_limits; StructuralChangeKind is enum class with uint8_t underlying type. The source describes default initializers and no user-declared constructor. Aggregate status and current generation validation must be checked in the actual revision. A non-sentinel index alone is only shape validation, not proof of registry liveness.

Reported KairoMath/CMakeLists.txt (6–8 and 43–57): C++23 and FILE_SET CXX_MODULES. These are source-reported configuration claims, not an independently verified current build.

Canonical teaching code and verified repository excerpts live here. Organize by concept or subsystem, not by incoming chunk. Update existing sections when later chunks add variants, optimizations, tests, or failure handling.

For every snippet, label it as one of:

- `Canonical` - minimal explanatory implementation;
- `Repository-verified` - copied or adapted from an inspected repository location;
- `Pseudocode` - intentionally non-compilable structure;
- `To verify` - a provisional claim that must not be presented as repository fact.

Record language/version, inputs, outputs, preconditions, ownership and lifetime where relevant, complexity, failure behavior, and verification performed. Keep long repository listings out; quote only the lines needed to prove the point and link the exact anchor in `Sources and Code Anchors.md`.

## M001 - Width-safe fields and LSB-first bit packing

**Kind:** Canonical
**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 / M001]]
**Language:** C++23

### Contract

- Field width is 1-32; `shift + width` must fit a 32-bit carrier.
- Field insertion rejects values that do not fit instead of silently truncating them.
- Bit arrays contain only 0 or 1 and are packed LSB-first within each byte.
- Unpacking rejects a requested count larger than the source capacity.

### Snippet

```cpp
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

constexpr std::uint32_t low_mask(unsigned width) {
    if (width == 0 || width > 32)
        throw std::invalid_argument("field width must be in [1, 32]");
    return width == 32 ? ~std::uint32_t{0}
                       : (std::uint32_t{1} << width) - 1u;
}

constexpr std::uint32_t extract_field(
    std::uint32_t word, unsigned shift, unsigned width) {
    const auto low = low_mask(width);
    if (shift > 32u - width)
        throw std::invalid_argument("field exceeds carrier width");
    return (word >> shift) & low;
}

constexpr std::uint32_t insert_field(
    std::uint32_t word, std::uint32_t value,
    unsigned shift, unsigned width) {
    const auto low = low_mask(width);
    if (shift > 32u - width)
        throw std::invalid_argument("field exceeds carrier width");
    if (value > low)
        throw std::out_of_range("value does not fit field");
    const auto field = low << shift;
    return (word & ~field) | (value << shift);
}

std::vector<std::uint8_t> pack_bits(
    std::span<const std::uint8_t> bits) {
    std::vector<std::uint8_t> packed(bits.size() / 8u + (bits.size() % 8u != 0u), 0u);
    for (std::size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] > 1u)
            throw std::invalid_argument("a bit must be 0 or 1");
        packed[i / 8u] |= static_cast<std::uint8_t>(bits[i] << (i % 8u));
    }
    return packed;
}

std::vector<std::uint8_t> unpack_bits(
    std::span<const std::uint8_t> packed, std::size_t bit_count) {
    if (bit_count / 8u > packed.size() ||
        (bit_count / 8u == packed.size() && bit_count % 8u != 0u))
        throw std::invalid_argument("bit count exceeds packed capacity");
    std::vector<std::uint8_t> bits(bit_count);
    for (std::size_t i = 0; i < bit_count; ++i)
        bits[i] = static_cast<std::uint8_t>(
            (packed[i / 8u] >> (i % 8u)) & 1u);
    return bits;
}
```

### Cost and failure behavior

Field access over a fixed carrier is constant work. Packing and unpacking are $O(n)$, with $\lceil n/8\rceil$ packed bytes. The guards make width, value range, and source-capacity failures explicit rather than allowing invalid shifts, silent field truncation, or out-of-bounds access.

### Verification

Compiled with Homebrew Clang in C++23 mode using warnings as errors. Assertions covered `0xAD`, field replacement, LSB-first packing, round-trip unpacking, full-width masks, and rejected oversized unpack counts. See the current task verification record; no repository implementation was claimed by this canonical snippet.

## M002 - Exhaustive 8-bit addition oracle and saturation

**Kind:** Canonical
**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 / M002]]
**Language:** C++23

### Contract

- Inputs model two eight-bit operands plus a carry-in of 0 or 1.
- The calculation widens before retaining the architectural low byte.
- `carry` and `overflow` report unsigned and signed representability separately.
- Saturating addition clamps after widened calculation rather than allowing byte wrap.

### Snippet

```cpp
#include <cstdint>
#include <stdexcept>

struct Add8Result {
    std::uint8_t result;
    bool carry;
    bool overflow;
    bool negative;
    bool zero;
};

constexpr Add8Result add8(
    std::uint8_t a, std::uint8_t b, std::uint8_t carry_in) {
    if (carry_in > 1u)
        throw std::invalid_argument("carry-in must be 0 or 1");

    const std::uint16_t wide =
        static_cast<std::uint16_t>(a + b + carry_in);
    const auto result = static_cast<std::uint8_t>(wide & 0xFFu);
    const bool overflow =
        ((~(a ^ b) & (a ^ result) & 0x80u) != 0u);

    return {
        result,
        wide > 255,
        overflow,
        (result & 0x80u) != 0u,
        result == 0u
    };
}

constexpr std::uint8_t saturating_add_u8(
    std::uint8_t a, std::uint8_t b) {
    const std::uint16_t wide = static_cast<std::uint16_t>(a + b);
    if (wide > 255)
        return 255;
    return static_cast<std::uint8_t>(wide);
}
```

### Cost and failure behavior

For a fixed eight-bit architecture this is constant work. Widening preserves the ninth carry bit; the overflow expression independently compares operand and result signs. Invalid carry input fails explicitly. Saturation prevents wrap but intentionally changes algebraic behavior.

### Verification

Compiled with Homebrew Clang in C++23 mode and warnings as errors. An exhaustive driver checked all 131,072 `(a,b,carry_in)` states against independent widened unsigned and signed-range calculations, plus saturation boundaries and invalid carry rejection.

## Arithmetic reconstruction helpers

These are canonical teaching implementations. The multiplication helper widens the shifting partial product before any shift; widening only the final accumulator would lose information. Division widens the partial remainder so shifting it cannot discard the extra bit. Checked signed addition proves representability before executing the operation. Mixed-sign comparison checks the lower bound as well as the upper bound.

```cpp
#include <limits>
#include <utility>

constexpr std::uint32_t multiply_u16(
    std::uint16_t a, std::uint16_t b) {
    std::uint32_t partial = a;
    std::uint32_t result = 0;
    while (b != 0) {
        if ((b & 1u) != 0)
            result += partial;
        b = static_cast<std::uint16_t>(b >> 1);
        if (b != 0)
            partial <<= 1;
    }
    return result;
}

struct DivResult { std::uint32_t quotient, remainder; };

constexpr DivResult divide_u32(
    std::uint32_t dividend, std::uint32_t divisor) {
    if (divisor == 0)
        throw std::invalid_argument("division by zero");
    std::uint32_t quotient = 0;
    std::uint64_t remainder = 0;
    for (int bit = 31; bit >= 0; --bit) {
        remainder = (remainder << 1) | ((dividend >> bit) & 1u);
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= std::uint32_t{1} << bit;
        }
    }
    return {quotient, static_cast<std::uint32_t>(remainder)};
}

constexpr bool checked_add(int a, int b, int& result) {
    constexpr int hi = std::numeric_limits<int>::max();
    constexpr int lo = std::numeric_limits<int>::min();
    if ((b > 0 && a > hi - b) || (b < 0 && a < lo - b))
        return false;
    result = a + b;
    return true;
}

constexpr bool valid_index(int index, std::size_t size) {
    return index >= 0 && std::cmp_less(index, size);
}

// Binary subtraction only: carry=1 means no incoming borrow.
constexpr Add8Result subtract8(
    std::uint8_t a, std::uint8_t b, std::uint8_t carry_in) {
    return add8(a, static_cast<std::uint8_t>(b ^ 0xFFu), carry_in);
}
```

For subtraction, reusing add8 on the complemented operand also produces the correct binary subtraction overflow predicate. Decimal arithmetic and complete instruction timing are outside this helper's contract. The helper tests below cover its complete arithmetic input domain.

## M003 - Explicit byte codecs and checked alignment

**Kind:** Canonical · **Language:** C++23 · **Source:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 / M003]].

**Contract:** octets are eight bits; exact uint32/uint64 types must exist. Encode an unsigned 32-bit value in declared order; decode four bytes at a checked offset without typed unaligned dereferences. The input span is borrowed for the call only. Insufficient input or invalid power-of-two alignment/page shift throws invalid_argument; numeric rounding/translation overflow throws overflow_error. These functions manipulate bytes and numeric offsets; they neither allocate aligned objects nor query real page tables. Complexity is O(4) per codec and O(1) per numeric helper.

```cpp
#include <array>
#include <climits>
#include <limits>

static_assert(CHAR_BIT == 8);

enum class ByteOrder { Little, Big };

constexpr std::array<std::byte, 4> encode_u32(
    std::uint32_t value, ByteOrder order) {
    std::array<std::byte, 4> out{};
    for (unsigned i = 0; i < 4; ++i) {
        const unsigned digit = order == ByteOrder::Little ? i : 3u - i;
        out[i] = static_cast<std::byte>(
            (static_cast<std::uint64_t>(value) >> (8u * digit)) & 0xFFu);
    }
    return out;
}

std::uint32_t decode_u32(
    std::span<const std::byte> bytes, std::size_t offset, ByteOrder order) {
    // Subtraction after the lower bound avoids overflowing offset + 4.
    if (offset > bytes.size() || bytes.size() - offset < 4u)
        throw std::invalid_argument("not enough bytes for u32");
    std::uint64_t value = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const unsigned digit = order == ByteOrder::Little ? i : 3u - i;
        value |= std::to_integer<std::uint64_t>(bytes[offset + i])
                 << (8u * digit);
    }
    return static_cast<std::uint32_t>(value);
}

constexpr std::size_t align_up_checked(std::size_t offset, std::size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1u)) != 0)
        throw std::invalid_argument("alignment must be a nonzero power of two");
    const auto remainder = offset % alignment;
    const auto padding = remainder == 0 ? 0u : alignment - remainder;
    if (padding > std::numeric_limits<std::size_t>::max() - offset)
        throw std::overflow_error("aligned offset is not representable");
    return offset + padding;
}

struct PageParts {
    std::uint64_t vpn;
    std::uint64_t offset;
};

constexpr PageParts split_page(std::uint64_t address, unsigned shift) {
    if (shift == 0 || shift >= 64)
        throw std::invalid_argument("page shift must be in [1, 63]");
    const auto mask = (std::uint64_t{1} << shift) - 1u;
    return {address >> shift, address & mask};
}

constexpr std::uint64_t translated_address(
    std::uint64_t virtual_address, std::uint64_t frame, unsigned shift) {
    const auto parts = split_page(virtual_address, shift);
    if (frame > (std::numeric_limits<std::uint64_t>::max() >> shift))
        throw std::overflow_error("physical frame exceeds address width");
    return (frame << shift) | parts.offset;
}
```

**Why no reinterpret_cast load:** a span may start at any byte offset; bytewise decoding avoids assuming a live aligned uint32 object and imposes the external order explicitly. memcpy into an existing aligned object can copy representation safely for appropriate types, but does not convert host byte order. std::endian and std::byteswap are optional implementation tools, not substitutes for a format contract.

### Layout probe, not a wire format

```cpp
struct LayoutExample {
    std::uint8_t a;
    std::uint32_t b;
    std::uint8_t c;
};

struct VertexLike {
    std::uint8_t flags;
    std::uint64_t id;
    std::uint16_t layer;
    std::uint32_t color;
};

struct VertexLikeBetter {
    std::uint64_t id;
    std::uint32_t color;
    std::uint16_t layer;
    std::uint8_t flags;
};
```

Use sizeof, alignof, and offsetof on these standard-layout types to observe your compiler. Expected 12/24/16-byte results are conditional on the stated member alignments, not portable compile-time mandates. Neither object is serialized by copying its native bytes.

### M003 regression driver

Compile the canonical M001/M002/M003 snippets above together with this driver; it deliberately does not execute invalid pointer accesses. Tests cover golden bytes, both codec round trips, odd byte offset, truncation/huge-offset rejection, alignment minimality and maximum-boundary behavior, page decomposition/reconstruction, and measured ABI layout.

```cpp
#include <cassert>
#include <iostream>
#include <random>
#include <utility>

template<class Exception, class F>
void expect_exception(F&& action) {
    bool caught = false;
    try { std::forward<F>(action)(); }
    catch (const Exception&) { caught = true; }
    assert(caught);
}

void run_m004_lab();
void run_m005_lab();
void run_m006_lab();

int main() {
    run_m004_lab();
    run_m005_lab();
    run_m006_lab();
    const auto le = encode_u32(0x12345678u, ByteOrder::Little);
    const auto be = encode_u32(0x12345678u, ByteOrder::Big);
    assert((le == std::array<std::byte, 4>{
        std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12}}));
    assert((be == std::array<std::byte, 4>{
        std::byte{0x12}, std::byte{0x34}, std::byte{0x56}, std::byte{0x78}}));
    assert(decode_u32(le, 0, ByteOrder::Big) == 0x78563412u);
    std::mt19937 generator(20261006u);
    for (unsigned i = 0; i < 10000; ++i) {
        const auto x = static_cast<std::uint32_t>(generator());
        for (const auto order : {ByteOrder::Little, ByteOrder::Big}) {
            const auto encoded = encode_u32(x, order);
            assert(decode_u32(encoded, 0, order) == x);
            std::array<std::byte, 5> odd{};
            for (unsigned j = 0; j < 4; ++j) odd[j + 1] = encoded[j];
            assert(decode_u32(odd, 1, order) == x);
        }
    }
    for (std::size_t length = 0; length < 4; ++length)
        expect_exception<std::invalid_argument>([&] {
            (void)decode_u32(std::span(le).first(length), 0, ByteOrder::Little);
        });
    expect_exception<std::invalid_argument>([&] {
        (void)decode_u32(le, std::numeric_limits<std::size_t>::max(),
                         ByteOrder::Little);
    });
    for (std::size_t a = 1; a <= 1024; a *= 2) {
        for (std::size_t p = 0; p < 4096; ++p) {
            const auto result = align_up_checked(p, a);
            assert(result >= p && result % a == 0 && result - p < a);
            assert(align_up_checked(result, a) == result);
        }
    }
    const auto maximum = std::numeric_limits<std::size_t>::max();
    assert(align_up_checked(maximum, 1) == maximum);
    assert(align_up_checked(maximum - 7, 8) == maximum - 7);
    expect_exception<std::overflow_error>([&] {
        (void)align_up_checked(maximum, 8);
    });
    for (std::size_t a : {0u, 3u, 6u})
        expect_exception<std::invalid_argument>([&] { (void)align_up_checked(1, a); });

    const auto parts = split_page(0x12345ABCu, 12);
    assert(parts.vpn == 0x12345u && parts.offset == 0xABCu);
    assert(translated_address(0x12345ABCu, 0x987u, 12) == 0x987ABCu);
    for (unsigned k = 1; k < 64; ++k) {
        for (std::uint64_t x : {std::uint64_t{0}, std::uint64_t{1},
                                std::numeric_limits<std::uint64_t>::max()}) {
            const auto p = split_page(x, k);
            assert(translated_address(x, p.vpn, k) == x);
        }
    }
    for (unsigned k : {0u, 64u})
        expect_exception<std::invalid_argument>([&] { (void)split_page(0, k); });
    expect_exception<std::overflow_error>([] {
        (void)translated_address(0, std::uint64_t{1} << 52, 12);
    });
    assert(sizeof(LayoutExample) % alignof(LayoutExample) == 0);
    std::cout << "PASS: 10000 values in both byte orders at offsets 0 and 1; "
                 "45056 align-up cases; bounds and page cases\n";
    std::cout << "Measured ABI: Example size=" << sizeof(LayoutExample)
              << " align=" << alignof(LayoutExample)
              << " offsets=" << offsetof(LayoutExample, a) << ","
              << offsetof(LayoutExample, b) << "," << offsetof(LayoutExample, c)
              << "; Vertex=" << sizeof(VertexLike)
              << "; reordered=" << sizeof(VertexLikeBetter) << "\n";
}
```

**Limits:** numeric page functions are a model, not a live MMU test. This host's layout does not establish cross-platform ABI, SIMD performance, filesystem durability, or Kairo repository behavior.

## M004 - Binary interchange inspection and strict rounding laboratory

This cumulative section extends the existing driver, not a new per-chunk source file. The raw-word classifier never evaluates signaling NaNs as floats. Finite reconstruction requires the checked binary32/binary64 host. NaN numeric reconstruction is intentionally omitted because it would not preserve payload/signaling information.

Compile all cpp fences in this file in order with Clang C++23, warnings-as-errors, sanitizers, and `-ffp-model=strict`. The M003 main calls `run_m004_lab()`, defined below. Strict floating settings and a supported runtime environment are prerequisites; this is not a fast-math demonstration. See [Clang floating-point controls](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior).

```cpp
#include <algorithm>
#include <bit>
#include <cfenv>
#include <cmath>
#include <optional>

#pragma STDC FENV_ACCESS ON

namespace m004 {
using U32 = std::uint32_t;
enum class Kind { Zero, Subnormal, Normal, Infinity, QuietNaN, SignalingNaN };
struct Fields32 { bool negative; unsigned exponent; U32 fraction; Kind kind; };

constexpr Fields32 inspect_bits(U32 bits) {
    const bool sign = (bits >> 31) != 0;
    const unsigned e = (bits >> 23) & 255u;
    const U32 f = bits & 0x7fffffu;
    Kind kind = Kind::Normal;
    if (e == 0) kind = f == 0 ? Kind::Zero : Kind::Subnormal;
    else if (e == 255) {
        kind = f == 0 ? Kind::Infinity :
            (f & 0x400000u) != 0 ? Kind::QuietNaN : Kind::SignalingNaN;
    }
    return {sign, e, f, kind};
}

struct Fields64 { bool negative; unsigned exponent; std::uint64_t fraction; };
constexpr Fields64 inspect_bits64(std::uint64_t bits) {
    return {(bits >> 63) != 0,
            static_cast<unsigned>((bits >> 52) & 0x7ffu),
            bits & 0x000fffffffffffffull};
}

double decode_finite(U32 bits) {
    const auto f = inspect_bits(bits);
    if (f.exponent == 255) throw std::invalid_argument("not a finite word");
    if (f.fraction == 0 && f.exponent == 0)
        return std::copysign(0.0, f.negative ? -1.0 : 1.0);
    const double fraction = static_cast<double>(f.fraction) / 8388608.0;
    const double significand = f.exponent == 0 ? fraction : 1.0 + fraction;
    const int exponent = f.exponent == 0 ? -126 :
                         static_cast<int>(f.exponent) - 127;
    const double magnitude = std::ldexp(significand, exponent);
    return f.negative ? -magnitude : magnitude;
}

// Finite-only metric. Collapse signed zeros in the key itself.
constexpr U32 finite_key(U32 bits) {
    const U32 magnitude = bits & 0x7fffffffu;
    return (bits >> 31) != 0 ? 0x80000000u - magnitude :
                              0x80000000u + magnitude;
}
constexpr std::optional<U32> finite_ulp_distance(U32 a, U32 b) {
    if (inspect_bits(a).exponent == 255 || inspect_bits(b).exponent == 255)
        return std::nullopt;
    const U32 ka = finite_key(a), kb = finite_key(b);
    return ka > kb ? ka - kb : kb - ka;
}

// Deliberately lossy raw-word format policy; no signaling-NaN arithmetic.
constexpr U32 canonical_bits(U32 bits) {
    const auto f = inspect_bits(bits);
    if (f.kind == Kind::Zero) return 0;
    if (f.kind == Kind::QuietNaN || f.kind == Kind::SignalingNaN)
        return 0x7fc00000u;
    return bits;
}

// Reproduction of the supplied scalar excerpt, NOT a Kairo repository call.
bool supplied_nearly_equal(float a, float b, float tolerance) {
    const float difference = std::abs(a - b);
    if (difference <= tolerance) return true;
    float scale = std::abs(a) > std::abs(b) ? std::abs(a) : std::abs(b);
    if (scale < 1.0f) scale = 1.0f;
    return difference <= tolerance * scale;
}

// Alternative explicit teaching policy, float-specific. Widening makes
// finite differences/products safe here; this is not a generic-T claim.
bool policy_nearly_equal(float a, float b, double absolute, double relative) {
    if (!std::isfinite(absolute) || !std::isfinite(relative) ||
        absolute < 0.0 || relative < 0.0 || relative > 1.0)
        throw std::invalid_argument("invalid tolerance policy");
    if (a == b) return true; // includes signed zeros and equal infinities
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    const double da = a, db = b;
    const double difference = std::abs(da - db);
    const double scale = std::max(std::abs(da), std::abs(db));
    return difference <= absolute || difference <= relative * scale;
}

class RoundingScope {
    int previous_;
public:
    RoundingScope() : previous_(std::fegetround()) {
        if (previous_ == -1) throw std::runtime_error("rounding query failed");
    }
    RoundingScope(const RoundingScope&) = delete;
    RoundingScope& operator=(const RoundingScope&) = delete;
    void set(int mode) {
        if (std::fesetround(mode) != 0)
            throw std::runtime_error("rounding mode unsupported");
    }
    ~RoundingScope() { (void)std::fesetround(previous_); }
};

void laboratory() {
    static_assert(CHAR_BIT == 8 && sizeof(float) == sizeof(U32));
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    static_assert(std::numeric_limits<float>::is_iec559 &&
                  std::numeric_limits<float>::radix == 2 &&
                  std::numeric_limits<float>::digits == 24 &&
                  std::numeric_limits<float>::min_exponent == -125 &&
                  std::numeric_limits<float>::max_exponent == 128);
    static_assert(std::numeric_limits<double>::is_iec559 &&
                  std::numeric_limits<double>::radix == 2 &&
                  std::numeric_limits<double>::digits == 53 &&
                  std::numeric_limits<double>::min_exponent == -1021 &&
                  std::numeric_limits<double>::max_exponent == 1024);
    assert(std::bit_cast<U32>(1.0f) == 0x3f800000u);
    assert(std::bit_cast<std::uint64_t>(1.0) == 0x3ff0000000000000ull);
    const auto d = inspect_bits64(0x3ff0000000000000ull);
    assert(!d.negative && d.exponent == 1023 && d.fraction == 0);

    const std::array<std::pair<U32, Kind>, 13> golden{{
        {0x00000000u, Kind::Zero}, {0x80000000u, Kind::Zero},
        {0x00000001u, Kind::Subnormal}, {0x007fffffu, Kind::Subnormal},
        {0x00800000u, Kind::Normal}, {0x3f7fffffu, Kind::Normal},
        {0x3f800000u, Kind::Normal}, {0x3f800001u, Kind::Normal},
        {0x7f7fffffu, Kind::Normal}, {0x7f800000u, Kind::Infinity},
        {0xff800000u, Kind::Infinity}, {0x7fc00001u, Kind::QuietNaN},
        {0x7f800001u, Kind::SignalingNaN}
    }};
    for (const auto& [word, kind] : golden) {
        assert(inspect_bits(word).kind == kind);
        for (const auto order : {ByteOrder::Little, ByteOrder::Big})
            assert(decode_u32(encode_u32(word, order), 0, order) == word);
    }
    assert(std::bit_cast<U32>(10.625f) == 0x412a0000u);
    assert(decode_finite(0xc1540000u) == -13.25);
    assert(decode_finite(0x00000001u) == std::ldexp(1.0, -149));
    assert(std::signbit(decode_finite(0x80000000u)));
    assert((encode_u32(0x3f800000u, ByteOrder::Little) ==
        std::array<std::byte,4>{std::byte{0},std::byte{0},
                               std::byte{0x80},std::byte{0x3f}}));
    assert((encode_u32(0x7fc00001u, ByteOrder::Little) ==
        std::array<std::byte,4>{std::byte{1},std::byte{0},
                               std::byte{0xc0},std::byte{0x7f}}));

    std::mt19937 generator(20261007u);
    unsigned finite_checks = 0;
    for (unsigned i = 0; i < 10000; ++i) {
        const U32 word = static_cast<U32>(generator());
        const auto f = inspect_bits(word);
        assert(f.negative == ((word & 0x80000000u) != 0));
        assert(f.fraction == (word & 0x7fffffu));
        if (f.exponent == 255) continue; // no sNaN evaluation
        const double native = static_cast<double>(std::bit_cast<float>(word));
        const double reconstructed = decode_finite(word);
        assert(native == reconstructed);
        assert(std::signbit(native) == std::signbit(reconstructed));
        ++finite_checks;
    }
    expect_exception<std::invalid_argument>([] { (void)decode_finite(0x7f800000u); });
    assert(finite_ulp_distance(0u, 0x80000000u) == 0u);
    assert(finite_ulp_distance(0x80000001u, 1u) == 2u);
    assert(finite_ulp_distance(0x3f7fffffu, 0x3f800001u) == 2u);
    assert(!finite_ulp_distance(0x7f800000u, 0u));
    assert(!finite_ulp_distance(0x7fc00001u, 0u));
    assert(canonical_bits(0x80000000u) == 0u);
    assert(canonical_bits(0x7f800001u) == 0x7fc00000u);
    assert(canonical_bits(0x412a0000u) == 0x412a0000u);

    const float inf = std::numeric_limits<float>::infinity();
    const double upper_gap = static_cast<double>(std::nextafter(1.0f, inf)) - 1.0;
    const double lower_gap = 1.0 - static_cast<double>(std::nextafter(1.0f, -inf));
    assert(upper_gap == std::ldexp(1.0, -23));
    assert(lower_gap == std::ldexp(1.0, -24));
    assert(static_cast<double>(std::nextafter(1024.0f, inf)) - 1024.0 ==
           std::ldexp(1.0, -13));
    assert(static_cast<double>(std::nextafter(1e9f, inf)) - 1e9 == 64.0);
    assert(std::numeric_limits<float>::epsilon() == upper_gap);

    const int initial_mode = std::fegetround();
    assert(initial_mode != -1);
    {
        RoundingScope rounding;
        const std::array<int,4> modes{FE_TONEAREST,FE_UPWARD,FE_DOWNWARD,FE_TOWARDZERO};
        const std::array<double,4> inputs{2.5,-2.5,2.1,-2.1};
        const std::array<std::array<double,4>,4> expected{{
            {2,-2,2,-2},{3,-2,3,-2},{2,-3,2,-3},{2,-2,2,-2}
        }};
        for (unsigned m = 0; m < modes.size(); ++m) {
            rounding.set(modes[m]);
            for (unsigned i = 0; i < inputs.size(); ++i) {
                volatile double runtime_input = inputs[i];
                assert(std::nearbyint(runtime_input) == expected[m][i]);
            }
        }
        rounding.set(FE_TONEAREST);
        volatile double tie1 = 1.0 + std::ldexp(1.0,-24);
        volatile double tie2 = 1.0 + 3.0 * std::ldexp(1.0,-24);
        volatile float rounded1 = static_cast<float>(tie1);
        volatile float rounded2 = static_cast<float>(tie2);
        assert(std::bit_cast<U32>(static_cast<float>(rounded1)) == 0x3f800000u);
        assert(std::bit_cast<U32>(static_cast<float>(rounded2)) == 0x3f800002u);
        assert(std::round(2.5) == 3.0 && std::nearbyint(2.5) == 2.0);

        volatile float huge = 1e20f;
        const float naive = std::sqrt(huge * huge + huge * huge);
        const float robust = std::hypot(static_cast<float>(huge),static_cast<float>(huge));
        assert(std::isinf(naive) && std::isfinite(robust));
        assert(huge / naive == 0.0f);
        const float tiny_norm = std::hypot(1e-8f,0.0f);
        assert(tiny_norm > 0 && tiny_norm < std::numeric_limits<float>::epsilon());
        const float tol = 10 * std::numeric_limits<float>::epsilon();
        assert(!supplied_nearly_equal(inf,inf,tol));
        assert(supplied_nearly_equal(1.0f,inf,tol));
        assert(supplied_nearly_equal(inf,-inf,tol));
        assert(policy_nearly_equal(inf,inf,1e-6,1e-6));
        assert(!policy_nearly_equal(1.0f,inf,1e-6,1e-6));
        assert(!policy_nearly_equal(inf,-inf,1e-6,1e-6));
        assert(!policy_nearly_equal(std::numeric_limits<float>::quiet_NaN(),
                                   1.0f,1e-6,1e-6));
        assert(policy_nearly_equal(0.0f,-0.0f,0,0));
        expect_exception<std::invalid_argument>([] {
            (void)policy_nearly_equal(1,1,-1,1e-6);
        });
    }
    assert(std::fegetround() == initial_mode);
    std::cout << "PASS M004: 13 golden classes; 10000 seeded words ("
              << finite_checks << " finite reconstructions); spacing, zero-crossing "
                 "ULP, raw codecs, 4 rounding modes, runtime ties, norm and "
                 "comparison regressions; rounding mode restored\n";
}
} // namespace m004
void run_m004_lab() { m004::laboratory(); }
```

**Limits:** no exhaustive binary32 test, live repository calls, signaling-NaN arithmetic, performance benchmarks, FTZ/DAZ mode changes, complete FPU implementation, or cross-platform payload-preservation claim. The laboratory restores rounding direction; it deliberately exercises operations that may set floating status flags and does not promise to restore those flags.

## M005 - Strict arithmetic and state-update laboratory

Reuse M004's field inspection, validated float comparison and strict assumptions. This section adds arithmetic/order demonstrations and a simplified source-shaped weighted recurrence. It calls no Kairo or Maveb API. Compile all cpp fences in order with `-ffp-model=strict -ffp-contract=off`; run the shared driver, which calls this section after M004.

```cpp
#include <concepts>

namespace m005 {
template<std::floating_point T>
T naive(std::span<const T> values) {
    T sum = 0;
    for (T x : values) sum = sum + x;
    return sum;
}

template<std::floating_point T>
T kahan(std::span<const T> values) {
    T sum = 0, compensation = 0;
    for (T x : values) {
        const T y = x - compensation;
        const T next = sum + y;
        compensation = (next - sum) - y;
        sum = next;
    }
    return sum;
}

// Fixed midpoint split, no allocation or copying; finite-only teaching use.
template<std::floating_point T>
T pairwise(std::span<const T> values) {
    if (values.empty()) return T(0);
    if (values.size() == 1) return values[0];
    const std::size_t middle = values.size() / 2;
    const T left = pairwise<T>(values.first(middle));
    const T right = pairwise<T>(values.subspan(middle));
    return left + right;
}

template<std::floating_point State>
struct WeightedState { State distance = 0, weight = 0; };

// Toy normalized-distance contract: positive finite cap/weight, finite
// sample in [-1,1], existing nonnegative finite weight and distance.
// Float state emulates repeated narrowing, not an actual voxel object.
template<std::floating_point State>
void update(WeightedState<State>& state, double sample,
            double sample_weight, double cap) {
    if (!std::isfinite(cap) || cap <= 0 || !std::isfinite(sample_weight) ||
        sample_weight <= 0 || !std::isfinite(sample) || std::abs(sample) > 1 ||
        !std::isfinite(state.weight) || state.weight < 0 ||
        !std::isfinite(state.distance) || std::abs(state.distance) > 1)
        throw std::invalid_argument("invalid weighted state input");
    const double old_weight = static_cast<double>(state.weight);
    // Addition overflow would mean the positive cap wins; still a
    // teaching model, not a complete status-free allocator/voxel API.
    const double combined = std::min(cap, old_weight + sample_weight);
    const double contribution = std::min(sample_weight, combined);
    const double retained = combined - contribution;
    const double mean = (static_cast<double>(state.distance) * retained +
                         sample * contribution) / combined;
    state.distance = static_cast<State>(mean);
    state.weight = static_cast<State>(combined);
}

// Save both direction and flags. Restoration success is verified by lab.
class EnvironmentScope {
    std::fenv_t saved_{};
public:
    EnvironmentScope() {
        if (std::fegetenv(&saved_) != 0)
            throw std::runtime_error("environment snapshot failed");
    }
    EnvironmentScope(const EnvironmentScope&) = delete;
    EnvironmentScope& operator=(const EnvironmentScope&) = delete;
    ~EnvironmentScope() { (void)std::fesetenv(&saved_); }
};

void laboratory() {
    const int initial_mode = std::fegetround();
    const int initial_flags = std::fetestexcept(FE_ALL_EXCEPT);
    {
        EnvironmentScope environment;
        if (std::fesetround(FE_TONEAREST) != 0)
            throw std::runtime_error("nearest rounding unavailable");

        assert(std::bit_cast<std::uint32_t>(0.1f) == 0x3dcccccdu);
        assert(static_cast<double>(0.1f) == 13421773.0 / 134217728.0);
        assert(std::bit_cast<std::uint64_t>(0.1) == 0x3fb999999999999aull);
        volatile double tenth = 0.1, fifth = 0.2;
        const double sum_decimal = tenth + fifth;
        assert(std::bit_cast<std::uint64_t>(sum_decimal) == 0x3fd3333333333334ull);
        assert(std::bit_cast<std::uint64_t>(0.3) == 0x3fd3333333333333ull);
        unsigned remainder = 1;
        const std::array<unsigned,5> digits{0,0,0,1,1};
        for (unsigned digit : digits) {
            remainder *= 2;
            assert(remainder / 10 == digit);
            remainder %= 10;
        }
        assert(remainder == 2);

        volatile float large = 0x1p24f, one = 1.0f;
        assert(large + one == large);
        assert(large + 2.0f == 16777218.0f);
        assert(1.5f + 0.15625f == 1.65625f);
        assert(1753.0f / 1024 - 1751.0f / 1024 == 0x1p-9f);
        volatile double intended = 1.00000006;
        const float stored = static_cast<float>(intended);
        assert(std::bit_cast<std::uint32_t>(stored) == 0x3f800001u);
        assert(stored - 1.0f == 0x1p-23f);
        volatile double original_component = 100000001.0;
        const float narrow_component = static_cast<float>(original_component);
        assert(narrow_component == 100000000.0f);
        assert(static_cast<double>(narrow_component) != original_component);

        volatile double x = 1e16;
        const double direct = std::sqrt(x + 1.0) - std::sqrt(x);
        const double reformulated = 1.0 / (std::sqrt(x + 1.0) + std::sqrt(x));
        assert(direct == 0 && reformulated > 4.9e-9 && reformulated < 5.1e-9);

        assert(std::feclearexcept(FE_ALL_EXCEPT) == 0);
        volatile float maximum = std::numeric_limits<float>::max();
        volatile float overflow = maximum * 2.0f;
        const int overflow_flags = std::fetestexcept(FE_ALL_EXCEPT);
        assert(std::isinf(overflow));
        assert((overflow_flags & FE_OVERFLOW) && (overflow_flags & FE_INEXACT));
        assert(std::feclearexcept(FE_ALL_EXCEPT) == 0);
        volatile float minimum_normal = std::numeric_limits<float>::min();
        volatile float exact_subnormal = minimum_normal * 0.5f;
        const int exact_flags = std::fetestexcept(FE_ALL_EXCEPT);
        assert(exact_subnormal == 0x1p-127f);
        assert((exact_flags & (FE_UNDERFLOW | FE_INEXACT)) == 0);
        assert(std::feclearexcept(FE_ALL_EXCEPT) == 0);
        volatile float minimum_subnormal = std::numeric_limits<float>::denorm_min();
        volatile float tiny_zero = minimum_subnormal * 0.5f;
        const int tiny_flags = std::fetestexcept(FE_ALL_EXCEPT);
        assert(tiny_zero == 0);
        assert((tiny_flags & FE_UNDERFLOW) && (tiny_flags & FE_INEXACT));

        const float p = 0x1p-23f, a = 1.0f + p, c = -(1.0f + 2*p);
        volatile float product = a*a;
        const float separate = product + c;
        const float fused = std::fma(a,a,c);
        assert(separate == 0 && fused == 0x1p-46f);

        volatile float aa = 1e20f, bb = -1e20f, cc = 3.14f;
        volatile float left_partial = aa + bb, right_partial = bb + cc;
        assert(left_partial + cc == cc);
        assert(aa + right_partial == 0);

        volatile float scale = 1e10f, b = 1.0f+p, minus_one = -1.0f;
        volatile float inner = b + minus_one;
        const float factored = scale * inner;
        volatile float ab = scale*b, ac = scale*minus_one;
        const float expanded = ab+ac;
        assert(factored == 1192.0928955078125f && expanded == 1024.0f);
        assert(factored != expanded);

        const std::array<float,3> inputs{0x1p24f,1.0f,1.0f};
        assert(naive<float>(inputs) == 0x1p24f);
        assert(kahan<float>(inputs) == 16777218.0f);
        assert(pairwise<float>(inputs) == 16777218.0f);
        assert(pairwise<float>(std::span<const float>{}) == 0);
        const std::array<float,4> fixed{1e20f,1.0f,-1e20f,1.0f};
        const auto reference_bits = std::bit_cast<std::uint32_t>(pairwise<float>(fixed));
        for (unsigned i = 0; i < 1000; ++i)
            assert(std::bit_cast<std::uint32_t>(pairwise<float>(fixed)) == reference_bits);

        assert(m004::policy_nearly_equal(0,0.75f,1,0));
        assert(m004::policy_nearly_equal(0.75f,1.5f,1,0));
        assert(!m004::policy_nearly_equal(0,1.5f,1,0));
        expect_exception<std::invalid_argument>([] {
            (void)m004::policy_nearly_equal(1,1,
                std::numeric_limits<double>::infinity(),0);
        });

        WeightedState<float> capped_a, capped_b;
        for (double sample : {0.0,1.0,-1.0}) update(capped_a,sample,1,2);
        for (double sample : {0.0,-1.0,1.0}) update(capped_b,sample,1,2);
        assert(capped_a.weight == 2 && capped_b.weight == 2);
        assert(capped_a.distance == -0.25f && capped_b.distance == 0.25f);

        WeightedState<float> float_state;
        WeightedState<double> double_state;
        const float half_neighbor = 0.5f + 0x1p-24f;
        for (double sample : {0.5,static_cast<double>(half_neighbor),
                                  static_cast<double>(half_neighbor)}) {
            update(float_state,sample,1,10);
            update(double_state,sample,1,10);
        }
        assert(float_state.distance == 0.5f);
        assert(static_cast<float>(double_state.distance) == half_neighbor);
        expect_exception<std::invalid_argument>([&] { update(float_state,0,0,2); });

        const double inf = std::numeric_limits<double>::infinity();
        assert(std::llround(std::nextafter(2.5,-inf)) == 2);
        assert(std::llround(2.5) == 3);
        assert(std::llround(std::nextafter(2.5,inf)) == 3);
    }
    assert(std::fegetround() == initial_mode);
    assert(std::fetestexcept(FE_ALL_EXCEPT) == initial_flags);
    std::cout << "PASS M005: representation/alignment/cancellation; host range flags; "
                 "FMA/reassociation/distributivity; compensated and fixed-tree sums; "
                 "tolerance, capped-history, mixed-state and pixel-boundary regressions; "
                 "1000 repeat checks; floating environment restored\n";
}
} // namespace m005
void run_m005_lab() { m005::laboratory(); }
```

## Representation codecs and bounded file laboratory

**Contract:** this is a standalone teaching implementation, not a NanoQuant patch or a promised compatible production format. It requires eight-bit octets and the binary32 host checks already used by the cumulative lab. Fixed storage is int32; binary fraction count is 0–16, with int64 intermediates proved large enough. Fixed conversion uses half-away; fixed multiply/divide use nearest-even, checked narrowing and explicit zero rejection. Quantization accepts finite samples and a positive finite scale, uses nearest-even and clips to the declared small code domain before integer conversion. NaNs canonicalize in the half/BF16 converters, while tensor serialization preserves raw uint32 words.

The source-shaped tensor image fixes 28 header bytes, little-endian fields and words, nonzero dimensions, exact file length, and at most 4096 elements. It returns raw words rather than manufacturing typed objects in mapped bytes. CRC is calculated separately over specified complete bytes; no integrity field is claimed in a source repository format. These routines are bounded examples, not a complete application library.

Compile all cpp fences in order with warnings-as-errors, strict floating semantics, implicit contraction disabled, and sanitizers. The existing main calls `run_m006_lab()`.

```cpp
#include <sstream>
#include <iomanip>
#include <string>

namespace m006 {
std::uint64_t magnitude(std::int64_t x) {
    return x >= 0 ? static_cast<std::uint64_t>(x)
                  : static_cast<std::uint64_t>(-(x + 1)) + 1;
}

// Signed nearest-even division without negating INT64_MIN or doubling r.
std::int64_t round_ratio(std::int64_t numerator, std::int64_t denominator) {
    if (denominator == 0) throw std::domain_error("zero divisor");
    const bool negative = (numerator < 0) != (denominator < 0);
    const auto n = magnitude(numerator), d = magnitude(denominator);
    auto q = n / d;
    const auto r = n % d;
    if (r > d - r || (r == d - r && (q & 1u))) ++q;
    const auto limit = std::uint64_t{1} << 63;
    if (q > limit || (!negative && q == limit))
        throw std::overflow_error("wide quotient");
    if (negative && q == limit) return std::numeric_limits<std::int64_t>::min();
    const auto signed_q = static_cast<std::int64_t>(q);
    return negative ? -signed_q : signed_q;
}

std::int32_t narrow(std::int64_t x) {
    if (x < std::numeric_limits<std::int32_t>::min() ||
        x > std::numeric_limits<std::int32_t>::max())
        throw std::overflow_error("fixed destination");
    return static_cast<std::int32_t>(x);
}

std::int64_t scale(unsigned fraction_bits) {
    if (fraction_bits > 16) throw std::invalid_argument("fraction count");
    return std::int64_t{1} << fraction_bits;
}

std::int32_t from_double(double x, unsigned f = 16) {
    const auto s = scale(f);
    if (!std::isfinite(x)) throw std::invalid_argument("nonfinite fixed input");
    const double scaled = x * static_cast<double>(s);
    if (!std::isfinite(scaled)) throw std::overflow_error("scaled input");
    const double rounded = std::round(scaled); // Half away from zero.
    if (rounded < static_cast<double>(std::numeric_limits<std::int32_t>::min()) ||
        rounded > static_cast<double>(std::numeric_limits<std::int32_t>::max()))
        throw std::overflow_error("fixed input");
    return static_cast<std::int32_t>(rounded);
}

std::int32_t multiply(std::int32_t a, std::int32_t b, unsigned f = 16) {
    return narrow(round_ratio(static_cast<std::int64_t>(a) * b, scale(f)));
}

std::int32_t divide(std::int32_t a, std::int32_t b, unsigned f = 16) {
    return narrow(round_ratio(static_cast<std::int64_t>(a) * scale(f), b));
}

double round_even_small(double x) {
    // Caller bounds x to a small finite code interval first.
    const double lower = std::floor(x), tail = x - lower;
    const int integer = static_cast<int>(lower);
    return lower + (tail > 0.5 || (tail == 0.5 && integer % 2 != 0));
}

std::uint8_t int4_code(double x, double s) {
    if (!std::isfinite(x) || !std::isfinite(s) || s <= 0)
        throw std::invalid_argument("quantizer input");
    const double clipped = std::clamp(x / s, -8.0, 7.0);
    return static_cast<std::uint8_t>(static_cast<int>(round_even_small(clipped)) + 8);
}

std::vector<std::uint8_t> pack(std::span<const std::uint8_t> codes) {
    std::vector<std::uint8_t> bytes(codes.size()/2 + codes.size()%2, 0);
    for (std::size_t i = 0; i < codes.size(); ++i) {
        if (codes[i] > 15) throw std::invalid_argument("nibble");
        bytes[i/2] |= static_cast<std::uint8_t>(codes[i] << (4u * (i%2)));
    }
    return bytes;
}

std::vector<std::uint8_t> unpack(std::span<const std::uint8_t> bytes,
                                 std::size_t count) {
    // Avoid multiplying source length by two or adding one to count.
    const std::size_t need = count/2 + count%2;
    if (need != bytes.size()) throw std::invalid_argument("packed count");
    if (count%2 && (bytes.back() & 0xF0u))
        throw std::invalid_argument("noncanonical final nibble");
    std::vector<std::uint8_t> codes(count);
    for (std::size_t i = 0; i < count; ++i)
        codes[i] = static_cast<std::uint8_t>((bytes[i/2] >> (4u*(i%2))) & 15u);
    return codes;
}

double unorm8(std::uint8_t q) { return static_cast<double>(q)/255; }
double snorm8(std::int8_t q) { return std::max(static_cast<double>(q)/127, -1.0); }
std::uint8_t encode_unorm8(double x) {
    if (!std::isfinite(x)) throw std::invalid_argument("UNORM input");
    return static_cast<std::uint8_t>(round_even_small(std::clamp(x,0.0,1.0)*255));
}

float half_value(std::uint16_t h) {
    const unsigned e = (h >> 10) & 31u, f = h & 1023u;
    const std::uint32_t sign = std::uint32_t(h & 0x8000u) << 16;
    if (e == 31)
        return std::bit_cast<float>(sign | (f ? 0x7FC00000u : 0x7F800000u));
    float value = e == 0 ? std::ldexp(static_cast<float>(f), -24)
                         : std::ldexp(static_cast<float>(1024u + f),
                                      static_cast<int>(e) - 25);
    return std::copysign(value, (h & 0x8000u) ? -1.0f : 1.0f);
}

std::uint32_t round_bits(std::uint32_t x, unsigned shift) {
    assert(shift > 0 && shift <= 24);
    const auto q = x >> shift, r = x & ((std::uint32_t{1} << shift) - 1);
    const auto halfway = std::uint32_t{1} << (shift - 1);
    return q + (r > halfway || (r == halfway && (q & 1u)));
}

std::uint16_t half_bits(float x) {
    const auto word = std::bit_cast<std::uint32_t>(x);
    const auto sign = (word >> 16) & 0x8000u;
    const auto e = (word >> 23) & 255u, f = word & 0x7FFFFFu;
    if (e == 255) return static_cast<std::uint16_t>(f ? 0x7E00u : sign|0x7C00u);
    int he = static_cast<int>(e) - 127 + 15;
    if (he >= 31) return static_cast<std::uint16_t>(sign|0x7C00u);
    if (he <= 0) {
        if (he < -10) return static_cast<std::uint16_t>(sign);
        return static_cast<std::uint16_t>(
            sign | round_bits(f|0x800000u, static_cast<unsigned>(14-he)));
    }
    auto fraction = round_bits(f,13);
    if (fraction == 1024) { fraction = 0; ++he; }
    return static_cast<std::uint16_t>(sign | (std::uint32_t(he)<<10) | fraction);
}

std::uint16_t bfloat_bits(float x) {
    auto word = std::bit_cast<std::uint32_t>(x);
    if ((word & 0x7FFFFFFFu) > 0x7F800000u) return 0x7FC0u;
    word += 0x7FFFu + ((word >> 16)&1u);
    return static_cast<std::uint16_t>(word >> 16);
}
float bfloat_value(std::uint16_t h) {
    return std::bit_cast<float>(std::uint32_t(h) << 16);
}

std::uint64_t checked_product(std::uint64_t a, std::uint64_t b) {
    if (a && b > std::numeric_limits<std::uint64_t>::max()/a)
        throw std::overflow_error("shape product");
    return a*b;
}

void append_le(std::vector<std::byte>& bytes, std::uint64_t word, unsigned width) {
    if (width != 4 && width != 8) throw std::invalid_argument("field width");
    for (unsigned i = 0; i < width; ++i)
        bytes.push_back(static_cast<std::byte>((word >> (8u*i)) & 255u));
}
std::uint64_t read_le(std::span<const std::byte> bytes,
                      std::size_t offset, unsigned width) {
    if (width != 4 && width != 8) throw std::invalid_argument("field width");
    if (offset > bytes.size() || width > bytes.size()-offset)
        throw std::out_of_range("read bounds");
    std::uint64_t word = 0;
    for (unsigned i = 0; i < width; ++i)
        word |= std::uint64_t(std::to_integer<std::uint8_t>(bytes[offset+i]))<<(8u*i);
    return word;
}

constexpr std::array<std::uint8_t,8> magic{'N','Q','T','N','S','R','0','1'};
constexpr std::uint64_t element_limit = 4096;
struct TensorImage {
    std::uint64_t rows, cols;
    std::vector<std::uint32_t> words;
};

std::vector<std::byte> write_tensor(const TensorImage& tensor) {
    const auto count = checked_product(tensor.rows,tensor.cols);
    if (!tensor.rows || !tensor.cols || count > element_limit ||
        count != tensor.words.size()) throw std::invalid_argument("shape");
    std::vector<std::byte> bytes;
    for (auto c : magic) bytes.push_back(static_cast<std::byte>(c));
    append_le(bytes,1,4);
    append_le(bytes,tensor.rows,8);
    append_le(bytes,tensor.cols,8);
    for (auto word : tensor.words) append_le(bytes,word,4);
    return bytes;
}

TensorImage read_tensor(std::span<const std::byte> bytes) {
    if (bytes.size() < 28) throw std::out_of_range("short header");
    for (std::size_t i = 0; i < magic.size(); ++i)
        if (std::to_integer<std::uint8_t>(bytes[i]) != magic[i])
            throw std::invalid_argument("magic");
    if (read_le(bytes,8,4) != 1) throw std::invalid_argument("version");
    TensorImage tensor{read_le(bytes,12,8),read_le(bytes,20,8),{}};
    if (!tensor.rows || !tensor.cols) throw std::invalid_argument("empty shape");
    const auto count = checked_product(tensor.rows,tensor.cols);
    const auto payload = checked_product(count,4);
    if (count > element_limit) throw std::length_error("resource limit");
    if (payload != bytes.size()-28) throw std::out_of_range("payload length");
    tensor.words.reserve(static_cast<std::size_t>(count)); // Small proved bound.
    for (std::size_t i = 0; i < count; ++i)
        tensor.words.push_back(static_cast<std::uint32_t>(read_le(bytes,28+4*i,4)));
    return tensor;
}

std::uint32_t crc32(std::span<const std::byte> bytes) {
    std::uint32_t crc = 0xFFFFFFFFu;
    for (auto b : bytes) {
        crc ^= std::to_integer<std::uint8_t>(b);
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xEDB88320u : 0u);
    }
    return crc ^ 0xFFFFFFFFu;
}
std::uint8_t additive(std::span<const std::byte> bytes) {
    std::uint8_t sum = 0;
    for (auto b : bytes)
        sum = static_cast<std::uint8_t>(sum + std::to_integer<std::uint8_t>(b));
    return sum;
}

std::string hex_dump(std::span<const std::byte> bytes, std::size_t columns=16) {
    if (!columns || columns > 256) throw std::invalid_argument("dump columns");
    std::ostringstream out; // Local stream: does not alter caller formatting.
    out << std::hex << std::setfill('0');
    for (std::size_t offset = 0; offset < bytes.size();) {
        const auto width = std::min(columns,bytes.size()-offset);
        out << std::setw(8) << offset << "  ";
        for (std::size_t i = 0; i < width; ++i)
            out << std::setw(2) << std::to_integer<unsigned>(bytes[offset+i]) << ' ';
        out << '\n';
        offset += width; // At most size, no offset+columns wrap.
    }
    return out.str();
}

void laboratory() {
    m005::EnvironmentScope environment;
    if (std::fesetround(FE_TONEAREST) != 0) throw std::runtime_error("nearest mode");
    assert(from_double(3.25) == 212992);
    assert(from_double(0.5,0) == 1 && from_double(-0.5,0) == -1);
    assert(multiply(384,576,8) == 864);
    assert(divide(384,576,8) == 171);
    assert(round_ratio(1,2)==0 && round_ratio(3,2)==2);
    assert(round_ratio(-1,2)==0 && round_ratio(-3,2)==-2);
    assert(round_ratio(-5,2)==-2 && round_ratio(-7,2)==-4);
    assert(round_ratio(std::numeric_limits<std::int64_t>::min(),1) ==
           std::numeric_limits<std::int64_t>::min());
    expect_exception<std::overflow_error>([]{
        (void)round_ratio(std::numeric_limits<std::int64_t>::min(),-1); });
    expect_exception<std::domain_error>([]{ (void)divide(1,0); });
    expect_exception<std::overflow_error>([]{ (void)multiply(INT32_MAX,INT32_MAX); });
    expect_exception<std::overflow_error>([]{ (void)from_double(1e300); });
    expect_exception<std::invalid_argument>([]{
        (void)from_double(std::numeric_limits<double>::quiet_NaN()); });
    expect_exception<std::invalid_argument>([]{
        (void)from_double(std::numeric_limits<double>::infinity()); });
    expect_exception<std::invalid_argument>([]{ (void)scale(17); });
    assert(1'000'000LL * scale(10) <= INT32_MAX);
    assert(1'000'000LL * scale(11) <= INT32_MAX);
    assert(1'000'000LL * scale(12) > INT32_MAX);

    std::mt19937 generator(606);
    for (unsigned i=0; i<10000; ++i) {
        const auto a = static_cast<std::int32_t>(generator()%200001u)-100000;
        const auto b = static_cast<std::int32_t>(generator()%200001u)-100000;
        const auto f = 8u + generator()%9u;
        // Exact binary64 product in this bounded domain; independent nearbyint.
        const double reference = std::nearbyint(
            (static_cast<double>(a)*static_cast<double>(b))/static_cast<double>(scale(f)));
        assert(multiply(a,b,f) == static_cast<std::int32_t>(reference));
    }
    for (unsigned a=0; a<16; ++a) for (unsigned b=0; b<16; ++b) {
        const std::array<std::uint8_t,2> codes{std::uint8_t(a),std::uint8_t(b)};
        const auto bytes = pack(codes);
        assert(bytes.size()==1 && bytes[0]==a+16*b);
        assert(unpack(bytes,2)==std::vector<std::uint8_t>(codes.begin(),codes.end()));
    }
    const std::array<double,5> weights{-7,-3.2,0,2.9,6.8};
    std::vector<std::uint8_t> codes;
    for (double x : weights) codes.push_back(int4_code(x,1));
    assert((codes==std::vector<std::uint8_t>{1,5,8,11,15}));
    assert((pack(codes)==std::vector<std::uint8_t>{0x51,0xB8,0x0F}));
    for (std::size_t i=0; i<weights.size(); ++i)
        assert(std::abs(double(int(codes[i])-8)-weights[i]) <= 0.5);
    assert(int4_code(1,100.0/7)==8);
    assert(int4_code(-3.5,1)==4 && int4_code(-2.5,1)==6);
    assert(pack({}).empty() && unpack({},0).empty());
    expect_exception<std::invalid_argument>([]{
        const std::array<std::uint8_t,1> bad{16}; (void)pack(bad); });
    expect_exception<std::invalid_argument>([]{
        const std::array<std::uint8_t,1> bad{0xF1}; (void)unpack(bad,1); });
    expect_exception<std::invalid_argument>([]{ (void)int4_code(1,0); });
    expect_exception<std::invalid_argument>([]{ (void)int4_code(NAN,1); });
    assert(5/2+5%2 + 4*(5/32+(5%32!=0)) == 7);

    const std::array<double,4> row{-1,-3,2,4};
    double negative_sum=0, positive_sum=0;
    unsigned negatives=0, positives=0;
    for (double x : row) {
        if (x<0) { negative_sum+=x; ++negatives; }
        else { positive_sum+=x; ++positives; }
    }
    const double negative_mean=negative_sum/negatives, positive_mean=positive_sum/positives;
    double squares=0;
    for (double x : row) {
        const double reconstructed=x<0 ? negative_mean : positive_mean;
        squares+=(x-reconstructed)*(x-reconstructed);
    }
    assert(negative_mean==-2 && positive_mean==3 && std::sqrt(squares/row.size())==1);
    assert(unorm8(0)==0 && unorm8(255)==1 && unorm8(128)==128.0/255);
    assert(encode_unorm8(0.5)==128);
    assert(snorm8(-128)==-1 && snorm8(-127)==-1 && snorm8(127)==1);
    assert(half_value(0x3E00)==1.5f && half_value(0x7BFF)==65504);
    assert(half_value(1)==std::ldexp(1.0f,-24));
    assert(half_bits(1.5f)==0x3E00 && half_bits(100000)==0x7C00);
    assert(bfloat_bits(std::bit_cast<float>(0x3F808000u))==0x3F80);
    assert(bfloat_bits(std::bit_cast<float>(0x3F818000u))==0x3F82);
    assert(bfloat_bits(std::bit_cast<float>(0x7F800001u))==0x7FC0);
    assert(bfloat_bits(-0.0f)==0x8000 && bfloat_bits(INFINITY)==0x7F80);
    unsigned half_finite=0, bf_finite=0;
    for (std::uint32_t word=0; word<65536; ++word) {
        const auto h=static_cast<std::uint16_t>(word);
        if ((word&0x7C00u)!=0x7C00u) {
            assert(half_bits(half_value(h))==h); ++half_finite;
        }
        if ((word&0x7F80u)!=0x7F80u) {
            assert(bfloat_bits(bfloat_value(h))==h); ++bf_finite;
        }
    }

    TensorImage tensor{2,3,{0x3F800000,0xC0000000,0x3F000000,
                            0x40500000,0,0x41200000}};
    const std::array<std::uint8_t,52> golden{
        0x4E,0x51,0x54,0x4E,0x53,0x52,0x30,0x31,1,0,0,0,
        2,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,
        0,0,0x80,0x3F,0,0,0,0xC0,0,0,0,0x3F,
        0,0,0x50,0x40,0,0,0,0,0,0,0x20,0x41};
    const auto file = write_tensor(tensor);
    assert(file.size()==golden.size());
    for (std::size_t i=0; i<golden.size(); ++i)
        assert(std::to_integer<std::uint8_t>(file[i])==golden[i]);
    assert(read_tensor(file).words==tensor.words);
    auto odd=file;
    odd.insert(odd.begin(),std::byte{0xAB});
    assert(read_tensor(std::span<const std::byte>(odd).subspan(1)).words==tensor.words);
    const TensorImage raw{1,4,{0x80000000,0x7F800000,0x7FC00001,0x7F800001}};
    assert(read_tensor(write_tensor(raw)).words==raw.words);
    for (std::size_t n=0; n<file.size(); ++n)
        expect_exception<std::out_of_range>([&]{ (void)read_tensor(
            std::span<const std::byte>(file).first(n)); });
    auto bad=file; bad[0]=std::byte{0};
    expect_exception<std::invalid_argument>([&]{ (void)read_tensor(bad); });
    bad=file; bad[8]=std::byte{2};
    expect_exception<std::invalid_argument>([&]{ (void)read_tensor(bad); });
    bad=file; for (unsigned i=12;i<28;++i) bad[i]=std::byte{0xFF};
    expect_exception<std::overflow_error>([&]{ (void)read_tensor(bad); });
    bad=file; for (unsigned i=12;i<20;++i) bad[i]=std::byte{0};
    expect_exception<std::invalid_argument>([&]{ (void)read_tensor(bad); });
    bad=file; bad[12]=std::byte{1}; bad[13]=std::byte{0x10};
    expect_exception<std::length_error>([&]{ (void)read_tensor(bad); });
    bad=file; bad.push_back(std::byte{0});
    expect_exception<std::out_of_range>([&]{ (void)read_tensor(bad); });
    expect_exception<std::out_of_range>([&]{
        (void)read_le(file,std::numeric_limits<std::size_t>::max(),4); });
    expect_exception<std::invalid_argument>([&]{ (void)hex_dump(file,0); });
    assert(hex_dump(file).find("00000000  4e 51 54 4e")==0);

    const std::array<std::byte,9> check{
        std::byte{'1'},std::byte{'2'},std::byte{'3'},std::byte{'4'},
        std::byte{'5'},std::byte{'6'},std::byte{'7'},std::byte{'8'},std::byte{'9'}};
    assert(crc32(check)==0xCBF43926u);
    const std::array<std::byte,2> first{std::byte{1},std::byte{2}};
    const std::array<std::byte,2> second{std::byte{2},std::byte{1}};
    assert(additive(first)==additive(second) && crc32(first)!=crc32(second));
    const auto crc=crc32(file);
    for (std::size_t i=0; i<file.size(); ++i) for (unsigned bit=0;bit<8;++bit) {
        auto changed=file;
        changed[i]^=static_cast<std::byte>(1u<<bit);
        assert(crc32(changed)!=crc);
    }
    bad=file; bad[30]=std::byte{0};
    assert(read_tensor(bad).words[0]==0x3F000000 && crc32(bad)!=crc);
    std::cout << "PASS M006: 10000 bounded fixed products; 256 nibble pairs; "
              << half_finite << " finite half and " << bf_finite
              << " finite BF16 round trips; golden/odd-offset/raw-word files; "
              << "malformed shapes and bounds; 416 single-bit CRC checks\n";
}
} // namespace m006
void run_m006_lab() {
    const int mode = std::fegetround();
    const int flags = std::fetestexcept(FE_ALL_EXCEPT);
    m006::laboratory();
    assert(std::fegetround() == mode);
    assert(std::fetestexcept(FE_ALL_EXCEPT) == flags);
}
```


Limits: tests assume the M004 host format/strict environment; non-finite summation is outside these sum contracts. The weighted example is for small safe finite fixtures, not an overflow-proof general production update. No parallel atomic test, real depth-frame corpus, mesh extraction, cross-platform bit identity, performance benchmark or live repository invocation was performed.
