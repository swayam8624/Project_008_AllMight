# Impulse and force timing

Force changes momentum over an interval; impulse is the integrated momentum change. For constant mass, a linear impulse changes velocity by inverse mass times impulse, with no additional timestep factor. Off-centre application also changes angular momentum through the lever-arm cross product. Repeated frame application and fixed-step event application are distinct contracts.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From the push on a panel to a repeatable rigid-body simulator]].

## Mastery expansion — force integrates over time, impulse changes momentum directly

Force has units of momentum per time. An impulse (J=\int F\,dt) has units of momentum and changes linear momentum by (\Delta p=J). For a rigid body, an impulse at point (r) relative to the center of mass also changes angular momentum by (\Delta L=r\times J).

A force generator belongs inside the integration interval and is multiplied/integrated by the timestep. A collision impulse is usually computed from a contact constraint and applied as an instantaneous momentum correction; multiplying that impulse by (dt) again is dimensionally wrong.

Variable timestep changes the numerical behavior of force integration and constraint stabilization. Therefore logs/tests must state the timestep and whether a quantity is force, acceleration, impulse, velocity change or positional correction.

**Required next witness:** compare equal total impulse delivered as a short force pulse versus an instantaneous impulse and verify matching momentum under a controlled integrator.

