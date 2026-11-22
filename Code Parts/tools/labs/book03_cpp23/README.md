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


---

## Chapter 02 — P02 Coordinate Frame Playground

The next chapter introduces a different source of error: a location has no meaning until its coordinate frame, units and transformation direction are declared. Its C++23 mathematics is intentionally kept outside the picture serializer, so the new code can be checked against exact scalar examples.

**New files**

- include/book03/frame_math.hpp — separate Point2 and Vector2 types (point-plus-point not supported, but local/world frame tags are not yet compile-time types); Frame2 local-to-world/inverse/vector mapping; parent/child composition; ImageMap2 with positive pixels-per-metre scale and image-down v convention. Both finite-input and singular-basis guards are explicit.
- src/frame_playground.cpp — independent numerical inspection, native terminal REPL and SVG diagram generated directly from the frame model. No graphics library, JS geometry duplicate, GPU or network access.
- tests/frame_math_tests.cpp — 90-degree turn; sheared nonorthogonal inverse; singular and nearly singular axes; composition order; translation invariance of vectors; image bounds; continuous pixel round-trip; malformed and nonfinite inputs.

**CMake targets:** book03_frames and book03_frame_tests. The existing book03_canvas and book03_canvas_tests remain intact. Both test executables are registered with CTest in the same lab.

**P02 commands for a later standalone validation**

~~~bash
cmake -S "Code Parts/tools/labs/book03_cpp23" -B build/book03-p01 -DCMAKE_BUILD_TYPE=Release
cmake --build build/book03-p01 --parallel 4
ctest --test-dir build/book03-p01 --output-on-failure
build/book03-p01/book03_frames inspect
build/book03-p01/book03_frames svg build/book03-p01/frame.svg
build/book03-p01/book03_frames svg build/book03-p01/rotated.svg 2 1 90
build/book03-p01/book03_frames repl
~~~

Within the REPL, try entering \`origin 2 1\`, \`angle 90\`, \`point 3 1\`, \`save diagram.svg\` and \`quit\`. The expected reported world point is approximately (1,4) metres. SVG exports visualize the current local triangle over an image-oriented coordinate grid. The actual geometric correctness evidence remains the mathematical tests, not the mere appearance of the vector diagram.

**Known boundaries:** This is a 2D affine teaching model. A 3D scene graph requires third axes, homogeneous conventions and pose contracts; a pinhole camera requires intrinsics, perspective and clipping. The inverse rejects bases whose determinant is below a deliberately conservative relative/absolute threshold. It is not an adaptive exact arithmetic proof and is not intended as a universal geometric singularity oracle.

The series publication folder must remain unchanged. No new PDF/EPUB should appear during these lab runs.


---

## Chapter 03 — P03 Triangle and Barycentric Studio

The triangle project extends **the same** C++23 course, not a renamed publication or independent toy application. Its mathematical point values come from P02, and its generated PPM image uses P01's actual bounded RGB8 Canvas.

**Read and inspect these sources in order:**

1. \`include/book03/triangle_geometry.hpp\` — exact bounded \`IntegerPoint2\` orientation/containment; floating \`Triangle2\` barycentric sampling, signed twice-area, interpolation and deliberately noncertified near-boundary uncertainty. Read the source's numeric-domain proof.
2. \`tests/triangle_geometry_tests.cpp\` — an independent shoelace reference, 4,000 deterministic integer-oracle checks, winding/translation/reflection properties, affine invariance of weights, interpolation values, bad-input and degeneracy tests.
3. \`src/triangle_studio.cpp\` — CLI numeric inspection, C++-generated barycentric PPM image, SVG diagram and an interactive REPL. No camera API or GPU is hidden in the implementation.
4. \`CMakeLists.txt\` — target \`book03_triangles\` and independent CTest target \`book03_triangle_tests\` added alongside existing chapter projects.

**Student-visible P03 interface**

~~~text
book03_triangles inspect
book03_triangles inspect 5 3
book03_triangles ppm triangle-gradient.ppm
book03_triangles svg triangle-static.svg
book03_triangles repl
~~~

The \`inspect\` mode's default triangle uses Cartesian +Y-up coordinates: A=(0,0), B=(6,0), C=(0,4), query P=(1.5,1). It prints signed twice-area=24, weights=(0.5,0.25,0.25), an interior classification and the 17.5 scalar interpolation result for corner values 10/20/30.

The \`ppm\` and \`svg\` standalone commands are controlled **image-coordinate demonstrations** on a fixed 640×420 triangle with +v downward. Do not mistake the negative numeric winding of that example for an error in the +Y-up reference triangle. The colored triangle is a teaching interpolation of encoded channel values, not a physically linear-light result.

The \`repl\` is interactive and can now write a separate SVG using the **current user-edited triangle**, not the fixed sample plate:

~~~text
triangle 0 0 6 0 0 4
point 1.5 1
save your-triangle.svg
point 5 3
area
quit
~~~

The \`save\` command draws the current Cartesian triangle and last query using a documented 25 px/metre world-to-image projection; for visual legibility, all points must be within [-10,10] m in x and y. The numeric inspector itself is not artificially restricted to that SVG preview domain.

**Correctness boundaries**

The exact orientation and containment results require integer coordinates within [-1e6,+1e6]. This limit is proven to fit \`int64_t\` determinant operations. This is a **true bounded exact oracle**, not a claim of a full arbitrary-precision kernel.

The floating barycentric query is **not certified numerically** for all representable binary64 coordinates. It returns \`BoundaryUncertain\` or \`DegenerateOrUncertified\` when its documented guard is too small for a safe classroom classification. That guard is an explanatory filter, not a rigorous adaptive error bound. Chapter 05 will cover such methods in much greater depth with the pre-existing manuscript preserved.

**Validation protocol:** Build the lab's CMake targets, execute all CTests, inspect P03's numeric output, check PPM header/payload and SVG XML, inspect the gradient visually, and record toolchain + source identity. The GitHub CI file defines the same checks but must not be called passing until an actual successful run is observed. The author still owns the final Mac Book 03 PDF/EPUB build.
