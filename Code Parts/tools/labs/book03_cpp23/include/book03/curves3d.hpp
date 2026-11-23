#pragma once

// Book 03 / Chapter 08 — cubic curves, derivatives, C/G joins, patches.
//
// Native C++23; no graphics API or external math package.
// Reuses Chapter 06 Point3/Vector3 and Chapter 07 indexed Mesh3.
//
// Important conventions:
// * All curve parameters t,u,v lie in the CLOSED interval [0,1].
// * Cubic Bézier endpoints are P0 and P3. Inner controls are generally
//   NOT points on the curve. We do not assert equal parameter steps are
//   equal arc-length steps.
// * Hermite tangents are dP/dt on the normalized parameter domain.
// * The local UNIFORM cubic B-spline span generally DOES NOT interpolate
//   its first and last control vertices; it can be converted to a Bézier
//   span without changing its geometric locus.
// * C1/C2 compares derivatives under a shared parameter speed; G1 only
//   requires a positive collinear direction (within stated tolerance).
// * de Casteljau and direct Bernstein paths are independent C++ methods,
//   useful for cross-checks. Both use floating arithmetic and can round.
// * Adaptive tessellation checks a mathematically conservative control-
//   hull-to-chord distance in exact arithmetic. Because it is evaluated
//   in double, returned "met tolerance" is an EDUCATIONAL floating-policy
//   result, NOT a rigorously certified world-space Hausdorff guarantee.
// * A bicubic tensor-product patch is NOT a trimmed CAD surface or NURBS.
//   Its generated rectangular parameter grid is a triangle mesh with
//   shared INDEXED grid vertices; it does not automatically stitch an
//   adjacent patch's boundaries, detect singular normals or self-intersect.
// * Every numeric output is checked finite; degenerate or unsupported
//   parameter ranges are rejected, not silently clamped.
//
// The deliberately small API is an approachable reference implementation,
// not a production CAGD/mesh-refinement library or curvature fitter.

#include "book03/mesh_topology.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace book03 {

[[nodiscard]] inline double curve_parameter(double t) {
    if(!std::isfinite(t) || t<0.0 || t>1.0)
        throw std::invalid_argument("curve parameter must be finite in [0,1]");
    return t;
}

[[nodiscard]] inline Point3 curve_lerp(Point3 a,Point3 b,double t) {
    curve_parameter(t);
    if(!finite(a)||!finite(b))
        throw std::invalid_argument("curve requires finite control points");
    const Point3 out=a+t*(b-a);
    if(!finite(out)) throw std::overflow_error("curve lerp overflow");
    return out;
}

[[nodiscard]] inline Vector3 curve_vector_lerp(
    Vector3 a,Vector3 b,double t) {
    curve_parameter(t);
    if(!finite(a)||!finite(b))
        throw std::invalid_argument("curve requires finite vectors");
    const Vector3 out=a+t*(b-a);
    if(!finite(out)) throw std::overflow_error("curve vector lerp overflow");
    return out;
}

[[nodiscard]] inline Vector3 cubic_vector_casteljau(
    std::array<Vector3,4> controls,double t) {
    curve_parameter(t);
    for(int width=3;width>0;--width)
        for(int i=0;i<width;++i)
            controls[i]=curve_vector_lerp(controls[i],controls[i+1],t);
    return controls[0];
}

// Bernstein's convex coefficients. For t in [0,1] each coefficient is
// nonnegative and the sum is one (modulo floating rounding). This underpins
// the convex-hull and adaptive chord-distance arguments.
[[nodiscard]] inline std::array<double,4> cubic_bernstein(double t) {
    curve_parameter(t);
    const double q=1.0-t;
    return {q*q*q, 3.0*t*q*q, 3.0*t*t*q, t*t*t};
}

[[nodiscard]] inline Point3 weighted_point(
    const std::array<Point3,4>& p,
    const std::array<double,4>& w) {
    Point3 out{};
    for(std::size_t i=0;i<4;++i) {
        if(!finite(p[i])||!std::isfinite(w[i]))
            throw std::invalid_argument("nonfinite weighted curve input");
        out.x+=w[i]*p[i].x;
        out.y+=w[i]*p[i].y;
        out.z+=w[i]*p[i].z;
    }
    if(!finite(out))
        throw std::overflow_error("weighted curve result nonfinite");
    return out;
}

struct CubicBezier3 {
    std::array<Point3,4> p{};

    void validate() const {
        for(Point3 q:p) if(!finite(q))
            throw std::invalid_argument("nonfinite Bézier control point");
    }

    [[nodiscard]] Point3 at(double t) const {
        // Iterative de Casteljau: three layers of linear interpolation.
        validate();
        curve_parameter(t);
        auto level=p;
        for(int width=3;width>0;--width)
            for(int i=0;i<width;++i)
                level[i]=curve_lerp(level[i],level[i+1],t);
        return level[0];
    }

    [[nodiscard]] Point3 bernstein_at(double t) const {
        // Separate analytic formulation. The test suite compares this
        // against de Casteljau for ordinary well-scaled input.
        validate();
        return weighted_point(p,cubic_bernstein(t));
    }

    [[nodiscard]] Vector3 tangent(double t) const {
        validate();
        curve_parameter(t);
        const std::array<Vector3,4> d{
            3.0*(p[1]-p[0]),
            3.0*(p[2]-p[1]),
            3.0*(p[3]-p[2]),
            {0,0,0}
        };
        // Quadratic de Casteljau for derivative controls.
        const Vector3 a=curve_vector_lerp(d[0],d[1],t);
        const Vector3 b=curve_vector_lerp(d[1],d[2],t);
        return curve_vector_lerp(a,b,t);
    }

    [[nodiscard]] Vector3 acceleration(double t) const {
        validate();
        curve_parameter(t);
        const Vector3 a=6.0*((p[2]-p[1])-(p[1]-p[0]));
        const Vector3 b=6.0*((p[3]-p[2])-(p[2]-p[1]));
        return curve_vector_lerp(a,b,t);
    }

    [[nodiscard]] std::pair<CubicBezier3,CubicBezier3> split(
        double t) const {
        validate();
        curve_parameter(t);
        const Point3 p01=curve_lerp(p[0],p[1],t);
        const Point3 p12=curve_lerp(p[1],p[2],t);
        const Point3 p23=curve_lerp(p[2],p[3],t);
        const Point3 p012=curve_lerp(p01,p12,t);
        const Point3 p123=curve_lerp(p12,p23,t);
        const Point3 middle=curve_lerp(p012,p123,t);
        return {
            CubicBezier3{{p[0],p01,p012,middle}},
            CubicBezier3{{middle,p123,p23,p[3]}}
        };
    }
};

struct CubicHermite3 {
    Point3 start{},end{};
    Vector3 start_tangent{},end_tangent{}; // dP/dt over t ∈ [0,1].

    void validate() const {
        if(!finite(start)||!finite(end)||
           !finite(start_tangent)||!finite(end_tangent))
            throw std::invalid_argument("nonfinite Hermite inputs");
    }

    [[nodiscard]] Point3 at(double t) const {
        validate();
        curve_parameter(t);
        const double t2=t*t,t3=t2*t;
        const double h00=2*t3-3*t2+1;
        const double h10=t3-2*t2+t;
        const double h01=-2*t3+3*t2;
        const double h11=t3-t2;
        const Point3 out{
            h00*start.x+h10*start_tangent.x+
                h01*end.x+h11*end_tangent.x,
            h00*start.y+h10*start_tangent.y+
                h01*end.y+h11*end_tangent.y,
            h00*start.z+h10*start_tangent.z+
                h01*end.z+h11*end_tangent.z
        };
        if(!finite(out))
            throw std::overflow_error("Hermite evaluation overflow");
        return out;
    }

    [[nodiscard]] CubicBezier3 as_bezier() const {
        validate();
        return {{{start,
                  start+(1.0/3.0)*start_tangent,
                  end-(1.0/3.0)*end_tangent,
                  end}}};
    }
};

struct UniformCubicBSplineSpan3 {
    // One local unit span of a NONRATIONAL UNIFORM cubic B-spline.
    // The four controls generally are NOT interpolated endpoints.
    std::array<Point3,4> control{};

    void validate() const {
        for(Point3 p:control) if(!finite(p))
            throw std::invalid_argument("nonfinite B-spline control");
    }

    [[nodiscard]] Point3 at(double t) const {
        validate();
        curve_parameter(t);
        const double t2=t*t,t3=t2*t;
        const std::array<double,4> weights{
            (1.0-3*t+3*t2-t3)/6.0,
            (4.0-6*t2+3*t3)/6.0,
            (1.0+3*t+3*t2-3*t3)/6.0,
            t3/6.0
        };
        return weighted_point(control,weights);
    }

    [[nodiscard]] CubicBezier3 as_bezier() const {
        validate();
        const auto& p=control;
        return {{weighted_point(p,{1.0/6,4.0/6,1.0/6,0}),
                 weighted_point(p,{0,2.0/3,1.0/3,0}),
                 weighted_point(p,{0,1.0/3,2.0/3,0}),
                 weighted_point(p,{0,1.0/6,4.0/6,1.0/6})}};
    }
};

struct CurveContinuity {
    double position_gap{};
    double tangent_gap{};
    double acceleration_gap{};
    bool c0{},g1{},c1{},c2{};
};

[[nodiscard]] inline CurveContinuity join_continuity(
    const CubicBezier3& left,const CubicBezier3& right,
    double length_tolerance=1e-9,
    double angular_tolerance=1e-9) {
    left.validate();
    right.validate();
    if(!std::isfinite(length_tolerance)||length_tolerance<0 ||
       !std::isfinite(angular_tolerance)||angular_tolerance<0 ||
       angular_tolerance>1.0)
        throw std::invalid_argument("invalid curve join tolerances");

    const Vector3 p=right.p[0]-left.p[3];
    const Vector3 t0=left.tangent(1.0),t1=right.tangent(0.0);
    const Vector3 a0=left.acceleration(1.0),
                  a1=right.acceleration(0.0);
    CurveContinuity r{};
    r.position_gap=magnitude(p);
    r.tangent_gap=magnitude(t1-t0);
    r.acceleration_gap=magnitude(a1-a0);
    r.c0=r.position_gap<=length_tolerance;
    r.c1=r.c0&&r.tangent_gap<=length_tolerance;
    r.c2=r.c1&&r.acceleration_gap<=length_tolerance;

    const double l0=magnitude(t0),l1=magnitude(t1);
    if(r.c0 && l0>0 && l1>0) {
        const Vector3 u0=unit(t0),u1=unit(t1);
        const double alignment=dot(u0,u1);
        const double sideways=magnitude(cross(u0,u1));
        // Positive directional alignment avoids treating a 180° reversal
        // as a smooth G1 join. Angular tolerance is sine-like here.
        r.g1=alignment>0.0 && sideways<=angular_tolerance;
    }
    return r;
}

[[nodiscard]] inline double chord_point_distance(
    Point3 point,Point3 a,Point3 b) {
    if(!finite(point)||!finite(a)||!finite(b))
        throw std::invalid_argument("nonfinite flattening control");
    const Vector3 chord=b-a;
    const double size=magnitude(chord);
    if(size==0.0) return magnitude(point-a);
    const Vector3 direction=unit(chord);
    const double along=std::clamp(dot(point-a,direction),0.0,size);
    const Point3 closest=a+along*direction;
    return magnitude(point-closest);
}

[[nodiscard]] inline double cubic_chord_control_bound(
    const CubicBezier3& curve) {
    curve.validate();
    const double d1=chord_point_distance(
        curve.p[1],curve.p[0],curve.p[3]);
    const double d2=chord_point_distance(
        curve.p[2],curve.p[0],curve.p[3]);
    return std::max(d1,d2);
}

struct CurveSample {
    double t{};
    Point3 p{};
};
struct CurveTessellation {
    std::vector<CurveSample> samples{};
    double max_terminal_control_bound{};
    bool tolerance_met_under_float_policy{true};
    std::size_t terminal_segments{};
};

[[nodiscard]] inline CurveTessellation tessellate_curve(
    const CubicBezier3& curve,double tolerance,
    unsigned max_depth=12) {
    curve.validate();
    if(!std::isfinite(tolerance)||!(tolerance>0.0))
        throw std::invalid_argument("tessellation tolerance must be >0");
    if(max_depth>16)
        throw std::invalid_argument("maximum tessellation depth is 16");

    CurveTessellation out{};
    out.samples.push_back({0.0,curve.p[0]});

    // Explicit iterative de Casteljau subdivision avoids unbounded call
    // stack and records each terminal interval's actual parameter value.
    struct Pending {
        CubicBezier3 piece{};
        double t0{},t1{};
        unsigned depth{};
    };
    std::vector<Pending> stack{};
    stack.push_back({curve,0.0,1.0,0});
    while(!stack.empty()) {
        const Pending current=stack.back();
        stack.pop_back();
        const double bound=cubic_chord_control_bound(current.piece);
        if(bound<=tolerance||current.depth>=max_depth) {
            out.samples.push_back({current.t1,current.piece.p[3]});
            out.max_terminal_control_bound=std::max(
                bound,out.max_terminal_control_bound);
            if(bound>tolerance)
                out.tolerance_met_under_float_policy=false;
            ++out.terminal_segments;
        } else {
            const auto [left,right]=current.piece.split(0.5);
            const double mid=(current.t0+current.t1)/2.0;
            // Stack is LIFO. Add right first so output t is monotonic.
            stack.push_back({right,mid,current.t1,current.depth+1});
            stack.push_back({left,current.t0,mid,current.depth+1});
        }
    }
    return out;
}

// Bicubic tensor-product Bézier patch P(u,v), each parameter in [0,1].
// control[i][j] corresponds to u-axis control index i and v-axis index j.
// Its derivatives are dP/du and dP/dv (metres per unit parameter).
struct BicubicBezierPatch3 {
    std::array<std::array<Point3,4>,4> control{};

    void validate() const {
        for(const auto& row:control)
            for(Point3 p:row) if(!finite(p))
                throw std::invalid_argument("nonfinite patch control");
    }

    [[nodiscard]] Point3 at(double u,double v) const {
        validate();
        curve_parameter(u);
        curve_parameter(v);
        std::array<Point3,4> columns{};
        for(int j=0;j<4;++j) {
            columns[j]=CubicBezier3{{control[0][j],control[1][j],
                                     control[2][j],control[3][j]}}.at(u);
        }
        return CubicBezier3{columns}.at(v);
    }

    [[nodiscard]] Vector3 partial_u(double u,double v) const {
        validate();
        std::array<Vector3,4> derivatives{};
        for(int j=0;j<4;++j) {
            derivatives[j]=CubicBezier3{{control[0][j],control[1][j],
                                         control[2][j],control[3][j]}}
                                         .tangent(u);
        }
        return cubic_vector_casteljau(derivatives,v);
    }

    [[nodiscard]] Vector3 partial_v(double u,double v) const {
        validate();
        std::array<Vector3,4> derivatives{};
        for(int i=0;i<4;++i) {
            derivatives[i]=CubicBezier3{control[i]}.tangent(v);
        }
        return cubic_vector_casteljau(derivatives,u);
    }

    [[nodiscard]] Vector3 normal(double u,double v) const {
        return unit(cross(partial_u(u,v),partial_v(u,v)));
    }
};

[[nodiscard]] inline Mesh3 patch_to_indexed_mesh(
    const BicubicBezierPatch3& patch,
    std::uint32_t u_segments,std::uint32_t v_segments) {
    patch.validate();
    if(u_segments==0||v_segments==0||
       u_segments>128||v_segments>128)
        throw std::invalid_argument(
            "patch grid segments must be in [1,128]");
    Mesh3 mesh{};
    const auto width=static_cast<std::size_t>(v_segments)+1u;
    const auto index=[width](std::uint32_t i,std::uint32_t j) {
        return static_cast<std::uint32_t>(
            static_cast<std::size_t>(i)*width+j);
    };
    for(std::uint32_t i=0;i<=u_segments;++i)
        for(std::uint32_t j=0;j<=v_segments;++j)
            mesh.positions.push_back(patch.at(
                static_cast<double>(i)/u_segments,
                static_cast<double>(j)/v_segments));

    for(std::uint32_t i=0;i<u_segments;++i) {
        for(std::uint32_t j=0;j<v_segments;++j) {
            const auto a=index(i,j),b=index(i+1,j),
                       c=index(i+1,j+1),d=index(i,j+1);
            // Consistent oriented shared diagonal a-c.
            mesh.triangles.push_back({{a,b,c}});
            mesh.triangles.push_back({{a,c,d}});
        }
    }
    mesh.validate();
    return mesh;
}

[[nodiscard]] inline CubicBezier3 reference_garden_curve() {
    // Upward leaf-like arch. Parameter is NOT arc length.
    return {{{{-2.0,0.0,0.0},{-2.0,2.0,0.0},
              {2.0,2.0,0.0},{2.0,0.0,0.0}}}};
}

[[nodiscard]] inline BicubicBezierPatch3 reference_garden_patch() {
    BicubicBezierPatch3 patch{};
    // A curved but well-defined 4x4 garden canopy. The boundary is at y=2.
    // Interior controls increase height, so the patch is not a plane.
    for(int i=0;i<4;++i)
        for(int j=0;j<4;++j) {
            const double hill=(i==1||i==2)&&(j==1||j==2)?1.8:0.0;
            patch.control[i][j] = {
                -2.0+4.0*static_cast<double>(i)/3.0,
                2.0+hill,
                -2.0+4.0*static_cast<double>(j)/3.0
            };
        }
    return patch;
}

} // namespace book03
