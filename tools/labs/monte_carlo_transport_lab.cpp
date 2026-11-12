// PROJECTALLMIGHT Monte Carlo transport reference: cosine sampling, throughput, and Russian roulette expectation.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

struct Rng{
    std::uint64_t state=0x9e3779b97f4a7c15ULL;
    explicit Rng(std::uint64_t seed):state(seed?seed:1){}
    std::uint32_t next_u32(){
        state ^= state>>12; state ^= state<<25; state ^= state>>27;
        return static_cast<std::uint32_t>((state*2685821657736338717ULL)>>32);
    }
    double uniform(){
        return (static_cast<double>(next_u32())+0.5)/4294967296.0;
    }
};

struct V3{ double x=0,y=0,z=0; };

V3 cosine_sample(Rng& rng){
    const double u1=rng.uniform();
    const double u2=rng.uniform();
    const double r=std::sqrt(u1);
    const double phi=2.0*std::numbers::pi*u2;
    const double x=r*std::cos(phi);
    const double y=r*std::sin(phi);
    const double z=std::sqrt(std::max(0.0,1.0-u1));
    return {x,y,z};
}

double cosine_pdf(V3 w){
    return w.z>0 ? w.z/std::numbers::pi : 0.0;
}

void test_cosine_sampling_moments(){
    Rng rng(1234);
    const int n=300000;
    double mean_z=0;
    for(int i=0;i<n;++i) mean_z+=cosine_sample(rng).z;
    mean_z/=n;
    // For cosine-weighted hemisphere p=cos(theta)/pi, E[cos(theta)] = 2/3.
    require(std::fabs(mean_z-2.0/3.0)<0.004,"cosine sampler moment disagrees with analytic expectation");
}

void test_lambertian_environment_estimator(){
    Rng rng(9876);
    const double albedo=0.63;
    const double environment=2.5;
    const int n=100000;
    double estimate=0;

    for(int i=0;i<n;++i){
        const V3 wi=cosine_sample(rng);
        const double f=albedo/std::numbers::pi;
        const double pdf=cosine_pdf(wi);
        // Rendering equation estimator: L_i * f * cos(theta) / pdf.
        estimate += environment*f*wi.z/pdf;
    }
    estimate/=n;
    const double exact=albedo*environment;
    require(std::fabs(estimate-exact)<1e-10,
            "cosine-sampled Lambertian constant-environment estimator should cancel exactly");
}

void test_russian_roulette_expectation(){
    Rng rng(42);
    const double x=3.7;
    const double survival=0.31;
    const int n=1000000;
    double sum=0;
    for(int i=0;i<n;++i){
        if(rng.uniform()<survival) sum += x/survival;
    }
    const double mean=sum/n;
    require(std::fabs(mean-x)<0.025,"Russian roulette compensation changed expectation");
}

double balance_weight(double pa,double pb){
    require(pa>=0&&pb>=0&&pa+pb>0,"invalid MIS PDFs");
    return pa/(pa+pb);
}

void test_mis_partition_of_weight(){
    for(const auto [a,b]: std::vector<std::pair<double,double>>{{1,2},{0.1,4},{9,0.5}}){
        const double wa=balance_weight(a,b);
        const double wb=balance_weight(b,a);
        require(std::fabs((wa+wb)-1.0)<1e-12,"balance weights for same sample do not partition unity");
    }
}

} // namespace

int main(){
    try{
        test_cosine_sampling_moments();
        test_lambertian_environment_estimator();
        test_russian_roulette_expectation();
        test_mis_partition_of_weight();
        std::cout<<"monte_carlo_transport_lab: PASS\n"
                 <<"proved: cosine-hemisphere sampling moment, Lambertian throughput/PDF cancellation, roulette expectation compensation, balance-weight partition\n"
                 <<"not proved: scene geometry, BVH traversal, next-event light sampling, full path MIS, participating media, spectral transport, or renderer image parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"monte_carlo_transport_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
