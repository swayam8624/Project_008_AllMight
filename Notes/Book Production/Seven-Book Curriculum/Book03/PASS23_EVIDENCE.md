# Book03 — Pass 23 evidence and acceptance

Chapter 23 — Why Straight Lines Bow. Branch curriculum/book03-geometry-images-pass01.

## Delivered

2,861-word chapter, eight exercises and five original SVG teaching figures (101–105). The native reference implements polynomial Brown–Conrady distortion in normalized CV coordinates, an analytic Jacobian, a conservative real-arithmetic injectivity bound on a disk, damped Newton inversion with explicit failure statuses and five angular projection families.

## Observed native validation

GCC 14.2.0 (Debian 14.2.0-19), standalone C++23:
```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude distortion_tests.cpp -o tests
./tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude distortion_studio.cpp -o studio
./studio distortion.csv
```

Observed output: Book03/P23 distortion contracts: PASS; max inverse error=1.37772e-12. No compiler warnings or sanitizer diagnostics emitted. The 4,000 seeded interior points use a single fixed coefficient set, rather than 4,000 independently calibrated lenses. All four analytic Jacobian entries were compared against centered finite differences. The test additionally checks a folded model, an outside target and 505 angular radius/inverse round trips.

Exact local and committed Git blob matches:
- include/book03/distortion.hpp: 689ece2565e78e2d1eea4d618e8b2f0a022b5463
- tests/distortion_tests.cpp: f524ca637cd4e15decca0e40bacbd0e6d390ce81
- src/distortion_studio.cpp: 459b44baaf9431c07052ca40e19e10612f47a7c0

An independent Python calculation verified all 265 rows of distortion.csv against an expanded polynomial and recomputed inverse residual. Maximum coordinate recovery error: 1.2999087009795554e-12. Maximum reported residual: 9.524668312934452e-13. Maximum accepted Newton iterations: 5. Forward-map agreement threshold: 1e-14; recovered coordinate threshold: 5e-12; recomputed residual threshold: 1.001e-12. All values are dimensionless normalized coordinates, not measured camera calibration accuracy.

For the reference coefficients k=(-0.18,0.04,-0.005), p=(0.004,-0.003), R=1, the conservative derivative bound B=0.831 implies a real-arithmetic lower separation factor of 0.169. The code accepts B<0.9. Its floating computation is not a formal interval certificate. The forward map can evaluate models that the inverse declines to certify.

## Boundaries

A residual tolerance does not prove exact existence for a target infinitesimally outside the image of the disk. NoProgress and IterationLimit are solver failure statuses, not mathematical nonexistence certificates. The inverse may reject valid lenses under its sufficient-only bound. The angular helpers restrict the five families to the forward hemisphere excluding the rim; they are not a full >180-degree fisheye model. This lab verifies point maps, not a complete image remapper or physical calibration. The figures are analytical diagrams, not measured optical photographs. Native standalone tests and studio passed; cumulative compilation, CI execution, P21 frontend, full source reconciliation and publication rendering remain unverified. No PDF or EPUB generated.
