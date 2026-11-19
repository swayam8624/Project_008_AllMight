# Book Two — Content-first manuscript audit and remediation gate

**Series:** From Bits to Research · Book Two of Seven  
**Public working title:** From Equations to Geometry  
**As-of:** 20 November 2026  
**Source branch:** `curriculum/book02-mathematics-refinement-pass01`  
**State:** **CONTENT REVIEW OPEN.** No further EPUB/KDP acceptance claims should be made until the original chapter content and every new addition are judged together.

## Why this gate exists

The author wants the **book's knowledge, coherence, exercises, research connections and code** correct before any new format work. Earlier efforts successfully established numerical teaching labs, seven figure exports, 35 add-on TeX fragments and a valid *supplement-only* EPUB proof, but they did **not** complete whole-manuscript mathematical or narrative verification. Those checks must not be misrepresented as final content closure.

All canonical original source files remain protected. The 16 original generated TeX chapters contain approximately **1.75 million Unicode characters**, or approximately **225,000 broad TeX-token words**. A conservative automated inventory found **1,017 section/subsection headings, 3,801 display-math starts, 112 literal code environments and 76 diagram/image inclusions**. These metrics establish that the actual book is substantial. They **do not** prove that every paragraph is clear, every equation is true, or every research claim is traceable. Counts are approximate and depend on the TeX extraction rules.

Original snapshot: `Book Production/AllMight Codex/LaTeX/Generations/.generation-n3azv4es/`. Do not rewrite those generated chapters directly; reconcile actual corrections in their canonical authoring source and publish them through the official generation workflow.

## First-pass chapter audit (source inspected, not a chapter-by-chapter mathematical certificate)

| Order | Original chapter ID | Actual thematic scope evidenced in the source | Main content acceptance risk and next review |
|---:|---|---|---|
| 1 | `01-foundations-i-ch017` | Python, profiling, buffers, zero-copy, GIL, vectorization, stability, linear algebra, workbench; mathematics foundations begin inside same chapter | Python and advanced performance arrive before some foundational objects. Keep the preface/reader contract and confirm nothing essential relies on unavailable prerequisite reading |
| 2 | `01-foundations-i-ch018` | Vectors, optical reflection/refraction, barycentric geometry, predicates, convex support, transformations | Many `Immediate footing` interruptions; verify that each is at first use and not a repeated glossary that breaks causal narrative |
| 3 | `01-foundations-i-ch019` | Frames, camera inverses, quaternion algebra, slerp, rigid transforms, linear systems and pivoted LU | Independently test row/column conventions, half-angle quaternion updates, tangent normal treatment, singularity and LU permutation |
| 4 | `01-foundations-i-ch020` | QR/LDLT/Cholesky, eigensystems, conditioning, calculus foundations | Verify factorization domains, rank-deficiency and epsilon-delta/calculus entry transitions |
| 5 | `02-foundations-ii-ch021` | Jacobians, differentials, Hessians, perspective projection, nonlinear sensitivity | Check derivative shape, coordinate spaces, depth sign, small-step truncation and all stated numerical Jacobian examples |
| 6 | `02-foundations-ii-ch022` | Riemann integration, quadrature, ODE introduction, dual-number AD, reverse tape, PCA and probability bridge | The span from integration to autodiff to probability is wide; verify every reason for moving to the next mathematical tool |
| 7 | `02-foundations-ii-ch023` | Measure spaces, conditional probability, Bayes, distributions, density, statistical measurement error | More than 120 subheadings; check prose flow and whether formal measure theory is introduced before prerequisite examples |
| 8 | `02-foundations-ii-ch024` | Exponential/Poisson processes, multivariate Gaussian, covariance, whitening, PCA, SVD | Test Gaussian normalizations, covariance rank/singularity, conditional formula domains and unit consistency |
| 9 | `02-foundations-ii-ch025` | PDE/heat/Green methods, mechanics, symplectic geometry, explicit and implicit ODE integration, error | Dense transitions between PDEs, Hamiltonian mechanics and integrators; independently verify stability factors and conservation claims |
| 10 | `02-foundations-ii-ch026` | Matrix functions, exp/log, optimization, convexity, KKT, duality and nonlinear least squares | Verify strong assumptions for convexity, matrix function branch/domain and correct KKT/dual signs |
| 11 | `02-foundations-ii-ch027` | Gauss–Newton, LM, 3D pose manifold updates, SE(3) and a C++23 source implementation | Verify tangent/Jacobian conventions, gauge and near-π behavior in the provided SE(3) program |
| 12 | `02-foundations-ii-ch028` | Optimizer workbench, root finding, IRLS, Schur, conditioning, error/stability and stable arithmetic | More than 110 subheadings and 24 code environments; code example completeness and transitions are high-priority |
| 13 | `02-foundations-ii-ch029` | Discretization, numerical convergence, PRNGs, inverse sampling, bandits and experimental design | Ensure numerical error studies transition into stochastic estimation and sequential control without misleadingly conflating them |
| 14 | `02-foundations-ii-ch030` | Monte Carlo, CLT, bias/variance, correlation, inference, confidence, robust statistics | Verify independence assumptions, effective sample size and finite-sample confidence claims before publication |
| 15 | `02-foundations-ii-ch031` | Statistical C++23 contracts, covariance, numerical tails, probability sampling, renderer estimator cases | Check implementation error handling, probabilities vs densities, sample support and bridge to actual pixel evidence |
| 16 | `03-visual-computing-i-ch012` | Probability/sampling and rendering; later sections already move into raster coverage/depth and coordinate spaces | Imported source navigation refers to GPU Architecture and another graphics volume. This is a cross-book continuity problem needing explicit new bridge and link-remapping acceptance |

**Review status of every row above:** thematic inspection complete; **full independent mathematical and code review pending**. Sections/exercises cannot be given a blanket S5 classification just because adjacent numeric labs passed.

## Verified content issue and actual revision begun

The transition from sampling contracts into pixel coverage, including the last imported visual-computing chapter, is weak. The new source is staged at:

`Supplementary/Book 02 Content Review/36-from-probability-to-pixels.tex`

It has a concrete forcing problem: what fraction of one unit-square pixel is covered by an affine triangle? The exact triangle
(A=(-1/4,1/4), B=(5/4,1/4), C=(1/2,5/4))
clips to a six-vertex polygon of exact rational area **119/192**. The lesson derives its vertices and area twice, directly links the determinant to an edge test, proves the unbiased sample-coverage estimate and the independent Bernoulli variance (p(1-p)/N), distinguishes closed-set area integration from rasterizer shared-edge ownership, and explains what a true camera/rendering pipeline still has to add.

The independent runnable implementation is
`tools/labs/book02_coverage_oracle.py`
with tests in `tools/labs/test_book02_coverage_oracle.py`.
They check exact polygon coordinates/area, full/half/zero coverage, winding invariance, on-boundary policy, deliberate failures and independently seeded sampling variance.

The original source-driven diagram generator is
`tools/labs/book02_coverage_visual.py`.
It produces a vector PDF, SVG and PNG plus a 60-replicate CSV and provenance report. This diagram is **content**, not a fabricated performance benchmark.

The content lesson is intentionally **staged outside the 35-section publication inventory** until manuscript review approves its location. It must eventually be integrated into the canonical teaching source—not merely copied into final EPUB or appended as a disconnected PDF chapter. Therefore no EPUB inventory count or KDP proof is changed by this addition today.

## Important remaining deficiencies

**Citation scope:** Primary-source records demonstrate selected inspection of Shewchuk, libigl, Crane, Sharp/Crane, Ceres, Fischler/Bolles, Veach and several Kairo source files. Not all original mathematics books were read in full, and the original manuscript's many claims are not yet tracked claim by claim. The existing evidence ladder S0–S5 must be applied precisely.

**Executable reach:** Passing Book Two's small stand-alone geometry/numerical source suites does not prove native C++ module builds for every KairoMath/KairoPhysicsMath example or full GPU equivalence. Original chapter code blocks need an exact-head compile, run, negative-test and documented-assumption ledger.

**Link integrity:** The frozen source chapter boundary for `03-visual-computing-i-ch012` contains navigation to adjacent source material now placed in other physical volumes. This may be valid as a source pointer but is poor if presented as the next chapter in Book Two. Resolve logically with correct reader routes, not by deleting the source explanation.

**Teaching density:** Extensive developed content is valuable, but the work still needs a full narrative pass for symbol reuse, terminology, repeated micro-headings, no-prerequisite jumps, and misleadingly short statements inside advanced derivations. Numerical/citation correctness must be evaluated separately from reading quality.

**Scientific illustrations:** The seven Pass 3 source-computed diagrams are verifiable numerical examples but not exact published-paper benchmark replicas. Ensure all new figures are both scientifically explanatory and aesthetically consistent with the actual book, at appropriate printed size and with independent alt-text in digital versions.

## Content acceptance contract before EPUB

1. Finish source-grounded narrative and mathematical review for **all 16 chapters**, not just the frontmatter, stage headings or 35 supplements.
2. Correct actual wrong derivations in canonical source, add worked examples where concept bridges are insufficient, and preserve every valid explanation/code block.
3. Inventory algorithms and code blocks with exact compiler/version, source path, expected outputs and negative/adversarial tests; do not claim a pass without execution evidence.
4. Audit linked primary paper/repository claims against source material and record source sections or exact file SHA where used.
5. Integrate the staged chapter-transition lesson and its source-generated scientific figure at its pedagogically correct point after the above review.
6. Recheck **whole-book** progression and study projects; confirm a reader can explain assumptions, derive the main equation, implement a minimal version, break it and interpret failure.
7. Only then resume full EPUB and PDF production, actual reader validation and cover placement.

**Author-supplied cover:** The JPEG is on the author's Mac (local Downloads path supplied in conversation). The remote GitHub environment cannot certify the JPEG image from that path; do not mark the cover reviewed or hard-code a personal absolute filesystem path into the public source repository.

**Do not run Git commands in the user's existing `Master` vault to resolve this branch mismatch while content is still under audit.** Work stays isolated on the remote content branch, then publish the reviewed source through a non-destructive integration strategy.
