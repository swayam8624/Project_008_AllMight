# Center of mass and inertia

The center of mass is the point at which translational motion can be separated cleanly from rotational motion for a rigid body.

For point masses $m_i$ at positions $r_i$,

$$
r_{cm}=\frac{1}{M}\sum_i m_i r_i,
\qquad
M=\sum_i m_i.
$$

For a continuous density $\rho(r)$,

$$
r_{cm}=\frac{1}{M}\int r\rho(r)\,dV.
$$

A visual pivot, modeling origin or collider-local origin does not have to equal the center of mass.

## Inertia tensor

Relative to center of mass, the inertia tensor is

$$
I=
\int
\left(
\|r\|^2\mathbf{1}-rr^{\mathsf T}
\right)dm.
$$

It maps angular velocity to angular momentum in body coordinates:

$$
L=I\omega
$$

when the tensor and vector are expressed in compatible frames.

In world space, if body orientation is rotation $R$,

$$
I_{world}=RI_{body}R^{\mathsf T},
$$

and similarly for the inverse inertia when invertible.

## Parallel-axis theorem

Moving the reference point away from the center of mass changes inertia:

$$
I_P=
I_{cm}
+
M\left(
\|d\|^2\mathbf{1}-dd^{\mathsf T}
\right),
$$

where $d$ is the displacement between reference points.

This is why using a collider-origin inertia as though it were a center-of-mass inertia couples translation and rotation incorrectly.

## Scaling

Changing units does not change the physical body. Numerically converting metres to centimetres changes coordinate values and unit labels consistently.

Physically resizing a uniform-density object does change mass and inertia. Under uniform length scale $s$ in 3D at fixed density, mass scales as $s^3$ and inertia as $s^5$.

## Impulse coupling

An impulse $J$ at lever arm $r$ changes angular velocity by

$$
\Delta\omega=I^{-1}_{world}(r\times J).
$$

This feeds directly into contact effective mass and constraint solving.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From the push on a panel to a repeatable rigid-body simulator]].

Related: [[Impulse and force timing]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Part 1637 — Composite-body mass properties|Mass properties and inertia tensor]].
