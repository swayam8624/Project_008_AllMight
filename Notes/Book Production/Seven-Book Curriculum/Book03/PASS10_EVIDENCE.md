# Book 03 — Pass 10 evidence and acceptance gates

**Series:** From Bits to Research  
**Volume:** Book 03 — From Geometry to Images  
**Reader chapter:** 10 — A Mesh You Can Trust  
**Branch:** \`curriculum/book03-geometry-images-pass01\`  
**Date:** 24 November 2026  
**Nature of record:** Additive GitHub source authoring plus observed source-preservation audits. Writing test code/CI steps is not equivalent to running them.

## Manuscript delivered

\`Chapters/10 - A Mesh You Can Trust.md\` is a long original reader draft of approximately **8,500 words**. It continues the original paper-house/garden narrative with an unfamiliar wooden dragon OBJ asset and develops:

1. Four separate trust layers: syntax, references, topology and downstream geometric/semantic suitability.
2. Wavefront OBJ position, texture and normal syntax, positive one-based and negative relative indexing, stream-specific index domains and full numeric-token and bounded-memory validation.
3. Per-face-corner UV/normal identities versus geometric/topological vertex identity, with a checkable **4 geometric vertices / 6 GPU tuples** connected two-triangle quad witness.
4. Face cross-product normals, equal/area/angle-weighted smoothing, hard normal boundaries and deliberate failure of naive averaging across bow-tie fans or cylinder cap rims; UV tangent Jacobian singularity and handedness.
5. A **strictly convex planar quad** as an accepted triangulation policy, with explicit rejection of concave, self-crossing, warped and unsupported N-gons rather than inventing a correct triangulation.
6. Mesh diagnosis and an evidence-carrying repair transaction without pretending to infer artist intent or guarantee global embedding from Euler alone.
7. Finite AABB and separation of unknown units, axis conventions, and semantic scene transforms from parser correctness.
8. glTF buffer / bufferView / accessor / primitive hierarchy; independent offset+stride address derivation; a 256-byte fixture with first element at byte 24 and final exclusive byte 132; safe bounds reasoning that checks division limits before count-by-stride multiplication.
9. Context-dependent glTF alignment, normalized restrictions, sparse accessors, nodes, TRS composition and serialized column-major versus internal row-major storage conventions, **without claiming a full glTF loader exists**.
10. Untrusted-file and external-resource safety, source-derived native Asset Trust Studio commands, numerous independent falsification questions and selected numeric answers.
11. A Chapter 11 bridge into asset identity, hierarchy, coordinate frames and scene transform graphs.

The chapter references the actual old mesh-topology/attribute notes and the authoritative official Khronos glTF specification and Library of Congress OBJ source, and states the code's subset.

## Original six source-controlled SVG figures

The following NEW assets were committed under \`Notes/Book Production/Seven-Book Curriculum/Book03/Assets/\`, each containing a title/description and an actual chapter embedding:

- \`34-four-layers-of-mesh-trust.svg\` — separate syntax, references, topology, application-suitability assurances.
- \`35-one-position-many-rendering-tuples.svg\` — four geometric square IDs and six distinct GPU corner tuples without tearing the logical surface.
- \`36-convex-quad-not-all-polygons.svg\` — safe restricted convex triangulation versus concave and warped counterexamples.
- \`37-gltf-buffer-view-accessor.svg\` — literal byte offsets, element starts, end-exclusive last byte of the official glTF address example.
- \`38-asset-studio-trust-witnesses.svg\` — identical-looking square triangles with opposite versus conflicting shared-edge directions.
- \`39-deliberate-repair-transaction.svg\` — preserving original input, diagnosing, selecting intent, editing an explicit copy and revalidating.

This brings the cumulative Book 03 **new original figure source count to 39**. Authored SVG correctness and semantic metadata need subsequent rendered page and e-reader proof; these source figures are not already promoted into an EPUB/PDF.

## C++23 implementation actually added

\`Code Parts/tools/labs/book03_cpp23/include/book03/asset_import.hpp\` supplies:
- \`ObjInputError\` with source line context and \`ObjImportLimits\` for lines, characters, positions, UVs, normals and emitted triangle faces.
- Bounded \`obj_bounded_line\` that rejects overlong lines **while consuming bytes**, before a giant \`std::string\` can grow.
- Strict fully consumed finite \`std::from_chars\` decimals and checked signed OBJ positive/negative indices, each relative to the correct *current* source stream.
- \`ObjCornerKey\` with independent optional UV/normal records (missing attributes are not fabricated).
- \`ObjAsset\` with separate \`topological_mesh()\` (position IDs only) and \`to_rendering_mesh()\` (unique complete position/UV/normal tuple IDs).
- A limited **triangle and strictly convex planar quad** OBJ parser. The quad path explicitly tests distinct position IDs, near-planarity and consistent strictly convex turns after dominant-plane projection. Unsupported OBJ N-gons, colors, coordinates and freeform directives are rejected. \`o/g/s/usemtl/mtllib\` are *recognized* and explicitly recorded as ignored warnings; no external file is opened.
- \`inspect_obj_asset\` reusing P07 \`inspect_mesh\` and reporting geometric position count, rendering tuple count, separate triangle count, optional metadata warnings, zero supplied normals and finite AABB.
- \`check_gltf_layout\` for a small supported subset of scalar/vector unsigned-byte, unsigned-short, unsigned-int and float accessors: view boundedness, element-size/stride/count arithmetic, alignment at component granularity, defined stride 4–252 in multiples of four, rejection of normalized FLOAT/UINT32 and sparse accessors.
- The deliberate limitation that glTF JSON/GLB, semantic vertex-attribute alignment, index-vs-vertex usage restrictions, materials, animations and scene graph decoding are **not** implemented.

Two source correctness fixes made during authoring:
1. Quad face-budget comparison was made **underflow-safe** if the maximum face budget was smaller than the number of triangles produced by a quad.
2. Text line-length was constrained in the actual stream-reading loop, not simply checked after an unbounded \`std::getline\`.
A third alignment correction rejects normalized \`UNSIGNED_INT\` as well as \`FLOAT\` per glTF 2.0.

\`tests/asset_import_tests.cpp\` adds the independent **eleventh CTest**:
- 4-position, 6-UV, 2-normal source with 2 triangle faces and 6 GPU vertices but 4 topological IDs.
- Positive and negative per-stream index resolution at face time, including later positions that do not retroactively change the earlier face; \`p\`, \`p//n\` absent-attribute cases.
- Convex planar quad -> indexed two-triangle disk; concave, self-intersecting, nonplanar and 5+-corner rejection.
- Repeated-index face, collinear distinct-index face and wrong shared-edge winding diagnostic *after* successful OBJ parsing; ignored metadata counts and zero provided normal record.
- Invalid OBJ zero/forward/out-of-bounds indices, malformed/missing tokens and nonfinite position, line/count/face budget stress.
- An independent glTF 256-byte layout witness plus illegal view extent, accessor overrun, short stride, misalignment, normalized FLOAT, sparse unsupported and overflowed-count adversaries.

\`src/asset_studio.cpp\` adds the native \`book03_assets\` CLI:
- \`demo seam|quad|broken|repeated\` built-in source fixtures.
- \`inspect FILE.obj\` for a real local file, with the studio's 5 MiB file-size gate in addition to parser limits.
- \`svg INPUT.obj OUTPUT.svg\` for actual source-indexed triangle and colored edge topology; projection explicitly is an oblique diagnostic, not a real camera/render.
- \`gpu INPUT.obj OUTPUT.csv\` for actual complete attribute tuple deduplication evidence.
- \`layout\` showing the hand-checkable glTF address witness.
- \`repl\`, with transactional replacement of current asset only after a candidate file passes the supported grammar.

**Project registration:** CMake adds \`book03_asset_import\`, \`book03_assets\`, \`book03_asset_tests\` and \`book03_obj_corner_domains_and_gltf_bounds_contracts\`. The cumulative source now has **ten native C++23 labs and eleven CTests**.

**Dedicated workflow:** the existing Book03 GitHub Actions workflow now specifies CMake/CTest for all labs, then creates its own small OBJ seam fixture, checks actual CLI reports, parses exported SVG and attribute CSV with Python, checks the separate UV/normal tuples, exercises REPL changes and confirms glTF byte offsets. CI workflow source exists; no successful run status is implied by this file.

## Original research/source custody

\`PASS10_LEGACY_AUDIT.md\` contains actual connected-source identities and sections inspected:
- Legacy \`23 - Mesh Topology and Connectivity.md\`, blob \`991dfa264100be10e1df3efc801966ae1d383da1\`, original master lines 57090–59762.
- Legacy \`41 - Per-Vertex and Per-Corner Mesh Attributes.md\`, blob \`ca921d2747a6321fb9bebcde97edb086bb74edb7\`, original master lines 95949–99206.
- Legacy \`02 - Surfaces, Topology and Parameterization.md\`, blob \`41a7b270a4ee80ec12cdd57c8421351565f25ec4\`, original master lines 2204–3118.

The old derived chapter headers contain source-span SHA256 metadata, recorded verbatim but **not** independently recomputed against the complete continuous master. Additional old long-form material (notably global mesh repair, full glTF materials/nodes/skins and advanced tangents) remains to be reconciled; linking a chapter does not mean it has been physically merged into the current new reader.

The official glTF 2.0 specification and public Khronos buffer tutorial were additionally consulted during P10 source authoring to validate the normalized restriction, byte stride rules and two-offset calculation. The newly generated C++23 code and SVG assets are original.

## Verification status

**Source writes observed:** Each newly authored file or modification was acknowledged by the connected GitHub repository. Source reads and crosschecks confirm the chapter, C++23 types, tests and sample figures exist at the intended paths.

**Not observed:** A successful native C++23 compile on AppleClang/GCC/Clang, execution of all eleven CTests, output of the built \`book03_assets\` binary, a green GitHub Actions completion, corpus-scale imported OBJ stress testing, independent complete original-source SHA256 verification, PDF/EPUB composition, rendered final book proof, and KDP publication.

**Deliberately outside scope:** General OBJ N-gon triangulation, MTL loading, complete glTF JSON/GLB parsing, malicious binary fuzzing certification, general self-intersection and watertightness proofs, automatic welding/repair, exact robust geometric predicates in 3D, GPU material execution and full asset provenance/units inference.

## Next authoring pass

**Chapter 11 — The Same Tree in Five Worlds.** Build instance-preserving scene graphs with one shared asset and multiple node transforms; frame composition, cycle detection, parent-child invariants, correct inverse-transpose normal transforms under nonuniform scale and animated world/local pose propagation. Preserve original sources and continue on the same dedicated Book03 branch. The user's one final Mac book compile remains deferred until all 32 chapters and source reconciliation are finished.
