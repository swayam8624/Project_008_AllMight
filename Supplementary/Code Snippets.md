# Code Snippets

Canonical teaching code and verified repository excerpts live here. Organize by concept or subsystem, not by incoming chunk. Update existing sections when later chunks add variants, optimizations, tests, or failure handling.

For every snippet, label it as one of:

- `Canonical` - minimal explanatory implementation;
- `Repository-verified` - copied or adapted from an inspected repository location;
- `Pseudocode` - intentionally non-compilable structure;
- `To verify` - a provisional claim that must not be presented as repository fact.

Record language/version, inputs, outputs, preconditions, ownership and lifetime where relevant, complexity, failure behavior, and verification performed. Keep long repository listings out; quote only the lines needed to prove the point and link the exact anchor in `Sources and Code Anchors.md`.

## M001 - Width-safe fields and LSB-first bit packing

**Kind:** Canonical
**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 / M001]]
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
**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 / M002]]
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

**Kind:** Canonical · **Language:** C++23 · **Source:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 / M003]].

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

int main() {
    run_m004_lab();
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
