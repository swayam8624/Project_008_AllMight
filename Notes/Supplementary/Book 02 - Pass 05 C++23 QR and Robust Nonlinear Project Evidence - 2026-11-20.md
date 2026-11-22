# Book Two — Pass 05: Native QR and Robust Nonlinear Project

**Book:** From Equations to Geometry (Book Two of Seven)
**Branch:** \`curriculum/book02-mathematics-refinement-pass01\`
**Date:** 20 November 2026
**Status:** Source implementation and tests committed. Full author-environment PDF/EPUB publication approval is not implied.

## Governing promises

- The original sixteen manuscript chapter files remain byte-locked and unchanged.
- The original 36 additive Book Two TeX supplements remain in place; this pass **extends** three of them rather than replacing material.
- All new algorithmic steps are written in C++23. The Python plotting utility only reads native CSV output.
- Other books, especially Book One, are not edited; the working branch is the Book Two-only branch.
- The production PDF is **not** compiled here. Typography must still be checked on the author's Mac, including page breaks, embedded fonts, scientific figure placement, and print trim.
- The complete author-cache EPUB is not certified by the source-level integration proof.

## Implemented native project

The C++23 source is \`tools/labs/book02_cpp23/src/advanced.hpp\`, integrated through \`src/main.cpp\`.

1. Dense, full-column-rank least squares by Householder QR with greedy column pivoting. It rejects rank deficiency against an explicit relative tolerance, retains permutation order, computes residual RMS from the original system, and exposes an R-diagonal ratio **not** mislabeled as a certified condition number.
2. Nonlinear curve model \(a\exp(bx)+c\), with an analytical residual Jacobian, Huber loss, IRLS weights, adaptive Levenberg-style damping, and actual-objective acceptance. Its step uses QR of an augmented weighted system, never explicit \(J^TWJ\). It returns a convergence flag rather than asserting every run converges.
3. Declared synthetic experiment: 35 observations, generating coefficients \((2,-0.7,0.5)\), deterministic sinusoidal perturbations, sample-index-15 outlier of +3.5, identical initial guesses and two different objectives (Huber and ordinary LS).
4. Independent tests for known QR solutions, rank-deficient rejection, nonfinite-input rejection, Jacobian/finite-difference agreement, and the declared outlier-recovery fixture.
5. Native CSV exports and optional plots for QR coefficient sensitivity, robust-vs-ordinary fit diagnostics, per-observation predictions, and the original heat equation.

## Chapter insertion sites

- \`Assets/Book02/08-conditioning-case.tex\`: pivoted Householder derivation, rank tolerance, exactly specified four-row perturbation, forward error versus residual.
- \`Assets/Book02/24-robust-least-squares-paper-lab.tex\`: weighted/damped QR derivation, Huber influence, cost acceptance, synthetic controls and outcomes.
- \`Assets/Book02/35-cumulative-numerical-geometry-atlas.tex\`: native cumulative extension, source/plot separation, explicit failure modes.
- \`Assets/Book02/preamble.tex\` continues to provide semantic, breakable, low-saturation color boxes; this pass uses them rather than changing the series-wide style.

## Run from the repo root

\`\`\`bash
git switch curriculum/book02-mathematics-refinement-pass01
git pull --ff-only origin curriculum/book02-mathematics-refinement-pass01
cmake -S tools/labs/book02_cpp23 -B build/book02-cpp23 -DCMAKE_BUILD_TYPE=Release
cmake --build build/book02-cpp23 --parallel
ctest --test-dir build/book02-cpp23 --output-on-failure
build/book02-cpp23/book02_workbench qr-sensitivity > build/book02-cpp23/qr-sensitivity.csv
build/book02-cpp23/book02_workbench robust-fit
build/book02-cpp23/book02_workbench robust-data > build/book02-cpp23/robust-data.csv
python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/robust-data.csv --output build/book02-cpp23/robust-data.png
python3 tools/labs/book02_cpp23/plot.py build/book02-cpp23/qr-sensitivity.csv --output build/book02-cpp23/qr-sensitivity.png
\`\`\`

The plot commands require Matplotlib; CMake and the C++23 tests do not.
The figure code reads exactly the C++ export schemas and never silently re-solves the problem.

## Verification and limitations

A separate local C++23 harness compiled under both Clang and GCC with \`-O2 -Wall -Wextra -Wpedantic\`. In the declared experiment the Huber parameter Euclidean error was about 0.00922, while the ordinary squared-error fit was about 2.67. These are **single synthetic-fixture values**, not uncertainty estimates or universally valid algorithmic comparisons.

GitHub Actions \`book02-cpp23-native-and-editorial.yml\` is the source-of-truth independent CMake/CTest gate. The initial integrated code, later TeX additions, and the strengthened editorial gate had successful runs in this pass. For exact run IDs and later validation, use the branch Actions history.

A semantic EPUB integration failure revealed an existing local-variable shadowing bug in \`book02_epub_supplements.py\`: the loop's role-class \`name\` variable overwrote the source filename, making \`Path(name)\` receive \`None\`. This pass repairs it using \`role_class\`, preserving the \`name\` source key; EPUB acceptance depends on a subsequent green semantic EPUB run.

**Outstanding work:** 2D RANSAC and exact general binary64 predicates in native C++23; FEM operators, implicit heat/geodesics and sparse solvers; MIS with multiple independent native seeds; production nonlinear optimization, SVD and high-precision baselines; independently audited numerical plots; the complete Mac PDF/EPUB visual and KDP gates. This pass does **not** claim to have completed them.
