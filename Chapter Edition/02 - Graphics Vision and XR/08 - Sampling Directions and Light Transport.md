# 08 — Sampling Directions and Light Transport

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/09 - Point Lights, MIS and Environment Lighting|Next chapter]]

Uniformly sampling angles is not the same as uniformly sampling directions on a sphere. These chapters derive the relevant distributions and show how practical path-tracer choices affect variance and convergence.

**By the end:** Construct and compare directional sampling strategies for path tracing.

**Recall question:** Why does uniform theta sampling overrepresent some regions of a sphere?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 16535–19360; source-span SHA256 7c6f2a0a8cefb6d7221ab8bcc33a0e7a400344c81b2b38c4deb75ebf02570803. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Next — Pass 7: M123 + M124, Parts 2031–2069

This is where the Monte Carlo machinery becomes a practical path tracer: **uniform sphere/hemisphere sampling, cosine-weighted sampling, local tangent bases, inverse-CDF/sample transformations, GGX importance sampling, variance and importance-sampling theory, MIS motivation and balance/power heuristics, light sampling versus BRDF sampling, path throughput, Russian roulette, next-event estimation, point/area/environment-light sampling, area↔solid-angle PDF conversion, environment CDF/alias sampling, progressive accumulation, noise/fireflies/clamping, and path-event classification.**



This pass is the point where the rendering equation from Pass 6 becomes an actual practical path tracer.



The central equation from Pass 6 was

$$
L_o(x,\omega_o)
=
L_e(x,\omega_o)
+
\int_{\Omega^+}
f_r(x,\omega_i,\omega_o)
L_i(x,\omega_i)
\left(n\cdot\omega_i\right)
\,d\omega_i.
$$

Monte Carlo turned this into

$$
\boxed{
\hat L_o
=
L_e+
\frac{1}{N}
\sum_{k=1}^{N}
\frac{
f_r(\omega_k,\omega_o)
L_i(\omega_k)
|n\cdot\omega_k|
}{
p(\omega_k)
}
}
$$

and nearly everything in Pass 7 is about one question:

$$
\boxed{
\text{How should we choose }p(\omega)\text{ so that this estimator is cheap, correct, and low variance?}
}
$$

---

**Sampling a direction is sampling a point on a sphere**

A unit direction

$$
\omega=(x,y,z)
$$

satisfies

$$
x^2+y^2+z^2=1.
$$

Using spherical coordinates around a chosen normal axis,

$$
x=\sin\theta\cos\phi,
$$

$$
y=\cos\theta,
$$

$$
z=\sin\theta\sin\phi.
$$

I am using $+Y$ as the local normal axis here because your current `CosineHemisphereDirection()` does exactly that.

The differential solid angle is

$$
\boxed{
d\omega=\sin\theta\,d\theta\,d\phi.
}
$$

That $\sin\theta$ is the source of many sampling formulas that otherwise look mysterious.

---

**Why choosing $\theta$ uniformly is not uniform on a sphere**

A common first attempt is

$$
\theta=\pi\xi_1,
\qquad
\phi=2\pi\xi_2,
$$

with

$$
\xi_1,\xi_2\sim U[0,1).
$$

That is **not** uniform in solid angle.

Near the poles, many values of $\phi$ describe points that are physically very close together.

The area element of the unit sphere is

$$
dA=d\omega
=
\sin\theta\,d\theta\,d\phi.
$$

When

$$
\theta\rightarrow0,
$$

$$
\sin\theta\rightarrow0.
$$

So equal increments of $\theta$ do not correspond to equal surface area.

Uniform $\theta$ causes clustering near the poles.

This is one of the first rules of sampling:

$$
\boxed{
\text{uniform parameter values do not imply uniform geometric measure.}
}
$$

---

**Uniform sphere sampling**

The total solid angle of a sphere is

$$
4\pi.
$$

Therefore a uniform spherical-direction PDF is

$$
\boxed{
p_{\text{sphere}}(\omega)
=
\frac{1}{4\pi}.
}
$$

To generate such a direction, observe that

$$
z=\cos\theta.
$$

From

$$
d\omega
=
\sin\theta\,d\theta\,d\phi,
$$

and

$$
dz=-\sin\theta\,d\theta,
$$

we get

$$
d\omega=d\phi\,dz
$$

up to sign/orientation.

Therefore uniform solid angle corresponds to uniform:

$$
\phi\in[0,2\pi)
$$

and uniform:

$$
z\in[-1,1].
$$

Let

$$
\xi_1,\xi_2\sim U[0,1).
$$

Choose

$$
\boxed{
z=1-2\xi_1
}
$$

and

$$
\boxed{
\phi=2\pi\xi_2.
}
$$

The radius of the horizontal circle is

$$
r=\sqrt{1-z^2}.
$$

Therefore

$$
\boxed{
\omega=
\left(
r\cos\phi,
z,
r\sin\phi
\right).
}
$$

Every equal-solid-angle patch receives the same probability.

---

**Uniform hemisphere sampling**

For a hemisphere around $+Y$,

$$
y=\cos\theta\in[0,1].
$$

Hemisphere solid angle is

$$
2\pi.
$$

Thus

$$
\boxed{
p_{\text{hemisphere}}
=
\frac{1}{2\pi}.
}
$$

Let:

$$
\xi_1,\xi_2\sim U[0,1).
$$

Use

$$
\boxed{
y=\xi_1
}
$$

and

$$
\phi=2\pi\xi_2.
$$

Then

$$
r=\sqrt{1-y^2}.
$$

So:

```cpp
Vec3 SampleUniformHemisphere(
    float xi1,
    float xi2)
{
    const float y = xi1;

    const float phi =
        2.0f * Pi * xi2;

    const float radius =
        std::sqrt(
            std::max(
                0.0f,
                1.0f - y * y));

    return
    {
        radius * std::cos(phi),
        y,
        radius * std::sin(phi)
    };
}
```

Its PDF is always:

```cpp
1.0f / (2.0f * Pi)
```

for directions in the upper hemisphere.

---

**Uniform hemisphere sampling is correct but often inefficient**

Suppose we have a Lambertian surface:

$$
f_r=\frac{\rho}{\pi}.
$$

Its rendering-equation integrand contains

$$
f_rL_i\cos\theta.
$$

Even if incoming radiance were constant,

$$
L_i=C,
$$

the integrand varies as

$$
\cos\theta.
$$

Near the normal:

$$
\cos\theta\approx1.
$$

Near grazing:

$$
\cos\theta\approx0.
$$

Uniform hemisphere sampling spends just as many samples near grazing as near the normal even though those grazing samples contribute much less.

That is legal.

It is merely wasteful.

---

**Cosine-weighted hemisphere sampling**

For Lambertian transport, a much better PDF is

$$
\boxed{
p(\omega)
=
\frac{\cos\theta}{\pi}.
}
$$

First verify normalization:

$$
\int_{\Omega^+}
\frac{\cos\theta}{\pi}
\,d\omega
$$

$$
=
\frac1\pi
\int_0^{2\pi}
\int_0^{\pi/2}
\cos\theta
\sin\theta
\,d\theta\,d\phi.
$$

Inner integral:

$$
\int_0^{\pi/2}
\cos\theta\sin\theta\,d\theta
=
\frac12.
$$

Outer integral:

$$
2\pi.
$$

Therefore:

$$
\frac1\pi
(2\pi)
\frac12
=
1.
$$

So it is a valid PDF.

---

**Deriving the cosine-weighted sampling formula**

Marginal distribution in $\theta$ is proportional to

$$
\cos\theta\sin\theta.
$$

Normalized angular PDF is:

$$
p_\theta(\theta)
=
2\cos\theta\sin\theta.
$$

CDF:

$$
F(\theta)
=
\int_0^\theta
2\sin t\cos t\,dt.
$$

Since:

$$
\frac{d}{dt}\sin^2t
=
2\sin t\cos t,
$$

we get:

$$
\boxed{
F(\theta)
=
\sin^2\theta.
}
$$

Set:

$$
\xi=F(\theta)
=
\sin^2\theta.
$$

Therefore:

$$
\sin\theta
=
\sqrt{\xi},
$$

and:

$$
\cos\theta
=
\sqrt{1-\xi}.
$$

Let:

$$
r=\sin\theta=\sqrt{\xi_2}.
$$

Choose:

$$
\phi=2\pi\xi_1.
$$

Then:

$$
x=r\cos\phi,
$$

$$
z=r\sin\phi,
$$

$$
y=\sqrt{1-\xi_2}.
$$

Thus:

$$
\boxed{
\omega_{\text{local}}
=
\left(
\sqrt{\xi_2}\cos(2\pi\xi_1),
\sqrt{1-\xi_2},
\sqrt{\xi_2}\sin(2\pi\xi_1)
\right).
}
$$

This is exactly the structure currently present in Kairo's `CosineHemisphereDirection()`.

The square root is not arbitrary.

It comes directly from inverting the CDF.

---

**Equivalent disk interpretation**

Cosine-weighted hemisphere sampling can also be understood as:

$$
\text{uniform disk}
\rightarrow
\text{project upward onto hemisphere}.
$$

Sample a uniform unit disk:

$$
r=\sqrt{\xi_1},
$$

$$
\phi=2\pi\xi_2.
$$

Then:

$$
x=r\cos\phi,
$$

$$
z=r\sin\phi.
$$

Lift onto sphere:

$$
y=\sqrt{1-x^2-z^2}.
$$

The resulting hemisphere density becomes:

$$
\frac{\cos\theta}{\pi}.
$$

This geometric view is often easier to remember than the CDF derivation.

---

**Why the square root appears in uniform disk sampling**

A disk has area element:

$$
dA=r\,dr\,d\phi.
$$

If we chose:

$$
r=\xi
$$

uniformly, samples would cluster near the center.

Disk area within radius $r$ is:

$$
A(r)=\pi r^2.
$$

Normalized CDF:

$$
F(r)=r^2.
$$

Set:

$$
\xi=r^2.
$$

Therefore:

$$
\boxed{
r=\sqrt{\xi}.
}
$$

Again:

$$
\boxed{
\text{inverse CDF produces the square root.}
}
$$

Sampling formulas should be reconstructed from measure, not memorized.

---

**Local coordinates are not world coordinates**

The formulas above assume the hemisphere is oriented around:

$$
N_{\text{local}}=(0,1,0).
$$

But the actual surface normal might be:

$$
N=(0.3,0.8,-0.52).
$$

We need an orthonormal basis:

$$
T,
N,
B.
$$

Then local direction:

$$
\omega_l=(x,y,z)
$$

becomes world direction:

$$
\boxed{
\omega_w
=
xT+
yN+
zB.
}
$$

This is the same tangent-frame idea from normal mapping, now used in reverse: instead of transforming a normal into world space, we transform a sampled direction.

---

**Constructing a robust basis**

Given unit normal:

$$
N.
$$

Choose a helper axis not nearly parallel to $N$.

For example:

```cpp
Vec3 BuildTangent(Vec3 n)
{
    const Vec3 helper =
        std::abs(n.y) < 0.999f
        ? Vec3{0, 1, 0}
        : Vec3{1, 0, 0};

    return Normalize(
        Cross(helper, n));
}
```

Then:

$$
T=\operatorname{normalize}(H\times N)
$$

and:

$$
B=N\times T.
$$

Kairo currently does approximately:

```cpp
tangent =
    abs(normal.y) < 0.999
        ? normalize(cross(Up, normal))
        : UnitX;

bitangent =
    normalize(cross(normal, tangent));
```

Then:

```cpp
world =
    normalize(
        tangent * x +
        normal  * y +
        bitangent * z);
```

That is exactly Part 2034.

---

**Why the helper-axis test exists**

If:

$$
N\approx(0,1,0)
$$

and helper is also:

$$
(0,1,0),
$$

then:

$$
H\times N\approx0.
$$

Normalization becomes unstable or undefined.

So we deliberately choose an axis sufficiently different from the normal.

This small numerical detail is one of those things that separates “formula works on paper” from “renderer survives every normal.”

---

**Sampling Lambertian BRDFs**

Lambert BRDF:

$$
f_r=\frac{\rho}{\pi}.
$$

Cosine PDF:

$$
p(\omega_i)
=
\frac{n\cdot\omega_i}{\pi}.
$$

Path throughput update is:

$$
\frac{
f_r
(n\cdot\omega_i)
}{
p(\omega_i)
}.
$$

Substitute:

$$
\frac{
(\rho/\pi)
(n\cdot\omega_i)
}{
(n\cdot\omega_i)/\pi
}.
$$

Everything cancels:

$$
\boxed{
\beta_{\text{new}}
=
\beta_{\text{old}}\rho.
}
$$

This is why a cosine-sampled Lambert bounce can reduce to:

```cpp
throughput *= albedo;
```

There is no missing $\pi$.

The:

$$
1/\pi,
\quad
\cos\theta,
\quad
\cos\theta/\pi
$$

cancel exactly.

Current Kairo's recursive indirect term:

```cpp
indirect =
    albedo *
    TracePath(nextRay) /
    survivalProbability;
```

implicitly relies on precisely this cancellation.

That is mathematically legitimate for a cosine-sampled ideal Lambert bounce.

---

**Importance sampling**

General Monte Carlo variance is determined by:

$$
Y=
\frac{f(X)}{p(X)}.
$$

For estimator:

$$
\hat I=\frac1N\sum Y_i.
$$

Variance:

$$
\operatorname{Var}[\hat I]
=
\frac{1}{N}
\operatorname{Var}[Y].
$$

Now:

$$
E[Y^2]
=
\int
\left(
\frac{f(x)}{p(x)}
\right)^2
p(x)\,dx.
$$

Therefore:

$$
E[Y^2]
=
\boxed{
\int
\frac{f(x)^2}{p(x)}
\,dx.
}
$$

So:

$$
\boxed{
\operatorname{Var}(Y)
=
\int
\frac{f(x)^2}{p(x)}
\,dx
-
I^2.
}
$$

This equation explains importance sampling more rigorously than “sample bright things more.”

If $f(x)$ is large but $p(x)$ is tiny, then:

$$
\frac{f(x)^2}{p(x)}
$$

can become enormous.

That produces variance and potentially fireflies.

---

**Ideal importance distribution**

For a nonnegative integrand, the zero-variance distribution is:

$$
\boxed{
p^*(x)
=
\frac{
f(x)
}{
\int f
}.
}
$$

Then:

$$
\frac{f(x)}{p^*(x)}
=
\int f
=
I.
$$

Every sample gives exactly:

$$
I.
$$

Variance:

$$
0.
$$

We cannot normally use this because its normalization contains the integral we are trying to estimate.

But the principle is fundamental:

$$
\boxed{
p(x)\text{ should resemble }|f(x)|.
}
$$

---

**Why cosine sampling reduces Lambert variance**

For constant incoming radiance:

$$
L_i=C.
$$

Lambert integrand:

$$
g(\omega)
=
\frac{\rho}{\pi}
C
\cos\theta.
$$

Therefore:

$$
g(\omega)\propto\cos\theta.
$$

Cosine PDF:

$$
p(\omega)
=
\frac{\cos\theta}{\pi}.
$$

Thus:

$$
g/p
=
\rho C
$$

is constant.

Zero variance.

If incoming illumination is not constant, variance remains, but cosine sampling still matches one major factor of the integrand.

---

**Sampling GGX: what are we actually sampling?**

For Cook–Torrance:

$$
f_s=
\frac{D(H)F(H)G}{4N_LN_V}.
$$

The lobe is strongly influenced by:

$$
D(H).
$$

Therefore, rather than choosing incoming direction $\omega_i$ uniformly, we can sample a **microfacet half-vector** $H$ according to the GGX normal distribution and reflect the outgoing/view vector around it.

The chain is:

$$
\xi_1,\xi_2
\rightarrow
H
\rightarrow
\omega_i.
$$

That introduces a change-of-variables PDF.

---

**GGX NDF**

For isotropic GGX:

$$
D(H)
=
\frac{
\alpha^2
}{
\pi
[
(N\cdot H)^2(\alpha^2-1)+1
]^2
}.
$$

The projected microfacet normal density is:

$$
\boxed{
p_H(H)
=
D(H)(N\cdot H).
}
$$

Why the cosine?

Because $D$ describes microfacet area per macro projected area, and converting it into a proper directional sampling density includes projected orientation.

---

**Deriving isotropic GGX half-vector sampling**

One common sampling formulation is:

$$
\phi=2\pi\xi_1
$$

and:

$$
\tan^2\theta_h
=
\frac{
\alpha^2\xi_2
}{
1-\xi_2
}.
$$

Therefore:

$$
\tan\theta_h
=
\alpha
\sqrt{
\frac{\xi_2}{1-\xi_2}
}.
$$

From:

$$
1+\tan^2\theta
=
\frac1{\cos^2\theta},
$$

we get:

$$
\cos^2\theta_h
=
\frac1{
1+
\alpha^2
\frac{\xi}{1-\xi}
}.
$$

Multiply numerator and denominator by:

$$
1-\xi.
$$

Then:

$$
\boxed{
\cos^2\theta_h
=
\frac{
1-\xi
}{
1+(\alpha^2-1)\xi
}.
}
$$

Hence:

$$
\boxed{
\cos\theta_h
=
\sqrt{
\frac{
1-\xi_2
}{
1+(\alpha^2-1)\xi_2
}
}.
}
$$

Then:

$$
\sin\theta_h
=
\sqrt{
1-\cos^2\theta_h
}.
$$

Local half-vector:

$$
H_l=
(
\sin\theta_h\cos\phi,
\cos\theta_h,
\sin\theta_h\sin\phi
).
$$

Transform it into the surface basis.

---

**Half-vector to sampled light direction**

Suppose:

$$
V
$$

points away from the surface toward the camera.

A microfacet normal:

$$
H
$$

reflects $-V$ into incoming-light direction:

$$
\boxed{
L=
\operatorname{reflect}(-V,H).
}
$$

Then reject if:

$$
N\cdot L\le0.
$$

But this transformation changes probability density.

The reflection Jacobian gives:

$$
\boxed{
p_L(L)
=
\frac{
p_H(H)
}{
4|V\cdot H|
}.
}
$$

Thus if:

$$
p_H=D(H)(N\cdot H),
$$

then:

$$
\boxed{
p_L
=
\frac{
D(H)(N\cdot H)
}{
4|V\cdot H|
}.
}
$$

For a GGX importance-sampled BRDF estimator, this PDF—not merely $D$—belongs in the denominator.

---

**Why ordinary NDF sampling wastes samples**

Suppose view direction is highly grazing.

You sample a microfacet normal $H$ according to:

$$
D(H)(N\cdot H).
$$

Some sampled microfacets are geometrically invisible from $V$.

Others reflect $-V$ below the macroscopic surface:

$$
N\cdot L<0.
$$

Those samples contribute zero.

At grazing angles this waste can become significant.

Ordinary NDF sampling samples **all microfacets**.

But the camera only sees a subset.

This motivates visible-normal sampling.

---

**Visible Normal Distribution Function**

The GGX visible-normal distribution conditions the microfacet distribution on visibility from the outgoing direction.

Conceptually:

$$
\boxed{
p_{\text{VNDF}}(H|V)
\propto
D(H)
G_1(V,H)
\frac{
V\cdot H
}{
N\cdot V
}.
}
$$

The exact normalized form for visible microfacet normals can be expressed approximately as:

$$
\boxed{
D_V(H)
=
\frac{
G_1(V)
(V\cdot H)
D(H)
}{
N\cdot V
}
}
$$

for valid visible normals.

This automatically suppresses microfacets the viewer cannot see.

The result is much lower wasted-sample rate, particularly for:

- rough surfaces,
- grazing views,
- GGX's long tails.

This is why modern production GGX samplers generally prefer VNDF sampling over blindly sampling the bare NDF.

---

**Heitz-style GGX VNDF idea**

The classic practical strategy can be understood geometrically.

First stretch the view vector by roughness:

$$
V_h=
\operatorname{normalize}
(
\alpha_xV_x,
\alpha_yV_y,
V_z
).
$$

Then construct a basis around the stretched view.

Then sample a projected disk corresponding to visible slopes.

Then warp the disk sample according to the visible projected hemisphere.

Then reconstruct the normal in stretched space.

Finally unstretch:

$$
H=
\operatorname{normalize}
(
\alpha_xH_x,
\alpha_yH_y,
\max(0,H_z)
).
$$

The details are subtler than ordinary inverse-CDF GGX sampling because the distribution is view-dependent.

The important conceptual distinction is:

$$
\boxed{
\text{NDF sampling: sample surface normals that exist}
}
$$

versus:

$$
\boxed{
\text{VNDF sampling: sample surface normals that exist and are visible from }V.
}
$$

---

**Importance sampling cannot optimize every term simultaneously**

The full integrand is:

$$
f_r(\omega_i,\omega_o)
L_i(\omega_i)
\cos\theta_i.
$$

BRDF importance sampling approximates:

$$
f_r\cos\theta.
$$

It knows nothing about:

$$
L_i.
$$

Light importance sampling approximates bright incoming radiance:

$$
L_i.
$$

It knows less about the BRDF.

Suppose a tiny bright area light illuminates a diffuse wall.

BRDF sampling may never randomly hit the tiny light.

Suppose instead a highly glossy surface sees a huge environment.

Uniformly/light-sampling the environment may frequently choose directions outside the narrow specular lobe.

Neither strategy is universally best.

That is why we need **multiple importance sampling**.

---

**MIS motivation**

Suppose we have two estimators for the same integral.

Technique A:

$$
p_A(x)
$$

is good in one region.

Technique B:

$$
p_B(x)
$$

is good somewhere else.

Rather than choose only one technique, sample from both and weight each contribution.

The weights must satisfy:

$$
\boxed{
w_A(x)+w_B(x)=1
}
$$

where both techniques can generate the same contribution.

Then the combined estimator remains correct if designed properly.

---

**Balance heuristic**

For technique $i$ using $n_i$ samples and PDF $p_i(x)$, the balance weight is:

$$
\boxed{
w_i(x)
=
\frac{
n_i p_i(x)
}{
\sum_j n_jp_j(x)
}.
}
$$

For one sample from light sampling and one from BRDF sampling:

$$
n_L=n_B=1.
$$

Then:

$$
\boxed{
w_L
=
\frac{
p_L
}{
p_L+p_B
}
}
$$

and:

$$
\boxed{
w_B
=
\frac{
p_B
}{
p_L+p_B
}.
}
$$

Notice:

$$
w_L+w_B=1.
$$

---

**Power heuristic**

A commonly used MIS weighting is:

$$
\boxed{
w_i
=
\frac{
(n_ip_i)^\beta
}{
\sum_j(n_jp_j)^\beta
}.
}
$$

Usually:

$$
\beta=2.
$$

For one sample from each:

$$
\boxed{
w_L
=
\frac{
p_L^2
}{
p_L^2+p_B^2
}.
}
$$

Kairo's current helper is exactly:

```cpp
float PowerHeuristic(
    float pdfA,
    float pdfB)
{
    const float a2 = pdfA * pdfA;
    const float b2 = pdfB * pdfB;

    return
        a2 /
        max(a2 + b2, epsilon);
}
```

That is the canonical two-technique, one-sample-each power heuristic.

---

**Why power heuristic often works better than balance**

Suppose:

$$
p_A=100p_B.
$$

Balance heuristic gives:

$$
w_A
=
\frac{100}{101}
\approx0.9901.
$$

Power heuristic:

$$
w_A
=
\frac{10000}{10001}
\approx0.9999.
$$

The power heuristic more aggressively favors the sampling technique that considers the sample likely.

This suppresses high-variance contributions from techniques that accidentally stumble into regions they sample poorly.

---

**MIS does not mean “multiply a sample by a heuristic”**

MIS requires that multiple techniques are genuinely capable of sampling the same contribution and that each PDF is evaluated in the same measure.

A correct two-technique direct-light estimator is schematically:

$$
\hat L=
\underbrace{
\frac{
f(x_L)
}{
p_L(x_L)
}
w_L(x_L)
}_{\text{light-sampled}}
+
\underbrace{
\frac{
f(x_B)
}{
p_B(x_B)
}
w_B(x_B)
}_{\text{BSDF-sampled}}.
$$

Both techniques must exist.

Simply calculating:

$$
w_L
=
\frac{p_L^2}{p_L^2+p_B^2}
$$

and multiplying only the light-sampled estimator by $w_L$ while never evaluating the corresponding BSDF estimator generally reduces expected energy.

That is bias.

---

**Important current-Kairo observation**

Current `DirectLambert()` computes an area-light sample and evaluates:

$$
p_L
=
\frac{r^2}
{A\cos\theta_L}
$$

and:

$$
p_B=
\frac{\cos\theta_x}{\pi}.
$$

Then:

$$
w=
\frac{p_L^2}{p_L^2+p_B^2}.
$$

That is the correct shape for an MIS power weight.

However, the current path integrator does not have a matching direct-light BSDF-sampling MIS branch at that point.

Its ordinary bounce can eventually hit emissive geometry, but the current `AreaLight` is a separate light object rather than necessarily a sampled emissive primitive, and no symmetric MIS weight is applied when an indirect ray reaches emission.

Therefore the current area-light weighting is best understood as an **incomplete MIS scaffold**, not a full unbiased two-technique MIS estimator.

This is a concrete mathematical upgrade target.

---

**Light sampling versus BRDF sampling**

Consider a small area light visible from a diffuse surface.

The solid angle subtended by the light may be:

$$
10^{-4}\,\text{sr}.
$$

Hemisphere:

$$
2\pi\approx6.28\,\text{sr}.
$$

Probability that a random uniform hemisphere direction lands on the light is approximately:

$$
\frac{10^{-4}}{6.28}
\approx1.6\times10^{-5}.
$$

You could wait tens of thousands of samples for one useful light hit.

Directly sampling the light guarantees the sample lands on it.

That is why next-event estimation is so powerful.

---

**The opposite case: sharp glossy BRDF**

Suppose a roughness:

$$
r=0.03.
$$

The GGX lobe is extremely narrow.

An enormous area light occupies much of the hemisphere.

Uniformly sampling the light can produce many directions for which:

$$
f_s\approx0.
$$

A GGX/VNDF BRDF sample naturally points near the reflection direction.

Thus BRDF sampling wins.

MIS adapts between the two.

---

**Path throughput**

A path is:

$$
x_0\rightarrow x_1\rightarrow x_2\rightarrow\dots
$$

where:

$$
x_0=\text{camera}.
$$

At each bounce the scattering estimator contributes:

$$
\frac{
f_r
|\cos\theta|
}{
p
}.
$$

Define path throughput:

$$
\boxed{
\beta_0=1.
}
$$

After bounce $k$:

$$
\boxed{
\beta_{k+1}
=
\beta_k
\frac{
f_k|\cos\theta_k|
}{
p_k
}.
}
$$

Therefore:

$$
\boxed{
\beta_n
=
\prod_{k=1}^{n}
\frac{
f_k|\cos\theta_k|
}{
p_k
}.
}
$$

It means:

> How much weight does radiance encountered later in this path have when transported back toward the camera?

---

**Why throughput is multiplicative**

Suppose first surface reflects:

$$
50\%
$$

of relevant energy.

Second reflects:

$$
20\%.
$$

Then only:

$$
0.5\times0.2=0.1
$$

remains.

Path transport is sequential attenuation/scattering.

Thus multiplicative accumulation is natural.

A path tracer often has:

```cpp
Color radiance = 0;
Color beta     = 1;
```

where `radiance` accumulates arrived/emitted/direct energy, while `beta` carries future path weight.

These are conceptually different variables.

---

**The iterative path-tracing loop**

A clean diffuse/glossy path tracer is usually easier to reason about iteratively than recursively:

```cpp
Color TracePath(
    Ray ray,
    RNG& rng)
{
    Color L = Color::Black();

    // Throughput from camera to current vertex.
    Color beta = Color::White();

    bool previousWasDelta = true;

    for (uint32_t bounce = 0;
         bounce < MaxDepth;
         ++bounce)
    {
        const auto hit =
            scene.Intersect(ray);

        if (!hit)
        {
            // Environment radiance reaching this path.
            L += beta *
                 scene.Environment(
                     ray.Direction);
            break;
        }

        const Material& material =
            scene.Material(
                hit->MaterialID);

        // Surface emission.
        L += beta *
             material.Emission;

        // Direct-light next-event estimation would happen here.

        BSDFSample s =
            SampleBSDF(
                material,
                *hit,
                -ray.Direction,
                rng);

        if (!s.Valid ||
            s.Pdf <= 0.0f)
        {
            break;
        }

        const float cosine =
            std::abs(
                Dot(
                    hit->ShadingNormal,
                    s.Wi));

        beta *=
            s.F *
            (cosine / s.Pdf);

        ray =
            SpawnRay(
                *hit,
                s.Wi);

        // Russian roulette after a few bounces...
    }

    return L;
}
```

That is the skeleton behind essentially every conventional unidirectional path tracer.

---

**Adding emitted radiance along a path**

Suppose the path reaches an emitter at bounce $k$.

Its emitted radiance is:

$$
L_e.
$$

Camera contribution is:

$$
\boxed{
\Delta L
=
\beta_kL_e.
}
$$

Why multiply by throughput?

Because the emitter's radiance has already passed through all preceding BRDF interactions.

For example:

$$
C\rightarrow\text{red wall}\rightarrow\text{light}.
$$

The light is white.

But the path is filtered by the red wall's reflectance.

So camera receives:

$$
\beta L_e.
$$

---

**Emission double-counting with NEE**

Once next-event estimation is introduced, a path can discover an emitter in two ways.

First:

$$
\text{explicit light sample from current vertex}.
$$

Second:

$$
\text{BSDF sample that happens to hit that same light}.
$$

If we fully add both without MIS, we double count that transport path.

A robust MIS path tracer therefore tracks enough context to weight emissive hits appropriately.

Common logic is approximately:

$$
\text{primary/emitter hit after delta event}
\Rightarrow
\text{full emission},
$$

while a non-delta BSDF hit on a light gets an MIS weight against the corresponding light-sampling PDF.

This detail becomes critical once you stop using purely diffuse educational transport.

---

**Throughput update $f\cos/p$**

The update:

$$
\boxed{
\beta
\leftarrow
\beta
\frac{
f_r(\omega_i,\omega_o)
|\cos\theta_i|
}{
p(\omega_i)
}
}
$$

is not a heuristic.

It is the Monte Carlo estimator.

If this factor is wrong, the renderer is mathematically solving a different integral.

Each piece comes from somewhere specific:

$$
f_r
$$

is material scattering,

$$
|\cos\theta|
$$

is projected area,

$$
p
$$

corrects for how frequently the sample was chosen.

---

**Lambert cancellation in throughput**

Again, for Lambert:

$$
f=\frac{\rho}{\pi}.
$$

Cosine sampling:

$$
p=\frac{\cos\theta}{\pi}.
$$

Thus:

$$
\frac{f\cos\theta}{p}
=
\rho.
$$

This is why current Kairo's diffuse bounce code can use:

```cpp
indirect =
    albedo *
    TracePath(nextRay);
```

before Russian-roulette compensation.

The simplification is specific to:

$$
\boxed{
\text{Lambert BRDF + cosine-weighted PDF}.
}
$$

Do not carry that shortcut into GGX unless you derive a corresponding cancellation.

---

**Maximum bounce depth**

A practical path tracer often has:

$$
d_{\max}.
$$

Stopping deterministically at a fixed depth causes bias because paths that should continue are discarded.

However, a maximum depth still serves useful purposes:

$$
\text{bug containment},
$$

$$
\text{bounded runtime},
$$

$$
\text{stack/resource safety},
$$

$$
\text{debug reproducibility}.
$$

Production renderers may set it high and rely primarily on Russian roulette for normal termination.

---

**Russian roulette path termination**

Suppose throughput has become:

$$
\beta=(0.03,0.02,0.01).
$$

Most future contributions are likely small.

Continuing every path wastes computation.

Russian roulette probabilistically terminates a path.

Let survival probability be:

$$
p_s.
$$

Draw:

$$
\xi\sim U[0,1).
$$

If:

$$
\xi>p_s,
$$

terminate.

If path survives, compensate:

$$
\boxed{
\beta
\leftarrow
\frac{\beta}{p_s}.
}
$$

---

**Why Russian roulette remains unbiased**

Suppose continuation contribution without roulette would be:

$$
C.
$$

With roulette:

$$
X=
\begin{cases}
C/p_s, & \text{with probability }p_s\\
0, & \text{with probability }1-p_s.
\end{cases}
$$

Expected value:

$$
E[X]
=
p_s\frac{C}{p_s}
+
(1-p_s)0
$$

$$
=
C.
$$

Therefore:

$$
\boxed{
E[X]=C.
}
$$

No bias.

Variance increases somewhat, but expected computation decreases.

This is the classic variance-versus-cost tradeoff.

---

**Choosing survival probability**

A common choice is based on maximum throughput component:

$$
p_s
=
\operatorname{clamp}
(
\max(\beta_r,\beta_g,\beta_b),
p_{\min},
p_{\max}
).
$$

Why maximum channel?

Suppose:

$$
\beta=(0.9,0.01,0.01).
$$

Average throughput is only:

$$
0.3067.
$$

Terminating based on average could prematurely kill an important red path.

Maximum component better respects the strongest remaining spectral channel.

Typical upper clamp:

$$
0.95
$$

ensures even bright paths eventually terminate.

---

**Current Kairo Russian roulette**

Kairo currently computes:

$$
p_s
=
\max(
r_{\text{albedo}},
g_{\text{albedo}},
b_{\text{albedo}},
0.15
)
$$

then clamps:

$$
p_s\le0.95.
$$

Roulette begins at:

$$
\text{depth}\ge2.
$$

On survival, indirect radiance is divided by:

$$
p_s.
$$

The compensation is correct in principle.

The limitation is that it uses the **current surface albedo**, not accumulated path throughput.

Once materials become glossy, metallic, transmissive or importance-sampled in more complex ways, survival probability should usually derive from actual:

$$
\beta.
$$

---

**Why Russian roulette should not begin immediately**

At bounce 0 or 1, throughput is often still significant.

Terminating very short paths increases noise badly.

Also direct/first-bounce transport is perceptually dominant.

Therefore common policy:

$$
\text{never roulette first few bounces}.
$$

Then enable it after:

$$
2,\ 3,\ 5
$$

or another chosen depth.

Current Kairo starts at depth 2.

That is a sensible educational default.

---


Suppose current path hits diffuse surface $x$.

A pure BSDF path tracer chooses a random outgoing bounce direction.

Eventually it may hit a light.

If lights are small, this is inefficient.

Next-event estimation explicitly asks:

$$
\boxed{
\text{Can I connect this surface directly to a sampled light point right now?}
}
$$

At each non-delta surface:

1. select a light;
2. sample a point/direction on that light;
3. test shadow visibility;
4. evaluate BRDF;
5. divide by light PDF;
6. optionally MIS-weight against BSDF sampling.

That direct connection is a one-segment path extension toward an emitter.

---

**Direct-light estimator**

Suppose light-sampled direction is:

$$
\omega_i.
$$

Directional light PDF:

$$
p_L(\omega_i).
$$

Then:

$$
\boxed{
\hat L_{\text{direct}}
=
\frac{
L_i(\omega_i)
f_r(\omega_i,\omega_o)
|n_x\cdot\omega_i|
V(x,\omega_i)
}{
p_L(\omega_i)
}.
}
$$

With MIS:

$$
\boxed{
\hat L_{\text{direct}}
=
w_L
\frac{
L_i f_r\cos\theta_xV
}{
p_L
}.
}
$$

This is the mathematically complete expression.

---

**Area-light sampling**

Consider rectangular area light parameterized by:

$$
P_L(u,v)
=
P_0+
uU+
vV.
$$

Suppose:

$$
u,v\in[-1/2,1/2].
$$

Light area is:

$$
\boxed{
A=
\|U\times V\|.
}
$$

Sampling uniformly on area means:

$$
p_A(y)
=
\frac1A.
$$

That PDF has units:

$$
\mathrm{m}^{-2}.
$$

But our rendering equation integrates over **solid angle** at the shading point.

So we must transform measures.

---

**Area-to-solid-angle differential relationship**

Let surface point be:

$$
x.
$$

Sampled light point:

$$
y.
$$

Distance:

$$
r=\|y-x\|.
$$

Direction:

$$
\omega=
\frac{y-x}{r}.
$$

Emitter normal:

$$
n_y.
$$

A small light patch:

$$
dA_y
$$

subtends solid angle:

$$
\boxed{
d\omega
=
\frac{
|n_y\cdot(-\omega)|
}{
r^2
}
dA_y.
}
$$

This is geometry.

Projected emitter area is:

$$
dA_y\cos\theta_y,
$$

and angular size falls as:

$$
1/r^2.
$$

---

**Converting the PDF**

Probability must remain invariant:

$$
p_A(y)dA
=
p_\omega(\omega)d\omega.
$$

Therefore:

$$
p_\omega
=
p_A
\frac{dA}{d\omega}.
$$

Using:

$$
d\omega
=
\frac{
\cos\theta_y
}{
r^2
}
dA,
$$

we get:

$$
\boxed{
p_\omega
=
p_A
\frac{
r^2
}{
|\cos\theta_y|
}.
}
$$

For uniform area:

$$
p_A=\frac1A.
$$

Thus:

$$
\boxed{
p_\omega
=
\frac{
r^2
}{
A|n_y\cdot(-\omega)|
}.
}
$$

This is exactly the formula current Kairo computes as:

```cpp
lightPdf =
    lightDistance * lightDistance /
    (lightArea * lightFacing);
```

The PDF conversion itself is correct.

---

**Area-light estimator after conversion**

If emitter has radiance:

$$
L_e(y,-\omega),
$$

then direct estimator is:

$$
\boxed{
\hat L_d
=
\frac{
L_e
f_r
|n_x\cdot\omega|
V
}{
p_\omega
}.
}
$$

Substitute:

$$
p_\omega
=
\frac{r^2}{A\cos\theta_y}.
$$

Then:

$$
\hat L_d
=
L_e
f_r
\cos\theta_x
V
\frac{
A\cos\theta_y
}{
r^2
}.
$$

The familiar geometric term emerges:

$$
\boxed{
G(x,y)
=
\frac{
\cos\theta_x\cos\theta_y
}{
r^2
}.
}
$$

So area sampling and the geometric form of light transport are simply two representations of the same quantity.

---

**Current Kairo area-light caveat**

Kairo currently calculates:

$$
p_\omega
=
\frac{r^2}{A\cos\theta_y}
$$

for MIS weighting.

But the contribution itself uses approximately:

$$
\text{Intensity}
\frac1{r^2}
\cos\theta_x
$$

without dividing the estimator by:

$$
p_\omega.
$$

Therefore its `AreaLight` behaves more like a collection of soft point-light samples than a rigorously defined radiance-emitting area source.

That is fine for an educational renderer if documented, but it is not the same estimator as physically defined area-light Monte Carlo integration.

For rigorous NEE, choose clear emitter semantics such as:

$$
L_e\quad[\mathrm{W\,m^{-2}\,sr^{-1}}]
$$

and evaluate:

$$
L_e f_r\cos\theta_x/p_\omega.
$$

---

**Current Kairo area sample pattern caveat**

The current code computes:

$$
u=
\frac{
(sample\bmod2)+0.5
}{2}
-0.5
$$

and analogous $v$.

That creates a fixed:

$$
2\times2
$$

stratified pattern.

For:

$$
Samples=4,
$$

that is reasonable.

For:

$$
Samples>4,
$$

the `% 2` logic repeats the same four positions.

So increasing `AreaLight::Samples` beyond four does not generate additional distinct sample locations.

A scalable sampler should generate arbitrary stratification or random/low-discrepancy 2D points.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/09 - Point Lights, MIS and Environment Lighting|Next chapter]]
