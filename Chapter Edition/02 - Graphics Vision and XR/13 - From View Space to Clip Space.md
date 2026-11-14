# 13 — From View Space to Clip Space

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility|Next chapter]]

A vertex shader moves positions through a sequence of coordinate spaces before clipping. Homogeneous coordinates make perspective projection possible and explain why clip space has four components.

**By the end:** Trace a vertex from view coordinates through clip coordinates and clipping.

**Recall question:** Why can perspective projection not be represented by a 3×3 affine transform in 3D?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 28320–31769; source-span SHA256 cfb093b9ae71f87be0582e488bddba421a963189da090b4c068283090e5435cd. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### View → clip

$$
\mathbf p_c=P\mathbf p_v.
$$

The projection matrix does something more interesting than “make things smaller with distance.”

It creates a **homogeneous representation** in which later division by $w_c$ produces perspective.

The result is

$$
\mathbf p_c=
(x_c,y_c,z_c,w_c).
$$

This is **clip space**.

It is not NDC.

That distinction is absolutely essential.

---

**Your actual vertex shader**

Current `KairoRenderer/shaders/triangle.vert` performs:

```glsl
const vec4 worldPosition =
    draw.model * vec4(inPosition, 1.0);

gl_Position =
    camera.projection * camera.view * worldPosition;
```

lines approximately **40–41**.

Therefore the actual Kairo path is:

```text
MeshVertex.Position
        ↓
draw.model
        ↓
worldPosition
        ↓
camera.view
        ↓
view
        ↓
camera.projection
        ↓
gl_Position = clip coordinates
```

The shader also explicitly forwards:

```text
vertex color
normal
world-space position
light clip position
UV
```

to the fragment stage.

That will become critical when we discuss interpolation.

---

**Why clip coordinates have four components**

Perspective projection cannot be represented as a plain 3×3 Euclidean linear transform.

The trick of projective geometry is to delay division.

We construct something approximately of the form

$$
x_c = s_xx_v,
$$

$$
y_c = s_yy_v,
$$

and

$$
w_c \propto -z_v
$$

for the common right-handed camera convention.

Then

$$
x_{\text{ndc}}
=
\frac{x_c}{w_c}
\approx
\frac{s_xx_v}{-z_v}.
$$

So as distance increases, projected size decreases.

The perspective effect therefore occurs specifically because:

$$
\boxed{x_{\text{ndc}} = x_c/w_c}
$$

and similarly for $y$.

The projection matrix prepares the numerator and denominator.

The divide creates perspective.

---

**Why clipping must happen before perspective division**

This is one of the most important conceptual points in M109.

Consider a triangle crossing the camera/near plane.

One vertex may have a valid positive homogeneous $w$; another may approach

$$
w\rightarrow0.
$$

If you divide first,

$$
x/w
$$

can explode toward infinity.

A triangle that is geometrically ordinary in projective space turns into absurd screen coordinates.

Therefore the logical order is:

$$
\boxed{
\text{clip-space primitive}
\rightarrow
\text{homogeneous clipping}
\rightarrow
\text{perspective divide}
}
$$

not:

$$
\text{divide}
\rightarrow
\text{try to clip giant screen triangle}.
$$

The current Vulkan specification defines the usual clip volume as

$$
-w_c\le x_c\le w_c,
$$

$$
-w_c\le y_c\le w_c,
$$

$$
0\le z_c\le w_c
$$

unless the optional negative-one-to-one depth convention is enabled. [Khronos Registry](https://registry.khronos.org/vulkan/specs/latest-ratified/pdf/vkspec.pdf?utm_source=chatgpt.com)

Notice that every inequality is expressed in terms of the homogeneous coordinate $w_c$.

That is why the stage is called **homogeneous clipping**.

---

**Clip planes as signed functions**

Each clipping boundary can be written as

$$
d(\mathbf v)\ge0.
$$

For example, the left plane

$$
x_c\ge-w_c
$$

becomes

$$
x_c+w_c\ge0.
$$

Define

$$
d_L(\mathbf v)=x_c+w_c.
$$

Similarly:

$$
d_R=w_c-x_c,
$$

$$
d_B=y_c+w_c,
$$

$$
d_T=w_c-y_c,
$$

$$
d_N=z_c,
$$

$$
d_F=w_c-z_c.
$$

Then a point is inside a particular plane when its corresponding $d$ is non-negative.

This unifies clipping into one operation.

---

**Clipping an edge against a plane**

Suppose edge endpoints are $A$ and $B$.

Their signed plane values are

$$
d_A=d(A),\qquad
d_B=d(B).
$$

The segment is

$$
P(t)=A+t(B-A),\quad t\in[0,1].
$$

Because a clip-plane function is linear,

$$
d(P(t))
=
d_A+t(d_B-d_A).
$$

At the intersection,

$$
d(P(t))=0.
$$

Therefore

$$
d_A+t(d_B-d_A)=0
$$

and

$$
\boxed{
t=
\frac{d_A}{d_A-d_B}
}.
$$

Then

$$
\boxed{
P=A+t(B-A)
}.
$$

That formula is the entire mathematical core of polygon clipping against one linear half-space.

---

**Canonical clipping code**

```cpp
struct ClipVertex
{
    Vec4 position;     // x_c, y_c, z_c, w_c

    // These are outputs produced by the pre-rasterization stage.
    // When clipping introduces a new vertex, they must be interpolated
    // consistently along the original primitive edge.
    Vec3 worldPosition;
    Vec3 normal;
    Vec2 uv;
};

template<class PlaneDistance>
std::vector<ClipVertex> ClipAgainstPlane(
    std::span<const ClipVertex> input,
    PlaneDistance distance)
{
    std::vector<ClipVertex> output;

    if (input.empty())
        return output;

    ClipVertex previous = input.back();
    float dPrevious = distance(previous.position);
    bool previousInside = dPrevious >= 0.0f;

    for (const ClipVertex& current : input)
    {
        const float dCurrent = distance(current.position);
        const bool currentInside = dCurrent >= 0.0f;

        // Crossing the plane means one endpoint is inside and the
        // other is outside. Generate the intersection vertex.
        if (currentInside != previousInside)
        {
            const float denominator = dPrevious - dCurrent;

            // dPrevious / (dPrevious - dCurrent)
            // is obtained by solving d(A + t(B-A)) = 0.
            const float t = dPrevious / denominator;

            ClipVertex intersection;

            intersection.position =
                previous.position +
                t * (current.position - previous.position);

            intersection.worldPosition =
                previous.worldPosition +
                t * (current.worldPosition - previous.worldPosition);

            intersection.normal =
                previous.normal +
                t * (current.normal - previous.normal);

            intersection.uv =
                previous.uv +
                t * (current.uv - previous.uv);

            output.push_back(intersection);
        }

        if (currentInside)
            output.push_back(current);

        previous       = current;
        dPrevious      = dCurrent;
        previousInside = currentInside;
    }

    return output;
}
```

This is essentially Sutherland–Hodgman clipping applied in homogeneous space.

For a complete Vulkan-style view volume, apply six planes sequentially.

The output polygon may contain more than three vertices and is then triangulated.

---

**Perspective divide**

Once a primitive has survived clipping, its clip-space positions undergo:

$$
\boxed{
x_d=\frac{x_c}{w_c},\qquad
y_d=\frac{y_c}{w_c},\qquad
z_d=\frac{z_c}{w_c}
}
$$

where the subscript $d$ denotes normalized device coordinates.

This is the perspective divide defined by Vulkan's coordinate transformation stage. [Khronos Registry](https://registry.khronos.org/vulkan/specs/latest-ratified/pdf/vkspec.pdf?utm_source=chatgpt.com)

Typical Vulkan NDC is

$$
x_d,y_d\in[-1,1]
$$

and

$$
z_d\in[0,1].
$$

Do not confuse that with traditional OpenGL's historical

$$
z\in[-1,1].
$$

This distinction becomes particularly important when building projection matrices.

---

**NDC is not screen space**

NDC is still resolution independent.

A vertex at

$$
(0,0)
$$

is the center regardless of whether the target is:

```text
800×600
1920×1080
3840×2160
```

The viewport transformation maps NDC into actual framebuffer coordinates.

For viewport width $W$, height $H$, origin $(x_0,y_0)$:

$$
x_f=x_0+\frac{W}{2}(x_d+1),
$$

$$
y_f=y_0+\frac{H}{2}(y_d+1)
$$

for a positive-height Vulkan viewport.

The formal Vulkan form is

$$
x_f=\frac{p_x}{2}x_d+o_x,
$$

$$
y_f=\frac{p_y}{2}y_d+o_y,
$$

$$
z_f=p_z z_d+o_z.
$$ 
<!-- Original citation marker: :chatgpt-content-reference{index="4"} -->



---

**The Vulkan Y problem and what Kairo actually does**

Framebuffer $y$ normally grows downward.

Mathematical camera coordinates normally treat positive $y$ as upward.

There are several ways to reconcile that.

One is a negative Vulkan viewport height.

Another is modifying the projection matrix.

Your current `Camera.cppm` chooses the second strategy.

Current lines roughly 115–140:

```cpp
Mat4f projection = Perspective(...);

// Vulkan positive-height viewport.
// Flip clip-space Y once.
projection(1u, 1u) *= -1.0f;
```

That is a good example of why coordinate-space conventions must be traced rather than guessed.

Your pipeline is effectively:

```text
world Y-up
    ↓
view
    ↓
projection
    ↓
explicit clip-space Y flip
    ↓
positive-height Vulkan viewport
    ↓
framebuffer coordinates
```

If you later introduced a negative-height viewport without removing this projection flip, you would flip twice.

That is a classic “everything is upside down” bug.

---

**M110 begins — screen-space triangle setup**

After viewport transformation, suppose the projected triangle is

$$
v_0=(2,2),\quad
v_1=(8,3),\quad
v_2=(4,9).
$$

The rasterizer needs to answer:

> Which sample locations lie within this triangle?

It does not perform a point-in-polygon test from scratch for every sample.

Triangles admit an extremely efficient affine representation using **edge equations**.

---

**Edge functions**

For directed edge $a\rightarrow b$, define

$$
\boxed{
E_{ab}(p)
=
(b_x-a_x)(p_y-a_y)
-
(b_y-a_y)(p_x-a_x)
}
$$

This is a 2D cross product.

Geometrically,

$$
E_{ab}(p)
$$

is proportional to the signed area of the parallelogram formed by

$$
b-a
$$

and

$$
p-a.
$$

Its sign says which side of the directed line $p$ lies on.

It can be expanded into

$$
E(x,y)=Ax+By+C.
$$

Specifically,

$$
A=-(b_y-a_y),
$$

$$
B=b_x-a_x,
$$

$$
C=(b_y-a_y)a_x-(b_x-a_x)a_y.
$$

Now something wonderful happens.

Moving one pixel in $x$:

$$
E(x+1,y)=E(x,y)+A.
$$

Moving one pixel in $y$:

$$
E(x,y+1)=E(x,y)+B.
$$

So once an edge equation has been evaluated at one location, nearby samples can be evaluated through additions instead of recomputing everything.

This is why edge equations map beautifully to hardware rasterizers.

---

**Triangle orientation and signed area**

Take

$$
v_0,v_1,v_2.
$$

The doubled signed triangle area is

$$
\boxed{
A_2=E_{v_0v_1}(v_2)
}.
$$

If

$$
A_2>0
$$

under our chosen $y$-up convention, the vertices are counter-clockwise.

If

$$
A_2<0,
$$

they are clockwise.

If

$$
A_2=0,
$$

the triangle is degenerate.

Three points lie on one line.

No finite-area region exists to rasterize.

This same area quantity will shortly become the denominator of barycentric coordinates.

So one determinant drives:

$$
\text{orientation},
\quad
\text{degeneracy},
\quad
\text{coverage},
\quad
\text{barycentrics}.
$$

That is a very important unification.

---

**Inside-triangle tests**

Assume the triangle orientation has been normalized consistently.

A sample $p$ is inside when it lies on the interior side of all three edges.

Conceptually:

$$
E_{01}(p)\ge0,
$$

$$
E_{12}(p)\ge0,
$$

$$
E_{20}(p)\ge0.
$$

That gives a complete triangle point test.

The rasterizer therefore computes a bounding rectangle:

$$
x_{\min}=
\lfloor\min(x_0,x_1,x_2)\rfloor,
$$

etc.,

then only visits samples inside that box.

This converts a theoretical scan over the entire framebuffer into work approximately proportional to the triangle's screen footprint.

---

**Why `>= 0` is not sufficient**

Imagine two adjacent triangles sharing exactly one edge.

Triangle A says the shared edge belongs to it.

Triangle B also says the shared edge belongs to it.

Then shared-edge pixels can be shaded twice.

If instead both use a strict test,

$$
E>0,
$$

neither triangle owns the exact boundary.

Then cracks can appear.

Hardware therefore needs a deterministic ownership convention.

The common convention is the **top-left rule**.

Samples exactly on particular top/left boundary edges count as covered; samples exactly on the complementary bottom/right edges do not.

The crucial invariant is:

$$
\boxed{
\text{a shared edge is owned by exactly one adjacent triangle}
}
$$

rather than the wording “top” or “left” itself.

Hardware typically evaluates this using fixed-point/subpixel edge equations so neighboring primitives agree bit-for-bit about the shared edge.

This is one reason naive CPU rasterizers using arbitrary floating-point epsilons can disagree with GPU output at triangle boundaries.

---

**Pixel centers**

A pixel is an area.

Rasterization uses one or more **sample positions** within that area.

Under conventional single-sample rasterization, a logical pixel at integer coordinate

$$
(i,j)
$$

is normally tested at something equivalent to its center,

$$
(i+0.5,j+0.5).
$$

So asking whether pixel $(7,10)$ is covered does not usually mean evaluating the edge equations at

$$
(7,10).
$$

It means evaluating at the specified sample location inside that pixel.

That half-pixel detail is enough to generate visibly shifted rasterization when implemented incorrectly.

---

**MSAA sample positions**

With 4× MSAA, a pixel conceptually contains four coverage samples:

```text
+----------------+
|    •       •   |
|                |
|       pixel    |
|                |
| •          •   |
+----------------+
```

Their precise locations are implementation-defined or queryable depending on the graphics API/features.

Rasterization now produces a **coverage mask** such as

```text
sample 0 = covered
sample 1 = covered
sample 2 = not covered
sample 3 = covered
```

or binary

```text
1011
```

rather than a single yes/no pixel decision.

This is why MSAA improves geometric edge quality: coverage is measured more finely even if the fragment shader is not necessarily executed independently for every sample.

We will go much deeper into this in M111.

---

**Barycentric coordinates emerge directly from the same edge equations**

For triangle

$$
v_0,v_1,v_2
$$

and point $p$, define

$$
A=E_{12}(v_0).
$$

Then

$$
\boxed{
\lambda_0=
\frac{E_{12}(p)}{A}
}
$$

$$
\boxed{
\lambda_1=
\frac{E_{20}(p)}{A}
}
$$

$$
\boxed{
\lambda_2=
\frac{E_{01}(p)}{A}
}
$$

with

$$
\lambda_0+\lambda_1+\lambda_2=1.
$$

The geometry is beautiful.

Each numerator is the area of a subtriangle opposite one vertex.

Therefore at $v_0$:

$$
\lambda_0=1,\quad
\lambda_1=0,\quad
\lambda_2=0.
$$

At the centroid:

$$
\lambda_0=
\lambda_1=
\lambda_2=
\frac13.
$$

The same rasterization state has therefore simultaneously answered:

> Is this point inside?

and:

> Where inside the triangle is it?

---

**Affine attribute interpolation**

Suppose vertex colors are

$$
C_0=(1,0,0),
$$

$$
C_1=(0,1,0),
$$

$$
C_2=(0,0,1).
$$

For affine interpolation:

$$
\boxed{
C(p)=
\lambda_0C_0+
\lambda_1C_1+
\lambda_2C_2
}
$$

At the centroid:

$$
C=
\frac13(1,0,0)
+
\frac13(0,1,0)
+
\frac13(0,0,1)
$$

giving

$$
C=
\left(
\frac13,\frac13,\frac13
\right).
$$

That works perfectly when the quantity truly varies affinely in screen space.

But ordinary perspective-projected attributes do not.

This creates one of rasterization's most important traps.

---

**Why naive screen-space interpolation is wrong**

Imagine an edge in 3D with endpoints

$$
A,\qquad B
$$

at different depths.

A texture coordinate varies linearly along the actual 3D edge.

After perspective projection, however, equal intervals in world/view space do **not** become equal intervals on screen.

Perspective performs approximately

$$
x'=\frac{x}{z}.
$$

That division is nonlinear.

Therefore if you calculate screen barycentrics and directly perform

$$
u=
\lambda_0u_0+
\lambda_1u_1+
\lambda_2u_2,
$$

the result is generally not the texture coordinate corresponding to the original projected 3D point.

This causes the classic affine-texture “swimming” or warping seen in early software renderers and some older consoles.

---

**Deriving perspective-correct interpolation**

Let the original primitive-space barycentric weights be

$$
\alpha_i.
$$

A pre-projection varying $a$ satisfies

$$
a=
\sum_i\alpha_i a_i.
$$

After perspective division, the screen-space barycentrics become different values

$$
\lambda_i.
$$

The relationship is

$$
\alpha_i
=
\frac{\lambda_i/w_i}
{\sum_j\lambda_j/w_j}.
$$

Substitute that into the attribute equation:

$$
a=
\sum_i
a_i
\frac{\lambda_i/w_i}
{\sum_j\lambda_j/w_j}.
$$

The denominator is shared:

$$
\boxed{
a(p)
=
\frac{
\displaystyle
\sum_i\lambda_i\frac{a_i}{w_i}
}{
\displaystyle
\sum_i\lambda_i\frac1{w_i}
}
}
$$

This is the perspective-correct interpolation equation.

The implementation interpretation is even more important than memorizing it.

At every vertex, think of storing:

$$
\frac{a_i}{w_i}
$$

and

$$
\frac1{w_i}.
$$

Rasterization linearly interpolates those screen-space quantities.

Then reconstruct:

$$
a=
\frac{a/w}{1/w}.
$$

So:

$$
\boxed{
\text{interpolate reciprocal }w
}
$$

is not some random graphics trick.

It is what mathematically undoes the projective distortion introduced by dividing coordinates by $w$.

---

**A numerical example**

Suppose two endpoints have

$$
u_0=0,\qquad w_0=1
$$

and

$$
u_1=1,\qquad w_1=4.
$$

At the screen midpoint,

$$
\lambda_0=\lambda_1=0.5.
$$

Naive interpolation gives

$$
u_{\text{naive}}
=
0.5(0)+0.5(1)=0.5.
$$

Perspective-correct interpolation instead computes

$$
\frac{u_0}{w_0}=0,
$$

$$
\frac{u_1}{w_1}=0.25,
$$

and

$$
\frac1{w_0}=1,\qquad
\frac1{w_1}=0.25.
$$

Therefore

$$
u=
\frac{
0.5(0)+0.5(0.25)
}{
0.5(1)+0.5(0.25)
}
$$

$$
=
\frac{0.125}{0.625}
$$

$$
=
0.2.
$$

Not

$$
0.5.
$$

That is a massive difference.

The nearer endpoint occupies more of the apparent screen-space perspective relationship.

---

**Canonical interpolation code**

```cpp
template<typename T>
T PerspectiveInterpolate(
    const T& a0,
    const T& a1,
    const T& a2,
    float w0,
    float w1,
    float w2,
    float lambda0,
    float lambda1,
    float lambda2)
{
    // Interpolate reciprocal homogeneous W.
    const float reciprocalW =
        lambda0 / w0 +
        lambda1 / w1 +
        lambda2 / w2;

    // Interpolate attribute/W in screen space.
    const T numerator =
        a0 * (lambda0 / w0) +
        a1 * (lambda1 / w1) +
        a2 * (lambda2 / w2);

    // Recover the original varying.
    return numerator / reciprocalW;
}
```

That one function connects Parts 1832–1835.

---

**`smooth`, `noperspective`, and `flat`**

There are conceptually three useful interpolation behaviors.

The normal perspective mode is equivalent to:

```glsl
smooth
```

and uses the equation we derived.

`noperspective` instead performs plain framebuffer-space interpolation:

$$
a=
\lambda_0a_0+
\lambda_1a_1+
\lambda_2a_2.
$$

That can be desirable for screen-space quantities.

`flat` performs no interpolation at all. One provoking vertex supplies the primitive's value.

Integer varyings are generally flat because interpolating discrete IDs such as

```text
material ID = 17
object ID = 42
```

would make no semantic sense.

---

**Fragment shader invocation**

Once a covered location survives enough rasterization setup, the implementation can generate a fragment shader invocation.

Conceptually the shader receives interpolated inputs:

```glsl
layout(location=0) in vec3 inColor;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in vec4 inLightClipPosition;
layout(location=4) in vec2 inTexCoord;
```

That is exactly the current `triangle.frag` interface in your Kairo renderer.

The fragment shader does **not** receive the three original vertices.

The rasterizer has already transformed those per-vertex outputs into per-fragment interpolated values.

That distinction separates the pre-rasterization and post-rasterization stages.

---

**Screen-space derivatives**

Now consider:

```glsl
dFdx(value)
dFdy(value)
```

These are not symbolic calculus operations performed by a compiler.

The GPU executes fragment work in small neighboring groups—traditionally reasoned about as $2\times2$ quads.

Suppose a varying $f$ has values approximately

```text
f00    f10

f01    f11
```

Then a horizontal derivative can be approximated through neighboring invocation differences such as

$$
\frac{\partial f}{\partial x}
\approx
f_{10}-f_{00},
$$

and vertically,

$$
\frac{\partial f}{\partial y}
\approx
f_{01}-f_{00}.
$$

Vulkan/SPIR-V formally defines fine/coarse derivative behavior over derivative groups/quads; ordinary derivative instructions may use an implementation-selected fine or coarse form. Implicit-LOD texture sampling itself relies on coordinate derivatives. [Khronos Registry](https://registry.khronos.org/vulkan/specs/latest/pdf/vkspec.pdf?utm_source=chatgpt.com)

This is why fragments outside the actual primitive can sometimes exist as **helper invocations**: a neighboring live fragment may need their values to calculate derivatives.

They participate in computation but must not produce ordinary framebuffer side effects.

---

**What your Kairo fragment shader actually uses derivatives for**

This is one of the strongest repository mappings in this pass.

Current `triangle.frag`, lines approximately **121–136**:

```glsl
const vec3 geometricNormal = normalize(inNormal);

const vec3 dpdx = dFdx(inWorldPosition);
const vec3 dpdy = dFdy(inWorldPosition);

const vec2 duvdx = dFdx(inTexCoord);
const vec2 duvdy = dFdy(inTexCoord);

const float determinant =
    duvdx.x * duvdy.y -
    duvdx.y * duvdy.x;
```

Then it reconstructs a tangent:

```glsl
tangent =
    (dpdx * duvdy.y -
     dpdy * duvdx.y) /
    determinant;
```

This is solving the local differential system

$$
dP
=
T\,du+B\,dv.
$$

Across screen $x$:

$$
P_x
=
T\,u_x+B\,v_x
$$

and across screen $y$:

$$
P_y
=
T\,u_y+B\,v_y.
$$

Write this as

$$
\begin{bmatrix}
P_x & P_y
\end{bmatrix}
=
\begin{bmatrix}
T & B
\end{bmatrix}
\begin{bmatrix}
u_x & u_y\\
v_x & v_y
\end{bmatrix}.
$$

The UV Jacobian determinant is

$$
D=u_xv_y-u_yv_x.
$$

Solving gives

$$
\boxed{
T=
\frac{
P_xv_y-P_yv_x
}{D}
}
$$

which is exactly what your shader implements.

It checks

$$
|D|>10^{-8}
$$

before dividing.

So the code is not merely “using dFdx for normal maps.”

It is solving a local $2\times2$ coordinate transformation between screen-space variation and texture-space variation.

That is the math-to-code bridge your curriculum is asking for.

---

**Derivative failure cases**

If UVs collapse locally,

$$
D\approx0,
$$

the tangent basis becomes singular.

Examples include degenerate UV triangles, duplicated UV coordinates and extremely compressed texture parameterization.

Then

$$
1/D
$$

becomes enormous and the tangent can explode numerically.

Your shader handles this by leaving the geometric normal in place when

$$
|D|\le10^{-8}.
$$

There is another important failure mode: derivatives inside highly divergent control flow.

If neighboring invocations do not execute compatible paths, derivative availability and meaning become tricky. Shader languages and APIs impose specific derivative-group rules; this is why derivative-based operations should not be casually hidden behind arbitrary per-fragment branches.

---

**Depth testing — what problem it solves**

Suppose two triangles project onto the same sample.

Triangle A produced depth

$$
z_A=0.3.
$$

Triangle B produced

$$
z_B=0.7.
$$

Under conventional Vulkan depth where smaller values are closer, A should win.

The depth buffer stores visibility history.

Conceptually:

```cpp
if (incomingDepth < storedDepth)
{
    storedDepth = incomingDepth;
    writeColor();
}
```

Without a depth buffer, visibility would depend almost entirely on draw order.

---

**Depth buffer representation**

Your current Kairo implementation uses

```cpp
VK_FORMAT_D32_SFLOAT
```

in `VulkanDepth.cppm`.

So each depth sample has a 32-bit floating-point depth component.

The image is created with

```cpp
VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
```

and resides in device-local memory.

Current `VulkanTriangle` rendering clears depth to:

```cpp
depthClear.depthStencil = { 1.0f, 0u };
```

and uses viewport:

```cpp
minDepth = 0.0f;
maxDepth = 1.0f;
```

The pipeline currently performs:

```cpp
depth.depthTestEnable  = VK_TRUE;
depth.depthWriteEnable = VK_TRUE;
depth.depthCompareOp   = VK_COMPARE_OP_LESS;
```

around current lines **541–545**.

Therefore Kairo currently means:

```text
nearer → smaller depth
farther → larger depth

clear = 1
accept incoming if incoming < stored
```

That is conventional depth, not reversed-Z.

---

**Depth comparison functions**

Common comparisons conceptually include

```text
NEVER
LESS
EQUAL
LESS_OR_EQUAL
GREATER
NOT_EQUAL
GREATER_OR_EQUAL
ALWAYS
```

The choice is not cosmetic.

For conventional Z:

$$
\text{clear}=1,\qquad
\text{compare}=LESS
$$

is typical.

For reversed-Z:

$$
\text{clear}=0,\qquad
\text{compare}=GREATER
$$

or `GREATER_OR_EQUAL` is typical.

If you reverse the projection but forget the comparison function, almost everything disappears.

If you reverse the compare function but forget to change the clear value, nearly everything also disappears.

These parameters form one invariant.

---

**Depth write control**

Depth **testing** and depth **writing** are separate.

Opaque rendering normally uses:

```text
depth test  = enabled
depth write = enabled
```

because the nearest opaque surface should occlude later geometry.

Transparent rendering commonly uses:

```text
depth test  = enabled
depth write = disabled
```

because transparent fragments should generally be tested against opaque geometry without making the first transparent surface permanently hide every transparent layer behind it.

Your Kairo pipeline already reflects this distinction.

Its opaque pipeline is created with depth writes enabled, while the transparent path requests depth writes disabled.

That becomes important in Pass 2 when we cover transparency ordering.

---

**Early versus late depth testing**

The conceptual pipeline might be drawn:

```text
rasterize
  ↓
fragment shader
  ↓
depth test
```

but that can waste enormous work.

Suppose an expensive fragment shader executes 200 instructions and the resulting fragment is completely hidden behind existing geometry.

If the GPU already knows its depth will fail, it would prefer:

```text
rasterize
  ↓
depth test
  ↓
if visible → execute shader
```

That is **early depth testing**.

Vulkan explicitly models early-fragment-test and late-fragment-test pipeline stages: early tests occur before fragment shading, late tests after it. [Khronos Registry](https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkPipelineStageFlagBits2KHR.html?utm_source=chatgpt.com)

But whether an implementation can perform the optimization depends on shader behavior.

Things such as writing fragment depth, certain side effects, sample-mask manipulation or discard behavior can constrain what may safely happen early.

Your current Kairo fragment shader performs:

```glsl
if (alphaMask && alpha < cutoff)
    discard;
```

so you should not imagine the implementation as a simplistic immutable stage diagram.

Real GPUs often separate:

```text
early rejection
fragment execution
late/commit tests
```

and use hierarchical visibility optimizations around them.

---

**Why perspective depth is nonlinear**

This is the foundation of Parts 1844–1845.

Let positive camera distance be

$$
d=-z_v>0.
$$

For a finite near plane $n$ and far plane $f$, conventional Vulkan-style normalized depth can be expressed as

$$
\boxed{
z(d)
=
\frac{f}{f-n}
-
\frac{fn}{(f-n)d}
}
$$

Check the near plane:

$$
d=n.
$$

Then

$$
z(n)=
\frac{f}{f-n}
-
\frac{f}{f-n}
=0.
$$

Check the far plane:

$$
d=f.
$$

Then

$$
z(f)=
\frac{f}{f-n}
-
\frac{n}{f-n}
=1.
$$

So:

$$
n\rightarrow0,
\qquad
f\rightarrow1.
$$

But note the $1/d$ term.

Depth is not linear with world-space distance.

---

**Why most conventional depth precision sits near the camera**

Differentiate:

$$
z(d)=A-\frac{B}{d}
$$

where

$$
B=\frac{fn}{f-n}.
$$

Then

$$
\boxed{
\frac{dz}{dd}
=
\frac{B}{d^2}
}
$$

The change in depth value per metre therefore falls roughly as

$$
\frac1{d^2}.
$$

Far away, huge changes in world distance produce tiny depth-buffer differences.

That is where Z-fighting comes from.

Two surfaces can have different actual distances but quantize to effectively the same depth value.

---

**Your camera settings show why this matters**

Current `CameraPose` defaults include approximately:

$$
n=0.1
$$

and

$$
f=5000.
$$

The ratio is

$$
\frac fn=50000.
$$

For those values, conventional depth gives approximately:

| Distance $d$ | Conventional depth |
|---:|---:|
| $0.1$ | $0$ |
| $1$ | $0.900018$ |
| $10$ | $0.9900198$ |
| $100$ | $0.99901998$ |
| $1000$ | $0.999919998$ |
| $5000$ | $1$ |

Look at what happened.

The first 10 metres consumed nearly the entire numerical interval.

Everything from hundreds to thousands of metres is crushed extremely close to 1.

This is why casually reducing the near plane from

$$
0.1
$$

to

$$
0.001
$$

can destroy depth precision.

The near plane is not only a clipping parameter.

It is a precision-control parameter.

---

**Reversed-Z**

Reversed-Z swaps the mapping:

$$
\boxed{
\text{near}\rightarrow1
}
$$

$$
\boxed{
\text{far}\rightarrow0
}
$$

For finite far plane, one suitable depth mapping is

$$
\boxed{
z_R(d)
=
-\frac{n}{f-n}
+
\frac{fn}{(f-n)d}
}
$$

Check:

$$
z_R(n)=1,
$$

$$
z_R(f)=0.
$$

Notice also:

$$
z_R(d)=1-z_{\text{conventional}}(d).
$$

For your $0.1$ to $5000$ camera:

| Distance | Reversed depth |
|---:|---:|
| $0.1$ | $1$ |
| $1$ | $0.099982$ |
| $10$ | $0.0099802$ |
| $100$ | $0.0009800$ |
| $1000$ | $0.0000800$ |
| $5000$ | $0$ |

With an infinite far plane,

$$
f\rightarrow\infty,
$$

this simplifies beautifully:

$$
\boxed{
z_R(d)=\frac nd
}
$$

which yields

$$
d=n\rightarrow1
$$

and

$$
d\rightarrow\infty\rightarrow0.
$$

---

**Why reversed-Z helps floating-point depth so much**

A common incomplete explanation is:

> “Reversed-Z has more precision.”

That is not enough.

If the depth buffer were uniformly spaced fixed-point values, merely replacing $z$ with $1-z$ would not magically create more representable levels.

The important interaction is:

$$
\boxed{
\text{projective }1/d\text{ depth distribution}
+
\text{floating-point relative precision}
}
$$

IEEE floating point has dense representable numbers near zero.

Conventional depth sends distant geometry toward

$$
1,
$$

where floating-point values are relatively coarse in absolute spacing.

Reversed-Z sends distant geometry toward

$$
0,
$$

where floating-point representation provides progressively smaller absolute spacing.

Those two nonlinearities partially counteract each other.

This is why:

$$
\boxed{\text{reversed-Z + floating-point depth}}
$$

is particularly powerful.

Your existing `D32_SFLOAT` format is therefore already a good format for adopting reversed-Z.

---

**Converting Kairo to reversed-Z**

At a conceptual level you would need one coherent change set.

Current state:

```cpp
clearDepth = 1.0f;
compare    = VK_COMPARE_OP_LESS;
projection = conventional depth;
```

Reversed state:

```cpp
clearDepth = 0.0f;
compare    = VK_COMPARE_OP_GREATER;
projection = reversed-Z depth mapping;
```

If using an infinite reversed projection, the far-plane term can often disappear from the perspective projection entirely.

But do not blindly patch only those two Vulkan values.

You must audit every place that assumes:

```text
smaller depth = closer
```

including shadow maps, depth reconstruction, linear-depth debug views, SSAO/SSR later, occlusion systems, Hi-Z reduction direction and any manual depth comparison shader.

For example, your current shadow shader performs:

```glsl
projected.z - receiverBias > storedDepth
```

for shadow-map comparison.

That shadow pipeline currently has its own conventional depth semantics.

Changing camera depth and changing shadow depth are separable decisions.

That is exactly the sort of cross-system assumption that breaks when reversed-Z is treated as a one-line optimization.

---

**A minimal CPU representation of M109 + M110**

Here is the core mechanism in one coherent skeleton.

```cpp
struct RasterVertex
{
    // Homogeneous position produced by vertex processing.
    Vec4 clip;

    // Varyings produced by the vertex shader.
    Vec3 worldPosition;
    Vec3 normal;
    Vec2 uv;
};

struct ScreenVertex
{
    Vec2 screen;

    // Retain reciprocal W because perspective interpolation needs it.
    float invW;

    // Pre-divided varyings.
    Vec3 worldPositionOverW;
    Vec3 normalOverW;
    Vec2 uvOverW;

    // Depth after projection.
    float depth;
};

ScreenVertex ToScreen(
    const RasterVertex& v,
    float viewportWidth,
    float viewportHeight)
{
    const float invW = 1.0f / v.clip.w;

    const Vec3 ndc{
        v.clip.x * invW,
        v.clip.y * invW,
        v.clip.z * invW
    };

    ScreenVertex out{};

    out.screen.x =
        (ndc.x * 0.5f + 0.5f) * viewportWidth;

    out.screen.y =
        (ndc.y * 0.5f + 0.5f) * viewportHeight;

    out.depth = ndc.z;

    out.invW = invW;

    out.worldPositionOverW =
        v.worldPosition * invW;

    out.normalOverW =
        v.normal * invW;

    out.uvOverW =
        v.uv * invW;

    return out;
}

float Edge(
    Vec2 a,
    Vec2 b,
    Vec2 p)
{
    return
        (b.x - a.x) * (p.y - a.y) -
        (b.y - a.y) * (p.x - a.x);
}

void ShadeSample(
    const ScreenVertex& v0,
    const ScreenVertex& v1,
    const ScreenVertex& v2,
    Vec2 sample,
    float& storedDepth)
{
    const float area =
        Edge(v0.screen, v1.screen, v2.screen);

    // Degenerate projected triangle.
    if (std::abs(area) < 1.0e-8f)
        return;

    const float l0 =
        Edge(v1.screen, v2.screen, sample) / area;

    const float l1 =
        Edge(v2.screen, v0.screen, sample) / area;

    const float l2 =
        Edge(v0.screen, v1.screen, sample) / area;

    // Simplified educational coverage test.
    // Production rasterization additionally implements exact
    // top-left ownership and subpixel rules.
    if (l0 < 0.0f ||
        l1 < 0.0f ||
        l2 < 0.0f)
    {
        return;
    }

    const float invW =
        l0 * v0.invW +
        l1 * v1.invW +
        l2 * v2.invW;

    const float w =
        1.0f / invW;

    const Vec2 uv =
        (
            l0 * v0.uvOverW +
            l1 * v1.uvOverW +
            l2 * v2.uvOverW
        ) * w;

    const Vec3 worldPosition =
        (
            l0 * v0.worldPositionOverW +
            l1 * v1.worldPositionOverW +
            l2 * v2.worldPositionOverW
        ) * w;

    // Depth handling shown conventionally here.
    // Exact hardware interpolation semantics are pipeline-defined.
    const float depth =
        l0 * v0.depth +
        l1 * v1.depth +
        l2 * v2.depth;

    if (depth >= storedDepth)
        return;

    storedDepth = depth;

    // Fragment shading would now consume things such as:
    // worldPosition
    // normal
    // uv
}
```

This is not yet our full production CPU rasterizer—that belongs especially to M114—but every central mechanism from this pass is now visible as executable machinery.

---

**What hardware does differently**

A production GPU does not literally run the nested C++ loop above.

The conceptual stages remain valid, but implementations aggressively optimize them.

Triangle setup hardware generates edge equations.

Screen coordinates commonly use fixed/subpixel representations to make edge ownership stable.

Triangles may be distributed into tiles.

Coverage may operate on hierarchical blocks rather than one isolated sample at a time.

Depth may be rejected hierarchically before full fragment execution.

Interpolation coefficients may be set up once per primitive.

Fragment execution occurs in SIMD/SIMT groups.

Texture LOD selection reuses quad derivatives.

Hidden-surface rejection can eliminate huge amounts of later work.

That is an important distinction:

$$
\boxed{
\text{logical pipeline}
\neq
\text{literal sequential silicon pipeline}
}
$$

The API guarantees observable behavior, not that the GPU contains one physical box corresponding to every diagram arrow.

---

**Full repository trace for this pass**

The curriculum specifically names `VulkanTriangle.cppm`, the shader directory and `VulkanDepth.cppm` for M109/M110.

I inspected current `main` rather than relying on those filenames alone.

| Concept | Current Kairo evidence |
|---|---|
| Vertex record | `Mesh.cppm:23–29` — position/color/normal/UV |
| Indexed mesh | `Mesh.cppm:39–49,61` — `vector<uint32_t>` indices |
| GPU vertex format | `VulkanTriangle.cppm:447–458` |
| Triangle-list topology | around `459–462`, plus pipeline assembly |
| Vertex/index binding | around `712–718` |
| Indexed draw | `vkCmdDrawIndexed` around `718` |
| Model transform | `triangle.vert:40` |
| View/project transform | `triangle.vert:41` |
| Clip output | `gl_Position` at `triangle.vert:41` |
| Interpolated fragment inputs | `triangle.frag:3–7` |
| Screen derivatives | `triangle.frag:122–125` |
| Tangent reconstruction | `triangle.frag:126–136` |
| Viewport | `VulkanTriangle.cppm:269–277` |
| Depth format | `VulkanDepth.cppm` → `VK_FORMAT_D32_SFLOAT` |
| Depth clear | `VulkanTriangle.cppm` → `1.0f` |
| Depth comparison | around `541–545` → `LESS` |
| Camera projection | `Camera.cppm:115–140` |
| Vulkan Y correction | `Camera.cppm:136–139` |

So your current real path is approximately:

```text
Mesh.cppm
CPU MeshVertex[] + uint32 indices
        ↓
VulkanHostBuffer
        ↓
VkVertexInputAttributeDescription
        ↓
vkCmdBindVertexBuffers
vkCmdBindIndexBuffer
        ↓
vkCmdDrawIndexed
        ↓
triangle.vert
object position
        ↓ model
world
        ↓ view
view
        ↓ projection
clip / gl_Position
        ↓
fixed-function clipping
        ↓
perspective divide
        ↓
NDC
        ↓
viewport
        ↓
triangle setup / coverage
        ↓
perspective interpolation
        ↓
triangle.frag
        ↓
D32 depth test
        ↓
color/object-ID attachments
```

That is the real theory-to-repository mapping.

---

**Failure modes you should now be able to diagnose**

This is the one list I want you to retain from the lesson because each symptom maps to a specific raster stage.

1. **Exploding triangle across the screen:** likely bad projection, invalid $w$, clipping failure, NaNs or wrong coordinate convention.
2. **Mesh shape correct but textures warp with depth:** affine interpolation used where perspective-correct interpolation was required.
3. **Hairline cracks between adjacent triangles:** inconsistent edge ownership/top-left implementation, floating-point coverage disagreement or mismatched vertices.
4. **Half-pixel shift:** sample-center/viewport convention error.
5. **Everything vertically inverted:** Vulkan viewport/projection Y convention mismatch; in Kairo specifically, remember `projection(1,1) *= -1`.
6. **Geometry disappears after enabling depth:** wrong clear value, wrong compare operator, projection-depth convention mismatch or invalid depth range.
7. **Distant coplanar objects flicker:** depth precision/Z-fighting, often worsened by an excessively tiny near plane.
8. **Normal map suddenly produces garbage on one triangle:** UV Jacobian determinant near zero, derivative/tangent reconstruction degeneracy.
9. **Some vertices appear to contain garbage:** vertex stride/offset/format disagreement.
10. **Large indexed mesh has surprising VS cost:** poor index locality/post-transform-cache reuse.
11. **One triangle is missing around the near plane:** clipping or projection convention error rather than necessarily backface culling.
12. **Reversed-Z attempt produces a blank frame:** projection, clear value and comparison direction were not changed as one invariant.

---

**The deepest conceptual connection of this pass**

There is one chain worth reconstructing without notes:

$$
\boxed{
\text{3D triangle}
}
$$

becomes

$$
\boxed{
PVMp
}
$$

which gives homogeneous

$$
(x_c,y_c,z_c,w_c),
$$

which is clipped using inequalities containing $w_c$,

then divided:

$$
(x_c/w_c,\;y_c/w_c,\;z_c/w_c),
$$

then mapped into framebuffer coordinates,

then converted into three linear edge equations,

those edge equations simultaneously determine

$$
\text{coverage}
$$

and

$$
\text{barycentric coordinates},
$$

those barycentrics interpolate

$$
a/w
$$

and

$$
1/w
$$

to reconstruct perspective-correct attributes,

the resulting fragment can compute spatial derivatives from neighboring invocations,

and its projected $z$ competes against persistent depth-buffer state.

That is rasterization as one mathematical machine.

Not a collection of API calls.

---

**Research bridge**

A strong falsifiable experiment for this phase would be a **CPU/GPU raster agreement harness**.

Build a small reference rasterizer with configurable:

```text
floating vs fixed-point edges
top-left convention
sample locations
affine vs perspective interpolation
conventional vs reversed depth
```

Then render carefully designed adversarial triangles:

```text
near-plane crossings
subpixel slivers
shared edges
very large coordinates
almost-degenerate triangles
huge depth ratios
UV degeneracy
```

Against Kairo Vulkan output.

Measure:

$$
\text{coverage disagreement rate},
$$

$$
\text{depth disagreement},
$$

$$
\text{interpolant error},
$$

and boundary disagreement spatially.

The interesting research/debug question is not:

> “Does my CPU rasterizer look similar?”

It is:

> “Which API/hardware rasterization conventions account for every remaining disagreement after mathematical equivalence is enforced?”

That gives you an actual validation instrument for later Metal↔Vulkan comparisons rather than screenshot eyeballing.

---

**PASS 1 RECALL SHEET**

| Cue | What you must reconstruct |
|---|---|
| **Problem** | Continuous triangles must become deterministic discrete sample coverage and interpolated fragment state. |
| **Input** | Vertex bytes + vertex format/layout + index bytes + topology. |
| **Core chain** | object → world → view → clip → clipping → divide → NDC → viewport → triangle setup → coverage → interpolation → fragment → depth |
| **Clip test** | Vulkan default: $-w\le x\le w,\ -w\le y\le w,\ 0\le z\le w$. |
| **Clip intersection** | $t=d_A/(d_A-d_B)$. |
| **Perspective divide** | $x_d=x_c/w_c,\ y_d=y_c/w_c,\ z_d=z_c/w_c$. |
| **Edge function** | $E_{ab}(p)=(b-a)\times(p-a)$. |
| **Area** | $A_2=E_{01}(v_2)$. Zero means degenerate. |
| **Barycentrics** | Opposite-edge areas divided by total area. |
| **Perspective interpolation** | $a=\frac{\sum\lambda_i a_i/w_i}{\sum\lambda_i/w_i}$. |
| **Coverage invariant** | Shared boundary must belong to exactly one primitive → top-left rule. |
| **Derivative model** | Neighboring fragment invocations approximate screen derivatives. |
| **Conventional depth** | near→0, far→1, clear 1, compare LESS. |
| **Reversed-Z** | near→1, far→0, clear 0, compare GREATER. |
| **Infinite reversed depth** | $z=n/d$. |
| **Kairo today** | D32 float, clear 1, LESS → conventional Z. |
| **Kairo derivative use** | `dFdx/dFdy` world position + UV → reconstruct tangent basis. |
| **Precision insight** | Projection already produces $1/d$-like depth; reversed-Z exploits float precision near zero. |

---

**Blank-screen reconstruction challenge**

Without looking upward, rebuild this entire diagram:

```text
MeshVertex bytes
       +
index bytes
       +
topology
       ↓
vertex fetch
       ↓
vertex shader
       ↓
M
       ↓
world
       ↓
V
       ↓
view
       ↓
P
       ↓
(xc,yc,zc,wc)
       ↓
homogeneous clipping
       ↓
divide by wc
       ↓
NDC
       ↓
viewport
       ↓
screen triangle
       ↓
3 edge equations
       ↓
coverage + barycentrics
       ↓
(a/w, 1/w) interpolation
       ↓
fragment inputs
       ↓
dFdx/dFdy / texture LOD etc.
       ↓
depth comparison
       ↓
surviving fragment
```

Then derive, from memory:

$$
t=\frac{d_A}{d_A-d_B},
$$

$$
E_{ab}(p),
$$

$$
\lambda_0,\lambda_1,\lambda_2,
$$

$$
a=
\frac{\sum\lambda_i a_i/w_i}
{\sum\lambda_i/w_i},
$$

$$
z_{\text{conv}}(d)
=
\frac{f}{f-n}
-
\frac{fn}{(f-n)d},
$$

and

$$
z_{\text{reverse},\infty}(d)
=
\frac nd.
$$

If you can derive those and explain where each quantity physically exists in Kairo/Vulkan, then M109/M110 are becoming reconstructable rather than merely recognizable.

**Repository status:** verified against the current connected `swayam8624/KairoRenderer` main branch during this pass.
**Mastery gap deliberately deferred:** complete CPU rasterizer, exact MSAA/resolve behavior, stencil/culling/render-target semantics and pipeline-level black-frame reconstruction belong to M111/M114 in **Pass 2**, not because they are being pruned but because they are outside Parts 1801–1845.

**Depth gate: passed for Pass 1.**

**Next: Pass 02 — M111–M114, Parts 1846–1900: stencil, culling/winding, MSAA, blending/transparency, Hi-Z/early-Z, GPU-driven rendering foundations, then the full CPU rasterizer ↔ Vulkan pipeline correspondence.**


### + M113 + M114


The mastery cards deliberately group those 55 parts into four systems: M111 owns framebuffer/raster-output mechanics, M112 owns blending/transparency/visibility efficiency, M113 owns submission and programmable GPU execution/state, and M114 forces us to reconstruct the entire pipeline in software and debug it from first principles.

We finished Pass 1 at:

$$
\text{triangle}
\rightarrow
\text{coverage}
\rightarrow
\text{interpolants}
\rightarrow
\text{fragment candidate}
\rightarrow
\text{depth}.
$$

Now the question becomes much larger:

$$
\boxed{
\text{What happens to that fragment candidate before it becomes persistent framebuffer state?}
}
$$

And then:

$$
\boxed{
\text{How do we submit enough geometry to feed that machinery efficiently?}
}
$$

---

### Stencil, culling, scissor/viewports, attachments, load/store, MSAA

---

**Depth alone is not enough: the stencil buffer**

A depth buffer stores a geometric ordering quantity.

A stencil buffer stores something fundamentally different:

$$
\boxed{\text{small integer state associated with framebuffer samples}}
$$

Usually the stencil value is 8 bits:

$$
s\in[0,255].
$$

Stencil is not intrinsically:

- an object ID,
- a mask,
- a shadow flag,
- an outline marker,
- a portal number.

It is simply integer state that the raster pipeline can compare and modify according to rules.

You assign the semantics.

Conceptually, each sample may contain:

```text
color     = RGBA
depth     = z
stencil   = uint8
```

For packed depth-stencil formats, depth and stencil may physically share one image format, such as `D24_UNORM_S8_UINT` or `D32_SFLOAT_S8_UINT`.

Your current Kairo viewport does **not** have stencil.

It uses:

```cpp
VK_FORMAT_D32_SFLOAT
```

for its depth attachment.

That contains depth only.

So M111 stencil theory is currently **canonical theory rather than implemented Kairo behavior**.

---

**The stencil test**

For an incoming fragment, define:

$$
s_a = \text{stored stencil value},
$$

$$
s_r = \text{stencil reference}.
$$

The pipeline can compare them using operations analogous to depth:

$$
s_r \stackrel{?}{<,>,=,\neq,\dots} s_a.
$$

But stencil also has masks.

A more accurate conceptual comparison is

$$
(s_r \land M_c)
\quad\text{compare}\quad
(s_a \land M_c),
$$

where $M_c$ is the compare mask.

The Vulkan stencil test is explicitly per sample, and Vulkan maintains separate front-face and back-face stencil state. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/fragops.html?utm_source=chatgpt.com)

That last fact matters enormously.

Stencil configuration is not necessarily:

```text
one operation for the whole triangle
```

but potentially:

```text
front-facing polygons → one policy
back-facing polygons  → another policy
```

which enables techniques such as stencil shadow volumes.

---

**Stencil has three different update paths**

Suppose a fragment arrives.

Three major events can occur:

```text
stencil test fails
depth test fails
depth test passes
```

Stencil lets you specify an operation for each.

Conceptually:

```cpp
if (!StencilTest())
{
    stencil = StencilFailOp(stencil);
}
else if (!DepthTest())
{
    stencil = DepthFailOp(stencil);
}
else
{
    stencil = PassOp(stencil);
}
```

Typical operations include:

```text
KEEP
ZERO
REPLACE
INCREMENT
DECREMENT
INVERT
INCREMENT_AND_WRAP
DECREMENT_AND_WRAP
```

This gives stencil its power.

It is not merely a boolean mask.

It is a tiny state machine attached to every sample.

---

**Example: drawing an outline using stencil**

Imagine drawing an object with an outline.

First pass:

```text
draw object normally
depth test enabled
stencil reference = 1
on successful fragment:
    stencil = 1
```

The object's visible pixels now have:

$$
s=1.
$$

Second pass:

Render a slightly enlarged version of the object, but only where

$$
s\neq1.
$$

That means the enlarged geometry appears only outside the original object's stencil footprint.

Result:

```text
enlarged colored silhouette
minus
original object area
=
outline
```

The important point is not memorizing the trick.

The important point is understanding the stencil machine:

$$
\text{geometry generates samples}
\rightarrow
\text{stencil classifies existing sample state}
\rightarrow
\text{pipeline conditionally permits/modifies fragments}.
$$

---

**Stencil write masks**

Suppose the stored stencil byte is

```text
10101100
```

and you only want to modify its lower four bits.

Use a write mask:

```text
00001111
```

Conceptually:

$$
s_{\text{new}}
=
(s_{\text{old}}\land\neg M_w)
\;|\;
(s_{\text{op}}\land M_w).
$$

That lets different subsystems share stencil bits.

For example:

```text
bits 0–2 : portal region
bit 3    : selection mask
bits 4–7 : lighting classification
```

Whether sharing stencil like this is wise depends heavily on engine complexity, but mathematically the facility exists.

---

**Backface culling**

A closed opaque surface normally has one side facing outward.

For many meshes, triangles facing away from the camera cannot contribute visible pixels.

Rasterizing them wastes:

- triangle setup,
- coverage testing,
- potential fragment work,
- bandwidth.

So we classify each triangle by projected orientation.

From Pass 1, projected signed area is:

$$
A_2 =
(x_1-x_0)(y_2-y_0)
-
(y_1-y_0)(x_2-x_0).
$$

Its sign determines winding under a particular coordinate convention.

For example:

$$
A_2>0
$$

may mean counter-clockwise.

Then the pipeline can declare:

```text
front face = CCW
cull mode  = back
```

and discard triangles classified as back-facing before fragment rasterization.

---

**Winding is a convention, not physical truth**

The triangle

$$
(v_0,v_1,v_2)
$$

and triangle

$$
(v_0,v_2,v_1)
$$

contain the same geometric points.

But their orientation is reversed.

So the renderer needs a convention:

```text
counter-clockwise projected ordering = front-facing
```

or

```text
clockwise projected ordering = front-facing.
```

Vulkan exposes this with `frontFace`.

Your Kairo pipeline currently sets:

```cpp
rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
```

but also:

```cpp
rasterizer.cullMode = VK_CULL_MODE_NONE;
```

Therefore the declared front-face convention currently does **not eliminate either side** in your main mesh path.

Both sides rasterize.

That is effectively two-sided rasterization.

---

**Why negative-scale transforms can reverse winding**

Suppose a model matrix has scale

$$
S=
\begin{bmatrix}
-1&0&0\\
0&1&0\\
0&0&1
\end{bmatrix}.
$$

Its determinant is

$$
\det S=-1.
$$

That means it reverses orientation.

A triangle that was CCW in object space can become CW after transformation.

This is why mirrored transforms can unexpectedly interact with face culling.

The deeper invariant is:

$$
\boxed{
\det(M_{3\times3})<0
\Rightarrow
\text{orientation reversal}
}
$$

although projection and viewport conventions also participate in final screen winding.

So a broken culling bug cannot always be solved by flipping `frontFace`. You must understand the entire transform chain.

---

**Two-sided rendering**

Some surfaces genuinely require both sides:

```text
leaves
cloth
paper
thin sheets
some particles
debug geometry
```

Then backface culling can be disabled.

But two-sided rendering introduces another question:

> What should the surface normal mean on the back side?

Suppose the front normal is

$$
N.
$$

For a back-facing fragment, you may want

$$
N'=-N.
$$

Shader languages provide mechanisms such as `gl_FrontFacing` that let the fragment shader distinguish orientation.

A typical two-sided shading policy is:

```glsl
vec3 N = normalize(inNormal);

if (!gl_FrontFacing)
    N = -N;
```

This is not universally correct—for physically thin materials you may want a specialized BSDF—but it demonstrates that “disable culling” is only the first half of two-sided rendering.

---

**Scissor testing**

A viewport and scissor are commonly confused.

The **viewport** transforms normalized device coordinates into framebuffer coordinates.

The **scissor** rejects fragments outside a rectangular framebuffer region.

So:

$$
\boxed{\text{viewport = coordinate transformation}}
$$

while

$$
\boxed{\text{scissor = rectangular coverage restriction}}.
$$

If your viewport is

```text
0,0 → 1920×1080
```

but your scissor is

```text
400,200 → 800×600,
```

geometry still transforms relative to the viewport but fragments outside the 800×600 rectangle are discarded.

Scissors are extremely useful for:

```text
UI clipping
split-screen regions
editor panes
partial clears
shadow atlas regions
damage regions
```

without writing custom shader clipping code.

---

**Kairo viewport and scissor**

Your current `VulkanTriangle::Record` dynamically configures:

```cpp
viewport.width    = viewportExtent.width;
viewport.height   = viewportExtent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;

scissor.extent = viewportExtent;

vkCmdSetViewport(...);
vkCmdSetScissor(...);
```

So today:

$$
\text{viewport rectangle}
=
\text{scissor rectangle}.
$$

That means the scissor is not currently performing an interesting crop.

But because viewport/scissor are dynamic pipeline state, you can change them without rebuilding the graphics pipeline.

---

**Attachments: persistent destinations of rasterization**

A render pass may write several outputs.

A fragment shader might emit:

```glsl
layout(location = 0) out vec4 color;
layout(location = 1) out uint objectID;
```

These map to color attachments.

Meanwhile fixed-function depth testing updates a depth attachment.

So conceptually a framebuffer might be:

```text
Attachment 0:
RGBA16F HDR color

Attachment 1:
R32_UINT object ID

Attachment 2:
D32_FLOAT depth
```

That is almost exactly your current editor viewport.

Kairo's `VulkanViewportTarget` currently creates:

$$
\text{color} = \texttt{R16G16B16A16_SFLOAT},
$$

$$
\text{objectID} = \texttt{R32_UINT},
$$

$$
\text{depth} = \texttt{D32_SFLOAT}.
$$

This is a very good example of why “the framebuffer” is not synonymous with “the final color image.”

A framebuffer is a set of images participating together in rendering.

---

**Why the object-ID attachment is integer**

Suppose object ID 16,777,217 were encoded into an RGB color.

You would now depend on:

```text
color encoding
rounding
format conversion
possibly gamma transfer
possibly blending
possibly filtering
```

to preserve identity.

That is terrible.

Kairo instead uses:

```cpp
VK_FORMAT_R32_UINT
```

and writes an actual integer.

That gives semantic integrity:

$$
\boxed{\text{identity remains integer data}}
$$

rather than masquerading as color.

This is an important production principle.

---

**Load operations**

When a render pass begins, what should an attachment's existing contents mean?

Vulkan gives operations conceptually equivalent to:

```text
LOAD
CLEAR
DONT_CARE
```

`LOAD` means:

> Preserve the previous contents because this pass needs them.

`CLEAR` means:

> Initialize the attachment to a known value.

`DONT_CARE` means:

> Previous contents do not matter.

This is not only semantic.

On tile-based architectures especially, avoiding unnecessary attachment preservation can save memory traffic.

---

**Store operations**

At the end of the pass, you similarly decide:

```text
STORE
DONT_CARE
```

`STORE` means later work needs the generated contents.

`DONT_CARE` means the attachment can be discarded after its current purpose has been fulfilled.

This is one of the simplest places where knowing future resource lifetime directly turns into GPU bandwidth optimization.

---

**Kairo's load/store decisions tell us its dataflow**

Current viewport attachments:

```text
HDR color:
load  = CLEAR
store = STORE

Object ID:
load  = CLEAR
store = STORE

Depth:
load  = CLEAR
store = DONT_CARE
```

Why?

Color must survive because the editor samples/presents it later.

Object ID must survive because picking can copy a pixel to a readback buffer later.

Depth only needs to participate during rasterization in that pass.

The viewport does not currently sample it later.

Therefore:

$$
\boxed{
\text{depth persistence after this pass has no value}
}
$$

and `DONT_CARE` is appropriate.

That is exactly the sort of resource-lifetime reasoning a render graph will automate later.

---

**MSAA changes the framebuffer from one sample to many**

At 1×:

```text
one pixel
└── one coverage/depth/color sample
```

At 4× MSAA:

```text
one pixel
├── sample 0
├── sample 1
├── sample 2
└── sample 3
```

Each sample has an exact sample location.

Rasterization tests primitive coverage against those sample positions, producing a coverage mask. Vulkan describes multisampling precisely this way: each framebuffer pixel has `rasterizationSamples` exact sample points, each attachment holds per-sample color/depth/stencil state, and the resulting multisampled color can later be resolved to a single-sample image. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/primsrast.html?utm_source=chatgpt.com)

So MSAA is not:

> “Render the image bigger and shrink it.”

That is supersampling.

---

**A 4× MSAA edge example**

Suppose a pixel contains four samples:

```text
•      •

   / triangle boundary

•      •
```

and the triangle covers three.

Coverage mask:

```text
1110
```

Three depth samples may now pass.

If fragment shading is performed once at pixel frequency, the result can conceptually be replicated to covered color samples.

On resolve, suppose three samples contain red:

$$
(1,0,0)
$$

and one contains background black:

$$
(0,0,0).
$$

A simple box resolve gives

$$
C=
\frac{3(1,0,0)+(0,0,0)}4
=
(0.75,0,0).
$$

The edge therefore appears partially covered rather than as a binary staircase.

---

**MSAA resolve**

A multisampled attachment cannot in general be treated exactly like a normal single-sample image.

For presentation you typically need:

$$
N\text{-sample image}
\rightarrow
1\text{-sample image}.
$$

For color, a typical resolve is effectively an averaging process.

Depth/stencil resolves are more nuanced because averaging depth has no universal visibility interpretation.

APIs therefore provide separate depth/stencil resolve semantics where supported.

Your current Kairo viewport is explicitly:

```cpp
image.samples = VK_SAMPLE_COUNT_1_BIT;
```

for all three attachments.

So Kairo currently has:

$$
\boxed{\text{no MSAA in this viewport path}}
$$

and therefore no resolve attachment or resolve pass here.

---

**Supersampling versus multisampling**

Suppose final target resolution is

$$
1920\times1080.
$$

For 4× supersampling, you could render approximately

$$
3840\times2160
$$

and downsample.

Everything runs at the higher resolution:

```text
coverage
depth
fragment shader
texture sampling
post-process intermediates
```

This produces excellent quality but is expensive.

4× MSAA instead stores multiple coverage/depth/color samples per pixel while often avoiding 4× full fragment shading.

Thus:

$$
\boxed{
\text{SSAA attacks general high-frequency shading + geometry aliasing}
}
$$

whereas

$$
\boxed{
\text{MSAA primarily attacks polygon-edge coverage aliasing}
}
$$

and is much cheaper when fragment shading is expensive.

It does not magically fix shader aliasing inside a triangle.

A high-frequency normal map can still shimmer under MSAA.

That is the domain of mipmapping, anisotropic filtering, TAA, supersampling and other sampling strategies later.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/12 - Probability, Statistics and Uncertainty|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/14 - Transparency, Blending and Visibility|Next chapter]]
