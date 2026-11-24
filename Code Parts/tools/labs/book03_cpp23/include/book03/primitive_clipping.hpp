#pragma once
// P17: bounded floating reference, not certified watertight clipping.
// Closed continuous half-spaces; raster sample ownership comes in P18.
// Raw smooth attributes interpolate in the ORIGINAL edge parameter.
// Generated normals are intentionally NOT normalized until fragment sampling.
#include "book03/vertex_pipeline.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>
namespace book03 {
struct Window2 {double left{},top{},right{640},bottom{480};};
struct Line2 {ImageSample a{},b{};};
inline void validate_window_line(Window2 w,Line2 l){
    for(double x:{w.left,w.top,w.right,w.bottom,l.a.u,l.a.v,l.b.u,l.b.v})
        if(!std::isfinite(x)||std::abs(x)>1e12)
            throw std::invalid_argument("nonfinite or out-of-domain window coordinate");
    if(w.left>=w.right||w.top>=w.bottom)
        throw std::invalid_argument("empty window");
}
[[nodiscard]] inline unsigned window_outcode(Window2 w,ImageSample p){
    return (p.u<w.left?1u:0u)|(p.u>w.right?2u:0u)|
           (p.v<w.top?4u:0u)|(p.v>w.bottom?8u:0u);
}
[[nodiscard]] inline std::optional<Line2> cohen_sutherland(Window2 w,Line2 l){
    validate_window_line(w,l);
    for(int i=0;i<16;++i){
        const unsigned a=window_outcode(w,l.a),b=window_outcode(w,l.b);
        if(!(a|b))return l;
        if(a&b)return std::nullopt;
        const unsigned c=a?a:b;
        const auto p=l.a,q=l.b;
        ImageSample s{};
        if(c&1u||c&2u){
            s.u=(c&1u)?w.left:w.right;
            s.v=std::lerp(p.v,q.v,(s.u-p.u)/(q.u-p.u));
        }else{
            s.v=(c&4u)?w.top:w.bottom;
            s.u=std::lerp(p.u,q.u,(s.v-p.v)/(q.v-p.v));
        }
        if(a)l.a=s;else l.b=s;
    }
    throw std::runtime_error("Cohen-Sutherland did not converge in floating policy");
}
[[nodiscard]] inline std::optional<Line2> liang_barsky(Window2 w,Line2 l){
    validate_window_line(w,l);
    const double dx=l.b.u-l.a.u,dy=l.b.v-l.a.v;
    // Every plane has g(t)=g0+slope*t >= 0. Intersect its allowed interval.
    const std::array<double,4> g{l.a.u-w.left,w.right-l.a.u,
                                 l.a.v-w.top,w.bottom-l.a.v};
    const std::array<double,4> slope{dx,-dx,dy,-dy};
    double lo=0,hi=1;
    for(std::size_t i=0;i<4;++i){
        if(slope[i]==0){if(g[i]<0)return std::nullopt;continue;}
        const double t=-g[i]/slope[i];
        if(slope[i]>0)lo=std::max(lo,t);else hi=std::min(hi,t);
        if(lo>hi)return std::nullopt;
    }
    // Clamp only final coordinate rounding at a known accepted interval.
    auto at=[&](double t){return ImageSample{
        std::clamp(std::lerp(l.a.u,l.b.u,t),w.left,w.right),
        std::clamp(std::lerp(l.a.v,l.b.v,t),w.top,w.bottom)};};
    return Line2{at(lo),at(hi)};
}

enum class ClipPlane {Left,Right,Bottom,Top,ZMin,ZMax};
inline constexpr std::array clip_planes{ClipPlane::Left,ClipPlane::Right,
    ClipPlane::Bottom,ClipPlane::Top,ClipPlane::ZMin,ClipPlane::ZMax};
[[nodiscard]] inline double clip_distance(Homogeneous4 h,ClipPlane p){
    switch(p){
    case ClipPlane::Left:return h.w+h.x;
    case ClipPlane::Right:return h.w-h.x;
    case ClipPlane::Bottom:return h.w+h.y;
    case ClipPlane::Top:return h.w-h.y;
    case ClipPlane::ZMin:return h.z;
    case ClipPlane::ZMax:return h.w-h.z;
    }
    throw std::invalid_argument("unknown clip plane");
}
// A clip vertex has no fabricated original index. Barycentric provenance
// identifies the source triangle's three corners, including seam attributes.
struct ClipVertex {
    Homogeneous4 clip{};
    VertexPayload varying{};
    std::array<double,3> source_weights{};
};
using ClipPolygon=std::vector<ClipVertex>;
[[nodiscard]] inline ClipVertex clip_mix(const ClipVertex& a,const ClipVertex& b,double t){
    if(!std::isfinite(t)||t<0||t>1)throw std::invalid_argument("clip interpolation parameter");
    if(a.varying.uv.has_value()!=b.varying.uv.has_value()||
       a.varying.world_normal.has_value()!=b.varying.world_normal.has_value())
        throw std::invalid_argument("mixed clip varying presence");
    auto mix=[&](double x,double y){return std::lerp(x,y,t);};
    ClipVertex v;
    v.clip={mix(a.clip.x,b.clip.x),mix(a.clip.y,b.clip.y),
            mix(a.clip.z,b.clip.z),mix(a.clip.w,b.clip.w)};
    const auto x=a.varying.world,y=b.varying.world;
    v.varying.world={mix(x.x,y.x),mix(x.y,y.y),mix(x.z,y.z)};
    if(a.varying.uv){
        const auto u=*a.varying.uv,w=*b.varying.uv;
        v.varying.uv=UV2{mix(u.u,w.u),mix(u.v,w.v)};
    }
    if(a.varying.world_normal){
        const auto n=*a.varying.world_normal,m=*b.varying.world_normal;
        v.varying.world_normal=Vector3{mix(n.x,m.x),mix(n.y,m.y),mix(n.z,m.z)};
    }
    for(std::size_t i=0;i<3;++i)v.source_weights[i]=mix(a.source_weights[i],b.source_weights[i]);
    return v;
}
[[nodiscard]] inline ClipVertex clip_crossing(const ClipVertex& a,const ClipVertex& b,ClipPlane plane){
    const double da=clip_distance(a.clip,plane),db=clip_distance(b.clip,plane);
    if(da==0)return a;
    if(db==0)return b;
    if((da>0)==(db>0))throw std::logic_error("clip crossing requires opposite signs");
    // Always order the outside endpoint first. A reversed edge uses the
    // same floating expression; provenance remains specific to its face.
    const auto& outside=da<0?a:b;const auto& inside=da<0?b:a;
    const double u=std::abs(da<0?da:db),v=std::abs(da<0?db:da);
    const double scale=std::max(u,v);
    const double t=(u/scale)/(u/scale+v/scale);
    auto q=clip_mix(outside,inside,t);
    // Snap to the intersected plane exactly in stored coordinates.
    // This changes only the clip tuple by rounding-scale error in the
    // supported domain; it is not an exact source-geometry construction.
    switch(plane){
    case ClipPlane::Left:q.clip.x=-q.clip.w;break;
    case ClipPlane::Right:q.clip.x=q.clip.w;break;
    case ClipPlane::Bottom:q.clip.y=-q.clip.w;break;
    case ClipPlane::Top:q.clip.y=q.clip.w;break;
    case ClipPlane::ZMin:q.clip.z=0;break;
    case ClipPlane::ZMax:q.clip.z=q.clip.w;break;
    }
    return q;
}
inline void validate_clip_input(const ClipVertex& v){
    if(!finite(v.clip)||!finite(v.varying.world)||
       (v.varying.uv&&(!std::isfinite(v.varying.uv->u)||!std::isfinite(v.varying.uv->v)))||
       (v.varying.world_normal&&!finite(*v.varying.world_normal)))
        throw std::invalid_argument("nonfinite clip vertex");
    for(double x:{v.clip.x,v.clip.y,v.clip.z,v.clip.w})
        if(std::abs(x)>1e12)throw std::invalid_argument("clip coordinate exceeds teaching domain");
}
[[nodiscard]] inline ClipPolygon clip_triangle(const std::array<ProcessedVertex,3>& source){
    ClipPolygon polygon;
    for(std::size_t i=0;i<3;++i){
        ClipVertex v{source[i].clip,source[i].varying,{}};
        v.source_weights[i]=1;validate_clip_input(v);
        if(v.varying.uv.has_value()!=source[0].varying.uv.has_value()||
           v.varying.world_normal.has_value()!=source[0].varying.world_normal.has_value())
            throw std::invalid_argument("mixed triangle varying presence");
        polygon.push_back(v);
    }
    // Sutherland-Hodgman: each pass clips against ONE linear half-space.
    // Treat exactly on-plane as inside. Strict crossings generate vertices;
    // an on-plane endpoint is already emitted by its own inside case.
    for(auto plane:clip_planes){
        if(polygon.empty())break;
        ClipPolygon next;
        for(std::size_t i=0;i<polygon.size();++i){
            const auto& a=polygon[i];
            const auto& b=polygon[(i+1)%polygon.size()];
            const double da=clip_distance(a.clip,plane),db=clip_distance(b.clip,plane);
            if(da>=0)next.push_back(a);
            if((da<0&&db>0)||(da>0&&db<0))next.push_back(clip_crossing(a,b,plane));
        }
        polygon=std::move(next);
    }
    // A surviving line/point has no triangle area. Degenerate polygons
    // with >=3 entries are left to later screen-area rejection.
    if(polygon.size()<3)return {};
    // Do not silently clamp unrelated coordinates or divide at w=0.
    // Small drift across a previously clipped plane is explicit failure.
    for(const auto& v:polygon){
        validate_clip_input(v);
        if(!classify_clip(v.clip).inside())
            throw std::domain_error("clip boundary drift or nonpositive w; unresolved polygon");
    }
    return polygon;
}
[[nodiscard]] inline ProcessedVertex to_screen_vertex(const ClipVertex& v,Viewport viewport){
    ProcessedVertex p{};
    p.clip=v.clip;p.varying=v.varying;
    p.classification=classify_clip(v.clip);
    if(!p.classification.inside())throw std::domain_error("unresolved clip vertex");
    p.screen=viewport.map(perspective_divide(v.clip));
    // Only clip/varying/classification/screen are meaningful here.
    // source/local/camera are NOT reconstructed original vertex metadata.
    return p;
}
[[nodiscard]] inline std::vector<std::array<ClipVertex,3>> fan_triangulate(const ClipPolygon& p){
    std::vector<std::array<ClipVertex,3>> out;
    for(std::size_t i=1;i+1<p.size();++i)out.push_back({p[0],p[i],p[i+1]});
    return out;
}
} // namespace book03
