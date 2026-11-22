// Differential test: swept candidate event mode must agree with exhaustive pair mode.
// The implementation under test is included with its standalone main disabled.
#define ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include "disk_event_pipeline_lab.cpp"
#undef ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include <random>
#include <string>

void equivalent(std::vector<Disk> input,double duration,double restitution) {
    auto reference=input, accelerated=input;
    const Result a=simulate(reference,duration,restitution,256,false);
    const Result b=simulate(accelerated,duration,restitution,256,true);
    check(a.events==b.events,"different collision event counts");
    check(a.batches==b.batches,"different simultaneous event batches");
    near(a.simulated,b.simulated,1e-9);
    check(reference.size()==accelerated.size(),"different world object counts");
    for(std::size_t i=0;i<reference.size();++i) {
        const Disk& x=reference[i],&y=accelerated[i];
        check(x.id==y.id,"body identity changed");
        near(x.centre.x,y.centre.x,1e-8);
        near(x.centre.y,y.centre.y,1e-8);
        near(x.velocity.x,y.velocity.x,1e-8);
        near(x.velocity.y,y.velocity.y,1e-8);
    }
}
int main() {
    // Analytical high-speed wall rebound: both modes must simulate the remainder.
    equivalent({{1,{0,0},{0,0},1,0},{2,{10,0},{-100,0},1,1}},0.1,1);
    // Two concurrent events share the middle body. Candidate ordering matters.
    equivalent({{1,{-4,0},{1,0},1,1},{2,{0,0},{0,0},1,1},
                {3,{4,0},{-1,0},1,1}},3,1);
    // The second event is caused by the first impact, so bounds must be rebuilt.
    equivalent({{1,{-6,0},{4,0},1,1},{2,{0,0},{0,0},1,1},
                {3,{6,0},{0,0},1,1}},3,1);
    equivalent({{1,{-3,0},{2,0},1,1},{2,{3,0},{0,0},1,1}},2,0);

    std::mt19937_64 rng(0xC0111510u);
    std::uniform_real_distribution<double> speed(10,20),mass(0.2,4);
    std::uniform_real_distribution<double> restitution(0,1),slow(-1,1);
    std::uniform_real_distribution<double> offset(-100,100);
    constexpr int trials=1000;
    for(int trial=0;trial<trials;++trial) {
        const double shift=offset(rng),incoming=speed(rng);
        std::vector<Disk> world{
            {1,{shift,0},{incoming,0},1,1/mass(rng)},
            {2,{shift+5,0},{0,0},1,1/mass(rng)}
        };
        // More bodies increase broadphase selectivity without unwanted initial overlap.
        for(std::uint64_t id=3;id<=12;++id)
            world.push_back({id,{shift+30+12*(id-3),0},{slow(rng),0},1,1/mass(rng)});
        try {
            equivalent(world,0.35,restitution(rng));
        } catch(const std::exception& error) {
            std::cerr<<"differential trial "<<trial<<" failed: "<<error.what()<<'\n';
            throw;
        }
    }
    std::cout<<"swept event integration: 4 fixed + 1000 seeded equivalent scenes PASS\n";
}
