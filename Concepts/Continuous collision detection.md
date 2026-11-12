# Continuous collision detection

Continuous collision detection asks whether moving shapes meet during an interval, rather than checking only the endpoints. A time-of-impact result must state the motion model and the conservative bounds used. Advancement based only on translational closing speed can miss rotational motion; iteration caps and uncertain convergence need explicit outcomes.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From a Scene to a Collision Answer: Geometry as a Chain of Contracts]].

## Mastery expansion — solve for first time of contact, not just overlap

Discrete collision detection answers whether shapes overlap at sampled times. Continuous collision detection asks whether their swept motion intersects during the interval and, when needed, estimates the earliest time of impact (TOI).

For simple relative-motion pairs, the problem can reduce to a polynomial root. A moving sphere against a stationary sphere becomes a ray-versus-expanded-sphere problem in relative space. More complex rotating convex bodies generally require conservative advancement, swept bounds or specialized formulations.

A trustworthy CCD API distinguishes:

- no hit during ([0,\Delta t]);
- initial overlap;
- touching at the interval boundary;
- a valid interior TOI;
- numerical failure/non-convergence.

The returned normal/contact geometry must be evaluated consistently at the impact configuration, not at the end-of-step overlap.

Conservative methods may report a time slightly earlier than true contact to preserve safety. That is different from an exact root and should be documented as such.

**Required next witness:** randomized differential tests for analytic simple pairs, high-speed tunneling fixtures, grazing/tangent cases, initial-overlap policy, and a later extension to rotating 3-D convex conservative advancement.

