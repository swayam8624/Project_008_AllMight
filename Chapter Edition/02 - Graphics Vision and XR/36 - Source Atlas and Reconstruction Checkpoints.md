# 36 — Source Atlas and Reconstruction Checkpoints

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/35 - A Complete Renderer as an Executable Forward Model|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds|Next chapter]]

This span is a source atlas: it records course coverage, source identity and reconstruction checkpoints alongside the book’s teaching path. Reading it well means distinguishing verified source material, transcript limits and the concepts developed elsewhere in the volume.

**By the end:** Use source provenance and reconstruction checkpoints to understand coverage and evidence boundaries.

**Recall question:** What does a source record establish, and what remains unverified by that record?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 80439–82102; source-span SHA256 a99a6f5d2b4bdee5073e1b5a2b0e03d9b7188e48213026397cda427724a64d12. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Source legend - actual playlist coverage

The following ledger accounts for every playlist position retrieved on 27 October 2026. Topics were synthesized by concept; one source can contribute to several chapters and is counted once. Technical speech sources were read sequentially. English automatic captions can mistranscribe terms, so mathematical explanations were checked against official notes and independent examples. The CMU dynamics lecture uses local Whisper small.en transcription cross-checked with its exact slides. The student showcase uses sampled frames throughout its duration, with claims limited to visible title cards and outputs.

The CMU slides are from Fall 2020. The MIT graphics captions are the supplied Fall 2021 recordings; the twenty OCW graphics decks are explicitly Fall 2012 supplements, rather than a claimed 2021 archive. The MIT machine-vision combined notes are Fall 2020; Lecture 1 has no section in that PDF. Levoy’s twenty-six decks support the eighteen Google Spring 2016 sessions, based on Stanford CS178 taught in 2009–2014. Slide text/page references and video captions were consulted; the record does not claim visual inspection of every slide or every video frame.

The durable local source cache contains inventories, original caption files, compact reading copies, PDF materials, per-video review records, and verification reports. Its location is `/Users/swayamsingal/.codex/source-cache/university-graphics-vision-2026-10-27`. The cache is separate from the teaching volume; essential explanations and complete lab code remain above.

### Computer Graphics — CMU 15-462/662, Fall 2020

[Playlist](https://www.youtube.com/playlist?list=PL9_jI1bdZmz2emSh0UQ5iOdT2xRHFHL7E)

#### CMU-WELCOME - Course welcome and learning workflow

[Source video](https://www.youtube.com/watch?v=W6yEALqsD7k) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** remote pre-recorded lectures, recitations, office hours; mini-homework conceptual checks; four Scotty3D coding assignments; modeling, rendering and animation; historical course exams and individual collaboration rules; self-paced chunking and questions.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#How to use this volume without losing the thread|Learning workflow]].

#### CMU-01 - Course Overview

[Source video](https://www.youtube.com/watch?v=PhxV_JrXeVk) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** What graphics studies; image synthesis as a cross-cutting pipeline; rasterization versus ray tracing; course roadmap..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/01_intro/01_intro_slides.pdf) (pages 1–64).


#### CMU-02 - Linear Algebra (P)Review

[Source video](https://www.youtube.com/watch?v=2c8XQlQApx8) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Vectors, points, bases, linear combinations, span, independence, matrix-vector multiplication, linear maps, composition, inverse, determinants, eigenvectors/eigenvalues, diagonalization, SVD..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/02_linearalgebra/02_linearalgebra_slides.pdf) (pages 1–61).


#### CMU-03 - Vector Calculus (P)Review

[Source video](https://www.youtube.com/watch?v=O7Bec2uX3ZQ) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Euclidean norm and dot product, including Gram-matrix metrics for nonorthonormal bases; Cross product as oriented area normal in 3D and signed area in 2D; cross-product matrix is skew symmetric; Determinants as signed volume and orientation; linear maps scale volume by determinant; Derivative limit, one-sided differentiability, Taylor approximation, directional derivative and gradient; Function-space L2 gradients for inner products and squared norms; Divergence/source-sink, curl/vorticity, 2D rotated-field relation, fluid stream function; Laplacian as sum of second derivatives and local average deviation; cosine/sine eigenfunction example; Hessian as second derivative matrix and quadratic approximation.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/03_vectorcalc/03_vectorcalc_slides.pdf) (pages 1–52).


#### CMU-04 - Drawing a Triangle and an Intro to Sampling

[Source video](https://www.youtube.com/watch?v=B0hsT2npIc0) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Rasterization samples triangle coverage; reconstruction emits pixel-area light; Sampling/reconstruction mismatch causes jaggies, shimmer, moire, audio pitch alias, and wagon-wheel reversal; Nyquist-Shannon and sinc apply cleanly only to band-limited signals; hard edges have unbounded frequency; Supersampling at 1, 4, 16, and 1024 samples per pixel improves receding checkerboard but cannot exactly solve general scene coverage; Point inclusion uses three oriented half-plane tests and a shared-edge convention; Raster traversal tradeoffs: incremental row walk and memory coherence versus GPU parallel sample tests in a triangle bounding box; Coarse block classification, early-ins/outs and optional recursive hierarchy help thin triangles and wasted samples.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/04_drawingatriangle/04_drawingatriangle_slides.pdf) (pages 1–79).


#### CMU-05 - Spatial Transformations

[Source video](https://www.youtube.com/watch?v=QmFBHSJS0Gw) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Linear maps as transforms; rotation, reflection, scaling, shear; invariants; matrix composition; affine translation; homogeneous coordinates; points versus vectors; perspective transform; scene graphs; instancing; SVD and transform interpolation..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Coordinate frames, transformations, and pose|Coordinate frames, transformations, and pose]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/05_transformations/05_transformations_slides.pdf) (pages 1–59).


#### CMU-06 - 3D Rotations and Complex Representations

[Source video](https://www.youtube.com/watch?v=YF5ZUlKxSgE) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Rotation invariants and degrees of freedom; Commutativity contrast: 2D vs 3D rotations; Euler-angle parameterization and gimbal lock derivation; Axis-angle matrix representation; Complex numbers as geometric 2D vectors; Imaginary unit as quarter turn; multiplication combines angle addition and magnitude multiplication; Cartesian complex product and Euler formula; Complex representation makes composed rotations and derivatives compact; Quaternion basis/algebra including noncommutativity; Scalar-vector quaternion product uses dot and cross products; Unit quaternion conjugation rotates embedded 3D vector; Axis-angle quaternion and smooth spherical linear interpolation (SLERP); Complex analysis for conformal surface parameterization and texture maps; Complex/quaternion iterative fractal examples; Lie group/log and exponential map for rotation statistics.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Coordinate frames, transformations, and pose|Coordinate frames, transformations, and pose]] · [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Curves as controllable functions|Curves as controllable functions]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Animation curves and motion paths|Animation curves and motion paths]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/06_3drotations/06_3drotations_slides.pdf) (pages 1–33).


#### CMU-07 - Perspective Projection and Texture Mapping

[Source video](https://www.youtube.com/watch?v=_4Q4O2Kgdo4) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Perspective/orthographic camera projection; view transform constructed from camera center and Gram-Schmidt basis; View-frustum clipping and near/far planes; extreme depth range causes z-fighting; Clip-space normalization matrix, homogeneous divide, screen mapping with vertical reflection; Barycentric coordinates as edge-distance/height and area ratios; reused raster half-plane values; Perspective-correct interpolation via reciprocal depth and attribute-over-depth; UV texture mapping supports color, material masks, normal maps, displacement, ambient occlusion and environment maps; Magnification uses nearest or bilinear texel filtering; minification needs footprint averaging and mipmaps; Mip selection from neighboring UV derivatives and log2 footprint; trilinear blending avoids level jumps; anisotropic filtering handles elongated footprints.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/07_texture/07_texture_slides.pdf) (pages 1–69).


#### CMU-08 - Depth and Transparency

[Source video](https://www.youtube.com/watch?v=WtYfF48Z9mA) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Depth interpolated across projected triangle with barycentrics; Z-buffer stores nearest depth per sample; initialize infinity; per-fragment test/update makes opaque rendering order independent; Z-buffer handles intersecting primitives since depth order decided independently at each sample; avoids polygon splitting/sorting; Supersampling requires depth per subpixel, then average resolved colors; General depth-buffer technique can rasterize spheres without triangulation; Alpha models visibility/opacity as scalar from 0 to 1; koala hair/coverage example; Over operator is order dependent; foreground/background compositing; Naive straight-alpha color blend leads to dark fringes; Premultiplied alpha representation stores alpha times color and alpha together; same over operation across channels; Premultiplied alpha relates to homogeneous coordinates; normalize/divide after combining; Upsample blue object stored on green background: straight alpha creates green fringe; premultiplication prevents it; Downsample leaf edges into mipmaps: premultiplication avoids muddy color; repeated compositing preserves saturated red color; Transparent depth management: sorted back-to-front works only without intersecting surfaces; depth test opaque first then disable writes for transparent; depth peeling/OIT alternatives; ray tracing easier for transparency; Pipeline summary world/view transforms, clipping, viewport, coverage, UV, texture filtering, depth test, framebuffer; GPU fixed-function clipping/filter/blend hardware; heterogeneous architecture; rasterizer and ray tracing hardware.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/08_depthtransparency/08_depthtransparency_slides.pdf) (pages 1–58).


#### CMU-09 - Introduction to Geometry

[Source video](https://www.youtube.com/watch?v=MakhXtIX2YM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Geometry as measurable shape/size/pattern/position; many encodings suit differing needs; Circle represented linguistically, implicit equation, explicit trig parameter, dynamics, polygon approximation, symmetry/curvature, tomography; Geometry classes: rotational glass, assembled engine CSG, facial expression/telepresence accuracy, cloth and fluid changing topology, multiscale temples, volumetric fur, microscopic protein shape/function; Explicit point cloud vs connected polygon mesh vs splines/NURBS/subdivision; implicit membership tests; Implicit zero-set f(x)=0, sign interior/exterior; sampling implicit surface hard but membership query easy (x=1.23 plane and unit sphere test point (3/4,1/2,1/4), squared norm 7/8); Explicit parameterized surface samples easily but membership test hard (torus point example); Polynomial implicit shapes including sphere, torus, heart; equations become difficult to author; Constructive solid geometry using union/intersection/difference trees for mechanical shapes; Blobby implicit modeling with summed Gaussian fields; level threshold changes two circles into peanut-like merging shape; Distance functions and min operation for hard union; combine/blend fields for modeling; Grid sampled level sets from signed scalar field; aliasing and storage O(n^3); medical CT/MRI and fluid water/air; narrow-band sparse grids; Fractals as self-similar detail; Mandelbrot set iteration z←z²+c and bounded-orbit membership; coloring escape time and zoom 10^-50; iterated function systems; Point clouds can represent/sample any shape and attach normals/colors but sparse gaps and connectivity impede simulation; Polygon mesh vertex plus connectivity enables simulation and adaptive detail; irregular neighborhoods/data structure complexity; Triangle mesh explicit encoding via coordinate vertices and index triples; each face maps reference barycentric simplex linearly into 3D; Higher-order basis to represent curves: Bernstein polynomial basis and local control; Bezier curves endpoints, tangent, convex hull; High degree Bezier under-controls intermediates; join low-degree pieces with knots and position/tangent continuity; glyph/vector program curves; Bezier patches as tensor products of Bernstein bases/control grid; continuity, patch topology restrictions and alternatives (NURBS, Gregory, PM, polar); Homogeneous/rational splines represent conics exactly; NURBS means nonuniform knots, rational projection, piecewise polynomial B-spline; weights control influence; Subdivision curves/surfaces via repeated split + weighted vertex averaging; Lane-Riesenfeld binomial weights produce B-spline limit; Catmull-Clark quads and Loop triangles; interpolating/approximating and irregular-vertex continuity tradeoffs.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Surfaces, patches, topology, and parameterization|Surfaces, patches, topology, and parameterization]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/09_introgeometry/09_introgeometry_slides.pdf) (pages 1–70).


#### CMU-10 - Meshes and Manifolds

[Source video](https://www.youtube.com/watch?v=HePDHsp8spU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Surface as object shell; manifold chart intuition and nonmanifold pinch/weld counterexamples; Manifold mesh fans-not-fins criteria, including boundary one-face strips and closed boundary loops; Triangle soup, indexed connectivity, incidence matrices and sparse coordinate storage tradeoffs; Half-edge fields twin/next/origin/edge/face; face and vertex traversal algorithms; invariant cycle argument; Topological validity is independent of vertex embedding: self-intersections, collapse, quotient surface and skinny triangles; Edge flip, split and collapse pointer rewrites with two-triangle assumptions; Array/linked-list locality versus neighborhood and mutation performance; subdivision control cages.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Surfaces, patches, topology, and parameterization|Surfaces, patches, topology, and parameterization]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/10_meshes/10_meshes_slides.pdf) (pages 1–36).


#### CMU-11 - Digital Geometry Processing

[Source video](https://www.youtube.com/watch?v=BwpEoZYcrwY) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Digital geometry processing extends signal processing from audio/images/video to shapes; manifold + half-edge supports local processing; Acquisition-process-fabrication geometry pipeline; scan via depth sensors and print slices; reconstruction, edit/transform/analyze, fabrication; Reconstruction sources: points; oriented points/normals; multi-view photos with color/occlusion; line-density integrals CT/MRI; Reconstruction techniques: silhouettes to visual hull, point samples/Voronoi, PDE methods, Radon inversion then marching cubes; Upsample vs downsample vs resample; image bilinear interpolation vs subdivision; random point-cloud thinning vs adaptive mesh simplification; remeshing changes element quality, not solely count; Mesh quality depends on purpose: rendering prioritizes detail, numerical simulation favors non-sliver near-equilateral triangles; sampling/aliasing can erase geometry even when sample points lie exactly on surface; Tube with spherical caps needs dense sampling in curved end caps; cylinder triangulation produces checkerboard shading, wrong normals/area despite perfect vertex locations; Triangle normal agreement helps preserve many surface properties; sliver elements cause numerical issues; Delaunay criterion empty circumcircles; maximum minimum angle; smoothest linear/barycentric interpolation (Rippa theorem); Laplace maximum principle; may trade off geometric approximation fidelity; Regular vertex degree: degree six triangles, degree four quads; supports element shape, regular computation, subdivision; topology of sphere forces extraordinary vertices; place them carefully; Subdivision criteria interpolating vs approximating, limit surface continuity/differentiability and behavior at irregular vertices; Catmull-Clark: split each face into quads; face centroid, edge point average of endpoints + adjacent face points; vertex rule (Q+2R+(n-3)S)/n using face-point mean Q, edge-point mean R, old point S, degree n; avoids new irregular vertices; Catmull-Clark produces smooth normals/curvature and glossy reflection/caustics on quad topology; starting from triangle mesh makes valence six irregular for quad subdivision and gives artifacts; Loop is for triangles; Loop: split triangle into four; edge midpoint weighted 3/8 endpoints and 1/8 opposite vertices; old vertex weights from degree-dependent u, with u=3/16 at degree 3 else 3/(8n) and center 1-nu; convex hull property; use halfedge split/flip to construct topology; Downsample using greedy edge collapse and quadratic error metric (QEM); skull examples 10x and 100x fewer triangles remain recognizable; Point-to-plane signed distance n dot (x-p); sum squared distances over neighboring triangle planes yields quadric; homogeneous coordinates turn plane into 4-vector and squared distance into U^T K U; sum constant 4x4 matrices; For collapse, combine endpoint quadrics and minimize over xyz; quadratic form x^TAx+2w^Tx+d has stationary point -A^-1w, minimum when A positive definite; singular/indefinite cases mean nonunique or saddle; QEM algorithm: per triangle plane quadric, aggregate at vertices and edges, choose lowest-cost collapse, merge quadrics and recompute local costs; reject flips by checking normal dot product; Delaunay remeshing by greedy flipping edges whose opposite angle sum exceeds pi; guaranteed convergence in planar triangulation but worst-case O(n²), no general 3D convergence; Flips can also heuristically reduce total degree deviation from six; no guarantee, irregular valences are topologically necessary, degree irregularity diffuses in practice; Laplacian smoothing: move each vertex toward neighbor mean; project displacement into tangent plane to avoid shrinking/distorting surface; four-step remeshing: split >4/3 target length, collapse <4/5, flip for degree, tangentially center; Repeated image and mesh down/up sampling causes blur/shape degradation; preserve original reference and project processed vertices to closest original surface point; motivates next lecture geometric queries.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/11_geometryprocessing/11_geometryprocessing_slides.pdf) (pages 1–47).


#### CMU-12 - Geometric Queries

[Source video](https://www.youtube.com/watch?v=GPPyUT9Pytw) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Geometry queries differ from mesh connectivity queries; closest point, inside/outside, containment, triangle intersections; Processing pipeline motivated by projecting degraded mesh vertices back to source; closest point query; Euclidean distance between points; closest point on implicit line n^T x=c derived by moving from query P along unit normal with t=c−n^T P, resulting P+(c−n^T P)n; Closest point on segment: project to supporting line; parameterize A+t(B−A); if t in [0,1] use projected point, else choose nearest endpoint; Point-to-triangle closest point cases: interior, edges, vertices; inside triangle uses rasterizer half-plane/barycentric coverage test; otherwise compare three segment results; Point-to-triangle in 3D: project to triangle plane, test extruded triangle prism via three half-spaces, then recurse to edge/vertex if outside; plane projection generalizes from line; Naive mesh closest point tests all n triangles, keeps best distance, O(n); Implicit signed distance surface: zero set, magnitude is distance; follow negative gradient (finite differences on grid) iteratively to surface; precompute/cache closest point locations for speed-memory amortization; Ray as oriented half-line r(t)=o+td, t≥0; closed surface point-inside parity from odd/even intersection count; shadows via visibility rays; collision/trajectory use; Ray-implicit sphere: f(x)=||x||²−1, substitute o+td to get at²+bt+c=0; discriminant means miss/tangent/two hits; negative roots lie behind ray origin; Ray-plane: plane n^T x=c; t=(c−n^T o)/(n^T d); negative t behind origin and zero denominator parallel/no finite hit; Ray-triangle: intersect plane, then use barycentric coordinates (signed subtriangle areas), hit iff coordinates are nonnegative and sum one; compositional reuse of rasterizer test; Ray-triangle numerical robustness matters because photorealistic images require millions of intersections; lecture example Pixar Coco frame computation at ~50 hours; Intel Embree, NVIDIA OptiX/RTX and raster+ray hybrids mentioned in 2020 context; Mesh-mesh intersection for self-collision/cloth: decompose triangle-triangle into edge-triangle; ray-plane, finite interval, point-in-triangle checks; test edges from both triangles; Continuous collision in time: treat time as extra dimension, moving triangles as space-time prisms, homogeneous coordinates extensions; preview only, hard problem; Line-line intersection as solving simultaneous linear equations; nearly parallel lines are ill-conditioned, small normal changes move intersection greatly; finite precision and robust predicates/linear algebra; Next topic acceleration generalizes binary search to higher-dimensional space.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Acceleration structures and robust queries|Acceleration structures and robust queries]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/12_geometricqueries/12_geometricqueries_slides.pdf) (pages 1–28).


#### CMU-13 - Spatial Data Structures

[Source video](https://www.youtube.com/watch?v=NF7r-pC8fFc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Refresh ray/triangle hit: plane equation, barycentric coordinates, alternative 3×3 simultaneous solve; First-hit query over n primitives; naive scan O(n); one scene AABB still O(n) worst case but can reject misses; Axis-aligned box intersection from per-axis t intervals; intersect x/y/z slab intervals and ray parameter range, empty intersection means miss; Binary search analogy: sorting costs O(n log n) once, query O(log n); amortize build over millions of queries; BVH groups primitives into disjoint leaf sets; interior node box encloses child boxes; quality depends on tightness and overlap; BVH recursive ray traversal and closest hit record; leaf tests primitives; interior tests children; visit nearer box first, then prune farther child if its near t exceeds current closest hit; Geometry partitions into primitives; candidate group balance alone not enough—tight bounds and low overlap matter more; SAH expected cost C=C_trav + P_A C_A+P_B C_B; leaf cost proportional to primitive count; estimate hit probabilities by surface area ratio (SAH); SAH assumptions random rays/no occlusion; practical heuristic approximates unknown ray workload; BVH builder: axis-aligned candidate split planes, centroid bins (e.g. 32), union bbox and primitive counts per bucket, evaluate SAH, recurse; leaf size threshold; Degenerate centroid distributions/equal bounds can defeat BVH; huge number of candidate primitive partitions precludes exhaustive search; KD tree spatial partitioning by axis-aligned planes; front-to-back traversal enables guaranteed closest first exit when hit lies inside current cell; KD trees duplicate references to primitives overlapping multiple cells; require hit point within current leaf; mailboxing caches prior primitive test; Uniform voxel grid: store overlapping primitive lists; walk ray cells using 3D analogue of line rasterization; choose grid resolution carefully; heuristic voxel count ~primitive count; Uniform grid ray traversal cost O(n^(1/3)) under uniform distribution; excels heightfields and grass, fails teapot-in-stadium/nonuniform scale; Combine grids with BVHs/KD trees for multiscale detail; hair/grass vs room geometry motivates hybrid; Quadtree/octree hierarchical spatial partitioning, four/eight child splits; easy construction/adaptation but usually slower than KD due to fixed split choices; Comparison: BVH construction cost vs traversal, dimension-independent node count and refit for moderate animation; spatial structures ordered traversal/early exit but repeated primitive tests; grids cheap but distribution-sensitive; Hierarchical acceleration applies to containment, closest-point, shape distance, N-body Barnes-Hut / fast multipole, rendering visibility and ray tracing; Rasterization coverage+depth-buffer algorithm reviewed; ray casting reverses loops, casts one 3D ray per image sample and chooses closest triangle hit; Rasterization triangle order and streaming allows unbounded scene size, uses depth storage, less sensitive to geometry organization; ray casting sample order needs whole scene+acceleration but naturally supports transparency, shadows and reflection visibility; High-performance rendering accelerates both samples/rays and geometry; rasterization good for simple fast coverage, ray methods for general visibility and photorealism.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Acceleration structures and robust queries|Acceleration structures and robust queries]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/13_spatialdatastructures/13_spatialdatastructures_slides.pdf) (pages 1–50).


#### CMU-14 - Color

[Source video](https://www.youtube.com/watch?v=77WBg0Mprt0) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Color has practical dependence on illumination, display/print gamut, scientific measurement, art and perception; Light electromagnetic oscillation; frequency and wavelength inversely related; thermal radiation from charged moving particles explains stove red→orange→yellow; thermal imaging; Visible spectrum is narrow band; UV/IR/x-ray/microwave frequencies outside vision; white is a spectral mixture; Sun output peaks in visible frequencies; Emission spectrum records source power vs frequency; absorption spectrum records fraction absorbed by material, converted to heat; plants absorb red/blue and reflect green; Observed reflected spectrum = source emission times surface reflectance at each frequency; changing lamp changes same paint appearance; Visual system: pupil/lens/retina; rods support dim-light intensity, cones support color under brighter light; approximately 120M rods vs 6M cones; brightness sensitivity exceeds chromatic sensitivity; Blind spot where optic nerve exits retina; peripheral cone density drops, color vision poor; demo with identical-shape colored items entering periphery shows context-driven perceptual fill-in; Three cone curves S/M/L are broad and overlapping, M/L near green; roughly 64% L, 32% M; sensor channel output is integral of spectrum times response curve; Three responses cannot recover arbitrary spectrum; distinct spectra with equal SML integrals are metamers; useful for printer reproduction but changes under other lighting or hyperspectral cameras; Metamerism examples: counterfeit currency/passports with UV response, art pigment authenticity/restoration, animal colors, remote vegetation identification; Color space = available palette/gamut; color model = coordinate/naming scheme; trade-offs in storage, user convenience, task processing; Additive RGB light mixing: red+green=yellow, red+blue=magenta; subtractive CMY paint/filters remove spectral power; CMYK adds black because mixing CMY cannot produce efficient deep black; HSV offers user-friendly hue/saturation/value; SML models eye; XYZ separates luminance Y from chromaticity X/Z; Lab aims perceptual uniformity; Hexadecimal HTML RGB channels encode 8 bits each; #FF6600 example is orange; Pantone and named crayon colors are discrete standards; Color operations depend on model: alpha compositing/interpolation in RGB vs HSV differ; Y′CbCr separates luma and chroma to exploit lower human color acuity; downsample Cb/Cr by 20x per dimension (400x fewer chroma samples) while preserving full luma, then bilinear upsample; Gamut differs by device; RGB display vs CMYK print produces muted out-of-gamut colors; CIE 1931 XYZ reference and xy chromaticity diagram visualize gamut; display primary mixtures form triangle; sRGB is narrower than visible gamut; MacAdam ellipses show just-noticeable differences; color-vision variation/accessibility means chosen colors must be discriminable by different observers; Color management transforms between device models/spaces; exact spectral match ideal but device gamut prevents it; ICC/profile dialogues reflect complex mapping; Gamma response: CRT luminance Y proportional to voltage V^gamma; encode inverse 1/gamma; raw cameras also require gamma correction; Visual adaptation responds to changes; negative-color American flag afterimage demo yields positive red-white-blue; upcoming radiometry/light-material topics.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/14_color/14_color_slides.pdf) (pages 1–65).


#### CMU-15 - Radiometry

[Source video](https://www.youtube.com/watch?v=5lGYm8L_rfo) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Purpose: physically meaningful intensity and color support photorealistic rendering and architecture/previsualization; geometric optics assumes straight rays and wavelengths much smaller than scene geometry, ignoring diffraction/interference; Radiant energy Q is total photon energy; photon energy hc/λ means blue photon carries more energy than red; unit tracking debugs rendering calculations; Radiant flux Φ=dQ/dt in joules/second (watts); integrating flux over time recovers energy; Irradiance E=dΦ/dA=d²Q/(dA dt) in W/m²; spatial/time density at sensor/pixel; spectral power distribution further divides by wavelength, W/m³; Radiometry distinguishes energy total, flux/time, energy density/area, irradiance/area/time; dimensional analysis provides sanity checks; Radian = arc length/radius; steradian = spherical patch area/r²; unit sphere has 4π sr; Sun and Moon each subtend about 60 microsteradians, giving comparable apparent sizes despite physical differences; Differential solid angle dω=sinθ dθ dφ; integral over sphere gives 4π; omega sometimes denotes either direction or solid angle; Cosine foreshortening: projected patch A cosθ; irradiance from fixed beam falls with obliquity; season explanation; basic shading max(0,n·L); Directional light at infinity has constant direction and no falloff; isotropic point source radiates uniformly over 4π; intensity I=Φ/(4π); Inverse-square falloff derived from constant flux over spheres of area 4πr²; doubling distance quarter irradiance; artists sometimes violate for convenience but physical renders should respect it; Radiance L is directional density of irradiance per solid angle, W m⁻² sr⁻¹, with surface cosine relationship; incoming and outgoing radiance can differ; Radiance is constant along unobstructed vacuum rays, underlies ray-based rendering and pinhole camera measurement; finite aperture integrates radiance across ray directions; Light field assigns radiance (or spectral radiance) to each ray; spherical gantry/microlens arrays capture more dimensions; postcapture refocus and shift viewpoint; tradeoff spatial resolution; Irradiance at point integrates incident L over hemisphere with cosine weighting; uniform overcast hemisphere yields E=πL before visibility; Geometry occludes rays; visibility gates integral causing shadows; ambient occlusion approximates self-shadowing under uniform environment, can bake per-surface texture or estimate in screen space from nearby depth samples; Area sources integrate incident radiance over projected solid angle; uniform disk yields projected solid angle πsin²α and soft shadows vs point light; general integrals need numerical methods; Goniometric diagrams measure source intensity vs direction and can drive physically informed lighting/architectural visualization; Photometry is human-weighted radiometry; luminance Y integrates spectral radiance times photopic response V(λ); loses wavelength/color information and response differs day vs night; Geometric optics misses small scale diffraction/iridescence and large-scale general-relativistic gravitational lensing; lensing distorts galaxy images.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/15_radiometry/15_radiometry_slides.pdf) (pages 1–63).


#### CMU-16 - The Rendering Equation

[Source video](https://www.youtube.com/watch?v=Ttxdbn7TSLI) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Spectral radiance contains direction-dependent light information; incoming vs outgoing are conventions; Irradiance integrates radiance over hemisphere with projected-area cosine; this cosine differs conceptually from Lambert cosine shading; Rendering equation: outgoing radiance = emitted radiance + hemisphere integral of incoming radiance × BRDF × cosine; recursive because incident light at hit point comes from further scene intersections; Backward ray tracing follows camera rays to target pixel and then bounces toward sources; rasterization lacks direct control over these ray integrations; emissive object provides recursion base case; Geometric-optics surface reflection leaves same side with unchanged frequency; material response determines spectral absorption and directional scattering; Ideal specular mirror reflects around normal; diffuse sends equal amount to all outgoing directions; glossy broadens mirror lobe; bicycle reflectors and Moon demonstrate retroreflection; real materials mix lobes; BRDF f_r(ω_i,ω_o) describes outgoing scattering per incoming/outgoing direction pair, is nonnegative, and hemispherical reflected energy ≤ incoming energy (remainder absorbed as heat); units sr⁻¹; Helmholtz reciprocity exchanges incoming/outgoing directions; intuitive microscopic mirror argument; Lambertian BRDF is constant ρ/π; integrating yields outgoing radiance proportional to albedo ρ and irradiance E/π; Specular direction in angles has equal incidence/reflection polar angle and opposite azimuth; vector reflection formula; ideal BRDF uses Dirac delta, cannot be found by ordinary random sample so follow exact direction; Transmission/refractive direction follows Snell law using relative indices of refraction; air approximates vacuum; Total internal reflection for denser-to-rarer medium beyond critical angle; underwater Snell window and dark reflected exterior; Fresnel reflectance grows at grazing angles; ocean horizon mirrors sky, shallow view shows underwater objects; rendered glass comparison demonstrates appearance impact; Anisotropic BRDF depends on azimuth, as in brushed metal; orientation can be stored in texture; Subsurface scattering: light enters one point, scatters inside then exits elsewhere; explains jade, skin, backlit red ears, leaves; BSSRDF generalizes BRDF by incident position/direction and exit position/direction; requires integration over surface and incoming hemisphere; improves side-lit marble; Monte Carlo approximates reflection integral from random directions with integrand/PDF weighting; more samples reduce error; importance distribution should follow BRDF lobe; Path tracing partitions direct and indirect illumination, recursively samples bounces and handles infinite paths by careful termination; mirror/glass sphere example illustrates realism from secondary/tertiary paths compared with direct-only rasterization.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/16_renderingequation/16_renderingequation_slides.pdf) (pages 1–58).


#### CMU-17 - Numerical Integration

[Source video](https://www.youtube.com/watch?v=EGNZm9m382s) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Rendering integrals aggregate illumination over directions; graphics also integrates pixel brightness, area, curvature and other quantities; Closed-form integrals often unavailable; numerical quadrature approximates integral with weighted samples; Integral equals mean function value times domain width; fundamental theorem integrates derivative to endpoint difference; Constant integrates as width×constant; affine integrates exactly via midpoint or average of endpoints; distinguish affine offset from strict linear function; Gauss quadrature chooses special nodes/weights to integrate degree-n polynomials exactly; general quadrature weighted finite function samples; Piecewise-affine integral is sum of per-interval trapezoids; a single point can miss piecewise structure and alias; nodes/weights encode rule; Trapezoid rule uses endpoint samples and piecewise-linear interpolation; error decreases O(h²) for suitable smooth functions; Tensor product application in 2D costs O(n²) with still O(h²) error; in k dimensions O(n^k), prohibitive for rendering path variables (curse of dimensionality); Monte Carlo samples random points; each run varies but expectation equals integral; accepts discontinuous functions such as silhouettes; RMS error O(N^-1/2) depends on total sample count rather than dimension; Random variable X takes outcomes; PDF/probability mass nonnegative and normalized; discrete fair die example; CDF cumulative sum has endpoints 0 and 1; Sample discrete distribution by uniform u∈[0,1] and locate CDF interval; cumulative array sorted, binary search improves linear scan to logarithmic; Continuous probability density integrates to one; interval probability is integral of density; CDF is cumulative integral and interval probability difference; Inverse transform sampling draws uniform u and applies inverse CDF; worked polynomial density p(x)=3(1−x)² yields CDF inversion x=1−(1−u)^(1/3); histogram approaches target as samples grow; Unit disk uniform sampling requires area-correct Jacobian: independent uniform θ=2πu and r=√v; uniform r biases toward center; Rejection sampling draws uniform bounding-square point and keeps only if inside disk; uniformity preserved, ≤half waste for disk; thin target region makes it inefficient; Sampling strategy balances derivation/evaluation cost and estimator accuracy for compute budget; target density can be sampled with inverse CDF or rejection, setup may be reused for common rendering materials/lights.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/17_numericalintegration/17_numericalintegration_slides.pdf) (pages 1–38).


#### CMU-18 - Monte Carlo Rendering

[Source video](https://www.youtube.com/watch?v=FUZJNlRqrAc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Photorealistic ray-traced image combines color, material/BRDF, radiometry, geometry queries, BVH acceleration, rendering equation and numerical integration; slower but handles global illumination flexibly; Rasterization processes primitive→covered screen samples with z-buffer; ray tracing reverses loop to sample→scene intersections accelerated by BVH; compare shadows, reflections, refraction/transparency, indirect light and limits of rasterization tricks; Pixel radiance integral can be viewed as integral over complete light paths; each MC sample is a path; MC estimator: integral over domain is domain measure times mean integrand under uniform sampling; curse of dimensionality motivates MC; Discrete expectation as probability-weighted outcome sum; fair coin coded heads=1/tails=0 has expectation 1/2; linearity of expectation; Variance means squared deviation from mean, not uneven probability bars; deterministic always-head coin has zero variance, more uniform outcomes spanning range can have higher variance; Var(Y)=E[Y²]−E[Y]²; independent sample mean variance σ²/N; Geometric demonstration: toss coconuts/random points into square circumscribing unit circle; count fraction inside circle to estimate π; converge toward 3.14159 as sample count rises; Importance sampling from nonuniform density p requires contribution f(x)/p(x); half-red/half-blue region sampled 8x more in red must downweight red and upweight blue to preserve purple mean; Choose proposal resembling integrand: glossy BRDF lobe or bright environment-map areas; avoid spending samples on zero/low contributions; Direct-light example: square area source, ground receiver and occluder; visibility V(P,P′) is 0/1; uniform hemisphere density 1/(2π), irradiance integral L_i cosθ dω estimated with hemisphere area times sample mean; Uniform sphere/hemisphere sampling from rejection-sampled unit cube and normalization; direct estimator queries ray tracer for occluder and light visibility; Area-light importance sampling changes integration domain from solid angle to emitter surface; Jacobian dω=cosθ_light dA/r², retaining visibility, cosine and radiance; uniform area samples weighted by area/N; Comparison of same scene and 100 samples: area-light sampling visibly much smoother than uniform hemisphere sampling; even one area sample beats uniform strategy; 16 samples look strong; variance/cost tradeoff; Cosine-weighted hemisphere PDF p(ω)=cosθ/π is normalized because hemisphere integral of cosθ is π; cancels cosine in integrand, simplifying estimator to π times average incoming radiance; Recursive path tracing samples incoming direction according to cosine or BRDF, calls recursively for radiance from next hit; expected value correct even with one sample though image noisy; Path length examples: direct light only; one/two bounces reveal indirect illumination; four bounces reveal hanging lanterns/glass via transmission/reflection events, with more bounces increasing brightness; Russian roulette chooses probabilistic continuation rather than fixed maximum bounce; low-contribution paths terminate; survivors reweighted by inverse keep probability to preserve unbiased expectation; Concrete roulette visualization: keep 50% below luminance threshold 0.25; progressively more aggressive thresholds shorten render time 6.4→5.1 seconds and increase shadow noise; tradeoff depends on scene/material/light; Summary: path estimator remains unbiased when sample PDFs/weights and randomized termination are correct; smart sample distributions strongly change image quality.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/18_montecarloraytracing/18_montecarloraytracing_slides.pdf) (pages 1–52).


#### CMU-19 - Variance Reduction

[Source video](https://www.youtube.com/watch?v=IQhLk_XaFc8) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Monte Carlo integration reviewed; discrete vs continuous random variables and probability density; probability of exact time zero but interval probability integral of density; Expected value vs mode: hypothetical sleep-time density peaks but mean 44.9 minutes falls in low-density region; flaw of averages; Variance and standard deviation definition; graphical noise across image pixels; Checkerboard function on [0,2]² with half black/white has mean 1/2 and variance 1/4; cannot reduce integrand variance, only variance of estimator; Estimator traits: unbiased means expected estimate is true integral for each sample count; consistent means probability of fixed error tends to zero as sample count grows; Fixed regular m×m Riemann grid can be consistent but biased; single uniform image-point sample scaled by area is unbiased but inconsistent if count fixed; growing MC is both; rasterization fast but neither for full physical result; Glossy coffee cup + point source creates caustic ring on diffuse table; eye→table→glossy mirror→light path has near-zero probability under forward random path sampling due delta-like direction and point source; Disco ball demonstrates compounding rare path events: camera must hit tiny lit wall patch then sample narrow mirror direction; 16→128→8192 spp slowly resolves caustic; Importance sampling: choose p near nonnegative integrand f; divide contribution by p; ideal p=f/I yields constant I but impossible because I is answer sought; material/BRDF and area-light proposals; perfect mirror/point source limiting proposals; Path-space formulation: each path encoded by sequence of unit random values in K-dimensional hypercube; F is path throughput; path length controls dimension; Russian roulette for length; Bidirectional path tracing connects eye and light subpaths; different splits capture glass egg; direct lighting connection; forward paths miss point lights, reverse miss pinhole camera; connection can miss narrow door slit; Metropolis light transport mutates existing good path; acceptance ratio target value new/old clamped at one; dependent random walk spends more steps on high contribution; ergodicity/zero-density barriers and initial burn-in concerns; Gaussian local step plus global random jumps; Image-density walk illustrates MLT: random walk visits bright pixels proportionally and reconstructs downtown Pittsburgh; pool refraction path much clearer at same time than ordinary path tracing; Multiple importance sampling balance heuristic combines light and material PDFs and counts; grid of light sizes versus mirror-to-diffuse materials shows hybrid best; Independent random samples are uniform in expectation but clump; stratify N bins with one uniform sample each, lowering estimator variance (cannot be worse under the stated setup); can stratify projected triangular light domain; Discrepancy measures difference between area fraction and sample fraction over region families; Koksma bound links discrepancy and function total variation; caveat discontinuity and axis-aligned regions; Halton samples use radical inverse digit reflection in prime bases (1234 base 10→.4321); Hammersley adds i/N, looks better but N must be known; Regular grids may alias even with low discrepancy: sample phase shift on sinusoid changes estimate between 0 and 1; Blue noise: isotropy and weak directional/frequency preference; spectrum ring reflects minimum spacing; retinal cell motivation; zone plate example and Fourier spectrum compared with random/grid-like/blue-noise patterns; Poisson disk sampling with non-overlap and Lloyd relaxation using Voronoi-cell centroids/area to make well-spaced uniform or importance-weighted samples; Discrete weighted sampling: CDF inverse with binary search O(log n); alias table preprocessing equal columns/two labels/threshold yields O(1) samples; Photon mapping stores light-origin photon hits in k-d tree then gathers neighbors; useful caustics/participating media; finite-radius estimate biased but convergent; Finite-element radiosity partitions diffuse geometry into patches and solves equilibrium linear system; mesh quality challenging and specular effects omitted; Finite sampling cannot certify no missed feature; shrinking a pinhole while brightening a source creates arbitrarily hard-to-find contribution.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/19_variancereduction/19_variancereduction_slides.pdf) (pages 1–63).


#### CMU-20 - Introduction to Animation

[Source video](https://www.youtube.com/watch?v=XVTw68iHef0) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Animation history: ancient vase motion sequence; Leonardo muscle swelling/volume; Monet movement impression; phenakistoscope slit prevents blur and uses aliasing; Muybridge horse gait answers all-feet-airborne question; early hand-drawn films, Prince Achmed silhouette puppets, Snow White first hand-drawn feature; Sutherland Sketchpad; Boeing Man, ASCII cat, Utah facial animation, The Works stalled on render time; zoetrope/strobe 3D-printed spinning sculpture; Perceptual beta phenomenon: successive still frames perceived as movement through brain visual memory; phi phenomenon anticipation cues affect perceived speed; persistence-of-vision retinal-trail account debunked/insufficient; Traditional keyframes by senior artist and in-betweens by apprentice; digital equivalent uses 3D rig and interpolates positions, colors, lights, camera, transforms; Motion source classes: keyframing, mocap from bright markers/cameras triangulated into joint tracks, and procedural algorithmic/simulation motion; Interpolation and physical spline intuition; piecewise linear has velocity jumps and singular/abrupt acceleration at corners; splines piecewise polynomials with ordered knots and exact key constraints; Cubic spline rationale: small-displacement elastic strip minimizes squared second derivative; practical closed form and continuity; NURBS exact circles/ovals; clothoids for road acceleration; high-degree polynomial Runge endpoint oscillation; Cubic endpoint fit: 4 polynomial coefficients, endpoint positions only 2 conditions leave nonunique family; endpoint positions and endpoint derivatives yield 4x4 linear system and unique Hermite curve; constraints linear in coefficients; Piecewise natural cubic spline imposes values, first and second derivative continuity; equation count 4n−2 plus two endpoint zero second derivatives; global coupled system; interpolation+C2 but nonlocal; Bezier/Hermite segment uses endpoints and tangents; control polygon and convex hull intuition; endpoint conditions determine each segment independently; interpolation+locality but not necessarily C2; Catmull–Rom infers tangent at key from two neighboring key values/time, uses Hermite segments, local and interpolatory; useful camera path starting point; Cubic B-spline basis recursively combines piecewise constant basis over knot intervals; support is local; cubic basis gives C2 continuity but does not interpolate controls; no free lunch among interpolation/smoothness/locality; Camera animation separately interpolates xyz location and orientation/frame; rig hierarchy tree of rotations/translations exposes many DOFs; spline animation of joint values automates in-betweening; Inverse kinematics specifies end effector hand target at times then solves other joint angles by numerical optimization; Skeletal animation uses rigid bone/joint transforms; per-vertex bone weights blend transformed locations via linear blend skinning; common volume-collapse issue motivates improved techniques; Blend shapes store alternate poses (happy/sad/sleepy/awake), spline coefficients over time and combine vertex positions.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Curves as controllable functions|Curves as controllable functions]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Animation curves and motion paths|Animation curves and motion paths]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/20_introanimation/20_introanimation_slides.pdf) (pages 1–55).


#### CMU-21 - Dynamics and Time Integration

[Source video](https://www.youtube.com/watch?v=Fi9xu3z97W4) · **Evidence used:** local Whisper small.en transcription plus exact 2020 slides.

**Topics used:** Contrasts spline/keyframe kinematics with force-driven dynamics and simulation, trading less manual work for more computation; Configuration q(t), velocity qdot, mass/inertia, force and constraints g(q,qdot,t)=0; generalized coordinate concatenation (six 2D billiard balls→12D configuration and matching velocity) gives a trajectory in state space; ODE depends on time derivatives only; qddot=M^-1 f(q,qdot,t); second-order equation can be split qdot=v, vdot=M^-1 f; Exponential scalar ODE udot=au solved u=be^(at); sign a>0 growth and a<0 decay; Ballistic rock under constant gravity: v(t)=v0+gt, q(t)=q0+tv0+(1/2)t^2g; force/mass acceleration distinction; Pendulum initially described by Cartesian coordinates with unit-circle constraint, then reduce to one generalized angle theta which obeys constraint automatically; Lagrangian recipe T−U; Euler–Lagrange d/dt(partial L/partial qdot)=partial L/partial q; pendulum T=1/2 m l^2 thetadot^2, U=mgl(1−cos theta) up to constant, yielding theta_ddot=−(g/l)sin(theta); Small-angle approximation sin(theta)≈theta gives harmonic oscillator theta(t)=A cos(sqrt(g/l)t)+B sin(sqrt(g/l)t); general pendulum lacks elementary closed form and needs numerical integration; Double pendulum with two coordinates exhibits chaos: small initial-state differences create very different trajectories; n-body problems similarly resist closed-form solutions; Particle/agent examples: galaxy and n-body, flocking attraction to visible/K-nearest-neighbor center, repulsion/alignment; simple force rules create emergent behavior; Particle systems represent crowds/evacuation bottlenecks, fluids, granular materials, molecular dynamics, cosmological dark matter; scalable detail; Barnes–Hut/fast multipole hierarchical aggregation of distant particles; continuum/PDE alternative; Mass-spring system energy U=1/2 k(l−l0)^2, where k stiffness, l current distance and l0 rest length; mesh edges as springs model cloth and hair, pin cloth vertices to animated body, collision detection/response prevents interpenetration; Numerical integration replaces time derivative with finite differences q_(k+1)−q_k over dt; ODE time-only derivatives, PDE adds spatial derivatives; frame intervals may require smaller internal steps; Forward Euler evaluates derivative at current state; update q_(k+1)=q_k+dt f(q_k); explicit but pendulum gains energy and spins due to numerical instability; Stability analysis on udot=−au (a>0): forward update u_(k+1)=(1−a dt)u_k, so stable only when |1−a dt|<1, i.e. 0<dt<2/a; stiff springs/hair require tiny time steps; Backward Euler evaluates at unknown next state and requires solving implicit equation; for udot=−au, factor 1/(1+a dt) makes it unconditionally stable for positive dt, but introduces artificial damping in conservative pendulum; Symplectic Euler updates velocity using current configuration and then position using new velocity; staggered update preserves pendulum energy far better over long times than standard forward/backward Euler; leapfrog and Verlet related alternatives; Integrator quality has tradeoffs among stability, accuracy, convergence, conservation, symmetry and computation cost; Derivative methods: hand derivation (fast, accurate but laborious/error-prone); finite differences (black-box/general but perturbation-size/roundoff and high function-call cost); automatic differentiation (propagate value+derivative pairs via chain/product rule, accurate/fast but requires new scalar types); symbolic differentiation (expression tree transform, reusable formula but expression swell); geometric reasoning; For scalar f:R→R derivative is limit difference quotient; for F:R^n→R directional derivative along x is scalar; gradient grad F is vector satisfying grad F dot x = D_xF; potential gradient relates to force; Finite difference step too large has truncation error; too small causes floating-point cancellation so error rises again; Forward-mode AD example f(x)=a x^2 at x=2 carries pairs (value, derivative), (2,1)^2=(4,4), multiplying by a yields (4a,4a), matching f=4a and fprime=4a; Geometric gradient example: triangle area with opposite edge e and unit normal n; moving a vertex along e changes neither base nor height, so gradient is perpendicular to e, with magnitude |e|/2, direction n cross e (sign depends on vertex orientation); simpler than expanded symbolic expression; Contact mechanics/collision response acknowledged as important but deferred.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/21_dynamicstimeintegration/21_dynamicstimeintegration_slides.pdf) (pages 1–46).


#### CMU-22 - Optimization

[Source video](https://www.youtube.com/watch?v=sDAEFFoiKZ0) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Optimization formalizes an objective (cost/loss) minimized over parameter vector x subject to feasible constraints; maximizing becomes minimizing negative objective; equality can be expressed by two opposite inequalities; Dido/isoperimetric illustration: maximize enclosed area with fixed boundary length; circle is optimal shape; graphics examples include character balance, mesh symmetry, inertia-aware fabrication, glider design, machine learning; Discrete versus continuous domains: exhaustive combinations can be exponential; minimum spanning tree has efficient Prim/Kruskal algorithms; traveling salesperson is NP-hard; continuous domains have infinitely many candidates and need calculus/optimization methods; Feasibility asks whether any parameter satisfies constraints; objective boundedness, feasible set, compactness (closed and bounded) and coercivity (cost grows without bound far away) are distinct conditions; examples: minimize x unbounded below, minimize exp(-x) bounded with unattained infimum, contradictory x=1 and x=-1 constraints, disjoint disk and half-plane; Local minimum beats nearby candidates; global is best feasible point everywhere; neither global nor local minimizer necessarily unique; examples: protein-energy basin and machine learning local solutions, path-string local minimum caught around city obstacles; A 2D saddle objective over unit disk has two distinct global minimizers (0,±1), showing local/global minima may be multiple; function visualization as height over feasible set; Unconstrained necessary local-minimum tests for twice differentiable f: gradient zero and Hessian positive semidefinite; second derivative/Hessian rejects maxima and saddles but is not sufficient alone for all edge cases; Constrained minima do not need zero objective gradient or positive semidefinite unconstrained Hessian; KKT conditions include constraint derivatives and Lagrange multipliers/dual variables; Convex set contains segment joining any pair of its points; convex objective has graph below chords; convex problems avoid spurious local minima and support global guarantees, especially when strongly convex gives unique optimum; complexity still depends on problem class; Convex quadratic objective f(x)=1/2 x^T A x−x^T b, A positive semidefinite; gradient Ax−b, Hessian A, minimizer solves Ax=b; positive definite yields bowl/unique solution, semidefinite can yield a flat set of solutions, indefinite saddle may lack a minimizer; Gradient descent as gradient-flow ODE xdot=−grad f and explicit update x_(k+1)=x_k−tau grad f(x_k); step-size selection via line search (Armijo-Wolfe conditions) or small fixed steps; too-large step overshoots/diverges, too-small wastes work; Gradient descent can oscillate slowly in narrow valleys because it follows local slope and ignores curvature/distant shape; nonconvex problems have initialization-sensitive local minima; Newton method uses inverse Hessian direction x_(k+1)=x_k−tau H(x_k)^−1 grad f(x_k), from local quadratic approximation and curvature scaling; rapid near a well-behaved minimum, but Hessian can be singular/indefinite and point toward saddle; quasi-Newton, L-BFGS, trust region alternatives approximate curvature/control steps; choose solver based on problem and empirical prior work; Forward kinematics: hierarchical chain of rigid segments connected by joints; joint types and angle limits constrain configurations; root transform propagates to children; Planar two-link chain: points p0,p1,p2 and reference offsets u0,u1; hierarchical angles theta0,theta1 yield p1=p0+R(theta0)u0 and p2=p1+R(theta0)R(theta1)u1 (or complex rotations e^(i theta)); child angle is relative to parent orientation; Inverse kinematics chooses joint variables to place endpoint near target; objective f(theta)=1/2 ||p_target−p_end(theta)||^2; compute endpoint Jacobian/gradient with respect to joint angles and use gradient descent; joint limits and other constraints may be incorporated; robot arm pick-and-place and character hand reaching as examples; Optimization converts high-level artistic intent (hand/foot reaches target) into lower-level coordinated joint rotations, reducing manual keyframing; Lecture concludes with next topic PDEs with derivatives across space and time for waves and other physical phenomena.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#PDEs, fields, and discrete solvers|PDEs, fields, and discrete solvers]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/22_optimization/22_optimization_slides.pdf) (pages 1–32).


#### CMU-23 - Physically Based Animation and PDEs

[Source video](https://www.youtube.com/watch?v=gqdXsw_Q_as) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** PDE seeks a function of spatial position and time, with spatial and temporal derivatives; contrasts ODEs (time derivatives only); illustrative ripple wave satisfies height acceleration proportional to Laplacian; PDE categories: linearity vs nonlinearity (Burgers term u du/dx nonlinear); order counts highest space/time derivative; canonical elliptic Laplace Δu=0, parabolic heat u_t=Δu, hyperbolic wave u_tt=Δu; elliptic harmonic interpolation, heat diffusion/viscosity and steady state, hyperbolic wave propagation preserves and propagates error; Burgers equation u_t+u u_x=αu_xx models shock formation balanced by diffusion; nonlinear and first-order in time/second in space; Applications shown: liquids with density changes, vorticity-based wispy smoke, cloth wrinkles/buckles, elastic bodies, hair, fracture past yield, viscoelastic permanent deformation, snow via mixed particle/grid Material Point Method; real-time GPU fire and water; Wave demo on 128x128 grid stores height u and velocity v, damps by factor α, randomly injects stone impulses, estimates Laplacian by four neighbors minus four times center with periodic wraparound, updates v then u; final colors/height visualization; Lagrangian discretization tracks material particles through space; flexible bounds and intuitive particle representation but neighbor search/distribution are difficult; Eulerian discretization fixes grid locations and measures fields as material passes, offering regular/cache-friendly operations but grid-bound, resolution blur and aliasing; hybrid FLIP, material point, particle-level set combine both; Solve-for-variable choice matters: velocity vs vorticity (curl of velocity) can alter efficiency and smoke detail; general pipeline choose formulation, spatial discretization, time discretization, then update equations; Laplacian Δu=div(grad u)=sum of coordinate second derivatives; coordinate-independent; measures deviation from local neighbor average and vanishes for locally harmonic/flat field; Finite differences: forward slope (u_(i+1)−u_i)/h; second derivative (u_(i+1)−2u_i+u_(i−1))/h²; 2D 5-point Laplacian (four neighbors−4center)/h²; mesh cotangent Laplacian sums neighbor differences weighted by (cot α+cot β), connecting to regular-grid stencil; Laplace Δu=0 yields harmonic function, each interior value equals neighbor average; Jacobi iteration repeatedly averages but converges slowly across large grids; index grid values, assemble sparse matrix with 4 on diagonal and −1 neighbors and use sparse linear solver; Boundary conditions are essential: periodic wraparound is one choice; Dirichlet fixes value, Neumann fixes normal derivative, Robin combines value/derivative; boundary choices encode desired interpolation data; In 1D Laplace φ''=0 solutions are affine c x+d; arbitrary endpoint values can be interpolated but independently chosen endpoint slopes must agree; in 2D arbitrary Dirichlet data can intuitively relax via heat flow to harmonic steady state; For harmonic solution with Neumann data, divergence theorem requires net boundary flux integral ∫∂Ω n·∇φ ds=0; solution also determined only up to additive constant without a pinned value; check numerical residual b−Ax to detect inconsistent setup rather than trusting solver blindly; Heat PDE u_t=Δu with explicit Euler u_(k+1)=u_k+Δt Δ_hu_k diffuses initial heat; at steady state reduces to Laplace; Wave PDE u_tt=Δu can be integrated by introducing velocity v=u_t and evolving u_t=v, v_t=Δu, or centered second difference in time from two prior states; need initial displacement and velocity (or two initial frames); same spatial Laplacian works on grid and triangle mesh, e.g. cotangent formula; General PDE algorithms can transfer between representations by changing the spatial operator; position-based thin-shell low-res simulation plus Loop subdivision upsampling yields interactive wave-like mesh behavior; The course warns detailed physically based simulation methods require computational physics, applied mathematics, geometry and numerical analysis foundations.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Mathematical language: points, vectors, and derivatives|Mathematical language: points, vectors, and derivatives]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#PDEs, fields, and discrete solvers|PDEs, fields, and discrete solvers]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Exact Fall 2020 slides](https://15462.courses.cs.cmu.edu/fall2020content/lectures/23_pdes/23_pdes_slides.pdf) (pages 1–49).


#### CMU-SHOWCASE - Student creations and project outcomes

[Source video](https://www.youtube.com/watch?v=yJ5eY3EIImA) · **Evidence used:** sampled visual showcase.

**Topics used:** Student project showcase includes raster line art and pixel art; procedural fractals and trees; mesh editing and subdivision modeling; physically based path tracing with visible glass, reflections, environment lighting and noisy renders; bouncing ball animation; spline interpolation, blended skinning, inverse kinematics, articulated figures and instancing..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Project synthesis]].

### Computer Graphics — MIT 6.837, Fall 2021

[Playlist](https://www.youtube.com/playlist?list=PLQ3UicqQtfNtqt2yL3KgKV-yn0NEPbRVi)

#### MITG-01 - Introduction

[Source video](https://www.youtube.com/watch?v=tPzC879i-ag) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Course framing and scope; technical graphics as algorithms connecting scene descriptions to images; offline film versus interactive frame budgets; geometry representations and modeling; appearance, materials, lighting, animation, simulation, ray tracing and rasterization; pixel grids, sampling and aliasing; global illumination and shadows; color perception, RGB and displays; application contexts including film, games, CAD/CAM, architectural analysis, engineering, AR/VR, scientific visualization, and medical imaging..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]].

#### MITG-02 - Cubic curves

[Source video](https://www.youtube.com/watch?v=1BZ0ayVjWEk) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Parametric curves and alternate shape representations; line/circle examples; tessellation and adaptive sampling motivation; polynomial function spaces; monomial basis limitations; cubic Hermite interpolation from endpoint values and derivatives; matrix change of basis; separating a 2D/3D curve from its parameter speed; cubic blossom definition by symmetry, separate affinity, and diagonal property; blossom of powers and linear combinations; Bernstein basis; Bézier control points; de Casteljau evaluation and exact recursive subdivision; geometry, spline, and monomial matrices; preview of piecewise polynomial splines and continuity needs..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Curves as controllable functions|Curves as controllable functions]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-03 - Curves and surfaces

[Source video](https://www.youtube.com/watch?v=LzIjSdNoR9s) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** De Casteljau review and parameterization; splines as piecewise polynomials; regular curves, unit tangents, curvature vectors, normal vectors, and osculating circles; parameter versus geometric continuity (C0/G1/C1/C2/G2); cusps and regularity; Bézier join constraints and editing dependencies; uniform cubic B-spline local control, three-control overlap, C2 joins, repeated controls/knots reducing continuity, and convex-hull/local influence; NURBS and exact rational circles; surface representation alternatives (triangle meshes, tensor-product splines, subdivision, implicit and procedural geometry); bicubic tensor-product Bézier patches and 16 controls; boundary matching and seam continuity; surface partial derivatives and unit normals from their cross product; patch trimming and displacement mapping; corner cutting/Chaikin’s algorithm; Loop and Catmull–Clark subdivision; adaptivity and extraordinary vertices; implicit level sets for merging fluid topology..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Curves as controllable functions|Curves as controllable functions]] · [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization#Surfaces, patches, topology, and parameterization|Surfaces, patches, topology, and parameterization]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-04 - Transformations

[Source video](https://www.youtube.com/watch?v=WqmL_e7OGeU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Coordinate frames and changes of basis; abstract vectors versus coordinate tuples; linear maps represented by matrices acting on coordinates; translations are affine rather than linear; points and vectors in affine spaces; affine frames and homogeneous point/vector coordinates; transformation composition; 4×4 affine matrices; homogeneous coordinates up to nonzero scale; points at infinity/directions; pinhole camera and image-plane projection; dehomogenization as divide by depth; camera frame and coordinate changes; normal transformation derived by preserving perpendicularity, using inverse transpose and normalization; singular-transform caveats; handedness and programmable pipelines..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#Coordinate frames, transformations, and pose|Coordinate frames, transformations, and pose]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-05 - Hierarchical modeling

[Source video](https://www.youtube.com/watch?v=-Dhx44D0Iok) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Kinematic chains; joint degrees of freedom, limits, and transforms; forward kinematics; rotation representations and Euler-angle gimbal lock; quaternion double cover; hierarchical scene graphs; depth-first traversal and accumulated world transforms; matrix stacks and push/pop rather than inverse-on-ascend; world and camera transform separation; inverse kinematics as nonlinear inversion of end-effector position; two-link reach and multiple solutions; Jacobian/Taylor linearization and Newton iteration; initialization/conditioning/failure cases; mesh-based IK over example poses; transformation of surface normals..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-06 - Animation:  Skinning/enveloping

[Source video](https://www.youtube.com/watch?v=oAQkIAGKvUI) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Traditional cel animation and in-betweening; keyframing and time splines; squash-and-stretch, anticipation, and stylized motion; procedural animation for clocks, crowds, and generated environments; physically based animation for secondary motion, with control/fidelity tradeoffs; motion capture and retargeting; skeleton posing and mesh deformation; skinning/enveloping terminology; skeletal-subspace/linear-blend skinning; per-vertex per-bone weights as positive partition of unity; bind/rest pose and bone matrices; inverse bind transforms; weighted transform candidates; painting weights and automatic/geometric weight methods; sparse influences; blend-of-rotations volume loss, candy-wrapper artifacts, and dual-quaternion alternatives; automatic rigging overview..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Scene graphs and articulated geometry|Scene graphs and articulated geometry]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-07 - Particle systems

[Source video](https://www.youtube.com/watch?v=-PGuhbZ2khE) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Physically based animation versus authored particle rules; particle-system lifecycle (emission, state, forces, lifetime/removal); first-order ODEs and reduction of Newton second-order motion by adding velocity state; phase space; forward Euler and time-step error/stability; point particle approximations and nonphysical artistic force fields; gravity, drag, pairwise force cost O(N^2), fast multipole aggregation for far-field interactions at roughly O(N log N), Leonard-Jones 6-12 potential as cohesion/repulsion model; SPH and Lagrangian versus Eulerian fluid representations; one-way/two-way coupling and hybrid fluid/spray; Boids neighborhood rules (separation, alignment, cohesion), predators; Poisson-like emission, particle editors and spline-authored behaviors; billboard sprites/textured particles and velocity-aligned spark streaks..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-08 - Cloth simulation

[Source video](https://www.youtube.com/watch?v=PJ8VzufNZ8o) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** ODE integration accuracy versus stability; forward Euler amplification and step restriction for x dot=-kx; Taylor expansion and local/global error order; stiffness and fast/slow scales; CFL condition; explicit versus implicit integration; trapezoid and midpoint methods, RK4, adaptive steps; backward Euler implicit equation, Newton solve, unconditional stability for scalar decay and numerical viscosity; mass-spring systems and Hooke force; spring network forces add; hair curvature, curvature springs, twist/buckling and discrete elastic rods; cloth mesh anisotropy; structural/shear/flexion spring families; stiffness, damping, refinement convergence and time-step limits; pinned vertices, contact/collision constraints; double pendulum coordinate reduction and angular torus state..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-09 - Ray casting

[Source video](https://www.youtube.com/watch?v=mJHGcDc37kY) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Pinhole and orthographic cameras; image plane, field of view, aspect ratio, normalized coordinates and view frustum; ray parameterization; plane equation and signed distance; ray-plane intersection and parallel case; sphere implicit equation and quadratic roots/discriminant; nearest positive root; sphere normal; rendering cost as pixels times primitives..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-10 - Ray casting II

[Source video](https://www.youtube.com/watch?v=gDayCevuE-U) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Barycentric coordinates as affine weights; area-ratio interpretation; 2x2 coordinates of a planar point and 3x3 ray-triangle solve; determinant degeneracy; barycentric interpolation of colors, normals and material parameters; object interface; CSG interval sets and Boolean operations; self-shadow epsilon; instancing and inverse transformed rays; inverse-transpose normals; ray-tracing cost..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-11 - Ray tracing

[Source video](https://www.youtube.com/watch?v=UL54Rf2O9RM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Secondary shadow/reflection/refraction rays; ambient and direct lighting; binary visibility, self-shadow bias and early-out; mirror reflection vector; Fresnel angle dependence and Schlick approximation; Snell vector derivation; total internal reflection; wavelength-dependent dispersion and rainbows; caustics and inverse design; rough reflection via sampling and noise; area lights and penumbra; antialiasing/motion blur/depth of field sampling costs; recursion cutoff and ray trees; work imbalance across pixels..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-12 - Ray tracing II

[Source video](https://www.youtube.com/watch?v=mr_fHXW2YxE) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Ray count and cost factors; conservative bounding volumes; axis-aligned box slab test using maximum entry and minimum exit; construction amortization; triangle/sphere bounds; BVH split heuristics and traversal; BVH overlap vs unique primitive assignment; KD-tree spatial partitions, triangle duplication and front/back traversal using parent t intervals; early termination and valid interval tests; median and surface-area heuristics; ray marching in uniform grids with DDA stepping, repeated primitive tests and cell-size tradeoffs..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries#Acceleration structures and robust queries|Acceleration structures and robust queries]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-13 - Shading

[Source video](https://www.youtube.com/watch?v=0xmeaXs5-ZU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Radiometry intuition; inverse-square point-light attenuation and practical clamped/spline falloff; Lambert cosine; spotlights and angular profiles; BRDF definition/domain/assumptions; isotropic vs anisotropic materials; BRDF measurement with gonio-reflectometer and data fitting; Lambertian BRDF; ideal mirror delta limitation; Phong diffuse/specular/ambient and nonphysical energy behavior; Blinn/Torrance half vector and Fresnel effects; microfacet normal distributions, masking/occlusion; model fitting and compound lobes; vertex-normal interpolation and renormalization; spatially varying BRDF parameters..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-14 - Texture mapping

[Source video](https://www.youtube.com/watch?v=TQTciMjuxrY) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Viewer-dependent visibility culling vs generic BVH/KD structures; BRDF parameters varying over surfaces; UV parameterization and seam tradeoffs; barycentric 3D-to-UV lookup; discontinuous UVs at shared vertices; bilinear reconstruction; minification and mipmapping/memory overhead; anisotropic footprints; manual, analytic, projected, and optimized UV generation; injectivity and overlap; Tutte weighted-average linear system; artist semantic seam placement; tiling; arbitrary parameter/material maps; tangent-space normal mapping and limitations; procedural shaders; Perlin noise, octave composition, threshold/ridge/marble patterns..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/19 - Textures as Sampled Surface Signals#Textures and surface detail|Textures and surface detail]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-15 - Antialiasing

[Source video](https://www.youtube.com/watch?v=eHXDosme1eo) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Sampling vs quantization; pixel and display reconstruction; spatial/temporal aliasing, jaggies, moire and wagon-wheel effect; sampling/reconstruction duality; Fourier series/transform as inner products with sinusoids; frequency-domain filtering; bandwidth/Nyquist rate; sampling creates shifted spectral copies; aliasing as overlap; ideal sinc reconstruction and convolution; practical finite filters; oversampling/downsampling; box vs Mitchell bicubic filter; image-space quality/perception; mipmapping/anisotropic footprint relation; font and RGB subpixel ClearType anti-aliasing..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-16 - Global illumination

[Source video](https://www.youtube.com/watch?v=xjBwPxU7tUk) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Direct vs indirect illumination and backward ray tracing; reflectance equation and rendering equation as coupled integral system; emitted radiance; global illumination effects (color bleed, soft shadows, caustics); Monte Carlo ray tracing recursive branching; path tracing one random continuation per path and many paths per pixel; shot noise and variance; irradiance caching in screen space with geometric/normal checks and adaptive samples; photon mapping as forward light-particle pass followed by camera pass and spatial lookup; subsurface scattering/BSSRDF; deterministic quadrature curse of dimensionality; Monte Carlo error O(1/sqrt(N)); importance sampling with probability compensation; BRDF importance sampling; stratification/jitter; bias-variance considerations..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities#The rendering equation and light transport|The rendering equation and light transport]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-17 - Rasterization

[Source video](https://www.youtube.com/watch?v=aCTEiX59ERM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Real-time frame budget/latency; rasterization vs ray tracing; pipeline and memory model; projection and view/model transformations; scan conversion/fragments; triangle setup and edge half-planes; bounding-box candidates; clipping, frustum culling and homogeneous rasterization; vertex/fragment shaders; depth/Z buffer and double buffering; Z-fighting precision; painter algorithm failure; preview of perspective-correct barycentrics..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-18 - Rasterization II

[Source video](https://www.youtube.com/watch?v=bZAu7QtyYvo) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Perspective-correct interpolation via homogeneous camera coordinates and inverse triangle matrix; screen vs 3D barycentrics; depth interpolation; supersampling and jitter; multisampling separates subpixel coverage from shading; mipmapping levels and trilinear filtering; texture footprint Jacobian and anisotropy; ellipsoidal filter approximation; transparency depth policy; global illumination precomputation; deferred shading, pre-Z pass, tile-based rendering..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Rasterization, coverage, and depth|Rasterization, coverage, and depth]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-19 - Shadow maps

[Source video](https://www.youtube.com/watch?v=jDw_EbXv-1I) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Light visibility as camera visibility from light; two-pass shadow mapping; light-space depth map and camera-space query; compare fragment light depth with nearest light depth; bias/self-shadowing; shadow map resolution and aliasing; hardware PCF/multiple samples; shadow volumes via stencil buffer and front/back z-fail logic; silhouette extrusion/infinite frustum; tradeoffs; cascaded shadow maps and view-dependent precision; deep shadow maps for smoke, hair and fractional transmittance..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections#Visibility queries and ray intersections|Visibility queries and ray intersections]].

#### MITG-20 - Color

[Source video](https://www.youtube.com/watch?v=aIVVBZ7cJBc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Visual response and computational allocation; rods/cones and foveal distribution; foveated/attentive displays and eye-tracking latency; spectral power distribution and cone response integrals; tristimulus projection and metamers; illuminant/material interaction and color constancy; color deficiency; additive RGB display synthesis, subtractive CMY(K) printing; XYZ representation and primary mixing as linear algebra; luminance/chroma processing; nonlinear gamma encoding and linear-light computation..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]].

**Supporting materials:** [Fall 2012 supplemental OCW notes](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/pages/lecture-notes/).


#### MITG-21 - Image processing

[Source video](https://www.youtube.com/watch?v=WWLxaVzkneI) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Per-pixel color transforms and tone mapping; alpha/matting and compositing, premultiplied alpha; HDR exposure fusion, registration and motion ghosts; local contrast enhancement/CLAHE; resampling/downsampling and aliasing; bilinear and edge-aware upscaling; moving least squares warps; convolution/correlation and kernel examples; Laplacian edge response; direct and Fourier convolution cost; Gaussian separability and repeated box filters via central-limit intuition; bilateral edge-preserving filtering; median filter for outliers/speckle..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]].

#### MITG-22 - Output devices

[Source video](https://www.youtube.com/watch?v=Z5693mG_iCE) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** CRT electron-beam raster/phosphor and magnetic deflection; LCD polarization/twisted nematic/backlight and subpixel RGB geometry; ClearType subpixel antialiasing; direct LED and plasma panels; DLP micromirror duty cycle and RGB color wheel; persistent reflective e-ink; stereo and head-mounted VR; monocular/binocular depth cues and discomfort; lenticular/N-view and light-field displays; swept-volume and static volumetric displays; Pepper’s Ghost and phone-camera AR..

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]].

### Machine Vision — MIT 6.801, Fall 2020

[Playlist](https://www.youtube.com/playlist?list=PLUl4u3cNGP63pfpS1gV5P9tDxxL_e4W8O)

#### MITV-01 - Introduction to Machine Vision

[Source video](https://www.youtube.com/watch?v=tY2gczObpfU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** machine vision as image-to-world description linked to action; forward/inverse imaging, ill-posedness, least-squares and noise gain; shape-from-shading constraint counting; focus of expansion and time-to-contact demonstrations; camera intrinsics and camera-to-robot extrinsic calibration; pinhole camera obscura; radiometry, albedo and foreshortening; photometric stereo as adding independent light constraints; extended Gaussian image introduced for orientation/shape recognition.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference#A scene description becomes an image|A scene description becomes an image]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]].

#### MITV-02 - Image Formation, Perspective Projection, Time Derivative, Motion Field

[Source video](https://www.youtube.com/watch?v=uM5AoUgCA00) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** perspective projection; time derivatives and motion field; focus of expansion; brightness constancy and optical flow equation.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 9-12; relevant lecture section headed Lecture 2..


#### MITV-03 - Time to Contact, Focus of Expansion, Direct Motion Vision Methods, Noise Gain

[Source video](https://www.youtube.com/watch?v=t9w_WuyVJ-A) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** time to contact; focus of expansion; direct motion methods; noise gain.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 13-18; relevant lecture section headed Lecture 3..


#### MITV-04 - Fixed Optical Flow, Optical Mouse, Constant Brightness Assumption, Closed Form Solution

[Source video](https://www.youtube.com/watch?v=3NarS3QpaU0) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** fixed optical flow; optical mouse; constant brightness; least-squares closed-form solution; multiscale sampling.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 19-25; relevant lecture section headed Lecture 4..


#### MITV-05 - TCC and FOR MontiVision Demos, Vanishing Point, Use of VPs in Camera Calibration

[Source video](https://www.youtube.com/watch?v=SSlPV1iYjHw) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** time-to-contact and focus-of-expansion demonstrations; vanishing points; camera calibration from vanishing points.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): 26-32.


#### MITV-06 - Photometric Stereo, Noise Gain, Error Amplification, Eigenvalues and Eigenvectors Review

[Source video](https://www.youtube.com/watch?v=OOrT17RsVxE) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** photometric stereo; conditioning and noise gain; eigenvalues and eigenvectors.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Surface orientation from illumination|Surface orientation from illumination]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 33-35; relevant lecture section headed Lecture 6..


#### MITV-07 - Gradient Space, Reflectance Map, Image Irradiance Equation, Gnomonic Projection

[Source video](https://www.youtube.com/watch?v=qqtEQsBjh28) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** gradient space; reflectance maps; image irradiance equation; gnomonic projection.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Surface orientation from illumination|Surface orientation from illumination]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 36-39; relevant lecture section headed Lecture 7..


#### MITV-08 - Shading, Special Cases, Lunar Surface, Scanning Electron Microscope, Green's Theorem

[Source video](https://www.youtube.com/watch?v=LP7z6vVmnms) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Lambertian and Hapke reflectance; lunar surface imaging; scanning electron microscopy; Green theorem in photometric stereo.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 40-45; relevant lecture section headed Lecture 8..


#### MITV-09 - Shape from Shading, General Case - From First Order Nonlinear PDE to Five ODEs

[Source video](https://www.youtube.com/watch?v=J4gbaRyWpoM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** shape from shading as a first-order nonlinear PDE; characteristics and five ODEs.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 46-51; relevant lecture section headed Lecture 9..


#### MITV-10 - Characteristic Strip Expansion, Shape from Shading, Iterative Solutions

[Source video](https://www.youtube.com/watch?v=XSUjp-cDyeY) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** characteristic strip expansion; iterative shape-from-shading solution.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Shape from shading and reflectance|Shape from shading and reflectance]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 52-58; relevant lecture section headed Lecture 10..


#### MITV-11 - Edge Detection, Subpixel Position, CORDIC, Line Detection (US 6,408,109)

[Source video](https://www.youtube.com/watch?v=kjCQ2DUn55Y) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** edge localization and subpixel position; line detection; CORDIC gradient direction.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 59-64; relevant lecture section headed Lecture 11..


#### MITV-12 - Blob Analysis, Binary Image Processing, Green's Theorem, Derivative and Integral

[Source video](https://www.youtube.com/watch?v=jLvgNmGuxGk) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** blob analysis and binary image processing; Green theorem for region moments; derivatives and integrals as convolutions.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 65-72; relevant lecture section headed Lecture 12..


#### MITV-13 - Object Detection, Recognition and Pose Determination, PatQuick (US 7,016,539)

[Source video](https://www.youtube.com/watch?v=lVfm6C51t_Y) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** binary templates and normalized correlation; industrial object recognition and pose; PatQuick.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 73-77; relevant lecture section headed Lecture 13..


#### MITV-14 - Inspection in PatQuick, Hough Transform, Homography, Position Determination, Multi-Scale

[Source video](https://www.youtube.com/watch?v=6SvDRpQeZrA) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** PatQuick inspection; Hough transform; homography and position; multiscale search.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 78-83; relevant lecture section headed Lecture 14..


#### MITV-15 - Alignment, PatMax, Distance Field, Filtering and Sub-Sampling (US 7,065,262)

[Source video](https://www.youtube.com/watch?v=R1yLwFOfwwc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** PatMax alignment; distance fields; filtering and subsampling.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 84-92; relevant lecture section headed Lecture 15..


#### MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)

[Source video](https://www.youtube.com/watch?v=tblzZhYZtvU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** fast convolution; low-pass filter approximations; integral images.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion#Optical flow and motion from images|Optical flow and motion from images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 93-105; relevant lecture section headed Lecture 16..


#### MITV-17 - Photogrammetry, Orientation, Axes of Inertia, Symmetry, Orientation

[Source video](https://www.youtube.com/watch?v=R8O0vlHBeNc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** photogrammetry; axes of inertia and symmetry; absolute, relative, interior and exterior orientation.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 106-114; relevant lecture section headed Lecture 17..


#### MITV-18 - Rotation and How to Represent It, Unit Quaternions, the Space of Rotations

[Source video](https://www.youtube.com/watch?v=uiG6KY0Cqd8) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** rotation representations and singularities; Hamilton quaternion algebra and noncommutative multiplication; unit-quaternion vector rotation and composition; rotation space S3 with antipodal identification; closed-form quaternion absolute orientation eigenproblem; rotation-space sampling and computational tradeoffs.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 115–127; relevant lecture section headed Lecture 18..


#### MITV-19 - Absolute Orientation in Closed Form, Outliers and Robustness, RANSAC

[Source video](https://www.youtube.com/watch?v=kigLhcTm_gg) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** stable rotation representation conversion; absolute orientation with unknown common scale; symmetric scale objective and reciprocal inverse; Rayleigh quotient quaternion eigen-solution; minimal correspondences and degeneracy; coplanar orientation decomposition; least squares robustness and RANSAC; uniform/random and regular rotation-space sampling.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 128–139; relevant lecture section headed Lecture 19..


#### MITV-20 - Space of Rotations, Regular Tessellations, Critical Surfaces, Binocular Stereo

[Source video](https://www.youtube.com/watch?v=Mcy-LlcD8jI) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** sphere/rotation tessellation and symmetry; quadrics and critical surfaces; epipolar geometry and search reduction; relative orientation five-DOF and monocular scale ambiguity; coplanarity condition and essential correspondence constraint; closest approach of skew rays and cheirality; image-space versus world-space residuals and iterative weighting; quaternion parameterization, symmetry and nonlinear optimization; critical surface examples.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 140–149; relevant lecture section headed Lecture 20..


#### MITV-21 - Relative Orientation, Binocular Stereo, Structure, Quadrics, Calibration, Reprojection

[Source video](https://www.youtube.com/watch?v=-n0cXgPSwOw) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** critical quadrics and ruled one-sheet hyperboloid; planar critical configurations; radial distortion and plumb-line calibration; camera intrinsic/extrinsic parameter coupling; Tsai-style closed-form initialization; homogeneous linear systems and scale choice; stable recovery of proper rotation; planar versus volumetric calibration targets; nonlinear reprojection optimization and local minima; Monte Carlo noise gain.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 150–159; relevant lecture section headed Lecture 21..


#### MITV-22 - Exterior Orientation, Recovering Position & Orientation, Bundle Adjustment, Object Shape

[Source video](https://www.youtube.com/watch?v=x4F1oqm7ETk) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** exterior orientation/resection from 3-D control points and image rays; six degrees of freedom and minimum three correspondences; tripod/ray ranges from triangle laws and three-sphere camera-center intersection; mirror ambiguity and above-ground/cyclic-order disambiguation; attitude as rotation matching camera/world direction triples; orthonormality issue; RANSAC for outlier correspondences; bundle adjustment of camera poses, 3-D points, intrinsics and radial distortion; image-plane reprojection residuals and Levenberg–Marquardt/local-minimum sensitivity; feature matching and drone caldera reconstruction example; mesh and generalized-cylinder representations/nonuniqueness; Minkowski area-weighted face-normal vectors and closed-polyhedron zero-sum condition; Gaussian map and EGI density as inverse curvature; 2-D circle/ellipse derivation, inversion, orientation signature; 3-D Gaussian curvature and signed saddle interpretation; integral curvature at nonsmooth edges/corners.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]].

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 160–170; relevant lecture section headed Lecture 22..


#### MITV-23 - Gaussian Image, Solids of Revolution, Direction Histograms, Regular Polyhedra

[Source video](https://www.youtube.com/watch?v=s9wu_z8EKAc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Gaussian map and extended Gaussian image area-density meaning; integral curvature for corners/edges and cube vertex example; zero centroid first-moment constraint for valid closed EGI; sphere and ellipsoid EGI; parameterized normals and curvature extrema; rotation equivariance for recognition and orientation; solids-of-revolution Gaussian-curvature derivation from narrow bands; curvature formula for r(s) and r(z), sphere verification; torus normal multiplicity, signed saddle curvature, area-mass convention and descriptor nonuniqueness; discrete EGI from triangle normals and areas; directional histograms in fibers, axons, vessels and tissue; spherical-bin quality, nearest-center dot-product assignment and computation tradeoffs; dodecahedral grid and rotation-induced bin permutation; regular/Archimedean polyhedra segue.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]].

### Digital Photography — Marc Levoy, Google 2016, based on Stanford CS178

[Playlist](https://www.youtube.com/playlist?list=PL7ddpXYvFXspUN0N-gObF1GXoCA-DA-7i)

**Supporting materials:** [Fall 2020 combined lecture notes](https://ocw.mit.edu/courses/6-801-machine-vision-fall-2020/9ec44f69e72995c370b3797818ba01e6_MIT6_801F20_lectureNotes.pdf): Official lecture notes PDF pp. 171-184; relevant lecture section headed Lecture 23..


#### PHOTO-01 - Photography session 01

[Source video](https://www.youtube.com/watch?v=y7HrM-fk_Rc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Perspective and viewpoint; Pinhole and camera obscura; Field of view and focal length; Exposure, shutter and motion blur; Lens aperture and f-number.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Image formation](https://drive.google.com/file/d/0B6w2XfPHEcZYdVd4anZzZTc3aE0/view?usp=sharing) (pages 1-66).


#### PHOTO-02 - Photography session 02

[Source video](https://www.youtube.com/watch?v=d_sJFslHQsg) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Exposure as a creative constraint; ISO and sensor sensitivity; History of photography and changing camera practices; Extreme photography and computational methods.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Choosing a camera](https://drive.google.com/open?id=0B6w2XfPHEcZYQlF2U1VjRHRNLUE) (pages 1-11) · [History of photography (part 1)](https://drive.google.com/open?id=0B6w2XfPHEcZYcm1nc0lRV0h4dzg) (pages 1-25) · [Extreme photography](https://drive.google.com/file/d/0B6w2XfPHEcZYVTZ5YzFzdXc3b0U/view?usp=sharing) (pages 1-81).


#### PHOTO-03 - Photography session 03

[Source video](https://www.youtube.com/watch?v=Q-hIFIHay9I) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Refraction and image formation; Thin-lens equation; Virtual images and close focus; Circle of confusion and depth of field.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Optics I: Lenses and apertures](https://drive.google.com/file/d/0B6w2XfPHEcZYT0dwaFF4emd6Vzg/view?usp=sharing) (pages 1-81).


#### PHOTO-04 - Photography session 04

[Source video](https://www.youtube.com/watch?v=Y6kwPir0uMM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Hyperfocal distance and depth of field; Bokeh and occlusion; Lens aberrations; Diffraction and Airy disc; Wide-angle and telephoto lens design.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Optics II: Practical photographic lenses](https://drive.google.com/open?id=0B6w2XfPHEcZYTnZIR2VpWThHM0U) (pages 1-48).


#### PHOTO-05 - Photography session 05

[Source video](https://www.youtube.com/watch?v=VCcDJAiObEw) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Photography as technology and historical practice; Wet-plate and landscape records; Composition: line, form, pattern, texture, tone and color; Balance, framing, negative space and directional structure.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [History of photography (part 2)](https://drive.google.com/open?id=0B6w2XfPHEcZYNm5MczhBTkxsanc) (pages 1-31) · [Composing good photographs](https://drive.google.com/file/d/0B6w2XfPHEcZYSkZXWG9mSGpFMVk/view?usp=sharing) (pages 1-82).


#### PHOTO-06 - Photography session 06

[Source video](https://www.youtube.com/watch?v=njlDb9-6aOU) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** SLR viewing and autofocus hardware; Phase-detection and contrast autofocus; Exposure metering and 18-percent gray; Zone system and expose-to-the-right; RAW, JPEG and tone response.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Autofocus (AF)](https://drive.google.com/open?id=0B6w2XfPHEcZYQnR3SG43Z2VIRmM) (pages 1-50) · [Exposure metering (AE)](https://drive.google.com/open?id=0B6w2XfPHEcZYbG9EWlhacm80SFE) (pages 1-28).


#### PHOTO-07 - Photography session 07

[Source video](https://www.youtube.com/watch?v=02tgqvQzp0Y) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Spatial frequency and Fourier representation; MTF and resolution; Nyquist sampling and aliasing; Prefiltering and reconstruction; Sampling, quantization and dithering.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Sampling and pixels](https://drive.google.com/open?id=0B6w2XfPHEcZYc1lhZjNHU1NNVDQ) (pages 1-66).


#### PHOTO-08 - Photography session 08

[Source video](https://www.youtube.com/watch?v=VSdUlxgpKn4) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Photoelectric effect and quantum efficiency; CCD and CMOS architectures; Pixel wells, full-well capacity and blooming; ADC and quantization; Color-filter arrays and demosaicing; Shot, dark, fixed-pattern and read noise.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Photons and sensors](https://drive.google.com/open?id=0B6w2XfPHEcZYell0Tkt2YjkwYWc) (pages 1-51).


#### PHOTO-09 - Photography session 09

[Source video](https://www.youtube.com/watch?v=1ah8EEqfFsM) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Night photography techniques; Astrophotography and tracking; Light painting and stacking; Trick photography and composites; Image evidence, manipulation and intent.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Night photography & astrophotography](https://drive.google.com/open?id=0B6w2XfPHEcZYaTJ5M2RhZzBwWTg) (pages 1-59) · [Special effects and trick photography](https://drive.google.com/open?id=0B6w2XfPHEcZYdklBY3BfVTFSSnM) (pages 1-49).


#### PHOTO-10 - Photography session 10

[Source video](https://www.youtube.com/watch?v=ia9w4ZtdULs) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Signal-to-noise ratio and dynamic range; Exposure, gain and photon collection; Camera shake and image stabilization; Frame alignment and stacking; Active autofocus and depth sensing.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Noise and ISO](https://drive.google.com/open?id=0B6w2XfPHEcZYS2xETmR4SVlIczg) (pages 1-50) · [Image stabilization (IS)](https://drive.google.com/open?id=0B6w2XfPHEcZYcFpESFU4dEM3QkE) (pages 1-40).


#### PHOTO-11 - Photography session 11

[Source video](https://www.youtube.com/watch?v=AaJz5RMtO4c) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Photography as art and constructed image; Pictorialism and modernism; Portraiture, still life and sports photography; Sports camera constraints and computational assistance.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [History of photography (part 3)](https://drive.google.com/open?id=0B6w2XfPHEcZYU1hzN1BtalBqQ1U) (pages 1-59) · [Why is sports photography hard?](https://drive.google.com/open?id=0B6w2XfPHEcZYc1BpLTNjZGUzbk0) (pages 1-65).


#### PHOTO-12 - Photography session 12

[Source video](https://www.youtube.com/watch?v=gRT0mtabjI4) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Human color vision and receptor responses; Metamerism and spectral sensitivity; Color-matching experiments and primaries; Color gamut; Additive and subtractive mixing.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Color I: trichromatic theory](https://drive.google.com/open?id=0B6w2XfPHEcZYekpvMVN6MkdlZ0U) (pages 1-35).


#### PHOTO-13 - Photography session 13

[Source video](https://www.youtube.com/watch?v=6T_4mVZcozs) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Hue, saturation and lightness; Chromaticity diagrams and XYZ; Correlated color temperature and white balance; Opponent color spaces; Gamut mapping and color management.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Color II: applications in photography](https://drive.google.com/open?id=0B6w2XfPHEcZYWllrZHZzRHlZWlk) (pages 1-52).


#### PHOTO-14 - Photography session 14

[Source video](https://www.youtube.com/watch?v=p964fzQC11g) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Photography as historical record and social tool; Dry plates, roll film and Kodak system; Halftone printing; Documentary photography and reform; Farm Security Administration.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [History of photography (part 4)](https://drive.google.com/open?id=0B6w2XfPHEcZYTjRoLWhnSUFEYTQ) (pages 1-84).


#### PHOTO-15 - Photography session 15

[Source video](https://www.youtube.com/watch?v=kO68jM_1eho) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Radiometry and photometry; Intensity, luminance and illuminance; Solid angle and inverse-square falloff; Extended sources and distance invariance; Diffuse, specular and anisotropic reflection; BRDF, BSSRDF and Fresnel effects.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Light and reflection](https://drive.google.com/open?id=0B6w2XfPHEcZYNHdYdjhMakktS28) (pages 1-44).


#### PHOTO-16 - Photography session 16

[Source video](https://www.youtube.com/watch?v=gy-6WhxVIqc) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Light-source spatial and angular extent; Portrait lighting and key/fill ratios; Studio lighting and special subjects; Flash guide numbers and synchronization; Flash plus ambient and computational flash/no-flash.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Photographic lighting](https://drive.google.com/open?id=0B6w2XfPHEcZYN1dYTWZndkZDNVE) (pages 1-72).


#### PHOTO-17 - Photography session 17

[Source video](https://www.youtube.com/watch?v=Jb4ojXL3xcA) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Camera processing pipeline and tone curves; HDR capture and exposure fusion; Local tone mapping; Bilateral denoising and unsharp masking; JPEG color subsampling and DCT compression.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Light measurement and radiometry|Light measurement and radiometry]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [In-camera image processing](https://drive.google.com/open?id=0B6w2XfPHEcZYeTFKZWhndVBOYTA) (pages 1-115).


#### PHOTO-18 - Photography session 18

[Source video](https://www.youtube.com/watch?v=fE0GQ9Tvn4U) · **Evidence used:** YouTube English automatic captions, original English preferred.

**Topics used:** Panorama capture and parallax; Perspective homography stitching; Cylindrical and spherical projections; Modern photography history and image culture.

**Integrated here:** [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Exposure, sensors, and dynamic range|Exposure, sensors, and dynamic range]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]].

**Supporting materials:** [Panoramas](https://drive.google.com/open?id=0B6w2XfPHEcZYYVNCbmdFRUI1Zjg) (pages 1-40) · [History of photography (part 5)](https://drive.google.com/open?id=0B6w2XfPHEcZYMU1OLTI2QUZQa1E) (pages 1-62).


### Playlist totals - unique entries actually used

| Playlist | Entries integrated | Published English caption reviews | Local transcription | Sampled visual showcase |
|---|---:|---:|---:|---:|
| CMU 15-462/662 | 25 / 25 | 23, including welcome | 1 | 1 |
| MIT 6.837 Fall 2021 | 22 / 22 | 22 | 0 | 0 |
| MIT 6.801 Fall 2020 | 23 / 23 | 23 | 0 | 0 |
| Levoy digital photography, added | 18 / 18 | 18 | 0 | 0 |
| **Total** | **88 / 88** | **86** | **1** | **1** |

The first three playlists contribute all 70 requested entries. The added playlist contributes 18 entries. Counts represent source use and review status; they do not claim that every frame was watched or that course projects and research systems were fully reimplemented.

### Expanded coverage across eight course playlists

| Playlist | Entries |
|---|---:|
| CMU 15-462/662, Fall 2020 | 25 |
| MIT 6.837, Fall 2021 | 22 |
| MIT 6.801, Fall 2020 | 23 |
| Levoy digital photography (Stanford CS178 based), added | 18 |
| Computer Graphics and Visual Computing (2025/26 CSC 4505) | 26 |
| MIT 18.156 Projection Theory, Spring 2025 | 24 |
| Computer Graphics, Fall 2009 | 21 |
| Stanford CS231N Deep Learning for Computer Vision I 2025 | 18 |
| **Total** | **177 / 177** |

The original four course groups contribute 88 entries; P1, P2, P3 and Stanford CS231n add 89. The expanded total counts playlist entries once. The source notes below distinguish caption review from visual inspection and from mathematical-paper evidence.

### Computer Graphics and Visual Computing (2025/26 CSC 4505)

[Playlist](https://www.youtube.com/playlist?list=PLubb3AIPdPhYu-auUzK10YzvERCshyQ9M)

#### P1-01 - Module 1.1: Course Intro & Administrivia Part 1 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=dA0O3xbMhZw) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Course scope and learning expectations; Capstone problem selection and literature search; Instructor research context and applied examples.

#### P1-02 - Module 1.2: Course Intro & Administrivia Part 2 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=SAqv-gqy0tA) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Computer graphics as visual communication; Research and capstone topic discovery; Applications: games, medicine, CAD, visualization, GIS, VR/AR, simulation.

#### P1-03 - Module 2.1: Fundamentals of Visual Computing Part 1 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=hoZcPvCIFyk) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Graphics/vision/image-processing input-output distinction; Video frames and frame/key-frame selection; Raster versus vector representation and scaling.

#### P1-04 - Module 2.2: Fundamentals of Visual Computing Part 2 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=KyFhkeTwnDE) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Texture mapping and lighting motivation; Polygon mesh and triangle geometry; Vertex/edge/face and vertex attributes.

#### P1-05 - Module 2.3: Fundamentals of Visual Computing Part 3 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=VPxqKOYjpG0) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Triangle primitive motivation and GPU processing; NumPy array and legacy OpenGL setup recap; Coordinate normalization.

#### P1-06 - Module 3.1: The Graphics Pipeline Part 1 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=QEiWsz7cmnw) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Logical versus API/hardware pipeline; 3D-to-2D rendering challenge; Six coordinate spaces.

#### P1-07 - Module 3.2: The Graphics Pipeline Part 2 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=rRQiFwFzFJM) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Programmable versus fixed stages; Vertex specification and buffer objects; Model/view/projection transforms in vertex shader.

#### P1-08 - Module 4.1: Introduction to OpenGL Part 1 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=I1VFptM0obM) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Fragment shader inputs and color outputs; Depth, stencil and blending; API contract and abstraction.

#### P1-09 - Module 4.2: Introduction to OpenGL Part 2 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=CpN9JORFZ7s) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** OpenGL implementation and version checks; Linux mesa-utils, glxinfo and glxgears; Android Termux/VNC demonstration setup.

#### P1-10 - Module 4.3: Tutorial — OpenGL Implementation Q&A Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=Mt8QU6JWY20) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** PyOpenGL binding versus OpenGL implementation version; Backward-compatible use of legacy examples; Course setup checklist.

#### P1-11 - Module 4.4: Introduction to OpenGL Part 3 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=sr-4ZAnBXIM) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Why NumPy for contiguous vertex attributes; Import alias and array constructors; Array dtype, dimension, shape and size.

#### P1-12 - Module 4.5: Introduction to OpenGL Part 4 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=Eu4pGvCAOhI) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Tuple/list-to-array conversion and explicit dtypes; One- and multi-dimensional array shape; Indexing and half-open slicing.

#### P1-13 - Module 4.6: Introduction to OpenGL Part 5 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=e2GQBMBWyh0) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Dot product and matrix multiplication; reshape, transpose and flatten; NumPy 4x4 transform data.

#### P1-14 - Module 4.7: Introduction to OpenGL Part 6 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=EtgZoNuFZiw) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Pygame initialization and display/context flags; Event loop, keyboard exit, resize and viewport; Clock, frame pacing and double buffering.

#### P1-15 - Module 5.1: Vertex Specification Part 01 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=llQ74GoG1po) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Vertex as structured attribute record; Position XYZ/XYZW and homogeneous W; RGB/RGBA interpolation and alpha.

#### P1-16 - Module 5.2: Vertex Specification Part 02 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=wl-w5PZfmII) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Course-level pipeline framing; Model and world space illustrated by game objects; Transformations and camera view recap.

#### P1-17 - Module 5.3: Vertex Specification Part 03 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=QGwjwKvDF0I) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Legacy/core scope; Immediate-mode begin/end; Vertex-call dimensions/types/vector variants.

#### P1-18 - Module 5.4: Vertex Specification Part 04 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=yNa86goEb64) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** NDC mapping and viewport; Legacy point/line/triangle modes; Line and triangle grouping rules.

#### P1-19 - Module 5.5: Vertex Specification Part 05 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=IaV0iweZLxY) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Triangles as surface mesh primitives; Triangle fan and hub connectivity; Triangle strip vertex reuse.

#### P1-20 - Module 5.6: Vertex Specification Part 06 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=S8gMtV0Zsa4) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Triangle geometry and shared corners; Per-vertex XYZ/RGB array; float32 shape and indexing.

#### P1-21 - Module 5.7: Vertex Specification Part 07 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=ztaaLo_SVjQ) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Indexed drawing versus positions; Cube corners and 12 triangle triples; Integer index data and array shape.

#### P1-22 - Module 6.1: Geometric Transformations Part 01 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=ynFh7s0SBpg) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Transformation purpose: model space to world space; 2D translation and homogeneous coordinates; Uniform and nonuniform scale.

#### P1-23 - Module 6.2: Geometric Transformations Part 02 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=Utop5s8YRM4) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Transformation purpose: model space to world space; 2D translation and homogeneous coordinates; Uniform and nonuniform scale.

#### P1-24 - Module 6.3: Geometric Transformations Part 03 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=CeHVh4oSX6g) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Legacy versus core OpenGL context; Identity matrix reset and accumulation; GL_TRANSLATE_F, GL_ROTATE_F, GL_SCALE_F arguments.

#### P1-25 - Module 7.1: Geometric Transformations Part 01 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=oeEe2WiSku8) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** Viewing problem and virtual camera motivation; 2D world window versus screen viewport; Viewport-coordinate convention distinction.

#### P1-26 - Module 7.2: Geometric Transformations Part 02 Graphics & Visual Computing 2026

[Source video](https://www.youtube.com/watch?v=GnIE0V5dS1w) · Full transcript reviewed; YouTube English original automatic captions (en-orig); 30-second normalized transcript; exact cue-word deduplication.

**Topics used:** World versus view coordinates; Camera fixed at origin looking −Z; Inverse camera motion / translate world by −eye.

### MIT 18.156 Projection Theory, Spring 2025

[Playlist](https://www.youtube.com/playlist?list=PLUl4u3cNGP62K9_7Pg26ytrAOXwXFpN43)

#### P2-01 - Lecture 01: Introduction to Projection Theory

[Source video](https://www.youtube.com/watch?v=1A1kWkxK3QY) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Introduces orthogonal projections and the exceptional-direction question for finite X; the grid predicts |E_S(X)| on the order of S²/|X| over a useful range, with Szemerédi–Trotter proving the sharp upper bound and topology entering its proof. Extends the question to line-fiber integrals of a density (CAT scan); the Fourier slice identity says the Fourier transform of a projected density is the restriction of the original transform to the corresponding subspace. Explains why point projections and projections of thick balls differ: clustering changes the behavior. Introduces finite-field linear maps, the F_p² subfield obstruction inside F_{p²}², and the sum-product connection.

#### P2-02 - Lecture 02: Fundamental Methods of Projection Theory

[Source video](https://www.youtube.com/watch?v=sEZHGMV2LiI) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Defines π_θ(x₁,x₂)=x₁+θx₂ over F_q and S=max_θ∈D|π_θ(X)|. Contrasts an integer grid with a subfield example; the latter shows why a prime-field hypothesis is essential for the grid conjecture. Proves two estimates. Coincidences: each small image forces ≳|X|²/S colliding ordered distinct pairs; each pair determines at most one direction, hence |D|≲S when S<|X|/2. Orthogonality: choose at most S covering lines per direction, sum their indicators F, split each line indicator into mean 1/q and mean-zero part. The mean-zero parts of nonparallel lines are exactly orthogonal.

#### P2-03 - Lecture 03: Projection Theory in Euclidean Space

[Source video](https://www.youtube.com/watch?v=OAI8C20E7R4) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Transfers collision counting to disjoint unit balls X in B(0,R) and 1/R-separated directions D. Defines N_X(r) as maximum local ball occupancy and N_D(ρ) as maximum directions in an arc of length ρ. For a colliding pair of projected balls separated by distance ≈r, the direction lies in an arc of width ≈1/r; there are at most N_D(1/r) such directions, while the number of nearby second balls is ≤N_X(r). Summing dyadic r gives |D|≲(S/|X|)Σ_r N_X(r)N_D(1/r). Compares curve-neighborhood, well-spaced grid, and clumped-ball examples; spacing affects the bound. Hausdorff spacing N_X(R^β)≲|X|^β (and similarly for D) reduces the scale sum to endpoint behavior and yields the finite-field-style alternative (up to logarithmic/power losses). Then develops a real Fourier lemma: smooth 1×R tubes.

#### P2-04 - Lecture 04: The Fourier Method in Euclidean Space

[Source video](https://www.youtube.com/watch?v=X-4how4KkPQ) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Digests the Euclidean Fourier estimate and distinguishes its added information from the elementary total L² bound. Low-frequency support in B(0,1/r) makes a piece locally constant on radius-r balls: choose a smooth Fourier cutoff η=1 on that ball, write G=G*η̌, use rapid decay by integration by parts and a positive majorant ψ_r; Cauchy–Schwarz gives |G|²≲|G|²*ψ_r. Applying this on X and using N_X(r)/r² as local density, plus a tube-count bound of ≲rN_D(r/R) inside an r×R container, gives the clustered-direction estimate |D|≲(SR/|X|) max_r N_X(r)N_D(r/R)/r² up to logarithms. Under Hausdorff spacing it collapses to the finite-field-style alternative. Reviews independent historical rediscoveries of the Fourier/double-counting methods. Introduces sieve as reduction maps Z→Z/pZ; squares occupy ≈p/2 quadratic residues, and Linnik’s bound |X|≲√N follows by counting (x₁,x₂,p) with x₁≡x₂ mod p: each unequal pair has at most log N prime divisors, while diagonal pairs contribute |X||D|..

#### P2-05 - Lecture 05: The Large Sieve

[Source video](https://www.youtube.com/watch?v=i5lpHaGpeaI) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** States Linnik large-sieve theorem: for f:[N]→C, M≤√N, Σ_{p∼M}||(π_p f)_H||²₂ ≲ (N/M)||f_H||²₂. Mod-p reduction samples f̂ at α/p; distinct reduced fractions α/p from primes p∼M are separated by ≳M⁻². Since f is supported on [N], a smooth cutoff gives f̂ local constancy at scale 1/N; M²≤N ensures the sampling peaks have bounded overlap, converting the sum to an integral by Plancherel. Uses this for sparse residue supports (quadratic residues imply |A|≲√N) and average residue-class discrepancy ≲N^{1/4}. Explains Bombieri–Vinogradov motivation but does not yet prove it..

#### P2-06 - Lecture 06: Projections and Smoothing

[Source video](https://www.youtube.com/watch?v=iUB_y1OXBo4) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Explains a 3D finite-field line estimate as the PSet 1 repair: in F_q² the mean-zero line indicators of distinct nonparallel lines are orthogonal, but in F_q³ their Fourier supports are planes whose pairwise intersections are lines, so orthogonality fails. Group lines by direction W; a fixed nonzero frequency lies in W⊥ for at most q+1 directions. Cauchy–Schwarz yields ||F_H||²₂≤(q+1)Σ_W||F_{W,H}||²₂≤(q+1)q²|L|. Then studies Euclidean fiber-integral projections: Fourier-slice identity, polar Plancherel, and Sobolev embedding. Exact theorem: if f∈L²(R^d), supp f⊂B₁, and (d−1)/2>k+1/2, then ∫_{S^{d−1}}||π_θf||²_{C^k}dθ≲||f||²₂. The unit-ball support controls the low-frequency term via ||f||₁≲||f||₂; prove for Schwartz functions then extend by density. Cube projection along the diagonal connects with the central limit theorem and Gaussian smoothing..

#### P2-07 - Lecture 07: Applications of the Large Sieve to Number Theory

[Source video](https://www.youtube.com/watch?v=-UJgxLaFzfM) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Defines Δ_q(N)=max_{a∈(Z/qZ)*}|π(N; q,a)−π(N)/φ(q)|. Contrasts Dirichlet, Siegel–Walfisz, GRH and Montgomery bounds; states Rényi–Bombieri–Vinogradov average over q≤N^{1/2−ε}. Explains the sieve identity RPS=1*D_{p₁}*…*D_{p_r}, where RPS detects integers with no prime factor in S.

#### P2-08 - Lecture 08: The Szemeredi-Trotter Theorem

[Source video](https://www.youtube.com/watch?v=3h0jvsOnTIc) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Presents the full Szemerédi–Trotter theorem I(X,L)≲|X|+|L|+|X|^{2/3}|L|^{2/3}, and derives the projection corollary |D|≤S²/|X|+1. The N×N grid with rational slopes of height ≤M has |X|≈N², |D|≈M², projection size ≲MN, |L|≈M³N, and I≈N²M², saturating the middle term. Other known sharp families include rectangular grids, some number-field grids, and projective images; an inverse classification is open. Double counting gives only I≲|X||L|^{1/2}+|L| and fails over finite fields (all points/all lines in F_q² give q³ incidences), proving that topology is essential. The proof's cell-decomposition lemma partitions R² into open cells plus a wall, with each line meeting O(s) cells/wall pieces and each cell containing ≤|X|/s² points. Polynomial ham-sandwich (from Borsuk–Ulam) repeatedly bisects point sets; degrees grow geometrically and sum to O(s). Cellwise double counting yields I≲s^{-1/2}|X||L|^{1/2}+s|L|.

#### P2-09 - Lecture 09: Reflections on the Szemeredi-Trotter Theorem

[Source video](https://www.youtube.com/watch?v=lvdIUKULiak) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Analyzes what the ST cell proof does not capture: local per-cell incidence bounds discard cross-cell line relationships, the very structure needed to classify extremizers. Separates flexible linear-term examples from rigid grid-type middle-term examples; projective point-line duality swaps chains and stars but takes a grid to a different sharp configuration. For prescribed projection directions, normalizes three slopes to {0,1,∞}; with a fourth transcendental t, constructs P_{k,s}={Σ_{j<k}a_jt^j:0≤a_j<s}, X=P×P, |X|=s^{2k}, and max projection ≲(2s)^{k+1}.

#### P2-10 - Lecture 10: Sum-Product Theory

[Source video](https://www.youtube.com/watch?v=VCOrx2jEKXk) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Builds additive-combinatorics tools for BKT. Corrects the approximate-subfield lemma: for A⊂F_p, either (A−A)/(A−A)=F_p or a bounded-complexity rational expression built from A (including (A·A)^{⊕3}−(A·A)^{⊕3}, divided by A−A) has size at least |A|². If c is outside the ratio set, representations a₁+ca₂ are unique, so |A+cA|=|A|². If the ratio set is proper, repeated addition of 1 in the prime field provides b in it with b+1 outside; clear denominators and expand to force growth. Reviews Freiman–Ruzsa: bounded doubling implies containment in a bounded-dimensional generalized arithmetic progression, but available quantitative bounds are huge; polynomial Freiman–Ruzsa remains open. Proves Ruzsa triangle inequality by injection A×(B−C)→(A−B)×(A−C), yielding |A||B−C|≤|A−B||A−C| and |A+A|≤K|A| ⇒ |A−A|≤K²|A|. States Plünnecke: if |A+B|≤K|A|, then |B^{⊕m}−B^{⊕n}|≤K^{m+n}|A|.

#### P2-11 - Lecture 11: Contagious Structure in Projection Theory

[Source video](https://www.youtube.com/watch?v=Uz7aJtbfqkU) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Corrects the previous approximate-subfield expression: denominators cannot simply be multiplied through a cardinality estimate; take a literal bounded-complexity rational expression and clear denominators. Develops contagious structure for small projections/sumsets. Ruzsa transfers smallness through dilations (A−t₁A and A−t₂A small imply A−t₁t₂A small); Plünnecke transfers sums to differences.

#### P2-12 - Lecture 12: The Bourgain-Katz-Tao Projection Theorem

[Source video](https://www.youtube.com/watch?v=aHQxdnufLbg) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Completes arbitrary-set BKT with Balog–Szemerédi–Gowers (BSG). Projection form: if |A|,|B|≤N, X⊂A×B has |X|≥K⁻¹N² and |π_t(X)|≤KN, then find substantial A′⊂A,B′⊂B with X′=X∩(A′×B′) of size ≥K⁻O(1)N² and |A′+tB′|≤K^O(1)N. The restricted small image creates high additive energy by Cauchy–Schwarz; BSG removes unstructured garbage and keeps a large structured rectangle. P-R gives small A′−A′ too. Fix t₁ with A′+t₁B′ small. For any t, choose one pair (a₁,a₂) for each value of a₁−(t/t₁)a₂ in π_{−t/t₁}(A′×A′); map (c,(a,b)) to (a₁−a,a₂+t₁b,a+tb). It is injective into (A′−A′)×(A′+t₁B′)×π_t(X′): from output (u,v,w), recover c=u−(t/t₁)v+w, then the fixed pair (a₁,a₂), then a=a₁−u and b=(w−a)/t (handle t=0 by the trivial coordinate projection). Hence |π_t(X′)|≥(|X′|/N²)|π_{−t/t₁}(A′×A′)|. The transformed direction set retains positive power size, so product-set BKT forces growth.

#### P2-13 - Lecture 13: The Balog-Szemeredi-Gowers Theorem

[Source video](https://www.youtube.com/watch?v=DGBuM5pEdoo) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Develops BSG by representing X as a bipartite graph, counting length-two paths via Cauchy–Schwarz, selecting a neighborhood with few bad pairs, pruning vertices with too many bad partners, then choosing right vertices with enough neighbors among the good pairs. This yields large A′, B′ with many length-three paths between every cross-pair; each path gives a representation a+b=z1−z2+z3 from restricted sums, so |A′+B′|≤|π1(X)|³/P. OCW notes verify BSG theorem hypotheses and constants; captions only, no video-board inspection..

#### P2-14 - Lecture 14: The Bourgain Projection Theorem Part 1 over the Real Numbers

[Source video](https://www.youtube.com/watch?v=0UYCJ0cjG9k) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Recalls finite-field BKT and square-root injectivity baseline; real counterexamples are clustered directions and clustered point sets. States real Bourgain projection theorem under power-law nonconcentration of point set and direction set, in robust form for every sufficiently dense subset. Defines δ-covering numbers, mesh-box counting, and transfer of Ruzsa/Plünnecke/BSG with dimension constants. Explains why finite-field ratio-set growth fails over R (intervals, small denominators, lack of canonical +1 escape, additive subgroups), then introduces clipped ratio set with denominator cutoff and the real order/gap lemma using B/2∪(B+1)/2. Proof of the polynomial expansion is sketched, details omitted; OCW notes checked..

#### P2-15 - Lecture 15: The Bourgain Projection Theorem, Part 2

[Source video](https://www.youtube.com/watch?v=1lql9PGvLG4) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Defines (Δ,m)-uniform sets through level-dependent branching factors constant across all occupied parent cubes. Proves the useful implications: uniform plus lower covering bounds at each dyadic scale implies a δ-scale nonconcentration set; uniform plus δ-nonconcentration propagates the bound to all coarser scales. Proves uniformization for any subadditive set function by bottom-up dyadic pigeonholing branching counts and trimming each parent to a common number of children, losing at most [2d log(1/Δ)]^m. Shows how uniformity repairs the scale mismatch in iteration, but notes that Q(A) need not itself be uniform; source presents a sketch of the bridge. OCW notes checked..

#### P2-16 - Lecture 16: The Bourgain Projection Theorem, Part 3

[Source video](https://www.youtube.com/watch?v=85YISwhp7MQ) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** States robust polynomial expansion: for dense Z⊂A^k, a fixed polynomial image Q(Z) expands, not just Q(A^k). Explains how a dense image subset and BSG/energy extract a structured rectangle; symmetry transfers expansion to dense subsets, then a fiber-weighted subadditive function plus uniformization selects a uniform portion that remains large at each scale. This yields a higher-dimensional nonconcentration subset inside the expanded image and permits finite iteration. Finishes the real BPT proof architecture: normalize three well-separated directions, use repeated BSG reductions, ensure projected coordinate sets themselves are nonconcentrated, and apply robust product-set growth. Lectures/notes explicitly give proof sketches and omit parameter-heavy details; these limitations are recorded, not presented as fully proved in source. OCW notes checked..

#### P2-17 - Lecture 17: Random Walks on Finite Groups, Part 1

[Source video](https://www.youtube.com/watch?v=QVDLMx0Guiw) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Defines convolution random walks on finite groups, shows the singular-value mixing estimate and Cayley expansion inequality, then proves an order-p lower bound on dimensions of nontrivial representations of SL2(Fp). Uses this to bound the nontrivial singular value by Cp²‖μ‖₂² and explains why upper-triangular subgroups are the sharp obstruction scale..

#### P2-18 - Lecture 18: Random Walks on Finite Groups, Part 2

[Source video](https://www.youtube.com/watch?v=_jSFwF8rMdY) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Develops the Sarnak–Xue route to Selberg expansion: lift the walk to SL2(Z), estimate mass in the principal congruence subgroup by counting determinant-one matrices, then convert an L2 bound into a spectral gap. The matrix count uses p² divisibility and gives a p⁻³ density up to divisor losses; the hyperbolic heat-kernel viewpoint makes the random-walk counting step precise. Ends with the Borel-subgroup obstruction and its approximate-subgroup interpretation. The lecture marks the random-walk distribution claim as vague/proof-sketch..

#### P2-19 - Lecture 19: Random Walks on Finite Groups, Part 3

[Source video](https://www.youtube.com/watch?v=bBzcaYNAfcI) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Unpacks Bourgain–Gamburd as two inputs: failure of mixing produces an approximate subgroup by convolution flattening, dyadic level sets, multiplicative energy, and noncommutative Balog–Szemerédi–Gowers; then a Helfgott/Larsen–Pink style classification forces moderately large approximate subgroups in SL2(Fp) to concentrate in a Borel coset. Prime-field sum–product excludes an approximate subfield, yielding spectral gap when the step set avoids Borel-coset concentration. The lecture provides a proof sketch, not complete proofs of the deep inverse/classification inputs..

#### P2-20 - Lecture 20: Homogeneous Dynamics, Part 1

[Source video](https://www.youtube.com/watch?v=rChKr66KG-c) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Introduces homogeneous spaces G/Γ and identifies SL2(R)/SL2(Z) with covolume-one lattices. Contrasts periodic-or-dense unipotent orbits (Hedlund) with more complicated diagonal orbits; computes how diagonal conjugation expands, contracts, and preserves the neutral tangent direction under a right-invariant metric. A long shear is renormalized to a unit shear, and twisting of the compressing direction connects orbit spreading to projection theorems. The cusp invalidates a naive compact-tube proof and corresponds to lattices near periodicity; notes also connect this orbit picture to Oppenheim..

#### P2-21 - Lecture 21: Homogeneous Dynamics, Part 2

[Source video](https://www.youtube.com/watch?v=5QV8Gr1z16E) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Studies twisting of the contracting direction in homogeneous dynamics. A cross-section recurrence models each renormalized orbit segment by near-projections and yields N_{j+1} ≳ δ^{-1}N_j^{1/2}; iterating approaches planar δ^{-2} covering. Sharpness is witnessed by disks. Extends the idea to SL3, where a one-parameter curve of directions can fail if flat (equatorial slab example); curvature and Fourier decoupling address the restricted-direction obstruction. Quantitative orbit-density statements are cited as context, not proved..

#### P2-22 - Lecture 22: Sharp Projection Theorems, Part 1: Introduction and Beck's Theorem.

[Source video](https://www.youtube.com/watch?v=hBG7nMXTOcw) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Sets up the Furstenberg rich-tube problem with E of size δ^{-t}, about δ^{-s} directions through each point, and typical richness r. Incidence counting relates |T| to |E||T_x|/r. The competing star, grid, and random constructions divide parameters into A (s≥t, double counting), C (s+t≥2, Fourier), and B (s<t<2−s, hard). Develops continuum Beck: rectangle nonconcentration forces some x to see at least min(δ^{-u},δ^{-1}) directions. Pigeonholing, direction-set nonconcentration, double counting, an Orponen–Shmerkin epsilon improvement, and scale iteration form the proof architecture; transcript/notes do not supply every technical lemma..

#### P2-23 - Lecture 23: Sharp Projection Theorems, Part 2: AD Regular Case

[Source video](https://www.youtube.com/watch?v=I6ZSrgxiofs) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Defines AD regularity through uniform branching at every scale and proves the key submultiplicative principle conceptually: thicken to a coarse scale, rescale each occupied coarse ball, and multiply coarse and fine richness. AD regularity is what makes every selected local pattern admissible. A projective reduction makes directions common across base points. Near equality forces product-like structure inside a tube; asymmetric BSG and Plünnecke–Ruzsa lead to the real ABC sum-product theorem and an epsilon improvement. The lecture explicitly leaves the full structured-set and spacing-case derivation to research papers..

#### P2-24 - Lecture 24: Sharp Projection Theorems, Part 3: Combining Different Scales

[Source video](https://www.youtube.com/watch?v=h0Ln82BR4R0) · Full transcript reviewed; YouTube original English captions (en-orig); timestamped transcript.

**Topics used:** Completes the course proof outline. For well-spaced E of N δ-balls, the GSW rich-tube bound is |T_R(E)| ≲_ε δ^{-ε}N²/R³ once R exceeds the random-tube baseline δN by a power; Fourier analysis handles the baseline, thickening to ρN≈R handles larger R, and a two-ends double count removes fat-tube multiplicity. Then defines the branching function f(x)=log_{1/δ}|E_{δ^x}|_{δ^x}, increasing, t-Lipschitz-below and 2-Lipschitz-above. AD regularity is linear slope; semi-well-spaced graphs are above a broken profile of slopes 2−s and s. Scale cutting concatenates coarse and fine problems. Rademacher differentiation plus the semi-well-spaced alternative partitions the hard range into controllable pieces. The final decomposition and some deep theorem proofs remain outlined, not derived in full..

### Computer Graphics, Fall 2009

[Playlist](https://www.youtube.com/playlist?list=PL_w_qWAQZtAZhtzPI5pkAtcUVgmzdAP8g)

#### P3-01 - Introduction to Computer Graphics

[Source video](https://www.youtube.com/watch?v=01YSK5gIEYQ) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** UC Davis ECS 175 / instructor Ken Joy; Course purpose: 3D graphics, math, optics, programming; API-level development and visual iteration.

#### P3-02 - Curves in the Plane

[Source video](https://www.youtube.com/watch?v=0NbD-c0Ctdk) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Why interpolation can oscillate; Control polygons and design curves; Chaikin quarter/three-quarter corner cutting.

#### P3-03 - Moving Objects in Space

[Source video](https://www.youtube.com/watch?v=wArGifkRD2A) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** 2D/3D local frames and basis vectors; Coordinates relative to origin and basis; Homogeneous row-vector convention.

#### P3-04 - Rotations About an Arbitrary Axis

[Source video](https://www.youtube.com/watch?v=gRVxv8kWl0Q) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** MVC and software design patterns; Qt signals/slots and generated moc code; Model owns curve data/draw routines.

#### P3-05 - The Camera Transform

[Source video](https://www.youtube.com/watch?v=mpTl003EXCY) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Separates camera pose/orientation from frustum-to-canonical-cube projection. Pinhole model; perspective comes from homogeneous W proportional to camera depth and divide by W.; Camera inputs: eye, viewing direction/center of attention, advisory up vector, field angle, near and far. Move eye to origin and orient axes so camera looks down negative Z; then map the truncated viewing pyramid to image-space cube.; Derives row-vector projection with W=-Z; uses near/far boundary constraints to map near to +1 and far to -1 in the course convention. Includes vertical top edge n tan(alpha/2), field angle, projection, depth clipping, screen mapping..

#### P3-06 - Clipping

[Source video](https://www.youtube.com/watch?v=og7hOFypKpQ) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Reviews camera pose and center-of-attention versus direction vector, then motivates camera basis from viewing direction and up; up is advisory and the computed basis must be orthogonal.; Explains transform order, camera matrix C followed by viewing/projection matrix V under row vectors, divide by W, image space and negative-W points behind camera. Matrix order mismatch can produce blank output.; Defines plane by point P and normal n; signed test d=n·(Q−P) classifies inside/outside/on-plane once normal orientation is chosen. Derives plane equation form ax+by+cz−d=0..

#### P3-07 - Depth Buffers and Ray Tracing

[Source video](https://www.youtube.com/watch?v=Xks1v4GNUiY) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Revisits signed-distance clipping and why canonical/image-space plane normals allow cheap component/sign tests; clipping may be delayed in homogeneous coordinates, with caveat that behind-camera points must be rejected.; Depth-buffer mechanics: one depth value per pixel alongside color; rasterize each polygon, compare its interpolated depth to stored depth, write color and depth only when it wins. Source uses larger image-space z as nearer and initializes to a very negative value; mapping and comparison must be changed consistently in other conventions.; Shows why depth buffering handles interpenetrating surfaces more robustly than a global sort; highly parallel per-pixel test. Transparency requires additional ordering/blending treatment; shadows are a second visibility question relative to lights..

#### P3-08 - Color and Shading

[Source video](https://www.youtube.com/watch?v=TEjDYtkLRdQ) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Uses object-oriented scene construction as a model: objects/classes have draw and transform behavior; classes compose into rooms/scenes. Reiterates the course assignment distinction between a manually constructed camera/view transform and GLU viewing calls in image space.; Motivates shading as light transport; physical light bounces recursively. RGB samples continuous spectra into three channels; 8-bit RGB gives 24 bits/channel combination space. RGB is additive, CMY is subtractive; HLS/HSV-like models aid human color selection. The speaker conflates HLS value and saturation; draft corrects that.; Point-light simplification; constant ambient term approximates indirect room illumination. Diffuse/Lambert term scales by max(0,N·L), with unit normal and light direction; grazing light approaches zero and aligned light is maximal. Material base/diffuse color controls channel response..

#### P3-09 - Quaternions

[Source video](https://www.youtube.com/watch?v=mHVwd8gYLnI) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Motivates noncommuting rotations with an order-sensitive 90° x/y example and connection to Euler-angle gimbal lock.; Builds quaternion algebra as scalar a plus vector v. Addition is componentwise; product scalar is a1a2−v1·v2 and vector is a1v2+a2v1+v1×v2. Conjugate (a,−v), norm sqrt(a²+|v|²), inverse conjugate divided by norm squared; noncommutativity comes from cross product.; Unit quaternion axis/angle rotation, closure under product, composition into one orientation, conversion to 4×4 matrix for rendering. Transcript uses ambiguous/full-angle language; mathematically correct rotation encoding uses half-angle q=(cos(theta/2),u sin(theta/2)) and q p q* for pure-vector p..

#### P3-10 - Shading and Texturing

[Source video](https://www.youtube.com/watch?v=Is6D5rnWEvs) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Recasts the ambient/diffuse/specular shading equation and connects each coefficient to a material/light response.; Explains flat shading (one face normal/result), Gouraud shading (compute colors at vertices, interpolate colors), and Phong shading (interpolate normals, evaluate lighting per fragment). Small specular peaks can be missed by Gouraud; Phong captures them more reliably at greater cost.; Introduces micropolygons about half a pixel as a strategy for detailed shading/textures and surface variation..

#### P3-11 - The Hierarchical Modeling System

[Source video](https://www.youtube.com/watch?v=D8ZYHXom0qk) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Course assignment discussion emphasizes repeated furniture and components, camera controls, near clipping, and reuse of one model with transforms. These are supporting context, not required tasks.; Demonstrates an editable command-file modeling system (called Midas by the speaker): define polygons, transform in place, create transformed copies, extrude a polygon into side faces, assemble components. Shows unit square faces translated/rotated into a cube, then repeated/assembled into a 3×3×3 arrangement.; Explains the component structure as a directed graph/tree: leaf nodes hold low-level polygons, higher nodes apply transforms and assemble children. Shared repeated geometry is instanced by transform rather than duplicated by hand..

#### P3-12 - Scan Conversion

[Source video](https://www.youtube.com/watch?v=23HEwdcphg4) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Defines scan conversion/rasterization as identifying pixel samples covered by a polygon in device space after image-space mapping. Historic scan-line/CRT explanation motivates processing horizontal rows.; Maps NDC square to an Nx×Ny device grid while depth and attributes accompany geometry. Uses a chosen pixel corner as sample location in the lecture; real raster fill rules need consistent shared-edge handling.; Splits polygons at vertex y-levels into trapezoid-like bands so each band can be scanned with two active edge trackers; triangles are a special case. Horizontal/degenerate edges require careful handling..

#### P3-13 - Curves and Surfaces

[Source video](https://www.youtube.com/watch?v=FTg1DUr7bhY) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Contrasts corner-cutting/subdivision curves with endpoint-interpolating cubic Bezier curves; de Casteljau subdivision gives a stable split and points converge to curve.; Bezier convex-hull and variation-diminishing properties; tangent endpoint formulas and smooth joins: G1 collinearity, C1 equal tangent vectors; higher derivative continuity imposes more constraints.; Introduces B-spline/NURBS curve construction: local degree/order, knot sequence, uniform vs nonuniform, endpoint multiplicity/clamping. NURBS are piecewise polynomial/rational curves; transcript calls them “Beast blind” via ASR..

#### P3-14 - Visible Surface Algorithms

[Source video](https://www.youtube.com/watch?v=hmlWFi1TdbI) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Surveys five visible-surface algorithm families: depth buffer, scanline, ray tracing, painter/BSP ordering, and Warnock recursive image subdivision. Lecture says depth buffer/ray tracing are dominant general methods; painter returned in games; Warnock’s spatial subdivision is useful in parallel systems; scanline has niche/hybrid use.; Develops painter’s algorithm: sort and draw back-to-front so later polygons overwrite earlier pixels. Cyclic/intersecting geometry prevents a global order; split/clip polygons to resolve. BSP stores splitting polygon planes, splits crossing polygons, and traverses near/far subtrees in camera-dependent order.; BSP drawbacks: skewed trees can degrade toward quadratic construction/search; grouped/ordered scene geometry yields imbalance; splitting creates many tiny fragments. Bounding boxes and randomized splitters are discussed as practical help. Stored partition can support changing viewpoints through reversed traversal order..

#### P3-15 - Ray Tracing

[Source video](https://www.youtube.com/watch?v=Ahp6LDQnK4Y) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Ray model: one primary ray per pixel through a camera plane, intersect scene primitives, retain nearest positive hit and shade it. Naive cost O(pixels×objects); numeric 1M×1K ≈ 1B tests. Ray tracing avoids view/image-space projection pipeline for primary visibility.; Sphere intersection by projecting center-to-ray-origin vector along unit ray and checking perpendicular distance against radius; two roots on hit, tangent special case. Plane intersection derives t from dot products; denominator N·D=0 is parallel/coplanar edge case. Polygon membership via barycentric coordinates, half-space clipping, or odd/even crossing.; Explains Turner Whitted’s recursive illumination idea: shadow rays toward lights, reflection rays, and refracted rays; closest hit decides visibility. Suppress direct diffuse/specular when shadowed while retaining the course ambient approximation; add weighted reflected/refracted radiance recursively..

#### P3-16 - Subdivision Surfaces Part 1

[Source video](https://www.youtube.com/watch?v=9uscFr2Hht0) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Explains why corner-cutting curves have limits (parabolic pieces, no exact circle) and extends them to regular grids by applying 1/4–3/4 along u then v, creating nested quad meshes and biquadratic-by-biquadratic limit patches.; Derives bilinear interpolation over a four-corner cell: P(u,v)=(1−u)(1−v)P00+u(1−v)P10+(1−u)vP01+uvP11. Values at quarter parameters recover child points; coefficients sum to one, preserving affine combinations.; Generalizes the intuitive construction to irregular polygon meshes with face averages/radial edge points, producing smaller face polygons, edge quads, and vertex polygons. Extraordinary vertices retain original valence; ordinary regions become quads and locally look like regular subdivision patches. Identifies Doo–Sabin; exact canonical masks are more specific than this transcript’s sketch..

#### P3-17 - Subdivision Surfaces Part 2

[Source video](https://www.youtube.com/watch?v=THiF7-QxKXk) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Finishes Catmull–Clark subdivision on arbitrary polygon meshes. Computes face points as face-vertex averages, interior edge points as averages of the two endpoint vertices and the adjacent face points, and updated vertex points from local face/edge neighborhoods plus a valence-dependent original-vertex weight. Each pass can be scheduled in three passes (faces, edges, vertices).; Clarifies why extraordinary vertex valence changes local limit behavior; changing the weight can create spikes or alter smoothness, so the mask is mathematical, not an arbitrary artistic constant. Transcript gestures at C=3/n, but does not fully state standard generalized mask.; Builds a sphere mesh by subdividing icosahedron triangles four ways and projecting new midpoints radially onto a sphere. Triangle count quadruples exactly per refinement (20→80→320→1280); transcript’s “1,200” is approximate/ASR or arithmetic error..

#### P3-18 - Shadows Part 1

[Source video](https://www.youtube.com/watch?v=kmdRmXuOjIY) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Contrasts ground-plane projected shadows, shadow volumes and depth-buffer shadows; lecture covers the first two and previews depth-map comparison. Ground-plane projection is a simple game-friendly trick but only projects onto a planar receiver.; Derives line-light-to-surface projection onto a plane using a plane point/normal and a ray from light through each object point. Solve a ratio of dot products; parallel ray/plane denominator zero means no unique hit. Offset projected polygon slightly to avoid depth-buffer z-fighting.; Shadow becomes a separately rendered dark, ambient-only polygon. Complexity can approach one extra projected polygon per source polygon; texture/fuzzy edge or projected ellipse can cheaply suggest a soft contact shadow but loses actual shape detail..

#### P3-19 - Shadows Part 2

[Source video](https://www.youtube.com/watch?v=qiJhB0fOY6I) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Develops depth-buffer shadow mapping as visibility from the light: render a depth map per light, then render from the eye and determine whether each visible point is in front of or behind the closest depth stored from the light. Multiple lights generally require separate maps.; The 2009 lecture derives reprojection between eye and light image spaces with camera/view 4x4 transforms, inverse transforms and homogeneous coordinates. Retaining clip-space W matters: divide by W and it is lost, so a point must be reconstructed/transformed before perspective division. This is the lecture’s matrix-heavy route to the same light-space depth comparison used by practical shadow maps.; At a mapped sample, compare current light-space depth with stored light depth. If sample depth is farther, another surface blocks the light. A projected point rarely hits a texel center; treating one sample as decisive makes jagged/aliased shadow boundaries..

#### P3-20 - Camera Motion

[Source video](https://www.youtube.com/watch?v=HAJv25Afsz8) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Introduces Catmull–Rom interpolating splines for camera paths: specified waypoints lie on the path, unlike a Bezier control polygon. Continuity levels C0 (positions), C1 (first derivative/tangent), C2 (second derivative/curvature behavior) matter perceptually for camera motion.; Rejects global Lagrange interpolation for local path editing: a small waypoint perturbation can cause large oscillation elsewhere. A cubic Hermite/Ferguson segment is determined by endpoints and endpoint tangents; its four constraints solve four cubic coefficients or a 4×4 matrix. Hermite and Bezier are convertible cubic bases.; Catmull–Rom sets each interior waypoint tangent from neighboring points, typically half the difference between previous and next waypoint (uniform parameterization). That makes adjacent segment tangents agree, giving C1 continuity. Endpoints need phantom points/one-sided tangents or an assumed second derivative; different endpoint choices change path behavior..

#### P3-21 - Branches in Computer Graphics

[Source video](https://www.youtube.com/watch?v=NEIJW3ZJ1GE) · Full transcript reviewed; YouTube original English captions (en-orig.vtt); normalized 30-second transcript.

**Topics used:** Ends with a course-level map of the larger graphics field, separating interactive/pipeline graphics from scientific visualization, VR, graphics hardware, animation, geometric design, and image-based reconstruction.; Describes SIGGRAPH’s role as a large professional conference and a source of specialist communities/conferences; listed branches include interactive 3D/games (I3D), scientific visualization, VR, animation, CAD/CAGD/curves and surfaces, solid modeling, digital photography, parallel visualization, and graphics hardware. Attendance, paper acceptance and conference history numbers are lecture recollections, not independently verified/current statistics.; Connects mobile devices’ integrated GPUs to interactive rendering and augmented reality: camera/image input can be linked with contextual data or virtual content. Notes device memory constraints. Examples and timelines are 2009 forecasts, not current product claims..

### Stanford CS231N Deep Learning for Computer Vision I 2025

[Playlist](https://www.youtube.com/playlist?list=PLoROMvodv4rOmsNzYBMe0gJY2XS8AQg16)

#### SC-01 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 1: Introduction

[Source video](https://www.youtube.com/watch?v=2fq9wYslV0A) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** Learning to see: from pixels to trainable vision models; 1. What a computer vision system has to learn.

#### SC-02 - Stanford CS231N Spring 2025 Lecture 2: Image Classification with Linear Classifiers

[Source video](https://www.youtube.com/watch?v=pdqofxJeBN8) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 2. Classification begins with a representation; 3. Nearest neighbors: remember first, compare at prediction time; x_train: (N, D); y_train: (N,); x: (D,).

#### SC-03 - Stanford CS231N Spring 2025 Lecture 3: Regularization and Optimization

[Source video](https://www.youtube.com/watch?v=dyNGd06MWn4) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** Stanford CS231N | Spring 2025 | Lecture 3: Regularization and Optimization.

#### SC-04 - Stanford CS231N Spring 2025 Lecture 4: Neural Networks and Backpropagation

[Source video](https://www.youtube.com/watch?v=25zD5qJHYsk) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 2. Classification begins with a representation; 3. Nearest neighbors: remember first, compare at prediction time; x_train: (N, D); y_train: (N,); x: (D,).

#### SC-05 - Stanford CS231N Spring 2025 Lecture 5: Image Classification with CNNs

[Source video](https://www.youtube.com/watch?v=f3g1zGdxptI) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 10. Convolutions keep image neighborhoods intact; 11. Pooling downsamples feature maps; 12. Translation equivariance is a useful design property.

#### SC-06 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 6: CNN Architectures

[Source video](https://www.youtube.com/watch?v=aVJy4O5TOk8) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 13. CNN layers need stable signals and enough variation; 14. Activations make the network nonlinear; 15. Architectural patterns: VGG and residual connections.

#### SC-07 - Stanford CS231N Spring 2025 Lecture 7: Recurrent Neural Networks

[Source video](https://www.youtube.com/watch?v=kG2lAPBF7zA) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 19. Recurrent networks carry a state through a sequence; 20. Backpropagation through time shares gradients across steps; 21. Sequence models inherit the biases in their data.

#### SC-08 - Stanford CS231N Spring 2025 Lecture 8: Attention and Transformers

[Source video](https://www.youtube.com/watch?v=RQowiOF_FvQ) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 22. Attention lets each output retrieve relevant input; 23. Scaled dot-product attention and its dimensions; 24. Queries, keys, values, and two attention patterns.

#### SC-09 - Stanford CS231N Spring 2025 Lecture 9: Object Detection, Image Segmentation, Visualizing

[Source video](https://www.youtube.com/watch?v=PTypu6GqEd4) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 27. Dense prediction: give an answer at every pixel; 28. Encoder-decoder shape recovery and U-Net skips; 29. Detection: locate each object as well as name it.

#### SC-10 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 10: Video Understanding

[Source video](https://www.youtube.com/watch?v=wElqklprhPE) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 32. Video adds time, cost, and motion; 33. Three ways to fuse video frames; 34. Motion streams, recurrent models, and attention.

#### SC-11 - Stanford CS231N Spring 2025 Lecture 11: Large Scale Distributed Training

[Source video](https://www.youtube.com/watch?v=9MvD-XsowsE) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 36. Large-model training is a communication problem too; 37. Data parallelism averages gradients across replicas; 38. Sharding model state trades memory for communication.

#### SC-12 - Stanford CS231N Spring 2025 Lecture 12: Self-Supervised Learning

[Source video](https://www.youtube.com/watch?v=4howBU7THbM) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 42. Self-supervision creates training targets from the data; 43. Transformation and reconstruction pretext tasks; 44. Masked autoencoders predict hidden patches.

#### SC-13 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 13: Generative Models 1

[Source video](https://www.youtube.com/watch?v=zbHXQRUNlH0) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 47. Generative models represent possible outputs; 48. Likelihood and autoregressive factorization; 49. Autoencoders and latent variables.

#### SC-14 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 14: Generative Models 2

[Source video](https://www.youtube.com/watch?v=Edr4uZFh4EE) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 50. GANs learn by competing; 51. Rectified flow turns sampling into an integration problem; 52. Conditioning and classifier-free guidance.

#### SC-15 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 15: 3D Vision

[Source video](https://www.youtube.com/watch?v=7lxrKDKtykM) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 55. 3D vision begins with representation choices; 56. 3D data and tasks set the learning problem; 57. Learning on grids, sets, and surfaces.

#### SC-16 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 16: Vision and Language

[Source video](https://www.youtube.com/watch?v=mQOK0Mfyrkk) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 61. Foundation models and adaptation; 62. CLIP: align images with their descriptions; 63. Add captioning and spatial features to vision-language models.

#### SC-17 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 17: Robot Learning

[Source video](https://www.youtube.com/watch?v=XSfmOH_xVSU) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 68. Robot learning is a feedback problem; 69. Perception must support action; 70. Reinforcement learning: learn from outcomes.

#### SC-18 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 18: Human-Centered AI

[Source video](https://www.youtube.com/watch?v=g8UaBfj6Sh8) · Full transcript reviewed; YouTube original English captions (en-orig); normalized 30-second buckets.

**Topics used:** 74. Human vision shaped computer vision's early goals; 75. AI can see more detail—and humans have limits; 76. AI that helps people see and act.

### Research papers and current-topic orientation

#### R-02 - NeRF: Representing Scenes as Neural Radiance Fields for View Synthesis

[Primary paper PDF](https://arxiv.org/pdf/2003.08934)

PDF pp. 1–25 (complete paper and appendices A–D; references excluded) fully reviewed: complete substantive body and technical appendices where present Equation review: Volume-rendering integral, discrete alpha/transmittance weights, positional encoding, coarse/fine sampling, and NDC derivation checked in extracted primary PDF text; no raster page check. Related video boundary: Promotional demonstration of later Instant NeRF, not original NeRF experiments or their technical derivation. Caption status: Full en-orig automatic transcript read; it contains only brief nontechnical speech (“so,” “you”) plus music/applause. This is a 43-second promotional demo, not a technical lecture.

#### R-03 - Instant Neural Graphics Primitives with a Multiresolution Hash Encoding

[Primary paper PDF](https://nvlabs.github.io/instant-ngp/assets/mueller2022instant.pdf)

PDF pp. 1–15 (complete paper and appendices A–E; references excluded) fully reviewed: complete substantive body and technical appendices where present Equation review: Multiresolution schedule, hash lookup, interpolation, smoothstep, SDF/ray-marching formulas checked in extracted primary PDF text; no raster page check. Caption status: Full en-orig automatic transcript read; it contains only brief nontechnical speech (“so,” “you”) plus music/applause. This is a 43-second promotional demo, not a technical lecture.

#### R-04 - 3D Gaussian Splatting for Real-Time Radiance Field Rendering

[Primary paper PDF](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/3d_gaussian_splatting_low.pdf)

PDF pp. 1–14 (complete paper and appendices A–D; references excluded) fully reviewed: complete substantive body and technical appendices where present Equation review: Gaussian density/covariance, projected covariance, alpha compositing and optimization loss checked in extracted primary PDF text; no raster page check. Caption status: Full en-orig automatic transcript read; sparse overview of initialization, anisotropic splats, optimization, test claims, and ablation. Detailed method and results are taken from the paper, not inferred from this brief presentation.

#### R-05 - 2D Gaussian Splatting for Geometrically Accurate Radiance Fields

[Primary paper PDF](https://arxiv.org/pdf/2403.17888)

PDF pp. 1–13 (complete body and supplemental appendices A–D; references excluded) fully reviewed: complete substantive body and supplemental appendices A–D Equation review: 2D Gaussian plane parameterization, projective ray-splat intersection, depth-distortion and normal-consistency losses, median/expected depth, and single-pass distortion recurrence checked in extracted primary PDF text; no raster page check. Caption status: Full en-orig automatic transcript read; presentation covers planar splats, ray intersections, filtering, geometry regularizers, and mesh extraction. Captions are imperfect; formal details are sourced to the paper.

#### R-06 - Spatiotemporal Reservoir Resampling for Real-Time Ray Tracing with Dynamic Direct Lighting

[Primary paper PDF](https://benedikt-bitterli.me/restir/bitterli20restir.pdf)

PDF pp. 1–17 (complete paper and appendices A–B; references excluded) fully reviewed: complete substantive body, limitations, and appendices A–B Equation review: Direct-light integral, MIS/RIS estimators, weighted reservoir update, reservoir reuse weights, mixed-PDF expected-weight derivation, unbiased balance-heuristic correction, and WIS/RIS appendix relation checked in extracted primary PDF text; no raster page check. Caption status: no directly related source video identified in the supplied inventory

#### R-08 - Incremental Potential Contact: Intersection- and Inversion-free, Large-Deformation Dynamics

[Primary paper PDF](https://ipc-sim.github.io/file/IPC-paper-350ppi.pdf)

PDF pp. 1–20 (complete paper body and conclusion; references excluded) fully reviewed: complete substantive body, contact/friction/distance methods, evaluations, comparisons and conclusion Equation review: Admissible trajectory and epsilon-complementarity definitions, clamped log barrier, Newton barrier Hessian, implicit momentum balance/contact force, smoothed lagged friction potential, point-triangle/edge-edge distances and parallel-edge mollifier checked in extracted primary PDF text; no raster page check. Caption status: no caption track; 160-second supplemental video awaiting parent frame inspection

#### R-09 - As-Rigid-As-Possible Surface Modeling

[Primary paper PDF](https://igl.ethz.ch/projects/ARAP/arap_web.pdf)

PDF pp. 1–8 (complete paper body and conclusion; references excluded) fully reviewed: complete paper body and conclusion; references excluded Equation review: Per-cell weighted rigidity energy, covariance/SVD optimal rotation, cotangent weights, and alternating sparse Laplacian position solve checked in extracted primary PDF text; no raster page check. Caption status: no caption track; 210-second ARAP demo awaiting parent frame inspection

#### R-10 - Instant Field-Aligned Meshes

[Primary paper PDF](https://igl.ethz.ch/projects/instant-meshes/instant-meshes-SA-2015-jakob-et-al-compressed.pdf)

PDF pp. 1–15 (complete body and conclusion; references excluded) fully reviewed: complete substantive paper and conclusion; references excluded Equation review: Local Gauss–Seidel smoothing, symmetry-aware N-RoSy orientation, integer translation position field, local symmetry search, and weighted extraction equations checked in extracted primary PDF text; no raster page check. Caption status: Full en-orig automatic transcript read; overview of orientation/position fields, feature alignment, interactive tools, point clouds, and a large remeshing demo claim. Detailed evidence is sourced to the paper.

#### R-01 - Rendering Synthetic Objects into Real Scenes: Bridging Traditional and Image-based Graphics with Global Illumination and High Dynamic Range Photography

[Primary paper PDF](https://www.pauldebevec.com/Research/IBL/debevec-siggraph98.pdf)

PDF pp. 1–10 (complete paper; no technical appendix) fully reviewed: complete substantive body and technical appendices where present Equation review: Differential compositing and Lambertian reflectance-update equations in the extracted primary PDF text; glyphs readable; no raster page check. Caption status: no related caption track

#### R-07 - Mitsuba 2: A Retargetable Forward and Inverse Renderer

[Primary paper PDF](https://rgl.s3.eu-central-1.amazonaws.com/media/papers/NimierDavidVicini2019Mitsuba2_7.pdf)

PDF pp. 1–16 (complete body, system design, applications and limitations; references excluded) fully reviewed: complete substantive body and conclusion; references excluded Equation review: Differentiable change-of-variables equations, gradient-index Eikonal ODE, radiative transfer equation and its transmittance definition checked in extracted primary PDF text; no raster page check. Caption status: no directly related source video identified in the supplied inventory

#### R-11 - 3DGUT: Enabling Distorted Cameras and Secondary Rays in Gaussian Splatting

[Primary paper PDF](https://research.nvidia.com/labs/toronto-ai/3DGUT/res/3DGUT_full_resolution.pdf)

PDF pp. 1–13 (main paper and supplemental appendices A–F; references pp. 14–15 excluded) fully reviewed: complete paper body and supplemental appendices A–F; references excluded Equation review: 3D Gaussian/EWA projection, unscented sigma-point and moment weights, maximum-response ray parameter, generalized Gaussian kernel, projection KL tests and Waymo objective checked in extracted primary PDF text; no raster page check. Caption status: no directly related source video identified in the supplied inventory; unavailable NeRF talk is not counted

#### R-12 - ReSTIR-LOD: Level-of-Detail Reservoir Resampling for Efficient Global Illumination

[Primary paper PDF](https://research.nvidia.com/labs/rtr/publication/wang2026levelOfDetail/wang2026levelOfDetail.pdf)

PDF pp. 1–8 (complete paper body and discussion/conclusion; references excluded) fully reviewed: complete substantive paper and discussion/conclusion; references excluded Equation review: RIS/GRIS resampling weights and unbiased contribution weights, partial-bijection UV vertex map, world-space greedy matching, and surface-area Jacobian ratio checked in extracted primary PDF text; no raster page check. Caption status: no directly related source video identified in the supplied inventory

#### R-13 - What is next in computer graphics - SIGGRAPH 2026 research orientation

[Orientation video](https://www.youtube.com/watch?v=sIcGH3M5yPc)

This SIGGRAPH 2026 trailer is orientation only; its brief mentions are not technical evidence for the papers.

### Auxiliary video evidence: eight unique available videos

There are 12 technical paper studies and eight unique available auxiliary videos. Five available clips have full automatic-caption tracks reviewed; three captionless demos were visually sampled at 16 distributed frames each. R-13 is the SIGGRAPH orientation trailer and is listed once in the source catalog and once here only for status accounting. The 43-second Instant NeRF promotional clip is linked to the NeRF paper as a boundary note but is counted once as one video. The attempted NeRF technical talk `LRAqeM8EjOo` was unavailable and is not included in the eight. Caption review does not mean soundtrack or every visual frame was inspected.

| Video ID | Title | Evidence reviewed | Boundary |
|---|---|---|---|
| [R4a7ZCF6_t4](https://www.youtube.com/watch?v=R4a7ZCF6_t4) | 1998 Rendering With Natural Light by Paul Debevec | No captions; 16 distributed visual samples reviewed over 95 s. | Images show environment-lighting examples; visual samples do not prove quantitative agreement. Title and credits confirmed; sampled frames show reflective and translucent colored spheres on sculptural supports in a sunlit courtyard. Reflections and shadows motivate measured illumination; no quantitative physical agreement is inferred from images. |
| [DJ2hcC1orc4](https://www.youtube.com/watch?v=DJ2hcC1orc4) | NVIDIA Instant NeRF: NVIDIA Research Turns 2D Photos Into 3D Scenes in the Blink of an AI | Full en-orig automatic-caption track reviewed; 43 s. | Promotional Instant NeRF clip; transcript contains only brief nontechnical speech, music and applause; not a NeRF lecture. |
| [T_kXY43VZnk](https://www.youtube.com/watch?v=T_kXY43VZnk) | 3D Gaussian Splatting for Real-Time Radiance Field Rendering | Full en-orig automatic-caption track reviewed; ~5 min. | Short 3DGS overview; performance claims belong to the presentation setup, not an independent test. |
| [oaHCtB6yiKU](https://www.youtube.com/watch?v=oaHCtB6yiKU) | 2D Gaussian Splatting for Geometrically Accurate Radiance Fields | Full en-orig automatic-caption track reviewed; ~6:40. | 2DGS overview; imperfect captions; paper supplies formal definitions. |
| [sX5qQEDvHVU](https://www.youtube.com/watch?v=sX5qQEDvHVU) | [SIGGRAPH 2020] Incremental Potential Contact (IPC) - Supplemental | No captions; 16 distributed visual samples reviewed over 160 s. | Supplemental IPC examples; sampled images do not prove absence of every collision. Title confirmed IPC intersection/inversion-free large-deformation dynamics. Samples show deforming shapes and thin structures, slow-motion and zoom views, falling objects and a contact scene. Tiny timing labels are not used as measurements. Samples do not independently establish absence of every collision. |
| [ltX-qUjbkdc](https://www.youtube.com/watch?v=ltX-qUjbkdc) | As-Rigid-As-Possible Surface Modeling | No captions; 16 distributed visual samples reviewed over 210 s. | ARAP deformation examples; samples do not establish global rigidity or convergence. Title and Sorkine/Alexa authors confirmed. Samples show a bent triangulated strip, handle-driven creature/limb deformation, cactus examples, stretched surfaces and relief detail. Samples support the demonstration description, not a global rigidity or convergence guarantee. |
| [U6wtw6W4x3I](https://www.youtube.com/watch?v=U6wtw6W4x3I) | Instant Field Aligned Meshes (SIGGRAPH ASIA 2015) | Full en-orig automatic-caption track reviewed; ~3:30. | InstantMeshes overview; large-model runtime is a reported demo claim. |
| [sIcGH3M5yPc](https://www.youtube.com/watch?v=sIcGH3M5yPc) | What’s Next in Computer Graphics? \| SIGGRAPH 2026 Technical Papers | Full en-orig automatic-caption track reviewed; ~4:36. | SIGGRAPH 2026 orientation trailer; not technical evidence for a specific paper. |




## Source trail for the geometry and numerical expansion

The Parts 1101–1700 source routing, selected textbook pages, caption qualifications and performed checks are recorded in [[Supplementary/Manual Integration - Sources and Verification]].


---

## From captured kernels to illuminated animated surfaces

### Reconstruction checkpoint — project a Gaussian before optimizing the rasterizer

`tools/labs/gaussian_projection_reference_lab.cpp` now serves as the independent CPU oracle for the most important screen-space transformation. It finite-differences the pinhole camera projection and compares the result with the analytic Jacobian; propagates a positive-semidefinite 3D covariance through $J\Sigma J^{\mathsf T}$; checks projected symmetry and nonnegative eigenvalues; verifies the expected quadratic scaling with focal length; exercises low-order spherical-harmonic directionality; and demonstrates that front-to-back alpha composition has order-independent final opacity but generally order-dependent color.

That is deliberately narrower than “Gaussian renderer complete.” Conic inversion, tile duplication/prefix scan, stable tile-depth sorting, higher-order SH, native GPU rasterization, temporal low-pass behavior and current Maveb/Vulkax parity remain their own milestones. [[Gaussian covariance projection and footprint]], [[Gaussian alpha compositing and order]], and [[Captured-world similarity alignment]] carry the derivations and the Sim(3) covariance scale rule into the editing/reconstruction story.


The following extension follows data from GPU work generation through explicit API execution, renderer planning, Gaussian images, lighting, and animated surfaces. Definitions and worked examples precede the retained detailed lessons. Source-project observations about Kairo, Maveb and Vulkax are supplied draft claims; they have not been independently certified against a current repository revision. API examples retained below include fragments and pseudocode. Their presence is not a claim that every snippet was compiled or every device path tested.
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/35 - A Complete Renderer as an Executable Forward Model|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds|Next chapter]]
