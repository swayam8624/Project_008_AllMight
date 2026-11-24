#pragma once
// P14 — column-vector homogeneous projection, graphics local -Z forward,
// NDC X/Y in [-1,1], depth in [0,1], viewport origin top-left, Y down.
// Clip membership is CLOSED; pixel membership is HALF-OPEN (P13).
// Matrix storage is ROW-major; multiplication still uses column vectors.
// This is a scalar reference, not a GPU API adapter or triangle clipper.
#include "book03/pinhole_camera.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
namespace book03 {
struct Homogeneous4 {double x{},y{},z{},w{};};
[[nodiscard]] inline bool finite(Homogeneous4 p){
    return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&std::isfinite(p.w);
}
struct ProjectionMatrix {
    std::array<std::array<double,4>,4> row{};
    [[nodiscard]] Homogeneous4 apply(Homogeneous4 p) const {
        if(!finite(p))throw std::invalid_argument("nonfinite homogeneous input");
        std::array<double,4> r{};
        for(int i=0;i<4;++i)
            r[i]=row[i][0]*p.x+row[i][1]*p.y+row[i][2]*p.z+row[i][3]*p.w;
        Homogeneous4 q{r[0],r[1],r[2],r[3]};
        if(!finite(q))throw std::overflow_error("nonfinite clip coordinate");
        return q;
    }
};
enum class DepthOrder {Forward,Reverse};
struct DepthMap {
    double near_plane{0.1};
    std::optional<double> far_plane{100.0};
    DepthOrder order{DepthOrder::Forward};
    void validate() const {
        if(!std::isfinite(near_plane)||near_plane<=0||
           (far_plane&&(!std::isfinite(*far_plane)||*far_plane<=near_plane)))
            throw std::invalid_argument("invalid depth interval");
    }
    // For perspective: q(d)=a+b/d, clip z=a*d+b, clip w=d.
    // Avoid f*n overflow by computing a dimensionless coefficient first.
    [[nodiscard]] std::array<double,2> perspective_coefficients() const {
        validate();
        if(!far_plane)return order==DepthOrder::Reverse?
            std::array{0.0,near_plane}:std::array{1.0,-near_plane};
        const double f=*far_plane,n=near_plane;
        const double a=n/(f-n),b=n*(1+a);
        if(!std::isfinite(a)||!std::isfinite(b))
            throw std::overflow_error("depth coefficients overflow");
        return order==DepthOrder::Reverse?std::array{-a,b}:std::array{1+a,-b};
    }
    [[nodiscard]] double perspective_depth(double d) const {
        if(!std::isfinite(d)||d<=0)throw std::invalid_argument("invalid axial depth");
        const auto [a,b]=perspective_coefficients();
        const double q=a+b/d;
        if(!std::isfinite(q))throw std::overflow_error("depth overflow");
        return q;
    }
    // Only [0,1] stored depth values. Infinite far endpoint has no finite
    // reconstructed point. nullopt means infinity, not zero depth.
    [[nodiscard]] std::optional<double> inverse_perspective(double q) const {
        validate();
        if(!std::isfinite(q)||q<0||q>1)
            throw std::invalid_argument("depth outside [0,1]");
        const double near_q=order==DepthOrder::Reverse?1:0;
        if(q==near_q)return near_plane;
        if(q==1-near_q)return far_plane;
        const auto [a,b]=perspective_coefficients();
        const double d=b/(q-a);
        if(!std::isfinite(d)||d<=0)throw std::overflow_error("inverse depth unrepresentable");
        return d;
    }
};
[[nodiscard]] inline ProjectionMatrix perspective_matrix(
    CameraIntrinsics k,DepthMap depth) {
    k.validate();const auto [a,b]=depth.perspective_coefficients();
    const double W=static_cast<double>(k.width),H=static_cast<double>(k.height);
    // u=fx*x/d-skew*y/d+cx; v=-fy*y/d+cy.
    // ndc_x=2u/W-1; ndc_y=1-2v/H; d=-z.
    ProjectionMatrix m;
    m.row={{{2*(k.fx/W),-2*(k.skew/W),1-2*(k.cx/W),0},
            {0,2*(k.fy/H),2*(k.cy/H)-1,0},
            {0,0,-a,b},{0,0,-1,0}}};
    for(const auto& r:m.row)for(double v:r)if(!std::isfinite(v))
        throw std::overflow_error("projection coefficient overflow");
    return m;
}
struct OrthoBounds {double left{-2},right{2},bottom{-1.5},top{1.5};};
[[nodiscard]] inline ProjectionMatrix orthographic_matrix(
    OrthoBounds b,DepthMap depth) {
    depth.validate();
    if(!depth.far_plane)throw std::invalid_argument("orthographic needs finite far");
    const double dx=b.right-b.left,dy=b.top-b.bottom;
    if(!std::isfinite(dx)||!std::isfinite(dy)||dx<=0||dy<=0)
        throw std::invalid_argument("invalid orthographic bounds");
    const double n=depth.near_plane,f=*depth.far_plane,D=f-n;
    const bool rev=depth.order==DepthOrder::Reverse;
    ProjectionMatrix m;
    m.row={{{2/dx,0,0,-b.left/dx-b.right/dx},
            {0,2/dy,0,-b.bottom/dy-b.top/dy},
            {0,0,rev?1/D:-1/D,rev?f/D:-n/D},
            {0,0,0,1}}};
    for(const auto& r:m.row)for(double v:r)if(!std::isfinite(v))
        throw std::overflow_error("orthographic coefficient overflow");
    return m;
}
struct ClipClassification {
    std::array<bool,6> pass{}; // left,right,bottom,top,z-min,z-max
    bool positive_w{};
    [[nodiscard]] bool inside() const {
        return positive_w&&std::all_of(pass.begin(),pass.end(),[](bool b){return b;});
    }
};
[[nodiscard]] inline ClipClassification classify_clip(Homogeneous4 p) {
    if(!finite(p))throw std::invalid_argument("nonfinite clip coordinate");
    return {{{p.x>=-p.w,p.x<=p.w,p.y>=-p.w,p.y<=p.w,p.z>=0,p.z<=p.w}},p.w>0};
}
struct NdcPoint {double x{},y{},z{};};
[[nodiscard]] inline NdcPoint perspective_divide(Homogeneous4 p) {
    if(!finite(p)||p.w<=0)throw std::domain_error("nonpositive or invalid clip w");
    NdcPoint n{p.x/p.w,p.y/p.w,p.z/p.w};
    if(!std::isfinite(n.x)||!std::isfinite(n.y)||!std::isfinite(n.z))
        throw std::overflow_error("NDC overflow");
    return n;
}
struct ScreenPoint {ImageSample image{};double depth{};};
struct Viewport {
    double x{},y{},width{640},height{480};
    void validate() const {
        if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(width)||
           !std::isfinite(height)||width<=0||height<=0)
            throw std::invalid_argument("invalid viewport");
    }
    [[nodiscard]] ScreenPoint map(NdcPoint p) const {
        validate();
        ScreenPoint q{{x+(0.5*p.x+0.5)*width,
                       y+(0.5-0.5*p.y)*height},p.z};
        if(!std::isfinite(q.image.u)||!std::isfinite(q.image.v)||!std::isfinite(q.depth))
            throw std::overflow_error("nonfinite viewport output");
        return q;
    }
};
struct ProjectionTrace {
    Homogeneous4 clip{};
    ClipClassification classification{};
    std::optional<NdcPoint> ndc{};
    std::optional<ScreenPoint> screen{};
};
[[nodiscard]] inline ProjectionTrace trace_projection(
    ProjectionMatrix m,Viewport viewport,Point3 local) {
    viewport.validate();
    ProjectionTrace t;
    t.clip=m.apply({local.x,local.y,local.z,1});
    t.classification=classify_clip(t.clip);
    // No ordinary screen point is produced for rejected clip coordinates.
    // The next chapter will clip primitives, not discard every crossing face.
    if(t.classification.inside()){
        t.ndc=perspective_divide(t.clip);t.screen=viewport.map(*t.ndc);
    }
    return t;
}
} // namespace book03
