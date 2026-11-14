# 40 — Coupled contacts and iterative response

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/39 - Composable timesteps and stable simulation policy|Previous chapter]] · [[Chapter Edition/03 - Game Engine/41 - Evidence, tools, and revisable design|Next chapter]]

One isolated collision impulse does not determine a whole stack because contacts constrain one another. This chapter builds a small coupled solver and shows how ordering, friction, and iteration affect the result.

**By the end:** Reconstruct the contact constraints and solve policy for a multi-contact system, stating where the reference witness stops.

**Recall question:** Why can resolving one contact change another contact’s penetration or friction condition, and how does another solver sweep respond?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 99261–100532; source-span SHA256 be8001578d5efcb293d236b3644df0a2f0dfcd434c936daa4a4cd1e32c127af3. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## From one collision impulse to a coupled contact problem — reconstructed Phase 18 bridge

One isolated impact can be solved with one scalar impulse. A resting crate immediately breaks that model: two contact points belong to one rigid body, and an impulse at either point changes both center velocity and angular velocity. Therefore it changes the normal speed at both points.

For contact $i$, let $\mathbf p_i$ be the world point and let unit normal $\mathbf n$ point from body $A$ toward body $B$. The lever arms are $\mathbf r_{Ai}=\mathbf p_i-\mathbf x_A$ and $\mathbf r_{Bi}=\mathbf p_i-\mathbf x_B$. Point velocity is $\mathbf v+\boldsymbol\omega\times\mathbf r$, so the pre-solve normal speed is

$$
u_i=\mathbf n^\mathsf T\left[(\mathbf v_B+\boldsymbol\omega_B\times\mathbf r_{Bi})-(\mathbf v_A+\boldsymbol\omega_A\times\mathbf r_{Ai})\right]
$$

Use SI units: positions are metres, linear velocity is metres/second, angular velocity is radians/second, mass is kilograms, inverse inertia is kilogram$^{-1}$ metre$^{-2}$, and impulse is newton-seconds. With the normal from $A$ to $B$, $u_i<0$ means closing. A normal impulse $\lambda_i\ge0$ pushes $B$ along $\mathbf n$ and applies the equal-and-opposite impulse to $A$; ordinary contact does not pull.

Let $u$ collect pre-solve speeds and $\lambda$ collect normal impulses. The contact-space effective-mass matrix maps impulses to speed changes:

$$
w=u+K\lambda,\qquad K=JM^{-1}J^\mathsf T.
$$

$J^\mathsf T\lambda$ maps contact impulses to generalized body impulses, $M^{-1}$ converts them to changes in linear and angular velocity, and $J$ projects those changes back to contact speeds. Thus $K_{ij}$ measures how impulse $j$ changes speed at contact $i$. For planar rigid bodies with a common normal,

$$
K_{ij}=m_A^{-1}+m_B^{-1}+I_A^{-1}(\mathbf r_{Ai}\times\mathbf n)(\mathbf r_{Aj}\times\mathbf n)+I_B^{-1}(\mathbf r_{Bi}\times\mathbf n)(\mathbf r_{Bj}\times\mathbf n).
$$

Off-diagonal terms express coupling. For frictionless zero-restitution contact, solve the complementarity conditions

$$
\lambda\ge0,\qquad w=u+K\lambda\ge0,\qquad \lambda_iw_i=0.
$$

A positive impulse requires zero residual closing speed; a separating point needs no impulse. For a unit-mass, unit-inertia crate descending at $2\,\mathrm{m/s}$ with contacts at $x=-1$ and $x=1$, $K=\begin{bmatrix}2&0\\0&2\end{bmatrix}$ and $u=(-2,-2)^\mathsf T$, giving $\lambda=(1,1)^\mathsf T\,\mathrm{N\,s}$. Moving the right contact to $x=2$ gives $K=\begin{bmatrix}2&-1\\-1&5\end{bmatrix}$ and $\lambda=(4/3,2/3)^\mathsf T\,\mathrm{N\,s}$. Sequential impulses approximate this coupled solution through repeated projected updates; a block solve handles a small set together.

The implementation checks all four two-contact active sets: both points, first only, second only, or neither. It accepts a candidate only when impulses are nonnegative and post-speeds satisfy complementarity within a scale-aware tolerance. A near-singular determinant skips the both-active inverse; a feasible one-point fallback may still exist for duplicate contact rows. If no candidate is feasible, the island is rejected before body mutation. This witness covers normal velocity response only; restitution targets, penetration stabilization, friction, warm-start cache lifetime, and larger islands are separate policies.

### Executable active-set witness with numeric and failure traces

The following C++20 program implements the two-point block and exercises symmetric, asymmetric, single-active, separating, rank-deficient, immovable, and invalid-normal cases. Its comments state the equations, units, branch rules, and invariants beside the operations they explain.

```cpp
/*
Teaching contract: this is a 2-D rigid-body velocity solve for two unilateral
normal contacts on one body pair. Positions are metres, linear speeds m/s,
angular speed rad/s, inverse mass kg^-1, inverse inertia kg^-1 m^-2, and each
normal impulse is kg m/s. The unit normal points from A toward B; a positive
relative normal speed means separation. The contact-point velocity is
v + omega x r, so rotating bodies can close at one point and separate at the
other even when their centers move together.

For contact i, let J_i be its normal-velocity row. The effective-mass block
K_ij = J_i M^-1 J_j^T maps impulse j into speed change at point i. We solve
w = u + K lambda >= 0, lambda >= 0, lambda_i*w_i = 0. The four active sets
(both, left, right, neither) are enumerated and checked against complementarity;
a singular both-active block falls through to feasible one-point cases. The
witness fails closed when no case exists and verifies the reconstructed post
speeds against the matrix equation. This is not friction, penetration bias,
warm starting, manifold reduction, or a production island solver.
*/
// C++20 two-point unilateral contact witness: 2x2 block complementarity, not a complete engine solver.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/two_point_contact_block_lab.cpp -o /tmp/two_point && /tmp/two_point
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

struct Vec2 { double x{}, y{}; };
constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
constexpr Vec2 operator*(Vec2 a, double k) { return {a.x * k, a.y * k}; }
constexpr double dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
constexpr double cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
constexpr Vec2 spin(double w, Vec2 r) { return {-w * r.y, w * r.x}; }

void require(bool ok, const char* what) {
    if (!ok) throw std::runtime_error(what);
}
void near(double got, double want, double tolerance = 1e-9) {
    require(std::isfinite(got) && std::abs(got - want) <= tolerance,
            "numerical check failed");
}
void finite(Vec2 a) { require(std::isfinite(a.x) && std::isfinite(a.y), "nonfinite point"); }

struct Body {
    Vec2 center{}, velocity{};
    double omega{}, inverseMass{}, inverseInertia{};
};
void validate(const Body& b) {
    finite(b.center); finite(b.velocity);
    require(std::isfinite(b.omega) && std::isfinite(b.inverseMass) &&
            std::isfinite(b.inverseInertia) && b.inverseMass >= 0.0 &&
            b.inverseInertia >= 0.0, "invalid body");
}
struct ContactPair { std::array<Vec2, 2> worldPoints{}; Vec2 normal{0, 1}; };
struct BlockResult {
    std::array<double, 2> impulses{}, postSpeeds{};
    int activeConstraints{};
};

Vec2 pointVelocity(const Body& b, Vec2 r) {
    return b.velocity + spin(b.omega, r);
}
void apply(Body& a, Body& b, Vec2 ra, Vec2 rb, Vec2 normal, double impulse) {
    const Vec2 p = normal * impulse;
    a.velocity = a.velocity - p * a.inverseMass;
    b.velocity = b.velocity + p * b.inverseMass;
    a.omega -= a.inverseInertia * cross(ra, p);
    b.omega += b.inverseInertia * cross(rb, p);
}

// A contact is unilateral: its impulse can push, but never pull.
// The two-point matrix captures how an impulse at either point affects BOTH gaps.
BlockResult solveBlock(Body& a, Body& b, ContactPair pair) {
    validate(a); validate(b); finite(pair.normal);
    const double normalLength = std::hypot(pair.normal.x, pair.normal.y);
    require(std::isfinite(normalLength) && normalLength > 0, "invalid normal");
    const Vec2 n = pair.normal * (1.0 / normalLength);
    std::array<Vec2, 2> ra{}, rb{};
    std::array<double, 2> leverA{}, leverB{}, before{};
    for (int i = 0; i < 2; ++i) {
        finite(pair.worldPoints[i]);
        ra[i] = pair.worldPoints[i] - a.center;
        rb[i] = pair.worldPoints[i] - b.center;
        leverA[i] = cross(ra[i], n);
        leverB[i] = cross(rb[i], n);
        before[i] = dot(pointVelocity(b, rb[i]) - pointVelocity(a, ra[i]), n);
        require(std::isfinite(before[i]), "nonfinite relative velocity");
    }
    // K maps each impulse to both normal speeds; off-diagonal terms are rotational coupling.
    double K[2][2]{};
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            K[i][j] = a.inverseMass + b.inverseMass +
                      a.inverseInertia * leverA[i] * leverA[j] +
                      b.inverseInertia * leverB[i] * leverB[j];
    for (auto& row : K)
        for (double value : row)
            require(std::isfinite(value), "effective mass overflow");

    // Solve w = before + K*lambda >= 0, lambda >= 0, lambda_i*w_i = 0.
    // Enumerate both-active, left-only, right-only and none, accepting only
    // complementarity-feasible cases. Zero determinant means no unique
    // both-active solution; a one-active solution may still exist.
    constexpr double eps = 1e-10;
    const double determinant = K[0][0] * K[1][1] - K[0][1] * K[1][0];
    const double matrixScale = std::max({1.0, std::abs(K[0][0] * K[1][1]),
                                          std::abs(K[0][1] * K[1][0])});
    auto feasible = [&](std::array<double, 2> lambda) {
        std::array<double, 2> gap{
            before[0] + K[0][0] * lambda[0] + K[0][1] * lambda[1],
            before[1] + K[1][0] * lambda[0] + K[1][1] * lambda[1]
        };
        const double scale = std::max({1.0, std::abs(before[0]), std::abs(before[1])});
        const double tolerance = eps * scale;
        for (int i = 0; i < 2; ++i) {
            if (!std::isfinite(lambda[i]) || !std::isfinite(gap[i]) ||
                lambda[i] < -tolerance || gap[i] < -tolerance ||
                (lambda[i] > tolerance && std::abs(gap[i]) > tolerance))
                return false;
        }
        return true;
    };
    std::array<double, 2> answer{};
    bool found = false;
    // Avoid determinant division when the block is singular or poorly conditioned.
    if (std::isfinite(determinant) && determinant > eps * matrixScale) {
        const std::array<double, 2> both{
            (-K[1][1] * before[0] + K[0][1] * before[1]) / determinant,
            ( K[1][0] * before[0] - K[0][0] * before[1]) / determinant
        };
        if (feasible(both)) { answer = both; found = true; }
    }
    // Rank-deficient pairs may still admit one active point; test its effect on both gaps.
    if (!found && K[0][0] > eps) {
        const std::array<double, 2> left{-before[0] / K[0][0], 0};
        if (feasible(left)) { answer = left; found = true; }
    }
    if (!found && K[1][1] > eps) {
        const std::array<double, 2> right{0, -before[1] / K[1][1]};
        if (feasible(right)) { answer = right; found = true; }
    }
    if (!found && feasible({0, 0})) found = true;
    require(found, "no valid contact impulse: degenerate or inconsistent pair");

    // Mutate bodies only after a feasible active set is found.
    for (int i = 0; i < 2; ++i) {
        // Tiny negative roundoff is projected to zero, not treated as attraction.
        answer[i] = std::max(0.0, answer[i]);
        apply(a, b, ra[i], rb[i], n, answer[i]);
    }
    validate(a); validate(b);
    BlockResult result;
    result.impulses = answer;
    for (int i = 0; i < 2; ++i) {
        result.postSpeeds[i] =
            dot(pointVelocity(b, rb[i]) - pointVelocity(a, ra[i]), n);
        near(result.postSpeeds[i],
             before[i] + K[i][0] * answer[0] + K[i][1] * answer[1], 1e-8);
        require(result.postSpeeds[i] >= -1e-8, "still closing after block solve");
        if (answer[i] > eps) ++result.activeConstraints;
    }
    return result;
}
int main() {
    const Body ground{{0, 0}, {0, 0}, 0, 0, 0};
    { // Symmetric crate: K = diag(2,2); a -2m/s fall needs 1N*s at each point.
        Body crate{{0, 1}, {0, -2}, 0, 1, 1};
        Body fixed = ground;
        const auto result = solveBlock(fixed, crate,
                                      ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 1); near(result.impulses[1], 1);
        near(crate.velocity.y, 0); near(crate.omega, 0);
        require(result.activeConstraints == 2, "both contacts should act");
    }
    { // Asymmetric footprint: K=[[2,-1],[-1,5]], impulses 4/3 and 2/3.
        Body fixed = ground, crate{{0, 1}, {0, -2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{2, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 4.0 / 3.0);
        near(result.impulses[1], 2.0 / 3.0);
        near(crate.velocity.y, 0); near(crate.omega, 0);
    }
    { // Single active point: positive separating speed at the second point.
        Body fixed = ground, crate{{0, 1}, {0, 0}, 1, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 0.5); near(result.impulses[1], 0.0);
    }
    { // Already separating: neither contact is allowed to pull the body down.
        Body fixed = ground, crate{{0, 1}, {0, 2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}});
        near(result.impulses[0], 0); near(result.impulses[1], 0);
    }
    { // Duplicate contact positions make K rank deficient; one point suffices.
        Body fixed = ground, crate{{0, 1}, {0, -2}, 0, 1, 1};
        const auto result = solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{-1, 0}}, Vec2{0, 1}});
        near(result.impulses[0] + result.impulses[1], 1);
        near(result.postSpeeds[0], 0); near(result.postSpeeds[1], 0);
    }
    { // Two immovable bodies cannot repair closing speed via impulse.
        Body fixed = ground, immovable{{0, 1}, {0, -1}, 0, 0, 0};
        bool rejected = false;
        try { (void)solveBlock(fixed, immovable, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 1}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "immovable closing geometry should be rejected");
    }
    { // Hostile inputs must fail before polluting another island.
        Body fixed = ground, crate{{0, 1}, {0, -1}, 0, 1, 1};
        bool rejected = false;
        try { (void)solveBlock(fixed, crate, ContactPair{{Vec2{-1, 0}, Vec2{1, 0}}, Vec2{0, 0}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "zero normal must be rejected");
    }
    std::cout << "two-point contact block: 7 suites PASS\n";
}
```

**Reconstruction check:** explain why the off-diagonal terms couple the contacts, list all four active sets, and say why a singular pair can still have a feasible one-contact solution.

### An animation library is one part of an authoring workflow

Suppose the panel is attached to an animated character. Evaluating a skeletal pose is the narrow numerical task: sample local joint transforms, combine them through the hierarchy, and use the resulting transforms to move mesh vertices. A working engine must also import animation assets, associate them with the right skeleton, expose them to an author, choose playback states, blend transitions and coordinate with physics. A **skeleton** is the hierarchy of joints; a **pose** assigns their transforms at a particular instant. **Skinning** uses joint influence weights to deform a mesh from those transforms.

Hazel's animation devlog describes first integrating a third-party skeletal animation library and later replacing that path with a custom implementation. It emphasizes importing, asset-system integration and editor workflow, rather than presenting the pose evaluator as the whole feature. The captions report good performance for the replacement. They do not say Hazel rejected it for poor performance; a sentence about considering its use in another project was incorrectly extracted that way. [Source devlog](https://www.youtube.com/watch?v=HPZo1oFESYQ)

To reason about the subsystem, trace one clip from disk to the visible character. A **clip** stores animation tracks over time. Importing converts external tracks and joint identifiers into the engine's representation. Playback chooses a clip time and a rule for looping or stopping. A local joint transform is measured relative to its parent, so hierarchy evaluation must use a consistent parent-before-child order. A clip whose joint ordering differs from the mesh's skin bindings needs an explicit remapping. Correct interpolation on the wrong joint is still the wrong animation.

State switching and blending solve different problems. Switching replaces one active state. Blending combines compatible pose contributions over an interval or according to a controller. It needs agreement on skeleton identity, spaces, rotation interpolation, missing tracks and the meaning of weights. An animation graph describes how states and blend operations are selected; it is more than a diagram of every clip name. A graph must have defined update order and a policy for transitions triggered during a frame.

Physics adds an authority question: which system controls each transform? A ragdoll uses linked physical bodies and constraints to drive a character. Mixing authored motion and simulated motion requires a stated authority/blending policy and a common timing convention. Simply running animation and physics in sequence can make one overwrite the other's results. The devlog mentions blending, inverse kinematics and physics coordination as work still to develop at that point; those mentions are roadmap evidence, not demonstrations of completed features.


#### Separate animation sampling, action state, and behavioral control

Before deciding who owns root motion, separate three architectural questions that are often collapsed into “the animation system.” Gregory describes a useful three-layer decomposition. The lowest **animation pipeline** turns clip data and blend specifications into a final skeletal pose and skinning palette. Above it, an **action state machine** represents animation-scale actions such as idle, locomotion, jump, attack, or reload and manages transitions. Above that, **animation controllers** coordinate behavior modes—combat, cover, climbing, vehicle use, and other game-specific situations—without forcing high-level player or AI logic to micromanage individual clips.

The low-level pipeline is itself a sequence with data dependencies. First, clip data is made usable for the requested time: compressed tracks may be decompressed and local joint transforms sampled. If several contributions are active, poses are blended according to their rules, including masks or additive layers where supported. The local hierarchy is then accumulated into global joint transforms. Procedural post-processing such as IK or ragdoll adjustment can modify the pose. If that processing changes local transforms after global transforms were already needed, the affected global chain must be recomputed. Finally, global joint transforms combine with inverse-bind transforms to produce the matrix palette or equivalent skinning representation consumed by rendering.

That order explains several bugs that a formula-only treatment misses. An IK solver cannot correctly place a hand against a world target if it is given stale global transforms. A post-process that changes a parent without updating descendants leaves the skeleton internally inconsistent. A renderer that consumes the previous palette while bounds were computed from the new pose can cull visible geometry. Compression and streaming belong before sampling because a clip that is not resident cannot be evaluated merely because a state machine selected it. Each stage therefore has both a mathematical contract and a data-lifetime contract.

The action-state layer solves a different problem: transitions are temporal rules, not pose arithmetic. “Run” may transition to “jump” only under particular gameplay conditions, a transition may have a blend duration, and an event may occur at a marker inside a clip. Multiple **state layers** can operate on different subsets of the skeleton—for example locomotion on the lower body while an upper-body layer aims and fires. That is not the same as blindly averaging two full-body poses. The layer must state which joints it controls, whether its contribution is override or additive, and how conflicting requests resolve.

The controller layer then maps game meaning to those animation actions. An AI system can request “take cover” or “fire while retreating” without selecting a particular clip frame. The controller may choose locomotion state, upper-body action, aim target, root-motion policy, and procedural corrections. This preserves a useful dependency direction: gameplay decides **intent**, the animation controller decides **presentation/motion strategy**, the pose pipeline evaluates **skeletal state**, and the authoritative movement system decides what world-space motion is actually permitted. The next section’s root-motion and physics contract sits exactly at that final hand-off.

Do not treat these three layers as mandatory class boundaries. An engine can merge them or represent them as graph nodes, data tables, or generated programs. The decomposition is conceptual: pose evaluation, action transition logic, and high-level behavior coordination have different inputs, state, failure modes, and testing strategies. Keeping those meanings distinct makes it possible to test a clip sampler without running combat AI, test a state transition without a GPU, and test a controller’s requested actions before solving final skeletal motion.

Source basis: Gregory, *Game Engine Architecture*, §§11.9–11.12 on animation system architecture, the animation pipeline, action state machines, state layers, and animation controllers.

#### Animation proposes motion; a single system commits it

A clip is a time-indexed set of local joint transforms, but the character *controller* must first select how time advances and which clips contribute. An idle-to-run blend may combine local translation, scale and rotation (quaternion interpolation uses normalized conventions and shortest-arc handling), but a blend between differently authored skeletons needs an explicit retargeting map, root convention and compatible reference pose. Sampling a clip at display rate and firing a footstep event whenever the sample crosses a marker is unsafe: one game tick may have two display evaluations, while a slow display frame can skip over several markers. Marker/event collection belongs to the authoritative animation advance at a defined tick boundary and must account for wrap-around, reverse/variable playback if supported and transition blends that could otherwise fire two copies of the same semantic footstep.

Root motion exposes an especially common double-application bug. Let an authored clip request displacement `Δx_root=0.5 m` over a tick. The character motor/collision stage determines that only `Δx_allowed=0.3 m` can be moved because of a wall. The gameplay-world transform advances by `0.3 m`. The final rendered skeleton is then placed relative to that **resolved** world transform; adding the clip's `0.5 m` to the world transform again would move `0.8 m`, penetrate the wall and disagree with physics. If a ragdoll owns a limb or the entire root, declare the blend/override boundary; do not let animation and physics independently write the same authoritative transform without a reconciliation rule. The root-motion track may still affect the local animated pose, but the corresponding world-space displacement is not a second movement command.

A practical tick contract reads: controller state `(active clips, normalized times, blend weights, transition state)` -> local-pose samples -> provisional blended pose and desired root displacement -> character controller/physics -> resolved world root and contacts -> final pose, inverse-bind matrices, bounds and animation events -> immutable render snapshot. CPU skinning and GPU skinning have different data lifetimes. If compute skinning writes a vertex buffer for drawing, producer/consumer barriers and frames-in-flight matter; the pose owning the previous render frame cannot be overwritten simply because the next CPU simulation tick has begun. Bounds must conservatively contain skinned geometry or culling can remove a correctly animated actor. Footstep audio and particles are dispatched from stable tick events, not from each camera render pass.

Consider the witness's deliberately narrow `Character`: it accepts one displacement request per tick, clamps against a supplied allowed displacement, records which tick consumed the request, and emits one footstep event. An extra draw evaluation of the same tick is ignored. This proves a **logical once-per-tick contract**, not production motion matching, IK, ragdoll blend, imported curve retargeting, skinning quality or audio scheduling. Those remain distinct practical implementation requirements. Gregory's Chapter 11 offers the production subsystem motivation; the current engine's richer math and graphics treatment supplies the mechanics, and this bridge states who gets to commit the answer.

#### Reconstruct the animation controller, not just the pose formula

The root-motion ownership rule above tells us **who** commits a character transform, but not how a controller produces the request. A controller is a small temporal program. Its state includes clip identity, clip-local time, whether an animation is looping, transition progress, event positions, blend weights and the last fixed tick on which it advanced. That state is not interchangeable with the current rendered skeleton. It must survive rendering zero or three times between consecutive fixed ticks, and it must not advance again simply because a camera asks for a different pose.

Take two compatible clips: `Idle`, whose root does not move, and `Walk`, whose author-time root track travels one meter during each one-second cycle. To evaluate a joint at time `t`, first locate the two keys bracketing the clip-local time `τ = t mod T`, where `T` is the positive clip duration. For translation keys `p0,p1` at times `t0,t1`, the local interpolation fraction is `a = (τ−t0)/(t1−t0)`, and the interpolated translation is `p=(1−a)p0+a p1`. This is only defined when keys are strictly increasing, and it is not a universal treatment of authored cubic splines: spline interpolation needs its own tangents and interpolation contract. For unit quaternions `q0,q1`, use shortest-arc spherical interpolation. Normalize the inputs, negate `q1` if `dot(q0,q1)<0`, and interpolate by `sin((1−a)θ)/sin θ` and `sin(aθ)/sin θ` with `θ=acos(dot(q0,q1))`. Near coincident rotations, normalized linear interpolation avoids division by a nearly zero sine. The quaternions `q` and `−q` describe the same spatial rotation even though their coefficients differ.

A transition from `Idle` to `Walk` lasting 0.20 seconds defines a blend `w(t)` from zero to one. The visual foot joint can be blended as `pose=(1−w)IdlePose+w WalkPose` for translations and by quaternion interpolation for rotations. This deliberately assumes compatible joint indices, rest pose, parent hierarchy and coordinate units. For different skeletons, joint-name equality is not enough: retargeting must resolve the rest-pose orientation, hierarchy, limb proportion, scale and root-bone conventions. A generic arithmetic average of rotation matrices would not preserve orthogonality. Additive animation layers and masks require separate reference poses and normalized composition policies.

The moving root needs more care than the visual foot. If the walk clip has unwrapped root displacement `r(t)` with units meters, its request over a tick is an integral of its velocity contribution. For a *linear* fade and constant walk velocity over the tick, `Δx_requested = [r(t1)−r(t0)]·(w0+w1)/2`, where `w0` and `w1` are blend weights at the beginning and end. This trapezoidal expression is **exact for this particular linear weight and constant velocity**, not an arbitrary approximation guaranteed exact for all curves. Starting the 0.20-second transition with a 0.10-second simulation tick gives `w0=0, w1=0.5`, and a 0.10-meter raw walk displacement; therefore the first tick requests `0.10·0.25=0.025 m`. The next tick has weights `0.5` and `1`, so it requests `0.10·0.75=0.075 m`. If collision permits only 0.05 m during that second tick, the authoritative root is `0.025+0.05=0.075 m`, not the unconstrained 0.10 m and certainly not 0.10 m plus another animation offset. A transition with nonlinear weighting requires numerical/analytic integration or deliberately documented approximation.

A looping pose resets its local time, but its **unwrapped** root displacement must not jump backward. For each completed cycle `n` and remainder `τ`, use `r(t)=nD+r_cycle(τ)` with per-cycle displacement `D` (one meter in the example). A footstep marker is also not a property of the render frame: it is a time-crossing query. Record a marker when its occurrence time `nT+m` lies in the half-open interval `(previousTickTime,currentTickTime]`, where `m` is the marker position within the cycle. This rule prevents duplicates at an exactly shared boundary and catches events when one tick crosses the end of a loop. For a backward-playback or seek operation, choose a separate, explicit event policy; do not reuse the positive-time crossing rule silently. An actual blend tree additionally needs a semantic policy for competing footsteps in simultaneous clips, for example dominant-clip gating or normalized phase matching.

The **final visual pose** is a different product again. In column-vector convention, a skinned vertex position in bind coordinates is `p_deformed = Σ_j weight_j (GlobalJoint_j · InverseBind_j) p_bind`, with weights summing to one for conventional linear blend skinning. `GlobalJoint_j` depends on the resolved character root and parent-to-child joint transforms. The inverse-bind transform maps from mesh bind coordinates into each joint's rest space. If the root is updated by physics after animation sampled its provisional pose, the final skinning palette and culling bounds must use the resolved root, not an old provisional transform. Nonuniform scales, shear and normal transformation need special care; dual-quaternion skinning addresses some linear-blend artifacts but has its own scale/blending limitations. GPU uploads must retain the palette and skinned buffers until all consuming draws finish, which is a separate resource-lifetime proof.

The following complete CPU witness models a two-clip controller with one animated foot joint and a scalar root track. It proves clip interpolation, quaternion sign equivalence, duration/marker validation, crossing three events over a loop boundary, blend-weight integration, collision authority and duplicate-tick rejection. The scope is intentionally smaller than a shipping animation engine: there is no asset importer, skeletal remapping, GPU palette buffer, additive layer, IK, ragdoll, multithreaded evaluation, compression or notification bus. This boundary matters more than pretending a short mathematical example proves all those subsystems. Keep the code here in the primary chapter; the identical separately compilable file is `tools/labs/animation_controller_contract_lab.cpp`.

```cpp
// C++20 reference: clip evaluation, controller blending, root ownership and tick events.
// CPU contract only: no skeleton importer, inverse-bind matrices or GPU skinning.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace animation {
void check(bool ok, std::string_view problem) {
    if (!ok) throw std::runtime_error(std::string(problem));
}
struct Vec3 { double x = 0, y = 0, z = 0; };
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 operator*(Vec3 v, double s) { return {v.x*s, v.y*s, v.z*s}; }
Vec3 interpolate(Vec3 a, Vec3 b, double t) { return a*(1-t)+b*t; }

// Quaternion convention: w + xi + yj + zk. A and -A represent the same
// spatial rotation, so shortest-arc slerp must change B's sign if dot < 0.
struct Quat { double w=1, x=0, y=0, z=0; };
double dot(Quat a, Quat b) {
    return a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z;
}
Quat operator-(Quat q) { return {-q.w,-q.x,-q.y,-q.z}; }
Quat operator+(Quat a, Quat b) {
    return {a.w+b.w,a.x+b.x,a.y+b.y,a.z+b.z};
}
Quat operator*(Quat q, double s) { return {q.w*s,q.x*s,q.y*s,q.z*s}; }
Quat normalized(Quat q) {
    const double squared=dot(q,q);
    check(std::isfinite(squared) && squared>1e-24, "invalid quaternion");
    return q*(1.0/std::sqrt(squared));
}
Quat slerp(Quat a, Quat b, double t) {
    check(std::isfinite(t) && t>=0 && t<=1, "invalid slerp weight");
    a=normalized(a); b=normalized(b);
    double cosine=dot(a,b);
    if (cosine<0) { b=-b; cosine=-cosine; }
    cosine=std::clamp(cosine,0.0,1.0);
    // Near equal rotations, linear blending avoids division by tiny sine.
    if (cosine>0.9995) return normalized(a*(1-t)+b*t);
    const double angle=std::acos(cosine);
    const double s=std::sin(angle);
    return normalized(a*(std::sin((1-t)*angle)/s)+
                      b*(std::sin(t*angle)/s));
}

struct JointPose { Vec3 translation; Quat rotation; };
JointPose blend(JointPose a, JointPose b, double w) {
    return {interpolate(a.translation,b.translation,w),slerp(a.rotation,b.rotation,w)};
}
struct Key { double time; JointPose pose; };
struct Marker { double time; std::string name; };
struct Clip {
    double duration = 1.0;             // Positive seconds per loop.
    double root_distance = 0.0;        // Meters per completed loop.
    std::vector<Key> foot_keys;        // One animated local joint in this model.
    std::vector<Marker> markers;       // Author-time semantic events.
};
void validate(const Clip& clip) {
    check(std::isfinite(clip.duration) && clip.duration>0, "bad clip duration");
    check(clip.foot_keys.size()>=2, "insufficient pose keys");
    check(std::isfinite(clip.root_distance), "bad root displacement");
    double previous=-1.0;
    for (const Key& k:clip.foot_keys) {
        check(std::isfinite(k.time) && k.time>=0 && k.time<=clip.duration &&
              k.time>previous, "pose keys must be sorted and unique");
        normalized(k.pose.rotation); // Reject invalid animation input early.
        previous=k.time;
    }
    check(clip.foot_keys.front().time==0 &&
          clip.foot_keys.back().time==clip.duration,
          "loop must have boundary pose keys");
    previous=-1.0;
    for (const Marker& marker:clip.markers) {
        check(marker.time>=0 && marker.time<clip.duration &&
              marker.time>previous && !marker.name.empty(), "invalid markers");
        previous=marker.time;
    }
}

JointPose pose_at(const Clip& clip, double absolute_seconds) {
    check(std::isfinite(absolute_seconds) && absolute_seconds>=0,
          "invalid clip time");
    const double t=std::fmod(absolute_seconds,clip.duration);
    // Inclusive right key means the pose remains defined for exact key times.
    auto upper=std::upper_bound(clip.foot_keys.begin(),clip.foot_keys.end(),t,
        [](double time, const Key& k) { return time<k.time; });
    if (upper==clip.foot_keys.begin()) return upper->pose;
    if (upper==clip.foot_keys.end()) return clip.foot_keys.back().pose;
    const Key& right=*upper; const Key& left=*(upper-1);
    const double alpha=(t-left.time)/(right.time-left.time);
    return blend(left.pose,right.pose,alpha);
}

// This simplified clip declares linear root movement over each loop. The
// unwrapped function makes a 0.99s -> 1.01s update advance, not jump backward.
double root_at(const Clip& clip,double absolute_seconds) {
    const double loops=std::floor(absolute_seconds/clip.duration);
    const double remainder=absolute_seconds-loops*clip.duration;
    return loops*clip.root_distance+
           (remainder/clip.duration)*clip.root_distance;
}
struct Event { std::string name; std::int64_t cycle; };
std::vector<Event> crossed_markers(const Clip& clip,double before,double after) {
    check(after>=before && std::isfinite(before) && std::isfinite(after),
          "backward or invalid event interval");
    check(after-before<=4*clip.duration, "event interval exceeds bounded update");
    std::vector<Event> fired;
    const auto first=static_cast<std::int64_t>(std::floor(before/clip.duration));
    const auto last=static_cast<std::int64_t>(std::floor(after/clip.duration));
    for (auto loop=first;loop<=last;++loop) {
        for (const Marker& marker:clip.markers) {
            const double when=loop*clip.duration+marker.time;
            if (when>before && when<=after)
                fired.push_back({marker.name,loop});
        }
    }
    return fired;
}

struct Advance {
    JointPose visual_foot;
    double requested_root_meters=0.0;
    std::vector<Event> events;
};
class Controller {
    const Clip& idle_;
    const Clip& walk_;
    bool walking_=false;
    double idle_time_=0,walk_time_=0;
    double transition_elapsed_=0;
    static constexpr double fade_seconds=0.20;
    std::uint64_t last_tick_=0;
public:
    Controller(const Clip& idle,const Clip& walk):idle_(idle),walk_(walk) {
        validate(idle_);validate(walk_);
    }
    void begin_walk() {
        check(!walking_, "duplicate transition request");
        walking_=true;transition_elapsed_=0;walk_time_=0;
    }
    Advance tick(std::uint64_t tick_id,double dt) {
        check(tick_id>last_tick_ && std::isfinite(dt) && dt>0,
              "tick must be positive and monotonic");
        check(dt<=0.10, "timestep exceeds animation contract");
        last_tick_=tick_id;
        const double old_walk=walk_time_;
        const double w0=walking_?std::clamp(transition_elapsed_/fade_seconds,0.0,1.0):0.0;
        idle_time_+=dt;
        if (!walking_) return {pose_at(idle_,idle_time_),0.0,{}};
        walk_time_+=dt;
        transition_elapsed_+=dt;
        const double w1=std::clamp(transition_elapsed_/fade_seconds,0.0,1.0);
        // Trapezoidal blend of movement weights is exact for a linear fade.
        // A nonlinear fade needs actual integration to avoid timestep bias.
        const double requested=(root_at(walk_,walk_time_)-root_at(walk_,old_walk))
                                *0.5*(w0+w1);
        auto pose=blend(pose_at(idle_,idle_time_),pose_at(walk_,walk_time_),w1);
        // Clip events are tick events; visual draws do not call tick().
        // A production controller must define event policy for multi-clip blends.
        return {pose,requested,crossed_markers(walk_,old_walk,walk_time_)};
    }
};

struct CharacterMotor {
    double authoritative_root_m=0;
    double previous_root_m=0;
    std::vector<std::string> dispatched_events;
    void commit(const Advance& proposal,double allowed_m) {
        check(std::isfinite(allowed_m) && allowed_m>=0,"bad collision allowance");
        previous_root_m=authoritative_root_m;
        authoritative_root_m+=std::min(proposal.requested_root_meters,allowed_m);
        for (const Event& e:proposal.events) dispatched_events.push_back(e.name);
    }
    double visual_root(double alpha) const {
        check(alpha>=0 && alpha<=1,"invalid presentation alpha");
        return previous_root_m*(1-alpha)+authoritative_root_m*alpha;
    }
};
} // namespace animation

int main() {
    using namespace animation;
    const Quat identity{};
    const Quat half_turn{0,0,0,1}; // 180 deg about local z.
    Clip idle{1,0,{{0,{{0,0,0},identity}},{1,{{0,0,0},identity}}}, {}};
    Clip walk{1,1,
        {{0,{{0,0,0},identity}},
         {0.5,{{0,0.2,0},half_turn}},
         {1,{{0,0,0},identity}}},
        {{0.25,"left-footstep"},{0.75,"right-footstep"}}};
    validate(walk);
    const JointPose at_half=pose_at(walk,0.5);
    check(std::abs(at_half.translation.y-0.2)<1e-12,"lost keyframe");
    const Quat quarter=slerp(identity,half_turn,0.5);
    check(std::abs(quarter.w-std::sqrt(0.5))<1e-12 &&
          std::abs(quarter.z-std::sqrt(0.5))<1e-12,
          "incorrect half-way quaternion orientation");
    check(std::abs(dot(quarter,slerp(identity,-half_turn,0.5)))-1<1e-12,
          "antipodal quaternion sign changed orientation");
    check(std::abs(root_at(walk,1.01)-root_at(walk,0.99)-0.02)<1e-12,
          "root movement snapped at loop boundary");
    const auto boundary_events=crossed_markers(walk,0.24,1.26);
    check(boundary_events.size()==3 && boundary_events[0].name=="left-footstep" &&
          boundary_events[2].cycle==1,"markers lost across loops");

    Controller controller(idle,walk);
    CharacterMotor motor;
    auto stand=controller.tick(1,0.1);
    check(stand.requested_root_meters==0 && stand.events.empty(),
          "idle state unexpectedly moved");
    controller.begin_walk();
    auto first=controller.tick(2,0.1);
    check(std::abs(first.requested_root_meters-0.025)<1e-12,
          "first half-fade root integral incorrect");
    motor.commit(first,1.0);
    const double before_draw=motor.authoritative_root_m;
    (void)motor.visual_root(0.5); // Presentation is read-only.
    (void)motor.visual_root(0.5);
    check(motor.authoritative_root_m==before_draw,"draw modified gameplay root");
    auto second=controller.tick(3,0.1);
    check(std::abs(second.requested_root_meters-0.075)<1e-12,
          "second half-fade root integral incorrect");
    motor.commit(second,0.05); // A wall permits less than the desired delta.
    check(std::abs(motor.authoritative_root_m-0.075)<1e-12,
          "root motion double-applied or ignored collision");
    auto third=controller.tick(4,0.1);
    motor.commit(third,1.0);
    check(motor.dispatched_events.size()==1 &&
          motor.dispatched_events[0]=="left-footstep",
          "footstep event must fire exactly once at the crossing tick");
    bool repeat_rejected=false;
    try { (void)controller.tick(4,0.1); }
    catch (const std::runtime_error&) {repeat_rejected=true;}
    check(repeat_rejected,"same animation tick executed twice");
    bool malformed_rejected=false;
    try { validate(Clip{1,1,{{0,{}},{0,{}}},{}}); }
    catch (const std::runtime_error&) {malformed_rejected=true;}
    check(malformed_rejected,"duplicate author-time keys accepted");
    std::cout << "PASS clip interpolation, shortest-arc blend, loop markers, "
                 "fade root integration, collision authority, tick events\n";
}

```

### One executable world-contract reconstruction — where these hand-offs meet

The sections on startup, input, asset streaming, world identity and animation have one shared failure mode: an operation finishes on one timeline while a different owner is not yet ready to accept its result. The following compact C++20 program is therefore placed **inside the teaching volume**, not outsourced to an unexamined repository example. It follows one set of exact ownership contracts. The independent source file is `tools/labs/engine_world_contract_lab.cpp`; `tools/check_continuous_code_sync.py` checks byte-for-byte agreement of the fenced source body with that file. This is deliberately a deterministic model; its absence of actual asynchronous I/O makes races reproducible here but does not demonstrate that a real worker queue is synchronized correctly.

Read the source from its state changes. The service manager first builds a topological order **without side effects**, then starts each service; a failed service cleans up its own partial state, while successfully started services stop in reverse. The radial stick mapper uses the exact Euclidean radius and does not silently reinterpret diagonal input. The tick input buffer queues both edges of a short tap and empties only at a simulation boundary; UI capture forces an existing gameplay hold to release. The asset slot identifies completions by generation, with separate outstanding reservations, resident bytes and pins. The world stores stable `(cell,local)` identifiers and resolves them only against resident cells. The simulation stores previous/current snapshots plus unconsumed time; interpolation reads but does not change the authoritative state. Finally, the character consumes a root-motion proposal at most once per fixed tick.

A complete reconstruction should reproduce these positive and negative results: failure while starting the renderer unwinds window and platform; 0.5 stick radius becomes 0.375 without changing direction; a press followed by release reaches the next tick as two ordered edges but never repeats; a cancelled 64-byte request cannot publish after a 40-byte successor is issued; resident publication charges 36 actual bytes rather than 40 estimated bytes; a pinned asset/cell refuses eviction; object `(7,42)` survives as an identifier but cannot be dereferenced after unloading cell 7; a 30 ms display interval causes one 20 ms simulation tick and 10 ms remainder, so the authoritative position 0.04 is presented as interpolation 0.02 without mutation; and a blocked 0.5 m root-motion request advances only 0.3 m even if display processing repeats the same tick. Numbers here are test fixtures, not production costs, frame timings or asset sizes.

Build and run the source independently with `clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -DNDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer tools/labs/engine_world_contract_lab.cpp -o /tmp/engine-world-lab && /tmp/engine-world-lab`. GCC C++20 with `-O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror` is also supported. The authoring environment ran both configurations successfully; this statement refers only to the following program, not to a production Kairo build or remote GitHub CI.

```cpp
// C++20 reference: lifecycle, input edges, streaming publication, and world-time ownership.
// This is a deterministic CPU model. It does not perform real I/O or GPU submission.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lab {
void require(bool condition, std::string_view why) {
    if (!condition) throw std::runtime_error(std::string(why));
}

// A provider may allocate something before its start() reports failure. That
// partial allocation must be rolled back by the provider, because the manager
// is only allowed to stop providers that reached the Ready state.
struct Service {
    std::string name;
    std::vector<std::size_t> dependencies;
    bool fail_after_partial_acquisition = false;
    bool ready = false;
    bool partial = false;
    std::vector<std::string>* trace = nullptr;

    bool start() {
        require(!ready && !partial, "service started twice");
        partial = true;
        trace->push_back("acquire:" + name);
        if (fail_after_partial_acquisition) {
            trace->push_back("rollback-partial:" + name);
            partial = false;
            return false;
        }
        partial = false;
        ready = true;
        trace->push_back("ready:" + name);
        return true;
    }
    void stop() {
        require(ready && !partial, "invalid service stop");
        trace->push_back("stop:" + name);
        ready = false;
    }
};

class ServiceManager {
    std::vector<Service> services_;
    std::vector<std::size_t> started_;
public:
    explicit ServiceManager(std::vector<Service> services) : services_(std::move(services)) {}
    // Preflight all graph edges and construct a deterministic topological order
    // BEFORE starting anything. Cycles or missing services cause no side effects.
    bool start_all() {
        require(started_.empty(), "manager must be stopped before restarting");
        const std::size_t n = services_.size();
        std::vector<bool> finished(n, false);
        std::vector<std::size_t> order;
        order.reserve(n);
        for (std::size_t completed = 0; completed < n; ++completed) {
            std::optional<std::size_t> next;
            for (std::size_t i = 0; i < n && !next; ++i) {
                if (finished[i]) continue;
                bool eligible = true;
                for (std::size_t dep : services_[i].dependencies) {
                    require(dep < n && dep != i, "invalid dependency");
                    eligible = eligible && finished[dep];
                }
                if (eligible) next = i;
            }
            require(next.has_value(), "dependency cycle");
            finished[*next] = true;
            order.push_back(*next);
        }
        for (std::size_t index : order) {
            if (!services_[index].start()) {
                stop_all(); // The failed service cleaned up its own partial state.
                return false;
            }
            started_.push_back(index);
        }
        return true;
    }
    void stop_all() {
        // Reverse successful-start order protects each provider from its users.
        for (auto it = started_.rbegin(); it != started_.rend(); ++it)
            services_[*it].stop();
        started_.clear();
    }
};

struct Stick { double x = 0.0, y = 0.0; };
Stick radial_dead_zone(Stick raw, double threshold) {
    require(std::isfinite(raw.x) && std::isfinite(raw.y), "nonfinite stick");
    require(threshold >= 0.0 && threshold < 1.0, "bad dead-zone threshold");
    const double radius = std::hypot(raw.x, raw.y);
    if (radius <= threshold) return {};
    const double bounded = std::min(radius, 1.0);
    // Apply the same radial gain to both axes. The direction is unchanged.
    const double new_radius = (bounded - threshold) / (1.0 - threshold);
    return {raw.x * new_radius / radius, raw.y * new_radius / radius};
}

enum class Edge { Pressed, Released };
class TickInput {
    bool held_ = false;
    std::deque<Edge> pending_;
public:
    void on_physical_button(bool pressed, bool ui_has_capture) {
        // UI ownership suppresses gameplay presses and actively releases a
        // previously held gameplay action so focus transfer cannot stick it.
        if (ui_has_capture) {
            if (held_) lose_focus();
            return;
        }
        if (pressed == held_) return; // Ignore OS repeat, not genuine transitions.
        held_ = pressed;
        pending_.push_back(pressed ? Edge::Pressed : Edge::Released);
    }
    std::vector<Edge> sample_at_tick_boundary() {
        std::vector<Edge> delivered(pending_.begin(), pending_.end());
        pending_.clear(); // Each transition is consumed at most once.
        return delivered;
    }
    bool held() const { return held_; }
    void lose_focus() {
        if (held_) on_physical_button(false, false);
    }
};

enum class AssetState { Empty, Requested, Decoded, Resident };
struct AssetSlot {
    std::uint64_t generation = 0;
    AssetState state = AssetState::Empty;
    std::size_t reserved_bytes = 0;
    std::size_t resident_bytes = 0;
    unsigned pins = 0; // World/render users explicitly prevent eviction.
};

class AssetBudget {
    std::size_t limit_;
    std::size_t reserved_ = 0;
    std::size_t resident_ = 0;
public:
    explicit AssetBudget(std::size_t limit) : limit_(limit) {}
    // Estimated reservation is charged before work starts. This is a logical
    // memory budget, not a GPU fence or exact peak staging-memory estimator.
    std::uint64_t request(AssetSlot& slot, std::size_t estimate) {
        require(slot.state == AssetState::Empty, "asset already active");
        require(estimate <= limit_ - (reserved_ + resident_), "asset budget exceeded");
        reserved_ += estimate;
        slot.reserved_bytes = estimate;
        slot.state = AssetState::Requested;
        return ++slot.generation;
    }
    void cancel(AssetSlot& slot) {
        require(slot.state == AssetState::Requested || slot.state == AssetState::Decoded,
                "cannot cancel inactive request");
        reserved_ -= slot.reserved_bytes;
        slot.reserved_bytes = 0;
        slot.state = AssetState::Empty;
        ++slot.generation; // Invalidates completions already in flight.
    }
    bool decode_finished(AssetSlot& slot, std::uint64_t generation) {
        if (slot.generation != generation || slot.state != AssetState::Requested)
            return false;
        slot.state = AssetState::Decoded;
        return true;
    }
    bool publish(AssetSlot& slot, std::uint64_t generation, std::size_t bytes) {
        if (slot.generation != generation || slot.state != AssetState::Decoded)
            return false;
        // Reject incorrect estimates without changing accounted memory.
        if (bytes > slot.reserved_bytes) return false;
        reserved_ -= slot.reserved_bytes;
        slot.reserved_bytes = 0;
        resident_ += bytes;
        slot.resident_bytes = bytes;
        slot.state = AssetState::Resident;
        return true;
    }
    bool evict(AssetSlot& slot) {
        if (slot.state != AssetState::Resident || slot.pins != 0) return false;
        resident_ -= slot.resident_bytes;
        slot.resident_bytes = 0;
        slot.state = AssetState::Empty;
        ++slot.generation;
        return true;
    }
    std::size_t charged() const { return reserved_ + resident_; }
    std::size_t resident() const { return resident_; }
};

// Persistent authored identity is not a pointer to a resident object. A cell
// can disappear while the identity remains usable in saved quests or scripts.
struct ObjectId { std::uint32_t cell, local; };
struct Cell {
    std::uint32_t cell = 0;
    bool resident = false;
    unsigned pins = 0;
};
std::optional<ObjectId> resolve(const Cell& cell, ObjectId id) {
    if (!cell.resident || cell.cell != id.cell) return std::nullopt;
    return id;
}
bool unload(Cell& cell) {
    if (cell.pins != 0) return false;
    cell.resident = false;
    return true;
}

struct Simulation {
    double previous = 0.0;
    double current = 0.0;
    double velocity = 2.0;
    double accumulator = 0.0;
    std::uint64_t tick = 0;
    static constexpr double dt = 0.02;

    void update(double real_seconds) {
        require(std::isfinite(real_seconds) && real_seconds >= 0.0,
                "invalid monotonic delta");
        accumulator += std::min(real_seconds, 0.1); // Explicit overload policy.
        unsigned steps = 0;
        while (accumulator + 1e-12 >= dt && steps < 5) {
            previous = current;
            current += velocity * dt; // Only fixed simulation writes authority.
            accumulator -= dt;
            ++tick;
            ++steps;
        }
        if (accumulator >= dt) accumulator = std::fmod(accumulator, dt);
    }
    double render_interpolated() const {
        const double alpha = std::clamp(accumulator / dt, 0.0, 1.0);
        return previous * (1.0 - alpha) + current * alpha;
    }
};

// Root motion is a movement *request*. Collision/character movement produces
// the authoritative displacement; skinning then follows the resolved root.
struct Character {
    double world_x = 0.0;
    unsigned footstep_events = 0;
    std::optional<std::uint64_t> last_applied_tick;
    void apply_root_request(double requested, double max_allowed, std::uint64_t tick) {
        require(requested >= 0.0 && max_allowed >= 0.0, "invalid root request");
        if (last_applied_tick && *last_applied_tick == tick) return;
        last_applied_tick = tick; // Tick authority accepts at most one proposal.
        world_x += std::min(requested, max_allowed); // Collision-resolved delta.
        if (requested > 0.0) ++footstep_events; // One gameplay event per tick.
    }
};
} // namespace lab

int main() {
    using namespace lab;
    std::vector<std::string> trace;
    ServiceManager manager({
        {"renderer", {1}, true, false, false, &trace},
        {"window", {2}, false, false, false, &trace},
        {"platform", {}, false, false, false, &trace}
    });
    require(!manager.start_all(), "expected renderer startup failure");
    require(trace == std::vector<std::string>({
        "acquire:platform", "ready:platform", "acquire:window", "ready:window",
        "acquire:renderer", "rollback-partial:renderer", "stop:window", "stop:platform"
    }), "startup rollback order changed");

    // Reject dependency cycles before creating a single native resource.
    std::vector<std::string> invalid_trace;
    ServiceManager cyclic({
        {"a", {1}, false, false, false, &invalid_trace},
        {"b", {0}, false, false, false, &invalid_trace}
    });
    bool cycle_rejected = false;
    try { cyclic.start_all(); }
    catch (const std::runtime_error&) { cycle_rejected = true; }
    require(cycle_rejected && invalid_trace.empty(),
            "cycle had effects before graph validation");

    auto stick = radial_dead_zone({0.3, 0.4}, 0.2); // Radius = 0.5.
    require(std::abs(std::hypot(stick.x, stick.y) - 0.375) < 1e-12,
            "incorrect radial rescale");
    require(std::abs(stick.x / stick.y - 0.75) < 1e-12,
            "dead zone changed stick direction");
    require(radial_dead_zone({0.1, 0.1}, 0.2).x == 0.0, "inside dead zone");
    TickInput action;
    action.on_physical_button(true, false);
    action.on_physical_button(false, false); // Both edges between two ticks.
    const auto edges = action.sample_at_tick_boundary();
    require(edges.size() == 2 && edges[0] == Edge::Pressed &&
            edges[1] == Edge::Released && !action.held(), "tap was lost");
    require(action.sample_at_tick_boundary().empty(), "edge fired twice");
    TickInput captured;
    captured.on_physical_button(true, false);
    captured.on_physical_button(true, true); // Focus moved to an editor control.
    const auto capture_edges = captured.sample_at_tick_boundary();
    require(capture_edges.size() == 2 &&
            capture_edges[1] == Edge::Released && !captured.held(),
            "UI capture left a gameplay action stuck");

    AssetBudget tiny(8);
    AssetSlot first, second;
    tiny.request(first, 8);
    bool overbudget_rejected = false;
    try { tiny.request(second, 1); }
    catch (const std::runtime_error&) { overbudget_rejected = true; }
    require(overbudget_rejected && tiny.charged() == 8,
            "overbudget admission modified accounting");
    tiny.cancel(first);
    AssetBudget budget(128);
    AssetSlot door;
    const auto stale = budget.request(door, 64);
    budget.cancel(door);
    const auto current = budget.request(door, 40);
    require(!budget.decode_finished(door, stale), "stale decode accepted");
    require(budget.decode_finished(door, current), "current decode refused");
    require(!budget.publish(door, current, 41), "oversized publication accepted");
    require(budget.publish(door, current, 36) && budget.resident() == 36,
            "publication accounting incorrect");
    door.pins = 1; // A frame snapshot still references this resource.
    require(!budget.evict(door), "evicted a pinned asset");
    door.pins = 0;
    require(budget.evict(door) && budget.charged() == 0, "asset leak");

    Cell cell{7, true, 1};
    ObjectId quest_target{7, 42};
    require(resolve(cell, quest_target).has_value(), "lost resident target");
    require(!unload(cell), "unloaded pinned world cell");
    cell.pins = 0;
    require(unload(cell) && !resolve(cell, quest_target).has_value(),
            "unloaded object still resolved");

    Simulation sim;
    sim.update(0.03); // One 20 ms simulation step; 10 ms render remainder.
    require(sim.tick == 1 && std::abs(sim.current - 0.04) < 1e-12,
            "simulation advanced incorrectly");
    const double authoritative = sim.current;
    require(std::abs(sim.render_interpolated() - 0.02) < 1e-12,
            "visual interpolation incorrect");
    require(sim.current == authoritative, "render sampling wrote simulation state");
    Character actor;
    actor.apply_root_request(0.5, 0.3, 1); // A wall limits root delta.
    actor.apply_root_request(0.5, 0.5, 1); // Duplicate display evaluation ignored.
    require(actor.world_x == 0.3 && actor.footstep_events == 1,
            "root motion or per-tick side-effect duplicated");

    std::cout << "PASS startup rollback, input edges, request generations, "
                 "budgets, world IDs, interpolation, root-motion ownership\n";
}

```


### Extracting a method should expose its inputs

The older software-rendering episode on wall ordering fixes a visual overlap by changing when a render-distance effect and wall rendering execute. That example concerns a painter-style software composition order; it is not a universal substitute for depth testing in a modern renderer. It also moves a value into an object field when extracting wall code into a method. [Source episode](https://www.youtube.com/watch?v=98K5Bm2sApY)

The general lesson is to identify both dependencies before moving code. An image-composition dependency says which previous pixels an operation reads or replaces. A data dependency says which current camera or movement values it consumes. Passing a frame's inputs explicitly can make the latter visible. A mutable object field can also work, but it introduces a required write-before-read order and raises the possibility of using a previous frame's value. Renaming a variable alone does not define its lifetime or update contract.


## Grow behaviour and presentation without losing ownership

### A screen-space circle is still geometry

A collider and a circle renderer can share an entity, but they answer different questions. The collider contributes a shape to the physics simulation; a renderer contributes visible pixels. For a debugging circle or soft-edged 2D effect, we can use a small quad as a carrier and decide in the fragment shader which of its pixels belong to the circle. This lets us keep ordinary indexed quad geometry while adding a circle-specific vertex format and shader parameters. The distinction matters: the underlying triangles are still rectangular, so GPU visibility and editor picking must use the shader's circle coverage rather than assume the mesh itself has a circular boundary.

As an original mathematical model (the noisy transcript does not preserve the exact shader expression), let the quad's interpolated local coordinate be $q=(x,y)$, with the center at $(0,0)$ and the outer radius represented by $r=1$. A natural signed distance is

$$
d(q)=\sqrt{x^2+y^2}-r.
$$

Inside the circle, $d<0$; on its edge, $d=0$; outside, $d>0$. A shader can use this quantity to draw a filled disc, a ring, or a softened edge. For a ring with width $t$, keep pixels whose radial distance lies near the target contour; for a soft edge, turn a small interval around $d=0$ into fractional alpha. The source episode exposes “thickness” and “fade” controls and adjusts local quad positions from $[-0.5,0.5]$ to $[-1,1]$ so this implicit boundary has a convenient unit radius. The particular parameter ranges and defaults are implementation choices, not standard meanings of “thickness” or “fade.” (Source `src-5237dcb6e45168308d04`, “Rendering Circles in a Game Engine,” captions around 00:14:55–00:20:02 and 00:31:40–00:32:25.)

This is a representation trade: a small fixed mesh carries a boundary that the shader evaluates per pixel. It avoids tessellating a circle into many short line segments when a filled or softened shape is desired. But the fragment shader does not make the rasterized quad disappear from every other system. If the object-ID attachment writes an entity ID for all quad fragments, clicking a transparent corner can still select the circle. The demonstrated fix restricts the ID write to fragments inside the circle. That makes pixel picking follow the visible mask, subject to the usual attachment, depth/blend and readback rules. A ray against the quad would still report the rectangular carrier unless it also tests the implicit circle boundary. (Same source, captions around 00:52:26–00:57:43.)

The circle has an independent batching contract as well. Its shader consumes local position, color, thickness, fade and entity identity, while a sprite shader consumes texture-specific attributes. The tutorial therefore gives circles their own vertex layout and draw count, while reusing the quad index buffer because the index pattern is unchanged. Reuse is valid because both layouts describe the same four corners; it does not mean they can be submitted in one batch under incompatible shaders. If the number of circles grows, either flush several bounded batches or grow a buffer deliberately. A large fixed reserve is not automatically faster or more memory-efficient. (Same source, captions around 00:20:02–00:24:52 and 00:27:47–00:28:34.)

A final check follows the feature across the whole engine. The circle must draw in the editor, write selection identity only where intended, survive entity duplication, participate in scene save/load, and render after Play copies the edit scene. The episode first sees a shader compile failure, then discovers ordinary quads are broken by a missing vertex-array bind; later it adds the circle component to duplication and serialization paths and reports saving/reloading a test scene. These failures show why a correct distance formula is only one part of a renderer feature. The source is a noisy automatic transcript and the final code was not independently built here, so the traces document the reported process rather than certify a current implementation. (Same source, captions around 00:45:22–00:47:17 and 00:59:24–01:04:40.)


With a stable scene model, new features become other consumers and producers of world information. For each one, keep its clock, accepted commands, published results, and lifetime obligations visible.

### Give each native script its own lifecycle

A native script is C++ behavior attached to an entity. The early callback design stores setup and update lambdas in a component, but the episode backs away from that indirection: a `ScriptableEntity` base class can expose virtual `OnCreate`, `OnUpdate`, and `OnDestroy` methods. They have default bodies, so a derived camera controller can override only what it needs. `Scene` invokes the protected callbacks, while the script author supplies the behavior. [Native Scripting (now with virtual functions!)](https://www.youtube.com/watch?v=1cHEcrIn8IQ)

The script type is not instantiated when editor code attaches it. Construction is deferred until the scene starts, leaving a place for instance data to be loaded before behavior begins. A noncapturing function pointer records how to create the derived type later; its return type is the common `ScriptableEntity` interface. Because destruction happens through that base pointer, the base needs a virtual destructor. The compiler catches the derived-to-base return mismatch in the recording, and the example adds an explicit cast.

Lifecycle ownership is still incomplete in this snapshot. The scene update path makes an instance, calls `OnCreate`, then calls `OnUpdate` with the frame timestep. The speaker explicitly leaves scene stop and `OnDestroy` integration for later, noting the editor cannot yet repeatedly play and stop a scene. That is an important boundary: virtual hooks make the intended interface clearer, but they do not by themselves guarantee teardown or repeated-play correctness.

A practical review detail appears in the same episode. EnTT's grouped component access returns a tuple whose elements are references, so copying that tuple into a structured binding preserves access to the referenced components. A direct `get` result is different: binding it with plain `auto` copies the component, while `auto&` refers to the stored component. Check the expression's return type when deciding which binding is correct. These are narrated code-review notes, not an independent source diff or runtime audit.


### Make scripts cooperate with the same lifetime rules

A **script** is game behavior supplied through an embedded language or another controllable programming layer. It might say that a door opens after a plate is occupied, or that an enemy changes destination when it hears a sound. The engine still owns the world and resources. Exposing raw internal pointers to scripts does not remove the need for validation.

Offer a small API expressed in game concepts: query a transform, request a spawn, send an interaction, or set a target. A **binding** connects a script-visible operation to native code. Define what happens when the entity no longer exists, the component is absent, a type is wrong, or an exception occurs. A script error should identify the script, object, and operation. Decide whether it disables that behavior, stops Play, or produces a recoverable diagnostic.

Reloading a script combines code lifetime and data lifetime. Old functions may still be executing, old instances may refer to a previous module, and the replacement may change field types. Pause or reach a safe boundary, validate the new code, choose which state is preserved, and publish the replacement. A scripting language's garbage collector handles its own managed allocations; it does not automatically wait for GPU work or repair native dangling references.

Keep execution phases visible. If scripts change transforms before physics, say whether those writes set a desired pose, teleport the body, or affect forces. If scripts run afterward, say whether they may only react or can schedule a change for the next tick. Two services both “owning position” creates disagreement unless there is a deliberate synchronization rule.

#### Design a scripting boundary around what game behavior needs
A game script usually needs to manipulate game concepts such as entities, scenes, and components. It may not need access to every low-level engine API. One Hazel development log compares a script to a puppeteer: the engine provides the objects and controls, while the script pulls the strings that make gameplay happen. Keeping that boundary deliberate can limit complexity and cross-language data movement. The speaker also cautions that some work may be faster inside the scripting language, so the boundary should be chosen based on the task rather than an absolute rule. [[caption source](https://www.youtube.com/watch?v=ilqJfU3bUtM), full cleaned transcript]

Before committing to a language, prototype the integration points. The speaker describes separate sandbox tests for function binding, resource handling, and reference counting. He first tries C# in a smaller codebase, then moves the work into Hazel-dev, where an early inspector exposes entity components and a script component runs an update function. He explicitly says the C# API is still very limited. This is a useful distinction for teaching: the demonstration establishes a first integration seam, not a finished scripting contract. The comparison of Lua, Python, and C# is historical and specific to the speaker’s goals; do not present it as current language guidance. [[caption source](https://www.youtube.com/watch?v=ilqJfU3bUtM), full cleaned transcript]


### Add scripting at a deliberate boundary

Scripting lets a game author change behavior without rebuilding the engine for every gameplay adjustment. It can mean native C++ classes compiled into a project, a managed runtime such as C#, or an embedded language. Each choice changes iteration speed, safety, debugging, deployment, and the complexity of the engine-to-script bridge.

The public Hazel snapshot includes `ScriptEngine`, `ScriptGlue`, script-related components, and a `ScriptableEntity` abstraction. That shows a scripting boundary existed in the inspected code. It does not establish that every later scripting feature or public release is production-ready. The Game Engine playlist also contains distinct videos on native scripting; title discovery alone cannot tell us which exact feature appeared in which episode. For a specific claim about the explanation, use that video’s transcript and mark it read.

A simple script lifecycle might offer create, update, and destroy callbacks. The engine must decide whether scripts can access entities by stable ID, whether they may add or remove components, how errors are reported, and what happens if a script is reloaded while running. The bridge should expose a small, stable set of operations rather than raw pointers into engine internals. An invalid entity handle should fail safely and visibly.

#### Native versus managed scripting

Native scripting gives direct C++ access, high performance, and easy reuse of existing libraries. It also means compilation is part of iteration, and a crashing script can crash the whole process. ABI compatibility and module unloading can be difficult. Managed scripting can offer faster code reload and stronger runtime checks, but it introduces a language runtime, bindings, garbage-collection interactions, packaging, and deployment concerns. A small engine should choose based on the project’s users, not on a general belief that one language is always better.

A practical boundary includes explicit value types and handles: a transform value, an entity ID, a vector type, and functions to read or write components. The C++ side validates the handle and owns component storage. The script side calls documented operations. This boundary is also where versioning matters; a saved script reference should survive a class rename only if the project has a migration or stable identifier scheme.

##### Check an in-memory layout before treating it as a bridge

A C++ type called `Vec3` is not automatically a safe C# bridge type. Names tell us intent, not object representation. An engine may define its vector with different field order, extra state, custom invariants, alignment, or non-trivial copy behavior. A managed `Vector3` can have its own layout rules too. Before exchanging raw bytes, inspect both concrete definitions and the marshalling contract; do not infer compatibility from the shared name.

The lab defines a deliberately plain `BridgeVec3` with three `float` members. Compile-time checks constrain this native build target to 8-bit bytes, 32-bit IEC 559 floats, standard-layout and trivially-copyable members, twelve total bytes, and offsets 0, 4, and 8. `memcpy` copies a live value into another live object of the same type, then checks finite exact sample values. A second struct has the same size but reverses its first and last fields. Copying the bytes into it succeeds as a memory operation yet produces the wrong meaning: its `x` reads the original `z` and its `z` reads the original `x`. Equal size alone is not a field contract.

`memcpy` here avoids union type punning and pointer reinterpretation. It demonstrates only the native target's stated layout and a same-process copy. It does **not** test Mono or .NET marshalling, calling conventions, native/managed type agreement, garbage-collector pinning or lifetime, network serialization, or integration with an engine. Raw in-memory bytes are not a portable wire format.

For a real boundary, define the native and managed structures explicitly, state field order and representation on both sides, and validate the ABI with tests at that boundary. Also write down ownership and lifetime: who creates and frees a resource, whether a pointer is borrowed or owned, how long a borrow remains valid, how null is handled, and whether managed memory must be pinned. This lab has no pointers or retained resources, so it makes no claim about those contracts.

###### Run the native lab

The exact C++20 source is retained in the following inline block; a verified source copy remains in the batch cache. The normal and UBSan-only commands compiled and ran successfully on the local macOS target, each producing `roundtrip: x=1.25 y=-2.50 z=9.00` and `same-size wrong order: x=9.00 y=-2.50 z=1.25`. The ASan+UBSan build succeeded, but execution produced no output and timed out; sanitizer execution is unverified.

```sh
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror bridge_layout_lab.cpp -o /tmp/bridge_layout_lab && /tmp/bridge_layout_lab
# exit 0

clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -fno-omit-frame-pointer bridge_layout_lab.cpp -o /tmp/bridge_layout_lab_ubsan && /tmp/bridge_layout_lab_ubsan
# exit 0; no UBSan diagnostic

clang++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer bridge_layout_lab.cpp -o /tmp/bridge_layout_lab_san
# build exit 0

timeout 10s env ASAN_OPTIONS=detect_leaks=0 /tmp/bridge_layout_lab_san
# timeout status 124, no output; ASan run unverified
```

```cpp
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <type_traits>

struct BridgeVec3 {
    float x, y, z;
};

struct WrongOrderVec3 {
    float z, y, x;
};

static_assert(CHAR_BIT == 8, "requires 8-bit bytes");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "requires 32-bit IEC 559 float");
static_assert(std::is_standard_layout_v<BridgeVec3>);
static_assert(std::is_trivially_copyable_v<BridgeVec3>);
static_assert(sizeof(BridgeVec3) == 12);
static_assert(offsetof(BridgeVec3, x) == 0);
static_assert(offsetof(BridgeVec3, y) == 4);
static_assert(offsetof(BridgeVec3, z) == 8);
static_assert(sizeof(WrongOrderVec3) == sizeof(BridgeVec3));

int main() {
    const BridgeVec3 sent{1.25f, -2.5f, 9.0f};
    BridgeVec3 received{};
    std::memcpy(&received, &sent, sizeof sent);
    if (received.x != sent.x || received.y != sent.y || received.z != sent.z)
        return 1;

    WrongOrderVec3 wrong{};
    std::memcpy(&wrong, &sent, sizeof sent);
    if (wrong.x != sent.z || wrong.y != sent.y || wrong.z != sent.x)
        return 2;

    std::printf("roundtrip: x=%.2f y=%.2f z=%.2f\n",
                received.x, received.y, received.z);
    std::printf("same-size wrong order: x=%.2f y=%.2f z=%.2f\n",
                wrong.x, wrong.y, wrong.z);
}
```

#### Embed a managed runtime behind an engine boundary

The Hazel 2D scripting episode builds a native host around Mono so the engine can load a C# class library and invoke managed code. Keep the boundary explicit: the engine initializes the runtime, sets the managed assembly search path, creates an application domain, loads an assembly, and resolves types and methods through metadata. A managed object is allocated from its class and its constructor is invoked; later method calls pass arguments in the representation Mono expects. Strings need a managed string object created in the active domain, while primitive arguments are passed by address. A native pointer to a managed object is not automatically rooted or pinned: its use must follow Mono's GC and domain rules, and metadata pointers do not extend the object's lifetime. Mono's [embedding guide](https://www.mono-project.com/docs/advanced/embedding/) distinguishes internal calls from P/Invoke and documents object-movement and foreign-thread attachment concerns.

This setup is useful because the host can inspect the assembly before trying to call arbitrary names. The episode prints discovered types and argues that the editor should use that metadata to offer valid script classes and fields, rather than making users type class names without feedback. It also exposes a useful sequencing lesson: managed assemblies such as mscorlib must be placed where the runtime's configured search path expects them, and native Mono libraries must be linked alongside any platform libraries their build references.

The source is an exploratory Windows build using a particular Mono release, Visual Studio solution, and Premake setup. The choice to vendor binaries, attempt static linking, and include managed libraries reflects that historical experiment; the discussion does not prove a portable packaging policy. More importantly, cleanup is not solved: app-domain unloading races or aborts in the demonstration, and the author leaves reliable shutdown/restart unresolved. The episode calls C# from native Hazel code but defers the internal-call bridge back into engine functionality, ECS lifecycle integration, durable editor-authored values, and a finished script workflow. Treat it as a first runtime-hosting slice, not a completed scripting system.

### Managed scripts extend the host, not the engine's storage model

The later C# clips are best understood as a bridge between a managed runtime and native ECS storage. Native engine components remain native; a C# wrapper carries enough entity/context identity to request values such as transform translation through interop. The C# object is therefore a view/proxy, not the owner of component memory. The engine also needs to load managed assemblies, reflect supported fields for the editor, and decide how those values cross the boundary. (The Cherno, “Using C# Scripting with the Entity Component System,” [MVw9m47xLQw](https://www.youtube.com/watch?v=MVw9m47xLQw), ~00:00–05:00 and 19:44–24:44; “C++ Engine Components in C#,” [K7bJnLmyzto](https://www.youtube.com/watch?v=K7bJnLmyzto), ~00:00–05:00 and 19:47–24:47; “Reading/Writing C# Fields in Editor,” [YEPkLqPW15I](https://www.youtube.com/watch?v=YEPkLqPW15I), ~00:00–05:00 and 19:47–24:47.)

A small native API is safer to evolve than exposing raw component pointers. A native call should define string/array representation, copy versus borrow behavior, lifetime, and error reporting. Managed reflection should expose only field types the editor can read/write correctly; a visible but nonfunctional field is worse than an explicit unsupported-type message. Moving game code into a separate assembly also means the runtime package needs a project root, script assembly, and managed runtime libraries. This connects the C# feature to the earlier clean-distribution failure sequence: scripting adds dependencies that must be included and loaded deliberately.

#### Keep managed scripts as clients of native engine state

A C# component wrapper can represent an entity/component capability without becoming the owner of the engine's component storage. In the example, the managed wrapper carries entity identity; calls such as querying a rigid body or applying an impulse cross back to native code, where the component remains stored. This keeps the engine's ECS storage authoritative while giving scripts a convenient surface. A wrapper reference therefore needs an explicit validity rule: which entity it names, how long that identity remains valid, and what happens after deletion or scene replacement. The historical demonstration introduces wrapper and registration patterns but does not establish a complete lifetime-safe contract. [C++ Engine Components in C#, 00:16:45–00:26:02; 00:32:50–00:39:01]

A native/managed call has several independent agreements: the registered method name and signature, argument representation, ownership of returned or borrowed memory, and how long referenced objects stay alive. The C++/C# episode demonstrates Mono internal-call registration and string conversion, then discusses structs whose field layout must agree on both sides. Matching a type name such as `Vector3` is insufficient; compare concrete definitions and validate both declarations plus the actual managed/native boundary. Native `sizeof` and field offsets alone do not establish managed ABI compatibility. Managed references also need explicit GC roots/pinning and app-domain lifetime rules. The [Mono embedding guide](https://www.mono-project.com/docs/advanced/embedding/) covers object movement, thread attachment, and internal-call argument representation. The small companion lab checks one native target's assumptions only. It does not test Mono/.NET marshalling, calling conventions, garbage-collector pinning, or engine integration. [Calling C++ from C#, 00:16:11–00:19:01; 00:33:46–00:41:26; 00:48:24–01:01:33]

Prefer a small, explicit bridge function over exposing a large engine type wholesale. For example, a managed “apply impulse at center” operation can deliberately hide a lower-level physics API's world-point parameter. That convenience is safe only when its semantics are stated clearly and the native call implements them. The tutorial adapts the historical Box2D API used by that Hazel/Mono branch after checking what the point parameter means; that is a good reminder to read the backend contract instead of guessing from a method name. Its C++ signature and pointer idioms are version-specific and are not a current Box2D API recommendation. [C++ Engine Components in C#, 01:27:16–01:42:23]

### A moving picture needs a pose, not only a position

The door can be moved as one rigid object, but a walking person needs changing internal shape. A **skeleton** is a hierarchy of joints. A **pose** gives their transforms at a moment. A **clip** provides recorded or authored changes over time. **Skinning** uses joint influence weights to deform mesh vertices from that pose. These are separate data and computation stages; an animation file is not automatically a ready character.

For two clips, identify the pose representation before blending. Positions can often be blended componentwise; rotations need a representation and interpolation rule appropriate to rotations. A quaternion is a four-component representation of a rotation, with unit length for the usual rotation interpretation. Linear blending without normalization and attention to sign equivalence can produce unintended results. The integrated mathematics volume develops that mechanism, but the immediate architectural lesson is that a transform is structured data, not an arbitrary float array to average.

An animation **state machine** selects and transitions between modes such as idle, walk, and carry. A **blend tree** combines poses according to parameters such as speed. **Root motion** is displacement carried by the animation's root rather than computed only by movement code. Decide how it meets physics: does animation request a displacement that collision can restrict, or does it override position? If the feet appear to walk while the collider stands still, examine that contract before changing the mesh.

**Worked pipeline:** accept the player's movement intent, compute permitted locomotion, update the animation's speed parameter, evaluate the joint pose, combine it with the object's world transform, prepare drawing data, and render. A physics-driven ragdoll uses a different owner for some joint transforms. The transition should explain who writes each transform on each side of the handoff.

**Test:** freeze game time while allowing the editor camera to move. The character's pose should remain frozen if animation uses game time, while interface motion can continue if it uses another clock. Then single-step one tick and inspect the pose change. This catches accidental use of wall time inside one animation component.

### Sound has another clock and another deadline

An **audio buffer** is a block of sample values the device will play. A **sample rate** states how many samples represent a second. A **mixer** combines active sounds into output channels. The audio device requests work according to its own schedule; it does not wait politely for a slow rendering frame.

The game's `PlayLatch` event can request a sound, but decoding a large compressed file or taking a contested lock inside a time-critical mixing callback can cause an audible gap. Prepare data outside that path where possible. Send bounded requests through an explicitly safe mechanism and decide what happens if requests arrive faster than they can be consumed.

A **voice** is an active playback instance. Two doors can use the same audio asset while having different voices, start times, volumes, and positions. A shared file buffer is not the same as a shared playback cursor. A **streamed** sound keeps feeding data as it plays rather than requiring the entire decoded recording to remain resident. Long music and short latch sounds may therefore need different preparation and memory policies.

For spatial sound, define the listener and source coordinate frames. Attenuation changes loudness with distance; panning distributes signal across outputs; occlusion models obstruction. Those are chosen approximations and artistic controls, not guarantees that a source's physical energy is simulated. If the sound moves backward when the camera rotates, check frame conventions and listener orientation before changing the audio file.

**Test:** repeatedly trigger the latch while moving the camera, then force an asset load and a slow graphics frame. Listen for missed starts or gaps, and record request/voice counts. A system that sounds fine while idle may fail under the engine workload that matters.

#### Author the sound's cause, location, and environment

Return to the workshop door. The motor hums while it is moving; its latch clicks once when it closes. These need different controls. A continuously playing loop has a start, a stop, and parameters that change during playback. A one-shot sound is requested by a discrete event. If a script requests the latch sound on every update while the door is closed, it can accidentally start many overlapping playback instances. Detect the transition into the closed state, or consume a single close event, rather than treating a persistent state as a new event every frame.

A **sound source** describes where a sound originates and how it plays. A **listener** describes the viewpoint from which the mix is heard. In a first-person scene, the listener commonly follows the player or camera, while a machine's source remains with the machine. Moving the listener is not the same operation as moving every source. Decide explicitly which camera owns the listener when the editor has several views, and what happens when the player changes cameras.

A **trigger volume** is a region that detects entry, continued overlap, or exit. Entering a reverberant chamber might start an ambient loop or change an acoustic parameter. These are separate choices: playing a recording of a room does not itself make all other sounds behave as if they are in that room. Define entry and exit behavior, including what happens when the player teleports, reloads a scene, or is destroyed while inside the volume.

An audio effect changes a signal before it reaches the final mix. A **low-pass filter** attenuates higher frequencies; a **high-pass filter** attenuates lower frequencies. Their names describe which frequency region they let pass more readily. A low-pass change can suggest muffling, but it is only an approximation of what a wall does. **Reverberation** adds a decaying pattern of reflected sound. A **bus** groups signals for routing and shared processing. A **send** routes some signal into another processing path, so a reverb send can combine a direct sound with a separately processed reverberant contribution. The amount sent is a parameter, not a second request to start the source.

Keep persistent asset data separate from playback state. Two identical motors can share a sound asset while having different source positions, playback cursors, filter settings, and start times. Stopping one must not accidentally stop the other. When play mode ends, release or stop its voices deliberately; otherwise sound can outlive the world that caused it.

The fully read Hazel audio log connects scene sources, a player listener, scripted ambience, and a trigger-driven sound. It names filters and reverb while describing work in progress. The door example and definitions above expand those ideas for teaching; they do not claim that the video exposes a complete mixer implementation or establishes a particular audio backend. [Caption source: Adding Audio to a Game Engine](https://www.youtube.com/watch?v=BmGHojuQMbk).


### An enemy needs an answerable question at each layer

An enemy trying to reach the player poses several different problems. **Decision-making** chooses what it wants now. **Pathfinding** finds a route through a representation of traversable space. **Steering** turns the route into local motion intent. Physics or a movement controller determines what actually happens against current obstacles.

A **navigation mesh** approximates walkable areas with connected polygons. A graph of room connections can be enough for another game; a grid can suit tile movement. A route is not a collision guarantee. The crate may have moved after the route was computed, a door may have shut, or the enemy may not fit through a gap.

For a simple enemy, use explicit states: patrol, investigate, pursue, recover. Define transitions from observable conditions such as a sound event, visibility result, or lost-target timeout. A **behavior tree** organizes choices and tasks in a tree with evaluation rules. A **utility model** scores alternatives. Neither name removes the need to specify when a task succeeds, fails, remains active, or is cancelled.

**Worked failure:** the enemy alternates rapidly between pursuing and investigating at a doorway. Perhaps two thresholds are identical and noisy visibility switches the state every tick. Add a deliberate memory or transition condition, not an arbitrary wait everywhere. A lower threshold for staying in a state than entering it is **hysteresis**: the system resists switching from a small fluctuation. First confirm that this behavior serves the design; a decisive enemy and a skittish enemy may need different rules.

The useful debugging tool shows the current intent, chosen route, last perception, active task, and reason for the latest transition. “The AI is broken” becomes a specific question: did perception fail, did the decision use stale information, did routing reject the goal, or did movement fail to follow a valid route?

#### Start multiplayer design from the game’s authority needs
A multiplayer engine does not need to synchronize every entity by default. First decide which simulation the server owns, which behavior stays local to the client, and what information crosses the network. A small server and client prototype can expose the right boundary: run server-side gameplay code and physics in a stripped runtime, then report or validate relevant state. Labels such as client, server, or shared can guide which scripts and entities participate. Treat this as an architectural experiment; a working server simulation does not establish secure authority, latency handling, reconciliation, or packet-loss behavior. (Full caption read: [caption source](https://www.youtube.com/watch?v=miNInZ806a0).)


### A remote world is an estimate with an authority rule

Networking is not merely serializing every entity each frame. A **protocol** defines messages, their meanings, and how receivers handle order, loss, duplication, and incompatible versions. **Authority** states which participant can decide each kind of truth. An input request from a client is not automatically permission to teleport an authoritative server object.

Three approaches illustrate different contracts. Lockstep exchanges inputs and relies on sufficiently matching simulation. **Snapshot replication** sends selected state. **Prediction** lets a client estimate the effect of its own recent inputs before an authoritative answer arrives. **Reconciliation** brings that estimate back into agreement, often by replaying inputs after the corrected state. These can be combined, but only with clearly defined timelines.

A remote object may be drawn from buffered snapshots, accepting some display delay to smooth irregular arrivals. Fiedler's snapshot-interpolation article explains this different contract from deterministic lockstep and the tradeoff between buffering, bandwidth, and smooth presentation. It does not imply that the local predicted player and every remote entity should use an identical treatment. [Snapshot interpolation](https://gafferongames.com/post/snapshot_interpolation/).

For the door, send a command with identity and sequence information, then replicate a permitted state change or its inputs according to the protocol. If the same packet arrives twice, it should not necessarily create two crates or charge twice for opening the door. **Idempotent** handling means that repeating a request has the same effect as applying it once, where that is the intended contract. Other events need deduplication records or explicit sequence rules.

**Test:** introduce delay, dropped messages, duplicates, and out-of-order delivery in a controlled harness. Observe world state, not only a network counter. Distinguish a delayed visual estimate from a disagreement about the authoritative door. A stable local frame rate says nothing about protocol correctness.

#### Prove a connection with rectangles before adding a 3D renderer

For the first multiplayer workshop test, draw each player as a rectangle. The local player changes position, a client sends a message, the server stores player state, and other clients draw the updated rectangles. A **headless server** runs without a graphical presentation. A **snapshot** describes selected state at a point in the server's processing. Sending a simple collection of player IDs and states makes this path observable before meshes, lighting, and animation can obscure a network error.

Keep the packet contract explicit. A packet type tells the receiver what kind of message it is parsing. A stable player identifier distinguishes one player's state from another's. Serialization defines how those fields become bytes and how the receiver reconstructs them. A successful local demonstration establishes that those particular messages traveled and were understood; it does not establish behavior under delayed, duplicated, malformed, or lost messages.

There is a critical authority distinction. If a client sends its desired position and the server copies that position into the shared state, the server is a relay and store for client-supplied movement. Calling the process a server does not make the movement securely authoritative. A stronger design might have the server accept permitted inputs, advance movement itself, and reject impossible actions. That change introduces prediction and reconciliation needs on clients; it cannot be obtained merely by renaming a packet.

Threading belongs in the contract too. A network callback may run while the game thread is reading the same player collection. A **mutex** permits mutually exclusive access to protected data. One prototype pattern locks briefly to copy a consistent snapshot, then unlocks before serialization and sending. This avoids keeping the state locked during potentially expensive work, but the copy itself still needs valid ownership and a consistent lifetime. Another pattern queues received messages for processing by the thread that owns the world. Define queue limits and disconnect handling; neither pattern automatically solves every concurrency issue.

The fully read Cubed live tutorial uses 2D player rectangles and a headless server before introducing 3D Vulkan rendering. It narrates local-client and Linux-VPS demonstrations, while acknowledging prototype limitations in state validation, callbacks, timing, and thread coordination. The architecture above expands those learning points; no remote server deployment or networking correctness test was performed for these notes. [Caption source: C++ 3D Multiplayer Game from Scratch](https://www.youtube.com/watch?v=W1MSxy90BLg).


#### Treat a real deployment as part of the prototype
An **ABI**, application binary interface, specifies machine-level calling and data conventions between compiled components. **Marshalling** converts values into the representation expected across a language or process boundary. A platform change can expose mismatches that a same-machine demonstration did not reveal.


A multiplayer server that runs on a developer desktop has not yet demonstrated the deployment path. Build a headless configuration, run it under the target operating system, copy the actual project and runtime assets, then connect clients over the network. Each transition tests a different assumption: platform-specific build behavior, server-only resource needs, packaging, address resolution, and firewall reachability. Keep a small client/server scenario that can be repeated after fixes. Cross-platform failures can come from mundane causes such as path case or ABI/marshalling behavior; narrow them with a reproducible failing call, sanitizer output, and a known working platform comparison. (Full caption read: [caption source](https://www.youtube.com/watch?v=hDCnTTyxLl8).)


##### Separate server responsibilities and prove the deployment target

A server that does not draw a window still may accidentally link client graphics libraries if client and server share one executable and build configuration. A reviewer tests a historical SDL2/ZeroMQ project on Linux and argues for a headless target: remove display and rendering dependencies when the server does not need them, or use a framework’s no-window mode when that is the right tradeoff. A dedicated server target can reduce irrelevant dependencies and clarify which code is expected to run without a display. The source also demonstrates a successful build and server launch, then observes poor client behavior under remote latency; those are separate outcomes and should be reported separately. [Headless deployment review](https://www.youtube.com/watch?v=lAXOmE481j4&t=5m33s)

Build organization affects iteration as well as deployment. The same review observes each game recompiling all engine source files and recommends a shared static library. This can reduce repeated compilation and make ownership boundaries clearer, while still linking the engine code into each final executable. The correct target arrangement depends on the project, but repeated source inclusion should be a deliberate choice rather than an unnoticed build-file accident.

A Linux VPS test is a useful portability check only when the exact environment and commands are recorded. The reviewer installs a compiler and CMake, configures and builds, launches the server, and then tests an actual remote client. That sequence distinguishes “the code compiles on Linux,” “the process launches without a display,” and “the multiplayer experience remains usable with network delay.” Each is a separate claim requiring its own evidence.

#### Bound the work queued between network peers
Sending state every frame can overwhelm a client’s event queue when its frame rate falls behind. Choose an update cadence, measure queue depth, and keep packets limited to the state needed for the next simulation step. Validate payload-size calculations against the number and type of replicated values, then test reconnects and multiple clients. A successful short demo establishes that the path can work; it does not show the queue remains bounded under slow clients, high latency, or many entities. (Full caption read: [caption source](https://www.youtube.com/watch?v=hDCnTTyxLl8).)


### Shipping is another consumer of the same data

The editor may find raw images through a developer's absolute paths. A packaged game must find prepared assets through a stable runtime layout. **Cooking** is preparation for a target: converting formats, resolving dependencies, stripping unnecessary data, and recording versions. A **build artifact** is an output of that preparation, such as an executable or an asset bundle. Neither should depend silently on a file that happens to exist on one developer's desktop.

Keep the dependency graph explicit. If a material uses a texture and a mesh uses that material, changing the texture's relevant input should invalidate the appropriate prepared result. A **content hash** is a digest of bytes used to identify a particular content version; it helps recognize change but does not by itself define compatibility or every relevant input.

The packaged workshop test starts from a clean output directory, launches without the editor, loads the level, operates the plate, saves user state where permitted, closes, and reloads. Missing optional content should follow a fallback policy; missing essential content should fail with a useful error. A demo that runs from the repository root is evidence for that environment, not yet for distribution.


### Use pull requests to make collaboration inspectable

A pull request is a proposed change from one branch or fork into another, accompanied by a reviewable diff and discussion. The practical teaching point is that collaboration includes explaining intent, responding to review, testing a proposed change, and deciding who owns the merge. The episode frames PRs as a team workflow at 00:15–02:15; its first example shows a suggestion that receives discussion and is ultimately closed by its author, not an automatic acceptance. [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=15s) [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=210s)

The video then triages several proposals: some are stale or superseded, one is merged, another is liked but blocked by a conflict, and a build-script change is reproduced locally before being accepted as a candidate fix. The captions walk through discussion and stale state at 04:20–10:30, including a check that a proposed runtime-library change had already become obsolete at 09:20–10:00; they merge a startup-project change around 10:50–12:00, then reproduce a missing-output-directory build failure at 14:40–18:10. The useful general rule is to validate the actual proposed diff against the current base; a friendly review comment or an apparently small patch is not proof it builds. [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=260s) [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=560s) [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=650s) [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=880s)

A branch is a named line of development. A fork is a server-side copy under another account; a contributor can make a branch there and propose merging it back to the upstream repository. The episode outlines that path near 21:20–22:20. Before merge, check the diff, conflicts, build/test evidence, and whether the base branch changed since the proposal was opened. The lesson itself leaves Linux support open pending a Linux VM check around 20:00–21:00, which is a useful example of recording an unverified platform claim instead of treating source changes as portability proof. [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=1280s) [Pull Requests](https://www.youtube.com/watch?v=h0OgLT6KJi8&t=1200s)

These are process lessons from a historical public-repository walkthrough. The exact proposal statuses, people, build commands, and repository state belong to that recording; they are not current Hazel status. Generalize the habits, not the old decisions: write a focused description, keep the patch narrow, test the changed path, respond to review, and state what remains unverified. The captions also mention a README contribution guide as future work, so do not infer a complete contribution policy from this video alone.

#### Review contributions against the current direction

A repository review has two jobs: decide whether a change fits the current code, and decide whether it fits the project's purpose. In the Hazel 2D series, the author merges a physics branch, resolves a component-copy conflict, updates the manually enumerated component list, and adjusts old Vulkan SDK assumptions that broke the debug build. The review distinguishes a missing optional debug library from the minimum SDK needed to compile.

Older pull requests are judged against both the planned renderer rewrite and the educational format. A repeat-count request becomes a simpler repeat boolean after checking what the input callback actually reports. Some old profiling or key-mapping work is left out as obsolete or out of scope. A triangle-based line renderer is kept for later review because its approach may survive a backend change. The question of physical key positions across keyboard layouts remains unresolved.

The practical lesson is specific: a contribution can be technically plausible yet stale, incompatible with the next architectural step, or mismatched with a teaching series where the author needs to explain the implementation. Record what was accepted, revised, deferred, or rejected, and why, rather than treating every open request as a generic feature backlog.

### Make a performance claim identify its workload, measurement, and unfinished work

A frame-time number only answers a useful question when we know what scene, camera, hardware, and measurement produced it. In this Kohi stream, the developer first observes roughly 42 ms per frame in a scene with Sponza, terrain, a car, cascaded shadow maps, and an animated text HUD. That is a symptom and a baseline for this run, not a general benchmark for Vulkan, the engine, or a shipping game. RenderDoc captures initially fail to open or save, and the developer suspects the tool, driver, or machine; moving the capture output to another drive eventually makes a capture work. That sequence is a reminder to separate tool failures from application measurements. [Performance session, 7:34](https://www.youtube.com/watch?v=dJYWL9YAelY&t=454s) [Performance session, 8:00](https://www.youtube.com/watch?v=dJYWL9YAelY&t=480s) [Performance session, 8:22](https://www.youtube.com/watch?v=dJYWL9YAelY&t=499s) [Performance session, 44:17](https://www.youtube.com/watch?v=dJYWL9YAelY&t=2656s)

The first experiment enables frustum culling for the shadow work. In this context, a frustum is the volume a camera or light can see; culling avoids submitting geometry outside that volume. The displayed frame time improves when culling is enabled, but objects and shadows pop in and out because the per-cascade shadow frusta are not yet being combined or selected correctly. A faster image with missing geometry is not a successful optimization. The lesson then sketches distance-to-light-line testing using a bounding sphere: for a point $p$, a line with point $a$ and nonzero direction $d$, the perpendicular distance is

$$
\operatorname{dist}(p,\ell)=\frac{\lVert(p-a)\times d\rVert}{\lVert d\rVert}.
$$

A mesh's transformed center and conservative radius can be compared with the light's relevant radius. The prototype initially used an unset / wrong center, produced invalid values, and failed sorting; after splitting cascade-data preparation from geometry gathering and correcting the transformed center, the demo showed fewer shadow geometries as the camera moved away. That is evidence of a working local experiment in this scene, not proof of robust shadow culling across arbitrary transforms or scenes. [Performance session, 84:21](https://www.youtube.com/watch?v=dJYWL9YAelY&t=5061s) [Performance session, 84:48](https://www.youtube.com/watch?v=dJYWL9YAelY&t=5088s) [Performance session, 93:53](https://www.youtube.com/watch?v=dJYWL9YAelY&t=5633s) [Performance session, 109:16](https://www.youtube.com/watch?v=dJYWL9YAelY&t=6556s) [Performance session, 127:47](https://www.youtube.com/watch?v=dJYWL9YAelY&t=7667s) [Performance session, 151:03](https://www.youtube.com/watch?v=dJYWL9YAelY&t=9063s) [Performance session, 169:36](https://www.youtube.com/watch?v=dJYWL9YAelY&t=10176s)

The source keeps a useful list of work that remains: split the large terrain into visible chunks; add mesh LOD; investigate many scene and shadow draw submissions; inspect the text/HUD cost; review staging-buffer uploads and queue use; and obtain trustworthy GPU timing. The developer explicitly doubts the GPU timer because it measures command preparation/submission without a reliable GPU completion wait, and the units shown in the overlay are inconsistent with observed frame duration. RenderDoc later points at shadow passes and UI/text in this particular capture, but the speaker still concludes that CPU-side work may be dominant and that more profiling is required. Do not turn that tentative diagnosis into a confirmed CPU bottleneck or a measured speedup. [Performance session, 178:32](https://www.youtube.com/watch?v=dJYWL9YAelY&t=10712s) [Performance session, 181:00](https://www.youtube.com/watch?v=dJYWL9YAelY&t=10857s) [Performance session, 184:19](https://www.youtube.com/watch?v=dJYWL9YAelY&t=11059s) [Performance session, 194:04](https://www.youtube.com/watch?v=dJYWL9YAelY&t=11644s)

The distance formula is for an infinite line and requires $\lVert d\rVert>0$. It does not by itself test a finite ray, cone, cascade volume or complete shadow visibility. A conservative culling rule must still retain every caster that can affect the intended receivers.

#### Measure the workload, then state what the evidence proves

Hardware features become useful through workloads, constraints, and software behavior. In a reaction to a PlayStation 5 presentation, the reviewer separates headline clock and throughput numbers from evidence that a shipped game can sustain a frame rate. Heat, room conditions, power use, and workload all affect sustained behavior; a specification alone does not show every game’s outcome. This is a useful engineering habit: translate a hardware claim into a testable question, then measure the workload that matters (ROAD TO PS5 reaction, 00:00:00–00:08:12).

The same distinction applies to audio. A head-related transfer function (HRTF) describes how a listener’s ears and head filter sound arriving from different directions. The presentation describes measuring responses at sampled positions, then using those filters to make a sound appear to come from a location. A real game must also deal with positions between samples, moving or extended sources, and different speaker or headphone setups. That makes “3D audio” a set of workload and listener-fit problems, not one checkbox. The cited video is a creator’s reaction to a presentation; its hardware facts and roadmap statements remain attributed claims, not independent measurements (ROAD TO PS5 reaction, 00:13:31–00:20:33, 00:23:14–00:27:17).

### Separate an issue report from a proposed fix

A repository can make learning reproducible: when each lesson has a commit, a reader can inspect the diff to see what changed and browse the repository at that commit to recover the exact teaching state. The history answers “what did this episode add?”; the checked-out revision answers “what files and behavior existed immediately afterward?” These are different views of the same versioned project.

Use an issue to describe a problem, desired behavior, or proposed direction before investing in a large change whose fit is uncertain. Use a pull request to present an actual patch for review. A small change such as selecting the runnable sandbox as the generated IDE startup target can often be assessed from a narrow diff. A new platform backend or build-system migration carries broader assumptions; agreeing on direction and expected verification first reduces wasted implementation. A milestone groups related issues and patches around a deliverable, while a project board tracks work state. These tools organize decisions; they do not prove the change builds, runs on every platform, or fits the maintainers’ roadmap. [Caption source: GitHub and Hazel](https://www.youtube.com/watch?v=DJEprZioFmU)

A useful contribution loop is: state the observed failure and environment; reproduce or narrow it; agree on the smallest change that addresses the behavior; submit a focused diff; then report exactly which configurations were built and exercised. In the episode, the creator distinguishes a simple Premake startup-project patch from Linux support or CMake support, which would require broader testing and design discussion. That is a historical example of review scope, not a statement about the repository's current policy or build setup. [Caption source: GitHub and Hazel](https://www.youtube.com/watch?v=DJEprZioFmU)

### Connect implementation changes to play tests

When changing allocation strategy, test the player-visible scenario that motivated it: load and unload several levels, run repeated frame scratch allocations, and leave asynchronous work in flight across a buffer swap. Confirm that no old pointer is used after its lifetime ends and that pressure does not silently overwrite neighboring storage. These checks follow from the lifetime rules in the selected memory-management pages; this batch does not include an implementation or test result. (Game Engine Architecture, PDF pp. 229-240.)

When changing integration order, timestep policy, or damping, replay the same action at different frame durations and compare movement, stopping distance, and collision timing. Then ask whether the result is understandable to a player. A numerically stable update that makes a jump feel unexpectedly weak can still break the design contract. Conversely, a tuned physical exaggeration can be valid when the outcome remains consistent and legible. The physics selection establishes approximations and time semantics; the game-design selections explain why perceptible consistency and meaningful consequences matter. (Game Physics Engine Development, PDF pp. 73-76; Game Design: Theory and Practice, PDF pp. 33-35; Rules of Play, PDF pp. 49-52.)


#### Treat iteration evidence as part of the feature

The jam streams show why an engine feature should be tested in the game context that motivated it. A transform copy/paste action lets a developer transfer tuned values from a running scene into editing without relying on screenshots and manual re-entry. A depth-based fog experiment blends scene color toward a configurable fog color; the team then tries to make a ghost transparent, hits a crash, changes a shader setting, and judges the visual result in the prototype. Those are useful steps in an investigation, but the stream does not establish a general transparency contract or a finished renderer feature. Name the state accurately: experiment, working in this scene, integrated path, tested workflow, or released feature ([How We Made a Game in 72 HOURS using OUR OWN GAME ENGINE](https://www.youtube.com/watch?v=tQ_pjZDfm-0&t=1m10s)).

That distinction matters for renderer work too. The historical Sparky Direct3D demo draws blended textures with an updating model matrix, but still lacks cube maps and framebuffers. A PBR dagger then reveals unsupported ambient-occlusion and cavity maps, mismatched gloss response, and absent post-processing; debug gloss controls make one variable easier to inspect while low-gloss behavior remains an open question. A clear demonstration of one material is not full format support. Record the tested scene, missing inputs, remaining edge cases, and which roadmap items are still plans ([Sparky Engine Dev Log #6 - The Start of Direct3D 11](https://www.youtube.com/watch?v=AsNE-eGmGxs&t=7m34s)).

A shipped jam build gives another kind of test: does the player understand the objective and can the game recover from common mistakes? In *Saving Captain Chino*, a compass is introduced because some players did not know which way to go; the player can use the compass again after its cooldown. The narrator also identifies a duplicated coffee pickup and weapon replacement behavior as bugs. The project reports a world around 50 by 50 chunks, each 16 by 16 tiles, with four terrain tile types, but those are details of this build—not universal Hazel limits. When a short game becomes an engine test, preserve the evidence at the level it supports: observed usability issue, game-specific implementation, stated dimension, or unresolved bug ([We Made a Game About Delivering Coffee in a War Zone in Hazel in 72 Hours!](https://www.youtube.com/watch?v=ahrdtYwBLTk&t=2m10s)).

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/39 - Composable timesteps and stable simulation policy|Previous chapter]] · [[Chapter Edition/03 - Game Engine/41 - Evidence, tools, and revisable design|Next chapter]]
