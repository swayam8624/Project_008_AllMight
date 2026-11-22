// From Geometry to Images / Chapter 03: Shapes That Know Their Borders.
//
// A numerical triangle inspection mode, a PPM barycentric color plate and a
// native SVG geometric diagram. All numeric classification is delegated to
// triangle_geometry.hpp, not reimplemented in this visualization.
// Uses Chapter 01 Canvas and Chapter 02 Point2 rather than duplicating them.

#include "book03/pixel_canvas.hpp"
#include "book03/triangle_geometry.hpp"

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
#include <utility>

namespace {

using book03::ApproxLocation;
using book03::BarycentricWeights;
using book03::Canvas;
using book03::Point2;
using book03::Rgb8;
using book03::Triangle2;

constexpr int width = 640;
constexpr int height = 420;

[[nodiscard]] const char* to_string(ApproxLocation location) {
    switch (location) {
    case ApproxLocation::Interior: return "interior";
    case ApproxLocation::Exterior: return "exterior";
    case ApproxLocation::BoundaryUncertain: return "boundary-uncertain";
    case ApproxLocation::DegenerateOrUncertified:
        return "degenerate-or-uncertified";
    }
    return "unknown";
}

[[nodiscard]] Triangle2 reference_world_triangle() {
    return {{0.0, 0.0}, {6.0, 0.0}, {0.0, 4.0}};
}

// This second triangle explicitly uses IMAGE coordinates: +v is downward.
// Thus its negative determinant is not a mistake or a world-facing normal.
[[nodiscard]] Triangle2 image_triangle() {
    return {{70.0, 350.0}, {570.0, 350.0}, {320.0, 60.0}};
}

void numerical_inspection(const Triangle2& t, Point2 p) {
    const auto s = t.sample(p);
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Triangle vertices A=(" << t.a.x << "," << t.a.y
              << "), B=(" << t.b.x << "," << t.b.y
              << "), C=(" << t.c.x << "," << t.c.y << ")\n";
    std::cout << "Point P=(" << p.x << "," << p.y << ")\n";
    std::cout << "Signed twice-area = " << s.twice_signed_area << '\n';
    std::cout << "Winding in the NUMERIC x-right y-up convention: "
              << (s.twice_signed_area > 0 ? "counterclockwise" :
                  s.twice_signed_area < 0 ? "clockwise" : "degenerate")
              << '\n';
    if (s.location != ApproxLocation::DegenerateOrUncertified) {
        std::cout << "Weights (A,B,C) = (" << s.barycentric.at_a << ","
                  << s.barycentric.at_b << ","
                  << s.barycentric.at_c << ")\n"
                  << "Weight sum = " << s.barycentric.sum() << '\n';
        std::cout << "Scalar T(A=10,B=20,C=30) = "
                  << Triangle2::interpolate(s.barycentric, {10,20,30})
                  << '\n';
    }
    std::cout << "Approximate classification = "
              << to_string(s.location) << '\n';
    std::cout << "Caution: floating classification is NOT an exact "
                 "predicate on arbitrary inputs.\n";
}

// Encoded demonstration RGB: this interpolation is a *visualization* of
// barycentric weights, not a physical linear-radiance or color-managed claim.
[[nodiscard]] Rgb8 blended_illustration(BarycentricWeights w) {
    const auto channel = [&](std::array<double, 3> values) {
        const double mixed = Triangle2::interpolate(w, values);
        return static_cast<std::uint8_t>(
            std::clamp(std::lround(std::clamp(mixed, 0.0, 255.0)),
                       0L, 255L));
    };
    return {channel({230, 52, 54}),
            channel({66, 208, 72}),
            channel({61, 83, 230})};
}

void write_ppm(const std::filesystem::path& target) {
    const Triangle2 triangle = image_triangle();
    Canvas pixels{width, height, {248, 248, 245}};
    constexpr Rgb8 edge_color{244, 168, 57};
    constexpr Rgb8 uncertain_color{184, 83, 79};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Point2 sample_pos{static_cast<double>(x) + 0.5,
                                    static_cast<double>(y) + 0.5};
            const auto result = triangle.sample(sample_pos);
            if (result.location == ApproxLocation::Interior) {
                pixels.set({x,y}, blended_illustration(result.barycentric));
            } else if (result.location == ApproxLocation::BoundaryUncertain) {
                pixels.set({x,y}, uncertain_color);
            }
        }
    }

    // The line overlay makes the three actual stored vertex positions
    // inspectable. The *overlay* is NOT used as the containment oracle.
    const auto endpoint = [](Point2 p) {
        return book03::Pixel{static_cast<int>(p.x),
                             static_cast<int>(p.y)};
    };
    pixels.draw_line(endpoint(triangle.a), endpoint(triangle.b), edge_color);
    pixels.draw_line(endpoint(triangle.b), endpoint(triangle.c), edge_color);
    pixels.draw_line(endpoint(triangle.c), endpoint(triangle.a), edge_color);

    std::ofstream file{target, std::ios::binary | std::ios::trunc};
    if (!file) throw std::runtime_error("unable to create PPM file");
    pixels.write_ppm(file);
    file.flush();
    if (!file) throw std::runtime_error("unable to finish PPM file");
    std::cout << "Wrote barycentric demonstration " << target.string()
              << " (" << width << " x " << height << ", RGB8 P6)\n";
}

void write_svg(const std::filesystem::path& target) {
    const Triangle2 triangle = image_triangle();
    const Point2 p{320.0, 230.0};
    const auto sample = triangle.sample(p);
    std::ofstream out{target, std::ios::binary | std::ios::trunc};
    if (!out) throw std::runtime_error("cannot open SVG output");

    out << std::fixed << std::setprecision(6);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"760\" "
           "height=\"540\" viewBox=\"0 0 760 540\" role=\"img\" "
           "aria-labelledby=\"title desc\">\n"
           "<title id=\"title\">Barycentric triangle inspector</title>\n"
           "<desc id=\"desc\">An image-coordinate triangle with corners "
           "(70,350), (570,350), and (320,60), in pixels with v downward. "
           "A selected query is drawn with computed barycentric weights. "
           "This vector illustration is derived from the C++23 numeric "
           "triangle model.</desc>\n"
           "<rect width=\"760\" height=\"540\" fill=\"#fbfaf6\"/>\n"
           "<text x=\"28\" y=\"37\" font-family=\"sans-serif\" "
           "font-size=\"21\" fill=\"#234453\">Triangle laboratory</text>\n"
           "<text x=\"28\" y=\"58\" font-family=\"sans-serif\" "
           "font-size=\"13\" fill=\"#63777c\">Image coordinates: +u right, "
           "+v down. Geometric winding sign follows this basis.</text>\n";

    out << "<polygon points=\""
        << triangle.a.x << "," << triangle.a.y << " "
        << triangle.b.x << "," << triangle.b.y << " "
        << triangle.c.x << "," << triangle.c.y
        << "\" fill=\"#c8e7dd\" stroke=\"#267a6f\" stroke-width=\"3\"/>\n";

    for (const auto& [label, point] : std::array<std::pair<const char*,Point2>,3>{{
            {"A",triangle.a},{"B",triangle.b},{"C",triangle.c}}}) {
        out << "<circle cx=\"" << point.x << "\" cy=\"" << point.y
            << "\" r=\"7\" fill=\"#1e7168\" stroke=\"#fff\" "
            << "stroke-width=\"2\"/>\n";
        out << "<text x=\"" << point.x + 12.0 << "\" y=\""
            << point.y - 9.0 << "\" font-family=\"sans-serif\" "
            << "font-size=\"16\" fill=\"#24424c\">" << label << "</text>\n";
    }

    out << "<circle cx=\"" << p.x << "\" cy=\"" << p.y
        << "\" r=\"7\" fill=\"#b35f40\" stroke=\"white\" "
        << "stroke-width=\"2\"/>\n";
    out << "<text x=\"" << p.x + 13.0 << "\" y=\""
        << p.y - 9.0 << "\" font-family=\"sans-serif\" "
        << "font-size=\"15\" fill=\"#9f5e39\">P</text>\n";
    out << "<rect x=\"26\" y=\"439\" width=\"704\" height=\"75\" rx=\"10\" "
           "fill=\"#eaf2f0\"/>\n";
    out << "<text x=\"40\" y=\"463\" font-family=\"monospace\" "
        << "font-size=\"14\" fill=\"#315760\">Twice signed area = "
        << sample.twice_signed_area << "</text>\n";
    out << "<text x=\"40\" y=\"486\" font-family=\"monospace\" "
        << "font-size=\"14\" fill=\"#315760\">Weights (A,B,C) = ("
        << sample.barycentric.at_a << ", "
        << sample.barycentric.at_b << ", "
        << sample.barycentric.at_c << ")</text>\n";
    out << "<text x=\"40\" y=\"505\" font-family=\"sans-serif\" "
        << "font-size=\"11\" fill=\"#60747b\">Approximate location: "
        << to_string(sample.location)
        << ". Source code is the numerical contract, not the SVG.</text>\n";
    out << "</svg>\n";
    out.flush();
    if (!out) throw std::runtime_error("unable to finish SVG");
    std::cout << "Wrote triangle SVG " << target.string() << '\n';
}

// Interactive export: unlike the fixed reference plate, this uses the
// CURRENT triangle and most recently inspected point from the REPL.
// A single 2D world-to-image map owns the y-up -> v-down conversion.
void save_current_svg(const std::filesystem::path& target,
                      const Triangle2& triangle, Point2 queried) {
    const auto within_display = [](Point2 p) {
        return book03::finite(p) &&
               std::abs(p.x) <= 10.0 && std::abs(p.y) <= 10.0;
    };
    if (!within_display(triangle.a) ||
        !within_display(triangle.b) ||
        !within_display(triangle.c) ||
        !within_display(queried)) {
        throw std::invalid_argument(
            "interactive SVG preview requires coordinates within [-10,10] m");
    }

    // Continuous pixel positions, not integer storage indices.
    const book03::ImageMap2 map{{380.5, 270.5}, 25.0, 25.0, 760, 540};
    const auto a = map.world_to_image(triangle.a);
    const auto b = map.world_to_image(triangle.b);
    const auto c = map.world_to_image(triangle.c);
    const auto p = map.world_to_image(queried);
    const auto sample = triangle.sample(queried);

    std::ofstream out{target, std::ios::binary | std::ios::trunc};
    if (!out) throw std::runtime_error("cannot create interactive SVG");
    out << std::fixed << std::setprecision(5);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"760\" "
           "height=\"540\" viewBox=\"0 0 760 540\" role=\"img\" "
           "aria-labelledby=\"title desc\">\n"
           "<title id=\"title\">Interactive triangle at current coordinates</title>\n"
           "<desc id=\"desc\">World coordinates use X right and Y up "
           "measured in metres. The current triangle is mapped to a screen "
           "with vertical image coordinates pointing down. A query point "
           "is shown with its approximate classification. The triangle "
           "positions were computed by the same C++23 numeric model as "
           "the console.</desc>\n"
           "<rect width=\"760\" height=\"540\" fill=\"#fbfaf6\"/>\n"
           "<text x=\"30\" y=\"34\" font-size=\"20\" "
           "font-family=\"sans-serif\" fill=\"#264453\">"
           "Your triangle, your coordinates</text>\n";

    const auto ox = map.world_to_image({0.0,0.0});
    out << "<line x1=\"50\" y1=\"" << ox.v
        << "\" x2=\"710\" y2=\"" << ox.v
        << "\" stroke=\"#a2b7bc\" stroke-width=\"2\"/>\n";
    out << "<line x1=\"" << ox.u
        << "\" y1=\"60\" x2=\"" << ox.u
        << "\" y2=\"460\" stroke=\"#a2b7bc\" stroke-width=\"2\"/>\n";

    out << "<polygon points=\""
        << a.u << "," << a.v << " "
        << b.u << "," << b.v << " "
        << c.u << "," << c.v
        << "\" fill=\"#d0e8dd\" fill-opacity=\"0.9\" "
           "stroke=\"#287d72\" stroke-width=\"3\"/>\n";

    for (const auto& pair : std::array<std::pair<const char*,
                                                book03::ImagePosition2>,3>{{
             {"A",a},{"B",b},{"C",c}}}) {
        out << "<circle cx=\"" << pair.second.u << "\" cy=\""
            << pair.second.v
            << "\" r=\"6\" fill=\"#277e75\" stroke=\"white\" "
               "stroke-width=\"2\"/>\n";
        out << "<text x=\"" << pair.second.u + 10.0
            << "\" y=\"" << pair.second.v - 9.0
            << "\" font-family=\"sans-serif\" font-size=\"14\" "
               "fill=\"#234953\">" << pair.first << "</text>\n";
    }

    out << "<circle cx=\"" << p.u << "\" cy=\"" << p.v
        << "\" r=\"7\" fill=\"#bd7651\" stroke=\"white\" "
           "stroke-width=\"2\"/>\n";
    out << "<text x=\"" << p.u + 12.0 << "\" y=\"" << p.v - 9.0
        << "\" font-family=\"sans-serif\" font-size=\"14\" "
           "fill=\"#a96642\">P</text>\n";
    out << "<rect x=\"30\" y=\"474\" width=\"700\" height=\"43\" rx=\"9\" "
           "fill=\"#eaf2f0\"/>\n";
    out << "<text x=\"44\" y=\"492\" font-family=\"sans-serif\" "
           "font-size=\"13\" fill=\"#365b61\">"
        << "P = (" << queried.x << ", " << queried.y << ") m; "
        << "location = " << to_string(sample.location) << "</text>\n";
    out << "<text x=\"44\" y=\"509\" font-family=\"monospace\" "
           "font-size=\"12\" fill=\"#365b61\">"
        << "Signed twice-area = " << sample.twice_signed_area << "</text>\n";
    out << "</svg>\n";
    out.flush();
    if (!out) throw std::runtime_error("cannot finish interactive SVG");
    std::cout << "Wrote interactive current-triangle SVG "
              << target.string() << '\n';
}

[[nodiscard]] double finite_number(std::string_view text) {
    const std::string copy{text};
    std::size_t count = 0;
    const double value = std::stod(copy, &count);
    if (count != copy.size() || !std::isfinite(value)) {
        throw std::invalid_argument("argument must be a finite number");
    }
    return value;
}

void repl() {
    Triangle2 triangle = reference_world_triangle();
    Point2 last_query{1.5, 1.0};
    std::cout << "Book 03 P03 — Triangle inspector\n"
                 "Commands: point X Y | area | triangle AX AY BX BY CX CY | "
                 "svg FILE | ppm FILE | save FILE | reset | help | quit\n";
    std::string line;
    while (std::cout << "triangle> " && std::getline(std::cin, line)) {
        std::istringstream in{line};
        std::string command;
        in >> command;
        if (command=="quit" || command=="exit") break;
        try {
            if (command=="help") {
                std::cout << "point X Y | area | triangle AX AY BX BY CX CY | "
                             "svg FILE | ppm FILE | save FILE | reset | quit\n";
            } else if (command=="reset") {
                triangle=reference_world_triangle();
                std::cout << "Reset to right triangle A(0,0), B(6,0), C(0,4)\n";
            } else if (command=="area") {
                std::cout << "Twice signed area: "
                          << triangle.twice_signed_area() << '\n';
            } else if (command=="point") {
                std::string a,b,extra;
                if (!(in >> a >> b) || (in >> extra)) {
                    throw std::invalid_argument("point requires two numbers");
                }
                last_query={finite_number(a),finite_number(b)};
                numerical_inspection(triangle,last_query);
            } else if (command=="triangle") {
                std::array<std::string,6> args;
                for(auto& word:args) {
                    if(!(in >> word)) {
                        throw std::invalid_argument(
                            "triangle requires six numbers");
                    }
                }
                std::string extra;
                if(in >> extra) {
                    throw std::invalid_argument("too many triangle numbers");
                }
                const Triangle2 next{
                    {finite_number(args[0]),finite_number(args[1])},
                    {finite_number(args[2]),finite_number(args[3])},
                    {finite_number(args[4]),finite_number(args[5])}};
                (void)next.twice_signed_area();
                triangle=next;
                std::cout << "Updated reference triangle; use 'point' or 'area'\n";
            } else if (command=="svg" || command=="ppm" || command=="save") {
                std::string name,extra;
                if(!(in >> name) || (in >> extra)) {
                    throw std::invalid_argument("provide one output path");
                }
                // Illustration modes deliberately use the fixed image-space
                // reference triangle, which is separately documented.
                if(command=="svg") write_svg(name);
                else if(command=="ppm") write_ppm(name);
                else save_current_svg(name,triangle,last_query);
            } else if(!command.empty()) {
                std::cout << "Unknown command; enter 'help'.\n";
            }
        } catch(const std::exception& e) {
            std::cout << "Input rejected: " << e.what() << '\n';
        }
    }
}

void usage() {
    std::cerr << "Usage:\n"
                 "  book03_triangles inspect [x y]\n"
                 "  book03_triangles ppm OUT.ppm\n"
                 "  book03_triangles svg OUT.svg\n"
                 "  book03_triangles repl\n";
}
} // namespace

int main(int argc, char** argv) {
    if(argc < 2) {
        usage();
        return 2;
    }
    try {
        const std::string_view command{argv[1]};
        if(command=="inspect" && (argc==2 || argc==4)) {
            Point2 p{1.5,1.0};
            if(argc==4) p={finite_number(argv[2]),finite_number(argv[3])};
            numerical_inspection(reference_world_triangle(),p);
        } else if(command=="ppm" && argc==3) {
            write_ppm(argv[2]);
        } else if(command=="svg" && argc==3) {
            write_svg(argv[2]);
        } else if(command=="repl" && argc==2) {
            repl();
        } else {
            usage();
            return 2;
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03/P03 error: " << e.what() << '\n';
        return 1;
    }
}
