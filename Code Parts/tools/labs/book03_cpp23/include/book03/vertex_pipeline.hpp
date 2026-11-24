#pragma once
// P16: indexed CPU vertex-stage reference. Clip positions and raw varyings
// survive EVEN for rejected vertices; P17 will clip assembled primitives.
// One evaluation per referenced rendering ID in this immutable draw batch.
// Not a hardware vertex-cache model, material system or fragment rasterizer.
#include "book03/asset_import.hpp"
#include "book03/projection_pipeline.hpp"
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>
namespace book03 {
struct VertexPayload {
    Point3 world{};
    std::optional<UV2> uv{};
    std::optional<Vector3> world_normal{};
};
struct ProcessedVertex {
    ObjCornerKey source{};
    Point3 local{},camera{};
    VertexPayload varying{};
    Homogeneous4 clip{};
    ClipClassification classification{};
    std::optional<ScreenPoint> screen{};
};
struct VertexDraw {
    std::vector<std::optional<ProcessedVertex>> vertices;
    std::vector<TriangleIndices> triangles;
    std::size_t invocations{};
};
class VertexStage {
    SceneAffine model_;
    RigidPose camera_;
    ProjectionMatrix projection_;
    Viewport viewport_;
    std::optional<Linear3> normal_matrix_;
public:
    VertexStage(SceneAffine model,RigidPose camera,ProjectionMatrix projection,
                Viewport viewport)
        :model_(model),camera_(camera),projection_(projection),viewport_(viewport) {
        viewport_.validate();
    }
    [[nodiscard]] ProcessedVertex process(const ObjRenderingVertex& v) {
        if(!finite(v.position)||(v.uv&&(!std::isfinite(v.uv->u)||!std::isfinite(v.uv->v))))
            throw std::invalid_argument("nonfinite vertex input");
        ProcessedVertex out;
        out.source=v.key;out.local=v.position;
        out.varying.world=model_.point(v.position);
        out.varying.uv=v.uv;
        const auto center=camera_.point({0,0,0});
        const auto local=camera_.rotation.inverse().apply(out.varying.world-center);
        out.camera={local.x,local.y,local.z};
        if(v.normal){
            // Lazy per-draw inverse transpose: a position-only draw can
            // legitimately have a singular model; an authored normal cannot.
            if(!normal_matrix_)normal_matrix_=model_.linear.inverse().transpose();
            out.varying.world_normal=unit(normal_matrix_->apply(*v.normal));
        }
        const auto t=trace_projection(projection_,viewport_,out.camera);
        out.clip=t.clip;out.classification=t.classification;out.screen=t.screen;
        return out;
    }
    [[nodiscard]] VertexDraw draw(const ObjAsset::RenderingMesh& mesh) {
        // Validate all referenced IDs before assembling any output.
        for(const auto& f:mesh.triangles)for(auto id:f.vertex)
            if(id>=mesh.vertices.size())throw std::out_of_range("invalid rendering index");
        VertexDraw result;
        result.vertices.resize(mesh.vertices.size());
        result.triangles=mesh.triangles;
        for(const auto& f:mesh.triangles)for(auto id:f.vertex){
            if(!result.vertices[id]){
                result.vertices[id]=process(mesh.vertices[id]);
                ++result.invocations;
            }
        }
        return result;
    }
};
struct InterpolationSample {
    std::array<double,3> screen_weights{},surface_weights{};
    VertexPayload varying{};
    double depth{};
};
// The precondition is an already clipped triangle with positive w.
// Screen barycentrics are supplied by the caller (P18 will compute coverage).
// Smooth attributes use reciprocal-w weights. NDC depth is screen-linear.
// Missing attributes stay missing; mixed presence is an explicit error.
[[nodiscard]] inline InterpolationSample interpolate_inside_triangle(
    const std::array<ProcessedVertex,3>& v,std::array<double,3> lambda) {
    double sum=0;
    for(std::size_t i=0;i<3;++i){
        if(!v[i].screen||!v[i].classification.inside()||!finite(v[i].clip)||
           v[i].clip.w<=0||!std::isfinite(lambda[i])||lambda[i]<0||lambda[i]>1)
            throw std::invalid_argument("requires clipped positive-w triangle and interior weights");
        sum+=lambda[i];
    }
    if(std::abs(sum-1)>1e-12)throw std::invalid_argument("barycentric sum");
    // Rescale inverse w by min(w) to avoid overflow on tiny positive w.
    const double wmin=std::min({v[0].clip.w,v[1].clip.w,v[2].clip.w});
    std::array<double,3> beta{};
    double den=0;
    for(std::size_t i=0;i<3;++i){
        lambda[i]/=sum;
        beta[i]=lambda[i]*(wmin/v[i].clip.w);den+=beta[i];
    }
    if(!(den>0)||!std::isfinite(den))throw std::overflow_error("unrepresentable interpolation denominator");
    for(auto& b:beta)b/=den;
    const bool uv=v[0].varying.uv.has_value(),normal=v[0].varying.world_normal.has_value();
    InterpolationSample out{lambda,beta,{},0};
    Vector3 p{},n{};UV2 tex{};
    for(std::size_t i=0;i<3;++i){
        const auto& a=v[i].varying;
        if(a.uv.has_value()!=uv||a.world_normal.has_value()!=normal)
            throw std::invalid_argument("mixed varying presence");
        p=p+beta[i]*Vector3{a.world.x,a.world.y,a.world.z};
        if(uv){tex.u+=beta[i]*a.uv->u;tex.v+=beta[i]*a.uv->v;}
        if(normal)n=n+beta[i]*(*a.world_normal);
        out.depth+=lambda[i]*(v[i].clip.z/v[i].clip.w);
    }
    if(!finite(p)||!std::isfinite(out.depth)||
       (uv&&(!std::isfinite(tex.u)||!std::isfinite(tex.v))))
        throw std::overflow_error("nonfinite interpolated varying");
    out.varying.world={p.x,p.y,p.z};
    if(uv)out.varying.uv=tex;
    if(normal)out.varying.world_normal=unit(n); // cancellation can reject
    return out;
}
// Signed screen-space barycentrics. This bounded floating reference refuses
// near-degenerate triangles; not an exact coverage/top-left rule.
[[nodiscard]] inline std::array<double,3> screen_barycentrics(
    const std::array<ProcessedVertex,3>& v,ImageSample p) {
    for(const auto& x:v)if(!x.screen)throw std::invalid_argument("unclipped triangle");
    if(!std::isfinite(p.u)||!std::isfinite(p.v))throw std::invalid_argument("nonfinite sample");
    const auto a=v[0].screen->image,b=v[1].screen->image,c=v[2].screen->image;
    const double x=b.u-a.u,y=b.v-a.v,u=c.u-a.u,w=c.v-a.v;
    const double scale=std::max({std::abs(x),std::abs(y),std::abs(u),std::abs(w)});
    if(!(scale>0)||!std::isfinite(scale))throw std::domain_error("degenerate screen triangle");
    const double X=x/scale,Y=y/scale,U=u/scale,W=w/scale;
    const double d=X*W-Y*U;
    if(std::abs(d)<=1e-12)throw std::domain_error("uncertain screen triangle");
    const double px=(p.u-a.u)/scale,py=(p.v-a.v)/scale;
    const double l1=(px*W-py*U)/d,l2=(X*py-Y*px)/d;
    if(!std::isfinite(l1)||!std::isfinite(l2))throw std::overflow_error("barycentric overflow");
    return {1-l1-l2,l1,l2};
}
} // namespace book03
