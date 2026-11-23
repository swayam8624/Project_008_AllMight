# Book 03 — Pass 07 evidence and open acceptance gates

**Series:** From Bits to Research  
**Book:** 03 — From Geometry to Images  
**Chapter:** 07 — One Shape, Many Triangles  
**Branch:** \`curriculum/book03-geometry-images-pass01\`  
**Date:** 23 November 2026  
**State:** new reader manuscript, topology implementation, tests, vector figures, source-custody audit and workflow authored. A source write acknowledgment does not mean compilation or testing has passed.

## Completed new reader material

\`Chapters/07 - One Shape, Many Triangles.md\` is a long, fully written reader-first draft (approximately **6,800 words**, depending on how mathematical notation is tokenized). It continues the three-dimensional house story: identical-looking triangles can be disconnected when their vertex IDs differ. The chapter carefully derives indexed triangles, canonical edge keys, directed face traversal, boundary/interior/winding/nonmanifold incidence, half-edge cycles and symmetric twins, disconnected incident vertex fans, the closed house \(V=10,E=24,F=16\), Euler assumptions, per-face-corner UV domains, coincident-but-disconnected seam vertices, a sparse CSR neighbor graph, a hand-derived scalar Laplacian and the limits of mesh repairs. Each central concept includes narrative explanations, equations, numeric examples, C++23 source contracts, falsification questions and selected exercise answers.

**Five original source-controlled figures**, with semantic SVG title/description and chapter embedding:
- \`Assets/17-indexed-quad-adjacency.svg\`: opposite traversals of a shared indexed diagonal.
- \`Assets/18-coincident-positions-separate-vertices.svg\`: identical screen geometry with different indexed connectivity.
- \`Assets/19-nonmanifold-edge-and-bowtie.svg\`: three-face edge versus disconnected incident vertex fans.
- \`Assets/20-closed-indexed-house.svg\`: a 10-position/16-triangle/24-edge closed house, using the same original 3D coordinates and faces as the C++23 fixture. Incidence was independently enumerated, confirming 24 unique edge keys with exactly two opposite uses each.
- \`Assets/21-halfedge-twins-and-cycles.svg\`: three-edge face cycles and a symmetric opposite-directed twin pair.

All original chapter figures belong to the notes source tree, not final generated EPUB/PDF output. Visual rendering and final print/e-reader legibility still need independent review.

## Actual C++23 implementation

**\`include/book03/mesh_topology.hpp\`** implements:
- validated \`Mesh3\` with positions, ordered face index triples and optional **per-face-corner** UV values;
- canonical undirected \`EdgeKey\` and directed \`EdgeUse\` records;
- exact combinatorial classification of boundary, consistently oppositely wound interior, winding-conflict and >2-face nonmanifold edges;
- separate repeated-index versus cross-product-zero geometric-face diagnostics;
- face edge-connected components via disjoint sets;
- vertex incident-face fan connectivity and boundary incidence check, detecting the bow-tie case missed by edge-only counts;
- reports of unused stored positions and exact equal-valued coincident-position groups **without welding**;
- combinatorial Euler \(V-E+F\) of the explicitly specified referenced valid-face complex;
- immutable CSR adjacency and scalar **uniform graph** Laplacian, not the cotangent-weighted surface differential operator;
- fixtures: \`house\`, \`quad\`, \`flipped\`, \`seam\`, \`nonmanifold\`, \`bowtie\`, \`degenerate\`, \`collinear\`.

**\`include/book03/halfedge_mesh.hpp\`** adds face-owned directed half-edge records with \`origin\`, \`destination\`, \`face\`, \`next\`, \`previous\` and \`twin\` handles. It pairs only edges with two opposite directed uses; a boundary, winding-conflict or three-face nonmanifold edge has an invalid twin, never a fabricated arbitrary pairing. It validates three-step face cycles, previous/next reversibility, endpoint reversal and symmetric twins. All handles are stable only until rebuilding; no mutable modeling kernel or generation-safe editing is claimed.

**\`tests/mesh_topology_tests.cpp\`** includes independent square/house incidence oracles, duplicate seam IDs, flipped winding, nonmanifold edges, bow-tie vertex, repeated index, geometrically collinear face, unused position, malformed index/UV/coordinates, CSR degree and Laplacian witnesses, and **300 deterministic independent quad-strip size cases** checked against hand-derived \(V=2(N+1),E=4N+1,F=2N\), boundary \(2N+2\), interior \(2N-1\).

**\`tests/halfedge_mesh_tests.cpp\`** independently checks quad and house halfedge totals and twin cycles, refuses false twins on seam/conflict/three-face edges, excludes repeated-index faces and intentionally breaks \`next\` or twin symmetry to verify the validator detects corruption.

**\`src/mesh_studio.cpp\`** has native \`book03_mesh inspect|svg|csv|repl\` with an optional selected named fixture. Its numeric report, SVG and CSV come from the same C++23 source geometry and incidence graph; REPL exports reflect the **current** fixture. The SVG explicitly uses an oblique parallel map, not an actual camera or depth-buffered renderer.

## Build and authored workflow

CMake adds \`book03_mesh_topology\`, \`book03_mesh\`, \`book03_mesh_tests\`, \`book03_halfedge_tests\`. The chapter adds **two** separate CTest suites, \`book03_mesh_topology_incidence_and_manifold_contracts\` and \`book03_halfedge_cycle_and_twin_contracts\`. Across seven cumulative labs the total is **eight CTests**.

The Book03 GitHub Actions workflow defines source editorial checks, compilation and CTest, then checks produced house and seam SVG/CSV, numeric house V/E/F/Euler and edge directions, bow-tie vertex and nonmanifold edge status, and REPL fixture changes. An authored workflow is **not** an observed completed successful run.

\`Code Parts/tools/labs/book03_cpp23/editorial_gate.py\` extends the source-only check to Chapter 07, required new code files and the **twenty-one** original figures. It does not build a book or prove mathematics.

## Original legacy source custody

\`PASS07_LEGACY_AUDIT.md\` records **actually inspected** portions of the old \`Notes/Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md\`, including the original 4-vertex quad, GPU-indexing memory example, per-corner data and seams, half-edge traversal, winged-edge comparison, nonmanifold vertex links, boundary loops, adjacency and components, CSR, Laplacian and the beginning of geometric degeneracy.

The derived chapter's original source-span SHA256 header was recorded for later independent verification; it was *not* rehashed against its complete master. The much longer original source must be integrated subsection by subsection, not merely named in a ledger. Remaining original winged-edge, full boundary-loop, cotangent-weighted and numerical-repair research belongs to deeper later chapters, not a fake shortcut inside this introductory lab.

The original \`Notes/Continuous Notes/\` sources, generated \`Notes/Chapter Edition/\` reading copies, LaTeX stage, earlier published Books 1–2 and local \`From Bits to Research/\` PDF/EPUB publication library have not been edited.

## Guarantees and deliberately unclaimed results

- Indexed edge incidence, face-ID membership, edge winding comparisons and graph-component counts are exact *combinatorial* computations on validated integer vertex identities.
- Float geometry is **not exact**: a standard double cross product can misclassify some near-collinear faces under cancellation/underflow; Chapter 05's exact binary64 orientation proof is **2D only**.
- \`closed_orientable_edge_candidate()\` checks necessary local conditions. It is not proof of absence of self-intersections, global orientability for arbitrary complexes, physical watertightness, correct outward normals, a single component or suitable manufacturing geometry.
- Equal XYZ coordinates do not authorize automatic welding. A renderer's duplicated UV/normal seam vertex can intentionally differ from the conceptual surface vertex.
- The halfedge builder does not promise mutable topology editing, persistent handles or boundary-cycle extraction. Its missing-twin convention is explicitly part of the API.
- The uniform graph Laplacian is not the geometry-sensitive cotangent-weighted discrete Laplace–Beltrami operator; no smoothing/physics quality claim is made.
- SVG figures are mathematical illustrations, **not** measured performance charts or calibrated perspective images.

## Status at pass close

**Authored and committed:** the 7th full reader chapter, 5 SVG figures, P07 mesh and halfedge C++ headers, mesh REPL and two test suites, CMake/CTest and CI updates, editorial gate, lab README, original-source audit and source ledger.

**Independently checked within the work:** the closed house 16-face edge use list was separately enumerated: 24 unique keys, every key used twice and in opposite directions; each of the 10 vertices has one incident fan by independent small-data traversal. The C++23 CSR standard-library constructor/empty-subspan expression was separately probed with a local compiler as a limited syntax test; this is **not** execution of the full GitHub repository tests.

**Not observed:** full repository native C++23 compilation, all eight CTests, actual generated SVG/CSV screenshots from the GitHub code, GitHub Actions passing result, full-body source-unit reconciliation, whole-book PDF/EPUB compilation or KDP preview. Do not describe these as green until evidence is available.

A final GitHub comparison against \`main\` should confirm additions are restricted to \`Notes/Book Production/Seven-Book Curriculum/Book03/\`, \`Code Parts/tools/labs/book03_cpp23/\` and the dedicated Book03 workflow. No changes to original manuscript/release files should appear.

## Pass 08 next

Chapter 08 — **Drawing Smooth Things with Control Points**. Build an original, human-readable deep progression from piecewise lines to Bézier/Hermite control geometry, continuity \(C^0,C^1,C^2\), derivatives, de Casteljau evaluation and error-controlled tessellation. Inspect and reconcile the substantial existing graphics/numerics source first. Extend the same C++23 project with an editable curve/patch studio, independent polynomial and de Casteljau oracles, curvature/numeric conditioning tests, and original vector visual proofs. Later mesh/tessellation output should preserve P07's indexed identity and manifold constraints rather than silently disconnecting adjacent patches.
