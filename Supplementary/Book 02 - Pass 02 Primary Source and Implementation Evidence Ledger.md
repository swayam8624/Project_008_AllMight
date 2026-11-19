# Book Two — Pass 02: Primary-source and implementation evidence ledger

**Volume:** *From Equations to Geometry* (From Bits to Research, Book Two)
**Purpose:** Distinguish what was reviewed, independently derived, implemented, tested, and what still requires replication or library acceptance.
**Version context:** 2026-11-19. Source status is explicitly bounded below; no claim of exhaustive reading of all PDFs, textbooks, code files, original experimental datasets or paper benchmark settings.

## 1. Verification ladder

| Level | Meaning | What it does *not* mean |
|---|---|---|
| S0 — Catalogued | Relevant paper/book/repo identified and scoped | Content inspected |
| S1 — Primary entry/documentation inspected | Primary author page, abstract, documentation, or specific source passage actually examined | Full paper/textbook source complete |
| S2 — Independent derivation written | Equations and conditions reconstructed in original explanations | Program correct or author benchmarks reproduced |
| S3 — Teaching implementation written | Original small reproducible algorithm under `tools/labs` | Equivalent to the paper's production or original source |
| S4 — Program checks passed | Exact current code version passed deterministic tests and numerical reference comparisons | Full theorem proof, all edge cases, performance generalization |
| S5 — Published-result reproduction | Original publication dataset, metric, baselines, hardware and figure reproduced with recorded evidence | Never claim without explicit corresponding evidence |

All outputs are committed to `curriculum/book02-mathematics-refinement-pass01`. Original Book Two chapter sources are preserved byte-identically to their pre-refinement source blobs.

## 2. Primary paper and numerical software mapping

| Source | Actually inspected | Derived/added in Book Two | Companion code | Current boundary |
|---|---|---|---|---|
| Jonathan R. Shewchuk, 1997, `https://www.cs.cmu.edu/~quake/robust.html` | Author's original predicate page and published-paper metadata; inspectable `predicates.c` lineage found | Determinant, sign vs approximate value, filtering and exact fallback explanation in `21-robust-predicates-research.tex` | `book02_papers_geometry.py`: filtered fast sign and exact rational fallback on stored binary64 inputs; naive comparison | **Not** Shewchuk floating-point expansion arithmetic, incircle/3D completeness or published performance figures |
| Fischler & Bolles, 1981, `https://doi.org/10.1145/358669.358692` | Original publication record and supporting method source | Minimal sample, 2D orthogonal residual, consensus/refit, idealized hypothesis count in `25-ransac-paper-lab.tex` | Same geometry module with deterministic synthetic dataset | **Not** the original paper's camera-location experiment; seed does not certify generalization |
| Crane, Weischedel & Wardetzky, 2013, `https://www.cs.cmu.edu/~kmcrane/Projects/HeatMethod/` | Author's project page; independent CGAL manual verifies 3-step mathematical structure | Mass + heat, normalized gradient, Poisson and gauge in `23-heat-method-paper-lab.tex` | `book02_papers_numeric.py`: planar P1 FEM, implicit heat, triangle fields and Poisson with Euclidean flat-domain oracle | **Not** published meshes, original accuracy-speed plots or full curved/nonmanifold domains |
| Sharp & Crane, 2020, `https://www.cs.cmu.edu/~kmcrane/Projects/NonmanifoldLaplace/index.html` | Author/publisher page and code repo `nmwsharp/nonmanifold-laplacian`; repo notes it differs from the paper experiment code | P1 cotangent/stiffness construction and explicit nonmanifold limitations in `22-cotangent-operators-research.tex` | Finite-element operator in numeric module plus earlier `cotan_energy_lab.py` | Tufted cover **not** reconstructed, despite the paper being discussed; no nonmanifold parity claim |
| Ceres Solver docs, `https://ceres-solver.readthedocs.io/latest/nnls_modeling.html`, `https://ceres-solver.readthedocs.io/latest/tutorial.html` | Modeling equations and source-documented robust curve fitting | Residual Jacobian, IRLS/LM-like update, Huber cost, acceptance logic in `24-robust-least-squares-paper-lab.tex` | Numeric module's independently written optimizer with SciPy reference | **Not** Ceres-internal C++ solver/trust-region behavior or bundle-adjustment parity |
| libigl documentation, `https://libigl.github.io/tutorial/`, source `https://github.com/libigl/libigl` | Triangle cotangent Laplacian, mass matrix and sign-convention tutorial | FEM stiffness and mass explanations, sign warning in `22` and `26` | Array operators built using SciPy sparse assembly; tests of energy, symmetry, row sums | No compiled libigl binary comparison; different sign/mass convention must be normalized |
| Veach & Guibas, 1995, `https://lightfield.stanford.edu/papers/combine/` | Original author's paper landing and method outline | Multi-sample balance-heuristic expectation derivation in `27-multiple-importance-sampling.tex` | `book02_papers_sampling.py` with two normalized PDFs and exact integral (e-1) | Not paper's light transport, sample-cost evaluation, original image baselines or power heuristic implementation |
| Eric Veach, 1997, `https://graphics.stanford.edu/papers/veach_thesis/` | Original dissertation index including Chapter 9 | Explicit simple MIS / probability-measure bridge, later rendering handoff | Same sampling lab | Chapter 9 PDF not independently proof-checked in entirety |
| NIST, Monte Carlo uncertainty, `https://www.nist.gov/publications/consistency-monte-carlo-uncertainty-analyses` | Research abstract and public methodological context | Clarification of independent replicated units, linked to Book Two's clustered inference witness | Earlier `research_inference_lab.py`, MIS independent-replication test | Not the full NIST uncertainty implementation or original data study |
| PCG, `https://www.pcg-random.org/paper.html` | Primary paper index | Cross-reference only; explain deterministic seed does not prove RNG statistical quality | Python `random.Random` is used, **not** a PCG implementation | PCG original algorithm and published tests are explicitly out of scope for this pass |

## 3. User-repository evidence: actual source, not README-only claims

The following **specific** repository files were opened through the GitHub connector on their respective default branches. Their retrieved blob SHA values are pinned here for honest evidence:

| Repository / file | Source blob SHA | What the inspected source establishes | Follow-up |
|---|---|---|---|
| KairoMath / `Decomposition.cppm` | `0213583db1ef016b936eda3b7e782925eb6a6ef7` | Implements norm helper with `std::hypot`, decomposition tolerance, and symmetry validation, plus decompositions. | Run source-level and compiled adversarial tiny/huge-scale tests; require exact-head build. |
| KairoMath / `SVD.cppm` | `e80a40b4114755495c95697f9bc588a19c88a627` | Includes Golub–Kahan bidiagonalization using Householder transformations; examined branch accumulates certain norms by squared sums. | Test large scale/overflow, near-rank deficiency and reference singular values; don't claim failure without execution. |
| KairoMath / `Optimization.cppm` | `3c605d19d979ef071fbdf8f4fd2dbc6ba8ebd6e1` | Exposes `OptimizationSettings`, `OptimizationResult` and constrained numerical-iteration interfaces. | Test gradient/Jacobian conventions and loss/stopping semantics against SciPy or Ceres on matched residuals. |
| KairoMath / `Probability.cppm` | `06412b01cddd7a99560c853903ff197fdf8cb65a` | Explicit `std::mt19937_64` generator and uniform distribution API; `Pdf` handles an inclusive boundary in piecewise definition. | Test endpoint/zero width conventions and deterministic seeding; endpoint inclusion is measure-zero for continuous density, not by itself a defect. |
| KairoMath / `TensorAutograd.cppm` | `5897969b0c3046a44fc255e90365c1a4b18a3452` | Graph nodes with gradient accumulation, shape validation and backward traversal. | Test gradients, graph reuse and zeroing on exact heads; do not assume all differentiable kernels passed. |
| KairoPhysicsMath / `Integrators.cppm` | `0815c1c6521847d2658099e5e4726854e3ae6fd7` | Contains explicit/semi-implicit Euler, Verlet and RK2/RK4 scalar helpers; validates some finite inputs and positive timesteps. | Independently test truncation/stability, derivative callback returning nonfinite values and conserved quantities; avoid claiming built/tested. |

Both Kairo repositories' `STATUS.md` files currently explicitly reject earlier percentage completion claims as `UNVERIFIED`. Their source files are therefore **case studies** rather than accepted numerical gold standards. Maveb and Vulkax are available as future experimental systems; this pass does not audit their latest exact-head campaign evidence.

## 4. What has actually been built

- `tools/labs/book02_papers_geometry.py`: exact sign of binary64 orientation with a conservative fast filter and rational fallback; 2D line fitting with deterministic consensus, adaptive-trial estimate and orthogonal refit.
- `tools/labs/book02_papers_numeric.py`: own finite-element stiffness and lumped-mass assembly; single-step implicit heat solve, normalized negative gradient and anchored Poisson distance; independent exponential least-squares fit with Huber weights and accepted LM-like trial steps.
- `tools/labs/book02_papers_sampling.py`: two normalized proposal PDFs, inverse CDF, equal-count MIS balance estimate, independent-replicate statistics and exact analytical integral.
- Three corresponding `test_book02_papers_*.py` suites: positive, invalid, degenerate, analytic, numerical-parity, variance, and repeatability checks.
- `tools/labs/book02_papers_cpp23.cpp`: independent standard-library C++23 first-stage orientation sign filter that refuses uncertain values, bounded-integer exact cross-checks, P1 triangle stiffness and heat-step energy/positivity witness, a screened tiny linear solve, and a stable large-magnitude norm test. This C++ source is a teaching companion, not a replacement for Shewchuk's full adaptive precision implementation.
- The research CI additionally compiles with `g++ -std=c++23 -O2 -Wall -Wextra -Werror -pedantic` and executes that binary; acceptance requires an actual green run.
- Seven paper-derived LaTeX teaching sections (`21-...` through `27-...`), added at original chapter boundaries through the working-copy injection; all original stage source chapter files remain untouched.

## 5. Acceptance gates and nonclaims

The CI workflow `.github/workflows/book02-research-pass02.yml` pins Python 3.12, NumPy 2.2.6 and SciPy 1.15.3, then runs all research-program tests in normal and optimized Python. Record the exact successful run URL and commit hash only after it completes; earlier failed runs are **not** acceptance. Mathematical figure placement and full chapter preservation continue to be guarded by the separate Pass 1 workflow.

A verified result means the declared **synthetic** cases passed; no wall-clock speedups were fabricated. A source paper named in this ledger has **not** necessarily been read cover to cover. A repo containing a method is **not** proof it is correct. A numerically close fit is not proof of physical validity, and a single seed is not sufficient to infer general stochastic performance.

## 6. Remaining verification and handoff

Pass 3: visual research atlas, original 3D/2D mathematical figures generated from exported numerical arrays, error/condition/accuracy dashboards, scene presets, and chapter-integrated projects. Performance figures require measured data and full provenance.

Pass 4: execute exact-head Kairo module acceptance if available, inspect complete mathematical text and original paper limits, port supplemental content and figure semantics to the reflowable EPUB source path, verify PDF glyphs/page breaks and captions, and perform final publication and exercise review.

No source text is deleted or rewritten merely because its content overlaps a paper-derived supplement.
