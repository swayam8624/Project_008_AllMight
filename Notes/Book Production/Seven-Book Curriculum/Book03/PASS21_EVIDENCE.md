# Book03 — Pass 21 evidence and acceptance

Chapter 21 — A Whole Rasterizer We Can Explain. Branch curriculum/book03-geometry-images-pass01.

## Delivered
2,713-word reader, eight exercises, three new SVGs (93–95, 95 cumulative), backend and front-end adapter with separate tests. Figure 95 contains a lossless four-colour indexed PNG of the native gallery, locally verified pixel-identical to its PPM. The figure is a native screen-space witness, not an executed OBJ render.

## Observed native validation
Exact local and committed Git blob identities:
- software_renderer.hpp: 92e5be6e01e21f1a6546c61f515af11e82a36f32
- renderer_tests.cpp: 28bf0063695e56abf25fc739e36a999a15824e07
- renderer_gallery.cpp: 0d6a565b5f5b5b163eef2ae2ce616f21e4805c00
- raster_coverage.hpp: 267716ff7e9c24c1d8d9f5a290b744e14e74b5a6
- visibility.hpp: 4f0818b2fe2c0bd625ceb24004f7e4cfff5d27eb
- sampling.hpp: b0d61dc26c0f670b9b852e62966cb36c2f93fa2d

GCC 14.2.0 (Debian 14.2.0-19). Local standalone commands:
```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude renderer_tests.cpp -o renderer_tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude renderer_gallery.cpp -o renderer_gallery
./renderer_tests
./renderer_gallery gallery.ppm
```
Observed: Book03/P21 backend contracts: PASS. No compiler warning or UBSan diagnostic. Gallery: triangles=3 candidates=93644 covered=41012 passed=41012 failed=0 ties=0. The PPM is 320x240, SHA256 080c527052dd2c3b15294bd64774fcd5496844de5409730ac04f6d77501da303. Pixel counts: background (53,63,72) 46581; foreground (237,158,97) 11172; dark checker (81,123,109) 9440; light checker (139,206,184) 9607.

An independent Python rational-arithmetic oracle used integer edge determinants at doubled sample-center coordinates, the stated top-left rule, exact rational depth comparisons, and normalized reciprocal-w weights. It checked 76531 exact color matches away from checker discontinuities. The remaining 269 samples are exactly on a checker boundary; each has one of the two permitted checker colors. All 76800 pixels were checked under this stated policy. The 269 ambiguous samples are not counted as exact parity matches.

## Full pipeline integration
renderer_frontend.hpp (blob 417e7fbaffc350c2dad4b195fd4f3011a6f36d9a) connects the actual P10 ObjAsset rendering tuples, P16 VertexStage, P17 clip_triangle/fan_triangulate/to_screen_vertex, and P21 backend. renderer_obj.cpp (blob 520fd76afb09661e0c786f8a7a74c69c145d96f8) reads the bounded OBJ subset and places the model four units forward. renderer_frontend_tests.cpp (blob 9af1c2d12eaa6110491e28ab1c44c1ff1865606a) requires the exact 40000-sample projected quad and full occupancy mask. CMake registers two new CTests and CI checks the native gallery and OBJ command independently.

Local git ls-remote failed: Could not resolve host: github.com. The full dependency tree could not be retrieved to the native compiler. The front-end sources and integration tests are committed, not observed passing. The backend test/gallery ran against the exact P18–20 dependencies already present. No claim of cumulative CMake success, complete OBJ-to-image execution, or production renderer performance.

## Boundaries
Opaque single center sample, bounded positive clip w and UV range, unfiltered diagnostic checker, explicit depth occupancy, fixed surface colors, no physical lighting or texture file loading. Equal represented depth and identical IDs do not establish an order-independent color. Clip rounding and prior source defects may still require integration repair. PPM output is non-atomic. The front end does not automatically frame arbitrary models or load scene files. No PDF or EPUB generated. Original-source reconciliation, final typesetting and publication proof remain pending.
