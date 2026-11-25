# Book 03 — From Geometry to Images

**Subtitle:** 3D Geometry, Transformations, Cameras, Projection, and Image Formation  
**Series:** From Bits to Research  
**Active branch:** curriculum/book03-geometry-images-pass01  
**Author:** Swayam Singal  
**State:** Authoring passes 01–32; all thirty-two planned narrative chapters, thirty-two cumulative C++23 laboratories, thirty-four source-authored CTests and one hundred forty-two original SVG teaching figures are committed. All 32 chapter paths and numbered headings have been verified. Publication acceptance remains open. NO Book 03 PDF or EPUB has been generated or promoted.

> A computer can draw a triangle. This book is about discovering what that deceptively simple sentence requires us to know, and then doing it ourselves.

## First read

1. [The editorial and source-preservation contract](EDITORIAL_CONTRACT.md) — our rules for the manuscript, narrative and release layout.
2. [Thirty-two-chapter reading and project plan](CHAPTER_PLAN.md) — our roadmap, not a false claim of completion.
3. [Original-source routing ledger](SOURCE_LEDGER.md) — where the substantial pre-existing graphics/camera material lives and how we will preserve it.
4. [Chapter 01 — The Picture Made of Little Windows](Chapters/01%20-%20The%20Picture%20Made%20of%20Little%20Windows.md) — opening full-length, reader-facing chapter draft.
5. [Chapter 02 — Every Place Needs an Address](Chapters/02%20-%20Every%20Place%20Needs%20an%20Address.md) — full narrative on points, displacements, frames, inverse transforms and world-to-image mapping.
6. [Chapter 03 — Shapes That Know Their Borders](Chapters/03%20-%20Shapes%20That%20Know%20Their%20Borders.md) — signed area, winding, barycentrics, interpolation, exact bounded integer predicates and uncertain floating boundaries.
7. [Chapter 04 — Move the World Without Losing It](Chapters/04%20-%20Move%20the%20World%20Without%20Losing%20It.md) — affine and homogeneous transformations, active/passive frames, pivots, inverses, normals and conditioning.
8. [Chapter 05 — The Triangle That Almost Wasn't There](Chapters/05%20-%20The%20Triangle%20That%20Almost%20Wasn%27t%20There.md) — reproducible cancellation and underflow, exact represented-binary64 predicates, signed integer limbs and uncertainty.
9. [Chapter 06 — Step Out of the Page](Chapters/06%20-%20Step%20Out%20of%20the%20Page.md) — complete 3D world foundations, cross products, planes, normals, ray/plane/sphere intersections and conditioning.
10. [Chapter 07 — One Shape, Many Triangles](Chapters/07%20-%20One%20Shape%2C%20Many%20Triangles.md) — indexed mesh identity, edge incidence, orientation, halfedge twins, bow-tie fans, invisible seams, Euler and CSR/Laplacian.
11. [Chapter 08 — Drawing Smooth Things with Control Points](Chapters/08%20-%20Drawing%20Smooth%20Things%20with%20Control%20Points.md) — de Casteljau, Hermite, B-spline, blossoms, continuity, adaptive tessellation, bicubic patches and real indexed mesh output.
12. [Chapter 09 — Making a World from Rules](Chapters/09%20-%20Making%20a%20World%20from%20Rules.md) — real indexed terrain, capped cylinder, pole-safe sphere, signed distance and shared-edge marching tetrahedra.
13. [Chapter 10 — A Mesh You Can Trust](Chapters/10%20-%20A%20Mesh%20You%20Can%20Trust.md) — strict OBJ corner streams, GPU tuple identity, indexed topology diagnostics, normals, safe convex triangulation and glTF buffer-offset checks.
14. [Chapter 11 — The Same Tree in Five Worlds](Chapters/11%20-%20The%20Same%20Tree%20in%20Five%20Worlds.md) — shared assets, hierarchy, normal matrices, reparenting and bounds.
15. [Chapter 12 — Rotations Without Headaches](Chapters/12%20-%20Rotations%20Without%20Headaches.md) — proper rotations, Euler singularities, quaternions, SLERP and rigid poses.
16. [Chapter 13 — The Tiny Hole That Makes a Picture](Chapters/13%20-%20The%20Tiny%20Hole%20That%20Makes%20a%20Picture.md) — calibrated projection, camera frames, intrinsics, focal length, rays and Jacobian.
17. [Chapter 14 — From a 3D Point to a Screen Address](Chapters/14%20-%20From%20a%203D%20Point%20to%20a%20Screen%20Address.md) — calibrated homogeneous projection, clip membership, NDC, viewport and depth storage.
18. [Chapter 15 — Two Eyes Looking at One House](Chapters/15%20-%20Two%20Eyes%20Looking%20at%20One%20House.md) — disparity, epipolar geometry, calibrated triangulation and uncertainty.
19. [Chapter 16 — The Long Journey of One Vertex](Chapters/16%20-%20The%20Long%20Journey%20of%20One%20Vertex.md) — indexed vertex processing, raw clip payloads and perspective-correct interpolation.
20. [Chapter 17 — Shapes That Refuse to Leave the Window](Chapters/17%20-%20Shapes%20That%20Refuse%20to%20Leave%20the%20Window.md) — segment and homogeneous polygon clipping, raw payloads and provenance.
21. [Chapter 18 — Who Owns the Edge](Chapters/18%20-%20Who%20Owns%20the%20Edge.md) — exact snapped integer coverage, top-left ownership and incremental edge functions.
22. [Chapter 19 — Which Surface Gets Seen](Chapters/19%20-%20Which%20Surface%20Gets%20Seen.md) — opaque visibility, depth storage, reverse-Z and equal-depth policies.
23. [Chapter 20 — A Pixel Is Not a Tiny Square of Reality](Chapters/20%20-%20A%20Pixel%20Is%20Not%20a%20Tiny%20Square%20of%20Reality.md) — sampling, aliasing, reconstruction and antialiasing.
24. [Chapter 21 — A Whole Rasterizer We Can Explain](Chapters/21%20-%20A%20Whole%20Rasterizer%20We%20Can%20Explain.md) — native-tested raster backend and separately gated OBJ-to-image integration.
25. [Chapter 22 — A Lens Bends the Story](Chapters/22%20-%20A%20Lens%20Bends%20the%20Story.md) — thin-lens optics, aperture, focus tolerance and diffraction.
26. [Chapter 23 — Why Straight Lines Bow](Chapters/23%20-%20Why%20Straight%20Lines%20Bow.md) — distortion, Jacobians, injectivity and angular projection.
27. [P01–P23: C++23 Image, Frame, Triangle, Affine, Predicate, 3D, Mesh, Curve, Procedural, Asset Trust, Scene, Rotation, Camera, Projection, Stereo, Vertex, Clipping, Raster, Visibility, Sampling, Renderer, Lens and Distortion Studios](../../../../Code%20Parts/tools/labs/book03_cpp23/README.md) — numerical code, exact tests, terminal REPL and source-generated SVG diagrams.
28. [Pass 02 original-source audit](PASS02_LEGACY_AUDIT.md) — records the inspected camera/geometry foundation spans without editing the canonical manuscripts.
29. [Pass 02 evidence record](PASS02_EVIDENCE.md) — provenance, exact changes and what is not yet verified.
30. [Pass 03 source audit](PASS03_LEGACY_AUDIT.md) — original geometry/rasterization content preserved with exact source identities.
31. [Pass 03 evidence record](PASS03_EVIDENCE.md) — what the third round actually delivered and which acceptance gates remain.
32. [Pass 04 source audit](PASS04_LEGACY_AUDIT.md) — maps original basis, active/passive, camera and normal-source boundaries.
33. [Pass 04 evidence record](PASS04_EVIDENCE.md) — authored code, mathematical checks and honest validation limits.
34. [Pass 05 source-subsection audit](PASS05_LEGACY_AUDIT.md) — robust predicates plus original camera-frustum content routed to their proper chapters.
35. [Pass 05 evidence record](PASS05_EVIDENCE.md) — exact input-representation proof, tests, reporting and remaining acceptance gates.
36. [Pass 06 original-source audit](PASS06_LEGACY_AUDIT.md) — actual inspected 3D ray/mesh/camera source entrypoints, with subsequent chapters' responsibilities preserved.
37. [Pass 06 evidence record](PASS06_EVIDENCE.md) — new 3D math APIs, tests, figures, source custody and open validation gates.
38. [Pass 07 original-source audit](PASS07_LEGACY_AUDIT.md) — actually inspected halfedge, winged-edge, vertex-link, CSR and Laplacian source sections.
39. [Pass 07 evidence record](PASS07_EVIDENCE.md) — authored mesh and halfedge C++23 project, figures, exact combinatorial checks and outstanding runtime/release gates.
40. [Pass 08 original-source audit](PASS08_LEGACY_AUDIT.md) — actual inspected original MIT graphics source sections on cubic curves, splines, patches and later subdivision material.
41. [Pass 08 evidence record](PASS08_EVIDENCE.md) — new cubic/B-spline/Hermite math, studio, original illustrations and unobserved runtime gates.
42. [Pass 09 original-source audit](PASS09_LEGACY_AUDIT.md) — original parametric and implicit surface proofs, SDFs, learned-shape source atlas and preserved later material.
43. [Pass 09 evidence record](PASS09_EVIDENCE.md) — procedural mesh generators, numerical/topological checks, source exports, original figures and open execution gates.
44. [Pass 10 original-source audit](PASS10_LEGACY_AUDIT.md) — actual OBJ/glTF, per-corner shading, half-edge/topology and scene source paths, with implementation boundaries.
45. [Pass 10 evidence record](PASS10_EVIDENCE.md) — native OBJ subset/import studio, glTF bounds kernel, eleven CTests and open runtime acceptance gates.
46. [Pass 11 original-source audit](PASS11_LEGACY_AUDIT.md) — hierarchy, transform-chain and shading source custody.
47. [Pass 11 evidence record](PASS11_EVIDENCE.md) — scene forest, five instances, twelfth CTest and outstanding validation.
48. [Pass 12 original-source audit](PASS12_LEGACY_AUDIT.md) — quaternion, animation and rigid-pose source custody.
49. [Pass 12 evidence record](PASS12_EVIDENCE.md) — orientation debugger, thirteenth CTest and outstanding validation.
50. [Pass 13 original-source audit](PASS13_LEGACY_AUDIT.md) — pinhole, intrinsics, pose and projection sensitivity custody.
51. [Pass 13 evidence record](PASS13_EVIDENCE.md) — native camera simulator, fourteenth CTest and outstanding gates.
52. [Pass 14 original-source audit](PASS14_LEGACY_AUDIT.md) — projection, clipping and depth source custody.
53. [Pass 14 evidence record](PASS14_EVIDENCE.md) — scalar depth experiment, fifteenth CTest and acceptance boundaries.
54. [Pass 15 original-source audit](PASS15_LEGACY_AUDIT.md) — stereo, disparity, calibration and XR source custody.
55. [Pass 15 evidence record](PASS15_EVIDENCE.md) — reconstruction laboratory and sixteenth CTest.
56. [Pass 16 original-source audit](PASS16_LEGACY_AUDIT.md) — vertex and varying source custody.
57. [Pass 16 evidence record](PASS16_EVIDENCE.md) — vertex tracer, seventeenth CTest and P05 CSV correction.
58. [Pass 17 original-source audit](PASS17_LEGACY_AUDIT.md) — homogeneous planes and clipping source custody.
59. [Pass 17 evidence record](PASS17_EVIDENCE.md) — clipping explorer, eighteenth CTest and numerical limitations.
60. [Pass 18 original-source audit](PASS18_LEGACY_AUDIT.md) — raster sample ownership custody.
61. [Pass 18 evidence record](PASS18_EVIDENCE.md) — observed standalone GCC/UBSan pass and remaining gates.
62. [Pass 19 original-source audit](PASS19_LEGACY_AUDIT.md) — visibility and depth source custody.
63. [Pass 19 evidence record](PASS19_EVIDENCE.md) — observed standalone test, studio and four-mode output verification.
64. [Pass 20 original-source audit](PASS20_LEGACY_AUDIT.md) — sampling source custody.
65. [Pass 20 evidence record](PASS20_EVIDENCE.md) — verified native sampling trials and 960 measurements.
66. [Pass 21 original-source audit](PASS21_LEGACY_AUDIT.md) — renderer integration source custody.
67. [Pass 21 evidence record](PASS21_EVIDENCE.md) — native renderer gallery, pixel oracle and outstanding frontend gate.
68. [Pass 22 original-source audit](PASS22_LEGACY_AUDIT.md) — original optics custody.
69. [Pass 22 evidence record](PASS22_EVIDENCE.md) — native ray geometry, 3,000 configurations and 404 verified rows.
70. [Pass 23 original-source audit](PASS23_LEGACY_AUDIT.md) — distortion and angular projection source custody.
71. [Pass 23 evidence record](PASS23_EVIDENCE.md) — native Jacobian and inverse verification.

## What is already authored?

Pass 01 begins the new book with an original narrative on the distinction between a physical scene, a mathematical image and a stored pixel; discrete addressing; continuous sample locations; integer drawing; overflow-aware image buffers; PPM export; and a deliberately imperfect line rasterizer. The first C++23 project and accompanying tests are intended as a runnable contract, not a replacement for the full reader book.

Pass 02 adds a second full chapter on coordinate frames, point versus vector types, basis matrices and inverse maps, units, nested transform composition, a world-up/image-down coordinate bridge, exact examples and deliberate failures. Its P02 C++23 code extends the same companion with a typed affine frame model, independent numerical tests and a terminal-controlled SVG frame visualizer. Four original accessible figures now support Chapters 01–02.

Pass 03 adds a third long narrative chapter with two scientific diagrams, a bounded exact integer oracle, a deliberately noncertified double-precision barycentric query, and a C++23 triangle REPL that can visualize and save both fixed reference output and the reader's current edited triangle.

Pass 04 adds the fourth narrative chapter with original affine/transform-order/normal diagrams, a new homogeneous-affine 2D C++23 module, independent numerical tests including 2,000 deterministic transform round trips, and a house studio with inspectable matrices, SVG/PPM output, transform-order witness and editable terminal REPL.

Pass 05 drafts the full numerical-geometry chapter, with three original explanatory SVG figures and a self-contained C++23 exact orientation predicate for **finite stored IEEE binary64 coordinates**. The exact engine uses the previous bounded-int64 oracle for small integer inputs and a signed arbitrary-limb dyadic path otherwise, with 5,000 deterministic cross-checks specified in the new test source. The project adds a numeric observatory with an interactive REPL and reproducible SVG/CSV decision reports.

Pass 06 drafts the long three-dimensional transition chapter: points and displacements in space, dot and cross, true face normals, planes and signed distances, forward rays and their intersection policies, spheres and stable quadratic root selection. Four new original scientific figures continue the series visual style. A native `geometry3d.hpp` math reference, `spatial_studio.cpp` oblique 3D scene viewer (SVG/CSV/REPL) and independent `geometry3d_tests.cpp` suite introduce 2,500 deterministic vector identities and boundary cases. The P03 bounded exact 2D orientation serves as an independent source of evidence for a triangle embedded in the 3D XY plane, without falsely claiming that the new 3D cross product is exact.

Pass 07 adds the long indexed-mesh reader chapter, five original vector teaching plates, an exact integer-index topology report with eight deliberate valid/invalid fixtures, a disjoint-set component and vertex-fan inspector, equal-valued coordinate seam reporting, per-face-corner UV domains, a compact indexed halfedge face/twin validator, and CSR adjacency with a source-documented scalar uniform graph Laplacian. Its two independent C++23 test suites include 300 deterministic strip count oracles; the native mesh studio produces selectable-fixture SVG/CSV plus numerical inspection. The CMake project now contains seven cumulative labs and eight CTests. The original long mesh chapter, halfedge/winged-edge and boundary-loop proofs are mapped but not overwritten.

Pass 08 authors the complete garden-tree curve-and-patch reader chapter, including mathematically derived de Casteljau/Bernstein, Hermite, a nonrational uniform cubic B-spline span, blossom identities, Catmull–Rom and NURBS scope boundaries, G1/C1/C2 join examples, a finite-chord adaptive tessellation criterion, and a sixteen-control bicubic patch that becomes a proper indexed P07 mesh. Six original mathematical SVGs bring the total to 27. The native C++23 `curves3d.hpp` laboratory, independently compared evaluations, patch tests, and editable SVG/CSV/OBJ workbench add a ninth CTest across eight cumulative labs. The original source atlas and legacy cubic curve/surface chapter bodies are inspected and mapped without being overwritten. The author will perform the final book compile only after all 32 chapters and source reconciliation are complete.

Pass 09 authors the full procedural-world reader chapter, six new mathematical SVGs and a native C++23 generation lab. It implements a heightfield with analytic up normals, a capped cylinder with modulo angular winding, a sphere with unique pole vertices, and an implicit sphere from a bounded Cartesian field using fixed marching tetrahedra and globally shared cached intersection vertex IDs. Its C++23 tests independently validate terrain/disk and capped-cylinder/sphere topologies, SDF signs/gradients, unsupported grid ties and under-sampling, and the restricted sphere extraction's genus-zero closure. The native world REPL exports live source-derived SVG, indexed OBJ and CSV. Existing source-atlas and earlier full geometry/learned-shape manuscripts are mapped and remain untouched. The nine cumulative C++23 labs now declare ten CTests; native success has not yet been independently observed. **Figures total 33.**

Pass 10 creates a substantive new chapter about trusting imported geometry rather than trusting a parser-success flag. Six original scientific SVGs explain four validation layers, the four-topological/six-rendering-vertex UV/normal seam, convex-only quad triangulation, glTF two-offset and stride byte addressing, edge-winding audit failures, and deliberate before/after repair policies. The native C++23 source adds a bounded OBJ subset parser with independent face-corner position/UV/normal indices, safe one-based and negative resolution, full-tuple GPU vertex deduplication, P07 topology diagnostics, explicit unsupported-feature rejection, an overflow-aware glTF binary accessor layout checker, and a small interactive Asset Trust Studio that can inspect real OBJ files and export source-derived SVG/CSV evidence. The source-authored eleventh CTest includes adversarial input and counterexamples. Original long mesh topology and per-corner attribute manuscripts were directly inspected and mapped rather than modified. **Figures total 39; labs total 10; CTests authored total 11.**

Pass 11 adds the scene-forest reader, one shared tree asset and five placed instances, affine composition and inverse, world-preserving reparenting, iterative cycle checks, transformed bounds and inverse-transpose normal math. Five original diagrams bring the cumulative count to 44. The native scene studio exports real transformed geometry and numeric instance evidence. The twelfth CTest is authored, not yet observed passing.

Pass 12 adds SO(3), Rodrigues, Euler singularity, Hamilton quaternion algebra, shortest-arc SLERP and rigid SE(3) poses; the native orientation debugger exports quaternion and transformed basis samples. Five original figures bring the total to 49. The thirteenth CTest is authored but not yet observed passing.

Pass 13 adds the calibrated pinhole model, explicit optical frame, crop and resize calibration, inverse rays and axial depth, projection Jacobian and the C++23 landmark simulator. Five original figures bring the cumulative count to 54. The fourteenth CTest is authored but not yet observed passing.

Pass 14 adds calibrated perspective and orthographic matrices, homogeneous clip classification, viewport mapping and finite/infinite forward/reverse depth. The scalar storage-only experiment independently compares binary32 and uniform 24-bit rounding; the native debugger exports 964 samples. Six original figures bring the total to 60. Native compilation remains unobserved.

Pass 15 adds corrected stereo disparity, epipolar geometry, closest-ray triangulation and uncertainty, plus a wrong-correspondence counterexample. The native viewer exports two synthetic views and analytic uncertainty. Six new figures bring the total to 66. The sixteenth CTest is authored, not observed passing.

Pass 16 connects indexed asset tuples to model/view/projection and derives reciprocal-w smooth varying interpolation and screen-linear depth. Six figures bring the total to 72. A new tracer and seventeenth CTest are authored; an earlier P05 CSV case-label collision is corrected at source level. Native execution remains unobserved.

Pass 17 adds rectangular segment clipping, homogeneous triangle clipping, source-corner provenance and fan triangulation. Five new figures bring the total to 77. The eighteenth CTest is authored but native execution remains unobserved.

Pass 18 adds bounded integer sample ownership with five figures (82 cumulative). Its exact-hash-matched standalone coverage test passed natively under GCC 14.2.0 with UBSan. Integrated studio, cumulative CMake/CTest and publication remain unverified.

Pass 19 adds per-sample opaque visibility, depth representation and tie policies. Five figures bring the total to 87. Both standalone test and studio passed hash-verified GCC/UBSan execution, and all 3,072 CSV rows agreed with an independent oracle. Cumulative integration remains unverified.

Pass 20 adds sampling theory and a native-validated analytic quadrature lab. Five figures bring the total to 92. Standalone GCC/UBSan tests and studio passed with exact source hashes and 960 independently checked measurements. Full renderer integration remains pending.

Pass 21 adds a native-tested opaque screen-space backend and actual gallery image, plus the committed OBJ/model/view/clipping adapter. Three figures bring the total to 95. Two new CTests separate backend and front-end acceptance. The local DNS blocker prevented full dependency retrieval; only the backend and gallery were observed passing.

Pass 22 adds thin-lens optical geometry, focus tolerance and diffraction context. Five figures bring the total to 100. The exact-hash-matched native lens tests and CSV studio passed under GCC/UBSan; cumulative integration remains unverified.

Pass 23 adds normalized lens distortion, a conservative injectivity bound, damped Newton and five angular projection families. Five figures bring the count to 105. The exact committed standalone C++23 tests and studio passed GCC/UBSan with 4,000 seeded trials and 265 independently checked exports. Cumulative integration remains unverified.

The first twenty-three passes do **not** assert that all 32 chapters have been drafted, that the legacy source has been fully integrated, that the C++ tests have passed without observed results, or that a PDF or EPUB exists. Later passes will use the substantial legacy chapter material at the locations documented in SOURCE_LEDGER.md. The author explicitly plans to compile and visually review the finished book on macOS at the end of the book-development process.

## Teaching voice

Use one continuous, warm, lively explanatory voice. A curious child should be able to follow the *story* of the problem; an advanced reader should still be able to derive every equation, understand every program, and challenge its numerical assumptions. An analogy is an invitation, never a substitute for proof. Avoid empty jokes, filler, vague promises, and mechanical repetitions of “what we will learn.”

A typical chapter follows a tiny scene or question → a counterintuitive observation → careful definitions → diagram or exact worked example → derived mathematics → commented C++23 implementation → tests and failure cases → reader exercises → bridge to the next idea. Do not make every page a list. Put substantial meaning into paragraphs.

## Repository boundaries — nonnegotiable

The checked **current main** layout separates notes and code:

- Working Book 03 manuscript: Notes/Book Production/Seven-Book Curriculum/Book03/
- Authoritative existing manuscripts: Notes/Continuous Notes/
- Derived reading copies: Notes/Chapter Edition/ (do not hand-edit these)
- Existing seven-stage TeX generation: Notes/Book Production/AllMight Codex/LaTeX/
- Executable companion code: Code Parts/tools/labs/book03_cpp23/
- Actual, local publication library: From Bits to Research/EPUB/, From Bits to Research/PDF/, Archive/, Manifests/.

The publication directory remains untouched until explicit promotion of an approved single whole-book PDF and approved EPUB. Never create another publications tree, rename a released file, move the entire repository, overwrite a generated chapter or manufacture an output by copying a preview.

Do not modify Books 1 or 2: both are already published. The legacy stage name “03-Geometry-Cameras-and-Image-Formation” is a generator identifier; its content is relevant source, but the public title remains **From Geometry to Images**. Preserve original material and fold it into the eventual reader composition with traceable source mapping rather than replacing it with 32 short summaries.

## Planned release

When the manuscript, project labs and figure proofs are genuinely complete, compile and validate **one full reflowable EPUB** and **one full reading PDF** for Book 03. The author will run the Mac-specific production compile and inspect its output. Until then: manuscript and code only, no PDF, EPUB, or local release commands.

## Pass ledger

| Pass | Deliverable | State |
|---|---|---|
| 01 | Original first chapter, 32-chapter route, source-preservation ledger, portable C++23 Pixel Canvas | Authored; compilation/verification reported separately |
| 02 | Coordinate frames, inverse transforms, typed point/vector math, image mapping, terminal coordinate playground, two SVG diagrams | Authored; compilation/verification reported separately |
| 03 | Signed orientation/area, barycentric triangle containment/interpolation, bounded exact oracle, user-editable triangle REPL and original figures | Introductory triangle work authored; full mesh topology and advanced adaptive predicates reserved for Chapters 05 and 07 |
| 04 | Narrative affine transformations, active/passive bases, homogeneous matrices, inverse and pivot, normal transform, original SVG plates and C++23 affine house studio | Authored; native C++23 and final publication validation reported separately |
| 05 | Exact binary64 orientation, cancellation/subnormal/overflow witnesses, source-preserving numerical chapter, 3 SVG figures and CSV/SVG diagnostic lab | Authored; independent target-build and observed CI success remain open |
| 06 | 3D coordinate and normal mathematics, plane/ray/sphere intersection, spatial SVG/CSV/REPL, four figures and 2,500 seeded cross/dot checks | Authored; target compiler execution and observed CI success remain open |
| 07 | Indexed 3D mesh incidence, closed house reference, disconnected seam, winding/nonmanifold/bow-tie diagnostics, per-corner UVs, halfedge cycles/twins, CSR/Laplacian, five SVGs | Authored; original source preserved; compiler/CTest and final reader proof remain open |
| 08 | Bézier de Casteljau/Bernstein, Hermite, B-spline span, C/G joins, adaptive tessellation and 16-control bicubic indexed patch; six figures and editable C++23 studio | Authored; C++23/CI native success and source-unit reconciliation pending |
| 09 | Procedural heightfield/capped cylinder/UV sphere/strict implicit sphere, analytic normals, grid-edge shared extraction, independent counts and six original figures | Authored; native execution and source-unit reconciliation pending |
| 10 | Trustworthy imported OBJ subset, per-corner UV/normal index domains, GPU tuple conversion, glTF buffer layout proof, topology audit, six original diagrams | Authored; cumulative native runtime passed; full original-source reconciliation pending |
| 11 | Shared mesh instances, parent forest, affine composition, world-preserving reparent, normal matrices and bounds | Authored; native runtime and original-source integration pending |
| 12 | Proper rotations, Rodrigues, fixed XYZ, quaternion algebra, SLERP/NLERP and rigid SE(3) poses | Authored; native runtime and original-source integration pending |
| 13 | Calibrated pinhole, camera frames, intrinsics, FOV, image coordinates, world rays and projection derivatives | Authored; native runtime and original-source integration pending |
| 14 | Homogeneous projection, clip membership, NDC, viewport, finite/infinite depth and reverse-Z | Authored; native runtime and original-source integration pending |
| 15 | Calibrated stereo, epipolar constraints, triangulation, correspondence counterexamples and uncertainty | Authored; native runtime and original-source reconciliation pending |
| 16 | Indexed vertex processing, clip payloads, smooth varying interpolation and NDC depth | Authored; native runtime and source reconciliation pending |
| 17 | Rectangular and homogeneous primitive clipping, intersection payloads, provenance and fan assembly | Authored; native runtime and source reconciliation pending |
| 18 | Integer triangle coverage, top-left sample ownership and incremental edge evaluation | Authored; standalone native suite passed; integration pending |
| 19 | Opaque visibility, depth storage and equal-depth policies | Authored; standalone native tests and studio passed; integration pending |
| 20 | Sampling, aliasing, footprint integration and antialiasing foundations | Authored; standalone native tests and studio passed |
| 21 | Integrated software renderer and native gallery | Backend and OBJ front-end passed cumulative Ubuntu CI |
| 22 | Thin-lens optics, focus tolerance and geometric defocus | Authored; standalone native tests and studio passed |
| 23 | Distortion, Jacobians, numerical inversion and angular camera families | Authored; standalone native tests and studio passed |
| 24 | Exposure, photon statistics, dark and read noise, full well, gain and ADC | Authored; standalone native tests and studio passed |
| 25 | Relative XYZ, linear RGB, Display P3, transfer curves, white balance and HDR | Authored; standalone native tests and studio passed |
| 26 | Spatial camera DLT, decomposition, planar constraints, reprojection and noise | Authored; standalone native tests and studio passed |
| 27 | Essential/fundamental geometry, normalized eight-point estimation, Sampson residual, degeneracy and scale | Authored; standalone native tests and studio passed |
| 28 | Harris, normalized descriptors, ratio/mutual matching and RANSAC verification | Authored; standalone native tests and studio passed |
| 29 | Essential pose recovery, multi-ray triangulation, point refinement and bundle adjustment derivation | Authored; standalone native tests and studio passed |
| 30 | PCA normals, implicit fields, aligned depth TSDF and supported zero crossings | Authored; standalone native tests and studio passed |
| 31 | Integrated optical fixture, sparse reconstruction, PCA check, render adapter and exports | Authored; standalone native integrated tests and independent export verifier passed |
| 32 | Verification, scale ambiguity, independent derivative checks, paired baseline experiment and final evidence capstone | Authored; standalone native tests and sweep passed |
| Release gates | Source-unit reconciliation, math and figure audit, print/EPUB composition and acceptance | Native build/CTest passed; remaining gates open |



## Pass 24

[Chapter 24 — Light Becomes a Measurement](Chapters/24%20-%20Light%20Becomes%20a%20Measurement.md) continues the narrative into photon counts, Poisson shot noise, dark current, Gaussian read noise, gain and ADC saturation. P24 includes three analytical SVG figures and the C++23 sensor emulator. [Evidence](PASS24_EVIDENCE.md) and [source custody](PASS24_LEGACY_AUDIT.md) record the exact native results and limits. Next: Chapter 25 — Colors That Tell the Truth.


## Pass 25

[Chapter 25 — Colors That Tell the Truth](Chapters/25%20-%20Colors%20That%20Tell%20the%20Truth.md) moves from spectra and metamerism to XYZ, chromaticity, primary matrices, signed gamut coordinates, sRGB response, neutral balance and relative HDR. P25 includes four analytical SVG figures and the C++23 color laboratory. [Evidence](PASS25_EVIDENCE.md) and [source custody](PASS25_LEGACY_AUDIT.md) record exact native results and limits. Next: Chapter 26 — Teaching a Camera About Itself.


## Pass 26

[Chapter 26 — Teaching a Camera About Itself](Chapters/26%20-%20Teaching%20a%20Camera%20About%20Itself.md) derives normalized spatial DLT, camera decomposition, planar calibration constraints, distortion refinement principles and held-out verification. Four SVG figures and a native C++23 spatial calibration lab accompany the reader. [Evidence](PASS26_EVIDENCE.md) and [source custody](PASS26_LEGACY_AUDIT.md) record actual results and limits. Next: Chapter 27 — Two Photographs, One Hidden Point.


## Pass 27

[Chapter 27 — Two Photographs, One Hidden Point](Chapters/27%20-%20Two%20Photographs%2C%20One%20Hidden%20Point.md) derives E/F, Sampson residual, normalized eight-point estimation, cheirality and scale gauge. Four SVG figures and a native C++23 epipolar laboratory accompany the reader. [Evidence](PASS27_EVIDENCE.md) and [source custody](PASS27_LEGACY_AUDIT.md) record exact results and limits. Next: Chapter 28 — Finding the Same Corner Twice.


## Pass 28

[Chapter 28 — Finding the Same Corner Twice](Chapters/28%20-%20Finding%20the%20Same%20Corner%20Twice.md) develops the structure tensor, Harris, normalized descriptors, scale-space and binary feature context, ratio/mutual matching, RANSAC probability and precision/recall. Four SVG figures and a native C++23 feature/verification laboratory accompany the reader. [Evidence](PASS28_EVIDENCE.md) and [source custody](PASS28_LEGACY_AUDIT.md) record actual results and limits. Next: Chapter 29 — Building a Scene from Pictures.


## Pass 29

[Chapter 29 — Building a Scene from Pictures](Chapters/29%20-%20Building%20a%20Scene%20from%20Pictures.md) develops feature tracks, essential pose disambiguation, multiview triangulation, pixel-space point refinement, incremental registration and bundle adjustment with Schur elimination. Five original SVG figures and a C++23 reconstruction laboratory accompany the narrative. [Evidence](PASS29_EVIDENCE.md) and [source custody](PASS29_LEGACY_AUDIT.md) record observed results and limits. Next: Chapter 30 — Points Become Surfaces.


## Pass 30

[Chapter 30 — Points Become Surfaces](Chapters/30%20-%20Points%20Become%20Surfaces.md) develops local PCA normals, explicit and implicit representations, projective TSDF fusion, unknown-state semantics, interpolation and sparse-volume design. Five figures and a C++23 aligned-depth surface workshop accompany the reader. [Evidence](PASS30_EVIDENCE.md) and [source custody](PASS30_LEGACY_AUDIT.md) record actual results and limits. Next: Chapter 31 — A Studio for Geometry and Images.


## Pass 31

[Chapter 31 — A Studio for Geometry and Images](Chapters/31%20-%20A%20Studio%20for%20Geometry%20and%20Images.md) connects prior optical projection, sparse reconstruction, surface normal estimation, opaque visibility and image encoding into one controlled C++23 studio. Four figures, five exported artifacts and a repeatable independent verifier accompany the reader. [Evidence](PASS31_EVIDENCE.md) and [source custody](PASS31_LEGACY_AUDIT.md) record the 230,400 pixel-pass checks, exact hashes and limitations. Next: Chapter 32 — If It Looks Right, Is It Right?


## Pass 32 — narrative completion

[Chapter 32 — If It Looks Right, Is It Right?](Chapters/32%20-%20If%20It%20Looks%20Right%2C%20Is%20It%20Right.md) concludes the reader with independent oracles, metamorphic tests, numerical tolerances, geometric sensitivity, reproducibility and a capstone evidence dossier. The C++23 workshop verifies scale ambiguity and runs a 20,000-point paired baseline sweep. [Evidence](PASS32_EVIDENCE.md) and [source custody](PASS32_LEGACY_AUDIT.md) document the actual results and limitations. The chapter inventory is complete; cumulative verification and publication production are not.

## Release acceptance checklist

- [x] All 32 chapter paths and numbered headings individually verified.
- [x] P32 standalone native C++23 tests and paired sweep executed.
- [x] Full companion build, all 34 CTests (including P21 frontend), and all native workshop/export checks passed on Ubuntu at revision `a54c4a1`; [CI run](https://github.com/swayam8624/Project_008_AllMight/actions/runs/45562690493). Later prose edits require final source-gate confirmation.
- [ ] Complete original-source reconciliation, mathematical and pedagogical review, internal cross-reference and figure checks.
- [ ] Release builder and PDF/EPUB production in the existing repository layout.
- [ ] Visual PDF review and EPUB validation/navigation; cross-format content comparison.
- [ ] Final release acceptance before promotion. Books 01 and 02 and their published assets remain untouched.


## Pass 34 — cumulative engineering acceptance

[Pass 34 evidence](PASS34_EVIDENCE.md) records the P10 fixture correction and successful full CI run. [Foundations source map](PASS34_FOUNDATIONS_SOURCE_MAP.md) routes 18,928 body lines and 627 headings from seven original derived chapters. Three selected reader additions in Chapters 14, 17 and 31 address pixel-center conventions, frustum culling and camera-relative arithmetic. Source reconciliation and publication acceptance remain open.


## Pass 35 — graphics, vision and XR source reconciliation

[Source map](PASS35_GRAPHICS_SOURCE_MAP.md) inventories fourteen primary Graphics Vision and XR derived sources (28,118 routed lines, 502 heading entries). Together with P34, 21 primary derived files and 47,046 source body lines are mapped. Four selected additions in Chapters 06, 07, 20 and 29 recover ray–triangle intersection, cotangent operators, texture addressing and rigid registration. [Evidence](PASS35_EVIDENCE.md) records the independently checked numerical witnesses and exact tested script. Full line-by-line reconciliation and publication remain pending.


## Passes 36–37 — reader refinement and verified source custody

P36 adds selected answers for Chapters 24–32, repairs 336 inline-code delimiters and removes nine stale planned handoffs. P37 adds the [pinned source receipt](SOURCE_CUSTODY_BASELINE.json), derives the Chapter 08 surface metric and improves figures 129, 132 and 136. [Evidence](PASS37_EVIDENCE.md) records the full successful CI with 25 exact pinned source checks and all 34 CTests, while explicitly retaining 39 historical manifest discrepancies as unresolved. No release artifacts generated.


## Pass 38 — complete reader composition

[Preface and reading route](FRONT_MATTER.md), [companion and notation guide](BACK_MATTER.md) and the new Book 03 proof driver now assemble the entire reader from a common Pandoc AST. [Preparation CI](https://github.com/swayam8624/Project_008_AllMight/actions/runs/45571742458) passed 32 chapters, 142 rendered SVGs, 2,533 math nodes and 64 code blocks. [Evidence and commands](PASS38_EVIDENCE.md) explain the new proof workflow, caption/reference checks, print smoke test and actual-output QA. Full proof compilation, visual reader acceptance, cover integration and historical manifest reconciliation remain open. No published PDF/EPUB was changed.
