# Russian roulette path termination

Russian roulette reduces expected path length without deterministically deleting every long path. At a continuation point, choose a survival probability $p\in(0,1]$. If the path terminates, its future contribution is zero. If it survives, divide the surviving throughput by $p$.

For a continuation contribution $X$,

$$
Y=
\begin{cases}
X/p,&\text{with probability }p,\\
0,&\text{with probability }1-p.
\end{cases}
$$

Then

$$
\mathbb E[Y]
=
p\frac{X}{p}+(1-p)0
=
X.
$$

The compensation preserves the continuation expectation. It does not guarantee lower variance, and it does not undo bias from a separate hard bounce cap.

## Choosing the survival probability

A common design bases $p$ on remaining throughput magnitude, usually after a minimum depth. But the exact scalar used—maximum channel, luminance-like measure, spectral bound—changes variance and path behavior. Clamp policies must be documented.

A very small $p$ creates rare, very large weights $1/p$, which can increase variance dramatically. A $p$ too close to one saves little work. The point is an efficiency trade-off, not a ritual after N bounces.

## Conditional probability matters

The compensation uses the probability of surviving **at that decision**, conditioned on the path state that reached it. If the probability changes per bounce, multiply the path throughput by the reciprocal of each actual survival probability encountered. Do not divide later by a generic average probability.

## Executable reference

tools/labs/monte_carlo_transport_lab.cpp runs one million deterministic-seed roulette trials and verifies that the sample mean remains close to the un-terminated contribution after survival compensation.

That does not prove a complete renderer. It isolates the stochastic invariant so a path tracer can test roulette independently of geometry, BSDF and light-sampling bugs.

Read the connected derivation at [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Follow one path without losing its probability]].

Related: [[Multiple importance sampling]], [[Reproducible numerical benchmarking]].
