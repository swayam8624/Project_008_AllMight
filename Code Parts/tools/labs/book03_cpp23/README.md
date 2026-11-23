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


---

## Chapter 04 — P04 Affine Transformation Studio

**Why this project exists.** A picture of a transformed paper house can be convincing even when the matrix is mathematically wrong. P04 therefore puts a compact 3×3 homogeneous affine reference between the existing point/vector concepts, P03 triangle queries and P01 image output. It explains what moves, what stays local, and what an inverse can safely undo.

### Read in this order

1. \`include/book03/affine2.hpp\` — the six coefficients of a column-vector 2D affine map, an explicit 3×3 row-major **storage** export, translation/rotation/scale/shear/reflection constructors, matrix-composition operator, pivot transformation, conservative inverse, and inverse-transpose normal transform. Inspect all comments about frames and numerical limitations.
2. \`tests/affine2_tests.cpp\` — independent hand-calculated examples, comparison against the P02 \`Frame2\` contract, P03 triangle determinant/weight invariance, normal perpendicularity, invalid/singular transforms, and 2,000 deterministic seeded round trips.
3. \`src/transform_studio.cpp\` — inspectable world-space matrices, immutable local house vertices, C++-generated SVG and PPM, a numerical transform-order illustration, and a terminal REPL that can save its current transformed house.
4. \`CMakeLists.txt\` — \`book03_transforms\` executable, \`book03_affine_tests\` test binary, and \`book03_affine_transform_contracts\` CTest next to the previous three chapters' targets.

### Student interface

~~~text
book03_transforms inspect
book03_transforms svg transformed-house.svg
book03_transforms ppm transformed-house.ppm
book03_transforms order two-orders.svg
book03_transforms repl
~~~

In the REPL:

~~~text
reset
rotate 90
translate 2 0
point 1 0
inspect
svg my-transformed-house.svg
ppm my-transformed-house.ppm
quit
~~~

Each new command is **pre-multiplied in world space**. The line above rotates the local point first and translates second; the \`point\` command should report approximately world (2,1) metres. The diagram and image use the same exact C++23 \`Affine2\` calculation, with the shared P02 \`ImageMap2\` converting world +Y up to image +v down. The two-order SVG displays why \`T*R\` and \`R*T\` are not interchangeable.

The demonstration exports are limited to an 840×580 image canvas and require the transformed house's vertices to remain inside it. When a user moves a vertex beyond the visible bounds, the export refuses the operation rather than pretending that clipped geometry was implemented. Clipping belongs to Chapter 17 and is not hidden in this early lab.

### Evidence and known limits

P04 uses explicit row-major array **serialization** for printed homogeneous matrices while still applying them to column vectors. The matrix's top-left 2×2 linear part maps vectors. Translation affects points. Normals use the inverse transpose. Inverse evaluation rejects a zero determinant and matrices that fail a conservative relative-determinant threshold. It is neither a universal conditioning estimate nor a substitute for an exact predicate library.

P04 RGB8 output intentionally paints simplified flat-color triangles. It is **not** physically based rendering, a production camera, a depth-buffered rasterizer, or a GPU experiment. The exact triangle test developed earlier retains its separate bounded integer domain. This project is about trustworthy transforms, not prematurely claiming capabilities that later books will develop.

The CI workflow checks target compilation, CTest execution, numeric REPL order, PPM byte size and SVG structure once it is actually run. **Do not report “passed” merely because the checks are written.** The final combined PDF/EPUB build remains the author's later Mac-side review; running this lab must not write to the established local \`From Bits to Research/\` publication library.


---

## Chapter 05 — P05 Exact-Orientation Laboratory

**Purpose.** Demonstrate a *real*, reproducible topology failure in ordinary binary64 product-subtraction and fix that decision with an exact sign for the **stored** finite input coordinates. It is the next chapter in the cumulative Book 03 project; nothing about P01–P04 needs to be overwritten.

### Read these three new files

1. \`include/book03/exact_orient2d.hpp\` — binary64-to-integer bit decoding; magnitude limbs and carry-safe exact add/subtract/multiply; bounded integer path; exact dyadic fallback; exact triangle point containment. It also includes a deliberate *naive* diagnostic with **two separately rounded products**, using volatile stores solely to avoid optimization silently fusing away the teaching example.
2. \`tests/exact_orient2d_tests.cpp\` — exact minus-four cancellation; smallest subnormal positive orientation even when the ordinary product underflows; maximum finite-coordinate extreme cases; signed zero, invalid inputs, exact on-edge classification, 5,000 deterministic half-integer inputs compared with the independent P03 bounded-integer oracle.
3. \`src/precision_studio.cpp\` — inspect stored coordinates (17 significant digits), naive numerical result, exact signed result, and the chosen path; write a fixture CSV and a mathematically grounded diagnostic SVG; interactively enter six finite coordinates and save the corresponding report.

### The numerical contract in one sentence

For **finite IEEE-754 binary64 coordinate values actually represented in memory**, the exact oracle computes the correct sign of the 2D orientation determinant using a bounded int64 oracle or exact arbitrary-limb signed integer arithmetic.

The core insight is that every finite binary64 \(x\) can be written as an exact signed integer \(N_x 2^{-1074}\). On this common scale, the whole orientation determinant's sign is the exact sign of an integer expression. The code requires the standard eight-byte IEEE binary64 format and documents the 32-bit limb arithmetic proof. It never promises correctness relative to lost pre-storage measurement/decimal information.

**This is NOT a Shewchuk-style adaptive floating error-bound filter.** It is adaptive by accepted integer range and exact dyadic fallback, not a claim to best-in-class runtime. The paper by Jonathan Shewchuk is credited in the narrative bibliography and inspected source ledger; no third-party implementation was imported.

### Reader commands

~~~text
book03_precision inspect
book03_precision point 0 0 10000000000000000 10000000000000002 10000000000000002 10000000000000004
book03_precision csv orientation-cases.csv
book03_precision svg orientation-decisions.svg
book03_precision repl
~~~

The initial hard case is deliberately **not** to scale:

- \(A=(0,0)\), \(B=(N,N+2)\), \(C=(N+2,N+4)\), with \(N=10^{16}\).
- All coordinates are representable. Exact determinant: \(-4\). Naive two-rounded-product determinant: zero.
- \`orient_binary64\` must report exact sign \(-1\) using the dyadic limb path.
- Another fixture uses the smallest positive subnormal \(\eta=2^{-1074}\). Its exact area-sign is positive, but the naive \(\eta^2\) product underflows.
- A third fixture demonstrates positive exact sign despite overflowing naive product. A fourth shows **input rounding**: \(N+1\) was not preserved in binary64, so no exact predicate can reconstruct it.

The \`repl\` accepts \`triangle ax ay bx by cx cy\`, \`inspect\`, \`cases\`, \`csv PATH\`, \`svg PATH\`, \`reset\` and \`quit\`. Every report is computed from the same C++23 exact predicate. SVG panels are **qualitative computed diagnoses**. No benchmark or statistical speedups are invented.

### Verification plan

The CMake targets are \`book03_precision\` and \`book03_exact_tests\`; the CTest name is \`book03_binary64_exact_orientation_contracts\`, registered beside earlier four tests. The GitHub workflow is authored to compile all five projects, execute CTest, inspect exact and naive fixture values, parse the generated CSV and SVG, and exercise the numerical REPL.

**Do not call a test passing until observed compiler and test logs confirm it.** Cross-compiler or performance claims require separate execution. All source outputs are local build artifacts, not Book 03 EPUB/PDF releases. Leave the \`From Bits to Research/\` publication library, compiled LaTeX staging, original books and continuous notes untouched.
