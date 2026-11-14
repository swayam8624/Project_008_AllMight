# 22 — Convex Support Mapping and Distance

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/21 - Collision Implementations and Repository Comparison|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity|Next chapter]]

Support mapping reduces a convex shape to an extreme point in a requested direction. That operation provides a common interface for distance and collision algorithms across several convex primitives.

**By the end:** Compute support points and explain how they enable convex distance queries.

**Recall question:** Why can an extreme-point query represent many different convex shapes uniformly?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 53176–56087; source-span SHA256 cff52aa0cd684f710b80da20f913d8e2a7f7e837d28e5c26f3e3e059df489022. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Support Mapping

Original Parts 1170–1174

**Part 1170 — What a support mapping actually means**

This is one of the most important concepts in Pass 02 because it leads directly into GJK collision detection in M075.

Given a nonempty compact convex shape $K$ and a direction $D$, a support point is a point on the shape maximizing projection along $D$.

Formally:

$$
\boxed{ S_K(D)\in \underset{X\in K}{\operatorname{argmax}} \ D\cdot X }
$$

The maximum projection value is called the support function:

$$
\boxed{ h_K(D)=\max_{X\in K}D\cdot X }
$$

The distinction matters.

The support function returns a scalar.

The support mapping returns a point achieving that scalar.

For a nonzero direction, the support point generally lies on the boundary of the convex shape.

It may not be unique. A direction perpendicular to a box face, for example, has an entire face of maximizers.

A deterministic implementation chooses one valid maximizer.

**Geometric intuition**

Imagine shining parallel light rays toward an object.

The support point in direction $D$ is one of the points furthest along that direction.

Equivalently, move a plane perpendicular to $D$ toward the object until the plane first touches its extreme boundary.

The contact location is a support feature, and any point on that feature maximizing the projection is a support point.

The support direction selects an extreme point. The dashed line illustrates an associated supporting plane in 2D.

**Part 1171 — Support point of a sphere**

Let a sphere have centre $C$ and radius $r$.

Any point on its surface can be written:

$$
X=C+ru
$$

where:

$$
\|u\|=1
$$

We want to maximize:

$$
D\cdot X
$$

Substitute:

$$
D\cdot X=D\cdot C+rD\cdot u
$$

The first term is constant.

The Cauchy–Schwarz inequality gives:

$$
D\cdot u\le\|D\|\|u\|=\|D\|
$$

Equality occurs when:

$$
u=\frac{D}{\|D\|}
$$

Therefore:

$$
\boxed{ S_{\mathrm{sphere}}(D)= C+r\frac{D}{\|D\|} }
$$

for nonzero $D$.

The support function is:

$$
\boxed{ h_{\mathrm{sphere}}(D)=C\cdot D+r\|D\| }
$$

For $D=0$, every point in the sphere has the same projection value, zero. The implementation may return a fixed valid point such as the centre.

The support function is homogeneous for nonnegative scaling:

$$
h_K(\lambda D)=\lambda h_K(D),\qquad\lambda\ge0
$$

This means that normalizing the input direction is not necessary for identifying a support feature, except where a particular formula requires it to construct the point.

**Part 1172 — Support point of an AABB**

Let the AABB have minimum $m$, maximum $M$, and query direction $D$.

We maximize:

$$
D\cdot X=D_xX_x+D_yX_y+D_zX_z
$$

Because the coordinate constraints are independent, each term is maximized independently.

Thus:

$$
\boxed{ S_i(D)= \begin{cases} M_i,&D_i\ge0\\\ m_i,&D_i<0 \end{cases} }
$$

This is exactly the positive-vertex computation used in frustum AABB classification.

We have now connected two previously separate-looking operations:

AABB-plane rejection and AABB support mapping use the same optimization.

A compact implementation is:

```
// Each coordinate chooses the endpoint maximizing dot(direction, point).
Vec3 supportAABB(const AABB& box, Vec3 direction) {
    return {
        direction.x >= 0 ? box.max.x : box.min.x,
        direction.y >= 0 ? box.max.y : box.min.y,
        direction.z >= 0 ? box.max.z : box.min.z
    };
}
```

This executes in constant time with no allocation.

**Part 1173 — Support point of an OBB**

Recall the oriented box representation:

$$
X=C+\alpha_0u_0+\alpha_1u_1+\alpha_2u_2
$$

where:

$$
-e_i\le\alpha_i\le e_i
$$

Take the dot product:

$$
D\cdot X= D\cdot C+ \sum_{i=0}^2\alpha_i(D\cdot u_i)
$$

Each $\alpha_i$ can be chosen independently.

To maximize its contribution, choose:

$$
\alpha_i= \begin{cases} e_i,&D\cdot u_i\ge0\\\ -e_i,&D\cdot u_i<0 \end{cases}
$$

Therefore:

$$
\boxed{ S_{\mathrm{OBB}}(D)= C+\sum_{i=0}^2 u_i e_i\,\operatorname{sgn}_+(D\cdot u_i) }
$$

where $\operatorname{sgn}_+$ uses $+1$ for zero.

This is the oriented equivalent of the AABB supporting-corner rule.

It requires three dot products and a weighted sum of the axes.

The expression also shows why an OBB support mapping is inexpensive despite the shape being rotated.

**Part 1174 — Support point of a capsule**

A capsule is the Minkowski sum of a segment and a sphere:

$$
K=[A,B]\oplus B_r
$$

A useful property of Minkowski sums is:

$$
\boxed{ h_{A\oplus B}(D)=h_A(D)+h_B(D) }
$$

Why?

By definition:

$$
h_{A\oplus B}(D) = \max_{a\in A,b\in B}D\cdot(a+b)
$$

Expand:

$$
=\max_{a,b}(D\cdot a+D\cdot b)
$$

Because $a$ and $b$ can be selected independently:

$$
=\max_a D\cdot a+\max_b D\cdot b
$$

Therefore:

$$
=h_A(D)+h_B(D)
$$

For a segment, the support point is whichever endpoint has the greater projection along $D$:

$$
E= \begin{cases} A,&D\cdot A\ge D\cdot B\\\ B,&D\cdot B>D\cdot A \end{cases}
$$

For the sphere component, the support displacement is:

$$
r\frac{D}{\|D\|}
$$

Thus:

$$
\boxed{ S_{\mathrm{capsule}}(D) = E+r\frac{D}{\|D\|} }
$$

for nonzero $D$.

This is a remarkably simple formula for a three-dimensional rounded shape.

The deeper principle is that a complex convex shape can sometimes be built by combining support maps of simpler shapes.

**Connection to Minkowski difference and GJK**

This result becomes extremely important in the next collision phase.

For two convex sets $A$ and $B$, define their Minkowski difference as:

$$
A-B=A\oplus(-B)
$$

The origin belongs to this difference exactly when the original convex shapes intersect:

$$
\boxed{ A\cap B\ne\varnothing \iff 0\in A-B }
$$

The proof follows directly: $0=a-b$ if and only if $a=b$, meaning the sets share a point.

The support mapping of the Minkowski difference is:

$$
\boxed{ S_{A-B}(D)=S_A(D)-S_B(-D) }
$$

Deriving this:

$$
\begin{aligned} S_{A-B}(D) &=\underset{a\in A,b\in B}{\operatorname{argmax}} D\cdot(a-b) \end{aligned}
$$

The maximizing $a$ is $S_A(D)$, while the minimizing projection of $b$ along $D$ is $S_B(-D)$.

Therefore the support point of the difference can be computed without constructing the difference as a mesh.

This is the mathematical gateway to GJK.

GJK repeatedly requests such support points and updates a simplex to reason about the origin's relationship to the Minkowski difference.

In other words, the support mappings we derived for spheres, AABBs, OBBs, and capsules provide a common interface for a generalized convex-collision algorithm.

### Convex Hulls

Original Parts 1175–1178

**Part 1175 — What is a convex hull?**

Given a set of points:

$$
S=\\{P_0,P_1,\ldots,P_{n-1}\\}
$$

its convex hull is the smallest convex set containing all those points.

Formally:

$$
\boxed{ \operatorname{conv}(S)= \left\{ \sum_{i=0}^{n-1}\lambda_iP_i: \lambda_i\ge0,\; \sum_i\lambda_i=1 \right\} }
$$

The coefficients are nonnegative and sum to one, so every point in the hull is a convex combination of the input points.

This generalizes triangle barycentric coordinates.

A triangle is the convex hull of its three vertices.

A tetrahedron is the convex hull of four affinely independent points in 3D.

A polygon's convex hull discards inward concavities while retaining the outer convex envelope.

Convex hulls are useful for simplified collision proxies, bounding geometry, mesh processing, spatial reasoning, and initializing more advanced geometric algorithms.

Interior points do not contribute vertices to the outer convex hull. A point set and its hull are not the same representation.

**Part 1176 — Graham scan: full algorithmic derivation**

Graham scan constructs the convex hull of a two-dimensional point set by ordering points around a pivot and removing turns incompatible with the convex boundary.

Its typical time complexity is:

$$
\boxed{O(n\log n)}
$$

because sorting dominates the linear scan.

**Choose a pivot**

Select the point with the smallest Y coordinate.

If multiple points share the same Y coordinate, choose the smallest X coordinate.

Call this point $P_0$.

This point is guaranteed to lie on the convex hull because no input point lies below it.

The tie-breaking convention makes the pivot selection deterministic.

**Sort points by polar angle**

For every other point $P_i$, consider the displacement:

$$
V_i=P_i-P_0
$$

Sort the points by their angle around $P_0$.

Because $P_0$ is the bottommost point, the other vectors lie in the upper half-plane under the chosen coordinate convention, which allows orientation comparisons to order their angles.

A comparator can use:

$$
\operatorname{orient2D}(P_0,A,B)
$$

If the orientation is positive, $A$ precedes $B$ in increasing counterclockwise angle.

If it is negative, $B$ precedes $A$.

If it is zero, the two points lie on the same ray from the pivot, so distance provides a tie-breaker.

For a hull containing only extreme boundary vertices, keeping the farther point of collinear same-ray candidates is important.

The ordering must satisfy the strict-weak-ordering requirement of the sorting algorithm.

Using approximate epsilon comparisons carelessly can violate comparator transitivity and cause incorrect or undefined sorting behavior.

This is another reason exact or certified orientation predicates matter in computational geometry.

**Build a stack**

Start with the pivot and process the angularly sorted points.

Maintain a stack containing the current candidate hull boundary.

Whenever a new point $P$ arrives, examine the last two stack points $A,B$.

Compute:

$$
\Delta=\operatorname{orient2D}(A,B,P)
$$

If:

$$
\Delta>0
$$

the path makes a counterclockwise turn and is consistent with the chosen CCW hull orientation.

If:

$$
\Delta<0
$$

the path makes a clockwise turn and the middle point $B$ cannot remain on the desired convex boundary at that stage.

Pop $B$ and test again with the preceding stack edge.

If:

$$
\Delta=0
$$

the points are collinear. For a hull containing only extreme vertices, the middle point can also be removed under the appropriate ordering convention.

This repeats until the candidate produces an acceptable turn or the stack has too few points to form a turn.

Then push the candidate.

**Why popping is justified**

Suppose the current candidate chain contains points $A,B$, followed by new point $P$.

If $A\rightarrow B\rightarrow P$ makes a clockwise turn while constructing a CCW convex boundary, the chain bends inward relative to the boundary orientation.

Keeping $B$ would violate convexity.

The algorithm removes it and reevaluates the earlier boundary.

This is not merely a greedy guess. The angular ordering and orientation invariant ensure that discarded points do not need to reappear as extreme vertices in the eventual convex hull.

**Worked trace**

Consider points:

$$
\begin{aligned} A&=(0,0)\\\ B&=(4,0)\\\ C&=(4,4)\\\ D&=(0,4)\\\ E&=(2,1)\\\ F&=(2,2) \end{aligned}
$$

The convex hull should be the square $A,B,C,D$.

The bottommost-leftmost pivot is $A$.

The points are ordered by angle from the pivot. In one valid processing order:

$$
B,E,C,F,D
$$

The algorithm starts with:

$$
[A,B]
$$

When $E$ arrives, the turn from $A$ to $B$ to $E$ is positive, so $E$ is temporarily retained.

When $C$ arrives:

$$
\operatorname{orient2D}(B,E,C)
$$

is negative.

Therefore $E$ is removed.

The algorithm then verifies that the chain $A,B,C$ forms a positive turn.

Later, $F$ is also removed as subsequent outer boundary points establish that it lies inside the convex hull.

The final hull is:

$$
\boxed{[A,B,C,D]}
$$

The interior points were allowed to enter the working stack temporarily. The algorithm removed them once enough later geometric information became available.

**Important edge cases**

If all points coincide, the hull is a single point.

If all distinct points are collinear, the minimal hull can be represented by the two extreme endpoints.

If there are duplicate points, they should be handled deliberately before angular sorting.

If several points share the same polar angle from the pivot, the tie-breaking and collinear-point policy must be consistent.

For topology-sensitive use, nearly collinear points are especially dangerous because a wrong orientation sign can make the algorithm remove an actual extreme vertex or retain an invalid one.

**Part 1177 — Andrew's monotone chain**

Andrew's monotone chain is another $O(n\log n)$ algorithm for the two-dimensional convex hull.

It shares the stack-based turn-removal idea with Graham scan, but organizes the points differently.

Instead of sorting by angle from one pivot, it sorts points lexicographically by X and then Y.

After sorting, it constructs a lower hull and an upper hull separately.

**Lexicographic sorting**

Sort points such that:

$$
A\<B
$$

if:

$$
A_x\<B_x
$$

or, when X coordinates are equal:

$$
A_y\<B_y
$$

This gives a left-to-right ordering.

Duplicate points are removed or handled explicitly.

**Construct the lower hull**

Process points in ascending lexicographic order.

For each new point $P$, inspect the last two points in the current lower hull.

While:

$$
\operatorname{orient2D}(A,B,P)\le0
$$

remove the last point $B$, then repeat.

Once the last turn is acceptable, append $P$.

The invariant is that the retained chain has strictly positive consecutive turns when the policy removes collinear boundary points.

**Construct the upper hull**

Process the sorted points in reverse order.

Apply the same orientation-and-pop rule.

The reverse traversal constructs the upper convex boundary.

Finally, concatenate the lower and upper hulls, omitting duplicated endpoints.

**Why the algorithm works**

The lexicographic order establishes a progression across the point set.

At each step, the lower-hull stack maintains the extreme convex chain among points processed so far.

A non-left turn indicates that the current middle point is unnecessary for the lower convex envelope.

The same reasoning applies to the upper chain in reverse order.

Every point enters and leaves each stack at most once.

Therefore, after sorting:

$$
T_{\mathrm{scan}}(n)=O(n)
$$

The sorting step is:

$$
T_{\mathrm{sort}}(n)=O(n\log n)
$$

Thus total complexity is:

$$
\boxed{T(n)=O(n\log n)}
$$

and auxiliary stack storage is:

$$
\boxed{S(n)=O(n)}
$$

The CGAL convex-hull documentation discusses the Graham–Andrew family and its $O(n\log n)$ construction.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://doc.cgal.org\&sz=32)

doc.cgal.org



**C++23 implementation**

Here is the central monotone-chain implementation from the reference lab, with its computational structure preserved.

```

std::vector<Vec2> convexHull(std::vector<Vec2> points) {
    // Sort lexicographically: increasing X, then increasing Y.
    // Geometry coordinates must be finite for this comparator.
    std::sort(points.begin(), points.end(),
        [](const Vec2& a, const Vec2& b) {
            return a.x < b.x ||
                   (a.x == b.x && a.y < b.y);
        });

    // Remove exact duplicate coordinates.
    points.erase(
        std::unique(points.begin(), points.end(),
            [](const Vec2& a, const Vec2& b) {
                return a.x == b.x && a.y == b.y;
            }),
        points.end()
    );

    // A point or a pair of points is already its own hull.
    if (points.size() <= 1)
        return points;

    std::vector<Vec2> lower;
    std::vector<Vec2> upper;

    // Lower hull, moving left to right.
    for (const Vec2& p : points) {
        while (lower.size() >= 2) {
            const Vec2& a = lower[lower.size() - 2];
            const Vec2& b = lower.back();

            // A non-left turn makes the middle point unnecessary
            // for an extreme-vertex-only convex boundary.
            if (orient2D(a, b, p) > 0)
                break;

            lower.pop_back();
        }

        lower.push_back(p);
    }

    // Upper hull, moving right to left.
    for (auto it = points.rbegin();
         it != points.rend(); ++it) {

        const Vec2& p = *it;

        while (upper.size() >= 2) {
            const Vec2& a = upper[upper.size() - 2];
            const Vec2& b = upper.back();

            if (orient2D(a, b, p) > 0)
                break;

            upper.pop_back();
        }

        upper.push_back(p);
    }

    // Both chains contain the extreme left/right endpoints.
    // Remove one duplicate from each before concatenation.
    lower.pop_back();
    upper.pop_back();

    lower.insert(
        lower.end(),
        upper.begin(),
        upper.end()
    );

    return lower;
}

```

The code assumes a compatible `Vec2` type and `orient2D()` operation.

The `orient2D()` routine in the independently compiled reference lab uses ordinary floating-point arithmetic; it is suitable for teaching the algorithm but does not guarantee exact orientation signs for arbitrary finite input.

For a production hull builder requiring strong topological correctness, that predicate layer is a critical point of improvement.

**Graham scan versus monotone chain**

Both algorithms have the same asymptotic time complexity.

Graham scan organizes points angularly around a pivot. Andrew's algorithm organizes them lexicographically and constructs two chains.

Monotone chain often provides a particularly straightforward implementation for general unordered point sets because its sorting and stack invariants are easy to inspect.

However, neither should be chosen solely from its theoretical complexity.

For small point sets, constant factors and memory allocation can dominate.

For large sets, robust predicates, cache behavior, sorting performance, and the intended handling of collinear boundary points matter.

If a polygon boundary is already provided in a useful order, other algorithms may avoid a full sort.

**Part 1178 — Extending convex hulls into 3D**

A 3D convex hull is the smallest convex set containing a collection of spatial points.

For points:

$$
P_i\in\mathbb R^3
$$

it is still defined by convex combinations:

$$
\boxed{ \operatorname{conv}(S)= \left\{ \sum_i\lambda_iP_i: \lambda_i\ge0,\quad\sum_i\lambda_i=1 \right\} }
$$

But the boundary is no longer an ordered polygonal chain.

Instead, a full-dimensional polyhedral hull has faces, edges, and vertices.

A typical implementation represents the hull using an indexed polygon mesh, commonly triangulating its facets for geometric processing.

**Geometry of a convex polyhedron**

Every boundary face has an outward-pointing normal.

For a face containing $A,B,C$:

$$
N=(B-A)\times(C-A)
$$

The normal orientation should be chosen consistently outward.

For every other point $P$ inside or on the hull, an outward-oriented face satisfies:

$$
\boxed{ N\cdot(P-A)\le0 }
$$

The sign reverses when the face winding is reversed.

This condition gives a geometric test for whether a candidate face is compatible with being a supporting face of the convex hull.

In exact arithmetic, all points must lie on or behind every outward-facing hull plane.

**Initial tetrahedron**

A common incremental or Quickhull-style implementation begins by finding four affinely independent points.

These define a nondegenerate tetrahedron.

The four points must not be coplanar, which can be tested through the signed tetrahedral volume:

$$
\boxed{ \Delta= (B-A)\cdot((C-A)\times(D-A)) }
$$

For a full-dimensional tetrahedron:

$$
\Delta\ne0
$$

This is the 3D orientation predicate introduced in Pass 01.

Its sign determines the orientation of the four points.

If all input points are coplanar, the problem reduces to constructing a planar convex hull in the relevant plane.

If they are collinear, it reduces to finding extreme endpoints.

If all coincide, the hull is a point.

This dimension classification must be handled before attempting an ordinary 3D surface construction.

**Incremental hull construction**

Assume we already have a valid convex polyhedron and want to insert a new point $P$.

There are two possibilities.

If $P$ lies inside or on every supporting half-space of the current hull, adding it does not change the convex hull.

If $P$ lies outside, the hull must expand.

Some existing faces are visible from $P$, meaning $P$ lies on the exterior side of their planes.

Those visible faces will no longer belong to the boundary of the expanded hull.

The algorithm must identify the boundary separating visible faces from nonvisible faces.

This boundary is called the horizon.

**What the horizon means**

Imagine a solid object viewed from a point outside it.

Some faces are visible, while others are hidden behind the object's silhouette.

The horizon is the boundary between these regions.

During convex hull expansion, the horizon edges define where new triangular faces must be attached.

The conceptual operation is:

Existing convex polyhedron

Find a point outside

Identify visible faces

Trace horizon edges

Remove visible faces and connect horizon to new point

Expanded convex hull

Each new face joins one horizon edge to the inserted point.

Its winding must be chosen so its normal points outward.

The resulting mesh must remain a closed, consistently oriented, convex surface.

**Topological invariants**

A valid full-dimensional convex polyhedron requires coherent face adjacency and consistently oriented edges.

Each edge of a closed manifold triangulated hull normally belongs to exactly two triangular faces.

Adjacent faces must traverse their shared edge in opposite directions.

No face should have zero area under an exact nondegenerate representation, although coplanar points may require a deliberate triangulation policy.

For a convex polyhedron topologically equivalent to a sphere, Euler's formula applies:

$$
\boxed{V-E+F=2}
$$

where $V$, $E$, and $F$ are counts of vertices, edges, and faces.

For a purely triangular closed hull:

$$
3F=2E
$$

because each triangular face contributes three edge incidences, and every edge belongs to two faces.

Combining this with Euler's formula gives:

$$
\boxed{F=2V-4}
$$

and:

$$
\boxed{E=3V-6}
$$

for a full-dimensional triangulated convex polyhedron with $V\ge4$.

These are valuable structural sanity checks in a hull builder.

They are not substitutes for checking convexity, self-intersection, edge consistency, or duplicate topology.

**Quickhull and other approaches**

Quickhull maintains a growing hull and processes points outside its current boundary.

It uses visible faces, horizon construction, and assignment of remaining points to candidate outside sets.

Maintaining face-to-point conflict information can avoid repeatedly testing every point against every face.

In three dimensions, favourable input distributions may yield efficient practical performance, while worst-case costs can be quadratic for common Quickhull-style strategies.

Other approaches include incremental insertion, divide-and-conquer construction, and algorithms based on lifting or duality.

The most appropriate choice depends on the required numerical robustness, point count, degeneracy distribution, update frequency, and whether the hull is static or dynamic.

**Numerical degeneracy in 3D**

Three-dimensional hull construction is particularly sensitive to orientation errors.

Suppose a candidate point lies almost exactly on a hull face.

A tiny sign error may cause the algorithm to classify the point on the wrong side.

That can corrupt the visible-face set, disconnect the horizon, produce inconsistent face winding, or generate an invalid nonmanifold mesh.

For robust hull construction, certified 3D orientation predicates are therefore much more important than their small algebraic formulas might suggest.

The appropriate response is not necessarily to increase every epsilon.

A consistent exact-predicate or filtered-predicate approach is often preferable when the sign controls topological modifications.

**Support mapping on a convex hull**

If the convex hull is represented by vertices:

$$
V_0,\ldots,V_{h-1}
$$

its support mapping can be implemented by finding:

$$
\boxed{ S(D)=\underset{V_i}{\operatorname{argmax}}\ D\cdot V_i }
$$

A brute-force scan costs:

$$
O(h)
$$

where $h$ is the number of hull vertices.

For a fixed small hull, this may be perfectly reasonable.

For larger convex polyhedra with adjacency data, support searches can sometimes exploit directional coherence and hill-climbing strategies that move through neighbouring vertices toward greater projection.

Such approaches require care with ties, numerical error, and the possibility of incomplete or invalid adjacency information.

This support interface lets arbitrary convex hulls participate in the same GJK-style collision algorithms as spheres, boxes, and capsules.

### C++23 Reference Lab and Verification

**Complete Pass 02 source**

I've prepared a standalone C++23 reference implementation for the complete conceptual scope of this pass.

Geometry Pass 02 — C++23 reference implementation

Independent source covering bounds, transforms, spatial queries, support maps, polygons, and 2D convex hulls

It includes actual implementations of AABB construction, merging, overlap and intersection; corner-based and absolute-matrix affine bounds; OBB local transforms and point queries; capsule distance, support, and axis fallback; six-plane conservative frustum classification; three-plane intersection; support mappings; shoelace area; polygon centroid; convexity tests; Graham scan; and Andrew's monotone chain.

The file does not yet implement a production 3D convex-hull builder; the algorithmic architecture and invariants for that are covered above.

Compilation and reference assertions passed

Compiled with GCC C++23, optimization and warnings. The tests include affine-bounds equivalence, box queries, OBB coordinate transformations, capsule axis-surface correctness, frustum rejection, and convex hull results.

The code is a reference teaching implementation, not a certified exact-geometry kernel. Its orientation, three-plane conditioning, finite-input policies, and floating-point robustness would require further engineering for arbitrary production inputs.

**Connecting the algorithms to your own repositories**

The most important engineering connections extend beyond `KairoGeometry`.

For KairoGameEngine and KairoSpatial, the AABB representation becomes the basic unit of broadphase queries, BVH nodes, scene culling, and hierarchical visibility.

For KairoPhysicsEngine, OBBs and capsules provide practical collision primitives. Their support mappings provide a bridge to generalized convex collision.

For Maveb, bounding geometry becomes important when organizing reconstruction regions, mesh proxies, Gaussian spatial groups, and camera-visible scene content. The specific implementation path should be checked against the current Maveb repository before attributing any one bounding strategy to it.

For Vulkax and deformable simulation, conservative spatial bounds can help identify potentially interacting regions before more precise tests. The trade-off between tight bounds and update cost becomes especially relevant for rapidly deforming geometry.

The unifying idea is that the best bounding structure is not necessarily the tightest one; it is the structure that minimizes total query, update, memory, and synchronization cost for the actual workload.

AABB transformation has a different cost profile from OBB updates. A capsule may approximate a moving limb more closely than a box. A convex hull may capture arbitrary shape better but require more expensive support searches or construction.

Understanding these trade-offs is what connects geometric mathematics to an actual engine architecture.

### Production Reliability and Research

**What needs numerical and performance validation**

A production-grade geometry layer should be tested using geometric invariants, not merely a handful of manually selected inputs.

For AABBs, transforming all eight corners and using the absolute-matrix method should produce equivalent bounds within the chosen floating-point error policy. The transformed box should contain every transformed original corner.

For OBBs, local-to-world and world-to-local transforms should round-trip under an orthonormal frame. Validity checks should reject invalid half-extents and nonorthogonal axes.

For capsules, nearest-surface results should actually lie on the surface, including queries at the central axis, near endpoints, and in the degenerate sphere case.

For frusta, all plane normals must follow a consistent inward-facing convention. Reverse-Z and infinite-far projection variants require independent tests.

For convex polygons, reversing winding should reverse signed area but preserve unsigned area and centroid. Convex hull results should contain all input points and have consistently oriented boundary turns.

For hull algorithms, tests should include duplicates, collinear data, almost-collinear data, extreme coordinate magnitudes, and adversarial numerical configurations.

**Research bridge**

A useful research direction for this phase is:

Can workload-adaptive bounding volumes reduce total broadphase and visibility-query cost compared with a fixed AABB-only representation?

Consider a workload containing static meshes, rotating rigid objects, moving capsules, and deformable geometry.

A possible experiment would compare AABB-only bounds, OBB-oriented bounds, capsule proxies, and selected convex-hull proxies.

The independent variables would include the primitive type, aspect ratio, angular motion, deformation frequency, and spatial density.

The dependent measurements would include update cost, overlap candidate count, false-positive rate, query latency, and memory consumption.

The experiment must account for the fact that an apparently tighter representation may be more expensive to maintain.

For example, repeatedly rebuilding convex hull proxies for rapidly deforming objects could cost more than the collision work they save.

A useful hypothesis would be that dynamically selecting bounding-volume representations from measured workload characteristics reduces total cost without increasing false negatives.

A fair comparison would need identical underlying geometry and trajectories, a common exact or sufficiently validated geometric reference for candidate correctness, and measurements that include both construction and query costs.

This is directly related to your broader interest in contact-work-aware spatial partitioning: the system should adapt based on the cost of useful work, rather than optimizing one isolated geometric property.

### Reconstruction and Mastery Gate

**Reconstruction challenge**

The strongest way to verify understanding is to rebuild these mechanisms from a blank file.

For the AABB system, derive both representations, build the minimal enclosing bounds from arbitrary points, prove why independent clamping gives the nearest point, and derive the absolute-matrix transform formula without referring to the lesson.

For OBBs, derive the world-to-local transform using the transpose of an orthonormal matrix, then explain why the approach fails if the axes contain shear or lose orthogonality.

For capsules, derive the signed distance formula from segment projection, then explain how to construct a valid nearest surface point for an axis-centred query.

For frusta, begin with homogeneous clip inequalities and derive all six plane coefficient combinations. Explain the difference between conventional and reverse-Z physical near/far semantics.

For polygons, derive the shoelace area formula and centroid equation, including why the centroid is not generally the average of the vertices.

For convex hulls, implement both Graham scan and monotone chain, and justify the orientation tests, stack removals, complexity, and collinear-point policy.

Finally, derive the support mapping of a capsule from the Minkowski-sum identity, and show why:

$$
S_{A-B}(D)=S_A(D)-S_B(-D)
$$

holds.

If you can reconstruct those operations and explain their numerical failure modes, you have the principal foundations needed for advanced convex collision algorithms.

**Compact recall sheet**

M069–M070 · Core reconstruction cues

AABB: Cartesian product of intervals. Min-max and centre–half-extents representations are equivalent.

Bounds operations: coordinate-wise extrema for construction and merge; interval intersection for overlap; clamp for nearest solid point.

Affine transform:

$$
\boxed{C'=AC+T,\qquad E'=|A|E}
$$

OBB: local coordinates are dot products with orthonormal axes. Containment and closest-point queries reduce to a local AABB problem.

Capsule: segment plus ball. Signed distance equals nearest-segment distance minus radius.

Frustum: intersection of inward-facing half-spaces. Extract plane constraints from clip-space inequalities.

Convexity: every segment between points of the set remains inside the set.

Polygon area: shoelace determinant sum.

Centroid: signed-area-weighted combination of boundary triangle centroids.

Support map:

$$
\boxed{S_K(D)\in\arg\max_{x\in K}D\cdot x}
$$

Convex hull: smallest convex set containing the input. Orientation predicates decide which points remain on the boundary.

Major numerical risks: nonfinite bounds, invalid OBB frames, scale-dependent thresholds, nearly collinear hull points, singular plane intersections, and silent fallback geometry.


Theoretical and implementation teaching coverage, not a claim of independently demonstrated mastery.

| Mastery unit                                                  | Original parts                                 | Status                                   |
| ------------------------------------------------------------- | ---------------------------------------------- | ---------------------------------------- |
| M069 — AABBs, OBBs, capsules                                  | 1133–1157                                      | Taught                                   |
| M070 — Frusta, convex polygons, support mapping, convex hulls | 1158–1178                                      | Taught                                   |
| Executable reference implementation                           | C++23                                          | Compiled and basic assertions passed     |
| Production numerical robustness                               | Adversarial and certified-predicate validation | Further validation required              |
| 3D convex hull                                                | Conceptual algorithm and invariants            | Full implementation remains a deeper lab |

Next — Pass 03: M071 + M072, Parts 1179–1200.

We move into Voronoi regions, simplex geometry, tetrahedral barycentrics, signed tetrahedral volume, line-line and segment-segment closest approaches, plane-plane and three-plane intersections, mesh topology, half-edge and winged-edge structures, manifold and nonmanifold geometry, adjacency, robust orientation, adversarial testing, and final geometry-library invariants.

That will complete Phase 12's foundational geometry system and prepare us for Phase 13: intersections, SAT, GJK, EPA, and continuous collision detection.


Phase 12 · Advanced Computational Geometry, Simplex Mathematics, Mesh Topology, and Robust Geometric Algorithms

M071 + M072

Phase 12 final pass

Original Parts 1179–1200

M071: Voronoi regions, simplices, tetrahedral barycentrics, signed volumes, closest approaches, plane intersections, and fundamental mesh topology.

M072: Boundary and adjacency structures, geometric degeneracy, robust predicates, adversarial testing, and production geometry-library contracts.

Repository: `KairoGeometry/Distance.cppm`, `Segment.cppm`, `Triangle.cppm`, and the existing geometry tests.

This pass completes the foundations required before moving into explicit collision and intersection algorithms in Phase 13.

We need to understand a crucial transition:

Geometry is not merely a collection of positions. It is a mathematical system combining distance, orientation, connectivity, dimensionality, and numerical decisions.

A mesh can contain perfectly reasonable-looking triangle positions but invalid connectivity. A closest-point algorithm can return a finite vector while violating its minimization condition. A mathematically nondegenerate tetrahedron can become numerically singular. A collision algorithm can report the wrong result because a single orientation sign was incorrectly evaluated.

To understand and debug these problems, we must connect several mathematical and computational layers rather than learn isolated formulas.

### PART I — M071: Voronoi Regions and Simplex Geometry

**Part 1179 — Voronoi regions and nearest-feature reasoning**

In Pass 01, we derived the seven-region closest-point algorithm for a triangle.

We found that the nearest point may belong to its interior, one of its three edges, or one of its three vertices.

The deeper concept behind that algorithm is a partition of space into regions where a particular geometric feature determines the closest point.

For a finite set of sites $s_1,\ldots,s_n$, the ordinary Voronoi cell of site $s_i$ is:

$$
V_i=\\{x:\|x-s_i\|\le\|x-s_j\|\ \forall j\\}
$$

That means every location in the cell is at least as close to $s_i$ as to any other site.

But for a triangle or a convex polytope, the corresponding closest-feature regions are slightly different. The sites are no longer merely isolated points: we classify the closest feature as a vertex, an edge, a face, or a higher-dimensional interior.

**The nearest-point optimization problem**

Let $K\subseteq\mathbb R^n$ be a nonempty closed convex set.

Given a query point $P$, define:

$$
\boxed{ Q^*=\underset{Q\in K}{\operatorname{argmin}}\frac12\|P-Q\|^2 }
$$

The factor $1/2$ is included because it simplifies differentiation without changing the minimizer.

For any nonempty closed convex set in Euclidean space, this nearest point exists and is unique.

Why unique?

Suppose there were two distinct minimizers $Q_1,Q_2$ at the same distance from $P$.

Since $K$ is convex, their midpoint:

$$
Q_m=\frac{Q_1+Q_2}{2}
$$

also belongs to $K$.

By the strict convexity of squared Euclidean distance, the midpoint would be closer to $P$ than the two distinct minimizers, contradicting their optimality.

Thus the minimizer is unique.

This proof applies to the nearest point in the filled convex set, including its interior.

For a point strictly inside a solid convex object, the nearest point in the solid is the query point itself. The nearest point on the object's boundary is a different optimization problem and may not be unique.

**The fundamental projection optimality condition**

The nearest-point solution satisfies:

$$
\boxed{ (P-Q^*)\cdot(X-Q^*)\le0 \qquad\forall X\in K }
$$

This condition says that from the nearest point $Q^*$, every feasible direction toward the convex set makes an angle of at least $90^\circ$ with the vector pointing toward the query $P$.

We can derive it.

Choose any feasible $X\in K$, and construct:

$$
Q(t)=Q^*+t(X-Q^*)
$$

For $0\le t\le1$, this remains inside $K$ by convexity.

Define:

$$
f(t)=\frac12\|P-Q(t)\|^2
$$

Because $Q^*$ minimizes distance, moving from $t=0$ into the feasible set cannot immediately decrease $f$.

Therefore:

$$
f'(0^+)\ge0
$$

Differentiating:

$$
f'(0)=-(P-Q^*)\cdot(X-Q^*)
$$

Thus:

$$
(P-Q^*)\cdot(X-Q^*)\le0
$$

This is the projection optimality condition.

It is both necessary and sufficient for a convex set.

**Why this explains triangle closest-point algorithms**

If the nearest point lies on a triangle's face interior, then movement along both positive and negative in-plane directions is feasible.

The residual $P-Q^*$ must therefore be orthogonal to the triangle's plane.

If it lies on an edge interior, movement in both directions along the edge is feasible, so the residual must be orthogonal to the edge. But movement toward the triangle's interior must not decrease the distance.

If the nearest point lies at a vertex, only certain directions into the triangle are feasible. The residual must satisfy the corresponding directional inequalities.

These are exactly the dot-product conditions we derived for the triangle's seven Voronoi regions.

The algorithm is not a collection of unrelated if-statements. It is an optimized active-constraint solver.

**Connection to constrained optimization**

Suppose a convex shape can be represented by constraints:

$$
g_i(Q)\le0
$$

The closest-point problem becomes:

$$
\min_Q \frac12\|P-Q\|^2
$$

subject to those inequalities.

The Karush–Kuhn–Tucker framework introduces multipliers $\lambda_i\ge0$ for active constraints.

At the optimum, the stationarity condition is:

$$
Q^*-P+\sum_i\lambda_i\nabla g_i(Q^*)=0
$$

along with feasibility and complementary slackness:

$$
\lambda_i g_i(Q^*)=0
$$

For a polygon or polyhedron, the active constraints identify the feature containing the closest point.

A face has one active supporting-plane constraint. An edge of a full-dimensional 3D polyhedron commonly has two independent active face constraints. A vertex can have three or more active constraints.

This optimization viewpoint becomes extremely useful in GJK and contact generation.

**The relationship to GJK**

The Gilbert–Johnson–Keerthi algorithm works with the Minkowski difference of two convex shapes and attempts to find the point of that difference closest to the origin, or determine that the origin lies inside.

Its internal simplex updates are therefore nearest-point operations subject to convex-combination constraints.

When GJK decides whether to retain or discard a simplex vertex, it is reasoning about which simplex features can participate in the closest point to the origin.

This is directly connected to the Voronoi-region tests from our triangle algorithm.

In Phase 13 we will use this observation to derive GJK rather than treating it as a mysterious collection of cross products and direction updates.

**Part 1180 — Simplex geometry: point, segment, triangle, tetrahedron**

A simplex is the simplest possible polytope of a given dimension.

An $n$-simplex is the convex hull of $n+1$ affinely independent vertices.

0-simplex

Point

1-simplex

Segment

2-simplex

Triangle

3-simplex

Tetrahedron

A 0-simplex consists of one point. A 1-simplex consists of two distinct endpoints and the segment between them. A 2-simplex is a nondegenerate filled triangle. A 3-simplex is a nondegenerate solid tetrahedron.

**Affine independence**

Vertices $P_0,\ldots,P_n$ are affinely independent if the displacement vectors:

$$
P_1-P_0,\ldots,P_n-P_0
$$

are linearly independent.

For a segment, this requires distinct endpoints.

For a triangle, the two edge vectors must not be collinear.

For a tetrahedron, three edge vectors must span three-dimensional space.

Why this matters: the simplex must have the intended dimension. Four points that all lie in one plane do not define a 3-simplex, even though there are four vertices.

**Barycentric coordinates in general dimensions**

Every point $X$ in the affine hull of a nondegenerate simplex can be written uniquely as:

$$
\boxed{ X=\sum_{i=0}^{n}\lambda_iP_i,\qquad \sum_{i=0}^{n}\lambda_i=1 }
$$

Inside the closed simplex:

$$
\lambda_i\ge0
$$

for every vertex.

The nonnegative barycentric coefficients define a convex combination.

The affine-sum condition ensures translation invariance, while nonnegativity restricts the point to the convex hull.

**Simplex faces and dimensions**

A tetrahedron has four triangular faces, six edges, and four vertices.

Each triangular face is a 2-simplex.

Each edge is a 1-simplex.

Each vertex is a 0-simplex.

The entire tetrahedron is a 3-simplex.

This nested structure explains why a nearest-point algorithm for a tetrahedron naturally considers a smaller simplex when the query is outside one of its faces.

It also explains the simplex update process in GJK: when one barycentric coordinate cannot participate in a feasible nearest point, the algorithm can discard the corresponding vertex and continue on a lower-dimensional face.

**Why a simplex is ideal for collision algorithms**

Simplices have few vertices, and their geometry can be expressed through linear algebra.

For a triangle, two independent edge vectors span the affine plane.

For a tetrahedron, three independent edge vectors span three-dimensional space.

This permits compact barycentric calculations, feature classification, determinant tests, and nearest-point updates.

In three-dimensional GJK, a working simplex needs no more than four vertices to represent a full-dimensional candidate containing the origin.

That fact is closely related to Carathéodory's theorem, which states that a point in the convex hull of a set in $\mathbb R^3$ can be represented as a convex combination of at most four points from that set.

GJK implementations may temporarily process additional algorithmic state, but their fundamental simplex reasoning operates on this small bounded number of vertices.

**Part 1181 — Deriving tetrahedral barycentric coordinates**

Let a tetrahedron have vertices:

$$
A,B,C,D\in\mathbb R^3
$$

We want to express a point $P$ as:

$$
\boxed{ P=\lambda_AA+\lambda_BB+\lambda_CC+\lambda_DD }
$$

subject to:

$$
\lambda_A+\lambda_B+\lambda_C+\lambda_D=1
$$

We need to derive all four coefficients.

**Convert to a local basis**

Eliminate $\lambda_A$:

$$
\lambda_A=1-\lambda_B-\lambda_C-\lambda_D
$$

Substitute into the point equation:

$$
P=A+\lambda_B(B-A)+\lambda_C(C-A)+\lambda_D(D-A)
$$

Define:

$$
e_1=B-A,\quad e_2=C-A,\quad e_3=D-A
$$

and:

$$
r=P-A
$$

Then:

$$
\boxed{ r=\lambda_Be_1+\lambda_Ce_2+\lambda_De_3 }
$$

In matrix form:

$$
\begin{pmatrix} e_{1x}&e_{2x}&e_{3x}\\\ e_{1y}&e_{2y}&e_{3y}\\\ e_{1z}&e_{2z}&e_{3z} \end{pmatrix} \begin{pmatrix} \lambda_B\\\ \lambda_C\\\ \lambda_D \end{pmatrix} = \begin{pmatrix} r_x\\\ r_y\\\ r_z \end{pmatrix}
$$

Thus tetrahedral barycentric coordinates are the solution to a $3\times3$ linear system.

A nondegenerate tetrahedron has an invertible edge matrix.

**Determinant form**

Define the oriented determinant:

$$
\Delta= \det[e_1,e_2,e_3]
$$

Then, using Cramer's rule:

$$
\boxed{ \lambda_B= \frac{\det[r,e_2,e_3]}{\Delta} }
$$

$$
\boxed{ \lambda_C= \frac{\det[e_1,r,e_3]}{\Delta} }
$$

$$
\boxed{ \lambda_D= \frac{\det[e_1,e_2,r]}{\Delta} }
$$

and finally:

$$
\boxed{ \lambda_A=1-\lambda_B-\lambda_C-\lambda_D }
$$

The numerator determinants correspond to oriented sub-tetrahedral volumes.

This is the 3D generalization of the area-based barycentric coordinates derived for triangles.

**Why determinant ratios are geometrically meaningful**

The determinant:

$$
\Delta=\det[B-A,C-A,D-A]
$$

is six times the signed volume of the original tetrahedron.

Replacing one edge-matrix column with the displacement $r=P-A$ produces a determinant proportional to the corresponding sub-tetrahedral volume.

Dividing by the original determinant cancels the volume units and gives a dimensionless weight.

The volume ratios are signed because points outside the tetrahedron may lie across one or more supporting face planes.

This is why negative barycentric weights naturally represent exterior points.

**Worked example: unit tetrahedron**

Choose:

$$
A=(0,0,0)
$$

$$
B=(1,0,0)
$$

$$
C=(0,1,0)
$$

$$
D=(0,0,1)
$$

and query:

$$
P=(0.1,0.2,0.3)
$$

The edge matrix is the identity:

$$
E= \begin{pmatrix} 1&0&0\\\ 0&1&0\\\ 0&0&1 \end{pmatrix}
$$

Therefore:

$$
\Delta=1
$$

The barycentric coefficients are immediately:

$$
\lambda_B=0.1
$$

$$
\lambda_C=0.2
$$

$$
\lambda_D=0.3
$$

and:

$$
\lambda_A=1-0.1-0.2-0.3=0.4
$$

Thus:

$$
\boxed{ (\lambda_A,\lambda_B,\lambda_C,\lambda_D) =(0.4,0.1,0.2,0.3) }
$$

All four weights are positive and sum to one.

Therefore the point lies inside the tetrahedron.

**Containment using tetrahedral barycentrics**

A point lies inside or on a nondegenerate tetrahedron if:

$$
\boxed{ \lambda_A,\lambda_B,\lambda_C,\lambda_D\ge0 }
$$

with the barycentric sum equal to one.

If one weight is zero, the point lies on the opposite triangular face.

If two appropriate weights vanish, it lies on an edge.

If three vanish, it lies at the remaining vertex.

This creates a direct correspondence between the barycentric weights and simplex features.

**Example of an outside point**

Consider:

$$
P=(0.7,0.5,0.2)
$$

Its barycentric coordinates relative to the unit tetrahedron are:

$$
\lambda_B=0.7,\quad\lambda_C=0.5,\quad\lambda_D=0.2
$$

Therefore:

$$
\lambda_A=1-1.4=-0.4
$$

The point lies outside the tetrahedron.

The negative $\lambda_A$ indicates that it lies beyond the triangular face opposite vertex $A$, relative to the simplex's affine-coordinate constraints.

**The numerical problem**

If the tetrahedron is nearly flat:

$$
|\Delta|\approx0
$$

the edge matrix becomes nearly singular.

Small changes to vertex positions or rounding error may create large changes in computed barycentric weights.

A finite nonzero determinant is not enough to guarantee that the solution is numerically reliable.

The condition number of the edge matrix controls how strongly input and arithmetic perturbations can influence the solution.

In a numerical geometry library, we should distinguish:

- A tetrahedron that is mathematically degenerate.
- A valid tetrahedron that is poorly conditioned.
- A tetrahedron that is unsuitable for a chosen numerical precision.
- A tetrahedron with invalid or nonfinite coordinates.

For robust containment, determinant-sign predicates can avoid some of the problems associated with calculating approximate barycentric ratios.

For interpolation, however, actual barycentric values are required, so accurate coordinate computation remains necessary.

**Part 1182 — Signed tetrahedral volume**

The volume of the parallelepiped spanned by three edge vectors is the absolute scalar triple product.

For tetrahedron $ABCD$, define:

$$
a=B-A,\quad b=C-A,\quad c=D-A
$$

The signed determinant is:

$$
\boxed{ \Delta=a\cdot(b\times c) }
$$

The signed tetrahedral volume is:

$$
\boxed{ V_s=\frac16a\cdot(b\times c) }
$$

and unsigned volume is:

$$
\boxed{ V=|V_s| }
$$

The factor $1/6$ arises because a tetrahedron occupies one-sixth of the corresponding parallelepiped.

**Deriving the factor from base area and height**

Take the triangular base $ABC$.

Its area is:

$$
A_{\mathrm{base}}= \frac12\|a\times b\|
$$

Let its unit normal be:

$$
n=\frac{a\times b}{\|a\times b\|}
$$

The signed height of vertex $D$ relative to that base is:

$$
h_s=n\cdot c
$$

The ordinary volume of a pyramid is:

$$
V=\frac13A_{\mathrm{base}}|h_s|
$$

Substituting:

$$
V=\frac13\left(\frac12\|a\times b\|\right) \left|\frac{(a\times b)\cdot c}{\|a\times b\|}\right|
$$

The cross-product magnitudes cancel, giving:

$$
V=\frac16|(a\times b)\cdot c|
$$

which equals the absolute scalar triple product divided by six.

**Orientation and sign**

The sign of the determinant depends on vertex ordering.

Exchanging any two tetrahedron vertices reverses the orientation and the signed volume.

An even permutation preserves the sign.

This allows orientation predicates to distinguish the relative orientation of four spatial points.

For a mathematical tetrahedron, the sign is a topological orientation property, not a physical volume with negative magnitude.

The absolute volume remains nonnegative.

**Affine transformation of volume**

Suppose a tetrahedron is transformed by:

$$
P'=MP+T
$$

where $M$ is a $3\times3$ linear transformation.

Its edge matrix transforms to:

$$
E'=ME
$$

The determinant becomes:

$$
\det(E')=\det(ME)
$$

Using determinant multiplicativity:

$$
\det(E')=\det(M)\det(E)
$$

Therefore:

$$
\boxed{ V_s'=\det(M)V_s }
$$

This is an important result.

Translation does not affect signed volume.

A proper rotation has determinant $+1$, so signed volume is preserved.

A reflection has a negative determinant, so orientation reverses.

Uniform scaling by $s$ changes volume by $s^3$, including orientation reversal for negative $s$.

Nonuniform scaling changes volume by the product of its axis scale factors.

This is useful for verifying transforms, checking mesh orientation, and deriving mass properties.

**Near-zero volume and robust orientation**

The exact 3D orientation predicate evaluates the sign of:

$$
\det[B-A,C-A,D-A]
$$

Its sign may become unreliable under ordinary floating-point evaluation when coordinates nearly cancel.

This is not merely a problem of calculating a slightly inaccurate volume.

A wrong sign can reverse the classification of a point relative to a tetrahedral face or corrupt a convex-hull construction.

Jonathan Shewchuk's adaptive-precision geometric predicates address this class of problem using filtered arithmetic and exact expansion evaluation when necessary.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://people.eecs.berkeley.edu\&sz=32)

people.eecs.berkeley.edu



We will return to the exact-predicate architecture in M072.

### PART II — Closest Approaches Between Lines and Segments

**Part 1183 — Deriving the closest approach between two infinite lines**

In Pass 01, we solved the distance from a point to a line.

Now both geometric objects have a free parameter.

Let the first line be:

$$
L_1(s)=P+sU
$$

and the second:

$$
L_2(t)=Q+tV
$$

where $P,Q$ are points and $U,V$ are nonzero direction vectors.

Both parameters range over all real numbers.

We want the pair of points minimizing their squared separation:

$$
\boxed{ \min_{s,t}\|P+sU-(Q+tV)\|^2 }
$$

**Convert the problem into a quadratic**

Define:

$$
R=P-Q
$$

Then the displacement between the line points is:

$$
W(s,t)=R+sU-tV
$$

The squared-distance objective is:

$$
f(s,t)=W(s,t)\cdot W(s,t)
$$

Expanding:

$$
\begin{aligned} f(s,t) ={}&R\cdot R+2s(U\cdot R)-2t(V\cdot R)\\\ &+s^2(U\cdot U)+t^2(V\cdot V)-2st(U\cdot V) \end{aligned}
$$

Introduce five scalar products:

$$
\begin{aligned} a&=U\cdot U\\\ b&=U\cdot V\\\ c&=U\cdot R\\\ e&=V\cdot V\\\ f_0&=V\cdot R \end{aligned}
$$

The symbol $f_0$ is distinguished from the objective function $f(s,t)$.

Taking partial derivatives gives:

$$
\frac{\partial f}{\partial s} = 2c+2as-2bt
$$

$$
\frac{\partial f}{\partial t} = -2f_0-2bs+2et
$$

At an unconstrained minimum:

$$
as-bt=-c
$$

$$
bs-et=-f_0
$$

Equivalently:

$$
\boxed{ \begin{pmatrix} a&-b\\\ -b&e \end{pmatrix} \begin{pmatrix}s\\\t\end{pmatrix} = \begin{pmatrix}-c\\\f_0\end{pmatrix} }
$$

The matrix is the Gram matrix of $U$ and $-V$.

Its determinant is:

$$
\boxed{\Delta=ae-b^2}
$$

For nonparallel directions, $\Delta>0$ in exact arithmetic.

Solving gives:

$$
\boxed{ s^*=\frac{bf_0-ce}{\Delta} }
$$

$$
\boxed{ t^*=\frac{af_0-bc}{\Delta} }
$$

The closest points are then:

$$
\boxed{ X=P+s^*U,\qquad Y=Q+t^*V }
$$

and:

$$
\boxed{d=\|X-Y\|}
$$

**Geometric interpretation**

At the optimum, the displacement between the nearest points is perpendicular to both line directions.

We can verify this from the derivative conditions:

$$
(X-Y)\cdot U=0
$$

$$
(X-Y)\cdot V=0
$$

Thus the shortest connecting segment between two nonparallel infinite lines is a common perpendicular.

This includes two possibilities.

The lines may intersect, in which case their shortest separation is zero.

Or they may be skew, meaning they are nonparallel but lie in different planes and do not intersect.

In the skew case, the shortest connecting segment has nonzero length and is perpendicular to both lines.

**Cross-product distance formula**

For nonparallel lines, $U\times V$ is perpendicular to both directions.

Its unit direction is:

$$
N=\frac{U\times V}{\|U\times V\|}
$$

The perpendicular separation between the lines is the magnitude of the displacement between their origins projected onto this normal.

Therefore:

$$
\boxed{ d= \frac{|(Q-P)\cdot(U\times V)|} {\|U\times V\|} }
$$

This formula directly computes distance, while the Gram-system solution also produces the closest-point parameters.

The formulas are mathematically equivalent for nonparallel lines, but their numerical behavior can differ.

**Worked example: skew lines**

Consider:

$$
L_1(s)=(s,0,0)
$$

and:

$$
L_2(t)=(0,t,1)
$$

The first is the X axis; the second is a line parallel to the Y axis at height $z=1$.

Set:

$$
P=(0,0,0),\quad U=(1,0,0)
$$

$$
Q=(0,0,1),\quad V=(0,1,0)
$$

Then:

$$
R=(0,0,-1)
$$

The dot products are:

$$
a=1,\quad e=1,\quad b=0,\quad c=0,\quad f_0=0
$$

Therefore:

$$
\Delta=1
$$

and:

$$
s^*=0,\quad t^*=0
$$

The closest points are:

$$
X=(0,0,0)
$$

$$
Y=(0,0,1)
$$

Hence:

$$
\boxed{d=1}
$$

The connecting displacement lies along Z, perpendicular to both line directions.

**Parallel and nearly parallel lines**

When:

$$
U\times V=0
$$

the directions are parallel.

Correspondingly:

$$
\Delta=ae-b^2=0
$$

The $2\times2$ system becomes singular.

The closest-point parameters may not be unique.

For two parallel infinite lines, choose any point on the first and project it onto the second.

For example, take $P$ on the first line. Its closest point on the second is:

$$
t=\frac{(P-Q)\cdot V}{V\cdot V}
$$

The distance between those points equals the distance between the parallel lines.

When the directions are nearly parallel, the determinant is small and the problem becomes ill-conditioned.

Notice that:

$$
ae-b^2 = \|U\|^2\|V\|^2\sin^2\theta
$$

where $\theta$ is the angle between directions.

A useful dimensionless parallelism measure is:

$$
\boxed{ \frac{\Delta}{ae}=\sin^2\theta }
$$

This is more meaningful than comparing $\Delta$, whose units are length to the fourth power when direction vectors carry length units, against a fixed numerical epsilon.

There is still a trade-off: treating two nearly parallel but nonparallel lines as parallel can change their nearest-point solution substantially when the closest parameters lie far from their stored origins.

A robust solver must decide whether it is computing exact geometric relationships, approximate local relationships, or results restricted to a finite numerical domain.

**Part 1184 — Closest points between two finite segments**

Now restrict the lines to finite domains:

$$
S_1(s)=P+sU,\quad 0\le s\le1
$$

$$
S_2(t)=Q+tV,\quad 0\le t\le1
$$

The objective remains:

$$
\boxed{ \min_{0\le s,t\le1} \|R+sU-tV\|^2 }
$$

This is a convex quadratic optimization problem over a square in parameter space.

The unconstrained line-line solution may fall outside that square.

This is why simply computing the infinite-line closest points and independently clamping both parameters is not generally correct.

**Why independent clamping fails**

The parameters are coupled through the term:

$$
-2st(U\cdot V)
$$

Changing $s$ can change the optimal $t$, and changing $t$ can change the optimal $s$.

Suppose the line-line solution has:

$$
s^*<0
$$

If we clamp $s$ to zero, the optimal $t$ must be recomputed for the now-fixed endpoint $P$.

It is not generally the old unconstrained $t^*$.

This is the key constraint-handling issue.

**Deriving the constrained solution**

There are two broad cases.

First, the unconstrained stationary point lies inside the parameter square:

$$
0\le s^*\le1,\qquad0\le t^*\le1
$$

Then it is the global optimum.

Second, the unconstrained stationary point lies outside the square, or the quadratic is singular because of parallel directions.

In this case, at least one global minimizer exists on the boundary of the parameter square.

The boundary consists of four edges:

$$
s=0,\quad s=1,\quad t=0,\quad t=1
$$

Each boundary edge reduces the problem to a point-to-segment closest-point calculation.

For example, fixing $s=0$ leaves:

$$
\min_{0\le t\le1}\|P-(Q+tV)\|^2
$$

which is exactly the projection-and-clamp problem derived in Pass 01.

Therefore a clear reference implementation can evaluate:

- The feasible unconstrained stationary point, if one exists.
- The nearest point on segment 2 to endpoint $P$.
- The nearest point on segment 2 to endpoint $P+U$.
- The nearest point on segment 1 to endpoint $Q$.
- The nearest point on segment 1 to endpoint $Q+V$.

Choosing the candidate with the smallest squared separation gives the global solution in exact arithmetic.

This approach also handles parallel segments, including overlapping ones, because an optimum exists among the examined boundary cases even if a continuum of equally optimal interior pairs exists.

**Why this works mathematically**

The objective is convex because it is the squared norm of an affine function.

Its Hessian is:

$$
H= 2\begin{pmatrix} a&-b\\\ -b&e \end{pmatrix}
$$

For any vector $z=(z_1,z_2)$:

$$
z^THz = 2\|z_1U-z_2V\|^2\ge0
$$

Thus the Hessian is positive semidefinite.

When the unconstrained minimizer is feasible, it is globally optimal.

Otherwise, a constrained global minimizer lies on the boundary.

Each boundary edge is itself a convex one-dimensional optimization problem, solved exactly by projection and clamping.

This gives a proof of the algorithm rather than a heuristic justification.

**Worked example**

Take the first segment:

$$
A=(0,0,0),\quad B=(4,0,0)
$$

and second:

$$
C=(2,3,1),\quad D=(2,-1,1)
$$

Their parametric equations are:

$$
S_1(s)=(4s,0,0)
$$

$$
S_2(t)=(2,3-4t,1)
$$

The first segment lies on the X axis.

The second runs along Y at $x=2,z=1$.

The nearest points must have matching X and Y coordinates if feasible.

Set:

$$
4s=2
$$

giving:

$$
s=\frac12
$$

and:

$$
3-4t=0
$$

giving:

$$
t=\frac34
$$

Both parameters are within their domains.

The nearest points are:

$$
X=(2,0,0)
$$

$$
Y=(2,0,1)
$$

Therefore:

$$
\boxed{d=1}
$$

**Parallel and overlapping segments**

Consider:

$$
S_1=[(0,0,0),(3,0,0)]
$$

$$
S_2=[(1,0,0),(5,0,0)]
$$

The segments overlap along the X interval $[1,3]$.

Their minimum distance is zero.

But the closest-point pair is not unique: any shared point in the overlapping interval is valid.

This has important implications for collision contact generation.

A closest-point routine can correctly return one pair while still being insufficient to construct an entire contact manifold.

For stable resting contact, the solver may need feature intervals, multiple contact points, or persistent feature IDs.

Those concerns appear in M076.

**Degenerate segments**

If the first segment has zero length, it is a point.

Then the problem reduces to point-to-segment distance.

If both have zero length, the distance is ordinary point-to-point distance.

A correct routine should handle these situations explicitly.

The mathematical degeneracy condition is:

$$
\|U\|^2=0
$$

A numerical policy may deliberately treat sufficiently short segments as points, but that threshold must reflect the units and expected workload.

**Part 1185 — Segment-segment distance and its use in collision**

Once we have closest points $X,Y$, the squared distance is:

$$
\boxed{d^2=\|X-Y\|^2}
$$

The ordinary distance is:

$$
\boxed{d=\sqrt{d^2}}
$$

The squared distance is often preferable for overlap tests.

**Capsule-capsule collision**

Two capsules are defined by segments $S_1,S_2$ and radii $r_1,r_2$.

Their filled volumes overlap or touch exactly when:

$$
\boxed{ d(S_1,S_2)\le r_1+r_2 }
$$

Equivalently:

$$
\boxed{ d^2(S_1,S_2)\le(r_1+r_2)^2 }
$$

This turns a 3D shape-shape overlap problem into a segment-segment distance calculation.

For example, suppose the segment-segment distance is $1.2$, with radii $0.5$ and $0.8$.

The combined radius is:

$$
0.5+0.8=1.3
$$

Since:

$$
1.2<1.3
$$

the capsules penetrate.

A typical scalar penetration estimate for this configuration is:

$$
\boxed{ \delta=1.3-1.2=0.1 }
$$

This is useful for contact construction when the closest-axis configuration provides a suitable normal.

**The zero-distance normal problem**

Suppose two capsule central segments intersect.

Then:

$$
X=Y
$$

and:

$$
d=0
$$

A candidate contact normal based on:

$$
n=\frac{X-Y}{\|X-Y\|}
$$

is undefined.

The overlap test remains valid, but contact-normal construction requires an alternative.

Possible policies include using persistent contact information, relative motion, a suitable feature normal, or another well-defined geometric separation direction.

The choice affects solver stability.

This is an example of why distance calculation, Boolean overlap, penetration depth, and contact-manifold generation are different operations.

**Actual KairoGeometry implementation**

Your [KairoGeometry/Segment.cppm](https://github.com/swayam8624/KairoGeometry/blob/master/Segment.cppm?utm_source=chatgpt.com) contains `ClosestPoints()` at approximately lines 395–489 on the inspected `master` branch.

It computes the direction vectors and relative offset, then the dot products:

$$
a=d_1\cdot d_1,\quad e=d_2\cdot d_2,\quad f=d_2\cdot r
$$

For two nondegenerate segments, it calculates:

$$
b=d_1\cdot d_2
$$

$$
\Delta=ae-b^2
$$

It finds a candidate $s$, clamps it, calculates $t$, and then adjusts $s$ when the second parameter falls outside its allowed domain.

This is the efficient coupled-parameter strategy commonly used for segment-segment closest-point queries.

It explicitly handles cases where either or both segments are considered degenerate.

The important numerical concern is:

```
const T denominator = a * e - b * b;

if (std::abs(denominator) > epsilon) {
    // Nonparallel branch.
}
```

The denominator is the Gram determinant, with scale proportional to the fourth power of segment length.

The default epsilon, however, is derived from machine epsilon alone.

Consequently, the decision depends on input scale.

A better parallelism criterion can compare:

$$
\Delta
$$

against a relative threshold proportional to:

$$
ae
$$

with a clear policy for very short segments.

Even that criterion is approximate; for nearly parallel segments, one should test the resulting nearest points against a reliable reference.

**Existing tests and their limitations**

I also inspected `KairoGeometry/tests/GeometryTests.cpp`.

The repository contains a segment-segment case where one segment lies on the X axis and the other lies along Y, with the expected closest points:

$$
(1,0,0),\qquad(1,1,0)
$$

and squared distance:

$$
1
$$

This validates one ordinary configuration.

The repository also contains a certification test that performs 25,000 randomized iterations covering selected sphere/box/capsule overlap symmetries and AABB closest-point containment.

That is useful existing validation infrastructure, but it does not by itself establish segment-segment numerical reliability across nearly parallel, overlapping, very small, very large, or degenerate configurations.

The reference implementation accompanying this pass includes additional segment cases and demonstrates a boundary-candidate formulation for comparison.

### PART III — Intersections of Planes

**Part 1186 — Intersection line of two planes**

Two nonparallel planes intersect along an infinite line.

Let their equations be:

$$
n_1\cdot X+d_1=0
$$

$$
n_2\cdot X+d_2=0
$$

We need to find both a point on the intersection and its direction.

**Deriving the line direction**

A vector $U$ parallel to the intersection line must lie within both planes.

Therefore:

$$
n_1\cdot U=0
$$

$$
n_2\cdot U=0
$$

The cross product is perpendicular to both normals.

Thus:

$$
\boxed{ U=n_1\times n_2 }
$$

If $U=0$, the normals are parallel, so the planes are parallel or coincident rather than intersecting in one unique line.

**Finding one point on the intersection**

We want a point $P$ satisfying:

$$
n_1\cdot P=-d_1
$$

$$
n_2\cdot P=-d_2
$$

For a nonparallel pair, one convenient solution is:

$$
\boxed{ P= \frac{ (d_2n_1-d_1n_2)\times(n_1\times n_2) }{ \|n_1\times n_2\|^2 } }
$$

This particular point is perpendicular to the intersection direction, making it the point on the line closest to the origin.

To verify, let:

$$
U=n_1\times n_2
$$

Use the vector triple-product identity:

$$
a\cdot(b\times c)=(a\times b)\cdot c
$$

and the standard cross-product identities to show that:

$$
n_1\cdot P=-d_1
$$

and:

$$
n_2\cdot P=-d_2
$$

The denominator normalizes the cross-product construction.

The final intersection line is:

$$
\boxed{ L(t)=P+tU }
$$

**Worked example**

Take planes:

$$
x=2
$$

and:

$$
y=3
$$

Their coefficient representations are:

$$
n_1=(1,0,0),\quad d_1=-2
$$

$$
n_2=(0,1,0),\quad d_2=-3
$$

The direction is:

$$
U=n_1\times n_2=(0,0,1)
$$

The point formula yields:

$$
P=(2,3,0)
$$

Therefore:

$$
\boxed{ L(t)=(2,3,t) }
$$

This is exactly the set of points satisfying both plane equations.

**Parallel versus coincident planes**

If:

$$
n_1\times n_2=0
$$

the normals are linearly dependent.

But this does not tell us whether the two planes are distinct parallel planes or the same plane.

For nonzero plane normals, one method is to normalize their coefficient vectors consistently and compare the complete plane equations, accounting for the possibility of opposite normal orientation.

For example:

$$
x=2
$$

and:

$$
2x=4
$$

describe the same plane.

But:

$$
x=2
$$

and:

$$
x=3
$$

describe distinct parallel planes.

A robust geometry API should report these as distinct outcomes:

```
enum class PlanePairKind {
    IntersectingLine,
    ParallelDistinct,
    Coincident,
    InvalidInput
};
```

Returning only an optional line loses the distinction between distinct parallel planes and coincident planes.

That may be acceptable for an API whose only purpose is to obtain a unique intersection line, but a general geometry library benefits from richer query results.

**Part 1187 — Intersection point of three planes**

Three planes can intersect in one point, a common line, a common plane, or no common point, depending on their relationships.

For a unique point, their normal vectors must be linearly independent.

Let:

$$
n_1\cdot X+d_1=0
$$

$$
n_2\cdot X+d_2=0
$$

$$
n_3\cdot X+d_3=0
$$

The system is:

$$
NX=-d
$$

where the rows of $N$ are the three plane normals.

**Determinant criterion**

Define:

$$
\boxed{ \Delta=n_1\cdot(n_2\times n_3) }
$$

If:

$$
\Delta\ne0
$$

the normal matrix is invertible and there is a unique intersection point.

The solution is:

$$
\boxed{ X= \frac{ -d_1(n_2\times n_3) -d_2(n_3\times n_1) -d_3(n_1\times n_2) }{ \Delta } }
$$

**Deriving the formula**

The numerator is a linear combination of three vectors, each perpendicular to two of the plane normals.

Consider the first plane equation.

Take the dot product of the numerator with $n_1$.

The terms containing $n_3\times n_1$ and $n_1\times n_2$ vanish because a vector is orthogonal to its cross product with another vector.

Only the first term remains:

$$
n_1\cdot[-d_1(n_2\times n_3)] =-d_1\Delta
$$

After dividing by $\Delta$:

$$
n_1\cdot X=-d_1
$$

The other two plane equations follow cyclically.

Thus the formula satisfies all three constraints.

**Worked example**

Take:

$$
x=2,\quad y=3,\quad z=4
$$

The normals are the standard basis vectors:

$$
n_1=(1,0,0)
$$

$$
n_2=(0,1,0)
$$

$$
n_3=(0,0,1)
$$

with offsets:

$$
d_1=-2,\quad d_2=-3,\quad d_3=-4
$$

The determinant is:

$$
\Delta=1
$$

and the intersection is:

$$
\boxed{X=(2,3,4)}
$$

**Connection to frustum corners**

In Pass 02, we used this exact formula to construct the eight corners of a finite frustum.

Each corner is the intersection of three planes, such as Left, Bottom, and Near.

The operation is therefore not merely an isolated geometry exercise.

It directly supports camera-frustum debugging, clipping, convex-polyhedron construction, and spatial visualization.

**Numerical conditioning**

When the three normal vectors are nearly linearly dependent, the determinant approaches zero.

The solution may become extremely sensitive to small perturbations in plane coefficients.

A numerical implementation should distinguish detecting an exact rank deficiency from deciding that the system is too ill-conditioned for the desired precision.

A scale-aware determinant check can use a normalized measure such as:

$$
\frac{|\Delta|} {\|n_1\|\|n_2\|\|n_3\|}
$$

This quantity is dimensionless, but it does not provide a complete condition-number guarantee.

For particularly demanding applications, a rank-revealing factorization or carefully designed robust predicate and construction strategy may be more appropriate.

**Part 1188 — The triangle plane equation**

A nondegenerate triangle defines a plane.

Let the vertices be:

$$
A,B,C
$$

Define:

$$
U=B-A,\quad V=C-A
$$

Their cross product is:

$$
N=U\times V
$$

The supporting plane is:

$$
\boxed{ N\cdot(X-A)=0 }
$$

Expanding gives:

$$
\boxed{ N\cdot X-N\cdot A=0 }
$$

Thus:

$$
d=-N\cdot A
$$

and:

$$
\boxed{ N\cdot X+d=0 }
$$

**Why this is not enough for triangle containment**

The supporting plane extends infinitely.

The triangle occupies only a bounded region of that plane.

A point can satisfy the plane equation while lying far outside the triangle.

Therefore a complete triangle containment test requires both coplanarity and a bounded-region condition, usually barycentric or edge-half-space classification.

This is the same distinction we encountered in Pass 01, now expressed as a general principle:

A supporting geometric object and the bounded primitive built from it are not the same set.

**Geometric normal and degeneracy**

The triangle's unit geometric normal is:

$$
n=\frac{N}{\|N\|}
$$

provided:

$$
\|N\|>0
$$

If the triangle vertices are collinear, no unique plane is determined.

This is a genuine mathematical failure, not merely a division-by-zero programming issue.

Returning an arbitrary fallback normal can be useful for visualization, but should not be mistaken for recovering the original triangle's geometry.

Your `Triangle.cppm` uses a cross-product normal and provides a safe-normalization fallback. Its barycentric implementation solves the same Gram system we derived in Pass 01.

In M072, we will revisit how those numerical assumptions should be represented explicitly in an engine library.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/21 - Collision Implementations and Repository Comparison|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity|Next chapter]]
