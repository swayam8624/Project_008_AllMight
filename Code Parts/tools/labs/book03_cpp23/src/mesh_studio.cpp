// Book 03 / Chapter 07 — native C++23 mesh topology observatory.
//
// Analyze indexed topology, export an edge-incidence CSV, or show a genuine
// 3D mesh wireframe. The SVG is a parallel oblique illustration, not a
// calibrated camera, depth buffer, shaded mesh, or proof of nonintersection.
//
// Commands:
//   book03_mesh inspect [house|quad|flipped|seam|nonmanifold|bowtie|degenerate|collinear]
//   book03_mesh svg OUTPUT.svg [fixture]
//   book03_mesh csv OUTPUT.csv [fixture]
//   book03_mesh repl
//
// In the terminal REPL, fixture NAME changes the selected dataset; outputs
// must reflect that current selection rather than a fixed example.
//
// Meaningful invariants (NOT implied by just a nice-looking picture):
// boundary=1 incidence, consistent=2 opposite, winding conflict=2 same,
// nonmanifold edge >=3, bow-tie vertex disconnected incident face fans.
// Coincident positions are diagnostic data, not authorization to weld.

#include "book03/mesh_topology.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::Mesh3;
using book03::MeshReport;
using book03::Point3;
using book03::EdgeKind;

struct PagePoint { double u{},v{}; };

// Same general oblique teaching choice as P06, slightly enlarged for mesh
// diagnostics. Algebra is explicit and independent of stored camera state.
[[nodiscard]] PagePoint project(Point3 p) {
    if(!book03::finite(p))
        throw std::invalid_argument("nonfinite mesh position");
    const PagePoint q{
        425.0+86.0*p.x-55.0*p.z,
        400.0+22.0*p.x-87.0*p.y+37.0*p.z
    };
    if(!std::isfinite(q.u)||!std::isfinite(q.v))
        throw std::overflow_error("mesh figure projection overflow");
    return q;
}

[[nodiscard]] const char* kind_name(EdgeKind kind) {
    switch(kind) {
        case EdgeKind::Boundary: return "boundary";
        case EdgeKind::ConsistentInterior: return "consistent-interior";
        case EdgeKind::WindingConflict: return "winding-conflict";
        case EdgeKind::NonManifold: return "nonmanifold";
    }
    return "unrecognized";
}

[[nodiscard]] const char* kind_color(EdgeKind kind) {
    switch(kind) {
        case EdgeKind::Boundary: return "#c28350";
        case EdgeKind::ConsistentInterior: return "#238477";
        case EdgeKind::WindingConflict: return "#bd4c53";
        case EdgeKind::NonManifold: return "#8657a5";
    }
    return "#42494a";
}

void inspect(std::ostream& out,const Mesh3& mesh) {
    const MeshReport r=book03::inspect_mesh(mesh);
    out << "Indexed mesh report (identity is vertex index, not XYZ)\n"
        << "positions=" << r.position_count
        << " referenced=" << r.referenced_vertex_count
        << " faces=" << r.face_count
        << " edges=" << r.edge_count << '\n'
        << "Euler V-E+F="<<r.euler_characteristic
        << " (combinatorial count, not a manifold certificate)\n"
        << "boundary="<<r.boundary_edges
        << " consistent-interior="<<r.consistent_interior_edges
        << " winding-conflict="<<r.winding_conflict_edges
        << " nonmanifold="<<r.nonmanifold_edges<<'\n'
        << "face-edge-components="<<r.face_edge_components
        << " disconnected-vertex-fans="<<r.disconnected_vertex_fans<<'\n'
        << "repeated-index-faces="<<r.repeated_index_faces
        << " zero-area-faces="<<r.zero_area_faces
        << " unused-vertices="<<r.unused_vertices
        << " coincident-position-groups="<<r.duplicate_position_groups
        << '\n'
        << "closed-orientable-edge-candidate="
        << (r.closed_orientable_edge_candidate()?"yes":"no")
        << " (necessary checks, NOT sufficient for self-intersection/"
           "global oriented 2-manifold)\n";

    for(const auto& edge:r.edges) {
        if(edge.kind==EdgeKind::ConsistentInterior) continue;
        out << "edge ("<<edge.key.lo<<","<<edge.key.hi<<"): "
            <<kind_name(edge.kind)<<" faces";
        for(const auto& incident:edge.uses)
            out << " " << incident.face << "(" <<incident.from
                <<"->"<<incident.to<<")";
        out <<'\n';
    }
    for(const auto& group:r.coincident_index_groups) {
        out << "same-position different-index IDs:";
        for(auto index:group)out<<" "<<index;
        out<<'\n';
    }
    for(auto index:r.disconnected_fan_vertex_ids)
        out<<"nonmanifold vertex fan ID: "<<index<<'\n';
    for(auto index:r.repeated_face_ids)
        out<<"repeated-index face ID: "<<index<<'\n';
    for(auto index:r.zero_area_face_ids)
        out<<"zero-area face ID: "<<index<<'\n';
}

void svg(std::ostream& out,const Mesh3& mesh,
         std::string_view fixture) {
    const auto r=book03::inspect_mesh(mesh);
    out << std::fixed << std::setprecision(4);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1050\" "
           "height=\"630\" viewBox=\"0 0 1050 630\" "
           "role=\"img\" aria-labelledby=\"title desc\">\n"
        << "<title id=\"title\">Indexed mesh topology inspector</title>\n"
        << "<desc id=\"desc\">The original C++23 indexed mesh's vertices "
           "and edge incidence are drawn with an explicitly parallel "
           "oblique coordinate map. Orange means boundary edge, green "
           "means two oppositely wound faces, red means two same-direction "
           "winding-conflict incidences, purple means three or more "
           "incident faces. Equal coordinates do not imply shared vertex "
           "indices. The diagnostic is not perspective photography, "
           "a depth-buffered render or a proof of global manifoldness."
           "</desc>\n"
        << "<rect width=\"1050\" height=\"630\" rx=\"18\" fill=\"#fbfaf6\"/>\n"
        << "<text x=\"25\" y=\"36\" font-family=\"sans-serif\" "
           "font-size=\"23\" fill=\"#27454e\" font-weight=\"600\">"
           "One picture can hide several different meshes</text>\n"
        << "<text x=\"25\" y=\"59\" font-family=\"sans-serif\" "
           "font-size=\"13\" fill=\"#657c81\">Fixture: "
        << fixture
        << " | edges by vertex ID, not by equal coordinates</text>\n"
        << "<rect x=\"17\" y=\"81\" width=\"765\" height=\"461\" "
           "rx=\"13\" fill=\"#f1f5f2\"/>\n";

    // Draw each indexed triangular face using its original 3D point
    // coordinates. For reversed or duplicated coplanar faces the same
    // outline may overlap: this is PRECISELY why the diagnostics matter.
    for(std::size_t i=0;i<mesh.triangles.size();++i) {
        const auto& f=mesh.triangles[i].vertex;
        const PagePoint a=project(mesh.positions[f[0]]);
        const PagePoint b=project(mesh.positions[f[1]]);
        const PagePoint c=project(mesh.positions[f[2]]);
        out << "<polygon points=\"" << a.u << ","<<a.v<<" "
            <<b.u<<","<<b.v<<" "<<c.u<<","<<c.v
            <<"\" fill=\""<<(i%2?"#cce5db":"#e5dacf")
            <<"\" fill-opacity=\"0.48\" stroke=\"none\"/>\n";
    }
    for(const auto& e:r.edges) {
        const PagePoint a=project(mesh.positions[e.key.lo]);
        const PagePoint b=project(mesh.positions[e.key.hi]);
        out << "<line x1=\""<<a.u<<"\" y1=\""<<a.v
            <<"\" x2=\""<<b.u<<"\" y2=\""<<b.v
            <<"\" stroke=\""<<kind_color(e.kind)
            <<"\" stroke-width=\""<<(e.kind==EdgeKind::NonManifold?6:3)
            <<"\" stroke-linecap=\"round\"/>\n";
    }

    // Diagram only ID labels; coincident positions intentionally overlap,
    // because numeric report exposes different indices at the same place.
    for(std::size_t i=0;i<mesh.positions.size();++i) {
        const PagePoint p=project(mesh.positions[i]);
        out<<"<circle cx=\""<<p.u<<"\" cy=\""<<p.v
           <<"\" r=\"4.8\" fill=\"#294e5b\" stroke=\"white\"/>\n"
           <<"<text x=\""<<p.u+8<<"\" y=\""<<p.v-8
           <<"\" font-size=\"12\" font-family=\"monospace\" "
             "fill=\"#355560\">v"<<i<<"</text>\n";
    }

    out << "<rect x=\"805\" y=\"87\" width=\"229\" height=\"434\" "
           "rx=\"11\" fill=\"#ebf2ef\"/>\n"
        << "<text x=\"820\" y=\"118\" font-family=\"sans-serif\" "
           "font-size=\"17\" font-weight=\"600\" fill=\"#355760\">"
           "Index-level report</text>\n";
    auto line=[&](int y,const std::string& message) {
        out<<"<text x=\"819\" y=\""<<y<<"\" font-family=\"monospace\" "
              "font-size=\"13\" fill=\"#385c65\">"
           <<message<<"</text>\n";
    };
    line(155,"V = "+std::to_string(r.referenced_vertex_count));
    line(177,"E = "+std::to_string(r.edge_count));
    line(199,"F = "+std::to_string(r.face_count));
    line(221,"Euler = "+std::to_string(r.euler_characteristic));
    line(251,"boundaries = "+std::to_string(r.boundary_edges));
    line(273,"good shared = "+std::to_string(r.consistent_interior_edges));
    line(295,"winding bad = "+std::to_string(r.winding_conflict_edges));
    line(317,"nonmanifold = "+std::to_string(r.nonmanifold_edges));
    line(339,"face groups = "+std::to_string(r.face_edge_components));
    line(361,"bad fans = "+std::to_string(r.disconnected_vertex_fans));
    line(383,"coincident = "+std::to_string(r.duplicate_position_groups));
    line(405,"zero area = "+std::to_string(r.zero_area_faces));
    line(427,"repeat IDs = "+std::to_string(r.repeated_index_faces));
    line(462,"closed candidate:");
    line(484,r.closed_orientable_edge_candidate()?"yes (necessary)":"no");

    out << "<rect x=\"25\" y=\"555\" width=\"1007\" height=\"55\" "
           "rx=\"10\" fill=\"#e9f2ee\"/>\n";
    const std::array<std::pair<std::string_view,std::string_view>,4> legend{{
        {"boundary","#c28350"},{"consistent","#238477"},
        {"winding conflict","#bd4c53"},{"nonmanifold","#8657a5"}
    }};
    for(std::size_t i=0;i<legend.size();++i) {
        const int x=45+static_cast<int>(i)*245;
        out<<"<line x1=\""<<x<<"\" y1=\"578\" x2=\""<<x+33
           <<"\" y2=\"578\" stroke=\""<<legend[i].second
           <<"\" stroke-width=\"5\"/>\n"
           <<"<text x=\""<<x+42<<"\" y=\"583\" "
             "font-family=\"sans-serif\" font-size=\"14\" fill=\"#3f6066\">"
           <<legend[i].first<<"</text>\n";
    }
    out << "<text x=\"39\" y=\"603\" font-size=\"10\" "
           "font-family=\"sans-serif\" fill=\"#657e83\">"
           "Every line is drawn from the indexed C++23 mesh. "
           "The view is intentionally not a camera projection.</text>\n"
        << "</svg>\n";
    if(!out) throw std::runtime_error("mesh SVG stream failure");
}

void csv(std::ostream& out,const Mesh3& mesh) {
    const auto report=book03::inspect_mesh(mesh);
    out << "edge_lo,edge_hi,classification,incidence,face_ids\n";
    for(const auto& edge:report.edges) {
        out<<edge.key.lo<<","<<edge.key.hi<<","
           <<kind_name(edge.kind)<<","<<edge.uses.size()<<",";
        for(std::size_t i=0;i<edge.uses.size();++i) {
            if(i)out<<"|";
            out<<edge.uses[i].face;
        }
        out<<'\n';
    }
    if(!out)throw std::runtime_error("mesh CSV stream failure");
}

template<class Write>
void export_file(std::string_view path,const Mesh3& m,
                 std::string_view fixture,Write&& fn) {
    std::ofstream f{std::filesystem::path(std::string(path)),
                    std::ios::binary|std::ios::trunc};
    if(!f)throw std::runtime_error("cannot open mesh output");
    fn(f,m,fixture);
    f.flush();
    if(!f)throw std::runtime_error("cannot finalize mesh output");
    std::cout<<"Wrote "<<path<<'\n';
}

void repl() {
    std::string selected="house";
    std::cout<<"Book03/P07: Indexed Mesh Observatory\n"
                "Commands: fixture NAME | inspect | svg FILE | csv FILE "
                "| help | quit\n"
                "Fixtures: house quad flipped seam nonmanifold bowtie "
                "degenerate collinear\n";
    std::string input;
    while(std::cout<<"mesh> " && std::getline(std::cin,input)) {
        std::istringstream stream{input};
        std::string command; stream>>command;
        if(command=="quit"||command=="exit")break;
        try {
            if(command=="help") {
                std::cout<<"fixture NAME | inspect | svg FILE | csv FILE "
                           "| help | quit\n";
            } else if(command=="fixture") {
                std::string next,extra;
                if(!(stream>>next)||(stream>>extra))
                    throw std::invalid_argument("fixture needs one name");
                (void)book03::fixture_mesh(next);
                selected=next;
                std::cout<<"Selected "<<selected<<'\n';
            } else if(command=="inspect") {
                inspect(std::cout,book03::fixture_mesh(selected));
            } else if(command=="svg"||command=="csv") {
                std::string name,extra;
                if(!(stream>>name)||(stream>>extra))
                    throw std::invalid_argument("provide one output path");
                const auto m=book03::fixture_mesh(selected);
                if(command=="svg") export_file(name,m,selected,svg);
                else export_file(name,m,selected,
                    [](std::ostream& out,const Mesh3& mesh,
                       std::string_view){ csv(out,mesh); });
            } else if(!command.empty()) {
                std::cout<<"Unknown command; type help.\n";
            }
        } catch(const std::exception& error) {
            std::cout<<"Rejected: "<<error.what()<<'\n';
        }
    }
}

void usage() {
    std::cerr<<"Usage:\n"
                " book03_mesh inspect [fixture]\n"
                " book03_mesh svg FILE [fixture]\n"
                " book03_mesh csv FILE [fixture]\n"
                " book03_mesh repl\n";
}

} // namespace

int main(int argc,char** argv) {
    if(argc<2) {usage();return 2;}
    try {
        const std::string_view cmd{argv[1]};
        if(cmd=="repl"&&argc==2) {repl();return 0;}
        if(cmd=="inspect"&&(argc==2||argc==3)) {
            inspect(std::cout,book03::fixture_mesh(
                argc==3?argv[2]:"house"));
            return 0;
        }
        if((cmd=="svg"||cmd=="csv")&&(argc==3||argc==4)) {
            const std::string fixture=argc==4?argv[3]:"house";
            const auto m=book03::fixture_mesh(fixture);
            if(cmd=="svg") export_file(argv[2],m,fixture,svg);
            else export_file(argv[2],m,fixture,
                [](std::ostream& out,const Mesh3& mesh,
                   std::string_view){csv(out,mesh);});
            return 0;
        }
        usage();
        return 2;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P07 error: "<<error.what()<<'\n';
        return 1;
    }
}
