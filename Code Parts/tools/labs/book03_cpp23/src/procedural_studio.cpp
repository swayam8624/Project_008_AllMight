// Book 03 / Chapter 09 — procedural world design workshop.
//
// Pure native C++23. Every requested visualization and export uses the
// SAME real indexed Mesh3 and P07 incidence analyzer. The source SVG is a
// static PARALLEL OBLIQUE scientific diagram, not a calibrated camera,
// GPU display or general hidden-surface renderer.
//
// Terminal use:
//   book03_world inspect [terrain|cylinder|sphere|implicit]
//   book03_world svg OUTPUT.svg [shape]
//   book03_world obj OUTPUT.obj [shape]
//   book03_world csv OUTPUT.csv [shape]
//   book03_world repl
//
// REPL:
//   shape terrain|cylinder|sphere|implicit
//   resolution INTEGER
//   radius positive_metres
//   amplitude signed_metres
//   inspect | svg FILE | obj FILE | csv FILE | reset | quit
//
// This is not an OBJ or glTF asset importer. OBJ export writes 1-based
// indexed positions and triangle faces only, with no UV/material claims.
// The side/cap normal crease of a cylinder needs per-corner normal data
// in a future renderer; positions alone do not encode shading choices.

#include "book03/procedural_geometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using book03::Point3;
using book03::Mesh3;
using book03::MeshReport;

enum class Shape { Terrain, Cylinder, Sphere, Implicit };

[[nodiscard]] const char* shape_name(Shape kind) noexcept {
    switch(kind) {
        case Shape::Terrain: return "terrain";
        case Shape::Cylinder: return "cylinder";
        case Shape::Sphere: return "sphere";
        case Shape::Implicit: return "implicit";
    }
    return "unknown";
}
[[nodiscard]] Shape parse_shape(std::string_view s) {
    if(s=="terrain")return Shape::Terrain;
    if(s=="cylinder")return Shape::Cylinder;
    if(s=="sphere")return Shape::Sphere;
    if(s=="implicit")return Shape::Implicit;
    throw std::invalid_argument(
        "shape must be terrain, cylinder, sphere or implicit");
}

[[nodiscard]] double number(const std::string& s) {
    std::size_t last=0;
    const double x=std::stod(s,&last);
    if(last!=s.size()||!std::isfinite(x)||std::abs(x)>1e4)
        throw std::invalid_argument("enter finite scalar within ±10000");
    return x;
}
[[nodiscard]] std::uint32_t resolution(const std::string& s) {
    const double x=number(s);
    if(x!=std::floor(x)||x<3||x>24)
        throw std::invalid_argument("resolution must be integer 3..24");
    return static_cast<std::uint32_t>(x);
}

struct WorldState {
    Shape shape{Shape::Terrain};
    std::uint32_t divisions{12};
    double radius{0.91};
    double amplitude{0.45};
    double size{4.0};

    [[nodiscard]] Mesh3 mesh() const {
        switch(shape) {
            case Shape::Terrain:
                return book03::make_heightfield(
                    {amplitude,1.0},divisions,divisions,size,size);
            case Shape::Cylinder:
                return book03::make_capped_cylinder(
                    divisions,radius,2.0);
            case Shape::Sphere:
                return book03::make_uv_sphere(
                    divisions,std::max(2u,divisions/2),radius);
            case Shape::Implicit:
                return book03::extract_sphere_isosurface(
                    {radius},divisions,1.5).mesh;
        }
        throw std::logic_error("unhandled procedural shape");
    }
};

struct Page {double x{},y{};};
[[nodiscard]] Page project(Point3 p) {
    if(!book03::finite(p))
        throw std::invalid_argument("nonfinite projected geometry");
    const Page image{
        405.0 + 83.0*p.x-52.0*p.z,
        382.0 - 80.0*p.y+25.0*p.z+12.0*p.x
    };
    if(!std::isfinite(image.x)||!std::isfinite(image.y))
        throw std::overflow_error("diagram projection overflow");
    return image;
}

// Analyze the populated graph before moving it into the result pair:
// moving it first would make topology inspection read a moved-from mesh.
[[nodiscard]] std::pair<Mesh3,MeshReport> generated(const WorldState& state) {
    Mesh3 m=state.mesh();
    MeshReport r=book03::inspect_mesh(m);
    return {std::move(m),std::move(r)};
}

void inspect(std::ostream& out,const WorldState& world) {
    const auto [mesh,report]=generated(world);
    out<<std::fixed<<std::setprecision(9)
       <<"Book03/P09 procedural world shape="<<shape_name(world.shape)
       <<"\n"
       <<"resolution="<<world.divisions
       <<" radius="<<world.radius
       <<" metres amplitude="<<world.amplitude
       <<" metres\n"
       <<"positions="<<report.position_count
       <<" referenced="<<report.referenced_vertex_count
       <<" faces="<<report.face_count
       <<" edges="<<report.edge_count<<"\n"
       <<"Euler V-E+F="<<report.euler_characteristic<<"\n"
       <<"boundary="<<report.boundary_edges
       <<" consistent-interior="<<report.consistent_interior_edges
       <<" winding-conflict="<<report.winding_conflict_edges
       <<" nonmanifold="<<report.nonmanifold_edges<<"\n"
       <<"components="<<report.face_edge_components
       <<" disconnected-vertex-fans="<<report.disconnected_vertex_fans
       <<" repeated-index-faces="<<report.repeated_index_faces
       <<" zero-area-faces="<<report.zero_area_faces<<"\n"
       <<"closed-orientable-edge-candidate="
       <<(report.closed_orientable_edge_candidate()?"yes":"no")
       <<" (NOT global embedding or self-intersection certification)\n";
    if(world.shape==Shape::Terrain) {
        const book03::TerrainField field{world.amplitude,1.0};
        const auto normal=field.normal(0,0);
        out<<"Terrain h(0,0)="<<field.height(0,0)
           <<" analytic-up-normal=("<<normal.x<<","
           <<normal.y<<","<<normal.z<<")\n";
    }
    if(world.shape==Shape::Cylinder) {
        const auto normal=book03::cylinder_side_normal(0);
        out<<"Cylinder side normal at phi=0=("<<normal.x<<","
           <<normal.y<<","<<normal.z
           <<"), bottom cap=(0,-1,0), top cap=(0,1,0)\n";
    }
    if(world.shape==Shape::Sphere||world.shape==Shape::Implicit) {
        const book03::SphereDistance sdf{world.radius};
        out<<"Sphere SDF at origin="<<sdf.sample({0,0,0})
           <<" metres (negative inside)\n"
           <<"Sphere SDF at (radius,0,0)="
           <<sdf.sample({world.radius,0,0})<<"\n";
    }
    if(world.shape==Shape::Implicit) {
        const auto evidence=book03::extract_sphere_isosurface(
            {world.radius},world.divisions,1.5);
        out<<"Implicit lattice samples="<<evidence.lattice_samples
           <<" inside samples="<<evidence.occupied_lattice_samples
           <<" global cached edge crossings="
           <<evidence.cached_crossings<<"\n"
           <<"Implicit field interpolation and radial winding "
             "are sphere-specific: not an exact sphere.\n";
    }
}

void svg(std::ostream& out,const WorldState& world) {
    const auto [mesh,r]=generated(world);
    out<<std::fixed<<std::setprecision(4)
       <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       <<"<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "width=\"1100\" height=\"675\" viewBox=\"0 0 1100 675\" "
         "role=\"img\" aria-labelledby=\"title desc\">\n"
       <<"<title id=\"title\">Procedural "<<shape_name(world.shape)
         <<" — source-indexed mesh and topology evidence</title>\n"
       <<"<desc id=\"desc\">Generated by the C++23 P09 procedural geometry "
         "lab from the current selected shape and settings. "
         "This is an uncalibrated parallel oblique geometry diagram, not "
         "a real perspective camera, shadow renderer or certified surface "
         "intersection image. Every shown face uses the same indexed 3D "
         "mesh consumed by the independent P07 topology inspector. "
         "Terrain has open boundary; sphere and capped cylinder aim for "
         "a closed genus-zero surface. Implicit sphere is approximated "
         "through cached intersections of a tetrahedral Cartesian grid."
         "</desc>\n"
       <<"<rect width=\"1100\" height=\"675\" rx=\"18\" fill=\"#fbfaf6\"/>\n"
       <<"<text x=\"28\" y=\"40\" font-size=\"23\" "
         "font-family=\"sans-serif\" font-weight=\"600\" fill=\"#284c56\">"
         "A world made from a rule: "<<shape_name(world.shape)
         <<"</text>\n"
       <<"<text x=\"28\" y=\"63\" font-size=\"13\" "
         "font-family=\"sans-serif\" fill=\"#677e84\">"
         "Exact index identities; floating geometric samples; "
         "parallel oblique illustration</text>\n"
       <<"<rect x=\"20\" y=\"86\" width=\"787\" height=\"476\" "
         "rx=\"13\" fill=\"#ecf3ef\"/>\n";

    // Drawing is illustrative wireframe with gentle translucent fill:
    // no hidden-surface removal or claim of visual camera correctness.
    // A dense iso-mesh can be drawn without face-by-face text duplication.
    for(std::size_t f=0;f<mesh.triangles.size();++f) {
        const auto& tri=mesh.triangles[f].vertex;
        const Page a=project(mesh.positions[tri[0]]);
        const Page b=project(mesh.positions[tri[1]]);
        const Page c=project(mesh.positions[tri[2]]);
        out<<"<polygon points=\""<<a.x<<","<<a.y<<" "
           <<b.x<<","<<b.y<<" "<<c.x<<","<<c.y
           <<"\" fill=\""<<(f%2?"#b8ddd0":"#d5e8dc")
           <<"\" fill-opacity=\"0.25\" stroke=\"none\"/>\n";
    }
    for(const auto& edge:r.edges) {
        const Page a=project(mesh.positions[edge.key.lo]);
        const Page b=project(mesh.positions[edge.key.hi]);
        out<<"<line x1=\""<<a.x<<"\" y1=\""<<a.y
           <<"\" x2=\""<<b.x<<"\" y2=\""<<b.y
           <<"\" stroke=\""
           <<(edge.kind==book03::EdgeKind::Boundary?
                    "#b57854":"#6caa98")
           <<"\" stroke-width=\"0.85\"/>\n";
    }

    out<<"<rect x=\"830\" y=\"100\" width=\"248\" height=\"453\" "
          "rx=\"12\" fill=\"#e8f1ec\"/>\n";
    auto label=[&](int y,std::string s) {
        out<<"<text x=\"844\" y=\""<<y<<"\" "
              "font-family=\"monospace\" font-size=\"13\" "
              "fill=\"#355f67\">"<<s<<"</text>\n";
    };
    label(135,"Shape: "+std::string(shape_name(world.shape)));
    label(165,"Resolution: "+std::to_string(world.divisions));
    label(202,"Indexed V: "+std::to_string(r.referenced_vertex_count));
    label(232,"Indexed E: "+std::to_string(r.edge_count));
    label(262,"Faces: "+std::to_string(r.face_count));
    label(292,"Boundary edges: "+std::to_string(r.boundary_edges));
    label(322,"Euler: "+std::to_string(r.euler_characteristic));
    label(356,"Components: "+std::to_string(r.face_edge_components));
    label(389,"Winding errors: "+std::to_string(r.winding_conflict_edges));
    label(420,"Nonmanifold: "+std::to_string(r.nonmanifold_edges));
    label(463,"Topology checked by P07");
    label(485,"Not a CAD certificate");
    out<<"<rect x=\"28\" y=\"581\" width=\"1047\" height=\"65\" "
          "rx=\"10\" fill=\"#e6efea\"/>\n"
       <<"<text x=\"46\" y=\"610\" font-family=\"sans-serif\" "
          "font-size=\"14\" fill=\"#385e67\">"
          "Orange edges are open boundaries; green edges have two "
          "consistently oriented triangle uses.</text>\n"
       <<"<text x=\"46\" y=\"634\" font-family=\"sans-serif\" "
          "font-size=\"12\" fill=\"#657e82\">"
          "The image is a source-derived illustration. "
          "It does not hide unseen faces or measure rendering speed."
          "</text>\n</svg>\n";
    if(!out)throw std::runtime_error("SVG export failed");
}

void obj(std::ostream& out,const WorldState& state) {
    const Mesh3 mesh=state.mesh();
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)
       <<"# Book03/P09 native procedural indexed mesh\n"
       <<"# World units: metres; +Y up\n"
       <<"# Shape: "<<shape_name(state.shape)<<"\n"
       <<"# Only v and f: no normals, UVs, materials or camera\n";
    for(Point3 p:mesh.positions)
        out<<"v "<<p.x<<" "<<p.y<<" "<<p.z<<'\n';
    for(const auto& t:mesh.triangles)
        out<<"f "<<t.vertex[0]+1<<" "<<t.vertex[1]+1
           <<" "<<t.vertex[2]+1<<'\n';
    if(!out)throw std::runtime_error("OBJ export failed");
}
void csv(std::ostream& out,const WorldState& state) {
    const Mesh3 mesh=state.mesh();
    out<<std::setprecision(std::numeric_limits<double>::max_digits10);
    out<<"id,x,y,z\n";
    for(std::size_t i=0;i<mesh.positions.size();++i) {
        const auto& p=mesh.positions[i];
        out<<i<<","<<p.x<<","<<p.y<<","<<p.z<<'\n';
    }
    if(!out)throw std::runtime_error("CSV export failed");
}
template<class Writer>
void export_file(const std::string& path,
                 const WorldState& state,Writer&& writer) {
    // Construct actual geometry BEFORE opening destination: invalid
    // parameters do not truncate a prior useful result.
    const auto mesh=state.mesh();
    mesh.validate();
    std::ofstream output{std::filesystem::path(path),
                         std::ios::binary|std::ios::trunc};
    if(!output)
        throw std::runtime_error("cannot create output file");
    writer(output,state);
    output.flush();
    if(!output)
        throw std::runtime_error("failed to finalize output file");
    std::cout<<"Wrote "<<path<<" from "<<shape_name(state.shape)<<'\n';
}

void repl() {
    WorldState state{};
    std::cout<<"Book03/P09 Procedural World Studio\n"
                "Commands: shape terrain|cylinder|sphere|implicit "
                "| resolution 3..24 | radius metres | amplitude metres "
                "| inspect | svg FILE | obj FILE | csv FILE | reset | quit\n";
    std::string line;
    while(std::cout<<"world> " && std::getline(std::cin,line)) {
        std::istringstream input{line};
        std::string command;input>>command;
        if(command=="exit"||command=="quit")break;
        try {
            if(command=="reset") {
                state=WorldState{};
                std::cout<<"Restored reference terrain.\n";
            } else if(command=="help") {
                std::cout<<"shape NAME, resolution N, radius METRES, "
                           "amplitude METRES, inspect, svg/obj/csv FILE, "
                           "reset, quit\n";
            } else if(command=="inspect") {
                inspect(std::cout,state);
            } else if(command=="shape"||command=="radius"||
                      command=="amplitude"||command=="resolution") {
                std::string value,extra;
                if(!(input>>value)||(input>>extra))
                    throw std::invalid_argument(
                        "supply exactly one argument");
                WorldState next=state;
                if(command=="shape")next.shape=parse_shape(value);
                else if(command=="resolution")next.divisions=resolution(value);
                else if(command=="radius")
                    next.radius=book03::procedural_positive(
                        number(value),"radius must be positive metres");
                else next.amplitude=number(value);
                // Validate NEW design before replacing prior valid scene.
                const Mesh3 checked=next.mesh();
                checked.validate();
                state=next;
                std::cout<<"Updated "<<command<<"="<<value<<'\n';
            } else if(command=="svg"||command=="obj"||command=="csv") {
                std::string output,extra;
                if(!(input>>output)||(input>>extra))
                    throw std::invalid_argument(
                        "give exactly one output path");
                if(command=="svg")export_file(output,state,svg);
                else if(command=="obj")export_file(output,state,obj);
                else export_file(output,state,csv);
            } else if(!command.empty()) {
                std::cout<<"Unknown command; use help.\n";
            }
        } catch(const std::exception& err) {
            std::cout<<"Rejected: "<<err.what()<<'\n';
        }
    }
}
void usage() {
    std::cerr<<"Usage: book03_world inspect [shape] | "
                "svg OUT.svg [shape] | obj OUT.obj [shape] | "
                "csv OUT.csv [shape] | repl\n";
}

} // namespace

int main(int argc,char** argv) {
    if(argc<2){usage();return 2;}
    try {
        const std::string_view command{argv[1]};
        WorldState state{};
        if(command=="repl"&&argc==2) {
            repl();
        } else if(command=="inspect"&&argc>=2&&argc<=3) {
            if(argc==3)state.shape=parse_shape(argv[2]);
            inspect(std::cout,state);
        } else if((command=="svg"||command=="obj"||command=="csv") &&
                  (argc==3||argc==4)) {
            if(argc==4)state.shape=parse_shape(argv[3]);
            if(command=="svg")export_file(argv[2],state,svg);
            else if(command=="obj")export_file(argv[2],state,obj);
            else export_file(argv[2],state,csv);
        } else {
            usage();return 2;
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Book03/P09 error: "<<e.what()<<'\n';
        return 1;
    }
}
