# Impulse and force timing

Force and impulse are related but they belong to different timing contracts.

Force is a rate of momentum change:

$$
F=\frac{dp}{dt}.
$$

Impulse integrates force over time:

$$
J=\int_{t_0}^{t_1}F(t)\,dt
=
\Delta p.
$$

For constant mass and a linear impulse applied directly to the center of mass,

$$
\Delta v=M^{-1}J.
$$

There is **no additional timestep factor** when applying an impulse that is already an integrated momentum change. Multiplying by $\Delta t$ again turns the update into the wrong units.

For a force held constant through a numerical step,

$$
J\approx F\Delta t,
$$

so the timestep enters when converting the force into an approximate impulse.

## Off-centre impulse

An impulse at world-space point $p$ relative to center of mass $x$ has lever arm

$$
r=p-x.
$$

Angular impulse is

$$
\Delta L=r\times J.
$$

With world inverse inertia $I^{-1}$,

$$
\Delta\omega=I^{-1}(r\times J).
$$

A contact solver uses this coupling when computing the effective mass seen along a contact direction.

## Frame event versus physical force

A “jump button pressed” event should not necessarily apply the same force every rendered frame while the button remains down. A one-shot impulse, a force over a defined simulation interval, and a controller velocity target are different game/physics contracts.

The same issue appears with explosions, springs and contact responses.

## Fixed step

Simulation forces should normally be accumulated and integrated on the fixed simulation clock. Rendering interpolation does not apply physics again. If multiple rendering frames occur between physics ticks, repeating a force per render frame introduces frame-rate dependence.

## Verification

Test the same physical scenario with different render rates but the same fixed simulation step. For impulses, verify momentum change directly. For forces, integrate over equal physical time and compare trajectories/convergence order.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From the push on a panel to a repeatable rigid-body simulator]].

Related: [[Center of mass and inertia]], [[Time integrators and symplectic updates]].

## Constraint-level continuation

Forces change momentum over time; collision impulses change momentum at a discrete event. `tools/labs/constraint_friction_island_lab.cpp` now connects that distinction to a minimal sequential-impulse solver: an approaching normal velocity produces a nonnegative impulse that removes approach, a tangential impulse is clamped by $|j_t|\le\mu j_n$, and repeated sweeps propagate support through a two-body stack.

The same witness builds contact islands with union-find. This makes a practical ownership boundary visible: contacts couple bodies inside an island, while disconnected islands may be solved independently. Angular effective mass, restitution thresholds, warm starting, sleeping and a full two-dimensional friction cone remain higher solver layers.

