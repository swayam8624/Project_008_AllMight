// C++20 analytic linear-motion CCD for two disks, not a general rigid-body CCD system.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror disk_ccd_lab.cpp -o /tmp/disk_ccd && /tmp/disk_ccd
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
struct Vec2 { double x{},y{}; };
constexpr Vec2 operator+(Vec2 a,Vec2 b){return {a.x+b.x,a.y+b.y};}
constexpr Vec2 operator-(Vec2 a,Vec2 b){return {a.x-b.x,a.y-b.y};}
constexpr Vec2 operator*(Vec2 a,double t){return {a.x*t,a.y*t};}
constexpr double dot(Vec2 a,Vec2 b){return a.x*b.x+a.y*b.y;}
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
void near(double a,double b,double eps=1e-10){check(std::isfinite(a)&&std::abs(a-b)<eps,"numeric result mismatch");}
struct Disk{Vec2 center{},velocity{};double radius{};};
struct Hit{double time{};Vec2 normal{};bool initiallyOverlapping{};};
void validate(const Disk& d){
 check(std::isfinite(d.center.x)&&std::isfinite(d.center.y)&&
       std::isfinite(d.velocity.x)&&std::isfinite(d.velocity.y)&&
       std::isfinite(d.radius)&&d.radius>=0,"invalid disk");
}
// Relative separation p(t) = p0 + t*v, with disk radius sum R.
// Solve |p(t)|^2 = R^2, or a*t^2 + b*t + c = 0.
std::optional<Hit> first_disk_contact(const Disk& A,const Disk& B,double dt){
 validate(A);validate(B);
 check(std::isfinite(dt)&&dt>=0,"invalid frame duration");
 const Vec2 p=B.center-A.center, v=B.velocity-A.velocity;
 const double R=A.radius+B.radius;
 const double a=dot(v,v), b=2*dot(p,v), c=dot(p,p)-R*R;
 check(std::isfinite(R)&&std::isfinite(a)&&std::isfinite(b)&&std::isfinite(c),
       "out-of-range relative geometry");
 auto normalAt=[&](double t){
   const Vec2 sep=p+v*t;
   const double d=std::hypot(sep.x,sep.y);
   if(d>0)return sep*(1.0/d);
   const double speed=std::hypot(v.x,v.y);
   // Deterministic fallback for coincident centres; no unique geometric normal exists.
   return speed>0?v*(-1.0/speed):Vec2{1,0};
 };
 if(c<=0)return Hit{0,normalAt(0),c<0}; // existing overlap handled by another stage
 if(a==0 || b>=0)return std::nullopt; // no relative motion or not closing
 // fma avoids one rounding in the discriminant when supported by the platform.
 const double discriminant=std::fma(b,b,-4*a*c);
 if(!(discriminant>=0))return std::nullopt; // includes NaN rejection
 const double root=std::sqrt(discriminant);
 // Numerically stable quadratic roots. Here b<0, so q is nonzero and positive.
 const double q=-0.5*(b- root);
 if(!(q>0))return std::nullopt;
 const double nearRoot=c/q;
 const double farRoot=q/a;
 const double t=std::min(nearRoot,farRoot);
 if(!(std::isfinite(t)&&t>=0&&t<=dt))return std::nullopt;
 return Hit{t,normalAt(t),false};
}
int main(){
 const Disk still{{0,0},{0,0},1},bullet{{10,0},{-100,0},1};
 auto h=first_disk_contact(still,bullet,0.1);
 check(h.has_value(),"tunneling hit missed");near(h->time,0.08);near(h->normal.x,1);
 check(!first_disk_contact(still,bullet,0.05),"hit beyond frame accepted");
 check(!first_disk_contact(still,Disk{{10,0},{100,0},1},1),"separating disk hit");
 check(!first_disk_contact(still,Disk{{10,2.1},{-100,0},1},1),"near miss hit");
 h=first_disk_contact(still,Disk{{10,2},{-10,0},1},2);
 check(h.has_value(),"grazing hit missed");near(h->time,1);near(h->normal.y,1);
 h=first_disk_contact(still,Disk{{1,0},{0,0},1},1);
 check(h.has_value()&&h->initiallyOverlapping,"overlap policy broken");near(h->time,0);
 h=first_disk_contact(still,Disk{{0,0},{0,0},1},0);
 check(h.has_value(),"coincident overlap missed");near(h->normal.x,1);
 // Relative motion is invariant under a common uniform Galilean velocity.
 h=first_disk_contact(Disk{{0,0},{55,7},1},Disk{{10,0},{-45,7},1},0.1);
 check(h.has_value(),"relative velocity test failed");near(h->time,0.08);
 // Large coordinate and velocity magnitudes exercise the stable-root path.
 h=first_disk_contact(Disk{{0,0},{0,0},1},Disk{{1000000,0},{-1000000,0},1},2);
 check(h.has_value(),"large-scale impact missed");near(h->time,0.999998,1e-8);
 bool rejected=false;
 try{(void)first_disk_contact(still,bullet,-1);}catch(const std::runtime_error&){rejected=true;}
 check(rejected,"negative timestep accepted");
 rejected=false;
 try{(void)first_disk_contact(still,Disk{{10,0},{-1,0},-1},1);}catch(const std::runtime_error&){rejected=true;}
 check(rejected,"negative radius accepted");
 std::cout<<"disk CCD: tunneling, grazing, overlap, misses, invariance, validation PASS\n";
}
