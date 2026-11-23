// Book 03 / P06 — independent geometry-3D witnesses.
//
// These tests exercise hand-derived vectors, the exact P03 bounded 2D oracle
// embedded into XY, ray/plane substitution, sphere quadratic roots, negative
// direction/normal and invalid-input cases. The 3D floating numerics remain
// *approximate*; the older exact 2D oracle is not silently called a 3D proof.

#include "book03/geometry3d.hpp"
#include "book03/triangle_geometry.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace {

using book03::Point3;
using book03::Vector3;
using book03::Triangle3;
using book03::Plane3;
using book03::Ray3;
using book03::Sphere3;
using book03::RayPlaneKind;
using book03::RaySphereKind;

void require(bool condition,std::string_view description) {
    if(!condition) throw std::runtime_error(std::string(description));
}
void near(double a,double b,double tolerance,std::string_view why) {
    if(!std::isfinite(a)||!std::isfinite(b) ||
       std::abs(a-b)>tolerance*(1.0+std::abs(b)))
        throw std::runtime_error(std::string(why));
}
void near_point(Point3 a,Point3 b,double tolerance,std::string_view why) {
    near(a.x,b.x,tolerance,why);
    near(a.y,b.y,tolerance,why);
    near(a.z,b.z,tolerance,why);
}
void near_vec(Vector3 a,Vector3 b,double tolerance,std::string_view why) {
    near(a.x,b.x,tolerance,why);
    near(a.y,b.y,tolerance,why);
    near(a.z,b.z,tolerance,why);
}
template<class E,class F> void must_throw(F&& f,std::string_view why) {
    try { f(); } catch(const E&) { return; }
    throw std::runtime_error(std::string(why));
}

// Compile-time distinction: an address is not a displacement, and two
// addresses cannot be added arbitrarily.
template<class A,class B> concept CanAdd =
    requires(A a,B b){a+b;};
static_assert(!CanAdd<Point3,Point3>);
static_assert(CanAdd<Point3,Vector3>);
static_assert(CanAdd<Vector3,Vector3>);
static_assert(std::is_same_v<decltype(Point3{}-Point3{}),Vector3>);

void point_vector_and_dot() {
    const Point3 a{1,1,2},b{4,5,2};
    const Vector3 displacement=b-a;
    require(displacement==Vector3{3,4,0},"point difference is vector");
    require(a+displacement==b,"point plus displacement restores address");
    near(book03::magnitude(displacement),5.0,0,"3-4-5 triangle length");
    near(book03::magnitude(Vector3{3,4,12}),13.0,0,"3-4-12 length");
    near(book03::dot({1,0,0},{2,3,0}),2,0,"unit x projection");
    near(book03::dot({1,2,3},{4,-1,2}),8,0,"independent dot algebra");
    near_vec(book03::unit({0,0,-13}),{0,0,-1},0,"normalization");
    near(book03::magnitude(book03::unit({3,4,12})),1.0,1e-15,
         "normalized arbitrary vector length");
    const double den=std::numeric_limits<double>::denorm_min();
    near_vec(book03::unit({den,0,0}),{1,0,0},0,
             "std::hypot handles tiny representable direction");
}

void cross_and_embedded_old_triangle() {
    const Vector3 x{1,0,0},y{0,1,0},z{0,0,1};
    require(book03::cross(x,y)==z,"right-handed x cross y = z");
    require(book03::cross(y,x)==-z,"cross product reverses sign");
    near(book03::dot(book03::cross({2,3,5},{7,11,13}),
                     {2,3,5}),0,0,"cross perpendicular to first");
    near(book03::dot(book03::cross({2,3,5},{7,11,13}),
                     {7,11,13}),0,0,"cross perpendicular to second");

    const Triangle3 floor{{0,0,0},{2,0,0},{0,0,3}};
    require(floor.doubled_area_vector()==Vector3{0,-6,0},
            "XZ floor winding points down");
    near(floor.area(),3.0,0,"floor triangle area three square metres");
    near_vec(floor.face_normal(),{0,-1,0},0,"floor downward normal");
    const Triangle3 reversed{floor.a,floor.c,floor.b};
    near_vec(reversed.face_normal(),{0,1,0},0,
             "winding reversal negates normal");
    near(reversed.area(),floor.area(),0,"area independent of winding");

    const Triangle3 xy{{0,0,0},{6,0,0},{0,4,0}};
    const auto normal=xy.doubled_area_vector();
    const auto two_d=book03::exact_twice_area({0,0},{6,0},{0,4});
    near(normal.z,static_cast<double>(two_d),0,
         "independent exact P03 signed twice-area oracle");
    near(xy.area(),12,0,"same planar area after 3D embedding");

    const Triangle3 collapsed{{0,0,0},{1,2,3},{2,4,6}};
    near(collapsed.area(),0,0,"collinear triangle area zero");
    must_throw<std::invalid_argument>(
        [&]{(void)collapsed.face_normal();},
        "collapsed triangle has no unique face normal");
}

void normalized_planes_and_ray_cases() {
    const auto floor=Plane3::through({0,0,0},{0,10,0});
    near_vec(floor.normal,{0,1,0},0,
             "factory removes arbitrary tenfold normal magnitude");
    near(floor.signed_distance({1,1,2}),1.0,0,
         "signed distance one metre above floor");
    near(floor.signed_distance({1,-3,2}),-3.0,0,
         "negative below floor");
    const auto vertical=Plane3::through({3,0,0},{1,0,0});
    near(vertical.signed_distance({4,2,1}),1.0,0,
         "X=3 wall distance");

    const auto ray=Ray3::from({1,3,2},{0,-4,0});
    near_vec(ray.direction,{0,-1,0},0,
             "ray from factory has unit direction");
    const auto hit=book03::intersect(ray,floor);
    require(hit.kind==RayPlaneKind::Hit && hit.point.has_value(),
            "downward ray hits floor");
    near(hit.metres,3.0,0,"ray t explicitly means metres");
    near_point(*hit.point,{1,0,2},0,"analytic ray-plane intersection");
    near(floor.signed_distance(*hit.point),0,0,
         "independent substituted plane residual");
    near_point(ray.at(hit.metres),*hit.point,0,
               "reported hit agrees with ray parameterization");

    const auto up=book03::intersect(
        Ray3::from({0,3,0},{0,1,0}),floor);
    require(up.kind==RayPlaneKind::Behind && !up.point.has_value(),
            "line meets floor behind upward ray");

    const auto parallel=book03::intersect(
        Ray3::from({0,3,0},{1,0,0}),floor);
    require(parallel.kind==RayPlaneKind::Parallel &&
            !parallel.point.has_value(),"horizontal ray is parallel");

    const auto coplanar=book03::intersect(
        Ray3::from({0,0,0},{1,0,0}),floor);
    require(coplanar.kind==RayPlaneKind::CoplanarWithinPolicy,
            "ray in floor plane is not a unique intersection point");

    const auto uncertain=book03::intersect(
        Ray3::from({0,3,0},{1,-1e-16,0}),floor);
    require(uncertain.kind==RayPlaneKind::NearParallelUncertain &&
            !uncertain.point.has_value(),
            "almost parallel ray must not invent certified distant hit");
}

void sphere_roots_and_ray_domain() {
    const auto sphere=Sphere3::from({0,1,0},1.0);
    const auto front=book03::intersect(
        Ray3::from({0,1,5},{0,0,-12}),sphere);
    require(front.kind==RaySphereKind::Hit && front.point.has_value(),
            "sphere first forward hit");
    near(front.metres,4.0,0,"front sphere intersection four metres");
    near(front.other_line_metres,6.0,0,"back sphere intersection six metres");
    near_point(*front.point,{0,1,1},0,"sphere front point");
    near(book03::magnitude(*front.point-sphere.center),
         sphere.radius,0,"sphere surface equation independently checked");

    const auto inside=book03::intersect(
        Ray3::from({0,1,0},{1,0,0}),sphere);
    require(inside.kind==RaySphereKind::Hit,"ray can start inside ball");
    near(inside.metres,1,0,"inside ray first forward exit");
    near(inside.other_line_metres,-1,0,
         "entry root behind a ray starting inside sphere");

    const auto away=book03::intersect(
        Ray3::from({0,1,5},{0,0,1}),sphere);
    require(away.kind==RaySphereKind::Behind &&
            !away.point.has_value(),"sphere is behind ray");

    const auto miss=book03::intersect(
        Ray3::from({3,1,5},{0,0,-1}),sphere);
    require(miss.kind==RaySphereKind::Miss,
            "ray two metres outside sphere misses");

    const auto tangent=book03::intersect(
        Ray3::from({1,1,5},{0,0,-1}),sphere);
    require(tangent.kind==RaySphereKind::TangentWithinPolicy,
            "exact simple tangent under declared policy");
    near(tangent.metres,5,0,"tangent distance five metres");

    const auto close=book03::intersect(
        Ray3::from({1+1e-13,1,5},{0,0,-1}),sphere);
    require(close.kind==RaySphereKind::NearTangentUncertain,
            "almost tangent does not receive a certified sign");
}

void malformed_and_overflow() {
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    const double huge=std::numeric_limits<double>::max();

    must_throw<std::invalid_argument>(
        []{(void)book03::unit({0,0,0});},"zero vector cannot normalize");
    must_throw<std::invalid_argument>(
        [&]{(void)book03::dot({nan,0,0},{1,2,3});},
        "dot rejects nonfinite inputs");
    must_throw<std::invalid_argument>(
        [&]{(void)Plane3::through({0,0,0},{0,0,0});},
        "zero plane normal rejected");
    must_throw<std::invalid_argument>(
        [&]{(void)Plane3::through({inf,0,0},{1,0,0});},
        "plane rejects nonfinite point");
    must_throw<std::invalid_argument>(
        [&]{(void)Ray3::from({0,0,0},{0,0,0});},
        "ray requires a nonzero direction");
    must_throw<std::invalid_argument>(
        [&]{(void)Sphere3::from({0,0,0},-1);},
        "sphere radius must be positive");
    must_throw<std::invalid_argument>(
        [&]{(void)Sphere3::from({nan,0,0},1);},
        "sphere center finite");
    must_throw<std::invalid_argument>(
        [&]{(void)Ray3::from({0,0,0},{0,1,0}).at(-1);},
        "forward ray does not accept negative t");
    must_throw<std::invalid_argument>(
        [&]{(void)Ray3::from({0,0,0},{0,1,0}).at(inf);},
        "infinite travelled distance forbidden");
    must_throw<std::overflow_error>(
        [&]{(void)book03::cross({huge,huge,0},{0,huge,0});},
        "cross product overflow reported");
    must_throw<std::overflow_error>(
        [&]{(void)book03::magnitude({huge,huge,huge});},
        "unrepresentable vector magnitude rejected");
    must_throw<std::overflow_error>(
        [&]{(void)book03::dot({huge,0,0},{huge,0,0});},
        "nonfinite dot result rejected");

    // Raw direct construction does not bypass the normalized ray contract.
    const Ray3 bad_ray{{0,0,0},{0,7,0}};
    must_throw<std::invalid_argument>(
        [&]{(void)book03::intersect(
            bad_ray,Plane3::through({0,0,0},{0,1,0}));},
        "direct Ray3 data still checked by intersection");
}

void seeded_vector_orthogonality() {
    std::uint32_t rng=0x93B2A261u;
    auto next=[&]() -> double {
        rng^=rng<<13;
        rng^=rng>>17;
        rng^=rng<<5;
        return (static_cast<double>(rng%20001u)-10000.0)/1700.0;
    };
    for(int i=0;i<2500;++i) {
        const Vector3 a{next(),next(),next()};
        const Vector3 b{next(),next(),next()};
        const Vector3 n=book03::cross(a,b);
        near(book03::dot(a,n),0,2e-12,
             "seeded cross perpendicular to first vector");
        near(book03::dot(b,n),0,2e-12,
             "seeded cross perpendicular to second vector");
        near_vec(book03::cross(b,a),-n,1e-12,
                 "seeded cross antisymmetry");
        const Vector3 c{next(),next(),next()};
        near(book03::dot(a+b,c),
             book03::dot(a,c)+book03::dot(b,c),2e-12,
             "seeded dot distributivity");
    }
}

} // namespace

int main() {
    try {
        point_vector_and_dot();
        cross_and_embedded_old_triangle();
        normalized_planes_and_ray_cases();
        sphere_roots_and_ray_domain();
        malformed_and_overflow();
        seeded_vector_orthogonality();
        std::cout << "Book03/P06: 3D planes, normals, rays, spheres and 2500 seeded checks passed\n";
        return 0;
    } catch(const std::exception& err) {
        std::cerr << "Book03/P06 FAILED: "<<err.what()<<"\n";
        return 1;
    }
}
