// PROJECTALLMIGHT advanced real-time effects CPU oracle.
// Covers deterministic jitter, TAA neighborhood clipping, simplified screen-space ray marching,
// SSAO-style visibility, Beer-Lambert transmittance, and particle update/kill/compaction.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <span>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }
bool near(double a,double b,double e=1e-9){ return std::fabs(a-b)<=e; }

double radical_inverse(std::uint32_t index,std::uint32_t base){
    require(base>=2,"Halton base must be >= 2");
    double inv=1.0/static_cast<double>(base);
    double factor=inv;
    double value=0.0;
    while(index){
        value+=static_cast<double>(index%base)*factor;
        index/=base;
        factor*=inv;
    }
    return value;
}

struct Jitter { double x{},y{}; };
Jitter halton_jitter(std::uint32_t frame){
    // Start at index one to avoid the all-zero Halton point.
    const std::uint32_t i=frame+1;
    return {radical_inverse(i,2)-0.5,radical_inverse(i,3)-0.5};
}

double clamp_history_3x3(const std::array<double,9>& current,double history){
    const auto [lo_it,hi_it]=std::minmax_element(current.begin(),current.end());
    return std::clamp(history,*lo_it,*hi_it);
}

struct ScreenHit {
    bool hit=false;
    int x=-1;
    double ray_depth=0.0;
    double scene_depth=0.0;
};

// Simplified 1D screen-space reflection oracle. Ray depth is assumed to move monotonically
// away from the camera. A crossing from in-front to behind the sampled depth marks a hit.
ScreenHit march_depth_1d(
    const std::vector<double>& depth,
    int start_x,
    double start_ray_depth,
    double depth_step,
    int pixel_step,
    int max_steps) {

    require(!depth.empty(),"depth buffer must not be empty");
    require(start_x>=0&&start_x<static_cast<int>(depth.size()),"start pixel outside buffer");
    require(depth_step>0,"ray depth must advance positively in this oracle");
    require(pixel_step!=0,"pixel step must not be zero");
    require(max_steps>0,"max steps must be positive");

    double ray=start_ray_depth;
    double previous_delta=ray-depth[static_cast<std::size_t>(start_x)];
    int x=start_x;
    for(int step=0;step<max_steps;++step){
        x+=pixel_step;
        ray+=depth_step;
        if(x<0||x>=static_cast<int>(depth.size())) break;
        const double scene=depth[static_cast<std::size_t>(x)];
        const double delta=ray-scene;
        if(previous_delta<0.0&&delta>=0.0){
            return {true,x,ray,scene};
        }
        previous_delta=delta;
    }
    return {};
}

// SSAO-style binary visibility oracle in tangent space: a sample is occluded when a
// reconstructed scene surface lies closer to the camera than the sample's expected depth,
// with an explicit thickness allowance.
double ssao_visibility(
    std::span<const double> expected_sample_depths,
    std::span<const double> observed_scene_depths,
    double thickness) {

    require(expected_sample_depths.size()==observed_scene_depths.size(),"SSAO arrays must match");
    require(!expected_sample_depths.empty(),"SSAO sample set must not be empty");
    require(thickness>=0,"SSAO thickness must be nonnegative");

    std::size_t visible=0;
    for(std::size_t i=0;i<expected_sample_depths.size();++i){
        const bool occluded=observed_scene_depths[i]+thickness<expected_sample_depths[i];
        if(!occluded) ++visible;
    }
    return static_cast<double>(visible)/static_cast<double>(expected_sample_depths.size());
}

double beer_lambert(double sigma_t,double distance){
    require(sigma_t>=0&&distance>=0,"negative extinction or distance");
    return std::exp(-sigma_t*distance);
}

struct Particle{
    std::uint32_t id{};
    double position{};
    double velocity{};
    double age{};
    double lifetime{};
};

void update_particle(Particle& p,double dt){
    require(dt>=0,"negative particle timestep");
    p.position+=p.velocity*dt;
    p.age+=dt;
}

std::vector<Particle> update_and_compact(std::vector<Particle> particles,double dt){
    for(auto& p:particles) update_particle(p,dt);
    std::vector<Particle> alive;
    alive.reserve(particles.size());
    std::copy_if(
        particles.begin(),particles.end(),std::back_inserter(alive),
        [](const Particle& p){ return p.age<p.lifetime; });
    return alive;
}

void test_halton(){
    const auto j0=halton_jitter(0);
    const auto j1=halton_jitter(1);
    require(near(j0.x,0.0)&&near(j0.y,-1.0/6.0),"first Halton jitter changed");
    require(near(j1.x,-0.25)&&near(j1.y,1.0/6.0),"second Halton jitter changed");

    // 2x3 first-six sequence should cover the unit square strata without repeating a point.
    std::vector<std::pair<double,double>> points;
    for(std::uint32_t i=0;i<6;++i){
        auto j=halton_jitter(i);
        require(j.x>=-0.5&&j.x<0.5&&j.y>=-0.5&&j.y<0.5,"jitter outside pixel");
        points.emplace_back(j.x,j.y);
    }
    std::sort(points.begin(),points.end());
    require(std::adjacent_find(points.begin(),points.end())==points.end(),"Halton jitter repeated");
}

void test_taa_neighborhood(){
    const std::array<double,9> n{0.1,0.2,0.3,0.2,0.25,0.3,0.15,0.2,0.28};
    require(near(clamp_history_3x3(n,2.0),0.3),"bright history must clamp to neighborhood max");
    require(near(clamp_history_3x3(n,-1.0),0.1),"dark history must clamp to neighborhood min");
    require(near(clamp_history_3x3(n,0.23),0.23),"in-range history should remain unchanged");
}

void test_ssr_crossing(){
    // Scene is far until x=5, where an object becomes closer. The ray starts in front,
    // advances deeper, and must cross the scene depth.
    const std::vector<double> depth{10,10,10,10,10,4.0,4.0,4.0};
    const auto h=march_depth_1d(depth,0,1.0,0.75,1,8);
    require(h.hit,"SSR oracle missed depth crossing");
    require(h.x==5,"SSR crossing occurred at unexpected pixel");

    const std::vector<double> no_surface(8,100.0);
    require(!march_depth_1d(no_surface,0,1.0,0.5,1,8).hit,
            "SSR oracle invented a hit with no crossing");
}

void test_ssao(){
    const std::array<double,4> expected{2.0,2.0,2.0,2.0};
    const std::array<double,4> observed{1.0,2.0,3.0,1.95};
    // first sample is clearly occluded; last is protected by 0.1 thickness.
    require(near(ssao_visibility(expected,observed,0.1),0.75),"SSAO visibility fraction wrong");
    require(near(ssao_visibility(expected,expected,0.0),1.0),"matching depths should be visible");
}

void test_volumetric(){
    require(near(beer_lambert(0.0,100.0),1.0),"zero extinction must transmit all light");
    require(near(beer_lambert(2.0,0.0),1.0),"zero distance must transmit all light");
    require(near(beer_lambert(0.5,2.0),std::exp(-1.0)),"Beer-Lambert analytic value wrong");
    require(near(beer_lambert(0.5,4.0),beer_lambert(0.5,2.0)*beer_lambert(0.5,2.0)),
            "transmittance over concatenated homogeneous segments must multiply");
}

void test_particles(){
    std::vector<Particle> p{
        {10,0.0,1.0,0.0,2.0},
        {11,4.0,-2.0,0.5,1.0},
        {12,5.0,0.5,0.9,1.0},
    };
    const auto alive=update_and_compact(p,0.25);
    require(alive.size()==2,"expired particle was not compacted");
    require(alive[0].id==10&&alive[1].id==11,"stable particle compaction order changed");
    require(near(alive[0].position,0.25)&&near(alive[1].position,3.5),
            "particle integration wrong");
    require(near(alive[0].age,0.25)&&near(alive[1].age,0.75),
            "particle age update wrong");
}

} // namespace

int main(){
    try{
        test_halton();
        test_taa_neighborhood();
        test_ssr_crossing();
        test_ssao();
        test_volumetric();
        test_particles();
        std::cout
            <<"realtime_effects_reference_lab: PASS\n"
            <<"proved: deterministic Halton jitter, TAA min/max neighborhood clamp, simplified screen-space depth-crossing march, SSAO-style visibility/thickness logic, Beer-Lambert transmittance, deterministic particle integration/death/stable compaction\n"
            <<"not proved: production TAA variance clipping/reactive masks, 2D hierarchical-Z SSR, GTAO/HBAO, multiple scattering/froxel integration, GPU particle free lists/sort, temporal denoising, or live renderer parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"realtime_effects_reference_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
