// C++20 sequential-impulse witness. Normal points A->B; units: m, s, kg, radians.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/persistent_contact_lab.cpp -o /tmp/persistent_contact && /tmp/persistent_contact
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Vec2 { double x{},y{}; };
constexpr Vec2 operator+(Vec2 a,Vec2 b){return {a.x+b.x,a.y+b.y};}
constexpr Vec2 operator-(Vec2 a,Vec2 b){return {a.x-b.x,a.y-b.y};}
constexpr Vec2 operator*(Vec2 a,double k){return {a.x*k,a.y*k};}
constexpr double dot(Vec2 a,Vec2 b){return a.x*b.x+a.y*b.y;}
constexpr double cross(Vec2 a,Vec2 b){return a.x*b.y-a.y*b.x;}
constexpr Vec2 spin(double w,Vec2 r){return {-w*r.y,w*r.x};}
void ensure(bool p,const char* s){if(!p)throw std::runtime_error(s);}
void near(double a,double b,double eps=1e-9){
 if(!(std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=eps))
  throw std::runtime_error("unexpected numerical result");
}
void validate(Vec2 v){ensure(std::isfinite(v.x)&&std::isfinite(v.y),"nonfinite vector");}
struct Body{Vec2 center{},linear{};double omega{},invMass{},invInertia{};};
void validate(const Body& b){
 validate(b.center);validate(b.linear);
 ensure(std::isfinite(b.omega)&&std::isfinite(b.invMass)&&std::isfinite(b.invInertia),"nonfinite body");
 ensure(b.invMass>=0&&b.invInertia>=0,"negative inverse mass/inertia");
}
struct Contact{
 std::uint64_t featureId{}; // Cache valid only while bodies, feature and orientation match.
 Vec2 worldPoint{},normal{0,1};
 double friction{},restitution{},normalImpulse{},tangentImpulse{},targetNormalSpeed{};
};
Vec2 unit(Vec2 v){
 validate(v);const double m=std::hypot(v.x,v.y);
 ensure(std::isfinite(m)&&m>std::numeric_limits<double>::min(),"invalid normal");
 return v*(1/m);
}
void validate(const Contact& c){
 validate(c.worldPoint);validate(c.normal);
 ensure(std::isfinite(c.friction)&&c.friction>=0&&std::isfinite(c.restitution)
  &&c.restitution>=0&&c.restitution<=1,"invalid material");
 ensure(std::isfinite(c.normalImpulse)&&c.normalImpulse>=0
  &&std::isfinite(c.tangentImpulse)&&std::isfinite(c.targetNormalSpeed),"invalid impulse cache");
}
struct Row{Body* a{};Body* b{};Contact* c{};Vec2 ra{},rb{},n{},t{};double kn{},kt{};};
Vec2 relative_velocity(const Row& r){
 return r.b->linear+spin(r.b->omega,r.rb)-r.a->linear-spin(r.a->omega,r.ra);
}
double inverse_effective_mass(const Row& r,Vec2 d){
 const double ca=cross(r.ra,d),cb=cross(r.rb,d);
 return r.a->invMass+r.b->invMass+r.a->invInertia*ca*ca+r.b->invInertia*cb*cb;
}
void apply(const Row& r,Vec2 p){
 r.a->linear=r.a->linear-p*r.a->invMass;
 r.b->linear=r.b->linear+p*r.b->invMass;
 r.a->omega-=r.a->invInertia*cross(r.ra,p);
 r.b->omega+=r.b->invInertia*cross(r.rb,p);
}
Row make_row(Body& a,Body& b,Contact& c,double threshold=0.5,bool newImpact=true){
 validate(a);validate(b);validate(c);
 ensure(std::isfinite(threshold)&&threshold>=0,"invalid bounce threshold");
 Row r{&a,&b,&c,c.worldPoint-a.center,c.worldPoint-b.center,unit(c.normal)};
 r.t={-r.n.y,r.n.x};
 r.kn=inverse_effective_mass(r,r.n);r.kt=inverse_effective_mass(r,r.t);
 ensure(std::isfinite(r.kn)&&std::isfinite(r.kt)&&r.kn>=0&&r.kt>=0,"invalid effective mass");
 // Sample and freeze restitution before applying impulses. No new bounce on persistent rest contact.
 if(newImpact){
  const double vn=dot(relative_velocity(r),r.n);
  c.targetNormalSpeed=vn< -threshold?-c.restitution*vn:0;
 }else c.targetNormalSpeed=0;
 ensure(std::abs(c.tangentImpulse)<=c.friction*c.normalImpulse+1e-9,"invalid friction cache");
 return r;
}
void warm_start(const Row& r){
 if(r.kn>0)apply(r,r.n*r.c->normalImpulse+r.t*r.c->tangentImpulse);
}
void solve_one(const Row& r){
 if(r.kn==0)return; // static/static, or locked in this direction
 Contact& c=*r.c;
 const double vn=dot(relative_velocity(r),r.n),oldN=c.normalImpulse;
 c.normalImpulse=std::max(0.0,oldN+(c.targetNormalSpeed-vn)/r.kn);
 apply(r,r.n*(c.normalImpulse-oldN));
 if(r.kt==0)return;
 const double vt=dot(relative_velocity(r),r.t),oldT=c.tangentImpulse;
 const double frictionBound=c.friction*c.normalImpulse;
 c.tangentImpulse=std::clamp(oldT-vt/r.kt,-frictionBound,frictionBound);
 apply(r,r.t*(c.tangentImpulse-oldT));
}
void sweep(std::vector<Row>& rows,int iterations){
 ensure(iterations>=0&&iterations<=10000,"invalid iteration count");
 for(int i=0;i<iterations;++i)for(const Row& r:rows)solve_one(r);
}
int main(){
 { // Equal masses: coefficient of restitution one conserves kinetic energy and momentum.
  Body a{{0,0},{2,0},0,1,1},b{{1,0},{0,0},0,1,1};
  Contact c{11,{0.5,0},{1,0},0,1};
  Row r=make_row(a,b,c);std::vector<Row> rows{r};sweep(rows,8);
  near(a.linear.x,0);near(b.linear.x,2);near(c.normalImpulse,2);
  near(a.linear.x+b.linear.x,2);near((dot(a.linear,a.linear)+dot(b.linear,b.linear))/2,2);
  sweep(rows,8);near(c.normalImpulse,2); // Restitution target was not regenerated.
 }
 { // Tangent = (-1,0) for upward normal; +1 tangent impulse opposes +x sliding.
  Body ground{{0,0},{0,0},0,0,0},box{{0,1},{3,-2},0,1,1};
  Contact c{12,{0,1},{0,1},0.5,0};
  Row r=make_row(ground,box,c);std::vector<Row> rows{r};sweep(rows,5);
  near(box.linear.y,0);near(box.linear.x,2);
  near(c.normalImpulse,2);near(c.tangentImpulse,1);
 }
 { // Off-center normal impulse produces angular velocity through r cross P.
  Body wall{{0,0},{0,0},0,0,0},box{{1,0},{-2,0},0,1,1};
  Contact c{13,{1,1},{1,0},0,0};
  Row r=make_row(wall,box,c);std::vector<Row> rows{r};sweep(rows,8);
  near(box.linear.x,-1);near(box.omega,-1);
 }
 { // Separating velocity and zero effective mass cannot generate attractive impulse.
  Body a{{0,0},{1,0},0,1,1},b{{1,0},{2,0},0,1,1};
  Contact c{14,{0.5,0},{1,0},0,0};
  Row r=make_row(a,b,c);std::vector<Row> rows{r};sweep(rows,2);
  near(c.normalImpulse,0);near(a.linear.x,1);near(b.linear.x,2);
  a.invMass=a.invInertia=b.invMass=b.invInertia=0;
  rows={make_row(a,b,c)};sweep(rows,2);near(c.normalImpulse,0);
 }
 { // Coupled two-point manifold, then warm-start with the preserved feature cache.
  Body ground{{0,0},{0,0},0,0,0},box{{0,1},{0,-2},0,1,1};
  Contact left{15,{-1,1},{0,1},0,0},right{16,{1,1},{0,1},0,0};
  std::vector<Row> rows{make_row(ground,box,left),make_row(ground,box,right)};
  sweep(rows,20);near(box.linear.y,0,1e-6);near(box.omega,0,1e-6);
  ensure(left.normalImpulse>=0&&right.normalImpulse>=0,"unilateral impulse");
  // Rebuilt velocities are *pre-warm-start*, otherwise the cached impulse is double-applied.
  box.linear={0,-2};box.omega=0;
  Row l=make_row(ground,box,left,0.5,false),r=make_row(ground,box,right,0.5,false);
  warm_start(l);warm_start(r);rows={l,r};sweep(rows,20);
  near(box.linear.y,0,1e-6);near(box.omega,0,1e-6);
 }
 { // A corrupted normal or NaN friction must be rejected rather than poison an island.
  Body a{},b{};Contact bad{17,{0,0},{0,0},0,0};bool rejected=false;
  try{(void)make_row(a,b,bad);}catch(const std::runtime_error&){rejected=true;}
  ensure(rejected,"zero normal accepted");bad.normal={1,0};
  bad.friction=std::numeric_limits<double>::quiet_NaN();rejected=false;
  try{(void)make_row(a,b,bad);}catch(const std::runtime_error&){rejected=true;}
  ensure(rejected,"NaN friction accepted");
 }
 std::cout<<"persistent-contact witness: 6 suites PASS\n";
}
