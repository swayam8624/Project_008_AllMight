That closes the original rigid-body mechanics reconstruction through Part 1700. The supplied lesson-body provenance for Parts 1701–1800 is missing, so the following contact and constraint material is an explicit reconstruction, not a claim that the original source text was preserved.

## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge

One isolated impact can be solved with one scalar impulse. A resting crate immediately breaks that model: two contact points belong to one rigid body, and an impulse at either point changes both center velocity and angular velocity. Therefore it changes the normal speed at both points.

For contact $i$, let $\mathbf p_i$ be the world point and let unit normal $\mathbf n$ point from body $A$ toward body $B$. The lever arms are $\mathbf r_{Ai}=\mathbf p_i-\mathbf x_A$ and $\mathbf r_{Bi}=\mathbf p_i-\mathbf x_B$. Point velocity is $\mathbf v+\boldsymbol\omega\times\mathbf r$, so the pre-solve normal speed is

$$
u_i=\mathbf n^\mathsf T\left[(\mathbf v_B+\boldsymbol\omega_B\times\mathbf r_{Bi})-(\mathbf v_A+\boldsymbol\omega_A\times\mathbf r_{Ai})\right]
$$

Use SI units: positions are metres, linear velocity is metres/second, angular velocity is radians/second, mass is kilograms, inverse inertia is kilogram$^{-1}$ metre$^{-2}$, and impulse is newton-seconds. With the normal from $A$ to $B$, $u_i<0$ means closing. A normal impulse $\lambda_i\ge0$ pushes $B$ along $\mathbf n$ and applies the equal-and-opposite impulse to $A$; ordinary contact does not pull.

Let $u$ collect pre-solve speeds and $\lambda$ collect normal impulses. The contact-space effective-mass matrix maps impulses to speed changes:

$$
w=u+K\lambda,\qquad K=JM^{-1}J^\mathsf T.
$$

$J^\mathsf T\lambda$ maps contact impulses to generalized body impulses, $M^{-1}$ converts them to changes in linear and angular velocity, and $J$ projects those changes back to contact speeds. Thus $K_{ij}$ measures how impulse $j$ changes speed at contact $i$. For planar rigid bodies with a common normal,

$$
K_{ij}=m_A^{-1}+m_B^{-1}+I_A^{-1}(\mathbf r_{Ai}\times\mathbf n)(\mathbf r_{Aj}\times\mathbf n)+I_B^{-1}(\mathbf r_{Bi}\times\mathbf n)(\mathbf r_{Bj}\times\mathbf n).
$$

Off-diagonal terms express coupling. For frictionless zero-restitution contact, solve the complementarity conditions

$$
\lambda\ge0,\qquad w=u+K\lambda\ge0,\qquad \lambda_iw_i=0.
$$

A positive impulse requires zero residual closing speed; a separating point needs no impulse. For a unit-mass, unit-inertia crate descending at $2\,\mathrm{m/s}$ with contacts at $x=-1$ and $x=1$, $K=\begin{bmatrix}2&0\\0&2\end{bmatrix}$ and $u=(-2,-2)^\mathsf T$, giving $\lambda=(1,1)^\mathsf T\,\mathrm{N\,s}$. Moving the right contact to $x=2$ gives $K=\begin{bmatrix}2&-1\\-1&5\end{bmatrix}$ and $\lambda=(4/3,2/3)^\mathsf T\,\mathrm{N\,s}$. Sequential impulses approximate this coupled solution through repeated projected updates; a block solve handles a small set together.

The implementation checks all four two-contact active sets: both points, first only, second only, or neither. It accepts a candidate only when impulses are nonnegative and post-speeds satisfy complementarity within a scale-aware tolerance. A near-singular determinant skips the both-active inverse; a feasible one-point fallback may still exist for duplicate contact rows. If no candidate is feasible, the island is rejected before body mutation. This witness covers normal velocity response only; restitution targets, penetration stabilization, friction, warm-start cache lifetime, and larger islands are separate policies.

### Executable active-set witness with numeric and failure traces

The following C++20 program implements the two-point block and exercises symmetric, asymmetric, single-active, separating, rank-deficient, immovable, and invalid-normal cases. Its comments state the equations, units, branch rules, and invariants beside the operations they explain.

```cpp
{{TWO_POINT_CONTACT_BLOCK_LAB}}
```

**Reconstruction check:** explain why the off-diagonal terms couple the contacts, list all four active sets, and say why a singular pair can still have a feasible one-contact solution.
