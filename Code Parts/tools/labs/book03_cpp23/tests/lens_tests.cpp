#include "book03/thin_lens.hpp"
#include <iostream>
#include <random>
using namespace book03;
void near(double a,double b,double e=1e-10){if(!std::isfinite(a)||std::abs(a-b)>e*(1+std::abs(b)))throw std::runtime_error("lens witness");}
int main(){try{
    ThinLens l{50,8,3000};near(l.image_distance(150),75);near(l.blur_mm(3000),0);
    near(l.hyperfocal_mm(.03),10466.6666666667);near(l.airy_infinity_mm(.00055),.010736);
    auto limits=l.acceptable(.03);near(l.blur_mm(limits.near_mm),.03);near(l.blur_mm(*limits.far_mm),.03);
    ThinLens hyper{50,8,l.hyperfocal_mm(.03)};near(hyper.infinity_blur_mm(),.03);
    ThinLens beyond{50,8,12000};if(beyond.acceptable(.03).far_mm)throw std::runtime_error("infinity boundary");
    std::mt19937 rng(220022);
    for(int i=0;i<3000;++i){
        ThinLens a{double(20+rng()%101),double(2+rng()%15),double(1000+rng()%9000)};
        double z=500+rng()%20000,r=a.aperture_mm()/2;
        auto p=a.sensor_hit({12,-7},z,{r,0}),q=a.sensor_hit({12,-7},z,{-r,0});
        near(std::hypot(p.x-q.x,p.y-q.y),a.blur_mm(z));
        near((p.x+q.x)/2,-a.sensor_mm()*12/z);
        near(p.y,a.sensor_mm()*7/z);
        auto f=a.sensor_hit({12,-7},a.focus_mm,{0,r});
        near(f.x,-a.sensor_mm()*12/a.focus_mm);near(f.y,a.sensor_mm()*7/a.focus_mm);
    }
    bool rejected=false;try{(void)l.image_distance(50);}catch(const std::invalid_argument&){rejected=true;}
    if(!rejected)throw std::runtime_error("invalid object distance");
    std::cout<<"Book03/P22 lens contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
