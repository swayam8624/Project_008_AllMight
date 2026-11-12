// PROJECTALLMIGHT tiny scene-transport reference.
// Connects camera rays, geometry intersection, diffuse transport, two sampling techniques and MIS to a pixel estimate.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct V3{ double x{},y{},z{}; };
V3 operator+(V3 a,V3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator-(V3 a,V3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
double length(V3 v){ return std::sqrt(dot(v,v)); }
V3 normalize(V3 v){ const double l=length(v); require(l>0,"zero vector"); return v*(1.0/l); }

struct Ray{ V3 o,d; };
struct Sphere{ V3 c; double r; double albedo; };

bool hit(const Sphere& s,const Ray& ray,double& t,V3& n){
    const V3 oc=ray.o-s.c;
    const double b=dot(oc,ray.d);
    const double c=dot(oc,oc)-s.r*s.r;
    const double disc=b*b-c;
    if(disc<0) return false;
    const double root=std::sqrt(disc);
    double candidate=-b-root;
    if(candidate<1e-6) candidate=-b+root;
    if(candidate<1e-6) return false;
    t=candidate;
    const V3 p=ray.o+ray.d*t;
    n=normalize(p-s.c);
    return true;
}

struct Rng{
    std::uint64_t s;
    explicit Rng(std::uint64_t seed):s(seed?seed:1){}
    std::uint32_t u32(){ s^=s>>12; s^=s<<25; s^=s>>27; return static_cast<std::uint32_t>((s*2685821657736338717ULL)>>32); }
    double u(){ return (static_cast<double>(u32())+0.5)/4294967296.0; }
};

V3 basis_tangent(V3 n){
    return normalize(std::fabs(n.z)<0.9 ? V3{-n.y,n.x,0.0} : V3{0.0,-n.z,n.y});
}
V3 cosine_sample_world(V3 n,Rng& rng){
    const double u1=rng.u(),u2=rng.u();
    const double r=std::sqrt(u1),phi=2.0*std::numbers::pi*u2;
    const double x=r*std::cos(phi),y=r*std::sin(phi),z=std::sqrt(1.0-u1);
    const V3 t=basis_tangent(n);
    const V3 b{n.y*t.z-n.z*t.y,n.z*t.x-n.x*t.z,n.x*t.y-n.y*t.x};
    return normalize(t*x+b*y+n*z);
}
V3 uniform_hemi_world(V3 n,Rng& rng){
    const double z=rng.u();
    const double phi=2.0*std::numbers::pi*rng.u();
    const double r=std::sqrt(std::max(0.0,1.0-z*z));
    const V3 t=basis_tangent(n);
    const V3 b{n.y*t.z-n.z*t.y,n.z*t.x-n.x*t.z,n.x*t.y-n.y*t.x};
    return normalize(t*(r*std::cos(phi))+b*(r*std::sin(phi))+n*z);
}

double balance(double p_this,double p_other){ return p_this/(p_this+p_other); }

// One-bounce Lambertian under constant environment. The exact outgoing radiance is albedo * env.
// We intentionally estimate the same integral using both light-uniform and BSDF-cosine strategies with balance MIS.
double estimate_pixel(const Sphere& sphere,const Ray& camera,int samples,std::uint64_t seed,double env){
    double t=0; V3 n{};
    if(!hit(sphere,camera,t,n)) return env;
    Rng rng(seed);
    double sum=0.0;
    const double f=sphere.albedo/std::numbers::pi;
    for(int i=0;i<samples;++i){
        // Technique A: uniform hemisphere p=1/(2pi)
        const V3 wa=uniform_hemi_world(n,rng);
        const double ca=std::max(0.0,dot(n,wa));
        const double pa=1.0/(2.0*std::numbers::pi);
        const double pba=ca/std::numbers::pi;
        sum += env*f*ca/pa*balance(pa,pba);

        // Technique B: cosine hemisphere p=cos/pi
        const V3 wb=cosine_sample_world(n,rng);
        const double cb=std::max(0.0,dot(n,wb));
        const double pb=cb/std::numbers::pi;
        const double plb=1.0/(2.0*std::numbers::pi);
        sum += env*f*cb/pb*balance(pb,plb);
    }
    return sum/static_cast<double>(samples);
}

void test_geometry(){
    Sphere s{{0,0,0},1.0,0.7};
    double t=0; V3 n{};
    require(hit(s,{{0,0,3},{0,0,-1}},t,n),"center camera ray missed sphere");
    require(std::fabs(t-2.0)<1e-12,"sphere hit distance wrong");
    require(std::fabs(n.z-1.0)<1e-12,"sphere normal wrong");
    require(!hit(s,{{0,0,3},{1,0,0}},t,n),"clear miss reported as hit");
}

void test_scene_transport(){
    const Sphere s{{0,0,0},1.0,0.62};
    const Ray camera{{0,0,3},{0,0,-1}};
    const double env=2.4;
    const double exact=s.albedo*env;
    const double est=estimate_pixel(s,camera,250000,20261112,env);
    require(std::fabs(est-exact)<0.01,"scene MIS estimate did not converge to Lambertian constant-environment solution");

    // A miss sees the environment directly.
    const double miss=estimate_pixel(s,{{0,0,3},normalize({1,0,0})},100,1,env);
    require(std::fabs(miss-env)<1e-12,"environment miss path wrong");
}

} // namespace

int main(){
    try{
        test_geometry();
        test_scene_transport();
        std::cout
            <<"path_transport_scene_lab: PASS\n"
            <<"proved: camera ray/sphere intersection, shading normal, Lambertian throughput, uniform-light and cosine-BSDF sampling, balance MIS, pixel convergence to an analytic constant-environment result\n"
            <<"not proved: emissive-geometry NEE, multi-bounce recursion, BVH traversal, dielectric/specular transport, participating media, spectral paths, or rendered-image parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"path_transport_scene_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
