# 25 — SAT for Triangle and Convex Polygon Tests

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/24 - Ray–Triangle Intersection|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/26 - Contact Features and Manifolds|Next chapter]]

Separating axis tests turn overlap into a search for a projection axis with disjoint intervals. Triangle–box and convex polygon cases reveal how candidate axes arise from geometry.

**By the end:** Apply SAT to triangle–AABB and convex polygon intersection.

**Recall question:** Why can triangle vertices all lie outside a box even when the triangle intersects it?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 61971–64643; source-span SHA256 9bf0db86e9647d07edd2f6254dad19b3576ab6b7edfbee1088565f1e1ca8fe22. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### PART VII — Triangle-AABB SAT and Convex Polygon SAT

Original Parts 1238–1239

**Part 1238 — Triangle-AABB intersection using SAT**

This is an extremely important intersection test for mesh BVHs, voxelization, spatial partitioning, physics broadphases, and reconstruction.

Suppose we have a triangle and an axis-aligned box.

We want to determine whether their closed geometric regions intersect.

A triangle is a convex polygon embedded in 3D.

An AABB is a convex polyhedron.

Therefore SAT applies.

**Why checking triangle vertices is insufficient**

One tempting approach is to test whether any triangle vertex lies inside the box.

But imagine a large triangle passing directly through a small box.

All three triangle vertices can lie outside the box while the triangle intersects it.

The reverse is also possible: all eight box corners can lie outside the triangle's supporting plane while the box crosses the triangle.

Thus vertex containment alone cannot solve triangle-box overlap.

We need a complete separating-axis test.

**The 13 candidate axes**

For a nondegenerate triangle and a three-dimensional AABB, the standard finite candidate set contains:

$$
\boxed{ 3+1+9=13 }
$$

| Candidate family         | Number | Origin                          |
| ------------------------ | ------ | ------------------------------- |
| AABB coordinate axes     | 3      | Box face normals                |
| Triangle face normal     | 1      | Triangle's supporting plane     |
| Triangle-edge × box-axis | 9      | Edge-edge separation directions |

Let the triangle vertices be:

$$
V_0,V_1,V_2
$$

and its edge vectors:

$$
E_0=V_1-V_0
$$

$$
E_1=V_2-V_1
$$

$$
E_2=V_0-V_2
$$

The three box axes are:

$$
X=(1,0,0),\quad Y=(0,1,0),\quad Z=(0,0,1)
$$

The triangle normal is:

$$
\boxed{ N=E_0\times E_1 }
$$

The nine remaining axes are:

$$
E_i\times X,\quad E_i\times Y,\quad E_i\times Z
$$

for:

$$
i\in\\{0,1,2\\}
$$

**Transform to box-centred coordinates**

Let the AABB have centre $C$ and half-extents:

$$
H=(h_x,h_y,h_z)
$$

Translate the triangle vertices:

$$
\boxed{ P_i=V_i-C }
$$

The box now becomes:

$$
[-H,H]
$$

This simplifies projections because the box's projected interval is symmetric about zero.

**Projecting the triangle**

For any candidate axis $L$:

$$
p_i=L\cdot P_i
$$

The triangle's projection interval is:

$$
\boxed{ p_{\min}=\min(p_0,p_1,p_2) }
$$

$$
\boxed{ p_{\max}=\max(p_0,p_1,p_2) }
$$

**Projecting the box**

Since the box is centred at the origin, its projected interval is:

$$
[-r,r]
$$

where:

$$
\boxed{ r=h_x|L_x|+h_y|L_y|+h_z|L_z| }
$$

This is the box support-radius formula from earlier.

The axis separates the triangle and box if:

$$
\boxed{ p_{\min}>r }
$$

or:

$$
\boxed{ p_{\max}<-r }
$$

If either condition holds, return false immediately.

If all thirteen candidate axes overlap, the nondegenerate triangle and box intersect in exact arithmetic.

**Worked example**

Let the box be:

$$
B=[(-1,-1,-1),(1,1,1)]
$$

and triangle:

$$
V_0=(0,0,0)
$$

$$
V_1=(3,0,0)
$$

$$
V_2=(0,3,0)
$$

The triangle lies in the XY plane.

Its first vertex is inside the box, so intersection is immediately evident.

But consider instead a large triangle whose vertices all lie outside the box while its face passes across the box's interior.

The SAT algorithm still succeeds because its projection intervals overlap on every required axis.

This demonstrates why SAT is more general than vertex containment.

**Degenerate triangles**

If the triangle has zero area:

$$
E_0\times E_1=0
$$

its face normal is undefined.

The triangle geometrically degenerates into a segment or point.

A robust implementation should handle that lower-dimensional case deliberately.

For example, calculate whether any surviving triangle edge segment intersects the AABB.

This avoids depending on a normal computed from a zero cross product.

**Numerical robustness**

The standard 13-axis algorithm is exact in real arithmetic for valid nondegenerate inputs.

In floating-point arithmetic, near-parallel axes, very thin triangles, and large translated coordinates can create rounding sensitivity.

If the test is used as conservative broadphase rejection, a small tolerance may be acceptable, provided it does not generate false negatives.

If used for an exact topology-changing decision, a more rigorous numerical approach may be required.

**KairoGeometry implementation**

Your [Intersection.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Intersection.cppm?utm_source=chatgpt.com) contains a `TriangleAABBOverlap()` helper that constructs thirteen candidate axes: the three coordinate axes, the triangle normal, and nine edge-cross axes.

It then applies a projection-overlap test to each axis.

This matches the standard mathematical structure.

The next validation questions should concern degenerate-triangle semantics, tolerance scaling, and agreement with independently implemented reference tests.

For Maveb-style local geometry updates and spatial queries, this type of test can be used to identify triangle regions potentially interacting with voxel blocks or other axis-aligned spatial cells.

**Part 1239 — Convex polygon SAT in 2D**

Now consider two convex polygons in two-dimensional space.

Unlike the 3D OBB case, we do not need edge-cross-product axes.

For two convex polygons, testing the normals of all polygon edges is sufficient.

**Why edge normals suffice in 2D**

The separating line between two disjoint convex polygons can be positioned as a supporting line of at least one relevant polygon feature.

For polygonal geometry, the corresponding separating direction can be chosen perpendicular to a polygon edge.

Thus the finite set of candidate axes is obtained from every edge of both polygons.

Suppose polygon A has $m$ edges and polygon B has $n$ edges.

We need at most:

$$
\boxed{m+n}
$$

candidate axes, before removing duplicates or parallel directions.

**Constructing an edge normal**

For a directed edge:

$$
E=(e_x,e_y)
$$

a perpendicular vector is:

$$
\boxed{ N=(-e_y,e_x) }
$$

The opposite perpendicular is also valid.

For Boolean SAT, the sign of the candidate axis does not change whether projected intervals are separated.

Therefore either perpendicular direction works.

**Project both polygons**

For polygon A:

$$
a_{\min}=\min_i(N\cdot A_i)
$$

$$
a_{\max}=\max_i(N\cdot A_i)
$$

For polygon B:

$$
b_{\min}=\min_j(N\cdot B_j)
$$

$$
b_{\max}=\max_j(N\cdot B_j)
$$

Reject intersection if:

$$
a_{\max}\<b_{\min}
$$

or:

$$
b_{\max}\<a_{\min}
$$

If no edge normal separates the polygons, they overlap or touch.

**Worked example**

Consider square A:

$$
A=[(0,0),(2,0),(2,2),(0,2)]
$$

and square B:

$$
B=[(3,0),(5,0),(5,2),(3,2)]
$$

Take the X-axis direction:

$$
N=(1,0)
$$

The projection intervals are:

$$
I_A=[0,2]
$$

$$
I_B=[3,5]
$$

Because:

$$
2<3
$$

the axis separates the squares.

There is no need to test any remaining axes.

**Overlap example**

Move square B left by $1.5$:

$$
B=[(1.5,0),(3.5,0),(3.5,2),(1.5,2)]
$$

Now:

$$
I_A=[0,2]
$$

$$
I_B=[1.5,3.5]
$$

These intervals overlap.

The Y intervals also overlap:

$$
[0,2]
$$

Because the shapes are axis-aligned rectangles, the X and Y axes suffice.

For arbitrarily rotated convex polygons, the normals of all edges from both polygons must be considered.

**Algorithmic complexity**

A straightforward implementation scans every vertex of both polygons for every candidate axis.

There are:

$$
m+n
$$

candidate axes.

Each projection test costs:

$$
O(m+n)
$$

Therefore the straightforward total complexity is:

$$
\boxed{ O((m+n)^2) }
$$

When both polygons have roughly $n$ vertices:

$$
O(n^2)
$$

But the convex ordering of vertices can be exploited.

For suitable convex polygon representations, rotating-calipers or coherent-support techniques can reduce redundant scanning and potentially provide linear-time intersection or separation algorithms.

For small collider polygons, the simple SAT implementation may still be preferable because it is easy to verify and has modest constant costs.

**Convexity precondition**

A critical API requirement is that the input polygons are convex.

Applying this algorithm directly to arbitrary concave polygons is not generally a complete intersection test.

One solution is to decompose the concave polygon into convex pieces and test those pieces.

Another is to use a more general polygon intersection algorithm.

The chosen approach depends on whether the geometry is static, dynamic, simple, self-intersecting, or represented with holes.

### PART VIII — Complete C++23 Reference Lab

**Executable implementation**

The reference lab for Pass 04 is now compiled and tested.

Geometry Pass 04 — complete C++23 reference

Independent source · 450+ lines · Full intersection and SAT reference implementations

The file includes the intersection algorithms from M073 and the primitive-overlap and SAT operations from M074.

It contains documented implementations of ray-plane, stable ray-sphere, Möller–Trumbore ray-triangle, slab-based ray-AABB, transformed ray-OBB, finite-cylinder-plus-hemisphere ray-capsule, segment-plane, and segment-triangle queries.

It also implements sphere-sphere and sphere-box contacts, capsule overlap, AABB containment-aware penetration, OBB SAT with fifteen candidate axes, triangle-AABB SAT, and 2D convex-polygon SAT.

**Test results**

C++23 compilation passed

36 deterministic reference assertions passed

UndefinedBehaviorSanitizer run passed

Compiled with GCC using `-std=c++23`, warning flags, and separately tested with UndefinedBehaviorSanitizer. These results validate the included cases, not every possible floating-point input or topology configuration.

**Adversarial cases included**

The important tests are not only conventional surface intersections.

The reference includes a ray originating inside an AABB, a ray parallel to a slab boundary, a ray originating inside a capsule and exiting through its end, a tangent sphere-ray intersection, a contained sphere requiring box-face penetration depth, a contained AABB whose minimum translation exceeds its overlap width, and rotated OBBs separated only by an edge-cross axis.

These specifically target algorithmic assumptions that ordinary examples may not reveal.

**Limitations of the reference**

The implementation assumes finite valid geometry, nonnegative radii and extents, and orthonormal OBB axes.

It uses double-precision arithmetic and scale-sensitive heuristics in some cases. It does not implement exact predicates, certified interval error bounds, a watertight triangle-ray algorithm, a general polygon-clipping contact manifold, or a continuous collision detector.

The algorithms are intended as readable, independently testable references rather than as a drop-in certified production kernel.

### PART IX — Actual KairoGeometry and KairoPhysicsEngine Review

**Current implementation comparison**

| Area               | Verified current implementation                       | Recommended validation                                 |
| ------------------ | ----------------------------------------------------- | ------------------------------------------------------ |
| Ray-plane          | Analytic intersection with epsilon parallelism test   | Coplanar versus distinct-parallel classification       |
| Ray-sphere         | Conventional quadratic roots                          | Stable roots, tangent and extreme-scale cases          |
| Ray-triangle       | Two-sided Möller–Trumbore with ray-opposing normal    | Watertight edges, backface flag, near-degenerate tests |
| Ray-AABB           | Slab method with zero-direction handling              | Inside-ray surface-exit semantics                      |
| Ray-OBB            | Transform to local AABB and back                      | Parameter consistency and valid OBB basis              |
| Ray-capsule        | Cylinder and complete endpoint spheres                | Hemisphere filtering                                   |
| Primitive overlaps | Sphere, capsule, box, plane and triangle combinations | Tangency, containment, degenerate feature cases        |
| OBB SAT            | Full fifteen-axis structure in KairoGeometry          | Differential tests and robust tolerances               |
| Physics box SAT    | Fifteen candidate axes, overlap-based normal/depth    | Containment and edge-edge manifolds                    |
| Physics sphere-box | Closest-point contact and fallback normal             | Contained-sphere minimum translation depth             |

These are grounded in the current repository files:

[KairoGeometry intersection implementation](https://github.com/swayam8624/KairoGeometry/blob/master/Intersection.cppm?utm_source=chatgpt.com)[KairoPhysicsEngine narrowphase implementation](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Narrowphase.cppm?utm_source=chatgpt.com)

**Three priority regression tests**

The first concerns a ray beginning inside an AABB. Under first-boundary-hit semantics, the hit should be the forward exit surface, not the ray origin.

The second concerns a capsule ray travelling along the capsule axis. Endpoint-sphere roots that lie inside the cylindrical body must not be reported as external surface hits.

The third concerns penetration depth when one shape contains another. Ordinary overlap-width calculations may underestimate the required separation translation.

These are strong candidates for tests because they are mathematically specific, reproducible, and directly connected to the existing code.

**Contact manifold limitations**

Your KairoPhysicsEngine already contains a box-box contact-building stage following SAT.

It considers box vertices lying inside the opposing box and includes a fallback based on support points.

That is a reasonable starting architecture, but it does not automatically provide all the feature-clipping behaviour needed for robust face-face and edge-edge manifolds.

A dedicated feature-clipping implementation can provide more consistent contact positions in configurations where neither box has vertices strictly inside the other, or where the actual overlap patch is better represented by clipped incident geometry.

We will cover this fully in M076.

### PART X — Numerical Robustness, Performance, and Research

**Numerical robustness across the complete collision layer**

We have encountered several different numerical hazards in this pass.

The ray-plane problem becomes unstable when the ray direction is nearly parallel to the plane.

The ray-sphere problem can suffer catastrophic cancellation in quadratic-root evaluation.

Ray-triangle intersection can produce inconsistent edge classifications between adjacent triangles.

Ray-AABB calculations can encounter NaN when zero direction components are combined with reciprocal multiplication at exact slab boundaries.

Ray-capsule queries can select mathematically valid roots belonging to surfaces internal to the composite shape.

SAT can become uncertain when candidate axes are nearly degenerate or when projected intervals are almost tangent.

These problems have different causes and cannot all be solved by one globally increased epsilon.

**Tolerance policies should reflect mathematical meaning**

For plane parallelism, an angular measure is natural.

For ray-intersection endpoints, the parameter interval and direction scale matter.

For collision slop, a world-space distance tolerance may be appropriate.

For near-zero triangle area, an area-relative or scale-relative criterion can help detect poorly conditioned geometry.

For topology-changing orientation decisions, certified predicates may be necessary.

For broadphase overlap, deliberately conservative bounds can be acceptable.

The correct numerical policy depends on whether the operation must avoid false negatives, avoid false positives, preserve exact topology, or produce a physically stable approximate contact.

**CPU, SIMD, and GPU considerations**

A Boolean collision test often benefits from early exit.

SAT can return false as soon as it finds one separating axis.

On a CPU, this may save substantial work when most shape pairs are disjoint.

On a GPU executing groups of threads together, early exits may lead to divergence if different lanes exit at different times.

A fixed-axis batch implementation may sometimes perform better despite executing additional arithmetic.

Likewise, a branch-heavy ray-capsule implementation may be appropriate for occasional character-controller queries but less attractive for millions of independent GPU queries.

For high-throughput workloads, consider data layout, coherence, vectorization, memory access, numerical behaviour, and the cost of constructing query inputs.

This is where the performance and data-oriented programming concepts from earlier mastery phases become relevant.

**Research bridge — Collision correctness as a measurable system property**

A meaningful research direction is to investigate how numerical robustness policies affect both collision correctness and simulation cost across deformable and reconstructed geometry.

Consider a physical simulation with many thin triangles, capsules, and convex proxies.

A broadphase creates candidate pairs. A narrowphase classifies their geometric relationship. Contact generation provides constraints. The solver updates motion.

If an early classification is incorrect, its effects can propagate through the entire pipeline.

A missed collision may cause penetration.

An unnecessary false-positive contact may create forces where none should exist.

A poorly oriented normal may push two colliders further into one another.

A penetration depth computed for the wrong feature direction may lead to persistent overlap or unstable correction.

A useful experiment would compare baseline fixed-epsilon geometry, scale-aware approximate geometry, and certified predicates at critical decisions.

The workloads could include nearly tangent primitives, thin rotated boxes, nearly parallel capsule axes, deforming triangle meshes, and changing spatial scales.

Measure classification disagreement against an independent reference, contact-depth error, invalid normal frequency, pair throughput, solver stability, and downstream trajectory divergence.

This could form part of a broader physical-validation investigation, particularly when comparing the behaviour of independent implementations rather than merely showing that one engine executes a demo successfully.

### PART XI — Reconstruction Exercises

**Mastery questions**

To demonstrate that the ideas have become reconstructible rather than merely recognizable, work through these problems without referring to the formulas above.

1\. Starting with a ray and an implicit plane equation, derive the intersection parameter. Explain distinct-parallel, coplanar, near-parallel, and finite-ray-interval cases, including which are mathematical distinctions and which are numerical policies.

2\. Expand the ray-sphere equation into a quadratic and derive its discriminant. Explain the geometric meaning of the discriminant using the line's closest point to the sphere centre. Then derive the stable-root formula from Vieta's relation.

3\. Derive Möller–Trumbore from the equation equating ray and barycentric triangle positions. Explain the determinant's sign, backface culling, barycentric constraints, and why adjacent triangles may require more robust shared-edge handling.

4\. Derive ray-AABB slab intersection from coordinate inequalities. Explain why a zero ray-direction component must be handled deliberately and how inside-ray volume semantics differ from first-boundary-hit semantics.

5\. Derive the ray-capsule cylinder quadratic by decomposing the ray into components parallel and perpendicular to the capsule axis. Explain why complete endpoint-sphere testing is insufficient.

6\. Derive sphere-sphere, sphere-plane, sphere-box, sphere-OBB, capsule-sphere, and capsule-capsule overlap conditions from distance and projection principles. Distinguish each Boolean result from the corresponding contact-generation problem.

7\. Explain why the overlap width of contained intervals is smaller than the minimum separation translation. Extend the corrected formula from AABBs to OBB projection intervals.

8\. Prove the Separating Axis Theorem using convex closest-point optimality. Then explain why the OBB test has fifteen axes and why the triangle-AABB test has thirteen.

9\. Derive the OBB relative-rotation matrix $R_{ij}=A_i\cdot B_j$ and obtain one face-axis inequality and one edge-cross-product inequality. Explain the numerical hazards of almost-parallel cross axes.

10\. Implement two independent OBB SAT routines: a generic projected-radius version and a specialized matrix version. Differential-test them with random orientations, near-tangent configurations, containment, and adversarial edge-edge separation.

The final reconstruction challenge is to take the reference code, remove the ray and SAT implementations, and reproduce them using only their mathematical contracts. Verify the results against the original reference and introduce at least five additional adversarial configurations.

**Pass 04 recall sheet**

M073 — Ray and segment intersections

Ray-plane:

$$
\boxed{t=-\frac{N\cdot O+d}{N\cdot D}}
$$

Ray-sphere:

$$
at^2+bt+c=0
$$

with $a=D\cdot D$, $b=2(O-C)\cdot D$, and $c=\|O-C\|^2-r^2$.

Stable quadratic roots:

$$
q=-\frac12(b+\operatorname{copysign}(\sqrt\Delta,b))
$$

$$
t_0=q/a,\qquad t_1=c/q
$$

with special-case handling.

Ray-triangle: Solve the ray/barycentric linear system using cross products and a scalar triple product.

Ray-AABB: Intersect three slab parameter intervals.

Ray-OBB: Transform the ray to the box's orthonormal local frame.

Ray-capsule: Intersect the finite cylindrical body and two outward hemispheres.

Segment queries: Restrict ray parameter to $[0,1]$; handle coplanarity separately.

M074 — Primitive overlap and SAT

Sphere-sphere:

$$
\boxed{\|C_B-C_A\|\le r_A+r_B}
$$

Sphere-box: Compare sphere radius against centre-to-solid-box distance.

Capsule-capsule: Compare central segment distance against summed radii.

SAT projection interval:

$$
\boxed{ I_K(n)=[-h_K(-n),h_K(n)] }
$$

OBB projected radius:

$$
\boxed{ r=\sum_{i=0}^{2}e_i|n\cdot U_i| }
$$

OBB SAT: Six face axes and nine edge-cross axes.

Triangle-AABB SAT: Three box axes, one triangle normal, nine edge-cross axes.

2D polygon SAT: Test edge normals from both convex polygons.

Minimum translation: Use signed interval separation distances, not merely shared interval width when containment is possible.


| Mastery unit                                               | Original parts                        | Coverage                                   |
| ---------------------------------------------------------- | ------------------------------------- | ------------------------------------------ |
| M073 — Rays, segments, primitives and stable intersections | 1201–1216                             | Mathematical derivations, code, edge cases |
| M074 — Primitive overlaps, contacts and SAT                | 1217–1239                             | Mathematical derivations, code, edge cases |
| C++23 reference lab                                        | 36 assertions                         | Compiled and passed                        |
| Repository comparisons                                     | Geometry + Physics                    | Current implementations inspected          |
| Advanced production work                                   | Exact predicates, full manifolds, CCD | Scheduled in subsequent mastery units      |

Pass 04 covers all 39 original parts, 1201–1239. The reference implementation demonstrates the core algorithms, while the numerical caveats and the more elaborate contact-manifold cases remain explicit rather than being hidden behind a claim of production completeness.

Next — Pass 05: M075 + M076, Parts 1240–1270.

This is where the collision system advances beyond primitive-specific tests and SAT into Minkowski differences, GJK simplex evolution, termination conditions, EPA penetration recovery, contact feature generation, manifold clipping, persistent contacts, continuous collision detection, swept volumes, time of impact, and conservative advancement.


Phase 13 · Minkowski Geometry, GJK, EPA, Contact Manifolds, and Continuous Collision Detection


This pass connects convex geometry to an operational physics engine. We will derive how GJK determines whether two arbitrary support-mapped convex shapes intersect, how its distance formulation produces closest points, how EPA estimates penetration from an intersecting simplex, how contact clipping creates stable solver constraints, and how continuous collision detection prevents objects from passing through one another between simulation steps.

The main repository references are [KairoPhysicsEngine/ConvexCollision.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/ConvexCollision.cppm?utm_source=chatgpt.com), [Narrowphase.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/Narrowphase.cppm?utm_source=chatgpt.com), and [PhysicsWorld.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/PhysicsWorld.cppm?utm_source=chatgpt.com).

The distinction to remember from Pass 04 is that intersecting, calculating separation distance, recovering penetration, generating contact points, and calculating first time of impact are separate mathematical problems. They can share geometry infrastructure, but they do not return interchangeable results.

### From Minkowski Geometry to GJK

**Parts 1240–1241 — Minkowski difference and origin containment**

Suppose two convex shapes $A,B\subset\mathbb R^3$ occupy the same scene. We want to know whether they intersect.

A traditional approach would examine their individual faces, edges, and vertices. But this would require specialized handling for different shape representations.

A sphere has no polygonal faces. A capsule has curved surfaces. A convex hull may contain an arbitrary number of vertices. An oriented box has a finite set of orthogonal faces.

GJK exploits a remarkable geometric transformation: rather than test these two shapes directly, it constructs information about their Minkowski difference.

**Definition**

The Minkowski sum of two sets is:

$$
A\oplus B=\\{a+b\mid a\in A,\ b\in B\\}
$$

The Minkowski difference used in collision detection is:

$$
\boxed{ C=A-B=A\oplus(-B)=\\{a-b\mid a\in A,\ b\in B\\} }
$$

Here $-B$ means the set containing the negation of every point of $B$.

It is not the ordinary set-theoretic difference $A\setminus B$.

The construction considers every possible displacement from a point of B to a point of A.

**Why intersection becomes origin containment**

Assume the two shapes share a point $p$.

Then:

$$
p\in A,\qquad p\in B
$$

Choose:

$$
a=p,\qquad b=p
$$

The corresponding Minkowski difference point is:

$$
a-b=p-p=0
$$

Therefore:

$$
A\cap B\ne\varnothing \implies 0\in A-B
$$

Conversely, suppose:

$$
0\in A-B
$$

By definition there must exist points $a\in A,b\in B$ such that:

$$
a-b=0
$$

Thus:

$$
a=b
$$

That point belongs to both original shapes.

Consequently:

$$
\boxed{ A\cap B\ne\varnothing \iff 0\in A-B }
$$

This equivalence is exact for arbitrary sets, not only convex ones. Convexity becomes important for efficient support-based searching.

GJK converts the original two-shape intersection problem into the question of whether a single convex set contains the origin.

**Why the Minkowski difference remains convex**

Suppose A and B are convex.

Choose two points from their Minkowski difference:

$$
c_1=a_1-b_1,\qquad c_2=a_2-b_2
$$

For:

$$
t\in[0,1]
$$

their convex combination is:

$$
(1-t)c_1+tc_2
$$

Substitute:

$$
=(1-t)(a_1-b_1)+t(a_2-b_2)
$$

Rearrange:

$$
=[(1-t)a_1+ta_2]-[(1-t)b_1+tb_2]
$$

Since A and B are convex, the two bracketed terms belong to A and B respectively.

Therefore their difference belongs to $A-B$.

Hence:

$$
\boxed{ A,B\text{ convex}\implies A-B\text{ convex} }
$$

This property allows GJK to exploit nearest-point reasoning, supporting planes, and simplices.

**Worked one-dimensional example**

Let:

$$
A=[0,3],\qquad B=[2,5]
$$

Every difference $a-b$ lies between:

$$
0-5=-5
$$

and:

$$
3-2=1
$$

Therefore:

$$
A-B=[-5,1]
$$

The origin is inside this interval, so A and B intersect. Their original intersection is:

$$
A\cap B=[2,3]
$$

Now move B to:

$$
B=[4,6]
$$

The Minkowski difference becomes:

$$
A-B=[-6,-1]
$$

The origin is outside, so the intervals do not intersect.

This simple example captures the entire idea behind GJK.

**The configuration-space interpretation**

Suppose B is translated by a vector $t$.

The translated set is:

$$
B+t=\\{b+t:b\in B\\}
$$

A collision occurs when:

$$
A\cap(B+t)\ne\varnothing
$$

Equivalently, there exist $a\in A,b\in B$ such that:

$$
a=b+t
$$

Thus:

$$
t=a-b
$$

Therefore:

$$
\boxed{ A\cap(B+t)\ne\varnothing \iff t\in A-B }
$$

This gives the configuration-space obstacle interpretation. The Minkowski difference describes the translations of B that cause collision with A.

It also explains why penetration depth can be recovered by finding the nearest boundary of the Minkowski difference to the origin.

**Part 1242 — GJK support mapping**

Constructing the entire Minkowski difference explicitly would often be expensive or impossible.

For two complex convex polyhedra, it could have many faces and vertices. For a sphere or capsule, its boundary can be curved.

GJK avoids constructing it.

Instead, GJK asks for the extreme point of the Minkowski difference in a chosen direction.

**Support function versus support point**

For a compact convex shape $K$, define the support function:

$$
\boxed{ h_K(d)=\max_{x\in K}d\cdot x }
$$

The support point is any point realizing this maximum:

$$
\boxed{ S_K(d)\in\arg\max_{x\in K}d\cdot x }
$$

The direction $d$ does not need to be normalized to identify an extreme feature, but some primitive-specific formulas normalize it to construct the returned point.

**Deriving the Minkowski support identity**

For:

$$
C=A-B
$$

we want to maximize:

$$
d\cdot(a-b)
$$

Expand:

$$
d\cdot a-d\cdot b
$$

The maximizing choice of $a$ is:

$$
a^*=S_A(d)
$$

To maximize the negative contribution:

$$
-d\cdot b
$$

we choose the point maximizing the projection along $-d$:

$$
b^*=S_B(-d)
$$

Therefore:

$$
\boxed{ S_{A-B}(d)=S_A(d)-S_B(-d) }
$$

This is the mathematical engine of GJK.

We can sample the Minkowski difference's boundary without constructing its mesh.

**Examples from previous passes**

For a sphere:

$$
S_{\mathrm{sphere}}(d)=c+r\frac d{\|d\|}
$$

For an AABB:

$$
S_i(d)= \begin{cases} \mathrm{Max}_i,&d_i\ge0\\\ \mathrm{Min}_i,&d_i<0 \end{cases}
$$

For an OBB:

$$
S_{\mathrm{OBB}}(d)= c+\sum_i u_ie_i\operatorname{sgn}(d\cdot u_i)
$$

For a capsule:

$$
S_{\mathrm{capsule}}(d)= \operatorname{argmax}_{p\in\\{a,b\\}}d\cdot p +r\frac d{\|d\|}
$$

For a convex hull represented by vertices:

$$
S_{\mathrm{hull}}(d)= \underset{v_i}{\operatorname{argmax}}\ d\cdot v_i
$$

The last operation is $O(n)$ for a full scan of $n$ vertices. Sphere, AABB, OBB, and capsule support queries are constant-time.

**Retaining witness points**

A production GJK support point should preserve the original support points, not merely their difference.

```
struct SupportVertex {
    Vec3 minkowski;  // pointA - pointB

    Vec3 pointA;     // support point on original shape A
    Vec3 pointB;     // support point on original shape B

    uint32_t featureA;
    uint32_t featureB;
};
```

Why preserve both points?

Because:

$$
w=a-b
$$

does not uniquely identify $a$ and $b$ after the subtraction has discarded them.

But the original witnesses are needed later for closest-point reconstruction and EPA contact generation.

A support query can construct the record as follows:

```
SupportVertex supportMinkowski(
    const Shape& A,
    const Shape& B,
    Vec3 direction)
{
    // Extreme point of A in the search direction.
    const SupportResult a = support(A, direction);

    // Extreme point of B in the opposite direction.
    const SupportResult b = support(B, -direction);

    // Preserve both original witnesses and their difference.
    return {
        .minkowski = a.position - b.position,
        .pointA = a.position,
        .pointB = b.position,
        .featureA = a.featureId,
        .featureB = b.featureId
    };
}
```

This is an interface sketch; the standalone reference lab provides concrete types and implementations.

**Zero search directions**

If:

$$
d=0
$$

every shape point has the same dot-product projection, zero.

The support point is not unique.

A geometry API may return a deterministic point, but GJK usually needs to interpret a near-zero search direction as a special algorithmic state.

It may indicate that the origin lies on the current simplex, that a degeneracy occurred, or that arithmetic has stalled.

Blindly replacing a zero direction with an arbitrary axis can change the geometric meaning of the current simplex step.

**GJK's fundamental loop**

A simplex is the convex hull of a small number of affinely independent points.

In three dimensions, GJK uses simplices consisting of one point, two points, three points, or four points.

These correspond to a point, segment, triangle, and tetrahedron.

The algorithm maintains a simplex in Minkowski-difference space, not in the coordinate system of either original collider.

A high-level intersection procedure is:

1. Choose an initial search direction.
2. Request a Minkowski support point.
3. Add it to the simplex.
4. Calculate a direction toward the origin from the current nearest simplex feature.
5. Request another support point in that direction.
6. If the support geometry proves the origin cannot be enclosed, report separation.
7. Otherwise update the simplex and repeat.
8. If the simplex contains the origin, report intersection.

The original GJK family also includes a distance formulation, in which nearest-point calculations are central rather than incidental. A useful technical treatment connects the simplex closest-point operation to Johnson's distance subalgorithm.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://uu.diva-portal.org\&sz=32)

uu.diva-portal.org+1



**Part 1243 — GJK simplex update for a line**

Suppose the current simplex contains two points $A$ and $B$ in Minkowski-difference space.

Let A be the most recently added support point.

Define:

$$
AB=B-A
$$

and:

$$
AO=-A
$$

where $O=(0,0,0)$ is the origin.

We need to determine which feature of the segment is closest to the origin.

**Vertex-A region**

Consider the projection of the origin onto the infinite line through AB.

The unconstrained segment parameter is:

$$
t^*=\frac{AO\cdot AB}{AB\cdot AB}
$$

If:

$$
AO\cdot AB\le0
$$

then:

$$
t^*\le0
$$

The closest point on the segment is A.

Therefore B is unnecessary.

Reduce the simplex to:

$$
\\{A\\}
$$

and choose the new search direction:

$$
\boxed{d=AO}
$$

**Edge region**

If:

$$
AO\cdot AB>0
$$

the origin lies in the forward half-space from A along AB.

The origin may project onto the segment interior or beyond B. In the usual GJK intersection update, prior simplex invariants help determine the relevant region, but a general closest-simplex implementation should still handle both endpoints.

For the line-feature case, the desired search direction is perpendicular to AB and points toward the origin.

Take:

$$
AB\times AO
$$

This is perpendicular to both AB and AO.

Then cross again with AB:

$$
\boxed{ d=(AB\times AO)\times AB }
$$

Using the vector triple-product identity:

$$
(a\times b)\times c = b(a\cdot c)-a(b\cdot c)
$$

we obtain:

$$
d=(AB\cdot AB)AO-(AO\cdot AB)AB
$$

Factor:

$$
\boxed{ d=\|AB\|^2 \left( AO-\frac{AO\cdot AB}{\|AB\|^2}AB \right) }
$$

The expression in parentheses is exactly the component of AO perpendicular to AB.

Thus the triple-cross direction is the nearest-feature residual multiplied by $\|AB\|^2$.

**Why the triple cross can vanish**

If the origin lies on the line AB:

$$
AO\parallel AB
$$

then:

$$
AB\times AO=0
$$

and therefore:

$$
d=0
$$

If the origin lies within the segment, the current simplex already contains it.

But this does not automatically mean that the simplex is a suitable three-dimensional EPA seed.

For overlapping convex bodies, the origin can be contained by a lower-dimensional simplex even though the full Minkowski difference has a nonzero three-dimensional interior.

For touching bodies, the origin can lie on the boundary of the Minkowski difference.

A correct implementation must distinguish these possibilities or report an ambiguous/touching state until additional geometry resolves them.

This is one of the practical traps in collision-oriented GJK implementations.

**Part 1244 — GJK simplex update for a triangle**

Now suppose the simplex contains three points:

$$
A,B,C
$$

Again, A is the newest support point.

Define:

$$
AB=B-A,\qquad AC=C-A,\qquad AO=-A
$$

The oriented triangle normal is:

$$
\boxed{ ABC=AB\times AC }
$$

We must determine whether the origin lies closest to a vertex, an edge, or the triangular face.

This is the Voronoi-region problem from M068, now applied to the origin instead of an arbitrary query point.

**Outside the AC edge**

The vector:

$$
ABC\times AC
$$

lies in the triangle's plane and points across the AC edge under the chosen winding.

If:

$$
\boxed{ (ABC\times AC)\cdot AO>0 }
$$

the origin lies on the exterior side of AC.

We then determine whether the closest feature is AC or the vertex A, using the associated dot-product region tests.

If AC is retained, reduce the simplex to:

$$
\\{A,C\\}
$$

and calculate a direction perpendicular to AC toward the origin.

**Outside the AB edge**

Similarly:

$$
AB\times ABC
$$

points across the AB boundary.

When:

$$
\boxed{ (AB\times ABC)\cdot AO>0 }
$$

the origin belongs to the corresponding exterior half-plane.

The simplex can be reduced to AB or to a vertex, depending on the full closest-feature conditions.

**Face region**

If neither outside-edge condition applies, the origin projects within the triangle's interior under the intended simplex invariant.

The search direction is parallel to its normal.

If:

$$
ABC\cdot AO>0
$$

choose:

$$
\boxed{d=ABC}
$$

Otherwise:

$$
\boxed{d=-ABC}
$$

Reversing the triangle's winding can maintain a consistent ordering with the chosen search direction.

The next Minkowski support point is requested along $d$.

This searches for geometry capable of extending the simplex toward the origin from the triangle's current supporting plane.

**A numerical degeneracy**

If:

$$
AB\times AC\approx0
$$

the simplex triangle has nearly zero area.

The face direction becomes unreliable.

A robust nearest-simplex algorithm can reduce the degenerate triangle to its surviving segment or vertex features.

That is preferable to treating an invalid normal as evidence of collision.

**Connection to barycentric coordinates**

The origin belongs to the triangle's filled region if it can be represented as:

$$
0=\lambda_AA+\lambda_BB+\lambda_CC
$$

with:

$$
\lambda_i\ge0,\qquad \lambda_A+\lambda_B+\lambda_C=1
$$

For a nondegenerate triangle whose plane passes through the origin, this condition establishes origin containment in the triangle simplex.

For a triangle whose plane does not pass through the origin, its barycentric coordinates describe the projection of the origin onto the plane.

The corresponding nearest point has residual normal to the face.

This links the GJK triangle update directly to the barycentric derivations from earlier passes.

**Part 1245 — GJK simplex update for a tetrahedron**

A tetrahedron is the first simplex capable of enclosing a three-dimensional neighbourhood.

Let its vertices be:

$$
A,B,C,D
$$

Its four triangular faces are:

$$
ABC,\quad ABD,\quad ACD,\quad BCD
$$

To determine whether the tetrahedron contains the origin, we need to test whether the origin lies on the interior side of each face.

**Outward face normals**

Consider face ABC, opposite vertex D.

Calculate:

$$
N=(B-A)\times(C-A)
$$

This normal might point toward D or away from D.

To orient it outward from the tetrahedron, check:

$$
N\cdot(D-A)
$$

If this is positive, N points toward the opposite vertex and therefore inward for that face.

Reverse it:

$$
N\leftarrow-N
$$

Then N points outward.

**Test the origin relative to the face**

With outward normal N, the origin is outside face ABC when:

$$
N\cdot(O-A)>0
$$

Since $O=0$:

$$
\boxed{ N\cdot(-A)>0 }
$$

If this occurs, the origin cannot lie inside the tetrahedron.

The closest simplex feature must lie on the exterior-facing boundary of the tetrahedron, and the algorithm can reduce to a suitable face or its subfeatures.

A general nearest-simplex solver can examine all four faces and choose the closest feasible feature.

**Origin containment**

If the origin lies on the interior side of all four oriented faces, then:

$$
\boxed{ 0\in\operatorname{conv}\\{A,B,C,D\\} }
$$

Equivalently, there exist barycentric weights:

$$
\lambda_A,\lambda_B,\lambda_C,\lambda_D\ge0
$$

summing to one such that:

$$
\boxed{ 0=\lambda_AA+\lambda_BB+\lambda_CC+\lambda_DD }
$$

If all weights are strictly positive, the origin lies in the tetrahedron interior.

If one or more weights are zero, it may lie on its boundary.

This distinction will matter when initializing EPA.

**Signed volume and determinant tests**

From Pass 03, the tetrahedron's orientation is:

$$
\Delta= (B-A)\cdot[(C-A)\times(D-A)]
$$

If:

$$
\Delta=0
$$

the tetrahedron is degenerate, and its vertices do not span three-dimensional volume.

A GJK routine should not treat a degenerate four-point simplex as a valid origin-enclosing tetrahedron merely because some face tests happen to pass.

For robust implementation, tetrahedral volume and face orientation must be handled together.

**Why four vertices suffice**

In $\mathbb R^3$, Carathéodory's theorem states that a point in a convex hull can be represented as a convex combination of at most four of its points.

Thus, if the origin belongs to the Minkowski difference of two compact convex shapes, it can be represented using at most four Minkowski support points in an appropriate convex-hull representation.

This motivates the tetrahedral simplex structure.

It does not mean an arbitrary four support points will enclose the origin, or that the algorithm will be numerically reliable without appropriate simplex selection.

**Part 1246 — Choosing the GJK search direction**

The search direction is not arbitrary once the algorithm has begun.

Its job is to identify a supporting point that could improve the current simplex's relationship to the origin.

For a nearest-simplex formulation, let:

$$
q= \underset{x\in\operatorname{conv}(S)}{\arg\min} \|x\|
$$

where $S$ is the current simplex.

Then the natural direction toward the origin is:

$$
\boxed{ d=-q }
$$

Why is this a useful direction?

If $q\ne0$, the nearest point on the simplex lies in the direction from the origin toward $q$.

The vector $-q$ points from that nearest point toward the origin.

Requesting support in direction $-q$ asks whether the full Minkowski difference extends farther toward the origin than the current simplex.

If it does, the simplex can potentially improve.

If it does not, the current simplex may already provide the closest point of the entire Minkowski difference.

**Initial direction**

The first direction may be chosen from the relative centres:

$$
d_0=C_B-C_A
$$

This is often useful because it roughly points toward the other body.

But centre-based directions can be zero when the centres coincide, even if the shapes have different orientations or extents.

A fixed nonzero fallback axis is therefore common.

The initial direction primarily affects performance and numerical behaviour, not the mathematical definition of the collision problem.

**Direction normalization**

A support query only depends on direction, not positive direction magnitude.

Therefore normalizing $d$ is mathematically permissible when $d\ne0$.

However, the scale of $d$ can carry useful information for convergence checks.

Many GJK distance algorithms retain the unnormalized nearest-point residual because its squared magnitude represents the current squared separation estimate.

Normalizing it prematurely can erase useful scale information.

**Part 1247 — GJK termination criteria**

A GJK implementation needs more than one termination condition.

The mathematical algorithm makes progress toward a geometric answer. The numerical algorithm must also handle finite precision, repeated support points, and limited iteration budgets.

**Separation certificate**

Suppose the current search direction is $d$, pointing toward the origin.

Request:

$$
w=S_{A-B}(d)
$$

If:

$$
\boxed{ w\cdot d<0 }
$$

then even the most extreme point of the Minkowski difference along $d$ lies strictly on the negative side of the plane through the origin.

Since:

$$
\max_{x\in A-B}d\cdot x=w\cdot d<0
$$

the origin cannot belong to the Minkowski difference.

Therefore the original shapes are disjoint.

This is a mathematically valid separating-plane certificate in exact arithmetic.

**Tangency and equality**

If:

$$
w\cdot d=0
$$

the supporting plane passes through the origin.

This does not, by itself, establish strict separation.

The shapes may touch.

Treating equality as an ordinary miss can violate a closed-solid intersection contract.

A tolerance-based implementation may report `Touching`, `Uncertain`, or a conservative overlap depending on its intended use.

**Origin-containing simplex**

If the simplex contains the origin, we have established:

$$
0\in A-B
$$

and therefore intersection.

But the simplex may contain the origin on its boundary or interior.

A full-dimensional interior simplex is useful for starting EPA.

A lower-dimensional origin-containing simplex may be enough to establish Boolean intersection but is not automatically sufficient to initialize a three-dimensional EPA polytope.

**Duplicate support points**

Suppose a new support query returns a point already present in the simplex.

Repeatedly adding the same point cannot expand its convex hull.

This may indicate convergence, an exact finite-polytope extremum, a poor search direction, or numerical stagnation.

A duplicate-point test should be part of the algorithm, but the correct result is not always “no collision.”

The routine should distinguish convergence from unresolved degeneracy.

**Maximum iterations**

A production implementation needs a maximum iteration count to guarantee bounded work.

But reaching the iteration limit is not proof of separation.

Returning `false` merely because the algorithm exhausted its budget converts a computational failure into a geometric assertion.

A richer result is:

```
enum class GJKStatus {
    Separated,
    TouchingOrOverlapping,
    Intersecting,
    Degenerate,
    IterationLimit
};
```

A Boolean query can then apply an explicit policy to uncertain results.

**Part 1248 — Numerical degeneracy and simplex cycling**

GJK is compact in principle, but numerical robustness is one of its most difficult implementation concerns.

**Duplicate and nearly duplicate supports**

For smooth shapes, nearby support directions may produce extremely close support points.

For polyhedra, a range of nearby directions can select the same vertex.

A naïve algorithm may repeatedly request essentially the same support and fail to improve its simplex.

Comparing points by an absolute fixed squared-distance epsilon can also behave inconsistently when shape scales vary.

**Degenerate simplex features**

A line simplex can have coincident endpoints.

A triangle simplex can have collinear vertices.

A tetrahedron can have coplanar vertices.

Each case invalidates part of the corresponding geometric derivation.

The algorithm should reduce to a valid lower-dimensional feature where possible.

**False origin containment**

Near the origin, rounding can change the sign of simplex face tests.

A nearly coplanar tetrahedron may appear to contain the origin even though the exact orientation tests disagree.

Conversely, a genuinely enclosing simplex may be rejected because one determinant is evaluated with an incorrect sign.

This is one place where robust orientation predicates can improve reliability.

**Scale-dependent stopping thresholds**

A fixed tolerance such as:

$$
\|d\|^2<10^{-14}
$$

has squared-distance units if $d$ is a displacement vector.

It is therefore not scale invariant.

For a simulation involving both centimetre-sized objects and kilometre-sized worlds, one fixed threshold may not be appropriate.

A more deliberate policy can combine absolute world-space tolerances, relative geometry scales, and dimensionless improvement criteria.

**Symmetry tests**

For any two shapes:

$$
\operatorname{Intersects}(A,B) = \operatorname{Intersects}(B,A)
$$

The same Boolean result should be obtained when the argument order is reversed.

For separated shapes, the distance should satisfy:

$$
d(A,B)=d(B,A)
$$

The witness points should exchange roles.

Numerical differences are possible, but large disagreement indicates an algorithmic problem.

**What your implementation currently does**

I inspected [ConvexCollision.cppm](https://github.com/swayam8624/KairoPhysicsEngine/blob/main/ConvexCollision.cppm?utm_source=chatgpt.com).

Your implementation already contains a meaningful GJK pipeline with a `ConvexSupportVertex` preserving Minkowski and original witness points, a four-element `ConvexSimplex`, line/triangle/tetrahedron update routines, and explicit iteration limits.

It also perturbs the initial search direction slightly and uses fixed thresholds for very short directions, duplicate supports, and insufficient support progress.

These are practical attempts to handle degeneracy, but they require validation over shape scale and near-tangent configurations.

One particularly important implementation detail is that the current routine only proceeds to EPA when GJK reports intersection through a four-point simplex.

An origin-containing segment or triangle can therefore lead to no returned penetration result.

That may be acceptable when the chosen API deliberately excludes tangency or degenerate contact, but it can also exclude some true overlapping configurations in which the simplex reaches the origin before becoming a tetrahedron.

A correct treatment requires distinguishing the collision classification from the ability to construct an EPA initialization polytope.

**Part 1249 — GJK's distance formulation**

The original GJK method is fundamentally a convex-distance algorithm, although many game-engine implementations emphasize its Boolean intersection variant.

The distance between shapes A and B is:

$$
\boxed{ d(A,B)= \min_{a\in A,b\in B}\|a-b\| }
$$

But:

$$
a-b\in A-B
$$

Therefore:

$$
\boxed{ d(A,B)= \min_{x\in A-B}\|x\| }
$$

This gives us an elegant result:

The minimum distance between two convex shapes equals the distance from the origin to their Minkowski difference.

**Closest point on the simplex**

Let the current simplex contain Minkowski vertices:

$$
w_0,\ldots,w_{k-1}
$$

The closest point on its convex hull to the origin solves:

$$
\boxed{ \min_{\lambda} \left\| \sum_i\lambda_iw_i \right\|^2 }
$$

subject to:

$$
\lambda_i\ge0,\qquad \sum_i\lambda_i=1
$$

This is a small convex quadratic program.

For a simplex with up to four vertices, we can solve it by examining active faces and their affine projections or by specialized Voronoi-region formulas.

The active barycentric weights identify the nearest simplex feature.

**Recovering original witness points**

Each Minkowski vertex preserves:

$$
w_i=a_i-b_i
$$

If the closest point is:

$$
q=\sum_i\lambda_iw_i
$$

then:

$$
q= \sum_i\lambda_i(a_i-b_i)
$$

Rearrange:

$$
q= \left(\sum_i\lambda_ia_i\right) - \left(\sum_i\lambda_ib_i\right)
$$

Define:

$$
\boxed{ P_A=\sum_i\lambda_ia_i }
$$

$$
\boxed{ P_B=\sum_i\lambda_ib_i }
$$

Then:

$$
\boxed{ q=P_A-P_B }
$$

Therefore, at convergence:

$$
\boxed{ d(A,B)=\|P_A-P_B\| }
$$

The original witness points are reconstructed from the same barycentric weights that locate the nearest Minkowski point.

This is why preserving the original support points is so important.

**A convergence criterion derived from optimality**

Let $q$ be the current simplex point nearest the origin.

Choose search direction:

$$
d=-q
$$

and obtain the support point:

$$
w=S_{A-B}(-q)
$$

At the true nearest point of the complete convex Minkowski difference, the projection optimality condition requires:

$$
q\cdot(x-q)\ge0
$$

for every $x\in A-B$.

Thus:

$$
q\cdot x\ge\|q\|^2
$$

The most challenging point is the one minimizing $q\cdot x$, which is the support point in direction $-q$.

Therefore a global optimality condition is:

$$
\boxed{ q\cdot w\ge\|q\|^2 }
$$

Equivalently, define the progress gap:

$$
\boxed{ g=\|q\|^2-q\cdot w }
$$

At convergence in exact arithmetic:

$$
g\le0
$$

In floating-point arithmetic, the algorithm uses a carefully selected tolerance.

This criterion is more informative than terminating only because the distance change between successive iterations is small.

**Why distance GJK matters for CCD**

Suppose two shapes are currently separated by distance:

$$
d>0
$$

If they move, the distance gives us a geometric margin before collision.

We can use motion bounds to advance the simulation through intervals that are guaranteed not to contain contact.

This is the foundation of conservative advancement, which we will derive in M076.

GJK distance therefore connects static geometry to continuous collision detection.

### EPA and Penetration Recovery

**Part 1250 — Why EPA is needed after GJK**

Suppose GJK determines that two convex bodies intersect.

We know:

$$
0\in C=A-B
$$

But an intersection Boolean does not tell us how to separate them.

For a physics solver, we generally want a translation direction $n$ and a magnitude $\delta$ such that moving B away from A by approximately $\delta n$ resolves the overlap.

That problem is closely related to the boundary of the Minkowski difference.

**Penetration as a configuration-space problem**

Recall that translating B by $t$ causes collision exactly when:

$$
t\in A-B
$$

Suppose the bodies currently overlap, so:

$$
0\in C
$$

To remove the overlap, the translation must move outside C.

The smallest translation magnitude needed to reach its boundary is:

$$
\boxed{ \delta=\min_{q\in\partial C}\|q\| }
$$

where $\partial C$ denotes the boundary of the Minkowski difference.

For the usual convention of allowing tangency as the final nonpenetrating configuration, the corresponding direction is:

$$
\boxed{ n=\frac{q^*}{\|q^*\|} }
$$

with $q^*$ a nearest boundary point.

Then the translation of B is:

$$
\boxed{ t=\delta n }
$$

under the convention $C=A-B$.

The difference $C$ shifts to $C-t$ when B is translated by $t$, so the selected boundary point reaches the origin.

**GJK distance versus EPA penetration**

For separated objects, the origin lies outside C.

GJK distance attempts to find the closest point of the filled convex set C to the origin.

For overlapping objects, the origin lies inside C, so:

$$
\min_{q\in C}\|q\|=0
$$

That tells us nothing about the distance to the boundary.

EPA — the Expanding Polytope Algorithm — addresses this by constructing and expanding a polyhedral approximation of the boundary of C around the origin.

Its goal is to find a boundary feature nearest the origin and recover its supporting direction and penetration depth.

The essential distinction is:

$$
\boxed{ \begin{aligned} \text{GJK distance:}&\quad \min_{q\in C}\|q\|\\\ \text{EPA penetration:}&\quad \min_{q\in\partial C}\|q\| \end{aligned} }
$$

EPA assumes a suitable origin-containing initial polytope and uses additional support queries to refine it.

**Why an origin-containing tetrahedron is required**

In 3D, EPA commonly begins with a nondegenerate tetrahedron containing the origin in its interior.

A segment or triangle has no three-dimensional interior, so it cannot directly form the closed volumetric boundary EPA expects.

If GJK detects overlap through an origin-containing segment, EPA needs an initialization procedure that expands the simplex into a full-dimensional enclosing polytope.

That procedure is not trivial when the Minkowski difference is very thin, tangent, nearly degenerate, or itself lower-dimensional.

One possible approach is to sample additional support directions, construct a candidate tetrahedron, and verify strict origin containment.

The reference lab includes a deliberately simple version of this idea, but its limitations will be discussed in the validation section.

**Part 1251 — EPA polytope expansion**

EPA maintains a convex polytope $P$ built from support points of the Minkowski difference.

Initially:

$$
P_0\subseteq C
$$

and the origin must lie inside $P_0$.

The algorithm repeatedly expands P toward the actual boundary of C.

**The initial faces**

A tetrahedron has four triangular faces.

Each EPA face stores three vertex indices, an outward-facing unit normal, and a distance from its supporting plane to the origin.

A useful representation is:

```
struct EPAFace {
    std::array<std::uint32_t, 3> indices;

    // Outward-facing unit normal of the Minkowski polytope.
    Vec3 normal;

    // Positive distance from the origin to the face plane.
    double distance;
};
```

The polytope also stores the support vertices, including their witnesses on A and B.

**Why outward orientation matters**

For face vertices $A,B,C$, calculate:

$$
N=(B-A)\times(C-A)
$$

Normalize:

$$
n=\frac N{\|N\|}
$$

For a polytope containing the origin, an outward normal should satisfy:

$$
n\cdot A>0
$$

when the face is strictly separated from the origin.

If:

$$
n\cdot A<0
$$

reverse the face winding and normal.

Its positive face distance is:

$$
\boxed{ d_f=n\cdot A }
$$

For a correctly oriented convex polytope enclosing the origin, every supporting face distance is nonnegative.

This is a critical invariant.

An incorrectly oriented face can make EPA expand inward, select the wrong face, or calculate a meaningless penetration direction.

**The expansion step**

Choose the polytope face nearest the origin.

Let its normal be:

$$
n_f
$$

and its plane distance:

$$
d_f
$$

Request a Minkowski support point in the outward normal direction:

$$
\boxed{ w=S_C(n_f) }
$$

Its projection is:

$$
\boxed{ d_w=n_f\cdot w }
$$

Because the current polytope is contained in C, the support point should lie at least as far along $n_f$ as the existing supporting face, ignoring numerical error.

If:

$$
d_w>d_f
$$

the current face does not yet reach the actual shape boundary in that direction.

We can expand the polytope by adding $w$.

Conceptual 2D projection of polytope expansion. A new support point extends the approximation toward the true Minkowski-difference boundary.

**Visible faces**

The new support point may lie outside several existing polytope faces.

For each face with outward normal $n_i$ and vertex $p_i$, evaluate:

$$
\boxed{ n_i\cdot(w-p_i) }
$$

When this is positive, $w$ lies beyond that face's outward half-space.

The face is visible from the new support point and should be removed from the expanding polytope.

In finite precision, the comparison requires an explicit tolerance or orientation policy.

**The horizon**

Removing visible faces leaves a boundary separating removed faces from retained faces.

This boundary is the horizon.

Each horizon edge belongs to one removed face and one retained face in a valid closed manifold polytope.

The algorithm connects the new support point to every horizon edge, creating new triangular faces.

The updated polytope now encloses a larger region.

**Detecting the horizon using directed edges**

Suppose a removed visible face contributes oriented edges:

$$
(a,b),\quad(b,c),\quad(c,a)
$$

Add these edges to a temporary boundary collection.

If a later removed face contributes the reverse edge:

$$
(b,a)
$$

the two edges cancel.

Why?

The edge is shared internally by two removed faces, so it is not part of the final horizon.

Edges that survive cancellation lie between removed and retained faces.

A simple educational implementation can use a vector of directed edges and remove reversed pairs.

A production implementation often uses adjacency, half-edge connectivity, or more efficient indexed structures.

**Polytope invariants**

After expansion, a valid EPA polytope should remain closed, convex, consistently oriented, and origin-enclosing.

Every internal polytope edge should have exactly two incident faces.

Every face should have a valid outward normal.

The horizon should form a coherent boundary cycle, or a valid set of cycles under the algorithm's geometric assumptions.

A newly added point should not coincide with an existing point in a way that creates zero-area faces.

These invariants are not merely implementation hygiene. EPA's convergence argument depends on the polytope remaining a valid approximation of the convex Minkowski difference.

**Part 1252 — Why EPA chooses the closest face**

For every face $f$, define its positive distance from the origin:

$$
d_f
$$

EPA selects:

$$
\boxed{ f^*=\arg\min_f d_f }
$$

Why prioritize the nearest face?

Because we want the nearest boundary direction from the origin.

The polytope is currently an inner approximation of the Minkowski difference. Its nearest face identifies a direction in which its current boundary is closest to the origin.

Expanding that face helps refine the currently most restrictive part of the approximation.

**Face distance is not vertex distance**

A face's closest plane point is:

$$
\boxed{ q_f=n_fd_f }
$$

This is the perpendicular projection of the origin onto the face's supporting plane.

It may lie in the face interior, edge, or vertex region depending on geometry.

For a well-formed convex polytope containing the origin, the face with globally minimum positive supporting-plane distance has a closest-plane point compatible with the polytope's nearest boundary distance. But intermediate numerical degeneracy or invalid face geometry can break assumptions used by a naïve implementation.

This is why an EPA result should verify the selected face and barycentric reconstruction rather than assume that a small plane distance is sufficient.

**Face selection complexity**

If there are $F$ current faces, scanning all faces costs:

$$
O(F)
$$

per iteration.

A min-heap keyed by face distance can improve face selection in some implementations, but polytope updates invalidate or remove faces, so heap maintenance and stale entries must be managed.

For small EPA polytopes, a linear scan is often simple and practical.

For high-throughput or complex shapes, the complete cost includes support queries, visibility classification, horizon construction, face allocation, and termination checks.

Optimizing only the minimum-face lookup may not improve total performance.

**Part 1253 — Recovering penetration normal, depth, and contact witnesses**

Suppose EPA converges on a face with outward normal:

$$
n
$$

and positive distance:

$$
d
$$

The corresponding approximate minimum translation vector for B relative to A is:

$$
\boxed{ \mathrm{MTV}=dn }
$$

under the convention:

$$
C=A-B
$$

The penetration depth is:

$$
\boxed{\delta=d}
$$

The normal is:

$$
\boxed{n_{\mathrm{EPA}}=n}
$$

For a valid full-dimensional polytope and a correctly oriented face, this vector points toward a locally minimal translation direction of the original B shape.

**Why normal orientation must not be arbitrary**

Remember:

$$
C=A-B
$$

If B is translated by $t$, then:

$$
C'=A-(B+t)=C-t
$$

If the closest Minkowski boundary point is:

$$
q=dn
$$

translating B by:

$$
t=dn
$$

moves the difference shape by $-dn$, bringing that boundary point to the origin.

This derivation fixes the translation convention.

The normal should not be flipped merely to point toward whichever object centre happens to be nearer.

For asymmetric convex bodies, the minimum translation direction need not align with their centre displacement.

If a solver uses a different convention, the conversion must be mathematically explicit.

**Recovering original contact points using barycentrics**

Let the selected EPA face have Minkowski vertices:

$$
w_0,w_1,w_2
$$

Each stores:

$$
w_i=a_i-b_i
$$

The closest point on its plane is:

$$
q=dn
$$

Find barycentric weights:

$$
\lambda_0,\lambda_1,\lambda_2
$$

such that:

$$
\boxed{ q=\lambda_0w_0+\lambda_1w_1+\lambda_2w_2 }
$$

with:

$$
\lambda_0+\lambda_1+\lambda_2=1
$$

and ideally nonnegative weights for a point on the triangular face.

Now reconstruct the witnesses:

$$
\boxed{ P_A=\lambda_0a_0+\lambda_1a_1+\lambda_2a_2 }
$$

$$
\boxed{ P_B=\lambda_0b_0+\lambda_1b_1+\lambda_2b_2 }
$$

Their difference is:

$$
P_A-P_B = \sum_i\lambda_i(a_i-b_i)
$$

Therefore:

$$
\boxed{ P_A-P_B=q=dn }
$$

This is the bridge from a mathematical point in Minkowski space back to physical positions on the original objects.

**Worked sphere example**

Take two spheres:

$$
C_A=(0,0,0),\quad r_A=1
$$

$$
C_B=(1.5,0,0),\quad r_B=1
$$

Their penetration depth is:

$$
\delta=1+1-1.5=0.5
$$

The normal from A to B is:

$$
n=(1,0,0)
$$

The corresponding surface witnesses are:

$$
P_A=(1,0,0)
$$

and:

$$
P_B=(0.5,0,0)
$$

Therefore:

$$
P_A-P_B=(0.5,0,0)
$$

which equals:

$$
\delta n
$$

The same geometric relationship is what EPA attempts to reconstruct for arbitrary convex shapes.

For spheres, the analytic formula is simpler, so EPA would usually be unnecessary.

**Contact midpoint**

A narrowphase may choose a representative contact position:

$$
\boxed{ P_c=\frac{P_A+P_B}{2} }
$$

In our sphere example:

$$
P_c=(0.75,0,0)
$$

This is useful for calculating lever arms and contact impulses.

But the midpoint alone loses the distinction between the two body-surface witnesses.

A robust contact representation may store both local anchors and reconstruct the world-space separation during each solver iteration.

**Part 1254 — EPA convergence, tolerances, and failure modes**

EPA usually terminates when the newly requested support point is sufficiently close to the current selected face along its normal.

Let:

$$
w=S_C(n)
$$

and:

$$
d_w=n\cdot w
$$

The current face distance is:

$$
d_f
$$

Define the support gap:

$$
\boxed{ g=d_w-d_f }
$$

If:

$$
g\le\epsilon
$$

the current face is sufficiently close to the actual supporting boundary in that direction under the chosen approximation policy.

**What the gap means**

The current polytope is contained in C.

Its face lies at $d_f$ from the origin.

The support point of the actual shape lies at $d_w$ along the same normal.

Thus $g$ measures how much farther the actual Minkowski shape extends in this direction beyond the current polytope face.

A small gap means that particular supporting plane has been approximated closely.

A rigorous global error guarantee requires additional assumptions about the validity of the polytope, the selected nearest face, the geometry and the support oracle.

**Absolute versus relative convergence**

The gap has distance units when the direction is normalized.

An absolute termination test is:

$$
g\le\epsilon_{\mathrm{abs}}
$$

A combined policy might use:

$$
\boxed{ g\le\epsilon_{\mathrm{abs}} + \epsilon_{\mathrm{rel}}L }
$$

where $L$ is a characteristic geometric length.

This is preferable to assuming that one fixed absolute epsilon works equally well for microscopic geometry and very large worlds.

**Duplicate support points**

If a newly requested support point is identical or effectively identical to an existing polytope vertex, the algorithm cannot expand normally.

That may indicate convergence.

But if the support gap remains substantial, it may indicate numerical stagnation or an inconsistent support oracle.

A robust implementation should not automatically report success in every duplicate-point case.

**Nonmanifold horizons**

When visible-face classification is numerically inconsistent, the horizon may become disconnected or contain invalid edge incidences.

The algorithm may then generate overlapping faces, nonmanifold connectivity, or incorrectly oriented triangles.

This is a topological failure, not merely an inaccurate penetration depth.

It connects EPA directly to the half-edge and manifold-validation concepts in Pass 03.

**Failure states**

A production EPA implementation should distinguish at least:

| Outcome          | Meaning                                                                      |
| ---------------- | ---------------------------------------------------------------------------- |
| Converged        | A penetration approximation satisfying the configured tolerance was obtained |
| Invalid seed     | No suitable full-dimensional origin-containing initial polytope              |
| Degenerate face  | A face normal or barycentric construction is undefined                       |
| Stalled          | Duplicate support or no meaningful expansion without convergence             |
| Invalid topology | Horizon or face connectivity violates required invariants                    |
| Iteration limit  | Maximum permitted work reached before convergence                            |

These are algorithmic results, not all geometric misses.

**Actual implementation and test evidence**

Your `KairoPhysicsEngine::CollideConvex()` already implements EPA face expansion, closest-face selection, support-gap termination, witness interpolation, and a fixed 64-iteration EPA limit.

The current code uses an absolute gap threshold and returns `std::nullopt` for several failure conditions. That keeps the public interface simple, but conflates separation, degenerate initialization, stagnation, and iteration-limit failure.

In the independent teaching lab created for this pass, I implemented a separate GJK distance stage and an EPA initialization search over sampled support directions. The lab then expands the selected polytope and reconstructs witnesses.

The deterministic overlapping-box test produces:

$$
\boxed{ \delta=0.5,\qquad n=(1,0,0) }
$$

with the expected witness separation.

However, in additional randomized penetrating AABB tests, this educational EPA implementation produced usable results for 50 of 59 sampled cases. It therefore has not demonstrated production-level reliability. The seed construction and expansion topology require further work, particularly for less favourable geometric configurations.

This is why an iteration cap and an optional result are insufficient by themselves to establish robustness.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/24 - Ray–Triangle Intersection|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/26 - Contact Features and Manifolds|Next chapter]]
