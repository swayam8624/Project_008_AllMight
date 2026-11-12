# Center of mass and inertia

The center of mass is the mass-weighted average position. Using it as the dynamic origin removes translation–rotation cross terms, while the inertia tensor describes resistance to rotation about that origin. A collider origin or visual pivot need not equal the center of mass. Physical resizing at fixed density changes mass and inertia; merely changing units does not change the physical body.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From the push on a panel to a repeatable rigid-body simulator]].

## Mastery expansion — translation and rotation need different mass summaries

The center of mass is the mass-weighted mean position. Relative to the center of mass, the inertia tensor summarizes resistance to angular acceleration:

$$
I=\int (\|r\|^2\mathbf{1}-rr^T)\,dm.
$$

It depends on mass distribution and reference point, not just total mass. Moving an inertia tensor to a \parallel axis requires the parallel-axis theorem; rotating it into world coordinates requires (I_W=R I_B R^T).

A solver often stores inverse mass and inverse inertia because impulses map directly to velocity changes. Static/infinite-mass bodies use zero inverse mass/inertia by policy.

Degenerate shapes and near-zero principal inertias need validation before inversion.

**Required next witness:** compute a box inertia analytically, rotate it, apply an off-center impulse and verify linear/angular momentum changes against the matrix formula.

