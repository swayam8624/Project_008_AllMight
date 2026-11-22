// Book 03 / P04 — "Move the World Without Losing It".
//
// A native C++23 transform laboratory. All geometry passes through Affine2,
// the earlier P02 ImageMap2 and P03 Triangle2 queries, and the P01 RGB8 Canvas.
// SVG/PPM are *outputs* of the same numeric model, not duplicate algorithms.
//
// CLI:
//    book03_transforms inspect
//    book03_transforms svg OUTPUT.svg
//    book03_transforms ppm OUTPUT.ppm
//    book03_transforms order OUTPUT.svg
//    book03_transforms repl
//
// In the REPL, each operation is PRE-MULTIPLIED in world coordinates:
//   new_transform = new_operation * old_transform
// This means commands are applied in the order the user types them.

#include "book03/affine2.hpp"
#include "book03/pixel_canvas.hpp"
#include "book03/triangle_geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace {

using book03::Affine2;
using book03::Canvas;
using book03::ImageMap2;
using book03::ImagePosition2;
using book03::Point2;
using book03::Rgb8;
using book03::Triangle2;
using book03::ApproxLocation;

constexpr int W = 840;
constexpr int H = 580;
const ImageMap2 screen{{420.5,330.5},50.0,50.0,W,H};

// The house retains immutable MODEL-LOCAL geometry. All movement is represented
// by a single Affine2, just as the chapter says. The units are metres.
const std::array<Point2,4> wall{{
    {-1.5,0.0},{1.5,0.0},{1.5,1.5},{-1.5,1.5}
}};
const std::array<Point2,3> roof{{
    {-1.8,1.5},{0.0,2.8},{1.8,1.5}
}};

double parse(std::string_view text) {
    std::string word{text};
    std::size_t consumed=0;
    const double value=std::stod(word,&consumed);
    if(consumed!=word.size() || !std::isfinite(value)) {
        throw std::invalid_argument("expected finite scalar");
    }
    // Keep the visual laboratory within a humane classroom range.
    if(std::abs(value)>1'000'000.0) {
        throw std::invalid_argument("parameter exceeds classroom range");
    }
    return value;
}

template<std::size_t N>
std::array<Point2,N> transformed(const std::array<Point2,N>& input,
                                 const Affine2& map) {
    std::array<Point2,N> output{};
    for(std::size_t i=0;i<N;++i) output[i]=map.transform_point(input[i]);
    return output;
}

template<std::size_t N>
std::array<ImagePosition2,N> screen_positions(
    const std::array<Point2,N>& points) {
    std::array<ImagePosition2,N> out{};
    for(std::size_t i=0;i<N;++i) {
        out[i]=screen.world_to_image(points[i]);
        if(!screen.pixel_at(out[i]).has_value()) {
            throw std::out_of_range(
                "transformed house vertex lies outside the visible canvas");
        }
    }
    return out;
}

template<std::size_t N>
void polygon_svg(std::ostream& out,
                 const std::array<ImagePosition2,N>& pts,
                 std::string_view fill,std::string_view stroke,
                 bool dashed=false) {
    out << "<polygon points=\"";
    for(const auto p:pts) out << p.u << "," << p.v << " ";
    out << "\" fill=\"" << fill << "\" stroke=\"" << stroke
        << "\" stroke-width=\"3\"";
    if(dashed) out << " stroke-dasharray=\"8 5\"";
    out << "/>\n";
}

void write_svg(std::ostream& out,const Affine2& movement) {
    movement.validate();
    const auto base_wall=screen_positions(wall);
    const auto base_roof=screen_positions(roof);
    const auto moved_wall=screen_positions(transformed(wall,movement));
    const auto moved_roof=screen_positions(transformed(roof,movement));
    const auto origin=screen.world_to_image({0,0});
    const auto current_origin=screen.world_to_image(
        movement.transform_point({0,0}));
    const auto current_x=screen.world_to_image(
        movement.transform_point({1,0}));
    const auto current_y=screen.world_to_image(
        movement.transform_point({0,1}));

    out << std::fixed << std::setprecision(5);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"840\" "
           "height=\"580\" viewBox=\"0 0 840 580\" role=\"img\" "
           "aria-labelledby=\"title desc\">\n"
        << "<title id=\"title\">Affine transformed house studio</title>\n"
        << "<desc id=\"desc\">Gray dashed outlines show immutable local "
           "house geometry in world coordinates. Filled shapes are the same "
           "vertices after the chosen C++23 affine transformation. "
           "Horizontal world X increases right, world Y increases up, "
           "and SVG pixel row increases down. Red and blue axes are "
           "transformed local basis directions.</desc>\n"
        << "<rect width=\"840\" height=\"580\" rx=\"16\" fill=\"#fbfaf6\"/>\n"
        << "<text x=\"28\" y=\"35\" font-family=\"sans-serif\" font-size=\"21\" "
           "fill=\"#274354\" font-weight=\"600\">"
           "The same house in two frames</text>\n"
        << "<text x=\"28\" y=\"55\" font-family=\"sans-serif\" font-size=\"13\" "
           "fill=\"#647984\">Dashes: original. Colors: transformed. "
           "Coordinates in metres.</text>\n";

    for(int x=-7;x<=7;++x) {
        const auto p=screen.world_to_image({static_cast<double>(x),0.0});
        out << "<line x1=\"" << p.u << "\" y1=\"64\" x2=\"" << p.u
            << "\" y2=\"525\" stroke=\"#edf0eb\"/>\n";
    }
    for(int y=-3;y<=5;++y) {
        const auto p=screen.world_to_image({0.0,static_cast<double>(y)});
        out << "<line x1=\"55\" y1=\"" << p.v << "\" x2=\"785\" y2=\""
            << p.v << "\" stroke=\"#edf0eb\"/>\n";
    }
    out << "<line x1=\"40\" y1=\"" << origin.v
        << "\" x2=\"805\" y2=\"" << origin.v
        << "\" stroke=\"#a7b5bd\" stroke-width=\"2\"/>\n";
    out << "<line x1=\"" << origin.u
        << "\" y1=\"70\" x2=\"" << origin.u
        << "\" y2=\"525\" stroke=\"#a7b5bd\" stroke-width=\"2\"/>\n";

    polygon_svg(out,base_wall,"none","#a3afaf",true);
    polygon_svg(out,base_roof,"none","#a3afaf",true);
    polygon_svg(out,moved_wall,"#d8e9d9","#347766");
    polygon_svg(out,moved_roof,"#e8b9a9","#a35d4c");

    out << "<line x1=\"" << current_origin.u << "\" y1=\""
        << current_origin.v << "\" x2=\"" << current_x.u
        << "\" y2=\"" << current_x.v
        << "\" stroke=\"#b56a42\" stroke-width=\"4\"/>\n";
    out << "<line x1=\"" << current_origin.u << "\" y1=\""
        << current_origin.v << "\" x2=\"" << current_y.u
        << "\" y2=\"" << current_y.v
        << "\" stroke=\"#377ea2\" stroke-width=\"4\"/>\n";
    out << "<circle cx=\"" << current_origin.u << "\" cy=\""
        << current_origin.v
        << "\" r=\"6\" fill=\"#3b6573\" stroke=\"white\"/>\n";

    out << "<rect x=\"25\" y=\"532\" width=\"790\" height=\"35\" rx=\"8\" "
           "fill=\"#e9f2ee\"/>\n"
        << "<text x=\"42\" y=\"555\" font-size=\"13\" "
           "font-family=\"monospace\" fill=\"#325760\">"
        << "det(A)=" << movement.determinant()
        << " | origin=(" << movement.tx << "," << movement.ty << ")"
        << " | vectors: no translation; normals: inverse transpose</text>\n"
        << "</svg>\n";
    if(!out) throw std::runtime_error("SVG stream failure");
}

void svg_file(const std::filesystem::path& path,const Affine2& move) {
    std::ofstream file{path,std::ios::binary|std::ios::trunc};
    if(!file) throw std::runtime_error("cannot open SVG");
    write_svg(file,move);
    file.flush();
    if(!file) throw std::runtime_error("cannot finish SVG");
    std::cout << "Saved affine SVG: " << path.string() << '\n';
}

// Only the inside/outside/barycentric interpolation comes from P03. No
// duplicate triangle/raster predicate is hidden in the drawing path.
void fill_triangle(Canvas& pixels,
                   const std::array<ImagePosition2,3>& pts,Rgb8 color) {
    Triangle2 t{{pts[0].u,pts[0].v},
                {pts[1].u,pts[1].v},
                {pts[2].u,pts[2].v}};
    for(int y=0;y<H;++y) for(int x=0;x<W;++x) {
        const auto s=t.sample({
            static_cast<double>(x)+0.5,
            static_cast<double>(y)+0.5});
        if(s.location==ApproxLocation::Interior ||
           s.location==ApproxLocation::BoundaryUncertain) {
            pixels.set({x,y},color);
        }
    }
}

template<std::size_t N>
void stroke(Canvas& pixels,
            const std::array<ImagePosition2,N>& coords,Rgb8 color) {
    for(std::size_t i=0;i<N;++i) {
        const auto& a=coords[i];
        const auto& b=coords[(i+1)%N];
        pixels.draw_line(
            {static_cast<int>(a.u),static_cast<int>(a.v)},
            {static_cast<int>(b.u),static_cast<int>(b.v)},color);
    }
}

void write_ppm(const std::filesystem::path& path,const Affine2& move) {
    Canvas pixels{W,H,{248,249,245}};
    const auto pw=screen_positions(transformed(wall,move));
    const auto pr=screen_positions(transformed(roof,move));
    // Split a quadrilateral into two triangles using its diagonal.
    // Both subtriangles come from the same Affine2 transform.
    fill_triangle(pixels,{pw[0],pw[1],pw[2]},{191,221,196});
    fill_triangle(pixels,{pw[0],pw[2],pw[3]},{191,221,196});
    fill_triangle(pixels,pr,{206,126,105});

    stroke(pixels,pw,{39,105,91});
    stroke(pixels,pr,{141,67,50});

    std::ofstream out{path,std::ios::binary|std::ios::trunc};
    if(!out) throw std::runtime_error("cannot open transformed PPM");
    pixels.write_ppm(out);
    out.flush();
    if(!out) throw std::runtime_error("cannot finish transformed PPM");
    std::cout << "Saved affine image: " << path.string() << '\n';
}

void inspect(const Affine2& movement) {
    movement.validate();
    const auto mat=movement.row_major_3x3();
    std::cout << std::fixed << std::setprecision(9);
    std::cout << "Book03 P04 — column-vector affine matrix:\n";
    for(int row=0;row<3;++row) {
        std::cout << "[";
        for(int col=0;col<3;++col) {
            std::cout << std::setw(14) << mat[row*3+col];
        }
        std::cout << " ]\n";
    }
    std::cout << "determinant=" << movement.determinant() << '\n';
    for(const auto p:std::array<Point2,3>{{
            {0,0},{1,0},{2,1}}}) {
        const auto q=movement.transform_point(p);
        std::cout << "point (" << p.x << "," << p.y << ") -> ("
                  << q.x << "," << q.y << ")\n";
    }
    const auto x=movement.transform_vector({1,0});
    std::cout << "direction (1,0) -> (" << x.x << "," << x.y
              << "), no translation\n";
    try {
        const auto back=movement.inverse().transform_point(
            movement.transform_point({1,0}));
        std::cout << "inverse round-trip (1,0) -> ("
                  << back.x << "," << back.y << ")\n";
    } catch(const std::exception& e) {
        std::cout << "inverse unavailable: " << e.what() << '\n';
    }
}

void order_svg(const std::filesystem::path& path) {
    const auto t=Affine2::translation(2,0);
    const auto r=Affine2::rotation_degrees(90);
    const auto tr=(t*r).transform_point({1,0});
    const auto rt=(r*t).transform_point({1,0});
    std::ofstream out{path,std::ios::binary|std::ios::trunc};
    if(!out) throw std::runtime_error("cannot open order SVG");

    out << std::fixed << std::setprecision(4);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "width=\"760\" height=\"320\" viewBox=\"0 0 760 320\" "
           "role=\"img\" aria-labelledby=\"title desc\">\n"
           "<title id=\"title\">Transform order changes the destination</title>\n"
           "<desc id=\"desc\">The same input point (1,0) in metres gives "
           "(2,1) after rotation by 90 degrees then translation by (2,0), "
           "but (0,3) after translation then rotation. Matrices multiply "
           "right-to-left for column vectors.</desc>\n"
           "<rect width=\"760\" height=\"320\" fill=\"#fbfaf6\"/>\n"
           "<text x=\"26\" y=\"33\" font-size=\"20\" font-family=\"sans-serif\" "
           "fill=\"#244653\">The order is the story</text>\n";

    auto panel=[&](double offset,std::string_view title,Point2 result,
                   std::string_view color) {
        const double cx=offset+148,cy=240;
        const double px=cx+result.x*42,py=cy-result.y*42;
        out << "<rect x=\"" << offset << "\" y=\"54\" width=\"350\" "
               "height=\"240\" rx=\"12\" fill=\"#eef3f0\"/>\n"
               "<text x=\"" << offset+14 << "\" y=\"78\" font-size=\"15\" "
               "font-family=\"sans-serif\" fill=\"#2f5660\">"
            << title << "</text>\n";
        out << "<line x1=\"" << offset+35 << "\" y1=\"" << cy
            << "\" x2=\"" << offset+331 << "\" y2=\"" << cy
            << "\" stroke=\"#b7c8cb\"/>\n";
        out << "<line x1=\"" << cx << "\" y1=\"100\" x2=\"" << cx
            << "\" y2=\"272\" stroke=\"#b7c8cb\"/>\n";
        out << "<circle cx=\"" << cx+42 << "\" cy=\"" << cy
            << "\" r=\"6\" fill=\"#758d98\"/>\n";
        out << "<line x1=\"" << cx+42 << "\" y1=\"" << cy
            << "\" x2=\"" << px << "\" y2=\"" << py
            << "\" stroke=\"" << color
            << "\" stroke-width=\"2\" stroke-dasharray=\"7 5\"/>\n";
        out << "<circle cx=\"" << px << "\" cy=\"" << py
            << "\" r=\"8\" fill=\"" << color
            << "\" stroke=\"white\" stroke-width=\"2\"/>\n";
        out << "<text x=\"" << offset+14 << "\" y=\"286\" "
               "font-size=\"13\" font-family=\"monospace\" fill=\"#37545b\">"
            << "(" << result.x << "," << result.y << ") m</text>\n";
    };
    panel(20,"First rotate, then translate",tr,"#247a6e");
    panel(390,"First translate, then rotate",rt,"#b46d4b");
    out << "</svg>\n";
    out.flush();
    if(!out) throw std::runtime_error("cannot finish order SVG");
    std::cout << "Order test: rotate then translate -> ("
              << tr.x << "," << tr.y << "); translate then rotate -> ("
              << rt.x << "," << rt.y << ")\n";
}

[[nodiscard]] Affine2 default_transform() {
    return Affine2::translation(1,0.4) *
           Affine2::rotation_degrees(25) *
           Affine2::shear_x(0.25);
}

void repl() {
    Affine2 current=Affine2::identity();
    std::cout << "Book03 P04 — affine transform playground\n"
                 "Commands: translate X Y | rotate DEGREES | scale X Y | "
                 "shear K | mirror | pivot X Y DEGREES | point X Y | "
                 "inspect | svg PATH | ppm PATH | reset | help | quit\n";
    std::string line;
    while(std::cout << "transform> " && std::getline(std::cin,line)) {
        std::istringstream in{line};
        std::string cmd;
        in >> cmd;
        if(cmd=="quit" || cmd=="exit") break;
        try {
            if(cmd=="help") {
                std::cout << "translate X Y | rotate DEG | scale X Y | "
                             "shear K | mirror | pivot X Y DEG | point X Y | "
                             "inspect | svg FILE | ppm FILE | reset | quit\n";
            } else if(cmd=="reset") {
                current=Affine2::identity();
                std::cout << "Identity restored.\n";
            } else if(cmd=="mirror") {
                current=Affine2::reflect_about_world_y()*current;
                std::cout << "Mirrored across world Y axis.\n";
            } else if(cmd=="inspect") {
                inspect(current);
            } else if(cmd=="svg" || cmd=="ppm") {
                std::string name,extra;
                if(!(in >> name) || (in >> extra))
                    throw std::invalid_argument("supply one output file path");
                if(cmd=="svg") svg_file(name,current);
                else write_ppm(name,current);
            } else if(cmd=="translate" || cmd=="scale" || cmd=="point") {
                std::string x,y,extra;
                if(!(in >> x >> y) || (in >> extra))
                    throw std::invalid_argument("supply exactly two numbers");
                const double a=parse(x),b=parse(y);
                if(cmd=="translate") {
                    current=Affine2::translation(a,b)*current;
                    std::cout << "World translation applied.\n";
                } else if(cmd=="scale") {
                    current=Affine2::scale(a,b)*current;
                    std::cout << "World-axis scale applied.\n";
                } else {
                    const auto q=current.transform_point({a,b});
                    std::cout << std::fixed << std::setprecision(8)
                              << "Local (" << a << "," << b
                              << ") -> world (" << q.x << "," << q.y
                              << ")\n";
                }
            } else if(cmd=="rotate" || cmd=="shear") {
                std::string value,extra;
                if(!(in >> value) || (in >> extra))
                    throw std::invalid_argument("supply one scalar");
                const double a=parse(value);
                current=(cmd=="rotate" ? Affine2::rotation_degrees(a) :
                                     Affine2::shear_x(a))*current;
                std::cout << "World-axis operation applied.\n";
            } else if(cmd=="pivot") {
                std::string x,y,ang,extra;
                if(!(in >> x >> y >> ang) || (in >> extra))
                    throw std::invalid_argument("supply pivot x y degrees");
                const Point2 pivot{parse(x),parse(y)};
                const double degrees=parse(ang);
                current=Affine2::around_pivot(
                    Affine2::rotation_degrees(degrees),pivot)*current;
                std::cout << "World-space pivot rotation applied.\n";
            } else if(!cmd.empty()) {
                std::cout << "Unknown command; enter help.\n";
            }
        } catch(const std::exception& e) {
            std::cout << "Input rejected: " << e.what() << '\n';
        }
    }
}

void usage() {
    std::cerr << "Usage:\n"
                 " book03_transforms inspect\n"
                 " book03_transforms svg OUTPUT.svg\n"
                 " book03_transforms ppm OUTPUT.ppm\n"
                 " book03_transforms order OUTPUT.svg\n"
                 " book03_transforms repl\n";
}

} // namespace

int main(int argc,char** argv) {
    if(argc<2) { usage(); return 2; }
    try {
        const std::string_view mode{argv[1]};
        if(mode=="inspect" && argc==2) {
            inspect(default_transform());
        } else if(mode=="svg" && argc==3) {
            svg_file(argv[2],default_transform());
        } else if(mode=="ppm" && argc==3) {
            write_ppm(argv[2],default_transform());
        } else if(mode=="order" && argc==3) {
            order_svg(argv[2]);
        } else if(mode=="repl" && argc==2) {
            repl();
        } else {
            usage();
            return 2;
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03 P04 error: " << e.what() << '\n';
        return 1;
    }
}
