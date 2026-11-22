# Book Two — Pass 01 Completion and Verification
**Series:** From Bits to Research
**Book:** From Equations to Geometry — Mathematics, Numerical Methods, Optimization, and Visual Computing
**Pass:** 1 of 4 — mathematical foundations, editorial continuity and preservation
**Date:** 2026-11-19
**Decision:** COMPLETE FOR THE DEFINED PASS-1 SCOPE. Not a declaration that the book, its original corpus, every source paper or the final PDF/EPUB is fully verified.

## 1. Scope and evidence rule

This is an **additive editorial pass** over the actual existing 16 source chapters allocated in `LaTeX/Tools/stage-catalog.json`, not a wholesale rewrite. All original generated chapter source files remain verbatim. The two physical print books continue to include their 8+8 original chapter sources; the full stage proof includes all sixteen. The added material is placed at specific source chapter boundaries by `LaTeX/Tools/book02_visuals.py`, which changes only temporary working `main.tex` copies at build time.

The initial structural inspection read the actual source chapter bodies, headings, examples, references and selected detailed mathematical passages. Direct inspection covered all sixteen; mathematical spot checks included frame/vector operations, inverse conditioning, optical refraction, projection Jacobians, differential calculus, automatic differentiation, probability conditioning, matrix functions, optimization/root certification, numerical heat equations, Monte Carlo estimators and dependence-sensitive inference.

A selected-passage editorial audit is **not** an exhaustive independent proof audit of all ~1.7 MB of original mathematics. Whole-corpus mathematical acceptance remains a final publication requirement in Pass 4.

## 2. Original content preservation: checked against baseline branch

Compared each physical part's generated `content/` directory in `curriculum/book02-mathematics-refinement-pass01` with `curriculum/cpp23-nes-project-pass01` using GitHub content-blob hashes:

| Source distribution | Assigned source chapters | Identical blob hashes |
|---|---:|---:|
| Book 2 Part 1 | 8 | 8 |
| Book 2 Part 2 | 8 | 8 |
| **Total** | **16** | **16** |

No original chapter was deleted, replaced, reduced, or silently renumbered by this pass. Changes to the larger continuous Markdown masters were deliberately avoided; the canonical next authoring step must consider the vault master-first rule and how to distribute the new teaching into non-LaTeX reading formats.

## 3. Chapter-by-chapter Pass 1 teaching map

Each line below names the original chapter ID retained in the production book and the new, independently checkable supplement added adjacent to it. These are mathematical extensions and verification exercises, **not** an assertion that the original chapter lacks the named subject.

| Original source chapter | Independent addition(s) | Specific verification objective |
|---|---|---|
| `01-foundations-i-ch017` — Python/Numerical Tools | `06-reader-contract.tex` before chapter; `12-array-contract-case.tex` after | Interpret arrays as measured quantities with coordinate/shape/type contracts; translation-invariance witness |
| `01-foundations-i-ch018` — Vectors for Light and Frames | `13-refraction-contract-case.tex` | Correctly distinguish actual total internal reflection from roundoff/invalid refraction |
| `01-foundations-i-ch019` — Coordinate Frames and Linear Systems | `01-linear-transformation.tex`, `07-metric-gradient-case.tex` | Show exact basis images; distinguish differential covector and metric gradient |
| `01-foundations-i-ch020` — Matrix Factorizations | `08-conditioning-case.tex` | Compute condition numbers; explain squared conditioning and small-residual ambiguity |
| `02-foundations-ii-ch021` — Multivariable Derivatives and Sensitivity | `14-projection-jacobian-case.tex` | Derive pixel-domain camera Jacobian with units and first-order remainder |
| `02-foundations-ii-ch022` — Integration and Quadrature | `15-differentiation-quadrature-case.tex` | Compare analytic/AD/finite-difference directional derivatives; verify numerical integration rules |
| `02-foundations-ii-ch023` — Probability Models | `16-bayes-base-rate-case.tex` | Separate likelihood, prevalence, and posterior probability |
| `02-foundations-ii-ch024` — Waiting Times, Covariance and Decomposition | `02-gaussian-uncertainty.tex` | Evaluate normalized 2D Gaussian density and geometry of covariance |
| `02-foundations-ii-ch025` — Continuous Models | `04-heat-diffusion.tex`, `09-heat-stability-case.tex` | Compare exact analytic diffusion field with discrete FTCS mode-stability bound |
| `02-foundations-ii-ch026` — Matrix Functions and Nonlinear Least Squares | `17-matrix-exponential-case.tex` | Exponentiate a skew generator; distinguish rotation invariance from Euler discretization |
| `02-foundations-ii-ch027` — Gauss–Newton for Geometry and Pose | `03-optimization-landscape.tex`, `10-gauge-nullspace-case.tex` | Recompute exact descent steps; expose an unobservable gauge direction |
| `02-foundations-ii-ch028` — Reliable Optimization and Root Finding | `18-bracketed-root-case.tex` | Provide a ten-step bisection bracket/error certificate versus unsafe Newton start |
| `02-foundations-ii-ch029` — Discretization and Experimental Design | `05-convergence-study.tex` | Validate exact forward-Euler error curve and first-order refinement slope |
| `02-foundations-ii-ch030` — Monte Carlo Estimates and Uncertainty | `19-clustered-inference-case.tex` | Show that repeated correlated frames cannot be counted as independent scenes |
| `02-foundations-ii-ch031` — Probability Implementation Contracts | `11-sampling-support-case.tex` | Prove that absent proposal support cannot be repaired by reweighting |
| `03-visual-computing-i-ch012` — Probability, Statistics and Uncertainty | `20-sampling-measure-case.tex` | Convert area/PDF and solid-angle/PDF with the correct Jacobian and physical units |

The five source-generated scientific figures are exact mathematical illustrations (linear transform, Gaussian density, optimization objective, heat eigenmode, Euler convergence), **not invented empirical observations or benchmark numbers**.

## 4. Source and code verification

### Automated branch-level evidence
`tools/labs/book02_numerical_contracts.py` checks the original five figure formulas and the added independent chapter-specific mathematical examples. It uses explicit runtime exceptions rather than `assert` to remain meaningful under optimized Python.

`Book Production/AllMight Codex/LaTeX/Tools/test_book02_visuals.py` checks:
- Exact chapter-ID coverage against the actual `stage-catalog.json`.
- Preservation of the generated P01/P02 chapter-input order from the current production generation.
- Every original `\\input` survives injection in full and exactly once.
- All scientific assets exist, are copied under the temporary build, and are inserted adjacent to their intended chapter.
- The reader prelude precedes the first original chapter.
- Additions are idempotent and never applied to another book.
- Invalid or ambiguous input sequences reject explicitly.

**GitHub Actions successful run:** [Book Two — Mathematical Pass One Contracts, run 44581128237](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44581128237), tested at commit `2e53d1bf3d8b5768b3fb859d211c1422d6112045`. The recorded job completed **successfully** for Python syntax checks, numerical test in normal mode and with `-O`, full 16-source integration/unittest checks in normal and optimized modes.

A separate local numerical cross-check `/mnt/data/book02_numeric_contracts.py` also passed in normal and `-O` Python in the assistant environment. That separate file is not asserted to be byte-identical to the committed GitHub test, whose exact-version green result is provided by Actions.

### What these results do not establish
A green numerical witness does not show that every previous theorem, multi-page derivation, every source book, the complete generated chapter or an external paper is correct. Working-copy injection tests do not prove final output's type size, page breaks, accessible captions, EPUB semantics, embedded fonts, external asset licensing or GPU fidelity. A deterministic analytical test is not an experimental speedup result.

## 5. Explicit Pass 1 acceptance decision

| Gate | Decision | Basis |
|---|---|---|
| Existing chapter inventory and thematic inspection | Pass | 16 actual source documents inspected and individually mapped |
| Preservation/no unnecessary rewriting | Pass | 8/8 + 8/8 GitHub source blobs identical |
| Independent mathematical explanation at every chapter boundary | Pass | 16 chapter-owned addenda, with multiple cases where needed |
| Original scientific figures source integrated for print build | Pass at source level | Five PGFPlots source plates with declared equations, captions and reconstruction exercises; hooked into stage and print copy preparation |
| Mathematical/placement program checks | Pass for tested witnesses | Exact branch CI run 44581128237 successful |
| Full book peer review, compiled print layout and EPUB parity | **Not in Pass 1 scope** | Must be demonstrated during subsequent passes; not silently waived |

## 6. Boundaries for Passes 2–4

**Pass 2 — Sources, papers and implementation evidence.** Read and reproduce exact primary methods in scope (Shewchuk, discrete differential geometry, heat method, nonlinear solvers, reproducible sampling). Examine exact versions of Eigen, Ceres, libigl, KairoMath, KairoPhysicsMath and relevant Maveb/Vulkax modules; run parity and adversarial experiments. Do not present older repository README claims as independently verified performance or correctness.

**Pass 3 — Projects and visual demonstrations.** Turn each source-generated analytic plot into integrated reader-friendly exposition with original vector/mesh heatmaps, parameter-sweep dashboards, interactive numerical cases and source code. Keep all figures mathematically grounded and do not invent measurements. Add alternative visuals when an existing illustration is too small, abstract, decorative, or unexplained.

**Pass 4 — Book-level verification and publication.** Review each source lesson's proof/derivation, compiled examples and dependencies; fix only justified errors; inspect print figures for clipping and minimum readable type; verify source files/figure provenance/licensing and figure numbering; make the new materials present in both PDF and semantic reflowable EPUB (the current injection is LaTeX-side only); resolve anchors, cross-volume navigation and stable images. The new public cover/title must be applied consistently without breaking the staging catalogue. Finally validate complete PDF, EPUB and reader exercises on the author's Mac workflow.

**Remaining acceptance distinction:** Pass 1 complete means the first additive conceptual/editorial repair pass is completed and tested, **not** that final Book Two is complete, nor that the later three passes are optional.
