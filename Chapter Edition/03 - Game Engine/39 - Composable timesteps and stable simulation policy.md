# 39 — Composable timesteps and stable simulation policy

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift|Previous chapter]] · [[Chapter Edition/03 - Game Engine/40 - Coupled contacts and iterative response|Next chapter]]

A simulation often subdivides or combines work under frame deadlines, but the result depends on how state updates compose. This chapter tracks damping, fixed steps, and remainders as explicit policy choices.

**By the end:** Explain when timestep subdivision preserves a model and when parameterization or state updates change the result.

**Recall question:** If a step is split into two halves, which position, velocity, and damping results should match, and what assumptions make them differ?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 95878–99260; source-span SHA256 6a95d25e7c677e3626e6b5c9bd09678c94c2536fb3e1aff0d5984b4394d2677a. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Timestep composability

Two half steps:

$$
e^{-\lambda h/2}
e^{-\lambda h/2}
=
e^{-\lambda h}.
$$

Therefore changing the number of substeps does not alter pure exponential damping.

This is superior to:

$$
v\leftarrow v(1-\lambda h),
$$

which has timestep-dependent behavior and can become negative when:

$$
\lambda h>1.
$$

---

### Damping removes momentum and energy intentionally

Damping is an explicit non-conservative force model.

Momentum is not expected to remain conserved when damping exists because damping represents interaction with an external medium/environment.

Energy loss is intentional.

If you are testing conservation, disable damping.

That sounds obvious, but it is a common benchmark mistake.

---

### Part 1688 — Angular damping implementation

Kairo uses the analogous law:

$$
\boxed{
\omega'
=
\omega e^{-\lambda_ah}.
}
$$

This is a convenient gameplay model.

But physically, rotational drag is often more complicated.

Because:

$$
L=I\omega,
$$

an isotropic exponential decay of $\omega$ does not necessarily correspond to one physically derived torque law for an asymmetric rotating object.

It is better understood as:

$$
\boxed{
\text{engine angular-velocity decay policy}.
}
$$

That is fine.

---

### Equivalent damping torque in simple case

For constant scalar inertia I and:

$$
\dot\omega=-\lambda\omega,
$$

then:

$$
I\dot\omega
=
-\lambda I\omega.
$$

So damping torque:

$$
\boxed{
\tau_d
=
-\lambda I\omega.
}
$$

For a full inertia tensor:

$$
\tau_d
=
-\lambda I\omega
=
-\lambda L.
$$

An exact physically motivated implementation could apply a torque proportional to angular momentum.

Kairo instead directly decays $\omega$, which is numerically simple and predictable.

---

### Part 1689 — Energy in rigid-body motion

A rigid body's kinetic energy has translational and rotational components:

$$
\boxed{
T
=
T_{linear}
+
T_{rotational}.
}
$$

For a body in a potential field there may also be:

$$
U.
$$

Total mechanical energy:

$$
\boxed{
E=T+U.
}
$$

For gravity near Earth:

$$
U_g
=
mgy
$$

under the conventional y-up coordinate system.

For a spring:

$$
U_s
=
\frac12kx^2.
$$

Therefore a falling, bouncing, spring-connected rigid body may exchange energy among:

```text
gravitational potential
translational kinetic
rotational kinetic
spring potential
```

while damping/friction/restitution remove some.

Tracking these terms is one of the strongest ways to diagnose integrator behavior.

---

### Energy is scalar, not vector

Momentum has direction.

Energy does not.

Two bodies moving in opposite directions can have zero total momentum while both possess positive kinetic energy.

Example:

$$
m=1,
\qquad
v_A=1,
\qquad
v_B=-1.
$$

Total momentum:

$$
P=1-1=0.
$$

Total kinetic energy:

$$
T=
\frac12+
\frac12
=
1.
$$

So conservation of momentum and conservation of energy are separate tests.

---

### Part 1690 — Translational kinetic energy

For mass m and speed v:

$$
\boxed{
T_{linear}
=
\frac12m\|\mathbf v\|^2.
}
$$

Derivation comes from work-energy theorem.

Work:

$$
dW
=
\mathbf F\cdot d\mathbf x.
$$

Using:

$$
F=m\frac{dv}{dt}
$$

and:

$$
dx=vdt,
$$

we obtain:

$$
dW
=
m\frac{dv}{dt}vdt
=
mv\,dv.
$$

Integrate from zero to v:

$$
W
=
m\int_0^v u\,du
=
\frac12mv^2.
$$

Kairo's `LinearKineticEnergy()` implements this relation.

Current test verifies:

$$
m=2,
\quad
v=(3,0,0)
$$

gives:

$$
T=
\frac12(2)(9)=9.
$$

Correct.

---

### Why speed squared matters

Doubling speed multiplies energy by:

$$
4.
$$

That is why high-speed numerical explosions become catastrophic so quickly.

A velocity clamp from 1000 to 100 can remove 99% of energy because energy scales quadratically.

---

### Part 1691 — Rotational kinetic energy

For full 3D rigid-body rotation:

$$
\boxed{
T_{rot}
=
\frac12
\omega^T
I_w
\omega.
}
$$

Since:

$$
L=I_w\omega,
$$

you can also write:

$$
\boxed{
T_{rot}
=
\frac12
\omega\cdot L.
}
$$

In principal coordinates:

$$
I=
diag(I_1,I_2,I_3),
$$

therefore:

$$
T_{rot}
=
\frac12
(
I_1\omega_1^2+
I_2\omega_2^2+
I_3\omega_3^2
).
$$

Kairo's rotational energy helper computes:

```cpp
0.5f *
Dot(
    angularVelocity,
    worldInertia *
    angularVelocity);
```

which is exactly right.

---

### Static body energy

A static body's velocity should be zero.

So energy zero.

A kinematic body may have nonzero velocity and therefore mathematically has kinetic energy, but because it is externally driven with effectively infinite solver mass, it does not participate in a closed mechanical-energy conservation test in the same way.

A kinematic body is an **external actuator** from the dynamic system's perspective.

That distinction matters enormously when interpreting diagnostics.

---

### Part 1692 — Momentum conservation checks

For an isolated system:

$$
\boxed{
\mathbf P
=
\sum_i
m_i\mathbf v_i.
}
$$

If net external force is zero:

$$
\frac{dP}{dt}=0.
$$

So:

$$
P(t)=P(0).
$$

Similarly total angular momentum about a fixed inertial origin O is:

$$
\boxed{
\mathbf L_O
=
\sum_i
[
(\mathbf c_i-\mathbf O)\times m_i\mathbf v_i
+
I_i\omega_i
].
}
$$

The first term is orbital angular momentum of each COM.

The second is spin angular momentum around its COM.

With no external torque:

$$
\frac{dL_O}{dt}=0.
$$

---

### Collision momentum test

For a two-body collision with no gravity, damping, constraints to the world, or velocity clamps:

Before:

$$
P_{before}
=
m_Av_A+
m_Bv_B.
$$

After solver:

$$
P_{after}
=
m_Av_A'+
m_Bv_B'.
$$

Check:

$$
\boxed{
\|P_{after}-P_{before}\|
<
\epsilon.
}
$$

If your equal-and-opposite impulse application is correct, this should be extremely tight aside from float error.

---

### Why equal and opposite impulse guarantees linear momentum conservation

Body A gets:

$$
-J.
$$

Body B gets:

$$
+J.
$$

Momentum changes:

$$
\Delta P_A=-J
$$

and:

$$
\Delta P_B=+J.
$$

Therefore:

$$
\Delta P_{total}=0.
$$

Independent of masses.

That is one of the strongest invariants for contact-solver testing.

---

### When momentum is not supposed to be conserved

Do not run a momentum conservation assertion while:

```text
gravity is active,
drag/damping is active,
a kinematic body pushes the system,
a static wall receives no simulated reaction,
velocity clamps fire,
external forces act.
```

These introduce external impulse relative to the simulated dynamic subsystem.

A dynamic ball bouncing off an immovable static floor does **not** conserve the dynamic ball's momentum because the Earth/floor is effectively an external infinite-mass reservoir.

If you included the entire Earth, momentum would be conserved.

The simulation intentionally does not.

---

### Part 1693 — Energy drift in numerical integration

Even if the real physical system conserves energy:

$$
E(t)=constant,
$$

a numerical integrator may not.

Define drift:

$$
\boxed{
\Delta E(t)
=
E_{numerical}(t)-E_0.
}
$$

Relative drift:

$$
\boxed{
\delta_E
=
\frac{E(t)-E_0}
{\max(|E_0|,\epsilon)}.
}
$$

Plot this over time.

It is one of the clearest integrator diagnostics.

---

### Explicit Euler energy growth

For harmonic oscillator:

$$
\ddot x=-\omega^2x,
$$

explicit Euler injects energy.

The state spirals outward.

Semi-implicit Euler generally keeps the trajectory bounded for a reasonable timestep and causes energy to oscillate around the true energy.

That is why symplectic methods are preferred for long-running mechanical systems even when their formal local order is still low.

---

### Semi-implicit Euler does not conserve the exact Hamiltonian

This point matters.

Semi-implicit Euler is **symplectic**, not exactly energy conserving.

It approximately conserves a modified nearby Hamiltonian.

Therefore you often see bounded oscillatory energy error rather than exact constant energy.

That is qualitatively much better than secular explosion, but still numerical drift.

---

### Damping versus drift

If damping is active, energy should decrease.

That is not "drift."

It is modelled dissipation.

Separate:

$$
\boxed{
\text{physical/modelled energy change}
}
$$

from:

$$
\boxed{
\text{numerical energy error}.
}
$$

A test must know the analytic/model expectation.

---

### Collision restitution

Restitution:

$$
e=1
$$

ideally gives an elastic normal collision.

But friction, rotational coupling, multiple contacts and iterative solver approximations complicate total kinetic-energy conservation.

For:

$$
e<1,
$$

energy loss is intentional.

Again, conservation checks must be matched to the scenario.

---

### Energy-error experiment you should build

Take one undamped body under gravity with no collisions.

Total:

$$
E
=
\frac12mv^2+
mgy.
$$

Run:

$$
h=\frac1{30},
\frac1{60},
\frac1{120},
\frac1{240}.
$$

Measure:

$$
\max_t
\left|
\frac{E(t)-E_0}{E_0}
\right|.
$$

A first-order method should show approximately linear convergence as h decreases.

That gives actual evidence for integration accuracy.

For Reality Probe-style research this is much more useful than saying:

> It looked stable.

---

### Part 1694 — Determinism concerns in rigid-body simulation

A deterministic simulation means conceptually:

$$
\boxed{
State_{n+1}
=
F(State_n,Inputs_n)
}
$$

and repeated execution with identical initial state/input produces the same output.

But "deterministic" has several levels.

Same executable on same machine is easiest.

Same compiler on different machines is harder.

Different compilers on same architecture is harder still.

Cross-architecture bitwise determinism is substantially harder.

---

### Floating-point non-associativity

Real numbers satisfy:

$$
(a+b)+c
=
a+(b+c).
$$

Floating point may not.

Example conceptually:

$$
a=10^{20},
\quad
b=-10^{20},
\quad
c=1.
$$

One ordering:

$$
(a+b)+c
=
0+1
=
1.
$$

Another:

$$
a+(b+c)
$$

may round $b+c$ back to $-10^{20}$, giving:

$$
0.
$$

Therefore changing reduction/order can change results.

---

### Sequential impulse ordering

Suppose contacts:

$$
C_1,C_2,C_3.
$$

Gauss-Seidel solver updates velocity after each constraint.

Order:

$$
C_1\rightarrow C_2\rightarrow C_3
$$

can produce a slightly different approximate result from:

$$
C_3\rightarrow C_2\rightarrow C_1.
$$

With many frames those differences can diverge.

Therefore stable contact ordering matters.

Kairo already deliberately sorts several contact/event structures deterministically.

That is a strong foundation.

---

### Parallelism

If solver islands are independent, they can run in parallel safely without cross-island numerical ordering interactions.

Within one connected island, naïvely parallelizing constraints introduces races and nondeterministic update order.

Kairo exposes:

```cpp
EnableParallelIslands
```

which is a much cleaner parallel boundary than arbitrary parallel contact solving.

Independent islands are naturally isolated.

---

### FMA and compiler differences

A compiler may implement:

$$
a*b+c
$$

as one fused operation:

$$
FMA(a,b,c)
$$

with one rounding instead of multiply rounding plus add rounding.

Both are valid numerically but produce different bits.

Similarly SIMD vectorization can change summation order.

Therefore exact bitwise determinism may require restrictive compiler/floating-point settings.

---

### Denormals, FTZ and DAZ

Very small floating values may be treated differently depending on hardware/runtime modes such as:

```text
flush-to-zero,
denormals-are-zero.
```

Those differences can eventually alter sleeping thresholds or constraint paths.

For strict determinism, floating-point environment itself becomes part of simulation configuration.

---

### Unordered containers

Never let:

```cpp
for (auto& [key,value] : unordered_map)
```

define physics constraint order if deterministic iteration is required.

Hash iteration order can differ.

Use stable arrays/sorted keys when order affects numerical results.

We saw this same principle in ECS determinism earlier.

---

### Kairo's replay architecture is exactly the correct diagnostic

`PhysicsReplay` records:

$$
FixedDt
$$

plus initial:

$$
PhysicsWorldSnapshot
$$

and frame commands.

After every step it records:

$$
ExpectedStateHash.
$$

Verification restores the initial snapshot, replays identical commands, steps at the same fixed dt, then compares:

$$
ActualStateHash.
$$

The hash is computed over the serialized deterministic world snapshot using FNV-1a.

This gives an immediate divergence frame.

That is excellent engineering.

---

### State hash is not proof of cross-platform mathematical equivalence

A raw serialized-state hash requires byte equality.

Two states differing by:

$$
10^{-7}
$$

in one float produce unrelated hashes.

So state hashing is excellent for:

```text
same-build regression,
exact replay,
detecting first divergence.
```

For cross-platform scientific comparisons you may additionally need tolerant metrics:

$$
\|x_A-x_B\|,
\quad
\|v_A-v_B\|,
\quad
\angle(q_A,q_B).
$$

Both tools are useful.

Exact hashes and numerical tolerances answer different questions.

---

### Part 1695 — Floating-point scale and unit choice

Floating-point accuracy is **relative**.

Single precision has roughly seven decimal digits of precision.

If coordinates are around:

$$
1,
$$

tiny differences are representable.

If coordinates are around:

$$
10^7,
$$

spacing between representable floats becomes large enough that centimetre-scale motion may disappear.

Therefore scale is not cosmetic.

---

### Extremely tiny objects are also problematic

Suppose object dimensions:

$$
10^{-6}m.
$$

Then penetration slop:

$$
0.01m
$$

is ten thousand times the object size.

Your collision tolerances become meaningless.

Broadphase fattening and normalization epsilons may dominate the geometry.

So both enormous and tiny scale are dangerous.

---

### Dimensionless numerical ratios matter

Solver quality often depends more on ratios than absolute numbers.

Examples:

$$
\frac{largestMass}{smallestMass},
$$

$$
\frac{objectSpeed\cdot dt}{smallestFeatureSize},
$$

$$
\frac{penetrationSlop}{objectSize},
$$

$$
\omega dt.
$$

These give better stability intuition than isolated values.

---

### Mass ratios

Suppose:

$$
m_A=1kg
$$

and:

$$
m_B=10^9kg.
$$

Then:

$$
m_A^{-1}=1
$$

while:

$$
m_B^{-1}=10^{-9}.
$$

Numerically, one body is almost static compared with the other.

Large mass ratios can make iterative constraints converge poorly.

Many game physics engines recommend limiting ratios inside connected stacks/joints.

---

### Part 1696 — SI units in a physics engine

Kairo's current conventions already strongly imply SI-like units:

$$
\boxed{
1\ position\ unit=1m
}
$$

$$
\boxed{
time=seconds
}
$$

$$
\boxed{
mass=kg
}
$$

$$
\boxed{
linear\ velocity=m/s
}
$$

$$
\boxed{
angular\ velocity=rad/s
}
$$

$$
\boxed{
force=N
}
$$

$$
\boxed{
torque=N\,m
}
$$

and standard gravity:

$$
9.80665m/s^2.
$$

This is the right default.

---

### Full dimensional table

| Quantity | Unit |
|---|---|
| Position | m |
| Time | s |
| Mass | kg |
| Linear velocity | m/s |
| Linear acceleration | m/s² |
| Force | N = kg·m/s² |
| Impulse | N·s = kg·m/s |
| Angular velocity | rad/s |
| Angular acceleration | rad/s² |
| Torque | N·m |
| Inertia | kg·m² |
| Angular momentum | kg·m²/s |
| Energy | J = kg·m²/s² |
| Density | kg/m³ |

Dimensional analysis is one of your cheapest correctness tools.

If code adds:

$$
Velocity+Force,
$$

something is conceptually wrong.

If an impulse is multiplied by dt, something is wrong.

If inertia has units kg rather than kg·m², something is wrong.

---

### Why SI helps tooling and research

If:

```text
1 unit = 1 metre
```

then an authored sphere radius:

```text
0.5
```

means:

$$
0.5m.
$$

Gravity:

```text
-9.80665
```

has its standard interpretation.

Density:

```text
1000
```

means approximately water in:

$$
kg/m^3.
$$

No mysterious conversion constant is required in every equation.

For Vulkax/Reality Probe-style experiments this is especially valuable because measured real-world quantities map directly into simulation parameters.

---

### You can render in another scale without changing physics units

Suppose your graphics world convention conceptually wants centimetres.

Do not necessarily rewrite the physics engine.

Bridge:

$$
100\ renderer\ units
=
1\ physics\ metre.
$$

But every conversion boundary must be explicit.

It is usually much cleaner for a unified engine to keep world/physics transforms in metres and let content pipelines adapt source-unit conventions.

---

### Part 1697 — Debug visualization of center of mass and inertia

A physics debug renderer should not only draw collider wireframes.

For rigid-body dynamics, the most useful diagnostic images often show:

$$
COM,
$$

$$
principal\ axes,
$$

$$
force,
$$

$$
torque,
$$

$$
linear\ velocity,
$$

$$
angular\ velocity.
$$

Kairo's current `PhysicsDebug.cppm` already exposes renderer-neutral value objects for collider shapes, AABBs and contacts.

That is good architecture.

But it currently does **not** expose a dedicated COM/inertia visualization record.

So M102 exposes a real extension opportunity.

---

### Center-of-mass marker

For a COM-centered runtime state:

$$
c_w=
State.Position.
$$

Draw a small cross or sphere.

For body-origin-based authoring:

$$
c_w
=
x_{origin}
+
R c_{local}.
$$

Showing both:

```text
authoring origin
COM
```

with a line between them is extraordinarily useful for debugging asymmetric compound bodies.

---

### Principal axes visualization

Suppose local inertia tensor eigendecomposes:

$$
I_b=QDQ^T.
$$

Columns of Q are principal axes.

Transform them to world:

$$
e_i^w=R e_i^b.
$$

Draw three colored lines from COM:

```text
X principal axis
Y principal axis
Z principal axis.
```

This immediately shows whether the tensor's orientation matches the shape.

---

### How long should inertia axes be?

You can scale inversely with square root of principal moment:

$$
L_i
\propto
\frac1{\sqrt{I_i}}.
$$

Then an axis that is easy to rotate around appears visually longer.

Or use:

$$
L_i\propto\sqrt{I_i/M}
$$

which has dimensions of length and corresponds roughly to a radius of gyration:

$$
\boxed{
k_i
=
\sqrt{\frac{I_i}{M}}.
}
$$

Radius of gyration is physically meaningful.

It answers:

> At what equivalent radius could I concentrate all mass to produce the same moment of inertia?

So a beautiful debug representation is:

$$
AxisLength_i
=
Scale
\sqrt{\frac{I_i}{M}}.
$$

---

### Inertia ellipsoid

A more sophisticated visualization uses the inertia ellipsoid.

For direction n:

$$
I_n=n^TIn.
$$

You can display an ellipsoid aligned to principal axes whose radii encode inertia or inverse inertia.

This gives an immediate visual sense of rotational resistance.

For a sphere:

all axes equal.

For a long rod:

one principal moment differs dramatically.

For an irregular compound:

axes may be rotated relative to geometry.

---

### Force and torque arrows

Force arrow at application point:

$$
p\rightarrow p+sF.
$$

Torque arrow from COM:

$$
c\rightarrow c+s_\tau\tau.
$$

For an off-center force, showing:

```text
lever arm r,
force F,
torque r×F
```

simultaneously makes incorrect COM selection immediately obvious.

This is exactly the kind of debug tool that saves hours.

---

### Suggested Kairo debug contract

A renderer-neutral addition could look like:

```cpp
struct DebugRigidBodyDynamics
{
    BodyID Body = InvalidBodyID;

    Vec3f CenterOfMass =
        Vec3f::Zero();

    Vec3f LinearVelocity =
        Vec3f::Zero();

    Vec3f AngularVelocity =
        Vec3f::Zero();

    Vec3f AccumulatedForce =
        Vec3f::Zero();

    Vec3f AccumulatedTorque =
        Vec3f::Zero();

    std::array<Vec3f, 3>
        PrincipalAxesWorld;

    Vec3f PrincipalMoments =
        Vec3f::Zero();

    bool Sleeping = false;
};
```

`PhysicsDebug` should produce the values.

Renderer decides colours/line thickness.

The physics library should remain renderer-independent, preserving the current architecture.

---

### Part 1698 — Unit tests for mass properties

Mass-property tests should not merely check:

```text
mass > 0.
```

They should assert mathematical invariants.

Current Kairo tests already cover sphere/box/capsule/static mass, transformed inverse inertia, parallel-axis shift and broad scale certification.

That is a good base.

We can make the suite considerably stronger.

---

### Sphere tests

For radius r and density $\rho$:

$$
m
=
\rho
\frac43\pi r^3.
$$

Then:

$$
I_x=I_y=I_z
=
\frac25mr^2.
$$

Test exact formula.

Also test scaling.

If radius doubles at fixed density:

$$
m'=
8m
$$

and:

$$
I'=
32I.
$$

That catches unit/scaling mistakes beautifully.

---

### Box test

Half extents:

$$
(a,b,c).
$$

Mass:

$$
m=8abc\rho.
$$

Tensor:

$$
I_x
=
\frac m3(b^2+c^2)
$$

$$
I_y
=
\frac m3(a^2+c^2)
$$

$$
I_z
=
\frac m3(a^2+b^2).
$$

Current test with:

$$
(a,b,c)=(1,2,3),
\quad
\rho=0.5
$$

correctly obtains:

$$
m=24
$$

and:

$$
I_x=104.
$$

Good.

---

### Symmetry property test

For every generated mass property:

$$
\boxed{
I_{ij}\approx I_{ji}.
}
$$

Explicitly verify all off-diagonal pairs.

This catches accidental matrix-convention bugs.

---

### Positive inertia test

For any nonzero test vector n:

$$
\boxed{
n^TIn>0
}
$$

for a proper dynamic 3D body.

You can sample many deterministic n directions.

Stronger implementation:

```text
Cholesky decomposition succeeds
```

or eigenvalues all positive.

---

### Inverse consistency

For a nonsingular inertia tensor:

$$
\boxed{
II^{-1}\approx I_3.
}
$$

This is especially important because Pass 16 found a real gap: Kairo's generic `MakeMassProperties()` currently uses `InverseDiagonalInertia()` even though a supplied tensor may contain off-diagonal terms.

A test with a non-diagonal physical tensor would expose this immediately.

For example:

$$
I=
\begin{bmatrix}
2&-1&0\\
-1&2&0\\
0&0&4
\end{bmatrix}.
$$

Check:

$$
II^{-1}\approx I_3.
$$

The current diagonal-only inverse would fail.

That test should be added before compound-body generalization.

---

### Parallel-axis theorem test

Start with known:

$$
I_C.
$$

Shift by d:

$$
I_P
=
I_C+
m(d^2I-dd^T).
$$

Test an offset that is **not axis aligned**, for example:

$$
d=(1,2,3).
$$

Why?

Axis-aligned tests only exercise diagonal additions.

An arbitrary d produces products of inertia:

$$
-d_xd_y,
\quad
-d_xd_z,
\quad
-d_yd_z.
$$

That is precisely where generic tensor bugs hide.

---

### COM tests

For parts:

$$
m_1,m_2,\ldots
$$

verify:

$$
c
=
\frac{\sum m_ic_i}{\sum m_i}.
$$

Then verify:

$$
\sum_i
m_i(c_i-c)
\approx0.
$$

That second equation is a strong invariant.

It proves c truly is the combined center of mass.

---

### Principal-moment triangle inequalities

For physical inertia around COM:

$$
I_x\le I_y+I_z
$$

and cyclic permutations.

Include this as a sanity assertion for generated primitive/compound mass properties.

It catches impossible tensors.

---

### Part 1699 — Unit tests for impulse response

Impulse tests should verify both local formulas and system conservation.

Current Kairo already has an excellent certification loop for COM impulses across many masses:

$$
\Delta v
=
m^{-1}J
$$

and:

$$
\Delta\omega=0.
$$

That is good.

Now extend systematically.

---

### Center-of-mass impulse test

Initial:

$$
v=0,
\quad
\omega=0.
$$

Impulse J at COM.

Expected:

$$
v'=m^{-1}J.
$$

Expected:

$$
\omega'=0.
$$

Exact conceptual test.

---

### Off-center impulse test

Choose:

$$
r=(1,0,0),
$$

$$
J=(0,2,0),
$$

$$
I^{-1}=I_3.
$$

Then:

$$
r\times J
=
(0,0,2).
$$

So:

$$
\Delta\omega=(0,0,2).
$$

Current PhysicsMath test effectively checks this style of response.

---

### Parallel impulse test

If:

$$
J\parallel r,
$$

then:

$$
r\times J=0.
$$

Therefore no angular velocity change.

That catches incorrect cross-product ordering.

If implementation accidentally computes:

$$
J\times r,
$$

the sign will be reversed.

---

### Static impulse test

For:

$$
m^{-1}=0
$$

and:

$$
I^{-1}=0,
$$

after any finite impulse:

$$
v'=v,
\qquad
\omega'=\omega.
$$

Kinematic should also ignore solver/gameplay impulse under Kairo's semantics.

---

### Equal-and-opposite pair test

Body A receives:

$$
-J.
$$

Body B receives:

$$
+J.
$$

Compute before/after total linear momentum.

Require:

$$
\boxed{
P_{before}\approx P_{after}.
}
$$

This tests actual two-body impulse application rather than one helper in isolation.

---

### Angular momentum conservation pair test

For an internal contact at common world point p, compute total angular momentum about a fixed origin O before and after.

Require:

$$
L_{before}\approx L_{after}.
$$

This is significantly stronger because it simultaneously tests:

```text
linear impulse,
angular impulse,
lever arms,
world-space inertia.
```

---

### Energy test for elastic 1D collision

Two equal masses:

$$
m_1=m_2=1.
$$

Before:

$$
v_1=1,\quad v_2=0.
$$

Perfectly elastic head-on collision:

$$
e=1.
$$

Expected:

$$
v_1'=0,
\quad
v_2'=1.
$$

Momentum:

$$
1
$$

before and after.

Energy:

$$
\frac12
$$

before and after.

This is an excellent full-response regression.

---

### Inelastic collision test

With:

$$
e=0,
$$

equal masses and same starting velocities, normal kinetic energy should decrease while momentum remains conserved.

That confirms the solver is dissipating energy intentionally rather than losing momentum.

---

### Part 1700 — Implementing a standalone RigidBody integrator and proving every update mathematically

Now we assemble the whole phase.

We want a standalone rigid body that contains:

$$
x,q,v,\omega,
$$

$$
m^{-1},
\quad
I_b,
\quad
I_b^{-1},
$$

$$
F,\tau,
$$

gravity/damping/speed policies.

We will make one deliberate choice:

**The integrator will support the full gyroscopic Euler rigid-body equation**, because this is the mathematically complete version. I will also show exactly where a gameplay engine may choose Kairo's simplified approximation.

We will use:

$$
State.Position
=
WorldCenterOfMass.
$$

That removes the COM ambiguity found in Pass 16.

---

### The standalone data model

```cpp
#include <algorithm>
#include <cmath>
#include <stdexcept>

// Assume these are Kairo-style math types:
//
// Vec3f
// Mat3f
// Quatf
//
// with:
// Cross(a,b)
// Dot(a,b)
// Transpose(M)
// ToMatrix3(q)
// Rotate(q,v)
// Matrix/vector multiplication.

struct RigidBodyState
{
    // World-space center of mass.
    Vec3f Position = Vec3f::Zero();

    // Body-local orientation -> world-space orientation.
    Quatf Rotation = Quatf::Identity();

    // World-space linear velocity in metres / second.
    Vec3f LinearVelocity = Vec3f::Zero();

    // World-space angular velocity in radians / second.
    Vec3f AngularVelocity = Vec3f::Zero();
};

struct RigidBodyMass
{
    // Physical mass in kg.
    float Mass = 1.0f;

    // Precomputed reciprocal.
    float InverseMass = 1.0f;

    // Inertia about the center of mass, expressed
    // in the body's local coordinate system.
    Mat3f LocalInertia = Mat3f::Identity();

    // Precomputed inverse of LocalInertia.
    //
    // IMPORTANT:
    // This must be the full inverse if LocalInertia
    // contains off-diagonal terms.
    Mat3f LocalInverseInertia =
        Mat3f::Identity();
};

struct RigidBodyForces
{
    // World-space continuous force accumulated
    // for the next integration step.
    Vec3f Force = Vec3f::Zero();

    // World-space continuous torque accumulated
    // for the next integration step.
    Vec3f Torque = Vec3f::Zero();
};

struct RigidBody
{
    RigidBodyState State;
    RigidBodyMass Mass;
    RigidBodyForces Forces;

    bool Dynamic = true;

    bool EnableGravity = true;
    float GravityScale = 1.0f;

    // Exponential decay rates in 1 / second.
    float LinearDamping = 0.0f;
    float AngularDamping = 0.0f;

    // Numerical safety ceilings.
    float MaxLinearSpeed = 1000.0f;
    float MaxAngularSpeed = 1000.0f;
};
```

The state is intentionally small and explicit.

No collider is required to derive basic Newton-Euler integration.

Collision response is a separate layer.

---

### World-space inertia

Local inertia is fixed:

$$
I_b.
$$

Orientation matrix:

$$
R.
$$

Therefore:

$$
\boxed{
I_w
=
RI_bR^T
}
$$

and:

$$
\boxed{
I_w^{-1}
=
RI_b^{-1}R^T.
}
$$

Code:

```cpp
Mat3f WorldInertia(
    const RigidBody& body)
{
    const Mat3f R =
        ToMatrix3(
            body.State.Rotation);

    return
        R *
        body.Mass.LocalInertia *
        Transpose(R);
}

Mat3f WorldInverseInertia(
    const RigidBody& body)
{
    const Mat3f R =
        ToMatrix3(
            body.State.Rotation);

    return
        R *
        body.Mass.LocalInverseInertia *
        Transpose(R);
}
```

No inversion occurs each frame.

We precomputed local inverse once.

---

### Force accumulation

Continuous force:

```cpp
void AddForce(
    RigidBody& body,
    const Vec3f& force)
{
    body.Forces.Force +=
        force;
}
```

Continuous torque:

```cpp
void AddTorque(
    RigidBody& body,
    const Vec3f& torque)
{
    body.Forces.Torque +=
        torque;
}
```

Force at point:

```cpp
void AddForceAtPoint(
    RigidBody& body,
    const Vec3f& force,
    const Vec3f& worldPoint)
{
    // Translational effect.
    body.Forces.Force +=
        force;

    // Lever arm measured from COM.
    const Vec3f r =
        worldPoint -
        body.State.Position;

    // Rotational effect.
    body.Forces.Torque +=
        Cross(r, force);
}
```

Why?

Because:

$$
\tau=r\times F.
$$

---

### Immediate impulse response

```cpp
void ApplyImpulseAtPoint(
    RigidBody& body,
    const Vec3f& impulse,
    const Vec3f& worldPoint)
{
    if (!body.Dynamic)
        return;

    // Linear momentum:
    //
    // J = Δp = m Δv
    //
    // => Δv = J / m.
    body.State.LinearVelocity +=
        impulse *
        body.Mass.InverseMass;

    const Vec3f r =
        worldPoint -
        body.State.Position;

    // Angular impulse:
    //
    // ΔL = r × J
    const Vec3f angularImpulse =
        Cross(r, impulse);

    // Δω = I^-1 ΔL.
    body.State.AngularVelocity +=
        WorldInverseInertia(body) *
        angularImpulse;
}
```

No `dt`.

That is not an implementation convenience.

It follows directly from:

$$
J=\Delta p.
$$

---

### Velocity clamp

```cpp
void ClampMagnitude(
    Vec3f& value,
    float maximum)
{
    const float lengthSq =
        value.LengthSquared();

    const float maxSq =
        maximum * maximum;

    if (lengthSq > maxSq)
    {
        value *=
            maximum /
            std::sqrt(lengthSq);
    }
}
```

We explicitly understand this as:

$$
\text{numerical policy},
$$

not Newtonian physics.

---

### Exponential-map orientation update

Rather than Kairo's current first-order quaternion derivative integration, our final standalone integrator can use the finite rotation exponential.

```cpp
Quatf IntegrateOrientation(
    const Quatf& q,
    const Vec3f& worldAngularVelocity,
    float dt)
{
    const float speed =
        worldAngularVelocity.Length();

    // For tiny angles, use first-order expansion.
    //
    // This avoids division by an almost-zero speed.
    if (speed < 1.0e-7f)
    {
        const Quatf omega{
            worldAngularVelocity,
            0.0f
        };

        return (
            q +
            omega *
            q *
            (0.5f * dt)
        ).Normalized();
    }

    const Vec3f axis =
        worldAngularVelocity /
        speed;

    const float angle =
        speed * dt;

    const float halfAngle =
        0.5f * angle;

    const float sinHalf =
        std::sin(halfAngle);

    const Quatf delta{
        axis * sinHalf,
        std::cos(halfAngle)
    };

    // World-space omega means pre-multiply.
    return (
        delta *
        q
    ).Normalized();
}
```

For constant $\omega$, this represents the exact finite orientation increment.

---

### Full Newton-Euler force integration

Translation:

$$
a
=
g+
m^{-1}F.
$$

Rotation requires:

$$
\tau
=
I_w\alpha+
\omega\times(I_w\omega).
$$

Therefore:

$$
\boxed{
\alpha
=
I_w^{-1}
[
\tau-
\omega\times(I_w\omega)
].
}
$$

Code:

```cpp
void IntegrateForces(
    RigidBody& body,
    const Vec3f& gravity,
    float dt)
{
    if (!body.Dynamic)
        return;

    // --------------------------------------------------------
    // LINEAR NEWTON EQUATION
    //
    // F = m a
    //
    // a = F / m
    //
    // Gravity is already an acceleration.
    // --------------------------------------------------------

    Vec3f acceleration =
        body.Forces.Force *
        body.Mass.InverseMass;

    if (body.EnableGravity)
    {
        acceleration +=
            gravity *
            body.GravityScale;
    }

    // Semi-implicit Euler updates velocity first.
    body.State.LinearVelocity +=
        acceleration * dt;

    // --------------------------------------------------------
    // ROTATIONAL NEWTON-EULER EQUATION
    //
    // τ = dL/dt
    //
    // L = Iω
    //
    // In world coordinates:
    //
    // τ = I α + ω × (Iω)
    //
    // Therefore:
    //
    // α = I^-1 [τ - ω × (Iω)]
    // --------------------------------------------------------

    const Mat3f worldInertia =
        WorldInertia(body);

    const Mat3f worldInverseInertia =
        WorldInverseInertia(body);

    const Vec3f angularMomentum =
        worldInertia *
        body.State.AngularVelocity;

    const Vec3f gyroscopicTerm =
        Cross(
            body.State.AngularVelocity,
            angularMomentum);

    const Vec3f angularAcceleration =
        worldInverseInertia *
        (
            body.Forces.Torque -
            gyroscopicTerm
        );

    body.State.AngularVelocity +=
        angularAcceleration * dt;

    // --------------------------------------------------------
    // ENGINE DAMPING
    //
    // dv/dt = -lambda v
    //
    // exact finite update:
    //
    // v(t+h) = v(t) e^(-lambda h)
    // --------------------------------------------------------

    if (body.LinearDamping > 0.0f)
    {
        body.State.LinearVelocity *=
            std::exp(
                -body.LinearDamping *
                dt);
    }

    if (body.AngularDamping > 0.0f)
    {
        body.State.AngularVelocity *=
            std::exp(
                -body.AngularDamping *
                dt);
    }

    // Numerical safety policies.
    ClampMagnitude(
        body.State.LinearVelocity,
        body.MaxLinearSpeed);

    ClampMagnitude(
        body.State.AngularVelocity,
        body.MaxAngularSpeed);
}
```

Kairo currently simplifies the angular acceleration to:

```cpp
angularVelocity +=
    worldInverseInertia *
    torque *
    dt;
```

which omits:

$$
\omega\times(I\omega).
$$

For gameplay, that can be intentional.

For a mathematically complete standalone rigid-body integrator, we include it.

---

### Pose integration

Now velocity has already been updated.

That gives us semi-implicit Euler.

```cpp
void IntegratePose(
    RigidBody& body,
    float dt)
{
    if (!body.Dynamic)
        return;

    // --------------------------------------------------------
    // Translation
    //
    // dx/dt = v
    //
    // Semi-implicit Euler uses the newly updated velocity.
    // --------------------------------------------------------

    body.State.Position +=
        body.State.LinearVelocity *
        dt;

    // --------------------------------------------------------
    // Orientation
    //
    // q(t+h) =
    // exp(0.5 * Omega * h) q(t)
    //
    // for constant world-space angular velocity.
    // --------------------------------------------------------

    body.State.Rotation =
        IntegrateOrientation(
            body.State.Rotation,
            body.State.AngularVelocity,
            dt);
}
```

---

### Force clearing

```cpp
void ClearForces(
    RigidBody& body)
{
    body.Forces.Force =
        Vec3f::Zero();

    body.Forces.Torque =
        Vec3f::Zero();
}
```

This occurs after the full simulation step has consumed the inputs.

---

### Complete standalone step

```cpp
void StepRigidBody(
    RigidBody& body,
    const Vec3f& gravity,
    float dt)
{
    if (!std::isfinite(dt) ||
        dt <= 0.0f)
    {
        throw std::invalid_argument(
            "dt must be finite and positive.");
    }

    // 1. Continuous forces modify momentum/velocity.
    IntegrateForces(
        body,
        gravity,
        dt);

    // 2. In a full physics engine, CONTACT / JOINT IMPULSES
    //    belong here.
    //
    //    SolveConstraints(body, ...);
    //
    // The solver changes v and omega immediately.
    //
    // Pose integration therefore uses post-solver velocities.

    // 3. Advance center of mass and orientation.
    IntegratePose(
        body,
        dt);

    // 4. Continuous inputs have now been consumed.
    ClearForces(body);
}
```

That four-stage sequence is the core of the entire phase:

$$
\boxed{
\text{continuous forces}
\rightarrow
\text{velocities}
\rightarrow
\text{constraint impulses}
\rightarrow
\text{pose}
\rightarrow
\text{clear forces}.
}
$$

Kairo's real `PhysicsWorld` expands the middle enormously with broadphase, narrowphase, contact solver, CCD, joints, sleep and events, but the fundamental dynamic update is exactly this structure.

---

### Proof of every state update

We can now mathematically defend each line.

For linear velocity:

$$
F=m\frac{dv}{dt}
$$

so:

$$
\frac{dv}{dt}=m^{-1}F.
$$

Discretize:

$$
\boxed{
v_{n+1}
=
v_n+
m^{-1}F_nh.
}
$$

Add gravity acceleration:

$$
\boxed{
v_{n+1}
=
v_n+
(g+m^{-1}F_n)h.
}
$$

For angular velocity:

$$
\tau
=
\frac{dL}{dt}
$$

and:

$$
L=I_w\omega.
$$

Therefore:

$$
\tau
=
I_w\dot\omega+
\omega\times(I_w\omega).
$$

Hence:

$$
\boxed{
\dot\omega
=
I_w^{-1}
[
\tau-
\omega\times(I_w\omega)
].
}
$$

Discretize:

$$
\boxed{
\omega_{n+1}
=
\omega_n+
I_w^{-1}
[
\tau-
\omega_n\times(I_w\omega_n)
]h.
}
$$

For translation:

$$
\dot x=v.
$$

Semi-implicit:

$$
\boxed{
x_{n+1}
=
x_n+
v_{n+1}h.
}
$$

For orientation:

$$
\dot q
=
\frac12\Omega q.
$$

For constant omega over the interval:

$$
\boxed{
q_{n+1}
=
e^{\frac12\Omega h}
q_n.
}
$$

For damping:

$$
\dot v=-\lambda v.
$$

Exact finite solution:

$$
\boxed{
v_{new}
=
v_{old}e^{-\lambda h}.
}
$$

And force clearing is simply enforcing the semantic contract that:

$$
F_n
$$

was the input for interval:

$$
[t_n,t_{n+1}].
$$

There is no longer any "magic" state update in the integrator.

Every line follows from an equation or an explicitly declared engine policy.

---

### A complete energy diagnostic for that body

```cpp
float TranslationalEnergy(
    const RigidBody& body)
{
    return
        0.5f *
        body.Mass.Mass *
        body.State.LinearVelocity.LengthSquared();
}

float RotationalEnergy(
    const RigidBody& body)
{
    const Mat3f I =
        WorldInertia(body);

    return
        0.5f *
        Dot(
            body.State.AngularVelocity,
            I *
            body.State.AngularVelocity);
}

float KineticEnergy(
    const RigidBody& body)
{
    return
        TranslationalEnergy(body) +
        RotationalEnergy(body);
}

Vec3f LinearMomentum(
    const RigidBody& body)
{
    return
        body.State.LinearVelocity *
        body.Mass.Mass;
}

Vec3f SpinAngularMomentum(
    const RigidBody& body)
{
    return
        WorldInertia(body) *
        body.State.AngularVelocity;
}
```

These functions are not merely debug conveniences.

They become assertions against the physical model.

---

### Standalone validation experiment 1 — free translation

Set:

$$
F=0,
\quad
g=0,
\quad
damping=0.
$$

Then:

$$
v=constant.
$$

Expected:

$$
x(t)
=
x_0+v_0t.
$$

Any velocity drift indicates a bug.

---

### Experiment 2 — constant force

Set:

$$
F=constant.
$$

Then:

$$
a=\frac Fm.
$$

Analytic:

$$
v(t)=v_0+at.
$$

$$
x(t)
=
x_0+
v_0t+
\frac12at^2.
$$

Velocity should be exact up to roundoff for constant acceleration under Euler velocity integration.

Position has first-order global error under semi-implicit Euler.

Reduce h and verify convergence.

---

### Experiment 3 — COM impulse

At:

$$
t=0,
$$

apply J at COM.

Expected immediately:

$$
v^+
=
v^-+
m^{-1}J
$$

and:

$$
\omega^+=\omega^-.
$$

No position discontinuity.

---

### Experiment 4 — torque-free sphere

A sphere has:

$$
I=cI_3.
$$

Gyroscopic term:

$$
\omega\times(I\omega)
=
c\omega\times\omega
=
0.
$$

No torque:

$$
\tau=0.
$$

Therefore:

$$
\dot\omega=0.
$$

Angular velocity must remain constant.

Excellent orientation-integration test.

---

### Experiment 5 — torque-free asymmetric body

Set:

$$
\tau=0
$$

but choose:

$$
I_1\neq I_2\neq I_3
$$

and initial omega not aligned with a principal axis.

Then full Euler dynamics should produce evolving angular velocity while conserving:

$$
L
$$

and approximately conserving:

$$
T_{rot}.
$$

Kairo's current simplified:

$$
\dot\omega=I^{-1}\tau=0
$$

would keep world omega constant instead.

This experiment directly quantifies the approximation identified in Pass 16.

For research validation, this is an important benchmark.

---

### Experiment 6 — damping half-life

Set:

$$
v_0=10.
$$

Choose:

$$
\lambda=\ln2.
$$

After exactly:

$$
1s,
$$

expect:

$$
v=5.
$$

This should remain approximately true whether simulation is:

$$
30Hz,
60Hz,
120Hz,
240Hz
$$

because exponential damping composes correctly.

Excellent regression.

---

### Experiment 7 — energy drift sweep

Run the same conservative initial condition at:

$$
30,60,120,240Hz.
$$

Record:

$$
E(t).
$$

Plot relative error.

This turns "the integrator seems okay" into quantitative evidence.

---

### Final Kairo audit for M101/M102

| Area | Current Kairo state |
|---|---|
| Force clearing after each step | Implemented |
| Force retained through sleeping decision | Correctly implemented |
| Immediate gameplay impulse | Implemented |
| Contact impulses during solver | Implemented |
| Linear speed clamp | Implemented |
| Angular speed clamp | Implemented |
| Clamp activation diagnostics | Not evident |
| Global gravity | SI standard gravity |
| Per-world gravity scale | Implemented |
| Per-body gravity scale | Implemented |
| Disable-gravity per body | Implemented |
| Exponential linear damping | Implemented |
| Exponential angular damping | Implemented |
| Linear kinetic-energy helper | Implemented |
| Rotational kinetic-energy helper | Implemented |
| Linear momentum helper | Implemented |
| Deterministic world snapshots | Implemented |
| Replay command recording | Implemented |
| Per-frame exact state hashes | Implemented |
| Stable fixed timestep | Implemented |
| Parallel-island policy | Implemented |
| SI-like unit model | Strongly established |
| Collider/AABB/contact debug visualization | Implemented |
| COM debug visualization | Missing dedicated primitive |
| Principal inertia-axis visualization | Missing dedicated primitive |
| Primitive mass tests | Implemented |
| Scale-range mass certification | Implemented |
| COM impulse certification | Implemented |
| Off-center impulse test | Implemented |
| Non-axis-aligned compound inertia inverse regression | Still needed |
| Momentum-conservation collision tests | Should be expanded |
| Energy-drift benchmark suite | Should be added |
| Full gyroscopic rigid-body dynamics | Not currently implemented |
| First-order normalized quaternion integration | Current implementation |
| Exponential-map orientation update | Recommended improvement |

---

### Exact M101 + M102 coverage

| Part | Topic | Status |
|---:|---|---|
| 1682 | Force clearing after a step | ✓ |
| 1683 | Impulse application timing | ✓ |
| 1684 | Velocity clamping and why it can be dangerous | ✓ |
| 1685 | Maximum angular speed and numerical concerns | ✓ |
| 1686 | Gravity scaling per body | ✓ |
| 1687 | Linear damping implementation | ✓ |
| 1688 | Angular damping implementation | ✓ |
| 1689 | Energy in rigid-body motion | ✓ |
| 1690 | Kinetic energy of translation | ✓ |
| 1691 | Kinetic energy of rotation | ✓ |
| 1692 | Momentum conservation checks | ✓ |
| 1693 | Energy drift in numerical integration | ✓ |
| 1694 | Determinism concerns in rigid-body simulation | ✓ |
| 1695 | Floating-point scale and unit choice | ✓ |
| 1696 | SI units in a physics engine | ✓ |
| 1697 | Debug visualization of COM and inertia | ✓ |
| 1698 | Unit tests for mass properties | ✓ |
| 1699 | Unit tests for impulse response | ✓ |
| 1700 | Standalone RigidBody integrator with mathematical proof | ✓ |


The entire rigid-body mechanics phase can now be compressed into one dependency chain.

A rigid body is a mass distribution reduced to:

$$
\boxed{
m,\ c,\ I
}
$$

with dynamic state:

$$
\boxed{
x,\ q,\ v,\ \omega.
}
$$

Continuous influences act through:

$$
\boxed{
F=\dot p
}
$$

and:

$$
\boxed{
\tau=\dot L.
}
$$

Discrete instantaneous interactions act through:

$$
\boxed{
J=\Delta p
}
$$

and:

$$
\boxed{
r\times J=\Delta L.
}
$$

Thus:

$$
\boxed{
\Delta v=m^{-1}J
}
$$

and:

$$
\boxed{
\Delta\omega
=
I_w^{-1}(r\times J).
}
$$

The full rotational dynamics are:

$$
\boxed{
\tau
=
I_w\alpha+
\omega\times(I_w\omega),
}
$$

so:

$$
\boxed{
\alpha
=
I_w^{-1}
[
\tau-\omega\times(I_w\omega)
].
}
$$

The pose then advances with:

$$
\boxed{
v_{n+1}
=
v_n+a_nh,
}
$$

$$
\boxed{
x_{n+1}
=
x_n+v_{n+1}h,
}
$$

and orientation with:

$$
\boxed{
q_{n+1}
=
\exp
\left(
\frac12\Omega h
\right)
q_n.
}
$$

The simulation becomes production-ready only when you also reason about:

$$
\boxed{
\text{force lifetime}
}
$$

$$
\boxed{
\text{impulse ordering}
}
$$

$$
\boxed{
\text{damping and clamps}
}
$$

$$
\boxed{
\text{energy/momentum diagnostics}
}
$$

$$
\boxed{
\text{floating-point scale}
}
$$

$$
\boxed{
\text{fixed timestep and determinism}.
}
$$

And the most important architectural idea from the whole phase is that physics correctness is not judged by:

> "The box falls and looks believable."

It is judged by whether you can state **which equations are being solved, which approximations are intentional, which quantities should be conserved, which quantities are deliberately dissipated, how numerical error scales with timestep, and which engine policies intentionally break ideal mechanics**.

For Kairo specifically, the physics foundation is already much stronger than a toy engine: it has proper mass/inertia structures, immediate impulses, fixed-step runtime integration, deterministic snapshot/replay hashing, stable sleeping semantics, exponential damping, CCD support, conservation-friendly equal-and-opposite solver machinery, and renderer-independent debug geometry. The remaining improvements exposed across Passes 16–18 are now sharply defined rather than vague: **fix general non-diagonal inertia inversion, formalize COM-relative runtime geometry, add child orientation to compound mass construction, optionally add full gyroscopic dynamics, move orientation integration to the exponential map, expose first-class kinematic targets, add clamp diagnostics, visualize COM/principal inertia axes, and add quantitative energy/momentum regression suites.**

That closes the original rigid-body mechanics reconstruction through Part 1700. The supplied lesson-body provenance for Parts 1701–1800 is missing, so the following contact and constraint material is an explicit reconstruction, not a claim that the original source text was preserved.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift|Previous chapter]] · [[Chapter Edition/03 - Game Engine/40 - Coupled contacts and iterative response|Next chapter]]
