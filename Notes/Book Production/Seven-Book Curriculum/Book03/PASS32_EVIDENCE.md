# Pass 32 — Final capstone evidence

Standalone native GCC 14.2.0 (Debian 14.2.0-19), C++23, -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all. Both capstone_tests and capstone_main compiled and executed without warnings or sanitizer reports. Native output: P32 PASS scale_error=4.79633e-13 jacobian_error=2.08502e-09. Test reconstructs nine P31 facade vertices under each of two translation-scale changes (.5, 2), verifies scaled positions, low reprojection residual and byte-identical encoded PPM. Central differences test 54 projection Jacobian entries with step 1e-5 m and absolute threshold 1e-7 px/m. Identical zero-baseline rays rejected. Two noiseless points pass five nonzero baseline checks.

Paired sweep: seed 3201; 20 trial groups; 200 points/group at Z=4m, X,Y uniform [-1,1] m; four standard-normal draws per point reused for all five baselines; sigma=.25 px per coordinate; known aligned cameras, focal scale 800 px. 100 summary rows, 20,000 attempted and 20,000 accepted reconstructions, 40,000 observations. Mean per-trial RMS:

| Baseline m | Depth RMS m | Position RMS m | Pixel RMS px | First-order depth sigma m |
|---:|---:|---:|---:|---:|
| .05 | .1435047635 | .1463806797 | .1771921472 | .1414213562 |
| .10 | .07141738069 | .07285443247 | .1771921472 | .07071067812 |
| .20 | .03566274038 | .0364012494 | .1771921472 | .03535533906 |
| .40 | .01782453245 | .01823990446 | .1771921472 | .01767766953 |
| .80 | .008911135077 | .009213969418 | .1771921472 | .008838834765 |

Independent Python checked all 100 rows for finite numeric fields, 200/200 counts, 20 rows per baseline, independently computed theoretical approximation and grouped means. No raw per-point observations exported; no independent rerun of the noisy fits. If accepted=0, RMS fields must be treated as undefined, not zero error. No timing experiment performed. Same-scene scale ambiguity under fixed flat diagnostic colors and in-frustum geometry is not general image invariance under arbitrary lighting or clipping.

Exact local Git hashes verified against committed files:
- include/book03/capstone.hpp fcb65867183676a9b19ff484dba7dc1a86a85108
- tests/capstone_tests.cpp 0fea6ed0007ee71451982a7305b93ad5d5ac5874
- src/capstone_main.cpp a2fdaf8d57b0f27a329100dd1a49bc1e0cc84490

Eight inherited headers match the P31 committed identities (see PASS31_EVIDENCE.md). Direct Git access failed DNS; commits and chapter inventory inspected using repository connector. Four original SVGs 139–142. All 32 chapter files individually fetched with matching numbered headings. This does not verify all body links, formulas, figures, source preservation or release typography. Full cumulative build, CI and P21 frontend execution remain unverified in this pass. No PDF/EPUB produced.
