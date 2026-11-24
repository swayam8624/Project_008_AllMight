#include "book03/distortion.hpp"
#include <iostream>
#include <random>
using namespace book03;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
int main(){try{
    BrownConrady m{-.18,.04,-.005,.004,-.003,1};
    check(m.derivative_bound()<.9,"certificate");
    auto p=m.forward({.5,0});check(std::abs(p.x-(.5*(1-.18*.25+.04*.0625-.005*.015625)-.00225))<1e-14,"radial tangential witness");
    std::mt19937 rng(230023);auto u=[&]{return double(rng())/4294967296.;};
    double worst=0;
    for(int i=0;i<4000;++i){
        double r=.98*std::sqrt(u()),a=2*std::numbers::pi*u();DistortionPoint x{r*std::cos(a),r*std::sin(a)};
        auto y=m.forward(x);auto inv=undistort(m,y);
        check(inv.status==InverseStatus::Converged,"inverse convergence");
        worst=std::max(worst,std::hypot(x.x-inv.point.x,x.y-inv.point.y));
        check(worst<5e-12,"round trip");
        constexpr double h=1e-6;auto j=m.jacobian(x);
        auto a1=m.forward({x.x+h,x.y}),a0=m.forward({x.x-h,x.y});
        auto b1=m.forward({x.x,x.y+h}),b0=m.forward({x.x,x.y-h});
        check(std::abs((a1.x-a0.x)/(2*h)-j.xx)<1e-8&&std::abs((a1.y-a0.y)/(2*h)-j.yx)<1e-8&&
              std::abs((b1.x-b0.x)/(2*h)-j.xy)<1e-8&&std::abs((b1.y-b0.y)/(2*h)-j.yy)<1e-8,"Jacobian oracle");
    }
    check(undistort(BrownConrady{-1,0,0,0,0,1},{.1,0}).status==InverseStatus::Uncertified,"fold rejection");
    check(undistort(m,{5,5}).status!=InverseStatus::Converged,"outside target");
    for(auto mode:{AngularProjection::Rectilinear,AngularProjection::Equidistant,AngularProjection::Equisolid,AngularProjection::Stereographic,AngularProjection::Orthographic})
        for(int i=0;i<=100;++i){double a=1.5*i/100;check(std::abs(angular_inverse(mode,angular_radius(mode,a))-a)<1e-14,"angular inverse");}
    std::cout<<"Book03/P23 distortion contracts: PASS; max inverse error="<<worst<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
