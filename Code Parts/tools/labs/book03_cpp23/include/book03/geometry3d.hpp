#pragma once

// Book 03 / Chapter 06 — Standard C++23 three-dimensional geometry.
//
// World reference convention: right-handed, +X right, +Y up, +Z toward a
// viewer in the introductory drawing. Point3 contains a position in metres;
// Vector3 is a displacement (or an explicitly documented unit direction).
// The types prevent point+point, but not mixing named coordinate frames.
// The latter must be stated at every API boundary, as in Chapter 02.
//
// Correctness limits:
// - Finite inputs/results only. std::hypot is used for stable magnitudes.
// - Normalization rejects zero or unrepresentable nonzero directions.
// - Ray3::from normalizes its direction, so ray distance t is in metres.
// - Plane3::through normalizes its normal, so signed distance is in metres.
// - Plane near-parallel and sphere near-tangent classification use explicit
//   *heuristic* relative guards. Neither is a certified 3D predicate.
// - No perspective camera, ray-triangle, GPU acceleration or mesh topology
//   is hidden in this introduction. Later chapters own those components.

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>

namespace book03 {

struct Vector3 {
    double x{}, y{}, z{};
    [[nodiscard]] friend constexpr bool operator==(
        const Vector3&, const Vector3&) = default;
};

struct Point3 {
    double x{}, y{}, z{};
    [[nodiscard]] friend constexpr bool operator==(
        const Point3&, const Point3&) = default;
};

[[nodiscard]] constexpr Vector3 operator-(Point3 a, Point3 b) noexcept {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
[[nodiscard]] constexpr Point3 operator+(Point3 a, Vector3 d) noexcept {
    return {a.x+d.x, a.y+d.y, a.z+d.z};
}
[[nodiscard]] constexpr Point3 operator-(Point3 a, Vector3 d) noexcept {
    return {a.x-d.x, a.y-d.y, a.z-d.z};
}
[[nodiscard]] constexpr Vector3 operator+(Vector3 a, Vector3 b) noexcept {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
[[nodiscard]] constexpr Vector3 operator-(Vector3 a, Vector3 b) noexcept {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
[[nodiscard]] constexpr Vector3 operator*(double s, Vector3 v) noexcept {
    return {s*v.x, s*v.y, s*v.z};
}
[[nodiscard]] constexpr Vector3 operator-(Vector3 v) noexcept {
    return {-v.x,-v.y,-v.z};
}

[[nodiscard]] inline bool finite(Point3 p) noexcept {
    return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
}
[[nodiscard]] inline bool finite(Vector3 v) noexcept {
    return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);
}

[[nodiscard]] inline double dot(Vector3 a,Vector3 b) {
    if(!finite(a)||!finite(b))
        throw std::invalid_argument("dot requires finite vectors");
    const double value=a.x*b.x+a.y*b.y+a.z*b.z;
    if(!std::isfinite(value))
        throw std::overflow_error("dot result not finite");
    return value;
}

[[nodiscard]] inline Vector3 cross(Vector3 a,Vector3 b) {
    if(!finite(a)||!finite(b))
        throw std::invalid_argument("cross requires finite vectors");
    const Vector3 out{
        a.y*b.z-a.z*b.y,
        a.z*b.x-a.x*b.z,
        a.x*b.y-a.y*b.x
    };
    if(!finite(out))
        throw std::overflow_error("cross result not finite");
    return out;
}

[[nodiscard]] inline double magnitude(Vector3 v) {
    if(!finite(v)) throw std::invalid_argument("nonfinite vector");
    const double length=std::hypot(v.x,v.y,v.z);
    if(!std::isfinite(length))
        throw std::overflow_error("vector length overflow");
    return length;
}

[[nodiscard]] inline Vector3 unit(Vector3 v) {
    const double length=magnitude(v);
    if(!(length>0.0))
        throw std::invalid_argument("zero vector has no unit direction");
    const Vector3 result{v.x/length,v.y/length,v.z/length};
    if(!finite(result) || magnitude(result)==0.0)
        throw std::overflow_error("could not normalize vector");
    return result;
}

struct Triangle3 {
    Point3 a{},b{},c{};

    [[nodiscard]] Vector3 doubled_area_vector() const {
        if(!finite(a)||!finite(b)||!finite(c))
            throw std::invalid_argument("nonfinite triangle corner");
        return cross(b-a,c-a);
    }

    [[nodiscard]] double area() const {
        return 0.5*magnitude(doubled_area_vector());
    }

    [[nodiscard]] Vector3 face_normal() const {
        const Vector3 u=b-a, v=c-a;
        const Vector3 n=doubled_area_vector();
        const double nu=magnitude(u), nv=magnitude(v);
        const double nn=magnitude(n);
        // This is a declared conservative classroom policy, NOT a proven
        // orientation predicate. It rejects near-collinear configurations
        // whose normalized cross magnitude is too small to trust here.
        if(!(nu>0.0)||!(nv>0.0)||!(nn>0.0) ||
           nn / nu / nv <=
              64.0 * std::numeric_limits<double>::epsilon()) {
            throw std::invalid_argument(
                "triangle has degenerate or numerically unstable normal");
        }
        return unit(n);
    }
};

struct Plane3 {
    Vector3 normal{0,1,0}; // Unit normal (dimensionless).
    double offset{};       // Metres, d = -normal dot known plane point.

    [[nodiscard]] static Plane3 through(Point3 p,Vector3 normal) {
        if(!finite(p)) throw std::invalid_argument("invalid plane point");
        const Vector3 n=unit(normal);
        const double d=-dot(n,{p.x,p.y,p.z});
        if(!std::isfinite(d))
            throw std::overflow_error("plane offset not finite");
        return {n,d};
    }

    void validate() const {
        if(!finite(normal)||!std::isfinite(offset))
            throw std::invalid_argument("invalid plane");
        if(std::abs(magnitude(normal)-1.0)>
           1e-12) throw std::invalid_argument("plane normal is not unit");
    }

    [[nodiscard]] double signed_distance(Point3 point) const {
        validate();
        if(!finite(point)) throw std::invalid_argument("nonfinite query");
        const double value=dot(normal,{point.x,point.y,point.z})+offset;
        if(!std::isfinite(value))
            throw std::overflow_error("plane distance not finite");
        return value; // Metres because normal is unit length.
    }
};

struct Ray3 {
    Point3 origin{};
    Vector3 direction{0,0,-1}; // Guaranteed unit length by from().

    [[nodiscard]] static Ray3 from(Point3 origin,Vector3 direction) {
        if(!finite(origin))
            throw std::invalid_argument("nonfinite ray origin");
        Ray3 ray{origin,unit(direction)};
        ray.validate();
        return ray;
    }

    void validate() const {
        if(!finite(origin)||!finite(direction) ||
           std::abs(magnitude(direction)-1.0)>1e-12) {
            throw std::invalid_argument(
                "ray must have finite origin and unit direction");
        }
    }

    [[nodiscard]] Point3 at(double metres) const {
        validate();
        if(!std::isfinite(metres)||metres<0)
            throw std::invalid_argument("ray parameter must be >= 0 metres");
        const Point3 result=origin+metres*direction;
        if(!finite(result))
            throw std::overflow_error("ray point outside finite range");
        return result;
    }
};

enum class RayPlaneKind {
    Hit,
    Behind,
    Parallel,
    CoplanarWithinPolicy,
    NearParallelUncertain
};
struct RayPlaneResult {
    RayPlaneKind kind{RayPlaneKind::Parallel};
    std::optional<Point3> point{};
    double metres{}; // Meaningful only for Hit; otherwise unused.
};

[[nodiscard]] inline RayPlaneResult intersect(
    const Ray3& ray,const Plane3& plane) {
    ray.validate();
    plane.validate();
    const double denominator=dot(plane.normal,ray.direction);
    const double signed_height=plane.signed_distance(ray.origin);

    // Unit vectors make this threshold dimensionless and independent
    // of translation/length units in the direction. Not a certified bound.
    constexpr double angular_guard=
        64.0*std::numeric_limits<double>::epsilon();

    if(denominator==0.0) {
        // With axis-aligned exact examples, zero is exact. In general,
        // *coplanarity* here is only a user-facing tolerance policy.
        const double position_scale=std::max({
            1.0,std::abs(ray.origin.x),std::abs(ray.origin.y),
            std::abs(ray.origin.z),std::abs(plane.offset)});
        const double distance_guard=
            64.0*std::numeric_limits<double>::epsilon()*position_scale;
        return {std::abs(signed_height)<=distance_guard ?
            RayPlaneKind::CoplanarWithinPolicy :
            RayPlaneKind::Parallel,{},0.0};
    }
    if(std::abs(denominator)<=angular_guard)
        return {RayPlaneKind::NearParallelUncertain,{},0.0};

    const double t=-signed_height/denominator;
    if(!std::isfinite(t))
        return {RayPlaneKind::NearParallelUncertain,{},0.0};
    if(t<0.0) return {RayPlaneKind::Behind,{},0.0};

    return {RayPlaneKind::Hit,ray.at(t),t};
}

struct Sphere3 {
    Point3 center{};
    double radius{1.0}; // Metres.

    [[nodiscard]] static Sphere3 from(Point3 center,double radius) {
        if(!finite(center)||!std::isfinite(radius)||!(radius>0.0))
            throw std::invalid_argument(
                "sphere needs finite center and positive finite radius");
        return {center,radius};
    }

    void validate() const {
        if(!finite(center)||!std::isfinite(radius)||!(radius>0.0))
            throw std::invalid_argument("invalid sphere");
    }
};

enum class RaySphereKind {
    Hit,
    Miss,
    Behind,
    TangentWithinPolicy,
    NearTangentUncertain
};

struct RaySphereResult {
    RaySphereKind kind{RaySphereKind::Miss};
    std::optional<Point3> point{};
    double metres{}; // First forward hit, when present.
    double other_line_metres{}; // Other root when hit, may be behind.
};

[[nodiscard]] inline RaySphereResult intersect(
    const Ray3& ray,const Sphere3& sphere) {
    ray.validate();
    sphere.validate();
    const Vector3 q=ray.origin-sphere.center;
    const double b=dot(q,ray.direction);
    const double c=dot(q,q)-sphere.radius*sphere.radius;
    if(!std::isfinite(c))
        throw std::overflow_error("sphere quadratic coefficient overflow");

    const double bb=b*b;
    if(!std::isfinite(bb))
        throw std::overflow_error("sphere squared coefficient overflow");
    const double delta=bb-c;
    if(!std::isfinite(delta))
        throw std::overflow_error("sphere discriminant overflow");

    // This proximity test is an explicit classroom heuristic, not
    // a rigorous uncertainty interval for all binary64 inputs.
    const double scale=std::max({1.0,std::abs(bb),std::abs(c)});
    const double guard=64.0*std::numeric_limits<double>::epsilon()*scale;
    if(delta<0.0 && -delta>guard)
        return {RaySphereKind::Miss,{},0.0,0.0};
    if(delta!=0.0 && std::abs(delta)<=guard)
        return {RaySphereKind::NearTangentUncertain,{},0.0,0.0};
    if(delta<0.0) return {RaySphereKind::Miss,{},0.0,0.0};

    const double root=std::sqrt(delta);
    if(delta==0.0) {
        const double t=-b;
        if(t<0.0) return {RaySphereKind::Behind,{},0.0,0.0};
        return {RaySphereKind::TangentWithinPolicy,ray.at(t),t,t};
    }
    // The q-form reduces cancellation in the smaller quadratic root.
    // Equation: t^2+2bt+c=0, hence t roots are q and c/q.
    const double qstable=-b-std::copysign(root,b);
    const double t1=qstable;
    const double t2=(qstable==0.0) ? (-b+root) : c/qstable;
    if(!std::isfinite(t1)||!std::isfinite(t2))
        throw std::overflow_error("sphere root overflow");
    const double near_t=std::min(t1,t2);
    const double far_t=std::max(t1,t2);
    if(far_t<0.0) return {RaySphereKind::Behind,{},0.0,0.0};
    const double first=near_t>=0.0 ? near_t : far_t;
    return {RaySphereKind::Hit,ray.at(first),first,
            first==near_t ? far_t : near_t};
}

} // namespace book03
