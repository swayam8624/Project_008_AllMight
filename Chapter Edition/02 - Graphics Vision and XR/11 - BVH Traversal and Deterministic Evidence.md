# 11 — BVH Traversal and Deterministic Evidence

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty|Next chapter]]

A BVH accelerates ray queries by pruning regions, but its traversal behavior still needs inspection. Heatmaps and deterministic image tests expose performance patterns and regressions that a single final render can hide.

**By the end:** Use traversal visualizations and deterministic tests to evaluate a BVH implementation.

**Recall question:** What can a traversal heatmap reveal that a rendered image cannot?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 24622–27310; source-span SHA256 1225f024e04f63c2ec9acfc23dfc104098fe9e3d8265b7750ef0d546157741a9. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### PARTS 2090–2100

**BVH traversal heatmap**

A normal image shows what the ray hit.

A traversal heatmap shows how much work was required.

Current Kairo counts:

$$
\text{BVHVisitedNodes}
$$

for a single ray and maps approximately:

$$
h=
\operatorname{clamp}
\left(
\frac{
N_{\text{visited}}
}{
32
},
0,1
\right).
$$

Then produces a blue→red-style diagnostic mapping.

This lets you see regions where BVH structure performs poorly.

---

**What traversal heatmaps reveal**

Imagine most image is dark blue:

$$
5\text{ nodes/ray}
$$

but one cluster is red:

$$
50\text{ nodes/ray}.
$$

Possible causes include:

- overlapping child bounds,
- poor split heuristic,
- giant primitives,
- badly distributed geometry,
- too-large leaves.

The beauty image does not reveal these.

Performance diagnostics need their own visualization.

---

**Kairo's AccelerationDifference mode is especially good**

Current Kairo computes the BVH hit and brute-force hit.

It compares:

$$
\text{primitive ID},
$$

$$
t,
$$

$$
\text{material},
$$

$$
UV.
$$

Agreement:

$$
\text{black}.
$$

Mismatch:

$$
\text{red}.
$$

That is exactly how acceleration structures should be developed:

$$
\boxed{
\text{optimize against an obviously correct oracle}.
}
$$

Do not debug the BVH and triangle intersection simultaneously.

---

**Deterministic rendering tests**

For the same:

$$
\text{scene},
\text{seed},
\text{sample count},
\text{configuration},
$$

a deterministic CPU renderer should ideally produce the same image.

Kairo already tests repeated renders of:

$$
\text{Whitted},
\text{PBR},
\text{Path}
$$

for exact equality under identical settings.

This is very valuable.

A stochastic renderer does not have to be nondeterministic.

The samples can be pseudo-random but reproducible.

---

**Thread-count determinism**

An even stronger test is:

$$
I_{\text{1 thread}}
=
I_{\text{8 threads}}.
$$

Kairo currently tests this for Whitted.

Because Path mode derives RNG state from pixel/sample identity rather than worker identity, it is architecturally well positioned to pass the same test for Path mode too.

I would explicitly add:

```cpp
TEST_CASE(
    "Path render is invariant to thread count")
{
    Scene a = MakeScene();
    Scene b = a;

    a.Settings.ThreadCount = 1;
    b.Settings.ThreadCount = 8;

    a.Settings.SampleSeed =
        b.Settings.SampleSeed = 12345;

    const Film A =
        Renderer{}.Render(a).Image;

    const Film B =
        Renderer{}.Render(b).Image;

    RequireImagesNear(
        A,
        B,
        0.0f);
}
```

That proves work scheduling does not alter sampling.

---

**Cross-platform determinism is harder**

Exact equality across:

- CPU architectures,
- compilers,
- optimization flags

is more difficult because floating-point expression contraction and evaluation order may differ.

For same binary/machine:

$$
0\text{-tolerance}
$$

may be appropriate.

Across platforms, use tight numerical tolerances.

Determinism should be scoped precisely.

---

**Reference-image testing**

A reference-image test stores a known-correct result:

$$
R.
$$

Render:

$$
I.
$$

Compare.

Simple RMSE:

$$
\boxed{
RMSE
=
\sqrt{
\frac1N
\sum_i
(I_i-R_i)^2
}.
}
$$

Maximum absolute error:

$$
\boxed{
E_{\max}
=
\max_i|I_i-R_i|.
}
$$

You often want both.

A tiny RMSE can hide one catastrophic wrong pixel.

---

**Compare in linear space**

If one image is tone-mapped/gamma-encoded and the other isn't, your metric is meaningless.

For renderer correctness, compare:

$$
\boxed{\text{linear HDR radiance}}
$$

whenever possible.

Display-space reference images are useful for final presentation regression, but they mix:

- integrator,
- exposure,
- tone mapper,
- transfer function.

That makes root-cause isolation harder.

---

**Reference images for a stochastic integrator**

A 4-SPP path image should not necessarily equal a high-SPP reference.

Instead use either deterministic same-sample regression or statistical convergence.

For code regression:

same seed + same SPP should remain stable.

For physical correctness:

compare increasing SPP against a high-quality reference.

Those are different tests.

---

**Monte Carlo convergence test**

Suppose ground-truth/reference image is:

$$
R.
$$

Render:

$$
I_1,\ I_4,\ I_{16},\ I_{64},\ I_{256}.
$$

Compute errors:

$$
E_N.
$$

For ordinary Monte Carlo we expect approximately:

$$
\boxed{
E_N
\propto
\frac1{\sqrt N}.
}
$$

Thus quadrupling samples should approximately halve stochastic error.

---

**Do not assert strict monotonic error per single realization**

Because rendering is stochastic, it is possible:

$$
E_{64}>E_{16}
$$

for one particular random sample set even when the estimator is perfectly correct.

A robust convergence test can:

- use nested sample prefixes,
- average across several seeds,
- test approximate slope statistically,
- assert high-SPP error is substantially below low-SPP error rather than every step monotonically improving.

The mathematical convergence trend matters more than one accidental fluctuation.

---

**Log-log convergence slope**

Take:

$$
\log E_N
$$

versus:

$$
\log N.
$$

If:

$$
E\propto N^{-1/2},
$$

then:

$$
\log E
=
-\frac12\log N+C.
$$

Expected slope for root-mean-square error with finite variance and independent samples:

$$
\boxed{-0.5.}
$$

Mean-square error instead has expected slope $-1$. Correlated sampling, heavy tails and reference noise can change the observed curve; a plateau is a diagnostic clue, not proof of a particular bug.

This is a surprisingly powerful test.

If the curve stops dropping and plateaus:

$$
\boxed{
\text{you likely hit bias}.
}
$$

Examples:

- wrong BRDF,
- incorrect MIS,
- fixed-depth truncation,
- clamping,
- missing indirect path family.

---

**Measuring rays per second**

Basic metric:

$$
\boxed{
RPS
=
\frac{
N_{\text{rays}}
}{
t_{\text{seconds}}
}.
}
$$

Kairo already computes this from `TotalRays(stats)` and `RenderMilliseconds`.

But there is a current problem.

---

**Current Kairo Path rays/sec is incomplete**

`TotalRays()` currently counts:

$$
\text{PrimaryRays}
+
\text{ShadowRays}
+
\text{ReflectionRays}.
$$

Current `PathIntegrator.cppm` increments:

$$
\text{ShadowRays}
$$

but does not increment a counter for ordinary indirect bounce rays.

Its recursive call:

```cpp
TracePath(
    scene,
    bounceRay,
    depth + 1,
    ...)
```

is therefore a real ray traversal that is absent from `TotalRays()`.

So for Path mode:

$$
\boxed{
\text{current RaysPerSecond denominator is undercounted}.
}
$$

That should be corrected before treating path-tracer RPS as a benchmark.

---

**Better ray statistics**

Conceptually track:

```cpp
PrimaryRays
ShadowRays
DiffuseBounceRays
GlossyBounceRays
SpecularRays
TransmissionRays
```

Then:

$$
\text{TotalRays}
=
\sum.
$$

Also track:

$$
\text{PrimitiveTests/s},
$$

$$
\text{AABBTests/s},
$$

$$
\text{NodesVisited/ray}.
$$

Because two ray tracers can have identical RPS while one executes twice as many BVH tests per ray.

---

**Benchmark the same workload**

Rays/sec only means something when workload is controlled.

A primary camera ray through empty sky is far cheaper than a diffuse path ray through dense geometry.

Measure:

$$
\text{same scene},
$$

$$
\text{same resolution},
$$

$$
\text{same SPP},
$$

$$
\text{same depth},
$$

$$
\text{same compiler mode}.
$$

Separate build time from traversal time when comparing BVHs.

---

**Current CPU BVH traversal has a major easy optimization**

KairoSpatial currently creates:

```cpp
std::vector<SpatialIndex> stack;
stack.reserve(64);
```

inside every `Raycast()`.

That likely means a heap allocation for every ray.

At:

$$
10^8
$$

rays, even an excellent allocator cannot make this free.

Replace with a fixed local stack:

```cpp
std::array<SpatialIndex, 64> stack;
uint32_t stackSize = 0;
```

for normal binary BVH depth, with a safe overflow fallback/assertion.

This is probably one of the highest-value straightforward CPU traversal optimizations in the current code.

---

**Current closest-hit path allocates hit storage unnecessarily**

`SpatialRaycastResult` currently does:

```cpp
result.Hits.push_back(*hit);
```

every time a closer primitive is discovered.

But `Scene::Intersect()` only needs:

$$
\text{Closest}.
$$

Therefore the general spatial query API is paying for a dynamic hit vector even when the ray tracer wants one hit.

A specialized:

```cpp
RaycastClosest()
```

that stores only:

```cpp
bool hit;
SpatialRayHit closest;
```

would remove allocation and memory writes.

General APIs are convenient.

Hot paths deserve specialized variants.

---

**Duplicate AABB testing**

Current closest traversal tests a node when popped.

For an internal node, it then tests both child AABBs to determine near/far ordering.

Those children are pushed.

When a child is later popped, its AABB is tested again.

So child bounds can be evaluated twice.

A traversal stack can instead store:

```cpp
{ nodeIndex, tNear }
```

and avoid repeating a test already performed at the parent.

This trades stack size for fewer slab tests.

Benchmark it.

---

**Precompute inverse direction**

AABB slab intersection repeatedly computes:

$$
\frac1{D_x},
\quad
\frac1{D_y},
\quad
\frac1{D_z}.
$$

A ray traversal can precompute:

$$
\boxed{
invD=
(1/D_x,1/D_y,1/D_z)
}
$$

and signs once.

Then each box intersection becomes mostly:

- subtract,
- multiply,
- min/max.

Across dozens of AABB tests per ray, that matters.

Handle zero/near-zero components carefully.

---

**Node memory layout**

Traversal performance is often memory-bound.

A BVH node should be compact.

Each unnecessary byte means fewer nodes per cache line.

Potential improvements include:

$$
\text{32-byte nodes},
$$

$$
\text{implicit child indices},
$$

$$
\text{leaf bit packing},
$$

$$
\text{contiguous depth-first layout}.
$$

A sophisticated CPU renderer may use:

$$
BVH4
$$

or:

$$
BVH8
$$

instead of binary BVH so SIMD lanes test several child boxes simultaneously.

---

**BVH4/BVH8 intuition**

Binary node:

$$
2
$$

children.

An AVX2 CPU has:

$$
8
$$

float lanes.

Testing one child at a time wastes SIMD width.

A wide BVH can test:

$$
4
$$

or:

$$
8
$$

AABBs together.

This can improve:

- SIMD utilization,
- traversal depth,
- memory efficiency.

The tradeoff is more complex building/traversal and potentially looser bounds.

---

**Triangle intersection optimization**

Möller–Trumbore currently recomputes:

$$
E_1=B-A,
$$

$$
E_2=C-A
$$

for every test unless geometry representation has them precomputed.

For static ray-tracing meshes, store triangle data optimized for intersection rather than only traditional raster vertex/index representation.

Possible representations precompute:

$$
A,
E_1,E_2
$$

or use specialized watertight intersection formulations.

The best representation depends on memory bandwidth and SIMD strategy.

---

**Watertight triangle intersection**

Möller–Trumbore can produce precision inconsistencies on shared edges.

Two adjacent triangles may disagree about whether a ray exactly on their common edge intersects.

This can create tiny cracks.

Production ray tracers often use **watertight ray-triangle intersection** algorithms designed so adjacent triangles agree under floating-point arithmetic.

That becomes important for:

- distant geometry,
- large coordinate magnitudes,
- shadow rays,
- thin meshes.

---

**TLAS and BLAS**

Suppose a mesh has:

$$
100,000
$$

triangles and appears:

$$
500
$$

times.

Flattening all instances produces:

$$
50\text{ million}
$$

triangles in one BVH.

Instead use:

$$
\boxed{
BLAS=\text{bottom-level acceleration structure per mesh}
}
$$

and:

$$
\boxed{
TLAS=\text{top-level structure over instances}.
}
$$

Ray first intersects instance bounds.

Then transforms into local space and traverses mesh BLAS.

This is also the basic organization used by modern hardware ray-tracing APIs.

---

**Packet ray tracing**

A CPU SIMD core can process multiple rays together.

A packet:

$$
R_0,R_1,R_2,R_3
$$

for SSE-like 4-wide processing, or larger vectors for AVX.

Store origins as structure of arrays:

```text
Ox = [Ox0 Ox1 Ox2 Ox3 ...]
Oy = [Oy0 Oy1 Oy2 Oy3 ...]
Oz = [...]
```

Then AABB computations operate on multiple rays in one SIMD instruction.

---

**Why primary rays make good packets**

Neighboring primary pixels have coherent directions.

They tend to traverse similar BVH nodes.

Therefore a packet can traverse:

$$
\text{one node}
$$

for many rays simultaneously.

This improves SIMD utilization.

Shadow rays toward the same light can also be coherent.

---

**Why diffuse path rays destroy packet coherence**

After a diffuse bounce:

$$
\omega_i
$$

is randomly sampled.

Neighboring camera paths can fly in completely different directions.

Then within one packet:

```text
ray 0 → left subtree
ray 1 → right subtree
ray 2 → miss
ray 3 → left
```

SIMD lanes diverge.

Eventually packet tracing can spend time processing inactive lanes.

This is one reason packet tracing is more naturally effective for coherent primary/secondary rays than for deep diffuse path tracing.

---

**Megakernel path tracing**

The simplest GPU design:

One GPU thread owns one path.

It performs:

```text
generate ray
intersect
shade
sample light
shadow
sample BSDF
repeat
```

inside one large shader/kernel.

This is called a **megakernel** architecture.

Advantages:

- simple path state,
- low queue traffic,
- straightforward control flow.

Disadvantage:

Different threads take different branches and bounce counts.

SIMT divergence can become severe.

---

**SIMT divergence preview**

Suppose a GPU warp/wave contains:

```text
lane 0 → diffuse
lane 1 → mirror
lane 2 → glass
lane 3 → miss
...
```

The GPU may have to execute multiple branch paths while masking inactive lanes.

So although each ray is independent logically, executing all ray types together can waste hardware throughput.

Phase 22 will go extremely deep into exactly this SIMT behavior.

---

**Wavefront path tracing**

Wavefront rendering decomposes one giant path kernel into stages.

Conceptually:

```text
Ray Queue
   ↓
Intersection stage
   ↓
Hit Queue / Miss Queue
   ↓
Material shading stage
   ↓
Shadow Queue
   ↓
BSDF continuation Queue
   ↓
Intersection stage again
```

Instead of letting a warp contain arbitrary path states, work is regrouped by operation.

---

**Wavefront advantage**

Suppose 1,000,000 active paths exist.

After intersection:

$$
400k
$$

hit diffuse,

$$
100k
$$

hit glass,

$$
500k
$$

miss.

Wavefront compaction can process homogeneous batches.

That improves:

- branch coherence,
- resource utilization,
- specialization opportunities.

Especially on GPUs.

---

**Wavefront cost**

Queues require:

- memory for path state,
- atomic/parallel append,
- compaction,
- synchronization,
- more global memory traffic.

So wavefront is not automatically faster.

For short coherent workloads, a megakernel may win.

For deeply divergent, complex BSDF workloads, wavefront often becomes attractive.

Again:

$$
\boxed{\text{measure}.}
$$

---

**Path state for wavefront rendering**

Each active path needs something like:

```cpp
struct PathState
{
    Ray Ray;

    Color Throughput;
    Color Radiance;

    RNGState RNG;

    uint32_t PixelIndex;
    uint32_t Depth;

    MediumStack Media;

    float PreviousPdf;
    uint32_t Flags;
};
```

Millions of these states exist simultaneously.

Memory layout becomes crucial.

---

**AoS versus SoA path state**

Array of structures:

```text
Ray, beta, RNG, depth
Ray, beta, RNG, depth
Ray, beta, RNG, depth
```

Structure of arrays:

```text
RayOrigins[]
RayDirections[]
BetaR[]
BetaG[]
BetaB[]
Depth[]
...
```

GPU/SIMD stages often benefit from SoA because each stage touches only a subset of fields.

Intersection needs:

$$
O,D.
$$

It does not care about:

$$
\text{radiance accumulator}.
$$

Loading giant AoS path records wastes bandwidth.

---

**GPU path tracing architecture**

A compute/software GPU tracer generally needs:

$$
\text{camera sample generation},
$$

$$
\text{GPU BVH/acceleration traversal},
$$

$$
\text{material buffers},
$$

$$
\text{texture sampling},
$$

$$
\text{light sampling structures},
$$

$$
\text{path state},
$$

$$
\text{RNG/low-discrepancy state},
$$

$$
\text{film accumulation}.
$$

Hardware RT APIs replace much of the explicit triangle/BVH traversal machinery, but not:

- BSDF logic,
- PDFs,
- MIS,
- light sampling,
- path-state scheduling.

The difficult Monte Carlo reasoning survives hardware acceleration completely.

---

**Hardware ray tracing is not a path tracer**

Hardware may accelerate:

$$
\boxed{
\text{ray → acceleration structure → triangle hit}
}
$$

but it does not automatically solve:

$$
L_o=
L_e+
\int fL\cos\theta\,d\omega.
$$

You still design:

- BSDF sampling,
- light selection,
- MIS,
- Russian roulette,
- media,
- accumulation.

Hardware acceleration makes visibility queries faster.

The integrator remains your algorithm.

---

**A small physically based path tracer — architecture**

Now we combine everything from Passes 6–8.

We need:

```text
Camera
Sampler
Scene/BVH
Hit records
BSDF system
Light sampler
Environment sampler
Medium state
MIS
Russian roulette
Film accumulator
```

The execution per camera sample is:

$$
\text{camera}
\rightarrow
\text{closest hit}
\rightarrow
\text{emission}
\rightarrow
\text{NEE}
\rightarrow
\text{BSDF event}
\rightarrow
\text{throughput}
\rightarrow
\text{medium update}
\rightarrow
\text{roulette}
\rightarrow
\text{next ray}.
$$

---

**Core BSDF interface**

```cpp
enum class BSDFEvent : uint32_t
{
    None         = 0,
    Diffuse      = 1 << 0,
    Glossy       = 1 << 1,
    Delta        = 1 << 2,
    Reflection   = 1 << 3,
    Transmission = 1 << 4
};

struct BSDFSample
{
    Vec3 Wi;

    Color F;

    // Directional density in sr^-1 for continuous lobes.
    // For delta events this field follows the renderer's
    // chosen discrete/delta convention.
    float Pdf = 0.0f;

    BSDFEvent Event =
        BSDFEvent::None;

    float Eta = 1.0f;

    [[nodiscard]]
    bool IsDelta() const
    {
        return HasFlag(
            Event,
            BSDFEvent::Delta);
    }
};
```

Evaluate:

```cpp
Color EvaluateBSDF(...);
```

PDF:

```cpp
float PdfBSDF(...);
```

Sampling:

```cpp
BSDFSample SampleBSDF(...);
```

Those three operations should agree mathematically.

---

**Why Evaluate/Pdf/Sample agreement matters**

Suppose `SampleBSDF()` generates GGX directions according to:

$$
p_{\text{GGX}}.
$$

But `PdfBSDF()` accidentally returns cosine PDF:

$$
\frac{N\cdot L}{\pi}.
$$

Then:

$$
\frac{f\cos}{p}
$$

is wrong.

Your result may still look glossy.

It will not converge to the rendering equation.

One of the most important integrator invariants is:

$$
\boxed{
\text{the PDF reported for a generated sample must equal the distribution that actually generated it}.
}
$$

---

**Light sample record**

```cpp
struct LightSample
{
    Vec3 Wi;

    Color Radiance;

    float Distance;

    // Complete PDF including light-selection probability.
    float Pdf;

    bool Delta;
};
```

If there are $N$ lights chosen uniformly:

$$
P(\text{choose light})
=
\frac1N.
$$

Then if an area light directional PDF is:

$$
p_\omega,
$$

complete PDF is:

$$
\boxed{
p_L=
\frac1N p_\omega.
}
$$

Forgetting the light-selection probability produces an estimator scaled by light count.

Classic bug.

---

**Complete MIS-ready path loop**

```cpp
Color TracePath(
    const Scene& scene,
    Ray ray,
    Sampler& sampler)
{
    Color L =
        Color::Black();

    Color beta =
        Color::White();

    MediumStack media =
        MediumStack::Air();

    // Data needed to MIS-weight a light/environment
    // reached by the previous BSDF sample.
    float previousBSDFPdf = 0.0f;
    bool previousWasDelta = true;

    for (uint32_t bounce = 0;
         bounce < MaxDepth;
         ++bounce)
    {
        //-----------------------------------------------------
        // 1. Closest visibility query.
        //-----------------------------------------------------
        const auto hit =
            scene.IntersectClosest(ray);

        //-----------------------------------------------------
        // 2. Environment miss.
        //-----------------------------------------------------
        if (!hit)
        {
            const Color Le =
                scene.EnvironmentRadiance(
                    ray.Direction);

            if (bounce == 0 ||
                previousWasDelta)
            {
                L +=
                    beta * Le;
            }
            else
            {
                const float lightPdf =
                    scene.EnvironmentPdf(
                        ray.Direction);

                const float weight =
                    PowerHeuristic(
                        previousBSDFPdf,
                        lightPdf);

                L +=
                    beta *
                    Le *
                    weight;
            }

            break;
        }

        const Material& material =
            scene.Material(
                hit->MaterialIndex);

        const Vec3 wo =
            -ray.Direction;

        //-----------------------------------------------------
        // 3. Surface emission.
        //-----------------------------------------------------
        if (material.IsEmitter())
        {
            const Color Le =
                material.Emission;

            if (bounce == 0 ||
                previousWasDelta)
            {
                L +=
                    beta * Le;
            }
            else
            {
                const float lightPdf =
                    scene.PdfLightHit(
                        ray,
                        *hit);

                const float weight =
                    PowerHeuristic(
                        previousBSDFPdf,
                        lightPdf);

                L +=
                    beta *
                    Le *
                    weight;
            }
        }

        //-----------------------------------------------------
        // 4. Explicit direct-light sample.
        //
        // Skip ordinary NEE for pure delta BSDFs because an
        // arbitrary light direction has zero BSDF support.
        //-----------------------------------------------------
        if (!material.IsPureDelta())
        {
            const LightSample light =
                scene.SampleOneLight(
                    *hit,
                    sampler);

            if (light.Pdf > 0.0f)
            {
                const float cosTheta =
                    std::max(
                        Dot(
                            hit->ShadingNormal,
                            light.Wi),
                        0.0f);

                if (cosTheta > 0.0f)
                {
                    const Ray shadow =
                        SpawnShadowRay(
                            *hit,
                            light.Wi,
                            light.Distance);

                    if (!scene.Occluded(shadow))
                    {
                        const Color f =
                            EvaluateBSDF(
                                material,
                                *hit,
                                wo,
                                light.Wi,
                                media);

                        const float bsdfPdf =
                            PdfBSDF(
                                material,
                                *hit,
                                wo,
                                light.Wi,
                                media);

                        const float misWeight =
                            light.Delta
                            ? 1.0f
                            : PowerHeuristic(
                                light.Pdf,
                                bsdfPdf);

                        L +=
                            beta *
                            f *
                            light.Radiance *
                            (
                                cosTheta *
                                misWeight /
                                light.Pdf
                            );
                    }
                }
            }
        }

        //-----------------------------------------------------
        // 5. Sample the BSDF to continue the path.
        //-----------------------------------------------------
        BSDFSample bs =
            SampleBSDF(
                material,
                *hit,
                wo,
                media,
                sampler);

        if (!bs.IsValid())
            break;

        //-----------------------------------------------------
        // 6. Throughput update.
        //-----------------------------------------------------
        if (bs.IsDelta())
        {
            // Delta BSDF routines normally return an F/PDF
            // convention already designed for the exact event.
            beta *=
                bs.F /
                bs.Pdf;
        }
        else
        {
            const float cosine =
                std::abs(
                    Dot(
                        hit->ShadingNormal,
                        bs.Wi));

            if (cosine <= 0.0f ||
                bs.Pdf <= 0.0f)
            {
                break;
            }

            beta *=
                bs.F *
                (cosine / bs.Pdf);
        }

        if (!IsFinite(beta) ||
            MaxComponent(beta) <= 0.0f)
        {
            break;
        }

        //-----------------------------------------------------
        // 7. Update medium ONLY after transmission.
        //-----------------------------------------------------
        if (HasFlag(
                bs.Event,
                BSDFEvent::Transmission))
        {
            UpdateMediumStack(
                media,
                *hit,
                material);
        }

        previousWasDelta =
            bs.IsDelta();

        previousBSDFPdf =
            bs.Pdf;

        //-----------------------------------------------------
        // 8. Spawn robustly offset next ray.
        //-----------------------------------------------------
        ray =
            SpawnSurfaceRay(
                *hit,
                bs.Wi,
                HasFlag(
                    bs.Event,
                    BSDFEvent::Transmission));

        //-----------------------------------------------------
        // 9. Russian roulette.
        //-----------------------------------------------------
        if (bounce >= 3)
        {
            const float survival =
                std::clamp(
                    MaxComponent(beta),
                    0.05f,
                    0.95f);

            if (sampler.Next1D(
                    SampleDimension::Roulette) >
                survival)
            {
                break;
            }

            beta /=
                survival;
        }
    }

    return L;
}
```

There are convention-level details still to settle for exact delta-BSDF and refractive-$\eta^2$ weighting, but the **architecture** is now complete.

---

**Camera sample to film**

Outside the integrator:

```cpp
for pixel:
{
    PixelStatistics stats;

    for sampleIndex:
    {
        const Vec2 jitter =
            sampler.Pixel2D(
                pixel,
                sampleIndex);

        const Ray primary =
            camera.GenerateRay(
                (x + jitter.x) / width,
                (y + jitter.y) / height);

        const Color sample =
            TracePath(
                scene,
                primary,
                sampler);

        stats.AddSample(sample);
    }

    film[pixel] =
        stats.mean;
}
```

Everything is accumulated in:

$$
\boxed{\text{linear HDR}.}
$$

Only the final presentation path tone maps.

---

**Geometry fault isolation**

Suppose output is wrong.

First ask whether the rays hit the correct surfaces.

Render:

$$
\text{depth},
$$

$$
\text{primitive ID},
$$

$$
\text{barycentric},
$$

$$
\text{normal}.
$$

Run:

$$
BVH
$$

versus:

$$
\text{brute force}.
$$

If those differ, stop.

There is no point debugging GGX while geometry is incorrect.

---

**PDF fault isolation**

Suppose geometry is correct but brightness is unstable or biased.

Verify sampler distribution.

For hemisphere PDF:

$$
\int p(\omega)d\omega
\stackrel{?}{=}1.
$$

Histogram generated samples.

Check:

$$
p_{\text{returned}}
=
p_{\text{actual generator}}.
$$

For area light verify:

$$
p_\omega=
p_A\frac{r^2}{\cos\theta_L}.
$$

For mixtures verify:

$$
p=
\sum_i q_i p_i.
$$

For selected lights include:

$$
P(\text{light choice}).
$$

A PDF bug often causes:

- brightness scaling errors,
- fireflies,
- nonconvergence.

---

**BRDF fault isolation**

Use known directions.

Test:

$$
f\ge0.
$$

For reciprocal BRDF:

$$
f(\omega_i,\omega_o)
\approx
f(\omega_o,\omega_i).
$$

Numerically integrate hemispherical reflectance:

$$
\int
f(\omega_i,\omega_o)
\cos\theta_i
d\omega_i
\le1+\epsilon.
$$

Use reference spheres.

Use white-furnace tests.

If the BSDF itself is wrong, no sampler can repair it.

---

**White furnace test**

Place material inside a uniform unit-radiance environment:

$$
L_i=1.
$$

For a perfectly energy-conserving opaque material with no absorption in the considered model, outgoing result should not exceed expected energy bounds and should exhibit predictable behavior.

This is especially useful for:

- microfacet energy conservation,
- layered materials,
- transmission models.

If roughness changes cause unexplained brightness gain/loss, the furnace test exposes it.

---

**Visibility fault isolation**

Disable occlusion temporarily.

If direct lighting becomes correct, investigate shadow rays.

Check:

$$
t_{\min},
t_{\max},
$$

$$
\text{origin bias},
$$

$$
\text{any-hit BVH},
$$

$$
\text{light distance}.
$$

Compare:

$$
\operatorname{Occluded}_{BVH}
$$

against brute force.

Visibility bugs often look like BRDF bugs because they simply remove or add radiance.

---

**Accumulation fault isolation**

Render one deterministic sample and inspect it directly.

Then render the same sample through progressive accumulation.

They should agree.

Verify:

$$
\mu_n=
\frac1n
\sum X_i.
$$

Make sure tone mapping is not inside the accumulation loop.

Check sample count.

Check whether pass restarts accidentally reuse identical sample sequences.

Check for NaN/Inf.

A perfectly correct path contribution can be destroyed by incorrect accumulation.

---

**The complete diagnostic matrix**

| Failure domain | Diagnostic evidence | Typical symptom |
|---|---|---|
| Camera/geometry | depth, primitive ID, barycentrics | warped/missing geometry |
| BVH | acceleration-difference mode | random missing/incorrect hits |
| Normal | normal pass | inverted or discontinuous lighting |
| Material input | albedo/roughness/F0 pass | wrong colors/material classification |
| PDF | histogram + normalization + returned-PDF check | wrong brightness, fireflies |
| BRDF | reciprocity/energy/reference spheres | systematically wrong shading |
| Visibility | shadow brute force vs BVH | light leaks, acne, black patches |
| MIS | compare light-only, BSDF-only, MIS | energy loss/gain, strange noise |
| Medium state | visualize medium ID/IOR | wrong nested glass/TIR |
| RNG | seed/dimension visualization | stripes, repeating noise |
| Accumulation | sample vs running mean | brightness drift, nonconvergence |
| Tone mapping | inspect linear HDR first | highlight/color errors only in final output |
| Performance | rays, nodes, primitive tests, cache profile | correct image but slow |

That table is essentially the practical meaning of Part 2100.

---

**Kairo Phase-21 implementation audit**

Kairo already has a surprisingly strong foundation.

It has:

$$
\text{pinhole camera rays},
$$

$$
\text{sphere/triangle intersection},
$$

$$
\text{BVH + brute-force oracle},
$$

$$
\text{SAH and Morton modes},
$$

$$
\text{any-hit shadows},
$$

$$
\text{Whitted reflection/refraction},
$$

$$
\text{cosine path bouncing},
$$

$$
\text{Russian roulette},
$$

$$
\text{area-light PDF calculation},
$$

$$
\text{power heuristic helper},
$$

$$
\text{stratified deterministic pixel samples},
$$

$$
\text{multithreaded dynamic tile scheduling},
$$

$$
\text{linear HDR film},
$$

$$
\text{Radiance RGBE output},
$$

$$
\text{normal/albedo/depth/BVH/debug modes},
$$

$$
\text{deterministic regression tests}.
$$

But several things separate it from a fully physically based production path tracer.

The important gaps are not “write more ray tracing.”

They are architectural.

---

**Highest-value Kairo corrections from Phase 21**

The current `PathIntegrator` should stop treating every non-emissive material as Lambert.

Introduce a real BSDF interface.

Then integrate:

$$
\text{Mirror}
$$

as delta reflection,

$$
\text{Glass}
$$

as Fresnel-selected reflection/transmission,

$$
\text{PBR}
$$

as diffuse + GGX sampling.

Preserve geometric and shading normals separately.

Introduce medium-interface state.

Turn current one-sided MIS scaffold into symmetric NEE/BSDF MIS.

Turn area-light sampling into a properly normalized estimator rather than a set of point-light-like samples.

Add environment importance sampling.

Then fix path-ray accounting before benchmarking.

Those are the conceptual completion steps.

---

**Highest-value CPU-performance corrections**

Before exotic packet tracing, the current CPU BVH path has several likely low-hanging wins:

$$
\boxed{
\text{remove per-ray heap stack allocation}
}
$$

then:

$$
\boxed{
\text{remove unnecessary closest-hit vector accumulation}
}
$$

then:

$$
\boxed{
\text{avoid duplicate child AABB tests}
}
$$

then:

$$
\boxed{
\text{precompute invDir/sign}
}
$$

then evaluate:

$$
\text{compact node layout},
$$

$$
\text{BVH4/8},
$$

$$
\text{SIMD packet traversal}.
$$

Do not jump directly to GPU because CPU code is “too slow” if the CPU hot loop still allocates a vector per ray.

---

**Performance hierarchy**

A path tracer's runtime can be thought of approximately as:

$$
T
\approx
N_{\text{paths}}
\times
E[\text{bounces/path}]
\times
(
C_{\text{BVH}}
+
C_{\text{intersection}}
+
C_{\text{shading}}
+
C_{\text{sampling}}
).
$$

Sampling improvements reduce:

$$
N_{\text{paths}}
$$

needed for quality.

Russian roulette reduces:

$$
E[\text{bounces/path}].
$$

BVH improvements reduce:

$$
C_{\text{BVH}}.
$$

SIMD/GPU improves throughput of the inner operations.

Therefore rendering performance is not “optimize triangle intersection.”

It is a multiplicative systems problem.

---

**The Phase-21 mental model**

You should now see path tracing as three coupled machines.

The first is the **geometric query machine**:

$$
Ray
\rightarrow
BVH
\rightarrow
Hit.
$$

The second is the **probability machine**:

$$
Sample
\rightarrow
PDF
\rightarrow
BSDF/Light event
\rightarrow
MIS
\rightarrow
Throughput.
$$

The third is the **execution machine**:

$$
Millions\ of\ paths
\rightarrow
Threads/SIMD/GPU
\rightarrow
Film
\rightarrow
Statistics.
$$

A renderer is only correct and useful if all three agree.

---

**PASS 8 RECALL SHEET**

| Concept | Core reconstruction |
|---|---|
| Nested media | path carries current medium state |
| IOR transition | $\eta=\eta_i/\eta_t$ |
| Front face | derive from geometric normal |
| TIR | $\eta^2(1-\cos^2\theta_i)>1$ |
| Critical angle | $\arcsin(\eta_t/\eta_i)$ |
| Fresnel $F_0$ | $((\eta_i-\eta_t)/(\eta_i+\eta_t))^2$ |
| Glass selection | reflection $F$, transmission $1-F$ |
| Medium update | only after transmission |
| Colored RR | survival based on path throughput, often max RGB |
| RR compensation | $\beta\leftarrow\beta/p_s$ |
| Deterministic RNG | index by pixel/sample/dimension/seed |
| Correlation | uniform marginals can still produce bad joint samples |
| Stratification | one jittered sample per stratum |
| Low discrepancy | minimize holes/clumps in integration domain |
| Van der Corput | reversed radical digits |
| Sobol | dimension-addressable digital low-discrepancy sequence |
| Blue noise | suppress low-frequency spatial error |
| Tile rendering | contiguous image work units |
| Dynamic scheduling | workers atomically fetch next tile |
| False sharing | different variables on same cache line |
| Film | accumulate linear HDR samples |
| Precision | float sum can lose small increments at large magnitude |
| HDR output | preserve scene radiance before tone mapping |
| Tone mapping | exposure → nonlinear compression → display encoding |
| Normal pass | $0.5N+0.5$ |
| Depth pass | isolate camera/closest-hit geometry |
| BVH heatmap | visualize traversal cost |
| Determinism test | same seed/config → same samples |
| Reference image | numerical regression against trusted result |
| MC convergence | error approximately $N^{-1/2}$ |
| Rays/sec | rays divided by render seconds, but ray accounting must be complete |
| Packet tracing | SIMD multiple coherent rays |
| Wavefront | queue paths by processing stage |
| GPU megakernel | one thread owns entire path |
| GPU wavefront | separate intersection/shading/shadow queues |
| Path tracer | visibility + BSDF/light PDFs + MIS + throughput + media + accumulation |
| Fault isolation | geometry → PDF → BRDF → visibility → accumulation |

---

**Blank-page mastery gate**

From memory, start with a ray entering **glass while already underwater** and explain why:

$$
\eta_i\neq1.
$$

Track the medium state through:

$$
\text{air}
\rightarrow
\text{water}
\rightarrow
\text{glass}
\rightarrow
\text{water}.
$$

Then derive:

$$
\sin^2\theta_t
=
\left(
\frac{\eta_i}{\eta_t}
\right)^2
(
1-\cos^2\theta_i
)
$$

and identify exactly when TIR occurs and why the medium stack does not change.

Then prove colored Russian roulette preserves expectation.

Then explain why a renderer seeded by **thread ID** can change when thread scheduling changes, while Kairo's pixel/sample seeding is much more deterministic.

Then distinguish:

$$
\text{random},
$$

$$
\text{stratified},
$$

$$
\text{low discrepancy},
$$

$$
\text{blue noise}
$$

without calling them synonyms.

Then explain how two threads can corrupt performance while writing completely different variables due to cache-line false sharing.

Then explain why:

$$
\frac1N\sum T(X_i)
$$

is not equal to:

$$
T\left(
\frac1N\sum X_i
\right).
$$

Then design a deterministic convergence test and explain why a stochastic 64-SPP result need not be strictly closer to reference than 16 SPP on every single run.

Then inspect Kairo's current RPS statistic and recognize why Path mode undercounts ordinary bounce rays.

Then explain why per-ray `std::vector` allocation in BVH traversal is a larger immediate optimization target than jumping straight to GPU ray tracing.

Finally, reconstruct a complete camera-to-light path:

$$
\text{pixel sample}
\rightarrow
\text{camera ray}
\rightarrow
\text{BVH}
\rightarrow
\text{surface}
\rightarrow
\text{emission/NEE}
\rightarrow
\text{BSDF sample}
\rightarrow
\frac{f\cos}{p}
\rightarrow
\text{MIS}
\rightarrow
\text{medium transition}
\rightarrow
\text{roulette}
\rightarrow
\text{next bounce}
\rightarrow
\text{linear film}
\rightarrow
\text{HDR/tone mapping}.
$$

If you can reconstruct that chain and identify where every PDF, visibility term, medium transition and probability entered, then you understand **path tracing as an engine system**, not merely as recursive ray code.


**Phase 21 is now complete: M121–M126, Parts 2001–2100.**

### Next — Pass 9: M127 + M128 + M129, Parts 2101–2156

That begins **Phase 22 — GPU Architecture, SIMT Execution, Memory Hierarchy, Compute, and Parallel GPU Algorithms**: CPU latency architecture versus GPU throughput architecture, SMs/CUs, warps/wavefronts/SIMT execution, occupancy, registers and spilling, shared/threadgroup memory, coalescing, cache hierarchy, divergence and reconvergence, dispatch dimensions, synchronization/barriers/atomics, prefix scans/reductions/histograms/compaction, and the mapping of graphics/compute workloads onto actual GPU execution hardware.


Renderers evaluate light transport by tracing how emission and reflection contribute along paths to an image.

### Rendering equation and numerical integration

At surface point $x$, outgoing radiance in direction $\omega_o$ consists of emitted light and reflected incoming light:

$$
L_o(x,\omega_o)=L_e(x,\omega_o)+\int_\Omega f_r(x,\omega_i,\omega_o)L_i(x,\omega_i)|n\cdot\omega_i|\,d\omega_i.
$$

$L_e$ is emitted radiance, $L_i$ incoming radiance, $f_r$ the BRDF, $n$ the normal, and $\Omega$ the hemisphere above the surface. The cosine accounts for projected area. Visibility determines whether geometry blocks a sampled direction. Incoming radiance depends on radiance leaving other surfaces, so the equation is recursive. Expanding it produces paths of light bounces: direct light reaches a surface then camera; indirect light includes more interactions. The transport equation remains the same for every material; material response changes through the BRDF. Fresnel reflection makes many dielectrics more reflective at grazing angles. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]]

An integral accumulates a continuous range of contributions. Midpoint quadrature evaluates the center of each interval; trapezoidal quadrature joins endpoint samples; Simpson’s rule fits a quadratic. These deterministic rules work well for smooth low-dimensional functions. Visibility boundaries create discontinuities, and multidimensional light paths become expensive for tensor grids: $m$ samples per dimension require $m^d$ evaluations in dimension $d$. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-17 - Numerical Integration|CMU-17]]

Monte Carlo integration uses random samples. If $X$ is drawn from density $p(x)$, then

$$
\int f(x)\,dx=\mathbb E\left[\frac{f(X)}{p(X)}\right]\approx\frac1N\sum_{i=1}^N\frac{f(X_i)}{p(X_i)}.
$$

The weighting follows from expectation: $\mathbb E[f(X)/p(X)]=\int (f(x)/p(x))p(x)dx=\int f(x)dx$. Density must be positive wherever $f$ contributes and measured in the same domain as the integral. The estimator is unbiased, but random error remains. For independent samples $Y_i=f(X_i)/p(X_i)$, variance of the mean is $\operatorname{Var}(Y)/N$, so standard deviation decreases as $1/\sqrt N$. Four times as many samples roughly halves noise. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-17 - Numerical Integration|CMU-17]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

For a small numerical trace, integrate $f(x)=x^2$ over the unit interval; the exact answer is $1/3$. Uniform samples $0.2,0.5,0.9$ produce mean square $(0.04+0.25+0.81)/3=0.3667$. The discrepancy is sampling error. A proposal density $p(x)$ that resembles the integrand makes weighted samples less variable. For $f(x)=2x$, the exact integral is 1. Uniform sampling has contribution $2X$ and variance $1/3$. Sampling instead from $p(x)=2x$ makes $f(X)/p(X)=1$ for almost every sample, yielding zero variance. Real rendering integrals are more complex, but the example explains [[Monte Carlo rendering estimators and variance|importance sampling]]. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### From the rendering equation to paths

Use operator notation $L=L_e+KL$, where $K$ represents one reflection-and-visibility step. Substitution yields $L=L_e+KL_e+K^2L_e+K^3L_e+\cdots$. The term $L_e$ is direct emission at the point. $KL_e$ includes paths that reflect once before reaching emission. $K^2L_e$ includes two reflections, and so on. Path tracing estimates this infinite series by following random directions and accumulating weighted contributions. Absorption and energy loss usually make long-path contributions diminish, but some environments preserve light for many bounces.

Direct-light sampling selects a source and tests visibility to it. If the source is small, sampling a random hemisphere direction may almost never hit it. Conversely, sampling only the light may miss glossy paths where material response concentrates energy. The path formulation clarifies that the challenge is to place probability mass on path configurations that carry large contribution. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### One-bounce Monte Carlo estimator

For a surface point, estimate reflected radiance by sampling directions $\omega_i$ from density $p(\omega_i)$ and evaluating the BRDF, incoming radiance, and cosine. For $N$ samples, the estimate is

$$
\widehat L_o=L_e+\frac1N\sum_{k=1}^N\frac{f_r(x,\omega_k,\omega_o)L_i(x,\omega_k)|n\cdot\omega_k|}{p(\omega_k)}.
$$

Each symbol has a specific role: $L_e$ emits from the surface; $f_r$ redirects incoming light; $L_i$ is the radiance arriving from the sampled direction; $n$ is the surface normal; $p$ is the directional sampling density; and the cosine is projected area. Visibility is included in $L_i$ by making it zero when geometry blocks that direction. If the PDF is expressed per unit area rather than per unit solid angle, it must first be converted. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

The estimator is an expectation written as a sample average. If directions are drawn uniformly over a hemisphere, the probability density per unit solid angle is $1/(2\pi)$, because the hemisphere has solid angle $2\pi$. Substituting that density into the one-sample contribution multiplies the integrand by $2\pi$. The cosine still matters: a ray arriving nearly tangent to a surface contributes little projected flux, while a ray arriving near the normal contributes more. This gives a useful importance distribution. Since $\int_{H^2}\cos\theta\,d\omega=\pi$, the density $p(\omega)=\cos\theta/\pi$ is normalized. Sampling with this density cancels the cosine in the estimator, leaving $\pi L_i$ for each sampled direction. The code becomes simpler and, when incident radiance is roughly constant, the samples are allocated in proportion to the part of the integrand that matters. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

Another proposal samples a finite area light directly. Let $x$ be the shaded point and $y$ a point on the emitter. Write $r=\|y-x\|$ for their distance, and let $\theta_y$ be the angle between the emitter's normal and the direction from $y$ toward $x$. A patch of emitter area $dA_y$ subtends solid angle

$$
d\omega = \frac{\cos\theta_y}{r^2}\,dA_y.
$$

This is a [[Probability density and change of measure|change of variables]]: the same visible set of light samples is being described on the emitter surface instead of as directions at $x$. Therefore the integrand must acquire the Jacobian $\cos\theta_y/r^2$. The surface's own projected-area cosine at $x$ remains as well, along with visibility, which is zero when an occluder blocks the segment. With uniform area samples, the estimator multiplies the average by the emitter area. This avoids wasting most rays on empty sky when the emitter subtends only a small part of the hemisphere. In the lecture's square-light / blocker / ground-plane comparison, 100 uniform hemisphere samples are grainy, while 100 area-light samples produce a much smoother shadow and illumination field; the area-light result remains strong at 16 samples, and even its one-sample result compares favorably to uniform sampling. The lesson is about variance per unit computation, not a guarantee that every single random estimate will look clean. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

The path version multiplies a throughput term by the sampled BRDF, cosine, and inverse density at each bounce. A path contribution might be written $C= L_e\prod_{j=1}^b f_j|n_j\cdot\omega_j|/p_j$. Here $b$ is bounce count, and each factor belongs to one interaction. Long paths can have very small throughput; Russian roulette terminates them stochastically and compensates survivors by dividing by continuation probability. This is an estimator design choice, not a physical event that must occur in the scene. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

Why does path tracing recurse? At one surface, the rendering equation asks for incoming radiance from every direction. A ray in one sampled direction either reaches an emitter or hits another surface; in the second case, the next surface has the same integral to evaluate. Thus one random path is a chain of surface interactions from the camera toward an emitter. Even a single path estimate has the correct expected value when every sampled direction is weighted by its probability density, but individual paths are noisy; averaging many independent paths reduces variance. Limiting paths to direct illumination gives hard, dark regions wherever a surface receives no direct source. Adding indirect bounces fills some of those regions. Longer paths matter selectively: a glass lantern may look black if paths stop before the sequence of reflection and refraction events that transports light through it, while four-bounce paths can reveal its glass structure. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

Russian roulette makes path termination probabilistic. Suppose a candidate path has contribution $C$ and the renderer keeps it with probability $q$. On a surviving path, use $C/q$; on a terminated path, use zero. The expected result is

$$
q\frac{C}{q}+(1-q)0=C.
$$

The estimate therefore remains unbiased even though some paths are skipped. A deterministic rule such as “discard every contribution below a threshold” would systematically remove dim light and bias the answer. In the lecture's example, more aggressive termination shortens rendering time while making shadows noisier; the useful setting depends on how much remaining throughput the scene carries and how much variance the renderer can tolerate. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

### Bidirectional path construction

Camera-only path tracing launches a path from the camera and samples successive surface directions. This is effective when important paths are easy to find from the camera side. It struggles with difficult light connections, such as a small opening that admits illumination into a room or a caustic where light focuses through specular events. Bidirectional path tracing also launches subpaths from emitters and connects light-side and camera-side vertices. It can discover a connection even when neither a camera-only nor a light-only random walk would likely create the entire path.

The estimator must account for the probability of generating a path by every strategy that could have produced it. Multiple importance weights compare those path densities so the same contribution is not overcounted. Path length is itself a sampling decision: short paths often represent direct illumination; longer ones capture more indirect transport but may have diminishing contribution and higher cost. Russian roulette can end long low-throughput paths without a fixed bounce cutoff. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### Metropolis light transport as a path-space walk

Metropolis-Hastings sampling creates a Markov chain whose states are light paths. From current path $x$, propose new path $y$ with transition density $T(y|x)$ and accept it with probability

$$
\alpha(x,y)=\min\left(1,\frac{\pi(y)T(x|y)}{\pi(x)T(y|x)}\right),
$$

where $\pi$ is the target path-contribution distribution. The ratio corrects for asymmetric proposal probabilities. Rejected proposals keep the current path, maintaining the desired stationary distribution. In rendering, path mutations can explore regions where image contribution is large, reusing promising paths rather than restarting from scratch. This can help with rare, high-energy transport, but chains need sufficient exploration and careful normalization; correlation among samples changes variance behavior. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### Photon mapping and radiosity

Photon mapping traces particles of light from sources and stores where they hit surfaces. At a shading point, nearby photon contributions estimate incoming illumination. The estimate is a density estimate: using too large a neighborhood blurs detail, while too small a neighborhood has high variance. Photon maps can represent caustics that camera-side random paths rarely find. They are often biased at finite neighborhood radius, with bias decreasing as the estimate is refined.

Radiosity divides diffuse surfaces into patches and solves for energy exchange among them. A form factor describes what fraction of energy leaving one patch reaches another. The discrete system couples all patches; dense systems can be expensive, while spatial hierarchy or iterative methods reduce work. Radiosity assumes diffuse response and typically solves a global energy balance, whereas path tracing samples individual transport paths. These approaches target related light transport from different numerical angles. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-19 - Variance Reduction|CMU-19]]

### Recursive light paths: shadows, mirrors, glass, and sampled effects

A shadow query asks whether any blocker lies between a surface point and a light. This is a binary visibility test, so a shadow ray can stop at its first valid hit instead of finding the closest blocker. The local illumination may still contain an ambient term, but ambient light in this basic model is a constant hack that prevents completely black unlit regions; it is not a simulation of bounced light. A point light yields hard shadow boundaries. An area light is a continuum of positions, each with a different visibility result; sampling $M$ positions and averaging estimates the lit fraction. Fully blocked samples form the umbra, fully visible samples the lit region, and a mixture forms the penumbra. If a shadow ray starts exactly at the hit, floating-point error may report the surface itself as a blocker, creating self-shadow speckles. Start at $p+\epsilon n$ or require a candidate parameter $t>\epsilon$, with $\epsilon$ chosen relative to scene scale. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (00:07:40–00:20:20)

For an ideal mirror, reflect the incoming direction across the surface normal. Given a unit normal $n$ and incident unit direction $d$ pointing toward the surface, the outgoing direction is

$$
r=d-2(d\cdot n)n.
$$

The formula subtracts twice the normal projection, flipping its sign while leaving the tangential component unchanged. A secondary ray from the hit point travels along $r$; its returned radiance contributes to the original hit color. This is recursive: the reflected ray may strike another mirror and launch another ray. Real surfaces are imperfect. Fresnel reflection describes the increase in reflectance at grazing angles, where a surface looks more mirror-like. Schlick's inexpensive approximation is $F(\theta)=F_0+(1-F_0)(1-\cos\theta)^5$, where $F_0$ is the normal-incidence reflectance. It retains the angle trend with only a dot product and a fifth power. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (00:20:20–00:30:20)

Refraction changes direction across an interface because light travels at different speeds in different materials. Snell's law is $n_i\sin\theta_i=n_t\sin\theta_t$, where $n_i,n_t$ are refractive indices and angles are measured relative to the normal. To compute the new vector without inverse trigonometric functions, take unit incident direction $i$ pointing toward the viewer in the lecture's convention and a suitably oriented unit normal $n$. Define $\eta=n_i/n_t$ and $c=i\cdot n$. The transmitted direction can be written

$$
t=-\eta i+\left(\eta c-\sqrt{1-\eta^2(1-c^2)}\right)n.
$$

The tangential component is scaled according to Snell's law; the normal component is chosen to keep $\|t\|=1$ and point into the second medium. The sign convention for $i$ and $n$ must be consistent—many implementations use an incident vector pointing toward the surface and flip the normal when exiting. As a worked case, for air to glass with $\eta=1/1.5$ and incidence angle $30^\circ$, Snell gives $\sin\theta_t=(1/1.5)\cdot0.5=1/3$, so $\theta_t\approx19.47^\circ$: the ray bends toward the normal. From glass to air at a sufficiently grazing angle, the square-root radicand becomes negative; there is no real transmitted direction, and the event is total internal reflection. Fiber optics use repeated internal reflections to guide light. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (00:30:20–00:54:55)

Reflection and refraction are only some of the possible branches. Rainbows require wavelength-dependent refraction plus internal reflection in water droplets; tracing only one RGB ray cannot reproduce the full dispersion without modeling several wavelengths. Caustics—bright focused patterns from curved reflective or refractive surfaces—are hard for camera-launched paths because the important light paths occupy a small set of directions. The lecture also uses ray tracing as a forward model for lens design and caustic shaping: alter a glass surface, trace rays through it, and optimize the resulting pattern. These effects distinguish recursive specular paths from general global illumination. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (00:54:55–01:03:20)

Sampling adds realistic variation and computational cost. Perturbing a perfect mirror ray can approximate a rough or brushed surface, but a single independent random direction per pixel appears as white noise. Repeating samples and averaging reduces variance. Choosing random points on an area light estimates soft shadows; choosing positions in a lens aperture gives depth of field; sampling scene time gives motion blur; multiple samples within a pixel reduce jagged edges. If each branch creates $b$ new rays to depth $k$, a full ray tree may contain $1+b+b^2+\dots+b^k$ rays, so costs grow rapidly. Renderers cap recursion depth or terminate branches whose remaining energy is negligible. This still leaves unequal work across pixels: a small background ray may hit many detailed objects while another hits a simple surface, so static allocation can leave processors idle. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-11 - Ray tracing|MITG-11]] (01:03:20–01:19:04)

### Global illumination solves a coupled light transport problem

Direct lighting accounts for light that reaches a surface from explicit sources. It misses the red tint that a red wall contributes to a nearby floor, soft indirect fill, and many caustics. Backward ray tracing starts at the eye because rays that reach the eye are the ones worth computing, but physical light travels from source to eye. Reversing path direction is valid for ordinary reciprocal reflection; it does not mean the algorithm has simulated every light path. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (00:04:49–00:16:10)

Using the rendering equation defined earlier, outgoing radiance at a surface point depends on emitted radiance and the visible hemisphere of incoming reflected radiance. Its terms are the BRDF, incoming radiance, surface normal, and projected-area cosine. Because incoming radiance itself comes from other surfaces’ outgoing radiance, this integral couples positions and directions throughout the scene. A finite approximation can be solved as a large linear system (radiosity is one family), but general high-dimensional scenes make this expensive. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (00:16:12–00:26:33)

Monte Carlo rendering estimates the integral by random samples. If directions $\omega_j$ are drawn from a probability density $p(\omega)$, an unbiased estimate is

$$
\widehat L_o=L_e+\frac1N\sum_{j=1}^{N}\frac{f_r(x,\omega_j,\omega_o)L_i(x,\omega_j)\max(0,n\cdot\omega_j)}{p(\omega_j)}.
$$

The division by $p$ compensates for where samples are concentrated: sampling a region twice as often must not make that region count twice as much. Direct light sampling is usually added explicitly because a small, bright source is unlikely to be hit by a random hemisphere direction. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (00:26:50–00:31:34; 01:16:00–01:19:14)

A naïve recursive integrator takes many samples at each bounce, so work grows exponentially: 100 directions at each of two bounces means roughly $100^2$ paths. Path tracing takes one sampled continuation at each hit, then averages many independent paths per pixel. Each path is noisy, but cost grows approximately linearly with path count rather than with a branch factor at every bounce. With $N$ independent samples, Monte Carlo standard error typically decreases as $1/\sqrt{N}$; four times as many samples roughly halves noise. Bright, rare paths cause shot noise, which is especially visible around glossy reflections or small sources. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (00:27:02–00:43:35)

Irradiance caching and photon mapping reuse work with different coordinates. Irradiance caching stores expensive indirect-light estimates at selected image pixels and interpolates nearby values, while checking that hit geometry and normals are similar. It adds samples around image-space edges where indirect illumination changes quickly. Its error can show the cache spacing in smooth regions. Photon mapping first emits photons forward from light sources, storing position and energy at surface hits; a camera pass then gathers nearby photon records. This puts reusable samples in scene space instead of current-view image space. Neither method is exact: each trades time for blur, density artifacts, or biased reconstruction. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (00:43:35–01:00:00)

Subsurface scattering requires a broader material response than a BRDF. A BSSRDF relates incoming light at one surface point and direction to outgoing light at another point and direction. This models light entering skin or wax and emerging nearby, so the path can move laterally inside the material. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (01:00:00–01:02:57)

Sampling plans matter because the integration domain is high dimensional: each bounce adds more coordinates. A regular grid can be effective in low dimensions, but grid cost scales like $m^d$ for $m$ samples per axis in dimension $d$, and regular patterns can alias against scene structure. Random Monte Carlo error has a weaker dimension dependence but converges slowly. Importance sampling chooses a density proportional to expected contribution—such as the BRDF lobe—then divides by that density to remain unbiased. A bright source outside the chosen lobe can still be missed, so hybrid samplers combine BRDF sampling with direct-light samples or broader exploration. Stratification divides the domain into cells and jitters one sample per cell to avoid accidentally leaving large regions empty. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-16 - Global illumination|MITG-16]] (01:03:01–01:22:37)

### Additional geometry notes for cameras

These lecture examples are integrated by topic. The 2009 interface material below is explicitly historical; it is not a modern OpenGL core recipe.

#### Rotate around an arbitrary line by aligning it to a known axis

A 3D rotation needs an axis, which is a line specified by a point $P=(x_p,y_p,z_p)$ and direction $V=(x_v,y_v,z_v)$. The coordinate-axis rotation matrices from P3-03 are easy to construct, so transform the arbitrary axis until it coincides with the z axis, rotate by the desired angle $\alpha$, then undo those alignment transforms. For row vectors, the sequence is

$$
M=T(-P)\,R_y(-\theta)\,R_x(\phi)\,R_z(\alpha)\,R_x(-\phi)\,R_y(\theta)\,T(P).
$$

A row point is multiplied on the right: $Q'=QM$. The rightmost operation in this written row-vector product happens last. Reading from the left, the point is first translated so the axis passes through the origin. Then a y rotation removes the direction's x component, placing the axis in the yz plane. An x rotation removes its y component, aligning it with +z. After $R_z(\alpha)$, reverse the x and y alignments and translate the axis back. Seven 4×4 matrices can be composed into one matrix that performs the rotation. This is an example of using simple transformations as building blocks for a less obvious one. (P3-04)

For direction $V$, a robust choice of alignment angles is

$$
\theta=\operatorname{atan2}(x_v,z_v),\qquad
\phi=\operatorname{atan2}\left(y_v,\sqrt{x_v^2+z_v^2}\right).
$$

The lecture derives these angles from right triangles and corrects its diagram/sign setup while teaching. `atan2` preserves quadrant information more reliably than a one-argument inverse tangent. If the axis is already parallel to y, the first angle is unnecessary; if the direction is zero, it does not define an axis at all. These are useful implementation edge cases beyond the chalkboard walkthrough. (P3-04)

Example: let the axis pass through $P=(1,2,0)$ in direction $V=(0,1,1)$, and rotate $Q=(2,2,0)$ by $90^\circ$. Here $\theta=0$ and $\phi=45^\circ$. Translate $Q$ relative to the axis to get $(1,0,0)$. The x alignment leaves that point unchanged; the z-axis quarter-turn changes it to $(0,1,0)$. Undoing the x alignment gives $(0,\frac{1}{\sqrt2},-\frac{1}{\sqrt2})$. Translating back yields

$$
Q'=(1,2+\tfrac{1}{\sqrt2},-\tfrac{1}{\sqrt2})\approx(1,2.707,-0.707).
$$

A direct right-hand check gives the same result: the unit axis is $(0,1/\sqrt2,1/\sqrt2)$, and rotating the offset $(1,0,0)$ by $90^\circ$ adds its cross product $(0,1/\sqrt2,-1/\sqrt2)$. This independently checks the sequence and sign convention. (P3-04; numeric example derived from the source construction.)

#### Color and the simplified local-lighting model

A physical surface does not simply “have” a color independent of illumination. Light has a spectrum; a material absorbs and reflects portions of it; the eye integrates the resulting spectrum. The course reduces this continuous process to three RGB channels because it is practical for graphics hardware. In normalized RGB, each channel ranges from 0 to 1: red is $(1,0,0)$, green $(0,1,0)$, blue $(0,0,1)$, white $(1,1,1)$, black $(0,0,0)$. Equal channel values lie along the gray diagonal. An 8-bit-per-channel buffer stores $256^3=2^{24}$ RGB combinations. RGB is additive light: red plus green light can appear yellow. Pigments and printing are better described with subtractive CMY/CMYK, so mixing crayon colors is not the same operation as adding display light. HLS/HSL and HSV-like models rearrange color coordinates into hue and intensity/saturation controls that are easier for a person to choose. HSL lightness and HSV value are not the same coordinate, despite the lecture’s shorthand. (P3-08)

The full light-transport problem contains repeated bounces: a wall illuminates a table, the table illuminates another wall, and those contributions continue. Physically based global illumination can approximate this energy exchange, but the introductory fixed-function model makes a deliberately simple local approximation. It treats background illumination as a constant ambient term, a point light as a source with a position and RGB color, and a material as a set of per-channel response coefficients. This is a teaching model, not an accurate description of metals, glass, or all real surfaces. (P3-08)

For diffuse reflection, let $N$ be a unit surface normal and $L$ a unit direction from the surface toward the light. Lambert’s cosine law gives the orientation factor $N\cdot L=\cos\theta$. When the light grazes the surface, the cosine approaches zero; when it faces the normal, it is one. The useful rendering form clamps the back-facing case: $D=\max(0,N\cdot L)$. If $N=(0,1,0)$ and $L=(0.6,0.8,0)$, then $D=0.8$. A red diffuse material with coefficient $k_d=0.6$ under unit red light contributes $1\times0.6\times0.8=0.48$ red before ambient and specular terms. A unit normal matters: otherwise the dot product also scales by vector lengths and ceases to be the cosine. Polygon normals can be found from the cross product of two edges; reverse edge order reverses the normal. (P3-08)

Ambient light is a shortcut for many indirect bounces. It brightens surfaces even when the explicit point light does not reach them, but because it is constant it cannot create realistic spatially varying indirect illumination or cast shadows. The lecture’s per-channel local model can be written in an explanatory normalized form:

$$
C_{out}=k_a\odot C_{ambient}+k_d\odot C_{light}\,\max(0,N\cdot L)+k_s\odot C_{light}\,\max(0,N\cdot H)^m.
$$

Here each RGB triple is evaluated channel by channel, $\odot$ means component-wise multiplication, $k_a,k_d,k_s$ are material ambient/diffuse/specular responses, and $C_{light}$ colors the direct light. The first term is constant ambient, the second is Lambertian diffuse, and the third makes a view-dependent highlight. Clamp the final channels to the supported range. Exact coefficient names and multiplication order vary across legacy APIs; the equation captures the lecture’s simplified model. (P3-08)

For a shiny highlight, classical Phong compares the view direction $V$ with the ideal reflected direction $R$ and raises their cosine to a gloss exponent. The power makes the peak narrow: for $0\le c\le1$, $c^{100}$ drops far more quickly away from $c=1$ than $c^5$. Blinn’s practical variant uses the halfway direction $H=\operatorname{normalize}(L+V)$ and the factor $\max(0,N\cdot H)^m$. The halfway vector is easy to compute and yields the Blinn–Phong model used in many introductory renderers. A larger exponent $m$ narrows the highlight and makes the object look glossier; it does not mean the material is physically more reflective at every angle. The highlight is tinted by the light and scaled by a specular material coefficient. (P3-08)

This model explains why a plastic toy can look convincing but a metal or dielectric is harder: it compresses complex microfacet behavior into a few tunable values. The instructor maps those values to creative controls. A texture changes surface color; bump mapping perturbs the normal used by the lighting calculation, changing apparent small-scale relief without changing the silhouette; moving the light changes $L$ and highlight position; changing $m$ alters gloss. The orange example makes the normal role intuitive: perturb normals over a smooth orange-colored sphere and the light response starts to resemble textured peel. (P3-08)

#### Shadow representations: geometry, volumes, and depth maps

A shadow is a visibility question from the light. A surface point is directly lit if the segment from that point toward a light source is unobstructed. Three representation choices in the 2009 lectures answer that question in different spaces: project a fake shadow polygon onto a floor, construct a 3D shadow volume and count crossings, or render a depth map from the light and compare depths. (P3-18, P3-19)

For a point light $L$, object point $P$, and receiver plane through $Q$ with normal $N$, the ray from the light through the object is $X(t)=L+t(P-L)$. A projected shadow point lies on the plane, so

$$
N\cdot(X(t)-Q)=0,\qquad t=\frac{N\cdot(Q-L)}{N\cdot(P-L)}.
$$

Then $X=L+t(P-L)$. If the denominator is zero, the line is parallel to the plane (or lies wholly in it), so there is no unique projected point. For a simple numeric check, let the floor be $z=0$, light $L=(0,0,4)$, and object point $P=(1,0,2)$. Along the ray the z coordinate is $4+t(2-4)=4-2t$. Setting it to zero gives $t=2$, so the projected point is $(2,0,0)$, farther from the light’s vertical axis than the object because the light ray diverges. The projected polygon can be drawn dark and offset slightly from the floor to avoid z-fighting. It is efficient and useful for contact shadows, but it cannot cast a shadow over a curved body or arbitrary receiver because its construction assumes one plane. A fuzzy texture or an ellipse under a character is a deliberate approximation, not a visibility solution. (P3-18)

A shadow volume is the region behind a caster as seen from a light. For a polygonal caster, extrude its silhouette away from the light and close the resulting shape. To test a camera ray, count each entry into the volume as +1 and each exit as −1. If the ray reaches the visible surface with count zero, it is outside all volumes and lit; a positive count means it is inside at least one shadow. The crossing sign can be determined from the ray direction’s dot product with the volume face normal. This supports arbitrary receiving geometry, but every caster polygon may create one or more extra faces. Overlapping volumes lead to many crossings, so the stencil/counting method may be manageable while volume construction, storage and fill work become expensive. (P3-18)

An area light has extent. A point in the umbra is blocked from every part of the source; a point in the penumbra sees some of it. A single point-light projection cannot express that gradual transition. Repeated or layered shadow volumes can approximate partial coverage, but at a high geometry cost. This explains why shadows are a balance between accurate visibility and practical sampling or storage. (P3-18)

Shadow mapping stores the closest depth visible from a light in a depth texture. During the eye-view pass, transform each visible point into the light’s clip space, divide by its homogeneous $w$, map x/y into the light’s texture coordinates and compare its light-space depth with the stored depth at that location. If the point is farther than the stored blocker, it is in shadow. If it is nearer or equal (within a bias), the light is visible. For example, if the map stores depth $0.42$ and the receiver’s projected depth is $0.67$, the receiver lies behind the surface seen by the light and is shadowed. With depth increasing away from the light, “farther” is a larger value; exact comparisons depend on the depth convention. A small depth bias avoids self-shadowing caused by quantization and interpolation, but too much bias detaches shadows from casters. (P3-18, P3-19)

The lecture’s 2009 derivation walks between eye and light image spaces with inverse view/projection matrices. It emphasizes retaining homogeneous $w$: once a clip point is divided by $w$, that scale is lost and cannot simply be inverted to recover the same homogeneous point. Reconstruct in homogeneous coordinates, transform back through inverse projection and view, then project into the other camera’s space. This makes the coordinate relationship explicit; a typical modern shadow-map shader instead computes light-space coordinates directly from the world-space position before the eye-space projection. (P3-19)

Shadow maps have two coupled resolution limits. A narrow light frustum clips casters or receivers outside its stored view; a very wide frustum spreads a fixed number of texels across more world area, so a single texel covers a larger region and shadows look blocky. Higher resolution helps but costs memory and rendering work. Several lights generally need several maps. Point lights may need multiple views around the source, while a directional light can use an orthographic view; these choices constrain coverage. A point mapped between texel centers also creates aliasing. Percentage-closer filtering (PCF) compares a neighborhood of depth samples and averages the visibility results. A 4×4 filter averages sixteen binary comparisons; if four samples are lit, the fractional result is 4/16. This softens jagged boundaries and can suggest an area light, but ordinary PCF blurs the visibility edge rather than physically integrating all directions from a source. (P3-19)

**OpenGL era note.** The 2009 lecturer speaks of an OpenGL function for shadow maps and discusses the fixed-function transform pipeline. That is historical context. In modern programmable OpenGL, a common route is to render a depth texture from the light, pass light-space coordinates to a shader, and use a GLSL shadow sampler with depth comparison; the official [OpenGL Shading Language 4.60 specification](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) defines these shadow sampler comparisons. Thus the geometric algorithm remains relevant while the API path has changed. (P3-19)

#### Source video index

The transcript references use these playlist positions and exact video URLs.

| Source ID | Video |
|---|---|
| P3-01 | [Introduction to Computer Graphics](https://www.youtube.com/watch?v=01YSK5gIEYQ) |
| P3-02 | [Curves in the Plane](https://www.youtube.com/watch?v=0NbD-c0Ctdk) |
| P3-03 | [Moving Objects in Space](https://www.youtube.com/watch?v=wArGifkRD2A) |
| P3-04 | [Rotations About an Arbitrary Axis](https://www.youtube.com/watch?v=gRVxv8kWl0Q) |
| P3-05 | [The Camera Transform](https://www.youtube.com/watch?v=mpTl003EXCY) |
| P3-06 | [Clipping](https://www.youtube.com/watch?v=og7hOFypKpQ) |
| P3-07 | [Depth Buffers and Ray Tracing](https://www.youtube.com/watch?v=Xks1v4GNUiY) |
| P3-08 | [Color and Shading](https://www.youtube.com/watch?v=TEjDYtkLRdQ) |
| P3-09 | [Quaternions](https://www.youtube.com/watch?v=mHVwd8gYLnI) |
| P3-10 | [Shading and Texturing](https://www.youtube.com/watch?v=Is6D5rnWEvs) |
| P3-11 | [The Hierarchical Modeling System](https://www.youtube.com/watch?v=D8ZYHXom0qk) |
| P3-12 | [Scan Conversion](https://www.youtube.com/watch?v=23HEwdcphg4) |
| P3-13 | [Curves and Surfaces](https://www.youtube.com/watch?v=FTg1DUr7bhY) |
| P3-14 | [Visible Surface Algorithms](https://www.youtube.com/watch?v=hmlWFi1TdbI) |
| P3-15 | [Ray Tracing](https://www.youtube.com/watch?v=Ahp6LDQnK4Y) |
| P3-16 | [Subdivision Surfaces Part 1](https://www.youtube.com/watch?v=9uscFr2Hht0) |
| P3-17 | [Subdivision Surfaces Part 2](https://www.youtube.com/watch?v=THiF7-QxKXk) |
| P3-18 | [Shadows Part 1](https://www.youtube.com/watch?v=kmdRmXuOjIY) |
| P3-19 | [Shadows Part 2](https://www.youtube.com/watch?v=qiJhB0fOY6I) |
| P3-20 | [Camera Motion](https://www.youtube.com/watch?v=HAJv25Afsz8) |
| P3-21 | [Branches in Computer Graphics](https://www.youtube.com/watch?v=NEIJW3ZJ1GE) |

### A programmable, differentiable rendering system

*Connect to:* Monte Carlo rendering; optimization; inverse rendering; physical light transport; CPU/GPU execution.

Mitsuba 2 is a renderer-system paper, not a new surface representation. Its central design problem is how to add different execution modes—scalar, vectorized, GPU, spectral, polarized, and differentiable—without maintaining separate copies of every sampling and shading algorithm. The authors build generic renderer code on Enoki, a C++ array and program-transformation library. A developer writes operations on array-like values; compile-time types choose how those values are represented and which transformations apply. The same high-level algorithm can then be retargeted to scalar values, CPU packets, or GPU arrays.

On the GPU, ordinary eager execution would launch a kernel for each small arithmetic operation. A path tracer performs many simple operations between irregular scene queries, so launch and memory-transfer overhead can dominate. Enoki instead records operations symbolically in a single-static-assignment graph. Reading or otherwise observing the array triggers just-in-time (JIT) compilation of the accumulated graph into a GPU kernel. The first rendering may pay the compilation cost; identical kernels can be cached for later iterations. The approach fuses small operations while allowing a large renderer to build its graph at runtime, without describing the entire codebase in advance.

A renderer's control flow is irregular. Different rays hit different materials and take different branches. Enoki uses **masks** to mark which lanes of a vector are active. A vectorized branch computes only for lanes that satisfy its condition. To avoid synchronizing the GPU for every step, code should keep many operations vertical (per-lane) and use horizontal reductions such as `all(mask)` sparingly. Dynamic method dispatch is more difficult because a vector of rays may point to many material objects. Enoki groups lanes by unique object pointer, calls that material's method for its active subset, then scatters results back. Gather and scatter also need special derivative rules: in reverse mode, a gather's gradient becomes a scatter-add, while a scatter's gradient becomes a gather.

Differentiable rendering creates a second systems problem: reverse-mode [[Automatic differentiation and backpropagation|automatic differentiation]] (AD) must retain intermediate values until the output loss is known. A wavefront renderer can store large intersection records and sampled values; at image-scale sample counts, the memory cost can reach multiple gigabytes. Enoki simplifies its computation graph at JIT boundaries. It collapses interior nodes by multiplying derivative weights along paths, trading extra arithmetic for fewer stored nodes. A greedy rule collapses low-cost nodes first. This makes the method a hybrid of forward and reverse differentiation rather than a strict traversal of the original graph. The authors' Cornell Box measurement shows JIT alone lowers runtime substantially; graph simplification further reduces memory from 2.43 GiB to 1.36 GiB in the reported setting, with a small timing change. This is a benchmark of their 2019 implementation, not a current performance promise.

The system can also change the *meaning* of its radiance values without duplicating the renderer. For polarization, it represents light with Stokes vectors and material transformations with Mueller matrices. Each Mueller matrix is tied to incident and outgoing reference frames, so those frames must be rotated consistently when light scatters. The renderer uses Mueller-matrix values throughout to keep its interfaces symmetric, even though this performs some unnecessary arithmetic. The paper demonstrates a dielectric between crossed linear polarizers: the glass changes the polarization state so some light passes the second filter. It also enables spectrum-aware transport rather than treating every ray as three fixed RGB numbers; the paper's renderer samples four wavelengths per ray across 360–830 nm by default.

#### Differentiable caustics and media

A differentiable renderer can optimize a scene parameter to make a rendered image match a target. For caustics, one example changes a refractive height field so focused light forms a desired image. Another changes a spatially varying refractive index. In a gradient-index medium, rays curve because the index varies with position. The paper writes the ray equation as

$$\frac{d^2\mathbf{x}}{dt^2}=n(\mathbf{x})\nabla n(\mathbf{x}).$$

The implementation integrates this equation with a Leapfrog step while tracing light. Gradient descent then updates the height field or index grid from the image error. These demonstrations show that a general renderer can support inverse design with a custom differentiable simulation; the authors explicitly say their height-field results do not match purpose-built methods in contrast and precision.

For a scattering medium, the radiative transfer equation (RTE) accounts for light lost to absorption or scattering out of the ray and gained from light scattered into it. In simplified notation,

$$L(\mathbf{x},\omega)=\int_0^\infty T_r(\mathbf{x},\mathbf{x}_t)\int_{S^2}\sigma_s(\mathbf{x}_t)L(\mathbf{x}_t,\omega')f_p(\omega,\omega')\,d\omega'\,dt,$$

where $\sigma_s$ is the scattering coefficient, $f_p$ is the phase function, and $T_r=\exp(-\int_0^t\sigma_t(\mathbf{x}_s)ds)$ is transmittance, with extinction $\sigma_t=\sigma_a+\sigma_s$. Sampling the distance to the next event normally uses a parameter-dependent probability distribution. Directly differentiating through that sampling transform can fail when it becomes discontinuous—for example, when the sampled event leaves the volume bounds. The paper's solution is to differentiate the integral before changing variables into primary-sample space, then detach the sampling transform and its probability from the AD graph. This keeps the derivative on the integrand while avoiding an invalid derivative through a discontinuous sampling map.

The authors initially explored delta tracking, which supports unbiased event sampling but can take a variable number of steps and create incoherent GPU execution. They use regular ray marching instead. This gives more coherent GPU work but introduces a small rendering bias. Their smoke-density experiment is synthetic: one view is ambiguous and creates artifacts; matching nine views constrains the density better. Their translucent-slab experiment is also simplified: extinction is homogeneous and monochromatic, while voxel albedo varies. Differentiable rendering improves on a naive color-to-albedo initialization because multiple scattering couples neighboring voxel values.

#### Coherent bundles for Markov-chain rendering

Mitsuba 2's vectorization also supports a path-sampling method designed for coherence. A conventional packet tracer may launch many rays together, but their paths quickly diverge into different branches and memory locations. Markov-chain Monte Carlo light transport explores nearby points in **primary sample space**, the random numbers that define a path. The paper's coherent pseudo-marginal Metropolis light transport (CPMMLT) smooths the target function by convolving it with a Gaussian in that space:

$$\pi(\mathbf{u})=\int_U G(\boldsymbol\delta)f(\mathbf{u}-\boldsymbol\delta)\,d\boldsymbol\delta.$$

It estimates this smoothed target with a bundle of nearby paths, evaluating them together in vector lanes. The Gaussian width trades exploration against coherence: nearby samples are more likely to take similar path branches, so the CPU's vector units and caches do more useful work. The method uses pseudo-marginal MCMC: an unbiased estimate of the smoothed target can be used while preserving the chain's target distribution. The paper then weights the bundle's contributions to reconstruct an unbiased estimate of the original, unblurred image. In its tests, CPMMLT evaluates about two to six times more rays per second than the PSSMLT baseline, depending on the scene. Those tests ran on 28-core Intel Xeon Gold 6132 machines; the result is specific to that hardware and comparison.

**Practical limits.** Reverse-mode AD stores state and may run out of memory as resolution, samples, and path depth grow. Rendering losses have poor local minima, so parameter initialization matters. Ray marching trades a small bias for GPU coherence. The 2019 paper's software and interfaces are historical: describe the architecture and evidence from the paper, but do not treat its Enoki/Mitsuba 2 API as the current Mitsuba 3 or Dr.Jit API. Its examples demonstrate system flexibility; they do not establish that every inverse-rendering problem is well posed or that one view determines a unique scene.

**Source:** Nimier-David et al., “Mitsuba 2: A Retargetable Forward and Inverse Renderer” (2019), full substantive paper, PDF pp. 1–16. [Primary PDF](https://rgl.s3.eu-central-1.amazonaws.com/media/papers/NimierDavidVicini2019Mitsuba2_7.pdf).

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/10 - A Production Path Tracer Media, Randomness and Validation|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty|Next chapter]]
