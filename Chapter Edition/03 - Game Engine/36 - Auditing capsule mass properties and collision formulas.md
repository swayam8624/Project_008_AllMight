# 36 — Auditing capsule mass properties and collision formulas

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/35 - A complete rigid-body state before the formula audit|Previous chapter]] · [[Chapter Edition/03 - Game Engine/37 - Physical examples and state validation|Next chapter]]

A working formula can still fail because its units, shape assumptions, or contact conventions were copied incorrectly. This chapter audits capsule geometry and related cases to expose those assumptions.

**By the end:** Check capsule mass and inertia formulas dimensionally and geometrically before treating them as an engine implementation.

**Recall question:** Which limiting shape or numeric witness would falsify this formula, and what independent computation would confirm the correction?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 85120–88628; source-span SHA256 c919cfa2491225331b695fc68fec261840abaae8c638c5bfbfc9458580125701. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### KAIRO AUDIT 4 — CAPSULE FORMULA

Current formula is deliberately approximate.

That is fine for a game-engine V1.

What matters is that you understand the hierarchy:

$$
\text{sphere/box formulas}
$$

are exact analytic formulas under their assumptions.

$$
\text{current capsule formula}
$$

is an engineering approximation.

$$
\text{compound arbitrary shape}
$$

requires actual mass integration or carefully combined primitives.

Never flatten those into "all are exact."

---

**Dimensional sanity checking**

You should become obsessive about units.

Mass:

$$
[m]=kg.
$$

Position:

$$
[x]=m.
$$

Velocity:

$$
[v]=m/s.
$$

Acceleration:

$$
[a]=m/s^2.
$$

Force:

$$
[F]=kg\,m/s^2.
$$

Torque:

$$
[\tau]=kg\,m^2/s^2.
$$

Moment of inertia:

$$
[I]=kg\,m^2.
$$

Angular velocity:

$$
[\omega]=1/s
$$

because radians are dimensionless.

Then:

$$
I\omega
$$

has:

$$
kg\,m^2/s,
$$

which is angular momentum.

And:

$$
I^{-1}\tau
$$

has:

$$
\frac1{kg\,m^2}
\frac{kg\,m^2}{s^2}
=
1/s^2,
$$

which is angular acceleration.

If an equation fails dimensions, it is wrong.

---

**Scaling sanity checks**

Sphere:

$$
m\sim r^3.
$$

Sphere inertia:

$$
I\sim mr^2.
$$

Therefore:

$$
I\sim r^5.
$$

Double radius:

$$
m'=8m.
$$

Then:

$$
I'
=
8m\cdot4r^2
=
32I.
$$

So a geometrically doubled sphere at same density has:

$$
\boxed{
32\times
}
$$

the moment of inertia.

This is a fantastic check for a mass-properties implementation.

---

**Translation/rotation analogy — but know where it breaks**

You can remember:

$$
F\leftrightarrow\tau
$$

$$
m\leftrightarrow I
$$

$$
v\leftrightarrow\omega
$$

$$
p\leftrightarrow L
$$

$$
a\leftrightarrow\alpha.
$$

But the rotational side is harder because:

$$
I
$$

is a tensor and rotates with the body.

That is the source of almost every extra complication.

---

**A physically coherent force application path**

A robust physics body path can look like:

```cpp
struct RigidBodyState
{
    // World-space center of mass.
    Vec3f CenterOfMassPosition =
        Vec3f::Zero();

    Quaternionf Orientation =
        Quaternionf::Identity();

    Vec3f LinearVelocity =
        Vec3f::Zero();

    Vec3f AngularVelocity =
        Vec3f::Zero();
};

struct RigidBodyMass
{
    float Mass = 0.0f;
    float InverseMass = 0.0f;

    // Inertia about COM in the body's local frame.
    Matrix3f LocalInertia =
        Matrix3f::Zero();

    Matrix3f LocalInverseInertia =
        Matrix3f::Zero();
};

struct RigidBodyForces
{
    Vec3f Force =
        Vec3f::Zero();

    Vec3f Torque =
        Vec3f::Zero();
};

void AddForceAtWorldPoint(
    RigidBodyState& state,
    RigidBodyForces& forces,
    const Vec3f& force,
    const Vec3f& worldPoint)
{
    // Continuous force accelerates the center of mass.
    forces.Force += force;

    // The same force produces rotational influence according
    // to its lever arm from the center of mass.
    const Vec3f r =
        worldPoint -
        state.CenterOfMassPosition;

    forces.Torque +=
        Cross(r, force);
}
```

This representation makes the physical origin explicit and prevents the COM/body-origin ambiguity.

---

**Full force integration with gyroscopic term**

Conceptually:

```cpp
void IntegrateForces(
    RigidBodyState& state,
    const RigidBodyMass& mass,
    const RigidBodyForces& forces,
    const Vec3f& gravity,
    float dt)
{
    // Linear Newton:
    //
    // dv/dt = g + F / m
    const Vec3f linearAcceleration =
        gravity +
        forces.Force *
            mass.InverseMass;

    state.LinearVelocity +=
        linearAcceleration * dt;

    // Convert body-space inertia to world coordinates.
    const Matrix3f R =
        ToMatrix3(
            state.Orientation);

    const Matrix3f worldInertia =
        R *
        mass.LocalInertia *
        Transpose(R);

    const Matrix3f worldInverseInertia =
        R *
        mass.LocalInverseInertia *
        Transpose(R);

    // Full rotational Newton-Euler equation:
    //
    // τ = Iα + ω × (Iω)
    const Vec3f angularMomentum =
        worldInertia *
        state.AngularVelocity;

    const Vec3f gyroscopicTorque =
        Cross(
            state.AngularVelocity,
            angularMomentum);

    const Vec3f angularAcceleration =
        worldInverseInertia *
        (
            forces.Torque -
            gyroscopicTorque
        );

    state.AngularVelocity +=
        angularAcceleration * dt;
}
```

That gives you the full conceptual model.

Whether a gameplay engine chooses the full gyroscopic term is a separate engineering decision.

---

**Why many engines omit the gyroscopic term**

The term can create:

```text
precession
wobble
intermediate-axis instability
high-spin stiffness.
```

Those are physically real.

But many games prioritize:

```text
stability
predictability
designer control
solver robustness
```

over exact free rigid-body dynamics.

So simplifying:

$$
\alpha=I^{-1}\tau
$$

can be intentional.

For a research-oriented simulator, however, you need to document the approximation.

That is especially important if experimental conclusions depend on dynamic trajectories.

---

**Force versus torque accumulator ordering**

Continuous external effects:

```text
gravity
drag
thrusters
springs
motors
```

populate:

$$
F,\tau.
$$

Then velocity integration updates:

$$
v,\omega.
$$

Then constraints/collisions can update velocity through impulses.

Then positions/orientation advance.

Very roughly:

```text
accumulate continuous forces
        ↓
integrate v and ω
        ↓
collision/constraint solver
        ↓
integrate x and q
        ↓
clear forces
```

Kairo currently follows this broad structure.

Passes 17/18 will go much deeper into impulses and contact solving.

---

**A force should usually not directly alter position**

Bad conceptual API:

```cpp
AddForce(...)
{
    position += force;
}
```

Force changes momentum/velocity.

Then integration changes position.

The chain is:

$$
F
\rightarrow
a
\rightarrow
v
\rightarrow
x.
$$

Similarly:

$$
\tau
\rightarrow
\alpha
\rightarrow
\omega
\rightarrow
q.
$$

Keeping these stages separate makes simulation understandable.

---

**COM and collision geometry**

Suppose asymmetric body has local COM:

$$
c_{local}=(0.5,0,0).
$$

A collider vertex is:

$$
x_{local}=(2,0,0).
$$

If simulation `State.Position` is COM, local offset from COM is:

$$
x_{local}-c_{local}
=
(1.5,0,0).
$$

World vertex:

$$
x_w
=
COM_w+
R(1.5,0,0).
$$

This is the transformation you need for non-centered mass distributions.

If you instead transform by:

$$
COM_w+R(2,0,0),
$$

you have implicitly assumed local COM = origin.

---

**COM and authored transforms**

This often surprises engine developers.

Artists want:

```text
object origin
```

where it is convenient for editing.

Physics wants:

```text
center of mass
```

because equations are simplest there.

Those need not be identical.

So you may need:

```text
AuthoringTransform
        |
        | + LocalCOM offset
        v
PhysicsCOMTransform
```

and reverse synchronization after simulation.

This is not a physics nuisance.

It is the correct separation of authoring and dynamics coordinate frames.

---

**Center of mass and torque invariance caveat**

Torque depends on reference point.

Torque about origin O:

$$
\tau_O.
$$

Torque about COM C differs by:

$$
\tau_C
=
\tau_O-
(c-o)\times F_{net}.
$$

Therefore when writing:

$$
\tau=r\times F,
$$

you must know **about which point** torque is being computed.

Rigid-body rotational equations are cleanest about the COM.

That is why all contact-force lever arms use:

$$
r=p-c_{COM}.
$$

---

**Angular momentum also depends on reference point**

Angular momentum:

$$
L_O
=
\sum
(x_i-O)
\times
m_iv_i.
$$

Around COM, translational and rotational momentum separate nicely.

Total angular momentum about arbitrary origin O can be decomposed:

$$
\boxed{
L_O
=
(c-O)\times Mv_C
+
L_C.
}
$$

This is another reason COM is the preferred simulation reference.

---

**Principal axes are not necessarily geometric bounding-box axes**

For arbitrary mesh:

```text
local X/Y/Z
```

chosen by artist are not automatically principal inertia axes.

The true principal axes come from eigenvectors of the inertia tensor.

For a strangely shaped mesh, they may be rotated relative to model axes.

If you want a diagonal inertia representation, you may need to compute that eigensystem.

---

**Convex hull / mesh mass properties**

For arbitrary closed mesh, a production engine can compute:

$$
V
$$

volume,

$$
c
$$

center of mass,

and:

$$
I
$$

through tetrahedral decomposition/integration.

One common method decomposes the closed triangle mesh into signed tetrahedra with a common origin, accumulating volume and polynomial moments.

That would be the natural future extension beyond sphere/box/capsule primitives.

Do not approximate arbitrary mesh inertia from its AABB unless you intentionally accept a coarse approximation.

---

**Static triangle meshes and mass**

Current Kairo only allows triangle-mesh colliders on static bodies.

That is a sensible restriction.

Static body has:

$$
m^{-1}=0
$$

and:

$$
I^{-1}=0.
$$

So you do not need robust dynamic triangle-mesh mass properties or rotational response.

Dynamic concave triangle meshes are expensive and troublesome for both inertia and collision.

---

**Why inverse inertia should belong to mass properties**

Collision solving repeatedly computes terms like:

$$
I^{-1}
(r\times n).
$$

Computing matrix inverse during every contact would be wasteful.

So precompute:

$$
I_{body}^{-1}
$$

once when mass properties change.

Then transform into world each step/query:

$$
I_w^{-1}
=
RI_b^{-1}R^T.
$$

Excellent data-oriented design.

---

**When mass properties must be recomputed**

If any of these change:

```text
shape geometry
density
mass override
compound-child geometry
compound-child transform
center of mass
locked rotation axes
```

mass properties become dirty.

Then recompute:

$$
M,\quad
c,\quad
I_b,\quad
I_b^{-1}.
$$

Do not recompute every frame when geometry is unchanged.

---

**Dynamic mass changes**

Changing mass while body is moving raises semantics.

If velocity stays fixed while mass changes:

$$
p=mv
$$

changes instantly.

If you intend momentum conservation:

$$
m_{old}v_{old}
=
m_{new}v_{new}.
$$

Therefore:

$$
v_{new}
=
\frac{m_{old}}{m_{new}}
v_{old}.
$$

Most game engines simply let designer mass changes alter momentum because mass editing is a gameplay operation.

But know the physical distinction.

---

**Force accumulation versus direct gravity in Kairo**

Kairo contains both:

```cpp
GravityForce(
    mass,
    gravity);
```

and the world-level acceleration path.

That is not contradictory.

`GravityForce()` is reusable physical math.

`PhysicsWorld` chooses a more efficient integration convention:

$$
a_g=g.
$$

The important thing is not to apply both simultaneously or gravity doubles.

---

**Mass zero edge case**

`SphereMassProperties(radius, density)` permits:

$$
density=0.
$$

Then:

$$
mass=0
$$

and inverse mass:

$$
0.
$$

The returned mass properties look static.

But `MakeRigidBody()` rejects them for dynamic bodies.

That is a reasonable layered design:

```text
math library can represent zero-mass mass properties
engine body semantics decide whether dynamic zero mass is valid.
```

---

**Numerical range considerations**

Suppose:

$$
m=10^{-30}
$$

and:

$$
m^{-1}=10^{30}.
$$

A modest force can create enormous acceleration.

Likewise enormous masses produce inverse masses near zero.

A production engine should define supported physical scales.

Later M101 explicitly covers unit/scale choices.

For now remember:

$$
\boxed{
finite\ float
\neq
numerically\ reasonable\ physical\ state.
}
$$

Input validation eventually needs scale policy, not only `isfinite()`.

---

**Why Kairo certification tests are useful but incomplete**

Current certification verifies hundreds of sphere configurations remain finite and:

$$
m\,m^{-1}\approx1.
$$

Good.

It tests identity/rotated world inverse inertia.

Good.

It tests parallel-axis shift along x.

Good.

But for this M098 audit, an important missing regression test would be a **non-axis-aligned compound** that produces off-diagonal inertia.

That would immediately reveal the diagonal-inverse issue.

---

**Test we should add conceptually**

Two equal point-like parts at:

$$
(1,1,0)
$$

and:

$$
(-1,-1,0).
$$

COM:

$$
0.
$$

Combined tensor has nonzero:

$$
I_{xy}.
$$

Then verify:

$$
I\,I^{-1}
\approx
I_3.
$$

If `InverseDiagonalInertia()` is used, that test fails.

That is precisely the regression test the generic mass-property path needs.

---

**World inertia transform test**

Choose:

$$
I_b=
diag(1,2,3).
$$

Rotate 90° about z.

x and y axes swap.

Therefore world inertia becomes approximately:

$$
diag(2,1,3).
$$

Current Kairo test does the corresponding check for inverse inertia.

That is a good test because it proves tensor rotation rather than only identity orientation.

---

**Symmetry test**

For arbitrary valid inertia:

$$
I_{ij}=I_{ji}.
$$

Test:

```cpp
REQUIRE(
    Near(
        I(0,1),
        I(1,0)));

REQUIRE(
    Near(
        I(0,2),
        I(2,0)));

REQUIRE(
    Near(
        I(1,2),
        I(2,1)));
```

Mass-property operations should preserve symmetry within tolerance.

Parallel-axis formula does.

Rotation:

$$
RIR^T
$$

does too.

---

**Positive-definiteness test**

For a dynamic body, test several vectors n:

$$
n^TIn>0
$$

for nonzero n.

A stronger mathematical check is Cholesky decomposition or eigenvalues:

$$
\lambda_i>0.
$$

If one principal moment is negative:

mass properties are physically invalid.

---

**Triangle inequalities for principal inertia**

For any physical rigid body about its COM, principal moments obey relationships like:

$$
I_1\le I_2+I_3
$$

and cyclic permutations.

Reason:

$$
I_x=\int(y^2+z^2)dm
$$

while:

$$
I_y+I_z
=
\int(x^2+z^2+x^2+y^2)dm
$$

$$
=
I_x+
2\int x^2dm
\ge I_x.
$$

So:

$$
\boxed{
I_x\le I_y+I_z.
}
$$

These are useful mass-property sanity checks.

---

**Rigid body as reduced particle system**

Here is the deepest perspective.

Imagine N particles.

State would be roughly:

$$
3N
$$

positions and:

$$
3N
$$

velocities.

Rigid constraints remove almost all independent degrees of freedom.

Because:

$$
x_i
=
c+
Rr_i,
$$

all positions reduce to:

```text
one COM
one orientation.
```

And all velocities reduce to:

```text
one linear velocity
one angular velocity.
```

So the body becomes:

$$
\boxed{
(c,q,v,\omega)
}
$$

plus mass properties.

This is why rigid-body physics is computationally tractable.

---

**Everything mass distribution contributes can be compressed**

For translation, the entire distribution reduces to:

$$
M.
$$

For rotation around COM, the entire distribution reduces to:

$$
I.
$$

That is profound.

Millions of particles are replaced by:

$$
1\text{ scalar}
+
1\text{ symmetric 3×3 matrix}.
$$

A symmetric 3×3 matrix contains only six unique values.

In principal axes, only three.

That is the mathematical compression enabling a rigid-body engine.

---

**Why COM and inertia are coupled**

If you compute inertia about a point that is not COM, translational and rotational dynamics become coupled more awkwardly.

Choosing COM creates:

$$
\sum m_ir_i=0.
$$

This annihilates cross terms.

So the preferred state:

$$
\boxed{
Position = COM
}
$$

is not merely convention.

It diagonalizes the conceptual separation between translation and rotation.

---

**Newton-Euler equations**

The complete free rigid-body dynamics can now be summarized as:

$$
\boxed{
\dot x=v
}
$$

$$
\boxed{
\dot p=F
}
$$

$$
\boxed{
p=mv
}
$$

$$
\boxed{
\dot L=\tau
}
$$

$$
\boxed{
L=I_w\omega
}
$$

plus orientation evolution:

$$
\dot R=[\omega]_\times R
$$

under one common convention, or equivalent quaternion equation.

This coupled set is the true core of rigid-body simulation.

M100 will turn it into discrete integration code.

---

**Translation is unusually simple**

For constant mass:

$$
\dot v=m^{-1}F.
$$

Nothing about orientation affects translational mass.

Mass is isotropic scalar.

Rotation is more complicated:

$$
\dot\omega
=
I_w^{-1}
[
\tau-\omega\times(I_w\omega)
].
$$

And:

$$
I_w
$$

depends on orientation.

That is why rotational physics occupies so much more machinery.

---

**Do not confuse local inertia with current world inertia**

Local:

$$
I_b
$$

is fixed to body.

World:

$$
I_w=RI_bR^T.
$$

Suppose long rod rotates 90°.

Its resistance to world-x torque changes because its long axis moved.

If you use local diagonal inertia directly with world torque:

$$
\alpha=
I_b^{-1}\tau_w,
$$

you mix coordinate spaces.

Wrong.

You must either:

```text
convert torque to body space
solve
convert alpha back
```

or use world inverse:

$$
I_w^{-1}.
$$

Kairo uses world inverse correctly.

---

**Equivalent body-space solve**

World torque:

$$
\tau_w.
$$

Convert:

$$
\tau_b=R^T\tau_w.
$$

Then:

$$
\alpha_b=I_b^{-1}\tau_b.
$$

Back:

$$
\alpha_w=R\alpha_b.
$$

So:

$$
\alpha_w
=
RI_b^{-1}R^T\tau_w.
$$

Therefore:

$$
I_w^{-1}
=
RI_b^{-1}R^T.
$$

Same formula derived from another angle.

You should be able to derive it both ways.

---

**Why no scale matrix appears**

Rigid bodies assume fixed geometry.

Orientation transformation:

$$
R
$$

is orthogonal.

A non-uniform scale:

$$
S
$$

changes mass distribution.

You cannot simply use:

$$
RS
$$

inside inertia transform and expect correct physics.

If object scale changes, recompute mass/inertia for scaled geometry.

Rigid-body transform should conceptually contain:

```text
translation + rotation
```

while scale belongs to shape construction/mass recomputation.

---

**Coordinate handedness**

Cross products depend on handedness conventions.

Kairo's tests establish its practical convention.

For:

$$
r=(1,0,0)
$$

and:

$$
F=(0,10,0),
$$

it expects:

$$
\tau=(0,0,10).
$$

That is conventional right-handed cross-product behavior:

$$
x\times y=z.
$$

When integrating with rendering or imported assets, preserve coordinate-system conversion consistency.

---

**Torque is an axial vector**

Under ordinary rotations, torque behaves like a vector.

Under mirror/reflection transformation, it behaves differently from a polar vector.

So strictly:

$$
\tau
$$

and:

$$
\omega
$$

are axial/pseudovectors.

This becomes relevant in coordinate-system handedness conversions.

If you mirror one axis while importing geometry, blindly treating angular quantities as ordinary positions can introduce sign errors.

---

**Center-of-mass transform for scene synchronization**

Suppose editor stores object-origin world transform:

$$
x_o,R.
$$

Local COM:

$$
c_l.
$$

Physics COM:

$$
c_w
=
x_o+
Rc_l.
$$

So converting scene→physics:

$$
\boxed{
x_{physics}
=
x_{scene}
+
R\,c_l.
}
$$

Converting physics→scene:

$$
\boxed{
x_{scene}
=
x_{physics}
-
R\,c_l.
}
$$

This will matter when M100 reaches scene/kinematic synchronization.

---

**A complete mass-property builder for compounds**

Conceptually:

```cpp
struct CompoundMassPart
{
    float Mass = 0.0f;

    // Position of this child's COM in body-local coordinates.
    Vec3f Center = Vec3f::Zero();

    // Child's orientation relative to body local frame.
    Quaternionf Rotation =
        Quaternionf::Identity();

    // Tensor about the child's own COM,
    // expressed in child's principal/local axes.
    Matrix3f Inertia =
        Matrix3f::Zero();
};

MassProperties BuildCompound(
    std::span<const CompoundMassPart> parts)
{
    float totalMass = 0.0f;
    Vec3f weightedCenter =
        Vec3f::Zero();

    for (const auto& part : parts)
    {
        totalMass += part.Mass;

        weightedCenter +=
            part.Center *
            part.Mass;
    }

    if (totalMass <= 0.0f)
    {
        return StaticMassProperties();
    }

    const Vec3f center =
        weightedCenter /
        totalMass;

    Matrix3f totalInertia =
        Matrix3f::Zero();

    for (const auto& part : parts)
    {
        // First orient each child's tensor into the common
        // aggregate-body coordinate frame.
        const Matrix3f R =
            ToMatrix3(
                part.Rotation);

        const Matrix3f oriented =
            R *
            part.Inertia *
            Transpose(R);

        // Then shift from the child's COM to aggregate COM.
        const Vec3f d =
            part.Center -
            center;

        totalInertia +=
            ParallelAxisTheorem(
                oriented,
                part.Mass,
                d);
    }

    return MakeMassPropertiesWithFullInverse(
        totalMass,
        center,
        totalInertia);
}
```

That is the general architecture Kairo's current simpler implementation is moving toward.

---

**Cache principal representation if compounds are static**

Compound mass-property calculation is authoring/build-time or body-construction work.

Do not perform:

```text
eigendecomposition
full mass integration
compound composition
```

per physics frame.

Once body geometry is stable:

```text
compute once
store local COM
store local inertia
store inverse.
```

Per frame only rotate inverse tensor.

---

**Engine debugging view you will eventually want**

For each body visualize:

```text
COM point
principal axes
principal inertia magnitudes
force vector
torque vector
linear velocity
angular velocity.
```

A principal-axis visualization can draw three colored axes scaled approximately by inverse rotational resistance.

This is scheduled formally in M102, but understanding why it helps belongs here.

If COM is wrong, torque behavior becomes immediately obvious.

If principal axes look nonsensical, mass properties are wrong.

---

**Physics research sanity checks**

For research-quality simulation, build tests around conservation quantities.

No external force:

$$
P_{total}
=
\sum p_i
$$

should remain approximately constant.

No external torque:

$$
L_{total}
$$

should remain approximately conserved.

No damping and perfectly conservative forces:

$$
E
$$

should remain approximately conserved, subject to integration error.

These invariants reveal mistakes that "it looks okay" cannot.

---

**Simple gravity sanity check**

Start:

$$
v_0=0.
$$

Constant gravity:

$$
g.
$$

Analytic:

$$
v(t)=gt.
$$

Position:

$$
x(t)
=
x_0+
\frac12gt^2.
$$

Your numerical engine should converge toward this as:

$$
dt\rightarrow0.
$$

This will become crucial when comparing explicit vs semi-implicit integration.

---

**Spring analytic sanity check**

Undamped one-dimensional spring:

$$
m\ddot x+kx=0.
$$

Natural frequency:

$$
\omega_n=
\sqrt{\frac{k}{m}}.
$$

Solution:

$$
x(t)
=
A\cos(\omega_nt)
+
B\sin(\omega_nt).
$$

Numerical integrator should reproduce this approximately.

If amplitude explodes uncontrollably under apparently reasonable dt, integrator/stiffness combination is unstable.

---

**Torque sanity check**

A body with scalar principal inertia:

$$
I
$$

under constant torque:

$$
\tau
$$

starting from rest gives:

$$
\alpha=
\frac{\tau}{I}.
$$

So:

$$
\omega(t)
=
\frac{\tau}{I}t.
$$

This gives you a trivial angular regression test.

---

**Parallel-axis sanity check**

Move an entire body farther from a rotational axis.

Moment must increase.

Since:

$$
I'
=
I+
m(d^2I-dd^T),
$$

for any unit rotation axis n:

$$
I'_n
=
n^TI'n.
$$

The added scalar moment becomes:

$$
m(
d^2-
(n\cdot d)^2
).
$$

But:

$$
d^2-(n\cdot d)^2
=
d_\perp^2.
$$

Therefore:

$$
\boxed{
I'_n
=
I_n+
md_\perp^2.
}
$$

Exactly the scalar parallel-axis theorem.

The matrix equation correctly generalizes it.

---

**Principal-axis sanity check**

If tensor is diagonal:

$$
I=
diag(1,2,3)
$$

then unit x torque gives simplified:

$$
\alpha=(\tau_x,0,0).
$$

But rotate body 45° relative world.

World tensor develops off-diagonals.

A world-axis torque can now create angular acceleration with multiple world components.

That is not a bug.

That is the tensor doing its job.

---

**Why inertia tensor is stored about COM, not object origin**

If you store inertia about arbitrary authoring origin, then moving the authoring pivot changes physics inertia despite geometry/mass being physically unchanged.

That is undesirable.

Store canonical:

$$
I_{COM}.
$$

Authoring pivot remains metadata/transforming convenience.

Physics should not depend on where Maya/Blender pivot happens to be.

---

**Rigid body versus collider**

Another architecture distinction:

```text
RigidBody
```

owns:

```text
mass
COM
motion
forces
```

Collider owns:

```text
shape
local transform
material
filtering.
```

One rigid body may have multiple colliders.

Mass properties can be derived from them, but they are not conceptually the same object.

That distinction becomes crucial for compound bodies.

---

**Multiple colliders and one COM**

Suppose rigid body has:

```text
box collider on left
sphere collider on right.
```

Their combined geometry determines:

$$
M
$$

and:

$$
COM.
$$

But solver still has:

```text
one rigid body
one v
one ω
one q.
```

All collider points move according to that body's rigid transform.

That is the exact meaning of "compound rigid body."

---

**What current Kairo already does particularly well**

The physics math layer has several strong architectural properties.

`Force.cppm`, `MassProperties.cppm`, `InertiaTensor.cppm`, and `RigidBodyMath.cppm` are largely **stateless mathematical utilities**.

They do not own:

```text
world
broadphase
colliders
renderer
scene.
```

That is good module layering.

The engine layer then owns:

```text
RigidBody
PhysicsWorld
sleeping
collision modes
integration
solver.
```

This is exactly the mechanism/policy separation from Phase 16 appearing inside physics.

---

**Exact M097/M098 Kairo gap map**

| Area | Current Kairo |
|---|---|
| Particle→rigid-body state model | Correct conceptual representation |
| Position/orientation/linear/angular velocity | Implemented |
| Force + torque accumulation | Implemented |
| Gravity | Implemented |
| Linear drag helper | Implemented |
| Hooke spring helper | Implemented |
| Exponential body damping | Implemented |
| Primitive sphere mass/inertia | Implemented, exact |
| Primitive box mass/inertia | Implemented, exact |
| Capsule mass | Exact volume |
| Capsule inertia | Explicit approximation |
| Parallel-axis theorem | Implemented |
| Composite COM | Implemented |
| Composite shifted inertia | Implemented for same-frame child tensors |
| Child relative inertia orientation | Not represented by current combine API |
| Local inverse inertia | Implemented |
| Full non-diagonal inverse | Missing in generic builder |
| World inverse inertia $RI^{-1}R^T$ | Implemented correctly |
| Nonzero COM offset through runtime transforms | Not fully propagated |
| Full gyroscopic angular equation | Simplified/omitted |
| Finite-value validation | Strong |
| Non-axis-aligned compound regression | Missing from inspected tests |

Those are the important findings to carry forward.

---

**Whiteboard derivation you should be able to reproduce**

Start with point masses:

$$
x_i
=
c+Rr_i.
$$

Differentiate:

$$
v_i
=
v_c+
\omega\times(Rr_i).
$$

Total momentum:

$$
P
=
\sum_i m_iv_i.
$$

Substitute:

$$
P
=
\sum_i m_iv_c
+
\sum_i
m_i
[
\omega\times(Rr_i)
].
$$

First:

$$
\sum_i m_iv_c
=
Mv_c.
$$

Second:

$$
\omega\times
\sum_i m_iRr_i.
$$

But relative coordinates about COM satisfy:

$$
\sum_i m_ir_i=0.
$$

Therefore:

$$
\boxed{
P=Mv_c.
}
$$

That is why COM translation separates cleanly.

---

**Angular-momentum derivation**

Angular momentum around COM:

$$
L
=
\sum_i
r_i^{world}
\times
m_i
(
\omega\times r_i^{world}
).
$$

Using vector triple product:

$$
r\times(\omega\times r)
=
(r\cdot r)\omega
-
(r\cdot\omega)r.
$$

Therefore:

$$
L
=
\sum_i
m_i
[
r_i^2I-r_ir_i^T
]
\omega.
$$

Factor ω:

$$
L=
I\omega.
$$

And inertia tensor emerges naturally:

$$
\boxed{
I=
\sum_i
m_i
(
r_i^2I-r_ir_i^T
).
}
$$

So the inertia tensor is not arbitrary mathematical machinery.

It falls directly out of angular momentum of a rigid collection of point masses.

---

**That is the conceptual bridge of this whole pass**

From rigid constraints:

$$
x_i=c+Rr_i
$$

we derived:

$$
P=Mv
$$

and:

$$
L=I\omega.
$$

Therefore millions of particle dynamics collapse into:

$$
\boxed{
F=\dot P
}
$$

and:

$$
\boxed{
\tau=\dot L.
}
$$

That is Newton-Euler rigid-body mechanics.

Everything else in Phase 17 is numerical implementation of those equations.

---

**Reconstruction challenge — M097**

Given a 2 kg body with:

$$
F=(6,0,0)N
$$

and:

$$
dt=0.5s,
$$

inverse mass:

$$
0.5.
$$

Acceleration:

$$
a=
F/m
=
(3,0,0)m/s^2.
$$

Velocity change:

$$
\Delta v
=
a\,dt
=
(1.5,0,0)m/s.
$$

You should be able to derive this immediately from:

$$
v_{new}
=
v+
F\,m^{-1}dt.
$$

---

**Reconstruction challenge — gravity**

Body A:

$$
m=1kg.
$$

Body B:

$$
m=100kg.
$$

Gravity forces:

$$
F_A=1g
$$

and:

$$
F_B=100g.
$$

Accelerations:

$$
a_A=
F_A/1=g
$$

and:

$$
a_B=
F_B/100=g.
$$

Therefore ideal gravity acceleration is mass-independent.

---

**Reconstruction challenge — torque**

COM:

$$
(0,0,0).
$$

Point:

$$
(2,0,0).
$$

Force:

$$
(0,3,0).
$$

Then:

$$
r=(2,0,0).
$$

Torque:

$$
\tau
=
r\times F
=
(0,0,6).
$$

Units:

$$
N\,m.
$$

---

**Reconstruction challenge — box inertia**

Box half-extents:

$$
(1,2,3)
$$

mass:

$$
24.
$$

Then:

$$
I_{xx}
=
\frac{24}{3}
(2^2+3^2)
$$

$$
=
8(4+9)
$$

$$
=
104.
$$

Exactly the value Kairo's current test checks.

---

**Reconstruction challenge — world tensor**

Local inverse:

$$
I_b^{-1}
=
diag(1,2,3).
$$

Rotate body 90° around z.

x and y principal axes swap.

Therefore:

$$
I_w^{-1}
=
diag(2,1,3).
$$

This is precisely what:

$$
RI_b^{-1}R^T
$$

should produce.

---

**Reconstruction challenge — parallel axis**

Point-like mass:

$$
m=2.
$$

Offset:

$$
d=(1,0,0).
$$

Its local inertia at its own position is zero for the conceptual point.

Shift:

$$
I
=
m(d^2I-dd^T).
$$

Here:

$$
d^2=1.
$$

$$
dd^T
=
\begin{bmatrix}
1&0&0\\
0&0&0\\
0&0&0
\end{bmatrix}.
$$

Therefore:

$$
I
=
2
\begin{bmatrix}
0&0&0\\
0&1&0\\
0&0&1
\end{bmatrix}.
$$

So:

$$
I_x=0,
$$

because point lies exactly on x-axis.

But:

$$
I_y=I_z=2.
$$

Current Kairo test verifies this exact case.

---

**Reconstruction challenge — why tensor is symmetric**

Write:

$$
rr^T.
$$

Transpose:

$$
(rr^T)^T
=
rr^T.
$$

Identity is symmetric.

Therefore:

$$
r^2I-rr^T
$$

is symmetric.

Integrate:

$$
I
$$

remains symmetric.

No memorization required.

---

**Reconstruction challenge — why principal axes exist**

Physical inertia tensor is real symmetric.

Spectral theorem:

$$
I=QDQ^T.
$$

Columns of Q:

principal axes.

Diagonal entries D:

principal moments.

This means there is always an orthonormal local basis where inertia becomes diagonal.

That is the deep reason diagonal local inertia is not merely a convenient special case—it is always possible if you choose principal coordinates.

---

**But principal axes and visual body frame can differ**

If Kairo eventually diagonalizes arbitrary compound inertia, it must remember orientation:

$$
Q.
$$

Otherwise you cannot transform from body visual/local coordinates into principal inertia coordinates.

So simply replacing a full tensor with:

```text
three diagonal numbers
```

without retaining Q loses information.

This is another reason the data model must be designed carefully.

---

**Exact coverage — M097 + M098**

| Part | Topic | Status |
|---:|---|---|
| 1601 | Particles versus rigid bodies | ✓ |
| 1602 | Fixed internal-distance rigid-body assumption | ✓ |
| 1603 | Translational rigid-body state | ✓ |
| 1604 | Rotational rigid-body state | ✓ |
| 1605 | Center of mass | ✓ |
| 1606 | COM from point masses | ✓ |
| 1607 | COM from uniform shapes | ✓ |
| 1608 | Mass versus inverse mass | ✓ |
| 1609 | Infinite mass through inverse mass zero | ✓ |
| 1610 | Density and mass from volume | ✓ |
| 1611 | Linear momentum | ✓ |
| 1612 | Newton's first law | ✓ |
| 1613 | Newton's second law in force form | ✓ |
| 1614 | Newton's second law in momentum form | ✓ |
| 1615 | Newton's third law | ✓ |
| 1616 | Force accumulation | ✓ |
| 1617 | Gravity as force/acceleration | ✓ |
| 1618 | Drag-force concepts | ✓ |
| 1619 | Springs and Hooke's law | ✓ |
| 1620 | Damping forces | ✓ |
| 1621 | Torque from off-center force | ✓ |
| 1622 | Lever-arm vectors | ✓ |
| 1623 | Angular velocity | ✓ |
| 1624 | Angular acceleration | ✓ |
| 1625 | Angular momentum | ✓ |
| 1626 | Moment of inertia as rotational mass | ✓ |
| 1627 | Inertia-tensor motivation | ✓ |
| 1628 | Inertia-tensor matrix representation | ✓ |
| 1629 | Principal moments | ✓ |
| 1630 | Principal axes | ✓ |
| 1631 | Point-mass inertia tensor | ✓ |
| 1632 | Solid-sphere inertia | ✓ |
| 1633 | Solid-box inertia | ✓ |
| 1634 | Solid-cylinder inertia concept | ✓ |
| 1635 | Capsule inertia concept | ✓ |
| 1636 | Parallel-axis theorem | ✓ |
| 1637 | Composite mass properties | ✓ |
| 1638 | Local-space inertia | ✓ |
| 1639 | Inverse inertia tensor | ✓ |
| 1640 | Rotating inertia into world space | ✓ |
| 1641 | $RI^{-1}R^T$ world inverse formula | ✓ |
| 1642 | Why inertia tensors are symmetric | ✓ |


The equation chain you should now be able to derive from nothing is:

$$
\boxed{
x_i=c+Rr_i
}
$$

which gives:

$$
\boxed{
v_i=v_c+\omega\times r_i
}
$$

then:

$$
\boxed{
P=Mv_c
}
$$

and:

$$
\boxed{
L=I\omega
}
$$

therefore:

$$
\boxed{
F=\dot P
}
$$

and:

$$
\boxed{
\tau=\dot L.
}
$$

The mass distribution itself compresses into:

$$
\boxed{
M,\quad c,\quad I.
}
$$

The runtime solver primarily wants:

$$
\boxed{
M^{-1},\quad I^{-1}.
}
$$

And because the body's inertia rotates with orientation:

$$
\boxed{
I_w^{-1}
=
R I_b^{-1} R^T.
}
$$

The three most important Kairo-specific findings from this pass are also now clear. First, **`State.Position` should have an explicit COM convention**, because the existing `LocalCenterOfMass` field is not yet fully propagated when it becomes nonzero. Second, **generic compound inertia must not use `InverseDiagonalInertia()` after parallel-axis operations have produced off-diagonal terms** unless the tensor has first been diagonalized into principal coordinates. Third, Kairo's current angular force integration uses the common gameplay approximation:

$$
\dot\omega=I^{-1}\tau
$$

rather than the full Euler rigid-body equation:

$$
\boxed{
\dot\omega
=
I^{-1}
[
\tau-\omega\times(I\omega)
].
}
$$

For ordinary game physics this can be deliberate. For physically controlled research replication, that approximation must be measured and documented.

**Pass 17 is M099 + M100, Parts 1643–1681.** That is where all of today's state becomes an actual simulator: **linear and angular impulses, off-center impulse response, force-versus-impulse distinctions, rigid-body memory state, sleeping/activity, static/kinematic/dynamic semantics, explicit and semi-implicit Euler, quaternion derivative and exponential-map orientation updates, renormalization, fixed-step accumulators, multiple substeps, spiral-of-death handling, snapshots, and scene↔physics synchronization.**


### Impulses, Off-Center Response, Rigid-Body Runtime State, Sleeping, Static/Kinematic/Dynamic Semantics, Euler Integration, Quaternion Dynamics, Exponential Maps, Fixed Timestep, Substeps, Spiral-of-Death Control, Physics Snapshots, Render Interpolation, and Scene Synchronization


Pass 16 gave us the continuous rigid-body equations:

$$
\mathbf F=\frac{d\mathbf p}{dt},
\qquad
\boldsymbol\tau=\frac{d\mathbf L}{dt},
$$

with

$$
\mathbf p=m\mathbf v,
\qquad
\mathbf L=I\boldsymbol\omega.
$$

Pass 17 now answers two engine questions:

$$
\boxed{\text{How does a finite event change velocity instantly?}}
$$

and:

$$
\boxed{\text{How do we turn continuous differential equations into discrete frames without destroying stability?}}
$$

The first question gives us **impulses**.

The second gives us **integration and fixed timesteps**.

---

### IMPULSE RESPONSE AND THE ACTUAL RIGID-BODY OBJECT

### Part 1643 — Linear impulse

Force is an instantaneous rate of momentum change:

$$
\mathbf F=\frac{d\mathbf p}{dt}.
$$

Integrate both sides over a finite interval:

$$
\int_{t_0}^{t_1}\mathbf F\,dt
=
\int_{t_0}^{t_1}\frac{d\mathbf p}{dt}dt.
$$

Therefore:

$$
\boxed{
\mathbf J
=
\Delta\mathbf p
}
$$

where:

$$
\boxed{
\mathbf J
=
\int\mathbf F\,dt
}
$$

is **impulse**.

Units:

$$
[\mathbf J]
=
N\,s
=
kg\,m/s.
$$

That is exactly the same unit as momentum.

This is not a coincidence. Impulse literally *is momentum transfer*.

If a constant force $F$ acts for $\Delta t$:

$$
\mathbf J
=
\mathbf F\Delta t.
$$

A collision, explosion, bullet impact, solver contact correction, or jump impulse is usually better represented through $\mathbf J$ than through a huge force applied for an arbitrarily chosen frame.

---

### Part 1644 — Linear velocity change from impulse

For constant body mass:

$$
\mathbf p=m\mathbf v.
$$

Apply impulse:

$$
\mathbf p'
=
\mathbf p+\mathbf J.
$$

Then:

$$
m\mathbf v'
=
m\mathbf v+\mathbf J.
$$

Divide by mass:

$$
\boxed{
\mathbf v'
=
\mathbf v+
\frac{\mathbf J}{m}.
}
$$

Using inverse mass:

$$
w=m^{-1},
$$

we obtain:

$$
\boxed{
\Delta\mathbf v
=
w\mathbf J.
}
$$

Kairo already implements precisely this operation:

```cpp
inline void ApplyLinearImpulse(
    Vec3f& linearVelocity,
    float inverseMass,
    const Vec3f& impulse)
{
    // J changes momentum instantly.
    // Since p = m v:
    //
    // Δv = J / m = J * inverseMass.
    linearVelocity +=
        impulse * inverseMass;
}
```

Notice the absence of:

```cpp
dt
```

That is the critical distinction.

A force must be integrated through time.

An impulse has already been integrated over time.

Therefore:

$$
\boxed{
\text{Impulse application must not multiply by }dt.
}
$$

---

### Static-body consequence

For a static body:

$$
m^{-1}=0.
$$

Therefore:

$$
\Delta v
=
0\mathbf J
=
0.
$$

This gives us immovable bodies almost automatically.

This same "inverse quantity zero means infinite resistance" pattern applies to inverse inertia.

---

### Part 1645 — Angular impulse

The rotational equivalent follows the same logic.

Torque satisfies:

$$
\boldsymbol\tau
=
\frac{d\mathbf L}{dt}.
$$

Integrate over time:

$$
\int\boldsymbol\tau dt
=
\Delta\mathbf L.
$$

Define angular impulse:

$$
\boxed{
\mathbf J_\omega
=
\int\boldsymbol\tau dt.
}
$$

Then:

$$
\boxed{
\mathbf L'
=
\mathbf L+
\mathbf J_\omega.
}
$$

Using:

$$
\mathbf L=I\boldsymbol\omega,
$$

we obtain:

$$
I\boldsymbol\omega'
=
I\boldsymbol\omega+
\mathbf J_\omega.
$$

Therefore:

$$
\boxed{
\boldsymbol\omega'
=
\boldsymbol\omega+
I^{-1}\mathbf J_\omega.
}
$$

Kairo's direct angular-impulse helper mirrors this exactly:

```cpp
inline void ApplyAngularImpulse(
    Vec3f& angularVelocity,
    const Matrix3f& worldInverseInertia,
    const Vec3f& angularImpulse)
{
    angularVelocity +=
        worldInverseInertia *
        angularImpulse;
}
```

Again:

$$
dt
$$

does not appear.

---

### Part 1646 — Angular velocity change from an off-center impulse

Suppose a linear impulse:

$$
\mathbf J
$$

is applied at world point:

$$
\mathbf p.
$$

Let body center of mass be:

$$
\mathbf c.
$$

Lever arm:

$$
\mathbf r
=
\mathbf p-\mathbf c.
$$

The corresponding angular impulse is:

$$
\boxed{
\mathbf J_\omega
=
\mathbf r\times\mathbf J.
}
$$

Therefore:

$$
\boxed{
\Delta\boldsymbol\omega
=
I_w^{-1}
(
\mathbf r\times\mathbf J
).
}
$$

At the same time the body's COM receives linear velocity change:

$$
\boxed{
\Delta\mathbf v
=
m^{-1}\mathbf J.
}
$$

So one off-center collision impulse modifies **both** translation and rotation.

That is the fundamental contact-response operation.

---

### Example

Let:

$$
m=2kg
$$

so:

$$
m^{-1}=0.5.
$$

Impulse:

$$
\mathbf J=(0,4,0).
$$

Applied one metre right of COM:

$$
\mathbf r=(1,0,0).
$$

Linear change:

$$
\Delta v
=
0.5(0,4,0)
=
(0,2,0).
$$

Angular impulse:

$$
r\times J
=
(1,0,0)\times(0,4,0)
=
(0,0,4).
$$

If:

$$
I_w^{-1}
=
\begin{bmatrix}
1&0&0\\
0&1&0\\
0&0&0.5
\end{bmatrix},
$$

then:

$$
\Delta\omega
=
(0,0,2).
$$

One event produced both:

$$
\Delta v=(0,2,0)
$$

and:

$$
\Delta\omega=(0,0,2).
$$

---

### Part 1647 — Applying an impulse at a world-space point

The complete algorithm is therefore:

```cpp
void ApplyImpulseAtWorldPoint(
    MotionState& state,
    float inverseMass,
    const Matrix3f& worldInverseInertia,
    const Vec3f& impulse,
    const Vec3f& worldPoint)
{
    // 1. Linear response acts at the body's center of mass.
    state.LinearVelocity +=
        impulse * inverseMass;

    // 2. Measure the application point relative to COM.
    const Vec3f r =
        worldPoint -
        state.Position;

    // 3. Linear impulse creates angular impulse r × J.
    const Vec3f angularImpulse =
        Cross(r, impulse);

    // 4. Convert angular momentum change into angular velocity.
    state.AngularVelocity +=
        worldInverseInertia *
        angularImpulse;
}
```

This is essentially what Kairo's `ApplyImpulseAtPoint()` already performs.

The important assumption, as established in Pass 16, is:

$$
\boxed{
State.Position
=
WorldCenterOfMass.
}
$$

Otherwise:

```cpp
worldPoint - state.Position
```

is not the correct torque arm.

---

### Part 1648 — Applying an impulse at center of mass

If:

$$
\mathbf p=\mathbf c,
$$

then:

$$
\mathbf r=0.
$$

So:

$$
\mathbf r\times\mathbf J=0.
$$

Therefore:

$$
\Delta\omega=0.
$$

Only translation changes:

$$
\boxed{
\Delta v=m^{-1}J.
}
$$

Kairo's `RuntimePhysicsBridge::ApplyEntityImpulse()` deliberately gets:

```cpp
const auto point =
    body.State.Position;
```

then calls:

```cpp
ApplyBodyImpulseAtPoint(
    body,
    impulse,
    point);
```

so the player-facing entity impulse is explicitly a **center-of-mass impulse**.

That is a clean semantic API.

---

### Part 1649 — Force versus impulse

This distinction must become automatic.

Force:

$$
\boxed{
\mathbf F
=
\frac{d\mathbf p}{dt}
}
$$

has units:

$$
N.
$$

Impulse:

$$
\boxed{
\mathbf J
=
\Delta\mathbf p
}
$$

has units:

$$
N\,s.
$$

For a force over one fixed simulation step:

$$
\mathbf J
\approx
\mathbf F\,dt.
$$

Then:

$$
\Delta v
=
m^{-1}\mathbf Fdt.
$$

For an impulse:

$$
\Delta v
=
m^{-1}\mathbf J.
$$

The distinction in code should therefore be:

```cpp
// Continuous influence.
AddForce(body, force);

// Instantaneous momentum change.
ApplyImpulse(body, impulse);
```

not:

```cpp
AddForce(body, giantNumber);
```

to fake a collision.

---

### Why collisions use impulses

Collision contact is conceptually very short in time.

Real material collision involves extremely large forces over tiny intervals.

A real force-time curve might look like:

$$
F(t)
$$

with peak force enormous.

But the body-level quantity we care about is:

$$
\int F(t)\,dt.
$$

That is exactly impulse.

So rigid-body collision solvers skip modelling microscopic contact deformation and solve directly for:

$$
J.
$$

---

### Force is frame-duration sensitive; impulse is not

Suppose you incorrectly implement a jump as:

```cpp
AddForce(
    body,
    Vec3f{0, 500, 0});
```

for exactly one frame.

At:

$$
dt=1/60
$$

the momentum change is:

$$
J=500/60\approx8.33.
$$

At:

$$
dt=1/120
$$

it becomes:

$$
J\approx4.17.
$$

Your jump height changes with physics frequency.

A jump intended as instantaneous velocity change should usually be impulse-based.

---

### Part 1650 — Torque impulse versus continuous torque

Continuous torque:

$$
\boldsymbol\tau
$$

changes angular momentum according to:

$$
\frac{dL}{dt}
=
\tau.
$$

Over one time step:

$$
\Delta L
\approx
\tau dt.
$$

Angular impulse:

$$
\mathbf J_\omega
$$

directly states:

$$
\Delta L
=
J_\omega.
$$

Thus:

$$
\Delta\omega
=
I^{-1}\tau dt
$$

for torque,

whereas:

$$
\Delta\omega
=
I^{-1}J_\omega
$$

for angular impulse.

Again the difference is the factor:

$$
dt.
$$

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/35 - A complete rigid-body state before the formula audit|Previous chapter]] · [[Chapter Edition/03 - Game Engine/37 - Physical examples and state validation|Next chapter]]
