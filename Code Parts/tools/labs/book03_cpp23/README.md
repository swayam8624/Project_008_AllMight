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

Within the REPL, try entering `origin 2 1`, `angle 90`, `point 3 1`, `save diagram.svg` and `quit`. The expected reported world point is approximately (1,4) metres. SVG exports visualize the current local triangle over an image-oriented coordinate grid. The actual geometric correctness evidence remains the mathematical tests, not the mere appearance of the vector diagram.

**Known boundaries:** This is a 2D affine teaching model. A 3D scene graph requires third axes, homogeneous conventions and pose contracts; a pinhole camera requires intrinsics, perspective and clipping. The inverse rejects bases whose determinant is below a deliberately conservative relative/absolute threshold. It is not an adaptive exact arithmetic proof and is not intended as a universal geometric singularity oracle.

The series publication folder must remain unchanged. No new PDF/EPUB should appear during these lab runs.


---

## Chapter 03 — P03 Triangle and Barycentric Studio

The triangle project extends **the same** C++23 course, not a renamed publication or independent toy application. Its mathematical point values come from P02, and its generated PPM image uses P01's actual bounded RGB8 Canvas.

**Read and inspect these sources in order:**

1. `include/book03/triangle_geometry.hpp` — exact bounded `IntegerPoint2` orientation/containment; floating `Triangle2` barycentric sampling, signed twice-area, interpolation and deliberately noncertified near-boundary uncertainty. Read the source's numeric-domain proof.
2. `tests/triangle_geometry_tests.cpp` — an independent shoelace reference, 4,000 deterministic integer-oracle checks, winding/translation/reflection properties, affine invariance of weights, interpolation values, bad-input and degeneracy tests.
3. `src/triangle_studio.cpp` — CLI numeric inspection, C++-generated barycentric PPM image, SVG diagram and an interactive REPL. No camera API or GPU is hidden in the implementation.
4. `CMakeLists.txt` — target `book03_triangles` and independent CTest target `book03_triangle_tests` added alongside existing chapter projects.

**Student-visible P03 interface**

~~~text
book03_triangles inspect
book03_triangles inspect 5 3
book03_triangles ppm triangle-gradient.ppm
book03_triangles svg triangle-static.svg
book03_triangles repl
~~~

The `inspect` mode's default triangle uses Cartesian +Y-up coordinates: A=(0,0), B=(6,0), C=(0,4), query P=(1.5,1). It prints signed twice-area=24, weights=(0.5,0.25,0.25), an interior classification and the 17.5 scalar interpolation result for corner values 10/20/30.

The `ppm` and `svg` standalone commands are controlled **image-coordinate demonstrations** on a fixed 640×420 triangle with +v downward. Do not mistake the negative numeric winding of that example for an error in the +Y-up reference triangle. The colored triangle is a teaching interpolation of encoded channel values, not a physically linear-light result.

The `repl` is interactive and can now write a separate SVG using the **current user-edited triangle**, not the fixed sample plate:

~~~text
triangle 0 0 6 0 0 4
point 1.5 1
save your-triangle.svg
point 5 3
area
quit
~~~

The `save` command draws the current Cartesian triangle and last query using a documented 25 px/metre world-to-image projection; for visual legibility, all points must be within [-10,10] m in x and y. The numeric inspector itself is not artificially restricted to that SVG preview domain.

**Correctness boundaries**

The exact orientation and containment results require integer coordinates within [-1e6,+1e6]. This limit is proven to fit `int64_t` determinant operations. This is a **true bounded exact oracle**, not a claim of a full arbitrary-precision kernel.

The floating barycentric query is **not certified numerically** for all representable binary64 coordinates. It returns `BoundaryUncertain` or `DegenerateOrUncertified` when its documented guard is too small for a safe classroom classification. That guard is an explanatory filter, not a rigorous adaptive error bound. Chapter 05 will cover such methods in much greater depth with the pre-existing manuscript preserved.

**Validation protocol:** Build the lab's CMake targets, execute all CTests, inspect P03's numeric output, check PPM header/payload and SVG XML, inspect the gradient visually, and record toolchain + source identity. The GitHub CI file defines the same checks but must not be called passing until an actual successful run is observed. The author still owns the final Mac Book 03 PDF/EPUB build.


---

## Chapter 04 — P04 Affine Transformation Studio

**Why this project exists.** A picture of a transformed paper house can be convincing even when the matrix is mathematically wrong. P04 therefore puts a compact 3×3 homogeneous affine reference between the existing point/vector concepts, P03 triangle queries and P01 image output. It explains what moves, what stays local, and what an inverse can safely undo.

### Read in this order

1. `include/book03/affine2.hpp` — the six coefficients of a column-vector 2D affine map, an explicit 3×3 row-major **storage** export, translation/rotation/scale/shear/reflection constructors, matrix-composition operator, pivot transformation, conservative inverse, and inverse-transpose normal transform. Inspect all comments about frames and numerical limitations.
2. `tests/affine2_tests.cpp` — independent hand-calculated examples, comparison against the P02 `Frame2` contract, P03 triangle determinant/weight invariance, normal perpendicularity, invalid/singular transforms, and 2,000 deterministic seeded round trips.
3. `src/transform_studio.cpp` — inspectable world-space matrices, immutable local house vertices, C++-generated SVG and PPM, a numerical transform-order illustration, and a terminal REPL that can save its current transformed house.
4. `CMakeLists.txt` — `book03_transforms` executable, `book03_affine_tests` test binary, and `book03_affine_transform_contracts` CTest next to the previous three chapters' targets.

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

Each new command is **pre-multiplied in world space**. The line above rotates the local point first and translates second; the `point` command should report approximately world (2,1) metres. The diagram and image use the same exact C++23 `Affine2` calculation, with the shared P02 `ImageMap2` converting world +Y up to image +v down. The two-order SVG displays why `T*R` and `R*T` are not interchangeable.

The demonstration exports are limited to an 840×580 image canvas and require the transformed house's vertices to remain inside it. When a user moves a vertex beyond the visible bounds, the export refuses the operation rather than pretending that clipped geometry was implemented. Clipping belongs to Chapter 17 and is not hidden in this early lab.

### Evidence and known limits

P04 uses explicit row-major array **serialization** for printed homogeneous matrices while still applying them to column vectors. The matrix's top-left 2×2 linear part maps vectors. Translation affects points. Normals use the inverse transpose. Inverse evaluation rejects a zero determinant and matrices that fail a conservative relative-determinant threshold. It is neither a universal conditioning estimate nor a substitute for an exact predicate library.

P04 RGB8 output intentionally paints simplified flat-color triangles. It is **not** physically based rendering, a production camera, a depth-buffered rasterizer, or a GPU experiment. The exact triangle test developed earlier retains its separate bounded integer domain. This project is about trustworthy transforms, not prematurely claiming capabilities that later books will develop.

The CI workflow checks target compilation, CTest execution, numeric REPL order, PPM byte size and SVG structure once it is actually run. **Do not report “passed” merely because the checks are written.** The final combined PDF/EPUB build remains the author's later Mac-side review; running this lab must not write to the established local `From Bits to Research/` publication library.


---

## Chapter 05 — P05 Exact-Orientation Laboratory

**Purpose.** Demonstrate a *real*, reproducible topology failure in ordinary binary64 product-subtraction and fix that decision with an exact sign for the **stored** finite input coordinates. It is the next chapter in the cumulative Book 03 project; nothing about P01–P04 needs to be overwritten.

### Read these three new files

1. `include/book03/exact_orient2d.hpp` — binary64-to-integer bit decoding; magnitude limbs and carry-safe exact add/subtract/multiply; bounded integer path; exact dyadic fallback; exact triangle point containment. It also includes a deliberate *naive* diagnostic with **two separately rounded products**, using volatile stores solely to avoid optimization silently fusing away the teaching example.
2. `tests/exact_orient2d_tests.cpp` — exact minus-four cancellation; smallest subnormal positive orientation even when the ordinary product underflows; maximum finite-coordinate extreme cases; signed zero, invalid inputs, exact on-edge classification, 5,000 deterministic half-integer inputs compared with the independent P03 bounded-integer oracle.
3. `src/precision_studio.cpp` — inspect stored coordinates (17 significant digits), naive numerical result, exact signed result, and the chosen path; write a fixture CSV and a mathematically grounded diagnostic SVG; interactively enter six finite coordinates and save the corresponding report.

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
- `orient_binary64` must report exact sign \(-1\) using the dyadic limb path.
- Another fixture uses the smallest positive subnormal \(\eta=2^{-1074}\). Its exact area-sign is positive, but the naive \(\eta^2\) product underflows.
- A third fixture demonstrates positive exact sign despite overflowing naive product. A fourth shows **input rounding**: \(N+1\) was not preserved in binary64, so no exact predicate can reconstruct it.

The `repl` accepts `triangle ax ay bx by cx cy`, `inspect`, `cases`, `csv PATH`, `svg PATH`, `reset` and `quit`. Every report is computed from the same C++23 exact predicate. SVG panels are **qualitative computed diagnoses**. No benchmark or statistical speedups are invented.

### Verification plan

The CMake targets are `book03_precision` and `book03_exact_tests`; the CTest name is `book03_binary64_exact_orientation_contracts`, registered beside earlier four tests. The GitHub workflow is authored to compile all five projects, execute CTest, inspect exact and naive fixture values, parse the generated CSV and SVG, and exercise the numerical REPL.

**Do not call a test passing until observed compiler and test logs confirm it.** Cross-compiler or performance claims require separate execution. All source outputs are local build artifacts, not Book 03 EPUB/PDF releases. Leave the `From Bits to Research/` publication library, compiled LaTeX staging, original books and continuous notes untouched.


---

## Chapter 06 — P06 Three-Dimensional Geometry and Spatial Studio

**One cumulative C++23 project, six chapters.** We have moved from Chapter 01's bounded pixels to Chapter 02's frames, Chapter 03's triangles, Chapter 04's affine transforms, Chapter 05's represented-binary64 exact 2D orientation, and now a distinct 3D geometry layer. The old modules and their independent tests are preserved. This laboratory is pure C++23; no graphics API, GUI, Python runtime, linear-algebra dependency or network access is needed.

**Read in this order:**

1. `include/book03/geometry3d.hpp` — true right-handed three-dimensional `Point3` versus `Vector3`, dot/cross, stable three-argument `std::hypot`, unit directions, `Triangle3` winding/normal, normalized planes with signed distances, `Ray3` with `t` measured in metres, `Sphere3`, and explicit ray-plane/ray-sphere result statuses. Read all numeric-policy comments.
2. `tests/geometry3d_tests.cpp` — typed point/vector compile-time restrictions; orthogonality/winding; old P03 exact 2D oriented area embedded into the 3D XY plane; unit-normal distances; analytic and independently substituted ray-plane hit; parallel/coplanar/behind/uncertain outcomes; sphere entry/exit/tangency and near-tangency; zero, nonfinite and overflow rejection; 2,500 deterministic cross/dot property inputs.
3. `src/spatial_studio.cpp` — numeric inspector, original 3D house/sphere wireframe SVG, CSV of actual primitive query results, and a terminal REPL for reader-specified rays. The same C++23 geometry objects are used for the numeric report, SVG and CSV; no independent script duplicates hit math.
4. `CMakeLists.txt` — `book03_space` executable, `book03_space_tests` and the sixth registered CTest `book03_3d_geometry_ray_plane_sphere_contracts`.

**Commands:**

~~~bash
cmake -S "Code Parts/tools/labs/book03_cpp23" -B build/book03-p01 -DCMAKE_BUILD_TYPE=Release
cmake --build build/book03-p01 --parallel 4
ctest --test-dir build/book03-p01 --output-on-failure
build/book03-p01/book03_space inspect
build/book03-p01/book03_space svg build/book03-p01/house-3d.svg
build/book03-p01/book03_space csv build/book03-p01/ground-and-sphere.csv
build/book03-p01/book03_space repl
~~~

The terminal accepts `ray ox oy oz dx dy dz`, `inspect`, `svg FILE`, `csv FILE`, `reset`, `help` and `quit`. Example:

~~~text
ray 1 3 2 0 -1 0
inspect
svg your-ray.svg
csv your-ray.csv
quit
~~~

For the above ray, the infinite floor plane \(Y=0\) is met at \(t=3\) metres with point \((1,0,2)\). The direction entered by the reader is normalized in `Ray3::from`; the reported \(t\) measures physical distance in metres. The sphere is a separate independent query, not an arbitrary polygon/collider system.

**Diagram convention:** world \((X,Y,Z)\) is right-handed, with \(+Y\) up; the page mapping is `u=430+52*x-30*z`, `v=370+20*x-55*y+18*z` with coefficients in pixels per metre. This is a parallel **oblique illustration**, not a calibrated camera or a real perspective projection. The generated SVG contains a world grid, original house geometry, a triangulated roof normal, sphere wireframe rings and markers for actual hit points when present.

**Numerical limitations that must remain explicit:** the unit-normal/normalized-ray factories improve contracts but do not certify arbitrary huge/small construction results. Ray-plane near-parallel and ray-sphere near-tangent decisions use *documented heuristic thresholds* and expose uncertainty; they are not proved exact 3D predicates. The previous Chapter 05 exact sign applies to two-dimensional triples only. A plane hit is not yet a bounded triangle hit, a ray-sphere hit is not a mesh/scene query, and no unimplemented visibility or perspective system should be inferred from a wireframe picture.

**Validation contract:** execute the six CTests on a supported C++23 compiler, compare explicit numeric witnesses with independently derived values, parse the generated SVG, read the CSV rows and inspect visual placement. The dedicated GitHub CI source defines these checks, but successful execution must be observed before reporting a pass. The final Book 03 PDF/EPUB compilation remains reserved for the author's full-book Mac review, with the publication directory and old books unchanged.


---

## Chapter 07 — P07 Indexed Mesh Topology and Half-Edge Studio

The little 3D house now has actual **indexed mesh identity**. Source geometry and topology are separate; coincident XYZ coordinates do not imply shared vertex indices. The P07 lab is cumulative with `Point3`/`Triangle3` from Chapter 06.

**Suggested reading order:**

1. `include/book03/mesh_topology.hpp`: `Mesh3` uses `positions` and `TriangleIndices`; an optional per-face-corner UV array demonstrates how two face corners can refer to one position but use different texture coordinates. Read `EdgeKey`, `EdgeUse`, and `MeshReport` before reading `inspect_mesh()`. It classifies boundary edges, consistently opposite shared uses, winding conflicts, and edges with three or more incident faces. It tests local vertex fans for bow ties, reports exact equal-valued duplicate-position groups **without welding**, and counts face components using shared indexed edges.
2. `tests/mesh_topology_tests.cpp`: independent known quad and house oracles; flipped winding, separate coincident seam vertices, a three-face edge, bow-tie vertex, repeated-index versus geometric-zero-area faces, invalid index/UV/coordinate rejection, and **300 deterministic indexed strips** checked against independently derived counts \(V=2(N+1)\), \(E=4N+1\), \(F=2N\).
3. `include/book03/halfedge_mesh.hpp`: an additional traversable representation of each eligible face's three directed uses, with `next`, `previous`, and optionally `twin`. The convention is explicit: boundary, winding-conflict, and >2-face edges have *no fabricated twin*. The validator checks `next^3` cycles, inverse next/previous links, endpoint reversal and symmetric twins. Handles are rebuild-local, not stable generational IDs.
4. `tests/halfedge_mesh_tests.cpp`: the independent eighth CTest verifies face-cycle and twin contracts, that every edge of the closed house has a twin, that defective edges and vertex-only contact never receive spurious twins, and that deliberately broken links are detected.
5. `src/mesh_studio.cpp`: CLI inspector, SVG/CSV and interactive fixture selector. The diagram maps real indexed 3D coordinates through an explicitly **oblique parallel illustration**, with edge color for each incidence class. A screenshot alone cannot reveal a coincident-but-disconnected seam; use the report and CSV.

**CLI and reader interaction:**

~~~text
book03_mesh inspect house
book03_mesh inspect quad
book03_mesh inspect seam
book03_mesh inspect flipped
book03_mesh inspect nonmanifold
book03_mesh inspect bowtie
book03_mesh svg house-topology.svg house
book03_mesh csv house-edges.csv house
book03_mesh repl
~~~

In the `repl`:

~~~text
fixture seam
inspect
svg invisible-seam.svg
csv seam-edges.csv
fixture nonmanifold
inspect
quit
~~~

Fixtures: `house`, `quad`, `flipped`, `seam`, `nonmanifold`, `bowtie`, `degenerate` and `collinear`. The saved results always reflect the current selected fixture; no stale default drawing is substituted.

The closed reference house has **10 positions, 24 undirected edges and 16 triangular faces** with two opposite incident uses per edge; its combinatorial Euler characteristic is \(10-24+16=2\). Its closedness and orientation status is a set of *necessary local checks*, not a certified self-intersection-free embedding or physical watertightness proof. The quad is a disk with \((V,E,F)=(4,5,2)\), Euler one and four boundary edges. The seam fixture stores seven positions, references six, has six boundary edges and two edge-connected face components. A single vertex-only bow tie has a disconnected fan without a three-face edge.

**Safety / scope:** `inspect_mesh` uses exact discrete index and direction comparisons, but ordinary floating-point cross products to report geometric zero-area faces. It is **not** an exact 3D degeneracy predicate and does not automatically weld identical positions or repair meshes. The twin builder deliberately avoids nonmanifold or same-direction pairings. The half-edge constructor is immutable/rebuild-based and does not implement surface edits, generation-safe handles, boundary loop extraction, self-intersection certification or an accelerated renderer. These remain separate problems.

**Quality gates:** the CMake project registers the seventh laboratory, `book03_mesh` and two P07 test executables: `book03_mesh_tests` and `book03_halfedge_tests`. The cumulative CTest count is now **eight** (P01–P06 one each, P07 two). The dedicated CI source also checks the reference house count and incidence, the seam's separate components and equal positions, bow-tie fan, three-face-edge classification, and source-generated SVG and CSV outputs. Tests must be actually executed before claiming a pass. None of these commands writes to the author's final Book 03 PDF/EPUB publication library.


---

## Chapter 08 — P08 Curve, Spline and Bicubic Patch Workshop

**Purpose:** Make smooth mathematical geometry *genuinely editable* while preserving the original native C++23 project and the indexed topology contracts from P07. The original controller is a terminal REPL, not a fake interactive screenshot. It can change both curve and patch control points and regenerate all source-derived output files.

**Source reading order:**

1. `include/book03/curves3d.hpp`. Start at `curve_lerp()` and `CubicBezier3::at()`. Follow de Casteljau's three levels, then compare `bernstein_at()` as a separately implemented analytic evaluation. Continue through the exact algebraic split, first and second parameter derivatives, direct Hermite basis, nonrational uniform cubic B-spline span (and its conversion to a cubic Bézier), \(C^0/G^1/C^1/C^2\) join criteria, finite-chord control-hull based adaptive tessellation and its explicit exhausted-depth status.
2. `BicubicBezierPatch3` in the same header. Sixteen `Point3` controls have row index \(u\) and column index \(v\). Evaluate four cubic curves along \(u\), then one along \(v\), and differentiate each direction independently. A unit normal follows from \(S_u \times S_v\) if the cross product is nonzero. `patch_to_indexed_mesh()` makes an explicit rectangular parameter grid with one vertex ID per sampled location and shared edge indices inside the grid; it does **not** stitch independently generated patches.
3. `tests/curves3d_tests.cpp`. The ninth independent CTest verifies hard numerical witnesses, de Casteljau against Bernstein at 2,001 samples, exact reparameterization identities under subdivision, Hermite and Bézier conversion, uniform cubic B-spline span basis and \(C^2\) joins, \(G^1\)-not-\(C^1\) and \(C^1\)-not-\(C^2\) examples, a curve whose endpoints coincide but whose interior does not, explicit depth-budget exhaustion, and independent P07 indexed patch-grid invariants.
4. `src/curve_studio.cpp`. Source-generated SVG depicts the actual evaluated curve, editable handles, and patch sampling. CSV exports actual adaptive \((t,x,y,z)\) samples. OBJ exports only indexed vertices and triangle faces, using 1-based IDs; no materials, trim, UVs, normals or camera are claimed. All outputs are determined by the same in-memory controls and geometry math.

**Reader commands (after the cumulative CMake build):**

~~~bash
build/book03-p01/book03_curves inspect
build/book03-p01/book03_curves svg build/book03-p01/garden-curves.svg
build/book03-p01/book03_curves csv build/book03-p01/garden-samples.csv
build/book03-p01/book03_curves obj build/book03-p01/garden-patch.obj
build/book03-p01/book03_curves repl
~~~

The REPL understands:

~~~text
control 1 -2 3 0
control 2 2 3 0
tolerance 0.015
inspect
svg taller-arch.svg
csv taller-arch.csv
patch-control 1 1 -0.666667 4.0 -0.666667
obj adjusted-canopy.obj
reset
quit
~~~

**Mathematically checkable fixtures:** The code's default curve is a four-metre-wide garden arch with controls \((-2,0,0),(-2,2,0),(2,2,0),(2,0,0)\). At \(t=0.5\), its point is \((0,1.5,0)\), the derivative is \((6,0,0)\), the second derivative \((0,-12,0)\). Chapter 08's first hand derivation uses the **two-metre-wide** arch \((0,0),(0,2),(2,2),(2,0)\), whose midpoint derivative is **\((3,0)\)**. These are deliberately different widths, not a contradiction. Changing studio control \(P_1\) to height 3 while leaving other controls alone increases the default curve's midpoint Y to **1.875**.

The default patch has \(12\times12\) parameter cells, **169 shared indexed positions**, **288 triangles**, **456 unique undirected indexed edges**, **48 boundary edges**, Euler characteristic one. For a generic \(m\times n\) grid:

\[
V=(m+1)(n+1),\quad F=2mn,\quad E=3mn+m+n,\quad B=2(m+n).
\]

The patch topology is inspected using the P07 `inspect_mesh()` module as an independent connectedness, winding, boundary and Euler check. This protects against a patch implementation that merely draws visually overlapping disconnected triangle soup.

**Numerical contract:** `t,u,v` must be finite in \([0,1]\). Curve/patch controls must be finite. Hermite endpoint tangents are derivatives with respect to normalized parameter, not automatically metres per second. The B-spline supports a **single nonrational uniform cubic span**, not general NURBS; knot vectors, weights and trimming are deliberately documented but not implemented.

Adaptive recursion measures each cubic's interior controls against the *finite chord segment* and subdivides until the stated criterion is met or the depth budget is exhausted. Its exact-math control-hull distance argument is conservative, but a `double` implementation is **not** a proven floating upper interval bound. The returned `tolerance_met_under_float_policy` and terminal bound are diagnostic, not a certified Hausdorff metric. Zero-length endpoint chord does **not** erase a loop.

**Validation:** CMake includes eight cumulative labs and **nine CTests**. The dedicated Book 03 CI source is authored to compile and run the ninth test, inspect the current editable REPL control, parse generated SVG/CSV, and verify every OBJ face references 1..169. It also checks default patch source counts and a known modified Bézier midpoint. These sources **have not been reported as executed successfully** until independent compiler/CTest/CI logs confirm them.

**Publication:** No chapter's PNG/PDF/EPUB is promoted by this project. Original Book 01 and Book 02 distribution assets, continuous notes, derived chapter editions and the final `From Bits to Research/` publication routes remain unchanged.


---

## Chapter 09 — P09 Procedural World Workshop

This lab turns mathematical rules into **real indexed 3D triangle meshes** using only C++23 and the earlier P06/P07 geometry and topology modules. It is additive to the existing P01–P08 labs; it does not overwrite any original project or production publication.

**Reading and execution order:**

1. `include/book03/procedural_geometry.hpp`: begin with `TerrainField`, its analytic `height(x,z)` and upward `normal(x,z)`; continue with `make_heightfield()` and its explicitly reversed winding in the world XZ ground plane. Next follow the `make_capped_cylinder()` angular modulo wrapping and shared-cap indices, then `make_uv_sphere()` with exactly one north/south pole index.
2. Read `SphereDistance`: the analytic SDF is \(\|\mathbf p\|-r\), with negative interior, positive exterior and a radial gradient undefined at the origin. Compare `central_gradient()` to the analytic gradient under an explicit step size rather than believing finite differences are exact.
3. Read `extract_sphere_isosurface()`: sample a bounded Cartesian field grid, divide each cube into **six fixed tetrahedra**, interpolate strict sign changes by \(t=f_A/(f_A-f_B)\), and cache the intersection ID by canonical original **lattice edge**. One inside/three outside or three inside/one outside gives one triangle; two inside/two outside gives two triangles. Orient each output triangle radially outward as a sphere-specific contract. The function rejects exact-zero sampled grid vertices instead of silently allowing repeated/degenerate output.
4. `tests/procedural_geometry_tests.cpp`: these independent oracles derive terrain \(V=(m+1)(n+1)\), \(E=3mn+m+n\), \(F=2mn\), \(B=2(m+n)\), \(\chi=1\); capped cylinder \(V=2s+2\), \(E=6s\), \(F=4s\), \(\chi=2\); UV sphere \(V=2+s(t-1)\), \(F=2s(t-1)\), \(E=3s(t-1)\), \(\chi=2\); and the closed sphere candidate's edge incidence. They also check radial orientation and wrong-input rejection. A controlled implicit-sphere comparison checks grid-edge vertex reuse and sphere topology, not CAD accuracy.
5. `src/procedural_studio.cpp`: a terminal REPL for shape, resolution, radius and terrain amplitude; SVG from actual indexed faces and edge keys; OBJ from real indexed positions and faces; CSV of actual source-generated positions. The visualization uses a static parallel oblique map and does **not** claim a calibrated camera, hidden-surface removal or measured GPU output.

**Commands after cumulative CMake build:**

~~~bash
build/book03-p01/book03_world inspect terrain
build/book03-p01/book03_world inspect cylinder
build/book03-p01/book03_world inspect sphere
build/book03-p01/book03_world inspect implicit
build/book03-p01/book03_world svg terrain.svg terrain
build/book03-p01/book03_world obj capped-cylinder.obj cylinder
build/book03-p01/book03_world csv uv-sphere-vertices.csv sphere
build/book03-p01/book03_world repl
~~~

REPL:

~~~text
shape terrain
resolution 12
amplitude 0.65
inspect
svg wavy-ground.svg
shape cylinder
radius 0.8
obj small-tower.obj
shape sphere
resolution 12
inspect
shape implicit
resolution 9
inspect
obj extracted-sphere.obj
reset
quit
~~~

The default studio uses 12 subdivisions, sphere/cylinder radius **0.91 m**, terrain amplitude **0.45 m**, and a square \(4\times4\) m ground domain. For implicit extraction, the cubic lattice half-extent is **1.5 m** and the radius must fit strictly inside it. The controlled nine-cell and thirteen-cell independent oracle tests instead use the same radius **0.91 m** and a 1.5 m half-extent.

**Quick count ledger for default meshes:**

| Shape | Indexed V | Indexed E | Faces F | Boundary edges | Euler | Expected |
|---|---:|---:|---:|---:|---:|---|
| Heightfield 12×12 | 169 | 456 | 288 | 48 | 1 | One connected open disk |
| Capped cylinder, 12 slices | 26 | 72 | 48 | 0 | 2 | One closed genus-zero candidate |
| UV sphere, 12 slices × 6 stacks | 62 | 180 | 120 | 0 | 2 | One closed genus-zero candidate |
| Sphere SDF marching tetra, grid 12³ | Resolution-dependent; verify live | Resolution-dependent | Resolution-dependent | Expected 0 | Expected 2 | One closed sphere-like component under strict sample/tie assumptions |

**Important numeric limitations:** `make_heightfield` is a graph and cannot represent an overhang. The cylinder's geometric cap rims need separate per-corner shading normals in a renderer, even though geometric ring indices are intentionally shared. Longitude is periodic; the UV sphere stores one pole vertex each rather than a degenerate pole ring. The implicit extraction is **only for a centered sphere** in its present radial orientation policy; no exact-zero lattice ties, generalized SDF/occupancy extraction, general CAD trimming, topological repair, self-intersection certification or geometric error bound is claimed.

**CI and release:** The cumulative CMake adds the **ninth native C++23 lab** and the **tenth CTest**, `book03_procedural_heightfield_sphere_cylinder_isosurface_contracts`. The dedicated Book03 workflow is authored to check source editorial location, compile all labs, execute CTest, then inspect numeric reports and generated SVG/CSV/OBJ for all four shapes and live REPL edits. An authored check is **not** an observed passing build. This project does not write to the final EPUB/PDF library or regenerate any legacy manuscript.


---

## Chapter 10 — P10 Native OBJ Asset Trust Workshop

**Goal:** Understand exactly what “loaded successfully” does and does not establish. Convert a narrow but real Wavefront OBJ subset into (a) the *logical* indexed geometric mesh needed by P07 topology, and (b) the separately indexed rendering-vertex tuple stream. Validate a small glTF 2.0 byte-layout subset using overflow-aware arithmetic, without pretending to parse a GLB/JSON scene.

**Read in this order:**

1. `include/book03/asset_import.hpp`. Start at `ObjInputError`, `ObjImportLimits`, `obj_bounded_line` and `obj_decimal`: source lines are limited as they are read, numeric tokens must be fully consumed and finite, and the importer refuses unsupported directives. `obj_resolve_index` handles positive one-based OBJ references and negative references to each currently defined stream separately. `obj_corner` decodes `p`, `p/t`, `p//n` and `p/t/n` without inventing absent attributes.
2. Continue through `obj_check_convex_quad`: only planar **strictly convex** four-corner polygons are fan-triangulated; general concave and N-gons are explicitly not claimed. `import_obj` preserves all accepted face corners with their original attribute IDs, while `ObjAsset::topological_mesh` shares *position IDs* and `ObjAsset::to_rendering_mesh` creates one GPU vertex per distinct `(position,UV,normal)` tuple.
3. Read `inspect_obj_asset`, which reuses P07 `inspect_mesh` to distinguish a grammatical face from a valid connected nondegenerate surface, and separately records bounds, ignored metadata warnings and zero supplied normal vectors.
4. Read `GltfBufferView`, `GltfAccessor`, and `check_gltf_layout`. The authoritative glTF 2.0 bufferView/accessor math is `bufferView.byteOffset + accessor.byteOffset + i*effectiveStride`. The bounded implementation computes element size, validates ranges/alignment and checks count by division *before* a possibly overflowing multiplication. It only supports a documented component/vector subset and **does not** parse glTF JSON, GLB, scene hierarchy, sparse accessors or actual bytes.
5. Read `tests/asset_import_tests.cpp`. Its independent tests construct a square that has four geometric position IDs but six unified GPU tuples, negative indices resolved at face time and against independent streams, correct `p//n`, convex quad triangulation, refusal of concave/self-intersecting/warped quads and general N-gons, line and face budgets, malformed tokens, legal-but-broken faces, misleading zero normals and glTF offset/stride/count overflow adversaries.
6. Read `src/asset_studio.cpp`. It loads a real, size-bounded OBJ from a local path, prints source and P07 topology diagnostics, renders actual triangle/edge evidence in an illustrative SVG and exports the real rendering tuple stream as CSV. Its REPL replaces the active asset only after a candidate file parses successfully. The fixed SVG map is an oblique diagnostic diagram, **not** a calibrated camera or a robust visibility renderer.

**Commands after building the cumulative CMake project:**

~~~bash
build/book03-p01/book03_assets demo seam
build/book03-p01/book03_assets demo quad
build/book03-p01/book03_assets demo broken
build/book03-p01/book03_assets demo repeated

build/book03-p01/book03_assets inspect model.obj
build/book03-p01/book03_assets svg model.obj model-audit.svg
build/book03-p01/book03_assets gpu model.obj model-gpu-tuples.csv
build/book03-p01/book03_assets layout
build/book03-p01/book03_assets repl
~~~

REPL:

~~~text
demo broken
inspect
demo seam
inspect
svg good-seam.svg
gpu seam-gpu.csv
file model.obj
inspect
layout
quit
~~~

**Small checkable data:**

For two square faces `f 1/1/1 2/2/1 3/3/1` and `f 1/4/2 3/5/2 4/6/2`, the geometry has **4 positions, 5 indexed edges, 2 faces, Euler 1, 4 boundary edges, 1 consistent interior edge**, while its rendering stream has **6 distinct per-corner tuples**. That is intentional, not a corrupted import.

An OBJ `f -3/-3/-1 -2/-2/-1 -1/-1/-1` is resolved against position, UV and normal stream counts **at the instant the face is read**, with all resulting indices zero-based. A later declaration does not change the already resolved face.

The restricted glTF layout fixture uses a 256-byte buffer, view start 16 and length 128, stride 24, accessor relative offset 8, five `VEC3 FLOAT` values with packed size 12. The global element starts are **24,48,72,96,120** and the exclusive end is **132** inside the view's global end at **144**. A large invalid count must fail through the `(remaining-element_size)/stride` inequality before multiplying.

**Limits:** The OBJ parser is an educational bounded subset: ASCII `v`, `vt`, `vn`, `f` triangles and strictly convex planar quads, with recognized metadata (`o,g,s,usemtl,mtllib`) recorded but **ignored**. No external MTL resources are followed. General N-gons, colors, homogeneous coordinates, scene nodes, animations, skinning, materials, normal/tangent regeneration, geometric welding and topological repair are not claimed. The file size limit of the terminal tool is 5 MiB; the parser also caps lines, line characters, positions and face counts. The glTF checker is an overflow-safe **layout kernel**, not a full format loader.

**Testing:** CMake now declares **ten cumulative C++23 labs and eleven CTests**, including `book03_obj_corner_domains_and_gltf_bounds_contracts`. The Book03 GitHub Actions source runs native compilation and CTest, then creates a real local seam OBJ fixture, checks four-versus-six vertex identity, parses source SVG and GPU CSV exports, verifies the separate attribute indices, exercises the REPL and checks the known glTF byte witness. **Do not report these tests as green before an actual CMake/CTest/GitHub Actions execution is observed.**

**Publication:** Final PDF/EPUB compilation is deferred until all 32 Book03 chapters and original-source reconciliations are finished. The original continuous notes, derived chapter editions, legacy LaTeX stage and already published Books 01–02 remain read-only.


---

## P11 — Shared-asset Scene Forest

Read `include/book03/scene_forest.hpp` in order: Linear3 column maps, SceneAffine, scene_translate/scale/rotate_y, SceneNode/SceneEvaluation, SceneForest::evaluate and reparent, five_tree_world. Then read `tests/scene_forest_tests.cpp` and `src/scene_studio.cpp`. One seven-position, three-face local tree mesh is referenced by five nodes. Kahn's iterative ordering evaluates parents first, even if array order is reversed. Nodes are distinct from assets. Reparenting with world preservation solves L_new = inverse(W_parent) W_child and keeps full affine shear. Normals use the inverse transpose; negative determinant reverses winding parity. Inverse rejection is heuristic, not certified. The bounds scan transforms all source positions.

After the eventual cumulative native build, the reader can run:

~~~text
book03_scene inspect
book03_scene svg five-trees.svg
book03_scene csv five-trees.csv
book03_scene repl
~~~

REPL commands: `move 0 2 0 0`, `parent 3 1 world`, `parent 0 2 local` (cycle rejected), `inspect`, `svg FILE`, `csv FILE`, `quit`. The SVG is an oblique diagnostic projection, without a camera, visibility or material implementation. The CSV records origins, bounds and determinant. The independent twelfth CTest exercises numeric witnesses, shared identity, parent order, 5000-node chain, singular inverse and failed-edit rollback. No native test pass is asserted yet.


---

## P12 — Orientation Debugger

Read `include/book03/rotation3.hpp` from Rotation3 normalization through axis_angle, Hamilton multiplication, vector application, fixed_xyz and SLERP, then the independent Rodrigues formula, principal Euler extraction and RigidPose. The convention is right-handed active column-vector rotation, radians, quaternion order wxyz; the rightmost product acts first. Compare the independent `tests/rotation3_tests.cpp` and inspect `src/rotation_studio.cpp` for degrees-to-radians conversion and actual output generation.

After the eventual cumulative CMake build:

~~~text
book03_rotation inspect
book03_rotation svg orientations.svg
book03_rotation csv orientations.csv
book03_rotation repl
~~~

REPL: `axis 0 0 1 120`, `sample 0.5`, `euler 30 90 20`, `inspect`, `start`, `axis 1 1 1 120`, `csv FILE`, `svg FILE`, `quit`. The default 61 samples follow a 120-degree Z rotation. The near-lock Euler flag is diagnostic; the quaternion stays authoritative. The near-coincidence branch uses NLERP, not exact constant angular speed. The thirteenth authored CTest checks analytic rotations, 3,000 seeded cases and 10,000 accumulated updates. No successful native test run is asserted yet. No PDF or EPUB is generated in this pass.


---

## P13 — Calibrated Pinhole Camera Simulator

Read `include/book03/pinhole_camera.hpp` in this order: CameraIntrinsics and centered_fov, crop and resize; PinholeCamera optical, project, optical_direction, ray, pixel_ray, unproject_axial and optical_jacobian. The rigid pose is camera-to-world, graphics local -Z is forward, optical +Z is forward and +Y is down. Continuous coordinates use the top-left image-edge convention, integer pixel samples use half-integer centers. Axial depth is not unit-ray range. Read the independent `tests/pinhole_camera_tests.cpp` then the actual `src/pinhole_studio.cpp` exporter.

After the eventual CMake build:

~~~text
book03_camera inspect
book03_camera svg landmarks.svg
book03_camera csv landmarks.csv
book03_camera repl
~~~

REPL: `focal 800 800`, `move 0 0 2`, `yaw 15`, `principal 300 220`, `ray 319 239`, `inspect`, `csv FILE`, `svg FILE`, `reset`, `quit`. The default 640x480 model has focal scales 400 px, principal point (320,240) and seven world landmarks. The outputs are point projection diagnostics, not rasterized triangles or camera visibility. The fourteenth CTest contains 3,000 seeded optical Jacobian and round-trip cases. No successful native run is asserted yet. No PDF/EPUB is generated.


---

## P14 — Projection and Depth Debugger

Read `include/book03/projection_pipeline.hpp`: Homogeneous4, ProjectionMatrix, DepthMap, perspective_matrix, orthographic_matrix, classify_clip, perspective_divide, Viewport and trace_projection. The convention is column vectors, graphics local -Z forward, zero-to-one NDC depth and top-left viewport. Storage is row-major; this is independent of mathematical multiplication. Compare `tests/projection_pipeline_tests.cpp` with `src/projection_studio.cpp`. The point classifier is not a primitive clipper.

After the eventual native CMake build:

~~~text
book03_projection inspect
book03_projection csv depth-storage.csv
book03_projection svg depth-curves.svg
book03_projection repl
~~~

REPL commands: `point 1 1 -4`, `reverse`, `forward`, `depth 0.1 0` (infinite), `depth 1 9`, `ortho`, `perspective`, `inspect`, `csv FILE`, `svg FILE`, `quit`. The CSV and SVG always use the fixed infinite-far near-0.1 reference experiment, not the current REPL policy. CSV contains 964 data rows for IEEE binary32 and uniform 24-bit storage in both depth orders. It isolates storage quantization, not actual GPU errors. No observed native build/CTest success is asserted.


---

## P15 — Stereo Reconstruction Laboratory

Read `include/book03/stereo_geometry.hpp`: RectifiedStereo, corrected disparity and row residual, axial depth, depth_sigma, triangulate_rays, triangulate_samples, epipolar_coplanarity. Then read `tests/stereo_geometry_tests.cpp` and `src/stereo_studio.cpp`. The aligned rig uses equal focal scales, zero skew, possibly different principal points, and a positive metric baseline along left camera-local X. General reconstruction uses actual P13 rays and arbitrary valid poses. A midpoint is a metric geometric estimate, not a pixel-space optimizer.

After the eventual native build:

~~~text
book03_stereo inspect
book03_stereo csv stereo-depth.csv
book03_stereo svg two-house-views.svg
book03_stereo repl
~~~

REPL: `noise -20`, `inspect`, `noise 0`, `baseline 0.05`, `point 1 1 -16`, `csv FILE`, `svg FILE`, `quit`. Default focal scale 400 px and baseline 0.2 m reconstruct the generating point (1,1,-4). The deliberately wrong horizontal match reconstructs (0.5,0.5,-2), illustrating the insufficiency of reprojection consistency. CSV contains 101 synthetic depth samples with sigma disparity 0.25 pixels and mapped interval endpoints. SVG draws actual synthetic corresponding house points. No automatic image matching, calibration or native test pass is asserted.


---

## P16 — Indexed Vertex Pipeline Tracer

Read `include/book03/vertex_pipeline.hpp` after asset_import.hpp, scene_forest.hpp, rotation3.hpp and projection_pipeline.hpp. Follow VertexStage::process and draw, then screen_barycentrics and interpolate_inside_triangle. Draw-scoped reuse is by full rendering ID. The raw clip coordinate and payload remain available even if a screen position does not exist. Smooth UV/world position use normalized reciprocal-w weights; divided NDC depth uses screen weights. Interpolated normals are renormalized. This is not a clipper or coverage rasterizer.

After the eventual native build:

~~~text
book03_vertex inspect
book03_vertex csv varying-edge.csv
book03_vertex svg varying-edge.svg
~~~

The fixed OBJ witness has w=(2,4,8), screen centroid UV=(2/7,1/7), and forward depth=85/114. Along its second edge corrected V=t/(2-t), rather than t. The independent tests include seam and draw-state identities, stage transforms, rejection cases and 3,000 seeded original-surface round trips. Source registration is verified separately from native execution.


---

## P17 — Primitive Clipping Explorer

Read `include/book03/primitive_clipping.hpp` after vertex_pipeline.hpp. Start with Window2, outcodes, cohen_sutherland and liang_barsky, then ClipPlane, ClipVertex, clip_mix, clip_crossing, clip_triangle, to_screen_vertex and fan_triangulate. Homogeneous clipping retains raw smooth varyings and original triangle-corner weights. Generated normal vectors remain unnormalized. The six closed planes are followed by positive-w and full membership checks. Floating boundary failures are explicit; no certified watertightness is claimed.

After the eventual native build:

~~~text
book03_clip inspect
book03_clip csv clipped-polygon.csv
book03_clip svg clipped-polygon.svg
~~~

The fixed fixture produces a four-vertex polygon and two fan triangles. Tests include 4,000 seeded line comparisons, 2,000 single-plane triangle provenance checks and a behind-camera projection witness. The studio is not an interactive editor or filled rasterizer. Native execution remains unverified.
