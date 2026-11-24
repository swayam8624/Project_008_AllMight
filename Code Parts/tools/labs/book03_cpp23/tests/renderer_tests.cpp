#include "book03/software_renderer.hpp"
#include <iostream>
#include <sstream>
using namespace book03;
void check(bool x,const char* why){if(!x)throw std::runtime_error(why);}
using T=std::array<RenderVertex,3>;
T tri(double q){return {{{{0,0},q,2,0,0},{{8,0},q,4,1,0},{{0,8},q,8,0,1}}};}
int main(){try{
    for(auto order:{VisibilityOrder::Forward,VisibilityOrder::Reverse}){
        auto q=[&](double v){return order==VisibilityOrder::Forward?v:1-v;};
        SoftwareRenderer r(8,8,{order,DepthStorage::Float64,TiePolicy::LowestID});
        std::string first;
        for(bool swap:{false,true}){
            r.clear();auto a=tri(q(.25)),b=tri(q(.75));
            if(swap){r.draw(a,1,{1,.5,0});r.draw(b,2,{0,0,1},false);}else{r.draw(b,2,{0,0,1},false);r.draw(a,1,{1,.5,0});}
            int hits=0;
            for(int y=0;y<8;++y)for(int x=0;x<8;++x){
                const auto p=r.winner(x,y);if(x+y>=7){check(!p,"outside");continue;}
                ++hits;check(p&&p->id==1&&p->depth==q(.25),"opaque winner");
                const double b=(x+.5)/8,c=(y+.5)/8,a=1-b-c,den=a/2+b/4+c/8;
                const double u=(b/4)/den,v=(c/8)/den;
                const double g=(static_cast<int>(std::floor(8*u)+std::floor(8*v))%2)?.32:1.;
                auto actual=r.color(x,y);check(actual.r==g&&actual.g==.5*g&&actual.b==0,"UV shader oracle");
            }
            check(hits==28&&r.stats().covered==56&&r.stats().passed+r.stats().failed==56,"work accounting");
            std::ostringstream out;r.ppm(out);if(swap)check(out.str()==first,"draw order image");else first=out.str();
        }
    }
    SoftwareRenderer r(8,8);bool rejected=false;auto t=tri(.5);t[0].w=0;
    try{r.draw(t,1,{1,1,1});}catch(const std::invalid_argument&){rejected=true;}
    check(rejected&&!r.winner(0,0),"invalid vertex before draw");
    std::cout<<"Book03/P21 backend contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
