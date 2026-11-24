# Book 03 — Original source inventory and integration ledger

**Checked against the restructured main branch on 23 November 2026.** This file maps known source locations and marks the boundary of what has actually been read. It is NOT a claim that every old chapter has already been audited line by line.

## Canonical existing text that must not be lost

| Source location in Notes/ | Relationship to Book 03 | Current action |
|---|---|---|
| Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md | Extensive math-to-camera, geometry and implementation narrative across previous books | Preserve in full; audit B3 spans and reuse with source anchors |
| Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md | Graphics, optics, geometry, rasterization, imaging, reconstruction and later XR | Preserve in full; route appropriate material into Book 03 without duplicating Book 4/7 depth |
| Continuous Notes/Game Engine - From First Frame to Living Worlds.md | Mesh/camera/renderer applications and engineering failure cases | Cross-reference and extract only where relevant, retaining provenance |
| Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem.md | Existing chapter bridging geometry, frames, projection and reconstruction | Primary legacy Book 03 source |
| Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling.md | Geometry to pixels | Primary legacy Book 03 source |
| Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity.md | Robust predicates and camera sensitivity | Primary legacy Book 03 source |
| Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry.md | Frame and ray derivations | Primary legacy Book 03 source |
| Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field.md | Geometric optics and sensor bridge | Primary Book 03 with Book 4 boundary |
| Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging.md | Engine camera contracts | Primary legacy Book 03 source |
| Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline.md | End-to-end forward pipeline | Primary legacy Book 03 source |
| Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference.md | Forward versus inverse imaging and measurement model | Primary Book 03 introduction support |
| Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md | Shape/mesh representations | Book 03 |
| Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation.md | Optical image formation | Book 03 |
| Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction.md | Image formation/sampling | Book 03; keep transport and AI extensions in owning volumes |
| Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space.md | Projection and clipping | Book 03 |
| Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence.md | Rasterization methods | Book 03 (CPU proof); Book 6 GPU integration |
| Chapter Edition/02 - Graphics Vision and XR/19 - Textures as Sampled Surface Signals.md | Sampling and surface coordinates | Relevant Book 03 bridge |
| Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md | Topological structure | Book 03 |
| Chapter Edition/02 - Graphics Vision and XR/24 - Ray–Triangle Intersection.md | Geometric query | Book 03; light transport applications in Book 4 |
| Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping.md | Geometry and clipping | Book 03 with Book 5 collision separation |
| Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion.md | Mesh/geometry operators | Book 03 and Book 2 numerical connections |
| Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting.md | Camera pose and geometric inference | Book 03; advanced vision in Book 7 |
| Chapter Edition/02 - Graphics Vision and XR/33 - Projection Scale and Incidence Geometry.md | Camera and projective geometry | Book 03 |
| Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md | Correct mesh representation | Book 03 |

The notes above are routing candidates, not a license to move entire derived chapters into this book without reviewing the overlap.

## Existing LaTeX generation

Root: Notes/Book Production/AllMight Codex/LaTeX/

- CURRENT.json selects **.generation-n3azv4es** and contains an existing Stage 03 named 03-Geometry-Cameras-and-Image-Formation.
- Generations/.generation-n3azv4es/03-Geometry-Cameras-and-Image-Formation-P01/main.tex points to many source chapter fragments (including 02-foundations-ii-ch032 through ch038, visual-computing material, and other routed source sections). It is **not** the new final book manuscript.
- physical-parts.json may carry a different older generation identifier. Do not silently reconcile or rewrite it during drafting.
- Do not modify active original generated chapters, manifests, source caches, figures or existing PDFs/EPUBs.

## Integration rule for every future revision

Every imported source unit needs its old path and source hash (and ideally exact span), its new Book 03 section, a brief reason for placement, and a test that original source is still present. Rewrite proven errors transparently in the true source; do not “clean up” by dropping valid material. The 32-chapter plan is the **reader-facing architecture**, not permission to shrink the original corpus.

## Round 01 source custody

The Chapter 01 narrative is a new **additive** reader-facing introduction. It intentionally starts before the old camera material: pixels, image addresses, and the first visible algorithm. It is not described as a byte-for-byte rewrite of the long original chapters, and it is not yet automatically injected into the stage 03 LaTeX build. The C++23 Pixel Canvas is new reproducible source under Code Parts/tools/labs/book03_cpp23/.

## Next source audit

Read every full legacy Book 03 source chapter in source order; record source byte hashes, figures/code environment counts, proofs, unique examples, and cross-book boundaries. Then write a source-preserving composition map that links new chapters to existing paragraphs and assets without double-counting content. Do this **before** replacing any stage source or generating reader artifacts.


## Pass 02 — targeted primary-source inspection

The [Pass 02 legacy-source audit](PASS02_LEGACY_AUDIT.md) now records exact inspected Git blob SHAs, canonical source spans, original matrix/camera conventions and Book 03 chapter destinations for seven Computing Foundations chapters (32–38) plus two Graphics/Vision chapters (clip space and mesh topology). This is a **verified entrypoint inventory**, not an exhaustive full-body or mathematical audit. It leaves the originals untouched.

**New chapter:** Chapters/02 - Every Place Needs an Address.md — an additive first-principles explanation of point/vector distinction, the origin and basis in an affine frame, inverse transforms, nested composition, units and 2D world-up/image-down mapping, plus worked arithmetic and tests.

**New independent source:** Code Parts/tools/labs/book03_cpp23/include/book03/frame_math.hpp, src/frame_playground.cpp and tests/frame_math_tests.cpp. The numeric frame code does not pretend to replace the old camera extrinsic, clip-space or rasterization chapters. Its 2D assumptions are explicit.

**New original diagrams:** Assets/03-two-frames-one-place.svg and Assets/04-world-to-image-frame.svg. Both have accessible descriptions and numeric contracts. No legacy diagram or generated asset was moved.

Remaining: full source-unit mapping for each long chapter, math verification, additive reader composition, and author-run final PDF/EPUB review. This pass does not edit source masters or the frozen Stage 03 generation.


## Pass 03 — triangle geometry and original robust predicates

The [Pass 03 legacy-source audit](PASS03_LEGACY_AUDIT.md) records the actually inspected source **openings**, original source-span IDs, precise Git blob SHAs and reader placement for robust orientation, rasterization, ray–triangle intersection, mesh queries, topology and per-corner mesh attributes. Those originals are not yet exhaustively source-unit audited and remain unchanged.

The new additive reader chapter is `Chapters/03 - Shapes That Know Their Borders.md`; it begins from the paper house's roof and builds left/right orientation, winding, signed area, barycentric mixing, interpolation, degeneracy, numerical uncertainty, affine invariance and the careful boundary between a predicate and a construction.

The new C++23 companion is `Code Parts/tools/labs/book03_cpp23/include/book03/triangle_geometry.hpp`, its numerically grounded testing target `tests/triangle_geometry_tests.cpp` and its `src/triangle_studio.cpp` output/interactive inspector. New source figures `Assets/05-the-directed-edge.svg` and `Assets/06-barycentric-roof.svg` stay under the existing Book03 notes directory.

Source integration of the old advanced robust-predicate sections in reader Chapter 05 and mesh topology sections in Chapter 07 remains outstanding. No old source may be deleted simply because the new Chapter 03 introduces the subject.


## Pass 04 — active/passive maps, affine composition and normal transformation

The [Pass 04 legacy-source audit](PASS04_LEGACY_AUDIT.md) records actual inspected opening spans and Git blob identities for original coordinate-basis, frame, camera, light-vector and clip-space chapters. In particular, the original Computing Foundations **Coordinate Frames and Linear Systems** chapter explicitly distinguishes active transformation from passive change of basis, with \([v]_C=C^{-1}B[v]_B\) and an original worked \((4,2)\rightarrow(3,-1)\) coordinate example. New reader Chapter 04 introduces the distinction with a paper-house analogy and concrete arithmetic while preserving the original chapter.

The new `Chapters/04 - Move the World Without Losing It.md` develops identity, translation, rotation, nonuniform scale, shear, reflection, affine versus projective homogeneous coordinates, matrix multiplication order, nested frames, pivot transforms, determinant/area/winding changes, inverse and conditioning, and inverse-transpose normals. The discussion uses a declared column-vector algebra and never infers transform direction from matrix memory storage.

The new C++23 companion `Code Parts/tools/labs/book03_cpp23/include/book03/affine2.hpp` provides that affine contract; `src/transform_studio.cpp` uses the earlier chapters' canvas, frame and triangle structures to generate inspectable SVG/PPM outputs and a native command-line playground; `tests/affine2_tests.cpp` adds deterministic scalar/property checks. Original source figures `Assets/07-transform-family.svg`, `08-transform-order.svg` and `09-normals-under-shear.svg` illustrate the concepts and remain inside the unchanged Book03 notes folder.

**Source-integrity boundary:** none of the inspected original full masters, reading copies or LaTeX-generated Stage 03 chapter fragments was rewritten, shortened or moved. The entrypoint audit is not a completed full-body source-unit reconciliation. That deeper step must precede any claim of whole-book completion or release.


## Pass 05 — robust predicate sources require subsection-level routing

The new reader Chapter 05, `Chapters/05 - The Triangle That Almost Wasn't There.md`, develops exact decisions for finite stored IEEE binary64 geometry through a disappearing-triangle story. It adds two independent adversarial scenarios—large representable products whose naive difference rounds to zero despite a true \(-4\) determinant; and the smallest subnormal coordinates whose positive determinant underflows in ordinary binary64 multiplication—plus explicit differentiation of **input rounding** from **arithmetic rounding**.

The new project under `Code Parts/tools/labs/book03_cpp23/` comprises:
- `include/book03/exact_orient2d.hpp` — a novel, independently authored binary64 dyadic big-integer sign oracle; bounded exact-int64 fast path reuses P03. This does not implement Shewchuk expansion arithmetic.
- `tests/exact_orient2d_tests.cpp` — 5,000 deterministic exact half-grid references, extreme and invalid values, topological consistency and hard numeric failure fixtures.
- `src/precision_studio.cpp` — numeric inspector and C++-generated decision-report CSV and SVG, including an interactive query.
- `Assets/10-the-vanishing-triangle.svg`, `11-subnormal-area.svg`, and `12-certified-sign-interval.svg` — original accessible figures, the last explaining *what a certified floating filter would need*, not falsely claiming P05 implements it.

The [Pass 05 source-subsection audit](PASS05_LEGACY_AUDIT.md) inspects the original long Computing Foundations/34 source **beyond the first robust-predicate section**. Its subsequent material covers finite camera frusta, corner derivations, inverse view-projection, zero-to-one depth, reversed-Z, infinity, planes and culling. Preserve and route these substantial original sections to Books 03 Chapters 13–17 and downstream rendering chapters; never discard them because the new numerical predicate chapter reuses the original heading's first subject.

The actual legacy full bodies and their source-span hashes remain to be verified and source-unit reconciled before Book 03 release. Published Books 1–2, original continuous notes, derived reading copies, LaTeX staging and the `From Bits to Research/` output tree have not been edited.


## Pass 06 — three dimensions with truthful query boundaries

The sixth new reader chapter is `Chapters/06 - Step Out of the Page.md`. It begins with a paper house acquiring real depth and develops typed 3D positions/displacements, right-handed axes, dot/cross products, winding and true face normals, planes and normalized signed distances, line/ray/segment parameter domains, ray-plane and sphere intersections, conditioning and an explicit oblique **non-camera** visualization map.

New C++23 companion files are `Code Parts/tools/labs/book03_cpp23/include/book03/geometry3d.hpp`, `tests/geometry3d_tests.cpp` and `src/spatial_studio.cpp`. The 3D module is cumulative with the earlier point/vector and geometric-predicate chapters, but **does not claim to inherit Chapter 05's exact 2D orientation guarantees**. It handles near-parallel rays and near-tangent spheres with named uncertainty statuses under a stated conservative *heuristic* rule. The tests embed a planar XY triangle into 3D and compare its cross-product Z component to the earlier exact bounded-integer 2D twice-area to expose sign mistakes.

Four new original source figures are `Assets/13-right-handed-space.svg`, `14-3d-cross-normal.svg`, `15-ray-plane-cases.svg` and `16-house-leaves-the-page.svg`. The scene SVG generated by `book03_space` projects a genuine 3D house, surface normal and sphere rings using a declared **parallel oblique map**, and prints the actual ray-plane/sphere result computed by the library. It is not a pinhole-camera rendering, calibrated photogrammetry or measured performance chart.

The [Pass 06 legacy-source audit](PASS06_LEGACY_AUDIT.md) records the actually inspected original-source openings with exact Git blob SHAs. In particular, the historic Ray–Triangle Intersection chapter derives infinite supporting-plane intersection **followed by barycentric containment** and then a more efficient Möller–Trumbore formulation; the Mesh Queries chapter distinguishes primitive narrowphase from a million-triangle scene query and acceleration structure; the camera foundation declares right-handed \(-Z\) view conventions. Keep these full long sections for later complete reader chapters instead of replacing them with the introductory ray-plane and oblique diagram code.

The authoritative continuous notes, generated chapter edition, TeX stage outputs, physical-part manifests and already published Books 1–2 have not been edited. Final source-unit mapping and verified full-book content preservation remain open.


## Pass 07 — mesh connectivity, halfedge twins, CSR and graph operators

Reader Chapter 07, `Chapters/07 - One Shape, Many Triangles.md`, is a new substantial narrative from cardboard/glue identity to indexed geometry, directed edge keys, boundary, winding and nonmanifold incidence, bow-tie vertices, symmetric halfedge twins and face cycles, Euler with its topological assumptions, disconnected identical-coordinate seams, per-face-corner attributes, sparse CSR adjacency and an independently derived uniform graph Laplacian.

The cumulative native C++23 lab adds `include/book03/mesh_topology.hpp`, `include/book03/halfedge_mesh.hpp`, `tests/mesh_topology_tests.cpp`, `tests/halfedge_mesh_tests.cpp` and `src/mesh_studio.cpp`. Their domain is **exact combinatorial index connectivity** with deliberate warnings for floating geometric zero-area uncertainty, not a general mesh-repair engine. A closed house with 10 indexed positions, 24 unique undirected edges and 16 faces is a checkable reference. Separate defective fixtures include a two-component invisible seam, a flipped face, three-face edge, disconnected vertex fans, repeated-index and collinear geometry. The per-corner UV model avoids conflating a GPU rendering vertex with a geometric/topological vertex. The static CSR plus scalar uniform graph Laplacian illustrates why wrong adjacency changes the numerical operator, without claiming to implement cotangent-weighted differential geometry.

Five original technical figures are `Assets/17-indexed-quad-adjacency.svg`, `18-coincident-positions-separate-vertices.svg`, `19-nonmanifold-edge-and-bowtie.svg`, `20-closed-indexed-house.svg` and `21-halfedge-twins-and-cycles.svg`, all embedded into the manuscript.

The [Pass 07 original-source audit](PASS07_LEGACY_AUDIT.md) directly inspects the source chapter `Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md` (blob `991dfa264100be10e1df3efc801966ae1d383da1`, declared original source master lines 57090–59762). It routes material on half-edge and winged-edge, indexed rendering versus topological identity, manifold vertex links, boundary loops, face/vertex adjacency, CSR, graph Laplacian and numeric degeneracy into their right reader locations. **The old long source body remains unmodified**, and its full subsection-by-subsection source-unit reconciliation, including uninspected later material, is explicitly pending. GPU vertex caches belong to the later real-time volume; cotangent-weighted surface operators, full topological repair, patch continuity and physically meaningful collision filtering require their own deeper treatment.

No original continuous master, derived chapter-edition file, LaTeX stage, published Book 1/2 file or local `From Bits to Research/` EPUB/PDF publication file was touched. Neither the authored tests nor the source ledger establishes an observed successful compile.


## Pass 08 — cubic curves, splines and tensor-product patches

The new manuscript `Chapters/08 - Drawing Smooth Things with Control Points.md` completes the first curve-and-surface reader milestone. It develops cubic Bézier interpolation with actual de Casteljau steps; an independent Bernstein expression; first/second derivatives and a curvature regularity warning; cubic blossom symmetry/affinity; subdivision and parameter maps; \(C^0/G^1/C^1/C^2\) joins; Hermite tangent constraints; uniform Catmull–Rom interpolation as a Hermite variant; a nonrational uniform cubic B-spline span and correct Bézier conversion; knot and rational/NURBS limitations; a mathematically conservative finite-chord control-hull criterion with honest floating and budget limits; and sixteen-control bicubic tensor-product patches evaluated into **one indexed P07-compatible triangle mesh**. It contains numerous ordinary-language stories, hand calculations and falsification exercises instead of a disconnected formulas list.

The existing original source was **actually located** through `Notes/Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map.md` and source atlas `36 - Source Atlas and Reconstruction Checkpoints.md`, not presumed absent:
- `Notes/Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference.md`, blob `c727c2f278bd852fc194dabba9ab77fa30940b37`, original Graphics master lines 1–2203. Its curves section at reading-copy lines approximately 1927–2215 has Hermite/Bernstein matrices, blossoms, Bézier subdivisions, Catmull–Rom, B-splines and rational/sampling content.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md`, blob `41a7b270a4ee80ec12cdd57c8421351565f25ec4`, original master lines 2204–3118. It contains tensor-product cubic patch formulae, partial derivatives and normal, boundary continuity and the alternative subdivision/implicit geometries.
- The source atlas `36 - Source Atlas and Reconstruction Checkpoints.md`, blob `6dc8d20a7aaf867a6bb6bb5d835ffe246c6aaadb`, maps **MITG-02 — Cubic curves**, **MITG-03 — Curves and surfaces**, P3-13 and P3-20 camera motion into those original chapters. Preserve its source-video links, captions provenance, dates and lecture-specific examples.

The full detailed mapping, range metadata, remaining unique material and limitations of the original source-span hash checks are documented in [PASS08_LEGACY_AUDIT.md](PASS08_LEGACY_AUDIT.md). The long continuous master returned an empty payload through the connected fetch route, so its declared source-span SHA256 values remain **unverified**. Existing chapter-edition contents and source atlas were readable and remain unmodified.

P08 adds `Code Parts/tools/labs/book03_cpp23/include/book03/curves3d.hpp`, `src/curve_studio.cpp`, `tests/curves3d_tests.cpp`, registers a ninth CTest, and adds a reader-editable SVG/CSV/indexed-OBJ export studio. These source outputs are *not* a final PDF/EPUB build. Six new original figures appear in `Assets/22-de-casteljau-levels.svg` through `Assets/27-bicubic-patch-grid.svg`, for **27 total** source-controlled teaching figures.

**Not claimed:** generic NURBS/knot-vector editing, exact rational circles in the code, screen-space error certification, trimmed/self-intersection-free CAD, multi-patch watertightness, certified binary64 spatial geometry, executed green CI, or final series source reconciliation. The mathematical introductory chapter teaches several of these distinctions; the pure C++23 reference intentionally implements a limited, testable subset.

No original `Notes/Continuous Notes/`, `Notes/Chapter Edition/`, legacy LaTeX/build file, published Books 01–02 or the final `From Bits to Research/` EPUB/PDF publication tree was changed by this authoring pass.


## Pass 09 — procedural surface rules, analytic normals and extracted level sets

The new reader `Chapters/09 - Making a World from Rules.md` turns the paper-house scene into a real small procedural world. It distinguishes parametric maps, implicit zero sets and finite indexed mesh approximations; derives a heightfield with analytic partial-derivative normals; builds a modulo-wrapped capped cylinder and pole-safe latitude/longitude sphere with the complete \(V/E/F\) and Euler proofs; then derives sphere signed distance and gradient and walks through strict-sign marching tetrahedra, lattice-edge interpolation, global index caching and the limits of geometric approximation. The reader includes complete native C++23 lab instructions and explicit exercises with numeric solutions.

The actual original-source paths inspected and mapped are:
- `Notes/Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md`, blob `41a7b270a4ee80ec12cdd57c8421351565f25ec4`, declared original Graphics master lines 2204–3118 and header-recorded source-span hash. It includes parameterized vs implicit surfaces, normals, signed distance, surface topology, subdivision and operators.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations.md`, blob `50083685d85e449e0d205bc728903a0cbdc03bac`, declared original Graphics master lines 78561–79471. It includes explicit/implicit/learned shape representation comparisons, SDFs, sampling and neural field limitations.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints.md`, blob `6dc8d20a7aaf867a6bb6bb5d835ffe246c6aaadb`, records CMU geometry/manifold/geometric-query lectures, MIT patch material and learned 3D source provenance.

The [Pass 09 source custody audit](PASS09_LEGACY_AUDIT.md) records exact reading-copy source identities and inspected passages, while avoiding the false claim that a continuous master's declared source-span hash was independently recomputed or that the older thousands of pages are already fully merged. Full original proofs and specialized representation work remain for later volume passes. No original continuous or derived manuscript was edited.

Cumulative C++23 P09 files are `include/book03/procedural_geometry.hpp`, `tests/procedural_geometry_tests.cpp` and `src/procedural_studio.cpp` under `Code Parts/tools/labs/book03_cpp23/`. The same `Point3` and `Mesh3` types and P07 topology checks are reused. The code implements a finite, bounded, sphere-specific marching-tetra extractor with six fixed tetra per cubic lattice cell and a canonical sorted lattice-edge key cache. It does **not** claim a generic exact CAD/NURBS or general occupancy/learned-field mesher. Its numerical policy explicitly rejects exact zero grid sample ties, insufficient sampled inside/outside evidence and invalid bounds rather than silently manufacturing apparently watertight geometry. The C++23 tests authored for P09 bring the cumulative source to **nine labs and ten CTests**.

Six new vector teaching figures are `Assets/28-three-ways-to-describe-a-shape.svg` through `Assets/33-procedural-representations-and-topology.svg` (**33 original Book03 SVG figures total**), each sourced from the correct math/representation and embedded in Chapter 09. Their authored existence is not a substitute for final rendered proof.

**Still outstanding:** actual native C++23 compilation/CTest/CI green observation, cross-platform floating parity, final book production previews and full original source-unit reconciliation. No released EPUB/PDF and no published Books 01–02 or legacy LaTeX files have been modified.


## Pass 10 — trustworthy imported mesh data, OBJ corner domains and glTF binary layout

New reader `Chapters/10 - A Mesh You Can Trust.md` is a long original paper-house/dragon storyline that distinguishes **grammar**, **referential validity**, **topology** and downstream **geometric/application suitability**. It derives Wavefront OBJ one-based and negative reference rules; explains separate per-corner position/UV/normal streams, noninterference between logical indexed topology and GPU rendering tuple deduplication; derives flat, area/angle smooth normals and UV tangent conditioning; refuses to silently fan-triangulate concave or warped polygons; explains AABB, units, explicit repairs and the glTF buffer/bufferView/accessor chain with overflow-aware offset/stride math and contextual scene semantics.

P10 **actually inspected** original reading-copy sources:
- `Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md`, Git blob `991dfa264100be10e1df3efc801966ae1d383da1`, declared original Graphics master lines 57090–59762, especially per-corner rendering seams, face index incidence, manifold, boundary and bow-tie fan proofs.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md`, Git blob `ca921d2747a6321fb9bebcde97edb086bb74edb7`, original Graphics master lines 95949–99206, including per-corner GPU identity, normal weighting, tangent UV determinants, OBJ `v/vt/vn`, negative index semantics, convex/fan polygon caveats, glTF buffers/accessors, nodes, skin/morph and engine data layout.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md`, blob `41a7b270a4ee80ec12cdd57c8421351565f25ec4`, original master lines 2204–3118, differential/parametric geometry and topology/repair background.

More detailed actual ranges, identified legacy gaps and the limits of header-declared source-span hashes are in [PASS10_LEGACY_AUDIT.md](PASS10_LEGACY_AUDIT.md). **No** complete original continuous master checksum or comprehensive source unit reconciliation is claimed. The original long-form source passages remain intact.

P10 source implements `Code Parts/tools/labs/book03_cpp23/include/book03/asset_import.hpp`: a deliberately strict **bounded OBJ triangle/strictly convex planar quad subset** with optional independent UV and normal references and position-only topological mesh graph; unique GPU rendering vertex tuples, source warnings/diagnostics and a restricted overflow-safe glTF 2.0 accessor/bufferView **layout checker** (no GLB/JSON loading). The teaching terminal `src/asset_studio.cpp` can inspect actual small OBJ files, output annotated topological SVG and GPU tuple CSV, demonstrate an exact byte offset witness and provide transactional REPL fixture switching. `tests/asset_import_tests.cpp` adds the independent **eleventh CTest across ten cumulative C++23 labs**, including malformed tokens, source bounds, negative OBJ IDs, four geometric/six render vertices, strict quad cases, bad topology after grammatical success, glTF alignment/count/stride/overflow and explicit unsupported sparse handling. CMake and Book03 CI source are updated; **CI success has not been observed**.

New original mathematical figures `Assets/34-four-layers-of-mesh-trust.svg`, `35-one-position-many-rendering-tuples.svg`, `36-convex-quad-not-all-polygons.svg`, `37-gltf-buffer-view-accessor.svg`, `38-asset-studio-trust-witnesses.svg`, `39-deliberate-repair-transaction.svg`, all embedded in Chapter 10; the Book03 figure source total is **39**. They require later rendered print/e-reader proof.

External normative format material checked: Khronos glTF 2.0 official spec https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html and Khronos buffer tutorial; Library of Congress Wavefront OBJ description https://www.loc.gov/preservation/digital/formats/fdd/fdd000507.shtml. Original C++23 source and SVG plates are authored from mathematical/format rules rather than copied third-party code or artwork.

**Future preservation obligations:** full old OBJ/glTF material/skin/animation coverage, actual scene hierarchy and data ownership, full source-unit reconciliation, robust 3D predicates, topological/geometric repair and screen-level validation. This chapter does **not** claim full glTF parsing, MTL resolution, universal OBJ polygon support, exact CAD or a tested runtime deliverable. It did not alter any continuous notes, generated chapter editions, legacy LaTeX stage, already published Books 01–02 or `From Bits to Research/` publication library.
