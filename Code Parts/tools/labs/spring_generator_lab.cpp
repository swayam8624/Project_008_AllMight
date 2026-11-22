// Signed spring and bungee laws; source-sign and integration-order witnesses.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

struct Vec3 {
    double x{}, y{}, z{};
};

static double length(const Vec3& v) {
    const double scale = std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
    if (scale == 0.0) return 0.0;
    const double sx = v.x / scale, sy = v.y / scale, sz = v.z / scale;
    return scale * std::sqrt(sx * sx + sy * sy + sz * sz);
}
static bool finite(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
static bool near(double a, double b, double eps = 1e-12) {
    return std::abs(a - b) <= eps;
}
static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
static void validate(Vec3 position, Vec3 endpoint, double stiffness,
                     double restLength) {
    if (!finite(position) || !finite(endpoint) || !std::isfinite(stiffness) ||
        !std::isfinite(restLength) || stiffness < 0.0 || restLength < 0.0)
        throw std::invalid_argument("invalid spring input");
}
static Vec3 springForceFromLength(Vec3 r, double L, double stiffness,
                                  double restLength) {
    if (!std::isfinite(L)) throw std::overflow_error("spring length overflow");
    if (L == 0.0) return {}; // Witness policy: no direction, so choose zero force.
    // Normalize scaled components so a rounded subnormal length does not
    // materially change the direction or require an overflowing reciprocal.
    const double scale = std::max({std::abs(r.x), std::abs(r.y), std::abs(r.z)});
    const double sx = r.x / scale, sy = r.y / scale, sz = r.z / scale;
    const double scaledLength = std::sqrt(sx * sx + sy * sy + sz * sz);
    const Vec3 n{sx / scaledLength, sy / scaledLength, sz / scaledLength};
    const double magnitude = -stiffness * (L - restLength);
    if (!std::isfinite(magnitude))
        throw std::overflow_error("spring force magnitude overflow");
    const Vec3 force{magnitude * n.x, magnitude * n.y, magnitude * n.z};
    if (!finite(force)) throw std::overflow_error("spring force component overflow");
    return force;
}
// Returns force ON the particle. n points outward from endpoint to particle.
// Stiffness is N/m; positions and rest length are m.
static Vec3 springForce(Vec3 position, Vec3 endpoint, double stiffness,
                        double restLength) {
    validate(position, endpoint, stiffness, restLength);
    const Vec3 r{position.x - endpoint.x, position.y - endpoint.y,
                 position.z - endpoint.z};
    if (!finite(r)) throw std::overflow_error("spring displacement overflow");
    const double L = length(r);
    return springForceFromLength(r, L, stiffness, restLength);
}
// A bungee exerts no force while slack, including exactly at rest length.
static Vec3 bungeeForce(Vec3 position, Vec3 endpoint, double stiffness,
                        double restLength) {
    validate(position, endpoint, stiffness, restLength);
    const Vec3 r{position.x - endpoint.x, position.y - endpoint.y,
                 position.z - endpoint.z};
    if (!finite(r)) throw std::overflow_error("bungee displacement overflow");
    const double L = length(r);
    if (!std::isfinite(L)) throw std::overflow_error("bungee length overflow");
    if (L <= restLength) return {};
    return springForceFromLength(r, L, stiffness, restLength);
}
// Scalar reductions of the two printed sign patterns, solely for comparison.
static double printedAnchoredForce1D(double x, double stiffness, double restLength) {
    const double L = std::abs(x);
    if (L == 0.0) return 0.0;
    const double magnitude = std::abs(L - restLength) * stiffness;
    return -(x / L) * magnitude;
}
static double printedBungeeForce1D(double x, double stiffness, double restLength) {
    const double L = std::abs(x);
    if (L <= restLength) return 0.0;
    const double magnitude = stiffness * (restLength - L);
    return -(x / L) * magnitude;
}
struct ParticleState { double x, v; };
// Position-first forward Euler ordering from PDF pp. 93-94 / printed pp. 70-71.
static ParticleState integrateForwardEuler(ParticleState s, double force,
                                           double mass, double h) {
    if (!std::isfinite(s.x) || !std::isfinite(s.v) || !std::isfinite(force) ||
        !std::isfinite(mass) || mass <= 0.0 || !std::isfinite(h) || h <= 0.0)
        throw std::invalid_argument("state, force, mass and duration must be finite; mass and duration positive");
    const double nextX = s.x + s.v * h;
    const double acceleration = force / mass;
    const double nextV = s.v + acceleration * h;
    if (!std::isfinite(nextX) || !std::isfinite(acceleration) ||
        !std::isfinite(nextV))
        throw std::overflow_error("forward-Euler state overflow");
    return {nextX, nextV};
}
template<class Exception, class F>
static void expectThrow(F&& f, const char* message) {
    bool threw = false;
    try { f(); }
    catch (const Exception&) { threw = true; }
    require(threw, message);
}

int main() {
    constexpr double k = 8.0;       // N/m
    constexpr double rest = 1.0;    // m
    const Vec3 anchor{};

    const Vec3 stretched = springForce({1.5, 0, 0}, anchor, k, rest);
    const Vec3 compressed = springForce({0.5, 0, 0}, anchor, k, rest);
    const Vec3 negativeStretched = springForce({-1.5, 0, 0}, anchor, k, rest);
    const Vec3 negativeCompressed = springForce({-0.5, 0, 0}, anchor, k, rest);
    const Vec3 atRest = springForce({1.0, 0, 0}, anchor, k, rest);
    const Vec3 coincident = springForce(anchor, anchor, k, rest);
    const Vec3 tautBungee = bungeeForce({1.5, 0, 0}, anchor, k, rest);
    const Vec3 slackBungee = bungeeForce({0.5, 0, 0}, anchor, k, rest);
    const Vec3 boundaryBungee = bungeeForce({1.0, 0, 0}, anchor, k, rest);

    require(near(stretched.x, -4.0), "positive stretched spring direction");
    require(near(compressed.x, 4.0), "positive compressed spring direction");
    require(near(negativeStretched.x, 4.0), "negative stretched spring direction");
    require(near(negativeCompressed.x, -4.0), "negative compressed spring direction");
    require(near(atRest.x, 0.0), "spring at rest");
    require(near(coincident.x, 0.0), "coincident-point zero-force policy");
    require(near(tautBungee.x, -4.0), "taut bungee direction");
    require(near(slackBungee.x, 0.0), "slack bungee");
    require(near(boundaryBungee.x, 0.0), "bungee at rest-length boundary");
    require(near(printedAnchoredForce1D(0.5, k, rest), -4.0),
            "printed anchored-sign reduction");
    require(near(printedBungeeForce1D(1.5, k, rest), 4.0),
            "printed bungee-sign reduction");

    expectThrow<std::invalid_argument>(
        [&] { (void)springForce({0,0,0}, anchor, -1.0, rest); },
        "negative stiffness must be rejected");
    expectThrow<std::invalid_argument>(
        [&] { (void)springForce({std::numeric_limits<double>::quiet_NaN(),0,0}, anchor, k, rest); },
        "nonfinite position must be rejected");
    expectThrow<std::overflow_error>(
        [&] { (void)springForce({2,0,0}, anchor, std::numeric_limits<double>::max(), 0.0); },
        "nonfinite force magnitude must be rejected");
    const double tiny = std::numeric_limits<double>::denorm_min();
    const Vec3 tinySeparation = springForce({tiny,0,0}, anchor, 1.0, 0.0);
    require(tinySeparation.x == -tiny && tinySeparation.y == 0.0 &&
            tinySeparation.z == 0.0,
            "subnormal separation normalization");

    const Vec3 tinyDiagonal = springForce({tiny, tiny, tiny}, anchor, 1.0, 1.0);
    require(near(tinyDiagonal.x, 1.0 / std::sqrt(3.0)),
            "subnormal diagonal retains its direction");
    require(near(length(tinyDiagonal), 1.0), "subnormal force has unit magnitude");

    std::cout << "force law, anchor=0 m, rest=1 m, k=8 N/m\n"
              << "stretched spring: intended=" << stretched.x
              << " N, printed anchored listing="
              << printedAnchoredForce1D(1.5, k, rest) << " N\n"
              << "compressed spring: intended=" << compressed.x
              << " N, printed anchored listing="
              << printedAnchoredForce1D(0.5, k, rest) << " N\n"
              << "taut bungee: intended=" << tautBungee.x
              << " N, printed listing="
              << printedBungeeForce1D(1.5, k, rest) << " N\n"
              << "slack bungee: intended=" << slackBungee.x << " N\n";

    constexpr double mass = 2.0;       // kg
    constexpr double springK = 8.0;    // N/m
    constexpr double x0 = 0.5;         // m from natural length
    constexpr double v0 = 0.4;         // m/s
    constexpr double h = 0.25;         // s
    const double omega = std::sqrt(springK / mass);
    const double target = x0 * std::cos(omega * h) +
                          (v0 / omega) * std::sin(omega * h);
    const double xForwardEuler =
        integrateForwardEuler({x0, v0}, -4.0, mass, h).x;
    require(near(xForwardEuler, x0 + v0 * h), "position-first Euler update");
    require(!near(xForwardEuler, target, 1e-6),
            "current force cannot set this step's position");
    std::cout << "\nundamped endpoint example: omega=" << omega
              << " 1/s, analytic target=" << target
              << " m, listed forward-Euler next position=" << xForwardEuler
              << " m (current force affects velocity first)\n";
}
