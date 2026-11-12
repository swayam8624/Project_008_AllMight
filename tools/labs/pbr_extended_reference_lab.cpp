// PROJECTALLMIGHT extended PBR oracle: anisotropic GGX normalization, dielectric Fresnel/refraction, furnace integrals.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ double x{},y{},z{}; };
V3 operator+(V3 a,V3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
double len(V3 a){ return std::sqrt(dot(a,a)); }
V3 norm(V3 a){ const double l=len(a); require(l>0,"zero vector"); return a*(1.0/l); }

double ggx_aniso_D(V3 m,double ax,double ay){
    if(m.z<=0) return 0;
    require(ax>0&&ay>0,"roughness axes must be positive");
    const double q=(m.x*m.x)/(ax*ax)+(m.y*m.y)/(ay*ay)+m.z*m.z;
    return 1.0/(std::numbers::pi*ax*ay*q*q);
}

double fresnel_dielectric(double cos_i,double eta_i,double eta_t){
    require(eta_i>0&&eta_t>0,"IOR must be positive");
    cos_i=std::clamp(cos_i,-1.0,1.0);
    bool entering=cos_i>=0;
    if(!entering){ std::swap(eta_i,eta_t); cos_i=std::fabs(cos_i); }
    const double sin_i=std::sqrt(std::max(0.0,1.0-cos_i*cos_i));
    const double sin_t=eta_i/eta_t*sin_i;
    if(sin_t>=1.0) return 1.0;
    const double cos_t=std::sqrt(std::max(0.0,1.0-sin_t*sin_t));
    const double rpar=((eta_t*cos_i)-(eta_i*cos_t))/((eta_t*cos_i)+(eta_i*cos_t));
    const double rper=((eta_i*cos_i)-(eta_t*cos_t))/((eta_i*cos_i)+(eta_t*cos_t));
    return 0.5*(rpar*rpar+rper*rper);
}

bool refract(V3 wi,V3 n,double eta_i,double eta_t,V3& wt){
    wi=norm(wi); n=norm(n);
    double cos_i=dot(wi,n);
    if(cos_i<0){ n=n*(-1.0); cos_i=-cos_i; }
    const double eta=eta_i/eta_t;
    const double sin2_t=eta*eta*std::max(0.0,1.0-cos_i*cos_i);
    if(sin2_t>=1.0) return false;
    const double cos_t=std::sqrt(1.0-sin2_t);
    // wi points toward the interface; transmitted direction continues through it.
    wt=norm(wi*(-eta)+n*(eta*cos_i-cos_t));
    return true;
}

double lambert_furnace(double albedo){
    const int nt=1000;
    const int np=32;
    double sum=0;
    const double dt=(0.5*std::numbers::pi)/nt;
    const double dp=(2.0*std::numbers::pi)/np;
    for(int t=0;t<nt;++t){
        const double theta=(t+0.5)*dt;
        const double c=std::cos(theta),s=std::sin(theta);
        for(int p=0;p<np;++p){ (void)p; sum+=(albedo/std::numbers::pi)*c*s*dt*dp; }
    }
    return sum;
}

void test_anisotropic_normalization(){
    const int nt=900,np=128;
    const double dt=(0.5*std::numbers::pi)/nt,dp=(2.0*std::numbers::pi)/np;
    for(const auto [ax,ay]: {std::pair{0.2,0.7},std::pair{0.5,0.5},std::pair{0.9,0.3}}){
        double sum=0;
        for(int t=0;t<nt;++t){
            const double th=(t+0.5)*dt,c=std::cos(th),sn=std::sin(th);
            for(int p=0;p<np;++p){
                const double ph=(p+0.5)*dp;
                const V3 m{sn*std::cos(ph),sn*std::sin(ph),c};
                sum+=ggx_aniso_D(m,ax,ay)*c*sn*dt*dp;
            }
        }
        require(std::fabs(sum-1.0)<0.006,"anisotropic GGX projected normalization drift");
    }
}

void test_dielectric_limits_and_tir(){
    const double f0=std::pow((1.0-1.5)/(1.0+1.5),2);
    require(std::fabs(fresnel_dielectric(1.0,1.0,1.5)-f0)<1e-12,"dielectric normal-incidence Fresnel wrong");
    require(fresnel_dielectric(1e-7,1.0,1.5)>0.999,"grazing dielectric Fresnel should approach one");

    V3 wt{};
    require(refract({0,0,1},{0,0,1},1.0,1.5,wt),"normal-incidence air-to-glass should refract");
    require(std::fabs(std::fabs(wt.z)-1.0)<1e-12,"normal-incidence refraction direction wrong");
    const V3 grazing=norm({0.9,0,0.435889894});
    require(!refract(grazing,{0,0,1},1.5,1.0,wt),"glass-to-air angle above critical should total-internally reflect");
}

void test_white_furnace(){
    for(double a:{0.0,0.2,0.7,1.0}){
        require(std::fabs(lambert_furnace(a)-a)<3e-5,"Lambertian white-furnace integral must equal albedo");
    }
}

} // namespace

int main(){
    try{
        test_anisotropic_normalization();
        test_dielectric_limits_and_tir();
        test_white_furnace();
        std::cout<<"pbr_extended_reference_lab: PASS\n"
                 <<"proved: anisotropic GGX projected normalization, exact dielectric Fresnel limits, Snell refraction/TIR, Lambertian white-furnace energy integral\n"
                 <<"not proved: microfacet multiple scattering, rough transmission BTDF, spectral IOR, measured BRDF fitting, texture/material import, or renderer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"pbr_extended_reference_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
