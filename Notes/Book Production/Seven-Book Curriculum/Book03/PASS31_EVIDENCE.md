# Pass 31 — Integrated studio evidence

GCC 14.2.0 (Debian 14.2.0-19), C++23, -O2 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-sanitize-recover=all. Standalone compilation and execution of studio_tests.cpp and studio_main.cpp passed; no warnings or sanitizer findings. Seven prior dependency headers copied byte-for-byte from inspected repository sources and verified via git hash-object. Direct git DNS failed; GitHub connector used for commits.

Native output: P31 PASS max_3d=5.37628e-15 rms_pixels=1.97807e-14 pixels=56000,13600,4800,2400,. Nine vertices, five supplied triangles and 27 exact preassociated observations across three known poses. Three full render passes (reference, reconstructed, reversed reconstructed submission order) checked against an analytic screen region oracle, 230,400 pixel-pass checks of winning IDs and linear RGB; depth checked on every occupied pixel, within 1e-10. Recovered PCA wall normal agrees up to sign. Reference and reconstructed PPM identical. Near/side out-of-frustum rejection and invalid face-index rejection checked. CLI refuses existing output directory (exit 1).

Independent Python standard-library verifier scripts/verify_studio.py ran successfully on actual exported files: every one of 76,800 RGB pixels agrees with analytic coverage and independently encoded palette; PPM equality; OBJ nine vertices and exact five one-based triplets; CSV 27 unique observations; independent aligned stereo max difference 4.9960036108132044e-15 m; recomputed pixel RMS 1.3945207838976845e-14; metrics counts and tolerance checked. PPM SHA256 8143ddb3714124daa871a28854b320aed450ba88c44d3a0543c353bf3738452e.

Exact committed/local source blob IDs:
- raster_coverage.hpp 267716ff7e9c24c1d8d9f5a290b744e14e74b5a6
- visibility.hpp 4f0818b2fe2c0bd625ceb24004f7e4cfff5d27eb
- sampling.hpp b0d61dc26c0f670b9b852e62966cb36c2f93fa2d
- software_renderer.hpp 92e5be6e01e21f1a6546c61f515af11e82a36f32
- epipolar.hpp 0568a83769ed3401a590e0feb3fca2675fe68462
- sfm.hpp ffe711e1ae6dbdc99c7500b453df221e4673a249
- surface.hpp 3195643eba49b96b4d03084732913c239873a43a
- studio.hpp 5e4b7b62fc4cb1542d63b795c1bbfb5cb5418d6a
- studio_tests.cpp fa160b0d2467bffd88a452ec94914c78aed79c69
- studio_main.cpp 78a6343d5c94107f7aba45e5ff455db736294371
- verify_studio.py 226be31d0136a8964f4420743eed990e09600ba8

Scope: known calibrated poses, synthetic preassociated observations, supplied topology, flat constant-depth diagnostic surfaces; no real-photo feature tracking, pose estimation in studio, general clipping, new TSDF fusion, inferred mesh connectivity, GUI or generic file import. The OBJ retains optical convention without material or camera metadata. CLI is not transactional; partial writes on failure possible. The standalone tested integration does not establish cumulative build or full CTest success, nor repair P21 front-end verification. Source reconciliation, figure review and publication acceptance pending. Four original schematic SVGs (135–138); no PDF/EPUB generated.
