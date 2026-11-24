// Controlled pre-clipped perspective witness: a tiled quadrilateral behind
// an opaque triangle. No claim of OBJ/model/clip adapter execution.
#include "book03/software_renderer.hpp"
#include <fstream>
#include <iostream>
using namespace book03;
int main(int argc,char** argv){try{
    if(argc!=2)throw std::invalid_argument("book03_renderer_gallery OUTPUT.ppm");
    SoftwareRenderer r(320,240,{VisibilityOrder::Forward,DepthStorage::Float64,TiePolicy::LowestID});
    // Screen coordinates and q=20/19*(1-1/w) for n=1,f=20.
    auto v=[](double x,double y,double w,double u,double t){return RenderVertex{{x,y},20./19*(1-1/w),w,u,t};};
    auto a=v(25,210,3,0,0),b=v(290,210,3,1,0),c=v(220,55,9,1,1),d=v(100,55,9,0,1);
    r.draw({a,b,c},1,{.26,.62,.48});r.draw({a,c,d},2,{.26,.62,.48});
    r.draw({v(104,175,2,0,0),v(256,175,2,1,0),v(184,28,2,.5,1)},3,{.85,.34,.12},false);
    std::ofstream out(argv[1],std::ios::binary);if(!out)throw std::runtime_error("open failed");r.ppm(out);out.flush();if(!out)throw std::runtime_error("write failed");
    auto s=r.stats();std::cout<<"triangles="<<s.triangles<<" candidates="<<s.candidates<<" covered="<<s.covered<<" passed="<<s.passed<<" failed="<<s.failed<<" ties="<<s.ties<<"\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
