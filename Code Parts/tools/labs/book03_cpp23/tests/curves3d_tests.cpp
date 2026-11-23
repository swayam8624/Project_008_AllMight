// Book03/P08 — independent curve, spline, patch and topology contracts.
//
// Tests distinguish mathematical exact identities from floating numerical
// comparisons. They do NOT claim certified Hausdorff approximation bounds,
// robust patch self-intersection, trimmed CAD, or arc-length parameterization.

#include "book03/curves3d.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using book03::Point3;
using book03::Vector3;
using book03::CubicBezier3;
using book03::CubicHermite3;
using book03::UniformCubicBSplineSpan3;
using book03::BicubicBezierPatch3;

void require(bool condition,std::string_view why) {
    if(!condition) throw std::runtime_error(std::string(why));
}

void near(double a,double b,double eps,std::string_view why) {
    if(!std::isfinite(a)||!std::isfinite(b)||
       std::abs(a-b)>eps*(1.0+std::abs(b)))
        throw std::runtime_error(std::string(why));
}
void near_point(Point3 a,Point3 b,double eps,std::string_view why) {
    near(a.x,b.x,eps,why);
    near(a.y,b.y,eps,why);
    near(a.z,b.z,eps,why);
}
void near_vec(Vector3 a,Vector3 b,double eps,std::string_view why) {
    near(a.x,b.x,eps,why);
    near(a.y,b.y,eps,why);
    near(a.z,b.z,eps,why);
}
template<class E,class F>
void rejects(F&& action,std::string_view why) {
    try {action();}
    catch(const E&) {return;}
    throw std::runtime_error(std::string(why));
}

void bezier_endpoints_derivatives_and_midpoint() {
    const CubicBezier3 c=book03::reference_garden_curve();
    near_point(c.at(0),{-2,0,0},0,"P0 is evaluated start");
    near_point(c.at(1),{2,0,0},0,"P3 is evaluated finish");
    near_point(c.at(0.5),{0,1.5,0},1e-15,
               "independent Bernstein hand midpoint of arch");
    near_vec(c.tangent(0),{0,6,0},0,
             "3*(P1-P0) endpoint tangent");
    near_vec(c.tangent(1),{0,-6,0},0,
             "3*(P3-P2) endpoint tangent");
    near_vec(c.tangent(0.5),{6,0,0},1e-15,
             "analytic mid-curve derivative");
    near_vec(c.acceleration(0.5),{0,-12,0},1e-15,
             "second derivative independent polynomial witness");

    // Independent Bernstein weights and de Casteljau path.
    for(int k=0;k<=2000;++k) {
        const double t=static_cast<double>(k)/2000.0;
        const Point3 a=c.at(t),b=c.bernstein_at(t);
        near_point(a,b,4e-13,
                   "de Casteljau and analytic Bernstein evaluate same curve");
        require(a.x>=-2.000000000001&&a.x<=2.000000000001 &&
                a.y>=-1e-12&&a.y<=2.000000000001,
                "convex hull bounding box includes curve point");
    }
    const auto weights=book03::cubic_bernstein(0.3);
    near(weights[0]+weights[1]+weights[2]+weights[3],
         1.0,1e-15,"Bernstein basis partition of unity");
}

void de_casteljau_split_is_reparameterization() {
    const auto original=book03::reference_garden_curve();
    const auto [left,right]=original.split(0.37);
    const Point3 join=original.at(0.37);
    near_point(left.p[3],join,1e-14,"left split ends at joined point");
    near_point(right.p[0],join,1e-14,"right split starts at joined point");
    for(int k=0;k<=300;++k) {
        const double s=static_cast<double>(k)/300.0;
        near_point(left.at(s),original.at(0.37*s),1e-12,
                   "subdivision left preserves geometric curve");
        near_point(right.at(s),
                   original.at(0.37+0.63*s),1e-12,
                   "subdivision right preserves geometric curve");
    }
    const auto [zero_left,zero_right]=original.split(0);
    near_point(zero_left.p[0],zero_left.p[3],0,
               "split at zero left curve collapses to start");
    near_point(zero_right.p[0],original.p[0],0,
               "split at zero right starts at original start");
}

void hermite_and_bezier_are_same_shape() {
    const CubicHermite3 h{
        {-2,0,0},{2,0,0},{0,6,0},{0,-6,0}
    };
    const auto bez=h.as_bezier();
    const auto reference=book03::reference_garden_curve();
    for(int k=0;k<=500;++k) {
        const double t=static_cast<double>(k)/500;
        near_point(h.at(t),bez.at(t),2e-12,
                   "independent direct Hermite vs control conversion");
        near_point(bez.at(t),reference.at(t),2e-12,
                   "Hermite endpoint derivative constructs same controls");
    }
    near_vec(bez.tangent(0),h.start_tangent,1e-14,
             "Hermite start tangent in normalized parameter");
    near_vec(bez.tangent(1),h.end_tangent,1e-14,
             "Hermite end tangent in normalized parameter");
}

void continuity_is_not_just_touching_endpoints() {
    const CubicBezier3 straight{{{
        {0,0,0},{1,0,0},{2,0,0},{3,0,0}
    }}};
    const CubicBezier3 c2{{{
        {3,0,0},{4,0,0},{5,0,0},{6,0,0}
    }}};
    const auto smooth=book03::join_continuity(straight,c2,1e-10);
    require(smooth.c0&&smooth.g1&&smooth.c1&&smooth.c2,
            "two identical parameter-speed lines join C2");

    const CubicBezier3 only_c1{{{
        {3,0,0},{4,0,0},{5,1,0},{6,1,0}
    }}};
    const auto join1=book03::join_continuity(straight,only_c1);
    require(join1.c0 && join1.g1 && join1.c1 && !join1.c2,
            "matching endpoint and derivative but different curvature-like second derivative");

    const CubicBezier3 only_g1{{{
        {3,0,0},{5,0,0},{6,0,0},{7,0,0}
    }}};
    const auto joing=book03::join_continuity(straight,only_g1);
    require(joing.c0 && joing.g1 && !joing.c1 && !joing.c2,
            "same positive tangent direction but different speed is G1 not C1");

    const CubicBezier3 corner{{{
        {3,0,0},{3,1,0},{3,2,0},{3,3,0}
    }}};
    const auto joint=book03::join_continuity(straight,corner);
    require(joint.c0 && !joint.g1 && !joint.c1,
            "touching at endpoint does not imply tangent continuity");

    const CubicBezier3 disconnected{{{
        {3.1,0,0},{4,0,0},{5,0,0},{6,0,0}
    }}};
    require(!book03::join_continuity(straight,disconnected).c0,
            "a visible endpoint gap fails even C0 continuity");
}

void uniform_cubic_spline_and_bezier_conversion() {
    const UniformCubicBSplineSpan3 linear{{{
        {0,0,0},{1,0,0},{2,0,0},{3,0,0}
    }}};
    near_point(linear.at(0),{1,0,0},1e-15,
               "uniform B-spline span does not start at P0");
    near_point(linear.at(1),{2,0,0},1e-15,
               "uniform span does not end at P3");
    for(int k=0;k<=600;++k) {
        const double t=static_cast<double>(k)/600;
        near_point(linear.at(t),{1+t,0,0},3e-13,
                   "uniform B-spline interpolates local line analytically");
        near_point(linear.at(t),linear.as_bezier().at(t),3e-13,
                   "B-spline basis and Bézier conversion agree");
    }

    const std::array<Point3,5> q{{
        {0,0,0},{1,2,0},{2,-1,0},{3,1,0},{4,0,0}
    }};
    const UniformCubicBSplineSpan3 first{{{
        q[0],q[1],q[2],q[3]
    }}};
    const UniformCubicBSplineSpan3 second{{{
        q[1],q[2],q[3],q[4]
    }}};
    const auto c1=first.as_bezier(),c2=second.as_bezier();
    const auto join=book03::join_continuity(c1,c2,1e-12);
    require(join.c0&&join.c1&&join.c2&&join.g1,
            "adjacent equal-knot uniform cubic B-spline spans are C2");
    for(int k=0;k<=400;++k) {
        const double t=static_cast<double>(k)/400.0;
        near_point(first.at(t),c1.at(t),3e-13,
                   "first spline span equals converted Bézier");
        near_point(second.at(t),c2.at(t),3e-13,
                   "second spline span equals converted Bézier");
    }
}

void adaptive_chord_tessellation_and_depth_exhaustion() {
    const CubicBezier3 straight{{{
        {0,0,0},{1,0,0},{2,0,0},{3,0,0}
    }}};
    const auto coarse=book03::tessellate_curve(straight,0.01);
    require(coarse.samples.size()==2&&coarse.terminal_segments==1,
            "exactly collinear controls need one chord");
    require(coarse.tolerance_met_under_float_policy,
            "zero control-bound always meets positive tolerance");

    const auto c=book03::reference_garden_curve();
    const double tolerance=0.015;
    const auto fine=book03::tessellate_curve(c,tolerance,13);
    require(fine.samples.size()>5 &&
            fine.samples.size()==fine.terminal_segments+1,
            "arch needs multiple adaptively refined chords");
    require(fine.tolerance_met_under_float_policy &&
            fine.max_terminal_control_bound<=tolerance,
            "all terminal bounds meet requested classroom criterion");
    near_point(fine.samples.front().p,c.p[0],0,
               "first tessellation vertex is P0");
    near_point(fine.samples.back().p,c.p[3],0,
               "last tessellation vertex is P3");

    for(std::size_t i=1;i<fine.samples.size();++i) {
        require(fine.samples[i].t>fine.samples[i-1].t,
                "curve tessellation parameter values strictly increase");
        near_point(fine.samples[i].p,c.at(fine.samples[i].t),3e-13,
                   "sample positions correspond to original curve");
    }

    // Dense independent samples check visible point-to-polyline distance.
    // Not a formal floating-point Hausdorff proof, but a useful falsifier.
    std::size_t segment=1;
    double worst=0;
    for(int k=0;k<=2000;++k) {
        const double t=static_cast<double>(k)/2000;
        while(segment+1<fine.samples.size() &&
              t>fine.samples[segment].t)
            ++segment;
        worst=std::max(worst,book03::chord_point_distance(
            c.at(t),fine.samples[segment-1].p,fine.samples[segment].p));
    }
    require(worst<=tolerance*1.01,
            "dense sampled arch stays near its returned polyline");

    const auto depth_limited=book03::tessellate_curve(c,1e-8,0);
    require(!depth_limited.tolerance_met_under_float_policy &&
            depth_limited.samples.size()==2,
            "depth budget failure must be reported, never hidden");

    const CubicBezier3 cusp{{{
        {0,0,0},{3,2,0},{-3,2,0},{0,0,0}
    }}};
    const auto loop=book03::tessellate_curve(cusp,0.03);
    require(loop.samples.size()>2&&
            loop.tolerance_met_under_float_policy,
            "zero endpoint chord must not erase a genuine loop");
}

void bicubic_patch_derivatives_and_indexed_mesh() {
    BicubicBezierPatch3 plane{};
    for(int i=0;i<4;++i)
        for(int j=0;j<4;++j)
            plane.control[i][j]={static_cast<double>(i),
                                  static_cast<double>(j),0};
    near_point(plane.at(0.4,0.6),{1.2,1.8,0},2e-14,
               "tensor-product linear plane evaluation");
    near_vec(plane.partial_u(0.3,0.7),{3,0,0},1e-13,
             "independent affine patch partial derivative along u");
    near_vec(plane.partial_v(0.3,0.7),{0,3,0},1e-13,
             "independent affine patch partial derivative along v");
    near_vec(plane.normal(0.3,0.7),{0,0,1},1e-13,
             "patch normal from du cross dv");

    const auto mesh=book03::patch_to_indexed_mesh(plane,2,3);
    const auto report=book03::inspect_mesh(mesh);
    require(mesh.positions.size()==12 && mesh.triangles.size()==12,
            "2 by 3 patch grid has 12 vertices and 12 triangles");
    require(report.edge_count==23 && report.boundary_edges==10 &&
            report.consistent_interior_edges==13,
            "independent grid E=3mn+m+n and boundary 2(m+n)");
    require(report.euler_characteristic==1 &&
            report.face_edge_components==1 &&
            report.disconnected_vertex_fans==0,
            "indexed rectangular patch is one coherent topological disk");
    require(report.winding_conflict_edges==0 &&
            report.nonmanifold_edges==0,
            "shared grid indices have opposite edge traversal");
    for(const auto& t:mesh.triangles) {
        const book03::Triangle3 tri{
            mesh.positions[t.vertex[0]],
            mesh.positions[t.vertex[1]],
            mesh.positions[t.vertex[2]]};
        near_vec(tri.face_normal(),{0,0,1},1e-13,
                 "sampled planar patch faces preserve expected normal");
    }

    const auto garden=book03::reference_garden_patch();
    const auto real=book03::patch_to_indexed_mesh(garden,6,7);
    const auto r=book03::inspect_mesh(real);
    require(r.position_count==56 && r.face_count==84 &&
            r.edge_count==139 && r.boundary_edges==26 &&
            r.euler_characteristic==1 &&
            r.face_edge_components==1 &&
            r.winding_conflict_edges==0 &&
            r.nonmanifold_edges==0,
            "curved garden patch remains valid as indexed grid topology");
    require(garden.at(0.5,0.5).y>garden.at(0,0).y,
            "interior canopy is higher than boundary control point");
}

void malformed_parameter_and_geometry_rejected() {
    const auto curve=book03::reference_garden_curve();
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    rejects<std::invalid_argument>(
        [&]{(void)curve.at(-0.1);},"negative t rejected");
    rejects<std::invalid_argument>(
        [&]{(void)curve.at(1.0001);},"t over one rejected");
    rejects<std::invalid_argument>(
        [&]{(void)curve.at(nan);},"NaN t rejected");
    rejects<std::invalid_argument>(
        [&]{(void)book03::tessellate_curve(curve,0.0);},
        "nonpositive flattening tolerance rejected");
    rejects<std::invalid_argument>(
        [&]{(void)book03::tessellate_curve(curve,0.01,17);},
        "excessive recursion memory depth rejected");
    rejects<std::invalid_argument>(
        [&]{(void)book03::join_continuity(curve,curve,-1);},
        "negative joining tolerance rejected");
    rejects<std::invalid_argument>(
        [&]{(void)book03::patch_to_indexed_mesh(
            book03::reference_garden_patch(),0,3);},
        "zero grid resolution rejected");
    rejects<std::invalid_argument>(
        [&]{(void)book03::patch_to_indexed_mesh(
            book03::reference_garden_patch(),129,3);},
        "overlarge grid resolution rejected");

    auto invalid=curve;
    invalid.p[1].x=inf;
    rejects<std::invalid_argument>(
        [&]{(void)invalid.at(0.5);},
        "nonfinite Bézier control rejected");
    auto bad=book03::reference_garden_patch();
    bad.control[2][2].y=nan;
    rejects<std::invalid_argument>(
        [&]{(void)bad.at(0.5,0.5);},
        "nonfinite patch control rejected");
    BicubicBezierPatch3 collapsed{};
    for(auto& row:collapsed.control)
        for(auto& p:row)p={0,0,0};
    rejects<std::invalid_argument>(
        [&]{(void)collapsed.normal(0.5,0.5);},
        "singular patch normal must not be invented");
}

} // namespace

int main() {
    try {
        bezier_endpoints_derivatives_and_midpoint();
        de_casteljau_split_is_reparameterization();
        hermite_and_bezier_are_same_shape();
        continuity_is_not_just_touching_endpoints();
        uniform_cubic_spline_and_bezier_conversion();
        adaptive_chord_tessellation_and_depth_exhaustion();
        bicubic_patch_derivatives_and_indexed_mesh();
        malformed_parameter_and_geometry_rejected();
        std::cout<<"Book03/P08: cubic curve, C/G joins, spline spans, "
                    "adaptive chords and indexed patches: PASS\n";
        return 0;
    } catch(const std::exception& err) {
        std::cerr<<"Book03/P08 FAILED: "<<err.what()<<'\n';
        return 1;
    }
}
