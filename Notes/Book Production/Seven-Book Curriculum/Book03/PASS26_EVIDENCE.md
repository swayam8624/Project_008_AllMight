# Book03 — Pass 26 evidence and acceptance

Chapter 26 — Teaching a Camera About Itself. Four original SVG figures (113–116), eight exercises, spatial normalized DLT, direct one-sided Jacobi SVD, singular-gap rejection, physical camera row decomposition, reprojection residuals and held-out tests. The chapter derives planar homography constraints and describes nonlinear distortion refinement without claiming that the executable implements them.

## Observed validation

GCC 14.2.0 (Debian 14.2.0-19), C++23 with -Wall -Wextra -Wpedantic -Werror -O2 -fsanitize=undefined -fno-sanitize-recover=all. Both standalone test and studio built and ran. An initial studio misleading-indentation warning was corrected before the final clean build.

P26 PASS exact RMS=6.37644e-14 heldout worst=0.0890431. Exact rotated synthetic camera uses 75 landmarks, fx=800, fy=820, skew=4, cx=320, cy=240, translation=(0.1,-0.2,4), rotation angle 0.3 rad about Y. Test checks five intrinsics, translation and third rotation row; 100 seeded trials with 0.2-pixel coordinate noise check a withheld point; a planar-only fixture is rejected. No claim of testing every rotation component independently.

Studio uses a separate identity-rotation camera with the same intrinsics and translation=(0,0,4). Five noise levels, twenty trials each, 75 fitting points and twenty noiseless held-out points per fit. Mean training/held-out Euclidean RMS in pixels:

| Coordinate noise sigma | Mean training RMS | Mean held-out RMS |
|---:|---:|---:|
| 0 | 5.27144e-14 | 7.43867e-14 |
| 0.1 | 0.135655 | 0.021931 |
| 0.25 | 0.343268 | 0.055851 |
| 0.5 | 0.674684 | 0.112667 |
| 1 | 1.354980 | 0.251324 |

Independent NumPy SVD of an unnormalized design matrix for the noiseless identity-camera fixture recovered the reference P to maximum absolute error 4.843059286940843e-11. Python checked count and finite bounds of all 100 CSV summaries and independently recomputed grouped means. The noisy fits themselves were not independently reproduced from their raw observations; those are not exported by the studio. No real-camera accuracy claim.

Exact locally compiled and committed Git blob matches:
- include/book03/calibration.hpp: e2721c49ee7c950f7ca0dc7323cf302d9fb4a0b6
- tests/calibration_tests.cpp: 10b9c4971c8ccab7ca92649234a1f17982c77eeb
- src/calibration_studio.cpp: ee260a477103bc442cfb68dc34ebf96df3d00835

## Boundaries

Spatial DLT minimizes algebraic error; the singular-gap thresholds are heuristic and not a physical uncertainty certificate. Planar multi-view calibration, robust outlier handling, distortion fitting, nonlinear refinement, uncertainty estimation and real-target detection are not implemented. Held-out points are within the approximate fixture region, not extrapolation. Direct Git DNS unavailable; GitHub connector writes verified by exact blob SHA. Cumulative build and CI, P21 front end, full legacy reconciliation, figure rendering review and publication layout remain unverified. No PDF/EPUB produced.
