#pragma once
// P32: independent witnesses for the P29/P31 optical reconstruction contract.
#include "book03/studio.hpp"
#include <random>
namespace book03::capstone {
using sfm::V; using sfm::M;
struct Sample { V point; std::array<double,4> noise; };
struct Summary { std::size_t attempted{},accepted{}; double depth_rmse{},position_rmse{},pixel_rmse{}; };
// Each observation coordinate has independent sigma-pixel noise. The caller
// reuses the same samples at each baseline for a paired experiment.
inline Summary evaluate(const std::vector<Sample>& samples,double baseline,double sigma){
    if(samples.empty()||!std::isfinite(baseline)||baseline<=0||baseline>1||!std::isfinite(sigma)||sigma<0||sigma>1)throw std::invalid_argument("experiment domain");
    constexpr M k{{{800,0,320},{0,800,240},{0,0,1}}};
    const auto ki=epipolar::inverse_k(800,800,0,320,240);
    const std::vector<sfm::Pose> poses{sfm::Pose{},sfm::Pose{sfm::identity,{-baseline,0,0}}};
    Summary out;out.attempted=samples.size();double z2=0,p2=0,r2=0;
    for(const auto& s:samples){
        sfm::Track track{{0,sfm::project(poses[0],k,s.point)},{1,sfm::project(poses[1],k,s.point)}};
        for(int j=0;j<2;++j)for(int c=0;c<2;++c)track[j].pixel[c]+=sigma*s.noise[2*j+c];
        try{auto x=sfm::triangulate(poses,ki,track);x=sfm::refine_point(poses,k,track,x);
            double ze=x[2]-s.point[2],pe=sfm::norm(sfm::add(x,s.point,-1)),re=sfm::cost(poses,k,track,x);
            if(!std::isfinite(ze)||!std::isfinite(pe)||!std::isfinite(re))continue;
            z2+=ze*ze;p2+=pe*pe;r2+=re;++out.accepted;
        }catch(const std::domain_error&){/* preserve rejected count */}
    }
    if(out.accepted){out.depth_rmse=std::sqrt(z2/out.accepted);out.position_rmse=std::sqrt(p2/out.accepted);out.pixel_rmse=std::sqrt(r2/(2*out.accepted));}
    return out;
}
// An independent closed-form first-order depth uncertainty for rectified views.
inline double predicted_depth_sigma(double z,double baseline,double sigma){return z*z*std::sqrt(2.)*sigma/(800*baseline);}
} // namespace book03::capstone
