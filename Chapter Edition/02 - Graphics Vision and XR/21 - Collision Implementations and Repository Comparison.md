# 21 — Collision Implementations and Repository Comparison

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/20 - Geometry Queries from Scene to Contact|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/22 - Convex Support Mapping and Distance|Next chapter]]

The canonical implementation gives a concrete collision pipeline, and the repository comparison tests whether the existing code satisfies the same contracts. The focus is implementation evidence rather than an abstract list of algorithms.

**By the end:** Compare a reference collision implementation with an existing engine implementation.

**Recall question:** What evidence would show that two implementations agree beyond matching one example?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 50455–53175; source-span SHA256 4d17188383f66aa2ba12ab023e382a345b02ac8dd00c1b00f946111c929d3acc. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Canonical C++23 implementation and repository comparison

**The complete reference lab**

I prepared and compiled an independent, commented C++23 reference implementation that covers the mathematics of M067 and M068.

Geometry Pass 01 — complete C++23 source

Standalone geometry reference and executable assertion tests

The source implements line, ray and segment closest-point queries; plane construction and projection; sphere containment, signed distance and surface projection; triangle area and barycentric reconstruction; point-in-triangle classification; and seven-region closest-point evaluation with degenerate-triangle fallback.

The source separates two ideas that should never be confused: a formula that is mathematically correct for ideal arithmetic, and the input-validation/numerical policies necessary to turn it into a dependable API.

Compilation and initial reference tests passed

Compiled using GCC with C++23, optimization, and standard warning flags. Assertions passed for the covered examples, but this is not a formal proof of numerical robustness.

For an independent reference implementation, I've chosen `double` and explicit `std::optional` failure reporting where a mathematical object is undefined. This makes it suitable as a CPU-side learning and testing reference.

It is not a replacement for Shewchuk-style exact predicates or a complete production numerical kernel.

**Comparison with your actual KairoGeometry implementation**

The source I inspected is on the repository's `master` branch. The following observations are grounded in the current code, rather than inferred from the curriculum.

| Module                                                                                                                | Existing implementation                                                    | Important engineering consideration                        |
| --------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------- | ---------------------------------------------------------- |
| [Ray.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Ray.cppm?utm_source=chatgpt.com)                   | Origin, direction, evaluation, normalization and closest-point helpers     | Normalized-ray contract versus arbitrary stored directions |
| [Line.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Line.cppm?utm_source=chatgpt.com)                 | Infinite-line projection using dot products                                | Scale-aware handling of short directions                   |
| [Segment.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Segment.cppm?utm_source=chatgpt.com)           | Explicit zero-length segment semantics; parameter clamping                 | Numerical threshold applied to squared segment length      |
| [Plane.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Plane.cppm?utm_source=chatgpt.com)               | Normal-offset representation, constructors, signed distance and projection | Unit-normal invariants and collinear-point construction    |
| [Sphere.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Sphere.cppm?utm_source=chatgpt.com)             | Centre-radius representation and radial queries                            | Distinguishing surface from solid; finite-value validation |
| [Triangle.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Triangle.cppm?utm_source=chatgpt.com)         | Cross-product normals, area, Gram-based barycentrics, containment          | Scale-dependent degeneracy and fallback-normal semantics   |
| [ClosestPoint.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/ClosestPoint.cppm?utm_source=chatgpt.com) | Seven-region closest-point triangle method                                 | Near-degenerate behavior and numerical branch reliability  |

**One particularly important observation**

Your `Triangle.cppm` barycentric method evaluates:

```
const T denominator =
    d00 * d11 - d01 * d01;

if (std::abs(denominator) <= epsilon) {
    return { T(-1), T(-1), T(-1) };
}
```

The numerator and denominator formulas themselves follow the standard Gram-system derivation.

However, the denominator has units of length to the fourth power, whereas the default epsilon comes directly from machine precision multiplied by a constant.

This makes the degeneracy classification dependent on the coordinate scale.

Furthermore, returning `(-1,-1,-1)` gives callers a sentinel that does not satisfy the usual barycentric sum-to-one invariant. This can work if every caller checks for it, but an explicit success/failure result would make misuse harder.

Your `ClosestPoint.cppm` already implements the Ericson-style Voronoi-region method and includes an edge-based fallback when its final face denominator is too small.

That is a useful starting point. The remaining question is how well it behaves on near-degenerate and extreme-scale inputs, particularly because several early region conditions are evaluated before the final denominator guard.

We should test that behavior rather than automatically classify it as incorrect.

**Production-oriented geometry architecture**

For the longer-term Kairo engine, an especially useful separation would be:

```
KairoGeometry
│
├── Primitive Values
│   ├── Ray, Line, Segment
│   ├── Plane, Sphere, Triangle
│   └── Later: AABB, OBB, Capsule, Frustum
│
├── Predicate Layer
│   ├── Orientation and sidedness
│   ├── Containment and overlap
│   └── Numerical robustness policy
│
├── Construction Layer
│   ├── Projection
│   ├── Closest-point queries
│   └── Intersection-point recovery
│
├── Query Layer
│   ├── Finite ray intervals
│   ├── Query results and feature identifiers
│   └── Error/degeneracy reporting
│
└── Validation Layer
    ├── Unit and property tests
    ├── Scale and translation tests
    ├── Adversarial numerical cases
    └── Debug visualizations
```

This architecture becomes valuable when the same mathematical primitives are shared by the renderer, physics engine, mesh tools, spatial accelerators, and reconstruction systems.

**Numerical and implementation decisions that matter in production**

**Memory and execution**

The formulas covered in this pass use small fixed-size data types and can generally execute without dynamic allocation.

For individual editor-picking queries, an array-of-structures representation is convenient because the code naturally operates on one complete primitive.

For millions of primitive queries, the preferred representation may change.

A batch of spheres stored as:

```
struct Sphere {
    Vec3 center;
    float radius;
};

std::vector<Sphere> spheres;
```

offers simple ownership and iteration.

An alternative structure-of-arrays layout separates centres and radii:

```
std::vector<float> centerX;
std::vector<float> centerY;
std::vector<float> centerZ;
std::vector<float> radius;
```

This may improve SIMD utilization when processing large numbers of independent queries, although performance depends on the actual access pattern, vectorization strategy, cache behavior, and target hardware.

For triangle queries, the seven-region algorithm has data-dependent branches. On SIMD CPUs or SIMT GPUs, different lanes may enter different feature regions, potentially reducing throughput.

Possible production adaptations include batching by query type, using mask-based selection, minimizing duplicated calculations, precomputing reusable triangle data, and grouping spatially coherent queries.

But any transformation must preserve the numerical and geometric behavior, particularly near boundaries.

**Coordinate-space discipline**

A query point and a primitive must be expressed in compatible coordinate spaces.

If a triangle is stored in object space and the query arrives in world space, either transform the query into object space or transform the triangle into world space.

The distinction becomes important under nonuniform scale.

Euclidean distances and closest-point locations are not generally preserved by an arbitrary affine transformation. Transforming a world-space query into an object's local space and minimizing an ordinary local Euclidean distance may produce the wrong world-space closest point under nonuniform scaling.

For rigid transformations, which preserve distances, this issue does not arise.

For general affine transforms, use a world-space calculation or account for the induced metric.

Specifically, if $A$ is the linear transformation, a local displacement $\delta x$ has world-space squared distance:

$$
\|\!A\delta x\!\|^2 = \delta x^T A^T A\delta x
$$

Thus $A^TA$ acts as the metric matrix for world-space distance optimization expressed in local coordinates.

This is an important connection between geometry, linear algebra, and engine transform design.

### Research bridge and validation

**A research question worth testing in KairoGeometry**

A useful falsifiable research question is:

How much numerical reliability can KairoGeometry gain from scale-aware degeneracy tests and adaptive predicates without materially increasing typical closest-point and classification costs?

We can investigate this using three variants of the same geometry-query workload.

| Variant              | Numerical policy                                                   |
| -------------------- | ------------------------------------------------------------------ |
| A — Baseline         | Existing fixed-epsilon behavior                                    |
| B — Scale-aware      | Dimensionally consistent, workload-aware thresholds                |
| C — Robust predicate | Certified orientation signs where required, with adaptive fallback |

The experiment would use random and adversarial triangles, including triangles with extremely large coordinates, tiny edges, nearly collinear vertices, and points very close to edges and supporting planes.

It would also include realistic geometry distributions drawn from mesh and reconstruction workloads.

Relevant measurements would include classification disagreements against a high-precision reference, closest-point error, percentage of non-finite results, time per query, tail latency, and sensitivity to scale.

A crucial research distinction is that a tolerant application-level classification is not necessarily expected to agree with an exact mathematical sign. We must define separate ground truths for exact predicates and tolerance-based decisions.

The null hypothesis could be that improved numerical policies provide no meaningful reliability improvement on realistic inputs, or that any improvement comes with unacceptable runtime overhead.

This gives us an actual experimental question, rather than merely the statement that robust geometry is important.

**Reconstruction challenge**

Do these without referring to the derivations.

1. Starting from $Q(t)=A+t(B-A)$, derive the closest-point parameter by minimizing squared distance. Explain why a line uses the unconstrained result, a ray uses a lower bound, and a segment uses clamping.
2. Derive the general projection formula for a plane whose normal is not necessarily normalized. Explain precisely why the simpler formula fails for an arbitrary normal.
3. Prove that the Gram determinant used in triangle barycentrics equals the squared magnitude of the triangle's edge cross product.
4. Explain why barycentric containment alone cannot establish that a 3D point lies on a triangle.
5. Derive the vertex-A condition for the triangle closest-point algorithm, then reconstruct how the three edge regions and face region complete the solution.
6. Explain how a zero-radius sphere, a zero-length segment, and a collinear triangle should behave under different geometry-query APIs.
7. Identify the dimensional mismatch in a fixed-epsilon triangle degeneracy test and propose a scale-aware alternative.
8. Explain when the closest point can be transformed between local and world coordinates without changing the minimization result.

A strong mastery test is to implement a working closest-point-on-triangle routine from an empty source file, without copying the reference implementation, and then construct adversarial cases that challenge your own implementation.

**Compact recall sheet**

M067–M068 · Geometry reconstruction cues

Central model: geometry consists of representations, predicates, constructions, and well-defined numerical contracts.

Ray/line/segment: $P(t)=O+tD$. Their primary distinction is the permitted parameter domain.

Projection: minimize squared distance. The solution follows from orthogonality of the residual and the allowed direction.

Plane: $N\cdot X+d=0$. Signed Euclidean distance requires normalization.

Sphere: centre plus radius. Distinguish signed boundary distance, unsigned surface distance, and distance to the filled solid.

Triangle: ordered vertices determine edge directions, geometric normal, area, winding, and a two-dimensional affine coordinate system.

Barycentrics: $P=uA+vB+wC$, with $u+v+w=1$. Coplanarity is separate from containment.

Closest triangle point: choose among seven feature Voronoi regions: three vertices, three edges, one face.

Degeneracy: avoid assuming that every invalid or poorly conditioned object has a meaningful plane, normal, or inverse.

Numerical principle: tolerances depend on semantics, units, scale, conditioning, and whether exact decisions are required.


Phase 12 · Bounding Volumes, Frusta, Convex Geometry, Support Mapping, and Convex Hulls



Repository targets: `KairoGeometry/AABB.cppm`, `OBB.cppm`, `Capsule.cppm`, and `Frustum.cppm`.

Mastery objective: Understand how an engine represents bounded geometric objects, computes their spatial relationships, derives tight or conservative enclosing volumes, performs hierarchical visibility rejection, and reasons about arbitrary convex shapes.

This pass builds directly on the previous one. We already know how to describe individual geometric primitives and calculate closest points. Now we must answer a much more important systems question:

> When a scene contains millions of geometric primitives, how do we determine where they are, how large they are, whether they could interact, and whether expensive geometric algorithms are even necessary?

The answer begins with bounding volumes and ultimately leads to convex geometry, collision acceleration, BVHs, SAT, and GJK.

### Axis-Aligned Bounding Boxes

Original Parts 1133–1148

**Why bounding volumes exist**

Imagine a scene containing 500,000 triangles.

You want to know whether a ray intersects any triangle.

The naïve approach tests all 500,000 triangles individually. Each intersection can involve cross products, dot products, divisions, multiple branches, and additional memory accesses.

A more efficient approach surrounds groups of triangles with geometrically simpler volumes.

For example, surround 200 triangles with a box. Before checking those triangles, ask whether the ray intersects the box.

If the ray misses the box, it cannot intersect any triangle completely contained inside that box.

One inexpensive test has eliminated 200 potentially expensive tests.

This is the fundamental principle of conservative spatial acceleration:

$$
\boxed{\text{Actual geometry}\subseteq\text{Bounding volume}}
$$

For spatial pruning, a bounding volume must not exclude valid geometry.

It is allowed to include empty space, because this may produce additional candidate tests. But an incorrectly small bound can cause the engine to discard real geometry, creating a false negative.

This distinction between conservative rejection and exact geometric confirmation will become essential throughout the upcoming collision and spatial-acceleration phases.

**Parts 1133–1135 — AABB representations and conversions**

An Axis-Aligned Bounding Box, abbreviated AABB, is a rectangular box whose faces remain parallel to the axes of its defining coordinate system.

In 3D, it has six face planes: two perpendicular to X, two perpendicular to Y, and two perpendicular to Z.

An AABB is not defined by the orientation of an object inside it. It is defined by the minimum and maximum allowed coordinate values.

Min-max representation and the equivalent centre–half-extents representation of the same AABB.

**Min-max representation**

Let:

$$
m=(m_x,m_y,m_z),\qquad M=(M_x,M_y,M_z)
$$

The AABB is the Cartesian product of three closed intervals:

$$
\boxed{ B=[m_x,M_x]\times[m_y,M_y]\times[m_z,M_z] }
$$

For every point $P$ inside the box:

$$
m_x\le P_x\le M_x
$$

and similarly for Y and Z.

A valid nonempty AABB requires:

$$
m_i\le M_i
$$

for every coordinate, with finite coordinates under the usual finite-bounds contract.

Equality on one or more axes is permitted. For example, an AABB with zero Z thickness represents a rectangle lying in a plane.

If all three dimensions are zero, the box represents a point.

These lower-dimensional cases are geometrically valid and need not be confused with an empty AABB.

**Centre–half-extents representation**

An equivalent representation uses centre $C$ and half-extents $E$:

$$
C=(c_x,c_y,c_z)
$$

$$
E=(e_x,e_y,e_z)
$$

with:

$$
e_x,e_y,e_z\ge0
$$

The box contains all points satisfying:

$$
|P_x-c_x|\le e_x
$$

$$
|P_y-c_y|\le e_y
$$

$$
|P_z-c_z|\le e_z
$$

This representation is useful for transformations, support mapping, SAT, and symmetric reasoning around the centre.

**Deriving the conversion**

The centre of each interval is the midpoint:

$$
\boxed{C=\frac{m+M}{2}}
$$

The half-extents are half of the interval widths:

$$
\boxed{E=\frac{M-m}{2}}
$$

Conversely:

$$
\boxed{m=C-E,\qquad M=C+E}
$$

For example, consider:

$$
m=(1,-2,3),\quad M=(7,4,11)
$$

Then:

$$
C=(4,1,7)
$$

and:

$$
E=(3,3,4)
$$

Both representations describe the exact same geometric set.

An implementation should distinguish half-extents from full dimensions. Confusing them produces a box twice as large or half as large as intended.

The basic formulas also raise a numerical issue. Calculating `(min + max) / 2` may overflow even when the midpoint is representable. Computing `min + (max - min) / 2` can avoid that particular overflow but may itself overflow when the interval spans sufficiently extreme values.

For practical scene coordinates, the simpler expressions are often adequate. A fully general numerical geometry library must define its supported coordinate range.

**Parts 1136–1138 — Constructing, expanding, and merging AABBs**

**Constructing an AABB from a set of points**

Given points:

$$
P_0,P_1,\ldots,P_{n-1}
$$

the smallest axis-aligned box containing all points is obtained by taking coordinate-wise extrema:

$$
\boxed{ m_j=\min_i(P_i)_j }
$$

$$
\boxed{ M_j=\max_i(P_i)_j }
$$

for $j\in\\{x,y,z\\}$.

This is the minimal AABB because every valid enclosing AABB must extend at least as far as each minimum and maximum coordinate.

No smaller coordinate interval can contain the extreme points.

A single sequential pass computes all six extrema:

```
AABB bounds = AABB::Empty();

for (const Vec3& point : meshVertices) {
    bounds.expand(point);
}
```

Here `AABB::Empty()` is a sentinel representing an accumulator that has not yet received any valid points.

A common implementation initializes minimum coordinates to positive infinity and maximum coordinates to negative infinity. Your KairoGeometry implementation instead uses the largest and lowest finite representable values.

Both can work when the API handles invalid or empty boxes consistently.

The complexity is:

$$
T(n)=O(n),\qquad S(n)=O(1)
$$

excluding the input data.

This is optimal for an arbitrary unsorted point set because every point could contain a new extreme coordinate.

**Expanding by one point**

Suppose an existing box has corners $m,M$, and we add a new point $P$.

The new bounds are:

$$
m'=\min(m,P)
$$

$$
M'=\max(M,P)
$$

where `min` and `max` are applied independently to each coordinate.

For example:

$$
m=(1,2,3),\quad M=(4,5,6)
$$

and:

$$
P=(-2,3,9)
$$

The expanded box becomes:

$$
m'=(-2,2,3),\quad M'=(4,5,9)
$$

Expansion is monotonic: adding points cannot shrink a box.

This is an invariant we can test automatically.

**Merging two boxes**

Let boxes $A$ and $B$ have extrema:

$$
(m_A,M_A),\quad(m_B,M_B)
$$

The smallest AABB containing both boxes is:

$$
\boxed{ m=\min(m_A,m_B) }
$$

$$
\boxed{ M=\max(M_A,M_B) }
$$

This operation computes the axis-aligned bounding box of their union. It does not compute the geometric union itself.

Two spatially separated boxes will have a merged bounding box containing empty space between them.

This matters enormously in a BVH. Parent nodes contain their children, but their bounds usually also contain empty space.

Tighter parent bounds tend to improve traversal pruning, although the optimal hierarchy also depends on overlap, construction cost, primitive distribution, and query workload.

**Parts 1139–1140 — Surface area and volume**

Let the full box dimensions be:

$$
d=M-m=(d_x,d_y,d_z)
$$

**Volume**

The volume of a rectangular solid is the product of its side lengths:

$$
\boxed{ V=d_xd_yd_z }
$$

Its physical units are cubic length units.

For a valid box with zero thickness along one axis, the volume is zero.

**Surface area**

The box has three pairs of opposite faces.

The XY faces each have area $d_xd_y$. The YZ faces each have area $d_yd_z$. The ZX faces each have area $d_zd_x$.

Therefore:

$$
\boxed{ S=2(d_xd_y+d_yd_z+d_zd_x) }
$$

For a box with dimensions $2,3,4$:

$$
V=2(3)(4)=24
$$

$$
S=2(6+12+8)=52
$$

**Why surface area is often more useful than volume**

In ray tracing and BVH construction, surface area appears in the Surface Area Heuristic (SAH).

A simplified intuition is that, for suitable ray distributions, the likelihood of a ray encountering a bounded region is related to that region's surface area.

A common binary BVH cost model is:

$$
C= C_{\mathrm{traversal}} + \frac{S_L}{S_P}C_L + \frac{S_R}{S_P}C_R
$$

where $S_L,S_R,S_P$ denote left-child, right-child, and parent surface areas. $C_L$ and $C_R$ are estimated costs of processing those children.

These ratios are approximations based on assumptions about rays and the distribution of work, not universal probabilities for every scene and query distribution.

Why not use volume?

Volume alone often gives a poorer approximation of ray encounter probability. Very thin but wide geometry can have little volume while still presenting a large projected target to rays.

We will derive SAH much more formally in Phase 14.

For now, remember that AABB surface area is not merely a geometric convenience; it becomes a cost-model ingredient for spatial acceleration.

**Parts 1141–1143 — Point containment, closest point, and squared distance**

**Containment**

A point lies inside or on the closed AABB when every coordinate belongs to its corresponding interval:

$$
\boxed{ m_i\le P_i\le M_i }
$$

for all three axes.

The test performs six scalar comparisons and has constant time and space complexity.

An epsilon-extended version can test:

$$
m_i-\epsilon\le P_i\le M_i+\epsilon
$$

where the epsilon must have the same units as the coordinates.

Such a test answers whether a point belongs to an expanded tolerance region, not whether it belongs exactly to the mathematical box.

**Deriving the closest point**

Given a point $P$, we want:

$$
\min_{Q\in B}\|P-Q\|^2
$$

Expand:

$$
\|P-Q\|^2= (P_x-Q_x)^2+ (P_y-Q_y)^2+ (P_z-Q_z)^2
$$

The constraints on $Q_x,Q_y,Q_z$ are independent.

Consequently, the minimization separates into three one-dimensional problems.

For each axis, the closest permitted coordinate is:

$$
\boxed{ Q_i=\operatorname{clamp}(P_i,m_i,M_i) }
$$

This is not an arbitrary implementation trick. It is the exact solution of three independent constrained quadratic minimizations.

If a coordinate is inside its interval, retain it. If it falls below the interval, use the minimum endpoint. If it exceeds the interval, use the maximum endpoint.

**Worked example**

Let:

$$
m=(0,0,0),\quad M=(4,3,2)
$$

and:

$$
P=(6,-2,1)
$$

Clamp independently:

$$
Q_x=4,\quad Q_y=0,\quad Q_z=1
$$

Therefore:

$$
\boxed{Q=(4,0,1)}
$$

Squared distance:

$$
d^2=(6-4)^2+(-2-0)^2+(1-1)^2
$$

$$
\boxed{d^2=8}
$$

and:

$$
d=\sqrt8=2\sqrt2
$$

An equivalent formula avoids explicitly storing the nearest point.

Define the outside displacement along each axis:

$$
\delta_i=\max(m_i-P_i,\ 0,\ P_i-M_i)
$$

Then:

$$
\boxed{ d^2(P,B)=\delta_x^2+\delta_y^2+\delta_z^2 }
$$

**Distance to the solid versus distance to its boundary**

The clamp formula returns the nearest point in the filled AABB.

For a point inside the box, the nearest point in the solid is the point itself, and the distance is zero.

This differs from finding the nearest point on the box's surface.

For a point inside the box, the nearest boundary lies along the axis with the smallest distance to one of the six faces.

Its boundary distance is:

$$
\min_i(P_i-m_i,\ M_i-P_i)
$$

This distinction matters in penetration recovery. A physics solver may need the distance and direction to the nearest boundary rather than the distance to the filled solid.

**Parts 1144–1145 — AABB overlap and intersection construction**

Two AABBs overlap if their coordinate intervals intersect on every axis.

Let boxes $A$ and $B$ have minima and maxima $m_A,M_A,m_B,M_B$.

Two intervals are separated on an axis if:

$$
M_{A,i}\<m_{B,i}
$$

or:

$$
M_{B,i}\<m_{A,i}
$$

Consequently, the boxes overlap or touch when:

$$
\boxed{ M_{A,i}\ge m_{B,i} \quad\land\quad M_{B,i}\ge m_{A,i} }
$$

for all three axes.

This is a special case of the Separating Axis Theorem.

For axis-aligned boxes, the three coordinate axes are sufficient. For oriented boxes, additional axes are required.

**Why all three axes must overlap**

An AABB is the Cartesian product of three intervals.

The intersection of two such products is nonempty exactly when each corresponding pair of intervals has nonempty intersection.

Thus three independent interval-overlap tests are both necessary and sufficient.

If any one axis is separated, the boxes are separated in 3D.

**Touching semantics**

Consider:

$$
A_x=[0,2],\quad B_x=[2,5]
$$

Their intersection contains the coordinate $x=2$.

With inclusive inequalities, the intervals overlap at a boundary.

With strict inequalities, they would not count as overlapping.

Both conventions can be useful, but the choice must be deliberate.

Collision candidate generation typically benefits from including touching configurations. Contact-generation code may later distinguish zero penetration from positive penetration.

**Constructing the intersection box**

If the boxes overlap, the exact intersection of two AABBs is another AABB:

$$
\boxed{ m_I=\max(m_A,m_B) }
$$

$$
\boxed{ M_I=\min(M_A,M_B) }
$$

For example:

$$
A=[(0,0,0),(4,4,4)]
$$

$$
B=[(2,1,-1),(6,3,2)]
$$

Their intersection is:

$$
\boxed{I=[(2,1,0),(4,3,2)]}
$$

If any minimum component exceeds the corresponding maximum component, the intersection is empty.

This is different from merging boxes. Merging produces the smallest enclosing AABB; intersection produces the common region.

**Engine implications**

For broadphase collision detection, AABB overlap means two objects might collide, not that their actual geometry intersects.

Two objects can occupy opposite corners of overlapping bounding boxes without touching.

These are false positives from the broadphase, which the narrowphase must resolve.

A correct broadphase must prioritize avoiding false negatives while controlling the number of false-positive candidate pairs.

**Parts 1146–1148 — Transforming AABBs**

An AABB is axis-aligned in its chosen coordinate frame.

After transforming an object, its original local-space AABB may no longer be axis-aligned in world space.

Therefore, transforming an AABB means finding a new axis-aligned box that encloses all transformed points.

This is one of the most important operations in scene graphs, animation, collision broadphases, render culling, and BVH updates.

**Translation**

Let the translation vector be $T$.

Every point transforms as:

$$
P'=P+T
$$

The transformed AABB is:

$$
\boxed{ m'=m+T,\quad M'=M+T }
$$

Translation preserves the box dimensions, volume, and surface area.

**Axis-aligned scale**

Let scaling be:

$$
S=\operatorname{diag}(s_x,s_y,s_z)
$$

with no rotation or shear.

If every scale component is positive:

$$
m'=Sm,\quad M'=SM
$$

If a scale component is negative, minimum and maximum reverse on that axis.

The general component-wise solution is:

$$
\boxed{ m'_i=\min(s_im_i,s_iM_i) }
$$

$$
\boxed{ M'_i=\max(s_im_i,s_iM_i) }
$$

For scale followed by translation, add the translation to both results.

**General affine transformations**

Suppose we have:

$$
\boxed{ P'=AP+T }
$$

where $A$ is a $3\times3$ linear transformation and $T$ is translation.

The matrix $A$ may include rotation, scale, reflection, or shear.

A straightforward method transforms all eight original AABB corners and constructs a new box from their coordinate-wise extrema.

This works because an affine transformation maps the original box to a parallelepiped, and each transformed coordinate is an affine function of the original coordinates. The extrema of an affine function over a convex polytope occur at vertices.

Therefore all eight corners are sufficient to determine the exact enclosing AABB in ideal arithmetic.

This method is used in the bounding-box transformation described by Physically Based Rendering.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org



**The more efficient absolute-matrix method**

We can derive the transformed bounds without enumerating all eight corners.

Write the original box as:

$$
P=C+q
$$

where:

$$
-e_i\le q_i\le e_i
$$

Apply the affine transformation:

$$
P'=A(C+q)+T
$$

Expand:

$$
P'=AC+T+Aq
$$

The transformed centre is:

$$
\boxed{C'=AC+T}
$$

Now consider output coordinate $j$:

$$
(Aq)_j=\sum_{i=0}^{2}A_{ji}q_i
$$

Since each $q_i$ is independently restricted to $[-e_i,e_i]$, the maximum possible magnitude of this coordinate is:

$$
\max |(Aq)_j| = \sum_i|A_{ji}|e_i
$$

Therefore the new half-extents are:

$$
\boxed{ E'=|A|E }
$$

Here $|A|$ means element-wise absolute value, not the matrix determinant or matrix norm.

The final bounds are:

$$
\boxed{ m'=C'-E' }
$$

$$
\boxed{ M'=C'+E' }
$$

This is the absolute-matrix method, and the derivation establishes that it is exact for the enclosing AABB of an affine-transformed box in real arithmetic.

**Worked rotation example**

Consider a two-dimensional rectangle embedded in 3D with half-extents:

$$
E=(2,1,0)
$$

Rotate it $45^\circ$ about the Z axis.

The rotation matrix is:

$$
R= \begin{pmatrix} \cos\theta&-\sin\theta&0\\\ \sin\theta&\cos\theta&0\\\ 0&0&1 \end{pmatrix}
$$

At $\theta=45^\circ$:

$$
|\cos\theta|=|\sin\theta|=\frac{\sqrt2}{2}
$$

Apply the absolute matrix:

$$
E'_x=2\frac{\sqrt2}{2}+1\frac{\sqrt2}{2}
$$

$$
E'_y=2\frac{\sqrt2}{2}+1\frac{\sqrt2}{2}
$$

Therefore:

$$
\boxed{ E'=\left(\frac{3\sqrt2}{2},\frac{3\sqrt2}{2},0\right) }
$$

The original full dimensions were $4\times2$.

After rotation, the world-axis-aligned enclosure has full X and Y dimensions:

$$
3\sqrt2\approx4.243
$$

Rotation has not changed the object's area, but it has increased the area of its axis-aligned enclosure.

This explains why AABBs around rotating objects may become increasingly loose at certain orientations.

Illustrative relative geometry: an object's shape is preserved under rotation, but its axis-aligned bounding enclosure changes.

**Numerical and transform caveats**

The eight-corner method and the absolute-matrix method have the same mathematical result for an affine transform, but floating-point rounding can produce slightly different numerical bounds.

For conservative culling, the implementation may need outward-rounded bounds or carefully chosen expansion margins so arithmetic error does not accidentally exclude the transformed geometry.

The affine assumption is also important. A general projective transform involving homogeneous perspective division does not behave like $P'=AP+T$. Merely transforming eight corners is not always sufficient to obtain safe projected bounds, particularly when geometry crosses a problematic projective plane.

In ordinary engine architecture, world-space BVH/AABB updates should use affine object-to-world transforms. Camera projection belongs to a different stage.

**Your actual `AABB.cppm`**

Your [KairoGeometry/AABB.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/AABB.cppm?utm_source=chatgpt.com) currently implements `TransformAABB()` by transforming all eight corners of the box through `TransformPoint()` and expanding the result.

That is the canonical general affine approach.

The absolute-matrix formula is a worthwhile alternative to implement and benchmark, particularly for very large scene updates, because it replaces eight transformed corners with one centre transform and a compact extents calculation.

We should benchmark the implementations under the same matrix convention and floating-point constraints before asserting a specific performance improvement.

Your implementation also includes the expected containment, closest-point, overlap, intersection, and merging functionality. It provides a good basis for constructing property-based tests that compare the eight-corner and absolute-matrix transform methods.

### Oriented Bounding Boxes

Original Parts 1149–1154

**Why an OBB exists**

An AABB is computationally convenient, but it cannot rotate independently of its coordinate axes.

Imagine a long, thin beam rotated diagonally through a scene.

Its world-space AABB might contain much more empty space than the beam itself.

This increases false positives in spatial searches and collision broadphases.

An Oriented Bounding Box (OBB) represents a rectangular box with its own local orthonormal coordinate frame.

The box rotates with the object while preserving its rectangular shape.

**Parts 1149–1150 — OBB representation and invariants**

An OBB can be represented by:

$$
(C,u_0,u_1,u_2,E)
$$

where:

- $C$ is the world-space centre.
- $u_0,u_1,u_2$ are the three local unit axes expressed in world coordinates.
- $E=(e_0,e_1,e_2)$ contains the nonnegative half-extents.

The word orthonormal means:

$$
\boxed{\|u_i\|=1}
$$

and:

$$
\boxed{u_i\cdot u_j=0\quad(i\ne j)}
$$

For a conventional right-handed frame:

$$
u_0\times u_1=u_2
$$

Although the last condition is useful for frame conventions, an orthonormal basis with reversed handedness also describes the same geometric category of oriented box.

Any point in the box can be written as:

$$
\boxed{ P=C+\alpha_0u_0+\alpha_1u_1+\alpha_2u_2 }
$$

subject to:

$$
|\alpha_i|\le e_i
$$

This is the fundamental OBB equation.

In matrix form, let:

$$
U= \begin{bmatrix} u_0&u_1&u_2 \end{bmatrix}
$$

Then:

$$
P=C+U\alpha
$$

where $\alpha$ is the point's local coordinate vector.

Because $U$ is orthonormal:

$$
\boxed{U^{-1}=U^T}
$$

That property makes OBB coordinate conversion extremely efficient.

**Part 1151 — Transforming a point into OBB local coordinates**

Starting from:

$$
P=C+U\alpha
$$

subtract the centre:

$$
P-C=U\alpha
$$

Multiply by $U^T$:

$$
U^T(P-C)=U^TU\alpha
$$

Since:

$$
U^TU=I
$$

we obtain:

$$
\boxed{\alpha=U^T(P-C)}
$$

Expanding the matrix multiplication:

$$
\boxed{ \alpha_i=(P-C)\cdot u_i }
$$

This is why the OBB local-space transform can be implemented using three dot products.

No general matrix inverse is required.

**Numerical example**

Let:

$$
C=(3,2,0)
$$

and a 45-degree-rotated basis:

$$
u_0=\frac{1}{\sqrt2}(1,1,0)
$$

$$
u_1=\frac{1}{\sqrt2}(-1,1,0)
$$

$$
u_2=(0,0,1)
$$

Suppose:

$$
P=(3+\sqrt2,2+\sqrt2,0)
$$

Then:

$$
P-C=(\sqrt2,\sqrt2,0)
$$

Project onto the first axis:

$$
\alpha_0=(P-C)\cdot u_0=2
$$

The second axis gives:

$$
\alpha_1=0
$$

and the third:

$$
\alpha_2=0
$$

Therefore the point has local coordinates:

$$
\boxed{\alpha=(2,0,0)}
$$

An otherwise complicated world-space position is now a simple local displacement along the box's first axis.

**Parts 1152–1154 — OBB containment, closest point, and distance**

**Containment**

Transform the query point to local coordinates:

$$
\alpha=U^T(P-C)
$$

Then test:

$$
\boxed{ |\alpha_0|\le e_0,\quad |\alpha_1|\le e_1,\quad |\alpha_2|\le e_2 }
$$

This is exactly the AABB containment problem in the box's own orthonormal frame.

**Closest point**

Clamp the local coordinates:

$$
\beta_i=\operatorname{clamp}(\alpha_i,-e_i,e_i)
$$

Transform back to world space:

$$
\boxed{ Q=C+U\beta }
$$

The Euclidean distance is:

$$
\boxed{ d(P,\mathrm{OBB})=\|P-Q\| }
$$

We can perform the minimization in local coordinates because orthonormal transformations preserve Euclidean distances:

$$
\|P-Q\|^2= \|U^T(P-C)-U^T(Q-C)\|^2
$$

This equality would not generally hold for arbitrary nonuniform scaling or shear.

**What breaks when the OBB axes are not orthonormal?**

Suppose the axes are not mutually perpendicular.

Then:

$$
U^TU\ne I
$$

Consequently:

$$
U^{-1}\ne U^T
$$

Using dot products as if they were the inverse transform gives incorrect local coordinates.

Likewise, independently clamping those coordinates no longer solves the correct Euclidean closest-point problem.

The geometric shape may be a general parallelepiped rather than an OBB.

This reveals why the orthonormal-axis invariant is not a minor implementation preference. It is a mathematical precondition for the entire efficient query algorithm.

**Your actual `OBB.cppm`**

Your [KairoGeometry/OBB.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/OBB.cppm?utm_source=chatgpt.com) stores `Center`, an array of three `Axes`, and `HalfExtents`.

`WorldToLocalPoint()` calculates three dot products with the axes. `ClosestPoint()` clamps those local coordinates and transforms the result back into world space.

That is exactly the mathematical approach derived above.

The module also exposes `HasNormalizedAxes()`, `HasOrthogonalAxes()`, and `IsValid()`, which is valuable because these queries depend on valid axes.

One subtle point: its `NormalizeAxes()` operation normalizes the axes individually. Individual normalization does not necessarily make the axes mutually orthogonal.

For example, the two unit vectors:

$$
u_0=(1,0,0)
$$

$$
u_1=(1/\sqrt2,1/\sqrt2,0)
$$

are already normalized but not orthogonal.

Normalizing them again does not repair the frame.

To construct a valid OBB frame from imperfect axes, an explicit orthonormalization procedure, a validated rotation matrix, or a unit quaternion is preferable, depending on the source representation.

**OBB versus AABB in an engine**

The OBB tends to represent rotated elongated geometry more tightly.

But tighter bounds do not automatically mean faster queries.

An AABB is inexpensive to construct, update, overlap-test, and vectorize. An OBB requires a local frame and more arithmetic for many queries.

A dynamic physics engine might use OBBs for accurate box-shaped colliders but AABBs in its broadphase spatial index.

That combination is common in principle:

$$
\text{OBB or shape collider} \rightarrow \text{world-space AABB proxy} \rightarrow \text{broadphase} \rightarrow \text{precise narrowphase}
$$

It avoids paying the cost of OBB-to-OBB testing for every potential pair.

### Capsules

Original Parts 1155–1157

**Why capsules are useful**

A capsule is the solid obtained by sweeping a sphere of radius $r$ along a line segment.

Equivalently, it is the Minkowski sum:

$$
\boxed{ K=[A,B]\oplus B_r }
$$

where $[A,B]$ is the central line segment and $B_r$ is a solid ball of radius $r$ centred at the origin.

The Minkowski sum of sets $X$ and $Y$ is:

$$
X\oplus Y=\\{x+y:x\in X,\ y\in Y\\}
$$

The capsule therefore contains every point whose distance to the central segment is at most $r$.

This is a powerful representation because it turns a three-dimensional rounded volume into a one-dimensional closest-point problem plus a radius.

Capsules are useful for character controllers, collision proxies, limbs, skeletal systems, swept paths, and certain continuous-collision calculations.

Capsule as a segment swept by a radius-r ball. The dashed centreline is its skeletal segment.

**Part 1155 — Segment-plus-radius representation**

A capsule needs:

$$
A,\ B,\ r
$$

with:

$$
r\ge0
$$

No explicit rotational quaternion is necessary when the endpoints are already expressed in the desired coordinate system.

If the endpoints coincide:

$$
A=B
$$

the capsule reduces to a sphere.

If:

$$
r=0
$$

the capsule reduces to its central segment.

If both conditions hold, the capsule becomes a single point.

These are mathematically meaningful degeneracies.

The capsule's axis length is:

$$
L=\|B-A\|
$$

Its complete end-to-end length is:

$$
H=L+2r
$$

For a nondegenerate axis, its volume is the volume of a cylinder plus the volume of two hemispheres:

$$
\boxed{ V=\pi r^2L+\frac43\pi r^3 }
$$

Its surface area is:

$$
\boxed{ S=2\pi rL+4\pi r^2 }
$$

The formulas reduce correctly to sphere formulas when $L=0$.

**Part 1156 — Closest-point computation**

The capsule's geometry is defined relative to its central segment.

Given query point $P$, first find the closest point on the axis segment.

Set:

$$
D=B-A
$$

For $A\ne B$:

$$
t=\operatorname{clamp} \left( \frac{(P-A)\cdot D}{D\cdot D}, 0,1 \right)
$$

Then:

$$
\boxed{Q_{\mathrm{axis}}=A+tD}
$$

This result is the closest point on the capsule's central skeleton.

Next calculate:

$$
V=P-Q_{\mathrm{axis}}
$$

If $V\ne0$, the corresponding radial point on the capsule boundary is:

$$
\boxed{ Q_{\mathrm{surface}} = Q_{\mathrm{axis}} + r\frac{V}{\|V\|} }
$$

For query points outside the capsule, this is the nearest surface point.

For interior queries away from the central axis, it identifies a boundary point along the nearest-segment radial direction; this also gives the nearest boundary point for a standard capsule.

When the query lies exactly on the axis, the radial direction is undefined. A deterministic perpendicular direction is needed, and a special case is required when the segment itself has zero length.

**Part 1157 — Point-to-capsule distance**

The signed distance to a capsule's boundary is:

$$
\boxed{ d_{\mathrm{signed}}(P)= \|P-Q_{\mathrm{axis}}\|-r }
$$

For the filled capsule, unsigned solid distance is:

$$
\boxed{ d_{\mathrm{solid}}(P)= \max(0,d_{\mathrm{signed}}(P)) }
$$

Squared distance to the filled capsule is:

$$
\boxed{ d_{\mathrm{solid}}^2(P)= \left[\max(0,d_{\mathrm{signed}}(P))\right]^2 }
$$

The containment test follows immediately:

$$
\boxed{ P\in K \iff \operatorname{distance}^2(P,[A,B])\le r^2 }
$$

This means a capsule containment query can often avoid square roots completely.

**Worked example**

Consider:

$$
A=(0,0,0),\quad B=(0,4,0),\quad r=1
$$

and query:

$$
P=(3,2,0)
$$

The segment axis is:

$$
D=(0,4,0)
$$

Projection gives:

$$
t=\frac{(3,2,0)\cdot(0,4,0)}{16} =\frac{8}{16}=0.5
$$

The nearest axis point is:

$$
Q_{\mathrm{axis}}=(0,2,0)
$$

Distance from the axis:

$$
\|P-Q_{\mathrm{axis}}\|=3
$$

Therefore:

$$
d_{\mathrm{solid}}=3-1=2
$$

The closest surface point is:

$$
\boxed{Q_{\mathrm{surface}}=(1,2,0)}
$$

And the Euclidean distance from the query to that surface point is exactly $2$.

**The exact-axis degeneracy and an issue in your code**

I inspected [KairoGeometry/Capsule.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Capsule.cppm?utm_source=chatgpt.com).

The implementation correctly computes the nearest point on the axis and uses a segment projection.

However, when the vector from the axis point to the query is approximately zero, its `ClosestPoint()` method falls back to:

```
return axisPoint + Direction() * Radius;
```

Here `Direction()` is the direction along the capsule's axis.

For a query exactly on the central axis, this can return a point inside the capsule rather than a point on its surface.

For example, let the capsule axis extend from $y=0$ to $y=10$, with radius $1$, and query:

$$
P=(0,5,0)
$$

The nearest axis point is:

$$
Q_{\mathrm{axis}}=P
$$

If the axis direction is $+Y$, the fallback gives:

$$
Q=(0,6,0)
$$

But this point remains inside the capsule. Its actual boundary is one unit away in any perpendicular radial direction.

A valid deterministic boundary point would be:

$$
Q=(1,5,0)
$$

The fallback should therefore construct a stable vector perpendicular to the axis, rather than use the axis direction itself. When the axis is degenerate, the capsule reduces to a sphere and a fixed radial direction can be chosen.

This is a concrete, repository-verified geometry concern worth adding to KairoGeometry's tests.

**Constructing a stable perpendicular vector**

For a nonzero capsule direction $D$, normalize it:

$$
\hat D=\frac{D}{\|D\|}
$$

Choose a coordinate axis $R$ that is not nearly parallel to $\hat D$.

Then compute:

$$
U=\frac{\hat D\times R}{\|\hat D\times R\|}
$$

Because $U\cdot\hat D=0$, the vector $U$ is a valid radial direction.

To avoid near-zero cross products, choose the reference axis using the smallest-magnitude component of $\hat D$.

For example, if $\hat D=(0,1,0)$, choosing $R=(1,0,0)$ gives:

$$
\hat D\times R=(0,0,-1)
$$

This is a valid perpendicular fallback.

The general lesson is that a robust geometric fallback must satisfy the geometric invariant of the result, not merely return a finite vector.

### Frusta, Planes, and Visibility Classification

Original Parts 1158–1163

We now move from representing individual objects to answering whether those objects belong to a spatial viewing region.

This is one of the fundamental steps in a renderer.

The camera does not need to draw every object in the entire world. It primarily needs geometry capable of contributing to the current image.

A view frustum is the region of space corresponding to a camera's viewing volume.

For a conventional finite perspective camera, the frustum resembles a truncated pyramid.

Conceptual finite camera frustum. A perspective camera's viewing volume is bounded by near, far, left, right, top, and bottom planes.

**Part 1158 — Frustum representation by six planes**

A finite perspective frustum can be represented by six oriented plane inequalities.

Let plane $i$ have coefficients:

$$
(n_i,d_i)
$$

with its interior side defined by:

$$
n_i\cdot P+d_i\ge0
$$

Then the frustum is the intersection of six half-spaces:

$$
\boxed{ F=\bigcap_{i=0}^{5} \\{P:n_i\cdot P+d_i\ge0\\} }
$$

The six planes are conventionally called Left, Right, Bottom, Top, Near, and Far.

The critical invariant is consistent orientation. Every normal must point toward the half-space designated as the frustum interior.

**Why a frustum is an intersection of half-spaces**

Imagine a single side plane of the viewing pyramid.

That plane divides all of 3D space into two half-spaces.

Only one contains points potentially visible through that side of the camera.

The left plane excludes points too far left; the right plane excludes points too far right; and the remaining planes impose corresponding constraints.

The points satisfying every constraint form the frustum.

This is an example of an important general fact:

The intersection of convex sets is convex.

Every half-space is convex, so a frustum represented as an intersection of half-spaces is also convex.

That fact will let us use convexity, support maps, and other geometric tools with frusta.

**Deriving frustum planes from a view-projection matrix**

This derivation is essential because it connects the representation to the actual graphics pipeline.

Assume column vectors and a combined matrix:

$$
M=P_{\mathrm{projection}}V_{\mathrm{view}}
$$

A world-space homogeneous point is:

$$
X=(x,y,z,1)^T
$$

Its clip-space coordinates are:

$$
X_c=MX
$$

Write:

$$
X_c=(x_c,y_c,z_c,w_c)^T
$$

For the conventional Vulkan/Metal clip-depth convention used by your current frustum module:

$$
-w_c\le x_c\le w_c
$$

$$
-w_c\le y_c\le w_c
$$

$$
0\le z_c\le w_c
$$

These inequalities define the valid homogeneous clip volume, before perspective division.

**Left clipping plane**

The left boundary is:

$$
x_c\ge-w_c
$$

Rearrange:

$$
w_c+x_c\ge0
$$

Since:

$$
x_c=M_0X,\qquad w_c=M_3X
$$

where $M_i$ means matrix row $i$, we obtain:

$$
\boxed{ (M_3+M_0)X\ge0 }
$$

Therefore the left frustum plane coefficients are:

$$
\boxed{L=M_3+M_0}
$$

**Right clipping plane**

The right constraint is:

$$
x_c\le w_c
$$

Thus:

$$
w_c-x_c\ge0
$$

and:

$$
\boxed{R=M_3-M_0}
$$

**Bottom and top**

Similarly:

$$
y_c\ge-w_c
$$

gives:

$$
\boxed{B=M_3+M_1}
$$

while:

$$
y_c\le w_c
$$

gives:

$$
\boxed{T=M_3-M_1}
$$

**Depth planes**

For zero-to-one clip depth, the two depth inequalities are:

$$
z_c\ge0
$$

and:

$$
w_c-z_c\ge0
$$

Therefore:

$$
\boxed{D_0=M_2}
$$

$$
\boxed{D_1=M_3-M_2}
$$

For a conventional projection, these correspond to the geometric near and far boundaries in the expected order.

However, with reverse-Z projection, their physical near/far roles may exchange even though the algebraic inequalities remain the same.

An engine must distinguish the clip-depth inequality from the physical near/far meaning.

With infinite-far projection, one depth boundary may no longer correspond to a finite geometric plane, and the frustum may not possess eight finite corners.

**Plane normalization**

Each extracted plane is a homogeneous coefficient vector:

$$
(a,b,c,d)
$$

Define:

$$
L=\sqrt{a^2+b^2+c^2}
$$

For nonzero $L$, normalize all four coefficients:

$$
(a',b',c',d')= \frac{(a,b,c,d)}{L}
$$

Dividing the normal without dividing $d$ would change the plane's location.

The normalized plane supports signed-distance queries:

$$
s(P)=a'x+b'y+c'z+d'
$$

It also makes radius-based sphere-frustum tests convenient.

**Other graphics conventions**

The formulas change when clip-space conventions change.

For an OpenGL-style depth range of $[-w_c,w_c]$, the two depth inequalities are:

$$
z_c+w_c\ge0
$$

and:

$$
w_c-z_c\ge0
$$

yielding:

$$
D_0=M_3+M_2,\qquad D_1=M_3-M_2
$$

Matrix orientation also matters. A row-vector convention changes how the same mathematical operation is expressed in storage and indexing.

Therefore copying a plane-extraction routine from another graphics engine without checking its conventions can produce incorrect culling even if the matrix arithmetic compiles perfectly.

**Your actual `Frustum.cppm`**

Your [KairoGeometry/Frustum.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Frustum.cppm?utm_source=chatgpt.com) documents row-major matrix storage, column-vector multiplication, and a zero-to-one clip-depth convention.

It extracts the planes using the row combinations derived above.

This is a valuable example of code whose correctness depends on architectural conventions beyond the function itself.

The important verification task is to ensure that the actual projection matrices used by the renderer, including any reverse-Z modes, agree with these assumptions and with the semantic names of the extracted planes.

**Part 1159 — Frustum corner representation**

A finite, nondegenerate frustum has eight corner points.

Four belong to the near plane and four to the far plane.

One way to obtain these corners is to invert the combined view-projection matrix and unproject the eight corners of normalized device coordinates.

Another is to intersect triples of frustum planes.

**Deriving three-plane intersection**

Given three planes:

$$
n_1\cdot X+d_1=0
$$

$$
n_2\cdot X+d_2=0
$$

$$
n_3\cdot X+d_3=0
$$

form the matrix:

$$
N= \begin{pmatrix} n_1^T\\\ n_2^T\\\ n_3^T \end{pmatrix}
$$

Then:

$$
NX= -\begin{pmatrix} d_1\\\d_2\\\d_3 \end{pmatrix}
$$

If the determinant is nonzero, the system has a unique solution.

The determinant is:

$$
D=n_1\cdot(n_2\times n_3)
$$

Using the inverse of the row-vector matrix, or equivalently Cramer's rule, gives:

$$
\boxed{ X= \frac{ -d_1(n_2\times n_3) -d_2(n_3\times n_1) -d_3(n_1\times n_2) }{ n_1\cdot(n_2\times n_3) } }
$$

This formula reconstructs a corner without explicitly computing a general $3\times3$ matrix inverse.

For instance, the near-left-bottom corner is the intersection of the Near, Left, and Bottom planes.

**Degenerate intersections**

If:

$$
|D|\approx0
$$

the planes may be linearly dependent or nearly so.

Their intersection may be nonexistent, nonunique, or numerically ill-conditioned.

The algorithm should signal failure rather than blindly dividing by a small determinant.

This is especially relevant when working with infinite-far projections, extreme field-of-view values, or malformed matrices.

Your `Frustum.cppm` uses the triple-plane-intersection formula inside `GetCorners()` and returns `false` when the determinant fails an epsilon-based test.

That is a reasonable failure-reporting interface. The numerical threshold and the handling of intentionally infinite frusta remain important validation concerns.

**Part 1160 — Point containment in a frustum**

With six inward-facing plane inequalities, point containment is:

$$
\boxed{ P\in F \iff n_i\cdot P+d_i\ge0 \quad\forall i }
$$

The algorithm evaluates each plane and stops immediately if any one constraint fails.

This is an $O(1)$ operation because the number of planes is fixed.

The same structure generalizes to a convex polyhedron with $m$ half-spaces, where it takes $O(m)$ operations.

**Worked example**

Consider a box-shaped convex viewing volume:

$$
-1\le x,y,z\le1
$$

Its left plane is:

$$
x+1\ge0
$$

Its right plane is:

$$
-x+1\ge0
$$

and there are corresponding positive and negative constraints for Y and Z.

The point $P=(0.5,-0.5,0.5)$ satisfies all six inequalities.

The point $Q=(2,0,0)$ fails:

$$
-2+1=-1<0
$$

Therefore $Q$ is outside.

The same principle works for an ordinary truncated-pyramid frustum, except its side-plane normals are angled.

**Part 1161 — Classifying a sphere against a plane**

Now imagine a sphere whose centre is slightly outside the frustum.

The sphere itself might still partially enter the viewing volume.

Testing only the centre would incorrectly cull it.

We must account for its radius.

Let a plane have inward unit normal $n$ and offset $d$.

For sphere centre $C$ and radius $r$, the centre's signed distance is:

$$
s=n\cdot C+d
$$

The sphere extends a distance $r$ in both directions along the plane normal.

Therefore the smallest signed distance of any point on the sphere is:

$$
s-r
$$

and the largest is:

$$
s+r
$$

This yields three cases:

$$
\boxed{ \begin{aligned} s<-r&:\quad\text{completely outside}\\\ s>r&:\quad\text{completely on positive side}\\\ -r\le s\le r&:\quad\text{crosses or touches plane} \end{aligned} }
$$

If the normal is not unit length, the corresponding threshold is:

$$
r\|n\|
$$

rather than $r$.

**Worked example**

Take plane:

$$
x=0
$$

with inward normal $n=(1,0,0)$, so the valid side is $x\ge0$.

A sphere with:

$$
C=(-3,0,0),\quad r=2
$$

has:

$$
s=-3
$$

Since:

$$
-3<-2
$$

it is completely outside.

But if:

$$
C=(-1,0,0),\quad r=2
$$

then:

$$
s=-1
$$

The sphere crosses the plane because:

$$
-2\le-1\le2
$$

It must not be rejected based on that plane.

**Frustum-wide sphere testing**

A sphere can be rejected from a convex frustum if any one plane proves that the sphere lies completely outside.

However, passing all six plane tests means only that no individual plane has excluded it.

It does not necessarily prove an exact sphere–frustum intersection.

The sphere can occupy a region outside a frustum corner while coming sufficiently close to each separate plane to pass all six individual tests.

This distinction makes the six-plane algorithm a conservative culling test, not a complete exact-intersection solver.

**Parts 1162–1163 — AABB-plane classification and supporting vertices**

A sphere has the same radius in every direction.

A box does not.

To classify a box against a plane, we need to determine its maximum and minimum projections onto the plane normal.

This introduces the central concept of a support point.

For a box $B$ and direction $n$, its supporting point maximizes:

$$
n\cdot X
$$

over all points $X\in B$.

**Positive and negative vertices**

For an AABB with extrema $m,M$, define the positive vertex $P^+$ component-wise:

$$
\boxed{ P_i^+= \begin{cases} M_i,&n_i\ge0\\\ m_i,&n_i<0 \end{cases} }
$$

Why?

Consider the dot product:

$$
n\cdot P=n_xP_x+n_yP_y+n_zP_z
$$

To maximize the X contribution, choose the largest X coordinate when $n_x>0$, and the smallest X coordinate when $n_x<0$.

The dimensions are independent, so each coordinate can be optimized separately.

The negative vertex $P^-$ makes the opposite choice:

$$
\boxed{ P_i^-= \begin{cases} m_i,&n_i\ge0\\\ M_i,&n_i<0 \end{cases} }
$$

These are respectively the maximum- and minimum-projection corners along the direction $n$.

**Plane classification**

Let the plane be:

$$
n\cdot X+d=0
$$

with positive half-space designated as inside.

If:

$$
n\cdot P^++d<0
$$

then even the box point most favourable to the positive half-space lies outside.

Therefore the entire box is outside.

If:

$$
n\cdot P^-+d>0
$$

then even the least favourable point lies inside, so the entire box is on the positive side.

Otherwise the box straddles or touches the plane.

**Equivalent centre-radius formulation**

For box centre $C$ and half-extents $E$, the maximum absolute projection of its local offsets onto $n$ is:

$$
\boxed{ r=|n_x|e_x+|n_y|e_y+|n_z|e_z }
$$

The centre residual is:

$$
s=n\cdot C+d
$$

Therefore:

$$
s<-r
$$

means entirely outside, and:

$$
s>r
$$

means entirely on the positive side.

This is equivalent to the supporting-vertex classification.

**How your frustum code uses this**

Your `Frustum::IntersectsAABB()` constructs the positive vertex independently for each plane.

It rejects the box when:

```
plane.SignedDistanceToPoint(positive) < 0
```

This is a correct conservative plane-rejection strategy, assuming consistent plane orientation and valid geometric inputs.

It is efficient because it requires no transformed vertices, no general polygon clipping, and no expensive shape-shape contact algorithm.

For an OBB, your code uses the analogous projected-radius formula:

$$
r= e_0|n\cdot u_0| + e_1|n\cdot u_1| + e_2|n\cdot u_2|
$$

This is the appropriate generalization from axis-aligned boxes to oriented boxes.

**Hierarchical frustum culling**

Now connect this to a scene hierarchy.

Suppose a parent BVH node contains ten thousand triangles.

If the parent's bounding box lies completely outside one frustum plane, every child is also outside.

The renderer can reject that entire subtree without visiting its contents.

This is where the algorithms from M069 and M070 become engine architecture rather than isolated geometric functions.

For a large scene, the ideal flow is:

```
Camera view-projection matrix
        ↓
Extract frustum planes
        ↓
Test root spatial bounds
        ↓
Reject completely outside subtrees
        ↓
Descend through surviving bounds
        ↓
Collect potentially visible objects
        ↓
Perform finer visibility and rendering work
```

Frustum culling is not the same as occlusion culling.

An object inside the frustum might still be hidden behind another object. Determining that requires additional information such as depth buffers, hierarchical Z, portals, or explicit visibility structures.

### Convex Polygons, Winding, Area, and Centroids

Original Parts 1164–1169

Up to this point, we have been examining familiar geometric primitives: boxes, capsules, and frusta.

Now we need a more general mathematical framework.

We want to describe geometry that might not fit into a convenient predefined primitive but still has enough structure to support efficient intersection, containment, and optimization algorithms.

That structure is convexity.

**Part 1164 — Convex polygon representation**

A set $K\subseteq\mathbb R^n$ is convex if, for every pair of points $A,B\in K$, the entire segment joining them also belongs to $K$.

Formally:

$$
\boxed{ (1-t)A+tB\in K \quad \forall A,B\in K,\ t\in[0,1] }
$$

This is one of the most consequential definitions in computational geometry.

It says that travelling in a straight line between two points inside the shape never takes us outside it.

A triangle is convex. An AABB is convex. An OBB, sphere, and capsule are convex.

A crescent-shaped region is generally not convex.

Convex

The connecting segment remains inside

Concave

The connecting segment leaves the shape

A convex polygon in two dimensions can be represented by an ordered sequence of vertices:

$$
P_0,P_1,\ldots,P_{n-1}
$$

with successive vertices joined by edges, including the closing edge from $P_{n-1}$ to $P_0$.

The order must describe the polygon boundary consistently. An arbitrary collection of points is not itself an ordered polygon.

For a simple polygon, the boundary does not self-intersect, except that adjacent edges meet at their shared endpoints.

In memory, a polygon might be represented using `std::vector<Vec2>` for a standalone value or a vertex/index buffer for a larger mesh system.

The vector representation supports simple traversal. A half-edge or related topology structure becomes useful when we also need adjacency and efficient topological editing, which we will cover in M071–M072.

**Part 1165 — Polygon winding order**

Winding describes the orientation in which polygon vertices traverse the boundary.

In the conventional Cartesian XY coordinate system, a polygon may be counterclockwise (CCW) or clockwise (CW).

The sign of its oriented area tells us the winding.

However, screen-space Y axes often point downward. Under such a coordinate convention, a sequence that appears clockwise on the screen may have counterclockwise orientation in a mathematical Y-up system.

This is why winding conventions must always specify the coordinate system.

**Why winding matters**

Winding determines which side of an oriented edge is the polygon interior. That affects convexity tests, half-space construction, face normals, clipping algorithms, polygon triangulation, and rasterizer front-face conventions.

Reversing all vertex indices reverses the polygon's orientation without changing its geometric region.

For an oriented polygon, the interior is consistently on the same side of each directed boundary edge.

For a CCW convex polygon, the interior lies to the left of every directed edge.

**Part 1166 — Polygon area using the shoelace formula**

Let a simple polygon have vertices:

$$
P_i=(x_i,y_i)
$$

with cyclic indexing:

$$
P_n=P_0
$$

Its signed area is:

$$
\boxed{ A_s= \frac12 \sum_{i=0}^{n-1} (x_iy_{i+1}-x_{i+1}y_i) }
$$

This is the shoelace formula.

**Where the formula comes from**

Consider the oriented triangle formed by the origin and two consecutive polygon vertices $P_i,P_{i+1}$.

Its signed area is:

$$
A_i=\frac12(x_iy_{i+1}-x_{i+1}y_i)
$$

This expression is the determinant of the vectors from the origin to the two vertices, divided by two.

Now sum over the complete polygon boundary.

For a simple polygon, the signed contributions combine to produce the polygon's oriented area. Regions outside the polygon cancel algebraically as the directed edges traverse the boundary.

Thus:

$$
A_s=\sum_i A_i
$$

The unsigned geometric area is:

$$
\boxed{A=|A_s|}
$$

For a CCW polygon in ordinary Y-up coordinates, $A_s>0$. For a CW polygon, $A_s<0$.

**Worked example**

Consider the rectangle:

$$
P_0=(0,0),\quad P_1=(4,0),\quad P_2=(4,3),\quad P_3=(0,3)
$$

The shoelace sum is:

$$
\begin{aligned} 2A_s={}&(0\cdot0-4\cdot0)\\\ &+(4\cdot3-4\cdot0)\\\ &+(4\cdot3-0\cdot3)\\\ &+(0\cdot0-0\cdot3) \end{aligned}
$$

Therefore:

$$
2A_s=24
$$

and:

$$
\boxed{A_s=12}
$$

This agrees with the rectangle's ordinary area:

$$
4\times3=12
$$

Reversing the vertex sequence gives signed area $-12$, but the geometric area remains $12$.

**Numerical problems**

The formula accumulates products and differences. Large translated coordinates can produce significant cancellation even when the polygon is small.

For example, a tiny polygon located around coordinates of magnitude $10^9$ may be poorly represented by naïve multiplication and summation.

One useful improvement is to translate the polygon close to the origin before evaluating its area, because signed area is translation invariant.

Compensated summation may reduce accumulation error. For topology-critical orientation, certified predicates or exact arithmetic may still be required.

**Part 1167 — Polygon centroid**

The centroid of a uniform-density polygon is its geometric centre of mass.

It is not generally the arithmetic average of the vertices.

For example, adding an extra collinear vertex to one edge does not change the polygon's region, so it should not change its geometric centroid. But it may change the simple average of the vertex coordinates.

Therefore the centroid must account for area rather than vertex count.

**Deriving the centroid formula**

For a region $R$ with area $A$, its centroid is:

$$
C_x=\frac1A\iint_R x\,dA
$$

$$
C_y=\frac1A\iint_R y\,dA
$$

For a triangle with uniform density, the centroid is the average of its three vertices.

The oriented triangle formed by the origin, $P_i$, and $P_{i+1}$ has signed area:

$$
A_i=\frac12k_i
$$

where:

$$
k_i=x_iy_{i+1}-x_{i+1}y_i
$$

Its centroid is:

$$
C_i=\frac{P_i+P_{i+1}}3
$$

Combining the oriented triangle contributions using area weighting:

$$
C=\frac{\sum_i A_iC_i}{\sum_i A_i}
$$

Substitute the expressions:

$$
C= \frac{ \sum_i \frac{k_i}{2}\frac{P_i+P_{i+1}}3 }{ A_s }
$$

Therefore:

$$
\boxed{ C_x= \frac{ \sum_i(x_i+x_{i+1})k_i }{ 6A_s } }
$$

$$
\boxed{ C_y= \frac{ \sum_i(y_i+y_{i+1})k_i }{ 6A_s } }
$$

These formulas are valid for a nondegenerate simple polygon with consistent boundary orientation and uniform area density.

They also work for concave simple polygons through signed-area cancellation.

For a self-intersecting path, the algebraic integrals can represent signed winding-weighted quantities rather than the centroid of the ordinary filled region. That distinction requires an explicit fill rule.

**Worked example**

For the previous $4\times3$ rectangle:

$$
A_s=12
$$

The centroid is:

$$
\boxed{C=(2,1.5)}
$$

The formula correctly gives the centre of the rectangle.

For an irregular polygon, the same calculation weighs larger area contributions more heavily.

**Why zero area must be handled**

The formula divides by:

$$
6A_s
$$

If the polygon is degenerate, the denominator vanishes.

A geometry library must decide whether to report an invalid centroid, return a fallback point, or compute the centroid of a lower-dimensional object such as a polyline.

These are different geometric definitions.

The reference lab uses `std::optional<Vec2>` so a polygon with zero algebraic area does not silently produce a fabricated area centroid.

**Part 1168 — Convexity testing**

A useful convexity test for a simple polygon examines consecutive edge turns.

Let:

$$
e_i=P_{i+1}-P_i
$$

The signed turn at vertex $P_{i+1}$ can be evaluated using:

$$
\boxed{ t_i=\operatorname{orient2D}(P_i,P_{i+1},P_{i+2}) }
$$

For a CCW convex polygon, all nonzero turn signs should be positive.

For a CW convex polygon, they should be negative.

**Why does this work?**

At each convex vertex, the polygon boundary turns consistently in the same orientation without producing an inward reflex angle.

A concave polygon contains at least one reflex vertex, causing a turn in the opposite direction.

For a simple polygon, consistent turns with the appropriate boundary conditions characterize convexity.

**Important caveat: self-intersection**

A polygonal path can make consistently signed turns while still winding around a self-intersecting star shape.

Therefore a local turn-sign test alone is not sufficient for arbitrary self-intersecting vertex sequences.

The polygon must also satisfy the simple-boundary assumption.

A reliable general polygon-validation pipeline should distinguish the validity of the polygon boundary from the convexity of the region it represents.

**Collinearity**

When three consecutive vertices are collinear:

$$
t_i=0
$$

Whether this is allowed depends on the polygon representation contract.

A weakly convex polygon may contain redundant collinear vertices.

A strictly convex polygon generally requires no collinear consecutive vertices and strictly convex interior angles.

The numerical implementation must also decide what to do with points that are nearly collinear.

For geometry-topology decisions, using an arbitrary absolute epsilon as the orientation sign can introduce inconsistencies. Robust predicates are preferable when the classification affects hull construction or mesh topology.

**Part 1169 — Half-space representation of convex geometry**

We previously represented a convex polygon through its vertices.

An alternative is to represent it by the intersection of oriented half-planes.

Let every directed edge define an inward-facing normal $n_i$ and offset $d_i$.

The corresponding half-plane is:

$$
H_i=\\{P:n_i\cdot P+d_i\ge0\\}
$$

Then a closed convex polygon can be represented as:

$$
\boxed{ K=\bigcap_iH_i }
$$

This is known as a half-space or H-representation.

The vertex representation is often called a V-representation.

**Deriving inward normals in 2D**

For a CCW polygon edge:

$$
e=(e_x,e_y)
$$

the interior lies to the left.

A left-facing perpendicular is:

$$
\boxed{n=(-e_y,e_x)}
$$

For edge starting at $A$, the half-space condition is:

$$
n\cdot(P-A)\ge0
$$

Expanding:

$$
n\cdot P-n\cdot A\ge0
$$

Define:

$$
d=-n\cdot A
$$

Then:

$$
n\cdot P+d\ge0
$$

This establishes the half-space representation directly from a consistently oriented polygon boundary.

**H-representation versus V-representation**

The two representations answer different operations efficiently.

With vertices, it is natural to render edges, calculate area, inspect topology, and find extreme vertices.

With half-spaces, it is natural to classify points, clip geometry, and express convex constraints.

Converting between them is itself a computational geometry problem, especially in higher dimensions.

An important connection is that a convex polygon is both the convex hull of its vertices and the intersection of its supporting half-planes.

This duality is foundational for convex optimization and collision detection.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/20 - Geometry Queries from Scene to Contact|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/22 - Convex Support Mapping and Distance|Next chapter]]
