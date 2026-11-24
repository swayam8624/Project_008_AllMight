#include "book03/calibration.hpp"
#include <iostream>
#include <random>
using namespace book03::calibration;
void check(bool ok){if(!ok)throw std::runtime_error("calibration test");}
Camera truth(){double c=std::cos(.3),s=std::sin(.3);return {{{800*c-320*s,4,800*s+320*c,800*.1+4*(-.2)+320*4},{-240*s,820,240*c,820*(-.2)+240*4},{-s,0,c,4}}};}
int main(){const auto p=truth();std::vector<Match> obs;for(int x=-2;x<=2;++x)for(int y=-2;y<=2;++y)for(int z=0;z<3;++z){V3 w{.3*x,.3*y,.3*z};obs.push_back({w,project(p,w)});}auto f=dlt(obs);auto k=decompose(f.p);check(f.rms_pixels<1e-8);check(std::abs(k.fx-800)<1e-7&&std::abs(k.fy-820)<1e-7&&std::abs(k.cx-320)<1e-7&&std::abs(k.cy-240)<1e-7&&std::abs(k.skew-4)<1e-7);check(norm(sub(k.translation,{.1,-.2,4}))<1e-8);check(norm(sub(k.rotation[2],{-std::sin(.3),0,std::cos(.3)}))<1e-8);
    std::mt19937_64 rng(26);std::normal_distribution<double> noise(0,.2);double worst=0;for(int j=0;j<100;++j){auto noisy=obs;for(auto& o:noisy)for(double& v:o.pixel)v+=noise(rng);auto fit=dlt(noisy);auto a=project(fit.p,{.13,-.17,.42}),b=project(p,{.13,-.17,.42});worst=std::max(worst,std::hypot(a[0]-b[0],a[1]-b[1]));check(fit.rms_pixels<.5);}check(worst<.2);
    auto planar=obs;for(auto& o:planar){o.world[2]=0;o.pixel=project(p,o.world);}try{(void)dlt(planar);check(false);}catch(const std::domain_error&){}
    std::cout<<"P26 PASS exact RMS="<<f.rms_pixels<<" heldout worst="<<worst<<'\n';}
