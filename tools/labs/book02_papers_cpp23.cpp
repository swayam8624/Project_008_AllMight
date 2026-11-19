// Book Two — C++23 mathematical contracts, independently written reference lab.
//
// Source lineage: Shewchuk's filtered-predicate idea, P1 triangle FEM
// stiffness, and the numerical conditioning discussion in Book Two.
// This is NOT a copy/port of predicates.c or a general stable solver.
// An orientation near the floating-point error bound returns UNDECIDED,
// so callers cannot silently mistake an unverified sign for an exact result.
//
// Build: c++ -std=c++23 -O2 -Wall -Wextra -Werror -pedantic \
//              tools/labs/book02_papers_cpp23.cpp -o book02_cpp23
// Execute: ./book02_cpp23

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>

namespace book02 {
struct Vec2 final { double x{}, y{}; };
using Matrix2 = std::array<std::array<double, 2>, 2>;
using Matrix3 = std::array<std::array<double, 3>, 3>;

void require(bool truth, const char* message) {
    if (!truth) throw std::runtime_error(message);
}

bool finite(Vec2 p) noexcept {
    return std::isfinite(p.x) && std::isfinite(p.y);
}

double cross(Vec2 a, Vec2 b) noexcept {
    return a.x * b.y - a.y * b.x;
}

Vec2 operator-(Vec2 a, Vec2 b) noexcept {
    return {a.x - b.x, a.y - b.y};
}

enum class SignResult { Negative = -1, Uncertain = 0, Positive = 1 };

// Conservative first-stage floating-point sign filter; this function
// DOES NOT pretend to prove exact zero or decide uncertified cases.
// For an exact-sign fallback on binary64 coordinates, use the Python
// Fraction.from_float educational oracle. Production code should prefer
// a fully verified adaptive expansion-based predicate.
SignResult orientation_filtered(Vec2 a, Vec2 b, Vec2 c) {
    if (!finite(a) || !finite(b) || !finite(c))
        throw std::invalid_argument("orientation requires finite coordinates");
    const Vec2 ac = a - c;
    const Vec2 bc = b - c;
    const double left = ac.x * bc.y;
    const double right = ac.y * bc.x;
    const double d = left - right;
    const double s = std::abs(left) + std::abs(right);
    const double bound = 64.0 * std::numeric_limits<double>::epsilon() * s;
    if (!std::isfinite(d) || !std::isfinite(s) ||
        !std::isfinite(bound) || std::abs(d) <= bound) {
        return SignResult::Uncertain;
    }
    return d > 0.0 ? SignResult::Positive : SignResult::Negative;
}

struct TriangleP1 final {
    double area{};
    Matrix3 stiffness{};
};

// Each P1 shape-function gradient is constant over a triangle.
// The positive-semidefinite stiffness K_ij = area * grad(N_i) dot grad(N_j)
// is equivalent to a cotangent form for a valid planar triangle, with
// explicit convention K = -L for libraries that export negative L.
TriangleP1 triangle_stiffness(const std::array<Vec2, 3>& p) {
    for (Vec2 point : p)
        if (!finite(point))
            throw std::invalid_argument("nonfinite triangle vertex");
    const double twice = cross(p[1] - p[0], p[2] - p[0]);
    if (!std::isfinite(twice) || std::abs(twice) < 1e-14)
        throw std::invalid_argument("degenerate or unrepresentable area");
    const double area = std::abs(twice) / 2.0;
    const std::array<Vec2, 3> grad {{
        {(p[1].y - p[2].y) / twice, (p[2].x - p[1].x) / twice},
        {(p[2].y - p[0].y) / twice, (p[0].x - p[2].x) / twice},
        {(p[0].y - p[1].y) / twice, (p[1].x - p[0].x) / twice},
    }};
    TriangleP1 result;
    result.area = area;
    for (std::size_t i = 0; i < 3; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            result.stiffness[i][j] =
                area * (grad[i].x * grad[j].x + grad[i].y * grad[j].y);
        }
    }
    return result;
}

double energy(const Matrix3& K, const std::array<double, 3>& u) noexcept {
    double value = 0.0;
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            value += u[i] * K[i][j] * u[j];
    return 0.5 * value;
}

// Pedagogical scaled 2-by-2 Cramer's rule. Condition screening avoids
// treating numerically indistinguishable columns as reliable inversion.
// It is NOT a stable general-purpose least-squares implementation.
std::optional<Vec2> small_system_solve(Matrix2 A, Vec2 rhs) {
    if (!finite(rhs)) return std::nullopt;
    double scale = 0.0;
    for (const auto& row : A)
        for (double v : row) {
            if (!std::isfinite(v)) return std::nullopt;
            scale = std::max(scale, std::abs(v));
        }
    if (scale == 0.0) return std::nullopt;
    for (auto& row : A)
        for (double& v : row) v /= scale;
    rhs.x /= scale;
    rhs.y /= scale;
    const double det = A[0][0]*A[1][1] - A[0][1]*A[1][0];
    const double threshold = 64.0 * std::numeric_limits<double>::epsilon();
    if (!std::isfinite(det) || std::abs(det) <= threshold)
        return std::nullopt;
    const Vec2 result {
        ( A[1][1]*rhs.x - A[0][1]*rhs.y)/det,
        (-A[1][0]*rhs.x + A[0][0]*rhs.y)/det
    };
    return finite(result) ? std::optional<Vec2>{result} : std::nullopt;
}

bool near(double a, double b, double tol=1e-12) noexcept {
    return std::abs(a-b) <= tol * std::max({1.,std::abs(a),std::abs(b)});
}
} // namespace book02

int main() {
    using namespace book02;
    const Vec2 a{0.,0.}, b{3.,0.}, c{0.,2.};
    require(orientation_filtered(a,b,c) == SignResult::Positive,
            "basic positive orientation");
    require(orientation_filtered(a,c,b) == SignResult::Negative,
            "orientation reversal");
    require(orientation_filtered({0,0},{1,1},{2,2}) == SignResult::Uncertain,
            "do not certify exact collinearity with an approximate determinant");
    const double huge = 1e308;
    require(orientation_filtered({0.,0.},{huge,huge},
            {.9*huge,std::nextafter(.9*huge,0.)}) == SignResult::Uncertain,
            "must refuse a sign when products overflow");
    bool rejected = false;
    try {
        (void)orientation_filtered({0.,std::numeric_limits<double>::quiet_NaN()},
                                   b,c);
    } catch (const std::invalid_argument&) { rejected=true; }
    require(rejected,"nonfinite point not rejected");

    // Compare every certified orientation with an exact 64-bit integer
    // calculation. This verifies the finite bounded domain below and is
    // NOT a proof that arbitrary binary64 inputs always certify safely.
    std::mt19937_64 rng(20261119);
    std::uniform_int_distribution<long long> draw(-10000,10000);
    std::size_t decided=0,uncertain=0;
    for (int sample=0;sample<1000;++sample) {
        const long long ax=draw(rng), ay=draw(rng);
        const long long bx=draw(rng), by=draw(rng);
        const long long cx=draw(rng), cy=draw(rng);
        const long long exact =
            (ax-cx)*(by-cy) - (ay-cy)*(bx-cx);
        const int expected = (exact>0)-(exact<0);
        const SignResult got = orientation_filtered(
            {static_cast<double>(ax),static_cast<double>(ay)},
            {static_cast<double>(bx),static_cast<double>(by)},
            {static_cast<double>(cx),static_cast<double>(cy)});
        if (got == SignResult::Uncertain) ++uncertain;
        else {
            ++decided;
            require(static_cast<int>(got) == expected,
                    "filtered predicate contradicted exact integer sign");
        }
    }
    require(decided > 900,"too few ordinary cases certified");

    const auto tri=triangle_stiffness({Vec2{0.,0.},Vec2{2.,0.},
                                       Vec2{.5,.5}});
    require(near(tri.area,.5),"triangle area");
    const Matrix3 expected {{
        {{1.25,.25,-1.5}},
        {{.25,.25,-.5}},
        {{-1.5,-.5,2.}}
    }};
    for (std::size_t i=0;i<3;++i) {
        double sum=0;
        for (std::size_t j=0;j<3;++j) {
            require(near(tri.stiffness[i][j],expected[i][j]),
                    "finite-element entry");
            sum+=tri.stiffness[i][j];
        }
        require(near(sum,0.),"constant energy nullspace");
    }
    const std::array<double,3> old_field{{0.,1.,0.}};
    std::array<double,3> new_field{};
    const double vertex_mass=tri.area/3;
    const double dt=.01;
    for (std::size_t i=0;i<3;++i) {
        double Ku=0;
        for(std::size_t j=0;j<3;++j) Ku+=tri.stiffness[i][j]*old_field[j];
        new_field[i]=old_field[i]-dt*Ku/vertex_mass;
    }
    require(near(new_field[0],-.015)&&near(new_field[1],.985)&&
            near(new_field[2],.03),"explicit step trace");
    require(new_field[0]<0.,"energy can fall even if positivity is lost");
    require(energy(tri.stiffness,new_field)<energy(tri.stiffness,old_field),
            "selected energy decrease");

    const auto gradient=small_system_solve(
        {{{4.,2.},{2.,10.}}}, {2.,7.});
    require(gradient && near(gradient->x,1./6) &&
            near(gradient->y,2./3), "metric gradient recovery");
    require(!small_system_solve(
        {{{1.,1.},{1.,1.+1e-15}}}, {1.,1.}),
        "nearly singular matrix must be rejected");

    const double stable_norm=std::hypot(1e200,1e200);
    require(std::isfinite(stable_norm) && near(stable_norm/1e200,std::sqrt(2.)),
            "stable norm must avoid squared sum overflow");
    std::cout<<"book02_cpp23: PASS; certified orientations="
             <<decided<<"; uncertain="<<uncertain
             <<"; tested scalar, coordinate and mesh invariants\n";
}
