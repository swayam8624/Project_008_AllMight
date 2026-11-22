// Portable scalar oracle for independent packed 8-bit lane addition.
// This models three arithmetic policies; it does not select or prove SIMD code.
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <limits>

struct Results {
    std::array<std::uint8_t, 4> wrapping{};
    std::array<std::uint8_t, 4> saturating{};
    std::array<std::uint16_t, 4> widening{};
};

constexpr Results add_lanes(const std::array<std::uint8_t, 4>& a,
                            const std::array<std::uint8_t, 4>& b) {
    Results out{};
    for (std::size_t lane = 0; lane < a.size(); ++lane) {
        // Promote before adding so the exact sum is visible and in range.
        // Unsigned int can represent the largest sum, 255+255.
        const unsigned sum = static_cast<unsigned>(a[lane]) +
                             static_cast<unsigned>(b[lane]);
        const auto exact = static_cast<std::uint16_t>(sum);
        // Narrowing to uint8_t applies the unsigned modulo-2^8 conversion.
        out.wrapping[lane] = static_cast<std::uint8_t>(exact);
        // Saturation is an explicit policy, not ordinary uint8_t addition.
        out.saturating[lane] = static_cast<std::uint8_t>(
            exact > std::numeric_limits<std::uint8_t>::max()
                ? std::numeric_limits<std::uint8_t>::max() : exact);
        // Preserve each lane's exact sum in the wider result type.
        out.widening[lane] = exact;
    }
    return out;
}

template<class T, std::size_t N>
bool equals(const std::array<T, N>& actual, const std::array<T, N>& expected) {
    for (std::size_t i = 0; i < N; ++i)
        if (actual[i] != expected[i]) return false;
    return true;
}

int main() {
    constexpr std::array<std::uint8_t, 4> a{250, 1, 128, 0};
    constexpr std::array<std::uint8_t, 4> b{10, 255, 128, 7};
    constexpr auto r = add_lanes(a, b);
    constexpr std::array<std::uint8_t, 4> wrap{4, 0, 0, 7};
    constexpr std::array<std::uint8_t, 4> sat{255, 255, 255, 7};
    constexpr std::array<std::uint16_t, 4> wide{260, 256, 256, 7};
    // Keep checks active under NDEBUG: no assert-only test contract.
    if (!equals(r.wrapping, wrap) || !equals(r.saturating, sat) ||
        !equals(r.widening, wide)) {
        std::cerr << "packed-lane scalar oracle mismatch\n";
        return 1;
    }
    // Reproduce the solved exercise independently of the first example.
    constexpr auto exercise = add_lanes({15, 240, 127, 250}, {20, 30, 1, 10});
    if (exercise.wrapping != std::array<std::uint8_t, 4>{35, 14, 128, 4} ||
        exercise.saturating != std::array<std::uint8_t, 4>{35, 255, 128, 255} ||
        exercise.widening != std::array<std::uint16_t, 4>{35, 270, 128, 260}) {
        std::cerr << "exercise arithmetic mismatch\n";
        return 1;
    }
    std::cout << "packed lane scalar oracle: PASS\n";
    return 0;
}
