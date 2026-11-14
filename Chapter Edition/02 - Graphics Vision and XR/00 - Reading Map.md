# Graphics, Vision and XR — From Geometry to Images and Perception — Reading Map

Follow the image from geometry and light through rendering, inference, captured worlds and perception. The source atlas and extended XR walkthrough are retained explicitly, alongside the technical chapters.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Continuous master]] · [[Chapter Edition/README|All three books]]

| Chapter | Estimated pages | What you will learn |
|---|---:|---|

| [[Chapter Edition/02 - Graphics Vision and XR/01 - From Worlds to Images and Inference|01 — From Worlds to Images and Inference]] | 54.8 | Trace the graphics forward problem and distinguish it from vision’s inverse problem. |

| [[Chapter Edition/02 - Graphics Vision and XR/02 - Surfaces, Topology and Parameterization|02 — Surfaces, Topology and Parameterization]] | 47.6 | Choose between parametric, implicit and mesh representations based on the operation required. |

| [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation|03 — Lens Optics and Image Formation]] | 41.1 | Relate focal length, object distance, focus tolerance and image sampling. |

| [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction|04 — Sampling, Filtering and Reconstruction]] | 57.3 | Explain how sampling and filtering affect image quality and implementation. |

| [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport|05 — Reflectance, Materials and Light Transport]] | 69.2 | Relate material parameters to a physically meaningful scattering model. |

| [[Chapter Edition/02 - Graphics Vision and XR/06 - From Material Data to GPU Bindings|06 — From Material Data to GPU Bindings]] | 61.8 | Trace material data from glTF through engine representation to GPU binding. |

| [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities|07 — The Rendering Equation and Path Probabilities]] | 58.4 | Interpret path contributions and their probability weights in a Monte Carlo renderer. |

| [[Chapter Edition/02 - Graphics Vision and XR/08 - Sampling Directions and Light Transport|08 — Sampling Directions and Light Transport]] | 49.0 | Construct and compare directional sampling strategies for path tracing. |

| [[Chapter Edition/02 - Graphics Vision and XR/09 - Point Lights, MIS and Environment Lighting|09 — Point Lights, MIS and Environment Lighting]] | 49.1 | Compare point-light and environment estimators and explain MIS weighting. |

| [[Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation|10 — A Production Path Tracer: Media, Randomness and Validation]] | 37.6 | Identify the implementation and validation contracts needed for a robust path tracer. |

| [[Chapter Edition/02 - Graphics Vision and XR/11 - BVH Traversal and Deterministic Evidence|11 — BVH Traversal and Deterministic Evidence]] | 58.0 | Use traversal visualizations and deterministic tests to evaluate a BVH implementation. |

| [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty|12 — Probability, Statistics and Uncertainty]] | 40.0 | Distinguish probability models from sample statistics and uncertainty estimates. |

| [[Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space|13 — From View Space to Clip Space]] | 56.0 | Trace a vertex from view coordinates through clip coordinates and clipping. |

| [[Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility|14 — Transparency, Blending and Visibility]] | 58.4 | Explain source-over blending and the visibility tradeoffs around transparency. |

| [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|15 — Rasterization Foundations and Renderer Evidence]] | 42.7 | Separate implementation-specific renderer evidence from general rasterization concepts. |

| [[Chapter Edition/02 - Graphics Vision and XR/16 - GPU Memory, Divergence and Execution Order|16 — GPU Memory, Divergence and Execution Order]] | 52.9 | Reason about memory access, divergence and synchronization in GPU programs. |

| [[Chapter Edition/02 - Graphics Vision and XR/17 - Build, Test and Reproduce the Program|17 — Build, Test and Reproduce the Program]] | 55.2 | Identify the build and test evidence required to reproduce a program. |

| [[Chapter Edition/02 - Graphics Vision and XR/18 - C++ Representation, Ownership and Lifetime|18 — C++ Representation, Ownership and Lifetime]] | 32.1 | Use representation and lifetime invariants to assess C++ resource operations. |

| [[Chapter Edition/02 - Graphics Vision and XR/19 - Textures as Sampled Surface Signals|19 — Textures as Sampled Surface Signals]] | 47.3 | Connect texture sampling, surface footprints and filtering choices. |

| [[Chapter Edition/02 - Graphics Vision and XR/20 - Geometry Queries from Scene to Contact|20 — Geometry Queries from Scene to Contact]] | 49.1 | Trace a collision query through geometry conversion, predicates and contact output. |

| [[Chapter Edition/02 - Graphics Vision and XR/21 - Collision Implementations and Repository Comparison|21 — Collision Implementations and Repository Comparison]] | 47.8 | Compare a reference collision implementation with an existing engine implementation. |

| [[Chapter Edition/02 - Graphics Vision and XR/22 - Convex Support Mapping and Distance|22 — Convex Support Mapping and Distance]] | 49.9 | Compute support points and explain how they enable convex distance queries. |

| [[Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity|23 — Mesh Topology and Connectivity]] | 45.8 | Explain how mesh connectivity differs from a set of geometric positions. |

| [[Chapter Edition/02 - Graphics Vision and XR/24 - Ray–Triangle Intersection|24 — Ray–Triangle Intersection]] | 53.0 | Derive ray–triangle intersection from plane and barycentric tests. |

| [[Chapter Edition/02 - Graphics Vision and XR/25 - SAT for Triangle and Convex Polygon Tests|25 — SAT for Triangle and Convex Polygon Tests]] | 46.3 | Apply SAT to triangle–AABB and convex polygon intersection. |

| [[Chapter Edition/02 - Graphics Vision and XR/26 - Contact Features and Manifolds|26 — Contact Features and Manifolds]] | 64.2 | Explain how contact features become a usable collision manifold. |

| [[Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping|27 — Mesh Queries, Classification and Clipping]] | 62.6 | Distinguish primitive tests from mesh-level ray, overlap and classification queries. |

| [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections|28 — Visibility Queries and Ray Intersections]] | 54.4 | Specify ray-query intervals and surface behavior for visibility. |

| [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries|29 — Acceleration Structures and Robust Queries]] | 46.6 | State BVH invariants and connect them to robust geometric queries. |

| [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion|30 — Mesh Operators, Boundaries and Diffusion]] | 52.0 | Relate mesh operators, boundary conditions and diffusion modes. |

| [[Chapter Edition/02 - Graphics Vision and XR/31 - Learning a Visual Classifier|31 — Learning a Visual Classifier]] | 45.8 | Describe the steps that turn labeled examples into a visual classifier. |

| [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting|32 — Pose Estimation and Robust Fitting]] | 46.0 | Relate image correspondences, camera transforms and robust pose fitting. |

| [[Chapter Edition/02 - Graphics Vision and XR/33 - Projection Scale and Incidence Geometry|33 — Projection Scale and Incidence Geometry]] | 58.2 | Use projection, scale and incidence relationships to constrain geometry. |

| [[Chapter Edition/02 - Graphics Vision and XR/34 - Learned 3D Shapes and Scene Representations|34 — Learned 3D Shapes and Scene Representations]] | 48.8 | Compare 3D representations in relation to the tasks they support. |

| [[Chapter Edition/02 - Graphics Vision and XR/35 - A Complete Renderer as an Executable Forward Model|35 — A Complete Renderer as an Executable Forward Model]] | 52.1 | Trace a renderer from scene inputs to image output and identify its evidence limits. |

| [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints|36 — Source Atlas and Reconstruction Checkpoints]] | 63.6 | Use source provenance and reconstruction checkpoints to understand coverage and evidence boundaries. |

| [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds|37 — Gaussian Splatting and Editable Captured Worlds]] | 54.8 | Explain how anisotropic Gaussian primitives represent and render captured scenes. |

| [[Chapter Edition/02 - Graphics Vision and XR/38 - Projecting Ellipsoids into Screen-Space Conics|38 — Projecting Ellipsoids into Screen-Space Conics]] | 55.7 | Derive the screen-space conic induced by a projected ellipsoid. |

| [[Chapter Edition/02 - Graphics Vision and XR/39 - Tiling, Ordering and Compositing at Scale|39 — Tiling, Ordering and Compositing at Scale]] | 51.8 | Describe the tiling, sorting and compositing stages for large kernel sets. |

| [[Chapter Edition/02 - Graphics Vision and XR/40 - Lighting, Reconstruction and Surface Motion|40 — Lighting, Reconstruction and Surface Motion]] | 50.1 | Connect lighting and reconstruction choices to animated surface representations. |

| [[Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes|41 — Per-Vertex and Per-Corner Mesh Attributes]] | 50.7 | Choose per-vertex or per-corner attributes based on mesh continuity. |

| [[Chapter Edition/02 - Graphics Vision and XR/42 - XR Radiometry, Optics and Perception|42 — XR: Radiometry, Optics and Perception]] | 43.6 | Relate display radiometry and optics to perception in an XR system. |

| [[Chapter Edition/02 - Graphics Vision and XR/43 - XR Extended Walkthrough and Verification Checkpoints|43 — XR: Extended Walkthrough and Verification Checkpoints]] | 43.9 | Reconstruct and verify the XR walkthrough while distinguishing repeated exposition from its added checkpoints. |
