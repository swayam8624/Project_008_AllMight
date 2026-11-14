# 26 — Contact Features and Manifolds

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/25 - SAT for Triangle and Convex Polygon Tests|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping|Next chapter]]

A collision result needs contact locations and normals, not merely an overlap boolean. Feature recovery and manifold construction provide the data needed by a solver.

**By the end:** Explain how contact features become a usable collision manifold.

**Recall question:** Why can a boolean overlap test not provide enough information to resolve a collision?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 64644–67889; source-span SHA256 cbdb3ae0a1c943608f988c7406a11d4805ad8f84c6789f36d77c0eb971d34566. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Contact Features and Manifolds

Original Parts 1255–1262

**Part 1255 — Recovering contact points after convex collision**

By the end of EPA, we may know the penetration normal $n$, depth $\delta$, and one reconstructed pair of surface witnesses.

Is that enough for a rigid-body solver?

Sometimes. For a sphere touching another sphere, one representative contact point is generally a natural choice.

For two boxes resting face-to-face, one point is not necessarily sufficient.

The geometric contact region may cover a two-dimensional patch. If the solver uses only one arbitrary point, the resulting impulses may not accurately resist rotation about the relevant axes.

**Contact constraints act at spatial locations**

A force or impulse applied to a rigid body changes both linear and angular momentum.

If impulse $J$ acts at point $P$ relative to centre of mass $C$, define lever arm:

$$
r=P-C
$$

The angular impulse is:

$$
\boxed{ \Delta L=r\times J }
$$

Thus the location of the contact point matters physically.

Two contacts with the same normal and penetration depth can produce different rotational effects if they act at different locations.

This is why a single penetration vector is not a complete contact description.

**EPA supplies witnesses, not necessarily a full manifold**

The EPA face-witness calculation yields a pair:

$$
P_A,\quad P_B
$$

with approximately:

$$
P_A-P_B=\delta n
$$

That can provide one contact constraint.

But if the touching features are two broad planar faces, there may be an entire region of plausible contacts.

To construct a stable manifold, we often need to inspect the actual geometric features on A and B.

Those features might be vertices, edges, or faces.

**Feature classification**

For a pair of polyhedra, contact geometry commonly falls into categories such as vertex-face, edge-edge, or face-face relationships.

The categories determine how candidate contact points should be generated.

For a vertex-face configuration, one vertex and the opposing face may define a natural contact.

For edge-edge contact, a nearest-point calculation between the two edges can provide a contact location.

For face-face contact, clipping one face against the other usually provides multiple points.

This is why EPA is often followed by feature recovery rather than directly feeding one witness midpoint to the solver.

**What a manifold stores**

A practical three-dimensional manifold can contain a shared normal and several contact points.

Each point may have separate local anchors on A and B, signed separation, accumulated normal impulse, friction impulses, and feature identifiers.

```
struct ContactPoint {
    Vec3 localAnchorA;
    Vec3 localAnchorB;

    // Negative values indicate penetration under our convention.
    double separation;

    // Cached impulses from the previous simulation step.
    double accumulatedNormalImpulse;
    Vec2 accumulatedTangentImpulse;

    std::uint64_t featureId;
};

struct ContactManifold {
    Vec3 normalAtoB;

    std::array<ContactPoint, 4> contacts;
    std::size_t count;
};
```

This is a conceptual 3D manifold layout. A particular solver may store more or fewer fields, different tangent representations, or speculative contacts with positive separation.

The important design principle is that a contact manifold is solver input with geometric and temporal meaning, not merely a list of world-space points.

**Part 1256 — Feature IDs and persistent contacts**

Two bodies touching across successive physics steps usually retain much of their geometric contact relationship.

For example, a box sitting on a floor does not construct an entirely unrelated contact arrangement every frame.

The contact points move slightly as the bodies rotate or settle.

This is an opportunity to reuse information from previous steps.

**Why temporal coherence matters**

Most iterative rigid-body solvers do not solve the exact simultaneous contact problem in one step.

They repeatedly adjust impulses until constraint violations become sufficiently small.

A solver beginning from zero impulses every frame may need more iterations to converge.

If previous contact impulses are still relevant, we can use them as an initial estimate.

This is called warm starting.

**Accumulated normal impulse**

Suppose a contact uses accumulated normal impulse:

$$
\lambda_n^{(k)}
$$

from the previous frame.

At the beginning of the new frame, a warm-started solver applies the cached impulse approximately:

$$
\boxed{ J_n=\lambda_n^{(k)}n }
$$

with equal and opposite impulses on the bodies.

The iterative solver then calculates corrections to the accumulated impulse.

This often improves convergence for resting stacks and persistent contacts.

But the cached impulse is only meaningful if it belongs to the same continuing geometric contact.

**Why matching by array position is unreliable**

Imagine a manifold with four points.

In one frame, the points are stored in order:

$$
A,B,C,D
$$

In the next frame, the clipping algorithm returns:

$$
C,A,D,B
$$

If the solver simply copies impulse number 0 to new point number 0, it assigns A's old impulse to C.

That can produce artificial torque and jitter.

A stable matching method is required.

**Feature-based identifiers**

A contact may be generated from a known pair of geometric features.

For example, a clipped point might derive from an incident edge and a reference side edge.

A feature ID can encode the relevant identities:

$$
\boxed{ \operatorname{ContactID} = \operatorname{encode}(\operatorname{feature}_A,\operatorname{feature}_B) }
$$

The exact encoding depends on the shape representation.

For boxes, useful IDs can include face indices, edge indices, vertex indices, and reference/incident orientation.

The feature ID must be stable under small body motion.

It should not simply be a hash of the world-space contact position, because that position changes continuously even while the same features remain in contact.

Erin Catto's contact-manifold work and the Box2D family use feature identity and persistence to improve temporal coherence. The clipping-based construction is also described in the dyn4j contact tutorial.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://dyn4j.org\&sz=32)

dyn4j.org+1



**Geometric matching as an alternative**

If stable feature IDs are unavailable, a solver can attempt to match new and old contacts using local anchors.

Suppose the previous contact had local anchors:

$$
a_A^{\mathrm{old}},a_B^{\mathrm{old}}
$$

and the new contact has:

$$
a_A^{\mathrm{new}},a_B^{\mathrm{new}}
$$

Define a matching error:

$$
\boxed{ E= \|a_A^{\mathrm{new}}-a_A^{\mathrm{old}}\|^2+ \|a_B^{\mathrm{new}}-a_B^{\mathrm{old}}\|^2 }
$$

Also require normal agreement:

$$
\boxed{ n_{\mathrm{new}}\cdot n_{\mathrm{old}}\ge\tau_n }
$$

Choose the best unused previous contact satisfying the configured thresholds.

This avoids reusing an impulse from a nearby but geometrically unrelated contact.

However, distance-based matching may become ambiguous when several contacts are closely spaced.

**Your implementation already performs contact caching**

Your [PhysicsWorld.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/PhysicsWorld.cppm?utm_source=chatgpt.com) contains `RestoreContactCache()` and `StoreContactCache()`.

The current cache stores body and collider identifiers, body-local anchors, a normal, and accumulated normal/tangent impulses.

During restoration it compares collider pairs, normal agreement, and distances between new and cached local anchors.

This is a meaningful warm-start system.

It is not presently the same as explicit feature-ID-based matching. The observed implementation uses proximity and normal coherence rather than persistent edge/face identifiers.

Adding feature IDs could improve matching for polyhedral contacts when topology is stable, but it would need to coexist with the current local-anchor fallback because not every shape has discrete persistent surface features.

**Part 1257 — Clipping incident faces against reference faces**

This is the central contact-manifold construction algorithm for colliding convex polyhedra.

Suppose SAT has already found a suitable contact normal.

We want to construct the overlap region between the relevant faces.

**Reference and incident faces**

Assume the selected contact normal points from body A toward body B:

$$
n
$$

Choose a face of A whose outward normal best agrees with $n$.

This becomes the reference face.

Choose a face of B whose outward normal is most opposed to $n$.

This becomes the incident face.

The incident face is clipped against the side planes bounding the reference face.

For arbitrary polyhedra, a face may contain any number of vertices.

For an OBB, a face is a rectangle with four vertices.

**Reference face coordinates**

Let the reference face have centre:

$$
C_R
$$

outward normal:

$$
n
$$

and orthonormal in-plane axes:

$$
u,v
$$

with half-extents:

$$
e_u,e_v
$$

Every point on the reference rectangle can be written as:

$$
\boxed{ P=C_R+\alpha u+\beta v }
$$

where:

$$
|\alpha|\le e_u,\qquad|\beta|\le e_v
$$

These inequalities define four side half-spaces.

They are:

$$
\boxed{ u\cdot(P-C_R)\le e_u }
$$

$$
\boxed{ -u\cdot(P-C_R)\le e_u }
$$

$$
\boxed{ v\cdot(P-C_R)\le e_v }
$$

$$
\boxed{ -v\cdot(P-C_R)\le e_v }
$$

The incident polygon must be clipped against all four constraints.

**Sutherland–Hodgman clipping**

Consider a clipping half-space:

$$
\boxed{ n_c\cdot X\le c }
$$

Define signed side value:

$$
s(X)=n_c\cdot X-c
$$

A point is inside when:

$$
s(X)\le0
$$

Now consider one incident polygon edge from $P_0$ to $P_1$.

Its parametric equation is:

$$
P(t)=P_0+t(P_1-P_0)
$$

We want the point where the edge crosses the clipping plane:

$$
n_c\cdot P(t)=c
$$

Substitute:

$$
n_c\cdot P_0+t\,n_c\cdot(P_1-P_0)=c
$$

Using:

$$
s_0=n_c\cdot P_0-c
$$

and:

$$
s_1=n_c\cdot P_1-c
$$

we obtain:

$$
\boxed{ t=\frac{s_0}{s_0-s_1} }
$$

The crossing point is:

$$
\boxed{ P_{\mathrm{cross}}= P_0+t(P_1-P_0) }
$$

This is the same affine-interpolation principle used for segment-plane intersection.

**Four clipping cases**

For each incident polygon edge, classify its endpoints.

If both endpoints are inside, keep the destination endpoint.

If the first is inside and second outside, keep the crossing point.

If the first is outside and second inside, keep the crossing point followed by the destination.

If both are outside, retain neither endpoint, unless additional geometry or degeneracy policies require special handling.

Applying this process to every polygon edge produces a new polygon contained in the clipping half-space.

Repeat the operation for every side plane of the reference face.

**Executable clipping code**

The reference lab uses this Sutherland–Hodgman implementation:

```

std::vector<Vec3> clipPolygon(
    std::vector<Vec3> polygon,
    Vec3 normal,
    double offset)
{
    // Retain the half-space dot(normal, point) <= offset.
    if (polygon.empty())
        return {};

    std::vector<Vec3> result;

    Vec3 previous = polygon.back();
    double previousSide =
        dot(normal, previous) - offset;

    for (const Vec3& current : polygon) {
        const double currentSide =
            dot(normal, current) - offset;

        const bool previousInside =
            previousSide <= 0.0;

        const bool currentInside =
            currentSide <= 0.0;

        // A change of side means the polygon edge
        // crosses the clipping plane.
        if (previousInside != currentInside) {
            const double t =
                previousSide /
                (previousSide - currentSide);

            const Vec3 intersection =
                previous +
                (current - previous) * t;

            result.push_back(intersection);
        }

        // Keep the destination when it belongs
        // to the accepted half-space.
        if (currentInside)
            result.push_back(current);

        previous = current;
        previousSide = currentSide;
    }

    return result;
}

```

This is a readable version of the algorithm implemented in the downloadable lab.

A production contact clipper should additionally propagate feature provenance, handle duplicate vertices created on clipping boundaries, and use an explicit numerical policy for points extremely close to a side plane.

**Final penetration filtering**

After clipping against the four reference side planes, the resulting points lie within the infinite prism above and below the reference rectangle.

But not every such point necessarily penetrates the reference face.

For an incident point $P_B$, calculate:

$$
\boxed{ s=n\cdot(P_B-C_R) }
$$

If:

$$
s>0
$$

the point lies in front of the reference face plane.

If:

$$
s<0
$$

the incident point lies behind that plane, producing a penetrating face contact under the selected configuration.

The projected point on the reference face is:

$$
\boxed{ P_A=P_B-sn }
$$

The contact penetration depth is:

$$
\boxed{ \delta=\max(0,-s) }
$$

The representative midpoint is:

$$
\boxed{ P_c=\frac{P_A+P_B}{2} }
$$

Notice that every clipped contact point has its own penetration depth. It should not automatically inherit the global SAT penetration depth.

That distinction is important for tilted faces, where different corners penetrate by different amounts.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://dyn4j.org\&sz=32)

dyn4j.org



**Part 1258 — Box-box contact manifold generation**

We can now assemble the full face-contact process.

First, run OBB-OBB SAT.

If a separating axis exists, reject the pair.

Otherwise, classify the selected minimum-penetration feature.

When the selected axis is associated with a face normal, use face clipping.

When it is associated with an edge-cross-product axis, use an edge-edge contact construction.

**Face-face worked example**

Take box A:

$$
A=[(-1,-1,-1),(1,1,1)]
$$

and box B:

$$
B=[(0.5,-1,-1),(2.5,1,1)]
$$

Their overlap along X is:

$$
[0.5,1]
$$

The penetration depth is:

$$
\delta=0.5
$$

Choose normal from A toward B:

$$
n=(1,0,0)
$$

A's reference face is:

$$
x=1
$$

B's incident face is:

$$
x=0.5
$$

Both faces span:

$$
-1\le y\le1,\qquad-1\le z\le1
$$

The incident face is already contained within the reference face's side boundaries.

Clipping therefore retains four corners:

$$
P_B=(0.5,\pm1,\pm1)
$$

The corresponding reference witnesses are:

$$
P_A=(1,\pm1,\pm1)
$$

Every contact has:

$$
\delta=0.5
$$

The midpoint contacts lie at:

$$
\boxed{ P_c=(0.75,\pm1,\pm1) }
$$

The result is a four-point contact manifold representing the rectangular overlap patch.

Two boxes overlapping along one axis. In 3D, their facing rectangular surfaces produce a clipped contact patch, typically represented by four corner contacts.

**Why face normals alone cannot determine the manifold**

The SAT normal identifies the direction along which the boxes can be separated.

But the contact point locations depend on the face geometry.

For the example above, the same penetration normal could be associated with many different offsets of B along Y and Z.

The clipped contact polygon changes accordingly.

A manifold algorithm therefore needs both the normal and actual incident/reference feature geometry.

**Edge-edge contacts**

Suppose two thin boxes intersect near one edge of each shape.

The separating-axis candidate may be:

$$
n=A_i\times B_j
$$

where $A_i$ and $B_j$ are the relevant edge directions.

Once those edge features have been identified, calculate the nearest points between their finite segments.

Let:

$$
P_A,\ P_B
$$

be the closest points.

A representative contact is:

$$
P_c=\frac{P_A+P_B}{2}
$$

with the SAT-derived normal and a compatible penetration estimate.

The contact reconstruction should handle nearly parallel edges and endpoint cases.

A face-clipping algorithm alone does not correctly represent every edge-edge configuration.

**Reference-face selection and flip-flopping**

When both bodies have plausible reference faces, a small numerical change can cause the algorithm to switch between them from one frame to the next.

That may reorder or shift contact points and disrupt warm starting.

A production manifold generator can prefer an existing reference feature when the competing candidate scores are nearly equal.

It can also apply deterministic tie-breaking and preserve stable feature IDs.

The purpose is not to change collision geometry arbitrarily, but to reduce unnecessary temporal changes in the contact representation.

**Your current box-box manifold generation**

The inspected `Narrowphase.cppm` contains `BuildBoxBoxContacts()`.

It checks vertices of A inside B, then vertices of B inside A, and includes a support-point fallback when none are found.

It also reduces the manifold to a maximum of four points.

This is functional vertex-based candidate generation. However, it is not the same as reference/incident face clipping.

A contact region can exist even when no box corner lies inside the other box, particularly in certain rotated edge or face configurations.

The support fallback can still provide a constraint, but one representative point may be less stable than a clipped patch.

The next useful engine improvement would be to classify the SAT-selected feature and use reference/incident face clipping for face contacts while retaining a separate edge-edge path.

**Part 1259 — Reducing many contacts to a stable manifold**

Clipping a convex polygon against another convex region can produce several candidate points.

A complicated polyhedron or a persistent manifold cache may accumulate more points than the solver wishes to process.

A typical real-time rigid-body solver uses a bounded number of contacts per pair.

For three-dimensional box collisions, four contacts is a common practical limit.

The problem is not merely choosing any four.

We want a set that provides a useful approximation of the contact patch and remains stable across frames.

**Why keeping the deepest four is not always ideal**

Suppose six candidate contact points all lie near one corner of a broad contact patch, while two other points lie near opposite corners.

Keeping only the four deepest points may cluster the contacts in one region.

The resulting impulses can poorly represent the contact's rotational support.

A better selection tries to preserve both penetration significance and spatial coverage.

**A practical reduction strategy**

One common approach is to retain the deepest point, then select other points that maximize spread across the contact patch.

For example, choose the next point farthest from the first, then a point maximizing triangle area with the first two, then a fourth that increases spatial coverage.

A simpler algorithm repeatedly chooses the candidate whose distance from the already selected set is greatest.

The goal is to avoid clustering.

However, the metric should ideally be evaluated in the contact plane rather than rewarding separation in the normal direction.

**Contact-patch area**

For selected points $P_0,P_1,P_2$, the triangle area is:

$$
\boxed{ A=\frac12 \|(P_1-P_0)\times(P_2-P_0)\| }
$$

If the points lie in the contact plane, this measures the area covered by those representatives.

For four points, a convex-hull area measure can help select a widely distributed set.

Maximizing geometric coverage can improve resistance to unwanted rotation for broad contacts, although it does not guarantee optimal solver convergence.

**Temporal coherence during reduction**

Suppose a previous frame already had four stable contacts.

A new frame produces six candidates, including those same features plus two new points.

Replacing the entire manifold based solely on current-frame penetration could destroy warm-start continuity.

A more stable policy can prefer persistent contacts when their geometry remains valid and their new separation values are still useful.

Contact IDs, local-anchor matching, deterministic ordering, and stable reduction tie-breaking work together.

**Your reduction routine**

Your `Narrowphase.cppm` already implements `ReduceContactPoints()`.

It retains the deepest point, then repeatedly chooses a point maximizing distance from the selected contacts.

That is a sensible spread-oriented baseline.

Its current distance comparison is performed on world-space contact positions.

For broad contact patches, using positions projected into the tangent plane may give a more direct measure of contact coverage.

The higher-priority improvement, however, is to generate geometrically appropriate candidates through feature clipping before reducing them.

**Part 1260 — Collision normal orientation conventions**

A contact normal is a directed quantity.

A vector and its negative describe the same geometric line but produce opposite impulse directions.

We therefore need one unambiguous convention.

For this pass, use:

$$
\boxed{ n=\text{unit normal pointing from collider A toward collider B} }
$$

This means the contact response on B acts along $+n$, while the equal-and-opposite impulse on A acts along $-n$.

**Consistency with the Minkowski difference**

We defined:

$$
C=A-B
$$

For a penetrating configuration, EPA returns an outward direction $n$ from the origin toward the nearest boundary of C.

Translating B by $\delta n$ shifts C by $-\delta n$, bringing that boundary to the origin.

Thus the EPA normal can be interpreted as a translation direction for B, consistent with the A-to-B separation convention.

This follows from the Minkowski-difference algebra, not necessarily from the vector between the shape centres.

**Why centre-difference flipping can be wrong**

A tempting rule is:

```
if (dot(normal, centerB - centerA) < 0)
    normal = -normal;
```

For spheres, the centre direction is naturally related to the contact normal.

For arbitrary asymmetric convex shapes, the correct minimum translation direction need not align with the centre displacement.

Forcing that alignment may reverse a geometrically valid EPA normal or make it inconsistent with reconstructed witness points.

A stronger approach derives the normal from the collision construction and then converts it only when the algorithm's documented A/B convention requires that conversion.

**Contact-relative velocity**

Let the contact offsets from the bodies' centres of mass be:

$$
r_A=P_A-C_A
$$

$$
r_B=P_B-C_B
$$

The velocity of a rigid-body point is:

$$
\boxed{ v(P)=v_C+\omega\times r }
$$

where $v_C$ is centre-of-mass linear velocity and $\omega$ is angular velocity.

Thus the two contact-point velocities are:

$$
v_A^c=v_A+\omega_A\times r_A
$$

$$
v_B^c=v_B+\omega_B\times r_B
$$

The relative velocity of B with respect to A is:

$$
\boxed{ v_{\mathrm{rel}}=v_B^c-v_A^c }
$$

Its component along the contact normal is:

$$
\boxed{ v_n=n\cdot v_{\mathrm{rel}} }
$$

With the A-to-B normal convention:

$$
v_n<0
$$

means the contact points are approaching along the normal.

$$
v_n>0
$$

means they are separating.

The solver's impulse sign convention should agree with this interpretation.

**Part 1261 — Penetration depth and signed separation conventions**

A reliable solver should not use one scalar interchangeably for gap, penetration, and positional correction.

Let the contact witnesses be $P_A$ and $P_B$, with normal $n$ from A toward B.

Define signed separation:

$$
\boxed{ s=n\cdot(P_B-P_A) }
$$

The meaning is:

$$
\boxed{ \begin{aligned} s>0 &: \text{positive separation}\\\ s=0 &: \text{touching}\\\ s<0 &: \text{penetration} \end{aligned} }
$$

Under this convention, the nonnegative penetration depth is:

$$
\boxed{ \delta=\max(0,-s) }
$$

For an EPA witness pair:

$$
P_A-P_B=\delta n
$$

Therefore:

$$
s=n\cdot(P_B-P_A)=-\delta
$$

This is internally consistent.

**Why signed separation is useful**

A physics solver may need to handle not only penetrating contacts but also speculative contacts separated by a small positive gap.

Suppose two bodies are currently separated by:

$$
s=0.01\text{ m}
$$

but their relative normal velocity is:

$$
v_n=-2\text{ m/s}
$$

Over a timestep:

$$
\Delta t=0.01\text{ s}
$$

their predicted normal closure is approximately:

$$
-2(0.01)=-0.02\text{ m}
$$

The predicted separation becomes:

$$
0.01-0.02=-0.01\text{ m}
$$

A speculative contact can attempt to restrict the motion before actual penetration occurs.

Such a constraint requires a signed gap, not just a positive penetration depth.

Modern Box2D documentation describes speculative collision constraints as part of its approach to handling fast motion.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://box2d.org\&sz=32)

box2d.org



**Contact velocity constraint**

For a nonpenetrating contact without restitution, the desired post-solve normal velocity is generally nonnegative:

$$
\boxed{ v_n^{\mathrm{post}}\ge0 }
$$

The normal impulse magnitude is constrained:

$$
\boxed{ \lambda_n\ge0 }
$$

because a standard unilateral contact may push bodies apart but not pull them together.

Ideal complementarity at a velocity-level contact can be expressed as:

$$
\boxed{ \lambda_n v_n^{\mathrm{post}}=0 }
$$

under the simplified no-bias, no-restitution, currently active contact model.

This expresses that a positive normal impulse is applied only when necessary to satisfy the nonpenetration velocity constraint.

Real solvers also introduce restitution, bias, softness, speculative separation, and numerical tolerances, which modify the exact constraint equations.

**Why penetration depth is not directly an impulse**

A penetration depth is measured in metres.

An impulse has units:

$$
\mathrm{kg\,m/s}
$$

Therefore we cannot simply apply an impulse numerically equal to penetration depth without introducing effective mass, timestep, and solver rules.

Penetration information may contribute to a velocity bias or position correction.

The actual impulse depends on relative velocity, mass, inertia, lever arms, and constraint coupling.

Those are the central mathematical subjects of Phase 18.

**Part 1262 — Broadphase false positives versus narrowphase correctness**

The broadphase and narrowphase have different reliability contracts.

A broadphase uses inexpensive spatial proxies to identify candidate pairs.

For example, it may test world-space AABBs even when the actual colliders are rotated boxes or convex hulls.

Two AABBs can overlap while their corresponding objects remain disjoint.

This is a false positive.

False positives are expected in a conservative broadphase.

**False negatives are more dangerous**

Suppose the actual objects intersect, but their broadphase proxies are too small or incorrectly updated.

The broadphase may discard the pair.

The narrowphase never receives it.

The collision is missed regardless of how accurate the narrowphase algorithms are.

Therefore the desired broadphase condition is:

$$
\boxed{ A_{\mathrm{actual}}\subseteq A_{\mathrm{proxy}} }
$$

and similarly for B.

If the actual shapes intersect, their enclosing proxies must also intersect.

**Narrowphase exactness is a separate requirement**

A narrowphase should perform the geometric query promised by its API.

But a finite-precision narrowphase may itself be approximate or conservative.

For instance, an OBB SAT implementation using epsilon-expanded projected radii may deliberately report overlap for some very small separations.

A GJK routine returning no collision after a numerical stall is not necessarily producing an exact separation certificate.

A collision system should therefore distinguish:

$$
\text{Definitely separated}
$$

$$
\text{Definitely overlapping}
$$

$$
\text{Touching or within tolerance}
$$

$$
\text{Unresolved}
$$

when its algorithm can enter uncertain states.

**Suggested result architecture**

```
enum class CollisionStatus {
    Separated,
    Touching,
    Penetrating,
    Unresolved
};

enum class FailureReason {
    None,
    InvalidInput,
    DegenerateSimplex,
    StalledSupport,
    InvalidEPAHull,
    IterationLimit
};

struct CollisionResult {
    CollisionStatus status;
    FailureReason failure;

    Vec3 normal;
    double penetrationDepth;

    std::vector<ContactPoint> contacts;
};
```

A production implementation might avoid dynamic allocation in the hot path through a small fixed-capacity contact array.

The important idea is to avoid confusing algorithmic nonconvergence with definite geometric separation.

**A complete engine pipeline**

Conservative broadphase

Generate candidate pairs

Primitive analytic tests / SAT / GJK

Determine geometric overlap or separation

EPA / feature recovery / clipping

Recover normal, depth, and contact features

Persistent manifold cache

Match contacts and restore accumulated impulses

Constraint solver

Compute impulses and positional corrections

This is the architecture connecting M075 and M076 to the rigid-body solver material in Phase 18.


<!-- BEGIN IMPLEMENTABLE SWEPT COLLISION GEOMETRY 2026-11-12 -->
#### Construct contact geometry that a solver can actually consume

The collision detector and the dynamics solver answer different questions. Geometry asks **whether two shapes occupy the same place now, which surfaces touch, and whether their swept motion first meets inside the current time interval**. Dynamics asks **what velocity change follows from that contact given masses, inertia and materials**. Treating these as one vaguely named "physics collision" function hides the data boundary where most implementation errors originate.

A useful narrow-phase result is a value containing the ordered pair of body handles, geometric feature IDs, world-space contact position(s), a unit normal with a declared orientation, signed separation or penetration depth, and a status distinguishing separated, touching, overlapping, degenerate and numerically uncertain geometry. The narrow phase should not silently invent friction or change velocity. The rigid-body solver later consumes the geometry and body state, computes effective mass and impulses, and may retain a contact cache keyed to the **same geometric features**, not to an arbitrary row index.

Start with two disks. Let $A$ have centre $\mathbf a$, radius $r_A$, and $B$ have centre $\mathbf b$, radius $r_B$. Put $\mathbf d=\mathbf b-\mathbf a$, $L=\|\mathbf d\|$, and $R=r_A+r_B$. When $L>0$, the A-to-B contact normal is $\mathbf n=\mathbf d/L$, and signed surface separation is $s=L-R$. Positive $s$ means no overlap, negative $s$ means penetration. The corresponding surface witness points are $\mathbf p_A=\mathbf a+r_A\mathbf n$ and $\mathbf p_B=\mathbf b-r_B\mathbf n$. A representative solver point may be their midpoint, but for penetrating disks those two points are distinct and this midpoint is a *policy*, not an actual surface intersection. For coincident centres, $\mathbf n$ is undefined: return an explicit degenerate result or a documented deterministic fallback, and never divide by $L=0$.

For polygons, the same output cannot be obtained by treating every overlap as one generic point. Two boxes resting face-to-face have a contact *segment* in 2D, or often a polygonal patch in 3D. A single point at the centre of the contact region may accidentally fail to provide the torque constraints needed to keep the boxes from tipping. An implementation can first choose a reference face and incident face from the collision normal, then clip the incident feature against the side planes of the reference face, retaining points whose signed separation satisfies the contact policy. In 2D, clipping an incident segment with endpoints $\mathbf q_0,\mathbf q_1$ against a half-space $\mathbf n_s\cdot(\mathbf x-\mathbf o)\le0$ uses signed distances $d_i=\mathbf n_s\cdot(\mathbf q_i-\mathbf o)$. If both are inside, keep both; if both are outside, keep none; if they straddle, insert the crossing at

$
\mathbf q(t)=\mathbf q_0+t(\mathbf q_1-\mathbf q_0),
\qquad t=\frac{d_0}{d_0-d_1}.
$

The denominator vanishes for equal signed distances; this is never an instruction to divide blindly. Treat exact or near-coplanarity according to a consistent signed-distance policy. Clip against both side planes and then evaluate each surviving point's separation from the reference face. Contact feature IDs should encode the contributing incident/reference vertices or edges and their canonical body ordering. When the clipper changes which two points survive, blindly copying the impulse from contact array element 0 of the previous frame into current element 0 can produce a spurious torque. A stable matcher uses feature identity and an explicitly bounded geometric fallback; it must invalidate stale data when topology, body identity or contact orientation changes.

**Memory representation.** One possible compact output type is a struct-of-arrays manifold for batch processing, or an array-of-structs for a small engine. In both cases, each contact row must own its normal (or reference a validated manifold normal), point, signed separation, feature key, and an optional cached impulse whose owner and frame lifetime are explicit. An indexed collider ID alone is unsafe if slots are reused: pair it with a generation. The independent \`tools/labs/contact_cache_lab.cpp\` models canonical pair/feature identifiers and normal/tangent projection, but it assumes some other narrow-phase algorithm has already generated those IDs. Do not mistake a cache policy witness for a complete polygon contact generator.

#### Why continuous collision detection requires motion geometry

A discrete overlap check only sees the shapes at chosen times. A fast disk may pass through a thin object and end the frame on the other side with no overlap. Treat the candidate time interval $[0,h]$ as a geometric dimension. With constant relative velocity $\mathbf v$ and initial relative centre $\mathbf p$, two disks touch whenever

$
\|\mathbf p+t\mathbf v\|^2=(r_A+r_B)^2.
$

This is a quadratic in time. Let $a=\mathbf v\cdot\mathbf v$, $b=2\mathbf p\cdot\mathbf v$, and $c=\mathbf p\cdot\mathbf p-(r_A+r_B)^2$. When initially separated ($c>0$), initially approaching ($b<0$) and $D=b^2-4ac\ge0$, the earliest nonnegative root must still be checked against $h$. A stable entry-root formulation for this branch uses $q=-\frac12(b-\sqrt D)$ and $t=c/q$. The point of this algebra is not sophistication for its own sake: computing a root by subtracting nearly equal floating-point numbers can erase the very short time interval that CCD is supposed to detect. Initial overlap is a separate status; it is not a valid instruction to divide by a time or silently push objects apart.

The exact first contact normal is the normalized relative displacement $\mathbf p+t\mathbf v$, provided it is nonzero. For two radius-1 disks, one fixed at the origin and one starting at $(10,0)$ and moving left at 100 m/s, the first contact is at 0.08 s. If the simulation frame is 0.10 s, the physics update must stop at 0.08 s, apply a collision response, and then process the remaining 0.02 s. Reporting the impact at the end of the frame fundamentally changes the trajectory. A grazing root is geometrically valid, although its zero closing normal speed generally means no impulse is required.

#### From swept regions to a broad phase with no missing candidates

Exact collision-time roots are too expensive to run for every pair of a large scene. A broad phase produces a *conservative superset* of collision candidates. For a disk at $\mathbf x$ with constant velocity $\mathbf v$, radius $r$, and horizon $h$, its centre traverses the segment from $\mathbf x$ to $\mathbf x+h\mathbf v$. A rectangle containing every possible disk position is

$
x_{\min}=\min(x_0,x_0+hv_x)-r,\qquad
x_{\max}=\max(x_0,x_0+hv_x)+r,
$

with analogous y bounds. The segment plus disk is geometrically a capsule; the swept axis-aligned bounding rectangle contains the capsule and is cheaper to test. The bounds should be conservatively rounded outward in a finite-precision implementation, and nonfinite endpoints must be rejected. For accelerating or rotating objects, these linear endpoint bounds are not automatically conservative; their motion bounds must include acceleration, angular sweeps and shape extent.

A sweep-and-prune implementation sorts all swept rectangles by minimum x, removes any active rectangle with maximum x strictly below the next rectangle's minimum x, and compares y intervals for the active candidates. If two actual disk regions intersect at some common time, that shared point lies inside *both* swept rectangles, so their x and y intervals overlap. Thus every genuine disk collision must survive as a candidate. The converse is false: broad-phase rectangles can overlap even if disks move past different locations at different times. False positives are acceptable because the narrow phase evaluates exact TOI; false negatives are not.

Sorting the bounds costs $O(n\log n)$. A naive active vector can still require $O(n^2)$ interval comparisons in a crowded scene, and a scene with every disk overlapping may truly have $\Theta(n^2)$ candidates. These are complexity facts, not reasons to discard correctness. A production engine might use a dynamic BVH, spatial hashing, grid cells, or incremental sorting based on temporal coherence, but each needs the same no-false-negative contract and an explicit strategy for updating bounds after an impact changes velocity.


#### Proving candidate generation, connecting the collision stages, and debugging misses

The swept-box algorithm above deliberately promises one thing: any true disk collision under its constant-velocity assumptions must appear among the broad-phase pairs. The exact time-of-impact solver promises a different thing: a selected candidate must have a defensible contact time, normal and overlap status. The integration layer then promises a third thing: the first event must occur *before* velocities change, all simultaneous constraints must see a coherent pre-impact state, and the remaining interval must be simulated with the new velocities. Treating these as three separately testable contracts is essential when porting collision work from a scalar CPU reference to a SIMD implementation, a GPU compute pass, or a production game engine.

The input to candidate generation is a frame-owned array of rigid-body snapshots. Every snapshot has a stable identity and a position, velocity and radius measured in consistent units. The broadphase output is a set of **unordered identity pairs**, not borrowed pointers to temporary array slots. In the independent [swept broadphase lab](../../tools/labs/swept_broadphase_lab.cpp), the public result uses sorted array indices to keep the standalone test small; a real engine with moving entities must translate those indices into generation-checked handles before later jobs reorder or destroy scene objects. Stable identity is not just a convenience: if an index changes meaning, the pair may name the wrong bodies even when the geometric intersection arithmetic was correct.

An important failure to resist is testing a broad phase only against its own candidate rules. That would prove little: two incorrect functions could agree. The independent reference disk-time equation instead enumerates each possible unordered pair and checks whether a first contact exists inside the horizon. For each oracle hit, a binary search verifies that the same pair occurs in the sorted broad-phase output. Extra broad-phase pairs are acceptable; an omitted true pair is not. For 35 bodies, there are $35\cdot34/2=595$ distinct pairs; across 200 seeded scenes, the check examines 119,000 unordered pairs. This **is** useful evidence for the stated generator and random domain, but it is not a proof that every floating-point configuration or accelerated/rotating shape is supported. At identical positions and exact edges, the caller also needs a consistent policy for tangent, initial-overlap and nonfinite inputs.

A broader **result-equivalence** test is needed before actually replacing all-pairs discovery in the event loop. Run the same initial world twice, once with the exhaustive pair enumerator and once with the swept broad phase followed by the identical narrow-phase solver. Normalize events by stable pair identity and declared time tolerance; then compare event ordering, contact normals, post-solve velocities, final positions, and conserved quantities. To make this comparison meaningful, freeze the initial scene and random seed, use identical restitution/friction policies and iteration counts, and rebuild swept bounds after every impulse that changes a velocity. Otherwise a broadphase computed at the beginning of the frame can miss a **future** collision created by an earlier impact. A classic three-disk chain demonstrates the failure: disk A strikes a resting B, which only then moves toward disk C. B's first swept box did not include the later collision path because that velocity did not exist until after A struck it.

The full [C++20 event-step witness](../../tools/labs/disk_event_pipeline_lab.cpp) makes the exhaustive version observable before optimization: it scans all pairs, finds the earliest TOI, advances every disk to that event, freezes the incoming restitution targets for a simultaneous batch, iterates accumulated normal impulses, and rescans the remaining interval. The companion [seeded property suite](../../tools/labs/disk_event_property_lab.cpp) supplies 10,000 separate isolated unequal-mass impacts, checking outgoing relative speed, linear momentum, nonincreasing kinetic energy, and exactly one impact event. The tested scene family is *not* general polygon CCD, and an event-based disk solver is not a replacement for persistent resting-contact constraints; the sources and their explicit contract remain part of the present lesson rather than an unexplained future promise.

#### A numerical debugging trace: find the stage that lost the collision

For a fixed radius-1 disk centred at $(0,0)$ and a second radius-1 disk initially at $(10,0)$ moving left at $100\ \mathrm{m/s}$, use a horizon of $0.10$ s. The moving disk's centre segment runs from $x=10$ to $x=0$; its swept x interval is $[-1,11]$, while the stationary disk's is $[-1,1]$. Their swept x and y intervals intersect, so this pair must survive the candidate stage. The narrow phase sees relative position $(10,0)$, velocity $(-100,0)$ and combined radius $2$, producing first entry time $t=0.08$ s and normal $(1,0)$ from stationary A toward moving B. At this event the moving centre is at $x=2$. An ideal elastic impulse against an infinite-mass body reverses its velocity to $+100\ \mathrm{m/s}$; the final $0.02$ s moves its centre to $x=4$.

That trace is an executable debugging contract. If the broadphase rejects the pair, inspect the swept endpoints and interval inequality. If the broadphase admits it but the narrowphase misses, inspect coefficient signs, discriminant, stable-root calculation, time horizon and direction of closing velocity. If TOI is correct but the disk is teleported to $x=0$ before response, the simulation advanced too far. If the final position is not $4$, inspect the normal convention, inverse mass, restitution target and remaining time. This kind of stage-local evidence makes a black-box physics bug reconstructable.

#### Finite precision and the cost of a seemingly harmless comparison

During property-based testing of the integrated disk stepper, a near-tangent post-event pair was sometimes classified as a deep pre-existing overlap. The earlier predicate compared the **squared-distance error** against a radius-scale tolerance. That representation can be poorly matched to the actual error produced when subtracting large world-space positions and advancing centres to an event time. The corrected witness computes the *linear* centre separation $d=\|\mathbf x_B-\mathbf x_A\|$, compares it against the combined radius $R$, and tolerates a small negative gap under an explicit coordinate-scale error budget. It still rejects overlaps too deep for that numerical uncertainty to explain.

A length tolerance scaled by world-coordinate magnitude is an engineering policy, not a universal robust geometric predicate. At coordinates near the floating-point representability limit, adding a small radius to a huge position may produce no change at all. Reducing precision to 32-bit, transforming to a different origin or changing the GPU's fused-operation behavior can change boundary classifications. Therefore a CPU/GPU agreement test should record coordinate origin, scalar precision, units, tolerance, compiler options and any fast-math settings. It should compare exact pair identities and physically meaningful final-state errors, not merely an average frame time or whether an image looks plausible.

#### How to reconstruct this for an engine or GPU backend

Implement the following sequence from the explanation rather than copying a monolithic collision library: store finite body state with generation-safe IDs; construct conservative swept intervals for one declared motion model; sort and produce deterministic unique candidate pairs; independently compute disk TOIs with initial-overlap and grazing outcomes; select the earliest batch; advance **all** bodies to that batch; solve a fixed-restition-target impulse system; reduce remaining time; rebuild future bounds after velocity changes; and compare the results against the exhaustive CPU oracle. The independent source laboratories above provide ready-to-run unit witnesses while the Game Engine continuous volume develops lifetime, scheduling, ownership and production integration. General convex-shape CCD and rotating bodies require new geometry and bounds, not a claim that the disk oracle already covers them.

**Completion checks:** explain the set-inclusion proof for swept bounds without referring to code; reproduce $0.08$ s and final $x=4$ by hand; demonstrate that a broadphase false positive cannot corrupt a narrow-phase result if it is checked; construct a three-disk rescheduling test; locate a deliberately injected wrong normal sign from the first differing intermediate variable; and state the evidence that would be needed to claim that a GPU port has no false negatives.

#### Executable segment clipping and feature geometry

Recovering a manifold from a polygon collision begins with actual half-space clipping, not an unexplained contact-point array. This independent C++20 fragment takes an incident segment and retains its part inside one plane, with a consistent convention that nonpositive signed distance is inside. Apply the same operation against both side planes of the reference face and finally reject points whose signed distance from the reference surface violates the chosen contact margin. This implementation returns 0, 1 or 2 points; the caller must attach incident and reference feature identifiers before reordering or caching contacts.

```cpp
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

struct Vec2 { double x{}, y{}; };
Vec2 operator+(Vec2 a, Vec2 b) { return {a.x+b.x,a.y+b.y}; }
Vec2 operator-(Vec2 a, Vec2 b) { return {a.x-b.x,a.y-b.y}; }
Vec2 operator*(Vec2 a, double t) { return {a.x*t,a.y*t}; }
double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }

// Keep p if dot(outwardNormal, p-planePoint) <= 0.
// Input is precisely two segment endpoints, not an arbitrary polygon.
std::vector<Vec2> clipSegment(Vec2 a, Vec2 b,
                              Vec2 planePoint, Vec2 outwardNormal) {
    const double da=dot(outwardNormal,a-planePoint);
    const double db=dot(outwardNormal,b-planePoint);
    if (!std::isfinite(da)||!std::isfinite(db))
        throw std::invalid_argument("invalid plane or segment");
    const bool insideA=(da<=0),insideB=(db<=0);
    if (insideA&&insideB) return {a,b};
    if (!insideA&&!insideB) return {};
    // Opposite classifications imply a nonzero signed-distance difference.
    const double denominator=da-db;
    if (denominator==0) throw std::domain_error("ambiguous crossing");
    const double t=da/denominator;
    if (!(t>=0&&t<=1)) throw std::domain_error("crossing outside segment");
    const Vec2 crossing=a+(b-a)*t;
    return insideA?std::vector<Vec2>{a,crossing}
                  :std::vector<Vec2>{crossing,b};
}

int main() {
    const auto inside=clipSegment({-1,0},{1,0},{0,0},{1,0});
    if (inside.size()!=2 || std::abs(inside[1].x)>1e-12)
        throw std::runtime_error("straddling segment regression failed");
    std::cout<<"clipped endpoint: "<<inside[1].x<<"\n";
}
```

For the worked segment from (-1,0) to (+1,0) and a clipping half-space x <= 0, the signed distances are -1 and +1. The crossing interpolation factor is (-1)/(-2)=1/2, giving the new endpoint (0,0) and a surviving segment from (-1,0) to (0,0). The planar clipping result is geometrically local: using it as a full manifold still requires selecting the reference/incident features, handling parallel or degenerate edges, evaluating penetration along the actual reference normal, merging duplicate contact points and giving the surviving contacts stable feature identities. This distinction is essential before attaching cached solver impulses.

#### Full implementation: disk TOI and conservative swept candidate generation

The shortest route from understanding the geometry to implementing it is to inspect two complete CPU witnesses side by side, not to memorize a list of algorithms. First, the disk TOI query takes *only geometry and the time horizon* and returns a first-impact result; it does not mutate body velocities. Second, the swept-AABB broadphase takes disk motion and a horizon and returns *candidate pairs*; it does not assert that those pairs collide. This separation permits the critical oracle relationship: every exact contact pair must be a broad-phase candidate, but candidates may be false positives.

The complete disk query source follows. Its initial-overlap policy and fallback normal are educational decisions; they cannot be generalized without additional shape-specific rules.

```cpp
// C++20 analytic linear-motion CCD for two disks, not a general rigid-body CCD system.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror disk_ccd_lab.cpp -o /tmp/disk_ccd && /tmp/disk_ccd
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
struct Vec2 { double x{},y{}; };
constexpr Vec2 operator+(Vec2 a,Vec2 b){return {a.x+b.x,a.y+b.y};}
constexpr Vec2 operator-(Vec2 a,Vec2 b){return {a.x-b.x,a.y-b.y};}
constexpr Vec2 operator*(Vec2 a,double t){return {a.x*t,a.y*t};}
constexpr double dot(Vec2 a,Vec2 b){return a.x*b.x+a.y*b.y;}
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
void near(double a,double b,double eps=1e-10){check(std::isfinite(a)&&std::abs(a-b)<eps,"numeric result mismatch");}
struct Disk{Vec2 center{},velocity{};double radius{};};
struct Hit{double time{};Vec2 normal{};bool initiallyOverlapping{};};
void validate(const Disk& d){
 check(std::isfinite(d.center.x)&&std::isfinite(d.center.y)&&
       std::isfinite(d.velocity.x)&&std::isfinite(d.velocity.y)&&
       std::isfinite(d.radius)&&d.radius>=0,"invalid disk");
}
// Relative separation p(t) = p0 + t*v, with disk radius sum R.
// Solve |p(t)|^2 = R^2, or a*t^2 + b*t + c = 0.
std::optional<Hit> first_disk_contact(const Disk& A,const Disk& B,double dt){
 validate(A);validate(B);
 check(std::isfinite(dt)&&dt>=0,"invalid frame duration");
 const Vec2 p=B.center-A.center, v=B.velocity-A.velocity;
 const double R=A.radius+B.radius;
 const double a=dot(v,v), b=2*dot(p,v), c=dot(p,p)-R*R;
 check(std::isfinite(R)&&std::isfinite(a)&&std::isfinite(b)&&std::isfinite(c),
       "out-of-range relative geometry");
 auto normalAt=[&](double t){
   const Vec2 sep=p+v*t;
   const double d=std::hypot(sep.x,sep.y);
   if(d>0)return sep*(1.0/d);
   const double speed=std::hypot(v.x,v.y);
   // Deterministic fallback for coincident centres; no unique geometric normal exists.
   return speed>0?v*(-1.0/speed):Vec2{1,0};
 };
 if(c<=0)return Hit{0,normalAt(0),c<0}; // existing overlap handled by another stage
 if(a==0 || b>=0)return std::nullopt; // no relative motion or not closing
 // fma avoids one rounding in the discriminant when supported by the platform.
 const double discriminant=std::fma(b,b,-4*a*c);
 if(!(discriminant>=0))return std::nullopt; // includes NaN rejection
 const double root=std::sqrt(discriminant);
 // Numerically stable quadratic roots. Here b<0, so q is nonzero and positive.
 const double q=-0.5*(b- root);
 if(!(q>0))return std::nullopt;
 const double nearRoot=c/q;
 const double farRoot=q/a;
 const double t=std::min(nearRoot,farRoot);
 if(!(std::isfinite(t)&&t>=0&&t<=dt))return std::nullopt;
 return Hit{t,normalAt(t),false};
}
int main(){
 const Disk still{{0,0},{0,0},1},bullet{{10,0},{-100,0},1};
 auto h=first_disk_contact(still,bullet,0.1);
 check(h.has_value(),"tunneling hit missed");near(h->time,0.08);near(h->normal.x,1);
 check(!first_disk_contact(still,bullet,0.05),"hit beyond frame accepted");
 check(!first_disk_contact(still,Disk{{10,0},{100,0},1},1),"separating disk hit");
 check(!first_disk_contact(still,Disk{{10,2.1},{-100,0},1},1),"near miss hit");
 h=first_disk_contact(still,Disk{{10,2},{-10,0},1},2);
 check(h.has_value(),"grazing hit missed");near(h->time,1);near(h->normal.y,1);
 h=first_disk_contact(still,Disk{{1,0},{0,0},1},1);
 check(h.has_value()&&h->initiallyOverlapping,"overlap policy broken");near(h->time,0);
 h=first_disk_contact(still,Disk{{0,0},{0,0},1},0);
 check(h.has_value(),"coincident overlap missed");near(h->normal.x,1);
 // Relative motion is invariant under a common uniform Galilean velocity.
 h=first_disk_contact(Disk{{0,0},{55,7},1},Disk{{10,0},{-45,7},1},0.1);
 check(h.has_value(),"relative velocity test failed");near(h->time,0.08);
 // Large coordinate and velocity magnitudes exercise the stable-root path.
 h=first_disk_contact(Disk{{0,0},{0,0},1},Disk{{1000000,0},{-1000000,0},1},2);
 check(h.has_value(),"large-scale impact missed");near(h->time,0.999998,1e-8);
 bool rejected=false;
 try{(void)first_disk_contact(still,bullet,-1);}catch(const std::runtime_error&){rejected=true;}
 check(rejected,"negative timestep accepted");
 rejected=false;
 try{(void)first_disk_contact(still,Disk{{10,0},{-1,0},-1},1);}catch(const std::runtime_error&){rejected=true;}
 check(rejected,"negative radius accepted");
 std::cout<<"disk CCD: tunneling, grazing, overlap, misses, invariance, validation PASS\n";
}
```

Now implement the broad phase that bounds every disk over the same motion horizon. Notice where bounds are conservatively rounded outward, which pairs are removed from the active set, why a y-interval check remains necessary after x sorting, and why output pairs are sorted before later processing. The seeded test compares against a separate disk-impact oracle; it does not pretend that a false-positive AABB pair is a collision.

```cpp
// C++20 swept disk-AABB broadphase witness: conservative candidates, never final collisions.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror swept_broadphase_lab.cpp -o /tmp/swept_broadphase && /tmp/swept_broadphase
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

struct Vec { double x{},y{}; };
struct Disk { std::uint64_t id{}; Vec p{},v{}; double radius{}; };
struct Box { double minX{},maxX{},minY{},maxY{}; std::size_t index{}; };
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
void validate(const Disk& d){
 require(std::isfinite(d.p.x)&&std::isfinite(d.p.y)&&std::isfinite(d.v.x)&&
         std::isfinite(d.v.y)&&std::isfinite(d.radius)&&d.radius>0,"invalid disk");
}
Box swept_box(const Disk& d,double dt,std::size_t index){
 validate(d);require(std::isfinite(dt)&&dt>=0,"invalid time");
 const double ex=std::fma(d.v.x,dt,d.p.x),ey=std::fma(d.v.y,dt,d.p.y);
 require(std::isfinite(ex)&&std::isfinite(ey),"nonfinite endpoint");
 const double negInf=-std::numeric_limits<double>::infinity();
 const double posInf=std::numeric_limits<double>::infinity();
 // Outward-round the conservative bounds to avoid loss at an exact boundary.
 const Box b{std::nextafter(std::min(d.p.x,ex)-d.radius,negInf),
             std::nextafter(std::max(d.p.x,ex)+d.radius,posInf),
             std::nextafter(std::min(d.p.y,ey)-d.radius,negInf),
             std::nextafter(std::max(d.p.y,ey)+d.radius,posInf),index};
 require(std::isfinite(b.minX)&&std::isfinite(b.maxX)&&
         std::isfinite(b.minY)&&std::isfinite(b.maxY),"nonfinite swept AABB");
 return b;
}
using Pair=std::pair<std::size_t,std::size_t>;
std::vector<Pair> broadphase(const std::vector<Disk>& disks,double dt){
 std::vector<Box> boxes;
 boxes.reserve(disks.size());
 for(std::size_t i=0;i<disks.size();++i) {
  for(std::size_t j=0;j<i;++j)require(disks[i].id!=disks[j].id,"duplicate disk id");
  boxes.push_back(swept_box(disks[i],dt,i));
 }
 std::sort(boxes.begin(),boxes.end(),[&](const Box& a,const Box& b){
  if(a.minX!=b.minX)return a.minX<b.minX;
  return disks[a.index].id<disks[b.index].id;
 });
 std::vector<Box> active;
 std::vector<Pair> pairs;
 for(const Box& box:boxes){
  // Everything ending strictly before this minimum X cannot overlap this box.
  active.erase(std::remove_if(active.begin(),active.end(),[&](const Box& a){
   return a.maxX<box.minX;
  }),active.end());
  for(const Box& a:active)if(a.minY<=box.maxY&&box.minY<=a.maxY){
   const auto i=std::min(a.index,box.index),j=std::max(a.index,box.index);
   pairs.emplace_back(i,j);
  }
  active.push_back(box);
 }
 std::sort(pairs.begin(),pairs.end());
 require(std::adjacent_find(pairs.begin(),pairs.end())==pairs.end(),"duplicate candidate");
 return pairs;
}
// Independent geometry-oracle polynomial, *not* using bounding boxes.
bool collides(const Disk& A,const Disk& B,double dt){
 const double px=B.p.x-A.p.x,py=B.p.y-A.p.y;
 const double vx=B.v.x-A.v.x,vy=B.v.y-A.v.y;
 const double R=A.radius+B.radius;
 const double c=px*px+py*py-R*R;
 if(c<=0)return true;
 const double a=vx*vx+vy*vy,b=2*(px*vx+py*vy);
 if(a==0||b>=0)return false;
 const double disc=std::fma(b,b,-4*a*c);
 if(disc<0)return false;
 const double root=std::sqrt(disc),q=-0.5*(b-root);
 return q>0&&c/q<=dt;
}
void verify(const std::vector<Disk>& disks,double dt){
 const auto candidates=broadphase(disks,dt);
 for(std::size_t i=0;i<disks.size();++i)for(std::size_t j=i+1;j<disks.size();++j)
  if(collides(disks[i],disks[j],dt))
   require(std::binary_search(candidates.begin(),candidates.end(),Pair{i,j}),
           "broadphase false negative");
}
int main(){
 {
  const std::vector<Disk> disks{{1,{0,0},{0,0},1},
   {2,{100,0},{-200,0},1},{3,{0,100},{0,0},1}};
  verify(disks,1);
  const auto p=broadphase(disks,1);
  require(p.size()==1&&p[0]==Pair{0,1},"expected one swept pair");
 }
 {
  // All disks widely spaced with zero velocity: no candidates expected.
  std::vector<Disk> disks;
  for(std::uint64_t i=0;i<100;++i)disks.push_back({i+1,{5.0*i,0},{0,0},1});
  require(broadphase(disks,1).empty(),"disjoint scene had candidates");
  verify(disks,1);
 }
 {
  std::mt19937_64 gen(0x5A17u);
  std::uniform_real_distribution<double> coord(-50,50),vel(-25,25),radius(0.2,4);
  for(int trial=0;trial<200;++trial){
   std::vector<Disk> disks;
   for(std::uint64_t i=0;i<35;++i)disks.push_back({i+1,{coord(gen),coord(gen)},
       {vel(gen),vel(gen)},radius(gen)});
   verify(disks,0.1+0.01*(trial%40));
  }
 }
 {
  bool rejected=false;
  try{(void)broadphase({{1,{0,0},{0,0},1},{1,{3,0},{0,0},1}},1);}
  catch(const std::runtime_error&){rejected=true;}
  require(rejected,"duplicate identity accepted");
  rejected=false;
  try{(void)broadphase({{1,{0,0},{0,0},1}},-1);}
  catch(const std::runtime_error&){rejected=true;}
  require(rejected,"negative dt accepted");
 }
 std::cout<<"swept broadphase: deterministic + 200 randomized scenes PASS\n";
}
```

These two programs are distinct translation units, each with its own main function and type definitions. Compile and run **separately**. To compose them into an engine library, extract the geometry types and functions into namespaced headers/modules, define ownership and error-return conventions, and make the broad-phase result feed the narrow-phase query. The Game Engine continuous note contains the fully assembled event-step source, including an optional swept broad phase and the exhaustive reference mode. That compositional step is itself a regression gate, not an assumption that the two standalone executables automatically work as a combined engine.

#### Implementation laboratory and debugging contract

Read and execute the standalone \`tools/labs/disk_ccd_lab.cpp\` first, tracing relative position, velocity, radius sum, discriminant, entry root and contact normal. Next inspect \`tools/labs/swept_broadphase_lab.cpp\`: it builds outward-rounded swept bounds, sorts by x, filters candidate pairs and checks its output against an independently implemented analytic disk TOI oracle across deterministic scenes and 200 seeded scenes of 35 disks. The third lab \`tools/labs/disk_event_pipeline_lab.cpp\` supplies the dynamics side, repeatedly querying earliest contact times and resolving a bounded set of events; the event stepper now exposes an optional swept-candidate mode, while retaining the exhaustive candidate mode as a reference. Its published code path was directly inspected, and the differential comparison laboratory is present; a successful hosted execution of that comparison has not yet been confirmed.

A geometry debug view should draw each disk at start, at TOI and at end; the centre trajectory; swept rectangles; narrow-phase candidate pairs; true impact pairs; contact normal arrows; and feature IDs. Logs should record the chosen contact time, squared discriminant, surface gap before and after, and the reason any pair was rejected. Useful invariants include body-order symmetry with consistent normal reversal, translation invariance within the chosen numerical scale, a true-TOI-pair subset of the broad-phase set, and finite normalized normals. A no-false-negative test is stronger evidence than a screenshot of several colliding disks, but 119,000 seeded pair comparisons cannot certify all floating-point extremes or complex convex geometry.

**Reconstruct rather than memorize:** derive the disk witnesses and sign convention, write the half-space segment clipper with all four inside/outside cases, design stable feature identifiers, derive the TOI quadratic and stable root, then write and validate the swept-AABB candidate loop. Explain which algorithms are exact analytic solutions under constant velocity and which rely on heuristic tolerances or approximations. Finally enumerate what is *not* implemented by these witnesses: reliable face clipping for all convex polytopes, rotating-body CCD, persistent contact production integration, accurate acceleration bounds, and performance of Kairo's live runtime.
<!-- END IMPLEMENTABLE SWEPT COLLISION GEOMETRY 2026-11-12 -->

### Continuous Collision Detection

Original Parts 1263–1270

**Part 1263 — Discrete collision detection**

A conventional rigid-body simulation advances the world through discrete time steps.

Let the simulation timestep be:

$$
\Delta t
$$

At time:

$$
t_n=n\Delta t
$$

the world has body positions, orientations, linear velocities, and angular velocities.

A simplified step may include force integration, position prediction, broadphase updates, narrowphase collision detection, contact generation, iterative constraint solving, and final state updates.

The exact ordering depends on the integration and solver architecture. Some engines solve velocity constraints before integrating positions. Others use predicted positions and position-level corrections.

The defining property of discrete collision detection is that collision geometry is evaluated at selected simulation states rather than continuously over every instant between them.

**Why this works for many ordinary objects**

Suppose a slowly moving sphere approaches a thick wall.

If the sphere moves only a small distance each timestep, it will usually be detected while approaching, touching, or slightly penetrating the wall.

The narrowphase generates a contact, and the solver adjusts motion.

The system can remain stable even though contact is not detected at the exact mathematical instant of first touch.

That is because many practical solvers tolerate small penetration and use constraint stabilization to correct it.

**The limitation is temporal sampling**

Discrete collision detection does not automatically know what happened between sample times.

Suppose the sphere is left of the wall at $t_n$ and right of it at $t_{n+1}$.

If the two sampled configurations are both disjoint, ordinary discrete overlap tests may never report a collision.

This is not a failure of sphere-wall geometry.

It is a failure to inspect the geometry's trajectory through time.

**Why a fixed timestep alone is insufficient**

Using smaller timesteps reduces the distance travelled per step, but it does not give a universal anti-tunnelling guarantee.

A sufficiently fast object or sufficiently thin obstacle can still be crossed between samples.

If the minimum relevant obstacle thickness is $h$, a rough displacement constraint for simple motion might require:

$$
\|v\|\Delta t\ll h
$$

But this is not a complete collision guarantee. It does not account for all relative velocities, rotations, curved paths, or contact geometries.

An engine needs a continuous or conservative temporal collision method when these situations matter.

**Part 1264 — Tunnelling at large timesteps**

Consider two spheres in one dimension.

Sphere A initially has:

$$
C_A(0)=0,\qquad r_A=1
$$

and moves right at:

$$
v_A=10\ \mathrm{m/s}
$$

Sphere B is stationary:

$$
C_B=5,\qquad r_B=1
$$

Let:

$$
\Delta t=1\ \mathrm{s}
$$

At the beginning:

$$
|C_B-C_A(0)|=5
$$

The radius sum is:

$$
r_A+r_B=2
$$

Therefore they do not overlap.

At the end:

$$
C_A(1)=10
$$

The centre separation is again:

$$
|10-5|=5
$$

Again, they do not overlap.

A discrete test at only these two times reports no collision.

The moving object begins and ends outside the target but passes through it during the timestep. Endpoint-only collision checks miss the event.

The first contact actually occurs when the centre separation equals the radius sum:

$$
5-10t=2
$$

Therefore:

$$
\boxed{t=0.3\ \mathrm{s}}
$$

The spheres remain overlapping until the moving sphere exits the opposite side.

That happens when:

$$
10t-5=2
$$

Thus:

$$
\boxed{t=0.7\ \mathrm{s}}
$$

The bodies are disjoint at both endpoints of the one-second step, yet overlap during the interval:

$$
\boxed{t\in[0.3,0.7]}
$$

This example makes tunnelling precise.

The problem is not that the overlap routine is inaccurate. The problem is that its input times omit the collision interval.

**Part 1265 — Why continuous collision detection is needed**

Continuous collision detection, or CCD, evaluates a geometric relationship across a time interval.

The objective is to determine whether moving shapes touch, overlap, or cross during that interval, and often to find the earliest time at which contact occurs.

Several different strategies exist.

Analytic swept tests derive exact equations for simple shapes undergoing simple motion. Sphere-sphere collision under constant linear velocity is an example.

Shape casting moves one shape along a specified trajectory and searches for its first interaction with another shape.

Conservative advancement uses separation distance and motion bounds to advance by intervals that are known to be safe.

Speculative contacts attempt to constrain predicted motion before penetration occurs, without necessarily computing an exact first contact time.

Substepping reduces motion per step and can help, but it does not replace a general CCD guarantee by itself.

These techniques solve related problems but have different cost and accuracy characteristics.

**CCD still requires a broadphase**

Running a precise continuous collision query for every pair of objects would be expensive.

A scene with $N$ objects contains:

$$
\frac{N(N-1)}2
$$

possible unordered pairs.

Therefore the broadphase should identify pairs whose swept bounds may overlap during the timestep.

For an object moving linearly with fixed orientation, a simple swept AABB can be constructed by merging its initial and final AABBs.

For pure translation:

$$
\boxed{ B_{\mathrm{swept}} = \operatorname{AABB}(B(t_0)\cup B(t_1)) }
$$

This bound contains all intermediate translated positions of the fixed-orientation shape.

For rotation or deformation, endpoint AABB merging is not generally conservative: intermediate geometry can extend outside both endpoint enclosures. A broader motion bound is required.

**CCD needs integration with the solver**

Suppose a collision occurs at:

$$
t_{\mathrm{TOI}}=0.3\Delta t
$$

The engine should not necessarily integrate the body to the full end-of-step position and then simply report that a collision occurred earlier.

It needs a response policy.

A common approach is to advance to a safe location near the impact, process collision constraints, and then handle the remaining motion according to the simulation architecture.

Multiple impacts in one timestep create additional complexity because resolving one impact changes subsequent trajectories.

CCD is therefore a physics-world scheduling and integration problem as well as a geometry problem.

Box2D documents both time-of-impact calculations and speculative constraints as parts of its collision system. It also distinguishes the computational trade-offs between ordinary continuous collision and more expensive bullet handling.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://box2d.org\&sz=32)

box2d.org+1



**Part 1266 — Swept-sphere collision**

We can derive continuous sphere-sphere collision analytically under constant translational velocity.

Let sphere A have initial centre:

$$
C_A
$$

and velocity:

$$
V_A
$$

Let sphere B have initial centre:

$$
C_B
$$

and velocity:

$$
V_B
$$

At time $t$:

$$
C_A(t)=C_A+V_At
$$

$$
C_B(t)=C_B+V_Bt
$$

The spheres touch when their centre separation equals the sum of radii:

$$
\boxed{ \|C_B(t)-C_A(t)\|=r_A+r_B }
$$

**Convert to relative motion**

Define initial relative displacement:

$$
\boxed{ R_0=C_B-C_A }
$$

and relative velocity:

$$
\boxed{ V=V_B-V_A }
$$

Then:

$$
C_B(t)-C_A(t)=R_0+Vt
$$

Let:

$$
R=r_A+r_B
$$

The contact condition is:

$$
\boxed{ \|R_0+Vt\|^2=R^2 }
$$

Expand:

$$
(R_0+Vt)\cdot(R_0+Vt)=R^2
$$

$$
R_0\cdot R_0+ 2t(R_0\cdot V)+ t^2(V\cdot V)=R^2
$$

Rearrange:

$$
\boxed{ at^2+bt+c=0 }
$$

where:

$$
\boxed{ a=V\cdot V }
$$

$$
\boxed{ b=2R_0\cdot V }
$$

$$
\boxed{ c=R_0\cdot R_0-R^2 }
$$

This is exactly the ray-sphere quadratic from Pass 04, now interpreted in relative-motion space.

**Initial overlap**

If:

$$
c\le0
$$

the spheres are already overlapping or touching.

Therefore the earliest intersection in the requested interval is:

$$
\boxed{t_{\mathrm{TOI}}=0}
$$

The solver may need separate initial-penetration recovery. A time-of-impact algorithm does not automatically solve already existing overlap.

**No relative motion**

If:

$$
a=0
$$

the relative velocity is zero.

The centre separation does not change during the interval.

If the bodies are initially disjoint, they remain disjoint under the constant-velocity model.

**Receding motion**

If initially separated and:

$$
R_0\cdot V\ge0
$$

the squared centre distance is initially nondecreasing.

For constant relative velocity, the minimum of that quadratic over future nonnegative time occurs at or before the initial time.

Thus the spheres cannot collide later under the assumed motion.

**Solve for first impact**

Calculate:

$$
\Delta=b^2-4ac
$$

If:

$$
\Delta<0
$$

there is no real surface contact time.

Otherwise, calculate the two roots.

The earliest valid nonnegative root within the requested timestep is the first impact.

For a physically valid initial separation and approaching constant relative motion, that is:

$$
\boxed{ t_{\mathrm{TOI}}= \frac{-b-\sqrt{\Delta}}{2a} }
$$

provided it lies inside the timestep interval.

A numerical implementation should use stable-root handling when coefficient magnitudes make cancellation significant.

**Recover the impact normal**

At impact:

$$
C_A^*=C_A+V_At_{\mathrm{TOI}}
$$

$$
C_B^*=C_B+V_Bt_{\mathrm{TOI}}
$$

Then:

$$
\boxed{ n=\frac{C_B^*-C_A^*} {\|C_B^*-C_A^*\|} }
$$

for noncoincident impact centres.

The surface contact points are:

$$
\boxed{ P_A=C_A^*+r_An }
$$

$$
\boxed{ P_B=C_B^*-r_Bn }
$$

At ideal tangency, these are equal in exact arithmetic.

This gives contact time, normal, and witnesses.

**Numerical example**

Use our earlier configuration:

$$
C_A=(0,0,0),\quad V_A=(10,0,0),\quad r_A=1
$$

$$
C_B=(5,0,0),\quad V_B=0,\quad r_B=1
$$

Then:

$$
R_0=(5,0,0)
$$

$$
V=(-10,0,0)
$$

and:

$$
R=2
$$

Thus:

$$
a=100
$$

$$
b=2(5)(-10)=-100
$$

$$
c=25-4=21
$$

The discriminant is:

$$
\Delta=(-100)^2-4(100)(21)
$$

$$
\Delta=1600
$$

The roots are:

$$
t=\frac{100\pm40}{200}
$$

giving:

$$
t_0=0.3,\qquad t_1=0.7
$$

The first impact is:

$$
\boxed{t_{\mathrm{TOI}}=0.3\ \mathrm{s}}
$$

This matches the geometric reasoning from Part 1264.

**Motion-model limitations**

The derivation assumes constant linear velocity over the query interval.

If the bodies accelerate, their relative position becomes nonlinear in time.

Under constant relative acceleration:

$$
R(t)=R_0+V_0t+\frac12At^2
$$

The equation:

$$
\|R(t)\|^2=R^2
$$

is generally quartic in $t$.

For arbitrary motion trajectories, the problem can require numerical root finding or interval-based methods.

The ordinary constant-velocity sphere sweep should not be claimed exact under acceleration or arbitrary deformation.

**Part 1267 — Swept-AABB collision**

Suppose an AABB moves along a linear path with fixed world-axis orientation.

We want to determine when it first intersects another AABB.

Rather than repeatedly testing the two boxes at different times, we can reduce the problem to a ray against an expanded box.

This is another Minkowski geometry application.

**Expanding the target box**

Let moving box A have centre:

$$
C_A
$$

and half-extents:

$$
E_A
$$

Let target box B have centre:

$$
C_B
$$

and half-extents:

$$
E_B
$$

The two boxes overlap exactly when their centre displacement lies within the combined half-extents:

$$
\boxed{ |C_{A,i}-C_{B,i}|\le E_{A,i}+E_{B,i} }
$$

for all axes.

Thus we can replace the two-box overlap problem with a point-inside-expanded-box problem.

The expanded target has centre $C_B$ and half-extents:

$$
\boxed{ E_{\mathrm{expanded}}=E_A+E_B }
$$

The moving box's centre becomes the moving point.

**Relative velocity**

Let the boxes have velocities:

$$
V_A,\quad V_B
$$

Define:

$$
\boxed{ V_{\mathrm{rel}}=V_A-V_B }
$$

Instead of moving both objects, hold the expanded target fixed in B-relative coordinates and move A's centre with velocity $V_{\mathrm{rel}}$.

The trajectory is:

$$
\boxed{ P(t)=C_A+V_{\mathrm{rel}}t }
$$

relative to the target's initial frame.

A swept-AABB collision is now equivalent to raycasting this moving centre against the expanded target AABB.

**Use the slab method**

For each axis $i$, the expanded target interval is:

$$
[C_{B,i}-E_{\mathrm{expanded},i}, C_{B,i}+E_{\mathrm{expanded},i}]
$$

Calculate ray entry and exit parameters using the slab method from Part 1210.

The overall entry time is:

$$
\boxed{ t_{\mathrm{enter}}=\max_i t_i^{\mathrm{near}} }
$$

and exit time:

$$
\boxed{ t_{\mathrm{exit}}=\min_i t_i^{\mathrm{far}} }
$$

A collision occurs during the requested interval when the intersection of:

$$
[t_{\mathrm{enter}},t_{\mathrm{exit}}]
$$

and:

$$
[0,\Delta t]
$$

is nonempty.

**Worked example**

Let moving box A have:

$$
C_A=(-5,0,0),\quad E_A=(1,1,1)
$$

with velocity:

$$
V_A=(10,0,0)
$$

Target box B has:

$$
C_B=(0,0,0),\quad E_B=(1,1,1)
$$

and is stationary.

The expanded target has half-extents:

$$
E_{\mathrm{expanded}}=(2,2,2)
$$

The moving point begins at:

$$
x=-5
$$

and moves right at ten units per second.

The expanded box's left boundary is:

$$
x=-2
$$

The entry equation is:

$$
-5+10t=-2
$$

Therefore:

$$
\boxed{ t_{\mathrm{TOI}}=0.3\ \mathrm{s} }
$$

The moving AABB first contacts B at this time.

**Recovering the collision normal**

The entry slab identifies the limiting coordinate axis.

In this example, the moving centre enters the expanded target through its negative X face.

The outward normal of that expanded face is:

$$
(-1,0,0)
$$

But our contact normal convention points from moving A toward B.

Therefore:

$$
\boxed{ n=(1,0,0) }
$$

The entry-face normal and collision A-to-B normal may have opposite signs depending on the chosen relative-motion representation.

The conversion must be explicit.

**Initial overlap and ties**

If the two boxes already overlap at the beginning, the earliest intersection time is zero.

The slab entry parameter may be negative, so an implementation should not mistake it for a future impact.

If the ray enters through two or three slab boundaries at the same time, the collision feature may be an edge or corner, and the normal may not be unique.

A deterministic tie-breaking policy can choose one normal, but an exact geometric contact construction may require richer feature information.

**Rotating boxes**

The expanded-AABB method assumes fixed box orientation relative to the chosen axes.

If the boxes rotate, their shapes in relative space are no longer represented by constant axis-aligned intervals.

Using only initial and final AABBs can miss intermediate rotational excursions.

For rotating OBBs, a more general convex sweep or conservative advancement method is necessary.

**Part 1268 — Time of impact as a geometric event**

Time of impact is the earliest time within the specified motion interval at which the chosen contact condition is satisfied.

For closed solids:

$$
\boxed{ t_{\mathrm{TOI}}= \inf\\{t:A(t)\cap B(t)\ne\varnothing\\} }
$$

This definition includes initial overlap at:

$$
t=0
$$

It also includes tangency.

**Absolute time versus normalized time**

A physics engine may report TOI in seconds or as a fraction of the timestep.

If the timestep begins at $t_0$ and lasts $\Delta t$, define:

$$
\boxed{ \alpha=\frac{t_{\mathrm{TOI}}-t_0}{\Delta t} }
$$

Then:

$$
\alpha\in[0,1]
$$

For example, if:

$$
\Delta t=0.02\ \mathrm{s}
$$

and contact occurs after:

$$
0.006\ \mathrm{s}
$$

then:

$$
\alpha=\frac{0.006}{0.02}=0.3
$$

The API must make the unit convention explicit.

Confusing seconds with normalized fraction can produce severe errors in integration.

**Query outcomes**

A complete TOI query may need to distinguish:

| Status                | Meaning                                          |
| --------------------- | ------------------------------------------------ |
| Initially overlapping | Contact condition already holds at fraction zero |
| Hit                   | First contact found within interval              |
| No hit                | No contact occurs under the motion model         |
| Unresolved            | Algorithm could not establish a reliable result  |
| Iteration limit       | Search budget exhausted                          |
| Invalid input         | Unsupported geometry or motion data              |

These statuses matter because time-of-impact algorithms often use iterative numerical searches.

A returned failure to converge is not necessarily proof that the bodies do not collide.

**Impact ordering**

Suppose a moving sphere encounters two objects during the same frame.

The first TOI might be:

$$
\alpha_1=0.2
$$

and the second:

$$
\alpha_2=0.6
$$

If the first collision changes the sphere's velocity, the second event's original predicted TOI may no longer be valid.

The engine may need to recompute subsequent collision predictions after applying the first impact response.

This makes exact multi-impact scheduling considerably more complicated than performing one swept query.

**TOI and physical response are distinct**

TOI determines when a geometric event occurs.

The physical response depends on restitution, friction, mass, inertia, contact velocity, and constraints.

For example, the same sphere-sphere TOI can lead to bouncing, sticking, or sliding under different material and body-state conditions.

Therefore a CCD routine should not silently encode one universal collision response.

**Part 1269 — Conservative advancement**

We now derive a more general CCD method for convex objects.

Suppose two convex shapes are separated at the current time.

We can calculate their separation distance using GJK:

$$
\boxed{ d(t)=\operatorname{distance}(A(t),B(t)) }
$$

If:

$$
d(t)>0
$$

the shapes are not yet intersecting.

The question is:

How far can we advance time while guaranteeing that the shapes cannot reach contact during the skipped interval?

This is conservative advancement.

**The fundamental Lipschitz bound**

For translational motion, let the two shapes move with constant velocities:

$$
V_A,\quad V_B
$$

Their relative velocity is:

$$
V_{\mathrm{rel}}=V_A-V_B
$$

Define:

$$
\boxed{ L=\|V_{\mathrm{rel}}\| }
$$

Over a time increment $\Delta t$, the relative shape displacement magnitude is:

$$
L\Delta t
$$

The distance between two fixed-orientation convex shapes under relative translation cannot decrease by more than the relative translation magnitude.

Thus:

$$
\boxed{ d(t+\Delta t)\ge d(t)-L\Delta t }
$$

This is a Lipschitz continuity bound.

It does not require the relative motion to point directly toward the current nearest-point normal.

It is conservative for any direction of relative translation.

**Deriving a safe step**

Suppose we want to stay at least a small distance:

$$
\epsilon
$$

away from contact.

Require:

$$
d(t+\Delta t)\ge\epsilon
$$

Using the lower bound:

$$
d(t)-L\Delta t\ge\epsilon
$$

Rearrange:

$$
L\Delta t\le d(t)-\epsilon
$$

Therefore:

$$
\boxed{ \Delta t_{\mathrm{safe}} \le \frac{d(t)-\epsilon}{L} }
$$

when $L>0$ and $d(t)>\epsilon$.

This gives a conservative temporal step.

The algorithm advances by that amount, recomputes distance, and repeats.

**A translation-only reference algorithm**

```

CCDResult conservativeAdvance(
    const ConvexShape& shapeA,
    const ConvexShape& shapeB,
    Vec3 displacementA,
    Vec3 displacementB,
    double distanceSlop)
{
    // Time is normalized over the interval [0, 1].
    double fraction = 0.0;

    // Maximum relative displacement during the whole interval.
    // This gives a conservative speed bound per unit fraction.
    const double speedBound =
        length(displacementA - displacementB);

    constexpr int maxIterations = 80;

    for (int iteration = 0;
         iteration < maxIterations;
         ++iteration)
    {
        // Evaluate each shape at the same current time fraction.
        ConvexShape currentA =
            translate(shapeA, displacementA * fraction);

        ConvexShape currentB =
            translate(shapeB, displacementB * fraction);

        // A distance-capable GJK query, including explicit failure status.
        DistanceResult result =
            gjkDistance(currentA, currentB);

        if (!result.isReliable())
            return CCDResult::unresolved(fraction);

        // We have reached the requested contact tolerance.
        if (result.distance <= distanceSlop)
            return CCDResult::hit(fraction);

        // No relative motion: separation cannot change.
        if (speedBound == 0.0)
            return CCDResult::noHit();

        const double safeStep =
            (result.distance - distanceSlop) / speedBound;

        // Numerical stagnation requires an explicit outcome.
        if (safeStep <= 0.0)
            return CCDResult::unresolved(fraction);

        // If even the maximum remaining relative movement
        // cannot close the gap, no collision occurs this step.
        if (fraction + safeStep > 1.0)
            return CCDResult::noHit();

        fraction += safeStep;
    }

    // Reaching the iteration limit is NOT proof of separation.
    return CCDResult::iterationLimit(fraction);
}

```

This is a simplified API presentation of the executable implementation.

It applies only to constant translational displacement of otherwise fixed-orientation convex shapes. The distance oracle must be reliable enough for the safety bound to be meaningful.

**Worked example**

Take two spheres of radius one.

Sphere A begins at:

$$
C_A=0
$$

and sphere B begins at:

$$
C_B=5
$$

Sphere A moves ten units toward B during the normalized timestep.

The initial separation distance between their surfaces is:

$$
d_0=5-2=3
$$

The relative displacement magnitude over the full step is:

$$
L=10
$$

Ignoring slop temporarily:

$$
\Delta\alpha=\frac3{10}=0.3
$$

Thus the safe advancement reaches the first contact fraction:

$$
\boxed{\alpha=0.3}
$$

With a small positive distance slop, the algorithm stops slightly before the mathematical contact time.

The reference implementation returns approximately:

$$
\alpha=0.299999
$$

for a slop of $10^{-5}$ in this example.

**Directional closing-speed refinement**

The full relative-speed magnitude is conservative but may be unnecessarily restrictive.

Suppose the nearest-point witnesses define a unit normal:

$$
n=\frac{P_B-P_A}{\|P_B-P_A\|}
$$

Under the A-to-B convention, the instantaneous relative normal velocity is:

$$
v_n=(V_B-V_A)\cdot n
$$

The instantaneous closing speed is:

$$
\boxed{ v_{\mathrm{closing}}=-v_n=(V_A-V_B)\cdot n }
$$

When positive, the bodies are approaching along the current normal.

A more aggressive advancement method may use a closing-speed estimate or an upper bound on normal closure.

However, the normal can change as the shapes move.

A method using only the current instantaneous closing speed must justify why it remains a safe bound over the interval.

For pure convex translation, stronger results can be derived using the convexity of the distance function along a linear trajectory, but the implementation still needs reliable closest features and convergence handling.

For general rotation and deformation, an instantaneous projection alone is not a sufficient global motion bound.

**Rotation requires an angular bound**

A rotating body has points moving because of angular velocity.

For a rigid body with angular speed magnitude:

$$
\|\omega\|
$$

and maximum point distance $R$ from its rotation centre, the speed contributed by rotation is bounded by:

$$
\boxed{ v_{\mathrm{angular}}\le\|\omega\|R }
$$

For two bodies, a conservative total relative point-speed bound is:

$$
\boxed{ V_{\max} = \|V_A-V_B\| + \|\omega_A\|R_A + \|\omega_B\|R_B }
$$

under suitable fixed bounds over the interval.

A safe advancement estimate can then use:

$$
\boxed{ \Delta t_{\mathrm{safe}} \le \frac{d-\epsilon}{V_{\max}} }
$$

This includes a bound on how much rotation may close the current separation.

It is conservative, often significantly more conservative than necessary.

For variable angular velocities, accelerations, or complex trajectories, the bound must cover the actual motion throughout the interval.

The broader principle is that CCD must bound the motion of every potentially contacting point, not just the centres of mass.

**Convergence limitations**

Conservative advancement may require many small steps near grazing contact.

If the motion is nearly tangent, the true separation decreases slowly.

If the numerical distance oracle becomes noisy near zero, the algorithm may oscillate or stall.

If the time step becomes smaller than representable progress in the current time parameter, repeatedly adding it may not change the stored fraction.

Therefore production CCD needs explicit progress thresholds, iteration caps, slop policies, and unresolved-result handling.

This is why a mathematically conservative step formula does not automatically create a production-grade CCD engine.

**Part 1270 — Relative motion for continuous collision detection**

Relative motion is the unifying mathematical tool behind many continuous collision algorithms.

If both objects translate with constant velocity, we can subtract their velocities and treat one as stationary.

But if they rotate, deform, or have different motion models, that reduction becomes more complicated.

**Pure translation**

Let:

$$
A(t)=A_0+V_At
$$

$$
B(t)=B_0+V_Bt
$$

Subtract $V_Bt$ from the entire world.

Then B becomes stationary:

$$
B'(t)=B_0
$$

while A moves with:

$$
\boxed{ V_{\mathrm{rel}}=V_A-V_B }
$$

This transformation preserves relative geometry because both shapes receive the same translation of the coordinate frame.

It is the foundation of sphere-sphere CCD and swept-AABB CCD.

**General rigid-body motion**

A rigid body point moves according to:

$$
\boxed{ X(t)=C(t)+R(t)x_{\mathrm{local}} }
$$

where $C(t)$ is translation and $R(t)$ is orientation.

Its instantaneous velocity is:

$$
\boxed{ v_X=v_C+\omega\times r }
$$

where:

$$
r=X-C
$$

For a pair of candidate contact points:

$$
P_A,\quad P_B
$$

their relative velocity is:

$$
\boxed{ v_{\mathrm{rel}}= (V_B+\omega_B\times r_B) - (V_A+\omega_A\times r_A) }
$$

This is the point-level relative velocity relevant to contact.

**Normal approach speed**

With contact normal $n$ from A to B:

$$
\boxed{ v_n=n\cdot v_{\mathrm{rel}} }
$$

If:

$$
v_n<0
$$

the points are approaching along the normal.

If:

$$
v_n>0
$$

they are separating.

This quantity appears both in continuous collision reasoning and in the contact solver.

But the candidate contact points can change as the shapes move, and the normal may rotate with the geometry.

A CCD algorithm therefore cannot assume the initial contact normal and witness points remain valid throughout an arbitrary rigid-body trajectory.

**Relative orientation**

For rigid-body orientations $R_A(t)$ and $R_B(t)$, the relative orientation is:

$$
\boxed{ R_{\mathrm{rel}}(t)=R_B(t)^TR_A(t) }
$$

under the usual orthonormal rotation-matrix convention.

Unlike relative translation, this is generally time dependent even if the angular velocities are constant.

Moreover, angular velocities do not simply subtract as coordinate-free vectors when their components are expressed in different rotating frames.

They must be represented in a common frame or transformed appropriately.

**Why this matters for your physics engine**

Suppose a fast-moving box rotates while travelling toward a narrow obstacle.

A CCD system sweeping only the box centre along a line with a fixed bounding radius may conservatively detect possible contact, but it does not calculate the exact first contact of the rotating box's surface.

The true first contact depends on its changing orientation and support features.

Likewise, if two dynamic objects move, using the moving object's displacement alone can miss interactions that occur because the target itself moves.

The relevant translation is relative motion.

For fully rotational CCD, both trajectories and their pointwise motion bounds must be accounted for.

### KairoPhysicsEngine: Verified Current State

**What is already implemented**

I inspected the current `main` branch of your physics repository rather than assuming that the curriculum describes code already present.

[ConvexCollision.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/ConvexCollision.cppm?utm_source=chatgpt.com)

The existing implementation contains support-mapped convex geometry, a four-element simplex, GJK simplex updates, EPA face expansion, support-gap termination, and witness reconstruction.

[Narrowphase.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Narrowphase.cppm?utm_source=chatgpt.com)

It contains primitive-specific contact generation, OBB SAT, box-box manifold construction, duplicate-contact removal, and reduction to four representative points.

[PhysicsWorld.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/PhysicsWorld.cppm?utm_source=chatgpt.com)

It contains discrete/continuous body modes, sphere-sweep queries, a dynamic-body CCD integration path, contact caching, and impulse restoration.

**Your CCD implementation is more advanced than a basic discrete engine**

`PhysicsWorld::SetBodyCollisionDetectionMode()` allows a body to use continuous collision mode.

`PhysicsWorld::SweepSphere()` exposes a continuous sphere query.

The current world integration code also has `IntegrateDynamicVelocitiesWithCCD()`.

For selected moving colliders, it computes a swept-sphere approximation using the collider's bounding radius, checks potential targets, computes contact fractions, limits motion, and processes corresponding continuous impacts.

For example, the moving proxy radius is calculated differently by shape type.

A sphere uses its radius.

A capsule uses its half-height plus its radius.

A box uses the Euclidean length of its half-extents.

These are bounding-sphere-style approximations for moving shapes, not exact orientation-sensitive swept convex-shape CCD.

**Relative dynamic-body motion**

The inspected CCD integration calculates a target displacement for dynamic target bodies and subtracts it from the moving body's displacement.

That is the relative-motion principle derived in Part 1270.

It also tracks motion fractions and sorts continuous impacts by time of impact.

These are meaningful architectural features.

**Important limitation**

The current CCD path uses a sphere proxy around the moving collider.

This can be conservative for many rigid shapes, but the first contact of the sphere proxy is not necessarily the first contact of the original collider.

For a long, thin rotated box, the enclosing sphere contains substantial empty space.

The sphere may report contact earlier than the actual box geometry would.

This can introduce conservative early stopping.

It is therefore more accurate to describe the existing system as a swept-sphere-proxy CCD path than as exact arbitrary-convex rotational CCD.

**Triangle-mesh sweeps**

The repository also contains `SweepSphereTriangleMesh()`.

It queries candidate triangles using spatial acceleration and iteratively measures distance from the moving sphere centre to triangle closest points.

The routine advances the sweep fraction using separation and travel distance, with a finite iteration cap.

This is closely related to the conservative advancement ideas we derived.

However, the current query has fixed tolerances and an iteration budget. Its numerical and temporal reliability still needs adversarial validation.

**Contact persistence**

The world stores and restores normal/tangent impulses by matching local anchors and normal agreement.

This means your engine already includes an important part of the persistent-contact architecture.

A natural progression is to add stable feature IDs for polyhedral clipping contacts, retain local-anchor matching as a fallback, and measure whether this reduces contact churn and solver iterations in resting stacks.

### The C++23 Reference Lab

**Complete implementation and stress tests**

I created a new standalone source file for this pass, independently of the Kairo repository.

Geometry Pass 05 — C++23 reference implementation

542 lines · Support mapping, GJK distance, EPA, clipping and CCD

Pass 05 — randomized and differential stress tests

89 lines · 6,000 analytic-distance comparisons and 2,000 independent OBB SAT comparisons

The reference implementation includes support maps for spheres, oriented boxes, and convex vertex hulls; Minkowski witness preservation; simplex closest-point optimization by active-face enumeration; a GJK distance query; EPA initialization and polytope expansion; EPA witness reconstruction; reference-face/incident-face clipping for boxes; analytic sphere-sphere TOI; swept-AABB TOI; and translation-only conservative advancement.

The GJK simplex closest-point implementation deliberately enumerates candidate faces instead of using a heavily optimized specialized simplex routine. This makes the mathematical constraints inspectable, but is not the approach I would choose for the final high-throughput KairoPhysicsEngine hot path.

Likewise, the EPA implementation uses an expensive sampled-support tetrahedron search for educational initialization. A production implementation needs a more reliable and efficient seed strategy.

**Compilation**

The main reference can be built using:

```

g++ -std=c++23 -O2 -Wall -Wextra -Wconversion -pedantic \
    geometry_pass05.cpp \
    -o geometry_pass05

./geometry_pass05

```

The separate stress tests can be built using:

```

g++ -std=c++23 -O2 -Wall -Wextra -Wconversion -pedantic \
    geometry_pass05_stress_tests.cpp \
    -o geometry_pass05_stress_tests

./geometry_pass05_stress_tests

```

Both files assume they are in the same directory.

The stress-test source includes the reference source with its built-in `main()` disabled, so it can exercise the same functions without duplicating their implementations.

**Verified results**

37 deterministic reference assertions passed

6,000 analytic-distance comparisons: 0 mismatches

2,000 rotated-OBB versus SAT comparisons: 0 mismatches

UndefinedBehaviorSanitizer reference run passed

The stress tests compare sphere-sphere GJK distances against the analytic centre-distance formula, and axis-aligned box distances against exact interval-gap formulas.

They also compare GJK overlap against a separately implemented 15-axis OBB SAT oracle for randomized rotated boxes.

These checks provide useful independent evidence, but they do not exhaust all adversarial numerical configurations.

**EPA reliability is not yet complete**

In the latest separate seeded stress suite, the educational EPA implementation returned usable results for:

**/ 59**

sampled penetrating AABB pairs

An earlier independently seeded diagnostic series returned 50 of 59.

The different results reflect different sampled geometries, not a contradiction in the test criteria.

The important conclusion is that the educational EPA implementation still has failure configurations.

Possible causes include inability to construct a sufficiently interior initial tetrahedron, geometric degeneracy during expansion, numerical horizon classification, or lack of convergence under the current policy.

The present tests do not isolate which mechanism accounts for every failure.

Therefore the next engineering step is to instrument EPA's failure categories and reproduce each failed configuration.

It would be misleading to regard the passing GJK distance tests as proof that EPA is robust.

### What Full Production Readiness Requires

**GJK and EPA are only one part of the narrowphase**

The complete high-quality collision subsystem needs a reliable numerical policy, correct shape invariants, exact or certified decisions where necessary, and separate geometric and algorithmic failure statuses.

For GJK, important invariants include valid convex support points, nonempty simplices, nonnegative barycentric weights, bounded simplex size, progress toward the origin, and correct treatment of duplicate supports.

For EPA, the polytope must remain closed, convex, consistently oriented, and origin-enclosing. Support expansion must preserve topology. Horizon extraction must be valid. Penetration witnesses should agree with the reported normal and depth within the algorithm's numerical tolerance.

For contact generation, witness points and normals must use one consistent A/B convention. Contact separations and per-point penetration depths must agree with those witnesses. Feature identifiers should remain stable across small movements, where the corresponding geometry remains persistent.

For CCD, motion bounds must actually enclose the swept geometry, distance-query failures must not be interpreted as definite misses, and time-of-impact output must use a clearly defined temporal unit.

These are testable contracts, not merely preferences.

**Performance architecture**

In your Kairo engine, convex support queries are on the hot path.

For sphere, capsule, and box primitives, support calculations can usually avoid allocation.

For a convex hull with $N$ vertices, a naïve support query costs:

$$
O(N)
$$

A hill-climbing or cached-direction support method may improve typical performance for a convex hull with reliable adjacency, but it requires careful handling of degeneracy and convergence.

GJK and EPA both have data-dependent iterations.

That makes batch performance harder to predict than fixed-cost primitive overlap tests.

A reasonable dispatch strategy is to use inexpensive analytic overlap routines for common primitive pairs, specialized SAT where appropriate, and GJK/EPA for general support-mapped convex pairs.

EPA should not be invoked for every broadphase candidate. It is useful only after overlap has been established and penetration information is required.

**Allocation and data layout**

The reference implementation uses `std::vector` where that makes the algorithm easier to inspect.

For a production GJK simplex, a fixed four-element array is preferable because its maximum capacity is known.

For EPA, dynamically expanding vertices and faces may require bounded scratch allocators, reusable per-thread storage, or small-buffer containers.

The choice affects memory traffic, latency, and multi-threaded collision processing.

A multithreaded narrowphase should avoid sharing mutable temporary simplex or EPA buffers between worker threads unless synchronization is explicitly designed.

**Robustness before micro-optimization**

An implementation that is ten percent faster but occasionally returns false separation for overlapping shapes may create severe simulation failures.

A correct algorithm with poor contact-manifold coherence may produce visibly unstable stacks even if its Boolean collision answers are almost always right.

Therefore benchmarking should measure more than average query time.

It should also measure numerical disagreement, unresolved cases, contact-normal discontinuities, manifold churn, cached-impulse reuse, and downstream solver behaviour.

### Research Connection to KairoPhysics, Vulkax, and Deformable Geometry

**Research question: Can contact-work-aware collision processing reduce cost without damaging temporal coherence?**

A promising research direction is to combine what we have now learned about convex collision, persistent manifolds, and conservative advancement with adaptive spatial processing.

A conventional collision pipeline often optimizes one stage in isolation.

For example, it may minimize broadphase candidate count.

But a broadphase representation that produces slightly fewer candidates can still be slower overall if its updates are expensive.

Likewise, a more aggressive narrowphase algorithm may reduce average runtime while increasing contact instability or downstream solver iterations.

This suggests a broader research question:

Can a collision system adapt its query and contact-generation strategy using measured narrowphase work, contact persistence, motion, and numerical uncertainty?

A possible experiment would compare a fixed collision-query strategy against an adaptive strategy that selects among analytic primitive tests, SAT, GJK, cached GJK features, and continuous queries based on the shapes and motion state.

The adaptive strategy would need strict correctness constraints. It should not skip CCD merely because a previous discrete test happened to pass, or discard a contact pair because GJK exhausted its iteration budget.

Useful variables would include contact density, relative speed, angular motion, geometric aspect ratio, shape convexity, support-query cost, and manifold persistence.

Useful metrics would include total collision-pipeline time, candidate-pair count, GJK/EPA iterations, unresolved-query rate, contact cache hit rate, constraint-solver iterations, penetration error, tunnelling frequency, and divergence in a physical-validation benchmark.

The result would be a genuine systems study rather than simply a faster implementation of one primitive query.

It also connects naturally to your work on adaptive broadphase structures and physical validation: the aim is to optimize useful collision work under explicit reliability constraints.

### Reconstruction and Mastery Gate

**Reconstruction problems**

To demonstrate understanding of this pass, derive and implement the following without copying the reference code.

First, prove the Minkowski-difference intersection theorem. Then derive the support mapping identity and explain why the support vertex must retain its original A and B witnesses.

Second, derive the line, triangle, and tetrahedron simplex updates. Explain how their Voronoi-region tests relate to constrained nearest-point optimization, and explain why origin containment by a segment is not necessarily a valid EPA tetrahedron.

Third, derive the GJK distance optimality gap:

$$
g=\|q\|^2-q\cdot S_C(-q)
$$

Explain when the gap certifies convergence in exact arithmetic and why numerical stagnation is a separate outcome.

Fourth, construct an EPA polytope manually from a tetrahedron containing the origin. Identify the closest face, request a support point, determine visible faces, trace the horizon, and rebuild the polytope. Then derive the contact witnesses from the converged face's barycentric weights.

Fifth, take two overlapping oriented boxes. Use SAT to identify a face contact, construct the reference and incident faces, derive all four clipping side planes, and calculate the retained contacts with their individual signed separations.

Sixth, derive an exact sphere-sphere TOI equation from relative motion. Then derive swept AABB from Minkowski expansion and the slab algorithm.

Seventh, prove the conservative advancement bound:

$$
d(t+\Delta t)\ge d(t)-\|V_A-V_B\|\Delta t
$$

for fixed-orientation translational motion, and explain how angular velocity changes the required bound.

Eighth, describe how a persistent contact manifold is matched across frames and why reusing an impulse from the wrong contact feature can create artificial torque.

For a complete implementation defence, instrument the reference lab so that every unresolved GJK or EPA query reports the simplex, support direction, support gap, current face count, and exact failure category.

Then use the failing EPA cases from the stress tests to investigate and repair specific algorithmic weaknesses.

**Final recall sheet**

M075 — GJK and EPA

Minkowski difference:

$$
C=A-B
$$

Intersection equivalence:

$$
\boxed{A\cap B\ne\varnothing\iff0\in C}
$$

Support mapping:

$$
\boxed{S_C(d)=S_A(d)-S_B(-d)}
$$

GJK distance:

$$
\boxed{d(A,B)=\min_{q\in C}\|q\|}
$$

Simplex witness recovery:

$$
\boxed{P_A=\sum_i\lambda_ia_i,\quad P_B=\sum_i\lambda_ib_i}
$$

EPA support gap:

$$
\boxed{g=n\cdot S_C(n)-d_f}
$$

EPA penetration:

$$
\boxed{\mathrm{MTV}=\delta n}
$$

M076 — Contact Manifolds and CCD

Signed contact separation:

$$
\boxed{s=n\cdot(P_B-P_A)}
$$

Per-contact penetration:

$$
\boxed{\delta=\max(0,-s)}
$$

Clipping intersection parameter:

$$
\boxed{t=\frac{s_0}{s_0-s_1}}
$$

Sphere-sphere CCD:

$$
\boxed{\|R_0+Vt\|^2=(r_A+r_B)^2}
$$

Swept AABB:

$$
\boxed{E_{\mathrm{expanded}}=E_A+E_B}
$$

Conservative advancement:

$$
\boxed{\Delta t_{\mathrm{safe}}\le\frac{d-\epsilon}{V_{\max}}}
$$

Rigid-body contact velocity:

$$
\boxed{ v_{\mathrm{rel}}= (V_B+\omega_B\times r_B) - (V_A+\omega_A\times r_A) }
$$


| Unit                                  | Original parts                    | Coverage                                                    |
| ------------------------------------- | --------------------------------- | ----------------------------------------------------------- |
| M075 — GJK, Minkowski difference, EPA | 1240–1254                         | Derivations, implementation, degeneracies, convergence      |
| M076 — Contacts, clipping, CCD, TOI   | 1255–1270                         | Derivations, implementation, motion assumptions, validation |
| Independent C++23 reference           | 542 lines                         | Compiled and deterministic tests passed                     |
| Differential geometry validation      | 8,000 comparisons                 | No mismatches in tested cases                               |
| EPA production robustness             | 59 sampled AABB penetration cases | Incomplete; failed configurations remain                    |
| KairoPhysics repository review        | GJK/EPA, manifold, CCD, caching   | Current code inspected                                      |

Pass 05 has covered all 31 original curriculum parts, 1240–1270. The mathematical and reference-implementation teaching is complete for this pass, while the demonstrated limits of EPA reliability and full rotational CCD remain explicit engineering work rather than silently assumed capabilities.

Next — Pass 06: M077 + M078, Parts 1271–1300. This completes Phase 13 with mesh ray and containment queries, polygon and frustum clipping, scale-aware tolerances, exact versus conservative collision semantics, collision-query APIs, SIMD/SoA batching, fuzz testing, visual debugging, and integration into a layered production collision library.


M077 + M078 | Original Parts 1271–1300 | Phase 13 | C++23 · Geometry · Physics · Rendering

Progress: Passes 01–05 completed as reported. We now cover Pass 06, with Passes 07–18 remaining afterward.

This pass connects the collision-detection mathematics from M073–M076 to a general-purpose engine query system. Earlier, we established how individual shapes intersect, how SAT and GJK determine convex separation, how EPA estimates penetration, and how CCD handles motion. Now the problem becomes considerably more demanding:

How do we query entire geometric scenes, produce reliable geometric answers, and expose those answers through an efficient API that remains correct under numerical error, degeneracy, animation, and large workloads?

The distinction matters. A ray–triangle test is a mathematical primitive. A complete raycasting system is an engineering subsystem with storage, query semantics, acceleration, hit records, ownership, error handling, threading, performance constraints, and testing.

Our coverage follows the original index:

| Parts     | Integrated subject                                                                 |
| --------- | ---------------------------------------------------------------------------------- |
| 1271–1276 | Mesh raycasts, closest/any hit, mesh overlaps, point containment, winding numbers  |
| 1277–1281 | Frustum–sphere/AABB/OBB queries and plane–box classification                       |
| 1282–1284 | Triangle–triangle and 2D segment/line intersections                                |
| 1285–1290 | Polygon clipping, frustum clipping, Cohen–Sutherland and Liang–Barsky              |
| 1291–1295 | Robust tolerances, conservative/exact tests, collision APIs and hit metadata       |
| 1296–1300 | SIMD, SoA, fuzzing, visual debugging and complete layered collision-library design |

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/25 - SAT for Triangle and Convex Polygon Tests|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping|Next chapter]]
