// PROJECTALLMIGHT microfacet reference: GGX normalization, Schlick Fresnel, Smith masking, reciprocity.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }
bool near(float a,float b,float e){ return std::fabs(a-b)<=e; }

struct V3{ double x=0,y=0,z=0; };
V3 operator+(V3 a,V3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
double len(V3 a){ return std::sqrt(dot(a,a)); }
V3 norm(V3 a){ const double l=len(a); require(l>0,"zero vector"); return a*(1.0/l); }

double ggx_D(double cos_m,double alpha){
    if(cos_m<=0.0) return 0.0;
    const double a2=alpha*alpha;
    const double c2=cos_m*cos_m;
    const double denom=c2*(a2-1.0)+1.0;
    return a2/(std::numbers::pi*denom*denom);
}

double smith_G1(double cos_v,double alpha){
    if(cos_v<=0.0) return 0.0;
    const double c2=cos_v*cos_v;
    const double tan2=(1.0-c2)/c2;
    return 2.0/(1.0+std::sqrt(1.0+alpha*alpha*tan2));
}

double schlick(double cos_theta,double f0){
    const double m=std::clamp(1.0-cos_theta,0.0,1.0);
    const double m2=m*m;
    return f0+(1.0-f0)*m2*m2*m;
}

double microfacet_brdf(V3 wi,V3 wo,double alpha,double f0){
    const double ni=wi.z, no=wo.z;
    if(ni<=0||no<=0) return 0;
    const V3 h=norm(wi+wo);
    const double D=ggx_D(h.z,alpha);
    const double G=smith_G1(ni,alpha)*smith_G1(no,alpha);
    const double F=schlick(std::clamp(dot(wi,h),0.0,1.0),f0);
    return D*G*F/(4.0*ni*no);
}

void test_ndf_projected_normalization(){
    // Midpoint quadrature over hemisphere: integral D(m) cos(theta) d omega = 1.
    for(double alpha: {0.2,0.5,1.0}){
        const int nTheta=800;
        const int nPhi=8; // isotropic D, phi repetition only supplies 2pi measure.
        double sum=0;
        const double dTheta=(0.5*std::numbers::pi)/nTheta;
        const double dPhi=(2.0*std::numbers::pi)/nPhi;
        for(int t=0;t<nTheta;++t){
            const double theta=(t+0.5)*dTheta;
            const double c=std::cos(theta), s=std::sin(theta);
            for(int p=0;p<nPhi;++p){
                (void)p;
                sum += ggx_D(c,alpha)*c*s*dTheta*dPhi;
            }
        }
        require(std::fabs(sum-1.0)<3e-3,"GGX projected-area normalization drift");
    }
}

void test_fresnel_limits(){
    require(near(schlick(1.0,0.04),0.04,1e-12),"Schlick normal-incidence value must equal F0");
    require(schlick(0.0,0.04)>0.999999,"Schlick grazing value should approach one");
}

void test_reciprocity_and_finiteness(){
    const V3 wi=norm({0.3,0.1,1.0});
    const V3 wo=norm({-0.2,0.4,1.0});
    for(double alpha:{0.05,0.2,0.7,1.0}){
        const double a=microfacet_brdf(wi,wo,alpha,0.04);
        const double b=microfacet_brdf(wo,wi,alpha,0.04);
        require(std::isfinite(a)&&a>=0,"BRDF must be finite nonnegative for valid directions");
        require(std::fabs(a-b)<1e-12,"isotropic microfacet BRDF reciprocity failed");
    }
}

} // namespace

int main(){
    try{
        test_ndf_projected_normalization();
        test_fresnel_limits();
        test_reciprocity_and_finiteness();
        std::cout<<"pbr_reference_lab: PASS\n"
                 <<"proved: GGX projected normalization, Schlick endpoint behavior, Smith-based isotropic BRDF reciprocity/finiteness\n"
                 <<"not proved: full white-furnace energy conservation, transmission, anisotropy, measured-material parity, texture decoding, or glTF renderer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"pbr_reference_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
