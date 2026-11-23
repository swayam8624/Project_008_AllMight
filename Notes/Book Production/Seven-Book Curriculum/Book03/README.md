# Book 03 — From Geometry to Images

**Subtitle:** 3D Geometry, Transformations, Cameras, Projection, and Image Formation  
**Series:** From Bits to Research  
**Active branch:** curriculum/book03-geometry-images-pass01  
**Author:** Swayam Singal  
**State:** Authoring passes 01–06; six substantial narrative chapters, six cumulative C++23 laboratories and sixteen original SVG teaching figures are committed. NO Book 03 PDF or EPUB has been generated or promoted.

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
10. [P01–P06: C++23 Image, Frame, Triangle, Affine, Exact Predicate and 3D Spatial Studios](../../../../Code%20Parts/tools/labs/book03_cpp23/README.md) — numerical code, exact tests, terminal REPL and source-generated SVG diagrams.
11. [Pass 02 original-source audit](PASS02_LEGACY_AUDIT.md) — records the inspected camera/geometry foundation spans without editing the canonical manuscripts.
12. [Pass 02 evidence record](PASS02_EVIDENCE.md) — provenance, exact changes and what is not yet verified.
13. [Pass 03 source audit](PASS03_LEGACY_AUDIT.md) — original geometry/rasterization content preserved with exact source identities.
14. [Pass 03 evidence record](PASS03_EVIDENCE.md) — what the third round actually delivered and which acceptance gates remain.
15. [Pass 04 source audit](PASS04_LEGACY_AUDIT.md) — maps original basis, active/passive, camera and normal-source boundaries.
16. [Pass 04 evidence record](PASS04_EVIDENCE.md) — authored code, mathematical checks and honest validation limits.
17. [Pass 05 source-subsection audit](PASS05_LEGACY_AUDIT.md) — robust predicates plus original camera-frustum content routed to their proper chapters.
18. [Pass 05 evidence record](PASS05_EVIDENCE.md) — exact input-representation proof, tests, reporting and remaining acceptance gates.
19. [Pass 06 original-source audit](PASS06_LEGACY_AUDIT.md) — actual inspected 3D ray/mesh/camera source entrypoints, with subsequent chapters' responsibilities preserved.
20. [Pass 06 evidence record](PASS06_EVIDENCE.md) — new 3D math APIs, tests, figures, source custody and open validation gates.

## What is already authored?

Pass 01 begins the new book with an original narrative on the distinction between a physical scene, a mathematical image and a stored pixel; discrete addressing; continuous sample locations; integer drawing; overflow-aware image buffers; PPM export; and a deliberately imperfect line rasterizer. The first C++23 project and accompanying tests are intended as a runnable contract, not a replacement for the full reader book.

Pass 02 adds a second full chapter on coordinate frames, point versus vector types, basis matrices and inverse maps, units, nested transform composition, a world-up/image-down coordinate bridge, exact examples and deliberate failures. Its P02 C++23 code extends the same companion with a typed affine frame model, independent numerical tests and a terminal-controlled SVG frame visualizer. Four original accessible figures now support Chapters 01–02.

Pass 03 adds a third long narrative chapter with two scientific diagrams, a bounded exact integer oracle, a deliberately noncertified double-precision barycentric query, and a C++23 triangle REPL that can visualize and save both fixed reference output and the reader's current edited triangle.

Pass 04 adds the fourth narrative chapter with original affine/transform-order/normal diagrams, a new homogeneous-affine 2D C++23 module, independent numerical tests including 2,000 deterministic transform round trips, and a house studio with inspectable matrices, SVG/PPM output, transform-order witness and editable terminal REPL.

Pass 05 drafts the full numerical-geometry chapter, with three original explanatory SVG figures and a self-contained C++23 exact orientation predicate for **finite stored IEEE binary64 coordinates**. The exact engine uses the previous bounded-int64 oracle for small integer inputs and a signed arbitrary-limb dyadic path otherwise, with 5,000 deterministic cross-checks specified in the new test source. The project adds a numeric observatory with an interactive REPL and reproducible SVG/CSV decision reports.

Pass 06 drafts the long three-dimensional transition chapter: points and displacements in space, dot and cross, true face normals, planes and signed distances, forward rays and their intersection policies, spheres and stable quadratic root selection. Four new original scientific figures continue the series visual style. A native `geometry3d.hpp` math reference, `spatial_studio.cpp` oblique 3D scene viewer (SVG/CSV/REPL) and independent `geometry3d_tests.cpp` suite introduce 2,500 deterministic vector identities and boundary cases. The P03 bounded exact 2D orientation serves as an independent source of evidence for a triangle embedded in the 3D XY plane, without falsely claiming that the new 3D cross product is exact.

The first six passes do **not** assert that all 32 chapters have been drafted, that the legacy source has been fully integrated, that the C++ tests have passed without observed results, or that a PDF or EPUB exists. Later passes will use the substantial legacy chapter material at the locations documented in SOURCE_LEDGER.md. The author explicitly plans to compile and visually review the finished book on macOS at the end of the book-development process.

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
| 07 | Indexed meshes, shared topology, adjacency and model-data validity | Not started |
| 08 | 3D cameras, poses and projection | Not started |
| 09 | CPU clipping, triangle rasterization and visibility | Not started |
| 10 | Lens/sensor/image formation and calibration | Not started |
| 11 | Stereo geometry and introductory reconstruction | Not started |
| 12+ | Complete 32-chapter coverage, source-unit reconciliation, math and figure audits, print/EPUB composition and acceptance | Not started |
