// C++20 two-point unilateral contact witness: 2x2 block complementarity, not a complete engine solver.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/two_point_contact_block_lab.cpp -o /tmp/two_point && /tmp/two_point
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

struct Vec2 { double x{}, y{}; };
constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
constexpr Vec2 operator*(Vec2 a, double k) { return {a.x * k, a.y * k}; }
constexpr double dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
constexpr double cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
constexpr Vec2 spin(double w, Vec2 r) { return {-w * r.y, w * r.x}; }

void require(bool ok, const char* what) {
    if (!ok) throw std::runtime_error(what);
}
void near(double got, double want, double tolerance = 1e-9) {
    require(std::isfinite(got) && std::abs(got - want) <= tolerance,
            "numerical check failed");
}
void finite(Vec2 a) { require(std::isfinite(a.x) && std::isfinite(a.y), "nonfinite point"); }

struct Body {
    Vec2 center{}, velocity{};
    double omega{}, inverseMass{}, inverseInertia{};
};
void validate(const Body& b) {
    finite(b.center); finite(b.velocity);
    require(std::isfinite(b.omega) && std::isfinite(b.inverseMass) &&
            std::isfinite(b.inverseInertia) && b.inverseMass >= 0.0 &&
            b.inverseInertia >= 0.0, "invalid body");
}
struct ContactPair { std::array<Vec2, 2> worldPoints{}; Vec2 normal{0, 1}; };
struct BlockResult {
    std::array<double, 2> impulses{}, postSpeeds{};
    int activeConstraints{};
};

Vec2 pointVelocity(const Body& b, Vec2 r) {
    return b.velocity + spin(b.omega, r);
}
void apply(Body& a, Body& b, Vec2 ra, Vec2 rb, Vec2 normal, double impulse) {
    const Vec2 p = normal * impulse;
    a.velocity = a.velocity - p * a.inverseMass;
    b.velocity = b.velocity + p * b.inverseMass;
    a.omega -= a.inverseInertia * cross(ra, p);
    b.omega += b.inverseInertia * cross(rb, p);
}

// A contact is unilateral: its impulse can push, but never pull.
// The two-point matrix captures how an impulse at either point affects BOTH gaps.
BlockResult solveBlock(Body& a, Body& b, ContactPair pair) {
    validate(a); validate(b); finite(pair.normal);
    const double normalLength = std::hypot(pair.normal.x, pair.normal.y);
    require(std::isfinite(normalLength) && normalLength > 0, "invalid normal");
    const Vec2 n = pair.normal * (1.0 / normalLength);
    std::array<Vec2, 2> ra{}, rb{};
    std::array<double, 2> leverA{}, leverB{}, before{};
    for (int i = 0; i < 2; ++i) {
        finite(pair.worldPoints[i]);
        ra[i] = pair.worldPoints[i] - a.center;
        rb[i] = pair.worldPoints[i] - b.center;
        leverA[i] = cross(ra[i], n);
        leverB[i] = cross(rb[i], n);
        before[i] = dot(pointVelocity(b, rb[i]) - pointVelocity(a, ra[i]), n);
        require(std::isfinite(before[i]), "nonfinite relative velocity");
    }
    double K[2][2]{};
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            K[i][j] = a.inverseMass + b.inverseMass +
                      a.inverseInertia * leverA[i] * leverA[j] +
                      b.inverseInertia * leverB[i] * leverB[j];
    for (auto& row : K)
        for (double value : row)
            require(std::isfinite(value), "effective mass overflow");

    // Solve w = before + K*lambda >= 0, lambda >= 0, lambda_i*w_i = 0.
    // Enumerate both-active, left-only, right-only and none, accepting only
    // complementarity-feasible cases. Zero determinant means no unique
    // both-active solution; a one-active solution may still exist.
    constexpr double eps = 1e-10;
    const double determinant = K[0][0] * K[1][1] - K[0][1] * K[1][0];
    const double matrixScale = std::max({1.0, std::abs(K[0][0] * K[1][1]),
                                          std::abs(K[0][1] * K[1][0])});
    auto feasible = [&](std::array<double, 2> lambda) {
        std::array<double, 2> gap{
            before[0] + K[0][0] * lambda[0] + K[0][1] * lambda[1],
            before[1] + K[1][0] * lambda[0] + K[1][1] * lambda[1]
        };
        const double scale = std::max({1.0, std::abs(before[0]), std::abs(before[1])});
        const double tolerance = eps * scale;
        for (int i = 0; i < 2; ++i) {
            if (!std::isfinite(lambda[i]) || !std::isfinite(gap[i]) ||
                lambda[i] < -tolerance || gap[i] < -tolerance ||
                (lambda[i] > tolerance && std::abs(gap[i]) > tolerance))
                return false;
        }
        return true;
    };
    std::array<double, 2> answer{};
    bool found = false;
    if (std::isfinite(determinant) && determinant > eps * matrixScale) {
        const std::array<double, 2> both{
            (-K[1][1] * before[0] + K[0][1] * before[1]) / determinant,
            ( K[1][0] * before[0] - K[0][0] * before[1]) / determinant
        };
        if (feasible(both)) { answer = both; found = true; }
    }
    if (!found && K[0][0] > eps) {
        const std::array<double, 2> left{-before[0] / K[0][0], 0};
        if (feasible(left)) { answer = left; found = true; }
    }
    if (!found && K[1][1] > eps) {
        const std::array<double, 2> right{0, -before[1] / K[1][1]};
        if (feasible(right)) { answer = right; found = true; }
    }
    if (!found && feasible({0, 0})) found = true;
    require(found, "no valid contact impulse: degenerate or inconsistent pair");

    for (int i = 0; i < 2; ++i) {
        // Tiny negative roundoff is projected to zero, not treated as attraction.
        answer[i] = std::max(0.0, answer[i]);
        apply(a, b, ra[i], rb[i], n, answer[i]);
    }
    validate(a); validate(b);
    BlockResult result;
    result.impulses = answer;
    for (int i = 0; i < 2; ++i) {
        result.postSpeeds[i] =
            dot(pointVelocity(b, rb[i]) - pointVelocity(a, ra[i]), n);
        near(result.postSpeeds[i],
             before[i] + K[i][0] * answer[0] + K[i][1] * answer[1], 1e-8);
        require(result.postSpeeds[i] >= -1e-8, "still closing after block solve");
        if (answer[i] > eps) ++result.activeConstraints;
    }
    return result;
}
int main() {
    const Body ground{{0, 0}, {0, 0}, 0, 0, 0};
    { // Symmetric crate: K = diag(2,2); a -2m/s fall needs 1N*s at each point.
        Body crate{{0, 1}, {0, -2}, 0, 1, 1};
        Body fixed = ground;
        const auto result = solveBlock(fixed, crate,
                                      ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 1); near(result.impulses[1], 1);
        near(crate.velocity.y, 0); near(crate.omega, 0);
        require(result.activeConstraints == 2, "both contacts should act");
    }
    { // Asymmetric footprint: K=[[2,-1],[-1,5]], impulses 4/3 and 2/3.
        Body fixed = ground, crate{{0, 1}, {0, -2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{2, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 4.0 / 3.0);
        near(result.impulses[1], 2.0 / 3.0);
        near(crate.velocity.y, 0); near(crate.omega, 0);
    }
    { // Single active point: positive separating speed at the second point.
        Body fixed = ground, crate{{0, 1}, {0, 0}, 1, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 0.5); near(result.impulses[1], 0.0);
    }
    { // Already separating: neither contact is allowed to pull the body down.
        Body fixed = ground, crate{{0, 1}, {0, 2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 0); near(result.impulses[1], 0);
    }
    { // Duplicate contact positions make K rank deficient; one point suffices.
        Body fixed = ground, crate{{0, 1}, {0, -2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{-1, 0}}, Vec2{0, 1}});
        near(result.impulses[0] + result.impulses[1], 1);
        near(result.postSpeeds[0], 0); near(result.postSpeeds[1], 0);
    }
    { // Two immovable bodies cannot repair closing speed via impulse.
        Body fixed = ground, immovable{{0, 1}, {0, -1}, 0, 0, 0};
        bool rejected = false;
        try { (void)solveBlock(fixed, immovable, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "immovable closing geometry should be rejected");
    }
    { // Hostile inputs must fail before polluting another island.
        Body fixed = ground, crate{{0, 1}, {0, -1}, 0, 1, 1};
        bool rejected = false;
        try { (void)solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 0}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "zero normal must be rejected");
    }
    std::cout << "two-point contact block: 7 suites PASS\n";
}