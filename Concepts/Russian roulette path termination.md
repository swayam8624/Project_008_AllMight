# Russian roulette path termination

Terminate a path probabilistically to reduce expected work. Divide surviving throughput by its conditional survival probability to retain the continuation expectation. This does not remove bias from a separate hard bounce cap.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Follow one path without losing its probability]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — random termination with compensation

Russian roulette terminates low-contribution paths probabilistically to control expected work without imposing a fixed depth bias. Suppose a path survives with probability (p). If it survives, divide its future throughput by (p). Then the expected contribution remains unchanged:

[
E[Y]=p\left(\frac{X}{p}\right)+(1-p)0=X.
]

The survival probability must be chosen from information available at the termination decision and bounded away from pathological values. Using raw maximum throughput is common, but the exact policy is an implementation choice and can affect variance dramatically.

Do not apply roulette so early that short important paths receive needless variance. Do not forget the compensation factor. Do not let NaN/negative throughput produce invalid probabilities. In spectral or RGB renderers, document whether the decision is shared across channels and how the scalar probability is derived.

Russian roulette changes the distribution of work and noise, not the underlying transport equation.

**Required next witness:** deterministic-seed comparisons of fixed-depth truncation versus compensated roulette, including a scene where fixed truncation visibly biases the answer.

