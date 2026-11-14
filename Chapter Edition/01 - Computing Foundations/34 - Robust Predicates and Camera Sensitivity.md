# 34 — Robust Predicates and Camera Sensitivity

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|Next chapter]]

Many geometry decisions need a reliable sign rather than a highly accurate coordinate, so robust predicates can determine topology while constructions calculate positions. The chapter reconnects those decisions to camera sensitivity and uncertainty in image measurements.

**By the end:** Distinguish predicates from constructions and analyze how geometric and camera parameters affect a result near a decision boundary.

**Recall question:** Why can a tiny coordinate error change a discrete geometric decision even when the coordinate itself looks accurate?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 84092–86864; source-span SHA256 7c5c6bbc8b2668606338540d3dd424fa10757eb9904d3c79e619641ec12f0e04. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## A sign can matter more than a coordinate: robust predicates

Many geometry algorithms do not need a highly accurate coordinate at every step. They need the sign of a quantity. Is a point left or right of an oriented edge? Does a fourth point lie inside a circle through three other points? Does a ray hit on one side of a plane? These are **predicates**: discrete answers derived from continuous input.

For planar points $A=(a_x,a_y)$, $B=(b_x,b_y)$, and $C=(c_x,c_y)$, the orientation determinant is

$$
\Delta = (b_x-a_x)(c_y-a_y)-(b_y-a_y)(c_x-a_x).
$$

If $\Delta>0$, $C$ is to the left of directed edge $AB$ under the usual Cartesian orientation; if $\Delta<0$, it is to the right; and if $\Delta=0$, the points are collinear. The magnitude is twice the signed triangle area, but a triangulator often cares only about the sign.

Suppose $A=(0,0)$, $B=(1,1)$, and $C=(2,2+\epsilon)$. Then $\Delta=\epsilon$. For ordinary nonzero $\epsilon$, the answer is straightforward. But if coordinates have already been rounded and $\epsilon$ is smaller than the accumulated arithmetic error, the computed sign can disagree with the exact sign of the represented inputs. Returning “almost zero” is not a safe fourth answer when the calling algorithm must choose a branch.

The consequence is larger than a slightly inaccurate area. Consider an incremental triangulation. A circumcircle test decides whether a triangle should be retained or flipped. If one test says a point lies inside and a later logically related test says it lies outside due to inconsistent rounding, edge flips can create a non-Delaunay mesh, a self-intersection, or a loop that never converges. The coordinates may look almost right while the connectivity is wrong. This is why robust geometry treats predicates as a separate engineering layer from approximate coordinate construction. (E01)

### Filtering before exact work

An adaptive predicate starts with fast floating-point arithmetic and a conservative error bound. For an expression $d$ and computed value $\widehat d$, suppose the analysis establishes

$$
|\widehat d-d|\le E.
$$

If $\widehat d>E$, then $d>0$ is certified. If $\widehat d<-E$, then $d<0$ is certified. If $|\widehat d|\le E$, the fast path cannot establish the sign, so a more accurate computation is needed. The important property is not that the first answer is “usually right”; it is that the fast path only returns when it has a proof that the sign cannot change within the error interval.

For determinants involving input floating-point coordinates, an exact expansion can represent a value as an unevaluated sum of nonoverlapping floating-point components. Error-free transformations such as `TwoSum` or `TwoProduct` split a rounded operation into a leading result plus a residual, under their stated arithmetic assumptions. The components preserve the information lost by a single rounded operation. A full expansion is unnecessary in most ordinary cases, but it can resolve a near-zero determinant exactly relative to the binary floating-point inputs.

This creates a **filtered predicate**: cheap ordinary precision handles well-separated cases, while rare ambiguous cases trigger higher precision. The result is exact for the supplied floating-point coordinates interpreted as their represented binary values; it cannot recover information lost when the original real coordinates were rounded. This does not require every coordinate, vector, or mesh operation to use arbitrary precision. That distinction keeps robust decisions practical.

### Predicates and constructions have different contracts

A predicate answers a finite choice, often just a sign. A construction produces a value, such as an intersection point or circumcenter. Exact predicates do not magically make every constructed coordinate exact. Conversely, a construction can be approximate while topology remains reliable if all branching decisions use certified predicates and the algorithm accounts for construction error.

For example, two almost-parallel lines may have a very distant intersection. The line-side tests can be exact for the input points, while the returned intersection coordinates can still be poorly conditioned: tiny perturbations in input shift the intersection a great deal. Exactness of a computation and sensitivity of the mathematical problem are different properties. This connects to the standard conditioning analysis: robust predicates prevent an arithmetic sign mistake; they do not make an ill-conditioned geometric configuration insensitive.

### Worked scale check

Let $A=(0,0)$, $B=(10^8,10^8)$, and $C=(2\cdot10^8,2\cdot10^8+1)$. The orientation determinant is

$$
\Delta=10^8(2\cdot10^8+1)-10^8(2\cdot10^8)=10^8>0.
$$

The perpendicular offset of $C$ from line $AB$ is only $\Delta/|B-A|\approx0.707$. The long, large-magnitude coordinates make a naive product-subtraction vulnerable to cancellation even though the answer is not zero. A robust implementation evaluates an error bound from the actual products; if their difference is safely outside the uncertainty, it returns quickly. For a configuration with offset many orders smaller, it falls back. Do not hard-code a universal epsilon: the safe threshold depends on the magnitudes and operations in the expression.

### Where this belongs in a renderer or vision system

Use robust predicates anywhere a floating-point sign controls topology or ownership: triangle edge coverage, clipping against nearly coincident planes, point-in-polygon, mesh repair, Delaunay construction, and geometric classification. For rasterization, exact edge equations plus a consistent boundary ownership rule prevent adjacent triangles from both claiming or both dropping a shared sample. For a camera-calibration pipeline, robust predicates are useful for data validation and geometric classification; the nonlinear parameter fit itself still needs residual modeling, conditioning analysis, and a sensible optimizer.

Practical checks: generate nearly collinear triples at several coordinate scales; compare signs under translation and uniform scale; permute input order and confirm the expected sign flip; include exactly collinear cases; test shared triangle edges for exactly one owner. These checks expose scale-sensitive epsilon patches that appear correct on ordinary scenes.


<!-- end V11.22 -->

<!-- begin V11.06 -->
## The viewing volume is six half-spaces

*Frustum shape/corners/planes, extraction, sphere and AABB tests, oblique clipping.*

The same projection inequalities describe a convex volume. We can reuse them to reject bounds before detailed rendering.

**Frustum geometry**

A perspective viewing frustum is the region of space that can project into the camera's visible image, subject to the near and far clipping distances.

For a conventional finite perspective camera, its shape is a truncated rectangular pyramid.

It has six boundary planes: left, right, top, bottom, near, and far.

The actual geometric viewing volume is the intersection of six half-spaces.

A half-space is the set of points lying on one side of a plane.

### Derive the geometry from field of view

Assume the camera looks along negative view-space $Z$.

Let positive forward depth be:

$$
 d=-z_v>0. 
$$

Let the vertical field of view be $\theta_y$, and aspect ratio be:

$$
 a=\frac WH. 
$$

Define:

$$
 t_y=\tan\left(\frac{\theta_y}{2}\right). 
$$

At depth $d$, the visible half-height is:

$$
 \boxed{h(d)=dt_y}. 
$$

The visible half-width is:

$$
 \boxed{w(d)=ad t_y}. 
$$

Therefore, a view-space point $(x,y,-d)$ is inside the lateral frustum boundaries exactly when:

$$
 \boxed{-ad t_y\le x\le ad t_y} 
$$

and:

$$
 \boxed{-dt_y\le y\le dt_y}. 
$$

With finite near and far distances:

$$
 \boxed{n\le d\le f}. 
$$

These six inequalities describe the entire symmetric perspective viewing frustum.

### Numerical example · V11.06.03

Suppose:

$$
 \theta_y=60^\circ,\qquad a=\frac{16}{9}, 
$$

$$
 n=0.1,\qquad f=100. 
$$

Since:

$$
 \tan30^\circ=\frac1{\sqrt3}\approx0.577350269, 
$$

the half-height at depth $d=10$ is:

$$
 h(10)=10(0.577350269)\approx5.773503. 
$$

The half-width is:

$$
 w(10)=\frac{16}{9}(5.773503)\approx10.264005. 
$$

Thus the visible rectangle at depth ten is approximately:

$$
 x\in[-10.264,10.264], 
$$

$$
 y\in[-5.774,5.774]. 
$$

The point:

$$
 (8,3,-10) 
$$

is inside.

The point:

$$
 (12,3,-10) 
$$

is outside the right frustum boundary.

The point:

$$
 (0,0,-0.05) 
$$

is outside the near boundary because its positive depth is smaller than $0.1$.

### Why a frustum is a convex volume

Each boundary is a linear inequality defining a half-space.

The intersection of convex sets is convex.

A half-space is convex.

Therefore a finite viewing frustum is convex.

This matters because convex volumes admit efficient support-function and separating-plane tests.

We can use very inexpensive bounding-volume tests before performing triangle-level rasterization or ray tracing.

### What frustum culling can and cannot prove

If an object is entirely outside the frustum, it cannot contribute through ordinary direct raster visibility under the assumed camera projection.

But being inside the frustum does not imply the object is visible.

It may be entirely hidden behind another object.

Frustum culling removes geometry outside the camera's viewing volume.

Occlusion culling attempts to remove geometry hidden behind other geometry.

They are distinct tests.

A shadow caster outside the camera frustum can still affect visible shadows. Thus camera-frustum culling must not automatically remove such objects from every rendering or lighting pass.

**Frustum corners**

A finite perspective frustum has eight corners: four on the near rectangle and four on the far rectangle.

For a symmetric camera, we can construct them analytically.

Define:

$$
 t_y=\tan(\theta_y/2). 
$$

Near half-height and half-width:

$$
 \boxed{ h_n=nt_y,\qquad w_n=ant_y }. 
$$

Far half-height and half-width:

$$
 \boxed{ h_f=ft_y,\qquad w_f=aft_y }. 
$$

In our camera coordinate system, the near corners are:

$$
 \boxed{ (\pm w_n,\pm h_n,-n) }. 
$$

The far corners are:

$$
 \boxed{ (\pm w_f,\pm h_f,-f) }. 
$$

Every independent choice of the two signs gives one corner.

### Worked construction

Let:

$$
 \theta_y=90^\circ,\qquad a=1, 
$$

$$
 n=1,\qquad f=3. 
$$

Because:

$$
 \tan45^\circ=1, 
$$

we obtain:

$$
 h_n=w_n=1, 
$$

$$
 h_f=w_f=3. 
$$

The near corners are:

$$
 (-1,-1,-1),\ (1,-1,-1), 
$$

$$
 (1,1,-1),\ (-1,1,-1). 
$$

The far corners are:

$$
 (-3,-3,-3),\ (3,-3,-3), 
$$

$$
 (3,3,-3),\ (-3,3,-3). 
$$

Notice that far-plane dimensions are three times near-plane dimensions because the far depth is three times the near depth.

### Transform corners into world space

Suppose the camera has world position $\mathbf c$ and camera-to-world rotation $R$.

Then:

$$
 \boxed{ \mathbf p_W=\mathbf c+R\mathbf p_C }. 
$$

For any of the eight view-space corners, use this transformation to obtain the corresponding world-space point.

Equivalently, if $C$ is the camera-to-world homogeneous matrix:

$$
 \boxed{ \tilde{\mathbf p}_W=C\tilde{\mathbf p}_C }. 
$$

### Frustum corners from inverse view-projection

For a general projection, including an asymmetric or off-axis camera, the analytical symmetric formulas may not apply.

Instead, construct the inverse view-projection:

$$
 \boxed{ Q=(PV)^{-1} }. 
$$

For a conventional zero-to-one clip-depth camera, the eight NDC corner positions are:

$$
 (x_n,y_n,z_n) 
$$

with:

$$
 x_n\in\\{-1,1\\},\quad y_n\in\\{-1,1\\},\quad z_n\in\\{0,1\\}. 
$$

Lift an NDC corner to homogeneous coordinates:

$$
 \tilde{\mathbf q}= \begin{bmatrix}x_n\\y_n\\z_n\\1\end{bmatrix}. 
$$

Compute:

$$
 \tilde{\mathbf p}=Q\tilde{\mathbf q}. 
$$

Then divide by its homogeneous coordinate:

$$
 \boxed{ \mathbf p_W= \frac{\tilde{\mathbf p}_{xyz}}{\tilde p_w} }. 
$$

This reconstructs the world-space frustum corner, assuming the inverse exists and the resulting point is finite.

### Reversed-Z changes corner assignments

For conventional zero-to-one depth:

$$
 z_n=0 \text{ is near},\qquad z_n=1 \text{ is far}. 
$$

For reversed-Z:

$$
 z_n=1 \text{ is near},\qquad z_n=0 \text{ is far}. 
$$

An inverse-projection implementation must use the appropriate depth endpoints.

Otherwise, it can swap its near and far corners or misidentify planes.

### Infinite-far geometry

An infinite-far projection has no finite far rectangle.

When unprojecting the far depth endpoint, a homogeneous result may have:

$$
 w=0. 
$$

This represents a direction or point at infinity rather than a finite Cartesian point.

An implementation should not divide by zero.

For infinite frusta, construct near corners and far rays, or represent the frustum through planes and unbounded directions.

**Frustum planes**

A plane in three-dimensional Euclidean space can be represented by:

$$
 \boxed{ \mathbf n\cdot\mathbf p+d=0 }. 
$$

Here:

$$
 \mathbf n= \begin{bmatrix}a\\b\\c\end{bmatrix} 
$$

is the plane normal, and $d$ is the plane offset.

In expanded form:

$$
 \boxed{ ax+by+cz+d=0 }. 
$$

The four plane coefficients can be stored as:

$$
 \boxed{ \boldsymbol\pi=(a,b,c,d)^T }. 
$$

### Signed distance

If the normal is unit length:

$$
 \\|\mathbf n\\|=1, 
$$

then:

$$
 \boxed{ s(\mathbf p)=\mathbf n\cdot\mathbf p+d } 
$$

is the signed Euclidean distance from the point to the plane.

If the normal is not normalized, the signed Euclidean distance is:

$$
 \boxed{ s(\mathbf p)= \frac{\mathbf n\cdot\mathbf p+d}{\\|\mathbf n\\|} }. 
$$

This distinction is crucial for sphere culling.

A plane equation can correctly classify a point's side without normalized coefficients, but a sphere-radius test involving physical distance must account for the normal's magnitude.

### Choose inward-facing normals

For frustum culling, a convenient convention is to orient every plane normal toward the interior.

Then:

$$
 \boxed{ \mathbf n_i\cdot\mathbf p+d_i\ge0 } 
$$

means the point is on or inside plane $i$'s permitted half-space.

A point is inside the entire frustum if it satisfies all six inequalities.

### Derive the near and far planes

For our view space, positive forward depth is:

$$
 d_{\mathrm{view}}=-z. 
$$

The near condition:

$$
 d_{\mathrm{view}}\ge n 
$$

becomes:

$$
 -z\ge n. 
$$

Thus:

$$
 \boxed{ -z-n\ge0 }. 
$$

The inward-facing near plane is:

$$
 \boxed{ \boldsymbol\pi_{\mathrm{near}}=(0,0,-1,-n) }. 
$$

The far condition:

$$
 d_{\mathrm{view}}\le f 
$$

becomes:

$$
 -z\le f. 
$$

Therefore:

$$
 \boxed{ z+f\ge0 }. 
$$

The inward-facing far plane is:

$$
 \boxed{ \boldsymbol\pi_{\mathrm{far}}=(0,0,1,f) }. 
$$

The unit normals point toward the interior.

### Derive the side planes

Let:

$$
 t_x=a\tan(\theta_y/2), 
$$

$$
 t_y=\tan(\theta_y/2). 
$$

The left condition is:

$$
 x\ge-d_{\mathrm{view}}t_x. 
$$

Since $d_{\mathrm{view}}=-z$:

$$
 x\ge zt_x. 
$$

Thus:

$$
 \boxed{x-t_xz\ge0}. 
$$

The unnormalized left plane is:

$$
 \boxed{ \boldsymbol\pi_L=(1,0,-t_x,0) }. 
$$

The right condition is:

$$
 x\le-d_{\mathrm{view}}t_x=-zt_x. 
$$

Therefore:

$$
 \boxed{-x-t_xz\ge0}, 
$$

giving:

$$
 \boxed{ \boldsymbol\pi_R=(-1,0,-t_x,0) }. 
$$

Similarly:

$$
 \boxed{ \boldsymbol\pi_B=(0,1,-t_y,0) } 
$$

and:

$$
 \boxed{ \boldsymbol\pi_T=(0,-1,-t_y,0) }. 
$$

For a symmetric perspective frustum, these four side planes pass through the camera origin.

Their plane offsets are zero.

### Normalize the side planes

The left normal is:

$$
 \mathbf n_L=(1,0,-t_x). 
$$

Its length is:

$$
 \sqrt{1+t_x^2}. 
$$

Therefore the normalized plane is:

$$
 \boxed{ \hat{\boldsymbol\pi}_L= \frac{(1,0,-t_x,0)} {\sqrt{1+t_x^2}} }. 
$$

The same normalization applies to the remaining side planes.

For a plane $(a,b,c,d)$, always divide all four coefficients by:

$$
 \sqrt{a^2+b^2+c^2}. 
$$

Normalizing only the first three coefficients but leaving the offset untouched changes the plane.

**Extracting frustum planes from a view-projection matrix**

This is one of the most useful frustum-culling derivations.

Instead of explicitly constructing frustum corners and taking cross products, we can extract plane inequalities directly from the view-projection matrix.

Let:

$$
 \boxed{ C=PV } 
$$

map world-space homogeneous points into clip space.

Write its rows as:

$$
 C= \begin{bmatrix} \mathbf r_0^T\\ \mathbf r_1^T\\ \mathbf r_2^T\\ \mathbf r_3^T \end{bmatrix}. 
$$

For a world-space homogeneous point $\tilde{\mathbf p}$:

$$
 x_c=\mathbf r_0^T\tilde{\mathbf p}, 
$$

$$
 y_c=\mathbf r_1^T\tilde{\mathbf p}, 
$$

$$
 z_c=\mathbf r_2^T\tilde{\mathbf p}, 
$$

$$
 w_c=\mathbf r_3^T\tilde{\mathbf p}. 
$$

The clip-space inequalities can therefore be pulled back into world-space plane inequalities.

### Derive the left plane

The clip-space left boundary is:

$$
 -w_c\le x_c. 
$$

Rearrange:

$$
 x_c+w_c\ge0. 
$$

Substitute the row expressions:

$$
 (\mathbf r_0+\mathbf r_3)^T\tilde{\mathbf p}\ge0. 
$$

Thus:

$$
 \boxed{ \boldsymbol\pi_L=\mathbf r_3+\mathbf r_0 }. 
$$

### Right plane

The right boundary is:

$$
 x_c\le w_c. 
$$

Thus:

$$
 w_c-x_c\ge0. 
$$

Therefore:

$$
 \boxed{ \boldsymbol\pi_R=\mathbf r_3-\mathbf r_0 }. 
$$

### Bottom and top planes

The bottom clip inequality is:

$$
 y_c+w_c\ge0. 
$$

Thus:

$$
 \boxed{ \boldsymbol\pi_B=\mathbf r_3+\mathbf r_1 }. 
$$

The top inequality is:

$$
 w_c-y_c\ge0. 
$$

Thus:

$$
 \boxed{ \boldsymbol\pi_T=\mathbf r_3-\mathbf r_1 }. 
$$

### Near and far for zero-to-one clip depth

For Vulkan/Metal-style zero-to-one clipping:

$$
 0\le z_c\le w_c. 
$$

The lower-depth boundary is:

$$
 z_c\ge0. 
$$

Therefore:

$$
 \boxed{ \boldsymbol\pi_{z=0}=\mathbf r_2 }. 
$$

The upper-depth boundary is:

$$
 w_c-z_c\ge0. 
$$

Therefore:

$$
 \boxed{ \boldsymbol\pi_{z=w}=\mathbf r_3-\mathbf r_2 }. 
$$

These are the two depth-boundary planes.

For a conventional zero-to-one projection, $z=0$ is the near plane.

For reversed-Z, $z=w$ is the near plane.

The extraction identities do not change under reversed-Z, but the semantic labels of the two depth planes do.

### OpenGL-style negative-one-to-one depth

Under traditional OpenGL clip depth:

$$
 -w_c\le z_c\le w_c. 
$$

The lower-depth plane is:

$$
 \boxed{ \boldsymbol\pi_{z=-w}=\mathbf r_3+\mathbf r_2 }. 
$$

The upper-depth plane remains:

$$
 \boxed{ \boldsymbol\pi_{z=w}=\mathbf r_3-\mathbf r_2 }. 
$$

Therefore a plane extractor must know the active clip-depth convention.

### Summary extraction table

| Frustum boundary                | Plane coefficients using matrix rows |
| ------------------------------- | ------------------------------------ |
| Left                            | $\mathbf r_3+\mathbf r_0$        |
| Right                           | $\mathbf r_3-\mathbf r_0$        |
| Bottom                          | $\mathbf r_3+\mathbf r_1$        |
| Top                             | $\mathbf r_3-\mathbf r_1$        |
| Zero-to-one lower depth         | $\mathbf r_2$                    |
| Negative-one-to-one lower depth | $\mathbf r_3+\mathbf r_2$        |
| Upper depth                     | $\mathbf r_3-\mathbf r_2$        |

These formulas assume column vectors, with world points multiplied on the right of the matrix:

$$
 \mathbf p_c=C\mathbf p_w. 
$$

For row-vector mathematics, corresponding formulas use columns rather than rows.

Row-major versus column-major storage does not determine whether to extract rows or columns. Multiplication convention does.

### Normalize and validate

For each extracted plane:

$$
 \boldsymbol\pi=(a,b,c,d), 
$$

calculate:

$$
 \ell=\sqrt{a^2+b^2+c^2}. 
$$

If:

$$
 \ell>0, 
$$

normalize:

$$
 \boxed{ \hat{\boldsymbol\pi}=\boldsymbol\pi/\ell }. 
$$

If $\ell$ is zero or numerically degenerate, the boundary may represent a redundant constraint rather than a finite plane.

This is especially relevant to infinite-far projections.

For an infinite reversed-Z projection, the far boundary can degenerate into a redundant clip inequality. A robust extractor should identify and omit it rather than attempt to normalize a zero normal.

The classic derivation of extracting frustum planes from combined transforms was presented by Gil Gribb and Klaus Hartmann; the exact row/column details must be adapted to the stated matrix convention.&#x20;


scottfielder.com

### Model-space frustum planes

You can extract planes from:

$$
 C=PVM 
$$

to obtain clipping inequalities expressed in the object's local coordinates.

This lets you test local-space bounds directly.

Alternatively, extract world-space planes from $PV$ and transform bounds into world space.

Both can be correct.

But choose one consistent coordinate space for the plane and the bound. Testing a world-space sphere against local-space planes is meaningless even when all inputs are finite.

**Sphere-frustum culling**

A sphere is defined by centre:

$$
 \mathbf c 
$$

and radius:

$$
 r\ge0. 
$$

Suppose we have a normalized inward-facing plane:

$$
 \mathbf n\cdot\mathbf p+d=0, \qquad \\|\mathbf n\\|=1. 
$$

The signed distance from the sphere centre to the plane is:

$$
 \boxed{ s=\mathbf n\cdot\mathbf c+d }. 
$$

The sphere extends $r$ units from its centre in every direction.

Therefore, its smallest signed distance to the plane is:

$$
 s-r. 
$$

Its largest signed distance is:

$$
 s+r. 
$$

### Derive the three cases

If:

$$
 \boxed{s<-r}, 
$$

even the nearest part of the sphere toward the plane's interior remains outside.

The sphere is completely outside that half-space.

It can be culled.

If:

$$
 \boxed{s>r}, 
$$

the sphere lies entirely inside that half-space.

If:

$$
 \boxed{-r\le s\le r}, 
$$

the sphere intersects the plane.

### Culling against all planes

For a convex frustum represented by inward planes, if a sphere lies entirely outside any one frustum plane, it cannot intersect the frustum.

Thus:

```
bool spherePossiblyVisible(
    const Sphere& sphere,
    std::span<const Plane> inwardPlanes
) {
    for (const Plane& plane : inwardPlanes) {
        const double distance =
            dot(plane.normal, sphere.center) +
            plane.offset;

        if (distance < -sphere.radius) {
            return false;
        }
    }

    return true;
}
```

This code assumes every plane normal has unit length and both the sphere and planes use the same coordinate system.

### Numerical example · V11.06.26

Take the near plane:

$$
 \boldsymbol\pi_N=(0,0,-1,-1). 
$$

The permitted region is:

$$
 -z-1\ge0, 
$$

meaning positive view depth is at least one.

Suppose the sphere has:

$$
 \mathbf c=(0,0,-0.5) 
$$

and radius:

$$
 r=0.2. 
$$

Its signed distance is:

$$
 s=0.5-1=-0.5. 
$$

Because:

$$
 -0.5<-0.2, 
$$

the sphere is entirely outside the near plane.

Cull it.

Now increase the radius:

$$
 r=0.6. 
$$

Then:

$$
 -r=-0.6. 
$$

Since:

$$
 s=-0.5\ge-0.6, 
$$

the sphere intersects the near plane.

We must retain it as potentially visible.

### Why this test is conservative

A sphere might pass every individual plane test and still not intersect the finite frustum, especially near corners or edges.

Passing all plane tests means no single plane by itself proves disjointness.

It is not a complete geometric intersection proof for every pair of convex objects.

That is acceptable for conservative culling because a false positive wastes rendering work but preserves the image.

A false negative, incorrectly culling visible geometry, is more dangerous.

### Transforming a local sphere into world space

Suppose a local sphere has radius $r_L$, and the object's linear transform is $A$.

Under pure uniform scale $s$:

$$
 r_W=|s|r_L. 
$$

For nonuniform scale, the transformed sphere becomes an ellipsoid.

To obtain a conservative world-space enclosing sphere, use:

$$
 \boxed{ r_W=\\|A\\|_2r_L } 
$$

where $\\|A\\|_2$ is the largest singular value of $A$.

A simpler conservative upper bound is the Frobenius norm:

$$
 r_W\le\\|A\\|_Fr_L, 
$$

although it may be looser.

Using only the largest diagonal scale component is not always conservative if shear is present.

**AABB-frustum culling**

An axis-aligned bounding box, or AABB, is defined by minimum and maximum coordinate vectors:

$$
 \mathbf b_{\min}, \qquad \mathbf b_{\max}. 
$$

Every contained point satisfies:

$$
 b_{\min,i}\le p_i\le b_{\max,i}. 
$$

For efficient plane testing, it is useful to express the box using its centre and half-extents.

Define:

$$
 \boxed{ \mathbf c= \frac{\mathbf b_{\min}+\mathbf b_{\max}}2 } 
$$

and:

$$
 \boxed{ \mathbf e= \frac{\mathbf b_{\max}-\mathbf b_{\min}}2 }. 
$$

Any point inside the box can be written:

$$
 \mathbf p=\mathbf c+\boldsymbol\delta 
$$

with:

$$
 |\delta_x|\le e_x, \qquad |\delta_y|\le e_y, \qquad |\delta_z|\le e_z. 
$$

### Derive the projected radius

For a normalized plane normal $\mathbf n$, the plane expression is:

$$
 s(\mathbf p)=\mathbf n\cdot\mathbf p+d. 
$$

Substitute:

$$
 \mathbf p=\mathbf c+\boldsymbol\delta. 
$$

Then:

$$
 s(\mathbf p)= \mathbf n\cdot\mathbf c+d+ \mathbf n\cdot\boldsymbol\delta. 
$$

Define centre distance:

$$
 \boxed{ s_c=\mathbf n\cdot\mathbf c+d }. 
$$

Now find the maximum possible magnitude of the offset term:

$$
 \mathbf n\cdot\boldsymbol\delta = n_x\delta_x+n_y\delta_y+n_z\delta_z. 
$$

Because each $\delta_i$ is bounded, the maximum absolute projection is:

$$
 \boxed{ r_{\mathrm{box}}= |n_x|e_x+|n_y|e_y+|n_z|e_z }. 
$$

This is the box's support radius along the plane normal.

### Culling condition

The minimum signed distance of the box to the plane is:

$$
 s_{\min}=s_c-r_{\mathrm{box}}. 
$$

The box lies entirely outside the inward half-space if its maximum signed distance is negative:

$$
 s_{\max}=s_c+r_{\mathrm{box}}<0. 
$$

Therefore the correct rejection condition is:

$$
 \boxed{ s_c<-r_{\mathrm{box}} }. 
$$

The corresponding conservative code is:

```
bool aabbPossiblyVisible(
    const AABB& box,
    std::span<const Plane> planes
) {
    const Vec3 centre =
        (box.minimum + box.maximum) * 0.5;

    const Vec3 halfExtent =
        (box.maximum - box.minimum) * 0.5;

    for (const Plane& plane : planes) {
        const Vec3 n = plane.normal;

        const double distance =
            dot(n, centre) + plane.offset;

        const double radius =
            std::abs(n.x) * halfExtent.x +
            std::abs(n.y) * halfExtent.y +
            std::abs(n.z) * halfExtent.z;

        if (distance < -radius) {
            return false;
        }
    }

    return true;
}
```

Plane normalization is not strictly required for this AABB support-radius comparison if the distance and radius use the same unnormalized normal, because both scale consistently. It is still useful for predictable tolerances and signed-distance interpretation.

### Numerical example · V11.06.31

Consider the inward plane:

$$
 x\ge0. 
$$

Its normal is:

$$
 \mathbf n=(1,0,0), 
$$

with $d=0$.

Let an AABB have centre:

$$
 \mathbf c=(-3,0,0) 
$$

and half-extents:

$$
 \mathbf e=(1,2,1). 
$$

The centre signed distance is:

$$
 s_c=-3. 
$$

The projected radius is:

$$
 r_{\mathrm{box}}= |1|(1)+0+0=1. 
$$

Since:

$$
 -3<-1, 
$$

the box is outside.

Now move its centre to:

$$
 (-0.5,0,0). 
$$

Then:

$$
 s_c=-0.5. 
$$

Because:

$$
 -0.5\not<-1, 
$$

the box intersects the permitted half-space.

It cannot be culled by that plane.

### Why AABB tests can outperform sphere tests

A sphere is rotationally symmetric and extremely cheap to test.

But it can be a poor fit for long, thin geometry such as walls, pipes, roads, or elongated meshes.

An AABB can fit such geometry more tightly along its coordinate axes.

However, when an object rotates, its world-space AABB can expand substantially.

A local-space AABB transformed by a rotation is an oriented bounding box, not automatically a world-space AABB.

A conservative world AABB can be computed using the transformed centre and absolute linear matrix:

$$
 \boxed{ \mathbf e_W=|A|\mathbf e_L } 
$$

where $|A|$ denotes the matrix formed by taking the absolute value of each element.

This follows from applying componentwise bounds to:

$$
 A\boldsymbol\delta. 
$$

The resulting world AABB is conservative under affine transformations.

### A robust culling pipeline

A practical renderer can use several levels of tests: whole-scene regions, object bounds, mesh clusters, and then detailed geometry.

Frustum culling should use the projection appropriate to the pass being culled.

Shadow cascades, reflections, stereo views, and camera jitter may require different culling volumes.

The camera frustum is not a universal visibility volume for the entire renderer.

**Oblique clipping planes**

An oblique clipping plane is a clipping boundary that is not aligned with the conventional near or far planes of the camera.

Ordinary perspective projection clips against planes such as:

$$
 z_{\mathrm{view}}=-n. 
$$

An oblique plane might instead have the equation:

$$
 \boxed{ ax+by+cz+d=0 } 
$$

with a normal not parallel to the camera's forward axis.

Examples include reflection planes, portal surfaces, and custom section cuts.

### Why ordinary near clipping is insufficient

Imagine rendering a mirror reflection.

A reflected camera observes the world from a virtual mirrored viewpoint.

We want to reject geometry lying on the wrong side of the mirror plane.

The conventional near plane may not coincide with that physical mirror surface.

If we keep ordinary clipping, geometry on the wrong side can contribute to the reflection image.

An oblique near plane makes the projection's clipping boundary match the desired geometric plane.

### Plane transformation between spaces

Suppose a plane in world space is:

$$
 \boldsymbol\pi_W= \begin{bmatrix} a\\b\\c\\d \end{bmatrix}. 
$$

A point transforms from world to view using:

$$
 \mathbf p_V=V\mathbf p_W. 
$$

Thus:

$$
 \mathbf p_W=V^{-1}\mathbf p_V. 
$$

The world plane equation is:

$$
 \boldsymbol\pi_W^T\mathbf p_W=0. 
$$

Substitute:

$$
 \boldsymbol\pi_W^TV^{-1}\mathbf p_V=0. 
$$

We want:

$$
 \boldsymbol\pi_V^T\mathbf p_V=0. 
$$

Therefore:

$$
 \boxed{ \boldsymbol\pi_V=V^{-T}\boldsymbol\pi_W }. 
$$

This is the inverse-transpose rule for transforming planes.

The same dual-space principle appears in normal transformations.

### Clipping as a half-space inequality

An inward-facing oblique clip plane can be written:

$$
 \boxed{ \boldsymbol\pi^T\tilde{\mathbf p}\ge0 }. 
$$

The clipping algorithm retains the permitted half-space.

A projection matrix can be modified so that a canonical clip boundary corresponds to this oblique view-space plane.

For zero-to-one clip depth, the canonical lower-depth boundary is:

$$
 z_c\ge0. 
$$

If the third row of a projection matrix is constructed to be proportional to the desired plane coefficients in view space, that lower clip boundary can represent the oblique plane.

But modifying the projection must also preserve a valid overall projective mapping and clipping behavior. Arbitrarily replacing a matrix row may break depth or the other frustum constraints.

### Oblique matrix construction is convention-dependent

The classical method associated with Eric Lengyel modifies projection coefficients so the selected clip boundary aligns with an arbitrary plane.

The exact algebra depends on whether the clip interval is $[-1,1]$ or $[0,1]$, the projection's row/column multiplication convention, its reversed-Z configuration, and which depth boundary is to be replaced.

For this reason, an OpenGL-focused oblique-projection snippet should not be copied directly into a Vulkan reversed-Z renderer.

A reliable implementation should derive the intended clip-plane equality, retain appropriate frustum boundaries, and test known points on both sides.

### Practical safeguards

The clip plane must be oriented so the intended half-space is retained.

It must not pass through an invalid camera configuration that makes the resulting projection singular or numerically unstable.

Reflection pipelines must also account for reflected handedness, face winding, clipping, and stencil or portal geometry.

A robust engine treats oblique clipping as a projection variant with explicit conventions, not as an unexplained adjustment to a few matrix entries.


### Figure C11-2 — Culling is a conservative half-space test

![[Visual Recall/Math Foundations/C11-2.png]]

This s–x cross-section has near=1, far=5 and ±45° side planes. For an inward unit plane normal, a sphere is outside only when signed centre distance < −radius. For an AABB use r = |nx|ex + |ny|ey + |nz|ez. Passing each plane does not prove exact intersection with the whole frustum.


<!-- end V11.06 -->

<!-- begin V11.10 -->
## Raster interpolation remembers homogeneous depth

*Perspective-correct interpolation, reciprocal depth, barycentrics, edge functions.*

A rasterizer interpolates after projection, so its screen-space weights differ from the original triangle weights.

**Why perspective-correct interpolation is necessary**

Suppose a triangle has texture coordinates assigned to its 3D vertices.

After perspective projection, rasterization computes barycentric coordinates relative to the projected triangle.

A tempting implementation is:

$$
 \boxed{ a_{\mathrm{naive}}= \lambda_0a_0+\lambda_1a_1+\lambda_2a_2 }. 
$$

Here $\lambda_i$ are screen-space barycentric weights.

But this generally does not equal the attribute value of the corresponding point on the original 3D triangle.

Why?

Because perspective projection does not preserve affine ratios along a line.

### A simple segment example

Consider two 3D endpoints along a projected line.

Endpoint 0 lies at positive camera depth:

$$
 d_0=1. 
$$

Endpoint 1 lies at:

$$
 d_1=4. 
$$

Let an attribute such as texture coordinate be:

$$
 a_0=0, 
$$

$$
 a_1=1. 
$$

Suppose a screen-space point is halfway between the projected endpoints.

A naive screen-space interpolation gives:

$$
 \boxed{ a_{\mathrm{naive}}=0.5 }. 
$$

But the 3D segment is not sampled halfway in its original affine parameter.

The nearer endpoint influences projected position more strongly because perspective magnification is inversely proportional to depth.

### Derive the corrected midpoint

For a screen-space halfway point, use weights:

$$
 \lambda_0=\lambda_1=0.5. 
$$

The perspective-correct attribute formula is:

$$
 a= \frac{ \lambda_0a_0/d_0+\lambda_1a_1/d_1 }{ \lambda_0/d_0+\lambda_1/d_1 }. 
$$

Substitute:

$$
 a= \frac{ 0.5(0)/1+0.5(1)/4 }{ 0.5/1+0.5/4 }. 
$$

The numerator is:

$$
 0.125. 
$$

The denominator is:

$$
 0.5+0.125=0.625. 
$$

Therefore:

$$
 \boxed{ a=\frac{0.125}{0.625}=0.2 }. 
$$

The true projectively corrected attribute is:

$$
 \boxed{0.2}, 
$$

not $0.5$.

This is a large difference.

### What visual error does naive interpolation cause?

Imagine a checkerboard texture mapped onto a floor receding into the distance.

If the rasterizer interpolates UV coordinates linearly in screen space, the pattern will warp incorrectly.

The texture may appear to slide, bend, or compress in ways inconsistent with the 3D surface.

Perspective-correct interpolation ensures that attributes vary according to the underlying projected 3D geometry.

It applies to UVs, colours, normals, positions, and other smoothly interpolated vertex attributes when those attributes are defined as affine functions over the original primitive.

### Depth is a special case

Depth interpolation deserves care.

The post-projection NDC depth is a quantity designed to be interpolated affinely across a screen-space triangle under the rasterizer's depth formulation.

It should not simply be treated as an arbitrary texture attribute requiring the same corrective division again.

For an attribute $a$ defined on a 3D triangle, the perspective-correct formula involves $a_i/w_i$.

For clip-space depth, the relationship follows directly from its own homogeneous numerator and denominator and the rasterizer's interpolation rules.

Understanding which quantity is being interpolated prevents double-correcting depth or mixing different spaces.

Scratchapixel's rasterization treatment works through this distinction in detail.&#x20;


**Deriving perspective-correct attribute interpolation**

We now derive the full formula for a triangle rather than memorizing the result.

Let the original 3D triangle vertices be:

$$
 P_0,P_1,P_2. 
$$

Let a point on the original triangle have 3D affine barycentric coordinates:

$$
 \alpha_0,\alpha_1,\alpha_2. 
$$

Then:

$$
 \boxed{ \alpha_0+\alpha_1+\alpha_2=1 } 
$$

and:

$$
 \boxed{ P=\alpha_0P_0+\alpha_1P_1+\alpha_2P_2 }. 
$$

Suppose an attribute is affinely interpolated on the original triangle:

$$
 \boxed{ a=\alpha_0a_0+\alpha_1a_1+\alpha_2a_2 }. 
$$

The problem is that the rasterizer knows screen-space barycentric weights:

$$
 \lambda_0,\lambda_1,\lambda_2, 
$$

not the original $\alpha_i$.

We need a relationship between them.

### Homogeneous projected vertices

Let the projected homogeneous vertices be:

$$
 C_i= \begin{bmatrix} x_i\\y_i\\z_i\\w_i \end{bmatrix}. 
$$

Their NDC XY positions are:

$$
 q_i= \begin{bmatrix} x_i/w_i\\ y_i/w_i \end{bmatrix}. 
$$

Because projective transformations are linear in homogeneous coordinates, the homogeneous image of the affine 3D combination is proportional to:

$$
 \boxed{ C=\sum_i\alpha_iC_i }. 
$$

Its homogeneous denominator is:

$$
 \boxed{ W=\sum_i\alpha_iw_i }. 
$$

Its projected XY position is:

$$
 q= \frac{ \sum_i\alpha_iw_iq_i }{ \sum_i\alpha_iw_i }. 
$$

Define:

$$
 \boxed{ \lambda_i=\frac{\alpha_iw_i}{W} }. 
$$

Then:

$$
 \sum_i\lambda_i=1. 
$$

And:

$$
 q=\sum_i\lambda_iq_i. 
$$

Thus $\lambda_i$ are precisely the affine barycentric weights in the projected screen-space triangle.

### Solve for the original weights

From:

$$
 \lambda_i=\frac{\alpha_iw_i}{W}, 
$$

we get:

$$
 \alpha_i=\lambda_i\frac W{w_i}. 
$$

We still need $W$.

Use:

$$
 \sum_i\alpha_i=1. 
$$

Therefore:

$$
 \sum_i\lambda_i\frac W{w_i}=1. 
$$

Factor $W$:

$$
 W\sum_i\frac{\lambda_i}{w_i}=1. 
$$

Hence:

$$
 \boxed{ W= \frac1{\sum_i\lambda_i/w_i} }. 
$$

Substitute:

$$
 \boxed{ \alpha_i= \frac{\lambda_i/w_i} {\sum_j\lambda_j/w_j} }. 
$$

This is the central relationship between screen-space and original-triangle barycentric coordinates.

### Recover the attribute

We know:

$$
 a=\sum_i\alpha_i a_i. 
$$

Substitute the recovered $\alpha_i$:

$$
 a= \sum_i a_i \frac{\lambda_i/w_i} {\sum_j\lambda_j/w_j}. 
$$

Factor the common denominator:

$$
 \boxed{ a= \frac{ \sum_i\lambda_i a_i/w_i }{ \sum_i\lambda_i/w_i } }. 
$$

That is perspective-correct interpolation.

It works componentwise for vector attributes.

### Numerical triangle example

Suppose a rasterized triangle has screen-space barycentric weights:

$$
 \lambda=(0.5,0.25,0.25). 
$$

Its clip $w$ values are:

$$
 w=(1,4,4). 
$$

The scalar vertex attribute is:

$$
 a=(0,1,0). 
$$

Naive interpolation gives:

$$
 a_{\mathrm{naive}} = 0.5(0)+0.25(1)+0.25(0) =\boxed{0.25}. 
$$

Perspective-correct numerator:

$$
 N= 0.5\frac01+ 0.25\frac14+ 0.25\frac04 =0.0625. 
$$

Denominator:

$$
 D= 0.5\frac11+ 0.25\frac14+ 0.25\frac14. 
$$

Thus:

$$
 D=0.5+0.0625+0.0625=0.625. 
$$

Therefore:

$$
 \boxed{ a=\frac{0.0625}{0.625}=0.1 }. 
$$

This example is tested in the compiled C++23 laboratory.

### Derive reciprocal-depth interpolation

From the earlier relationship:

$$
 W=\frac1{\sum_i\lambda_i/w_i}, 
$$

we obtain:

$$
 \boxed{ \frac1W= \sum_i\frac{\lambda_i}{w_i} }. 
$$

Thus reciprocal homogeneous $w$ is affine in screen-space barycentric coordinates.

For the conventional perspective matrix with:

$$
 w=d, 
$$

this becomes:

$$
 \boxed{ \frac1d= \sum_i\lambda_i\frac1{d_i} }. 
$$

This explains why reciprocal depth plays such a central role in rasterization.

### Why this is a rational expression

The corrected attribute is a ratio of two affine expressions in screen-space barycentric weights.

For a segment, it has the form:

$$
 a(t)= \frac{A+Bt}{C+Dt}. 
$$

This is a rational-linear or linear-fractional function.

It is not generally a straight linear interpolation in $t$.

That is the algebraic footprint of perspective projection.

### Important implementation choices

Hardware graphics APIs usually provide perspective-correct interpolation by default for smoothly interpolated fragment attributes.

Some shader languages offer interpolation qualifiers that explicitly request non-perspective interpolation.

Use non-perspective interpolation for quantities genuinely defined to vary linearly in screen space, not as a blanket performance optimization for arbitrary UVs.

Also distinguish flat interpolation, which assigns a primitive-constant value rather than interpolating it.

The correct interpolation mode is determined by the semantic meaning of the attribute.

**Barycentric coordinates in screen space**

Barycentric coordinates describe a point as an affine combination of triangle vertices.

Let a 2D triangle have raster-space or NDC vertices:

$$
 A=(x_0,y_0), 
$$

$$
 B=(x_1,y_1), 
$$

$$
 C=(x_2,y_2). 
$$

We want weights:

$$
 \lambda_0,\lambda_1,\lambda_2 
$$

satisfying:

$$
 \boxed{ P=\lambda_0A+\lambda_1B+\lambda_2C } 
$$

and:

$$
 \boxed{ \lambda_0+\lambda_1+\lambda_2=1 }. 
$$

### Derive using two edge vectors

Subtract $A$:

$$
 P-A= \lambda_1(B-A)+\lambda_2(C-A) 
$$

because:

$$
 \lambda_0=1-\lambda_1-\lambda_2. 
$$

Define:

$$
 \mathbf e_1=B-A, 
$$

$$
 \mathbf e_2=C-A, 
$$

$$
 \mathbf q=P-A. 
$$

We need to solve:

$$
 \mathbf q= \lambda_1\mathbf e_1+ \lambda_2\mathbf e_2. 
$$

This is a $2\times2$ linear system.

### Derive the determinant form

Define the scalar 2D cross product:

$$
 \boxed{ \operatorname{cross}_2(a,b)=a_xb_y-a_yb_x }. 
$$

Cross both sides with $\mathbf e_2$:

$$
 \operatorname{cross}_2(\mathbf q,\mathbf e_2) = \lambda_1 \operatorname{cross}_2(\mathbf e_1,\mathbf e_2) 
$$

because:

$$
 \operatorname{cross}_2(\mathbf e_2,\mathbf e_2)=0. 
$$

Therefore:

$$
 \boxed{ \lambda_1= \frac{ \operatorname{cross}_2(P-A,C-A) }{ \operatorname{cross}_2(B-A,C-A) } }. 
$$

Similarly, cross $\mathbf e_1$ with both sides:

$$
 \boxed{ \lambda_2= \frac{ \operatorname{cross}_2(B-A,P-A) }{ \operatorname{cross}_2(B-A,C-A) } }. 
$$

Then:

$$
 \boxed{ \lambda_0=1-\lambda_1-\lambda_2 }. 
$$

These are the screen-space barycentric coordinates.

### Numerical example · V11.10.15

Let:

$$
 A=(0,0),\quad B=(1,0),\quad C=(0,1). 
$$

Choose:

$$
 P=(0.25,0.25). 
$$

Then:

$$
 B-A=(1,0), 
$$

$$
 C-A=(0,1), 
$$

$$
 P-A=(0.25,0.25). 
$$

The denominator is:

$$
 \operatorname{cross}_2((1,0),(0,1))=1. 
$$

For $\lambda_1$:

$$
 \operatorname{cross}_2((0.25,0.25),(0,1))=0.25. 
$$

Thus:

$$
 \lambda_1=0.25. 
$$

For $\lambda_2$:

$$
 \operatorname{cross}_2((1,0),(0.25,0.25))=0.25. 
$$

Thus:

$$
 \lambda_2=0.25. 
$$

And:

$$
 \lambda_0=0.5. 
$$

Therefore:

$$
 \boxed{ \lambda=(0.5,0.25,0.25) }. 
$$

### Point-in-triangle testing

For a nondegenerate triangle, the point is inside or on its boundary when:

$$
 \boxed{ \lambda_i\ge0 } 
$$

for all three weights, subject to the chosen boundary convention and numerical tolerance.

If any weight is negative, the point lies outside the triangle.

### Triangle degeneracy

The denominator is twice the oriented area of the triangle:

$$
 \operatorname{cross}_2(B-A,C-A). 
$$

If it is zero, the triangle vertices are collinear and the ordinary barycentric coordinate solution is not unique.

For numerical code, extremely small area also causes sensitivity.

A robust rasterizer uses carefully defined edge functions and coverage conventions.

Floating-point tolerance rules must be chosen with attention to triangle scale and screen-space representation.

### Edge functions

Define an oriented edge function:

$$
 \boxed{ E_{AB}(P)= \operatorname{cross}_2(B-A,P-A) }. 
$$

It is affine in the coordinates of $P$.

For a consistent triangle winding, edge-function signs determine which side of each edge a sample lies on.

This is the basis of triangle rasterization.

The top-left rule used by many rasterizers defines ownership of shared edges so that adjacent triangles do not both incorrectly claim or both omit the same boundary samples.

That is an implementation-level coverage rule layered on top of the underlying barycentric mathematics.


### Figure C11-3 — Attributes follow reciprocal clip w

![[Visual Recall/Math Foundations/C11-3.png]]

Vertex clip w values are (1,4,4); attribute values are (0,1,1). At the screen centroid the affine value is 2/3 and the corrected value is 1/3. Use sum(λ a/w) / sum(λ/w); depth-buffer interpolation has its own projected-depth rule.


<!-- end V11.10 -->

<!-- begin V11.11 -->
## Intrinsics map optical rays to pixels

*Focal length, principal point, pixel focal lengths, resizing/cropping, intrinsic matrix.*

The rendering camera is now reversed into a measurement model: first specify how an optical ray lands on the sensor.

**Camera intrinsics: what belongs to the camera itself?**

A calibrated perspective camera is commonly described using two sets of parameters:

- Intrinsics, which describe how camera-frame rays map to image coordinates.
- Extrinsics, which describe how the camera frame is positioned and oriented relative to a world frame.

This distinction is not absolute across all camera models, but it is standard for pinhole-camera calibration.

The intrinsic parameters include focal lengths expressed in pixel units, principal-point location, and sometimes image-axis skew.

They do not, by themselves, determine where the camera is located in the world.

### Start from the physical pinhole model

Choose a computer-vision optical camera frame in which:

- $+X$ points image-right.
- $+Y$ points image-down.
- $+Z$ points forward along the optical axis.

This differs from our graphics camera frame, where $+Y$ is up and forward is $-Z$.

We will make this conversion explicit later.

For a point in camera coordinates:

$$
 \mathbf X_C= \begin{bmatrix} X_C\\Y_C\\Z_C \end{bmatrix}, 
$$

with:

$$
 Z_C>0, 
$$

the normalized pinhole coordinates are:

$$
 \boxed{ x=\frac{X_C}{Z_C}, \qquad y=\frac{Y_C}{Z_C} }. 
$$

These coordinates describe the direction of the incoming ray relative to the optical axis.

### From normalized image coordinates to pixels

A simple calibrated camera maps normalized coordinates to pixels using:

$$
 \boxed{ u=f_xx+c_x } 
$$

and:

$$
 \boxed{ v=f_yy+c_y }. 
$$

Here $u,v$ are image pixel coordinates.

The parameters have the following meanings.

$$
 f_x 
$$

is the horizontal focal scale in pixels.

$$
 f_y 
$$

is the vertical focal scale in pixels.

$$
 (c_x,c_y) 
$$

is the principal point, expressed in the same pixel-coordinate convention as the image observations.

The camera intrinsic model can include skew:

$$
 \boxed{ u=f_xx+s\,y+c_x }. 
$$

Usually skew is zero or small in modern ordinary digital cameras, but it should not be silently assumed zero when working with an arbitrary calibration file.

### Intrinsics are not just sensor metadata

The focal length recorded in EXIF metadata may be expressed in millimetres.

That is not the same numerical quantity as $f_x$ or $f_y$ in pixels.

To relate physical focal length to pixel focal scale, we also need information about the sensor's physical dimensions or pixel pitch.

Cropping, resizing, image stabilization, binning, and digital transformations can change the effective intrinsics of an image.

Therefore a calibration belongs to a particular image geometry and camera configuration, not merely to a camera manufacturer's model name.

OpenCV's calibration documentation establishes the intrinsic-matrix and extrinsic-matrix conventions used below.&#x20;


**Focal length**

Physical focal length is a distance associated with an optical system.

For an ideal pinhole projection, it controls the scale between ray angle and image-plane position.

Suppose the ideal image plane is at distance:

$$
 f 
$$

from the pinhole.

By similar triangles:

$$
 \frac{x_{\mathrm{image}}}{f} = \frac{X_C}{Z_C}. 
$$

Therefore:

$$
 \boxed{ x_{\mathrm{image}}= f\frac{X_C}{Z_C} }. 
$$

Similarly:

$$
 \boxed{ y_{\mathrm{image}}= f\frac{Y_C}{Z_C} }. 
$$

The image-plane dimensions and $f$ must use the same physical length units.

### Focal length versus field of view

Suppose the physical sensor height is:

$$
 H_s. 
$$

The vertical FOV is:

$$
 \boxed{ \theta_y= 2\arctan\left(\frac{H_s}{2f}\right) } 
$$

for the ideal rectilinear model.

Rearrange:

$$
 \tan(\theta_y/2)=\frac{H_s}{2f}. 
$$

Thus:

$$
 \boxed{ f=\frac{H_s}{2\tan(\theta_y/2)} }. 
$$

A larger focal length produces a narrower FOV when sensor size is fixed.

A smaller focal length produces a wider FOV.

### Numerical example · V11.11.06

Suppose:

$$
 H_s=24\text{ mm}, 
$$

$$
 f=35\text{ mm}. 
$$

Then:

$$
 \theta_y= 2\arctan\left(\frac{24}{70}\right). 
$$

Numerically:

$$
 \boxed{ \theta_y\approx37.85^\circ }. 
$$

The exact value depends on the adopted idealized sensor geometry.

For a real lens, distortion and the lens's projection characteristics can make the relationship more complicated, particularly for very wide-angle lenses.

### Why focal length controls perspective framing but not perspective existence

Sometimes people say that a long focal length compresses perspective.

In an ideal pinhole model, the relative perspective geometry of a fixed camera viewpoint is determined by object depth relationships.

Changing focal length changes the angular field of view and image magnification.

If a photographer moves the camera farther away and increases focal length to preserve an object's apparent size, the changed camera position alters the relative depth geometry. That repositioning produces the familiar perspective-compression effect.

It is important to distinguish changing focal length at a fixed viewpoint from changing viewpoint while maintaining framing.

**The principal point**

The principal point is the image location where the optical axis intersects the ideal image plane under the calibrated camera model.

Its coordinates are:

$$
 \boxed{(c_x,c_y)}. 
$$

It need not coincide exactly with the geometric centre of the image.

### Why it may be off-centre

Several factors can shift the effective principal point: sensor alignment, optical construction, image cropping, digital scaling, and other calibration or processing choices.

For a simple centred camera, one might use:

$$
 c_x\approx W/2, 
$$

$$
 c_y\approx H/2 
$$

when coordinates are defined continuously from the image's outer edges.

But this must be interpreted according to the image-coordinate convention.

If integer pixel indices denote pixel centres at coordinates $0,1,\ldots,W-1$, a geometric centre is often represented as:

$$
 \frac{W-1}{2}. 
$$

These are two coordinate conventions for the same geometric idea, differing by half a pixel.

A calibration should explicitly define whether integer coordinates refer to pixel centres or whether pixel centres are at half-integers.

### Principal-point offset changes ray directions

The normalized camera ray corresponding to pixel $(u,v)$, assuming zero skew and no distortion, has direction proportional to:

$$
 \boxed{ \mathbf d_C= \begin{bmatrix} (u-c_x)/f_x\\ (v-c_y)/f_y\\ 1 \end{bmatrix} }. 
$$

If $c_x$ changes while $u$ remains fixed, the inferred horizontal ray angle changes.

Thus a principal-point error causes systematic reprojection errors, especially when combining images from different cameras.

### Numerical example · V11.11.10

Suppose:

$$
 W=1920,\quad H=1080. 
$$

The nominal centred optical location under a continuous image-edge coordinate convention is:

$$
 (960,540). 
$$

But calibration gives:

$$
 (c_x,c_y)=(968,535). 
$$

The principal point is shifted eight pixels right and five pixels upward relative to that nominal centre.

A point along the optical axis maps to:

$$
 \boxed{(u,v)=(968,535)}. 
$$

If an algorithm assumes $(960,540)$, it assigns the optical-axis point an incorrect normalized direction.

**Pixel focal lengths $f_x$ and $f_y$**

The physical focal length $f$ is measured in length units.

The intrinsic focal parameters $f_x,f_y$ are measured in pixels.

Suppose the horizontal pixel pitch is:

$$
 p_x 
$$

millimetres per pixel.

Then:

$$
 \boxed{ f_x=\frac f{p_x} }. 
$$

Similarly, if vertical pixel pitch is $p_y$:

$$
 \boxed{ f_y=\frac f{p_y} }. 
$$

If the pixel pitches differ, $f_x$ and $f_y$ can differ even when physical focal length is the same.

### Sensor dimensions

If the physical sensor width is $S_x$ and the image width is $W$ pixels:

$$
 p_x=\frac{S_x}{W}. 
$$

Therefore:

$$
 \boxed{ f_x=\frac{fW}{S_x} }. 
$$

Likewise:

$$
 \boxed{ f_y=\frac{fH}{S_y} }. 
$$

These formulas assume the stated sensor region maps to the stated image dimensions without additional crop, resampling, or calibration changes.

### Numerical example · V11.11.12

Suppose:

$$
 f=35\text{ mm}, 
$$

$$
 S_x=36\text{ mm}, 
$$

$$
 W=6000\text{ pixels}. 
$$

Then:

$$
 f_x=\frac{35(6000)}{36}. 
$$

Thus:

$$
 \boxed{ f_x\approx5833.33\text{ pixels} }. 
$$

If the sensor height is 24 mm and the image height is 4000 pixels:

$$
 f_y=\frac{35(4000)}{24}. 
$$

Therefore:

$$
 \boxed{ f_y\approx5833.33\text{ pixels} }. 
$$

The two values agree because the image and sensor aspect ratios agree and the assumed pixels are square.

### How intrinsics change when resizing an image

Suppose the original image is $W\times H$.

Resize it by horizontal scale:

$$
 s_x 
$$

and vertical scale:

$$
 s_y. 
$$

Under a compatible continuous pixel-coordinate convention:

$$
 \boxed{ f_x'=s_xf_x, \qquad f_y'=s_yf_y }. 
$$

The principal point also transforms:

$$
 \boxed{ c_x'=s_xc_x, \qquad c_y'=s_yc_y }. 
$$

However, if the resize operation uses a pixel-centre alignment convention, half-pixel offsets may appear in the exact coordinate transformation.

For example, a common centre-preserving transformation between integer-centred pixel coordinate systems is:

$$
 u'=(u+0.5)s_x-0.5. 
$$

Then:

$$
 \boxed{ c_x'=(c_x+0.5)s_x-0.5 }. 
$$

The correct formula depends on the image resampler's coordinate convention.

### Cropping

If an image is cropped by removing $x_0$ pixels from the left and $y_0$ pixels from the top, under a consistent pixel coordinate system:

$$
 \boxed{ c_x'=c_x-x_0, \qquad c_y'=c_y-y_0 }. 
$$

Focal scales remain unchanged when cropping alone does not rescale the image.

Failing to update the principal point after cropping creates systematic calibration errors.

**The camera intrinsic matrix $K$**

The intrinsic parameters can be assembled into one matrix:

$$
 \boxed{ K= \begin{bmatrix} f_x&s&c_x\\ 0&f_y&c_y\\ 0&0&1 \end{bmatrix} }. 
$$

Here $s$ is the skew coefficient, expressed consistently with the camera's pixel-coordinate model.

For the common zero-skew case:

$$
 \boxed{ K= \begin{bmatrix} f_x&0&c_x\\ 0&f_y&c_y\\ 0&0&1 \end{bmatrix} }. 
$$

### From camera coordinates to homogeneous pixels

Take:

$$
 \mathbf X_C= \begin{bmatrix} X_C\\Y_C\\Z_C \end{bmatrix}. 
$$

Multiply:

$$
 K\mathbf X_C= \begin{bmatrix} f_xX_C+sY_C+c_xZ_C\\ f_yY_C+c_yZ_C\\ Z_C \end{bmatrix}. 
$$

This is a homogeneous pixel coordinate.

Divide by the third component $Z_C$:

$$
 \boxed{ u=f_x\frac{X_C}{Z_C} +s\frac{Y_C}{Z_C} +c_x } 
$$

and:

$$
 \boxed{ v=f_y\frac{Y_C}{Z_C} +c_y }. 
$$

Therefore:

$$
 \boxed{ \lambda \begin{bmatrix}u\\v\\1\end{bmatrix} = K\mathbf X_C } 
$$

for an appropriate nonzero homogeneous scale $\lambda$, often $Z_C$.

### Numerical example · V11.11.16

Let:

$$
 K= \begin{bmatrix} 800&0&960\\ 0&820&535\\ 0&0&1 \end{bmatrix}. 
$$

Consider camera-space point:

$$
 \mathbf X_C= \begin{bmatrix}1\\0.5\\2\end{bmatrix}. 
$$

Its normalized coordinates are:

$$
 x=\frac12=0.5, 
$$

$$
 y=\frac{0.5}{2}=0.25. 
$$

Pixel X:

$$
 u=800(0.5)+960. 
$$

Thus:

$$
 \boxed{u=1360}. 
$$

Pixel Y:

$$
 v=820(0.25)+535. 
$$

Therefore:

$$
 \boxed{v=740}. 
$$

The resulting pixel location is:

$$
 \boxed{(1360,740)}. 
$$

This exact numerical case is tested in the reference implementation.

### Inverse intrinsics

Suppose we observe an ideal undistorted pixel coordinate:

$$
 (u,v). 
$$

We want the normalized camera ray.

For zero skew:

$$
 \boxed{ x=\frac{u-c_x}{f_x} } 
$$

and:

$$
 \boxed{ y=\frac{v-c_y}{f_y} }. 
$$

The unnormalized camera ray is:

$$
 \boxed{ \mathbf d_C= \begin{bmatrix} x\\y\\1 \end{bmatrix} }. 
$$

With skew, solve the Y coordinate first:

$$
 y=\frac{v-c_y}{f_y}. 
$$

Then:

$$
 \boxed{ x=\frac{u-c_x-sy}{f_x} }. 
$$

This is equivalent to applying:

$$
 K^{-1} 
$$

to the homogeneous pixel vector.

### Why $K$ is not the same as a GPU projection matrix

The intrinsic matrix maps ideal camera rays to image pixel coordinates.

A GPU perspective projection matrix additionally maps 3D points into homogeneous clip coordinates and defines depth clipping.

It is possible to construct a graphics projection matrix corresponding to $K$, but doing so requires image dimensions, raster origin, optical-axis conventions, near/far depth behavior, and sometimes a half-pixel coordinate adjustment.

A $3\times3$ calibrated camera matrix and a $4\times4$ graphics projection matrix are related, but they are not interchangeable data structures.


<!-- end V11.11 -->


## derivative means local image sensitivity

A derivative estimates how much an output changes when an input changes by a small amount. For a camera, inputs may be a 3D point, focal length, lens coefficients, or pose parameters; outputs are image coordinates. The Jacobian is the table of these partial sensitivities, one output row by one input column.

For the pinhole map $u=fX/Z$, $v=fY/Z$, use the point $P=(X,Y,Z)=(2,1,4)$ and focal length $f=100$ pixels. Its ideal image coordinate is $(u,v)=(50,25)$ pixels. Differentiating gives

$$
J_P=\begin{bmatrix}
\partial u/\partial X&\partial u/\partial Y&\partial u/\partial Z\\
\partial v/\partial X&\partial v/\partial Y&\partial v/\partial Z
\end{bmatrix}
=\begin{bmatrix}f/Z&0&-fX/Z^2\\0&f/Z&-fY/Z^2\end{bmatrix}
=\begin{bmatrix}25&0&-12.5\\0&25&-6.25\end{bmatrix}.
$$

The units are pixels per world-unit. A small displacement $\delta P=(0.01,0,0)$ predicts $\delta u\approx J_P\delta P=0.25$ pixel and $\delta v\approx0$. This is a local approximation: it is accurate for sufficiently small changes and away from singular cases such as $Z=0$. A finite-difference check compares the analytic result to rerunning the camera with a small perturbation; the step must be large enough to survive rounding and small enough to stay local.

For calibration, collect observed image points and predict each from unknown parameters $\theta$. Their difference is the residual $r(\theta)=p_{observed}-p_{predicted}(\theta)$. Near a current estimate $\theta_0$, linearize: $r(\theta_0+\Delta\theta)\approx r(\theta_0)-J\Delta\theta$. A least-squares update chooses $\Delta\theta$ to reduce the sum of squared residuals. This local model explains why initialization matters: a nonlinear camera may have multiple minima, and the linear approximation does not promise a globally correct answer.


## covariance carries both size and direction of uncertainty

A covariance matrix is a compact description of how uncertain quantities vary together. Its diagonal entries are variances (squared units); off-diagonal entries are covariances; normalizing by the corresponding standard deviations gives correlation coefficients. For a 2D pixel measurement with independent standard deviation 0.5 pixel in each axis,

$$\Sigma_p=\begin{bmatrix}0.25&0\\0&0.25\end{bmatrix}\ \text{pixel}^2.$$

If coordinates are transformed by a locally linear rule with Jacobian $J$, uncertainty transforms approximately as $\Sigma_y=J\Sigma_xJ^T$. This is the same derivative table from the previous bridge, now used to propagate error rather than move a nominal point. With the camera Jacobian above and isotropic 3D point uncertainty $\Sigma_P=0.0001I$ world-unit$^2$, the predicted pixel covariance is

$$\Sigma_p=0.0001J_PJ_P^T=\begin{bmatrix}0.078125&0.0078125\\0.0078125&0.06640625\end{bmatrix}\ \text{pixel}^2\quad\text{(rounded to four decimal places: }\begin{bmatrix}0.0781&0.0078\\0.0078&0.0664\end{bmatrix}).$$

The off-diagonal term means the uncertainty ellipse is tilted. A scalar “error radius” would hide that orientation. For parameter estimation, the residual covariance says which observations are precise and which are noisy. Weighting by its inverse gives a Mahalanobis squared error $r^T\Sigma^{-1}r$: residuals along a high-uncertainty direction count less than equally large residuals along a well-measured direction. State that covariance propagation and linearized parameter uncertainty are approximations near the current solution.

This connects V10's probability tools to V11 without implying that every residual is Gaussian. A robust loss can reduce the influence of mismatches that violate the assumed noise model; it does not make a wrong camera model correct. Report residual units and the assumed measurement variance. A pixel error, angular error, and world-distance error cannot be compared by their raw numbers.

<!-- begin V11.12 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|Next chapter]]
