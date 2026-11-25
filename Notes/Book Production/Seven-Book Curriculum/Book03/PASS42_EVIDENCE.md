# Pass 42 — late-chapter depth expansion and guided workshops

Baseline: b793ca0432ceffa327fa2d0bd565bd4c4bcf7820. All fifteen Chapters 18–32 were expanded additively across six manuscript commits; the original chapter material, 142 SVG assets, Book 1/2 files, approved cover receipt, bibliography and publication layout are retained. The two earlier EPUBCheck issues reported at run 45591871840 were separately corrected: an invalid dc:date metadata value is now ISO 8601, and a stray literal escape tag in Chapter 08 is removed. No teaching content was deleted. The Chapter 21 stale implementation status was brought up to date.

The added material is 25,739 whitespace-delimited words/code/math tokens: 18,248 in the derivation/extension sections and 7,491 in fifteen guided workshops. This is an exact count of inserted section text using whitespace splitting, not a language-model token count or a pure prose word count.

| Chapter | Added words/code/math tokens | Main additions |
|---|---:|---|
| 18 | 1730 | Tiled coverage, interpolation gradients and exact edge ownership |
| 19 | 1635 | Conservative depth hierarchy, floating-depth resolution and alpha compositing |
| 20 | 1728 | Filters, texture footprints, mipmaps, temporal aliasing and sample integration |
| 21 | 1784 | Complete optical-to-raster sample trace, debugging dossier and fair workloads |
| 22 | 1692 | Magnification, focus sensitivity, PSF/MTF and aperture sampling |
| 23 | 1619 | Angular lenses, inverse rays, covariance and calibrated resampling |
| 24 | 1687 | Photon transfer, paired noise variance, exposure and rolling shutter |
| 25 | 1593 | Color-matrix calibration, chromatic adaptation, CIELAB and clipping |
| 26 | 1669 | Analytic pose/intrinsic Jacobians, conditioning, damping and held-out calibration |
| 27 | 1673 | Rectification, disparity uncertainty, plane homography and matching |
| 28 | 1660 | Scale-space response, subpixel fitting, Hamming distance and diagnostic metrics |
| 29 | 1695 | Inverse depth, weighted triangulation, gauge, Schur elimination and multiview |
| 30 | 1830 | Perspective TSDF, sparse indexing, extraction and completeness |
| 31 | 1756 | Snapshot identity, dependency graph, incremental rebuild and stale-task tests |
| 32 | 1988 | Paired benchmarks, uncertainty, ablation and reproducible research project |

## Acceptance and scope

- All 15 new guided workshops give explicit inputs, expected results, diagnostics and a path to a controlled experiment.
- Eleven new standalone C++23 teaching blocks, including two diagnostic record declarations, are extracted from their actual chapter text by `Code Parts/tools/labs/book03_cpp23/scripts/verify_depth_expansion.py`. Its independent bounded witnesses are separate from the existing 34 registered native tests.
- Local GCC 14.2 compilation of the transcribed standalone examples and matching C++ test harness succeeded with -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror and UBSan (no recover). The 10,000 affine half-space tile cases classified 108 full, 4,921 empty and 4,971 uncertain on the local standard library; zero false full/empty claims. Counts are implementation-dependent for uniform integer distributions; the invariant is the absence of incorrect conservative classifications.
- Local independent Python/NumPy numerical checks for all 15 guided workshop families passed (exact edge masks, visibility, filter, camera projection, focus, radial Newton, variance, color, calibration covariance, stereo, feature metrics, three-view triangulation, TSDF support/zero, units and aggregation). The committed checkout-side verifier includes corresponding standard-library numeric witnesses. It will compile the exact text from the checkout; that exact-script CI result is pending at authoring.
- Original cumulative native engineering CI run 45595327503 at 4842a296d57dfef0a83a0e55f57bd72e809cb749 (first expansion batch completed across all 15 chapters) passed. New extension script and guided workshops require their own final revision CI verification. Proposed larger renderer, optical, vision, reconstruction and studio extensions are clearly marked as exercises, not existing implemented or benchmarked production algorithms.
- The approved cover JPEG's SHA-256 was rechecked locally: 69d65d13f5b28695685b8ab42c580c17705d6021e1e05ac158c95e918313ce08.
- The source-level expansion is complete for this pass; no expanded final PDF page count is claimed. Final publication CI and Mac full proof (including EPUBCheck 5.4.0, visual review and promotion) remain required. Historical master-manifest drift remains disclosed.

Run the new check from the repository root:
~~~sh
python3 "Code Parts/tools/labs/book03_cpp23/scripts/verify_depth_expansion.py"
~~~

The existing Mac build and proof commands remain unchanged. No canonical published PDF or EPUB has been overwritten.
