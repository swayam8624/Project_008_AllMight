#pragma once
// P18: exact integer coverage AFTER a documented subpixel snap.
// Top-left image origin, y down, positive oriented area (clockwise on screen).
// No depth, shading, clipping or hardware-specific sample pattern here.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>
namespace book03 {
inline constexpr std::int64_t subpixel_scale=256;
inline constexpr std::int64_t raster_limit=1'000'000'000;
struct RasterPoint { double x{},y{}; };
struct FixedRasterPoint { std::int64_t x{},y{}; };
inline FixedRasterPoint snap_raster(RasterPoint p){
    constexpr double limit=double(raster_limit)/subpixel_scale;
    if(!std::isfinite(p.x)||!std::isfinite(p.y)||
       std::abs(p.x)>limit||std::abs(p.y)>limit)
        throw std::invalid_argument("screen coordinate outside raster domain");
    // round ties away from zero; scaling by 256 is exact for normal
    // binary64 values in this bounded domain.
    return {std::llround(p.x*subpixel_scale),std::llround(p.y*subpixel_scale)};
}
inline std::int64_t raster_edge(FixedRasterPoint a,FixedRasterPoint b,FixedRasterPoint p){
    // All components must be in [-1e9,1e9]. Differences <=2e9;
    // each product <=4e18; their difference <=8e18 < INT64_MAX.
    for(auto q:{a,b,p})if(q.x< -raster_limit||q.x>raster_limit||
                           q.y< -raster_limit||q.y>raster_limit)
        throw std::invalid_argument("fixed coordinate outside raster domain");
    return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);
}
inline bool inclusive_edge(FixedRasterPoint a,FixedRasterPoint b){
    // Positive-area winding in y-down coordinates: upward edges and
    // horizontal rightward edges own equality. Reversal flips ownership.
    return b.y<a.y||(b.y==a.y&&b.x>a.x);
}
inline std::int64_t floor_div(std::int64_t a,std::int64_t b){
    // positive divisor only; C++ signed division truncates toward zero.
    if(b<=0)throw std::invalid_argument("positive divisor required");
    auto q=a/b;return q-(a%b<0?1:0);
}
inline std::int64_t ceil_div(std::int64_t a,std::int64_t b){
    if(b<=0)throw std::invalid_argument("positive divisor required");
    auto q=a/b;return q+(a%b>0?1:0);
}
struct CoverageSample{
    std::int64_t x{},y{};
    std::array<double,3> weights{}; // original input order, snapped triangle
};
struct CoverageStats{
    std::uint64_t candidates{},covered{};
    bool degenerate{};
};
template<class Emit>
CoverageStats rasterize_triangle(std::array<RasterPoint,3> vertices,
                                std::int64_t width,std::int64_t height,Emit&& emit){
    if(width<=0||height<=0||width>16384||height>16384)
        throw std::invalid_argument("invalid framebuffer size");
    std::array<FixedRasterPoint,3> v{};
    for(int i=0;i<3;++i)v[i]=snap_raster(vertices[i]);
    std::array<int,3> original{0,1,2};
    auto area=raster_edge(v[0],v[1],v[2]);
    CoverageStats stats;
    if(area==0){stats.degenerate=true;return stats;}
    if(area<0){std::swap(v[1],v[2]);std::swap(original[1],original[2]);area=-area;}
    constexpr auto S=subpixel_scale,H=S/2;
    const auto x0=std::max<std::int64_t>(0,ceil_div(std::min({v[0].x,v[1].x,v[2].x})-H,S));
    const auto x1=std::min(width-1,floor_div(std::max({v[0].x,v[1].x,v[2].x})-H,S));
    const auto y0=std::max<std::int64_t>(0,ceil_div(std::min({v[0].y,v[1].y,v[2].y})-H,S));
    const auto y1=std::min(height-1,floor_div(std::max({v[0].y,v[1].y,v[2].y})-H,S));
    if(x0>x1||y0>y1)return stats;
    std::array<std::int64_t,3> row{},dx{},dy{};
    std::array<bool,3> inclusive{};
    for(int i=0;i<3;++i){
        const auto a=v[(i+1)%3],b=v[(i+2)%3];
        row[i]=raster_edge(a,b,{x0*S+H,y0*S+H});
        dx[i]=-(b.y-a.y)*S;dy[i]=(b.x-a.x)*S;
        inclusive[i]=inclusive_edge(a,b);
    }
    for(auto y=y0;y<=y1;++y){
        auto e=row;
        for(auto x=x0;x<=x1;++x){
            ++stats.candidates;
            bool inside=true;
            for(int i=0;i<3;++i)inside&=e[i]>0||(e[i]==0&&inclusive[i]);
            if(inside){
                CoverageSample s{x,y,{}};
                // Do NOT bias edge values used for interpolation.
                for(int i=0;i<3;++i)s.weights[original[i]]=double(e[i])/double(area);
                emit(s);++stats.covered;
            }
            if(x<x1)for(int i=0;i<3;++i)e[i]+=dx[i];
        }
        if(y<y1)for(int i=0;i<3;++i)row[i]+=dy[i];
    }
    return stats;
}
} // namespace book03
