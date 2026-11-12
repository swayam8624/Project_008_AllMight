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
