# Book03 — Pass 18 evidence

Chapter 18 — Who Owns the Edge. Branch curriculum/book03-geometry-images-pass01.

## Delivered
- 2,781-word narrative, ten exercises and selected answers.
- include/book03/raster_coverage.hpp: 256-step snapped screen grid, signed 64-bit checked edge arithmetic, winding normalization, top-left equality ownership, signed floor/ceil bounds, incremental traversal and unbiased barycentric weights restored to original input order.
- tests/raster_coverage_tests.cpp: analytic rectangle ownership for both diagonals, negative and half-pixel coordinates, signed division, tie rounding, snap collapse, outside bounding box and 2,000 seeded triangles compared to direct evaluation and reversed winding. Covered samples reconstruct the snapped position from barycentric weights.
- src/raster_studio.cpp: P17 clipped fan and rectangle coverage masks, CSV and P6 images via P01 Canvas. Expected rectangle: 391 single hits, zero duplicate hits, 377 uncovered.
- CMake targets book03_raster and book03_raster_tests; nineteenth cumulative CTest book03_integer_raster_coverage_contracts; CI checks full analytic masks and image bytes.
- Five original SVG figures 78–82, total 82. The analytic expected mask is not a screenshot of a compiled studio.

## Observed standalone native validation
The two exact committed files were reproduced locally and Git blob identities compared:
- raster_coverage.hpp: 267716ff7e9c24c1d8d9f5a290b744e14e74b5a6
- raster_coverage_tests.cpp: 0de52c5221a8d72baedd2c195dd71662605b4c07

GCC 14.2.0 (Debian 14.2.0-19). Command:
```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude raster_coverage_tests.cpp -o tests
./tests
```

Observed output: Book03/P18 coverage contracts: PASS. No compiler warnings or sanitizer diagnostics were emitted. Both exact hashes were rechecked on a second successful run. This is isolated native execution of the standalone coverage source and its tests, not a cumulative CMake/CTest or macOS run.

## Limits and outstanding gates
Coverage is exact only after snapping within the stated coordinate domain. Quantization changes continuous geometry and can collapse triangles. Shared-edge proof assumes an actual partition with identical snapped endpoints. P17 floating clipping can still reject unresolved polygons or introduce inconsistencies. Returned double weights approximate exact rational weights. No depth or shading is performed. The integrated studio, cumulative native CMake build, nineteen CTests, CI and final publication render remain unverified. All originals and publication directories are unchanged. No PDF or EPUB generated.
