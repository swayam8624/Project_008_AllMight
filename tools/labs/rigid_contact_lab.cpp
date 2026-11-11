// C++20 velocity-level rigid-contact witness. Contact normal points A -> B.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/rigid_contact_lab.cpp -o /tmp/rigid_contact && /tmp/rigid_contact
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
struct Vec { double x{}, y{}; };
constexpr Vec operator+(Vec a, Vec b) { return {a.x+b.x,a.y+b.y}; }
constexpr Vec operator-(Vec a, Vec b) { return {a.x-b.x,a.y-b.y}; }
constexpr Vec operator*(Vec a, double k) { return {a.x*k,a.y*k}; }
constexpr double dot(Vec a, Vec b) { return a.x*b.x+a.y*b.y; }
constexpr double cross(Vec a, Vec b) { return a.x*b.y-a.y*b.x; }
constexpr Vec omega_cross_r(double w, Vec r) { return {-w*r.y,w*r.x}; }
Vec normalized(Vec v) {
    const double len=std::hypot(v.x,v.y);
    if (!(len>0) || !std::isfinite(len)) throw std::invalid_argument("invalid normal");
    return v*(1.0/len);
}
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void near(double a,double b,double tol=1e-10) {
    check(std::abs(a-b)<=tol,"numeric contract failed");
}
struct Body {
    Vec position{},velocity{};
    double angle{}, angularVelocity{};
    double invMass{},invInertia{}; // Both zero for a static body.
};
struct Contact {
    Vec point{}, normal{1,0}; // World-space point, A -> B.
    double restitution{}, friction{};
    double normalImpulse{}, tangentImpulse{}; // Accumulated projected impulses.
};
Vec point_velocity(const Body& b, Vec r) {
    return b.velocity+omega_cross_r(b.angularVelocity,r);
}
void apply_impulse(Body& a,Body& b,Vec ra,Vec rb,Vec impulse) {
    // Apply equal and opposite contact impulses and their angular moments.
    a.velocity=a.velocity-impulse*a.invMass;
    b.velocity=b.velocity+impulse*b.invMass;
    a.angularVelocity-=a.invInertia*cross(ra,impulse);
    b.angularVelocity+=b.invInertia*cross(rb,impulse);
}
double effective_mass(const Body& a,const Body& b,Vec ra,Vec rb,Vec direction) {
    // Inverse mass of both bodies plus the lever-arm contribution along direction.
    const double ca=cross(ra,direction),cb=cross(rb,direction);
    return a.invMass+b.invMass+a.invInertia*ca*ca+b.invInertia*cb*cb;
}
void solve_contact(Body& a,Body& b,Contact& c,double restitutionThreshold=0.5) {
    check(c.restitution>=0 && c.restitution<=1 && c.friction>=0,"invalid material");
    const Vec n=normalized(c.normal),t{-n.y,n.x};
    const Vec ra=c.point-a.position,rb=c.point-b.position;
    const double kn=effective_mass(a,b,ra,rb,n);
    if (!(kn>0) || !std::isfinite(kn)) return; // static/static
    Vec relative=point_velocity(b,rb)-point_velocity(a,ra);
    const double vn=dot(relative,n);
    // For a persistent contact, a production solver freezes the restitution target
    // once per collision; this minimal witness treats the first call as that event.
    const double bounce=(c.normalImpulse==0 && vn<-restitutionThreshold)
                           ? -c.restitution*vn : 0;
    const double oldN=c.normalImpulse;
    c.normalImpulse=std::max(0.0,oldN+(bounce-vn)/kn);
    apply_impulse(a,b,ra,rb,n*(c.normalImpulse-oldN));
    relative=point_velocity(b,rb)-point_velocity(a,ra);
    const double kt=effective_mass(a,b,ra,rb,t);
    if (!(kt>0) || !std::isfinite(kt)) return;
    const double oldT=c.tangentImpulse;
    // Coulomb friction bound is relative to the accumulated normal impulse.
    c.tangentImpulse=std::clamp(oldT-dot(relative,t)/kt,
                               -c.friction*c.normalImpulse,c.friction*c.normalImpulse);
    apply_impulse(a,b,ra,rb,t*(c.tangentImpulse-oldT));
}
int main() {
    Body a{{0,0},{2,0},0,0,1,1},b{{1,0},{0,0},0,0,1,1};
    Contact c{{0.5,0},{1,0},1,0};
    solve_contact(a,b,c);
    near(a.velocity.x,0);near(b.velocity.x,2);
    near(a.velocity.x+b.velocity.x,2);near(c.normalImpulse,2);
    Body ground{{0,0},{0,0},0,0,0,0};
    Body slider{{0,1},{3,-2},0,0,1,1};
    Contact floor{{0,1},{0,1},0,0.5};
    solve_contact(ground,slider,floor);
    near(slider.velocity.y,0);near(slider.velocity.x,2);
    near(floor.normalImpulse,2);near(std::abs(floor.tangentImpulse),1);
    Body wall{{0,0},{0,0},0,0,0,0};
    Body off{{1,0},{-2,0},0,0,1,1};
    Contact corner{{1,1},{1,0},0,0};
    solve_contact(wall,off,corner);
    near(off.velocity.x,-1);near(off.angularVelocity,-1);
    Body separating{{0,0},{1,0},0,0,1,1};
    Body target{{1,0},{2,0},0,0,1,1};
    Contact noHit{{0.5,0},{1,0},0,0};
    solve_contact(separating,target,noHit);
    near(noHit.normalImpulse,0);
    Body wall2{{1,0},{0,0},0,0,0,0};
    solve_contact(wall,wall2,noHit);near(noHit.normalImpulse,0);
    std::cout << "rigid-contact: elastic, friction, rotation, separating, static/static PASS\n";
}
