// Original Phase 18, Parts 1701-1800: educational 2D impulse solver oracle.
// Model: planar rigid bodies, fixed contact geometry within each step,
// impulse-level normal & Coulomb friction constraints, optional warm start.
// NOT a broadphase, real 3D engine, position solver or robust convex CCD.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight::contacts {
void require(bool ok,const char* why) {
    if(!ok)throw std::runtime_error(why);
}
struct Vec2 {
    double x{},y{};
    constexpr Vec2 operator+(Vec2 b) const{return {x+b.x,y+b.y};}
    constexpr Vec2 operator-(Vec2 b) const{return {x-b.x,y-b.y};}
    constexpr Vec2 operator*(double s) const{return {x*s,y*s};}
    constexpr Vec2 operator/(double s) const{return {x/s,y/s};}
    Vec2& operator+=(Vec2 b){x+=b.x;y+=b.y;return *this;}
    Vec2& operator-=(Vec2 b){x-=b.x;y-=b.y;return *this;}
};
constexpr double dot(Vec2 a,Vec2 b){return a.x*b.x+a.y*b.y;}
constexpr double cross(Vec2 a,Vec2 b){return a.x*b.y-a.y*b.x;}
constexpr Vec2 angular_velocity_at(double w,Vec2 r) {
    return {-w*r.y,w*r.x};
}
double length(Vec2 v){return std::sqrt(dot(v,v));}
bool near(double a,double b,double e=1e-8){return std::abs(a-b)<=e;}
struct Body {
    Vec2 position{},velocity{};
    double angular_velocity{},inv_mass{},inv_inertia{};
    std::uint64_t stable_id{};
};
// Contact normal goes A -> B. Offsets rA,rB are WORLD vectors from each
// body's COM to the contact point; they must be refreshed after rotation.
struct Contact {
    std::size_t a{},b{};
    Vec2 normal{0,1},rA{},rB{};
    double restitution{},restitution_threshold{1.0},friction{};
    double penetration{},slop{0.005};
    std::uint64_t feature_id{};
    // These are accumulated TOTAL impulses, never per-iteration deltas.
    double accumulated_normal{},accumulated_tangent{};
    double restitution_target{};
};

Vec2 tangent(Vec2 n){return {-n.y,n.x};}
Vec2 point_velocity(const Body& body,Vec2 r) {
    return body.velocity+angular_velocity_at(body.angular_velocity,r);
}
Vec2 relative_velocity(const Body& A,const Body& B,const Contact& c) {
    return point_velocity(B,c.rB)-point_velocity(A,c.rA);
}
double inverse_effective_mass(const Body& A,const Body& B,
                              const Contact& c,Vec2 direction) {
    // J M^-1 J^T, planar Jacobian:
    // J=[-d_x,-d_y,-cross(rA,d),+d_x,+d_y,+cross(rB,d)].
    const double ra=cross(c.rA,direction);
    const double rb=cross(c.rB,direction);
    return A.inv_mass+B.inv_mass+
           ra*ra*A.inv_inertia+rb*rb*B.inv_inertia;
}
void apply_impulse(Body& A,Body& B,const Contact& c,Vec2 impulse) {
    // Equal and opposite impulse. Angular impulses are r cross p.
    A.velocity-=impulse*A.inv_mass;
    A.angular_velocity-=cross(c.rA,impulse)*A.inv_inertia;
    B.velocity+=impulse*B.inv_mass;
    B.angular_velocity+=cross(c.rB,impulse)*B.inv_inertia;
}
void validate_contact(const std::vector<Body>& bodies,const Contact& c) {
    if(c.a>=bodies.size()||c.b>=bodies.size()||c.a==c.b)
        throw std::invalid_argument("contact body references");
    if(!near(dot(c.normal,c.normal),1.0,1e-7))
        throw std::invalid_argument("contact normal must be unit");
    if(c.friction<0||c.restitution<0||c.restitution>1 ||
       c.restitution_threshold<0 || c.accumulated_normal<0)
        throw std::invalid_argument("contact material/impulse contract");
    if(bodies[c.a].inv_mass<0||bodies[c.b].inv_mass<0 ||
       bodies[c.a].inv_inertia<0||bodies[c.b].inv_inertia<0)
        throw std::invalid_argument("negative inverse mass/inertia");
}

void transfer_cached_impulses(const Contact& prior,Contact& current) {
    // A long-lived contact cache needs a stable geometric feature ID.
    // Do not reuse impulse when the feature/body identity or normal flips.
    if(prior.feature_id==current.feature_id &&
       prior.a==current.a && prior.b==current.b &&
       dot(prior.normal,current.normal)>0.98) {
        current.accumulated_normal=prior.accumulated_normal;
        current.accumulated_tangent=prior.accumulated_tangent;
    } else {
        current.accumulated_normal=0;
        current.accumulated_tangent=0;
    }
}

// Prepare target restitution BEFORE warm starting. Resting contacts should
// not repeatedly bounce from tiny gravity-induced relative velocities.
void prepare(std::vector<Body>& bodies,std::vector<Contact>& contacts,
             bool warm_start) {
    for(Contact& c:contacts) {
        validate_contact(bodies,c);
        Body& A=bodies[c.a];
        Body& B=bodies[c.b];
        const double vn=dot(relative_velocity(A,B,c),c.normal);
        c.restitution_target=vn < -c.restitution_threshold
                             ? -c.restitution*vn : 0;
        if(warm_start) {
            const Vec2 impulse=c.normal*c.accumulated_normal+
                               tangent(c.normal)*c.accumulated_tangent;
            apply_impulse(A,B,c,impulse);
        } else {
            c.accumulated_normal=0;
            c.accumulated_tangent=0;
        }
    }
}
void solve_one_normal(Body& A,Body& B,Contact& c) {
    const double k=inverse_effective_mass(A,B,c,c.normal);
    if(k<=0)return; // Both bodies immovable along the contact normal.
    const double vn=dot(relative_velocity(A,B,c),c.normal);
    const double incremental=(c.restitution_target-vn)/k;
    // Project the *accumulated* impulse onto lambda_n >= 0.
    const double next=std::max(0.0,c.accumulated_normal+incremental);
    const double applied=next-c.accumulated_normal;
    c.accumulated_normal=next;
    apply_impulse(A,B,c,c.normal*applied);
}
void solve_one_friction(Body& A,Body& B,Contact& c) {
    const Vec2 t=tangent(c.normal);
    const double k=inverse_effective_mass(A,B,c,t);
    if(k<=0)return;
    const double vt=dot(relative_velocity(A,B,c),t);
    const double change=-vt/k;
    const double limit=c.friction*c.accumulated_normal;
    const double next=std::clamp(c.accumulated_tangent+change,
                                 -limit,+limit);
    const double applied=next-c.accumulated_tangent;
    c.accumulated_tangent=next;
    apply_impulse(A,B,c,t*applied);
}
void solve(std::vector<Body>& bodies,std::vector<Contact>& contacts,
           int iterations,bool warm_start=false) {
    if(iterations<=0)throw std::invalid_argument("solver iterations");
    prepare(bodies,contacts,warm_start);
    // Projected Gauss-Seidel: each row immediately changes velocities seen
    // by later rows. Ordering can change numerical convergence.
    for(int iteration=0;iteration<iterations;++iteration)
        for(Contact& c:contacts) {
            Body& A=bodies[c.a];Body& B=bodies[c.b];
            solve_one_normal(A,B,c);
            solve_one_friction(A,B,c);
        }
}

// Pure geometry for a plane at y=0, with the legal circle above the plane.
// The circle center is currently above (radius or more). Relative plane
// motion, rotational hulls and arbitrary convex geometry are NOT covered.
std::optional<double> toi_circle_static_plane(
    double center_y,double radius,double vertical_speed,double dt) {
    if(radius<=0||dt<0)throw std::invalid_argument("TOI inputs");
    if(center_y-radius<=0)return 0;
    if(vertical_speed>=0)return std::nullopt;
    const double time=(center_y-radius)/(-vertical_speed);
    if(time<=dt)return time;
    return std::nullopt;
}
struct Aabb {Vec2 minimum,maximum;};
Aabb swept_circle_aabb(Vec2 center,Vec2 velocity,double radius,double dt) {
    if(radius<0||dt<0)throw std::invalid_argument("swept AABB inputs");
    const Vec2 endpoint=center+velocity*dt;
    return {{std::min(center.x,endpoint.x)-radius,
             std::min(center.y,endpoint.y)-radius},
            {std::max(center.x,endpoint.x)+radius,
             std::max(center.y,endpoint.y)+radius}};
}

// Collision layer/mask filtering is symmetric. Group-based exceptions,
// triggers, sleeping policies and sensors are application-level additions.
bool allows_pair(std::uint32_t catA,std::uint32_t maskA,
                 std::uint32_t catB,std::uint32_t maskB) {
    return (catA&maskB)!=0 && (catB&maskA)!=0;
}

// Maintain a completed-step event snapshot instead of issuing callbacks
// while the narrowphase/solver is still mutating contact state.
struct PairEvents {
    std::set<std::uint64_t> previous;
    std::vector<std::uint64_t> began,ended;
    void complete_step(std::set<std::uint64_t> current) {
        began.clear();ended.clear();
        for(auto id:current)
            if(!previous.contains(id))began.push_back(id);
        for(auto id:previous)
            if(!current.contains(id))ended.push_back(id);
        previous=std::move(current);
    }
};

// Island sleeping needs both angular and linear thresholds and a stable
// timeout; this is a single-body predicate, NOT an island/wake solver.
bool may_sleep(const Body& b,double quiet_seconds,
               double linear_threshold,double angular_threshold,
               double duration) {
    return quiet_seconds>=duration &&
           length(b.velocity)<linear_threshold &&
           std::abs(b.angular_velocity)<angular_threshold;
}

void test_inelastic_two_body_momentum() {
    // Two equal unit-mass bodies collide head-on in x. Conserves their
    // combined linear momentum; e=0 removes relative normal velocity.
    std::vector<Body> b(2);
    b[0].velocity={+1,0};b[0].inv_mass=1;
    b[1].velocity={-1,0};b[1].inv_mass=1;
    Contact c{};
    c.a=0;c.b=1;c.normal={1,0};c.restitution=0;c.friction=0;
    std::vector<Contact> contacts{c};
    const double momentum_before=b[0].velocity.x+b[1].velocity.x;
    solve(b,contacts,8);
    require(near(b[0].velocity.x,0)&&near(b[1].velocity.x,0),
            "head-on inelastic relative normal velocity");
    require(near(b[0].velocity.x+b[1].velocity.x,momentum_before),
            "equal opposite impulses preserve total linear momentum");
    require(near(contacts[0].accumulated_normal,1),
            "normal accumulated impulse is one");
}
void test_restitution_and_threshold() {
    std::vector<Body> b(2);
    b[1].velocity={0,-4};b[1].inv_mass=1;
    Contact c{};c.a=0;c.b=1;c.normal={0,1};
    c.restitution=0.5;c.restitution_threshold=0.2;
    std::vector<Contact> contacts{c};
    solve(b,contacts,4);
    require(near(b[1].velocity.y,2), "half-restitution bounce speed");
    require(near(contacts[0].accumulated_normal,6),
            "bounce impulse = 6");
    b[1].velocity={0,-0.01};
    contacts[0].accumulated_normal=0;
    solve(b,contacts,4);
    require(near(b[1].velocity.y,0), "low speed bounce suppressed");
}
void test_angular_effective_mass() {
    std::vector<Body> b(2);
    b[1].velocity={0,-4};b[1].inv_mass=1;b[1].inv_inertia=1;
    Contact c{};c.a=0;c.b=1;c.normal={0,1};c.rB={1,0};
    std::vector<Contact> contacts{c};
    require(near(inverse_effective_mass(b[0],b[1],c,c.normal),2),
            "normal effective mass denominator");
    solve(b,contacts,5);
    require(near(contacts[0].accumulated_normal,2),"off-center impulse");
    require(near(b[1].velocity.y,-2),"translational effect");
    require(near(b[1].angular_velocity,2),"rotational effect");
    require(near(dot(point_velocity(b[1],{1,0}),c.normal),0),
            "contact-point velocity is zero even if COM still moves");
}
void test_friction_cone_in_2d() {
    std::vector<Body> b(2);
    b[1].velocity={3,-1};b[1].inv_mass=1;
    Contact c{};c.a=0;c.b=1;c.normal={0,1};c.friction=0.5;
    std::vector<Contact> contacts{c};
    solve(b,contacts,6);
    require(near(b[1].velocity.y,0),"normal nonpenetration");
    require(near(b[1].velocity.x,2.5),"tangent capped by mu lambda_n");
    require(near(std::abs(contacts[0].accumulated_tangent),0.5),
            "friction cone boundary");
}
void test_stack_and_warm_start() {
    std::vector<Body> b(3);
    // Static ground, lower dynamic body, upper dynamic body.
    b[1].velocity={0,-1};b[1].inv_mass=1;
    b[2].velocity={0,-1};b[2].inv_mass=1;
    Contact ground{};ground.a=0;ground.b=1;ground.normal={0,1};
    ground.feature_id=101;
    Contact stack{};stack.a=1;stack.b=2;stack.normal={0,1};
    stack.feature_id=202;
    std::vector<Contact> contacts{ground,stack};
    solve(b,contacts,28);
    require(b[1].velocity.y>=-1e-7 &&
            b[2].velocity.y-b[1].velocity.y>=-1e-7,
            "stack projected normal constraints converge on oracle");
    require(contacts[0].accumulated_normal>0,
            "support impulse accumulated");

    // On a next fixed step, gravity adds downward velocity. A matching
    // stable feature may reuse accumulated impulses; the solver corrects them.
    const auto previous=contacts;
    b[1].velocity.y-=0.1;
    b[2].velocity.y-=0.1;
    std::vector<Contact> fresh{ground,stack};
    transfer_cached_impulses(previous[0],fresh[0]);
    transfer_cached_impulses(previous[1],fresh[1]);
    require(fresh[0].accumulated_normal>0,
            "matching feature recycles previous impulse");
    solve(b,fresh,28,true);
    require(b[1].velocity.y>=-1e-7 &&
            b[2].velocity.y-b[1].velocity.y>=-1e-7,
            "warm-started stack constraints");

    // Changing feature identity invalidates the cached impulse.
    Contact unmatched=ground;unmatched.feature_id=999;
    transfer_cached_impulses(previous[0],unmatched);
    require(near(unmatched.accumulated_normal,0),
            "stale feature must not warm-start");
}
void test_ccd_and_queries() {
    const auto hit=toi_circle_static_plane(3,0.5,-100,0.1);
    require(hit && near(*hit,0.025),"fast circle hits before endpoint");
    require(!toi_circle_static_plane(3,0.5,+100,0.1),
            "receding circle misses");
    const auto swept=swept_circle_aabb({0,3},{2,-100},0.5,0.1);
    require(near(swept.minimum.y,-7.5) &&
            near(swept.maximum.y,3.5),"swept AABB both endpoints");
    require(allows_pair(1,2,2,1),"two-way masks");
    require(!allows_pair(1,2,4,1),"one-sided mask does not authorize");

    PairEvents events;
    events.complete_step({11,22});
    require(events.began==std::vector<std::uint64_t>({11,22}) &&
            events.ended.empty(),"begin events first step");
    events.complete_step({22,33});
    require(events.began==std::vector<std::uint64_t>({33}) &&
            events.ended==std::vector<std::uint64_t>({11}),
            "begin and end diff buffered by step");

    Body sleepy{};
    require(may_sleep(sleepy,2,0.01,0.01,1),"quiet body sleep candidate");
    sleepy.angular_velocity=0.2;
    require(!may_sleep(sleepy,2,0.01,0.01,1),
            "moving angular body must stay awake");
}
} // namespace allmight::contacts

int main() {
    using namespace allmight::contacts;
    try {
        test_inelastic_two_body_momentum();
        test_restitution_and_threshold();
        test_angular_effective_mass();
        test_friction_cone_in_2d();
        test_stack_and_warm_start();
        test_ccd_and_queries();
        std::cout<<"phase18_contact_solver_lab: all invariants passed\n";
    } catch(const std::exception& error) {
        std::cerr<<"phase18_contact_solver_lab: FAIL: "<<error.what()<<"\n";
        return 1;
    }
    return 0;
}
