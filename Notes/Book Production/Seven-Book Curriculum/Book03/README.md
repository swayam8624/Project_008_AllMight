# Book 03 — From Geometry to Images

**Subtitle:** 3D Geometry, Transformations, Cameras, Projection, and Image Formation  
**Series:** From Bits to Research  
**Active branch:** curriculum/book03-geometry-images-pass01  
**Author:** Swayam Singal  
**State:** Authoring passes 01–17; seventeen substantial narrative chapters, seventeen cumulative C++23 laboratories, eighteen source-authored CTests and seventy-seven original SVG teaching figures are committed. NO Book 03 PDF or EPUB has been generated or promoted.

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
21. [P01–P17: C++23 Image, Frame, Triangle, Affine, Predicate, 3D, Mesh, Curve, Procedural, Asset Trust, Scene, Rotation, Camera, Projection, Stereo, Vertex and Clipping Studios](../../../../Code%20Parts/tools/labs/book03_cpp23/README.md) — numerical code, exact tests, terminal REPL and source-generated SVG diagrams.
22. [Pass 02 original-source audit](PASS02_LEGACY_AUDIT.md) — records the inspected camera/geometry foundation spans without editing the canonical manuscripts.
23. [Pass 02 evidence record](PASS02_EVIDENCE.md) — provenance, exact changes and what is not yet verified.
24. [Pass 03 source audit](PASS03_LEGACY_AUDIT.md) — original geometry/rasterization content preserved with exact source identities.
25. [Pass 03 evidence record](PASS03_EVIDENCE.md) — what the third round actually delivered and which acceptance gates remain.
26. [Pass 04 source audit](PASS04_LEGACY_AUDIT.md) — maps original basis, active/passive, camera and normal-source boundaries.
27. [Pass 04 evidence record](PASS04_EVIDENCE.md) — authored code, mathematical checks and honest validation limits.
28. [Pass 05 source-subsection audit](PASS05_LEGACY_AUDIT.md) — robust predicates plus original camera-frustum content routed to their proper chapters.
29. [Pass 05 evidence record](PASS05_EVIDENCE.md) — exact input-representation proof, tests, reporting and remaining acceptance gates.
30. [Pass 06 original-source audit](PASS06_LEGACY_AUDIT.md) — actual inspected 3D ray/mesh/camera source entrypoints, with subsequent chapters' responsibilities preserved.
31. [Pass 06 evidence record](PASS06_EVIDENCE.md) — new 3D math APIs, tests, figures, source custody and open validation gates.
32. [Pass 07 original-source audit](PASS07_LEGACY_AUDIT.md) — actually inspected halfedge, winged-edge, vertex-link, CSR and Laplacian source sections.
33. [Pass 07 evidence record](PASS07_EVIDENCE.md) — authored mesh and halfedge C++23 project, figures, exact combinatorial checks and outstanding runtime/release gates.
34. [Pass 08 original-source audit](PASS08_LEGACY_AUDIT.md) — actual inspected original MIT graphics source sections on cubic curves, splines, patches and later subdivision material.
35. [Pass 08 evidence record](PASS08_EVIDENCE.md) — new cubic/B-spline/Hermite math, studio, original illustrations and unobserved runtime gates.
36. [Pass 09 original-source audit](PASS09_LEGACY_AUDIT.md) — original parametric and implicit surface proofs, SDFs, learned-shape source atlas and preserved later material.
37. [Pass 09 evidence record](PASS09_EVIDENCE.md) — procedural mesh generators, numerical/topological checks, source exports, original figures and open execution gates.
38. [Pass 10 original-source audit](PASS10_LEGACY_AUDIT.md) — actual OBJ/glTF, per-corner shading, half-edge/topology and scene source paths, with implementation boundaries.
39. [Pass 10 evidence record](PASS10_EVIDENCE.md) — native OBJ subset/import studio, glTF bounds kernel, eleven CTests and open runtime acceptance gates.
40. [Pass 11 original-source audit](PASS11_LEGACY_AUDIT.md) — hierarchy, transform-chain and shading source custody.
41. [Pass 11 evidence record](PASS11_EVIDENCE.md) — scene forest, five instances, twelfth CTest and outstanding validation.
42. [Pass 12 original-source audit](PASS12_LEGACY_AUDIT.md) — quaternion, animation and rigid-pose source custody.
43. [Pass 12 evidence record](PASS12_EVIDENCE.md) — orientation debugger, thirteenth CTest and outstanding validation.
44. [Pass 13 original-source audit](PASS13_LEGACY_AUDIT.md) — pinhole, intrinsics, pose and projection sensitivity custody.
45. [Pass 13 evidence record](PASS13_EVIDENCE.md) — native camera simulator, fourteenth CTest and outstanding gates.
46. [Pass 14 original-source audit](PASS14_LEGACY_AUDIT.md) — projection, clipping and depth source custody.
47. [Pass 14 evidence record](PASS14_EVIDENCE.md) — scalar depth experiment, fifteenth CTest and acceptance boundaries.
48. [Pass 15 original-source audit](PASS15_LEGACY_AUDIT.md) — stereo, disparity, calibration and XR source custody.
49. [Pass 15 evidence record](PASS15_EVIDENCE.md) — reconstruction laboratory and sixteenth CTest.
50. [Pass 16 original-source audit](PASS16_LEGACY_AUDIT.md) — vertex and varying source custody.
51. [Pass 16 evidence record](PASS16_EVIDENCE.md) — vertex tracer, seventeenth CTest and P05 CSV correction.
52. [Pass 17 original-source audit](PASS17_LEGACY_AUDIT.md) — homogeneous planes and clipping source custody.
53. [Pass 17 evidence record](PASS17_EVIDENCE.md) — clipping explorer, eighteenth CTest and numerical limitations.

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

The first seventeen passes do **not** assert that all 32 chapters have been drafted, that the legacy source has been fully integrated, that the C++ tests have passed without observed results, or that a PDF or EPUB exists. Later passes will use the substantial legacy chapter material at the locations documented in SOURCE_LEDGER.md. The author explicitly plans to compile and visually review the finished book on macOS at the end of the book-development process.

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
| 10 | Trustworthy imported OBJ subset, per-corner UV/normal index domains, GPU tuple conversion, glTF buffer layout proof, topology audit, six original diagrams | Authored; native runtime and full original-source reconciliation pending |
| 11 | Shared mesh instances, parent forest, affine composition, world-preserving reparent, normal matrices and bounds | Authored; native runtime and original-source integration pending |
| 12 | Proper rotations, Rodrigues, fixed XYZ, quaternion algebra, SLERP/NLERP and rigid SE(3) poses | Authored; native runtime and original-source integration pending |
| 13 | Calibrated pinhole, camera frames, intrinsics, FOV, image coordinates, world rays and projection derivatives | Authored; native runtime and original-source integration pending |
| 14 | Homogeneous projection, clip membership, NDC, viewport, finite/infinite depth and reverse-Z | Authored; native runtime and original-source integration pending |
| 15 | Calibrated stereo, epipolar constraints, triangulation, correspondence counterexamples and uncertainty | Authored; native runtime and original-source reconciliation pending |
| 16 | Indexed vertex processing, clip payloads, smooth varying interpolation and NDC depth | Authored; native runtime and source reconciliation pending |
| 17 | Rectangular and homogeneous primitive clipping, intersection payloads, provenance and fan assembly | Authored; native runtime and source reconciliation pending |
| 18+ | Sample ownership through complete 32-chapter coverage, source-unit reconciliation, math and figure audits, print/EPUB composition and acceptance | Not started |

