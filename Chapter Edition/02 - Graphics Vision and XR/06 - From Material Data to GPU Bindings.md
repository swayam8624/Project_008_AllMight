# 06 — From Material Data to GPU Bindings

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities|Next chapter]]

A material authored in an interchange format must be decoded, represented and bound for the GPU. This chapter follows the data path and its scaling choices while preserving the difference between source material values and shader behavior.

**By the end:** Trace material data from glTF through engine representation to GPU binding.

**Recall question:** Where can a material pipeline change meaning or cost as assets scale?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 9521–13371; source-span SHA256 7ac0016132c970bd4398a5fa530cea178cc43d6ec9fdb5509802da480e069a88. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Blank-page mastery test

From no notes, reconstruct:

$$
\frac{\rho}{\pi}
$$

from the hemisphere integral.

Then derive:

$$
F_0=
\left(
\frac{\eta_i-\eta_t}
{\eta_i+\eta_t}
\right)^2
$$

and calculate why IOR $1.5$ gives about $0.04$.

Then explain every symbol in:

$$
\boxed{
f_s=
\frac{
D(H)F(V,H)G(L,V)
}{
4(N\cdot L)(N\cdot V)
}
}
$$

without saying “because that's the PBR formula.”

Then reconstruct:

$$
k_d=(1-F)(1-m)
$$

and explain why both factors exist.

Finally derive refraction:

$$
T=
\eta I+
\left[
\eta(-N\cdot I)
-
\sqrt{
1-\eta^2
(1-(N\cdot I)^2)
}
\right]N
$$

and explain exactly when the square root becomes imaginary and why that means total internal reflection.

If you can do those five derivations, then M117/M118 have moved from **shader familiarity** into **graphics-model mastery**.





This closes **Phase 20**. M119 owns **Parts 1987–1995**: glTF PBR representation and texture transforms, GPU material layouts, std140/std430, padding, bindless resources, material sorting and shader specialization. M120 owns **Parts 1996–2000**: reference-sphere validation, systematic PBR failure diagnosis, artistic versus physical material models, implementing the complete GGX metallic-roughness BRDF, and being able to defend every term and know when that model is the wrong one.

The transition from Pass 4 is:

$$
\boxed{
\text{we know the BRDF}
}
$$

to:

$$
\boxed{
\text{how do real assets encode it, how does CPU memory reach GPU memory correctly, how do thousands of materials scale, and how do we prove the resulting renderer is actually correct?}
}
$$

This is where “I can write a PBR shader” becomes “I can build and validate a PBR material system.”

---

### PARTS 1987–1995
### glTF → GPU material representation → scalable binding architecture

---

**Why glTF matters**

A renderer does not normally receive:

```text
red metal with roughness 0.27
```

directly from the artist.

The data passes through an asset format.

For modern real-time graphics, glTF 2.0 is one of the most important interchange formats.

Its core material model is **metallic-roughness PBR**. The format deliberately stores declarative material parameters while allowing the renderer's exact BRDF implementation some flexibility. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

That means glTF says approximately:

$$
\text{this material has these semantic inputs}
$$

rather than:

$$
\text{execute this shader source}.
$$

That is a very healthy architecture.

---

**Core glTF metallic-roughness representation**

A simplified material might look conceptually like:

```json
{
  "pbrMetallicRoughness": {
    "baseColorFactor": [1.0, 0.766, 0.336, 1.0],
    "metallicFactor": 1.0,
    "roughnessFactor": 0.25
  }
}
```

The important inputs are:

$$
C_{\text{base}},
\qquad
m,
\qquad
r.
$$

Defaults in core glTF are:

$$
C_{\text{base}}=(1,1,1,1),
$$

$$
m=1,
$$

$$
r=1.
$$

The material may additionally contain:

$$
\text{normalTexture},
$$

$$
\text{occlusionTexture},
$$

$$
\text{emissiveTexture},
$$

$$
\text{emissiveFactor},
$$

$$
\text{alphaMode},
$$

$$
\text{alphaCutoff},
$$

$$
\text{doubleSided}.
$$

Kairo's current persistent `MaterialArtifactData` already tracks essentially this same core set.

---

**Factor × texture semantics**

glTF generally treats factors and texture values multiplicatively.

For base color:

$$
\boxed{
C_{\text{base,final}}
=
C_{\text{base,factor}}
\odot
C_{\text{base,texture}}
}
$$

where $\odot$ denotes component-wise multiplication.

For roughness:

$$
\boxed{
r=
r_{\text{factor}}
r_{\text{texture}}
}
$$

and metallic:

$$
\boxed{
m=
m_{\text{factor}}
m_{\text{texture}}.
}
$$

If the texture is missing, the texture contribution behaves as 1.

That is why a fallback white texture is such a convenient GPU implementation:

$$
1\times\text{factor}
=
\text{factor}.
$$

Current Kairo does precisely that for several absent material channels.

---

**glTF color-space semantics are part of the material contract**

From Pass 3:

Base color texture RGB is sRGB-encoded and must be converted to linear light before lighting.

Metallic and roughness are data and must remain linear.

Core glTF explicitly requires base-color RGB to use sRGB encoding, while the metallic-roughness texture is linear. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

Therefore:

$$
\boxed{
\text{same PNG container}
\neq
\text{same numerical interpretation}.
}
$$

This is why Kairo's resolver takes:

```cpp
TextureSemantic
```

rather than treating every material texture identically.

---

**Metallic-roughness channel packing**

glTF packs two material parameters into one texture:

$$
G=\text{roughness},
$$

$$
B=\text{metallic}.
$$

The red and alpha channels are ignored for metallic-roughness calculations. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

So Kairo's current fragment shader correctly does:

```glsl
vec4 metallicRoughness =
    texture(metallicRoughnessMap, inTexCoord);

float metallic =
    material.factors.x *
    metallicRoughness.b;

float roughness =
    material.factors.y *
    metallicRoughness.g;
```

Note the order:

$$
\boxed{
G=\text{roughness},\qquad B=\text{metallic}.
}
$$

Swapping these is a classic glTF importer bug.

---

**Why texture packing is useful**

Suppose metallic and roughness were two independent R8 images.

Each sampling operation potentially means:

- another descriptor,
- another texture lookup,
- another memory access/cache footprint.

Packing them into channels allows:

$$
\text{one sample}
\rightarrow
\text{two parameters}.
$$

This is especially attractive because material maps often share the same UV parameterization.

A common engine-specific packing goes even further:

```text
R = AO
G = roughness
B = metallic
```

often called ORM.

Core glTF does not formally define metallic-roughness R as AO, though AO images can share texture storage in authoring pipelines if the separate glTF texture references point to the same image appropriately.

---

**Base color semantics change with metallic again**

From Pass 4:

If:

$$
m=0,
$$

base color approximates diffuse/body reflectance.

If:

$$
m=1,
$$

base color approximates conductor $F_0$.

The glTF specification explicitly follows this interpretation and uses:

$$
F_{0,\text{dielectric}}=0.04.
$$

Its reference model simplifies to:

$$
c_{\text{diff}}
=
\operatorname{lerp}(C_{\text{base}},0,m)
$$

and:

$$
F_0=
\operatorname{lerp}(0.04,C_{\text{base}},m).
$$ 
<!-- Original citation marker: :chatgpt-content-reference{index="4"} -->



That is exactly the model Kairo is implementing.

---

**glTF alpha is straight, not premultiplied**

The glTF base-color texture's stored RGB is not premultiplied by alpha. 
<!-- Original citation marker: :chatgpt-content-reference{index="5"} -->


Therefore the imported texture represents:

$$
(C,\alpha)
$$

not:

$$
(\alpha C,\alpha).
$$

That matches Kairo's current transparent blend state:

$$
C_o
=
\alpha_sC_s+
(1-\alpha_s)C_d.
$$

If you converted glTF textures to premultiplied alpha during import without changing the blend model, you would darken edges by effectively multiplying alpha twice.

---

**Vertex color multiplies base color too**

If glTF primitive data includes:

$$
\texttt{COLOR_0},
$$

the resulting color is another multiplier on the material base color. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

Kairo currently computes:

```glsl
baseColorAlpha =
    vec4(inColor, 1.0) *
    material.baseColorFactor *
    sampledBase;
```

Therefore:

$$
\boxed{
C_{\text{final}}
=
C_{\text{vertex}}
C_{\text{factor}}
C_{\text{texture}}
}
$$

which is correct for this interpretation.

---

**Normal texture semantic**

A glTF normal texture contains tangent-space normal data.

Its RGB is linear data, not sRGB color.

The sampled RGB is converted roughly via:

$$
N_t=2C-1.
$$

The normal scale affects X/Y components before normalization. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.pdf?utm_source=chatgpt.com)

Kairo already follows this:

```glsl
vec3 tangentNormal =
    texture(normalMap, inTexCoord).xyz *
    2.0 - 1.0;

tangentNormal.xy *= normalScale;
```

This is another place where the asset format and shader math line up cleanly.

---

**Occlusion strength nuance**

glTF defines occlusion strength such that:

$$
\boxed{
AO_{\text{final}}
=
1+
s(AO_{\text{sample}}-1)
}
$$

where $s\in[0,1]$. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?initialSessionID=262-7493612-7323838\&ld=ASXXSCTFRDirect\&utm_source=chatgpt.com)

Notice what this means.

If:

$$
s=0,
$$

then:

$$
AO_{\text{final}}=1.
$$

No occlusion effect.

If:

$$
s=1,
$$

then:

$$
AO_{\text{final}}
=
AO_{\text{sample}}.
$$

A simple multiplication:

$$
s\cdot AO_{\text{sample}}
$$

is **not the same equation**.

Current Kairo effectively does:

```glsl
ambientOcclusion =
    clamp(
        factor *
        texture(occlusionMap, uv).r,
        0.0,
        1.0);
```

That means Kairo's current `AmbientOcclusion` factor behaves as a scalar multiplier, not exactly as glTF's specified occlusion-strength interpolation.

This is a concrete glTF-fidelity mismatch.

For a sample:

$$
AO=0.3,
\quad
s=0.5,
$$

glTF says:

$$
AO_f=
1+0.5(0.3-1)
=
0.65.
$$

Kairo-style multiplication gives:

$$
0.15.
$$

That is dramatically darker.

So this should be fixed if strict glTF appearance matching is the target.

---

**Texture-coordinate sets**

glTF textures can choose which UV channel to use.

A texture info may reference:

$$
\texttt{TEXCOORD_0},
$$

$$
\texttt{TEXCOORD_1},
$$

and so forth.

Your `GltfTextureBinding` already stores:

```cpp
std::uint32_t TexCoord = 0;
```

which means the importer preserves the source UV-set selection.

But the current renderer material interface only exposes a single:

```text
inTexCoord
```

for the main shader path.

So the source artifact can represent multiple UV selections, but the real-time shader path currently does not expose arbitrary per-texture UV-set selection.

That is a real current architecture gap.

---

**KHR_texture_transform**

Texture coordinates often need more than “pick UV set 0 or 1.”

The Khronos `KHR_texture_transform` extension supports:

$$
\text{offset},
$$

$$
\text{rotation},
$$

$$
\text{scale},
$$

and optionally a `texCoord` override. [GitHub](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md?utm_source=chatgpt.com)

Conceptually:

$$
\boxed{
UV'
=
T_{\text{offset}}
R_{\theta}
S
UV
}
$$

using homogeneous 2D coordinates.

Let:

$$
p=
\begin{bmatrix}
u\\v\\1
\end{bmatrix}.
$$

Scale:

$$
S=
\begin{bmatrix}
s_u&0&0\\
0&s_v&0\\
0&0&1
\end{bmatrix}.
$$

Rotation:

$$
R=
\begin{bmatrix}
\cos\theta&-\sin\theta&0\\
\sin\theta&\cos\theta&0\\
0&0&1
\end{bmatrix}.
$$

Translation:

$$
T=
\begin{bmatrix}
1&0&o_u\\
0&1&o_v\\
0&0&1
\end{bmatrix}.
$$

Then:

$$
\boxed{
p'=TRSp.
}
$$

The extension explicitly describes this affine form. [GitHub](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md?utm_source=chatgpt.com)

---

**Why order matters in UV transforms**

Matrix multiplication is not commutative.

Usually:

$$
TRS
\neq
SRT.
$$

Consider UV:

$$
(0.2,0.4).
$$

If you scale first then offset:

$$
UV'=S UV+O.
$$

If you offset first:

$$
UV'=S(UV+O).
$$

The offset itself gets scaled.

So material importers must implement the extension's defined operation rather than applying “roughly the same transform.”

---

**Texture atlas interpretation**

Suppose a texture atlas is:

```text
+---------+---------+
| brick   | metal   |
+---------+---------+
| wood    | fabric  |
+---------+---------+
```

To access only the upper-right quadrant:

$$
scale=(0.5,0.5)
$$

and:

$$
offset=(0.5,0).
$$

Then:

$$
UV_{\text{atlas}}
=
0.5UV+
(0.5,0).
$$

Texture transforms therefore let many materials share one large texture allocation while remapping local UVs into subregions.

---

**Current Kairo texture-transform gap**

Your current `GltfTextureBinding` contains:

```cpp
Uri
MimeType
EmbeddedBytes
TexCoord
Scale
```

but that `Scale` is used for normal/occlusion strength semantics, not the full KHR texture transform.

It does not currently carry:

$$
\text{offset}[2],
$$

$$
\text{rotation},
$$

$$
\text{UV scale}[2].
$$

And your persistent `MaterialArtifactData` reduces texture slots to asset handles.

So `KHR_texture_transform` is currently not represented end-to-end.

A production extension would likely require per-texture metadata such as:

```cpp
struct TextureBinding
{
    TextureHandle Texture;

    uint32_t TexCoordSet;

    Vec2 Offset{0.0f, 0.0f};
    Vec2 Scale{1.0f, 1.0f};

    float Rotation = 0.0f;
};
```

Then the shader receives either:

$$
UV'
$$

already transformed by the vertex shader, or the parameters necessary to transform it per texture.

---

**Material sorting**

Before bindless rendering, the simplest optimization is to reorder draws to reduce expensive state changes.

Suppose scene submission order is:

```text
material A
material B
material A
material C
material B
material A
```

That may require six material switches.

Sort by material:

```text
A
A
A
B
B
C
```

Now only three material bindings.

Similarly, sort by:

$$
\text{pipeline},
$$

then:

$$
\text{material},
$$

then perhaps:

$$
\text{mesh}.
$$

---

**Draw sort keys**

A renderer can compress render state into a 64-bit key.

Conceptually:

```text
[ pass ]
[ pipeline ]
[ material ]
[ depth bucket ]
[ mesh ]
```

For example:

```cpp
uint64_t MakeOpaqueSortKey(
    uint8_t pass,
    uint16_t pipeline,
    uint16_t material,
    uint16_t depthBucket)
{
    return
        (uint64_t(pass)        << 56) |
        (uint64_t(pipeline)    << 40) |
        (uint64_t(material)    << 24) |
        uint64_t(depthBucket);
}
```

Sorting integer keys is extremely fast.

The exact bit allocation is engine-specific.

The key idea is:

$$
\boxed{
\text{render order becomes an explicit optimization policy}.
}
$$

---

**State sorting versus depth sorting**

Opaque rendering wants:

$$
\text{front-to-back}
$$

to improve early-Z.

But state optimization wants:

$$
\text{group identical material/pipeline}.
$$

These goals conflict.

Suppose sorting perfectly front-to-back creates:

$$
1000
$$

pipeline switches.

A material-first sort may produce only:

$$
10
$$

pipeline switches but more overdraw.

The best ordering depends on whether the scene is:

- CPU submission limited,
- fragment limited,
- pipeline-switch limited,
- overdraw limited.

A renderer often combines coarse depth buckets with state sorting.

---

**Transparent sorting is fundamentally different**

Transparent source-over generally requires:

$$
\text{back-to-front}.
$$

Therefore you cannot freely sort transparents by material without changing visual results.

Current Kairo handles this distinction:

Opaque/masked draws retain submission order.

Transparent draws are collected and stable-sorted by decreasing squared camera distance.

That is a visibility-correctness priority.

This means “sort all materials by pipeline” is not a universal optimization.

Render semantics come first.

---

**Current Kairo does not yet material-sort opaque draws**

Current opaque path effectively does:

```cpp
for each draw in submission order:
    if not Blend:
        drawOne(...)
```

Each draw performs:

```cpp
BindMaterial(...)
BindPipeline(...)
BindVertexBuffers(...)
BindIndexBuffer(...)
DrawIndexed(...)
```

So a scene alternating materials can cause repeated descriptor changes.

An immediate next optimization could group opaque draws by:

$$
\text{pipeline class}
\rightarrow
\text{material}
\rightarrow
\text{mesh/depth}.
$$

That should be benchmarked rather than assumed beneficial.

---

**Shader specialization**

Suppose your universal material shader contains:

```glsl
if (hasNormalMap) { ... }

if (hasClearcoat) { ... }

if (hasTransmission) { ... }

if (hasSheen) { ... }

if (isDoubleSided) { ... }
```

Advantages:

$$
\text{one shader/pipeline family}.
$$

Disadvantages:

- more branches,
- more live variables,
- more texture descriptors,
- register pressure,
- potentially divergent control flow.

Alternative:

compile variants:

```text
OPAQUE_NORMALMAP
OPAQUE_NO_NORMALMAP
TRANSMISSION
CLEARCOAT
...
```

Now work can be specialized.

But permutation count can explode.

---

**Specialization decisions belong on a spectrum**

Some properties fundamentally change the pipeline:

$$
\text{alpha blend vs opaque}
$$

because blend state and depth-write state differ.

This often deserves a distinct pipeline.

Some properties drastically alter shader structure:

$$
\text{transmission}
$$

or:

$$
\text{skin deformation}.
$$

These may justify variants.

Some are tiny:

$$
\text{baseColorFactor present}
$$

which should simply remain data.

The design question is:

$$
\boxed{
\text{when does removing runtime flexibility save enough GPU work to justify another compiled variant?}
}
$$

---

**Vulkan specialization constants**

Vulkan/SPIR-V also supports specialization constants.

Conceptually shader:

```glsl
layout(constant_id = 0)
const bool UseClearcoat = false;
```

At pipeline creation, the engine supplies:

$$
\texttt{UseClearcoat=true}.
$$

The shader compiler can often optimize away unreachable branches as though they were compile-time constants.

This offers a middle ground between:

- maintaining many source files,
- keeping every feature dynamic.

But each specialized value set still contributes to pipeline identity/compilation pressure.

---

**The dangerous shader permutation equation**

Suppose independent binary features are:

$$
n=15.
$$

Naively:

$$
2^{15}
=
32768
$$

variants.

If you add 5 roughness-model options:

$$
32768\times5
=
163840.
$$

And then:

- MSAA state,
- render-target format,
- vertex layout,
- shadow mode

can multiply pipeline count further.

This is why serious engines classify features instead of turning every checkbox into `#define`.

---

**Kairo's current specialization boundary**

Current Kairo already has meaningful hard variants:

$$
\text{static opaque},
$$

$$
\text{static transparent},
$$

$$
\text{skinned opaque},
$$

$$
\text{skinned transparent},
$$

$$
\text{shadow static},
$$

$$
\text{shadow skinned},
$$

$$
\text{debug line}.
$$

But normal mapping, metallic, roughness, AO and emissive remain one shared shader path.

That is a sensible small-engine boundary.

---

**Current descriptor architecture: precise audit**

Today, each Kairo draw gets a descriptor set containing approximately:

$$
\text{camera UBO},
$$

$$
\text{shadow sampler},
$$

five material textures,

$$
\text{material UBO},
$$

$$
\text{environment sampler},
$$

and optionally a skin UBO.

The descriptor pool is rebuilt when the material descriptors are marked dirty, and Kairo deliberately rebuilds at a fence-safe frame boundary.

This is correct and easy to reason about.

But it scales roughly with draw/material count rather than becoming one persistent global material/resource table.

That gives us the architectural progression:

```text
today:
per-draw descriptor sets

next:
deduplicate material descriptor sets

then:
material GPU array + material IDs

then:
large texture descriptor table

then:
GPU-generated draw + material IDs

eventually:
fully GPU-driven material submission
```

---

### PARTS 1996–2000
### How to prove a PBR renderer is actually correct

---

**“Looks good” is not validation**

Suppose your sphere looks shiny.

That proves almost nothing.

A wrong BRDF can look convincing under:

- one environment,
- one exposure,
- one camera,
- one material,
- one tone mapper.

You need controlled invariants.

M120 exists because PBR bugs can compensate for each other.

For example:

```text
forgot 1/pi
+
lights authored too dim
```

may look correct.

Or:

```text
roughness accidentally squared twice
+
environment mip selection too sharp
```

may look plausible.

Validation must isolate terms.

---

**Reference sphere matrix**

The standard diagnostic scene is a grid of spheres.

Columns:

$$
m=0\rightarrow1.
$$

Rows:

$$
r=0.05\rightarrow1.
$$

Or more commonly, separate dielectric and metal grids.

For example:

| Sphere | Metallic | Roughness |
|---|---:|---:|
| A | 0 | 0.05 |
| B | 0 | 0.25 |
| C | 0 | 0.50 |
| D | 0 | 1.00 |
| E | 1 | 0.05 |
| F | 1 | 0.25 |
| G | 1 | 0.50 |
| H | 1 | 1.00 |

Use identical:

- camera,
- geometry,
- light,
- exposure,
- tone mapper.

Only one parameter changes.

Then visual changes have interpretable causes.

---

**Why spheres are useful**

A sphere contains a continuum of surface normals.

Across one object, you see:

$$
N\cdot L
$$

from nearly:

$$
0
$$

to:

$$
1,
$$

and:

$$
N\cdot V
$$

across many angles.

So one sphere tests:

- normal incidence,
- grazing incidence,
- highlight width,
- Fresnel growth,
- roughness behavior.

A flat plane cannot expose all that at once.

---

**But spheres alone are insufficient**

A sphere does not thoroughly expose:

- tangent seams,
- mirrored UV behavior,
- thin geometry,
- alpha coverage,
- anisotropic footprints,
- texture transforms,
- transmission thickness.

So a proper validation suite also needs:

```text
sphere
plane
UV-seamed mesh
mirrored-UV mesh
normal-map test
metal/roughness ramp
emissive patch
glass slab
thick glass object
```

Conceptually.

The principle is:

$$
\boxed{
\text{one canonical scene per invariant}.
}
$$

---

**Numerical reference tests**

Do not validate only images.

Test the BRDF functions directly.

For example, Fresnel:

$$
F(1)=F_0.
$$

At grazing:

$$
F(0)=1.
$$

Therefore:

```cpp
TEST(FresnelNormalIncidence)
{
    Vec3 F0{0.04f};

    Vec3 F =
        FresnelSchlick(
            1.0f,
            F0);

    EXPECT_NEAR(F.x, 0.04f);
}
```

And:

```cpp
TEST(FresnelGrazing)
{
    Vec3 F =
        FresnelSchlick(
            0.0f,
            Vec3{0.04f});

    EXPECT_NEAR(F.x, 1.0f);
}
```

These are much more diagnostic than screenshot tests.

---

**GGX invariants**

For valid parameters:

$$
D\ge0.
$$

Likewise:

$$
0\le F\le1
$$

for ordinary reflectance input.

And:

$$
0\le G\le1.
$$

The resulting shader should remain finite:

$$
\neg\operatorname{isnan}(f),
$$

$$
\neg\operatorname{isinf}(f).
$$

Randomized tests can sample millions of valid directions and check these properties.

That catches singularities far more effectively than a few screenshots.

---

**BRDF reciprocity numerical test**

Generate random upper-hemisphere directions:

$$
L,V.
$$

Evaluate:

$$
f(L,V)
$$

and:

$$
f(V,L).
$$

For an isotropic reciprocal BRDF they should be approximately equal:

$$
\boxed{
f(L,V)
\approx
f(V,L).
}
$$

Within floating-point tolerance.

If not, something is wrong in:

- half-vector construction,
- geometry function,
- directional convention,
- dot products.

---

**Energy-conservation integration test**

For fixed incident $L$, numerically integrate:

$$
R=
\int_{\Omega^+}
f_r(L,V)
(N\cdot V)
d\omega_V.
$$

Then require:

$$
R\le1+\epsilon.
$$

Monte Carlo estimate:

$$
R
\approx
\frac1N
\sum_{k=1}^N
\frac{
f(L,V_k)
(N\cdot V_k)
}{
p(V_k)
}.
$$

This directly tests the physical constraint.

We will derive this estimator formally in Phase 21.

---

**Current Kairo testing gap**

Your current ray-tracer tests do have a PBR smoke test:

```text
PBR image renders finite pixels
```

and deterministic-render tests.

These are valuable for regression stability.

But they do not yet constitute a **reference-sphere BRDF validation suite**.

They verify approximately:

$$
\text{renderer executes},
$$

$$
\text{pixels are finite},
$$

$$
\text{image is nonblack}.
$$

They do not prove:

$$
\text{Fresnel is physically correct},
$$

$$
\text{roughness mapping is correct},
$$

$$
\text{energy is conserved},
$$

$$
\text{real-time and ray-traced material semantics match}.
$$

M120's next logical repository contribution is exactly such a test suite.

---

**Common PBR bug: wrong gamma**

Suppose base color stored sRGB:

$$
0.5.
$$

Correct linear value:

$$
\approx0.214.
$$

If you treat 0.5 as linear:

$$
\text{surface albedo}
$$

is more than twice as large as intended.

Symptoms:

- midtones too bright,
- diffuse materials washed out,
- material references don't match DCC tools.

If you decode twice:

$$
0.214
\rightarrow
\text{another nonlinear decode}
$$

the material becomes severely too dark.

The debugging question is not:

> “Does this texture look too dark?”

It is:

$$
\boxed{
\text{What numerical color space is this resource in at every boundary?}
}
$$

---

**Gamma diagnostic**

Use a base-color texture containing:

$$
0.5_{\text{sRGB}}.
$$

Expected linear sample:

$$
\approx0.214.
$$

Add a viewport debug mode that directly outputs sampled linear value into a float capture.

If you see:

$$
0.5,
$$

sRGB decode is missing.

If you see around:

$$
0.038,
$$

you likely decoded twice.

This turns “something is dark” into a numeric test.

---

**Common PBR bug: normal map treated as color**

Neutral tangent normal:

$$
(0.5,0.5,1)
$$

should decode to:

$$
(0,0,1).
$$

If sRGB decode runs first:

$$
0.5\rightarrow0.214
$$

therefore:

$$
2(0.214)-1=-0.572.
$$

The decoded normal becomes approximately:

$$
(-0.572,-0.572,1)
$$

before normalization.

Symptoms:

- entire surface appears tilted,
- lighting changes strangely across UV islands,
- neutral normal texture is not neutral.

Kairo's texture semantics currently prevent this by requiring normal maps to be linear.

---

**Common PBR bug: forgetting normalization**

After interpolation:

$$
\|N\|\neq1.
$$

After tangent-space scaling:

$$
\|N_t\|\neq1.
$$

After TBN transformation:

$$
\|N_w\|
$$

may drift.

Then:

$$
N\cdot L
$$

no longer equals cosine.

A dot product of nonunit vectors mixes angle with vector length.

Symptoms include:

- too bright highlights,
- strange terminator behavior,
- view-dependent intensity changes.

Normalize aggressively at semantic boundaries.

---

**Common PBR bug: flipped green normal channel**

Different normal-map conventions may interpret tangent-space Y differently.

One map may represent:

$$
+Y
$$

where another renderer expects:

$$
-Y.
$$

Symptom:

bumps appear as dents and vice versa along one axis.

A diagnostic normal map with a known directional slope is far better than random stone texture.

---

**Common PBR bug: tangent-space disagreement**

Even if normal RGB is correct, the tangent basis used by the renderer may disagree with the baker.

Typical symptom:

- seams around mirrored UV islands,
- lighting discontinuities across triangles,
- baked detail shifting around the mesh.

The solution is not “normalize harder.”

The tangent-generation convention must match.

For glTF assets this is often one reason to preserve provided tangent data.

Your current artifact can store tangent direction + handedness, but the Vulkan mesh path previously inspected reconstructs basis derivatives in the fragment shader.

That creates potential baker/render tangent mismatch.

---

**Common roughness bug: channel swap**

glTF:

$$
G=\text{roughness},
$$

$$
B=\text{metallic}.
$$

If swapped, a material whose metallic blue channel is 1 suddenly becomes roughness 1.

Symptoms can be spectacularly confusing:

- metals become matte,
- dielectric regions become metallic,
- changing one texture channel seems to affect unrelated properties.

Always build a diagnostic texture:

```text
G gradient
B constant
```

then invert it.

---

**Common roughness bug: wrong domain**

Artist parameter:

$$
r.
$$

Microfacet slope parameter:

$$
\alpha=r^2.
$$

GGX then uses:

$$
\alpha^2.
$$

If shader accidentally interprets input texture as already-$\alpha$ and squares again, effective response becomes:

$$
r^4
$$

too early.

If an implementation expected:

$$
\alpha=r^2
$$

but received raw $r$, highlights become substantially different.

The only cure is explicit naming:

```cpp
perceptualRoughness
alpha
alphaSquared
```

not three variables all called `roughness`.

---

**Roughness ramp test**

Render one row of spheres with:

$$
r=
0.05,\,
0.1,\,
0.2,\,
0.4,\,
0.6,\,
0.8,\,
1.0.
$$

Expected qualitative behavior:

highlight width increases monotonically.

Peak intensity generally decreases as energy spreads.

Fresnel at corresponding local angle should not mysteriously change because $F_0$ is independent of roughness.

If roughness changes overall base color dramatically, something else is coupled incorrectly.

---

**Common $F_0$ bug: using base color for every material**

Suppose red plastic:

$$
C_{\text{base}}=(0.8,0.05,0.05)
$$

and:

$$
m=0.
$$

Incorrect:

$$
F_0=C_{\text{base}}.
$$

Now its specular highlight becomes bright red.

But dielectric specular reflection should mostly reflect the illuminant/interface color, approximately neutral for ordinary plastic.

Correct:

$$
F_0\approx(0.04,0.04,0.04).
$$

Diffuse carries the red.

For metal:

$$
m=1,
$$

base color *does* become colored $F_0$.

---

**Common $F_0$ bug: 0.04 interpreted as sRGB**

The physical constant:

$$
0.04
$$

is a **linear reflectance**.

If you encode or decode it incorrectly through sRGB, its numerical value changes.

That would change material IOR-equivalent behavior.

Shader constants representing physical reflectance should live in linear space.

---

**Common energy bug**

Incorrect:

$$
f=
\frac{C}{\pi}
+
f_s.
$$

Correct dielectric energy split is closer to:

$$
f=
(1-F)
\frac{C}{\pi}
+
f_s.
$$

For metallic workflow:

$$
f=
(1-F)(1-m)
\frac{C}{\pi}
+
f_s.
$$

As noted in Pass 4, your current real-time Kairo shader uses the energy split, but current ray-tracer direct PBR uses:

$$
(1-m)C/\pi
$$

without multiplying diffuse by $(1-F)$.

That means the two renderers do not currently implement exactly the same BRDF.

A cross-renderer reference sphere test should expose that.

---

**Common environment-lighting bug**

A material looks correct under direct point light but wrong under environment lighting.

That usually means direct BRDF and IBL use inconsistent models.

For GGX IBL you generally need an environment prefiltered according to roughness and a BRDF integration term.

Simply sampling an ordinary mipmapped environment at:

$$
LOD=r\cdot L_{\max}
$$

is an approximation, which is what current Kairo does.

So if direct-light reference spheres match but environment reference spheres do not, the BRDF may be correct while the IBL integration is approximate.

Do not “fix” GGX to compensate for an IBL shortcut.

---

**Common tone-mapping bug masquerading as PBR error**

Suppose rough metallic highlights clip heavily.

You may conclude:

> specular BRDF is too bright.

But the actual BRDF may be correct and the tone mapper may be crushing highlights.

Therefore validation should capture:

$$
\text{pre-exposure scene-linear HDR}
$$

when possible.

Compare BRDF numerically **before tone mapping**.

Current Kairo tone maps inside `triangle.frag`, which makes this harder.

That is another reason to move tone mapping into a later post pass.

---

**Artistic material models versus physical models**

A physically based model tries to preserve constraints such as:

$$
\text{energy conservation},
$$

$$
\text{reciprocity},
$$

$$
\text{consistent response across lighting}.
$$

An artistic model prioritizes direct control.

For example:

```text
specularPower = 128
specularColor = pink
rimLight = blue
```

may look excellent for a stylized game.

It is not automatically inferior.

The key distinction is:

$$
\boxed{
\text{physical model}
\rightarrow
\text{predictability under changing illumination}
}
$$

versus:

$$
\boxed{
\text{artistic model}
\rightarrow
\text{intentional controllability}
}
$$

depending on the project's visual goal.

---

**Why PBR became dominant**

With old ad-hoc shading, an asset might be tuned under one studio light.

Move it outdoors and it looks wrong.

PBR constrains material parameters so the renderer interprets them consistently.

A copper material should remain recognizably copper under:

- daylight,
- overcast sky,
- indoor light,
- dark environment.

This improves:

- asset reuse,
- artist iteration,
- interchange,
- realism.

It does not mean the renderer exactly simulates every photon.

---

**When Lambert + GGX is insufficient**

The standard metallic-roughness model works extremely well for many opaque surfaces.

It is not enough for:

$$
\text{skin},
$$

$$
\text{wax},
$$

$$
\text{hair},
$$

$$
\text{cloth},
$$

$$
\text{car paint},
$$

$$
\text{iridescence},
$$

$$
\text{thin film},
$$

$$
\text{anisotropic brushed metal},
$$

$$
\text{volumetric glass}.
$$

Different physics demand different models.

PBR means:

$$
\boxed{
\text{physically interpretable model appropriate to the phenomenon}
}
$$

not:

$$
\boxed{
\text{use GGX for everything}.
}
$$

---

**Complete GGX metallic-roughness implementation**

Now we rebuild the entire shader cleanly.

```glsl
const float PI = 3.14159265358979323846;

// -----------------------------------------------------------------------------
// Schlick Fresnel.
//
// F0 is normal-incidence reflectance.
// VdotH controls the angle at the microscopic reflecting interface.
// -----------------------------------------------------------------------------
vec3 FresnelSchlick(
    float VdotH,
    vec3 F0)
{
    float oneMinus =
        1.0 - clamp(VdotH, 0.0, 1.0);

    float oneMinus2 =
        oneMinus * oneMinus;

    float oneMinus5 =
        oneMinus2 *
        oneMinus2 *
        oneMinus;

    return
        F0 +
        (vec3(1.0) - F0) *
        oneMinus5;
}

// -----------------------------------------------------------------------------
// GGX / Trowbridge-Reitz normal distribution.
//
// perceptualRoughness is the artist/glTF parameter.
// alpha = roughness^2 is the microfacet slope parameter.
// -----------------------------------------------------------------------------
float DistributionGGX(
    float NdotH,
    float perceptualRoughness)
{
    float alpha =
        perceptualRoughness *
        perceptualRoughness;

    float alpha2 =
        alpha * alpha;

    float denominator =
        NdotH * NdotH *
        (alpha2 - 1.0) +
        1.0;

    return
        alpha2 /
        max(
            PI *
            denominator *
            denominator,
            1e-7);
}

// -----------------------------------------------------------------------------
// One-direction Schlick approximation to Smith GGX masking.
// -----------------------------------------------------------------------------
float GeometrySchlickGGX(
    float NdotX,
    float perceptualRoughness)
{
    float r =
        perceptualRoughness +
        1.0;

    // Direct-light remapping.
    float k =
        (r * r) /
        8.0;

    return
        NdotX /
        max(
            NdotX *
            (1.0 - k) +
            k,
            1e-7);
}

// -----------------------------------------------------------------------------
// Joint light/view visibility.
// -----------------------------------------------------------------------------
float GeometrySmith(
    float NdotL,
    float NdotV,
    float perceptualRoughness)
{
    return
        GeometrySchlickGGX(
            NdotL,
            perceptualRoughness)
        *
        GeometrySchlickGGX(
            NdotV,
            perceptualRoughness);
}

struct MaterialSample
{
    vec3 baseColor;
    float metallic;
    float roughness;
    float ao;
    vec3 emissive;
};

struct BRDFResult
{
    vec3 diffuse;
    vec3 specular;
    vec3 fresnel;
};

BRDFResult EvaluateBRDF(
    vec3 N,
    vec3 V,
    vec3 L,
    MaterialSample material)
{
    BRDFResult result;

    result.diffuse  = vec3(0.0);
    result.specular = vec3(0.0);
    result.fresnel  = vec3(0.0);

    float NdotL =
        max(dot(N, L), 0.0);

    float NdotV =
        max(dot(N, V), 0.0);

    if (NdotL <= 0.0 ||
        NdotV <= 0.0)
    {
        return result;
    }

    vec3 H =
        normalize(L + V);

    float NdotH =
        max(dot(N, H), 0.0);

    float VdotH =
        max(dot(V, H), 0.0);

    // Metallic workflow:
    //
    // dielectric:
    //     neutral 4% interface reflectance
    //
    // conductor:
    //     colored F0 from base color
    vec3 F0 =
        mix(
            vec3(0.04),
            material.baseColor,
            material.metallic);

    vec3 F =
        FresnelSchlick(
            VdotH,
            F0);

    float D =
        DistributionGGX(
            NdotH,
            material.roughness);

    float G =
        GeometrySmith(
            NdotL,
            NdotV,
            material.roughness);

    vec3 specular =
        D * G * F /
        max(
            4.0 *
            NdotL *
            NdotV,
            1e-6);

    // Energy reaching diffuse body scattering:
    //
    // 1-F:
    // energy not immediately reflected at interface.
    //
    // 1-metallic:
    // conductors have no ordinary diffuse body lobe.
    vec3 kD =
        (vec3(1.0) - F) *
        (1.0 - material.metallic);

    vec3 diffuse =
        kD *
        material.baseColor /
        PI;

    result.diffuse =
        diffuse;

    result.specular =
        specular;

    result.fresnel =
        F;

    return result;
}

vec3 EvaluateDirectLight(
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 incidentRadiance,
    float visibility,
    MaterialSample material)
{
    float NdotL =
        max(dot(N, L), 0.0);

    if (NdotL <= 0.0)
        return vec3(0.0);

    BRDFResult brdf =
        EvaluateBRDF(
            N,
            V,
            L,
            material);

    // Rendering equation:
    //
    // Lo = f(L,V) Li (N dot L)
    return
        (
            brdf.diffuse +
            brdf.specular
        )
        *
        incidentRadiance
        *
        NdotL
        *
        visibility;
}
```

That is approximately the central real-time BRDF Kairo uses today.

---

**Term-by-term defense: `N`**

$$
N
$$

is the normalized shading normal.

Why needed?

It defines the local surface hemisphere and all cosine projections.

Alternative?

For flat geometric rendering, use geometric normal.

For normal mapping, use perturbed shading normal while retaining geometric normal for certain visibility and robustness decisions.

For hair, cloth or anisotropic materials, a full tangent frame may matter more than one normal alone.

---

**Term-by-term defense: `V`**

$$
V
$$

points from surface toward camera.

It appears in:

$$
N\cdot V,
$$

$$
H,
$$

$$
V\cdot H.
$$

Without $V$, the material could not have view-dependent Fresnel or specular reflection.

A Lambert-only diffuse shader does not require $V$, which is why old diffuse lighting often ignores camera position.

---

**Term-by-term defense: `L`**

$$
L
$$

points from surface toward light.

The rendering equation multiplies by:

$$
N\cdot L.
$$

This is projected-area irradiance.

It is not part of Fresnel.

It is not GGX roughness.

It comes from geometry of incoming flux on the macroscopic surface.

---

**Term-by-term defense: `H`**

$$
H=
\frac{L+V}{\|L+V\|}
$$

is the microscopic mirror normal that would reflect $L$ into $V$.

It exists because microfacet BRDFs ask:

> what fraction of microscopic facets have the exact orientation needed?

If your model is not microfacet specular reflection, $H$ may not be the right construct.

Lambert does not need it.

---

**Term-by-term defense: $D$**

$$
D(H)
$$

asks:

> how much microscopic area has normals around $H$?

GGX gives long specular tails.

Beckmann gives shorter exponential tails.

Charlie-like distributions can better serve sheen.

Anisotropic GGX replaces scalar roughness by direction-dependent:

$$
\alpha_x,\alpha_y.
$$

Therefore:

$$
\boxed{
D\text{ is where the assumed microscopic surface slope statistics live.}
}
$$

---

**Term-by-term defense: $F$**

$$
F
$$

is interface reflection.

At normal incidence:

$$
F=F_0.
$$

At grazing:

$$
F\rightarrow1.
$$

Schlick is an approximation.

If you require:

- exact dielectric polarization behavior,
- spectral conductors,
- thin-film interference,

you need a more sophisticated Fresnel model.

Thus:

$$
\boxed{
F\text{ is electromagnetic interface physics.}
}
$$

---

**Term-by-term defense: $G$**

$$
G
$$

accounts for microscopic masking and shadowing.

Without it, rough surfaces at grazing angles can reflect impossible amounts because the NDF counts facets that cannot simultaneously see the light and viewer.

Smith GGX is a physically motivated model.

Schlick-GGX is a cheaper approximation.

Thus:

$$
\boxed{
G\text{ is microscopic visibility.}
}
$$

---

**Term-by-term defense: denominator**

$$
4(N\cdot L)(N\cdot V)
$$

converts the microscopic facet distribution/reflection geometry into the macroscopic BRDF domain.

It arises from projected-area and reflection-mapping Jacobians.

It is not a tweakable artist parameter.

If you replace it arbitrarily, the result is no longer the same microfacet derivation.

---

**Term-by-term defense: $1/\pi$**

Lambert:

$$
f_d=
\frac{\rho}{\pi}.
$$

The factor normalizes the constant hemispherical BRDF so total reflected diffuse energy equals:

$$
\rho.
$$

It exists because:

$$
\int_{\Omega^+}\cos\theta\,d\omega=\pi.
$$

Again, not an artist parameter.

---

**Term-by-term defense: $1-F$**

If Fresnel reflects fraction:

$$
F,
$$

only approximately:

$$
1-F
$$

remains available to enter a dielectric and participate in body scattering.

This is the energy split.

Without it, diffuse and specular can double-use incident energy.

---

**Term-by-term defense: $1-m$**

For opaque metallic conductor:

$$
\text{diffuse}\approx0.
$$

Therefore:

$$
1-m
$$

kills the diffuse lobe as material becomes metallic.

This is a workflow approximation mixing conductor and dielectric models.

It is not a fundamental law of electromagnetism called “metalness.”

---

**Term-by-term defense: roughness**

Roughness controls statistical microfacet slope.

In Kairo/glTF-style GGX:

$$
\alpha=r^2.
$$

It changes $D$ and $G$.

It should not independently multiply final specular intensity just to make rough objects darker.

If a rough surface loses energy due to single-scattering microfacet approximation, the physically principled response is multiple-scattering compensation—not arbitrary intensity fading.

---

**Term-by-term defense: metallic**

Metallic chooses an interpolation between:

$$
\text{dielectric behavior}
$$

and:

$$
\text{conductor behavior}.
$$

At $m=0$:

$$
F_0\approx0.04,
$$

diffuse exists.

At $m=1$:

$$
F_0=C_{\text{base}},
$$

diffuse vanishes.

Intermediate values are practical material mixing/antialiasing territory.

---

**Term-by-term defense: AO**

AO approximates reduced indirect-light accessibility.

It should normally affect indirect/environment lighting more strongly than direct lights.

It is not:

$$
\text{shadow map}.
$$

It contains no light-specific visibility information.

Thus multiplying all lighting by AO is physically dubious.

---

**Term-by-term defense: emissive**

Emission adds:

$$
L_e.
$$

It is independent of incident illumination.

But in raster rendering, adding emissive to the current pixel does not automatically propagate it to other objects.

For physically propagated emission you need a global illumination/light transport mechanism.

---

**Alternative model: Oren–Nayar**

Use when:

$$
\text{diffuse surface is microscopically rough}
$$

and Lambert looks too flat.

Examples:

- concrete,
- plaster,
- clay.

Oren–Nayar produces angle-dependent diffuse behavior and retroreflection.

---

**Alternative model: Disney/Burley diffuse**

Use when you want a production-friendly rough diffuse approximation with moderate cost.

It improves:

- grazing response,
- rough diffuse appearance,

without requiring full Oren–Nayar complexity.

---

**Alternative model: anisotropic GGX**

Use for surfaces with directional roughness:

- brushed metal,
- machined surfaces,
- hair-like grooves.

Instead of:

$$
\alpha
$$

use:

$$
\alpha_x,\alpha_y.
$$

Now tangent and bitangent become physically meaningful scattering axes.

---

**Alternative model: clearcoat**

Use for layered dielectric coating:

- varnish,
- car paint,
- lacquer.

Add a secondary dielectric specular lobe and attenuate lower layers accordingly.

---

**Alternative model: sheen/cloth**

Use for fibre-driven grazing reflection:

- velvet,
- fabric,
- cloth.

Ordinary GGX specular does not capture the characteristic soft edge sheen well.

---

**Alternative model: BTDF/transmission**

Use when energy crosses the surface.

The ordinary BRDF is only same-side reflection.

Glass needs:

$$
\text{BSDF}
=
\text{BRDF}
+
\text{BTDF}.
$$

Then Snell, Fresnel, TIR and possibly Beer-Lambert absorption become essential.

---

**Alternative model: subsurface scattering**

Use when light travels noticeable distances inside material before exiting.

Examples:

- skin,
- wax,
- marble,
- milk.

A local surface BRDF cannot capture spatial transport.

You need BSSRDF/diffusion/random-walk style models.

---

**Alternative model: spectral rendering**

Use when RGB approximation is inadequate:

- dispersion,
- iridescence,
- spectral conductor accuracy,
- fluorescent effects.

Then:

$$
L(\lambda)
$$

must be represented at multiple wavelengths rather than three RGB channels.

---

**Reference-sphere implementation architecture**

A useful engine validation harness should produce a fixed grid programmatically.

For example:

```cpp
struct ReferenceMaterial
{
    float Metallic;
    float Roughness;
    Vec3 BaseColor;
};

std::vector<ReferenceMaterial>
BuildPBRReferenceGrid()
{
    constexpr float roughnesses[] =
    {
        0.05f,
        0.15f,
        0.30f,
        0.50f,
        0.75f,
        1.00f
    };

    std::vector<ReferenceMaterial> result;

    for (float metallic : {0.0f, 1.0f})
    {
        for (float roughness : roughnesses)
        {
            result.push_back(
            {
                .Metallic = metallic,
                .Roughness = roughness,
                .BaseColor =
                    metallic > 0.5f
                    ? Vec3{0.95f, 0.64f, 0.54f}
                    : Vec3{0.50f, 0.18f, 0.08f}
            });
        }
    }

    return result;
}
```

Every render should lock:

$$
\text{camera},
\text{light},
\text{exposure},
\text{resolution},
\text{tone mapping}.
$$

If any of those change, comparisons become contaminated.

---

**Better than screenshots: intermediate render modes**

Kairo already has some diagnostic shading modes.

Extend them.

A serious PBR debugger should be able to visualize:

```text
baseColor
normal
metallic
roughness
AO
F0
F
D
G
specular BRDF
diffuse BRDF
NdotL
NdotV
shadow visibility
scene-linear HDR
final tone-mapped color
```

One at a time.

Then a bad final image can be bisected exactly like a black frame.

For example:

$$
D\text{ wrong}
$$

is immediately distinguishable from:

$$
F_0\text{ wrong}.
$$

---

**Cross-renderer oracle**

You have something unusually valuable:

$$
\text{KairoRenderer}
$$

and:

$$
\text{KairoRayTracer}.
$$

Use them as mutual test oracles.

For a scene restricted to:

- one sphere,
- one directional light,
- no shadows,
- no environment,
- same camera,
- same GGX equations,
- same color space,
- same exposure,

the raster renderer and direct ray tracer should numerically agree closely for corresponding visible surface points.

If they do not:

$$
\boxed{
\text{that difference is evidence}.
}
$$

And we already know one candidate difference:

Ray tracer diffuse currently lacks the real-time shader's:

$$
1-F
$$

energy reduction.

That is exactly the sort of discrepancy a reference harness should catch.

---

**Validation at one surface point**

Choose:

$$
N=(0,0,1),
$$

$$
V=(0,0,1),
$$

and light:

$$
L=
\operatorname{normalize}(1,0,1).
$$

Then:

$$
N\cdot L
=
\frac1{\sqrt2}
\approx0.7071.
$$

Compute:

$$
H=
\operatorname{normalize}(L+V).
$$

Now independently calculate:

$$
N_H,
$$

$$
V_H,
$$

$$
D,
F,
G,
$$

$$
f_d,
f_s.
$$

Hard-code the expected floating-point result into a unit test.

This is much stronger than “sphere looks about right.”

---

**Property-based BRDF test**

Randomly generate:

$$
r\in[0.04,1],
$$

$$
m\in[0,1],
$$

$$
C\in[0,1]^3,
$$

valid upper-hemisphere $L,V$.

Then assert:

```cpp
CHECK(IsFinite(D));
CHECK(IsFinite(G));
CHECK(IsFinite(F));
CHECK(IsFinite(BRDF));

CHECK(D >= 0.0f);

CHECK(All(F >= 0.0f));
CHECK(All(F <= 1.0f));

CHECK(G >= 0.0f);
CHECK(G <= 1.0f);
```

Then reciprocity:

```cpp
BRDF a =
    Evaluate(N, L, V, material);

BRDF b =
    Evaluate(N, V, L, material);

CHECK(Near(a, b, epsilon));
```

This gives thousands of corner cases essentially for free.

---

**Image regression tests**

After numerical tests, render deterministic reference images.

Compare:

$$
I_{\text{new}}
$$

with:

$$
I_{\text{reference}}.
$$

Metrics can include:

$$
RMSE
=
\sqrt{
\frac1N
\sum_i
(I_i-R_i)^2
},
$$

and perceptual metrics such as SSIM.

But raw RGB image comparisons should ideally occur:

$$
\boxed{
\text{in a known linear color domain}
}
$$

rather than comparing unknown tone-mapped screenshots.

---

**Why image tests need tolerances**

GPU rendering can vary slightly across:

- hardware,
- drivers,
- compiler optimizations,
- floating-point contraction.

Exact bitwise equality can be too strict for cross-platform rasterization.

So define tolerances.

For deterministic CPU ray tracing, exact or tighter comparisons may be possible.

This lets the ray tracer act as a stronger numerical oracle than GPU screenshots.

---

**Complete production material path**

We can now reconstruct the whole Phase-20 dataflow:

```text
ARTIST / DCC
     ↓
glTF material
     ↓
baseColorFactor
metallicFactor
roughnessFactor
normal/occlusion/emissive
alpha policy
texture UV sets/transforms
     ↓
asset importer
     ↓
semantic classification
     ↓
sRGB color textures
linear data textures
normal maps
mip generation
     ↓
persistent engine artifact
     ↓
runtime material
     ↓
GPU material record
     ↓
std140 / std430 ABI
     ↓
descriptor set or bindless indices
     ↓
fragment shader
     ↓
sample textures
     ↓
decode normal
     ↓
build N, V, L, H
     ↓
D / F / G
     ↓
Cook–Torrance
     +
normalized diffuse
     ↓
direct + indirect + emission
     ↓
scene-linear HDR
     ↓
post/exposure/tone map
     ↓
display
```

Every arrow is an opportunity for a bug.

That is why physically based rendering is a **system**, not a 20-line fragment shader.

---

**Current Kairo Phase-20 audit**

| System | Current state |
|---|---|
| Metallic-roughness core | Implemented |
| Base-color sRGB semantics | Implemented through texture semantics/formats |
| Metallic-roughness G/B channels | Implemented |
| Normal map linear semantics | Implemented |
| Normal scale | Implemented |
| Emissive | Implemented |
| AO | Implemented, but strength equation does not exactly match glTF |
| glTF alpha modes | Imported |
| glTF double-sided flag | Stored, but raster pipeline currently culls nothing globally |
| glTF UV set index | Artifact stores it; renderer path does not fully exploit per-texture UV sets |
| `KHR_texture_transform` | Not represented end-to-end |
| Material GPU block | 64-byte four-`vec4` layout |
| std140-safe packing | Effectively yes |
| std430 material array | Not current architecture |
| Per-draw descriptor sets | Current architecture |
| Bindless texture table | Not current architecture |
| Opaque material sorting | Not currently performed |
| Transparent depth sorting | Implemented |
| Hard shader variants | opaque/blend × static/skinned etc. |
| General feature specialization | Limited |
| Reference sphere validation | Not yet comprehensive |
| PBR smoke tests | Present |
| Quantitative BRDF reciprocity/energy tests | Not currently evident |
| Real-time/ray-tracer BRDF agreement | Not exact today |
| HDR material target before tone map | Not preserved; tone map currently in material shader |

That is a very concrete completion state rather than abstract theory.

---

**The best architecture upgrade from this pass**

If we were evolving Kairo specifically from the knowledge in M119/M120, the most coherent next material architecture would be:

```text
PBRMaterial asset
        ↓
MaterialID
        ↓
std430 MaterialGPU[]
        ↓
each MaterialGPU stores:
    factors
    flags
    texture indices
    UV transform indices
        ↓
global texture descriptor array
        ↓
draw contains:
    meshID
    materialID
        ↓
shader:
    material = materials[materialID]
    texture = textures[material.textureIndex]
```

This architecture naturally connects to Pass 2's:

$$
\text{GPU-driven indirect rendering}.
$$

You would no longer need one descriptor set per draw.

---

**But do not prematurely bindless everything**

The current Kairo approach has major advantages:

$$
\text{simple lifetime model},
$$

$$
\text{easy validation},
$$

$$
\text{easy debugging},
$$

$$
\text{clear descriptor ownership}.
$$

Bindless introduces:

- feature queries,
- non-uniform indexing,
- large descriptor tables,
- slot allocation,
- lifetime/reuse rules,
- streaming synchronization.

The correct engineering rule is:

$$
\boxed{
\text{upgrade architecture when measured scale justifies the complexity}.
}
$$

Not because “bindless is modern.”

---

**Research bridge**

This phase gives you a very strong experimental protocol for renderer/image-fidelity research.

Suppose two reconstruction methods produce slightly different image residuals.

Before concluding:

> method A reconstructs geometry better,

you must normalize rendering semantics:

$$
\text{same color space},
$$

$$
\text{same BRDF},
$$

$$
\text{same roughness convention},
$$

$$
\text{same tangent basis},
$$

$$
\text{same texture filtering},
$$

$$
\text{same exposure},
$$

$$
\text{same tone mapper}.
$$

Otherwise your error metric may be measuring:

$$
\text{renderer mismatch}
$$

rather than:

$$
\text{scene representation mismatch}.
$$

The curriculum explicitly treats M119/M120 as the material and validation bridge for physically interpretable image fidelity.

---


| Concept | Reconstruction |
|---|---|
| glTF core material | metallic-roughness |
| base color | diffuse for dielectric, $F_0$ for metal |
| dielectric $F_0$ | 0.04 in core model |
| MR texture | G = roughness, B = metallic |
| base color texture | sRGB RGB, linear alpha |
| normal/MR/AO | linear data |
| factor × texture | material scalar/vector multiplies sampled value |
| texture transform | $UV'=TRS\,UV$ |
| std140 | extended alignment, conservative 16-byte aggregate layout |
| std430 | base alignment, tighter arrays/structs |
| padding | part of CPU↔GPU ABI |
| Kairo material GPU block | 4 vec4 = 64 bytes |
| bindless | resource table + shader indices instead of rebinding per object |
| descriptor indexing | runtime/nonuniform resource indexing + related descriptor features |
| material sorting | reduce pipeline/descriptor/state changes |
| transparent sorting | correctness often dominates material batching |
| shader specialization | move expensive structural choices from runtime branch to pipeline variant |
| permutation danger | feature dimensions multiply |
| reference spheres | isolate roughness/metallic/Fresnel behavior |
| gamma bug | wrong transfer domain |
| normal bug | wrong linearity/basis/normalization/channel orientation |
| roughness bug | wrong channel or wrong $r\leftrightarrow\alpha$ interpretation |
| $F_0$ bug | dielectric and conductor semantics confused |
| quantitative validation | invariants + reciprocity + energy integration + image regression |
| artistic model | control prioritized |
| physical model | predictive, constrained behavior prioritized |
| GGX | appropriate for many opaque microfacet surfaces, not all materials |

---

### Final Phase-20 reconstruction gate

You should now be able to start from this glTF-like description:

```json
{
  "baseColorFactor": [0.8, 0.2, 0.05, 1.0],
  "metallicFactor": 0.0,
  "roughnessFactor": 0.3
}
```

and explain, without skipping any boundary:

$$
\text{asset bytes}
$$

to:

$$
\text{linear shader parameters}
$$

to:

$$
F_0=0.04
$$

to:

$$
\alpha=r^2
$$

to:

$$
H=\frac{L+V}{\|L+V\|}
$$

to:

$$
D_{\mathrm{GGX}}
$$

to:

$$
F_{\mathrm{Schlick}}
$$

to:

$$
G_{\mathrm{Smith}}
$$

to:

$$
f_s=
\frac{DFG}{4N_LN_V}
$$

plus:

$$
f_d=
(1-F)(1-m)\frac{C}{\pi}
$$

then:

$$
L_o=
(f_d+f_s)L_iN_L,
$$

and finally explain exactly how those numbers are packed into CPU/GPU memory and which texture descriptor the shader reads.

Then you should be able to take a wrong image and determine whether the error is:

$$
\text{asset semantic},
$$

$$
\text{color space},
$$

$$
\text{UV transform},
$$

$$
\text{GPU layout},
$$

$$
\text{descriptor binding},
$$

$$
\text{normal frame},
$$

$$
\text{roughness mapping},
$$

$$
F_0,
$$

$$
D,
$$

$$
F,
$$

$$
G,
$$

$$
\text{energy split},
$$

$$
\text{IBL},
$$

or:

$$
\text{tone mapping}.
$$

At that point, you no longer merely “know PBR.”

You can **reconstruct, implement, transport, optimize and validate the entire material pipeline**.


Reflectance describes how incoming light is redirected, linking material response to visible shading.

### BRDF examples and angular response

The bidirectional reflectance distribution function (BRDF) describes a surface location and two directions: incoming light and outgoing view. Lambertian response is independent of those directions: once incoming irradiance is known, outgoing radiance is the same for every view direction. A mirror reflects only the direction that obeys equality of incidence and reflection angles; mathematically, its response is concentrated as a delta distribution. A glossy surface spreads reflection around that direction. Fresnel reflection varies with angle and refractive indices; dielectrics generally reflect more at grazing incidence. Microfacet models approximate a collection of tiny mirror-like facets and combine their distribution, masking, and Fresnel response.

An ideal delta-direction event is difficult for a sampler that draws arbitrary directions: the chance of sampling exactly one direction is zero. This is why path samplers need to represent specular events specially, and why a smooth density cannot describe every material. The rendering equation does not change; its integrand may contain both smooth lobes and singular contributions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

### Color under ultraviolet illumination

Two inks may look identical in visible daylight because their reflected spectra produce similar cone responses. Under ultraviolet illumination, a fluorescent ink may emit visible light while another ink does not. The pair then becomes distinguishable. This is useful in counterfeit detection for banknotes, passports, or security marks. The example demonstrates the limits of RGB: a stored triplet cannot predict appearance under arbitrary spectra if the material’s spectral behavior was discarded. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

### Lambertian versus perfect specular reflection

For a Lambertian material, $f_r=\rho/\pi$. If incoming radiance is constant across the hemisphere, irradiance is $\pi L$, because the cosine-weighted hemisphere integral is $\pi$. Then outgoing radiance is $L_o=(\rho/\pi)(\pi L)=\rho L$, independent of viewing direction. For a perfect mirror, only one reflected incoming direction contributes, so an integral over arbitrary directions is a poor direct numerical representation. A ray can instead reflect deterministically. These contrasting examples show why material models determine sampling strategies. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

### Shading links geometry to light

At a visible surface point, a shader combines material response with incoming illumination. A simple diffuse model follows Lambert's cosine law: a light contributes in proportion to

$$
\max(0,n\cdot l).
$$

where (n) is a unit surface normal and (l) points from the surface toward the light. A point light's intensity also falls with distance; an idealized model uses inverse-square attenuation 1/r^2. Specular response adds a bright highlight near the mirror-reflection direction. In a Blinn–Phong model, with view vector (v), one uses half-vector $h=(l+v)/\|l+v\|$ and a term $\max(0,n\cdot h)^s$. The exponent (s) controls highlight width: a larger exponent yields a tighter highlight. This is an empirical model, not a full physical account, but it lets a renderer assign visible cues to matte, shiny, or metallic-looking surfaces.

A more general material description is a BRDF f_r(x,ω_i,ω_o), the ratio relating incoming light from direction ω_i to outgoing light toward ω_o. It is constrained by energy conservation and reciprocity for physically plausible materials. The shading lecture builds a model from light direction, attenuation, diffuse and specular components, then asks what a material description must explain [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-13 - Shading|MITG-13]] (00:15:00–00:30:00; 00:50:00–01:10:00). The key separation is between geometry (where the point and normal are) and material (how the point scatters light). A bright pixel can result from a bright source, a facing normal, a reflective material, or several of these at once.

For a diffuse Lambertian surface with albedo ρ, the BRDF is f_r=ρ/π. The 1/π normalizes the reflected energy over the hemisphere: for unit incoming radiance, integrating f_r(n·ω) over all outgoing directions returns ρ. This example shows why a BRDF is not merely a brightness multiplier. It has to describe how energy is distributed with direction.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/07 - The Rendering Equation and Path Probabilities|Next chapter]]
