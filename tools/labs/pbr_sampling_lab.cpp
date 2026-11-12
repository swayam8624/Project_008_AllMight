// PBR and sampling reference lab: GGX D/G/F evaluation, deterministic
// hemispherical energy sanity, stable PDFs and a simple MIS integration oracle.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

namespace allmight {

constexpr double pi = 3.1415926535897932384626433832795;

struct V3 { double x{},y{},z{}; };

V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator*(V3 a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
double length(V3 v){return std::sqrt(dot(v,v));}
V3 normalize(V3 v){
    const double l=length(v);
    if(!(l>0.0)) throw std::invalid_argument("zero direction");
    return v*(1.0/l);
}

double schlick_fresnel(double cos_theta,double f0){
    const double c=std::clamp(cos_theta,0.0,1.0);
    const double m=1.0-c;
    const double m2=m*m;
    return f0+(1.0-f0)*m2*m2*m;
}

double ggx_D(double n_dot_h,double alpha){
    const double ndh=std::clamp(n_dot_h,0.0,1.0);
    const double a=std::max(alpha,1.0e-4);
    const double a2=a*a;
    const double d=ndh*ndh*(a2-1.0)+1.0;
    return a2/(pi*d*d);
}

double smith_G1(double n_dot_v,double alpha){
    const double ndv=std::clamp(n_dot_v,0.0,1.0);
    if(ndv<=0.0) return 0.0;
    const double a=std::max(alpha,1.0e-4);
    const double sin2=std::max(0.0,1.0-ndv*ndv);
    const double tan2=sin2/(ndv*ndv);
    return 2.0/(1.0+std::sqrt(1.0+a*a*tan2));
}

double ggx_brdf(V3 n,V3 v,V3 l,double alpha,double f0){
    const double nv=dot(n,v);
    const double nl=dot(n,l);
    if(nv<=0.0 || nl<=0.0) return 0.0;
    const V3 h=normalize(v+l);
    const double nh=std::max(0.0,dot(n,h));
    const double vh=std::max(0.0,dot(v,h));
    const double D=ggx_D(nh,alpha);
    const double G=smith_G1(nv,alpha)*smith_G1(nl,alpha);
    const double F=schlick_fresnel(vh,f0);
    return D*G*F/(4.0*nv*nl);
}

double integrate_reflected_energy(double alpha,double f0){
    const V3 n{0.0,0.0,1.0};
    const V3 v{0.0,0.0,1.0};
    constexpr int theta_n=240;
    constexpr int phi_n=480;
    double sum=0.0;
    for(int ti=0;ti<theta_n;++ti){
        const double theta=(static_cast<double>(ti)+0.5)*(0.5*pi/theta_n);
        const double st=std::sin(theta), ct=std::cos(theta);
        for(int pi_i=0;pi_i<phi_n;++pi_i){
            const double phi=(static_cast<double>(pi_i)+0.5)*(2.0*pi/phi_n);
            const V3 l{st*std::cos(phi),st*std::sin(phi),ct};
            const double domega=st*(0.5*pi/theta_n)*(2.0*pi/phi_n);
            sum += ggx_brdf(n,v,l,alpha,f0)*ct*domega;
        }
    }
    return sum;
}

// Integrand chosen because its exact integral over [0,1] is 1/3.
double f(double x){ return x*x; }

// Proposal p1(x)=1 uniform. Proposal p2(x)=2x for x in [0,1].
// We draw one sample from each and combine with the balance heuristic.
double mis_estimate(std::mt19937& rng){
    std::uniform_real_distribution<double> u(0.0,1.0);
    const double x1=u(rng);
    const double p1_x1=1.0;
    const double p2_x1=2.0*x1;
    const double w1=p1_x1/(p1_x1+p2_x1);
    const double c1=w1*f(x1)/p1_x1;

    const double u2=u(rng);
    const double x2=std::sqrt(u2); // inverse CDF of p2=2x
    const double p1_x2=1.0;
    const double p2_x2=2.0*x2;
    const double w2=p2_x2/(p1_x2+p2_x2);
    const double c2=w2*f(x2)/p2_x2;

    // One sample per technique; the sum of weighted technique estimators is
    // unbiased because weights sum to one pointwise.
    return c1+c2;
}

void tests(){
    for(double alpha:{0.05,0.2,0.5,1.0}){
        for(double ndh:{0.01,0.2,0.5,0.9,1.0}){
            const double d=ggx_D(ndh,alpha);
            assert(std::isfinite(d) && d>=0.0);
        }
    }

    assert(std::abs(schlick_fresnel(1.0,0.04)-0.04)<1.0e-12);
    assert(std::abs(schlick_fresnel(0.0,0.04)-1.0)<1.0e-12);

    for(double alpha:{0.1,0.3,0.6,1.0}){
        const double e=integrate_reflected_energy(alpha,0.04);
        assert(std::isfinite(e));
        assert(e>=0.0);
        assert(e<1.01); // single-scatter dielectric specular should not create energy here.
    }

    std::mt19937 rng(42U);
    constexpr int n=200000;
    double mean=0.0;
    for(int i=0;i<n;++i) mean+=mis_estimate(rng);
    mean/=static_cast<double>(n);
    assert(std::abs(mean-(1.0/3.0))<0.003);
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"pbr_sampling_lab: PASS\n";
    return 0;
}
