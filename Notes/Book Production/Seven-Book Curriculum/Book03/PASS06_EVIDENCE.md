# Book 03 — Pass 06 evidence and acceptance boundaries

**Book:** 03 — From Geometry to Images  
**Reader chapter:** 06 — Step Out of the Page  
**Development branch:** \`curriculum/book03-geometry-images-pass01\`  
**Authoring date:** 23 November 2026  
**Overall state:** Source, laboratory, tests, diagrams and checks authored. Native compiler and full-book build outcomes are separate acceptance gates.  
**Publishing constraint:** No local EPUB/PDF, LaTeX stage or existing Book 01/02 release was changed.

## Reader manuscript

\`Chapters/06 - Step Out of the Page.md\` is a substantial standalone narrative of about **5,400 words**. It keeps the original paper-house story and has the house discover its back surface, real depth and visibility. The prose gives a beginner the everyday object before deriving each equation, then follows through with numerical cases, exact assumptions, code contracts, falsification exercises, answer criteria and a bridge into the indexed mesh chapter.

Detailed coverage: Cartesian right-handed \((X,Y,Z)\) with \(+Y\) up; position-versus-displacement types; vector length and normalization; dot product/alignment/projection; cross-product right-hand convention and area; triangle face normals and winding; infinite plane equation and signed Euclidean distance; line versus ray versus segment parameter domains; analytic ray-plane intersection and its near-parallel conditioning; spheres and a ray's quadratic roots; cancellation-aware root choice and tangent ambiguity; source-vs-schematic 3D-to-2D projection; 2D exact-area/3D cross-product connection; limits of exactness and the need for later topology.

**Original chapter figures (4 new, 16 cumulative):**

- \`Assets/13-right-handed-space.svg\` — world-axis directions and a 3D location.
- \`Assets/14-3d-cross-normal.svg\` — physically meaningful face-normal orientation, winding reversal and area.
- \`Assets/15-ray-plane-cases.svg\` — forward, parallel and behind-the-origin parameter cases.
- \`Assets/16-house-leaves-the-page.svg\` — world-XYZ house wireframe and a sphere depicted by numerically projected great-circle rings. The initial schematic sphere circle was replaced in the same pass because a generic circle was not faithful to the declared oblique mapping.

Each figure declares its own SVG title/description and is embedded at its point of conceptual use. The illustration map is explicitly parallel-oblique, not a photo or simulated camera exposure.

## Source implementation

**\`Code Parts/tools/labs/book03_cpp23/include/book03/geometry3d.hpp\`** — C++23 module with typed \`Point3\` and \`Vector3\` APIs, safe finite input/result checks, \`std::hypot\` for length, normalized directions, \`Triangle3\` area/face normal with conservative degeneracy guards, \`Plane3\` with unit normal and distance units, \`Ray3\` with normalized direction and distance-based \(t\), \`Sphere3\`, and separate ray-plane/sphere intersection result types including explicit uncertain cases.

**\`tests/geometry3d_tests.cpp\`** — independently checked hand examples; point-versus-vector compile-time restrictions; dot/cross perpendicularity and winding; old P03 exact integer 2D signed-area reference embedded into the 3D XY plane; plane normalization and signed distance; independent ray-plane hit substitution and forward/behind/parallel/coplanar/near-parallel statuses; sphere front/back and internal ray roots, tangent, miss, behind and uncertain near-tangent; nonfinite, zero and overflow rejections; 2,500 seeded 3D cross/dot property cases. These test *specifications* exist; do not call them executed until logs exist.

**\`src/spatial_studio.cpp\`** — native C++23 \`book03_space inspect|svg|csv|repl\`. A small house, sphere, coordinate axes, roof-panel normal, reference ray and floor plane are all specified as actual 3D coordinates. The same library evaluates primitive intersections and provides the data for the figure and CSV. A user can change the ray from the REPL, re-inspect and save new outputs. No physical camera, graphics API or benchmark data are fabricated.

**Build/quality:** \`CMakeLists.txt\` adds \`book03_geometry3d\`, \`book03_space\`, \`book03_space_tests\` and sixth CTest \`book03_3d_geometry_ray_plane_sphere_contracts\`. The dedicated Book 03 GitHub Actions source now invokes the sixth lab and checks generated SVG structure, 3D scene contents, ground/sphere CSV rows and the user-edited reference ray's exact simple witness (1,3,2) + 3(0,-1,0) = (1,0,2). \`editorial_gate.py\` now requires Chapter 06, the P06 source code, and all sixteen original figures with meaningful accessible titles/descriptions.

## Original-source custody

\`PASS06_LEGACY_AUDIT.md\` records actual inspected original chapter entrypoints and blob identities for Graphics/Vision Ray–Triangle Intersection, Mesh Queries/Classification/Clipping, Mesh Topology/Connectivity and Computing Foundations camera geometry. It explicitly keeps:

- the original supporting-plane **then bounded barycentric** triangle hit derivation, plus Möller–Trumbore and boundary cases;
- mesh-query candidate selection, acceleration structures, any-hit/closest-hit/all-hit policies and meaningful scene result records;
- topological incidence, edge orientation, nonmanifold tests and per-corner attributes;
- the original right-handed camera local-\(-Z\) convention, projection, clip/raster and inverse reconstruction.

This pass **does not** replace those deep original chapters with a short overview; their complete subsection-level reconciliation remains future work. Previously published Books 1 and 2, \`Notes/Continuous Notes/\`, \`Notes/Chapter Edition/\`, \`Notes/Book Production/AllMight Codex/LaTeX/\` and the \`From Bits to Research/\` publication library are not edited.

## Numerical honesty

- A dot or cross product computed in ordinary \`double\` is approximate. The previous P05 exact binary64 orientation is **2D only**, and its proof does not extend automatically to 3D cross products or ray constructions.
- The C++ geometry module normalizes planes and rays so distances and ray parameters are in consistent metres. This does not certify all floating results against cancellation/overflow or poorly conditioned problems.
- The plane near-parallel and sphere near-tangent guards are explicit heuristic policy thresholds, not certified error bounds or complete exact predicates.
- Plane intersections are infinite-support-plane results, not bounded triangle hits or occlusion. No camera depth test, ray tracer, GPU or full physics engine was claimed.
- The SVG diagram projection is parallel/affine, with explicit metres-to-pixels coefficients. It is neither a pinhole camera nor a physical lens model.

## Verification state

**Observed:** GitHub branch write acknowledgments for the new Chapter 06 source, P06 C++ files, four original SVG assets, associated CMake/CI/editorial files, SOURCE_LEDGER addition and preservation audit. A final \`main...branch\` comparison must be recorded separately at the end of this pass.

**Not yet observed:** C++ compiler/CTest success on native or CI runners; pixel-accurate rendering of every SVG in a book production context; generated P06 SVG/CSV executable output; an independent cross-compiler run; an author-reviewed complete PDF/EPUB; full source-body reconciliation or final camera results. These remain explicit acceptance gates. **Do not present declared tests as passed measurements.**

## Next pass — Chapter 07: One Shape, Many Triangles

Use the working house model to build a real indexed mesh with vertex identifiers, face winding, edge keys, boundary versus two-sided versus nonmanifold edge classification, disconnected-but-coincident vertices, and normal/UV seams. Preserve and integrate the existing long source on topological connectivity and per-corner attributes. Supply C++23 deterministic mesh oracles, a broken-model repair/diagnosis workbench, trustworthy figures and an independently testable adjacency contract. Remain inside the existing Notes/Book03, Code Parts/book03_cpp23 and dedicated CI workflow paths; leave author-run final EPUB/PDF compilation to the completed book.
