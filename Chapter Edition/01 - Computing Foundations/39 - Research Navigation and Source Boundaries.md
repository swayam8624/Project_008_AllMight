# 39 — Research Navigation and Source Boundaries

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|Previous chapter]]

The final chapter preserves research connections, reader routes, provenance, and evidence boundaries behind the teaching chapters. These materials help readers continue investigation while keeping source records distinct from explanations and implementation claims.

**By the end:** Use source records and research navigation to verify a claim, identify its limits, and choose a next reading or experiment.

**Recall question:** What primary source, experiment, or runtime evidence would you need before extending one of this volume’s bounded claims?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 97116–98917; source-span SHA256 2ef3656b0dba9058075b610004c439399bf688314ce8d3862f8313bdfab594ac. -->

<!-- CHAPTER-SOURCE-BEGIN -->
# Teaching reconstruction, source records, and research navigation

The following source navigation, research connections, coverage boundaries and provenance are retained in full. These records preserve the evidence behind the teaching text; they do not replace the explanations in the preceding chapters.

<!-- begin V06.29 -->
## Research connections without borrowed certainty

The additional primary references sharpen particular boundaries; they do not make every example a research result.

- **Robust predicates:** compare a naive determinant with a certified sign routine on near-collinear stored inputs. Separate input uncertainty from arithmetic uncertainty. No certified routine was added here.
- **Frame construction:** compare helper-axis frames against a specialized orthonormal-basis algorithm on normals near difficult axes, including temporal continuity. No specialized paper algorithm or performance comparison was run here.
- **Numerical linear algebra:** compare Gram solves, classical Gram–Schmidt, modified Gram–Schmidt, and Householder QR using both reconstruction and orthogonality residuals. The small lab tests a frame, not a full QR implementation.
- **Rendering implementation:** inspect how a production triangle separates geometric and shading normals and handles degenerate texture derivatives. Our single-panel illustration does not implement a complete mesh pipeline.
- **Language and API contracts:** contrast valid array indexing with adjacent-member shortcuts, and CPU layouts with shader interfaces. Passing a sanitizer does not replace the language or buffer specification.

When reading a paper, record the input assumptions, measured outcome, adversarial cases, comparison baseline, precision/compiler environment, and limits. “Fast,” “robust,” and “production-ready” are claims to unpack, not adjectives to inherit from a pasted lesson.

This phase's achievement is a connected model: numbers provide scalars; vector laws provide movement; bases provide descriptions; inner products provide measurement; exterior products provide orientation; affine weights provide locations; projective representatives support perspective; and types preserve the distinctions at computational boundaries. The implementation is valuable because we can now explain both what it computes and why it sometimes must refuse to compute it.

[[Graphics Engine Mastery - Continuous Notes]] · [[Dependency Map]] · [[Keyword Index]] · [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Phase 6 - Geometry reconstruction proofs|Extended proofs]] · [[Supplementary/Worked Traces#Phase 6 - The robot panel tracebook|Extended traces]] · [[Visual Recall/06 - Geometry and Vector Implementation - Visual Recall|Visual-recall board]]

### Where measurement geometry leads next

The Gram matrix introduced here is the same kind of object that later appears as $A^\mathsf T A$ in least squares and covariance. Each entry is a dot product between columns, so it records how measured directions overlap. If those columns are nearly dependent, the measurements almost describe the same direction; solving becomes sensitive even though the equations are syntactically valid. Phase 8 makes that intuition precise through rank, null spaces, singular values, and condition numbers. Follow [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#The graphics problem hiding inside a matrix equation|the Phase 8 story]] when you want to see what the geometry of measurements implies for a numerical solve.

The triangle panel also gives a small model of later linear algebra: independent edge directions determine reachable positions; degenerating the triangle removes one direction; fitting noisy points asks for the closest reachable output. Those are the geometric beginnings of rank and least squares, not unrelated topics that suddenly appear in a solver chapter.

<!-- end V06.29 -->

<!-- begin V08.02 -->
## A reader route through Phase 8

Follow the chain from what a system means to what a numerical library can promise:

1. Start with [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A system is an intersection of constraints|constraints as geometry]] and [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Rank, the four spaces, and the shape of the answer|rank and the four spaces]]. Together they show which outputs are reachable and which input changes remain invisible.
2. Follow [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Elimination multipliers become stored factor data|elimination into reusable factors]]. Pivoting and factor storage preserve the equations while making repeated solves practical.
3. Continue through [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Orthogonal coordinates preserve the shape of the problem|orthogonal coordinates]] and [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#QR turns least squares into a triangular solve plus an irreducible residual|QR least squares]]. This is where stable coordinate changes leave the irreducible residual visible.
4. Read [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Eigenvalues, eigenspaces, and the two kinds of multiplicity|eigenvectors and eigenspaces]] for invariant directions of square maps, including the limits set by multiplicity and sensitivity.
5. Move from square maps to [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#The SVD equation as independent input-output channels|SVD]], which gives separate orthonormal input and output directions for every rectangular map. Follow [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Figure L6a - Watch the same directions become an ellipse|Figure L6a]] and its [[Visual Recall/phase8_svd_geometry.svg|editable vector figure]] alongside the [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#A concrete rotation, stretch, and coordinate trace|numeric coordinate trace]].
6. Trace SVD's applications through [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Least squares from decoupling, then minimum norm|least squares and minimum norm]] and [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Centered measurements reveal principal directions|PCA and geometric fitting]].
7. Close with [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#An SVD API begins with shapes and guarantees|shapes, validation, and API contracts]], where mathematical guarantees meet storage and solver status.

### Figure L1 - The route from measurements to a numerical API

```mermaid
%%{init: {'theme':'base','themeVariables':{'primaryColor':'#e0f2fe','primaryTextColor':'#15304a','lineColor':'#64748b','fontFamily':'sans-serif'}}}%%
flowchart LR
  A["Rows are measurements"] --> B["Ax = b<br/>constraints meet"]
  B --> C["Rank, pivots<br/>freedom and consistency"]
  C --> D["Factorizations<br/>reuse structure"]
  D --> E["Eigenvectors and SVD<br/>reveal preserved directions"]
  E --> F["Least squares, PCA,<br/>compression, matrix functions"]
  F --> G["An API reports results,<br/>residuals and limits"]
  classDef finish fill:#dcfce7,stroke:#15803d,color:#14532d;
  class G finish;
```

> **How to read Figure L1.** The route is a dependency map, not a claim that every application must run every algorithm. Rank and structure guide the choice; a validation contract makes the output inspectable.


<!-- end V08.02 -->

<!-- begin V08.83 -->
## A reconstruction desk for active teaching · V08.83.01

| Prompt | What a learner should reconstruct |
|---|---|
| Why can an overdetermined system still have an exact solution? | Show that extra rows may repeat or depend on earlier constraints; consistency is membership in $\operatorname{Col}(A)$. |
| What does the null space say physically? | Give a nonzero input change invisible to every row measurement. |
| Why apply row operations to the right-hand side too? | Each row operation replaces one equation by an equivalent combination; changing only $A$ changes the problem. |
| Why pivot? | A zero pivot blocks division; a very small pivot can amplify rounding. Explain why partial pivoting helps and why it is not a condition-number cure. |
| Why not form $A^{-1}$? | Solve through factors and compare work, rounding, and reuse. |
| Why is QR safer than normal equations? | Derive the squared condition number of $A^\mathsf T A$. |
| What makes symmetric eigenanalysis special? | State the orthonormal eigenbasis and why orthogonal coordinates preserve 2-norm. |
| How does SVD work for a rectangular map? | Name the distinct input/output spaces and trace $V^\mathsf T$, $\Sigma$, and $U$. |
| What does a small singular value mean? | Distinguish absent information, weak measurement, and the chosen numerical threshold. |
| What does the pseudoinverse choose? | Least-squares residual first, then minimum Euclidean norm among minimizers. |
| Why center data before PCA? | Separate variation around the mean from displacement of the mean itself. |
| What does polar decomposition separate? | Orthogonal motion and symmetric stretch; account for reflection and singular cases. |
| How do we validate an SVD? | Check reconstruction, both orthogonality conditions, ordering, shapes, and convergence separately. |
| When does a residual fail to prove a good solution? | Construct an ill-conditioned example with a small residual and large solution change. |
| Why can a matrix logarithm be ambiguous? | Discuss eigenvalue branches and the domain rather than treating the scalar formula as universal. |


<!-- end V08.83 -->

<!-- begin V08.84 -->
## Research and implementation references

These references are used to check algorithm families and numerical-library contracts; the chapter's examples remain teaching derivations and are not claims about a production implementation.

- The [[Supplementary/Sources and Code Anchors#Phase 8 - Source analysis and verification|source ledger]] records all seven supplied excerpts, corrections, and validation boundaries.
- LAPACK's [Users' Guide](https://netlib.org/lapack/lug/index.html) separates linear equations, orthogonal factorizations, least squares, eigenproblems, and singular value problems. Its [least-squares chapter](https://www.netlib.org/lapack/lug/node27.html) distinguishes full-rank QR/LQ drivers from rank-deficient SVD or complete-orthogonal-factorization paths.
- LAPACK's [`DGESVD` interface](https://netlib.org/lapack/explore-html/d1/d7f/group__gesvd_gac6bd5d4e645049e49bb70691180abf07.html) specifies factor shapes, optional singular vectors, workspace, and status. A returned numerical library contract should be checked at that level rather than inferred from a name alone.
- The [LAPACK `DGESV` routine](https://netlib.org/lapack/explore-html/d8/da6/group__gesv_ga831ce6a40e7fd16295752d18aed2d541.html) documents an LU solve with partial pivoting and row interchanges.
- SciPy's current [`expm` reference](https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.linalg.expm.html) describes a Padé-based matrix exponential and cites the scaling-and-squaring literature. It supports the algorithm family explanation, not a benchmark claim for this vault.


<!-- end V08.84 -->

<!-- begin V08.85 -->
## Source coverage and the limits of this teaching volume

All seven supplied Phase 8 excerpts are integrated: Parts 701–736, 737–769, and 770–800, without a numbered gap in the received material. The first SVD excerpt also contains an unnumbered geometric bridge and the eigenanalysis excerpt repairs conditioning concepts; both are woven into their prerequisite locations. The source titled “Parts 786–800” extends beyond the request's “till Part 770” description, so those supplied lessons are included rather than silently omitted. See [[Supplementary/Foundations#Phase 8 - Parts 701-800|the exact title ledger]].

The chapter explains the mathematics and algorithm choices but does not implement a complete dense or sparse solver library. It does not claim a GPU run, production performance, a universal rank threshold, certification of every residual, or proof of personal mastery. Those claims require separate implementation, target, and workload evidence.

[[Graphics Engine Mastery - Continuous Notes]] · [[Dependency Map]] · [[Keyword Index]] · [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Phase 8 - Linear systems and decompositions|Extended derivations]] · [[Supplementary/Worked Traces#Phase 8 - From constraints to a stable SVD contract|Worked trace]] · [[Visual Recall/08 - Linear Algebra - Visual Recall|Visual recall]]

<!-- end V08.85 -->

<!-- begin V10.01 -->
## Learning route

This volume integrates the full supplied Phase 10 teaching material for Parts 901–1000. The original explanations, mathematical derivations, examples, code snippets, and failure cases are preserved below as the base text, then grouped by conceptual dependency. The numbered source-part labels are removed from lesson headings; the exact part-to-section ledger remains in `source-to-chapter-map.md` and the detailed atom ledger.

The story proceeds from a model of uncertainty, through random quantities and their distributions, into moments and covariance geometry. Those laws then guide how deterministic computers generate samples, how sampling transformations change probability measures, and how weighted averages estimate rendering integrals. The final chapters ask what finite observations can establish, how to calculate their statistics stably, and how to distinguish random variation from systematic error.


<!-- end V10.01 -->

<!-- begin V10.15 -->
## Related vault chapters

- For overflow-safe norms, uncertainty in geometric directions, and surface frames, see [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#A direction can exist even when the naive length overflows|A direction can exist even when the naive length overflows]] and [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Surface movement becomes a local frame|Surface movement becomes a local frame]].
- For covariance under coordinate changes and inverse sensitivity, see [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Coordinates can change while the geometric object stays still|Coordinates can change while the geometric object stays still]] and [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#An inverse can exist and still amplify uncertainty|An inverse can exist and still amplify uncertainty]].
- For covariance eigenvectors, PSD structure, and stable matrix calculations, see [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Eigenvectors reveal the directions a square map does not turn|Eigenvectors reveal the directions a square map does not turn]] and [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Diagonalization leads to the stronger symmetric spectral theorem|Diagonalization leads to the stronger symmetric spectral theorem]].
- For the C++ state-lifetime context around PRNGs, see [[Chapter Edition/01 - Computing Foundations/02 - Values at Addresses Layout, Lifetimes, and Representation#A name does not decide how long storage lasts|A name does not decide how long storage lasts]].
- For the rendering equation and Monte Carlo estimator connection, see [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]].


<!-- end V10.15 -->

<!-- begin V10.16 -->
## Supplied source claims and provenance requiring verification

The following material preserves claims reported by the supplied lesson excerpts. It does not independently verify the repository snapshot, downloadable artifacts, or reported test outputs. Root build records are tracked separately.

The following repository-audit and reported-test sections were moved out of the teaching narrative because their claims depend on a repository revision or unattached compiled artifacts. Their source text is retained here for audit and later revalidation.

### Unverified curriculum cross-check claim

I verified all 30 part titles against the original curriculum appended to your Integrated 180-Chunk System. We will follow that coverage exactly and add the necessary mathematical foundations that the titles alone do not specify.

### T01 · Source-reported C++23 laboratory build claim

I created a complete standalone reference implementation covering the methods introduced in M055 and M056.

Phase 10 probability laboratory

Compiled & tested

C++23 · Bernoulli and binomial PMFs · uniform, normal, exponential, and Poisson distributions · Bayesian updating · 2D multivariate Gaussian · positive-semidefinite validation · Cholesky sampling · online covariance · statistical sampling tests.

Download the complete commented C++23 source

Compile with:
```bash
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic \
  phase10_m055_m056_probability.cpp \
  -o probability_lab

./probability_lab
```

I compiled and executed the source with the stated C++23 settings. Its assertions passed.

### T01 · Source-reported test artifact/results (source section 35: Actual numerical test results)

#### Actual numerical test results

The compiled program evaluates analytic distribution formulas and also performs stochastic validation.

C++23 EXECUTED REFERENCE OUTPUT

| Calculation                                   | Result      |
| --------------------------------------------- | ----------- |
| Bayesian $P(\text{hit}\mid+)$             | 0.568862275 |
| Binomial $P(K=2), n=5,p=0.3$              | 0.308700000 |
| Uniform $U(2,6)$ variance                 | 1.333333333 |
| Normal $\mathcal N(10,4)$ density at mean | 0.199471140 |
| Exponential rate 2, $P(T\le0.1)$          | 0.181269247 |
| Poisson mean 2, $P(K=3)$                  | 0.180447044 |
| 2D Gaussian density at mean                   | 0.027072806 |

Monte Carlo sampling test: 100,000 generated 2D Gaussian samples

| Moment        | Theoretical | Observed  |
| ------------- | ----------- | --------- |
| Mean X        | 0.500000    | 0.499162  |
| Mean Y        | -1.000000   | -1.002491 |
| Variance X    | 4.000000    | 3.992100  |
| Covariance XY | 1.200000    | 1.214963  |
| Variance Y    | 9.000000    | 8.989716  |

All assertions passed, including approximate statistical-moment checks.

Notice that empirical moments do not match the theoretical values exactly.

That is expected.

A finite random sample contains sampling variability.

The testing strategy uses reasonable numerical tolerances rather than requiring equality between sampled and theoretical moments.

Later, when we derive the law of large numbers, central limit theorem, and standard errors, we will be able to quantify how much deviation to expect for a given number of samples.

#### 1 Deterministic seed versus portable sample stream

The program uses:
```cpp
std::mt19937_64 engine(20261027ULL);
```

This provides an explicitly seeded pseudorandom engine.

The engine itself has standardized behavior, but C++ standard-library distribution classes are not generally required to generate identical sequences across different library implementations for the same engine state.

In other words, the program is reproducible under a controlled implementation, but its exact sampled Gaussian values may differ across standard libraries.

For cross-platform bitwise reproducibility, you may need a specified integer generator and explicitly implemented distribution transformations.

This becomes especially important in renderer benchmarking, distributed rendering, GPU kernels, and deterministic research experiments.

### T01 · Repository audit assertions (source section 36: Repository-grounded analysis: KairoMath Probability and Statistics)

#### Repository-grounded analysis: KairoMath Probability and Statistics

I inspected the current files:

[KairoMath/Probability.cppm](https://github.com/swayam8624/KairoMath/blob/master/Probability.cppm?utm_source=chatgpt.com)

[KairoMath/Statistics.cppm](https://github.com/swayam8624/KairoMath/blob/master/Statistics.cppm?utm_source=chatgpt.com)

These are actual repository observations, not assumptions about what the modules should contain.

#### 1 Existing probability API

The current `Probability.cppm` provides a `RandomGenerator` wrapping `std::mt19937_64`, together with `UniformDistribution`, `NormalDistribution`, `BernoulliDistribution`, and `ExponentialDistribution`.

These distributions provide varying combinations of probability evaluation, expected value, variance, cumulative probability, and sampling.

The module also contains weighted-index sampling and helpers for sample mean, variance, and standard deviation.

This provides a useful foundation.

However, the current inspected file does not implement dedicated `BinomialDistribution`, `PoissonDistribution`, or `MultivariateNormalDistribution` types.

The reference laboratory fills some of those mathematical gaps, but it is a separate standalone educational implementation. It has not been integrated into KairoMath.

#### 2 Numerical and validation improvements worth making

The current `NormalDistribution::Cdf` computes:
```cpp
return T(0.5) * (T(1) + std::erf(z));
```

This is mathematically correct, but may lose accuracy in the extreme lower tail.

The complementary-error-function formulation:
```cpp
return T(0.5) * std::erfc(-z);
```

is preferable for that regime when `z` contains the mean- and standard-deviation-normalized quantity divided by $\sqrt2$.

Likewise, the current exponential CDF computes:
```cpp
T(1) - std::exp(-Lambda * x)
```

which can suffer cancellation for very small $\lambda x$.

A more stable alternative is:
```cpp
-std::expm1(-Lambda * x)
```

for nonnegative $x$.

These are targeted numerical improvements, not changes to the distribution definitions.

#### 3 Validate nonfinite parameters explicitly

The current Bernoulli constructor checks whether the probability is below zero or above one.

But comparisons involving NaN are false.

A NaN probability can therefore evade a check written only as:
```cpp
if (p < 0 || p > 1)
```

unless additional validation occurs elsewhere.

A stronger condition is:
```cpp
if (!std::isfinite(p) || p < 0 || p > 1)
```

Similarly, distribution constructors should specify whether infinite bounds, infinite standard deviations, or nonfinite means are legal.

For ordinary finite-parameter probability APIs, rejecting nonfinite parameters is usually the clearer contract.

#### 4 Weighted sampling robustness

The current `SampleWeightedIndex` validates nonnegative weights and a positive total, then selects an index by accumulating weights.

For production robustness, it is also important to reject nonfinite weights, guard against overflow in the accumulated total, and choose boundary comparisons that do not accidentally select a zero-weight entry when a uniform target lands exactly on a cumulative boundary.

We will examine weighted discrete sampling algorithms properly in M057.

#### 5 Covariance and correlation implementation

The current `Statistics.cppm` has a `CovarianceMatrix` function that accepts a matrix with observations stored as rows and features stored as columns.

It computes a mean per feature and then accumulates centred products.

Its covariance divisor is:

$$
 n-1, 
$$

so it implements the conventional unbiased sample covariance estimator, not the population covariance using divisor $n$.

This distinction should be explicit in the API documentation.

The resulting matrix is populated symmetrically.

The associated `CorrelationMatrix` function divides covariance entries by the product of the corresponding standard deviations.

For a feature with zero variance, the inspected implementation assigns a correlation value of zero.

That is a chosen software policy. Mathematically, Pearson correlation is undefined when a standard deviation is zero.

A library should document this choice because returning zero could otherwise be misinterpreted as evidence of no association.

#### 6 Eigenvalues and covariance repair

The repository's PCA implementation computes a covariance matrix and obtains eigenvalues and eigenvectors.

It clamps reported negative eigenvalues to zero when assembling the explained-variance output.

Small negative eigenvalues can arise from numerical error, but significant negative eigenvalues may indicate a deeper problem.

A robust implementation should distinguish small tolerated numerical violations from major mathematical inconsistency.

It is also useful to test reconstruction, orthogonality, and eigenvalue residuals:

$$
 \\|\Sigma Q-Q\Lambda\\| 
$$

rather than trusting eigenvalue signs alone.

These checks connect the probability module to the eigenvalue and SVD tests from Phase 8.

### T01 · Removed completion/status and forward-prompt metadata

The source labels Parts 901–930 “Taught,” claims compiled C++23 validation and repository inspection, and points to the next prompt for Parts 931–965. These progress claims and prompt routing are not teaching content; the completion claims are unverified and the next supplied lessons are already integrated. The source reading references were retained in the teaching volume.

### T06 · Source-reported test artifact/results (source section 36: Complete C++23 sampling and statistics laboratory)

#### Complete C++23 sampling and statistics laboratory

I created and compiled a standalone implementation covering M057 and M058, including reproducible random streams, PCG32-style generation, independently addressable samples, unbiased bounded-integer sampling, inverse-CDF and rejection sampling, importance-weighted estimators, hemisphere sampling, online statistics, quantiles, histograms, confidence-interval calculations, and error metrics.

Phase 10 · M057–M058 Reference Implementation

Tests passed

Standalone C++23 source · approximately 18 KB · fully commented · compiled with strict warnings and numerical assertions.

Download the complete source

Compile with:
```bash
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror \
  phase10_m057_m058_sampling_statistics.cpp \
  -o sampling_lab

./sampling_lab
```

The reference uses an explicit seeded generator. Its assertions test determinism under repeated initialization, independently addressed sample inputs, the theoretical moments of importance-weighted estimators, hemisphere integration, online variance, merged Welford statistics, quantiles, and error metrics.

#### 1 Results from actual execution

COMPILED PROGRAM OUTPUT

| Test                           | Result       |
| ------------------------------ | ------------ |
| Exact $\int_0^1x^2dx$      | 0.333333333  |
| Uniform MC, 100,000 samples    | 0.333317224  |
| Importance MC, 100,000 samples | 0.332739762  |
| Theoretical variance reduction | 6.400000000× |
| Rejection acceptance rate      | 0.501014554  |
| Uniform hemisphere estimator   | 3.150050643  |
| Cosine hemisphere estimator    | 3.141592654  |
| Welford sample variance        | 3.500000000  |
| Large-offset sample variance   | 1.666666667  |
| Outlier sample mean / median   | 22 / 3       |
| Prediction MSE / RMSE / MAE    | 1 / 1 / 1    |

All numerical and statistical assertions passed.

There is an important statistical observation here. In this particular run, the uniform Monte Carlo estimate happened to be closer to the exact integral than the importance-sampled estimate.

That does not contradict the variance calculation.

A lower-variance estimator does not guarantee a smaller error in every single realization. It guarantees smaller variance across repeated independent experiments under the mathematical model.

The correct way to compare the two methods is over repeated independent trials, using the same sample budget or computational-cost budget, rather than judging them from one fortunate or unfortunate run.

This distinction should be central to every randomized algorithm benchmark.

### T06 · Repository audit assertions (source section 39: Actual repository audit)

#### Actual repository audit

The authoritative repository targets for these chunks are:

[KairoMath/Probability.cppm](https://github.com/swayam8624/KairoMath/blob/master/Probability.cppm?utm_source=chatgpt.com)

[KairoRayTracer/Sampler.cppm](https://github.com/swayam8624/KairoRayTracer/blob/main/Sampler.cppm?utm_source=chatgpt.com)

[KairoMath/Statistics.cppm](https://github.com/swayam8624/KairoMath/blob/master/Statistics.cppm?utm_source=chatgpt.com)

I inspected all three files directly.

#### 1 KairoMath probability generator

The existing `RandomGenerator` wraps `std::mt19937_64` with explicit caller-owned state.

It supports deterministic seeding, access to the underlying engine, bounded integer sampling, and uniform real sampling.

That is a useful design because random state is not hidden in global mutable variables.

The distribution helpers include uniform, normal, Bernoulli, and exponential distributions.

However, the current file does not contain a dedicated PCG or counter-based generator.

It also does not provide a general sampler keyed explicitly by pixel, sample index, and sampling dimension.

For a renderer, those may be useful additions.

#### 2 A subtle normal-distribution performance issue

The current normal distribution sampling method constructs a `std::normal_distribution<T>` object each time `Sample()` is called.

Some normal-distribution algorithms generate two normal samples at once and cache one for later reuse.

Reconstructing the distribution object on every call can prevent effective reuse of that cached result under implementations using such a strategy.

Whether this causes a material performance loss depends on the standard-library implementation and workload.

It is worth benchmarking persistent versus reconstructed distribution objects rather than assuming the difference is negligible.

#### 3 KairoRayTracer stratified sampling

The current `Sampler.cppm` defines `PixelSample`, `Hash01`, and `StratifiedPixelSample`.

The stratified sampler computes:

$$
 s=\left\lceil\sqrt N\right\rceil 
$$

where $N$ is `samplesPerPixel`.

It then maps:

$$
 s_x=\text{sampleIndex}\bmod s, 
$$

$$
 s_y=\left\lfloor\frac{\text{sampleIndex}}s\right\rfloor. 
$$

The jittered sample coordinates are constructed as:

$$
 u=\frac{s_x+\xi_x}{s}, \qquad v=\frac{s_y+\xi_y}{s}, 
$$

with pseudorandom jitters $\xi_x,\xi_y\in[0,1)$.

When $N=s^2$, and indices cover all cells, this gives a jittered $s\times s$ grid.

But when $N$ is not a perfect square, it uses only the first $N$ cells in a grid containing $s^2$ cells.

#### 4 Why this is a genuine bias risk

Suppose:

$$
 N=5. 
$$

Then:

$$
 s=\lceil\sqrt5\rceil=3. 
$$

The intended grid contains nine cells.

But only five are sampled.

With row-major indexing, the used cells are the first five, leaving the remaining four cells completely unsampled.

If the resulting samples are averaged with equal weights as a pixel integral estimate, the expected estimator is the average over those selected cells, not the average over the whole pixel.

For an integrand concentrated in the missing cells, the estimator will systematically underestimate the integral.

This is not merely a visual stratification imperfection. Under an ordinary equal-weight pixel-average interpretation, it can produce bias.

#### 5 A correct arbitrary-count alternative

One simple alternative is randomized Latin-hypercube sampling.

For $N$ samples, divide both coordinate axes into $N$ equal intervals.

Generate a random permutation:

$$
 \pi:\{0,\ldots,N-1\}\rightarrow\{0,\ldots,N-1\}. 
$$

For each sample $i$, draw independent jitters $u_i,v_i\sim U(0,1)$ and construct:

$$
 \boxed{ x_i=\frac{i+u_i}{N} } 
$$

and:

$$
 \boxed{ y_i=\frac{\pi(i)+v_i}{N} }. 
$$

Every horizontal stratum is visited once, and every vertical stratum is visited once.

For a randomly chosen permutation and properly randomized jitters, each individual sample has a uniform marginal distribution over the pixel when averaging over the randomization.

The equal-weight estimator is unbiased for an integrable pixel function under the corresponding randomization assumptions.

The samples are dependent by design, so the ordinary IID variance formula must not be applied blindly.

The complete reference file includes a Latin-hypercube pixel-offset generator, Fisher–Yates permutation using unbiased bounded integer selection, and an assertion that every marginal stratum is visited exactly once for an arbitrary non-square sample count of five.

#### 6 Other renderer checks

The current `StratifiedPixelSample` returns the pixel centre when `samplesPerPixel <= 1`.

That is a deterministic midpoint sample, not an ordinary random uniform sample.

It may be an intentional engineering choice, but its estimator properties differ from randomized single-sample Monte Carlo.

The implementation also does not explicitly reject a `sampleIndex` outside the intended sample count.

Invalid indices should not silently produce clamped boundary coordinates.

A production sampler should validate or clearly document the index contract.

Finally, its two jitters derive from hash calls using related integer inputs. This gives deterministic variation, but hashing alone does not prove that the resulting floating-point samples have the multidimensional distribution quality required for advanced rendering.

A sampler evaluation should include statistical tests and actual rendering convergence experiments.

#### 7 KairoMath Statistics

The current `Statistics.cppm` computes covariance matrices, correlation matrices, principal component analysis, and linear regression.

Covariance is calculated using centred observations and a divisor of $N-1$.

That implements the conventional unbiased sample covariance estimator under the usual assumptions.

However, the current module does not expose Welford-style streaming mean/variance objects, parallel mergeable moment accumulators, or confidence-interval calculations.

Those would be useful additions for renderer performance statistics and online Monte Carlo error tracking.

For numerical stability, streaming centred moments are preferable to maintaining only a raw sum and raw sum of squares in many high-offset workloads.

### T06 · Removed completion/status and forward-prompt metadata

The source labels Parts 931–965 “Covered,” claims compiled tests and repository inspection, and describes a later prompt for Parts 966–1000. The status claim is unverified; the forward prompt is not received teaching material. The compact formula recap was retained in the visual-recall file and external source links were retained in the teaching volume.

*Continuation of [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Learning route|10 - From Uncertainty to Trustworthy Samples - Probability and Monte Carlo]]. This continuation integrates the supplied source material for Parts 966–1000 into connected chapters. Explanations, derivations, examples, and fenced source-code bodies are retained and grouped by the numerical question they answer. Repository inspections and claims of execution are marked as source-reported, not independently verified here.*

Before choosing an algorithm, ask what makes its answer trustworthy. A graphics pipeline may combine matrix transforms, interpolation, physical integration, root finding, optimization, sampling, and statistical estimates. Each stage can introduce error. Correct algebra can still yield an inaccurate computation, while a precise computation can faithfully solve an inaccurate approximation. Numerical reliability means keeping those failure modes distinct.

The numerical range here develops that question through error propagation, conditioning, stability, interpolation, root finding, error bounds, discretization, sampling, and reproducible testing. The code laboratories named by the supplied source are KairoMath statistics and optimization and KairoRayTracer sampling; any repository-specific observations below remain source-reported until independently inspected.

**Coverage:** Parts 966–975 (floating-point error, conditioning, stability, and stable arithmetic); 976–983 (interpolation and curves); 984–988 (root finding and error bounds); 989–992 (discretization, truncation, roundoff, and convergence); 993–994 (stable probability weights); 995–998 (sampling designs); 999–1000 (benchmarking and numerical tests).



<!-- end V10.16 -->

<!-- begin V10.24 -->
## Repository observations reported by the supplied source · V10.24.01

> The following project inspections and implementation claims are reported by the supplied source. They have not been independently rechecked in this staging pass.


The current code inspection identifies several relevant engineering directions.

### 37.1 KairoMath statistics

[KairoMath/Statistics.cppm](https://github.com/swayam8624/KairoMath/blob/master/Statistics.cppm?utm_source=chatgpt.com) currently computes covariance by first calculating feature means and then accumulating centred cross-products.

This is preferable to a naive covariance formula based exclusively on subtracting raw second moments, but it is still a floating-point implementation whose accuracy depends on the data and summation method.

For extremely large or poorly scaled datasets, compensated reductions, pairwise accumulation, or mergeable online covariance can be useful improvements.

The file also computes correlation by dividing covariance values by standard deviations, and uses a zero-value policy for zero-variance features.

As discussed in the earlier statistics chapter, zero correlation is mathematically undefined when a standard deviation is zero; the chosen software policy should be clearly documented.

### 37.2 KairoMath optimization

[KairoMath/Optimization.cppm](https://github.com/swayam8624/KairoMath/blob/master/Optimization.cppm?utm_source=chatgpt.com) implements backtracking line search that returns a zero step when it cannot find an acceptable step above the configured minimum.

A caller can then observe a zero update and satisfy a step-tolerance stopping condition.

This creates a risk of treating line-search failure as convergence.

This numerical framework helps correct that behavior: distinguish successful convergence, a posteriori error evidence, numerical stagnation, and algorithmic failure.

An explicit `LineSearchFailed` status is more informative than returning zero as though it were an accepted step.

### 37.3 KairoRayTracer sampler

[KairoRayTracer/Sampler.cppm](https://github.com/swayam8624/KairoRayTracer/blob/main/Sampler.cppm?utm_source=chatgpt.com) implements hash-derived jitter and a square-grid stratification scheme.

As established in the earlier sampling discussion, a non-perfect-square sample count can leave part of the pixel sampling domain unused under the current row-major grid assignment.

The stratification proof explains why this matters mathematically.

The correct estimator must sample the full target domain according to an appropriately weighted design.

The laboratory's arbitrary-count Latin-hypercube construction provides one possible alternative.

It is not necessarily the optimal final renderer sampler. That decision requires benchmark comparisons against Sobol-family, randomized stratified, and other relevant techniques.


We can now formulate a rigorous research objective that connects the major ideas in this subject.

Consider a stochastic graphics or reconstruction pipeline with the following stages:

$$ \boxed{ \text{Scene model} \rightarrow \text{Discretization} \rightarrow \text{Numerical evaluation} \rightarrow \text{Sampling} \rightarrow \text{Estimator} \rightarrow \text{Validation} }. $$

A scientifically useful evaluation separates four questions.

Mathematical correctness: Does the algorithm compute the intended mathematical quantity under its assumptions?

Numerical stability: Does the finite-precision implementation unnecessarily amplify rounding errors?

Statistical correctness: Does the sampling and weighting scheme produce the intended estimator expectation and variance?

Experimental reproducibility: Can another run reconstruct the result, and do repeated trials support the claimed improvement?

A concrete benchmark could compare naive and stable quadratic intersection solvers, ordinary versus compensated radiance accumulation, different interpolation schemes on analytically known functions, and independent versus stratified or randomized low-discrepancy sampling.

For each experiment, construct an analytical oracle where possible.

Vary the problem scale, grid resolution, sample count, numeric precision, and random seed.

Report forward error, residual or backward-error proxies, observed convergence order, runtime, memory cost, and statistical uncertainty.

The most valuable result would not be a single universal winner. It would be an evidence-backed map of which algorithm fails, why it fails, and under which conditions an alternative succeeds.

That is the foundation of numerical-method research rather than merely numerical-method usage.


To confirm genuine mastery, you should now be able to reconstruct every major result independently.

Reconstruct the floating-point model and explain its domain of validity; construct examples of small backward error with large forward error; derive condition numbers; explain the difference between conditioning and stability; derive compensated and pairwise summation; implement a scaled norm; derive the stable quadratic formula through Vieta's relations; derive linear, bilinear, trilinear, cubic Hermite, Catmull–Rom, and Bézier interpolation; and prove the convergence mechanisms of bisection, secant, and Newton root finding.

Then derive contraction-based error bounds; distinguish discretization, truncation, and roundoff; estimate convergence order using mesh refinement; derive Richardson extrapolation; implement stable weight normalization and log-sum-exp; prove Fisher–Yates uniformity and stratified estimator unbiasedness; construct Latin-hypercube and Halton samples; and design controlled tests that distinguish algorithmic failure from stochastic variability.

A strong implementation test is to build a local laboratory so that every algorithm reports a structured result containing its error estimate, failure status, and relevant numerical diagnostics.

For stochastic algorithms, add repeated independent trials and measured confidence intervals rather than treating one random sample as a definitive result.


<!-- end V10.24 -->

<!-- begin V10.26 -->
## Linked numerical concepts

[[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Floating-point answers: error, conditioning, stability, and stable arithmetic|Algorithmic stability]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Floating-point answers: error, conditioning, stability, and stable arithmetic|Stable quadratic root evaluation]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reconstructing values and curves between samples|Interpolation versus approximation]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reconstructing values and curves between samples|Tensor-product interpolation]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reconstructing values and curves between samples|Cubic Hermite interpolation]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reconstructing values and curves between samples|Bézier curves and De Casteljau evaluation]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Finding roots with guarantees and convergence checks|Bracketed and open root solvers]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Finding roots with guarantees and convergence checks|A posteriori error bounds for contractive iteration]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#Separate discretization, truncation, and roundoff; measure convergence|Discretization, truncation, and roundoff]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#Separate discretization, truncation, and roundoff; measure convergence|Convergence order and Richardson extrapolation]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Keep probability weights stable before sampling|Stable probability normalization and log-sum-exp]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Design samples: without replacement, stratified, and low-discrepancy|Uniform shuffling without replacement]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Design samples: without replacement, stratified, and low-discrepancy|Latin-hypercube sampling]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#Separate discretization, truncation, and roundoff; measure convergence|Numerical error budgets]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Test numerical methods and benchmark reproducibly|Reproducible numerical benchmarking]]

<!-- end V10.26 -->

<!-- begin V11.21 -->
## Linked camera concepts

[[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Recover geometry from depth and construct screen-space rays|Axial depth and Euclidean range]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Recover geometry from depth and construct screen-space rays|Hardware depth-buffer linearization]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Recover geometry from depth and construct screen-space rays|Position reconstruction from depth]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Recover geometry from depth and construct screen-space rays|Screen-space ray generation and mouse picking]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Build projection and stereo geometry around real camera baselines|Off-axis stereo projection and disparity]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Represent directions over the full sphere|Omnidirectional camera projections]]


This supplement extends the camera material. It assumes the existing convention choices for coordinates, projection, and image sampling; symbols below are local to each section. The main theme is that a camera pipeline produces more than coordinates. It also makes discrete decisions, estimates unknown parameters from incomplete evidence, and maps finite pixel regions into geometric regions.


<!-- end V11.21 -->

<!-- begin V11.26 -->
## Relationship to existing Phase 11 material

These additions attach to existing topics rather than duplicating them: robust predicates extend numerical error and clipping; calibration observability extends intrinsics/extrinsics and lens distortion; ray footprints extend screen-space ray generation and camera sampling. The TUM video index remains a lead for a later captioned study, not an evidence source here.

Two reconstructions can fit exactly the same image measurements and still describe different 3D worlds. The reason is that cameras measure directions and relative alignments; they do not automatically provide a ruler, and an uncalibrated camera does not even provide the same Euclidean coordinate structure. A useful reconstruction method therefore begins by asking three questions: which observations are being fitted, which physical quantities are held fixed, and what transformations leave every observation unchanged?


<!-- end V11.26 -->

<!-- begin V11.31 -->
## Evidence and integration

This addition is grounded in the full caption reads for playlist positions 8–11 (review files `V08.md`–`V11.md`). It consolidates and clarifies lecture derivations rather than claiming a new algorithm. The lecture captions contain notation slips and skipped proofs; formulas above state their conventions explicitly and should be cross-checked against the existing Phase 11 convention before integration. Link from the current epipolar/essential-matrix, triangulation, homography, and multi-view chapters. Do not duplicate their introductory definitions; use these passages as an ambiguity-and-model-selection bridge.
## External course legend and evidence boundary

The selected course is **Multiple View Geometry, Professor Daniel Cremers, TUM/cvprtum**, [playlist](https://www.youtube.com/playlist?list=PLTBdjV_4f-EJn6udZ34tht9EVIW7lbeo4). All **17 playlist positions** are accounted for: **13 full English caption tracks** were read sequentially, and **four caption-blocked positions** use full-text reads of official course-chapter PDF companions. Companion coverage is explicitly different from access to all spoken content or video frames. The chapters above combine overlapping geometry with the supplied curriculum and develop genuinely new links in the research sections.

| Position | Video ID / link | Caption evidence | Actual caption or companion scope read | Novelty decision |
|---|---|---|---|---|
| 1 | `RDkwklFGMfo` [Lecture 1](https://www.youtube.com/watch?v=RDkwklFGMfo) | Full English auto-caption VTT and normalized text saved (11,950 words). Full sequential text read. | Vector spaces, bases, inner product/norm/metric; linear maps as matrices; groups and homogeneous affine/rigid transforms; range/kernel/rank and solvability. | Foundation recap overlaps existing linear algebra chapters. Camera link: homogeneous coordinates encode pose; rank/nullspace expose ambiguity in geometric systems. No separate camera expansion. See `reviews/tum-lecture-01-02-03-06.md`. |
| 2 | `6VbbYXpBIqA` [Lecture 2](https://www.youtube.com/watch?v=6VbbYXpBIqA) | Full English auto-caption VTT and normalized text saved (11,286 words). Full sequential text read. | Eigenstructure; symmetric/PSD matrices; norms and $A^TA$; skew matrices/cross products; SVD derivation and ellipsoid action; pseudoinverse minimum-norm least squares; reconstruction assumptions. | General conditioning insight overlaps existing math notes; lecturer cautions minimum norm is application-dependent. No new camera-specific derivation. See `reviews/tum-lecture-01-02-03-06.md`. |
| 3 | `khLM8VV8LuM` [Lecture 3](https://www.youtube.com/watch?v=khLM8VV8LuM) | Full English auto-caption VTT and normalized text saved (11,873 words). Full sequential text read. | Euclidean points vs coordinates; cross/triple products; rigid motion from preserved oriented orthonormal frames; $SO(3)$ tangent algebra, exponential/logarithm, Rodrigues formula; six-parameter $SE(3)$ twist. | Existing notes cover Lie pose coordinates. Review emphasizes invariant-based proof and that the algebra is tangent space, not a flat replacement for the group. See `reviews/tum-lecture-01-02-03-06.md`. |
| 4 | `IM1lXkhaIZc` [Lecture 4](https://www.youtube.com/watch?v=IM1lXkhaIZc) | English caption attempt failed with HTTP 429; one later single `android`-client retry also failed. | **Full companion text read:** official TUM Chapter 2, *Representing a Moving Scene*, all 26 extracted pages. Rigid motion, $SO(3)$/$SE(3)$, exponential/log maps, Rodrigues, twists, camera-frame velocity, adjoint and Euler-style local coordinates. | Companion supports core pose subject matter; it is not a transcript/video review. Exact added depth: twist velocity and adjoint frame transformation. See `reviews/tum-blocked-companion-pdfs.md`. |
| 5 | `H6WEt3xOyPU` [Lecture 5](https://www.youtube.com/watch?v=H6WEt3xOyPU) | English caption attempt failed with HTTP 429; one later single `android`-client retry also failed. | **Full companion text read:** official TUM Chapter 3, *Perspective Projection*, all 24 extracted pages. Pinhole/intrinsic matrix, spherical projection, radial distortion, point/line preimage and coimage, homogeneous/projective coordinates. | Companion overlaps camera/projection material; radial distortion and preimage/coimage are useful exact subtopics. Not video-specific evidence. See `reviews/tum-blocked-companion-pdfs.md`. |
| 6 | `WCyKnuhM1CE` [Lecture 6](https://www.youtube.com/watch?v=WCyKnuhM1CE) | Full English auto-caption VTT and normalized text saved (7,970 words). Full sequential text read. | Pixels to sparse matches; sparse/dense; texture can mask poor geometry; baseline tracking/triangulation tradeoff and drift; wide-baseline ambiguity, occlusion and nonrigidity; optical flow vs 3D motion; brightness constancy and local affine motion. | Existing notes cover flow basics. Additional value: occlusion, approximate intensity assumption, and local affine field as a proxy for perspective motion. See `reviews/tum-lecture-01-02-03-06.md`. |
| 7 | `5Ih3CwoSw_I` [Lecture 7](https://www.youtube.com/watch?v=5Ih3CwoSw_I) | Full English auto-caption track fully read (13,724 words). | Brightness constancy, differential flow constraint, aperture problem, Lucas–Kanade least squares, structure tensor rank/eigenvalues, contrast/threshold sensitivity, KLT drift, wide-baseline failures, normalized cross-correlation, and direct-method preview. Full review in `reviews/tum-lecture-07-12c.md`. | Reused narrowly for the explicit eigenvalue-conditioning connection in `camera-geometry-depth.md`; much of optical flow overlaps existing notes. |
| 8 | `9fvopDHdrFg` [Lecture 8](https://www.youtube.com/watch?v=9fvopDHdrFg) | Full English auto-caption VTT and normalized text saved (11,871 words); full sequential read. VTT SHA-256 `9d01b27672629c72520fff340fe3864a2f2cfbdd0ca98f8191466fc596bd4253`; text SHA-256 `a7525c441a06c84ef57648b13f7e9eccec79bbb2beb1b69d41f7bb86fac428d4`. | Two-view reconstruction setup: camera motion and scene structure from matched observations; discusses projection, rigid motion and correspondence assumptions before reconstruction. Full read in `reviews/V08.md`. | Overlaps projective/camera and correspondence chapters; no separate expansion. |
| 9 | `lQE5HgPFQ4M` [Lecture 9](https://www.youtube.com/watch?v=lQE5HgPFQ4M) | Full English auto-caption VTT and normalized text saved (9,724 words); full sequential read. VTT SHA-256 `ea4ab578ae57c6b73c23cf156d0edc35adee08016784157bfad500352dac73f0`; text SHA-256 `1a3ee51928d3aea0cfafa7e811850578f66d2399f5f5f501c3eb9fd4bdd6936c`. | Eight-point method continuation; epipolar constraints, essential/fundamental matrices, depths and relative camera geometry. Full read in `reviews/V09.md`. | Core two-view geometry is already in the Phase 11 narrative; no separate expansion. |
| 10 | `kjnMPg3Eo5w` [Lecture 10](https://www.youtube.com/watch?v=kjnMPg3Eo5w) | Full English auto-caption VTT and normalized text saved (5,688 words); full sequential read. VTT SHA-256 `c8b76754c06162adbd84c2af2e4b2b0ed6f074de201feb4f28887c97b9e4e3d1`; text SHA-256 `6267828662e0c2e5a150d0543de5104b9eca8d81385800edb9a3624c3f63f967`. | Uncalibrated reconstruction and homography/projective ambiguity; camera calibration matrix and projective transformations. Full read in `reviews/V10.md`. | Existing notes already distinguish calibrated and uncalibrated/projective cases; no separate expansion. |
| 11 | `Y-5gPheGSCc` [Lecture 11](https://www.youtube.com/watch?v=Y-5gPheGSCc) | Full English auto-caption VTT and normalized text saved (10,834 words); full sequential read. VTT SHA-256 `be12a660cb0315396c8f2e469e2d9833bda2d1da50020d736cc7082e48b15181`; text SHA-256 `4053402f7b12fe269fb2dca5fa016608017b603ff4f4de62fa7173e436215af1`. | Multiple-view point/line rank constraints, shared depth compatibility, rank cases and degenerate configurations. Full read in `reviews/V11.md`. | New joint-consistency teaching expansion in `camera-geometry-depth.md`; transcript proof-boundary and caption caveats retained in review. |
| 12a | `712JQ1WdjrM` [Lecture 12a](https://www.youtube.com/watch?v=712JQ1WdjrM) | Full English auto-caption VTT and normalized text saved (6,642 words). Full sequential text read. | Multiview point rank/preimage cases; scale gauge and rank-11 nullspace solve; alternating structure/motion limits; multiview line rank constraints, trilinear constraints, degeneracy and triplet sufficiency. | Existing notes cover point scale and BA. Additional niche derivation explains line degeneracy when translation lies in the preimage plane and reduction to image triplets. See `reviews/tum-lecture-12a-12b.md`. |
| 12b | `UTcFkJj91JI` [Lecture 12b](https://www.youtube.com/watch?v=UTcFkJj91JI) | Full English auto-caption VTT and normalized text saved (10,484 words). Full sequential text read. | Noisy observations/outlier distinction; MAP/ML and Gaussian reprojection loss; visibility masks and gauge reference; alternate BA parameterizations; covariance-weighted and iterative reweighted LS; gradient descent, step size, anisotropy and LS terminology. | Deepens existing BA with objective provenance, missing-observation handling, reference-frame choice and explicit nonconvex/global-optimality limits. See `reviews/tum-lecture-12a-12b.md`. |
| 12c | `ZK3eFPpc4KE` [Lecture 12c](https://www.youtube.com/watch?v=ZK3eFPpc4KE) | Full English auto-caption track fully read (10,760 words). | Reprojection error, nonlinear least squares, local minima and initialization, Gauss–Newton $J^TJ$, Levenberg–Marquardt damping, sparse/dense coupling, SfM and direct methods. Full review in `reviews/tum-lecture-07-12c.md`. | Reused for the bounded solver insight in `camera-geometry-depth.md`; calibration observability remains an original extension, not a lecture claim. |
| 12d | `EojzvJeZyo0` [Lecture 12d](https://www.youtube.com/watch?v=EojzvJeZyo0) | English caption attempt failed with HTTP 429; one `android`-client request also failed. | **Full companion text read:** official TUM Chapter 8, *Direct Approaches to Visual SLAM*, all 33 extracted pages. Direct dense depth/tracking, photometric+geometric RGB-D residuals, robust weights, loop closure, $Sim(3)$, photometric calibration, windowed optimization and Schur complement. | Companion only; blocked video is not transcript-reviewed. Added value: brightness/irradiance calibration and joint optimization/marginalization details. See `reviews/tum-blocked-companion-pdfs.md`. |
| 13 | `moYCUzmI3HQ` [Lecture 13](https://www.youtube.com/watch?v=moYCUzmI3HQ) | Full caption track saved. VTT header says `Language: en`; filename contains `.ar.en` due extractor track labeling. Normalized text: 10,736 words. Full sequential text read. | Sparse-to-dense motivation; functionals/objectives; denoising data and smoothness terms; Euler–Lagrange assumptions; gradient flow/PDE and explicit Euler; quadratic blur, total variation and convexity tradeoff; Perona–Malik vs ROF distinction. | General variational foundations only; relevant to dense reconstruction context but not camera-specific. See `reviews/tum-lecture-13.md`. |
| 14 | `jYgxnnu4oTc` [Lecture 14](https://www.youtube.com/watch?v=jYgxnnu4oTc) | English caption request failed with HTTP 429; no repeated language-variant requests. | **Full companion text read:** official TUM Chapter 10, *Variational Multiview Reconstruction*, all 20 extracted pages. Explicit/implicit shape, photoconsistency, empty-set shrinking bias, silhouette-ray constraints, convex relaxation/thresholding, super-resolution texture and space-time reconstruction. Chapter 9 remains context only, not a reviewed fallback. | Companion only; video not transcript-reviewed. The empty-set pathology and silhouette constraint are precise, useful topics; not new camera calibration. See `reviews/tum-blocked-companion-pdfs.md`. |
| File | Official title | Pages | PDF SHA-256 | Use |
|---|---|---:|---|---|
| `companions/chapter2.pdf` | Chapter 2 - Representing a Moving Scene | 26 | `7a368e92481413ab196cd9ff5709da04fee45017e083e7808332a2cec4c79ae5` | Fallback for Lecture 4 |
| `companions/chapter3.pdf` | Chapter 3 - Perspective Projection | 24 | `fdac3f543eff817f608c56c10e07e505529697d8779adc25ef2ce964c8b5f360` | Fallback for Lecture 5 |
| `companions/chapter8.pdf` | Chapter 8 - Direct Approaches to Visual SLAM | 33 | `aa23010fa2d4a3fbc9623456d749c388101581db8168e9b2c1ff80039a51254b` | Fallback for Lecture 12d |
| `companions/chapter9.pdf` | Chapter 9 - Variational Methods: A Short Intro | 13 | `e1025a2f9a6577f2734f66f004fed26d33b6f78184b256fb95461390a9d59cb7` | Context beside Lecture 14 fallback |
| `companions/chapter10.pdf` | Chapter 10 - Variational Multiview Reconstruction | 20 | `bc5b222acbe3b9c3787d5fe53599ded1c0cd0b8b0a38d46045b80bc714bb8936` | Fallback for Lecture 14 |
| Position | Video ID | VTT SHA-256 | Normalized text SHA-256 | Full-read review |
|---:|---|---|---|---|
| 1 | `RDkwklFGMfo` | `f9b7fe441fea55d23f08f5fd5f907192e3c4d7a3b2cf0df5e5beb1694df60f98` | `03194156572766c4694c31247027dce065f17326a1e3403d4d59ec74b915c8d9` | `reviews/tum-lecture-01-02-03-06.md` |
| 2 | `6VbbYXpBIqA` | `d2ebbd19d58dacf838b67f67afb5047d001c2f81fd6e55fa69ae35157a7defa8` | `97bf8944f145993ef3b83e4b420e6783aafd894fba217c8519f20b66eec88ecd` | `reviews/tum-lecture-01-02-03-06.md` |
| 3 | `khLM8VV8LuM` | `a00cbc6702b41ddc8a9e6cd98b1e5eaee8967a7a84e07383138f0acb0d0ae5bd` | `fcbdf390fbd31e26e0cb01443e35edd17b034c1ea6aa2258f76a0c42e1d794cd` | `reviews/tum-lecture-01-02-03-06.md` |
| 6 | `WCyKnuhM1CE` | `368f93ee99c4879aa8a8ca5993762915359a0ba0a8703fc423ec4bf0f4fcd758` | `231d32046841c57abec54f96f69763231d533b3f89898e2c01103ba1a12d9a7b` | `reviews/tum-lecture-01-02-03-06.md` |
| 7 | `5Ih3CwoSw_I` | `51ebfbb0b838c8a0bd2bc2916456713764f5167d241518d0673e71ac3d26c0e0` | `d0445acece458976541ecc3f4fb2bb4ed91036ffd736a374264b12118d7bf049` | `reviews/tum-lecture-07-12c.md` |
| 8 | `9fvopDHdrFg` | `9d01b27672629c72520fff340fe3864a2f2cfbdd0ca98f8191466fc596bd4253` | `a7525c441a06c84ef57648b13f7e9eccec79bbb2beb1b69d41f7bb86fac428d4` | `reviews/V08.md` |
| 9 | `lQE5HgPFQ4M` | `ea4ab578ae57c6b73c23cf156d0edc35adee08016784157bfad500352dac73f0` | `1a3ee51928d3aea0cfafa7e811850578f66d2399f5f5f501c3eb9fd4bdd6936c` | `reviews/V09.md` |
| 10 | `kjnMPg3Eo5w` | `c8b76754c06162adbd84c2af2e4b2b0ed6f074de201feb4f28887c97b9e4e3d1` | `6267828662e0c2e5a150d0543de5104b9eca8d81385800edb9a3624c3f63f967` | `reviews/V10.md` |
| 11 | `Y-5gPheGSCc` | `be12a660cb0315396c8f2e469e2d9833bda2d1da50020d736cc7082e48b15181` | `4053402f7b12fe269fb2dca5fa016608017b603ff4f4de62fa7173e436215af1` | `reviews/V11.md` |
| 12a | `712JQ1WdjrM` | `05e7812ae9503489b8c2dc9688b6d4e461b43e78431548be391b151d1730263f` | `17cbf55b68911ed39e8d087a1acab719c0d74f6671abe58f51e0aaeb126fb621` | `reviews/tum-lecture-12a-12b.md` |
| 12b | `UTcFkJj91JI` | `29c0faab12610ca70f8d6dc65442a741c8f588e1d426999e9268e94ef1e6b11e` | `b056f145aabddc0ac095f2f40387630a5058e445ff55c7a7485791a045637b3f` | `reviews/tum-lecture-12a-12b.md` |
| 12c | `ZK3eFPpc4KE` | `7056156fd36b1eff4001cfa5a664583c45c23267ab8814f7d6f6bb6efe232374` | `c7517421071e4e3ba8a7f5bc4e8a8dfe62c99a84f1e5012e99a729f331b3e2d6` | `reviews/tum-lecture-07-12c.md` |
| 13 | `moYCUzmI3HQ` | `705f28405fc9de638ce9307d9a59f9e941cb362cdaf5814811d8bc0adf4d012a` | `98c57681be506f4d946503fde807dca15bf48c086fc5fb4f24f88f14ba4edacd` | `reviews/tum-lecture-13.md` |

The preserved per-lecture reviews record definitions, assumptions, examples, overlap decisions, transcript line ranges and hashes. The course PDF fallback reviews record actual extracted page/text scope; their figures were not independently inspected. Primary references for the additional derivations include Shewchuk’s robust-predicate work and PBRT’s camera, texture-filtering and path-measure chapters. See the durable source ledger at `/Users/swayamsingal/.codex/source-cache/math-camera-completion-2026-10-28/scratch/external/source-ledger.md`.

### Current API contracts used by the camera derivations

A projection convention is an explicit contract between the matrix and pipeline state. Vulkan’s ordinary clip-depth inequalities are $0\le z_c\le w_c$; a supported depth-clip-control configuration can instead use the negative-one-to-one interval. Viewport depth mapping and depth comparison/clear state are separate choices. Always test the homogeneous inequalities before dividing: a ratio in range does not make a negative-$w_c$ point visible. These statements use the [Vulkan depth guide](https://docs.vulkan.org/guide/latest/depth.html) and [vertex post-processing specification](https://docs.vulkan.org/spec/latest/chapters/vertexpostproc.html), with the inequalities as the controlling geometric test.

OpenXR’s `XrFovf` members are **signed angles in radians**, not precomputed tangent values. For the ordinary ordered frustum, near-plane extents are $l=n\tan(\theta_l)$, $r=n\tan(\theta_r)$, $b=n\tan(\theta_d)$ and $t=n\tan(\theta_u)$. The application must also respect the runtime pose and any documented flipped view. The [current OpenXR field-of-view reference](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrFovf.html) defines those angle signs, units and flip cases.


<!-- end V11.32 -->

<!-- begin V11.33 -->
## Research concept links

[[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#A camera model must close the loop|Camera gauge freedoms and observability]] · [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#A camera model must close the loop|Ray differentials and camera sampling measures]]

### Primary research reference ledger

| ID | Source / author | URL | Evidence read | SHA-256 | Scope used |
|---|---|---|---|---|---|
| E01 | *Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates for Computational Geometry*, Jonathan Richard Shewchuk (1997), and author project page | https://www.cs.cmu.edu/~quake/robust.html | Paper abstract and project page text read; full paper PDF is linked there but was not downloaded in this scratch pass. **Primary technical source, read (page/abstract level)** | Not locally captured | Orientation/incircle decisions are determinant signs; roundoff near zero may reverse a decision; adaptive precision refines only when an error bound cannot certify the sign. The exposition below distinguishes predicates from approximate constructions and is an original synthesis, not a quotation. |
| E02 | *Camera Interface*, PBRT 4th edition | https://pbr-book.org/4ed/Cameras_and_Film/Camera_Interface | Full chapter web text read | Not locally captured | Camera sampling includes film position and shutter time; differential rays estimate nearby ray behavior; camera ray generation returns a weight, allowing camera models to encode nonuniform contribution. |
| E03 | *Texture Sampling and Antialiasing*, PBRT 4th edition | https://pbr-book.org/4ed/Textures_and_Materials/Texture_Sampling_and_Antialiasing | Full chapter web text read | Not locally captured | Camera differential rays separated by a pixel estimate screen-space footprint at a surface, supporting texture filtering; derivatives are propagated/solved at intersections. |
| E04 | *The Path-Space Measurement Equation*, PBRT 3rd edition | https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/The_Path-Space_Measurement_Equation | Full chapter web text read | Not locally captured | Perspective-camera sampling can be interpreted as importance sampling for the camera importance function; PDFs depend on the measure and camera model. Used only to motivate measure-aware sampling. |
| E05 | TUM Computer Vision Group, *Computer Vision II: Multiple View Geometry* online course | https://cvg.cit.tum.de/teaching/online/mvg | Official course page and the linked 17-entry playlist index inspected; 13 caption tracks fully read; four official companion PDFs fully text-read for blocked-caption positions. Per-position status, hashes and topics are in `playlist-review.md`. | See caption and companion hashes in `playlist-review.md` | Companion slides are not video transcripts. The selected 17 entries alone are reviewed; no channel-wide completeness claim. |
| V01 | *Multiple View Geometry - Lecture 7 (Prof. Daniel Cremers)*, cvprtum / TU München | https://www.youtube.com/watch?v=5Ih3CwoSw_I | Full English auto-caption track reviewed, 13,724 words after deduplicating overlapping VTT cues. **Read, full caption track** | VTT `51ebfbb0b838c8a0bd2bc2916456713764f5167d241518d0673e71ac3d26c0e0`; normalized text `d0445acece458976541ecc3f4fb2bb4ed91036ffd736a374264b12118d7bf049` | Optical flow / Lucas–Kanade: brightness constancy, aperture problem, least-squares structure tensor, rank/eigenvalue interpretation, thresholding and contrast scaling, KLT tracking drift, wide-baseline correspondence limits, affine patch motion, normalized cross-correlation, and direct methods preview. This overlaps existing optical-flow material; only the conditioning/rank interpretation is useful as a more explicit connection here. Auto-captions may contain recognition errors; formula-level claims were cross-checked against the existing notes and the original mathematical structure, not treated as perfect lecture transcription. |
| V02 | *Multiple View Geometry - Lecture 12c (Prof. Daniel Cremers)*, cvprtum / TU München | https://www.youtube.com/watch?v=ZK3eFPpc4KE | Full English auto-caption track reviewed, 10,760 words after deduplicating overlapping VTT cues. **Read, full caption track** | VTT `7056156fd36b1eff4001cfa5a664583c45c23267ab8814f7d6f6bb6efe232374`; normalized text `c7517421071e4e3ba8a7f5bc4e8a8dfe62c99a84f1e5012e99a729f331b3e2d6` | Bundle adjustment and nonlinear least squares: reprojection residuals, nonconvexity and local minima, Gauss–Newton's $J^TJ$ approximation, Levenberg–Marquardt damping, initialization, sparse-vs-dense coupling, and feature/direct reconstruction trade-offs. Captions also discuss results and algorithm history; those are not expanded here. |

The resource IDs V01/V02 in this research-reference table identify the two initially selected lectures (7 and 12c); playlist positions are recorded separately in the complete 17-position legend above.

<!-- end V11.33 -->


# Source map and concept graph

## Retained source sections

| Source | Section | In-book destination |
|---|---|---|
| V06 | Source orientation: 06 - From Movement to Meaning - Vectors, Geometry and Reliable Code | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#From Movement to Meaning: Vectors, Geometry, and Reliable Code|Open section]] |
| V06 | The panel gives us a reason to learn | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#The panel gives us a reason to learn|Open section]] |
| V06 | Numbers become a system of permitted operations | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Numbers become a system of permitted operations|Open section]] |
| V06 | An arrow is not the address written beside it | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#An arrow is not the address written beside it|Open section]] |
| V06 | A basis is a measuring arrangement, not the vector itself | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#A basis is a measuring arrangement, not the vector itself|Open section]] |
| V06 | The reachable movements form a vector space | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#The reachable movements form a vector space|Open section]] |
| V06 | A ruler needs more than coordinate components | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#A ruler needs more than coordinate components|Open section]] |
| V06 | A direction can exist even when the naive length overflows | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#A direction can exist even when the naive length overflows|Open section]] |
| V06 | The dot product turns two movements into a measurement | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#The dot product turns two movements into a measurement|Open section]] |
| V06 | Projection separates useful movement from sideways movement | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Projection separates useful movement from sideways movement|Open section]] |
| V06 | Rotation first becomes understandable in a plane | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Rotation first becomes understandable in a plane|Open section]] |
| V06 | Oriented area tells us which side of the panel is front | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Oriented area tells us which side of the panel is front|Open section]] |
| V06 | Signed volume extends the same idea into space | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Signed volume extends the same idea into space|Open section]] |
| V06 | An angle is a question with a domain | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#An angle is a question with a domain|Open section]] |
| V06 | Surface movement becomes a local frame | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Surface movement becomes a local frame|Open section]] |
| V06 | Orthonormal frames make reconstruction simple | [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#Orthonormal frames make reconstruction simple|Open section]] |
| V06 | Light uses the same decomposition in a new role | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Light uses the same decomposition in a new role|Open section]] |
| V06 | The sensor needs coordinates inside the triangle | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#The sensor needs coordinates inside the triangle|Open section]] |
| V06 | Weighted points lead from affine geometry to collision shapes | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Weighted points lead from affine geometry to collision shapes|Open section]] |
| V07 | Source orientation: 07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#From Basis Images to Moving Worlds: Matrices, Transforms and Quaternions|Open section]] |
| V07 | Keep the promises visible while moving between representations | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Keep the promises visible while moving between representations|Open section]] |
| V07 | The robot needs a rule for moving every direction | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#The robot needs a rule for moving every direction|Open section]] |
| V07 | A linear rule is determined by its basis images | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#A linear rule is determined by its basis images|Open section]] |
| V07 | Keep the transformation separate from its storage | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Keep the transformation separate from its storage|Open section]] |
| V07 | Combining rules explains the multiplication law | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Combining rules explains the multiplication law|Open section]] |
| V06 | A Gram matrix remembers the geometry of a slanted basis | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#A Gram matrix remembers the geometry of a slanted basis|Open section]] |
| V07 | Special matrices tell us what information a rule preserves | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Special matrices tell us what information a rule preserves|Open section]] |
| V07 | Derive rotations from the directions that stay fixed | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Derive rotations from the directions that stay fixed|Open section]] |
| V07 | Transpose lets us compare measurements and structure | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Transpose lets us compare measurements and structure|Open section]] |
| V07 | Determinant measures oriented volume and exposes lost dimensions | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Determinant measures oriented volume and exposes lost dimensions|Open section]] |
| V07 | Recovering an input has an algebraic route and a computational route | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Recovering an input has an algebraic route and a computational route|Open section]] |
| V07 | An inverse can exist and still amplify uncertainty | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#An inverse can exist and still amplify uncertainty|Open section]] |
| V06 | A normal is a measuring rule before it is an arrow | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#A normal is a measuring rule before it is an arrow|Open section]] |
| V06 | Oriented areas deserve their own representation | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Oriented areas deserve their own representation|Open section]] |
| V06 | Homogeneous coordinates preserve the role of a location | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Homogeneous coordinates preserve the role of a location|Open section]] |
| V07 | The origin exposes the limit of an ordinary linear map | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#The origin exposes the limit of an ordinary linear map|Open section]] |
| V07 | A frame combines directions with a place to stand | [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#A frame combines directions with a place to stand|Open section]] |
| V07 | Coordinates can change while the geometric object stays still | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Coordinates can change while the geometric object stays still|Open section]] |
| V07 | A camera is another frame, so viewing uses its inverse | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A camera is another frame, so viewing uses its inverse|Open section]] |
| V07 | A normal transforms as a measurement that must still vanish on tangents | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A normal transforms as a measurement that must still vanish on tangents|Open section]] |
| V07 | Orientation needs a state that can be composed | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Orientation needs a state that can be composed|Open section]] |
| V07 | The sandwich turns half angles into a full rotation | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#The sandwich turns half angles into a full rotation|Open section]] |
| V07 | Composition remembers which frame a new turn belongs to | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Composition remembers which frame a new turn belongs to|Open section]] |
| V07 | A matrix and a quaternion should agree on every basis direction | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A matrix and a quaternion should agree on every basis direction|Open section]] |
| V07 | Animation asks for a path, not merely an intermediate tuple | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Animation asks for a path, not merely an intermediate tuple|Open section]] |
| V07 | Euler coordinates remain useful when their chart is explicit | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Euler coordinates remain useful when their chart is explicit|Open section]] |
| V07 | Rigid motion adds translation without adding scale or shear | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Rigid motion adds translation without adding scale or shear|Open section]] |
| V07 | A transform type must say what its stored fields mean | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A transform type must say what its stored fields mean|Open section]] |
| V07 | A cached matrix is a dependency promise | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A cached matrix is a dependency promise|Open section]] |
| V08 | Source orientation: 08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#From Constraints to Spectra: Systems, Decompositions, and Matrix Functions|Open section]] |
| V08 | The graphics problem hiding inside a matrix equation | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#The graphics problem hiding inside a matrix equation|Open section]] |
| V08 | A linear equation is a measurement, and several equations are geometry | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A linear equation is a measurement, and several equations are geometry|Open section]] |
| V08 | A system is an intersection of constraints | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A system is an intersection of constraints|Open section]] |
| V08 | Rank separates what the measurements determine from what they leave free | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Rank separates what the measurements determine from what they leave free|Open section]] |
| V08 | Rank, the four spaces, and the shape of the answer | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Rank, the four spaces, and the shape of the answer|Open section]] |
| V08 | Matrix norms give scale to residuals and error claims | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Matrix norms give scale to residuals and error claims|Open section]] |
| V08 | Matrix norms put scale into error statements | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Matrix norms put scale into error statements|Open section]] |
| V08 | Elimination turns the geometry into an auditable calculation | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Elimination turns the geometry into an auditable calculation|Open section]] |
| V08 | Row operations preserve equations for a precise reason | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Row operations preserve equations for a precise reason|Open section]] |
| V08 | REF and RREF expose pivots and free variables | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#REF and RREF expose pivots and free variables|Open section]] |
| V08 | Elimination multipliers become stored factor data | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Elimination multipliers become stored factor data|Open section]] |
| V08 | Pivoting separates algorithm failure from matrix failure | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Pivoting separates algorithm failure from matrix failure|Open section]] |
| V08 | Triangular solves perform the cheap second half | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#Triangular solves perform the cheap second half|Open section]] |
| V08 | LU and Doolittle make elimination reusable | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#LU and Doolittle make elimination reusable|Open section]] |
| V08 | LUP carries the row permutation through the solve | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#LUP carries the row permutation through the solve|Open section]] |
| V08 | A runnable pivoted LU reference preserves the elimination history | [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#A runnable pivoted LU reference preserves the elimination history|Open section]] |
| V08 | Structure makes a decomposition a better tool than generic elimination | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Structure makes a decomposition a better tool than generic elimination|Open section]] |
| V08 | Positive definiteness is a structural promise | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Positive definiteness is a structural promise|Open section]] |
| V08 | LDLT reaches symmetric indefinite matrices | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#LDLT reaches symmetric indefinite matrices|Open section]] |
| V08 | Orthogonal coordinates preserve the shape of the problem | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Orthogonal coordinates preserve the shape of the problem|Open section]] |
| V08 | Reflections and plane rotations build QR without losing lengths | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Reflections and plane rotations build QR without losing lengths|Open section]] |
| V08 | QR turns least squares into a triangular solve plus an irreducible residual | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#QR turns least squares into a triangular solve plus an irreducible residual|Open section]] |
| V08 | Put the algorithms in a graphics context | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Put the algorithms in a graphics context|Open section]] |
| V08 | Eigenvectors reveal the directions a square map does not turn | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Eigenvectors reveal the directions a square map does not turn|Open section]] |
| V08 | Eigenvalues, eigenspaces, and the two kinds of multiplicity | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Eigenvalues, eigenspaces, and the two kinds of multiplicity|Open section]] |
| V08 | Diagonalization leads to the stronger symmetric spectral theorem | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Diagonalization leads to the stronger symmetric spectral theorem|Open section]] |
| V08 | Iteration extracts selected eigenpairs without solving a polynomial | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Iteration extracts selected eigenpairs without solving a polynomial|Open section]] |
| V08 | QR iteration and Jacobi rotations preserve the spectrum while simplifying shape | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#QR iteration and Jacobi rotations preserve the spectrum while simplifying shape|Open section]] |
| V08 | Residuals distinguish backward accuracy from forward certainty | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Residuals distinguish backward accuracy from forward certainty|Open section]] |
| V08 | Sensitivity depends on left and right eigenvectors | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Sensitivity depends on left and right eigenvectors|Open section]] |
| V08 | Why eigenvectors, coordinates, and condition numbers connect | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Why eigenvectors, coordinates, and condition numbers connect|Open section]] |
| V09 | Source orientation: 09 - From Local Change to Reliable Motion - Calculus and Optimization | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#From Local Change to Reliable Motion: Calculus and Optimization|Open section]] |
| V09 | Functions, limits, continuity, and scalar change | [[Chapter Edition/01 - Computing Foundations/20 - Choose Matrix Factorizations from Structure#Functions, limits, continuity, and scalar change|Open section]] |
| V09 | Multivariable derivatives and sensitivity | [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Multivariable derivatives and sensitivity|Open section]] |
| V09 | Jacobians, curvature, and local approximation | [[Chapter Edition/01 - Computing Foundations/21 - Multivariable Derivatives and Sensitivity#Jacobians, curvature, and local approximation|Open section]] |
| V09 | Integration and quadrature | [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Integration and quadrature|Open section]] |
| V09 | Numerical differentiation and automatic differentiation | [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#Numerical differentiation and automatic differentiation|Open section]] |
| V10 | Source orientation: 10 - From Uncertainty to Trustworthy Samples - Probability and Monte Carlo | [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability#From Uncertainty to Trustworthy Samples: Probability and Monte Carlo|Open section]] |
| V10 | 1. A probability model begins with possible outcomes | [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#1. A probability model begins with possible outcomes|Open section]] |
| V10 | 2. Random quantities and their probability laws | [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#2. Random quantities and their probability laws|Open section]] |
| V10 | 3. Expected values describe centers; covariance describes dependence | [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#3. Expected values describe centers; covariance describes dependence|Open section]] |
| V10 | 4. Discrete and continuous distribution families | [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#4. Discrete and continuous distribution families|Open section]] |
| V10 | 5. Waiting times, event counts, and covariance geometry | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#5. Waiting times, event counts, and covariance geometry|Open section]] |
| V08 | Why a matrix factorization is a change in how we see the map | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Why a matrix factorization is a change in how we see the map|Open section]] |
| V08 | Singular value decomposition covers every rectangular map | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Singular value decomposition covers every rectangular map|Open section]] |
| V08 | The SVD equation as independent input-output channels | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#The SVD equation as independent input-output channels|Open section]] |
| V08 | Deriving singular directions through a Gram matrix | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Deriving singular directions through a Gram matrix|Open section]] |
| V08 | Full, thin, and compact shapes for tall and wide matrices | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Full, thin, and compact shapes for tall and wide matrices|Open section]] |
| V08 | Rank, both nullspaces, and the four fundamental subspaces | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Rank, both nullspaces, and the four fundamental subspaces|Open section]] |
| V08 | A concrete rotation, stretch, and coordinate trace | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#A concrete rotation, stretch, and coordinate trace|Open section]] |
| V08 | Pseudoinverse makes least squares and minimum norm one operation | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Pseudoinverse makes least squares and minimum norm one operation|Open section]] |
| V08 | The Moore–Penrose inverse and its four identities | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#The Moore–Penrose inverse and its four identities|Open section]] |
| V08 | Least squares from decoupling, then minimum norm | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Least squares from decoupling, then minimum norm|Open section]] |
| V08 | Conditioning: weak singular directions amplify uncertainty | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Conditioning: weak singular directions amplify uncertainty|Open section]] |
| V08 | Low-rank approximation and why the largest directions win | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Low-rank approximation and why the largest directions win|Open section]] |
| V08 | Thresholding and regularization are different choices | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Thresholding and regularization are different choices|Open section]] |
| V08 | SVD in a graphics sensitivity picture | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#SVD in a graphics sensitivity picture|Open section]] |
| V08 | Why the pseudoinverse identities follow from projections | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Why the pseudoinverse identities follow from projections|Open section]] |
| V08 | PCA turns a cloud of points into measured variation | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#PCA turns a cloud of points into measured variation|Open section]] |
| V08 | Centered measurements reveal principal directions | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Centered measurements reveal principal directions|Open section]] |
| V08 | Plane fits, eigengaps, and interpretation of uncertain components | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Plane fits, eigengaps, and interpretation of uncertain components|Open section]] |
| V08 | Polar decomposition separates rotation-like motion from stretch | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Polar decomposition separates rotation-like motion from stretch|Open section]] |
| V08 | Polar decomposition separates frame dependent stretch from orientation | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Polar decomposition separates frame dependent stretch from orientation|Open section]] |
| V08 | Two-sided rotations, nearest proper rotation, and nonunique directions | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#Two-sided rotations, nearest proper rotation, and nonunique directions|Open section]] |
| V08 | How numerical software computes and checks an SVD | [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition#How numerical software computes and checks an SVD|Open section]] |
| V09 | ODEs and numerical simulation | [[Chapter Edition/01 - Computing Foundations/25 - Continuous Models and Numerical Error#ODEs and numerical simulation|Open section]] |
| V08 | Matrix functions turn repeated linear actions into one operator | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Matrix functions turn repeated linear actions into one operator|Open section]] |
| V08 | The exponential solves a linear differential equation | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#The exponential solves a linear differential equation|Open section]] |
| V08 | Logarithms and square roots need domains and branches | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Logarithms and square roots need domains and branches|Open section]] |
| V08 | Matrix powers range from repeated action to fractional functions | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Matrix powers range from repeated action to fractional functions|Open section]] |
| V08 | Series define functions but do not guarantee a good algorithm | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Series define functions but do not guarantee a good algorithm|Open section]] |
| V09 | Optimization and inverse problems | [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Optimization and inverse problems|Open section]] |
| V09 | Reliable optimization in practice | [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reliable optimization in practice|Open section]] |
| V10 | Floating-point answers: error, conditioning, stability, and stable arithmetic | [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Floating-point answers: error, conditioning, stability, and stable arithmetic|Open section]] |
| V10 | Reconstructing values and curves between samples | [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Reconstructing values and curves between samples|Open section]] |
| V10 | Finding roots with guarantees and convergence checks | [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding#Finding roots with guarantees and convergence checks|Open section]] |
| V10 | Separate discretization, truncation, and roundoff; measure convergence | [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#Separate discretization, truncation, and roundoff; measure convergence|Open section]] |
| V10 | 6. A computer produces deterministic pseudorandom sequences | [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#6. A computer produces deterministic pseudorandom sequences|Open section]] |
| V10 | 7. Transformations turn simple inputs into useful samples | [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#7. Transformations turn simple inputs into useful samples|Open section]] |
| V10 | 8. Monte Carlo estimates trade computation for uncertainty | [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#8. Monte Carlo estimates trade computation for uncertainty|Open section]] |
| V10 | 9. Finite samples need stable descriptive statistics | [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#9. Finite samples need stable descriptive statistics|Open section]] |
| V10 | 10. Inference and error metrics need explicit assumptions | [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#10. Inference and error metrics need explicit assumptions|Open section]] |
| V10 | 11. Implement probability without losing its numerical contracts | [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#11. Implement probability without losing its numerical contracts|Open section]] |
| V10 | 12. Use probability to evaluate graphics and research claims | [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#12. Use probability to evaluate graphics and research claims|Open section]] |
| V10 | Keep probability weights stable before sampling | [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Keep probability weights stable before sampling|Open section]] |
| V10 | Design samples: without replacement, stratified, and low-discrepancy | [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Design samples: without replacement, stratified, and low-discrepancy|Open section]] |
| V10 | Test numerical methods and benchmark reproducibly | [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts#Test numerical methods and benchmark reproducibly|Open section]] |
| V11 | Source orientation: 11 - From Coordinate Frames to Camera Images - Projective Geometry and Cameras | [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem#From Coordinate Frames to Camera Images: Projective Geometry and Cameras|Open section]] |
| V11 | A coordinate is a claim about a frame | [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem#A coordinate is a claim about a frame|Open section]] |
| V11 | A camera is an inverse pose | [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem#A camera is an inverse pose|Open section]] |
| V11 | Perspective belongs to projective geometry | [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem#Perspective belongs to projective geometry|Open section]] |
| V11 | Projection turns rays into clip coordinates | [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling#Projection turns rays into clip coordinates|Open section]] |
| V11 | Depth is a numerical design choice | [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling#Depth is a numerical design choice|Open section]] |
| V11 | A projected point still needs a raster convention | [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling#A projected point still needs a raster convention|Open section]] |
| V11 | A sign can matter more than a coordinate: robust predicates | [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity#A sign can matter more than a coordinate: robust predicates|Open section]] |
| V11 | The viewing volume is six half-spaces | [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity#The viewing volume is six half-spaces|Open section]] |
| V11 | Raster interpolation remembers homogeneous depth | [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity#Raster interpolation remembers homogeneous depth|Open section]] |
| V11 | Intrinsics map optical rays to pixels | [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity#Intrinsics map optical rays to pixels|Open section]] |
| V11 | Extrinsics place the optical camera in the world | [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Extrinsics place the optical camera in the world|Open section]] |
| V11 | Real lenses bend the pinhole image | [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Real lenses bend the pinhole image|Open section]] |
| V11 | Recover geometry from depth and construct screen-space rays | [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Recover geometry from depth and construct screen-space rays|Open section]] |
| V11 | Build projection and stereo geometry around real camera baselines | [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Build projection and stereo geometry around real camera baselines|Open section]] |
| V11 | Represent directions over the full sphere | [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry#Represent directions over the full sphere|Open section]] |
| V11 | Model exposure, physical optics, and depth of field | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Model exposure, physical optics, and depth of field|Open section]] |
| V11 | A camera image changes over time | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#A camera image changes over time|Open section]] |
| V11 | Large worlds need local arithmetic | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Large worlds need local arithmetic|Open section]] |
| V11 | Calibration needs informative observations and a fixed gauge | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Calibration needs informative observations and a fixed gauge|Open section]] |
| V11 | Calibrated two-view geometry: direction before distance | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Calibrated two-view geometry: direction before distance|Open section]] |
| V11 | Gauge freedom, singular values, and choosing a physical pose | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Gauge freedom, singular values, and choosing a physical pose|Open section]] |
| V11 | Planes change the model | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Planes change the model|Open section]] |
| V11 | Why three views contribute information beyond pairs | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Why three views contribute information beyond pairs|Open section]] |
| V11 | A camera ray carries a pixel footprint | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#A camera ray carries a pixel footprint|Open section]] |
| V11 | Four deeper links from the TUM lecture review | [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field#Four deeper links from the TUM lecture review|Open section]] |
| V11 | Make camera conventions explicit and debug the full transform chain | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Make camera conventions explicit and debug the full transform chain|Open section]] |
| V11 | A camera model must close the loop | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#A camera model must close the loop|Open section]] |
| V11 | A runnable camera oracle with declared conventions | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#A runnable camera oracle with declared conventions|Open section]] |
| V06 | Three stored numbers are not the complete contract | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Three stored numbers are not the complete contract|Open section]] |
| V06 | Storage and computation can choose different layouts | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Storage and computation can choose different layouts|Open section]] |
| V06 | Reliability begins at the boundary of the calculation | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Reliability begins at the boundary of the calculation|Open section]] |
| V06 | Types can remember the promises that three components forget | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#Types can remember the promises that three components forget|Open section]] |
| V06 | A complete scalar laboratory makes the story executable | [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging#A complete scalar laboratory makes the story executable|Open section]] |
| V06 | Teach the complete journey, then break one assumption at a time | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Teach the complete journey, then break one assumption at a time|Open section]] |
| V07 | The mathematical table needs an explicit storage agreement | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#The mathematical table needs an explicit storage agreement|Open section]] |
| V07 | Runtime views borrow a layout as well as a lifetime | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Runtime views borrow a layout as well as a lifetime|Open section]] |
| V07 | SIMD and loop order change the execution schedule, not the matrix question | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#SIMD and loop order change the execution schedule, not the matrix question|Open section]] |
| V07 | A complete scalar laboratory makes the contracts observable | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#A complete scalar laboratory makes the contracts observable|Open section]] |
| V07 | Teach the journey by changing one promise | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Teach the journey by changing one promise|Open section]] |
| V08 | Implementation kernels and shape reasoning | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Implementation kernels and shape reasoning|Open section]] |
| V08 | An implementation review checklist grounded in the map | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#An implementation review checklist grounded in the map|Open section]] |
| V08 | Householder reductions turn a dense SVD into a bidiagonal problem | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Householder reductions turn a dense SVD into a bidiagonal problem|Open section]] |
| V08 | Jacobi SVD removes column coupling pair by pair | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Jacobi SVD removes column coupling pair by pair|Open section]] |
| V08 | Scale-aware convergence and output conventions make results interpretable | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Scale-aware convergence and output conventions make results interpretable|Open section]] |
| V08 | Independent reconstruction and orthogonality checks test the SVD invariants | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Independent reconstruction and orthogonality checks test the SVD invariants|Open section]] |
| V08 | A matrix object must describe shape, ownership, and workspace | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#A matrix object must describe shape, ownership, and workspace|Open section]] |
| V08 | Dynamic matrix storage makes factorization dimensions and lifetimes explicit | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Dynamic matrix storage makes factorization dimensions and lifetimes explicit|Open section]] |
| V08 | Zero singular directions require deliberate orthonormal completion | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Zero singular directions require deliberate orthonormal completion|Open section]] |
| V08 | Matrix views need checked address arithmetic as well as a stride | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Matrix views need checked address arithmetic as well as a stride|Open section]] |
| V08 | Factorization is also a memory and lifetime decision | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Factorization is also a memory and lifetime decision|Open section]] |
| V08 | Workspace allocation is part of the solver contract | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Workspace allocation is part of the solver contract|Open section]] |
| V08 | Sparse problems change the economics of solving | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Sparse problems change the economics of solving|Open section]] |
| V08 | Sparse storage changes the cost model | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Sparse storage changes the cost model|Open section]] |
| V08 | Direct and iterative solvers suit different workloads | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Direct and iterative solvers suit different workloads|Open section]] |
| V08 | Choose a factorization from structure and the requested answer | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Choose a factorization from structure and the requested answer|Open section]] |
| V08 | Board workbench: follow the numbers before trusting the name | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Board workbench: follow the numbers before trusting the name|Open section]] |
| V08 | An SVD API begins with shapes and guarantees | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#An SVD API begins with shapes and guarantees|Open section]] |
| V08 | Numerical policies must share one rank decision | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Numerical policies must share one rank decision|Open section]] |
| V08 | A call lifecycle makes ownership reviewable | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#A call lifecycle makes ownership reviewable|Open section]] |
| V08 | Validate reconstruction, orthogonality, and triplets separately | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Validate reconstruction, orthogonality, and triplets separately|Open section]] |
| V08 | The library boundary is part of numerical correctness | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#The library boundary is part of numerical correctness|Open section]] |
| V08 | Close the journey with an algorithm-selection conversation | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Close the journey with an algorithm-selection conversation|Open section]] |
| V10 | Reconstructed C++23 laboratory | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#Reconstructed C++23 laboratory|Open section]] |
| V10 | A runnable laboratory for numerical contracts | [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline#A runnable laboratory for numerical contracts|Open section]] |
| V06 | Research connections without borrowed certainty | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Research connections without borrowed certainty|Open section]] |
| V08 | A reader route through Phase 8 | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#A reader route through Phase 8|Open section]] |
| V08 | A reconstruction desk for active teaching | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#A reconstruction desk for active teaching · V08.83.01|Open section]] |
| V08 | Research and implementation references | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Research and implementation references|Open section]] |
| V08 | Source coverage and the limits of this teaching volume | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Source coverage and the limits of this teaching volume|Open section]] |
| V10 | Learning route | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Learning route|Open section]] |
| V10 | Related vault chapters | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Related vault chapters|Open section]] |
| V10 | Supplied source claims and provenance requiring verification | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Supplied source claims and provenance requiring verification|Open section]] |
| V10 | Repository observations reported by the supplied source | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Repository observations reported by the supplied source · V10.24.01|Open section]] |
| V10 | Linked numerical concepts | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Linked numerical concepts|Open section]] |
| V11 | Linked camera concepts | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Linked camera concepts|Open section]] |
| V11 | Relationship to existing Phase 11 material | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Relationship to existing Phase 11 material|Open section]] |
| V11 | Evidence and integration | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Evidence and integration|Open section]] |
| V11 | External course legend and evidence boundary | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#External course legend and evidence boundary|Open section]] |
| V11 | Research concept links | [[Chapter Edition/01 - Computing Foundations/39 - Research Navigation and Source Boundaries#Research concept links|Open section]] |

## Concept graph connections

[[Concepts/A posteriori error bounds for contractive iteration|A posteriori error bounds for contractive iteration]] · [[Concepts/Algorithmic stability|Algorithmic stability]] · [[Concepts/Axial depth and Euclidean range|Axial depth and Euclidean range]] · [[Concepts/Barycentric coordinates and interpolation|Barycentric coordinates and interpolation]] · [[Concepts/Basis coordinates and dimension|Basis coordinates and dimension]] · [[Concepts/Binary32|Binary32]] · [[Concepts/Bracketed and open root solvers|Bracketed and open root solvers]] · [[Concepts/Bézier curves and De Casteljau evaluation|Bézier curves and De Casteljau evaluation]] · [[Concepts/Camera gauge freedoms and observability|Camera gauge freedoms and observability]] · [[Concepts/Camera view and clip-space pipeline|Camera view and clip-space pipeline]] · [[Concepts/Color encoding and linear-light operations|Color encoding and linear-light operations]] · [[Concepts/Common probability distributions and their assumptions|Common probability distributions and their assumptions]] · [[Concepts/Complex numbers and planar rotations|Complex numbers and planar rotations]] · [[Concepts/Conditional Gaussians and Schur complements|Conditional Gaussians and Schur complements]] · [[Concepts/Conditioning and singularity|Conditioning and singularity]] · [[Concepts/Confidence intervals and sequential evidence|Confidence intervals and sequential evidence]] · [[Concepts/Convergence order and Richardson extrapolation|Convergence order and Richardson extrapolation]] · [[Concepts/Convex combinations and convex hulls|Convex combinations and convex hulls]] · [[Concepts/Coordinate frames and change of basis|Coordinate frames and change of basis]] · [[Concepts/Coordinate handedness and orientation|Coordinate handedness and orientation]] · [[Concepts/Coordinate-space types and validated invariants|Coordinate-space types and validated invariants]] · [[Concepts/Covectors linear functionals and normal transforms|Covectors linear functionals and normal transforms]] · [[Concepts/Cross products and oriented area|Cross products and oriented area]] · [[Concepts/Cubic Hermite interpolation|Cubic Hermite interpolation]] · [[Concepts/Definite integrals and the fundamental theorem|Definite integrals and the fundamental theorem]] · [[Concepts/Derivative as a local linear map|Derivative as a local linear map]] · [[Concepts/Determinants orientation and volume|Determinants orientation and volume]] · [[Concepts/Differentiation rules and chain rule|Differentiation rules and chain rule]] · [[Concepts/Directional and total derivatives|Directional and total derivatives]] · [[Concepts/Discretization, truncation, and roundoff|Discretization, truncation, and roundoff]] · [[Concepts/Dot and cross-product rounding|Dot and cross-product rounding]] · [[Concepts/Dot products and orthogonality|Dot products and orthogonality]] · [[Concepts/Dual quaternions and rigid motion|Dual quaternions and rigid motion]] · [[Concepts/Eigenvalues and eigenspaces|Eigenvalues and eigenspaces]] · [[Concepts/Euler angles and gimbal singularity|Euler angles and gimbal singularity]] · [[Concepts/Expectation, covariance, and dependence|Expectation, covariance, and dependence]] · [[Concepts/Exterior products and geometric algebra|Exterior products and geometric algebra]] · [[Concepts/Finite differences and step-size balance|Finite differences and step-size balance]] · [[Concepts/Floating-point reassociation|Floating-point reassociation]] · [[Concepts/Flush-to-zero and denormals-are-zero|Flush-to-zero and denormals-are-zero]] · [[Concepts/Function contracts: domains, codomains, and images|Function contracts: domains, codomains, and images]] · [[Concepts/Gaussian elimination and pivoting|Gaussian elimination and pivoting]] · [[Concepts/Gram matrices and coordinate metrics|Gram matrices and coordinate metrics]] · [[Concepts/Hardware depth-buffer linearization|Hardware depth-buffer linearization]] · [[Concepts/Hessians and local curvature|Hessians and local curvature]] · [[Concepts/Homogeneous coordinates and perspective division|Homogeneous coordinates and perspective division]] · [[Concepts/Implicit differentiation and parameter sensitivities|Implicit differentiation and parameter sensitivities]] · [[Concepts/Interpolation versus approximation|Interpolation versus approximation]] · [[Concepts/Interpretation contract|Interpretation contract]] · [[Concepts/Jacobians, pushforwards, and pullbacks|Jacobians, pushforwards, and pullbacks]] · [[Concepts/LU factorization and reusable solves|LU factorization and reusable solves]] · [[Concepts/Latin-hypercube sampling|Latin-hypercube sampling]] · [[Concepts/Limits and continuity|Limits and continuity]] · [[Concepts/Linear affine and projective transformations|Linear affine and projective transformations]] · [[Concepts/Linear systems and fundamental subspaces|Linear systems and fundamental subspaces]] · [[Concepts/Matrices as linear maps and basis images|Matrices as linear maps and basis images]] · [[Concepts/Matrix SIMD tiling and cache reuse|Matrix SIMD tiling and cache reuse]] · [[Concepts/Matrix factorization choices|Matrix factorization choices]] · [[Concepts/Matrix functions|Matrix functions]] · [[Concepts/Matrix inversion pivoting and residuals|Matrix inversion pivoting and residuals]] · [[Concepts/Matrix multiplication and composition order|Matrix multiplication and composition order]] · [[Concepts/Matrix norms and numerical error|Matrix norms and numerical error]] · [[Concepts/Matrix storage indexing and strides|Matrix storage indexing and strides]] · [[Concepts/Matrix transpose symmetry and trace|Matrix transpose symmetry and trace]] · [[Concepts/Minkowski sums and support mappings|Minkowski sums and support mappings]] · [[Concepts/Moore-Penrose pseudoinverse|Moore-Penrose pseudoinverse]] · [[Concepts/Multivariate Gaussian distributions and whitening|Multivariate Gaussian distributions and whitening]] · [[Concepts/Newton and quasi-Newton optimization|Newton and quasi-Newton optimization]] · [[Concepts/Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt|Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt]] · [[Concepts/Norms lengths and induced distances|Norms lengths and induced distances]] · [[Concepts/Numerical error budgets|Numerical error budgets]] · [[Concepts/Numerical quadrature and error|Numerical quadrature and error]] · [[Concepts/ODE initial-value problems and stiffness|ODE initial-value problems and stiffness]] · [[Concepts/Off-axis stereo projection and disparity|Off-axis stereo projection and disparity]] · [[Concepts/Omnidirectional camera projections|Omnidirectional camera projections]] · [[Concepts/Online moments and Welford accumulation|Online moments and Welford accumulation]] · [[Concepts/Orthonormal frames and Gram-Schmidt|Orthonormal frames and Gram-Schmidt]] · [[Concepts/PCA and covariance eigensystems|PCA and covariance eigensystems]] · [[Concepts/Partial derivatives and gradients|Partial derivatives and gradients]] · [[Concepts/Polar decomposition|Polar decomposition]] · [[Concepts/Position reconstruction from depth|Position reconstruction from depth]] · [[Concepts/Probability spaces, events, and conditioning|Probability spaces, events, and conditioning]] · [[Concepts/Projection rejection and scalar components|Projection rejection and scalar components]] · [[Concepts/QR factorization and least squares|QR factorization and least squares]] · [[Concepts/Quaternion algebra and Hamilton product|Quaternion algebra and Hamilton product]] · [[Concepts/Quaternion interpolation and shortest paths|Quaternion interpolation and shortest paths]] · [[Concepts/Quaternion matrix and axis-angle conversion|Quaternion matrix and axis-angle conversion]] · [[Concepts/Quaternion rotation and double cover|Quaternion rotation and double cover]] · [[Concepts/Random variables and probability laws|Random variables and probability laws]] · [[Concepts/Ray differentials and camera sampling measures|Ray differentials and camera sampling measures]] · [[Concepts/Reflection refraction and shading directions|Reflection refraction and shading directions]] · [[Concepts/Reproducible numerical benchmarking|Reproducible numerical benchmarking]] · [[Concepts/Robust geometric predicates|Robust geometric predicates]] · [[Concepts/Robust norm and intermediate range|Robust norm and intermediate range]] · [[Concepts/Robust summaries and error metrics|Robust summaries and error metrics]] · [[Concepts/Robust vector angles|Robust vector angles]] · [[Concepts/SIMD and GPU data layouts|SIMD and GPU data layouts]] · [[Concepts/Sampling quality: stratification, quasi-Monte Carlo, and blue noise|Sampling quality: stratification, quasi-Monte Carlo, and blue noise]] · [[Concepts/Scalars fields and real-number models|Scalars fields and real-number models]] · [[Concepts/Screen-space ray generation and mouse picking|Screen-space ray generation and mouse picking]] · [[Concepts/Singular value decomposition|Singular value decomposition]] · [[Concepts/Sparse and iterative solvers|Sparse and iterative solvers]] · [[Concepts/Stable probability normalization and log-sum-exp|Stable probability normalization and log-sum-exp]] · [[Concepts/Stable quadratic root evaluation|Stable quadratic root evaluation]] · [[Concepts/Standard-layout and trivial copyability|Standard-layout and trivial copyability]] · [[Concepts/Stateful pseudorandom generators and stream state|Stateful pseudorandom generators and stream state]] · [[Concepts/Stateless keyed sampling and reproducibility|Stateless keyed sampling and reproducibility]] · [[Concepts/Surface tangents normals and bitangents|Surface tangents normals and bitangents]] · [[Concepts/Symmetric positive-definite matrices and Cholesky|Symmetric positive-definite matrices and Cholesky]] · [[Concepts/Taylor models and approximation error|Taylor models and approximation error]] · [[Concepts/Tensor-product interpolation|Tensor-product interpolation]] · [[Concepts/Time integrators and symplectic updates|Time integrators and symplectic updates]] · [[Concepts/Transform hierarchies TRS and shear|Transform hierarchies TRS and shear]] · [[Concepts/Triple products and signed volume|Triple products and signed volume]] · [[Concepts/Uniform shuffling without replacement|Uniform shuffling without replacement]] · [[Concepts/Unit directions and safe normalization|Unit directions and safe normalization]] · [[Concepts/Vector spaces subspaces and span|Vector spaces subspaces and span]] · [[Concepts/Vector storage SIMD and boundary layouts|Vector storage SIMD and boundary layouts]] · [[Concepts/Vector tolerances and finite-state diagnostics|Vector tolerances and finite-state diagnostics]] · [[Concepts/Vector value types and component access|Vector value types and component access]] · [[Concepts/Vectors coordinates and affine points|Vectors coordinates and affine points]]


## Recovered cumulative reconstruction proofs

These complete proof sections are recovered from the retained pre-cleanup derivation snapshot. They restore the targets of stale navigation links and supplement the teaching already in this volume.

## Phase 8 - Linear systems and decompositions

These compact reconstructions support [[Chapter Edition/01 - Computing Foundations/19 - Coordinate Frames and Linear Systems#The graphics problem hiding inside a matrix equation|the full Phase 8 narrative]].

### The solution set is one solution plus the null space

If $A\mathbf x_0=\mathbf b$, then $A\mathbf x=\mathbf b$ exactly when $A(\mathbf x-\mathbf x_0)=0$. Hence

$$
\{\mathbf x:A\mathbf x=\mathbf b\}=\mathbf x_0+\operatorname{Null}(A).
$$

Rank-nullity follows by choosing a basis of the null space, extending it to a basis of the input space, and observing that the images of the added basis vectors form a basis of the column space. Therefore $\dim\operatorname{Null}(A)+\operatorname{rank}(A)=n$.

### The least-squares residual is perpendicular to the reachable output

Let $\widehat x$ minimize $\|Ax-b\|_2^2$. For any direction $h$, differentiate along $x(t)=\widehat x+th$:

$$
\frac{d}{dt}\|A(\widehat x+th)-b\|_2^2\bigg|_{t=0}
=2h^\mathsf T A^\mathsf T(A\widehat x-b)=0.
$$

Since this holds for every $h$, $A^\mathsf T(A\widehat x-b)=0$. Thus the residual is orthogonal to every column of $A$. With thin QR $A=QR$ and full column rank, $Q^TQ=I$ preserves the norm and reduces the minimizer to $R\widehat x=Q^Tb$.

### Why normal equations square the 2-norm condition number

For full-column-rank $A$, the symmetric eigenvalues of $A^TA$ are the squared singular values of $A$. Therefore

$$
\kappa_2(A^TA)=\frac{\sigma_{\max}^2}{\sigma_{\min}^2}=\kappa_2(A)^2.
$$

This is an algebraic identity and a numerical warning: forming the Gram matrix can make small singular directions harder to distinguish.

### The pseudoinverse conditions select least-squares then minimum norm

For $A=U\Sigma V^T$, define the exact $A^+=V\Sigma^+U^T$ by reciprocating every positive singular value. The coordinates of $b$ along left-singular vectors with positive singular values determine $x$; components orthogonal to $\operatorname{Col}(A)$ remain residual. Null-space components of $x$ do not affect $Ax$, and choosing them as zero gives the minimum Euclidean norm.

A numerical threshold instead defines $A_\tau^+$ by discarding directions with $\sigma_i\le\tau$. When the discarded values include positive singular values, this is the pseudoinverse of a truncated matrix $A_\tau$, and its result need not minimize the original residual for $A$. The threshold belongs to a declared problem scale and precision.

### PCA through covariance and through the centered data SVD

For centered sample rows $X\in\mathbb R^{N\times d}$ and $X=U\Sigma V^T$,

$$
C=\frac{X^TX}{N-1}
=V\frac{\Sigma^T\Sigma}{N-1}V^T.
$$

Thus the right singular vectors of $X$ are covariance eigenvectors, and covariance eigenvalues equal $\sigma_i^2/(N-1)$. The identity explains PCA; computing SVD directly can avoid explicitly forming a Gram matrix with squared conditioning.

### Right polar decomposition from the SVD

For nonsingular square $A=U\Sigma V^T$, set $Q=UV^T$ and $H=V\Sigma V^T$. Then $Q$ is orthogonal, $H$ is symmetric positive definite, and

$$
QH=UV^TV\Sigma V^T=U\Sigma V^T=A.
$$

Also $H^2=V\Sigma^2V^T=A^TA$, so $H=(A^TA)^{1/2}$. If $A$ is singular, the positive-semidefinite square root still exists but the orthogonal completion may not be unique.

### Scaling and squaring identity

For any square matrix and integer $s\ge0$, the exponential's composition law gives

$$
e^A=\left(e^{A/2^s}\right)^{2^s}.
$$

The method chooses $s$ so a rational or polynomial approximation is accurate for the smaller-norm matrix, then squares. In floating point the identity motivates an algorithm; approximation and repeated rounding still need to be bounded and tested.

## Phase 7 - Transform reconstruction proofs

Essential proofs are in [[Chapter Edition/01 - Computing Foundations/18 - Vectors for Light and Frames#Keep the promises visible while moving between representations|the continuous chapter]]. These exercises deepen the same contracts rather than introduce prerequisites hidden in this supplement.

### A change of basis changes a map on both sides

Let $A$ represent one endomorphism in a reference basis, and let the columns of invertible $P$ be a new basis expressed in that reference. A new-coordinate input $x'$ represents $x=Px'$. Applying the map gives $y=APx'$, and expressing its output in the new basis gives $y'=P^{-1}APx'$. Thus its new matrix is $A'=P^{-1}AP$. This is a similarity transform, not a spatial motion of the object.

The determinant is invariant under this change because

$$
\det(P^{-1}AP)=\det(P)^{-1}\det(A)\det(P)=\det(A).
$$

The trace is invariant because cyclically moving the first factor gives $\operatorname{tr}(P^{-1}AP)=\operatorname{tr}(APP^{-1})=\operatorname{tr}(A)$. These invariants do not identify the entire map: many distinct maps share trace and determinant. A row-major/column-major storage conversion is not a similarity transform; it changes address mapping, not the chosen basis.

### A scaled quaternion separates the three rotation formulas

For nonzero $q=m\widehat q$ with $\|\widehat q\|=1$ and $m>0$, the true-inverse sandwich is

$$
q(0,v)q^{-1}=\widehat q(0,v)\widehat q^*.
$$

Its scale cancels. The conjugate sandwich instead equals $m^2\widehat q(0,v)\widehat q^*$. For $q=2q_Z$, where $q_Z$ is the positive-Z quarter turn, the actions on X are respectively $(0,1,0)$ and $(0,4,0)$. The optimized unit-only expression $v+w(2u\times v)+u\times(2u\times v)$ gives $(-3,4,0)$ on that same invalid input. This demonstrates why describing every nonunit failure as uniform scaling is inaccurate: it depends on the actual formula used.

### Why a restricted TRS representation is not closed

The columns of $RD$ are pairwise perpendicular when $R$ is orthogonal and $D$ diagonal: $(RD)^T(RD)=D^2$ is diagonal. Let $S=\operatorname{diag}(2,1)$ and let $R$ rotate by $45^\circ$. The column dot product of $SR$ is $-3/2$, which is nonzero. Therefore no orthogonal $R'$ and diagonal $D'$ can satisfy $SR=R'D'$. Embedding this example in three dimensions proves the same limitation for general nonuniform-scale transform hierarchies. The affine matrix still exists; only the restricted storage promise fails.

### Coupled normalization of a rigid dual quaternion

Let a finite blended pair be $(r,d)$ with $a=\|r\|>0$. Define $r'=r/a$, $d_0=d/a$ and $d'=d_0-r'(r'\cdot d_0)$. Then

$$
\|r'\|=1,\qquad r'\cdot d'=r'\cdot d_0-(r'\cdot r')(r'\cdot d_0)=0.
$$

The recovered $2d'r'^*$ has zero scalar part, as a translation pure quaternion requires. This is projection onto the rigid constraints, not a claim that every weighted blend has the desired animation path. Sign-align both halves together before blending, handle cancellation of the real part, and keep scale/shear outside the rigid representation.

## Phase 6 - Geometry reconstruction proofs

These extensions belong to [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems#The panel gives us a reason to learn|the Phase 6 story]]. Essential derivations remain in that volume. This section adds reconstruction exercises, not prerequisites hidden outside the teaching narrative.

### Why projection is an orthogonal decomposition

For a nonzero target vector $\mathbf b$, define $\alpha=(\mathbf a\cdot\mathbf b)/(\mathbf b\cdot\mathbf b)$ and $\mathbf r=\mathbf a-\alpha\mathbf b$. Then

$$
\mathbf r\cdot\mathbf b=\mathbf a\cdot\mathbf b-
\frac{\mathbf a\cdot\mathbf b}{\mathbf b\cdot\mathbf b}(\mathbf b\cdot\mathbf b)=0.
$$

For any candidate coefficient $t$, the difference is $\mathbf r+(\alpha-t)\mathbf b$. Its squared length is

$$
\|\mathbf r\|^2+(\alpha-t)^2\|\mathbf b\|^2,
$$

since its cross term vanishes. The minimum occurs uniquely at $t=\alpha$. Thus perpendicular residual, least-squares projection, and closest line point are one result.

For a unit axis $\mathbf u$, define the projection matrix $P=\mathbf u\mathbf u^\mathsf T$. Matrix multiplication reconstructs $(\mathbf a\cdot\mathbf u)\mathbf u$. Since $\mathbf u^\mathsf T\mathbf u=1$, we obtain $P^2=P$. The complementary rejection matrix $I-P$ is also idempotent. The plane reflection matrix is $I-2P$ and squares to identity:

$$
(I-2P)^2=I-4P+4P^2=I.
$$

This explains why reflecting twice restores a vector in exact arithmetic.

### Gram determinants connect length and area

For two generators, their Gram matrix is

$$
G=\begin{pmatrix}\mathbf a\cdot\mathbf a&\mathbf a\cdot\mathbf b\\
\mathbf a\cdot\mathbf b&\mathbf b\cdot\mathbf b\end{pmatrix}.
$$

Its determinant is $\|\mathbf a\|^2\|\mathbf b\|^2-(\mathbf a\cdot\mathbf b)^2$, the squared parallelogram area. For a basis matrix $B$ with three columns, $G=B^\mathsf TB$. Therefore

$$
\det(G)=\det(B^\mathsf T)\det(B)=\det(B)^2.
$$

This is why the Gram determinant loses the sign of orientation. Reflecting the complete frame changes $\det(B)$ but not its square. A zero determinant signals dependence; a small dimensional value by itself is not a complete conditioning estimate.

### Affine weights and translation

For points with coordinates $p_i$ and weights $w_i$, move every contributor by $t$. Their weighted result becomes

$$
\sum_iw_i(p_i+t)=\sum_iw_ip_i+\left(\sum_iw_i\right)t.
$$

Weights summing to one produce the translation of a point. Weights summing to zero produce an origin-independent displacement. Other sums do not transform as either canonical role without further interpretation. Nonnegative weights summing to one produce convex combinations. The sum condition and sign condition solve different problems.

### The normal transformation preserves a scalar pairing

Let a vector transform actively as $v'=Av$, and represent a linear measurement by column coefficients $n$, so it returns $n^\mathsf Tv$. Preserving every measured result requires $n'^\mathsf TAv=n^\mathsf Tv$ for every $v$, hence $n'=A^{-\mathsf T}n$ when $A$ is invertible.

For a passive basis description $v=Bc$, the same fixed measurement has coordinate coefficients $\omega_B=B^\mathsf Tn$. Notice that this is a different question from actively deforming a surface. Both formulas follow from the same pairing rather than contradictory transformation rules.

In nonorthonormal coordinates, a vector's coordinate column $c$ determines its metric measurement row $c^\mathsf TG$. Componentwise dotting assumes $G=I$. This connects basis changes, dual measurements and normals.

### Why perspective interpolation uses reciprocal weights

Let affine surface weights be $\mu_i$, with sum one, and homogeneous vertex values $H_i$. Before division, the combined representative is $H=\sum_i\mu_iH_i$. Its final coordinate is $W=\sum_i\mu_iw_i$. After division, the screen weights satisfy

$$
\lambda_i=\frac{\mu_iw_i}{W}.
$$

Thus $\mu_i=\lambda_iW/w_i$. Impose $\sum_i\mu_i=1$ to get $W=1/(\sum_i\lambda_i/w_i)$, then

$$
\mu_i=\frac{\lambda_i/w_i}{\sum_j\lambda_j/w_j}.
$$

Applying these weights to an affine attribute gives the perspective-correct formula in the volume. The derivation assumes a valid projected configuration with appropriate nonzero denominators and clipping; it is not an instruction to divide primitives crossing a projective boundary.

### Support queries commute with set addition in a specific way

For nonempty compact sets $A,B$, expand the dot product in the support definition:

$$
h_{A\oplus B}(d)=\max_{a\in A,b\in B}(d\cdot a+d\cdot b)
=h_A(d)+h_B(d).
$$

The maxima separate because the choices of $a$ and $b$ are independent. Reflecting $B$ gives $h_{-B}(d)=h_B(-d)$; a maximizing point is the negative of a maximizer for $B$ in direction $-d$. Thus one valid support point of the collision difference is $s_A(d)-s_B(-d)$. Maximizers need not be unique, so repeatable tie selection is part of an implementation contract.

### Proof, numerical algorithm and application policy

A mathematical identity can be correct while its direct arithmetic evaluation overflows or cancels. A numerically stable calculation can still solve an ill-conditioned problem. A certified sign for stored coordinates can still misrepresent a physically uncertain measurement. Teach these three boundaries separately; neither a tolerance nor a proof replaces all of them.


## Direct textbook expansion and continuing source audit

The seven-book source pages, new vector/metric/field teaching, original numerical reconstructions and ongoing corpus-review limits are recorded in [[Supplementary/Math Book Expansion - Teaching and Verification]]. The earlier caption and book corpus remains required work in [[Supplementary/Manual Integration - Pending Source Queue]].


---

## Physical Foundations — Rigid Bodies, Extended Motion, and Contact Constraints

Rigid-body mechanics is also foundational: it supplies the continuous and discrete motion model that later engine simulation consumes. The former standalone rigid-body narrative is preserved below in full, connecting mathematical state, forces, angular motion, collision, and contact constraints back into the same foundations volume.

> **Reading position.** This is an additive, continuous treatment of the physical-model chain covered by the Game Engine book's Parts 1601–1700. It does not mark any missing original source reviewed or supersede its source ledger. Its examples, equations, assumptions, failures and executable witness form one self-contained reading route rather than one hundred atomic headings. For an engine, the central question is: **what state must we own and update so a crate can translate, rotate, collide and come to rest predictably?**

## The crate exposes the weakness of point-mass motion

Imagine the workshop game: a player pushes a crate toward a pressure plate that opens a door. Representing the crate by one position and one velocity suffices only while we ignore rotation and shape. An off-centre shove visibly spins it. Two crates with identical centre-of-mass velocities can strike the plate differently when their orientations differ. We therefore need a *rigid body*: an idealized extended object whose points keep fixed distances from one another. Rigidity is a modelling assumption, not a claim that real wood or steel never deforms.

Choose the world frame, SI units, and a local body frame. Let the centre of mass position be `x` (metres), its orientation `R` a proper 3×3 rotation matrix, linear velocity `v` (m/s), and angular velocity `ω` (rad/s). A local point `r_local` on the crate maps into the world as

$$
p(t)=x(t)+R(t)r_{\mathrm{local}}.
$$

It moves with point velocity

$$
u(p)=v+\omega\times(Rr_{\mathrm{local}}).
$$

This is not an extra empirical rule. Differentiate the position, and use the fact that the derivative of a rotating fixed-length vector is its instantaneous angular velocity crossed with the vector. The cross product is perpendicular to both operands: it gives a tangential velocity and preserves the body's internal distances. If the crate rotates about its centre while `v=0`, a point at the centre stays still; a point at radius 0.5 m with `|ω|=2 rad/s` travels tangentially at 1 m/s.

**First-use distinction.** Orientation describes *where* the body's axes point; angular velocity describes *how fast and about which axis* the orientation changes. A single Euler-angle triple can become singular at some orientations and is not a vector that can safely be integrated by ordinary component addition. A unit quaternion or a rotation matrix is suitable for orientation storage; normalize a quaternion to manage floating-point drift. The angular velocity is represented in a declared frame; the equations below use world-space quantities. A renderer may interpolate visual poses but must not silently become the owner of authoritative physics state.

## Why the centre of mass, inertia and torque appear

A body may consist of particles with masses `m_i` at local positions `r_i`. Its total mass is `m=Σm_i`; choose the origin at the centre of mass so `Σm_i r_i=0`. For a continuous body use integrals rather than sums. The mass tells us resistance to linear acceleration: Newton's law gives `m dv/dt = F` when `F` is net external force.

Rotation introduces another question: **how is that mass distributed around an axis?** The world-space moment-of-inertia tensor `I` maps angular velocity to angular momentum `L=Iω`. In the body frame,

$$
I_{\mathrm{body}}=\int_{\mathrm{body}}(\|r\|^2\mathbf{1}-rr^T)\,dm,
\qquad I_{\mathrm{world}}=R I_{\mathrm{body}}R^T.
$$

The tensor is symmetric; it is positive definite for a three-dimensional body with a nondegenerate mass distribution. This definition follows by summing each particle's angular momentum `r × (ω × r) dm` and applying the vector triple-product identity. For a rectangular box of side lengths `a,b,c`, mass `m`, and axes through its centre, the diagonal body-frame entries are `m(b²+c²)/12`, `m(a²+c²)/12`, and `m(a²+b²)/12`. Units are kg·m². A thin or deliberately locked object can make an inverse inertia axis singular or constrained; do not invert such a tensor blindly.

A force `F` applied at world point `p` exerts torque `τ=(p-x)×F`. The lever arm explains why a force through the centre translates without spinning, while the same force at an offset creates angular acceleration. General rotational dynamics satisfy

$$
\frac{dL}{dt}=\tau,\quad L=I_{\mathrm{world}}\omega,\quad
I_{\mathrm{world}}\dot\omega+\omega\times(I_{\mathrm{world}}\omega)=\tau .
$$

The cross term matters because the inertia tensor rotates with the body. The simplified equation `α=I⁻¹τ` is not generally exact for freely rotating asymmetric bodies; it becomes a useful special case about a fixed principal axis, with negligible gyroscopic coupling, or for an instantaneous impulse update in which orientation is held fixed.


## Generalized coordinates describe the configuration the mechanism can actually have

The Newtonian state above is written in ambient Cartesian coordinates because a free rigid body naturally lives in three-dimensional space. A constrained mechanism often has fewer independent degrees of freedom. A pendulum bob occupies \(\mathbb R^3\), but once its rod length is fixed its allowed positions lie on a sphere; a planar pendulum needs only one angle. A door has thousands of mesh vertices but one hinge angle. **Generalized coordinates** \(q=(q_1,\ldots,q_n)\) parameterize the configuration manifold rather than storing every constrained Cartesian coordinate as independent state.

This is not merely a memory optimization. It changes the mathematical form of the dynamics. Let the Cartesian configuration be a smooth map

\[
x=x(q).
\]

Then velocity is obtained by the chain rule,

\[
\dot x
=
J(q)\dot q,
\qquad
J(q)=\frac{\partial x}{\partial q}.
\]

The kinetic energy becomes a quadratic form in generalized velocity,

\[
T(q,\dot q)
=
\frac12
\dot q^T M(q)\dot q,
\]

where \(M(q)\) is the generalized mass matrix induced by the embedding and physical masses/inertias. Potential energy is \(U(q)\). Define the Lagrangian

\[
L(q,\dot q,t)=T-U.
\]

Hamilton's principle states that the physical motion between fixed endpoint configurations is a stationary curve of the action

\[
\mathcal S[q]
=
\int_{t_0}^{t_1}
L(q,\dot q,t)\,dt.
\]

Vary the path by \(q(t)+\varepsilon\eta(t)\) with \(\eta(t_0)=\eta(t_1)=0\). Differentiating the action with respect to \(\varepsilon\), integrating the \(\partial L/\partial\dot q\) term by parts, and requiring the first variation to vanish for arbitrary \(\eta\) yields

\[
\boxed{
\frac{d}{dt}
\frac{\partial L}{\partial\dot q_i}
-
\frac{\partial L}{\partial q_i}
=
0
}
\]

for every generalized coordinate.

Arnold emphasizes a structural advantage that is easy to miss in component derivations: the extremal condition does not depend on whether the same curve is described in Cartesian, polar, or another smooth coordinate system. The formula for \(L\) changes with coordinates, but the physical extremal does not. That is the right reason to use generalized coordinates—not because angles are somehow more “physical,” but because coordinates should follow the configuration manifold.

For a simple planar pendulum with length \(\ell\), mass \(m\), and angle \(\theta\),

\[
T=\frac12m\ell^2\dot\theta^2,
\qquad
U=mg\ell(1-\cos\theta).
\]

The Euler–Lagrange equation gives

\[
m\ell^2\ddot\theta
+
mg\ell\sin\theta
=
0,
\]

or

\[
\ddot\theta+\frac{g}{\ell}\sin\theta=0.
\]

The rod-length constraint never appears as a multiplier because the coordinate already lives on the allowed one-dimensional configuration space. This is a **reduced-coordinate** model. The contact solver later in this chapter instead uses full body coordinates and imposes transient constraints through impulses. Both are legitimate; they solve different structural problems.

For an engine, the choice is architectural. A ragdoll assembled from unconstrained rigid bodies plus joints is flexible under topology changes and contact. A robotic arm represented directly by joint coordinates can eliminate joint drift and reduce the number of unknowns. Converting between the two requires explicit kinematics, Jacobians, inertia mapping, and a policy for contacts that act in Cartesian space.

Source basis: V. I. Arnold, *Mathematical Methods of Classical Mechanics*, Part II, Chapters 3–4, especially the calculus of variations, Lagrange's equations, coordinate-independence of extremals, holonomic constraints, and Lagrangian systems.

## D'Alembert's principle says ideal constraint reactions do no virtual work

A holonomic constraint restricts configuration to a submanifold \(M\). In Cartesian coordinates, the body may feel reaction forces that enforce the restriction. D'Alembert's principle characterizes an **ideal** constraint by saying the reaction has no component along any allowed virtual displacement.

Let \(x(t)\in M\), and let \(\xi\in T_xM\) be any tangent vector to the constraint manifold at the current configuration. For a unit-mass natural mechanical system with potential \(U(x)\), Arnold writes the constrained equation in the form

\[
\left(
\ddot x+\nabla U(x),
\xi
\right)
=
0
\qquad
\forall \xi\in T_xM.
\]

Equivalently, the residual

\[
\ddot x+\nabla U(x)
\]

is normal to the allowed tangent space. That normal component is supplied by the constraint reaction.

This is the geometric reason Lagrange multipliers appear. Suppose constraints are

\[
g_a(x)=0.
\]

Their gradients \(\nabla g_a\) span normal directions under regularity assumptions, so the Cartesian equation can be written

\[
M\ddot x
=
F_{\mathrm{applied}}
+
J_g(x)^T\lambda,
\]

where \(J_g\) is the constraint Jacobian and \(\lambda\) contains reaction magnitudes. Multiplying by an allowed virtual displacement \(\delta x\) satisfying \(J_g\delta x=0\) makes the constraint term vanish:

\[
\delta x^T J_g^T\lambda
=
(J_g\delta x)^T\lambda
=
0.
\]

The reaction enforces geometry without doing first-order work along permitted motion.

Do not overextend this idealization. Dry friction, motors, damping, joint limits with impacts, and compliant contacts can perform or dissipate work. A rolling-without-slip condition may be nonholonomic and cannot always be represented as a position-level manifold \(g(q)=0\). Game-engine joints often add stabilization terms because discrete integration drifts away from the ideal manifold. Those numerical terms should be identified separately from the physical constraint.

The contact constraint later in this chapter is unilateral rather than an equality manifold: bodies may separate but may not interpenetrate. Its multiplier/impulse must satisfy a nonnegativity/complementarity condition. D'Alembert's tangent/normal picture still helps: the normal reaction acts in the forbidden direction, while tangential motion is free until friction introduces another constraint law.

Source basis: Arnold, Part II, §21 on D'Alembert's principle and constrained extrema on manifolds. The Jacobian/multiplier engine form above is the standard finite-dimensional translation of the tangent-space statement.

## Small oscillations turn nonlinear mechanics into a generalized eigenproblem near equilibrium

A nonlinear mechanical system can be difficult globally and simple locally. Arnold begins the theory of oscillations by linearizing near an equilibrium because the linearized system often explains frequencies, stability, resonance, and the first visible response to small disturbances.

Let \(q_0\) be a stable equilibrium and write

\[
q=q_0+\eta.
\]

For a natural Lagrangian, expand the potential:

\[
U(q_0+\eta)
=
U(q_0)
+
\frac12\eta^T K\eta
+
O(\|\eta\|^3),
\]

because the first derivative vanishes at equilibrium. Here \(K\) is the Hessian of the potential at \(q_0\). Freeze the mass matrix to first order,

\[
T
\approx
\frac12\dot\eta^T M\dot\eta.
\]

The linearized Euler–Lagrange equations are

\[
M\ddot\eta+K\eta=0.
\]

Seek a harmonic mode

\[
\eta(t)=\phi e^{i\omega t}.
\]

Then

\[
(K-\omega^2M)\phi=0,
\]

or

\[
\boxed{
K\phi=\omega^2M\phi.
}
\]

This is a generalized eigenvalue problem. Eigenvectors \(\phi_j\) are mode shapes; \(\omega_j\) are natural angular frequencies. Under the usual symmetric positive-definite mass matrix and stable stiffness assumptions, modes can be chosen \(M\)-orthogonal:

\[
\phi_i^T M\phi_j=0
\qquad i\neq j.
\]

Expand displacement as

\[
\eta(t)=\sum_j a_j(t)\phi_j.
\]

The coupled system then decouples into scalar oscillators,

\[
\ddot a_j+\omega_j^2a_j=0,
\]

until damping, forcing, nonlinearities, or mode truncation are added.

This is the mechanics counterpart of PCA/SVD and Laplacian eigenmodes elsewhere in the curriculum: a basis is chosen that diagonalizes the important quadratic forms. Modal animation and reduced deformable simulation exploit exactly this structure. Keeping the lowest modes can reproduce large-scale compliant motion at much lower cost, but mode truncation cannot represent arbitrary local deformation or topology change.

Stability also depends on the Hessian. If \(K\) has a negative generalized eigenvalue relative to \(M\), then \(\omega^2<0\) for that direction and the linearized solution grows exponentially rather than oscillating. A zero mode can correspond to a symmetry or unconstrained rigid motion. Thus an eigenanalysis is also a local stability diagnostic.

Numerical stepping must respect the highest retained frequency. Explicit methods can become unstable when \(h\omega_{\max}\) is too large. A reduced model with fewer high-frequency modes can therefore be cheaper not only because it has fewer coordinates but because it may permit a larger stable timestep—though that changes the represented physics.

Source basis: Arnold, Chapter 5 on linearization and oscillations; the generalized mass/stiffness eigenproblem is also the normal-mode framework developed in Riley, Hobson, and Bence, *Mathematical Methods for Physics and Engineering*, Chapter 9.

## The Legendre transform turns configuration-space dynamics into phase-space dynamics

The Lagrangian formulation evolves \(q\) through a second-order differential equation. Hamiltonian mechanics introduces canonical momentum

\[
p_i
=
\frac{\partial L}{\partial\dot q_i}
\]

and, when the velocity-to-momentum map is invertible, uses \((q,p)\) as independent phase-space coordinates.

Define the Hamiltonian by the Legendre transform in the velocity variables,

\[
H(q,p,t)
=
p^T\dot q-L(q,\dot q,t),
\]

where \(\dot q\) is expressed as a function of \((q,p,t)\). Differentiating,

\[
dH
=
\dot q^Tdp
-
\frac{\partial L}{\partial q}^Tdq
-
\frac{\partial L}{\partial t}dt,
\]

after the \(p^Td\dot q\) terms cancel using \(p=\partial L/\partial\dot q\). The Euler–Lagrange equation gives \(\dot p=\partial L/\partial q\), so

\[
\boxed{
\dot q=
\frac{\partial H}{\partial p},
\qquad
\dot p=
-\frac{\partial H}{\partial q}.
}
\]

A second-order system in \(q\) has become a first-order system in twice as many variables.

For a standard particle with

\[
L(q,\dot q)
=
\frac12\dot q^TM\dot q-U(q)
\]

and constant positive-definite \(M\),

\[
p=M\dot q,
\]

and

\[
H(q,p)
=
\frac12p^TM^{-1}p+U(q).
\]

Here \(H\) equals total mechanical energy. That equality is common but not the definition of a Hamiltonian; generalized velocity-dependent potentials, time-dependent coordinates, or other Lagrangians require more care.

If \(H\) has no explicit time dependence, then along a Hamiltonian trajectory

\[
\frac{dH}{dt}
=
\nabla_qH^T\dot q
+
\nabla_pH^T\dot p
=
\nabla_qH^T\nabla_pH
-
\nabla_pH^T\nabla_qH
=
0.
\]

Energy conservation follows directly from the canonical equations. In a numerical engine this exact continuous invariant need not be exactly preserved by a discrete integrator. That distinction is why energy drift is a property of the time-stepping method, not proof that Hamiltonian mechanics itself is dissipative or explosive.

Source basis: Arnold, Chapter 3 §§14–15 on Legendre transforms and Hamilton's equations, especially the equivalence between Lagrange's equations and the canonical first-order Hamiltonian system.

## Symplectic structure explains why long-time phase behavior is a stronger target than one-step position error

Write canonical phase state as

\[
z=
\begin{bmatrix}
q\\p
\end{bmatrix},
\qquad
J=
\begin{bmatrix}
0&I\\
-I&0
\end{bmatrix}.
\]

Hamilton's equations are

\[
\dot z
=
J\nabla H(z).
\]

The canonical symplectic two-form is represented locally by \(J\). A smooth map \(\Phi\) is symplectic when its Jacobian satisfies

\[
D\Phi(z)^T J D\Phi(z)=J.
\]

This condition is stronger than ordinary volume preservation. Taking determinants implies

\[
|\det D\Phi|=1,
\]

so a symplectic map preserves phase volume, but many volume-preserving maps are not symplectic. Arnold develops both ideas: Liouville's theorem states that Hamiltonian phase flow preserves phase volume, while the symplectic formulation identifies the closed nondegenerate two-form preserved by Hamiltonian flow.

The distinction matters numerically. A generic high-order integrator can have excellent short-time state accuracy yet slowly distort the qualitative geometry of conservative motion. A **symplectic integrator** exactly preserves a nearby discrete symplectic map. It usually does not conserve the original Hamiltonian exactly at each step, but backward-error analysis often explains its bounded, oscillatory long-time energy error as exact motion under a nearby modified Hamiltonian for long intervals.

For the separable Hamiltonian

\[
H(q,p)=T(p)+U(q),
\]

symplectic Euler can be written in one variant as

\[
p_{n+1}
=
p_n-h\nabla U(q_n),
\]

\[
q_{n+1}
=
q_n+h\nabla T(p_{n+1}).
\]

For \(T=p^2/(2m)\), this is the familiar velocity-first/semi-implicit Euler update already used earlier in the engine curriculum. Its value is not that it is magically exact. Its update order gives a symplectic map for this Hamiltonian form, which often produces much better long-time qualitative behavior than explicit Euler.

Hamiltonian mechanics also introduces the **Poisson bracket**

\[
\{f,g\}
=
\nabla f^T J\nabla g.
\]

Along an autonomous Hamiltonian trajectory,

\[
\frac{df}{dt}
=
\{f,H\}.
\]

A quantity \(f\) is conserved when its Poisson bracket with \(H\) vanishes. This gives a compact algebraic language for invariants and canonical transformations. In a game engine one does not need to implement a Poisson-bracket type to benefit from the concept: it explains why position and momentum are paired, why canonical transforms preserve dynamics, and why integrator structure can matter beyond local truncation order.

Do not confuse this with contact/constraint projection. A collision impulse makes the trajectory nonsmooth and can intentionally dissipate energy through restitution/friction. A projected constraint solver can also destroy exact symplecticity. Structure-preserving free-flight integration is still useful, but the full engine is a hybrid dynamical system containing smooth Hamiltonian segments, external forces, constraints, impacts, sleeps, and authored corrections. Each layer should state which invariants it preserves or intentionally breaks.

Source basis: Arnold, Chapters 7–9, especially Hamiltonian phase flow, Liouville's theorem, symplectic manifolds, Hamiltonian vector fields, Poisson brackets, and canonical formalism. The explicit symplectic-Euler bridge is a numerical-analysis application of that structure.

## Turning continuous motion into an engine timestep

The differential equations describe ideal continuous time, but a game engine executes discrete updates. At a fixed timestep `h` seconds, a simple *semi-implicit Euler* translational step is

$$
v_{n+1}=v_n+h F_n/m,\qquad x_{n+1}=x_n+h v_{n+1}.
$$

Notice that position uses the **new** velocity. Explicit Euler would use `v_n`; the difference affects energy behaviour. Neither is exact. Forces can be state-dependent, and an unstable or too-large timestep can cause tunnelling, visible energy injection or constraint jitter. The timestep is part of the numerical model; simply multiplying an arbitrary penetration correction by `h` does not create a physically principled solver.

Rotation can be updated with a unit-quaternion increment derived from `ω h`, but the implementation must declare whether its angular velocity and quaternion multiplication use the world or local frame. Re-normalization treats storage drift, not integration error. For a production solver, decide the order of force integration, collision generation, velocity constraints, position correction, sleep decisions and render interpolation. Their data dependencies must be explicit.

## Why contact changes *both* linear and angular velocity

Now the crate reaches the floor. A collision detector supplies a contact point `p`, normal `n` (unit length, pointing from body A toward B), penetration depth `d` (metres), and optionally stable feature identifiers. This contact record is not itself a solver: it says *where and how* the bodies interfere, but not which changes in velocity satisfy the constraint.

Let `r_A=p-x_A` and `r_B=p-x_B`; point velocities are `u_A=v_A+ω_A×r_A` and `u_B=v_B+ω_B×r_B`. The relative normal speed is

$$
v_n=n\cdot(u_B-u_A).
$$

Negative `v_n` means the bodies approach along the chosen normal. A scalar normal impulse `j≥0` acts as `-jn` on A and `+jn` on B. The resulting linear velocity changes are `Δv_A=-jn/m_A` and `Δv_B=+jn/m_B`. Since the impulses act at offsets, they also change angular velocities:

$$
\Delta\omega_A=-I_A^{-1}(r_A\times jn),\qquad
\Delta\omega_B=+I_B^{-1}(r_B\times jn).
$$

The *effective inverse mass* along this normal is the amount by which a unit normal impulse changes relative normal speed:

$$
k_n=m_A^{-1}+m_B^{-1}
+n\cdot\Big[
(I_A^{-1}(r_A\times n))\times r_A
+(I_B^{-1}(r_B\times n))\times r_B
\Big].
$$

It is not just inverse mass: off-centre impulses also spend effort creating rotation. Treat static/kinematic bodies as zero inverse mass and zero inverse inertia in the impulse solve. Check that `k_n` is finite and positive before division.

For an isolated collision with restitution coefficient `e` in `[0,1]`, and approach speed `v_n<0`, the unconstrained impulse is

$$
j=-(1+e)v_n/k_n.
$$

Restitution models a bounce; `e=0` requests no normal rebound, and `e=1` idealizes perfectly elastic relative normal motion. Applying this formula independently to a stack of many simultaneous contacts is not a complete stack solver. A contact at rest should not receive a new bounce on every solver iteration: restitution is normally gated by a meaningful approaching-speed threshold.

**Numerical witness.** Two equal 2-kg bodies strike head-on at their centres with `v_A=+3 m/s`, `v_B=-1 m/s`, `n=+1`, and `e=0.5`. The relative speed is `v_n=-4 m/s`; because both lever arms vanish, `k_n=1/2+1/2=1 kg⁻¹`. Thus `j=6 N·s`. The final velocities are `v'_A=3-6/2=0 m/s` and `v'_B=-1+6/2=2 m/s`; the final separation speed is `2 m/s`, half the incoming closing speed. Total momentum is `2·3+2·(-1)=4 kg·m/s` before and `2·0+2·2=4 kg·m/s` after. With an off-centre contact, this scalar calculation is insufficient unless the rotational terms are included in `k_n`.

## How a resting contact avoids the endless-bounce problem

Stacks, resting crates and multi-point manifolds require a *persistent constraint solve*. Store each normal impulse's accumulated magnitude `λ≥0`. During a projected Gauss–Seidel iteration compute an incremental request `Δλ`, clamp the **total** accumulated impulse,

$$
\lambda_{\mathrm{new}}=\max(0,\lambda_{\mathrm{old}}+\Delta\lambda),\qquad
\Delta\lambda_{\mathrm{applied}}=\lambda_{\mathrm{new}}-\lambda_{\mathrm{old}}.
$$

Only apply the second quantity. Clamping the incremental impulse alone forbids an iteration from *removing* an excessive earlier impulse, so it can create artificial forces. Cached impulses from the previous frame can warm-start the solver when contact identifiers, normals and body ownership genuinely match. Do not carry impulses across unrelated contacts or silently reuse stale contact features after a topology change.

For penetration correction distinguish three mechanisms: a velocity-level bias that asks the next step to separate, a position-level correction that directly repairs overlap, and substepping/continuous collision detection that prevents some deep penetrations in the first place. Bias has velocity units: a typical bounded Baumgarte-style term has scale `β max(d-s,0)/h`, where `β` is dimensionless and `s` is a slop allowance in metres. This is *not* a proof of physical energy correctness; too much bias can create bounce, especially with a changing timestep. Position-only repairs can move objects without imparting consistent velocity. Fixed timesteps, capped correction, split impulses, and enough solver iterations each address different failure modes.

Friction applies impulses in the plane tangent to `n`. Choose tangential directions from the relative contact velocity (or a robust orthonormal basis when nearly stationary). The Coulomb bound is `|λ_t|≤μ λ_n` for a single tangential direction; a two-dimensional tangent plane has a disk or cone bound, not independently unconstrained scalar impulses. `μ` is dimensionless. Static and dynamic friction require an explicit model/approximation rather than treating `μ` as a magic damping constant.

## The live engine contract

The minimum coherent system owns: stable body IDs; poses and current/previous transforms; linear and angular velocities; mass/inverse mass; body-space inertia and orientation-dependent world inverse inertia; collision shapes; force/torque accumulators; and contact manifolds with feature IDs and accumulated impulses. The update loop is: accumulate inputs/forces → advance free velocities → update collision proxies → broad-phase potential pairs → narrow-phase contacts → warm start validated contacts → iterative velocity/friction solve → integrate poses and repair position as required → update sleep state → publish an immutable render snapshot. Broad phase must never be mistaken for proof of collision; it produces candidates that narrow phase tests. The renderer must not race writes into the physics owner's mutable pose array.

The pressure-plate gameplay rule should query a **physical contact or overlap event** with stable identifiers, rather than infer activation from a rendered sprite. Deterministic replay requires declared timestep, stable processing order, controlled floating-point environment where possible, and recorded inputs. It is not guaranteed merely because a fixed timestep was selected. A useful diagnostic stream records frame number, contact pair IDs, normal, penetration, relative normal speed, accumulated impulse and number of iterations.

## Reconstruction and failure tests

With an empty body list, there should be no contact events. Two approaching, centrally aligned bodies should conserve total linear momentum under internal impulses. An off-centre impulse should change angular momentum with sign matching `r×J`. Two separating bodies should not receive an attractive normal impulse. A resting stack should approach stable relative velocity without a fresh restitution bounce each frame. Static bodies should not be moved by inverse-mass division; a contact with zero total effective inverse mass should be reported as unsolvable rather than divided by zero. Rotating a rectangular crate should rotate its **world** inverse inertia, while its body-space inertia remains fixed. Finally, reducing `h` while holding the physical scenario constant should expose numerical convergence or non-convergence—not an unexplained change of physical laws.

The accompanying [standalone C++20 impulse witness](../../tools/labs/rigid_body_contact_lab.cpp) checks a fully specified central 1D collision and the projected accumulated-impulse rule. It is deliberately *not* a complete 3D rigid-body engine. In particular, it does not certify contact generation, inertia tensor rotation, friction cones or GPU execution. The chapter's equations make those missing production responsibilities explicit.

## How the next engineering problem follows

We have now obtained a physically meaningful state, derived the effects of an off-centre contact, and specified an iterative constraint update. The remaining engine-design challenge is not another isolated vocabulary item: it is how to preserve these invariants as contacts are created, persist or disappear across frames while rendering, networking, editing and gameplay all observe a coherent world. Those ownership and synchronization questions arise because the physical model now requires a stateful, reproducible execution pipeline.

---

## Depth-repair checkpoint — execution evidence behind the foundations

The canonical story now has a stricter companion contract: [[Supplementary/Mastery Evidence Ledger - 2026-11-12|the evidence ledger]] distinguishes explained/derived/reference/native/production/reconstructed levels, while `Supplementary/mastery_chunk_evidence.csv` names every M001–M180 chunk and its exact original-part interval.

For CPU scheduling, `tools/labs/work_stealing_scheduler_lab.cpp` continues the shared-queue job-pool discussion with owner-LIFO/thief-FIFO local queues, forced load imbalance, cross-worker steals, exactly-once execution and a deterministic integer payload oracle. It remains a mutex-protected semantic reference rather than a claim of lock-free Chase–Lev ordering or NUMA-optimal scheduling.

For GPU algorithms, `tools/labs/gpu_parallel_primitives_lab.cpp` turns scan/compaction/sort terminology into an independent CPU oracle: hierarchical exclusive scan must match a serial scan across many block sizes; predicate-scan-scatter preserves stable compaction order; byte histograms feed a stable four-pass 32-bit LSD radix sort checked against the standard numeric order. GPU barriers, subgroup implementations, occupancy and bank-conflict behavior still require native device evidence.

For compressed inference, the earlier Phase 29 reference is now joined by `tools/labs/gguf_metadata_tensor_lab.cpp`, which advances GGUF validation beyond the prefix into typed metadata, alignment and tensor descriptors. The boundary remains explicit: full metadata arrays/type catalogs, every quantized block, tokenizer semantics, ONNX protobuf/opset execution, SIMD/Metal kernels and trusted-model parity are later evidence levels rather than implied by a successful subset parser.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline|Previous chapter]]
