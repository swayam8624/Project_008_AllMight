// Coverage-only explorer. P17 supplies clipped triangles; P18 decides samples.
// P19 will choose visible surfaces. A hit count is not a depth buffer.
#include "book03/primitive_clipping.hpp"
#include "book03/raster_coverage.hpp"
#include "book03/pixel_canvas.hpp"
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
namespace {
using namespace book03;
constexpr int W=32,H=24;
struct Result{std::vector<int> hits=std::vector<int>(W*H);std::uint64_t candidates{},covered{};};
void draw(Result& r,std::array<RasterPoint,3> t){
    auto s=rasterize_triangle(t,W,H,[&](CoverageSample p){++r.hits[p.y*W+p.x];});
    r.candidates+=s.candidates;r.covered+=s.covered;
}
Result run(bool clipped){
    Result r;
    if(!clipped){
        const RasterPoint a{4.5,3.5},b{27.5,3.5},c{27.5,20.5},d{4.5,20.5};
        draw(r,{a,b,c});draw(r,{a,c,d});
    }else{
        std::array<ProcessedVertex,3> p{};
        p[0].clip={-2,0,.5,1};p[1].clip={0,-.5,.5,1};p[2].clip={0,.5,.5,1};
        for(auto& t:fan_triangulate(clip_triangle(p))){
            std::array<RasterPoint,3> v{};
            for(int i=0;i<3;++i){auto s=to_screen_vertex(t[i],{0,0,W,H}).screen->image;v[i]={s.u,s.v};}
            draw(r,v);
        }
    }
    return r;
}
void report(const Result& r){
    int single=0,multiple=0;
    for(int n:r.hits){single+=n==1;multiple+=n>1;}
    std::cout<<"candidates="<<r.candidates<<" hits="<<r.covered<<" single="<<single<<" multiple="<<multiple<<"\n";
}
void ppm(std::ostream& out,const Result& r){
    Canvas c(W,H);
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){
        int n=r.hits[y*W+x];
        c.set({x,y},
            n>1?Rgb8{186,79,62}:n==1?Rgb8{49,133,107}:Rgb8{242,240,231});
    }
    c.write_ppm(out);
}
void csv(std::ostream& out,const Result& r){
    out<<"x,y,hits\n";
    for(int y=0;y<H;++y)for(int x=0;x<W;++x)out<<x<<","<<y<<","<<r.hits[y*W+x]<<"\n";
}
}
int main(int argc,char** argv){try{
    const auto cmd=argc>1?std::string_view(argv[1]):"";
    const int expected=cmd=="inspect"?2:3;
    if((cmd!="inspect"&&cmd!="csv"&&cmd!="ppm")||argc<expected||argc>expected+1)
        throw std::invalid_argument("book03_raster inspect|csv FILE|ppm FILE [quad|clipped]");
    const auto mode=argc==expected+1?std::string_view(argv[expected]):"quad";
    if(mode!="quad"&&mode!="clipped")throw std::invalid_argument("unknown fixture");
    auto r=run(mode=="clipped");
    if(cmd=="inspect")report(r);
    else{
        std::ofstream out(argv[2],std::ios::binary);
        if(!out)throw std::runtime_error("output open failed");
        if(cmd=="ppm")ppm(out,r);else csv(out,r);
        out.flush();if(!out)throw std::runtime_error("output write failed");
    }return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
