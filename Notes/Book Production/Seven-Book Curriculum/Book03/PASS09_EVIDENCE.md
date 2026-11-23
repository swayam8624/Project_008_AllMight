# Book 03 — Pass 09 evidence and open verification gates

**Series:** From Bits to Research  
**Volume:** Book 03 — From Geometry to Images  
**Reader chapter:** 09 — Making a World from Rules  
**Branch:** \`curriculum/book03-geometry-images-pass01\`  
**Date:** 23 November 2026  
**Type:** committed additive manuscript / native C++23 mathematical implementation / original SVG / independent unit test source. This document is not proof that the GitHub workflow or C++ binary has executed successfully.

## New full-length reader deliverable

\`Chapters/09 - Making a World from Rules.md\` is approximately **8,300 words** of original explanatory prose, equations, worked examples, algorithm walkthroughs, explicit numerical and representation limits, a complete native lab handoff and a substantial reconstruct/falsify/transfer/implement/explain experiment sequence.

It keeps the paper-house world continuous: the scene develops a hill from a heightfield, a capped tower, a parametrically sampled sphere and a second sphere extracted from a mathematical signed-distance field. The central lesson is **a continuous mathematical representation, its samples, and the resulting indexed triangle mesh are different objects**.

Specific mathematical and engineering contracts covered in the actual chapter:
- Parametric map, implicit zero contour and explicit indexed mesh distinction; squared field is not the same as signed Euclidean distance.
- \(h(x,z)=A\sin(fx)\cos(fz)\) terrain; units of amplitude and frequency; analytic height derivatives and \((-h_x,1,-h_z)\) upward normals; XZ ground winding reversal compared with an XY patch; inability to express caves.
- Independent rectangular-grid counts \(V=(m+1)(n+1)\), \(F=2mn\), \(E=3mn+m+n\), \(B=2(m+n)\) and Euler one, including a hand-worked 8×10 terrain.
- Capped-cylinder parametric formula, modulo angular seam and *no duplicated theta=2pi vertex*, true shared rim IDs, cap orientation, \(V=2s+2,E=6s,F=4s\), Euler two and separate desired per-corner side/cap normals.
- UV sphere via polar and longitude parameters, one north and one south vertex, longitude wrapping, ring-count proof \(V=2+s(t-1),F=2s(t-1),E=3s(t-1)\), Euler two, pole singularity and triangle crowding; exact smooth sphere versus chord-faceted mesh.
- Analytic sphere SDF \(\|x\|-r\), sign conventions, gradient \(\nabla d=x/\|x\|\) outside the singular origin, units, finite differences and cautions about min/max field combinations.
- Strict small marching-tetrahedra extraction: six fixed tetrahedra per Cartesian cube, sign-case counts 0/1/2/3/4, one or two triangles, edge crossing fraction \(f_A/(f_A-f_B)\), global **lattice edge index** cache, radial sphere-specific face orientation, exact-zero grid-sample rejection and undersampling.
- Expected closed sphere \(3F=2E\), \(V-E+F=2\), hence \(F=2V-4,E=3V-6\), with geometric self-intersection limits.
- A real-arithmetic convexity argument showing linear field-interpolated sphere crossings lie on or inside the ideal sphere; numeric tests use tolerance.
- Determinism, finite grid complexity \(O(N^3)\), learned implicit and more advanced meshing context, explicit refusal to overclaim a generic CAD or neural renderer.
- Complete reader commands and exercises with selected independent answers.
- Chapter 10 bridge into glTF/OBJ asset ingestion, UV and normal domains, schema/index/unit validation and mesh-robustness contracts.

## Six newly authored original figure assets

All belong to the existing source-controlled \`Notes/Book Production/Seven-Book Curriculum/Book03/Assets/\` (and all are linked from the manuscript):

1. \`28-three-ways-to-describe-a-shape.svg\`: continuous circle parameterization, implicit zero set and 12-vertex polygon comparison.
2. \`29-heightfield-rule-to-grid.svg\`: mathematically sampled terrain with XZ and positive Y conventions, the same 8×10 grid-count witness \(V=99,E=258,F=160,B=36,\chi=1\).
3. \`30-cylinder-wrapping-and-caps.svg\`: 12-slice capped cylinder with modulo indexing, two cap centers and independent position/normal seams, \(V=26,E=72,F=48\).
4. \`31-sphere-poles-and-seam.svg\`: actual 12×6 latitude-longitude triangulation with one vertex per pole, \(V=62,E=180,F=120,\chi=2\).
5. \`32-marching-tetrahedra-edge-identity.svg\`: two neighboring tetrahedra crossing a grid edge, sample values -0.2 and +0.3, interpolated \(t=0.4\), canonical key (17,42) and one output vertex ID.
6. \`33-procedural-representations-and-topology.svg\`: four representation miniatures and their separate boundary/closure expectations; the implicit inset is explicitly conceptual, not a measured algorithm screenshot.

Source figures have SVG titles/descriptions, are directly embedded in the reader and use only original authored geometry/text—no third-party figure or font is copied.

**Cumulative Book 03 original technical figures: 33.**

## Actual C++23 implementation

\`Code Parts/tools/labs/book03_cpp23/include/book03/procedural_geometry.hpp\` adds:
- \`TerrainField\` with analytic height and derivative normal;
- \`make_heightfield\` with one indexed grid and upward winding;
- \`make_capped_cylinder\` with angular modulo vertex identity and closed cap fans;
- \`make_uv_sphere\` with one indexed pole each and wound latitude rings;
- \`SphereDistance\` real analytic SDF, \`gradient\` and finite-difference \`central_gradient\`;
- \`extract_sphere_isosurface\`: a bounded strict-sign sphere-specific marching-tetrahedra extractor, sharing one mesh vertex per globally cached intersected **lattice edge** and orienting triangles radially outward.

All input resolution/radius/width/height bounds are explicit. It rejects nonfinite or invalid dimensions, exactly zero field values at lattice samples, undersampled fields with no inside/outside transition and degenerate triangle orientation cases. The library is pure C++23 and composes the already-existing \`Point3\`, \`Vector3\`, \`Mesh3\` and \`inspect_mesh\` contracts.

\`Code Parts/tools/labs/book03_cpp23/tests/procedural_geometry_tests.cpp\` adds the **tenth CTest** across nine labs, with independent:
- terrain closed-form height, derivative normal and finite-difference comparisons;
- 1×1 and 8×10 indexed disk/Euler/winding oracles;
- capped cylinder n=3 and n=12 closed-edge/orientation witnesses, plus correct sharp-rim shading expectation;
- pole-safe sphere orientation and topology across (slices,stacks)=(3,2),(7,4),(12,6),(23,11);
- SDF signs, nonzero gradient and undefined center gradient;
- implicit sphere extraction at multiple controlled grid resolutions, global intersection cache count, closed-edge incidence, genus-zero Euler and radial triangle direction;
- exact-zero lattice grid tie, too-small unsampled sphere, invalid resolution, invalid height/radius and nonfinite field input rejections.

\`Code Parts/tools/labs/book03_cpp23/src/procedural_studio.cpp\` adds \`book03_world inspect|svg|obj|csv|repl\`. The user selects a real generator and sets resolution/radius/amplitude; all numerical reports and file exports come from the actual corresponding \`Mesh3\`. Source-generated SVG diagrams intentionally show an oblique parallel projection with no hidden-surface removal or camera calibration; the OBJ contains geometry-only one-based triangle indices; the CSV contains the actual indexed positions. An invalid interactive setting is rejected before changing the stored selected state.

CMake adds \`book03_procedural_geometry\`, \`book03_world\` and \`book03_world_tests\` with the new test registration \`book03_procedural_heightfield_sphere_cylinder_isosurface_contracts\`. The dedicated Book03 GitHub workflow now specifies ten CTests across nine source projects and an independent P09 SVG/CSV/OBJ/REPL proof step.

## Additional independent reasoning check, not a native run

To catch silent combinatorial errors before labeling the designs finished, an independent Python rendition of the exact restricted sphere-tetrahedral sign cases, edge cache and orientation rules was executed in the assistant environment. For the **same analytic sphere radius 0.91 m** and grid half-extent **1.5 m**, the independent reference returned:

| Cartesian cells per axis | Referenced output V | Indexed output E | Triangular faces F | Euler | Shared-edge incidences / winding |
|---|---:|---:|---:|---:|---|
| 9 | 422 | 1260 | 840 | 2 | Every edge 2 incident opposite uses, 0 conflict |
| 12 | 758 | 2268 | 1512 | 2 | Every edge 2 incident opposite uses, 0 conflict |
| 13 | 914 | 2736 | 1824 | 2 | Every edge 2 incident opposite uses, 0 conflict |

The independent 12×6 latitude-longitude case was also enumerated from its actual mathematical face-index rules as \(V=62,F=120,E=180\), zero winding mismatches and outward face normals; 3×2, 7×4 and 23×11 cases were separately checked by the same small independent reasoning code.

**These are independent algorithmic sanity checks, not compilation/execution of the GitHub C++ header, not GitHub Actions passing status, not real rasterized plots, and not evidence of cross-platform floating reproducibility.** An actual CMake build, CTest and source-export validation remain necessary to confirm the native code matches them.

## Original source custody

\`PASS09_LEGACY_AUDIT.md\` records actually inspected source details, including original derived chapter blobs:
- \`02 - Surfaces, Topology and Parameterization.md\`: \`41a7b270a4ee80ec12cdd57c8421351565f25ec4\` (parametric vs implicit, geometric normals, level sets, manifold/topology/mesh operators).
- \`34 - Learned 3D Shapes and Scene Representations.md\`: \`50083685d85e449e0d205bc728903a0cbdc03bac\` (point/mesh/parametric/implicit survey, neural fields and SDF representation limits).
- \`36 - Source Atlas and Reconstruction Checkpoints.md\`: \`6dc8d20a7aaf867a6bb6bb5d835ffe246c6aaadb\` (CMU geometry/manifold/queries, MIT surface material, later learned 3D).

Original declared source master spans and SHA256 metadata are recorded in the audit, without falsely claiming independently rehashed continuous master contents. The original master, derived reading copies, LaTeX stage, Books 01–02 and existing EPUB/PDF release library have **not been edited**.

## Gate status and limits

**Committed / checked as source:** full narrative, six new original SVG files, C++23 procedural library, unit test source, terminal design studio and export code, CMake/CTest and CI wiring, original source map, and book/lab README/ledger updates.

**Still unobserved:** successful native C++23 compilation, all ten CTests passing, actual exported files rendered by the compiled C++ binary, GitHub Actions green status, machine-by-machine numeric parity, final print/e-reader visual QA, full original-content source-unit integration, final Book03 LaTeX/EPUB/PDF generation or KDP publishing.

**Deliberately unclaimed:** guaranteed no geometric self-intersections, certified sphere distance extraction bounds, generic marching cubes, arbitrary field topology, robust exact 3D predicates, NURBS/CAD watertightness, split/merge dynamic mesh editing, normal-mapped materials, texture UVs or a rendering engine.

## Next pass

Chapter 10 — **A Mesh You Can Trust**. Inspect and preserve original mesh topology, per-face-corner attributes, glTF/OBJ ingestion, units and buffers. Author robust bounds/index/finite-data checks, nonmanifold/seam diagnostics and a real source-constrained input project, retaining the existing committed Book03 source tree and leaving the reader's final PDF/EPUB compilation for the full-volume review.
