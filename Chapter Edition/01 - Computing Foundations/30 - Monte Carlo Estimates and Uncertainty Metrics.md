# 30 — Monte Carlo Estimates and Uncertainty Metrics

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|Next chapter]]

Monte Carlo methods trade computation for uncertainty, with error shrinking according to sampling assumptions rather than dimension in the same way as grids. This chapter derives the estimator, works numerical examples, and chooses image-error metrics that match the evaluation goal.

**By the end:** Estimate expectations from random samples, quantify uncertainty, and select an error metric whose weighting reflects the intended comparison.

**Recall question:** Why does independent Monte Carlo standard error scale as $1/\sqrt{N}$, and what does that scaling not guarantee?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 72218–75691; source-span SHA256 6b7237dab5d2dfc78a068b1981f142645a840bd9044bc08ce18094382fc321ee. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## 8. Monte Carlo estimates trade computation for uncertainty

### Monte Carlo estimation

Monte Carlo estimation uses randomized sampling to approximate an expectation or integral.

Suppose:

$$
 I=\mathbb E[Y] 
$$

and we can generate independent identically distributed samples:

$$
 Y_1,\ldots,Y_N. 
$$

The sample-mean estimator is:

$$
 \boxed{ \widehat I_N= \frac1N\sum\_{i=1}^NY_i }. 
$$

If the expectation exists, this is a natural estimator.

If $Y$ has finite variance:

$$
 \sigma_Y^2=\operatorname{Var}(Y), 
$$

then:

$$
 \boxed{ \operatorname{Var}(\widehat I_N) = \frac{\sigma_Y^2}{N} } 
$$

for independent samples.

Its standard deviation, often called the standard error of the mean, is:

$$
 \boxed{ \operatorname{SE}(\widehat I_N) = \frac{\sigma_Y}{\sqrt N} }. 
$$

#### 1 Why the convergence rate is $1/\sqrt N$

Let:

$$
 \widehat I_N= \frac1N\sum_iY_i. 
$$

For independent samples:

$$
 \operatorname{Var}\left(\sum_iY_i\right) = \sum_i\operatorname{Var}(Y_i) =N\sigma_Y^2. 
$$

Scaling variance by $1/N^2$:

$$
 \operatorname{Var}(\widehat I_N) = \frac{N\sigma_Y^2}{N^2} = \frac{\sigma_Y^2}{N}. 
$$

Take the square root:

$$
 \operatorname{SE}= \frac{\sigma_Y}{\sqrt N}. 
$$

Therefore halving the standard error requires increasing the number of samples by a factor of four.

To reduce it by a factor of ten, we need approximately 100 times as many samples.

This scaling is central to Monte Carlo rendering performance.

#### 2 A numerical example

Suppose an estimator contribution has variance:

$$
 \sigma_Y^2=0.09. 
$$

Then:

$$
 \sigma_Y=0.3. 
$$

With:

$$
 N=100, 
$$

the standard error is:

$$
 \operatorname{SE} = \frac{0.3}{10} =\boxed{0.03}. 
$$

With:

$$
 N=10{,}000, 
$$

it becomes:

$$
 \frac{0.3}{100} =\boxed{0.003}. 
$$

We used 100 times more samples to reduce the standard error tenfold.

#### 3 Why dimensionality matters differently in Monte Carlo

Deterministic grid quadrature can suffer dramatically as dimension increases.

For example, using 100 samples along each of ten dimensions would require:

$$
 100^{10}=10^{20} 
$$

grid points.

Ordinary independent Monte Carlo retains the $1/\sqrt N$ standard-error relationship without an explicit exponent depending on the dimension.

This is a major reason Monte Carlo is attractive for high-dimensional light transport.

But dimension can still affect the variance enormously.

A poor sampling distribution in high dimension may give rare, extremely large weights, resulting in practical convergence that is unusably slow.

So Monte Carlo is not immune to high-dimensional difficulty; its classical independent-sample rate simply has a different dimension dependence from tensor-product quadrature.

### Unbiased estimators

An estimator $\widehat\theta$ is a random quantity constructed from observed samples.

It estimates an unknown fixed parameter $\theta$.

The estimator is unbiased if:

$$
 \boxed{ \mathbb E[\widehat\theta]=\theta }. 
$$

Unbiasedness is a statement about the estimator's average behavior over repeated hypothetical datasets or sample sequences.

It does not mean every individual estimate equals the truth.

#### 1 The sample mean is unbiased

Let:

$$
 X_1,\ldots,X_N 
$$

have common finite mean:

$$
 \mathbb E[X_i]=\mu. 
$$

Define:

$$
 \bar X=\frac1N\sum_iX_i. 
$$

Then:

$$
 \begin{aligned} \mathbb E[\bar X] &=\frac1N\sum_i\mathbb E[X_i]\\ &=\frac1N(N\mu)\\ &=\mu. \end{aligned} 
$$

Therefore:

$$
 \boxed{\mathbb E[\bar X]=\mu}. 
$$

Independence is not required for this unbiasedness derivation, although independence matters for its usual variance formula and many convergence results.

#### 2 Unbiasedness of Monte Carlo integration

For:

$$
 \widehat I_N= \frac1N\sum_i\frac{f(X_i)}{p(X_i)}, 
$$

we derived:

$$
 \mathbb E[\widehat I_N]=I. 
$$

This property depends on using the correct PDF, valid support, and well-defined expectation.

If the renderer samples using one density but divides by another, the equality can fail.

#### 3 An unbiased estimator can have enormous variance

Consider a random estimate:

$$
 \widehat\theta= \begin{cases} 0,&\text{probability }0.99,\\ 100,&\text{probability }0.01. \end{cases} 
$$

Its mean is:

$$
 \mathbb E[\widehat\theta]=1. 
$$

If the true parameter is $\theta=1$, the estimator is unbiased.

But 99% of its individual outputs are zero.

Its second moment is:

$$
 \mathbb E[\widehat\theta^2] = 0.99(0)+0.01(10000)=100. 
$$

Its variance is:

$$
 100-1^2=\boxed{99}. 
$$

This is an unbiased but extremely noisy estimator.

Unbiasedness alone is not a sufficient quality criterion.

### Biased estimators

The bias of an estimator is:

$$
 \boxed{ \operatorname{Bias}(\widehat\theta) = \mathbb E[\widehat\theta]-\theta }. 
$$

An estimator is biased when its expectation differs from the target parameter.

Bias can arise intentionally through regularization, truncation, clamping, approximate models, or statistical correction choices.

It can also arise accidentally through incorrect weighting or broken sampling logic.

#### 1 A simple biased estimator

Suppose $X_i$ have mean $\mu$.

Define:

$$
 \widehat\mu= \frac1{N+1}\sum\_{i=1}^NX_i. 
$$

Then:

$$
 \mathbb E[\widehat\mu] = \frac{N}{N+1}\mu. 
$$

Therefore the bias is:

$$
 \boxed{ -\frac{\mu}{N+1} }. 
$$

The estimator approaches unbiasedness as $N$ grows, but it is biased for finite $N$ unless $\mu=0$.

#### 2 Bias from clamping fireflies

Suppose a rendering algorithm estimates radiance from a random contribution $Y\ge0$.

Large contributions occasionally occur.

To reduce visible fireflies, an implementation may clamp:

$$
 Y\_{\mathrm{clamped}}=\min(Y,T). 
$$

Since:

$$
 Y\_{\mathrm{clamped}}\le Y 
$$

for every sample:

$$
 \mathbb E[Y\_{\mathrm{clamped}}] \le \mathbb E[Y]. 
$$

Unless no contributions exceed $T$, the inequality is strict under suitable integrability conditions.

The estimator becomes negatively biased.

The image may look cleaner, but its expected brightness is changed.

This does not make clamping universally unacceptable. It means its variance reduction comes with bias and should be treated as an explicit quality trade-off.

#### 3 Self-normalized importance sampling

Suppose we know a target distribution only up to a normalizing constant.

We may use an estimator:

$$
 \widehat\mu= \frac{ \sum_iw_i f(X_i) }{ \sum_iw_i }. 
$$

This ratio is often called a self-normalized importance-sampling estimator.

It can be consistent under appropriate conditions, but it is generally biased at finite sample count because:

$$
 \mathbb E\left[\frac AB\right] \ne \frac{\mathbb E[A]}{\mathbb E[B]} 
$$

in general.

This is a good example of why an estimator can be mathematically reasonable and still not be exactly unbiased.

### Estimator variance

Bias measures systematic average displacement from the target. Repeated measurements reduce random variation in the average but cannot remove a calibration offset: a miscalibrated instrument can converge precisely to the wrong mean. MIT 18.05 Class 6b emphasizes this distinction between random error and measurement bias. [MIT 18.05 Class 6b notes](https://ocw.mit.edu/courses/18-05-introduction-to-probability-and-statistics-spring-2022/mit18_05_s22_classC0-prep-b.pdf)

Variance measures how much the estimator changes across repeated independent datasets or runs.

For an estimator:

$$
 \widehat\theta, 
$$

its variance is:

$$
 \boxed{ \operatorname{Var}(\widehat\theta) = \mathbb E[ (\widehat\theta-\mathbb E[\widehat\theta])^2 ] }. 
$$

A low-variance estimator is consistent across repeated experiments, but it may be consistently wrong if it is biased.

#### 1 The bias–variance decomposition

A central quantity is mean squared error:

$$
 \operatorname{MSE}(\widehat\theta) = \mathbb E[(\widehat\theta-\theta)^2]. 
$$

Insert and subtract the estimator's mean:

$$
 \widehat\theta-\theta = (\widehat\theta-\mathbb E[\widehat\theta]) + (\mathbb E[\widehat\theta]-\theta). 
$$

Square:

$$
 \begin{aligned} (\widehat\theta-\theta)^2 &= (\widehat\theta-\mathbb E[\widehat\theta])^2\\ &\quad+ 2(\widehat\theta-\mathbb E[\widehat\theta]) (\mathbb E[\widehat\theta]-\theta)\\ &\quad+ (\mathbb E[\widehat\theta]-\theta)^2. \end{aligned} 
$$

Take expectations.

The cross term vanishes because:

$$
 \mathbb E[ \widehat\theta-\mathbb E[\widehat\theta] ]=0. 
$$

Therefore:

$$
 \boxed{ \operatorname{MSE}(\widehat\theta) = \operatorname{Var}(\widehat\theta) + \operatorname{Bias}(\widehat\theta)^2 }. 
$$

This is one of the most important equations in statistical estimation.

#### 2 Why a biased estimator can have smaller MSE

Suppose estimator A is unbiased:

$$
 \operatorname{Bias}(A)=0, 
$$

but has:

$$
 \operatorname{Var}(A)=9. 
$$

Then:

$$
 \operatorname{MSE}(A)=9. 
$$

Estimator B has bias:

$$
 \operatorname{Bias}(B)=1 
$$

and variance:

$$
 \operatorname{Var}(B)=2. 
$$

Then:

$$
 \operatorname{MSE}(B)=2+1^2=\boxed3. 
$$

Estimator B has smaller mean squared error despite being biased.

That is why estimator selection cannot rely exclusively on unbiasedness.

In rendering, some controlled-bias methods may produce smaller perceptual or numerical error at finite computational budgets. But their biases must be understood, measured, and reported rather than concealed.

#### 3 Correlated samples change variance

For:

$$
 \bar Y=\frac1N\sum_iY_i, 
$$

the general variance formula is:

$$
 \boxed{ \operatorname{Var}(\bar Y) = \frac1{N^2} \left[ \sum_i\operatorname{Var}(Y_i) + 2\sum\_{i\<j}\operatorname{Cov}(Y_i,Y_j) \right] }. 
$$

The familiar:

$$
 \sigma^2/N 
$$

appears only when the samples have common variance and pairwise covariances vanish.

Independent sampling is a sufficient condition.

If samples are positively correlated, variance can decrease more slowly.

If samples have suitable negative correlations, the average variance can be reduced.

This provides a mathematical route to understanding antithetic sampling and stratification.

### Law of large numbers

The law of large numbers explains why sample averages can approach expected values as more observations are collected.

For independent identically distributed random variables with finite mean $\mu$, the strong law states under its standard assumptions:

$$
 \boxed{ \bar X_N= \frac1N\sum\_{i=1}^NX_i \longrightarrow\mu \quad\text{almost surely} }. 
$$

Almost-sure convergence means that, with probability one, the infinite sequence of sample averages converges to the expectation.

The weak law describes convergence in probability:

$$
 \boxed{ P(|\bar X_N-\mu|>\varepsilon) \longrightarrow0 } 
$$

for every $\varepsilon>0$.

These are related but distinct modes of convergence.

#### 1 A derivation of the weak-law intuition using Chebyshev

Assume independent identically distributed samples with finite variance $\sigma^2$.

We know:

$$
 \operatorname{Var}(\bar X_N)=\frac{\sigma^2}{N}. 
$$

Chebyshev's inequality states:

$$
 P(|Y-\mathbb E[Y]|\ge\varepsilon) \le \frac{\operatorname{Var}(Y)}{\varepsilon^2}. 
$$

Apply it to the sample mean:

$$
 P(|\bar X_N-\mu|\ge\varepsilon) \le \frac{\sigma^2}{N\varepsilon^2}. 
$$

As $N\to\infty$:

$$
 \frac{\sigma^2}{N\varepsilon^2}\to0. 
$$

Therefore:

$$
 \boxed{ P(|\bar X_N-\mu|\ge\varepsilon)\to0 }. 
$$

This proves weak convergence under the finite-variance IID assumptions.

The broader laws of large numbers can apply under assumptions different from these, but the derivation shows why decreasing sample-mean variance is so useful.

#### 2 LLN does not give an immediate finite-sample guarantee

The law of large numbers says what happens asymptotically.

It does not tell us that 100 samples, 1,000 samples, or any particular finite count must produce a sufficiently accurate result.

To reason about finite-sample uncertainty, we need variance estimates, concentration inequalities, confidence intervals, or other probabilistic error bounds.

#### 3 When the mean does not exist

Some heavy-tailed distributions do not have a finite expected value.

For example, a standard Cauchy distribution has no defined finite mean.

Its sample average does not settle toward an ordinary finite population mean in the same way as IID samples with finite expectation.

A renderer or stochastic algorithm whose contribution distribution has pathological tails may violate assumptions behind familiar convergence arguments.

Thus, finite expectation and finite variance are not merely technical decorations. They determine which statistical tools are valid.

### Central limit theorem

The law of large numbers tells us where the sample mean tends to go.

The central limit theorem describes the shape of its fluctuations around the true mean after suitable scaling.

Suppose:

$$
 X_1,\ldots,X_N 
$$

are IID with:

$$
 \mathbb E[X_i]=\mu 
$$

and:

$$
 0<\operatorname{Var}(X_i)=\sigma^2<\infty. 
$$

The classical central limit theorem states:

$$
 \boxed{ \frac{\sqrt N(\bar X_N-\mu)}{\sigma} \xrightarrow{d}\mathcal N(0,1) }. 
$$

The notation $\xrightarrow{d}$ denotes convergence in distribution.

For sufficiently large $N$, this suggests:

$$
 \boxed{ \bar X_N \approx \mathcal N\left( \mu,\frac{\sigma^2}{N} \right) } 
$$

as an approximation to the sampling distribution of the mean.

#### 1 Deriving the normalization scale

Let:

$$
 S_N=\sum_iX_i. 
$$

Its mean is:

$$
 \mathbb E[S_N]=N\mu. 
$$

Its variance is:

$$
 \operatorname{Var}(S_N)=N\sigma^2. 
$$

Therefore its standard deviation is:

$$
 \sigma\sqrt N. 
$$

To centre and scale the sum:

$$
 Z_N= \frac{S_N-N\mu}{\sigma\sqrt N}. 
$$

Since:

$$
 S_N=N\bar X_N, 
$$

we obtain:

$$
 Z_N= \frac{N(\bar X_N-\mu)} {\sigma\sqrt N} = \frac{\sqrt N(\bar X_N-\mu)}{\sigma}. 
$$

That explains the normalization appearing in the theorem.

#### 2 Why the result is surprising

The individual $X_i$ need not be Gaussian.

They could be Bernoulli variables, uniform random numbers, exponentially distributed waiting times, or another distribution with finite variance.

Under the IID assumptions, the appropriately standardized sum approaches a Gaussian distribution.

The Gaussian emerges from aggregation.

#### 3 But CLT does not guarantee that every sample average is Gaussian

The approximation depends on the number of samples and the underlying distribution.

Highly skewed or heavy-tailed distributions can require much larger sample counts before a Gaussian approximation becomes satisfactory.

If the variance is infinite, the classical finite-variance CLT does not apply.

If samples are correlated, the IID theorem cannot be used without modification.

For a path tracer, rare high-energy contributions can produce strongly skewed or heavy-tailed estimator distributions, making naive Gaussian confidence intervals unreliable at modest sample counts.

#### 4 The effective sample size of correlated observations

For a stationary sequence with variance $\sigma^2$ and lag-$k$ autocorrelation $\rho_k$, the variance of the sample mean is:

$$
 \boxed{ \operatorname{Var}(\bar X_N) = \frac{\sigma^2}{N} \left[ 1+ 2\sum\_{k=1}^{N-1} \left(1-\frac{k}{N}\right)\rho_k \right] }. 
$$

When the autocorrelations vanish, we recover $\sigma^2/N$.

For persistent positive correlations, the bracketed factor is greater than one.

A heuristic effective sample size is:

$$
 N\_{\mathrm{eff}} \approx \frac{N}{ 1+2\sum\_{k\ge1}\rho_k } 
$$

when the relevant series converges and the approximation is appropriate.

Thus, one million correlated samples may contain substantially less statistical information than one million independent samples.

#### 5 What LLN and CLT imply for rendering

The law of large numbers explains why an appropriate Monte Carlo estimator converges to its expectation.

The central limit theorem explains why its sufficiently well-behaved sample mean may have approximately Gaussian fluctuations around the target, with standard deviation proportional to $1/\sqrt N$.

Neither theorem says that the estimator's expectation equals the physically correct image.

That requires the rendering model, sampling distribution, weighting, and integration measure to be correct.

A biased estimator can converge very confidently to the wrong value.

the sampling and Monte Carlo chapters established how to produce samples and construct estimators. the statistical validation chapters addresses the final—and scientifically decisive—question:

Given a finite collection of observations, what can we legitimately conclude?

Suppose two path tracers render the same scene. Algorithm A produces an image with a lower reported error on one run, while algorithm B is faster.

Does that prove A is more accurate? Does B have better sample efficiency? Were both tested using comparable random streams? Is the result stable across seeds and scenes? Is the numerical difference large relative to its statistical uncertainty?

Statistics allows us to answer those questions without relying on impressions from a single experiment.

![[Visual Recall/Math Foundations/F10-4_mc-error-bias-floor.png]]

**F10-4. Monte Carlo random error and a fixed bias floor.** For the stated integral, unbiased RMSE falls with sample count while the biased estimator retains its offset.

![[Visual Recall/Math Foundations/F10-5_importance-constant-weight.png]]

**F10-5. A matched importance proposal can make contributions constant.** For the stated nonnegative integrand and support, the matched proposal gives constant contributions; uniform-proposal variance is 4/45. This does not generalize to arbitrary signed integrands.


<!-- end V10.09 -->

<!-- begin V10.10 -->

### The empirical distribution is a model built directly from the sample

The central limit theorem tells us how one important statistic behaves asymptotically, but it does not by itself explain a much broader statistical pattern: how can we estimate an unknown property of a population when we do not want to commit to a narrow parametric family?

Suppose the unknown data-generating distribution is \(F\), and we observe

\[
X_1,\ldots,X_n\sim F.
\]

The **empirical distribution function** is

\[
\boxed{
\widehat F_n(x)
=
\frac{1}{n}
\sum_{i=1}^{n}
\mathbf 1\{X_i\le x\}
}.
\]

At any query value \(x\), \(\widehat F_n(x)\) is simply the fraction of observed samples not exceeding \(x\). It is a step function with a jump of \(1/n\) at every observation, counting repeated observations with repeated mass.

This construction is more important than it first appears. It says that the observed sample itself can define a complete discrete probability model: place probability \(1/n\) on each observed datum. That model is not asserted to be the true population. It is a data-supported surrogate that becomes increasingly informative as sampling improves.

For a concrete sample

\[
2,\;2,\;5,\;9,
\]

the empirical model assigns total mass \(1/2\) to value \(2\), mass \(1/4\) to \(5\), and mass \(1/4\) to \(9\). Therefore

\[
\widehat F_4(4)=\frac{2}{4}=\frac12,
\qquad
\widehat F_4(7)=\frac{3}{4}.
\]

The empirical CDF is useful because many population quantities can be regarded as **functionals of a distribution**. The population mean can be written

\[
\mu(F)=\int x\,dF(x).
\]

Replacing \(F\) by \(\widehat F_n\) gives

\[
\mu(\widehat F_n)
=
\int x\,d\widehat F_n(x)
=
\frac1n\sum_iX_i
=
\bar X.
\]

So the ordinary sample mean is a special case of a much larger pattern.

#### The plug-in principle

If the desired population quantity is

\[
\theta=T(F),
\]

then the **plug-in estimator** replaces the unknown \(F\) with its empirical estimate:

\[
\boxed{
\widehat\theta_{\mathrm{plug\text{-}in}}
=
T(\widehat F_n)
}.
\]

This one idea unifies many familiar formulas.

For the population variance,

\[
V(F)=
\int (x-\mu(F))^2\,dF(x),
\]

the direct plug-in estimate is

\[
V(\widehat F_n)
=
\frac1n\sum_i(X_i-\bar X)^2.
\]

Notice that this is the denominator-\(n\) empirical variance, not the denominator-\((n-1)\) unbiased sample-variance estimator discussed later. The distinction is instructive. **Plug-in estimation and unbiased estimation are different design criteria.** A plug-in estimator asks, “what do I obtain by replacing the unknown distribution by the empirical distribution?” An unbiased estimator asks, “does the repeated-sampling expectation equal the target?” Sometimes the same formula satisfies both goals; sometimes it does not.

Quantiles provide another example. If \(q_\alpha(F)\) is the population \(\alpha\)-quantile, then \(q_\alpha(\widehat F_n)\) is an empirical quantile. Medians, trimmed statistics, empirical moments, and many risk summaries can all be viewed this way.

The empirical-distribution viewpoint also makes the bootstrap section later in this volume much less mysterious. A nonparametric bootstrap sample is simply a fresh IID sample drawn from \(\widehat F_n\). The bootstrap therefore asks what the estimator would do if the empirical distribution were the population. It is not random perturbation for its own sake; it is repeated sampling from a concrete estimated distribution.

#### Convergence of a distribution estimate is stronger than convergence of one statistic

For any fixed \(x\), the random variables

\[
\mathbf 1\{X_i\le x\}
\]

are Bernoulli with expectation \(F(x)\). Their sample mean is exactly \(\widehat F_n(x)\). The law of large numbers therefore suggests

\[
\widehat F_n(x)\to F(x)
\]

at continuity points of \(F\).

A stronger result, the Glivenko–Cantelli theorem, says under the IID setup that the maximum CDF discrepancy converges to zero:

\[
\boxed{
\sup_x
\left|
\widehat F_n(x)-F(x)
\right|
\longrightarrow 0
}
\]

almost surely.

The important conceptual jump is from estimating one number to estimating an entire function. Once a whole distribution can be approximated, many downstream functionals become estimable through the same object. But continuity matters: a tiny change in a distribution can sometimes produce a large change in a poorly behaved functional. “The empirical CDF converges” does not mean every imaginable statistic inherits a simple Gaussian approximation.

For graphics, simulation, and experimental engineering this matters whenever a pipeline reports more than a mean. Frame-time percentiles, tail latencies, reconstruction-error quantiles, residual distributions, visibility probabilities, and empirical BRDF or sensor-error summaries are all distributional objects. Reporting only an average can hide the behavior the system actually needs to control.

### The delta method propagates asymptotic uncertainty through a nonlinear function

Suppose an estimator \(\widehat\theta_n\) already has an asymptotic normal approximation:

\[
\sqrt n(\widehat\theta_n-\theta)
\xrightarrow{d}
\mathcal N(0,\sigma^2).
\]

Now suppose the quantity we actually care about is not \(\theta\), but a smooth transformation

\[
\psi=g(\theta).
\]

Examples appear constantly:

- a variance estimate may be converted to a standard deviation by \(g(v)=\sqrt v\);
- an odds estimate may be converted to probability;
- a camera calibration parameter may be transformed into field of view;
- a positive scale may be logged;
- an estimated physical coefficient may be converted into a derived observable.

We could derive a new sampling distribution from scratch for every transformation. The **delta method** instead uses the same local linearization idea that already appears throughout calculus and numerical analysis.

Taylor-expand \(g\) around the true parameter:

\[
g(\widehat\theta_n)
=
g(\theta)
+
g'(\theta)(\widehat\theta_n-\theta)
+
R_n,
\]

where the remainder is smaller than the first-order term under the usual differentiability and convergence conditions. Multiply by \(\sqrt n\):

\[
\sqrt n
\left[
g(\widehat\theta_n)-g(\theta)
\right]
=
g'(\theta)
\sqrt n(\widehat\theta_n-\theta)
+
\sqrt nR_n.
\]

If the remainder vanishes in probability, the limiting distribution becomes

\[
\boxed{
\sqrt n
\left[
g(\widehat\theta_n)-g(\theta)
\right]
\xrightarrow{d}
\mathcal N
\left(
0,\,
[g'(\theta)]^2\sigma^2
\right)
}.
\]

Equivalently, when the large-\(n\) variance of \(\widehat\theta_n\) is approximately \(\sigma^2/n\),

\[
\operatorname{Var}\!\left[g(\widehat\theta_n)\right]
\approx
[g'(\theta)]^2
\frac{\sigma^2}{n}.
\]

This is statistical uncertainty propagation through a Jacobian.

#### Vector form: the same Jacobian logic used elsewhere in this book

Let

\[
\widehat{\boldsymbol\theta}\in\mathbb R^p
\]

satisfy

\[
\sqrt n
(\widehat{\boldsymbol\theta}-\boldsymbol\theta)
\xrightarrow{d}
\mathcal N(\mathbf 0,\Sigma).
\]

For a differentiable scalar function

\[
g:\mathbb R^p\to\mathbb R,
\]

the first-order perturbation is

\[
g(\widehat{\boldsymbol\theta})
-
g(\boldsymbol\theta)
\approx
\nabla g(\boldsymbol\theta)^T
(\widehat{\boldsymbol\theta}-\boldsymbol\theta).
\]

Therefore

\[
\boxed{
\sqrt n
\left[
g(\widehat{\boldsymbol\theta})
-
g(\boldsymbol\theta)
\right]
\xrightarrow{d}
\mathcal N
\left(
0,\,
\nabla g^T
\Sigma
\nabla g
\right)
}.
\]

For a vector-valued transformation

\[
\mathbf g:\mathbb R^p\to\mathbb R^m
\]

with Jacobian \(J_g\),

\[
\boxed{
\operatorname{Cov}
\left[
\mathbf g(\widehat{\boldsymbol\theta})
\right]
\approx
\frac1n
J_g
\Sigma
J_g^T
}.
\]

This is the same mathematical structure used earlier for first-order covariance propagation. The difference is interpretive: here \(\Sigma\) describes an estimator's asymptotic sampling fluctuation; in a state-estimation calculation it may describe uncertainty in a random state vector. The algebra can be identical while the source of uncertainty is different.

#### Worked example: uncertainty of an estimated standard deviation

Suppose an estimator of variance \(v=\sigma^2\) is approximately normal:

\[
\widehat v
\approx
\mathcal N(v,\tau^2).
\]

We want the standard deviation

\[
s=\sqrt v.
\]

Here

\[
g(v)=\sqrt v,
\qquad
g'(v)=\frac{1}{2\sqrt v}.
\]

The delta method gives

\[
\operatorname{Var}(\widehat s)
\approx
\frac{\tau^2}{4v}.
\]

So an approximate standard error is

\[
\boxed{
\operatorname{se}(\widehat s)
\approx
\frac{\operatorname{se}(\widehat v)}
{2\sqrt v}
}.
\]

In practice \(v\) is unknown, so a plug-in step often replaces it by \(\widehat v\). This shows how two ideas connect rather than living in separate chapters:

\[
\text{estimate distribution or parameter}
\rightarrow
\text{plug in unknown quantities}
\rightarrow
\text{linearize the requested transformation}
\rightarrow
\text{propagate uncertainty}.
\]

#### Where the first-order delta method can fail

A derivative is not a ceremonial requirement. It controls the approximation.

If

\[
g'(\theta)=0,
\]

the first-order term disappears and a second-order expansion may determine the limiting behavior. If \(g\) is not differentiable at the true value, the ordinary delta method may be invalid. If the parameter lies on a boundary, the estimator's limiting distribution itself may be non-Gaussian. If the transformation has a very large derivative near the operating point, small parameter uncertainty can become large output uncertainty.

A common numerical example is reciprocal depth:

\[
z=\frac1d.
\]

Its derivative is

\[
\frac{dz}{dd}=-\frac1{d^2}.
\]

When disparity \(d\) approaches zero, first-order uncertainty in depth explodes. That is not merely a bad covariance matrix; it reflects real geometric ill-conditioning. The same Jacobian that propagates uncertainty also exposes sensitivity.

The broader lesson is that **asymptotic normality is not closed under arbitrary nonlinear operations without conditions**. A transformed estimate should inherit its uncertainty through an explicitly justified transformation, not through the assumption that every downstream number is “approximately Gaussian.”

Source basis: Wasserman, *All of Statistics*, especially the chapters on empirical distributions and plug-in estimation, convergence, the delta method, and the bootstrap. The graphics, calibration, frame-time, and reciprocal-depth examples above are integrations into this volume's existing computational narrative.


## 9. Finite samples need stable descriptive statistics

### Sample mean

Suppose we observe $N$ values:

$$
 x_1,x_2,\ldots,x_N. 
$$

Their arithmetic sample mean is:

$$
 \boxed{ \bar x=\frac1N\sum\_{i=1}^{N}x_i }. 
$$

This is a descriptive statistic of the observed data.

When the observations are draws from a distribution, the sample mean can also estimate the population mean:

$$
 \mu=\mathbb E[X]. 
$$

#### 1 Exact example

Let:

$$
 x=(1,2,3,4,5,6). 
$$

Then:

$$
 \bar x= \frac{1+2+3+4+5+6}{6} =\boxed{3.5}. 
$$

This number summarizes the centre of the observed data.

It does not tell us how spread out the observations are or whether the data contain outliers.

#### 2 Why the arithmetic mean is sensitive to outliers

Consider:

$$
 x=(1,2,3,4,100). 
$$

The mean is:

$$
 \bar x=\frac{110}{5}=\boxed{22}. 
$$

Four of the five observations are between one and four, yet the mean is 22.

The mean is not necessarily wrong; it is doing exactly what its definition requires.

But if our goal is to describe the typical observation, the mean can be misleading for heavily skewed or contaminated samples.

#### 3 The mean as a least-squares minimizer

There is an important connection to Phase 9.

Consider choosing one number $m$ to represent all observations, minimizing squared error:

$$
 E(m)= \sum\_{i=1}^{N}(x_i-m)^2. 
$$

Differentiate:

$$
 E'(m)= -2\sum_i(x_i-m). 
$$

Set to zero:

$$
 \sum_i(x_i-m)=0. 
$$

Thus:

$$
 \sum_ix_i-Nm=0, 
$$

so:

$$
 \boxed{ m=\frac1N\sum_ix_i=\bar x }. 
$$

The second derivative is:

$$
 E''(m)=2N>0. 
$$

Therefore the sample mean is the unique minimizer of the total squared deviation.

This explains why squared-error objectives are associated with arithmetic means.

It also explains why the mean is sensitive to outliers: squaring disproportionately penalizes large deviations.

### Sample variance

Sample variance measures the spread of observed values around their sample mean.

One natural descriptive version is:

$$
 \boxed{ v_N= \frac1N \sum\_{i=1}^{N}(x_i-\bar x)^2 }. 
$$

If our goal is to estimate the variance of an underlying population from IID samples, the conventional unbiased estimator is:

$$
 \boxed{ s^2= \frac1{N-1} \sum\_{i=1}^{N}(x_i-\bar x)^2 } 
$$

for $N\ge2$.

These have different denominators and different interpretations.

#### 1 Worked example

Using:

$$
 x=(1,2,3,4,5,6), 
$$

the sample mean is:

$$
 \bar x=3.5. 
$$

The deviations are:

$$
 (-2.5,-1.5,-0.5,0.5,1.5,2.5). 
$$

Square them:

$$
 (6.25,2.25,0.25,0.25,2.25,6.25). 
$$

Sum:

$$
 6.25+2.25+0.25+0.25+2.25+6.25 =\boxed{17.5}. 
$$

The population-normalized empirical variance is:

$$
 v_N=\frac{17.5}{6} =\boxed{2.9166667}. 
$$

The unbiased sample variance estimate is:

$$
 s^2=\frac{17.5}{5} =\boxed{3.5}. 
$$

The distinction is not caused by arithmetic error. The formulas estimate or describe different quantities.

#### 2 Sample standard deviation

The sample standard deviation is:

$$
 \boxed{s=\sqrt{s^2}}. 
$$

For our example:

$$
 s=\sqrt{3.5}\approx1.870829. 
$$

It has the same units as the observations.

But even though $s^2$ is unbiased for population variance under the standard IID assumptions, $s$ is not generally an unbiased estimator of the population standard deviation.

Taking a nonlinear square root changes the expectation.

### Bessel correction

Why do we divide by $N-1$ rather than $N$ when estimating population variance?

The answer is not that one observation has physically disappeared.

The reason is that the sample mean is itself estimated from the observations.

This estimation introduces a systematic downward bias in the average squared deviations calculated around $\bar X$.

Let's prove it.

Suppose:

$$
 X_1,\ldots,X_N 
$$

are IID with population mean $\mu$ and finite population variance $\sigma^2$.

We want to evaluate:

$$
 \mathbb E\left[ \sum_i(X_i-\bar X)^2 \right]. 
$$

#### 1 Decompose deviations around the true mean

Write:

$$
 X_i-\bar X = (X_i-\mu)-(\bar X-\mu). 
$$

Square and sum:

$$
 \sum_i(X_i-\bar X)^2 = \sum_i[ (X_i-\mu)-(\bar X-\mu) ]^2. 
$$

Expand:

$$
 \begin{aligned} \sum_i(X_i-\bar X)^2 &= \sum_i(X_i-\mu)^2\\ &\quad-2(\bar X-\mu)\sum_i(X_i-\mu)\\ &\quad+N(\bar X-\mu)^2. \end{aligned} 
$$

But:

$$
 \sum_i(X_i-\mu)=N(\bar X-\mu). 
$$

Substitute:

$$
 \begin{aligned} \sum_i(X_i-\bar X)^2 &= \sum_i(X_i-\mu)^2\\ &\quad-2N(\bar X-\mu)^2 +N(\bar X-\mu)^2. \end{aligned} 
$$

Therefore:

$$
 \boxed{ \sum_i(X_i-\bar X)^2 = \sum_i(X_i-\mu)^2 - N(\bar X-\mu)^2 }. 
$$

#### 2 Take expectations

For the first term:

$$
 \mathbb E\left[ \sum_i(X_i-\mu)^2 \right] =N\sigma^2. 
$$

For the second term:

$$
 \mathbb E[(\bar X-\mu)^2] = \operatorname{Var}(\bar X) = \frac{\sigma^2}{N} 
$$

under independence.

Thus:

$$
 \begin{aligned} \mathbb E\left[ \sum_i(X_i-\bar X)^2 \right] &= N\sigma^2- N\frac{\sigma^2}{N}\\ &=(N-1)\sigma^2. \end{aligned} 
$$

Therefore:

$$
 \boxed{ \mathbb E\left[ \frac1{N-1} \sum_i(X_i-\bar X)^2 \right] = \sigma^2 }. 
$$

That proves the usual unbiased sample variance formula.

#### 3 Degrees of freedom intuition

The deviations from the sample mean satisfy:

$$
 \sum_i(X_i-\bar X)=0. 
$$

If you know $N-1$ deviations, the final deviation is determined by this constraint.

The centred deviations therefore have only $N-1$ freely varying components.

This provides the geometric intuition behind the denominator.

In more complex models, degrees of freedom relate to the number and rank of estimated parameters, not simply to memorizing a universal subtraction of one.

For instance, regression residual variance estimation can involve $N-p$, where $p$ is the rank or number of appropriately independent fitted parameters under the model.

### Online mean computation

Suppose samples arrive one at a time.

We may not want to store every sample just to calculate the mean.

Let:

$$
 \mu_n= \frac1n\sum\_{i=1}^{n}x_i 
$$

be the mean after $n$ observations.

When a new sample $x\_{n+1}$ arrives:

$$
 \mu\_{n+1} = \frac{ \sum\_{i=1}^{n}x_i+x\_{n+1} }{ n+1 }. 
$$

Since:

$$
 \sum\_{i=1}^{n}x_i=n\mu_n, 
$$

we have:

$$
 \mu\_{n+1} = \frac{n\mu_n+x\_{n+1}}{n+1}. 
$$

Rearrange:

$$
 \mu\_{n+1} = \mu_n+ \frac{x\_{n+1}-\mu_n}{n+1}. 
$$

Thus:

$$
 \boxed{ \mu\_{n+1} = \mu_n+ \frac{x\_{n+1}-\mu_n}{n+1} }. 
$$

This is the online mean update.

#### 1 Worked trace

Suppose samples arrive as:

$$
 4,\ 7,\ 10. 
$$

Start with the first observation:

$$
 \mu_1=4. 
$$

The second sample is seven:

$$
 \mu_2 = 4+\frac{7-4}{2} =5.5. 
$$

The third sample is ten:

$$
 \mu_3 = 5.5+\frac{10-5.5}{3} =7. 
$$

The direct mean is:

$$
 \frac{4+7+10}{3}=7. 
$$

Both calculations agree.

#### 2 C++23 implementation

```cpp
double onlineMean(std::span<const double> samples) {
    if (samples.empty())
        throw std::invalid_argument("No samples");

    double mean = 0.0;
    std::size_t count = 0;

    for (const double x : samples) {
        ++count;

        // Move the current mean toward the new sample.
        // The movement shrinks as the sample count grows.
        mean += (x - mean) /
                static_cast<double>(count);
    }

    return mean;
}
```

The update uses constant storage.

But a numerically stable mean algorithm does not automatically give us a stable variance algorithm.

The naive formula:

$$
 \operatorname{Var}(X)= \mathbb E[X^2]-\mathbb E[X]^2 
$$

can suffer severe cancellation in floating-point arithmetic.

That motivates Welford's algorithm.

### Welford's numerically stable variance algorithm

This is one of the most practically valuable statistical algorithms in the phase.

We want to accumulate variance online while avoiding the numerical problems caused by subtracting two nearly equal large values.

Maintain three quantities:

$$
 n=\text{number of samples}, 
$$

$$
 \mu_n=\text{sample mean}, 
$$

$$
 M\_{2,n}= \sum\_{i=1}^{n}(x_i-\mu_n)^2. 
$$

The sample variance will later be:

$$
 s^2=\frac{M\_{2,n}}{n-1}. 
$$

#### 1 Derive the update

Suppose we have $n$ samples and receive a new value $x$.

Define the difference from the old mean:

$$
 \boxed{\delta=x-\mu_n}. 
$$

The new mean is:

$$
 \boxed{ \mu\_{n+1} = \mu_n+\frac{\delta}{n+1} }. 
$$

Define the difference from the new mean:

$$
 \boxed{ \delta_2=x-\mu\_{n+1} }. 
$$

Welford's scatter update is:

$$
 \boxed{ M\_{2,n+1} = M\_{2,n}+\delta\delta_2 }. 
$$

Let's derive why.

#### 2 Expand the new scatter

By definition:

$$
 M\_{2,n+1} = \sum\_{i=1}^{n}(x_i-\mu\_{n+1})^2 + (x-\mu\_{n+1})^2. 
$$

For old samples:

$$
 x_i-\mu\_{n+1} = (x_i-\mu_n)-(\mu\_{n+1}-\mu_n). 
$$

Define:

$$
 d=\mu\_{n+1}-\mu_n=\frac{\delta}{n+1}. 
$$

Then:

$$
 \sum\_{i=1}^{n}(x_i-\mu\_{n+1})^2 = \sum_i[(x_i-\mu_n)-d]^2. 
$$

Expand:

$$
 = \sum_i(x_i-\mu_n)^2 - 2d\sum_i(x_i-\mu_n) + nd^2. 
$$

The centred deviations sum to zero:

$$
 \sum_i(x_i-\mu_n)=0. 
$$

Thus:

$$
 \sum\_{i=1}^{n}(x_i-\mu\_{n+1})^2 = M\_{2,n}+nd^2. 
$$

The new sample's deviation from the new mean is:

$$
 x-\mu\_{n+1} = x-\mu_n-\frac{\delta}{n+1}. 
$$

Because $\delta=x-\mu_n$:

$$
 x-\mu\_{n+1} = \delta\left(1-\frac1{n+1}\right) = \frac{n}{n+1}\delta. 
$$

Therefore:

$$
 \begin{aligned} M\_{2,n+1} &= M\_{2,n} +n\frac{\delta^2}{(n+1)^2} +\frac{n^2\delta^2}{(n+1)^2}\\ &= M\_{2,n} +\frac{n(n+1)\delta^2}{(n+1)^2}\\ &= M\_{2,n}+\frac{n}{n+1}\delta^2. \end{aligned} 
$$

But:

$$
 \delta_2=\frac{n}{n+1}\delta. 
$$

Therefore:

$$
 \boxed{ M\_{2,n+1} = M\_{2,n}+\delta\delta_2 }. 
$$

This is the Welford recurrence.

#### 3 The implementation

```cpp
struct Welford {
    std::size_t count = 0;

    double mean = 0.0;

    // Sum of squared deviations from the current mean.
    double m2 = 0.0;

    void add(double x) {
        if (!std::isfinite(x))
            throw std::domain_error("Nonfinite sample");

        ++count;

        const double delta = x - mean;

        mean += delta / static_cast<double>(count);

        const double delta2 = x - mean;

        // Corrected centred sum-of-squares update.
        m2 += delta * delta2;
    }

    double sampleVariance() const {
        if (count < 2)
            throw std::domain_error("Need at least two samples");

        // Unbiased sample variance under IID assumptions.
        return m2 / static_cast<double>(count - 1);
    }

    double populationVariance() const {
        if (count == 0)
            throw std::domain_error("No samples");

        return m2 / static_cast<double>(count);
    }
};
```

This stores only count, mean, and accumulated centred scatter.

It can handle arbitrarily long streams in $O(1)$ storage, assuming the count type does not overflow.

#### 4 Why naive variance is numerically dangerous

Suppose the observations are:

$$
 10^9+1,\quad10^9+2,\quad10^9+3,\quad10^9+4. 
$$

The true mean is:

$$
 10^9+2.5. 
$$

Their deviations are:

$$
 -1.5,-0.5,0.5,1.5. 
$$

The squared deviations sum to:

$$
 2.25+0.25+0.25+2.25=5. 
$$

Thus the unbiased sample variance is:

$$
 \boxed{s^2=\frac53\approx1.6666667}. 
$$

Now consider computing variance using:

$$
 \frac1N\sum_ix_i^2-\bar x^2. 
$$

Both terms are of order $10^{18}$.

The desired variance is of order one.

Subtracting such large, nearly equal floating-point numbers can erase the meaningful small difference.

Welford instead accumulates centred differences directly, substantially improving numerical behavior in many cases.

It is not immune to every possible floating-point problem, but it avoids an especially severe cancellation mechanism.

Welford's 1962 paper formalized stable corrected sums of squares and products, providing an important historical foundation for streaming numerical statistics. 

doi.org

#### 5 Merging statistics from parallel workers

Suppose worker A processes $n_A$ samples and worker B processes $n_B$.

Their summaries are:

$$
 (n_A,\mu_A,M\_{2,A}) 
$$

and:

$$
 (n_B,\mu_B,M\_{2,B}). 
$$

We want to combine them without replaying individual samples.

Define:

$$
 n=n_A+n_B, 
$$

$$
 \delta=\mu_B-\mu_A. 
$$

The merged mean is:

$$
 \boxed{ \mu= \mu_A+\delta\frac{n_B}{n} }. 
$$

The merged scatter is:

$$
 \boxed{ M_2= M\_{2,A}+M\_{2,B} + \delta^2\frac{n_An_B}{n} }. 
$$

The additional term accounts for the difference between the workers' mean values.

This makes streaming statistics especially useful in parallel rendering, where each tile or thread group can accumulate local moments and the results can be merged afterward.

The merge operation is associative in exact arithmetic under its model, but floating-point results can depend slightly on merge order.

For deterministic parallel reductions, define a stable merge tree.

### Histograms

A histogram groups observations into intervals called bins.

Suppose we collect pixel noise magnitudes:

$$
 x_1,\ldots,x_N. 
$$

Choose bin edges:

$$
 b_0\<b_1<\cdots\<b_K. 
$$

The count in bin $j$ is:

$$
 \boxed{ c_j= \#\{i:b_j\le x_i < b_{j+1}\} }. 
$$

The half-open convention prevents observations on shared boundaries from being counted twice.

#### 1 Counts, frequencies, and density

A histogram can display several different quantities.

Count:

$$
 c_j. 
$$

Relative frequency:

$$
 \frac{c_j}{N}. 
$$

Density estimate for a bin of width $h_j$:

$$
 \boxed{ \hat p_j= \frac{c_j}{Nh_j} }. 
$$

These are not interchangeable.

For a density histogram, the probability represented by the bin is approximated by:

$$
 \hat p_jh_j=\frac{c_j}{N}. 
$$

The area of the histogram bar, not simply its height, corresponds to probability.

This matters when bins have unequal widths.

#### 2 Bin width affects interpretation

Too few bins hide structure.

Too many bins create noisy estimates with sparse counts.

There is no universally optimal histogram resolution.

Some practical rules scale bin width with sample size, such as the Freedman–Diaconis rule:

$$
 \boxed{ h=2\operatorname{IQR}N^{-1/3} } 
$$

where IQR is the interquartile range.

This can be useful for continuous data with sufficient variation, but it does not automatically solve the problem for multimodal, bounded, highly discrete, or extremely heavy-tailed data.

#### 3 Histograms in renderer debugging

Useful histograms include luminance contributions, path lengths, PDF values, importance weights, residual magnitudes, depth errors, and image-space reconstruction errors.

However, an error histogram alone does not show whether errors are spatially clustered.

A renderer might have a reasonable global error distribution but visible checkerboard artifacts caused by sampler correlations.

Spatial error maps and histograms answer different questions.

### Quantiles

A quantile describes a threshold below which a specified fraction of a distribution lies.

For a population distribution with CDF $F$, the generalized $p$-quantile is:

$$
 \boxed{ Q(p)=\inf\{x:F(x)\ge p\} }. 
$$

Typical examples include:

$$
 Q(0.25) 
$$

for the first quartile,

$$
 Q(0.5) 
$$

for a median, and:

$$
 Q(0.95) 
$$

for the 95th percentile.

#### 1 Empirical quantiles

For a finite sample, sort the observations:

$$
 x\_{(1)}\le\cdots\le x\_{(N)}. 
$$

Different software libraries use different interpolation conventions for sample quantiles.

One common convention, often called type 7, uses:

$$
 h=(N-1)p. 
$$

Let:

$$
 i=\lfloor h\rfloor, \qquad t=h-i. 
$$

Using zero-based sorted indices:

$$
 \boxed{ \hat Q(p)=(1-t)x\_{(i)}+t x\_{(i+1)} } 
$$

with appropriate endpoint handling.

#### 2 Numerical example: exponential quantile

Let:

$$
 x=(1,2,3,4,100). 
$$

The observations are already sorted.

For:

$$
 p=0.25, 
$$

we have:

$$
 h=(5-1)(0.25)=1. 
$$

Thus:

$$
 \hat Q(0.25)=2. 
$$

For:

$$
 p=0.75, 
$$

we obtain:

$$
 h=3. 
$$

Therefore:

$$
 \hat Q(0.75)=4. 
$$

The interquartile range is:

$$
 \boxed{ \operatorname{IQR}=Q(0.75)-Q(0.25)=2 }. 
$$

The extreme outlier at 100 does not affect these particular quartile values under this convention.

#### 3 Quantiles as performance metrics

A system's mean frame time can hide rare severe slowdowns.

For example, one engine might have a mean frame time of 12 ms but occasional 80 ms stalls.

Reporting the 95th or 99th percentile frame time reveals tail behavior.

The same principle applies to reconstruction errors, allocation latencies, numerical solver iteration counts, and rendered image error distributions.

Quantiles are especially useful when a small number of extreme outcomes matter operationally.

But a quantile alone does not explain the frequency or severity of values beyond it. Tail-focused analysis may require additional metrics.

### Median

The median is the 50th percentile.

For an odd sample count, it is the middle sorted observation.

For an even sample count, a conventional sample median is the average of the two middle observations.

For:

$$
 (1,2,3,4,100), 
$$

the median is:

$$
 \boxed{3}. 
$$

The mean is 22.

The difference illustrates the median's resistance to extreme values.

#### 1 Median as an absolute-error minimizer

The sample mean minimizes squared deviations.

The sample median minimizes absolute deviations:

$$
 \boxed{ m^\star\in \arg\min_m\sum_i|x_i-m| }. 
$$

Why?

For a value $m$ not equal to any observation, the derivative of $|x_i-m|$ with respect to $m$ is:

$$
 -1 
$$

when $x_i>m$, and:

$$
 +1 
$$

when $x_i\<m$.

Thus the total derivative depends on how many observations lie to either side.

A minimizer occurs when at least half the observations lie at or below $m$, and at least half lie at or above it.

For an odd sample count, the middle order statistic satisfies that condition.

For an even sample count, every point between the two middle observations minimizes total absolute deviation.

The midpoint of that interval is the conventional reported median, but it is not necessarily the unique absolute-error minimizer.

This is a useful connection between statistics and optimization.

### Robust statistics

Robust statistics concerns procedures that remain informative when data contain outliers, heavy tails, deviations from modeling assumptions, or contamination.

The mean and variance can be strongly affected by large observations.

The median and interquartile range are more resistant to some forms of contamination.

#### 1 Median absolute deviation

Define the sample median:

$$
 m=\operatorname{median}(x_i). 
$$

Calculate absolute deviations from the median:

$$
 d_i=|x_i-m|. 
$$

The median absolute deviation, or MAD, is:

$$
 \boxed{ \operatorname{MAD} = \operatorname{median}(|x_i-m|) }. 
$$

For:

$$
 x=(1,2,3,4,100), 
$$

the median is three.

The absolute deviations are:

$$
 (2,1,0,1,97). 
$$

Sort:

$$
 (0,1,1,2,97). 
$$

Therefore:

$$
 \boxed{\operatorname{MAD}=1}. 
$$

The outlier at 100 has little influence on this result.

#### 2 Gaussian-consistent MAD scale

For an approximately Gaussian distribution, MAD can be scaled to estimate standard deviation:

$$
 \boxed{ \hat\sigma\_{\mathrm{MAD}} \approx1.4826\\,\operatorname{MAD} }. 
$$

The factor comes from the standard normal distribution's median absolute deviation:

$$
 \Phi^{-1}(0.75)\approx0.67449. 
$$

Its reciprocal is approximately:

$$
 \frac1{0.67449}\approx1.4826. 
$$

This scaling is meaningful under a Gaussian reference model and should not be blindly interpreted as a universally unbiased standard-deviation estimate for every distribution.

#### 3 Trimmed means and winsorization

A trimmed mean removes a specified fraction of the smallest and largest observations before averaging.

For:

$$
 (1,2,3,4,100), 
$$

remove the smallest and largest values to obtain:

$$
 (2,3,4). 
$$

The trimmed mean is:

$$
 \boxed{3}. 
$$

Winsorization instead replaces extreme observations with less extreme boundary values rather than discarding them.

These are different robustification strategies with different statistical properties.

#### 4 Robustness has costs

A robust estimator may reduce sensitivity to contamination but lose statistical efficiency under an ideal clean Gaussian model.

Robust statistics is not about always replacing means with medians.

It is about choosing estimators whose assumptions and failure behavior match the data and application.

In reconstruction, for example, we may need to distinguish valid large geometric errors from incorrect feature correspondences.

Suppressing every large residual can hide a genuine modeling failure.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#9. Finite samples need stable descriptive statistics|online variance]] — Online statistics update a mean and centered sum of….



<!-- end V10.10 -->

<!-- begin V10.11 -->
### Choose the next measurement using the geometry of what is still unknown

Suppose we are comparing rendering configurations whose expected quality can be approximated by a small linear model. An **action** is a configuration we can choose. Its feature vector $a$ records the quantities used by the model. An unknown parameter vector $\theta$ maps those features to expected reward $a^{\mathsf T}\theta$. A **reward** is the scalar observation we want to improve, such as a chosen quality score under a fixed time budget. This is a modeling example, not a claim that arbitrary rendering quality is exactly linear.

At step $s$, observe $y_s=a_s^{\mathsf T}\theta+\eta_s$, where $\eta_s$ is noise. Only the chosen action's reward is observed; this partial-feedback decision setting is called a **bandit problem**. The observations can inform other actions when they share features. A separate mean for every action would discard that relationship.

Choose an estimate by regularized least squares:

$$
\widehat\theta=\underset{\vartheta}{\operatorname{argmin}}
\left[\sum_s(y_s-a_s^{\mathsf T}\vartheta)^2
+\lambda\|\vartheta\|_2^2\right],\qquad\lambda>0.
$$

The parameter $\lambda$ penalizes large coefficients. Expanding the quadratic and setting its derivative to zero gives

$$
V\widehat\theta=b,\qquad
V=\lambda I+\sum_s a_sa_s^{\mathsf T},\qquad
b=\sum_s a_sy_s.
$$

For every nonzero $h$, $h^{\mathsf T}Vh=\lambda\|h\|^2+\sum_s(a_s^{\mathsf T}h)^2>0$. Thus the system has a unique solution even before observations span every direction. The regularization also biases the estimate; algebraic uniqueness does not prove statistical accuracy. Solve the positive-definite system rather than repeatedly forming $V^{-1}$.

To represent uncertainty, suppose we have established a set

$$
\mathcal C=\{\vartheta:
(\vartheta-\widehat\theta)^{\mathsf T}V
(\vartheta-\widehat\theta)\le r^2\}.
$$

This is an ellipsoid. Along an eigenvector of $V$ with eigenvalue $\nu$, its semiaxis length is $r/\sqrt\nu$, not $r/\nu$. That square root follows by substituting a displacement $te$ into the quadratic inequality: $t^2\nu\le r^2$.

For an action $a$, the largest plausible predicted reward is the estimate plus the largest scalar measurement over this ellipsoid. Our dual-norm result therefore gives

$$
\max_{\vartheta\in\mathcal C}a^{\mathsf T}\vartheta
=a^{\mathsf T}\widehat\theta
+r\sqrt{a^{\mathsf T}V^{-1}a}.
$$

The second term is an uncertainty bonus, not a mysterious extra preference. When $a\ne0$, a maximizing displacement is

$$
\delta=\frac{rV^{-1}a}{\sqrt{a^{\mathsf T}V^{-1}a}}.
$$

Substitution verifies that it lies on the ellipsoid and attains the claimed bonus. If $a=0$, the predicted reward and bonus are zero without evaluating that denominator.

Take $V=\operatorname{diag}(10,2)$, $\widehat\theta=(0.5,0.3)$, $r=1$ and actions $a_x=(1,0)$ and $a_y=(0,1)$. Greedy prediction favors $a_x$, with scores $0.5$ and $0.3$. Optimistic scores are approximately $0.816228$ and $1.007107$, so the second action is chosen because its direction remains more uncertain. The numerical choice of $r=1$ illustrates the geometry; it is not a calibrated coverage guarantee.

A genuine **confidence set** needs a probability statement about containing the true parameter. Its radius must be justified using assumptions about noise, parameter bounds, regularization and the adaptively chosen observations. For fixed nonadaptive Gaussian designs, covariance can be derived directly; adaptive designs require the appropriate conditional-noise and concentration analysis. Do not reuse a fixed-design interval merely because the normal equations look the same. “Optimism” names the action-selection principle; it does not establish that an uncalibrated ellipse is statistically valid.

This connects vectors, quadratic forms, least squares, dual norms and experimental choice in one calculation. The development uses *Bandit Algorithms*, physical pages 249–251 and 254. It does not claim to reproduce the book's full regret analysis or to supply a production policy for unconstrained renderer tuning.

## 10. Inference and error metrics need explicit assumptions

### Confidence intervals

Suppose our goal is to estimate a population mean:

$$
 \mu. 
$$

We observe a sample mean:

$$
 \bar X. 
$$

Because the sample is random, $\bar X$ varies between repeated experiments.

A confidence interval is a random interval constructed from the sample by a procedure with a specified coverage property.

A 95% confidence interval procedure is designed so that, under its assumptions, approximately 95% of the intervals constructed across repeated samples contain the fixed true parameter.

It does not mean the fixed parameter has a 95% probability of lying in the particular observed interval under the usual frequentist interpretation.

This distinction is fundamental. 

#### 1 Known population standard deviation

If the sampling distribution of the mean is normal, or a suitable normal approximation is justified, and the population standard deviation $\sigma$ is known:

$$
 \frac{\bar X-\mu}{\sigma/\sqrt N} 
$$

has a standard-normal distribution exactly under normal sampling, or approximately under a suitable CLT regime.

For a two-sided 95% interval, the standard-normal critical value is approximately:

$$
 z\_{0.975}=1.959964. 
$$

Thus:

$$
 \boxed{ \mu\in \bar x\pm1.96\frac{\sigma}{\sqrt N} } 
$$

is the familiar 95% normal confidence interval.

#### 2 Unknown population standard deviation

Usually $\sigma$ is not known.

We estimate it from the sample using $s$.

For IID normal observations, the statistic:

$$
 \boxed{ T= \frac{\bar X-\mu}{s/\sqrt N} } 
$$

has a Student's t-distribution with:

$$
 N-1 
$$

degrees of freedom.

The corresponding interval is:

$$
 \boxed{ \bar x \pm t\_{0.975,N-1}\frac{s}{\sqrt N} }. 
$$

For large $N$, the t critical value approaches the normal critical value.

For small $N$, t critical values are larger, reflecting uncertainty from estimating the standard deviation.

The NIST engineering-statistics handbook provides the same distinction between known-$\sigma$ normal intervals and estimated-$\sigma$ t intervals. 

itl.nist.gov+1

#### 3 Numerical example

Return to:

$$
 x=(1,2,3,4,5,6). 
$$

We computed:

$$
 \bar x=3.5, 
$$

$$
 s^2=3.5, 
$$

so:

$$
 s\approx1.870829. 
$$

The estimated standard error is:

$$
 \operatorname{SE}= \frac{s}{\sqrt6} = \sqrt{\frac{3.5}{6}} \approx0.763763. 
$$

A normal-approximation interval uses:

$$
 3.5\pm1.96(0.763763). 
$$

This is approximately:

$$
 \boxed{[2.003,\ 4.997]}. 
$$

But with only six observations, and assuming IID normal sampling, the appropriate exact t critical value for five degrees of freedom is about:

$$
 t\_{0.975,5}\approx2.571. 
$$

Therefore the t interval is approximately:

$$
 3.5\pm2.571(0.763763), 
$$

giving:

$$
 \boxed{[1.536,\ 5.464]}. 
$$

The t interval is wider.

Crucially, if the observations are not plausibly sampled from a normal population, the small-sample t guarantee does not simply carry over unchanged.

#### 4 Confidence intervals for Monte Carlo rendering

Suppose a pixel is estimated from $N$ independent contribution samples:

$$
 Y_1,\ldots,Y_N. 
$$

The mean is:

$$
 \bar Y. 
$$

The sample variance is:

$$
 s_Y^2= \frac1{N-1}\sum_i(Y_i-\bar Y)^2. 
$$

The estimated standard error is:

$$
 \boxed{ \widehat{\operatorname{SE}}= \frac{s_Y}{\sqrt N} }. 
$$

A normal-approximation 95% confidence interval is:

$$
 \boxed{ \bar Y\pm1.96\frac{s_Y}{\sqrt N} } 
$$

when the assumptions and sample size make that approximation appropriate.

But rendering contributions may be highly skewed or heavy-tailed.

If rare fireflies dominate the variance, a small sample may drastically underestimate the true uncertainty simply because no extreme contribution has been observed yet.

#### 5 Adaptive stopping and repeated peeking

Suppose an algorithm repeatedly checks the confidence interval after every new sample and stops as soon as the interval looks sufficiently narrow.

A conventional fixed-sample confidence interval does not automatically retain its nominal coverage under arbitrary optional stopping.

The stopping rule depends on the observed data.

For rigorous adaptive stopping, use a procedure designed for sequential inference, such as appropriate confidence sequences, sequential tests, or carefully controlled stopping rules.

This is important in adaptive path tracing, where sample counts depend on observed noise estimates.

### Hypothesis testing

Hypothesis testing evaluates how compatible observed data are with a specified statistical hypothesis.

Suppose we compare two renderers.

We want to know whether their expected errors differ.

Define:

$$
 H_0:\mu_A=\mu_B 
$$

as the null hypothesis.

An alternative might be:

$$
 H_1:\mu_A\ne\mu_B. 
$$

A test statistic summarizes evidence against the null.

#### 1 Type I and Type II errors

A Type I error occurs when we reject a true null hypothesis.

A Type II error occurs when we fail to reject a false null hypothesis.

The significance level:

$$
 \alpha 
$$

typically controls the probability of a Type I error under the test's assumptions and specified null model.

The power of a test is:

$$
 \boxed{ 1-\beta } 
$$

where $\beta$ is its Type II error probability under a specified alternative.

Power depends on sample size, effect magnitude, variability, and the test procedure.

#### 2 What a p-value means

A p-value measures how extreme the observed test statistic, or something more extreme under the test's definition, would be if the null hypothesis were true.

It is not the probability that the null hypothesis is true.

It is not the probability that the result happened purely by chance.

It does not measure effect size.

A very small p-value can occur for a practically negligible effect with enough observations.

A large p-value can occur when the study lacks enough power to detect a meaningful difference.

#### 3 Paired experiments

For rendering algorithm comparisons, a paired design can be especially useful.

Suppose both algorithms are evaluated on the same scenes and corresponding seeds.

Define a difference for each trial:

$$
 D_i=E\_{A,i}-E\_{B,i}. 
$$

Then analyze:

$$
 \bar D=\frac1N\sum_iD_i. 
$$

The variance of one paired difference is:

$$
 \operatorname{Var}(D) = \operatorname{Var}(E_A)+ \operatorname{Var}(E_B)- 2\operatorname{Cov}(E_A,E_B). 
$$

If the paired errors are positively correlated because scene difficulty affects both algorithms similarly, their difference may have substantially lower variance than two unpaired measurements.

This makes paired comparisons statistically efficient.

However, the pairing must reflect comparable experimental units. Merely giving two different algorithms the same integer seed does not guarantee their generated samples are equivalent or that the pairing has the intended correlation structure.

#### 4 A paired t-test

Assuming the paired differences satisfy the relevant normality assumptions, or an appropriate large-sample approximation is valid, test:

$$
 H_0:\mathbb E[D]=0. 
$$

Compute the sample mean difference:

$$
 \bar D 
$$

and sample standard deviation:

$$
 s_D. 
$$

The test statistic is:

$$
 \boxed{ t= \frac{\bar D}{s_D/\sqrt N} }. 
$$

Under an exact IID normal-difference null model, this follows a t-distribution with $N-1$ degrees of freedom.

The test's p-value is calculated from the appropriate tail probability of that reference distribution.

#### 5 Multiple comparisons

If a study tests 100 different configurations at significance level 0.05 without correction, false positives can become common.

Under independent true-null tests, the expected number of false rejections would be:

$$
 100(0.05)=5. 
$$

Not every study has independent tests, but the example illustrates the issue.

Procedures such as Bonferroni or Holm corrections control familywise error under their respective assumptions.

False-discovery-rate procedures serve a different objective.

A research study should specify the primary hypotheses and comparison plan rather than selecting whichever of many exploratory comparisons happened to give a favorable p-value.

#### 6 Statistical significance versus practical significance

Suppose a new renderer reduces mean error by 0.01% with an extremely small p-value.

That may be statistically detectable but not operationally meaningful.

Alternatively, a 20% reduction in error accompanied by wide uncertainty may be promising but not yet firmly established.

Report effect sizes, uncertainty intervals, workload characteristics, and computational cost alongside hypothesis-test results.

For an engineering research claim, those quantities are often more informative than a p-value alone.

<!-- Bible-final-pass: research inference -->
#### A research claim needs more than a p-value

The paired t-test above is useful, but a paper-quality result needs four separate questions:

1. How large is the effect?
2. How uncertain is that estimate?
3. How surprising would the data be under a stated null model?
4. Was the experiment powerful enough to detect an effect that would actually matter?

For paired measurements, keep the per-unit difference in the original unit:

~~~text
D_i = E_A,i - E_B,i
~~~

The mean difference $\overline{D}$ is the effect in the original engineering unit. Do not throw that away. A standardized paired effect size is often:

~~~text
d_z = mean(D) / standard_deviation(D)
~~~

The standardized value helps compare scales, but it does not replace the original unit. "0.8 standard deviations" and "0.18 ms" answer different questions.

When a normal model for paired differences is doubtful, a paired sign-flip/randomization test can be useful if its design assumption is justified. Under the sharp paired null, keep each observed magnitude but flip its sign. Recompute the statistic for all sign choices, or for a large random sample of them, and measure how often the null statistic is at least as extreme as the observed one.

This is not assumption-free. Its validity comes from the sign-exchangeability or randomization argument. If the experimental design does not justify that exchangeability, the resulting p-value does not have the claimed interpretation. The code comments keep this assumption attached to the exact enumeration rather than presenting it as a generic test.

A paired bootstrap answers a different question. Resample the pairs, or equivalently their paired differences, together with replacement. Recompute the statistic on each bootstrap sample. If the design was paired, do not independently resample algorithm A and algorithm B; that destroys the covariance that made pairing useful.

Power asks whether the experiment could detect a meaningful effect. Formally:

~~~text
power = P(reject H0 | a specified alternative is true)
~~~

Power depends on the effect regarded as important, the variability, the test, the significance level, the design, and the sample size. A study that has only 20 percent power to detect the improvement it cares about cannot conclude "there is no difference" merely because p is above 0.05.

For complex graphics/XR experiments, simulation-based power analysis is often clearer than forcing a closed-form formula. Specify a realistic effect and noise model, generate many complete synthetic studies, rerun the entire analysis pipeline, and count how often the decision rule detects the effect.

Multiple comparisons add another contract. Holm's step-down procedure controls the familywise error rate without using the single blunt Bonferroni threshold for every hypothesis. False-discovery-rate procedures answer a different question. The correction strategy should be chosen before inspecting which comparison looks favorable whenever the analysis is confirmatory.

A strong result table therefore reports more than a p-value: effect in the original unit, uncertainty interval, experimental unit and sample count, pairing/blocking structure, exact test or interval construction, multiple-comparison policy, and the compute/time/memory cost needed to obtain the effect.

The executable witness below preserves pairing, computes d_z, performs an exact sign-flip test for a small sample, forms a paired bootstrap percentile interval, and applies Holm adjustment. Code comments retain the exchangeability assumption, the exponential enumeration cap, and the fact that a percentile interval is not guaranteed to have nominal coverage for every small or skewed sample.


~~~python
#!/usr/bin/env python3
"""Small deterministic inference witness for paired graphics/engine experiments.

It intentionally uses only the Python standard library.  The goal is to make
the research contracts executable: preserve pairing, report an effect size,
obtain a randomization p-value without a normality assumption, build a paired
bootstrap interval, and correct a family of p-values with Holm's procedure.
"""

from __future__ import annotations
import itertools
import math
import random
import statistics


def paired_differences(a: list[float], b: list[float]) -> list[float]:
    # Position i must name the same scene/run/block in both conditions; unequal
    # or empty arrays do not define the requested paired comparison.
    if len(a) != len(b) or not a:
        raise ValueError("paired samples must be non-empty and have equal length")
    if any(not math.isfinite(x) for x in a + b):
        raise ValueError("paired observations must be finite")
    differences = [x - y for x, y in zip(a, b)]
    if any(not math.isfinite(x) for x in differences):
        raise ValueError("paired differences overflowed the finite range")
    return differences


def finite_differences(d: list[float]) -> list[float]:
    """Validate and normalize differences without changing their signs or ratios."""
    if not d or any(not math.isfinite(x) for x in d):
        raise ValueError("paired differences must be non-empty and finite")
    scale = max(abs(x) for x in d)
    return [x / scale for x in d] if scale else [0.0] * len(d)


def cohen_dz(d: list[float]) -> float:
    # Standardize the mean paired change by its sample spread. Constant effects
    # need an explicit result because division by zero is otherwise undefined.
    if len(d) < 2:
        raise ValueError("need at least two paired differences")
    normalized = finite_differences(d)
    sd = statistics.stdev(normalized)
    if sd == 0.0:
        # Reporting convention for a degenerate denominator: no changes report
        # zero, while a constant nonzero change flags signed infinity. This is
        # not a finite effect-size estimate; preserve the improvement's sign.
        mean = statistics.fmean(normalized)
        return math.copysign(math.inf, mean) if mean != 0.0 else 0.0
    return statistics.fmean(normalized) / sd


def exact_sign_flip_pvalue(d: list[float]) -> float:
    """Two-sided exact randomization test under sign-exchangeability.

    Under the sharp paired null, each difference may have either sign with
    equal probability.  This is exact only when that sign-flip null is a
    defensible design/model assumption.
    """
    normalized = finite_differences(d)
    n = len(normalized)
    # Exact enumeration costs 2**n; fail clearly above this teaching cap.
    if n > 20:
        raise ValueError("exact enumeration is intentionally capped at n<=20")
    observed = abs(math.fsum(normalized))
    # Compare dimensionless signed sums. A few local ULPs cover only addition
    # roundoff; unlike a fixed epsilon, this tolerance does not depend on the
    # measurement unit or the magnitude of the original observations.
    extreme = 0
    total = 0
    for signs in itertools.product((-1.0, 1.0), repeat=n):
        candidate = abs(math.fsum(s * x for s, x in zip(signs, normalized)))
        tolerance = 4.0 * max(math.ulp(observed), math.ulp(candidate))
        extreme += candidate >= observed or observed - candidate <= tolerance
        total += 1
    return extreme / total


def paired_bootstrap_mean_ci(
    d: list[float], *, confidence: float = 0.95, draws: int = 20_000, seed: int = 7
) -> tuple[float, float]:
    if not (0.0 < confidence < 1.0):
        raise ValueError("confidence must lie in (0,1)")
    if draws < 100:
        raise ValueError("use enough bootstrap draws to estimate quantiles")
    normalized = finite_differences(d)
    rng = random.Random(seed)
    n = len(normalized)
    scale = max(abs(x) for x in d)
    # Resampling differences is equivalent to resampling intact pairs and
    # retains the covariance that independent marginal resampling would lose.
    means = []
    for _ in range(draws):
        # One bootstrap draw contains n sampled indices, with replacement.
        means.append(statistics.fmean(normalized[rng.randrange(n)] for _ in range(n)))
    means.sort()
    # Percentile order statistics are an interval construction, not a universal
    # small-sample coverage guarantee.
    alpha = 1.0 - confidence
    lo = means[int((alpha / 2.0) * (draws - 1))]
    hi = means[int((1.0 - alpha / 2.0) * (draws - 1))]
    return lo * scale, hi * scale


def holm_adjust(pvalues: list[float]) -> list[float]:
    """Holm step-down adjusted p-values preserving original order."""
    # Invalid inputs should not turn into plausible-looking adjusted values.
    if any(not math.isfinite(p) or not 0.0 <= p <= 1.0 for p in pvalues):
        raise ValueError("p-values must be finite and lie in [0,1]")
    m = len(pvalues)
    order = sorted(range(m), key=pvalues.__getitem__)
    adjusted = [0.0] * m
    running = 0.0
    for rank, idx in enumerate(order):
        # The running maximum enforces monotone step-down adjusted p-values.
        candidate = (m - rank) * pvalues[idx]
        running = max(running, candidate)
        adjusted[idx] = min(1.0, running)
    return adjusted


def expect_value_error(operation, *args) -> None:
    """Make negative-contract checks survive python -O."""
    try:
        operation(*args)
    except ValueError:
        return
    raise RuntimeError(f"{operation.__name__} accepted invalid input")


def main() -> None:
    # Same six scenes, same measurement units, same pairing.
    baseline = [10.4, 8.8, 12.1, 9.7, 11.5, 10.0]
    proposed = [9.8, 8.3, 11.2, 9.5, 10.7, 9.2]
    d = paired_differences(baseline, proposed)

    mean_gain = statistics.mean(d)
    effect = cohen_dz(d)
    p = exact_sign_flip_pvalue(d)
    ci = paired_bootstrap_mean_ci(d)

    require(mean_gain > 0.0, "paired mean should favor proposed method")
    require(effect > 1.0, "synthetic paired effect should be large")
    require(0.0 <= p <= 1.0, "randomization p-value lies in [0,1]")
    require(ci[0] > 0.0, "synthetic paired interval should exclude zero")

    raw = [0.004, 0.018, 0.041, 0.20]
    adj = holm_adjust(raw)
    require(all(a >= p0 for a, p0 in zip(adj, raw)), "adjustment cannot lower p-values")
    require(adj[0] <= adj[1] <= adj[2] <= adj[3], "sorted Holm results are monotone")

    # Scaling an engineering measurement changes its unit, not its exact
    # sign-flip inference. Normalization plus relative ULP ties preserves p.
    for unit_scale in (1e-20, 1.0, 1e20):
        scaled = [value * unit_scale for value in d]
        require(exact_sign_flip_pvalue(scaled) == p,
                "sign-flip p-value changed when only the measurement unit changed")

    # Negative contracts demonstrate that invalid pair shapes and p-values
    # fail loudly rather than yielding a result that looks statistically valid.
    invalid_calls = (
        (paired_differences, ([1.0], [])),
        (paired_differences, ([math.nan], [0.0])),
        (paired_differences, ([math.inf], [0.0])),
        (paired_differences, ([1e308], [-1e308])),
        (cohen_dz, ([1.0, math.nan],)),
        (cohen_dz, ([1.0, math.inf],)),
        (exact_sign_flip_pvalue, ([],)),
        (exact_sign_flip_pvalue, ([math.nan],)),
        (exact_sign_flip_pvalue, ([math.inf],)),
        (paired_bootstrap_mean_ci, ([math.nan],)),
        (paired_bootstrap_mean_ci, ([math.inf],)),
        (holm_adjust, ([1.1],)),
    )
    for operation, args in invalid_calls:
        expect_value_error(operation, *args)

    # Constant differences have no estimated spread. Check both directions so
    # an all-negative effect cannot accidentally be reported as improvement.
    require(cohen_dz([2.0, 2.0]) == math.inf, "positive degenerate effect sign")
    require(cohen_dz([-2.0, -2.0]) == -math.inf, "negative degenerate effect sign")
    require(cohen_dz([0.0, 0.0]) == 0.0, "zero-change reporting convention")

    print(f"paired mean improvement: {mean_gain:.6f}")
    print(f"Cohen dz: {effect:.6f}")
    print(f"exact sign-flip p-value: {p:.6f}")
    print(f"95% paired bootstrap percentile CI: [{ci[0]:.6f}, {ci[1]:.6f}]")
    print("Holm adjusted:", ", ".join(f"{x:.6f}" for x in adj))
    print("research inference: paired, finite, scale, degenerate, interval and Holm checks PASS")


def require(condition: bool, message: str) -> None:
    # Explicit checks survive python -O, which removes assert statements.
    if not condition:
        raise RuntimeError(message)


if __name__ == "__main__":
    main()
~~~

### Likelihood turns a forward probability model into an inverse estimation rule

Probability starts with a model and asks what observations it can generate. Statistical inference reverses that direction: after seeing observations, we ask which model parameters remain plausible. Wasserman makes this probability/inference inversion the organizing idea of *All of Statistics*. The distinction matters in visual computing because a renderer, camera-noise model, tracking model, or reconstruction system often supplies the **forward law**, while calibration and inverse graphics solve the corresponding **inverse statistical problem**.

Let independent observations \(X_1,\ldots,X_n\) have density or mass function \(f(x;\theta)\). Once the data \(x_1,\ldots,x_n\) are fixed, the same product

\[
L_n(\theta)=\prod_{i=1}^{n} f(x_i;\theta)
\]

is viewed as a function of the unknown parameter. It is the **likelihood**, not a probability density over \(\theta\); in general it need not integrate to one with respect to the parameter. The maximum-likelihood estimator is

\[
\hat\theta_{\mathrm{MLE}}\in\arg\max_{\theta\in\Theta}L_n(\theta).
\]

Products of many small probabilities underflow, so implementations normally maximize the log-likelihood

\[
\ell_n(\theta)=\sum_{i=1}^{n}\log f(x_i;\theta).
\]

The logarithm is monotone, so it preserves the maximizer while converting products into sums. Constants independent of \(\theta\) may be dropped when finding the argmax, but not when the actual normalized likelihood value matters.

The familiar least-squares objective is therefore not a mysterious universal choice. If

\[
Y_i=g_\theta(X_i)+\varepsilon_i,\qquad
\varepsilon_i\sim\mathcal N(0,\sigma^2)
\]

independently with fixed \(\sigma\), then

\[
-\ell_n(\theta)
=
\frac{1}{2\sigma^2}\sum_i
\left(Y_i-g_\theta(X_i)\right)^2
+\text{constant}.
\]

Minimizing squared residuals is exactly maximum likelihood under this Gaussian conditional-noise model. Change the noise law and the statistically motivated loss changes. A Laplace error model leads to an absolute-deviation term; a Bernoulli observation model leads to a log-loss/cross-entropy term. This is why an inverse-rendering objective should say which residual distribution it assumes rather than merely calling a loss “standard.”

Maximum likelihood also supplies an information geometry. For a one-parameter regular model the score is

\[
S_\theta(X)=\frac{\partial}{\partial\theta}\log f(X;\theta),
\]

and the Fisher information can be written, under the usual regularity assumptions, as

\[
I(\theta)=
\mathbb E_\theta[S_\theta(X)^2].
\]

Large information means the likelihood changes strongly when the parameter moves; weak information means nearby parameters produce hard-to-distinguish observations. For regular fixed-dimensional models, the MLE often has asymptotic variance approximately \(1/(nI(\theta))\) in the scalar case and an inverse Fisher-information matrix in the multivariate case. This is an asymptotic statement, not a guarantee for small data, singular models, boundary parameters, changing model dimension, or poorly identifiable inverse problems. A reconstruction can have a perfectly implemented optimizer and still possess little information about one parameter.

A useful engine/research diagnostic is therefore to keep three objects distinct:

\[
\boxed{
\text{forward model}
\;\longrightarrow\;
\text{likelihood}
\;\longrightarrow\;
\text{estimator}
}
\]

The forward model states what data would look like under a parameter. The likelihood scores parameters using the observed data. The estimator is a rule that converts the likelihood and any other assumptions into a reported parameter. Conflating them makes it difficult to tell whether failure came from the physical model, the statistical assumption, or the numerical optimizer.

Source basis: Wasserman, *All of Statistics*, Chapters 6 and 9, especially the model/inference distinction, likelihood and log-likelihood definitions, maximum likelihood, large-sample properties, Fisher information, and assumption checking.

### Bootstrap asks what would happen if we could repeat the data-collection process

An estimate is only one number. Scientific use usually needs to know how much that number would vary under repeated sampling. Sometimes an exact sampling distribution is available; often it is not. The **bootstrap** approximates that repeated-sampling experiment using the observed empirical distribution.

Given observations \(X_1,\ldots,X_n\), the empirical distribution places mass \(1/n\) on each observed value. A nonparametric bootstrap replicate draws

\[
X_1^*,\ldots,X_n^*
\]

independently **with replacement** from those observed values and recomputes the statistic,

\[
\hat\theta^*=T(X_1^*,\ldots,X_n^*).
\]

Repeating this \(B\) times produces \(\hat\theta_1^*,\ldots,\hat\theta_B^*\). Their sample spread estimates the sampling spread of \(\hat\theta\). For example,

\[
\widehat{\operatorname{se}}_{\mathrm{boot}}
=
\sqrt{
\frac{1}{B-1}
\sum_{b=1}^{B}
(\hat\theta_b^*-\bar{\theta}^*)^2
}.
\]

The key conceptual step is that the bootstrap does **not** resample arbitrary numbers around the estimator. It substitutes the empirical distribution for the unknown population and then repeats the same estimator on samples drawn from that surrogate population. A **parametric bootstrap** instead fits a parametric model \(f(x;\hat\theta)\), simulates data from that fitted model, and reruns the estimator. The two procedures make different assumptions.

Bootstrap confidence intervals can be built in several ways. A percentile interval uses empirical quantiles of the bootstrap replicates. A pivotal construction instead approximates the distribution of \(\hat\theta-\theta\) using \(\hat\theta^*-\hat\theta\). More sophisticated corrections exist because bootstrap distributions can be biased or skewed. The important engineering lesson is not to memorize one interval recipe but to identify which sampling distribution the resampling scheme is trying to approximate.

Bootstrap validity is not automatic. Statistics defined by extreme order behavior, nonsmooth boundaries, dependence, or non-IID data can defeat the naive IID bootstrap. Wasserman gives the uniform-endpoint MLE as a concrete warning: the ordinary nonparametric bootstrap can put anomalously large probability on reproducing the observed sample maximum because it can never generate values beyond that maximum. Time series, spatially correlated pixels, rays sharing random paths, and temporally correlated benchmark frames also violate the simple IID resampling story unless blocks or another dependence-aware scheme are used.

The bootstrap therefore complements the uncertainty material already developed here. A standard error formula is a theorem under assumptions. A bootstrap is a computational approximation under assumptions. Neither becomes trustworthy merely because it produces a smooth histogram.

Source basis: Wasserman, *All of Statistics*, Chapters 7–9, especially the empirical distribution, bootstrap variance estimation, bootstrap confidence intervals, the jackknife appendix, parametric bootstrap, and examples where nonparametric bootstrap behavior is poor.

### Decision theory separates an estimator from the consequences of being wrong

Two estimators can have different error patterns without one being universally “better.” Statistical decision theory makes the comparison explicit by specifying a **loss function**

\[
L(\theta,a),
\]

where \(\theta\) is the unknown state/parameter and \(a\) is the action or estimate reported by the procedure. The estimator \(\delta(X)\) is then a decision rule. Its frequentist **risk** at parameter value \(\theta\) is the expected loss over data generated from that \(\theta\):

\[
R(\theta,\delta)
=
\mathbb E_\theta[
L(\theta,\delta(X))
].
\]

Under squared-error loss this becomes mean squared error,

\[
R(\theta,\delta)
=
\operatorname{Var}_\theta[\delta(X)]
+
\operatorname{Bias}_\theta[\delta(X)]^2,
\]

which connects decision theory directly to the bias–variance decomposition already used in rendering and learning experiments.

Different losses encode genuinely different tasks. Squared error punishes large mistakes strongly. Absolute error changes the optimal point summary from a mean-like answer toward a median-like one. Zero-one loss asks whether a discrete decision is correct. Kullback–Leibler loss compares distributions. A perception system choosing a pose estimate, a renderer choosing a denoised pixel, and a safety test deciding “collision/no collision” can therefore prefer different decision rules even when they observe the same data.

A **Bayes rule** minimizes risk averaged under a prior distribution on \(\theta\). Under squared error, the posterior mean is the Bayes estimator. A **minimax rule** minimizes the maximum risk over \(\theta\):

\[
\delta_{\mathrm{minimax}}
\in
\arg\min_\delta
\sup_{\theta}R(\theta,\delta).
\]

These are different design criteria. Bayes risk asks for good average performance under a specified prior; minimax risk asks for protection against the worst parameter in the chosen class. An **admissible** rule is one that is not uniformly dominated by another rule. Admissibility is a weak sanity condition, not proof that a rule is practically good: a bizarre rule can be admissible because no single alternative beats it everywhere.

This framework also prevents over-reading maximum likelihood. In regular fixed-dimensional parametric models, MLEs often enjoy attractive local large-sample risk properties, but Wasserman explicitly notes that this story can fail when the dimension grows with the data. “MLE” is an estimation construction, not a theorem of universal optimality.

For visual-computing evaluation, write the loss before choosing the estimator. If a depth error of \(10\) cm at two metres matters differently from the same absolute error at fifty metres, ordinary squared depth error may not represent the application. If catastrophic outliers matter more than typical error, a mean metric can hide the relevant risk. Decision theory turns this from a metrics argument into a precise statement about which errors the system is being optimized to avoid.

Source basis: Wasserman, *All of Statistics*, Chapter 12 on loss, risk, Bayes rules, minimax rules, admissibility, and the large-sample relationship between maximum likelihood, Bayes, and minimax procedures.

### Generalization is a prediction problem, not a training-fit problem

A model fitted and scored on the same observations is rewarded for remembering the noise in those observations. Wasserman’s regression discussion makes the distinction through **prediction risk** versus **training error**. Adding parameters can always make the training fit no worse, while future prediction can deteriorate because variance increases. This is the operational meaning of overfitting.

Suppose a model class indexed by \(S\) produces a fitted predictor \(\hat r_S\). A future-data risk has the form

\[
R(S)
=
\mathbb E[
L(Y^*,\hat r_S(X^*))
],
\]

where the expectation includes a fresh observation. The training loss instead evaluates the same samples used to fit \(\hat r_S\). It is generally optimistically biased as an estimate of future risk because the fitted values depend on those same responses.

**Cross-validation** breaks that reuse deliberately. In \(K\)-fold cross-validation, partition the sample into \(K\) blocks. For each fold, fit on the other \(K-1\) blocks and score on the held-out block. Aggregate the held-out losses. The procedure estimates performance of the full fitting pipeline, including preprocessing and hyperparameter selection that occurs **inside** each training split. If normalization, feature selection, or parameter tuning sees the held-out data before scoring, the apparent validation estimate leaks information.

The bias–variance tradeoff is therefore not merely a slogan about “simple versus complex.” A larger model can reduce approximation bias while increasing sensitivity to the particular training sample. Regularization, feature selection, early stopping, and model averaging all change that trade. Their value should be judged on new data or through a defensible resampling procedure, not by training fit.

For classification, empirical error alone also says little about what the hypothesis class could memorize. The **VC dimension** asks for the largest finite set that a class can shatter—realize every binary labeling on. Finite VC dimension supports uniform generalization bounds under IID assumptions. The lesson is not that every graphics model needs a VC-dimension calculation. It is that generalization depends jointly on sample size and the richness of the rule family being selected from. A benchmark repeatedly consulted during development can effectively become training data even when no gradient ever touches it.

Source basis: Wasserman, Chapters 13 and 22, especially regression model selection, prediction risk versus training error, cross-validation, the bias–variance tradeoff, classification risk, shattering, and VC dimension.

### A graphical model can encode conditional independence without proving causation

A directed acyclic graph can make a multivariate probability law easier to reason about. If each variable \(X_i\) has parent set \(\pi_i\), a distribution Markov to a DAG factors as

\[
p(x_1,\ldots,x_k)
=
\prod_{i=1}^{k}
p(x_i\mid \pi_i).
\]

This factorization says which conditional dependencies the model retains. It can reduce a high-dimensional joint distribution into local conditional models and exposes conditional independences through the graph.

The graph’s arrows are not automatically causal arrows. **d-separation** gives a graphical criterion for conditional independence in a DAG under the relevant Markov/faithfulness assumptions. Chains and forks are blocked when conditioning on the intermediate node; colliders behave oppositely—conditioning on a collider, or on an appropriate descendant, can open a path that was otherwise blocked. This is why “control for every measured variable” is not a safe causal recipe: conditioning on the wrong variable can create association.

Causal questions require another operation: intervention. Wasserman distinguishes passive conditioning

\[
P(Y\mid X=x)
\]

from an intervention written in his notation as

\[
P(Y\mid X:=x).
\]

To represent setting \(X=x\), arrows entering \(X\) are removed and the corresponding conditional factor for \(X\) is replaced by the intervention. Observation asks what tends to be true among cases where \(X=x\); intervention asks what would happen under a rule that forces \(X\) to \(x\). Those distributions can differ.

A confounder \(Z\) that causes both treatment \(X\) and outcome \(Y\) can make association misleading. Randomization breaks the assignment dependence and can make the causal effect identifiable without modeling the original treatment-selection mechanism. In observational data, adjustment requires the relevant confounders to be measured and the causal assumptions to be defensible. An unobserved confounder cannot be repaired by adding more optimizer iterations.

This distinction is directly relevant to graphics/vision experiments. If a new reconstruction method is evaluated mostly on easier scenes because it fails to initialize on harder ones, conditioning on “successful run” can create selection effects. If exposure settings affect both image noise and which frames enter a dataset, the observed noise/accuracy correlation is not automatically causal. A DAG can make those assumptions visible, but the graph itself must be justified by experimental design or domain knowledge.

Source basis: Wasserman, Chapter 17 on DAG factorization, the Markov condition, d-separation, intervention versus observation, confounding, and the warning that causal graph structure is not generally recoverable from observational data without assumptions.

### Mean squared error

The term mean squared error, or MSE, has two closely related but distinct uses.

In statistical estimation, it measures expected squared error of a random estimator:

$$
 \boxed{ \operatorname{MSE}(\widehat\theta) = \mathbb E[(\widehat\theta-\theta)^2] }. 
$$

As derived in the sampling and Monte Carlo chapters:

$$
 \boxed{ \operatorname{MSE} = \operatorname{Var}(\widehat\theta) + \operatorname{Bias}(\widehat\theta)^2 }. 
$$

In model evaluation, we often calculate the empirical average squared prediction error:

$$
 \boxed{ \operatorname{MSE}\_{\mathrm{empirical}} = \frac1N\sum\_{i=1}^{N} (\hat y_i-y_i)^2 }. 
$$

The first is a theoretical performance quantity of a random estimator.

The second is a statistic calculated from a particular evaluation dataset.

They are related, but not automatically identical.

#### 1 Numerical example: mean squared error

Suppose the true observations are:

$$
 y=(1,2,4). 
$$

The predicted values are:

$$
 \hat y=(2,1,5). 
$$

The errors are:

$$
 e=(1,-1,1). 
$$

Square:

$$
 e^2=(1,1,1). 
$$

Therefore:

$$
 \boxed{ \operatorname{MSE}= \frac{1+1+1}{3}=1 }. 
$$

#### 2 MSE sensitivity to outliers

Suppose a single prediction error is ten times larger than another.

Its squared contribution is 100 times larger.

Thus MSE strongly penalizes large errors.

This may be desirable when large errors are especially costly.

But it can be misleading when outliers arise from data corruption or mismatched correspondences that should be handled separately.

### Root mean squared error

Root mean squared error is:

$$
 \boxed{ \operatorname{RMSE} = \sqrt{ \frac1N\sum_i(\hat y_i-y_i)^2 } }. 
$$

RMSE has the same units as the original measurements.

If position errors are measured in metres, RMSE is measured in metres.

MSE would instead have units of square metres.

#### 1 Example

For the previous errors:

$$
 e=(1,-1,1), 
$$

we have:

$$
 \operatorname{MSE}=1. 
$$

Thus:

$$
 \boxed{\operatorname{RMSE}=1}. 
$$

Now consider errors:

$$
 e=(1,1,3). 
$$

Then:

$$
 \operatorname{MSE} = \frac{1+1+9}{3} =\frac{11}{3}. 
$$

Therefore:

$$
 \boxed{ \operatorname{RMSE} = \sqrt{\frac{11}{3}} \approx1.914854 }. 
$$

#### 2 RMSE and Euclidean vector norms

For a residual vector:

$$
 \mathbf r= \begin{bmatrix} r_1\\vdots\\r_N \end{bmatrix}, 
$$

the Euclidean norm is:

$$
 \\|\mathbf r\\|\_2= \sqrt{\sum_i r_i^2}. 
$$

Therefore:

$$
 \boxed{ \operatorname{RMSE} = \frac{\\|\mathbf r\\|\_2}{\sqrt N} }. 
$$

This directly connects RMSE to least-squares optimization and vector norms.

#### 3 Aggregation matters

Suppose you report RMSE across many scenes.

Computing the arithmetic mean of per-scene RMSE values is generally not the same as computing RMSE over all individual measurements.

The first gives scenes equal weight.

The second gives measurements equal weight, unless explicitly reweighted.

Both may be valid depending on the evaluation objective, but they answer different questions.

A benchmark must specify the aggregation procedure.

### Mean absolute error

Mean absolute error, or MAE, is:

$$
 \boxed{ \operatorname{MAE} = \frac1N\sum\_{i=1}^{N} |\hat y_i-y_i| }. 
$$

Instead of squaring errors, it takes their absolute magnitudes.

For:

$$
 e=(1,-1,1), 
$$

we obtain:

$$
 \boxed{\operatorname{MAE}=1}. 
$$

For:

$$
 e=(1,1,3), 
$$

the result is:

$$
 \boxed{ \operatorname{MAE}=\frac53\approx1.666667 }. 
$$

Compare RMSE:

$$
 1.914854. 
$$

RMSE is larger because it gives more influence to the error of magnitude three.

#### 1 MAE and the L1 norm

For residual vector $\mathbf r$:

$$
 \\|\mathbf r\\|\_1=\sum_i|r_i|. 
$$

Therefore:

$$
 \boxed{ \operatorname{MAE}= \frac1N\\|\mathbf r\\|\_1 }. 
$$

MSE and RMSE are connected to the L2 norm.

MAE is connected to the L1 norm.

That connection explains their different sensitivity to outliers and their different optimization properties.

#### 2 Mean versus median under loss functions

As derived earlier:

- Squared-error minimization produces the arithmetic mean as the optimal constant prediction.
- Absolute-error minimization produces a median as an optimal constant prediction.

The loss function therefore determines what kind of central tendency the optimization algorithm learns.

In probabilistic terms, squared error is also related to Gaussian likelihood modeling, while absolute error corresponds naturally to a Laplace-style error model under appropriate assumptions.

This links Phase 9 optimization objectives to Phase 10 statistical modeling.

### Relative error metrics

Absolute errors have units.

A one-metre error may be enormous for a small object and negligible for a planetary-scale quantity.

Relative error attempts to express the error compared with a meaningful reference scale.

For an approximation $\hat x$ to a nonzero true value $x$:

$$
 \boxed{ e\_{\mathrm{rel}} = \frac{|\hat x-x|}{|x|} }. 
$$

The corresponding percentage error is:

$$
 100e\_{\mathrm{rel}}\\%. 
$$

#### 1 Numerical example: relative error metrics

Suppose the true length is:

$$
 x=200\ \mathrm{mm}. 
$$

The estimate is:

$$
 \hat x=202\ \mathrm{mm}. 
$$

The absolute error is:

$$
 2\ \mathrm{mm}. 
$$

The relative error is:

$$
 \frac2{200}=0.01. 
$$

Thus the percentage error is:

$$
 \boxed{1\%}. 
$$

#### 2 Why relative error fails near zero

Suppose:

$$
 x=10^{-8} 
$$

and:

$$
 \hat x=10^{-6}. 
$$

The absolute error is approximately:

$$
 9.9\times10^{-7}. 
$$

But the relative error is:

$$
 \frac{9.9\times10^{-7}}{10^{-8}} =99. 
$$

That is 9,900%.

The large relative error is mathematically correct, but may or may not be useful depending on the application's physical scales.

If the true value is zero, the ordinary relative-error formula is undefined.

Therefore relative error should not be used blindly for values near zero.

#### 3 Mean absolute percentage error

For observations $y_i\ne0$:

$$
 \boxed{ \operatorname{MAPE} = \frac1N \sum_i \left| \frac{\hat y_i-y_i}{y_i} \right| }. 
$$

Multiply by 100 to express the result as a percentage.

MAPE is easy to interpret for positive-scale quantities, but it is problematic when true observations are zero or near zero.

It can also behave asymmetrically and unintuitively for some forecasting problems.

For values that can be zero or negative, other metrics may be preferable.

#### 4 Symmetric MAPE

One commonly used version of symmetric MAPE is:

$$
 \boxed{ \operatorname{sMAPE} = \frac1N\sum_i \frac{2|\hat y_i-y_i|} {|\hat y_i|+|y_i|} }. 
$$

The result is often multiplied by 100.

Different references use different conventions and scaling factors.

Even this formula has a zero-over-zero ambiguity when both prediction and truth are zero, so implementations must define a policy.

Despite its name, sMAPE does not eliminate every undesirable property of percentage errors.

#### 5 Normalized RMSE

A benchmark may report:

$$
 \operatorname{NRMSE} = \frac{\operatorname{RMSE}}{s\_{\mathrm{reference}}} 
$$

where $s\_{\mathrm{reference}}$ is a specified scale.

Possible choices include the reference mean, range, standard deviation, or another characteristic magnitude.

These lead to different numerical values.

Therefore an NRMSE result is incomplete unless the normalization convention is reported.

#### 6 Geometric error metrics

For 3D reconstruction, we might measure:

$$
 e_i=\\|\hat{\mathbf p}\_i-\mathbf p_i\\|\_2 
$$

in metres.

Then report mean Euclidean distance, RMSE of Euclidean distances, median point error, upper percentiles, or a threshold-based accuracy fraction.

For image reprojection:

$$
 e_i= \left\\| \begin{bmatrix}\hat u_i\\hat v_i\end{bmatrix} - \begin{bmatrix}u_i\\v_i\end{bmatrix} \right\\|\_2. 
$$

Those errors are measured in pixels.

The camera's image resolution, focal length, depth distribution, and evaluation protocol affect their interpretation.

For normals, angular error may be more meaningful than Euclidean componentwise error.

For rotations, a geodesic angular distance on $SO(3)$ is generally more interpretable than direct componentwise differences of Euler angles or quaternions.

There is no universally meaningful error metric independent of the object's mathematical structure and the research question.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#10. Inference and error metrics need explicit assumptions|coverage interval]] — A confidence interval is a procedure with repeated-sample coverage….

**Concept link:** [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics#10. Inference and error metrics need explicit assumptions|quantile]] — Histograms, quantiles, medians, and robust scales summarize different features….




<!-- end V10.11 -->

<!-- begin V10.12 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/31 - Implement Probability with Numerical Contracts|Next chapter]]
