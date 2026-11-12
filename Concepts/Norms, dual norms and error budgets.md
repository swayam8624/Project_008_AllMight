# Norms, dual norms and error budgets

A norm defines a size rule. Its dual norm measures the largest scalar response a linear measurement can produce over the unit ball of that norm.

For a covector represented by coefficients $a$,

$$
\|a\|_* = \max_{\|e\|\le 1} a^{\mathsf T}e,
$$

and therefore

$$
|a^{\mathsf T}e|\le \|a\|_*\,\|e\|.
$$

This turns an error budget into a sensitivity bound. The choice of norm matters because different “allowed error shapes” have different worst cases. A Euclidean ball is self-dual: the dual of $\ell_2$ is $\ell_2$. A componentwise box uses $\ell_\infty$ for the error and has $\ell_1$ as the dual measurement norm. An $\ell_1$ error budget has $\ell_\infty$ dual sensitivity.

Example: if $a=(2,-3)$ and $\|e\|_\infty\le0.01$, then

$$
|a^{\mathsf T}e|
\le (|2|+|{-3}|)0.01
=0.05.
$$

This is more informative than a generic epsilon because it says what perturbations are permitted and what scalar quantity is being protected.

Error budgets in graphics and simulation should also carry units. A state vector mixing metres, radians, exposure values and normalized texture coordinates does not acquire a physically meaningful Euclidean norm merely because the values are stored in one array. The metric or scaling must state what “one unit of error” means in each subspace.

Full derivation and worked trace: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Bound a measurement using a dual norm]].

Related: [[Metric gradients and differential measurements]], [[Forward and backward error]], [[Numerical error budgets]].
