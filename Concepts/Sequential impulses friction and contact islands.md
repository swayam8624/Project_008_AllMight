# Sequential impulses, Coulomb friction, and contact islands

A collision detector reports geometric contact; a solver must convert that contact into velocity changes while respecting unilateral nonpenetration, friction and body coupling.

## Normal impulse

Let $\mathbf n$ be a unit normal pointing from body A toward body B, and let $\mathbf v_A,\mathbf v_B$ be their translational velocities at contact. In the rotationally ignored two-body reference case, relative normal velocity is

$$
v_n=(\mathbf v_B-\mathbf v_A)\cdot\mathbf n,
$$

so an approaching contact has $v_n<0$ under this convention. For positive masses, the effective inverse mass is

$$
k=\frac{1}{m_A}+\frac{1}{m_B}>0.
$$

A perfectly inelastic impulse that removes approach is

$$
j_n=\max\left(0,-\frac{v_n}{k}\right).
$$

The nonnegative clamp expresses the unilateral constraint: a contact may push bodies apart but should not pull separated bodies together. If $k=0$ (both bodies are immovable under this scalar model), skip the row instead of dividing; for $k>0$, $j_n$ has impulse units of kg m/s. The applied velocity changes are $\Delta\mathbf v_A=-j_n\mathbf n/m_A$ and $\Delta\mathbf v_B=+j_n\mathbf n/m_B$.

## Coulomb friction

A unit tangent $\mathbf t$ defines relative tangential speed $v_t=(\mathbf v_B-\mathbf v_A)\cdot\mathbf t$ after the normal update. In this translation-only model, $k_t=k$; the unconstrained impulse is $j_t^*=-v_t/k_t$. Coulomb friction clamps it by normal support:

$$
|j_t|\le\mu j_n,
\qquad
j_t=\operatorname{clamp}(j_t^*,-\mu j_n,\mu j_n).
$$

Here $\mu\ge0$ is the dimensionless friction coefficient. If $k_t=0$, skip the tangent row rather than divide. This is the simplest interval approximation to Coulomb friction in one tangent direction. A full 3D solver needs two tangent directions or a friction-cone approximation and rotational effective mass.

## Why sequential iteration is needed

One impulse changes velocities used by neighboring contacts. In a stack, solving ground-to-lower body and then lower-to-upper body disturbs the first constraint again. Repeated Gauss-Seidel-like sweeps propagate support through the stack and converge toward mutually compatible velocities. Warm starting, ordering and stabilization affect convergence speed.

## Contact islands

Contacts and joints form a graph. Union-find can group connected dynamic bodies into independent **islands**. Different islands may be solved in parallel because no constraint couples their velocities. Sleeping and wake-up policy usually operates on the same connectivity.

## Executable reference

`tools/labs/constraint_friction_island_lab.cpp` verifies union-find island separation, unilateral normal impulse, Coulomb tangent clamping, and convergence of a tiny two-body stack over repeated sequential sweeps.

The witness deliberately excludes angular Jacobians, restitution thresholds, multidirectional friction, warm-start scaling and current production-engine parity.

Related: [[Impulse and force timing]], [[Continuous collision detection]], [[Center of mass and inertia]].
