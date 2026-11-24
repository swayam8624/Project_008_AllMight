#pragma once
// P29: calibrated optical coordinates, +Z forward; first pose fixed, baseline one.
#include "book03/epipolar.hpp"
#include <optional>
namespace book03::sfm {
using epipolar::V;using epipolar::M;using epipolar::dot;using epipolar::mul;using epipolar::product;using epipolar::transpose;using epipolar::cross;
inline V add(V a,V b,double s=1){for(int i=0;i<3;++i)a[i]+=s*b[i];return a;}
inline V scale(V a,double s){for(auto& x:a)x*=s;return a;}
inline double norm(V a){return std::hypot(a[0],a[1],a[2]);}
inline V unit(V a){double n=norm(a);if(!(n>1e-12)||!std::isfinite(n))throw std::domain_error("zero direction");return scale(a,1/n);}
inline M columns(V a,V b,V c){return transpose({a,b,c});}
inline constexpr M identity{{{1,0,0},{0,1,0},{0,0,1}}};
struct Pose{M r=identity;V t{};};
struct Observation{std::size_t camera;V pixel;};
using Track=std::vector<Observation>;
inline V optical(Pose p,V x){return add(mul(p.r,x),p.t);}
inline V project(Pose p,M k,V x){auto y=optical(p,x);if(!(y[2]>1e-8))throw std::domain_error("behind camera");auto z=mul(k,y);return {z[0]/z[2],z[1]/z[2],1};}
// Small pivoted solve for bounded teaching normal systems, not a general solver.
inline V solve(M a,V b){double bound=0;for(auto row:a)for(double x:row)bound=std::max(bound,std::abs(x));
    for(int i=0;i<3;++i){int p=i;for(int j=i+1;j<3;++j)if(std::abs(a[j][i])>std::abs(a[p][i]))p=j;
        if(!(std::abs(a[p][i])>1e-12*bound))throw std::domain_error("weak normal system");
        std::swap(a[p],a[i]);std::swap(b[p],b[i]);double d=a[i][i];for(int k=i;k<3;++k)a[i][k]/=d;b[i]/=d;
        for(int j=0;j<3;++j)if(j!=i){double f=a[j][i];for(int k=i;k<3;++k)a[j][k]-=f*a[i][k];b[j]-=f*b[i];}
    }return b;
}
inline void check_track(const std::vector<Pose>& poses,const Track& track){if(track.size()<2||track.size()>poses.size())throw std::invalid_argument("track length");std::vector<bool> seen(poses.size());for(auto o:track){if(o.camera>=poses.size()||seen[o.camera])throw std::invalid_argument("duplicate or invalid camera");seen[o.camera]=true;epipolar::point(o.pixel);}}
// Least squares of perpendicular distances to unit world rays.
inline V triangulate(const std::vector<Pose>& poses,M ki,const Track& track){check_track(poses,track);M h{};V b{};
    for(auto o:track){auto p=poses[o.camera];auto rt=transpose(p.r);auto d=unit(mul(rt,mul(ki,o.pixel)));auto c=scale(mul(rt,p.t),-1);M q=identity;
        for(int i=0;i<3;++i)for(int j=0;j<3;++j){q[i][j]-=d[i]*d[j];h[i][j]+=q[i][j];}
        b=add(b,mul(q,c));
    }auto x=solve(h,b);for(auto o:track)if(!(optical(poses[o.camera],x)[2]>1e-8))throw std::domain_error("negative depth");return x;
}
// Build proper U,V from the two dominant singular directions. Their cross
// products choose the null-vector signs; E scale/sign is arbitrary.
inline std::array<Pose,4> candidates(M e){e=epipolar::normalized(e);auto s=epipolar::right_svd<3>({e[0],e[1],e[2]});auto column=[&](std::size_t i){return V{s.v[0][i],s.v[1][i],s.v[2][i]};};
    if(s.sigma[s.order[1]]<.1*s.sigma[s.order[2]])throw std::domain_error("rank one essential");
    auto v1=unit(column(s.order[2])),v2=unit(column(s.order[1])),v3=unit(cross(v1,v2));auto u1=unit(mul(e,v1)),u2=unit(mul(e,v2));u2=unit(add(u2,u1,-dot(u1,u2)));auto u3=unit(cross(u1,u2));
    auto u=columns(u1,u2,u3),vt=transpose(columns(v1,v2,v3));const M w{{{0,-1,0},{1,0,0},{0,0,1}}};auto a=product(product(u,w),vt),b=product(product(u,transpose(w)),vt);return {{{a,u3},{a,scale(u3,-1)},{b,u3},{b,scale(u3,-1)}}};
}
struct Seed{Pose pose;std::array<std::size_t,4> votes;};
inline Seed recover(M f,M k,M ki,const std::vector<epipolar::Match>& matches){if(matches.size()<8)throw std::invalid_argument("seed matches");auto cs=candidates(product(product(transpose(k),f),k));std::array<std::size_t,4> votes{};
    for(int j=0;j<4;++j)for(auto m:matches){try{auto x=triangulate({Pose{},cs[j]},ki,{{0,m.a},{1,m.b}});auto d=unit(x),e=unit(add(x,mul(transpose(cs[j].r),cs[j].t)));if(norm(cross(d,e))>.005)++votes[j];}catch(const std::domain_error&){} }
    std::array<int,4> order{0,1,2,3};std::sort(order.begin(),order.end(),[&](int a,int b){return votes[a]>votes[b];});if(votes[order[0]]<8||votes[order[0]]<=votes[order[1]]||votes[order[0]]<.8*matches.size())throw std::domain_error("ambiguous seed");return {cs[order[0]],votes};
}
inline double cost(const std::vector<Pose>& poses,M k,const Track& track,V x){double c=0;for(auto o:track){auto p=project(poses[o.camera],k,x);c+=std::pow(p[0]-o.pixel[0],2)+std::pow(p[1]-o.pixel[1],2);}return c;}
// Pixel Jacobian rows with respect to camera-frame point. K is upper triangular.
inline std::array<V,2> jacobian(M k,V y){double z=y[2];if(!(z>1e-8))throw std::domain_error("behind camera");return {{{k[0][0]/z,k[0][1]/z,-(k[0][0]*y[0]+k[0][1]*y[1])/(z*z)},{0,k[1][1]/z,-k[1][1]*y[1]/(z*z)}}};}
inline void accumulate(M& h,V& g,V j,double residual){for(int a=0;a<3;++a){g[a]-=j[a]*residual;for(int b=0;b<3;++b)h[a][b]+=j[a]*j[b];}}
inline V refine_point(const std::vector<Pose>& poses,M k,const Track& track,V x){check_track(poses,track);double old=cost(poses,k,track,x);
    for(int it=0;it<12;++it){M h{};V g{};for(auto o:track){auto p=poses[o.camera];auto y=optical(p,x),q=project(p,k,x);auto j=jacobian(k,y);for(int a=0;a<2;++a)accumulate(h,g,mul(transpose(p.r),j[a]),q[a]-o.pixel[a]);}
        V d;try{d=solve(h,g);}catch(const std::domain_error&){break;}bool accepted=false;for(int l=0;l<16;++l){auto trial=add(x,d,std::ldexp(1.,-l));try{double c=cost(poses,k,track,trial);if(c<old){x=trial;old=c;accepted=true;break;}}catch(const std::domain_error&){} }if(!accepted||norm(d)<1e-10)break;
    }return x;
}
} // namespace book03::sfm
