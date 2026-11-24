// P13 independent camera witnesses; all image coordinates use the
// continuous top-left edge convention, and axial depth is optical Z.
#include "book03/pinhole_camera.hpp"
#include <iostream>
#include <random>
#include <limits>
#include <string>
#include <string_view>
namespace {
using namespace book03;
void require(bool b,std::string_view m){if(!b)throw std::runtime_error(std::string(m));}
void near(double a,double b,double e=1e-10){require(std::isfinite(a)&&std::abs(a-b)<=e*(1+std::abs(b)),"camera numeric witness");}
void same(Point3 a,Point3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
template<class E,class F>void reject(F f){try{f();}catch(const E&){return;}throw std::runtime_error("expected rejection");}
void perspective(){
    PinholeCamera c;
    auto p=c.project({1,1,-4});
    near(p.image.u,420);near(p.image.v,140);near(p.axial_depth,4);
    require(p.inside_image,"front visible sample");
    auto q=c.project({2,2,-8});
    near(q.image.u,p.image.u);near(q.image.v,p.image.v);
    near(c.project({1,1,-8}).image.u,370);
    same(c.unproject_axial(p.image,4),{1,1,-4});
    auto r=c.ray(p.image);same(r.origin,{0,0,0});
    near(r.direction.x,1/std::sqrt(18.0));near(r.direction.y,1/std::sqrt(18.0));
    near(r.direction.z,-4/std::sqrt(18.0));
    auto a=c.pixel_ray(0,0);auto b=c.ray({0.5,0.5});
    near(dot(a.direction,b.direction),1);
    require(c.project({-3.2,0,-4}).inside_image,"left edge included");
    require(!c.project({3.2,0,-4}).inside_image,"right edge excluded");
    require(!c.project({10,0,-4}).inside_image,"outside but projectable");
}
void extrinsics_and_skew(){
    PinholeCamera c{{640,480,400,300,310,230,20},
        {Rotation3::axis_angle({0,1,0},std::numbers::pi/2),{3,2,1}}};
    // Camera looks toward world -X. Local (1,-2,-4) maps to (-1,0,0).
    const Point3 world{-1,0,0};
    same(c.optical(world),{1,2,4});
    auto p=c.project(world);
    near(p.image.u,420);near(p.image.v,380);
    same(c.unproject_axial(p.image,4),world);
    auto ray=c.ray(p.image);
    same(ray.origin,{3,2,1});
    const auto expected=unit(world-ray.origin);
    near(dot(ray.direction,expected),1);
}
void calibration_changes(){
    PinholeCamera c{{800,600,500,450,410,290,12},{}};
    const Point3 p{1,2,-5};auto a=c.project(p).image;
    auto cropped=c;cropped.intrinsics=c.intrinsics.crop(100,80,500,400);
    auto b=cropped.project(p).image;
    near(b.u,a.u-100);near(b.v,a.v-80);
    auto resized=cropped;resized.intrinsics=cropped.intrinsics.resize(250,600);
    auto d=resized.project(p).image;
    near(d.u,b.u*0.5);near(d.v,b.v*1.5);
    same(resized.unproject_axial(d,5),p);
    auto k=CameraIntrinsics::centered_fov(800,600,std::numbers::pi/2);
    near(k.fx,300);near(k.fy,300);near(k.cx,400);near(k.cy,300);
}
void derivatives_and_roundtrips(){
    PinholeCamera c{{640,480,400,380,310,235,17},{}};
    std::mt19937_64 gen(130013);
    auto sample=[&]{return static_cast<double>(gen()>>11)*0x1.0p-53;};
    for(int i=0;i<3000;++i){
        Point3 p{4*sample()-2,4*sample()-2,1+9*sample()};
        auto project=[&](Point3 q){return c.project({q.x,-q.y,-q.z}).image;};
        auto m=project(p);same(c.unproject_axial(m,p.z),{p.x,-p.y,-p.z});
        const auto j=c.optical_jacobian(p);
        constexpr double h=1e-5;
        for(int k=0;k<3;++k){
            auto a=p,b=p;
            if(k==0){a.x+=h;b.x-=h;}
            if(k==1){a.y+=h;b.y-=h;}
            if(k==2){a.z+=h;b.z-=h;}
            const auto u=project(a),v=project(b);
            const double du=k==0?j[0].x:k==1?j[0].y:j[0].z;
            const double dv=k==0?j[1].x:k==1?j[1].y:j[1].z;
            near((u.u-v.u)/(2*h),du,2e-7);
            near((u.v-v.v)/(2*h),dv,2e-7);
        }
        near(dot(j[0],{p.x,p.y,p.z}),0,1e-8);
        near(dot(j[1],{p.x,p.y,p.z}),0,1e-8);
    }
}
void bad_inputs(){
    PinholeCamera c;
    reject<std::domain_error>([&]{(void)c.project({1,1,0});});
    reject<std::domain_error>([&]{(void)c.project({0,0,1});});
    reject<std::invalid_argument>([&]{(void)c.unproject_axial({0,0},0);});
    reject<std::out_of_range>([&]{(void)c.pixel_ray(640,0);});
    reject<std::out_of_range>([&]{(void)c.intrinsics.crop(639,0,2,1);});
    reject<std::invalid_argument>([&]{(void)CameraIntrinsics::centered_fov(640,480,0);});
    c.intrinsics.fx=0;
    reject<std::invalid_argument>([&]{(void)c.project({0,0,-1});});
    c.intrinsics.fx=400;c.intrinsics.cy=std::numeric_limits<double>::quiet_NaN();
    reject<std::invalid_argument>([&]{(void)c.ray({0,0});});
}
}
int main(){try{perspective();extrinsics_and_skew();calibration_changes();
    derivatives_and_roundtrips();bad_inputs();
    std::cout<<"Book03/P13 calibrated pinhole contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
