# Multiple importance sampling

Multiple importance sampling combines estimators from several sampling techniques without pretending that one proposal distribution is good everywhere.

Suppose two techniques generate the same contribution $f(x)$ using PDFs $p_a(x)$ and $p_b(x)$. With one sample from each technique, an MIS estimator can use weights $w_a(x)$ and $w_b(x)$ satisfying appropriate partition and support conditions. The balance heuristic uses

$$
w_a(x)=\frac{n_a p_a(x)}{n_a p_a(x)+n_b p_b(x)},
$$

with the symmetric definition for $w_b$. For one sample from each, $n_a=n_b=1$.

The key idea is not “average two answers.” The estimator must evaluate the PDFs in the **same measure** for the same sampled event. Mixing a solid-angle PDF with an area PDF without the Jacobian conversion makes the weights meaningless.

## Support matters

If technique A can sample a contribution that B cannot, then $p_b(x)=0$ at that event and A should receive full weight. But if the implementation accidentally reports zero because it evaluated the wrong lobe, wrong hemisphere, or wrong light-selection probability, MIS can hide a bug behind a numerically finite result.

For direct lighting, the full light-sampling PDF may include both discrete light selection and continuous point/direction sampling:

$$
p_{light}=p(\text{choose light})\,p(\text{sample point or direction}\mid\text{light}).
$$

The BSDF PDF must be converted to the same directional measure before comparison.

## Power heuristic

A common alternative is

$$
w_a=
\frac{(n_a p_a)^\beta}
{(n_a p_a)^\beta+(n_b p_b)^\beta},
$$

often with $\beta=2$. This suppresses a technique more aggressively when its PDF is much smaller than the competing proposal.

The heuristic changes variance, not the target integral, when the estimator is constructed correctly.

## Executable reference

tools/labs/monte_carlo_transport_lab.cpp checks the simple balance-weight partition property and separately verifies cosine-hemisphere transport and Russian-roulette expectation compensation. It is a mathematical estimator witness, not yet a complete path tracer with next-event estimation and scene lights.

Read the connected derivation at [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Follow one path without losing its probability]].

Related: [[Russian roulette path termination]], [[Inverse, rejection, and importance sampling]], [[Sampling support and missing strata]].
