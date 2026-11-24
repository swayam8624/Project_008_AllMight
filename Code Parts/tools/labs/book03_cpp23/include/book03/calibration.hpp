#pragma once
// P26: normalized 3D-to-2D DLT, optical +Z forward, pixels, metres.
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>
namespace book03::calibration {
using V3=std::array<double,3>;
using V2=std::array<double,2>;
using Row=std::array<double,12>;
using Camera=std::array<std::array<double,4>,3>;
struct Match { V3 world; V2 pixel; };
inline double dot(V3 a,V3 b){return std::inner_product(a.begin(),a.end(),b.begin(),0.);}
inline V3 sub(V3 a,V3 b,double scale=1){for(int i=0;i<3;++i)a[i]-=scale*b[i];return a;}
inline double norm(V3 a){return std::sqrt(dot(a,a));}
inline V2 project(const Camera& p,V3 x){
    V3 y{};for(int i=0;i<3;++i)y[i]=p[i][3]+dot({p[i][0],p[i][1],p[i][2]},x);
    if(!std::isfinite(y[2])||y[2]<=1e-12)throw std::domain_error("nonpositive projective depth");
    V2 u{y[0]/y[2],y[1]/y[2]};for(double v:u)if(!std::isfinite(v))throw std::overflow_error("projection");return u;
}
struct Fit {Camera p;double gap_ratio;double rms_pixels;};
inline Fit dlt(const std::vector<Match>& obs){
    if(obs.size()<6||obs.size()>10000)throw std::invalid_argument("DLT observation count");
    V3 c{};V2 d{};const double n=static_cast<double>(obs.size());
    for(const auto& o:obs){for(int j=0;j<3;++j){if(!std::isfinite(o.world[j])||std::abs(o.world[j])>1e6)throw std::invalid_argument("world domain");c[j]+=o.world[j]/n;}for(int j=0;j<2;++j){if(!std::isfinite(o.pixel[j])||std::abs(o.pixel[j])>1e6)throw std::invalid_argument("pixel domain");d[j]+=o.pixel[j]/n;}}
    double a=0,b=0;for(const auto& o:obs){auto x=sub(o.world,c);a+=dot(x,x)/n;b+=(std::pow(o.pixel[0]-d[0],2)+std::pow(o.pixel[1]-d[1],2))/n;}
    if(a<1e-20||b<1e-20)throw std::domain_error("collapsed observations");
    const double s=std::sqrt(3/a),t=std::sqrt(2/b);
    std::vector<Row> A;A.reserve(2*obs.size());
    for(const auto& o:obs){const auto x=sub(o.world,c);std::array<double,4> h{s*x[0],s*x[1],s*x[2],1};Row u{},v{};for(int j=0;j<4;++j){u[j]=h[j];v[j+4]=h[j];u[j+8]=-t*(o.pixel[0]-d[0])*h[j];v[j+8]=-t*(o.pixel[1]-d[1])*h[j];}A.push_back(u);A.push_back(v);}
    // One-sided Jacobi SVD: rotate columns directly, avoiding A^T A.
    std::array<Row,12> V{};for(int i=0;i<12;++i)V[i][i]=1;
    bool converged=false;
    for(int sweep=0;sweep<100;++sweep){bool changed=false;
        for(int i=0;i<12;++i)for(int j=i+1;j<12;++j){double aa=0,bb=0,ab=0;for(const auto& r:A){aa+=r[i]*r[i];bb+=r[j]*r[j];ab+=r[i]*r[j];}if(aa==0||bb==0||std::abs(ab)<=1e-13*std::sqrt(aa*bb))continue;
            changed=true;const double z=(bb-aa)/(2*ab),q=std::copysign(1.,z)/(std::abs(z)+std::hypot(1.,z)),co=1/std::hypot(1.,q),si=q*co;
            auto rotate=[&](auto& rows){for(auto& r:rows){double x=r[i],y=r[j];r[i]=co*x-si*y;r[j]=si*x+co*y;}};rotate(A);rotate(V);
        }if(!changed){converged=true;break;}
    }
    if(!converged)throw std::runtime_error("Jacobi SVD did not converge");
    Row sig{};for(const auto& r:A)for(int j=0;j<12;++j)sig[j]+=r[j]*r[j];for(double& z:sig)z=std::sqrt(z);
    std::array<int,12> order{};std::iota(order.begin(),order.end(),0);std::sort(order.begin(),order.end(),[&](int i,int j){return sig[i]<sig[j];});
    const double second=sig[order[1]],ratio=second>0?sig[order[0]]/second:1;
    // Sufficient engineering gate, not a universal rank certificate.
    if(second<=1e-8*sig[order[11]]||ratio>.2)throw std::domain_error("ambiguous camera null space");
    Camera p{};for(int i=0;i<3;++i)for(int j=0;j<4;++j)p[i][j]=V[4*i+j][order[0]];
    // Denormalize: P = T_image^{-1} P_normalized T_world.
    for(auto& row:p){for(int j=0;j<3;++j)row[j]*=s;for(int j=0;j<3;++j)row[3]-=row[j]*c[j];}
    for(int j=0;j<4;++j){p[0][j]=p[0][j]/t+d[0]*p[2][j];p[1][j]=p[1][j]/t+d[1]*p[2][j];}
    double z=norm({p[2][0],p[2][1],p[2][2]});if(z<1e-15||!std::isfinite(z))throw std::domain_error("nonfinite camera");
    if(p[2][3]+dot({p[2][0],p[2][1],p[2][2]},obs.front().world)<0)z=-z;
    for(auto& row:p)for(double& v:row)v/=z;
    double error=0;for(const auto& o:obs){auto u=project(p,o.world);error+=(std::pow(u[0]-o.pixel[0],2)+std::pow(u[1]-o.pixel[1],2))/n;}
    return {p,ratio,std::sqrt(error)};
}
struct Intrinsics {double fx,fy,skew,cx,cy;std::array<V3,3> rotation;V3 translation;};
inline Intrinsics decompose(const Camera& p){
    const V3 a{p[0][0],p[0][1],p[0][2]},b{p[1][0],p[1][1],p[1][2]},c{p[2][0],p[2][1],p[2][2]};
    if(std::abs(norm(c)-1)>1e-8)throw std::invalid_argument("camera scale");
    const double cy=dot(b,c);auto ry=sub(b,c,cy);const double fy=norm(ry);if(fy<1e-9)throw std::domain_error("degenerate fy");for(double& v:ry)v/=fy;
    const double cx=dot(a,c),skew=dot(a,ry);auto rx=sub(sub(a,c,cx),ry,skew);const double fx=norm(rx);if(fx<1e-9)throw std::domain_error("degenerate fx");for(double& v:rx)v/=fx;
    const V3 cross{rx[1]*ry[2]-rx[2]*ry[1],rx[2]*ry[0]-rx[0]*ry[2],rx[0]*ry[1]-rx[1]*ry[0]};if(dot(cross,c)<.999999)throw std::domain_error("improper rotation");
    const double tz=p[2][3],ty=(p[1][3]-cy*tz)/fy,tx=(p[0][3]-cx*tz-skew*ty)/fx;
    return {fx,fy,skew,cx,cy,{rx,ry,c},{tx,ty,tz}};
}
} // namespace book03::calibration
