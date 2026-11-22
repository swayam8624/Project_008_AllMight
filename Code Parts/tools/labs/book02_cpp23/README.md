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
    build/book02-cpp23/book02_workbench orientation-exact
    build/book02-cpp23/book02_workbench orientation-audit > build/book02-cpp23/orientation-audit.csv
    python3 tools/labs/book02_cpp23/predicate_oracle.py build/book02-cpp23/book02_workbench
    build/book02-cpp23/book02_workbench fit
    build/book02-cpp23/book02_workbench heat > build/book02-cpp23/heat.csv
    build/book02-cpp23/book02_workbench monte-carlo > build/book02-cpp23/monte-carlo.csv
    build/book02-cpp23/book02_workbench qr-sensitivity > build/book02-cpp23/qr-sensitivity.csv
    build/book02-cpp23/book02_workbench robust-fit > build/book02-cpp23/robust-fit.csv
    build/book02-cpp23/book02_workbench robust-data > build/book02-cpp23/robust-data.csv
    build/book02-cpp23/book02_workbench ransac-summary > build/book02-cpp23/ransac-summary.csv
    build/book02-cpp23/book02_workbench ransac-data > build/book02-cpp23/ransac-data.csv

Only if matplotlib is available and a visualization is wanted:

    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/heat.csv --output build/book02-cpp23/heat.png
    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/robust-data.csv --output build/book02-cpp23/robust-data.png
    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/qr-sensitivity.csv --output build/book02-cpp23/qr-sensitivity.png
    python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/ransac-data.csv --output build/book02-cpp23/ransac-data.png

## Contract and explicit limitations

Matrix: 2x2 scaled/pivoted elimination; singularity is decided using
a tolerance, not by a symbolic rank proof. Geometry: exact integer-grid
signs are separately supported, and a new C++23 dyadic-rational oracle
returns the exact sign for all six finite IEEE-754 binary64 input values.
The older double filter can return UNCERTAIN, which must not be confused
with collinearity; its unproved bound is diagnostic and is never a substitute
for the new authoritative exact-sign result. Fit: centered ordinary
least squares, not an outlier-robust regression. PDE: 1D explicit heat
equation with fixed zero boundaries and r <= 1/2. Integration: seeded
Monte Carlo with Welford sample variance and analytic reference 2/pi;
identical seeds are not promised to give identical random doubles on all
standard-library implementations. A successful test does not certify
unexamined input ranges, physics, numerical conditioning, or performance.

The progression is vector/matrix -> predicates -> least squares ->
stable time stepping -> sampling/uncertainty -> pivoted dense QR ->
robust nonlinear inverse problems -> orthogonal geometry / RANSAC consensus.

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

## Native geometry experiment

`src/geometry.hpp` contains a normalized 2D implicit line model,
two-point hypotheses, fixed-trial RANSAC under an orthogonal distance
threshold, and orthogonal total-least-squares refitting of a
selected consensus. It recomputes the reported support after refinement
instead of pretending that support cannot change.

The native synthetic fixture has 80 points close to `y=2*x+1`
and 30 structured outliers close to `y=-7+0.4*x`. All jitter is
deterministic, the declared distance threshold is 0.04 in coordinate
units, the seed is 73013, and the trial budget is fixed at 400.
`ransac-summary` exposes true/false positives, precision, recall,
RMS, consensus coefficients and the all-point TLS baseline.
`ransac-data` exports all 110 labeled synthetic observations and
the computed selection flags. The figure renderer **only plots these
C++ rows**.

This native fixture deliberately differs from the earlier Python
paper-experiment dataset. Its successful consensus does not verify
RANSAC performance under intersecting structures, multiple models,
extreme coordinates, distribution shifts, or arbitrary thresholds.
`ideal_trials` derives a bound under independent, with-replacement
pair-sampling assumptions; the implemented finite-sample
fixed-trial procedure makes no such formal guarantee.

## Exact IEEE-754 binary64 geometric orientation

`src/exact_predicates.hpp` uses `std::bit_cast` to recover each finite
binary64 coordinate's exact signed integer mantissa and exponent. It
subtracts coordinates and multiplies determinant terms using variable-
length base-`2^32` integer limbs. The resulting sign is exact **for the
stored binary64 numbers**, even for subnormals, coordinate-difference
overflow, and almost collinear triples. It rejects NaNs/infinities and
requires a true IEEE-754 binary64 `double`.

Run `orientation-exact` to view six edge fixtures.
`orientation-audit` exports those plus 500 reproducible finite
bit-pattern triples as exact hexadecimal floating-point strings.
`predicate_oracle.py` independently parses each input as a Python
`Fraction` and compares all 506 exact signs. Python is the reference
checker here, **not** the implementation of the geometric predicate.
CTest adds two native commands and, when Python3 is available,
the independent rational-oracle check.

The standalone exact algorithm has been locally checked under GCC
and Clang and matched a separate Python rational oracle on 500 random
finite input triples before integration. The full build must pass CI
and the author's AppleClang regression checks as well.
This is a clear, correct teaching reference, not an optimized adaptive
expansion predicate or a production benchmarking result.

## Still not implemented

A general SVD, adaptive high-performance expansion predicates,
robust incircle/orient3d predicates, sparse matrices or iterative solvers, finite-element assembly in
this native workbench, 3D mesh/graphics clients, production-quality
nonlinear optimization (including globalization/gauge handling),
independent extended-precision numerical oracles, statistical
multiple-seed inference, or full PDF/EPUB visual acceptance.
No results from the Python-led research atlas are relabeled
as C++23 implementations until a native reimplementation exists.
