// Book 03 / P10 — native C++23 Asset Trust Studio.
//
// A real OBJ inspection tool using the P10 limited grammar and P07 indexed
// mesh topology, not a decorative viewer or a full general 3D engine.
//
//   book03_assets demo seam|quad|broken|repeated
//   book03_assets inspect FILE.obj
//   book03_assets svg INPUT.obj OUTPUT.svg
//   book03_assets gpu INPUT.obj OUTPUT.csv
//   book03_assets layout
//   book03_assets repl
//
// "gpu" exports one unified render vertex per distinct (p,UV,normal) tuple,
// while topological_mesh() references only the original position IDs.
// The file loader enforces a bounded source size, and the parser bounds
// individual line sizes *while reading*, counts and face dimensions.
// It does NOT parse GLB/glTF JSON, MTL files, general concave N-gons,
// animation, skinning, scene units, or authoring coordinate conventions.
// No topology repairs happen without a separate explicit user policy.

#include "book03/asset_import.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::ObjAsset;

[[nodiscard]] std::string demo_source(std::string_view name) {
    if(name=="seam") return
        "# One indexed square; different UV and normal tuples on shared corners\n"
        "v 0 0 0\nv 2 0 0\nv 2 2 0\nv 0 2 0\n"
        "vt 0 0\nvt 1 0\nvt 1 1\n"
        "vt 0.2 0\nvt 0.8 0.8\nvt 0.2 1\n"
        "vn 0 0 1\nvn 0 0 -1\n"
        "f 1/1/1 2/2/1 3/3/1\n"
        "f 1/4/2 3/5/2 4/6/2\n";
    if(name=="quad") return
        "# One convex four-corner OBJ polygon becomes two indexed triangles\n"
        "v 0 0 0\nv 2 0 0\nv 2 2 0\nv 0 2 0\n"
        "f 1 2 3 4\n";
    if(name=="broken") return
        "# The parser accepts legal faces, but P07 detects bad winding\n"
        "v 0 0 0\nv 2 0 0\nv 2 2 0\nv 0 2 0\n"
        "f 1 2 3\nf 1 4 3\n";
    if(name=="repeated") return
        "# Legal face grammar is not proof of a valid surface\n"
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "f 1 2 2\n";
    throw std::invalid_argument(
        "demo name must be seam, quad, broken or repeated");
}
[[nodiscard]] ObjAsset load_file(const std::string& filename) {
    const std::filesystem::path path{filename};
    const auto size=std::filesystem::file_size(path);
    if(size>5u*1024u*1024u)
        throw std::invalid_argument(
            "OBJ teaching studio rejects files above 5 MiB");
    std::ifstream in(path,std::ios::binary);
    if(!in)throw std::runtime_error("cannot read requested OBJ");
    return book03::import_obj(in);
}
[[nodiscard]] ObjAsset demo_asset(std::string_view name) {
    std::istringstream in{demo_source(name)};
    return book03::import_obj(in);
}
void report(std::ostream& out,const ObjAsset& asset) {
    const auto r=book03::inspect_obj_asset(asset);
    const auto& m=r.topology;
    out<<std::fixed<<std::setprecision(6)
       <<"Book03/P10 imported OBJ subset\n"
       <<"Position vertices="<<r.geometric_vertices
       <<" rendering-tuple vertices="<<r.rendering_vertices
       <<" triangular faces="<<r.triangles<<"\n"
       <<"Position/UV/normal streams="
       <<asset.positions.size()<<"/"<<asset.texcoords.size()<<"/"
       <<asset.normals.size()<<"\n"
       <<"Indexed V="<<m.referenced_vertex_count
       <<" E="<<m.edge_count
       <<" F="<<m.face_count
       <<" Euler="<<m.euler_characteristic<<"\n"
       <<"Boundary="<<m.boundary_edges
       <<" consistent-interior="<<m.consistent_interior_edges
       <<" winding-conflict="<<m.winding_conflict_edges
       <<" nonmanifold="<<m.nonmanifold_edges<<"\n"
       <<"Face components="<<m.face_edge_components
       <<" disconnected-fans="<<m.disconnected_vertex_fans
       <<" repeated-index-faces="<<m.repeated_index_faces
       <<" geometric-zero-area="<<m.zero_area_faces
       <<" unused-positions="<<m.unused_vertices<<"\n"
       <<"AABB min=("<<r.bounds_min.x<<","<<r.bounds_min.y
       <<","<<r.bounds_min.z<<") max=("<<r.bounds_max.x
       <<","<<r.bounds_max.y<<","<<r.bounds_max.z<<")\n"
       <<"Ignored-metadata warnings="<<r.warnings
       <<" invalid-supplied-normals="<<r.invalid_supplied_normals<<"\n"
       <<"Closed orientable EDGE candidate="
       <<(m.closed_orientable_edge_candidate()?"yes":"no")
       <<" (not a physical embedding or manifold proof)\n";
    for(const auto& warning:asset.warnings)
        out<<"WARNING "<<warning<<'\n';
    if(m.repeated_index_faces||m.zero_area_faces||
       m.winding_conflict_edges||m.nonmanifold_edges||
       m.disconnected_vertex_fans)
        out<<"AUDIT FAILED: topology/geometry has reported issues; "
              "no automatic repair was applied.\n";
    else
        out<<"AUDIT: no listed local defect found. "
              "This does NOT certify collision or physical solidity.\n";
}
void gpu_csv(std::ostream& out,const ObjAsset& asset) {
    const auto gpu=asset.to_rendering_mesh();
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)
       <<"gpu_id,position_id,uv_id,normal_id,x,y,z,u,v,nx,ny,nz\n";
    for(std::size_t i=0;i<gpu.vertices.size();++i) {
        const auto& v=gpu.vertices[i];
        out<<i<<","<<v.key.position<<",";
        if(v.key.texcoord)out<<*v.key.texcoord;
        out<<",";
        if(v.key.normal)out<<*v.key.normal;
        out<<","<<v.position.x<<","<<v.position.y<<","
           <<v.position.z<<",";
        if(v.uv)out<<v.uv->u<<","<<v.uv->v;
        else out<<",";
        out<<",";
        if(v.normal)out<<v.normal->x<<","<<v.normal->y
                        <<","<<v.normal->z;
        else out<<",,";
        out<<'\n';
    }
    if(!out)throw std::runtime_error("cannot write GPU tuple CSV");
}
struct Page {double x{},y{};};
[[nodiscard]] Page project(book03::Point3 p) {
    const Page q{
        255.0+140.0*p.x-45.0*p.z,
        450.0-140.0*p.y+17.0*p.z
    };
    if(!std::isfinite(q.x)||!std::isfinite(q.y))
        throw std::overflow_error("cannot draw extreme OBJ coordinates");
    return q;
}
void svg(std::ostream& out,const ObjAsset& asset) {
    const auto mesh=asset.topological_mesh();
    const auto result=book03::inspect_obj_asset(asset);
    const auto& report=result.topology;
    out<<std::fixed<<std::setprecision(3)
       <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       <<"<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "width=\"1090\" height=\"650\" viewBox=\"0 0 1090 650\" "
         "role=\"img\" aria-labelledby=\"title desc\">\n"
       <<"<title id=\"title\">OBJ import: position graph versus "
         "rendering corner tuples</title>\n"
       <<"<desc id=\"desc\">Diagram generated from actual imported OBJ "
         "position IDs, triangles and unique indexed edges. "
         "Positions remain topologically shared even if independent "
         "normal and UV indices force different GPU render vertices. "
         "Boundary edges are orange, consistent two-face edges green, "
         "winding conflict edges red and nonmanifold edges purple. "
         "The drawing is an illustrative oblique projection with no "
         "camera or visibility guarantees.</desc>\n"
       <<"<rect width=\"1090\" height=\"650\" rx=\"15\" fill=\"#fbfaf7\"/>\n"
       <<"<text x=\"28\" y=\"40\" font-family=\"sans-serif\" "
         "font-size=\"23\" font-weight=\"600\" fill=\"#284c57\">"
         "An OBJ can parse and still be a broken mesh</text>\n"
       <<"<text x=\"28\" y=\"63\" font-family=\"sans-serif\" "
         "font-size=\"13\" fill=\"#667d83\">"
         "Topology follows shared position IDs; shading follows "
         "distinct per-corner attribute tuples</text>\n"
       <<"<rect x=\"21\" y=\"90\" width=\"745\" height=\"449\" "
         "rx=\"13\" fill=\"#eef4ef\"/>\n";
    for(std::size_t i=0;i<mesh.triangles.size();++i) {
        const auto& face=mesh.triangles[i].vertex;
        const Page a=project(mesh.positions[face[0]]);
        const Page b=project(mesh.positions[face[1]]);
        const Page c=project(mesh.positions[face[2]]);
        out<<"<polygon points=\""<<a.x<<","<<a.y<<" "
           <<b.x<<","<<b.y<<" "<<c.x<<","<<c.y
           <<"\" fill=\""<<(i%2?"#c5dfd1":"#d7e9df")
           <<"\" fill-opacity=\"0.38\"/>\n";
    }
    for(const auto& edge:report.edges) {
        const Page a=project(mesh.positions[edge.key.lo]);
        const Page b=project(mesh.positions[edge.key.hi]);
        const char* color=
            edge.kind==book03::EdgeKind::Boundary?"#b78254":
            edge.kind==book03::EdgeKind::WindingConflict?"#b94b4b":
            edge.kind==book03::EdgeKind::NonManifold?"#9456a7":
            "#327e71";
        out<<"<line x1=\""<<a.x<<"\" y1=\""<<a.y
           <<"\" x2=\""<<b.x<<"\" y2=\""<<b.y
           <<"\" stroke=\""<<color<<"\" stroke-width=\"3\"/>\n";
    }
    for(std::size_t i=0;i<mesh.positions.size();++i) {
        const auto p=project(mesh.positions[i]);
        out<<"<circle cx=\""<<p.x<<"\" cy=\""<<p.y
           <<"\" r=\"5\" fill=\"#2f746f\" stroke=\"#fff\"/>\n"
           <<"<text x=\""<<p.x+10<<"\" y=\""<<p.y-10
           <<"\" font-size=\"13\" font-family=\"monospace\" "
             "fill=\"#315a66\">v"<<i<<"</text>\n";
    }
    out<<"<rect x=\"785\" y=\"105\" width=\"279\" height=\"415\" "
         "rx=\"12\" fill=\"#e8f1ec\"/>\n";
    auto label=[&](int y,const std::string& str) {
        out<<"<text x=\"800\" y=\""<<y<<"\" "
              "font-size=\"13\" font-family=\"monospace\" "
              "fill=\"#375f69\">"<<str<<"</text>\n";
    };
    label(143,"Geometric positions: "+std::to_string(result.geometric_vertices));
    label(175,"GPU corner tuples: "+std::to_string(result.rendering_vertices));
    label(210,"Unique indexed edges: "+std::to_string(report.edge_count));
    label(240,"Triangle faces: "+std::to_string(report.face_count));
    label(276,"Euler: "+std::to_string(report.euler_characteristic));
    label(311,"Boundary: "+std::to_string(report.boundary_edges));
    label(344,"Winding conflict: "+std::to_string(report.winding_conflict_edges));
    label(376,"Nonmanifold: "+std::to_string(report.nonmanifold_edges));
    label(423,"Red = wrong directed use");
    label(451,"Orange = open boundary");
    label(478,"Green = valid interior");
    out<<"<rect x=\"25\" y=\"558\" width=\"1040\" height=\"63\" "
         "rx=\"10\" fill=\"#e5eee9\"/>\n"
       <<"<text x=\"43\" y=\"584\" font-family=\"sans-serif\" "
         "font-size=\"13\" fill=\"#3f626a\">"
         "One position can have many UV or normal tuples without "
         "becoming many topological vertices.</text>\n"
       <<"<text x=\"43\" y=\"606\" font-family=\"sans-serif\" "
         "font-size=\"12\" fill=\"#657d82\">"
         "A valid triangle parser is not a watertightness, physical "
         "units, collision or scene-graph verifier.</text>\n"
       <<"</svg>\n";
    if(!out)throw std::runtime_error("OBJ audit SVG write failed");
}
template<class Writer>
void save(const std::string& path,const ObjAsset& asset,
          Writer&& writer) {
    std::ofstream out(std::filesystem::path{path},
                      std::ios::binary|std::ios::trunc);
    if(!out)throw std::runtime_error("unable to create requested output");
    writer(out,asset);
    out.flush();
    if(!out)throw std::runtime_error("unable to finish requested output");
    std::cout<<"Wrote "<<path<<'\n';
}
void layout_example() {
    const book03::GltfBufferView view{
        .byte_offset=16,.byte_length=128,.byte_stride=24
    };
    const book03::GltfAccessor accessor{
        .component=book03::GltfComponent::Float,
        .type=book03::GltfType::Vec3,
        .byte_offset=8,.count=5
    };
    const auto layout=book03::check_gltf_layout(256,view,accessor);
    std::cout<<"glTF byte example: base buffer 256 bytes\n"
             <<"bufferView offset="<<view.byte_offset
             <<" length="<<view.byte_length
             <<" stride="<<view.byte_stride<<"\n"
             <<"accessor relative offset="<<accessor.byte_offset
             <<" count="<<accessor.count<<" VEC3 FLOAT\n"
             <<"first byte="<<layout.first_byte
             <<" element size="<<layout.element_size
             <<" effective stride="<<layout.effective_stride
             <<" after last byte="<<layout.after_last_byte<<"\n"
             <<"This checks layout only; no JSON/GLB or bytes decoded.\n";
}
void repl() {
    ObjAsset current=demo_asset("seam");
    std::cout<<"Book03/P10 Asset Trust Studio\n"
                "Commands: demo seam|quad|broken|repeated | file FILE.obj "
                "| inspect | svg OUT.svg | gpu OUT.csv | layout "
                "| help | quit\n";
    std::string line;
    while(std::cout<<"asset> " && std::getline(std::cin,line)) {
        std::istringstream in{line};
        std::string cmd;in>>cmd;
        if(cmd=="quit"||cmd=="exit")break;
        try {
            if(cmd=="help")
                std::cout<<"demo NAME | file FILE.obj | inspect | "
                           "svg OUT.svg | gpu OUT.csv | layout | quit\n";
            else if(cmd=="inspect")report(std::cout,current);
            else if(cmd=="layout")layout_example();
            else if(cmd=="demo"||cmd=="file") {
                std::string arg,extra;
                if(!(in>>arg)||(in>>extra))
                    throw std::invalid_argument("one demo name or file path");
                const ObjAsset candidate=
                    cmd=="demo"?demo_asset(arg):load_file(arg);
                current=candidate; // transactional update on parse success
                std::cout<<"Loaded "<<cmd<<" "<<arg<<'\n';
            } else if(cmd=="svg"||cmd=="gpu") {
                std::string out,extra;
                if(!(in>>out)||(in>>extra))
                    throw std::invalid_argument("one output path");
                if(cmd=="svg")save(out,current,svg);
                else save(out,current,gpu_csv);
            } else if(!cmd.empty())std::cout<<"Unknown command; use help.\n";
        } catch(const std::exception& error) {
            std::cout<<"Rejected: "<<error.what()<<'\n';
        }
    }
}
void usage() {
    std::cerr<<"Usage: book03_assets demo NAME | inspect FILE.obj | "
                "svg INPUT.obj OUTPUT.svg | gpu INPUT.obj OUTPUT.csv | "
                "layout | repl\n";
}
} // namespace

int main(int argc,char** argv) {
    if(argc<2){usage();return 2;}
    try {
        const std::string_view cmd{argv[1]};
        if(cmd=="layout"&&argc==2)layout_example();
        else if(cmd=="repl"&&argc==2)repl();
        else if(cmd=="demo"&&argc==3)
            report(std::cout,demo_asset(argv[2]));
        else if(cmd=="inspect"&&argc==3)
            report(std::cout,load_file(argv[2]));
        else if(cmd=="svg"&&argc==4)
            save(argv[3],load_file(argv[2]),svg);
        else if(cmd=="gpu"&&argc==4)
            save(argv[3],load_file(argv[2]),gpu_csv);
        else{usage();return 2;}
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P10 error: "<<error.what()<<'\n';
        return 1;
    }
}
