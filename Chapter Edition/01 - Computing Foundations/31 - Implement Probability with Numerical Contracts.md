# 31 — Implement Probability with Numerical Contracts

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|Next chapter]]

Probability and covariance implementations must preserve the mathematical differences among probability mass, density, samples, and estimates. This chapter builds stable representations, tail calculations, tests, and deterministic checks around randomized code.

**By the end:** Implement probability and covariance routines while preventing overflow, preserving tail accuracy, and validating random algorithms deterministically.

**Recall question:** How can a randomized routine be tested deterministically without claiming that one seed proves its statistical behavior?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 75692–78257; source-span SHA256 9af1918d5ef4d23b1620773377e15ed44d16de26e2d97e9f196dba9b1cd7bb90. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## 11. Implement probability without losing its numerical contracts

### C++23 implementation: probability and covariance from first principles

The mathematical foundations are now in place. We can turn them into code while preserving the distinctions between probabilities, densities, samples, covariance, and numerical approximations.

#### 1 Representation: mathematical objects should have distinct contracts

At a basic level, a Bernoulli distribution needs only one probability parameter. A binomial distribution needs the number of trials and success probability. A Gaussian needs a mean and standard deviation, or a mean vector and covariance matrix.

But these types have different domains.

A Bernoulli probability must satisfy:

$$
 0\le p\le1. 
$$

A normal standard deviation must satisfy:

$$
 \sigma>0. 
$$

An exponential rate must satisfy:

$$
 \lambda>0. 
$$

A nonsingular multivariate Gaussian covariance must be symmetric positive definite.

The constructors should enforce these invariants.

For example:

```cpp
struct Normal {
    double mu;
    double sigma;

    Normal(double mean, double sd)
        : mu(mean), sigma(sd) {

        // A valid nonsingular Gaussian requires sigma > 0.
        // A NaN or infinite parameter is not accepted.
        if (!std::isfinite(mu) ||
            !std::isfinite(sigma) ||
            !(sigma > 0.0)) {

            throw std::domain_error(
                "Invalid normal distribution parameters"
            );
        }
    }

    [[nodiscard]]
    double pdf(double x) const {
        const double z = (x - mu) / sigma;

        return std::exp(-0.5 * z * z) /
               (sigma * std::sqrt(2.0 * std::numbers::pi));
    }

    [[nodiscard]]
    double cdf(double x) const {
        const double z =
            (x - mu) /
            (sigma * std::numbers::sqrt2);

        // erfc avoids cancellation from 1 + erf(z)
        // when z is far into the negative tail.
        return 0.5 * std::erfc(-z);
    }

    [[nodiscard]]
    double survival(double x) const {
        const double z =
            (x - mu) /
            (sigma * std::numbers::sqrt2);

        // Direct upper-tail calculation avoids 1 - CDF(x)
        // when CDF(x) is extremely close to one.
        return 0.5 * std::erfc(z);
    }
};
```

This is a conceptual excerpt of the complete implementation. The downloaded file uses explicitly named constants and supplies additional methods and tests.

#### 2 Numerical tail accuracy

Why should a normal distribution expose both `cdf()` and `survival()`?

Mathematically:

$$
 S(x)=1-F(x). 
$$

But in floating-point arithmetic, if $F(x)$ is extremely close to one, the subtraction can lose many significant digits.

For example, the upper tail of a standard normal at a large positive $z$ is tiny. An expression computing `1.0 - cdf(z)` may round to zero long before the actual probability underflows.

Using:

$$
 S(z)=\frac12\operatorname{erfc}(z/\sqrt2) 
$$

allows the implementation to calculate that tail more directly.

The same principle applies to exponential CDF calculations and log probabilities.

Mathematical equivalence does not guarantee numerical equivalence.

### Computing discrete probabilities without overflowing factorials

The binomial PMF contains:

$$
 \binom nk= \frac{n!}{k!(n-k)!}. 
$$

A naive factorial-based implementation becomes problematic for large $n$.

The laboratory instead uses logarithms and the gamma function:

```cpp
[[nodiscard]]
double binomial_pmf(
    unsigned n,
    unsigned k,
    double p
) {
    if (!std::isfinite(p) ||
        p < 0.0 || p > 1.0) {
        throw std::domain_error("Invalid probability");
    }

    if (k > n)
        return 0.0;

    // Avoid log(0) and correctly handle deterministic cases.
    if (p == 0.0)
        return k == 0 ? 1.0 : 0.0;

    if (p == 1.0)
        return k == n ? 1.0 : 0.0;

    // ln(n choose k)
    const double logChoose =
        std::lgamma(static_cast<double>(n) + 1.0)
      - std::lgamma(static_cast<double>(k) + 1.0)
      - std::lgamma(static_cast<double>(n - k) + 1.0);

    // Compute log probability first.
    // log1p(-p) is useful when p is small.
    const double logProbability =
        logChoose +
        static_cast<double>(k) * std::log(p) +
        static_cast<double>(n - k) * std::log1p(-p);

    return std::exp(logProbability);
}
```

The mathematical expression is unchanged.

But we avoid constructing huge factorials before dividing them.

The function explicitly handles $p=0$ and $p=1$, where the distribution becomes deterministic.

For extremely small probabilities, even `std::exp(logProbability)` may underflow to zero. If the caller needs to compare or combine extreme-tail probabilities, an API returning log probability may be preferable.

#### 1 Poisson PMF

A corresponding Poisson implementation is:

```cpp
[[nodiscard]]
double poisson_pmf(unsigned k, double lambda) {
    if (!std::isfinite(lambda) || lambda < 0.0)
        throw std::domain_error("Invalid Poisson mean");

    if (lambda == 0.0)
        return k == 0 ? 1.0 : 0.0;

    const double logProbability =
        static_cast<double>(k) * std::log(lambda)
        - lambda
        - std::lgamma(static_cast<double>(k) + 1.0);

    return std::exp(logProbability);
}
```

It uses:

$$
 \log P(K=k) = k\log\lambda-\lambda-\log(k!). 
$$

Again, we do not calculate a large factorial explicitly.

This is a recurring numerical principle: when probabilities are represented as products of many small factors, log-space arithmetic can significantly improve reliability.

For distributions with extremely small probabilities, log probabilities also make multiplication into addition:

$$
 \log(p_1p_2)=\log p_1+\log p_2. 
$$

The reverse transformation, summing probabilities represented in log space, requires care and motivates the log-sum-exp technique covered later in Phase 10.

### Implementing a multivariate Gaussian without explicitly inverting covariance

The mathematical PDF contains:

$$
 \Sigma^{-1} 
$$

and:

$$
 \det\Sigma. 
$$

A naive implementation might construct the inverse, calculate its determinant, and evaluate the quadratic form directly.

But matrix inversion is often unnecessary and can be numerically undesirable.

For a symmetric positive-definite covariance, Cholesky factorization gives:

$$
 \boxed{\Sigma=LL^T}. 
$$

Define:

$$
 \mathbf d=\mathbf x-\boldsymbol\mu. 
$$

The squared Mahalanobis distance is:

$$
 d_M^2=\mathbf d^T\Sigma^{-1}\mathbf d. 
$$

Since:

$$
 \Sigma^{-1}=L^{-T}L^{-1}, 
$$

we have:

$$
 d_M^2= \mathbf d^TL^{-T}L^{-1}\mathbf d. 
$$

Let:

$$
 \mathbf y=L^{-1}\mathbf d. 
$$

Then:

$$
 \boxed{d_M^2=\mathbf y^T\mathbf y}. 
$$

We obtain $\mathbf y$ by solving the lower-triangular system:

$$
 L\mathbf y=\mathbf d. 
$$

No explicit inverse is required.

Likewise:

$$
 \det\Sigma=(\det L)^2. 
$$

Because $L$ is triangular:

$$
 \det L=\prod_iL\_{ii}. 
$$

Therefore:

$$
 \boxed{ \log\det\Sigma = 2\sum_i\log L\_{ii} }. 
$$

These identities lead to the stable log-density expression:

$$
 \boxed{ \log p(\mathbf x)= -\frac n2\log(2\pi) -\frac12\log\det\Sigma -\frac12\mathbf y^T\mathbf y }. 
$$

#### 1 The 2D implementation

The complete source includes this Cholesky-based structure:

```cpp
using Vec2 = std::array<double, 2>;

struct Mat2 {
    double a00, a01;
    double a10, a11;
};

struct Normal2D {
    Vec2 mu;
    Mat2 cov;

    // Lower-triangular Cholesky entries:
    //
    // L = [l00   0 ]
    //     [l10  l11]
    //
    // Sigma = L * transpose(L)
    double l00;
    double l10;
    double l11;

    double logDet;

    Normal2D(Vec2 mean, Mat2 sigma)
        : mu(mean), cov(sigma) {

        // The complete source first checks symmetry
        // and positive semidefiniteness.
        if (!is_psd(cov))
            throw std::domain_error("Invalid covariance");

        // The ordinary 2D density requires STRICT
        // positive definiteness, not merely PSD.
        if (!(cov.a00 > 0.0))
            throw std::domain_error("Singular covariance");

        l00 = std::sqrt(cov.a00);
        l10 = cov.a10 / l00;

        // Schur complement for the second pivot.
        const double schur =
            cov.a11 - l10 * l10;

        if (!(schur > 0.0))
            throw std::domain_error("Singular covariance");

        l11 = std::sqrt(schur);

        logDet =
            2.0 * (std::log(l00) + std::log(l11));
    }

    [[nodiscard]]
    double log_pdf(Vec2 x) const {
        // Forward substitution:
        // Solve L*y = (x-mu).
        const double y0 =
            (x[0] - mu[0]) / l00;

        const double y1 =
            ((x[1] - mu[1]) - l10 * y0) / l11;

        // This is the squared Mahalanobis distance.
        const double mahalanobisSquared =
            y0 * y0 + y1 * y1;

        return
            -std::log(2.0 * std::numbers::pi)
            -0.5 * logDet
            -0.5 * mahalanobisSquared;
    }
};
```

The complete tested file includes PDF evaluation, sampling, stricter validation, and numerical assertions.

#### 2 Operation cost and storage

For a dense $n\times n$ positive-definite covariance, Cholesky factorization requires $O(n^3)$ arithmetic.

Once the factor is available, evaluating the Mahalanobis distance through triangular solves costs $O(n^2)$ for a dense matrix.

For a fixed covariance used to evaluate many points, we should factorize once and reuse the factor.

Repeatedly recomputing the factorization for each observation would waste work.

For small $2\times2$ or $3\times3$ covariances, specialized kernels can be very efficient, but they still need numerical safeguards near degeneracy.

For large sparse covariances or precision matrices, matrix structure can significantly change the best factorization and evaluation strategy.

### Online covariance accumulation

Suppose a simulator or renderer produces millions of sampled two-dimensional observations.

We want their empirical covariance.

Storing every sample solely to compute mean and covariance may consume unnecessary memory.

Instead, we can maintain streaming statistics.

For sample $n$, maintain the current mean:

$$
 \boldsymbol\mu_n 
$$

and centred scatter matrix:

$$
 S_n= \sum\_{i=1}^{n} (\mathbf x_i-\boldsymbol\mu_n) (\mathbf x_i-\boldsymbol\mu_n)^T. 
$$

When a new sample $\mathbf x$ arrives, define:

$$
 \boldsymbol\delta=\mathbf x-\boldsymbol\mu_n. 
$$

Update the mean:

$$
 \boxed{ \boldsymbol\mu\_{n+1} = \boldsymbol\mu_n+ \frac{\boldsymbol\delta}{n+1} }. 
$$

The scatter update is:

$$
 \boxed{ S\_{n+1} = S_n+ \boldsymbol\delta (\mathbf x-\boldsymbol\mu\_{n+1})^T }. 
$$

The sample covariance estimate is:

$$
 \boxed{ \widehat\Sigma= \frac{S_n}{n-1} } 
$$

for $n\ge2$.

The corresponding population-normalized empirical covariance divides by $n$.

#### 1 A complete 2D numerical example

Take three observations:

$$
 \mathbf x_1= \begin{bmatrix}1\\1\end{bmatrix}, 
$$

$$
 \mathbf x_2= \begin{bmatrix}2\\3\end{bmatrix}, 
$$

$$
 \mathbf x_3= \begin{bmatrix}3\\5\end{bmatrix}. 
$$

The sample mean is:

$$
 \boldsymbol\mu= \begin{bmatrix}2\\3\end{bmatrix}. 
$$

The centred observations are:

$$
 \mathbf d_1= \begin{bmatrix}-1\\-2\end{bmatrix}, 
$$

$$
 \mathbf d_2= \begin{bmatrix}0\\0\end{bmatrix}, 
$$

$$
 \mathbf d_3= \begin{bmatrix}1\\2\end{bmatrix}. 
$$

Compute outer products:

$$
 \mathbf d_1\mathbf d_1^T= \begin{bmatrix}1&2\\2&4\end{bmatrix}, 
$$

$$
 \mathbf d_2\mathbf d_2^T=0, 
$$

$$
 \mathbf d_3\mathbf d_3^T= \begin{bmatrix}1&2\\2&4\end{bmatrix}. 
$$

Sum:

$$
 S= \begin{bmatrix} 2&4\\4&8 \end{bmatrix}. 
$$

Divide by:

$$
 n-1=2. 
$$

Therefore:

$$
 \boxed{ \widehat\Sigma= \begin{bmatrix} 1&2\\ 2&4 \end{bmatrix} }. 
$$

This is exactly the singular PSD covariance we analyzed earlier.

The samples lie on a straight line, so their variation is effectively one-dimensional.

#### 2 C++23 streaming implementation

```cpp
struct OnlineCovariance2D {
    std::size_t n = 0;

    std::array<double, 2> mean{0.0, 0.0};

    // Accumulated centered co-moments.
    double s00 = 0.0;
    double s01 = 0.0;
    double s11 = 0.0;

    void add(std::array<double, 2> x) {
        ++n;

        // Difference from OLD mean.
        const double dx0 = x[0] - mean[0];
        const double dx1 = x[1] - mean[1];

        // Update mean.
        mean[0] += dx0 / static_cast<double>(n);
        mean[1] += dx1 / static_cast<double>(n);

        // Cross-products use the NEW mean in the second factor.
        s00 += dx0 * (x[0] - mean[0]);
        s01 += dx0 * (x[1] - mean[1]);
        s11 += dx1 * (x[1] - mean[1]);
    }

    [[nodiscard]]
    Mat2 sampleCovariance() const {
        if (n < 2)
            throw std::domain_error(
                "Need at least two observations"
            );

        const double divisor =
            static_cast<double>(n - 1);

        return {
            s00 / divisor,
            s01 / divisor,
            s01 / divisor,
            s11 / divisor
        };
    }
};
```

The off-diagonal value is stored once and copied symmetrically into the resulting matrix.

This prevents a trivial asymmetry caused by independently accumulating $\Sigma\_{01}$ and $\Sigma\_{10}$ in different arithmetic orders.

The update requires constant storage independent of the number of samples.

For higher-dimensional data, it extends naturally to a vector mean and matrix of centred co-moments.

We will develop online mean, Welford variance, statistical estimation, and related numerical details more extensively in the statistical validation chapters.


<!-- end V10.12 -->

<!-- begin V10.13 -->
## 12. Use probability to evaluate graphics and research claims

### Additional connection: Bayesian estimation as optimization

The relation between Bayes' theorem and Gaussian least squares is important enough to derive explicitly.

Suppose we want to estimate an unknown scalar $x$.

Before receiving a measurement, our uncertainty is:

$$
 X\sim\mathcal N(\mu_0,\sigma_0^2). 
$$

This is the prior.

Now observe:

$$
 Y=x+\varepsilon, 
$$

where measurement noise is:

$$
 \varepsilon\sim\mathcal N(0,\tau^2) 
$$

and is independent of $X$.

The likelihood of observation $y$ given $x$ is:

$$
 p(y\mid x) \propto \exp\left( -\frac{(y-x)^2}{2\tau^2} \right). 
$$

The prior density is:

$$
 p(x)\propto \exp\left( -\frac{(x-\mu_0)^2}{2\sigma_0^2} \right). 
$$

Bayes gives:

$$
 p(x\mid y)\propto p(y\mid x)p(x). 
$$

Multiply the two exponential expressions:

$$
 p(x\mid y) \propto \exp\left[ -\frac12 \left( \frac{(y-x)^2}{\tau^2} + \frac{(x-\mu_0)^2}{\sigma_0^2} \right) \right]. 
$$

Therefore maximizing the posterior is equivalent to minimizing:

$$
 \boxed{ E(x)= \frac{(y-x)^2}{2\tau^2} + \frac{(x-\mu_0)^2}{2\sigma_0^2} }. 
$$

The first term fits the measurement.

The second penalizes deviation from the prior.

This is exactly the structure of a regularized least-squares objective.

#### 1 Deriving the posterior mean

Differentiate:

$$
 E'(x)= \frac{x-y}{\tau^2} + \frac{x-\mu_0}{\sigma_0^2}. 
$$

Set equal to zero:

$$
 \frac{x-y}{\tau^2} + \frac{x-\mu_0}{\sigma_0^2} =0. 
$$

Collect $x$:

$$
 x\left( \frac1{\tau^2} +\frac1{\sigma_0^2} \right) = \frac y{\tau^2} +\frac{\mu_0}{\sigma_0^2}. 
$$

Therefore:

$$
 \boxed{ \mu\_{\mathrm{post}} = \frac{ y/\tau^2+\mu_0/\sigma_0^2 }{ 1/\tau^2+1/\sigma_0^2 } }. 
$$

The posterior variance is:

$$
 \boxed{ \sigma\_{\mathrm{post}}^2 = \left( \frac1{\sigma_0^2} +\frac1{\tau^2} \right)^{-1} }. 
$$

Notice that inverse variance acts like information or precision.

A measurement with small variance receives larger weight.

A highly uncertain prior receives smaller weight.

#### 2 Numerical example: Bayesian quadratic objective

Let:

$$
 \mu_0=0,\qquad \sigma_0^2=4. 
$$

We observe:

$$
 y=3 
$$

with measurement variance:

$$
 \tau^2=1. 
$$

The posterior variance is:

$$
 \sigma\_{\mathrm{post}}^2= \left(\frac14+1\right)^{-1} =\boxed{0.8}. 
$$

The posterior mean is:

$$
 \mu\_{\mathrm{post}} = 0.8\left(0+\frac31\right) =\boxed{2.4}. 
$$

The observation says three, but the prior was centered at zero.

The posterior compromise is $2.4$, weighted according to the two uncertainties.

The posterior standard deviation is:

$$
 \sqrt{0.8}\approx0.894427. 
$$

This is smaller than either the prior standard deviation of two or the measurement noise standard deviation of one.

Under the model assumptions, combining information reduces uncertainty.

This simple derivation contains the conceptual foundation of Gaussian sensor fusion, maximum-a-posteriori estimation, and Kalman filtering.

### Probability research bridge: from statistical models to reliable graphics

The theoretical foundations of the probability foundations chapter and the distribution and covariance chapters are directly relevant to research, but only when we preserve the meaning of each probability quantity.

Consider three different uses of Gaussian mathematics.

In sensor calibration, a Gaussian can describe measurement uncertainty.

In probabilistic reconstruction, a Gaussian can describe uncertainty about geometry or pose.

In Gaussian splatting, a Gaussian-shaped spatial kernel can describe a rendering primitive's spatial support or contribution.

These uses share mathematics but are not semantically identical.

A covariance matrix attached to a rendered Gaussian primitive does not automatically quantify epistemic uncertainty about the true location of the scene surface.

Similarly, a small reprojection residual does not automatically imply a narrow posterior uncertainty distribution if the problem contains weakly observed directions or gauge freedoms.

A rigorous research system should distinguish uncertainty about observations, uncertainty about model parameters, and spatial extent represented by a rendering basis function.

#### 1 A useful controlled experiment

One possible experiment is to evaluate covariance propagation through perspective projection.

Start with a family of 3D Gaussian distributions with known means and covariances.

Vary their mean depth, depth variance, anisotropy, orientation, and position relative to the camera centre.

For each configuration, calculate the first-order projected covariance:

$$
 \Sigma\_{\mathrm{predicted}} = J\_\pi\Sigma\_{3D}J\_\pi^T. 
$$

Then generate samples from the 3D distribution, project samples satisfying a defined visibility and clipping policy, and estimate their empirical image-space covariance.

Compare the predicted and empirical covariances.

Because perspective projection is nonlinear and singular at zero depth, the experiment must explicitly handle support near the camera plane and distinguish unconditional distributions from distributions conditioned on valid visibility.

A useful outcome would be to characterize where first-order covariance propagation remains accurate and where higher-order effects or clipping substantially change the result.

Possible diagnostic quantities include covariance eigenvalue error, principal-axis orientation error, mean-projection bias, failure rate, and numerical conditioning.

This connects Phase 9's calculus and optimization with Phase 10's probabilistic modeling.

### Phase 10, Prompt 4 mastery reconstruction

The most valuable exercise is not memorizing the distribution formulas. It is reconstructing why they have their particular form and when their assumptions fail.

For the probability foundations chapter, you should be able to reconstruct the probability axioms, conditional probability, Bayes' theorem, PMF/PDF/CDF distinctions, and the definitions of expectation, variance, covariance, and correlation. You should also derive marginal and conditional distributions from a joint distribution, explain why zero correlation does not imply independence, and demonstrate the laws of total expectation and total variance.

For the distribution and covariance chapters, you should derive the Bernoulli and binomial moments, the uniform mean and variance, Gaussian normalization, exponential memorylessness, the Poisson limit of the binomial, and the multivariate Gaussian density through an affine transformation of independent standard normals.

For the computational portion, rebuild the normal CDF using `erfc`, the exponential CDF using `expm1`, the binomial and Poisson log PMFs using `lgamma`, and the Cholesky-based Gaussian log-density evaluation.

Finally, construct a $2\times2$ covariance matrix that is positive definite, one that is PSD but singular, and one that is indefinite. Explain geometrically what each case means and what an ordinary Cholesky-based Gaussian density evaluator should do.

The mathematical core is the probability space $(\Omega,\mathcal F,P)$, random variables defined on that space, and probability laws for their values.

Conditional probability changes the reference information. Independence is a factorization property. PMFs assign masses to discrete values, while PDFs assign density relative to a measure. CDFs accumulate probability and exist for every real-valued random variable.

Expected value describes the weighted centre. Variance describes squared spread. Covariance describes coupled spread. Correlation standardizes covariance but does not measure every form of dependence.

Bernoulli models one binary trial. Binomial models the count of successes in independent identically distributed Bernoulli trials. Uniform models constant density over a region. Gaussian models a particular smooth bell-shaped distribution. Exponential models memoryless waiting times. Poisson models event counts under its defining assumptions.

A multivariate Gaussian combines mean, covariance, and correlations into a joint distribution. Positive-definite covariance permits a full-dimensional nonsingular Gaussian density. Singular PSD covariance describes a distribution confined to a lower-dimensional affine subspace.

Key engineering invariants: all probabilities are valid and normalized; PDF units match the integration measure; covariance is symmetric PSD; Gaussian density evaluation avoids unnecessary inversion; nonfinite inputs and singular cases are explicit; sampling tests use statistical tolerances rather than exact equality.

#### Further reading: sampling and rendering
For deeper source study, Physically Based Rendering: From Theory to Implementation is particularly valuable because it integrates sampling theory with actual rendering algorithms and source code. 

 Veach's light-transport dissertation is a foundational research reference for probability measures, conditional and marginal densities, Monte Carlo estimators, and multiple importance sampling.

### An exact rendering example: cosine-weighted hemisphere sampling

We can connect the sampling transformations and Monte Carlo sections directly to the rendering equation.

At a surface point, outgoing radiance in direction $\omega_o$ can be expressed as:

$$
 \boxed{ L_o(\omega_o) = \int_H f_r(\omega_i,\omega_o) L_i(\omega_i) (\mathbf n\cdot\omega_i)\\,d\omega_i } 
$$

where $H$ is the surface hemisphere, $f_r$ is the BRDF, $L_i$ is incoming radiance, $\mathbf n$ is the surface normal, and the cosine factor accounts for projected area.

For a direction sampled from a solid-angle PDF $p\_\omega$, a one-sample Monte Carlo contribution is:

$$
 \boxed{ Y= \frac{ f_r(\omega_i,\omega_o) L_i(\omega_i) (\mathbf n\cdot\omega_i) }{ p\_\omega(\omega_i) } }. 
$$

This assumes the integrand and PDF are expressed using compatible measures.

#### 1 Uniform hemisphere sampling

For a uniform hemisphere:

$$
 p\_\omega=\frac1{2\pi}. 
$$

Let:

$$
 z=\cos\theta. 
$$

Under a uniform hemisphere distribution, $z\sim U(0,1)$.

For the test integral:

$$
 I=\int_H\cos\theta\\,d\omega, 
$$

the Monte Carlo contribution becomes:

$$
 Y\_{\mathrm{uniform}} = \frac z{1/(2\pi)} =2\pi z. 
$$

The expected contribution is:

$$
 \mathbb E[Y\_{\mathrm{uniform}}] = 2\pi\mathbb E[z] =2\pi\frac12 =\pi. 
$$

Its variance is:

$$
 \operatorname{Var}(Y\_{\mathrm{uniform}}) = (2\pi)^2\operatorname{Var}(z). 
$$

Because a uniform $[0,1]$ variable has variance $1/12$:

$$
 \boxed{ \operatorname{Var}(Y\_{\mathrm{uniform}}) = \frac{\pi^2}{3} }. 
$$

#### 2 Cosine-weighted hemisphere sampling

Instead, choose:

$$
 \boxed{ p\_\omega(\omega)=\frac{\cos\theta}{\pi} }. 
$$

It is normalized because:

$$
 \int_H\frac{\cos\theta}{\pi}\\,d\omega=1. 
$$

The weighted contribution becomes:

$$
 Y\_{\mathrm{cosine}} = \frac{\cos\theta}{\cos\theta/\pi} =\boxed{\pi} 
$$

where the PDF is positive.

Every ideal sample gives exactly $\pi$.

Therefore:

$$
 \boxed{ \operatorname{Var}(Y\_{\mathrm{cosine}})=0 }. 
$$

That explains the program output of $3.141592654$.

#### 3 Constructing the cosine-weighted direction

Take two uniform random numbers:

$$
 u_1,u_2\in[0,1). 
$$

Construct:

$$
 r=\sqrt{u_1}, \qquad \phi=2\pi u_2. 
$$

Then:

$$
 \boxed{ \omega= \begin{bmatrix} r\cos\phi\\ r\sin\phi\\ \sqrt{1-u_1} \end{bmatrix} }. 
$$

This first samples a uniform point on the unit disk, then lifts it onto the upper hemisphere.

The resulting solid-angle PDF is:

$$
 \boxed{ p\_\omega=\frac{\omega_z}{\pi} }. 
$$

The complete reference implementation includes this sampling map.

#### 4 Lambertian reflection

For a Lambertian surface of reflectance $\rho$:

$$
 f_r=\frac\rho\pi. 
$$

Suppose incoming radiance is constant:

$$
 L_i(\omega)=L. 
$$

The outgoing radiance becomes:

$$
 L_o= \int_H \frac\rho\pi L\cos\theta\\,d\omega. 
$$

Because:

$$
 \int_H\cos\theta\\,d\omega=\pi, 
$$

we obtain:

$$
 \boxed{L_o=\rho L}. 
$$

A cosine-weighted importance sample gives contribution:

$$
 Y= \frac{ (\rho/\pi)L\cos\theta }{ \cos\theta/\pi } =\rho L. 
$$

The estimator has zero variance for this idealized constant-radiance Lambertian case.

In a real scene, incoming radiance varies with direction and can be affected by visibility, other materials, and complex transport.

Cosine-weighted sampling no longer guarantees zero variance, but it still matches an important factor of the integrand.

This is the practical meaning of importance sampling in rendering.

### Related techniques: multiple importance sampling, Russian roulette, and control variates

These techniques extend the mathematics we have now established. We will revisit advanced sampling designs later, but their foundational equations belong in the complete picture.

#### 1 Multiple importance sampling

Sometimes there is no single PDF that approximates the integrand well everywhere.

In direct lighting, sampling a light source may be effective for small bright lights, while sampling the BSDF may be effective for narrow specular lobes.

Multiple importance sampling, or MIS, combines samples from different distributions.

Suppose technique $j$ draws $n_j$ samples from $p_j(x)$.

A general MIS estimator is:

$$
 \boxed{ \widehat I= \sum_j\frac1{n_j} \sum\_{i=1}^{n_j} w_j(X\_{ji}) \frac{f(X\_{ji})}{p_j(X\_{ji})} }. 
$$

The weights must satisfy suitable conditions, including:

$$
 \sum_jw_j(x)=1 
$$

where the integrand contributes and the combined sampling techniques provide coverage.

One common weighting scheme is the balance heuristic:



$$
 \boxed{ w_j(x)= \frac{n_jp_j(x)} {\sum_kn_kp_k(x)} }. 
$$

These weights favor techniques that assign relatively high sampling probability to the evaluated point.

For unbiasedness, each technique must have the stated marginal density, the weights must sum to one wherever the integrand contributes, and the techniques must cover that support. Independence between techniques is not needed for the expectation because expectation is linear, but dependence adds covariance terms to estimator variance. Use an independent-sample variance formula only when its assumptions hold.

When using MIS in rendering, every compared PDF must be expressed with respect to a compatible measure.

Area-light PDFs, solid-angle PDFs, and discrete probabilities for delta lights cannot be combined naively without handling their different measure structures.

Veach's research on robust Monte Carlo light transport established much of the formal MIS framework used in physically based rendering.

#### 2 Russian roulette

A path tracer may terminate a path randomly to avoid evaluating arbitrarily long paths.

Suppose the remaining path contribution would be $Y$.

Let the survival probability be:

$$
 q\in(0,1]. 
$$

Define:

$$
 \widehat Y= \begin{cases} Y/q,&\text{with probability }q,\\ 0,&\text{with probability }1-q. \end{cases} 
$$

Then:

$$
 \mathbb E[\widehat Y\mid Y] = q\frac Yq+(1-q)0 =Y. 
$$

Thus:

$$
 \boxed{ \mathbb E[\widehat Y]=\mathbb E[Y] }. 
$$

Proper Russian-roulette termination can preserve unbiasedness while reducing expected computational work.

But it increases variance relative to always evaluating the same contribution when the latter is feasible.

Termination probabilities must be chosen carefully, especially when throughput changes across paths.

Deterministically truncating paths after a fixed depth is generally a different procedure and can introduce bias by omitting nonzero higher-bounce contributions.

#### 3 Control variates

Suppose we want:

$$
 I=\mathbb E[Y]. 
$$

Assume we also have a correlated variable $C$ with known expectation:

$$
 \mathbb E[C]=\mu_C. 
$$

Construct:

$$
 \boxed{ Z=Y-\beta(C-\mu_C) }. 
$$

Its expectation is:

$$
 \mathbb E[Z] = \mathbb E[Y]-\beta(\mathbb E[C]-\mu_C) =I. 
$$

So the estimator remains unbiased for any fixed $\beta$ under the assumptions.

Its variance is:

$$
 \begin{aligned} \operatorname{Var}(Z) &= \operatorname{Var}(Y) +\beta^2\operatorname{Var}(C)\\ &\quad-2\beta\operatorname{Cov}(Y,C). \end{aligned} 
$$

Differentiate with respect to $\beta$:

$$
 2\beta\operatorname{Var}(C) -2\operatorname{Cov}(Y,C)=0. 
$$

Therefore the variance-minimizing coefficient is:

$$
 \boxed{ \beta^\star= \frac{\operatorname{Cov}(Y,C)} {\operatorname{Var}(C)} } 
$$

when $\operatorname{Var}(C)>0$.

This explicitly uses covariance to reduce Monte Carlo variance.

It is an elegant connection between the probability foundations chapter's covariance theory, the sampling and Monte Carlo chapters's estimators, and the statistical validation chapters's statistical analysis.

### Designing a statistically credible renderer benchmark

A mathematically rigorous benchmark should distinguish several independent questions.

Is the estimator correct? For simple analytical scenes, compare its expectation or repeated-run average against a known solution.

Is the implementation numerically stable? Test extreme PDFs, near-zero weights, grazing angles, invalid geometry, very large sample counts, and accumulation precision.

How variable is the estimate? Repeat the experiment using multiple independently generated streams and measure between-run variance, not only variance calculated from one fixed correlated sequence.

Does the method improve efficiency? Compare error versus sample count and error versus wall-clock time. Importance sampling can reduce variance per sample while increasing computation per sample.

Does the result generalize? Evaluate multiple representative scenes, materials, light configurations, and sampling budgets. Report distributional metrics rather than selecting a single showcase scene.

A rigorous design might fix a collection of benchmark scenes, define a trusted reference value or image for each, and preselect a collection of test seeds.

For each method and each seed, run the same specified workload and collect pixel estimates, error metrics, runtime, sampling count, and failure diagnostics.

Then compare the methods using paired analyses where appropriate.

This produces evidence about estimator bias, variance, computational cost, and robustness instead of merely showing that one rendered image looks cleaner.

A particularly valuable experiment for your renderer would compare its current sampler against a corrected arbitrary-count sampler over non-square sample counts such as 3, 5, 7, and 10, as well as perfect-square counts.

Use a set of analytically integrable pixel functions, including constant, linear, and localized peaked functions.

For each count, repeat with multiple seeds and measure both the average error and its variance.

This can directly test whether the current partial-grid design introduces systematic integration bias and whether a revised sampler improves performance.

### Final synthesis: what the 35 parts establish

the sampling and Monte Carlo chapters and the statistical validation chapters should now form one continuous chain in your mental model:

$$
 \boxed{ \text{Generator} \rightarrow \text{Sample distribution} \rightarrow \text{Weighted contribution} \rightarrow \text{Estimator} \rightarrow \text{Statistical evidence} } 
$$

Every arrow has assumptions.

A generator must produce suitable random-looking inputs.

A sampling transformation must produce the claimed probability law.

The estimator must use the correct PDF and measure.

Its variance must be analyzed under the actual dependence structure.

The statistics must distinguish finite-sample uncertainty from bias and numerical error.

A wrong PDF can create bias. A high-variance estimator can produce unstable images. Correlated samples can invalidate naive standard-error calculations. A poor summary metric can conceal outliers or perceptually important errors.

And a confidence interval cannot rescue an estimator whose expectation is wrong.

#### Further reading: probability and inference
The most valuable external references for continued study are [Physically Based Rendering, Fourth Edition](https://pbr-book.org/4ed/?utm_source=chatgpt.com) for the sampling and light-transport implementation, [O'Neill's PCG research paper](https://www.pcg-random.org/paper.html?utm_source=chatgpt.com) for PRNG architecture, [Welford's original corrected-sums paper](https://doi.org/10.1080/00401706.1962.10490022?utm_source=chatgpt.com) for numerical statistics, and [NIST's confidence-interval treatment](https://www.itl.nist.gov/div898/handbook/eda/section3/eda352.htm?utm_source=chatgpt.com) for statistical inference.



<!-- end V10.13 -->

<!-- begin V10.21 -->
## Keep probability weights stable before sampling


In stochastic rendering, probabilistic inference, and discrete sampling, we often start with nonnegative weights:

$$ w_1,\ldots,w_n. $$

We want normalized probabilities:

$$ \boxed{ p_i=\frac{w_i}{\sum_jw_j} }. $$

This formula is mathematically straightforward but can overflow or underflow.

### 28.1 Overflow example

Suppose:

$$ w_1=w_2=w_3=10^{308}. $$

Each weight is finite in binary64.

But their sum is:

$$ 3\times10^{308}, $$

which exceeds the largest finite binary64 value.

The direct sum overflows to infinity.

Naive normalization gives:

$$ p_i=\frac{10^{308}}{\infty}=0. $$

All three probabilities become zero.

Their sum is zero, not one.

The implementation has destroyed a perfectly valid distribution.

### 28.2 Scale by the largest weight

Define:

$$ \boxed{ m=\max_iw_i }. $$

Assume all weights are finite, nonnegative, and at least one is positive.

Then:

$$ p_i= \frac{w_i/m}{\sum_jw_j/m}. $$

Why is this equivalent?

Multiply numerator and denominator by $m$:

$$ \frac{w_i/m}{\sum_jw_j/m} = \frac{w_i}{\sum_jw_j}. $$

Now every scaled weight satisfies:

$$ 0\le\frac{w_i}{m}\le1. $$

The intermediate sum cannot overflow merely because individual weights are near the floating-point maximum, provided the count and accumulation are handled within representable limits.

For our example:

$$ m=10^{308}. $$

Every scaled weight is one.

The total is three.

Thus:

$$ \boxed{ p_1=p_2=p_3=\frac13 }. $$

### 28.3 Underflow is a separate concern

Suppose:

$$ w_1=1, \qquad w_2=10^{-300}, \qquad w_3=10^{-1000} $$

in ideal real arithmetic.

The third value is not representable as a positive binary64 number and underflows to zero when stored.

No normalization strategy can recover a nonzero value that never survived the input representation.

If weights span extreme logarithmic ranges, represent them as log weights from the beginning.

That leads directly to the log-sum-exp method.

### 28.4 Reject invalid probability inputs

A robust weight-normalization API should define behavior for:

- Empty arrays.
- All-zero weights.
- Negative weights.
- NaNs.
- Positive infinities.
- Underflowed values.
- Extremely large collections.
- A sum that rounds slightly away from one.

The example laboratory rejects negative and nonfinite ordinary weights and rejects an all-zero weight vector.

Some applications may want a special policy for positive infinite weights, such as assigning probability uniformly among all infinite-weight entries.

That can be defined, but it must be explicit. It is not a consequence of ordinary finite-weight normalization.


Suppose we have log weights:

$$ \ell_1,\ldots,\ell_n. $$

The corresponding unnormalized weights are:

$$ w_i=e^{\ell_i}. $$

The log of their total is:

$$ \boxed{ L=\log\left(\sum_ie^{\ell_i}\right) }. $$

This is the log-sum-exp function.

A naive implementation calculates each exponential, sums, and takes the logarithm.

But exponentials can overflow.

### 29.1 A numerical example

Consider:

$$ \ell=(1000,1001,1000). $$

The exact total is:

$$ e^{1000}+e^{1001}+e^{1000}. $$

Every individual exponential exceeds the representable range of binary64.

A naive implementation produces infinity.

But the correct log-sum-exp is finite.

Factor out $e^{1001}$:

$$ \begin{aligned} L &= \log\left( e^{1001} (e^{-1}+1+e^{-1}) \right)\\ &= 1001+\log(1+2e^{-1}). \end{aligned} $$

Numerically:

$$ \boxed{ L\approx1001.551444713932 }. $$

### 29.2 General derivation

Choose:

$$ m=\max_i\ell_i. $$

Then:

$$ \sum_ie^{\ell_i} = e^m\sum_ie^{\ell_i-m}. $$

Taking logarithms:

$$ \boxed{ L= m+\log\left( \sum_ie^{\ell_i-m} \right) }. $$

Every exponent:

$$ \ell_i-m $$

is nonpositive.

Therefore:

$$ e^{\ell_i-m}\le1. $$

The largest exponent is zero and contributes exactly one in ideal arithmetic.

This prevents exponential overflow for finite log weights.

### 29.3 Softmax probabilities

The normalized probabilities are:

$$ \boxed{ p_i= \frac{e^{\ell_i}} {\sum_je^{\ell_j}} }. $$

Using the shifted expression:

$$ \boxed{ p_i= \frac{e^{\ell_i-m}} {\sum_je^{\ell_j-m}} }. $$

For:

$$ \ell=(1000,1001,1000), $$

the shifted values are:

$$ (-1,0,-1). $$

The unnormalized weights become:

$$ (e^{-1},1,e^{-1}). $$

The total is:

$$ 1+2e^{-1}\approx1.735758882. $$

Therefore:

$$ \boxed{ p\approx (0.211941558,\ 0.576116885,\ 0.211941558) }. $$

The probabilities are finite, nonnegative, and sum to approximately one.

### 29.4 Numerical implementation

```
double logsumexp(std::span<const double> logits) {
    if (logits.empty())
        throw std::invalid_argument("No log weights");

    // A production API must also define its policy for
    // NaN, +infinity and all -infinity inputs.
    const double m =
        *std::max_element(logits.begin(), logits.end());

    double sum = 0.0;

    for (const double x : logits) {
        // The largest exponent is zero.
        // Every other finite exponent is negative.
        sum += std::exp(x - m);
    }

    return m + std::log(sum);
}
```

The complete reference implementation adds explicit validation and uses compensated accumulation for the exponentials.

### 29.5 Log-sum-exp bounds

Let:

$$ m=\max_i\ell_i. $$

At least one shifted exponential equals one, so:

$$ \sum_ie^{\ell_i-m}\ge1. $$

Also, there are $n$ terms, each at most one:

$$ \sum_ie^{\ell_i-m}\le n. $$

Take logarithms:

$$ 0\le \log\left(\sum_ie^{\ell_i-m}\right) \le\log n. $$

Add $m$:

$$ \boxed{ m\le\operatorname{LSE}(\ell)\le m+\log n }. $$

Thus log-sum-exp is a smooth approximation to the maximum.

### 29.6 Derivative of log-sum-exp

Let:

$$ L(\ell)=\log\left(\sum_je^{\ell_j}\right). $$

Differentiate with respect to $\ell_i$:

$$ \frac{\partial L}{\partial\ell_i} = \frac{e^{\ell_i}}{\sum_je^{\ell_j}}. $$

Therefore:

$$ \boxed{ \nabla L=\operatorname{softmax}(\ell) }. $$

The Hessian is:

$$ \boxed{ H_L= \operatorname{diag}(\mathbf p)-\mathbf p\mathbf p^T }. $$

This matrix is positive semidefinite.

Why?

It has the same structure as the covariance matrix of a categorical one-hot random vector.

Thus log-sum-exp is convex.

This connects probability normalization, covariance matrices, optimization, and automatic differentiation.

### 29.7 Research grounding

Blanchard, Desmond Higham, and Nicholas Higham studied floating-point accuracy of log-sum-exp and softmax formulations, including shifting, overflow avoidance, and differences between mathematically equivalent evaluation methods. Their numerical analysis supports using carefully shifted formulas rather than assuming all algebraically equivalent implementations are equally accurate.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://academic.oup.com\&sz=32)

Oxford Academic+1





<!-- end V10.21 -->

<!-- begin V10.22 -->
## Design samples: without replacement, stratified, and low-discrepancy


Suppose we have $N$ distinct elements and want to select $k$ of them without selecting any element twice.

Sampling without replacement differs fundamentally from sampling with replacement.

With replacement, each draw can select an element already chosen.

Without replacement, the remaining candidate set changes after every draw.

### 30.1 A simple example

Suppose we have four possible sample locations:

$$ A,B,C,D. $$

We choose two without replacement.

If the first selection is $A$, the second must come from:

$$ B,C,D. $$

There are:

$$ 4\times3=12 $$

ordered possible pairs, assuming uniform choice at each stage.

The probability of the ordered pair $(A,B)$ is:

$$ \frac14\cdot\frac13=\frac1{12}. $$

If order does not matter, there are:

$$ \binom42=6 $$

unordered pairs.

Each unordered pair corresponds to two ordered pairs, so its probability is:

$$ \frac26=\frac16 $$

after the appropriate counting, or directly $1/\binom42$.

### 30.2 Fisher–Yates shuffle

The Fisher–Yates algorithm generates a random permutation of an array.

For indices:

$$ 0,1,\ldots,N-1, $$

start from the final position.

Choose a uniformly random index from the remaining unswapped prefix.

Swap that element into the current final position.

Repeat.

```
template<class URBG>
void fisherYates(
    std::span<std::uint32_t> indices,
    URBG& rng
) {
    for (std::size_t i = indices.size(); i > 1; --i) {
        // Uniform among all remaining positions.
        std::uniform_int_distribution<std::size_t> dist(
            0, i - 1
        );

        const std::size_t j = dist(rng);

        std::swap(indices[i - 1], indices[j]);
    }
}
```

### 30.3 Proving uniformity

At the first step, each of the $N$ elements has probability:

$$ \frac1N $$

of occupying the final position.

At the second step, each of the remaining $N-1$ elements has probability:

$$ \frac1{N-1} $$

of occupying the preceding position.

Continue.

The probability of any particular complete permutation is:

$$ \frac1N \cdot \frac1{N-1} \cdots \frac12 \cdot1. $$

Therefore:

$$ \boxed{ P(\text{one specified permutation})=\frac1{N!} }. $$

Every permutation has the same probability under exact uniform index selection.

The algorithm takes:

$$ \boxed{O(N)} $$

time and can operate in place.

If only $k$ elements are needed, a partial shuffle can produce the first $k$ selections without shuffling every element.

### 30.4 Why unbiased integer selection matters

A Fisher–Yates shuffle is not uniform if the bounded integer generator introduces unequal index probabilities.

For example:

```
j = rng() % i;
```

can produce modulo bias when the generator's integer range is not divisible by $i$.

Therefore the bounded-index selection must be designed to avoid that bias.

The complete reference uses `std::uniform_int_distribution` for this purpose, while documenting that its precise output stream may vary between standard-library implementations.

### 30.5 Reservoir sampling

Suppose a dataset is too large to store, or its size is not known in advance.

We want a uniform sample of $k$ elements without replacement.

Reservoir sampling maintains only $k$ selected elements while processing a stream.

For the classic Algorithm R:

1. Store the first $k$ elements.
2. For stream element $i$, using one-based indexing, select a random integer $j$ uniformly from $1$ through $i$.
3. If $j\le k$, replace reservoir entry $j$ with the new element.

This produces a uniform size-$k$ subset of the first $N$ elements when the algorithm completes.

### 30.6 Why reservoir sampling is uniform

Consider an element arriving at position $i$.

For $i>k$, it enters the reservoir with probability:

$$ \frac{k}{i}. $$

At a later step $j>i$, any particular reservoir element is replaced with probability:

$$ \frac{k}{j}\cdot\frac1k=\frac1j. $$

Thus its probability of surviving step $j$ is:

$$ 1-\frac1j=\frac{j-1}{j}. $$

Its probability of surviving all subsequent steps through $N$ is:

$$ \prod_{j=i+1}^{N}\frac{j-1}{j} = \frac iN. $$

Multiply the entry probability and survival probability:

$$ \frac ki\cdot\frac iN = \boxed{\frac kN}. $$

Each element has the same inclusion probability.

This is useful for selecting debug samples from large streams, sampling records from simulations, and building bounded-memory statistics.


Ordinary independent random sampling may leave large gaps and produce clusters.

Stratified sampling divides the domain into regions called strata and controls how many samples are drawn from each region.

Suppose:

$$ D=\bigcup_{j=1}^{K}D_j $$

where strata are disjoint apart from boundaries.

Let:

$$ P(X\in D_j)=w_j. $$

The expected value can be decomposed:

$$ \mathbb E[f(X)] = \sum_jw_j\mathbb E[f(X)\mid X\in D_j]. $$

This follows from the law of total expectation.

### 31.1 Equal-volume stratification

For uniform sampling on $[0,1]$, divide the domain into $N$ equal intervals:

$$ D_i= \left[\frac iN,\frac{i+1}{N}\right). $$

Generate one independent uniform jitter:

$$ U_i\sim U(0,1). $$

Construct:

$$ \boxed{ X_i=\frac{i+U_i}{N} } $$

for:

$$ i=0,\ldots,N-1. $$

Every interval receives exactly one sample.

The equal-weight estimator is:

$$ \boxed{ \widehat I_{\mathrm{strat}} = \frac1N\sum_i f(X_i) }. $$

### 31.2 Proving unbiasedness

Each $X_i$ is uniform within its own interval.

Therefore:

$$ \mathbb E[f(X_i)] = N\int_{i/N}^{(i+1)/N}f(x)\,dx. $$

Take expectation of the estimator:

$$ \begin{aligned} \mathbb E[\widehat I_{\mathrm{strat}}] &= \frac1N\sum_i\mathbb E[f(X_i)]\\ &= \frac1N\sum_i N\int_{i/N}^{(i+1)/N}f(x)\,dx\\ &= \sum_i\int_{i/N}^{(i+1)/N}f(x)\,dx\\ &= \int_0^1f(x)\,dx. \end{aligned} $$

Thus:

$$ \boxed{ \mathbb E[\widehat I_{\mathrm{strat}}]=I }. $$

The estimator is unbiased under independent uniform jitter within each stratum and the stated equal-volume setup.

### 31.3 Variance reduction for a linear function

Consider:

$$ f(x)=x. $$

The exact integral is:

$$ I=\int_0^1x\,dx=\frac12. $$

Under ordinary independent uniform Monte Carlo with $N$ samples:

$$ \operatorname{Var}(\widehat I_{\mathrm{MC}}) = \frac{\operatorname{Var}(U)}N = \boxed{\frac1{12N}}. $$

Under equal-width stratification:

$$ X_i=\frac{i+U_i}{N}. $$

Its variance is:

$$ \operatorname{Var}(X_i) = \frac{\operatorname{Var}(U_i)}{N^2} = \frac1{12N^2}. $$

The strata jitters are independent, so the estimator variance is:

$$ \operatorname{Var}(\widehat I_{\mathrm{strat}}) = \frac1{N^2} \sum_{i=0}^{N-1}\operatorname{Var}(X_i). $$

Therefore:

$$ \operatorname{Var}(\widehat I_{\mathrm{strat}}) = \frac1{N^2} N\frac1{12N^2}. $$

Thus:

$$ \boxed{ \operatorname{Var}(\widehat I_{\mathrm{strat}}) = \frac1{12N^3} }. $$

Compare:

$$ \frac{ \operatorname{Var}(\widehat I_{\mathrm{MC}}) }{ \operatorname{Var}(\widehat I_{\mathrm{strat}}) } = \frac{1/(12N)}{1/(12N^3)} = \boxed{N^2}. $$

For the simple linear function, stratification produces a dramatic variance reduction.

This $N^2$ factor is specific to this integrand and stratification scheme. It is not a universal rate for every problem.

### 31.4 General variance decomposition

For equal-probability strata, let each stratum have conditional mean $\mu_i$ and conditional variance $\sigma_i^2$.

For one independent sample per stratum:

$$ \boxed{ \operatorname{Var}(\widehat I_{\mathrm{strat}}) = \frac1{N^2}\sum_i\sigma_i^2 }. $$

For ordinary IID sampling from the entire domain:

$$ \operatorname{Var}(\widehat I_{\mathrm{MC}}) = \frac{\sigma^2}{N}. $$

The law of total variance gives:

$$ \sigma^2= \frac1N\sum_i\sigma_i^2 + \frac1N\sum_i(\mu_i-\mu)^2. $$

Therefore:

$$ \operatorname{Var}(\widehat I_{\mathrm{MC}}) = \frac1{N^2}\sum_i\sigma_i^2 + \frac1{N^2}\sum_i(\mu_i-\mu)^2. $$

Subtract stratified variance:

$$ \boxed{ \operatorname{Var}(\widehat I_{\mathrm{MC}}) - \operatorname{Var}(\widehat I_{\mathrm{strat}}) = \frac1{N^2}\sum_i(\mu_i-\mu)^2 \ge0 }. $$

Thus under this precise equal-probability, one-independent-sample-per-stratum design, stratification cannot have greater variance than ordinary IID sampling of the same count.

But other practical schemes, imperfect allocations, dependent randomization, or different weighting rules need their own analysis.

### 31.5 The non-square pixel sampling problem

Suppose a pixel sampler uses:

$$ s=\lceil\sqrt N\rceil $$

and constructs an $s\times s$ grid, but draws only from the first $N$ cells.

If $N\ne s^2$, some cells are never sampled.

An equal-weight average over the chosen cells generally does not represent the full pixel integral.

This is a risk in the current inspected `KairoRayTracer/Sampler.cppm` implementation and should be corrected or explicitly compensated for.

Possible solutions include choosing exact grid dimensions and sample counts, using a suitable weighted partial-strata construction, or using a different arbitrary-count sampling design.

Latin-hypercube sampling is one such design.


Latin-hypercube sampling, or LHS, generalizes one-dimensional stratification across multiple coordinate dimensions.

Suppose we want $N$ samples in:

$$ [0,1]^d. $$

For each dimension, divide the interval into $N$ equal strata.

Each stratum is used exactly once along that coordinate.

### 32.1 Two-dimensional construction

Generate a random permutation:

$$ \pi:\{0,\ldots,N-1\}\rightarrow\{0,\ldots,N-1\}. $$

Generate independent jitters:

$$ U_i,V_i\sim U(0,1). $$

Construct:

$$ \boxed{ x_i=\frac{i+U_i}{N} } $$

and:

$$ \boxed{ y_i=\frac{\pi(i)+V_i}{N} }. $$

Every horizontal stratum receives one sample.

Every vertical stratum receives one sample.

### 32.2 Why the permutation matters

Without permutation, we might pair:

$$ x_i\in D_i,\qquad y_i\in D_i. $$

This would place samples near the diagonal of the unit square.

We would have good one-dimensional coverage but poor two-dimensional joint coverage.

A randomized permutation breaks that obvious diagonal alignment.

However, Latin-hypercube sampling does not guarantee that every two-dimensional cell of an $N\times N$ grid is occupied.

There are $N^2$ such cells but only $N$ samples.

Its main elementary guarantee is one-dimensional marginal stratification.

### 32.3 Arbitrary sample counts

Unlike a perfect-square grid, Latin-hypercube sampling works naturally for:

$$ N=3,\ 5,\ 7,\ 10,\ldots $$

without leaving an unused rectangular region merely because $N$ is not a square.

This makes it useful for flexible per-pixel sample budgets.

### 32.4 Higher-dimensional construction

In $d$ dimensions, choose a separate random permutation:

$$ \pi_j $$

for each dimension $j$.

Construct:

$$ \boxed{ x_{ij}= \frac{\pi_j(i)+U_{ij}}N }. $$

Here $i$ indexes samples and $j$ indexes dimensions.

Each coordinate dimension visits all its strata exactly once.

### 32.5 Why LHS can help

If an integrand's variation is strongly influenced by individual coordinates, LHS can reduce sampling variability by ensuring good marginal coverage.

But interactions between dimensions can still cause difficult behavior.

For example, an integrand may be concentrated in a narrow diagonal region. One-dimensional marginal stratification alone does not guarantee efficient sampling of that structure.

Different randomized LHS designs also produce different dependence structures.

We should not apply the ordinary IID standard-error formula automatically to the individual LHS samples.

For statistical error assessment, use properly randomized independent LHS replicates or another analysis appropriate to the design.


Ordinary pseudorandom sampling aims to emulate independent random draws.

Quasi-random methods instead construct sequences intended to cover the sampling domain evenly.

They are often called low-discrepancy sequences.

Common families include Halton and Sobol sequences.

### 33.1 Discrepancy intuition

Imagine sampling points inside a unit square.

For any axis-aligned rectangle:

$$ R=[0,a)\times[0,b), $$

the rectangle occupies fraction:

$$ ab $$

of the unit square's area.

If we have $N$ sample points, the empirical fraction inside that rectangle is:

$$ \frac{\#\{\mathbf x_i\in R\}}{N}. $$

The discrepancy for that rectangle is the difference between empirical and expected area fraction:

$$ \left| \frac{\#\{\mathbf x_i\in R\}}{N}-ab \right|. $$

The star discrepancy is:

$$ \boxed{ D_N^\star= \sup_{a,b\in[0,1]} \left| \frac{\#\{\mathbf x_i\in[0,a)\times[0,b)\}}N-ab \right| }. $$

In $d$ dimensions, use anchored axis-aligned boxes.

Low-discrepancy sequences seek to make these coverage differences small.

### 33.2 Why discrepancy matters for integration

The Koksma–Hlawka inequality states, under appropriate bounded-variation assumptions:

$$ \boxed{ \left| \frac1N\sum_{i=1}^{N}f(\mathbf x_i) - \int_{[0,1]^d}f(\mathbf x)\,d\mathbf x \right| \le V_{\mathrm{HK}}(f)D_N^\star }. $$

Here:

- $V_{\mathrm{HK}}(f)$ is the Hardy–Krause variation of the integrand.
- $D_N^\star$ is the sample set's star discrepancy.

The inequality links numerical integration error to both integrand structure and sample coverage.

This is different from the standard independent Monte Carlo variance formula.

But the bound can be uninformative or inapplicable for discontinuous or extremely irregular integrands with unsuitable variation properties.

Rendering integrands often contain visibility discontinuities, so low-discrepancy theory should be interpreted carefully.

### 33.3 Halton sequence

A Halton sequence uses radical-inverse functions in different integer bases.

For base $b$, write an integer $n$ in base $b$:

$$ n=a_0+a_1b+a_2b^2+\cdots. $$

The radical inverse reflects the digits across the decimal point in base $b$:

$$ \boxed{ \phi_b(n)= \frac{a_0}{b}+ \frac{a_1}{b^2}+ \frac{a_2}{b^3}+\cdots }. $$

### 33.4 Binary radical-inverse example

Take:

$$ n=5. $$

In binary:

$$ 5=(101)_2. $$

Reverse the binary digits after the radix point:

$$ \phi_2(5)=(0.101)_2. $$

Therefore:

$$ \boxed{ \phi_2(5)=\frac12+\frac18=\frac58=0.625 }. $$

The early base-two radical-inverse values, starting from $n=1$, are:

$$ 0.5,\quad0.25,\quad0.75,\quad0.125,\quad0.625,\ldots $$

These values spread across progressively finer dyadic intervals.

### 33.5 Two-dimensional Halton

A common 2D Halton construction uses bases two and three:

$$ \boxed{ \mathbf x_n= (\phi_2(n),\phi_3(n)) }. $$

For $n=1$:

$$ \boxed{ \mathbf x_1= \left(\frac12,\frac13\right) }. $$

For $n=2$:

$$ \boxed{ \mathbf x_2= \left(\frac14,\frac23\right) }. $$

For $n=3$:

$$ \boxed{ \mathbf x_3= \left(\frac34,\frac19\right) }. $$

The points are deterministic and deliberately distributed.

### 33.6 Sobol sequences

Sobol sequences use base-two digital constructions involving direction numbers.

They are especially common in modern rendering because they can provide good multidimensional coverage and efficient digital implementation.

However, the quality depends on dimension assignments, direction numbers, scrambling, and sampling layout.

Simply labeling a sequence "Sobol" does not guarantee optimal behavior for every integrand.

### 33.7 Randomized quasi-Monte Carlo

Deterministic low-discrepancy points do not form independent random samples.

If we want statistical error estimates across repeated runs, we can use randomized low-discrepancy constructions.

One simple randomization for periodic integration domains is a random shift modulo one:

$$ \boxed{ \tilde{\mathbf x}_i = (\mathbf x_i+\mathbf U)\bmod1 } $$

where $\mathbf U\sim U([0,1]^d)$ is a random shift.

For suitable settings, this preserves uniform marginal distribution and allows independent randomized replicates using different shifts.

More sophisticated randomizations, such as Owen scrambling for digital nets, can retain valuable low-discrepancy properties while introducing controlled randomness.

The expected estimator and variance must be analyzed according to the actual randomized design.

### 33.8 Why rendering benefits

In many rendering problems, improved coverage reduces visible clustering and can improve convergence.

However, the best sampler also depends on the integrand's dimensionality, discontinuities, path-structure decisions, and how dimensions are mapped to random variables.

The official [pbrt-v4 User's Guide](https://www.pbrt.org/users-guide-v4?utm_source=chatgpt.com) describes Sobol-family and other samplers and notes that the practical benefits vary with sampling rates and rendering workloads.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbrt.org\&sz=32)

pbrt.org





### Figure N10-11 — Arbitrary sample counts need full-domain support

![[Visual Recall/Math Foundations/N10-11.png]]

Left: choosing the first five of nine equal cells and averaging equally gives E[y] = 0.3, while the uniform-square value is 0.5. Right: one point per marginal bin is a midpoint illustration; randomized LHS jitters and permutes appropriately, and is not full multidimensional stratification.


<!-- end V10.22 -->


<!-- BEGIN IMPLEMENTABLE CONTACT MATHEMATICS 2026-11-12 -->
### From a geometric gap to a numerically trustworthy collision event

A collision algorithm is an example of a mathematical model meeting finite-precision computation. The same problem ties together vector spaces, inner products, gradients, root finding, Jacobians, unilateral constraints, and numerical error. We will derive the complete system so that implementing it later requires translating quantities into memory, not inventing missing mathematics.

Consider two planar disks A and B. Their centres are $\mathbf x_A,\mathbf x_B\in\mathbb R^2$ (metres), their constant velocities over the current candidate interval are $\mathbf v_A,\mathbf v_B\in\mathbb R^2$ (metres per second), and their radii $r_A,r_B$ are nonnegative lengths. Define relative position $\mathbf p=\mathbf x_B-\mathbf x_A$, relative velocity $\mathbf v=\mathbf v_B-\mathbf v_A$, and sum of radii $R=r_A+r_B$. Because only relative separation matters, adding the same velocity $\mathbf u$ to both objects does not change the contact time: $(\mathbf v_B+\mathbf u)-(\mathbf v_A+\mathbf u)=\mathbf v$. This is a Galilean invariance test for the mathematics and its implementation.

At time $t\in[0,h]$, their centre displacement is $\mathbf p(t)=\mathbf p+t\mathbf v$. Their signed surface separation is

$
g(t)=\|\mathbf p+t\mathbf v\|-R.
$

Here $g>0$ means separated, $g=0$ means touching, and $g<0$ means overlapping. At a new impact we seek the smallest nonnegative root of $g(t)=0$. Squaring the equality is safe because both the norm and $R$ are nonnegative, giving the polynomial

$
F(t)=\|\mathbf p+t\mathbf v\|^2-R^2
=(\mathbf v\cdot\mathbf v)t^2
+2(\mathbf p\cdot\mathbf v)t
+\mathbf p\cdot\mathbf p-R^2.
$

Write $a=\mathbf v\cdot\mathbf v$ (units m²/s²), $b=2\mathbf p\cdot\mathbf v$ (m²/s), and $c=\mathbf p\cdot\mathbf p-R^2$ (m²). The discriminant $D=b^2-4ac$ has units m⁴/s². If $c>0$, the disks begin separated. If $a=0$, relative motion is zero and a future new impact cannot occur. If $b\ge0$, squared separation is initially nondecreasing, and with $a\ge0$ it never turns toward a contact in forward time. If $D<0$, the infinite straight trajectories miss. Otherwise the earliest root must additionally lie in $[0,h]$ and produce a closing rather than separating contact.

The textbook quadratic root $(-b-\sqrt D)/(2a)$ is not always a numerically safe expression. When $-b$ and $\sqrt D$ are almost equal, subtraction can destroy significant digits. In the approaching branch $b<0$ define $q=-\tfrac12(b-\sqrt D)$, which adds two positive magnitudes instead of subtracting close ones. The two algebraic roots are $c/q$ and $q/a$, because their product is $c/a$ and their sum is $-b/a$. For separated approaching disks, $c/q$ is the entry root. This is a concrete instance of *algebraic reformulation for numerical stability*: mathematically equivalent formulas are not computationally equivalent with floating-point rounding.

A worked collision is a stationary radius-1 disk at $(0,0)$ and a radius-1 disk starting at $(10,0)$ with velocity $(-100,0)$ m/s. We obtain $a=10000$, $b=-2000$, $c=96$, and $D=4{,}000{,}000-3{,}840{,}000=160{,}000$. The entry time is $(2000-400)/20000=0.08$ seconds. A frame of length $h=0.10$ seconds must be split into 0.08 seconds before impact and 0.02 seconds after the response; otherwise even a correct impulse equation is applied at the wrong time.


#### Executable first-contact quadratic

The derivation can become a complete C++20 program without inventing missing algorithmic decisions. The following stand-alone driver checks the input domain, identifies the approaching branch, uses the numerically safer quadratic root and tests the 0.08-second impact. It deliberately rejects pre-existing overlaps; the event solver needs a separate policy for those.

```cpp
// Mathematics continuous-note C++20 first-contact oracle; code and tested source stay identical.
#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
struct Vec2 { double x{}, y{}; };
struct Disk2 { Vec2 centre{}, velocity{}; double radius{}; };
double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }

std::optional<double> firstTouch(Disk2 a, Disk2 b, double h) {
    const auto valid = [](Disk2 d) {
        return std::isfinite(d.centre.x)&&std::isfinite(d.centre.y)&&
               std::isfinite(d.velocity.x)&&std::isfinite(d.velocity.y)&&
               std::isfinite(d.radius)&&d.radius>0;
    };
    if (!valid(a)||!valid(b)||!std::isfinite(h)||h<0)
        throw std::invalid_argument("invalid disk or horizon");
    Vec2 p{b.centre.x-a.centre.x,b.centre.y-a.centre.y};
    Vec2 v{b.velocity.x-a.velocity.x,b.velocity.y-a.velocity.y};
    const double R=a.radius+b.radius;
    const double aa=dot(v,v),bb=2*dot(p,v),cc=dot(p,p)-R*R;
    if (!std::isfinite(aa)||!std::isfinite(bb)||!std::isfinite(cc))
        throw std::overflow_error("nonfinite collision coefficients");
    if (cc<0) throw std::domain_error("existing overlap");
    if (cc==0) return 0.0; // Touching, not automatically a new impact.
    if (aa==0||bb>=0) return std::nullopt;
    const double discriminant=std::fma(bb,bb,-4*aa*cc);
    if (!std::isfinite(discriminant))
        throw std::overflow_error("discriminant overflow");
    if (discriminant<0) return std::nullopt;
    const double q=-0.5*(bb-std::sqrt(discriminant));
    if (!(q>0)) return std::nullopt;
    const double t=cc/q;
    if (!std::isfinite(t)||t<0||t>h) return std::nullopt;
    return t;
}
int main() {
    auto near=[](double actual,double expected) {
        if (!std::isfinite(actual)||std::abs(actual-expected)>1e-10)
            throw std::runtime_error("unexpected first-contact time");
    };
    const Disk2 fixed{{0,0},{0,0},1};
    const Disk2 moving{{10,0},{-100,0},1};
    const auto impact=firstTouch(fixed,moving,0.1);
    if (!impact) throw std::runtime_error("fast contact was missed");
    near(*impact,0.08);
    if (firstTouch(fixed,moving,0.05))
        throw std::runtime_error("collision outside horizon was accepted");
    if (firstTouch(fixed,{{10,0},{100,0},1},1.0))
        throw std::runtime_error("separating disks reported impact");
    if (firstTouch(fixed,{{10,0},{0,0},1},1.0))
        throw std::runtime_error("static separated disks reported impact");
    if (firstTouch(fixed,{{10,2.1},{-10,0},1},2.0))
        throw std::runtime_error("geometric near miss reported impact");
    const auto tangent=firstTouch(fixed,{{10,2},{-10,0},1},2.0);
    if (!tangent) throw std::runtime_error("tangency not found");
    near(*tangent,1.0);
    const auto galilean=firstTouch({{0,0},{55,7},1},
                                   {{10,0},{-45,7},1},0.1);
    if (!galilean) throw std::runtime_error("relative motion invariance failed");
    near(*galilean,0.08);
    bool rejected=false;
    try { (void)firstTouch(fixed,{{1,0},{0,0},1},1); }
    catch(const std::domain_error&) { rejected=true; }
    if (!rejected) throw std::runtime_error("initial overlap not rejected");
    rejected=false;
    try { (void)firstTouch(fixed,moving,-1); }
    catch(const std::invalid_argument&) { rejected=true; }
    if (!rejected) throw std::runtime_error("negative time accepted");
    std::cout<<"math first contact: entry, no-hit, tangency, invariance and validation PASS\n";
}
```

Reconstruct the coefficient trace from the code, not merely from its expected output. The expanded `main` now executes the no-motion, no-hit, shorter-horizon, grazing, relative-motion-invariance, initial-overlap and invalid-timestep branches in addition to the 0.08-second impact. Change one assumption at a time and explain why the branch and returned state must change. The exact program is also built as [the first-contact Math laboratory](../../tools/labs/math_contact_toi_lab.cpp), so its source and this embedded code can be checked for byte agreement in CI. The zero-separation branch is a stated laboratory policy, not a sufficient general resting-contact solver. This small program does not certify every floating-point exponent range; the full event laboratory adds a scale-sensitive post-event contact policy.

A contact normal at a non-coincident surface point is the gradient direction of the distance function with respect to the relative centre:

$
\mathbf n=\nabla_{\mathbf p}\|\mathbf p\|
=\frac{\mathbf p}{\|\mathbf p\|}.
$

The derivative of gap with respect to time follows the chain rule:

$
\dot g(t)=\mathbf n(t)\cdot\mathbf v.
$

That scalar is the **normal relative velocity**: negative for closing, positive for opening, zero for instantaneous grazing. The derivative is undefined when centres coincide; returning a deterministic fallback normal is an API policy, not a mathematically unique surface normal. This distinction also explains why a root with zero normal velocity is a geometric tangency but need not require an impulse.

For a planar rigid body whose contact point is offset from its centre by $\mathbf r$, velocity at that point is $\mathbf v_{\text{point}}=\mathbf v_{\text{centre}}+\omega(-r_y,r_x)$. Thus the normal relative velocity is $\mathbf n\cdot[(\mathbf v_B+\omega_B\times\mathbf r_B)-(\mathbf v_A+\omega_A\times\mathbf r_A)]$. An impulse $\lambda\mathbf n$ acts positively on B and negatively on A. Applying the linear impulse-momentum law and the planar angular moment law gives a change in normal speed of $k_n\lambda$, with

$
k_n=m_A^{-1}+m_B^{-1}
+I_A^{-1}(\mathbf r_A\times\mathbf n)^2
+I_B^{-1}(\mathbf r_B\times\mathbf n)^2.
$

The symbols $m^{-1}$ and $I^{-1}$ are inverse mass and inverse moment of inertia, not inverted coordinates; set them to zero for a body constrained to remain static. The scalar $k_n$ is the one-row version of $\mathbf J\mathbf M^{-1}\mathbf J^\mathsf T$, where $\mathbf J$ maps generalized velocities to the contact's relative normal velocity and $\mathbf M$ is the generalized mass matrix. This expression makes the rotational terms unavoidable: off-centre impulses change the contact speed through angular motion. A normal impulse cannot attract two nonadhesive bodies, so its accumulated multiplier obeys the projection

$
\lambda_{\mathrm{new}}
=\max\left(0,\lambda_{\mathrm{old}}+
\frac{b_{\mathrm{target}}-u_n}{k_n}\right).
$

Here $u_n$ is current relative normal speed and $b_{\mathrm{target}}$ is the frozen outgoing speed from the beginning of the impact. Only $\Delta\lambda=\lambda_{\mathrm{new}}-\lambda_{\mathrm{old}}$ may be applied to velocities. Re-applying the full stored impulse every iteration repeatedly counts previous impulses. When $k_n=0$, the row is unresponsive and cannot be divided by. With friction, an independent tangent $\mathbf t=(-n_y,n_x)$ gives $k_t$ by replacing $\mathbf n$ with $\mathbf t$; the accumulated tangent multiplier is clamped into $[-\mu\lambda_n,\mu\lambda_n]$. The **sign of its scalar** depends on the chosen tangent basis, not the intuitive world direction.

The gap $g$ and squared polynomial $F$ make different *error scales* visible. If a computed contact is near the surface, a tiny negative $F$ does not necessarily imply meaningful penetration; subtraction of squared large values can amplify relative error. However, a fixed tolerance in metres is also wrong at large translated coordinates: subtracting world-space centres at magnitude $10^{12}$ can lose tiny separation detail before the quadratic is evaluated. An implementation must state a local length-error budget, its world-coordinate scale assumptions, and the policy for rejecting genuinely deep overlaps. The independently tested disk event laboratory first failed randomized collision checks because its squared-gap tolerance misclassified roundoff after impact; the corrected reference compares a linear surface gap using a coordinate-scaled tolerance. This still does **not** certify correctness at arbitrary exponent ranges or replace robust interval predicates.

A conservative broad phase must never filter out a pair which touches at any instant of the candidate interval. For a disk whose centre follows $\mathbf x(t)=\mathbf x_0+t\mathbf v$, each x coordinate remains between $x_0$ and $x_0+hv_x$. Expand both endpoint extrema by the disk radius; repeat in y. Every occupied point over the interval lies in that swept axis-aligned rectangle, so overlapping disks imply overlapping swept rectangles. The converse fails: rectangles may overlap even if the bodies pass at different times. This provides both a formal one-way implication and an implementable oracle test: every pair returned by the exact time-of-impact solver must be present in the broad-phase candidates, while extra candidates are permitted.

To verify an implementation, do not stop at several example pictures. Test invariance under adding a common velocity; symmetry under swapping the body labels with consistently flipped normal; preserved linear momentum for isolated frictionless impacts; kinetic-energy conservation only for ideal isolated elastic impacts; non-increasing energy for inelastic impacts; and no false negatives in candidate filtering. Include a seeded ensemble of unequal-mass impacts, replay the failing seed, inspect the coefficients and intermediate roots, and separate a finite-regression pass from a formal proof. The runnable witnesses in the repository are \`tools/labs/disk_ccd_lab.cpp\`, \`disk_event_pipeline_lab.cpp\`, \`disk_event_property_lab.cpp\`, and \`swept_broadphase_lab.cpp\`; their actual GitHub source and local test state must be checked before citing them as backend evidence.

**Reconstruction check.** Starting from the definition of a signed surface gap, derive the quadratic coefficients and their physical units. Then derive $\mathbf n=\nabla g$, obtain the Jacobian row and inverse effective mass, and explain why the contact multiplier is projected. Implement an independent TOI function using a stable quadratic formulation and verify the 0.08-second collision. Finally implement swept bounds and prove the one-way implication from collision to candidate overlap. A correct answer also identifies what cannot be established by the current tests: rotating convex shapes, acceleration during a step, persistent stacks, certified floating-point predicates, and native Kairo integration.
<!-- END IMPLEMENTABLE CONTACT MATHEMATICS 2026-11-12 -->

<!-- begin V10.23 -->
## Test numerical methods and benchmark reproducibly


Numerical algorithms should be evaluated using experiments that others can reproduce and independently inspect.

For deterministic solvers, we can often compare an output against a known mathematical reference.

For stochastic estimators, one run is not enough to establish expected performance.

A complete benchmark must specify the numerical method, workload, randomization, sampling budget, error metric, and statistical comparison procedure.

#### Separate sample-level and run-level observations

Suppose a Monte Carlo method uses:

$$ N=1000 $$

samples to produce one estimate:

$$ \widehat I^{(1)}. $$

We repeat the entire experiment using a different seed to obtain:

$$ \widehat I^{(2)}. $$

Continue for $R$ independent runs:

$$ \widehat I^{(1)},\ldots,\widehat I^{(R)}. $$

The between-run mean is:

$$ \bar I_R= \frac1R\sum_{r=1}^{R}\widehat I^{(r)}. $$

The between-run variance is:

$$ s_R^2= \frac1{R-1} \sum_r(\widehat I^{(r)}-\bar I_R)^2. $$

This measures variation among complete independent randomized experiments.

It is not the same as the variance of individual contributions within one run.

### 34.2 A paired benchmark

Suppose we compare methods A and B on the same scenes and corresponding replicate identifiers.

For replicate $r$, calculate:

$$ D_r=E_{A,r}-E_{B,r}. $$

Then analyze:

$$ \bar D= \frac1R\sum_rD_r. $$

The sample variance of the differences is:

$$ s_D^2= \frac1{R-1}\sum_r(D_r-\bar D)^2. $$

The estimated standard error is:

$$ \boxed{ \operatorname{SE}(\bar D) = \frac{s_D}{\sqrt R} } $$

when the replicate differences satisfy the independence assumptions relevant to the analysis.

The pairing may reduce variance by controlling common scene or seed effects.

But the statistical unit must be chosen carefully. Pixels within one rendered image are spatially dependent, and treating every pixel as an independent benchmark replicate can severely overstate the amount of independent evidence.

### 34.3 What to record

A reproducible numerical benchmark should record the exact source revision, compiler and optimization flags, build mode, hardware, floating-point configuration, thread count, random generator algorithm, seed or stream derivation, number of samples, mathematical test case, reference-generation method, stopping criteria, and error metric.

It should also record whether an output was measured before or after warm-up, whether caches were populated, and whether CPU/GPU synchronization costs were included.

These details can materially affect results.

### 34.4 Performance versus sample count

A method can reduce variance per sample but require more computation per sample.

Suppose method A has:

$$ \operatorname{Var}(Y_A)=1 $$

and costs one unit of time per sample.

Method B has:

$$ \operatorname{Var}(Y_B)=0.25 $$

but costs four units of time per sample.

At equal sample count, B has one-quarter the variance.

At equal time budget $T$, however:

$$ N_A=T, \qquad N_B=\frac T4. $$

The estimator variances are:

$$ \operatorname{Var}(\hat I_A)=\frac1T, $$

$$ \operatorname{Var}(\hat I_B) = \frac{0.25}{T/4} =\frac1T. $$

Thus the methods have equal variance at equal computational budget in this simplified model.

A proper benchmark should report both sample efficiency and time efficiency.


This final atomic part is the practical culmination of Phases 9 and 10.

A numerical test should identify what kind of failure it is intended to detect.

If a result differs from the expected answer, we want to determine whether the cause is truncation, discretization, roundoff, statistical variance, incorrect mathematical assumptions, or a programming defect.

The correct test design uses multiple independent forms of evidence.

#### Exact analytical test cases

Choose problems with known exact answers.

For example:

$$ \int_0^1x^2\,dx=\frac13. $$

For root finding:

$$ x^2-2=0 $$

has roots:

$$ \pm\sqrt2. $$

For vector norm:

$$ \|(3,4)\|_2=5. $$

For probability normalization:

$$ \sum_ip_i=1. $$

These provide reference values that are independent of the implementation under test.

However, exact-answer tests should not rely on a second implementation that secretly uses the same defective algorithm.

Independent formulations are preferable.

#### Property-based tests

A property test checks mathematical invariants over many inputs.

For linear interpolation:

$$ \operatorname{lerp}(a,b,0)=a, $$

$$ \operatorname{lerp}(a,b,1)=b. $$

For bilinear interpolation, the four corners should reproduce the four input values.

For a covariance matrix:

$$ \Sigma^T=\Sigma, $$

and:

$$ \mathbf v^T\Sigma\mathbf v\ge0 $$

for tested vectors $\mathbf v$, allowing a suitable numerical tolerance.

For normalized probabilities:

$$ p_i\ge0, $$

$$ \sum_ip_i\approx1. $$

For a bisection solver, the maintained bracket should continue to contain a sign change under the assumed continuous-function problem.

Property tests often catch mistakes that are not obvious from a small set of hand-chosen output values.

#### Metamorphic testing

A metamorphic test transforms an input in a way that predicts how the output should transform.

For example, scaling all nonnegative sampling weights by a positive constant should not change their normalized probabilities:

$$ \frac{cw_i}{\sum_jcw_j} = \frac{w_i}{\sum_jw_j}. $$

Thus:

$$ \boxed{ \operatorname{normalize}(c\mathbf w) = \operatorname{normalize}(\mathbf w) } $$

for suitable positive $c$ and finite arithmetic conditions.

For a vector norm:

$$ \boxed{ \|c\mathbf x\|=|c|\|\mathbf x\| }. $$

For a linear interpolation function:

$$ \boxed{ \operatorname{lerp}(a+c,b+c,t) = \operatorname{lerp}(a,b,t)+c } $$

in exact arithmetic.

Floating-point implementations may satisfy these only approximately, but their deviations are useful diagnostics.

#### Convergence-order tests

Suppose an algorithm claims second-order accuracy.

Run it with step sizes:

$$ h,\qquad h/2,\qquad h/4. $$

Measure error against a trusted reference.

Estimate:

$$ p_{\mathrm{observed}} = \log_2 \left( \frac{E(h)}{E(h/2)} \right). $$

If it consistently approaches two in the asymptotic regime, that supports the claimed order.

If it approaches one instead, the implementation may contain a first-order component or the theory may not apply to the chosen test.

If the error stops decreasing as $h$ becomes very small, roundoff, solver tolerance, or reference error may dominate.

This is more informative than testing only one resolution.

#### Cross-precision tests

Run the same controlled problem in `float` and `double`.

If the error changes dramatically, floating-point precision may be important.

But remember that improved results in `double` do not prove the algorithm is stable.

A stronger evaluation compares multiple formulations and precisions.

For example, compare naive and stable quadratic solvers using the same coefficient types.

If the stable formulation recovers the small root while the naive formula returns zero, the experiment isolates the cancellation mechanism.

#### Statistical tests

For randomized algorithms, use independent repeated trials to test expected quantities.

Suppose a sampler is intended to generate:

$$ X\sim U(0,1). $$

Its theoretical moments are:

$$ \mathbb E[X]=0.5, $$

$$ \operatorname{Var}(X)=\frac1{12}. $$

Generate many samples and estimate the moments.

The observed mean should be close to 0.5 within an appropriate statistical tolerance.

But requiring an exact mean of 0.5 would be wrong.

Likewise, an observed moment falling inside a tolerance band does not prove the generator is high quality in all dimensions.

Additional tests should examine correlation, multidimensional structure, and application-level behavior.

#### Deterministic tests of randomized code

Even a stochastic algorithm can contain deterministic properties.

For example, a Latin-hypercube sampler with $N$ samples should visit every one-dimensional stratum exactly once.

That can be tested without relying on approximate statistics.

A stateless sampler should return the same value for the same complete input tuple.

A sample-without-replacement procedure should never repeat an element within one sample set.

A PDF transformation should satisfy known Jacobian relations.

These are strong unit-test targets because they do not require thousands of random trials to detect a deterministic implementation defect.

### Reference code excerpts and source-reported validation

> The supplied source describes a standalone C++23 laboratory as verified and refers to a downloadable `.cpp` file. The scratch inputs contain code excerpts and compile commands, but not that complete standalone file. Its claimed build and run results below are source-reported and were not reproduced in this staging pass.

The supplied source reports a C++23 laboratory covering stable sums, norms, and quadratic roots; interpolation; root finding; convergence measurement; probability normalization; sampling; and online moments. The command and snippets are retained as source material. The complete laboratory file is absent from the provided inputs.

Compile:

```
c++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror \
  phase10_m059_m060_numerical_reference.cpp \
  -o phase10_numerical_lab

./phase10_numerical_lab
```

The supplied source also reports compiling and executing the laboratory with AddressSanitizer and UndefinedBehaviorSanitizer; those results were not independently reproduced here.

### Results reported by the supplied source

> These execution results are copied from the supplied source. The scratch package contains no `int main` reference laboratory, so these claims were not reproduced in this staging pass.

Reported numerical results

| Test                                       | Observed result                |
| ------------------------------------------ | ------------------------------ |
| Exact cancellation sum                     | 2                              |
| Naive sequential sum                       | 0                              |
| Kahan compensated sum                      | 2                              |
| Neumaier compensated sum                   | 2                              |
| Pairwise sum, chosen ordering              | 0                              |
| Stable norm of $(10^{200},10^{200})$   | $1.41421356\times10^{200}$ |
| Stable small quadratic root                | $-10^{-8}$                 |
| Bisection for $\sqrt2$                 | 40 iterations                  |
| Secant for $\sqrt2$                    | 6 iterations                   |
| Newton for $\sqrt2$                    | 4 iterations                   |
| Euler observed order                       | 0.969610                       |
| Richardson-refined Euler value             | 2.718133087                    |
| Exact exponential value $e$            | 2.718281828                    |
| Log-sum-exp of $(1000,1001,1000)$      | 1001.551444714                 |
| Stratified estimate of $\int_0^1x^2dx$ | 0.333339835                    |
| Exact integral                             | 0.333333333                    |

All reference assertions passed, including root accuracy, interpolation invariants, normalization, LHS stratum coverage, Halton coordinates, and Welford statistics.

The summation example demonstrates an important limitation: a pairwise reduction can be much more accurate in many cases yet still fail on a particular adversarial input ordering.

No single summation strategy should be declared universally exact.

Likewise, the bisection, secant, and Newton iteration counts are results for the chosen smooth test function and stopping tolerances, not universal performance rankings.

The Euler observed order of approximately 0.97 is consistent with first-order convergence over the tested finite resolutions. The limit tends toward order one as the asymptotic regime is approached, provided roundoff and other errors do not dominate.


<!-- end V10.23 -->


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|Next chapter]]
