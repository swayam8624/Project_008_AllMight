# Book 03 — Pass 10 original-source audit: imported mesh data, corner attributes and binary layouts

**Date:** 24 November 2026  
**Working branch:** \`curriculum/book03-geometry-images-pass01\`  
**Rule:** Never destroy or shorten original long notes or alter publication outputs while developing the additive Book03 reader.

## Source files genuinely inspected in the connected repository

### Legacy 23 — Mesh Topology and Connectivity

**Path:** \`Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md\`  
**Git blob (connected fetch):** \`991dfa264100be10e1df3efc801966ae1d383da1\`  
**Derived header states:** source Graphics master \`Notes/Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md\`, original master lines **57090–59762**, declared source-span SHA256 \`527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8\`. This checksum comes from original header metadata, not a newly recomputed master hash.

Actually inspected reading-copy portions:
- Lines roughly **18–250**: geometry versus vertex identity, directed face corners, canonical undirected edge keys, original 4-vertex/5-edge/2-face square, rendering vertex storage and the important distinction between geometric topology and UV/hard-normal splits.
- **250–540**: half-edge and winged-edge designs, face traversals, twin relations and editing lifetime; still mostly relevant to Chapter 07 and later real mesh-repair operations.
- **545–720**: edge manifoldness versus vertex manifoldness, disconnected bow-tie fans, vertex-link cycle/path criterion, boundary-loop ambiguity and improper geometric welding.
- Original chapter continues to several thousand lines; its full repair, topology, collision and downstream numerical proofs remain in the authoritative manuscript and have **not** been claimed re-integrated merely by this targeted audit.

### Legacy 41 — Per-Vertex and Per-Corner Mesh Attributes

**Path:** \`Notes/Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md\`  
**Git blob (connected fetch):** \`ca921d2747a6321fb9bebcde97edb086bb74edb7\`  
**Derived header states:** Graphics master lines **95949–99206**, declared source-span SHA256 \`c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9\`. Header-declared metadata was recorded verbatim, not independently rehashed.

Actually inspected reading-copy content:
- Roughly **14–175**: per-vertex versus per-corner records, GPU tuple deduplication, correct treatment of hard shading and UV seams, manifoldness and local incident fan conditions.
- **348–679**: flat cross-product normals, smooth/equal, area-weighted and angle-weighted normals; UV Jacobian, tangent calculation, orthogonalization and normal-map handedness; the core correct distinction between shading attributes and original mesh topology.
- **679–815**: OBJ \`v/vt/vn\`, independent indices, one-based positive references, negative relative references, tuple-based rendering deduplication, the danger of naive fan triangulation of concave polygons, ear clipping overview. **This directly informed the P10 limited importer.**
- **816–1100**: glTF buffer -> bufferView -> accessor -> attribute, two offsets and byteStride, component/count and index bounds, node transform hierarchy, storage-vs-multiplication conventions, materials, animation, skins and morph targets. **The P10 executable currently validates only a byte-layout subset of this wider topic.**
- **1100–1200**: morph-target deltas, and honest distinction between theory and actual Kairo/Maveb implementation claims. The original continues to **3277 reading-copy lines**, with substantial advanced material not completely read or merged here.

### Legacy 02 — Surfaces, Topology and Parameterization

**Path:** \`Notes/Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization.md\`  
**Git blob:** \`41a7b270a4ee80ec12cdd57c8421351565f25ec4\`  
**Header-declared master range:** **2204–3118** and checksum \`124ad7fa099f4d4b38f41d50bfa82f53d8cab0b2d962cd14b22be2cb1f69f270\`.

Actually inspected selected reading-copy paragraphs: distinction between surface geometry and connectivity, Euler assumptions, half-edge interfaces, boundary and smoothing issues, UV parameterization and limitations of topology-only evidence. The full original reading copy is not represented by P10 alone.

## Verified public format reference

The actual Khronos **glTF 2.0 specification**, https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html, and its associated bufferView schema/tutorial were independently consulted on 24 November 2026. The normative spec includes:
- element address from bufferView byte offset + accessor byte offset + element stride;
- accessor element length, count, alignment and bounds rules;
- if a bufferView defines byteStride, schema range 4–252 and multiple of 4;
- specific 4-byte vertex attribute alignment requirements and semantic type restrictions;
- \`normalized\` forbidden for \`FLOAT\` and \`UNSIGNED_INT\`, sparse accessors and contextual restrictions.

The official Library of Congress Wavefront OBJ format record https://www.loc.gov/preservation/digital/formats/fdd/fdd000507.shtml and original manuscript records support one-based face indices and separate attribute streams; other publicly documented OBJ readers confirm negative relative indices against the current relevant stream.

No official external C++ library or third-party SVG was copied into the repo. The P10 C++23 implementation is new source describing established file-format behavior in a deliberately restricted subset.

## New reader-to-legacy coverage map

| Original deep-source subject | New Chapter 10 reader treatment and testable contract | Not yet claimed implemented |
|---|---|---|
| Geometric vertex identity versus per-face-corner attribute domain | Real four-position/six-render-tuple square; separate normal/UV streams, correct OBJ independent indexing | Full arbitrary attribute channels, GPU packed layouts for skinning, morphs, multiple UV sets |
| OBJ positive/negative references | Signed resolution at face read time for each separate stream; zero/out-of-current-bounds rejection | Full multiobject OBJ/MTL renderer, extended freeform geometry, vertex colors |
| Source vertex records and resource control | Strict numeric token consumption, bounded line reads, index and face counts | Comprehensive audit against memory exhaustion on all platform/stream failure modes |
| OBJ polygon triangulation | Triangles and strictly convex planar quads only; 3D plane and dominant-axis projected turn tests | General robust ear clipping, nonplanar N-gon policy, exact robust orientation predicates |
| Normals and hard seams | Flat / area / angle normal equations, zero-normal audit, distinct normal corner tuples | Production normal and MikkTSpace tangent generator with mirrored chart details |
| Bounds and scale | Finite AABB, unknown unit/axis contracts and explicit semantic warnings | Unit metadata inference, complete object/scene bounds, oriented bounding boxes |
| Topological validity | Reuse P07 independent edge/winding/vertex fan diagnostics without editing source | General mesh repair, guaranteed global orientability, self-intersection-free solid or collision suitability |
| glTF byte layout | C++23 overflow-safe checked bufferView/accessor offsets, stride, scalar/vector components, restricted alignment | JSON/GLB decoding, sparse accessor decoding, attribute-semantic 4-byte alignment proofs, all glTF types and extensions |
| glTF nodes, transform semantics | TRS parent-child composition explanation and row/column storage distinction | Full scene graph, materials, textures, animation, skinning, morph targets, glTF importer |
| Repair workflow | Preserve original, diagnose, specify policy, repair an explicit copy, revalidate | Automatic repair module, automatic artist-intent deduction, physical or topology certification |

## P10 produced source and visual records

- \`Chapters/10 - A Mesh You Can Trust.md\`: new ~8,500-word long-form teaching manuscript with an original dragon-import story, independent byte/index derivations, OBJ/MTL limitations, glTF example, geometric and topology invariants, runtime/project handoff and exercises.
- \`Code Parts/tools/labs/book03_cpp23/include/book03/asset_import.hpp\`: strict bounded OBJ triangle/convex planar quad reader, independent optional corner streams, separated topology and GPU tuple conversion, asset audit, overflow-safe restricted glTF accessor checker.
- \`.../tests/asset_import_tests.cpp\`: separate independent fixture assertions, unsupported-input/adversarial tests, and glTF byte-layout bounds/overflow checks.
- \`.../src/asset_studio.cpp\`: native P10 studio for \`demo\`, \`inspect FILE.obj\`, \`svg\`, \`gpu\`, \`layout\`, and interactive \`repl\`.
- \`Assets/34-four-layers-of-mesh-trust.svg\` through \`Assets/39-deliberate-repair-transaction.svg\`: six newly-authored original diagrams, embedded in the new reader, depicting validation layers, GPU tuple seams, safe quad scope, byte layout, winding failure and explicit repair.

## Limits still requiring real validation

The parsing library handles a specified **subset** of OBJ; unsupported directives reject, and known but ignored metadata directives record warnings. It is not equivalent to Wavefront OBJ 3D asset format coverage. In particular \`mtllib\` is not followed and \`s\` and \`usemtl\` are not applied. It does not automatically repair geometry.

The glTF layout helper accepts only specified component kinds and SCALAR/VEC2/VEC3/VEC4 types. It has no knowledge of a specific accessor's use as index/vertex/animation input, and does not enforce *all* context-dependent glTF requirements. Unsupported sparse data rejects. Float sign and exact topological index checks are not interchangeable.

Native CMake/C++23 compilation, all eleven CTests, GitHub Actions success, final scientific SVG screenshot/print proof, real-world large asset ingestion, complete original source-unit reconciliation and the full Book03 PDF/EPUB build remain **unobserved or pending** at this writing. No source check should be described as proof of runtime correctness.

**Next:** Chapter 11, *The Same Tree in Five Worlds*. Carry stable asset identity into one shared mesh and multiple world instances; parent-child transform graphs, nonuniform scales, normal transforms, cycles, animation and world/local coordinate policies. Reuse the already-authored asset/geometry contracts without modifying published books or original master notes.
