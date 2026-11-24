# Pass 15 — Original-source custody: stereo and epipolar geometry

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

The new reader and laboratory are additive. Original notes, derived reading copies, previously published books and publication directories are preserved.

## Original passages inspected

- `Notes/Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry.md`, blob `ae269463e966c7c8e19dfa18954d2a4552438833`. Header-declared master range 87112–90166 and source-span SHA256 `d05c86caf710d5dae5ba53d28b3e1e882519751530a0c533621a24830008f64c`. Located reading-copy passages: 404–420 depth ambiguity and metric scale; 2098–2104 rectified stereo and correspondence assumptions; 2216–2285 off-axis and aligned stereo with principal-point correction; 2301–2361 disparity sensitivity, rectification and general geometry; 2366–2490 IPD, baseline, asymmetric display frusta and toe-in; 2541–2655 motion parallax, binocular cues and analytic uncertainty figure.
- `Notes/Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem.md`, blob `2bb93ac0b57df60f2b7e7655ad196373215a30bf`. Header-declared master range 78505–80940, SHA256 `340c3f3eab22b702f8d250fbf0fdb8bd0a1e0b2e848681aa23ee77df8dacef80`. Located lines 1758 and 2248 connect projective normalization and lines to rectification, epipolar geometry and reconstruction.
- Existing `pinhole_camera.hpp` blob `4a980a1fb282d350747d3b9078c09c417b373a0e`, `geometry3d.hpp` blob `b386703e351ccb6bf8730754949f2683903628e7`, and `rotation3.hpp` blob `bd61b3765a18eb9678dcb3582295e58eada1adea`: inspected camera projection, unit-ray and rigid-pose interfaces.

Header-declared ranges and hashes are provenance metadata, not recomputed master hashes. This is targeted source mapping, not a completed original-source reconciliation.

## New treatment and outstanding obligations

P15 derives principal-point-corrected disparity, aligned depth, covariance propagation, epipolar coplanarity, essential/fundamental matrices and rectification homography, closest-line triangulation, cheirality and pixel residuals. The native implementation supports aligned pairs and arbitrary calibrated camera rays with explicit weak-parallax and behind-camera diagnostics.

The lab does not estimate E/F, solve calibration, warp photographs, find corresponding features, optimize reprojection, model occlusion, or simulate an actual headset. These remain future reader and project obligations. The original stereo display and XR-specific material is preserved. All accuracy claims refer to synthetic analytic conditions.

**Next:** Chapter 16 — The Long Journey of One Vertex.
