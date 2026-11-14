# 33 — Projection, Clipping, and Raster Sampling

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|Next chapter]]

Orthographic and perspective projections map camera-space geometry into clip coordinates, where homogeneous clipping and division establish image position. The chapter continues through pixel and texel sampling, fullscreen rendering, and historical conventions that can invalidate naive offsets.

**By the end:** Derive projection mappings and explain clipping, perspective division, raster coordinates, and sample-center conventions.

**Recall question:** Why must clipping occur before perspective division, and when is a half-pixel adjustment incorrect?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 80694–84091; source-span SHA256 32c5a1323849c45482aa8ab1fe8b09afadcc574f1190425580f4e0622544d155. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Projection turns rays into clip coordinates

*Orthographic and perspective geometry, similar triangles, homogeneous clipping, perspective divide, complete matrix derivations.*

With a canonical view frame in place, the remaining question is how rays become bounded image coordinates. Similar triangles lead to the perspective divide.

**Orthographic projection geometry**

Imagine a camera looking along local $-Z$.

Consider two equal-sized squares, one at:

$$
 z=-2 
$$

and another at:

$$
 z=-20. 
$$

Under orthographic projection, their horizontal and vertical image dimensions are equal.

The depth changes, but the XY scale does not.

The camera is defined by a rectangular viewing volume with horizontal boundaries $l,r$, vertical boundaries $b,t$, and near/far distances $n,f$.

For our right-handed convention:

$$
 z_{\mathrm{near}}=-n, \qquad z_{\mathrm{far}}=-f 
$$

where:

$$
 0\<n\<f. 
$$

The view volume is a rectangular box, rather than a pyramidal frustum.

### Why parallel rays matter

Under orthographic projection, the projected XY position depends only on the camera-space XY coordinates.

For a centred orthographic camera:

$$
 x_n=k_xx_v, \qquad y_n=k_yy_v. 
$$

There is no division by depth.

A point at:

$$
 (1,2,-3) 
$$

and a point at:

$$
 (1,2,-300) 
$$

have identical projected XY coordinates.

They may have different depth-buffer values, but they occupy the same raster position.

### When orthographic projection is useful

Orthographic cameras are useful for CAD views, architectural plans, level editors, minimaps, technical diagrams, and some stylized games.

They are also useful for analyzing geometry because projected dimensions are not affected by camera depth.

However, calling an orthographic view physically realistic is not generally correct for a finite-distance pinhole camera. It can be interpreted as a limiting case in which projection rays become approximately parallel.

**Orthographic projection matrix derivation**

We want to map the horizontal camera-space interval:

$$
 x\in[l,r] 
$$

to:

$$
 x_n\in[-1,1]. 
$$

Assume an affine mapping:

$$
 x_n=ax+b. 
$$

Enforce the endpoints:

$$
 al+b=-1, 
$$

$$
 ar+b=1. 
$$

Subtract:

$$
 a(r-l)=2. 
$$

Therefore:

$$
 \boxed{ a=\frac2{r-l} }. 
$$

Substitute into the first equation:

$$
 b=-1-\frac{2l}{r-l}. 
$$

Combine:

$$
 \boxed{ b=-\frac{r+l}{r-l} }. 
$$

Thus:

$$
 \boxed{ x_n= \frac{2x}{r-l} -\frac{r+l}{r-l} }. 
$$

For the vertical coordinate, the same reasoning gives:

$$
 \boxed{ y_n= \frac{2y}{t-b} -\frac{t+b}{t-b} }. 
$$

### Derive the depth mapping

Under our right-handed camera convention, points in front have negative view-space $z$.

We want the conventional zero-to-one depth mapping:

$$
 z=-n\longmapsto z_n=0, 
$$

$$
 z=-f\longmapsto z_n=1. 
$$

Assume:

$$
 z_n=Az+B. 
$$

The endpoint equations are:

$$
 -An+B=0, 
$$

$$
 -Af+B=1. 
$$

Subtract the first equation from the second:

$$
 -A(f-n)=1. 
$$

Thus:

$$
 \boxed{ A=-\frac1{f-n} }. 
$$

From the first equation:

$$
 B=An=-\frac n{f-n}. 
$$

Therefore:

$$
 \boxed{ z_n=\frac{-z-n}{f-n} }. 
$$

### Assemble the matrix

Using column-vector multiplication:

$$
 \boxed{ P_{\mathrm{ortho}}^{[0,1]}= \begin{bmatrix} \dfrac2{r-l}&0&0&-\dfrac{r+l}{r-l}\\ 0&\dfrac2{t-b}&0&-\dfrac{t+b}{t-b}\\ 0&0&-\dfrac1{f-n}&-\dfrac n{f-n}\\ 0&0&0&1 \end{bmatrix} }. 
$$

Multiply it by:

$$
 \begin{bmatrix}x\\y\\z\\1\end{bmatrix}. 
$$

The resulting homogeneous coordinate has:

$$
 w_c=1. 
$$

Therefore the perspective division does nothing to XY.

This is why the orthographic matrix does not produce perspective shrinking.

### OpenGL default depth variant

If the target depth interval is:

$$
 [-1,1], 
$$

the depth mapping must send:

$$
 -n\mapsto-1, \qquad -f\mapsto1. 
$$

Solving the same affine mapping gives:

$$
 \boxed{ z_n= -\frac{2z+f+n}{f-n} }. 
$$

Thus the depth row becomes:

$$
 \begin{bmatrix} 0&0&-\dfrac2{f-n}&-\dfrac{f+n}{f-n} \end{bmatrix}. 
$$

The XY rows remain unchanged.

### Reversed-Z orthographic variant

For reversed zero-to-one depth:

$$
 -n\mapsto1, 
$$

$$
 -f\mapsto0. 
$$

The mapping is:

$$
 \boxed{ z_n=\frac{z+f}{f-n} }. 
$$

The third row becomes:

$$
 \begin{bmatrix} 0&0&\dfrac1{f-n}&\dfrac f{f-n} \end{bmatrix}. 
$$

Orthographic depth is linear in camera-space depth. Reversing the mapping does not create the same large floating-point precision benefit that reversed-Z produces for perspective projection.

### Numerical example · V11.03.08

Let:

$$
 l=-4,\ r=4,\ b=-3,\ t=3, 
$$

$$
 n=1,\ f=101. 
$$

For a point:

$$
 \mathbf p_V= \begin{bmatrix}2\\1.5\\-51\\1\end{bmatrix}, 
$$

the horizontal coordinate is:

$$
 x_n=\frac{2(2)}8=0.5. 
$$

The vertical coordinate is:

$$
 y_n=\frac{2(1.5)}6=0.5. 
$$

The depth is:

$$
 z_n=\frac{51-1}{101-1}=\frac{50}{100}=0.5. 
$$

Therefore:

$$
 \boxed{ \mathbf p_{\mathrm{NDC}}=(0.5,0.5,0.5) }. 
$$

Every coordinate lies at the midpoint between its corresponding boundaries.

**Perspective projection geometry**

In perspective projection, rays from visible scene points converge at the camera centre.

A point's apparent position depends on both its transverse displacement and its depth.

For our camera convention, define positive forward distance:

$$
 \boxed{d=-z_v>0}. 
$$

The camera looks toward $-Z$, but it is more convenient to express physical depth as a positive number $d$.

Suppose the camera has a virtual image plane at distance $s>0$.

A point:

$$
 (x_v,y_v,-d) 
$$

projects to image-plane coordinates:

$$
 \boxed{ x_{\mathrm{image}}=\frac{s}{d}x_v } 
$$

and:

$$
 \boxed{ y_{\mathrm{image}}=\frac{s}{d}y_v }. 
$$

The key factor is:

$$
 \frac1d. 
$$

As depth increases, apparent size decreases.

Side-view schematic of perspective geometry. The apparent extent of the view grows in proportion to distance from the camera.

### Projected size

Suppose an object has physical height $H_o$ and is approximately perpendicular to the viewing direction.

Its projected height on the virtual image plane is:

$$
 \boxed{ h_{\mathrm{image}}=\frac{sH_o}{d} }. 
$$

If the object moves from depth $d$ to depth $2d$:

$$
 h'_{\mathrm{image}} = \frac{sH_o}{2d} = \frac12h_{\mathrm{image}}. 
$$

Thus doubling distance halves apparent size in the ideal pinhole model.

This is the basic geometric explanation for perspective.

### Perspective is a rational mapping

The projected coordinate:

$$
 x_{\mathrm{image}}=\frac{s x_v}{d} 
$$

is a ratio of quantities.

It is not an affine transformation of $(x_v,y_v,z_v)$, because the denominator depends on $z_v$.

A conventional $3\times3$ linear transformation cannot directly express this mapping in ordinary Cartesian 3D coordinates.

Homogeneous coordinates solve that problem: we perform a linear transformation in 4D and then divide by $w$.

**Similar triangles behind perspective projection**

We can derive perspective using elementary geometry.

Consider a world point whose horizontal displacement from the camera's optical axis is $x$, at depth $d$.

Let $x_s$ be its displacement on an image plane at distance $s$.

The two right triangles formed by the camera centre, optical axis, and projected point are similar.

Therefore:

$$
 \frac{x_s}{s}=\frac{x}{d}. 
$$

Multiply by $s$:

$$
 \boxed{ x_s=\frac{s}{d}x }. 
$$

The same derivation gives:

$$
 \boxed{ y_s=\frac{s}{d}y }. 
$$

### Field of view from the viewing triangle

Suppose the visible image-plane height is $H_s$.

The vertical field of view is $\theta_y$.

The upper half of the viewing triangle satisfies:

$$
 \tan\left(\frac{\theta_y}{2}\right) = \frac{H_s/2}{s}. 
$$

Therefore:

$$
 \boxed{ H_s=2s\tan\left(\frac{\theta_y}{2}\right) }. 
$$

At depth $d$, the visible world-space height is:

$$
 \boxed{ H(d)=2d\tan\left(\frac{\theta_y}{2}\right) }. 
$$

This formula is useful for determining the footprint of a pixel at a given camera depth and for selecting spatial detail relative to screen resolution.

### Numerical example · V11.03.12

Suppose:

$$
 \theta_y=60^\circ, \qquad d=10. 
$$

Then:

$$
 H(10)=20\tan30^\circ. 
$$

Since:

$$
 \tan30^\circ=\frac1{\sqrt3}, 
$$

we get:

$$
 \boxed{ H(10)\approx11.5470 }. 
$$

At depth 20:

$$
 H(20)\approx23.0940. 
$$

The visible height doubles with depth.

For a framebuffer with 1080 pixels vertically, the idealized world-space height corresponding to one pixel at depth 10 is approximately:

$$
 \frac{11.5470}{1080}\approx0.01069 
$$

world units, assuming a locally front-facing plane and simple pinhole projection.

At depth 20, that quantity doubles.

This is one reason distant geometry has smaller screen-space footprints.

**The perspective divide**

The perspective divide converts homogeneous clip coordinates:

$$
 \mathbf p_C= (x_c,y_c,z_c,w_c) 
$$

into NDC:

$$
 \boxed{ \mathbf p_N= \left( \frac{x_c}{w_c}, \frac{y_c}{w_c}, \frac{z_c}{w_c} \right) }. 
$$

In our right-handed perspective convention, we will choose:

$$
 \boxed{ w_c=-z_v=d }. 
$$

Then any clip coordinate proportional to camera-space $x$ or $y$ becomes inversely proportional to depth after division.

For example, choose:

$$
 x_c=k_xx_v. 
$$

Then:

$$
 x_n= \frac{k_xx_v}{-z_v} = k_x\frac{x_v}{d}. 
$$

This is exactly the similar-triangle relationship.

### Worked example · V11.03.13

Suppose the projection produces:

$$
 \mathbf p_C= \begin{bmatrix} 2\\1\\3\\4 \end{bmatrix}. 
$$

The NDC position is:

$$
 \boxed{ \mathbf p_N= \begin{bmatrix} 0.5\\0.25\\0.75 \end{bmatrix} }. 
$$

The division is applied to all three spatial coordinates, including depth.

### What happens when $w_c=0$?

The perspective divide is undefined.

For our perspective matrix:

$$
 w_c=-z_v. 
$$

Thus a point lying exactly on the camera plane:

$$
 z_v=0 
$$

has:

$$
 w_c=0. 
$$

It cannot be projected as an ordinary finite camera-visible point through the perspective divide.

The clipping and rasterization pipeline must handle such geometry before performing the division.

### What happens for negative $w_c$?

A point behind the camera has:

$$
 z_v>0, 
$$

so:

$$
 w_c=-z_v<0. 
$$

Blindly dividing a behind-camera point can produce XY coordinates that look numerically plausible while describing an invalid visible projection.

This can cause geometry to appear mirrored, jump across the screen, or produce nonsensical bounding boxes.

A correct pipeline uses homogeneous clipping and visibility rules rather than treating every finite divided coordinate as a visible pixel.

**Homogeneous clip coordinates**

Homogeneous coordinates represent geometric points in projective space.

For ordinary affine 3D points, we commonly use:

$$
 (x,y,z,1). 
$$

But a general projective point is represented by:

$$
 \boxed{ (x,y,z,w) } 
$$

with not all components zero.

Two nonzero homogeneous vectors represent the same projective point if they differ by a nonzero scalar factor:

$$
 \boxed{ (x,y,z,w) \sim (\lambda x,\lambda y,\lambda z,\lambda w) } 
$$

for:

$$
 \lambda\ne0. 
$$

For example:

$$
 (2,4,6,2) 
$$

and:

$$
 (1,2,3,1) 
$$

represent the same finite point.

After homogeneous division, both produce:

$$
 (1,2,3). 
$$

### Why projective equivalence matters

Suppose a projection matrix produces:

$$
 \mathbf p_C=(x_c,y_c,z_c,w_c). 
$$

Multiplying all four coordinates by a positive nonzero scalar does not change the post-divide NDC values.

The projective point is unchanged.

However, when interpreting clip inequalities in a graphics pipeline, the sign and representation of $w_c$ matter. We should not assume every arbitrary homogeneous rescaling is interchangeable with the hardware's forward-visible clipping convention.

### Affine transformations versus projective transformations

An affine transformation has a final row of:

$$
 (0,0,0,1) 
$$

under the conventional homogeneous matrix representation.

Therefore the resulting $w$ of an affine point remains one.

A perspective projection matrix contains a final row involving the original $z$ coordinate.

Thus:

$$
 w_c 
$$

depends on depth.

That is the mathematical mechanism allowing perspective projection to be expressed as a $4\times4$ matrix followed by division.

### Geometric significance of homogeneous $w=0$

In projective geometry, homogeneous vectors with:

$$
 w=0 
$$

can represent points at infinity, corresponding to directions.

This connects to affine direction vectors, which we also represent using homogeneous $w=0$.

But the geometric interpretation must be handled carefully: a direction under an affine transformation and a projective point at infinity are related concepts, not evidence that every $w=0$ value in a graphics shader can be used as an ordinary finite position.

Projective points at infinity receive a deeper treatment in the projective-geometry chapter.

**The role of clip-space $w$**

The fourth clip coordinate has several distinct roles that are easy to conflate.

First, it enables the perspective divide:

$$
 x_n=x_c/w_c, \qquad y_n=y_c/w_c. 
$$

Second, it defines the homogeneous clipping boundaries.

Third, it is used in perspective-correct interpolation of vertex attributes.

Fourth, its value carries information about camera depth under a particular projection convention.

### Why $w_c=-z_v$ is natural

Our camera looks toward negative view-space $Z$.

A visible point has:

$$
 z_v=-d, \qquad d>0. 
$$

If we set:

$$
 w_c=-z_v, 
$$

then:

$$
 w_c=d>0. 
$$

The perspective divide gives:

$$
 x_n\propto x_v/d. 
$$

Thus projected displacement decreases as positive depth increases.

The $w$ coordinate naturally represents forward view depth for this conventional perspective matrix.

### $w$ is not always depth

The identity:

$$
 w_c=-z_v 
$$

is a property of the particular perspective matrix we choose.

It is not a universal definition of the fourth homogeneous coordinate.

For a conventional orthographic projection:

$$
 w_c=1. 
$$

For an oblique or more general projective transformation, $w_c$ may be a different linear combination of input coordinates.

Therefore code should not assume every clip-space $w$ equals metric depth unless the projection contract guarantees it.

### Perspective-correct interpolation preview

Suppose an attribute $a$, such as UV coordinates, is defined at triangle vertices.

Directly interpolating $a$ linearly in screen space generally does not reproduce the correct value across a perspectively projected 3D triangle.

The rasterizer uses relationships involving:

$$
 \frac{a_i}{w_i} 
$$

and:

$$
 \frac1{w_i}. 
$$

The attribute can be reconstructed using:

$$
 \boxed{ a= \frac{ \sum_i\lambda_i(a_i/w_i) }{ \sum_i\lambda_i(1/w_i) } } 
$$

where $\lambda_i$ are screen-space barycentric coordinates.

The projective-equivalence derivation below supplies the missing step for perspective-correct attribute interpolation.

For now, understand why $w$ must survive projection long enough for the rasterizer to use it.

**Perspective projection matrix derivation**

We now derive the full perspective projection matrix rather than memorizing it.

Assume:

- Right-handed camera coordinates.
- Camera forward is $-Z$.
- Positive near and far distances $0\<n\<f$.
- Vertical field of view $\theta_y$.
- Aspect ratio $a=W/H$.
- Zero-to-one NDC depth, with near mapped to 0 and far mapped to 1.
- Column-vector multiplication.

### Derive the vertical scale

At positive depth $d=-z_v$, the half-height of the visible region is:

$$
 h(d)=d\tan(\theta_y/2). 
$$

A point at the top of the view has:

$$
 y_v=h(d). 
$$

We want it to map to:

$$
 y_n=1. 
$$

Therefore:

$$
 y_n= \frac{y_v}{d\tan(\theta_y/2)}. 
$$

Define:

$$
 \boxed{ s_y=\frac1{\tan(\theta_y/2)} }. 
$$

Then:

$$
 \boxed{ y_n=\frac{s_yy_v}{d} }. 
$$

### Derive the horizontal scale

The visible width is the visible height multiplied by aspect ratio:

$$
 w(d)=a\,h(d). 
$$

Thus the horizontal half-width is:

$$
 a\,d\tan(\theta_y/2). 
$$

Therefore:

$$
 \boxed{ x_n= \frac{x_v}{ad\tan(\theta_y/2)} }. 
$$

Define:

$$
 \boxed{ s_x=\frac{s_y}{a} }. 
$$

Then:

$$
 \boxed{ x_n=\frac{s_xx_v}{d} }. 
$$

### Choose homogeneous $w$

Let:

$$
 \boxed{ w_c=-z_v=d }. 
$$

Then we can generate the desired XY numerators using:

$$
 x_c=s_xx_v, \qquad y_c=s_yy_v. 
$$

After division:

$$
 x_n=x_c/w_c, \qquad y_n=y_c/w_c. 
$$

### Derive the depth row

Assume the clip-space depth is linear in view-space $z$:

$$
 \boxed{ z_c=Az_v+B }. 
$$

Since:

$$
 w_c=-z_v, 
$$

the NDC depth is:

$$
 z_n= \frac{Az_v+B}{-z_v}. 
$$

Let positive view depth be:

$$
 d=-z_v. 
$$

Then:

$$
 z_c=-Ad+B, 
$$

$$
 w_c=d. 
$$

Therefore:

$$
 \boxed{ z_n=-A+\frac Bd }. 
$$

We require near depth:

$$
 d=n\longmapsto z_n=0. 
$$

Thus:

$$
 -A+\frac Bn=0. 
$$

So:

$$
 \boxed{B=An}. 
$$

We also require far depth:

$$
 d=f\longmapsto z_n=1. 
$$

Thus:

$$
 -A+\frac Bf=1. 
$$

Substitute $B=An$:

$$
 -A+\frac{An}{f}=1. 
$$

Factor:

$$
 A\left(-1+\frac nf\right)=1. 
$$

Therefore:

$$
 A\frac{n-f}{f}=1. 
$$

Hence:

$$
 \boxed{ A=\frac f{n-f} =-\frac f{f-n} }. 
$$

And:

$$
 \boxed{ B=An= \frac{fn}{n-f} =-\frac{fn}{f-n} }. 
$$

### Assemble the matrix · V11.03.26

We want:

$$
 x_c=s_xx_v, 
$$

$$
 y_c=s_yy_v, 
$$

$$
 z_c=Az_v+B, 
$$

$$
 w_c=-z_v. 
$$

Therefore:

$$
 \boxed{ P_{\mathrm{perspective}}^{[0,1]}= \begin{bmatrix} \dfrac1{a\tan(\theta_y/2)}&0&0&0\\ 0&\dfrac1{\tan(\theta_y/2)}&0&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix} }. 
$$

Every nonzero entry has now been derived.

The bottom row produces perspective $w$.

The top rows produce the field-of-view scaling.

The third row produces the requested near-to-far depth mapping.

### Verify the near plane

Take a point on the optical axis at the near plane:

$$
 \mathbf p_V=(0,0,-n,1)^T. 
$$

Its clip $z$ is:

$$
 z_c= -\frac f{f-n}(-n)-\frac{fn}{f-n}. 
$$

Thus:

$$
 z_c= \frac{fn}{f-n}-\frac{fn}{f-n}=0. 
$$

Its clip $w$ is:

$$
 w_c=n. 
$$

Therefore:

$$
 \boxed{z_n=0/n=0}. 
$$

### Verify the far plane

At:

$$
 z_v=-f, 
$$

we get:

$$
 z_c= \frac{f^2}{f-n}-\frac{fn}{f-n}. 
$$

Factor:

$$
 z_c= \frac{f(f-n)}{f-n}=f. 
$$

And:

$$
 w_c=f. 
$$

Thus:

$$
 \boxed{z_n=f/f=1}. 
$$

### Numerical matrix example

Use:

$$
 \theta_y=60^\circ, 
$$

$$
 a=\frac{16}{9}, 
$$

$$
 n=0.1,\qquad f=1000. 
$$

Because:

$$
 \tan30^\circ=\frac1{\sqrt3}, 
$$

the vertical scale is:

$$
 s_y=\sqrt3\approx1.7320508. 
$$

The horizontal scale is:

$$
 s_x=\frac{\sqrt3}{16/9} \approx0.9742786. 
$$

The depth coefficients are approximately:

$$
 A=-1.00010001, 
$$

$$
 B=-0.100010001. 
$$

Thus:

$$
 \boxed{ P\approx \begin{bmatrix} 0.9742786&0&0&0\\ 0&1.7320508&0&0\\ 0&0&-1.00010001&-0.100010001\\ 0&0&-1&0 \end{bmatrix} }. 
$$

This is a conventional right-handed, zero-to-one, non-reversed perspective projection.

It is directly compatible with the mathematical convention used by the existing `KairoMath::Perspective` function, subject to viewport Y handling.


<!-- end V11.03 -->

<!-- begin V11.04 -->
## Depth is a numerical design choice

*FOV and aspect, near/far planes, nonlinear precision, Z-fighting, reversed-Z, infinite far plane.*

The projection also assigns depth. That mapping controls both clipping and how finely the depth buffer distinguishes nearby surfaces.

**Vertical field of view**

Vertical field of view, written:

$$
 \boxed{\theta_y}, 
$$

is the angular extent visible from the bottom to the top of the image.

For a pinhole camera with virtual image-plane half-height $h$ and image-plane distance $s$:

$$
 \boxed{ \tan(\theta_y/2)=\frac hs }. 
$$

Therefore:

$$
 \boxed{ \theta_y=2\arctan(h/s) }. 
$$

### Field of view controls perspective scale

The projection Y scale is:

$$
 \boxed{ s_y=\frac1{\tan(\theta_y/2)} }. 
$$

A smaller field of view gives a larger projection scale.

Objects appear larger in the image.

A larger field of view gives a smaller projection scale.

Objects appear smaller, with more of the surrounding scene visible.

### Numerical comparison

For:

$$
 \theta_y=30^\circ, 
$$

we have:

$$
 s_y=\frac1{\tan15^\circ}\approx3.732. 
$$

For:

$$
 \theta_y=60^\circ, 
$$

we have:

$$
 s_y=\frac1{\tan30^\circ}\approx1.732. 
$$

For:

$$
 \theta_y=90^\circ, 
$$

we have:

$$
 s_y=\frac1{\tan45^\circ}=1. 
$$

As vertical FOV increases, projected object height decreases for the same depth.

### Units: degrees versus radians

C++ trigonometric functions expect radians.

Convert degrees to radians:

$$
 \boxed{ \theta_{\mathrm{rad}}= \theta_{\mathrm{deg}}\frac\pi{180} }. 
$$

A camera API should use a clearly named type or field such as:

```
double verticalFovRadians;
```

rather than an ambiguous `float fov`.

A common camera bug is interpreting 60 degrees as 60 radians.

The resulting projection scale is nonsensical for an ordinary perspective camera.

**Horizontal field of view**

Horizontal FOV depends on the vertical FOV and aspect ratio.

Let the image-plane half-width be $w$ and half-height be $h$.

The aspect ratio is:

$$
 a=\frac wh. 
$$

We know:

$$
 \tan(\theta_y/2)=\frac hs. 
$$

Also:

$$
 \tan(\theta_x/2)=\frac ws. 
$$

Because:

$$
 w=ah, 
$$

we get:

$$
 \tan(\theta_x/2) = a\frac hs. 
$$

Therefore:

$$
 \boxed{ \tan(\theta_x/2) = a\tan(\theta_y/2) }. 
$$

Take arctangent:

$$
 \boxed{ \theta_x= 2\arctan \left( a\tan(\theta_y/2) \right) }. 
$$

This formula is essential when changing window aspect ratio.

### Numerical example · V11.04.05

Suppose:

$$
 \theta_y=60^\circ, \qquad a=\frac{16}{9}. 
$$

Then:

$$
 \theta_x= 2\arctan \left( \frac{16}{9}\tan30^\circ \right). 
$$

Numerically:

$$
 \boxed{ \theta_x\approx91.492845^\circ }. 
$$

Thus a 60° vertical FOV corresponds to approximately 91.5° horizontal FOV at 16:9.

### Why FOV angles are not proportional to aspect ratio

A tempting mistake is:

$$
 \theta_x=a\theta_y. 
$$

This is generally wrong.

The correct relationship is through the tangent of the half-angle.

Perspective geometry is nonlinear in angle.

For small angles, $\tan\alpha\approx\alpha$, so the linear approximation may appear reasonable.

For wide FOVs, the error becomes significant.

**Aspect ratio**

The projection aspect ratio is:

$$
 \boxed{ a=\frac WH } 
$$

where $W,H$ describe the intended image or viewport dimensions, assuming square pixels in the usual simple model.

For:

$$
 W=1920,\quad H=1080, 
$$

we have:

$$
 \boxed{ a=\frac{16}{9}\approx1.777778 }. 
$$

Aspect ratio affects the horizontal scale of a perspective projection when vertical FOV is held constant:

$$
 s_x=\frac{s_y}{a}. 
$$

### Horizontal versus vertical FOV locking

A camera system can choose to preserve vertical FOV when the viewport changes size.

This is common in game engines.

If the viewport becomes wider while vertical FOV stays fixed, the horizontal FOV increases.

Alternatively, a system may preserve horizontal FOV.

Then the vertical FOV changes according to aspect ratio.

The choice changes what the camera reveals during resizing.

### Example · V11.04.08

Suppose the vertical FOV remains:

$$
 60^\circ. 
$$

At aspect 1:1:

$$
 \theta_x=60^\circ. 
$$

At aspect 16:9:

$$
 \theta_x\approx91.49^\circ. 
$$

At aspect 21:9:

$$
 \theta_x= 2\arctan\left(\frac{21}{9}\tan30^\circ\right) \approx106.82^\circ. 
$$

Wider aspect reveals more horizontal angular extent under vertical-FOV locking.

### Pixel aspect ratio

Our simple equation assumes square pixels.

Some imaging systems use non-square pixel aspect ratios or calibration matrices with different horizontal and vertical scale factors.

In general calibrated-camera mathematics, the relationship between pixel dimensions, physical sensor dimensions, and focal length can require more than the framebuffer aspect ratio alone.

The intrinsic matrix $K$ is derived in the calibration chapter.

**The near clipping plane**

The near clipping plane is located at positive camera depth:

$$
 \boxed{d=n} 
$$

or:

$$
 z_v=-n 
$$

under our right-handed convention.

For ordinary perspective projection:

$$
 \boxed{n>0}. 
$$

A near plane at exactly zero causes a degenerate perspective frustum and invalid matrix coefficients.

### What does the near plane actually do?

It establishes the closest permitted camera-space depth for ordinary visible geometry.

Geometry closer than the near plane is clipped, assuming the standard depth-clipping behavior.

This prevents the rasterizer from attempting to project geometry that intersects the singular camera centre.

### Near clipping is not the same as ray-intersection epsilon

A ray tracer may use a minimum ray parameter:

$$
 t_{\min}=\varepsilon 
$$

to avoid self-intersections or reject nearby hits.

That ray parameter is not automatically equivalent to a hardware camera near clipping plane.

A ray tracer can model rays with different origin conventions and intersection domains.

The two concepts serve related but distinct purposes.

### Why a very small near plane can be dangerous

Suppose:

$$
 n=0.0001, \qquad f=1000. 
$$

The far-to-near ratio is:

$$
 \frac fn=10^7. 
$$

Under conventional perspective depth mapping, finite depth-buffer precision becomes especially problematic at large distances.

The near plane has enormous influence on this precision distribution.

This surprises developers who assume moving the far plane outward is always the main cause of poor depth precision.

In conventional perspective projection, moving the near plane unnecessarily close to zero can be much more damaging.

**The far clipping plane**

The far plane is located at positive camera depth:

$$
 \boxed{d=f} 
$$

with:

$$
 f>n. 
$$

Under ordinary finite perspective projection, it defines the farthest depth accepted by the conventional clipping volume.

### What the far plane controls

It affects the visible depth range and the coefficients of the depth projection.

Under conventional depth testing, geometry beyond the far plane is clipped.

But increasing $f$ does not increase field of view.

Field of view determines angular visibility.

Near/far planes determine depth limits.

These are independent camera properties in the idealized projection model.

### Numerical example · V11.04.14

Suppose:

$$
 n=0.1,\qquad f=100. 
$$

The depth range extends from 0.1 to 100 world units.

If the far plane becomes:

$$
 f=1000, 
$$

the projection now accepts geometry ten times farther away.

The XY perspective relationship for a point at a given depth remains unchanged if field of view and aspect ratio remain fixed.

But the depth mapping changes because the near/far coefficients change.

### Can the far plane be infinite?

Yes, for perspective projection we can take the limit:

$$
 f\to\infty 
$$

and derive a finite projection matrix.

This does not mean every point at infinite physical distance can be rasterized as ordinary finite geometry.

It means the projection no longer uses a finite far clipping boundary in the same way.

The infinite-far projection follows by taking the finite-far limit.

**Depth precision distribution in perspective projection**

This part explains an enormous number of rendering artifacts.

Many developers assume a depth buffer stores physical distance from the camera:

$$
 d=0.1,\ 1,\ 10,\ 100,\ldots 
$$

in a linear fashion.

For conventional perspective projection, that is not what happens.

The post-divide depth is a function of reciprocal camera-space depth:

$$
 \boxed{ z_{\mathrm{NDC}}(d)=A+\frac Bd }. 
$$

The constants depend on the projection convention and near/far planes.

For our conventional right-handed zero-to-one projection:

$$
 \boxed{ z_{\mathrm{NDC}}(d) = \frac f{f-n} - \frac{fn}{(f-n)d} }. 
$$

You can rewrite this as:

$$
 \boxed{ z_{\mathrm{NDC}}(d) = \frac f{f-n}\left(1-\frac nd\right) }. 
$$

This formula maps:

$$
 d=n\mapsto0, 
$$

$$
 d=f\mapsto1. 
$$

But the mapping is not linear in $d$.

### Why depth precision concentrates near the camera

Differentiate:

$$
 z(d)= \frac f{f-n} - \frac{fn}{(f-n)d}. 
$$

The derivative is:

$$
 \boxed{ \frac{dz}{dd} = \frac{fn}{(f-n)d^2} }. 
$$

The derivative decreases as:

$$
 \frac1{d^2}. 
$$

Therefore a fixed change in physical depth produces a much smaller change in stored depth when the object is far away.

Near the camera, depth differences are spread across a relatively large range of buffer values.

Far away, many different physical depths are compressed into a tiny portion of the depth interval.

### Numerical depth mapping

Use:

$$
 n=0.1,\qquad f=1000. 
$$

The conventional depth mapping gives:

| Positive view depth $d$ | Stored normalized depth |
| --------------------------- | ----------------------- |
| 0.1                         | 0                       |
| 1                           | 0.900090009             |
| 10                          | 0.990099010             |
| 100                         | 0.999099910             |
| 1000                        | 1.000000000             |

Notice what happens.

By depth one, approximately 90% of the normalized depth interval has already been used.

At depth ten, approximately 99% has been used.

The entire region from depth 100 to 1000 occupies only a small fraction of the depth range near one.

Perspective depth is nonlinear

Near = 0.1, far = 1000. The horizontal axis shows physical view depth; conventional depth values cluster near 1 at large distances.

Standard 0–1 depthInfinite reversed-Z depth

00.30.60.91.20.10.20.5125101001000

The chart compares conventional finite-far depth with infinite reversed-Z. The latter has no finite far clipping boundary. Both use reciprocal-depth relationships, but they place the numerically sensitive parts of the range in different regions of floating-point representation.

### Approximate physical depth resolution

Suppose the stored normalized depth changes by a small amount:

$$
 \Delta z. 
$$

Using a first-order approximation:

$$
 \Delta z\approx\frac{dz}{dd}\Delta d. 
$$

Therefore:

$$
 \Delta d\approx \frac{\Delta z}{dz/dd}. 
$$

Substitute the derivative:

$$
 \boxed{ \Delta d \approx \frac{(f-n)d^2}{fn}\Delta z }. 
$$

This is an approximate local mapping from depth-buffer spacing to physical view-depth spacing.

### A 24-bit fixed-point example

For a normalized 24-bit fixed-point depth buffer, the approximate quantization step is:

$$
 \Delta z= \frac1{2^{24}-1} \approx5.96\times10^{-8}. 
$$

With:

$$
 n=0.1,\quad f=1000, 
$$

we obtain the following first-order physical depth increments:

| View depth | Approximate depth increment |
| ---------- | --------------------------- |
| 0.1        | $5.96\times10^{-9}$     |
| 1          | $5.96\times10^{-7}$     |
| 10         | $5.96\times10^{-5}$     |
| 100        | $5.96\times10^{-3}$     |
| 1000       | $5.96\times10^{-1}$     |

At depth 1000, the local depth interval represented by one quantization step is approximately 0.596 world units under this first-order calculation.

That is a major loss of distinguishable depth compared with the near plane.

These are local approximations based on idealized fixed-point spacing, not guarantees that every depth comparison resolves exactly the listed distance. Rasterization, interpolation, fragment depth, quantization, and geometry orientation also affect practical behavior.

### How the near plane affects precision

The precision formula contains:

$$
 \frac1n. 
$$

If the near plane is pushed closer to zero, the far-depth resolution deteriorates.

For a large depth range $f\gg n$, the coefficient:

$$
 \frac{f-n}{fn} 
$$

is approximately:

$$
 \frac1n. 
$$

Therefore:

$$
 \boxed{ \Delta d\approx\frac{d^2}{n}\Delta z }. 
$$

This makes the near plane extremely important.

For conventional perspective depth, choose the largest acceptable near distance that does not clip essential nearby geometry.

### Fixed-point versus floating-point depth

A fixed-point depth buffer has approximately uniform representable spacing in its normalized integer range.

A floating-point depth buffer has nonuniform representable spacing, with much finer absolute spacing near zero than near one.

This is why simply reversing the depth order provides a particularly strong benefit when combined with floating-point depth storage.

The analysis and diagrams in Nathan Reed's [Visualizing Depth Precision](https://developer.nvidia.com/blog/visualizing-depth-precision/?utm_source=chatgpt.com) develop this reciprocal-depth relationship in detail.&#x20;


**Z-fighting**

Z-fighting occurs when two surfaces produce depth values that are indistinguishable or nearly indistinguishable at the depth buffer's available precision.

The rasterizer must decide which surface is in front.

If the stored depth values are equal or fluctuate under numerical differences, fragments from different surfaces can alternately win the depth test.

The result may appear as flickering, striped patterns, unstable overlays, or surface fragments switching visibility.

### A basic example

Suppose two walls are positioned at:

$$
 d_1=500.000, 
$$

$$
 d_2=500.001. 
$$

Their physical separation is:

$$
 0.001. 
$$

But under conventional perspective depth, the difference in normalized depth may be smaller than one representable depth-buffer step.

The buffer cannot reliably distinguish them.

### Other causes that resemble Z-fighting

Not every flickering surface is caused by insufficient depth quantization.

Possible contributors include overlapping coplanar polygons, inconsistent transforms, polygon depth bias, insufficient precision in the vertex transformation, shader-computed fragment depth, incorrect depth compare modes, and temporal movement that changes geometry's quantization.

An inverted Y axis or incorrect face culling can also produce visual symptoms that developers mistakenly attribute to depth precision.

The diagnostic must follow actual clip and depth values, not merely the visual appearance.

### Remedies

A better near plane, floating-point reversed-Z depth, appropriate geometric separation, and carefully applied depth bias can improve the situation.

But every remedy has trade-offs.

Artificially moving one surface can alter geometry.

Polygon offset can create incorrect shadows or contact artifacts.

Changing depth comparison without changing projection and clear values can break the entire visibility ordering.

Reversed-Z helps numerical depth precision substantially, but it does not make truly coplanar surfaces uniquely ordered.

**Reversed-Z depth buffers**

Reversed-Z changes the depth mapping so that the near plane corresponds to a larger depth value and the far plane corresponds to a smaller value.

For zero-to-one depth:

$$
 \boxed{ d=n\mapsto1 } 
$$

and:

$$
 \boxed{ d=f\mapsto0 }. 
$$

The camera remains right-handed and forward remains $-Z$.

Reversed-Z does not require changing camera handedness.

It changes the depth mapping and associated depth-test convention.

### Deriving reversed-Z

We still use:

$$
 w_c=-z_v=d. 
$$

Assume:

$$
 z_c=Az_v+B. 
$$

Therefore:

$$
 z_n=-A+\frac Bd. 
$$

Now enforce:

$$
 -A+\frac Bn=1 
$$

and:

$$
 -A+\frac Bf=0. 
$$

Subtract the second equation from the first:

$$
 B\left(\frac1n-\frac1f\right)=1. 
$$

Thus:

$$
 B\frac{f-n}{nf}=1. 
$$

Therefore:

$$
 \boxed{ B=\frac{nf}{f-n} }. 
$$

From the far equation:

$$
 -A+\frac Bf=0. 
$$

So:

$$
 A=\frac Bf=\frac n{f-n}. 
$$

Hence:

$$
 \boxed{ z_c= \frac n{f-n}z_v+ \frac{nf}{f-n} }. 
$$

The complete reversed-Z perspective matrix becomes:

$$
 \boxed{ P_{\mathrm{reverse}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&\dfrac n{f-n}&\dfrac{nf}{f-n}\\ 0&0&-1&0 \end{bmatrix} }. 
$$

After division:

$$
 \boxed{ z_n(d)= \frac{nf}{(f-n)d} -\frac n{f-n} }. 
$$

At the near plane:

$$
 d=n: 
$$

$$
 z_n= \frac f{f-n}-\frac n{f-n}=1. 
$$

At the far plane:

$$
 d=f: 
$$

$$
 z_n= \frac n{f-n}-\frac n{f-n}=0. 
$$

The boundary conditions are satisfied exactly in real arithmetic.

### Why floating-point reversed-Z works

Conventional perspective depth places far geometry near depth value one, where floating-point numbers have relatively coarse absolute spacing.

Reversed-Z places far geometry near zero, where floating-point numbers have much finer absolute spacing.

The nonlinear $1/d$ depth relationship is partly compensated by floating-point spacing becoming finer near zero.

The resulting precision is much more evenly useful over a large depth range.

This does not mean reversed-Z produces a mathematically linear depth buffer.

It means the interaction between reciprocal-depth mapping and floating-point representation is advantageous.

### Required pipeline changes

A conventional depth buffer often uses:

```
depthClearValue = 1.0f;
depthCompareOp = LESS;
```

With reversed-Z, use the corresponding convention:

```
depthClearValue = 0.0f;
depthCompareOp = GREATER;
```

or `GREATER_OR_EQUAL` if its equality semantics match the intended pipeline.

The projection matrix must match that convention.

Depth-based effects such as hierarchical Z, depth reconstruction, shadow comparisons, and screen-space algorithms must use compatible assumptions.

### Hierarchical Z interpretation

With conventional depth, smaller values are nearer.

With reversed-Z, larger values are nearer.

Algorithms that build hierarchical depth extrema must account for this inversion.

For example, a reduction tree that stores a minimum depth value may need to use a maximum instead for an equivalent near-surface interpretation, depending on the precise culling or occlusion query.

The correct reduction operator is determined by the query's semantics, not merely by the name "reversed-Z."

### Is reversed-Z equally useful for integer depth?

The large precision benefit is most pronounced with floating-point depth buffers.

For uniformly spaced fixed-point depth values, reversing the normalized depth order does not fundamentally change the magnitude of reciprocal-depth quantization intervals.

It changes which numerical codes represent near versus far, but not the underlying uniform depth spacing.

This is why floating-point depth plus reversed-Z is the particularly important combination.&#x20;


**Infinite far-plane projection**

An infinite far plane is obtained by taking:

$$
 \boxed{f\to\infty}. 
$$

We can derive the resulting matrices from the finite-far formulas.

### Conventional zero-to-one infinite perspective

Start from:

$$
 P= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix}. 
$$

As:

$$
 f\to\infty, 
$$

we have:

$$
 \frac f{f-n}\to1. 
$$

Also:

$$
 \frac{fn}{f-n}\to n. 
$$

Therefore:

$$
 \boxed{ P_{\infty,\mathrm{standard}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-1&-n\\ 0&0&-1&0 \end{bmatrix} }. 
$$

After division:

$$
 z_n(d)= \frac{d-n}{d} = 1-\frac nd. 
$$

Thus:

$$
 \boxed{ z_n(d)=1-\frac nd }. 
$$

At $d=n$, depth is zero.

As $d\to\infty$, depth approaches one.

### Reversed-Z infinite perspective

Start from:

$$
 P_{\mathrm{reverse}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&\dfrac n{f-n}&\dfrac{nf}{f-n}\\ 0&0&-1&0 \end{bmatrix}. 
$$

As $f\to\infty$:

$$
 \frac n{f-n}\to0. 
$$

And:

$$
 \frac{nf}{f-n}\to n. 
$$

Thus:

$$
 \boxed{ P_{\infty,\mathrm{reverse}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&0&n\\ 0&0&-1&0 \end{bmatrix} }. 
$$

After division:

$$
 z_c=n, 
$$

$$
 w_c=d. 
$$

Therefore:

$$
 \boxed{ z_n(d)=\frac nd }. 
$$

This is extraordinarily simple.

Near plane:

$$
 d=n\Rightarrow z_n=1. 
$$

Infinite depth:

$$
 d\to\infty\Rightarrow z_n\to0. 
$$

### Recover positive view depth

For this particular infinite reversed-Z projection:

$$
 z_n=\frac nd. 
$$

Solve:

$$
 \boxed{ d=\frac n{z_n} }. 
$$

For $z_n=0$, the ideal mapping corresponds to infinite depth.

For finite depth:

$$
 z_n>0. 
$$

This simple relation is useful for understanding depth reconstruction, although robust implementation must also account for the viewport depth transform, precision, and any projection modifications.

Depth linearization follows in the later depth-reconstruction chapter.

### Numerical example · V11.04.33

Let:

$$
 n=0.1. 
$$

For:

$$
 d=1, 
$$

we have:

$$
 z_n=\frac{0.1}{1}=0.1. 
$$

For:

$$
 d=10, 
$$

we have:

$$
 z_n=0.01. 
$$

For:

$$
 d=100, 
$$

we have:

$$
 z_n=0.001. 
$$

For:

$$
 d=1000, 
$$

we have:

$$
 z_n=0.0001. 
$$

The matrix needs no finite far value.

### Infinite far does not eliminate every depth issue

The infinite projection removes the finite far-clipping boundary under its standard usage.

It does not make floating-point geometry calculations infinitely precise.

Very distant world positions may still lose precision during world-to-camera transformations.

Large coordinates may still produce catastrophic cancellation.

Rasterization precision and vertex shader arithmetic remain finite.

Thus infinite reversed-Z and camera-relative high-precision transforms solve related but distinct problems.


### Figure C11-1 — Depth mapping and floating-point spacing work together

![[Visual Recall/Math Foundations/C11-1.png]]

RH camera looks down −Z, zero-to-one depth, n=0.1 and finite conventional far=100000; reversed curve uses infinite far. Right: error after rounding depth to binary32 and moving to its next representable value, including rounding and inversion effects. This is not a complete GPU transform-error model.


<!-- end V11.04 -->

<!-- begin V11.05 -->
## A projected point still needs a raster convention

*Handedness, API depth conventions, Y direction, viewport, pixel centres, half-pixel history.*

NDC is still continuous geometry. A viewport and sample convention determine which framebuffer location receives it.

**Left-handed versus right-handed projection conventions**

The handedness of a coordinate system describes the orientation of its basis.

It should not be confused with the depth interval $[0,1]$ versus $[-1,1]$, or with the direction of the raster Y axis.

These are separate design choices.

A conventional right-handed camera frequently uses:

$$
 \boxed{ \text{camera forward}=-Z }. 
$$

A conventional left-handed camera formulation frequently uses:

$$
 \boxed{ \text{camera forward}=+Z }. 
$$

The full interpretation depends on the associated basis convention.

### Right-handed projection

Under our reference convention:

$$
 d=-z_v, 
$$

so the homogeneous depth denominator is:

$$
 \boxed{w_c=-z_v}. 
$$

The final projection row is:

$$
 \boxed{ (0,0,-1,0) }. 
$$

### Conventional left-handed perspective

For a camera using positive view-space $Z$ as forward depth:

$$
 d=z_v>0. 
$$

Choose:

$$
 \boxed{w_c=z_v}. 
$$

Now assume:

$$
 z_c=Az_v+B. 
$$

We want:

$$
 z_n=\frac{Ad+B}{d} =A+\frac Bd. 
$$

For conventional zero-to-one depth:

$$
 d=n\mapsto0, 
$$

$$
 d=f\mapsto1. 
$$

Thus:

$$
 A+\frac Bn=0 
$$

and:

$$
 A+\frac Bf=1. 
$$

Subtract:

$$
 B\left(\frac1n-\frac1f\right)=-1. 
$$

Therefore:

$$
 \boxed{ B=-\frac{nf}{f-n} }. 
$$

Then:

$$
 A=-\frac Bn=\frac f{f-n}. 
$$

The conventional positive-Z-forward perspective matrix becomes:

$$
 \boxed{ P_{\mathrm{LH}}^{[0,1]}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&1&0 \end{bmatrix} }. 
$$

Compare with our right-handed zero-to-one matrix:

$$
 P_{\mathrm{RH}}^{[0,1]}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix}. 
$$

The signs of the depth-related terms differ because the view-space forward-depth convention differs.

### Handedness does not determine NDC depth range

A right-handed camera can use:

$$
 z_n\in[0,1]. 
$$

That is exactly what your KairoMath projection helper does.

A left-handed camera can also use:

$$
 z_n\in[0,1]. 
$$

Likewise, a right-handed camera can target traditional OpenGL:

$$
 z_n\in[-1,1]. 
$$

These are independent axes of configuration.

A projection API should avoid a single vague flag such as:

```
bool isVulkan;
```

that implicitly chooses handedness, depth interval, raster origin, and reversed-Z all at once.

Instead, model the actual independent conventions explicitly.

### Handedness and face orientation

Changing coordinate orientation may reverse winding when geometry passes through the full transform and viewport pipeline.

A triangle that was front-facing under one convention may be classified as back-facing under another if the transformation or raster Y mapping reverses orientation.

Thus changing handedness or flipping projection Y should prompt a check of front-face winding and culling state.

It is not enough to make the camera image visually upright.

**OpenGL clip-depth conventions**

Traditional OpenGL uses a symmetric clip-depth interval.

For a vertex in homogeneous clip space:

$$
 (x_c,y_c,z_c,w_c), 
$$

the default clip volume includes:

$$
 \boxed{-w_c\le z_c\le w_c}. 
$$

After division:

$$
 \boxed{-1\le z_n\le1}. 
$$

This differs from the ordinary Vulkan/Metal zero-to-one depth convention.

### Derive the right-handed OpenGL perspective depth row

Again let:

$$
 z_c=Az_v+B, 
$$

$$
 w_c=-z_v=d. 
$$

Thus:

$$
 z_n=-A+\frac Bd. 
$$

For traditional OpenGL, impose:

$$
 d=n\mapsto-1, 
$$

$$
 d=f\mapsto1. 
$$

The equations are:

$$
 -A+\frac Bn=-1, 
$$

$$
 -A+\frac Bf=1. 
$$

Subtract:

$$
 B\left(\frac1n-\frac1f\right)=-2. 
$$

Therefore:

$$
 B=-\frac{2nf}{f-n}. 
$$

From the far equation:

$$
 -A+\frac Bf=1. 
$$

Thus:

$$
 A=\frac Bf-1. 
$$

Substitute:

$$
 A=-\frac{2n}{f-n}-1. 
$$

Combine:

$$
 \boxed{ A=-\frac{f+n}{f-n} }. 
$$

Therefore:

$$
 \boxed{ P_{\mathrm{OpenGL,default}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-\dfrac{f+n}{f-n}&-\dfrac{2nf}{f-n}\\ 0&0&-1&0 \end{bmatrix} }. 
$$

### Depth-buffer mapping

Traditional OpenGL typically maps NDC depth $[-1,1]$ into the window-depth interval $[0,1]$:

$$
 \boxed{ z_{\mathrm{window}}=\frac{z_n+1}{2} } 
$$

when the depth range is configured to its usual defaults.

This does not mean its clip-depth convention was zero-to-one.

Clipping and post-divide depth representation are different stages.

### Modern OpenGL clip control

Modern desktop OpenGL supports:

```
glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
```

under the appropriate version and capability requirements.

This allows OpenGL to use a zero-to-one clip-depth convention.

The origin can also be changed:

```
glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);
```

This alters clip-control behavior, including the coordinate origin interpretation.

It can be useful for unifying depth conventions with other graphics APIs.

The exact semantics are defined in the OpenGL `glClipControl` specification.&#x20;


### Reversed-Z in OpenGL

For the usual floating-point reversed-Z benefits, a zero-to-one clip-depth convention is preferable.

A suitable pipeline uses a reversed projection mapping, depth clear value zero, and a greater-style depth comparison.

Simply changing the depth comparison operator without changing the projection and depth mapping does not implement reversed-Z correctly.

**Vulkan clip-depth conventions**

Vulkan's ordinary depth-clipping convention uses:

$$
 \boxed{ 0\le z_c\le w_c }. 
$$

After perspective division:

$$
 \boxed{ 0\le z_n\le1 }. 
$$

This is why your KairoMath projection helper is documented as Vulkan/Metal-friendly.

Vulkan's official specification defines the clip-coordinate, perspective-division, and viewport transformation stages explicitly.&#x20;


### What Vulkan does not prescribe

Vulkan does not require your application to use a left-handed world coordinate system.

You can construct a right-handed view matrix and a right-handed projection matrix with zero-to-one depth.

Vulkan also does not require your camera to have a specific field-of-view parameterization.

Those are application choices.

### Viewport depth transform

A Vulkan viewport includes:

```
VkViewport viewport{};
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
```

Under the ordinary zero-to-one mapping, framebuffer depth is:

$$
 \boxed{ z_f= \text{minDepth} + z_n(\text{maxDepth}-\text{minDepth}) }. 
$$

For the usual defaults:

$$
 z_f=z_n. 
$$

Vulkan also permits configurations where the depth-range endpoints are reversed, subject to its specification and enabled features.

But reversing the viewport depth range is not the same as deriving a reversed-Z projection. The combined transformation and precision behavior must be evaluated, especially if the objective is floating-point reversed-Z precision.

### Negative viewport height

Vulkan permits negative viewport height under the relevant core-version or extension support.

This can flip framebuffer Y during viewport mapping.

For example, a viewport can be configured conceptually as:

```
VkViewport viewport{};
viewport.x = 0.0f;
viewport.y = static_cast<float>(height);
viewport.width = static_cast<float>(width);
viewport.height = -static_cast<float>(height);
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
```

The negative height reverses the Y direction of the mapping.

This is one way to align a conventional $+Y$-up mathematical camera projection with a top-left framebuffer interpretation.

If the application already flips Y inside the projection matrix, applying another viewport flip may reverse the image again.

Vulkan's viewport documentation explicitly describes the effect of negative height.&#x20;


### Winding and rasterization

A Y flip changes the orientation of projected triangles.

Therefore front-face definitions such as clockwise versus counterclockwise winding must be checked against the actual framebuffer-space orientation.

A renderer can display an empty scene because every triangle is being culled, even when its position, view, and projection mathematics are otherwise correct.

This is an important debugging distinction.

**Metal clip-depth conventions**

Metal uses a zero-to-one normalized depth convention for ordinary visible geometry:

$$
 \boxed{ 0\le z_n\le1 }. 
$$

Like Vulkan, it can use a right-handed camera model that looks toward local $-Z$.

The world-space handedness and view matrix are application choices.

### Metal projection matrices

For our right-handed zero-to-one convention, the matrix:

$$
 P= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix} 
$$

is mathematically appropriate for ordinary finite perspective projection.

A reversed-Z Metal projection can instead use:

$$
 P_{\mathrm{reverse}}= \begin{bmatrix} s_x&0&0&0\\ 0&s_y&0&0\\ 0&0&\dfrac n{f-n}&\dfrac{nf}{f-n}\\ 0&0&-1&0 \end{bmatrix}. 
$$

Metal accepts clip-space positions from vertex shaders and performs perspective division and viewport mapping as part of its rasterization pipeline. Apple's depth-testing documentation provides a concrete description of this process.&#x20;


### Metal matrix memory conventions

Apple's SIMD matrix types expose column vectors, for example through `simd_float4x4` columns.

This is a storage and API representation detail.

The mathematical transform may still be expressed as:

$$
 \mathbf p'=M\mathbf p. 
$$

When transferring a CPU-side row-major matrix to a shader expecting column-based storage, ensure the memory representation is converted correctly.

But do not indiscriminately transpose every matrix merely because one CPU library uses row-major storage and Metal uses column-based matrix types.

The correct operation depends on both multiplication semantics and memory layout.

A CPU row-major matrix with column-vector multiplication can be semantically identical to a GPU matrix with column-vector multiplication. Only the storage arrangement may require conversion.

### Metal depth states

For conventional depth:

```
// Conceptual Metal settings:
depthClear = 1.0;
compareFunction = Less;
```

For reversed-Z:

```
depthClear = 0.0;
compareFunction = Greater;
```

The actual implementation uses Metal API enumerations and depth-stencil state descriptors.

A proper reversed-Z conversion must also update any shaders that reconstruct depth or use depth comparisons.

### Maveb's existing projection

Your inspected [Maveb Camera.cpp](https://github.com/swayam8624/Maveb/blob/main/engine/scene/src/Camera.cpp?utm_source=chatgpt.com) already implements a right-handed reversed-Z perspective matrix, including an infinite-far-plane option.

Its finite-depth coefficients are:

$$
 \boxed{ A=\frac n{f-n}, \qquad B=\frac{nf}{f-n} }. 
$$

Its infinite-far coefficients become:

$$
 \boxed{ A=0,\qquad B=n }. 
$$

These match our derivations exactly.

Its orthographic code also maps near to one and far to zero under its right-handed view convention.

This is an important distinction between your codebases: Maveb's inspected camera path uses reversed-Z, while KairoMath's general perspective helper implements conventional zero-to-one depth.

That is not automatically a bug, but any shared rendering or geometry utility must know which convention it is receiving.

**Y-axis convention differences across graphics APIs**

This topic causes a disproportionate number of visual bugs.

We must distinguish several separate choices.

A world coordinate system can use $+Y$ up.

A camera coordinate system can use $+Y$ up.

An NDC convention can map positive Y toward a particular side of the viewport.

A framebuffer can use a top-left origin with positive Y downward.

A texture can have its first stored row at the top.

These are separate conventions.

### The mathematical NDC-to-raster choice

For an image with top-left origin, width $W$, height $H$, and positive raster Y downward, a common mapping is:

$$
 \boxed{ x_p=\frac{x_n+1}{2}W } 
$$

and:

$$
 \boxed{ y_p=\frac{1-y_n}{2}H }. 
$$

Under this mapping:

$$
 y_n=1 
$$

maps to raster top:

$$
 y_p=0. 
$$

And:

$$
 y_n=-1 
$$

maps to raster bottom:

$$
 y_p=H. 
$$

### Vulkan's positive-height viewport

Under the standard Vulkan viewport formula:

$$
 y_f=\frac{H}{2}y_n+y_0+\frac H2 
$$

for a positive viewport height $H$.

Thus:

$$
 y_n=-1\mapsto y_0, 
$$

$$
 y_n=1\mapsto y_0+H. 
$$

With a top-left framebuffer interpretation, positive NDC Y maps downward.

To make a conventional $+Y$-up camera render upright using a positive-height Vulkan viewport, the application can negate projected Y:

$$
 \boxed{ y_c'=-y_c }. 
$$

This is the strategy used by your inspected `KairoRenderer/Camera.cppm`:

```
projection(1u, 1u) *= -1.0f;
```

For the symmetric perspective and orthographic matrices in that file, the Y scaling resides in the $(1,1)$ entry, so changing its sign flips projected Y.

For a more general off-axis projection containing additional Y-row terms, negating only the $(1,1)$ entry would not necessarily represent the desired complete Y flip.

### Alternative: negative Vulkan viewport height

Instead of modifying projection, the viewport can perform the Y inversion using negative height.

These strategies should not both be applied unless the intended composition has been explicitly analyzed.

### OpenGL window origin

Traditional OpenGL uses a lower-left origin for window coordinates by default.

Thus a positive NDC Y coordinate normally maps upward in its window coordinate system.

Modern OpenGL's clip-control origin settings can change that behavior.

### Metal viewport interpretation

Metal's raster viewport coordinates are conventionally interpreted with a top-left origin and positive Y downward.

Its viewport transformation accounts for the mapping from the Metal NDC convention to viewport coordinates.

Therefore a Metal renderer should not blindly copy a Vulkan-specific projection-Y flip without checking the complete pipeline.

### Texture coordinates are yet another layer

Suppose your projection is correct and the geometry renders upright.

A texture can still appear upside down because its UV origin differs from the image loader's row-order convention.

That should be fixed in the texture-coordinate or asset-loading contract rather than by modifying the camera projection.

Rule: Never fix an unexplained Y inversion by randomly negating coordinates in several subsystems. Determine which coordinate conversion has the wrong sign.

**The viewport transform**

The viewport transform converts NDC into framebuffer coordinates.

For a generic axis-aligned viewport with:

$$
 x_0,y_0,W,H, 
$$

a mathematical mapping with X increasing right and NDC Y increasing up is:

$$
 \boxed{ x_p=x_0+\frac W2(x_n+1) } 
$$

and, for top-left raster coordinates:

$$
 \boxed{ y_p=y_0+\frac H2(1-y_n) }. 
$$

### Deriving X mapping

We want:

$$
 x_n=-1\mapsto x_0, 
$$

$$
 x_n=1\mapsto x_0+W. 
$$

Assume:

$$
 x_p=ax_n+b. 
$$

The equations give:

$$
 -a+b=x_0, 
$$

$$
 a+b=x_0+W. 
$$

Subtract:

$$
 2a=W. 
$$

Thus:

$$
 a=W/2. 
$$

Then:

$$
 b=x_0+W/2. 
$$

Therefore:

$$
 \boxed{ x_p=x_0+\frac W2(x_n+1) }. 
$$

### Deriving Y mapping

For a top-left raster convention, we want:

$$
 y_n=1\mapsto y_0, 
$$

$$
 y_n=-1\mapsto y_0+H. 
$$

Assume:

$$
 y_p=cy_n+d. 
$$

Then:

$$
 c+d=y_0, 
$$

$$
 -c+d=y_0+H. 
$$

Subtract:

$$
 -2c=H. 
$$

Therefore:

$$
 c=-H/2. 
$$

Then:

$$
 d=y_0+H/2. 
$$

Thus:

$$
 \boxed{ y_p=y_0+\frac H2(1-y_n) }. 
$$

### Viewport offsets

Suppose an editor scene is rendered inside a viewport panel starting at:

$$
 (x_0,y_0)=(200,100) 
$$

with dimensions:

$$
 W=800,\qquad H=600. 
$$

A point at NDC centre:

$$
 (0,0) 
$$

maps to:

$$
 x_p=200+400=600, 
$$

$$
 y_p=100+300=400. 
$$

Thus:

$$
 \boxed{(x_p,y_p)=(600,400)}. 
$$

If a picking routine assumes the viewport starts at $(0,0)$, it will interpret the wrong pixel.

This is a frequent editor bug when the rendering target occupies only part of a larger application window.

### Viewport depth mapping

For zero-to-one depth and normal viewport endpoints:

$$
 \boxed{ z_f=\mathrm{minDepth} + z_n(\mathrm{maxDepth}-\mathrm{minDepth}) }. 
$$

For OpenGL-style negative-one-to-one depth mapped into a conventional zero-to-one window interval:

$$
 \boxed{ z_f=\frac{z_n+1}{2} } 
$$

when the depth range uses the corresponding defaults.

The exact hardware transformation depends on the active API and configured depth range.

### Viewport versus scissor

A viewport defines the geometric mapping from NDC to framebuffer coordinates.

A scissor rectangle restricts which framebuffer pixels may be written by rasterization.

The two are not interchangeable.

A viewport can project geometry outside a scissor region, and the scissor can clip fragment writes without changing the projection mathematics.

For an editor or tiled renderer, both must be configured consistently.

**Screen-space pixel centres**

A raster image contains discrete pixels.

Under the common modern convention, the centre of pixel:

$$
 (i,j) 
$$

lies at continuous raster coordinate:

$$
 \boxed{ (x_p,y_p)=(i+0.5,j+0.5) }. 
$$

The top-left pixel is indexed:

$$
 (0,0). 
$$

Its centre is:

$$
 \boxed{(0.5,0.5)}. 
$$

The next pixel to the right has centre:

$$
 (1.5,0.5). 
$$

This half-unit offset is not a mysterious antialiasing correction. It follows from defining integer coordinates at pixel-cell corners.

### Pixel-centre coordinates in NDC

Assume a full-size top-left raster viewport.

The horizontal viewport mapping is:

$$
 x_p=\frac{x_n+1}{2}W. 
$$

For pixel column $i$, use its centre:

$$
 x_p=i+0.5. 
$$

Solve:

$$
 i+0.5=\frac{x_n+1}{2}W. 
$$

Multiply:

$$
 \frac{2(i+0.5)}W=x_n+1. 
$$

Thus:

$$
 \boxed{ x_n=\frac{2(i+0.5)}W-1 }. 
$$

For Y:

$$
 y_p=\frac{1-y_n}{2}H. 
$$

At row $j$:

$$
 y_p=j+0.5. 
$$

Therefore:

$$
 \boxed{ y_n=1-\frac{2(j+0.5)}H }. 
$$

These formulas convert discrete pixel indices to corresponding NDC sample positions.

### Example: top-left pixel at 1920×1080

For:

$$
 i=0,\qquad j=0, 
$$

we have:

$$
 x_n=\frac1{1920}-1. 
$$

Thus:

$$
 \boxed{x_n\approx-0.999479167}. 
$$

And:

$$
 y_n=1-\frac1{1080}. 
$$

Thus:

$$
 \boxed{y_n\approx0.999074074}. 
$$

The pixel centre is slightly inside the NDC boundaries rather than exactly at $(-1,1)$.

### A subtle result for even-resolution images

Consider a 1920-pixel-wide image.

The geometric centre of the viewport is:

$$
 x_p=960. 
$$

But pixel centres occur at:

$$
 i+0.5. 
$$

The two central pixel centres are:

$$
 959.5 
$$

and:

$$
 960.5. 
$$

There is no pixel whose centre is exactly at:

$$
 960. 
$$

The optical axis of a centred camera passes between the two central columns.

The same occurs vertically for an even image height.

Thus the optical axis in a 1920×1080 image lies between four central pixel centres.

This is not a bug.

For an odd-resolution image, there can be one central pixel whose centre aligns with the optical axis.

### Picking and ray generation

To generate a camera ray through pixel $(i,j)$, we commonly begin with its centre:

$$
 u=\frac{i+0.5}{W}, 
$$

$$
 v=\frac{j+0.5}{H}. 
$$

Subpixel sampling adds jitter:

$$
 u=\frac{i+\xi_x}{W}, 
$$

$$
 v=\frac{j+\xi_y}{H}, 
$$

where:

$$
 \xi_x,\xi_y\in[0,1). 
$$

This connects pixel-sampling strategies with camera projection mathematics.

Screen-space ray generation follows directly from the pixel-centre convention established here.

**Half-pixel conventions and historical issues**

The half-pixel offset is historically confusing because graphics APIs have not always used the same raster-origin convention.

The most important example is the difference between Direct3D 9 and Direct3D 10.

### Direct3D 9 pixel centres

Direct3D 9 traditionally defined the upper-left pixel centre at integer coordinate:

$$
 \boxed{(0,0)}. 
$$

Thus the first pixel centre was not at $(0.5,0.5)$ under that convention.

Applications rendering screen-aligned quads sometimes needed an offset to align geometry with texels.

### Direct3D 10 and later convention

Direct3D 10 changed the raster-coordinate convention so that the upper-left corner of the upper-left pixel is the origin, and the centre lies at:

$$
 \boxed{(0.5,0.5)}. 
$$

Microsoft documents this historical distinction explicitly.&#x20;


### Why blindly adding half a pixel is wrong

Suppose a modern renderer already maps pixel indices using:

$$
 x_p=i+0.5. 
$$

If we apply an additional half-pixel translation copied from an old Direct3D 9 tutorial, the sample location becomes:

$$
 i+1. 
$$

This creates an unintended shift.

For image reconstruction, such a shift can produce blurred textures, edge mismatches, incorrect temporal reprojection, or misaligned screen-space effects.

The correct rule is not:

> Always add half a pixel.

It is:

> Define whether coordinates refer to pixel corners or pixel centres, then perform exactly the conversion that convention requires.

### Texture texel centres

For a texture of width $W$, a conventional normalized UV coordinate for the centre of texel $i$ is:

$$
 \boxed{ u_i=\frac{i+0.5}{W} }. 
$$

For a texture of height $H$:

$$
 \boxed{ v_j=\frac{j+0.5}{H} } 
$$

under a top-left-style normalized indexing convention.

For example, for a $4\times4$ texture, the centres along either dimension are:

$$
 \boxed{ 0.125,\ 0.375,\ 0.625,\ 0.875 }. 
$$

They are not:

$$
 0,\ 0.25,\ 0.5,\ 0.75. 
$$

The latter values correspond to cell boundaries under the usual normalized coordinate mapping.

### Fullscreen triangles and post-processing

Modern renderers frequently use a fullscreen triangle or fullscreen primitive for post-processing.

The shader reconstructs UV coordinates from raster or vertex positions.

If the pipeline already uses the modern pixel-centre convention, adding an old half-texel correction can produce shifted sampling.

For temporal antialiasing, even subpixel shifts matter because motion vectors, previous-frame projection, and image reconstruction depend on consistent sample positions.

This links pixel-centre mathematics to camera jitter and temporal transforms.

### Historical compatibility

When porting old rendering code, inspect whether it was designed for:

- Direct3D 9 integer-centred pixel coordinates.
- Modern half-integer pixel centres.
- OpenGL lower-left framebuffer origin.
- Top-left raster coordinates.
- A shader convention that already applies an offset.
- A texture-loading system with a different row origin.

Do not retain a half-pixel fix merely because it was present in the original source.

The correct transform is determined by the actual pipeline conventions.


<!-- end V11.05 -->

<!-- begin V11.22 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|Next chapter]]
