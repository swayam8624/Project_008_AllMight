#pragma once
// P27: pixel homogeneous points, optical +Z forward, x2^T F x1=0.
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>
namespace book03::epipolar {
using V=std::array<double,3>;
using M=std::array<V,3>;
struct Match { V a,b; }; // last coordinate must be one
inline double dot(V a,V b){return std::inner_product(a.begin(),a.end(),b.begin(),0.);}
inline V mul(M a,V b){return {dot(a[0],b),dot(a[1],b),dot(a[2],b)};}
inline M transpose(M a){M b{};for(int i=0;i<3;++i)for(int j=0;j<3;++j)b[i][j]=a[j][i];return b;}
inline M product(M a,M b){M c{};b=transpose(b);for(int i=0;i<3;++i)for(int j=0;j<3;++j)c[i][j]=dot(a[i],b[j]);return c;}
inline V cross(V a,V b){return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};}
inline M essential(M r,V t){return product({{{0,-t[2],t[1]},{t[2],0,-t[0]},{-t[1],t[0],0}}},r);}
// K inverse for the upper triangular five-intrinsic camera model.
inline M inverse_k(double fx,double fy,double s,double cx,double cy){
    if(!(fx>0&&fy>0))throw std::invalid_argument("positive focal scales");
    for(double v:{fx,fy,s,cx,cy})if(!std::isfinite(v)||std::abs(v)>1e6)throw std::invalid_argument("intrinsic domain");
    if(fx<1e-6||fy<1e-6)throw std::invalid_argument("focal scale domain");
    return {{{1/fx,-s/(fx*fy),(s*cy/fy-cx)/fx},{0,1/fy,-cy/fy},{0,0,1}}};
}
inline M fundamental(M e,M k1inv,M k2inv){return product(product(transpose(k2inv),e),k1inv);}
inline void point(V p){if(p[2]!=1||!std::isfinite(p[0])||!std::isfinite(p[1])||std::abs(p[0])>1e6||std::abs(p[1])>1e6)throw std::invalid_argument("pixel point domain");}
inline M normalized(M f){double n=0;for(auto row:f)for(double x:row)n=std::hypot(n,x);if(!std::isfinite(n)||n==0)throw std::domain_error("zero or invalid matrix");for(auto& row:f)for(double& x:row)x/=n;return f;}
// Sampson squared distance: first-order approximation, in pixel^2.
inline double sampson(M f,Match m){point(m.a);point(m.b);f=normalized(f);auto l=mul(f,m.a),r=mul(transpose(f),m.b);double e=dot(m.b,l),d=l[0]*l[0]+l[1]*l[1]+r[0]*r[0]+r[1]*r[1];if(d<=1e-30)throw std::domain_error("undefined Sampson gradient");return e*e/d;}
// Right singular vectors by column orthogonalization, not normal equations.
template<std::size_t N> struct Svd{std::array<std::array<double,N>,N> v{};std::array<double,N> sigma{};std::array<std::size_t,N> order{};};
template<std::size_t N> Svd<N> right_svd(std::vector<std::array<double,N>> a){
    Svd<N> out;for(std::size_t i=0;i<N;++i)out.v[i][i]=1;
    bool done=false;
    for(int sweep=0;sweep<160;++sweep){bool changed=false;
        for(std::size_t i=0;i<N;++i)for(std::size_t j=i+1;j<N;++j){double aa=0,bb=0,ab=0;for(auto r:a){aa+=r[i]*r[i];bb+=r[j]*r[j];ab+=r[i]*r[j];}
            // Ignore numerically vanishing columns relative to the pair.
            if(std::min(aa,bb)<=1e-30*std::max(aa,bb)||std::abs(ab)<=1e-13*std::sqrt(aa*bb))continue;
            double z=(bb-aa)/(2*ab),t=std::copysign(1.,z)/(std::abs(z)+std::hypot(1.,z)),c=1/std::hypot(1.,t),s=t*c;
            auto rotate=[&](auto& rows){for(auto& r:rows){double x=r[i],y=r[j];r[i]=c*x-s*y;r[j]=s*x+c*y;}};rotate(a);rotate(out.v);changed=true;
        }if(!changed){done=true;break;}
    }
    if(!done)throw std::runtime_error("SVD sweep limit");
    for(auto r:a)for(std::size_t j=0;j<N;++j)out.sigma[j]+=r[j]*r[j];
    for(auto& x:out.sigma)x=std::sqrt(x);
    std::iota(out.order.begin(),out.order.end(),0);std::sort(out.order.begin(),out.order.end(),[&](auto i,auto j){return out.sigma[i]<out.sigma[j];});return out;
}
inline M similarity(const std::vector<Match>& obs,bool second){
    V c{};double n=static_cast<double>(obs.size());for(auto m:obs){V p=second?m.b:m.a;point(p);c[0]+=p[0]/n;c[1]+=p[1]/n;}double r=0;for(auto m:obs){V p=second?m.b:m.a;r+=(std::pow(p[0]-c[0],2)+std::pow(p[1]-c[1],2))/n;}if(r<1e-20)throw std::domain_error("collapsed image points");double s=std::sqrt(2/r);return {{{s,0,-s*c[0]},{0,s,-s*c[1]},{0,0,1}}};
}
struct Fit{M f;double gap;};
inline Fit eight_point(const std::vector<Match>& obs){
    if(obs.size()<8||obs.size()>10000)throw std::invalid_argument("match count");
    M t1=similarity(obs,false),t2=similarity(obs,true);std::vector<std::array<double,9>> a;
    for(auto m:obs){auto x=mul(t1,m.a),y=mul(t2,m.b);std::array<double,9> row{};for(int i=0;i<3;++i)for(int j=0;j<3;++j)row[3*i+j]=y[i]*x[j];a.push_back(row);}
    auto s=right_svd<9>(a);double next=s.sigma[s.order[1]],gap=next>0?s.sigma[s.order[0]]/next:1;
    if(next<=1e-8*s.sigma[s.order[8]]||gap>.3)throw std::domain_error("ambiguous epipolar null space");
    M f{};for(int i=0;i<3;++i)for(int j=0;j<3;++j)f[i][j]=s.v[3*i+j][s.order[0]];
    // Subtract the smallest singular component: nearest rank-at-most-two
    // matrix in normalized Frobenius norm (not the pixel residual optimum).
    auto q=right_svd<3>({f[0],f[1],f[2]});auto k=q.order[0];V v{q.v[0][k],q.v[1][k],q.v[2][k]},fv=mul(f,v);
    if(q.sigma[q.order[1]]<=1e-8*q.sigma[q.order[2]])throw std::domain_error("rank one estimate");
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)f[i][j]-=fv[i]*v[j];
    return {normalized(product(product(transpose(t2),f),t1)),gap};
}
} // namespace book03::epipolar
