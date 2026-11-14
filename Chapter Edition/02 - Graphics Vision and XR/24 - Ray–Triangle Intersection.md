# 24 — Ray–Triangle Intersection

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/25 - SAT for Triangle and Convex Polygon Tests|Next chapter]]

A ray first meets the triangle’s plane, then the candidate point must pass a barycentric containment test. Separating those stages clarifies edge cases and numerical contracts.

**By the end:** Derive ray–triangle intersection from plane and barycentric tests.

**Recall question:** Why is a plane hit insufficient to establish a triangle hit?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 58761–61970; source-span SHA256 3ffa6980cdf14c6007237c740d88381cfe177214e26ca62f815836135e6fde94. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### PART III — Ray-Triangle Intersection

**Part 1207 — Plane intersection followed by barycentric containment**

A triangle is a bounded region of a plane.

This suggests a two-stage ray intersection algorithm.

First, intersect the ray with the triangle's supporting plane.

Second, determine whether the intersection point lies within the triangle.

Let the triangle vertices be:

$$
A,B,C
$$

Compute:

$$
E_1=B-A,\qquad E_2=C-A
$$

The unnormalized geometric normal is:

$$
N=E_1\times E_2
$$

The supporting plane satisfies:

$$
N\cdot(X-A)=0
$$

Substitute the ray:

$$
N\cdot(O+tD-A)=0
$$

Thus:

$$
\boxed{ t=\frac{N\cdot(A-O)}{N\cdot D} }
$$

If $N\cdot D=0$, the ray is parallel to or coplanar with the supporting plane.

Otherwise, evaluate:

$$
P=O+tD
$$

Now calculate the barycentric coordinates of $P$.

Write:

$$
P=A+uE_1+vE_2
$$

where:

$$
u\ge0,\quad v\ge0,\quad u+v\le1
$$

The third barycentric weight is:

$$
w=1-u-v
$$

We accept the intersection when:

$$
\boxed{ t\ge0,\quad u\ge0,\quad v\ge0,\quad u+v\le1 }
$$

together with the ray's actual interval constraints.

**Why the plane and triangle tests are separate**

The plane intersection asks whether the ray reaches the triangle's infinite supporting plane.

The barycentric test asks whether that point belongs to the finite triangular region.

A ray can intersect the supporting plane while completely missing the triangle.

**Why this method is useful**

It is conceptually simple and easy to debug.

It can reuse precomputed triangle planes.

It naturally exposes the geometric normal and barycentric coordinates.

However, for triangle meshes without precomputed planes, we can combine the calculations more efficiently.

That leads to the Möller–Trumbore method.

**Part 1208 — Deriving Möller–Trumbore ray-triangle intersection**

Möller and Trumbore's 1997 algorithm solves ray-triangle intersection using a coordinate transformation and determinant relationships without requiring a stored triangle-plane equation.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.tandfonline.com\&sz=32)

tandfonline.com+1



We begin with the ray:

$$
O+tD
$$

and the triangle's barycentric representation:

$$
A+uE_1+vE_2
$$

At an intersection:

$$
O+tD=A+uE_1+vE_2
$$

Rearrange:

$$
O-A=uE_1+vE_2-tD
$$

Define:

$$
T=O-A
$$

Then:

$$
\boxed{ T=uE_1+vE_2-tD }
$$

This is a three-variable linear system for $u,v,t$.

Its coefficient matrix has columns:

$$
[E_1,E_2,-D]
$$

**Construct the determinant**

Define:

$$
\boxed{ P=D\times E_2 }
$$

Then:

$$
\boxed{ \Delta=E_1\cdot P }
$$

This is a scalar triple product.

Its magnitude is related to the projected triangle area along the ray direction.

If it is zero in exact arithmetic, the ray direction is parallel to the triangle's supporting plane, or the triangle itself is degenerate.

**Derive $u$**

Take the dot product of:

$$
T=uE_1+vE_2-tD
$$

with $P=D\times E_2$.

Because $P$ is perpendicular to both $D$ and $E_2$:

$$
T\cdot P=u(E_1\cdot P)
$$

Therefore:

$$
\boxed{ u=\frac{T\cdot P}{\Delta} }
$$

This gives one barycentric coordinate without solving the entire matrix explicitly.

**Derive $v$**

Define:

$$
\boxed{ Q=T\times E_1 }
$$

Take the dot product of the original equation with a suitable cross-product expression, or apply Cramer's rule, to obtain:

$$
\boxed{ v=\frac{D\cdot Q}{\Delta} }
$$

**Derive $t$**

The ray parameter is:

$$
\boxed{ t=\frac{E_2\cdot Q}{\Delta} }
$$

Thus the complete algorithm is:

$$
\begin{aligned} E_1&=B-A\\\ E_2&=C-A\\\ P&=D\times E_2\\\ \Delta&=E_1\cdot P\\\ T&=O-A\\\ u&=(T\cdot P)/\Delta\\\ Q&=T\times E_1\\\ v&=(D\cdot Q)/\Delta\\\ t&=(E_2\cdot Q)/\Delta \end{aligned}
$$

with the proper validity and barycentric constraints.

**Why it avoids a separate plane calculation**

The determinant carries information about the ray's orientation relative to the triangle.

The cross products simultaneously provide the terms required for the ray parameter and barycentric coordinates.

We never explicitly construct the triangle's normalized plane.

This reduces unnecessary work and storage when processing large triangle meshes.

**C++23 implementation**

```

struct TriangleHit {
    double t;
    double u;
    double v;
    Vec3 point;
    Vec3 geometricNormal;
};

std::optional<TriangleHit> intersectRayTriangle(
    const Ray& ray,
    const Triangle& triangle,
    double tMin,
    double tMax)
{
    const Vec3 e1 = triangle.b - triangle.a;
    const Vec3 e2 = triangle.c - triangle.a;

    // P is perpendicular to the ray direction and second edge.
    const Vec3 p = cross(ray.direction, e2);

    // Signed determinant distinguishes parallelism and sidedness.
    const double determinant = dot(e1, p);

    // This threshold assumes bounded, reasonably scaled inputs.
    // A production robust predicate needs a deliberate error policy.
    if (std::abs(determinant) <= 1e-12)
        return std::nullopt;

    const double inverseDet = 1.0 / determinant;
    const Vec3 originFromA = ray.origin - triangle.a;

    // Weight of vertex B.
    const double u = dot(originFromA, p) * inverseDet;

    if (u < 0.0 || u > 1.0)
        return std::nullopt;

    const Vec3 q = cross(originFromA, e1);

    // Weight of vertex C.
    const double v = dot(ray.direction, q) * inverseDet;

    if (v < 0.0 || u + v > 1.0)
        return std::nullopt;

    const double t = dot(e2, q) * inverseDet;

    if (t < tMin || t > tMax)
        return std::nullopt;

    const Vec3 n = cross(e1, e2);
    const double normalLength = length(n);

    if (!(normalLength > 0.0))
        return std::nullopt;

    return TriangleHit{
        t,
        u,
        v,
        ray.origin + ray.direction * t,
        n / normalLength
    };
}

```

The returned barycentric weight for vertex A is:

$$
\boxed{w=1-u-v}
$$

The function returns the geometric normal determined by the triangle's stored winding.

It does not automatically flip the normal toward the ray.

That distinction is deliberate because the correct normal convention depends on the calling system.

**Part 1209 — Backface culling in ray-triangle tests**

For an oriented triangle:

$$
N=E_1\times E_2
$$

The triangle's front side is the side toward which $N$ points.

A ray approaches the front side when:

$$
\boxed{ N\cdot D<0 }
$$

because the ray travels opposite the outward-facing normal.

Recall that:

$$
\Delta=E_1\cdot(D\times E_2)
$$

By scalar triple-product identities:

$$
\boxed{ \Delta=-D\cdot(E_1\times E_2) }
$$

Therefore:

$$
\Delta=-D\cdot N
$$

For a front-facing hit:

$$
D\cdot N<0
$$

so:

$$
\boxed{\Delta>0}
$$

under this particular winding convention.

This leads to two different ray-triangle modes.

**Two-sided intersection**

Accept either sign of the determinant, provided its magnitude is not too small.

Conceptually:

$$
|\Delta|>\tau
$$

This permits rays from either side of the triangle.

It is useful for picking thin surfaces, two-sided mesh queries, and geometry that does not represent a closed solid.

**Front-face-only intersection**

Accept only:

$$
\Delta>\tau
$$

This rejects back-facing triangles.

It can reduce unnecessary work and enforce a one-sided surface model.

But it must agree with the mesh's winding conventions.

**Why ray culling and rasterizer culling are related but not identical**

Both use face orientation.

However, a rasterizer commonly determines front-face winding after projection and viewport transformation.

A ray-triangle routine reasons directly about the dot product between the geometric normal and ray direction.

The conventions must be reconciled rather than assumed identical.

**Normal orientation in collision systems**

Suppose a collision routine returns a triangle normal.

It may return the stored geometric normal, a normal oriented against the ray, or a normal from collider A toward collider B.

These choices are not equivalent.

Your `KairoGeometry::RayTriangle()` currently calculates the triangle normal and flips it when its dot product with the ray direction is positive.

That gives a normal opposing the incident ray.

This is a useful convention for ray-query consumers, but it loses information about the triangle's original winding unless the result separately records front-face status.

A richer hit structure could contain both the oriented geometric normal and a `frontFace` flag, allowing the caller to choose its policy.

**Watertightness and shared edges**

There is another important problem with ordinary floating-point ray-triangle intersection.

Suppose two triangles share an edge.

A ray passes exactly through that edge.

Ideally, the mesh should not develop a crack in the intersection result.

But if each triangle independently evaluates its barycentric boundaries with rounding error, both may reject the ray.

That can create a false miss through a watertight geometric surface.

For production ray tracing, specialised watertight ray-triangle algorithms use more consistent projected edge-function calculations and carefully designed numerical handling.

Möller–Trumbore is an essential foundational algorithm, but it should not be assumed to guarantee watertight ray traversal under arbitrary floating-point inputs.

### PART IV — Ray-Box and Ray-Capsule Intersections

**Part 1210 — Deriving the ray-AABB slab method**

An axis-aligned bounding box is the intersection of three infinite slabs.

A slab is the region between two parallel planes.

For the X axis:

$$
x_{\min}\le x\le x_{\max}
$$

For Y and Z, we have equivalent inequalities.

A point is inside the box only when it satisfies all three slab constraints simultaneously.

This gives us the idea behind the ray-box algorithm: find the interval of ray parameters belonging to each slab, then intersect the three intervals.

The ray is tested against each pair of parallel slab boundaries. Only the common parameter interval survives.

**Intersection with the X slab**

The X coordinate of a ray is:

$$
x(t)=O_x+tD_x
$$

The slab constraint is:

$$
x_{\min}\le O_x+tD_x\le x_{\max}
$$

Assuming $D_x\ne0$, the two boundary parameters are:

$$
\boxed{ t_x^0=\frac{x_{\min}-O_x}{D_x} }
$$

$$
\boxed{ t_x^1=\frac{x_{\max}-O_x}{D_x} }
$$

If $D_x>0$, the minimum boundary is encountered first.

If $D_x<0$, the maximum boundary is encountered first.

Therefore:

$$
\boxed{ t_x^{\mathrm{near}}=\min(t_x^0,t_x^1) }
$$

$$
\boxed{ t_x^{\mathrm{far}}=\max(t_x^0,t_x^1) }
$$

The ray lies inside the X slab exactly when its parameter belongs to:

$$
[t_x^{\mathrm{near}},t_x^{\mathrm{far}}]
$$

Similarly, obtain intervals for Y and Z.

**Intersecting the intervals**

The complete box intersection interval is:

$$
I=I_x\cap I_y\cap I_z
$$

Its lower endpoint is:

$$
\boxed{ t_{\mathrm{enter}}= \max(t_x^{\mathrm{near}},t_y^{\mathrm{near}},t_z^{\mathrm{near}}) }
$$

Its upper endpoint is:

$$
\boxed{ t_{\mathrm{exit}}= \min(t_x^{\mathrm{far}},t_y^{\mathrm{far}},t_z^{\mathrm{far}}) }
$$

The supporting infinite line intersects the box when:

$$
\boxed{ t_{\mathrm{enter}}\le t_{\mathrm{exit}} }
$$

For a ray restricted to:

$$
t\in[t_{\min},t_{\max}]
$$

the intersection of the line-box interval with that ray interval must be nonempty.

This is the mathematical basis of the slab algorithm used throughout ray tracing and spatial acceleration.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org



**Worked example**

Consider the box:

$$
B=[(-1,-1,-1),(1,1,1)]
$$

and ray:

$$
O=(-3,0,0),\quad D=(1,0,0)
$$

Along X:

$$
t_x^0=\frac{-1-(-3)}1=2
$$

$$
t_x^1=\frac{1-(-3)}1=4
$$

Thus:

$$
I_x=[2,4]
$$

Along Y and Z, the ray is parallel to their slab boundaries and its origin lies inside those slabs.

Therefore those axes impose no additional parameter restriction.

The complete interval is:

$$
\boxed{I=[2,4]}
$$

The entry point is:

$$
R(2)=(-1,0,0)
$$

The exit point is:

$$
R(4)=(1,0,0)
$$

**Why this is an ideal BVH primitive test**

A BVH contains many bounding boxes.

During traversal, a ray must repeatedly determine whether each box could contain a closer intersection.

The slab algorithm requires only a small, fixed number of scalar operations and is independent of the number of triangles inside the box.

If the ray misses a node's AABB, the entire node can be rejected.

The nearest and farthest parameters also provide information for traversal ordering and pruning.

For example, child nodes with smaller entry parameters can be visited before farther children, improving the chance of finding a close triangle hit early.

Once a closest triangle hit is found, the maximum permitted ray parameter can be reduced.

Any BVH node that begins farther away cannot contain a better hit.

**Part 1211 — Reciprocal ray directions**

The slab formula contains divisions:

$$
t=\frac{x_{\mathrm{boundary}}-O_x}{D_x}
$$

When testing one ray against thousands of AABBs, the same direction is reused.

We can precompute:

$$
\boxed{ I_x=\frac1{D_x},\quad I_y=\frac1{D_y},\quad I_z=\frac1{D_z} }
$$

Then:

$$
t_x^0=(x_{\min}-O_x)I_x
$$

$$
t_x^1=(x_{\max}-O_x)I_x
$$

The direction reciprocals need only be calculated once per ray.

The sign of each reciprocal also reveals which slab boundary is near and which is far.

A prepared-ray representation could contain:

```
struct PreparedRay {
    Vec3 origin;
    Vec3 direction;
    Vec3 inverseDirection;

    std::array<bool, 3> negativeDirection;

    double tMin;
    double tMax;
};
```

For a unit direction, `t` represents Euclidean distance. For a general nonzero direction, it represents an affine ray parameter.

The prepared-ray API should not silently assume that every direction component has a finite reciprocal.

**Performance motivation**

Division is generally more expensive than multiplication on many processors, although the exact performance depends on CPU architecture, compiler optimization, and instruction throughput.

Precomputing reciprocals also reduces redundant work across repeated box tests.

The reference Physically Based Rendering discusses precomputed reciprocal directions, direction signs, interval updates, and floating-point robustness in its ray-bounds implementation.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org



However, performance must be measured in context.

The optimized approach may increase ray-object storage and cache pressure. It may also interact differently with SIMD execution, special-value handling, and compiler vectorization.

The best approach depends on the target architecture and workload.

**Part 1212 — Zero direction components and IEEE floating-point hazards**

Suppose:

$$
D_x=0
$$

The ray's X coordinate never changes:

$$
x(t)=O_x
$$

There are two cases.

If:

$$
x_{\min}\le O_x\le x_{\max}
$$

the ray remains inside the X slab for every parameter.

This slab imposes no additional restriction:

$$
I_x=(-\infty,\infty)
$$

If:

$$
O_x\<x_{\min}
$$

or:

$$
O_x>x_{\max}
$$

the ray never enters that slab.

The box intersection is empty.

**Why blindly multiplying by infinity can fail**

Under common IEEE 754 floating-point behaviour:

$$
\frac1{+0}=+\infty
$$

and:

$$
\frac1{-0}=-\infty
$$

But if the ray origin lies exactly on a slab boundary:

$$
x_{\min}-O_x=0
$$

then:

$$
0\cdot\infty
$$

produces NaN.

NaN does not obey normal ordered comparisons.

For example, comparisons involving NaN generally do not behave like comparisons involving ordinary finite values.

A naïve slab routine may propagate NaN into its interval endpoints or accidentally accept/reject a hit.

**Robust implementation strategy**

One straightforward strategy is to handle zero directions explicitly before computing reciprocals.

```

bool updateSlab(
    double origin,
    double direction,
    double slabMin,
    double slabMax,
    double& tEnter,
    double& tExit)
{
    // A parallel ray either stays within this slab
    // forever or never enters it.
    if (direction == 0.0) {
        return origin >= slabMin &&
               origin <= slabMax;
    }

    const double inverseDirection = 1.0 / direction;

    double nearT = (slabMin - origin) * inverseDirection;
    double farT  = (slabMax - origin) * inverseDirection;

    // Negative ray directions reverse the boundary order.
    if (nearT > farT)
        std::swap(nearT, farT);

    // Intersect this slab's parameter interval with
    // the interval accumulated from previous axes.
    tEnter = std::max(tEnter, nearT);
    tExit  = std::min(tExit, farT);

    // An empty intersection proves that the ray misses.
    return tEnter <= tExit;
}

```

This avoids the `0 * infinity` hazard for exact zero direction components.

The function assumes valid, finite slab bounds and finite ray origin values. It does not constitute a complete interval-error analysis.

More aggressively optimized implementations can use carefully chosen min/max semantics and precomputed signs while preserving correct handling of infinities and NaNs.

**Another issue: rays beginning inside the box**

If a ray begins inside the box, the line's entry parameter lies behind the ray origin:

$$
t_{\mathrm{enter}}<0
$$

while:

$$
t_{\mathrm{exit}}>0
$$

For a Boolean ray-volume intersection test, the ray already intersects the solid at $t=0$.

For a first-boundary-hit query, the relevant hit is the exit parameter:

$$
\boxed{t=t_{\mathrm{exit}}}
$$

These have different semantics.

Your current `KairoGeometry::RayAABB()` initializes `tMin` to zero. When the origin is inside, it retains that value and can return `t=0` with a fallback normal rather than the actual exit surface.

That may be acceptable for a solid-volume query, but it differs from the behaviour of your `RaySphere()` when its origin lies inside a sphere, where the positive exit root is returned.

For a consistent raycasting API, I recommend explicitly distinguishing `IntersectsSolid` and `FirstSurfaceCrossing` semantics.

**Part 1213 — Ray-OBB intersection by local-space transformation**

An OBB is simply an AABB expressed in its own orthonormal coordinate frame.

Let the OBB have centre $C$, local unit axes $U_0,U_1,U_2$, and half-extents $E$.

Construct the matrix:

$$
U= \begin{bmatrix} U_0&U_1&U_2 \end{bmatrix}
$$

Because its axes are orthonormal:

$$
U^{-1}=U^T
$$

**Transform the ray origin**

For world-space ray origin $O$, local coordinates are:

$$
\boxed{ O'=U^T(O-C) }
$$

**Transform the ray direction**

Directions are not translated.

Therefore:

$$
\boxed{ D'=U^TD }
$$

The local ray becomes:

$$
R'(t)=O'+tD'
$$

Substitute the transformations:

$$
R'(t)=U^T(O-C)+tU^TD
$$

Factor:

$$
R'(t)=U^T(O+tD-C)
$$

This is exactly the world-space ray transformed into the OBB's local frame.

**Perform an AABB test**

In local coordinates, the OBB becomes:

$$
[-E,E]
$$

Therefore use the ordinary slab algorithm:

$$
\operatorname{RayAABB}(O',D',[-E,E])
$$

The ray parameter $t$ does not change under this coordinate transformation because the ray direction is transformed consistently without reparameterization.

**Transform the result back**

If the local hit point is $P'$:

$$
\boxed{ P=C+UP' }
$$

If the local geometric normal is $N'$:

$$
\boxed{ N=UN' }
$$

For an orthonormal frame, this preserves unit normal length.

**Why normalization can break parameter consistency**

Suppose the local ray direction is independently normalized after the coordinate transformation.

Then its magnitude may change, and the numerical ray parameter no longer necessarily corresponds to the original world-space parameter.

For a pure rotation, length is preserved, so normalization should be unnecessary except for correcting accumulated numerical error.

For an arbitrary affine object transform containing nonuniform scale, the inverse-transformed ray direction generally changes length.

Re-normalizing without adjusting the parameter interval changes the ray parameterization.

A robust object-space intersection implementation must either preserve the transformed direction magnitude or consistently convert parameter values.

The same issue appears in ray tracing transformed meshes.

**Your repository**

Your `KairoGeometry::RayOBB()` follows the expected approach.

It transforms the origin and direction into the OBB frame, constructs a local AABB with bounds $[-E,E]$, performs `RayAABB()`, and transforms the hit point and normal back.

This is mathematically correct under the OBB's orthonormal-axis invariant.

Because it delegates to `RayAABB()`, it inherits that routine's inside-ray and endpoint semantics.

**Part 1214 — Ray-capsule intersection**

A capsule is a segment swept by a sphere of radius $r$.

Geometrically, its boundary consists of a cylindrical middle section and two hemispherical ends.

This makes its ray intersection more complicated than a sphere's, but it remains analytically tractable.

Let the central segment endpoints be:

$$
A,B
$$

with axis:

$$
U=B-A
$$

and squared length:

$$
L^2=U\cdot U
$$

Assume $L^2>0$.

**Decompose the ray relative to the capsule axis**

The ray is:

$$
R(t)=O+tD
$$

Define:

$$
M=O-A
$$

We want to isolate the component perpendicular to the capsule axis.

The perpendicular part of the origin displacement is:

$$
\boxed{ M_\perp= M-U\frac{M\cdot U}{L^2} }
$$

Similarly, the perpendicular part of the ray direction is:

$$
\boxed{ D_\perp= D-U\frac{D\cdot U}{L^2} }
$$

The perpendicular displacement from the axis at ray parameter $t$ is:

$$
M_\perp+tD_\perp
$$

For a point on the cylindrical surface, its perpendicular distance from the axis equals $r$:

$$
\|M_\perp+tD_\perp\|^2=r^2
$$

**Derive the cylinder quadratic**

Expand:

$$
(M_\perp+tD_\perp)\cdot (M_\perp+tD_\perp)=r^2
$$

Thus:

$$
\boxed{ at^2+bt+c=0 }
$$

where:

$$
\boxed{ a=D_\perp\cdot D_\perp }
$$

$$
\boxed{ b=2M_\perp\cdot D_\perp }
$$

$$
\boxed{ c=M_\perp\cdot M_\perp-r^2 }
$$

Solve the quadratic using the stable-root method from Part 1206.

However, this describes the infinite cylinder.

We still need to restrict hits to the capsule's finite central body.

**Axial-coordinate test**

For a candidate hit:

$$
P=O+tD
$$

calculate its normalized position along the capsule axis:

$$
\boxed{ h=\frac{(P-A)\cdot U}{L^2} }
$$

The cylindrical middle section exists only for:

$$
\boxed{ 0\le h\le1 }
$$

If a candidate cylinder root has $h<0$ or $h>1$, it belongs to the infinite cylinder extension, not the capsule's actual cylindrical surface.

Reject that candidate.

**The hemispherical ends**

Each capsule end is part of a sphere.

The sphere centred at $A$ contributes only the hemisphere beyond the start of the segment:

$$
\boxed{h\le0}
$$

The sphere centred at $B$ contributes only the hemisphere beyond the end:

$$
\boxed{h\ge1}
$$

Therefore the correct procedure is to raycast against each endpoint sphere and retain only roots lying on the appropriate hemisphere.

A spherical-root candidate on the interior side of the end cap may belong to a surface inside the filled capsule, not to its actual external boundary.

**A concrete incorrect-hit example**

Consider the capsule:

$$
A=(0,-2,0)
$$

$$
B=(0,2,0)
$$

$$
r=1
$$

A ray starts at the centre:

$$
O=(0,0,0)
$$

and travels along positive Y:

$$
D=(0,1,0)
$$

The actual capsule exit surface is at:

$$
y=3
$$

Therefore the correct first positive surface crossing is:

$$
\boxed{t=3}
$$

Now consider the complete sphere centred at endpoint $B=(0,2,0)$.

The ray intersects that sphere at:

$$
y=1
$$

and:

$$
y=3
$$

The point $y=1$ is inside the cylindrical body of the capsule.

It is not on the external capsule boundary.

A ray-capsule implementation that tests complete endpoint spheres without filtering their roots can incorrectly report:

$$
t=1
$$

instead of $t=3$.

**Verified KairoGeometry concern**

Your current `KairoGeometry::RayCapsule()` tests the two complete endpoint spheres and then tests the cylindrical body, choosing the smallest returned hit parameter.

The endpoint-sphere candidates are not restricted to their outward hemispheres in the inspected implementation.

The example above exposes why this is significant.

The reference lab for this pass implements the hemisphere restrictions and includes an assertion that the axial ray exits at $t=3$.

**Other capsule edge cases**

If:

$$
A=B
$$

the capsule degenerates into a sphere.

If the ray is parallel to the capsule axis, the perpendicular direction component becomes zero:

$$
D_\perp=0
$$

The cylinder quadratic is degenerate. The endpoint hemispheres may still provide valid surface intersections.

If the ray starts inside the capsule, the first surface crossing may be the cylinder side or an end hemisphere.

If the ray is tangent, quadratic roots may coincide.

A complete implementation must combine valid candidates under one consistent ray-interval and boundary policy.

**Parts 1215–1216 — Segment-plane and segment-triangle intersections**

A finite segment between $A$ and $B$ can be represented as:

$$
S(t)=A+t(B-A)
$$

with:

$$
\boxed{t\in[0,1]}
$$

This makes many segment intersection algorithms equivalent to ray intersection algorithms with a restricted parameter interval.

**Segment-plane intersection**

For plane:

$$
N\cdot X+d=0
$$

substitute the segment:

$$
N\cdot(A+t(B-A))+d=0
$$

Therefore:

$$
\boxed{ t= -\frac{N\cdot A+d}{N\cdot(B-A)} }
$$

when the denominator is nonzero.

The segment intersects the plane exactly when:

$$
0\le t\le1
$$

A parallel segment may be completely separate from the plane or entirely contained in it.

There is also a useful endpoint-classification approach.

Define:

$$
s_A=N\cdot A+d
$$

$$
s_B=N\cdot B+d
$$

If the signs are strictly opposite, the segment crosses the plane interior.

If either signed residual is zero, an endpoint lies on the plane.

If both are zero, the entire segment is coplanar.

With opposite signs, linear interpolation gives:

$$
\boxed{ t=\frac{s_A}{s_A-s_B} }
$$

This formula is equivalent to the earlier result.

**Segment-triangle intersection**

One direct method is:

$$
R(t)=A+t(B-A),\qquad t\in[0,1]
$$

and run Möller–Trumbore using that interval.

The barycentric and determinant conditions remain unchanged.

The segment-domain constraint rejects triangle hits beyond either endpoint.

**Coplanar segment-triangle intersection**

A segment may lie in the same plane as a triangle.

The ordinary nonparallel Möller–Trumbore algorithm rejects this case because its determinant is zero.

But the segment and triangle may still overlap geometrically.

To handle coplanar intersections fully, one approach is to project both into a well-conditioned 2D coordinate plane and perform segment-polygon overlap tests.

This includes detecting segment-edge crossings, segment endpoints inside the triangle, and collinear edge overlap.

A Boolean intersection API should explicitly decide whether coplanar overlap counts as a hit.

A single-point raycast API may instead report a separate coplanar status because the intersection may be an interval rather than one isolated point.

### PART V — M074: Primitive Overlap and Contact Geometry

Original Parts 1217–1228

We now move from rays intersecting stationary surfaces to two geometric solids occupying space simultaneously.

The important change is that we no longer have one distinguished ray direction.

For many collision pairs, we need to infer a suitable contact direction from the geometry itself.

We will begin with shapes whose nearest points and overlap conditions can be calculated analytically, then derive the general Separating Axis Theorem.

**Part 1217 — Sphere-sphere overlap**

Let two filled spheres be:

$$
S_A=(C_A,r_A)
$$

$$
S_B=(C_B,r_B)
$$

Their centres are separated by:

$$
d=\|C_B-C_A\|
$$

The spheres overlap or touch exactly when their centre distance is no greater than the sum of their radii:

$$
\boxed{ d\le r_A+r_B }
$$

Why?

Any point inside sphere A is within radius $r_A$ of $C_A$.

Any point inside sphere B is within radius $r_B$ of $C_B$.

If the centre separation exceeds the sum of radii, the triangle inequality prevents any point from belonging to both spheres.

Conversely, when the centre separation is no greater than the radius sum, the two closed balls share at least one point.

Squaring gives:

$$
\boxed{ \|C_B-C_A\|^2\le(r_A+r_B)^2 }
$$

This avoids the square root.

The formula assumes valid nonnegative radii.

**Worked example**

Let:

$$
C_A=(0,0,0),\quad r_A=2
$$

$$
C_B=(3,0,0),\quad r_B=2
$$

The centre distance is:

$$
d=3
$$

The radius sum is:

$$
r_A+r_B=4
$$

Since $3<4$, the spheres penetrate.

Their overlap along the centre direction has depth:

$$
4-3=1
$$

**Part 1218 — Sphere-sphere contact normal, witnesses, and penetration**

Suppose:

$$
\Delta=C_B-C_A
$$

and:

$$
d=\|\Delta\|
$$

For $d>0$, the unit normal pointing from A toward B is:

$$
\boxed{ n=\frac{\Delta}{d} }
$$

This normal is meaningful because spheres are rotationally symmetric.

The penetration depth is:

$$
\boxed{ \delta=\max(0,r_A+r_B-d) }
$$

for overlapping or touching spheres.

**Surface witness points**

The surface point on A facing B is:

$$
\boxed{ P_A=C_A+r_An }
$$

The surface point on B facing A is:

$$
\boxed{ P_B=C_B-r_Bn }
$$

Their signed separation along the normal is:

$$
(P_B-P_A)\cdot n
$$

Substitute:

$$
(C_B-C_A)\cdot n-r_A-r_B
$$

Since:

$$
(C_B-C_A)\cdot n=d
$$

we obtain:

$$
\boxed{ (P_B-P_A)\cdot n=d-r_A-r_B=-\delta }
$$

when the spheres penetrate.

Thus the witness points encode the penetration directly.

**Contact-position convention**

A solver may store both surface points separately.

Alternatively, it may store a single representative point, such as:

$$
P_c=\frac{P_A+P_B}{2}
$$

The midpoint is convenient but does not make the overlapping witness points identical.

In the penetrating configuration, each lies on its respective object's surface.

The solver may use local-space anchors and reconstruct the world-space positions during constraint evaluation.

**Coincident centres**

If:

$$
C_A=C_B
$$

then:

$$
d=0
$$

and the normal formula is undefined.

There is no unique radial direction between the centres.

The collision state remains well defined, but contact-normal selection requires a deterministic or history-dependent convention.

Possible choices include a previous contact normal, relative-motion direction, or fixed fallback axis.

For two nonzero-radius coincident spheres, the smallest translation magnitude needed to eliminate overlap is:

$$
\delta=r_A+r_B
$$

The fallback direction can be chosen arbitrarily because all directions are geometrically equivalent for coincident spheres.

**A useful physics convention**

Suppose normal $n$ always points from collider A toward collider B.

Then a positive normal impulse on B acts along $+n$, while its equal-and-opposite counterpart on A acts along $-n$.

That convention must be preserved across the contact manifold and solver.

If a narrowphase arbitrarily reverses a contact normal, the solver may apply impulses in the wrong direction.

This is why collision normals should not be treated as purely cosmetic vectors.

**Parts 1219–1220 — Sphere-plane overlap and contact point**

A plane is an infinite mathematical surface, whereas a half-space represents a filled region on one side of that plane.

This distinction affects both overlap and contact semantics.

Let a plane have unit normal $n$ and offset $d$:

$$
n\cdot X+d=0
$$

For sphere centre $C$ and radius $r$, the signed centre distance is:

$$
\boxed{ s=n\cdot C+d }
$$

**Two-sided plane-surface intersection**

A sphere intersects the infinite plane surface when:

$$
\boxed{ |s|\le r }
$$

If the centre lies farther than its radius from the plane, the entire sphere is on one side.

If the centre lies within one radius of the plane, the sphere touches or crosses it.

For an unnormalized plane normal $N$, the condition is:

$$
\boxed{ |N\cdot C+d|\le r\|N\| }
$$

**Closest point on the plane**

With unit normal:

$$
\boxed{ Q=C-sn }
$$

This point is the orthogonal projection of the sphere centre onto the plane.

For a sphere touching a floor from above, the corresponding sphere witness point is:

$$
\boxed{ P_S=C-rn }
$$

where $n$ points upward from the floor toward the allowed region.

The amount of penetration through the plane is:

$$
\boxed{ \delta=r-s }
$$

when the sphere violates the one-sided floor constraint $s\ge r$.

**Two-sided versus one-sided depth**

For an infinite two-sided plane surface, when the sphere intersects it, a local translation away from the plane can have magnitude:

$$
\boxed{ \delta_{\mathrm{two-sided}}=r-|s| }
$$

The direction is chosen according to the sign of $s$, with ambiguity when the centre lies exactly on the plane.

For a one-sided floor, the sphere is required to remain above the plane, and the correction is:

$$
\boxed{ \delta_{\mathrm{floor}}=\max(0,r-s) }
$$

This distinction matters when the sphere centre has already passed below the plane.

A two-sided surface model may allow the sphere to separate by moving farther downward.

A one-sided floor model instead requires movement back toward the permitted half-space.

**Contact point versus intersection curve**

When a sphere crosses a plane, their mathematical surfaces intersect in a circle, not generally at one point.

A collision solver usually does not need that entire circle.

It needs one or more contact constraints representing the interaction.

The projected centre point on the plane is often a useful contact location for sphere-plane collision.

Therefore “contact point” is not necessarily the unique point of mathematical surface intersection.

**Part 1221 — Sphere-AABB overlap**

Let the sphere have centre $C$ and radius $r$.

Let the AABB have minimum and maximum corners:

$$
m,M
$$

From Phase 12, the nearest point in the filled AABB is found by independently clamping the sphere centre's coordinates:

$$
\boxed{ Q_i=\operatorname{clamp}(C_i,m_i,M_i) }
$$

Then the sphere overlaps the box exactly when the centre lies within radius $r$ of the solid box:

$$
\boxed{ \|C-Q\|^2\le r^2 }
$$

This is a surprisingly simple exact overlap test for valid filled geometry.

**Worked example**

Suppose:

$$
B=[(-1,-1,-1),(1,1,1)]
$$

and sphere:

$$
C=(2,0,0),\quad r=1
$$

The nearest box point is:

$$
Q=(1,0,0)
$$

Therefore:

$$
d=\|C-Q\|=1
$$

The sphere touches the box at:

$$
(1,0,0)
$$

A closed-solid intersection test returns true.

**Sphere centre inside the box**

If $C$ lies inside the AABB, clamping returns:

$$
Q=C
$$

Therefore:

$$
\|C-Q\|=0
$$

The overlap test correctly returns true for any valid sphere radius, including zero.

But the same calculation does not provide a useful separation normal.

This is why Boolean overlap and contact generation require different algorithms.

**Part 1222 — Sphere-AABB contact generation**

We must distinguish sphere centres outside the box from centres inside it.

**Sphere centre outside the box**

Let:

$$
Q=\operatorname{ClosestPoint}_{B}(C)
$$

and:

$$
V=C-Q
$$

If:

$$
\|V\|>0
$$

the direction moving the sphere away from the box is:

$$
\boxed{ m=\frac{C-Q}{\|C-Q\|} }
$$

The penetration depth is:

$$
\boxed{ \delta=r-\|C-Q\| }
$$

provided the sphere overlaps the box.

If your collision convention requires a normal from sphere A toward box B, that normal is:

$$
n=-m
$$

The sign is important.

**Sphere centre inside the box**

Now suppose:

$$
C\in B
$$

The clamp result is:

$$
Q=C
$$

and:

$$
V=0
$$

The direction $V/\|V\|$ is undefined.

We need another geometric construction.

A valid approach is to calculate the distances from the sphere centre to the six box faces.

For axis X, the distances are:

$$
C_x-m_x
$$

and:

$$
M_x-C_x
$$

The corresponding distances exist for Y and Z.

Let the smallest face distance be:

$$
\boxed{ d_f= \min_i(C_i-m_i,\ M_i-C_i) }
$$

Choose the associated outward box-face direction $m$.

To move the sphere completely outside the box along that direction, the centre must reach the selected face and then move another radius.

Therefore the required translation magnitude is:

$$
\boxed{ \delta=d_f+r }
$$

This is the key containment-case formula.

**Worked containment example**

Take:

$$
B=[(-1,-1,-1),(1,1,1)]
$$

and sphere:

$$
C=(0,0,0),\quad r=0.25
$$

The sphere lies completely inside the box.

The nearest box face is one unit away in every coordinate direction.

Therefore:

$$
d_f=1
$$

The sphere must move:

$$
\boxed{\delta=1+0.25=1.25}
$$

to lie outside the box while remaining tangent.

A correction of only $0.25$ would leave the sphere deeply inside the box.

**Actual KairoPhysicsEngine observation**

In the inspected [Narrowphase.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Narrowphase.cppm?utm_source=chatgpt.com), the `MakeSphereBoxContact()` routine computes a closest point on the box.

When the centre-to-closest-point squared distance is very small, it selects a fallback normal and sets:

```
penetration = sphereRadius;
```

For a sphere strictly contained inside the box, this does not account for the distance from the sphere centre to the nearest box face.

If the penetration field is intended to represent a minimum translation depth, this is an underestimation.

The correct containment case requires additional face-distance information.

This is a concrete numerical and geometric contract that deserves a dedicated physics test.

**Contact-generation subtlety**

For an exterior sphere, the nearest box point $Q$ provides a meaningful feature witness.

For a contained sphere, a box-face point must be constructed explicitly.

The solver also needs a consistent convention for which object should move along the normal.

A contact manifold that returns an arbitrarily chosen face but a penetration depth calculated for a different direction is internally inconsistent.

The contact feature, normal, and penetration must describe the same geometric separation.

**Part 1223 — Sphere-OBB overlap**

An OBB can be transformed into an AABB in its own orthonormal frame.

Let the OBB have centre $B_C$, axis matrix $U$, and half-extents $E$.

Transform the sphere centre:

$$
\boxed{ C'=U^T(C-B_C) }
$$

The OBB becomes the local AABB:

$$
[-E,E]
$$

Compute the nearest local point:

$$
\boxed{ Q'_i=\operatorname{clamp}(C'_i,-E_i,E_i) }
$$

Because $U$ is orthonormal, it preserves Euclidean distance:

$$
\|C-Q\|=\|C'-Q'\|
$$

Thus:

$$
\boxed{ \|C'-Q'\|^2\le r^2 }
$$

is the sphere-OBB overlap condition.

The same inside-versus-outside distinction applies when generating contacts.

This is one of the advantages of a clean geometry library: the sphere-AABB algorithm can be reused for sphere-OBB queries through an appropriate coordinate transformation.

**Part 1224 — Capsule-sphere overlap**

A capsule is defined by segment $[A,B]$ and radius $r_C$.

A sphere is defined by centre $S$ and radius $r_S$.

The capsule consists of all points within $r_C$ of its axis segment.

The sphere consists of all points within $r_S$ of its centre.

Therefore the two solids overlap when the distance from the sphere centre to the capsule axis is no greater than the sum of their radii.

Let:

$$
Q=\operatorname{ClosestPoint}_{[A,B]}(S)
$$

Then:

$$
\boxed{ \|S-Q\|^2\le(r_C+r_S)^2 }
$$

This is exact for filled capsule-sphere overlap.

**Why the radius sum appears**

The capsule is the Minkowski sum of its axis segment and a radius-$r_C$ ball.

The sphere is itself a translated radius-$r_S$ ball.

The two rounded volumes intersect when the distance between their skeletal sets is at most the radius sum.

This generalizes the sphere-sphere condition, where both skeletal sets are single points.

**Part 1225 — Capsule-capsule overlap from segment distance**

Let two capsules have central segments:

$$
S_A=[A_0,A_1]
$$

$$
S_B=[B_0,B_1]
$$

and radii:

$$
r_A,\quad r_B
$$

From Pass 03, we know how to calculate the minimum squared distance between two finite segments:

$$
d^2(S_A,S_B)
$$

The capsules intersect when:

$$
\boxed{ d^2(S_A,S_B)\le(r_A+r_B)^2 }
$$

This is the same skeletal-distance principle applied to two segments.

**Contact construction**

Suppose the segment-segment closest-point routine returns:

$$
P_A,\quad P_B
$$

with:

$$
d=\|P_B-P_A\|
$$

For $d>0$, a candidate normal from capsule A to capsule B is:

$$
\boxed{ n=\frac{P_B-P_A}{d} }
$$

A penetration estimate for the rounded skeletal features is:

$$
\boxed{ \delta=r_A+r_B-d }
$$

The corresponding surface witness points are:

$$
Q_A=P_A+r_An
$$

$$
Q_B=P_B-r_Bn
$$

As with spheres, these witnesses are useful when the selected closest-axis configuration determines the appropriate local contact.

When the closest segment points coincide, the normal is undefined and a fallback or more general contact-feature procedure is required.

For parallel overlapping capsule axes, the nearest-point pair may not be unique. A single point-pair result may also be insufficient to represent the full contact manifold.

**Part 1226 — Capsule-plane overlap**

Let a capsule have endpoints $A,B$ and radius $r$.

Let the plane have unit normal $n$ and offset $d$.

Calculate endpoint signed distances:

$$
s_A=n\cdot A+d
$$

$$
s_B=n\cdot B+d
$$

The signed plane residual along the capsule axis is a linear function.

Therefore its minimum and maximum values occur at the segment endpoints.

If:

$$
s_As_B\le0
$$

the axis segment touches or crosses the plane.

The capsule therefore intersects the plane for any nonnegative radius.

If the endpoints lie strictly on the same side, the nearest point on the segment to the plane is the endpoint with smaller absolute signed distance.

Thus the capsule-plane surface intersection condition is:

$$
\boxed{ \begin{cases} \text{true},&s_As_B\le0\\\ \min(|s_A|,|s_B|)\le r,&\text{otherwise} \end{cases} }
$$

For a non-unit plane normal, compare endpoint residuals against:

$$
r\|n\|
$$

rather than $r$.

**Capsule-plane contact**

A capsule lying horizontally on a floor may touch the plane along a line segment.

A capsule standing vertically may touch through one rounded end.

Therefore the mathematical contact set can have different dimensionality depending on orientation.

A physics engine may approximate that set with one or more contact points.

The correct contact normal and penetration convention depend on whether the plane is a two-sided surface or a one-sided solid boundary.

**Part 1227 — AABB-AABB overlap intervals**

Two AABBs overlap when their coordinate intervals overlap on all three axes.

For axis $i$, the intervals are:

$$
A_i=[a_{\min,i},a_{\max,i}]
$$

$$
B_i=[b_{\min,i},b_{\max,i}]
$$

The ordinary intersection interval is:

$$
\boxed{ I_i=[ \max(a_{\min,i},b_{\min,i}), \min(a_{\max,i},b_{\max,i}) ] }
$$

It is nonempty when:

$$
\boxed{ \max(a_{\min,i},b_{\min,i}) \le \min(a_{\max,i},b_{\max,i}) }
$$

The boxes intersect when this holds for X, Y, and Z.

This is an elementary version of SAT, since the coordinate axes are the only candidate separating axes required for AABBs.

**Overlap width**

When intervals overlap, their common width is:

$$
\boxed{ w_i= \min(a_{\max,i},b_{\max,i}) - \max(a_{\min,i},b_{\min,i}) }
$$

This quantity describes how much of the axis is shared by both intervals.

But it is not always the amount one box must be translated to separate them.

That distinction leads to Part 1228.

**Part 1228 — AABB-AABB penetration-axis selection**

For a partially overlapping pair of intervals, their overlap width often equals the shortest separation translation along that axis.

But if one interval contains the other, the overlap width underestimates the translation needed to separate the interval pair.

**Containment counterexample**

Let:

$$
A=[-2,2]
$$

and:

$$
B=[-1,1]
$$

The intersection interval is:

$$
[-1,1]
$$

with width:

$$
w=2
$$

But translating B by only two units gives:

$$
B'=[1,3]
$$

which still overlaps A over $[1,2]$.

To move B to the right until it merely touches A, we need:

$$
B'=[2,4]
$$

which requires translation magnitude:

$$
3
$$

Likewise, moving left requires magnitude $3$.

Thus:

$$
\boxed{\delta_{\mathrm{MTV}}=3\ne w=2}
$$

**Correct signed separation distances**

For any overlapping intervals A and B, consider moving B along the positive axis until its minimum reaches A's maximum:

$$
b_{\min}+\delta_+=a_{\max}
$$

Therefore:

$$
\boxed{ \delta_+=a_{\max}-b_{\min} }
$$

For moving B along the negative axis until its maximum reaches A's minimum:

$$
b_{\max}-\delta_-=a_{\min}
$$

Thus:

$$
\boxed{ \delta_-=b_{\max}-a_{\min} }
$$

The smallest translation magnitude along that axis is:

$$
\boxed{ \delta_i=\min(\delta_+,\delta_-) }
$$

The sign of the chosen translation depends on which direction is smaller.

For two AABBs, choose the coordinate axis with the smallest such translation magnitude.

**Complete AABB minimum translation vector**

Calculate the two separation distances for X, Y, and Z.

Select the smallest positive candidate across all six signed directions.

This yields a minimum-magnitude translation that brings the AABBs into nonpenetrating boundary contact, under the convention that touching is permitted.

The selected axis becomes a candidate collision normal.

**Why a single SAT axis is not always a complete contact manifold**

For an AABB overlap, the minimum translation direction is relatively simple.

But the actual contact region might be a face patch, an edge, or a point.

Selecting the minimum-penetration axis does not automatically construct the geometrically appropriate contact points.

Contact generation must determine which features are involved.

That is why box-box collision code often follows SAT with feature selection and clipping.

### PART VI — The Separating Axis Theorem

Original Parts 1229–1237

This is the principal mathematical development of M074.

Previously, we derived specific formulas for sphere-sphere, sphere-box, capsule-capsule, and AABB-AABB overlap.

Each works because it exploits the structure of a particular pair of shapes.

Now we want one geometric principle that applies to convex objects more generally.

The Separating Axis Theorem (SAT) supplies that principle for convex polytopes.

**Part 1229 — Why SAT is needed**

Suppose we want to determine whether two rotated boxes intersect.

AABB interval tests are no longer sufficient.

The boxes may overlap along the world X, Y, and Z axes while remaining separated along a diagonal direction.

Likewise, comparing their centres is insufficient.

Two boxes can have close centres but miss each other because their extents and orientations differ.

We need to test whether there exists a direction along which the projections of the two shapes are separated.

**Geometric intuition**

Take two convex polygons lying on a table.

Imagine shining parallel light along a direction perpendicular to some axis.

Each polygon casts a one-dimensional projected interval on that axis.

If the intervals do not overlap, the original polygons cannot intersect.

Their spatial separation is certified by that axis.

Separated projected intervals prove that the two shapes cannot intersect. One separating axis is sufficient to reject overlap.

The important direction of reasoning is:

$$
\boxed{ \text{Separated projections} \implies \text{Separated shapes} }
$$

This implication is true for arbitrary sets.

The more powerful convexity result says that for suitable disjoint convex sets, some separating direction exists.

For convex polytopes, we can further reduce the search to a finite set of candidate axes.

**Part 1230 — Why SAT works for convex polytopes**

Let $A$ and $B$ be two nonempty compact convex sets.

Suppose:

$$
A\cap B=\varnothing
$$

Because the sets are compact, there exists a pair of closest points:

$$
a^*\in A,\quad b^*\in B
$$

with positive separation:

$$
d=\|b^*-a^*\|>0
$$

Define:

$$
\boxed{ n=\frac{b^*-a^*}{d} }
$$

This direction points from the closest point on A toward the closest point on B.

**Constructing a separating direction**

From the convex projection optimality condition derived in Pass 03:

$$
(b^*-a^*)\cdot(a-a^*)\le0
$$

for every $a\in A$.

Dividing by $d$:

$$
n\cdot a\le n\cdot a^*
$$

Similarly, considering $b^*$ as the closest point of B to $a^*$, we obtain:

$$
n\cdot b\ge n\cdot b^*
$$

for every $b\in B$.

But:

$$
n\cdot b^*-n\cdot a^* = n\cdot(b^*-a^*)=d>0
$$

Therefore:

$$
\boxed{ \max_{a\in A}n\cdot a < \min_{b\in B}n\cdot b }
$$

The projected intervals are strictly separated.

This constructs a separating axis from the closest-point geometry.

**What the theorem tells us**

For nonempty compact convex sets:

$$
\boxed{ A\cap B=\varnothing \iff \exists n\ne0: \operatorname{proj}_n(A) \cap \operatorname{proj}_n(B)=\varnothing }
$$

Here the projections are the corresponding one-dimensional intervals.

For intersecting shapes, no separating axis exists.

For tangent shapes, the projected intervals may touch at an endpoint, consistent with the closed-solid intersection convention.

**Why convexity matters**

For nonconvex shapes, the full projection intervals may overlap in every direction even though the shapes are disjoint.

For example, one nonconvex object can surround empty regions containing another disconnected object.

Projection intervals lose information about internal holes and concavities.

For such shapes, SAT on the whole object is not generally a complete intersection test.

A common solution is to decompose nonconvex geometry into convex components or use a spatial hierarchy of simpler primitives.

**From infinitely many directions to finitely many axes**

The separation theorem permits any direction.

Testing infinitely many directions is impossible.

For convex polytopes, the important observation is that the boundary features of the Minkowski difference determine a finite set of candidate separating directions.

In 2D, polygon edge normals suffice.

In 3D, candidate directions arise from polygonal face normals and cross products of edge directions from the two polyhedra.

For boxes, this finite set has a particularly simple structure.

**Part 1231 — Projection intervals of convex shapes**

For a shape $K$ and nonzero axis $n$, define the scalar projection of a point $X$:

$$
p_n(X)=n\cdot X
$$

The projected interval of $K$ is:

$$
\boxed{ I_K(n)= \left[ \min_{X\in K}n\cdot X,\; \max_{X\in K}n\cdot X \right] }
$$

The upper endpoint is the support function:

$$
\boxed{ h_K(n)=\max_{X\in K}n\cdot X }
$$

The lower endpoint can be expressed as:

$$
\boxed{ -h_K(-n) }
$$

Therefore:

$$
\boxed{ I_K(n)=[-h_K(-n),h_K(n)] }
$$

This is a powerful connection to Phase 12.

SAT and support mapping are two different uses of the same extreme-projection geometry.

**Projecting a sphere**

For a sphere centred at $C$ with radius $r$, the maximum projection is:

$$
h(n)=n\cdot C+r\|n\|
$$

and the minimum is:

$$
n\cdot C-r\|n\|
$$

Thus:

$$
\boxed{ I_S(n)= [n\cdot C-r\|n\|,\; n\cdot C+r\|n\|] }
$$

For a unit axis:

$$
I_S(n)=[n\cdot C-r,n\cdot C+r]
$$

**Projecting an AABB**

Let the box have centre $C$ and half-extents $E$.

Its projected centre is:

$$
c=n\cdot C
$$

Its projected radius is:

$$
\boxed{ r=|n_x|e_x+|n_y|e_y+|n_z|e_z }
$$

Therefore:

$$
\boxed{I_B(n)=[c-r,c+r]}
$$

This is the same formula used in frustum-AABB classification.

**Projecting an OBB**

Let the OBB have unit axes $U_0,U_1,U_2$ and half-extents $e_0,e_1,e_2$.

The projected radius is:

$$
\boxed{ r= e_0|n\cdot U_0| + e_1|n\cdot U_1| + e_2|n\cdot U_2| }
$$

The interval is:

$$
\boxed{ [n\cdot C-r,\ n\cdot C+r] }
$$

We derived this support-radius expression in Phase 12.

Now it becomes the central computational operation for OBB SAT.

**Projecting a convex polygon**

For vertices $V_0,\ldots,V_{k-1}$, the extreme projections are:

$$
p_{\min}=\min_i(n\cdot V_i)
$$

$$
p_{\max}=\max_i(n\cdot V_i)
$$

For a convex polygon, it is sufficient to test vertices because a linear function reaches its maximum and minimum over a compact convex polytope at vertices.

A direct scan costs:

$$
O(k)
$$

For small polygons, this is straightforward.

For large convex polyhedra, support-mapping approaches may exploit adjacency or directional coherence to avoid scanning all vertices in every query.

**Part 1232 — The interval separation test**

Suppose two projected intervals are:

$$
I_A=[a_{\min},a_{\max}]
$$

and:

$$
I_B=[b_{\min},b_{\max}]
$$

They are strictly separated when:

$$
\boxed{ a_{\max}\<b_{\min} }
$$

or:

$$
\boxed{ b_{\max}\<a_{\min} }
$$

Equivalently, they overlap or touch when:

$$
\boxed{ a_{\max}\ge b_{\min} \quad\land\quad b_{\max}\ge a_{\min} }
$$

For two shapes whose projection intervals are represented by centres and radii, let:

$$
c_A=n\cdot C_A
$$

$$
c_B=n\cdot C_B
$$

and projected radii $r_A,r_B$.

The intervals overlap when:

$$
\boxed{ |c_B-c_A|\le r_A+r_B }
$$

Since:

$$
c_B-c_A=n\cdot(C_B-C_A)
$$

we obtain the very useful SAT condition:

$$
\boxed{ |n\cdot(C_B-C_A)|\le r_A+r_B }
$$

If the left side exceeds the right side, the axis separates the shapes.

**SAT requires normalized axes for depth comparison**

For Boolean separation, the axis does not need to be normalized.

If $n$ is scaled by positive factor $k$, every projection and radius scales by the same factor.

The separation inequality is unchanged.

But if we compare overlap magnitudes across different axes to determine minimum penetration, axis length matters.

A raw overlap measured along axis $n$ scales with $\|n\|$.

Therefore SAT candidate axes should be normalized, or overlap measurements should be divided by axis length, before comparing penetration depths.

This becomes particularly important for OBB edge-cross-product axes, whose magnitudes can vary considerably.

**Part 1233 — Why OBB-OBB SAT needs 15 candidate axes**

Consider two nondegenerate oriented boxes.

Let box A have orthonormal axes:

$$
A_0,A_1,A_2
$$

and box B have:

$$
B_0,B_1,B_2
$$

For 3D boxes, the standard complete SAT candidate set contains:

$$
\boxed{ 3+3+9=15\text{ axes} }
$$

The candidates are:

Candidate separating axes for two OBBs

**3**

A's face normals

**3**

B's face normals

**9**

Edge-cross axes

Each candidate tests whether the projected intervals can be separated. One successful separating axis proves the boxes do not intersect.

The first six are:

$$
A_0,A_1,A_2,B_0,B_1,B_2
$$

The remaining nine are:

$$
\boxed{ A_i\times B_j,\qquad i,j\in\\{0,1,2\\} }
$$

**Why face normals are necessary**

A box face can provide the separating plane between two boxes.

If box B lies beyond one of box A's faces, the corresponding face normal can separate them.

Similarly, one of box B's face normals may provide a separation that none of box A's axes detects.

Therefore both sets of face normals are necessary.

**Why edge-cross axes are necessary**

Two rotated boxes can approach one another in an edge-to-edge configuration.

The critical separating direction may be perpendicular to one edge from A and one edge from B.

If the edge directions are $A_i,B_j$, a vector perpendicular to both is:

$$
\boxed{ n=A_i\times B_j }
$$

This explains the remaining nine axes.

Checking only the six face normals is not sufficient for general OBB-OBB intersection.

**A concrete cross-axis separation case**

In the compiled reference lab, I included two long, thin OBBs.

Box A uses world-aligned axes and half-extents:

$$
E_A=(2,0.3,0.4)
$$

Box B has the same half-extents, a rotated orthonormal frame, and centre displacement approximately:

$$
C_B-C_A=(-2.0815,0.9094,-0.1561)
$$

For this pair, all six face-normal projection tests pass.

Nevertheless, one of the edge-cross-product axes separates the boxes.

Therefore an implementation testing only six axes would incorrectly report overlap.

The full 15-axis implementation correctly rejects the pair.

This is a useful adversarial regression because it verifies that the cross axes are not merely theoretical extras.

**Part 1234 — OBB face-axis tests in matrix form**

The straightforward SAT implementation can construct every axis and calculate projected radii using dot products.

But we can derive a more efficient formulation by expressing the second box's axes in the first box's coordinate frame.

Let:

$$
\boxed{ R_{ij}=A_i\cdot B_j }
$$

This $3\times3$ matrix contains the relative orientation between the two OBB frames.

Also define centre displacement:

$$
T=C_B-C_A
$$

Express the translation in A's frame:

$$
\boxed{ t_i=T\cdot A_i }
$$

Let A's half-extents be:

$$
a_0,a_1,a_2
$$

and B's half-extents be:

$$
b_0,b_1,b_2
$$

**Testing A's first face axis**

Choose:

$$
n=A_0
$$

A's projected radius is simply:

$$
r_A=a_0
$$

because its other two axes are perpendicular to $A_0$.

B's projected radius is:

$$
r_B= b_0|A_0\cdot B_0| + b_1|A_0\cdot B_1| + b_2|A_0\cdot B_2|
$$

Using $R_{ij}$:

$$
\boxed{ r_B= b_0|R_{00}|+ b_1|R_{01}|+ b_2|R_{02}| }
$$

The projected centre distance is:

$$
|t_0|
$$

Therefore the separating condition is:

$$
\boxed{ |t_0|> a_0+b_0|R_{00}|+b_1|R_{01}|+b_2|R_{02}| }
$$

For the general A face axis $A_i$:

$$
\boxed{ |t_i|> a_i+\sum_{j=0}^{2}b_j|R_{ij}| }
$$

If this inequality is true for any $i$, the boxes are disjoint.

**Testing B's face axes**

Now choose:

$$
n=B_j
$$

B's projected radius is simply:

$$
r_B=b_j
$$

A's projected radius is:

$$
r_A=\sum_{i=0}^2a_i|R_{ij}|
$$

The projected centre displacement is:

$$
T\cdot B_j
$$

Because $T=\sum_i t_iA_i$:

$$
T\cdot B_j=\sum_i t_iR_{ij}
$$

Therefore the separating condition is:

$$
\boxed{ \left|\sum_i t_iR_{ij}\right| > b_j+\sum_i a_i|R_{ij}| }
$$

This gives the three B face-axis tests.

The advantage of the relative rotation matrix is that its nine dot products are calculated once and reused across all fifteen tests.

**Part 1235 — Deriving the nine edge-cross-product tests**

Choose one candidate:

$$
\boxed{ L=A_0\times B_0 }
$$

We will derive its complete SAT inequality.

**Projected centre separation**

Express:

$$
T=t_0A_0+t_1A_1+t_2A_2
$$

Then:

$$
T\cdot L= T\cdot(A_0\times B_0)
$$

The component $t_0A_0$ contributes zero because $A_0$ is perpendicular to $A_0\times B_0$.

Using the oriented orthonormal basis identities:

$$
\boxed{ |T\cdot L|= |t_2R_{10}-t_1R_{20}| }
$$

The sign inside the absolute value depends on basis orientation, but the separation magnitude follows this expression under the usual right-handed frame convention.

**Projected radius of A**

For box A:

$$
r_A= \sum_i a_i|A_i\cdot L|
$$

Because $L$ is perpendicular to $A_0$, the first term vanishes.

The remaining terms simplify to:

$$
\boxed{ r_A=a_1|R_{20}|+a_2|R_{10}| }
$$

**Projected radius of B**

Likewise, $L$ is perpendicular to $B_0$, so B's first term vanishes.

The remaining terms give:

$$
\boxed{ r_B=b_1|R_{02}|+b_2|R_{01}| }
$$

**The final cross-axis test**

The complete separation condition becomes:

$$
\boxed{ |t_2R_{10}-t_1R_{20}| > a_1|R_{20}|+ a_2|R_{10}|+ b_1|R_{02}|+ b_2|R_{01}| }
$$

This is one of the nine edge-cross-product conditions.

The other eight follow by cyclically choosing the remaining pairs $A_i\times B_j$.

**Why this formulation is efficient**

Instead of creating a cross-product vector, normalizing it, and projecting every box axis, the matrix formulation reuses values from $R$ and $t$.

This can reduce operations and make branch behaviour predictable.

For Boolean overlap tests, the cross axis does not need to be explicitly normalized because both sides of the inequality scale consistently.

But if we want to compare penetration depths across axes, we must account for the cross-axis magnitude.

**General-purpose versus specialized SAT**

A generic SAT implementation can iterate through candidate axes and call a projected-radius function.

This is excellent for learning, testing, and supporting different shapes.

A specialized 15-axis OBB routine uses the relative rotation matrix and hand-derived scalar expressions.

That version can be faster in a physics narrowphase, depending on hardware and compiler optimization.

It is also easier to make a transcription error in one of the nine cross-axis formulas.

A strong engineering strategy is to retain a readable generic SAT reference and differential-test the optimized implementation against it.

Your `KairoGeometry::OBBOBB()` already uses the specialized relative-orientation matrix approach.

Your `KairoPhysicsEngine::FindBoxBoxSAT()` uses a more generic candidate-axis loop.

This gives you a natural opportunity to compare independent implementations.

**Part 1236 — Degenerate and nearly parallel cross axes**

Suppose two candidate edge directions are parallel:

$$
A_i\parallel B_j
$$

Then:

$$
\boxed{ A_i\times B_j=0 }
$$

The cross product does not define a usable separating direction.

This axis contributes no independent constraint and can be skipped.

**Nearly parallel axes**

If the two directions are nearly parallel:

$$
\|A_i\times B_j\|\approx0
$$

For unit vectors:

$$
\|A_i\times B_j\|=|\sin\theta|
$$

where $\theta$ is the angle between them.

Normalizing this tiny vector divides by a small magnitude.

Rounding error in the cross product can be amplified into a large angular error.

Therefore an implementation may deliberately skip cross axes beneath a small threshold.

But this introduces a trade-off.

A cross axis that is almost zero may still be the only separating direction for a very close edge-edge configuration.

Skipping every small cross axis can produce false-positive overlap results.

For a conservative broadphase, a false positive may be acceptable.

For an exact narrowphase overlap test, it means the algorithm is no longer guaranteed to detect every separation.

**Why adding epsilon to the relative rotation matrix helps**

A common OBB SAT implementation calculates:

$$
\boxed{ \operatorname{AbsR}_{ij}=|R_{ij}|+\epsilon }
$$

The small positive adjustment makes projected radii slightly larger.

This helps avoid some false separations caused by floating-point rounding near parallel configurations.

But the epsilon changes the test conservatively.

It may cause two geometrically separate boxes to be classified as overlapping when their separation is extremely small.

It also needs to be chosen relative to precision and the intended numerical contract.

An epsilon adjustment is not a substitute for understanding the arithmetic error bounds.

**Production considerations**

A useful strategy is to distinguish a conservative overlap query from a higher-accuracy geometric separation query.

The conservative query can tolerate extra candidates.

The accurate query may need more careful precision, adaptive handling, or a fallback solver when the SAT configuration becomes poorly conditioned.

Do not silently claim exact geometric overlap if the implementation deliberately enlarges projected radii or discards nearly parallel candidate axes.

Your `KairoGeometry::OBBOBB()` uses an epsilon-adjusted absolute rotation matrix.

Your `KairoPhysicsEngine::TestBoxAxis()` skips axes whose squared length is below a fixed threshold.

Both approaches are standard engineering patterns, but their numerical behaviour should be documented and tested at multiple scales.

**Part 1237 — Selecting a minimum-penetration SAT axis**

Once every separating-axis test has passed, the objects overlap under the SAT model.

We can then use the candidate axes to estimate or calculate a minimum translation direction for separating convex polytopes.

The obvious idea is to choose the axis with the smallest projected overlap.

However, we must carefully define overlap.

**Ordinary overlap width is insufficient under containment**

Suppose projected intervals are:

$$
A=[-2,2],\quad B=[-1,1]
$$

Their common interval has width $2$.

But translating B out of A requires a displacement of magnitude $3$, as derived in Part 1228.

Therefore selecting axes using only:

$$
\min(a_{\max},b_{\max})- \max(a_{\min},b_{\min})
$$

can give incorrect translation depths in containment cases.

**Correct interval translation candidates**

Along a unit axis $n$, let:

$$
A=[a_{\min},a_{\max}]
$$

$$
B=[b_{\min},b_{\max}]
$$

For overlapping intervals, the translation required to move B in the positive axis direction is:

$$
\boxed{ \delta_+=a_{\max}-b_{\min} }
$$

For the negative axis direction:

$$
\boxed{ \delta_-=b_{\max}-a_{\min} }
$$

Select the smaller magnitude and corresponding direction.

Then repeat for every SAT candidate axis.

**Centre-radius form**

For OBBs:

$$
I_A=[c_A-r_A,c_A+r_A]
$$

$$
I_B=[c_B-r_B,c_B+r_B]
$$

Define:

$$
s=c_B-c_A
$$

Then the positive translation is:

$$
\delta_+ = (c_A+r_A)-(c_B-r_B)
$$

Thus:

$$
\boxed{ \delta_+=r_A+r_B-s }
$$

Likewise:

$$
\boxed{ \delta_-=r_A+r_B+s }
$$

For a unit axis, compare the two positive translation distances and choose the smaller.

This works even when one projected interval contains the other.

**Why the axis must be normalized for penetration**

Suppose candidate axis $n$ has length $k$.

Its projected interval values are scaled by $k$.

Therefore its raw translation distance is also scaled by $k$.

Comparing raw values from different axis magnitudes would be invalid.

For a true Euclidean translation depth along the axis, use:

$$
\boxed{ \hat n=\frac n{\|n\|} }
$$

or divide the raw projected depth by $\|n\|$.

This is especially important for OBB edge-cross axes.

**Minimum translation vector versus contact points**

For intersecting full-dimensional convex polytopes, testing the relevant SAT directions provides candidate translations associated with the facets of their Minkowski difference.

The smallest valid candidate translation determines a minimum translation direction under the mathematical model.

But that does not automatically identify the contact features or provide a stable contact manifold.

Consider two boxes touching face-to-face.

The minimum penetration axis may be normal to the contacting faces.

Yet the actual contact region is a polygonal patch.

A solver may need several representative points from that patch.

For an edge-edge configuration, the contact feature and point construction are different.

Therefore an SAT-based narrowphase often separates the algorithm into:

$$
\boxed{ \text{SAT overlap and axis} \rightarrow \text{reference/incident features} \rightarrow \text{clipping} \rightarrow \text{contact manifold} }
$$

We will derive manifold clipping and persistent contact features in M076.

**KairoPhysicsEngine implementation**

Your `FindBoxBoxSAT()` computes the overlap value:

```
const float overlap =
    ProjectBoxRadius(boxA, normal) +
    ProjectBoxRadius(boxB, normal) -
    distance;
```

where `distance` is the absolute projected centre separation.

For ordinary partial overlap, this produces the expected overlap depth along the axis.

For interval containment, it does not include the additional distance required to translate the contained interval to a separating boundary.

This is a notable issue if the routine's `penetration` output is intended to represent the true minimum translation depth.

Your `BuildBoxBoxContacts()` then uses the selected normal and penetration to generate points, reducing them to a bounded contact set.

This is already a more advanced pipeline than simple Boolean SAT, and it provides an excellent target for containment and edge-edge regression tests.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/25 - SAT for Triangle and Convex Polygon Tests|Next chapter]]
