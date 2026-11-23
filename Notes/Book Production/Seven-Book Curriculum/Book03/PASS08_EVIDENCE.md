# Book 03 — Pass 08 evidence and outstanding verification

**Series:** From Bits to Research  
**Volume:** Book 03 — From Geometry to Images  
**Chapter:** 08 — Drawing Smooth Things with Control Points  
**Branch:** curriculum/book03-geometry-images-pass01  
**Date:** 23 November 2026  
**Publication rule:** No intermediate PDF/EPUB build, release promotion or change to already published Books 01–02.

## New full-length manuscript

\`Chapters/08 - Drawing Smooth Things with Control Points.md\` is a newly authored, long, narrative-first reader draft (over 7,500 words after source reconciliation). It begins with the house's stiff, straight-edged garden tree and develops from linear interpolation to genuine cubic curves. Each equation has human-language motivation, worked numbers, connection to C++ types, falsification tests and exercises.

Coverage is substantive rather than a vocabulary list:
- Linear interpolation between affine points and the distinction between normalized parameter, arc length and physical time.
- Three-level de Casteljau evaluation at \(t=1/2\), with a hand-derived midpoint and exact control-polygon distinction.
- Cubic Bernstein basis, convex hull, independent numerical evaluation and why interior controls are not general interpolation waypoints.
- Parameter derivatives, second derivative, endpoint tangents, speed/acceleration chain rule and curvature for regular 3D curves.
- The cubic blossom, its symmetry, separate affinity, diagonal identity, control-value identities and relationship to de Casteljau and subdivision.
- Exact real-arithmetic de Casteljau subdivision and the new subcurves' parameter maps.
- \(C^0, G^1, C^1, C^2\) joins with different geometric and parameter-speed requirements.
- Cubic Hermite interpolation and conversion to/from Bézier handles.
- Uniform Catmull–Rom via waypoint-based Hermite tangents, plus explicit warning that the code does not implement a full Catmull–Rom path editor.
- Uniform nonrational cubic B-spline local basis, its lack of endpoint-control interpolation, overlapping spans, \(C^2\) join under simple knots and exact polynomial basis conversion to Bézier.
- Knot multiplicity, rational Bézier formula, a correct rational quadratic quarter-circle construction and explicit NURBS-vs-nonrational distinctions.
- Adaptive tessellation using control-hull-to-**finite-segment** distance, closed-endpoint loop adversary, max-depth failure and the difference between a mathematically conservative criterion and a floating-point certification.
- Sixteen-control bicubic tensor-product patches, two independent parameter directions, partial derivatives, regular normals, singularity warnings and parameter-grid triangle sampling.
- Properly indexed rectangular patch mesh derivations and independent P07 topology checks; shared grid vertices vs disconnected seams across multiple patches.
- Twelve-plus reconstruct/falsify/transfer/implement/explain problems with selected answers and a Chapter 09 natural transition.

**Scale distinction:** The chapter's first hand example spans X from 0 to 2 and has midpoint derivative \((3,0)\). The native studio reference curve spans X from −2 to 2, so its midpoint derivative is \((6,0,0)\). Both are mathematically correct; the lab README explicitly notes the difference.

## Six original authored SVG scientific illustrations

The new assets occupy the existing \`Notes/Book Production/Seven-Book Curriculum/Book03/Assets/\` directory:

1. \`22-de-casteljau-levels.svg\`: actual successive linear interpolants and mathematically computed midpoint.
2. \`23-bezier-subdivision.svg\`: one original curve and its two exact mathematical subcurve shapes, with independent control polygons at \(s=0.37\).
3. \`24-curve-continuity.svg\`: distinction among gap, only-\(C^0\), \(G^1\)-not-\(C^1\), and fully straight \(C^2\) joins, clearly labelled as schematic.
4. \`25-uniform-bspline-spans.svg\`: two actual local uniform cubic span samples computed from five controls and their common knot.
5. \`26-adaptive-curve-tessellation.svg\`: independently computed recursive chords for an arch and a loop with identical endpoint locations; explicitly not a certified error performance chart.
6. \`27-bicubic-patch-grid.svg\`: actual sixteen-control bicubic garden geometry with six-by-six connected parameter grid and independent combinatorial counts \(V=49,E=120,F=72,B=24,\chi=1\).

All figures include original SVG titles and descriptions. No third-party illustrations or downloaded fonts were added to the source tree. Source figures are not yet proof of final EPUB/PDF print legibility; that needs later rendering and reader review.

**Cumulative Book 03 figures: 27.**

## Actual cumulative C++23 deliverables

**\`include/book03/curves3d.hpp\`** (original new header):
- \`CubicBezier3::at\` de Casteljau, independent \`bernstein_at\`, first/second parameter derivatives and algebraic subdivision with exact parameter intervals.
- \`CubicHermite3\` direct basis evaluator and conversion to Bézier.
- \`UniformCubicBSplineSpan3\` direct nonrational uniform cubic basis and Bézier conversion. It **does not claim a full global knot solver, NURBS or trimmed surface**.
- \`CurveContinuity\` and \`join_continuity\` report separate point gap, derivative gap, second derivative gap, \(C^0,G^1,C^1,C^2\) conditions with declared tolerances.
- Control-point-to-*finite chord segment* distance criterion; de Casteljau adaptive tessellation with finite positive tolerance, bounded depth (≤16), ordered samples and explicit \`tolerance_met_under_float_policy=false\` when it cannot reach the stated criterion within the budget.
- \`BicubicBezierPatch3\`: sixteen controls, tensor-product evaluation, both parameter partials and regular unit normal. Sampling functions create \`Mesh3\` directly, with shared indexed positions and consistent face winding.

**\`tests/curves3d_tests.cpp\`** (new ninth CTest):
- Hand-solved de Casteljau midpoint, exact endpoints/first and second derivatives.
- 2,001 sample comparisons of independent Bernstein and de Casteljau implementations.
- Hundreds of algebraic subdivision reparameterization checks.
- Direct Hermite vs converted Bézier basis comparisons; \(G^1\)-not-\(C^1\), \(C^1\)-not-\(C^2\) and disconnection counterexamples.
- Noninterpolated B-spline endpoints and \(C^2\) uniform overlapping span.
- A zero-chord loop that **must** be tessellated, monotone parameter order, dense independent point-to-polyline sample tests, maximum-depth exhaustion.
- Flat bicubic patch with independent \((3u,3v,0)\) and derivative/normal checks.
- P07 independent indexed mesh counts for a 2×3 plane (\(V=12,E=23,F=12\)) and 6×7 curved patch (\(V=56,E=139,F=84\)).
- Domain, nonfinite control, invalid resolution and zero/singular normal rejection.

**\`src/curve_studio.cpp\`** (new native terminal workshop):
- \`book03_curves inspect|svg|csv|obj|repl\`, actual curve control editing and 4×4 patch control editing.
- REPL \`control ID X Y Z\`, \`patch-control I J X Y Z\`, \`tolerance value\`, \`reset\`, and file exports.
- Source-calculated SVG of handles, adaptive Bézier samples, actual patch triangulation and unique indexed edges (oblique parallel illustration, no camera/visibility claim).
- Numeric \((t,x,y,z)\) CSV of current adaptive samples.
- 1-based indexed geometry-only OBJ positions/faces from current patch controls; default 12×12 grid is **169 vertices, 288 faces, 456 unique edges, 48 boundary edges, Euler one**.
- The SVG/CSV/OBJ and report use the **same in-memory mathematics** rather than a separate decorative source.

**Project wiring:** CMake adds an interface target \`book03_curve_math\`, \`book03_curves\` and \`book03_curves_tests\`. The cumulative project now has **eight labs and nine CTests** (Chapter 07 contributes two).

**Dedicated workflow:** the same established Book03 GitHub Action now includes source/figure gates and a P08 stage verifying the numeric report, current REPL edits, ordered CSV samples, SVG semantics, exact OBJ counts and face-index bounds. The step is authored, **not claimed green** without an observed CI run.

## Original-source preservation and source-custody work

A targeted original-file search identified the actual reading-copy sources in the old Graphics & Vision source atlas (rather than inventing a legacy source path). \`PASS08_LEGACY_AUDIT.md\` records precise file paths, blobs and inspected source ranges for:

- \`Notes/Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference.md\`, especially its long \`Curves as controllable functions\` section and original Hermite/Bernstein/blossom/Catmull–Rom/B-spline/rational/sampling material.
- \`Notes/Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md\`, especially its sixteen-control patches, partial derivatives, seams, subdivision and differential geometry.
- \`Notes/Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints.md\`, with MITG-02, MITG-03, P3-13 and P3-20 course/video source records.

The declared original continuous master source-span SHA256 values are preserved from existing derived chapter headers, **not independently rehashed**; the connected fetch for the enormous master supplied an empty content payload. A future complete source-unit reconciliation and independent hash validation remain required before release.

External primary/foundational references were checked and credited in the chapter: Farin, *Curves and Surfaces for CAGD*; Pomax, *A Primer on Bézier Curves*; Patrikalakis/Maekawa/Cho, *Shape Interrogation*. The new diagrams and implementations are original, not copies of their graphics or library code.

**Do not edit or discard:** any file in the original \`Notes/Continuous Notes/\`, \`Notes/Chapter Edition/\`, \`Notes/Book Production/AllMight Codex/LaTeX/\`, existing published Books 01–02 or \`From Bits to Research/\` distribution structure. This pass writes only the new Book03 additions, cumulative C++23 companion and dedicated workflow.

## Validation status — authored versus observed

**Observed authoring:** New chapter, new mathematical code/tests/workbench, six SVG files, CMake/CI/editorial contracts and accurate original-source audit have been acknowledged by the connected GitHub repository. Several central identities were independently checked algebraically while writing: the \(t=1/2\) arch midpoint; direct Bernstein weights; B-spline-to-Bézier conversion; the correct difference between two-metre and four-metre control spans; planar/curved grid combinatorial counts.

**Not yet observed:** target compiler successfully building the full C++23 project, the ninth CTest passing, generated source SVG/CSV/OBJ artifacts from the executed native binary, GitHub Actions success, cross-compiler parity, screenshot-level scientific figure QA, a complete manuscript/source-unit reconciliation, a full Book03 LaTeX/PDF/EPUB release, or KDP upload/preview. A final repository compare against \`main\` is required before declaring the branch-scoped preservation check complete.

## Next substantive authoring pass

Chapter 09 — **Making a World from Rules**. Extend P08's real indexed-geometry output into parametric primitives, implicit level sets, normals, pole/seam handling and scientifically checkable procedural mesh generation. Inspect the actual existing \`Surfaces, Topology and Parameterization\` and source-atlas material before authoring. Guard against duplicated seam indices, mismatched periodic wraps, degenerate poles and obviously invalid implicit gradients. Keep full final Book 03 compilation on the author's Mac until all chapters and research/source mappings are complete.
