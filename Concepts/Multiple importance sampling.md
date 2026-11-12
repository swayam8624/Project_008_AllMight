# Multiple importance sampling

MIS combines estimators from multiple sampling techniques using compatible measures and complementary weights. Computing a weight for only one estimator while omitting its needed counterpart generally changes the expected answer.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Follow one path without losing its probability]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — combine estimators, not arbitrary weights

Multiple importance sampling combines samples drawn from different proposal distributions when each proposal is effective in different regions. In direct lighting, a BSDF sampler may find sharp specular directions well while a light sampler efficiently finds a small emitter.

If technique (i) draws from density (p_i(x)) and contributes estimator (f(x)/p_i(x)), MIS assigns a weight (w_i(x)) while preserving the condition that the weights over participating techniques \sum to one on the supported sample.

The balance heuristic is

$$
w_i(x)=\frac{n_i p_i(x)}{\sum_j n_j p_j(x)},
$$

and the power heuristic commonly squares the weighted densities. The densities must be expressed with respect to the **same measure** before comparison. Comparing an area PDF directly to a solid-angle PDF is a dimensional error.

Support also matters. If one technique has zero density where another can sample, the weight logic must not divide by zero or erase the only valid contribution.

MIS reduces variance; it does not excuse a biased base estimator, incorrect BSDF, wrong visibility result or inconsistent PDF conversion.

**Required next witness:** a small path-tracing scene where light-only, BSDF-only and MIS estimates are compared against a high-sample reference with fixed seeds and convergence/error plots.

