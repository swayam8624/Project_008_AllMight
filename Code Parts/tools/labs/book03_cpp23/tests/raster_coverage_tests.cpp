#include "book03/raster_coverage.hpp"
#include <iostream>
#include <random>
#include <vector>
#include <string>
using namespace book03;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
using Tri=std::array<RasterPoint,3>;
std::vector<int> mask(Tri t,int w=16,int h=16){
    std::vector<int> m(w*h);
    rasterize_triangle(t,w,h,[&](CoverageSample s){
        ++m[s.y*w+s.x];
        double sum=0,x=0,y=0;
        for(int i=0;i<3;++i){auto q=snap_raster(t[i]);sum+=s.weights[i];
            x+=s.weights[i]*q.x/256.;y+=s.weights[i]*q.y/256.;}
        check(std::abs(sum-1)<1e-12,"weight sum");
        check(std::abs(x-(s.x+.5))<1e-9&&std::abs(y-(s.y+.5))<1e-9,"weight reconstruction");
    });return m;
}
void quad(double left,double top,double right,double bottom){
    RasterPoint a{left,top},b{right,top},c{right,bottom},d{left,bottom};
    auto p=mask({a,b,c}),q=mask({a,c,d});
    auto r=mask({b,a,d}),s=mask({d,c,b});
    for(int y=0;y<16;++y)for(int x=0;x<16;++x){
        int expected=x+.5>=left&&x+.5<right&&y+.5>=top&&y+.5<bottom;
        check(p[y*16+x]+q[y*16+x]==expected,"diagonal AC ownership");
        check(r[y*16+x]+s[y*16+x]==expected,"diagonal BD ownership");
    }
}
int main(){try{
    quad(2,2,10,10);quad(2.5,2.5,10.5,10.5);quad(-2.5,-1.5,8.5,9.5);
    check(floor_div(-257,256)==-2&&ceil_div(-257,256)==-1,"signed division");
    check(snap_raster({.5/256,-.5/256}).x==1&&snap_raster({.5/256,-.5/256}).y==-1,"ties");
    auto deg=rasterize_triangle(Tri{{{1,1},{1.0001,1},{1,1.0001}}},16,16,[](auto){});
    check(deg.degenerate&&deg.covered==0,"snap collapse");
    auto outside=rasterize_triangle(Tri{{{-5,-5},{-3,-5},{-5,-3}}},16,16,[](auto){});
    check(outside.candidates==0,"outside bbox");
    std::mt19937 rng(180018);
    auto coord=[&]{return (int(rng()%8192)-2048)/256.;};
    for(int n=0;n<2000;++n){
        Tri t{{{coord(),coord()},{coord(),coord()},{coord(),coord()}}};
        auto m=mask(t);std::swap(t[1],t[2]);check(mask(t)==m,"winding invariance");
        // Independent direct evaluation per sample (no incremental state).
        auto a=snap_raster(t[0]),b=snap_raster(t[1]),c=snap_raster(t[2]);
        auto orient=[](auto a,auto b,auto p){return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);};
        if(orient(a,b,c)<0)std::swap(b,c);
        for(int y=0;y<16;++y)for(int x=0;x<16;++x){
            FixedRasterPoint p{x*256+128,y*256+128};
            auto pass=[&](auto u,auto v){auto e=orient(u,v,p);return e>0||(e==0&&(v.y<u.y||(v.y==u.y&&v.x>u.x)));};
            int want=orient(a,b,c)>0&&pass(a,b)&&pass(b,c)&&pass(c,a);
            check(m[y*16+x]==want,"direct oracle");
        }
    }
    std::cout<<"Book03/P18 coverage contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
