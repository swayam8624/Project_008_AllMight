# Bandit exploration and regret

A multi-armed bandit models sequential choices when only the reward of the chosen action is observed. It separates exploitation—choosing what currently looks best—from exploration—choosing an uncertain action to learn about it.

Let arm $a$ have unknown mean reward $\mu_a$. Over $T$ rounds, one common stationary pseudo-regret benchmark is

$$
R_T=T\mu^*-\sum_{t=1}^{T}\mu_{A_t},
$$

where $\mu^*$ is the largest fixed-arm mean and $A_t$ is the chosen arm at round $t$.

Regret is meaningful only relative to the benchmark and environment assumptions. In a nonstationary problem, “best fixed arm” may be the wrong target.

## UCB intuition

Upper-confidence-bound methods choose an action using an optimistic estimate such as

$$
\hat\mu_a+
c\sqrt{\frac{\log t}{N_a}},
$$

where $N_a$ is the number of samples from arm $a$. The uncertainty bonus shrinks with evidence and is larger for rarely tried arms.

The derivation of a confidence radius depends on reward bounds/noise assumptions. Reusing a textbook radius for arbitrary unbounded timing noise or correlated frame measurements removes the theorem that motivated it.

## Nonstationary workloads

Render cost, network latency, cache behavior and user demand can change. An all-history mean adapts slowly after a regime shift. Sliding-window estimates, exponential discounting or explicit change detection trade statistical efficiency in stationary periods for responsiveness.

## Adaptive sampling can change an estimator

Choosing what to measure next based on observed rewards is useful for experimental efficiency, but it can make a naive final average biased toward frequently selected outcomes. Adaptive experiment allocation and unbiased Monte Carlo integration are different problems.

When selection probabilities are known, importance weighting can correct some adaptive sampling schemes:

$$
\hat\mu=
\frac1T\sum_t
\frac{\mathbf 1[A_t=a]X_t}{p_t(a)}.
$$

Very small selection probabilities can make the variance enormous, so mathematical unbiasedness does not guarantee a practical estimator.

## Graphics/research use

Bandits are plausible for choosing which configuration to benchmark, which reconstruction region deserves another expensive measurement, or which experiment to run next. They should not be inserted into a pixel estimator on the assumption that maximizing a reward signal automatically preserves the target rendering integral.

The direct mathematics expansion is at [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choose the next measurement using the geometry of what is still unknown]]. The graphics narrative connects adaptive computation at [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From Random Samples to Deliberate Computation]].

Related: [[Confidence intervals and sequential evidence]], [[Inverse, rejection, and importance sampling]], [[Sampling support and missing strata]].
