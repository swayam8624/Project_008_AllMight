// Seeded property regression for the independently implemented disk event solver.
// This is a controlled isolated-pair experiment, not a general solver proof.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/disk_event_property_lab.cpp -o /tmp/disk_property && /tmp/disk_property
#define ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include "disk_event_pipeline_lab.cpp"
#undef ALLMIGHT_EVENT_PIPELINE_NO_MAIN
#include <random>

int main() {
    // Fixed seed makes every failing case reproducible on the same toolchain.
    std::mt19937_64 rng(0xA11A11u);
    std::uniform_real_distribution<double> initialSeparation(3.0,15.0);
    std::uniform_real_distribution<double> closingSpeed(0.2,15.0);
    std::uniform_real_distribution<double> mass(0.2,8.0);
    std::uniform_real_distribution<double> restitution(0.0,1.0);
    constexpr int trialCount=10000;
    for(int trial=0;trial<trialCount;++trial) {
        const double separation=initialSeparation(rng);
        const double speed=closingSpeed(rng);
        const double massA=mass(rng),massB=mass(rng);
        const double e=restitution(rng);
        // The first disk approaches the second on the x axis.
        std::vector<Disk> disks{
            {1,{0,0},{speed,0},1,1/massA},
            {2,{separation,0},{0,0},1,1/massB}
        };
        const double duration=(separation-2)/speed+0.1;
        const Vec2 initialMomentum=momentum(disks);
        const double initialEnergy=energy(disks);
        Result result{};
        try {
            result=simulate(disks,duration,e);
        } catch(const std::exception& ex) {
            std::cerr<<"trial "<<trial<<" separation "<<separation
                     <<" speed "<<speed<<" massA "<<massA
                     <<" massB "<<massB<<" restitution "<<e
                     <<" exception "<<ex.what()<<'\n';
            throw;
        }
        check(result.events==1,"expected one isolated collision");
        const double outgoingSpeed=disks[1].velocity.x-disks[0].velocity.x;
        // Newton restitution: outgoing separation speed equals e times incoming closing speed.
        check(std::abs(outgoingSpeed-e*speed)<=1e-8*std::max(1.0,speed),
              "restitution property failed");
        const double finalMomentum=momentum(disks).x;
        check(std::abs(finalMomentum-initialMomentum.x)<=
              1e-8*std::max(1.0,std::abs(initialMomentum.x)),
              "linear momentum property failed");
        check(energy(disks)<=initialEnergy+1e-8*std::max(1.0,initialEnergy),
              "energy unexpectedly increased");
    }
    std::cout<<"disk event properties: 10000 seeded isolated collisions PASS\n";
}
