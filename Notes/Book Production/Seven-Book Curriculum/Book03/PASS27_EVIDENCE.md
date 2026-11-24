# Book03 — Pass 27 evidence and acceptance

Chapter 27 — Two Photographs, One Hidden Point. Four original SVG figures (117–120), eight exercises, relative optical pose, E and F, epipoles, Sampson residual, RMS-normalized eight-point estimation, Jacobi SVD and rank-two projection. Essential decomposition, cheirality, triangulation and scale gauge are derived; P15 retains its existing closest-ray implementation.

## Observed validation

GCC 14.2.0 (Debian 14.2.0-19), C++23, -Wall -Wextra -Wpedantic -Werror -O2 -fsanitize=undefined -fno-sanitize-recover=all. Standalone test and studio both built and ran without compiler warnings or sanitizer diagnostics.

P27 PASS max exact Sampson squared=4.68233e-26. The 100 noiseless noncoplanar correspondences agree with analytic F up to sign and scale. A separate 100-point planar fixture is rejected. A four-pixel perturbation gives nonzero residual, unchanged by scaling F by seven within tolerance.

Studio: five noise levels, 20 fits per level, 100 fitting correspondences and 50 independent noiseless held-out correspondences per fit: 100 fits and 5,000 held-out matches.

| Noise sigma (px per coordinate) | Mean training Sampson RMS (px) | Mean held-out Sampson RMS (px) |
|---:|---:|---:|
| 0 | 6.47562e-14 | 6.62678e-14 |
| 0.1 | 0.09605084626 | 0.02770693428 |
| 0.25 | 0.2367632291 | 0.07125360742 |
| 0.5 | 0.4767195644 | 0.148025711 |
| 1 | 0.9744861877 | 0.2969876974 |

Python independently checked count, finite nonnegative values, gap bound and grouped means of all 100 CSV rows. A separate independently seeded NumPy fixture with the same generating camera compared unnormalized design-matrix SVD against analytic F (normalized sign-invariant Frobenius difference 1.40788e-16). This is independent geometry verification, not replication of the C++ random stream or independent recomputation of every noisy fit, because raw observations are not exported.

Exact locally compiled and committed Git blob matches:
- include/book03/epipolar.hpp: 0568a83769ed3401a590e0feb3fca2675fe68462
- tests/epipolar_tests.cpp: e3bd74b20daed6ef9169a90cfb80a7f2730e7c51
- src/epipolar_studio.cpp: 031584d81bd1f4de289f1e220e51828f883dda55

## Boundaries

Synthetic known correspondences only, not actual photographs. Eight-point estimator uses RMS rather than mean radial normalization; singular-gap thresholds are heuristic. Sampson residual is a first-order approximation, not a 3D error. No essential pose decomposition, novel triangulation code, robust outlier selection or feature matching in P27. General minimal eight-point, noisy degeneracies and extreme coordinate regimes need further independent tests. Direct Git DNS unavailable locally; repository writes verified by blob SHA. Cumulative CMake/CTest and CI, P21 front end, source reconciliation, final visual review and publication rendering remain pending. No PDF/EPUB generated.
