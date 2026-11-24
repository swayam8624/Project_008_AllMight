#pragma once
// P12 — orientation as a unit quaternion and a proper orthogonal map.
// Right-handed active rotations, column vectors, radians, Hamilton product.
// qB*qA applies A first, B second. The four stored components are w,x,y,z.
// A quaternion is NOT a general Vec4, and neither scale nor shear is rotation.
#include "book03/scene_forest.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
namespace book03 {
struct QuaternionComponents {
    double w{1},x{},y{},z{};
};

// Keep the unit constraint behind a construction boundary. Scaling by the
// largest component before taking the norm prevents ordinary overflow and
// underflow when even a very large or tiny nonzero input is normalized.
class Rotation3 {
    QuaternionComponents q_{};
    explicit Rotation3(QuaternionComponents q):q_(q){}
public:
    Rotation3()=default;
    [[nodiscard]] static Rotation3 normalized(QuaternionComponents q) {
        if(!std::isfinite(q.w)||!std::isfinite(q.x)||
           !std::isfinite(q.y)||!std::isfinite(q.z))
            throw std::invalid_argument("nonfinite quaternion");
        const double s=std::max({std::abs(q.w),std::abs(q.x),
                                 std::abs(q.y),std::abs(q.z)});
        if(s==0)throw std::invalid_argument("zero quaternion");
        q={q.w/s,q.x/s,q.y/s,q.z/s};
        const double n=std::hypot(std::hypot(q.w,q.x),std::hypot(q.y,q.z));
        return Rotation3{{q.w/n,q.x/n,q.y/n,q.z/n}};
    }
    [[nodiscard]] QuaternionComponents components() const noexcept {return q_;}
    [[nodiscard]] static Rotation3 axis_angle(Vector3 axis,double radians) {
        if(!finite(axis)||!std::isfinite(radians))
            throw std::invalid_argument("nonfinite axis or angle");
        const double m=std::max({std::abs(axis.x),std::abs(axis.y),std::abs(axis.z)});
        if(m==0)throw std::invalid_argument("zero rotation axis");
        // The axis is a direction; scaling its components does not change it.
        const Vector3 u=unit({axis.x/m,axis.y/m,axis.z/m});
        const double h=0.5*std::remainder(radians,4*std::numbers::pi);
        const double s=std::sin(h);
        return normalized({std::cos(h),s*u.x,s*u.y,s*u.z});
    }
    [[nodiscard]] Rotation3 inverse() const noexcept {
        return Rotation3{{q_.w,-q_.x,-q_.y,-q_.z}};
    }
    [[nodiscard]] friend Rotation3 operator*(Rotation3 a,Rotation3 b) {
        const auto p=a.q_,q=b.q_;
        const Vector3 u{p.x,p.y,p.z},v{q.x,q.y,q.z};
        const Vector3 xyz=p.w*v+q.w*u+cross(u,v);
        return normalized({p.w*q.w-dot(u,v),xyz.x,xyz.y,xyz.z});
    }
    [[nodiscard]] Vector3 apply(Vector3 v) const {
        if(!finite(v))throw std::invalid_argument("nonfinite rotation input");
        // q(0,v)q* in its expanded vector form. The two crosses are with
        // the quaternion vector part, not with an arbitrary rotation axis.
        const Vector3 u{q_.x,q_.y,q_.z};
        const Vector3 t=2.0*cross(u,v);
        const Vector3 out=v+q_.w*t+cross(u,t);
        if(!finite(out))throw std::overflow_error("rotation result overflow");
        return out;
    }
    [[nodiscard]] Linear3 matrix() const {
        return {apply({1,0,0}),apply({0,1,0}),apply({0,0,1})};
    }
    // Extrinsic fixed X, then Y, then Z, equivalently Rz Ry Rx.
    // This names the order; "Euler rotation" alone is ambiguous.
    [[nodiscard]] static Rotation3 fixed_xyz(double x,double y,double z) {
        return axis_angle({0,0,1},z)*axis_angle({0,1,0},y)*axis_angle({1,0,0},x);
    }
    [[nodiscard]] static Rotation3 slerp(Rotation3 a,Rotation3 b,double t) {
        if(!std::isfinite(t)||t<0||t>1)
            throw std::invalid_argument("SLERP parameter must lie in [0,1]");
        auto p=a.q_,q=b.q_;
        double d=p.w*q.w+p.x*q.x+p.y*q.y+p.z*q.z;
        // q and -q represent one orientation. Flip the second endpoint
        // when needed so the lifted path uses a short arc on S^3.
        if(d<0){q={-q.w,-q.x,-q.y,-q.z};d=-d;}
        d=std::clamp(d,0.0,1.0);
        auto mix=[&](double u,double v){
            return normalized({u*p.w+v*q.w,u*p.x+v*q.x,
                               u*p.y+v*q.y,u*p.z+v*q.z});
        };
        // Near coincidence: normalized linear interpolation avoids the
        // small sin(theta) denominator. Not exact constant-speed SLERP.
        if(d>0.9995)return mix(1-t,t);
        const double theta=std::acos(d),den=std::sin(theta);
        return mix(std::sin((1-t)*theta)/den,std::sin(t*theta)/den);
    }
};

// Independent axis-angle formula, useful as a mathematical oracle for
// Rotation3::apply. Projection onto the axis remains invariant.
[[nodiscard]] inline Vector3 rodrigues(Vector3 axis,double angle,Vector3 v) {
    if(!finite(axis)||!finite(v)||!std::isfinite(angle))
        throw std::invalid_argument("nonfinite Rodrigues argument");
    const double m=std::max({std::abs(axis.x),std::abs(axis.y),std::abs(axis.z)});
    if(m==0)throw std::invalid_argument("zero rotation axis");
    const Vector3 k=unit({axis.x/m,axis.y/m,axis.z/m});
    const double c=std::cos(angle),s=std::sin(angle);
    const Vector3 out=c*v+s*cross(k,v)+((1-c)*dot(k,v))*k;
    if(!finite(out))throw std::overflow_error("Rodrigues result overflow");
    return out;
}

struct EulerXYZ {
    double x{},y{},z{};
    bool near_lock{};
};
// Extract one principal fixed-XYZ triple. At a lock, set z=0 and solve
// for the remaining coupled degree of freedom. Near a lock the returned
// orientation is only an approximation; the quaternion remains authoritative.
[[nodiscard]] inline EulerXYZ principal_fixed_xyz(Rotation3 q) {
    const auto R=q.matrix();
    const double sy=std::clamp(-R.x.z,-1.0,1.0);
    const double cy=std::hypot(R.x.x,R.x.y);
    const double y=std::atan2(sy,cy);
    if(cy>1e-8)
        return {std::atan2(R.y.z,R.z.z),y,
                std::atan2(R.x.y,R.x.x),false};
    return {std::atan2(-R.z.y,R.y.y),y,0,true};
}

// SE(3): a proper rotation and translation, without arbitrary affine scale.
// Composition and inverse match P11 SceneAffine but preserve the rigid type.
struct RigidPose {
    Rotation3 rotation{};
    Vector3 translation{};
    [[nodiscard]] Point3 point(Point3 p) const {
        const Vector3 v=rotation.apply({p.x,p.y,p.z})+translation;
        if(!finite(v))throw std::overflow_error("nonfinite rigid point");
        return {v.x,v.y,v.z};
    }
    [[nodiscard]] RigidPose inverse() const {
        const auto r=rotation.inverse();
        return {r,-r.apply(translation)};
    }
    [[nodiscard]] SceneAffine affine() const {
        if(!finite(translation))throw std::invalid_argument("nonfinite pose");
        return {rotation.matrix(),translation};
    }
    [[nodiscard]] friend RigidPose operator*(RigidPose a,RigidPose b) {
        const Vector3 t=a.rotation.apply(b.translation)+a.translation;
        if(!finite(t))throw std::overflow_error("nonfinite pose composition");
        return {a.rotation*b.rotation,t};
    }
};
[[nodiscard]] inline RigidPose interpolate_pose(RigidPose a,RigidPose b,double t) {
    // Separate translation interpolation and short-arc orientation
    // interpolation: not an SE(3) screw interpolation or motion planner.
    const auto r=Rotation3::slerp(a.rotation,b.rotation,t);
    const auto p=(1-t)*a.translation+t*b.translation;
    if(!finite(p))throw std::overflow_error("nonfinite interpolated position");
    return {r,p};
}
} // namespace book03
