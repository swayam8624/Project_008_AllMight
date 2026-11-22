#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <optional>

// World-space positions and basis axes are measured in metres.
// Local coefficients are dimensionless multiples of the chosen axes.
struct Vec2 {
    double x{};
    double y{};
};

struct Frame2 {
    Vec2 origin; // World-space location of local (0,0).
    Vec2 axis0;  // World displacement made by one local-X step.
    Vec2 axis1;  // World displacement made by one local-Y step.
};

constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x+b.x, a.y+b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x-b.x, a.y-b.y}; }
constexpr Vec2 operator/(Vec2 a, double s) { return {a.x/s, a.y/s}; }
constexpr double cross(Vec2 a, Vec2 b) { return a.x*b.y - a.y*b.x; }
bool finite(Vec2 p) { return std::isfinite(p.x) && std::isfinite(p.y); }

// Active construction: build a world point from an origin and two basis axes.
Vec2 to_world(const Frame2& f, Vec2 local) {
    return {
        std::fma(local.y, f.axis1.x, std::fma(local.x, f.axis0.x, f.origin.x)),
        std::fma(local.y, f.axis1.y, std::fma(local.x, f.axis0.y, f.origin.y))
    };
}

// Passive coordinate recovery. No inverse matrix is constructed in memory;
// the two signed-area ratios are Cramer's rule applied to a 2x2 system.
std::optional<Vec2> to_local(const Frame2& f, Vec2 world) {
    if (!finite(world) || !finite(f.origin) ||
        !finite(f.axis0) || !finite(f.axis1)) return std::nullopt;

    const double n0 = std::hypot(f.axis0.x, f.axis0.y);
    const double n1 = std::hypot(f.axis1.x, f.axis1.y);
    if (!(std::isfinite(n0) && std::isfinite(n1)) || n0 == 0 || n1 == 0)
        return std::nullopt;

    // Sine of the signed angle between unit basis axes; independent of scale.
    const Vec2 a = f.axis0 / n0;
    const Vec2 b = f.axis1 / n1;
    const double signed_sin = cross(a, b);
    constexpr double min_sin = 64 * std::numeric_limits<double>::epsilon();
    if (!std::isfinite(signed_sin) || std::abs(signed_sin) <= min_sin)
        return std::nullopt; // Exact or numerically ill-conditioned basis.

    const Vec2 displacement = world - f.origin;
    if (!finite(displacement)) return std::nullopt;

    // Coordinate i is the oriented area ratio. Scale BEFORE multiplying
    // to avoid an avoidable overflow in cross(axis0, axis1).
    const Vec2 local{
        cross(displacement / n0, b) / signed_sin,
        cross(a, displacement / n1) / signed_sin
    };
    if (!finite(local)) return std::nullopt;
    return local;
}

bool near(double a, double b, double tol = 1e-12) {
    return std::abs(a-b) <= tol * std::max({1.0, std::abs(a), std::abs(b)});
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    const Frame2 panel{{5,-1}, {2,0}, {1,3}};
    const Vec2 target{2,-1};
    const Vec2 measured = to_world(panel, target);
    require(near(measured.x, 8) && near(measured.y, -4), "World coordinate mapping failed");

    const auto recovered = to_local(panel, measured);
    require(recovered && near(recovered->x, 2) && near(recovered->y, -1), "Coordinate recovery failed");

    // The basis Gram metric measures actual world length in local coordinates.
    // G = [[4,2],[2,10]]; q = [2,-1] => q^T G q = 18 m^2.
    const double g00 = 4, g01 = 2, g11 = 10;
    const double squared_length =
        g00*target.x*target.x + 2*g01*target.x*target.y + g11*target.y*target.y;
    require(near(squared_length, 18), "Gram metric length mismatch");

    const Frame2 nearly_parallel{{0,0}, {1,0}, {1,1e-15}};
    require(!to_local(nearly_parallel, {1,2}), "Ill-conditioned basis not rejected");
    require(!to_local(panel, {std::numeric_limits<double>::infinity(),0}), "Nonfinite input not rejected");
}
