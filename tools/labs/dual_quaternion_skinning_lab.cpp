// Dual-quaternion rigid transform reference lab.
// Demonstrates encoding, translation recovery, antipodal-safe blending and
// point transformation. Non-rigid scale/shear are intentionally out of scope.

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

#ifdef assert
#undef assert
#endif
#define ALLMIGHT_REQUIRE(expr) do { if (!(expr)) throw std::runtime_error("check failed: " #expr); } while (false)
#define assert(expr) ALLMIGHT_REQUIRE(expr)

namespace allmight {

struct V3 { double x{},y{},z{}; };
V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V3 operator*(V3 a,double s){return {a.x*s,a.y*s,a.z*s};}
double length(V3 v){return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);}

struct Q {
    double x{},y{},z{},w{1.0};
};
Q operator+(Q a,Q b){return {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w};}
Q operator*(Q q,double s){return {q.x*s,q.y*s,q.z*s,q.w*s};}
Q operator-(Q q){return {-q.x,-q.y,-q.z,-q.w};}
double dot(Q a,Q b){return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;}
Q conjugate(Q q){return {-q.x,-q.y,-q.z,q.w};}
Q mul(Q a,Q b){
    return {
        a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
        a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
        a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
        a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z
    };
}
Q normalized(Q q){
    const double n=std::sqrt(dot(q,q));
    if(!(n>1.0e-14)) throw std::runtime_error("zero quaternion");
    return q*(1.0/n);
}
Q axis_angle(V3 axis,double angle){
    const double l=length(axis);
    if(!(l>1.0e-14)) throw std::invalid_argument("zero axis");
    axis=axis*(1.0/l);
    const double h=0.5*angle;
    const double s=std::sin(h);
    return {axis.x*s,axis.y*s,axis.z*s,std::cos(h)};
}
V3 rotate(Q q,V3 p){
    q=normalized(q);
    const Q r=mul(mul(q,{p.x,p.y,p.z,0.0}),conjugate(q));
    return {r.x,r.y,r.z};
}

struct DQ { Q real; Q dual; };

DQ from_rt(Q rotation,V3 t){
    const Q r=normalized(rotation);
    const Q tq{t.x,t.y,t.z,0.0};
    return {r,mul(tq,r)*0.5};
}

DQ normalized(DQ q){
    const double n=std::sqrt(dot(q.real,q.real));
    if(!(n>1.0e-14)) throw std::runtime_error("zero dual quaternion real part");
    q.real=q.real*(1.0/n);
    q.dual=q.dual*(1.0/n);
    // Project numerical drift so real·dual = 0, required for a rigid unit DQ.
    const double rd=dot(q.real,q.dual);
    q.dual=q.dual + q.real*(-rd);
    return q;
}

V3 translation(DQ q){
    q=normalized(q);
    const Q t=mul(q.dual,conjugate(q.real))*2.0;
    return {t.x,t.y,t.z};
}

V3 transform(DQ q,V3 p){
    q=normalized(q);
    return rotate(q.real,p)+translation(q);
}

DQ blend(DQ a,DQ b,double t){
    if(t<0.0||t>1.0) throw std::invalid_argument("blend parameter");
    a=normalized(a); b=normalized(b);
    if(dot(a.real,b.real)<0.0){
        b.real=-b.real;
        b.dual=-b.dual;
    }
    return normalized({
        a.real*(1.0-t)+b.real*t,
        a.dual*(1.0-t)+b.dual*t
    });
}

bool near(V3 a,V3 b,double eps=1.0e-8){return length(a-b)<=eps;}

void tests(){
    const Q identity{};
    const DQ tr=from_rt(identity,{3.0,-2.0,5.0});
    assert(near(translation(tr),{3.0,-2.0,5.0}));
    assert(near(transform(tr,{1.0,2.0,3.0}),{4.0,0.0,8.0}));

    const double pi=3.14159265358979323846;
    const DQ rz=from_rt(axis_angle({0,0,1},0.5*pi),{0,0,0});
    assert(near(transform(rz,{1,0,0}),{0,1,0},1.0e-7));

    // Flipping both real and dual parts encodes the same rigid transform.
    DQ anti{-tr.real,-tr.dual};
    assert(near(transform(anti,{2,3,4}),transform(tr,{2,3,4})));

    const DQ a=from_rt(identity,{0,0,0});
    const DQ b=from_rt(identity,{10,0,0});
    assert(near(transform(blend(a,b,0.5),{1,0,0}),{6,0,0}));

    assert(near(transform(blend(a,b,0.0),{1,2,3}),transform(a,{1,2,3})));
    assert(near(transform(blend(a,b,1.0),{1,2,3}),transform(b,{1,2,3})));
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"dual_quaternion_skinning_lab: PASS\n";
    return 0;
}
