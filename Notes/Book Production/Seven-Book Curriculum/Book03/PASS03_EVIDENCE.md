# Book 03 — Pass 03 authoring and verification evidence

**Series:** From Bits to Research  
**Book:** 03 — From Geometry to Images  
**Authoring date:** 23 November 2026  
**Branch:** curriculum/book03-geometry-images-pass01  
**Source directory contract:** Notes/Book Production/Seven-Book Curriculum/Book03/  
**Companion code:** Code Parts/tools/labs/book03_cpp23/  
**Publication library:** From Bits to Research/ (local; unchanged and Git-ignored)  
**State:** editorial source and test code written; no whole-book publication compile.

## Completed authoring, not a placeholder

The new chapter, \`Chapters/03 - Shapes That Know Their Borders.md\`, develops the paper-house roof narrative into a genuinely geometric triangle. It contains roughly 5,000 words, derives the 2D signed orientation determinant and its antisymmetry, area and winding, the three barycentric signed-area ratios, an exactly worked 6×4 right triangle, affine reconstruction, boundary/outside cases, interpolated values, degeneracy, numerical failure, frame-flipping, and tests of conceptual transfer. The chapter deliberately explains why a basic double-precision determinant is not the same as a robust orientation predicate, and why a correct algorithm should sometimes refuse a boundary certification.

The original scientific source material in Notes/Continuous Notes/ and Notes/Chapter Edition/ has not been shortened or replaced by this friendly introduction.

## Original teaching figures

- \`Assets/05-the-directed-edge.svg\` — coordinates and determinant signs for left and right of A→B in Cartesian world-up coordinates. Explicit world-frame assumptions, alt text, title and description.
- \`Assets/06-barycentric-roof.svg\` — the full A(0,0), B(6,0), C(0,4), P(1.5,1) construction with weights (1/2,1/4,1/4) and an outside point. The area and point coordinates are mathematical constructions, not invented measurements.

Both vector files sit in the Book03 notes-assets directory next to Chapters 01–02's four previous figures. Final print/EPUB legibility and actual display must still be reviewed.

## The P03 C++23 implementation

**Numerical library \`include/book03/triangle_geometry.hpp\`:** exact integer determinant and containment for each coordinate in [-1e6,+1e6], a range whose int64 safety is derived in the source; full left/right/winding/edge/inside/outside and degenerate cases. A separate floating Triangle2 API returns signed twice-area, barycentric weights, interpolation, and a deliberately **uncertified** near-boundary/near-degenerate status. The guard is educational; it is not a rigorous adaptive predicate.

**Independent tests \`tests/triangle_geometry_tests.cpp\`:** reference area computed by a second shoelace formula; seeded 4,000-case property sweep; winding antisymmetry; translation invariance; reflected sign; barycentric weight recovery; interpolation; affine invariance; exact integer edge classification; zero-area/ill-conditioned geometry and malformed/nonfinite input. Verification is only a *declared test contract* until real compiler execution is observed.

**Executable \`src/triangle_studio.cpp\`:** numeric inspection of the standard world-up right triangle, a P6 barycentric color gradient using P01 Canvas in image-v-down coordinates, an explanatory static SVG, a terminal triangle REPL, and \`save\` for an SVG based on the **current reader-edited triangle and query**. The image mixing is of example RGB channel numbers, not a physical light or colorimetric proof.

**Build/CI:** Existing CMake project gained \`book03_triangles\`, \`book03_triangle_tests\` and \`book03_triangle_geometry_contracts\` CTest. The Book 03 GitHub Actions workflow now checks triangle numeric outputs, PPM dimensions/payload, SVG parseability and the live REPL-generated SVG, as well as the prior P01/P02 facilities. An implemented workflow is not an observed successful run; actual CI result remains to be read.

## Legacy manuscript audit

\`PASS03_LEGACY_AUDIT.md\` records the inspected opening spans, exact Git blob identities, existing source-header span IDs, major unique explanations and target reader chapters for robust predicates, mesh topology, ray–triangle intersection, rasterization, classification and per-corner mesh attributes.

**Open limitation:** these are targeted source openings, not a full-body, paragraph-by-paragraph source audit. The final Book 03 composition must reconcile and preserve the original substantial source, its equations/figures/code and references. Do not interpret the newly drafted introductory chapters as the final complete multi-thousand-page book.

## Acceptance gates not yet passed

- No native macOS compilation, full C++23 compiler/test logs or independently confirmed successful workflow run have been recorded here. The source/test assertions have been reviewed, not executed in a verified target toolchain.
- No authored Figure 3.1/3.2 has yet undergone final e-reader and print-page typography inspection. Geometry schematics have semantic descriptions and known numeric inputs; the reader layout remains pending.
- No full Book 03 EPUB, complete PDF, staged manuscript composition, final author cover, EPUBCheck or KDP preview was produced.
- All full original source content beyond the inspected entry spans still needs a finer-grained integration map.

## Next round: Chapter 04 — Move the World Without Losing It

Work in the **same branch** and existing source/code folders. Chapter 04 must develop complete affine transformations, composition order, homogeneous 3×3 matrices, inverses, negative determinants/reflections, shears and scale, point/vector transform boundaries and deliberate numerical failure. Build a true transform laboratory using P02 and P03, with C++23 numerical oracles, meaningful SVG + raster examples and continuity with the moved paper house. Do not repeat Book 2's entire mathematics text, nor jump into Book 6's Vulkan/Metal implementations.

Prioritize the final legacy-source merge plan early enough that the original deep chapters are not crowded out by new introductory material. The author will perform the one complete PDF/EPUB Mac compile and visual review when the **whole book** is ready; do not ask for per-chapter builds.
