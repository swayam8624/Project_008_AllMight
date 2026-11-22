// Book 03 / P02 — independent numerical and failure-case witnesses.
//
// Floating-point trigonometry is checked with declared tolerances, while
// discrete image-cell ownership and interface type distinctions are exact.
// This is 2D affine geometry, not a 3D camera or a GPU shader validation.

#include "book03/frame_math.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace {

using book03::Frame2;
using book03::ImageMap2;
using book03::ImagePixel2;
using book03::ImagePosition2;
using book03::Point2;
using book03::Vector2;

template <class A, class B>
concept CanAdd = requires(A a, B b) {
    a + b;
};

static_assert(!CanAdd<Point2, Point2>,
    "two points cannot be added as though they were displacements");
static_assert(CanAdd<Point2, Vector2>,
    "point plus a displacement must be supported");

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

void near(double actual, double expected, double tol,
          std::string_view message) {
    if (!(std::isfinite(actual) && std::abs(actual - expected) <= tol)) {
        throw std::runtime_error(std::string(message));
    }
}

void near_point(Point2 actual, Point2 expected, double tol,
                std::string_view message) {
    near(actual.x, expected.x, tol, message);
    near(actual.y, expected.y, tol, message);
}

template <class Exception, class Operation>
void requires_throw(Operation operation, std::string_view message) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(std::string(message));
}

void point_vs_displacement() {
    const Point2 a{1.0, 1.0};
    const Point2 b{5.0, 3.0};
    const Vector2 displacement = b - a;
    require(displacement == Vector2{4.0, 2.0},
            "subtracting two points gives displacement");
    require(a + displacement == b, "point plus displacement gives point");
    require(book03::midpoint(a, b) == Point2{3.0, 2.0},
            "explicit affine midpoint is well defined");
}

void quarter_turn_and_vectors() {
    const Frame2 rotated = Frame2::rotation(
        {2.0, 1.0}, book03::degrees_to_radians(90.0));
    const Point2 world = rotated.local_to_world({3.0, 1.0});
    near_point(world, {1.0, 4.0}, 1e-12,
               "quarter-turn local point must map to world (1,4)");
    near_point(rotated.world_to_local(world), {3.0, 1.0}, 1e-12,
               "inverse rotation must recover local address");

    const Vector2 movement = rotated.transform_vector({2.0, 0.0});
    near(movement.x, 0.0, 1e-12, "direction must turn around x");
    near(movement.y, 2.0, 1e-12, "direction must turn around y");

    // Moving the origin changes point addresses but not the transformed
    // displacement when the basis axes are unchanged.
    Frame2 shifted = rotated;
    shifted.origin = {200.0, -50.0};
    const auto unchanged = shifted.transform_vector({2.0, 0.0});
    near(unchanged.x, movement.x, 1e-12, "translation may not move vector");
    near(unchanged.y, movement.y, 1e-12, "translation may not move vector");
}

void nonorthogonal_basis_and_singular_cases() {
    // Basis columns (2,0) and (1,1), origin (1,2), local point (3,-1).
    // World = (1,2)+(6,0)+(-1,-1) = (6,1).
    const Frame2 shear{{1.0, 2.0}, {2.0, 0.0}, {1.0, 1.0}};
    shear.validate();
    near(shear.determinant(), 2.0, 0.0, "known determinant");
    near_point(shear.local_to_world({3.0, -1.0}), {6.0, 1.0},
               1e-12, "sheared frame forward");
    near_point(shear.world_to_local({6.0, 1.0}), {3.0, -1.0},
               1e-12, "sheared frame inverse not transpose");

    const Frame2 collapsed{{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}};
    requires_throw<std::invalid_argument>(
        [&] { collapsed.validate(); },
        "collinear axes are singular");
    requires_throw<std::invalid_argument>(
        [&] { (void)collapsed.world_to_local({1.0, 0.0}); },
        "no unique inverse for collinear axes");
    const Frame2 nearly_collapsed{
        {0.0, 0.0}, {1.0, 0.0}, {1.0, 1e-14}};
    requires_throw<std::invalid_argument>(
        [&] { nearly_collapsed.validate(); },
        "numerically unstable basis should be rejected");

    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    requires_throw<std::invalid_argument>(
        [&] { Frame2{{inf, 0.0}, {1.0, 0.0}, {0.0, 1.0}}.validate(); },
        "infinite origin rejected");
    requires_throw<std::invalid_argument>(
        [&] { (void)Frame2::rotation({0.0, 0.0}, nan); },
        "NaN angle rejected");
}

void frame_composition() {
    // Parent: origin (10,0), 90 CCW. Child: parent-local origin (2,0),
    // another 90 CCW. Child-local (1,1) should map to world (9,1).
    const Frame2 parent = Frame2::rotation(
        {10.0, 0.0}, book03::degrees_to_radians(90));
    const Frame2 child = Frame2::rotation(
        {2.0, 0.0}, book03::degrees_to_radians(90));
    const Frame2 combined = Frame2::compose(parent, child);
    const Point2 local{1.0, 1.0};
    const Point2 nested =
        parent.local_to_world(child.local_to_world(local));
    near_point(nested, {9.0, 1.0}, 1e-12,
               "independent sequential composition");
    near_point(combined.local_to_world(local), nested, 1e-12,
               "combined frame must equal sequential mapping");
    near_point(combined.world_to_local(nested), local, 1e-12,
               "composed inverse must recover local point");

    const auto rotation = Frame2::rotation(
        {0.0, 0.0}, book03::degrees_to_radians(90));
    const Frame2 translate{{2.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}};
    near_point(Frame2::compose(translate, rotation).local_to_world({1, 0}),
               {2, 1}, 1e-12, "translate after rotation");
    near_point(Frame2::compose(rotation, translate).local_to_world({1, 0}),
               {0, 3}, 1e-12, "rotate after translation");
}

void world_up_is_image_down() {
    const ImageMap2 map{{320.5, 240.5}, 60.0, 60.0, 640, 480};
    const ImagePosition2 image = map.world_to_image({1.0, 2.0});
    near(image.u, 380.5, 0.0, "correct rightward image position");
    near(image.v, 120.5, 0.0, "world up must reduce image row coordinate");

    const auto chosen = map.pixel_at(image);
    require(chosen.has_value(), "projected point lies inside image");
    require(*chosen == ImagePixel2{380, 120},
            "continuous image position maps to correct pixel index");
    const auto center = map.center(*chosen);
    near(center.u, 380.5, 0.0, "center u");
    near(center.v, 120.5, 0.0, "center v");

    const Point2 recovered = map.image_to_world(image);
    near_point(recovered, {1.0, 2.0}, 1e-12,
               "world-image continuous round trip");

    require(!map.pixel_at({-0.01, 10.0}), "reject negative u");
    require(!map.pixel_at({10.0, -0.01}), "reject negative v");
    require(!map.pixel_at({640.0, 20.0}), "right edge is outside");
    require(!map.pixel_at({20.0, 480.0}), "bottom edge is outside");
    require(map.pixel_at({0.0, 0.0}) == ImagePixel2{0, 0},
            "top-left half-open cell owns origin");
    require(map.pixel_at({639.999, 479.999}) == ImagePixel2{639, 479},
            "last cell owned before exclusive boundary");
    require(!map.pixel_at({
        std::numeric_limits<double>::quiet_NaN(), 1.0}),
        "nonfinite point has no pixel address");
}

void invalid_maps_and_overflow() {
    ImageMap2 bad;
    bad.pixels_per_metre_y = 0.0;
    requires_throw<std::invalid_argument>(
        [&] { bad.validate(); }, "zero pixel-per-metre scale");
    bad = ImageMap2{};
    bad.width = 0;
    requires_throw<std::invalid_argument>(
        [&] { (void)bad.world_to_image({1.0, 2.0}); },
        "zero image width rejected");
    bad = ImageMap2{};
    bad.world_origin.u = std::numeric_limits<double>::infinity();
    requires_throw<std::invalid_argument>(
        [&] { bad.validate(); }, "nonfinite image origin rejected");

    const ImageMap2 map{{100.0, 100.0}, 1e300, 1e300, 640, 480};
    requires_throw<std::overflow_error>(
        [&] { (void)map.world_to_image({1e300, 1.0}); },
        "finite input multiplication can still overflow");
    requires_throw<std::invalid_argument>(
        [&] { (void)map.image_to_world({
            std::numeric_limits<double>::infinity(), 1.0}); },
        "infinite image input rejected");
}

} // namespace

int main() {
    try {
        point_vs_displacement();
        quarter_turn_and_vectors();
        nonorthogonal_basis_and_singular_cases();
        frame_composition();
        world_up_is_image_down();
        invalid_maps_and_overflow();
        std::cout << "Book03/P02: all frame, inverse and image-mapping tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Book03/P02 FAILED: " << error.what() << '\n';
        return 1;
    }
}
