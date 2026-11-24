# Pass 12 — Original-source custody: rotations and rigid camera poses

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

This pass adds new reader and C++23 laboratory sources. The original continuous notes, derived reading copies, legacy typesetting stage, published Books 01–02 and final distribution directories are read-only.

## Directly inspected original reading-copy identities

- `Notes/Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md`, blob `ca921d2747a6321fb9bebcde97edb086bb74edb7`. Its original header declares Graphics master source lines 95949–99206 and source-span SHA256 `c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9`. Located passages at reading-copy lines 1877–1887 describe dual quaternion skinning, 1966–2030 describe unit-quaternion interpolation and endpoint sign correction, 2098 addresses Hermite rotation sampling and normalization, 2156 and 2220 address interpolation, 2440–2442 discuss encoding redundancy, and 2520–2524 discuss finite data, nonzero quaternions and animation input validation. Only relevant passages were located; this is not full source-unit reconciliation.
- `Notes/Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging.md`, blob `957f1b0ddd3c620cf28e5289b606687c8f32d71c`. Header-declared master source lines 93037–95790 and source-span SHA256 `ff9fb23aab9ebc413a2ce9ee1acf22cb01bdfc4a2c0af26d14b3f670299495fb`. Located passages at reading-copy line 80 describe normalized quaternion camera pose, 1532–1552 identify orthonormality and rigid-camera inverse requirements, 2107 identifies separate translation/quaternion interpolation, 2311 distinguishes a Vec4 from a quaternion. These are source references, not newly recomputed continuous-master hashes.
- Existing Book03 `geometry3d.hpp`, blob `b386703e351ccb6bf8730754949f2683903628e7`, provides Point3, Vector3, dot, cross and unit. `scene_forest.hpp`, blob `5a0d29d51ec730b3b8793dc3c40e0ff5d98f875e`, provides column Linear3 and SceneAffine. Their actual interfaces were read before writing P12.

## Coverage and remaining obligations

| Subject | New P12 treatment | Still owed |
|---|---|---|
| Proper rotation and frame conventions | SO(3), orthogonality, determinant parity, fixed XYZ and active/passive distinction | Full camera calibration frame conventions and imported asset conventions |
| Axis-angle and local rotation algebra | Rodrigues decomposition, skew matrix and exponential identity | Stable general SO(3) logarithm/Jacobians and numerical optimization |
| Quaternion orientation | Hamilton product, conjugate, half-angle, double cover, scale-first normalization | Matrix-to-quaternion conversion for noisy input, quantization and animation key handling |
| Euler singularity | Both lock signs, principal extraction and near-lock flag | Other Euler orders, continuous editor angle unwrapping |
| Orientation interpolation | Shortest-arc SLERP, endpoint sign correction, near-coincident NLERP branch | Multi-key interpolation, angular velocity, continuity across key intervals, cubic quaternion curves |
| Rigid pose | SE(3) composition and inverse, separate linear translation and quaternion interpolation | Screw interpolation, se(3) exponential/logarithm, camera pose estimation, skinning and dual quaternions |

The new chapter and implementation are independently authored. No claim of complete original-source integration, native execution, GPU performance, animation runtime or final publication is made.

**Next:** Chapter 13 — The Tiny Hole That Makes a Picture. Pinhole perspective, camera-local rays, focal length, intrinsics/extrinsics and image coordinates.
