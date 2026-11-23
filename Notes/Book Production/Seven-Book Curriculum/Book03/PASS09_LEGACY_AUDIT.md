# Book 03 — Pass 09 original-source custody: procedural meshes, level sets and implicit modeling

**Date:** 23 November 2026  
**Working branch:** curriculum/book03-geometry-images-pass01  
**Preservation contract:** canonical long continuous notes, derived chapter editions, legacy LaTeX stage, published Books 01/02 and all EPUB/PDF distribution assets are **read-only**. This pass adds a new reader chapter, C++23 lab, original editable figures and audit files to the separate Book03 subtree.

## Directly inspected source files — real Git blob references

| Actual source path under \`Notes/Chapter Edition/02 - Graphics Vision and XR/\` | Git blob inspected | Declared canonical source master span | Material found and relevant to P09 |
|---|---|---|---|
| \`02 - Surfaces, Topology and Parameterization.md\` | \`41a7b270a4ee80ec12cdd57c8421351565f25ec4\` | Graphics master **2204–3118**, declared source-span SHA256 \`124ad7fa099f4d4b38f41d50bfa82f53d8cab0b2d962cd14b22be2cb1f69f270\` | Original representations at reading-copy lines approx. **18–61**: parametric versus implicit expressions, 2D charts, surface normals, orientability, patch singularities and continuity; approx. **80–125**: indexed mesh validity and Laplacian operators; approx. **235–240**: signed distance as a geometric signal; approx. **345–380**: subdivision and topology; later source approx. **429–550**: manifold, orientation, boundary chains and algebraic topology; approx. **656–685**: level-set component and Reeb graph; approx. **685–800**: surface differential operators and convergence/boundary caveats. |
| \`34 - Learned 3D Shapes and Scene Representations.md\` | \`50083685d85e449e0d205bc728903a0cbdc03bac\` | Graphics master **78561–79471**, declared source-span SHA256 \`82fb2193cb9627611c0b2754398c75529739a400d13e80c24155c9c474bcb3dc\` | Reading-copy lines approx. **18–40** distinguish point clouds, explicit meshes, parametric shapes and implicit functions; approx. **69–101** describe neural implicit fields, occupancies, signed distances and the modeling task; approx. **150–180** cover multiresolution hash-grid features, interpolation, sampled SDF training and limits. This is **future-facing context**, not a request to put a neural renderer in P09. |
| \`36 - Source Atlas and Reconstruction Checkpoints.md\` | \`6dc8d20a7aaf867a6bb6bb5d835ffe246c6aaadb\` | Graphics master **81441–83104**, declared source-span SHA256 \`a99a6f5d2b4bdee5073e1b5a2b0e03d9b7188e48213026397cda427724a64d12\` | Real course and reading provenance: **CMU-09 Introduction to Geometry** (around reading-copy line 122), **CMU-10 Meshes and Manifolds** (around 133), **CMU-12 Geometric Queries** (around 155), MITG-03 Curves and Surfaces (around 318), and **SC-15 Stanford CS231N 3D Vision** (around 1539). Their recorded lecture/video/slides metadata and uncertainty must survive final composition. |

The blob IDs were returned by the connected GitHub source reader. The declared source-span SHA256 hashes and line ranges are **metadata from the existing derived headers**, not independent recomputation of complete master body checksums. The long continuous master was not successfully returned as full content by this connector in Pass 08; do not claim a verified master hash or completeness of original-source reconciliation.

## Correspondence between original material and the new chapter

| Source concept | New P09 narrative and test witness | Material still owed to the full edition |
|---|---|---|
| Parametric shape \(\mathbf S(u,v)\), implicit \(F(\mathbf x)=0\), indexed mesh | Circle formulas; explicit distinction among generative surface, level set and stored finite triangle mesh; first original figure | Detailed charts/parameter distortion, inverse map, reparameterization and topological manifold proofs |
| Field is not automatically signed *distance* | Compare squared circle field versus Euclidean signed distance; sphere SDF sign and units tests | General signed-distance computation for arbitrary meshes, error bounds, medial-axis nondifferentiability |
| Heightfield as parameter graph \(y=h(x,z)\) | Analytic sine-cosine height, exact partial derivatives and normals, explicit cave/overhang limitation; independent grid Euler tests | Noise/fractal/erosion, tile boundaries/LOD, landscape memory/performance, physical collision/rendering |
| Consistent orientation and per-corner attributes | XZ plane is wound opposite XY; capped cylinder shares geometric ring IDs but needs distinct rim normals | Full tangent-space/UV seam policy and OBJ/glTF materials in Chapter 10 |
| Parameter singularity and periodic seam | UV sphere with one true north/south index, modulo-wrapped longitude, analytic radius and outward-winding tests | Equal-area/low-distortion meshing, icosphere, sampling and optimal triangle quality |
| Implicit sphere field and gradient | Exact mathematical \(d=\|\mathbf x\|-r\), analytic radial gradient excluding center; separate finite difference diagnostic | General robust gradient/normal and distance error certification, neural/SDF fields, distance acceleration |
| Level-set triangulation | Restricted six-tetra per cube extraction with linear root interpolation and global **lattice-edge ID** cache, exact-zero tie rejection | Full marching-cubes ambiguity, asymptotic decider, dual contouring, feature-preserving extraction, adaptive/octree boundaries |
| Mesh closure and topology | Independent P07 incidence tests for disk vs closed sphere, wrap/poles, halfedge fan; mathematically derived \(V,E,F\) counts | Global self-intersection/geometry watertightness proof, full oriented 3-manifold validation and repair |
| Representation and inference | Explicit conceptual bridge to learned implicit fields and the older original 3D representation survey | NeRF/Instant-NGP training/perception/rendering and uncertain reconstructions routed to later books and chapters |

## Newly authored implementation and source figures

The new reader \`Chapters/09 - Making a World from Rules.md\` is an original, long human-first narrative with correct units, hand-derived topological identities, analytic and sampled normal formulas, real C++23 experiments, falsification examples and selected exercise solutions.

The cumulative native library adds \`Code Parts/tools/labs/book03_cpp23/include/book03/procedural_geometry.hpp\`:
- \`TerrainField\` and \`make_heightfield\` with analytic derivatives, a connected XZ grid and upward face winding.
- \`make_capped_cylinder\` using one indexed ring per end, modulo angular seam and shared center-fan cap indices.
- \`make_uv_sphere\` using one vertex per pole, interior longitude rings and outward face orientation.
- \`SphereDistance\` analytic sphere SDF and gradient, \`central_gradient\` numeric diagnostic.
- \`extract_sphere_isosurface\` with fixed six-tetra cube decomposition, global cache keyed by sorted *lattice vertex IDs*, sign-changing linear edge roots and radial orientation for a **sphere centered on the origin**. Reject exactly zero grid-corner values and invalid/undersampled bounding grids rather than silently inventing a mesh.

\`tests/procedural_geometry_tests.cpp\` checks known field signs, derivatives, independent finite-difference normals, explicit terrain/cylinder/sphere counts, face-normal directions, 3D radial sphere consistency, cached implicit shared indexing and rejection cases. The expected implicit genus-zero topology was also independently mirrored in a small separate Python reasoning check during authoring, for multiple finite grids; that is **not a native C++ execution result**.

\`src/procedural_studio.cpp\` produces inspectable current-state SVG, indexed geometry-only OBJ and CSV, with a REPL to alter selected shape, resolution, radius and terrain amplitude. Its diagram uses an explicitly non-camera parallel oblique projection, not a GPU renderer or scientific performance plot.

Six new original figures:
- \`Assets/28-three-ways-to-describe-a-shape.svg\`
- \`Assets/29-heightfield-rule-to-grid.svg\`
- \`Assets/30-cylinder-wrapping-and-caps.svg\`
- \`Assets/31-sphere-poles-and-seam.svg\`
- \`Assets/32-marching-tetrahedra-edge-identity.svg\`
- \`Assets/33-procedural-representations-and-topology.svg\`

All six are embedded in Chapter 09. No pre-existing manuscript or release figure was overwritten.

## Accuracy and limitations — no inflated research result

The field-derived sphere is an exact analytic SDF **in real arithmetic**; its finite sampling and tetrahedra interpolation are not exact shape reconstruction. The interpolated root is biased inward relative to the true sphere under exact convex norm reasoning; tests apply an explicit floating tolerance, not bitwise certitude.

The extractor is **not a generic full marching-cubes or arbitrary-field tetrahedra engine**. Its outward orientation witness is only radial for a centered sphere, and exact-zero lattice vertex values are rejected. Floating signs near zero are not certified; the source cannot guarantee watertightness or manifoldness for arbitrary learned SDF input or self-intersecting geometry.

Topology checks are discrete indexed invariants: expected Euler and incidence do not certify the absence of geometric self-intersection or unique shading normals at sharp attribute seams.

The analytic terrain does not make caves or overhangs; the UV sphere parameterization distorts area near poles; the generated OBJ lacks UVs, normals, materials and any general scene graph. The REPL is a tiny native educational editor, not a game engine.

No confirmed successful native compilation or all-ten-CTest pass has yet been observed. The GitHub Actions additions are authored acceptance checks, not evidence of green CI. No EPUB/PDF/KDP build was run.

## Next original-source path

Chapter 10 — **A Mesh You Can Trust** should inspect both \`Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md\` and \`41 - Per-Vertex and Per-Corner Mesh Attributes.md\`, plus the original glTF/OBJ and scene-serialization / GPU data source notes. Build robust input validation and explicit attribute-domain conversion; do not assume file-format parsing alone proves topology or physically meaningful unit choices. Keep existing Books 01–02 and source continuous manuscripts untouched.
