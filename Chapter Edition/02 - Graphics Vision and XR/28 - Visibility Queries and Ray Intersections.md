# 28 — Visibility Queries and Ray Intersections

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries|Next chapter]]

Visibility queries carry intervals and surface rules that determine what counts as a hit. The span develops those contracts from ray intersections toward practical visibility tests.

**By the end:** Specify ray-query intervals and surface behavior for visibility.

**Recall question:** How do the query interval and hit policy change the meaning of a visibility result?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 70959–73706; source-span SHA256 ebec4d3282c57f7e7f93acf64351677cc903b54cabbc3a7ffb0850a46259e4ed. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Visibility queries and ray intersections

### Let a visibility query carry an interval and a surface contract

Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.


This pass begins **Phase 21: Ray Tracing, Monte Carlo Light Transport, Sampling, BVH Rendering, and Path Tracing**. M121 owns **Parts 2001–2013**: rays, camera/subpixel ray generation, ray intervals/self-hit avoidance, hit records, geometric versus shading normals, front/back-face handling, brute-force traversal, triangle meshes, BVHs, and any-hit versus closest-hit queries. M122 then owns **Parts 2014–2030**: recursive Whitted ray tracing, direct light, shadow/reflection/refraction rays, recursion limits, the limitations of Whitted transport, the rendering equation, its individual radiometric terms, hemisphere integration, and finally the derivation of Monte Carlo integration and probability-density functions.

The mastery cards make that boundary explicit: M121 is the **visibility/query engine**, whereas M122 turns those visibility queries into a **light-transport algorithm** and then mathematically generalizes that algorithm into the rendering equation and Monte Carlo estimation.

The conceptual transition from rasterization is enormous.

Rasterization asks:

$$
\boxed{
\text{Which framebuffer samples does this primitive cover?}
}
$$

Ray tracing reverses the question:

$$
\boxed{
\text{What surface, if any, lies along this path from this point in this direction?}
}
$$

That reversal sounds simple, but it changes the renderer's entire architecture.

---

### PARTS 2001–2013
### Rays as geometric visibility queries

---

**A ray is a parameterized line with direction**

A 3D ray is

$$
\boxed{
P(t)=O+tD
}
$$

where:

$$
O=\text{origin},
$$

$$
D=\text{direction},
$$

and usually

$$
t\ge0.
$$

For example:

$$
O=(1,2,3)
$$

and

$$
D=(0,0,-1).
$$

Then:

$$
P(0)=(1,2,3),
$$

$$
P(2)=(1,2,1),
$$

$$
P(5)=(1,2,-2).
$$

So $t$ moves us along the ray.

---

**Why normalized ray direction is useful**

If:

$$
\|D\|=1,
$$

then:

$$
\|P(t)-O\|
=
\|tD\|
=
|t|.
$$

Therefore, for forward intersections:

$$
\boxed{t=\text{world-space distance from the ray origin}.}
$$

This is extremely convenient.

If $D$ is not normalized, $t$ is still a perfectly valid parameter, but it is not physical distance.

KairoGeometry's current `Ray` explicitly documents this convention:

```cpp
P(t) = Origin + Direction * t
```

with normalized direction expected by distance/intersection APIs.

That is a good engine invariant.

---

**Rays are not merely “camera rays”**

A ray is a general geometric query.

The same mathematical object can answer:

```text
camera visibility
shadow visibility
mirror reflection
refraction
mouse picking
AI line-of-sight
collision query
audio occlusion
```

The renderer assigns the ray a transport interpretation.

Geometrically they are all:

$$
O+tD.
$$

This is one reason your separation of `KairoGeometry::Ray` from `KairoRayTracer` is architecturally good.

Ray mathematics belongs to geometry.

Radiance transport belongs to the renderer.

---

**Primary camera rays**

Rasterization starts from geometry and asks where it projects.

A ray tracer starts from a camera sample and asks what geometry lies along its viewing direction.

For each image sample:

$$
(x,y)
$$

we generate:

$$
R_{x,y}.
$$

The most basic model is a **pinhole camera**.

All primary rays originate at one camera point:

$$
O=C.
$$

Their directions differ according to where they cross an imaginary image plane.

Your KairoRayTracer currently implements exactly this: a pinhole camera with no finite aperture, focus distance or motion blur.

---

**Camera coordinate basis**

We need an orthonormal camera basis:

$$
F=\text{forward},
$$

$$
R=\text{right},
$$

$$
U=\text{up}.
$$

Given:

$$
P_c=\text{camera position},
$$

$$
T=\text{camera target},
$$

$$
U_w=\text{world-up hint},
$$

we begin with:

$$
F=
\frac{T-P_c}
{\|T-P_c\|}.
$$

For the handedness used by current KairoRayTracer:

$$
R=
\operatorname{normalize}
(
F\times U_w
).
$$

Then recompute a perfectly orthogonal up:

$$
U=
\operatorname{normalize}
(
R\times F
).
$$

Why recompute $U$?

Because the user's/world's up vector may not be exactly orthogonal to the viewing direction.

This Gram-Schmidt-like construction creates a clean camera basis.

Current `Camera::LookAt()` does exactly this.

---

**Deriving the perspective film plane**

Suppose vertical field of view is:

$$
\theta.
$$

Place an imaginary film plane one unit in front of the pinhole.

Half-height is given by basic trigonometry:

$$
\tan\frac{\theta}{2}
=
\frac{h/2}{1}.
$$

Therefore:

$$
\boxed{
h_{\frac12}
=
\tan\left(\frac{\theta}{2}\right)
}
$$

and full film-plane height is:

$$
2\tan(\theta/2).
$$

If aspect ratio is:

$$
a=\frac{W}{H},
$$

then:

$$
\boxed{
w_{\frac12}
=
a\,h_{\frac12}.
}
$$

Kairo's `GenerateRay()` uses exactly:

```cpp
filmHalfHeight =
    tan(verticalFOV * 0.5);

filmHalfWidth =
    filmHalfHeight * AspectRatio;
```

So this is not abstract textbook math—we are reconstructing the actual camera path.

---

**Pixel coordinates to normalized raster coordinates**

Suppose image dimensions are:

$$
W\times H.
$$

Pixel:

$$
(i,j).
$$

The center of that pixel is:

$$
x_s=i+\frac12,
$$

$$
y_s=j+\frac12.
$$

Normalize to:

$$
u=\frac{i+0.5}{W},
$$

$$
v=\frac{j+0.5}{H}.
$$

Thus:

$$
u,v\in(0,1).
$$

Kairo's camera expects normalized raster coordinates in:

$$
[0,1].
$$

---

**Normalized raster coordinates to film plane**

Kairo uses:

$$
x_f=(2u-1)w_{\frac12}
$$

and:

$$
y_f=(1-2v)h_{\frac12}.
$$

Why the second equation uses:

$$
1-2v
$$

rather than:

$$
2v-1?
$$

Because typical raster image Y increases downward:

```text
v = 0 → image top
v = 1 → image bottom
```

while camera $U$ points upward.

Therefore:

$$
v=0
\Rightarrow
y_f=+h_{\frac12},
$$

$$
v=1
\Rightarrow
y_f=-h_{\frac12}.
$$

---

**Film position to world ray**

A point on the virtual film plane is:

$$
Q=
C+
F+
x_fR+
y_fU.
$$

The direction from camera origin to that point is:

$$
Q-C
=
F+x_fR+y_fU.
$$

Therefore:

$$
\boxed{
D=
\operatorname{normalize}
(
F+x_fR+y_fU
)
}
$$

and:

$$
\boxed{
R(t)=C+tD.
}
$$

This is exactly your current KairoRayTracer `GenerateRay()` implementation.

---

**Camera example**

Suppose:

$$
F=(0,0,-1),
$$

$$
R=(1,0,0),
$$

$$
U=(0,1,0),
$$

aspect:

$$
a=16/9,
$$

vertical FOV:

$$
60^\circ.
$$

Then:

$$
h_{\frac12}
=
\tan30^\circ
\approx0.57735.
$$

Width:

$$
w_{\frac12}
=
0.57735\frac{16}{9}
\approx1.0264.
$$

For center screen:

$$
u=v=0.5.
$$

Then:

$$
x_f=0,
\qquad
y_f=0.
$$

Thus:

$$
D=F=(0,0,-1).
$$

Good.

For far right-center:

$$
u=1,
\quad
v=0.5,
$$

we obtain:

$$
x_f=1.0264,
$$

so:

$$
D
=
\operatorname{normalize}
(1.0264,0,-1).
$$

That is a ray through the right edge of the film.

---

**Pixel centers are only one possible sample**

If we always shoot exactly:

$$
(i+0.5,j+0.5),
$$

we perform one deterministic sample per pixel.

This can alias high-frequency edges.

A better estimate samples a point inside the pixel:

$$
x=i+\xi_x,
$$

$$
y=j+\xi_y
$$

with:

$$
\xi_x,\xi_y\in[0,1).
$$

Then:

$$
u=
\frac{i+\xi_x}{W},
\qquad
v=
\frac{j+\xi_y}{H}.
$$

This is **subpixel sampling**.

---

**Why subpixel sampling produces anti-aliasing**

A pixel is not actually a point measurement.

Ideally it stores some filtered integral over an area of the image plane.

Conceptually:

$$
C_{ij}
=
\frac{1}{A_p}
\int_{A_p}
L(x,y)
\,dx\,dy.
$$

One center ray estimates this integral using one sample.

Multiple jittered rays estimate it better:

$$
\boxed{
C_{ij}
\approx
\frac1N
\sum_{k=1}^{N}
L(x_k,y_k).
}
$$

This is already Monte Carlo integration, even before path tracing.

The randomness does not magically create accuracy.

It converts structured aliasing into stochastic estimation error that decreases as sample count increases.

We will formalize that near the end of this pass.

---

**Why one ray can still miss a tiny object**

Suppose a very thin wire crosses a pixel but not its exact center.

One center ray:

$$
\text{miss}
$$

so pixel becomes background.

But the correct pixel-filtered value should contain some wire contribution.

Multiple subpixel samples may observe it.

Thus:

$$
\boxed{
\text{ray tracing itself does not eliminate aliasing}.
}
$$

Sampling theory still applies.

---

**Rays need intervals, not merely $t\ge0$**

In practice we rarely want the entire interval:

$$
[0,\infty).
$$

We want:

$$
\boxed{
t\in[t_{\min},t_{\max}].
}
$$

Examples:

Primary ray:

$$
t_{\min}\approx\epsilon,
\quad
t_{\max}=\infty.
$$

Point-light shadow ray:

$$
t_{\min}\approx\epsilon,
\quad
t_{\max}=d_{\text{light}}-\epsilon.
$$

Closest-hit ray while traversing:

$$
t_{\max}=
\text{nearest hit found so far}.
$$

This interval is not cosmetic metadata.

It encodes the query.

---

**Why $t_{\max}$ matters for shadow rays**

Surface point:

$$
P.
$$

Point light:

$$
L_p.
$$

Distance:

$$
d=
\|L_p-P\|.
$$

Shadow ray direction:

$$
D=
\frac{L_p-P}{d}.
$$

Suppose there is an object *behind* the light.

A query extending to infinity would hit it and conclude:

```text
light blocked
```

even though the blocker lies beyond the light.

Correct interval:

$$
0<t<d.
$$

So:

$$
\boxed{
\text{shadow visibility is a finite-segment query, not an infinite-ray query}.
}
$$

Current Kairo correctly supplies:

$$
\texttt{lightDistance - bias}
$$

to `Scene::Occluded()` for point lights.

---

**Self-intersection**

Suppose primary ray intersects a surface at floating-point result:

$$
P.
$$

Mathematically $P$ lies exactly on the surface.

Now spawn a reflected ray:

$$
R(t)=P+tD_r.
$$

The intersection routine evaluates the same surface.

Due to finite floating-point error, the reconstructed point may effectively lie:

$$
10^{-7}
$$

inside the surface.

Then the outgoing ray immediately intersects its own source.

Symptoms:

```text
shadow acne
black speckles
reflection acne
self-shadowing
false internal hits
```

---

**The simplest ray bias**

Offset the ray origin:

$$
\boxed{
P'=P+\epsilon N.
}
$$

For a reflected ray going to the exterior:

$$
+\epsilon N.
$$

For a refracted ray entering the object:

$$
-\epsilon N.
$$

Current Kairo Whitted tracing does exactly this.

Reflection:

```cpp
hit.Position +
hit.Normal * bias
```

Refraction into the opposite side:

```cpp
hit.Position -
normal * bias
```

---

**Why a fixed epsilon is imperfect**

Suppose:

$$
\epsilon=10^{-4}.
$$

For a scene whose objects are around:

$$
10^{-3}\text{ units}
$$

large, this offset is huge.

For a world with coordinates around:

$$
10^8,
$$

it may be below meaningful floating-point precision.

Therefore robust ray tracing often uses scale-aware offsets.

Kairo uses:

$$
\boxed{
\epsilon=
\max(
\epsilon_{\text{absolute}},
\epsilon_{\text{relative}}\cdot t_{\text{hit}}
)
}
$$

through `RayBiasForHit()`.

That is much better than one universal constant.

---

**Ray interval versus origin offset**

These solve related but different problems.

You can reject intersections below:

$$
t_{\min}=\epsilon.
$$

That prevents tiny positive self-hits.

But physically moving the origin out of the numerical surface also helps when calculations at the new origin would otherwise start on the wrong floating-point side.

Robust production ray tracers frequently use a carefully computed origin offset **and** meaningful intervals.

---

**What must a hit record contain?**

An intersection routine's job is not merely:

```text
hit = true
```

Shading needs context.

A useful hit record may contain:

$$
t,
$$

$$
P,
$$

$$
N_g=\text{geometric normal},
$$

$$
N_s=\text{shading normal},
$$

$$
(u,v),
$$

$$
\text{barycentrics},
$$

$$
\text{primitive ID},
$$

$$
\text{material ID},
$$

$$
\text{frontFace}.
$$

Possibly also:

- tangent frame,
- derivatives,
- instance ID,
- medium/interface information.

---

**Current Kairo SurfaceHit**

Today Kairo stores:

```cpp
struct SurfaceHit
{
    bool Hit;
    float Distance;
    Vec3f Position;
    Vec3f Normal;
    Vec2f UV;
    uint32_t PrimitiveIndex;
    uint32_t MaterialIndex;
};
```

This is sufficient for the current renderer.

But it collapses:

$$
N_g
$$

and:

$$
N_s
$$

into one `Normal`.

That becomes increasingly limiting for robust path tracing.

---

**Geometric normal versus shading normal**

The **geometric normal** comes from actual geometry.

For triangle:

$$
\boxed{
N_g
=
\operatorname{normalize}
[
(B-A)\times(C-A)
].
}
$$

It defines the true local surface orientation.

The **shading normal** may be:

- interpolated vertex normal,
- normal-map perturbed,
- smoothed across triangles.

For triangle barycentrics:

$$
(\lambda_0,\lambda_1,\lambda_2),
$$

an interpolated vertex normal is:

$$
\boxed{
N_s=
\operatorname{normalize}
(
\lambda_0N_0+
\lambda_1N_1+
\lambda_2N_2
).
}
$$

These can differ significantly.

---

**Why both normals matter**

Imagine a low-poly sphere with smooth vertex normals.

The geometric surface is faceted.

The shading appearance is smooth.

For lighting:

$$
N_s
$$

is generally desirable.

But for determining:

```text
which side of the true surface did the ray hit?
```

you need:

$$
N_g.
$$

For origin offsetting, medium transitions, and robust ray transport, blindly using perturbed shading normals can send rays to the wrong physical side of geometry.

Therefore a production hit structure should usually preserve both.

---

**Current Kairo triangle normal behavior**

KairoGeometry returns the geometric triangle hit normal.

Then `KairoRayTracer::Primitive` optionally interpolates vertex normals:

$$
N_s=
\operatorname{normalize}
(
\lambda_A N_A+
\lambda_B N_B+
\lambda_C N_C
).
$$

It checks:

$$
N_s\cdot N_g<0
$$

and flips $N_s$ if necessary.

Then the hit stores this interpolated normal.

So current Kairo ensures the shading normal remains in the same hemisphere as the geometry normal, which is good.

But the geometric normal itself is no longer retained in `SurfaceHit`.

That is a future robustness improvement.

---

**Front-face versus back-face hits**

Define incoming ray travel direction:

$$
D.
$$

Geometric outward normal:

$$
N_{\text{out}}.
$$

If:

$$
D\cdot N_{\text{out}}<0,
$$

the ray hits the front/outside surface.

If:

$$
D\cdot N_{\text{out}}>0,
$$

it hits the back/inside-facing side.

A common hit-record convention is:

$$
\boxed{
\text{frontFace}
=
(D\cdot N_{\text{out}})<0
}
$$

and store a ray-opposing normal:

$$
N=
\begin{cases}
N_{\text{out}}, & \text{front face}\\
-N_{\text{out}}, & \text{back face}.
\end{cases}
$$

Then:

$$
D\cdot N\le0
$$

always.

---

**Why front/back information matters for glass**

Suppose ray enters glass.

Then:

$$
\eta_i=1,
\quad
\eta_t=1.5.
$$

Upon exiting:

$$
\eta_i=1.5,
\quad
\eta_t=1.
$$

Without knowing whether we are entering or exiting, we cannot choose the correct ratio:

$$
\eta=
\frac{\eta_i}{\eta_t}.
$$

Current Kairo Whitted glass infers this by testing:

$$
D\cdot N.
$$

If positive, it flips the normal and uses the inverse medium transition.

That works for simple isolated air/glass objects.

---

**Ray–sphere intersection derivation**

Sphere:

$$
\|P-C\|^2=r^2.
$$

Ray:

$$
P=O+tD.
$$

Substitute:

$$
\|O+tD-C\|^2=r^2.
$$

Let:

$$
L=O-C.
$$

Then:

$$
\|L+tD\|^2=r^2.
$$

Expand dot product:

$$
(L+tD)\cdot(L+tD)=r^2.
$$

Therefore:

$$
L\cdot L
+
2tL\cdot D
+
t^2D\cdot D
-
r^2
=
0.
$$

This is:

$$
\boxed{
at^2+bt+c=0
}
$$

with:

$$
a=D\cdot D,
$$

$$
b=2L\cdot D,
$$

$$
c=L\cdot L-r^2.
$$

Discriminant:

$$
\Delta=b^2-4ac.
$$

If:

$$
\Delta<0
$$

no hit.

If:

$$
\Delta=0
$$

tangent hit.

If:

$$
\Delta>0
$$

two intersections.

Roots:

$$
\boxed{
t_{0,1}
=
\frac{-b\mp\sqrt{\Delta}}{2a}.
}
$$

Choose the nearest valid root inside the ray interval.

Current `KairoGeometry::RaySphere()` follows exactly this quadratic derivation.

---

**Sphere normal**

At sphere hit point:

$$
P.
$$

Outward normal:

$$
\boxed{
N_g=
\frac{P-C}{r}.
}
$$

If $P$ lies on the sphere, this is already unit length.

That is one reason spheres are ideal early ray-tracing primitives: intersection and normal computation are analytically simple.

---

**Ray–plane intersection**

Plane can be represented by:

$$
N\cdot(P-P_0)=0.
$$

Ray:

$$
P=O+tD.
$$

Substitute:

$$
N\cdot(O+tD-P_0)=0.
$$

Expand:

$$
N\cdot(O-P_0)+tN\cdot D=0.
$$

Therefore:

$$
\boxed{
t=
\frac{
N\cdot(P_0-O)
}{
N\cdot D
}.
}
$$

If:

$$
|N\cdot D|\approx0,
$$

ray is parallel to plane.

Again, the returned $t$ must be checked against the ray interval.

---

**Ray–triangle intersection is more important than almost anything else**

Real scenes are largely triangle meshes.

If a mesh has:

$$
10^7
$$

triangles, the renderer may execute billions of ray-triangle tests.

Thus intersection math matters enormously.

The classic algorithm is **Möller–Trumbore**.

KairoGeometry's `RayTriangle()` explicitly implements it.

---

**Deriving ray–triangle intersection**

Triangle:

$$
A,B,C.
$$

Define edges:

$$
E_1=B-A,
$$

$$
E_2=C-A.
$$

Any point in the triangle can be parameterized as:

$$
P=A+uE_1+vE_2
$$

with:

$$
u\ge0,
$$

$$
v\ge0,
$$

$$
u+v\le1.
$$

Ray:

$$
P=O+tD.
$$

Set them equal:

$$
O+tD
=
A+uE_1+vE_2.
$$

Rearrange:

$$
O-A
=
uE_1+vE_2-tD.
$$

We need to solve simultaneously for:

$$
u,v,t.
$$

Möller–Trumbore solves this using cross/dot products without explicitly constructing a matrix inverse.

---

**Möller–Trumbore step by step**

Define:

$$
P_v=D\times E_2.
$$

Determinant:

$$
\boxed{
\det=
E_1\cdot P_v.
}
$$

If:

$$
|\det|\approx0,
$$

ray and triangle plane are parallel or triangle is degenerate.

Then:

$$
\det^{-1}=\frac1{\det}.
$$

Define:

$$
T=O-A.
$$

First barycentric coordinate:

$$
\boxed{
u=
(T\cdot P_v)\det^{-1}.
}
$$

Reject if:

$$
u<0
\quad\text{or}\quad
u>1.
$$

Next:

$$
Q=T\times E_1.
$$

Second barycentric coordinate:

$$
\boxed{
v=
(D\cdot Q)\det^{-1}.
}
$$

Reject if:

$$
v<0
$$

or:

$$
u+v>1.
$$

Finally ray distance:

$$
\boxed{
t=
(E_2\cdot Q)\det^{-1}.
}
$$

Check interval.

The third barycentric coordinate is:

$$
\boxed{
w=1-u-v.
}
$$

---

**Why the barycentric conditions define the triangle**

Recall:

$$
P=
wA+uB+vC
$$

with:

$$
w+u+v=1.
$$

Points inside the triangle satisfy:

$$
w,u,v\ge0.
$$

Since:

$$
w=1-u-v,
$$

the conditions become:

$$
u\ge0,
$$

$$
v\ge0,
$$

$$
u+v\le1.
$$

Therefore Möller–Trumbore solves intersection and inside-triangle classification simultaneously.

---

**Complete commented triangle intersection**

```cpp
struct Ray
{
    Vec3 origin;
    Vec3 direction;

    // Explicit query interval.
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::infinity();
};

struct TriangleHit
{
    float t;

    // Barycentrics corresponding to A, B, C.
    float w;
    float u;
    float v;
};

std::optional<TriangleHit>
IntersectTriangle(
    const Ray& ray,
    Vec3 A,
    Vec3 B,
    Vec3 C)
{
    constexpr float epsilon = 1.0e-8f;

    const Vec3 E1 = B - A;
    const Vec3 E2 = C - A;

    // pvec is perpendicular to both ray direction and E2.
    const Vec3 pvec =
        Cross(ray.direction, E2);

    const float determinant =
        Dot(E1, pvec);

    // Near-zero determinant means:
    // - triangle is degenerate, or
    // - ray is parallel to triangle plane.
    if (std::abs(determinant) < epsilon)
        return std::nullopt;

    const float inverseDet =
        1.0f / determinant;

    const Vec3 tvec =
        ray.origin - A;

    const float u =
        Dot(tvec, pvec) *
        inverseDet;

    if (u < 0.0f || u > 1.0f)
        return std::nullopt;

    const Vec3 qvec =
        Cross(tvec, E1);

    const float v =
        Dot(ray.direction, qvec) *
        inverseDet;

    if (v < 0.0f || u + v > 1.0f)
        return std::nullopt;

    const float t =
        Dot(E2, qvec) *
        inverseDet;

    if (t < ray.tMin ||
        t > ray.tMax)
    {
        return std::nullopt;
    }

    return TriangleHit
    {
        .t = t,
        .w = 1.0f - u - v,
        .u = u,
        .v = v
    };
}
```

This gives us both geometry and interpolation coordinates.

---

**Interpolating mesh data at a ray hit**

Once barycentrics are:

$$
\lambda_A,\lambda_B,\lambda_C,
$$

UV:

$$
\boxed{
UV=
\lambda_AUV_A+
\lambda_BUV_B+
\lambda_CUV_C.
}
$$

Shading normal:

$$
\boxed{
N_s=
\operatorname{normalize}
(
\lambda_AN_A+
\lambda_BN_B+
\lambda_CN_C
).
}
$$

Vertex color similarly.

This is the exact same barycentric interpolation concept used during rasterization.

That is a beautiful connection:

$$
\boxed{
\text{rasterizer computes barycentrics from screen-space coverage}
}
$$

while:

$$
\boxed{
\text{ray tracer computes barycentrics from 3D ray-triangle intersection}.
}
$$

Once barycentrics exist, attribute interpolation is conceptually similar.

---

**Kairo's triangle path**

Current KairoGeometry performs Möller–Trumbore.

Then `KairoRayTracer::Primitive` recomputes barycentrics from the hit position and uses them to interpolate:

$$
N,
$$

$$
UV.
$$

If no UVs exist, it uses:

$$
(\lambda_B,\lambda_C)
$$

as fallback UV-like coordinates.

The separation is:

```text
KairoGeometry:
geometry-only hit

KairoRayTracer:
rendering attributes + material identity
```

That is a clean library boundary.

---

**Brute-force scene traversal**

Suppose there are:

$$
N
$$

primitives.

The simplest query is:

```cpp
closest = none;

for each primitive:
    hit = Intersect(ray, primitive);

    if hit closer than current:
        closest = hit;
```

Complexity:

$$
\boxed{O(N)\text{ primitive tests per ray}.}
$$

If:

$$
N=10^6
$$

and image uses:

$$
1920\times1080
\approx2.07\times10^6
$$

primary rays, then brute force may require approximately:

$$
2.07\times10^{12}
$$

primitive tests **before shadow rays or bounces**.

Impossible for practical rendering.

Acceleration structures are therefore not optional optimization.

They are fundamental to ray tracing.

---

**Brute force is still incredibly valuable**

Kairo keeps `BruteForceIntersect()`.

That is a good decision.

Why?

Because brute force is simple enough to be a **correctness oracle**.

If BVH and brute force disagree:

$$
\boxed{
\text{the acceleration structure is wrong, not the underlying geometry query}.
}
$$

Current comments explicitly state this purpose.

This is excellent engineering.

---

**Bounding volumes**

Instead of testing every triangle, group primitives inside a cheap bounding shape.

If the ray misses the bound:

$$
\boxed{
\text{it cannot hit any contained primitive}.
}
$$

AABB = Axis-Aligned Bounding Box.

Represent:

$$
B_{\min}
=
(x_{\min},y_{\min},z_{\min}),
$$

$$
B_{\max}
=
(x_{\max},y_{\max},z_{\max}).
$$

AABB tests are much cheaper than testing dozens/hundreds of triangles.

---

**AABB slab interpretation**

An AABB is intersection of three coordinate slabs.

X slab:

$$
x_{\min}\le x\le x_{\max}.
$$

Ray:

$$
x(t)=O_x+tD_x.
$$

Solve entry/exit:

$$
t_{x0}
=
\frac{x_{\min}-O_x}{D_x},
$$

$$
t_{x1}
=
\frac{x_{\max}-O_x}{D_x}.
$$

Order them:

$$
t_x^-=\min(t_{x0},t_{x1}),
$$

$$
t_x^+=\max(t_{x0},t_{x1}).
$$

Do the same for Y and Z.

The ray lies inside the whole box when it lies in **all three slabs simultaneously**.

Thus:

$$
\boxed{
t_{\text{enter}}
=
\max(
t_x^-,
t_y^-,
t_z^-
)
}
$$

and:

$$
\boxed{
t_{\text{exit}}
=
\min(
t_x^+,
t_y^+,
t_z^+
).
}
$$

Hit exists if:

$$
\boxed{
t_{\text{enter}}
\le
t_{\text{exit}}
}
$$

and intervals overlap the ray query range.

KairoGeometry's `RayAABB()` uses this slab principle.

---

**Parallel-axis special case**

If:

$$
D_x\approx0,
$$

then division:

$$
1/D_x
$$

becomes problematic.

Geometrically the ray never changes X.

Therefore:

If:

$$
O_x<x_{\min}
\quad\text{or}\quad
O_x>x_{\max},
$$

there is no hit.

Otherwise the ray is inside the X slab for all $t$, so X imposes no restriction.

Kairo's implementation explicitly handles this.

---

**The BVH**

BVH means:

$$
\boxed{\text{Bounding Volume Hierarchy}.}
$$

Conceptually:

```text
root bounds entire scene
├── left bounds subset
│   ├── ...
│   └── ...
└── right bounds subset
    ├── ...
    └── ...
```

Internal node:

$$
\text{contains child bounds}.
$$

Leaf:

$$
\text{contains actual primitives}.
$$

Traversal:

```text
test node AABB
if miss:
    reject entire subtree

if leaf:
    test primitives

otherwise:
    visit children
```

A good BVH dramatically reduces the number of expensive primitive intersections.

---

**KairoSpatial BVH node**

Current node stores approximately:

```cpp
struct BVHNode
{
    AABB Bounds;

    Index Left;
    Index Right;

    Index FirstPrimitive;
    uint32_t PrimitiveCount;

    Index Parent;
};
```

Leaf test:

```cpp
PrimitiveCount > 0
```

Current build settings include:

$$
\text{MaxLeafSize}=4,
$$

$$
\text{MaxDepth}=64.
$$

That is a very typical compact binary BVH representation.

---

**Building a simple median BVH**

For a primitive subset:

1. compute node bounds;
2. if few primitives remain, make leaf;
3. compute centroid bounds;
4. choose largest centroid extent axis;
5. sort primitives by centroid on that axis;
6. split at median;
7. recurse.

The user requested no shallow listicles, so interpret that as one recursive invariant:

$$
\boxed{
\text{each recursive call partitions a primitive set into two spatially more coherent subsets until leaf criteria are reached}.
}
$$

The median split gives approximately balanced primitive counts.

That tends to keep tree depth around:

$$
O(\log N).
$$

But balanced primitive count does not necessarily mean good ray-traversal cost.

---

**Surface Area Heuristic**

A better split estimates how likely rays are to enter each child.

For AABBs, surface area:

$$
A=
2(
dx\,dy+
dy\,dz+
dz\,dx
).
$$

A classical normalized SAH form is approximately:

$$
C=
C_T
+
\frac{A_L}{A_P}N_LC_I
+
\frac{A_R}{A_P}N_RC_I.
$$

where:

$$
C_T=\text{node traversal cost},
$$

$$
C_I=\text{primitive intersection cost},
$$

$$
A_P=\text{parent area},
$$

$$
A_L,A_R=\text{child areas}.
$$

Current KairoSpatial uses a simplified proportional cost:

$$
\boxed{
C=
A_LN_L+
A_RN_R
}
$$

for candidate splits and compares against leaf cost:

$$
C_{\text{leaf}}
=
A_PN.
$$

That captures the same core intuition:

$$
\boxed{
\text{small spatial bounds containing few primitives are desirable}.
}
$$

---

**Why surface area predicts traversal probability**

Imagine rays arriving with roughly broad/random directional distribution.

A larger box is geometrically more likely to be intersected.

Thus surface area acts as a useful approximation for visitation probability.

The exact assumptions behind SAH are idealized, but empirically it is extremely effective.

This is one of the foundational ideas in production ray-tracing acceleration.

---

**Morton ordering**

KairoSpatial also supports Morton ordering.

Normalize primitive centroids into:

$$
[0,1)^3.
$$

Quantize coordinates.

Interleave bits:

```text
x bits: x2 x1 x0
y bits: y2 y1 y0
z bits: z2 z1 z0

Morton:
x2 y2 z2 x1 y1 z1 x0 y0 z0
```

Nearby points often receive nearby Morton codes.

Thus sorting by Morton code gives a cheap spatial ordering.

This is particularly attractive for fast BVH construction and GPU-friendly algorithms.

Current KairoSpatial uses 10-bit-per-axis quantization for its Morton key generation.

---

**Closest-hit traversal**

For a primary ray, we need the first visible surface.

Maintain:

$$
t_{\text{closest}}.
$$

Initially:

$$
\infty.
$$

Whenever a primitive hit is found at:

$$
t<t_{\text{closest}},
$$

update:

$$
t_{\text{closest}}=t.
$$

Now any BVH node whose nearest possible AABB entry is farther than:

$$
t_{\text{closest}}
$$

can be rejected.

This is powerful pruning.

---

**Near-child-first traversal**

Suppose both child AABBs intersect.

Left begins at:

$$
t_L=3.
$$

Right begins at:

$$
t_R=100.
$$

Visit left first.

If it produces a primitive hit at:

$$
t=5,
$$

the right child starting at 100 can immediately be discarded.

Thus traversal order affects performance even though it must not affect final correctness.

Current KairoSpatial `Raycast()` computes both child-bound distances and pushes the farther child first onto the stack, so the nearer child is popped/visited first.

That is exactly the right principle.

---

**Any-hit traversal**

For shadows we only need:

$$
\boxed{
\exists\text{ blocker?}
}
$$

We do **not** need the nearest blocker.

Therefore the algorithm can immediately return once any valid primitive intersects before the light.

Pseudo-logic:

```cpp
if (primitiveHit)
    return true;
```

No sorting.

No closest-hit update.

No need to inspect the rest of the tree.

Current KairoSpatial has a dedicated:

```cpp
RaycastAny(...)
```

that does precisely this.

And KairoRayTracer `Scene::Occluded()` uses it.

This is Part 2012 directly realized in your repository.

---

**Closest-hit versus any-hit**

These are semantically different query types.

Closest hit:

$$
\boxed{
\arg\min_{i}
t_i.
}
$$

Any hit:

$$
\boxed{
\exists i:t_i\in[t_{\min},t_{\max}]?
}
$$

For primary visibility:

$$
\text{closest hit}.
$$

For shadow visibility:

$$
\text{any hit}.
$$

For picking:

typically closest hit.

For certain collision/visibility queries:

possibly any hit.

This distinction is one of the simplest but most important acceleration optimizations.

---

**Kairo M121 architecture**

The current chain is approximately:

```text
Camera::GenerateRay
        ↓
Rayf
        ↓
Scene::Intersect
        ↓
brute force
   OR
KairoSpatial::Raycast
        ↓
BVH AABB traversal
        ↓
callback into KairoRayTracer::Intersect
        ↓
KairoGeometry::RaySphere / RayTriangle
        ↓
SurfaceHit
        ↓
renderer shading
```

For shadows:

```text
surface
   ↓
shadow ray
   ↓
Scene::Occluded
   ↓
KairoSpatial::RaycastAny
   ↓
stop on first valid blocker
```

This is a very clean separation of responsibilities.

---

### PARTS 2014–2030
### Whitted transport → rendering equation → Monte Carlo

---


Visibility is a geometric query: rays and surface intersections determine what a viewpoint can see.

### Query contracts

A geometric query should specify its input domain and exact result. A ray query might ask for any intersection, the closest positive intersection, or all intersections in order. A point-surface query might return the closest point, distance, face index, and barycentric coordinates. An inside-outside query requires a closed oriented surface or a defined winding convention. Different queries can reuse the same primitive tests but have different early-exit rules. A shadow ray only needs to know whether a blocker occurs before the light; a camera ray needs the closest hit; a translucent ray may need multiple ordered hits.

For a closest-point-on-triangle query, classify the point relative to each Voronoi region of the triangle: face interior, one of three edge regions, or one of three vertex regions. If the perpendicular projection lands inside the triangle, return it. If outside, clamp to the nearest edge segment or vertex. The output includes barycentric coordinates and distance. Those coordinates let the caller interpolate material or geometric attributes at the closest point. A degenerate triangle has no well-defined face interior, so the query must fall back to its edges or endpoints. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]]

An implicit-surface ray query solves $F(o+td)=0$. If $F$ is a polynomial, this can produce a polynomial equation in $t$. A sphere gives a quadratic; more complicated surfaces may require root finding. Newton iteration uses local derivatives and can converge rapidly near a root, but it may fail when the initial guess is poor or derivative is small. Bracketing methods maintain an interval known to contain a root and are more robust but may converge more slowly. The lecture’s geometric-query motivation includes the instability of intersecting nearly parallel lines: a tiny input perturbation can move the computed intersection a large distance. Robustness must account for conditioning, not just floating-point precision. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-12 - Geometric Queries|CMU-12]]

### A ray answers the visibility question

A camera ray is written $r(t)=o+td$, with origin $o$, direction $d$, and $t\ge0$. Given a camera sample, the ray asks which surface is encountered first. For a sphere with center $c$ and radius $R$, substituting o+td into $\|r(t)-c\|^2=R^2$ gives a quadratic. Let m=o-c. Then

$$
(d\cdot d)t^2+2(d\cdot m)t+(m\cdot m-R^2)=0.
$$

A negative discriminant means no hit. Two positive roots mean the ray enters and exits the sphere; the smaller positive root is the visible surface. If d is normalized, d·d=1. The ray-casting lecture builds rendering from this “cast one ray and find the nearest intersection” operation [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:10:00–00:20:00; , 00:05:00–00:20:00).

For a numerical trace, set o=(0,0,0), d=(0,0,1), c=(0,0,5), and R=1. Then m=o-c=(0,0,-5), so the quadratic becomes t²-10t+24=0. Its roots are t=4 and t=6. The first root is the sphere's front surface; the second is its back surface. The hit normal is (r(4)-c)/R=(0,0,-1). This one calculation produces both visibility depth and a geometric normal for later shading.

For a triangle, barycentric coordinates express a point as $p=\alpha a+\beta b+\gamma c$, with $\alpha+\beta+\gamma=1$. The point lies inside the triangle when all three weights are nonnegative. If a ray hit is expressed in barycentric coordinates, those same weights interpolate per-vertex attributes such as color, normal, or texture coordinates. This is a powerful bridge from geometry to shading: the hit test returns not just a location but a location's coordinates in the primitive. The second casting lecture uses barycentric coordinates and moves among triangle primitives and constructive solid geometry (CSG) [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:15:00–00:45:00).

CSG builds a solid from simpler solids with union, intersection, and difference. For ray rendering, each primitive contributes an ordered set of entry/exit intervals; Boolean operations combine those intervals. For example, a drilled hole can be a solid cylinder subtracted from a box. The resulting boundary can be rendered without first remeshing the Boolean result. CSG is easy to describe compositionally but needs careful treatment of coincident surfaces, inside/outside state, and numerical tolerances. The lecture's ray-casting discussion uses CSG to extend primitive intersection beyond spheres and triangles [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:20:00–00:50:00).

Ray casting gives direct visibility and local shading. Ray tracing extends the question recursively: after the camera ray hits, where does mirror-reflected or transmitted light come from? A reflected direction is $r=d-2d\cdot nn$ for normalized incident direction (d) and normal (n). Refraction uses Snell's law. If the refractive indices are n_1,n_2, then $n_1\sin\theta_1=n_2\sin\theta_2$; some incidence angles cause total internal reflection instead of a transmitted ray. A ray tree branches at reflective and refractive surfaces, and recursion depth or energy thresholds limit work. This captures mirror and glass effects, but does not by itself solve all indirect lighting: a diffuse bounce can send energy toward many directions. The tracing lectures derive secondary rays, reflection/refraction behavior, and recursive ray trees [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:05:00–00:20:00; 00:40:00–01:00:00; , 00:25:00–00:45:00).

An acceleration structure reduces the number of expensive primitive tests. A bounding volume hierarchy (BVH) encloses groups of objects in boxes; if a ray misses a box, it can skip every primitive below it. A k-d tree recursively partitions space. Both trade construction/storage work for faster repeated queries. For (N) triangles, a brute-force ray tests all (N); an effective hierarchy often visits a much smaller subset, though worst-case behavior can still approach (N). The second ray-tracing lecture discusses axis-aligned bounding boxes and traversal as an algorithmic cost problem [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-12 - Ray tracing II|MITG-12]] (00:30:00–00:55:00).

### From the camera to a surface: the ray is a query, not a line segment

Ray casting reverses the usual light path. Light travels from sources, bounces from surfaces, then reaches the eye; a renderer instead starts at each image sample and asks which scene surface can send light back to that sample. A pinhole camera places one origin at the eye and sends different directions through a virtual image plane. With a camera origin $e$, orthonormal view basis $(u,v,w)$, image-plane coordinates $(s_x,s_y)$, and focal distance $f$, a simple perspective direction is $d=s_xu+s_yv+fw$. The ray is $r(t)=e+td$, with $t\geq 0$. A larger field of view widens the ray pyramid; image aspect ratio scales its horizontal and vertical extents. Normalized image coordinates commonly map the rectangle to $[-1,1]^2$. The finite pyramid-shaped visibility region is the [[Camera view and clip-space pipeline|view frustum]]; rasterizers clip against its near and far planes, and interactive renderers can reject objects wholly outside it. An orthographic camera instead launches parallel directions from different points on a rectangle, avoiding perspective foreshortening. These are design choices in a renderer: fisheye rays and even multi-perspective images made by taking different columns from viewpoints orbiting an elephant are legitimate camera maps even though they do not model a conventional lens. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] (00:35:00–00:45:00)

The ray direction need not be unit length. Its parameter $t$ is a distance only when $\|d\|=1$; in general it is the scalar that places the hit on the parameterized ray. This distinction matters after transformations and for comparing distances. For one ray, every object can report a candidate $t$; visibility chooses the least candidate satisfying a positive threshold and any finite-light endpoint bound. A surface behind the camera has $t<0$ and is rejected. With a light at $L$, a shadow query is a segment query: if the point is $p$, set $d_L=L-p$ and only accept intersections with $0<t<1$ in $p+td_L$. This avoids mistaking geometry beyond the light for an occluder. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (00:45:00–00:56:00; , 00:08:00–00:20:20)

An implicit equation describes membership by a scalar function. For a plane with normal $n$ and offset $d$, $h(x)=n\cdot x+d$; the plane is $h(x)=0$. When $n$ is unit length, $h(x)$ is signed distance: positive and negative signs identify opposite sides. Substituting the ray gives $n\cdot(o+td)+d=0$, so

$$
t=-\frac{n\cdot o+d}{n\cdot d}.
$$

The denominator has geometric meaning. If $n\cdot d=0$, the ray direction is parallel to the plane; there is no isolated crossing (the ray may be disjoint or lie in the plane). A robust implementation compares $|n\cdot d|$ with a scale-aware tolerance, then separately handles the coincident case if the application needs it. Even if the measure of exactly parallel rays is zero in an ideal continuum, floating-point coordinates, shared edges, and deliberately aligned cameras make boundary configurations routine in graphics. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] (00:47:00–00:56:00)

For a sphere of center $c$ and radius $R$, membership is $h(x)=\|x-c\|^2-R^2=0$. Put $m=o-c$ and substitute $o+td$:

$$
(d\cdot d)t^2+2(d\cdot m)t+(m\cdot m-R^2)=0.
$$

Call the coefficients $A=d\cdot d$, $B=2d\cdot m$, and $C=m\cdot m-R^2$. The discriminant $\Delta=B^2-4AC$ classifies the geometry: negative means the line misses, zero means it is tangent, positive means two line crossings. The ray still needs a root policy. If $o=(0,0,0)$, $d=(0,0,1)$, $c=(0,0,5)$ and $R=1$, then $A=1$, $B=-10$, $C=24$, and $\Delta=4$. The roots are $4$ and $6$; the first is the front surface and the second the exit surface. If the ray begins inside the sphere, one root is negative and the positive exit root is the first visible crossing. Never simply hard-code the minus branch: root order depends on the sign of $B$ and on where the origin lies. For a sphere centered at $c$, the outward normal at hit $q$ is $(q-c)/R$; normalize it if the intersection routine or later transform changed its length. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] (00:56:00–01:09:20)

This calculation exposes brute-force cost. With $P$ pixels and $N$ primitives, one camera sample per pixel and a full primitive scan costs approximately $PN$ intersection tests before shading. The geometry can be represented implicitly (sphere, plane, algebraic surface) or as triangles; the common interface can simply return hit status, $t$, point, normal, and surface metadata. A ray tracer can therefore support a new primitive by implementing its query, without changing camera or shading code. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-09 - Ray casting|MITG-09]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:10:00–00:20:00; 01:09:50–01:11:35; , 00:42:00–00:45:10)

### A triangle hit has coordinates inside the triangle

A triangle is finite, so intersecting its supporting plane is only half the test. Let its non-collinear vertices be $a,b,c$. Any point $p$ on their plane has affine or barycentric coordinates

$$
$p=\alpha a+\beta b+\gamma c$,\qquad \alpha+\beta+\gamma=1.
$$

The sum-to-one constraint makes this an affine combination; it leaves two degrees of freedom, exactly the dimension of a plane. If each weight is nonnegative, $p$ is inside the triangle. A negative weight means the point lies across the line opposite that vertex. At the corners the triples are $(1,0,0)$, $(0,1,0)$, and $(0,0,1)$. On the edge from $a$ to $b$, $\gamma=0$. These tests make barycentrics useful beyond ray hits: they are local coordinates for every point of a triangle. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:07:00–00:18:10)

The area interpretation gives intuition and a useful check. If $A$ is the triangle area and $A_a$ is the area of the subtriangle opposite $a$ made by joining $p$ to the other two vertices, then $\alpha=A_a/A$; similarly for $\beta$ and $\gamma$. At vertex $a$, the opposite-$a$ subtriangle is the whole triangle, so the weight is one, and the other two areas vanish. For a numerical example, choose $a=(0,0)$, $b=(4,0)$, $c=(0,2)$ and $p=(1,0.5)$. Then $p=\alpha a+\beta b+\gamma c=(4\beta,2\gamma)$. Thus $\beta=1/4$, $\gamma=1/4$, and $\alpha=1/2$. The weights sum to one and are nonnegative, so the point is inside. These weights say that half of the location is anchored at $a$, one quarter at $b$, and one quarter at $c$. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:19:00–00:22:00)

To compute the weights algebraically, set $e_1=b-a$, $e_2=c-a$, and $q=p-a$. Then $q=\beta e_1+\gamma e_2$. Taking dot products with both edges creates the $2\times2$ Gram system

$$
\begin{bmatrix}e_1\cdot e_1&e_1\cdot e_2\\e_1\cdot e_2&e_2\cdot e_2\end{bmatrix}
\begin{bmatrix}\beta\\\gamma\end{bmatrix}=
\begin{bmatrix}e_1\cdot q\\e_2\cdot q\end{bmatrix},\qquad \alpha=1-\beta-\gamma.
$$

The matrix is invertible for a non-degenerate triangle. For a ray, equate the two descriptions of the unknown point:

$$
o+td=a+\beta(b-a)+\gamma(c-a).
$$

This is three scalar equations for $t,\beta,\gamma$. Solve the $3\times3$ linear system (a pivoted solver is safer than writing an explicit inverse), reject singular or nearly singular systems, then require $t$ to be in front of the origin and all weights to be nonnegative within a small, scale-aware edge tolerance. The system becomes singular when the ray is parallel to the triangle plane; a near-singular system corresponds to a nearly parallel, numerically sensitive case. A standard-library Python helper for the exact interior test can be written once the linear solver supplies weights:

```python
def accepts_triangle_hit(t, alpha, beta, gamma, eps=1e-9):
    return t > eps and min(alpha, beta, gamma) >= -eps
```

An actual robust renderer must scale the tolerance to coordinate and triangle size, and use a consistent edge-ownership rule so two adjacent triangles neither leave a gap nor both claim a boundary sample. The lecture recounts how rounding makes this shared-edge requirement unexpectedly difficult. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:22:00–00:36:20; 00:16:40–00:18:10; 00:38:20–00:39:10)

Barycentric coordinates also transport attributes from vertices to the hit. If vertex colors are $C_a,C_b,C_c$, then $C(p)=\alpha C_a+\beta C_b+\gamma C_c$. The same formula can interpolate texture coordinates, material parameters, or normals. Interpolated normals are generally renormalized before lighting. A flat geometric triangle has one constant face normal, while interpolated vertex normals imitate a smoothly varying surface and suppress the faceted appearance of a coarse mesh. The material can vary across a face as well; the interpolated values describe a continuous piecewise-linear field over the mesh. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:36:20–00:42:00)

### Boolean solids can be intersected without building their boundary mesh

Constructive solid geometry (CSG) represents a solid by a tree of primitive solids joined with union $A\cup B$, intersection $A\cap B$, or difference $A\setminus B$. A cube with a cylindrical hole, for instance, is the cube minus the cylinder. A ray query can evaluate this shape without explicitly constructing the complicated Boolean boundary. Instead, each object returns the parameter intervals on which the ray is inside it; a CSG node applies ordinary one-dimensional set operations to its children's intervals. If $A$ occupies $[1,3]$ and $B$ occupies $[2,6]$, then $A\setminus B=[1,2)$, $A\cup B=[1,6]$, and $A\cap B=[2,3]$. Keep endpoints and entry/exit surface identities so the final boundary can supply the correct normal and material; subtraction reverses the normal on the cut surface. If an object has several disconnected components, it returns a set of intervals rather than one pair. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:45:00–01:07:30)

This is a rendering representation, not automatically a manufacturing representation. A ray tracer only needs to answer queries; a 3D printer, milling tool, or downstream mesh algorithm may need an explicit boundary or watertight volume. That conversion can require a separate geometry engine. CSG nodes compose cleanly because each asks its children for intervals, applies a Boolean operation, and returns new intervals; recursion through the tree follows naturally from the object interface. Coincident boundaries, tangent rays, and transparent participating media need extra policies because membership alone does not determine how volume absorption or scattering should combine. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-10 - Ray casting II|MITG-10]] (00:53:00–01:06:20)

### Shadows are visibility from a second viewpoint

A shadow map renders the scene from the light's point of view and stores the nearest depth at each light-space sample. During camera rendering, a surface point is transformed into light space and compared against the stored depth. If it lies farther away than the stored nearest surface, it is occluded from the light and is shadowed. The method reuses rasterization and the depth buffer to answer a new visibility query. The shadow-map lecture explains this two-viewpoint idea and compares shadow-map variants and filtering [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (00:15:00–00:35:00; 00:45:00–01:10:00).

Depth comparisons are sensitive to finite precision and sampling. A tiny mismatch can make a surface incorrectly shadow itself (shadow acne); a depth bias helps but too much bias detaches the shadow (Peter Panning). A point light needs multiple directions or a cube map, while a directional light can use a simpler orthographic projection. Hard shadow maps produce abrupt edges; filtering estimates a softer fraction of blocked samples but can blur or leak at boundaries. The important conceptual separation is that “is this point lit?” is another visibility question, now evaluated from the light rather than the camera.


### A line corridor can be a cheap prefilter for shadow-caster work

A culling test is useful only after we name the volume it tests. Frustum culling asks whether a bound intersects the viewing volume for a particular pass. Occlusion culling asks whether other surfaces hide it. A point-to-line distance asks a different, narrower question: is a bounded object close enough to a chosen line that it might matter to a line-centered effect? It can be a cheap candidate filter, but it cannot replace the pass's actual clipping or visibility rules.

A Kohi performance stream explores such a test while discussing cascaded shadow work. The presenter describes a line by a start point and a direction vector, then reasons about whether a mesh's center lies within a chosen radius of it. At 01:48:08 the idea is introduced as a distance-to-line check; around 01:50:04 the presenter looks up the cross-product formula and notes that the available representation has a direction, not finite endpoints. At 01:57:36 he states the construction in terms of the point minus line start, crossed with the direction, divided by the direction length. This is live narration and code editing, so the teaching below reconstructs the geometry and does not claim to reproduce a verified implementation. [Kohi Ep. 161, 01:48:08–01:57:36](https://www.youtube.com/watch?v=dJYWL9YAelY&t=6488s)

Let $a$ be a point on an infinite line, let $d$ be its nonzero direction, and let $p$ be the query point. The perpendicular distance is

$$
\operatorname{dist}(p,\ell)=\frac{\|(p-a)\times d\|}{\|d\|}.
$$

The cross-product magnitude is the area of a parallelogram whose base has length $\|d\|$ and whose height is the perpendicular distance. Dividing by the base leaves the height. This works even if $d$ is not normalized; a zero direction is invalid and needs an explicit policy.

A small reference implementation makes the operation order explicit. Inputs are ordinary finite-scale coordinates with representable intermediate products; the extreme-magnitude policy is discussed below. This reference is used in the saved numerical checks, and is not a native engine implementation.

```python
import math

def length(v):
    return math.hypot(*v)

def cross(a, b):
    return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2],
            a[0]*b[1]-a[1]*b[0])

def line_distance(p, origin, direction):
    magnitude = length(direction)
    if magnitude == 0:
        raise ValueError("zero direction")
    return length(cross(tuple(x-y for x,y in zip(p,origin)), direction))/magnitude
```

For a numerical check, take $a=(0,0,0)$, $d=(0,0,4)$ and $p=(3,0,2)$. Then $(p-a)\times d=(3,0,2)\times(0,0,4)=(0,-12,0)$, whose length is 12. Since $\|d\|=4$, the point is 3 units from the line. If an object is represented by a sphere of radius $r_s=0.5$ and the effect accepts a corridor radius $r_c=0.4$, this object is safely outside because $3>r_s+r_c=0.9$. A sphere centered at $p=(0.6,0,2)$ is a candidate because $0.6\leq0.9$. The second result means “keep for a more exact test,” not “definitely contributes.”

Before rejecting a shadow caster, prove that this corridor encloses the relevant shadow region. For directional light, imagine every possible receiver point lies within radius $r_c$ of a line parallel to the light rays. Extending a receiver along that light direction does not change its distance from the line. Therefore a point that can cast onto that receiver is also inside the same infinite corridor. A caster sphere wholly outside the corridor can then be rejected. Without that enclosure argument, an arbitrary camera line and guessed radius can discard a caster whose shadow reaches the scene. A light with diverging rays needs a different bound. The tutorial's line construction does not by itself prove this enclosure for every cascade.

The sphere makes the point test conservative for an entire mesh: any point of a sphere lies at most $r_s$ from its center, so if the center is farther than $r_s+r_c$, the sphere cannot reach the corridor. A false positive costs later work but keeps a possible contributor. A false negative can remove real work from the image. That asymmetry is why the radius must enclose the transformed geometry. The stream first considers extents, then notices that scaling and transforms change the bound and leaves caching/recalculation as a question (02:00:42–02:06:41). A robust implementation can transform a local box's corners and bound those world-space points, or use a conservative affine-transform bound; simply measuring untransformed local extents is insufficient under scale. The source's evolving calculation should not be read as proof that its final radius is conservative.

For a reusable transformed bound, write a world point as $x_w=A x_l+t$, where $A$ is the three-by-three linear part and $t$ is translation. A local sphere of center $c_l$ and radius $r_l$ maps to an ellipsoid in general. Its center is $A c_l+t$. A safe enclosing sphere can use

$$
r_w=\|A\|_F r_l,\qquad
\|A\|_F=\sqrt{\sum_{i=1}^3\sum_{j=1}^3 A_{ij}^2}.
$$

The subscript F names the [[Concepts/Matrix norms and numerical error|Frobenius norm]]: square every matrix entry, add, then take the square root. The Cauchy–Schwarz inequality says $|v\cdot u|\leq\|v\|\|u\|$: a dot product cannot exceed the product of the two vector lengths in magnitude. Applying it to each matrix row gives $\|Au\|^2\leq\|A\|_F^2\|u\|^2$, which proves the enclosure. It may be loose. For a rotation followed by independent axis scales, the largest absolute scale gives the tighter radius multiplier. Translation moves the center and does not enlarge the radius.

Do not extend that largest-scale shortcut to an arbitrary sheared matrix by taking only its longest column. For $A=\begin{bmatrix}1&1&0\\0&1&0\\0&0&1\end{bmatrix}$, the longest column has length $\sqrt2\approx1.414$, but the maximum length stretch is approximately $1.618$. A local unit sphere can therefore escape that guessed world sphere. The Frobenius multiplier is 2 and safely encloses it. Recompute bounds for animation, morphing or shader displacement when those change the geometry; a correct static transform does not cover an independently moving surface. This bound and counterexample are original mathematical development, not a claim that the episode implemented them.

An infinite line also has no near/far endpoint. If the target is a finite segment from $a$ to $b$, compute $t=((p-a)\cdot(b-a))/\|b-a\|^2$, clamp $t$ to $[0,1]$, and measure to $a+t(b-a)$. For a ray, clamp only below zero. If the target is instead a shadow cascade's frustum, use that cascade's actual planes or a conservative bound for the cascade. Distance to a line alone says nothing about depth range, side planes, occlusion, or whether a caster can project into the receiver region. The stream's dialogue moves among “line,” radius, and cascade clipping; keep these as distinct geometric contracts.

#### Compute pass inputs before gathering objects

The same session exposes a separate reliability lesson. The geometry-gathering loop tries to use a cascade center and radius before the loop has reached the iteration that computes the farthest cascade data. At 02:33:03 the presenter notices that the center has not been set; around 02:34:20 he identifies that the required last-cascade value is not available on the first iteration. At 02:35:58 he proposes splitting the work: first calculate cascade data, then run another pass to gather rendered geometry. [Kohi Ep. 161, 02:33:03–02:35:58](https://www.youtube.com/watch?v=dJYWL9YAelY&t=9183s)

This is a producer-consumer dependency. The culling query consumes a completed center, direction, and radius; therefore the cascade setup must produce all of them before object gathering begins. The safe structure is conceptually:

```text
for each cascade:
    compute split, projection, center, and bounds

for each cascade:
    for each candidate object:
        test the completed bounds
        append possible contributors
```

The extra traversal is not automatically slower than a tangled single loop. It can avoid reading partially initialized state and makes the dependency visible. If later profiling shows the second traversal is expensive, the design can be reorganized while preserving the invariant that every query sees fully constructed cascade data. This is a general scheduling rule for render preparation: establish complete pass inputs, held unchanged while they are queried, before parallel or batched consumers inspect them.

The presenter later observes fewer geometries in the shadow pass while moving and turning the camera, calling the result an optimization (02:49:34). A later timing overlay is described as roughly two milliseconds, but the session has uncertain capture/timer conditions and labels the shadow pass as dominant (02:51:57). This is useful as a debugging sequence—form a candidate test, observe whether submitted work changes, then profile—but not a controlled benchmark or proof that the whole application is CPU-bound. Keep the measured scope, timer, camera path, and scene density attached to any performance conclusion.

The same stream says that transparent meshes are separated and submitted last in a forward-rendering path, with distance ordering, while the presenter notes that this policy may change for deferred rendering or order-independent transparency (01:45:48). That is a different correctness/performance tradeoff from shadow-caster culling: alpha blending can depend on ordering, while shadow inclusion depends on whether an object contributes to the shadow pass. One scene list should not silently impose a camera transparency order on every pass.

**Evidence limit.** These explanations derive from English caption cues, not a repository inspection or independent build. The episode contains exploratory narration and corrections while coding. In particular, it does not establish that the final line test handles every transform, that the line represents the exact cascade volume, or that the reported frame-time change came from this change alone. Use the math as a reviewable design pattern; validate any implementation against known bounds and the actual shadow-pass volume.

In floating-point code, rescale extreme directions or use stable norm calculations before evaluating cross products and squared distances. The exact distance formula is invariant under rescaling a nonzero direction; overflow and underflow in a naive implementation are not. Keep the invalid-zero-direction policy separate from a tolerance chosen for roundoff.


![[Visual Recall/Game Engine/corridor-and-shear.png]]

**Figure — a safe rejection needs an enclosing bound.** Left: the shaded strip is the y=0 cross-section of a line corridor with radius 0.4. Each object sphere has radius 0.5. The center at x=0.6 is retained; the center at x=3 is rejected. Retention means possible contribution, not proved visibility. Right: shearing a unit sphere creates an ellipsoid. Its central z=0 cross-section escapes the dashed radius-1.414 guess; the marked direction stretches to 1.618. The larger Frobenius radius 2 is safe but is not drawn. Both panels use equal horizontal and vertical length scales within each panel. [[Visual Recall/Game Engine/corridor-and-shear.svg|Editable geometric figure]].



### Shadow maps reuse visibility from the light’s viewpoint

A shadow asks whether a surface point can see a light. For a point light, render the scene once from the light’s position and store the nearest depth in a light-space depth map. Then render from the camera. For each camera fragment, recover its 3D position, project that position into light space, and compare its light-space depth $z_f$ with the stored nearest depth $z_s$ at that shadow-map texel. The point receives direct light when $z_f\le z_s+\epsilon$; otherwise a nearer occluder blocks it. The key duality is visibility from the eye versus visibility from the light. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (00:17:03–00:25:41)

The comparison is not exact in finite precision. Projection, depth quantization, and texel lookup can make a surface compare against itself as if it were behind an occluder. This self-shadowing appears as acne or streaks. A small depth bias $\epsilon$ moves the receiver away from the stored surface for comparison. Too little bias leaves acne; too much bias detaches the shadow from the caster (“Peter Panning”). The light-space map is also sampled on a finite grid: one coarse texel can cover a large ground area, producing blocky or unstable shadow boundaries. Filtering nearby depth comparisons (percentage-closer filtering) softens the sampled edge, but does not create a true area-light penumbra by itself. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (00:25:43–00:28:16; 00:42:50–00:44:06)

Shadow volumes take a geometric approach. For each silhouette edge as seen from the light, extrude a polygon away from the light to bound the points hidden behind the occluder. A stencil buffer records crossings of front and back volume faces along the camera ray; a nonzero count indicates the visible fragment lies inside a shadow volume. This avoids shadow-map texel resolution, but volume geometry and robust near/far handling can be costly. Z-pass counts volume crossings after the visible surface; z-fail (“Carmack’s reverse”) counts crossings behind it and handles cases where the volume extends beyond the far plane, at the cost of requiring closed volumes and careful caps. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (00:50:21–01:06:39)

A single shadow map spends equal resolution across its light projection, although the camera may see only a small region. Cascaded shadow maps fit several nested light-space projections to camera depth ranges: fine texels cover nearby visible areas, and coarser maps cover farther regions. The renderer selects the most detailed cascade that contains a fragment. Blending across cascade boundaries can hide abrupt transitions, but multiple maps cost extra rendering and can shimmer as the camera or light moves. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (00:44:06–00:49:31)

Opaque shadow maps store binary visibility. Smoke, hair, and other participating media attenuate light gradually along a ray. A deep shadow map stores a transmittance function $T(z)$ for each light-space texel: $T=1$ before entering the medium and then decreases as optical depth accumulates. At camera shading time, query the transmittance at the fragment’s light depth and multiply its direct illumination by $T$. Discretizing a depth-varying curve into samples is more expensive than one depth value, but represents partial occlusion. It still omits multiple scattering, where light is redirected within the medium. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-19 - Shadow maps|MITG-19]] (01:06:41–01:11:17)

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/27 - Mesh Queries, Classification and Clipping|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/29 - Acceleration Structures and Robust Queries|Next chapter]]
