# 38 — Projecting Ellipsoids into Screen-Space Conics

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/39 - Tiling, Ordering and Compositing at Scale|Next chapter]]

Projecting a 3D ellipsoid requires transforming its center and covariance into camera coordinates. The resulting screen-space conic links 3D shape to a two-dimensional footprint.

**By the end:** Derive the screen-space conic induced by a projected ellipsoid.

**Recall question:** Why does translation affect the projected mean while covariance follows the linear transform?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 84850–87884; source-span SHA256 7aac5dd41cb728fb18cfd25087a00e097736bb750dff3cabbf9941114e72b343. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### FROM A 3D ELLIPSOID TO A SCREEN-SPACE CONIC

This is the mathematical core.

Everything so far built:

$$
\mu_w,
\Sigma_w,
\alpha,
SH.
$$

Now we need a pixel footprint.

---


World Gaussian center:

$$
\mu_w=
\begin{bmatrix}
x_w\\y_w\\z_w
\end{bmatrix}.
$$

World-to-camera rigid transform:

$$
\begin{bmatrix}
R_c&t_c\\
0&1
\end{bmatrix}.
$$

Then:

$$
\boxed{
\mu_c
=
R_c\mu_w+t_c.
}
$$

Write:

$$
\mu_c=
(X,Y,Z)^T.
$$

Maveb's isolated reference API explicitly defines camera as looking along:

$$
\boxed{+Z}
$$

and warns adapters to make renderer-coordinate conversion explicit.

That is excellent API documentation because many renderers use $-Z$ forward.

---

**Translation affects the mean, not covariance**

If:

$$
X'=AX+b,
$$

then:

$$
E[X']
=
A\mu+b.
$$

But covariance:

$$
Cov(X')
=
A\Sigma A^T.
$$

Translation disappears because:

$$
(X'+-\mu')
=
A(X-\mu).
$$

So center transformation uses:

$$
R\mu+t.
$$

Covariance uses only:

$$
R\Sigma R^T.
$$

---


Pinhole camera projection:

$$
u=
f_x\frac{X}{Z}
+
c_x,
$$

$$
v=
f_y\frac{Y}{Z}
+
c_y.
$$

Exactly Maveb:

```cpp
centerX =
    focalX * X / Z +
    centerX;

centerY =
    focalY * Y / Z +
    centerY;
```

---


Intrinsics matrix:

$$
\boxed{
K=
\begin{bmatrix}
f_x&0&c_x\\
0&f_y&c_y\\
0&0&1
\end{bmatrix}.
}
$$

$f_x,f_y$ are in **pixel units**.

If vertical field of view is:

$$
FOV_y
$$

and image height:

$$
H,
$$

then:

$$
\tan(FOV_y/2)
=
\frac{H/2}{f_y}.
$$

Therefore:

$$
\boxed{
f_y=
\frac{H}
{2\tan(FOV_y/2)}.
}
$$

For square pixels:

$$
f_x
$$

can be derived consistently from horizontal FOV or aspect.

If:

$$
FOV_x
=
2\arctan(
aspect\tan(FOV_y/2)
),
$$

then:

$$
f_x=
\frac{W}
{2\tan(FOV_x/2)}.
$$

Under square-pixel/aspect-consistent construction this yields the expected equivalent focal scale.

---

**Principal point**

Usually:

$$
c_x\approx W/2,
$$

$$
c_y\approx H/2.
$$

But calibrated cameras need not have centered principal point.

Maveb's reference camera stores both explicitly:

```text id="30s5gu"
focalX
focalY
centerX
centerY
```

which is the more general model.

---


Under a purely linear transformation:

$$
Y=AX+b,
$$

a Gaussian remains Gaussian:

$$
X\sim\mathcal N(\mu,\Sigma)
$$

implies:

$$
Y
\sim
\mathcal N(
A\mu+b,
A\Sigma A^T
).
$$

Perspective projection is not linear because of:

$$
X/Z,\quad Y/Z.
$$

So it does not globally transform a Gaussian into a mathematically exact Gaussian.

Instead we linearize projection around its center.

Locally:

$$
projection
\approx
affine.
$$

Affine transform of Gaussian is Gaussian.

Therefore the projected footprint is approximated by a 2D Gaussian.

Its constant-density contours are ellipses.

This local affine approximation is the same fundamental machinery used by EWA splatting. [vcg.seas.harvard.edu](https://vcg.seas.harvard.edu/content/3-publications/20010101-surface-splatting/p371-zwicker.pdf?utm_source=chatgpt.com)

---


Let projection:

$$
\mathbf f:
\mathbb R^3
\rightarrow
\mathbb R^2.
$$

For point near mean:

$$
\mathbf x=
\mu+\delta.
$$

Taylor expansion:

$$
\mathbf f(\mu+\delta)
=
\mathbf f(\mu)
+
J_{\mathbf f}(\mu)\delta
+
O(\|\delta\|^2).
$$

Drop higher-order terms:

$$
\boxed{
\mathbf f(\mu+\delta)
\approx
\mathbf f(\mu)
+
J\delta.
}
$$

So local perturbations transform by:

$$
\delta_{screen}
\approx
J\delta_{camera}.
$$

Then covariance follows directly.

---


Projection:

$$
u(X,Y,Z)
=
f_x\frac XZ+c_x.
$$

Compute derivatives:

$$
\frac{\partial u}{\partial X}
=
\frac{f_x}{Z}.
$$

$$
\frac{\partial u}{\partial Y}
=
0.
$$

$$
\frac{\partial u}{\partial Z}
=
-f_x\frac{X}{Z^2}.
$$

Similarly:

$$
v=
f_y\frac YZ+c_y.
$$

So:

$$
\frac{\partial v}{\partial X}=0,
$$

$$
\frac{\partial v}{\partial Y}
=
\frac{f_y}{Z},
$$

$$
\frac{\partial v}{\partial Z}
=
-f_y\frac{Y}{Z^2}.
$$

Therefore:

$$
\boxed{
J=
\begin{bmatrix}
\frac{f_x}{Z}
&
0
&
-\frac{f_xX}{Z^2}
\\
0
&
\frac{f_y}{Z}
&
-\frac{f_yY}{Z^2}
\end{bmatrix}.
}
$$

This equation must become instinctive.

---

**Maveb code is the Jacobian verbatim**

CPU:

```cpp
jacobianX =
{
    fx / Z,
    0,
    -fx * X / Z²
};

jacobianY =
{
    0,
    fy / Z,
    -fy * Y / Z²
};
```

Metal kernel:

```metal
float3 jacobianX =
    float3(
        fx * inverseZ,
        0,
        -fx * X *
         inverseZ * inverseZ);

float3 jacobianY =
    float3(
        0,
        fy * inverseZ,
        -fy * Y *
         inverseZ * inverseZ);
```

There is no mystery abstraction here.

That code is exactly the calculus.

---

**Why projection explodes near the camera**

Terms contain:

$$
\frac1Z
$$

and:

$$
\frac1{Z^2}.
$$

As:

$$
Z\rightarrow0^+,
$$

Jacobian magnitude explodes.

Therefore projected covariance can become enormous.

This is why near-plane culling is not only geometrically meaningful but numerically important.

---

**Original GraphDECO adds another stabilization near FOV boundaries**

The canonical CUDA implementation clamps normalized projected $x/z$ and $y/z$ to a slightly expanded field-of-view range before constructing the covariance Jacobian. It also adds $0.3$ to the 2D covariance diagonal. [GitHub](https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu?utm_source=chatgpt.com)

Maveb currently matches the $+0.3$ regularization but does not perform that same 1.3× tangent-FOV clamp in the inspected CPU/Metal projection code.

That is not automatically wrong—the implementations can define their own valid projection domain—but it is a meaningful difference from GraphDECO when stress-testing very large/off-axis splats.

---


World covariance:

$$
\Sigma_w.
$$

Camera linear rotation:

$$
R_c.
$$

Then:

$$
\boxed{
\Sigma_c
=
R_c
\Sigma_w
R_c^T.
}
$$

Why?

Let:

$$
\delta_c
=
R_c\delta_w.
$$

Then:

$$
E[
\delta_c
\delta_c^T
]
=
E[
R_c
\delta_w
\delta_w^T
R_c^T
].
$$

Move constants out:

$$
=
R_c
E[
\delta_w\delta_w^T
]
R_c^T.
$$

Therefore:

$$
=
R_c\Sigma_wR_c^T.
$$

---

**More general affine transform**

If world-to-camera upper-left block is not a pure rotation but a general linear matrix:

$$
A,
$$

then covariance transformation is still:

$$
\boxed{
A\Sigma A^T.
}
$$

The Maveb CPU helper actually multiplies by the transform's upper 3×3 linear block, so mathematically it supports this more general affine covariance transform.

The GPU variable is called `cameraRotation`, implying the intended camera transform is rigid.

---


After camera covariance:

$$
\Sigma_c,
$$

and local projection Jacobian:

$$
J\in\mathbb R^{2\times3},
$$

screen covariance is:

$$
\boxed{
\Sigma_{2D}
=
J
\Sigma_c
J^T.
}
$$

Dimension check:

$$
(2\times3)
(3\times3)
(3\times2)
=
2\times2.
$$

Correct.

---

**Derivation from local perturbations**

We have:

$$
\delta_s
\approx
J\delta_c.
$$

Screen covariance:

$$
E[
\delta_s
\delta_s^T
].
$$

Substitute:

$$
=
E[
J\delta_c
\delta_c^T
J^T
].
$$

Pull $J$ out:

$$
=
J
E[
\delta_c\delta_c^T
]
J^T.
$$

Therefore:

$$
\boxed{
\Sigma_s=
J\Sigma_cJ^T.
}
$$

That is the entire derivation.

---

**Expanded implementation**

Let Jacobian rows be:

$$
j_x,
j_y.
$$

Then:

$$
\Sigma_s=
\begin{bmatrix}
j_x\Sigma_cj_x^T
&
j_x\Sigma_cj_y^T
\\
j_y\Sigma_cj_x^T
&
j_y\Sigma_cj_y^T
\end{bmatrix}.
$$

Because covariance symmetric:

$$
j_x\Sigma j_y^T
=
j_y\Sigma j_x^T.
$$

Maveb calculates exactly:

$$
a=j_x\Sigma j_x^T,
$$

$$
b=j_x\Sigma j_y^T,
$$

$$
c=j_y\Sigma j_y^T.
$$

---


Maveb adds:

$$
0.3
$$

to both diagonal entries:

$$
a\leftarrow a+0.3,
$$

$$
c\leftarrow c+0.3.
$$

Thus:

$$
\boxed{
\Sigma_s'
=
\Sigma_s
+
0.3I.
}
$$

The canonical GraphDECO rasterizer uses the same constant and explicitly describes it as a low-pass stabilization ensuring Gaussians retain a minimum screen footprint. [GitHub](https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu?utm_source=chatgpt.com)

---

**Why adding $\lambda I$ helps**

Eigenvalues of:

$$
\Sigma
$$

are:

$$
\lambda_i.
$$

Eigenvalues of:

$$
\Sigma+\epsilon I
$$

become:

$$
\lambda_i+\epsilon.
$$

So the smallest eigenvalue can no longer collapse below $\epsilon$, ignoring numerical roundoff.

That improves:

- invertibility,
- minimum pixel footprint,
- stability,
- anti-aliasing behavior.

---

**What $0.3$ means**

Variance:

$$
0.3\text{ pixels}^2.
$$

Corresponding standard deviation:

$$
\sqrt{0.3}
\approx0.548\text{ pixels}.
$$

A 3σ radius from that term alone is:

$$
3\sqrt{0.3}
\approx1.64\text{ pixels}.
$$

So even an extremely tiny projected splat retains a nonzero footprint.

---


We now have:

$$
\boxed{
\Sigma_{2D}
=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix}.
}
$$

Interpretation:

$$
a=\sigma_x^2,
$$

$$
c=\sigma_y^2,
$$

while:

$$
b
$$

describes correlation/rotation.

If:

$$
b=0,
$$

ellipse axes align with screen axes.

If:

$$
b\ne0,
$$

ellipse is rotated.

---

**Example**

Suppose:

$$
\Sigma=
\begin{bmatrix}
9&0\\
0&1
\end{bmatrix}.
$$

Standard deviations:

$$
3,\ 1.
$$

So one-sigma ellipse has radii:

$$
3,\ 1.
$$

Three-sigma:

$$
9,\ 3.
$$

If we rotate covariance 45°, diagonal entries/off-diagonal terms change, while eigenvalues remain:

$$
9,\ 1.
$$

Same ellipse shape.

Different screen orientation.

---


For:

$$
\Sigma=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix},
$$

determinant:

$$
\boxed{
D=ac-b^2.
}
$$

For SPD 2×2 covariance we need:

$$
a>0
$$

and:

$$
D>0.
$$

Geometrically, determinant relates to ellipse area.

Product of eigenvalues:

$$
\lambda_1\lambda_2=D.
$$

One-sigma ellipse area:

$$
A=
\pi
\sqrt{\lambda_1}
\sqrt{\lambda_2}
=
\boxed{
\pi\sqrt{D}.
}
$$

So determinant approaching zero means the ellipse collapses toward a line.

---


For:

$$
A=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix},
$$

inverse is:

$$
\boxed{
A^{-1}
=
\frac1{ac-b^2}
\begin{bmatrix}
c&-b\\
-b&a
\end{bmatrix}.
}
$$

Proof:

$$
A
\begin{bmatrix}
c&-b\\
-b&a
\end{bmatrix}
=
\begin{bmatrix}
ac-b^2&0\\
0&ac-b^2
\end{bmatrix}.
$$

Divide by determinant.

---


Rasterization does not need covariance itself at every pixel.

It needs:

$$
\Sigma^{-1}.
$$

Define conic:

$$
C=
\Sigma^{-1}
=
\begin{bmatrix}
A&B\\
B&C
\end{bmatrix}.
$$

Maveb stores:

$$
A=
c/D,
$$

$$
B=
-b/D,
$$

$$
C=
a/D.
$$

Its struct names are:

```text id="cgxb8q"
inverseA
inverseB
inverseC.
```

---

**Pixel evaluation**

Let pixel center be:

$$
p=
(x+0.5,y+0.5).
$$

Projected Gaussian center:

$$
\mu_s.
$$

Then:

$$
\Delta=
\begin{bmatrix}
dx\\dy
\end{bmatrix}
=
p-\mu_s.
$$

Mahalanobis squared distance:

$$
q=
\Delta^T
C
\Delta.
$$

Expand:

$$
\boxed{
q=
A\,dx^2
+
2B\,dx\,dy
+
C\,dy^2.
}
$$

Maveb CPU oracle uses exactly this expression.

---

**Gaussian screen weight**

Then:

$$
\boxed{
w=
e^{-q/2}.
}
$$

At center:

$$
q=0,
$$

so:

$$
w=1.
$$

On one-sigma contour:

$$
q=1,
$$

$$
w=e^{-1/2}.
$$

On three-sigma contour:

$$
q=9,
$$

$$
w=e^{-4.5}.
$$

Maveb CPU oracle rejects:

$$
q>9.
$$

So it evaluates only the 3σ ellipse inside its conservative square raster bound.

#### First-use completion — from one Gaussian's footprint to the final pixel

The ellipse test $q\leq9$ only determines whether a projected Gaussian is a *candidate contributor*. It does not yet calculate the final pixel, because several Gaussians can overlap the same sample and obscure each other. For a projected Gaussian $i$, let $q_i=(x-\mu_i)^T\Sigma_{2D,i}^{-1}(x-\mu_i)$ be its squared elliptical distance at pixel sample $x$; $\mu_i$ is its projected center and $\Sigma_{2D,i}$ is the screen covariance, assumed positive definite. The unnormalized weight is $g_i(x)=e^{-q_i/2}$. If its stored opacity is $o_i\in[0,1]$, a common splatting convention defines the effective sample opacity as
$$
\alpha_i(x)=\min(\alpha_{\max},\,o_i e^{-q_i/2}),
$$
where $\alpha_{\max}<1$ may be a deliberate numerical clamp. The cutoff and clamp are separate approximations: $q_i>9$ skips the contribution, whereas the clamp caps the contribution of a retained Gaussian. Neither operation turns the Gaussian into an opaque mesh surface.

With overlapping splats, order them from **near to far using one declared camera-depth convention**. Reversed-Z may encode near objects with a larger stored depth, so blindly sorting an API depth value ascending is not universally front-to-back. For the sorted contributors, let $T_0=1$ be the fraction of light not yet blocked, and let $C_0=(0,0,0)$ be accumulated *linear* RGB. For the next Gaussian:
$$
C_{i+1}=C_i+T_i\alpha_i c_i,\qquad
T_{i+1}=T_i(1-\alpha_i).
$$
Here $c_i$ is its view-dependent RGB contribution in the declared linear working space. A background $C_{bg}$ enters *after* the last contributor as $C=C_n+T_n C_{bg}$. This is front-to-back premultiplied-alpha compositing written without pretending that stored Gaussian colors were already premultiplied. The product form
$$
T_i=\prod_{j<i}(1-\alpha_j)
$$
explains the recurrence: a farther Gaussian contributes only through the surviving transmittance from every nearer one.

For two overlapping center samples, take a near **red** splat with $\alpha_0=0.6$, and a far **blue** splat with $\alpha_1=0.5$. Start with $T=1,C=(0,0,0)$. Red contributes $(0.6,0,0)$ and leaves $T=0.4$. Blue contributes only $0.4(0.5)(0,0,1)=(0,0,0.2)$, leaving $T=0.2$. The result before background is $C=(0.6,0,0.2)$. Reverse the order and you obtain $C=(0.3,0,0.5)$, still with $T=0.2$. Thus sorting is a **visible mathematical requirement**, not an optional optimization. Overlapping equal-depth or intersecting splats require a declared approximation/tie policy; one global center-depth ordering cannot reproduce all possible volumetric occlusion.

```cpp
#include <array>
#include <algorithm>
#include <cmath>
#include <span>

struct SplatSample {
    double ellipticalDistanceSquared; // q, already computed in screen space.
    double opacity;                    // Stored amplitude in [0,1].
    std::array<double, 3> linearRGB;   // At this viewing direction.
};

struct PixelState {
    std::array<double, 3> linearRGB{0.0, 0.0, 0.0};
    double transmittance = 1.0;
};

// Caller guarantees near-to-far order, finite inputs and valid opacities.
// This CPU reference deliberately shows the exact sequential dependency.
PixelState composite_front_to_back(std::span<const SplatSample> sorted)
{
    PixelState pixel;
    for (const SplatSample& s : sorted) {
        const double q = s.ellipticalDistanceSquared;
        if (q > 9.0) continue; // Explicit 3-sigma squared-distance policy.
        const double alpha = std::clamp(s.opacity * std::exp(-0.5 * q),
                                        0.0, 0.999);
        for (int channel = 0; channel < 3; ++channel)
            pixel.linearRGB[channel] +=
                pixel.transmittance * alpha * s.linearRGB[channel];
        pixel.transmittance *= (1.0 - alpha);
        if (pixel.transmittance < 1e-4) break; // Declared approximation.
    }
    return pixel; // Add T * background before display encoding.
}
```

Why stop at a small $T$? If every remaining source and the background have channels in $[0,1]$, the **maximum missing residual contribution** in any channel is bounded by the surviving $T$. Stopping at $10^{-4}$ therefore gives a conditional channel-error bound of at most $10^{-4}$ in that working range; unbounded HDR radiance has **no such numerical error bound without an additional radiance limit**. Likewise, $q\leq9$ encloses $1-e^{-9/2}\approx98.89\%$ of the total probability mass of an ideal normalized two-dimensional Gaussian. That is not a guarantee about image error: learned amplitudes, overlap, colors, sorting and pixel footprints affect the actual rendered result. Keep the cutoff a measured quality/performance parameter.

A GPU tile renderer can assign Gaussian references to screen tiles and sort the tile's references by depth, but it must still evaluate each pixel's $q_i$, opacity, RGB and the recurrence above. The tile list is a conservative candidate structure, not a composited pixel. A prefix sum builds disjoint reference ranges; sorting and sequential transmittance then answer a different question. For correctness, compare a scalar CPU oracle against the GPU's color and $T$ on tiny scenes with one splat, two reversed-order splats, zero opacity, footprint cutoff, overlapping centers, invalid covariance and early termination. The [first-use C++ witness](../../tools/labs/first_use_contract_lab.cpp) checks the two-splat numeric example. Source claims about a particular Maveb revision require separately inspected code; this section teaches the algorithm itself rather than asserting a current implementation audit.

---


Solve:

$$
det(
\Sigma-\lambda I
)
=0.
$$

For:

$$
\Sigma=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix},
$$

$$
(a-\lambda)
(c-\lambda)
-b^2=0.
$$

Expand:

$$
\lambda^2
-
(a+c)\lambda
+
(ac-b^2)
=0.
$$

Quadratic formula:

$$
\boxed{
\lambda_{\pm}
=
\frac{
(a+c)
\pm
\sqrt{
(a-c)^2+4b^2
}
}{2}.
}
$$

Maveb computes:

```cpp
discriminant =
    sqrt(
      max(
        0,
        (a-c)^2 +
        4b^2));

maximumEigenvalue =
    0.5 *
    (a+c+discriminant);
```

Exactly:

$$
\lambda_{max}.
$$

---

**Eigenvector orientation**

For debugging or tight oriented geometry, ellipse angle can be obtained from:

$$
\boxed{
\theta=
\frac12
atan2(
2b,
a-c
).
}
$$

A major-axis unit vector:

$$
\mathbf e_{max}
=
(\cos\theta,\sin\theta)
$$

with sign/order adjusted according to which eigenvalue it corresponds to.

Maveb's simple CPU oracle does not need this because it uses an isotropic square bound based on $\lambda_{max}$.

Vulkax's projected representation does preserve major/minor oriented axes for raster geometry, so eigenvector interpretation becomes operational there.

---


Principal standard deviations:

$$
\sigma_1=
\sqrt{\lambda_1},
$$

$$
\sigma_2=
\sqrt{\lambda_2}.
$$

At cutoff $k$:

$$
\boxed{
r_1=
k\sqrt{\lambda_1},
\quad
r_2=
k\sqrt{\lambda_2}.
}
$$

For Maveb:

$$
k=3.
$$

Thus maximum conservative radius:

$$
\boxed{
r=
3\sqrt{\lambda_{max}}.
}
$$

Current CPU and Metal implementations use exactly this.

---


Maveb uses square:

$$
[x_c-r,x_c+r]
\times
[y_c-r,y_c+r].
$$

This contains the entire 3σ ellipse because every directional variance is bounded by:

$$
\lambda_{max}.
$$

Formally for unit $\mathbf u$:

$$
\mathbf u^T
\Sigma
\mathbf u
\le
\lambda_{max}.
$$

Hence no ellipse direction extends beyond:

$$
3\sqrt{\lambda_{max}}.
$$

---

**A tighter axis-aligned bound is possible**

For ellipse:

$$
\Delta^T
\Sigma^{-1}
\Delta
\le k^2,
$$

maximum x coordinate displacement is:

$$
\boxed{
|dx|_{max}
=
k\sqrt{a}.
}
$$

Maximum y:

$$
\boxed{
|dy|_{max}
=
k\sqrt{c}.
}
$$

So tighter AABB can use:

$$
r_x=3\sqrt a,
$$

$$
r_y=3\sqrt c.
$$

The $\lambda_{max}$ square is simpler and always conservative but can cause extra raster/tile work for very elongated Gaussians.

That performance distinction becomes important in Pass 16.

---

**Why conservative bounds are required**

A false positive bound:

> rasterize some pixels where Gaussian ultimately has negligible/no support

costs performance.

A false negative bound:

> fail to rasterize a pixel with valid contribution

changes the rendered image.

Therefore:

$$
\boxed{
\text{bounding must err outward, never inward}.
}
$$

---


Maveb's reference renderer uses:

$$
Z<nearPlane
$$

or:

$$
Z>farPlane
$$

to reject.

Because reference camera looks along:

$$
+Z.
$$

This means:

$$
Z\le0
$$

is behind or at camera.

Near plane additionally prevents unstable:

$$
1/Z
$$

and:

$$
1/Z^2.
$$

---

**Large Gaussian crossing near plane**

There is a subtle approximation.

A Gaussian center might satisfy:

$$
Z<near
$$

while its covariance volume extends into visible space.

Center culling would remove the whole primitive.

This is not a mathematically exact clipping of the Gaussian volume.

Most real 3DGS rasterizers rely on point-center/frustum-style visibility approximations because splats are generally sufficiently localized.

If you use gigantic Gaussians near the camera, this becomes a potential artifact.

That is a good stress test for Maveb.

---


With screen center:

$$
(c_x,c_y)
$$

and radius:

$$
r,
$$

entire Gaussian is left of screen if:

$$
c_x+r<0.
$$

Entirely above:

$$
c_y+r<0.
$$

Entirely right if:

$$
c_x-r\ge W.
$$

Entirely below:

$$
c_y-r\ge H.
$$

Maveb Metal performs exactly these four tests.

If any true:

$$
tileCount=0
$$

and projected record is invalid.

---

**Then clamp the surviving bounds**

For visible splat:

$$
x_{min}
=
\max(
0,
\lfloor c_x-r\rfloor
),
$$

$$
x_{max}
=
\min(
W-1,
\lceil c_x+r\rceil
).
$$

Similarly y.

The GPU then converts those pixel coordinates into tile bounds.

That last step belongs conceptually to Part 2453, so we stop before teaching the tile scheduling algorithm itself.

---


After low-pass regularization:

$$
D=
ac-b^2.
$$

Maveb checks:

$$
D\le10^{-12}
$$

or non-finite.

If so, reject.

Why?

Because conic needs:

$$
1/D.
$$

A tiny determinant produces huge coefficients and numerical instability.

---

**Determinant threshold alone is not a complete condition-number test**

Suppose:

$$
\lambda_1=10^{12},
$$

$$
\lambda_2=10^{-6}.
$$

Then:

$$
det=
10^6,
$$

which looks large.

But condition number:

$$
\kappa=
\frac{\lambda_1}{\lambda_2}
=
10^{18}.
$$

The matrix is terribly ill-conditioned.

For hostile inputs, a stronger test considers:

$$
\lambda_{min},
$$

or a scale-relative criterion such as:

$$
\boxed{
det
>
\epsilon
(trace(\Sigma))^2.
}
$$

The $+0.3I$ regularization already greatly limits this problem for ordinary 3DGS data.

---

**Radius sanity**

Maveb CPU oracle additionally rejects:

$$
r>10^6
$$

as unsafe.

This protects the reference implementation from a pathological Gaussian causing effectively unbounded raster work.

The inspected Metal projection kernel checks:

$$
isfinite(r)
$$

and:

$$
r>0
$$

plus screen overlap, but does not contain that same explicit $10^6$ upper bound.

That means hostile/extreme covariance cases can have slightly different failure behavior between CPU and GPU paths.

For ordinary learned assets this probably does not matter, but a correctness oracle should have an explicit stress case for it.

---


At every screen point:

$$
0<w\le1.
$$

Thus:

$$
\alpha(p)
=
\alpha_0w(p)
\le
\alpha_0.
$$

This gives a very useful proof.

If:

$$
\alpha_0<\alpha_{threshold},
$$

then:

$$
\alpha(p)
<
\alpha_{threshold}
$$

for **every pixel**.

Therefore the Gaussian can be culled before projection/raster scheduling with no effect under that thresholding policy.

---

**Maveb's current behavior**

Maveb CPU reference does not currently projection-cull based solely on base opacity.

It projects the Gaussian, then during per-pixel evaluation computes:

$$
\alpha
=
\min(
0.99,
\alpha_0
e^{-q/2}
)
$$

and skips if:

$$
\boxed{
\alpha<
1/255.
}
$$

Because:

$$
w\le1,
$$

a Gaussian with:

$$
\alpha_0<1/255
$$

will necessarily contribute nothing anywhere.

So moving that check into projection would be a mathematically safe optimization relative to the existing per-pixel threshold.

---

**Vulkax already models opacity as a projection cull reason**

Its native projection result accounting explicitly distinguishes:

```text id="qdflga"
Visible
Opacity
BehindCamera
OutsideImage
```

and validates:

$$
InputSplats
=
Visible
+
CulledOpacity
+
CulledBehind
+
CulledOutside.
$$

That is a useful diagnostic model.

Maveb could expose the same category counters in its projection pipeline if detailed visibility telemetry becomes useful.

---

### THE COMPLETE PASS-15 TRANSFORMATION

You should now be able to start from one PLY row and reach the screen-space ellipse without hand-waving.

Stored Gaussian:

$$
\boxed{
G=
(
\mu,
\ell,
q,
o,
SH
).
}
$$

Decode scales:

$$
\boxed{
s=e^\ell.
}
$$

Normalize quaternion:

$$
\boxed{
q\leftarrow
q/\|q\|.
}
$$

Construct rotation:

$$
R=R(q).
$$

Construct 3D covariance:

$$
\boxed{
\Sigma_w
=
R
diag(s_x^2,s_y^2,s_z^2)
R^T.
}
$$

Decode opacity:

$$
\boxed{
\alpha_0=
sigmoid(o).
}
$$

Evaluate view-dependent SH color:

$$
\boxed{
C(\mathbf d)
=
\max
\left(
0,
0.5+
\sum_i
c_iY_i(\mathbf d)
\right).
}
$$

Transform center:

$$
\boxed{
\mu_c=
R_c\mu_w+t_c
=
(X,Y,Z).
}
$$

Cull invalid depth.

Project center:

$$
\boxed{
u=
f_xX/Z+c_x,
\qquad
v=
f_yY/Z+c_y.
}
$$

Transform covariance:

$$
\boxed{
\Sigma_c=
R_c\Sigma_wR_c^T.
}
$$

Construct Jacobian:

$$
\boxed{
J=
\begin{bmatrix}
f_x/Z&0&-f_xX/Z^2\\
0&f_y/Z&-f_yY/Z^2
\end{bmatrix}.
}
$$

Project covariance:

$$
\boxed{
\Sigma_{2D}
=
J\Sigma_cJ^T.
}
$$

Regularize:

$$
\boxed{
\Sigma'_{2D}
=
\Sigma_{2D}
+
0.3I.
}
$$

Write:

$$
\Sigma'=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix}.
$$

Check:

$$
\boxed{
D=ac-b^2>0.
}
$$

Invert:

$$
\boxed{
C=
\frac1D
\begin{bmatrix}
c&-b\\
-b&a
\end{bmatrix}.
}
$$

Largest eigenvalue:

$$
\boxed{
\lambda_{max}
=
\frac{
a+c+
\sqrt{(a-c)^2+4b^2}
}{
2
}.
}
$$

Three-sigma conservative radius:

$$
\boxed{
r=
3\sqrt{\lambda_{max}}.
}
$$

Cull if screen rectangle does not intersect viewport.

What survives is a projected splat containing approximately:

```cpp id="d8pb5g"
struct ProjectedGaussian
{
    float2 center;
    float depth;

    float conicA;
    float conicB;
    float conicC;

    float radius;

    float opacity;
    float3 color;

    uint sourceIdentity;
};
```

#### First-use completion — schedule the projected ellipse into actual tile work

We already know the screen-space center, conic, bounding radius, depth, color and source identity. A projected Gaussian is now a *bounded screen-space workload*. The remaining questions are concrete: which pixels might it affect, how do GPU threads find the Gaussians relevant to their tile without testing every Gaussian in the scene, and how do those contributors reach the pixel in an order suitable for transmittance compositing? We can answer them here, without deferring tiling and sorting to another numbered pass.

Let a render target have width $W$, height $H$, and square tile width $T$ pixels. Give tiles integer coordinates $(t_x,t_y)$, with $0\le t_x<\lceil W/T\rceil$ and $0\le t_y<\lceil H/T\rceil$. The conservative circular bound derived from the screen covariance is a circle of radius $r=3\sqrt{\lambda_{\max}}$ around projected center $(u,v)$. The actual Gaussian ellipse may cover much less area; the circle is useful because it allows cheap rejection without missing the entire three-sigma ellipse. Clip its screen rectangle to the viewport. If the rectangle is empty, the Gaussian contributes no tile references.

For a nonempty clipped rectangle, conservatively map the minimum and maximum screen coordinates to tile indices by dividing by $T$ and taking the floor, clamped to valid tile coordinates. The exact upper-bound formula depends on whether the rectangle uses inclusive or exclusive pixel bounds. Never mix conventions: using an exclusive upper bound with an inclusive floor can either generate an unnecessary extra tile or, worse, omit a boundary sample after a mismatched clamp. Conservative overcount only wastes work; undercount removes contributions.

For example, let $W=H=64$, $T=16$, the center be $(20,20)$ and radius be $10$ pixels. Its bound runs from coordinate $10$ through $30$ on each axis, so it intersects tile columns 0 and 1 and tile rows 0 and 1. It emits four references, one each for tiles $(0,0),(1,0),(0,1),(1,1)$. Those references do *not* copy the full Gaussian record: they name the Gaussian (by a stable index or validated handle) and the tile that may need it.

A scalable implementation is a count–scan–scatter pipeline. In the **count** stage each visible Gaussian reports how many tile references it requires. Suppose three Gaussians request counts $[2,0,1]$. An **exclusive scan** constructs offsets $[0,2,2,3]$. The first Gaussian owns output slots 0 and 1, the second owns an empty range beginning and ending at 2, and the third owns slot 2. The last offset is the required capacity $K=3$. This is a memory-allocation proof: before scattering, the producer knows how many references exist and which nonoverlapping range belongs to each Gaussian. A parallel GPU scan must establish that these offsets are ready before the scatter dispatch reads them. A workgroup barrier alone does not complete a cross-workgroup scan.

```text
# Read-only projected records, already validated and viewport-clipped.
for each gaussian i:
    tile_rectangle[i] = conservative_tile_range(center[i], radius[i])
    count[i] = number_of_tiles(tile_rectangle[i])

offset[0] = 0
for i in [0, gaussian_count):
    offset[i+1] = checked_add(offset[i], count[i])

K = offset[gaussian_count]          # Reject overflow or resource-budget excess.
references = allocate(K)            # No kernel may write past K.

for each gaussian i:
    output = offset[i]
    for each tile in tile_rectangle[i]:
        references[output++] = (tile_id(tile), depth_key(i), source_id(i))
    assert(output == offset[i+1])   # A CPU witness; GPU uses validation counters.

stable_sort(references, by tile_id then near_to_far_depth then tie_policy)
for each tile:
    for each pixel sample in tile:
        C = (0,0,0); transmittance = 1
        for each reference of this tile in the established depth order:
            compute conic distance q for this pixel and referenced Gaussian
            if q > cutoff: continue
            alpha = opacity * exp(-q/2), with declared clamping
            C += transmittance * alpha * linear_color
            transmittance *= (1-alpha)
            if transmittance < cutoff_T: break
        C += transmittance * linear_background
```

This pseudocode makes each intermediate state explicit. **Count** is one integer per input Gaussian. **Scan** owns a prefix array of $N+1$ entries. **Scatter** writes exactly $K=\sum_i count_i$ references. **Sort** groups those references by tile and establishes the compositing order. **Raster** evaluates the actual conic at individual pixel samples, eliminating false positives left by the conservative tile bound. The tile list is therefore an acceleration structure for contributors, not a final visibility answer.

A naive renderer considers all $N$ splats for each of the $M$ tiles, producing $O(NM)$ potential pair tests. A tiled pipeline instead emits $K$ actual overlap references and sorts or bins them. A small compact Gaussian can touch very few tiles; a giant close-up Gaussian can touch nearly all of them. This changes both the asymptotic work and the memory budget. Use a counter width that can represent the total $K$, check multiplication/addition overflow, and set a deliberate policy for unreasonably large footprints. Prefix-sum and sorting are not free, so on small workloads the simplest CPU or GPU path may still be faster. Measure count, scan, scatter, sort and raster time separately.

**Depth ordering is not implied by the GPU's stored depth convention.** Choose a monotone near-to-far key in the declared camera space, then encode or compare it consistently. A reversed-Z depth buffer, a positive-Z computer-vision camera and a negative-Z graphics camera may all represent the same physical scene with different numeric depth order. At equal depths use a deterministic tie policy if reproducibility matters, while recognizing that per-Gaussian center-depth sorting is only an approximation when spatially extended kernels intersect.

Finally, the dependency graph is executable, not decorative: projected records must be ready before count; scan must finish before scatter; references must be complete and sorted before raster; and output attachments must survive until the final consumer. The source Gaussian buffer must remain allocated while references still contain its indices. A resize invalidates tile geometry and may require reallocation. An edit that changes Gaussian count, covariance, opacity or camera pose requires rebuilding the affected candidate ranges; reusing old ranges without a proof of conservativeness can silently remove pixels.

The immediate reconstruction test is now possible: for the 64-by-64 target, independently enumerate the four tile coordinates of the $(20,20)$ radius-10 Gaussian, verify the count–scan–scatter ranges on $[2,0,1]$, and compare the final two-Gaussian pixel against the earlier front-to-back CPU oracle. This proves separate facts about indexing, memory safety, order and color rather than accepting one plausible-looking screenshot as evidence for every stage.

---

### HOW THIS MAPS TO MAVEB'S ACTUAL METAL KERNEL

The current `aetherGaussianProject` kernel essentially follows the derivation line-for-line.

Input:

```text id="pcn1vt"
AetherGaussianGpu
```

contains center, log-scale, quaternion, opacity logit and SH.

Then:

```metal id="3sxvec"
cameraPoint =
    worldToCamera *
    float4(position,1);
```

implements:

$$
\mu_c.
$$

Then:

```metal id="9a8s32"
scales =
    exp(logScale);
```

implements:

$$
s=e^\ell.
$$

Then:

```metal id="d32mgd"
worldCovariance =
    R *
    diag(s²) *
    transpose(R);
```

implements:

$$
\Sigma_w.
$$

Then:

```metal id="bwtx1c"
covariance =
    cameraRotation *
    worldCovariance *
    transpose(cameraRotation);
```

implements:

$$
\Sigma_c.
$$

Then two Jacobian row vectors exactly implement:

$$
J.
$$

Then:

```metal id="bd7vol"
a =
    dot(jx, covariance*jx)
    + 0.3;

b =
    dot(jx, covariance*jy);

c =
    dot(jy, covariance*jy)
    + 0.3;
```

implements:

$$
J\Sigma J^T+0.3I.
$$

Then:

```metal id="tf2x2w"
det =
    a*c-b*b;
```

then conic:

```text id="qebz4r"
c/det
-b/det
a/det
```

then:

$$
3\sqrt{\lambda_{max}}
$$

radius.

Then it computes screen center and culls offscreen.

Then it stores sigmoid opacity and evaluated SH color.

So this is not merely theory relevant to Maveb.

It is literally the mathematical explanation of the currently executing GPU instructions.

---

### CPU ORACLE VERSUS METAL PROJECTION

The two paths agree on the critical mathematical chain.

| Stage | CPU `ReferenceRasterizer` | Metal `gaussian.metal` |
|---|---|---|
| Log scale decode | `exp(logScale)` | `exp(logScale)` |
| Scale→variance | $s^2$ | $s^2$ |
| Quaternion | normalized on import | normalized again |
| 3D covariance | $R\,diag(s^2)R^T$ | same |
| Camera covariance | $R_c\Sigma R_c^T$ | same |
| Projection | $fX/Z+c$ | same |
| Jacobian | analytic | analytic |
| Low-pass | +0.3 diagonal | +0.3 diagonal |
| Conic | closed-form inverse | closed-form inverse |
| Radius | $3\sqrt{\lambda_{max}}$ | same |
| SH | degree 0–3 | degree 0–3 |
| Opacity | sigmoid clamp ±30 | sigmoid clamp ±30 |
| Offscreen cull | during raster bound generation | projection kernel |
| Extreme radius guard | $r\le10^6$ | no matching explicit upper cap in inspected kernel |
| Pixel compositing | CPU oracle | later Metal composite kernel |

That last separation is important.

`aetherGaussianProject` does **not render pixels yet**.

It turns expensive 3D primitive representation into compact screen-space data.

---

### A NUMERICAL EXAMPLE

Suppose a Gaussian in camera space has:

$$
\mu_c=
(1,0.5,5).
$$

Use:

$$
f_x=f_y=1000.
$$

Principal point:

$$
(960,540).
$$

Screen center:

$$
u=
1000\frac15+960
=
1160.
$$

$$
v=
1000\frac{0.5}{5}+540
=
640.
$$

Now Jacobian:

$$
J=
\begin{bmatrix}
200&0&-40\\
0&200&-20
\end{bmatrix}.
$$

Suppose camera covariance is isotropic:

$$
\Sigma_c=
0.01I.
$$

Then:

$$
\Sigma_s=
J(0.01I)J^T
=
0.01JJ^T.
$$

Compute:

$$
JJ^T
=
\begin{bmatrix}
200^2+40^2
&
(-40)(-20)
\\
(-40)(-20)
&
200^2+20^2
\end{bmatrix}.
$$

So:

$$
JJ^T=
\begin{bmatrix}
41600&800\\
800&40400
\end{bmatrix}.
$$

Thus:

$$
\Sigma_s=
\begin{bmatrix}
416&8\\
8&404
\end{bmatrix}.
$$

Add low-pass:

$$
\Sigma'_s=
\begin{bmatrix}
416.3&8\\
8&404.3
\end{bmatrix}.
$$

Determinant:

$$
D=
416.3(404.3)-64.
$$

Approximately:

$$
D\approx168236.
$$

Conic:

$$
C=
\frac1D
\begin{bmatrix}
404.3&-8\\
-8&416.3
\end{bmatrix}.
$$

Eigen discriminant:

$$
\sqrt{
(416.3-404.3)^2
+
4(8)^2
}
$$

$$
=
\sqrt{
144+256
}
=
20.
$$

Therefore:

$$
\lambda_{max}
=
\frac{
416.3+404.3+20
}{2}
=
420.3.
$$

Three-sigma radius:

$$
r=
3\sqrt{420.3}
\approx61.5\text{ px}.
$$

So the Gaussian centered at:

$$
(1160,640)
$$

gets a conservative raster box about:

$$
1098.5\ldots1221.5
$$

by:

$$
578.5\ldots701.5.
$$

That is exactly the kind of record the projection kernel produces before tile scheduling.

---

### WHY THE JACOBIAN APPROXIMATION WORKS — AND WHERE IT BREAKS

If Gaussian is small relative to:

$$
Z,
$$

projection changes approximately linearly across its support.

Then first-order approximation is excellent.

If Gaussian is huge or extremely close to camera, projection curvature over its support becomes significant.

True projected shape is no longer exactly one ellipse generated from the center Jacobian.

So one approximation condition is roughly:

$$
\boxed{
\frac{
\text{Gaussian spatial extent}
}{
camera\ distance
}
\ll1.
}
$$

This is not normally written as a hard renderer invariant, but it explains why pathological giant near-camera Gaussians are problematic.

The EWA lineage explicitly relies on local affine approximation of projective mapping. [vcg.seas.harvard.edu](https://vcg.seas.harvard.edu/content/3-publications/20020101-ewa-splatting/zwicker_ewa.pdf?utm_source=chatgpt.com)

---

### WHY COVARIANCE IS SUCH A POWERFUL REPRESENTATION

A naive oriented ellipsoid could store:

```text id="nmuml2"
three axis vectors
three radii
```

but covariance gives all of this in one symmetric mathematical object.

It transforms cleanly:

$$
\Sigma'
=
A\Sigma A^T.
$$

It projects cleanly:

$$
\Sigma_s=
J\Sigma J^T.
$$

Its inverse evaluates screen-space weighting:

$$
q=
d^T\Sigma^{-1}d.
$$

Its eigenvalues give radii.

Its eigenvectors give orientation.

Its determinant detects collapse.

The same matrix simultaneously encodes geometry, filtering footprint and rasterization kernel.

That is why covariance is the core representation of anisotropic splatting.

---

### THE THREE MOST COMMON COVARIANCE BUGS

The first is forgetting to square scale:

Wrong:

$$
\Sigma=
R
diag(s)
R^T.
$$

Correct:

$$
\boxed{
\Sigma=
R
diag(s^2)
R^T.
}
$$

Scale is standard deviation.

Covariance stores variance.

The second is wrong transpose order:

Wrong:

$$
R^TDR
$$

when your mathematical convention expects:

$$
RDR^T.
$$

That rotates axes differently.

The third is using the whole camera 4×4 on covariance including translation.

Translation does not belong in covariance propagation.

Use upper linear 3×3 only.

---

### THE FOUR MOST COMMON PROJECTION BUGS

One is wrong forward axis:

$$
+Z
$$

versus:

$$
-Z.
$$

Two is mixing normalized-device coordinates with pixel-space focal lengths.

Three is forgetting:

$$
Z^2
$$

in Jacobian depth derivatives.

Four is using:

$$
\Sigma J
$$

instead of:

$$
J\Sigma J^T.
$$

Every one of these can still produce “something visible,” which makes 3DGS bugs deceptive.

---

### THE SH DEBUGGING CHECK

For a degree-zero Gaussian:

$$
C=
C_0\,dc+0.5.
$$

Color must not change as camera rotates around Gaussian.

If it does, degree-zero handling is broken.

For degree-one data, evaluate at:

$$
(1,0,0),
(-1,0,0),
(0,1,0),
(0,-1,0),
(0,0,1),
(0,0,-1).
$$

The signs of the first-order basis become obvious.

This catches:

- camera→Gaussian versus Gaussian→camera reversal,
- x/y/z swaps,
- coefficient-order errors.

Given the Maveb/Vulkax higher-order storage-layout difference identified above, this is a particularly valuable test.

---

### THE PLY DEBUGGING CHECK

For one synthetic Gaussian, create PLY with:

$$
position=(1,2,3)
$$

$$
logScale=(\ln2,\ln3,\ln4)
$$

$$
q=(1,0,0,0)
$$

$$
opacityLogit=0
$$

so decoded:

$$
scale=(2,3,4)
$$

and:

$$
opacity=0.5.
$$

Expected covariance:

$$
\Sigma=
diag(
4,9,16
).
$$

Then choose one nonzero SH coefficient at a time.

If loader/rasterizer does not reproduce expected basis response exactly, the property mapping is wrong.

This single synthetic file tests a huge amount of infrastructure.

---


You should now be able to derive, not memorize, the entire path.

Start with a PLY Gaussian containing:

$$
x,y,z,
$$

$$
scale_0,scale_1,scale_2,
$$

$$
rot_0,\dots,rot_3,
$$

$$
opacity,
$$

$$
f_dc,
f_rest.
$$

Explain why stored scale is:

$$
\ell
$$

rather than:

$$
s.
$$

Recover:

$$
s=e^\ell.
$$

Explain why:

$$
\Sigma
=
Rdiag(s^2)R^T
$$

is automatically positive semidefinite.

Prove:

$$
x^T\Sigma x
=
\|M^Tx\|^2.
$$

Then explain why quaternion must be normalized.

Then derive:

$$
\alpha=sigmoid(o)
$$

and:

$$
d\alpha/do
=
\alpha(1-\alpha).
$$

Then explain why maximum Gaussian alpha occurs at its center.

Then explain why base opacity below a raster threshold allows whole-Gaussian culling.

Then derive coefficient count:

$$
(L+1)^2.
$$

Explain why degree three has:

$$
45
$$

`f_rest` values rather than 48.

Then evaluate degree-one SH by hand for direction:

$$
(1,0,0).
$$

Then explain why reversing direction changes odd-degree terms.

Then explain Maveb's channel-major PLY layout.

Then explain why stable ID cannot equal storage index if sorting/filtering occurs.

Then take world center:

$$
\mu_w
$$

and derive:

$$
\mu_c=R_c\mu_w+t.
$$

Then derive perspective:

$$
u=f_xX/Z+c_x
$$

$$
v=f_yY/Z+c_y.
$$

Then, from calculus alone, derive:

$$
J=
\begin{bmatrix}
f_x/Z&0&-f_xX/Z^2\\
0&f_y/Z&-f_yY/Z^2
\end{bmatrix}.
$$

Then prove covariance transformation:

$$
Cov(AX+b)=A\Sigma A^T.
$$

Use it twice:

$$
\Sigma_c=
R_c\Sigma_wR_c^T
$$

then:

$$
\Sigma_s=
J\Sigma_cJ^T.
$$

Then explain why second equation is approximate while first is exact for rigid/affine camera transforms.

Then regularize:

$$
\Sigma_s\leftarrow
\Sigma_s+0.3I.
$$

Write:

$$
\Sigma_s=
\begin{bmatrix}
a&b\\
b&c
\end{bmatrix}.
$$

Derive:

$$
det=
ac-b^2.
$$

Derive inverse:

$$
\Sigma_s^{-1}
=
\frac1{det}
\begin{bmatrix}
c&-b\\
-b&a
\end{bmatrix}.
$$

Then derive pixel exponent:

$$
q=
A dx^2+
2Bdxdy+
Cdy^2.
$$

Explain why:

$$
q=9
$$

means a three-sigma contour.

Then solve characteristic polynomial and obtain:

$$
\lambda_{\pm}
=
\frac{
a+c\pm
\sqrt{(a-c)^2+4b^2}
}{2}.
$$

Then derive:

$$
r=3\sqrt{\lambda_{max}}.
$$

Then explain why this radius creates a conservative square but:

$$
3\sqrt a,\quad3\sqrt c
$$

can produce a tighter axis-aligned rectangle.

Then explain why:

$$
Z\rightarrow0
$$

is dangerous from the Jacobian itself.

Then explain why offscreen culling can operate entirely on the projected bound.

Finally, open Maveb's `aetherGaussianProject` mentally and map each line to one equation above.

If you can do that without looking at the code, then you understand 3D Gaussian projection rather than merely knowing that “3DGS projects ellipses.”

---


The current **Maveb** path is unusually useful pedagogically because its CPU oracle and Metal implementation track one another closely. The canonical asset preserves trained log-scale/logit values; PLY loading is strict and supports ASCII and binary little-endian; quaternion normalization and SH degrees 0–3 are explicit; CPU and GPU construct $R\,diag(s^2)R^T$, transform it into camera space, use the analytically derived perspective Jacobian, add the same $0.3I$ low-pass covariance, invert the same 2×2 matrix and use the same three-sigma maximum-eigenvalue bound. Its CPU oracle then supplies deterministic color, depth and dominant-source-ID evidence, making it a real correctness oracle rather than merely a second renderer.

There are also four details worth retaining in your mental bug list. Maveb's canonical Gaussian itself does not yet use a persistent stable ID; its oracle IDs are source-index-derived. The CPU oracle has an extreme-radius safety cap that the inspected Metal projection kernel does not mirror exactly. Maveb does not currently projection-cull low base opacity even though its $1/255$ pixel threshold makes such a cull provably safe. Finally, Vulkax's inspected higher-order SH indexing does not currently appear to use the same GraphDECO channel-major ordering that Maveb explicitly documents, so that should be regression-tested rather than assumed equivalent.



That is the completion of the Gaussian phase: the projected ellipse now becomes a **scalable rasterizer**. We will derive 16×16 screen tiling, Gaussian→tile range expansion, count arrays and exclusive prefix sums as a CSR-like reference structure, depth/tile key construction, why sorting order is required by alpha compositing, stable radix sorting, quadratic Gaussian weight evaluation, transmittance and premultiplied front-to-back accumulation, early termination and numerical underflow; then map the CPU reference raster loop against Maveb's complete Metal pipeline—projection → scan → key generation → radix histogram/offset/scatter → tile ranges → composite—and validate CPU/GPU output using RMSE, PSNR, max error and changed-pixel fractions. M150 then moves from “rendered splats” into the reason **MAVEB exists**: stable captured-world identity, Gaussian ↔ entity/DOF correspondence, covariance deformation under edits, affine motion transfer, appearance-versus-physical representation, support-weighted appearance/physics coupling, provenance and the limits of treating a learned radiance-field primitive as physical matter.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/39 - Tiling, Ordering and Compositing at Scale|Next chapter]]
