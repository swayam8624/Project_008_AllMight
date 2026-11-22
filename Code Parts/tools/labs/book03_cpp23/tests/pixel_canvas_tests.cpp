// Book 03 / P01 — executable contracts for our first visible picture.
//
// The tests deliberately inspect serialized bytes rather than relying only
// on a viewer screenshot. All fixtures are deterministic; no GPU is needed.

#include "book03/pixel_canvas.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::Canvas;
using book03::Pixel;
using book03::Rgb8;

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(std::string{message});
    }
}

template <typename Exception, typename Operation>
void require_throw(Operation operation, std::string_view message) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(std::string{message});
}

void test_dimensions_and_initial_values() {
    const Canvas canvas{7, 5, {4, 5, 6}};
    require(canvas.width() == 7 && canvas.height() == 5, "dimensions");
    require(canvas.size() == 35, "pixel count");
    require(canvas.at({0, 0}) == Rgb8{4, 5, 6}, "clear value first");
    require(canvas.at({6, 4}) == Rgb8{4, 5, 6}, "clear value last");

    require_throw<std::invalid_argument>(
        [] { (void)Canvas{0, 1}; }, "reject zero width");
    require_throw<std::invalid_argument>(
        [] { (void)Canvas{1, 0}; }, "reject zero height");
    require_throw<std::invalid_argument>(
        [] { (void)Canvas{8193, 1}; }, "reject over-limit width");
    require_throw<std::invalid_argument>(
        [] { (void)Canvas{8192, 8192}; }, "reject excessive area");
    require_throw<std::invalid_argument>(
        [] { (void)Canvas{std::numeric_limits<std::size_t>::max(), 2}; },
        "reject product-overflow candidate before multiplication");
}

void test_pixel_addresses_and_centers() {
    Canvas canvas{7, 5};
    canvas.set({3, 2}, {255, 17, 9});
    require(canvas.at({3, 2}) == Rgb8{255, 17, 9}, "set pixel");
    require(canvas.at({3, 1}) == Rgb8{0, 0, 0}, "row independence");
    const auto center = canvas.center({3, 2});
    require(center.x == 3.5 && center.y == 2.5,
            "continuous pixel center is not integer index");

    require_throw<std::out_of_range>(
        [&] { canvas.set({7, 2}, {0, 0, 0}); }, "x == width rejected");
    require_throw<std::out_of_range>(
        [&] { (void)canvas.at({-1, 0}); }, "negative x rejected");
    require_throw<std::out_of_range>(
        [&] { (void)canvas.center({0, 5}); }, "y == height rejected");
    require_throw<std::out_of_range>(
        [&] { (void)canvas.at({0, std::numeric_limits<int>::min()}); },
        "very negative y rejected");
}

void test_rectangles_are_half_open() {
    Canvas canvas{4, 4};
    canvas.fill_rectangle(1, 1, 3, 3, {14, 15, 16});
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            const bool inside = x >= 1 && x < 3 && y >= 1 && y < 3;
            require(canvas.at({x, y}) ==
                        (inside ? Rgb8{14, 15, 16} : Rgb8{0, 0, 0}),
                    "rectangle did not obey [min,max)");
        }
    }
    canvas.fill_rectangle(2, 2, 2, 4, {99, 99, 99});
    require(canvas.at({2, 2}) == Rgb8{14, 15, 16},
            "empty rectangle must leave pixels unchanged");

    require_throw<std::out_of_range>(
        [&] { canvas.fill_rectangle(-1, 0, 2, 2, {1, 2, 3}); },
        "negative rectangle origin rejected");
    require_throw<std::out_of_range>(
        [&] { canvas.fill_rectangle(0, 0, 5, 3, {1, 2, 3}); },
        "rectangle outside image rejected");
    require_throw<std::out_of_range>(
        [&] { canvas.fill_rectangle(2, 2, 1, 3, {1, 2, 3}); },
        "reversed rectangle rejected");
}

void test_lines_and_endpoints() {
    constexpr Rgb8 yellow{250, 180, 10};
    Canvas canvas{7, 5};
    canvas.draw_line({0, 0}, {6, 4}, yellow);

    // Exact expected path for the chosen Bresenham tie rule.
    constexpr std::array<Pixel, 7> expected{{
        {0, 0}, {1, 1}, {2, 1}, {3, 2}, {4, 3}, {5, 3}, {6, 4}
    }};

    std::size_t colored = 0;
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 7; ++x) {
            bool should_be_colored = false;
            for (const Pixel p : expected) {
                if (p.x == x && p.y == y) {
                    should_be_colored = true;
                }
            }
            const bool is_colored = canvas.at({x, y}) == yellow;
            require(is_colored == should_be_colored, "unexpected line path");
            colored += static_cast<std::size_t>(is_colored);
        }
    }
    require(colored == expected.size(), "line visits expected number of samples");

    Canvas singleton{3, 3};
    singleton.draw_line({2, 1}, {2, 1}, yellow);
    require(singleton.at({2, 1}) == yellow, "zero-length line draws one");
    require(singleton.at({1, 1}) == Rgb8{0, 0, 0},
            "zero-length line must not draw neighbors");

    Canvas all_octants{7, 7};
    for (const Pixel finish : std::array<Pixel, 8>{{
             {6, 3}, {6, 6}, {3, 6}, {0, 6},
             {0, 3}, {0, 0}, {3, 0}, {6, 0}
         }}) {
        all_octants.clear({0, 0, 0});
        all_octants.draw_line({3, 3}, finish, yellow);
        require(all_octants.at({3, 3}) == yellow, "octant start");
        require(all_octants.at(finish) == yellow, "octant end");
    }

    require_throw<std::out_of_range>(
        [&] { all_octants.draw_line({3, 3}, {7, 3}, yellow); },
        "unclipped line outside image must be rejected");
}

void test_ppm_bytes_and_failed_stream() {
    Canvas canvas{3, 2};
    canvas.set({2, 1}, {12, 34, 56});
    std::ostringstream out{std::ios::out | std::ios::binary};
    canvas.write_ppm(out);
    const std::string bytes = out.str();
    const std::string header = "P6\n3 2\n255\n";
    require(bytes.substr(0, header.size()) == header, "P6 header");
    require(bytes.size() == header.size() + 18, "exact RGB payload size");

    // Six pixels, three channels each; (2,1) is the final pixel.
    const std::size_t last = header.size() + 3 * (1 * 3 + 2);
    require(static_cast<unsigned char>(bytes[last]) == 12,
            "red channel byte");
    require(static_cast<unsigned char>(bytes[last + 1]) == 34,
            "green channel byte");
    require(static_cast<unsigned char>(bytes[last + 2]) == 56,
            "blue channel byte");
    for (std::size_t n = header.size(); n < last; ++n) {
        require(static_cast<unsigned char>(bytes[n]) == 0,
                "unmodified RGB bytes must be zero");
    }

    std::ostringstream failed;
    failed.setstate(std::ios::badbit);
    require_throw<std::runtime_error>(
        [&] { canvas.write_ppm(failed); },
        "failed stream must not produce a false success");
}

} // namespace

int main() {
    try {
        test_dimensions_and_initial_values();
        test_pixel_addresses_and_centers();
        test_rectangles_are_half_open();
        test_lines_and_endpoints();
        test_ppm_bytes_and_failed_stream();
        std::cout << "Book03/P01: all pixel, line and PPM contracts passed\n";
    } catch (const std::exception& error) {
        std::cerr << "Book03/P01 CONTRACT FAILURE: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
