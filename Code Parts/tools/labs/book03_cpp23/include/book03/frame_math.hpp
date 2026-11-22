#pragma once

// Book 03 / Chapter 02 — A point is a place, not a displacement.
//
// Point2 and Vector2 distinguish places from displacements, not one named
// coordinate frame from another; local/world meaning is defined by each
// API boundary. Frame-tagged types are a later extension.
// This small 2D affine model is a CPU *reference* for coordinate reasoning.
// It is intentionally independent of graphics APIs or C++ matrix libraries.
// Axes are the world displacement per one unit of local coordinate; our
// teaching examples use metres for local and world positions. All algebraic
// vectors here are columns. Memory storage order is irrelevant to the math.
//
// Frame2::local_to_world(local) = origin + axis_x * local.x
//                                         + axis_y * local.y
// Frame2::world_to_local(world) solves that exact affine relation.
// ImageMap2 maps world X right / Y up (metres) to image u right / v down
// (continuous pixels). It is NOT a 3D perspective camera.
//
// Numeric limitations: double arithmetic, an explicitly conservative inverse
// conditioning guard, finite inputs only. The guard is a teaching constraint,
// not a substitute for the full condition-number studies in Book 02.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>

namespace book03 {

// A displacement or axis coefficient in the declared coordinate frame.
struct Vector2 {
    double x{};
    double y{};

    [[nodiscard]] friend constexpr bool operator==(
        const Vector2&, const Vector2&) = default;
};

// A position in the frame specified by the calling API. This type does not
// itself encode whether that frame is local, parent, or world.
struct Point2 {
    double x{};
    double y{};

    [[nodiscard]] friend constexpr bool operator==(
        const Point2&, const Point2&) = default;
};

// Deliberately NO point + point operator. Points represent places. Their
// difference is a displacement, and a displacement can move a place.
[[nodiscard]] constexpr Vector2 operator-(Point2 a, Point2 b) {
    return {a.x - b.x, a.y - b.y};
}

[[nodiscard]] constexpr Point2 operator+(Point2 p, Vector2 d) {
    return {p.x + d.x, p.y + d.y};
}

[[nodiscard]] constexpr Point2 operator-(Point2 p, Vector2 d) {
    return {p.x - d.x, p.y - d.y};
}

[[nodiscard]] constexpr Vector2 operator+(Vector2 a, Vector2 b) {
    return {a.x + b.x, a.y + b.y};
}

[[nodiscard]] constexpr Vector2 operator-(Vector2 a, Vector2 b) {
    return {a.x - b.x, a.y - b.y};
}

[[nodiscard]] constexpr Vector2 operator*(double s, Vector2 v) {
    return {s * v.x, s * v.y};
}

[[nodiscard]] constexpr Point2 midpoint(Point2 a, Point2 b) {
    // An affine combination: point + half of the displacement.
    return a + 0.5 * (b - a);
}

[[nodiscard]] inline bool finite(Point2 p) noexcept {
    return std::isfinite(p.x) && std::isfinite(p.y);
}
[[nodiscard]] inline bool finite(Vector2 v) noexcept {
    return std::isfinite(v.x) && std::isfinite(v.y);
}

[[nodiscard]] inline double degrees_to_radians(double degrees) {
    if (!std::isfinite(degrees)) {
        throw std::invalid_argument("nonfinite rotation angle");
    }
    // Standard C++ library pi instead of relying on nonstandard M_PI.
    return degrees * 0.01745329251994329576923690768488612713443;
}

struct Frame2 {
    // Frame origin is a WORLD point. Basis columns map LOCAL coordinate
    // displacements to WORLD displacements. An axis coefficient is the world
    // displacement per +1 unit of the corresponding local coordinate.
    Point2 origin{};
    Vector2 axis_x{1.0, 0.0};
    Vector2 axis_y{0.0, 1.0};

    [[nodiscard]] static Frame2 rotation(Point2 where, double radians) {
        if (!finite(where) || !std::isfinite(radians)) {
            throw std::invalid_argument("rotation requires finite parameters");
        }
        const double c = std::cos(radians);
        const double s = std::sin(radians);
        Frame2 frame{where, {c, s}, {-s, c}};
        frame.validate();
        return frame;
    }

    [[nodiscard]] double determinant() const noexcept {
        return axis_x.x * axis_y.y - axis_y.x * axis_x.y;
    }

    void validate() const {
        if (!finite(origin) || !finite(axis_x) || !finite(axis_y)) {
            throw std::invalid_argument("frame contains nonfinite values");
        }
        const double p = axis_x.x * axis_y.y;
        const double q = axis_y.x * axis_x.y;
        const double det = p - q;
        if (!std::isfinite(det)) {
            throw std::invalid_argument("frame determinant is nonfinite");
        }

        // For introductory use, reject axes whose determinant is tiny
        // relative to the magnitudes of the determinant's two terms.
        // It also rejects very small absolute scales, intentionally.
        const double scale = std::max({1.0, std::abs(p), std::abs(q)});
        if (std::abs(det) <= 1e-12 * scale) {
            throw std::invalid_argument("frame axes are singular or unstable");
        }
    }

    [[nodiscard]] Vector2 transform_vector(Vector2 local) const {
        validate();
        if (!finite(local)) {
            throw std::invalid_argument("nonfinite local vector");
        }
        Vector2 world{
            axis_x.x * local.x + axis_y.x * local.y,
            axis_x.y * local.x + axis_y.y * local.y};
        if (!finite(world)) {
            throw std::overflow_error("transformed vector is nonfinite");
        }
        return world; // Crucially NO translation.
    }

    [[nodiscard]] Point2 local_to_world(Point2 local) const {
        if (!finite(local)) {
            throw std::invalid_argument("nonfinite local point");
        }
        const Vector2 movement =
            transform_vector({local.x, local.y});
        const Point2 world = origin + movement;
        if (!finite(world)) {
            throw std::overflow_error("transformed point is nonfinite");
        }
        return world;
    }

    [[nodiscard]] Point2 world_to_local(Point2 world) const {
        validate();
        if (!finite(world)) {
            throw std::invalid_argument("nonfinite world point");
        }
        const Vector2 q = world - origin;
        const double det = determinant();
        const Point2 local{
            ( axis_y.y * q.x - axis_y.x * q.y) / det,
            (-axis_x.y * q.x + axis_x.x * q.y) / det};
        if (!finite(local)) {
            throw std::overflow_error("inverse-transformed point nonfinite");
        }
        return local;
    }

    // Composition means "child first, parent second":
    // combined(local) == parent(child(local)).
    [[nodiscard]] static Frame2 compose(const Frame2& parent,
                                        const Frame2& child) {
        parent.validate();
        child.validate();
        const Frame2 result{
            parent.local_to_world(child.origin),
            parent.transform_vector(child.axis_x),
            parent.transform_vector(child.axis_y)};
        result.validate();
        return result;
    }
};

struct ImagePosition2 {
    // Continuous image-plane coordinate, measured in pixels, not an index.
    double u{};
    double v{};
};

struct ImagePixel2 {
    std::size_t column{};
    std::size_t row{};
    [[nodiscard]] friend constexpr bool operator==(
        const ImagePixel2&, const ImagePixel2&) = default;
};

struct ImageMap2 {
    // Position of world (0,0) in the CONTINUOUS pixel coordinate system.
    ImagePosition2 world_origin{0.5, 0.5};
    // Positive scale values; world Y up maps to image v down.
    double pixels_per_metre_x{1.0};
    double pixels_per_metre_y{1.0};
    std::size_t width{640};
    std::size_t height{480};

    void validate() const {
        // The 8192 maximum is a classroom guard, not a format/API limit.
        if (width == 0 || height == 0 ||
            width > 8192 || height > 8192 ||
            !std::isfinite(world_origin.u) ||
            !std::isfinite(world_origin.v) ||
            !std::isfinite(pixels_per_metre_x) ||
            !std::isfinite(pixels_per_metre_y) ||
            pixels_per_metre_x <= 0.0 ||
            pixels_per_metre_y <= 0.0) {
            throw std::invalid_argument("invalid orthographic image mapping");
        }
    }

    [[nodiscard]] ImagePosition2 world_to_image(Point2 world) const {
        validate();
        if (!finite(world)) {
            throw std::invalid_argument("nonfinite world input");
        }
        const ImagePosition2 image{
            world_origin.u + pixels_per_metre_x * world.x,
            world_origin.v - pixels_per_metre_y * world.y};
        if (!std::isfinite(image.u) || !std::isfinite(image.v)) {
            throw std::overflow_error("image coordinate overflow");
        }
        return image;
    }

    [[nodiscard]] Point2 image_to_world(ImagePosition2 image) const {
        validate();
        if (!std::isfinite(image.u) || !std::isfinite(image.v)) {
            throw std::invalid_argument("nonfinite image position");
        }
        const Point2 world{
            (image.u - world_origin.u) / pixels_per_metre_x,
            (world_origin.v - image.v) / pixels_per_metre_y};
        if (!finite(world)) {
            throw std::overflow_error("world coordinate overflow");
        }
        return world;
    }

    [[nodiscard]] std::optional<ImagePixel2> pixel_at(
        ImagePosition2 image) const {
        validate();
        if (!std::isfinite(image.u) || !std::isfinite(image.v) ||
            image.u < 0.0 || image.v < 0.0 ||
            image.u >= static_cast<double>(width) ||
            image.v >= static_cast<double>(height)) {
            return std::nullopt;
        }
        // Bounds were checked BEFORE conversion to an unsigned index.
        return ImagePixel2{
            static_cast<std::size_t>(std::floor(image.u)),
            static_cast<std::size_t>(std::floor(image.v))};
    }

    [[nodiscard]] ImagePosition2 center(ImagePixel2 pixel) const {
        validate();
        if (pixel.column >= width || pixel.row >= height) {
            throw std::out_of_range("pixel center outside image");
        }
        return {static_cast<double>(pixel.column) + 0.5,
                static_cast<double>(pixel.row) + 0.5};
    }
};

} // namespace book03
