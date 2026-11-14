# 27 — Mesh Queries, Classification and Clipping

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/26 - Contact Features and Manifolds|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections|Next chapter]]

A primitive intersection test and a scene query solve different problems; meshes add containment, clipping and classification questions. The chapter connects these operations while keeping their assumptions visible.

**By the end:** Distinguish primitive tests from mesh-level ray, overlap and classification queries.

**Recall question:** What extra work is required to turn a ray–triangle test into a mesh containment query?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 67890–70958; source-span SHA256 e7a6dba6ac80dd93ac1fc24c080cf25f2d60526c6a27b90ac043ee0164bb3394. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Mesh Ray, Overlap, Containment, Classification and Clipping

**The fundamental distinction: primitive query versus scene query**

Imagine a ray fired from a camera into a scene containing a reconstructed building.

The building contains 1,000,000 triangles.

For one triangle, we already know that an intersection can be determined using a method such as Möller–Trumbore. But a scene raycast has additional responsibilities:

1. Determine which triangles could possibly intersect.
2. Decide whether one hit or every hit is needed.
3. Resolve the closest valid intersection.
4. Return a useful geometric feature and surface normal.
5. Handle instances transformed into world space.
6. Avoid scanning every triangle whenever possible.

These responsibilities belong to different layers of the system.

```
                         WORLD-SPACE QUERY
                    Ray(origin, direction, tMax)
                                |
                                v
                       Scene / Collider Query
                                |
                                v
                     Mesh or Spatial Acceleration
                                |
                   Candidate triangle selection
                                |
                                v
                       Ray–Triangle Narrowphase
                                |
                    Candidate hit(s), barycentrics
                                |
                                v
                      Query Policy / Filtering
                     /         |          \
                    /          |           \
              Any hit      Closest hit      All hits
                   \           |            /
                    \          v           /
                      Hit record / Result
```

The most important architectural observation is that the narrowphase computes whether a candidate intersects, while the query policy decides what answer the caller needs.

That separation allows the same ray–triangle mathematics to support rendering visibility, editor selection, physics queries, mouse picking and sensor simulation.

**Deriving ray–triangle intersection from barycentric coordinates**

We begin with the triangle vertices $A,B,C\in\mathbb R^3$.

A ray is parameterized as

$$
R(t)=O+tD,\qquad t\geq0
$$

where $O$ is the ray origin, $D$ is its nonzero direction, and $t$ is the ray parameter.

A triangle point is

$$
P(u,v)=A+u(B-A)+v(C-A)
$$

subject to

$$
u\geq0,\qquad v\geq0,\qquad u+v\leq1.
$$

The missing barycentric weight is $w=1-u-v$, so the point is also

$$
P=wA+uB+vC.
$$

At the intersection, both parameterizations describe the same point:

$$
O+tD=A+uE_1+vE_2
$$

with

$$
E_1=B-A,\qquad E_2=C-A.
$$

Rearranging,

$$
tD-uE_1-vE_2=A-O.
$$

We could solve this as a $3\times3$ linear system, but the Möller–Trumbore algorithm exploits cross products and scalar triple products to obtain the same result without explicitly inverting a matrix.

Let

$$
P_v=D\times E_2,\qquad \Delta=E_1\cdot P_v.
$$

The determinant $\Delta$ tells us whether the vectors required for the intersection are linearly independent.

If $\Delta=0$ in exact arithmetic, the ray direction and triangle edges do not define a unique intersection solution. This covers parallel/coplanar configurations and degenerate triangles. A numerical implementation needs a better-conditioned rejection test than a universal fixed epsilon.

Set $T_v=O-A$. Using Cramer's rule expressed through scalar triple products gives

$$
u=\frac{T_v\cdot P_v}{\Delta}.
$$

Next define

$$
Q_v=T_v\times E_1.
$$

Then

$$
v=\frac{D\cdot Q_v}{\Delta}
$$

and

$$
t=\frac{E_2\cdot Q_v}{\Delta}.
$$

These are not arbitrary formulas: they are the solution of our original three-dimensional linear system.

The algorithm accepts an intersection only if the barycentric weights lie inside the triangle and $t$ lies in the requested ray interval.

**Canonical C++ implementation**

```

#include <cmath>
#include <optional>

struct Vec3 {
    double x, y, z;

    Vec3 operator-(const Vec3& b) const {
        return {x - b.x, y - b.y, z - b.z};
    }

    Vec3 operator+(const Vec3& b) const {
        return {x + b.x, y + b.y, z + b.z};
    }

    Vec3 operator*(double s) const {
        return {x * s, y * s, z * s};
    }
};

double dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

struct Triangle {
    Vec3 a, b, c;
};

struct Hit {
    double t;
    double u;
    double v;
    Vec3 position;
    Vec3 normal;
};

std::optional<Hit> intersectTriangle(
    const Ray& ray,
    const Triangle& tri,
    double tMin,
    double tMax)
{
    const Vec3 e1 = tri.b - tri.a;
    const Vec3 e2 = tri.c - tri.a;

    // Unnormalized triangle normal.
    const Vec3 n = cross(e1, e2);
    const double n2 = dot(n, n);

    // Reject zero-area triangles in this reference algorithm.
    if (n2 == 0.0)
        return std::nullopt;

    const Vec3 p = cross(ray.direction, e2);
    const double det = dot(e1, p);

    // This exact comparison is only the reference arithmetic
    // decision, not a robust production predicate.
    if (det == 0.0)
        return std::nullopt;

    const double invDet = 1.0 / det;
    const Vec3 s = ray.origin - tri.a;

    const double u = dot(s, p) * invDet;
    if (u < 0.0 || u > 1.0)
        return std::nullopt;

    const Vec3 q = cross(s, e1);

    const double v = dot(ray.direction, q) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return std::nullopt;

    const double t = dot(e2, q) * invDet;
    if (t < tMin || t > tMax)
        return std::nullopt;

    const double invNormalLength = 1.0 / std::sqrt(n2);
    Vec3 normal = n * invNormalLength;

    // Return a face-forward normal as this API's convention.
    // A geometric outward normal would preserve original winding.
    if (dot(normal, ray.direction) > 0.0)
        normal = normal * -1.0;

    return Hit{
        t, u, v,
        ray.origin + ray.direction * t,
        normal
    };
}

```

This code deliberately uses `double` and explicit `tMin`/`tMax` bounds, but it is still a reference implementation, not a complete robust predicate.

Notice four design decisions.

First, the algorithm does not require the ray direction to have unit length. If $D$ is not normalized, $t$ is a parameter, not a world-space distance. The actual Euclidean distance from the ray origin is $t\|D\|$.

Second, `std::optional<Hit>` distinguishes no hit from a successful query. We do not interpret `t == 0` as failure because a ray may genuinely start on a surface.

Third, the returned $u,v$ allow later interpolation of attributes. For example, with texture coordinates $T_A,T_B,T_C$,

$$
T=(1-u-v)T_A+uT_B+vT_C.
$$

The exact same barycentric coefficients can interpolate vertex normals, UV coordinates, colors, or other triangle attributes, although perspective-correct interpolation is a separate issue when interpolating screen-space rasterization results.

Fourth, the normal is made to face the incoming ray. This is useful for certain picking or shading operations but is not necessarily the desired convention for collision response, closed-mesh containment or back-face classification.

A production hit record should distinguish the geometric normal from the potentially oriented response or shading normal.

**Hand-worked ray trace**

Consider the triangle

$$
A=(0,0,0),\quad B=(2,0,0),\quad C=(0,2,0)
$$

and ray

$$
O=(0.5,0.5,3),\quad D=(0,0,-1).
$$

Then

$$
E_1=(2,0,0),\qquad E_2=(0,2,0).
$$

Calculate

$$
P_v=D\times E_2=(2,0,0)
$$

and

$$
\Delta=E_1\cdot P_v=4.
$$

Since $T_v=(0.5,0.5,3)$,

$$
u=\frac{T_v\cdot P_v}{4}=\frac14.
$$

Next,

$$
Q_v=T_v\times E_1=(0,6,-1)
$$

and therefore

$$
v=\frac{D\cdot Q_v}{4}=\frac14.
$$

Finally,

$$
t=\frac{E_2\cdot Q_v}{4}=\frac{12}{4}=3.
$$

The intersection is

$$
R(3)=(0.5,0.5,0).
$$

Its barycentric weights are

$$
(w,u,v)=(0.5,0.25,0.25).
$$

They are nonnegative and sum to one, confirming that the intersection lies inside the triangle.

The triangle normal is $(0,0,1)$, which opposes the ray direction as expected.

This example gives us a mathematical oracle against which to check the implementation.

**Closest-hit, any-hit and all-hit semantics**

A mesh raycast is not one algorithm with one termination condition. Its query semantics determine what work may be skipped.

**Closest-hit query**

A closest-hit query returns the intersection with the smallest valid nonnegative ray parameter.

For $N$ candidate triangles, let their intersection distances be

$$
t_1,t_2,\ldots,t_k.
$$

The required result is

$$
t_{\text{closest}}=\min\\{t_i\mid t_{\min}\leq t_i\leq t_{\max}\\}.
$$

The natural reference implementation maintains the best current distance. Every time it finds a closer hit, that distance becomes the new maximum allowed distance for subsequent candidates.

```

#include <span>
#include <optional>
#include <cstdint>
#include <limits>

struct MeshHit {
    Hit geometricHit;
    std::uint32_t triangleID;
};

// A reference implementation with O(N) triangle tests.
// Triangle IDs are indices into a stable mesh snapshot.
std::optional<MeshHit> raycastMeshClosest(
    const Ray& ray,
    std::span<const Triangle> triangles,
    double maximumDistance)
{
    std::optional<MeshHit> best;
    double closest = maximumDistance;

    for (std::size_t i = 0; i < triangles.size(); ++i) {
        const auto candidate = intersectTriangle(
            ray,
            triangles[i],
            0.0,
            closest
        );

        if (!candidate)
            continue;

        // Deterministically select the lowest triangle ID
        // when two triangles have exactly equal hit parameters.
        if (!best ||
            candidate->t < closest ||
            (candidate->t == closest &&
             i < best->triangleID)) {

            closest = candidate->t;
            best = MeshHit{
                *candidate,
                static_cast<std::uint32_t>(i)
            };
        }
    }

    return best;
}

```

Here, `closest` is not merely an output variable. It is also a pruning bound.

When using a bounding volume hierarchy (BVH), if a node's first possible intersection is farther than `closest`, none of its triangles can improve the result. The entire subtree can be skipped.

However, do not confuse first encountered with nearest. Traversal order does not guarantee distance order unless the algorithm specifically enforces that property.

**Any-hit query**

An any-hit query only asks whether an acceptable intersection exists:

$$
\exists i:\ \operatorname{Intersect}(R,T_i)=\text{true}.
$$

It can terminate immediately on the first accepted hit.

This makes any-hit ideal for binary visibility and occlusion questions, such as whether an opaque object blocks a shadow ray.

```

bool raycastMeshAny(
    const Ray& ray,
    std::span<const Triangle> triangles,
    double maximumDistance)
{
    for (const Triangle& triangle : triangles) {
        if (intersectTriangle(
                ray, triangle, 0.0, maximumDistance)) {
            return true;
        }
    }
    return false;
}

```

The mathematical result is independent of triangle order, provided every accepted intersection satisfies the same predicate. Execution time is not: the first hit might be at the beginning or end of the candidate sequence.

Any-hit is not always equivalent to "first hit, return immediately." A rendering system may reject intersections because the material is transparent or alpha-masked. That requires an acceptance filter; the query must continue when a candidate is rejected.

Ray-tracing APIs therefore often distinguish ray traversal from intersection filtering or occlusion acceptance. Embree has separate intersection and occlusion interfaces, illustrating this architectural distinction in a production ray-tracing system.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://github.com\&sz=32)

GitHub



**All-hit query**

An all-hit query returns all relevant intersections, commonly sorted by distance.

It serves different purposes: penetrations along a sensor ray, editor multi-selection, geometric analysis, and potentially inside/outside testing.

But the result itself needs a contract.

Suppose a ray hits exactly the common edge shared by two triangles. Should this be reported as one geometric intersection or two primitive intersections?

Both are legitimate, depending on the consumer.

An all-hit API should specify whether it returns raw primitive hits or deduplicated surface events. Deduplication must not inadvertently merge distinct surfaces at coincident positions.

**Manual execution trace**

Imagine a ray crossing triangles in the following traversal order:

Three intersections along a ray; the engine's triangle traversal order can differ from geometric distance order.

Suppose traversal processes A, then B, then C. The resulting state evolves as follows:

| Triangle | Candidate $t$ | Closest-hit state | Any-hit state            |
| -------- | ----------------- | ----------------- | ------------------------ |
| A        | 12                | 12                | Returns true immediately |
| B        | 3                 | 3                 | Already terminated       |
| C        | 6                 | Remains 3         | Already terminated       |

Thus closest-hit returns B, even though A was encountered first. Any-hit returns true on A, because the caller does not care which triangle is nearest. All-hit collects distances 12, 3, and 6 and may sort them into 3, 6, 12.

For a flat mesh containing $N$ triangles, worst-case traversal costs $O(N)$ intersection tests. With a BVH, many rays visit far fewer primitives, but performance depends on hierarchy quality, geometry distribution, ray coherence and traversal implementation. A BVH does not make every possible ray query logarithmic.

**How this actually appears in Kairo**

I inspected the current GitHub source rather than treating the curriculum's repository paths as proof of their behavior.

In [KairoGeometry/Intersection.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Intersection.cppm?utm_source=chatgpt.com), the generic `RayHit<T>` type is defined around lines 32–51. `RayTriangle` occupies approximately lines 213–295 and implements Möller–Trumbore. It rejects near-zero determinants, checks barycentric bounds, rejects hits behind the ray origin, and flips the resulting triangle normal when necessary.

The important distinction is that this is a primitive query. It takes one `Triangle<T>`, not an indexed triangle mesh.

Your physics engine supplies the next layers.

[KairoPhysicsEngine/Collider.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Collider.cppm?utm_source=chatgpt.com), lines 124–131, defines:

```

struct TriangleMeshCollider final
{
    std::vector<Vec3f> Vertices;
    std::vector<std::array<std::uint32_t, 3>> Triangles;
    kairo::foundation::spatial::BVH Acceleration;
    float SurfaceThickness = 1.0e-3f;
    bool DoubleSided = true;
};

```

Here we can identify the actual data representation.

`Vertices` owns the local-space vertex coordinates; `Triangles` owns three vertex indices per face; `Acceleration` stores a spatial index over those triangles; `SurfaceThickness` controls a thin collision shell used in contact generation; and `DoubleSided` affects whether raycasting accepts intersections from both triangle orientations.

The arrays and BVH belong to the collider value. That is a materially different ownership design from an API that merely stores a non-owning span into an asset database.

The `ColliderShape` uses `std::variant`, so `TriangleMeshCollider` participates in the same tagged union as sphere, capsule, plane, AABB, box and convex-hull colliders.

**Construction and validation**

In `Collider.cppm`, lines 518–568, the code validates vertex counts, triangle indices, repeated indices, degenerate triangles and finite vertices before constructing a BVH.

For each triangle it constructs an AABB and expands the bounds by the surface thickness:

$$
B_{\min}'=B_{\min}-(h,h,h)
$$

$$
B_{\max}'=B_{\max}+(h,h,h).
$$

Here $h$ is `SurfaceThickness`.

The BVH is constructed with settings including a maximum leaf size of four, maximum depth of 64, and SAH-based construction enabled.

Crucially, those bounds are local-space data. Rigid rotation and translation can move the collider without rebuilding the internal BVH, provided the local vertex geometry remains unchanged.

A limitation worth recording is that the degeneracy threshold is fixed:

```
if (Cross(b - a, c - a).LengthSquared() <= 1.0e-12f)
    throw std::invalid_argument(...);
```

Because the quantity being compared has units of length to the fourth power, this threshold is not invariant under scaling. We will address this in M078.

**World-space ray to mesh-local ray**

`PhysicsWorld.cppm`, lines 2272–2315, contains `RaycastTriangleMesh`.

The code performs a rigid inverse transform:

$$
O_{\text{local}}=R^{-1}(O_{\text{world}}-c)
$$

$$
D_{\text{local}}=R^{-1}D_{\text{world}},
$$

where $R$ is the collider's world rotation and $c$ is its world-space center.

It then calls `kairo::foundation::spatial::Raycast` on the mesh BVH and supplies a callback that intersects the candidate triangle.

After a hit, the normal is rotated back:

$$
n_{\text{world}}=R\,n_{\text{local}}.
$$

The existing path uses rigid rotations. Under general nonuniform scaling, transforming normals would instead require the inverse-transpose of the applicable linear transform, followed by normalization.

This distinction is not cosmetic: an incorrect normal changes the direction of a contact impulse or a reflected ray.

**Actual query semantics**

In [PhysicsWorld.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/PhysicsWorld.cppm?utm_source=chatgpt.com), lines 1059–1148:

- `RaycastAll` gets broadphase candidates, invokes shape-specific narrowphase raycasts, accumulates accepted collider hits, and sorts them by increasing distance, using collider ID as an exact-tie ordering rule.
- `Raycast` calls `RaycastAll` and returns its first element.

Consequently, the current public closest-hit route has the simplicity of code reuse, but it also pays for collection and sorting. A dedicated closest-hit path could instead maintain one best record and prune the remaining candidates using the current best distance.

The inspected `PhysicsRayHit` record, lines 64–73, contains body ID, collider ID, world-space point, normal, distance and trigger status.

It does not include a triangle feature ID or barycentric weights in that public record. That is a concrete API-extension opportunity for triangle selection, surface attribute interpolation, and persistent feature matching.

**Mesh overlap queries**

A mesh raycast asks whether a one-dimensional half-line intersects a surface. A mesh overlap query asks whether a shape occupies any geometry in common with that surface or collision representation.

Examples include a sphere touching a terrain mesh, a capsule penetrating a wall, a collision box intersecting a reconstructed object, and two triangle meshes interpenetrating.

These are not all the same mathematical query.

A closed triangle surface has a two-dimensional boundary in three-dimensional space. It may be interpreted as a solid with an interior, while an open surface has no unambiguous solid interior. In a physics engine, a triangle may instead be artificially thickened to produce a contactable shell.

That choice determines what "overlap" means.

**The triangle–AABB SAT test**

Consider a triangle $T$ and an axis-aligned bounding box $B$.

SAT says that disjoint convex objects can be separated by an axis on which their projection intervals do not overlap. For a triangle and AABB in 3D, a standard complete nondegenerate SAT candidate set has 13 axes:

- The three coordinate axes of the AABB.
- The triangle's face normal.
- Nine cross products between each of the triangle's three edges and the AABB's three coordinate axes.

For an axis $a$, the triangle projection interval is

$$
p_{\min}=\min(a\cdot A,a\cdot B,a\cdot C)
$$

$$
p_{\max}=\max(a\cdot A,a\cdot B,a\cdot C).
$$

For an AABB represented by center $c$ and half-extents $e=(e_x,e_y,e_z)$, its projection interval has center

$$
m=a\cdot c
$$

and radius

$$
r=e_x|a_x|+e_y|a_y|+e_z|a_z|.
$$

Why this radius?

Any box point can be represented as $x=c+\delta$, where $|\delta_i|\le e_i$. Its axis projection is

$$
a\cdot x=a\cdot c+a\cdot\delta.
$$

The largest possible positive contribution is obtained by choosing each $\delta_i$ to have the sign of $a_i$, giving

$$
\max(a\cdot\delta)=\sum_i |a_i|e_i.
$$

Therefore the projection intervals are separated if

$$
p_{\max}\<m-r
$$

or

$$
p_{\min}>m+r.
$$

This explains why the box does not need eight projected corners for every axis.

Your `Intersection.cppm` already contains `intersection_detail::TriangleAABBOverlap` around lines 626–677. It constructs the 13 candidate axes and tests overlap using `OverlapsOnAxis`.

The implementation skips axes whose squared length falls below an epsilon-dependent threshold. That avoids meaningless near-zero axes, but a fixed axis-magnitude threshold can also cause accuracy issues for unusually scaled geometry.

**From SAT to a mesh-overlap query**

For a triangle mesh, we can repeat the primitive test:

$$
\operatorname{OverlapMesh}(S,M) = \bigvee_{i=0}^{N-1}\operatorname{Overlap}(S,T_i).
$$

The logical OR is important. Any overlap is sufficient for a Boolean query, but a contact manifold query cannot stop at the first overlap if it needs multiple contact points.

With acceleration, the pipeline is:

```
Query shape
    |
    v
Conservative world bounds
    |
    v
Convert query bounds into mesh local space
    |
    v
Mesh BVH overlap traversal
    |
    v
Candidate triangle indices
    |
    v
Exact/narrowphase shape–triangle tests
    |
    v
Any overlap / all overlaps / contact manifold
```

The BVH's candidate selection is allowed to generate false positives. It must not exclude a triangle that really intersects the query shape.

**Your mesh collision implementation: thick prisms**

In `KairoPhysicsEngine/Narrowphase.cppm`, lines 643–662, `MakeTriangleCollisionPrism` constructs six vertices by offsetting the three triangle vertices along the surface normal:

$$
A_\pm=A\pm\frac{h}{2}n,\quad B_\pm=B\pm\frac{h}{2}n,\quad C_\pm=C\pm\frac{h}{2}n.
$$

This creates a thin convex triangular prism.

The physics narrowphase can now use convex collision machinery against each prism, including the GJK/EPA-related routines from earlier passes.

For mesh–convex collisions, lines 734 onward, the engine queries the mesh BVH for candidate triangles, constructs candidate prisms, performs convex penetration tests, and selects the deepest reported penetration.

For mesh–mesh collisions, lines 673 onward, both mesh BVHs participate in candidate selection before prism–prism collision tests.

This is a useful production design, but its semantics differ from mathematically exact triangle-surface intersection.

A prism with thickness $h$ produces a nonzero collision volume even when the original triangle has zero thickness. It can also produce contacts that would not exist for infinitely thin surfaces. That is an intentional geometric approximation, not an arithmetic error.

A single deepest contact is also not equivalent to a complete multi-point manifold. In stacking and deformable-contact scenarios, the distinction can influence torque, stability and contact persistence.

**Point containment in a mesh**

Mesh overlap answers whether two surfaces or collision shapes meet. Containment asks a different question:

Given a point $P$, is it inside the volume enclosed by a mesh?

A point may be inside a closed cube without lying on any triangle. Therefore testing a point against individual triangle surfaces cannot answer containment.

**Method A: ray parity**

For a closed, watertight mesh with suitable nondegenerate query handling, cast a ray from $P$ toward infinity.

A ray starting outside crosses the surface an even number of times, while a generic ray starting inside crosses it an odd number of times.

The parity rule is

$$
\operatorname{Inside}(P)=(N_{\text{crossings}}\bmod2)=1.
$$

Take a cube occupying $[-1,1]^3$. A point $P=(0,0,0)$ casting a ray along $+X$ crosses the right face at $x=1$ once, so it is inside.

For $P=(2,0,0)$, the same ray never crosses the cube, so it is outside.

The issue comes at shared edges, vertices, coplanar faces and duplicated triangles. Two adjacent triangles may report the same surface crossing twice. If those raw primitive hits are counted independently, parity can be corrupted.

A robust implementation needs a boundary convention, a watertight ray-intersection predicate, or geometric crossing deduplication.

And if $P$ lies exactly on the mesh boundary, "inside" must be defined separately. Treating boundary as a third classification state is often preferable.

**Method B: generalized winding numbers**

Ray parity discards orientation information. Generalized winding numbers retain the signed angular contribution of every oriented surface element.

For a point $P$ and one oriented triangle $ABC$, define

$$
a=A-P,\quad b=B-P,\quad c=C-P.
$$

The signed solid angle subtended by the triangle can be computed as

$$
\Omega= 2\operatorname{atan2} \left( \det(a,b,c), \|a\|\|b\|\|c\| +(a\cdot b)\|c\| +(b\cdot c)\|a\| +(c\cdot a)\|b\| \right).
$$

Here

$$
\det(a,b,c)=a\cdot(b\times c)
$$

is the scalar triple product. Its sign carries the orientation contribution, while the denominator accounts for relative angles and lengths.

Summing over all triangles yields

$$
W(P)=\frac{1}{4\pi}\sum_{i=1}^{N}\Omega_i(P).
$$

For a consistently outward-oriented, watertight, non-self-intersecting closed mesh, the winding number is approximately one for an interior point and zero outside, with sign depending on the exact orientation convention.

The remarkable advantage is that winding numbers can remain useful for imperfect or open meshes, where inside/outside is no longer a strict topological Boolean. Values can become fractional, representing an orientation-dependent measure rather than an unambiguous solid classification.

This is especially relevant to mesh reconstruction and mesh repair.

The 2018 SIGGRAPH work Fast Winding Numbers for Soups and Clouds develops acceleration techniques for generalized winding-number queries over disconnected and defective geometry.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://doi.org\&sz=32)

doi.org+1



The relationship to Maveb is direct: reconstructed surfaces can contain holes, overlapping fragments, inconsistent winding and imperfect connections. A conventional ray-parity test may fail in exactly those situations. A generalized winding-number field can support volumetric classification and signed-distance construction, but the threshold used to turn a fractional value into a Boolean must be treated as a modeling choice, not a universal theorem.

For a direct implementation, summing every triangle costs $O(N)$ per point. Large voxel grids make that impractical. Hierarchical approximation can accelerate evaluation, trading bounded approximation error and preprocessing cost against query throughput.

**Frustum and plane classification**

A camera frustum is the portion of space admitted by its view-projection inequalities. Before rasterizing an object, a renderer wants to know whether its bounding volume can be rejected.

This is a collision-style query, although its purpose is visibility culling.

**Represent a frustum using six inward-facing planes**

Each plane has equation

$$
d(P)=n\cdot P+c,
$$

where $n$ is an inward-facing unit normal and $c$ is the signed plane offset.

For this convention:

$$
d(P)\ge0
$$

means the point is on the accepted side of the plane.

For a conventional finite frustum with six valid planes, a point lies inside when

$$
\forall i\in\\{1,\ldots,6\\},\quad d_i(P)\ge0.
$$

A bounding shape cannot be rejected merely because its center lies outside a plane. Some other portion of the shape may still be inside.

**Sphere–frustum intersection**

Let the sphere have center $c_s$ and radius $r$.

The smallest signed distance attained by any sphere point along an inward plane normal is

$$
d_{\min}=n\cdot c_s+c-r
$$

for a normalized plane.

The sphere is completely outside that plane if even its nearest accepted-side extent is negative, equivalently:

$$
n\cdot c_s+c<-r.
$$

If any frustum plane satisfies this, reject the sphere.

Otherwise, retain it as potentially visible.

This test is conservative. Passing all six plane tests does not necessarily prove intersection with the frustum, because each plane may be satisfied by a different point of the sphere.

**AABB–plane classification using support vertices**

For an AABB with center $b$ and half-extents $e$, the maximum and minimum plane distances are

$$
d_{\max}=n\cdot b+c+ \sum_{j=1}^3|n_j|e_j
$$

and

$$
d_{\min}=n\cdot b+c- \sum_{j=1}^3|n_j|e_j.
$$

These provide a three-state classification:

$$
\begin{cases} d_{\max}<0 & \text{entire box outside the plane}\\\ d_{\min}>0 & \text{entire box inside the plane}\\\ \text{otherwise} & \text{box intersects the plane boundary}. \end{cases}
$$

The same expressions can be implemented with positive and negative support vertices.

For each axis, choose `Max` when the corresponding plane-normal component is positive, otherwise choose `Min`. That constructs the vertex maximizing $n\cdot x$.

If even that maximizing vertex is outside, the entire box is outside.

In `KairoGeometry/Frustum.cppm`, `IntersectsAABB` at approximately lines 247–270 implements this positive-vertex rejection.

**OBB–plane classification**

An OBB has center $c$, normalized orthogonal axes $u_0,u_1,u_2$, and half-extents $e_0,e_1,e_2$.

Any point in the OBB can be expressed as

$$
x=c+\sum_{i=0}^{2}\alpha_i u_i, \qquad|\alpha_i|\le e_i.
$$

Projecting onto a plane normal $n$ gives

$$
n\cdot x+c_p = n\cdot c+c_p+ \sum_i\alpha_i(n\cdot u_i).
$$

Therefore its support radius along the plane normal is

$$
r=\sum_{i=0}^{2}e_i|n\cdot u_i|.
$$

This yields the rejection test

$$
n\cdot c+c_p<-r.
$$

The derivation is identical to the AABB case; only the box basis changes.

Your `Frustum.cppm` uses this formulation in `IntersectsOBB`, around lines 281–301.

**A worked classification**

Consider the half-space $x\ge0$, represented by

$$
n=(1,0,0),\qquad c=0.
$$

An AABB is centered at $b=(0.3,0,0)$ with half-extents $e=(0.5,0.2,0.2)$.

The center distance is $0.3$, while the projection radius is $0.5$.

Therefore:

$$
d_{\min}=0.3-0.5=-0.2
$$

$$
d_{\max}=0.3+0.5=0.8.
$$

The box straddles the plane. Rejecting it because its minimum $x$ is negative would be incorrect.

Now move the center to $b=(-0.8,0,0)$. Then

$$
d_{\max}=-0.8+0.5=-0.3.
$$

Every point lies outside the accepted half-space, so the box can safely be rejected.

**The conservative-versus-exact distinction**

A critical point: repeated plane rejection is sufficient to prove nonintersection when a separating plane is found, but passing all six tests is not always sufficient to prove geometric intersection.

If an accurate frustum–OBB overlap decision is required, additional separating axes may need to be tested. For two convex polyhedra, the relevant candidates include face normals and cross products of edge directions.

For rendering, conservative frustum tests are frequently preferable: unnecessarily drawing one extra object costs performance, while incorrectly culling a visible object causes a missing object.

That is why the desired error policy depends on the consumer.

**Triangle–triangle intersection**

Triangle–triangle intersection is one of the fundamental operations behind mesh collision, self-intersection detection, geometry validation, boolean mesh operations and reconstruction repair.

Let

$$
T_A=(A_0,A_1,A_2),\qquad T_B=(B_0,B_1,B_2).
$$

The triangles intersect if their geometric sets share at least one point.

That includes intersection through an edge, a vertex, a crossing segment, partial coplanar overlap, or complete containment of one coplanar triangle within another.

**First rejection: triangle planes**

Compute the plane of $T_A$:

$$
n_A=(A_1-A_0)\times(A_2-A_0)
$$

and, for each vertex of $T_B$, evaluate

$$
d_i=n_A\cdot(B_i-A_0).
$$

If all $d_i$ are strictly positive, every vertex of $T_B$ lies on one side of $T_A$'s plane. The triangles cannot intersect.

The same is true if all three distances are strictly negative.

Then repeat this with the plane of $T_B$ against vertices of $T_A$.

Note that these are unnormalized plane evaluations. If you want metric distances, divide by $\|n_A\|$. Robust sign classification does not necessarily require normalization, but numerical tolerance comparisons must account for the magnitude of the unnormalized normal.

Passing both plane tests does not prove intersection. It only tells us that an immediate plane-separation rejection was unsuccessful.

**Noncoplanar triangles**

Two nonparallel triangle planes intersect along a line. Each triangle's intersection with that line is an interval, possibly empty or a single point.

The triangles intersect if those intervals overlap.

One implementation strategy explicitly calculates the plane–triangle intersection intervals. Another tests triangle edges against the opposite triangle. A SAT-based strategy considers triangle face normals and appropriate cross products of edges.

Your current `Intersection.cppm`, `TriangleTriangle` at lines 1732–1823, performs plane-side rejection followed by edge–triangle intersection tests.

This is a reasonable implementation structure, but each component must agree about endpoints, coplanarity and epsilon semantics.

**Coplanar triangles**

If both triangles lie in the same plane, generic 3D ray–triangle routines that reject parallel rays are not sufficient.

The problem should be reduced to 2D.

Choose the coordinate axis corresponding to the largest absolute normal component, and drop that coordinate.

For example, when

$$
|n_z|\ge\max(|n_x|,|n_y|),
$$

project vertices onto the $XY$ plane.

This is a particularly stable projection choice because it avoids projecting the triangle into an almost edge-on view.

The resulting two-dimensional problem can be solved using segment–segment intersections and point-in-triangle containment tests.

Both kinds of tests are necessary. A small triangle may lie entirely inside a larger triangle without any edges crossing.

Your implementation has a dedicated `CoplanarTriangleTriangle` helper around lines 766–896 of `Intersection.cppm`, which is called when the plane-distance checks classify the triangles as coplanar.

**When SAT is preferable**

A triangle is convex, so SAT is applicable.

For nondegenerate noncoplanar triangles, suitable separating axes include the two face normals and the nine cross products between the edges of the two triangles.

However, when the triangles are coplanar, their face normals do not separate the triangles in the plane. The cross products of coplanar edges are also perpendicular to that plane. You must add in-plane edge-normal tests or switch to a 2D polygon-overlap routine.

This is a classic example of why a formula that looks complete under generic assumptions may become insufficient under degeneracy.

A robust triangle–triangle API should also distinguish Boolean overlap from intersection geometry. Returning `true` is much easier than constructing the exact shared segment or coplanar overlap polygon.

**Segment–segment and line–line intersection in 2D**

These tests form the mathematical foundation for polygon clipping, planar topology, silhouette processing, mesh UV operations and coplanar collision handling.

**Derivation using the two-dimensional cross product**

A 2D line can be parameterized as

$$
L_1(t)=P+tr
$$

and another as

$$
L_2(u)=Q+us.
$$

Here $P,Q\in\mathbb R^2$ are origins and $r,s\in\mathbb R^2$ are nonzero directions.

At intersection,

$$
P+tr=Q+us.
$$

Thus

$$
tr-us=Q-P.
$$

The scalar 2D cross product is

$$
a\times b=a_xb_y-a_yb_x.
$$

It represents the signed area of the parallelogram spanned by $a$ and $b$.

Cross both sides of the intersection equation with $s$:

$$
t(r\times s)-u(s\times s)=(Q-P)\times s.
$$

Since $s\times s=0$,

$$
\boxed{ t=\frac{(Q-P)\times s}{r\times s} }
$$

Similarly, crossing with $r$ gives

$$
\boxed{ u=\frac{(Q-P)\times r}{r\times s} }
$$

provided $r\times s\ne0$.

For infinite lines, any finite $t,u$ values are acceptable. For segments parameterized from endpoints, both parameters must lie in $[0,1]$.

The distinction is entirely in the accepted parameter domains.

**Three fundamental cases**

When $r\times s\ne0$, the supporting lines intersect uniquely. The segments intersect only when their parameters are inside both closed intervals.

When $r\times s=0$ but $(Q-P)\times r\ne0$, the supporting lines are parallel and distinct.

When both are zero, they are collinear. There may be no overlap, a single shared endpoint, or a whole interval of overlap.

A function returning only one `Point` cannot faithfully represent all possible segment intersections unless its contract deliberately reduces overlapping segments to one representative point.

**A documented implementation**

```

#include <algorithm>
#include <cmath>

struct Vec2 {
    double x, y;

    Vec2 operator+(Vec2 b) const {
        return {x + b.x, y + b.y};
    }

    Vec2 operator-(Vec2 b) const {
        return {x - b.x, y - b.y};
    }

    Vec2 operator*(double t) const {
        return {x * t, y * t};
    }
};

double cross2(Vec2 a, Vec2 b) {
    return a.x*b.y - a.y*b.x;
}

double dot2(Vec2 a, Vec2 b) {
    return a.x*b.x + a.y*b.y;
}

enum class SegmentHitKind {
    None,
    Point,
    Overlap
};

struct SegmentHit2D {
    SegmentHitKind kind = SegmentHitKind::None;

    // For Point: a == b.
    // For Overlap: [a,b] is the shared segment.
    Vec2 a{};
    Vec2 b{};
};

SegmentHit2D intersectSegments(
    Vec2 p0, Vec2 p1,
    Vec2 q0, Vec2 q1)
{
    const Vec2 r = p1 - p0;
    const Vec2 s = q1 - q0;

    const double rr = dot2(r, r);
    const double ss = dot2(s, s);

    // Degenerate input: both segments are points.
    if (rr == 0.0 && ss == 0.0) {
        if (p0.x == q0.x && p0.y == q0.y)
            return {SegmentHitKind::Point, p0, p0};
        return {};
    }

    // One point against one proper segment.
    if (rr == 0.0) {
        const double u = dot2(p0 - q0, s) / ss;
        if (u >= 0.0 && u <= 1.0 &&
            cross2(p0 - q0, s) == 0.0)
            return {SegmentHitKind::Point, p0, p0};
        return {};
    }

    if (ss == 0.0) {
        const double t = dot2(q0 - p0, r) / rr;
        if (t >= 0.0 && t <= 1.0 &&
            cross2(q0 - p0, r) == 0.0)
            return {SegmentHitKind::Point, q0, q0};
        return {};
    }

    const Vec2 qp = q0 - p0;
    const double denom = cross2(r, s);

    if (denom != 0.0) {
        const double t = cross2(qp, s) / denom;
        const double u = cross2(qp, r) / denom;

        if (t < 0.0 || t > 1.0 ||
            u < 0.0 || u > 1.0)
            return {};

        const Vec2 point = p0 + r * t;
        return {SegmentHitKind::Point, point, point};
    }

    // Parallel supporting lines. Check collinearity.
    if (cross2(qp, r) != 0.0)
        return {};

    // Project the second segment onto the first segment.
    const double t0 = dot2(q0 - p0, r) / rr;
    const double t1 = dot2(q1 - p0, r) / rr;

    const double lo =
        std::max(0.0, std::min(t0, t1));

    const double hi =
        std::min(1.0, std::max(t0, t1));

    if (lo > hi)
        return {};

    const Vec2 a = p0 + r * lo;
    const Vec2 b = p0 + r * hi;

    if (lo == hi)
        return {SegmentHitKind::Point, a, a};

    return {SegmentHitKind::Overlap, a, b};
}

```

This implementation illustrates correct case separation, but its exact floating-point equality comparisons are not robust for arbitrary computed coordinates. The production improvement is to use reliable orientation predicates for the topological decisions.

The mathematical orientation test for three 2D points is

$$
\operatorname{orient2d}(A,B,C) = (B-A)\times(C-A).
$$

A positive result indicates counterclockwise orientation, a negative result clockwise orientation, and zero indicates collinearity.

Using four such orientation tests can establish whether two noncollinear segments properly cross. Collinear cases require additional interval checks.

A robust geometry kernel should calculate the sign of the orientation as reliably as possible before attempting to construct the actual intersection coordinates. We will return to this distinction in M078.

**Polygon clipping against a half-space**

The purpose of clipping is not merely to answer whether two shapes intersect. Clipping constructs the portion of a geometric object that remains inside a permitted region.

Let the clipping half-space be

$$
H=\\{x\mid n\cdot x+c\ge0\\}.
$$

Suppose a polygon edge runs from $A$ to $B$.

Parameterize the edge:

$$
P(t)=A+t(B-A),\qquad 0\le t\le1.
$$

At the clipping-plane boundary,

$$
n\cdot P(t)+c=0.
$$

Substitute the edge equation:

$$
n\cdot A+c+t\,n\cdot(B-A)=0.
$$

Let

$$
d_A=n\cdot A+c,\qquad d_B=n\cdot B+c.
$$

Since $n\cdot(B-A)=d_B-d_A$, we obtain

$$
d_A+t(d_B-d_A)=0.
$$

Therefore

$$
\boxed{t=\frac{d_A}{d_A-d_B}}.
$$

The clipped intersection point is

$$
P=A+t(B-A).
$$

The formula is valid for a genuine boundary-crossing edge. If $d_A=d_B$, the edge is parallel to the clipping boundary or lies on it. That case must be handled separately rather than dividing by zero.

**Sutherland–Hodgman polygon clipping**

Sutherland–Hodgman takes a polygon and repeatedly clips its edges against each half-space.

The critical algorithmic invariant is that the output polygon contains exactly the portion of the input polygon that survives the processed clipping planes, subject to the implementation's boundary convention.

For an edge from the previous vertex $S$ to the current vertex $E$, four cases exist:

| $S$ | $E$ | Output                              |
| ------- | ------- | ----------------------------------- |
| Inside  | Inside  | $E$                             |
| Inside  | Outside | Boundary intersection               |
| Outside | Inside  | Boundary intersection, then $E$ |
| Outside | Outside | Nothing                             |

A compact implementation must process the closing edge from the last vertex back to the first, or the polygon can be incorrectly left open.

```

#include <vector>

struct HalfSpace2 {
    Vec2 normal;
    double offset;

    // Positive or zero means inside.
    double signedValue(Vec2 p) const {
        return dot2(normal, p) + offset;
    }
};

std::vector<Vec2> clipPolygon(
    const std::vector<Vec2>& polygon,
    const HalfSpace2& plane)
{
    std::vector<Vec2> output;

    if (polygon.empty())
        return output;

    output.reserve(polygon.size() + 1);

    Vec2 previous = polygon.back();
    double previousD = plane.signedValue(previous);
    bool previousInside = previousD >= 0.0;

    for (const Vec2 current : polygon) {
        const double currentD = plane.signedValue(current);
        const bool currentInside = currentD >= 0.0;

        // A genuine inside/outside transition requires
        // an intersection with the clipping boundary.
        if (previousInside != currentInside) {
            const double denominator = previousD - currentD;

            // In exact arithmetic this cannot be zero
            // for a strict sign transition.
            const double t = previousD / denominator;

            output.push_back(
                previous + (current - previous) * t
            );
        }

        // An endpoint inside survives clipping.
        if (currentInside)
            output.push_back(current);

        previous = current;
        previousD = currentD;
        previousInside = currentInside;
    }

    return output;
}

```

This code has two key properties. It preserves polygon traversal order, and it generates boundary intersections exactly where an edge changes sides.

It is appropriate as an educational reference for finite valid coordinates. A production implementation should define how it handles vertices exactly on the boundary, nearly coincident vertices, duplicate outputs and zero-area polygons.

**Worked clipping trace**

Take the triangle

$$
A=(-1,0),\quad B=(1,-1),\quad C=(1,1)
$$

and clip it against $x\ge0$.

Vertex A is outside, while B and C are inside.

Along AB, $x$ changes from $-1$ to $+1$, so the intersection parameter is $t=0.5$, producing

$$
I_1=(0,-0.5).
$$

Along CA, the same boundary is crossed at

$$
I_2=(0,0.5).
$$

The resulting polygon is a quadrilateral:

$$
I_1,\ B,\ C,\ I_2.
$$

The significant point is that clipping a triangle may produce more than three vertices.

For a rendering pipeline, that quadrilateral can subsequently be triangulated into two triangles. For contact generation, the clipped polygon can instead represent a contact region.

**Triangle-plane and view-frustum clipping**

We now have everything necessary to clip a triangle against an arbitrary plane.

Compute the signed plane value of each vertex. If all three vertices are inside, keep the triangle unchanged. If all are outside, discard it. Otherwise, clip the three edges using Sutherland–Hodgman, producing three or four vertices in the ordinary nondegenerate cases.

This is also conceptually related to contact manifold clipping from M076. When generating box–box contacts, for example, a reference face's side planes can clip an incident face to recover contact points.

**Why rasterization clipping belongs in homogeneous clip space**

A graphics pipeline transforms a world-space point into a homogeneous clip-space vector:

$$
p_c=M_{\text{projection}}M_{\text{view}}p_w.
$$

Write the resulting coordinates as

$$
p_c=(x_c,y_c,z_c,w_c).
$$

For the conventional Vulkan/Direct3D-style depth clip volume, the homogeneous inequalities include

$$
-w_c\le x_c\le w_c,
$$

$$
-w_c\le y_c\le w_c,
$$

$$
0\le z_c\le w_c.
$$

After clipping, normalized device coordinates are obtained through perspective division:

$$
x_{\text{ndc}}=\frac{x_c}{w_c},\qquad y_{\text{ndc}}=\frac{y_c}{w_c},\qquad z_{\text{ndc}}=\frac{z_c}{w_c}.
$$

The correct operation order is critical:

```
Object-space triangle
          |
          v
World transformation
          |
          v
View transformation
          |
          v
Homogeneous clip-space triangle
          |
          v
Clip against canonical homogeneous half-spaces
          |
          v
New clip-space vertices / triangles
          |
          v
Perspective divide
          |
          v
Viewport transformation
          |
          v
Rasterization
```

Why not divide first and then clip?

Perspective division is nonlinear along an edge. Interpolating between already-divided endpoint coordinates can produce incorrect newly clipped vertex data and complications around $w_c=0$, especially for triangles crossing the camera plane.

Clipping in homogeneous space allows each edge to be interpolated linearly in its homogeneous coordinates before division.

For a clip boundary described by a homogeneous linear form $f(p_c)\ge0$, calculate

$$
t=\frac{f(A_c)}{f(A_c)-f(B_c)}
$$

and

$$
P_c=(1-t)A_c+tB_c.
$$

All attached vertex attributes must follow the clipping interpolation contract. After perspective division, the rasterizer must still apply perspective-correct interpolation where appropriate.

For example, texture coordinates are not generally obtained by affine interpolation of screen-space endpoint UVs; the perspective-correct form involves interpolation of $u/w$, $v/w$, and $1/w$.

Reverse-Z does not eliminate the need for homogeneous clipping. The projection matrix changes the mapping of camera depth into clip depth, but valid clip-volume inequalities still depend on the graphics API and projection convention. An infinite-far projection may also result in a degenerate or redundant far plane, so a fixed six-plane frustum representation needs to account for that configuration.

This connects directly to rendering pipelines in Kairo and Maveb: clipping, culling and depth conventions must all agree or geometry can disappear even when the individual mathematical components look correct.

**Cohen–Sutherland line clipping**

Cohen–Sutherland specializes line-segment clipping to an axis-aligned rectangle.

The ingenious idea is to assign each point a compact outcode describing which boundary regions it lies outside.

For a rectangle

$$
x_{\min}\le x\le x_{\max}
$$

$$
y_{\min}\le y\le y_{\max},
$$

use four bits representing left, right, bottom and top.

A point inside the rectangle has outcode `0000`.

For endpoints with outcodes $c_0,c_1$, two fast cases follow.

If

$$
c_0\lor c_1=0,
$$

both endpoints are inside, so the entire segment is accepted.

If

$$
c_0\land c_1\ne0,
$$

both endpoints are outside the same boundary. The segment cannot enter the rectangle and can immediately be rejected.

Otherwise, the segment is not trivially classified. Select an endpoint outside and intersect the segment with one of the violated boundaries. Replace the outside endpoint and repeat.

This is why the method is useful: it often avoids expensive work for obviously accepted or rejected segments.

It is not a general polygon-clipping algorithm. It is specifically designed around rectangular clipping regions and their outcodes, although related outcode schemes extend to other clipping volumes.

**Liang–Barsky line clipping**

Liang–Barsky attacks the same rectangular segment-clipping problem algebraically.

Rather than repeatedly replacing endpoints, it maintains the interval of acceptable segment parameters.

Let

$$
P(t)=P_0+tD,\qquad D=P_1-P_0,\qquad 0\le t\le1.
$$

Each accepted half-space is

$$
n_i\cdot P(t)+c_i\ge0.
$$

Substituting the segment equation,

$$
n_i\cdot P_0+c_i+t(n_i\cdot D)\ge0.
$$

Define

$$
q_i=n_i\cdot P_0+c_i,\qquad p_i=n_i\cdot D.
$$

Then

$$
q_i+t p_i\ge0.
$$

If $p_i>0$, the segment is moving into the permitted side, giving a lower bound

$$
t\ge-\frac{q_i}{p_i}.
$$

If $p_i<0$, it is moving toward the rejected side, giving an upper bound

$$
t\le-\frac{q_i}{p_i}.
$$

If $p_i=0$, the segment is parallel to that boundary: either the entire segment satisfies it or the entire segment violates it.

Intersect all four resulting parameter intervals with the original interval $[0,1]$.

If the lower bound ever exceeds the upper bound, no accepted portion exists.

**Complete reference implementation**

```

#include <array>
#include <optional>

struct Segment2 {
    Vec2 start;
    Vec2 end;
};

std::optional<Segment2> clipLiangBarsky(
    Segment2 segment,
    double xmin, double xmax,
    double ymin, double ymax)
{
    if (xmin > xmax || ymin > ymax)
        return std::nullopt;

    const Vec2 d = segment.end - segment.start;

    // Inward normal, signed half-space offset.
    const std::array<HalfSpace2, 4> boundaries{{
        {{ 1.0,  0.0}, -xmin}, // x >= xmin
        {{-1.0,  0.0},  xmax}, // x <= xmax
        {{ 0.0,  1.0}, -ymin}, // y >= ymin
        {{ 0.0, -1.0},  ymax}  // y <= ymax
    }};

    double enter = 0.0;
    double exit = 1.0;

    for (const HalfSpace2& boundary : boundaries) {
        const double q =
            boundary.signedValue(segment.start);

        const double p = dot2(boundary.normal, d);

        if (p == 0.0) {
            if (q < 0.0)
                return std::nullopt;
            continue;
        }

        const double t = -q / p;

        if (p > 0.0) {
            // Entering the allowed half-space.
            enter = std::max(enter, t);
        } else {
            // Leaving the allowed half-space.
            exit = std::min(exit, t);
        }

        if (enter > exit)
            return std::nullopt;
    }

    return Segment2{
        segment.start + d * enter,
        segment.start + d * exit
    };
}

```

Suppose the rectangle is $[-1,1]\times[-1,1]$ and the segment travels from $(-2,0)$ to $(3,0)$.

Its direction is $D=(5,0)$.

The left boundary $x\ge-1$ requires

$$
-2+5t\ge-1 \quad\Longrightarrow\quad t\ge0.2.
$$

The right boundary $x\le1$ requires

$$
-2+5t\le1 \quad\Longrightarrow\quad t\le0.6.
$$

The Y boundaries impose no additional restriction because the segment remains at $y=0$.

The final interval is

$$
t\in[0.2,0.6].
$$

The clipped segment endpoints are

$$
P(0.2)=(-1,0),\qquad P(0.6)=(1,0).
$$

Liang–Barsky is particularly attractive when the caller wants the clipped parameter interval directly, as it avoids constructing multiple intermediate segments.

Cohen–Sutherland, Liang–Barsky and Sutherland–Hodgman now form a connected conceptual family: all classify geometry against half-spaces, but they organize and retain intermediate state differently.

That completes the mathematical mechanisms of M077, Parts 1271–1290. We now build the reliability, performance and validation layer that makes these mechanisms suitable for an engine.

### Numerical Robustness, Collision API Design, SIMD, Fuzzing and Library Engineering

Original Parts 1291–1300

M077 established the geometry. M078 addresses the problem that makes geometry libraries notoriously difficult to trust:

A mathematically correct equation does not automatically produce a correct floating-point decision, and a correct primitive does not automatically produce a reliable engine subsystem.

**Robust comparisons near tangency**

Consider two spheres with centers $c_A,c_B$ and radii $r_A,r_B$.

Their mathematical overlap condition is

$$
\|c_B-c_A\|^2\le(r_A+r_B)^2.
$$

Define the signed separation expression

$$
s=\|c_B-c_A\|^2-(r_A+r_B)^2.
$$

Then:

$$
s<0:\text{ penetration},\qquad s=0:\text{ tangency},\qquad s>0:\text{ separation}.
$$

The logic is exact in real arithmetic. In floating-point arithmetic, computing $s$ may involve subtracting two nearly equal quantities.

If both quantities are approximately four and their true difference is extremely small, rounding may erase or reverse the distinction.

This is a condition-sensitive situation, not necessarily a programming bug in the usual sense.

**Error propagates through a predicate**

Suppose a floating-point computation evaluates an expression $f(x)$, producing

$$
\widehat f(x)=f(x)+e,
$$

where $e$ is the numerical evaluation error.

If we have a trustworthy bound

$$
|e|\le E,
$$

the sign can be classified safely when

$$
\widehat f>E
$$

or

$$
\widehat f<-E.
$$

But if

$$
|\widehat f|\le E,
$$

the calculated value is too close to zero for the current arithmetic to certify its sign.

A robust strategy is therefore:

```
Evaluate cheaply in ordinary precision
                |
                v
Estimate a mathematically justified error bound
                |
       +--------+--------+
       |                 |
       v                 v
  Sign is certain    Sign uncertain
       |                 |
       v                 v
 Return decision   Higher precision or
                   adaptive exact predicate
                           |
                           v
                    Certified decision
```

This is fundamentally different from treating every near-zero answer as zero.

The latter changes the geometry. The former seeks to determine the sign of the intended geometric predicate accurately.

Jonathan Shewchuk's adaptive-precision geometric predicates are a foundational example of this strategy. His work provides exact-sign techniques for orientation and related determinant predicates on supported floating-point inputs, performing extra arithmetic when necessary rather than for every ordinary case.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://people.eecs.berkeley.edu\&sz=32)

people.eecs.berkeley.edu+1



**Floating-point precision is relative to magnitude**

For IEEE-754 binary32, the significand has 24 bits of precision, giving machine epsilon

$$
\epsilon_{\text{float}}=2^{-23}\approx1.19209\times10^{-7}.
$$

For binary64,

$$
\epsilon_{\text{double}}=2^{-52}\approx2.22045\times10^{-16}.
$$

These values describe floating-point spacing relative to magnitude, not universal geometric distance tolerances.

For example, binary32 spacing near $10^8$ is eight units.

Therefore, storing positions near $10^8$ in `float` can destroy positional differences smaller than that spacing before any collision algorithm even executes.

If two distinct geometric points have already rounded to the same floating-point representation, switching the intersection calculation to `double` cannot recover their lost original separation.

This is why local-coordinate geometry, origin rebasing and higher-precision world transforms can matter as much as the intersection formula itself.

**Choosing epsilon by geometric scale**

A fixed numerical tolerance such as

```
constexpr float epsilon = 1.0e-6f;
```

has no universal geometric meaning.

Is it a length tolerance, squared-distance tolerance, angular tolerance, determinant threshold, contact slop, or relative rounding budget?

The correct answer depends on what quantity the predicate computes.

**Dimensional analysis**

Consider these expressions:

| Quantity                                                        | Physical units |
| --------------------------------------------------------------- | -------------- |
| Distance $\|A-B\|$                                      | Length $L$ |
| Squared distance $\|A-B\|^2$                            | $L^2$      |
| Triangle cross-product magnitude $\|(B-A)\times(C-A)\|$ | $L^2$      |
| Squared cross-product magnitude                                 | $L^4$      |
| Signed 3D orientation determinant                               | $L^3$      |
| Unit-normal dot product                                         | Dimensionless  |

Applying the same `epsilon` directly to each quantity is dimensionally inconsistent.

A more meaningful length tolerance can have the form

$$
\tau_L=\tau_{\text{absolute}}+ k\epsilon_{\text{machine}}S,
$$

where $S$ is a characteristic length scale, $k$ accounts for arithmetic complexity and error amplification, and $\tau_{\text{absolute}}$ expresses a deliberately chosen application-level length tolerance.

This is a useful engineering model, not a universal rigorous error bound. For a certified predicate, the error bound must be derived for the particular arithmetic expression.

The choice of $S$ matters. Using the magnitude of a huge world-space origin can produce an excessively generous tolerance even when nearby local geometry is well resolved.

**Correctly scaling triangle degeneracy tests**

Let

$$
e_1=B-A,\quad e_2=C-A.
$$

A triangle is geometrically degenerate when

$$
\|e_1\times e_2\|=0.
$$

For nonzero edges, an angular quality measure is

$$
q=\frac{\|e_1\times e_2\|} {\|e_1\|\|e_2\|}.
$$

Because

$$
\|e_1\times e_2\| =\|e_1\|\|e_2\||\sin\theta|,
$$

this normalized quantity equals $|\sin\theta|$.

A scale-independent near-collinearity test can therefore use

$$
q\le\tau_{\text{angular}}.
$$

Equivalently,

$$
\|e_1\times e_2\|^2 \le \tau_{\text{angular}}^2 \|e_1\|^2\|e_2\|^2.
$$

This is useful for detecting numerically ill-conditioned skinny triangles.

However, a triangle being skinny is not the same as being exactly degenerate. An application may reject poor-quality triangles for simulation conditioning while a geometry predicate must still correctly classify them.

An exact geometric-degeneracy predicate and a mesh-quality policy should not be conflated.

**A practical tolerance policy**

```

#include <algorithm>
#include <cmath>
#include <limits>

struct GeometryTolerance {
    // An application-level absolute distance tolerance.
    double absoluteLength = 1.0e-9;

    // Multiplier for relative floating-point error.
    double relativeFactor = 32.0;

    // Dimensionless angular/conditioning threshold.
    // This is a policy choice, not exact degeneracy.
    double angularThreshold = 1.0e-10;

    double lengthTolerance(double scale) const {
        return absoluteLength +
            relativeFactor *
            std::numeric_limits<double>::epsilon() *
            std::abs(scale);
    }

    bool nearlyEqualDistance(
        double a, double b,
        double characteristicLength) const
    {
        return std::abs(a - b) <=
            lengthTolerance(characteristicLength);
    }

    bool nearlyParallel(
        const Vec3& e1,
        const Vec3& e2) const
    {
        const Vec3 crossProduct = cross(e1, e2);

        const double area4 =
            dot(crossProduct, crossProduct);

        const double e1Length2 = dot(e1, e1);
        const double e2Length2 = dot(e2, e2);

        if (e1Length2 == 0.0 || e2Length2 == 0.0)
            return true;

        const double threshold2 =
            angularThreshold * angularThreshold;

        // Compares quantities with the same L^4 units.
        return area4 <=
            threshold2 * e1Length2 * e2Length2;
    }
};

```

There is an additional subtlety. Products such as `threshold2 * e1Length2 * e2Length2` may overflow or underflow at extreme scales. Production-quality code should use scaled arithmetic, checked ranges or a normalized formulation when the input domain is broad.

Also, even a scale-aware epsilon cannot provide certified robustness near an arbitrarily ill-conditioned determinant. When topological consistency is required, filtered or exact-sign predicates are the stronger tool.

**Conservative versus exact collision tests**

The word exact has at least three different meanings in collision engineering.

Geometrically exact means that the algorithm solves the requested problem for the specified ideal shape, rather than a bounding approximation.

Numerically exact can mean computing the sign of a predicate exactly for the represented inputs, even when its coordinates are stored in floating point.

Physically accurate means the geometric and dynamic model resembles the intended physical system sufficiently well. An exact collision test against an artificial thick prism is still testing an approximation of an infinitely thin triangle.

These meanings must not be mixed.

**Conservative culling**

A conservative broadphase must preserve all genuinely overlapping pairs.

Let $O(A,B)$ be the actual overlap predicate and $C(A,B)$ be the candidate predicate.

For conservative broadphase rejection, we require

$$
O(A,B)\Rightarrow C(A,B).
$$

If the shapes really overlap, they must survive broadphase filtering.

The reverse implication is not required.

Thus false-positive candidates are acceptable. False-negative candidates are not.

The same principle applies to visibility culling: a potentially visible object may be retained unnecessarily, but a genuinely visible object should not disappear because of an overly aggressive approximation.

**Conservative numerical tests**

Suppose a box's calculated maximum signed plane distance is $\widehat d_{\max}$ and we know the numerical uncertainty cannot exceed $E$.

We may safely reject the box when

$$
\widehat d_{\max}+E<0.
$$

If we instead reject whenever $\widehat d_{\max}<0$, floating-point error may create a false negative near the boundary.

The difference is that the first decision explicitly includes uncertainty.

A useful API might return three states instead of pretending that every boundary case is certain:

```
enum class GeometricClassification {
    DefinitelyOutside,
    DefinitelyInside,
    BoundaryOrUncertain
};
```

This does not mean every physics query needs three states. A realtime game may deliberately map uncertain cases to overlap and accept extra narrowphase work. A CAD or mesh-boolean library may escalate to adaptive precision.

**Which approach should each subsystem use?**

| Subsystem               | Desired property                         | Typical approach                                           |
| ----------------------- | ---------------------------------------- | ---------------------------------------------------------- |
| Broadphase              | No missed candidates                     | Conservative bounds                                        |
| Frustum culling         | No visible geometry accidentally removed | Conservative rejection                                     |
| Picking                 | Stable nearest selected surface          | Robust ray intersection and deterministic tie policy       |
| Mesh boolean operations | Topological consistency                  | Robust predicates and careful construction                 |
| Physics narrowphase     | Stable contacts at an acceptable cost    | Geometric tests plus controlled tolerance/contact policies |
| CCD                     | No unacceptable tunneling                | Conservative time advancement or robust TOI methods        |
| Mesh repair             | Avoid inconsistent classification        | Adaptive predicates, winding methods and validation        |

A particularly important distinction is between a numerical tolerance and a contact margin.

A contact margin intentionally changes when two bodies begin generating constraints. It is part of the simulation model, not merely a workaround for floating-point error.

Likewise, the `SurfaceThickness` in your triangle-mesh collider deliberately changes its contact geometry. It should not be described as an exact triangle–triangle collision test.

**Designing the collision-query API**

The mathematical algorithm is only one responsibility of the public API.

A good collision-query interface must specify what is being queried, what information is returned, what coordinate space is used, what constitutes a hit, and whether the result remains valid after the scene changes.

Consider the following design.

```

#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <vector>

using ColliderID = std::uint64_t;
using PrimitiveID = std::uint32_t;

enum class RayQueryMode : std::uint8_t {
    Any,
    Closest,
    All
};

enum class FacePolicy : std::uint8_t {
    FrontFaces,
    BackFaces,
    Both
};

enum class BoundaryPolicy : std::uint8_t {
    Inclusive,
    Exclusive
};

enum class QueryAccuracy : std::uint8_t {
    Fast,
    Conservative,
    Robust
};

struct RayQueryOptions {
    RayQueryMode mode = RayQueryMode::Closest;
    FacePolicy faces = FacePolicy::Both;
    BoundaryPolicy boundary = BoundaryPolicy::Inclusive;

    QueryAccuracy accuracy = QueryAccuracy::Conservative;

    // Intersect only colliders whose categories match.
    std::uint32_t layerMask = 0xFFFF'FFFFu;

    // Distances are measured in world units by this API.
    double maximumDistance =
        std::numeric_limits<double>::infinity();

    // Explicit query policy, not a hidden global epsilon.
    GeometryTolerance tolerance{};
};

struct FeatureID {
    enum class Type : std::uint8_t {
        Face, Edge, Vertex, Unknown
    };

    PrimitiveID primitive = 0;
    Type type = Type::Unknown;
    std::uint32_t localFeature = 0;
};

struct CollisionHit {
    ColliderID collider = 0;

    // Feature identity is stable only relative to a
    // particular geometry generation or immutable snapshot.
    FeatureID feature{};

    double rayParameter = 0.0;
    double worldDistance = 0.0;

    Vec3 worldPoint{};
    Vec3 geometricNormal{};
    Vec3 orientedNormal{};

    // Triangle barycentric coordinates.
    // w = 1 - u - v.
    double u = 0.0;
    double v = 0.0;

    bool frontFacing = true;
    bool initialOverlap = false;

    // Allows the caller to reject stale result identities.
    std::uint64_t geometryGeneration = 0;
};

```

This is a proposed API, not a transcription of an existing Kairo type.

Its fields represent specific decisions worth examining.

**Hit parameter versus physical distance**

A ray is represented by

$$
P(t)=O+tD.
$$

If $D$ is normalized, $t$ has the same numerical value as traveled distance.

Otherwise,

$$
\ell=t\|D\|.
$$

A public API must decide whether its maximum distance bounds $\ell$ or $t$.

Your `PhysicsWorld::Raycast` currently normalizes the incoming direction internally, so its reported distance is intended to be measured along the normalized world-space ray.

That prevents different caller direction magnitudes from accidentally changing what `maxDistance` means.

**Geometric normal versus shading normal**

A geometric normal is derived from the surface geometry, typically

$$
n_g=\frac{(B-A)\times(C-A)} {\|(B-A)\times(C-A)\|}.
$$

A shading normal may be interpolated from vertex normals and transformed or modified by a normal map.

The shading normal may not be perpendicular to the actual triangle.

This is why physics contact generation should not blindly use a normal-mapped shading normal as if it defined geometric contact geometry.

For double-sided raycasts, an oriented normal may be flipped to oppose the incoming ray while the geometric normal remains tied to the triangle's winding.

**Feature identifiers**

For triangle meshes, a feature may identify:

- The triangle face being intersected.
- A specific edge.
- A specific vertex.
- A compound or generated contact feature.

When a hit lies on an edge shared by two triangles, feature identity becomes more complicated. A triangle-local edge ID is not automatically the same as a unique mesh-topology edge ID.

If the application requires persistent contacts or stable editor selection, it must define that mapping.

And if the mesh is remeshed, reordered, simplified, or reimported, primitive indices may change.

A stable asset identifier plus a geometry-generation counter can prevent stale hit results from silently referring to unrelated triangles.

**Ownership and thread safety**

A collision query may execute while another thread is modifying the world.

If query traversal reads a BVH while a different thread rebuilds its nodes, ordinary unsynchronized access is a data race and may cause undefined behavior.

Valid designs include immutable geometry snapshots, read–write locks, double-buffered spatial structures, staged structural changes, or epoch-based lifetime management.

Each has different memory and synchronization costs.

A `std::span` into immutable triangle data is efficient only while the referenced data remains alive and unchanged. It does not itself own the geometry.

For engine code, this must be part of the API contract, not an assumption hidden inside the query implementation.

**Error policy**

A query containing NaN or infinity in a direction vector should not silently return an arbitrary geometric answer.

At a public boundary, validate the input domain. A low-level trusted hot path may then rely on previously established preconditions.

The caller also needs to know whether an invalid query throws, returns an error object, triggers an assertion, or returns an empty hit.

Do not silently equate invalid input with a genuine geometric miss.

**SIMD batching**

So far, most query code executes one test at a time.

For workloads involving thousands of simple independent tests, CPU SIMD can evaluate multiple collision candidates per instruction.

SIMD stands for single instruction, multiple data.

Suppose we want to test one query AABB against many candidate AABBs.

For a candidate box $B_i$ and query box $Q$, overlap requires all six interval comparisons:

$$
B_{i,\min x}\le Q_{\max x}, \qquad B_{i,\max x}\ge Q_{\min x}
$$

and similarly for Y and Z.

A scalar implementation performs these comparisons for one candidate before advancing.

A SIMD implementation can load the minimum X coordinates of eight candidates into one AVX register, compare all eight against the same query maximum X, and continue with the other coordinates.

The algorithm is unchanged. The execution width changes.

However, vectorization is effective only if the data layout allows efficient loads.

**Array of Structures versus Structure of Arrays**

An array-of-structures representation stores each box together:

```

struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

std::vector<AABB> boxes;

```

Conceptually, memory looks like:

```
[minX0 minY0 minZ0 maxX0 maxY0 maxZ0]
[minX1 minY1 minZ1 maxX1 maxY1 maxZ1]
[minX2 minY2 minZ2 maxX2 maxY2 maxZ2]
...
```

This is convenient when processing all six coordinates of a single box. But when evaluating eight `minX` values together, they are separated by the rest of each structure.

A structure-of-arrays representation stores each coordinate field contiguously.

```
minX[]: [minX0 minX1 minX2 minX3 minX4 ...]
minY[]: [minY0 minY1 minY2 minY3 minY4 ...]
minZ[]: [minZ0 minZ1 minZ2 minZ3 minZ4 ...]
maxX[]: [maxX0 maxX1 maxX2 maxX3 maxX4 ...]
maxY[]: [maxY0 maxY1 maxY2 maxY3 maxY4 ...]
maxZ[]: [maxZ0 maxZ1 maxZ2 maxZ3 maxZ4 ...]
```

Now an AVX register can load eight candidate minimum X coordinates with one contiguous memory operation.

**A complete SoA representation**

```

#include <vector>
#include <cstddef>
#include <stdexcept>

struct AABBSoA {
    std::vector<float> minX, minY, minZ;
    std::vector<float> maxX, maxY, maxZ;

    std::size_t size() const noexcept {
        return minX.size();
    }

    void add(
        float x0, float y0, float z0,
        float x1, float y1, float z1)
    {
        if (x0 > x1 || y0 > y1 || z0 > z1)
            throw std::invalid_argument("Invalid AABB");

        // In production, reserve capacity for all six
        // arrays before batch insertion. Otherwise a
        // throwing allocation can leave sizes inconsistent.
        minX.push_back(x0);
        minY.push_back(y0);
        minZ.push_back(z0);

        maxX.push_back(x1);
        maxY.push_back(y1);
        maxZ.push_back(z1);
    }

    bool validLayout() const noexcept {
        const std::size_t n = minX.size();

        return minY.size() == n &&
               minZ.size() == n &&
               maxX.size() == n &&
               maxY.size() == n &&
               maxZ.size() == n;
    }
};

```

The code above exposes a real exception-safety limitation: appending to six independent vectors is not transactional. An engine implementation could reserve enough capacity first, use a size-controlled batch builder, or adopt fixed-capacity/storage strategies that preserve the equal-length invariant.

This is an example of why data-oriented optimization cannot ignore ordinary C++ lifetime and exception behavior.

**AVX2 implementation with scalar fallback**

The following reference takes one query AABB and tests it against all boxes in a valid SoA buffer. It uses unaligned loads, handles incomplete vector tails, and treats boundary touching as overlap.

```

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cmath>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

struct QueryBox {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

bool overlapScalar(
    const QueryBox& query,
    const AABBSoA& boxes,
    std::size_t i)
{
    return boxes.minX[i] <= query.maxX &&
           boxes.maxX[i] >= query.minX &&
           boxes.minY[i] <= query.maxY &&
           boxes.maxY[i] >= query.minY &&
           boxes.minZ[i] <= query.maxZ &&
           boxes.maxZ[i] >= query.minZ;
}

std::vector<std::size_t> queryOverlaps(
    const QueryBox& query,
    const AABBSoA& boxes)
{
    if (!boxes.validLayout())
        throw std::invalid_argument("SoA size mismatch");

    const auto finite = [](float x) {
        return std::isfinite(x);
    };

    if (!finite(query.minX) || !finite(query.minY) ||
        !finite(query.minZ) || !finite(query.maxX) ||
        !finite(query.maxY) || !finite(query.maxZ) ||
        query.minX > query.maxX ||
        query.minY > query.maxY ||
        query.minZ > query.maxZ)
        throw std::invalid_argument("Invalid query AABB");

    std::vector<std::size_t> result;
    std::size_t i = 0;
    const std::size_t n = boxes.size();

#if defined(__AVX2__)
    // Broadcast the query coordinates across 8 lanes.
    const __m256 qMinX = _mm256_set1_ps(query.minX);
    const __m256 qMinY = _mm256_set1_ps(query.minY);
    const __m256 qMinZ = _mm256_set1_ps(query.minZ);

    const __m256 qMaxX = _mm256_set1_ps(query.maxX);
    const __m256 qMaxY = _mm256_set1_ps(query.maxY);
    const __m256 qMaxZ = _mm256_set1_ps(query.maxZ);

    // Eight float32 candidates per AVX register.
    for (; i + 8 <= n; i += 8) {
        const __m256 ax0 = _mm256_loadu_ps(&boxes.minX[i]);
        const __m256 ay0 = _mm256_loadu_ps(&boxes.minY[i]);
        const __m256 az0 = _mm256_loadu_ps(&boxes.minZ[i]);

        const __m256 ax1 = _mm256_loadu_ps(&boxes.maxX[i]);
        const __m256 ay1 = _mm256_loadu_ps(&boxes.maxY[i]);
        const __m256 az1 = _mm256_loadu_ps(&boxes.maxZ[i]);

        // Ordered comparisons reject NaN inputs.
        const __m256 x0 =
            _mm256_cmp_ps(ax0, qMaxX, _CMP_LE_OQ);
        const __m256 x1 =
            _mm256_cmp_ps(ax1, qMinX, _CMP_GE_OQ);

        const __m256 y0 =
            _mm256_cmp_ps(ay0, qMaxY, _CMP_LE_OQ);
        const __m256 y1 =
            _mm256_cmp_ps(ay1, qMinY, _CMP_GE_OQ);

        const __m256 z0 =
            _mm256_cmp_ps(az0, qMaxZ, _CMP_LE_OQ);
        const __m256 z1 =
            _mm256_cmp_ps(az1, qMinZ, _CMP_GE_OQ);

        // All six conditions must hold in the same lane.
        __m256 mask = _mm256_and_ps(x0, x1);
        mask = _mm256_and_ps(mask, y0);
        mask = _mm256_and_ps(mask, y1);
        mask = _mm256_and_ps(mask, z0);
        mask = _mm256_and_ps(mask, z1);

        const int bits = _mm256_movemask_ps(mask);

        for (int lane = 0; lane < 8; ++lane) {
            if ((bits & (1 << lane)) != 0)
                result.push_back(i + lane);
        }
    }
#endif

    // Handle any candidates left after the vectorized loop.
    for (; i < n; ++i) {
        if (overlapScalar(query, boxes, i))
            result.push_back(i);
    }

    return result;
}

```

This implementation is deliberately explicit about the mapping between mathematics and instructions.

`_mm256_set1_ps` broadcasts one scalar into eight lanes. `_mm256_loadu_ps` loads eight adjacent float values without requiring a special aligned address. Each `_mm256_cmp_ps` executes eight comparisons, and `_mm256_and_ps` combines the resulting masks. `_mm256_movemask_ps` extracts the eight comparison outcomes into an integer.

The scalar tail is necessary because the number of boxes may not be divisible by eight.

The implementation also has an important precondition: all stored candidate box coordinates must be finite and obey `min <= max`. Query validation alone cannot guarantee that. In a real collision library, that invariant belongs to the SoA builder and its mutation API.

**Why SIMD is not automatically faster**

The SIMD loop performs fewer instruction-level comparisons, but the entire query includes memory loads, branch decisions and result insertion.

A workload with very few boxes may not justify the extra setup. A workload where nearly every box overlaps may become dominated by output production. A large BVH traversal may be limited by irregular pointer or index access rather than primitive comparisons.

An engine may benefit more from processing batches of coherent candidates than from blindly vectorizing every primitive function.

On an Apple Silicon target, AVX2 intrinsics are not applicable. The corresponding production design would use the appropriate ARM NEON/AdvSIMD facilities or a portable SIMD abstraction with architecture-specific implementations.

The mathematical SoA design is portable; the intrinsics are not.

And for GPUs, the mapping changes again. A GPU compute kernel might assign one thread to each AABB candidate and rely on SIMT execution and coalesced memory access rather than explicitly packing eight boxes into one CPU SIMD register.

The important design principle is to select the memory layout and execution method based on actual workload measurements.

**Fuzz testing collision routines**

Unit tests prove that the implementation behaves as expected for selected inputs. They do not establish that a geometric predicate behaves correctly across the enormous space of possible inputs.

This is especially important for collision mathematics because the most damaging bugs often occur in narrow regions of the input space: near-zero determinants, tangent contacts, shared edges, near-coplanar triangles, extremely thin shapes and very large or very small coordinate magnitudes.

Fuzzing systematically generates varied inputs and checks observable properties.

The difficult part is that a random intersection query does not automatically come with a known correct answer.

We need a test oracle.

**Strategy A: differential testing**

Implement two independent methods for the same mathematical query.

For example, compare a simple scalar triangle–AABB intersection implementation against an optimized SIMD or accelerated implementation.

If their Boolean results disagree, the disagreement becomes a candidate bug.

For mesh raycasts, a brute-force traversal of every triangle can serve as the reference for BVH correctness:

$$
\operatorname{RaycastBVH}(R,M) \stackrel{?}{=} \operatorname{RaycastBruteForce}(R,M).
$$

This validates the acceleration structure's candidate completeness and traversal logic, but only to the extent that both paths share correct primitive intersection semantics.

If the same broken ray–triangle function is used by both methods, differential comparison may miss the common defect.

An independent high-precision or robust geometric oracle can help address that limitation.

**Strategy B: metamorphic testing**

Metamorphic testing checks mathematical relationships between transformed inputs when the exact numerical answer is not already known.

For example, geometric intersection should be symmetric:

$$
\operatorname{Overlap}(A,B) = \operatorname{Overlap}(B,A).
$$

A common rigid transformation applied to both objects should preserve overlap:

$$
\operatorname{Overlap}(A,B) = \operatorname{Overlap}(TA,TB).
$$

For a uniform positive scale $s$, geometry should preserve intersection relationships:

$$
\operatorname{Overlap}(A,B) = \operatorname{Overlap}(sA,sB).
$$

These identities hold mathematically for appropriate shapes. In finite precision, transformed computations may differ near ambiguous boundaries, so a robust test framework should distinguish definite errors from cases within a justified uncertainty interval.

**Strategy C: adversarial cases**

Uniformly generated random geometry frequently misses the interesting cases.

A better generator deliberately constructs difficult arrangements: two spheres separated by exactly the sum of their radii, triangles sharing vertices or edges, triangles separated by a tiny signed plane distance, nearly parallel segments, zero-area triangles, zero-length rays, and large coordinates with tiny local separations.

The goal is not merely to generate more data. It is to generate inputs near the decision boundaries of the actual predicates.

**A reproducible test harness**

The following tests the earlier SoA batch overlap implementation against an independently written scalar reference, over many generated boxes.

```

#include <cassert>
#include <algorithm>
#include <random>
#include <vector>
#include <cstdint>

bool referenceOverlap(
    const QueryBox& a,
    const QueryBox& b)
{
    return a.minX <= b.maxX && a.maxX >= b.minX &&
           a.minY <= b.maxY && a.maxY >= b.minY &&
           a.minZ <= b.maxZ && a.maxZ >= b.minZ;
}

void fuzzAABBQueries()
{
    // A fixed seed makes failures reproducible.
    std::mt19937_64 rng{0xC0111510ULL};

    std::uniform_real_distribution<float> position(
        -100.0f, 100.0f);

    std::uniform_real_distribution<float> size(
        0.001f, 10.0f);

    auto randomBox = [&]() {
        const float x = position(rng);
        const float y = position(rng);
        const float z = position(rng);

        const float sx = size(rng);
        const float sy = size(rng);
        const float sz = size(rng);

        return QueryBox{
            x, y, z,
            x + sx, y + sy, z + sz
        };
    };

    for (int iteration = 0; iteration < 10000;
         ++iteration)
    {
        AABBSoA soa;
        std::vector<QueryBox> scalarBoxes;

        const QueryBox query = randomBox();

        // Vary the batch length to exercise SIMD tails.
        const int count = iteration % 33;

        for (int j = 0; j < count; ++j) {
            const QueryBox box = randomBox();

            scalarBoxes.push_back(box);
            soa.add(
                box.minX, box.minY, box.minZ,
                box.maxX, box.maxY, box.maxZ
            );
        }

        const auto optimized = queryOverlaps(query, soa);

        std::vector<std::size_t> reference;
        for (std::size_t j = 0;
             j < scalarBoxes.size(); ++j)
        {
            if (referenceOverlap(query, scalarBoxes[j]))
                reference.push_back(j);
        }

        // Both implementations must return the same IDs.
        assert(optimized == reference);
    }
}

```

This harness checks batching, indexing and SIMD tail behavior. It does not establish robust near-tangency behavior because it uses ordinary random boxes over a moderate numerical range.

To test robustness properly, add specialized generators that construct boundary cases analytically.

For example, create two boxes with

$$
A_{\max x}=B_{\min x}+\delta
$$

where $\delta$ is chosen from

$$
\\{0,\ \pm\operatorname{ULP}(x),\ \pm2\operatorname{ULP}(x),\ \pm10\operatorname{ULP}(x)\\}.
$$

Here `ULP(x)` is the spacing between adjacent representable floating-point values around $x$. A practical implementation can generate neighboring values using `std::nextafter`.

Now test the exact intended boundary convention. If tangency is inclusive, boxes sharing a face must count as overlapping. If the API uses strict interior penetration, tangency must not count as penetration.

Those are two different valid contracts.

**Applying fuzzing to the entire collision library**

A thorough collision test suite should combine several independent mechanisms:

- Differential comparisons between accelerated and brute-force mesh queries.
- Symmetry and rigid-transformation invariance.
- Scale families from microscopic geometry to large scenes.
- Explicit tangency, parallelism, coplanarity and degeneracy fixtures.
- Triangle-winding reversal and back-face policy tests.
- Stable replay from deterministic random seeds.
- Sanitizer builds for memory and undefined-behavior errors.
- Regression preservation of every discovered failure.

For C++ specifically, AddressSanitizer and UndefinedBehaviorSanitizer are useful for memory errors and undefined behavior, while ThreadSanitizer can help identify data races in concurrent query execution. These diagnose programming errors, not geometric correctness by themselves.

A sanitizer-clean collision library may still produce wrong geometric answers.

**Visual debugging collision algorithms**

Collision calculations are geometric, so raw numerical logs often fail to communicate the failure mechanism.

Suppose a triangle–box test returns a collision unexpectedly. The programmer may see only:

```
triangleIndex: 87
overlap: true
normal: (0.011, 0.999, -0.003)
```

That does not explain which separating axis failed, which triangle edge produced a near-zero cross product, or whether the bounding box was expanded conservatively.

An effective collision debugger should visualize the intermediate geometric reasoning.

**What should be visualized?**

For mesh raycasts, render the ray origin and direction, candidate BVH nodes, visited triangles, selected intersection points, hit distances and face normals.

For SAT, visualize the current test axis and both projection intervals. A failed separating axis should show the gap responsible for rejection.

For convex collision, visualize support points, GJK simplex evolution, EPA face normals and recovered penetration vectors.

For CCD, display the start and end transforms, swept bounds, the predicted time of impact and the configurations immediately before and after that time.

For a collision manifold, render normal arrows, contact points, penetration depths and persistent feature identifiers.

These views should also expose the difference between a broadphase candidate and a confirmed narrowphase contact.

**A useful debug-record design**

```

#include <vector>
#include <cstdint>

enum class DebugEventType : std::uint8_t {
    CandidateBox,
    TestedTriangle,
    SeparatingAxis,
    RejectedAxis,
    AcceptedHit,
    ContactPoint
};

struct CollisionDebugEvent {
    DebugEventType type{};

    std::uint32_t primitiveID = 0;

    Vec3 position{};
    Vec3 direction{};

    double scalar = 0.0;
};

struct CollisionDebugTrace {
    std::vector<CollisionDebugEvent> events;

    void record(
        DebugEventType type,
        std::uint32_t primitive,
        Vec3 position,
        Vec3 direction,
        double value)
    {
        events.push_back({
            type,
            primitive,
            position,
            direction,
            value
        });
    }
};

```

A production implementation should not allocate a fresh vector entry on every hot-path event during normal gameplay. Instead, debug instrumentation can be disabled entirely, routed into a preallocated bounded buffer, or sampled selectively.

The representation also needs a lifetime convention. If a debug event stores only a primitive index, the renderer must know which geometry snapshot that index refers to.

Otherwise, a mesh rebuild can make the visualization misleading.

**From symptom to diagnosis**

Imagine an object vanishing near the edge of the screen.

A useful diagnostic sequence is to draw its world-space AABB, extracted frustum planes, positive support vertex for each plane, and signed distance.

If the object disappears only when the camera rotates, possibilities include a wrong normal sign, incorrect matrix row/column convention, world/view space mismatch or an overly strict plane test.

The debug visualization should identify which plane rejected the object, letting you check the exact mathematical inequality instead of guessing.

That is the kind of debugging evidence a production renderer needs.

**Implementing a layered collision-query library**

We can now assemble all the previous material into one coherent architecture.

A collision library should not expose one enormous function that accepts every shape type and internally performs every operation.

Nor should every high-level consumer implement its own private ray–triangle test, epsilon selection and hit representation.

A useful architecture separates mathematical responsibilities while preserving efficient direct calls along critical paths.

There are several important relationships in this architecture.

The predicate kernel evaluates primitive mathematical decisions, such as whether three points are clockwise or whether an interval overlaps another.

The primitive-intersection layer constructs geometric results, such as a ray hit point and normal.

The algorithm layer combines primitives to solve more complicated problems: SAT projections, GJK simplex evolution, EPA penetration recovery, triangle–triangle intersection, clipping and continuous intersection.

The acceleration layer decides which expensive tests are necessary. It should only prune candidates when its bounds and query semantics justify pruning.

The public-query layer establishes the engine contract, including filtering, ownership, coordinate spaces and result ordering.

Finally, the engine consumers decide what the result means. A rendering system may treat a hit as an occluder, while a physics system may use the same geometric information to construct a contact constraint.

**Correctness invariants**

A layered collision library should enforce explicit invariants.

At the representation layer, a valid AABB must satisfy componentwise `min <= max`. Sphere radii must be nonnegative. A unit normal must actually be normalized when the API promises that property.

At the algorithm layer, a successful ray hit must satisfy the ray equation within the stated numerical accuracy:

$$
P_{\text{hit}}\approx O+tD.
$$

For a triangle hit, its barycentric weights must obey the triangle's containment conditions, subject to the specified boundary tolerance.

For closest-hit semantics, no valid accepted candidate may have a smaller hit parameter than the result.

At the acceleration layer, conservative pruning must not eliminate real intersections.

At the API layer, all returned coordinates must agree with the documented coordinate system, and every feature ID must identify valid geometry for the relevant snapshot.

At the concurrency layer, no query may access geometry whose lifetime has ended or mutate shared storage without proper synchronization.

These invariants become stronger when tested independently rather than relying on one large integration test.

**Complexity and performance**

Consider a mesh containing $N$ triangles, with a hierarchy containing $B$ bounding nodes.

A brute-force raycast has a worst-case cost of $O(N)$ primitive tests.

Building a BVH incurs preprocessing costs, typically depending on the split strategy and sorting or binning implementation. Its traversal usually reduces the number of expensive primitive tests, especially when the geometry is spatially well organized.

But a hierarchical structure can also produce poor traversal behavior for pathological meshes or rays, and its memory footprint may exceed that of the original triangle index buffer.

The total cost of a query is more appropriately decomposed as

$$
T_{\text{query}} = T_{\text{candidate traversal}} + N_{\text{primitive tests}}T_{\text{primitive}} + T_{\text{result construction}}.
$$

For all-hit queries, sorting may add

$$
O(k\log k)
$$

for $k$ collected results.

For a closest-hit query, maintaining only the best hit can avoid the full result-collection and sorting cost.

For batched AABB queries, SIMD may reduce the cost of primitive comparisons, but output generation and memory bandwidth can dominate.

This decomposition tells us what to profile and what not to optimize blindly.

**Repository audit — Current KairoGeometry and KairoPhysicsEngine**

The current source reveals both implemented capabilities and concrete places to extend the system. The following are observations from the files inspected for this pass, not claims that all possible repository modules have been exhaustively audited.

| Repository target                                                                                                                                  | Verified behavior                                                 | Engineering consideration                                 |
| -------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------- | --------------------------------------------------------- |
| [Geometry / Intersection.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Intersection.cppm?utm_source=chatgpt.com)                   | Primitive ray, triangle, SAT and plane tests                      | Fixed default epsilons and robustness near degeneracy     |
| [Geometry / Frustum.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Frustum.cppm?utm_source=chatgpt.com)                             | Plane extraction, sphere/AABB/OBB tests, corners                  | Conservative overlap semantics and projection conventions |
| [Physics / Collider.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Collider.cppm?utm_source=chatgpt.com)                         | Triangle meshes, validation, local BVH construction               | Scale-sensitive degeneracy rejection                      |
| [Physics / PhysicsWorld.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/PhysicsWorld.cppm?utm_source=chatgpt.com)                 | Raycast, RaycastAll, mesh raycasts, sphere sweeps                 | Closest-hit collection cost, feature metadata             |
| [Physics / Narrowphase.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Narrowphase.cppm?utm_source=chatgpt.com)                   | BVH-filtered mesh contacts using convex prisms                    | Contact approximation and manifold completeness           |
| [Geometry / GeometryTests.cpp](https://github.com/swayam8624/KairoGeometry/blob/master/tests/GeometryTests.cpp?utm_source=chatgpt.com)             | Ray/shape intersection and frustum cases                          | Expand adversarial and scale-family coverage              |
| [Physics / PhysicsEngineTests.cpp](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/tests/PhysicsEngineTests.cpp?utm_source=chatgpt.com) | Mesh validation, raycast, overlap, sweep, mesh–mesh and CCD cases | Extend differential and metamorphic testing               |

A few additional findings deserve explicit attention.

First, your `RaycastAll` semantics are per collider, not necessarily per triangle intersection. `PhysicsWorld::RaycastAll` accumulates one result from each shape-specific `RaycastCollider` call. The triangle-mesh path itself obtains a closest BVH ray hit. Therefore, multiple crossings of different triangles within the same mesh collider are not exposed as separate public results by this route.

That matters for the ray-parity containment algorithm. You cannot assume that the existing `PhysicsWorld::RaycastAll` directly provides the complete list of triangle crossings needed for parity.

Second, primitive `RayHit<T>` and physics `PhysicsRayHit` carry different information. The latter adds body/collider identity, distance and trigger status, but it does not expose mesh triangle identity or barycentrics. An editor-picking system requiring exact surface-feature selection would benefit from richer output.

Third, the existing mesh tests establish meaningful basic coverage. `PhysicsEngineTests.cpp`, around lines 419–492, checks invalid mesh indices, repeated indices, BVH validity, overlap queries, raycasting, sweep time of impact and debug shape reporting. Around lines 2173–2208, additional tests exercise mesh–mesh contacts and conservative CCD behavior.

Those are real tests, not hypothetical ones. But a successful focused test suite does not prove complete scale robustness or watertightness.

Fourth, `KairoGeometry::RayAABB` has a notable initial-overlap convention to examine. The inspected implementation initializes `tMin` to zero. When a ray starts inside a valid box, it may return a hit at $t=0$, representing initial overlap, rather than the positive-distance exit surface.

This can be a legitimate convention. However, a result at the interior origin should not be mistaken for a boundary intersection with a physically derived surface normal.

A stronger API would distinguish initial overlap from first boundary crossing, or explicitly select which interpretation the caller requests.

Fifth, the inspected primitive files do not expose the full polygon-clipping and 2D line-clipping functionality taught in M077. That means the canonical clipping algorithms developed here are implementation candidates, not verified descriptions of existing KairoGeometry functions.

The separation between verified implementation and proposed functionality is important. It prevents an architectural diagram from becoming an inaccurate description of the actual repository.

**Research bridge: robust collision queries at multiple scales**

For this phase, there is a meaningful experiment connecting geometry kernels, rendering reliability and physics simulation.

Research question: Can a filtered, scale-aware collision-query pipeline reduce near-boundary classification errors without introducing unacceptable performance overhead compared with conventional fixed-epsilon tests?

This is an experimental question, not a claim of novelty. Adaptive robust predicates are established research, and the experiment would need to isolate a contribution beyond adopting known algorithms.

**Experimental design**

The baseline would be the current primitive routines using their existing numerical thresholds.

A first alternative would use dimensionally appropriate scale-aware tolerance policies.

A second alternative would use filtered determinant predicates with higher precision or exact-sign fallback for the decision points where that is applicable.

The workloads should contain ordinary scenes as well as adversarial configurations: tangent spheres, nearly parallel triangles, skinny triangles, shared-edge raycasts, large-world coordinates, small reconstructed geometry and rapidly moving objects.

Measurements should include:

| Metric                                       | Why it matters                                               |
| -------------------------------------------- | ------------------------------------------------------------ |
| False-negative rate against a trusted oracle | Missed intersections are often the most dangerous errors     |
| False-positive rate                          | Measures conservativeness and unnecessary work               |
| Incorrect feature classification             | Detects wrong edge, face or vertex decisions                 |
| Query latency distribution                   | Reveals average and worst-case cost                          |
| Predicate fallback frequency                 | Quantifies how often higher precision is necessary           |
| Scale sensitivity                            | Measures consistency under uniform scaling                   |
| Topological inconsistency rate               | Important for mesh repair and containment                    |
| End-to-end simulation effects                | Tests whether predicate changes affect contacts or tunneling |

The oracle itself must be defined carefully. For example, using the same Möller–Trumbore predicate in both baseline and reference would not establish independent correctness.

For exact orientation signs, adaptive-precision methods are well established. For watertight ray–triangle intersections, Physically Based Rendering, fourth edition, discusses a ray-coordinate-space method that explicitly addresses shared-edge consistency and floating-point error bounds.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org+1



An especially useful ablation would compare direct world-space computation against local-space recentering, holding the underlying geometry and query semantics constant.

For Maveb, you could apply the experiment to captured meshes with reconstruction defects. For Vulkax, it could be applied to collision-query consistency in physical validation workloads.

The research contribution would have to come from an explicitly demonstrated new method, workload insight or system-level integration—not merely from changing a hardcoded epsilon into a configurable parameter.

**Final system reconstruction: one query from input to result**

To connect both chunks, reconstruct a concrete engine operation.

An editor user clicks on a visually rendered triangle mesh.

The engine first converts the mouse's screen coordinates into a world-space picking ray, using the active camera and the correct projection convention.

The physics or scene-query API validates the ray, establishes its maximum distance and filters allowed collision layers.

The broadphase identifies potentially intersected mesh colliders using their world bounds.

For a candidate mesh collider, the ray is transformed into local coordinates. Its local BVH traverses potentially intersected triangle bounds, rejecting nodes that cannot contain a closer hit.

The candidate triangles are tested using a ray–triangle predicate. A successful intersection produces the ray parameter, barycentric coordinates and geometric normal.

The closest-hit policy compares the result against its current best distance. It retains the nearest accepted hit and its associated feature metadata.

The engine transforms the resulting point and normal into the documented output coordinate space, constructs a stable hit record and returns it to the editor.

Finally, the editor resolves the hit collider and primitive ID to an asset, highlights the selected geometry and verifies that the relevant geometry generation is still valid.

Every component in this chain has a different failure mode.

The camera may construct an incorrect ray. The broadphase may reject a real candidate. The BVH may contain stale bounds. The primitive test may misclassify a shared edge. The result may report a local normal as a world normal. The editor may hold a primitive index after the mesh has been rebuilt.

Knowing the ray–triangle equation alone cannot diagnose these failures. Understanding the complete system can.

**Compression and recall sheet — M077 + M078**

This sheet is intentionally shorter than the lesson. Its purpose is to let you reconstruct the mechanisms rather than memorize the explanations.

M077 — Collision query mathematics

System: Query → Broadphase/BVH → Primitive intersection → Query policy → Hit record.

Ray–triangle: $R(t)=O+tD$; solve barycentrics $u,v,1-u-v$ through scalar triple products. Closest-hit minimizes accepted $t$; any-hit terminates on an accepted candidate; all-hit must define primitive/event semantics.

Mesh containment: Parity of surface crossings for suitable closed meshes; generalized winding $W=\sum\Omega/(4\pi)$ for oriented surfaces.

Frustum: For inward plane $d=n\cdot p+c$, reject a sphere when $d(c)<-r$. For a box, use support radius $r=\sum e_i|n\cdot u_i|$.

Triangle overlap: SAT, plane-side tests, edge–triangle intersections and a distinct coplanar method.

2D lines: $t=((Q-P)\times s)/(r\times s)$; segments additionally require valid parameter intervals and collinear-overlap handling.

Clipping: $t=d_A/(d_A-d_B)$. Sutherland–Hodgman reconstructs surviving polygons; Cohen–Sutherland uses outcodes; Liang–Barsky intersects parameter intervals. Clip graphics geometry before perspective division.

M078 — Reliable implementation

Robustness: Distinguish geometric tolerance, floating-point error bound, contact margin and exact predicate. Use dimensionally correct scales and higher precision when a sign cannot be certified.

Conservativeness: A broadphase may retain false positives but must not discard genuine intersections.

API: Define coordinate space, distance semantics, face policy, boundary policy, result ordering, ownership, validity and feature identity.

Performance: Profile query traversal, primitive testing, memory layout, result production and synchronization separately.

Verification: Differential oracles, metamorphic properties, adversarial fuzzing, debug geometry and deterministic regressions.

Repository anchors: `KairoGeometry/Intersection.cppm`, `Frustum.cppm`, `KairoPhysicsEngine/Collider.cppm`, `PhysicsWorld.cppm`, `Narrowphase.cppm`, plus their test directories.

**Blank-screen reconstruction challenge**

Try these without reopening the lesson. If an answer requires looking up the implementation, that is the specific mastery gap to repair.

The questions are hidden by default so you can attempt a blank-screen reconstruction before seeing the prompts.

Reveal 8 reconstruction challenges


All thirty original atomic topics, Parts 1271–1300, have now been addressed in the lesson, including their mathematical foundations, reference algorithms, relevant implementations, failure modes and integration concerns. The reference code has not been compiled and the new experiments have not been run in this session, so those verification steps—and your own reconstruction—remain separate from the teaching coverage.

18-pass study sequence

6 / 18

Pass 06 — M077 + M078 covered

12 passes remaining

The next dependency is Phase 14, which takes the exact geometry machinery developed in this pass and organizes it into increasingly sophisticated spatial structures.

Next — Pass 07: M079 + M080, Parts 1301–1336. We move into BVH representation and construction, splitting and surface-area heuristics, Morton ordering, LBVH, traversal, hit pruning, refitting and rebuilding, wide and quantized structures, and SIMD packet traversal.

That is where individual collision queries become scalable scene-query systems.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/26 - Contact Features and Manifolds|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/28 - Visibility Queries and Ray Intersections|Next chapter]]
