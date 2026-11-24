#pragma once
// P21 opaque reference backend: P18 coverage + P19 depth + P20 linear-light encode.
// Input is an already clipped, divided triangle, retaining original clip w.
#include "book03/visibility.hpp"
#include "book03/sampling.hpp"
#include <ostream>
namespace book03 {
struct RenderColor {double r{},g{},b{};};
struct RenderVertex {RasterPoint screen{};double q{},w{1},u{},v{};};
struct RenderStats {std::uint64_t triangles{},candidates{},covered{},passed{},failed{},ties{},degenerate{};};
class SoftwareRenderer {
    int w_,h_;
    VisibilityBuffer depth_;
    std::vector<RenderColor> colors_;
    RenderStats stats_{};
    static constexpr RenderColor background{.035,.05,.065};
public:
    SoftwareRenderer(int w,int h,VisibilityPolicy p={}):w_(w),h_(h),depth_(w,h,p),colors_(std::size_t(w)*h,background){}
    void clear(){depth_.clear();std::fill(colors_.begin(),colors_.end(),background);stats_={};}
    const RenderStats& stats()const{return stats_;}
    const auto& winner(int x,int y)const{return depth_.at(x,y);}
    RenderColor color(int x,int y)const{(void)depth_.at(x,y);return colors_[std::size_t(y)*w_+x];}
    // UV checker is a deliberately unfiltered diagnostic shader. The color
    // multiplier is flat for this primitive, not a lighting model.
    void draw(std::array<RenderVertex,3> v,std::uint32_t id,RenderColor base,bool checker=true){
        for(double c:{base.r,base.g,base.b})if(!std::isfinite(c)||c<0||c>1)throw std::invalid_argument("linear color");
        std::array<RasterPoint,3> p{};
        double wmin=std::numeric_limits<double>::infinity();
        for(int i=0;i<3;++i){
            if(!std::isfinite(v[i].q)||v[i].q<0||v[i].q>1||!std::isfinite(v[i].w)||v[i].w<1e-6||v[i].w>1e6||
               !std::isfinite(v[i].u)||!std::isfinite(v[i].v)||std::abs(v[i].u)>1e6||std::abs(v[i].v)>1e6)
                throw std::invalid_argument("renderer vertex domain");
            p[i]=v[i].screen;wmin=std::min(wmin,v[i].w);
        }
        auto c=rasterize_triangle(p,w_,h_,[&](CoverageSample s){
            double q=0,den=0,u=0,vv=0;
            for(int i=0;i<3;++i){
                q+=s.weights[i]*v[i].q;
                const double k=s.weights[i]*(wmin/v[i].w);
                den+=k;u+=k*v[i].u;vv+=k*v[i].v;
            }
            // Only depth winners write color. Compute the pure shader before
            // commit so a rejected shader cannot leave a mismatched record.
            u/=den;vv/=den;
            const auto ix=static_cast<long long>(std::floor(8*u)),iy=static_cast<long long>(std::floor(8*vv));
            double gain=checker&&((ix+iy)%2!=0)?.32:1.;
            bool tie=false;
            if(depth_.submit(s.x,s.y,std::clamp(q,0.,1.),id,tie)){
                colors_[std::size_t(s.y)*w_+s.x]={gain*base.r,gain*base.g,gain*base.b};++stats_.passed;
            }else ++stats_.failed;
            stats_.ties+=tie;
        });
        ++stats_.triangles;stats_.candidates+=c.candidates;stats_.covered+=c.covered;stats_.degenerate+=c.degenerate;
    }
    void ppm(std::ostream& out)const{
        out<<"P6\n"<<w_<<" "<<h_<<"\n255\n";
        for(auto c:colors_){const unsigned char rgb[]{
            static_cast<unsigned char>(std::lround(255*srgb_encode(c.r))),
            static_cast<unsigned char>(std::lround(255*srgb_encode(c.g))),
            static_cast<unsigned char>(std::lround(255*srgb_encode(c.b)))};
            out.write(reinterpret_cast<const char*>(rgb),3);}
        if(!out)throw std::runtime_error("PPM output failed");
    }
};
} // namespace book03
