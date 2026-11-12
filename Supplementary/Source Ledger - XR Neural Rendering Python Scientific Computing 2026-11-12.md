# Source Ledger — XR, Neural Rendering, Python, Scientific Computing and 3D Data Integration

Date checked: 2026-11-12

This ledger records sources actually inspected for this integration pass. It is not a claim that no other relevant source exists on the public internet.

## Uploaded sources actually inspected

- Luciano Ramalho, Fluent Python, 2nd ed. — Python data model, sequences, memory views, protocols, generators, concurrency, async and metaprogramming.
- Micha Gorelick & Ian Ozsvald, High Performance Python — profiling, NumPy, allocation, compilation, concurrency, multiprocessing, clusters and memory reduction.
- Wes McKinney, Python for Data Analysis, 2nd ed. — NumPy, pandas, loading/cleaning/joining/reshaping, visualization, grouping, time series and advanced NumPy.
- Michael T. Heath, Scientific Computing: An Introductory Survey, 2nd ed. — approximation/error, conditioning/stability, floating point, linear systems, least squares, eigenproblems, nonlinear equations, optimization, interpolation, integration/differentiation, ODE/PDE, FFT and random numbers.
- Abelson, Sussman & Sussman, Structure and Interpretation of Computer Programs, 2nd ed. — procedural/data abstraction, state, concurrency, streams, interpreters, register machines, storage and compilation.
- Takuro Wada, 3D Modeling and Printing by Python — STL triangle data, Blender Python API, script modeling and parameterized/repeated geometry.
- 3D Data Science with Python — point clouds, voxels, meshes, k-d trees, octrees/BVH, PCA features, ICP, RANSAC, region growing, meshing, voxelization, LiDAR, PointNet, segmentation/foundation models, Gaussian splatting and spatial AI.

## XR specifications and platform/runtime documentation actually checked

- Khronos OpenXR 1.1 specification and registry.
- Khronos OpenXR 1.1 updates, including local-floor space and stereo foveated rendering promoted into core.
- Apple Developer: Compositor Services.
- Apple Developer: Drawing fully immersive content using Metal.
- Apple Developer: Understanding the visionOS render pipeline.
- Varjo developer documentation: supported features.
- Varjo developer documentation: OpenXR overview.
- Varjo developer documentation: human-eye-resolution quad view and foveated rendering.
- NVIDIA VRWorks overview.
- NVIDIA VRWorks Variable Rate Shading documentation.
- Meta Horizon OS developer documentation: compositor, time warp and positional time warp.
- Meta Horizon OS developer documentation: Eye Tracked Foveated Rendering.

## Tracking and XR research actually checked

- Campos et al., ORB-SLAM3: An Accurate Open-Source Library for Visual, Visual-Inertial and Multi-Map SLAM.
- OpenVINS project/documentation was searched as a filter-based VIO reference family.
- Existing repository ILLIXR and FoveaBench material was re-audited. The evidence boundary remains: replay/software Vulkan evidence is not physical headset or hardware-VRS validation.

## Neural rendering and real-time neural graphics actually checked

- Existing repository deep sources: NeRF, Instant-NGP, 3D Gaussian Splatting, 2D Gaussian Splatting and 3DGUT.
- Yu et al., Plenoxels: Radiance Fields without Neural Networks.
- NVIDIA Real-Time Graphics Research: Real-time Neural Radiance Caching for Path Tracing.
- NVIDIA Research: Random-Access Neural Compression of Material Textures.
- NVIDIA Research: Real-Time Neural Appearance Models.
- NVIDIA / SIGGRAPH 2025 course: An Introduction to Neural Shading.
- NVIDIA Real-Time Graphics Research publication index, including current 2025-2026 neural rendering/material/light-transport directions.
- Li et al., DR-GS: Physically-Based Deformable and Relightable 2D Gaussians, 2026.
- Ahmad et al., From Fields to Splats: A Cross-Domain Survey of Real-Time Neural Scene Representations, 2025.
- Kwon et al., R3eVision: A Survey on Robust Rendering, Restoration, and Enhancement for 3D Low-Level Vision, 2025.

## Video sources actually checked or resolved

- Two Minute Papers: This Neural Network Learned To Look Around In Real Scenes! (NERF).
- Two Minute Papers: Creating Virtual Worlds 20x Faster! (3D Gaussian Splatting).
- Existing repository-recorded 3DGS/2DGS/Instant-NGP and reconstruction video evidence was re-audited where already present.
- Searches were run for SIGGRAPH XR courses, Stanford VR, MIT graphics/VR, NVIDIA neural-rendering videos, Freya Holmér, The Cherno, Sascha Willems and Brendan Galea. Queries that did not resolve to a sufficiently specific inspected source are not mislabeled as reviewed evidence.

## Integration map

- XR/perception/vision/tracking/neural-scene work -> University graphics/vision volume.
- XR subsystem ownership, OpenXR-facing architecture, foveation mechanisms and neural-renderer capability contracts -> Game Engine volume.
- Python object/data model, profiling, numerical stability, scientific computing, data analysis and reproducibility -> Computing Foundations volume.
- 3D Python point-cloud/reconstruction work -> University graphics/vision volume, with DCC/asset-pipeline consequences linked into the Engine volume.


## Additional foundational XR sources checked in the second sweep

- Ivan Sutherland, “The Ultimate Display” (1965), located through historical VR literature and retained as the conceptual origin for interactive computer-mediated displays.
- Ivan Sutherland, “A Head-Mounted Three Dimensional Display” (AFIPS 1968), cross-checked through Steve LaValle's archive of the original HMD photographs and historical notes.
- Steve LaValle, Virtual Reality historical HMD archive — clarified that “Sword of Damocles” more precisely names the overhead mechanical position sensor rather than the complete 1968 display system.
- Paul Milgram and Fumio Kishino, “A Taxonomy of Mixed Reality Visual Displays,” IEICE Transactions, 1994 — reality–virtuality continuum plus world-knowledge, reproduction-fidelity and presence dimensions.
- Ronald T. Azuma, “A Survey of Augmented Reality,” Presence, 1997 — real/virtual combination, real-time interaction, 3D registration, and the central role of sensing/registration error.
- Skarbez et al., “Revisiting Milgram and Kishino's Reality-Virtuality Continuum,” Frontiers in Virtual Reality, 2021 — modern critique/revision of the classic continuum.
- Bozkir et al., “Eye-Tracked Virtual Reality: A Comprehensive Survey on Methods and Privacy Challenges,” Proceedings of the IEEE 113(10), published online 2026 — eye-tracking pipeline plus authentication/privacy literature.
- David-John, Butler and Jain, “Privacy-preserving datasets of eye-tracking samples with applications in XR,” IEEE TVCG, 2023.
- Maynard et al., “EyeSpy: Inferring Eye Gaze via Side-Channel Attacks Against Foveated Rendering,” IEEE Symposium on Security and Privacy 2026 — foveation-dependent GPU timing as a gaze side channel.
- NVIDIA CloudXR SDK 6.2.1 / 6.2.0 / 6.1.0 release documentation, checked November 2026 — current OpenXR-server, WebRTC, hand/controller/haptics, network quality and foveated-streaming capabilities.

## Additional neural-scene papers checked in the second sweep

- Chen et al., “TensoRF: Tensorial Radiance Fields,” ECCV 2022 — tensor-factorized radiance fields.
- Fridovich-Keil et al., “K-Planes: Explicit Radiance Fields in Space, Time, and Appearance,” CVPR 2023 — plane-factorized arbitrary-dimensional radiance fields.
- Barron et al., “Zip-NeRF: Anti-Aliased Grid-Based Neural Radiance Fields,” ICCV 2023 — scale-aware anti-aliasing for fast grid-based fields.
- Li et al., “Neuralangelo: High-Fidelity Neural Surface Reconstruction,” CVPR 2023 — neural SDF surface reconstruction.
- Yu et al., “Mip-Splatting: Alias-free 3D Gaussian Splatting,” CVPR 2024 Best Student Paper — scale-aware 3D and 2D filtering for Gaussian splats.
- Wu et al., “4D Gaussian Splatting for Real-Time Dynamic Scene Rendering,” CVPR 2024 — time-dependent Gaussian representation via neural voxel features and deformation prediction.
- Guédon and Lepetit, “SuGaR: Surface-Aligned Gaussian Splatting for Efficient 3D Mesh Reconstruction and High-Quality Mesh Rendering,” CVPR 2024 — surface alignment, mesh extraction and mesh-bound Gaussian refinement.

## Additional learning/video/channel sources actually resolved

- Freya Holmér, “Math for Game Devs” 2022 series; the checked first part covers numbers, vectors, normalization, dot product, scalar/vector projection and assignments. A course index was also checked for the broader ten-part sequence through matrices, trigonometry, interpolation, derivatives, splines and procedural animation.
- Brendan Galea, “Vulkan Game Engine Tutorial” YouTube series — checked the introductory video and its stated engine-from-scratch Vulkan scope.
- Sascha Willems, Vulkan examples / HowToVulkan GitHub presence — checked current 2026 profile/resources and explicit Vulkan example structure.
- The Cherno / Hazel game-engine series — current channel/resource index checked; retained as engine-construction supplementary material rather than primary specification evidence.
- MIT 6.837 Introduction to Computer Graphics — current MIT Graphics course index and an archived 2020 lecture-series index checked; topics span curves, transforms, scene graphs, animation, ODEs/cloth, ray tracing, acceleration, materials, sampling, rasterization, shadows, color, image processing and displays.
- Stanford CS 148 Introduction to Computer Graphics and Imaging, Fall 2026 course page — checked current topic coverage including rasterization, meshes, light/color, BRDFs, ray tracing, sampling and acceleration structures.
- Khronos Vulkan Learn page — checked its current learning-resource index, including Brendan Galea tutorials and TU Wien's Vulkan lecture series.
- NVIDIA/SIGGRAPH 2025 “An Introduction to Neural Shading” course page — checked course scope, presenters, slides/code/video links and real-time neural-shader deployment focus.

