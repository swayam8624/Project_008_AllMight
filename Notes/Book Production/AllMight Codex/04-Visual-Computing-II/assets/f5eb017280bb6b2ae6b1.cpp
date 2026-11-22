// Project AllMight, Phase 4 / M014 (Parts 217-230).
// One asset-tile planning problem exposes compile-time evaluation, policy choice,
// static dispatch, and object-layout consequences. This is a CPU-only witness.
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message); // survives -DNDEBUG
}

enum class PixelFormat { R8, RGB8, RGBA8 };

// A type trait represents compile-time information as a type with a value.
// Each specialization defines only the supported format's channel count.
template <PixelFormat Format> struct PixelTraits;
template <> struct PixelTraits<PixelFormat::R8>
    : std::integral_constant<std::size_t, 1> {};
template <> struct PixelTraits<PixelFormat::RGB8>
    : std::integral_constant<std::size_t, 3> {};
template <> struct PixelTraits<PixelFormat::RGBA8>
    : std::integral_constant<std::size_t, 4> {};
static_assert(PixelTraits<PixelFormat::RGB8>::value == 3);
static_assert(std::is_base_of_v<std::integral_constant<std::size_t, 4>,
                                PixelTraits<PixelFormat::RGBA8>>);

struct TileDefinition { std::size_t width, height; };

// Immediate functions require compile-time evaluation for these calls.
// An invalid compile-time configuration must fail during compilation.
consteval TileDefinition make_tile_definition(std::size_t width,
                                               std::size_t height) {
    if (width == 0 || height == 0 || width > 64 || height > 64)
        throw "tile dimensions must be in [1, 64]";
    return {width, height};
}
inline constexpr TileDefinition kDefaultTile = make_tile_definition(8, 8);
static_assert(kDefaultTile.width * kDefaultTile.height == 64);

// A constexpr function can execute during constant evaluation OR at runtime.
// Its preconditions are checked in both cases. No unguarded multiplication.
constexpr std::size_t checked_product(std::size_t x, std::size_t y) {
    if (y != 0 && x > std::numeric_limits<std::size_t>::max() / y)
        throw std::overflow_error("byte-count multiplication overflow");
    return x * y;
}
static_assert(checked_product(8, 4) == 32);

// A 4-bit coordinate is spread into the even bit positions of one byte:
// abcd -> 0a0b0c0d. Two spread coordinates can be interleaved into a
// Morton/Z-order tile code, useful for addressing small 2D tile blocks.
constexpr std::uint8_t spread4(std::uint8_t n) noexcept {
    std::uint8_t result = 0;
    for (unsigned bit = 0; bit != 4; ++bit)
        result |= static_cast<std::uint8_t>(((n >> bit) & 1u) << (2u * bit));
    return result;
}

consteval std::array<std::uint8_t, 16> make_morton_table() {
    std::array<std::uint8_t, 16> result{};
    for (std::size_t n = 0; n < result.size(); ++n)
        result[n] = spread4(static_cast<std::uint8_t>(n));
    return result;
}
inline constexpr auto kSpread4 = make_morton_table();
static_assert(kSpread4[3] == 5 && kSpread4[5] == 17);

constexpr std::uint8_t morton4(std::uint8_t x, std::uint8_t y) {
    if (x >= 16 || y >= 16) throw std::out_of_range("4-bit Morton coordinate");
    return static_cast<std::uint8_t>(kSpread4[x] | (kSpread4[y] << 1));
}
static_assert(morton4(3, 5) == 39);

// constinit promises initialization at static initialization time, not constness
// and not thread safety. This demonstrator is used on a single thread only.
constinit std::uint32_t g_plans_recorded = 0;

// A stateless policy means no application-specific budget, not unchecked math.
// All paths still validate overflow, payload length and format constraints.
struct NoBudgetPolicy {
    void validate(std::size_t) const noexcept {}
};
struct ByteBudgetPolicy {
    std::size_t maximum;
    void validate(std::size_t count) const {
        if (count > maximum) throw std::length_error("tile exceeds byte budget");
    }
};
static_assert(std::is_empty_v<NoBudgetPolicy>);
static_assert(!std::is_empty_v<ByteBudgetPolicy>);

struct TilePlan {
    std::size_t width, height, bytes;
    std::uint8_t example_morton;
    bool alpha;
};

// The CRTP base calls a derived implementation at compile-time-known type.
// It is not a virtual interface and does not provide runtime extensibility.
template <class Derived> struct RecorderBase {
    void record(const TilePlan& plan) {
        static_cast<Derived&>(*this).record_impl(plan);
    }
};
struct CountingRecorder : RecorderBase<CountingRecorder> {
    std::size_t calls = 0, total_bytes = 0;
    void record_impl(const TilePlan& plan) {
        ++calls;
        total_bytes += plan.bytes;
    }
};

// Tags select distinct operations at compile time using ordinary overloads.
struct VerifyPayloadTag {};
struct MetadataOnlyTag {};
void check_payload(std::span<const std::byte> pixels,
                   std::size_t expected, VerifyPayloadTag) {
    if (pixels.size() != expected)
        throw std::invalid_argument("pixel data length does not match plan");
}
void check_payload(std::span<const std::byte>,
                   std::size_t, MetadataOnlyTag) noexcept {
    // This mode produces only a manifest and never asserts buffer readiness.
}

// Template policies are selected at compile time. A no_unique_address member
// allows an empty NoBudgetPolicy to overlap other storage where permitted.
template <PixelFormat Format, class BudgetPolicy, class Recorder>
class TilePlanner {
    [[no_unique_address]] BudgetPolicy budget_;
    Recorder recorder_;
public:
    TilePlanner(BudgetPolicy budget, Recorder recorder)
        : budget_(std::move(budget)), recorder_(std::move(recorder)) {}

    template <class Tag>
    TilePlan prepare(std::size_t width, std::size_t height,
                     std::span<const std::byte> payload, Tag tag) {
        if (width == 0 || height == 0)
            throw std::invalid_argument("tile dimensions cannot be zero");
        const std::size_t pixels = checked_product(width, height);
        const std::size_t bytes =
            checked_product(pixels, PixelTraits<Format>::value);
        budget_.validate(bytes);
        check_payload(payload, bytes, tag);

        // This branch is discarded during instantiation for non-alpha formats.
        // It is an actual compile-time format decision, not a runtime if.
        bool alpha = false;
        if constexpr (Format == PixelFormat::RGBA8) {
            alpha = true;
        }
        TilePlan plan{width, height, bytes, morton4(3, 5), alpha};
        recorder_.record(plan); // static CRTP call into the concrete recorder
        ++g_plans_recorded;
        return plan;
    }

    const Recorder& recorder() const noexcept { return recorder_; }
};

// Classic empty-base optimization (EBO), contrasted with regular and
// C++20 potentially-overlapping members. Sizes are observed, not mandated.
struct PlainPolicyHolder { NoBudgetPolicy policy; std::size_t counter; };
struct EboPolicyHolder : private NoBudgetPolicy { std::size_t counter; };
struct CompactPolicyHolder {
    [[no_unique_address]] NoBudgetPolicy policy;
    std::size_t counter;
};

void run() {
    require(kDefaultTile.width == 8, "consteval configuration");
    require(morton4(3, 5) == 39, "Morton table and composition");
    require(checked_product(7, 6) == 42, "constexpr function at runtime");
    const std::size_t value = 13;
    require(checked_product(value, 3) == 39, "runtime constexpr call");

    using RgbaPlanner = TilePlanner<PixelFormat::RGBA8,
                                    ByteBudgetPolicy, CountingRecorder>;
    RgbaPlanner limited{ByteBudgetPolicy{64}, CountingRecorder{}};
    std::vector<std::byte> rgba(8 * 2 * 4, std::byte{0x7f});
    TilePlan accepted = limited.prepare(8, 2, rgba, VerifyPayloadTag{});
    require(accepted.bytes == 64 && accepted.alpha,
            "RGBA tile validated and recorded");
    require(limited.recorder().calls == 1 &&
            limited.recorder().total_bytes == 64, "static CRTP recorder");

    bool too_large = false;
    try {
        (void)limited.prepare(8, 3, {}, MetadataOnlyTag{}); // 96 > 64
    } catch (const std::length_error&) { too_large = true; }
    require(too_large && limited.recorder().calls == 1,
            "budget failure must not publish a plan");

    bool wrong_payload = false;
    try {
        (void)limited.prepare(4, 4, {}, VerifyPayloadTag{}); // 64 required
    } catch (const std::invalid_argument&) { wrong_payload = true; }
    require(wrong_payload && limited.recorder().calls == 1,
            "payload failure leaves recorder unchanged");

    TilePlanner<PixelFormat::RGB8, NoBudgetPolicy, CountingRecorder>
        manifest{NoBudgetPolicy{}, CountingRecorder{}};
    auto metadata = manifest.prepare(4, 4, {}, MetadataOnlyTag{});
    require(metadata.bytes == 48 && !metadata.alpha,
            "manifest is a plan only, not a verified upload");
    require(manifest.recorder().calls == 1,
            "second static policy and recorder instance");

    bool overflow = false;
    try {
        (void)manifest.prepare(std::numeric_limits<std::size_t>::max(),
                               2, {}, MetadataOnlyTag{});
    } catch (const std::overflow_error&) { overflow = true; }
    require(overflow && manifest.recorder().calls == 1,
            "overflow checked before byte multiplication");

    // Runtime type erasure has a different tradeoff from the statically
    // selected CRTP Recorder. It can hold a runtime-selected callable target.
    std::size_t observed = 0;
    std::function<void(const TilePlan&)> erased =
        [&observed](const TilePlan& plan) { observed += plan.bytes; };
    erased(metadata);
    require(observed == 48, "type-erased runtime callable");
    erased = {}; // release its captured target

    require(g_plans_recorded == 2, "constinit counter updated by accepted plans");
    require(sizeof(PlainPolicyHolder) >= sizeof(std::size_t),
            "ordinary member layout observed");
    require(sizeof(EboPolicyHolder) >= sizeof(std::size_t),
            "EBO layout observed");
    require(sizeof(CompactPolicyHolder) >= sizeof(std::size_t),
            "no_unique_address layout observed");
    std::cout << "Layout bytes (plain/EBO/member): "
              << sizeof(PlainPolicyHolder) << '/' << sizeof(EboPolicyHolder)
              << '/' << sizeof(CompactPolicyHolder) << '\n';
    std::cout << "compile_time_asset_policy_lab: all checks passed\n";
}
} // namespace allmight

int main() {
    try { allmight::run(); }
    catch (const std::exception& e) {
        std::cerr << "compile_time_asset_policy_lab failed: " << e.what() << '\n';
        return 1;
    }
}
