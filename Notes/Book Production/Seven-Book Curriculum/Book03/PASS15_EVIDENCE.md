# Book03 — Pass 15 evidence and acceptance gates

**Chapter:** 15 — Two Eyes Looking at One House
**Branch:** curriculum/book03-geometry-images-pass01

## Committed source

The 4,327-word reader derives calibrated stereo disparity, principal-point correction, axial-depth sensitivity and covariance, epipolar plane, essential and fundamental matrices, ideal rectification, closest-ray midpoint, positive-depth checks, reprojection objectives and the false-correspondence counterexample. Twelve exercises and selected answers accompany the project.

- `include/book03/stereo_geometry.hpp`: RectifiedStereo, corrected disparity, row residual, axial depth, first-order uncertainty, closest-line estimator with weak-parallax and behind-camera diagnostics, ray gap, midpoint and per-eye reprojection residuals, world-space coplanarity.
- `tests/stereo_geometry_tests.cpp`: independent analytic four-metre witness, shifted principal points, skew-ray midpoint and one-metre gap, parallel and antiparallel rejection, false match at two metres, 3,000 seeded arbitrary-pose cases and reciprocal-depth uncertainty.
- `src/stereo_studio.cpp`: synthetic landmark pair, baseline and deterministic pixel perturbation controls, two-view SVG and 101-row analytic uncertainty CSV. No image matching or raster visibility.
- CMake targets book03_stereo and book03_stereo_tests; CTest book03_calibrated_stereo_contracts. Workflow source checks known source coordinates, false correspondence and baseline-dependent analytic exports.

Six new original figures 61–66 bring the source figure count to 66. All are linked in Chapter 15. The uncertainty figure uses fB=80 pixel-metres and sigma disparity=0.25 pixels, yielding local depth standard deviations 0.05 m at 4 m, 0.2 m at 8 m, and 0.8 m at 16 m.

## Boundaries

Closest-line midpoint does not minimize reprojection error. Angular threshold is an engineering heuristic, not a certified conditioning bound. The rectified implementation requires equal focal scales and zero skew; the general method supports the full P13 intrinsics. Wrong correspondences can satisfy epipolar, positive depth and reprojection checks. The uncertainty expression is local and conditional on correct matching and exact calibration; mapped disparity intervals are not automatically confidence intervals. No physical sensor data, actual matching, calibration fitting, rectification resampling, bundle adjustment or GPU benchmark is claimed.

**Acceptance:** native compilation, CTest execution, successful GitHub Actions, compiled-program SVG/CSV output, final figure rendering and complete source reconciliation remain unobserved. This is a committed source and test-contract record. No PDF or EPUB generated.
