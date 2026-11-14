# 09 — Point Lights, MIS and Environment Lighting

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/08 - Sampling Directions and Light Transport|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation|Next chapter]]

Point lights are discrete directional events while environment illumination occupies a continuous distribution. Multiple importance sampling connects competing estimators while keeping each light’s probability model explicit.

**By the end:** Compare point-light and environment estimators and explain MIS weighting.

**Recall question:** What makes a point light a delta distribution, and how does MIS use competing PDFs?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 19361–22205; source-span SHA256 16b6d00e3ebc5af580eefb93c84fdf8ec1efee1705d4fc8c8338215452502337. -->

<!-- CHAPTER-SOURCE-BEGIN -->
**Point-light sampling**

A point light has zero area.

From point $x$, there is exactly one direction:

$$
\omega_L
=
\frac{
P_L-x
}{
\|P_L-x\|
}.
$$

This is a delta-direction light sampling event.

There is no finite ordinary solid-angle PDF like:

$$
1/(4\pi)
$$

for sampling the point light itself.

It is a discrete/delta event.

Its contribution for radiant intensity $I$ is approximately:

$$
L_i
\propto
\frac{
I
}{
r^2
}.
$$

Then:

$$
L_d
=
f_r
L_i
\cos\theta
V.
$$

Kairo's `PointLight::Intensity` is explicitly documented in candela, so inverse-square attenuation is appropriate for its basic photometric interpretation.

---

**Delta lights and MIS**

A delta point light cannot be hit by randomly selecting an arbitrary continuous BSDF direction with nonzero probability.

Probability of selecting exactly one direction from a continuum is zero.

Therefore ordinary finite-PDF MIS comparison with a continuous BSDF sampler must treat delta distributions specially.

This is why rendering code usually labels sampling events:

$$
\text{delta}
$$

versus:

$$
\text{non-delta}.
$$

You should never attempt to “evaluate the point-light PDF as a huge number.”

It belongs to a different measure/distribution class.

---

**Environment lighting**

An environment map defines radiance for every direction:

$$
L_{\text{env}}(\omega).
$$

There is no finite light position.

A ray that escapes geometry returns:

$$
\boxed{
L_i=
L_{\text{env}}(\omega).
}
$$

Current Kairo does exactly this in `Scene::SampleEnvironment()`.

If environment texture exists, direction is converted into equirectangular coordinates:

$$
u=
\frac12+
\frac{
\operatorname{atan2}(z,x)
}{
2\pi
},
$$

$$
v=
\frac12-
\frac{
\arcsin(y)
}{
\pi
}.
$$

That is equivalent to latitude-longitude mapping.

---

**Uniform environment sampling**

One straightforward NEE strategy samples a uniform sphere direction:

$$
p_\omega
=
\frac1{4\pi}.
$$

Then test whether the direction is above the shading normal:

$$
n\cdot\omega>0.
$$

Evaluate:

$$
\frac{
L_{\text{env}}(\omega)
f_r
(n\cdot\omega)
}{
1/(4\pi)
}.
$$

This is unbiased.

But HDR environment maps often contain tiny, extremely bright features such as the sun.

Uniform sampling wastes almost all samples in dark sky regions.

---

**Uniform UV sampling is not uniform sphere sampling**

An equirectangular map uses:

$$
u=\frac{\phi}{2\pi},
\qquad
v=\frac{\theta}{\pi}.
$$

A texel spans:

$$
d\phi\,d\theta.
$$

Its spherical solid angle is:

$$
\boxed{
d\omega
=
\sin\theta
\,d\theta\,d\phi.
}
$$

Therefore texels near the equator represent much larger solid angle than texels near the poles.

If you uniformly choose map texels, you do **not** uniformly sample directions.

This is a classic environment-map bug.

---

**PDF of uniform UV sampling**

Since:

$$
d\phi=2\pi\,du,
$$

$$
d\theta=\pi\,dv,
$$

we get:

$$
d\omega
=
2\pi^2
\sin\theta
\,du\,dv.
$$

If:

$$
p_{uv}=1
$$

on the unit square, then:

$$
p_\omega
=
p_{uv}
\left|
\frac{
du\,dv
}{
d\omega
}
\right|.
$$

Thus:

$$
\boxed{
p_\omega
=
\frac{
1
}{
2\pi^2\sin\theta
}.
}
$$

The apparent singularity near poles reflects how many UV samples get packed into tiny solid angle there.

This is why one must always know the PDF's measure.

---

**Environment importance sampling**

A better environment sampler chooses directions roughly proportional to radiance contribution.

For a latitude-longitude texel at row corresponding to $\theta$, use weight:

$$
\boxed{
w_{ij}
=
Y_{ij}
\sin\theta_j
}
$$

where:

$$
Y_{ij}
$$

is luminance.

Why multiply by:

$$
\sin\theta?
$$

Because each texel's solid angle is proportional to it.

Sampling only by pixel luminance would over-represent polar texels relative to their actual angular area.

---

**Environment luminance**

A simple linear-RGB luminance approximation may be:

$$
Y
=
0.2126R+
0.7152G+
0.0722B.
$$

The exact coefficients depend on RGB primaries/working space.

For importance sampling, what matters most is a scalar proportional to expected energy.

Then:

$$
w_{ij}
=
Y_{ij}\sin\theta_j.
$$

Normalize all weights:

$$
P_{ij}
=
\frac{
w_{ij}
}{
\sum_{kl}w_{kl}
}.
$$

Now texels containing bright sun pixels receive much higher probability.

---

**Cumulative distribution function sampling**

For a 1D discrete distribution:

$$
p_0,p_1,\dots,p_{n-1},
$$

build cumulative:

$$
C_0=p_0,
$$

$$
C_1=p_0+p_1,
$$

$$
\dots
$$

$$
C_{n-1}=1.
$$

Draw:

$$
\xi\in[0,1).
$$

Find smallest index $i$ where:

$$
C_i>\xi.
$$

This can be located with binary search:

$$
O(\log n).
$$

For a 2D environment map, typically build:

$$
\boxed{
\text{marginal CDF over rows}
}
$$

plus:

$$
\boxed{
\text{conditional CDF over columns within each row}.
}
$$

---

**2D environment CDF construction**

For each row $j$, compute:

$$
w_{ij}
=
Y_{ij}\sin\theta_j.
$$

Row sum:

$$
W_j=
\sum_iw_{ij}.
$$

Build conditional distribution:

$$
P(i|j)
=
\frac{w_{ij}}{W_j}.
$$

Then marginal row distribution:

$$
P(j)
=
\frac{W_j}{\sum_kW_k}.
$$

Sampling:

$$
j\sim P(j),
$$

then:

$$
i\sim P(i|j).
$$

Joint probability:

$$
\boxed{
P(i,j)=P(j)P(i|j).
}
$$

Then jitter within the chosen texel and convert the discrete texel probability to a continuous directional PDF.

---

**Why texel probability is not yet a solid-angle PDF**

Suppose selected texel probability is:

$$
P_{ij}.
$$

If we sample uniformly inside that texel, UV density is:

$$
p_{uv}
=
P_{ij}
WH
$$

because texel UV area is:

$$
\frac1{WH}.
$$

Then:

$$
p_\omega
=
\frac{
p_{uv}
}{
2\pi^2\sin\theta
}.
$$

Therefore:

$$
\boxed{
p_\omega
=
\frac{
P_{ij}WH
}{
2\pi^2\sin\theta
}.
}
$$

The $\sin\theta$ in the importance weights and the conversion are not redundant; one handles texel solid angle during discrete selection, the other converts from UV density into directional density.

---

**Alias-table sampling**

CDF lookup costs:

$$
O(\log N)
$$

with binary search.

Alias method preprocesses a discrete distribution into two arrays, conceptually:

$$
\text{probability}[i],
$$

$$
\text{alias}[i].
$$

Sampling then requires:

1. choose one column uniformly;
2. draw one random threshold;
3. choose either column or alias.

Runtime:

$$
\boxed{O(1).}
$$

Preprocessing is:

$$
O(N).
$$

This is valuable for:

- thousands of lights,
- large environment distributions,
- repeated sampling over many rays.

---

**Alias-table intuition**

Imagine every bin should contain exactly average probability mass:

$$
1/N.
$$

Some original outcomes are underfull.

Some are overfull.

During preprocessing, use excess probability from overfull bins to top up underfull bins.

Each final column stores:

$$
\text{primary outcome}
$$

plus possibly:

$$
\text{alias outcome}.
$$

At sampling time, one uniform column and one coin flip reproduce the original nonuniform distribution.

The method trades preprocessing and memory for constant-time sampling.

---

**Current Kairo environment status**

Current Kairo:

$$
\text{can sample an environment when a ray misses}.
$$

It supports:

$$
\text{constant environment color}
$$

or:

$$
\text{lat-long texture}.
$$

What it does **not yet implement** is environment next-event estimation.

There is currently no:

$$
\text{uniform directional environment sampler},
$$

$$
\text{luminance}\times\sin\theta\text{ importance distribution},
$$

$$
\text{CDF},
$$

or:

$$
\text{alias table}
$$

in `Environment.cppm`.

Therefore bright HDR environment illumination can only be discovered by ordinary bounce directions.

That is a major variance opportunity.

---

**Multiple samples per pixel**

A pixel estimate after $N$ samples is:

$$
\boxed{
\bar X_N
=
\frac1N
\sum_{i=1}^NX_i.
}
$$

Each $X_i$ may include randomness from:

- subpixel location,
- BSDF direction,
- light sample,
- Russian roulette,
- environment sample,
- lens sample in future,
- time sample in future.

More samples reduce estimator variance:

$$
\operatorname{Var}(\bar X_N)
=
\frac{\sigma^2}{N}
$$

for independent samples.

Standard deviation:

$$
\boxed{
\sigma_{\bar X}
=
\frac{\sigma}{\sqrt N}.
}
$$

---

**Current Kairo pixel sampling**

Kairo's renderer uses `StratifiedPixelSample()`.

For:

$$
N>1,
$$

it computes grid side:

$$
s=
\left\lceil
\sqrt N
\right\rceil.
$$

Sample index determines stratum:

$$
s_x=i\bmod s,
$$

$$
s_y=\lfloor i/s\rfloor.
$$

Then adds deterministic hashed jitter inside each stratum.

This is better distributed than purely independent random points.

It also remains deterministic from:

$$
(x,y,\text{sample},\text{seed}).
$$

That is excellent for testing.

---

**Subtle stratification issue for non-square sample counts**

Suppose:

$$
N=5.
$$

Then:

$$
s=\lceil\sqrt5\rceil=3.
$$

Grid has:

$$
9
$$

possible cells.

But only first 5 are populated.

Taking the first five cells gives zero probability to four cells. For a generic pixel integrand, equal averaging then estimates the mean over the selected region rather than the full pixel. This can introduce bias, rather than only a less symmetric noise pattern.

So current sampling is a practical compact stratification method, not a mathematically perfect $N$-sample stratification for every arbitrary $N$.

Common production choices use square sample counts, Latin-hypercube methods, Sobol, PMJ, blue noise or other low-discrepancy sequences.

Those come later in the curriculum.

---

**Progressive accumulation**

Instead of rendering:

$$
1024
$$

samples in one blocking call, render:

$$
1
$$

sample per pixel repeatedly.

After pass 1:

$$
\bar X_1=X_1.
$$

After pass 2:

$$
\bar X_2=\frac{X_1+X_2}{2}.
$$

After $n$:

$$
\boxed{
\bar X_n
=
\frac1n
\sum_{k=1}^nX_k.
}
$$

This enables:

- progressively improving preview,
- stopping whenever quality is sufficient,
- adaptive sampling,
- interactive renderer feedback.

---

**Online mean update**

You do not need to store all samples.

Given previous mean:

$$
\mu_{n-1},
$$

new sample:

$$
x_n,
$$

update:

$$
\boxed{
\mu_n
=
\mu_{n-1}
+
\frac{
x_n-\mu_{n-1}
}{
n
}.
}
$$

Derivation:

$$
\mu_n
=
\frac{
(n-1)\mu_{n-1}+x_n
}{
n
}
$$

$$
=
\mu_{n-1}
+
\frac{x_n-\mu_{n-1}}{n}.
$$

This is numerically and operationally useful for progressive rendering.

---

**Current Kairo progressive renderer**

Kairo's `RenderProgressive(scene, passes)` currently runs one-sample-per-pixel render passes with incremented seeds.

It sums each completed pass image:

$$
S=
\sum_{k=1}^{P}I_k
$$

and finally divides by:

$$
P.
$$

This is mathematically equivalent to the arithmetic mean.

The limitation is architectural: it only computes the final average after the requested pass batch and does not maintain per-pixel online sample count/variance as first-class state.

The comments themselves describe future inspectable intermediate images.

---

**Per-pixel variance**

A pixel with samples:

$$
x_1,\dots,x_n
$$

has sample variance:

$$
\boxed{
s^2
=
\frac{
1
}{
n-1
}
\sum_i
(x_i-\bar x)^2.
}
$$

For RGB we can track variance separately per channel, or often track luminance variance to drive adaptive sampling.

High variance means:

$$
\text{pixel estimate remains statistically unstable}.
$$

Low variance means:

$$
\text{additional samples are producing similar results}.
$$

---

**Why naive variance computation can be numerically poor**

Computing:

$$
E[x^2]-E[x]^2
$$

can suffer catastrophic cancellation when values are large but variance is small.

Welford's online algorithm is robust.

Maintain:

$$
n,\mu,M_2.
$$

For new sample $x$:

$$
n\leftarrow n+1,
$$

$$
\delta=x-\mu,
$$

$$
\mu\leftarrow\mu+\delta/n,
$$

$$
\delta_2=x-\mu,
$$

$$
M_2\leftarrow M_2+\delta\delta_2.
$$

Then:

$$
\boxed{
s^2=
\frac{M_2}{n-1}.
}
$$

---

**A useful film accumulator**

```cpp
struct PixelStatistics
{
    uint64_t count = 0;

    Color mean = Color::Black();
    Color m2   = Color::Black();

    void AddSample(const Color& x)
    {
        ++count;

        const Color delta =
            x - mean;

        mean +=
            delta /
            static_cast<float>(count);

        const Color delta2 =
            x - mean;

        m2 +=
            delta * delta2;
    }

    Color Variance() const
    {
        if (count < 2)
            return Color::Black();

        return
            m2 /
            static_cast<float>(count - 1);
    }

    Color StandardError() const
    {
        if (count < 2)
            return Color::Black();

        return Sqrt(
            Variance() /
            static_cast<float>(count));
    }
};
```

This would be a natural evolution of Kairo's current `Film`, which presently stores only the final `Color3f` per pixel.

---

**Variance enables adaptive sampling**

Suppose pixel A has estimated standard error:

$$
0.001.
$$

Pixel B:

$$
0.25.
$$

Spending another 1000 samples equally on both is wasteful.

Adaptive sampling allocates more work to high-variance pixels.

A termination criterion might compare:

$$
\frac{
\text{standard error}
}{
|\text{mean}|+\epsilon
}
$$

against a threshold.

This is more advanced than current M124 requirements, but it follows directly from per-pixel variance.

---

**Noise versus bias**

An estimator can have:

$$
\text{variance}
$$

and:

$$
\text{bias}.
$$

Bias:

$$
\boxed{
\operatorname{Bias}
=
E[\hat I]-I.
}
$$

Variance:

$$
\boxed{
E[
(\hat I-E[\hat I])^2
].
}
$$

Unbiased noisy path tracer:

$$
E[\hat I]=I
$$

but finite images fluctuate.

A biased renderer may look smooth but converge to the wrong answer.

---

**Mean-squared error decomposition**

A useful identity is:

$$
\boxed{
MSE
=
\operatorname{Var}
+
\operatorname{Bias}^2.
}
$$

This explains why small controlled bias may sometimes be acceptable in production.

A slightly biased estimator with dramatically lower variance may produce lower visible error at finite rendering time.

But that must be a conscious engineering decision.

---

**Sources of bias**

A hard maximum bounce depth introduces bias.

Contribution clamping introduces bias.

An incorrect MIS scheme introduces bias.

Denoising can introduce reconstruction bias.

Approximate BRDFs are model bias relative to real physics.

Finite precision can introduce numerical errors.

Russian roulette, however, **does not inherently introduce bias** when survival compensation is correct.

That distinction matters.

---

**Fireflies**

Fireflies are isolated extremely bright samples.

They usually arise when:

$$
\boxed{
\frac{f(x)}{p(x)}
}
$$

becomes very large for a rare event.

Suppose:

$$
f(x)=1000
$$

but:

$$
p(x)=10^{-6}.
$$

Then weight:

$$
10^9.
$$

Even if event happens once in millions of samples, one occurrence can dominate a pixel average.

---

**Common firefly sources**

A glossy BRDF randomly landing on a tiny intense light is a classic case.

A bright HDR sun pixel sampled with a nearly uniform environment PDF is another.

Near-specular caustic paths can create extremely concentrated transport.

Shading-normal mismatches can cause artificial BRDF amplification.

Near-zero PDFs, malformed roughness, invalid normals or bad Jacobians can create numerical fireflies rather than legitimate statistical ones.

The correct first response is:

$$
\boxed{
\text{find why }f/p\text{ is huge}.
}
$$

Not immediately clamp the pixel.

---

**Clamping**

One firefly suppression technique:

$$
C'=
\min(C,C_{\max})
$$

componentwise or by luminance.

This reduces variance.

But:

$$
E[C']\neq E[C]
$$

whenever contributions above threshold genuinely occur.

Therefore clamping is biased.

The image usually becomes too dark in transport regions depending on rare high-energy paths.

---

**Why clamping can still be useful**

In interactive rendering or game-oriented offline preview, a tiny amount of bias may be preferable to obvious fireflies.

A better hierarchy of fixes is conceptually:

First fix incorrect PDFs/Jacobians/BRDFs.

Then improve importance sampling.

Then use NEE and MIS.

Then improve environment/light distributions.

Then, only if necessary, use carefully controlled clamping.

Clamping should be treated as a variance-management compromise, not as the mathematical solution.

---

**Path-space intuition**

Instead of thinking one bounce at a time, think of a whole path:

$$
\boxed{
\bar x
=
(x_0,x_1,\dots,x_k).
}
$$

Where:

$$
x_0=\text{camera},
$$

$$
x_k=\text{emitter/environment}.
$$

A path contribution roughly contains products of:

$$
\text{emission},
$$

$$
\text{BRDF/BSDF values},
$$

$$
\text{geometry terms},
$$

$$
\text{visibility},
$$

divided by path-sampling density.

Thus rendering can be viewed as integration over a gigantic **path space** rather than merely directions at one surface.

This is the conceptual foundation for:

- bidirectional path tracing,
- vertex connection and merging,
- Metropolis light transport.

We are not implementing those now, but the mental model starts here.

---

**Path notation and event classes**

A common shorthand uses:

$$
E=\text{emitter},
$$

$$
D=\text{diffuse},
$$

$$
G=\text{glossy},
$$

$$
S=\text{specular/delta},
$$

$$
C=\text{camera}.
$$

A simple direct diffuse path:

$$
CDE.
$$

A mirror reflection:

$$
CSE.
$$

Diffuse interreflection:

$$
CDDE.
$$

Caustic-like path:

$$
CDSSE
$$

depending on direction convention/path ordering.

Path-space classification helps explain why some algorithms handle certain transport well and others poorly.

---

**Specular delta distributions**

A perfect mirror reflects into exactly one outgoing direction.

Its BRDF cannot be represented as an ordinary smooth finite function.

It contains a Dirac delta distribution.

Informally:

$$
f_s
\propto
\delta(
\omega_i-\omega_r
).
$$

Why does this matter?

Because probability of selecting exactly the mirror direction using a continuous hemisphere sampler is zero.

Therefore perfect-specular events require specialized deterministic/delta sampling.

---

**Delta event versus very glossy event**

Rough GGX with:

$$
r=0.01
$$

is still technically a finite-width continuous distribution.

Perfect mirror:

$$
r=0
$$

in ideal mathematical form becomes a delta.

These behave differently in MIS.

For a delta event, there is usually only one valid transport direction.

The sampled BSDF event is treated with discrete probability rather than ordinary finite solid-angle density in the same way.

You must not evaluate the perfect mirror using a naïve:

$$
p_\omega=\text{finite number}
$$

as though it were diffuse.

---

**Diffuse event**

A diffuse event scatters over an entire hemisphere.

For Lambert:

$$
f=\rho/\pi.
$$

A continuum of directions has nonzero contribution.

Cosine-weighted sampling works well.

This is a **non-delta continuous event**.

---

**Glossy event**

A glossy BRDF such as GGX rough reflection also has continuous support over a range of directions, but concentrated around specular configurations.

It is non-delta as long as roughness is nonzero.

A GGX VNDF sampler is appropriate.

Thus:

$$
\text{diffuse}
$$

and:

$$
\text{glossy}
$$

are both continuous, but their ideal PDFs differ drastically.

---

**Why event classification matters for NEE**

At a diffuse surface:

$$
\text{sample lights explicitly}.
$$

At a rough glossy surface:

$$
\text{sample lights + BSDF with MIS}.
$$

At a perfect mirror surface:

there is usually no need to perform ordinary direct-light NEE toward arbitrary lights because only one outgoing direction is supported by the BSDF delta.

This is why path tracers often carry flags such as:

```cpp
BSDF_Diffuse
BSDF_Glossy
BSDF_Specular
BSDF_Reflection
BSDF_Transmission
```

They control both sampling and MIS logic.

---

**Glass as probabilistic event selection**

At an ideal dielectric interface, Fresnel gives reflection probability:

$$
F.
$$

Transmission energy fraction:

$$
1-F.
$$

Whitted tracing spawned both reflection and refraction rays.

Path tracing can instead select **one** branch.

Draw:

$$
\xi\sim U[0,1).
$$

If:

$$
\xi<F,
$$

choose reflection.

Otherwise choose transmission.

This avoids exponential ray branching.

---

**Why choosing probability equal to Fresnel is elegant**

Suppose reflected contribution factor contains:

$$
F.
$$

Sampling reflection with probability:

$$
p_r=F.
$$

Monte Carlo weight includes:

$$
\frac{F}{p_r}
=
\frac{F}{F}
=
1.
$$

Similarly transmission:

$$
p_t=1-F,
$$

and in the simplest lossless conceptual case:

$$
\frac{1-F}{p_t}=1.
$$

So Fresnel factor cancels selection probability.

This dramatically stabilizes ideal dielectric event sampling.

---

**Glass pseudocode**

```cpp
BSDFSample SampleIdealGlass(
    Vec3 wo,
    Vec3 n,
    float etaI,
    float etaT,
    RNG& rng)
{
    const float cosTheta =
        std::clamp(
            Dot(wo, n),
            0.0f,
            1.0f);

    const float F =
        FresnelDielectric(
            cosTheta,
            etaI,
            etaT);

    const float eta =
        etaI / etaT;

    const auto wt =
        Refract(
            -wo,
            n,
            eta);

    // Total internal reflection.
    if (!wt)
    {
        return
        {
            .Wi = Reflect(-wo, n),
            .Probability = 1.0f,
            .IsDelta = true,
            .IsTransmission = false
        };
    }

    if (Random01(rng) < F)
    {
        return
        {
            .Wi = Reflect(-wo, n),
            .Probability = F,
            .IsDelta = true,
            .IsTransmission = false
        };
    }

    return
    {
        .Wi = *wt,
        .Probability = 1.0f - F,
        .IsDelta = true,
        .IsTransmission = true
    };
}
```

The full radiance-domain transmission throughput requires careful refractive-index factors.

That belongs to the next pass together with nested media and explicit IOR tracking.

Do not simplify full dielectric transport to “just $1-F$” once we derive the BTDF rigorously.

---

**Total internal reflection in event selection**

If refraction has no real solution:

$$
\sin^2\theta_t>1,
$$

then:

$$
F=1.
$$

Reflection probability becomes:

$$
p_r=1.
$$

Transmission probability:

$$
p_t=0.
$$

So event selection naturally becomes deterministic mirror reflection.

No special arbitrary fallback is needed beyond recognizing TIR.

---

**Current Kairo Path mode versus theory**

This is the most important repository audit in this pass.

Current `TracePath()` only has a special material branch for:

```cpp
MaterialType::Emissive
```

There is no branch for:

```cpp
Mirror
Glass
PBR
```

inside the current path integrator.

After direct lighting, every non-emissive surface receives:

```cpp
CosineHemisphereDirection(hit.Normal)
```

and the next bounce is weighted approximately by:

```cpp
albedo / survivalProbability
```

Therefore current Kairo **Path mode effectively treats every non-emissive surface as diffuse Lambertian for indirect transport**, even when the material record says Mirror, Glass or PBR.

The Whitted integrator handles mirror/glass properly.

The direct PBR integrator handles GGX direct illumination.

But the path integrator has not yet unified those into a general BSDF transport system.

That is the largest architectural gap exposed by M123/M124.

---

**What a general Kairo BSDF layer should look like**

Instead of the path integrator knowing material types directly, introduce an abstraction conceptually like:

```cpp
struct BSDFSample
{
    Vec3 Wi;

    Color F;

    float Pdf;

    bool IsDelta;
    bool IsReflection;
    bool IsTransmission;
};

Color EvaluateBSDF(
    const Material& material,
    const SurfaceHit& hit,
    Vec3 wo,
    Vec3 wi);

float PdfBSDF(
    const Material& material,
    const SurfaceHit& hit,
    Vec3 wo,
    Vec3 wi);

BSDFSample SampleBSDF(
    const Material& material,
    const SurfaceHit& hit,
    Vec3 wo,
    RNG& rng);
```

Then the integrator operates generically:

```cpp
sample = SampleBSDF(...);

beta *=
    sample.F *
    abs(dot(N, sample.Wi)) /
    sample.Pdf;
```

Mirror returns delta reflection.

Glass returns Fresnel-selected delta reflection/transmission.

Lambert returns cosine-sampled diffuse.

PBR returns GGX/VNDF reflection plus diffuse mixture.

That architecture is where the renderer should move.

---

**Sampling a mixture BSDF**

Suppose metallic-roughness dielectric material contains:

$$
f=
f_d+f_s.
$$

One simple sampling strategy chooses diffuse with probability:

$$
p_d
$$

and specular with:

$$
p_s=1-p_d.
$$

If diffuse selected:

$$
\omega_i\sim p_d^{\text{dir}}.
$$

If specular:

$$
\omega_i\sim p_s^{\text{dir}}.
$$

The complete mixture PDF for an obtained direction is:

$$
\boxed{
p(\omega)
=
p_d\,p_d^{\text{dir}}(\omega)
+
p_s\,p_s^{\text{dir}}(\omega).
}
$$

You must evaluate the **full BSDF**:

$$
f_d+f_s
$$

and divide by the **mixture PDF**, not merely the PDF of the branch you happened to choose if both lobes have support there.

This is a common implementation error.

---

**Mixture probability should approximate lobe energy**

Choosing:

$$
p_d=0.5,
\quad
p_s=0.5
$$

is simple.

But if a metal has:

$$
f_d\approx0,
$$

half the samples are wasted.

Better choose lobe probabilities based on expected diffuse/specular energy.

For metallic:

$$
p_s\rightarrow1.
$$

For diffuse dielectric:

$$
p_d
$$

can be larger while still preserving nonzero specular selection.

This is another form of importance sampling.

---

**Full next-event estimation architecture**

At each non-delta path vertex $x$:

Choose a light index:

$$
j\sim P_{\text{light}}(j).
$$

Sample that light:

$$
s_L.
$$

Its complete light PDF is:

$$
\boxed{
p_L
=
P_{\text{light}}(j)
p(\omega|j).
}
$$

Evaluate:

$$
f_r,
\quad
L_i,
\quad
\cos\theta,
\quad
V.
$$

Evaluate BSDF PDF for the same direction:

$$
p_B.
$$

Apply:

$$
w_L=
\frac{p_L^2}{p_L^2+p_B^2}.
$$

Then add:

$$
\boxed{
L
\mathrel{+}=
\beta
w_L
\frac{
f_rL_i\cos\theta V
}{
p_L
}.
}
$$

That is the light-sampling side of MIS NEE.

---

**Matching BSDF-emitter MIS**

When the BSDF-sampled path later reaches a light/environment, evaluate the light technique's PDF for that same direction.

Suppose previous BSDF PDF was:

$$
p_B.
$$

Equivalent light PDF:

$$
p_L.
$$

Then:

$$
w_B
=
\frac{
p_B^2
}{
p_B^2+p_L^2
}.
$$

Emission contribution:

$$
\boxed{
L
\mathrel{+}=
\beta
w_B
L_e.
}
$$

If previous event was delta:

$$
w_B=1
$$

because ordinary light sampling could not generate that delta path in the same way.

This is the symmetry missing from many half-implemented MIS renderers.

---

**Environment NEE with MIS**

At a diffuse/glossy surface, sample environment direction:

$$
\omega_L
\sim
p_{\text{env}}.
$$

Trace shadow ray to infinity.

If unoccluded:

$$
L_i=
L_{\text{env}}(\omega_L).
$$

Evaluate:

$$
p_B=
\operatorname{PdfBSDF}(\omega_L).
$$

Then:

$$
w_L
=
\frac{
p_{\text{env}}^2
}{
p_{\text{env}}^2+p_B^2
}.
$$

Contribution:

$$
\boxed{
L_d
=
\beta
w_L
\frac{
L_{\text{env}}
f_r
\cos\theta
}{
p_{\text{env}}
}.
}
$$

If the subsequent BSDF ray misses the scene and sees the environment, perform the symmetric BSDF-side weighting.

That is how HDR skies can become both efficient and unbiased.

---

**Per-pixel firefly diagnostics**

A mature renderer should not just show a final noisy image.

Track at least conceptually:

$$
\max |L_{\text{sample}}|,
$$

$$
\max |\beta|,
$$

$$
\min p_{\text{BSDF}},
$$

$$
\min p_{\text{light}},
$$

$$
\text{bounce where max occurred},
$$

$$
\text{event class}.
$$

If one pixel has:

$$
10^6
$$

sample radiance while neighbors are around:

$$
2,
$$

you want to know whether it was caused by:

```text
GGX sample at bounce 4,
environment sun,
pdf = 1.2e-8
```

rather than merely seeing a white dot.

Path debugging needs transport metadata.

---

**Progressive convergence experiment**

For one fixed scene, render:

$$
1,\ 4,\ 16,\ 64,\ 256,\ 1024
$$

samples per pixel.

Use a very high-sample reference:

$$
R.
$$

Compute RMSE:

$$
RMSE_N
=
\sqrt{
\frac1M
\sum_p
\|I_N(p)-R(p)\|^2
}.
$$

For independent Monte Carlo, error should roughly trend:

$$
\boxed{
RMSE\propto N^{-1/2}
}
$$

until other errors dominate.

If error plateaus at a nonzero level, suspect **bias**, not ordinary Monte Carlo noise.

That is an extremely important debugging distinction.

---

**Noise pattern can diagnose sampling**

Uniform random sampling creates white-noise-like high-frequency variation.

Stratification suppresses low-frequency clustering in finite sample sets.

Low-discrepancy sequences create more even coverage.

Blue-noise sequences push error toward frequencies that are less visually objectionable.

The estimator can remain mathematically correct while the *spatial structure* of finite-sample error changes dramatically.

That will matter later for real-time rendering.

---

**Current Kairo progressive testing**

The current tests verify that progressive path rendering:

$$
\text{produces finite pixels}
$$

and:

$$
\text{is deterministic for the same seeds}.
$$

That is good infrastructure.

What is still absent is a real convergence test checking:

$$
\text{error decreases with sample count}
$$

and a per-pixel variance estimator.

Those are natural M124 additions.

---

**Research-quality variance comparison**

To compare sampling strategies, hold everything fixed except the sampler.

For example render the same scene using:

$$
\text{uniform hemisphere},
$$

$$
\text{cosine hemisphere},
$$

$$
\text{GGX NDF},
$$

$$
\text{GGX VNDF},
$$

$$
\text{light sampling},
$$

$$
\text{MIS}.
$$

Measure at fixed:

$$
\text{ray count}
$$

rather than merely fixed SPP.

Why?

A technique that launches three shadow rays per bounce costs more than one that launches none.

Quality per sample and quality per unit time are different metrics.

The meaningful research quantity is often:

$$
\boxed{
\text{error versus compute budget}.
}
$$

---

**Variance and BVH cost interact**

Suppose method A requires:

$$
100
$$

samples for target variance but each sample costs:

$$
5
$$

rays.

Method B needs:

$$
200
$$

samples but costs:

$$
1
$$

ray each.

Then:

$$
A=500\text{ rays},
$$

$$
B=200\text{ rays}.
$$

So “lower variance per sample” does not automatically mean faster rendering.

Integrator design must optimize:

$$
\boxed{
\text{variance}\times\text{cost}
}
$$

or an equivalent time-normalized metric.

This becomes very important with complex MIS and next-event estimation.

---

**Path tracing is probability bookkeeping**

At this stage the core renderer is no longer difficult because ray intersection is mysterious.

The hard part is maintaining a consistent probability story.

For every random choice you must be able to answer:

$$
\text{What was sampled?}
$$

$$
\text{Under what measure?}
$$

$$
\text{What was its PDF?}
$$

$$
\text{Did another technique also sample this event?}
$$

$$
\text{If yes, what is the MIS weight?}
$$

$$
\text{How does this choice update throughput?}
$$

If you cannot answer those questions for every bounce, the integrator is not yet trustworthy.

---

**A more complete Kairo-style path-loop target**

```cpp
Color TracePath(
    const Scene& scene,
    Ray ray,
    RNG& rng)
{
    Color L =
        Color::Black();

    Color beta =
        Color::White();

    float previousBsdfPdf = 0.0f;
    bool previousDelta = true;

    for (uint32_t bounce = 0;
         bounce < MaxDepth;
         ++bounce)
    {
        const auto hit =
            scene.Intersect(ray);

        if (!hit)
        {
            const Color Le =
                scene.SampleEnvironment(
                    ray.Direction);

            if (bounce == 0 ||
                previousDelta)
            {
                L += beta * Le;
            }
            else
            {
                const float lightPdf =
                    scene.EnvironmentPdf(
                        ray.Direction);

                const float w =
                    PowerHeuristic(
                        previousBsdfPdf,
                        lightPdf);

                L +=
                    beta *
                    Le *
                    w;
            }

            break;
        }

        const Material& material =
            scene.Materials[
                hit->MaterialIndex];

        // -----------------------------------------------------
        // Emissive surface hit.
        // -----------------------------------------------------
        if (IsEmitter(material))
        {
            const Color Le =
                material.Emission;

            if (bounce == 0 ||
                previousDelta)
            {
                L += beta * Le;
            }
            else
            {
                const float lightPdf =
                    scene.PdfEmitterHit(
                        ray,
                        *hit);

                const float w =
                    PowerHeuristic(
                        previousBsdfPdf,
                        lightPdf);

                L +=
                    beta *
                    Le *
                    w;
            }
        }

        const Vec3 wo =
            -ray.Direction;

        // -----------------------------------------------------
        // Next-event estimation for non-delta BSDFs.
        // -----------------------------------------------------
        if (!IsPureDelta(material))
        {
            LightSample light =
                scene.SampleLight(
                    *hit,
                    rng);

            if (light.Valid &&
                light.Pdf > 0.0f)
            {
                const float cosTheta =
                    std::max(
                        Dot(
                            hit->ShadingNormal,
                            light.Wi),
                        0.0f);

                if (cosTheta > 0.0f &&
                    !scene.Occluded(
                        SpawnShadowRay(
                            *hit,
                            light.Wi,
                            light.Distance)))
                {
                    const Color f =
                        EvaluateBSDF(
                            material,
                            *hit,
                            wo,
                            light.Wi);

                    const float bsdfPdf =
                        PdfBSDF(
                            material,
                            *hit,
                            wo,
                            light.Wi);

                    const float w =
                        light.IsDelta
                        ? 1.0f
                        : PowerHeuristic(
                            light.Pdf,
                            bsdfPdf);

                    L +=
                        beta *
                        f *
                        light.Radiance *
                        (cosTheta *
                         w /
                         light.Pdf);
                }
            }
        }

        // -----------------------------------------------------
        // Continue path by BSDF sampling.
        // -----------------------------------------------------
        BSDFSample bsdf =
            SampleBSDF(
                material,
                *hit,
                wo,
                rng);

        if (!bsdf.Valid ||
            bsdf.Pdf <= 0.0f)
        {
            break;
        }

        const float cosine =
            bsdf.IsDelta
            ? 1.0f
            : std::abs(
                Dot(
                    hit->ShadingNormal,
                    bsdf.Wi));

        beta *=
            bsdf.F *
            (cosine /
             bsdf.Pdf);

        if (!IsFinite(beta) ||
            MaxComponent(beta) <= 0.0f)
        {
            break;
        }

        previousBsdfPdf =
            bsdf.Pdf;

        previousDelta =
            bsdf.IsDelta;

        ray =
            SpawnRay(
                *hit,
                bsdf.Wi,
                bsdf.IsTransmission);

        // -----------------------------------------------------
        // Russian roulette.
        // -----------------------------------------------------
        if (bounce >= 3)
        {
            const float survival =
                std::clamp(
                    MaxComponent(beta),
                    0.05f,
                    0.95f);

            if (Random01(rng) >
                survival)
            {
                break;
            }

            beta /= survival;
        }
    }

    return L;
}
```

There are still subtleties around delta BSDF conventions, transmission Jacobians, shading normals, nested media and light-selection PDFs, but this architecture is qualitatively closer to a complete modern path tracer than current `TracePath()`.

---

**The deepest connection: every “rendering trick” is a sampling strategy**

A hard shadow ray is a very specialized light sample.

An area-light shadow ray is a sample from emitter area.

A cosine bounce is a BRDF-oriented sample.

GGX sampling is specular-lobe importance sampling.

Environment CDF sampling is incoming-radiance importance sampling.

MIS combines multiple distributions because no one proposal distribution tracks the entire integrand perfectly.

Russian roulette samples path length.

Subpixel jitter samples the pixel footprint.

Even glass Fresnel selection samples between discrete transport events.

Thus the path tracer can be understood as:

$$
\boxed{
\text{a carefully constructed sequence of random variables whose expectation equals the rendering equation.}
}
$$

That is the real conceptual leap of M123/M124.

---

**Current Kairo M123/M124 audit**

| Concept | Current Kairo |
|---|---|
| Subpixel stratification | Yes |
| Uniform sphere sampler | Not exposed in current path code |
| Uniform hemisphere sampler | Not current path strategy |
| Cosine hemisphere sampler | Yes |
| Surface-basis transform | Yes |
| Lambert importance sampling | Yes |
| GGX NDF sampling | No in `PathIntegrator` |
| GGX VNDF sampling | No |
| Power heuristic function | Yes |
| Complete two-sided MIS | No |
| Diffuse path throughput | Implicit via Lambert/cosine cancellation |
| Generic `f*cos/pdf` path loop | No |
| Maximum depth | Yes |
| Russian roulette | Yes |
| Survival from accumulated throughput | No; current surface albedo used |
| Point-light NEE | Yes |
| Directional/spot NEE | Yes |
| Area-light sampling | Fixed sample grid |
| Correct area→solid-angle PDF formula | Computed |
| Physically complete area-light estimator | Not yet |
| Environment miss radiance | Yes |
| Environment NEE | No |
| Environment importance map | No |
| CDF/alias sampler | No |
| Multiple samples/pixel | Yes |
| Progressive passes | Yes |
| Online per-pixel mean | Not yet |
| Per-pixel variance | Not yet |
| Firefly instrumentation | Not yet |
| Explicit delta/glossy/diffuse event types | Not in path integrator |
| Fresnel path-event selection | Whitted only, not current Path mode |
| Mirror transport in Path mode | Not yet |
| Glass transport in Path mode | Not yet |
| PBR/GGX indirect transport | Not yet |

This is an excellent place to draw the line between “features whose source code exists somewhere in the renderer” and “features integrated into a mathematically unified path tracer.”

---

**PASS 7 RECONSTRUCTION SHEET**

| Concept | Core equation |
|---|---|
| Uniform sphere | $p=1/(4\pi)$ |
| Uniform hemisphere | $p=1/(2\pi)$ |
| Uniform sphere sample | $z=1-2\xi,\;\phi=2\pi\xi$ |
| Cosine hemisphere | $p=\cos\theta/\pi$ |
| Cosine sample | $r=\sqrt{\xi},\;\phi=2\pi\xi$ |
| Local→world | $\omega=xT+yN+zB$ |
| Lambert | $f=\rho/\pi$ |
| Lambert+cosine cancellation | $f\cos/p=\rho$ |
| MC variance term | $\int f^2/p-I^2$ |
| Ideal importance PDF | $p^*\propto|f|$ |
| GGX half-vector PDF | $p_H=D(H)(N\cdot H)$ |
| Reflection Jacobian | $p_L=p_H/(4|V\cdot H|)$ |
| VNDF | condition GGX normals on visibility from $V$ |
| Balance MIS | $w_i=n_ip_i/\sum n_jp_j$ |
| Power MIS | $w_i=(n_ip_i)^2/\sum(n_jp_j)^2$ |
| Throughput | $\beta\leftarrow\beta f|\cos\theta|/p$ |
| Emission contribution | $L\leftarrow L+\beta L_e$ |
| Russian roulette | survive with $p_s$, then $\beta\leftarrow\beta/p_s$ |
| NEE | explicitly sample a light at current vertex |
| Uniform area PDF | $p_A=1/A$ |
| Area→solid-angle | $p_\omega=p_A r^2/|\cos\theta_L|$ |
| Point light | delta direction, inverse-square intensity |
| Uniform environment | $p_\omega=1/(4\pi)$ |
| Lat-long solid angle | $d\omega=2\pi^2\sin\theta\,du\,dv$ |
| Env importance weight | $w_{ij}=Y_{ij}\sin\theta_j$ |
| CDF sampling | invert cumulative probability |
| Alias sampling | $O(1)$ discrete sampling after preprocessing |
| Pixel mean | $\bar X_N=N^{-1}\sum X_i$ |
| Online mean | $\mu_n=\mu_{n-1}+(x_n-\mu_{n-1})/n$ |
| Sample variance | $s^2=M_2/(n-1)$ |
| Monte Carlo noise | $\sigma\propto1/\sqrt N$ |
| MSE | variance + bias² |
| Firefly | rare huge $f/p$ contribution |
| Clamping | lowers variance but introduces bias |
| Delta specular | probability mass concentrated on exact direction |
| Glass selection | reflection with $F$, transmission with $1-F$ |

The curriculum explicitly places M123 and M124 as the sampling, MIS, throughput, NEE, variance and path-event foundation before the later production path-tracer work.

**Blank-page mastery gate**

From no notes, derive uniform-sphere sampling by starting from:

$$
d\omega=\sin\theta\,d\theta\,d\phi
$$

and showing why:

$$
z=\cos\theta
$$

must be uniform.

Then derive cosine hemisphere sampling from:

$$
p_\theta=2\cos\theta\sin\theta
$$

until you get:

$$
\sin\theta=\sqrt{\xi}.
$$

Then show why Lambert plus cosine sampling yields:

$$
\frac{
(\rho/\pi)\cos\theta
}{
\cos\theta/\pi
}
=
\rho.
$$

Then derive:

$$
\operatorname{Var}
\left[
\frac{f(X)}{p(X)}
\right]
=
\int\frac{f^2}{p}-I^2
$$

and use it to explain importance sampling.

Then reconstruct GGX half-vector sampling and explain why converting:

$$
H\rightarrow L
$$

requires:

$$
\frac1{4|V\cdot H|}.
$$

Then derive the balance and power MIS heuristics and explain why weighting only the light-sampled half of a two-technique MIS scheme is incomplete.

Then reconstruct:

$$
\beta_{k+1}
=
\beta_k
\frac{
f_k|\cos\theta_k|
}{
p_k
}.
$$

Then prove Russian roulette is unbiased:

$$
E[X]=
p_s(C/p_s)=C.
$$

Then derive:

$$
d\omega
=
\frac{
\cos\theta_L
}{
r^2
}
dA
$$

and therefore:

$$
p_\omega
=
p_A
\frac{
r^2
}{
\cos\theta_L
}.
$$

Then explain why environment-map importance weights require:

$$
Y\sin\theta.
$$

Finally explain the difference between:

$$
\text{diffuse},
\quad
\text{glossy},
\quad
\text{delta specular},
$$

and why ideal glass can choose reflection with probability $F$ and transmission with probability $1-F$ without tracing both branches.

If you can reconstruct all of those pieces, then Monte Carlo path tracing is no longer “random rays.” It is a controlled probability estimator whose every random choice has a measure, a PDF, a transport factor and a reason.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/08 - Sampling Directions and Light Transport|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation|Next chapter]]
