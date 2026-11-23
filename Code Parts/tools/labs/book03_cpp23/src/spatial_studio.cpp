// Book 03 / P06 — "Step Out of the Page" spatial workbench.
//
// The only geometry library is geometry3d.hpp. Rendering is an explicitly
// chosen OBLIQUE (parallel, affine) diagram mapping, not a perspective camera.
// Its XY pixel positions cannot be interpreted as calibrated projections.
// The same 3D points drive the numeric result, SVG and CSV output.
//
// Commands:
//   book03_space inspect
//   book03_space svg OUT.svg
//   book03_space csv OUT.csv
//   book03_space repl
//
// In the REPL, set an arbitrary (finite, nonzero) ray with
//   ray ox oy oz dx dy dz
// then inspect the actual ground/sphere results, and save SVG or CSV.
//
// No proprietary runtime, Python renderer, GPU, Vulkan or Metal required.

#include "book03/geometry3d.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

using book03::Point3;
using book03::Vector3;
using book03::Ray3;
using book03::Sphere3;
using book03::Plane3;
using book03::Triangle3;
using book03::RayPlaneResult;
using book03::RaySphereResult;
using book03::RayPlaneKind;
using book03::RaySphereKind;

struct PagePoint {
    double u{},v{}; // Continuous SVG pixel coordinates; v increases down.
};

// Explicit parallel projection, 52/30/20/55/18 pixels per world metre.
// This is a deliberately oblique diagram, not the camera pipeline.
[[nodiscard]] PagePoint project(Point3 p) {
    if(!book03::finite(p))
        throw std::invalid_argument("nonfinite scene point");
    const PagePoint out{
        430.0+52.0*p.x-30.0*p.z,
        370.0+20.0*p.x-55.0*p.y+18.0*p.z
    };
    if(!std::isfinite(out.u)||!std::isfinite(out.v))
        throw std::overflow_error("diagram mapping overflow");
    return out;
}

[[nodiscard]] Plane3 floor_plane() {
    return Plane3::through({0,0,0},{0,1,0});
}
[[nodiscard]] Sphere3 garden_sphere() {
    return Sphere3::from({2,1,0},1);
}
[[nodiscard]] Ray3 reference_ray() {
    return Ray3::from({0,4,0},{0,-1,0});
}

[[nodiscard]] const char* label(RayPlaneKind kind) {
    switch(kind) {
      case RayPlaneKind::Hit: return "forward-hit";
      case RayPlaneKind::Behind: return "behind-ray";
      case RayPlaneKind::Parallel: return "parallel";
      case RayPlaneKind::CoplanarWithinPolicy: return "coplanar-within-policy";
      case RayPlaneKind::NearParallelUncertain: return "near-parallel-uncertain";
    }
    return "unrecognized-plane-status";
}
[[nodiscard]] const char* label(RaySphereKind kind) {
    switch(kind) {
      case RaySphereKind::Hit: return "forward-hit";
      case RaySphereKind::Miss: return "miss";
      case RaySphereKind::Behind: return "behind-ray";
      case RaySphereKind::TangentWithinPolicy: return "tangent-within-policy";
      case RaySphereKind::NearTangentUncertain: return "near-tangent-uncertain";
    }
    return "unrecognized-sphere-status";
}

[[nodiscard]] double parse(std::string_view text) {
    const std::string input{text};
    std::size_t position=0;
    const double value=std::stod(input,&position);
    if(position!=input.size()||!std::isfinite(value))
        throw std::invalid_argument("expect finite scalar");
    if(std::abs(value)>1'000'000.0)
        throw std::invalid_argument("visual teaching range is +/-1e6");
    return value;
}

void report(std::ostream& out,const Ray3& ray) {
    const auto floor=book03::intersect(ray,floor_plane());
    const auto sphere=book03::intersect(ray,garden_sphere());

    out << std::fixed << std::setprecision(9);
    out << "World: right-handed, +X right, +Y up, +Z toward viewer\n"
        << "Ray origin (metres) = (" << ray.origin.x << ","
        << ray.origin.y << "," << ray.origin.z << ")\n"
        << "Unit direction = (" << ray.direction.x << ","
        << ray.direction.y << "," << ray.direction.z << ")\n"
        << "Ground Y=0: " << label(floor.kind) << '\n';
    if(floor.point) {
        out << "  t = " << floor.metres << " metres\n"
            << "  hit = (" << floor.point->x << ","
            << floor.point->y << "," << floor.point->z << ")\n"
            << "  independent plane residual = "
            << floor_plane().signed_distance(*floor.point) << " metres\n";
    }
    out << "Ball center=(2,1,0), r=1 m: " << label(sphere.kind) << '\n';
    if(sphere.point) {
        out << "  first forward t = "<< sphere.metres << " metres\n"
            << "  hit = (" << sphere.point->x << ","
            << sphere.point->y << "," << sphere.point->z << ")\n"
            << "  other LINE root = " << sphere.other_line_metres
            << " metres (may be behind ray)\n";
    }

    const Triangle3 patch{
        {-1.2,2.0,1.2}, {1.2,2.0,1.2}, {0,3.0,1.2}
    };
    const Vector3 normal=patch.face_normal();
    out << "Front roof-panel normal = (" << normal.x << ","
        << normal.y << "," << normal.z << ")\n"
        << "Front panel triangle area = "<<patch.area()<<" m^2\n"
        << "Note: status guards are conservative floating policies, "
           "not certified geometric predicates.\n";
}

void svg_line(std::ostream& out,PagePoint a,PagePoint b,
              std::string_view color,int width=2,
              bool dash=false) {
    out << "<line x1=\"" << a.u << "\" y1=\"" << a.v
        << "\" x2=\"" << b.u << "\" y2=\"" << b.v
        << "\" stroke=\"" << color << "\" stroke-width=\""
        << width << "\" stroke-linecap=\"round\"";
    if(dash) out << " stroke-dasharray=\"7 6\"";
    out << "/>\n";
}
void world_line(std::ostream& out,Point3 a,Point3 b,
                std::string_view color,int width=2,bool dash=false) {
    svg_line(out,project(a),project(b),color,width,dash);
}

void circle(std::ostream& out,PagePoint p,double radius,
            std::string_view color) {
    out << "<circle cx=\"" << p.u << "\" cy=\"" << p.v
        << "\" r=\"" << radius << "\" fill=\"" << color
        << "\" stroke=\"#fff\" stroke-width=\"1.5\"/>\n";
}
void marker(std::ostream& out,Point3 p,std::string_view label,
            std::string_view color) {
    const auto q=project(p);
    circle(out,q,5,color);
    out << "<text x=\"" << q.u+9 << "\" y=\"" << q.v-8
        << "\" fill=\"" << color
        << "\" font-size=\"13\" font-family=\"sans-serif\">"
        << label << "</text>\n";
}

void draw_house(std::ostream& out) {
    // Every edge is defined in WORLD 3D metres, then transformed by the
    // single diagram map above. No hand-placed fake 2D roof coordinates.
    constexpr std::array<Point3,10> p{{
        {-1.2,0,-1.2}, {1.2,0,-1.2}, {1.2,0,1.2}, {-1.2,0,1.2},
        {-1.2,2,-1.2}, {1.2,2,-1.2}, {1.2,2,1.2}, {-1.2,2,1.2},
        {0,3,-1.2}, {0,3,1.2}
    }};
    constexpr std::array<std::pair<std::size_t,std::size_t>,17> edges{{
        {0,1},{1,2},{2,3},{3,0},         // Floor perimeter
        {4,5},{5,6},{6,7},{7,4},         // Wall-top perimeter
        {0,4},{1,5},{2,6},{3,7},         // Wall uprights
        {4,8},{5,8},{7,9},{6,9},{8,9}  // Ridge roof
    }};
    for(auto [a,b]:edges)
        world_line(out,p[a],p[b],
                   (a<4&&b<4)?"#89a39d":"#287c71",2);

    // A NORMAL computed from ordered triangle edges (not guessed from the
    // diagram). This is the same Triangle3 API exercised by the CTest.
    const Triangle3 front_patch{p[7],p[6],p[9]};
    const Vector3 normal=front_patch.face_normal();
    const Point3 centroid{
        (p[7].x+p[6].x+p[9].x)/3,
        (p[7].y+p[6].y+p[9].y)/3,
        (p[7].z+p[6].z+p[9].z)/3
    };
    world_line(out,centroid,centroid+1.15*normal,"#ba6d47",4);
    marker(out,centroid+1.15*normal,"normal","#a95d3e");
    marker(out,p[9],"roof ridge","#267669");
}

void draw_sphere_wire(std::ostream& out,const Sphere3& sphere) {
    constexpr int steps=72;
    for(int ring=0;ring<3;++ring) {
        PagePoint previous{};
        for(int i=0;i<=steps;++i) {
            const double t=2*std::numbers::pi*
                static_cast<double>(i)/static_cast<double>(steps);
            Point3 p{};
            if(ring==0) {
                // XZ equator, Y fixed.
                p=sphere.center+sphere.radius*Vector3{
                    std::cos(t),0,std::sin(t)};
            } else if(ring==1) {
                // XY meridian, Z fixed.
                p=sphere.center+sphere.radius*Vector3{
                    std::cos(t),std::sin(t),0};
            } else {
                // YZ meridian, X fixed.
                p=sphere.center+sphere.radius*Vector3{
                    0,std::cos(t),std::sin(t)};
            }
            const PagePoint q=project(p);
            if(i>0) svg_line(out,previous,q,"#ab7959",1);
            previous=q;
        }
    }
    marker(out,sphere.center,"sphere center","#a36643");
}

void write_svg(std::ostream& out,const Ray3& ray) {
    ray.validate();
    const auto ground=book03::intersect(ray,floor_plane());
    const auto ball=book03::intersect(ray,garden_sphere());

    out << std::fixed << std::setprecision(4);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "width=\"920\" height=\"620\" viewBox=\"0 0 920 620\" "
           "role=\"img\" aria-labelledby=\"title desc\">\n"
           "<title id=\"title\">A house, a ball, a plane and a world ray</title>\n"
           "<desc id=\"desc\">All positions originate from 3D C++23 "
           "world coordinates with +Y up. An explicitly oblique, parallel "
           "map is used for this page illustration, not a perspective "
           "camera. The same source geometry produces the house wireframe, "
           "surface normal, sphere rings and ray-plane intersection mark. "
           "The result statuses are numerical teaching policies, not "
           "certified exact 3D predicates.</desc>\n"
           "<rect width=\"920\" height=\"620\" rx=\"18\" fill=\"#fbfaf6\"/>\n"
           "<text x=\"26\" y=\"35\" font-size=\"21\" font-weight=\"600\" "
           "font-family=\"sans-serif\" fill=\"#234250\">"
           "The house steps into the third dimension</text>\n"
           "<text x=\"26\" y=\"56\" font-size=\"13\" font-family=\"sans-serif\" "
           "fill=\"#647b82\">One world coordinate model; oblique "
           "parallel drawing, not a camera photograph</text>\n";

    for(int x=-4;x<=4;++x)
        world_line(out,
            {static_cast<double>(x),0,-4},
            {static_cast<double>(x),0,4},"#e0e7e2",1);
    for(int z=-4;z<=4;++z)
        world_line(out,
            {-4,0,static_cast<double>(z)},
            {4,0,static_cast<double>(z)},"#e0e7e2",1);

    // Labelled axes from the SAME explicit coordinate mapping.
    world_line(out,{0,0,0},{2.5,0,0},"#c0714d",3);
    world_line(out,{0,0,0},{0,2.5,0},"#347b9c",3);
    world_line(out,{0,0,0},{0,0,2.5},"#479273",3);
    marker(out,{2.5,0,0},"+X","#c0714d");
    marker(out,{0,2.5,0},"+Y","#347b9c");
    marker(out,{0,0,2.5},"+Z","#479273");

    draw_house(out);
    draw_sphere_wire(out,garden_sphere());

    // Draw the specified unit-direction ray, independently of whether its
    // infinite line meets the plane. No reported false hit is created.
    const Point3 tip=ray.at(3.0);
    world_line(out,ray.origin,tip,"#a75465",3,true);
    marker(out,ray.origin,"ray origin","#a75465");
    if(ground.point) marker(out,*ground.point,"floor hit","#ad603d");
    if(ball.point) marker(out,*ball.point,"sphere hit","#ae603d");

    out << "<rect x=\"22\" y=\"549\" width=\"875\" height=\"51\" "
           "rx=\"10\" fill=\"#eaf2ef\"/>\n"
           "<text x=\"38\" y=\"573\" font-size=\"14\" "
           "font-family=\"monospace\" fill=\"#345b63\">"
        << "ground="<<label(ground.kind)
        << (ground.point?" t="+std::to_string(ground.metres)+" m":"")
        << " ; sphere="<<label(ball.kind)
        << "</text>\n"
        << "<text x=\"38\" y=\"591\" font-size=\"11\" "
           "font-family=\"sans-serif\" fill=\"#597679\">"
           "Colored vectors, edges and intersection marks are generated "
           "by the same numeric C++23 geometry.</text>\n"
        << "</svg>\n";
    if(!out) throw std::runtime_error("SVG stream failed");
}

void write_csv(std::ostream& out,const Ray3& ray) {
    const auto a=book03::intersect(ray,floor_plane());
    const auto b=book03::intersect(ray,garden_sphere());
    out << std::setprecision(std::numeric_limits<double>::max_digits10)
        << "primitive,status,has_hit,metres,x,y,z\n";

    auto row=[&](const char* name,const char* kind,
                 const std::optional<Point3>& p,double t) {
        out << name<<","<<kind<<","<<(p?1:0)<<",";
        if(p) out<<t<<","<<p->x<<","<<p->y<<","<<p->z;
        else out<<",,,";
        out << '\n';
    };
    row("ground",label(a.kind),a.point,a.metres);
    row("sphere",label(b.kind),b.point,b.metres);
    if(!out) throw std::runtime_error("CSV write failed");
}

template<class Writer>
void write_output(const std::filesystem::path& path,
                  const Ray3& ray,Writer&& fn) {
    std::ofstream out{path,std::ios::binary|std::ios::trunc};
    if(!out) throw std::runtime_error("cannot open destination file");
    fn(out,ray);
    out.flush();
    if(!out) throw std::runtime_error("output stream did not flush");
    std::cout << "Wrote "<<path.string()<<'\n';
}

void repl() {
    Ray3 ray=reference_ray();
    std::cout << "Book 03 P06 — Spatial Geometry Studio\n"
              << "Commands: ray ox oy oz dx dy dz | inspect | "
                 "svg FILE | csv FILE | reset | help | quit\n";
    std::string line;
    while(std::cout<<"space> "&&std::getline(std::cin,line)) {
        std::istringstream input{line};
        std::string command;input>>command;
        if(command=="quit"||command=="exit") break;
        try {
            if(command=="help") {
                std::cout << "ray ox oy oz dx dy dz | inspect | "
                             "svg FILE | csv FILE | reset | quit\n";
            } else if(command=="reset") {
                ray=reference_ray();
                std::cout << "Reference downward ray restored.\n";
            } else if(command=="inspect") {
                report(std::cout,ray);
            } else if(command=="svg"||command=="csv") {
                std::string name,extra;
                if(!(input>>name)||(input>>extra))
                    throw std::invalid_argument("provide one filename");
                if(command=="svg") write_output(name,ray,write_svg);
                else write_output(name,ray,write_csv);
            } else if(command=="ray") {
                std::array<std::string,6> arguments;
                for(auto& word:arguments) if(!(input>>word))
                    throw std::invalid_argument(
                        "ray needs origin XYZ then direction XYZ");
                std::string extra;
                if(input>>extra)
                    throw std::invalid_argument(
                        "ray requires exactly six numbers");
                ray=Ray3::from(
                    {parse(arguments[0]),parse(arguments[1]),
                     parse(arguments[2])},
                    {parse(arguments[3]),parse(arguments[4]),
                     parse(arguments[5])});
                report(std::cout,ray);
            } else if(!command.empty()) {
                std::cout<<"Unknown command; try help.\n";
            }
        } catch(const std::exception& e) {
            std::cout<<"Input rejected: "<<e.what()<<'\n';
        }
    }
}

void usage() {
    std::cerr << "Usage:\n"
                 " book03_space inspect\n"
                 " book03_space svg OUTPUT.svg\n"
                 " book03_space csv OUTPUT.csv\n"
                 " book03_space repl\n";
}

} // namespace

int main(int argc,char** argv) {
    if(argc<2) { usage();return 2; }
    try {
        const std::string_view action{argv[1]};
        if(action=="inspect"&&argc==2) report(std::cout,reference_ray());
        else if(action=="svg"&&argc==3)
            write_output(argv[2],reference_ray(),write_svg);
        else if(action=="csv"&&argc==3)
            write_output(argv[2],reference_ray(),write_csv);
        else if(action=="repl"&&argc==2) repl();
        else { usage();return 2; }
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Book03/P06 error: "<<e.what()<<'\n';
        return 1;
    }
}
