// Ray/path-transport reference lab: robust sphere intersection, cosine-weighted
// hemisphere sampling, Lambertian BRDF/PDF cancellation and Russian roulette
// unbiasedness. This is deliberately tiny and inspectable.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

namespace allmight {

constexpr double pi=3.1415926535897932384626433832795;

struct V3{double x{},y{},z{};};
V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V3 operator*(V3 a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double length(V3 v){return std::sqrt(dot(v,v));}
V3 normalize(V3 v){const double l=length(v); if(!(l>1e-15)) throw std::invalid_argument("zero vector"); return v*(1.0/l);}

struct Ray{V3 o;V3 d;};
struct Sphere{V3 c;double r;};

bool sphere_hit(const Ray& ray,const Sphere& s,double t_min,double t_max,double& t){
    const V3 oc=ray.o-s.c;
    const double a=dot(ray.d,ray.d);
    const double half_b=dot(oc,ray.d);
    const double c=dot(oc,oc)-s.r*s.r;
    const double disc=half_b*half_b-a*c;
    if(disc<0.0) return false;
    const double root=std::sqrt(disc);
    double candidate=(-half_b-root)/a;
    if(candidate<t_min||candidate>t_max){
        candidate=(-half_b+root)/a;
        if(candidate<t_min||candidate>t_max) return false;
    }
    t=candidate;
    return true;
}

struct Basis{V3 t,b,n;};
Basis basis_from_normal(V3 n){
    n=normalize(n);
    const V3 helper=std::abs(n.z)<0.999?V3{0,0,1}:V3{1,0,0};
    const V3 t=normalize(cross(helper,n));
    return {t,cross(n,t),n};
}
V3 local_to_world(const Basis& f,V3 v){return f.t*v.x+f.b*v.y+f.n*v.z;}

struct CosineSample{V3 direction;double pdf;};
CosineSample cosine_hemisphere(V3 n,double u1,double u2){
    if(u1<0||u1>=1||u2<0||u2>=1) throw std::invalid_argument("sample outside [0,1)");
    const double r=std::sqrt(u1);
    const double phi=2*pi*u2;
    const double x=r*std::cos(phi), y=r*std::sin(phi);
    const double z=std::sqrt(std::max(0.0,1.0-u1));
    const V3 w=normalize(local_to_world(basis_from_normal(n),{x,y,z}));
    const double cos_theta=std::max(0.0,dot(normalize(n),w));
    return {w,cos_theta/pi};
}

double lambert_brdf(double albedo){
    if(albedo<0||albedo>1) throw std::invalid_argument("albedo");
    return albedo/pi;
}

// Under constant incident radiance over the hemisphere, cosine sampling makes
// f*cos/pdf exactly equal to albedo, so this is a strong algebraic oracle.
double one_bounce_constant_environment(double albedo,double env,V3 n,double u1,double u2){
    const auto s=cosine_hemisphere(n,u1,u2);
    const double cos_theta=std::max(0.0,dot(normalize(n),s.direction));
    if(!(s.pdf>0.0)) return 0.0;
    return env*lambert_brdf(albedo)*cos_theta/s.pdf;
}

// Simulate a scalar multiplicative path with roulette after each attenuation.
// Without roulette, contribution after depth d is value * attenuation^d.
double roulette_chain(std::mt19937& rng,double value,double attenuation,int depth,double survival){
    if(depth<0||attenuation<0||survival<=0||survival>1) throw std::invalid_argument("roulette args");
    std::uniform_real_distribution<double> u(0.0,1.0);
    double beta=value;
    for(int i=0;i<depth;++i){
        beta*=attenuation;
        if(u(rng)>survival) return 0.0;
        beta/=survival;
    }
    return beta;
}

void tests(){
    {
        double t=0;
        const Ray r{{0,0,-5},normalize({0,0,1})};
        assert(sphere_hit(r,{{0,0,0},1.0},0.001,100.0,t));
        assert(std::abs(t-4.0)<1e-12);
        const Ray miss{{0,0,-5},normalize({0,1,0})};
        assert(!sphere_hit(miss,{{0,0,0},1.0},0.001,100.0,t));
    }

    {
        const double expected=0.65*3.0;
        for(int i=0;i<1000;++i){
            const double u1=(i+0.25)/1001.0;
            const double u2=(i*37%1000+0.5)/1000.0;
            const double estimate=one_bounce_constant_environment(0.65,3.0,{0,0,1},u1,u2);
            assert(std::abs(estimate-expected)<1e-10);
        }
    }

    {
        std::mt19937 rng(7U);
        constexpr int samples=400000;
        constexpr int depth=5;
        const double attenuation=0.8;
        const double expected=std::pow(attenuation,depth);
        double mean=0.0;
        for(int i=0;i<samples;++i) mean+=roulette_chain(rng,1.0,attenuation,depth,0.7);
        mean/=samples;
        assert(std::abs(mean-expected)<0.006);
    }
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"ray_path_reference_lab: PASS\n";
    return 0;
}
