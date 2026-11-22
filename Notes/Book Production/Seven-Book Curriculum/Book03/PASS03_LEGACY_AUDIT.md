# Book 03 — Pass 03 legacy-source custody audit

This document records **actual inspection of the opening spans** of relevant source-derived chapters in the restructured GitHub repository, plus a bounded destination plan. It does not claim full-body reading, complete paragraph routing, editorial/math QA, renderer correctness or native device verification. It is specifically written to prevent our new introductory chapters from replacing already substantial original material.

## The pre-existing geometry spine

The old AllMight continuous masters already contain signed-area geometry, robust predicates, barycentric mathematics, ray/mesh queries and rasterization. Pass 03 adds a first-principles entry chapter and an intentionally limited C++23 lab; it must not be passed off as complete reconstruction or a rigorous adaptive-exact geometry kernel.

| Original Notes/Chapter Edition/ source | Checked Git blob SHA | Original master source span and declared SHA256 | What the inspected source says | Reader placement |
|---|---|---|---|---|
| 01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity.md | c75a9727e22b771187f941ddfebe0042bb8ee9ba | Computing Foundations lines 84339–87111; f824915709a11c867778d38638510a33752e0269c1d2d5233cf5caca04f05d7c | Distinguishes a discrete sign predicate from approximate coordinates, derives orientation determinant, warns of cancellation and proposes fast filtering followed by proven error-bound/exact evaluation | Chapters 03 (intro), 05 (complete robust predicate), and 26 (camera numerical sensitivity) |
| 02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md | 991dfa264100be10e1df3efc801966ae1d383da1 | Graphics master lines 57090–59762; 527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8 | Mesh vertex positions and topological identities are distinct; indexed faces, shared edges, disconnected duplicates, winding and incidence are separate contracts | Chapter 07 complete indexed-mesh study and Chapter 10 validated ingestion |
| 02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence.md | 4ff530f607fe3ae9cd72c9da09794fb3c74b84e1 | Graphics master lines 35331–37362; a78b04e715d3b81b09dbc46ac29d254d8f057bb0a3210dcc2c4a1812faa8adc1 | Describes an observed historic Kairo renderer and then expands into edge-based rasterization foundations. Historical renderer facts require new pinned source verification before being described as current | Chapters 18–21, Book 6 GPU counterpart |
| 02 - Graphics Vision and XR/24 - Ray–Triangle Intersection.md | b6bf1b44ec478c09621ba1d5af7a6b2523d80cc5 | Graphics master lines 59763–62972; 3ffa6980cdf14c6007237c740d88381cfe177214e26ca62f815836135e6fde94 | Plane intersection gives a candidate point; barycentric test determines whether it lies within the bounded triangle. Distinguishes parallel/coplanar cases | Chapters 06, 18 and 21 as geometric bridge; complete ray transport later in Book 4 |
| 02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping.md | aca655f43e39cff8d7bcbe1c9a03f8e994f6602f | Graphics master lines 68892–71960; e7a6dba6ac80dd93ac1fc24c080cf25f2d60526c6a27b90ac043ee0164bb3394 | Separates primitive narrow-phase predicates from a mesh/scene query, with candidate selection, policy and meaningful hit returns | Chapter 10 mesh queries, Chapter 17 clipping, Book 5 collision and Book 4 tracing |
| 02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md | ca921d2747a6321fb9bebcde97edb086bb74edb7 | Graphics master lines 95949–99206; c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9 | A shared topological vertex can require separate rendering corner attributes (normals or UV seams), so GPU vertex identity is not the same as mesh topological identity | Chapters 07 and 10, then Chapter 21 renderer; Book 6 GPU storage |

Source-span hashes in this table are declarations written in the existing chapter headers, not hashes independently recomputed over the three full continuous master files. A later source-level audit must verify them before publishing a preservation proof.

## Exact versus approximate geometry boundaries

The reader's Chapter 03 is permitted to introduce

\[
\operatorname{orient}(A,B,P)=(b_x-a_x)(p_y-a_y)-(b_y-a_y)(p_x-a_x)
\]

and barycentric signed-area ratios. It must explain that the sign depends on vertex order and axis orientation; when image v increases downward, raw 2D winding sign can flip relative to a world y-up frame. The source's more difficult guarantee—**a predicate with a certified numerical sign**—is not obtained by picking a floating tolerance. It must be developed from genuine numerical analysis and a reference oracle in Chapter 05, preserving the old original depth.

P03 therefore keeps two *different* interfaces:

- An exact signed 64-bit orientation/triangle-containment result for grid coordinates within \([-10^6,10^6]\). Differences are at most \(2 \times 10^6\); terms are at most \(4 \times 10^{12}\); their subtraction stays below \(8 \times 10^{12}\), well inside signed 64-bit. This contract is mathematically bounded and testable.
- A noncertified double-precision barycentric query for general finite coordinates, which exposes near-boundary and degenerate-or-uncertified cases rather than promoting approximate floating results to proofs. The code's guard is deliberately a teaching heuristic, not an adaptive Shewchuk-style guarantee.

The exact grid predicate is not a replacement for Shewchuk, arbitrary-precision rational geometry or a full constrained triangulation kernel. The scalar gradient and SVG figure are didactic illustrations; attractiveness cannot certify the numeric test.

## What we preserve next

When constructing final Book 03 chapters, collect source units from the original master by explicit heading/span and hash. Verify whether a section has moved into a new chapter, remains as a cross-book reference, or needs a source-preserving addition. Track equations, code blocks and diagram references, not only prose. Do not flatten the original ~thousands of pages into thirty-two short chapters or call source routing complete because the opening paragraphs have been inspected.

The triangle's top-left edge fill convention, sample footprints and perspective-correct interpolation are later rasterization topics. Its barycentric coordinates are an affine interpolation foundation, but not yet a certified GPU pixel-coverage rule. Chapter 07 owns mesh topology. Chapter 18 owns edge ownership. Book 04 owns physical light transport; Book 06 owns actual GPU execution; Book 07 owns advanced reconstruction/vision.

## Pass 03 source edits

Only new source under \`Notes/Book Production/Seven-Book Curriculum/Book03/\`, new lab code under \`Code Parts/tools/labs/book03_cpp23/\`, and the dedicated Book 03 CI workflow are authorized. Original continuous notes, derived chapters, current TeX stage, compilation caches, source graphs, and the local \`From Bits to Research/\` EPUB/PDF library remain untouched.
