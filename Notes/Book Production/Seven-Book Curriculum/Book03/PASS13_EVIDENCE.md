# Book03 — Pass 13 evidence and acceptance gates

**Chapter:** 13 — The Tiny Hole That Makes a Picture  
**Date:** 24 November 2026  
**Branch:** curriculum/book03-geometry-images-pass01

## Committed reader and laboratory

The 4,933-word narrative derives metric and calibrated pinhole projection, positive optical axial depth, graphics-to-optical frame conversion, camera-to-world pose and world-to-optical extrinsics, the intrinsic K matrix and projective 3x4 camera matrix, FOV and units, continuous image-edge coordinates, pixel-center sampling, crop/resize calibration, inverse rays and unprojection, the many-to-one depth ambiguity, and a projection Jacobian with radial null space. Twelve reader exercises and selected numerical answers follow the implementation.

New native C++23 files under `Code Parts/tools/labs/book03_cpp23/`:

- `include/book03/pinhole_camera.hpp`: validated intrinsics, centered FOV factory, crop/resize, camera-to-world pose, optical conversion via center-relative inverse rotation, continuous projection and membership, inverse intrinsics, world and integer pixel rays, axial unprojection, and optical-space Jacobian.
- `tests/pinhole_camera_tests.cpp`: analytic forward projection (420,140), same-ray depth ambiguity, translated/rotated camera with skew (420,380), pixel centers, crop/resize, invalid inputs and 3,000 seeded finite-difference and inverse round-trip checks.
- `src/pinhole_studio.cpp`: seven world landmarks, camera placement and yaw, adjustable focal scales and principal point, pixel-ray inspection, actual continuous-projection CSV and SVG output. This is a point projection diagnostic, not a visibility or rasterization renderer.
- CMake targets book03_camera and book03_camera_tests, and the fourteenth authored CTest book03_calibrated_pinhole_contracts. Dedicated CI source checks the seven landmark coordinates and ray directions against analytic formulas under default, zoomed and moved camera states, and checks the SVG sample count.

Five new figures: 50-pinhole-similar-triangles.svg, 51-world-local-optical-frames.svg, 52-intrinsics-pixel-addresses.svg, 53-focal-length-and-fov.svg, 54-one-pixel-many-depths.svg. Total original SVG teaching sources: 54. The similar-triangle numerical example was corrected to match its drawn ratios. All figures are linked to Chapter 13.

## Deliberate boundaries

The camera's positive optical depth is not ray range or hardware depth. The image membership flag is not visibility or raster coverage. The centered FOV constructor does not describe arbitrary skewed or off-center frusta. No distortion, lens aperture, clipping, calibration fitting, pose Jacobian or image filtering is implemented. Source coordinate arithmetic is binary64, not exact or outward-rounded; extreme finite values may overflow. The seeded derivative domain is bounded. Crop and resize express continuous coordinate maps rather than resampling kernels.

**Acceptance:** native compilation, execution of fourteen CTests, successful GitHub Actions, actual binary-generated exports and print/EPUB proof remain unobserved. Original-source reconciliation is incomplete. These are committed sources and independent expected numerical contracts, not an observed passing runtime. See PASS13_LEGACY_AUDIT.md.
