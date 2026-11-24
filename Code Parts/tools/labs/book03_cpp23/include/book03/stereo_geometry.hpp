#pragma once
// P15: calibrated stereo geometry, not an automatic correspondence matcher.
// Camera-to-world poses and edge-coordinate samples follow P13. Metres,
// pixels, unit world rays. Closest-ray midpoint is a geometric estimator,
// not a maximum-likelihood solution in pixel measurement space.
#include "book03/pinhole_camera.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
namespace book03 {
struct RectifiedStereo {
    // Both cameras have identical orientation and compatible intrinsics.
    // Right center is baseline metres along left camera-local +X.
    CameraIntrinsics left{},right{};
    RigidPose left_pose{};
    double baseline{0.2};
    void validate() const {
        left.validate();right.validate();
        if(!std::isfinite(baseline)||baseline<=0||
           left.fx!=right.fx||left.fy!=right.fy||
           left.skew!=0||right.skew!=0)
            throw std::invalid_argument("rectified stereo requires positive baseline, shared focal scales and zero skew");
    }
    [[nodiscard]] PinholeCamera left_camera() const {
        validate();return {left,left_pose};
    }
    [[nodiscard]] PinholeCamera right_camera() const {
        validate();
        return {right,left_pose*RigidPose{Rotation3{}, {baseline,0,0}}};
    }
    // Principal point offsets can differ. Subtract them BEFORE disparity.
    [[nodiscard]] double disparity(ImageSample l,ImageSample r) const {
        validate();
        const double d=(l.u-left.cx)-(r.u-right.cx);
        if(!std::isfinite(d))throw std::invalid_argument("invalid disparity");
        return d;
    }
    [[nodiscard]] double row_residual(ImageSample l,ImageSample r) const {
        validate();
        const double e=(l.v-left.cy)-(r.v-right.cy);
        if(!std::isfinite(e))throw std::invalid_argument("invalid row residual");
        return e;
    }
    [[nodiscard]] double axial_depth(double d) const {
        validate();
        if(!std::isfinite(d)||d<=0)
            throw std::domain_error("nonpositive corrected disparity");
        const double z=left.fx*(baseline/d);
        if(!std::isfinite(z)||z<=0)throw std::overflow_error("unrepresentable stereo depth");
        return z;
    }
    [[nodiscard]] Point3 reconstruct(ImageSample l,ImageSample r,
                                     double max_row_error=0.5) const {
        if(!std::isfinite(max_row_error)||max_row_error<0)
            throw std::invalid_argument("invalid row tolerance");
        if(std::abs(row_residual(l,r))>max_row_error)
            throw std::domain_error("rectified row mismatch");
        return left_camera().unproject_axial(l,axial_depth(disparity(l,r)));
    }
    // First-order uncertainty, conditional on correct matching and exact
    // calibration. sigma_d is disparity stddev, not per-camera stddev.
    [[nodiscard]] double depth_sigma(double d,double sigma_d) const {
        if(!std::isfinite(sigma_d)||sigma_d<0)
            throw std::invalid_argument("invalid disparity uncertainty");
        const double z=axial_depth(d),s=z*(sigma_d/d);
        if(!std::isfinite(s))throw std::overflow_error("uncertainty overflow");
        return s;
    }
};
enum class TriangulationKind {Estimate,WeakParallax,BehindCamera};
struct TriangulationResult {
    TriangulationKind kind{TriangulationKind::WeakParallax};
    std::optional<Point3> point{};
    double left_range{},right_range{},ray_gap{},sin_angle{};
};
// Minimize ||C1+s*u - C2-t*v||^2 over two infinite supporting lines.
// Reject near-parallel AND near-antiparallel directions; both are singular.
// Cheirality rejects nonpositive fitted ray parameters. A small gap is not
// proof of a correct correspondence. The angular threshold is a policy.
[[nodiscard]] inline TriangulationResult triangulate_rays(
    const Ray3& l,const Ray3& r,double min_sin_angle=1e-6) {
    l.validate();r.validate();
    if(!std::isfinite(min_sin_angle)||min_sin_angle<=0||min_sin_angle>=1)
        throw std::invalid_argument("invalid parallax threshold");
    const auto u=l.direction,v=r.direction,b=r.origin-l.origin;
    if(!finite(b))throw std::overflow_error("baseline displacement overflow");
    const auto n=cross(u,v);
    const double sin_angle=magnitude(n);
    TriangulationResult out;
    out.sin_angle=sin_angle;
    if(sin_angle<=min_sin_angle)return out;
    // Cross-product formulas avoid directly subtracting 1-(u dot v)^2.
    const double den=dot(n,n);
    out.left_range=dot(cross(b,v),n)/den;
    out.right_range=dot(cross(b,u),n)/den;
    if(!std::isfinite(out.left_range)||!std::isfinite(out.right_range))
        throw std::overflow_error("triangulated range overflow");
    if(out.left_range<=0||out.right_range<=0){
        out.kind=TriangulationKind::BehindCamera;return out;
    }
    const Point3 a=l.at(out.left_range),c=r.at(out.right_range);
    const Vector3 gap=c-a;
    const Point3 mid=a+0.5*gap;
    if(!finite(mid))throw std::overflow_error("triangulation midpoint overflow");
    out.ray_gap=magnitude(gap);
    out.point=mid;out.kind=TriangulationKind::Estimate;
    return out;
}
struct StereoEstimate {
    TriangulationResult geometry{};
    std::optional<double> left_pixel_error{},right_pixel_error{};
};
[[nodiscard]] inline StereoEstimate triangulate_samples(
    const PinholeCamera& l,const PinholeCamera& r,ImageSample a,ImageSample b,
    double min_sin_angle=1e-6) {
    StereoEstimate out;
    out.geometry=triangulate_rays(l.ray(a),r.ray(b),min_sin_angle);
    if(!out.geometry.point)return out;
    const auto p=*out.geometry.point;
    // A closest-line midpoint is not necessarily in both forward optical
    // half-spaces, even if the two closest points have positive ranges.
    if(l.optical(p).z<=0||r.optical(p).z<=0){
        out.geometry.kind=TriangulationKind::BehindCamera;
        out.geometry.point.reset();return out;
    }
    const auto lp=l.project(p).image,rp=r.project(p).image;
    out.left_pixel_error=std::hypot(lp.u-a.u,lp.v-a.v);
    out.right_pixel_error=std::hypot(rp.u-b.u,rp.v-b.v);
    return out;
}
// World-space scalar triple product: b dot (u cross v). For unit world
// directions it has units of metres. It is zero for coplanar rays, even
// when their intersection lies behind one camera or is ill-conditioned.
// Not a normalized pixel residual and not a universal match score.
[[nodiscard]] inline double epipolar_coplanarity(
    const PinholeCamera& l,const PinholeCamera& r,ImageSample a,ImageSample b) {
    const auto u=l.ray(a),v=r.ray(b);
    return dot(v.origin-u.origin,cross(u.direction,v.direction));
}
} // namespace book03
