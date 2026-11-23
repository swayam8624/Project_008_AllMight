# Book 03 — Pass 08 original-source audit: curves, splines, patches and surface representation

**Authoring date:** 23 November 2026  
**Branch:** curriculum/book03-geometry-images-pass01  
**Scope:** inspect original reading-copy content and record exact Git blob identities; preserve the canonical continuous notes and existing large edition without deletions or moving source passages.

## The original source was located, not guessed

The reading map \`Notes/Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map.md\`, inspected blob \`59b5c29b46b180a1dc455b8eca96dca3e7fce245\`, identifies two essential original chapters for Book 03 Chapter 08.

| Original reading-copy file | Inspected blob | Source-master range and declared hash | Actually inspected content |
|---|---|---|---|
| \`01 - From Worlds to Images and Inference.md\` | \`c727c2f278bd852fc194dabba9ab77fa30940b37\` | Graphics master lines **1–2203**; header-declared SHA256 \`4e96f00f81fcedaba2f588a213edbd5f40d03c1303f3520719c1711b56329822\` | The original section **Curves as controllable functions**, reading-copy lines approximately **1927–2215**, including monomial and Hermite/Bézier basis changes, cubic blossom, de Casteljau, independent numerical examples, Catmull–Rom, B-spline basis/joins, rational representations, adaptive sampling, and the progression into tensor-product surfaces. Earlier formula/algorithm exposition at approximately lines **520–695** was also identified through direct content inspection and must not be dropped as redundant merely because the chapter ends with another curve section. |
| \`02 - Surfaces, Topology and Parameterization.md\` | \`41a7b270a4ee80ec12cdd57c8421351565f25ec4\` | Graphics master lines **2204–3118**; header-declared SHA256 \`124ad7fa099f4d4b38f41d50bfa82f53d8cab0b2d962cd14b22be2cb1f69f270\` | Reading-copy lines approximately **14–61** derive tensor-product bicubic Bézier patches, partial derivatives \(S_u,S_v\), normals, singular patch parameters, seam continuity and alternatives (implicit, mesh, subdivision). Further inspected passages approximately **130–380**, and **601–880**, develop parameterization distortion, subdivision masks, Loop and Catmull–Clark, differential geometry, Poisson/surface editing, mesh operators and topology. These belong to later Book 03 and Book 04–06 source reconciliation, not wholesale into one introductory cubic lab. |

The original source atlas \`36 - Source Atlas and Reconstruction Checkpoints.md\`, inspected blob \`6dc8d20a7aaf867a6bb6bb5d835ffe246c6aaadb\`, directly references:
- **MITG-02 — Cubic curves**: https://www.youtube.com/watch?v=1BZ0ayVjWEk , with original lecture records on cubic interpolation, Hermite/monomial/Bernstein matrices, blossoming and de Casteljau evaluation/subdivision.
- **MITG-03 — Curves and surfaces**: https://www.youtube.com/watch?v=LzIjSdNoR9s , on curvature/regularity, C/G continuity, uniform B-spline basis and knots, rational conic descriptions, sixteen-control patches, partial derivatives, seam continuity, and subdivision.
- **P3-13 — Curves and Surfaces**: https://www.youtube.com/watch?v=FTg1DUr7bhY , on control polygon and interpolation distinction, G1/C1 joins, B-spline knots and NURBS.
- **P3-20 — Camera Motion**: https://www.youtube.com/watch?v=HAJv25Afsz8 , on Catmull–Rom waypoint interpolation and endpoint tangent conventions.

All URLs are pre-existing source-atlas entries. Original license/attribution and lecture version metadata should be preserved through final composition. The actual exercises and original algorithmic witnesses must be reconciled rather than shortened into links alone.

The two SHA256 source-span hashes above are **metadata copied from the existing derived chapter headers**, not independently recomputed hashes against the full continuous master. In fact the connected file-fetch route returned an **empty content payload** for the long continuous master even when a line range was requested, so we cannot honestly certify the complete master content or the declared SHA256 from that fetch. The original reading-copy chapter data was accessible and inspected; the canonical files were not edited.

## Explicit subsection routing for the new reader chapter

| Original topic or proof | Where it is taught in new reader Chapter 08 | What remains for later deep integration |
|---|---|---|
| Linear interpolation, polynomial versus control representations | The branch/walk narrative, eq. (8.1) and de Casteljau calculation at \(\frac12\) | Preserve original 2000+ word comparison, basis matrices, sample code and lecture-specific examples |
| Bernstein basis, convex hull, affine invariance | Direct Bernstein and de Casteljau; separate C++ implementations and deterministic tests | Full blossoming, degree elevation and general basis conversion must retain original deep proofs |
| Cubic blossom: symmetric, separately affine, diagonal identity | Additional original-source-preserving blossom section with actual polarization formula | General multiaffine and polar-form derivations, full original numeric example |
| Bézier derivatives, curvature and time versus parameter speed | Endpoint tangent and second derivative, curvature formula and chain-rule time acceleration | Full regular curves, osculating circles, G2, Frenet frames and numerical curvature certification |
| Hermite endpoint position/derivative constraints | Full Hermite basis, conversion to/from Bézier controls, independent direct evaluator | Monomial and Hermite-to-Bernstein matrix representations in original source |
| Catmull–Rom interpolating waypoints | A clearly labelled uniform Catmull–Rom-to-Hermite derivation and parameter cautions | A camera-path implementation with centripetal vs uniform cases in Chapter 11/12 and animation volume |
| Uniform cubic B-spline span and C2 joins | Local basis, correct noninterpolation of outer controls, exact Bézier conversion, adjacent span C2 tests | Nonuniform knots, multiplicity/continuity proofs, global fit/optimization, degree elevation and rational weight handling |
| Rational B-splines / circular arcs | Rational basis formula and exact rational **quadratic** quarter circle example | A correctly specified NURBS evaluation/editing module later; never present the nonrational P08 uniform span as NURBS |
| De Casteljau subdivision and adaptive curve drawing | Split identity, control-hull to **finite segment** distance criterion, explicit max-depth failure | Certified numerical error bounds, screen-space perspective-aware tolerances, robust cusp and trim cases |
| Sixteen-control bicubic patches and normals | Original tensor-product formulation, planar oracle, derivatives, tangent normal, 3D canopy | Subdivision surfaces; patch boundary stitching/parameter mismatch; G1/C1 surface conditions; trimming and limit-surface singularities |
| Mesh and topology preservation during tessellation | Shared indexed grid vertices, \(V,E,F\), Euler proof, P07 independent topology oracle | Multi-patch shared-boundary repair, watertightness/embedding proofs, extraordinary-vertex handling and CAD import |

## New source and figures in this pass

New reader chapter: \`Notes/Book Production/Seven-Book Curriculum/Book03/Chapters/08 - Drawing Smooth Things with Control Points.md\`.

Cumulative C++23 lab source:
- \`include/book03/curves3d.hpp\` — cubic Bézier de Casteljau and independent Bernstein; derivatives, subdivision; Hermite direct basis and conversion; nonrational uniform cubic B-spline evaluation and Bézier conversion; continuity checks; adaptive finite-chord tessellation; bicubic patch with partial derivatives and indexed triangle sampling.
- \`tests/curves3d_tests.cpp\` — hand-checkable midpoint/tangent cases, 2000 de Casteljau/Bernstein checks, 300 subdivision reparameterization checks, independent Hermite/B-spline verification, G1/C1/C2 counterexamples, closed-endpoint loop, explicit depth-exhaustion status, and P07 2×3 and 6×7 indexed patch topology checks.
- \`src/curve_studio.cpp\` — reader-editable curve and patch controls, tolerance, CSV, SVG and indexed OBJ. The outputs derive from the *same* C++23 calculations rather than from a duplicated hand-made program.

Original figures (newly authored, semantically described SVGs): \`Assets/22-de-casteljau-levels.svg\`, \`23-bezier-subdivision.svg\`, \`24-curve-continuity.svg\`, \`25-uniform-bspline-spans.svg\`, \`26-adaptive-curve-tessellation.svg\`, \`27-bicubic-patch-grid.svg\`.

## External sources checked and credited

- Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed., primary book publisher page: https://shop.elsevier.com/books/curves-and-surfaces-for-cagd/farin/978-1-55860-737-8
- Pomax, *A Primer on Bézier Curves*, accessible interactive reference: https://pomax.github.io/bezierinfo/
- Patrikalakis, Maekawa and Cho, *Shape Interrogation for Computer Aided Design and Manufacturing*, Springer: https://link.springer.com/book/10.1007/978-3-642-04074-0

No external artwork or source code was copied into the new reader or lab. These are original implementations of known mathematics.

## Proof and engineering honesty

- The mathematical control-hull distance-to-chord criterion is conservative **in exact real arithmetic** because the chord is a convex set and distance to it is convex. The actual C++23 recursion uses rounded binary64 distances and is not a certified Hausdorff error bound.
- \`CubicBezier3\`, Hermite and B-spline functions reject nonfinite/out-of-domain inputs. Their derivatives are with respect to *normalized curve parameter*, not physical time or arc length.
- The local uniform cubic B-spline span is not a global nonuniform rational B-spline. Knot multiplicity, weight denominators, exact circle construction, trimming and editing are outside the code's current scope.
- The bicubic patch sampled into a single rectangular grid maintains shared vertex IDs and consistent face winding **inside its grid**. This is not automatic patch-to-patch stitching, global regularity, NURBS trimming, or collision proof.
- Two genuinely different evaluation formulas are present for Bézier and Hermite/B-spline, with hand derivations and independent topology oracles. Test source does not equal a confirmed passing test run.
- The original long manuscript and publication layout remain preserved; this audit records only the source excerpts actually read and the later reconciliation owed.

## The next source-ledger target

Chapter 09 — **Making a World from Rules** should preserve the original \`02 - Surfaces, Topology and Parameterization.md\` material on implicit scalar fields, level sets, parametric versus implicit modeling, subdivision and extraordinary vertices, and must use the original procedural and reconstruction source atlas where applicable. Build actual indexed meshes rather than disconnected triangle soup. Keep the new seven-book manuscript additive until a complete source reconciliation and the author's Mac release build.
