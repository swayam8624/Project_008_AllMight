// Compare exhaustive and swept candidate generation through the SAME event solver.
// This tests source-level behavior but does not replace native Kairo integration.
// c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/disk_event_broadphase_differential_lab.cpp -o /tmp/disk_diff && /tmp/disk_diff
#define ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include "disk_event_pipeline_lab.cpp"
#undef ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include <random>
#include <string>

void compare_world(const std::vector<Disk>& initial,double h,double e,const char* name) {
    auto exhaustive=initial,swept=initial;
    const auto reference=simulate(exhaustive,h,e,256,false);
    const auto accelerated=simulate(swept,h,e,256,true);
    if(reference.events!=accelerated.events || reference.batches!=accelerated.batches) {
        std::cerr<<name<<": event/batch mismatch\n";
        throw std::runtime_error("broadphase changed event sequence");
    }
    near(reference.simulated,accelerated.simulated,1e-9);
    check(exhaustive.size()==swept.size(),"body count changed");
    for(std::size_t i=0;i<exhaustive.size();++i) {
        check(exhaustive[i].id==swept[i].id,"body identity changed");
        near(exhaustive[i].centre.x,swept[i].centre.x,1e-8);
        near(exhaustive[i].centre.y,swept[i].centre.y,1e-8);
        near(exhaustive[i].velocity.x,swept[i].velocity.x,1e-8);
        near(exhaustive[i].velocity.y,swept[i].velocity.y,1e-8);
    }
}
int main() {
    // Disjoint, close, high-speed and simultaneous cases exercise ordering as well as pairing.
    compare_world({{1,{0,0},{0,0},1,0},{2,{10,0},{-100,0},1,1}},0.1,1,"wall");
    compare_world({{1,{-4,0},{1,0},1,1},{2,{0,0},{0,0},1,1},
                   {3,{4,0},{-1,0},1,1}},3,1,"simultaneous");
    compare_world({{1,{-6,0},{4,0},1,1},{2,{0,0},{0,0},1,1},
                   {3,{6,0},{0,0},1,1}},3,1,"chained");
    compare_world({{1,{0,0},{0,0},1,1},{2,{30,30},{0,0},1,1}},0.5,0,"disjoint");

    std::mt19937_64 generator(0xA118A11u);
    std::uniform_real_distribution<double> separation(3.0,18.0);
    std::uniform_real_distribution<double> speed(0.2,20.0);
    std::uniform_real_distribution<double> mass(0.3,6.0);
    std::uniform_real_distribution<double> restitution(0,1);
    for(int trial=0;trial<2000;++trial) {
        const double d=separation(generator),v=speed(generator);
        // An isolated one-dimensional impact is guaranteed and numerically checkable.
        const std::vector<Disk> world{
            {1,{0,0},{v,0},1,1/mass(generator)},
            {2,{d,0},{0,0},1,1/mass(generator)}
        };
        compare_world(world,(d-2)/v+0.1,restitution(generator),"seeded pair");
    }
    std::cout<<"event broadphase differential: 4 scenes + 2000 seeded pairs PASS\n";
}
