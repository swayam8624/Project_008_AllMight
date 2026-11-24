# Book03 — Pass 22 evidence and acceptance

Chapter 22 — A Lens Bends the Story. Branch curriculum/book03-geometry-images-pass01.

## Delivered
2,776-word reader, eight exercises and five original SVGs 96–100. The standalone thin-lens reference evaluates real image distance, aperture, focus, defocus diameter, infinity limit, acceptable interval, hyperfocal distance, paraxial pupil ray intersections and a separate infinity-focus diffraction scale.

## Observed native validation
GCC 14.2.0 (Debian 14.2.0-19), standalone C++23:
```sh
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude lens_tests.cpp -o lens_tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all -Iinclude lens_studio.cpp -o lens_studio
./lens_tests
./lens_studio csv lens.csv
```
Observed: Book03/P22 lens contracts: PASS. No compiler warnings or sanitizer diagnostics. 3,000 seeded configurations checked opposite pupil-ray separation, chief-ray image center and focused ray convergence. All 404 CSV rows independently checked in Python using v=1/(1/f-1/z), D*abs(1-v_s/v_z), pixel pitch .004 mm and Airy diameter 2.44*.00055*N. Absolute tolerances 1e-12 mm and 1e-9 pixel intervals, with Python math.isclose default relative tolerance 1e-9.

Exact local and committed Git blob matches:
- thin_lens.hpp: 7e2b30e8ab00ca020abd17c9c36e7f9f5db5bf4c
- lens_tests.cpp: 757a139dcbd519b65f5611a40de4b3d5f2b071af
- lens_studio.cpp: 655f90382deb0dafade6e831ad38d4c8a7ee751d

For f=50 mm, s=3000 mm, c=.03 mm, algebraic near/far limits in metres:
- f/2: 2.801644 / 3.228584
- f/4: 2.627891 / 3.494874
- f/8: 2.337905 / 4.185268
- f/16: 1.915220 / 6.918819

At f/8 the hyperfocal distance is 10466.6667 mm, infinity-focus Airy diameter at 550 nm is .010736 mm. Figures represent analytic geometry and reconstructed verified CSV values, not physical optical bench measurements.

## Boundaries
The paraxial ideal thin lens uses bounded millimetre inputs and a circular pupil in the lens plane. Acceptable returns algebraic limits; intersect with the real-image domain and implementation domain before evaluating extreme endpoints. The far-infinity classification near the threshold is floating, not exact. The Airy expression is for distant focus, not a complete finite-conjugate diffraction or combined point-spread simulation. No physical scene rendering, aperture visibility, sensor noise, exposure or compound optical tracing. The standalone native suite and CSV studio passed, but cumulative build, CI, full P21 front end, source reconciliation and publication rendering remain unverified. No PDF or EPUB generated.
