#pragma once
// P23: normalized CV coordinates (+x right, +y down). The model domain
// is a closed disk. Forward evaluation need not be invertible; inverse
// requires a conservative global injectivity certificate on that disk.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
namespace book03 {
struct DistortionPoint {double x{},y{};};
struct DistortionJacobian {double xx{},xy{},yx{},yy{};};
struct BrownConrady {
    double k1{},k2{},k3{},p1{},p2{},radius{1};
    void validate()const{
        for(double v:{k1,k2,k3,p1,p2})if(!std::isfinite(v)||std::abs(v)>4)
            throw std::invalid_argument("coefficient domain");
        if(!std::isfinite(radius)||radius<.01||radius>2)
            throw std::invalid_argument("radius domain");
    }
    void check(DistortionPoint p)const{
        validate();if(!std::isfinite(p.x)||!std::isfinite(p.y)||std::hypot(p.x,p.y)>radius)
            throw std::invalid_argument("point outside calibrated disk");
    }
    DistortionPoint forward(DistortionPoint p)const{
        check(p);const double x=p.x,y=p.y,q=x*x+y*y;
        const double l=1+q*(k1+q*(k2+q*k3));
        return {x*l+2*p1*x*y+p2*(q+2*x*x),y*l+p1*(q+2*y*y)+2*p2*x*y};
    }
    DistortionJacobian jacobian(DistortionPoint p)const{
        check(p);const double x=p.x,y=p.y,q=x*x+y*y;
        const double l=1+q*(k1+q*(k2+q*k3)),d=k1+q*(2*k2+3*q*k3);
        const double cross=2*x*y*d+2*p1*x+2*p2*y;
        return {l+2*x*x*d+2*p1*y+6*p2*x,cross,cross,l+2*y*y*d+6*p1*y+2*p2*x};
    }
    // Symmetric J: radial eigenvalue perturbations and tangential row sums
    // bound ||J-I||_2. Margin .9 is a conservative engineering decision.
    double derivative_bound()const{
        validate();const double q=radius*radius;
        return q*(3*std::abs(k1)+q*(5*std::abs(k2)+7*q*std::abs(k3)))+
            8*radius*(std::abs(p1)+std::abs(p2));
    }
};
enum class InverseStatus {Converged,Uncertified,NoProgress,IterationLimit};
struct DistortionInverse {InverseStatus status{};DistortionPoint point{};double residual{};int iterations{};};
inline DistortionInverse undistort(const BrownConrady& m,DistortionPoint target){
    m.validate();if(!std::isfinite(target.x)||!std::isfinite(target.y)||std::hypot(target.x,target.y)>10000)
        throw std::invalid_argument("target domain");
    if(m.derivative_bound()>=.9)return {InverseStatus::Uncertified,{},std::hypot(target.x,target.y),0};
    DistortionPoint p{}; // start inside calibrated disk, even for outside targets
    auto error=[&](DistortionPoint x){auto y=m.forward(x);return std::hypot(y.x-target.x,y.y-target.y);};
    double e=error(p);
    for(int i=0;i<40;++i){
        if(e<=1e-12)return {InverseStatus::Converged,p,e,i};
        const auto y=m.forward(p);const auto j=m.jacobian(p);
        const double a=y.x-target.x,b=y.y-target.y,det=j.xx*j.yy-j.xy*j.yx;
        const double dx=(j.yy*a-j.xy*b)/det,dy=(j.xx*b-j.yx*a)/det;
        bool accepted=false;
        for(int k=0;k<32;++k){
            const double t=std::ldexp(1.,-k);DistortionPoint q{p.x-t*dx,p.y-t*dy};
            if(std::hypot(q.x,q.y)>m.radius)continue;
            const double next=error(q);
            if(next<e){p=q;e=next;accepted=true;break;}
        }
        if(!accepted)return {InverseStatus::NoProgress,p,e,i};
    }
    return {e<=1e-12?InverseStatus::Converged:InverseStatus::IterationLimit,p,e,40};
}
enum class AngularProjection {Rectilinear,Equidistant,Equisolid,Stereographic,Orthographic};
// Restrict to the common forward hemisphere, excluding its singular rim.
inline double angular_radius(AngularProjection mode,double theta){
    if(!std::isfinite(theta)||theta<0||theta>=std::numbers::pi/2)
        throw std::invalid_argument("forward ray angle required");
    switch(mode){
    case AngularProjection::Rectilinear:return std::tan(theta);
    case AngularProjection::Equidistant:return theta;
    case AngularProjection::Equisolid:return 2*std::sin(theta/2);
    case AngularProjection::Stereographic:return 2*std::tan(theta/2);
    case AngularProjection::Orthographic:return std::sin(theta);
    }throw std::invalid_argument("angular projection");
}
inline double angular_inverse(AngularProjection mode,double r){
    if(!std::isfinite(r)||r<0)throw std::invalid_argument("angular radius");
    double t{};
    switch(mode){
    case AngularProjection::Rectilinear:t=std::atan(r);break;
    case AngularProjection::Equidistant:t=r;break;
    case AngularProjection::Equisolid:if(r>=std::numbers::sqrt2)throw std::invalid_argument("radius");t=2*std::asin(r/2);break;
    case AngularProjection::Stereographic:t=2*std::atan(r/2);break;
    case AngularProjection::Orthographic:if(r>=1)throw std::invalid_argument("radius");t=std::asin(r);break;
    default:throw std::invalid_argument("angular projection");
    }
    if(t>=std::numbers::pi/2)throw std::invalid_argument("forward ray angle required");
    return t;
}
} // namespace book03
