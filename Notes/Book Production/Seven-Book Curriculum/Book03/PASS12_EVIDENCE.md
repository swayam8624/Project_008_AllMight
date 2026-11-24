# Book03 — Pass 12 evidence and acceptance gates

**Chapter:** 12 — Rotations Without Headaches  
**Date:** 24 November 2026  
**Branch:** curriculum/book03-geometry-images-pass01

## New source

The 4,381-word chapter develops proper rotation matrices and SO(3), fixed-axis matrices and noncommuting rotation order, axis-angle and Rodrigues' formula, skew-symmetric exponential identity, fixed-XYZ Euler extraction and both gimbal-lock cases, Hamilton quaternion multiplication and half-angle derivation, antipodal equivalence, SLERP and near-coincident NLERP, SE(3) rigid composition and inverse, and separate position/orientation interpolation. Twelve reader experiments and selected numeric answers link the math to the C++23 debugger.

New laboratory files in `Code Parts/tools/labs/book03_cpp23/`:

- `include/book03/rotation3.hpp`: private normalized quaternion orientation, scale-first normalization, axis-angle and fixed XYZ factories, Hamilton composition, inverse, vector application, column matrix, short-arc SLERP, independent Rodrigues function, principal Euler extraction with near-lock diagnostic, RigidPose composition/inverse/affine conversion.
- `tests/rotation3_tests.cpp`: quarter-turn and noncommuting witnesses, diagonal-axis cyclic permutation, opposite quaternion representatives, finite extreme normalization and zero/nonfinite rejection, analytic 120-degree arc at thirteen parameters, short arc for 270 degrees, small-angle branch, both signs of Euler lock and ordinary extraction, rigid-pose composition/inverse, 3,000 seeded invariant cases and 10,000 accumulated small updates.
- `src/rotation_studio.cpp`: actual component, determinant and basis inspection, axis/Euler REPL in degrees, endpoint selection, sampling, 61-row CSV and five basis-frame SVG. The drawing is an oblique projection, not a camera renderer.
- CMake targets book03_rotation and book03_rotation_tests and the thirteenth cumulative CTest book03_rotation_and_rigid_pose_contracts. Dedicated CI source validates two analytic arcs, quaternion norm, determinant, gimbal-lock output and fifteen SVG basis lines.

Five new figures: 45-rotation-basis-and-parity.svg, 46-axis-angle-decomposition.svg, 47-euler-gimbal-lock.svg, 48-quaternion-double-cover.svg, 49-slerp-constant-angle.svg. The quaternion circle is a schematic slice of S³, not a picture of the whole four-dimensional sphere. The chapter's samples are analytic references, not recorded native output. Cumulative source figure count: 49.

## Numerical and implementation boundaries

Normalized quaternions obey the unit constraint only to floating precision. Ordinary bounded angles are the tested domain; very large binary64 angles can lose accuracy through representation and argument reduction. Euler extraction near lock deliberately returns an approximate representative with a flag rather than treating three angle fields as authoritative. The near-coincidence NLERP branch is not exactly constant angular speed. At relative 180 degrees the shortest orientation path is not unique. Endpoint-only interpolation cannot encode extra full turns. Rigid pose interpolation is not an SE(3) screw trajectory. No matrix-to-quaternion, noisy-matrix projection, skinning, full animation system or camera renderer is claimed.

**Acceptance:** native compilation, execution of the thirteen cumulative CTests, GitHub Actions completion, actual binary-generated images and numeric exports, original-source reconciliation and final PDF/EPUB composition are unverified. This record describes committed source and intended acceptance checks, not an observed passing run. See PASS12_LEGACY_AUDIT.md for source custody.
