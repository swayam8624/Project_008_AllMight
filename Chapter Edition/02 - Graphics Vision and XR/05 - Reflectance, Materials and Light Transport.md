# 05 — Reflectance, Materials and Light Transport

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/06 - From Material Data to GPU Bindings|Next chapter]]

A surface material describes how incoming energy is scattered toward outgoing directions. The chapter derives those responses, connects them to asset data, and checks mathematical properties before relying on rendered appearance.

**By the end:** Relate material parameters to a physically meaningful scattering model.

**Recall question:** Which numerical checks can reveal a broken BRDF before a scene render does?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 5493–9520; source-span SHA256 a991fb91656381fa229d00fa95405d65b41972810e12bb9f9316712472afb0b2. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Ask a surface where incoming energy can go

### Reconstruction checkpoint — test the microfacet equations, not only the picture

`tools/labs/pbr_reference_lab.cpp` numerically integrates the isotropic GGX normal distribution with the projected-area measure and checks that the result stays near one for several roughness values. It checks Schlick Fresnel at normal and grazing incidence and verifies reciprocity/finiteness of the isotropic GGX + Smith reflection model for paired incoming/outgoing directions.

Those are mathematical oracles, not a complete material renderer. White-furnace energy tests, transmission, anisotropic roughness, texture/color-space decoding and glTF scene parity remain separate. [[Microfacet normal distributions]] now develops why the NDF is a projected-area density rather than a drop-in directional PDF, so later sampling/MIS derivations have the right measure from the start.


A BRDF is a bidirectional reflectance distribution function. It relates irradiance arriving from one direction to radiance leaving in another. It is not a color by itself. For a diffuse surface with reflectance $\rho$, the Lambert BRDF is $\rho/\pi$. The factor $\pi$ is the integral of the cosine over a hemisphere, so the reflected fraction remains $\rho$ rather than accidentally becoming $\pi\rho$.

A rough metal panel has many unresolved surface slopes. A [[Concepts/Microfacet normal distributions|microfacet model]] describes those slopes statistically. Its normal distribution function $D$ is normalized with projected area:

$$
\int_{\Omega^+} D(m)(N\cdot m)\,d\omega_m=1.
$$

This is not generally the same as requiring $\int D(m)\,d\omega_m=1$. The probability density for sampling ordinary microfacet normals therefore includes the projected cosine. A visible-normal sampler also accounts for which facets can actually be seen from the current direction. [PBRT's microfacet derivation](https://pbr-book.org/4ed/Reflection_Models/Roughness_Using_Microfacet_Theory)

In a single-scattering reflection model, Fresnel $F$ describes interface reflection, masking-shadowing $G$ describes visibility of the facets, and the familiar $DFG/[4(N\cdot L)(N\cdot V)]$ converts the microscopic model to a macroscopic BRDF. A perceptual roughness-to-$\alpha$ mapping is a model convention: the $\alpha=r^2$ convention in these examples should not silently be applied to a library that already expects $\alpha$.

The common metallic-roughness shader is a useful approximation, but adding $(1-F)$-weighted Lambert diffuse to a microfacet lobe does not by itself prove exact physical layering or reciprocity. Conversely, a single-scattering rough-conductor model can lose energy because it omits repeated interreflection between facets. Use numerical integrals, documented model assumptions and controlled images together. A bright highlight alone is neither proof of energy gain nor proof of a correct BRDF: a narrow angular lobe can have values greater than one while its integrated reflectance remains bounded.

### Make the asset and the shader agree

A material travels through several representations: an authored asset, a decoded CPU record, an explicitly packed GPU record, resource bindings, and the shader's interpretation. Each transition can be correct in isolation and still disagree with the next. In glTF's core metallic-roughness workflow, base-color RGB uses sRGB texture encoding, roughness and metallic are linear data in the green and blue channels, and alpha has its own semantics. A shader's preferred premultiplied representation requires an explicit conversion from an asset's straight alpha. [The glTF material specification](https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/Specification.adoc) defines those asset encodings. The source below develops those contracts; general buffer ABI and descriptor-lifetime details are taught in [[Chapter Edition/01 - Computing Foundations/08 - Graphs, Search, and GPU Scheduling#Make a shader record an explicit binary contract|Make a shader record an explicit binary contract]].

Treat a texture index as a reference, not ownership. A recorded draw can still refer to that texture after the CPU has moved on to the next frame. The engine must retain the correct resource generation until every relevant GPU use completes. The corresponding engine lifecycle is developed in [[Chapter Edition/03 - Game Engine/25 - Renderer evolution through resource ownership#Let a renderer evolve without losing resource ownership|Let a renderer evolve without losing resource ownership]].

Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.


This pass covers the complete transition from **“a material has some sliders”** to a physically interpretable scattering model. M117 owns Parts **1945–1969**: material parameters, diffuse/specular reflection, BRDF definition and constraints, Fresnel/IOR, conductor-versus-dielectric behavior, microfacet theory, GGX/Beckmann, masking-shadowing, Smith geometry, Cook–Torrance, and the $4(N\!\cdot L)(N\!\cdot V)$ denominator. M118 owns Parts **1970–1986**: metallic-roughness and specular-gloss workflows, energy splitting, Disney diffuse, clearcoat, sheen, transmission, refraction, TIR, Beer–Lambert absorption, and emission.

The whole pass can be summarized by one transition:

$$
\boxed{
\text{artist parameters}
\longrightarrow
\text{physically constrained scattering function}
\longrightarrow
\text{outgoing radiance}
}
$$

We will use the convention

$$
N=\text{shading normal},
\qquad
L=\text{direction from surface toward light},
$$

$$
V=\text{direction from surface toward camera},
\qquad
H=\frac{L+V}{\|L+V\|}.
$$

All of these point **away from the surface**. Therefore, for visible front-facing reflection,

$$
N\cdot L>0,
\qquad
N\cdot V>0.
$$

That convention matters because graphics literature sometimes reverses incident directions.

---

**Material parameters are data; the shader is the scattering model**

A common conceptual mistake is to say:

> “Metallic, roughness and base color are the material.”

They are not.

They are **parameters supplied to a mathematical model**.

For example, Kairo currently stores approximately:

```cpp
struct PBRMaterial
{
    Vec3  BaseColor;
    float BaseColorAlpha;

    float Metallic;
    float Roughness;
    float AmbientOcclusion;

    Vec3  Emissive;
    float NormalScale;

    // textures...
};
```

None of these alone says how light interacts with the surface.

The shader decides that.

The same:

$$
\text{roughness}=0.4
$$

could drive GGX, Beckmann, Phong, Charlie sheen, a custom Gaussian lobe, or something completely nonphysical.

So the conceptual separation is:

$$
\boxed{
\text{material}
=
\text{parameters}
}
$$

while

$$
\boxed{
\text{BRDF/BSDF implementation}
=
\text{meaning assigned to those parameters}.
}
$$

That distinction becomes extremely important for asset interchange. A roughness texture only has predictable meaning if producer and consumer agree on the scattering model and roughness parameterization.

---

**Diffuse reflection is not “light multiplied by color”**

Consider light entering a material such as chalk, plastic paint or skin.

Some photons enter the surface, scatter internally, and eventually leave at different locations/directions.

At real-time rendering scales we often approximate that complicated subsurface process as **diffuse reflection at one point**.

The simplest idealization is Lambertian reflection.

Its defining property is:

$$
\boxed{
\text{outgoing radiance is independent of outgoing direction}
}
$$

not that the object “looks equally bright from all directions” in every possible perceptual sense.

The BRDF is constant with direction.

---

**Deriving the Lambertian BRDF**

Let diffuse reflectance or albedo be

$$
\rho.
$$

Physically:

$$
0\le\rho\le1.
$$

Suppose incident irradiance is

$$
E.
$$

A perfectly diffuse surface reflects total exitant power fraction

$$
\rho E.
$$

If outgoing radiance is constant over the hemisphere:

$$
L_o=C.
$$

Exitant radiant exitance is obtained by integrating outgoing radiance:

$$
M=
\int_{\Omega^+}
L_o(\omega_o)
(N\cdot\omega_o)
\,d\omega_o.
$$

Since $L_o=C$,

$$
M=
C
\int_{\Omega^+}
\cos\theta\,d\omega.
$$

From Pass 3,

$$
\int_{\Omega^+}
\cos\theta\,d\omega
=
\pi.
$$

Therefore:

$$
M=\pi C.
$$

But the reflected energy must be:

$$
M=\rho E.
$$

So:

$$
\pi C=\rho E
$$

and therefore:

$$
C=\frac{\rho}{\pi}E.
$$

Thus the Lambert BRDF is

$$
\boxed{
f_{\text{Lambert}}
=
\frac{\rho}{\pi}
}
$$

rather than merely $\rho$.

The $1/\pi$ is not an arbitrary “PBR correction.”

It is the normalization required so integrating the constant diffuse lobe over the hemisphere returns reflectance $\rho$.

---

**Why old shaders often omit $1/\pi$**

A historical real-time shader might compute:

$$
C=
\rho\,L_i\,\max(N\cdot L,0).
$$

That can look perfectly usable because light intensities are artistically tuned around the missing constant.

But a physically interpretable BRDF should be:

$$
C=
\frac{\rho}{\pi}
L_i
(N\cdot L).
$$

If your light intensity conventions were authored around the first equation, suddenly introducing $1/\pi$ makes everything about 3.14 times darker.

This is why physically based rendering is not achieved by swapping one shader function.

The **entire lighting calibration** has to agree.

---

**BRDF definition**

BRDF means:

$$
\text{Bidirectional Reflectance Distribution Function}.
$$

It answers:

> Given radiance arriving from direction $\omega_i$, how much outgoing radiance is generated toward $\omega_o$?

Formally:

$$
\boxed{
f_r(\omega_i,\omega_o)
=
\frac{dL_o(\omega_o)}
{dE_i(\omega_i)}
}
$$

but

$$
dE_i
=
L_i(\omega_i)
(N\cdot\omega_i)
d\omega_i.
$$

Therefore:

$$
\boxed{
f_r(\omega_i,\omega_o)
=
\frac{
dL_o(\omega_o)
}{
L_i(\omega_i)
(N\cdot\omega_i)
d\omega_i
}
}
$$

and consequently:

$$
dL_o
=
f_r
L_i
(N\cdot\omega_i)
d\omega_i.
$$

Integrating all incoming directions gives the reflection part of the rendering equation:

$$
\boxed{
L_o(\omega_o)
=
\int_{\Omega^+}
f_r(\omega_i,\omega_o)
L_i(\omega_i)
(N\cdot\omega_i)
d\omega_i.
}
$$

This equation is the backbone of essentially everything from direct PBR shading to path tracing.

---

**BRDF units**

Radiance units are:

$$
[L]
=
\frac{\mathrm W}
{\mathrm{m^2\,sr}}.
$$

Irradiance units are:

$$
[E]
=
\frac{\mathrm W}{\mathrm{m^2}}.
$$

Therefore:

$$
[f_r]
=
\frac{L}{E}
$$

so:

$$
[f_r]
=
\frac{
\mathrm W\,m^{-2}\,sr^{-1}
}{
\mathrm W\,m^{-2}
}
$$

giving:

$$
\boxed{
[f_r]=\mathrm{sr}^{-1}.
}
$$

That is why a BRDF is not literally a reflectance percentage.

It is a directional density.

---

**BRDF reciprocity**

For passive reciprocal materials under ordinary optical assumptions:

$$
\boxed{
f_r(\omega_i,\omega_o)
=
f_r(\omega_o,\omega_i)
}
$$

This is Helmholtz reciprocity.

If light can travel from direction $A$ to direction $B$ through the material's scattering behavior, reversing the path has the same reflectance.

Lambert satisfies this trivially:

$$
\frac{\rho}{\pi}
=
\frac{\rho}{\pi}.
$$

Cook–Torrance microfacet reflection is also constructed to satisfy reciprocity when its terms are symmetric.

This property becomes especially useful in light transport algorithms because camera paths and light paths can sometimes be reversed mathematically.

---

**BRDF energy conservation**

A surface must not reflect more energy than it receives unless it is actively emitting energy.

For every incoming direction $\omega_i$,

$$
\boxed{
\int_{\Omega^+}
f_r(\omega_i,\omega_o)
(N\cdot\omega_o)
\,d\omega_o
\le1.
}
$$

For Lambert:

$$
f_r=\frac{\rho}{\pi}.
$$

Then:

$$
\int_{\Omega^+}
\frac{\rho}{\pi}
\cos\theta_o
d\omega_o
=
\frac{\rho}{\pi}\pi
=
\rho.
$$

So energy conservation becomes simply:

$$
\rho\le1.
$$

That is why Lambert's normalization works.

---

**Reflectance can be colored**

For RGB:

$$
\rho=
(\rho_r,\rho_g,\rho_b).
$$

Energy conservation applies spectrally; in an RGB approximation we enforce it channel-wise.

For example:

$$
\rho=(0.8,0.2,0.1)
$$

means the material returns much more red-band energy than blue-band energy.

It does not mean the surface creates red light.

It selectively absorbs other wavelengths.

---

**Specular reflection**

For a perfectly smooth mirror, one incident direction maps into exactly one reflected direction.

Let incoming ray direction $I$ point **toward** the surface and normal $N$ point outward.

Reflection is:

$$
\boxed{
R=
I-2(I\cdot N)N
}
$$

If instead using our PBR convention where $L$ points outward toward the light, the incident travel direction is:

$$
I=-L.
$$

For perfect mirror reflection viewed along $V$,

$$
V=R.
$$

Equivalently, the microfacet normal that reflects $L$ into $V$ must align with the half vector:

$$
\boxed{
H=
\frac{L+V}{\|L+V\|}.
}
$$

---

**A perfect mirror BRDF is a delta distribution**

A perfect mirror does not have an ordinary finite-width specular lobe.

All reflected energy goes into one direction.

Mathematically the BRDF contains a Dirac delta.

Informally:

$$
f_{\text{mirror}}
\sim
F\,\delta(\omega_o-R(\omega_i)).
$$

This is why a perfect mirror cannot be correctly represented by “roughness = exactly zero” in a naive finite GGX formula without care.

As roughness approaches zero, the microfacet distribution approaches a delta-like spike.

Kairo therefore clamps real-time roughness to:

$$
0.04
$$

to avoid a numerically singular distribution.

That is a pragmatic real-time choice, not a statement that physically perfect mirrors do not exist.

---

**Fresnel reflection**

When electromagnetic radiation reaches an interface between two media, some energy reflects and some transmits.

The fraction depends on:

$$
\eta_i,\eta_t,
$$

the incoming and transmitted refractive indices, and on incidence angle.

Even perfectly smooth transparent glass reflects some light.

This means:

> “Transparent” does not mean “no reflection.”

At grazing angles, even ordinary glass becomes strongly reflective.

That is Fresnel behavior.

---

**Exact dielectric Fresnel equations**

For nonconductive media, separate light into perpendicular $s$ and parallel $p$ polarizations.

Let:

$$
\theta_i=\text{incident angle},
$$

$$
\theta_t=\text{transmitted angle}.
$$

Then amplitude-independent power reflectances are:

$$
\boxed{
R_s=
\left(
\frac{
\eta_i\cos\theta_i-\eta_t\cos\theta_t
}{
\eta_i\cos\theta_i+\eta_t\cos\theta_t
}
\right)^2
}
$$

and

$$
\boxed{
R_p=
\left(
\frac{
\eta_i\cos\theta_t-\eta_t\cos\theta_i
}{
\eta_i\cos\theta_t+\eta_t\cos\theta_i
}
\right)^2.
}
$$

For unpolarized light:

$$
\boxed{
F=
\frac{R_s+R_p}{2}.
}
$$

These are physically accurate dielectric Fresnel equations.

Real-time rendering usually avoids evaluating them directly.

---

**Fresnel at normal incidence**

At normal incidence:

$$
\theta_i=\theta_t=0.
$$

Therefore:

$$
\cos\theta_i=\cos\theta_t=1.
$$

Both polarization equations collapse to:

$$
\boxed{
F_0=
\left(
\frac{\eta_i-\eta_t}
{\eta_i+\eta_t}
\right)^2.
}
$$

For air:

$$
\eta_i\approx1.
$$

For common glass:

$$
\eta_t\approx1.5.
$$

Then:

$$
F_0=
\left(
\frac{1-1.5}
{1+1.5}
\right)^2
$$

$$
=
\left(
\frac{-0.5}{2.5}
\right)^2
$$

$$
=
0.04.
$$

Thus:

$$
\boxed{F_0\approx4\%}
$$

for IOR 1.5.

That is the origin of the ubiquitous:

```glsl
vec3 F0 = vec3(0.04);
```

It is not a magic PBR constant.

It corresponds to a dielectric around IOR 1.5.

---

**Inverting $F_0$ back into IOR**

For air-to-dielectric:

$$
F_0=
\left(
\frac{1-\eta}
{1+\eta}
\right)^2.
$$

Let:

$$
r=\sqrt{F_0}.
$$

For $\eta>1$,

$$
r=
\frac{\eta-1}{\eta+1}.
$$

Solve:

$$
r(\eta+1)=\eta-1
$$

$$
r\eta+r=\eta-1
$$

$$
\eta-r\eta=1+r
$$

$$
\eta(1-r)=1+r.
$$

Therefore:

$$
\boxed{
\eta=
\frac{1+\sqrt{F_0}}
{1-\sqrt{F_0}}.
}
$$

So $F_0$ and dielectric IOR are simply alternative parameterizations.

---

**Schlick Fresnel**

Schlick's approximation is:

$$
\boxed{
F(\cos\theta)
=
F_0
+
(1-F_0)(1-\cos\theta)^5.
}
$$

In microfacet reflection, use:

$$
\cos\theta=
V\cdot H
$$

or equivalently $L\cdot H$ under ideal reflection geometry.

At normal incidence:

$$
\cos\theta=1
$$

so:

$$
F=F_0.
$$

At grazing incidence:

$$
\cos\theta\rightarrow0
$$

so:

$$
F\rightarrow1.
$$

That reproduces the major physical behavior extremely cheaply.

Kairo currently implements exactly:

```glsl
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 +
           (1.0 - F0) *
           pow(1.0 - cosTheta, 5.0);
}
```

---

**Refractive index**

Refractive index is:

$$
\boxed{
\eta=\frac{c}{v}
}
$$

where $c$ is light speed in vacuum and $v$ is phase velocity in the material.

Typical values are roughly:

$$
\eta_{\text{air}}\approx1.0003,
$$

$$
\eta_{\text{water}}\approx1.33,
$$

$$
\eta_{\text{glass}}\approx1.45-1.6,
$$

$$
\eta_{\text{diamond}}\approx2.42.
$$

IOR controls both:

$$
\text{refraction direction}
$$

and

$$
F_0.
$$

So if a material exposes both arbitrary IOR and arbitrary $F_0$, they may become physically inconsistent unless deliberately treated as artistic parameters.

---

**Dielectrics versus conductors**

A dielectric such as:

```text
plastic
glass
ceramic
wood coating
water
```

allows electromagnetic fields to penetrate substantially before absorption/scattering.

Its interface specular reflection is usually nearly achromatic in visible RGB for many common materials.

Its visible body color generally comes from transmitted light being absorbed/scattered internally.

Thus for an opaque dielectric:

$$
\boxed{
\text{baseColor mainly controls diffuse/body reflection}
}
$$

while:

$$
\boxed{
F_0\text{ is usually low and near-neutral}.
}
$$

A conductor such as:

```text
gold
copper
silver
aluminium
```

has mobile electrons and strongly absorbs transmitted electromagnetic energy.

Its refractive index is complex:

$$
\boxed{
\tilde{\eta}
=
\eta+i\kappa
}
$$

where:

$$
\eta=\text{ordinary refractive-index term},
$$

$$
\kappa=\text{extinction coefficient}.
$$

This produces wavelength-dependent interface reflection.

Consequently metals often have **colored specular reflections**.

---

**Why metals have essentially no ordinary diffuse lobe**

For a dielectric, light can enter the material, scatter, and escape.

That gives body/diffuse reflection.

For a conductor, visible light entering the medium is rapidly absorbed.

Therefore an idealized opaque metal is modeled approximately as:

$$
\boxed{
f_{\text{metal}}
\approx
f_{\text{specular}}
}
$$

with:

$$
f_{\text{diffuse}}\approx0.
$$

This is why metallic-roughness systems multiply diffuse contribution by:

$$
1-\text{metallic}.
$$

---

**Colored $F_0$ for metals**

Instead of:

$$
F_0=(0.04,0.04,0.04),
$$

a metal may have:

$$
F_0=(0.95,0.64,0.54)
$$

or another strongly colored triplet depending on the conductor.

In metallic-roughness rendering, we approximate this through:

$$
\boxed{
F_0=
(1-m)F_{0,\text{dielectric}}
+
mC_{\text{base}}.
}
$$

For:

$$
m=0
$$

we obtain dielectric:

$$
F_0=0.04.
$$

For:

$$
m=1
$$

we obtain:

$$
F_0=C_{\text{base}}.
$$

Current Kairo does exactly:

```glsl
vec3 f0 =
    mix(vec3(0.04),
        baseColor,
        metallic);
```

---

**Microfacet theory**

A rough surface appears macroscopically smooth-ish, but microscopically it consists of many tiny mirror-like facets.

Each microfacet has its own normal:

$$
M.
$$

Every individual microfacet is assumed to reflect specularly.

A visible highlight arises because some subset of those facets happen to have the exact orientation needed to reflect $L$ into $V$.

For perfect reflection that orientation is:

$$
M=H.
$$

Therefore the BRDF asks:

> How many microfacets have normals close to $H$, how much Fresnel reflection do they produce, and how many are visible rather than masked or shadowed?

This gives the three famous microfacet terms:

$$
\boxed{
D\,F\,G.
}
$$

---

**The microfacet normal distribution**

The NDF:

$$
D(M)
$$

describes the statistical distribution of microfacet orientations.

More precisely, $D(M)$ is an area-density-like function over microfacet normals.

A proper isotropic NDF obeys a projected-area normalization:

$$
\boxed{
\int_{\Omega^+}
D(M)(N\cdot M)
\,d\omega_M
=
1.
}
$$

That cosine matters.

$D$ is not simply a probability density over directions.

If you need an actual microfacet-normal PDF sampled proportional to projected microfacet area, then often:

$$
p(M)=D(M)(N\cdot M).
$$

Later, importance sampling will depend on precisely this distinction.

---

**Roughness controls the NDF**

At tiny roughness:

```text
most microfacets ≈ macro normal
```

so:

$$
D(H)
$$

is highly concentrated around:

$$
H=N.
$$

The highlight becomes:

- small,
- sharp,
- intense.

At larger roughness:

```text
microfacet normals distributed broadly
```

so the lobe becomes:

- wider,
- dimmer at the peak,
- visible over more reflection directions.

Energy is being redistributed angularly.

Roughness should not simply “reduce reflection strength.”

---

**GGX / Trowbridge-Reitz**

The isotropic GGX NDF can be written:

$$
\boxed{
D_{\mathrm{GGX}}(N,H,\alpha)
=
\frac{
\alpha^2
}{
\pi
\left[
(N\cdot H)^2(\alpha^2-1)+1
\right]^2
}
}
$$

for:

$$
N\cdot H>0.
$$

Equivalent tangent-angle form:

$$
D(\theta_h)
=
\frac{
\alpha^2
}{
\pi
\cos^4\theta_h
(\alpha^2+\tan^2\theta_h)^2
}.
$$

The first form is much nicer for shader implementation.

Current Kairo:

```glsl
float DistributionGGX(
    vec3 N,
    vec3 H,
    float roughness)
{
    float alpha  = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH =
        max(dot(N, H), 0.0);

    float d =
        NdotH * NdotH *
        (alpha2 - 1.0) + 1.0;

    return alpha2 /
        (Pi * d * d);
}
```

That is the standard GGX structure.

---

**Why GGX became so popular**

GGX has a relatively long tail.

That means noticeably tilted microfacets remain more probable than with distributions such as Beckmann.

Visually this creates:

- broader grazing highlights,
- realistic long specular tails,
- pleasing rough surfaces.

Real surfaces often exhibit this kind of long-tail behavior more convincingly than classical Phong-like distributions.

It also has tractable sampling and masking-shadowing formulas.

That combination made GGX an excellent production choice.

---

**Beckmann distribution**

A classic isotropic Beckmann NDF is:

$$
\boxed{
D_{\mathrm B}(\theta_h)
=
\frac{
\exp\left(
-\tan^2\theta_h/\alpha^2
\right)
}{
\pi\alpha^2\cos^4\theta_h
}.
}
$$

Its tail falls exponentially.

GGX falls much more slowly.

Therefore, far from the normal:

$$
D_{\mathrm{GGX}}
>
D_{\mathrm{Beckmann}}
$$

for comparable roughness in many regions.

In practical visual language:

```text
Beckmann → shorter/tighter tail
GGX      → heavier/longer specular tail
```

Neither is “universally more physically correct.”

They represent different statistical surface models.

---

**Perceptual roughness versus microfacet $\alpha$**

A common production mapping is:

$$
\boxed{
\alpha=r^2
}
$$

where:

$$
r\in[0,1]
$$

is artist-facing roughness.

Why square it?

Because direct use of $\alpha$ tends not to produce a perceptually uniform slider.

Squaring gives artists finer control near the smooth region where small changes strongly alter highlights.

Kairo uses exactly:

```glsl
alpha = roughness * roughness;
```

inside GGX.

Then the formula uses:

$$
\alpha^2.
$$

Therefore shader code may contain:

```glsl
float alpha = roughness * roughness;
float alphaSquared = alpha * alpha;
```

which means:

$$
\alpha^2=r^4.
$$

That is intentional.

Do not “simplify” it because you think roughness was accidentally squared twice.

---

**A roughness naming trap**

Different codebases use names differently.

One engine might call:

```text
roughness = artist parameter r
alpha     = r²
```

another might call:

```text
roughness = alpha
```

and another might pass:

```text
alphaSquared
```

directly.

Therefore seeing:

```cpp
a = roughness * roughness;
a2 = a * a;
```

cannot be judged without knowing parameter semantics.

Many supposed PBR bugs are naming bugs.

---

**Microfacet masking and shadowing**

Imagine a rough surface seen at grazing angle.

Some microfacets oriented toward the camera are blocked by neighboring microfacets.

That is **masking**.

Likewise, some microfacets oriented toward the light cannot receive that light because another microfacet blocks it.

That is **shadowing**.

The NDF $D$ alone counts microfacet orientations regardless of whether those microfacets are actually visible from both directions.

So we need:

$$
\boxed{
G(L,V)
}
$$

with:

$$
0\le G\le1.
$$

---

**Smith masking-shadowing**

Smith theory builds the two-direction geometry function from one-direction visibility terms:

$$
\boxed{
G(L,V)
=
G_1(L)\,G_1(V)
}
$$

for a common separable approximation.

A GGX-compatible exact-ish isotropic $G_1$ can be written:

$$
\boxed{
G_1(V)
=
\frac{
2(N\cdot V)
}{
(N\cdot V)
+
\sqrt{
\alpha^2+
(1-\alpha^2)(N\cdot V)^2
}
}
}
$$

for positive $N\cdot V$.

Equivalent Smith lambda formulations are also common.

The geometric interpretation remains:

$$
G_1(V)
=
\text{fraction of relevant microfacets visible from }V.
$$

---

**Schlick-GGX approximation**

A cheaper approximation is:

$$
\boxed{
G_{\text{Schlick}}(x)
=
\frac{x}
{x(1-k)+k}
}
$$

where:

$$
x=N\cdot V
$$

or:

$$
N\cdot L.
$$

Then:

$$
G=
G_{\text{Schlick}}(N\cdot L)
G_{\text{Schlick}}(N\cdot V).
$$

A common direct-lighting remap is:

$$
\boxed{
k=\frac{(r+1)^2}{8}.
}
$$

That is exactly what current Kairo uses:

```glsl
float k =
    ((roughness + 1.0) *
     (roughness + 1.0)) / 8.0;
```

This is an approximation to Smith GGX, not the NDF itself.

---

**Cook–Torrance microfacet BRDF**

We now combine:

$$
D=\text{microfacet orientation density},
$$

$$
F=\text{Fresnel reflection},
$$

$$
G=\text{masking-shadowing}.
$$

The specular microfacet BRDF is:

$$
\boxed{
f_s(L,V)
=
\frac{
D(H)F(V,H)G(L,V)
}{
4(N\cdot L)(N\cdot V)
}
}
$$

for valid directions in the upper hemisphere.

That is the Cook–Torrance-style microfacet reflection model.

Every term has a geometric or electromagnetic meaning.

---

**Why the denominator exists at all**

This denominator is one of the most memorized and least understood parts of PBR.

It is not:

> “a normalization number somebody discovered experimentally.”

The microfacet model counts surface area in one geometric domain and predicts radiance in another.

Several coordinate/projection conversions are required.

The key ingredients are:

$$
\boxed{
\text{macro-surface projected area}
}
$$

versus:

$$
\boxed{
\text{microfacet projected area}
}
$$

and the transformation between:

$$
\boxed{
\text{half-vector solid angle}
}
$$

and:

$$
\boxed{
\text{reflected outgoing-direction solid angle}.
}
$$

For perfect specular reflection:

$$
H=
\frac{L+V}{\|L+V\|}.
$$

The reflection mapping has the Jacobian:

$$
\boxed{
d\omega_H
=
\frac{
d\omega_V
}{
4|V\cdot H|
}.
}
$$

That factor $4$ is one crucial part of the final denominator.

The remaining projected-area conversions produce the $N\cdot L$ and $N\cdot V$ terms.

After converting microfacet area distribution into reflected outgoing radiance per macroscopic projected area and outgoing solid angle, the factors reduce to:

$$
\boxed{
4(N\cdot L)(N\cdot V)
}.
$$

The half-vector dot factors arising during the intermediate microfacet projection/Jacobian transformation cancel for ideal reflection because:

$$
L\cdot H=V\cdot H.
$$

The result is therefore exactly:

$$
f_s=
\frac{DFG}
{4(N\cdot L)(N\cdot V)}.
$$

The important conceptual memory is:

$$
\boxed{
4N_LN_V
=
\text{geometry/Jacobian normalization required to express microscopic mirror area as a macroscopic BRDF}.
}
$$

---

**Why the denominator becomes numerically dangerous**

At grazing angles:

$$
N\cdot L\rightarrow0
$$

or

$$
N\cdot V\rightarrow0.
$$

The denominator approaches zero.

A naive implementation can produce:

$$
\infty
$$

or:

$$
NaN.
$$

Production code therefore typically checks:

$$
N\cdot L>0,
\qquad
N\cdot V>0
$$

and clamps the denominator:

```glsl
max(
    4.0 * NdotL * NdotV,
    1e-5);
```

Kairo does exactly this.

This epsilon is numerical protection, not part of the physical BRDF.

---

**The rendering equation with Cook–Torrance**

For one point light/directional sample:

$$
L_o
\approx
\left(
f_d+f_s
\right)
L_i
(N\cdot L).
$$

For Lambert + Cook–Torrance:

$$
L_o
=
\left[
k_d\frac{C}{\pi}
+
\frac{DFG}
{4N_LN_V}
\right]
L_iN_L.
$$

The external:

$$
N_L
$$

comes from the rendering equation.

Do not accidentally include another cosine inside the BRDF unless the model itself requires it.

This is one of the most common shader errors.

---

**Material workflow: metallic-roughness**

The metallic-roughness workflow attempts to encode a wide range of opaque materials using relatively intuitive artist parameters:

$$
C_{\text{base}},
\qquad
m,
\qquad
r.
$$

Where:

$$
m=\text{metallic},
\qquad
r=\text{roughness}.
$$

This is the workflow used by glTF's core PBR model and many engines.

Its genius is not that nature literally contains a “metallic slider.”

It is that the slider efficiently selects between two major material classes:

$$
\text{dielectric}
\leftrightarrow
\text{conductor}.
$$

---

**Base-color semantics change with metallic**

For dielectric:

$$
m=0.
$$

Base color represents approximate diffuse/body reflectance:

$$
C_d=C_{\text{base}}.
$$

Specular normal-incidence reflectance remains near:

$$
F_0\approx0.04.
$$

For metal:

$$
m=1.
$$

Diffuse becomes approximately zero.

Base color becomes specular normal-incidence color:

$$
F_0=C_{\text{base}}.
$$

Thus the same input field changes physical role depending on material class.

---

**Metallic should usually be nearly binary**

Most homogeneous real materials are:

$$
m=0
$$

or:

$$
m=1.
$$

Intermediate metallic values are useful around transitions:

- dirty metal,
- rust boundaries,
- antialiased texture edges,
- composite pixels.

But a uniformly “0.43 metallic plastic” is usually not a literal physical material.

It is an interpolation between two models.

This matters for texture authoring.

---

**Roughness semantics**

Roughness controls microfacet orientation spread.

It does **not** fundamentally control how much total specular energy exists.

At lower roughness:

```text
narrow + intense highlight
```

At higher roughness:

```text
broad + weaker peak
```

The lobe redistributes energy over angle.

In practical single-scattering microfacet models, some energy can be lost due to unresolved multiple scattering between facets, especially at high roughness, but the intended semantic remains angular spread.

---

**Metallic and roughness are independent axes**

Consider four material combinations:

| Metallic | Roughness | Appearance |
|---:|---:|---|
| 0 | low | glossy plastic / polished dielectric |
| 0 | high | matte plastic / rough dielectric |
| 1 | low | polished metal |
| 1 | high | rough/brushed-looking isotropic metal |

“Shiny” does not mean “metal.”

A polished black plastic can have a strong clear reflection while still being:

$$
m=0.
$$

That distinction is essential to believable material authoring.

---

**Ambient occlusion input**

Material AO is often interpreted approximately as:

$$
A\in[0,1]
$$

representing reduced accessibility of indirect environmental illumination.

The most important practical rule is:

$$
\boxed{
\text{AO generally should not simply darken direct lights}.
}
$$

A cavity can be strongly occluded from broad environment lighting but still receive a direct flashlight beam.

Kairo currently applies AO around its ambient/environment expression:

```glsl
ambient =
    (... ambient/environment ...)
    * ambientOcclusion;
```

while direct light remains outside that multiplication.

That is the right broad conceptual placement.

---

**Specular-glossiness workflow**

Another PBR parameterization uses:

$$
C_{\text{diffuse}},
$$

$$
C_{\text{specular}},
$$

$$
g=\text{glossiness}.
$$

Glossiness is roughly inverse roughness:

$$
r\approx1-g
$$

though exact conversions depend on the model.

This workflow provides direct artistic control over specular color.

But it makes it easier to author physically inconsistent combinations such as:

```text
bright diffuse
+
very bright specular
```

whose summed reflected energy exceeds one.

Metallic-roughness reduces some of this freedom in exchange for stronger physical plausibility.

---

**Why workflow conversion is lossy**

Suppose a specular-gloss material has:

$$
C_s=(0.2,0.1,0.05)
$$

and a nonzero diffuse color.

Is it:

- a dielectric with unusual specular tint,
- partially metallic,
- a layered material,
- an artistic approximation?

Metallic-roughness has fewer independent degrees of freedom.

Therefore converting from spec-gloss to metallic-roughness can require approximation.

Parameterizations are not always bijections.

---

**The diffuse/specular energy split**

Fresnel tells us fraction:

$$
F
$$

of incident interface energy that reflects specularly.

For an opaque dielectric, the remaining fraction available to enter the material is approximately:

$$
1-F.
$$

Some of that later returns as diffuse/body scattering.

Therefore a common real-time energy split is:

$$
\boxed{
k_s=F
}
$$

and:

$$
\boxed{
k_d=(1-F)(1-m).
}
$$

Then:

$$
f=
k_d\frac{C_{\text{base}}}{\pi}
+
f_s.
$$

The metallic factor ensures:

$$
m=1
\Rightarrow
k_d=0.
$$

---

**Current Kairo real-time shader does this correctly**

Your real-time fragment shader computes:

```glsl
vec3 fresnel = FresnelSchlick(...);

vec3 diffuseWeight =
    (vec3(1.0) - fresnel)
    * (1.0 - metallic);
```

and then:

```glsl
direct +=
    (
        diffuseWeight *
        baseColor / Pi
        +
        specular
    )
    *
    radiance *
    NdotL *
    visibility;
```

That is a coherent direct-light energy split.

---

**A useful repository discrepancy: KairoRayTracer's PBR direct path**

The current ray tracer uses:

```cpp
const Color3f diffuse =
    albedo *
    ((1.0f - metallic) / pi);
```

and then adds the full microfacet specular.

Notice what is absent:

$$
1-F.
$$

So the ray-tracer direct PBR implementation currently uses:

$$
f_d=
(1-m)\frac{C}{\pi}
$$

rather than:

$$
f_d=
(1-F)(1-m)\frac{C}{\pi}.
$$

That means its dielectric direct response is less strictly energy-partitioned than the real-time Kairo shader.

It can return diffuse energy without subtracting interface Fresnel energy.

That is a concrete implementation discrepancy worth fixing if you want renderer agreement.

---

**Diffuse normalization again**

If an artist chooses base-color reflectance:

$$
C=(0.8,0.2,0.1),
$$

the BRDF is not:

$$
f_d=C.
$$

It is:

$$
\boxed{
f_d=\frac{C}{\pi}.
}
$$

Why?

Because integrating the lobe must recover:

$$
C.
$$

This is the fundamental interpretation:

$$
\boxed{
\text{base color}
\approx
\text{hemispherical diffuse reflectance}
}
$$

not the BRDF magnitude itself.

---

**Lambert is not perfect for rough diffuse surfaces**

Lambert assumes outgoing diffuse radiance independent of view direction.

Real rough surfaces show effects such as:

- retroreflection,
- grazing-angle changes,
- micro-scale shadowing.

Oren–Nayar models some of this from rough diffuse facets.

Disney/Burley diffuse provides a simpler production-oriented correction.

---

**Disney/Burley diffuse**

A commonly used Burley diffuse term is:

$$
\boxed{
f_d=
\frac{C}{\pi}
F_{D,L}
F_{D,V}
}
$$

where:

$$
F_{D,L}
=
1+
(F_{D90}-1)(1-N\cdot L)^5
$$

and:

$$
F_{D,V}
=
1+
(F_{D90}-1)(1-N\cdot V)^5.
$$

A common Disney definition is:

$$
\boxed{
F_{D90}
=
0.5+
2r(L\cdot H)^2.
}
$$

This creates view/light-angle-dependent rough diffuse response.

When roughness is low and geometry is near normal incidence, behavior remains close to Lambert.

At grazing angles and rougher surfaces, the correction becomes more noticeable.

---

**Why Disney diffuse exists**

The goal is not merely artistic complexity.

Lambert tends to look too flat for many rough materials.

Burley's model was designed from measured/material-fitting observations to capture useful retroreflective and grazing behavior with little computational cost.

Thus:

$$
\boxed{
\text{Lambert}
=
\text{simple physically normalized baseline}
}
$$

while:

$$
\boxed{
\text{Burley diffuse}
=
\text{production-friendly rough diffuse approximation}.
}
$$

---

**Layered materials**

Real materials often contain several scattering layers.

For example, car paint may involve:

```text
air
→ clear varnish
→ colored basecoat
→ metallic flakes / substrate
```

Trying to represent all of that using one GGX lobe is insufficient.

Production models therefore add secondary lobes such as:

$$
\text{clearcoat}
$$

and:

$$
\text{sheen}.
$$

These lobes must ideally obey energy conservation with the base layer.

---

**Clearcoat**

Clearcoat models a thin, smooth dielectric layer above a base material.

Think:

- car paint,
- varnished wood,
- lacquer,
- coated carbon fiber.

The clearcoat lobe generally has:

- low dielectric $F_0$,
- independent roughness/gloss,
- relatively narrow specular behavior,
- reduced underlying-layer energy because some light is reflected by the coat first.

Conceptually:

$$
L=
L_{\text{coat}}
+
(1-F_{\text{coat,in}})
(1-F_{\text{coat,out}})
L_{\text{base}}
$$

in a more careful layered treatment.

Simple real-time models approximate this.

---

**GTR1 clearcoat concept**

Disney clearcoat is commonly represented with a GTR1-like distribution rather than the ordinary GGX/GTR2 distribution.

A common GTR1 NDF is:

$$
D_{\mathrm{GTR1}}
=
\frac{
\alpha^2-1
}{
\pi
\ln(\alpha^2)
\left[
1+(\alpha^2-1)(N\cdot H)^2
\right]
}
$$

for $\alpha\neq1$.

You do not need to memorize this equation for M118.

The important architectural concept is:

$$
\boxed{
\text{one material may contain multiple physically meaningful reflection lobes with different roughness distributions}.
}
$$

Current Kairo's real-time material does not yet expose clearcoat.

---

**Sheen**

Sheen models soft grazing reflection seen in materials such as cloth and fabrics.

Examples:

- velvet,
- satin-like surfaces,
- fibres.

Unlike clearcoat, it is not simply another glossy varnish.

It approximates scattering caused by microfibres and grazing-oriented structures.

A simple Disney-style sheen contribution is related to:

$$
(1-L\cdot H)^5
$$

multiplied by a sheen color/tint.

Thus it tends to appear primarily at grazing configurations.

Current Kairo does not yet expose a sheen lobe.

---

**Why just “adding lobes” can violate energy conservation**

Suppose:

$$
f=
f_{\text{diffuse}}
+
f_{\text{specular}}
+
f_{\text{clearcoat}}
+
f_{\text{sheen}}.
$$

If each lobe independently assumes it can reflect 100% of incoming energy, their sum may exceed one.

Correct layered models account for energy removed by upper layers before lower layers receive illumination.

In real-time engines, this is often approximated rather than solved spectrally/exactly, but you should always ask:

$$
\boxed{
\text{Where did the energy for this additional lobe come from?}
}
$$

That question catches many “looks nice but glows mysteriously” shaders.

---

**Transmission materials**

Until now our BRDF handled only reflection.

But glass sends substantial energy to the opposite side of the surface.

A full surface scattering model that supports reflection and transmission is a **BSDF**:

$$
\text{Bidirectional Scattering Distribution Function}.
$$

It contains:

$$
f_r
$$

for reflection and:

$$
f_t
$$

for transmission.

A BRDF lives on the same hemisphere as outgoing reflection.

A BTDF maps between opposite hemispheres.

The combined object is the BSDF.

---

**Thin transmission**

A thin glass sheet can be approximated as having negligible thickness.

Examples:

- window pane treated as infinitely thin,
- thin plastic film,
- foliage transmission approximations.

The renderer may perform a direction change or even simply straight-through transmission depending on the model, without tracing meaningful internal path length.

Because thickness is absent, volumetric attenuation cannot naturally depend on travel distance through the object.

---

**Volumetric transmission**

A solid glass object has two interfaces:

```text
air → glass
glass → air
```

Between them, light travels through the medium.

Therefore rendering requires:

- entering refraction,
- internal propagation,
- absorption/scattering,
- exiting refraction.

The path length inside the medium changes with geometry and view direction.

A thick green bottle cannot be reproduced correctly using only a surface alpha value.

Its color should become stronger where light travels through more glass.

That requires volumetric attenuation.

---

**Snell's law**

Refraction obeys:

$$
\boxed{
\eta_i\sin\theta_i
=
\eta_t\sin\theta_t.
}
$$

Define:

$$
\eta=
\frac{\eta_i}{\eta_t}.
$$

Then:

$$
\sin\theta_t=
\eta\sin\theta_i.
$$

Since:

$$
\sin^2\theta
=
1-\cos^2\theta,
$$

we obtain:

$$
\sin^2\theta_t
=
\eta^2
(1-\cos^2\theta_i).
$$

Therefore:

$$
\boxed{
\cos\theta_t
=
\sqrt{
1-
\eta^2
(1-\cos^2\theta_i)
}.
}
$$

That square root will appear directly in the vector formula.

---

**Deriving the refraction vector**

Let $I$ be normalized incident travel direction pointing toward the surface.

Let $N$ point into the incident medium.

Define:

$$
c_i=-N\cdot I.
$$

Split incident direction into tangential and normal components.

Tangential component:

$$
I_\parallel
=
I+c_iN.
$$

Its magnitude is:

$$
\sin\theta_i.
$$

Snell scales the tangential component by:

$$
\eta=\frac{\eta_i}{\eta_t}.
$$

Therefore transmitted tangential component is:

$$
T_\parallel=
\eta(I+c_iN).
$$

The transmitted normal magnitude is:

$$
c_t=
\sqrt{
1-\eta^2(1-c_i^2)
}.
$$

The transmitted vector points into the other medium, so normal component is:

$$
-c_tN.
$$

Thus:

$$
T=
\eta(I+c_iN)-c_tN.
$$

Collect $N$:

$$
\boxed{
T=
\eta I+
(\eta c_i-c_t)N.
}
$$

with:

$$
\boxed{
c_t=
\sqrt{
1-\eta^2(1-c_i^2)
}.
}
$$

This is the standard vector refraction equation.

---

**Canonical refraction implementation**

```cpp
std::optional<Vec3> Refract(
    const Vec3& incident,
    const Vec3& normal,
    float etaIncident,
    float etaTransmitted)
{
    // incident points toward the interface.
    // normal points into the incident medium.

    const float eta =
        etaIncident / etaTransmitted;

    const float cosIncident =
        std::clamp(
            -Dot(normal, incident),
            0.0f,
            1.0f);

    // Snell:
    // sin²(theta_t)
    const float sin2Transmitted =
        eta * eta *
        (1.0f -
         cosIncident * cosIncident);

    // No real transmitted angle exists.
    if (sin2Transmitted > 1.0f)
        return std::nullopt;

    const float cosTransmitted =
        std::sqrt(
            1.0f -
            sin2Transmitted);

    return Normalize(
        eta * incident +
        (
            eta * cosIncident -
            cosTransmitted
        ) * normal);
}
```

The `nullopt` case is total internal reflection.

---

**Total internal reflection**

Suppose light travels from high IOR to low IOR:

$$
\eta_i>\eta_t.
$$

From Snell:

$$
\sin\theta_t
=
\frac{\eta_i}{\eta_t}
\sin\theta_i.
$$

For sufficiently large $\theta_i$, the right side exceeds 1.

But:

$$
\sin\theta_t\le1.
$$

Therefore no real transmitted direction exists.

All energy reflects.

The threshold is the critical angle:

$$
\boxed{
\theta_c=
\arcsin
\left(
\frac{\eta_t}{\eta_i}
\right)
}
$$

for:

$$
\eta_i>\eta_t.
$$

For glass-to-air:

$$
\eta_i=1.5,
\qquad
\eta_t=1.
$$

Then:

$$
\theta_c=
\arcsin(2/3)
\approx41.8^\circ.
$$

At greater internal incidence angles:

$$
\boxed{
F=1
}
$$

and refraction disappears.

---

**KairoRayTracer already implements this behavior**

Its Whitted glass path computes an attempted refraction vector.

If the vector is effectively zero/invalid:

```cpp
fresnel = 1.0f;
```

so all energy goes to reflection.

Otherwise it uses Schlick Fresnel to mix:

$$
L=
F L_{\text{reflection}}
+
(1-F)L_{\text{refraction}}.
$$

This is a proper conceptual reflection/transmission split for a simple glass model.

---

**Fresnel and transmission energy**

For an ideal lossless dielectric interface:

$$
\boxed{
F+T=1.
}
$$

Therefore:

$$
T=1-F.
$$

This is why Kairo's Whitted glass uses:

```cpp
reflected * fresnel +
refracted * (1.0f - fresnel)
```

However, a rigorous radiance-domain BTDF contains additional refractive-index factors because radiance is not invariant across refractive interfaces in exactly the same form as in equal-index free-space propagation.

For basic Whitted rendering, the simple mixture is common.

For physically rigorous path tracing, the $\eta^2$ transport factors and chosen radiance/importance transport mode matter.

We will revisit that in the path-tracing passes.

---

**Nested dielectric problem**

A simple renderer often assumes:

```text
outside = air
inside  = object's IOR
```

Then entering uses:

$$
\eta=
\frac1{\eta_m},
$$

and exiting:

$$
\eta=
\eta_m.
$$

That works for one isolated glass object.

But suppose:

```text
air
→ glass
→ water
→ glass
→ air
```

You need to know the current medium.

A robust path tracer tracks a **medium stack** or equivalent interface IOR information.

Kairo's current Whitted glass path handles a simple air/object entering/exiting case, not a general nested-medium system.

That distinction will matter in Phase 21.

---

**Beer–Lambert absorption**

Light traveling through an absorbing medium decays exponentially.

For extinction/absorption coefficient:

$$
\sigma_a
$$

and distance:

$$
d,
$$

transmittance is:

$$
\boxed{
T(d)=e^{-\sigma_a d}.
}
$$

For RGB:

$$
\boxed{
T(d)
=
\left(
e^{-\sigma_r d},
e^{-\sigma_g d},
e^{-\sigma_b d}
\right).
}
$$

Then:

$$
L_{\text{exit}}
=
T(d)
L_{\text{entry}}.
$$

---

**Why absorption is exponential**

Consider differential path length $dx$.

Assume fractional radiance loss is proportional to current radiance:

$$
dL=-\sigma_aL\,dx.
$$

Divide:

$$
\frac{dL}{L}
=
-\sigma_a dx.
$$

Integrate:

$$
\ln L
=
-\sigma_a x+C.
$$

Therefore:

$$
L(x)
=
L_0e^{-\sigma_a x}.
$$

So Beer–Lambert is not an arbitrary fog fade.

It follows from constant proportional attenuation per unit distance.

---

**Thickness creates color**

Suppose:

$$
\sigma_a=
(0.2,1.0,2.0).
$$

At:

$$
d=0.1
$$

transmittance is approximately:

$$
T=
(0.980,\;0.905,\;0.819).
$$

At:

$$
d=2
$$

it becomes:

$$
T=
(0.670,\;0.135,\;0.018).
$$

The material becomes dramatically more reddish as thickness increases because blue/green are absorbed faster.

This is why colored glass should not simply use:

```text
alpha = 0.3
```

independent of thickness.

---

**Thin versus volume again**

For a thin transmission approximation, there may be no meaningful $d$.

Then Beer–Lambert cannot generate thickness-dependent coloration.

For a volumetric solid:

$$
d=
\text{distance between entry and exit interfaces}.
$$

Now absorption emerges naturally.

This is the fundamental semantic distinction between:

$$
\boxed{\text{surface transparency}}
$$

and:

$$
\boxed{\text{participating/absorbing material volume}}.
$$

---

**Emissive materials**

Emission contributes radiance independent of incident lighting:

$$
\boxed{
L_o=
L_e+
L_{\text{reflected}}
+
L_{\text{transmitted}}.
}
$$

For a purely emissive material:

$$
L_o=L_e.
$$

An emissive surface therefore does not need a light shining on it to be visible.

Your Kairo real-time shader does:

```glsl
vec3 emissive =
    emissiveFactor *
    texture(emissiveMap, uv).rgb;
```

then adds it:

```glsl
hdrColor =
    ambient +
    direct +
    emissive;
```

Conceptually that is correct.

---

**Emissive does not automatically mean “lights the scene”**

This distinction is extremely important.

A raster shader can display a glowing emissive billboard by adding:

$$
L_e
$$

to its own output.

But neighboring objects receive no illumination unless the renderer explicitly uses that emitting geometry in its lighting algorithm.

In path tracing, an emissive surface can naturally illuminate other surfaces **if paths sample/reach it appropriately**.

In real-time rasterization, emissive lighting usually needs:

- baked GI,
- probes,
- voxel/radiance caches,
- screen-space techniques,
- ray tracing,
- explicit converted lights.

Therefore:

$$
\boxed{
\text{visibly emissive}
\neq
\text{automatically a scene light}.
}
$$

---

**Current Kairo real-time direct-light equation**

Your current shader effectively evaluates:

$$
L_o
=
\left[
(1-F)(1-m)
\frac{C}{\pi}
+
\frac{DFG}{4N_LN_V}
\right]
L_iN_LV_s
$$

where:

$$
V_s
$$

is shadow visibility.

Its code corresponds to:

```glsl
H =
    normalize(V + L);

F =
    FresnelSchlick(
        max(dot(H, V), 0.0),
        F0);

D =
    DistributionGGX(
        N,
        H,
        roughness);

G =
    GeometrySchlickGGX(
        NdotV,
        roughness)
    *
    GeometrySchlickGGX(
        NdotL,
        roughness);

specular =
    D * G * F /
    max(
        4.0 *
        NdotV *
        NdotL,
        epsilon);

kD =
    (1.0 - F) *
    (1.0 - metallic);

BRDF =
    kD *
    baseColor / PI
    +
    specular;

Lo +=
    BRDF *
    radiance *
    NdotL *
    visibility;
```

That is the complete core of contemporary direct-light metallic-roughness PBR.

---

**Why $H$ matters**

Given $L$ and $V$, which microfacet normal would reflect one into the other?

The law of reflection requires equal incident and outgoing angles around the microfacet normal.

Therefore the facet normal must bisect them.

Hence:

$$
\boxed{
H=
\frac{L+V}{\|L+V\|}.
}
$$

GGX then asks:

$$
D(H)=
\text{how statistically likely is that required facet orientation?}
$$

Fresnel asks:

$$
F(V\cdot H)=
\text{how much does that facet reflect?}
$$

Smith asks:

$$
G(L,V)=
\text{is that facet visible to both light and viewer?}
$$

That is the most intuitive interpretation of the entire Cook–Torrance equation.

---

**One photon-story interpretation**

Imagine incident energy arriving from $L$.

First, only microfacets with normals around $H$ can redirect it toward $V$.

That population is represented by:

$$
D.
$$

Second, each qualifying facet reflects only a Fresnel fraction:

$$
F.
$$

Third, some qualifying facets are hidden from either light or viewer:

$$
G.
$$

Finally, because these facets are described statistically over microscopic area but our BRDF measures outgoing radiance over macroscopic projected area and solid angle, we divide by:

$$
4N_LN_V.
$$

Therefore:

$$
\boxed{
f_s=
\frac{
\text{available correctly oriented facets}
\times
\text{interface reflectance}
\times
\text{visibility}
}{
\text{macro/micro geometric conversion}
}
}
$$

which is far easier to reason about than memorizing $DFG/4NLNV$.

---

**Why Fresnel must also reduce diffuse**

Suppose dielectric Fresnel at some angle is:

$$
F=0.8.
$$

That means 80% of incoming interface energy reflects immediately.

Only roughly:

$$
20\%
$$

can enter the material.

If you simultaneously compute full-strength diffuse:

$$
C/\pi
$$

you effectively allow the same incident energy to be used twice.

Thus:

$$
k_d=1-F
$$

is not cosmetic.

It represents the energy that escaped immediate specular reflection.

For metals we additionally multiply:

$$
1-m
$$

because opaque metallic material has essentially no diffuse body-scattering channel.

---

**A numerical PBR trace with consistent directions**

Take $N=(0,0,1)$ and unit directions $L,V$ with $N\cdot L=0.8$, $N\cdot V=0.9$, and $L\cdot V=0.8818$. These values are geometrically attainable. For example, choose $L=(0.6,0,0.8)$ and choose $V$ with $V_z=0.9$ and azimuth satisfying $0.6V_x+0.72=0.8818$.

The half vector is $H=(L+V)/\|L+V\|$. Its dot products are constrained:

$$
V\cdot H=\sqrt{(1+L\cdot V)/2}=0.97,
\qquad N\cdot H=\frac{0.8+0.9}{2(0.97)}\approx0.876289.
$$

They cannot be chosen independently. The supplied example's value $N\cdot H=0.95$ was incompatible with its other dot products; the following trace corrects that geometry.

For linear base color $C=(0.8,0.2,0.1)$, metallic $m=0$, perceptual roughness $r=0.5$, and dielectric $F_0=0.04$, use $\alpha=r^2=0.25$. Schlick Fresnel gives $F=0.04+0.96(0.03)^5\approx0.0400000233$. With $h=N\cdot H$, GGX gives

$$
D=\frac{0.0625}{\pi[1-0.9375h^2]^2}\approx0.253554.
$$

For this example's separable Schlick geometry approximation, $k=(r+1)^2/8=0.28125$, $G_1(x)=x/[x(1-k)+k]$, and $G=G_1(0.8)G_1(0.9)\approx0.905994$. Therefore

$$
f_s=\frac{DFG}{4(0.8)(0.9)}\approx0.003191,
\qquad f_{d,r}=(1-F)\frac{0.8}{\pi}\approx0.244462.
$$

The red BRDF is approximately $0.247652\,\mathrm{sr}^{-1}$. Multiplying by unit incident radiance and $N\cdot L=0.8$ gives approximately $0.198122$ in the incident radiance units. This illustrates this particular real-time approximation; it does not establish exact reciprocity or conservation for every layered material.

---

**A metal version of the same surface**

Let:

$$
m=1.
$$

Then:

$$
k_d=0.
$$

And:

$$
F_0=C_{\text{base}}
=
(0.8,0.2,0.1).
$$

At near-normal half-vector angle:

$$
F\approx F_0.
$$

So specular becomes strongly red.

The material is now interpreted as a red/copper-like conductor rather than red plastic.

Same baseColor.

Different metallic classification.

Completely different optical meaning.

---

**Roughness does not alter $F_0$**

A subtle but critical distinction:

$$
F_0
$$

comes from material interface optical properties.

Roughness controls microfacet orientation distribution.

Changing roughness from:

$$
0.1\rightarrow0.9
$$

should broaden the specular lobe.

It should not fundamentally convert a 4% dielectric interface into a 40% one.

Some approximations modify effective grazing behavior or compensate for multiple scattering, but the material's basic Fresnel $F_0$ and geometric roughness are separate physical concepts.

---

**Current Kairo environment lighting is not a full PBR IBL solution**

Current real-time Kairo computes reflected direction:

$$
R=\operatorname{reflect}(-V,N)
$$

and picks environment mip based approximately on:

$$
\text{LOD}
=
r\times(\text{mipCount}-1).
$$

That is a useful rough reflection approximation.

However a full split-sum GGX IBL pipeline normally requires:

$$
\text{GGX-prefiltered environment}
$$

plus:

$$
\text{BRDF integration LUT}.
$$

Simply sampling an ordinary mipmapped environment does not exactly integrate:

$$
\int
L_i(\omega_i)
f_s(\omega_i,\omega_o)
(N\cdot\omega_i)
d\omega_i.
$$

That is beyond the exact M118 scope, but it matters when judging the physical accuracy of the present renderer.

---

**A coherent reference implementation**

The following compact GLSL separates semantic parameters from the actual BRDF and makes each physical term visible:

```glsl
const float PI = 3.14159265359;

float GGXDistribution(
    float NdotH,
    float perceptualRoughness)
{
    // Artist-facing roughness is mapped to microfacet alpha.
    float alpha =
        perceptualRoughness *
        perceptualRoughness;

    float alpha2 =
        alpha * alpha;

    float d =
        NdotH * NdotH *
        (alpha2 - 1.0) +
        1.0;

    return
        alpha2 /
        max(
            PI * d * d,
            1e-7);
}

float SchlickGGXGeometry1(
    float NdotX,
    float perceptualRoughness)
{
    // Common direct-light approximation to Smith GGX.
    float r =
        perceptualRoughness +
        1.0;

    float k =
        (r * r) / 8.0;

    return
        NdotX /
        max(
            NdotX * (1.0 - k) + k,
            1e-7);
}

float SmithGeometry(
    float NdotL,
    float NdotV,
    float perceptualRoughness)
{
    return
        SchlickGGXGeometry1(
            NdotL,
            perceptualRoughness)
        *
        SchlickGGXGeometry1(
            NdotV,
            perceptualRoughness);
}

vec3 FresnelSchlick(
    float VdotH,
    vec3 F0)
{
    float x =
        1.0 -
        clamp(VdotH, 0.0, 1.0);

    float x5 =
        x * x * x * x * x;

    return
        F0 +
        (vec3(1.0) - F0) *
        x5;
}

vec3 EvaluateMetallicRoughnessBRDF(
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 baseColor,
    float metallic,
    float roughness)
{
    float NdotL =
        max(dot(N, L), 0.0);

    float NdotV =
        max(dot(N, V), 0.0);

    if (NdotL <= 0.0 ||
        NdotV <= 0.0)
    {
        return vec3(0.0);
    }

    vec3 H =
        normalize(L + V);

    float NdotH =
        max(dot(N, H), 0.0);

    float VdotH =
        max(dot(V, H), 0.0);

    // Dielectric interface ≈ 4% at normal incidence.
    // Metallic workflow replaces this by colored conductor reflectance.
    vec3 F0 =
        mix(
            vec3(0.04),
            baseColor,
            metallic);

    float D =
        GGXDistribution(
            NdotH,
            roughness);

    float G =
        SmithGeometry(
            NdotL,
            NdotV,
            roughness);

    vec3 F =
        FresnelSchlick(
            VdotH,
            F0);

    vec3 specular =
        D * G * F /
        max(
            4.0 *
            NdotL *
            NdotV,
            1e-6);

    // Energy not immediately reflected at the interface can contribute
    // to dielectric body/diffuse scattering.
    vec3 diffuseWeight =
        (vec3(1.0) - F) *
        (1.0 - metallic);

    vec3 diffuse =
        diffuseWeight *
        baseColor /
        PI;

    // This function returns the BRDF itself.
    // The caller still multiplies by incident radiance and NdotL.
    return
        diffuse +
        specular;
}
```

Then one direct-light sample is:

```glsl
vec3 brdf =
    EvaluateMetallicRoughnessBRDF(
        N,
        V,
        L,
        baseColor,
        metallic,
        roughness);

vec3 outgoing =
    brdf *
    incidentRadiance *
    max(dot(N, L), 0.0) *
    visibility;
```

Notice the separation:

$$
\boxed{
\text{BRDF}
}
$$

versus:

$$
\boxed{
L_i(N\cdot L).
}
$$

This separation becomes vital in Monte Carlo rendering.

---

**What Kairo implements today versus what this pass teaches**

| Feature | Current Kairo real-time | Kairo ray tracer | Full theory in this pass |
|---|---:|---:|---:|
| Lambert diffuse | Yes as PBR diffuse | Yes | Yes |
| $1/\pi$ normalization | Yes | Yes | Yes |
| Schlick Fresnel | Yes | Yes | Yes |
| Dielectric $F_0=0.04$ | Yes | Yes/PBR | Yes |
| Explicit dielectric IOR | No in real-time material | Yes | Yes |
| Metallic colored $F_0$ | Yes | Yes | Yes |
| GGX NDF | Yes | Yes | Yes |
| Smith-style masking | Schlick approximation | Schlick approximation | Yes |
| Cook–Torrance | Yes | Yes | Yes |
| Diffuse reduced by $1-F$ | Yes | Not in current direct PBR path | Yes |
| Metallic-roughness | Yes | Yes | Yes |
| Specular-gloss workflow | No | No | Yes |
| Disney/Burley diffuse | No | No | Yes |
| Clearcoat | No | No | Yes |
| Sheen | No | No | Yes |
| Refraction | No real-time | Whitted glass | Yes |
| TIR | No real-time | Yes | Yes |
| Beer–Lambert absorption | No | No | Yes |
| Volumetric transmission | No | Simplified glass interfaces | Yes |
| Emissive | Yes | Yes | Yes |

That table is useful because it separates **mastery** from **current implementation**.

Knowing a clearcoat equation does not mean Kairo currently supports clearcoat.

---

**Failure modes you should now be able to diagnose**

A plastic surface looks metallic even with $m=0$: inspect whether baseColor has accidentally become $F_0$, whether the diffuse lobe was removed, or whether dielectric $F_0$ is too high.

A metal looks chalky: inspect whether diffuse remains nonzero for $m=1$.

A rough material becomes much darker as roughness increases: inspect microfacet multiple-scattering loss, environment prefiltering, and whether roughness is incorrectly reducing total specular amplitude instead of redistributing it.

Highlights explode into NaNs: inspect $N_L$, $N_V$, roughness near zero and the Cook–Torrance denominator.

Every dielectric seems to reflect exactly 4% regardless of actual optical IOR: the renderer is using the simplified metallic-roughness fixed-$F_0$ convention rather than an IOR-controlled dielectric model.

Glass never becomes mirror-like at grazing angles: Fresnel is missing.

Glass never exhibits TIR from inside: the refraction equation or medium transition is wrong.

Thick colored glass looks identical to thin glass: Beer–Lambert/path-length absorption is missing.

A red metal has white highlights: conductor $F_0$ has incorrectly been treated as neutral dielectric reflection.

A matte dielectric returns more energy than it receives: inspect whether full Lambert diffuse was added alongside full Fresnel specular without the $(1-F)$ split.

---

**The deepest conceptual connection**

The entire BRDF can now be reconstructed as a physical narrative.

Incident radiance arrives:

$$
L_i.
$$

Projected surface geometry contributes:

$$
N\cdot L.
$$

At the interface, Fresnel decides how much reflects:

$$
F.
$$

For rough specular reflection, only microfacets aligned with:

$$
H
$$

can reflect into the camera.

Their statistical abundance is:

$$
D(H).
$$

Some are hidden from light/view:

$$
G.
$$

Conversion from microscopic mirror area to macroscopic directional reflectance contributes:

$$
4N_LN_V.
$$

Therefore:

$$
f_s=
\frac{DFG}{4N_LN_V}.
$$

The energy not immediately reflected by a dielectric is approximately:

$$
1-F.
$$

That can participate in body scattering:

$$
f_d=
(1-F)(1-m)\frac{C}{\pi}.
$$

Then:

$$
\boxed{
L_o
=
\left[
(1-F)(1-m)
\frac{C}{\pi}
+
\frac{DFG}
{4N_LN_V}
\right]
L_iN_L
}
$$

for a single unobstructed direct-light sample.

That one equation is the center of Pass 4.

---

**Transmission extends the same energy story**

At a dielectric interface:

$$
F
$$

goes into reflection.

Approximately:

$$
1-F
$$

crosses the boundary.

Snell determines its new direction:

$$
\eta_i\sin\theta_i
=
\eta_t\sin\theta_t.
$$

If no transmitted solution exists:

$$
F=1
$$

through total internal reflection.

If the transmitted path travels inside a material for distance $d$, then:

$$
L\rightarrow
L
e^{-\sigma_a d}.
$$

Thus reflection, refraction and absorption are not separate visual tricks.

They are one energy-transport story:

$$
\boxed{
\text{interface split}
\rightarrow
\text{direction change}
\rightarrow
\text{medium attenuation}.
}
$$

---

**Research-quality validation strategy**

For PBR research, screenshot comparison is too weak.

A controlled validation suite should render canonical spheres or planes while varying exactly one parameter.

One sequence holds:

$$
m=0
$$

and sweeps:

$$
r=
0.05,\,
0.1,\,
0.2,\,
0.4,\,
0.8,\,
1.0.
$$

Another fixes roughness and compares:

$$
m=0
\quad\text{versus}\quad
m=1.
$$

Another sweeps camera angle toward grazing incidence to verify:

$$
F\rightarrow1.
$$

A dielectric suite changes:

$$
\eta=
1.1,\,
1.33,\,
1.5,\,
2.0
$$

and checks measured/analytic $F_0$.

Transmission tests should vary geometric thickness and verify:

$$
\ln T
$$

is linear with distance:

$$
\ln T=-\sigma_a d.
$$

That gives falsifiable tests rather than “this sphere looks realistic.”

---


| Concept | Equation / invariant |
|---|---|
| BRDF | $f_r=dL_o/dE_i$ |
| BRDF units | $\mathrm{sr}^{-1}$ |
| Rendering equation reflection | $L_o=\int f_rL_i(N\cdot\omega_i)d\omega_i$ |
| Reciprocity | $f_r(i,o)=f_r(o,i)$ |
| Conservation | $\int f_r(N\cdot o)d\omega_o\le1$ |
| Lambert | $f_d=\rho/\pi$ |
| Mirror reflection | $R=I-2(I\cdot N)N$ |
| Half vector | $H=(L+V)/\|L+V\|$ |
| Dielectric $F_0$ | $((\eta_i-\eta_t)/(\eta_i+\eta_t))^2$ |
| Schlick | $F=F_0+(1-F_0)(1-V\cdot H)^5$ |
| Common glass | $\eta\approx1.5\Rightarrow F_0\approx0.04$ |
| Conductor IOR | $\tilde{\eta}=\eta+i\kappa$ |
| GGX alpha | $\alpha=r^2$ |
| GGX NDF | $D=\alpha^2/[\pi((N_H)^2(\alpha^2-1)+1)^2]$ |
| Smith | $G\approx G_1(L)G_1(V)$ |
| Cook–Torrance | $f_s=DFG/(4N_LN_V)$ |
| Metallic $F_0$ | $F_0=\text{mix}(0.04,C,m)$ |
| Diffuse weight | $k_d=(1-F)(1-m)$ |
| PBR direct light | $L_o=(f_d+f_s)L_iN_L$ |
| Burley diffuse | Lambert-like $C/\pi$ with grazing/roughness correction |
| Clearcoat | secondary dielectric specular layer |
| Sheen | grazing/fibre-oriented secondary lobe |
| Snell | $\eta_i\sin\theta_i=\eta_t\sin\theta_t$ |
| Refraction vector | $T=\eta I+(\eta c_i-c_t)N$ |
| TIR | $1-\eta^2(1-c_i^2)<0$ |
| Critical angle | $\theta_c=\arcsin(\eta_t/\eta_i)$ |
| Beer–Lambert | $T=e^{-\sigma_a d}$ |
| Emission | $L_o=L_e+\text{scattered radiance}$ |

The curriculum explicitly treats this M117/M118 block as the foundation for physically interpretable materials and image-fidelity work, which is exactly why these terms need to be derivable rather than remembered as shader snippets.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/04 - Sampling, Filtering and Reconstruction|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/06 - From Material Data to GPU Bindings|Next chapter]]
