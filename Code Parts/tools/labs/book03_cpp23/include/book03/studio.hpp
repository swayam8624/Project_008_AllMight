#pragma once
// P31: controlled optical-space fixture, not a general image importer.
#include "book03/sfm.hpp"
#include "book03/surface.hpp"
#include "book03/software_renderer.hpp"
#include <iomanip>
namespace book03::studio {
using sfm::V; using sfm::M;
inline constexpr int width=320,height=240;
inline constexpr double near_z=1,far_z=10;
inline constexpr M k{{{160,0,160},{0,160,120},{0,0,1}}};
struct Face {std::array<std::size_t,3> vertices;std::uint32_t id;RenderColor linear;};
struct Mesh {std::vector<V> points;std::vector<Face> faces;};
// Construct an optical point from a declared continuous pixel and axial depth.
inline V from_pixel(double u,double v,double z){return {(u-160)*z/160,(v-120)*z/160,z};}
inline Mesh house(){return {{from_pixel(80,100,4),from_pixel(240,100,4),from_pixel(240,200,4),from_pixel(80,200,4),from_pixel(160,40,4),from_pixel(130,135,3.8),from_pixel(190,135,3.8),from_pixel(190,175,3.8),from_pixel(130,175,3.8)},{{{0,1,2},1,{.65,.43,.22}},{{0,2,3},1,{.65,.43,.22}},{{0,4,1},2,{.50,.12,.08}},{{5,6,7},3,{.08,.40,.65}},{{5,7,8},3,{.08,.40,.65}}}};}
inline std::vector<sfm::Pose> cameras(){return {sfm::Pose{},sfm::Pose{sfm::identity,{-.4,0,0}},sfm::Pose{sfm::identity,{0,-.3,0}}};}
// The renderer consumes divided NDC Z, not optical depth. Its clip w is Z.
inline RenderVertex vertex(V p){
    for(double x:p)if(!std::isfinite(x))throw std::invalid_argument("nonfinite point");
    if(p[2]<near_z||p[2]>far_z)throw std::domain_error("requires depth clipping");
    auto q=sfm::project(sfm::Pose{},k,p);
    if(q[0]<0||q[0]>width||q[1]<0||q[1]>height)throw std::domain_error("requires side clipping");
    const double depth=far_z/(far_z-near_z)*(1-near_z/p[2]);
    return {{q[0],q[1]},std::clamp(depth,0.,1.),p[2],0,0};
}
inline SoftwareRenderer render(const Mesh& m,bool reverse=false){
    SoftwareRenderer result(width,height,{VisibilityOrder::Forward,DepthStorage::Float64,TiePolicy::LowestID});
    // Check all vertices and indices before drawing any face.
    std::vector<RenderVertex> v;for(auto p:m.points)v.push_back(vertex(p));
    for(auto f:m.faces)for(auto i:f.vertices)if(i>=v.size())throw std::out_of_range("face index");
    for(std::size_t i=0;i<m.faces.size();++i){auto f=m.faces[reverse?m.faces.size()-1-i:i];result.draw({v[f.vertices[0]],v[f.vertices[1]],v[f.vertices[2]]},f.id,f.linear,false);}
    return result;
}
struct Reconstruction {Mesh mesh;std::vector<sfm::Track> tracks;double max_3d{},rms_pixels{};};
inline Reconstruction reconstruct(const Mesh& truth){
    Reconstruction result;result.mesh.faces=truth.faces;const auto poses=cameras();const auto ki=epipolar::inverse_k(160,160,0,160,120);double squared=0;
    for(auto p:truth.points){sfm::Track tr;for(std::size_t j=0;j<poses.size();++j)tr.push_back({j,sfm::project(poses[j],k,p)});
        auto x=sfm::triangulate(poses,ki,tr);x=sfm::refine_point(poses,k,tr,x);result.max_3d=std::max(result.max_3d,sfm::norm(sfm::add(x,p,-1)));squared+=sfm::cost(poses,k,tr,x);result.mesh.points.push_back(x);result.tracks.push_back(std::move(tr));}
    if(!truth.points.empty())result.rms_pixels=std::sqrt(squared/(truth.points.size()*poses.size()));
    return result;
}
inline void obj(std::ostream& out,const Mesh& m){
    // OBJ output retains optical +Z and metres; topology is supplied, not inferred.
    out<<"# optical +X right +Y down +Z forward; metres; supplied topology\n"<<std::setprecision(17);
    for(auto p:m.points)out<<"v "<<p[0]<<' '<<p[1]<<' '<<p[2]<<'\n';
    for(auto f:m.faces){out<<"f";for(auto i:f.vertices){if(i>=m.points.size())throw std::out_of_range("face index");out<<' '<<i+1;}out<<'\n';}
    if(!out)throw std::runtime_error("OBJ output");
}
} // namespace book03::studio
