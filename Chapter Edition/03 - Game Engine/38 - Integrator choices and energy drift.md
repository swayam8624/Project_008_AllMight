# 38 — Integrator choices and energy drift

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/37 - Physical examples and state validation|Previous chapter]] · [[Chapter Edition/03 - Game Engine/39 - Composable timesteps and stable simulation policy|Next chapter]]

The same force law can produce different motion when time is discretized differently. This chapter compares explicit and semi-implicit updates, damping, and energy behavior without confusing numerical drift with physical dissipation.

**By the end:** Derive one-step updates and predict stability and energy changes for a stated step size and force model.

**Recall question:** For the same initial state and step, how do the two integrators differ, and which energy change comes from damping versus discretization?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 92392–95877; source-span SHA256 bbbc9ae48404078c70158b6bf68ab03e87b3f3848d0936b107e29c38e9410358. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Explicit versus semi-implicit energy example

Consider:

$$
m=1,\quad k=1.
$$

Initial:

$$
x_0=1,\quad v_0=0.
$$

Take:

$$
h=0.1.
$$

Acceleration:

$$
a=-x.
$$

Initial energy:

$$
E_0
=
\frac12v^2+
\frac12x^2
=
0.5.
$$

Explicit Euler:

$$
x_1=1+0(0.1)=1.
$$

$$
v_1=0-1(0.1)=-0.1.
$$

Energy:

$$
E_1
=
0.5(0.01)
+
0.5(1)
=
0.505.
$$

Already increased.

Semi-implicit:

$$
v_1=-0.1.
$$

$$
x_1=1+(-0.1)(0.1)=0.99.
$$

Energy:

$$
E_1
=
0.005+
0.5(0.9801)
=
0.49505.
$$

It moved slightly below true energy instead of systematically growing.

Across many oscillations, symplectic behavior is much better suited to game dynamics.

---

### Damping and fixed timestep

Kairo uses:

$$
v\leftarrow ve^{-\lambda h}.
$$

Suppose you split one step h into two h/2 steps:

$$
v'
=
v e^{-\lambda h/2}
e^{-\lambda h/2}.
$$

Therefore:

$$
v'
=
v e^{-\lambda h}.
$$

So the damping factor composes exactly across substeps.

That is a beautiful property.

Compare naive damping:

$$
v\leftarrow v(1-\lambda h).
$$

Two half steps produce:

$$
v(1-\lambda h/2)^2
$$

which differs from:

$$
v(1-\lambda h).
$$

Kairo's exponential damping is therefore well suited to fixed/substepped simulation.

---

### Quaternion Euler versus exponential map numerical comparison

Take:

$$
\omega=(0,0,\pi)
$$

meaning:

$$
180^\circ/s.
$$

Let:

$$
dt=0.5.
$$

Exact rotation over step:

$$
\theta
=
\pi(0.5)
=
\pi/2.
$$

So exact incremental quaternion:

$$
q_{exact}
=
(0,0,\sin\pi/4,\cos\pi/4)
$$

$$
\approx
(0,0,0.7071,0.7071).
$$

First-order quaternion Euler from identity:

$$
q'
=
(0,0,\pi/4,1).
$$

Normalize.

Length:

$$
\sqrt{
1+
(\pi/4)^2
}
\approx1.2716.
$$

Normalized:

$$
q'
\approx
(0,0,0.6177,0.7864).
$$

This corresponds to a smaller angle than:

$$
90^\circ.
$$

At small dt error is much smaller, which is why fixed 60/120/240 Hz can make first-order orientation integration acceptable.

But the exponential map is clearly superior for large angular increments.

---

### Angular velocity clamping

Kairo has:

```cpp
MaxAngularSpeed.
```

Why?

Orientation integrators become inaccurate when:

$$
|\omega|dt
$$

is huge.

Collision detection can also fail because shape rotates dramatically inside one step.

Clamping is not physically pure, but is a pragmatic stability guard.

A more principled criterion might limit:

$$
\theta_{step}
=
|\omega|dt
$$

rather than angular velocity alone.

For example:

$$
\theta_{step}\le0.5rad
$$

could inform adaptive rotational substeps.

---

### Maximum linear speed and CCD

Similarly:

$$
d_{step}
=
|v|dt.
$$

If this exceeds characteristic collider thickness, discrete collision can tunnel.

Kairo offers:

```cpp
CollisionDetectionMode::Continuous
```

for fast movers.

Substeps and CCD solve related but different problems.

Smaller dt reduces travel per step.

CCD reasons about swept motion even when travel remains large.

---

### Fixed timestep as a frequency choice

At:

$$
60Hz:
h=0.0166667.
$$

At:

$$
120Hz:
h=0.0083333.
$$

At:

$$
240Hz:
h=0.0041667.
$$

Higher frequency generally improves numerical accuracy and constraint resolution, but increases:

$$
\text{broadphase cost}
+
\text{narrowphase cost}
+
\text{solver cost}
+
\text{integration cost}.
$$

If one physics step costs:

$$
C,
$$

CPU cost per real second is approximately:

$$
fC.
$$

Doubling frequency roughly doubles simulation work if all else remains equal.

---

### Simulation frequency must match problem scale

A slow third-person game may tolerate:

$$
60Hz.
$$

High-speed racing/robotics may prefer:

$$
120Hz
$$

or more.

Stiff research simulations may require much smaller h.

There is no universally correct physics frequency.

The right h depends on:

```text
object speed
collider thickness
constraint stiffness
mass ratios
desired energy accuracy
solver iteration count
CPU budget.
```

---

### Substeps versus solver iterations

These are not interchangeable.

Increasing solver iterations improves constraint convergence **within one timestep**.

Increasing substeps reduces timestep size and changes the temporal discretization itself.

For a bad spring instability, extra solver iterations may do nothing if the spring force is outside the constraint solver.

For tunnelling, more contact iterations may not fix a missed collision.

You need to know which error source you are addressing.

---

### Physics snapshots and replay architecture in current Kairo

Current `PhysicsReplay` records:

```cpp
float FixedDt;
PhysicsWorldSnapshot InitialState;
vector<PhysicsReplayFrame> Frames;
```

Each frame stores:

```cpp
vector<PhysicsReplayCommand>
ExpectedStateHash
```

Commands include:

```text
AddForce
AddTorque
ImpulseAtPoint
SetMotionState
WakeBody
SetCCD.
```

Replay does:

```text
Restore InitialState
       ↓
for each frame:
    apply exact commands
    Step(FixedDt)
    hash result
    compare expected.
```

This is an excellent system for detecting physics regressions.

It converts physics correctness into:

$$
\boxed{
State_{n+1}
=
F(State_n,Inputs_n,h)
}
$$

and checks that F remains stable.

---

### Why replay stores commands rather than frame deltas

If you only recorded resulting poses, replay would test serialization, not simulation.

Recording **inputs** and recomputing state tests the actual engine.

Expected state hash then answers:

> Did this code produce the same next state?

That is far more valuable.

---

### Physics restore and interpolation history

When `RuntimePhysicsBridge::RestoreSnapshot()` restores state, it does:

```cpp
m_Accumulator = 0.0f;
m_Events.clear();
```

then for every mapped entity sets:

```cpp
pose.Previous = world;
pose.Current = world;
```

This is very good.

Otherwise:

```text
Previous = pre-load transform
Current  = restored transform
```

would visually interpolate through a nonsensical path.

State restore needs presentation-history reset.

---

### Render interpolation should not feed back into physics

Another critical invariant:

$$
\boxed{
InterpolatedScenePose
\neq
PhysicsState.
}
$$

The scene pose rendered at alpha may not equal either exact physics snapshot.

Never take that interpolated pose on the next frame and use it as the new dynamic physics position.

That would corrupt deterministic fixed stepping.

Kairo keeps separate:

```text
PhysicsWorld state
PoseHistory
Scene presentation transform.
```

That separation is correct.

---

### Dynamic hierarchy caveat

Physics generally prefers world-space independent rigid bodies.

Scene hierarchies introduce parent transforms.

Suppose a dynamic child has a moving dynamic parent.

If both are independently simulated, the scene hierarchy may impose an additional transform relationship that physics does not enforce.

Kairo can convert each physics world pose back to a local scene transform parent-first, but that does not automatically create a physical constraint.

If you want two bodies physically linked, use a joint/constraint rather than relying on visual hierarchy.

This distinction is important.

---

### Static children under moving parents

A scene hierarchy can also make something authored as "static" move indirectly if its parent moves.

That contradicts static physics semantics.

The bridge should either disallow such topology, treat it as kinematic, rebuild its world pose appropriately, or define static relative to the simulation world snapshot.

These are engine-policy decisions.

The physics body type itself should remain semantically clear.

---

### Kinematic synchronization and hierarchy

For scene→kinematic:

1. evaluate the authored scene hierarchy first;
2. obtain final kinematic world target;
3. derive linear/angular velocity over fixed h;
4. update physics kinematic state;
5. run physics.

Do not derive velocity from raw local transform alone when parent is moving.

The physical body moves according to **world-space target motion**.

---

### Kinematic teleport versus kinematic sweep

There are two different APIs you may eventually want.

`TeleportKinematic()` says:

> Body instantly moves; do not derive ordinary collision velocity from the full displacement.

`SetKinematicTarget()` says:

> Move from current to target over the next fixed step; contacts should see corresponding velocity.

Those semantics are very different.

A single generic:

```cpp
SetTransform()
```

often becomes ambiguous.

---

### Sleeping and kinematics

Kinematic bodies normally do not need dynamic sleeping semantics because their movement is externally owned.

A stationary kinematic body can be treated cheaply, but "sleep" does not have the same meaning.

Dynamic bodies sleeping against a kinematic body should wake when the kinematic starts moving into them.

That is why motion changes need to propagate wake events.

---

### Static-to-kinematic transition

Changing body type at runtime is more than changing an enum.

Static:

$$
m^{-1}=0,\quad I^{-1}=0.
$$

Kinematic also has zero inverse mass but may have velocities.

Dynamic needs real mass properties.

Thus transitions can require:

```text
mass/inertia reconstruction
broadphase policy update
sleep state reset
velocity policy decision
contact cache invalidation.
```

Do not expose body type as an unconstrained public field if production runtime transitions are supported.

Use a controlled method.

---

### Dynamic-to-static transition

What happens to:

$$
v,\omega?
$$

Likely reset to zero.

What about force accumulator?

Clear.

What about sleeping?

Static is conceptually inactive.

What about contacts?

Solver effective masses change immediately.

This illustrates why mutation APIs should own invariants rather than direct struct field writes.

---

### Kairo's public mutable body vector

Current `PhysicsWorld` exposes:

```cpp
std::vector<RigidBody>& Bodies();
```

This is convenient for integrations and replay, but it weakens invariants because external code can directly alter:

```text
Type
Mass
ID
Active
Sleep
```

without going through validation.

For a production boundary, consider exposing narrower mutation APIs:

```text
SetMotionState
SetKinematicTarget
SetBodyType
SetMassProperties
SetVelocity
```

while returning only const body views publicly.

The replay code currently directly assigns:

```cpp
world.Bodies().at(id).State =
    value.State;
```

after validation, which is controlled internally enough today, but the API surface is broader than ideal.

---

### Snapshot topology and runtime identity

`RuntimePhysicsBridge::RestoreSnapshot()` validates that restored active body topology matches existing scene↔body maps.

That is important because:

```text
BodyID
```

is process-local runtime identity.

A random physics snapshot from another topology cannot safely be grafted onto the running scene.

The bridge checks the active-body closure before committing restore.

Excellent transactional thinking.

---

### A more complete interpolation architecture

The canonical game loop is:

```cpp
double accumulator = 0.0;

while (running)
{
    const double frameDelta =
        GetFrameDelta();

    accumulator +=
        std::min(
            frameDelta,
            maxFrameDelta);

    ReadFrameInput();

    uint32_t steps = 0;

    while (
        accumulator >= fixedDt &&
        steps < maxSubsteps)
    {
        // Preserve previous exact physics pose.
        CopyCurrentToPrevious();

        // Apply forces, controllers and kinematic
        // targets for THIS fixed step.
        FixedUpdateInput(fixedDt);

        physics.Step(fixedDt);

        CopyPhysicsToCurrentPose();

        accumulator -= fixedDt;
        ++steps;
    }

    if (accumulator >= fixedDt)
    {
        accumulator =
            std::fmod(
                accumulator,
                fixedDt);
    }

    const float alpha =
        static_cast<float>(
            accumulator /
            fixedDt);

    BuildInterpolatedPresentation(alpha);

    Render();
}
```

This is essentially the architecture your `RuntimePhysicsBridge::Advance()` already implements.

---

### Important alternative interpretation of interpolation alpha

There are two common schemes.

The classic accumulator interpolation using:

$$
\alpha=A/h
$$

and previous/current snapshots renders:

$$
Lerp(previous,current,\alpha).
$$

That intentionally presents a state one physics interval behind the theoretical current wall-clock time.

Another scheme extrapolates:

$$
current + velocity\cdot A,
$$

which reduces latency but predicts into an uncertain future.

Interpolation is stable.

Extrapolation is lower-latency but can be wrong after collisions.

Kairo correctly chooses interpolation.

---

### Why physics should not run at render rate

If physics update is tied to rendering:

```text
240 Hz monitor → 240 physics steps
30 Hz machine  → 30 physics steps.
```

Then CPU capability changes gameplay.

That is unacceptable for most simulations.

Fixed stepping decouples:

$$
\boxed{
simulation frequency
}
$$

from:

$$
\boxed{
presentation frequency.
}
$$

This is one of the most fundamental architecture boundaries in real-time engines.

---

### Why rendering should not wait for the next physics step

Opposite extreme:

```text
render only after each physics step
```

means a 60 Hz physics engine can never present smoother than 60 Hz.

With previous/current interpolation, a 144/240 Hz renderer can produce visually smooth motion from a 60 Hz simulation.

That is the whole point of snapshots.

---

### Input sampling problem

Suppose render runs at 240 Hz and physics at 60 Hz.

You receive four input samples before one physics step.

How do you apply them?

For axes such as throttle:

```text
latest value
time-weighted average
```

may be appropriate.

For edge events:

```text
jump pressed
fire pressed
```

you must latch the event until a fixed step consumes it.

Otherwise a press could occur and release entirely between physics steps and be missed.

Thus fixed timestep architecture implies an **input buffering architecture**.

This connects directly to the system scheduling material from Pass 11.

---

### Impulse event buffering

A robust jump input:

```cpp
struct PhysicsInputBuffer
{
    bool JumpPressed = false;
    float Throttle = 0.0f;
};
```

Render/input thread:

```cpp
if (jumpEdge)
    buffer.JumpPressed = true;

buffer.Throttle =
    currentThrottle;
```

Fixed step:

```cpp
if (buffer.JumpPressed)
{
    ApplyImpulse(
        player,
        jumpImpulse);

    buffer.JumpPressed = false;
}

AddForce(
    player,
    throttleForce *
        buffer.Throttle);
```

Impulse consumed once.

Force applied every substep.

That distinction follows directly from M099.

---

### Force timing with variable render rate

Suppose render frame runs no physics steps because:

$$
A<h.
$$

If you add a force immediately to the body's accumulator outside the fixed-step loop, that accumulator persists until the next physics step.

That may be okay.

But if another render frame overwrites/changes it or several control systems accumulate multiple times before a step, behavior may become render-frequency dependent.

Therefore engine-level continuous control is usually cleaner inside:

```text
FixedUpdate / BeforePhysicsStep.
```

External one-off force APIs still have uses, but simulation-system forces should align with physics steps.

---

### Rigid-body memory and cache behavior

Current AoS:

```text
ID
Active
Type
MotionState
Mass
Forces
gravity/damping/etc
sleep state.
```

A force-integration loop touches roughly:

```text
Active
Type
Sleeping
Mass.InverseMass
State velocities
Forces
Gravity settings
Damping/clamp settings.
```

It does not need:

```text
collision detection mode
some identity metadata
possibly debug fields.
```

A future large-scale engine might hot/cold split:

```cpp
struct BodyMotionHot
{
    Vec3f Position;
    Quaternionf Rotation;
    Vec3f LinearVelocity;
    Vec3f AngularVelocity;

    Vec3f Force;
    Vec3f Torque;

    float InverseMass;
    Matrix3f WorldInverseInertia;
};

struct BodyConfigCold
{
    BodyType Type;
    ...
};
```

But again: profile first.

For a modest engine body count, readability can outweigh marginal cache gains.

---

### Impulse invariants worth testing

For COM impulse:

$$
\Delta\omega=0.
$$

For off-center impulse:

$$
\Delta v=m^{-1}J
$$

regardless of application point.

Angular change:

$$
\Delta\omega
=
I^{-1}(r\times J).
$$

For static:

$$
\Delta v=0,
\quad
\Delta\omega=0.
$$

For equal-and-opposite two-body internal impulse:

$$
\Delta P_{total}=0.
$$

These should be direct unit tests.

Kairo already has a certification test applying center-of-mass impulses across many inverse masses and verifying:

$$
\Delta v=Jm^{-1}
$$

and:

$$
\Delta\omega=0.
$$

Good.

---

### Angular impulse conservation

Suppose two bodies receive internal contact impulse:

$$
J
$$

and:

$$
-J.
$$

Linear momentum change:

$$
\Delta P
=
J-J
=
0.
$$

Angular momentum around a common origin also balances when contact impulse is applied at the same world contact point.

For body A:

$$
\Delta L_A
=
r_A\times(-J)
$$

depending on normal convention.

For body B:

$$
\Delta L_B
=
r_B\times J.
$$

Including orbital angular momentum of COMs, the total internal interaction conserves angular momentum under ideal conditions.

This is the physical basis of contact impulses.

---

### Why contact impulses depend on rotational inertia

Suppose two bodies collide at their centers.

No angular arms:

$$
r_A=r_B=0.
$$

Effective denominator reduces to:

$$
m_A^{-1}+m_B^{-1}.
$$

Now move contact point far from COM.

The bodies can respond by rotating.

That gives additional ways to absorb contact velocity.

The effective denominator gains angular terms.

Therefore the scalar impulse needed to correct the same normal velocity becomes smaller.

This is why contact response cannot be computed using mass alone for off-center contacts.

---

### Time-step dependence of restitution

Ideal restitution relation:

$$
v_n'
=
-e v_n
$$

is conceptually impulse-based and not directly multiplied by dt.

But penetration correction/bias often contains terms like:

$$
\frac{\beta}{dt}\,\text{penetration}.
$$

Therefore solver behavior can still depend on fixed dt.

Another reason changing physics frequency changes the simulation even if impulse equations themselves have no dt.

#### First-use completion — restitution, penetration correction, and the timestep are different mechanisms

We can calculate the actual timestep dependence here instead of postponing it. Let the contact normal $n$ point from body A toward body B, and let $r_A,r_B$ run from their centers of mass to their contact points. Their contact-point velocities are
$$
u_A=v_A+\omega_A\times r_A,\qquad
u_B=v_B+\omega_B\times r_B.
$$
The relative normal speed is $v_n=n\cdot(u_B-u_A)$. A negative result means the two contact points are closing along the chosen normal. A normal impulse magnitude $j\geq0$ applies $-jn$ to A and $+jn$ to B; this increases $v_n$ by $jk$, where
$$
k=m_A^{-1}+m_B^{-1}
 +(r_A\times n)^TI_A^{-1}(r_A\times n)
 +(r_B\times n)^TI_B^{-1}(r_B\times n).
$$
Here $I_A^{-1},I_B^{-1}$ are the *world-space* inverse inertia tensors; fixed bodies contribute zeros. This effective inverse mass is a scalar because we are resolving one normal direction. It is positive for a movable direction, and zero means the chosen impulse cannot change the normal speed: never divide by such a denominator.

For an isolated impact, restitution $e\in[0,1]$ prescribes a post-impact separating speed $v_n^{target}=-e\,v_n^{pre}$ when the pre-solve speed is below a specified negative impact threshold. Thus an unconstrained one-contact impulse is
$$
j=\max\!\left(0,\frac{v_n^{target}-v_n^{current}}{k}\right).
$$
When two nonrotating bodies each have mass $1\,\mathrm{kg}$, $k=2\,\mathrm{kg}^{-1}$. If they approach at $-3\,\mathrm{m/s}$ with $e=0.5$, the target is $+1.5\,\mathrm{m/s}$ and $j=(1.5-(-3))/2=2.25\,\mathrm{kg\,m/s}$. No $\Delta t$ appears: this is an instantaneous velocity jump. A physical impulse has units $\mathrm{N\,s}$, not $\mathrm N$; multiplying this impulse by $\Delta t$ a second time would be dimensionally wrong.

Real-time solvers also need to prevent already-overlapping bodies from continuing to sink into each other. This is a *different* goal from restitution. Suppose the measured penetration depth $d$ is positive in metres, the tolerated slop is $s$, and $h$ is the fixed simulation step in seconds. A Baumgarte-style velocity bias can request
$$
b(h)=\frac{\beta}{h}\max(d-s,0),
$$
where dimensionless $\beta\in[0,1]$ chooses a fraction of the excess penetration to correct each step. For $d=0.020\,\mathrm m,\ s=0.005\,\mathrm m,\ \beta=0.20$ and $h=1/60\,\mathrm s$, this gives $b=0.18\,\mathrm{m/s}$. Halving the step to $1/120\,\mathrm s$ gives $b=0.36\,\mathrm{m/s}$. The bias is a speed target, not an impulse; a solver converts its requested velocity change to an impulse using $k$. At one idealized step the requested corrective displacement $bh=\beta(d-s)=0.003\,\mathrm m$ is the same, but that does **not** make the complete simulation timestep-invariant. Contact detection, repeated iterations, gravity integration, changing geometry, clamping, warm-start history and impact thresholds all change how often and with what state the correction is applied.

```cpp
// A single normal-constraint calculation, not a complete stacked-contact solver.
// Preconditions: finite quantities; h > 0; effectiveInverseMass > 0; normal is unit.
const double closingSpeed = -3.0;       // m/s: negative means approaching.
const double restitution = 0.5;
const double effectiveInverseMass = 2.0; // 1/kg, including angular terms if any.
const double impactTarget = -restitution * closingSpeed; // +1.5 m/s.
const double impactImpulse =
    std::max(0.0, (impactTarget - closingSpeed) / effectiveInverseMass);
// impactImpulse == 2.25 kg*m/s.

// Position-stabilization target is calculated separately.
const double penetration = 0.020; // m
const double slop = 0.005;        // m
const double beta = 0.20;
const double h = 1.0 / 60.0;      // s
const double biasSpeed = beta * std::max(0.0, penetration - slop) / h;
// biasSpeed == 0.18 m/s. Do not blindly add it to every restitution target.
```

In a *sequential impulse* solver, each contact keeps an **accumulated normal impulse** $\lambda_n\geq0$, not a fresh isolated impulse every iteration. If an iteration proposes $\Delta\lambda=(v_{target}-v_n)/k$, clamp the **total** $\lambda_{new}=\max(0,\lambda_{old}+\Delta\lambda)$, then apply only $\lambda_{new}-\lambda_{old}$. Clamping each increment independently can forbid necessary impulse reductions and prevent convergence. Warm starting reapplies last frame's accumulated impulses only after verifying that the cached contact still represents the same valid physical features and chosen timestep policy.

Penetration bias and restitution are deliberately not collapsed into one unexplained scalar. Adding both positive targets can inject artificial energy, especially in resting stacks or when collisions persist. Common approaches include position-level correction, split impulses that change positional state without injecting the same kinetic velocity, or velocity targets whose impact and resting-contact branches are explicitly separated. None is universally best. A robust implementation chooses and tests a policy for impact thresholds, slop, iteration count, contact persistence, friction coupling and fast motion. For example, a cube resting on a plane should not repeatedly bounce because the bias velocity is mistaken for a new impact; two separating bodies should not receive a new closing-contact impulse simply because the cached manifold still exists.

The timestep now has three independently checkable roles: it determines when forces change momentum, it enters the chosen numerical integrator, and it can scale an *algorithmic* contact-stabilization target. A fixed-step accumulator also decides whether enough substeps were performed to cover elapsed wall time. If it drops excess accumulated time under overload, simulation time no longer matches wall time, even though every executed physics step used the same $h$. To verify the engine behavior, log accepted wall delta, number of substeps, contact depth, pre-impact $v_n$, $k$, restitution target, bias target, accumulated impulse and final post-solve $v_n$ separately. An unexplained single "impulse value" is not adequate debugging evidence.

---

### Kairo's fixed-step overload behavior deserves one subtle note

`RuntimePhysicsBridge` first clamps frame delta:

$$
\Delta t_{accepted}
=
\min(
\Delta t_{wall},
MaximumFrameDeltaSeconds).
$$

Then applies at most:

$$
MaximumSubsteps.
$$

Then if still behind:

$$
A\leftarrow A\bmod h.
$$

Therefore there are **two places wall time can be discarded**:

first through maximum frame delta,

then through maximum substeps.

This is intentional.

When debugging a "simulation fell behind real time" event, `DroppedExcessTime` tells you about the second condition, but a frame delta larger than the clamp also loses time even if the substep cap is not separately flagged.

A future diagnostics struct might expose both:

```text
ClampedFrameDelta
DroppedSubstepBacklog.
```

That would make overload instrumentation clearer.

---

### Fixed-step accumulator precision

Current bridge stores:

```cpp
float m_Accumulator;
```

For ordinary real-time sessions this is probably fine because accumulator is kept below roughly one fixed step most of the time.

For very long-running simulations or precision-sensitive tooling, one might use:

```cpp
double
```

for wall-clock accumulator while keeping physics state float.

Because the accumulator is timekeeping, not solver state.

This is a low-cost robustness improvement.

---

### Floating accumulator comparison

The loop uses:

```cpp
while (
    accumulator >= fixedDt)
```

with floating values.

Repeated subtraction can produce tiny residual error.

That is normally manageable because the accumulator stays bounded.

An alternative is integer simulation ticks:

$$
tick\in\mathbb N
$$

with high-resolution integer time units.

But for a game runtime the current approach is conventional.

---

### Previous/current snapshots with zero physics steps

Suppose render frame is shorter than h.

No physics step runs.

Then:

```text
Previous
Current
```

remain unchanged.

Accumulator grows.

Alpha increases.

Renderer interpolates further toward Current.

Correct.

This produces smooth apparent motion even without a new physics update on every render frame.

---

### Multiple substeps and pose history

Suppose three fixed steps run in one frame.

For each step Kairo does:

```cpp
Previous = Current;
Step();
Current = NewPhysicsPose;
```

After three steps:

```text
Previous = state after step 2
Current  = state after step 3.
```

Exactly what interpolation needs.

You do **not** want:

```text
Previous = pose at beginning of render frame
Current = pose after three physics steps
```

because then interpolation would span three physics intervals and create presentation lag/distortion.

Kairo handles this correctly.

---

### Scene transform publication and scale

Physics current pose updates only:

```text
Translation
Rotation.
```

The existing scene transform scale remains.

That is appropriate because rigid-body physics does not continuously integrate scale.

Scale affects collider creation/mass properties at setup time.

A runtime non-uniform scale change would require collider/mass reconstruction rather than simply treating scale as another dynamic degree of freedom.

---

### Kairo collider scaling semantics

Runtime bridge builds shapes from absolute world scale.

For a box:

$$
HalfExtents'
=
HalfExtents\odot|Scale|.
$$

Sphere radius uses maximum absolute axis scale.

Capsule uses radial max of x/z and y for half-height.

These are practical authored-shape conversion conventions.

But if a parent scale changes after runtime body creation, the current body collider does not automatically become a new mass distribution unless bridge explicitly rebuilds it.

That belongs to runtime topology/property synchronization policy.

---

### Kinematic scene sync should account for scale changes separately

Position/rotation target synchronization can update motion state.

Scale changes are not just motion.

For a dynamic/kinematic collider, scale can change:

$$
geometry
$$

and therefore:

$$
collision bounds,
mass,
inertia.
$$

So a general scene→physics synchronization system should classify changes:

```text
pose-only change → kinematic target / teleport
shape scale change → rebuild collider/mass
material change → update material
filter change → update collision filtering.
```

Do not treat all transform changes identically.

---

### Explicit Euler still has educational value

Even if semi-implicit Euler is the game default, explicit Euler remains valuable because it makes numerical failure visible.

It is a baseline.

If a more advanced integrator is "better," you should compare it against something.

Kairo exposes both explicit and semi-implicit helpers for exactly that reason.

It also includes RK2/RK4 scalar helpers and Verlet-position helper as educational/reference primitives.

That separation between reference math and runtime solver is healthy.

---

### Why not simply use RK4 for everything?

RK4 is higher-order for smooth ODEs.

So why don't game physics engines just use RK4?

Because rigid-body game physics has:

```text
discontinuous collisions
inequality constraints
Coulomb friction
sleeping
clamps
contact manifold changes
impulses.
```

RK4 assumes evaluating a reasonably smooth derivative function at intermediate states.

Collision topology can change between those intermediate evaluations.

It would also require multiple expensive broadphase/narrowphase/force evaluations per step.

Higher-order ODE accuracy does not automatically translate into a better contact solver.

Semi-implicit Euler plus impulse constraints is therefore an excellent practical architecture.

---

### Verlet and position-based systems

Verlet:

$$
x_{n+1}
=
2x_n-x_{n-1}+a_nh^2.
$$

This stores previous position rather than explicit velocity.

It is attractive for:

```text
cloth
rope
particle constraints
position-based dynamics.
```

But Kairo's rigid-body solver already has explicit velocity because contact impulses naturally operate on momentum/velocity.

So semi-implicit velocity-based integration is the more natural rigid-body choice.

---

### Orientation exactness with constant angular velocity

For constant world $\omega$:

$$
q(t+h)
=
\exp\left(
\frac12\Omega h
\right)
q(t).
$$

This is the rotational counterpart to:

$$
x(t+h)
=
x(t)+vh
$$

for constant linear velocity.

The exponential-map method directly approximates/exacts this finite group motion.

That makes it especially attractive for Kairo's `AdvanceMotionState()`.

---

### Exponential map with changing angular velocity

Even exponential-map orientation integration is not magically exact if:

$$
\omega(t)
$$

changes significantly during the step.

You still need to choose which angular velocity represents the interval:

```text
old
new
midpoint.
```

Semi-implicit would typically use updated/new omega.

A midpoint estimate can improve accuracy.

So:

$$
\boxed{
\text{exact finite rotation for constant }\omega
\neq
\text{exact full rigid-body integration}.
}
$$

---

### Quaternion sign canonicalization

For deterministic serialization/replay, remember:

$$
q
$$

and:

$$
-q
$$

encode the same orientation but have different bytes.

If state hashes include raw quaternion bits, mathematically equivalent orientations can hash differently.

A deterministic physics serializer may canonicalize quaternion sign, for example requiring:

$$
w>0
$$

or a lexicographic fallback when $w=0$.

Whether Kairo currently does this in state hashing should be checked if cross-implementation replay equivalence becomes important.

Within the same deterministic integration path, raw sign may remain stable, so this is more of a robustness/reproducibility consideration.

---

### Sleeping state and snapshot restore

If a snapshot contains:

```text
Sleeping = true
SleepTimer = 0.4
```

restoring only pose/velocity would change future behavior.

The body might remain awake another half second in one run but asleep immediately in another.

Thus sleep state is legitimate deterministic simulation state.

Kairo validates snapshot sleep timer and body state, which is exactly the right perspective.

---

### Accumulated force in snapshots

Whether force accumulator must be included depends on snapshot timing.

If snapshots are always taken **after Step clears forces**, then accumulators should be zero.

But Kairo exposes snapshot capture more generally, so retaining force/torque gives more faithful restoration.

Same reasoning applies to pending contacts and warm-start caches.

Snapshot semantics should state at which simulation phase they are valid.

---

### Warm-start state and deterministic restore

Sequential impulse solvers often cache previous contact impulses.

Next frame starts with those impulses applied.

If you restore body positions/velocities but discard warm-start cache, the next step may differ.

Therefore deterministic replay snapshots may need contact cache state too.

Current Kairo topology transaction comments explicitly care about preserving contact/warm-start state for surviving bodies, which shows the architecture recognizes this issue.

---

### Interpolation alpha after dropping time

Kairo uses:

$$
A\leftarrow A\bmod h
$$

after overload.

Then:

$$
\alpha=A/h.
$$

That means interpolation remains valid:

$$
0\le\alpha<1.
$$

If it simply left:

$$
A>h
$$

after dropping steps, alpha would exceed one, turning interpolation into unintended extrapolation.

So modulo remainder is the correct presentation companion to dropping full fixed intervals.

---

### Scene synchronization needs clear authority tags

A robust bridge can explicitly classify:

```cpp
enum class TransformAuthority
{
    Scene,
    Physics
};
```

Then:

```text
Static      → Scene authority, copied at creation
Kinematic   → Scene/gameplay authority
Dynamic     → Physics authority
```

This makes the direction obvious.

Without authority, you risk the classic loop:

```text
scene writes physics
physics writes scene
scene writes old interpolation back into physics
```

which causes jitter and nondeterminism.

---

### Suggested Kairo scene-sync architecture

```cpp
void BeforeFixedPhysicsStep(float dt)
{
    for (Entity entity : kinematicEntities)
    {
        const Transformf target =
            scene.WorldTransform(entity);

        physics.SetKinematicTarget(
            BodyFor(entity),
            target,
            dt);
    }

    ApplyContinuousGameplayForces(dt);
    ConsumePendingPhysicsImpulses();
}

void AfterFixedPhysicsStep()
{
    // Do not write interpolation here.
    // Capture exact current physics snapshots only.
    CaptureCurrentDynamicPoses();
}

void PublishRenderTransforms(float alpha)
{
    // Presentation-only.
    for (Entity entity : dynamicBodies)
    {
        scene.SetPresentationTransform(
            Interpolate(
                previousPhysicsPose,
                currentPhysicsPose,
                alpha));
    }
}
```

The most important separation is:

$$
\boxed{
\text{fixed simulation synchronization}
\neq
\text{render presentation synchronization}.
}
$$

---

### Current Kairo architecture already nearly matches this

Its `Advance()` does:

```text
Previous = Current
BeforePhysicsStep listener
World.Step
SynchronizeCurrentPoses
...
PublishInterpolatedPoses
```

That is a very solid structure.

The missing universal piece is primarily:

```text
generic automatic kinematic target extraction from scene before each substep.
```

The hook is already in the correct place to add it.

---

### KAIRO PASS 17 AUDIT SUMMARY

| Area | Current Kairo |
|---|---|
| Linear impulse | Implemented correctly |
| $\Delta v=m^{-1}J$ | Implemented correctly |
| Angular impulse | Implemented correctly |
| Off-center impulse $r\times J$ | Implemented correctly |
| Center-of-mass impulse | Explicit runtime API |
| Force vs impulse distinction | Strongly separated |
| Continuous torque | Implemented through accumulator |
| Runtime body state | Coherent `RigidBody` + `MotionState` |
| Position | `Vec3f`, effectively COM convention |
| Orientation | normalized quaternion |
| Linear velocity | world-space |
| Angular velocity | world-space |
| Force/torque accumulators | Implemented, cleared after Step |
| Sleeping state | Implemented |
| Wake on force/impulse/contact | Implemented |
| Static body | Implemented |
| Kinematic body | Implemented |
| Dynamic body | Implemented |
| Explicit Euler helper | Implemented |
| Semi-implicit Euler helper | Implemented |
| Runtime semi-implicit ordering | Implemented across world phases |
| Quaternion derivative integration | Implemented |
| Renormalization | Implemented every update |
| Exponential-map orientation | Not current default |
| Fixed timestep | Implemented |
| Variable frame accumulator | Implemented |
| Multiple substeps | Implemented |
| Maximum substeps | Implemented |
| Maximum frame-delta clamp | Implemented |
| Backlog dropping | Implemented |
| Previous/current pose history | Implemented |
| Render interpolation | Implemented |
| Physics→scene hierarchy sync | Implemented parent-first |
| Snapshot restore pose-history reset | Implemented |
| Scene→kinematic automatic target sync | Extension point rather than complete generic pass |
| Deterministic replay/hash | Implemented |
| Full gyroscopic rotational dynamics | Still simplified as noted in Pass 16 |

---

### Reconstruction challenge — impulse

A body has:

$$
m=4kg,
\qquad
I^{-1}
=
diag(1,0.5,0.25).
$$

Impulse:

$$
J=(0,8,0)
$$

applied at:

$$
r=(2,0,0).
$$

Linear change:

$$
m^{-1}=0.25.
$$

Therefore:

$$
\Delta v
=
(0,2,0).
$$

Angular impulse:

$$
r\times J
=
(2,0,0)
\times
(0,8,0)
=
(0,0,16).
$$

Angular velocity change:

$$
\Delta\omega
=
diag(1,0.5,0.25)
(0,0,16)
$$

$$
=
(0,0,4).
$$

That entire operation should now take seconds to derive mentally.

---

### Reconstruction challenge — force versus impulse

Suppose same body receives:

$$
F=(0,8,0)N
$$

for:

$$
dt=0.25s.
$$

Equivalent impulse:

$$
J=Fdt
=
(0,2,0)N\,s.
$$

Velocity change:

$$
\Delta v
=
J/m
=
(0,0.5,0).
$$

If instead someone says:

```text
apply impulse (0,8,0)
```

the change is:

$$
(0,2,0).
$$

Four times larger.

That is why dimensions matter.

---

### Reconstruction challenge — body ownership

For a static wall:

$$
\text{scene/world geometry owns motion}.
$$

For a moving platform:

$$
\text{gameplay/scene owns motion}
\rightarrow
\text{kinematic physics}.
$$

For a falling crate:

$$
\text{physics owns motion}
\rightarrow
\text{scene presentation}.
$$

If you cannot identify motion authority, transform synchronization becomes unstable.

---

### Reconstruction challenge — semi-implicit Euler

Given:

$$
x_0=0,
\quad
v_0=1,
\quad
a=2,
\quad
dt=0.5.
$$

First update velocity:

$$
v_1
=
1+2(0.5)
=
2.
$$

Then position:

$$
x_1
=
0+2(0.5)
=
1.
$$

Explicit Euler would have used old velocity and produced:

$$
x_1=0.5.
$$

That one update-order difference defines the two schemes.

---

### Reconstruction challenge — quaternion derivative

World angular velocity:

$$
\omega=(0,1,0).
$$

Create:

$$
\Omega=(0,1,0,0).
$$

Then under Kairo's convention:

$$
\boxed{
\dot q
=
\frac12\Omega q.
}
$$

Euler update:

$$
q'
=
Normalize(
q+\dot qdt
).
$$

Exact constant-$\omega$ update:

$$
\Delta q
=
\left(
\hat\omega
\sin\frac{|\omega|dt}{2},
\cos\frac{|\omega|dt}{2}
\right),
$$

then:

$$
q'
=
\Delta q q.
$$

You should know both.

---

### Reconstruction challenge — accumulator

Let:

$$
h=10ms
$$

and frame delta:

$$
27ms.
$$

Accumulator begins zero.

Add:

$$
27.
$$

Step once:

$$
17.
$$

Step again:

$$
7.
$$

Stop.

Physics advanced:

$$
20ms.
$$

Remainder:

$$
7ms.
$$

Interpolation alpha:

$$
\alpha
=
7/10
=
0.7.
$$

Renderer displays:

$$
70\%
$$

between previous and current exact physics snapshots.

---

### Reconstruction challenge — spiral of death

If each:

$$
16.7ms
$$

physics step costs:

$$
20ms
$$

to execute, then simulating one real second takes approximately:

$$
60\times20ms=1.2s.
$$

The engine cannot catch up.

Backlog grows.

Maximum-substep clamp does not make physics faster.

It simply bounds the damage by deciding what simulation time to discard.

That is the exact meaning of spiral-of-death protection.

---

### Reconstruction challenge — interpolation versus simulation

Suppose:

$$
Previous=0,
\quad
Current=10,
\quad
\alpha=0.25.
$$

Rendered:

$$
2.5.
$$

But actual exact physics state is still:

$$
Current=10.
$$

Do not feed:

$$
2.5
$$

back into physics.

It exists only for presentation.

That separation is non-negotiable.

---

### Reconstruction challenge — kinematic velocity

A kinematic platform moves from:

$$
x_0=(0,0,0)
$$

to:

$$
x_1=(0,0.1,0)
$$

during:

$$
dt=1/60.
$$

Then:

$$
v
=
\frac{0.1}{1/60}
=
6m/s.
$$

If you set its position to 0.1 but keep:

$$
v=0,
$$

a passenger/contact solver does not see the correct moving-surface velocity.

This is why transform synchronization needs velocity derivation, not just pose copying.

---


| Part | Topic | Status |
|---:|---|---|
| 1643 | Linear impulse | ✓ |
| 1644 | Linear velocity change from impulse | ✓ |
| 1645 | Angular impulse | ✓ |
| 1646 | Angular velocity change from off-center impulse | ✓ |
| 1647 | Applying impulse at world-space point | ✓ |
| 1648 | Applying impulse at center of mass | ✓ |
| 1649 | Force versus impulse | ✓ |
| 1650 | Torque impulse versus continuous torque | ✓ |
| 1651 | Rigid-body state structure in memory | ✓ |
| 1652 | Position representation | ✓ |
| 1653 | Orientation quaternion representation | ✓ |
| 1654 | Linear-velocity representation | ✓ |
| 1655 | Angular-velocity representation | ✓ |
| 1656 | Force and torque accumulators | ✓ |
| 1657 | Sleeping flags and activity state | ✓ |
| 1658 | Static/kinematic/dynamic body types | ✓ |
| 1659 | Kinematic-body semantics | ✓ |
| 1660 | Dynamic-body semantics | ✓ |
| 1661 | Static-body semantics | ✓ |
| 1662 | Explicit Euler integration | ✓ |
| 1663 | Semi-implicit Euler integration | ✓ |
| 1664 | Why semi-implicit Euler is preferred | ✓ |
| 1665 | Position integration | ✓ |
| 1666 | Linear-velocity integration | ✓ |
| 1667 | Quaternion derivative from angular velocity | ✓ |
| 1668 | Quaternion orientation integration | ✓ |
| 1669 | Orientation renormalization | ✓ |
| 1670 | Exponential-map rotation integration | ✓ |
| 1671 | Fixed timestep | ✓ |
| 1672 | Variable timestep instability | ✓ |
| 1673 | Accumulator-based stepping | ✓ |
| 1674 | Multiple physics substeps | ✓ |
| 1675 | Maximum-substep budgets | ✓ |
| 1676 | Spiral of death | ✓ |
| 1677 | Dropping excessive accumulated time | ✓ |
| 1678 | Previous/current physics snapshots | ✓ |
| 1679 | Render interpolation | ✓ |
| 1680 | Physics→scene synchronization | ✓ |
| 1681 | Scene→kinematic synchronization | ✓ |


The core equation chain for this pass is now:

$$
\boxed{
J
=
\Delta p
}
$$

therefore:

$$
\boxed{
\Delta v
=
m^{-1}J
}
$$

and for an off-center impulse:

$$
\boxed{
\Delta\omega
=
I_w^{-1}
(r\times J).
}
$$

Continuous motion then advances through:

$$
\boxed{
v_{n+1}
=
v_n+a_nh
}
$$

followed by:

$$
\boxed{
x_{n+1}
=
x_n+v_{n+1}h,
}
$$

with orientation:

$$
\boxed{
\dot q
=
\frac12\Omega q
}
$$

or preferably, for a finite constant-$\omega$ step:

$$
\boxed{
q_{n+1}
=
\exp\left(
\frac12\Omega h
\right)q_n.
}
$$

The runtime-time architecture is:

$$
\boxed{
\text{wall time}
\rightarrow
\text{accumulator}
\rightarrow
\text{fixed substeps}
\rightarrow
\text{previous/current exact poses}
\rightarrow
\text{interpolated presentation}.
}
$$

And the authority rule is:

$$
\boxed{
Dynamic:
Physics\rightarrow Scene
}
$$

while:

$$
\boxed{
Kinematic:
Scene/Game\rightarrow Physics.
}
$$

Kairo's implementation is already particularly strong in this pass: **immediate linear/angular impulse response, sleeping-aware wake ordering, clear static/kinematic/dynamic semantics, semi-implicit runtime ordering, normalized quaternion integration, bounded fixed stepping, frame-delta clamping, explicit overload dropping, deterministic replay hashes, previous/current pose history, interpolation, teleport-history collapse, and parent-first world→local scene publication are all present.**

The two main technical upgrades exposed by M100 are **exponential-map orientation integration** for better finite angular updates and a **first-class automatic kinematic-target synchronization path** that derives both linear and angular velocities from scene target transforms before every fixed substep.

**Pass 18 is the final pass: M101 + M102, Parts 1682–1700.** It closes Phase 17 with force-clearing timing, impulse timing, speed clamps, gravity/damping tuning, translational and rotational energy, linear/angular momentum conservation checks, integration drift, deterministic physics, unit/scale policy, COM/inertia debugging, mass-property tests, impulse-response tests, and finally a complete standalone `RigidBody` integrator whose every state update we derive and prove mathematically.


### Force/Impulse Timing, Velocity Limits, Gravity and Damping Policy, Energy and Momentum Diagnostics, Integration Drift, Determinism, Units and Scale, COM/Inertia Debugging, Physics Tests, and a Fully Derived Standalone RigidBody Integrator

This is the **final pass of the 18-pass sequence** and closes **Phase 17 — Rigid-Body Dynamics: Mass, Inertia, Forces, Motion, and Integration**.


Passes 16 and 17 gave us the core mechanics:

$$
\mathbf F=\dot{\mathbf p},
\qquad
\boldsymbol\tau=\dot{\mathbf L},
$$

$$
\mathbf p=m\mathbf v,
\qquad
\mathbf L=I\boldsymbol\omega,
$$

$$
\Delta \mathbf v=m^{-1}\mathbf J,
$$

$$
\Delta\boldsymbol\omega
=
I_w^{-1}
(\mathbf r\times\mathbf J),
$$

and finally the discrete update:

$$
\mathbf v_{n+1}
=
\mathbf v_n+\mathbf a_nh,
$$

$$
\mathbf x_{n+1}
=
\mathbf x_n+\mathbf v_{n+1}h.
$$

Pass 18 is where we stop merely making bodies move and start asking whether the simulator is **numerically defensible**.

The central questions are now:

$$
\boxed{
\text{When exactly are forces consumed?}
}
$$

$$
\boxed{
\text{Which engine conveniences secretly violate physics?}
}
$$

$$
\boxed{
\text{How do we detect conservation failure and numerical drift?}
}
$$

and finally:

$$
\boxed{
\text{Can we derive every state update in one RigidBody integrator from first principles?}
}
$$

---

### Part 1682 — Force clearing after a step

A force accumulator represents continuous influences queued for the **current integration interval**.

If:

$$
\mathbf F_{acc}
=
\mathbf F_1+\mathbf F_2+\cdots+\mathbf F_n,
$$

then during a fixed step of duration $h$:

$$
\Delta\mathbf v
=
m^{-1}\mathbf F_{acc}h.
$$

After that interval has been consumed, those forces must not automatically survive into the next step.

Therefore the canonical lifecycle is:

$$
\boxed{
\text{accumulate}
\rightarrow
\text{integrate}
\rightarrow
\text{clear}.
}
$$

Kairo follows exactly that model. At the end of `PhysicsWorld::Step()` it calls:

```cpp
ClearForceAccumulators();
```

and the implementation simply walks every body and invokes:

```cpp
ClearForces(body.Forces);
```

so both force and torque become zero.

This means an engine system implementing continuous thrust must deliberately reapply it on every fixed step.

That is correct.

Suppose a thruster should produce:

$$
100N
$$

for one second at 60 Hz.

You want:

$$
60
$$

applications of:

$$
100N
$$

each integrated over:

$$
h=\frac1{60}.
$$

Total impulse:

$$
J
=
\sum_{k=1}^{60}
100\frac1{60}
=
100N\,s.
$$

If the force accumulator remained uncleared, one call would accidentally create permanent thrust.

---

### Why force clearing belongs after the complete step

You must not clear immediately after velocity integration if later stages still need to know whether an external force was applied.

Kairo's current ordering is subtly good here.

The broad order is:

$$
\text{IntegrateForces}
\rightarrow
\text{collision/solver}
\rightarrow
\text{pose integration}
\rightarrow
\text{UpdateSleeping}
\rightarrow
\text{ClearForceAccumulators}.
$$

`UpdateSleeping()` checks:

```cpp
if (HasAccumulatedForce(body))
{
    WakeRigidBody(body);
    continue;
}
```

before the force accumulator is cleared.

Therefore a body that received an external force during the current step cannot immediately decide:

> My final velocity is small, therefore I should sleep.

The system remembers that an active external influence occurred until sleeping policy has been evaluated.

Only then is the force forgotten.

This is an excellent example of seemingly tiny update ordering carrying semantic meaning.

---

### Forces are ephemeral inputs, not durable state

There is still one caveat.

A full physics snapshot taken **mid-step**, between force submission and force consumption, may need to store the force accumulator if exact restore semantics are expected.

Kairo's snapshot stores complete body records, so the force accumulator is naturally part of captured body state.

That is safer than assuming snapshots always happen after the step.

The general rule is:

$$
\boxed{
\text{Any transient value capable of changing future evolution belongs to deterministic state at the instant it is captured.}
}
$$

---

### Part 1683 — Impulse application timing

Impulses are fundamentally different.

A force describes:

$$
\frac{d\mathbf p}{dt}.
$$

An impulse describes:

$$
\Delta\mathbf p.
$$

Therefore:

$$
\boxed{
\text{an impulse is normally applied immediately to velocity.}
}
$$

If:

$$
\mathbf J
$$

is received, update:

$$
\mathbf v
\leftarrow
\mathbf v+m^{-1}\mathbf J.
$$

For an off-center impulse:

$$
\boldsymbol\omega
\leftarrow
\boldsymbol\omega+
I^{-1}
(\mathbf r\times\mathbf J).
$$

No force accumulator.

No multiplication by dt.

No later "integration" stage is necessary for the impulse itself.

Kairo correctly makes `ApplyBodyImpulseAtPoint()` mutate `MotionState` immediately.

---

### Timing within a fixed step

Timing still matters.

Suppose gameplay jump impulse occurs **before** contact solving.

Then the contact solver sees the new upward velocity.

Suppose it occurs **after** contact solving.

Then the collision solver did not know about that momentum change until the next step.

Both can be legitimate depending on architecture, but they are not numerically identical.

A clean fixed update usually has something conceptually like:

$$
\text{external commands/impulses}
\rightarrow
\text{forces}
\rightarrow
\text{collision solver}
\rightarrow
\text{pose integration}.
$$

That gives the constraint solver the most current velocity state.

---

### Contact impulses are applied during solving

Collision impulses are naturally applied during the solver because the solver repeatedly evaluates and changes velocities.

A sequential impulse solver might do:

$$
J_1
\rightarrow
v'
$$

then use $v'$ while solving:

$$
J_2.
$$

This is the Gauss-Seidel property we'll enter in Phase 18.

So "impulse timing" is not merely API design.

The order of individual constraint impulses can alter the iterative solution.

---

### Applying an impulse after `Step()`

Suppose:

```cpp
world.Step(h);
world.ApplyBodyImpulseAtPoint(...);
```

The body velocity changes immediately, but its position for the already-completed step obviously cannot change retroactively.

The impulse affects the next pose advance.

That is mathematically consistent:

$$
v(t^+)=v(t^-)+\Delta v
$$

while:

$$
x(t^+)=x(t^-).
$$

Impulses cause discontinuities in velocity, not position.

This is an important physical principle.

---

### Part 1684 — Velocity clamping and why it can be dangerous

A velocity clamp looks harmless.

Suppose:

$$
\|\mathbf v\|>v_{max}.
$$

Then:

$$
\boxed{
\mathbf v
\leftarrow
\mathbf v
\frac{v_{max}}{\|\mathbf v\|}.
}
$$

Kairo currently does precisely this for linear and angular velocity.

The helper preserves direction while limiting magnitude.

That can be valuable for catastrophic-state containment.

But it is **not physical**.

---

### Clamping destroys momentum

Momentum:

$$
\mathbf p=m\mathbf v.
$$

Before clamp:

$$
p_{old}=m\|\mathbf v\|.
$$

After:

$$
p_{new}=mv_{max}.
$$

If:

$$
\|\mathbf v\|>v_{max},
$$

then:

$$
p_{new}<p_{old}.
$$

Momentum simply vanished.

No external impulse caused that change.

Therefore:

$$
\boxed{
\text{velocity clamping violates momentum conservation.}
}
$$

---

### Clamping destroys kinetic energy even more strongly

Translational kinetic energy:

$$
T=
\frac12mv^2.
$$

If:

$$
v=1000
$$

and:

$$
v_{max}=100,
$$

energy becomes:

$$
\frac{100^2}{1000^2}
=
\frac1{100}
$$

of its previous value.

So 99% of kinetic energy disappears instantaneously.

A speed clamp is effectively an artificial, nonlinear energy sink.

---

### Why clamps still exist

Because simulation safety sometimes matters more than perfect physics.

A NaN-adjacent explosion could generate:

$$
v=10^8m/s.
$$

That body may then:

```text
break broadphase assumptions,
overflow world coordinates,
destroy CCD budgets,
create absurd solver impulses,
contaminate neighbouring bodies.
```

A clamp can stop one bad state from cascading through the simulation.

So the right attitude is:

$$
\boxed{
\text{Clamp = emergency numerical policy, not physical law.}
}
$$

---

### Clamps can hide bugs

Suppose an unstable constraint injects energy every frame.

Without clamp:

$$
v:
1,2,4,8,16,\ldots
$$

and the bug is obvious.

With:

$$
v_{max}=10,
$$

you see:

$$
1,2,4,8,10,10,10.
$$

The engine looks "stable."

But the solver is still wrong.

Therefore development builds should instrument clamp activation.

For example:

```cpp
struct VelocityClampStats
{
    std::uint64_t LinearClampCount = 0;
    std::uint64_t AngularClampCount = 0;

    float LargestPreClampLinearSpeed = 0.0f;
    float LargestPreClampAngularSpeed = 0.0f;
};
```

A clamp hit should be observable.

Otherwise it becomes a physics-error concealment mechanism.

---

### Current Kairo clamp placement

Kairo applies damping and then clamps during `ApplyVelocityControls()`:

```cpp
if (body.LinearDamping > 0.0f)
{
    body.State.LinearVelocity *=
        std::exp(
            -body.LinearDamping * dt);
}

if (body.AngularDamping > 0.0f)
{
    body.State.AngularVelocity *=
        std::exp(
            -body.AngularDamping * dt);
}

ClampVelocity(
    body.State.LinearVelocity,
    body.MaxLinearSpeed);

ClampVelocity(
    body.State.AngularVelocity,
    body.MaxAngularSpeed);
```

For dynamic bodies this happens **after force integration but before the contact solver**.

That means force-generated excessive speed is bounded before solving.

But solver impulses can subsequently push velocity beyond the maximum because the clamp is not repeated after every solver impulse.

That is not inherently wrong; in fact repeatedly clamping inside the solver could badly damage convergence.

It does mean `MaxLinearSpeed` is not necessarily a hard invariant at every instant of the simulation step.

It is an integration-phase policy.

That distinction should be documented.

---

### Part 1685 — Maximum angular speed and numerical concerns

Angular velocity deserves special attention because orientation integration error depends strongly on:

$$
\boxed{
\theta_{step}
=
\|\omega\|h.
}
$$

This is the rotation performed in one integration step.

Suppose:

$$
h=\frac1{60}
$$

and:

$$
\omega=1000rad/s.
$$

Then:

$$
\theta
\approx16.67rad.
$$

That is:

$$
\frac{16.67}{2\pi}
\approx2.65
$$

full revolutions in **one physics step**.

A first-order quaternion integrator cannot possibly trace that motion accurately.

Even an exponential-map orientation update can represent the final orientation, but collision detection still has a problem: geometry may have swept through objects multiple times between discrete samples.

So maximum angular speed is not merely:

> We don't want visually crazy spinning.

It is a numerical and collision-detection policy.

---

### Rotational feature speed

For a point at radius r:

$$
v_{rot}
=
\omega r.
$$

If:

$$
r=2m
$$

and:

$$
\omega=100rad/s,
$$

surface speed is:

$$
200m/s.
$$

Even if COM translation is zero, collider vertices are moving extremely fast.

A translation-only CCD approximation can miss rotational tunnelling.

Therefore angular velocity interacts with collision robustness.

---

### Better criterion than an arbitrary $\omega_{max}$

A useful diagnostic is:

$$
\theta_{step}
=
\omega h.
$$

You might define:

$$
\theta_{max}=0.25rad
$$

or another empirical bound and then derive:

$$
\omega_{recommended}
=
\frac{\theta_{max}}{h}.
$$

For:

$$
h=1/60,
$$

that gives:

$$
15rad/s.
$$

That may be too restrictive for games, but it shows the actual relationship.

The clamp should ideally be informed by timestep and characteristic body radius, not chosen in isolation.

Kairo's default:

```cpp
MaxAngularSpeed = 1000.0f;
```

is therefore clearly a catastrophic-state ceiling, not a tight numerical-accuracy guarantee.

That is perfectly reasonable as long as we interpret it correctly.

---

### Part 1686 — Gravity scaling per body

Kairo's gravity path is:

$$
\boxed{
a_g
=
G
\cdot
S_{global}
\cdot
S_{body}.
}
$$

The code is essentially:

```cpp
const Vec3f acceleration =
    (
        body.EnableGravity
        ? Gravity *
          (
              Settings.GravityScale *
              body.GravityScale
          )
        : Vec3f::Zero()
    )
    +
    body.Forces.Force *
        body.Mass.InverseMass;
```

Default gravity is:

$$
\boxed{
(0,-9.80665,0)m/s^2
}
$$

which is standard gravity.

This is an excellent SI-based default.

---

### Why body gravity scale is not mass

A gravity scale of:

$$
0.5
$$

means:

$$
a_g=
0.5g.
$$

It does **not** mean the body has half its mass.

The body's collision impulse response still uses its real:

$$
m^{-1}
$$

and inertia.

Gravity scale is a gameplay/environment acceleration multiplier.

This means two bodies of identical mass but different gravity scale behave differently under gravity while colliding identically under external impulses.

That is an intentional nonphysical game rule.

---

### Negative gravity scale

Current validation requires gravity scale to be finite, but does not require it to be nonnegative.

Therefore:

$$
GravityScale=-1
$$

would reverse gravity.

That can be useful.

Likewise:

$$
0
$$

creates zero gravity.

This is a flexible policy.

---

### Gravity as acceleration preserves equivalence principle

Because Kairo adds gravity directly as acceleration:

$$
a_g=g,
$$

all finite dynamic bodies receive the same gravitational acceleration independent of mass.

That is exactly the ideal Newtonian near-surface behavior.

If you instead accidentally accumulated:

$$
F=g
$$

as though g were a force, lighter bodies would accelerate more.

Dimensional correctness prevents that mistake.

---

### Part 1687 — Linear damping implementation

Kairo uses:

$$
\boxed{
\mathbf v'
=
\mathbf v
e^{-\lambda_lh}.
}
$$

This is one of the strongest small design choices in the physics implementation.

Why?

Suppose continuous decay obeys:

$$
\dot v=-\lambda v.
$$

Separate variables:

$$
\frac{dv}{v}
=
-\lambda dt.
$$

Integrate:

$$
\ln v
=
-\lambda t+C.
$$

Therefore:

$$
v(t)=v_0e^{-\lambda t}.
$$

Thus Kairo's damping update is the **exact solution** of a first-order exponential decay model over one step.

---

### Half-life interpretation

Designers often understand half-life better than an abstract damping coefficient.

Want velocity to halve after time:

$$
t_{1/2}.
$$

Set:

$$
\frac12
=
e^{-\lambda t_{1/2}}.
$$

Take ln:

$$
-\ln2
=
-\lambda t_{1/2}.
$$

Therefore:

$$
\boxed{
\lambda
=
\frac{\ln2}{t_{1/2}}.
}
$$

If half-life:

$$
1s,
$$

then:

$$
\lambda\approx0.6931s^{-1}.
$$

If half-life:

$$
0.1s,
$$

then:

$$
\lambda\approx6.931.
$$

This is an excellent way to expose damping to tools.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/37 - Physical examples and state validation|Previous chapter]] · [[Chapter Edition/03 - Game Engine/39 - Composable timesteps and stable simulation policy|Next chapter]]
