# Book 02 — From Equations to Geometry
## Editorial architecture, source audit, and research implementation plan
**Series:** From Bits to Research · Book Two of Seven
**Subtitle:** Mathematics, Numerical Methods, Optimization, and Visual Computing
**Audit started:** 2026-11-19
**Status:** analysis and editorial blueprint; **not** a claim that source-level refinement or full verification is complete.

> This is an additive planning and evidence file. Do not delete substantial source teaching. Canonical masters and their 16 assigned production chapters retain ownership; generated LaTeX is an output, not an authoring source. The Book One development branch is retained as the base of this branch.

## 1. Repository material inspected and important boundaries

The stage catalogue (`Book Production/AllMight Codex/LaTeX/Tools/stage-catalog.json`) assigns **16 source chapter IDs** to Stage 02, in two physical print parts in the active `CURRENT.json`. The chapter content in `Generations/.generation-n3azv4es/02-Mathematics-for-Visual-Computing-P0{1,2}/content` was inspected for title, chapter-level structure, subheadings, and references. That inspection is a mapping and risk assessment, not a mathematically independent line-by-line verification of every proof or program.

The three continuous masters, 129 chapter reading edition, Concepts notes, Supplementary audit ledgers and executable laboratory index coexist with the later seven-stage LaTeX division. Preserve link anchors and the master-first editing rule. In particular, the `Chapter Edition` generated reading copies and LaTeX `Generations` must not be edited in place.

The Nov 15 `Supplementary/Three-Book Content Completion.md` audit says 1,820 parts have matched passage assessments, only 4 parts final-accepted at its recorded checkpoint, and source-ledger review is not equivalent to full textbook/video coverage. The Nov 11 mathematics expansion says seven mathematical texts were selectively examined (some precise PDF ranges), not exhaustively read. We therefore must never advertise full mathematics-source closure on this basis.

## 2. Exact Book 02 allocated chapter inventory (verified at source)

| Source ID | Existing chapter title | Editorial action |
|---|---|---|
| 01-foundations-i-ch017 | Python and Numerical Tools for Graphics Systems | Preserve numerical toolchain material; introduce basic mathematical objects and experimental harness before implementation-heavy discussion where needed. |
| 01-foundations-i-ch018 | Vectors for Light and Frames | Preserve optics applications; develop vector/affine fundamentals before reflection/refraction, or locally teach each prerequisite. |
| 01-foundations-i-ch019 | Coordinate Frames and Linear Systems | Keep coordinate frames, quaternions, transforms, pivoted LU; add convention consistency and invariant-oriented diagram exercises. |
| 01-foundations-i-ch020 | Choose Matrix Factorizations from Structure | Deepen rank, nullspaces, conditioning, QR/Cholesky/LDLT/SVD choice and measurable backward error; avoid shortcut introductions to calculus. |
| 02-foundations-ii-ch021 | Multivariable Derivatives and Sensitivity | Use vector-valued differentials, covectors, Jacobians and Hessians; add AD comparisons and finite difference failure studies. |
| 02-foundations-ii-ch022 | Accumulation, Quadrature, and the Bridge to Probability | Separate integration, quadrature, autodiff and probability pedagogically; maintain explicit transitions and practical tests. |
| 02-foundations-ii-ch023 | Probability Models and Measurement Error | Preserve thorough distributions; add calibrated measurement models, dependence, likelihood, and model-misspecification exercises. |
| 02-foundations-ii-ch024 | Waiting Times, Covariance, and Matrix Decomposition | Tie exponential/Poisson to event processes; connect covariance geometry to SVD/PCA and rank-deficient observations. |
| 02-foundations-ii-ch025 | Continuous Models and Numerical Error | Clarify ODE vs PDE, boundary conditions, stability domains, mesh/grid resolution, implicit/explicit integrators, conservation. |
| 02-foundations-ii-ch026 | Matrix Functions and Nonlinear Least Squares | Connect matrix exponentials to time evolution and SO(3); develop convexity, duality, constrained optimization, objective choice. |
| 02-foundations-ii-ch027 | Gauss--Newton for Geometry and Pose | Derive robust nonlinear least squares, Schur complement, Jacobian blocks, manifold pose updates and identifiable gauges. |
| 02-foundations-ii-ch028 | Reliable Optimization and Root Finding | Retain substantial workbench; add global-vs-local convergence distinctions, diagnostics, adversarial numerical tests. |
| 02-foundations-ii-ch029 | Discretization, Convergence, and Experimental Design | Separate discretization/truncation/roundoff/model error; add convergence plots, uncertainty bands and reproducible test design. |
| 02-foundations-ii-ch030 | Monte Carlo Estimates and Uncertainty Metrics | Retain variance, error and inference; distinguish confidence intervals, biased estimators, stratified and QMC assumptions. |
| 02-foundations-ii-ch031 | Implement Probability with Numerical Contracts | Develop documented C++23 kernels with positive/negative/metamorphic tests and reference NumPy/SciPy parity. |
| 03-visual-computing-i-ch012 | Probability, Statistics and Uncertainty | Identify and reconcile overlap with chapters 23/29/30/31; retain unique sampling/rasterization bridge, avoid duplicated introductory narrative. |

**Risk flags supported by direct chapter reading:** ch017 begins with advanced Python/performance abstractions; ch018 begins with reflection/refraction; ch027 has substantial subsection-level teaching but no top-level `\\section` divisions; ch012 contains rasterization topics that also belong to Book Three. In generated-stage text, a number of reference links point to transient image hosts, favicon endpoints, or use tracking query strings. All require source-aware verification/asset normalization; no link deletion without preserving the graphic or intellectual content.

## 3. Proposed teaching narrative — 24 curriculum chapters / encounters

The 24 encounters are an editorial roadmap, **not** an instruction to replace 16 source IDs or to shrink them. Long existing chapters can contain multiple encounters. Preserve original headings as navigable anchors where possible.

### Part I: Mathematical language and computational measurement
1. Numbers, units, functions, precision, domains, coordinates, and why numerical work needs a contract.
2. Building a reproducible NumPy/SciPy and C++23 mathematical laboratory; arrays, broadcasting, shape/layout, memory, random streams and plotting.
3. Vectors, norms, dot/cross, projection, orientation, affine points, dual vectors and metric-dependent gradients.
4. Coordinate frames, bases, Gram matrices, reciprocal bases, covariance under coordinate changes.

### Part II: From vectors to reliable linear algebra
5. Linear maps, matrices, homogeneous coordinates, transform composition/inversion, rigid motions, quaternions, Lie groups as a preview.
6. Linear systems, rank/nullspace, conditioning and residuals; LU/pivoting, QR, Cholesky, LDLT.
7. Spectral geometry: eigenproblems, PCA, SVD, polar decomposition, low-rank approximation, uncertainty ellipsoids.
8. Sparse matrix storage, graph Laplacians, conjugate gradients, preconditioning and memory/time/accuracy tradeoffs.

### Part III: Calculus, sensitivity, optimization
9. Limits, one-variable derivatives, Taylor approximation, numerical differentiation and integration/quadrature.
10. Multivariable derivatives, Jacobians, Hessians, directional/total derivatives, the chain rule and inverse function sensitivity.
11. Forward/reverse automatic differentiation and finite-difference validation, including invalid/non-differentiable branches.
12. Convex sets/functions, constraints, Lagrange multipliers, KKT, duality, gradient methods and line searches.
13. Least squares, weighted/robust objectives, Gauss--Newton, Levenberg--Marquardt, parameterization, gauge freedoms and sensitivity.

### Part IV: Probability, sampling, inference and trust
14. Probability spaces, independence, random variables, distributions, conditioning, covariance, Bayes and stochastic processes.
15. Sampling algorithms, pseudorandom and quasi-random sequences, Monte Carlo integration, importance sampling, stratification and variance reduction.
16. Statistical estimation, confidence/credible intervals, uncertainty propagation, calibration, effect sizes, dependent observations and experimental design.

### Part V: From continuous equations to discrete worlds
17. Differential equations, existence/uniqueness assumptions, explicit/implicit/symplectic time integration and error control.
18. PDEs, heat/wave equations, boundary conditions, finite differences, stiffness and stability, Fourier modes and sampling.
19. Curves, splines, parametric surfaces, differential geometry, curvature and local/global geometric measurements.
20. Mesh topology, chains/boundaries, discrete gradients, cotangent Laplacians, mass matrices, FEM/DEC connections and diffusion on surfaces.
21. Robust geometric predicates, interval/error filtering, degeneracy, tangent/normal transport and geometric optimization.

### Part VI: Executable mathematical research
22. Numerical verification: exact/reference oracles, forward/backward error, convergence orders, metamorphic tests and reproducible benchmark methodology.
23. Scientific visualization: image/3D fields, uncertainty ellipsoids, vector fields, scalar mesh heatmaps, convergence/error plots, sensitivity surfaces and accurate visual encodings.
24. Capstone: an interactive geometry-and-numerics laboratory built over the preceding chapters, with a documented research replication and fully reproducible figures.

**Book boundary:** Full camera formation, rasterization and projection systems belong principally to Book Three; full light transport to Four; physics engine/constraint solving to Five; production GPU optimization to Six; neural rendering, XR research and scientific paper authorship to Seven. Book Two must teach enough of each mathematical prerequisite to support its applications without moving all future application chapters back into the mathematics volume.

## 4. Existing source books: confirmed presence versus review status

The Nov 11 mathematics audit records selected direct readings in exactly these supplied books:
- Riley, Hobson & Bence, *Mathematical Methods for Physics and Engineering* — reciprocal coordinates and inner products.
- Boyd & Vandenberghe, *Convex Optimization* — dual norms and convex projection.
- Wasserman, *All of Statistics* — expectation, covariance and dependence.
- Botsch et al., *Polygon Mesh Processing* — normals, triangle gradients and cotangent operators.
- Edelsbrunner & Harer, *Computational Topology* — chains, cycles and boundaries.
- Arnold, *Mathematical Methods of Classical Mechanics* — coordinate-dependent orientation and mechanics.
- Lattimore & Szepesvári, *Bandit Algorithms* — feature-action geometry and uncertainty-based decisions.

They are **source candidates for wider chapter-aligned verification**, not seven books already examined page by page. Integrate full original explanations in our own words with precise page/section references where directly validated. Add Higham's *Accuracy and Stability of Numerical Algorithms* and CMU/Stanford/MIT primary numerical teaching as targeted additional reference candidates.

## 5. Research paper reading and reproduction ladder

Use primary author/publisher sources, and classify each stage separately as linked → actually read → derivation reproduced → independent implementation → numerical experiments reproduced → research limitations discussed.

| Paper or primary work | Teaching purpose | Required reproduction |
|---|---|---|
| Shewchuk, *Robust Adaptive Floating-Point Geometric Predicates* (1996), and extended 1997 paper | Floating-point error filters, sign certification | orientation2d/orientation3d robust versus naive near-collinearity sweep and correctness oracle |
| Meyer et al., *Discrete Differential-Geometry Operators for Triangulated 2-Manifolds* (2003) | Geometry-to-operator derivation | Laplacian and mixed-area/gradient reconstruction on analytical meshes |
| Sharp & Crane, *A Laplacian for Nonmanifold Triangle Meshes* (2020) | Domain assumptions, nonmanifold geometry, nonnegative weights and boundary behavior | test meshes with nonmanifold and boundary cases; distinguish positive energy from maximum principle |
| Crane, Weischedel & Wardetzky, *Geodesics in Heat* (2013) | PDE, sparse solves, differential geometry | compare heat-method distances with reference geodesic distances and convergence/error metrics |
| Fischler & Bolles, *Random Sample Consensus* (1981) | Uncertainty, outlier-robust model fitting | fit known lines/planes with injected outliers; show success-rate sensitivity and failure cases |
| Foundational Gauss–Newton and Levenberg–Marquardt lineage | Numerical optimization rather than black-box API invocation | fit a noisy geometric model with analytic/finite-difference/AD Jacobians, trust-region ablation |
| Veach, *Robust Monte Carlo Methods for Light Transport Simulation* (1997) | Sampling theory/MIS as a later rendering bridge | Book Two introduces unbiasedness, importance weights and variance; Book Four performs full light-transport implementation |

Papers not yet read in full must not be labeled reproduced; prior manuscript citations or a working link are not verification.

Primary routes:
- https://www.cs.cmu.edu/~quake/robust.html
- https://www.cs.cmu.edu/~kmcrane/Projects/NonmanifoldLaplace/index.html
- https://www.cs.cmu.edu/~kmcrane/Projects/HeatMethod/
- https://www.cs.cmu.edu/~kmcrane/Projects/DDG/
- https://graphics.stanford.edu/papers/veach_thesis/
- https://web.stanford.edu/~boyd/cvxbook/
- https://nhigham.com/accuracy-and-stability-of-numerical-algorithms/
- https://graphics.stanford.edu/courses/cs205a/
- https://ocw.mit.edu/courses/18-02sc-multivariable-calculus-fall-2010/

## 6. Reference implementations and repository-grounded case studies

| Repository/documentation | Used for | Verification boundary |
|---|---|---|
| https://libeigen.gitlab.io/ | C++ dense matrices and decompositions | Reference result/parity tests; do not blindly equate with custom implementations. |
| https://github.com/DrTimothyAldenDavis/SuiteSparse | Sparse factorizations and CHOLMOD | Read solver assumptions and memory cost; do not assume faster in every regime. |
| https://ceres-solver.readthedocs.io/latest/ | Nonlinear least squares, robust loss and AD | Reproduce residual formulation, Jacobian, initialization and termination logic. |
| https://numpy.org/doc/stable/ and https://docs.scipy.org/doc/scipy/ | Independent numerical baselines | Pin tested versions, tolerances and array conventions in reproductions. |
| https://docs.jax.dev/en/latest/ | Automatic differentiation | Compare analytic, finite-difference, and AD on smooth/degenerate examples. |
| https://libigl.github.io/tutorial/ | Cotangent Laplacians and mesh processing | Confirm sign, area/mass normalization, manifold/boundary behavior. |
| https://geometry-central.net/ | Mesh operators, geodesics and intrinsic geometry | Inspect input-domain restrictions and method implementation. |
| https://polyscope.run/ | Interactive 3D geometry debugging/visualization | Figures must disclose color scale, units, sample population and limitations. |
| https://github.com/swayam8624/KairoMath | Custom vectors, transforms, matrix decomposition, optimization, probability, autodiff | Repository STATUS says completion unverified. Pin code/head and execute exact build/tests; distinguish code present from correct. |
| https://github.com/swayam8624/KairoPhysicsMath | SI units, integrators, inertia, constraints and differential equations | Repository STATUS says completion unverified; treat as inspectable lab source, not a proven solver. |
| https://github.com/swayam8624/Maveb | Bounding errors and uncertainty under local geometric reconstruction | Reuse carefully bounded mathematical examples, avoid claiming unpublished results universally. |
| https://github.com/swayam8624/Vulkax | Experimental design, measurement models, uncertainty and negative controls | Respect frozen scientific snapshot and distinguish successful and failed external validation. |

No external repository should be copied wholesale into prose. Read its source, explain the invariant, reproduce an independently authored minimal version, test against the library, and cite implementation/version accurately.

## 7. Cumulative project ladder: projects must be beautiful AND mathematically truthful

**Project A — The Mathematics Workbench.** C++23 reference kernels plus Python NumPy/SciPy verification: stable norm, dot, basis change, matrix inverse, LU/QR/SVD, residual plots and conditioning heatmaps. Include floating-point adversaries and range/NaN controls.

**Project B — Transform and Uncertainty Explorer.** Interactive 3D vector/frame/rotation controls; transformations, normal transport, PCA ellipsoids, covariance propagation and gauge ambiguity. Draw annotated coordinate axes and error vectors; verify forward/inverse consistency.

**Project C — The Fitting Laboratory.** Fit 2D and 3D noisy observations using analytical versus AD Jacobians, RANSAC, Gauss–Newton and LM. Surface parameter trajectories, convergence curves, residual distributions, condition numbers, outlier contamination and degeneracies.

**Project D — Randomness and Reliable Inference.** Monte Carlo integration, stratified/QMC samplers, confidence intervals and sequential pitfalls; show replicate distributions, coverage checks, variance-versus-budget curves and seeded repeatability.

**Project E — From Differential Equations to Surface Fields.** A rod, 2D grid and triangular mesh heat solver; explicit/implicit steps, sparse solves, boundary conditions, analytic reference where possible, timestep stability maps, error heatmaps, geodesic approximation and geometry coloring. Show a continuous animated scientific scene rather than only toy balls.

**Project F — Interactive Numerical Geometry Atlas.** One cohesive application combining A–E: 3D model/field viewer, scalar and vector heatmaps, error overlays, dashboards, benchmark tables, solver traces, reproducible scene presets, batch exports, reference citations and a reproducible miniature-paper-style investigation.

Each project needs source code, tests and saved figures in addition to prose. Any recorded performance number needs hardware/compiler/compiler flags/version/seed/workload/repetitions and uncertainty or variability treatment; never invent benchmark results.

## 8. Acceptance and planned refinement passes

A teaching unit is accepted only when it has (1) prerequisite bridge and precise conventions; (2) assumptions, derivation, and proof or justified approximation; (3) worked numbers/intermediate states; (4) commented C++23 and/or Python implementation and ownership/data layout; (5) adversarial counterexamples, degeneracies and alternatives; (6) exact, relative and stability-related error measurement; (7) reproducible positive/negative/metamorphic tests; (8) visualization that represents the mathematics accurately; (9) sources with evidence status; (10) exercises with solutions and a working project checkpoint.

Suggested pass allocation, strictly additive unless evidence proves an error: 
- P01 inventory, source custody, narrative map (this file).
- P02 scalar/affine/linear algebra foundations, reciprocal bases and matrix choice.
- P03 differential calculus, AD, numerical differentiation, quadrature.
- P04 probability, statistics, uncertainty and reproducible sampling.
- P05 nonlinear/convex optimization and geometric fitting.
- P06 ODE/PDE, discretization, Fourier modes, error/stability.
- P07 differential/discrete geometry, sparse operators, topology, robust predicates.
- P08 source-by-source original papers and library implementation parity.
- P09 integrated mathematical project code, 3D visualization and experiment dashboards.
- P10 source/citation cleanup, link/asset audit, exercise solutions, mathematical verification, LaTeX-readiness **without compiling on the author's behalf**.

**Immediate next source edit:** select original canonical math master passage behind ch017–ch020; write a clear zero-prerequisite bridge from scalars/units to vectors and linear transformations, preserving the Python discussion and existing computational optics examples. Add a self-contained documented lab for basis change, precision and stable linear solves, and tests before assigning any chapter an acceptance status.

**Not done here:** no original master narrative has been changed, no tests or PDF have been compiled, no source paper has been reproduced solely by creating this planning document.


## First visual-atlas integration (2026-11-19)

Five original, equation-driven figure plates have been authored at `Book Production/AllMight Codex/LaTeX/Assets/Book02/`. The new `Tools/book02_visuals.py` injects them non-destructively into temporary Book Two TeX copies during both logical-stage proof and physical-book composition via `Tools/build.py`. The original Markdown continuous masters, seven-stage catalogue, generated source files, and source chapter bodies are unchanged. Other books are not modified by the injector.

| Existing chapter anchor | Original visual plate | Mathematical provenance |
|---|---|---|
| 01-foundations-i-ch019 | Linear transformation of a unit circle | Explicit 2x2 matrix; basis images and closed-form ellipse parameterization |
| 02-foundations-ii-ch024 | Gaussian density surface and 2D heatmap | Properly normalized independent bivariate normal with declared covariance |
| 02-foundations-ii-ch025 | Heat-equation analytic surface and map | Exact single eigenmode on unit square at t=0.04 under zero Dirichlet conditions |
| 02-foundations-ii-ch027 | 3D optimization landscape and descent trajectory | Explicit strictly convex quadratic and checked iterates with alpha=0.1 |
| 02-foundations-ii-ch029 | Numerical convergence log-log plot | Closed-form forward-Euler error for y'=-y at T=1, with first-order reference |

The figures are vector PGFPlots graphics generated from their stated functions, **not** made-up benchmark outputs, photorealistic pictures or empirical result visualizations. Captions explicitly distinguish analytic values and simulated measurement. Every figure includes a first reconstruction exercise. Their color palette follows a navy/cyan/gold technical identity.

A standalone LuaLaTeX smoke test of the heat-equation analytic plot and PGFPlots preamble was executed successfully in the assistant container; the standalone PDF rendered and was inspected. This does **not** assert that the entire user-side Book Two PDF or all five plates have compiled successfully. The dedicated `test_book02_visuals.py` checks idempotence, chapter placement and other-book isolation, but its repository-side execution still requires verification. As part of the remaining final pass, compile both physical books on the author's macOS TeX runtime, visually inspect each printed plate at actual trim size and port semantic illustrations to EPUB.

**Publication distinction:** Figures inserted after a source chapter are reader-facing supplementary visual laboratories. They are not silently claimed to have come from or to have been reviewed by the original chapter author/source. The source chapter's complete text remains intact.

## Pass 1 continuation — additive mathematical proof and failure cases (2026-11-19)

**Editorial decision enforced:** Preserve all 16 existing stage source chapters, including their originally generated LaTeX inputs. No original continuous master or generated source fragment is rewritten. New sections live under `LaTeX/Assets/Book02/` and are inserted only into temporary LaTeX build copies by `Tools/book02_visuals.py`. Stage-P01 retains its original eight chapter inputs; P02 retains eight; the complete-stage proof retains all sixteen. This was cross-checked against the actual `main.tex` references on the working branch.

**Six new, self-contained instructional pieces** were added, each explaining a mathematical claim, its assumptions, a concrete numerical example or witness, a known failure mode, and an executable reconstruction exercise:

- `06-reader-contract.tex` — coordinates, units, affine points versus displacements, modeling versus floating-point representation, observable/algorithmic error and falsifiable numerical contracts. Positioned *before* original ch017.
- `07-metric-gradient-case.tex` — local differential coefficients versus metric-gradient vectors, derived using an explicit slanted basis and Gram matrix. Positioned after ch019, following its original text and visual plate.
- `08-conditioning-case.tex` — analytic 2x2 ill-conditioning, derivation of squared condition number under normal equations, and an observation perturbation that alters fitted coefficients while residuals stay near zero. Positioned after ch020.
- `09-heat-stability-case.tex` — two-dimensional FTCS diffusion update, unit analysis, Fourier-mode amplification and carefully scoped CFL condition for periodic/infinite grids. Positioned after ch025.
- `10-gauge-nullspace-case.tex` — gauge invariance and Jacobian null direction in a relative-position least-squares problem; shows why fixing a gauge does not add physical evidence. Positioned after ch027.
- `11-sampling-support-case.tex` — explicit two-state counterexample proving importance weighting cannot repair a proposal distribution that fails the target-support condition. Positioned after ch031.

**Test sources:** `tools/labs/book02_numerical_contracts.py` independently recomputes the analytic quantities underlying all five original scientific plates; it also checks a slanted-basis metric-gradient witness, normal-equation condition numbers, a perturbed linear solve, a numerical finite-difference heat-equation residual, rate estimates and explicit rejection of invalid inputs. `LaTeX/Tools/test_book02_visuals.py` was expanded for the new injections, original-chapter preservation and non-Book2 isolation.

**Actual verification achieved:** A separate local standalone mathematical oracle of the same examples was executed in this assistant environment in normal and `-O` Python modes, with passing numerical results. The exact newly committed `book02_numerical_contracts.py` and updated `test_book02_visuals.py` have **not yet been executed in a checkout of this branch**, because GitHub source was edited through a connected repository rather than a locally cloneable checkout. LaTeX TeX/PGF file brace balance was checked for all 12 Book02 assets, and the 16 chapter-source inputs in the physical and full-stage documents were checked for presence/uniqueness. These are structural checks, **not** a complete PDF proof or source-correctness certificate.

**Next steps:** Execute both committed Python suites in the author's checkout; build the two physical Book02 parts on the Mac; visually review all plates for actual page and EPUB behavior; continue remaining chapter-level mathematical checks and paper-level reproductions. Add new visual assets inside the teaching chapter or as verified chapter-end laboratories; never use unsourced stock plot aesthetics or invented performance measurements as research figures. Ensure figures and mathematical teaching reach the semantic EPUB export, not just the print PDF.

**Pass status:** meaningful additive progress; full Pass 1 not yet certified complete because the uninspected portions of all sixteen source chapters and unexecuted repo-level verification remain open.
