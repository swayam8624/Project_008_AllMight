# 27 — Gauss–Newton for Geometry and Pose

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|Next chapter]]

Gauss–Newton replaces the exact Hessian with a residual-Jacobian approximation, then applies parameter updates to geometric models such as camera poses. The chapter connects convergence, rank, observability, and transform conventions to whether an optimization result is meaningful.

**By the end:** Derive a Gauss–Newton update from residual linearization and diagnose rank deficiency or an ambiguous pose estimate.

**Recall question:** What information is lost when the exact Hessian is replaced by the Gauss–Newton approximation?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 63466–66236; source-span SHA256 772924b006241043470e4545a6c5b98505cfff5ec8c0248249ab6902a2576065. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Gauss–Newton

Newton's method for least squares would solve:

$$
H_E\Delta\mathbf x=-\nabla E.
$$

But the exact Hessian contains second derivatives of every residual.

Gauss–Newton replaces it with:

$$
H\_{\mathrm{GN}}=J^TJ.
$$

The resulting step solves:

$$
\boxed{ J^TJ\Delta\mathbf x=-J^T\mathbf r }.
$$

##### Deriving Gauss–Newton from residual linearization

Near the current parameters $\mathbf x$:

$$
\mathbf r(\mathbf x+\Delta\mathbf x) \approx \mathbf r+J\Delta\mathbf x.
$$

Replace the nonlinear objective with:

$$
m(\Delta\mathbf x) = \frac12\|\mathbf r+J\Delta\mathbf x\|^2.
$$

Expand:

$$
m= \frac12\mathbf r^T\mathbf r +\mathbf r^TJ\Delta\mathbf x +\frac12\Delta\mathbf x^TJ^TJ\Delta\mathbf x.
$$

Differentiate with respect to the step:

$$
\nabla\_{\Delta\mathbf x}m = J^T\mathbf r+J^TJ\Delta\mathbf x.
$$

Set this to zero:

$$
\boxed{ J^TJ\Delta\mathbf x=-J^T\mathbf r }.
$$

So Gauss–Newton is not an arbitrary Hessian approximation. It is the exact minimizer of the first-order linearized residual least-squares model when the resulting normal system has a unique solution.

##### Worked nonlinear example

Consider:

$$
r(x)=x^2-4.
$$

We minimize:

$$
E(x)=\frac12(x^2-4)^2.
$$

The residual derivative is:

$$
J(x)=2x.
$$

At $x_0=1$:

$$
r(1)=-3,\qquad J(1)=2.
$$

The Gauss–Newton equation is:

$$
J^2\Delta x=-Jr.
$$

Substituting:

$$
4\Delta x=-2(-3)=6.
$$

Therefore:

$$
\boxed{\Delta x=1.5}.
$$

The new iterate is:

$$
x_1=2.5.
$$

But the true positive root is $x=2$.

Gauss–Newton overshoots because the residual is nonlinear and its linear model is only locally accurate.

At $x=2.5$:

$$
r=2.25,\qquad J=5.
$$

The next step solves:

$$
25\Delta x=-11.25,
$$

giving:

$$
\Delta x=-0.45.
$$

Thus:

$$
x_2=2.05.
$$

We are much closer.

Repeated steps can converge rapidly near a suitable solution.

##### Rank deficiency and observability

Suppose the residual Jacobian has a nontrivial nullspace:

$$
J\mathbf d=0
$$

for some $\mathbf d\ne0$.

Then moving in direction $\mathbf d$ has no first-order effect on the residuals.

Consequently:

$$
J^TJ\mathbf d=0.
$$

The normal matrix is singular.

This may happen when the data cannot uniquely determine certain parameters, when observations are redundant, or when there is a gauge freedom.

For example, in some structure-from-motion formulations, jointly transforming all cameras and all reconstructed points by an appropriate global similarity transformation leaves the image reprojections unchanged.

Without fixing the gauge, the problem can have directions that are physically unobservable from the measurements.

A damping term may make the linear algebra solvable, but it does not create missing information.

Numerical regularization and physical identifiability are different issues.

#### Levenberg–Marquardt

Gauss–Newton can produce excellent steps near a good solution. It can also fail far from the solution when residual linearization is inaccurate, or when $J^TJ$ is singular or nearly singular.

Levenberg–Marquardt, commonly abbreviated LM, introduces damping.

A common form is:

$$
\boxed{ (J^TJ+\lambda I)\Delta\mathbf x=-J^T\mathbf r }
$$

with $\lambda>0$.

A scaled form uses a positive-definite or suitable diagonal matrix $D$:

$$
\boxed{ (J^TJ+\lambda D)\Delta\mathbf x=-J^T\mathbf r. }
$$

##### Deriving LM as regularized local least squares

Start with the linearized residual model:

$$
m(\Delta\mathbf x) = \frac12\|\mathbf r+J\Delta\mathbf x\|^2.
$$

Add a penalty on the step:

$$
m\_\lambda(\Delta\mathbf x) = \frac12\|\mathbf r+J\Delta\mathbf x\|^2 +\frac\lambda2\Delta\mathbf x^TD\Delta\mathbf x.
$$

Differentiate:

$$
\nabla m\_\lambda= J^T(\mathbf r+J\Delta\mathbf x) +\lambda D\Delta\mathbf x.
$$

Setting the gradient to zero:

$$
J^T\mathbf r+ J^TJ\Delta\mathbf x+ \lambda D\Delta\mathbf x=0.
$$

Therefore:

$$
\boxed{ (J^TJ+\lambda D)\Delta\mathbf x=-J^T\mathbf r }.
$$

LM balances residual improvement against the magnitude of the proposed parameter step.

##### Small damping versus large damping

When $\lambda$ is small:

$$
J^TJ+\lambda I\approx J^TJ.
$$

The step resembles Gauss–Newton.

When $\lambda$ is very large:

$$
J^TJ+\lambda I\approx\lambda I.
$$

Then:

$$
\Delta\mathbf x\approx-\frac1\lambda J^T\mathbf r.
$$

Since $J^T\mathbf r=\nabla E$, the step resembles a small gradient-descent update.

This gives LM a useful adaptive behavior.

Small damping trusts the local curvature model.

Large damping makes the update more conservative.

##### Worked scalar example

Let:

$$
r(x)=x^2-4.
$$

Start far from the positive root:

$$
x_0=0.1.
$$

Then:

$$
r=-3.99,\qquad J=0.2.
$$

The Gauss–Newton equation gives:

$$
(0.2)^2\Delta x=-(0.2)(-3.99).
$$

Thus:

$$
0.04\Delta x=0.798,
$$

so:

$$
\boxed{\Delta x=19.95}.
$$

The candidate would jump to $20.05$, where the residual is enormous.

Now introduce LM damping $\lambda=1$:

$$
(0.04+1)\Delta x=0.798.
$$

Therefore:

$$
\boxed{ \Delta x\approx0.767307692 }.
$$

The candidate is:

$$
x_1\approx0.867307692.
$$

This is still not the solution, but it avoids the catastrophic Gauss–Newton step.

The algorithm can evaluate whether the candidate actually improves the objective and adjust $\lambda$ accordingly.

##### Damping updates

A simple LM strategy reduces $\lambda$ after an accepted improving step and increases it after a rejected step.

A more principled trust-region implementation uses actual-versus-predicted reduction to adjust damping.

Suppose the undamped quadratic model predicts:

$$
m(0)-m(\Delta\mathbf x) = -\mathbf g^T\Delta\mathbf x -\frac12\Delta\mathbf x^TJ^TJ\Delta\mathbf x.
$$

The ratio:

$$
\rho= \frac{ E(\mathbf x)-E(\mathbf x+\Delta\mathbf x) }{ m(0)-m(\Delta\mathbf x) }
$$

indicates how accurately the local model predicted the accepted or rejected step.

A poor ratio suggests increasing damping or shrinking the trust region.

A good ratio permits more aggressive updates.

##### Why LM is widely used in geometric reconstruction

Camera calibration, bundle adjustment, pose fitting, nonlinear curve fitting, and many inverse problems naturally produce residual vectors with comparatively few parameters per residual block.

LM leverages that structure.

For very large sparse problems, a solver must exploit sparsity, block structure, and appropriate factorization or iterative methods.

In bundle adjustment, for example, cameras and 3D points create a characteristic sparse Jacobian structure. Eliminating point variables through a Schur complement can reduce the system to a smaller camera problem.

Ceres Solver provides practical trust-region and LM implementations for nonlinear least squares, including scaled damping and large-problem linear-solver strategies.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://ceres-solver.readthedocs.io\&sz=32)

Ceres Solver





M053 established how optimization algorithms generate candidate updates. M054 addresses what happens when the mathematical problem is ambiguous, badly scaled, noisy, constrained, numerically unstable, or represented using geometry that does not behave like an ordinary Euclidean vector.

These are not secondary implementation concerns.

In actual reconstruction, calibration, physics, and graphics systems, they are often the difference between a solver that works on a controlled example and a solver that remains reliable on real data.

#### Regularization: modifying the objective to encode preferences

Suppose measurements do not uniquely determine all the unknown parameters.

For a linear system:

$$
A\mathbf x=\mathbf b,
$$

there may be many possible solutions or none that satisfies every noisy observation exactly.

Least squares minimizes:

$$
\frac12\|A\mathbf x-\mathbf b\|^2.
$$

If $A$ is rank deficient, different parameter vectors can produce the same predictions.

Even when the solution is unique, it may be highly sensitive to measurement noise.

Regularization modifies the optimization problem by expressing an additional preference:

$$
\boxed{ \min\_{\mathbf x} \frac12\|A\mathbf x-\mathbf b\|^2 + \lambda R(\mathbf x) }.
$$

Here $R(\mathbf x)$ is a regularizer, and $\lambda\ge0$ controls its influence.

A regularizer can encode a preference for smaller parameters, sparsity, smoothness, closeness to a prior estimate, physically plausible deformations, or another desired property.

##### Why regularization can stabilize an inverse problem

Suppose we observe:

$$
y=x_1+x_2=10.
$$

There are infinitely many solutions:

$$
(x_1,x_2)=(0,10),(5,5),(10,0),\ldots
$$

If we minimize:

$$
E(x_1,x_2)=\frac12(x_1+x_2-10)^2,
$$

the optimal set is the entire line:

$$
x_1+x_2=10.
$$

The data alone cannot identify a unique solution.

Now add:

$$
R=\frac12(x_1^2+x_2^2).
$$

The regularized objective is:

$$
E\_\lambda= \frac12(x_1+x_2-10)^2 +\frac\lambda2(x_1^2+x_2^2).
$$

Differentiate:

$$
\frac{\partial E\_\lambda}{\partial x_1} = x_1+x_2-10+\lambda x_1,
$$

$$
\frac{\partial E\_\lambda}{\partial x_2} = x_1+x_2-10+\lambda x_2.
$$

Subtract the two equations at stationarity:

$$
\lambda(x_1-x_2)=0.
$$

For $\lambda>0$:

$$
x_1=x_2.
$$

Let both equal $s$. Then:

$$
2s-10+\lambda s=0.
$$

Thus:

$$
\boxed{ x_1=x_2=\frac{10}{2+\lambda} }.
$$

For very small positive $\lambda$, the solution approaches $(5,5)$.

Regularization has selected a unique small-norm solution.

But notice that for finite $\lambda$, the sum is:

$$
x_1+x_2=\frac{20}{2+\lambda},
$$

which is less than 10.

The solution is biased toward zero.

This is the regularization trade-off: improving stability and expressing prior preferences can introduce systematic bias.

##### Regularization does not create evidence

If the data cannot determine a parameter, a regularizer may select one value, but that value comes partly from the prior preference.

In a research system, this distinction must be explicit.

A camera calibration result that depends strongly on a pose prior should not be described as uniquely determined by the images.

A reconstructed surface that depends strongly on a smoothness penalty should not be described as fully recovered from observations alone.

Regularization resolves numerical or modeling ambiguity according to assumptions. It does not manufacture missing measurements.

#### L1 regularization and sparsity

The L1 norm is:

$$
\boxed{ \|\mathbf x\|\_1=\sum_i|x_i| }.
$$

An L1-regularized objective has the form:

$$
\boxed{ E(\mathbf x)= f(\mathbf x)+\lambda\|\mathbf x\|\_1 }.
$$

L1 regularization often encourages exact zeros in the solution.

Why?

Because $|x|$ has a sharp corner at zero. Unlike a smooth quadratic penalty, its slope magnitude away from zero does not shrink as $x$ approaches zero.

##### Derive the soft-thresholding operation

Consider the scalar problem:

$$
\min_x \frac12(x-a)^2+\lambda|x|.
$$

The first term prefers $x=a$. The L1 penalty prefers $x=0$.

For $x>0$:

$$
|x|=x.
$$

Differentiate:

$$
(x-a)+\lambda=0.
$$

Thus:

$$
x=a-\lambda.
$$

This solution is valid only when $a>\lambda$.

For $x<0$:

$$
|x|=-x.
$$

Differentiate:

$$
(x-a)-\lambda=0.
$$

Thus:

$$
x=a+\lambda.
$$

This is valid only when $a<-\lambda$.

For $|a|\le\lambda$, the optimum occurs at $x=0$, where the ordinary derivative of $|x|$ does not exist.

The subdifferential of $|x|$ at zero is:

$$
\partial|0|=[-1,1].
$$

The optimality condition:

$$
0\in x-a+\lambda\partial|x|
$$

is satisfied at $x=0$ when $|a|\le\lambda$.

Combining the cases:

$$
\boxed{ S\_\lambda(a)= \operatorname{sign}(a) \max(|a|-\lambda,0) }.
$$

This is the soft-thresholding operator.

Conceptual soft-thresholding curve: inputs inside the threshold interval map exactly to zero; larger inputs are shrunk toward zero. The faint diagonal represents the unregularized identity mapping.

For example, with $\lambda=2$:

$$
S_2(5)=3,\qquad S_2(-4)=-2,\qquad S_2(1)=0.
$$

##### Why ordinary gradient descent is not ideal for L1

The L1 norm is not differentiable at zero.

We can use subgradient methods, but proximal-gradient methods often exploit the regularizer's structure more effectively.

For:

$$
E(\mathbf x)=f(\mathbf x)+\lambda\|\mathbf x\|\_1,
$$

where $f$ is smooth, a proximal-gradient step is:

$$
\boxed{ \mathbf x\_{k+1} = S\_{\alpha\lambda} \left( \mathbf x_k-\alpha\nabla f(\mathbf x_k) \right) }.
$$

Soft-thresholding is applied componentwise.

This first takes a gradient step on the smooth objective and then applies the proximal operation associated with the L1 penalty.

The operation is not merely clipping small values heuristically. It is the exact minimizer of a well-defined local proximal subproblem.

##### Graphics applications

L1 penalties can encourage sparse deformation coefficients, sparse corrective blend shapes, sparse feature selection, or a small number of active residual components.

However, sparsity is meaningful only relative to a chosen representation. A parameter vector can be sparse in one basis and dense in another.

Therefore an L1 prior on deformation coefficients expresses a preference tied to that deformation basis, not a representation-independent physical law.

#### L2 regularization and ridge-type stabilization

The L2 norm is:

$$
\|\mathbf x\|\_2= \sqrt{\sum_i x_i^2}.
$$

In optimization, L2 regularization commonly refers to the squared L2 norm:

$$
\boxed{ R(\mathbf x)=\frac12\|\mathbf x\|\_2^2 }.
$$

Its gradient is:

$$
\nabla R=\mathbf x,
$$

and its Hessian is:

$$
H_R=I.
$$

For:

$$
E\_\lambda(\mathbf x) = \frac12\|A\mathbf x-\mathbf b\|^2 +\frac\lambda2\|\mathbf x\|^2,
$$

the gradient is:

$$
\nabla E\_\lambda = A^T(A\mathbf x-\mathbf b)+\lambda\mathbf x.
$$

Set to zero:

$$
\boxed{ (A^TA+\lambda I)\mathbf x=A^T\mathbf b }.
$$

This is a ridge or Tikhonov-regularized least-squares system.

For $\lambda>0$, $A^TA+\lambda I$ is positive definite even when $A$ is rank deficient.

That improves the mathematical invertibility of the regularized normal system.

##### Generalized Tikhonov regularization

More generally, suppose we have a prior parameter vector $\mathbf x_0$ and a linear regularization operator $L$:

$$
E(\mathbf x)= \frac12\|A\mathbf x-\mathbf b\|^2+ \frac\lambda2\|L(\mathbf x-\mathbf x_0)\|^2.
$$

Its gradient is:

$$
A^T(A\mathbf x-\mathbf b)+ \lambda L^TL(\mathbf x-\mathbf x_0).
$$

Setting it to zero gives:

$$
\boxed{ (A^TA+\lambda L^TL)\mathbf x = A^T\mathbf b+\lambda L^TL\mathbf x_0 }.
$$

Different $L$ matrices encode different priors.

If $L=I$, we penalize deviation from $\mathbf x_0$.

If $L$ is a discrete difference operator, we penalize changes between adjacent parameter values.

If $L$ approximates a spatial derivative, we can penalize deformation roughness.

##### Regularization versus LM damping

Both ridge regularization and LM damping may introduce a matrix term resembling $\lambda I$, but their meanings differ.

A persistent regularization term changes the actual objective and its optimum.

LM damping penalizes the current local step, or acts as part of a trust-region mechanism, and may change across iterations.

They are not interchangeable merely because their linear systems look similar.

For example, LM may ultimately reduce damping toward zero while still solving the original unregularized nonlinear least-squares objective.

A persistent L2 prior remains part of the objective at convergence.

#### Numerical gradient verification

We covered finite differences and automatic differentiation in M052, so here the focus is on how gradient verification becomes an engineering gate for an optimizer.

Suppose an implementation returns:

$$
\mathbf g\_{\mathrm{analytic}}.
$$

For a randomly selected direction $\mathbf d$, compute:

$$
D\_{\mathrm{analytic}} = \mathbf g\_{\mathrm{analytic}}^T\mathbf d.
$$

Then compute the central finite-difference estimate:

$$
D\_{\mathrm{numeric}} = \frac{ f(\mathbf x+h\mathbf d) - f(\mathbf x-h\mathbf d) }{2h}.
$$

We compare them across several reasonable values of $h$.

A useful scale-aware metric is:

$$
\boxed{ e= \frac{ |D\_{\mathrm{analytic}}-D\_{\mathrm{numeric}}| }{ \max(1,|D\_{\mathrm{analytic}}|,|D\_{\mathrm{numeric}}|) } }.
$$

The constant one here assumes the derivatives have been suitably nondimensionalized or that a unit-scale comparison is meaningful. For a general physical model, choose a reference scale compatible with the derivative's units.

##### What gradient checking can and cannot prove

A failed comparison can indicate an incorrect derivative, an invalid finite-difference step, a nonsmooth point, a mistaken coordinate convention, or inconsistent evaluations of the objective.

A successful comparison is evidence of correctness for the tested configurations and directions.

It is not proof that every derivative in a large system is correct.

In production, use deterministic smooth test cases, analytically known reference problems, multiple finite-difference scales, random directions, and carefully selected boundary cases.

For a nonlinear least-squares implementation, verify both the residual Jacobian and the resulting objective gradient $J^T\mathbf r$. Testing only the final optimizer result can hide derivative bugs that happen not to prevent convergence on an easy problem.

#### Conditioning of optimization problems

Conditioning describes how sensitive a mathematical solution is to changes in the problem's data or parameters.

Consider the quadratic objective:

$$
f(\mathbf x)= \frac12\mathbf x^TH\mathbf x-\mathbf b^T\mathbf x
$$

with $H$ symmetric positive definite.

Its minimizer satisfies:

$$
H\mathbf x^\star=\mathbf b.
$$

Now perturb $\mathbf b$ by $\delta\mathbf b$.

The minimizer changes by:

$$
H\delta\mathbf x=\delta\mathbf b.
$$

Thus:

$$
\boxed{ \delta\mathbf x=H^{-1}\delta\mathbf b }.
$$

If $H$ has a very small eigenvalue, its inverse has a very large eigenvalue.

Small data perturbations aligned with the corresponding eigenvector can produce large changes in the optimum.

##### Singular-value interpretation

Let:

$$
H=Q\Lambda Q^T
$$

for symmetric positive-definite $H$.

Then:

$$
H^{-1}=Q\Lambda^{-1}Q^T.
$$

Along eigenvector $\mathbf q_i$:

$$
H^{-1}\mathbf q_i= \frac1{\lambda_i}\mathbf q_i.
$$

Thus, small-curvature directions amplify solution sensitivity.

This is why nearly flat optimization valleys can be difficult even when the objective is mathematically smooth.

##### The condition number

The spectral condition number is:

$$
\boxed{ \kappa_2(H)= \frac{\lambda\_{\max}}{\lambda\_{\min}} }
$$

for symmetric positive-definite $H$.

A large value means the curvature scales are very different.

For general invertible matrices:

$$
\kappa_2(A)= \frac{\sigma\_{\max}(A)}{\sigma\_{\min}(A)},
$$

where $\sigma_i$ are singular values.

If the smallest singular value approaches zero, the condition number grows without bound.

##### Normal equations square conditioning

Suppose a residual Jacobian has singular values:

$$
\sigma_1=1,\qquad\sigma_2=10^{-4}.
$$

Then:

$$
\kappa_2(J)=10^4.
$$

The matrix $J^TJ$ has eigenvalues:

$$
1,\qquad10^{-8}.
$$

Thus:

$$
\boxed{\kappa_2(J^TJ)=10^8}.
$$

This is one reason directly solving the normal equations can be numerically troublesome.

Even when the residual Jacobian is moderately conditioned, squaring the condition number can make the normal matrix difficult to solve accurately.

A QR or SVD-based approach can avoid explicitly forming $J^TJ$, often providing better numerical behavior at greater or differently structured computational cost.

##### Ill-conditioning versus a broken algorithm

If the underlying observations do not constrain a direction, no amount of floating-point precision can recover information that was never present.

Conversely, an algorithm may produce inaccurate results on a well-conditioned mathematical problem because it uses unstable arithmetic or a poor stopping criterion.

The distinction is:

Conditioning describes the mathematical problem. Numerical stability describes how effectively the implementation handles that problem.

#### Scale sensitivity in optimization parameters

Suppose one parameter is camera translation measured in metres and another is rotation measured in radians.

A raw update of $0.1$ has different physical significance for each.

Even within the same units, the scale of a parameter can alter numerical optimization behavior.

Consider:

$$
f(x,y)=x^2+10^6y^2.
$$

The Hessian is:

$$
H= \begin{bmatrix} 2&0\\ 0&2\times10^6 \end{bmatrix}.
$$

Its condition number is:

$$
\kappa=10^6.
$$

Ordinary gradient descent is strongly restricted by the steep $y$-direction.

Now change variables:

$$
z_1=x,\qquad z_2=1000y.
$$

Equivalently:

$$
x=z_1,\qquad y=\frac{z_2}{1000}.
$$

Substitute:

$$
f=z_1^2+z_2^2.
$$

The Hessian in the new coordinates is:

$$
H_z= \begin{bmatrix} 2&0\\ 0&2 \end{bmatrix}.
$$

The condition number becomes one.

The underlying physical objective has not changed. We have changed its parameterization.

This demonstrates that an optimizer's difficulty can depend heavily on the coordinate representation used for the unknowns.

##### Parameter scaling and physical interpretation

In camera calibration, one could express translation in metres or millimetres.

The corresponding numerical derivative scales by a factor of 1000.

An algorithm that treats all parameter coordinates equally may therefore behave differently even though the physical problem is identical.

Good optimization interfaces should support explicit parameter scaling and should distinguish physical units from dimensionless solver coordinates.

#### Normalization of optimization variables

Suppose physical parameters are represented by:

$$
\mathbf x=\mathbf x_0+S\mathbf z,
$$

where $S$ is an invertible scaling matrix, often diagonal.

The optimizer works in normalized coordinates $\mathbf z$.

Define:

$$
\tilde f(\mathbf z)=f(\mathbf x_0+S\mathbf z).
$$

By the chain rule:

$$
\boxed{ \nabla\_{\mathbf z}\tilde f = S^T\nabla\_{\mathbf x}f }.
$$

For a twice-differentiable objective with constant $S$:

$$
\boxed{ H\_{\mathbf z}=S^TH\_{\mathbf x}S }.
$$

Thus, normalization changes both the gradient and Hessian representation.

##### How to choose scales

A common method is to choose each scale according to a characteristic physical magnitude.

For example, a camera parameter vector may contain translation in metres, rotation in radians, and focal length in pixels.

One might use:

$$
S= \operatorname{diag} (s_t,s_t,s_t,s\_\omega,s\_\omega,s\_\omega,s_f).
$$

The values should represent meaningful changes in the respective parameters, not arbitrary magnitudes selected solely to force the numbers into a preferred range.

For a least-squares problem, Jacobian-column scaling can also be useful because each column measures how strongly one parameter affects the residuals.

If a column is nearly zero, however, dividing by its norm can amplify noise or reveal an unobservable parameter.

Therefore scales need safe lower bounds and explicit diagnostics for near-zero sensitivity.

##### Normalization is not the same as changing the problem

An invertible coordinate transformation preserves the set of physical solutions when performed consistently.

Adding regularization in the normalized coordinates, however, may change the physical prior.

For instance:

$$
\|\mathbf z\|^2 = \|S^{-1}(\mathbf x-\mathbf x_0)\|^2.
$$

This penalizes physical directions according to their chosen scales.

Therefore normalization and regularization interact. A solver must know whether scaling is intended only for numerical conditioning or also expresses a prior preference.

#### Finite precision in iterative optimization

A numerical optimizer performs a long sequence of floating-point calculations.

Every iteration may involve objective evaluation, gradient calculation, matrix products, linear solves, line searches, and state updates.

Finite precision can interfere with each stage.

##### Catastrophic cancellation

Suppose:

$$
f(\mathbf x)=10^{12}
$$

and a candidate has:

$$
f(\mathbf x+\mathbf p)=10^{12}-10^{-4}.
$$

In exact real arithmetic, the candidate improves the objective.

But in binary64, the spacing between representable numbers around $10^{12}$ is larger than $10^{-4}$ or comparable to it depending on the exact values.

The two objective values may round to the same representable number.

The computed decrease becomes zero.

A solver could mistake this for convergence or stagnation.

##### Unrepresentable parameter updates

Suppose:

$$
x=10^{16}, \qquad \Delta x=0.1.
$$

In binary64 arithmetic, adding $0.1$ may not change the stored value at all.

The update:

```cpp
x += 0.1;
```

can leave `x` unchanged.

Therefore checking that the proposed step was nonzero mathematically is insufficient. The implementation may also need to verify that the resulting stored iterate actually differs from the previous iterate.

##### Nonfinite arithmetic

NaNs and infinities can arise from invalid divisions, square roots of negative values, exponent overflow, unstable residual functions, or singular geometric configurations.

A robust optimizer must validate objective values, gradients, directions, and candidate states before accepting them.

NaNs are particularly dangerous because ordinary comparisons involving NaN are false.

For example:

```cpp
if (candidateValue < currentValue)
```

will not accept a NaN candidate, but code that uses other expressions or fails to validate state can still propagate NaNs.

##### Summation order

For a large least-squares objective:

$$
E=\frac12\sum_i r_i^2,
$$

the residual contributions may vary by many orders of magnitude.

Floating-point addition is not associative:

$$
(a+b)+c
$$

need not equal:

$$
a+(b+c)
$$

after rounding.

Parallel reductions, SIMD evaluation, and different thread schedules may therefore produce slightly different objective or gradient values.

Pairwise summation or compensated summation can improve accumulation accuracy.

The appropriate choice depends on whether performance, numerical accuracy, deterministic reduction order, or exact reproducibility is the priority.

#### Stopping criteria

A numerical optimizer must eventually stop, but reaching a small numerical quantity does not automatically mean it has found a useful solution.

Common stopping signals include the gradient norm, step magnitude, objective improvement, constraint violation, maximum iterations, and computational time.

##### Gradient-based stopping

For an unconstrained differentiable problem:

$$
\boxed{ \|\nabla f(\mathbf x)\|\le\epsilon_g }
$$

indicates approximate first-order stationarity.

It does not prove a local minimum, and it is inappropriate as the only criterion in a problem with active constraints.

It is also scale-dependent unless the gradient is normalized suitably.

##### Step-based stopping

A step criterion might be:

$$
\|\Delta\mathbf x\| \le \epsilon_x.
$$

But a tiny step can occur because the line search failed, the damping is enormous, or floating-point resolution prevents movement.

Therefore a tiny step should not automatically be called convergence.

##### Objective-change stopping

A common criterion is:

$$
|f\_{k+1}-f_k|\le\epsilon_f.
$$

This can be useful, but a small objective change may mean the algorithm is stuck rather than near a solution.

For example, gradient descent on a badly conditioned objective can make tiny progress along a flat valley while remaining far from the optimum.

##### Multiple criteria and solver statuses

A production solver should distinguish conditions such as first-order convergence, small step with acceptable stationarity, maximum iterations, line-search failure, invalid objective, singular linear system, infeasible constraints, and numerical stagnation.

A single boolean `Converged` cannot convey all these distinctions.

In more sophisticated implementations, the result can include a termination category, detailed diagnostic reason, final gradient norm, projected-gradient norm, constraint violation, number of accepted/rejected steps, and numerical linear-solver status.

#### Absolute versus relative tolerances

Absolute tolerances compare values against fixed units or magnitudes.

Relative tolerances scale comparisons by a reference quantity.

A common parameter-step criterion is:

$$
\boxed{ \|\Delta\mathbf x\| \le \epsilon\_{\mathrm{abs}}+ \epsilon\_{\mathrm{rel}}\|\mathbf x\| }.
$$

This allows a fixed minimum precision requirement while accounting for the scale of the current parameter vector.

However, for parameters with different units, a raw Euclidean norm can be physically meaningless.

A better approach is to use normalized coordinates or componentwise tolerances:

$$
|\Delta x_i| \le \epsilon\_{\mathrm{abs},i} + \epsilon\_{\mathrm{rel},i}|x_i|.
$$

For objective changes, one might use:

$$
|f\_{k+1}-f_k| \le \epsilon\_{\mathrm{abs},f} + \epsilon\_{\mathrm{rel},f}\max(|f_k|,|f\_{k+1}|).
$$

The choice depends on whether the objective has a meaningful absolute scale.

##### Why relative tolerance alone fails near zero

If the true optimum objective is zero, a denominator based solely on the current objective can become tiny.

Relative error can become unstable or undefined.

Absolute tolerance helps prevent that singularity.

Conversely, a fixed absolute tolerance can be inappropriate across problems whose objective values differ by orders of magnitude.

This is why well-designed numerical software commonly combines both.

#### Detecting stagnation

Stagnation means the optimizer is making little or no progress, but it has not necessarily satisfied the desired optimality conditions.

Possible causes include poor conditioning, incorrect gradients, incompatible scaling, overly aggressive or overly conservative damping, finite-precision limitations, a nondifferentiable boundary, or a genuinely nearly flat region.

##### A useful diagnostic procedure

Suppose successive parameter values barely change.

First inspect:

$$
\|\nabla f\|.
$$

If the gradient is small, approximate stationarity may be a reasonable explanation.

If the gradient remains large, inspect the proposed search direction and the directional derivative:

$$
\nabla f^T\mathbf p.
$$

If this is nonnegative, the direction is not descending.

If it is negative, inspect the step length selected by the line search.

If the line search repeatedly shrinks the step to nearly zero, inspect objective evaluations along the direction.

The failure might arise from a bad local model, invalid trial states, cancellation in objective differences, or a domain boundary.

For a Newton or LM method, also inspect the linear-system conditioning and the magnitude of damping.

This diagnostic chain moves from the observable symptom to increasingly specific mechanisms.

##### Detecting repeated failure

A practical solver can maintain counters for consecutive rejected steps, repeated objective values, and negligible parameter movement.

For example, if ten consecutive iterations make no representable progress while the gradient remains large, it may be reasonable to terminate with a stagnation status.

The specific count is a policy choice, not a mathematical theorem.

A production system should expose enough diagnostics to distinguish stagnation from successful convergence.

#### Optimization under bounds

Some parameters are allowed only within an interval:

$$
l_i\le x_i\le u_i.
$$

Examples include camera focal length ranges, opacity constrained between zero and one, bounded material roughness, permissible motor torques, and constrained animation parameters.

The feasible set is a box:

$$
\mathcal C= \{\mathbf x:\mathbf l\le\mathbf x\le\mathbf u\}.
$$

##### The projection operator

For box constraints, Euclidean projection is componentwise clipping:

$$
\boxed{ [\Pi\_{\mathcal C}(\mathbf x)]\_i = \min(u_i,\max(l_i,x_i)) }.
$$

This is the closest feasible point to $\mathbf x$ under Euclidean distance.

But clipping can produce different behavior from an unconstrained optimizer because the update becomes nonlinear at the boundaries.

A projected update may move in a different direction from the raw proposed direction.

##### Why raw gradients cannot determine convergence under bounds

Consider:

$$
f(x)=(x-3)^2
$$

subject to:

$$
-1\le x\le1.
$$

The unconstrained minimum is $x=3$, which is infeasible.

The constrained minimum is:

$$
\boxed{x^\star=1}.
$$

Its gradient is:

$$
f'(1)=2(1-3)=-4.
$$

The gradient is not zero.

But the negative-gradient direction is positive, trying to move outside the allowed interval.

There is no feasible first-order descent direction.

This motivates projected-gradient methods and constrained stationarity measurements.

#### Projected gradient descent

The projected-gradient update is:

$$
\boxed{ \mathbf x\_{k+1} = \Pi\_{\mathcal C} \left( \mathbf x_k-\alpha\nabla f(\mathbf x_k) \right) }.
$$

It first computes an unconstrained gradient step, then projects the candidate into the feasible set.

##### Deriving the projected-gradient mapping

Define:

$$
\boxed{ G\_\alpha(\mathbf x)= \frac1\alpha \left[ \mathbf x- \Pi\_{\mathcal C} (\mathbf x-\alpha\nabla f(\mathbf x)) \right] }.
$$

For an unconstrained problem, projection is the identity:

$$
\Pi\_{\mathbb R^n}(\mathbf z)=\mathbf z.
$$

Therefore:

$$
G\_\alpha(\mathbf x) = \frac1\alpha[ \mathbf x-(\mathbf x-\alpha\nabla f) ] = \nabla f.
$$

Thus the projected-gradient mapping generalizes the ordinary gradient.

At a constrained first-order stationary point:

$$
\boxed{ G\_\alpha(\mathbf x^\star)=0 }
$$

for an appropriate positive $\alpha$, assuming the standard convex feasible-set projection formulation.

##### Numerical example: projection onto a feasible box

Consider:

$$
f(x,y)= \frac12(x-3)^2+2(y+1)^2.
$$

Subject to:

$$
-1\le x\le1,
$$

$$
-0.5\le y\le1.
$$

The unconstrained minimum is:

$$
(3,-1),
$$

which is infeasible.

The constrained minimum is:

$$
\boxed{ (x^\star,y^\star)=(1,-0.5) }.
$$

The gradient is:

$$
\nabla f= \begin{bmatrix} x-3\\ 4(y+1) \end{bmatrix}.
$$

At the constrained optimum:

$$
\nabla f(1,-0.5)= \begin{bmatrix} -2\\2 \end{bmatrix}.
$$

Its norm is nonzero.

Now compute the projected-gradient mapping with $\alpha=1$:

$$
\mathbf x^\star-\nabla f= \begin{bmatrix} 1\\-0.5 \end{bmatrix} - \begin{bmatrix} -2\\2 \end{bmatrix} = \begin{bmatrix} 3\\-2.5 \end{bmatrix}.
$$

Project back into the box:

$$
\Pi\_{\mathcal C}(3,-2.5)=(1,-0.5).
$$

Therefore:

$$
G_1(\mathbf x^\star)=0.
$$

This correctly identifies a constrained stationary point despite the nonzero ordinary gradient.

##### Projected line search

When using line search with a projected method, the trial update is:

$$
\mathbf x\_{\mathrm{trial}} = \Pi\_{\mathcal C} (\mathbf x-\alpha\mathbf g).
$$

The actual displacement is:

$$
\mathbf d= \mathbf x\_{\mathrm{trial}}-\mathbf x.
$$

It is generally not equal to $-\alpha\mathbf g$, because projection may alter individual components.

Consequently, a sufficient-decrease test should use the actual projected displacement or another condition designed for projected optimization.

This is a common implementation error: evaluating Armijo as though projection had not changed the step.

#### Coordinate descent

Gradient descent changes all parameters simultaneously.

Coordinate descent changes one coordinate or one block of coordinates at a time.

For:

$$
f(x_1,\ldots,x_n),
$$

a coordinate update solves or approximates:

$$
\boxed{ x_i^{k+1} = \arg\min_z f(x_1^{k+1},\ldots,x\_{i-1}^{k+1}, z,x\_{i+1}^{k},\ldots,x_n^k) }.
$$

This is a cyclic coordinate descent update when coordinates are processed in a fixed order.

The newly updated coordinates are immediately used for subsequent coordinate subproblems.

That resembles Gauss–Seidel iteration.

A Jacobi-style coordinate method, by contrast, could compute coordinate updates using a common previous iterate and combine them afterward.

##### Worked quadratic example

Consider:

$$
f(x,y)= (x-2)^2+2(y+1)^2+xy.
$$

Its partial derivatives are:

$$
f_x=2(x-2)+y,
$$

$$
f_y=4(y+1)+x.
$$

For fixed $y$, minimize over $x$:

$$
2(x-2)+y=0.
$$

Thus:

$$
\boxed{x=2-\frac y2}.
$$

For fixed $x$, minimize over $y$:

$$
4(y+1)+x=0.
$$

Thus:

$$
\boxed{y=-1-\frac x4}.
$$

Starting at $(0,0)$, a cyclic coordinate-descent step first updates:

$$
x_1=2-\frac02=2.
$$

Then, using the newly updated $x_1$:

$$
y_1=-1-\frac24=-1.5.
$$

After one sweep:

$$
\boxed{(x_1,y_1)=(2,-1.5)}.
$$

The next sweep updates $x$ using $y=-1.5$, and so on.

##### When coordinate descent helps

Coordinate descent can be effective when individual coordinate subproblems are inexpensive, updates have simple closed forms, or the objective has exploitable block structure.

Examples include some sparse regression problems, alternating optimization schemes, and models in which one parameter block can be optimized while holding others fixed.

However, it can be slow when variables are strongly coupled or the chosen coordinate axes are poorly aligned with the objective geometry.

For example, a narrow quadratic valley rotated 45 degrees relative to the coordinate axes can cause coordinate descent to make only small alternating progress.

Block coordinate descent can partially address this by updating groups of strongly coupled parameters together.

Alternating camera and geometry optimization in reconstruction systems is conceptually related, but the convergence behavior depends on the exact objective, update schedules, and parameter block structure.

### Differentiating vector-valued functions

We have repeatedly used gradients for scalar objectives. But most graphics transformations are vector-valued.

A camera projection maps 3D coordinates to 2D coordinates. A rigid transform maps a 3D point to another 3D point. A normalizing operation maps a vector to a unit vector.

For:

$$
\mathbf f:\mathbb R^n\rightarrow\mathbb R^m,
$$

the derivative is represented by a Jacobian:

$$
\boxed{ J\_{\mathbf f}= \begin{bmatrix} \frac{\partial f_1}{\partial x_1}&\cdots&\frac{\partial f_1}{\partial x_n}\\ \vdots&\ddots&\vdots\\ \frac{\partial f_m}{\partial x_1}&\cdots&\frac{\partial f_m}{\partial x_n} \end{bmatrix} }.
$$

Its meaning is:

$$
\delta\mathbf y\approx J\_{\mathbf f}\delta\mathbf x.
$$

This is the forward sensitivity calculation.

If a scalar loss $L$ depends on $\mathbf y=\mathbf f(\mathbf x)$, the gradient propagates backward:

$$
\boxed{ \nabla\_{\mathbf x}L = J\_{\mathbf f}^T\nabla\_{\mathbf y}L }.
$$

These two operations are essential to differentiable programming.

The first is a Jacobian-vector product:

$$
J\mathbf v.
$$

The second is a vector-Jacobian product, represented using column gradients as:

$$
J^T\mathbf w.
$$

##### Derivative of a cross product

Let $\mathbf a(t),\mathbf b(t)\in\mathbb R^3$.

Their cross product satisfies:

$$
\frac d{dt}(\mathbf a\times\mathbf b) = \mathbf a'\times\mathbf b+ \mathbf a\times\mathbf b'.
$$

This is a vector-valued product rule.

Using the skew-symmetric matrix:

$$
[\mathbf a]\_\times= \begin{bmatrix} 0&-a_z&a_y\\ a_z&0&-a_x\\ -a_y&a_x&0 \end{bmatrix},
$$

we have:

$$
\mathbf a\times\mathbf b=[\mathbf a]\_\times\mathbf b.
$$

A perturbation produces:

$$
\delta(\mathbf a\times\mathbf b) = \delta\mathbf a\times\mathbf b+ \mathbf a\times\delta\mathbf b.
$$

Since:

$$
\delta\mathbf a\times\mathbf b = -[\mathbf b]\_\times\delta\mathbf a,
$$

the Jacobians are:

$$
\boxed{ \frac{\partial(\mathbf a\times\mathbf b)}{\partial\mathbf a} = -[\mathbf b]\_\times }
$$

and:

$$
\boxed{ \frac{\partial(\mathbf a\times\mathbf b)}{\partial\mathbf b} = [\mathbf a]\_\times }.
$$

These formulas are useful in rigid-body mechanics, geometric constraints, angular velocity, and rotation optimization.

##### Derivative of a vector norm

For:

$$
f(\mathbf x)=\|\mathbf x\|\_2 = \sqrt{\mathbf x^T\mathbf x},
$$

differentiate:

$$
df= \frac1{2\sqrt{\mathbf x^T\mathbf x}} d(\mathbf x^T\mathbf x).
$$

Because:

$$
d(\mathbf x^T\mathbf x)=2\mathbf x^Td\mathbf x,
$$

we obtain:

$$
df= \frac{\mathbf x^T}{\|\mathbf x\|}d\mathbf x.
$$

Therefore:

$$
\boxed{ \nabla\_{\mathbf x}\|\mathbf x\| = \frac{\mathbf x}{\|\mathbf x\|} }
$$

when $\mathbf x\ne0$.

At zero, the Euclidean norm is not classically differentiable. Its convex subdifferential is the unit ball:

$$
\partial\|\mathbf0\|\_2= \{\mathbf g:\|\mathbf g\|\_2\le1\}.
$$

A solver or AD library must choose how to represent that nonsmooth case.

### Differentiating matrix-valued functions

A matrix can depend on one scalar, many parameters, or another matrix.

For a scalar parameter $t$, a matrix-valued function:

$$
A(t)\in\mathbb R^{m\times n}
$$

has derivative:

$$
\boxed{ A'(t)= \begin{bmatrix} a'\_{11}(t)&\cdots&a'\_{1n}(t)\\ \vdots&\ddots&\vdots\\ a'\_{m1}(t)&\cdots&a'\_{mn}(t) \end{bmatrix} }.
$$

The derivative is computed elementwise, but it also follows matrix-level rules.

For matrices $A(t),B(t)$:

$$
\boxed{ \frac d{dt}(AB)=A'B+AB' }.
$$

The factor order must be preserved. Matrix multiplication is generally noncommutative.

##### Differentiating a matrix inverse

Suppose $A(t)$ is invertible.

We know:

$$
A(t)A(t)^{-1}=I.
$$

Differentiate:

$$
A'A^{-1} + A\frac d{dt}(A^{-1}) =0.
$$

Multiply from the left by $A^{-1}$:

$$
A^{-1}A'A^{-1} + \frac d{dt}(A^{-1}) =0.
$$

Therefore:

$$
\boxed{ \frac d{dt}(A^{-1}) = -A^{-1}A'A^{-1} }.
$$

This formula is important when differentiating inverse transforms, inverse covariance matrices, and systems defined through linear solves.

However, it does not imply that an implementation should explicitly construct two matrix inverses.

##### Differentiating a linear solve

Suppose:

$$
A(\theta)\mathbf x(\theta)=\mathbf b(\theta).
$$

Differentiate with respect to $\theta$:

$$
A\_\theta\mathbf x+A\mathbf x\_\theta = \mathbf b\_\theta.
$$

Rearrange:

$$
A\mathbf x\_\theta= \mathbf b\_\theta-A\_\theta\mathbf x.
$$

Therefore:

$$
\boxed{ \mathbf x\_\theta = A^{-1}(\mathbf b\_\theta-A\_\theta\mathbf x) }.
$$

Computationally, solve:

$$
\boxed{ A\mathbf x\_\theta = \mathbf b\_\theta-A\_\theta\mathbf x }.
$$

This avoids explicitly forming the inverse.

It is also an example of implicit differentiation.

For large numerical systems, the derivative of a solution can be computed by solving another linear system using the same matrix or factorization.

That creates opportunities to reuse factorizations.

##### Matrix derivatives as higher-order objects

If:

$$
F:\mathbb R^{p\times q}\rightarrow\mathbb R^{m\times n},
$$

then its complete derivative formally contains relationships between every input matrix element and every output matrix element.

Written entrywise:

$$
\frac{\partial F\_{ij}}{\partial X\_{kl}}.
$$

This is a fourth-order tensor if the matrix indices remain explicit.

In practical numerical software, we often avoid explicitly storing that tensor.

Instead, we calculate differentials or Jacobian-vector and vector-Jacobian products using the matrix's natural structure.

For example, for:

$$
F(X)=AXB,
$$

where $A,B$ are constant:

$$
\boxed{ dF=A(dX)B }.
$$

Given an upstream gradient $G$, reverse differentiation produces:

$$
\boxed{ \nabla_XL=A^TGB^T }.
$$

This is much cleaner than constructing a gigantic fourth-order derivative tensor.

### Differentiating quadratic forms

Quadratic forms appear everywhere in optimization, mechanics, statistics, and numerical linear algebra.

Consider:

$$
f(\mathbf x)=\mathbf x^TA\mathbf x,
$$

where $A\in\mathbb R^{n\times n}$.

We want its gradient.

Take a small perturbation:

$$
\mathbf x\longrightarrow\mathbf x+d\mathbf x.
$$

Expand:

$$
\begin{aligned} f(\mathbf x+d\mathbf x) &=(\mathbf x+d\mathbf x)^TA(\mathbf x+d\mathbf x)\\ &=\mathbf x^TA\mathbf x +(d\mathbf x)^TA\mathbf x\\ &\quad+\mathbf x^TA(d\mathbf x) +(d\mathbf x)^TA(d\mathbf x). \end{aligned}
$$

Discard the second-order differential term to obtain the first-order change:

$$
df= (d\mathbf x)^TA\mathbf x+ \mathbf x^TA(d\mathbf x).
$$

The second scalar term can be transposed without changing its value:

$$
\mathbf x^TA(d\mathbf x) = (d\mathbf x)^TA^T\mathbf x.
$$

Therefore:

$$
df= (d\mathbf x)^T(A+A^T)\mathbf x.
$$

Hence:

$$
\boxed{ \nabla f(\mathbf x)=(A+A^T)\mathbf x }.
$$

If $A$ is symmetric:

$$
\boxed{ \nabla f=2A\mathbf x }.
$$

For the commonly used objective:

$$
f(\mathbf x)=\frac12\mathbf x^TA\mathbf x,
$$

with symmetric $A$:

$$
\boxed{ \nabla f=A\mathbf x, \qquad H_f=A }.
$$

##### General quadratic objective

Let:

$$
f(\mathbf x)= \frac12\mathbf x^TH\mathbf x+ \mathbf c^T\mathbf x+d,
$$

where $H$ is symmetric.

Then:

$$
\boxed{ \nabla f=H\mathbf x+\mathbf c },
$$

$$
\boxed{ \nabla^2f=H }.
$$

This is the basic mathematical structure solved by Newton, quadratic programming, and trust-region subproblems.

##### Differentiation with respect to the matrix

Suppose:

$$
f(A)=\mathbf x^TA\mathbf x
$$

and $\mathbf x$ is constant.

Write:

$$
f(A)=\operatorname{tr}(A\mathbf x\mathbf x^T).
$$

The differential is:

$$
df=\mathbf x^T(dA)\mathbf x.
$$

Using the Frobenius inner product:

$$
df= \operatorname{tr}((\mathbf x\mathbf x^T)^TdA).
$$

Thus:

$$
\boxed{ \nabla_Af=\mathbf x\mathbf x^T }.
$$

This is relevant when optimizing matrices themselves, such as metric matrices, covariance-like parameters, or quadratic energy coefficients.

If $A$ is constrained to be symmetric or positive definite, its permissible variations must also respect that constraint or use an appropriate parameterization.

### Differentiating normalized vectors

Normalization is a deceptively simple graphics operation:

$$
\mathbf n= \frac{\mathbf v}{\|\mathbf v\|}.
$$

It appears in normal-vector calculation, lighting, camera directions, geometric constraints, and learning systems.

We need its complete derivative.

Let:

$$
r=\|\mathbf v\|.
$$

Then:

$$
\mathbf n=r^{-1}\mathbf v.
$$

Differentiate:

$$
d\mathbf n = r^{-1}d\mathbf v+ \mathbf v\\,d(r^{-1}).
$$

Since:

$$
d(r^{-1})=-r^{-2}dr
$$

and:

$$
dr=\frac{\mathbf v^Td\mathbf v}{r},
$$

we have:

$$
d(r^{-1}) = -\frac{\mathbf v^Td\mathbf v}{r^3}.
$$

Substitute:

$$
d\mathbf n= \frac{d\mathbf v}{r} - \frac{\mathbf v\mathbf v^T}{r^3}d\mathbf v.
$$

Factor:

$$
d\mathbf n= \frac1r \left( I-\frac{\mathbf v\mathbf v^T}{r^2} \right)d\mathbf v.
$$

Because:

$$
\mathbf n=\frac{\mathbf v}{r},
$$

we obtain:

$$
\boxed{ J\_{\mathrm{normalize}}(\mathbf v) = \frac1{\|\mathbf v\|} (I-\mathbf n\mathbf n^T) }.
$$

##### Geometric interpretation

The matrix:

$$
I-\mathbf n\mathbf n^T
$$

projects onto the plane perpendicular to $\mathbf n$.

This means normalization removes first-order radial changes.

If:

$$
d\mathbf v=\alpha\mathbf n,
$$

then:

$$
(I-\mathbf n\mathbf n^T)d\mathbf v=0.
$$

A perturbation that only changes the vector's length does not change its normalized direction to first order.

But a perpendicular perturbation changes the direction.

The magnitude of this response is scaled by:

$$
\frac1{\|\mathbf v\|}.
$$

Thus short vectors are highly sensitive to perturbations.

##### Numerical example: Jacobian of vector normalization

Take:

$$
\mathbf v= \begin{bmatrix} 3\\4 \end{bmatrix}.
$$

Its length is:

$$
r=5.
$$

The normalized vector is:

$$
\mathbf n= \begin{bmatrix} 0.6\\0.8 \end{bmatrix}.
$$

The outer product is:

$$
\mathbf n\mathbf n^T= \begin{bmatrix} 0.36&0.48\\ 0.48&0.64 \end{bmatrix}.
$$

Therefore:

$$
I-\mathbf n\mathbf n^T= \begin{bmatrix} 0.64&-0.48\\ -0.48&0.36 \end{bmatrix}.
$$

Divide by five:

$$
\boxed{ J\_{\mathrm{normalize}}= \begin{bmatrix} 0.128&-0.096\\ -0.096&0.072 \end{bmatrix} }.
$$

A perturbation:

$$
d\mathbf v= \begin{bmatrix} 0.01\\0 \end{bmatrix}
$$

produces approximate normalized change:

$$
d\mathbf n = \begin{bmatrix} 0.00128\\-0.00096 \end{bmatrix}.
$$

The normalized vector's first component increases slightly while the second decreases, preserving unit length to first order.

##### What breaks at zero?

At:

$$
\mathbf v=\mathbf0,
$$

normalization is undefined.

The Jacobian formula contains:

$$
\frac1{\|\mathbf v\|}.
$$

No finite derivative exists there.

A production `SafeNormalize` function may choose a fallback direction, return zero, or use a threshold, but this introduces implementation-specific behavior.

Differentiating that implementation is not necessarily the same as differentiating the mathematical normalization operation.

In differentiable rendering or physics, near-zero normals and directions require deliberate treatment rather than silently assuming a derivative exists.

### Differentiating rotations and parameterizations

Rotation optimization is one of the most important places where ordinary Euclidean intuition can fail.

A 3D rotation has three degrees of freedom, but we can represent it with Euler angles, a quaternion, a $3\times3$ matrix, an axis-angle vector, or Lie-algebra coordinates.

These representations are related, but their derivatives are not interchangeable.

#### Derivative of a 2D rotation

Start with:

$$
R(\theta)= \begin{bmatrix} \cos\theta&-\sin\theta\\ \sin\theta&\cos\theta \end{bmatrix}.
$$

Differentiate elementwise:

$$
\boxed{ R'(\theta)= \begin{bmatrix} -\sin\theta&-\cos\theta\\ \cos\theta&-\sin\theta \end{bmatrix} }.
$$

For a point:

$$
\mathbf p'(\theta)=R(\theta)\mathbf p,
$$

the derivative is:

$$
\boxed{ \frac{d\mathbf p'}{d\theta} = R'(\theta)\mathbf p }.
$$

For:

$$
\mathbf p= \begin{bmatrix} 2\\0 \end{bmatrix}
$$

and $\theta=0$:

$$
R'(0)= \begin{bmatrix} 0&-1\\ 1&0 \end{bmatrix}.
$$

Therefore:

$$
\frac{d\mathbf p'}{d\theta} = \begin{bmatrix} 0\\2 \end{bmatrix}.
$$

A small positive rotation initially moves the point upward in the chosen mathematical coordinate convention.

That is the geometric meaning of the derivative.

#### The tangent space of $SO(3)$

A valid 3D rotation matrix satisfies:

$$
R^TR=I, \qquad \det R=1.
$$

Differentiate the orthogonality condition:

$$
\frac d{dt}(R^TR)=0.
$$

Applying the product rule:

$$
\dot R^TR+R^T\dot R=0.
$$

Let:

$$
\Omega=R^T\dot R.
$$

Then:

$$
\Omega^T+\Omega=0.
$$

Therefore $\Omega$ is skew-symmetric.

Every $3\times3$ skew-symmetric matrix can be represented by an angular velocity vector:

$$
\Omega=[\boldsymbol\omega]\_\times.
$$

Hence:

$$
\boxed{ \dot R=R[\boldsymbol\omega_b]\_\times }
$$

when $\boldsymbol\omega_b$ is expressed in the body frame.

Alternatively:

$$
\boxed{ \dot R=[\boldsymbol\omega_w]\_\times R }
$$

when angular velocity is expressed in the world frame.

These are equivalent descriptions under the appropriate frame transformation:

$$
\boldsymbol\omega_w=R\boldsymbol\omega_b.
$$

##### Left and right rotation perturbations

Suppose a point is transformed as:

$$
\mathbf p'=R\mathbf p.
$$

A small left-applied rotation perturbation is:

$$
R\_{\mathrm{new}} = \exp([\delta\boldsymbol\omega]\_\times)R.
$$

For a small perturbation:

$$
\exp([\delta\boldsymbol\omega]\_\times) \approx I+[\delta\boldsymbol\omega]\_\times.
$$

Then:

$$
\begin{aligned} \mathbf p'\_{\mathrm{new}} &\approx (I+[\delta\boldsymbol\omega]\_\times)R\mathbf p\\ &=R\mathbf p+ \delta\boldsymbol\omega\times(R\mathbf p). \end{aligned}
$$

Using:

$$
\mathbf a\times\mathbf b = -[\mathbf b]\_\times\mathbf a,
$$

we obtain:

$$
\boxed{ \frac{\partial(R\mathbf p)} {\partial\delta\boldsymbol\omega\_{\mathrm{left}}} = -[R\mathbf p]\_\times }.
$$

For a right-applied perturbation:

$$
R\_{\mathrm{new}} = R\exp([\delta\boldsymbol\omega]\_\times),
$$

the corresponding Jacobian is:

$$
\boxed{ \frac{\partial(R\mathbf p)} {\partial\delta\boldsymbol\omega\_{\mathrm{right}}} = -R[\mathbf p]\_\times }.
$$

These matrices are generally different.

The difference is not a convention that can be ignored. It follows from the different coordinate frames in which the rotation perturbations are expressed.

##### Differentiating a rigid transform

For:

$$
\mathbf p_c=R\mathbf p_w+\mathbf t,
$$

we may optimize camera pose using a six-dimensional local perturbation:

$$
\delta\boldsymbol\xi= \begin{bmatrix} \delta\boldsymbol\omega\\ \delta\mathbf v \end{bmatrix}.
$$

For a left perturbation of the full rigid transform:

$$
T\_{\mathrm{new}}= \exp([\delta\boldsymbol\xi]^\wedge)T,
$$

the first-order point variation is:

$$
\delta\mathbf p_c = -[\mathbf p_c]\_\times\delta\boldsymbol\omega +\delta\mathbf v.
$$

Therefore:

$$
\boxed{ J\_{\mathrm{pose}} = \begin{bmatrix} -[\mathbf p_c]\_\times&I \end{bmatrix} }.
$$

This is a $3\times6$ matrix.

If the resulting camera-space point is projected into image space, the complete Jacobian is:

$$
\boxed{ J\_{\mathrm{pixel,pose}} = J\_{\mathrm{projection}} \begin{bmatrix} -[\mathbf p_c]\_\times&I \end{bmatrix} }.
$$

The result is $2\times6$.

This is a fundamental structure in bundle adjustment, visual odometry, pose refinement, and differentiable camera models.

Notice that the expression changes if we perturb rotation alone while holding translation fixed, or if we choose a right perturbation of the full transform.

Derive from the exact update convention used by the implementation.

##### Euler angles and gimbal lock

Euler angles use a sequence of rotations, for example:

$$
R=R_z(\psi)R_y(\theta)R_x(\phi).
$$

The derivative with respect to one angle uses the product rule while the other matrices remain fixed:

$$
\frac{\partial R}{\partial\theta} = R_z(\psi) \frac{\partial R_y(\theta)}{\partial\theta} R_x(\phi).
$$

The order of multiplication remains essential.

Euler angles can become singular as a parameterization. In certain configurations, two angle variations produce the same instantaneous rotational effect, and the parameter-to-orientation Jacobian loses rank.

This is gimbal lock.

It does not mean the physical rotation itself becomes invalid. It means the chosen coordinates fail to provide a locally independent representation of orientation at that configuration.

For optimization, such singularities can cause poor conditioning or ambiguous parameter updates.

##### Quaternion constraints and differentiation

A unit quaternion satisfies:

$$
q^Tq=1.
$$

It stores four scalar components but represents only three independent rotational degrees of freedom.

Optimizing the four components as unconstrained variables may violate the unit-length condition.

One option is to normalize after updates:

$$
q\_{\mathrm{new}}= \frac{q+\delta q}{\|q+\delta q\|}.
$$

But this update is not equivalent to an intrinsic three-dimensional rotation update in every respect.

Another option is to use a local rotation perturbation and compose it with the existing orientation:

$$
q\_{\mathrm{new}}= \delta q(\delta\boldsymbol\omega)\otimes q.
$$

The small rotation vector $\delta\boldsymbol\omega\in\mathbb R^3$ is converted into a unit quaternion through an exponential-map-like construction.

This preserves the rotation manifold structure more naturally.

Also remember that $q$ and $-q$ represent the same physical rotation. A quaternion objective should not accidentally treat this representation ambiguity as a physical difference.

##### Repository connection: KairoMath Transform

The source claims that [KairoMath/Transform.cppm](https://github.com/swayam8624/KairoMath/blob/master/Transform.cppm?utm_source=chatgpt.com), the `Transform` type stores translation, rotation, and scale separately, rather than only storing a generic $4\times4$ matrix.

The rotation field uses a quaternion.

The inspected constructors and setters normalize the quaternion, and `Rotate` composes an incoming quaternion with the stored orientation and normalizes the result.

This establishes a unit-quaternion-oriented API, but it does not make the full transform automatically differentiable with respect to a minimal six-dimensional pose parameterization.

A differentiable pose optimizer would still need an explicit tangent-space update convention, appropriate Jacobians, and clear treatment of translation, scale, and quaternion normalization.

A particularly useful test would compare the analytical pose Jacobian with finite differences of the same left- or right-applied perturbation operation used by the engine.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares#Optimization and inverse problems|Gauss-Newton]] — Nonlinear least squares minimizes one half the squared norm….



<!-- end V09.07 -->

<!-- begin V09.08 -->

<!-- Bible-final-pass: SE3 exp log -->
#### From a six-number pose update to an actual rigid transform

The previous section derived the tangent space of SO(3) and showed how a small rotational perturbation changes a point. One bridge was still missing: how a six-component local pose update becomes a finite rigid transform, and how a nearby rigid transform becomes a six-component update again.

A rigid transform $T\in SE(3)$ contains a rotation $R\in SO(3)$ and a translation $t$:

$$
T=\begin{bmatrix}R&t\\0&1\end{bmatrix}
$$

A local six-vector is commonly written as:

$$
\xi=\begin{bmatrix}\omega\\\rho\end{bmatrix}
$$

$\omega$ is a three-component local rotation vector measured in radians. $\rho$ is the translational part of the tangent coordinate. In general, $\rho$ is not the final translation column $t$ when rotation is nonzero.

The wedge form is:

$$
\widehat{\xi}=\begin{bmatrix}[\omega]_\times&\rho\\0&0\end{bmatrix}
$$

The group exponential gives:

$$
\exp(\widehat{\xi})=\begin{bmatrix}\exp([\omega]_\times)&V(\omega)\rho\\0&1\end{bmatrix}
$$

Let $\theta=\lVert\omega\rVert$. Rodrigues' formula is:

$$
R=I+\frac{\sin\theta}{\theta}[\omega]_\times+
\frac{1-\cos\theta}{\theta^2}[\omega]_\times^2
$$

The translation uses the SO(3) left Jacobian:

$$
V(\omega)=I+\frac{1-\cos\theta}{\theta^2}[\omega]_\times+
\frac{\theta-\sin\theta}{\theta^3}[\omega]_\times^2
$$

The matrix $V$ is the coupling that is easy to miss. When $\omega=0$, $V=I$ and $t=\rho$. With rotation, the finite translation is $t=V(\omega)\rho$. Rotation and translation are therefore not six unrelated scalar slots.

Near $\theta=0$, the direct fractions lose numerical accuracy through cancellation. Use the series:

$$
R\approx I+[\omega]_\times+\tfrac12[\omega]_\times^2,
\qquad
V\approx I+\tfrac12[\omega]_\times+\tfrac16[\omega]_\times^2
$$

The logarithm reverses the construction. First recover $\omega=\operatorname{Log}(R)$, then recover $\rho$ from:

$$
\rho=V(\omega)^{-1}t
$$

For ordinary non-singular angles:

$$
V^{-1}=I-\tfrac12[\omega]_\times+
\left(\frac1{\theta^2}-\frac{1+\cos\theta}{2\theta\sin\theta}\right)
[\omega]_\times^2
$$

Near zero:

$$
V^{-1}\approx I-\tfrac12[\omega]_\times+\tfrac1{12}[\omega]_\times^2
$$

Now the earlier pose Jacobian has a precise update convention. A left update is:

$$T_{\text{new}}=\exp(\widehat{\delta\xi})T$$

A right update is:

$$T_{\text{new}}=T\exp(\widehat{\delta\xi})$$

These are not interchangeable. They express the increment in different frames, so the Jacobians differ. Any paper or implementation that says only "we optimize six pose parameters" is incomplete until it states component ordering, units, frame convention, and left-versus-right composition.

A simple hand trace makes the coupling visible. Let $\omega=(0,0,0.6)$ radians and $\rho=(1,0,0)$ length unit. The resulting translation is $V(\omega)\rho$, not simply $(1,0,0)$. The finite motion means "advance while rotating," so the displacement follows the geometry of the rigid-motion group.

This is why bundle adjustment, visual odometry, XR tracking, camera calibration, robotics, and inverse graphics commonly update a pose through a local Lie-algebra increment instead of adding values directly to a 4 by 4 matrix.

The executable witness implements Rodrigues' formula, an SO(3) logarithm for ordinary non-pi cases, V, inverse(V), and SE(3) round trips. Its code comments identify the small-angle series and the deliberate near-pi rejection. That rejection is a scope boundary: a production implementation needs a branch-aware axis recovery near pi, zero/tiny/ordinary/near-pi tests, inverse/composition checks, and analytic Jacobians compared against finite differences.


~~~cpp
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>

struct Vec3 {
    // Rotation vectors use radians; rho and t use the same length unit.
    double x{}, y{}, z{};
};

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
Vec3 operator*(double s, Vec3 v) { return {s*v.x, s*v.y, s*v.z}; }
double dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
double norm(Vec3 v) { return std::sqrt(dot(v,v)); }

struct Mat3 {
    // Row-major storage; multiplication consistently treats vectors as columns.
    std::array<std::array<double,3>,3> a{};
};

Mat3 identity() {
    // Zero initialization supplies off-diagonal zeros; set the identity diagonal.
    Mat3 M{};
    for (int i=0;i<3;++i) M.a[i][i]=1.0;
    return M;
}
Mat3 operator+(Mat3 A, Mat3 B) {
    Mat3 C{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) C.a[r][c]=A.a[r][c]+B.a[r][c];
    return C;
}
Mat3 operator-(Mat3 A, Mat3 B) {
    Mat3 C{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) C.a[r][c]=A.a[r][c]-B.a[r][c];
    return C;
}
Mat3 operator*(double s, Mat3 A) {
    for(auto& row:A.a) for(double& v:row) v*=s;
    return A;
}
Mat3 operator*(Mat3 A, Mat3 B) {
    // Standard row-by-column composition; k sums the shared dimension.
    Mat3 C{};
    for(int r=0;r<3;++r)
        for(int c=0;c<3;++c)
            for(int k=0;k<3;++k)
                C.a[r][c]+=A.a[r][k]*B.a[k][c];
    return C;
}
Vec3 operator*(Mat3 A, Vec3 v) {
    return {
        A.a[0][0]*v.x + A.a[0][1]*v.y + A.a[0][2]*v.z,
        A.a[1][0]*v.x + A.a[1][1]*v.y + A.a[1][2]*v.z,
        A.a[2][0]*v.x + A.a[2][1]*v.y + A.a[2][2]*v.z
    };
}
Mat3 transpose(Mat3 A) {
    Mat3 T{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) T.a[r][c]=A.a[c][r];
    return T;
}
double trace(Mat3 A) { return A.a[0][0]+A.a[1][1]+A.a[2][2]; }

Mat3 hat(Vec3 w) {
    // hat(w) is skew-symmetric and maps v to the cross product w cross v.
    Mat3 W{};
    W.a[0][1] = -w.z;
    W.a[0][2] =  w.y;
    W.a[1][0] =  w.z;
    W.a[1][2] = -w.x;
    W.a[2][0] = -w.y;
    W.a[2][1] =  w.x;
    return W;
}
Vec3 vee(Mat3 W) {
    // Read the three independent skew entries, inverse to hat() on skew matrices.
    return {W.a[2][1], W.a[0][2], W.a[1][0]};
}

Mat3 so3_exp(Vec3 w) {
    // Rodrigues' formula evaluates exp(hat(w)); theta is measured in radians.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // The direct fractions cancel near zero; use their limiting series.
        return identity() + W + 0.5*W2;
    }
    const double A=std::sin(theta)/theta;
    const double B=(1.0-std::cos(theta))/(theta*theta);
    return identity() + A*W + B*W2;
}

Vec3 so3_log(Mat3 R) {
    // Precondition: R is a valid proper rotation. This compact witness does
    // not project or repair malformed matrices before applying the log.
    // Trace yields cos(theta); clamp roundoff into acos's valid domain [-1,1].
    const double cos_theta=std::clamp((trace(R)-1.0)*0.5,-1.0,1.0);
    const double theta=std::acos(cos_theta);
    if(theta < 1e-8) {
        // The antisymmetric part is 2*hat(w) to first order near identity.
        return vee(0.5*(R-transpose(R)));
    }
    // The principal logarithm has a delicate axis branch near pi; reject that
    // region rather than divide by a nearly zero sin(theta) and fake support.
    if (std::abs(std::numbers::pi-theta) <= 1e-5)
        throw std::domain_error("compact SO(3) log witness does not support theta near pi");
    return vee((theta/(2.0*std::sin(theta)))*(R-transpose(R)));
}

Mat3 left_jacobian_so3(Vec3 w) {
    // V maps tangent translation rho to the finite transform translation t.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // Taylor limit avoids subtracting nearly equal trigonometric values.
        return identity() + 0.5*W + (1.0/6.0)*W2;
    }
    const double t2=theta*theta, t3=t2*theta;
    return identity()
         + ((1.0-std::cos(theta))/t2)*W
         + ((theta-std::sin(theta))/t3)*W2;
}

Mat3 left_jacobian_so3_inverse(Vec3 w) {
    // Invert V to recover rho after the logarithm has recovered omega.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // Use the inverse series because the closed-form coefficient is fragile.
        return identity() - 0.5*W + (1.0/12.0)*W2;
    }
    const double s=std::sin(theta), c=std::cos(theta);
    const double coeff=1.0/(theta*theta) - (1.0+c)/(2.0*theta*s);
    return identity() - 0.5*W + coeff*W2;
}

struct SE3 {
    // This transform maps a point p to R*p + t.
    Mat3 R;
    Vec3 t;
};

SE3 se3_exp(Vec3 omega, Vec3 rho) {
    // The group exponential couples translation to rotation through V(omega).
    const Mat3 R=so3_exp(omega);
    const Mat3 V=left_jacobian_so3(omega);
    return {R, V*rho};
}

std::pair<Vec3,Vec3> se3_log(SE3 T) {
    // Assume T contains a valid SE(3) rotation. Rotation is recovered first
    // because its Jacobian determines translation rho; near-pi input throws.
    const Vec3 omega=so3_log(T.R);
    const Mat3 Vinv=left_jacobian_so3_inverse(omega);
    return {omega, Vinv*T.t};
}

bool near(double a,double b,double eps=1e-9){ return std::abs(a-b)<=eps; }
bool near(Vec3 a,Vec3 b,double eps=1e-9){
    return near(a.x,b.x,eps)&&near(a.y,b.y,eps)&&near(a.z,b.z,eps);
}
bool near(Mat3 A,Mat3 B,double eps=1e-9){
    for(int r=0;r<3;++r) for(int c=0;c<3;++c)
        if(!near(A.a[r][c],B.a[r][c],eps)) return false;
    return true;
}

void require(bool condition, const char* message) {
    // Unlike assert, these checks remain active in optimized NDEBUG builds.
    if (!condition) throw std::runtime_error(message);
}

int main() {
    {
        // Ordinary-angle round trip checks both the recovered twist and SO(3) shape.
        const Vec3 w{0.2,-0.3,0.1};
        const Mat3 R=so3_exp(w);
        require(near(transpose(R)*R, identity(), 1e-10), "R must remain orthogonal");
        require(near(so3_log(R), w, 1e-10), "ordinary SO(3) exp/log round trip");
    }
    {
        // Select the small-angle Taylor branch explicitly.
        const Vec3 w{1e-10,-2e-10,3e-10};
        const Mat3 R=so3_exp(w);
        require(near(so3_log(R), w, 1e-9), "small-angle SO(3) exp/log round trip");
    }
    {
        // Both angular and translational tangent coordinates round-trip.
        const Vec3 omega{0.25,-0.15,0.05};
        const Vec3 rho{1.2,-0.4,0.7};
        const SE3 T=se3_exp(omega,rho);
        const auto [omega2,rho2]=se3_log(T);
        require(near(omega2,omega,1e-10), "SE(3) rotational coordinate round trip");
        require(near(rho2,rho,1e-10), "SE(3) translational coordinate round trip");
    }
    {
        // A nonzero omega makes t differ from rho, exposing the coupling.
        // Translation and rotation are coupled by V(omega): t is not generally rho.
        const Vec3 omega{0.0,0.0,0.6};
        const Vec3 rho{1.0,0.0,0.0};
        const SE3 T=se3_exp(omega,rho);
        require(!near(T.t,rho,1e-6), "finite rigid motion must expose V(omega) translation coupling");
        const auto [w2,r2]=se3_log(T);
        require(near(w2,omega,1e-10), "coupled example rotational round trip");
        require(near(r2,rho,1e-10), "coupled example translational round trip");
    }
    {
        // The compact logarithm promises to reject its unsupported near-pi
        // branch before its sin(theta) denominator becomes numerically unsafe.
        const Vec3 near_pi{std::numbers::pi - 5e-6, 0.0, 0.0};
        bool rejected = false;
        try {
            (void)so3_log(so3_exp(near_pi));
        } catch (const std::domain_error&) {
            rejected = true;
        }
        require(rejected, "near-pi logarithm must take its documented rejection branch");
    }
    std::cout << "se3 exp/log: 5 suites PASS\n";
}
~~~

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/26 - Matrix Functions and Nonlinear Least Squares|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/28 - Reliable Optimization and Root Finding|Next chapter]]
