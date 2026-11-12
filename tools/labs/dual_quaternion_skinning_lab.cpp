// PROJECTALLMIGHT Phase 27 witness: dual-quaternion rigid transforms and antipodal-safe blending.
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }
bool near(double a,double b,double e=1e-9){ return std::fabs(a-b)<=e; }

struct Q{
    double w=1,x=0,y=0,z=0;
};
Q operator+(Q a,Q b){ return {a.w+b.w,a.x+b.x,a.y+b.y,a.z+b.z}; }
Q operator-(Q a){ return {-a.w,-a.x,-a.y,-a.z}; }
Q operator*(Q a,double s){ return {a.w*s,a.x*s,a.y*s,a.z*s}; }
Q mul(Q a,Q b){
    return {
        a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z,
        a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
        a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
        a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w
    };
}
Q conj(Q q){ return {q.w,-q.x,-q.y,-q.z}; }
double qdot(Q a,Q b){ return a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z; }
double qlen(Q q){ return std::sqrt(qdot(q,q)); }
Q qnorm(Q q){ const double l=qlen(q); require(l>1e-15,"zero quaternion"); return q*(1.0/l); }

struct V3{ double x=0,y=0,z=0; };
V3 operator+(V3 a,V3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator-(V3 a,V3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V3 operator*(V3 a,double s){ return {a.x*s,a.y*s,a.z*s}; }
double len(V3 v){ return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z); }

Q axis_angle(V3 axis,double radians){
    const double l=len(axis); require(l>1e-15,"zero rotation axis");
    axis=axis*(1.0/l);
    const double h=0.5*radians;
    const double s=std::sin(h);
    return qnorm({std::cos(h),axis.x*s,axis.y*s,axis.z*s});
}

V3 rotate(Q q,V3 v){
    q=qnorm(q);
    const Q p{0,v.x,v.y,v.z};
    const Q r=mul(mul(q,p),conj(q));
    return {r.x,r.y,r.z};
}

struct DQ{ Q real; Q dual; };

DQ from_rt(Q rotation,V3 t){
    rotation=qnorm(rotation);
    const Q tq{0,t.x,t.y,t.z};
    return {rotation,mul(tq,rotation)*0.5};
}

V3 translation(DQ d){
    d.real=qnorm(d.real);
    const Q t=mul(d.dual,conj(d.real))*2.0;
    return {t.x,t.y,t.z};
}

V3 transform(DQ d,V3 p){
    return rotate(d.real,p)+translation(d);
}

DQ normalized(DQ d){
    const double l=qlen(d.real);
    require(l>1e-15,"dual quaternion real part vanished");
    return {d.real*(1.0/l),d.dual*(1.0/l)};
}

DQ blend(DQ a,DQ b,double t){
    require(t>=0&&t<=1,"blend weight outside [0,1]");
    // q and -q encode the same rigid rotation. Align hemispheres before linear blending.
    if(qdot(a.real,b.real)<0){
        b.real=-b.real;
        b.dual=-b.dual;
    }
    return normalized({a.real*(1.0-t)+b.real*t,a.dual*(1.0-t)+b.dual*t});
}

void test_rigid_distance(){
    const DQ d=from_rt(axis_angle({0,0,1},0.8),{3,-2,1});
    const V3 a{1,2,0}, b{-2,0.5,4};
    require(near(len(transform(d,a)-transform(d,b)),len(a-b),1e-9),
            "dual-quaternion rigid transform must preserve distances");
}

void test_translation_blend(){
    const DQ a=from_rt({}, {0,0,0});
    const DQ b=from_rt({}, {10,4,-2});
    const DQ m=blend(a,b,0.25);
    const V3 t=translation(m);
    require(near(t.x,2.5)&&near(t.y,1.0)&&near(t.z,-0.5),"identity-rotation translation blend mismatch");
}

void test_antipodal_alignment(){
    const DQ a=from_rt(axis_angle({0,1,0},1.2),{1,2,3});
    DQ b=a;
    b.real=-b.real;
    b.dual=-b.dual;
    const DQ m=blend(a,b,0.5);
    const V3 p{0.3,-0.2,0.9};
    const V3 pa=transform(a,p), pm=transform(m,p);
    require(len(pa-pm)<1e-9,"antipodal-equivalent dual quaternions must not cancel during blend");
}

} // namespace

int main(){
    try{
        test_rigid_distance();
        test_translation_blend();
        test_antipodal_alignment();
        std::cout<<"dual_quaternion_skinning_lab: PASS\n"
                 <<"proved: dual-quaternion rigid transform, distance preservation, pure-translation blend, antipodal hemisphere handling\n"
                 <<"not proved: multi-joint normalized DQ skinning under arbitrary weights, scale/shear, bind-pose import, GPU skinning, or animation-system parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"dual_quaternion_skinning_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
