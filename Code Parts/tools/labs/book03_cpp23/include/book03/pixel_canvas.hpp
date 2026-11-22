#pragma once

// From Geometry to Images — Chapter 01.
// This header deliberately uses standard C++23 only. Its job is not to be a
// production renderer: it is an auditable bridge from image addresses to bytes.
//
// Coordinate contract:
//   * (0, 0) is the top-left pixel; x increases rightward, y downward.
//   * Pixel indices are integers. Sample centers are (x + 0.5, y + 0.5).
//   * Rectangles have half-open bounds [x0, x1) by [y0, y1).
//   * RGB8 values are *encoded* color channels, not linear radiance.
//   * A P6 PPM stores red, green, blue bytes explicitly in row-major order.
//
// Negative inputs and impossible dimensions are rejected before indexing or
// allocation. No callers are permitted to infer the byte representation of
// an in-memory C++ struct from a file's wire format.

#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace book03 {

static_assert(CHAR_BIT == 8, "Book 03's PPM laboratory requires 8-bit bytes.");

struct Rgb8 {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};

    [[nodiscard]] friend constexpr bool operator==(Rgb8, Rgb8) = default;
};

struct Pixel {
    int x{};
    int y{};
};

struct SampleCenter {
    double x{};
    double y{};
};

class Canvas {
public:
    // The bounded allocation makes input validation teachable and prevents
    // unchecked width*height multiplication. These are lab limits, not PPM
    // format limits and not general GPU texture limits.
    static constexpr std::size_t maximum_dimension = 8192;
    static constexpr std::size_t maximum_pixels = 16'777'216;

    explicit Canvas(std::size_t width, std::size_t height,
                    Rgb8 clear = Rgb8{0, 0, 0})
        : width_(validated_width(width, height)),
          height_(height),
          pixels_(width_ * height_, clear) {}

    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] std::size_t size() const noexcept { return pixels_.size(); }

    // The center is a point in continuous image space, not a memory address.
    // Use an explicitly validated pixel so a bogus coordinate cannot become
    // a superficially plausible continuous location.
    [[nodiscard]] SampleCenter center(Pixel point) const {
        (void)index_of(point);
        return {static_cast<double>(point.x) + 0.5,
                static_cast<double>(point.y) + 0.5};
    }

    [[nodiscard]] Rgb8 at(Pixel point) const {
        return pixels_[index_of(point)];
    }

    void set(Pixel point, Rgb8 value) {
        pixels_[index_of(point)] = value;
    }

    void clear(Rgb8 value) {
        for (Rgb8& pixel : pixels_) {
            pixel = value;
        }
    }

    // A rectangle names covered integer pixel addresses, not a continuous
    // surface integral. End coordinates are exclusive. A zero-area rectangle
    // is valid and changes no samples.
    void fill_rectangle(int x0, int y0, int x1, int y1, Rgb8 color) {
        if (x0 < 0 || y0 < 0 || x1 < x0 || y1 < y0 ||
            static_cast<std::size_t>(x1) > width_ ||
            static_cast<std::size_t>(y1) > height_) {
            throw std::out_of_range("rectangle must lie within image bounds");
        }

        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                set({x, y}, color);
            }
        }
    }

    // Bresenham-style integer line drawing with inclusive endpoints.
    // Endpoints must be inside the image: no clipping is implied. The
    // comparisons implement a deterministic tie convention; they are NOT a
    // mathematical guarantee of antialiasing or geometric coverage.
    void draw_line(Pixel start, Pixel finish, Rgb8 color) {
        (void)index_of(start);
        (void)index_of(finish);

        int x = start.x;
        int y = start.y;
        const int dx = std::abs(finish.x - x);
        const int dy = std::abs(finish.y - y);
        const int sx = (x < finish.x) ? 1 : -1;
        const int sy = (y < finish.y) ? 1 : -1;
        int error = dx - dy;

        for (;;) {
            set({x, y}, color);
            if (x == finish.x && y == finish.y) {
                break;
            }

            const int doubled_error = 2 * error;
            if (doubled_error > -dy) {
                error -= dy;
                x += sx;
            }
            if (doubled_error < dx) {
                error += dx;
                y += sy;
            }
        }
    }

    // Binary PPM P6: a small, deliberately transparent file representation.
    // Neither C++ struct padding nor native-endian integers are serialized.
    // Every pixel writes exactly three bytes in R, G, B order.
    void write_ppm(std::ostream& output) const {
        output << "P6\n" << width_ << ' ' << height_ << "\n255\n";
        if (!output) {
            throw std::runtime_error("failed to write PPM header");
        }

        for (const Rgb8 pixel : pixels_) {
            const std::array<char, 3> channels{
                static_cast<char>(pixel.red),
                static_cast<char>(pixel.green),
                static_cast<char>(pixel.blue)};
            output.write(channels.data(),
                         static_cast<std::streamsize>(channels.size()));
            if (!output) {
                throw std::runtime_error("failed to write PPM image data");
            }
        }
    }

private:
    std::size_t width_{};
    std::size_t height_{};
    std::vector<Rgb8> pixels_{};

    static std::size_t validated_width(std::size_t width,
                                       std::size_t height) {
        if (width == 0 || height == 0 ||
            width > maximum_dimension || height > maximum_dimension ||
            width > maximum_pixels / height) {
            throw std::invalid_argument(
                "image dimensions are empty or exceed lab limits");
        }
        return width;
    }

    [[nodiscard]] std::size_t index_of(Pixel point) const {
        // Check negativity BEFORE converting to an unsigned type.
        if (point.x < 0 || point.y < 0 ||
            static_cast<std::size_t>(point.x) >= width_ ||
            static_cast<std::size_t>(point.y) >= height_) {
            throw std::out_of_range("pixel address is outside the canvas");
        }
        return static_cast<std::size_t>(point.y) * width_ +
               static_cast<std::size_t>(point.x);
    }
};

} // namespace book03
