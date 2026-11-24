#pragma once
// P13: calibrated ideal pinhole, no distortion or GPU clip/depth convention.
// Pose is camera-to-world, right-handed local +X right, +Y up, -Z forward.
// Optical coordinates (X,Y,Z)=(local.x,-local.y,-local.z), right/down/forward.
// Image continuous coordinates are measured from top-left image EDGE;
// integer pixel (i,j) is sampled at (i+0.5,j+0.5). Pixel units, metres.
#include "book03/rotation3.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace book03 {
struct CameraIntrinsics {
    std::size_t width{640},height{480};
    double fx{400},fy{400},cx{320},cy{240},skew{};
    void validate() const {
        if(width==0||height==0||width>1000000||height>1000000||
           !std::isfinite(fx)||!std::isfinite(fy)||fx<=0||fy<=0||
           !std::isfinite(cx)||!std::isfinite(cy)||!std::isfinite(skew))
            throw std::invalid_argument("invalid pinhole intrinsics");
    }
    [[nodiscard]] static CameraIntrinsics centered_fov(
        std::size_t w,std::size_t h,double vertical_radians) {
        if(w==0||h==0||!std::isfinite(vertical_radians)||
           vertical_radians<=0||vertical_radians>=std::numbers::pi)
            throw std::invalid_argument("invalid vertical FOV");
        const double f=0.5*static_cast<double>(h)/std::tan(vertical_radians/2);
        CameraIntrinsics k{w,h,f,f,0.5*static_cast<double>(w),
                          0.5*static_cast<double>(h),0};
        k.validate();return k;
    }
    // Crop shifts the coordinate origin; resizing scales the two pixel axes.
    // This is edge-coordinate geometry, not a particular resampling kernel.
    [[nodiscard]] CameraIntrinsics crop(std::size_t left,std::size_t top,
        std::size_t w,std::size_t h) const {
        validate();
        if(left>width||top>height||w>width-left||h>height-top||w==0||h==0)
            throw std::out_of_range("crop outside sensor");
        CameraIntrinsics k{w,h,fx,fy,cx-static_cast<double>(left),
                           cy-static_cast<double>(top),skew};
        k.validate();return k;
    }
    [[nodiscard]] CameraIntrinsics resize(std::size_t w,std::size_t h) const {
        validate();
        if(w==0||h==0)throw std::invalid_argument("empty resized image");
        const double sx=static_cast<double>(w)/static_cast<double>(width);
        const double sy=static_cast<double>(h)/static_cast<double>(height);
        CameraIntrinsics k{w,h,sx*fx,sy*fy,sx*cx,sy*cy,sx*skew};
        k.validate();return k;
    }
};
struct ImageSample {double u{},v{};};
struct CameraProjection {
    ImageSample image{};
    double axial_depth{}; // optical Z, NOT ray range or hardware depth
    bool inside_image{};
};
struct PinholeCamera {
    CameraIntrinsics intrinsics{};
    RigidPose camera_to_world{};
    // Do not conflate local graphics coordinates with optical coordinates.
    [[nodiscard]] Point3 optical(Point3 world) const {
        const auto p=camera_to_world.inverse().point(world);
        return {p.x,-p.y,-p.z};
    }
    [[nodiscard]] CameraProjection project(Point3 world) const {
        intrinsics.validate();
        const auto p=optical(world);
        if(!(p.z>0))throw std::domain_error("point at or behind pinhole");
        const double x=p.x/p.z,y=p.y/p.z;
        const double u=intrinsics.fx*x+intrinsics.skew*y+intrinsics.cx;
        const double v=intrinsics.fy*y+intrinsics.cy;
        if(!std::isfinite(u)||!std::isfinite(v))
            throw std::overflow_error("nonfinite pinhole projection");
        return {{u,v},p.z,u>=0&&v>=0&&
            u<static_cast<double>(intrinsics.width)&&
            v<static_cast<double>(intrinsics.height)};
    }
    // Invert upper triangular K. Off-image continuous samples are legal.
    [[nodiscard]] Vector3 optical_direction(ImageSample p) const {
        intrinsics.validate();
        if(!std::isfinite(p.u)||!std::isfinite(p.v))
            throw std::invalid_argument("nonfinite image sample");
        const double y=(p.v-intrinsics.cy)/intrinsics.fy;
        const double x=(p.u-intrinsics.cx-intrinsics.skew*y)/intrinsics.fx;
        if(!std::isfinite(x)||!std::isfinite(y))
            throw std::overflow_error("nonfinite inverse intrinsics");
        return {x,y,1};
    }
    [[nodiscard]] Ray3 ray(ImageSample p) const {
        const auto d=optical_direction(p);
        // Normalize before rotating, using a scale-first direction.
        const double m=std::max({std::abs(d.x),std::abs(d.y),1.0});
        const auto direction=camera_to_world.rotation.apply(
            unit({d.x/m,-d.y/m,-1/m}));
        return Ray3::from(camera_to_world.point({0,0,0}),direction);
    }
    [[nodiscard]] Ray3 pixel_ray(std::size_t i,std::size_t j) const {
        intrinsics.validate();
        if(i>=intrinsics.width||j>=intrinsics.height)
            throw std::out_of_range("pixel index outside image");
        return ray({static_cast<double>(i)+0.5,static_cast<double>(j)+0.5});
    }
    [[nodiscard]] Point3 unproject_axial(ImageSample p,double z) const {
        if(!std::isfinite(z)||z<=0)
            throw std::invalid_argument("axial depth must be positive finite");
        const auto d=optical_direction(p);
        return camera_to_world.point({z*d.x,-z*d.y,-z});
    }
    // Rows of the 2x3 derivative with respect to OPTICAL coordinates.
    // This is not a world-space or pose-parameter Jacobian.
    [[nodiscard]] std::array<Vector3,2> optical_jacobian(Point3 p) const {
        intrinsics.validate();
        if(!finite(p)||p.z<=0)throw std::invalid_argument("invalid optical point");
        const double a=intrinsics.fx/p.z,b=intrinsics.skew/p.z,c=intrinsics.fy/p.z;
        const Vector3 du{a,b,-(a*(p.x/p.z)+b*(p.y/p.z))};
        const Vector3 dv{0,c,-c*(p.y/p.z)};
        if(!finite(du)||!finite(dv))throw std::overflow_error("nonfinite Jacobian");
        return {du,dv};
    }
};
} // namespace book03
