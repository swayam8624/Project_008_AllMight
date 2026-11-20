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
    build/book02-cpp23/book02_workbench qr-sensitivity > build/book02-cpp23/qr-sensitivity.csv
    build/book02-cpp23/book02_workbench robust-fit > build/book02-cpp23/robust-fit.csv
    build/book02-cpp23/book02_workbench robust-data > build/book02-cpp23/robust-data.csv

Only if matplotlib is available and a visualization is wanted:

    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/heat.csv --output build/book02-cpp23/heat.png
    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/robust-data.csv --output build/book02-cpp23/robust-data.png
    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/qr-sensitivity.csv --output build/book02-cpp23/qr-sensitivity.png

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
stable time stepping -> sampling/uncertainty -> pivoted dense QR ->
robust nonlinear inverse problems.

## Current advanced native milestone

`src/advanced.hpp` implements educational full-column-rank
Householder QR with greedy column pivoting, relative rank rejection,
and the RMS residual computed from the original observations.
The `qr_diagonal_ratio` output is **not** an SVD condition number.
The `qr-sensitivity` export tests a nearly dependent four-by-two
measurement model with a single explicit observation perturbation.

The same native QR solver handles the augmented weighted Jacobian
system for a nonlinear exponential `a*exp(b*x)+c` fit. Huber IRLS
weights and adaptive damping propose steps; the actual Huber objective
accepts or rejects them. The `robust-fit` summary includes final
parameters, objective, final gradient, iteration count, accepted
steps, convergence flag, and parameter error against declared
synthetic truth. `robust-data` exports all 35 observations,
the one known outlier, the generating curve and both estimates.

For this small **one-outlier** synthetic experiment the robust fit
recovers the generating coefficients substantially better than
ordinary squared-error fitting. This is not a universal assertion
about robust regression or a benchmark of any production library.
The synthetic perturbation is 0.008*sin(0.77*i), and sample 15
(zero-based) has a +3.5 outlier. Initial parameters are
(1.5,-0.4,0.3), with the Huber threshold 0.05.

## Still not implemented

A general SVD, exact predicates for arbitrary binary64 coordinates,
sparse matrices or iterative solvers, finite-element assembly in
this native workbench, 3D mesh/graphics clients, production-quality
nonlinear optimization (including globalization/gauge handling),
independent extended-precision numerical oracles, statistical
multiple-seed inference, or full PDF/EPUB visual acceptance.
No results from the Python-led research atlas are relabeled
as C++23 implementations until a native reimplementation exists.
