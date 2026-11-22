// Book 03 / P04 — affine transformation contracts and adversarial tests.
//
// These witnesses complement the older frame and barycentric suites.
// Whenever possible, outputs are checked against scalar hand calculations
// or an independent Frame2 implementation, not another use of the same
// affine multiplication. The inverse/conditioning policy is classroom-only.

#include "book03/affine2.hpp"
#include "book03/triangle_geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::Affine2;
using book03::Frame2;
using book03::Point2;
using book03::Vector2;
using book03::Triangle2;
using book03::ApproxLocation;

void require(bool yes, std::string_view why) {
    if (!yes) throw std::runtime_error(std::string(why));
}

void near(double a,double b,double tol,std::string_view why) {
    if (!std::isfinite(a) || std::abs(a-b)>tol)
        throw std::runtime_error(std::string(why));
}

void near_pt(Point2 a,Point2 b,double tol,std::string_view why) {
    near(a.x,b.x,tol,why);
    near(a.y,b.y,tol,why);
}

void near_vec(Vector2 a,Vector2 b,double tol,std::string_view why) {
    near(a.x,b.x,tol,why);
    near(a.y,b.y,tol,why);
}

template<class Exception,class Fn>
void throws(Fn&& fn,std::string_view why) {
    try { fn(); }
    catch(const Exception&) { return; }
    throw std::runtime_error(std::string(why));
}

void identity_and_storage() {
    const auto m=Affine2::identity();
    near_pt(m.transform_point({4.0,-6.0}),{4.0,-6.0},0.0,"identity point");
    near_vec(m.transform_vector({4.0,-6.0}),{4.0,-6.0},0.0,"identity vector");
    near(m.determinant(),1.0,0.0,"identity determinant");
    const auto data=m.row_major_3x3();
    require(data == std::array<double,9>{1,0,0,0,1,0,0,0,1},
            "row-major output is matrix storage, not multiplication convention");

    const auto t=Affine2::translation(3.0,-2.0);
    near_pt(t.transform_point({1,2}),{4,0},0,"translation point");
    near_vec(t.transform_vector({1,2}),{1,2},0,"translation must not move vector");
    require(t.row_major_3x3() ==
            std::array<double,9>{1,0,3,0,1,-2,0,0,1},
            "translation is the last COLUMN in a column-vector matrix");
}

void transform_family() {
    const auto scale=Affine2::scale(2,3);
    near_pt(scale.transform_point({1,1}),{2,3},0,"nonuniform scale");
    near(scale.determinant(),6,0,"sixfold oriented area");

    const auto shear=Affine2::shear_x(0.5);
    near_pt(shear.transform_point({2,3}),{3.5,3},0,"horizontal shear");
    near(shear.determinant(),1.0,0,"shear preserves area");
    near_pt(shear.inverse().transform_point({3.5,3}),{2,3},0,
            "shear inverse is not transpose");

    const auto reflect=Affine2::reflect_about_world_y();
    near_pt(reflect.transform_point({2,3}),{-2,3},0,"reflect world x");
    near(reflect.determinant(),-1.0,0,"reflection flips orientation");

    const auto r=Affine2::rotation_degrees(90);
    near_pt(r.transform_point({2,1}),{-1,2},1e-12,"CCW rotation");
    near(r.determinant(),1.0,1e-12,"rotation area preserving");
}

void order_and_pivot() {
    const auto t=Affine2::translation(2,0);
    const auto r=Affine2::rotation_degrees(90);
    near_pt((t*r).transform_point({1,0}),{2,1},1e-12,
            "rightmost rotation first");
    near_pt((r*t).transform_point({1,0}),{0,3},1e-12,
            "rightmost translation first");
    near_pt((t*r).transform_point({1,0}),
            t.transform_point(r.transform_point({1,0})),1e-12,
            "composed evaluation equals separate evaluation");

    const auto pivot=Affine2::around_pivot(r,{2,1});
    near_pt(pivot.transform_point({2,1}),{2,1},1e-12,"pivot must not move");
    near_pt(pivot.transform_point({3,1}),{2,2},1e-12,"door corner around hinge");

    throws<std::invalid_argument>(
        [&] { (void)Affine2::around_pivot(t,{2,1}); },
        "pivot API must reject pretranslated linear input");
}

void hand_derived_inverse() {
    // A=[[2,1],[0,1]], t=(3,-2), P=(1,2) -> W=(7,0).
    const Affine2 m{2,1,0,1,3,-2};
    near(m.determinant(),2.0,0.0,"known affine determinant");
    near_pt(m.transform_point({1,2}),{7,0},0.0,"independent scalar forward");
    near_pt(m.inverse().transform_point({7,0}),{1,2},0.0,
            "explicit inverse calculation");
    near_pt((m.inverse()*m).transform_point({4,-5}),{4,-5},1e-12,
            "matrix inverse left identity");
    near_pt((m*m.inverse()).transform_point({4,-5}),{4,-5},1e-12,
            "matrix inverse right identity");

    const auto uniform_tiny=Affine2::scale(1e-12,1e-12);
    // Its determinant is small, but the two axes are not ill-conditioned
    // relative to each other. The inverse guard is scale-independent here.
    near_pt(uniform_tiny.inverse().transform_point({1e-12,2e-12}),
            {1,2},1e-12,"uniform small scale is not anisotropy");
}

void frames_agree_with_independent_implementation() {
    const Frame2 frame{{2,1},{0,1},{-1,0}};
    const auto m=Affine2::from_frame(frame);
    const Point2 local{3,1};
    near_pt(frame.local_to_world(local),{1,4},0.0,"frame scalar witness");
    near_pt(m.transform_point(local),
            frame.local_to_world(local),0.0,"Frame2 agrees with Affine2");
    near_pt(m.inverse().transform_point({1,4}),
            frame.world_to_local({1,4}),0.0,"inverse APIs agree");
    const auto returned=m.as_frame();
    near_pt(returned.local_to_world(local),{1,4},0.0,
            "representation round trip");

    const auto parent=Frame2::rotation({3,2},book03::degrees_to_radians(20));
    const auto child=Frame2::rotation({-1,4},book03::degrees_to_radians(-45));
    const Point2 p{2,-3};
    const auto scalar=parent.local_to_world(child.local_to_world(p));
    const auto matrix=(Affine2::from_frame(parent) *
                       Affine2::from_frame(child)).transform_point(p);
    near_pt(matrix,scalar,1e-12,"independent nested frame composition");
}

void triangle_area_and_barycentric_invariance() {
    const Triangle2 triangle{{0,0},{6,0},{0,4}};
    const Point2 query{1.5,1};
    const auto original=triangle.sample(query);
    require(original.location==ApproxLocation::Interior,
            "reference triangle interior");

    for(const Affine2 m:std::array<Affine2,4>{
            Affine2::translation(1,2),
            Affine2::scale(2,3),
            Affine2::shear_x(0.5),
            Affine2::reflect_about_world_y()}) {
        const Triangle2 moved{
            m.transform_point(triangle.a),
            m.transform_point(triangle.b),
            m.transform_point(triangle.c)};
        const auto result=moved.sample(m.transform_point(query));
        require(result.location==ApproxLocation::Interior,
                "nonsingular affine map preserves relative interior");
        near(moved.twice_signed_area(),
             m.determinant()*triangle.twice_signed_area(),1e-12,
             "determinant scales signed triangle area");
        near(result.barycentric.at_a,original.barycentric.at_a,1e-12,
             "weight A affine invariant");
        near(result.barycentric.at_b,original.barycentric.at_b,1e-12,
             "weight B affine invariant");
        near(result.barycentric.at_c,original.barycentric.at_c,1e-12,
             "weight C affine invariant");
    }

    const auto collapsed=Affine2::scale(1,0);
    const Triangle2 flat{
        collapsed.transform_point(triangle.a),
        collapsed.transform_point(triangle.b),
        collapsed.transform_point(triangle.c)};
    near(flat.twice_signed_area(),0,0,"zero scale collapses area");
    require(flat.sample({1,0}).location ==
            ApproxLocation::DegenerateOrUncertified,
            "a flattened triangle cannot have ordinary barycentrics");
}

void normals_follow_inverse_transpose() {
    // Vertical tangent (0,1), horizontal normal (1,0).
    // Under shear x'=x+y, transformed tangent (1,1).
    // Correct unnormalized normal is (1,-1), with zero dot product.
    const auto shear=Affine2::shear_x(1.0);
    const Vector2 tangent=shear.transform_vector({0,1});
    const Vector2 normal=shear.transform_normal({1,0});
    near_vec(tangent,{1,1},0,"sheared tangent");
    near_vec(normal,{1,-1},0,"inverse transpose normal");
    near(normal.x*tangent.x+normal.y*tangent.y,0.0,0.0,
         "normal must remain perpendicular");
    const Vector2 naive=shear.transform_vector({1,0});
    require(naive.x*tangent.x+naive.y*tangent.y != 0.0,
            "ordinary vector transform gives wrong sheared normal");
}

void malformed_inputs_and_policy() {
    const double inf=std::numeric_limits<double>::infinity();
    const double nan=std::numeric_limits<double>::quiet_NaN();
    throws<std::invalid_argument>(
        [&] { (void)Affine2::translation(nan,0); },
        "nonfinite translation rejected");
    throws<std::invalid_argument>(
        [&] { (void)Affine2::rotation_degrees(inf); },
        "nonfinite rotation angle rejected");
    throws<std::invalid_argument>(
        [&] { (void)Affine2::scale(1,0).inverse(); },
        "singular inverse rejected");
    throws<std::invalid_argument>(
        [&] { (void)Affine2::scale(1,1e-12).inverse(); },
        "severely anisotropic inverse rejected by stated policy");
    throws<std::invalid_argument>(
        [&] { (void)Affine2::identity().transform_normal({0,0}); },
        "a zero normal has no direction");
    throws<std::invalid_argument>(
        [&] { (void)Affine2::translation(1,0).transform_point({inf,0}); },
        "nonfinite point rejected");
    throws<std::overflow_error>(
        [] { (void)Affine2::scale(1e308,1).transform_point({1e308,1}); },
        "finite inputs can overflow transformed result");
    throws<std::overflow_error>(
        [] { (void)(Affine2::scale(1e308,1e308) *
                    Affine2::scale(1e308,1e308)); },
        "composition overflow rejected");

    const Affine2 not_invertible{1,2,2,4,0,0};
    throws<std::invalid_argument>(
        [&] { (void)not_invertible.inverse(); },
        "linearly dependent axes rejected");
}

void deterministic_property_sweep() {
    std::uint32_t state=0x91A6B317u;
    auto next=[&]() -> double {
        state^=state<<13;
        state^=state>>17;
        state^=state<<5;
        return (static_cast<double>(state % 2001u)-1000.0)/200.0;
    };

    for(int i=0;i<2000;++i) {
        const auto m=Affine2::translation(next(),next()) *
                     Affine2::rotation_degrees(next()*20.0) *
                     Affine2::shear_x(next()*0.1) *
                     Affine2::scale(1.1+std::abs(next())*0.2,
                                    1.1+std::abs(next())*0.2);
        const Point2 p{next(),next()};
        const Point2 world=m.transform_point(p);
        const Point2 back=m.inverse().transform_point(world);
        near_pt(back,p,1e-10,"seeded transform-inverse round-trip");
        const Vector2 v{next(),next()};
        const auto mapped=m.transform_vector(v);
        const auto shifted=m.transform_point(p+v);
        const auto base=m.transform_point(p);
        near_vec(shifted-base,mapped,1e-10,
                 "affine change of point equals transformed displacement");
    }
}

} // namespace

int main() {
    try {
        identity_and_storage();
        transform_family();
        order_and_pivot();
        hand_derived_inverse();
        frames_agree_with_independent_implementation();
        triangle_area_and_barycentric_invariance();
        normals_follow_inverse_transpose();
        malformed_inputs_and_policy();
        deterministic_property_sweep();
        std::cout << "Book03/P04: affine composition/inverse/normal contracts passed\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03/P04 FAILED: " << e.what() << '\n';
        return 1;
    }
}
