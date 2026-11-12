# Metric gradients and differential measurements

The differential of a scalar function is a linear measurement of displacement. The gradient is the vector that represents that measurement after an inner product or metric has been chosen.

At a point, let coordinate derivative coefficients be collected in $g$. Then

$$
df[h]=g^{\mathsf T}h.
$$

If the vector-space inner product is

$$
\langle u,h\rangle_G=u^{\mathsf T}Gh
$$

for symmetric positive-definite metric matrix $G$, the metric gradient is defined by

$$
df[h]=\langle \operatorname{grad}_G f,h\rangle_G
$$

for every displacement $h$. Therefore

$$
G\operatorname{grad}_G f=g,
\qquad
\operatorname{grad}_G f=G^{-1}g.
$$

In ordinary orthonormal Euclidean coordinates $G=I$, so derivative coefficients and gradient-vector coordinates happen to match. That familiar coincidence hides the conceptual distinction.

Example: for $f(x,y)=x+y$ and

$$
G=\begin{bmatrix}1&0\\0&4\end{bmatrix},
$$

the derivative coefficients are $g=(1,1)$ but

$$
\operatorname{grad}_G f=(1,1/4).
$$

The steepest direction depends on the ruler used to constrain step length. This is not merely abstract geometry: weighted optimization, pose parameters mixing translation and rotation, and preconditioned numerical methods all choose an effective metric.

The maximum first-order change over the metric unit ball is the dual norm of the differential:

$$
\max_{h^{\mathsf T}Gh\le1} g^{\mathsf T}h
=
\sqrt{g^{\mathsf T}G^{-1}g}.
$$

So metric gradients and dual-norm error bounds are two views of the same measurement-versus-vector relationship.

Full derivation and worked trace: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A derivative measures change; a gradient needs a ruler]].

Related: [[Norms, dual norms and error budgets]], [[Jacobians, pushforwards, and pullbacks]], [[Hessians and local curvature]].
