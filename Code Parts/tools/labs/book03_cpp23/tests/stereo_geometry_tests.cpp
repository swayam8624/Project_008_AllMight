// P15 independent stereo witnesses: use the known generating geometry,
// not a second implementation of the midpoint formula as the oracle.
#include "book03/stereo_geometry.hpp"
#include <iostream>
#include <random>
#include <string>
#include <string_view>
namespace {
using namespace book03;
void require(bool b,std::string_view m){if(!b)throw std::runtime_error(std::string(m));}
void near(double a,double b,double e=1e-9){
    require(std::isfinite(a)&&std::abs(a-b)<=e*(1+std::abs(b)),"stereo numeric witness");
}
void same(Point3 a,Point3 b,double e=1e-9){near(a.x,b.x,e);near(a.y,b.y,e);near(a.z,b.z,e);}
template<class E,class F>void reject(F f){try{f();}catch(const E&){return;}throw std::runtime_error("expected stereo rejection");}
void rectified(){
    RectifiedStereo s;
    const auto l=s.left_camera(),r=s.right_camera();
    const Point3 p{1,1,-4};
    const auto a=l.project(p).image,b=r.project(p).image;
    near(a.u,420);near(b.u,400);near(a.v,140);near(b.v,140);
    near(s.disparity(a,b),20);near(s.axial_depth(20),4);
    near(s.depth_sigma(20,.25),.05);
    same(s.reconstruct(a,b),p);
    const auto t=triangulate_samples(l,r,a,b);
    require(t.geometry.kind==TriangulationKind::Estimate&&t.geometry.point.has_value(),"exact pair");
    same(*t.geometry.point,p);
    near(t.geometry.ray_gap,0);near(*t.left_pixel_error,0);near(*t.right_pixel_error,0);
    near(epipolar_coplanarity(l,r,a,b),0);
    // Change both principal points; raw disparity is no longer the metric disparity.
    s.right.cx=300;s.right.cy=230;
    const auto q=s.right_camera().project(p).image;
    near(q.u,380);near(q.v,130);near(s.disparity(a,q),20);
    near(s.row_residual(a,q),0);same(s.reconstruct(a,q),p);
    reject<std::domain_error>([&]{(void)s.reconstruct(a,{q.u,q.v+2});});
    reject<std::domain_error>([&]{(void)s.axial_depth(0);});
    reject<std::domain_error>([&]{(void)s.axial_depth(-2);});
    s.right.fx=401;
    reject<std::invalid_argument>([&]{s.validate();});
}
void adversaries(){
    const auto l=Ray3::from({0,0,0},{0,0,-1});
    auto parallel=Ray3::from({1,0,0},{0,0,-1});
    require(triangulate_rays(l,parallel).kind==TriangulationKind::WeakParallax,"parallel");
    auto opposite=Ray3::from({1,0,0},{0,0,1});
    require(triangulate_rays(l,opposite).kind==TriangulationKind::WeakParallax,"antiparallel");
    auto diverging=Ray3::from({1,0,0},{1,0,-1});
    require(triangulate_rays(l,diverging).kind==TriangulationKind::BehindCamera,"behind");
    auto weak=Ray3::from({1,0,0},{-1e-9,0,-1});
    require(triangulate_rays(l,weak).kind==TriangulationKind::WeakParallax,"weak angle");
    // Skew lines: left closest point (0,0,-4), right (0,1,-4).
    auto skew=Ray3::from({1,1,0},{-1,0,-4});
    const auto t=triangulate_rays(l,skew);
    require(t.point.has_value(),"skew estimate");
    same(*t.point,{0,.5,-4});near(t.ray_gap,1);
    near(t.left_range,4);near(t.right_range,std::sqrt(17.));
    // A wrong horizontal match can satisfy the epipolar constraint exactly.
    RectifiedStereo s;auto a=s.left_camera().project({1,1,-4}).image;
    auto b=s.right_camera().project({1,1,-4}).image;b.u-=20;
    near(epipolar_coplanarity(s.left_camera(),s.right_camera(),a,b),0);
    same(s.reconstruct(a,b),{.5,.5,-2});
}
void arbitrary_poses(){
    PinholeCamera l{{640,480,400,380,310,235,17},
                    {Rotation3::axis_angle({0,1,0},.08),{-.2,.1,.05}}};
    PinholeCamera r{{800,600,510,470,405,290,-8},
                    {Rotation3::axis_angle({1,1,0},-.13),{.4,-.15,.1}}};
    std::mt19937_64 rng(150015);
    auto sample=[&]{return static_cast<double>(rng()>>11)*0x1.0p-53;};
    for(int i=0;i<3000;++i){
        Point3 p{2*sample()-1,2*sample()-1,-(2+18*sample())};
        const auto a=l.project(p).image,b=r.project(p).image;
        near(epipolar_coplanarity(l,r,a,b),0,1e-10);
        const auto t=triangulate_samples(l,r,a,b);
        require(t.geometry.point.has_value(),"general pose reconstruction");
        same(*t.geometry.point,p,2e-9);
        near(t.geometry.ray_gap,0,1e-9);
        near(*t.left_pixel_error,0,1e-7);near(*t.right_pixel_error,0,1e-7);
    }
}
void uncertainty(){
    RectifiedStereo s;
    near(s.depth_sigma(10,.25),.2);
    near(s.depth_sigma(5,.25),.8);
    const double z=s.axial_depth(20),lo=s.axial_depth(20.25),hi=s.axial_depth(19.75);
    require(hi-z>z-lo,"reciprocal depth asymmetric interval");
    near((hi-lo)/2,s.depth_sigma(20,.25),2e-5);
}
}
int main(){try{rectified();adversaries();arbitrary_poses();uncertainty();
    std::cout<<"Book03/P15 calibrated stereo contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
