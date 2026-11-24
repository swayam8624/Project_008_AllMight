# Book03 — Pass 20 evidence

Chapter 20 — A Pixel Is Not a Tiny Square of Reality. Branch curriculum/book03-geometry-images-pass01.

## Delivered

2,427-word chapter, ten exercises and selected solutions, five original SVGs 88–92 (92 cumulative). The chapter derives uniform-sampling ambiguity, analytic box response, midpoint quadrature error, single-step coverage, and linear-light resolve. SSAA/MSAA, texture LOD and temporal sampling are explained, not implemented as complete renderers.

## Observed native validation

GCC 14.2.0 (Debian 14.2.0-19), commands from local standalone directory:

```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude sampling_tests.cpp -o tests
./tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude sampling_studio.cpp -o studio
./studio csv samples.csv
```

Observed output: Book03/P20 sampling contracts: PASS. No compiler warnings or UBSan diagnostics emitted. 2,000 seeded trials each check the smooth cosine midpoint bound and single-step sample coverage error. All 960 studio CSV rows were independently checked in Python against analytic box integration and direct midpoint sums to absolute tolerance 1e-12.

Exact local and committed Git blob matches:
- include/book03/sampling.hpp: b0d61dc26c0f670b9b852e62966cb36c2f93fa2d
- tests/sampling_tests.cpp: caf0a47c4d5c4c268991226398fcd71202e3b6b0
- src/sampling_studio.cpp: 6fa5cc60f6324b9fcf327a88b19e049bcf42acab

RMSE over 192 observations per frequency (point / 4 / 16 samples):
- 0.2: 0.02272908 / 0.00135934 / 0.00008473
- 0.6: 0.17560517 / 0.00679432 / 0.00041434
- 1.2: 0.40726376 / 0.00906220 / 0.00051160
- 2.0: 0.36672792 / below 1e-12 / below 1e-12
- 3.7: 0.37946430 / 0.28267849 / 0.00231365

Figure 92 represents these observed scalar results. No GPU benchmark or universal improvement claim.

CMake registers book03_sampling and book03_sampling_tests; twenty-first cumulative CTest book03_sampling_and_quadrature_contracts. CI source independently checks the exported values.

## Source custody and boundaries

Computing Foundations 33 — Projection, Clipping, and Raster Sampling, blob ef481955be60941713582f1e5600191498f4bd8d: targeted located lines 3130 (half-unit coordinate offset) and 3389 (temporal antialiasing jitter and reprojection). Chapter plan blob 77937aecee267f9772c742da0a979c030385af4c confirms reconstruction, footprints, aliasing, MSAA and basic texture filtering scope. P20 independently derives the additional mathematics. Original-source reconciliation remains incomplete.

The implementation is one-dimensional analytic scalar sampling, not a general image resampler, MSAA attachment, mipmap generator, physical sensor simulator or temporal resolve. Extreme finite step coordinates may lose subpixel distinctions; tested domain is bounded. Native standalone suite and studio passed. Cumulative CMake, CI, end-to-end renderer and publication remain unverified. No PDF or EPUB generated.
