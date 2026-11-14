# 19 — Textures as Sampled Surface Signals

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/18 - C++ Representation, Ownership and Lifetime|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/20 - Geometry Queries from Scene to Contact|Next chapter]]

A texture stores samples of a signal over a surface, and its footprint changes with projection. Filtering must account for that changing footprint to avoid unstable detail and aliasing.

**By the end:** Connect texture sampling, surface footprints and filtering choices.

**Recall question:** Why does a minified texture need a different footprint than a magnified one?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 45446–48179; source-span SHA256 7652991984c323a7432aee7bf45394b9029c86219ca693a6cea9960a1be03bec. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Textures and surface detail

### Match a texture footprint to the surface signal

Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.

**A texture is a sampled signal**

Suppose there exists an ideal continuous function:

$$
f(u,v).
$$

A digital texture does not store that continuous function.

It stores discrete samples:

$$
f[i,j].
$$

These samples are **texels**.

Therefore:

$$
\boxed{
\text{texture sampling is a signal-reconstruction problem}
}
$$

not merely “look up a color in an image.”

This viewpoint explains:

- nearest filtering,
- bilinear filtering,
- mipmaps,
- anisotropy,
- aliasing.

---

**Texel versus pixel**

A **texel** belongs to a texture.

A **pixel** belongs to an output image.

They are not necessarily one-to-one.

One texel may cover hundreds of pixels during magnification.

Thousands of texels may map into a single pixel during minification.

Therefore:

$$
\boxed{
\text{texel density}
\neq
\text{screen pixel density}
}
$$

and texture filtering exists because we must map between those sampling grids.

---

**UV coordinates**

A texture coordinate commonly uses:

$$
(u,v)\in[0,1]^2.
$$

For width $W$, height $H$, a simple conceptual conversion is:

$$
x=uW,
\qquad
y=vH.
$$

But texels represent cells with centers, so filtering implementations usually account for the half-texel relationship.

A useful conceptual coordinate for bilinear filtering is:

$$
x=uW-\frac12,
$$

$$
y=vH-\frac12.
$$

Then integer texel centers align naturally with the reconstructed samples.

Actual Vulkan normalized-coordinate sampling has precise rules defined in the sampling specification, including coordinate conversion, image dimensions, wrapping and filtering. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/textures.html?utm_source=chatgpt.com)

---

**Wrapping modes**

What happens if:

$$
u=1.25?
$$

### Repeat

$$
u'=u-\lfloor u\rfloor.
$$

So:

$$
1.25\rightarrow0.25.
$$

---

### Clamp-to-edge

$$
u'=
\operatorname{clamp}(u,0,1).
$$

So:

$$
1.25\rightarrow1.
$$

---

### Mirrored repeat

Intervals alternate direction:

```text
0→1
1→0
0→1
...
```

This avoids the discontinuity that ordinary repetition produces at every tile boundary for some textures.

Your current `TextureSampler` supports:

```text
Repeat
MirroredRepeat
ClampToEdge
```

for U/V.

---

**Nearest sampling**

Nearest chooses the closest stored sample.

Conceptually:

$$
i=\operatorname{round}(x),
$$

$$
j=\operatorname{round}(y).
$$

Then:

$$
C=T[i,j].
$$

Advantages:

- cheap,
- preserves hard pixel-art edges,
- deterministic sample choice.

Disadvantages:

- blocky magnification,
- severe minification aliasing.

---

**Bilinear filtering**

Let texture coordinate map to:

$$
x=i+\alpha,
$$

$$
y=j+\beta
$$

where:

$$
0\le\alpha,\beta<1.
$$

Four texels contribute:

$$
T_{00}=T[i,j],
$$

$$
T_{10}=T[i+1,j],
$$

$$
T_{01}=T[i,j+1],
$$

$$
T_{11}=T[i+1,j+1].
$$

Interpolate horizontally:

$$
A=(1-\alpha)T_{00}+\alpha T_{10},
$$

$$
B=(1-\alpha)T_{01}+\alpha T_{11}.
$$

Then vertically:

$$
\boxed{
C=(1-\beta)A+\beta B.
}
$$

Expanded:

$$
C=
(1-\alpha)(1-\beta)T_{00}
+
\alpha(1-\beta)T_{10}
+
(1-\alpha)\beta T_{01}
+
\alpha\beta T_{11}.
$$

This is a separable triangle filter.

---

**Bilinear does not solve minification**

Suppose a distant surface maps one screen pixel onto a:

$$
64\times64
$$

texel footprint.

That pixel conceptually represents information from:

$$
4096
$$

source texels.

Bilinear filtering samples only four nearby texels.

Four samples cannot represent the average of 4096 high-frequency texels.

This violates sampling theory and causes:

- moiré,
- shimmering,
- temporal crawling,
- unstable fine patterns.

Thus:

$$
\boxed{\text{bilinear filtering is reconstruction, not sufficient prefiltering}}
$$

for heavy minification.

---

**Nyquist intuition**

If the underlying texture contains frequency $f$, the screen sampling rate must be at least:

$$
2f
$$

to represent it without aliasing.

When the texture footprint shrinks on screen, high-frequency components exceed the screen's Nyquist limit.

They must be removed **before sampling**.

This is exactly what mipmaps provide.

---

**Mipmaps**

Mip level 0:

$$
W\times H.
$$

Level 1:

$$
\frac W2\times\frac H2.
$$

Level 2:

$$
\frac W4\times\frac H4.
$$

Eventually:

$$
1\times1.
$$

Example:

```text
1024×1024
512×512
256×256
128×128
64×64
32×32
16×16
8×8
4×4
2×2
1×1
```

Each level is a prefiltered representation suitable for a larger texture footprint.

---

**Total mipmap memory**

For an uncompressed square texture:

$$
1+\frac14+\frac1{16}+\frac1{64}+\cdots
$$

is a geometric series.

Its sum tends toward:

$$
\frac{1}{1-\frac14}
=
\frac43.
$$

Therefore a complete mip chain costs only about:

$$
\boxed{33\%}
$$

more storage than the base image.

This is extraordinarily cheap relative to the quality and cache benefits.

---

**Correct mip generation**

A simplistic color mip takes a 2×2 block:

$$
C_{00},C_{10},C_{01},C_{11}
$$

and averages:

$$
C'=
\frac{
C_{00}+C_{10}+C_{01}+C_{11}
}{4}.
$$

But that average must happen in the **correct domain**.

For color:

```text
sRGB bytes
  ↓ decode
linear RGB
  ↓ average/filter
linear RGB mip
  ↓ encode if stored in sRGB format
sRGB bytes
```

Your KairoAssets implementation does exactly that.

---

**Normal maps cannot be mipmapped like ordinary RGB**

Suppose two normals are:

$$
N_1=(1,0,0),
$$

$$
N_2=(0,1,0).
$$

Their simple average is:

$$
(0.5,0.5,0).
$$

Its length is:

$$
\sqrt{0.5}
\approx0.707.
$$

It is no longer a unit normal.

A basic normal-map mip generator therefore:

1. decodes RGB to vectors,
2. averages vectors,
3. renormalizes,
4. re-encodes.

Your KairoAssets mip builder explicitly does this for normal maps.

That is an excellent example of:

$$
\boxed{
\text{texture semantic determines the correct filtering operation}.
}
$$

---

**Mipmap level selection**

Suppose UV changes by:

$$
\frac{\partial u}{\partial x},
\quad
\frac{\partial v}{\partial x},
\quad
\frac{\partial u}{\partial y},
\quad
\frac{\partial v}{\partial y}.
$$

Convert normalized UV derivatives into base-level texel units:

$$
\mathbf d_x=
\left(
W\frac{\partial u}{\partial x},
H\frac{\partial v}{\partial x}
\right),
$$

$$
\mathbf d_y=
\left(
W\frac{\partial u}{\partial y},
H\frac{\partial v}{\partial y}
\right).
$$

A common isotropic footprint estimate is:

$$
\rho=
\max(
\|\mathbf d_x\|,
\|\mathbf d_y\|
).
$$

Then:

$$
\boxed{
\lambda=\log_2\rho
}
$$

is the desired mip LOD.

Examples:

If:

$$
\rho=1
$$

then:

$$
\lambda=0.
$$

Use base level.

If:

$$
\rho=2
$$

then:

$$
\lambda=1.
$$

If:

$$
\rho=8
$$

then:

$$
\lambda=3.
$$

Vulkan similarly derives implicit LOD from coordinate derivatives, with the exact scale-factor and LOD procedure specified by the API. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/textures.html?utm_source=chatgpt.com)

---

**Why logarithm appears**

Every mip level halves resolution.

A scale factor of:

$$
2
$$

means one level down.

A scale of:

$$
4=2^2
$$

means two levels down.

A scale of:

$$
8=2^3
$$

means three levels down.

Therefore:

$$
\lambda=\log_2\rho.
$$

The logarithm is not an arbitrary graphics convention.

It follows directly from the geometric mip pyramid.

---

**Trilinear filtering**

Suppose:

$$
\lambda=3.25.
$$

Bilinear sample level 3:

$$
C_3.
$$

Bilinear sample level 4:

$$
C_4.
$$

Then interpolate:

$$
\boxed{
C=
0.75C_3+0.25C_4.
}
$$

This is trilinear filtering:

- bilinear in $u,v$ at first mip,
- bilinear in $u,v$ at second mip,
- linear between mip levels.

Vulkan's `VK_SAMPLER_MIPMAP_MODE_LINEAR` similarly selects neighboring mip levels and interpolates between them. [docs.vulkan.org](https://docs.vulkan.org/tutorial/latest/09_Generating_Mipmaps.html?utm_source=chatgpt.com)

---

**Kairo's current sampler**

Your current Vulkan sampler uses:

```cpp
magFilter  = VK_FILTER_LINEAR;
minFilter  = VK_FILTER_LINEAR;
mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
minLod     = 0;
maxLod     = mipLevels - 1;
```

Therefore Kairo already has:

$$
\boxed{\text{bilinear + trilinear mip filtering}}
$$

for ordinary material textures.

---

**Derivatives again**

Last pass we used:

```glsl
dFdx(inTexCoord)
dFdy(inTexCoord)
```

to build a tangent basis.

The texture hardware also needs essentially the same information.

A shader expression:

```glsl
texture(baseColorMap, inTexCoord)
```

normally performs **implicit LOD** lookup in the fragment stage.

The sampler derives how rapidly the UV changes across neighboring fragments and selects an appropriate mip level. Vulkan explicitly states that implicit-LOD image instructions calculate derivatives from neighboring fragment coordinates and use those derivatives for LOD selection. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/textures.html?utm_source=chatgpt.com)

Thus:

$$
\boxed{
\text{fragment quads}
\rightarrow
UV derivatives
\rightarrow
texture footprint
\rightarrow
LOD
}
$$

is an actual hardware/dataflow connection.

---

**Texture footprint is not generally square**

Consider a floor stretching into the distance.

Near the camera, one pixel might cover:

$$
2\times2
$$

texels.

Farther away, due to foreshortening, one screen pixel might cover an elongated region:

$$
2\times64
$$

texels.

An isotropic mip level sees essentially the largest dimension:

$$
64,
$$

and picks a low-resolution mip suitable for that scale.

But along the short direction, that mip is far too blurry.

This is why trilinear filtering causes distant ground textures to lose detail.

---

**Anisotropic filtering**

Anisotropic filtering recognizes that the footprint can be elongated.

Instead of approximating it as a square/circle, think of an ellipse.

Let major and minor footprint lengths be:

$$
a,\quad b.
$$

Anisotropy ratio:

$$
\boxed{
A=\frac{a}{b}.
}
$$

For:

$$
A=1
$$

the footprint is isotropic.

For:

$$
A=16
$$

it is highly elongated.

Anisotropic filtering samples more intelligently along the long direction while retaining higher resolution across the short direction.

The exact algorithm is implementation dependent in Vulkan, controlled by sampler anisotropy state and hardware limits. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/textures.html?utm_source=chatgpt.com)

---

**Kairo currently has anisotropy effectively disabled**

Your sampler contains:

```cpp
create.maxAnisotropy = 1.0f;
```

but does not enable:

```cpp
create.anisotropyEnable = VK_TRUE;
```

The zero-initialized structure therefore has anisotropy disabled.

So today:

$$
\boxed{
\text{Kairo = trilinear isotropic filtering}
}
$$

not anisotropic filtering.

A production change would need to:

- query sampler-anisotropy feature support,
- enable it,
- clamp requested anisotropy to device limits.

---

**CPU sampler skeleton**

A simplified software equivalent clarifies the entire hierarchy:

```cpp
struct ImageLevel
{
    int width;
    int height;
    std::vector<Vec4> texels; // Already linearized for this example.
};

struct Texture2D
{
    std::vector<ImageLevel> mipLevels;
};

float Repeat(float x)
{
    return x - std::floor(x);
}

const Vec4& FetchWrapped(
    const ImageLevel& image,
    int x,
    int y)
{
    // Repeat addressing.
    x %= image.width;
    y %= image.height;

    if (x < 0) x += image.width;
    if (y < 0) y += image.height;

    return image.texels[
        static_cast<std::size_t>(y * image.width + x)];
}

Vec4 SampleBilinear(
    const ImageLevel& image,
    Vec2 uv)
{
    // Normalized repeating UV coordinates.
    uv.x = Repeat(uv.x);
    uv.y = Repeat(uv.y);

    // Place integer texel coordinates at texel centers.
    const float x =
        uv.x * static_cast<float>(image.width) - 0.5f;

    const float y =
        uv.y * static_cast<float>(image.height) - 0.5f;

    const int x0 =
        static_cast<int>(std::floor(x));

    const int y0 =
        static_cast<int>(std::floor(y));

    const float tx = x - std::floor(x);
    const float ty = y - std::floor(y);

    const Vec4 c00 =
        FetchWrapped(image, x0,     y0);

    const Vec4 c10 =
        FetchWrapped(image, x0 + 1, y0);

    const Vec4 c01 =
        FetchWrapped(image, x0,     y0 + 1);

    const Vec4 c11 =
        FetchWrapped(image, x0 + 1, y0 + 1);

    const Vec4 row0 =
        c00 * (1.0f - tx) +
        c10 * tx;

    const Vec4 row1 =
        c01 * (1.0f - tx) +
        c11 * tx;

    return
        row0 * (1.0f - ty) +
        row1 * ty;
}

float ComputeMipLOD(
    Vec2 dUVdx,
    Vec2 dUVdy,
    int baseWidth,
    int baseHeight)
{
    // Convert normalized UV derivatives into texel-space derivatives.
    const Vec2 dx{
        dUVdx.x * baseWidth,
        dUVdx.y * baseHeight
    };

    const Vec2 dy{
        dUVdy.x * baseWidth,
        dUVdy.y * baseHeight
    };

    const float rhoX =
        std::sqrt(dx.x * dx.x + dx.y * dx.y);

    const float rhoY =
        std::sqrt(dy.x * dy.x + dy.y * dy.y);

    const float rho =
        std::max(rhoX, rhoY);

    // rho <= 1 means base-level magnification or approximately 1:1 sampling.
    return std::log2(std::max(rho, 1.0f));
}

Vec4 SampleTrilinear(
    const Texture2D& texture,
    Vec2 uv,
    Vec2 dUVdx,
    Vec2 dUVdy)
{
    const auto& base = texture.mipLevels.front();

    float lod =
        ComputeMipLOD(
            dUVdx,
            dUVdy,
            base.width,
            base.height);

    lod = std::clamp(
        lod,
        0.0f,
        static_cast<float>(
            texture.mipLevels.size() - 1));

    const int low =
        static_cast<int>(std::floor(lod));

    const int high =
        std::min(
            low + 1,
            static_cast<int>(
                texture.mipLevels.size() - 1));

    const float t =
        lod - static_cast<float>(low);

    const Vec4 a =
        SampleBilinear(texture.mipLevels[low], uv);

    const Vec4 b =
        SampleBilinear(texture.mipLevels[high], uv);

    return
        a * (1.0f - t) +
        b * t;
}
```

This is not a bit-for-bit Vulkan sampler implementation, but the conceptual pipeline is correct:

$$
UV
\rightarrow
\text{derivatives}
\rightarrow
\lambda
\rightarrow
\text{mip pair}
\rightarrow
\text{bilinear samples}
\rightarrow
\text{LOD interpolation}.
$$

---

**Texture compression**

Uncompressed RGBA8 costs:

$$
4\text{ bytes/texel}.
$$

A 4096×4096 texture requires:

$$
4096^2\times4
\approx67.1\text{ MB}
$$

before mipmaps.

With full mip chain:

$$
67.1\times\frac43
\approx89.5\text{ MB}.
$$

Ten such textures approach:

$$
895\text{ MB}.
$$

This is why GPU texture compression matters.

---

**Block compression**

Many GPU formats compress small blocks independently.

For example, BC1 uses 4×4 blocks.

A block represents:

$$
16\text{ texels}
$$

using 8 bytes.

So:

$$
\frac{8\times8\text{ bits}}
{16}
=
4\text{ bits/texel}.
$$

Compared with RGBA8:

$$
32\text{ bits/texel}.
$$

That is an 8× reduction.

Because blocks are independently decodable, the GPU can still perform random texture access without decompressing the entire image.

---

**Compression is semantic too**

Different texture types prefer different formats.

Examples:

```text
base color:
BC7 / ASTC high-quality color

normal XY:
BC5

single-channel masks:
BC4

mobile:
ASTC / ETC families
```

Why BC5 for normals?

Normal mapping can store:

$$
x,y
$$

and reconstruct:

$$
z=
\sqrt{
\max(
0,
1-x^2-y^2
)
}.
$$

So two high-quality signed-ish channels can be better than wasting general RGBA compression on XYZ.

---

**Kairo currently has no GPU block-compressed texture path**

Your current texture enum supports only:

```text
R8
RG8
RGBA8
RGBA16Float
```

No BCn, ASTC or ETC formats are represented yet.

So this portion of M116 is currently a future asset/runtime capability.

A later production texture artifact should probably encode:

```text
pixel/block format
color space
semantic
mip chain
possibly platform transcoding target
```

rather than assuming raw pixels.

---

**Normal maps**

A geometric normal approximates surface orientation at mesh resolution.

A normal map stores much higher-frequency orientation information.

Typically RGB encodes a tangent-space vector.

Stored value:

$$
C\in[0,1]^3.
$$

Decode:

$$
\boxed{
N_t=2C-1.
}
$$

For the neutral normal:

$$
N_t=(0,0,1).
$$

Its encoded RGB is:

$$
(0.5,0.5,1).
$$

Which explains the characteristic bluish appearance of normal maps.

The glTF specification likewise defines tangent-space normal textures as linear data, maps sampled RGB with $2C-1$, and requires the normal to be normalized before lighting. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

---

**Why tangent space**

Suppose the same normal-map texture is used on surfaces facing:

- +X,
- +Y,
- +Z.

If the map stored world-space normals, it could not simply rotate with the object.

Tangent space creates a local surface frame:

$$
T=\text{tangent},
$$

$$
B=\text{bitangent},
$$

$$
N=\text{surface normal}.
$$

A tangent-space map stores perturbations relative to that frame.

Thus the same texture naturally follows:

- object rotation,
- surface orientation,
- UV parameterization.

---

**TBN matrix**

Build:

$$
\boxed{
TBN=
\begin{bmatrix}
T&B&N
\end{bmatrix}
}
$$

Then:

$$
\boxed{
N_{\text{world}}
=
TBN\,N_{\text{tangent}}.
}
$$

For:

$$
N_t=(0,0,1),
$$

we obtain:

$$
N_w=N.
$$

For:

$$
N_t=(1,0,0),
$$

we obtain:

$$
N_w=T.
$$

So the tangent-space vector's XYZ coordinates mean weights along:

$$
T,B,N.
$$

---

**Building tangent and bitangent from geometry + UVs**

Take triangle edges:

$$
E_1=P_1-P_0,
$$

$$
E_2=P_2-P_0.
$$

Texture-coordinate differences:

$$
\Delta UV_1=
(u_1-u_0,v_1-v_0),
$$

$$
\Delta UV_2=
(u_2-u_0,v_2-v_0).
$$

We assume:

$$
E_1=
T\Delta u_1+
B\Delta v_1,
$$

$$
E_2=
T\Delta u_2+
B\Delta v_2.
$$

In matrix form:

$$
\begin{bmatrix}
E_1&E_2
\end{bmatrix}
=
\begin{bmatrix}
T&B
\end{bmatrix}
\begin{bmatrix}
\Delta u_1&\Delta u_2\\
\Delta v_1&\Delta v_2
\end{bmatrix}.
$$

The determinant is:

$$
D=
\Delta u_1\Delta v_2
-
\Delta u_2\Delta v_1.
$$

If:

$$
D=0,
$$

UV mapping is locally singular.

Otherwise:

$$
r=\frac1D.
$$

Then:

$$
\boxed{
T=
r(
E_1\Delta v_2
-
E_2\Delta v_1
)
}
$$

and

$$
\boxed{
B=
r(
E_2\Delta u_1
-
E_1\Delta u_2
).
}
$$

---

**Tangent should be orthogonalized**

Interpolated normals and tangents may cease to be exactly orthogonal.

Given normal $N$, Gram-Schmidt tangent correction:

$$
\boxed{
T'=
\operatorname{normalize}
(
T-N(N\cdot T)
)
}
$$

Then bitangent can be reconstructed as:

$$
B=
h(N\times T')
$$

where:

$$
h\in\{-1,+1\}
$$

is handedness.

This avoids storing an entire bitangent vector.

---

**Bitangent handedness**

Mirrored UV islands reverse texture-space orientation.

The determinant:

$$
D=
\Delta u_1\Delta v_2-
\Delta u_2\Delta v_1
$$

changes sign under such mirroring.

Therefore the tangent frame may be right-handed or left-handed.

A common representation stores tangent:

$$
(T_x,T_y,T_z,h)
$$

where:

$$
h=\pm1.
$$

Then:

$$
\boxed{
B=h(N\times T).
}
$$

If handedness is ignored, normal maps can invert or produce seams on mirrored UV islands.

---

**Canonical tangent construction**

```cpp
struct TangentFrame
{
    Vec3 tangent;
    Vec3 bitangent;
};

TangentFrame BuildTriangleTangentFrame(
    Vec3 p0,
    Vec3 p1,
    Vec3 p2,
    Vec2 uv0,
    Vec2 uv1,
    Vec2 uv2,
    Vec3 normal)
{
    const Vec3 e1 = p1 - p0;
    const Vec3 e2 = p2 - p0;

    const Vec2 dUV1 = uv1 - uv0;
    const Vec2 dUV2 = uv2 - uv0;

    const float determinant =
        dUV1.x * dUV2.y -
        dUV1.y * dUV2.x;

    if (std::abs(determinant) < 1.0e-8f)
    {
        // Degenerate UV parameterization.
        // A production implementation needs a stable fallback basis.
        const Vec3 tangent =
            BuildAnyPerpendicular(normal);

        return
        {
            tangent,
            Normalize(Cross(normal, tangent))
        };
    }

    const float r = 1.0f / determinant;

    Vec3 tangent =
        (e1 * dUV2.y -
         e2 * dUV1.y) * r;

    Vec3 bitangentRaw =
        (e2 * dUV1.x -
         e1 * dUV2.x) * r;

    // Remove any normal-direction component from T.
    tangent =
        Normalize(
            tangent -
            normal * Dot(normal, tangent));

    const float handedness =
        Dot(
            Cross(normal, tangent),
            bitangentRaw) < 0.0f
            ? -1.0f
            : 1.0f;

    const Vec3 bitangent =
        Cross(normal, tangent) *
        handedness;

    return { tangent, bitangent };
}
```

---

**Your current Kairo takes a different but mathematically related route**

Kairo does not currently require stored vertex tangents in its `MeshVertex`.

It has:

```text
Position
Color
Normal
TexCoord
```

Then in `triangle.frag` it reconstructs the tangent basis from derivatives:

```glsl
dpdx = dFdx(inWorldPosition);
dpdy = dFdy(inWorldPosition);

duvdx = dFdx(inTexCoord);
duvdy = dFdy(inTexCoord);
```

and computes:

$$
D=
u_xv_y-u_yv_x.
$$

Then:

$$
\boxed{
T=
\frac{
P_xv_y-P_yv_x
}{D}
}
$$

which is the same 2×2 inversion principle we derived.

The bitangent uses:

```glsl
cross(N, T) * sign(D)
```

to retain orientation.

That is mathematically coherent.

---

**Derivative-based tangent versus authored tangent**

Derivative reconstruction has advantages:

- no tangent vertex attribute,
- handles the actual interpolated UV/world geometry,
- convenient for dynamically generated geometry.

But authored/generated tangents are often preferable when matching a content-authoring tangent convention, especially across seams and baked normal maps.

Normal maps are frequently baked assuming a specific tangent-space generator, commonly MikkTSpace or an equivalent convention.

If your renderer reconstructs a different tangent frame than the baker used, visible seams can appear even if every vector is mathematically “reasonable.”

That will become important once asset fidelity becomes stricter.

---

**Applying a normal map**

A canonical GLSL path:

```glsl
vec3 geometricNormal = normalize(inNormal);

// World-space tangent frame.
vec3 tangent =
    normalize(
        inTangent.xyz -
        geometricNormal *
        dot(geometricNormal, inTangent.xyz));

float handedness =
    inTangent.w;

vec3 bitangent =
    handedness *
    cross(geometricNormal, tangent);

mat3 TBN =
    mat3(
        tangent,
        bitangent,
        geometricNormal);

// Normal-map textures are linear DATA.
// Do not sRGB-decode them.
vec3 tangentNormal =
    texture(normalMap, uv).xyz * 2.0 - 1.0;

// Optional material normal strength.
tangentNormal.xy *= normalScale;

// Re-normalize after scaling.
tangentNormal =
    normalize(tangentNormal);

vec3 shadingNormal =
    normalize(TBN * tangentNormal);
```

The glTF normal-scale convention similarly scales X/Y before normalization. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

---

**Height maps**

A height map stores:

$$
h(u,v)
$$

rather than a direction.

It describes scalar surface elevation.

A normal can be derived from its gradient.

Locally, if:

$$
z=h(u,v),
$$

then tangent directions are approximately:

$$
P_u=
(1,0,h_u),
$$

$$
P_v=
(0,1,h_v).
$$

Their cross product is:

$$
P_u\times P_v
=
(-h_u,-h_v,1).
$$

Thus:

$$
\boxed{
N
\propto
\left(
-\frac{\partial h}{\partial u},
-\frac{\partial h}{\partial v},
1
\right).
}
$$

This shows the relationship:

$$
\boxed{
\text{height map}
\rightarrow
\text{gradient}
\rightarrow
\text{normal map}.
}
$$

A normal map essentially stores local differential orientation directly.

---

**Height versus normal information**

Normal maps encode slope.

They generally lose absolute height.

Two height fields can differ by:

$$
h_2(u,v)=h_1(u,v)+100
$$

and have identical gradients:

$$
\nabla h_2=\nabla h_1.
$$

Therefore they yield identical normals.

So:

$$
\boxed{
\text{normal map does not uniquely determine height}.
}
$$

Height contains an integrable scalar field; normals contain local orientation.

---

**Parallax mapping**

Normal mapping changes lighting orientation but does not change where texture features appear as the camera moves.

Parallax mapping uses height to shift UVs according to view direction.

Transform viewing direction into tangent space:

$$
V_t=(V_x,V_y,V_z).
$$

A simple offset model is approximately:

$$
\boxed{
\Delta UV
=
s\,h(u,v)
\frac{(V_x,V_y)}{V_z}
}
$$

where $s$ controls apparent depth.

Then sample using:

$$
UV'=UV-\Delta UV
$$

depending on height convention.

As:

$$
V_z\rightarrow0
$$

at grazing angles, the shift becomes large.

That is where basic parallax approximations break down.

---

**Why parallax mapping is still fake geometry**

Parallax mapping changes texture lookup.

It does **not** move the rasterized surface.

Therefore it cannot correctly change:

- silhouette,
- true occlusion against other objects,
- physical intersection,
- cast-shadow geometry.

A brick wall may appear recessed internally but retain a perfectly flat silhouette.

Steep parallax and parallax occlusion mapping improve self-occlusion and ray-height intersection approximations, but still operate inside the original geometric surface.

---

**Displacement mapping**

Displacement actually changes position:

$$
\boxed{
P'=P+hN.
}
$$

Now the geometry itself moves.

Therefore displacement can affect:

- silhouette,
- depth,
- shadows,
- intersections,
- rasterized coverage.

But if the base mesh is too coarse, displacing only existing vertices cannot represent high-frequency height.

Therefore displacement often requires:

- dense meshes,
- tessellation,
- subdivision,
- mesh shader amplification,
- offline subdivision.

This is why displacement is more expensive but more geometrically correct.

---

**Normal vs parallax vs displacement**

Think of these as increasing levels of geometric commitment:

| Technique | Changes shading normal | Changes texture position | Changes geometry/depth | Changes silhouette |
|---|---:|---:|---:|---:|
| Normal map | Yes | No | No | No |
| Basic parallax | Usually + normal | Yes | No | No |
| POM / steep parallax | Yes | Yes + approximate self-occlusion | No | Usually no |
| Displacement | Yes indirectly | Geometry moves | Yes | Yes |

The best technique depends on viewing distance and geometric importance.

---

**Texture-space filtering is semantic filtering**

This is perhaps the most important M116 principle.

Different textures store different mathematical objects.

**Color map**

Stores radiometric/color quantity.

Correct reduction:

$$
\text{linearize}
\rightarrow
\text{filter}
\rightarrow
\text{encode}.
$$

**Normal map**

Stores directions.

Correct reduction:

$$
\text{decode vectors}
\rightarrow
\text{average}
\rightarrow
\text{renormalize}.
$$

**Roughness**

Stores a material parameter.

A straight arithmetic average may not preserve the appearance of a nonlinear BRDF perfectly.

**Alpha mask**

Simple averaging changes coverage.

Mip generation may need coverage-preserving filtering.

**Height**

Stores scalar geometry.

Its filtering may need to preserve extrema depending on use.

This means:

$$
\boxed{
\text{one generic RGBA mip generator is not physically correct for every texture semantic}.
}
$$

Kairo has already taken a strong first step by storing explicit `TextureSemantic`.

---

**Kairo current texture pipeline audit**

The current pipeline looks approximately like:

```text
asset image
    ↓
semantic classification
    ↓
TextureArtifact
    ├── ColorSpace = SRGB or Linear
    ├── Semantic = Color / Normal / Data / HDR
    ├── PixelFormat
    └── explicit mip chain
            ↓
VulkanTexture2D
    ↓
SRGB artifact
→ VK_FORMAT_*_SRGB

Linear artifact
→ VK_FORMAT_*_UNORM / SFLOAT
            ↓
VkSampler
    ├── linear magnification
    ├── linear minification
    ├── linear mip interpolation
    ├── repeat/mirror/clamp addressing
    └── anisotropy disabled
            ↓
triangle.frag
    ├── baseColorMap
    ├── normalMap
    ├── metallicRoughnessMap
    ├── emissiveMap
    └── occlusionMap
```

That is already a reasonably coherent color/texture architecture.

---

**Kairo's strongest correctness invariants today**

The important ones are:

$$
\boxed{
\text{Material factors are linear}
}
$$

$$
\boxed{
\text{Color textures may be sRGB}
}
$$

$$
\boxed{
\text{Normal/data/HDR textures must be linear}
}
$$

$$
\boxed{
\text{sRGB mip filtering is done in linear light}
}
$$

$$
\boxed{
\text{normal-map mip vectors are renormalized}
}
$$

$$
\boxed{
\text{Vulkan SRGB image formats perform sampling decode}
}
$$

These are exactly the sorts of invariants that prevent “why does everything look subtly wrong?” bugs.

---

**Important gaps in the current Kairo path**

There are also clear next steps.

**Anisotropic filtering**

Currently disabled.

Especially visible on:

- floors,
- roads,
- walls viewed obliquely.

**Texture compression**

No BC/ASTC/ETC artifact/runtime format path yet.

**Authored tangent-space compatibility**

Current tangent frame is derivative generated.

This may diverge from baked tangent-space conventions.

**HDR post pipeline**

Tone mapping currently happens inside the material fragment shader, before the HDR viewport image.

Ideally:

```text
material shading
→ HDR target
→ bloom/exposure/post
→ tone mapper
→ display transform
```

**Color-management expansion**

The current pipeline primarily assumes linear/sRGB semantics rather than a full working-space/display-transform architecture.

For serious film/image-fidelity ambitions, eventually you will care about:

```text
ACEScg-like working spaces
Display P3
Rec.2020
HDR display transforms
gamut mapping
```

but those are deliberately beyond the exact M115 atom list.

---

**A subtle texture bug: double sRGB decode**

Suppose a base-color texture uses:

```text
VK_FORMAT_R8G8B8A8_SRGB.
```

Sampling already returns linear values.

If your shader then does:

```glsl
color = pow(texture(...).rgb, vec3(2.2));
```

you decode it twice.

Result: dramatically too dark.

The rule is:

$$
\boxed{
\text{Know whether transfer decoding is performed by the texture format}
}
$$

before applying manual conversion.

---

**The opposite bug: treating sRGB bytes as linear**

If an ordinary PNG base-color image is uploaded as:

```text
VK_FORMAT_R8G8B8A8_UNORM
```

instead of SRGB, `texture()` returns its encoded values untouched.

Then the renderer multiplies those nonlinear values during lighting.

The result can look:

- too bright in midtones,
- incorrect in blends,
- wrong under mip filtering.

This is why texture semantic and color-space metadata cannot be an afterthought.

---

**A subtle normal-map bug: sRGB decode**

Suppose neutral normal:

$$
(0.5,0.5,1)
$$

is mistakenly treated as sRGB.

Decode 0.5:

$$
0.5\rightarrow0.214.
$$

Then convert to tangent normal:

$$
2(0.214)-1
=
-0.572.
$$

So instead of neutral:

$$
(0,0,1)
$$

you obtain approximately:

$$
(-0.572,-0.572,1).
$$

The whole surface appears tilted.

This is why:

$$
\boxed{\text{normal maps must be linear data}}
$$

is non-negotiable.

---

**LOD bias**

Texture lookup can apply a bias:

$$
\lambda'=\lambda+b.
$$

Positive bias:

$$
b>0
$$

selects lower-resolution mips earlier.

Result:

- blurrier,
- more stable,
- potentially faster.

Negative bias:

$$
b<0
$$

selects sharper mips.

Result:

- sharper,
- potentially more aliasing/shimmering.

Vulkan exposes sampler LOD bias directly in `VkSamplerCreateInfo`. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/samplers.html?utm_source=chatgpt.com)

This becomes particularly relevant with TAA/upscaling systems where a modest negative bias may be used to recover texture detail, but only if temporal reconstruction controls aliasing.

---

**Texture cache intuition**

Mipmaps are not merely anti-aliasing.

Suppose one distant pixel needs information from a 64×64 base-level footprint.

Without mips, accesses may touch many cache lines and still alias badly.

At the proper mip level, the same represented footprint may correspond to only a few nearby texels.

Therefore mipmapping improves:

$$
\text{quality}
$$

and:

$$
\text{memory locality/bandwidth}.
$$

This is why disabling mipmaps to “make textures sharper” is usually disastrous in large scenes.

---

**Texture compression and cache efficiency**

Compression also reduces bandwidth.

If uncompressed data costs:

$$
32\text{ bits/texel}
$$

and BC1 costs:

$$
4\text{ bits/texel},
$$

then many more texture blocks fit into:

- VRAM,
- caches,
- memory transactions.

Compression therefore improves not just capacity but potentially sampling throughput.

The trade-off is reconstruction error.

---

**Normal map strength**

Your current shader does:

```glsl
tangentNormal.xy *= normalScale;
```

before transforming into world space.

Why X/Y only?

Because:

$$
(x,y)
$$

encode lateral slope relative to the base normal axis.

Scaling them adjusts deviation from:

$$
(0,0,1).
$$

Then the result should be normalized.

The glTF convention similarly applies normal-scale to tangent-space X/Y. [Khronos Registry](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html?utm_source=chatgpt.com)

---

**Normal transformations and non-uniform scale**

Suppose model scale is:

$$
S=
\begin{bmatrix}
2&0&0\\
0&1&0\\
0&0&0.5
\end{bmatrix}.
$$

You cannot transform normals using the same matrix as positions.

Normals must remain perpendicular to transformed surface tangent vectors.

The correct normal transform is:

$$
\boxed{
N'
=
(M^{-1})^T N.
}
$$

Your Kairo pipeline already passes a separate normal matrix through push constants.

That becomes especially important when combining world-space derivative tangents with transformed geometric normals.

---

**Why normal mapping needs normalization several times**

Normalization appears in several places:

```text
interpolated geometric normal
    ↓ normalize

tangent after interpolation/derivation
    ↓ normalize/orthogonalize

sampled tangent-space normal
    ↓ decode
    ↓ optional scale
    ↓ normalize

TBN transformation
    ↓
world-space shading normal
    ↓ normalize
```

Skipping any of these can change BRDF behavior because dot products such as:

$$
N\cdot L
$$

assume unit vectors.

---

**Why interpolation destroys normal length**

Take two normals:

$$
N_0=(1,0,0),
$$

$$
N_1=(0,1,0).
$$

At midpoint:

$$
N=
0.5N_0+0.5N_1
=
(0.5,0.5,0).
$$

Length:

$$
\|N\|=
\sqrt{0.5}
\approx0.707.
$$

Therefore interpolated normals must be normalized before lighting.

This is not a numerical accident.

Linear interpolation of unit vectors does not generally stay on the unit sphere.

---

**The full color → texture → lighting dataflow**

For a base-color texture:

```text
PNG/JPEG encoded bytes
       ↓
classified as sRGB color
       ↓
mip generation:
decode → filter → re-encode
       ↓
VK_FORMAT_*_SRGB
       ↓
texture sampling
hardware sRGB → linear conversion
       ↓
linear baseColor
       ↓
BRDF / lighting arithmetic
       ↓
scene-linear HDR
       ↓
exposure
       ↓
tone mapping
       ↓
display-linear SDR/HDR signal
       ↓
display transfer encoding
       ↓
monitor
```

For a normal map:

```text
normal-map bytes
       ↓
classified as linear data
       ↓
linear mip generation:
decode vectors → filter → normalize → re-encode
       ↓
VK_FORMAT_*_UNORM
       ↓
texture sampling
       ↓
[0,1]^3
       ↓
2x - 1
       ↓
tangent-space normal
       ↓
normalScale
       ↓
normalize
       ↓
TBN
       ↓
world-space shading normal
       ↓
BRDF
```

If you can reconstruct these two paths, you understand much of this pass.

---

**Research bridge — image fidelity experiment**

A useful controlled experiment for M115/M116 would deliberately test each “small” color/sampling decision independently.

Build reference scenes containing:

- grayscale ramps,
- black/white checkerboards,
- high-frequency stripes,
- oblique textured planes,
- mirrored UV islands,
- normal-map spheres,
- extreme HDR emitters.

Then compare variants:

```text
correct sRGB decode
vs
treat sRGB as linear

linear mip averaging
vs
encoded-space averaging

normal-map renormalization
vs
raw RGB averaging

trilinear
vs
anisotropic

derivative-generated tangents
vs
authored MikkTSpace tangents

Reinhard
vs
filmic tone mapping
```

Measure:

$$
\text{RMSE},
$$

$$
\text{SSIM},
$$

temporal shimmer,

and, where appropriate, angular normal error:

$$
\boxed{
\theta_{\text{err}}
=
\cos^{-1}
(
N_{\text{ref}}\cdot N_{\text{test}}
)
}.
$$

For Maveb/Vulkax work, this matters because a renderer-validation experiment is meaningless if apparently geometric residuals are actually caused by:

- transfer-function mismatch,
- incorrect mip filtering,
- tangent mismatch,
- exposure/tone-map mismatch.

That is precisely why the curriculum labels M115/M116 as a shading foundation for image-fidelity research.

---

**PASS 3 RECALL SHEET**

| Cue | Reconstruct |
|---|---|
| Radiant energy | $Q$, joules |
| Flux | $\Phi=dQ/dt$, watts |
| Irradiance | $E=d\Phi/dA$, W/m² |
| Solid angle | $\Omega=A/r^2$, steradians |
| Radiance | $L=d^2\Phi/(dA\cos\theta\,d\omega)$ |
| Irradiance from radiance | $E=\int L_i\cos\theta\,d\omega$ |
| Constant hemispherical radiance | $E=\pi L$ |
| Vacuum ray | radiance invariant without absorption/scattering |
| Tristimulus | spectrum → three perceptual/color coordinates |
| Linear RGB | arithmetic proportional to light |
| sRGB decode | piecewise 12.92 / exponent 2.4 function |
| Lighting rule | decode first, compute in linear, encode last |
| Color space | primaries + white point + transfer function |
| HDR | scene values can exceed 1 |
| Exposure | $C'=2^{EV}C$ |
| Reinhard | $T(x)=x/(1+x)$ |
| Texture | sampled 2D signal |
| Texel vs pixel | source texture sample vs framebuffer sample |
| Bilinear | four texels |
| Mipmap | prefiltered pyramid |
| Full mip cost | ~4/3 base-level storage |
| LOD | $\lambda=\log_2\rho$ |
| Trilinear | bilinear at two mips + interpolate between mips |
| Anisotropy | elongated pixel footprint |
| Normal decode | $N_t=2C-1$ |
| Tangent determinant | $D=\Delta u_1\Delta v_2-\Delta u_2\Delta v_1$ |
| Tangent | $T=(E_1\Delta v_2-E_2\Delta v_1)/D$ |
| Bitangent | $B=h(N\times T)$ |
| Height-derived normal | $N\propto(-h_u,-h_v,1)$ |
| Parallax | shifts UV, not geometry |
| Displacement | $P'=P+hN$, changes actual geometry |
| Kairo color semantics | Color may be sRGB; Normal/Data/HDR linear |
| Kairo filtering | bilinear + trilinear; anisotropy currently off |
| Kairo normal frame | reconstructed from fragment derivatives |
| Kairo tone map | exposure + per-channel Reinhard currently inside material shader |

---

**Blank-screen reconstruction gate**

Without looking back, you should be able to rebuild this sequence:

```text
PHYSICAL LIGHT
     ↓
spectral energy
     ↓
RGB approximation
     ↓
radiance arriving at surface
     ↓
BRDF / lighting arithmetic
     ↓
scene-linear HDR RGB
     ↑
     │
base-color texture
sRGB stored
     ↓
decode to linear
     ↓
mip/filter in correct domain
     ↓
linear texture sample
     ┘

scene-linear HDR
     ↓
exposure: × 2^EV
     ↓
tone map
     ↓
display encoding
     ↓
screen
```

Then separately reconstruct:

$$
\mathbf d_x=
(Wu_x,Hv_x),
$$

$$
\mathbf d_y=
(Wu_y,Hv_y),
$$

$$
\rho=
\max(
\|\mathbf d_x\|,
\|\mathbf d_y\|
),
$$

$$
\lambda=\log_2\rho.
$$

And finally:

$$
E_1=T\Delta u_1+B\Delta v_1,
$$

$$
E_2=T\Delta u_2+B\Delta v_2
$$

leading to:

$$
T=
\frac{
E_1\Delta v_2-E_2\Delta v_1
}{
\Delta u_1\Delta v_2-
\Delta u_2\Delta v_1
}.
$$

If those equations, their units/domains, and their corresponding Kairo code paths can be reconstructed, M115/M116 are genuinely understood rather than vocabulary-memorized.


Textures attach spatially varying signals to surfaces, so filtering must follow their projected footprint.

### Texture is a function over a surface

A texture map stores a signal in a two-dimensional parameter domain. A surface point is assigned UV coordinates ((u,v)), and the renderer samples the image or function there. Parameterization is the mapping that flattens a surface to a domain; it inevitably introduces distortion, seams, or both. For a mesh triangle with per-vertex coordinates (u_i,v_i), barycentric interpolation transfers a hit point to texture space. If texture coordinates are outside the unit tile, the sampler needs a policy: repeat, clamp, mirror, or border color.

A sampled texture usually needs interpolation. If the query lies between four texel centers with fractional coordinates $a,b\in[0,1]$, bilinear interpolation blends horizontally and then vertically:

$$
T(u,v)=(1-b)((1-a)T_{00}+aT_{10})+b((1-a)T_{01}+aT_{11}).
$$

For instance, if the four texel values are 0, 1, 1, and 0 in row order, then at the center (a=b=0.5) the two horizontal interpolants are both 0.5, so the final result is 0.5. Nearest-neighbor sampling would instead jump between 0 and 1 at texel boundaries. Bilinear filtering removes that discontinuity, although it cannot recover detail that was lost when the image was sampled.

This produces a continuous-looking signal from discrete texels, but it is still a reconstruction filter. The texture lecture discusses parameterization, texture lookup, and bilinear interpolation, then connects shaders to procedural signals such as Perlin noise [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:25:00–00:45:00; 00:45:00–01:05:00). Procedural textures evaluate a function instead of storing every value, making them resolution-independent and useful for irregular natural patterns. Their high-frequency detail still has to be filtered when projected to a pixel grid.

### Texture coordinates make one surface address many kinds of detail

A triangle mesh describes where a surface is, but that is too coarse a storage scheme for fine color, roughness, or relief. Texture mapping gives each surface point a second address in a two-dimensional parameter domain. For a triangle with 3D corners $p_0,p_1,p_2$ and corresponding texture corners $u_0,u_1,u_2$, a hit point has barycentric weights $(\lambda_0,\lambda_1,\lambda_2)$, with $\lambda_i\ge 0$ and $\sum_i\lambda_i=1$. Reuse those weights in texture space:

$$
p=\sum_{i=0}^2\lambda_i p_i,\qquad u=\sum_{i=0}^2\lambda_i u_i.
$$

The mesh intersection answers where the ray hit in 3D; the second expression answers which texel neighborhood to sample. If $p$ has weights $(1/2,1/4,1/4)$, then the texture lookup is exactly the same weighted average of the three texture corners. This is why texture coordinates are attached to triangle corners rather than inferred from world position. A shared 3D vertex may have two different UV coordinates on opposite sides of a seam. If one vertex had to have only one UV address, seams could not open. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:15:34–00:21:04)

A UV atlas cuts a curved surface into patches before laying it flat. A triangle itself is planar and can be flattened without stretching, but the neighborhood around a curved vertex generally cannot be flattened while preserving every length and angle. A closed sphere cannot be continuously mapped to one flat disk without a cut or singularity. A designer therefore balances distortion against seam placement: seams hidden behind ears or under a character may be visually safer than a mathematically uniform cut. Leaving more edges connected improves continuity, but concentrates distortion; cutting every triangle removes much of that distortion while producing many discontinuous islands and difficult painting. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:21:46–00:25:58; 00:35:42–00:36:48)

UV generation is an optimization problem with both geometric and topological choices. Even with the cuts fixed, each mesh vertex contributes two unknowns. A local objective can penalize edge stretch or angle distortion. But a valid atlas must also avoid unintended overlap: if two unrelated triangles occupy the same UV region, they sample the same texture colors. The lecture's Tutte construction fixes the boundary to a convex shape and places each interior UV point at a weighted average of its neighbors. Each condition is linear, so all interior positions can be solved as a sparse linear system. This gives a useful injective starting map under the method's assumptions; later optimization may trade cut length against local distortion while preserving non-overlap. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:40:22–00:48:52)

Parameterization methods include artist-chosen seams, closed-form cylinder or sphere projections, and automatic optimization. A photo projected from a virtual camera is useful for image-based rendering, where a simple geometric proxy carries photographs from one or more views. It is less suitable for a freely turning object because a projection designed for one camera may stretch or slide under another. A tiled texture can reuse a small image with wraparound coordinates, while a normal map stores a perturbed normal rather than color. Normal mapping gives a low-polygon mesh fine shading variation, often baked from a high-resolution model, but cannot produce the true silhouette, parallax, or mutual occlusion of displaced geometry. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:37:49–00:40:20; 00:49:49–00:56:41)

A sampled image is a grid of texels (texture pixels), so a non-integer UV coordinate needs reconstruction. Nearest-neighbor lookup picks one texel and creates visible jumps. Bilinear filtering interpolates between four neighbors: interpolate horizontally in the two rows, then vertically between the results. The order gives the same bilinear polynomial. When the texture becomes small on screen, one output pixel covers many texture texels. Sampling only one can alias the pattern into flicker or moiré. A mip pyramid precomputes successively half-sized averages; selecting the level whose texel footprint roughly matches the screen footprint approximates the needed area average. The extra memory is bounded: for a square texture, the level sizes sum to $N^2(1+1/4+1/16+\cdots)=\frac43N^2$, ignoring tiny rounding effects. A surface tilted away from the camera has an elongated, direction-dependent footprint, so isotropic mip selection can blur too much in one direction; [[Convolution and filter footprints|anisotropic filtering]] samples that footprint more selectively. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:27:06–00:34:15)

Texture maps can hold any per-surface signal, not just RGB: diffuse coefficient, specular strength, roughness, or other BRDF parameters. Normal maps encode direction changes that perturb the normal used by lighting. Store directions in a coordinate frame that rotates with the surface (often tangent space), rather than naïvely storing world-space XYZ values that become invalid when the object moves. A coarse mesh with a normal map can reproduce the lighting of fine bumps from a distance, while remaining visibly flat in silhouette or when occlusion matters. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:49:49–00:56:41)

A procedural shader computes a surface signal from position or other attributes at shading time. For example, a checker can be built from parity of integer cell coordinates, so it extends indefinitely and has no stored image boundary. Procedural patterns offer compact, resolution-independent definitions, but they still require filtering: a mathematically sharp checkerboard can alias when many cells fall into one pixel. Perlin noise supplies smooth, locally varying values with a controllable spatial scale. Combining octaves at different frequencies creates multiscale structure; thresholding or taking absolute values introduces ridges, and perturbing a sinusoid with noise creates irregular marble veins. These are design functions, not physical laws. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-14 - Texture mapping|MITG-14]] (00:57:07–01:13:44)

> **Evidence boundary:** the available transcript runs through 01:14:23; video metadata gives a duration of 01:14:25. This is effectively full caption coverage, but no video frames were reviewed.

### Figure U9 - Homogeneous depth changes interpolation weights

![[Visual Recall/University Graphics and Vision/perspective-interpolation.png]]

**Read the figure.** A screen-space triangle has vertices $A_0=(0,0)$, $A_1=(2,0)$, and $A_2=(0,2)$. At point $P=(0.5,0.5)$, screen-space barycentric weights are $(\lambda_0,\lambda_1,\lambda_2)=(0.5,0.25,0.25)$; they sum to one and reconstruct $P$. The scalar vertex attributes are $a=(0,1,0)$ and clip-space homogeneous coordinates are $w=(1,2,4)$. Affine interpolation gives $\sum_i\lambda_i a_i=0.25$. Perspective-correct interpolation normalizes $\lambda_i/w_i$, yielding weights $(8/11,2/11,1/11)$ and attribute $2/11\approx0.181818$. This example treats the listed $w_i$ as the clip-space denominators used by the rasterizer; values are chosen to make the projective correction visible.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/18 - C++ Representation, Ownership and Lifetime|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/20 - Geometry Queries from Scene to Contact|Next chapter]]
