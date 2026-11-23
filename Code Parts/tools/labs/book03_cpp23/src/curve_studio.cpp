// Book 03 / P08 — Cubic Curve and Patch Studio (pure native C++23).
//
// A reader-editable design workbench that uses the SAME geometry
// implementation for all numeric reports, SVG visualization, CSV samples
// and exported triangle mesh. This is an oblique/parallel teaching diagram,
// NOT a camera, CAD NURBS importer, or visually certified geometric bound.
//
// Commands:
//   book03_curves inspect
//   book03_curves svg OUTPUT.svg
//   book03_curves csv OUTPUT.csv
//   book03_curves obj OUTPUT.obj
//   book03_curves repl
//
// In the REPL:
//   control index x y z
//   patch-control u_index v_index x y z
//   tolerance metres
//   inspect | svg FILE | csv FILE | obj FILE | reset | help | quit
//
// The resulting OBJ is an indexed triangular point/face example, no normals,
// UVs, materials, trimming or manifold repair. It is not a publication build.

#include "book03/curves3d.hpp"

#include <array>
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

namespace {

using book03::Point3;
using book03::CubicBezier3;
using book03::BicubicBezierPatch3;
using book03::CurveTessellation;
using book03::Mesh3;

struct Studio {
    CubicBezier3 curve{book03::reference_garden_curve()};
    BicubicBezierPatch3 patch{book03::reference_garden_patch()};
    double tolerance{0.02}; // World-space metres, educational criterion.

    [[nodiscard]] CurveTessellation curve_samples() const {
        return book03::tessellate_curve(curve,tolerance,13);
    }

    [[nodiscard]] Mesh3 mesh() const {
        return book03::patch_to_indexed_mesh(patch,12,12);
    }
};

struct Page { double x{},y{}; };
[[nodiscard]] Page project(Point3 p) {
    if(!book03::finite(p))
        throw std::invalid_argument("cannot project nonfinite point");
    // Explicit parallel oblique drawing, fixed pixels per metre.
    const Page q{325.0+85.0*p.x-26.0*p.z,
                 420.0-85.0*p.y+15.0*p.z};
    if(!std::isfinite(q.x)||!std::isfinite(q.y))
        throw std::overflow_error("diagram projection overflow");
    return q;
}
[[nodiscard]] double numeric(std::string_view str,double limit=1e5) {
    const std::string s{str};
    std::size_t consumed=0;
    const double x=std::stod(s,&consumed);
    if(consumed!=s.size()||!std::isfinite(x)||std::abs(x)>limit)
        throw std::invalid_argument("finite scalar outside teaching range");
    return x;
}
[[nodiscard]] std::size_t control_id(std::string_view s) {
    const double x=numeric(s,3.0);
    if(x!=std::floor(x)||x<0||x>3)
        throw std::invalid_argument("control index must be integer 0..3");
    return static_cast<std::size_t>(x);
}

void describe(std::ostream& out,const Studio& studio) {
    const auto& c=studio.curve;
    const auto sample=studio.curve_samples();
    const auto patch=studio.mesh();
    const auto topology=book03::inspect_mesh(patch);
    const Point3 midpoint=c.at(0.5);
    const auto start_tangent=c.tangent(0.0);
    const auto end_tangent=c.tangent(1.0);
    const auto normal=studio.patch.normal(0.5,0.5);
    out << std::fixed<<std::setprecision(8)
        <<"Cubic Bézier, de Casteljau, t in [0,1]\n"
        <<"World: +Y up, metres; view is PARALLEL oblique\n"
        <<"P(0.5)=("<<midpoint.x<<","<<midpoint.y<<","
        <<midpoint.z<<")\n"
        <<"P'(0)=("<<start_tangent.x<<","<<start_tangent.y
        <<","<<start_tangent.z<<") metres per parameter\n"
        <<"P'(1)=("<<end_tangent.x<<","<<end_tangent.y
        <<","<<end_tangent.z<<") metres per parameter\n"
        <<"Tessellation requested tolerance="<<studio.tolerance
        <<" metres\n"
        <<"Tessellation segments="<<sample.terminal_segments
        <<" samples="<<sample.samples.size()
        <<" terminal-control-bound="<<sample.max_terminal_control_bound
        <<" metres\n"
        <<"Tessellation floating-policy met="
        <<(sample.tolerance_met_under_float_policy?"yes":"NO")
        <<" (NOT a certified Hausdorff proof)\n"
        <<"Patch P(0.5,0.5)=("<<studio.patch.at(0.5,0.5).x<<","
        <<studio.patch.at(0.5,0.5).y<<","
        <<studio.patch.at(0.5,0.5).z<<")\n"
        <<"Patch center unit normal=("<<normal.x<<","<<normal.y
        <<","<<normal.z<<")\n"
        <<"Patch mesh: V="<<topology.referenced_vertex_count
        <<" E="<<topology.edge_count
        <<" F="<<topology.face_count
        <<" boundary="<<topology.boundary_edges
        <<" chi="<<topology.euler_characteristic<<'\n'
        <<"Patch mesh topology: components="<<topology.face_edge_components
        <<" winding-conflicts="<<topology.winding_conflict_edges
        <<" nonmanifold-edges="<<topology.nonmanifold_edges
        <<" vertex-fans="<<topology.disconnected_vertex_fans<<'\n'
        <<"This is an indexed parameter-grid approximation, "
          "not the exact bicubic patch surface.\n";
}

void point_mark(std::ostream& out,Page p,int radius,
                const char* color) {
    out<<"<circle cx=\""<<p.x<<"\" cy=\""<<p.y
       <<"\" r=\""<<radius<<"\" fill=\""<<color
       <<"\" stroke=\"#fbfaf6\" stroke-width=\"1.2\"/>\n";
}

void svg(std::ostream& out,const Studio& s) {
    s.curve.validate();s.patch.validate();
    const auto samples=s.curve_samples();
    const Mesh3 mesh=s.mesh();
    const auto report=book03::inspect_mesh(mesh);
    out<<std::fixed<<std::setprecision(4)
       <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       <<"<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "width=\"1080\" height=\"675\" viewBox=\"0 0 1080 675\" "
         "role=\"img\" aria-labelledby=\"title desc\">\n"
       <<"<title id=\"title\">Cubic Bézier control points, adaptive "
         "polyline and indexed bicubic patch</title>\n"
       <<"<desc id=\"desc\">The original C++23 geometry evaluates the "
         "Bézier curve and the bicubic tensor-product garden patch. "
         "The curve's orange handles connect four control points and "
         "the green sample polyline comes from recursive de Casteljau "
         "subdivision with a declared floating-policy chord-distance "
         "threshold. The patch is sampled into a 12 by 12 rectangular "
         "grid of indexed triangle faces, with shared vertex IDs. "
         "The view is an illustrative parallel oblique map, not a camera. "
         "The tessellation criterion is not a certified geometric error "
         "interval.</desc>\n"
       <<"<rect width=\"1080\" height=\"675\" rx=\"18\" fill=\"#fbfaf6\"/>\n"
       <<"<text x=\"27\" y=\"39\" font-size=\"23\" "
         "font-weight=\"600\" font-family=\"sans-serif\" fill=\"#254953\">"
         "Curves guide the eye. Indexed patches hold the surface.</text>\n"
       <<"<text x=\"27\" y=\"60\" font-size=\"13\" "
         "font-family=\"sans-serif\" fill=\"#687e82\">"
         "P08: true 3D source geometry, drawn through a fixed parallel "
         "oblique map</text>\n"
       <<"<rect x=\"16\" y=\"87\" width=\"756\" height=\"491\" "
         "rx=\"12\" fill=\"#edf3ef\"/>\n";

    // Patch triangles: both the positions and index topology come from P07.
    for(std::size_t i=0;i<mesh.triangles.size();++i) {
        const auto& tri=mesh.triangles[i].vertex;
        const Page a=project(mesh.positions[tri[0]]);
        const Page b=project(mesh.positions[tri[1]]);
        const Page c=project(mesh.positions[tri[2]]);
        out<<"<polygon points=\""<<a.x<<","<<a.y<<" "
           <<b.x<<","<<b.y<<" "<<c.x<<","<<c.y
           <<"\" fill=\""<<(i%2?"#c3dfd1":"#d7eae1")
           <<"\" fill-opacity=\"0.32\" stroke=\"none\"/>\n";
    }
    // Do not duplicate coincident shared edges in the diagram: use the
    // unique index-level edge keys reported by the topology inspector.
    for(const auto& edge:report.edges) {
        const Page a=project(mesh.positions[edge.key.lo]);
        const Page b=project(mesh.positions[edge.key.hi]);
        out<<"<line x1=\""<<a.x<<"\" y1=\""<<a.y
           <<"\" x2=\""<<b.x<<"\" y2=\""<<b.y
           <<"\" stroke=\"#8ab6a5\" stroke-width=\"0.85\"/>\n";
    }

    // The editable control polygon is not generally the curve itself.
    for(int i=0;i<3;++i) {
        const Page a=project(s.curve.p[i]);
        const Page b=project(s.curve.p[i+1]);
        out<<"<line x1=\""<<a.x<<"\" y1=\""<<a.y
           <<"\" x2=\""<<b.x<<"\" y2=\""<<b.y
           <<"\" stroke=\"#be8a62\" stroke-width=\"2.4\" "
             "stroke-dasharray=\"7 5\"/>\n";
    }

    // Adaptive sample positions are already computed by the library.
    out<<"<polyline fill=\"none\" stroke=\"#257c6e\" "
          "stroke-width=\"4\" stroke-linecap=\"round\" points=\"";
    for(const auto& row:samples.samples) {
        const Page p=project(row.p);
        out<<p.x<<","<<p.y<<" ";
    }
    out<<"\"/>\n";
    for(const auto& row:samples.samples)
        point_mark(out,project(row.p),2,"#2b8575");
    for(int i=0;i<4;++i) {
        const Page p=project(s.curve.p[i]);
        point_mark(out,p,5,"#b26e4b");
        out<<"<text x=\""<<p.x+9<<"\" y=\""<<p.y-9
           <<"\" font-family=\"monospace\" font-size=\"13\" "
             "fill=\"#a36047\">P"<<i<<"</text>\n";
    }

    out<<"<rect x=\"793\" y=\"93\" width=\"270\" height=\"462\" "
          "rx=\"12\" fill=\"#e9f2ee\"/>\n"
       <<"<text x=\"808\" y=\"124\" font-size=\"17\" "
         "font-weight=\"600\" font-family=\"sans-serif\" fill=\"#365b65\">"
         "Source, not guesswork</text>\n";
    auto label=[&](int y,const std::string& v) {
        out<<"<text x=\"808\" y=\""<<y<<"\" "
              "font-family=\"monospace\" font-size=\"13\" fill=\"#355c63\">"
           <<v<<"</text>\n";
    };
    label(162,"Cubic controls = 4");
    label(192,"Curve segments = "+std::to_string(
        samples.terminal_segments));
    label(222,"Tolerance = "+std::to_string(s.tolerance)+" m");
    label(252,"Policy met: "+std::string(
        samples.tolerance_met_under_float_policy?"yes":"NO"));
    label(296,"Patch grid = 12 x 12");
    label(326,"Indexed V = "+std::to_string(report.referenced_vertex_count));
    label(356,"Indexed E = "+std::to_string(report.edge_count));
    label(386,"Faces F = "+std::to_string(report.face_count));
    label(416,"Boundary E = "+std::to_string(report.boundary_edges));
    label(446,"Euler = "+std::to_string(report.euler_characteristic));
    out<<"<text x=\"808\" y=\"496\" font-size=\"12\" "
          "font-family=\"sans-serif\" fill=\"#647e81\">"
          "Green = evaluated polyline</text>\n"
       <<"<text x=\"808\" y=\"514\" font-size=\"12\" "
          "font-family=\"sans-serif\" fill=\"#647e81\">"
          "Brown = control polygon</text>\n"
       <<"<rect x=\"25\" y=\"592\" width=\"1037\" height=\"57\" "
          "rx=\"11\" fill=\"#e7f0eb\"/>\n"
       <<"<text x=\"41\" y=\"618\" font-size=\"15\" "
          "font-family=\"sans-serif\" fill=\"#395d66\">"
          "The surface approximation uses shared position IDs, "
          "not duplicated triangles with accidental cracks.</text>\n"
       <<"<text x=\"41\" y=\"638\" font-size=\"11\" "
          "font-family=\"sans-serif\" fill=\"#667e82\">"
          "Curve sampling respects the declared depth budget; "
          "a reported uncertainty is not hidden.</text>\n"
       <<"</svg>\n";
    if(!out)throw std::runtime_error("curve SVG write failed");
}

void csv(std::ostream& out,const Studio& s) {
    const auto samples=s.curve_samples();
    out<<std::setprecision(std::numeric_limits<double>::max_digits10);
    out<<"t,x,y,z\n";
    for(const auto& sample:samples.samples)
        out<<sample.t<<","<<sample.p.x<<","
           <<sample.p.y<<","<<sample.p.z<<'\n';
    if(!out)throw std::runtime_error("curve CSV write failed");
}

void obj(std::ostream& out,const Studio& s) {
    const auto m=s.mesh();
    out<<std::setprecision(std::numeric_limits<double>::max_digits10);
    out<<"# Book03/P08: original C++23 indexed patch example\n"
          "# Units: metres; +Y up; OBJ uses 1-based vertex IDs\n"
          "# Position/face geometry only: NO vt/vn, material, trim or camera\n";
    for(const auto& p:m.positions)
        out<<"v "<<p.x<<" "<<p.y<<" "<<p.z<<"\n";
    for(const auto& f:m.triangles)
        out<<"f "<<f.vertex[0]+1<<" "<<f.vertex[1]+1
           <<" "<<f.vertex[2]+1<<"\n";
    if(!out)throw std::runtime_error("OBJ write failed");
}

template<class Writer>
void save(const std::string& filename,const Studio& studio,
          Writer&& write) {
    std::ofstream out{std::filesystem::path(filename),
                      std::ios::binary|std::ios::trunc};
    if(!out) throw std::runtime_error("unable to open requested output");
    write(out,studio);
    out.flush();
    if(!out) throw std::runtime_error("unable to finalize curve output");
    std::cout<<"Wrote "<<filename<<'\n';
}

void repl() {
    Studio studio{};
    std::cout<<"Book03/P08 Curve and Patch Studio\n"
                "Commands: control ID x y z | patch-control I J x y z "
                "| tolerance metres | inspect | svg FILE | csv FILE "
                "| obj FILE | reset | help | quit\n";
    std::string line;
    while(std::cout<<"curve> " && std::getline(std::cin,line)) {
        std::istringstream input{line};
        std::string cmd;input>>cmd;
        if(cmd=="quit"||cmd=="exit")break;
        try {
            if(cmd=="reset") {
                studio=Studio{};
                std::cout<<"Reference garden reset.\n";
            } else if(cmd=="help") {
                std::cout<<"control ID x y z | patch-control I J x y z "
                           "| tolerance positive-metres | inspect | "
                           "svg FILE | csv FILE | obj FILE | reset | quit\n";
            } else if(cmd=="inspect") {
                describe(std::cout,studio);
            } else if(cmd=="tolerance") {
                std::string value,extra;
                if(!(input>>value)||(input>>extra))
                    throw std::invalid_argument("one positive tolerance");
                const double next=numeric(value,1e5);
                if(!(next>0.0))
                    throw std::invalid_argument("tolerance must be positive");
                studio.tolerance=next;
                std::cout<<"Tolerance="<<next<<" metres\n";
            } else if(cmd=="control"||cmd=="patch-control") {
                const std::size_t count=cmd=="control"?4:5;
                std::array<std::string,5> values{};
                for(std::size_t i=0;i<count;++i)
                    if(!(input>>values[i]))
                        throw std::invalid_argument(
                            "control requires complete indices and XYZ");
                std::string extra;
                if(input>>extra)
                    throw std::invalid_argument("too many control arguments");
                const std::size_t i=control_id(values[0]);
                const std::size_t j=cmd=="control"?0:control_id(values[1]);
                const std::size_t offset=cmd=="control"?1:2;
                const Point3 p{
                    numeric(values[offset]),
                    numeric(values[offset+1]),
                    numeric(values[offset+2])
                };
                if(cmd=="control")studio.curve.p[i]=p;
                else studio.patch.control[i][j]=p;
                std::cout<<"Updated "<<cmd<<" "<<i;
                if(cmd=="patch-control")std::cout<<","<<j;
                std::cout<<": ("<<p.x<<","<<p.y<<","<<p.z<<")\n";
            } else if(cmd=="svg"||cmd=="csv"||cmd=="obj") {
                std::string output,extra;
                if(!(input>>output)||(input>>extra))
                    throw std::invalid_argument("provide one output filename");
                if(cmd=="svg")save(output,studio,svg);
                else if(cmd=="csv")save(output,studio,csv);
                else save(output,studio,obj);
            } else if(!cmd.empty()) {
                std::cout<<"Unknown command; try help.\n";
            }
        } catch(const std::exception& error) {
            std::cout<<"Rejected: "<<error.what()<<'\n';
        }
    }
}
void usage() {
    std::cerr<<"Usage: book03_curves inspect | svg OUT.svg | csv OUT.csv "
                "| obj OUT.obj | repl\n";
}
} // namespace

int main(int argc,char** argv) {
    if(argc<2){usage();return 2;}
    try {
        const std::string_view cmd{argv[1]};
        const Studio s{};
        if(cmd=="inspect"&&argc==2)describe(std::cout,s);
        else if(cmd=="svg"&&argc==3)save(argv[2],s,svg);
        else if(cmd=="csv"&&argc==3)save(argv[2],s,csv);
        else if(cmd=="obj"&&argc==3)save(argv[2],s,obj);
        else if(cmd=="repl"&&argc==2)repl();
        else{usage();return 2;}
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Book03/P08 error: "<<error.what()<<'\n';
        return 1;
    }
}
