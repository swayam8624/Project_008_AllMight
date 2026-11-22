# Book 03 / P01 — Pixel Canvas

**Book:** From Geometry to Images.  
**Chapter:** 01, The Picture Made of Little Windows.  
**Project state:** Source and tests are authored in Pass 01. Native execution results are a separate evidence requirement; no macOS compilation or full-book build is claimed by adding these files.

## What this program actually proves

This small laboratory turns image coordinates and encoded RGB values into inspectable P6 PPM bytes. Its logic is independent of graphics APIs, drivers and devices. The source shows how the canvas owns a finite, validated array of pixels, why each pixel index is in bounds, and how a line walker decides which discrete sample addresses to visit.

It deliberately does **not** simulate a 3D camera, physically correct light transport, perspective projection, subpixel coverage, image reconstruction filters or a color-managed rendering pipeline. These are substantial later chapters. The tiny house is a teaching drawing, not a claim of a finished renderer.

## Files and reading order

1. include/book03/pixel_canvas.hpp — pixel format; coordinate conventions; size validation; rectangle/line drawing; P6 serialization. Read the invariant comments before individual methods.
2. src/main.cpp — two deterministic example images: a drawn house and an aliasing/line study.
3. tests/pixel_canvas_tests.cpp — exact file-byte oracle, malformed dimension rejection, half-open rectangles, line path/ties, all octants and stream failure.
4. CMakeLists.txt — standard C++23 targets and CTest integration.

## Output contract

Executable: book03_canvas

- house output.ppm — draw a small house with an outlined roof, window, door and grass.
- lines output.ppm — generate diagonal/horizontal/vertical sample paths for inspection.

The generated PPM has header P6, width, height, max channel value 255 and exactly 3*width*height bytes of RGB channel data. It is a visual artifact, not the complete scientific proof. All tests use deterministic values and require no network, Python, images or GPU.

## Testing protocol for a later local validation pass

Configure a CMake C++23 build of this directory, build the book03_canvas and book03_canvas_tests targets, run CTest, generate both example files, verify their headers/dimensions and visually inspect them in an image viewer. For editorial acceptance, also record the exact compiler, CMake version, source commit and output hashes.

The author plans to run Mac-side builds after the entire Book 03 manuscript is developed. Do **not** treat this README as a request for an interim Mac compilation. The CMake project can be exercised independently before then by the authoring environment if available, but results must be reported explicitly.

## Why PPM instead of PNG now?

Binary PPM gives the reader a chance to follow a named color channel all the way into exact file bytes without introducing compression and third-party format libraries. A later image asset pipeline can write PNG through an explicit, tested encoder. We intentionally separate “the pixel is correct” from “a viewer displayed the image attractively.”

## Preservation

Source lives under Code Parts/tools/labs/book03_cpp23/ in the **new repository layout**. The book manuscript lives under Notes/Book Production/Seven-Book Curriculum/Book03/. No generated PDF or EPUB is written into From Bits to Research/ by this laboratory.
