# Book03 — Pass 28 evidence and acceptance

Chapter 28 — Finding the Same Corner Twice. Four original SVG teaching figures (121–124), eight exercises, a grayscale Harris detector with local NMS, normalized 7x7 descriptors, squared-distance ratio test and unique reverse nearest-neighbour check. The separate RANSAC estimator reuses P27 normalized eight-point estimation and Sampson residuals, with bounded refit and explicit missing-model status.

## Observed validation

GCC 14.2.0 (Debian 14.2.0-19), C++23, -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all. Both standalone test and studio compiled and ran with no compiler warnings or sanitizer diagnostics.

P28 PASS translated matches=107. The 64x64 seeded texture and its shifted, positive-affine intensity-transformed copy yielded 107 correct pairs from 107 selected source features in corresponding interior regions. The Harris threshold in the second image was deliberately scaled by 0.6^4. Separate witnesses cover flat image, single straight edge, constant/boundary descriptor, duplicate descriptor ties and repeated degenerate correspondences. No claim of general connected-plateau suppression or photometric adaptation.

The separate RANSAC studio performed 50 fits at five corruption levels, ten independent scenes per level, 100 proposed correspondences per scene, sigma=0.2 pixels per coordinate, 1 pixel Sampson threshold and 1000 attempted hypotheses per fit. 50 noiseless held-out observations per successful fit; all 50 found models. 2500 held-out observations total. Python checked all CSV counts, finite nonnegative held-out errors and precision/recall formulas, and independently aggregated group means. It did not independently recompute the noisy fits because raw observations are not exported.

| Corruption % | Mean precision | Mean recall | Mean held-out RMS px | Total false positives |
|---:|---:|---:|---:|---:|
| 0 | 1.00000000 | 1.0 | 0.05680536 | 0 |
| 10 | 0.99670330 | 1.0 | 0.05579447 | 3 |
| 20 | 0.99753086 | 1.0 | 0.09997074 | 2 |
| 30 | 0.99440532 | 1.0 | 0.07431408 | 4 |
| 40 | 0.99508197 | 1.0 | 0.10151469 | 3 |

Exact locally compiled and committed Git blob matches:
- include/book03/epipolar.hpp (existing P27 dependency): 0568a83769ed3401a590e0feb3fca2675fe68462
- include/book03/features.hpp: 7565dff38549e76a343204ce3a6b248dcc566037
- include/book03/robust_matches.hpp: e7d7fc07d88a6392599a147e49a982236ee035c5
- tests/feature_tests.cpp: a98428c6de9343bb58f0fa1f69ef48eba8c3b77a
- src/matching_studio.cpp: f4518ef5de8ee8a3a3cbf369098a2708db9fbbd5

## Boundaries

Synthetic image translation and synthetic spatial correspondence verification are separate experiments, not an end-to-end real-photo matching pipeline. No scale/rotation-invariant implementation, SIFT/ORB clone, subpixel refinement, descriptor index, adaptive RANSAC stopping or competing-model detection. The Image teaching struct has public storage/dimensions and callers must preserve its construction invariant. Refit and SVD thresholds are heuristic. Uniform random outliers are not coherent scene ambiguity. Precision and recall depend on fixture labels; no universal performance or camera accuracy claim. The CSV is an experimental output, not a manuscript source. Local Git DNS failed; connector writes verified by exact blob SHA. Cumulative CMake/CTest, CI, P21 front end, source reconciliation, final visual rendering and publication acceptance remain pending. No PDF/EPUB generated.
