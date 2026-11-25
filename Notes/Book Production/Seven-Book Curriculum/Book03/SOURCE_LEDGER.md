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


## Pass 11 — hierarchical coordinate frames and shared asset placement

The new reader [Chapter 11](Chapters/11%20-%20The%20Same%20Tree%20in%20Five%20Worlds.md) derives local-to-parent and parent-to-world affine composition, correct transform order, asset-versus-node identity, iterative topological evaluation, world-preserving reparenting, inverse-transpose normals, reflection parity and world bounds. The new C++23 scene forest, studio and independent tests reuse earlier geometry and topology types. See [PASS11_LEGACY_AUDIT.md](PASS11_LEGACY_AUDIT.md) for actual inspected blobs and passages from legacy 41 (per-corner attributes, glTF hierarchy and normals) and legacy 37 (camera transform-chain contracts). Full animation, skinning, camera and imported-scene semantics remain to be integrated. Five new original diagrams bring the total to 44. All original master and derived files remain unchanged. Native C++23 and CI execution are unverified.


## Pass 12 — proper rotations, quaternion interpolation and rigid poses

[Chapter 12](Chapters/12%20-%20Rotations%20Without%20Headaches.md) derives SO(3), Rodrigues' formula, fixed XYZ Euler angles and their lock singularity, Hamilton multiplication, unit quaternion double cover, SLERP/NLERP and SE(3) poses. The actual legacy 41 and 37 reading-copy blobs and targeted passages on quaternion interpolation, animation validation and camera-pose conventions are recorded in [PASS12_LEGACY_AUDIT.md](PASS12_LEGACY_AUDIT.md). The new orientation debugger reuses P11 affine types, exports actual rotation samples and supplies a thirteenth independent CTest. Five original figures bring the total to 49. Full legacy reconciliation, native execution and publication remain unverified; original notes and published files are unchanged.


## Pass 13 — calibrated pinhole, optical coordinate frames and inverse rays

[Chapter 13](Chapters/13%20-%20The%20Tiny%20Hole%20That%20Makes%20a%20Picture.md) derives the forward camera model, calibration, FOV, pixel-center and image-edge conventions, crop/resize, axial versus ray-range depth, world rays and optical-space Jacobian. The actual inspected legacy 32 and 37 source identities and remaining specialized material are recorded in [PASS13_LEGACY_AUDIT.md](PASS13_LEGACY_AUDIT.md). The C++23 pinhole simulator reuses P12 rigid poses and P06 rays, exports real projected landmark source coordinates, and adds the fourteenth authored CTest. Five original figures bring the cumulative count to 54. Native runtime, complete legacy reconciliation and publication are unverified. All original and published source paths remain preserved.


## Pass 14 — homogeneous projection and depth representation

[Chapter 14](Chapters/14%20-%20From%20a%203D%20Point%20to%20a%20Screen%20Address.md) derives calibrated perspective, orthographic matrices, explicit clip membership, NDC, viewport, finite/infinite forward/reverse depth and a controlled scalar depth-storage experiment. [PASS14_LEGACY_AUDIT.md](PASS14_LEGACY_AUDIT.md) records actual inspected legacy 33 and 34 source identities and outstanding primitive clipping, frustum, API and rasterization subjects. The native projection debugger reuses P13 calibration, adds the fifteenth authored CTest and six new figures (60 cumulative). Source-written tests and CI are not observed runtime passes. Original manuscripts and distribution paths remain unchanged.


## Pass 15 — stereo geometry, triangulation and depth sensitivity

[Chapter 15](Chapters/15%20-%20Two%20Eyes%20Looking%20at%20One%20House.md) derives aligned stereo, corrected disparity, local uncertainty, epipolar constraints, essential/fundamental matrices, ideal rectification and general closest-ray reconstruction. [PASS15_LEGACY_AUDIT.md](PASS15_LEGACY_AUDIT.md) records directly located legacy 32 and 35 passages and preserved stereo/XR obligations. The new C++23 laboratory reuses P13 calibration and P12 pose, adding the sixteenth authored CTest and six original diagrams (66 cumulative). No native execution or complete legacy reconciliation is asserted. The original sources and published artifacts remain unchanged.


## Pass 16 — indexed vertex processing and projective interpolation

[Chapter 16](Chapters/16%20-%20The%20Long%20Journey%20of%20One%20Vertex.md) connects P10 rendering tuples and P11–14 affine/camera/projection contracts, retains raw clip payloads, derives reciprocal-w surface interpolation and separate screen-linear depth, and introduces the vertex-stage tracer. [PASS16_LEGACY_AUDIT.md](PASS16_LEGACY_AUDIT.md) records actual inspected legacy 33 and 41 material and remaining work. The seventeenth authored CTest and six new original SVGs (72 cumulative) are registered. The earlier P05 CSV duplicate case label was corrected. No native run or full source reconciliation is asserted. All original and published paths remain unchanged.


## Pass 17 — continuous and homogeneous primitive clipping

[Chapter 17](Chapters/17%20-%20Shapes%20That%20Refuse%20to%20Leave%20the%20Window.md) derives Cohen–Sutherland, Liang–Barsky, Sutherland–Hodgman, six homogeneous planes, generated raw varying payloads, provenance and fan assembly. [PASS17_LEGACY_AUDIT.md](PASS17_LEGACY_AUDIT.md) records inspected legacy 33/34 passages and actual P14/P16 dependencies. Five original figures bring the source count to 77, and the eighteenth authored CTest is registered. The bounded floating clipper does not claim certified watertightness. Native execution and complete legacy reconciliation remain pending. Original and published files are unchanged.


## Pass 18 — deterministic integer sample ownership

[Chapter 18](Chapters/18%20-%20Who%20Owns%20the%20Edge.md) derives the edge function, Y-down top-left convention, subpixel snapping, overflow bound, candidate bounds, incremental evaluation and unbiased barycentrics. [PASS18_LEGACY_AUDIT.md](PASS18_LEGACY_AUDIT.md) records inspected legacy 33/34 passages. Five figures bring the cumulative count to 82. The standalone P18 coverage suite was compiled and passed with GCC 14.2.0 and undefined-behavior sanitization after exact Git blob verification; [PASS18_EVIDENCE.md](PASS18_EVIDENCE.md) records the observed run. Cumulative CMake/CTest, integrated studio and publication remain unverified.


## Pass 19 — opaque visibility and depth storage

[Chapter 19](Chapters/19%20-%20Which%20Surface%20Gets%20Seen.md) derives per-sample opaque visibility, depth quantization and tie policies. [PASS19_LEGACY_AUDIT.md](PASS19_LEGACY_AUDIT.md) maps inspected original sources. [PASS19_EVIDENCE.md](PASS19_EVIDENCE.md) records hash-verified standalone C++23/UBSan test and studio passes, 18,000 seeded submissions, 3,072 independently checked CSV rows and byte-identical winner images. Five figures bring the total to 87. The twentieth cumulative CTest is registered; cumulative build and CI remain unobserved. Originals and release paths are preserved.


## Pass 20 — sampling, aliasing and footprint integration

[Chapter 20](Chapters/20%20-%20A%20Pixel%20Is%20Not%20a%20Tiny%20Square%20of%20Reality.md) derives aliasing, sinc box response, midpoint quadrature and coverage error; it explains reconstruction, SSAA/MSAA, mipmaps and temporal sampling. [PASS20_LEGACY_AUDIT.md](PASS20_LEGACY_AUDIT.md) maps targeted original material. [PASS20_EVIDENCE.md](PASS20_EVIDENCE.md) records actual exact-hash-matched GCC/UBSan runs, 2,000 seeded trials and 960 independently checked exported rows. Five figures bring the cumulative count to 92. The twenty-first CTest is registered. Full pipeline, cumulative CI and publication remain unverified.


## Pass 21 — assembled software renderer

[Chapter 21](Chapters/21%20-%20A%20Whole%20Rasterizer%20We%20Can%20Explain.md) connects the previous additive libraries, with a native-tested opaque screen-space backend and separately gated OBJ/model/clip adapter. [PASS21_LEGACY_AUDIT.md](PASS21_LEGACY_AUDIT.md) records actual inspected API identities. [PASS21_EVIDENCE.md](PASS21_EVIDENCE.md) records GCC/UBSan execution, exact source hashes, 76800 checked pixels (76531 exact color matches and 269 permitted checker-boundary colors), and an explicit DNS blocker for full integration. Three new figures bring the total to 95. Two new CTests bring the authored count to 23. No native full-front-end success or publication is asserted.


## Pass 22 — thin-lens optical image formation

[Chapter 22](Chapters/22%20-%20A%20Lens%20Bends%20the%20Story.md) derives image distance, signed magnification, geometric blur, pupil-ray intersections, acceptable focus limits and hyperfocal distance, with separate diffraction and aberration context. [PASS22_LEGACY_AUDIT.md](PASS22_LEGACY_AUDIT.md) maps original Graphics Vision and XR 03 lines 18–126. [PASS22_EVIDENCE.md](PASS22_EVIDENCE.md) records exact-hash-matched native GCC/UBSan passes, 3,000 configurations and 404 independently checked CSV rows. Five new figures bring the count to 100; twenty-four CTests are registered. Full cumulative integration and publication remain pending.


## Pass 23 — distortion, injectivity and angular projection

[Chapter 23](Chapters/23%20-%20Why%20Straight%20Lines%20Bow.md) develops normalized Brown–Conrady distortion, Jacobians, global separation, damped Newton inversion, image-remapping principles and five angular families. [PASS23_LEGACY_AUDIT.md](PASS23_LEGACY_AUDIT.md) maps targeted original Computing Foundations 35 and Graphics Vision and XR 03 passages. [PASS23_EVIDENCE.md](PASS23_EVIDENCE.md) records exact-hash-matched GCC/UBSan tests, 4,000 seeded point trials, 505 angular round trips and 265 independently verified CSV rows. Five figures bring the total to 105, and the twenty-fifth CTest is registered. Cumulative integration and publication remain pending.


## Pass 24 — physical exposure and sensor measurements

[Chapter 24](Chapters/24%20-%20Light%20Becomes%20a%20Measurement.md) distinguishes radiance, irradiance, radiant exposure, photon/electron counts and ADC codes; derives shot/dark/read noise and saturation contracts. [PASS24_LEGACY_AUDIT.md](PASS24_LEGACY_AUDIT.md) maps the inspected original Computing Foundations 36 passage. [PASS24_EVIDENCE.md](PASS24_EVIDENCE.md) records exact-hash-matched GCC/UBSan runs and 17 independently checked exposure rows. Three figures bring the total to 108, and the twenty-sixth CTest is registered. Cumulative integration and publication remain pending.


## Pass 25 — color and display response

[Chapter 25](Chapters/25%20-%20Colors%20That%20Tell%20the%20Truth.md) derives spectral ambiguity, XYZ/xyY and primary matrices, linear-light encoding contracts, RAW neutral gains and a relative HDR operator. [PASS25_LEGACY_AUDIT.md](PASS25_LEGACY_AUDIT.md) maps targeted source custody and W3C references. [PASS25_EVIDENCE.md](PASS25_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan passes, 10,000 triplets and 729 independent CSV checks. Four figures bring the total to 112, and the twenty-seventh CTest is registered. Cumulative integration and publication remain pending.


## Pass 26 — camera calibration

[Chapter 26](Chapters/26%20-%20Teaching%20a%20Camera%20About%20Itself.md) develops normalized spatial DLT, singular directions, row decomposition, planar homography constraints and geometric refinement principles. [PASS26_LEGACY_AUDIT.md](PASS26_LEGACY_AUDIT.md) records targeted camera source custody and primary references. [PASS26_EVIDENCE.md](PASS26_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan passes, 100 noisy tests, 100 separate studio trials and independent noiseless SVD verification. Four figures bring the total to 116, and the twenty-eighth CTest is registered. Cumulative integration and publication remain pending.


## Pass 27 — epipolar estimation

[Chapter 27](Chapters/27%20-%20Two%20Photographs%2C%20One%20Hidden%20Point.md) expands P15 stereo geometry into normalized eight-point estimation, rank-two correction, Sampson residual, essential-pose hypotheses and scale ambiguity. [PASS27_LEGACY_AUDIT.md](PASS27_LEGACY_AUDIT.md) records inspected reader/code identities and the Hartley reference. [PASS27_EVIDENCE.md](PASS27_EVIDENCE.md) records exact-hash-matched GCC/UBSan runs, 100 studio fits and 5,000 held-out correspondences, plus separate NumPy geometry verification. Four figures bring the count to 120; the twenty-ninth CTest is registered. Cumulative integration and publication remain pending.


## Pass 28 — features and robust correspondence verification

[Chapter 28](Chapters/28%20-%20Finding%20the%20Same%20Corner%20Twice.md) derives the structure tensor, Harris response, patch normalization and RANSAC probability; separates descriptor matching and geometric verification. [PASS28_LEGACY_AUDIT.md](PASS28_LEGACY_AUDIT.md) records targeted source custody and primary references. [PASS28_EVIDENCE.md](PASS28_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan runs, 107 translated image pairs, 50 robust fits and 2500 held-out correspondences. Four figures bring the count to 124; the thirtieth CTest is registered. Cumulative integration and publication remain pending.


## Pass 29 — sparse reconstruction

[Chapter 29](Chapters/29%20-%20Building%20a%20Scene%20from%20Pictures.md) implements calibrated essential-pose recovery, multi-ray triangulation and fixed-pose point refinement; derives joint bundle adjustment and describes incremental view registration. [PASS29_LEGACY_AUDIT.md](PASS29_LEGACY_AUDIT.md) records P27 dependency, chapter plan and primary references. [PASS29_EVIDENCE.md](PASS29_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan execution, 80 point refinements, 100 trials, 10,000 points and independent CSV/geometry checks. Five figures bring the count to 129; the thirty-first CTest is registered. Cumulative integration and publication remain pending.


## Pass 30 — point clouds and surface fusion

[Chapter 30](Chapters/30%20-%20Points%20Become%20Surfaces.md) derives covariance normals, implicit geometry, weighted projective TSDF and support-aware zero crossings; discusses isosurface extraction and sparse storage. [PASS30_LEGACY_AUDIT.md](PASS30_LEGACY_AUDIT.md) records prior reader routes and primary references. [PASS30_EVIDENCE.md](PASS30_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan execution, 100 trials, 21,900 observed crossings and independent CSV checks. Five figures bring the count to 134; the thirty-second CTest is registered. Cumulative integration and publication remain pending.


## Pass 31 — integrated studio

[Chapter 31](Chapters/31%20-%20A%20Studio%20for%20Geometry%20and%20Images.md) connects prior geometry/reconstruction and rendering modules with a controlled synthetic fixture and checked coordinate adapter. [PASS31_LEGACY_AUDIT.md](PASS31_LEGACY_AUDIT.md) records dependency custody. [PASS31_EVIDENCE.md](PASS31_EVIDENCE.md) records actual exact-hash-matched GCC/UBSan execution, 230,400 pixel-pass checks and independent Python export verification. Four figures bring the count to 138; the thirty-third CTest is registered. Full cumulative integration and publication remain pending.


## Pass 32 — final evidence capstone

[Chapter 32](Chapters/32%20-%20If%20It%20Looks%20Right%2C%20Is%20It%20Right.md) integrates numerical oracles, projective ambiguity, derivative checks and paired experiment design. [PASS32_EVIDENCE.md](PASS32_EVIDENCE.md) records exact-hash-matched standalone GCC/UBSan execution and 20,000 point reconstructions; [PASS32_LEGACY_AUDIT.md](PASS32_LEGACY_AUDIT.md) records dependency and chapter inventory custody. Four original figures bring the authoring count to 142; CMake registers the thirty-fourth contract test. All 32 planned chapter headings were fetched and verified individually. Original-source reconciliation and publication acceptance remain open, and no release artifacts have been generated.


## Pass 34 — foundations source-unit inventory and selected reconciliation

[PASS34_FOUNDATIONS_SOURCE_MAP.md](PASS34_FOUNDATIONS_SOURCE_MAP.md) records full-file retrieval, exact blob identities, 18,928 routed body lines, 627 headings, and declared master spans for seven Computing Foundations derived sources (32–38). Routes remain proposals until reviewed against the full destination content. Selected passages were read in detail and incorporated additively into Chapters 14, 17 and 31. [PASS34_EVIDENCE.md](PASS34_EVIDENCE.md) records the independent arithmetic witness and successful cumulative CI at the exact engineering revision. Next: Graphics Vision and XR source inventory and targeted gap reconciliation, followed by final source/notation/figure review. No original source or release artifacts changed.
