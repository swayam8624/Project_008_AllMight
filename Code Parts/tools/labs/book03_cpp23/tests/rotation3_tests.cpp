// P12 independent rotation witnesses and seeded invariants.
#include "book03/rotation3.hpp"
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <string_view>
namespace {
using namespace book03;
constexpr double pi=std::numbers::pi;
void require(bool b,std::string_view m){if(!b)throw std::runtime_error(std::string(m));}
void near(double a,double b,double e=1e-10){require(std::isfinite(a)&&std::abs(a-b)<=e*(1+std::abs(b)),"numeric witness");}
void same(Vector3 a,Vector3 b,double e=1e-10){near(a.x,b.x,e);near(a.y,b.y,e);near(a.z,b.z,e);}
void same_map(Rotation3 a,Rotation3 b,double e=1e-10){
    for(Vector3 v: {Vector3{1,0,0},Vector3{0,1,0},Vector3{0,0,1}})
        same(a.apply(v),b.apply(v),e);
}
template<class F>void invalid(F f){try{f();}catch(const std::invalid_argument&){return;}throw std::runtime_error("expected rejection");}
void witnesses(){
    const auto x=Rotation3::axis_angle({1,0,0},pi/2);
    const auto y=Rotation3::axis_angle({0,1,0},pi/2);
    const auto z=Rotation3::axis_angle({0,0,1},pi/2);
    same(x.apply({0,1,0}),{0,0,1});
    same(y.apply({0,0,1}),{1,0,0});
    same(z.apply({1,0,0}),{0,1,0});
    same((y*x).apply({0,1,0}),{1,0,0});
    same((x*y).apply({0,1,0}),{0,0,1});
    same_map(Rotation3::fixed_xyz(pi/2,pi/2,0),y*x);
    same(Rotation3::axis_angle({1,1,1},2*pi/3).apply({1,0,0}),{0,1,0});
    const auto q=z.components();
    same_map(z,Rotation3::normalized({-q.w,-q.x,-q.y,-q.z}));
    same_map(z,Rotation3::normalized({1e300*q.w,1e300*q.x,1e300*q.y,1e300*q.z}));
    same_map(z,Rotation3::normalized({1e-300*q.w,1e-300*q.x,1e-300*q.y,1e-300*q.z}));
    invalid([]{(void)Rotation3::normalized({0,0,0,0});});
    invalid([]{(void)Rotation3::axis_angle({0,0,0},0);});
    invalid([]{(void)Rotation3::normalized({1,0,0,std::numeric_limits<double>::infinity()});});
}
void interpolation(){
    const Rotation3 a{};
    const auto b=Rotation3::axis_angle({0,0,1},2*pi/3);
    for(int i=0;i<=12;++i){
        const double t=i/12.0,angle=t*2*pi/3;
        same(Rotation3::slerp(a,b,t).apply({1,0,0}),{std::cos(angle),std::sin(angle),0});
    }
    const auto q=b.components();
    const auto minus=Rotation3::normalized({-q.w,-q.x,-q.y,-q.z});
    for(double t:{0.,0.25,0.5,0.75,1.})same_map(b,Rotation3::slerp(b,minus,t));
    const auto long_arc=Rotation3::axis_angle({0,0,1},3*pi/2);
    same(Rotation3::slerp(a,long_arc,0.5).apply({1,0,0}),{std::sqrt(0.5),-std::sqrt(0.5),0});
    const auto close=Rotation3::axis_angle({0,0,1},1e-5);
    same(Rotation3::slerp(a,close,0.5).apply({1,0,0}),{std::cos(5e-6),std::sin(5e-6),0});
    invalid([&]{(void)Rotation3::slerp(a,b,-0.1);});
}
void euler_and_pose(){
    for(double sign:{-1.,1.}){
        const auto q=Rotation3::fixed_xyz(0.8,sign*pi/2,-0.3);
        const auto e=principal_fixed_xyz(q);
        require(e.near_lock,"gimbal lock reported");
        same_map(q,Rotation3::fixed_xyz(e.x,e.y,e.z));
        same_map(q,Rotation3::fixed_xyz(0.8+sign*0.4,sign*pi/2,-0.3+0.4));
    }
    const auto q=Rotation3::fixed_xyz(0.3,0.6,-1.1);
    const auto e=principal_fixed_xyz(q);
    require(!e.near_lock,"ordinary Euler pose");
    near(e.x,0.3);near(e.y,0.6);near(e.z,-1.1);
    const RigidPose a{Rotation3::axis_angle({0,0,1},pi/2),{3,0,0}};
    const RigidPose b{Rotation3{}, {2,0,0}};
    const auto p=(a*b).point({1,0,0});
    same({p.x,p.y,p.z},{3,3,0});
    const auto local=(a*b).inverse().point(p);
    same({local.x,local.y,local.z},{1,0,0});
    const auto mid=interpolate_pose({},a,0.5);
    same(mid.translation,{1.5,0,0});
    same(mid.rotation.apply({1,0,0}),{std::sqrt(0.5),std::sqrt(0.5),0});
    const auto w=(a*b).affine().point({1,0,0});
    same({w.x,w.y,w.z},{3,3,0});
}
void seeded_invariants(){
    std::mt19937_64 engine(120312);
    // Explicit conversion from the top 53 random bits, rather than relying
    // on implementation-dependent uniform_real_distribution algorithms.
    auto sample=[&]{return 2.0*static_cast<double>(engine()>>11)*0x1.0p-53-1.0;};
    for(int i=0;i<3000;++i){
        const Vector3 axis{sample(),sample(),sample()},v{sample(),sample(),sample()},u{sample(),sample(),sample()};
        const double angle=pi*sample();
        const auto q=Rotation3::axis_angle(axis,angle);
        const auto M=q.matrix();
        same(q.apply(v),rodrigues(axis,angle,v));
        near(magnitude(q.apply(v)),magnitude(v));
        near(dot(q.apply(v),q.apply(u)),dot(v,u));
        near(M.determinant(),1);
        same(M.transpose().apply(M.apply(v)),v);
        same(q.inverse().apply(q.apply(v)),v);
        const auto e=principal_fixed_xyz(q);
        same_map(q,Rotation3::fixed_xyz(e.x,e.y,e.z),2e-8);
    }
    auto q=Rotation3{};
    const auto step=Rotation3::axis_angle({1,2,3},0.001);
    for(int i=0;i<10000;++i)q=step*q;
    same_map(q,Rotation3::axis_angle({1,2,3},10),1e-9);
}
}
int main(){try{witnesses();interpolation();euler_and_pose();seeded_invariants();
    std::cout<<"Book03/P12 rotation and rigid pose contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
