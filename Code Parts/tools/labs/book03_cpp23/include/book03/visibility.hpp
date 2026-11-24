#pragma once
// P19: opaque, single-sample visibility on the P18 snapped screen grid.
// Inputs are already clipped; q is divided NDC Z, NOT metres or range.
// Quantize BEFORE comparing. An explicit empty bit is not hardware clear.
#include "book03/raster_coverage.hpp"
#include <limits>
#include <vector>
#include <optional>
namespace book03 {
enum class VisibilityOrder {Forward,Reverse};
enum class DepthStorage {Float64,Float32,UNorm24};
enum class TiePolicy {KeepFirst,KeepLast,LowestID};
struct VisibilityPolicy {
    VisibilityOrder order{VisibilityOrder::Forward};
    DepthStorage storage{DepthStorage::Float32};
    TiePolicy tie{TiePolicy::KeepFirst};
};
inline double stored_depth(double q,DepthStorage storage){
    if(!std::isfinite(q)||q<0||q>1)throw std::invalid_argument("NDC depth outside [0,1]");
    static_assert(std::numeric_limits<float>::is_iec559 &&
                  std::numeric_limits<float>::digits==24 &&
                  std::numeric_limits<float>::max_exponent==128);
    switch(storage){
    case DepthStorage::Float64:return q;
    case DepthStorage::Float32:return static_cast<double>(static_cast<float>(q));
    case DepthStorage::UNorm24:return std::round(q*16777215.0)/16777215.0;
    }
    throw std::invalid_argument("unknown depth storage");
}
struct DepthVertex {RasterPoint screen{};double q{};};
struct VisibleSample {double depth{};std::uint32_t id{};};
struct VisibilityStats {CoverageStats coverage{};std::uint64_t passed{},failed{},ties{};};
class VisibilityBuffer {
    std::int64_t w_,h_;
    VisibilityPolicy policy_;
    std::vector<std::optional<VisibleSample>> pixels_;
    std::size_t index(std::int64_t x,std::int64_t y)const{
        if(x<0||x>=w_||y<0||y>=h_)throw std::out_of_range("visibility address");
        return static_cast<std::size_t>(y*w_+x);
    }
public:
    VisibilityBuffer(std::int64_t w,std::int64_t h,VisibilityPolicy p={}):w_(w),h_(h),policy_(p){
        if(w<=0||h<=0||w>4096||h>4096||w*h>4'194'304)
            throw std::invalid_argument("visibility allocation bound");
        pixels_.resize(static_cast<std::size_t>(w*h));
    }
    void clear(){for(auto& p:pixels_)p.reset();}
    const std::optional<VisibleSample>& at(std::int64_t x,std::int64_t y)const{return pixels_[index(x,y)];}
    // Empty sample accepts either endpoint. Unlike a strict hardware test
    // against clear=1 or clear=0, this reference distinguishes empty from far.
    bool submit(std::int64_t x,std::int64_t y,double q,std::uint32_t id,bool& tie){
        auto& p=pixels_[index(x,y)];
        const double z=stored_depth(q,policy_.storage);
        tie=p&&z==p->depth;
        bool pass=!p;
        if(p){
            pass=policy_.order==VisibilityOrder::Forward?z<p->depth:z>p->depth;
            if(tie)pass=policy_.tie==TiePolicy::KeepLast||
                (policy_.tie==TiePolicy::LowestID&&id<p->id);
        }
        if(pass)p=VisibleSample{z,id};
        return pass;
    }
    VisibilityStats draw(std::array<DepthVertex,3> v,std::uint32_t id){
        std::array<RasterPoint,3> p{};
        for(int i=0;i<3;++i){
            (void)stored_depth(v[i].q,policy_.storage);p[i]=v[i].screen;
        }
        VisibilityStats result;
        result.coverage=rasterize_triangle(p,w_,h_,[&](CoverageSample s){
            // NDC depth is screen-linear. The edge weights belong to the
            // snapped geometry; clamp only arithmetic drift of a convex sum.
            const double q=std::clamp(s.weights[0]*v[0].q+
                s.weights[1]*v[1].q+s.weights[2]*v[2].q,0.,1.);
            bool tie=false;
            if(submit(s.x,s.y,q,id,tie))++result.passed;else ++result.failed;
            result.ties+=tie;
        });
        return result;
    }
};
} // namespace book03
