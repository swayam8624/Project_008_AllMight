#pragma once
#include "book03/epipolar.hpp"
#include <random>
#include <cstdint>
#include <optional>
namespace book03::features {
struct Consensus {epipolar::M f;std::vector<std::size_t> inliers;double sum_squared;};
inline Consensus classify(epipolar::M f,const std::vector<epipolar::Match>& obs,double threshold){
    Consensus result{f,{},0};
    for(std::size_t i=0;i<obs.size();++i){try{double e=epipolar::sampson(f,obs[i]);if(e<=threshold*threshold){result.inliers.push_back(i);result.sum_squared+=e;}}catch(const std::domain_error&){/* Undefined gradient: not an inlier. */}}
    return result;
}
inline bool better(const Consensus& a,const Consensus& b){return a.inliers.size()>b.inliers.size()||(a.inliers.size()==b.inliers.size()&&a.sum_squared<b.sum_squared);}
inline std::optional<Consensus> ransac(const std::vector<epipolar::Match>& obs,double threshold,int iterations,std::uint64_t seed){
    if(obs.size()<8||obs.size()>10000||!std::isfinite(threshold)||threshold<=0||threshold>100||iterations<1||iterations>100000)throw std::invalid_argument("RANSAC domain");
    for(auto m:obs){epipolar::point(m.a);epipolar::point(m.b);}
    std::mt19937_64 rng(seed);std::uniform_int_distribution<std::size_t> pick(0,obs.size()-1);
    std::optional<Consensus> best;
    for(int it=0;it<iterations;++it){std::vector<std::size_t> ids;std::vector<epipolar::Match> sample;
        while(ids.size()<8){auto i=pick(rng);if(std::find(ids.begin(),ids.end(),i)==ids.end()){ids.push_back(i);sample.push_back(obs[i]);}}
        try{auto c=classify(epipolar::eight_point(sample).f,obs,threshold);if(c.inliers.size()>=8&&(!best||better(c,*best)))best=std::move(c);}catch(const std::domain_error&){}catch(const std::runtime_error&){}
    }
    if(!best)return best;
    // Refit on the consensus, then rescore ALL observations. Keep only improvements.
    for(int k=0;k<3;++k){std::vector<epipolar::Match> in;for(auto i:best->inliers)in.push_back(obs[i]);
        try{auto c=classify(epipolar::eight_point(in).f,obs,threshold);if(!better(c,*best))break;best=std::move(c);}catch(const std::domain_error&){break;}catch(const std::runtime_error&){break;}
    }return best;
}
} // namespace book03::features
