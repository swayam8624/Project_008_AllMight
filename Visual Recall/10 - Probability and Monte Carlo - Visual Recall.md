# Probability and uncertainty — visual recall

```mermaid
flowchart LR
 M["Model and data scale"] --> D["Discretization choice"] --> A["Stable arithmetic"] --> S["Sampling and reduction"] --> E["Error budget and tests"]
 K["Problem conditioning"] -. "limits attainable accuracy" .-> E
 class M,D,A,S blue;class K amber;class E green;
classDef blue fill:#e5eff9,stroke:#2463a6,color:#18354c;
classDef green fill:#e3f3ed,stroke:#16806a,color:#174b3c;
classDef amber fill:#fff0dc,stroke:#c57820,color:#64400d;
```


Shortest route: start with a model of outcomes, then ask what distribution the computer actually sampled, how it weighted each sample, and what evidence supports the estimate.

## Reconstruct the chain

`(Ω, F, P)` describes outcomes, measurable events, and their probabilities. A random variable maps outcomes into numbers; a random vector preserves their joint behavior. A PMF assigns point mass to discrete values; a PDF gives density relative to a stated measure; a CDF accumulates probability and exists even when no PDF does.

Conditioning changes the information available. Independence means joint probability factors, not merely that events are disjoint. Bayes reverses the conditioning direction by multiplying likelihood by prior and normalizing over alternatives.

Expectation is a probability-weighted center. Variance is mean squared spread. Covariance records coupled variation; correlation divides it by both standard deviations but only captures linear association. For a vector, covariance is symmetric PSD because `aᵀΣa = Var(aᵀX) ≥ 0`.

Bernoulli describes one binary outcome; binomial counts IID Bernoulli successes. Uniform has constant density in its declared measure. Gaussian describes a quadratic-distance bell shape. Exponential waiting times are memoryless; Poisson counts belong to the corresponding constant-rate process. A multivariate Gaussian's covariance sets ellipse/ellipsoid orientation and scale.

The computational chain is:

```text
seed/state or keyed input → finite random bits → sample transform → actual PDF and measure
                              → weighted contribution f(X)/p(X) → average → uncertainty/error report
```

Every arrow carries assumptions. A wrong PDF or measure can bias the result. A low-variance estimator can still be biased. Correlated observations change the standard error. Confidence intervals do not repair a wrong expectation.

## Formula prompts

| Prompt | Reconstruct |
|---|---|
| Conditioning | `P(A|B)=P(A∩B)/P(B)`; chain rule multiplies successive conditional probabilities. |
| Bayes | `P(H|E)=P(E|H)P(H)/ΣᵢP(E|Hᵢ)P(Hᵢ)`. |
| Expected transform | `E[g(X)] = Σg(x)p(x)` or `∫g(x)p(x)dx`. |
| Variance/covariance | `Var(X)=E[X²]−E[X]²`; `Cov(X,Y)=E[XY]−E[X]E[Y]`. |
| Linear propagation | `E[AX+b]=Aμ+b`; `Cov(AX+b)=AΣAᵀ`; local nonlinear approximation uses `JΣJᵀ`. |
| Importance estimate | `I=∫f dx=Eₚ[f(X)/p(X)]`; average contributions and divide by the actual sampling density. |
| Monte Carlo error | IID finite-variance mean has `Var(Ȳ)=σ²/N`, `SE=σ/√N`; four times samples halves SE. |
| Estimator quality | `MSE = variance + bias²`. |
| Bessel/Welford | Unbiased sample variance uses `M₂/(n−1)`; update with `δ=x−μ`, `μ←μ+δ/n`, `M₂←M₂+δ(x−μ)`. |
| Error metrics | MSE squares; RMSE returns original units; MAE uses absolute error; relative metrics fail or need policy near zero. |

## Fast error checks

- Does a density integrate to one in the stated coordinate measure? Is its unit inverse measure?
- Are the samples distributed according to the PDF used in the weight?
- Is a stated confidence interval fixed-sample, and are IID/normal or approximation assumptions plausible?
- Is covariance symmetric PSD? Does singular mean invalid, or valid but lower-rank?
- Are sample and population variance being confused? Is a single run being mistaken for evidence of variance reduction?
- Is a “Gaussian” a normalized probability model or merely a rendering kernel?

## Targeted backlinks

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A direction can exist even when the naive length overflows|stable numerics]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#An inverse can exist and still amplify uncertainty|conditioning]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Coordinates can change while the geometric object stays still|coordinate changes]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Positive definiteness is a structural promise|positive definiteness]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Diagonalization leads to the stronger symmetric spectral theorem|covariance eigenstructure]]

Parts covered by supplied sources: 901–1000. The numerical continuation and figure index below complete this received phase.

## Source-provided recall and compact formula reference

## Supplied concise recall summary

The mathematical core is the probability space $(\Omega,\mathcal F,P)$, random variables defined on that space, and probability laws for their values.

Conditional probability changes the reference information. Independence is a factorization property. PMFs assign masses to discrete values, while PDFs assign density relative to a measure. CDFs accumulate probability and exist for every real-valued random variable.

Expected value describes the weighted centre. Variance describes squared spread. Covariance describes coupled spread. Correlation standardizes covariance but does not measure every form of dependence.

Bernoulli models one binary trial. Binomial models the count of successes in independent identically distributed Bernoulli trials. Uniform models constant density over a region. Gaussian models a particular smooth bell-shaped distribution. Exponential models memoryless waiting times. Poisson models event counts under its defining assumptions.

A multivariate Gaussian combines mean, covariance, and correlations into a joint distribution. Positive-definite covariance permits a full-dimensional nonsingular Gaussian density. Singular PSD covariance describes a distribution confined to a lower-dimensional affine subspace.

Key engineering invariants: all probabilities are valid and normalized; PDF units match the integration measure; covariance is symmetric PSD; Gaussian density evaluation avoids unnecessary inversion; nonfinite inputs and singular cases are explicit; sampling tests use statistical tolerances rather than exact equality.

## Supplied compact formula reference

Random generation and transformations

$$
 U\sim\mathcal U(0,1),\qquad X=F^{-1}(U) 
$$

$$
 p_Y(g(x)) = \frac{p_X(x)}{|\det J_g(x)|} 
$$

Importance sampling

$$
 \widehat I_N= \frac1N\sum_i\frac{f(X_i)}{p(X_i)} 
$$

$$
 \operatorname{Var}(\widehat I_N) = \frac1N \left[ \int\frac{f(x)^2}{p(x)}dx-I^2 \right] 
$$

Statistical estimation

$$
 \bar X=\frac1N\sum_iX_i 
$$

$$
 s^2=\frac1{N-1}\sum_i(X_i-\bar X)^2 
$$

$$
 \operatorname{SE}(\bar X)\approx\frac{s}{\sqrt N} 
$$

Welford update

$$
 \delta=x-\mu\_{\mathrm{old}} 
$$

$$
 \mu\_{\mathrm{new}}= \mu\_{\mathrm{old}}+\frac{\delta}{n} 
$$

$$
 M\_{2,\mathrm{new}}= M\_{2,\mathrm{old}}+ \delta(x-\mu\_{\mathrm{new}}) 
$$

Estimator quality

$$
 \operatorname{MSE}= \operatorname{Variance}+ \operatorname{Bias}^2 
$$

Prediction error

$$
 \operatorname{RMSE}= \sqrt{\frac1N\sum_i(\hat y_i-y_i)^2} 
$$

$$
 \operatorname{MAE}= \frac1N\sum_i|\hat y_i-y_i| 
$$

## Cholesky whitening

From Sigma=LL transpose and x=mu+Lz, solve z=inverse(L)(x-mu); then Mahalanobis squared distance is z transpose z. A singular covariance is supported on a lower-dimensional affine subspace.

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Original bridge: whiten a covariance ellipse|Rebuild the numerical witness]].

## Six figures

![[Visual Recall/Math Foundations/F10-1_dice-sum.png]]

**F10-1. Ordered outcomes and the exact two-dice sum PMF.** The supplied lesson already derives this example; the figure is a visual check of the 36 ordered pairs. PMF numerators sum to 36 and P(S=7)=1/6.

![[Visual Recall/Math Foundations/F10-2_covariance-dependence.png]]

**F10-2. Zero covariance need not mean independence.** For standard normal X, Cov(X,X squared)=0 by symmetry although X and X squared are dependent. Sample covariance is a finite-run witness.

![[Visual Recall/Math Foundations/F10-3_uniform-disk-sampling.png]]

**F10-3. Uniform area sampling requires a square-root radial transform.** The theoretical mean radius is 2/3 for uniform area in the unit disk and 1/2 for uniform radius; plotted sample means are finite-run illustrations.

![[Visual Recall/Math Foundations/F10-4_mc-error-bias-floor.png]]

**F10-4. Monte Carlo random error and a fixed bias floor.** For the stated integral, unbiased RMSE falls with sample count while the biased estimator retains its offset.

![[Visual Recall/Math Foundations/F10-5_importance-constant-weight.png]]

**F10-5. A matched importance proposal can make contributions constant.** For the stated nonnegative integrand and support, the matched proposal gives constant contributions; uniform-proposal variance is 4/45. This does not generalize to arbitrary signed integrands.

![[Visual Recall/Math Foundations/F10-6_toy-lcg-stream.png]]

**F10-6. A full-period toy LCG can still have a simple low-bit pattern.** For m=16, a=5, c=1, all 16 states are visited but the least-significant bit alternates. Full period does not imply good multidimensional quality. The keyed-input sketch indicates deterministic indexing only: a different key does not establish statistical independence or collision freedom.

## Phase 10 concept key map

Use each keyword for the concept note and the adjacent section link for the full derivation or example.

| Concept keyword | Full teaching section |
|---|---|
| [[Probability spaces, events, and conditioning|sample space]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#1. A probability model begins with possible outcomes\|Phase 10 section]] |
| [[Random variables and probability laws|random law]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#2. Random quantities and their probability laws\|Phase 10 section]] |
| [[Common probability distributions and their assumptions|normal distribution]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#4. Discrete and continuous distribution families\|Phase 10 section]] |
| [[Expectation, covariance, and dependence|first and second moments]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#3. Expected values describe centers; covariance describes dependence\|Phase 10 section]] |
| [[Multivariate Gaussian distributions and whitening|normal vector]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry\|Phase 10 section]] |
| [[Conditional Gaussians and Schur complements|Schur complement covariance]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry\|Phase 10 section]] |
| [[Stateful pseudorandom generators and stream state|engine state]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences\|Phase 10 section]] |
| [[Stateless keyed sampling and reproducibility|counter-based RNG]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences\|Phase 10 section]] |
| [[Sampling quality: stratification, quasi-Monte Carlo, and blue noise|quasi-Monte Carlo]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences\|Phase 10 section]] |
| [[Inverse, rejection, and importance sampling|inverse transform]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#7. Transformations turn simple inputs into useful samples\|Phase 10 section]] |
| [[Online moments and Welford accumulation|online variance]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#9. Finite samples need stable descriptive statistics\|Phase 10 section]] |
| [[Confidence intervals and sequential evidence|coverage interval]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#10. Inference and error metrics need explicit assumptions\|Phase 10 section]] |
| [[Robust summaries and error metrics|quantile]] | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#10. Inference and error metrics need explicit assumptions\|Phase 10 section]] |

## The chain

Input uncertainty is amplified by conditioning; finite-precision algorithms add their own error. Keep **problem sensitivity**, **algorithm stability**, **approximation error**, and **sampling uncertainty** distinct. A small residual or update is evidence only when assumptions provide an error bound.

## Formula prompts

| Prompt | Rebuild |
|---|---|
| First-order propagation | $\Delta y\approx J_f(x)\Delta x$; define units and perturbation scale. |
| Multiplication | Relative errors approximately add to first order. |
| Summation | $\gamma_n=nu/(1-nu)$; cancellation magnifies relative error. Pairwise depth is $O(\log n)$; compensation tracks lost low-order bits. |
| Stable norm | $m\sqrt{\sum_i(x_i/m)^2}$ with $m=\max_i|x_i|$; handle $m=0$. |
| Stable quadratic roots | Choose a noncancelling $q$, then roots $q/a$ and $c/q$; separately handle degenerate and range cases. |
| Linear interpolation | $(1-t)y_0+ty_1$ for $t\in[0,1]$; bounds require convex weights. |
| Bisection | Continuous sign-changing bracket; interval width halves each step. |
| Newton | $x_{n+1}=x_n-f(x_n)/f'(x_n)$; local quadratic convergence needs a simple root and a good neighborhood. |
| Error balance | Truncation often falls with $h^p$ while finite-difference roundoff grows like $u/h$; balance before shrinking $h$ indefinitely. |
| Convergence order | Compare $Q_h,Q_{h/2},Q_{h/4}$; Richardson cancels a leading $Ch^p$ term only in the asymptotic regime. |
| Log-sum-exp | $m+\log\sum_i e^{a_i-m}$, $m=\max_i a_i$; validate nonfinite/empty cases. |
| Stratified estimate | Sample every stratum and use weights matching its measure; arbitrary non-square grids must still cover the full domain. |

## Checks before trusting a result

- State arithmetic assumptions (normal/subnormal, overflow, rounding mode) and data scale.
- Check both residual/backward error and conditioning before interpreting forward accuracy.
- Verify interpolation range, boundary policy, and parameter units.
- Separate discretization, truncation, roundoff, and sampling error.
- Fix a reproducible reduction tree or record seed, sample design, and uncertainty.
- Test exact identities, invariants, edge cases, convergence order, and randomized strata.

## Continue in the volume

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Floating-point answers: error, conditioning, stability, and stable arithmetic|Error and arithmetic]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Reconstructing values and curves between samples|Interpolation and curves]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Finding roots with guarantees and convergence checks|Root solving]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Separate discretization, truncation, and roundoff; measure convergence|Error sources and convergence]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep probability weights stable before sampling|Stable weights]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Design samples: without replacement, stratified, and low-discrepancy|Sampling design]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Test numerical methods and benchmark reproducibly|Numerical tests]]

## Numerical reliability figures and their explanations

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-7 — Summation is an algorithm, not just a plus sign|N10-7 · Summation is an algorithm, not just a plus sign]] — Exact real sum is 10000. The three-term counterexample shows why Kahan is not an exact-arithmetic guarantee. Strict IEEE-style evaluation is assumed; reassociation can invalidate compensation.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-8 — More polynomial degree does not guarantee a better picture|N10-8 · More polynomial degree does not guarantee a better picture]] — Both interpolants match every node. Their behaviour between nodes differs. Chebyshev extrema reduce this example’s endpoint oscillation; this comparison is not a universal interpolation error bound.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-9 — Bézier and Hermite can tell the same curve story|N10-9 · Bézier and Hermite can tell the same curve story]] — Use Hermite tangents 3(P1−P0) and 3(P3−P2) for this cubic Bézier curve. The control polygon bounds the curve through nonnegative weights summing to one; inner control points need not lie on the curve.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-10 — A root solver needs both progress and a safety argument|N10-10 · A root solver needs both progress and a safety argument]] — The error comparison uses cos(x)−x with specified starting values. Bisection has a shrinking bracket; Newton and secant need additional hypotheses. The plotting floor is explicitly shown rather than interpreted as continuing convergence.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-11 — Arbitrary sample counts need full-domain support|N10-11 · Arbitrary sample counts need full-domain support]] — Left: choosing the first five of nine equal cells and averaging equally gives E[y] = 0.3, while the uniform-square value is 0.5. Right: one point per marginal bin is a midpoint illustration; randomized LHS jitters and permutes appropriately, and is not full multidimensional stratification.

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure N10-12 — Algebraically equal formulas can have unequal numerical error|N10-12 · Algebraically equal formulas can have unequal numerical error]] — The reference uses 90-digit Decimal arithmetic and compares binary64 formulas for b from 100 to 10¹⁶. This bounded example illustrates cancellation, not a complete robust quadratic solver.

## Linked numerical concepts

[[Concepts/Algorithmic stability|Algorithmic stability]] · [[Concepts/Stable quadratic root evaluation|Stable quadratic root evaluation]] · [[Concepts/Interpolation versus approximation|Interpolation versus approximation]] · [[Concepts/Tensor-product interpolation|Tensor-product interpolation]] · [[Concepts/Cubic Hermite interpolation|Cubic Hermite interpolation]] · [[Concepts/Bézier curves and De Casteljau evaluation|Bézier curves and De Casteljau evaluation]] · [[Concepts/Bracketed and open root solvers|Bracketed and open root solvers]] · [[Concepts/A posteriori error bounds for contractive iteration|A posteriori error bounds for contractive iteration]] · [[Concepts/Discretization, truncation, and roundoff|Discretization, truncation, and roundoff]] · [[Concepts/Convergence order and Richardson extrapolation|Convergence order and Richardson extrapolation]] · [[Concepts/Stable probability normalization and log-sum-exp|Stable probability normalization and log-sum-exp]] · [[Concepts/Uniform shuffling without replacement|Uniform shuffling without replacement]] · [[Concepts/Latin-hypercube sampling|Latin-hypercube sampling]] · [[Concepts/Numerical error budgets|Numerical error budgets]] · [[Concepts/Reproducible numerical benchmarking|Reproducible numerical benchmarking]]
