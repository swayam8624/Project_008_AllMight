# Continuous collision detection

Discrete collision detection tests shapes at selected instants. If an object moves from one side of a thin obstacle to the other between two simulation states, both endpoint tests can be non-overlapping even though the swept motion intersects. Continuous collision detection asks about the whole interval.

A CCD result is only meaningful after the **motion model** is stated: constant linear velocity, linear interpolation of transforms, rigid rotation, acceleration bound, deforming vertices, and so on.

## Time of impact

For simple primitives, time of impact can sometimes be solved analytically. For example, sphere-sphere CCD under constant relative velocity reduces to a quadratic in time.

For general convex shapes, conservative advancement repeatedly estimates separation distance $d$ and an upper bound on closing speed $v_c$, then advances by a safe amount related to

$$
\Delta t\le \frac{d}{v_c}.
$$

The bound must account for all motion capable of reducing separation. Using only translational relative speed can be unsafe for rotating shapes whose extremal points sweep quickly.

## Conservative versus exact

A conservative CCD algorithm may report contact slightly early but should not step past a true collision under its assumptions. Early contact changes simulation, so conservatism still has a measurable quality cost.

Iteration limits need an explicit outcome: separated, hit, no convergence/uncertain. Returning “no collision” when the solver simply exhausted iterations turns a performance cap into tunneling.

## Robustness

Near tangency, distance and time estimates become sensitive. Scale-aware tolerances should relate to object size, velocity/time scale and numerical precision. A single global epsilon is rarely correct.

Initial overlap is also a separate case: TOI = 0 may be meaningful, but penetration recovery and contact normal are not automatically solved by a sweep routine.

## Verification

Differential tests can compare an optimized CCD against dense time sampling for cases where dense sampling is a reliable conservative oracle, plus analytic primitive cases. Property tests should vary scale, velocity, near-tangent trajectories and iteration limits.

The repository already contains disk CCD/swept broadphase witnesses; those are not proof of arbitrary rotating convex CCD.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a Scene to a Collision Answer: Geometry as a Chain of Contracts]].

Related: [[GJK and EPA]], [[Incremental contact penetration refresh]].
