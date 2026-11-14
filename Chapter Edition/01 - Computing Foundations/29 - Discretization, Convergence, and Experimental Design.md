# 29 — Discretization, Convergence, and Experimental Design

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|Next chapter]]

Finite grids and samples change a continuous problem; truncation and floating-point arithmetic add separate errors. This chapter teaches convergence checks and experiment design so an accuracy claim is connected to the actual image or simulation budget.

**By the end:** Separate discretization, truncation, and roundoff, and design experiments that measure the error relevant to the application.

**Recall question:** How would you tell whether an observed error comes from grid resolution, numerical arithmetic, or an insufficient sample budget?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 69720–72217; source-span SHA256 5904cbf0b13e2b15146df64601fd0d11f88a7e53b874180b6121aebb2d5fc5ff. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Separate discretization, truncation, and roundoff; measure convergence


Many mathematical models are continuous.

A computer often represents them using finite grids, meshes, time steps, or sampled values.

That replacement introduces discretization error.

Suppose the continuous problem is:

$$ \mathcal L(u)=f, $$

where $\mathcal L$ is a differential operator.

A numerical scheme replaces it with:

$$ \mathcal L_h(u_h)=f_h, $$

where $h$ represents a grid-spacing or discretization scale.

The discrete solution $u_h$ generally differs from the exact continuous solution $u$.

### 24.1 Spatial discretization example

Consider the derivative:

$$ f'(x). $$

Approximate it using a central difference:

$$ D_hf(x)= \frac{f(x+h)-f(x-h)}{2h}. $$

For a sufficiently smooth function, Taylor expansion gives:

$$ f(x+h)= f(x)+hf'(x)+\frac{h^2}{2}f''(x) +\frac{h^3}{6}f'''(x)+O(h^4), $$

$$ f(x-h)= f(x)-hf'(x)+\frac{h^2}{2}f''(x) -\frac{h^3}{6}f'''(x)+O(h^4). $$

Subtract:

$$ f(x+h)-f(x-h) = 2hf'(x)+\frac{h^3}{3}f'''(x)+O(h^5) $$

under sufficient higher-order smoothness.

Divide by $2h$:

$$ D_hf(x)= f'(x)+\frac{h^2}{6}f'''(x)+O(h^4). $$

Therefore:

$$ \boxed{ D_hf(x)-f'(x)=O(h^2) }. $$

This is second-order discretization accuracy for the central-difference derivative.

### 24.2 Grid resolution in a TSDF

Suppose a signed-distance field is stored on a voxel grid with spacing $h$.

A smaller $h$ can represent finer geometric detail.

But decreasing $h$ also increases the number of grid cells.

In three dimensions, halving the voxel spacing across a fixed bounding volume increases the number of cells by roughly:

$$ 2^3=8. $$

The memory and computational costs can therefore grow rapidly.

A practical reconstruction system balances geometric discretization error against memory and runtime.

Sparse data structures, adaptive grids, and local refinement attempt to allocate resolution where it matters.

### 24.3 Discretization can change topology

A continuous implicit surface may contain thin structures, small holes, or nearby disconnected components.

A coarse grid may fail to resolve them.

Increasing numerical precision from `float` to `double` does not recover geometry that the grid never represented.

This is a discretization limitation, not primarily a rounding problem.

It matters in collision detection, signed-distance meshing, finite-element simulation, and 3D reconstruction.

### 24.4 Consistency, stability, and convergence

For many numerical methods solving differential equations, three concepts appear together.

Consistency means the discrete scheme approximates the continuous equation as resolution increases, in a suitably defined local sense.

Stability means errors do not grow uncontrollably under the scheme and chosen norms or time-step conditions.

Convergence means the discrete solution approaches the continuous solution as the discretization is refined.

For appropriately formulated linear, well-posed initial-value problems, the Lax equivalence theorem connects consistency and stability to convergence.

But that theorem should not be applied indiscriminately to nonlinear reconstruction pipelines, arbitrary discretizations, or stochastic systems.

Its assumptions matter.


Truncation error arises when an infinite mathematical representation or exact relation is replaced by a finite approximation.

A common example is truncating a Taylor series.

For a smooth function:

$$ f(x+h) = f(x)+hf'(x)+\frac{h^2}{2}f''(x) +\frac{h^3}{6}f'''(x)+\cdots. $$

If we retain only:

$$ f(x)+hf'(x), $$

the omitted terms form the truncation error.

### 25.1 Euler integration

For the ODE:

$$ y'(t)=f(t,y(t)), $$

the exact Taylor expansion is:

$$ y(t+h) = y(t)+hy'(t)+\frac{h^2}{2}y''(t)+O(h^3). $$

Euler's method uses:

$$ y_{n+1}=y_n+hf(t_n,y_n). $$

When started from the exact current state, its one-step defect is:

$$ \boxed{ \text{local one-step error}=O(h^2) }. $$

Under the usual smoothness and stability assumptions over a fixed finite time interval, the accumulated global error is:

$$ \boxed{ \text{global error}=O(h) }. $$

Why does the order decrease?

There are approximately:

$$ N=\frac Th $$

steps over a fixed interval of duration $T$.

Each local error is of order $h^2$.

A rough accumulation picture gives:

$$ N\,O(h^2) = \frac Th O(h^2) = O(h). $$

That is intuition rather than a complete proof; a rigorous global-error bound also accounts for how errors propagate through subsequent updates.

### 25.2 Difference between truncation and discretization

The terms overlap in many numerical-analysis contexts.

Discretization error broadly describes the difference introduced by replacing a continuous problem with a discrete one.

Truncation error often describes the local defect caused by retaining finitely many terms or using an approximate discrete relation.

For differential-equation solvers, local truncation error has a specific meaning tied to inserting the exact solution into the numerical scheme.

Do not treat the terms as universally disjoint categories.

### 25.3 Truncating a light-transport series

Suppose total radiance can be represented conceptually as a sum over path lengths:

$$ L=L_0+L_1+L_2+\cdots. $$

If a renderer deterministically stops after depth $K$, it computes:

$$ L^{(K)}=\sum_{k=0}^{K}L_k. $$

The omitted remainder is:

$$ \boxed{ R_K=\sum_{k=K+1}^{\infty}L_k }. $$

If the remainder is nonzero, deterministic truncation introduces bias relative to the full model.

Russian roulette can avoid that particular truncation bias under its correct weighting assumptions, at the cost of stochastic variance.

Thus deterministic truncation error and Monte Carlo variance are different mechanisms.


Roundoff error comes from representing and computing with finitely many floating-point values.

The central distinction is:

- Truncation error often decreases as resolution increases or a local approximation includes more terms.
- Roundoff error can increase when a calculation involves more operations, very small differences, or badly scaled intermediate values.

This creates situations in which increasing nominal numerical resolution eventually makes the result worse.

### 26.1 Finite differences reveal the trade-off

Consider forward differentiation:

$$ D_hf(x)= \frac{f(x+h)-f(x)}{h}. $$

Taylor expansion gives truncation error:

$$ \boxed{ E_{\mathrm{trunc}}\approx C_1h }. $$

But floating-point evaluations of $f(x+h)$ and $f(x)$ have numerical errors.

After subtracting and dividing by $h$, a simplified roundoff model is:

$$ \boxed{ E_{\mathrm{round}}\approx C_2\frac uh }. $$

Therefore total error behaves approximately like:

$$ \boxed{ E(h)\approx C_1h+C_2\frac uh }. $$

This is the crucial trade-off.

Reducing $h$ decreases truncation error but amplifies roundoff error.

### 26.2 Derive an optimal step scale

Differentiate the approximate error model:

$$ E'(h)=C_1-C_2\frac u{h^2}. $$

Set to zero:

$$ C_1-C_2\frac u{h^2}=0. $$

Therefore:

$$ h^2=\frac{C_2u}{C_1}. $$

Thus:

$$ \boxed{ h_{\mathrm{optimal}} \sim\sqrt u } $$

up to problem-dependent scales and constants.

For binary64:

$$ u\approx10^{-16}, $$

so:

$$ \sqrt u\approx10^{-8}. $$

That explains why an extremely tiny finite-difference step is not necessarily a good choice.

### 26.3 Central differences

For central differentiation:

$$ D_hf(x)= \frac{f(x+h)-f(x-h)}{2h}, $$

the truncation error is:

$$ O(h^2). $$

A simplified total-error model is:

$$ \boxed{ E(h)\approx C_1h^2+C_2\frac uh }. $$

Differentiate:

$$ E'(h)=2C_1h-C_2\frac u{h^2}. $$

Set to zero:

$$ 2C_1h^3=C_2u. $$

Therefore:

$$ \boxed{ h_{\mathrm{optimal}}\sim u^{1/3} } $$

up to problem-dependent scales.

For binary64, this is roughly $10^{-5}$ to $10^{-6}$, depending on the chosen constants and local function scales.

### 26.4 More precision is not a substitute for a better algorithm

If the problem is badly conditioned or the numerical method is unstable, simply increasing precision may postpone failure rather than address the underlying cause.

Conversely, using a stable formulation in binary32 can sometimes be preferable to using an unstable formulation in binary64.

The right choice depends on the error budget.


A numerical method has order $p$ if its error behaves asymptotically as:

$$ \boxed{ E(h)=Ch^p+o(h^p) } $$

for a nonzero leading coefficient $C$, under a specified limit such as $h\to0$.

The exponent $p$ is the convergence order.

If:

$$ p=1, $$

the method is first-order.

If:

$$ p=2, $$

it is second-order.

If:

$$ p=4, $$

it is fourth-order.

### 27.1 Step-size refinement

Suppose:

$$ E(h)\approx Ch^p. $$

Halve the step size:

$$ E(h/2)\approx C(h/2)^p. $$

Therefore:

$$ \frac{E(h)}{E(h/2)} \approx \frac{Ch^p}{Ch^p/2^p} =2^p. $$

Thus:

$$ \boxed{ p\approx \log_2\left( \frac{E(h)}{E(h/2)} \right) }. $$

### 27.2 Numerical example

Suppose an algorithm has errors:

| Step size | Error  |
| --------- | ------ |
| $h$   | 0.04   |
| $h/2$ | 0.01   |
| $h/4$ | 0.0025 |

The error decreases by a factor of four when the step size halves.

Therefore:

$$ 2^p=4. $$

Thus:

$$ \boxed{p=2}. $$

This is consistent with second-order convergence.

### 27.3 Estimating order without knowing the exact solution

Often the exact solution is unknown.

Suppose we compute:

$$ Q_h,\quad Q_{h/2},\quad Q_{h/4} $$

and assume:

$$ Q_h=Q^\star+Ch^p+o(h^p). $$

Then:

$$ Q_h-Q_{h/2} \approx Ch^p(1-2^{-p}). $$

And:

$$ Q_{h/2}-Q_{h/4} \approx Ch^p2^{-p}(1-2^{-p}). $$

Take the ratio:

$$ \frac{Q_h-Q_{h/2}} {Q_{h/2}-Q_{h/4}} \approx 2^p. $$

Thus:

$$ \boxed{ p_{\mathrm{observed}} \approx \log_2\left| \frac{Q_h-Q_{h/2}} {Q_{h/2}-Q_{h/4}} \right| }. $$

This is a common experimental order-of-convergence estimate.

### 27.4 Richardson extrapolation

If $p$ is known, use:

$$ Q_h=Q^\star+Ch^p+O(h^{p+1}) $$

and:

$$ Q_{h/2}=Q^\star+C(h/2)^p+O(h^{p+1}). $$

Multiply the second by $2^p$:

$$ 2^pQ_{h/2} = 2^pQ^\star+Ch^p+O(h^{p+1}). $$

Subtract $Q_h$:

$$ 2^pQ_{h/2}-Q_h = (2^p-1)Q^\star+O(h^{p+1}). $$

Therefore:

$$ \boxed{ Q_{\mathrm{Richardson}} = \frac{2^pQ_{h/2}-Q_h}{2^p-1} } $$

cancels the leading $Ch^p$ error term.

This can improve accuracy when the asymptotic error expansion is valid.

But if the results are dominated by floating-point roundoff, stochastic sampling noise, or discontinuities, Richardson extrapolation may behave poorly.

### 27.5 Root-finding convergence order versus discretization order

Be careful: the word order is used in several related contexts.

For numerical integration, we examine error as step size $h$ decreases.

For root-finding, we often examine:

$$ e_{n+1}\approx C|e_n|^p $$

as iterations progress.

Thus Newton's quadratic root convergence and a second-order finite-difference scheme are both described as order two, but their independent variables and convergence mechanisms differ.

Always identify which limit is being studied.



<!-- end V10.20 -->


# Samples become estimates with defensible uncertainty

Random generators and transformations produce samples; weighted estimates turn them into measurements. Stable statistics, inference, probability normalization and sampling design each need their own assumptions and checks.

<!-- begin V10.07 -->
## 6. A computer produces deterministic pseudorandom sequences

The previous lesson established the mathematics of uncertainty: probability distributions, expected values, variance, covariance, conditioning, and Gaussian random variables.

Now we need to answer a different set of questions.

How does a deterministic computer produce samples that behave like draws from a probability distribution? How do we transform those samples into ray directions, waiting times, or other physically meaningful quantities? How do we estimate an integral when evaluating it exactly is impossible? How do we know whether the estimate is correct, and how much uncertainty remains?

Most importantly, how do we distinguish statistical noise, estimator bias, numerical error, and actual implementation bugs?

These questions form the foundation of production Monte Carlo rendering, randomized numerical algorithms, stochastic simulation, and rigorous experimental evaluation.

We will develop sampling ideas around a recurring graphics problem: estimating a light contribution through sampled ray directions. The supplied excerpts refer to KairoRayTracer/Sampler.cppm, KairoMath/Probability.cppm, and KairoMath/Statistics.cppm. No repository checkout or pinned revision was available for this audit, so this volume does not claim to inspect or characterize those files.

The light-contribution example returns throughout the volume as a concrete use of sampling and probability.

### Sampling a uniform random variable

The starting point of many randomized algorithms is a random variable:

$$
 U\sim\mathcal U(0,1). 
$$

Its probability density is:

$$
 p_U(u)= \begin{cases} 1,&0\le u\le1,\\ 0,&\text{otherwise}, \end{cases} 
$$

where endpoint conventions do not affect this ideal continuous distribution.

Its CDF is:

$$
 F_U(u)=u,\qquad 0\le u\le1. 
$$

This distribution is important not because all physical quantities are uniform, but because a source of uniform random numbers can be transformed into many other distributions.

However, a computer does not store arbitrary real numbers. An ordinary binary floating-point representation contains finitely many representable values.

A practical uniform generator produces a discrete approximation to an ideal continuous uniform distribution.

#### 1 Deriving a uniform floating-point sample from random bits

Suppose we obtain an unsigned 32-bit integer $R$ uniformly distributed over:

$$
 \{0,1,\ldots,2^{32}-1\}. 
$$

Define:

$$
 \boxed{U=\frac{R}{2^{32}}}. 
$$

Then:

$$
 0\le U<1. 
$$

Every attainable lattice value:

$$
 \frac{k}{2^{32}} 
$$

has probability:

$$
 \frac1{2^{32}}. 
$$

The generated variable is not mathematically continuous. It is uniform over a finite dyadic grid.

For a `double`, this conversion can preserve those 32 random bits exactly because binary64 has 53 bits of significand precision.

A higher-resolution approach can use 53 random bits:

$$
 \boxed{ U=\frac{R\_{53}}{2^{53}} } 
$$

where $R\_{53}$ is a uniformly distributed integer from zero through $2^{53}-1$.

This provides samples in the half-open interval $[0,1)$ on a grid compatible with binary64 precision.

#### 2 Why half-open intervals matter

The interval:

$$
 [0,1) 
$$

contains zero but not one.

That convention is particularly convenient for indexing and inverse-transform sampling.

For example, if we choose an integer bucket using:

$$
 i=\lfloor NU\rfloor, 
$$

then $i$ lies between zero and $N-1$ whenever $U\in[0,1)$.

If $U=1$ were permitted, the expression could produce $i=N$, an invalid array index.

But some transformations require excluding zero as well. The expression:

$$
 -\log U 
$$

is undefined at $U=0$.

Therefore different sampling transformations may require $[0,1)$, $(0,1)$, or carefully handled boundary conventions.

A sampler interface should document its exact floating-point endpoint behavior.

#### 3 Why modulo reduction can produce bias

Suppose a generator produces uniformly distributed integers:

$$
 R\in\{0,1,\ldots,7\}. 
$$

We want one of three outcomes:

$$
 0,1,2. 
$$

A naive approach is:

$$
 K=R\bmod3. 
$$

List the results:

| $R$       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| ------------- | - | - | - | - | - | - | - | - |
| $R\bmod3$ | 0 | 1 | 2 | 0 | 1 | 2 | 0 | 1 |

The resulting probabilities are:

$$
 P(K=0)=\frac38, 
$$

$$
 P(K=1)=\frac38, 
$$

$$
 P(K=2)=\frac28. 
$$

The distribution is not uniform.

This is modulo bias.

#### 4 Rejection removes modulo bias

We can instead reject $R=6$ and $R=7$, retaining only integers from zero through five.

Among these six retained integers, each residue modulo three occurs exactly twice.

Therefore:

$$
 P(K=0)=P(K=1)=P(K=2)=\frac13. 
$$

The cost is that some generated values are discarded, so the number of generator draws is variable.

For production bounded-integer sampling, implementations use carefully designed rejection or multiplication-based algorithms that avoid modulo bias while minimizing wasted work.

A compiler's standard random-distribution implementation may provide this functionality, but exact sampling sequences can depend on the implementation.

### Pseudorandom-number generators

A pseudorandom-number generator, or PRNG, is a deterministic algorithm designed to produce sequences with useful statistical properties.

At its simplest, it has an internal state $s_n$ and a state transition:

$$
 \boxed{s\_{n+1}=T(s_n)}. 
$$

An output function maps the state to a generated value:

$$
 \boxed{r_n=O(s_n)}. 
$$

Given the initial state and the transition/output algorithms, the sequence is determined.

Randomness in the probabilistic sense is modeled through the seed distribution, an unknown initial state, or the statistical behavior of outputs under repeated experiments.

#### 1 What makes a generator good?

There is no single universal measure of quality.

A generator may be evaluated for its period, output distribution, correlations, equidistribution properties, statistical test performance, speed, memory footprint, seeding behavior, stream separation, skip-ahead support, reproducibility, and suitability for parallel execution.

For rendering, we care about spatial and temporal artifacts, not merely whether the histogram of generated values looks uniform.

A sequence can have a nearly perfect marginal histogram while exhibiting correlations that cause structured image noise.

For cryptography, an additional requirement is computational unpredictability. Ordinary simulation-oriented PRNGs generally do not satisfy this requirement.

Statistical quality and cryptographic security are different properties.

#### 2 Period

A finite-state deterministic generator must eventually repeat a state if run indefinitely.

Once a state repeats, the subsequent sequence repeats.

The period is the length of that repeating cycle.

A period of $2^{32}$ means the generator can produce up to approximately 4.29 billion state transitions before repeating on a maximal-length cycle.

A longer period is useful, but it does not guarantee good statistical quality.

A generator with an enormous period can still produce highly correlated or otherwise unsuitable values.

#### 3 State size versus output size

A generator may maintain 64 bits of state and produce 32-bit outputs.

Another may maintain thousands of bits of state and produce 64-bit outputs.

The output width does not tell us the full amount of internal state or its period.

This matters in memory-intensive parallel applications. One PRNG state per ray, pixel, particle, or thread can become expensive if the generator has large state.

Stateless and counter-based approaches provide an alternative, which we will derive in the stateless PRNG implementation section.

### PRNG state and determinism

Suppose we run:

```cpp
std::mt19937_64 generator(12345);
```

Then draw a sequence of numbers.

Running the same engine with the same seed produces the same raw engine sequence according to the specified algorithm.

That is valuable for reproducible tests.

But a simulation's reproducibility requires more than fixing one seed.

#### 1 Why a fixed seed may not fix the complete application

Imagine two workers sharing one PRNG.

Worker A requests three numbers.

Worker B requests two numbers.

If their execution order changes, they consume the generator stream in a different order.

Even if the initial seed is unchanged, the random numbers assigned to each logical task may differ.

Similarly, suppose a path tracer consumes random numbers only when a ray hits a surface.

Changing an earlier branch can change how many numbers are consumed, shifting subsequent samples.

A stateful sequential generator couples sample identity to execution history.

This can make debugging difficult when scheduling, batching, or scene changes alter random-number consumption.

#### 2 State snapshots

For deterministic replay, a stateful generator can expose a serializable state.

We can store:

$$
 s_n 
$$

at a checkpoint and restore it later.

For a complete replay, we may also need to preserve the state of distribution algorithms that cache intermediate results, such as some normal-distribution samplers.

Saving only the seed is not equivalent to saving the state after an arbitrary number of draws.

#### 3 Reproducibility levels

It helps to distinguish three types of reproducibility.

Exact algorithmic reproducibility means identical logical inputs produce the same integer [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#6. A computer produces deterministic pseudorandom sequences|random stream]].

Numerical reproducibility means floating-point outputs agree within specified tolerances.

Bitwise reproducibility means the final binary results are identical.

The last is the strongest and can be affected by floating-point operation order, hardware instructions, math-library implementations, parallel reductions, and scheduling.

For research, the required level must be stated explicitly.

#### 4 A deterministic per-pixel random stream

One strategy assigns each pixel a stream derived from its coordinates and a global seed.

For example:

$$
 s\_{\mathrm{pixel}}= H(\text{seed},x,y), 
$$

where $H$ is a mixing function.

Each pixel owns its own PRNG state.

This reduces interference between pixels, but samples within one pixel can still depend on their execution order.

For fully indexable sampling, we can derive random values from explicit coordinates including sample index and dimension.

That leads to stateless generation.

### Linear congruential generators

A linear congruential generator, or LCG, uses:

$$
 \boxed{ x\_{n+1}=(ax_n+c)\bmod m }. 
$$

Here $a$ is the multiplier, $c$ is the increment, and $m$ is the modulus.

The state consists of an integer $x_n$.

This is among the simplest possible PRNG designs.

#### 1 Numerical trace

Consider the toy generator:

$$
 x\_{n+1}=(5x_n+1)\bmod16. 
$$

Let:

$$
 x_0=3. 
$$

Then:

$$
 x_1=(5\cdot3+1)\bmod16=16\bmod16=0. 
$$

Next:

$$
 x_2=(5\cdot0+1)\bmod16=1. 
$$

Then:

$$
 x_3=(5\cdot1+1)\bmod16=6, 
$$

$$
 x_4=(5\cdot6+1)\bmod16=31\bmod16=15, 
$$

$$
 x_5=(5\cdot15+1)\bmod16=76\bmod16=12. 
$$

The sequence begins:

$$
 3,\ 0,\ 1,\ 6,\ 15,\ 12,\ldots 
$$

Every value is completely determined by the previous state.

#### 2 Conditions for a full period

For a mixed LCG with positive modulus $m$, the Hull–Dobell theorem gives conditions under which the generator has full period $m$:

1. $c$ and $m$ are relatively prime.
2. $a-1$ is divisible by every prime factor of $m$.
3. If $m$ is divisible by four, then $a-1$ is divisible by four.

For our toy example:

$$
 m=16,\quad a=5,\quad c=1. 
$$

We have:

$$
 \gcd(1,16)=1. 
$$

The only prime factor of 16 is 2, and:

$$
 a-1=4 
$$

is divisible by 2 and 4.

Therefore the generator has full period 16.

#### 3 Why a full-period LCG can still be poor

Look at the least significant bit.

When $a$ and $c$ are odd:

$$
 x\_{n+1}\equiv x_n+1\pmod2. 
$$

Therefore the lowest bit alternates between zero and one.

Its period is only two, even if the complete state has a much longer period.

This demonstrates why taking low bits from a simple LCG can be dangerous.

More broadly, successive tuples produced by LCGs can lie on relatively small families of parallel hyperplanes in higher-dimensional spaces.

That lattice structure can become visible in multidimensional sampling.

#### 4 LCG code

```cpp
struct LCG32 {
    std::uint32_t state;

    explicit LCG32(std::uint32_t seed)
        : state(seed) {}

    std::uint32_t next() noexcept {
        // Arithmetic intentionally wraps modulo 2^32.
        // This is defined for unsigned integer types.
        state = state * 1664525u + 1013904223u;
        return state;
    }
};
```

This illustrates the mechanism. It is not presented as the preferred generator for a modern rendering engine.

The constants determine particular period and statistical characteristics. Using a known pair of constants does not automatically make the sequence suitable for high-quality path tracing.

### Mersenne Twister

The Mersenne Twister family was designed to produce long-period pseudorandom sequences with strong equidistribution properties for its intended applications.

The commonly used MT19937 has period:

$$
 \boxed{2^{19937}-1}. 
$$

The exponent is a Mersenne-prime exponent.

The standard 32-bit variant uses a large internal state based on 624 32-bit words.

C++ provides:

```cpp
std::mt19937
```

and the 64-bit variant:

```cpp
std::mt19937_64
```

#### 1 How it differs from an LCG

An LCG advances one integer using a multiply-add recurrence.

Mersenne Twister uses a much larger state and bitwise recurrence over a binary field, followed by output-tempering operations.

The resulting sequence is designed to have an enormous period and useful equidistribution properties.

Its large state is an advantage for some statistical uses but a disadvantage for applications that need thousands or millions of independent generator instances.

#### 2 The state footprint

MT19937's underlying 624-word state requires roughly:

$$
 624\cdot4=2496\text{ bytes} 
$$

just for the principal 32-bit state array, not counting implementation metadata.

If a renderer naively stores one such generator per million simultaneous ray states:

$$
 2496\times10^6\text{ bytes} 
$$

is approximately 2.5 GB of state data alone.

That is undesirable for many GPU and high-throughput workloads.

A smaller-state generator or counter-based strategy can be more appropriate.

#### 3 Mersenne Twister is not cryptographically secure

Despite its long period and statistical properties, Mersenne Twister is not designed to resist state reconstruction from observed outputs.

Do not use it to generate encryption keys, session tokens, passwords, or other security-sensitive secrets.

For numerical simulation, that limitation may be irrelevant. For security, it is fundamental.

#### 4 Standard-library sequence versus distribution sequence

The C++ standard specifies the engine algorithms and their parameters.

But the exact transformation performed by standard-library distribution classes such as `std::uniform_real_distribution` and `std::normal_distribution` is not universally bit-identical across library implementations.

Thus:

```cpp
std::mt19937_64 engine(seed);
```

is a useful reproducibility foundation, but exact cross-library sample equivalence needs a specified distribution mapping as well.

### PCG-style generators

PCG stands for Permuted Congruential Generator.

The central design idea is to combine a simple state transition, often based on an LCG, with an output permutation that improves the statistical appearance of the exposed bits.

Conceptually:

$$
 \boxed{ s\_{n+1}=as_n+c\pmod{2^w} } 
$$

followed by:

$$
 \boxed{ r_n=P(s_n) } 
$$

where $P$ is a suitable output permutation.

The output permutation can include XOR shifts, rotations, and other bit-mixing operations, depending on the PCG variant.

The distinction is crucial: the state transition and output function serve different purposes.

The transition determines how the state evolves.

The output permutation transforms internal state bits into exposed random bits with improved distributional properties.

#### 1 PCG32 structure

A representative PCG32-style generator uses 64-bit internal state and emits 32-bit outputs.

Its state transition may use:

$$
 s\_{n+1} = 6364136223846793005\\,s_n+c \pmod{2^{64}}, 
$$

with an odd increment $c$.

The output applies a combination of XOR shifts and bit rotations to the old state.

A full implementation must handle seeding and stream selection correctly rather than simply assigning arbitrary fields.

Melissa O'Neill's PCG documentation includes a minimal C implementation, proper seeding guidance, and the mathematical and statistical motivation for the family. 

#### 2 Streams

Some PCG variants permit different streams using different increments.

This can be useful when assigning a logical random stream to each worker or subsystem.

However, different stream parameters should not automatically be described as mathematically independent random processes.

Their practical suitability depends on the generator family's guarantees, initialization strategy, and the particular statistical requirements.

#### 3 Why PCG is useful for engine development

A small generator state reduces memory pressure.

Fast integer operations are suitable for high-throughput computations.

Explicit seeds support deterministic debugging.

Generator instances can be assigned to objects or workers without hidden global state.

But PCG is not the only reasonable choice. Counter-based generators and carefully scrambled low-discrepancy samplers can be preferable in some GPU and rendering workloads.

Generator selection should be guided by target hardware and measured sample quality, not only CPU throughput.

### Seed selection

A seed determines the initial state or initial parameters of a pseudorandom generator.

A seed is not itself a guarantee of statistical quality.

#### 1 Three distinct seed requirements

Reproducibility: Given a specified experiment seed, we want the same logical samples when the experiment is repeated.

Stream separation: Different pixels, workers, particles, or simulations should avoid accidental reuse of the same random sequence when their outputs need independent-looking randomness.

Traceability: We want to reconstruct which random inputs produced an anomalous result.

A good experiment should record the master seed, generator algorithm and version, stream-derivation method, and relevant sample indexing scheme.

#### 2 A master-seed architecture

A practical system can derive stream identifiers from a master seed:

$$
 s_i=H(s\_{\mathrm{master}},i). 
$$

Here $i$ identifies a worker or logical stream, and $H$ is a suitable mixing function.

If the application also uses sample indices and dimensions:

$$
 r=F(s\_{\mathrm{master}},\text{pixel},\text{sample},\text{dimension}), 
$$

the generated value can be independent of execution scheduling.

#### 3 Time-based seeding is not automatically desirable

Using the current clock time as a seed can produce different runs, but that often makes debugging and scientific reproduction harder.

For research benchmarks, a fixed recorded collection of seeds is usually more useful than uncontrolled time-based seeding.

If different randomized trials are required, choose and record a set of trial seeds explicitly.

Do not rerun until a favorable result appears and report only that result.

#### 4 Seed collisions

If seeds are drawn from a small space, collisions can occur.

Two generator instances initialized with the same complete state usually produce the same subsequent stream.

If that is unintended, correlated samples or identical noise patterns can arise.

A wider master-seed space and a well-designed stream derivation reduce accidental collisions, but no finite scheme can provide infinitely many unique states.

### Hash-based stateless random generation

A stateful PRNG consumes its sequence in order.

A stateless generator instead calculates a random-looking output from explicit inputs.

For example:

$$
 \boxed{ r=H(\text{seed},x,y,s,d) } 
$$

where:

- $x,y$ identify a pixel.
- $s$ identifies the sample number.
- $d$ identifies a sampling dimension.
- The global seed identifies the experiment.

The output does not depend on how many random values were generated previously.

#### 1 Why sample dimension matters

A path tracer may need random values for camera jitter, lens position, light selection, light-surface coordinates, BSDF selection, direction sampling, and Russian roulette.

These represent different sampling dimensions.

If two unrelated decisions accidentally reuse the same random variable, unwanted correlation can appear.

An explicit dimension index helps keep logical random choices separate and reproducible.

#### 2 A simple stateless hash-mixing pattern

The following is a pedagogical 64-bit integer mixer. It uses unsigned overflow intentionally, which is defined modulo $2^{64}$.

```cpp
#include <cstdint>

// An avalanche-style 64-bit mixing function.
// It maps one 64-bit input to one scrambled 64-bit output.
//
// A mixer is NOT, by itself, a proof that a collection of
// indexed outputs behaves like independent random variables.
std::uint64_t mix64(std::uint64_t x) noexcept {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;

    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;

    x ^= x >> 31;
    return x;
}
```

We can derive a sample from explicit coordinates:

```cpp
std::uint64_t sample_key(
    std::uint64_t globalSeed,
    std::uint32_t pixelX,
    std::uint32_t pixelY,
    std::uint32_t sampleIndex,
    std::uint32_t dimension
) noexcept {
    std::uint64_t key = mix64(globalSeed);

    // Mix each logical coordinate into the accumulated key.
    // Different dimensions and pixels obtain different inputs.
    key = mix64(key ^ pixelX);
    key = mix64(key ^ pixelY);
    key = mix64(key ^ sampleIndex);
    key = mix64(key ^ dimension);

    return key;
}
```

This design is easy to reproduce and independent of task execution order.

However, simple hashing does not automatically provide the distributional guarantees of a carefully analyzed counter-based PRNG.

For demanding GPU simulation, counter-based families such as Philox or Threefry have been designed and studied specifically for parallel random generation. Their structure is grounded in the work of Salmon, Moraes, Dror, and Shaw on parallel random numbers. 

GitHub

#### 3 Stateless sampling and debugging

Suppose pixel $(500,250)$, sample 127, dimension 8 produces a bright firefly.

With stateless generation, we can reconstruct its random input directly from that tuple.

We do not need to replay every sample generated by every worker before it.

This is especially useful in massively parallel rendering, where execution order may vary between runs.

### Random-number quality for rendering

This is where probability theory meets actual image quality.

A renderer may use millions of samples, yet still produce objectionable artifacts if the sample distributions have undesirable spatial structure.

#### 1 One-dimensional uniformity is insufficient

Suppose a sequence has an approximately uniform histogram on $[0,1)$.

Now pair consecutive values:

$$
 (u_0,u_1),\quad(u_2,u_3),\quad\ldots. 
$$

If the paired points align along a small number of lines, the two-dimensional sampling pattern is poor even though the one-dimensional histogram looks good.

Rendering frequently uses two-dimensional or higher-dimensional samples.

A camera ray may use:

$$
 (u_1,u_2) 
$$

for pixel position, another pair for lens coordinates, and additional dimensions for bounce sampling.

Thus multidimensional correlations matter.

#### 2 White noise, stratification, and low discrepancy

Independent pseudorandom samples can cluster and leave gaps.

For example, ten random points in a pixel may coincidentally land in one small region.

[[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#6. A computer produces deterministic pseudorandom sequences|Stratified sampling]] subdivides the domain and places samples in separate regions to reduce large gaps.

Low-discrepancy constructions, such as suitably scrambled Sobol sequences, aim for more even coverage of multidimensional domains.

These techniques are not merely interchangeable forms of randomness.

They have different mathematical guarantees, implementation structures, and error behavior.

We will formally develop stratified and quasi-random methods in a later stratified and quasi-random sampling extension, but their importance begins here: sample quality depends on where points lie relative to the integrand, not merely on the marginal distribution of each coordinate.

#### 3 Test the estimator, not only the generator

A statistically strong integer PRNG does not guarantee low-variance rendered images.

Conversely, a carefully constructed low-discrepancy sampling scheme may outperform ordinary independent random samples for certain smooth integrals even though its samples are not independent in the conventional Monte Carlo sense.

A renderer should evaluate both mathematical sample properties and application-level outcomes.

Useful diagnostics include spatial point patterns, multidimensional scatter plots, autocorrelation, repeated-seed image variance, convergence versus sample count, firefly frequency, and benchmark-scene error against a trusted reference.

Physically Based Rendering, Fourth Edition treats sampling as a complete part of the rendering system, including stratification, sample placement, sampler interfaces, and the relationship between Monte Carlo and reconstruction error. 

pbr-book.org+2

![[Visual Recall/Math Foundations/F10-6_toy-lcg-stream.png]]

**F10-6. A full-period toy LCG can still have a simple low-bit pattern.** For m=16, a=5, c=1, all 16 states are visited but the least-significant bit alternates. Full period does not imply good multidimensional quality. The keyed-input sketch indicates deterministic indexing only: a different key does not establish statistical independence or collision freedom.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/29 - Discretization, Convergence, and Experimental Design#6. A computer produces deterministic pseudorandom sequences|counter-based RNG]] — Stateless sampling derives an output from explicit coordinates such….



<!-- end V10.07 -->

<!-- begin V10.08 -->
## 7. Transformations turn simple inputs into useful samples

### Transforming uniform random variables

We now have a source of approximately uniform random values. But a ray tracer needs directions on hemispheres, positions on surfaces, microfacet-normal distributions, lens samples, and many other nonuniform quantities.

How do we transform:

$$
 U\sim\mathcal U(0,1) 
$$

into a target distribution?

The general idea is to define a mapping:

$$
 \boxed{X=g(U)} 
$$

and choose $g$ so that the resulting variable $X$ has the desired probability law.

There are several ways to design such transformations, including inverse-CDF methods, geometric mappings, and rejection sampling.

We must distinguish two tasks:

1. Constructing a sample from a desired distribution.
2. Calculating the correct probability density of that sample.

The second is essential for unbiased Monte Carlo weighting.

#### 1 Simple affine transformations

Suppose:

$$
 U\sim\mathcal U(0,1). 
$$

We want:

$$
 X\sim\mathcal U(a,b). 
$$

Define:

$$
 \boxed{X=a+(b-a)U}. 
$$

Because $U\in[0,1]$, $X$ lies in $[a,b]$.

The inverse mapping is:

$$
 U=\frac{X-a}{b-a}. 
$$

Its derivative is:

$$
 \frac{dU}{dX}=\frac1{b-a}. 
$$

The density transformation formula gives:

$$
 p_X(x)= p_U\left(\frac{x-a}{b-a}\right) \left|\frac1{b-a}\right|. 
$$

Inside the interval, $p_U=1$, so:

$$
 \boxed{p_X(x)=\frac1{b-a}}. 
$$

The same affine transformation that changes the numerical range also changes the density according to the Jacobian.

#### 2 Why sampling and PDF calculation must agree

Suppose a renderer generates directions using a cosine-weighted sampler but evaluates their weights using a uniform-hemisphere PDF.

The generated samples follow one probability law, but the estimator divides by another.

The expectation no longer necessarily equals the intended integral.

This can produce systematic brightness errors, not merely noisy images.

Every sampling API should therefore define a coherent relationship among the generated sample, its distribution, its support, and the density measure.

A useful conceptual interface is:

```cpp
struct DirectionSample {
    Vec3 direction;

    // Probability density with respect to solid angle.
    // Units: inverse steradians.
    double pdfSolidAngle;
};
```

The PDF's measure is part of its meaning.

A surface-area PDF cannot be substituted for a solid-angle PDF without a Jacobian conversion.

### Inverse-transform sampling

Suppose $X$ has cumulative distribution function $F_X(x)$.

For a continuous strictly increasing CDF, we can sample $X$ by:

$$
 \boxed{X=F_X^{-1}(U)} 
$$

where:

$$
 U\sim\mathcal U(0,1). 
$$

Why does this work?

Let:

$$
 X=F^{-1}(U). 
$$

For a monotone increasing inverse:

$$
 P(X\le x) = P(F^{-1}(U)\le x). 
$$

Apply the increasing function $F$:

$$
 P(X\le x)=P(U\le F(x)). 
$$

Because $U$ is uniform on $[0,1]$:

$$
 P(U\le v)=v 
$$

for $v\in[0,1]$.

Therefore:

$$
 \boxed{P(X\le x)=F(x)}. 
$$

The constructed random variable has the desired CDF.

For general distributions with jumps, the generalized inverse, or quantile function, provides the corresponding construction.

#### 1 Sampling a triangular PDF

Consider the target density:

$$
 p(x)=2x,\qquad0\le x\le1. 
$$

Check normalization:

$$
 \int_0^12x\\,dx = [x^2]\_0^1=1. 
$$

Its CDF is:

$$
 F(x)=\int_0^x2t\\,dt=x^2. 
$$

Set:

$$
 u=F(x)=x^2. 
$$

Solve for $x$:

$$
 \boxed{x=\sqrt u}. 
$$

Thus:

$$
 \boxed{X=\sqrt U}. 
$$

The transformation requires only one uniform random number and a square root.

```cpp
double sampleIncreasingLinearPdf(double u) {
    // Target: p(x) = 2*x on [0,1].
    // Its CDF is x*x; [[Inverse, rejection, and importance sampling|inverse CDF]] is sqrt(u).
    if (!std::isfinite(u) || u < 0.0 || u >= 1.0)
        throw std::domain_error("Expected [0,1) sample");

    return std::sqrt(u);
}
```

The supplied source version used the following guard; both comparisons are false for NaN:

```cpp
double sampleIncreasingLinearPdf(double u) {
    // Target: p(x) = 2*x on [0,1].
    // Its CDF is x*x; inverse CDF is sqrt(u).
    if (u < 0.0 || u >= 1.0)
        throw std::domain_error("Expected [0,1) sample");

    return std::sqrt(u);
}
```

The staged teaching version adds an explicit finiteness check before the interval comparisons.

#### 2 Verifying the output distribution

For $0\le x\le1$:

$$
 P(X\le x) = P(\sqrt U\le x) = P(U\le x^2) =x^2. 
$$

Differentiate the CDF:

$$
 p_X(x)=2x. 
$$

The transformed sample follows the intended distribution.

#### 3 The discrete floating-point subtlety

A real PRNG produces a finite set of floating-point values rather than an ideal continuous $U$.

The transformed sequence is therefore a discrete approximation to the target continuous distribution.

Moreover, if $U=0$, the transformation gives $X=0$, where the target PDF $2x$ vanishes.

This is a zero-probability boundary in the continuous model, but the discrete sampler can encounter it.

If the next computation divides by $p(X)$, we must handle the boundary deliberately.

A sampling implementation should not silently produce an infinite estimator weight because it treated a finite-grid boundary as mathematically impossible.

#### 4 Sampling an exponential waiting time

For the exponential distribution:

$$
 F(x)=1-e^{-\lambda x}. 
$$

Set:

$$
 u=1-e^{-\lambda x}. 
$$

Rearrange:

$$
 e^{-\lambda x}=1-u. 
$$

Taking logarithms:

$$
 \boxed{ x=-\frac{\log(1-u)}{\lambda} }. 
$$

A numerically careful implementation uses:

```cpp
double sampleExponential(double u, double lambda) {
    if (!(lambda > 0.0) || !std::isfinite(lambda))
        throw std::domain_error("Invalid rate");

    if (!(u >= 0.0 && u < 1.0))
        throw std::domain_error("Invalid uniform input");

    // log1p(-u) accurately computes log(1-u),
    // particularly for small u.
    return -std::log1p(-u) / lambda;
}
```

The output follows the exponential law when the input is ideally uniform.

### Rejection sampling

Inverse-transform sampling requires access to an inverse CDF or a convenient sampling transformation.

What if the desired density is complicated and difficult to invert?

Rejection sampling constructs samples from a proposal distribution and accepts or rejects them so that the accepted values follow the target density.

Suppose the target PDF is $p(x)$ and we can easily sample a proposal PDF $q(x)$.

Choose a constant $M$ satisfying:

$$
 \boxed{ p(x)\le Mq(x) } 
$$

for every point in the relevant support.

The algorithm is:

1. Sample $X\sim q$.
2. Sample an independent $U\sim\mathcal U(0,1)$.
3. Accept $X$ if:

$$
 \boxed{ U\le\frac{p(X)}{Mq(X)} }. 
$$

Otherwise, reject and repeat.

The acceptance probability at a particular proposed $x$ is proportional to $p(x)/q(x)$.

#### 1 Proving the accepted distribution

The joint probability density of proposing $x$ and accepting it is proportional to:

$$
 q(x)\frac{p(x)}{Mq(x)} = \frac{p(x)}M. 
$$

The overall probability of accepting any proposal is:

$$
 P(\text{accept}) = \int q(x)\frac{p(x)}{Mq(x)}\\,dx. 
$$

Cancel $q(x)$:

$$
 P(\text{accept}) = \frac1M\int p(x)\\,dx. 
$$

Since $p$ is normalized:

$$
 \boxed{ P(\text{accept})=\frac1M }. 
$$

Conditioning on acceptance gives:

$$
 p(x\mid\text{accept}) = \frac{p(x)/M}{1/M} =p(x). 
$$

Therefore the accepted samples have the desired density.

This proof assumes a valid finite envelope $M$, an appropriate proposal support, and independent random inputs.

#### 2 Numerical example: sampling $p(x)=2x$

Use:

$$
 p(x)=2x,\qquad0\le x\le1. 
$$

Choose a uniform proposal:

$$
 q(x)=1. 
$$

Because:

$$
 2x\le2, 
$$

we can use:

$$
 M=2. 
$$

The acceptance probability becomes:

$$
 \frac{p(x)}{Mq(x)} = \frac{2x}{2} =x. 
$$

Therefore the algorithm is remarkably simple.

Generate two independent uniform random values $u_1,u_2$.

Set:

$$
 x=u_1. 
$$

Accept if:

$$
 \boxed{u_2\<x}. 
$$

Otherwise try again.

#### 3 Acceptance efficiency

The theoretical acceptance probability is:

$$
 \frac1M=\frac12. 
$$

Therefore the expected number of proposals per accepted sample is:

$$
 \boxed{M=2}. 
$$

On average, we need two proposal attempts to obtain one accepted sample.

But the exact number is random.

It follows a geometric-style waiting distribution under independent attempts.

The algorithm has an unbounded worst-case number of attempts, even though the expected number is finite.

This matters on GPUs.

If thousands of lanes execute rejection sampling together, different lanes may need different numbers of attempts. Some lanes finish early, while others continue looping.

Such divergence can reduce hardware utilization.

For distributions with efficient direct transformations, inverse sampling may avoid this variable-length execution.

#### 4 When rejection sampling is invalid or inefficient

If the proposal density is zero where the target is positive, no finite envelope can represent the missing region.

The sampling algorithm cannot generate samples from that region, creating bias or invalidity.

If $M$ is extremely large, the acceptance probability $1/M$ is tiny.

The method becomes inefficient.

The goal is therefore to choose a proposal that is easy to sample and resembles the target sufficiently well to keep $M$ small.

The PBRT sampling-algorithms appendix examines rejection sampling alongside inverse-transform and other sampling methods.

### Importance sampling

Importance sampling is among the most consequential concepts in Monte Carlo rendering.

Suppose our goal is to evaluate:

$$
 \boxed{ I=\int_Df(x)\\,dx }. 
$$

We choose a normalized PDF $p(x)$ that is strictly positive wherever $f(x)$ makes a relevant contribution, apart from harmless measure-zero exceptions.

Multiply and divide by the PDF:

$$
 I= \int_D \frac{f(x)}{p(x)}p(x)\\,dx. 
$$

Recognize the expectation:

$$
 \boxed{ I= \mathbb E\_{X\sim p} \left[ \frac{f(X)}{p(X)} \right] }. 
$$

Given $N$ independent samples $X_1,\ldots,X_N\sim p$, the Monte Carlo estimator is:

$$
 \boxed{ \widehat I_N= \frac1N\sum\_{i=1}^{N} \frac{f(X_i)}{p(X_i)} }. 
$$

The division by the actual sampling PDF is essential.

Sampling more frequently in important regions without compensating for the PDF generally changes the expected result.

#### 1 Derive unbiasedness

For one sample, define:

$$
 Y=\frac{f(X)}{p(X)}. 
$$

Then:

$$
 \mathbb E[Y] = \int_D \frac{f(x)}{p(x)}p(x)\\,dx = \int_Df(x)\\,dx =I. 
$$

Therefore:

$$
 \mathbb E[\widehat I_N] = \frac1N\sum_i\mathbb E[Y_i] =I. 
$$

Thus:

$$
 \boxed{ \mathbb E[\widehat I_N]=I }. 
$$

The estimator is unbiased under the stated sampling and integrability assumptions.

#### 2 Importance-sampling variance

The variance of one contribution is:

$$
 \operatorname{Var}(Y) = \mathbb E[Y^2]-I^2. 
$$

Now:

$$
 \mathbb E[Y^2] = \int_D \left(\frac{f(x)}{p(x)}\right)^2p(x)\\,dx = \int_D\frac{f(x)^2}{p(x)}\\,dx. 
$$

Therefore:

$$
 \boxed{ \operatorname{Var}(Y) = \int_D\frac{f(x)^2}{p(x)}\\,dx-I^2 }. 
$$

For independent samples with finite variance:

$$
 \boxed{ \operatorname{Var}(\widehat I_N) = \frac{\operatorname{Var}(Y)}{N} }. 
$$

Importance sampling is valuable when a good choice of $p$ substantially reduces this variance.

#### 3 Worked example: estimating $\int_0^1x^2\\,dx$

Consider:

$$
 I=\int_0^1x^2\\,dx=\boxed{\frac13}. 
$$

First use uniform sampling:

$$
 p_U(x)=1. 
$$

Then the sample contribution is:

$$
 Y_U=x^2. 
$$

Its expected value is:

$$
 \mathbb E[Y_U]=\frac13. 
$$

Its second moment is:

$$
 \mathbb E[Y_U^2] = \int_0^1x^4\\,dx =\frac15. 
$$

Thus:

$$
 \operatorname{Var}(Y_U) = \frac15-\frac19 = \boxed{\frac4{45}} \approx0.0888889. 
$$

Now choose the nonuniform importance PDF:

$$
 p_I(x)=2x. 
$$

We already know how to sample it:

$$
 X=\sqrt U. 
$$

The weighted estimator contribution becomes:

$$
 Y_I= \frac{x^2}{2x} =\frac x2 
$$

for $x>0$.

Its mean remains:

$$
 \mathbb E[Y_I] = \int_0^1\frac x2(2x)\\,dx = \int_0^1x^2\\,dx =\frac13. 
$$

Its second moment is:

$$
 \begin{aligned} \mathbb E[Y_I^2] &= \int_0^1\left(\frac x2\right)^2(2x)\\,dx\\ &= \frac12\int_0^1x^3\\,dx\\ &=\frac12\cdot\frac14\\ &=\frac18. \end{aligned} 
$$

Therefore:

$$
 \operatorname{Var}(Y_I) = \frac18-\frac19 = \boxed{\frac1{72}} \approx0.0138889. 
$$

Compare variances:

$$
 \boxed{ \frac{\operatorname{Var}(Y_U)} {\operatorname{Var}(Y_I)} = \frac{4/45}{1/72} =6.4 }. 
$$

The importance-sampling estimator has 6.4 times smaller per-sample variance than the uniform estimator.

At a fixed number of independent samples, the ratio of estimator variances is also 6.4.

The corresponding standard-deviation improvement is:

$$
 \sqrt{6.4}\approx2.53. 
$$

For equal target standard error and equal sample cost, uniform sampling would require approximately 6.4 times as many samples.

The equal-cost qualification matters. If generating importance samples were substantially more expensive, the performance comparison would change.

Variance of one Monte Carlo contribution

Exact analytical variances for estimating ∫₀¹ x² dx. Lower is better.

00.0250.050.0750.1Uniformp(x)=2xp\*(x)=3x²

#### 4 Deriving the ideal importance distribution

For a nonnegative integrand $f(x)\ge0$, suppose the integral is:

$$
 I=\int_Df(x)\\,dx>0. 
$$

Choose:

$$
 \boxed{ p^\star(x)=\frac{f(x)}I }. 
$$

Then:

$$
 \frac{f(x)}{p^\star(x)} = \frac{f(x)}{f(x)/I} =I 
$$

where $f(x)>0$.

Every sample contribution is identical.

Therefore:

$$
 \boxed{ \operatorname{Var}(Y)=0 }. 
$$

This is the zero-variance importance distribution.

For our example:

$$
 f(x)=x^2, \qquad I=\frac13. 
$$

Thus:

$$
 \boxed{ p^\star(x)=3x^2 }. 
$$

Its CDF is:

$$
 F(x)=x^3. 
$$

We can sample it using:

$$
 \boxed{ X=U^{1/3} }. 
$$

Every valid weighted contribution is:

$$
 \frac{x^2}{3x^2}=\frac13. 
$$

The variance is zero in the ideal mathematical model.

#### 5 Why we do not always use the ideal PDF

The formula:

$$
 p^\star(x)=\frac{f(x)}I 
$$

contains the unknown integral $I$.

In the general problem, we are estimating $I$ precisely because it is not readily known.

The ideal distribution might also be difficult to sample even if its normalization were known.

For signed integrands, an ideal zero-variance distribution is not generally obtained by simply setting $p=f/I$, because a PDF must be nonnegative.

A common variance-minimizing density among ordinary positive PDFs is proportional to $|f|$, subject to the relevant assumptions. If $f$ changes sign, weighted contributions retain a sign and zero variance is generally impossible under this approach unless special structure exists.

The practical goal is therefore to construct an efficiently sampled approximation to the important regions of the integrand.

For rendering, the BSDF, cosine factor, light-source geometry, and visibility may suggest different useful sampling distributions.

### Probability density under variable transformation

Importance sampling is valid only when the estimator uses the PDF corresponding to the actual sampled variable and measure.

Suppose a random variable $X$ has PDF $p_X(x)$, and we transform it:

$$
 Y=g(X). 
$$

If $g$ is differentiable, invertible, and has a nonzero Jacobian determinant, the transformed PDF is:

$$
 \boxed{ p_Y(y) = p_X(g^{-1}(y)) \left|\det J\_{g^{-1}}(y)\right| }. 
$$

Equivalently, at corresponding points $y=g(x)$:

$$
 \boxed{ p_Y(g(x)) = \frac{p_X(x)}{|\det J_g(x)|} }. 
$$

The determinant measures how a small volume element expands or contracts under the mapping.

#### 1 Why the determinant appears

For a small region near $x$, probability is approximately:

$$
 p_X(x)\\,dV_x. 
$$

The transformation maps it into a region with volume:

$$
 dV_y= |\det J_g(x)|\\,dV_x. 
$$

Probability is preserved:

$$
 p_Y(y)\\,dV_y=p_X(x)\\,dV_x. 
$$

Substitute the volume relation:

$$
 p_Y(y) |\det J_g(x)|dV_x = p_X(x)dV_x. 
$$

Cancel $dV_x$:

$$
 p_Y(y)= \frac{p_X(x)}{|\det J_g(x)|}. 
$$

This is the multivariable form of the one-dimensional inverse-derivative rule.

#### 2 Area PDF versus solid-angle PDF

In direct lighting, we may sample a point on a light source using a PDF defined with respect to surface area:

$$
 p_A(x). 
$$

But the rendering equation integrates incoming directions with respect to solid angle:

$$
 d\omega. 
$$

We need to convert the density.

Consider a small surface patch of area $dA$, at distance $r$ from the shading point.

Let $\theta_L$ be the angle between the light patch's normal and the direction toward the shading point.

The patch subtends solid angle:

$$
 \boxed{ d\omega= \frac{|\cos\theta_L|}{r^2}\\,dA }. 
$$

Therefore:

$$
 dA= \frac{r^2}{|\cos\theta_L|}\\,d\omega. 
$$

Probability conservation gives:

$$
 p_A\\,dA=p\_\omega\\,d\omega. 
$$

Substitute:

$$
 p_A\frac{r^2}{|\cos\theta_L|}\\,d\omega = p\_\omega\\,d\omega. 
$$

Thus:

$$
 \boxed{ p\_\omega= p_A\frac{r^2}{|\cos\theta_L|} }. 
$$

The inverse relation is:

$$
 \boxed{ p_A= p\_\omega\frac{|\cos\theta_L|}{r^2} }. 
$$

These are essential conversions for sampling area lights in path tracing.

#### 3 Physical meaning

A nearby patch subtends a larger solid angle than an equally sized distant patch.

Thus distance affects how probability density transforms.

A patch seen at a grazing angle subtends less solid angle than the same patch viewed face-on, so the angle also matters.

The $r^2$ and cosine factors are geometrical Jacobians.

They are not arbitrary rendering corrections.

#### 4 Singular and many-to-one cases

When:

$$
 |\cos\theta_L|=0, 
$$

the differential mapping becomes singular at the grazing configuration.

Implementations must avoid invalid divisions and handle degenerate geometry.

Also, one ray direction can sometimes intersect multiple surfaces along its path. In such cases, a naive one-to-one density transformation may be insufficient; the probability model must account for the actual sampling procedure and mapping multiplicity.

A PDF conversion must be derived for the geometry and sampling measure used by the algorithm, not copied mechanically without checking the assumptions.

![[Visual Recall/Math Foundations/F10-3_uniform-disk-sampling.png]]

**F10-3. Uniform area sampling requires a square-root radial transform.** The theoretical mean radius is 2/3 for uniform area in the unit disk and 1/2 for uniform radius; plotted sample means are finite-run illustrations.


<!-- end V10.08 -->

<!-- begin V10.09 -->
### Monte Carlo estimates an expectation by sampling

Suppose the quantity of interest can be written as an expectation $I=E[g(X)]$. Draw IID samples $X_1,\ldots,X_n$ from the modeled distribution and use

$$
\hat I_n=\frac{1}{n}\sum_{i=1}^n g(X_i).
$$

If $E|g(X)|<\infty$, then $E[\hat I_n]=I$: the estimator is unbiased. If $\operatorname{Var}(g(X))<\infty$, then $\operatorname{Var}(\hat I_n)=\operatorname{Var}(g(X))/n$. Under the law of large numbers, the estimate converges to $I$ in probability (and under standard IID integrability conditions, almost surely). Unbiasedness is a finite-sample expectation identity. Consistency is an asymptotic convergence property. Neither says that a particular finite estimate must be close to the answer; an unbiased estimator can have enormous variance.

A simple integral makes the construction visible. The target $I=\int_0^1 x\,dx=1/2$ equals $E[U]$ when $U\sim\operatorname{Uniform}(0,1)$. If ten independent uniform samples happen to be approximately

$$
0.05,\ 0.15,\ 0.25,\ 0.35,\ 0.45,\ 0.55,\ 0.65,\ 0.75,\ 0.85,\ 0.95,
$$

their mean is $0.50$. Their sample variance is approximately $0.0917$ and the estimated standard error of the mean is $\sqrt{0.0917/10}\approx0.096$. Treating those as random IID uniform draws, a large-sample normal interval using $1.96$ standard errors would be roughly $0.50\pm0.19$, or $[0.31,0.69]$. This is a noisy estimate even though the observed mean equals the exact integral. The sequence above is evenly spaced for arithmetic clarity, not evidence of independent random sampling; a randomized stratified estimator would need its own variance calculation.

For IID observations with finite variance, a central-limit approximation says the standardized sample mean approaches a standard normal distribution as $n$ grows. This motivates the approximate interval $\hat I_n\pm1.96\,s_g/\sqrt n$ for a nominal 95% interval, where $s_g$ is the sample standard deviation of $g(X_i)$. It is an approximation, not an exact finite-sample coverage guarantee. Heavy tails, very small sample sizes, correlated samples, adaptive stopping, and a misspecified sampling model can all invalidate the usual interval. There is no universal sample count such as “30” that repairs every distribution or dependence problem.

A rigorous finite-sample alternative follows from assumptions such as boundedness. If $Y_i\in[a,b]$ are independent with common mean $\mu$, Hoeffding's inequality gives

$$
P(|\bar Y_n-\mu|\ge\epsilon)\le2\exp\left(-\frac{2n\epsilon^2}{(b-a)^2}\right).
$$

Solving this bound yields a two-sided interval radius $(b-a)\sqrt{\log(2/\alpha)/(2n)}$ with coverage at least $1-\alpha$. For eight independent Bernoulli trials with four observed successes, $a=0,b=1,n=8,\alpha=0.05$, so the radius is $\sqrt{\log(40)/16}\approx0.480$. The resulting interval is approximately $[0.020,0.980]$, clipped to the parameter range $[0,1]$. It is much wider than a normal approximation at this small sample size, but its guarantee relies only on independent bounded observations and the stated mean model. Tighter intervals are possible with more assumptions or exact binomial methods.

### Confidence and credible intervals answer different questions

A frequentist $1-\alpha$ confidence procedure is a rule that maps random data to a random set $C(X)$. Its coverage statement is $P_\theta(\theta\in C(X))\ge1-\alpha$ for each parameter value $\theta$ in the model. Before observing data, the interval is random and the fixed parameter is covered with the stated long-run frequency. After one interval has been calculated, the frequentist statement does not assign probability $0.95$ to the fixed parameter lying inside that particular interval.

A Bayesian credible interval is calculated from a posterior distribution. Given observed data $x$, a $1-\alpha$ credible set $C(x)$ is chosen so that $P(\Theta\in C(x)\mid X=x)=1-\alpha$. Here $\Theta$ is modeled as random under the prior and likelihood. The statement is conditional on the observed data and depends on that prior/model. A 95% credible interval need not have 95% frequentist coverage across repeated datasets; frequentist coverage is a separate property to check.

For that dataset of four successes and four failures, a uniform prior $p\sim\operatorname{Beta}(1,1)$ gives posterior $p\mid x\sim\operatorname{Beta}(5,5)$. A Beta$(a,b)$ density on $0<p<1$ is proportional to $p^{a-1}(1-p)^{b-1}$. The four successes and four failures contribute likelihood $p^4(1-p)^4$, so multiplying prior and likelihood changes the two exponents from zero to four and produces the stated posterior. The posterior mean is $5/10=0.5$. A credible interval is obtained by taking posterior quantiles of that Beta distribution. The frequentist Hoeffding interval we just computed is instead derived from a sampling guarantee that holds for every fixed $p$. They may have different widths and interpretations while using the same observed count. Neither interval can be described correctly by its “95%” label alone.

A common failure is to calculate a posterior after choosing a prior but report it as a confidence interval without qualification. Another is to select a frequentist interval after many repeated looks and quote the single-look coverage as if nothing had been selected. Each procedure must include how the data were generated, how the interval was chosen, and whether optional stopping or multiple comparisons were part of that rule.

### Sampling selection changes the uncertainty story

Stopping when the data look favorable can bias a naïve summary. Let $X_i$ be IID Bernoulli$(p)$ trials. Observe one trial; stop immediately if it succeeds, otherwise observe exactly one more trial and then stop. The stopping time $N$ is therefore either one or two. The reported sample mean is $1$ after an immediate success, $1/2$ after failure then success, and $0$ after two failures. Its expectation is $p+(1-p)p/2$, which is $0.28$ when $p=0.2$, rather than $0.2$. The estimator is biased upward under this stopping rule. The reason is selection: the sample size and the reported data are linked by the success condition. The fixed-$n$ identity $E[\bar X_n]=p$ does not apply automatically to $\bar X_N$. If we instead stop at the first success with no two-trial cap, the stopped mean is $1/N$, not always one.

The same effect appears in adaptive systems. If a renderer keeps allocating paths to a pixel until its current estimate crosses a brightness threshold, then reports the stopped mean without an appropriate sequential method, the selected estimate can be biased. If a vision pipeline examines many candidate boxes and reports the one with the highest noisy score, that winning score is upward-selected even when each individual score estimator is unbiased. If an engine repeatedly checks a collision confidence interval and stops the first time it excludes zero, ordinary fixed-time coverage no longer automatically applies to the stopped interval.

Not every data-dependent policy destroys unbiasedness. Some carefully designed sequential estimators remain unbiased, and time-uniform confidence sequences provide coverage simultaneously over all sample counts under stated assumptions. But optional stopping cannot be ignored. The analysis has to match the actual stopping rule, not only the estimator that would have been used at a predetermined sample count.

Sample selection is also present in an adaptive query policy. An active vision system may inspect regions with high current uncertainty more often. A [[Concepts/Bandit exploration and regret|bandit]] may display the apparently better recommendation more frequently. These actions change which outcomes become observable. An offline average over the observed outcomes does not automatically estimate the average outcome under a different policy or over the full population. The missing observations require a model and, often, correction weights.

### Importance weighting corrects selection only with support

Suppose the target distribution over outcomes is $P$, but samples are drawn from a proposal distribution $Q$. If $Q$ assigns positive probability wherever $P$ assigns positive probability and the required expectation exists, then

$$
E_P[f(X)]=E_Q\left[f(X)\frac{p(X)}{q(X)}\right].
$$

The ratio $p/q$ is an importance weight. It corrects for how sampling differs from the target. The support condition matters: if $q(x)=0$ where $p(x)f(x)$ contributes nonzero mass, the target contribution is never observed and no finite weight can recover it. Tiny $q(x)$ preserves formal unbiasedness if nonzero, but can create huge weights and huge variance.

Use a two-point example. The target chooses $a$ or $b$ with probability $1/2$ each, and $f(a)=1,f(b)=0$, so the target expectation is $I=1/2$. Choose a proposal with $Q(a)=0.1,Q(b)=0.9$. The weighted sample is $Y=f(X)P(X)/Q(X)$: it equals $5$ with probability $0.1$ when $X=a$, and $0$ with probability $0.9$ when $X=b$. Then $E_Q[Y]=0.1(5)+0.9(0)=0.5$, exactly the target, but $E_Q[Y^2]=2.5$ and $\operatorname{Var}_Q(Y)=2.5-0.25=2.25$. One draw is usually zero and occasionally five. If $Q(a)$ were zero, the estimator would miss the whole nonzero target contribution and be biased.

In a bandit, suppose an arm's reward $r\in[0,1]$ is observed only when it is selected with conditional probability $q>0$. The importance-weighted observation $\hat r=\mathbf 1\{A=a\}r/q$ has conditional expectation $r$ when the reward is fixed for that round and the action is randomized according to $q$. Its second moment is $r^2/q$ and its variance is $r^2(1-q)/q$. Thus reducing the probability of an arm increases the variance of its counterfactual reward estimate. If $q=0$, there is no information about that arm from that round. Exploration probability is therefore not a cosmetic add-on: it controls the amount of information and the stability of weighted estimates.

For bounded logged rewards, inverse propensity weighting can give an unbiased estimate of a target policy's value when the logging probability is known, the target policy has support under the logging policy, and the relevant conditional model is correct. Large or misspecified propensity weights can make the estimate unusably noisy. Clipping weights may reduce variance but generally introduces bias. “We reweighted it” is not enough; report overlap, maximum weights, effective sample size, and the estimator's assumptions.

### Confidence bounds drive exploration under explicit assumptions

A stochastic $k$-armed bandit has $k$ actions, or arms. Each arm $i$ has a stationary reward distribution with mean $\mu_i$. At round $t$, the policy chooses $A_t$ using past observations, receives a reward $X_t$ from the selected arm, and does not observe the rewards that unselected arms would have produced. The optimal mean is $\mu^*=\max_i\mu_i$ and the gap of arm $i$ is $\Delta_i=\mu^*-\mu_i$. The expected cumulative regret through $n$ rounds is

$$
R_n=n\mu^*-E\left[\sum_{t=1}^n X_t\right]
=\sum_i\Delta_i E[T_i(n)],
$$

where $T_i(n)$ counts how often arm $i$ was selected. Regret is a measure of reward forgone while learning and acting; it is not the same as the probability of naming the best arm at the end.

For an arm with $T_i$ observations and empirical mean $\hat\mu_i$, a simple upper confidence index has the form

$$
U_i=\hat\mu_i+\sqrt{\frac{2\sigma^2\log(1/\delta)}{T_i}},
$$

when the centered reward noise is $\sigma$-subgaussian and the confidence event has been made valid for the relevant comparisons. If an arm has not been tried, its index is set to $+\infty$ or the policy explicitly samples each arm once. UCB chooses the arm with the largest index. A high index can come from a high observed mean or from a wide uncertainty bonus due to few samples.

A subgaussian variable is one whose moment-generating function has Gaussian-like upper growth: for centered $X$, $E[e^{\lambda X}]\le e^{\lambda^2\sigma^2/2}$ for all real $\lambda$. Independent subgaussian observations give an exponential tail bound for their sample mean. Bounded rewards are subgaussian with a scale related to their range, but finite variance alone does not imply the same tail behavior. Heavy-tailed observations need different robust estimators or concentration assumptions. A confidence formula used outside its distributional assumptions can be overconfident.

Here is a two-arm index calculation. Let $\delta=0.05$ and use the simple radius above with $\sigma=1$. Arm A has observations $1,1,0,1$, so $T_A=4$ and $\hat\mu_A=0.75$. Its bonus is $\sqrt{2\log(20)/4}\approx1.225$ and its index is about $1.975$. Arm B has one observation $0$, so $T_B=1$, $\hat\mu_B=0$, bonus $\sqrt{2\log(20)}\approx2.448$, and index $2.448$. Even though A's observed mean is higher, the policy selects B because its uncertainty is larger. After additional samples from B, the bonus shrinks and the evidence can favor A. The numerical radius is deliberately conservative and the two-arm example does not prove a full UCB guarantee.

There is a technical wrinkle: the number $T_i(t)$ of samples from an arm is random and chosen using past rewards. A fixed-sample concentration inequality cannot simply be applied once and assumed valid at every random time. Standard bandit analyses handle this by reasoning about the pre-generated reward sequence for each arm and taking a union bound over possible sample counts, or by using a time-uniform confidence sequence. The confidence level must also account for repeated decisions and arms. A single-look 95% interval does not automatically remain a 95% guarantee after hundreds of adaptive looks.

UCB-style analysis often proves that a suboptimal arm is played only while either its own empirical mean is unusually high or the optimal arm's index is unusually low. Once enough observations reduce the suboptimal arm's bonus, it should stop beating the optimal arm's index except on a low-probability event. The regret decomposition then converts expected suboptimal pulls into reward loss. The assumptions include a fixed reward distribution per arm, an appropriate tail bound, and a defined horizon or a valid horizon-free variant. When those assumptions change, the bound must change too.

For Bernoulli rewards, the variance depends on the mean. If rewards are near zero or one, a variance-aware confidence bound can be much tighter than a generic range-based bound. KL-UCB uses the Bernoulli relative entropy

$$
d(p,q)=p\log\frac pq+(1-p)\log\frac{1-p}{1-q}
$$

to define an upper confidence endpoint by solving $T_i\,d(\hat\mu_i,q)\le\log f(t)$ for the largest plausible $q$. Because this divergence is asymmetric, the interval width can differ above and below the observed mean. Such an improvement is earned by the Bernoulli model; it should not be applied to continuous HDR pixel values or arbitrary engine rewards without the corresponding likelihood model.

### Cumulative regret and best-arm identification are different goals

Suppose a diagnostic system can spend a fixed budget of 100 measurements deciding which of two sensors is more accurate. A cumulative-reward objective would reward it for repeatedly using the sensor with better expected performance while it learns. A best-arm identification objective instead rewards correctly naming the better sensor after exploration. The second may rationally sample both arms even near the end; the first may exploit one arm to harvest reward. The optimal allocation need not be the same.

Best-arm identification can be fixed confidence: stop at a data-dependent time and return an arm with error probability at most $\delta$, while trying to minimize the expected number of samples. Or it can be fixed budget: after $n$ samples, minimize the probability that the selected arm is suboptimal. Fixed-confidence algorithms such as track-and-stop combine an allocation rule with a stopping statistic that accumulates evidence against alternatives. Fixed-budget algorithms such as sequential halving repeatedly sample active arms and eliminate the lower half. These are pure-exploration tasks: there is no reward for early exploitation except insofar as it yields information.

The confidence level $\delta$ controls a probability of error under the specified environment class, not the probability that an arbitrary answer is correct in every possible world. A guarantee for Gaussian unit-variance arms does not automatically apply to skewed, drifting rewards or dependent observations. The stopping rule and allocation jointly determine the guarantee. If several arms are exactly tied, some formulations cannot stop with finite expected samples while retaining a uniform soundness guarantee over a rich class, because an arbitrarily small change could make another arm uniquely best.

Choosing among rendering strategies can be a pure-exploration task if the final output is a decision, such as which of two sampling distributions has lower estimated variance for a target class. But optimizing the decision criterion is not the same as estimating the final image without bias. A policy can correctly learn which strategy usually reduces a chosen loss while its output estimator remains biased due to incorrect weights, missing support, clamping, or data-dependent termination. The decision's utility and the image estimator's mathematical target must be specified separately.


### Adversarial bandits replace a reward distribution with a sequence and force randomization

The UCB discussion above assumes each arm has a stable reward law. *Bandit Algorithms* develops a different model in which the environment is represented by a sequence of reward vectors

\[
x_1,\ldots,x_n,\qquad x_t\in[0,1]^k,
\]

without assuming that arm \(i\) is repeatedly sampled from one fixed distribution. At round \(t\), the learner chooses a distribution \(P_t\), samples an action \(A_t\sim P_t\), and sees only \(x_{t,A_t}\). Performance is compared with the best **fixed** action in hindsight:

\[
R_n
=
\max_{i\in[k]}
\sum_{t=1}^{n}x_{t,i}
-
\mathbb E\!\left[
\sum_{t=1}^{n}x_{t,A_t}
\right].
\]

The comparator matters. Low adversarial regret does not mean the learner tracks an arbitrarily changing best arm; that stronger target needs a switching/nonstationary comparator.

Because unplayed rewards are hidden, the learner constructs an importance-weighted estimate. For an observed reward \(X_t=x_{t,A_t}\), a basic estimator for arm \(i\) is

\[
\hat x_{t,i}
=
\frac{\mathbf 1\{A_t=i\}X_t}{P_{t,i}}.
\]

Conditioned on the past and the current reward vector,

\[
\mathbb E[\hat x_{t,i}]
=
x_{t,i},
\]

provided \(P_{t,i}>0\). The price is variance: when \(P_{t,i}\) is tiny, \(1/P_{t,i}\) can be huge. **Exploration is therefore also a variance-control mechanism.** A policy that drives an arm’s probability essentially to zero loses reliable information about that arm.

Exp3 combines this estimator with exponential weighting. Cumulative estimated reward \( \hat S_{t-1,i} \) is mapped to a probability roughly proportional to

\[
\exp(\eta\hat S_{t-1,i}),
\]

with learning rate \(\eta\). Large \(\eta\) commits aggressively to the apparent leader; small \(\eta\) stays closer to uniform. In the bounded adversarial model, an appropriate choice gives the characteristic regret scale

\[
R_n=O(\sqrt{nk\log k}).
\]

The theorem is not “Exp3 works on arbitrary reality.” It assumes a particular feedback protocol, reward range, comparator, and adversary model. If the environment reacts after seeing the learner’s current random action in a prohibited way, or if rewards are unbounded/heavy-tailed without adaptation, the proof no longer applies as stated.

This model is valuable for engine experiments when rewards are not plausibly IID. Choosing among cache policies, sampling strategies, or online heuristics can change future workloads. An adversarial/no-regret view may be more honest than inventing a stationary arm mean. But the reward must still measure the real objective, and importance weighting must preserve support.

Source basis: Lattimore and Szepesvári, *Bandit Algorithms*, Chapters 11–12 on adversarial bandits, importance-weighted estimators, Exp3/Exp3-IX, and regret analysis.

### Context and linear structure let one observation teach us about more than one action

A contextual bandit observes a context \(c_t\) **before** choosing the action. The benchmark is no longer necessarily the best one global arm; it can be a policy mapping contexts to actions. If every possible context is treated as an unrelated problem, data fragment badly. The useful question is what structure lets experience transfer across contexts.

The expert-advice formulation represents candidate policies as experts. At each round, expert \(m\) provides a distribution over actions. Exp4 maintains weights over experts and mixes their recommendations, using bandit feedback and importance weighting to update the experts indirectly. This makes the comparator “best expert in hindsight” rather than “best arm.” The framework is powerful precisely because the internal form of an expert can be opaque to the bandit algorithm.

A **linear bandit** assumes more explicit structure. Each action is represented by a feature vector \(a\in\mathbb R^d\), and expected reward is

\[
\mathbb E[X_t\mid A_t=a]
=
\langle \theta_*,a\rangle
\]

for an unknown parameter \(\theta_*\). One observation informs every action whose features overlap the measured direction. A regularized least-squares estimate is

\[
\hat\theta_t
=
V_t^{-1}
\sum_{s=1}^{t}A_sX_s,
\qquad
V_t
=
\lambda I+\sum_{s=1}^{t}A_sA_s^T.
\]

The matrix \(V_t\) is not just bookkeeping. It records **which directions of parameter space have been excited by the chosen actions**. Directions with little action energy remain uncertain.

Optimism now operates over a confidence **set** rather than an interval. If \(C_t\) is a high-probability set containing \(\theta_*\), define

\[
\operatorname{UCB}_t(a)
=
\max_{\theta\in C_t}
\langle\theta,a\rangle,
\]

and choose the action with largest optimistic value. For ellipsoidal confidence sets this becomes a mean prediction plus a geometry-dependent uncertainty bonus involving \(V_t^{-1}\). An action can be attractive because its predicted reward is high or because it probes a weakly measured direction.

The adaptive design makes confidence analysis subtle. \(A_t\) depends on earlier rewards, so ordinary fixed-design least-squares intervals cannot simply be copied into the bandit loop. The book develops martingale/self-normalized tools precisely because the design matrix is data-dependent. This is a recurring lesson in active vision and adaptive rendering: once the algorithm chooses where to measure based on previous measurements, uncertainty calculations must cover that adaptivity.

A graphics example is choosing among candidate ray bundles whose feature vector describes depth variance, normal variation, material roughness, and prior sample count. A global arm mean throws away that structure. A linear model can share information across similar bundles, while its confidence geometry reveals which feature combinations have not been explored. If reward is nonlinear in those features, however, the linear-bandit guarantee is misspecified rather than merely conservative.

Source basis: *Bandit Algorithms*, Chapters 18–20 on contextual bandits, expert advice/Exp4, stochastic linear bandits, regularized least squares, optimism over confidence sets, and adaptive least-squares confidence bounds.

### Thompson sampling explores by sampling a plausible world, not by adding an optimism bonus

In a Bayesian bandit model, uncertainty is represented by a posterior distribution over the unknown environment. **Thompson sampling** draws a plausible parameter/environment from that posterior and plays an action optimal for the sampled world. In a finite-armed form, an equivalent description is that an arm is selected with probability equal to its posterior probability of being optimal.

For a Bernoulli arm with a Beta prior,

\[
\mu_i\sim\operatorname{Beta}(\alpha_i,\beta_i),
\]

success/failure observations update the posterior analytically. A Thompson step samples

\[
\tilde\mu_i\sim
\operatorname{Beta}(\alpha_i+s_i,\beta_i+f_i)
\]

for each arm and selects \(\arg\max_i\tilde\mu_i\). Arms with uncertain posteriors sometimes produce optimistic samples and are explored automatically; concentrated clearly inferior posteriors are sampled less often.

This should not be confused with “Bayesian means prior choice does not matter after enough data.” In a sequential bandit, an overly pessimistic prior can suppress an arm, which prevents collecting the data that would correct that pessimism. The feedback loop couples inference and data acquisition. Lattimore and Szepesvári explicitly warn that Thompson sampling is not a universal silver bullet; optimality depends on the model class and prior/algorithm assumptions.

For linear contextual problems, posterior sampling can be performed over the parameter vector. Under a Gaussian model, one samples a plausible \(\tilde\theta_t\) from a covariance shaped by accumulated design information and chooses the action maximizing \(\langle \tilde\theta_t,a\rangle\). This makes the connection to covariance geometry concrete: exploration happens in directions where the posterior remains wide.

The practical distinction among UCB, Exp3, and Thompson sampling is therefore not cosmetic. UCB constructs an explicit confidence-based optimistic bound under a stochastic model. Exp3 uses randomized exponential weighting and unbiased partial-feedback estimates to compete with a fixed action in an adversarial sequence. Thompson sampling samples a plausible environment from a Bayesian posterior. They can choose similar actions on one trace while carrying different correctness claims.

Source basis: *Bandit Algorithms*, Chapters 34–36 on Bayesian learning, Bayesian bandits, posterior distributions, Thompson sampling, Bayesian regret, and information-theoretic analysis.

### A bandit becomes a sequential-control problem when actions change what is observed next

A standard bandit reward for arm \(i\) does not require an evolving state whose transition depends on the chosen action. Many engine and robotics decisions do. Moving the camera changes the next viewpoint; a navigation action changes the next location; an allocation decision changes cache state; a control impulse changes future dynamics. The appropriate abstraction is a **Markov decision process** rather than a stateless arm table.

An MDP defines states \(S_t\), actions \(A_t\), transition probabilities

\[
P_a(s,s')
=
P(S_{t+1}=s'\mid S_t=s,A_t=a),
\]

and expected rewards \(r_a(s)\). A policy selects actions from state. The Markov property says the modeled current state contains the information needed for the next-state law; it does not say the real world has no memory. If crucial history is omitted from \(S_t\), the model is not Markov with respect to that state representation.

For a discounted finite MDP, a familiar Bellman optimality equation is

\[
V^*(s)
=
\max_a
\left[
r(s,a)
+
\gamma
\sum_{s'}
P(s'|s,a)V^*(s')
\right].
\]

Lattimore and Szepesvári’s final chapter develops the average-reward strongly connected setting, where an optimal gain and a relative value/bias function replace the simple discounted form. The exact Bellman equation therefore depends on the objective. The structural idea is the same: optimal value satisfies a fixed-point relation combining immediate reward with the value of successor states.

When \(P\) and \(r\) are unknown, reinforcement learning must explore **state–action dynamics**, not just reward means. Optimism can extend by constructing plausible MDPs or confidence regions for transitions/rewards and acting according to an optimistic model. The difficulty is now that poor exploration can prevent visiting entire state regions. A one-state MDP with action-dependent rewards collapses toward the bandit case; adding state introduces temporal credit and reachability.

This boundary is important for adaptive engine systems. If choosing a denoiser only affects the current frame score, a bandit may suffice. If it changes a temporal history buffer that affects future frames, the reward process has state. If choosing a collision broad-phase rebuild policy changes the structure and cost of future queries, current action and future workload are coupled. Modeling that coupling explicitly is better than forcing history-dependent rewards into a stationary-arm assumption.

Source basis: *Bandit Algorithms*, Chapter 38 on Markov decision processes, optimal policies, the Bellman optimality equation, learning in MDPs, and upper-confidence methods for reinforcement learning.

### Stationarity and reproducibility are part of the experiment

A stationary bandit assumes that arm $i$ keeps the same reward distribution over time. A renderer that changes scene lighting mid-run, a sensor whose calibration drifts, or a vision stream with seasonal domain shifts violates stationarity. A standard UCB algorithm may then retain old observations that no longer predict current rewards. A useful alternative may be a sliding window, discounting, change detection, or a comparator that is allowed to switch arms. Each addresses a different drift model and adds its own assumptions.

For an environment with at most $m-1$ change points, tracking regret compares the learner to the best arm sequence with at most that many switches. Restarting UCB at known change points is conceptually simple; unknown change points require detecting change or forgetting old evidence. In a nonstationary world, “the best arm” is time-indexed and the comparator must be explicit. No-regret relative to one fixed arm can be a meaningless target if the optimal action changes halfway through the data.

Reproducibility requires more than a random seed. Record the scene and sensor version, data ordering, random-number generator and stream assignment, policy version, candidate set, stopping criterion, reward definition, and any rejected or missing observations. In adaptive experiments, also retain selection probabilities and decision history so weighted estimates can be audited. A fixed seed makes one run repeatable; it does not make a dependent sample sequence IID or prove that uncertainty intervals are calibrated.

### A compute budget needs a reward that matches the real goal

A bandit can allocate an engine's limited query budget among actions, but “reward” must have a defined operational meaning. Possible choices include reduction in mean squared image error, reduction in a task-specific detection loss, information gain about a parameter, improvement in expected frame time, or a utility that balances image quality against latency. These objectives can rank actions differently. A perceptual quality metric may reward edge detail differently from radiometric accuracy. A query that helps a collision classification may have no value to the final rendered image.

To use a bandit model, specify the actions, context, observable reward, horizon, and whether action rewards are stationary and conditionally independent. If the same query is evaluated by re-rendering with a fresh random seed, one can estimate its expected utility; if action choice changes the future data or cache state, the environment is structured and history-dependent. If context changes with the pixel, object, or time step, a contextual bandit may be more appropriate than a single global arm mean. If the metric is observed only for the selected action, counterfactual reward estimation needs support and propensity correction.

Most importantly, adaptive compute allocation does not automatically preserve an unbiased image estimate. Suppose pixels are divided into regions and a controller samples high-variance regions more often. At a predetermined sample count, if each correctly weighted contribution has conditional expectation equal to the target given the past, averaging those contributions is unbiased by iterated expectation. A random per-pixel stopping count requires an additional argument; the conditional-mean property alone does not make the stopped sample average unbiased. If the controller instead oversamples bright regions and pools all raw values as though they came from the target distribution, it estimates the proposal-weighted image, not the target image. If it stops each pixel when a noisy confidence interval looks narrow, fixed-budget error formulas may no longer apply. If it clamps rare high contributions, it trades bias for variance reduction.

A safer architecture separates two contracts. The estimator contract states the quantity being estimated, sampling distribution, weights, support conditions, stopping rule, and uncertainty calculation. The controller contract states how compute is allocated according to observed information and which utility it tries to improve. The controller can be adaptive while the estimator remains unbiased, but that requires a proof or a valid sequential estimator—not an assumption that “more samples where needed” is harmless. Online optimization and statistical correctness are connected, yet they are not identical problems.

### An experiment design checklist in the language of science

Before running an adaptive rendering, vision, or engine experiment, write down the target quantity $\theta$ and the population or scene distribution over which it is defined. State which variables are random and which are fixed unknown parameters. Describe how one observation is sampled, whether observations are independent, and what data are missing when an action is not chosen. Decide whether the goal is estimating a mean, reducing a variance, making a one-time choice, or repeatedly earning reward.

Then name the estimator and its finite-sample expectation. List the assumptions used for its variance and confidence interval. If there is a confidence interval, say whether it is exact, asymptotic, or a concentration bound, and whether the coverage is pointwise or uniform over parameters. If the policy adaptively changes samples or stops, use an analysis that covers those decisions. If importance weights are used, check support and weight tails. If the environment drifts, define a nonstationary comparator or a change model. Finally, log enough information to replay and audit the choices.

These steps are not paperwork around the algorithm. They define the result. “The estimate is unbiased” means its expected value equals the specified target under the specified law. “The interval is 95%” means a named coverage or posterior probability statement under a named model. “The policy learns the best action” means a particular regret or identification result under assumptions about reward distributions and observations. When those sentences are made precise, uncertainty becomes a usable engineering quantity: it tells us what evidence we have, what remains unknown, and what an additional query can legitimately improve.

### References for deeper source checks

- Larry Wasserman, *All of Statistics*, Chapters 1–8 and 11, for probability foundations, estimation, concentration, convergence, intervals, bootstrap, and Bayesian interpretation.
- Tor Lattimore and Csaba Szepesvári, *Bandit Algorithms*, Chapters 4–7, 10–11, 31, and 33, for stochastic bandit assumptions, confidence-based exploration, importance weighting, nonstationarity, and pure exploration.
- Eric Veach, *Robust Monte Carlo Methods for Light Transport Simulation* (1997 dissertation), especially the Monte Carlo integration chapter, for unbiasedness, importance sampling, adaptive sample placement, and rendering estimators: [Stanford dissertation](https://graphics.stanford.edu/papers/veach_thesis/).
- Peter Auer, Nicolò Cesa-Bianchi, and Paul Fischer, “Finite-time Analysis of the Multiarmed Bandit Problem” (2002), for a foundational finite-time UCB analysis: [publication record](https://www2.compute.dtu.dk/pubdb/pubs/2088-full.html).
- William R. Thompson, “On the Likelihood That One Unknown Probability Exceeds Another in View of the Evidence of Two Samples” (1933), an early sequential allocation treatment: [Biometrika record](https://academic.oup.com/biomet/article-abstract/25/3-4/285/200862).

### A worked image-budget example: allocate samples without changing the target

Imagine an image simplified to two equally important regions, A and B. Their true mean radiances are $0.2$ and $0.8$, so the target whole-image mean is $(0.2+0.8)/2=0.5$. A compute controller estimates that B is more interesting and samples region A with probability $q_A=0.1$ and region B with $q_B=0.9$. If it simply averages observed radiances, its expected output is $0.1(0.2)+0.9(0.8)=0.74$. This is not noisy around the target $0.5$; it converges to the wrong target because the sampling policy changed the region mixture.

The desired region distribution is $p_A=p_B=0.5$. An importance weight for an observation in region $j$ is $p_j/q_j$. Thus, an A observation is weighted by $0.5/0.1=5$, and a B observation by $0.5/0.9\approx0.556$. The one-sample weighted random variable is $Y=1.0$ with probability $0.1$ and $Y\approx0.444$ with probability $0.9$. Its expectation is $0.1(1.0)+0.9(0.444)\approx0.5$. The estimator recovers the original whole-image mean despite allocating most samples to B. This example assumes each region's value is fixed and observed without additional noise; a real renderer has within-region path variance as well, which contributes extra variance.

A stratified design can avoid some of the weight instability by reserving samples in each region and combining regional estimates with the known pixel-area weights: $\hat I=0.5\bar X_A+0.5\bar X_B$. With at least one correctly sampled observation from each region and unbiased regional sample means, the combined estimate is unbiased regardless of the unequal sample counts. Its variance is $0.25\operatorname{Var}(\bar X_A)+0.25\operatorname{Var}(\bar X_B)$ if the region samples are independent. Spending more samples in a region with larger variance can reduce the total variance efficiently, but the sample allocation itself must be included if it depends on observed values. Keeping a small exploration floor also prevents the controller from permanently abandoning a region whose early samples happened to look quiet.

This makes the roles clear. The estimator decides how observations represent the target image. The controller decides where another observation is worth its cost. A controller may use a bandit to learn which region tends to yield more error reduction per unit time, while the final pixel estimate still uses area-correct weights. If reward is defined as immediate reduction in a perceptual loss, however, that reward may be noisy, nonstationary, and dependent on prior denoising state. UCB's guarantee is about maximizing cumulative expected reward under its bandit model; it does not imply a radiometrically unbiased image, a calibrated pixelwise interval, or a globally optimal perceptual image.

The same separation applies in vision. If a system selects uncertain image patches for labeling, it can use the resulting data to improve a classifier, but naïvely averaging only the selected labels estimates the selected-patch distribution. Population-level metrics require either representative sampling, known inclusion probabilities with valid weights, or a justified model-based adjustment. If an engine prioritizes collision checks near suspected contacts, the hit rate among queried pairs is not the scene-wide collision rate. Active computation can improve decisions while making naïve descriptive summaries less representative.

A final nuance is that a reward itself can be estimated. Suppose action A costs 1 ms and reduces estimated error by $0.02$ with high variance, while action B costs 3 ms and reduces it by $0.05$ with low variance. “Maximize error reduction” selects B by mean reward; “maximize reduction per millisecond” may select A; “minimize error subject to a frame-time cap” is a constrained objective. If actions alter caches or future variance, their reward depends on history and may not be stationary. Naming the utility and horizon is part of the scientific model, not a presentation detail.

For reproducibility, preserve at least a replayable trace of chosen actions, conditional selection probabilities, reward observations, sample counts, stopping times, and estimator weights. With this trace, one can separately check the controller's decisions and recompute the estimator. A seed alone cannot reveal whether support was missing, whether confidence was repeatedly checked, or whether a renderer's next sample distribution depended on a previous noisy estimate.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/30 - Monte Carlo Estimates and Uncertainty Metrics|Next chapter]]
