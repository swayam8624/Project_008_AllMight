# Sequential impulses, Coulomb friction, and contact islands

A collision detector reports geometric contact; a solver must convert that contact into velocity changes while respecting unilateral nonpenetration, friction and body coupling.

## Normal impulse

For a scalar contact normal with relative normal velocity

$$
v_n = (v_B-v_A)cdot n,
$$

an approaching contact has $v_n<0$ under this convention. Ignoring rotation for the smallest reference case, the effective inverse mass is

$$
k = m_A^{-1}+m_B^{-1}.
$$

A perfectly inelastic impulse that removes approach is

$$
j_n=maxleft(0,-rac{v_n}{k}ight).
$$

The nonnegative clamp expresses the unilateral constraint: a contact may push bodies apart but should not pull separated bodies together.

## Coulomb friction

A tangential impulse first computes the unconstrained velocity-cancelling value, then clamps it by the normal support:

$$
|j_t|lemu j_n.
$$

This is the simplest box/line approximation to Coulomb friction in one tangent direction. A full 3D solver needs two tangent directions or a friction-cone approximation and rotational effective mass.

## Why sequential iteration is needed

One impulse changes velocities used by neighboring contacts. In a stack, solving ground-to-lower body and then lower-to-upper body disturbs the first constraint again. Repeated Gauss-Seidel-like sweeps propagate support through the stack and converge toward mutually compatible velocities. Warm starting, ordering and stabilization affect convergence speed.

## Contact islands

Contacts and joints form a graph. Union-find can group connected dynamic bodies into independent **islands**. Different islands may be solved in parallel because no constraint couples their velocities. Sleeping and wake-up policy usually operates on the same connectivity.

## Executable reference

`tools/labs/constraint_friction_island_lab.cpp` verifies union-find island separation, unilateral normal impulse, Coulomb tangent clamping, and convergence of a tiny two-body stack over repeated sequential sweeps.

The witness deliberately excludes angular Jacobians, restitution thresholds, multidirectional friction, warm-start scaling and current production-engine parity.

Related: [[Impulse and force timing]], [[Continuous collision detection]], [[Center of mass and inertia]].
