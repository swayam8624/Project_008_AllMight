// PROJECTALLMIGHT raster pipeline-state reference: homogeneous clipping, 4x coverage, stencil and premultiplied blend.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V2{ double x{},y{}; };
struct V4{ double x{},y{},z{},w{}; };
V4 lerp(V4 a,V4 b,double t){ return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t,a.w+(b.w-a.w)*t}; }

double left_clip_distance(V4 v){ return v.x+v.w; } // homogeneous plane x >= -w

std::vector<V4> clip_left(const std::vector<V4>& in){
    std::vector<V4> out;
    if(in.empty()) return out;
    V4 a=in.back();
    double da=left_clip_distance(a);
    for(const V4 b:in){
        const double db=left_clip_distance(b);
        const bool ia=da>=0.0, ib=db>=0.0;
        if(ia!=ib){
            const double denom=da-db;
            require(std::fabs(denom)>1e-15,"clip crossing denominator vanished");
            const double t=da/denom;
            out.push_back(lerp(a,b,t));
        }
        if(ib) out.push_back(b);
        a=b; da=db;
    }
    return out;
}

double edge(V2 a,V2 b,V2 p){ return (p.x-a.x)*(b.y-a.y)-(p.y-a.y)*(b.x-a.x); }
bool inside_ccw(const std::array<V2,3>& t,V2 p){
    const double area=edge(t[0],t[1],t[2]);
    require(std::fabs(area)>1e-12,"degenerate coverage triangle");
    const double sign=area>0?1.0:-1.0;
    return sign*edge(t[0],t[1],p)>=0.0 &&
           sign*edge(t[1],t[2],p)>=0.0 &&
           sign*edge(t[2],t[0],p)>=0.0;
}

int coverage_4x(const std::array<V2,3>& t){
    // One common rotated-grid pattern in pixel-local [0,1]^2 coordinates.
    constexpr std::array<V2,4> samples{{{0.375,0.125},{0.875,0.375},{0.125,0.625},{0.625,0.875}}};
    int count=0;
    for(const V2 p:samples) if(inside_ccw(t,p)) ++count;
    return count;
}

std::uint8_t stencil_equal_increment(std::uint8_t stored,std::uint8_t reference){
    return stored==reference ? static_cast<std::uint8_t>(std::min<int>(255,stored+1)) : stored;
}

struct Color{ double r{},g{},b{},a{}; };
Color over_premultiplied(Color src,Color dst){
    require(src.a>=0&&src.a<=1&&dst.a>=0&&dst.a<=1,"alpha outside [0,1]");
    const double one_minus=1.0-src.a;
    return {src.r+dst.r*one_minus,
            src.g+dst.g*one_minus,
            src.b+dst.b*one_minus,
            src.a+dst.a*one_minus};
}

void test_homogeneous_clip(){
    const std::vector<V4> tri{{-2,0,0,1},{0.5,-0.5,0,1},{0.5,0.5,0,1}};
    const auto clipped=clip_left(tri);
    require(clipped.size()==4,"one-outside triangle should clip to a quad");
    for(const V4 v:clipped) require(left_clip_distance(v)>=-1e-12,"clipped vertex remains outside left plane");
    int on_plane=0;
    for(const V4 v:clipped) if(std::fabs(left_clip_distance(v))<1e-12) ++on_plane;
    require(on_plane==2,"clip should create two intersections on the plane");
}

void test_multisample_coverage(){
    const std::array<V2,3> tri{{{0,0},{1,0},{0,1}}};
    require(coverage_4x(tri)==2,"4x sample coverage trace changed");
}

void test_stencil_and_blend(){
    require(stencil_equal_increment(7,7)==8,"stencil equal/pass increment failed");
    require(stencil_equal_increment(7,8)==7,"stencil compare failure should preserve stored value");
    require(stencil_equal_increment(255,255)==255,"stencil increment must saturate");

    const Color out=over_premultiplied({0.4,0.2,0.0,0.5},{0.2,0.4,0.6,1.0});
    require(std::fabs(out.r-0.5)<1e-12 && std::fabs(out.g-0.4)<1e-12 &&
            std::fabs(out.b-0.3)<1e-12 && std::fabs(out.a-1.0)<1e-12,
            "premultiplied source-over result wrong");
}

} // namespace

int main(){
    try{
        test_homogeneous_clip();
        test_multisample_coverage();
        test_stencil_and_blend();
        std::cout<<"raster_pipeline_state_lab: PASS\n"
                 <<"proved: one homogeneous clip plane, generated intersections, explicit 4x sample coverage, stencil compare/pass behavior, premultiplied source-over blending\n"
                 <<"not proved: all six clip planes, sample-mask/centroid rules, derivatives, depth-stencil ordering for every API, conservative rasterization, or GPU parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"raster_pipeline_state_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
