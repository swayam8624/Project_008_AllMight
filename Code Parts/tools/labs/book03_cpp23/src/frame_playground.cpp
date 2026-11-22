// Book 03 / P02 — coordinate frame playground.
//
// Numeric geometry is computed ONLY by frame_math.hpp. SVG is a visualization
// of those calculations, not an independent geometry implementation.
// The REPL lets the reader change an object's world origin or rotation, ask
// for mapped points, and regenerate a vector diagram on demand.

#include "book03/frame_math.hpp"

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

using book03::Frame2;
using book03::ImageMap2;
using book03::ImagePosition2;
using book03::Point2;
using book03::Vector2;

constexpr int canvas_width = 760;
constexpr int canvas_height = 540;

// This is a 2D orthographic drawing convention: world +Y is page up,
// image +v is down. The scale is explicitly measured in pixels/metre.
const ImageMap2 diagram_map{{380.5, 300.5}, 65.0, 65.0,
                            canvas_width, canvas_height};

double parse_finite(std::string_view text) {
    const std::string copy{text};
    std::size_t consumed = 0;
    const double value = std::stod(copy, &consumed);
    if (consumed != copy.size() || !std::isfinite(value)) {
        throw std::invalid_argument("expected a finite number");
    }
    return value;
}

Frame2 frame_for(double x, double y, double degrees) {
    if (std::abs(x) > 1000.0 || std::abs(y) > 1000.0 ||
        std::abs(degrees) > 1'000'000.0) {
        throw std::invalid_argument("frame parameters exceed teaching limits");
    }
    return Frame2::rotation({x, y}, book03::degrees_to_radians(degrees));
}

void svg_line(std::ostream& out, ImagePosition2 a, ImagePosition2 b,
              std::string_view color, double width = 2.0) {
    out << "<line x1=\"" << a.u << "\" y1=\"" << a.v
        << "\" x2=\"" << b.u << "\" y2=\"" << b.v
        << "\" stroke=\"" << color << "\" stroke-width=\"" << width
        << "\" stroke-linecap=\"round\"/>\n";
}

// Draw a point in the WORLD frame after mapping to image pixels.
void svg_world_dot(std::ostream& out, Point2 world,
                   std::string_view fill, double radius = 5.0) {
    const auto image = diagram_map.world_to_image(world);
    out << "<circle cx=\"" << image.u << "\" cy=\"" << image.v
        << "\" r=\"" << radius << "\" fill=\"" << fill
        << "\" stroke=\"#fff\" stroke-width=\"1.5\"/>\n";
}

void write_diagram(std::ostream& out, const Frame2& frame) {
    frame.validate();

    out << std::fixed << std::setprecision(4);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "width=\"760\" height=\"540\" viewBox=\"0 0 760 540\" "
           "role=\"img\" aria-labelledby=\"title desc\">\n";
    out << "<title id=\"title\">Two coordinate frames and a transformed "
           "triangle</title>\n";
    out << "<desc id=\"desc\">An original 2D geometry teaching diagram. "
           "The global axes use metres and positive world Y up. A triangle "
           "is constructed in a translated and rotated local frame and "
           "transformed to world coordinates using the C++23 frame model."
           "</desc>\n";
    out << "<rect width=\"760\" height=\"540\" rx=\"16\" fill=\"#fbfaf6\"/>\n";
    out << "<text x=\"30\" y=\"32\" fill=\"#23384b\" font-size=\"19\" "
           "font-family=\"sans-serif\" font-weight=\"600\">"
           "The house has a frame of its own</text>\n";
    out << "<text x=\"30\" y=\"52\" fill=\"#667784\" font-size=\"12\" "
           "font-family=\"sans-serif\">World units: metres. Image v points down.</text>\n";

    // An honest reference grid generated in world metre increments. Axis
    // positions are calculated by the SAME image map used for the triangle.
    for (int x = -5; x <= 5; ++x) {
        svg_line(out,
                 diagram_map.world_to_image({static_cast<double>(x), -3.5}),
                 diagram_map.world_to_image({static_cast<double>(x), 3.5}),
                 "#e2e8e6", 1.0);
    }
    for (int y = -3; y <= 3; ++y) {
        svg_line(out,
                 diagram_map.world_to_image({-5.5, static_cast<double>(y)}),
                 diagram_map.world_to_image({5.5, static_cast<double>(y)}),
                 "#e2e8e6", 1.0);
    }
    svg_line(out, diagram_map.world_to_image({-5.5, 0.0}),
             diagram_map.world_to_image({5.5, 0.0}), "#617583", 2.0);
    svg_line(out, diagram_map.world_to_image({0.0, -3.5}),
             diagram_map.world_to_image({0.0, 3.5}), "#617583", 2.0);

    // Three local points define one triangle. The world points below all
    // come from Frame2::local_to_world. No duplicate rotation formulas
    // are embedded in the drawing.
    const std::array<Point2, 3> local{{
        {0.0, 0.0}, {2.0, 0.0}, {0.7, 1.5}
    }};
    std::array<Point2, 3> world{};
    for (std::size_t i = 0; i < local.size(); ++i) {
        world[i] = frame.local_to_world(local[i]);
    }

    out << "<polygon points=\"";
    for (const auto& p : world) {
        const auto q = diagram_map.world_to_image(p);
        out << q.u << "," << q.v << " ";
    }
    out << "\" fill=\"#c4e7db\" fill-opacity=\"0.8\" "
           "stroke=\"#247b70\" stroke-width=\"3\"/>\n";

    const auto origin = diagram_map.world_to_image(frame.origin);
    const Vector2 along_x = frame.transform_vector({1.0, 0.0});
    const Vector2 along_y = frame.transform_vector({0.0, 1.0});
    const auto tip_x = diagram_map.world_to_image(frame.origin + along_x);
    const auto tip_y = diagram_map.world_to_image(frame.origin + along_y);
    svg_line(out, origin, tip_x, "#bd6746", 4.0);
    svg_line(out, origin, tip_y, "#397eb8", 4.0);
    svg_world_dot(out, frame.origin, "#cf8257", 7.0);
    for (const auto& p : world) {
        svg_world_dot(out, p, "#1b7168");
    }

    out << "<text x=\"" << tip_x.u + 8.0 << "\" y=\"" << tip_x.v - 4.0
        << "\" font-size=\"13\" fill=\"#9d492b\" font-family=\"sans-serif\">"
           "local x</text>\n";
    out << "<text x=\"" << tip_y.u + 8.0 << "\" y=\"" << tip_y.v - 4.0
        << "\" font-size=\"13\" fill=\"#245d8c\" font-family=\"sans-serif\">"
           "local y</text>\n";
    out << "<rect x=\"20\" y=\"477\" width=\"720\" height=\"43\" rx=\"8\" "
           "fill=\"#e9f2f1\"/>\n";
    out << "<text x=\"32\" y=\"494\" font-size=\"12\" "
           "font-family=\"monospace\" fill=\"#365967\">"
           "Local triangle: (0,0), (2,0), (0.7,1.5) metres</text>\n";
    out << "<text x=\"32\" y=\"511\" font-size=\"12\" "
           "font-family=\"monospace\" fill=\"#365967\">"
           "Frame world origin: (" << frame.origin.x << ","
        << frame.origin.y << ") metres</text>\n";
    out << "</svg>\n";
    if (!out) {
        throw std::runtime_error("failed writing SVG data");
    }
}

void write_svg_file(const std::filesystem::path& path, const Frame2& frame) {
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file) {
        throw std::runtime_error("could not open SVG output file");
    }
    write_diagram(file, frame);
    file.flush();
    if (!file) {
        throw std::runtime_error("could not finish SVG output file");
    }
    std::cout << "Wrote " << path.string() << " from C++23 geometry\n";
}

void inspect_demo() {
    const Frame2 house = frame_for(2.0, 1.0, 90.0);
    const Point2 local{3.0, 1.0};
    const Point2 world = house.local_to_world(local);
    const Point2 returned = house.world_to_local(world);
    const ImageMap2 map{{320.5, 240.5}, 60.0, 60.0, 640, 480};
    const auto image = map.world_to_image({1.0, 2.0});

    std::cout << std::fixed << std::setprecision(8);
    std::cout << "House local point: (" << local.x << ", " << local.y
              << ") metres\nWorld point: (" << world.x << ", " << world.y
              << ") metres\nInverse local point: (" << returned.x
              << ", " << returned.y << ") metres\n";
    std::cout << "World (1,2) m to continuous pixel: (" << image.u
              << ", " << image.v << ")\n";
    if (const auto pixel = map.pixel_at(image)) {
        std::cout << "Stored pixel address: (" << pixel->column
                  << ", " << pixel->row << ")\n";
    }
}

// Small native interactive terminal: change frame origin and rotation, ask
// where a local point is, or write a fresh SVG from the current transform.
// The REPL uses C++23 math directly; no JavaScript transform replica exists.
void repl() {
    double x = 1.0;
    double y = 0.5;
    double angle_deg = 35.0;
    std::cout << "Book03/P02 — frame playground\n"
              << "Commands: origin X Y | angle DEGREES | point X Y | "
                 "save FILE.svg | inspect | help | quit\n";
    std::string line;
    while (std::cout << "frame> " && std::getline(std::cin, line)) {
        std::istringstream input{line};
        std::string command;
        input >> command;
        if (command == "quit" || command == "exit") break;
        try {
            if (command == "help") {
                std::cout << "origin X Y, angle DEGREES, point X Y, "
                             "save FILE.svg, inspect, quit\n";
            } else if (command == "origin" || command == "point") {
                std::string first, second, extra;
                if (!(input >> first >> second) || (input >> extra)) {
                    throw std::invalid_argument("supply exactly two numbers");
                }
                const double a = parse_finite(first);
                const double b = parse_finite(second);
                if (command == "origin") {
                    (void)frame_for(a, b, angle_deg);
                    x = a;
                    y = b;
                    std::cout << "World origin now (" << x << "," << y
                              << ") metres\n";
                } else {
                    const auto world =
                        frame_for(x, y, angle_deg).local_to_world({a, b});
                    std::cout << "Local (" << a << "," << b
                              << ") m -> world (" << world.x << ","
                              << world.y << ") m\n";
                }
            } else if (command == "angle") {
                std::string first, extra;
                if (!(input >> first) || (input >> extra)) {
                    throw std::invalid_argument("supply exactly one angle");
                }
                const double a = parse_finite(first);
                (void)frame_for(x, y, a);
                angle_deg = a;
                std::cout << "Rotation now " << angle_deg << " degrees\n";
            } else if (command == "save") {
                std::string name, extra;
                if (!(input >> name) || (input >> extra)) {
                    throw std::invalid_argument("supply one SVG path");
                }
                write_svg_file(name, frame_for(x, y, angle_deg));
            } else if (command == "inspect") {
                inspect_demo();
            } else if (!command.empty()) {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        } catch (const std::exception& err) {
            std::cout << "Input rejected: " << err.what() << '\n';
        }
    }
}

void usage() {
    std::cerr << "Usage:\n"
                 "  book03_frames inspect\n"
                 "  book03_frames svg OUT.svg [originX originY angleDegrees]\n"
                 "  book03_frames repl\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }
    try {
        const std::string_view command{argv[1]};
        if (command == "inspect" && argc == 2) {
            inspect_demo();
        } else if (command == "svg" && (argc == 3 || argc == 6)) {
            Frame2 frame = frame_for(1.0, 0.5, 35.0);
            if (argc == 6) {
                frame = frame_for(parse_finite(argv[3]),
                                  parse_finite(argv[4]),
                                  parse_finite(argv[5]));
            }
            write_svg_file(argv[2], frame);
        } else if (command == "repl" && argc == 2) {
            repl();
        } else {
            usage();
            return 2;
        }
        return 0;
    } catch (const std::exception& err) {
        std::cerr << "Book03/P02 error: " << err.what() << '\n';
        return 1;
    }
}
