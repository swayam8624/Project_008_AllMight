# Book03 — Pass 19 evidence and acceptance

Chapter 19 — Which Surface Gets Seen. Branch curriculum/book03-geometry-images-pass01.

## Delivered

3,376-word narrative, ten exercises and selected solutions. Five original figures 83–87 bring the total to 87.

- include/book03/visibility.hpp: bounded opaque per-sample visibility, Float64/Float32/UNorm24 storage simulation, forward/reverse order, explicit occupancy, KeepFirst/KeepLast/LowestID ties, screen-linear depth through P18 coverage and pass/fail/tie counts.
- tests/visibility_tests.cpp: constant and crossing-depth surfaces, forward/reverse order, drawing order, far endpoint occupancy, quantized ties, NaN rejection and 18,000 seeded scalar submissions across six order/storage combinations.
- src/visibility_studio.cpp: 32x24 screen-space crossing-field experiment, winner IDs and stored depths, CSV and P6 image.
- CMake registers book03_visibility and book03_visibility_tests; twentieth authored CTest book03_opaque_visibility_contracts. CI source independently checks all four configurations.

## Observed native results

The exact local source copies were checked against committed Git blob identities:
- raster_coverage.hpp: 267716ff7e9c24c1d8d9f5a290b744e14e74b5a6
- visibility.hpp: 4f0818b2fe2c0bd625ceb24004f7e4cfff5d27eb
- visibility_tests.cpp: fbedd4b5bfc7b5d1daf90507d52da059f88b0d72
- visibility_studio.cpp: d154c7d77030f548e260703561a61bdeb6ddfa2f

GCC 14.2.0 (Debian 14.2.0-19), local standalone build:
```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude visibility_tests.cpp -o tests
./tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude visibility_studio.cpp -o studio
./studio inspect
```

Observed: Book03/P19 visibility contracts: PASS. Studio: surface1=288 surface2=96 empty=384. No compiler warnings or UBSan diagnostics observed.

Four actual CSV exports (forward, swapped, reverse, reverse_swapped) were independently checked using sample centers u=x+0.5, v=y+0.5 and the condition 3u+4v<96. Expected forward depth min(0.125+0.5u/32,0.375); reverse depth 1 minus that value. Expected ID is 1 for u<16 and 2 otherwise. All 3,072 rows agreed exactly, including stored depths. Forward and reverse/swapped P6 images compared byte-identical. Figure 87 is a vector reconstruction of the independently verified native mask.

## Boundaries

The buffer accepts already-clipped screen vertices, not arbitrary world triangles. Coverage is exact on P18's snapped grid; interpolated depth uses binary64 and the selected simulated storage conversion. Explicit occupancy differs from a strict hardware clear sentinel. Equal-depth IDs are deterministic only under a stated tie policy and stable identities. No shading, blending, hardware format packing, GPU performance or complete rasterizer claim.

The standalone suite and studio passed; cumulative CMake/CTest, CI execution, full clipping/projection integration, original-source reconciliation and publication proof remain unverified. No PDF or EPUB generated.
