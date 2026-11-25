# Pass 30 — Surface workshop evidence

Native GCC 14.2.0 (Debian 14.2.0-19), C++23, -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all. Initial studio build failed misleading-indentation warning; corrected return placement, then clean compilation and execution. Tests printed P30 PASS normal agreement=1 weighted crossing=1.025. The exact 81-point tilted plane normal agreed up to sign within 1e-12; collinear input rejected. Weighted depth crossing 1.025 m checked to 1e-12, front clamping/weight accumulation, far-behind unknown and missing column tested.

Studio: 100 trials, twenty per frame count, 219 observed and 37 deliberately missing columns per trial, aligned orthographic depth, voxel spacing .02 m, truncation .12 m, noise sigma .015 m. All 21,900 requested observed crossings returned (optional.value() would fail otherwise). Mean per-trial RMSE in metres:

| Frames | First RMSE | Fused RMSE |
|---:|---:|---:|
| 1 | .01504055933243812 | .01504055933243812 |
| 2 | .015103047657259509 | .010480709767085249 |
| 4 | .015085915850749287 | .007445002873507997 |
| 8 | .014847657440322001 | .005318748112129023 |
| 16 | .014973195780211876 | .0037659694008007312 |

Python independently checked 100 CSV rows, finite errors, column count conservation, group counts/means and exact-to-1e-12 one-frame RMSE equality. Compared with analytical .015/sqrt(N); no independent replication of random draws. Missing columns were skipped by construction, not queried individually in the studio; separate unit test checks absence of a crossing on missing input.

Exact locally compiled and Git-verified blob identities:
- include/book03/surface.hpp: 3195643eba49b96b4d03084732913c239873a43a
- tests/surface_tests.cpp: 425da9f4300f15351456592214c2ec1e471d8f54
- src/surface_studio.cpp: 6b9c54ef3fabe06af3d6974d6780c93e64541654

Limits: aligned orthographic registered depth only, not moving perspective depth fusion. No meshing, nearest-neighbour search, oriented-normal propagation, sparse blocks, real-sensor data, noise-correlated evaluation or GPU implementation. PCA eigensolver is bounded and does not separately report iteration-budget exhaustion; weight overflow is not transactional. Five original SVG figures 130–134; final rendering/layout review pending. Cumulative CMake/CTest and CI, P21 frontend and full original-source reconciliation remain pending. No PDF/EPUB generated.
