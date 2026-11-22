# Book 03 — Pass 04 authoring and verification evidence

**Book:** 03 — From Geometry to Images  
**Authoring stage:** Pass 04, Chapter 04 — Move the World Without Losing It  
**Development branch:** curriculum/book03-geometry-images-pass01  
**Editorial status:** complete first draft of the chapter and source/tests; end-to-end publication QA and native execution remain separate gates.  
**Date:** 23 November 2026

## Authored chapter, not merely an outline

\`Chapters/04 - Move the World Without Losing It.md\` is a continuous narrative chapter that carries the house-and-garden example forward. It begins with the lived intuition of moving a paper cutout, then derives translation, rotation, nonuniform scale, shear and reflection; affine preservation of barycentrics; 3×3 homogeneous matrix representation and its distinction from future projective cameras; column-vector composition and order; parent/child frames and pivots; inverse construction and conditioning; signed-area determinant behavior; and inverse-transpose normal transformation. Worked examples include T*R versus R*T, pivot rotation of a door corner, a hand-solvable shear+translation inverse, reflection of the Chapter 03 roof, and a sheared tangent/normal perpendicularity witness. The chapter also has explicit reconstruct/falsify/transfer/implement/explain exercises and selected answers.

A follow-up narrative bridge incorporates the distinction between an **active** movement in a fixed frame and a **passive** change of coordinates for an unchanged vector, with the old Computing Foundations worked basis example \(C=[[1,-1],[1,1]]\) converting standard coordinates (4,2) to coefficients (3,-1). The original source chapter is untouched; the new approachable explanation points back to it.

## Figures — mathematical constructions, not invented results

- \`Assets/07-transform-family.svg\`: one original asymmetric house under identity, translation, shear and reflection, including explicitly specified determinants.
- \`Assets/08-transform-order.svg\`: identical input point (1,0), first rotation/translation ending at (2,1) versus reverse order ending at (0,3), with a single world origin and common scale.
- \`Assets/09-normals-under-shear.svg\`: tangent (0,1) / normal (1,0) before shear; transformed tangent (1,1); naively transformed normal (1,0) with nonzero dot product; correct inverse-transpose normal (1,-1) perpendicular to the new tangent.

The diagrams have semantic title and description elements, and the manuscript includes them beside their first conceptual use. They are original vector assets, not screenshots from a third-party playlist. Final PDF/EPUB page design and visible rendering have **not** been approved.

## New standard C++23 sources

**\`include/book03/affine2.hpp\`:** documented column-vector algebra with named 2D affine coefficients, 3×3 row-major *array export* clearly distinguished from algebraic convention; constructors for translation/rotation/scale/shear/reflection; point and vector transform; composition by \`operator*\`; pivot operation; determinant; inverse; and inverse-transpose normal. It can interoperate with the P02 \`Frame2\` in the subset accepted by both numerical contracts. Its inverse policy is explicitly a conservative, scale-aware relative-determinant guard, **not** a universal condition-number certificate.

**\`tests/affine2_tests.cpp\`:** exact scalar examples; independent \`Frame2\` comparisons; 3×3 storage convention; T*R order; pivot; hand-calculated inverse; triangle-area scaling and barycentric invariance through the P03 \`Triangle2\`; a sheared tangent/normal perpendicularity check; singular, nonfinite and overflowing cases; and 2,000 deterministic seeded transform/inverse/direction invariance cases.

**\`src/transform_studio.cpp\`:** \`book03_transforms inspect|svg|ppm|order|repl\`, output of numeric matrices and transformed points, generated SVG/PPM using P01–P03 contracts, a two-orders SVG and a terminal playground that pre-multiplies each requested operation in world space. The object-local house geometry stays immutable. Deliberate finite viewport bounds are checked before export; full clipping is for Chapter 17.

**Build and quality gates:** CMake registers \`book03_transforms\`, \`book03_affine_tests\` and CTest \`book03_affine_transform_contracts\` alongside P01–P03. The Book 03 workflow has newly authored steps to run the numerical tests, exercise the transform REPL, inspect PPM dimensions, parse the SVG output and check one exact transform-order example by numeric tolerance. The editorial gate also includes Chapter 04 and the three new figures, making the cumulative first-four-chapter source inventory 4 long drafts and 9 diagrams.

## Original source custody

\`PASS04_LEGACY_AUDIT.md\` records actual inspected opening spans and blob identities for legacy Computing Foundations Chapters 18, 19, 32 and 37 and Graphics/Vision Chapter 13, including the active/passive basis-change example and the distinction between affine homogeneous 2D coordinates and later projective clip-space coordinates.

The original continuous masters, reading copies, LaTeX Stage 03 generation, publication manifests and both finished Books 1 and 2 were not edited. The new material stays inside the previously approved Book03 notes and code folders plus the dedicated Book 03 CI workflow. A final \`main...\`branch Git comparison should confirm this for each pass.

## Validation state — distinguish code written from code executed

- The chapter, figures, C++23 source, CMake registrations, tests and workflow **have been authored**.
- Toolchain compilation, CTest output, GitHub Actions success, and emitted SVG/PPM visual inspection **have not yet been independently confirmed** in this record. Do not turn declared tests into a green status without observed runner evidence.
- This pass did not compile the book PDF or EPUB, alter the canonical publication layout, or claim a finalized reader.
- Full source-unit (not merely chapter-opening) reconciliation of the original long manuscripts, math/code proofing and publication layout review remain outstanding.

The user has requested to run the *single complete book* Mac compilation and review only once the manuscript is ready. Do not ask them to run small intermediate book builds.

## Where Chapter 05 should take the story

**The Triangle That Almost Wasn't There** should be a genuinely deep numerical geometry chapter, not another superficial tour. Use original robust-predicate sources, plus exact integer counterexamples and carefully constructed binary64 cancellation cases, then develop a certified adaptive or exact sign strategy with honest input-domain/representation guarantees. Connect geometry decisions to triangle validity, mesh topology and raster edge ownership without prematurely claiming a GPU pipeline or destroying the original deeper source.

Continue on the same branch and preserve the same directory structure.
