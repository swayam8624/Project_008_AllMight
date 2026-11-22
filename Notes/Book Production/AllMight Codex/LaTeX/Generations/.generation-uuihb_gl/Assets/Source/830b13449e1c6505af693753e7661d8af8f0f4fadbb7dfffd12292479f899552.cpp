// Standalone C++20 contact cache policy witness; no dependency on Kairo runtime.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror contact_cache_lab.cpp -o /tmp/contact_cache && /tmp/contact_cache
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

struct Vec2 { double x{}, y{}; };
constexpr Vec2 operator*(Vec2 a, double s) { return {a.x*s, a.y*s}; }
constexpr double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }
constexpr Vec2 tangent(Vec2 n) { return {-n.y, n.x}; }
void require(bool b, const char* msg) { if (!b) throw std::runtime_error(msg); }
void near(double a, double b, double eps=1e-10) {
    require(std::isfinite(a) && std::isfinite(b) && std::abs(a-b)<=eps, "unexpected numeric result");
}
Vec2 unit(Vec2 n) {
    require(std::isfinite(n.x) && std::isfinite(n.y), "nonfinite normal");
    const double length=std::hypot(n.x,n.y);
    require(length>1e-12 && std::isfinite(length), "degenerate normal");
    return n*(1.0/length);
}
// Feature indices come from a *narrowphase* feature matcher. They are not array indices.
// Pair orientation is canonical; swapping bodies is not interchangeable with reusing this cache.
struct Key {
    std::uint64_t bodyA{},bodyB{},featureA{},featureB{};
    auto operator<=>(const Key&) const = default;
};
struct ContactInput {
    Key key{};
    Vec2 normal{0,1};
    double friction{}, timeStep{};
};
struct CachedImpulse {
    Vec2 normal{0,1};
    double normalMagnitude{}, tangentMagnitude{}, friction{},timeStep{};
    std::uint64_t lastSeenFrame{};
};
struct Seed { double normal{}, tangent{}; bool reused{}; };
class ContactCache {
    std::map<Key,CachedImpulse> entries_;
    std::uint64_t frame_{};
    static constexpr double minimumNormalCosine=0.9659258262890683; // 15 deg
    static constexpr double maxStepRatio=2.0;
    static void validate(const ContactInput& in) {
        require(in.key.bodyA<in.key.bodyB, "body pair must have canonical orientation");
        require(std::isfinite(in.friction) && in.friction>=0, "invalid friction");
        require(std::isfinite(in.timeStep) && in.timeStep>0, "invalid timestep");
        (void)unit(in.normal);
    }
public:
    void begin_frame(std::uint64_t id) {
        require(id>frame_, "frame ids must increase"); frame_=id;
    }
    Seed lookup(const ContactInput& in) const {
        require(frame_>0, "begin_frame required"); validate(in);
        const auto it=entries_.find(in.key);
        if(it==entries_.end()) return {};
        const auto& old=it->second;
        // Only the immediately previous frame may warm start. Missing a frame invalidates state.
        if(old.lastSeenFrame!=frame_-1) return {};
        const Vec2 previous=unit(old.normal),current=unit(in.normal);
        const double alignment=dot(previous,current);
        if(alignment<minimumNormalCosine) return {};
        const double ratio=in.timeStep/old.timeStep;
        if(!std::isfinite(ratio) || ratio<1/maxStepRatio || ratio>maxStepRatio) return {};
        // A material edit invalidates the tangential cache, not the physical normal reaction.
        // Project the previous world-space tangential impulse onto the current tangent.
        const Vec2 previousTangentImpulse=tangent(previous)*old.tangentMagnitude;
        const double normal=old.normalMagnitude*ratio;
        const double tangentValue=(old.friction==in.friction)
            ? dot(previousTangentImpulse,tangent(current))*ratio : 0;
        return {normal,std::clamp(tangentValue,-in.friction*normal,in.friction*normal),true};
    }
    void store(const ContactInput& in,double normalImpulse,double tangentImpulse) {
        require(frame_>0,"begin_frame required"); validate(in);
        require(std::isfinite(normalImpulse) && normalImpulse>=0,"invalid normal impulse");
        require(std::isfinite(tangentImpulse),"invalid tangent impulse");
        require(std::abs(tangentImpulse)<=in.friction*normalImpulse+1e-9,"friction cone exceeded");
        // A repeated write for the same feature in one frame indicates a duplicate manifold row.
        const auto it=entries_.find(in.key);
        require(it==entries_.end() || it->second.lastSeenFrame!=frame_,"duplicate feature key");
        entries_[in.key]={unit(in.normal),normalImpulse,tangentImpulse,in.friction,in.timeStep,frame_};
    }
    void prune() {
        for(auto it=entries_.begin();it!=entries_.end();) {
            if(it->second.lastSeenFrame!=frame_)it=entries_.erase(it); else ++it;
        }
    }
    std::size_t size() const { return entries_.size(); }
};
int main() {
    const Key id{4,9,2,7};
    ContactCache cache; ContactInput in{id,{0,1},0.5,1.0/60.0};
    cache.begin_frame(1);
    require(!cache.lookup(in).reused,"new feature had cache");
    cache.store(in,2,0.5);
    cache.begin_frame(2);
    auto seed=cache.lookup(in);
    require(seed.reused,"persistent identity not reused"); near(seed.normal,2);near(seed.tangent,0.5);
    // Rotating the tangent frame changes the scalar, but preserves the projection.
    const double a=10*3.14159265358979323846/180;
    ContactInput rotated=in;rotated.normal={-std::sin(a),std::cos(a)};
    seed=cache.lookup(rotated);require(seed.reused,"small normal drift invalidated");
    near(seed.tangent,0.5*std::cos(a));
    ContactInput sharp=in;sharp.normal={-0.5,std::sqrt(0.75)};
    require(!cache.lookup(sharp).reused,"large normal drift accepted");
    ContactInput swapped=in;swapped.key={9,4,7,2};
    bool rejected=false;try{(void)cache.lookup(swapped);}catch(const std::runtime_error&){rejected=true;}
    require(rejected,"reversed body order accepted");
    ContactInput changedFeature=in;changedFeature.key.featureB=8;
    require(!cache.lookup(changedFeature).reused,"different geometry feature reused");
    ContactInput halfStep=in;halfStep.timeStep*=0.5;
    seed=cache.lookup(halfStep);require(seed.reused,"bounded timestep change rejected");
    near(seed.normal,1);near(seed.tangent,0.25);
    ContactInput jumpStep=in;jumpStep.timeStep*=4;
    require(!cache.lookup(jumpStep).reused,"large timestep jump accepted");
    ContactInput changedFriction=in;changedFriction.friction=0.1;
    seed=cache.lookup(changedFriction);require(seed.reused,"material edit erased normal cache");
    near(seed.normal,2);near(seed.tangent,0);
    cache.store(in,1,0.25);
    rejected=false;try{cache.store(in,1,0.25);}catch(const std::runtime_error&){rejected=true;}
    require(rejected,"duplicate feature admitted");
    cache.prune();require(cache.size()==1,"prune failure");
    cache.begin_frame(4);require(!cache.lookup(in).reused,"skipped frame reused");
    cache.prune();require(cache.size()==0,"stale feature survived prune");
    ContactInput bad=in;bad.normal={0,0};
    rejected=false;try{(void)cache.lookup(bad);}catch(const std::runtime_error&){rejected=true;}
    require(rejected,"degenerate normal accepted");
    std::cout<<"contact cache: identity, tangent transport, timestep, invalidation PASS\n";
}
