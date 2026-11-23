# Book 03 — Pass 06 source-custody audit: from 2D predicates into 3D geometry

**Inspection:** 23 November 2026, branch \`curriculum/book03-geometry-images-pass01\`.

This audit records **specific source openings inspected in connected GitHub** at their blob identities; it does not claim that the complete long originals or their derived chapter bodies have been read line by line. The user's authoritative continuous notes, compiled stages, generated chapter edition and previously published books were not changed.

| Existing reading-copy source (under \`Notes/\`) | Inspected Git blob SHA | Source-span declaration in original header | Evidence used and later chapter route |
|---|---|---|---|
| \`Chapter Edition/02 - Graphics Vision and XR/24 - Ray–Triangle Intersection.md\` | \`b6bf1b44ec478c09621ba1d5af7a6b2523d80cc5\` | Graphics master lines 59763–62972; header-declared SHA256 \`3ffa6980cdf14c6007237c740d88381cfe177214e26ca62f815836135e6fde94\` | Derives \`ray = O+tD\`, triangle normal \((B-A)\times(C-A)\), plane test, then **bounded** barycentric triangle containment; follows with Möller–Trumbore. P06 intentionally implements only primitive ray-plane (and ray-sphere) so it does not falsely claim a triangle hit. Complete original work must be integrated in the later triangle-query/raster and mesh chapters, not omitted. |
| \`Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping.md\` | \`aca655f43e39cff8d7bcbe1c9a03f8e994f6602f\` | Graphics master lines 68892–71960; header-declared SHA256 \`e7a6dba6ac80dd93ac1fc24c080cf25f2d60526c6a27b90ac043ee0164bb3394\` | Separates scene/mesh candidate selection, acceleration, narrowphase, any-hit/closest-hit/all-hit policies and meaningful result records. P06's ray primitive is only a first step; preserve the million-triangle/acceleration derivation for later full integration in Books 03/05/06. |
| \`Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md\` | \`991dfa264100be10e1df3efc801966ae1d383da1\` | Graphics master lines 57090–59762; header-declared SHA256 \`527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8\` | Indexed topology is distinct from point positions; shared edges, opposite directed incidence and nonmanifold properties are different contracts. Reserve the complete treatment for Book 03 Chapter 07. |
| \`Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem.md\` | \`2bb93ac0b57df60f2b7e7655ad196373215a30bf\` | Computing Foundations master lines 78505–80940; header-declared SHA256 \`340c3f3eab22b702f8d250fbf0fdb8bd0a1e0b2e848681aa23ee77df8dacef80\` | The original explicitly declares right-handed 3D camera local \(-Z\), column vectors, PVM order, camera/world/view/clip conventions, and calibrated projection. P06's **oblique parallel SVG** is an introductory diagram and not a substitute camera. Preserve the full original camera derivation in reader Chapters 11–17. |

The per-source-span SHA256 strings above are *metadata copied from original derived chapter headers*, not newly computed hashes of the long original master segments. They require independent source-body reconciliation.

## Where this pass adds something without deleting anything

The new \`Chapters/06 - Step Out of the Page.md\` teaches a paper house acquiring depth, then extends 2D points/vectors to 3D, derives dot/cross products, handedness, face normals, plane equations, signed distances, ray/line/segment parameter domains, ray-plane and ray-sphere intersections, numerical conditioning and a deliberate oblique map for paper diagrams.

The new \`include/book03/geometry3d.hpp\` is not a copy of the old renderer or physics implementation. It declares a small original, independent reference with \`Point3\`/\`Vector3\` type distinction; \`Triangle3\` orientation and area; normalized \`Plane3\`/\`Ray3\`; \`Sphere3\` and honest intersection status enums. It does not attempt to certify arbitrary floating 3D orientation, solve a scene/mesh query, or claim to emulate camera optics.

\`tests/geometry3d_tests.cpp\` independently compares embedded XY triangle cross-product area against the earlier bounded exact 2D predicate, checks ray/plane substitution, and distinguishes sphere roots and invalid/uncertain cases. The pedagogical \`src/spatial_studio.cpp\` exports an explicitly parallel oblique SVG and small CSV based on **the same source geometry**, rather than a manual matching approximation.

Four original illustrated SVGs are source-controlled alongside earlier diagrams:
- \`Assets/13-right-handed-space.svg\`;
- \`Assets/14-3d-cross-normal.svg\`;
- \`Assets/15-ray-plane-cases.svg\`;
- \`Assets/16-house-leaves-the-page.svg\`.

These are geometric constructions, not captured benchmark or physical measurement results.

## Exactness and numerical policy boundaries

- The previous Chapter 05 binary64 oracle is a **2D** exact sign predicate. P06's ordinary floating-point cross product does not inherit that proof in three dimensions.
- A unit normal produces a signed Euclidean plane distance in **metres**, subject to floating evaluation accuracy. A nonunit normal must be normalized or its dot/offset scaled correctly.
- A ray's direction is normalized on construction, so the ray parameter \(t\) is in **metres**. A full line's negative \(t\) is not a valid forward ray intersection.
- Ray-plane near-parallel and ray-sphere near-tangent thresholds are explicitly heuristic and return uncertainty rather than a false certified hit when appropriate.
- An exact plane hit does not imply a bounded triangle hit. Topology and boundary ownership are later contracts.
- The source-generated diagram uses a fixed affine \(\mathbb R^3\to\mathbb R^2\) map, **not** perspective, occlusion or calibration.

## Remaining original-source work

Do not reduce the multi-thousand-line original ray–triangle, mesh-query and camera sources to a few formulas. Before final publication, record full subsection-level mappings, verify the original master hashes and transfer all unique proofs, diagrams, code, experiments and citations into the corresponding reader chapters or explicitly documented series cross-references. Neither the original chapter edition nor the continuous master may be deleted or reorganized during these passes.

**Next research/editorial target:** Book 03 Chapter 07 — indexed meshes, shared topology and per-corner normals/UV seams, with a correct adjacency and manifold inspector and carefully chosen independent oracle tests.
