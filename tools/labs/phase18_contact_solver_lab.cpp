// Phase 18, M103-M108: deliberately bounded CPU oracle, C++20.
// This lab solves precomputed 2-D contacts. It does NOT implement a full
// narrowphase, general convex TOI, islands, sleeping or a production engine.
// Contracts and limitations are explained in the continuous physics chapter.
#include <algorithm>
#include <cmath>
#include <compare>
#include <iterator>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight::physics {
struct Vec2 {
    double x{}, y{};
    constexpr Vec2 operator+(Vec2 b) const { return {x+b.x, y+b.y}; }
    constexpr Vec2 operator-(Vec2 b) const { return {x-b.x, y-b.y}; }
    constexpr Vec2 operator*(double s) const { return {x*s, y*s}; }
};
constexpr double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }
constexpr double cross(Vec2 a, Vec2 b) { return a.x*b.y-a.y*b.x; }
constexpr Vec2 angular_cross(double w, Vec2 r) { return {-w*r.y,w*r.x}; }
double length(Vec2 a) { return std::sqrt(dot(a,a)); }
void require(bool yes,const char* message) {
    if (!yes) throw std::runtime_error(message);
}
bool approx(double a,double b,double eps=1e-9) {
    return std::abs(a-b)<=eps*std::max({1.,std::abs(a),std::abs(b)});
}

struct Body {
    Vec2 position{}, velocity{};
    double angle{}, angular_velocity{};
    double inv_mass{}, inv_inertia{};
    bool awake{true};
};

// Contact normal is from A toward B; impulses +normal push B away from A.
// Example: static floor is A, dynamic object is B and normal=(0,+1).
struct Contact {
    std::size_t a{}, b{};
    Vec2 normal{};
    Vec2 point{};                // World-space contact witness.
    double penetration{};        // Positive when objects overlap.
    double friction{0.5};
    double restitution{};
    std::uint64_t persistent_id{};
    double normal_impulse{}, tangent_impulse{};
    double normal_mass{}, tangent_mass{}, target_velocity{};
    Vec2 ra{}, rb{}, tangent{};
};

struct CacheKey {
    std::size_t a{}, b{};
    std::uint64_t feature{};
    friend auto operator<=>(const CacheKey&,const CacheKey&) = default;
};
struct Cached {
    double normal{}, tangent{};
};
// Cache validity requires stable body identity AND stable geometric feature.
// This example's indices are stable; production code needs generational IDs.
using ContactCache=std::map<CacheKey,Cached>;

double effective_inverse_mass(const Body& A,const Body& B,
                              Vec2 ra,Vec2 rb,Vec2 axis) {
    const double ta=cross(ra,axis),tb=cross(rb,axis);
    return A.inv_mass+B.inv_mass +
           A.inv_inertia*ta*ta+B.inv_inertia*tb*tb;
}

void apply_impulse(Body& A,Body& B,Vec2 ra,Vec2 rb,Vec2 impulse) {
    A.velocity=A.velocity-impulse*A.inv_mass;
    B.velocity=B.velocity+impulse*B.inv_mass;
    A.angular_velocity-=A.inv_inertia*cross(ra,impulse);
    B.angular_velocity+=B.inv_inertia*cross(rb,impulse);
}

Vec2 relative_velocity(const Body& A,const Body& B,Vec2 ra,Vec2 rb) {
    const Vec2 va=A.velocity+angular_cross(A.angular_velocity,ra);
    const Vec2 vb=B.velocity+angular_cross(B.angular_velocity,rb);
    return vb-va;
}

struct SolverOptions {
    unsigned iterations{12};
    double dt{1.0/60.0};
    double beta{0.15};
    double slop{0.002};
    double max_bias_velocity{2.0};
    double restitution_threshold{1.0};
    bool warm_start{true};
    bool positional_bias{true};
};

void solve_contacts(std::vector<Body>& bodies,
                    std::vector<Contact>& contacts,
                    ContactCache& cache,SolverOptions options={}) {
    require(options.dt>0 && std::isfinite(options.dt),"invalid time step");
    require(options.beta>=0 && options.beta<=1,"invalid bias factor");
    // Prepare all rows before any impulse can mutate body velocities.
    for(auto& c:contacts) {
        require(c.a<bodies.size() && c.b<bodies.size() && c.a!=c.b,
                "invalid contact body indices");
        require(std::isfinite(c.penetration) && c.penetration>=0,
                "invalid penetration depth");
        require(std::isfinite(c.friction) && c.friction>=0 &&
                std::isfinite(c.restitution) && c.restitution>=0 &&
                c.restitution<=1,"invalid contact material");
        const double nlength=length(c.normal);
        require(std::isfinite(nlength) && approx(nlength,1,1e-8),
                "contact normal must be a unit vector");
        c.ra=c.point-bodies[c.a].position;
        c.rb=c.point-bodies[c.b].position;
        c.tangent={-c.normal.y,c.normal.x};
        const double kn=effective_inverse_mass(
            bodies[c.a],bodies[c.b],c.ra,c.rb,c.normal);
        const double kt=effective_inverse_mass(
            bodies[c.a],bodies[c.b],c.ra,c.rb,c.tangent);
        require(kn>0 && kt>0 && std::isfinite(kn) && std::isfinite(kt),
                "contact between immovable/invalid bodies");
        c.normal_mass=1/kn;
        c.tangent_mass=1/kt;
        const double vn=dot(relative_velocity(
            bodies[c.a],bodies[c.b],c.ra,c.rb),c.normal);
        const double bounce=(vn < -options.restitution_threshold)
            ? -c.restitution*vn : 0.;
        const double depth=std::max(0.,c.penetration-options.slop);
        const double bias=options.positional_bias
            ? std::min(options.max_bias_velocity,
                       options.beta*depth/options.dt) : 0.;
        c.target_velocity=bounce+bias;
        c.normal_impulse=0;
        c.tangent_impulse=0;
        if(options.warm_start) {
            const auto it=cache.find({c.a,c.b,c.persistent_id});
            if(it!=cache.end()) {
                c.normal_impulse=std::max(0.,it->second.normal);
                c.tangent_impulse=std::clamp(it->second.tangent,
                   -c.friction*c.normal_impulse,
                    c.friction*c.normal_impulse);
            }
        }
    }
    if(options.warm_start) {
        for(auto& c:contacts) {
            auto& A=bodies[c.a];
            auto& B=bodies[c.b];
            apply_impulse(A,B,c.ra,c.rb,
                c.normal*c.normal_impulse+c.tangent*c.tangent_impulse);
        }
    }
    // Projected Gauss-Seidel: each row sees velocities updated by prior rows.
    for(unsigned iteration=0;iteration<options.iterations;++iteration) {
        for(auto& c:contacts) {
            auto& A=bodies[c.a];
            auto& B=bodies[c.b];
            const double vn=dot(relative_velocity(A,B,c.ra,c.rb),c.normal);
            const double proposal=(c.target_velocity-vn)*c.normal_mass;
            const double previous=c.normal_impulse;
            c.normal_impulse=std::max(0.,previous+proposal);
            const double delta=c.normal_impulse-previous;
            apply_impulse(A,B,c.ra,c.rb,c.normal*delta);

            const double vt=dot(relative_velocity(A,B,c.ra,c.rb),c.tangent);
            const double trial=c.tangent_impulse-vt*c.tangent_mass;
            const double bound=c.friction*c.normal_impulse;
            const double next=std::clamp(trial,-bound,bound);
            const double delta_t=next-c.tangent_impulse;
            c.tangent_impulse=next;
            apply_impulse(A,B,c.ra,c.rb,c.tangent*delta_t);
        }
    }
    // Rebuild cache: unmatched manifold features are invalidated this step.
    ContactCache fresh;
    for(const auto& c:contacts)
        fresh[{c.a,c.b,c.persistent_id}]=
            {c.normal_impulse,c.tangent_impulse};
    cache.swap(fresh);
}

std::optional<double> swept_disc_toi(Vec2 pa,Vec2 va,double ra,
                                     Vec2 pb,Vec2 vb,double rb,double dt) {
    require(dt>=0 && std::isfinite(dt),"invalid CCD time interval");
    require(ra>=0 && rb>=0,"invalid radius");
    const Vec2 p=pb-pa;
    const Vec2 v=vb-va;
    const double r=ra+rb;
    const double c=dot(p,p)-r*r;
    if(c<=0) return 0.; // Already touching/overlapping.
    const double a=dot(v,v);
    if(a==0) return std::nullopt;
    const double half_b=dot(p,v);
    // The nearest approach is in the past or motion is separating.
    if(half_b>=0) return std::nullopt;
    const double disc=half_b*half_b-a*c;
    if(disc<0) return std::nullopt;
    const double t=(-half_b-std::sqrt(disc))/a;
    if(t<0 || t>dt) return std::nullopt;
    return t;
}

// A fixed-step event edge: begin/end are differences of *sets* of pairs.
// Raw callbacks should not mutate the world during a solver iteration.
using Pair=std::pair<std::size_t,std::size_t>;
struct Events { std::vector<Pair> begin,end; };
Events contact_events(const std::vector<Pair>& previous,
                      const std::vector<Pair>& current) {
    auto a=previous,b=current;
    std::sort(a.begin(),a.end());
    std::sort(b.begin(),b.end());
    a.erase(std::unique(a.begin(),a.end()),a.end());
    b.erase(std::unique(b.begin(),b.end()),b.end());
    Events out;
    std::set_difference(b.begin(),b.end(),a.begin(),a.end(),
                        std::back_inserter(out.begin));
    std::set_difference(a.begin(),a.end(),b.begin(),b.end(),
                        std::back_inserter(out.end));
    return out;
}

void test_impulse_restitution() {
    std::vector<Body> bodies(2);
    bodies[0].inv_mass=0;
    bodies[1].inv_mass=1;
    bodies[1].velocity={0,-4};
    Contact c{};
    c.a=0;c.b=1;c.normal={0,1};c.point={0,0};
    c.friction=0;c.restitution=0.5;c.persistent_id=7;
    ContactCache cache;
    std::vector<Contact> rows{c};
    SolverOptions opts;
    opts.positional_bias=false;opts.iterations=6;
    solve_contacts(bodies,rows,cache,opts);
    require(approx(bodies[1].velocity.y,2),"restitution response");
    require(approx(rows[0].normal_impulse,6),"normal impulse");
    require(approx(bodies[0].velocity.y,0),"static floor remains fixed");
    require(approx(rows[0].tangent_impulse,0),"no friction");
}

void test_friction_and_moment() {
    std::vector<Body> bodies(2);
    bodies[1].inv_mass=1;bodies[1].inv_inertia=2;
    bodies[1].velocity={3,-2};
    Contact c{};
    c.a=0;c.b=1;c.normal={0,1};c.point={1,0};
    c.friction=0.4;c.persistent_id=19;
    ContactCache cache;
    std::vector<Contact> rows{c};
    SolverOptions opts;opts.positional_bias=false;
    solve_contacts(bodies,rows,cache,opts);
    require(rows[0].normal_impulse>=0,"nonnegative normal lambda");
    require(std::abs(rows[0].tangent_impulse) <=
            0.4*rows[0].normal_impulse+1e-10,"Coulomb clamp");
    require(std::isfinite(bodies[1].angular_velocity),"angular update finite");
    // Offset contact must produce torque from the normal component.
    require(std::abs(bodies[1].angular_velocity)>0,"off-center torque");
    require(approx(cache.at({0,1,19}).normal,rows[0].normal_impulse),
            "persistent cache stores impulses");
}

void test_warm_start_matching() {
    std::vector<Body> bodies(2);
    bodies[1].inv_mass=1;bodies[1].velocity={0,-3};
    Contact c{};c.a=0;c.b=1;c.normal={0,1};c.persistent_id=5;
    ContactCache cache;
    std::vector<Contact> rows{c};
    SolverOptions opts;opts.positional_bias=false;
    solve_contacts(bodies,rows,cache,opts);
    const double saved=cache.at({0,1,5}).normal;
    require(approx(saved,3),"first step cache");
    // Recreate pre-step velocities; zero iterations must still apply the
    // cached impulse (only for the matching persistent contact ID).
    bodies[1].velocity={0,-3};opts.iterations=0;
    solve_contacts(bodies,rows,cache,opts);
    require(approx(bodies[1].velocity.y,0),"warm start pre-applied");
    bodies[1].velocity={0,-3};
    rows[0].persistent_id=6;
    solve_contacts(bodies,rows,cache,opts);
    require(approx(bodies[1].velocity.y,-3),"unmatched feature not reused");
    require(cache.size()==1 && cache.contains({0,1,6}),"cache invalidation");
}

void test_coupled_stack_and_convergence() {
    // Static floor A, body B, body C. Two contacts couple B and C.
    // All normals up; the bottom B must support C as well as itself.
    std::vector<Body> bodies(3);
    bodies[1].inv_mass=1;bodies[2].inv_mass=1;
    bodies[1].velocity={0,-1};bodies[2].velocity={0,-1};
    Contact lower{};lower.a=0;lower.b=1;lower.normal={0,1};
    lower.persistent_id=31;lower.friction=0;
    Contact upper{};upper.a=1;upper.b=2;upper.normal={0,1};
    upper.persistent_id=32;upper.friction=0;
    std::vector<Contact> rows{lower,upper};
    ContactCache cache;
    SolverOptions opts;opts.positional_bias=false;opts.iterations=32;
    solve_contacts(bodies,rows,cache,opts);
    require(std::abs(bodies[1].velocity.y)<1e-6,"bottom stack stable");
    require(std::abs(bodies[2].velocity.y)<1e-6,"top stack stable");
    require(rows[0].normal_impulse>=rows[1].normal_impulse,
            "lower contact carries more cumulative normal impulse");
}

void test_toi_and_events() {
    const auto hit=swept_disc_toi({0,0},{10,0},1,
                                  {10,0},{0,0},1,1);
    require(hit && approx(*hit,0.8),"fast-disc TOI");
    require(!swept_disc_toi({0,0},{-10,0},1,
                           {10,0},{0,0},1,1),"separating pair");
    require(swept_disc_toi({0,0},{0,0},1,
                          {1,0},{0,0},1,1)==std::optional<double>(0),
            "initial overlap TOI zero");
    const auto events=contact_events({{1,2},{3,4}},{{3,4},{5,6},{5,6}});
    require(events.begin.size()==1 && events.begin[0]==Pair{5,6},
            "begin events deduplicated");
    require(events.end.size()==1 && events.end[0]==Pair{1,2},
            "end events");
}

} // namespace allmight::physics

int main() {
    try {
        using namespace allmight::physics;
        test_impulse_restitution();
        test_friction_and_moment();
        test_warm_start_matching();
        test_coupled_stack_and_convergence();
        test_toi_and_events();
        std::cout<<"phase18_contact_solver_lab: all checks passed\n";
    } catch(const std::exception& e) {
        std::cerr<<"phase18_contact_solver_lab: FAILED: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
