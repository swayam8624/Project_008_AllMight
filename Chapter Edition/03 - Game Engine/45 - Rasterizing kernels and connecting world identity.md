# 45 — Rasterizing kernels and connecting world identity

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/44 - Cross-backend conversion and agreement|Previous chapter]] · [[Chapter Edition/03 - Game Engine/46 - Game design from mechanics to narrative interfaces|Next chapter]]

A rendered kernel is a projected footprint, but editing and world reconstruction need more than pixels. This chapter connects scalable raster work to identity, geometry, and uncertainty in captured scenes.

**By the end:** Trace kernel projection and compositing while stating the conditions needed to associate visible samples with editable world entities.

**Recall question:** How does a changed kernel affect later transmittance, and what evidence connects its rendered footprint to a world-space object?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 108499–112054; source-span SHA256 39e2f801fc0e3b6a74db8fafeffd8e67737366aff333d0deef17faf5ad7d5034. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Rasterize kernels at scale and connect their identities — world correspondence

**Policy distinction:** the source opacity cutoff $A>0.999$ means remaining transmittance below $10^{-3}$. The inspected Graphdeco reference uses $10^{-4}$; these are separate accuracy/performance policies, as derived above.

### CAPTURED WORLDS: FROM APPEARANCE FIELD TO EDITABLE, PHYSICAL SCENE

This is where 3DGS stops being “a renderer feature” and becomes your research problem.

---


A reconstructed Gaussian scene is excellent at answering:

> What should this place look like from this camera?

But an engine/editor wants to ask:

> Which Gaussians are the chair?

> Move the chair.

> Simulate the curtain.

> Apply force here.

> Replace only damaged geometry.

> Preserve the rest of the captured world.

A raw unordered splat vector does not answer those questions.

So you need a layer above rendering:

$$
\boxed{
\text{Captured Appearance}
+
\text{Persistent Semantics}
+
\text{Physical Correspondence}.
}
$$

That is the basis of Maveb/Vulkax's world-Gaussian work.

---


A Gaussian has:

$$
\mu,
\Sigma,
\alpha,
SH.
$$

A physical body requires things such as:

$$
mass,
velocity,
material,
forces,
constraints,
collision topology.
$$

These are not equivalent.

---

**Covariance is not automatically an inertia tensor**

Gaussian covariance tells:

$$
\boxed{
\text{spatial appearance support}.
}
$$

Rigid-body inertia tells:

$$
\boxed{
\text{resistance to angular acceleration from mass distribution}.
}
$$

Both are symmetric matrices.

That superficial similarity does not make them physically interchangeable.

Do not do:

$$
I=\Sigma.
$$

That would be semantically unjustified.

---

**Opacity is not density**

Learned opacity:

$$
\alpha
$$

answers rendering visibility.

It is not automatically:

$$
kg/m^3.
$$

A glass object may have low opacity yet significant mass.

A painted visual artifact may have strong opacity but no corresponding physical volume.

Therefore:

$$
\boxed{
\text{radiance-field parameters need an explicit physics bridge}.
}
$$

---


Let semantic entity:

$$
E
$$

represent:

```text id="f0n5qg"
chair
table
cloth
wall
cup
```

We need mapping:

$$
\boxed{
Gaussian_i
\rightarrow
Entity_j.
}
$$

Maveb already has:

```cpp
GaussianEntityOwnership
```

where each Gaussian index has a persistent:

$$
world::EntityId.
$$

---

**Maveb ownership assignment**

Current ownership assignment ranks candidate entities using world-space AABBs.

A Gaussian whose center lies inside an entity is preferred.

If multiple contain it:

$$
\boxed{
\text{smaller container wins}.
}
$$

This prevents:

> cup Gaussian assigned to entire room

when cup's bounds also contain point.

If point is outside but close, nearest eligible bounds within configured distance can own it.

Tie breaks use stable entity ID.

Deterministic.

---

**Why entity IDs must persist**

Imagine entity data:

```text id="7x0yqd"
Entity 42 = chair
```

The chair may move.

Gaussian ordering may change.

Spatial index may rebuild.

But:

$$
EntityId=42
$$

must remain meaningful.

Semantic persistence is a different problem from buffer indexing.

---


One semantic entity may correspond to many simulation variables.

A cloth entity may map to:

$$
N
$$

cloth vertices.

A deformable volume:

$$
MPM\ particles.
$$

A rigid object:

$$
6
$$

DOFs.

Vulkax's correspondence model explicitly supports physical binding categories including concepts such as:

```text id="ijumvh"
surface vertex
MPM particle
DEM particle
grid cell
rigid body
```

and each binding has:

$$
weight.
$$

So architecture becomes:

$$
Gaussian
\rightarrow
Entity
\rightarrow
PhysicalDOFs.
$$

---

**Why not Gaussian→physics directly for everything?**

Semantic entity layer provides structure.

Suppose chair has:

$$
50,000
$$

Gaussians but one rigid transform.

Binding every Gaussian independently to six rigid DOFs is wasteful.

Instead:

$$
Gaussians
\rightarrow
ChairEntity
\rightarrow
RigidBody.
$$

For cloth, entity may map to a deformable mesh/particle set.

Different physical representation.

Same appearance layer.

---


Pass 15 established the problem.

Array index is ephemeral.

If splats are:

```text id="f90y7n"
[A,B,C]
```

and filter B:

```text id="f1gg6w"
[A,C]
```

C moves from:

$$
2\rightarrow1.
$$

If selection stored:

$$
index=2,
$$

selection is now invalid.

Stable identity instead:

$$
ID_C
$$

survives.

---

**Vulkax's stable ID**

Vulkax stores:

$$
(namespaceID,localID).
$$

Packed:

$$
\boxed{
ID=
(namespace<<32)
|
local.
}
$$

`GaussianIndexView` rebuilds:

$$
ID\rightarrow currentIndex
$$

after reorder/filter.

That is the correct pattern.

---

**Current Maveb nuance**

Maveb's raw canonical `GaussianAsset` currently remains vector-index based.

Higher world-Gaussian systems maintain persistent entity ownership/revision semantics, but a truly stable per-primitive ID embedded into the canonical Gaussian layer would be stronger for arbitrary reorder/filter operations.

So for M150's stable-selection ideal, Vulkax is currently the cleaner concrete example.

This distinction matters.

---


Editor selection should be:

$$
\boxed{
S=
\{ID_1,ID_2,\dots,ID_k\}.
}
$$

Not:

$$
\{17,84,932\}
$$

meaning array positions.

At render/edit time:

$$
ID
\rightarrow
currentIndex.
$$

If Gaussian no longer exists:

$$
\boxed{
\text{selection member becomes missing}
}
$$

rather than silently selecting another primitive.

---

**Entity selection can be even more stable**

Often editor wants:

$$
EntityID.
$$

Then query:

$$
GaussiansForEntity(E).
$$

This gives all appearance primitives belonging to entity.

Vulkax correspondence graph stores both:

$$
GaussianID\rightarrow EntityID
$$

and reverse:

$$
EntityID\rightarrow
[\text{Gaussian IDs}].
$$

It validates forward/reverse agreement.

Excellent.

---


Suppose physical deformation locally maps rest positions:

$$
x'
=
F x+t.
$$

Here:

$$
F
$$

is deformation gradient/local affine linear map.

Gaussian mean:

$$
\boxed{
\mu'
=
F\mu+t.
}
$$

How does covariance transform?

Exactly as before:

$$
\boxed{
\Sigma'
=
F
\Sigma
F^T.
}
$$

Same covariance transformation law from Pass 15.

---

**Derivation**

Rest displacement:

$$
\delta=x-\mu.
$$

After affine deformation:

$$
\delta'
=
F\delta.
$$

Then:

$$
\Sigma'
=
E[
\delta'\delta'^T
].
$$

Substitute:

$$
=
E[
F\delta
\delta^T
F^T
].
$$

Therefore:

$$
\boxed{
\Sigma'
=
F\Sigma F^T.
}
$$

No approximation for an affine map.

---

**Rigid rotation**

If:

$$
F=R,
$$

with:

$$
R^TR=I,
$$

then:

$$
\Sigma'
=
R\Sigma R^T.
$$

Eigenvalues unchanged.

Only orientation changes.

Exactly what an appearance Gaussian should do when rigid entity rotates.

Maveb's entity edit currently rotates Gaussian center and composes its quaternion accordingly.

---

**Uniform scale**

If:

$$
F=sI,
$$

then:

$$
\Sigma'
=
sI\Sigma sI
=
\boxed{s^2\Sigma}.
$$

Physical standard deviations multiply by:

$$
s.
$$

Since stored scale is logarithmic:

$$
\ell'=
\log(ss_i)
$$

$$
=
\log s+\ell_i.
$$

Maveb's uniform scale edit does exactly:

```cpp
logScale[axis] +=
    log(factor);
```

Perfect.

---

**Shear**

Suppose:

$$
F=
\begin{bmatrix}
1&k&0\\
0&1&0\\
0&0&1
\end{bmatrix}.
$$

Then:

$$
F\Sigma F^T
$$

generally rotates and changes principal variances.

A simple quaternion update plus component-wise scale is not enough unless you recompute eigensystem.

For arbitrary deformable physics, full covariance transformation is required.

---

**Recover Gaussian scale and orientation**

After deformation:

$$
\Sigma'
$$

is symmetric positive semidefinite if $F$ finite.

Eigendecompose:

$$
\Sigma'
=
Q\Lambda Q^T.
$$

Then:

$$
s_i'=
\sqrt{\lambda_i}.
$$

Stored:

$$
\boxed{
\ell_i'
=
\frac12
\log\lambda_i.
}
$$

Rotation matrix:

$$
R'=Q.
$$

Convert $R'$ back to quaternion.

Vulkax's MLS implementation does this route: it constructs rest covariance, applies local affine deformation, then converts the resulting covariance back into Gaussian shape parameters.

---


Suppose Gaussian rest position:

$$
X_g.
$$

It is supported by physical points:

$$
X_i
$$

with weights:

$$
w_i.
$$

Define Gaussian deformed position:

$$
\boxed{
x_g
=
\sum_i
w_i x_i.
}
$$

For this to reproduce translations:

$$
\boxed{
\sum_i w_i=1.
}
$$

This is **partition of unity**.

---

**Translation reproduction**

If every physical point translates by:

$$
t:
$$

$$
x_i=X_i+t.
$$

Then:

$$
x_g
=
\sum_i
w_i(X_i+t)
$$

$$
=
\sum_iw_iX_i
+
t\sum_iw_i.
$$

If:

$$
\sum_iw_i=1
$$

and rest reconstruction:

$$
\sum_iw_iX_i=X_g,
$$

then:

$$
\boxed{
x_g=X_g+t.
}
$$

Perfect.

---

**Full affine reproduction**

Suppose physical deformation is:

$$
x_i=
AX_i+b.
$$

Then:

$$
x_g
=
\sum_i
w_i
(AX_i+b).
$$

$$
=
A\sum_iw_iX_i
+
b\sum_iw_i.
$$

If:

$$
\sum_iw_i=1
$$

and:

$$
\sum_iw_iX_i=X_g,
$$

then:

$$
\boxed{
x_g=AX_g+b.
}
$$

This is affine reproduction.

That is a powerful property.

---

**Vulkax explicitly measures these errors**

Its `GaussianSupport` stores:

```text id="5d4p4w"
partitionOfUnityError
affineReproductionError.
```

And embedding stores global maxima.

That is excellent research instrumentation.

You are not merely asserting:

> embedding should preserve affine motion.

You measure how closely it does.

---

**MLS support construction**

Vulkax finds nearby physical support points.

It starts from distance-based kernel weights:

$$
k_i
=
\exp
\left(
-\frac{
\|X_i-X_g\|^2
}{
h^2
}
\right)
+\epsilon.
$$

Then solves a moment system to obtain corrected interpolation weights that reproduce affine basis:

$$
[1,x,y,z].
$$

This is moving-least-squares style interpolation.

---

**Important: corrected weights can differ from simple positive kernels**

Kernel:

$$
k_i>0.
$$

But affine-reproduction weights derived from the moment solve are not merely normalized Gaussian distance weights.

They include the basis correction required to reproduce:

$$
1,x,y,z.
$$

This distinction is essential.

Simple normalized inverse-distance weights guarantee partition of unity but not exact affine reproduction.

---


Suppose user clicks Gaussian $g$ and applies force:

$$
f_g.
$$

Transfer:

$$
\boxed{
f_i
=
w_i f_g.
}
$$

Total transferred force:

$$
\sum_i f_i
=
\left(
\sum_iw_i
\right)f_g.
$$

If partition of unity:

$$
\sum_iw_i=1,
$$

then:

$$
\boxed{
\sum_i f_i=f_g.
}
$$

Force conserved.

---

**Torque conservation**

Requested torque about origin:

$$
\tau_g=
x_g\times f_g.
$$

Transferred:

$$
\tau=
\sum_i
x_i\times f_i
$$

$$
=
\sum_i
x_i\times
(w_i f_g)
$$

$$
=
\left(
\sum_iw_ix_i
\right)
\times f_g.
$$

If interpolation reproduces Gaussian position:

$$
\sum_iw_ix_i=x_g,
$$

then:

$$
\boxed{
\tau=\tau_g.
}
$$

So affine reproduction is not merely visual nicety.

It directly supports physically meaningful force/torque transfer.

---

**Vulkax measures both**

Its force transfer evidence records:

$$
requestedForce,
transferredForce,
$$

$$
requestedTorque,
transferredTorque,
$$

and:

$$
forceConservationError,
torqueConservationError.
$$

That is exactly the kind of evidence a research system should expose.

---


After simulation updates physical points:

$$
X_i\rightarrow x_i,
$$

appearance Gaussians must follow.

Simplest:

$$
\mu_g'
=
\sum_iw_ix_i.
$$

But this moves centers only.

It leaves covariance unchanged.

Then a stretched cloth moves Gaussians but their ellipsoids do not stretch.

Visually wrong.

---

**Fit local affine deformation**

For Gaussian support, fit:

$$
x_i
\approx
F X_i+t.
$$

Weighted least squares:

$$
\min_{F,t}
\sum_i
k_i
\|
FX_i+t-x_i
\|^2.
$$

Vulkax builds a 4D affine basis:

$$
[1,X,Y,Z]
$$

and solves separate coefficient systems for output x/y/z.

That produces:

$$
F
$$

and:

$$
t.
$$

Then:

$$
\mu'=
F\mu+t
$$

and:

$$
\Sigma'
=
F\Sigma F^T.
$$

This is much stronger than merely translating splat centers.

---

**Non-accumulating deformation**

Vulkax stores original:

```text id="npbq8r"
restPosition
restLogScale
restRotation.
```

Every update maps from rest geometry.

Why?

If each frame did:

$$
\Sigma_{n+1}
=
F_n
\Sigma_n
F_n^T
$$

with noisy estimated $F_n$, numerical drift accumulates.

Instead:

$$
\boxed{
\Sigma_t
=
F_t
\Sigma_{rest}
F_t^T.
}
$$

More stable.

This is a crucial design choice.

---


Editable captured worlds need history.

You must be able to answer:

> Which entity changed?

> At what timestamp?

> Which Gaussians were affected?

> Which renderer buffers received the new version?

> What pixels could have changed?

> Can the edit be reproduced?

Maveb already treats this as a first-class system.

---

**Persistent world revision**

Current entity edit requires timestamp:

$$
t_{new}
>
t_{latest}.
$$

It prepares a world patch.

It preflights Gaussian selection and spatial relocation.

Only then commits persistent world revision.

This is transactional thinking:

$$
\boxed{
\text{validate first}
\rightarrow
\text{commit authoritative state}
\rightarrow
\text{publish derived changes}.
}
$$

---

**Gaussian publication journal**

Maveb's GPU pipeline maintains versioned Gaussian source buffers for its three frame slots.

Local edits journal changed source indices.

When a frame slot becomes reusable, only stale changed ranges need publication.

This is far better than uploading millions of unchanged Gaussians after every local edit.

It records:

```text id="z2q1ht"
touchedRecords
contiguousRanges
touchedBytes
fullBufferBytes.
```

Then:

$$
\boxed{
byteRatio
=
\frac{
touchedBytes
}{
fullBufferBytes
}.
}
$$

That is exactly the metric relevant to “repair only what changed.”

---

**Image revision certificates**

Maveb also contains Gaussian image revision certificates.

For changed splats it computes conservative per-pixel bounds on how much RGB could differ.

Why?

If an edit is local, you want evidence that pixels outside predicted support did not change unexpectedly.

This moves validation beyond:

> screenshot looks okay.

toward:

$$
\boxed{
\text{bounded, attributable image change}.
}
$$

---

**Early-termination allowance in certificate**

The certificate code explicitly accounts for compositor termination at:

$$
A>0.999.
$$

One endpoint may omit up to:

$$
0.001
$$

transmittance.

Comparing before and after permits up to:

$$
0.002
$$

two-sided transmittance difference.

So its conservative RGB bound includes:

$$
0.002\times colorUpperBound
$$

where changed alpha is present.

This is a very good example of connecting renderer approximation to formal edit verification.

---


This is one of the most important conceptual safeguards.

A learned Gaussian primitive has no inherent:

$$
mass.
$$

No inherent:

$$
Young's\ modulus.
$$

No inherent:

$$
friction.
$$

No inherent:

$$
collision\ surface.
$$

No connectivity.

No topology.

No conservation law.

It is an appearance primitive.

---

**A thin Gaussian does not prove a thin sheet**

A flat anisotropic covariance might represent:

- wall texture,
- edge of a volumetric cluster,
- a training artifact,
- foliage,
- actual sheet material.

You cannot infer material class from covariance alone.

Likewise many splats may overlap one physical point.

Or one large splat may span several physical elements.

Correspondence must be constructed and validated.

---

**Birth and death are not trivial**

If physical deformation tears cloth:

Should existing Gaussian split?

Should new Gaussians be born?

Which stable IDs do they get?

What SH coefficients?

What opacity?

Which entity owns them?

Maveb's current persistent entity edit explicitly leaves Gaussian birth/death outside that API because it requires explicit ownership/cardinality semantics.

That is correct.

Deleting/creating appearance primitives is a higher-level operation than affine deformation.

---


Here is the whole architecture, with every important transformation exposed.

```cpp id="t0uvl1"
// -----------------------------------------------------------------------------
// A deliberately explicit CPU-side reference architecture.
// This is not meant to replace Maveb's optimized implementation; it shows the
// semantic pipeline from imported trained parameters to final pixel values.
// -----------------------------------------------------------------------------

struct Gaussian
{
    Vec3 position;

    // Trained parameterization. Keep these encoded in canonical asset state.
    Vec3 logScale;

    // Quaternion in the canonical (w,x,y,z) convention.
    Quat rotation;

    float opacityLogit;

    // Spherical-harmonic color coefficients.
    Vec3 shDC;
    std::array<float, 45> shRest;
    uint32_t shRestCount;

    // A production editable-world system should use persistent identity here.
    GaussianId stableId;
};

struct ProjectedGaussian
{
    Vec2 center;
    float depth;
    float radius;

    // The symmetric inverse covariance:
    //
    // [ A B ]
    // [ B C ]
    //
    float A;
    float B;
    float C;

    float opacity;
    Vec3 color;

    TileRect tiles;

    GaussianId stableId;
};

ProjectedGaussian projectGaussian(
    const Gaussian& g,
    const Camera& camera)
{
    // -------------------------------------------------------------------------
    // 1. Recover physical Gaussian scale.
    //
    // log-scale guarantees positive scales after exponential decoding.
    // -------------------------------------------------------------------------
    const Vec3 scale
    {
        std::exp(g.logScale.x),
        std::exp(g.logScale.y),
        std::exp(g.logScale.z)
    };

    // -------------------------------------------------------------------------
    // 2. Convert quaternion to an orthonormal 3×3 rotation.
    // -------------------------------------------------------------------------
    const Mat3 R =
        rotationMatrix(
            normalize(g.rotation));

    // -------------------------------------------------------------------------
    // 3. Build world covariance:
    //
    // Σw = R diag(s²) Rᵀ
    // -------------------------------------------------------------------------
    const Mat3 variance =
        diagonal(
            scale.x * scale.x,
            scale.y * scale.y,
            scale.z * scale.z);

    const Mat3 sigmaWorld =
        R *
        variance *
        transpose(R);

    // -------------------------------------------------------------------------
    // 4. Transform center into camera coordinates.
    // -------------------------------------------------------------------------
    const Vec3 p =
        transformPoint(
            camera.worldToCamera,
            g.position);

    if (p.z < camera.nearPlane ||
        p.z > camera.farPlane)
    {
        throw Culled{};
    }

    // -------------------------------------------------------------------------
    // 5. Perspective-project mean.
    // -------------------------------------------------------------------------
    ProjectedGaussian out;

    out.center.x =
        camera.fx * p.x / p.z
        + camera.cx;

    out.center.y =
        camera.fy * p.y / p.z
        + camera.cy;

    out.depth =
        p.z;

    // -------------------------------------------------------------------------
    // 6. Transform covariance into camera coordinates.
    //
    // Translation does not affect covariance.
    // -------------------------------------------------------------------------
    const Mat3 Rc =
        upper3x3(
            camera.worldToCamera);

    const Mat3 sigmaCamera =
        Rc *
        sigmaWorld *
        transpose(Rc);

    // -------------------------------------------------------------------------
    // 7. Derivative of perspective projection.
    // -------------------------------------------------------------------------
    const float invZ =
        1.0f / p.z;

    const Vec3 jx
    {
        camera.fx * invZ,
        0.0f,
        -camera.fx *
         p.x *
         invZ * invZ
    };

    const Vec3 jy
    {
        0.0f,
        camera.fy * invZ,
        -camera.fy *
         p.y *
         invZ * invZ
    };

    // -------------------------------------------------------------------------
    // 8. Σscreen = J Σcamera Jᵀ.
    // Add low-pass variance exactly as the Maveb path does.
    // -------------------------------------------------------------------------
    const float a =
        dot(
            jx,
            sigmaCamera * jx)
        + 0.3f;

    const float b =
        dot(
            jx,
            sigmaCamera * jy);

    const float c =
        dot(
            jy,
            sigmaCamera * jy)
        + 0.3f;

    const float det =
        a * c - b * b;

    if (!std::isfinite(det) ||
        det <= 1e-12f)
    {
        throw Culled{};
    }

    // -------------------------------------------------------------------------
    // 9. Invert 2×2 covariance to form conic.
    // -------------------------------------------------------------------------
    out.A =  c / det;
    out.B = -b / det;
    out.C =  a / det;

    // -------------------------------------------------------------------------
    // 10. Conservative three-sigma screen bound.
    // -------------------------------------------------------------------------
    const float discriminant =
        std::sqrt(
            std::max(
                0.0f,
                (a - c) * (a - c)
                + 4.0f * b * b));

    const float lambdaMax =
        0.5f *
        (a + c + discriminant);

    out.radius =
        3.0f *
        std::sqrt(lambdaMax);

    // -------------------------------------------------------------------------
    // 11. Decode opacity and SH appearance.
    // -------------------------------------------------------------------------
    out.opacity =
        sigmoid(
            g.opacityLogit);

    out.color =
        evaluateSH(
            g,
            normalize(
                g.position -
                camera.worldPosition));

    out.stableId =
        g.stableId;

    // -------------------------------------------------------------------------
    // 12. Screen and tile culling.
    // -------------------------------------------------------------------------
    const int minX =
        clamp(
            floor(out.center.x -
                  out.radius),
            0,
            camera.width - 1);

    const int maxX =
        clamp(
            ceil(out.center.x +
                 out.radius),
            0,
            camera.width - 1);

    const int minY =
        clamp(
            floor(out.center.y -
                  out.radius),
            0,
            camera.height - 1);

    const int maxY =
        clamp(
            ceil(out.center.y +
                 out.radius),
            0,
            camera.height - 1);

    if (minX > maxX ||
        minY > maxY)
    {
        throw Culled{};
    }

    constexpr int TileSize = 16;

    out.tiles.minX =
        minX / TileSize;

    out.tiles.maxX =
        maxX / TileSize;

    out.tiles.minY =
        minY / TileSize;

    out.tiles.maxY =
        maxY / TileSize;

    return out;
}
```

Then reference expansion:

```cpp id="0qgv44"
// First determine how many tile references every Gaussian emits.
for (uint32_t g = 0;
     g < projected.size();
     ++g)
{
    counts[g] =
        projected[g].tiles.width()
        *
        projected[g].tiles.height();
}

// Exclusive scan allocates each Gaussian a disjoint write interval.
offsets =
    exclusivePrefixSum(
        counts);

const uint32_t totalReferences =
    offsets.back()
    + counts.back();

keys.resize(totalReferences);
values.resize(totalReferences);

// Expand one projected Gaussian into every tile it overlaps.
for (uint32_t g = 0;
     g < projected.size();
     ++g)
{
    uint32_t dst =
        offsets[g];

    for (
        uint32_t ty =
            projected[g].tiles.minY;
        ty <=
            projected[g].tiles.maxY;
        ++ty)
    {
        for (
            uint32_t tx =
                projected[g].tiles.minX;
            tx <=
                projected[g].tiles.maxX;
            ++tx)
        {
            const uint32_t tileId =
                ty * tileColumns + tx;

            // The implementation may encode this in two uint32s,
            // but semantically the composite key is:
            //
            //      primary   = tile
            //      secondary = depth
            //
            keys[dst] =
                {
                    tileId,
                    projected[g].depth
                };

            values[dst] =
                g;

            ++dst;
        }
    }
}
```

Sort stably:

```cpp id="mm1u1e"
// Stable order is required for deterministic equal-depth behavior.
stableSort(
    keys,
    values,
    [](const Key& a,
       const Key& b)
    {
        if (a.tile != b.tile)
            return a.tile <
                   b.tile;

        return a.depth <
               b.depth;
    });
```

Build ranges:

```cpp id="gr6rum"
for each tile:
    range[tile] =
        Empty;

for (
    uint32_t i = 0;
    i < keys.size();
    ++i)
{
    const uint32_t tile =
        keys[i].tile;

    if (range[tile].empty())
        range[tile].begin = i;

    range[tile].end =
        i + 1;
}
```

And finally composite:

```cpp id="ep4bnk"
for (
    int y = 0;
    y < height;
    ++y)
{
    for (
        int x = 0;
        x < width;
        ++x)
    {
        const uint32_t tile =
            (y / 16) *
            tileColumns
            +
            (x / 16);

        Vec3 color{0.0f};
        float transmittance =
            1.0f;

        float firstDepth =
            infinity;

        GaussianId dominantId{};
        float dominantContribution =
            0.0f;

        for (
            uint32_t r =
                ranges[tile].begin;
            r <
                ranges[tile].end;
            ++r)
        {
            const ProjectedGaussian& g =
                projected[
                    values[r]];

            // Pixel centers, not integer corners.
            const float dx =
                (x + 0.5f)
                - g.center.x;

            const float dy =
                (y + 0.5f)
                - g.center.y;

            const float q =
                g.A * dx * dx
                +
                2.0f *
                g.B *
                dx * dy
                +
                g.C * dy * dy;

            // Three-sigma ellipse.
            if (q > 9.0f)
                continue;

            const float alpha =
                std::min(
                    0.99f,
                    g.opacity *
                    std::exp(
                        -0.5f * q));

            if (
                alpha <
                1.0f / 255.0f)
            {
                continue;
            }

            const float weight =
                transmittance
                * alpha;

            color +=
                weight *
                g.color;

            if (
                firstDepth ==
                infinity)
            {
                firstDepth =
                    g.depth;
            }

            if (
                weight >
                dominantContribution)
            {
                dominantContribution =
                    weight;

                dominantId =
                    g.stableId;
            }

            transmittance *=
                1.0f - alpha;

            // Error remaining behind this point is now bounded by
            // approximately 0.1% of the remaining color range.
            if (
                transmittance <
                0.001f)
            {
                break;
            }
        }

        color +=
            transmittance *
            background;

        output[x,y] =
            color;
    }
}
```

That is the complete PLY→pixel story.

Everything else in a production implementation is principally about doing these same semantics:

$$
\boxed{
\text{faster, bounded, parallel, deterministic and editable}.
}
$$

---


When a Gaussian renderer is wrong, do not debug the whole image simultaneously.

There are six major semantic layers.

| Layer | What to inspect | Typical symptom |
|---|---|---|
| Representation | log-scale, quaternion, opacity logit, SH layout | everything malformed from load |
| Covariance | $R\,diag(s^2)R^T$, SPD, eigenvalues | wrong ellipse shape/orientation |
| Projection | camera transform, $J\Sigma J^T$, center, radius | splats shifted/huge/missing |
| Ordering | tile keys, radix stability, ranges | halos, wrong transparency, flicker |
| SH color | direction convention, coefficient ordering, degree | geometry correct but colors wrong |
| Alpha | sigmoid, Gaussian exponent, thresholds, transmittance | density/opacity wrong |

This is the debugging order I would use.

---

**Representation debug**

For one Gaussian print:

```text id="fn1jmm"
position
logScale
exp(logScale)
rotation length
opacityLogit
sigmoid(opacity)
SH degree
stable/source ID
```

If these are wrong:

Do not look at rasterizer.

---

**Covariance debug**

Check symmetry:

$$
\Sigma-\Sigma^T\approx0.
$$

Check eigenvalues:

$$
\lambda_i>0.
$$

Check determinant finite.

Check known identity quaternion:

$$
R=I.
$$

For:

$$
scale=(2,3,4),
$$

expect:

$$
\Sigma=
diag(4,9,16).
$$

If this fails, projection is irrelevant.

---

**Projection debug**

Use Maveb's depth mode.

Check:

$$
X,Y,Z
$$

camera coordinates.

Check center:

$$
u,v.
$$

Check:

$$
a,b,c.
$$

Check conic.

Check:

$$
radius.
$$

Check tile bounds.

If source ID view shows Gaussian in wrong area, projection/tiling is implicated.

---

**Ordering debug**

Maveb has explicit **sort-rank debug view**.

This is extremely valuable.

A near contributor should have lower normalized tile-local rank than farther contributor.

If color wrong only where splats overlap and rank is wrong:

$$
\boxed{
\text{do not blame SH}.
}
$$

Look at:

- float depth key,
- radix pass order,
- stability,
- range construction.

---

**SH debugging**

Disable higher orders.

Render DC only.

If DC is correct:

Enable degree 1.

Then degree 2.

Then degree 3.

This isolates coefficient-order bug.

Maveb's SH-band debug view shows which maximum stored degree the dominant Gaussian has, while appearance view shows evaluated result.

---

**Alpha debugging**

Render accumulated opacity directly.

Maveb already has that mode.

Check one Gaussian center:

$$
\alpha\approx
\min(
0.99,
sigmoid(logit)).
$$

At 1σ:

$$
\alpha
\approx
\alpha_0e^{-0.5}.
$$

At 3σ:

$$
\alpha
\approx
0.0111\alpha_0.
$$

If that relation fails, conic/exponent/opacity is wrong.

---

### THE DEEPEST ARCHITECTURAL CONNECTION OF PHASE 25

The Gaussian renderer and the editable-world system are actually two different mappings of the same primitive.

Rendering asks:

$$
\boxed{
Gaussian
\rightarrow
Pixels.
}
$$

Physics asks:

$$
\boxed{
Gaussian
\leftrightarrow
Physical\ DOFs.
}
$$

Semantics asks:

$$
\boxed{
Gaussian
\leftrightarrow
Entity.
}
$$

Persistence asks:

$$
\boxed{
Gaussian_t
\leftrightarrow
Gaussian_{t+1}.
}
$$

Maveb's research problem is not merely rendering Gaussians.

It is preserving all four correspondences while locally changing the world.

---

**Why local repair is difficult**

Suppose entity E moves.

You cannot simply edit all nearby Gaussians.

Nearby splats may belong to:

- wall behind E,
- floor,
- neighboring object,
- reflections,
- background.

So locality in Euclidean space is not enough.

You need:

$$
\boxed{
\text{spatial locality}
+
\text{semantic ownership}
+
\text{appearance support}
}.
$$

Maveb's current indexed local-update path combines dirty spatial regions and entity ownership.

That is substantially stronger than radius selection alone.

---

**Maveb local-update flow**

Current architecture roughly does:

```text id="ia3d5k"
Persistent entity edit
↓
Determine dirty world regions
↓
Query Gaussian spatial/overlay index
↓
Reject Gaussians whose stable owner
does not belong to edited entity
↓
Produce bounded selected set
↓
Preflight replacements / relocations
↓
Commit world revision
↓
Mutate selected Gaussians
↓
Update overlay spatial index
↓
Journal GPU publication ranges
↓
Render/verify changed image
```

This is directly aligned with your **Repair What Matters** research thesis.

---

**Ownership assignment itself is approximate**

Current Maveb uses Gaussian centers against semantic entity AABBs.

That is practical but imperfect.

A large anisotropic Gaussian's center may lie outside an entity while most of its support overlaps it.

Conversely center inside may have covariance extending into another entity.

A more sophisticated ownership model could integrate Gaussian support overlap:

$$
P(E|G)
$$

or weighted covariance-volume intersection rather than center-only proximity.

This is an actual research direction.

---

**Hard ownership versus soft correspondence**

Hard:

$$
owner(G_i)=E_j.
$$

Soft:

$$
w_{ij}
$$

with:

$$
\sum_jw_{ij}=1.
$$

Hard ownership is simple for editing.

Soft ownership may better represent boundaries or uncertain reconstruction.

For physics coupling, soft support weights are already natural.

So one could have:

$$
\boxed{
\text{hard semantic owner}
+
\text{soft physical support}.
}
$$

That is often a good separation.

---

**Covariance deformation reveals why appearance is richer than center tracking**

If you only update:

$$
\mu,
$$

a Gaussian field can translate but not deform correctly.

A stretching object requires:

$$
\Sigma'
=
F\Sigma F^T.
$$

This gives:

- rotation,
- elongation,
- compression,
- shear.

The covariance is therefore the appearance representation's local differential geometry.

That is one of the deepest reasons 3DGS pairs naturally with local affine physical deformation.

---

**But SH appearance also changes under deformation**

A subtle issue:

When object rotates, SH coefficients are directional functions in world/capture frame.

If Gaussian orientation changes physically, what should happen to its view-dependent radiance function?

For a rigid object whose appearance is attached to material, its directional appearance should rotate with object frame.

Leaving SH coefficients interpreted in the old world frame can produce inconsistent highlights.

Therefore full physical edit may require transforming/re-parameterizing directional appearance, not only:

$$
\mu,\Sigma.
$$

Current simple entity rotation primarily updates geometry/orientation.

This is an important appearance-physics research caveat.

---

**Similarly, opacity may not remain invariant**

Stretch a thin material.

Should opacity remain constant?

If opacity approximates integrated density along support, stretching geometry may change optical thickness.

Standard 3DGS opacity is learned appearance, not a physically conserved density variable.

So there is no universally correct physical rule such as:

$$
\alpha'=\alpha.
$$

This reinforces:

$$
\boxed{
\text{appearance transport is not automatically physical transport}.
}
$$

---

**Full physical deformation may need re-optimization**

Affine propagation gives a strong first guess:

$$
\mu'\!,
\Sigma'\!.
$$

But after large edit, view-dependent appearance may no longer be consistent with captured observations.

A local optimization step could refine:

$$
SH,
\alpha,
\Sigma
$$

while preserving unchanged regions.

That is exactly where local repair / constrained re-optimization becomes interesting.

---

**The rendering pipeline gives you the support for verification**

Because projected splat support is explicit, an edit affecting subset:

$$
S
$$

has a computable image footprint.

Each changed Gaussian projects to ellipse region.

Union:

$$
\Omega_S
=
\bigcup_{g\in S}
support(g).
$$

Pixels outside:

$$
\Omega_S
$$

should ideally remain unchanged except for indirect ordering/transmittance effects induced by changed splats.

Maveb's image revision certificate takes this idea further by bounding compositing effects.

This is a compelling research connection:

$$
\boxed{
\text{rendering mathematics}
\rightarrow
\text{edit locality certificate}.
}
$$

---

**Why alpha makes verification harder**

Even if one changed splat occupies a small region, its alpha can reveal/hide unchanged background layers behind it.

So image change is not simply:

$$
\Delta C=
\Delta C_{edited}.
$$

It also includes:

$$
\Delta T
\times
C_{background/unchanged}.
$$

That is why Maveb's certificate includes background visibility/transmittance effects.

This is mathematically necessary.

---

<!-- BEGIN REVIEWED CERTIFICATE CLARIFICATION -->

### An image-change certificate must name the edits it covers

The retained source proposes an approximate bound using total opacity variation. That idea needs an explicit domain before it can become a certificate. A **certificate** here is a numerical upper bound justified for a declared renderer, input range and class of edits. Calling an estimate conservative does not establish that every permitted edit satisfies it. In particular, a bound depending only on opacity changes cannot cover arbitrary appearance changes.

Start with a counterexample. One layer has opacity one half over black. Change its color from red to blue without changing its opacity. The pixel changes from $(0.5,0,0)$ to $(0,0,0.5)$, giving a maximum-channel difference of $0.5$. Total opacity change is zero. An opacity-only bound predicts zero, or merely its small termination allowance, and fails. Likewise, swap the order of two half-opaque red and blue layers: the image changes from $(0.5,0,0.25)$ to $(0.25,0,0.5)$ while each layer's own opacity and color remain unchanged. Depth order is part of the compositing function.

A correct starting case fixes the contributor order and varies only their opacities. Assume every opacity lies in $[0,1]$, every color channel and the background lie in $[0,M]$, and the renderer uses the same ordered layers before and after the edit. The scalar $M$ is a proven common channel bound for this comparison, not an automatically valid value of one for HDR data. Background composition makes the pixel a convex combination of these colors, so it also lies in $[0,M]$ per channel.

Hold every parameter except the opacity of layer $i$ fixed. Write the pixel as the accumulated foreground $F$, followed by foreground transmittance $T_i$, this layer and the fully composited remainder $B_i$:

$$
C=F+T_i\bigl(\alpha_i c_i+(1-\alpha_i)B_i\bigr).
$$

Changing this opacity from $\alpha_i$ to $\widetilde\alpha_i$ changes the pixel by

$$
\Delta C=T_i(\widetilde\alpha_i-\alpha_i)(c_i-B_i).
$$

Because $T_i\le1$ and both $c_i$ and $B_i$ are within the same nonnegative range, the maximum absolute channel change is at most $M|\Delta\alpha_i|$. Replace all opacities one at a time and apply the triangle inequality. This gives a deliberately loose but justified bound:

$$
\|\widetilde C-C\|_\infty
\le M\min\left(1,\sum_i|\widetilde\alpha_i-\alpha_i|\right).
$$

The sum is the sum of absolute changes in each layer's **evaluated pixel opacity**, not the difference of final accumulated opacity. Two layer changes can cancel in a final opacity statistic while changing the color. Changing a Gaussian's mean or covariance changes evaluated opacity across its footprint even when its stored base opacity stays fixed. Comparing only the stored opacity parameter misses those spatial changes.

Now allow colors and background to change too, while keeping a common contributor order and a common $[0,M]$ range. With fixed opacities, each layer's color has a nonnegative compositing weight no greater than one. Bounding those weights by one, and again using a sequence of parameter replacements, yields

$$
\|\widetilde C-C\|_\infty
\le\min\left(
 M,
 M\sum_i|\Delta\alpha_i|
 +\sum_i\|\Delta c_i\|_\infty
 +\|\Delta C_{bg}\|_\infty
\right).
$$

This is sufficient rather than tight. A sharper certificate can retain transmittance weights and local color contrasts, but must justify them over the full edit envelope. If channels may be negative and only $\|c\|_\infty\le M$ is known, two allowed colors can differ by $2M$. The nonnegative-range contrast argument and its cap of $M$ no longer apply. Declare the actual radiance representation before selecting constants.

Early termination adds another approximation. If a renderer stops with remaining transmittance below $\tau$ and replaces the remainder by its declared background, its per-channel deviation from full compositing is at most $M\tau$ under the nonnegative range assumptions. Comparing two independently terminated images therefore adds at most $M(\tau_{old}+\tau_{new})$ to the exact-image bound. For equal cutoffs $10^{-3}$ this allowance is $0.002M$; for equal cutoffs $10^{-4}$ it is $0.0002M$. These constants follow from the cutoff policy. They do not cover omitted color edits, sorting changes, non-finite inputs, or a different fallback color range.

A support change can sometimes be represented by giving an absent layer zero opacity in a common ordering. A genuine reversal of two surviving layers cannot be justified by merely comparing their original opacity arrays. Either prove the ordering stays fixed over the edit, include a separately derived ordering contribution, or compute a broader envelope over all allowed orderings. The red/blue swap is a small mandatory test for any implementation claiming to handle reordering.

Finally compare the observed per-pixel error with the bound and preserve the tested inputs. A numerical witness can falsify an invalid formula and check arithmetic in a declared case; it does not prove every implementation meets the mathematical assumptions. The source's statements about Maveb certificates remain implementation claims awaiting repository inspection. The derivation here supplies a checkable contract for such an inspection, rather than certifying unseen code.


#### Check which transmittance the stopping test actually uses

The residual bound above assumes that the **actual unprocessed remainder** has transmittance below the stated cutoff. A test on the next candidate value is different. In the inspected [Graphdeco forward CUDA kernel](https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu), the code computes a candidate transmittance, checks it against the cutoff, and can stop before accumulating that candidate layer or assigning the candidate transmittance. The stored transmittance can therefore remain above the cutoff. Reading the numeric constant alone does not establish the omitted-energy bound.

For example, let current transmittance be $T=0.009$ and the next opacity be $0.99$. The candidate is $0.00009$, below $10^{-4}$. If this layer is skipped when that test succeeds, a red layer over black loses a contribution of $0.00891$, which is much larger than $10^{-4}$. This is an arithmetic counterexample to applying the after-accumulation remainder bound to a before-accumulation stopping policy; it is not a measured artifact in a trained scene.

If candidate-based stopping uses $T(1-\alpha)<\tau$ and a proven opacity cap $\alpha\le a_{max}<1$, then the discarded current remainder instead satisfies

$$
T<\frac{\tau}{1-a_{max}}.
$$

With $a_{max}=0.99$, that worst-case bound is one hundred times the cutoff. It is loose but follows from the actual branch condition. A tighter implementation-specific bound must inspect the actual discarded layer and remainder. Footprint truncation and small-alpha rejection are additional approximation policies; they are not automatically covered by an early-termination allowance. Define whether the comparison target is an ideal untruncated compositor or the same renderer with a particular stop rule disabled.


<!-- END REVIEWED CERTIFICATE CLARIFICATION -->

**A useful per-pixel edit bound**

If changed layer opacity difference produces total alpha variation:

$$
\Delta A_p,
$$

and all possible colors are bounded by:

$$
\|C\|_\infty
\le
C_{max},
$$

then conservative pixel color difference can be bounded approximately by:

$$
\boxed{
\|\Delta C_p\|_\infty
\le
C_{max}
\min(
1,
\Delta A_p+\epsilon_{termination}
).
}
$$

Maveb's certificate implements this kind of reasoning.

That is much stronger than merely computing empirical RMSE after the fact.

---

**Empirical metrics versus certificates**

RMSE says:

> What actually differed in this render?

Certificate says:

> How much was allowed to differ under the mathematical edit envelope?

These solve different problems.

For research evidence you want both.

Empirical:

$$
RMSE,
PSNR,
maxError,
changedPixelFraction.
$$

Analytical/conservative:

$$
perPixelBound.
$$

Then verify:

$$
actualError_p
\le
certifiedBound_p.
$$

That becomes real evidence for locality preservation.

---

**GPU sorting complexity**

Let:

$$
M
=
\text{total tile references}.
$$

Projection:

$$
O(N).
$$

Scan:

$$
O(N).
$$

Reference generation:

$$
O(M).
$$

Fixed-pass radix sort:

$$
O(M).
$$

Range construction:

$$
O(M)
$$

in current simple serial range scan, though it can be parallelized.

Composite work:

$$
O(
\text{pixels}
\times
\text{average relevant splats examined}
).
$$

The critical growth variable is often:

$$
\boxed{M}
$$

rather than Gaussian count alone.

---

**Why enormous projected Gaussians are dangerous**

One Gaussian touching full 4K screen:

$$
3840/16=240
$$

tiles wide.

$$
2160/16=135
$$

tiles high.

References:

$$
240\times135
=
32,400.
$$

One primitive.

Thousands of huge Gaussians can explode reference buffer.

Therefore performance diagnostics should include:

$$
tileEntries,
$$

$$
maximumSplatsPerTile,
$$

$$
overflowedEntries.
$$

Maveb tracks the first/overflow statistics.

Vulkax tracks maximum splats per tile in its scheduling result.

These are much more informative than only “Gaussian count.”

---

**Tile occupancy distribution**

Average tile occupancy:

$$
\bar K
=
\frac{
M
}{
N_{tiles}
}.
$$

But average can hide hotspots.

Need:

$$
K_{max}
$$

and ideally histogram/percentiles.

A dense foreground region may have:

$$
K=2000
$$

while most tiles have:

$$
K=20.
$$

Those hot tiles dominate compute time because every pixel in tile loops long range.

Maveb's tile occupancy debug view is therefore performance visualization, not merely debugging candy.

---

**Early termination changes effective workload**

Raw tile occupancy:

$$
K_t.
$$

Actual processed splats for pixel $p$:

$$
K_p^{effective}
\le
K_t.
$$

Dense opaque scenes may terminate early after few splats.

Transparent/fog-like scenes may process nearly whole tile list.

So performance depends on both:

$$
\boxed{
\text{geometric overlap}
}
$$

and:

$$
\boxed{
\text{opacity distribution}.
}
$$

This is why early-termination counter is useful.

---

**Sorting cost versus compositing cost**

If Gaussians are tiny:

Reference count $M$ modest.

Sort cheap.

If splats huge:

$M$ explodes.

Sort dominates.

If tiles have huge depth complexity:

Composite dominates.

Profiling should separately measure:

```text id="8f90ti"
projection
scan
reference expansion
sort
range build
composite
```

not one single “Gaussian ms”.

---

**One possible optimization: tighter bounds**

Maveb currently uses:

$$
r=
3\sqrt{\lambda_{max}}
$$

for a square bound.

As Pass 15 showed, tighter axis-aligned extents:

$$
r_x=
3\sqrt{\Sigma_{xx}},
$$

$$
r_y=
3\sqrt{\Sigma_{yy}}
$$

can reduce false-positive tiles.

For extremely anisotropic rotated Gaussians this can substantially reduce:

$$
M.
$$

You still evaluate true conic at pixel level.

This is a plausible low-risk optimization if validated against oracle.

---

**Even tighter ellipse-tile intersection**

A rectangle derived from $r_x,r_y$ still includes tiles that only touch AABB corners outside ellipse.

You could test tile rectangle against conic ellipse.

But test cost may exceed saved work.

Optimization decision becomes:

$$
\boxed{
\text{extra projection/binning math}
\quad vs\quad
\text{fewer references/composite work}.
}
$$

Measure on real captured scenes.

---

**Depth key subtlety: float ordering assumptions**

Maveb uses raw positive float bits.

Safe while:

$$
depth>0
$$

and finite.

NaN must never enter.

Negative depth must be culled.

If future renderer uses reversed camera sign:

$$
-Z
$$

or transformed depth mapping, key generation must change.

This is a hidden contract between projection and sorter.

Document it.

---

**Depth center is only an ordering approximation**

A large elongated Gaussian occupies a 3D volume.

Its center depth:

$$
z_i
$$

is used for entire splat.

Two overlapping 3D Gaussians may geometrically interpenetrate such that a single global center-depth order is not correct for every pixel.

3DGS rasterizers generally accept this approximate visibility model.

This is one reason “correct alpha order” means:

$$
\boxed{
\text{correct according to chosen Gaussian depth approximation}
}
$$

not exact volumetric radiative transport.

Important conceptual limitation.

---

**Stable order is also provenance**

If equal-depth splats reorder nondeterministically:

Output differences may appear even when no scene edit happened.

That destroys:

- reproducible screenshots,
- local repair certificates,
- regression metrics,
- publication evidence.

So stable radix sorting is not only visual quality.

For Maveb's research objectives:

$$
\boxed{
\text{deterministic rendering is part of experimental validity}.
}
$$

---

**Debug IDs should eventually be stable IDs**

Current Maveb GPU source ID:

$$
sourceIndex+1.
$$

Useful for a static canonical vector.

For long-lived editable-world diagnostics, stronger architecture is:

$$
\boxed{
\text{stable Gaussian ID}
}
$$

written to ID buffer.

Then pixel evidence remains meaningful after compaction/reorder.

That would align Maveb rendering diagnostics with Vulkax's stable-ID model.

---

**32-bit ID limitation**

If stable ID is 64-bit:

$$
(namespace,local)
$$

but ID target is:

$$
R32Uint,
$$

you cannot losslessly store entire packed 64-bit identity.

Options include:

$$
RG32Uint
$$

or renderer-local 32-bit indirection:

$$
PixelID
\rightarrow
StableGaussianID.
$$

The latter may be sufficient if mapping remains immutable for a captured revision.

This is an engine ABI design question.

---

**The complete physical coupling loop**

A truly coupled world can be viewed as:

$$
\text{Captured Gaussian field}
$$

$$
\downarrow
$$

$$
\text{Semantic ownership}
$$

$$
\downarrow
$$

$$
\text{MLS / correspondence embedding}
$$

$$
\downarrow
$$

$$
\text{Physical simulation}
$$

$$
\downarrow
$$

$$
F,t
$$

$$
\downarrow
$$

$$
\mu'=F\mu+t
$$

$$
\Sigma'=F\Sigma F^T
$$

$$
\downarrow
$$

$$
\text{updated 3DGS field}
$$

$$
\downarrow
$$

$$
\text{projection/sort/composite}
$$

$$
\downarrow
$$

$$
\text{verification}.
$$

And forces can travel backwards:

$$
\text{Gaussian interaction}
\rightarrow
\text{support-weight force transfer}
\rightarrow
\text{physics}.
$$

That bidirectionality is the real appearance↔physics bridge.

---

**A crucial distinction: renderer stable order vs world stable identity**

These are different concepts.

Stable sort means:

> equal sorting keys preserve previous reference order.

Stable ID means:

> primitive identity survives storage reorder/filter.

Do not confuse them.

You need both.

Stable sort gives deterministic rasterization.

Stable IDs give persistent editing semantics.

---

**Maveb's current strongest design aspects**

Its Gaussian subsystem already has unusually good separations.

Canonical imported values remain in trained domains.

CPU oracle is deterministic and independent enough to be useful.

Metal projection is mathematically aligned with the oracle.

Tile reference capacity is explicitly bounded.

Optimized scan/radix paths retain deterministic fallbacks.

GPU debug outputs expose real production pipeline data.

Persistent edits journal versions rather than rewriting every frame-slot blindly.

Entity ownership and spatial indexing limit local edits.

Image certificates reason about how alpha changes affect visible pixels.

These are all good foundations for a serious research renderer.

---

**Current gaps worth remembering**

The canonical Maveb Gaussian itself still uses source indices rather than built-in persistent per-Gaussian IDs.

The world ownership mapping is currently hard/index-based.

Arbitrary deformable covariance transport is more developed in Vulkax's MLS path than in Maveb's simple entity transform edits.

SH directional appearance under physical rotation/deformation is not automatically solved by transforming covariance.

Opacity has no universally valid physical conservation law.

Gaussian birth/death under topology change remains an open semantic operation.

These are not “bugs” in the ordinary sense.

They are boundaries between an appearance renderer and a genuinely editable physical captured world.

---


Starting from one projected Gaussian, you should now be able to reconstruct why a scalable renderer cannot iterate every Gaussian for every pixel.

Derive:

$$
N_x=
\lceil W/T\rceil
$$

and:

$$
N_y=
\lceil H/T\rceil.
$$

From screen AABB derive:

$$
[t_{x0},t_{x1}]
\times
[t_{y0},t_{y1}].
$$

Then derive overlap count:

$$
K_i=
(t_{x1}-t_{x0}+1)
(t_{y1}-t_{y0}+1).
$$

Given counts:

$$
K_i,
$$

derive exclusive scan:

$$
O_i=
\sum_{j<i}K_j.
$$

Explain why:

$$
[O_i,O_i+K_i)
$$

gives race-free output interval.

Then explain why tile references create a sparse bipartite relation:

$$
Gaussian
\leftrightarrow
Tile.
$$

Explain why sorted `values` plus tile `ranges` is CSR-like.

Then explain Maveb's composite key.

Why does it store:

$$
(depthBits,tileID)
$$

but sort depth passes first and tile passes last?

Because LSD stable radix gives:

$$
\boxed{
tile\ primary,
depth\ secondary.
}
$$

Explain why raw positive IEEE float bits preserve depth order but negative/NaN values invalidate that assumption.

Then derive alpha noncommutativity with two colored layers.

Then derive front-to-back contribution:

$$
w_i=
T_i\alpha_i.
$$

Then derive:

$$
T_{i+1}
=
T_i(1-\alpha_i).
$$

Then show:

$$
T_n=
\prod_i(1-\alpha_i).
$$

Then show Maveb's accumulated-opacity version:

$$
A=1-T
$$

and:

$$
contribution=
(1-A)\alpha.
$$

Then explain why:

$$
A>0.999
$$

permits early termination.

Then explain why the 3σ test:

$$
q\le9
$$

prevents Gaussian `exp()` underflow in Maveb's evaluated path.

Then reconstruct the Metal dataflow:

$$
Project
\rightarrow
Scan
\rightarrow
GenerateKeys
\rightarrow
Radix
\rightarrow
BuildRanges
\rightarrow
Composite.
$$

Explain which buffer is produced and consumed at every edge.

Then explain why each stage living in an ordered Metal encoder plus tracked resources is synchronization, even though there is no Vulkan-style explicit barrier call.

Then explain why CPU reference globally stable-sorts and uses independent row bands instead of duplicating the exact GPU tile/radix algorithm.

A correctness oracle should not merely be the GPU algorithm rewritten in C++.

Then derive:

$$
RMSE,
$$

$$
PSNR,
$$

$$
E_{max},
$$

and:

$$
F_{changed}.
$$

Explain what each metric can hide that another reveals.

Then switch from rendering to world semantics.

Prove:

$$
\Sigma'
=
F\Sigma F^T.
$$

Prove affine-reproducing interpolation:

$$
\sum_iw_i=1,
$$

$$
\sum_iw_iX_i=X_g
$$

imply:

$$
\sum_iw_i(AX_i+b)
=
AX_g+b.
$$

Then use the same weights to prove force conservation:

$$
\sum_iw_if=f.
$$

Then use affine reproduction to derive torque preservation:

$$
\sum_i
x_i\times(w_if)
=
x_g\times f.
$$

Then explain why covariance is appearance support rather than inertia.

Explain why opacity is not mass density.

Explain why stable sort is not stable identity.

Explain why an index-based selection becomes invalid after filtering.

Then reconstruct the complete captured-world loop:

$$
\boxed{
PLY
\rightarrow
Gaussian field
\rightarrow
semantic entity correspondence
\rightarrow
physical support embedding
\rightarrow
physical deformation
\rightarrow
F\Sigma F^T
\rightarrow
updated Gaussian rendering
\rightarrow
image verification
}
$$

If you can reconstruct that chain from first principles, you understand not only 3D Gaussian Splatting, but why Gaussian captured worlds can become editable engine data.

---

### PHASE 25 VERDICT

**M145–M150, Parts 2401–2500 are now complete.**

The most important conceptual progression across the phase was:

$$
\boxed{
\text{Gaussian parameter}
\rightarrow
\text{3D covariance}
\rightarrow
\text{2D conic}
\rightarrow
\text{tile references}
\rightarrow
\text{ordered alpha contribution}
}
$$

for rendering, and independently:

$$
\boxed{
\text{Gaussian identity}
\rightarrow
\text{semantic entity}
\rightarrow
\text{physical support}
\rightarrow
\text{deformation}
\rightarrow
\text{verified local edit}
}
$$

for editable captured worlds.

That second chain is what transforms 3DGS from a novel-view-synthesis representation into the substrate of **MAVEB's “Repair What Matters” idea**: preserve the captured appearance field, modify only semantically and physically justified support, publish only the changed regions, and produce evidence that unchanged parts stayed unchanged.


This is the first of the two intentionally huge passes. It begins **Phase 26 — Advanced Real-Time Rendering**: many-light visibility as an indexing problem; forward, deferred, tiled and clustered lighting; logarithmic depth clusters and light/cluster intersection; GPU light lists and overflow; directional/spot/point shadows and cascades; PCF/PCSS/VSM/EVSM-style shadow filtering concepts; IBL/probes and real-time GI approximations; SSAO/GTAO-style occlusion; SSR and screen-space limitations; temporal reprojection, motion vectors, jitter, TAA history rejection and disocclusion; particles and GPU simulation; bloom, exposure, tone mapping, color grading and the complete post-processing chain—mapped back into Kairo/Maveb's actual clustered-lighting, shadow, TAA and bloom implementations.


---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/44 - Cross-backend conversion and agreement|Previous chapter]] · [[Chapter Edition/03 - Game Engine/46 - Game design from mechanics to narrative interfaces|Next chapter]]
