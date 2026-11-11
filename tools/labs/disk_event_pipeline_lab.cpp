// Independent C++20 disk event-pipeline laboratory, not the Kairo production solver.
// Units: metres, seconds, kilograms; 2D non-spinning disks moving at constant
// velocity between instantaneous, frictionless, normal impulses.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror disk_event_pipeline_lab.cpp -o /tmp/disk_events && /tmp/disk_events
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

struct Vec2 { double x{}, y{}; };
constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x+b.x,a.y+b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x-b.x,a.y-b.y}; }
constexpr Vec2 operator*(Vec2 a, double s) { return {a.x*s,a.y*s}; }
constexpr double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }
void check(bool b, const char* msg) { if (!b) throw std::runtime_error(msg); }
void near(double a,double b,double epsilon=1e-8) {
    check(std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=epsilon,"numerical invariant failed");
}
struct Disk {
    std::uint64_t id{};
    Vec2 centre{},velocity{};
    double radius{}, inverseMass{};
};
void validate(const Disk& d) {
    check(std::isfinite(d.centre.x)&&std::isfinite(d.centre.y)&&
          std::isfinite(d.velocity.x)&&std::isfinite(d.velocity.y)&&
          std::isfinite(d.radius)&&d.radius>0&&
          std::isfinite(d.inverseMass)&&d.inverseMass>=0,"invalid disk");
}
Vec2 unit(Vec2 v) {
    const double length=std::hypot(v.x,v.y);
    check(std::isfinite(length)&&length>0,"contact has undefined normal");
    return v*(1.0/length);
}
// All inputs are checked by simulate(). This root is stable in the approaching case.
// An initial overlap is rejected by the caller: this witness solves *new impacts*,
// not position stabilization. Touching disks are considered only if closing.
std::optional<double> time_of_impact(const Disk& a,const Disk& b,double duration) {
    const Vec2 p=b.centre-a.centre, v=b.velocity-a.velocity;
    const double radius=a.radius+b.radius;
    const double c=dot(p,p)-radius*radius;
    const double aa=dot(v,v),bb=2*dot(p,v);
    check(std::isfinite(c)&&std::isfinite(aa)&&std::isfinite(bb),"nonfinite geometry");
    // Relative tolerance addresses near-touch floating error in successive events.
    const double tolerance=64*std::numeric_limits<double>::epsilon()*
                         std::max({1.0,dot(p,p),radius*radius});
    check(c>=-tolerance,"pre-existing geometric overlap; stabilization required");
    if(c<=tolerance) {
        if(bb>=0)return std::nullopt;
        return 0.0;
    }
    if(aa==0||bb>=0)return std::nullopt;
    const double discriminant=std::fma(bb,bb,-4*aa*c);
    if(discriminant<0)return std::nullopt;
    const double root=std::sqrt(discriminant);
    const double q=-0.5*(bb-root); // bb<0 means q>0.
    if(!(q>0))return std::nullopt;
    const double t=c/q;
    if(!std::isfinite(t)||t<0||t>duration)return std::nullopt;
    // A grazing contact has zero closing normal velocity and needs no impulse.
    const Vec2 atImpact=p+v*t;
    if(dot(v,atImpact)>=0)return std::nullopt;
    return t;
}
struct Event { std::size_t a{},b{}; double t{}; Vec2 normal{}; double targetSpeed{},impulse{}; };
struct Result { std::size_t events{},batches{}; double simulated{}; };
// The event cap is a safety boundary: hitting it raises an error, never silently
// advances through unresolved contacts. All simultaneous candidates are collected
// deterministically by pair order and their initial restitution targets are frozen.
Result simulate(std::vector<Disk>& disks,double dt,double restitution=1,
                std::size_t maxEvents=256) {
    check(std::isfinite(dt)&&dt>=0&&std::isfinite(restitution)&&
          restitution>=0&&restitution<=1,"invalid time or restitution");
    for(const auto& disk:disks)validate(disk);
    for(std::size_t i=0;i<disks.size();++i)
        for(std::size_t j=i+1;j<disks.size();++j)
            check(disks[i].id!=disks[j].id,"duplicate stable body id");
    double remaining=dt;
    Result result{};
    while(remaining>0) {
        double first=std::numeric_limits<double>::infinity();
        std::vector<Event> candidates;
        // Broad phase is intentionally all-pairs O(n^2), an exact CPU oracle.
        for(std::size_t i=0;i<disks.size();++i)for(std::size_t j=i+1;j<disks.size();++j) {
            const auto hit=time_of_impact(disks[i],disks[j],remaining);
            if(!hit)continue;
            if(*hit<first)first=*hit;
            candidates.push_back({i,j,*hit});
        }
        if(!std::isfinite(first)) {
            for(auto& d:disks)d.centre=d.centre+d.velocity*remaining;
            result.simulated+=remaining;
            break;
        }
        // Include simultaneous impacts within the declared time tolerance.
        const double epsTime=64*std::numeric_limits<double>::epsilon()*std::max(1.0,dt);
        std::vector<Event> batch;
        for(auto e:candidates)if(std::abs(e.t-first)<=epsTime)batch.push_back(e);
        check(!batch.empty(),"event collection failure");
        check(batch.size()<=maxEvents-result.events,"event budget exhausted");
        for(auto& d:disks)d.centre=d.centre+d.velocity*first;
        remaining-=first;
        result.simulated+=first;
        for(auto& e:batch) {
            auto& a=disks[e.a];auto& b=disks[e.b];
            e.normal=unit(b.centre-a.centre);
            const double speed=dot(b.velocity-a.velocity,e.normal);
            check(speed<0,"non-closing event selected");
            e.targetSpeed=-restitution*speed; // frozen pre-solve value
        }
        // Projected iterative normal impulses: handles simultaneous shared bodies.
        for(int iteration=0;iteration<32;++iteration)for(auto& e:batch) {
            auto& a=disks[e.a];auto& b=disks[e.b];
            const double inverseMass=a.inverseMass+b.inverseMass;
            if(inverseMass==0)continue;
            const double vn=dot(b.velocity-a.velocity,e.normal);
            const double updated=std::max(0.0,e.impulse+(e.targetSpeed-vn)/inverseMass);
            const double delta=updated-e.impulse;
            e.impulse=updated;
            a.velocity=a.velocity-e.normal*(delta*a.inverseMass);
            b.velocity=b.velocity+e.normal*(delta*b.inverseMass);
        }
        result.events+=batch.size();
        ++result.batches;
        // At t=0 a resolved separating/tangent pair must be skipped by the next TOI.
        // If it remains closing due to insufficient convergence, cap catches the loop.
    }
    return result;
}
Vec2 momentum(const std::vector<Disk>& ds) {
    Vec2 p{};
    for(const auto& d:ds)if(d.inverseMass>0)p=p+d.velocity*(1/d.inverseMass);
    return p;
}
double energy(const std::vector<Disk>& ds) {
    double e=0;
    for(const auto& d:ds)if(d.inverseMass>0)e+=dot(d.velocity,d.velocity)/(2*d.inverseMass);
    return e;
}
int main() {
    {
        std::vector<Disk> ds{{1,{0,0},{0,0},1,0},{2,{10,0},{-100,0},1,1}};
        const auto r=simulate(ds,0.1,1);
        check(r.events==1&&r.batches==1,"fast wall event missing");
        near(ds[1].centre.x,4);near(ds[1].velocity.x,100);
        near(r.simulated,0.1);
    }
    {
        std::vector<Disk> ds{{1,{-4,0},{2,0},1,1},{2,{4,0},{-2,0},1,1}};
        const auto p=momentum(ds);const auto k=energy(ds);
        const auto r=simulate(ds,2,1);
        check(r.events==1,"two disk collision count");
        near(ds[0].velocity.x,-2);near(ds[1].velocity.x,2);
        near(momentum(ds).x,p.x);near(energy(ds),k);
    }
    {
        // Three disks arrive simultaneously: both constraint rows share the middle mass.
        std::vector<Disk> ds{{1,{-4,0},{1,0},1,1},{2,{0,0},{0,0},1,1},
                             {3,{4,0},{-1,0},1,1}};
        const auto before=momentum(ds); const double k=energy(ds);
        const auto r=simulate(ds,3,1);
        check(r.events==2&&r.batches==1,"simultaneous batch missed");
        near(ds[0].velocity.x,-1,1e-7);near(ds[1].velocity.x,0,1e-7);
        near(ds[2].velocity.x,1,1e-7);
        near(momentum(ds).x,before.x);near(energy(ds),k,1e-7);
    }
    {
        std::vector<Disk> ds{{1,{0,0},{1,0},1,1},{2,{10,0},{2,0},1,1}};
        const auto r=simulate(ds,2);
        check(r.events==0,"separating contacts should not collide");
        near(ds[0].centre.x,2);near(ds[1].centre.x,14);
    }
    {
        // No implicit correction of pre-existing deep overlap.
        std::vector<Disk> ds{{1,{0,0},{0,0},1,1},{2,{1,0},{0,0},1,1}};
        bool rejected=false;
        try{(void)simulate(ds,0.5);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"initial overlap was not rejected");
    }
    {
        std::vector<Disk> ds{{1,{-4,0},{2,0},1,1},{2,{4,0},{-2,0},1,1}};
        bool rejected=false;
        try{(void)simulate(ds,2,1,0);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"event budget failure was silent");
    }
    {
        // The first impact starts a second event later in the same frame.
        std::vector<Disk> ds{{1,{-6,0},{4,0},1,1},{2,{0,0},{0,0},1,1},
                             {3,{6,0},{0,0},1,1}};
        const auto r=simulate(ds,3,1);
        check(r.events==2&&r.batches==2,"sequential event rescheduling failed");
        near(ds[0].velocity.x,0);near(ds[1].velocity.x,0);
        near(ds[2].velocity.x,4);near(momentum(ds).x,4);
    }
    {
        // Perfectly inelastic velocity-level impact preserves momentum but loses energy.
        std::vector<Disk> ds{{1,{-3,0},{2,0},1,1},{2,{3,0},{0,0},1,1}};
        const auto originalMomentum=momentum(ds).x;
        const auto initialEnergy=energy(ds);
        const auto r=simulate(ds,2,0);
        check(r.events==1,"inelastic impact not detected");
        near(ds[0].velocity.x,1);near(ds[1].velocity.x,1);
        near(momentum(ds).x,originalMomentum);
        check(energy(ds)<initialEnergy,"inelastic impact gained energy");
    }
    {
        // Stable IDs must not alias; zero-duration steps must not mutate velocities.
        std::vector<Disk> ds{{1,{0,0},{1,0},1,1},{2,{10,0},{0,0},1,1}};
        const auto r=simulate(ds,0);
        check(r.events==0,"zero duration generated event");
        near(ds[0].centre.x,0);
        ds[1].id=1;
        bool rejected=false;
        try{(void)simulate(ds,1);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"duplicate stable identities accepted");
    }
    std::cout << "disk event pipeline: 9 scene suites PASS\n";
}
