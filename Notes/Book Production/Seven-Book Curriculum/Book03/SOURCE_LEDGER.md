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

The new additive reader chapter is \`Chapters/03 - Shapes That Know Their Borders.md\`; it begins from the paper house's roof and builds left/right orientation, winding, signed area, barycentric mixing, interpolation, degeneracy, numerical uncertainty, affine invariance and the careful boundary between a predicate and a construction.

The new C++23 companion is \`Code Parts/tools/labs/book03_cpp23/include/book03/triangle_geometry.hpp\`, its numerically grounded testing target \`tests/triangle_geometry_tests.cpp\` and its \`src/triangle_studio.cpp\` output/interactive inspector. New source figures \`Assets/05-the-directed-edge.svg\` and \`Assets/06-barycentric-roof.svg\` stay under the existing Book03 notes directory.

Source integration of the old advanced robust-predicate sections in reader Chapter 05 and mesh topology sections in Chapter 07 remains outstanding. No old source may be deleted simply because the new Chapter 03 introduces the subject.
