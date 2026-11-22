// From Geometry to Images — Chapter 01.
// A tiny drawing program that writes honest, inspectable binary PPM files.
// Try opening the generated image in an image viewer or converting PPM to PNG
// with an independent tool. The image is 2D by design; 3D cameras come later.

#include "book03/pixel_canvas.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

using book03::Canvas;
using book03::Rgb8;

void draw_house(Canvas& picture) {
    constexpr Rgb8 sky{213, 235, 248};
    constexpr Rgb8 lawn{135, 186, 132};
    constexpr Rgb8 wall{248, 233, 195};
    constexpr Rgb8 roof{157, 76, 68};
    constexpr Rgb8 window{104, 166, 208};
    constexpr Rgb8 door{114, 82, 65};
    constexpr Rgb8 ink{49, 52, 70};

    picture.clear(sky);
    picture.fill_rectangle(0, 170, 320, 200, lawn);
    picture.fill_rectangle(90, 90, 231, 171, wall);
    picture.fill_rectangle(190, 121, 217, 151, window);
    picture.fill_rectangle(130, 130, 159, 171, door);

    // The roof is an outline, not yet a triangle mesh or lighted surface.
    picture.draw_line({72, 92}, {160, 32}, roof);
    picture.draw_line({160, 32}, {249, 92}, roof);
    picture.draw_line({72, 92}, {249, 92}, roof);

    // A tiny window cross creates a familiar feature that is easy to inspect.
    picture.draw_line({203, 121}, {203, 150}, ink);
    picture.draw_line({190, 136}, {216, 136}, ink);
}

void draw_line_study(Canvas& picture) {
    constexpr Rgb8 background{250, 249, 245};
    constexpr Rgb8 dark{36, 55, 78};
    constexpr Rgb8 orange{204, 92, 40};

    picture.clear(background);
    picture.draw_line({12, 12}, {306, 184}, dark);
    picture.draw_line({12, 184}, {306, 12}, orange);
    picture.draw_line({160, 0}, {160, 199}, dark);
    picture.draw_line({0, 100}, {319, 100}, orange);
    picture.draw_line({0, 0}, {0, 0}, dark); // one-pixel "line"
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: book03_canvas house|lines output.ppm\n";
        return 2;
    }

    const std::string_view mode{argv[1]};
    if (mode != "house" && mode != "lines") {
        std::cerr << "Choose 'house' or 'lines'.\n";
        return 2;
    }

    try {
        Canvas image{320, 200};
        if (mode == "house") {
            draw_house(image);
        } else {
            draw_line_study(image);
        }

        const std::filesystem::path path{argv[2]};
        std::ofstream stream{path, std::ios::binary | std::ios::trunc};
        if (!stream) {
            throw std::runtime_error("could not open output image file");
        }
        image.write_ppm(stream);
        stream.flush();
        if (!stream) {
            throw std::runtime_error("could not finish output image file");
        }
        std::cout << "Wrote " << path.string() << " (" << image.width()
                  << " x " << image.height() << ", RGB8 P6)\n";
    } catch (const std::exception& error) {
        std::cerr << "Book 03 canvas failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
