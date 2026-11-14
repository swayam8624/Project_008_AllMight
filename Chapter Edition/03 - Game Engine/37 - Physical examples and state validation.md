# 37 — Physical examples and state validation

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/36 - Auditing capsule mass properties and collision formulas|Previous chapter]] · [[Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift|Next chapter]]

The derived body model needs examples that connect coefficients and memory layout to observable motion. This chapter turns representative bodies into checks for state, inertia, and update behavior.

**By the end:** Use physical examples to validate rigid-body state, inertia, and transformation synchronization against explicit invariants.

**Recall question:** What numeric result should this body produce from the stated mass, dimensions, and force, and which sign error would reverse it?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 88629–92391; source-span SHA256 a80c507d46aace47666a19d94e7d193bba95ff2087ca35225db495c3a8498ef8. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Physical examples

A continuously running reaction wheel or motor applies torque.

A hammer striking a wrench gives an angular impulse.

A thruster mounted away from COM applies force plus continuous torque.

A collision at the edge of a crate produces linear plus angular impulse.

The API should expose these concepts separately.

---

### Part 1651 — Rigid-body state structure in memory

The mathematical state becomes a concrete runtime structure.

Kairo currently stores roughly:

```cpp
struct RigidBody
{
    BodyID ID;
    bool Active;

    BodyType Type;

    MotionState State;
    MassProperties Mass;
    ForceAccumulation Forces;

    bool EnableGravity;
    float GravityScale;

    float LinearDamping;
    float AngularDamping;

    float MaxLinearSpeed;
    float MaxAngularSpeed;

    CollisionDetectionMode CollisionDetection;

    bool AllowSleeping;
    bool Sleeping;
    float SleepTimer;
};
```

And:

```cpp
struct MotionState
{
    Vec3f Position;
    Quaternionf Rotation;

    Vec3f LinearVelocity;
    Vec3f AngularVelocity;
};
```

This is an excellent place to understand the distinction between **state variables** and **parameters**.

State evolves dynamically:

$$
x,q,v,\omega.
$$

Parameters define response:

$$
m^{-1},I^{-1},damping,gravityScale.
$$

Accumulators are transient:

$$
F,\tau.
$$

Flags define simulation policy:

```text
static / dynamic / kinematic
sleeping
CCD mode
```

Those categories have different update lifetimes.

---

### State that changes every simulation step

The hot state is:

$$
x,\quad q,\quad v,\quad\omega.
$$

Potentially also:

```text
sleep timer
sleep flag
force accumulator
```

This is the data you care about when considering cache layout later.

Mass properties are usually much less frequently modified.

---

### Why storing everything in one `RigidBody` is acceptable initially

A single AoS structure is simple.

For thousands to hundreds of thousands of bodies, one could eventually separate:

```text
hot dynamic motion
cold configuration
collision policy
debug fields
```

into hot/cold arrays.

But architecture should be driven by profiling.

The current Kairo representation is readable and coherent.

---

### Part 1652 — Position representation

Position is ordinarily:

$$
\mathbf x=(x,y,z).
$$

Kairo uses:

```cpp
Vec3f
```

for physics state.

This is normal for world sizes where 32-bit float precision is sufficient.

A float has roughly 24 significant binary bits.

Relative precision is approximately:

$$
2^{-23}
\approx1.19\times10^{-7}.
$$

At distance:

$$
10^3m,
$$

resolution is around sub-millimetre-ish scale.

At:

$$
10^6m,
$$

resolution becomes roughly tenths of metres.

Thus giant open worlds often use:

```text
origin rebasing
double-precision world coordinates
cell-local float physics
```

rather than blindly making every physics calculation double.

---

### Physics position should mean COM

This deserves repetition.

For clean Newton-Euler simulation:

$$
\boxed{
Position=COM.
}
$$

Then:

$$
x_{point}
=
x_{COM}
+
Rr.
$$

If authoring scene transform represents another pivot, convert at the bridge.

That keeps solver equations correct and simple.

---

### Part 1653 — Orientation quaternion representation

A normalized quaternion:

$$
q=(\mathbf v,w)
$$

with:

$$
\|q\|=1
$$

represents orientation.

Unit quaternion advantages include compactness, no Euler gimbal lock in the representation, smooth interpolation, and efficient composition.

But remember:

$$
q
$$

and:

$$
-q
$$

represent the **same physical rotation**.

This matters in interpolation.

If:

$$
q_0\cdot q_1<0,
$$

one quaternion is normally negated before interpolation to follow the shortest rotational path.

---

### Rotation matrix relation

Quaternion corresponds to:

$$
R(q)\in SO(3).
$$

Then:

$$
R^TR=I,
$$

and:

$$
\det R=1.
$$

Those properties are why:

$$
R^{-1}=R^T
$$

and why our inertia transformations worked in Pass 16.

---

### Part 1654 — Linear velocity representation

Linear velocity:

$$
\mathbf v
$$

is usually world-space:

$$
m/s.
$$

Kairo stores it world-space.

Position integration is then trivial:

$$
x_{new}=x+v\,dt.
$$

Collision contact velocity likewise uses:

$$
v_p
=
v+
\omega\times r.
$$

No transform of v is needed if all solver equations are world-space.

That greatly reduces mistakes.

---

### Part 1655 — Angular velocity representation

Angular velocity:

$$
\boldsymbol\omega
$$

is also stored as a three-dimensional vector.

Units:

$$
rad/s.
$$

Kairo treats angular velocity as **world-space**.

We can infer this from the math:

```cpp
WorldInverseInertia(body) *
body.Forces.Torque
```

where torque and inverse inertia are world-space, producing world-space angular acceleration added directly to:

```cpp
State.AngularVelocity.
```

That convention determines quaternion derivative multiplication order later.

This is an important point.

---

### World angular velocity versus body angular velocity

If $\omega$ is world-space and quaternion rotates body→world, then commonly:

$$
\boxed{
\dot q
=
\frac12
\Omega_w q
}
$$

where:

$$
\Omega_w=(\omega_x,\omega_y,\omega_z,0).
$$

If $\omega$ were body-space, multiplication order would instead be:

$$
\dot q
=
\frac12
q\Omega_b.
$$

Kairo currently uses:

```cpp
derivative =
    omega *
    rotation *
    0.5f;
```

which is consistent with a world-space angular-velocity convention.

---

### Part 1656 — Force and torque accumulators

We already saw:

```cpp
struct ForceAccumulation
{
    Vec3f Force;
    Vec3f Torque;
};
```

The accumulator is **not persistent force history**.

It means:

> Continuous forces requested for the current physics integration interval.

After the step:

```cpp
ClearForces(body.Forces);
```

sets them back to zero.

If an engine system wants a force continuously—for example thrust—it must apply it each physics step.

This is a good design because continuous force becomes an explicit per-step input rather than a hidden permanent state.

---

### Impulses bypass accumulators

An impulse changes:

$$
v,\omega
$$

immediately.

It should not be stored in:

```text
ForceAccumulation
```

because then someone would multiply it by dt later.

That would be dimensionally wrong.

This separation is crucial.

---

### Part 1657 — Sleeping flags and activity state

A body at rest still costs CPU if we continuously:

```text
integrate gravity
detect contacts
solve resting constraints
update tiny floating-point jitter
```

Sleeping reduces this cost.

A sleeping dynamic body is conceptually:

```text
still a dynamic body
but temporarily omitted from active simulation
until an external event wakes it.
```

Kairo distinguishes:

```cpp
IsDynamicBodyType(body)
```

from:

```cpp
IsDynamic(body)
```

This is excellent.

The first means:

> Is this fundamentally a dynamic body?

The second means:

> Should it currently be dynamically integrated?

Current `IsDynamic()` requires:

```text
Active
Dynamic type
positive inverse mass
not sleeping.
```

---

### Sleeping state machine

Conceptually:

```text
AWAKE
  |
  | low velocity for sufficient time
  v
SLEEPING
  |
  | force / impulse / moving contact / explicit wake
  v
AWAKE
```

Kairo stores:

```cpp
bool Sleeping;
float SleepTimer;
bool AllowSleeping;
```

and global thresholds:

```cpp
SleepLinearSpeed
SleepAngularSpeed
SleepTime
```

in `PhysicsStepSettings`.

That is the correct basic architecture.

---

### Why sleeping needs a timer

If you sleep immediately whenever:

$$
|v|<v_{threshold},
$$

an oscillating object can repeatedly sleep and wake around the threshold.

Instead require:

$$
|v|<v_s
$$

and:

$$
|\omega|<\omega_s
$$

for:

$$
t>t_s.
$$

Only then sleep.

This introduces hysteresis through time.

---

### Sleeping body velocity

When a body transitions to sleep, engines usually snap:

$$
v=0,
\qquad
\omega=0.
$$

Otherwise tiny residual values can later reappear.

Kairo's destroyed bodies are explicitly zeroed; sleeping logic likewise needs a consistent zeroing policy.

---

### Wake conditions

A dynamic body should typically wake when it receives a meaningful external change such as force, torque, impulse, collision from an active dynamic body, changed transform, or changed constraint.

Kairo does several of these explicitly.

`AddBodyForce()` calls:

```cpp
WakeRigidBody(record);
```

before accumulating force.

`ApplyBodyImpulseAtPoint()` checks the body type, wakes it, and then applies impulse.

Contacts and joints have dedicated wake propagation.

That is exactly what you want.

---

### One Kairo nuance

`AddBodyForce()` and `AddBodyTorque()` currently wake and accumulate on any active body record, including static/kinematic records.

Those non-dynamic bodies will not integrate the force and the accumulator is later cleared.

This is harmless enough, but semantically the API could be stricter and simply no-op or reject continuous forces on non-dynamic bodies.

That would make behavior more explicit.

---

### Part 1658 — Body types: static, kinematic, dynamic

A practical rigid-body engine usually has exactly these three broad motion semantics.

| Type | Solver moves it? | User can move it? | Responds to impulses? | Has finite dynamic mass? |
|---|---:|---:|---:|---:|
| Static | no | usually not during simulation | no | no |
| Kinematic | no via solver | yes | no | effectively infinite |
| Dynamic | yes | indirectly / teleport carefully | yes | yes |

Kairo uses:

```cpp
enum class BodyType
{
    Static,
    Kinematic,
    Dynamic
};
```

which maps cleanly to this model.

---

### Part 1659 — Kinematic semantics

A kinematic body is moved by authored/scripted motion rather than force response.

It has effectively:

$$
m^{-1}=0
$$

for solver response.

But unlike static geometry, it can have:

$$
v\neq0,
\qquad
\omega\neq0.
$$

So it can push dynamic bodies through contact.

A moving platform is the classic example.

---

### Why kinematic velocity matters

Imagine an elevator moving upward at:

$$
2m/s.
$$

A character standing on it must see the contact surface velocity as:

$$
2m/s.
$$

If you simply teleport the kinematic transform every frame while storing:

$$
v=0,
$$

the contact solver has incomplete information.

For robust kinematics, either provide authored velocity explicitly or derive velocity from target pose differences:

$$
v_k
=
\frac{x_{target}-x_{current}}{dt}.
$$

For rotation, derive angular velocity from relative quaternion.

---

### Current Kairo semantics

`MakeRigidBody()` assigns:

```cpp
StaticMassProperties()
```

to both static and kinematic bodies.

Thus their solver inverse mass/inertia are zero.

`IntegrateKinematicVelocities()` then:

```cpp
ApplyVelocityControls(body, dt);
AdvanceMotionState(body.State, dt);
```

so a kinematic body advances according to its authored velocity but does not respond to forces/impulses.

That is coherent.

One design question remains: Kairo currently applies linear/angular **damping** to kinematic velocity. Since kinematic motion is typically explicitly authored, many engines would not damp it automatically. Kairo's choice is valid if damping is intentionally part of kinematic motion policy, but it should remain deliberate.

---

### Part 1660 — Dynamic-body semantics

Dynamic bodies are controlled by simulation.

For a dynamic body:

$$
m^{-1}>0.
$$

Its velocity changes from:

$$
\text{forces}
+
\text{torques}
+
\text{impulses}
+
\text{constraints}.
$$

Its position/orientation are then integrated.

Current Kairo checks during construction:

```cpp
if (
    desc.Type == BodyType::Dynamic &&
    desc.Mass.InverseMass <= 0.0f)
{
    throw ...;
}
```

Good.

"Dynamic infinite-mass body" would be contradictory.

---

### User movement of a dynamic body

There are several semantically different operations.

**Teleport** means set pose immediately, usually preserve or explicitly reset velocity, wake body, update broadphase, and reset render interpolation.

**Apply impulse** means physically alter momentum.

**Set target** usually means use a motor/controller.

**Set velocity** directly overrides dynamics.

Do not use teleport to simulate physical movement unless that is really the intended behavior.

Kairo's `SetEntityPosition()` explicitly behaves as a teleport-like operation: scene and physics pose change immediately, previous/current render pose history are collapsed, and the body is woken.

That avoids rendering a fake interpolation path from the old location.

Excellent.

---

### Part 1661 — Static-body semantics

Static body has:

$$
m^{-1}=0,
\qquad
I^{-1}=0.
$$

It participates in collision geometry but does not integrate.

Typical examples:

```text
terrain
walls
level architecture
fixed obstacles.
```

Static bodies should usually not be moved every frame.

If you need frequent movement, use kinematic.

Why?

Broadphase acceleration structures can optimize static geometry aggressively.

Changing static body poses may require expensive reinsertion/rebuilding.

Kairo also restricts triangle mesh colliders to static bodies, which is a sensible initial policy.

---

### one critical distinction

Do not think:

```text
static = velocity zero
kinematic = velocity nonzero
dynamic = velocity arbitrary.
```

That is wrong.

The actual distinction is **who owns motion**.

$$
\boxed{
Static:
no motion owner during simulation
}
$$

$$
\boxed{
Kinematic:
application owns motion
}
$$

$$
\boxed{
Dynamic:
physics owns motion
}
$$

That ownership distinction is much more useful than looking at current numerical velocity.

---

### DISCRETIZING NEWTON'S EQUATIONS

We now have state:

$$
x_n,
v_n.
$$

Acceleration:

$$
a_n.
$$

We need:

$$
x_{n+1},
v_{n+1}.
$$

Continuous equations:

$$
\dot x=v,
$$

$$
\dot v=a.
$$

A computer cannot normally solve arbitrary continuous systems symbolically every frame.

So we numerically integrate.

---

### Part 1662 — Explicit Euler

For differential equation:

$$
\dot y=f(y,t),
$$

explicit Euler approximates:

$$
\boxed{
y_{n+1}
=
y_n+
h f(y_n,t_n).
}
$$

For mechanics:

$$
x_{n+1}
=
x_n+
v_nh
$$

and:

$$
v_{n+1}
=
v_n+
a_nh.
$$

The key is:

$$
x
$$

uses **old velocity**.

Kairo's reference helper:

```cpp
state.Position +=
    state.LinearVelocity * dt;

state.Rotation =
    IntegrateAngularVelocity(
        state.Rotation,
        state.AngularVelocity,
        dt);

state.LinearVelocity +=
    linearAcceleration * dt;

state.AngularVelocity +=
    angularAcceleration * dt;
```

is explicit Euler in that sense.

---

### Explicit Euler error

Taylor expansion:

$$
x(t+h)
=
x(t)
+
h\dot x(t)
+
\frac{h^2}{2}\ddot x(t)
+
O(h^3).
$$

Euler keeps only:

$$
x+h\dot x.
$$

Local truncation error:

$$
O(h^2).
$$

Accumulated global error across fixed time:

$$
\boxed{
O(h).
}
$$

So Euler is first-order accurate.

Halving h approximately halves global integration error.

---

### Gravity example

Start:

$$
x_0=0,\quad v_0=0.
$$

Let:

$$
a=-10.
$$

Step:

$$
h=0.1.
$$

Explicit Euler gives:

$$
x_1
=
0+0(0.1)
=
0.
$$

Velocity:

$$
v_1
=
0-10(0.1)
=
-1.
$$

True analytic position after 0.1 s:

$$
x=-\frac12(10)(0.1)^2=-0.05.
$$

Euler lagged because position used old velocity.

---

### Part 1663 — Semi-implicit Euler

Semi-implicit Euler, also called symplectic Euler in this context, reverses update ordering:

$$
\boxed{
v_{n+1}
=
v_n+a_nh
}
$$

then:

$$
\boxed{
x_{n+1}
=
x_n+v_{n+1}h.
}
$$

Now gravity example gives:

$$
v_1=-1.
$$

Then:

$$
x_1=-0.1.
$$

This overshoots the analytic:

$$
-0.05.
$$

So why prefer it?

Because for many mechanical systems, especially oscillatory/conservative systems, its **long-term energy behavior** is dramatically better.

---

### Part 1664 — Why games prefer semi-implicit Euler

Consider harmonic oscillator:

$$
\ddot x
=
-\omega^2x.
$$

Explicit Euler:

$$
v_{n+1}
=
v_n-\omega^2x_nh,
$$

$$
x_{n+1}
=
x_n+v_nh.
$$

The update matrix is:

$$
\begin{bmatrix}
x_{n+1}\\v_{n+1}
\end{bmatrix}
=
\begin{bmatrix}
1&h\\
-\omega^2h&1
\end{bmatrix}
\begin{bmatrix}
x_n\\v_n
\end{bmatrix}.
$$

Eigenvalue magnitude:

$$
\sqrt{1+\omega^2h^2}>1.
$$

Therefore energy grows.

The oscillator spirals outward numerically.

Explicit Euler artificially injects energy.

---

### Semi-implicit oscillator

Semi-implicit:

$$
v_{n+1}
=
v_n-\omega^2x_nh
$$

then:

$$
x_{n+1}
=
x_n+h v_{n+1}.
$$

Substitute:

$$
x_{n+1}
=
(1-\omega^2h^2)x_n+
hv_n.
$$

Update matrix:

$$
A=
\begin{bmatrix}
1-\omega^2h^2&h\\
-\omega^2h&1
\end{bmatrix}.
$$

Its determinant:

$$
\det A=1.
$$

That volume-preserving/symplectic structure gives much better long-term qualitative behavior.

Energy tends to oscillate around the true value rather than monotonically explode for stable timestep ranges.

That is why semi-implicit Euler is extremely common in games.

---

### "More stable" does not mean unconditionally stable

If:

$$
h
$$

is too large for system frequency, semi-implicit Euler also fails.

For the simple harmonic oscillator the familiar stability condition is related to:

$$
h\omega<2.
$$

So:

$$
\boxed{
\text{Integrator choice cannot rescue an absurd timestep.}
}
$$

---

### Current Kairo integration architecture

Kairo does not literally call:

```cpp
SemiImplicitEuler(...)
```

inside `PhysicsWorld`.

Instead it performs the same sequencing across the whole solver:

```text
Integrate forces
        ↓
update v, ω
        ↓
collision/constraint impulses modify v, ω
        ↓
integrate x, q using the resulting velocities.
```

For dynamic bodies:

```cpp
body.State.LinearVelocity +=
    acceleration * dt;

body.State.AngularVelocity +=
    worldInverseInertia *
    torque *
    dt;
```

then after collision solving:

```cpp
AdvanceMotionState(
    body.State,
    dt);
```

which does:

```cpp
state.Position +=
    state.LinearVelocity * dt;

state.Rotation =
    IntegrateAngularVelocity(
        state.Rotation,
        state.AngularVelocity,
        dt);
```

That is semi-implicit in the important engine sense:

$$
\boxed{
\text{new/solved velocity moves the pose.}
}
$$

---

### Part 1665 — Position integration

For fixed velocity over one step:

$$
\boxed{
x_{n+1}
=
x_n+v\,dt.
}
$$

Kairo:

```cpp
state.Position +=
    state.LinearVelocity * dt;
```

Simple.

But the velocity used is the post-force/post-solver velocity.

That is why order matters.

---

### Position correction versus velocity integration

Collision engines may also directly correct penetrated positions.

That is **not** ordinary velocity integration.

It is constraint stabilization.

Kairo currently performs:

```text
SolveSolverIslands
CorrectSolverIslandPositions
IntegrateDynamicVelocitiesWithCCD
```

so contact/joint correction and normal motion integration are separate concerns.

Good.

---

### Part 1666 — Linear velocity integration

From:

$$
a=\frac{dv}{dt},
$$

over small step:

$$
\boxed{
v_{n+1}
=
v_n+a_n\,dt.
}
$$

For rigid body:

$$
a
=
g
+
m^{-1}F.
$$

So:

$$
\boxed{
v_{n+1}
=
v_n+
\left(
g+m^{-1}F
\right)dt.
}
$$

Kairo does exactly this.

Then applies exponential damping:

$$
v
\leftarrow
ve^{-\lambda dt}.
$$

Then velocity clamp.

This means current effective operation order is:

$$
\text{force integration}
\rightarrow
\text{damping}
\rightarrow
\text{clamp}
\rightarrow
\text{contact solver}.
$$

That order is part of your simulation semantics.

Changing it can change results.

---

### Part 1667 — Quaternion derivative from angular velocity

This is one of the most important derivations.

Suppose angular velocity is world-space:

$$
\omega=
(\omega_x,\omega_y,\omega_z).
$$

Define pure quaternion:

$$
\Omega
=
(\omega_x,\omega_y,\omega_z,0).
$$

For quaternion q mapping body orientation into world coordinates:

$$
\boxed{
\dot q
=
\frac12
\Omega q.
}
$$

Kairo implements:

```cpp
const Quaternionf omega
{
    angularVelocity,
    0.0f
};

const Quaternionf derivative =
    omega *
    rotation *
    0.5f;
```

which matches that convention.

---

### Deriving the half factor

For constant angular velocity over infinitesimal dt, rotation increment is approximately:

$$
dq
\approx
\left(
\frac12\omega dt,
1
\right).
$$

More rigorously, exact increment quaternion is:

$$
\delta q
=
\left(
\hat\omega
\sin\frac{\theta}{2},
\cos\frac{\theta}{2}
\right),
$$

with:

$$
\theta
=
|\omega|dt.
$$

For very small $\theta$:

$$
\sin(\theta/2)
\approx
\theta/2
$$

and:

$$
\cos(\theta/2)
\approx1.
$$

Therefore vector part:

$$
\hat\omega
\frac{|\omega|dt}{2}
=
\frac12\omega dt.
$$

Hence:

$$
q(t+dt)
\approx
q+
\frac12\Omega q\,dt.
$$

That is the quaternion derivative form.

---

### Why multiplication order matters

Quaternion multiplication is noncommutative:

$$
pq\neq qp.
$$

If angular velocity is represented in world coordinates:

$$
\dot q
=
\frac12\Omega_wq.
$$

If represented in body coordinates:

$$
\dot q
=
\frac12q\Omega_b.
$$

You cannot swap these casually.

Kairo's world-space angular velocity and:

```cpp
omega * rotation
```

are coherent with one another.

That consistency is more important than memorizing one universal ordering.

---

### Part 1668 — Quaternion orientation integration

First-order orientation integration:

$$
q_{n+1}
=
q_n+
\dot q_n dt.
$$

Using derivative:

$$
\boxed{
q_{n+1}
=
q_n+
\frac12
\Omega_n q_n dt.
}
$$

Kairo's current implementation:

```cpp
return
    (
        rotation +
        derivative * dt
    ).Normalized();
```

is exactly this.

This is a first-order approximation of rotational exponential motion.

---

### Angular velocity choice in semi-implicit integration

Kairo's `SemiImplicitEuler()` first updates:

```cpp
state.AngularVelocity +=
    angularAcceleration * dt;
```

then calls orientation integration using the **new angular velocity**.

Thus rotational state follows the same semi-implicit philosophy as translation.

---

### Part 1669 — Quaternion renormalization

A valid rotation quaternion must satisfy:

$$
\|q\|=1.
$$

Numerical integration produces:

$$
q'
=
q+\dot qdt.
$$

In finite precision:

$$
\|q'\|
\neq1
$$

exactly.

So normalize:

$$
\boxed{
q
\leftarrow
\frac{q}{\|q\|}.
}
$$

Kairo does this every orientation integration step.

That is correct and cheap.

---

### What happens without normalization

A non-unit quaternion used as though unit may introduce:

```text
scaling-like transform errors
incorrect rotation matrices
gradually increasing orientation distortion.
```

Even if the drift per frame is tiny, over thousands of frames it accumulates.

Kairo's certification test runs orientation integration for:

$$
20,000
$$

steps and checks the quaternion remains normalized within tolerance.

That is a good regression test.

---

### Normalization is not a fix for poor integration

This distinction matters.

Normalizing removes drift away from the unit quaternion manifold.

It does **not** make the angular trajectory high-order accurate.

You can have:

$$
\|q\|=1
$$

while orientation is still numerically wrong.

Normalization fixes representation validity, not integration truncation error.

---

### Part 1670 — Exponential-map orientation integration

If angular velocity is constant over a step, we can integrate orientation more geometrically.

Magnitude:

$$
\omega=
\|\boldsymbol\omega\|.
$$

Rotation angle during dt:

$$
\theta
=
\omega dt.
$$

If:

$$
\omega>0,
$$

axis:

$$
\hat\omega
=
\frac{\boldsymbol\omega}{\omega}.
$$

Increment quaternion:

$$
\boxed{
\Delta q
=
\left(
\hat\omega
\sin\frac{\theta}{2},
\cos\frac{\theta}{2}
\right).
}
$$

For world-space angular velocity:

$$
\boxed{
q_{new}
=
\Delta q\,q.
}
$$

This represents the exact finite rotation corresponding to constant angular velocity over the step.

---

### Exponential-map implementation

```cpp
Quaternionf IntegrateAngularVelocityExpMap(
    const Quaternionf& orientation,
    const Vec3f& angularVelocity,
    float dt)
{
    const float speed =
        angularVelocity.Length();

    // Small-angle branch avoids division by a tiny speed.
    if (speed < 1.0e-7f)
    {
        const Quaternionf omega{
            angularVelocity,
            0.0f
        };

        return (
            orientation +
            omega *
            orientation *
            (0.5f * dt)
        ).Normalized();
    }

    const Vec3f axis =
        angularVelocity /
        speed;

    const float angle =
        speed * dt;

    const float half =
        0.5f * angle;

    const float s =
        std::sin(half);

    const Quaternionf delta{
        axis * s,
        std::cos(half)
    };

    // World-space angular velocity convention.
    return (
        delta *
        orientation
    ).Normalized();
}
```

For constant $\omega$, this gives a much better orientation update than first-order quaternion Euler.

---

### Why exponential map is natural

Rotations do not live in ordinary Euclidean vector space.

They live on:

$$
SO(3).
$$

Angular velocity belongs to the corresponding tangent/Lie algebra:

$$
\mathfrak{so}(3).
$$

The exponential map takes a tangent-space angular displacement:

$$
\omega dt
$$

and maps it onto a finite rotation.

That is precisely what:

$$
\Delta q
$$

is doing.

You do not need advanced Lie theory to use it, but the mathematical structure explains why the method feels so natural.

---

### Is Kairo currently using exponential-map integration?

No.

The current `IntegrateAngularVelocity()` uses:

$$
q_{n+1}
=
Normalize(
q_n+
\frac12\Omega q_n dt
).
$$

That is first-order derivative integration plus renormalization.

It is a respectable V1 choice.

But an exponential-map variant would be a worthwhile improvement, especially for:

```text
large angular speeds
larger dt
research-quality rotational trajectories.
```

---

### Part 1671 — Fixed timestep

A fixed timestep means physics always advances by:

$$
\boxed{
dt=h
}
$$

for example:

$$
h=\frac1{60}s
$$

regardless of render-frame duration.

So the physics sequence is:

```text
step 0: t = 0
step 1: t = 1/60
step 2: t = 2/60
...
```

This gives every physics step the same numerical conditions.

---

### Why fixed timestep helps determinism

If the same state receives the same inputs and executes the same floating-point operations in the same order with the same:

$$
dt,
$$

replay is much more reproducible.

Variable dt changes numerical trajectories even when total elapsed time is identical.

Thus:

$$
\boxed{
fixed dt is one of the foundations of deterministic simulation.
}
$$

It is not sufficient for cross-machine bitwise determinism by itself, but it is enormously helpful.

---

### Part 1672 — Variable timestep instability

Naive game loop:

```cpp
while (running)
{
    float dt =
        TimeSinceLastFrame();

    physics.Step(dt);
}
```

Suppose render frames alternate:

$$
8ms,25ms,12ms,40ms,\dots
$$

Then integration matrix changes every step.

Spring stability changes.

Constraint bias changes.

Collision penetration changes.

Damping changes unless carefully formulated.

The same gameplay event produces different trajectories depending on rendering load.

That is undesirable.

---

### Nonlinear systems are especially sensitive

Generally:

$$
Step(dt_1+dt_2)
\neq
Step(dt_2)(Step(dt_1)).
$$

Numerical integration is not time-partition invariant.

Therefore:

```text
one 32 ms step
```

is not equivalent to:

```text
two 16 ms steps.
```

That is one reason substepping exists.

---

### Part 1673 — Accumulator-based fixed stepping

Suppose render-frame elapsed time:

$$
\Delta t_{frame}.
$$

Maintain accumulator:

$$
A.
$$

Each frame:

$$
A\leftarrow A+\Delta t_{frame}.
$$

Then while:

$$
A\ge h,
$$

perform:

$$
PhysicsStep(h)
$$

and:

$$
A\leftarrow A-h.
$$

Pseudo-code:

```cpp
accumulator += frameDelta;

while (accumulator >= fixedDt)
{
    physics.Step(fixedDt);

    accumulator -= fixedDt;
}
```

Physics consumes real time in fixed-size quanta.

---

### Example

Let:

$$
h=16.6667ms.
$$

Render frame arrives after:

$$
40ms.
$$

Accumulator:

$$
A=40ms.
$$

First step leaves:

$$
23.3333ms.
$$

Second leaves:

$$
6.6666ms.
$$

So physics advances:

$$
33.3334ms
$$

and leaves fractional remainder:

$$
6.6666ms.
$$

That remainder becomes interpolation alpha later.

---

### Current Kairo has two fixed-step layers

`PhysicsWorld::StepFixed()` itself supports an accumulator.

But the actual `RuntimePhysicsBridge` also owns a higher-level accumulator because it additionally needs:

```text
scene pose history
render interpolation
frame-delta clamping
runtime fixed-step callbacks.
```

That bridge-level ownership makes sense.

You generally should not stack both accumulator mechanisms simultaneously.

The runtime bridge calls:

```cpp
m_World.Step(
    FixedDeltaSeconds);
```

directly for each substep.

Good.

---

### Part 1674 — Multiple physics substeps

Suppose one rendered frame is long enough for three fixed steps.

The engine performs:

```text
Physics step
Physics step
Physics step
Render once.
```

This is normal.

The renderer and physics do not need matching frequencies.

Render:

$$
144Hz
$$

Physics:

$$
60Hz
$$

or:

Render:

$$
30Hz
$$

Physics may perform two 60Hz substeps in that frame.

---

### Why substeps improve collision and constraints

A moving body travels approximately:

$$
d=vdt.
$$

Halve dt:

$$
d/2.
$$

This reduces:

```text
penetration per step
constraint error
tunnelling probability
spring instability.
```

But doubles broadphase/narrowphase/solver work.

Substeps trade CPU time for numerical quality.

---

### Part 1675 — Maximum-substep budgets

What if frame stalls for:

$$
1s?
$$

At:

$$
60Hz,
$$

you would need:

$$
60
$$

physics steps in one rendered frame.

Executing all could take so long that the next frame is late too.

Therefore set:

$$
N_{max}.
$$

Kairo runtime default:

$$
\boxed{
MaximumSubsteps=8.
}
$$

It validates range:

$$
1\le N_{max}\le64.
$$

That is a practical safety guard.

---

### Maximum frame delta

Kairo also limits how much elapsed wall time enters the accumulator:

```cpp
m_Accumulator +=
    std::min(
        elapsedSeconds,
        MaximumFrameDeltaSeconds);
```

default:

$$
0.25s.
$$

So a debugger pause of ten seconds does not suddenly request 600 physics steps.

This is another anti-catastrophe mechanism.

---

### Part 1676 — Spiral of death

Imagine physics step h corresponds to:

$$
16.7ms
$$

of simulation time.

But each physics step actually takes:

$$
20ms
$$

CPU time.

You are falling behind.

The frame accumulates more elapsed time while trying to catch up.

More accumulated time requires more physics steps.

More steps take more time.

That creates positive feedback:

$$
\boxed{
\text{behind}
\rightarrow
\text{more work}
\rightarrow
\text{further behind}.
}
$$

This is the **spiral of death**.

---

### The mathematical condition

Suppose physics simulation cost per simulated second exceeds available real CPU time.

Let:

$$
C
$$

be real seconds required to simulate one physics second.

If:

$$
C>1,
$$

you fundamentally cannot keep real time.

Backlog grows without bound.

No accumulator trick can solve an overloaded system.

You must either reduce workload, lower simulation frequency, parallelize, degrade simulation quality, or drop simulation time.

---

### Part 1677 — Dropping excessive accumulated time

Once maximum substeps are exhausted, you must choose a policy.

Kairo's `PhysicsWorld::StepFixed()` currently does:

```cpp
if (
    steps == maxSubSteps &&
    m_FixedAccumulator >= fixedDt)
{
    m_FixedAccumulator = 0.0f;
}
```

The higher-level runtime bridge is slightly more refined:

```cpp
if (
    m_Accumulator >=
    m_Settings.FixedDeltaSeconds)
{
    m_Accumulator =
        std::fmod(
            m_Accumulator,
            m_Settings.FixedDeltaSeconds);

    result.DroppedExcessTime =
        true;
}
```

So it discards whole unprocessed fixed steps but preserves the sub-step remainder.

That is a good explicit overload policy.

---

### What dropping time means physically

Suppose:

$$
200ms
$$

of real elapsed time arrives.

Your substep budget allows only:

$$
8\times16.67
\approx133.3ms.
$$

Some simulation time is lost.

The game world advances less than wall time.

This produces a temporary slow-motion-like discontinuity.

But it prevents runaway catch-up.

That is often preferable.

---

### Other overload policies

A different engine might preserve all accumulated time and accept temporary frame stalls, clamp frame delta earlier, dynamically lower solver quality, run physics slower than real time, or resynchronize state from authoritative network/server state.

There is no universal answer.

Kairo deliberately chooses:

$$
\boxed{
\text{bounded simulation work over perfect wall-time catchup}.
}
$$

That is a reasonable game-runtime policy.

---

### Part 1678 — Previous/current physics snapshots

A fixed-step simulation updates only at discrete times:

$$
t_n,
t_{n+1},
t_{n+2}.
$$

Rendering may happen in between.

So retain two physics poses:

$$
Pose_{previous}
$$

and:

$$
Pose_{current}.
$$

Kairo's `RuntimePhysicsBridge` has:

```cpp
struct PoseHistory
{
    Transformf Previous;
    Transformf Current;
};
```

Before every fixed step:

```cpp
pose.Previous =
    pose.Current;
```

After physics:

```cpp
pose.Current.Translation =
    state.Position;

pose.Current.Rotation =
    state.Rotation;
```

This is exactly the architecture required for interpolation.

---

### "Snapshot" has two meanings

There are actually two different snapshot concepts in current Kairo.

The lightweight render snapshot is:

```text
Previous / Current Transform
```

used every frame for interpolation.

The complete deterministic simulation snapshot is:

```cpp
PhysicsWorldSnapshot
```

used for restore/replay/topology transactions.

Do not confuse them.

One is presentation history.

The other is full simulation state.

---

### Full physics snapshot

Kairo's replay system stores an initial:

```cpp
PhysicsWorldSnapshot
```

then a sequence of commands and expected state hashes.

Replay:

```text
restore snapshot
apply frame commands
fixed Step
hash state
compare.
```

That is an excellent deterministic-regression architecture.

It tests much more than visual output.

---

### Snapshot must include hidden dynamic memory

A correct deterministic world snapshot cannot store only:

```text
position
rotation.
```

It may need:

```text
linear/angular velocity
sleep state/timer
forces if captured between input and step
collider topology
constraint state
contact cache / warm-start information
fixed accumulator where relevant.
```

Anything influencing future evolution belongs in the state closure.

Kairo's physics snapshot/restore system is already significantly richer than a pose dump.

---

### Part 1679 — Render interpolation between physics snapshots

Let accumulator remainder be:

$$
A,
$$

fixed timestep:

$$
h.
$$

Define:

$$
\boxed{
\alpha
=
\frac{A}{h}.
}
$$

Since:

$$
0\le A<h,
$$

we have:

$$
0\le\alpha<1.
$$

Then rendered position:

$$
\boxed{
x_r
=
(1-\alpha)x_{prev}
+
\alpha x_{curr}.
}
$$

Kairo computes exactly:

```cpp
InterpolationAlpha =
    clamp(
        accumulator /
        FixedDeltaSeconds,
        0.0f,
        1.0f);
```

then calls:

```cpp
Interpolate(
    Previous,
    Current,
    alpha);
```

---

### Why rendering current physics pose directly jitters

Physics 60 Hz means pose changes every:

$$
16.67ms.
$$

At 144 Hz, rendering every:

$$
6.94ms.
$$

Several rendered frames see the same physics transform before it suddenly jumps.

Visually:

```text
frame 1: x0
frame 2: x0
frame 3: x0
frame 4: x1
frame 5: x1
...
```

Interpolation produces smoothly moving presentation poses between snapshots.

---

### Rotation interpolation

Positions use linear interpolation.

Orientations require quaternion interpolation.

For normalized quaternions, common choices are:

$$
SLERP
$$

or normalized linear interpolation:

$$
NLERP.
$$

SLERP:

$$
q(t)
=
\frac{
\sin((1-t)\theta)
}{
\sin\theta
}q_0
+
\frac{
\sin(t\theta)
}{
\sin\theta
}q_1.
$$

It gives constant angular speed along the shortest great-circle path after quaternion-sign correction.

NLERP:

$$
q(t)
=
Normalize(
(1-t)q_0+tq_1
)
$$

is cheaper and often visually sufficient.

Kairo's generic `Transformf::Interpolate` owns the exact policy.

---

### Interpolation introduces presentation latency

The classic previous/current interpolation scheme renders a state somewhere between the last two completed physics states.

That means presentation is roughly one physics step behind the most recently theoretically extrapolatable state.

At:

$$
60Hz
$$

this is roughly up to:

$$
16.7ms.
$$

The trade is:

$$
\boxed{
slight latency
\leftrightarrow
smooth, stable presentation.
}
$$

For most objects this is excellent.

For latency-sensitive local player cameras/controllers, engines sometimes use different prediction/presentation strategies.

---

### Teleports and interpolation

Suppose body teleports from:

$$
x=0
$$

to:

$$
x=100.
$$

If history remains:

```text
Previous = 0
Current = 100
```

renderer will animate a fake high-speed movement across the world.

Wrong.

On teleport:

$$
Previous=Current=100.
$$

Kairo's `SetEntityPosition()` does exactly this:

```cpp
pose.Previous = world;
pose.Current = world;
```

Excellent detail.

Snapshot restore does the same thing so the first frame after restore does not interpolate from pre-load state.

---

### Part 1680 — Physics→scene transform synchronization

Dynamic body is owned by physics.

After simulation:

$$
x_{physics},q_{physics}
$$

must become the runtime scene pose used by rendering/gameplay presentation.

The naive approach:

```cpp
sceneTransform =
    physicsTransform;
```

works only for root entities with identical coordinate conventions and no parent.

Kairo handles the harder case.

---

### Scene hierarchy problem

Suppose child entity physics produces world transform:

$$
T_w^{child}.
$$

Scene stores local transform relative to parent:

$$
T_l^{child}.
$$

If parent world transform is:

$$
T_w^{parent},
$$

then:

$$
\boxed{
T_l^{child}
=
(T_w^{parent})^{-1}
T_w^{child}.
}
$$

Kairo's `ToLocal()` performs this conversion.

For translation it uses:

```cpp
WorldToLocal(
    parentWorld,
    world.Translation);
```

for rotation:

```cpp
Inverse(parentWorld.Rotation) *
world.Rotation;
```

and scale is divided component-wise.

---

### Why parent-first publication is required

When converting a child's interpolated world pose into local coordinates, you need its parent's **current published world transform**.

Therefore process:

```text
root
  ↓
child
  ↓
grandchild
```

not arbitrary entity storage order.

Kairo explicitly traverses:

```cpp
for (root : RootEntities())
    PublishSubtree(root, alpha);
```

and recursively publishes children.

This is exactly the right scene-hierarchy synchronization rule.

---

### Physics should not directly depend on Scene

Notice the architecture:

```text
PhysicsWorld
```

does not know:

```text
EngineCore::Scene.
```

The bridge owns synchronization.

That preserves:

$$
PhysicsEngine
\nrightarrow
EngineCore scene policy.
$$

The integration layer depends on both.

This is Phase 16 architecture paying dividends inside Phase 17.

---

### Dynamic ownership direction

For dynamic body:

$$
\boxed{
Physics\rightarrow Scene.
}
$$

The physics pose is authoritative.

Scene receives an interpolated presentation transform.

Do not, in the same frame, also blindly copy scene transform back into the dynamic body.

That would create ownership conflict.

---

### Part 1681 — Scene→kinematic synchronization

Kinematic bodies reverse the authority direction.

For kinematic:

$$
\boxed{
Scene/Game logic\rightarrow Physics.
}
$$

The application specifies target movement.

Physics uses that movement in contacts but does not solve forces onto the kinematic body.

That makes synchronization fundamentally asymmetric:

$$
Dynamic:
Physics\rightarrow Scene
$$

$$
Kinematic:
Scene\rightarrow Physics.
$$

This is a crucial architecture invariant.

---

### Kinematic pose update should occur before physics step

Correct timeline:

```text
gameplay updates kinematic target
        ↓
scene/animation computes world pose
        ↓
bridge updates kinematic physics pose/velocity
        ↓
PhysicsWorld::Step
        ↓
contacts see moving kinematic velocity
        ↓
dynamic bodies respond.
```

If you push kinematic pose after physics, contacts are one step late.

---

### Deriving kinematic linear velocity

Given previous:

$$
x_0
$$

and target:

$$
x_1,
$$

fixed timestep:

$$
h,
$$

set:

$$
\boxed{
v_k
=
\frac{x_1-x_0}{h}.
}
$$

Then advance over h:

$$
x_0+v_kh=x_1.
$$

This lets contacts see the true platform velocity.

---

### Deriving kinematic angular velocity from quaternions

Previous orientation:

$$
q_0.
$$

Target:

$$
q_1.
$$

Relative rotation for world convention:

$$
\Delta q
=
q_1q_0^{-1}.
$$

Choose quaternion sign so:

$$
\Delta q_w\ge0
$$

for shortest path.

Write:

$$
\Delta q
=
(
\hat n\sin(\theta/2),
\cos(\theta/2)
).
$$

Then:

$$
\theta
=
2\operatorname{atan2}
(
\|\Delta q_v\|,
\Delta q_w
).
$$

So:

$$
\boxed{
\omega_k
=
\hat n
\frac{\theta}{h}.
}
$$

That is the angular velocity that carries q0 to q1 over the fixed step.

---

### A proper kinematic target API

Rather than directly mutating raw state:

```cpp
void SetKinematicTarget(
    BodyID body,
    const Vec3f& targetPosition,
    const Quaternionf& targetRotation,
    float fixedDt)
{
    RigidBody& b =
        RequireBody(body);

    if (b.Type != BodyType::Kinematic)
        throw ...;

    // Linear velocity that reaches target exactly at the
    // end of the next fixed simulation step.
    b.State.LinearVelocity =
        (
            targetPosition -
            b.State.Position
        ) / fixedDt;

    // Relative finite rotation.
    Quaternionf delta =
        targetRotation *
        b.State.Rotation.Conjugate();

    // q and -q represent the same orientation.
    // Flip to use the shortest angular route.
    if (delta.w < 0.0f)
        delta = -delta;

    delta =
        delta.Normalized();

    const float sinHalf =
        Vec3f{
            delta.x,
            delta.y,
            delta.z
        }.Length();

    if (sinHalf < 1.0e-7f)
    {
        b.State.AngularVelocity =
            Vec3f::Zero();
    }
    else
    {
        const Vec3f axis =
            Vec3f{
                delta.x,
                delta.y,
                delta.z
            } /
            sinHalf;

        const float angle =
            2.0f *
            std::atan2(
                sinHalf,
                delta.w);

        b.State.AngularVelocity =
            axis *
            (angle / fixedDt);
    }
}
```

Now collision response sees correct kinematic surface velocity.

---

### Current Kairo Part 1681 status

The runtime bridge does an excellent job for:

$$
Physics\rightarrow Scene
$$

and exposes a:

```cpp
RuntimeFixedStepListener::BeforePhysicsStep()
```

hook precisely at the right point for systems that need to make fixed-step input changes before physics.

It also provides explicit world-position synchronization through:

```cpp
SetEntityPosition().
```

What is not intrinsic in the inspected `RuntimePhysicsBridge` is a generic automatic pass that, before every substep, scans authored kinematic Scene transforms and derives their linear/angular velocities from changed target transforms.

Therefore Part 1681's **conceptual architecture is clear**, while the fully general automatic scene→kinematic target pipeline appears to remain an extension point.

That is a useful gap rather than a hidden one.

---

### KAIRO DEEP AUDIT — IMPULSE APPLICATION ORDER

Current player call:

```cpp
ApplyEntityImpulse(...)
```

reaches:

```cpp
PhysicsWorld::ApplyBodyImpulseAtPoint(...)
```

which performs:

```cpp
if (!IsDynamicBodyType(record))
{
    return;
}

WakeRigidBody(record);

ApplyImpulseAtPoint(
    record.State,
    record.Mass.InverseMass,
    WorldInverseInertia(record),
    impulse,
    worldPoint);
```

Notice the careful order.

It checks **body type**, not:

```cpp
IsDynamic(record)
```

because `IsDynamic()` returns false when sleeping.

A sleeping body is still allowed to receive an impulse.

Then:

```cpp
WakeRigidBody(record);
```

happens before:

```cpp
WorldInverseInertia(record).
```

This matters because `WorldInverseInertia()` returns zero if the body is not currently dynamic/awake.

So the ordering prevents a sleeping body's angular impulse from being accidentally lost.

That is a genuinely good implementation detail.

---

### Force versus impulse timing inside the current world

Current broad sequence is:

```text
IntegrateForces
        ↓
Kinematic motion update
        ↓
Broadphase
        ↓
Narrowphase
        ↓
Contact solver / impulses
        ↓
Position correction
        ↓
Dynamic pose integration
        ↓
Sleeping
        ↓
Clear force accumulators
```

This means external continuous forces affect velocity before contact solving.

Contact solver then sees those new velocities.

Finally post-solver velocities advance positions.

That is exactly the ordering you generally want for semi-implicit rigid-body simulation.

---

### Why immediate gameplay impulses can be applied before the step

Suppose gameplay does:

```cpp
ApplyEntityImpulse(player, jumpImpulse);
```

then:

```cpp
physics.Advance(...);
```

The body's velocity changes immediately before force integration/contact solving.

The next solver step therefore sees the new jump velocity.

Good.

If impulse were stored for some later arbitrary phase, ordering would be harder to reason about.

---

### But contact-solver impulses are internal to the step

External gameplay impulse:

```text
user-facing event
```

and contact impulse:

```text
constraint solution
```

share the same mathematics:

$$
\Delta v=M^{-1}J.
$$

But they occur at different architectural layers.

This is important.

Do not expose the sequential solver's internal accumulated contact impulse as though it were simply a gameplay impulse API.

The math is the same; ownership and lifetime are not.

---

### Contact effective mass preview

Although full contact constraints belong to Pass 18, M099's impulse system is already visible in Kairo's:

```cpp
ContactNormalDenominator(...)
```

The scalar denominator is:

$$
k
=
m_A^{-1}
+
m_B^{-1}
+
n\cdot
\left[
(I_A^{-1}(r_A\times n))\times r_A
+
(I_B^{-1}(r_B\times n))\times r_B
\right].
$$

Then normal impulse magnitude is approximately:

$$
\boxed{
j
=
-\frac{
(1+e)v_n
}{
k
}.
}
$$

This denominator is effectively the inverse of the two-body **effective mass along the contact normal**.

So everything we learned about linear and angular impulse is already assembling into the collision solver.

---

### Rigid-body state memory and ownership

There is a broader lesson here.

The body has multiple categories of "memory":

$$
\boxed{
\text{configuration memory}
}
$$

such as mass, damping and body type;

$$
\boxed{
\text{dynamic memory}
}
$$

such as position, velocity, sleeping state;

$$
\boxed{
\text{step-local memory}
}
$$

such as force accumulator;

and later:

$$
\boxed{
\text{solver memory}
}
$$

such as warm-start impulses.

When implementing snapshot/replay, you need to decide which memory affects future state.

Anything that does belongs to deterministic simulation state.

---

### Sleeping and determinism

Sleeping is not merely an optimization.

It changes the executed equations.

One run that sleeps a body at step:

$$
N
$$

while another does not can diverge immediately.

Therefore thresholds:

```text
SleepLinearSpeed
SleepAngularSpeed
SleepTime
```

must be considered deterministic simulation parameters.

They belong in replay/build configuration when exact simulation reproduction matters.

---

### Fixed timestep and determinism are not synonymous

Fixed timestep helps, but two machines can still diverge from:

```text
different floating-point instruction selection
FMA differences
parallel solver ordering
unordered iteration
different SIMD implementations
compiler optimization.
```

Kairo's physics replay hash architecture is therefore important.

Rather than assuming determinism, it can test:

$$
ExpectedStateHash
=
ActualStateHash.
$$

That is much stronger.

---

### Cross-platform deterministic replay

For serious deterministic requirements, you eventually need to control:

$$
\text{iteration order}
$$

$$
\text{floating-point environment}
$$

$$
\text{parallel reduction ordering}
$$

$$
\text{random seeds}
$$

$$
\text{solver order}
$$

$$
\text{dt}
$$

$$
\text{initial state}.
$$

Exact floating-point bitwise determinism across architectures is a very high bar.

For research replication, record enough state and tolerate carefully defined numerical error where bitwise identity is not mandatory.

---

### Fixed-step listener architecture

The runtime bridge currently invokes:

```cpp
listener->BeforePhysicsStep(
    FixedDeltaSeconds);
```

inside every substep immediately before:

```cpp
m_World.Step(
    FixedDeltaSeconds);
```

This is the right place for fixed-step simulation inputs.

For example:

```text
controller motor forces
kinematic target synchronization
vehicle controls
scripted physics actions.
```

Why not once per render frame?

Because one render frame may contain:

$$
0,1,2,\dots,N
$$

physics steps.

Physics-sensitive input must be sampled/applied at the physics-step boundary.

---

### Render-frame input versus physics-step input

Suppose one render frame triggers two physics substeps.

If you call:

```cpp
AddForce(...)
```

once before both, Kairo clears force after first `Step()`.

Second step gets no force.

If thrust is intended continuous across the entire frame, it needs to be re-applied before each fixed substep.

That is exactly why:

```cpp
BeforePhysicsStep()
```

exists.

This is an important integration detail.

---

### Continuous force with substeps

Suppose thrust:

$$
F=100N.
$$

One render frame requires three fixed steps.

Correct:

```text
step 1:
AddForce(100)
Step(h)

step 2:
AddForce(100)
Step(h)

step 3:
AddForce(100)
Step(h)
```

Total impulse:

$$
J
=
100(3h).
$$

If you apply force only once:

$$
J
=
100h.
$$

You under-apply by factor three.

This bug is extremely common when people first combine force accumulators with fixed stepping.

---

### Impulse under multiple substeps

An impulse is different.

A gameplay event:

```text
jump pressed
```

should usually apply once:

$$
J.
$$

Do not reapply it every substep unless the event semantics actually demand it.

So your fixed-step input layer must distinguish:

```text
stateful continuous control
```

from:

```text
edge-triggered impulse event.
```

For example:

```text
throttle held → force every step
jump pressed → impulse once.
```

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/36 - Auditing capsule mass properties and collision formulas|Previous chapter]] · [[Chapter Edition/03 - Game Engine/38 - Integrator choices and energy drift|Next chapter]]
