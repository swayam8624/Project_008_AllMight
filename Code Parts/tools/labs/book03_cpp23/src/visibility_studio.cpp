// P19 standalone visibility studio. Geometry is already in screen space.
// Color encodes the winning surface ID, not illumination.
#include "book03/visibility.hpp"
#include <fstream>
#include <iostream>
#include <string_view>
#include <iomanip>
namespace {
using namespace book03;
constexpr int W=32,H=24;
VisibilityBuffer scene(bool reverse,bool swapped){
    VisibilityBuffer b(W,H,{reverse?VisibilityOrder::Reverse:VisibilityOrder::Forward,
                            DepthStorage::Float32,TiePolicy::LowestID});
    auto q=[&](double d){return reverse?1-d:d;};
    std::array<DepthVertex,3> a{{{{0,0},q(.125)},{{32,0},q(.625)},{{0,24},q(.125)}}};
    std::array<DepthVertex,3> c{{{{0,0},q(.375)},{{32,0},q(.375)},{{0,24},q(.375)}}};
    if(swapped){b.draw(c,2);b.draw(a,1);}else{b.draw(a,1);b.draw(c,2);}
    return b;
}
void report(const VisibilityBuffer& b){
    int a=0,c=0,empty=0;
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){
        auto p=b.at(x,y);if(!p)++empty;else if(p->id==1)++a;else ++c;
    }
    std::cout<<"surface1="<<a<<" surface2="<<c<<" empty="<<empty<<"\n";
}
void csv(std::ostream& out,const VisibilityBuffer& b){
    out<<"x,y,occupied,id,depth\n"<<std::setprecision(17);
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){
        auto p=b.at(x,y);out<<x<<","<<y<<","<<(p?1:0)<<",";
        if(p)out<<p->id<<","<<p->depth;else out<<",";
        out<<"\n";
    }
}
void ppm(std::ostream& out,const VisibilityBuffer& b){
    out<<"P6\n"<<W<<" "<<H<<"\n255\n";
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){
        auto p=b.at(x,y);
        const std::array<unsigned char,3> color=!p?std::array<unsigned char,3>{242,240,231}:
            p->id==1?std::array<unsigned char,3>{49,133,107}:std::array<unsigned char,3>{186,122,72};
        out.write(reinterpret_cast<const char*>(color.data()),3);
    }
}
}
int main(int argc,char** argv){try{
    const auto cmd=argc>1?std::string_view(argv[1]):"";
    const int first=cmd=="inspect"?2:3;
    if((cmd!="inspect"&&cmd!="csv"&&cmd!="ppm")||argc<first)
        throw std::invalid_argument("book03_visibility inspect|csv FILE|ppm FILE [reverse] [swapped]");
    bool reverse=false,swapped=false;
    for(int i=first;i<argc;++i){
        const std::string_view flag=argv[i];
        if(flag=="reverse")reverse=true;else if(flag=="swapped")swapped=true;
        else throw std::invalid_argument("unknown flag");
    }
    auto b=scene(reverse,swapped);
    if(cmd=="inspect")report(b);
    else{
        std::ofstream out(argv[2],std::ios::binary);
        if(!out)throw std::runtime_error("output open failed");
        if(cmd=="csv")csv(out,b);else ppm(out,b);
        out.flush();if(!out)throw std::runtime_error("output write failed");
    }return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
