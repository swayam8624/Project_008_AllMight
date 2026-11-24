#pragma once
// P21 front-end adapter: reuse actual P10 importer, P16 vertex stage and
// P17 homogeneous clipper. No second scene transformation or clipping law.
#include "book03/primitive_clipping.hpp"
#include "book03/software_renderer.hpp"
namespace book03 {
struct FrontendStats {std::size_t vertices{},input_triangles{},clipped_away{},fan_triangles{};};
inline FrontendStats draw_mesh(SoftwareRenderer& r,const ObjAsset::RenderingMesh& mesh,
    SceneAffine model,RigidPose camera,ProjectionMatrix projection,Viewport viewport,
    std::uint32_t id,RenderColor color){
    VertexStage stage(model,camera,projection,viewport);
    const auto draw=stage.draw(mesh);
    FrontendStats stats{draw.invocations,draw.triangles.size(),0,0};
    for(const auto& face:draw.triangles){
        std::array<ProcessedVertex,3> v{};
        for(int i=0;i<3;++i)v[i]=*draw.vertices.at(face.vertex[i]);
        const auto polygon=clip_triangle(v);
        if(polygon.empty()){++stats.clipped_away;continue;}
        for(const auto& t:fan_triangulate(polygon)){
            std::array<RenderVertex,3> out{};
            bool uv=t[0].varying.uv.has_value();
            for(int i=0;i<3;++i){
                const auto p=to_screen_vertex(t[i],viewport);
                const auto s=*p.screen;
                const auto tex=t[i].varying.uv.value_or(UV2{});
                out[i]={{s.image.u,s.image.v},s.depth,t[i].clip.w,tex.u,tex.v};
            }
            r.draw(out,id,color,uv);++stats.fan_triangles;
        }
    }
    return stats;
}
} // namespace book03
