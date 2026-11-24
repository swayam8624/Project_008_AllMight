# Pass 29 — Reconstruction evidence

Native GCC 14.2.0 C++23 standalone build with -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all. Initial studio indentation warning corrected before final clean build. P29 PASS exact 3D max=5.66557e-13 improved=80 votes=80,0,0,0. Exact pose recovery checked against all rotation rows and translation direction; known-pose three-view triangulation tested on 80 points, followed by noisy point-only refinement at sigma=.3 px. Duplicate camera observations and identical parallel rays rejected.

Studio: 100 trials (20 per noise level), 100 points each, three supplied camera poses. Pixel RMS is per observation, 3D RMS per landmark in baseline units. Mean of trial RMS values:

| Noise sigma px | Initial pixel RMS | Refined pixel RMS | Initial 3D RMS | Refined 3D RMS |
|---:|---:|---:|---:|---:|
| 0 | 1.068435387e-12 | 4.529634793e-14 | 1.694495669e-13 | 2.626814958e-15 |
| .1 | .09963017497 | .09962069005 | .008908928823 | .008904289614 |
| .25 | .2526628393 | .2526354234 | .02065101873 | .02065786754 |
| .5 | .5068342726 | .5067545598 | .04346533634 | .04342867416 |
| 1 | 1.011609746 | 1.011267383 | .08374200474 | .08389263404 |

All 100 rows passed independent Python checks for counts, finite nonnegative values, grouped means and nonincrease of pixel RMS. Only 63 rows improved 3D RMS; report this rather than equating reprojection fit with 3D accuracy. A separate NumPy stacked least-squares fixture on 100 noiseless points gave maximum 3D error 6.69873e-14; independently checks geometry, not the actual C++ random stream. No raw observations exported; noisy fits not independently replicated.

Exact locally compiled and committed blob identities:
- epipolar.hpp (P27 dependency): 0568a83769ed3401a590e0feb3fca2675fe68462
- sfm.hpp: ffe711e1ae6dbdc99c7500b453df221e4673a249
- sfm_tests.cpp: 2804a5f92f6104c58067cfd2b3050dd218bfb5d2
- sfm_studio.cpp: e292ee8bd800aed89db301b56ec45a21937e2787

Limitations: exact two-view pose recovery and separate known-pose noisy multiview point refinement, not a complete incremental reconstruction. No track graph construction, noisy pose benchmark, PnP, joint bundle adjustment, real-photo input, or end-to-end reconstruction. Common calibrated K and valid finite proper rotations are caller contracts. Numerical thresholds are heuristic. Cumulative CMake/CTest, CI, P21 frontend, source reconciliation and visual publication acceptance pending. Direct Git DNS failed; connector writes verified by blob SHA. No release generated.
