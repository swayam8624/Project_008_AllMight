# Book Two — C++23 Numerical Geometry Workbench

This standards-only C++23 project is the beginning of the continuously
evolving mathematics/geometry implementation for From Equations to Geometry.
Numerical algorithms are written in C++23; Python is an OPTIONAL data
visualization utility only. The experiments are reference teaching code,
not validated production-grade numerical or computational geometry libraries.

From the repository root:

    cmake -S tools/labs/book02_cpp23 -B build/book02-cpp23 -DCMAKE_BUILD_TYPE=Release
    cmake --build build/book02-cpp23 --parallel
    ctest --test-dir build/book02-cpp23 --output-on-failure
    build/book02-cpp23/book02_workbench matrix
    build/book02-cpp23/book02_workbench orientation
    build/book02-cpp23/book02_workbench fit
    build/book02-cpp23/book02_workbench heat > build/book02-cpp23/heat.csv
    build/book02-cpp23/book02_workbench monte-carlo > build/book02-cpp23/monte-carlo.csv

Only if matplotlib is available and a visualization is wanted:

    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/heat.csv --output build/book02-cpp23/heat.png

## Contract and explicit limitations

Matrix: 2x2 scaled/pivoted elimination; singularity is decided using
a tolerance, not by a symbolic rank proof. Geometry: exact signs only
on a bounded integer grid; the double filter returns UNCERTAIN on hard
inputs, which must not be confused with collinear. Fit: centered ordinary
least squares, not an outlier-robust regression. PDE: 1D explicit heat
equation with fixed zero boundaries and r <= 1/2. Integration: seeded
Monte Carlo with Welford sample variance and analytic reference 2/pi;
identical seeds are not promised to give identical random doubles on all
standard-library implementations. A successful test does not certify
unexamined input ranges, physics, numerical conditioning, or performance.

The progression is vector/matrix -> predicates -> least squares ->
stable time stepping -> sampling/uncertainty. Later engineering passes
should add QR/SVD, exact general predicates, sparse solvers, robust
losses, nonlinear optimization, independent high-precision references,
multiple independent RNG seeds, and real graphics/physics clients.
