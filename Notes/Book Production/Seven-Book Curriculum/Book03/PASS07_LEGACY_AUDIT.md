# Book 03 — Pass 07 original-source audit: mesh topology, adjacency and its operators

**Date:** 23 November 2026  
**Working branch:** \`curriculum/book03-geometry-images-pass01\`  
**Intent:** Preserve substantial original long-form mesh and numerical content while adding a genuinely teachable narrative and runnable C++23 reference. This is **not** a declaration that all original thousands of lines were copied into the final reader.

## Directly inspected original source

The existing manuscript is:

\`Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md\`

**Git blob SHA at inspection:** \`991dfa264100be10e1df3efc801966ae1d383da1\`.

Its own derived chapter header declares the authoritative graphics master:

\`Notes/Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md\`

with **original master source lines 57090–59762**, header-declared source-span SHA256 \`527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8\`.

That SHA256 is original metadata, *not a newly recomputed hash over the full source master*. Do not treat it as an independently verified complete-body source integrity proof.

**Inspected derived reading-copy ranges:** approximately lines 1–1010, in several successive ranges. These are real contiguous portions that were read and used for decisions in this pass; the **remaining** long chapter body is not yet completely reconciled.

## What original source was actually found and where it belongs

| Original portion | Key material actually present | Book 03 reader strategy |
|---|---|---|
| Opening, Part 1189 | Geometry versus topology; indexed vertex IDs; directed triangle edges; edge key \((\min(i,j),\max(i,j))\); the exact 4-vertex/5-edge/2-face square and \(\chi=1\) | Rebuild this as an understandable connected-cardboard story and reuse the original mathematical instance. Full proof and references should remain available in final composition |
| Part 1190 | Rendering vertices vs conceptual topological vertices; 32-byte record numerical storage example; GPU index buffers and post-transform cache; UV seam and hard normal splits; memory locality | Introduce per-face-corner UV domains and distinguish seam topology from rendering duplication in Chapter 07. Preserve advanced GPU/cache and asset-buffer experiments for Books 03/06 |
| Part 1191 | Indexed and pointer half-edge designs; face-owned directed uses; \`next\`, \`previous\`, \`twin\`; identity, cyclic and reversal invariants; boundary-handling conventions; one-ring rotation; handle lifetime | Explain and independently implement an immutable C++23 half-edge builder in P07 with exact 3-cycle and twin validation; do not pretend it is a full mutable modeling kernel |
| Part 1192 | Winged-edge structure and edge-centred preceding/succeeding connections; trade-offs with half-edge | Keep substantive exposition in final original-source reconciliation; concise cross-reference from Chapter 07 until the complete comparison is integrated |
| Part 1193 | Disk/half-disk manifold local neighborhoods; three-face nonmanifold edge; bow-tie vertex; vertex-link cycle versus path; geometric versus combinatorial manifoldness | P07 tests detect both excess edge incidence and disconnected vertex fans. **Important boundary:** fan connectivity and boundary-degree checks are a deliberately useful necessary diagnostic, not a full verified vertex-link cycle/path implementation for arbitrary pathological complexes |
| M072 Part 1194 | Boundary edges and loops; inability to trace boundary uniquely at a branching vertex; reconstruction-hole candidates | P07 reports boundary **edges** only. It does **not** falsely claim to extract/repair full oriented boundary loops or decide which boundary should be filled. Preserve full treatment for Chapter 10 and research/repair sections |
| Part 1195 | Face-edge adjacency; BFS/DFS and O(F+E) traversal; connected components; physical collision exclusion caveat | P07 implements indexed edge-connected face components with disjoint-set; shares no connectivity by mere XYZ coincidence. Physics contact filtering stays a separate later policy |
| Part 1196 | Vertex one-ring adjacency and CSR storage; graph Laplacian \(L=D-A\); simple smoothing and cotangent-weighted extensions; degree/valence | P07 adds source-first \`VertexCSR\` and a scalar *uniform unnormalized graph Laplacian*, with hand-solvable quad test. The deeper cotangent/Laplace–Beltrami, mesh differential geometry and smoothing belong to later advanced sections and must be preserved |
| Part 1197 opening | Geometric degeneracy, dimension and units, conditioning versus arithmetic rounding error, transform-invariant tests | P07 reports repeated-index faces **separately** from geometrically collinear distinct-index triangles, notes double cross-product limitations, and preserves Chapter 05's exactness claim as **2D only** |

## Newly authored P07 files and their scope

New reader: \`Notes/Book Production/Seven-Book Curriculum/Book03/Chapters/07 - One Shape, Many Triangles.md\`, an extended narrative from the 3D house and cardboard analogy to indexed identity, edge directions, Euler/closed-house counts, disconnected seams, bow-tie vertex fans, nonmanifold edge incidence, half-edge navigation and symmetric twins, per-corner attributes, CSR graph adjacency, simple Laplacian derivation and appropriate repair boundaries.

New C++23 code under the existing cumulative lab:
- \`include/book03/mesh_topology.hpp\` — discrete incidence and vertex-fan/duplicate-position/component report; per-corner UV; static CSR and uniform graph Laplacian, as well as deterministic open/closed/broken fixtures.
- \`include/book03/halfedge_mesh.hpp\` — immutable face-cycle/half-edge construction with \`next\`, \`previous\`, and twins only for correctly opposite pairwise incidence; rebuild-only handle lifetime.
- \`tests/mesh_topology_tests.cpp\` — independently hand-counted quad and closed-house expectations, broken mesh fixtures, 300 deterministic strip-sized independent combinatorial witnesses, CSR and Laplacian tests.
- \`tests/halfedge_mesh_tests.cpp\` — face/twin symmetry and deliberate corruption tests.
- \`src/mesh_studio.cpp\` — current-fixture selection through CLI/REPL, indexed diagnostic numeric output, source-generated oblique SVG and edge-incidence CSV.

Original figures are source-controlled \`Assets/17-indexed-quad-adjacency.svg\`, \`18-coincident-positions-separate-vertices.svg\`, \`19-nonmanifold-edge-and-bowtie.svg\`, \`20-closed-indexed-house.svg\`, and \`21-halfedge-twins-and-cycles.svg\`.

**Explicit bounds:** topology comparisons are exact integer-index operations, but geometric area uses standard double cross-product; no exact 3D predicate, global collision/self-intersection certificate, automatic welding or correct physics contact filtering is claimed. Euler V−E+F is a count for a specified indexed complex, not a manifold proof. The half-edge structure excludes repeated-index faces, represents boundaries by missing twins and refuses to fabricate twins for invalid incidence.

## Legacy material still awaiting full integration

The inspected chapter continues beyond the ranges actually read. Full final composition must preserve the independent winged-edge exposition, boundary-loop algorithms, surface reconstruction context, advanced graph/CSR layouts, cotangent weights, mesh conditioning/proof material, source citations and numerical examples that remain in the original authoritative master.

It is essential not to mistake a *source map* for a physically merged book. All original continuous masters, generated chapter editions, LaTeX stage sources, earlier Books 01/02 and the final PDF/EPUB publication folders remain untouched.

## Next original-source route

Chapter 08 should inspect and preserve the old curve/control-point material (Bézier, Hermite, B-splines, derivatives, patches and tessellation) rather than writing a standalone toy chapter that ignores the legacy manuscript. Continue through the existing Book03 branch. Chapter 10 needs a deeper inspection and implementation of imported mesh contracts, boundary loops and repair choices after Chapters 08–09 establish the shape-generation material.
