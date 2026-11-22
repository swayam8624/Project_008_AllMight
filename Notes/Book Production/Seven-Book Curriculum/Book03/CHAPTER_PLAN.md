# Book 03 — Thirty-two chapters, one continuous journey

This is the **planned editorial route**, not a list of finished chapters. It expands the already substantial Book 03 source into a coherent standalone reader without discarding the original manuscript. The numbers below are **Book 03 reader numbers**, not the historic source-chapter numbers.

## The promise

A reader begins with a question a small child can ask: **“How does a computer know which tiny lights to turn on?”** By the end, they can build, reason about, test and explain the complete forward geometric route from a 3D object to an image—and then use images to estimate part of the world back again.

A recurring scene gives the concepts a home: a little paper house beside a tree, a bright ball, and a moving camera. It starts as a tiny image file, becomes vectors and triangles, turns into 3D surfaces, enters a camera, lands on pixels, becomes a photograph, and finally is reconstructed from two viewpoints. New projects can add different examples when they genuinely teach a new invariant. The paper house is a continuity device, not a requirement to force every topic into one demo.

For any new symbol, explain in ordinary language first. Then show the equation, a numeric case, its C++ representation, its unit/invariant and how to falsify the result. Keep the narrative close to experiments, not vague excitement.

## Part I — From coordinates to shapes

| Reader chapter | The human question that opens it | Deep requirements | Project / observable artifact |
|---|---|---|---|
| 01. The Picture Made of Little Windows | How do tiny stored numbers become a picture at all? | Image array; index versus sample point; row-major memory; RGB8 encoding; line rasterization; file writing; aliasing introduction | P01 — C++23 Pixel Canvas, PPM + assertions |
| 02. Every Place Needs an Address | If I point to “over there,” what does “there” mean? | Cartesian coordinates; axes, units, origins; points versus displacements; frames; world, image and local coordinates | P02 — draggable coordinate playground |
| 03. Shapes That Know Their Borders | How can three dots decide what is inside? | Segments, circles, signed area, winding, barycentrics, inside/outside; exact cases | P03 — triangle editor and hit-testing |
| 04. Move the World Without Losing It | Why does turning and then moving differ from moving and then turning? | Affine transformations, matrix composition, inverse, homogeneous coordinates, determinant/handedness | P04 — live transform laboratory |
| 05. The Triangle That Almost Wasn't There | Why does the same triangle sometimes point both ways? | Floating error, robust orientation, near-collinearity, degenerate cases, exact oracle | P05 — robust geometry comparison |

**Necessary first-principles test:** identical inputs, with exact or independent references where relevant, must make the same decision across supported compiler builds. Errors must be explained as failures of assumptions or representation, not covered up with large arbitrary epsilons.

## Part II — Shapes become 3D objects

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 06. Step Out of the Page | What changes when “above” joins left and right? | Coordinates, planes, normals, dot/cross, distance, intersections | 3D axis and plane inspector |
| 07. One Shape, Many Triangles | Why does a 3D model have more than vertices? | Mesh topology, winding, indices, adjacency, boundary and nonmanifold tests | Indexed mesh explorer |
| 08. Drawing Smooth Things with Control Points | How can a few handles bend a curve? | Bézier, Hermite, B-splines, derivatives, continuity and tessellation | Curve and patch editor |
| 09. Making a World from Rules | Can a mountain, cylinder or shell come from a formula? | Procedural meshes, parameterization, implicit fields and normals | Procedural workshop |
| 10. A Mesh You Can Trust | Why can a file load successfully and still describe nonsense? | Asset ingestion, glTF/OBJ boundary, validation, attribute seams, units, bounds | Mesh inspector |

## Part III — Worlds become camera views

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 11. The Same Tree in Five Worlds | Where is the tree if the camera moves? | Object/world/view spaces, local hierarchies, frame labels, basis change | Frame-chain visualizer |
| 12. Rotations Without Headaches | Why can three simple angle knobs surprise us? | Euler, axis-angle, quaternions, rigid maps, SO(3)/SE(3), pose composition | Orientation debugger |
| 13. The Tiny Hole That Makes a Picture | Why does the distant house look smaller? | Similar triangles, pinhole, focal length, pixel coordinates, intrinsics/extrinsics | Pinhole simulator |
| 14. From a 3D Point to a Screen Address | What does a projection matrix really do? | Perspective/orthographic projections, clip space, NDC, depth, reverse-Z | Projection laboratory |
| 15. Two Eyes Looking at One House | How can two pictures reveal depth? | Multi-camera poses, stereo baseline, rays, triangulation sensitivity | Stereo geometry viewer |

## Part IV — From camera geometry to pixels

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 16. The Long Journey of One Vertex | What happens to this one corner of the house? | Model/view/projection chain, homogeneous division, stage logs | Vertex pipeline tracer |
| 17. Shapes That Refuse to Leave the Window | What if the triangle reaches behind the camera? | Integer lines, exact clip conventions, 2D/homogeneous clipping, boundary cases | Clipping explorer |
| 18. Who Owns the Edge? | When two triangles touch, who colors their common pixels? | Edge functions, winding, sample centers, barycentric coverage, top-left rule | Triangle coverage microscope |
| 19. Which Surface Gets Seen? | Why can the back wall disappear? | Depth buffers, visibility, front/back, perspective-correct interpolation, z-fighting | Z/depth inspector |
| 20. A Pixel Is Not a Tiny Square of Reality | Why do diagonal lines make staircases? | Reconstruction/sampling, footprints, aliasing, MSAA, basic texture filtering | Sampling comparison gallery |
| 21. A Whole Rasterizer We Can Explain | Can our own C++23 code draw a complete 3D scene correctly? | Model loading, CPU draw pipeline, clipping, rasterization, output, performance and proof | P07 — deterministic software renderer |

The reader should be able to ask the rasterizer why any given pixel has that value, and the program should expose enough stage data to answer.

## Part V — Real cameras and photographs

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 22. A Lens Bends the Story | Why does adjusting focus blur the background? | Thin lens, aperture, focal plane, circle of confusion, depth of field | Thin-lens simulator |
| 23. Why Straight Lines Bow | Why do wide-angle cameras curve a building? | Radial/tangential distortion, fisheye models, undistortion | Distortion laboratory |
| 24. Light Becomes a Measurement | Why is a photograph noisy or overexposed? | Irradiance, exposure, f-number, gain, sensor samples, shot/read noise, quantization | Camera sensor emulator |
| 25. Colors That Tell the Truth | Why do numbers that look alike show different colors? | Linear light vs encoding, white balance, response curves, color spaces and dynamic range | Color/response inspector |

Do not confuse sensor imaging with full light-transport simulation; Book 4 owns that.

## Part VI — Images reveal geometry

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 26. Teaching a Camera About Itself | How can a checkerboard reveal focal length? | Camera calibration, intrinsics, DLT, distortion, residuals, noise | Calibration workbench |
| 27. Two Photographs, One Hidden Point | Where could the object be in 3D? | Epipolar geometry, E/F matrices, triangulation, cheirality, gauge | Epipolar explorer |
| 28. Finding the Same Corner Twice | How do we know those pixels refer to the same place? | Features, correspondences, geometric verification, RANSAC, outliers | Matching visualizer |
| 29. Building a Scene from Pictures | Can we reconstruct a little house from several views? | Minimal SfM, camera poses, sparse points, bundle adjustment, error metrics | Minimal reconstruction |
| 30. Points Become Surfaces | Can scattered 3D points become something solid? | Normals, meshes, implicit surfaces, first TSDF principles, confidence | Surface workshop |

## Part VII — One program and its proof

| Chapter | Opening question | Deep requirements | Project |
|---|---|---|---|
| 31. A Studio for Geometry and Images | Can all these tools speak the same language? | Unified data/coordinate contracts, camera/mesh/render/reconstruction modules, interchange, CLI/GUI separation | Geometry-to-Image Studio |
| 32. If It Looks Right, Is It Right? | How can we tell whether a beautiful image is actually correct? | Exact/reference oracles, property tests, numerical vs visual errors, tolerance and scale, reproducibility, figure ethics | Final capstone evidence dossier |

## Project sequence

P01 Pixel Canvas → P02 Coordinate Playground → P03 Robust Triangle Lab → P04 Transform Workshop → P05 Mesh Inspector → P06 Camera Studio → P07 Software Rasterizer → P08 Photographic Camera → P09 Calibration → P10 Mini SfM → P11 Numerical/Visual Error Observatory → P12 Geometry-to-Image Studio.

Each project needs a real executable artifact, user-facing explanation, source ownership contract, deterministic sample input, failure tests and usable illustrations. Some projects will share libraries, but no “project” is counted complete merely because its name exists in a plan.

## Exercise system

Each chapter will contain **Reconstruct**, **Falsify**, **Transfer** and **Explain** tracks:

- Reconstruct: derive the math with units and apply it to a small exact input.
- Falsify: find the input or assumption that breaks the naive version, and show the failing output.
- Transfer: adapt the idea to another camera, representation or real application.
- Explain: tell a younger reader what physically happened while retaining correct terminology.

Responses should include numbers, tests, counterexamples, traces or an explicit evaluation rubric. No unsourced benchmark claims or disguised reference-solution code.

## Figures and sources

Every newly authored figure is treated as a teaching artifact with a hypothesis, source data, generated output, semantic caption, alt text and independent visual/evidence review. Avoid tiny illegible labels or fake photorealism. Selective source families: Scratchapixel; Berkeley CS184; MIT 6.837; Crane's geometry notes; standard camera/calibration literature (e.g. Hartley–Zisserman and Zhang); Shewchuk robust predicates; official glTF and programming documentation. Each specific scientific or normative claim still needs source-by-source verification at its first use before publication.

## Preservation and the book boundary

The original long mathematical/camera/graphics chapters in Notes/Continuous Notes/, Notes/Chapter Edition/ and the existing stage 03 generation remain intact. SOURCE_LEDGER.md tracks where to integrate them. The proposed 32-chapter outline does not authorize rewriting those chapters into shorter 32-paragraph summaries, nor copying Book 4's lighting or Book 7's neural rendering wholesale.

**Pass 01 deliverable:** this route, a complete Chapter 01 draft and a first C++23 Pixel Canvas lab. All further chapters remain planned, not completed.
