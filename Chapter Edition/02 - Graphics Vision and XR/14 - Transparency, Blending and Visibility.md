# 14 — Transparency, Blending and Visibility

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|Next chapter]]

Transparency changes the order in which fragments contribute to the framebuffer. Alpha conventions, overdraw, depth tests and hierarchy queries determine both correctness and cost.

**By the end:** Explain source-over blending and the visibility tradeoffs around transparency.

**Recall question:** Why can ordinary depth writes produce incorrect results for translucent surfaces?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 31770–35299; source-span SHA256 cca08a61b824c23d34e7efe82368468cffe8228d30cca6795b826c341661cb77. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Blending, transparency, alpha testing, overdraw, early-Z, queries, Hi-Z

---

**Blending is a framebuffer recurrence**

Suppose a new fragment produces source color

$$
C_s
$$

and the framebuffer currently stores destination color

$$
C_d.
$$

A general blend operation can be written

$$
\boxed{
C_{\text{out}}
=
\operatorname{Op}
\left(
F_s C_s,
F_d C_d
\right)
}
$$

where $F_s$ and $F_d$ are configurable factors.

For ADD:

$$
C_{\text{out}}
=
F_sC_s+F_dC_d.
$$

Vulkan exposes exactly this model: source/destination RGB and alpha factors plus separate blend operations per color attachment. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/framebuffer.html?utm_source=chatgpt.com)

This means blending is not fundamentally a “transparency feature.”

It is a programmable recurrence on framebuffer state.

Transparency is one important use.

---

**Standard straight-alpha source-over**

Let

$$
\alpha_s
$$

be source opacity.

For straight/unassociated RGB, the standard color equation is

$$
\boxed{
C_{\text{out}}
=
\alpha_s C_s
+
(1-\alpha_s)C_d
}
$$

The corresponding output alpha is

$$
\boxed{
\alpha_{\text{out}}
=
\alpha_s+
(1-\alpha_s)\alpha_d
}
$$

which expands to

$$
\alpha_s+\alpha_d-\alpha_s\alpha_d.
$$

This is the Porter-Duff source-over operator.

The Vulkan configuration is:

```cpp
srcColor = SRC_ALPHA;
dstColor = ONE_MINUS_SRC_ALPHA;
colorOp  = ADD;

srcAlpha = ONE;
dstAlpha = ONE_MINUS_SRC_ALPHA;
alphaOp  = ADD;
```

And that is precisely what the current Kairo transparent pipeline uses.

So your current renderer is a concrete straight-alpha source-over implementation.

---

**A numerical blend trace**

Suppose the framebuffer currently stores blue:

$$
C_d=(0,0,1)
$$

and a 25% opaque red fragment arrives:

$$
C_s=(1,0,0),\qquad \alpha_s=0.25.
$$

Then

$$
C_{\text{out}}
=
0.25(1,0,0)
+
0.75(0,0,1)
$$

so

$$
\boxed{
C_{\text{out}}
=
(0.25,0,0.75)
}
$$

The important conceptual point:

The source RGB $(1,0,0)$ does **not** mean the final contribution is full red.

Alpha scales its contribution during blending.

---

**Premultiplied alpha**

Instead of storing

$$
(C,\alpha),
$$

store

$$
(\alpha C,\alpha).
$$

Define

$$
C'_s=\alpha_s C_s.
$$

Then source-over becomes

$$
\boxed{
C'_{\text{out}}
=
C'_s+(1-\alpha_s)C'_d
}
$$

which uses

```text
source factor      = ONE
destination factor = ONE_MINUS_SRC_ALPHA
```

rather than multiplying source RGB by alpha during the blend.

---

**Why premultiplied alpha is more than an optimization**

Consider a transparent texel.

Straight-alpha representation could contain

$$
C=(1,0,0),\qquad \alpha=0.
$$

That texel is “fully transparent red.”

Another could contain

$$
C=(0,1,0),\qquad \alpha=0.
$$

They are visually identical in isolation.

But interpolate those RGB values during texture filtering and hidden color can leak into partially transparent edge texels.

In premultiplied representation:

$$
\alpha=0
\Rightarrow
\alpha C=(0,0,0).
$$

Transparent texels have a canonical zero contribution.

That makes interpolation and filtering behave much more naturally.

This is one of the deepest reasons production compositing systems often prefer premultiplied alpha.

However, every layer must agree.

If the texture/shader already outputs premultiplied RGB but the blend state still uses `SRC_ALPHA`, you multiply by alpha twice:

$$
C_{\text{contribution}}
=
\alpha(\alpha C)
=
\alpha^2C.
$$

The result becomes too dark.

Your current Kairo pipeline is **straight alpha**, so do not start premultiplying shader output unless you change the blending state too.

---

**Additive blending**

Set

$$
F_s=1,\qquad F_d=1.
$$

Then

$$
\boxed{
C_{\text{out}}=C_s+C_d
}
$$

This is useful for phenomena where contributions accumulate:

```text
glows
energy effects
light-like particles
some bloom accumulation
```

On HDR floating-point targets this can exceed 1:

$$
(2.1,0.7,4.4)
$$

which is perfectly reasonable before tone mapping.

Additive blending is mathematically commutative:

$$
A+B=B+A,
$$

so pure additive transparent particles are much less sensitive to ordering than source-over transparency.

---

**Multiplicative blending**

Suppose:

$$
C_{\text{out}}=C_sC_d
$$

component-wise.

Then a source value of 1 leaves the destination unchanged:

$$
1\cdot C_d=C_d,
$$

while zero blacks it out.

This has historically been useful for modulation, darkening and certain decal/effect models.

One blend-state representation can be:

```text
source factor      = DST_COLOR
destination factor = ZERO
```

giving

$$
C_sC_d.
$$

Again, “blending” is much broader than transparency.

---

**Why ordinary transparency is order dependent**

Take black background and two 50%-opaque layers:

Red:

$$
R=(1,0,0),\quad \alpha=0.5.
$$

Blue:

$$
B=(0,0,1),\quad \alpha=0.5.
$$

Red over background:

$$
C_1=(0.5,0,0).
$$

Then blue over that:

$$
C_2
=
0.5(0,0,1)+0.5(0.5,0,0)
$$

$$
=
(0.25,0,0.5).
$$

Reverse the order.

Blue first:

$$
C'_1=(0,0,0.5).
$$

Then red:

$$
C'_2
=
0.5(1,0,0)+0.5(0,0,0.5)
$$

$$
=
(0.5,0,0.25).
$$

Therefore

$$
\boxed{
R\operatorname{over}B
\neq
B\operatorname{over}R
}
$$

and ordinary alpha transparency requires approximate back-to-front ordering.

---

**Why transparent geometry usually does not write depth**

Imagine:

```text
front transparent pane
rear transparent pane
```

If the front pane writes depth, the rear pane can fail the depth test entirely.

But physically we need both contributions.

So a common configuration is:

```text
depth test  = ON
depth write = OFF
```

This allows transparent geometry to respect opaque occluders while not preventing later transparent layers from contributing.

Your Kairo transparent pipeline does exactly this:

```cpp
m_TransparentPipeline =
    CreatePipeline(...,
                   depthWrite = VK_FALSE,
                   alphaBlend = true);
```

The opaque path uses depth writes.

---

**Kairo's current transparency sorting**

Current Kairo collects blended draws and stable-sorts them using approximate camera distance:

$$
d^2=\|T_{\text{model}}-P_{\text{camera}}\|^2.
$$

Larger distance is drawn first.

That is a reasonable object-level approximation.

But imagine two long transparent objects that intersect.

Object A's origin may be farther from the camera even though half of A is in front of B and half is behind it.

There exists no single object ordering that solves the geometry.

That is why order-independent transparency is a research/advanced-rendering topic, rather than something sorting can completely solve.

---

**Alpha testing / cutouts**

Some materials are not really translucent.

A leaf texture might represent:

```text
leaf     → fully opaque
outside  → fully empty
```

Instead of blending partial opacity, use a threshold:

$$
\text{if }\alpha < \alpha_c
\Rightarrow
\text{discard}.
$$

For example:

$$
\alpha_c=0.5.
$$

Then pixels become binary:

$$
\text{opaque or absent}.
$$

Advantages include:

```text
depth writes remain useful
sorting becomes much easier
hidden geometry can be rejected
```

The cost is aliasing around thin edges.

Your current `triangle.frag` implements exactly this for its mask mode:

```glsl
if (maskMode && baseColorAlpha.a < cutoff)
    discard;
```

So Kairo already has all three broad alpha semantics:

```text
opaque
masked/cutout
blended
```

---

**Alpha-to-coverage**

Suppose 4× MSAA gives four coverage bits.

Instead of converting alpha into framebuffer blending, alpha-to-coverage converts alpha approximately into a sample mask.

For example:

$$
\alpha\approx0.5
$$

might produce coverage of roughly two of four samples.

Conceptually:

```text
alpha 0.0 → 0000
alpha 0.25 → perhaps 1000
alpha 0.5 → perhaps 1010
alpha 0.75 → perhaps 1110
alpha 1.0 → 1111
```

Actual patterns are implementation-dependent and may use dithering.

The key benefit for foliage is that covered samples can still participate normally in depth.

So alpha-to-coverage can provide antialiased cutout edges without conventional translucent depth-write problems.

But it needs multisampling.

Your present 1× Kairo viewport cannot benefit meaningfully from alpha-to-coverage.

---

**Overdraw**

Suppose one final pixel is shaded by six opaque triangles before the nearest visible result survives.

The display shows one color.

The GPU may have performed six fragment shader executions.

Define an intuitive overdraw factor:

$$
O=
\frac{\text{fragment/sample work generated}}
{\text{visible output work}}.
$$

Large $O$ means enormous wasted work.

Common causes include:

```text
dense foliage
particles
large overlapping UI
poor draw ordering
very high geometric density
full-screen effects
```

Overdraw is particularly painful when fragment shaders are expensive.

---

**Fill rate and bandwidth are not the same thing**

“Fill rate” informally describes the ability to process/write large numbers of fragments or samples.

But the bottleneck might actually be:

$$
\text{shader ALU},
$$

$$
\text{texture bandwidth},
$$

$$
\text{attachment bandwidth},
$$

$$
\text{ROP/blending throughput},
$$

or some combination.

Consider Kairo's viewport.

Approximate attachment sizes:

$$
RGBA16F = 8\text{ bytes},
$$

$$
R32UI = 4\text{ bytes},
$$

$$
D32F = 4\text{ bytes}.
$$

That is roughly

$$
16\text{ bytes/sample}
$$

of attachment state before considering reads, compression, caches or tiling.

At 3840×2160:

$$
8\,294\,400\text{ pixels}.
$$

At 60 fps:

$$
8.2944\times10^6
\times16
\times60
\approx7.96\times10^9
$$

or almost

$$
\boxed{8\text{ GB/s}}
$$

of raw per-frame attachment traffic for a single write of each value.

With 4× samples, naive raw traffic would approach:

$$
\approx32\text{ GB/s}.
$$

Real architectures use compression and tiles to avoid much of this, but the estimate explains why unnecessary overdraw and multisampling can be expensive.

---

**Early-Z and front-to-back opaque ordering**

Suppose a nearby wall covers almost the entire screen.

If you draw 100 objects behind the wall first, their fragments may shade and later become hidden.

If you draw the wall first, subsequent fragments can often fail depth early.

Thus for opaque geometry:

$$
\boxed{
\text{front-to-back sorting}
\rightarrow
\text{better opportunity for early depth rejection}
}
$$

This competes with state sorting.

If switching material pipelines is expensive, you may choose:

```text
perfect depth sorting but many pipeline switches
```

or

```text
material batching but more overdraw.
```

Production renderers solve a multi-objective optimization rather than using one universal sort.

---

**Early-Z is not an absolute promise**

We discussed early versus late depth in Pass 1.

The fragment pipeline may contain operations that constrain early testing, such as explicit depth writes or certain fragment-side effects.

Vulkan's fragment pipeline defines depth/stencil testing as part of per-fragment operations and exposes explicit early/late fragment-test execution stages. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/fragops.html?utm_source=chatgpt.com)

Modern implementations also use hierarchical rejection before individual sample tests.

So “early-Z” should be understood as a family of implementations that avoid fragment work when visibility can be proven, not one mandatory little hardware block.

---

**Occlusion queries**

An occlusion query asks approximately:

> How many samples survived visibility testing for this draw?

A common use is:

```text
draw cheap bounding proxy
→ query visible sample count
→ skip expensive object next frame if zero
```

The danger is synchronization.

If the CPU does:

```text
issue query
wait immediately
decide what to draw
```

you can force CPU/GPU serialization.

The GPU may have to finish enormous amounts of queued work before the CPU receives the result.

Production use therefore often accepts latency:

```text
frame N query
frame N+1 or N+2 result
```

and uses conservative logic so temporary stale visibility does not cause popping.

---

**Hierarchical Z**

Testing every incoming sample against full-resolution depth is already useful, but many pixels can often be rejected in blocks.

Imagine an 8×8 block.

If every stored depth within that block is closer than an incoming primitive, the entire block can be discarded.

A Hi-Z structure stores coarse conservative summaries.

For conventional Z:

```text
near = 0
far  = 1
depth test = LESS
```

one useful conservative level stores the maximum depth of a region:

$$
H=\max(z_1,\dots,z_n).
$$

Suppose an incoming object's nearest possible depth over the block is

$$
z_{\text{near,obj}}.
$$

If

$$
z_{\text{near,obj}}\ge H,
$$

then since every existing sample satisfies

$$
z_i\le H\le z_{\text{near,obj}},
$$

the object is behind every stored depth value.

Reject the whole block.

---

**Reversed-Z reverses the Hi-Z reduction direction**

Under reversed-Z:

```text
near = 1
far  = 0
depth test = GREATER
```

the conservative bound flips.

A useful hierarchy stores

$$
H_R=\min(z_1,\dots,z_n).
$$

If the candidate object's nearest/highest depth satisfies

$$
z_{\text{near,obj}}\le H_R,
$$

then every existing sample is at least as near as the candidate, so it cannot pass `GREATER`.

This illustrates an important lesson from Pass 1:

> Reversed-Z is not merely a projection matrix change.

Any subsystem interpreting depth ordering must be audited.

Hi-Z is one of them.

---

### Instancing, batching, indirect drawing, GPU culling, mesh shaders, execution cost, data binding, PSOs and permutations

---

**Why draw-call overhead exists**

Suppose you have 50,000 identical rocks.

A naive renderer performs:

```cpp
for (const Rock& rock : rocks)
{
    BindPipeline();
    BindMesh();
    BindMaterial();
    SetTransform(rock.transform);
    DrawIndexed();
}
```

Even if each rock contains little geometry, the CPU must repeatedly:

```text
select state
encode commands
validate objects
write/update descriptors or constants
call API
manage command-buffer state
```

A draw call is not fundamentally expensive because “function calls are slow.”

It is expensive because it represents **submission and state-management work**.

If GPU geometry work becomes tiny but CPU submission remains one command per object, the renderer becomes CPU-bound.

---

**Instanced rendering**

If many objects use the same mesh and pipeline, issue one draw:

$$
\boxed{
\text{one mesh}
+
N\text{ instance records}
}
$$

instead of $N$ independent draws.

A conceptual instance record:

```cpp
struct InstanceData
{
    Mat4 model;
    Vec4 color;
};
```

Then the vertex shader receives an instance index:

$$
i=\text{InstanceIndex}.
$$

It fetches

$$
M_i
$$

and computes

$$
p_w=M_i p_o.
$$

One indexed draw can now request:

```text
indexCount   = mesh indices
instanceCount = 50,000
```

and the vertex workload becomes logically

$$
\text{each mesh vertex}
\times
\text{each instance}.
$$

---

**Instance buffers**

One way to supply per-instance data is another vertex-input stream whose rate is:

```cpp
VK_VERTEX_INPUT_RATE_INSTANCE
```

Rather than incrementing once per vertex, the record advances once per instance.

Conceptually:

```text
binding 0:
Vertex 0
Vertex 1
Vertex 2
...

binding 1:
Instance 0 transform
Instance 1 transform
Instance 2 transform
...
```

The vertex invocation sees:

```text
mesh attributes from binding 0
+
instance attributes from binding 1
```

Large modern engines often instead store instance data in structured/storage buffers because those scale better and allow GPU-side indexing, culling and compaction.

---

**Batching is broader than instancing**

Instancing requires meaningful commonality.

Usually:

```text
same mesh
same pipeline
often same material family
```

Batching is the broader strategy of reorganizing work so fewer submissions/state switches are needed.

You may batch by:

```text
pipeline
material
mesh
render pass
texture set
depth bucket
```

But batching has costs.

Suppose two objects are spatially far apart.

Combining them into one monolithic batch may destroy culling granularity.

One invisible object can no longer be removed independently.

Therefore:

$$
\boxed{
\text{submission efficiency}
\leftrightarrow
\text{culling granularity}
}
$$

is a real design trade-off.

---

**Indirect drawing**

In ordinary drawing the CPU records parameters directly:

```cpp
vkCmdDrawIndexed(
    cmd,
    indexCount,
    instanceCount,
    firstIndex,
    vertexOffset,
    firstInstance);
```

Indirect drawing moves those parameters into GPU-visible memory.

For indexed drawing, the conceptual command record is:

```cpp
struct DrawIndexedIndirectCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    uint32_t firstInstance;
};
```

Then:

```cpp
vkCmdDrawIndexedIndirect(..., indirectBuffer, ...);
```

causes the device to read drawing parameters from the buffer.

Vulkan defines indirect commands precisely this way: draw parameters are fetched by the device from a buffer during execution rather than embedded in the command recording itself. [docs.vulkan.org](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdDrawIndirect.html?utm_source=chatgpt.com)

Why is that transformative?

Because the **GPU itself can write that buffer**.

---

**GPU-driven rendering**

Now imagine this pipeline:

```text
scene instances
      ↓
compute shader
      ↓
frustum/occlusion/LOD culling
      ↓
visible instance list
      ↓
indirect command buffer
      ↓
graphics draw consumes buffer
```

No CPU readback is required.

The GPU decides:

$$
\text{what survives}
$$

and directly emits the work description consumed by rendering.

That is the core idea of GPU-driven rendering.

The CPU submits a broad workload once.

The GPU specializes it every frame.

---

**Compute-driven culling example**

Suppose we have one million instances.

Each instance stores:

```cpp
struct Instance
{
    BoundingSphere bounds;
    Mat4 transform;
    uint32_t meshID;
};
```

A compute invocation handles one instance.

Conceptually:

```glsl
uint i = gl_GlobalInvocationID.x;

if (i >= instanceCount)
    return;

Instance instance = instances[i];

if (!IntersectsFrustum(instance.bounds))
    return;

uint slot = atomicAdd(visibleCount, 1);

visibleInstances[slot] = i;
```

Now the compute pass has compacted the visible set.

A later stage constructs indirect draw commands.

The architecture has changed from:

$$
CPU\rightarrow\text{one command per object}
$$

to:

$$
CPU\rightarrow\text{dispatch}
\rightarrow
GPU\text{ visibility}
\rightarrow
GPU\text{ command data}
\rightarrow
GPU\text{ draws}.
$$

---

**Synchronization cannot disappear**

Suppose compute writes:

```text
indirectCommands[]
```

and graphics immediately consumes it.

The memory dependency must be declared.

Conceptually:

```text
compute shader WRITE
        ↓ barrier
indirect-command READ
```

With modern Vulkan synchronization2 terminology, the destination stage for consuming indirect commands is `DRAW_INDIRECT`, and indirect command data uses `INDIRECT_COMMAND_READ`. Khronos' synchronization examples explicitly show a shader-write → draw-indirect-read barrier for this pattern. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/synchronization.html?utm_source=chatgpt.com)

This is an important GPU-programming principle:

$$
\boxed{
\text{GPU-generated data does not imply automatically synchronized data}
}
$$

Computation and ordering are separate responsibilities.

---

**Multi-draw indirect**

Suppose the indirect buffer contains:

```text
command 0
command 1
command 2
...
command 9999
```

With multi-draw indirect, one recorded API command can execute many of these records.

This further reduces CPU submission work.

Vulkan's `multiDrawIndirect` feature controls whether a single indirect command may execute more than one draw record; otherwise the indirect draw count is restricted to zero or one. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/features.html?utm_source=chatgpt.com)

The GPU-driven pipeline then becomes approximately:

```text
compute:
    generate 7,842 visible draw records

graphics:
    execute those 7,842 records from one indirect buffer
```

Khronos' current GPU-driven material describes MDI precisely as the bridge from compute-generated visibility to GPU-side graphics submission. [docs.vulkan.org](https://docs.vulkan.org/samples/latest/samples/performance/multi_draw_indirect/README.html?utm_source=chatgpt.com)

---

**Mesh shaders change the pre-rasterization model**

The classic path is:

```text
vertex/index buffer
      ↓
fixed-function vertex fetch
      ↓
primitive assembly
      ↓
vertex shader
      ↓
optional tessellation/geometry stages
      ↓
rasterizer
```

Mesh shaders replace much of this front end with workgroup-programmable geometry generation.

Conceptually:

```text
workgroup
  reads meshlet data
  performs culling
  generates vertices
  generates primitive indices
        ↓
rasterizer
```

Khronos describes `VK_EXT_mesh_shader` as a compute-like programmable mechanism that can replace the traditional pre-rasterization vertex/primitive pipeline, with mesh shaders producing vertices and primitives directly for rasterization. [docs.vulkan.org](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mesh_shader.html?utm_source=chatgpt.com)

---

**Meshlets**

Mesh shaders become much easier to understand if you first understand meshlets.

Instead of one giant indexed mesh:

```text
500,000 triangles
```

partition it into small clusters, for example:

```text
meshlet:
≤ 64 vertices
≤ 126 triangles
bounding sphere
normal cone
material
```

Now each cluster becomes a convenient unit for:

```text
frustum culling
cone/backface culling
LOD
parallel processing
```

A mesh shader workgroup can process one meshlet.

It loads its compact local vertex/index data, rejects the entire cluster if invisible, and otherwise emits the required vertices and primitives.

This creates much finer control over geometry processing than the traditional “feed vertices one by one through fixed assembly” model.

---

**Task shaders**

Task shaders are an optional stage before mesh shaders.

Their mental model is:

```text
task workgroup
    ↓
coarse analysis / culling / LOD
    ↓
decide how many mesh workgroups exist
    ↓
emit payload
    ↓
mesh shader workgroups
```

A task shader might inspect a collection of meshlets and decide:

```text
meshlets 0–11 invisible → launch nothing
meshlet 12 visible       → launch one mesh group
meshlet 13 high detail   → launch several groups
```

Current Khronos documentation emphasizes exactly this role: task shaders can perform coarse culling and dynamically emit mesh shader groups via a workgroup-level payload. [docs.vulkan.org](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mesh_shader.html?utm_source=chatgpt.com)

Important: mesh shaders are **not automatically faster**.

The extension documentation itself warns that their flexibility can incur platform-dependent cost and that optimized traditional pipelines may remain faster for some workloads. [docs.vulkan.org](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mesh_shader.html?utm_source=chatgpt.com)

That is the right engineering attitude.

---

**Raster-order dependencies**

The GPU wants to execute fragments aggressively in parallel.

But some operations fundamentally care about ordering.

Suppose fragment B reads the color produced by fragment A at the same pixel and then writes another value.

If the two overlap, ordinary independent fragment execution has a data hazard:

$$
A\rightarrow B
$$

must be enforced.

Typical examples include:

```text
framebuffer feedback
certain order-dependent compositing algorithms
per-pixel linked structures
some programmable blending algorithms
```

You cannot assume:

> “Triangle A was submitted before triangle B, therefore every fragment of A finishes first.”

Graphics APIs expose explicit mechanisms for ordering/synchronization where required. Vulkan's current specification includes rasterization-order attachment access and fragment interlock mechanisms for cases where same-pixel ordering must be constrained. [Khronos Registry](https://registry.khronos.org/vulkan/specs/latest/pdf/vkspec.pdf?utm_source=chatgpt.com)

The performance lesson is important:

$$
\boxed{
\text{ordering constraints reduce freedom for parallel execution}
}
$$

so use them only when the algorithm genuinely requires them.

---

**Shader divergence**

GPUs execute nearby shader invocations in groups.

Suppose 32 lanes execute:

```glsl
if (materialType == METAL)
    DoMetal();
else
    DoGlass();
```

If every lane takes `METAL`, the group runs efficiently.

If half take each branch, hardware may need to execute:

```text
DoMetal()
with glass lanes masked

then

DoGlass()
with metal lanes masked
```

Useful lane utilization falls.

This is branch divergence.

The important distinction from CPU branch prediction is:

$$
\boxed{
\text{GPU divergence is primarily about lanes disagreeing}
}
$$

rather than predicting one control path incorrectly.

A branch can be perfectly predictable and still divergent.

---

**Divergence is not “branches are bad”**

Consider:

```glsl
if (distance > maxRange)
    return;
```

If nearly the entire wave takes the same path, this branch may be cheap and save huge work.

Conversely, replacing the branch with both computations and selecting afterward could be worse.

The correct question is:

$$
\text{How coherent is this branch across executing lanes?}
$$

not:

\[
\text{Can I remove every `if`?}
\]

This becomes particularly important with material megashaders.

---

**Register pressure**

Every active shader invocation needs temporary storage.

Suppose a shader uses many live values:

```text
positions
normals
several BRDF intermediates
many texture coordinates
large temporary arrays
```

The compiler may require more registers per invocation.

An SM/CU contains a finite register file.

If each invocation consumes $R$ registers and there are $R_{\text{total}}$ registers available, the rough upper bound on concurrently resident invocations is constrained by

$$
N
\le
\frac{R_{\text{total}}}{R}.
$$

So increasing registers per thread can reduce occupancy.

If pressure becomes severe, values may spill into slower memory.

Therefore a shader optimization that reduces instruction count while doubling register pressure can actually become slower.

This is why GPU performance cannot be reasoned from source-code operation count alone.

---

**Shader constant data**

Not all shader data changes at the same frequency.

Conceptually:

```text
per frame:
camera
time
global environment

per pass:
shadow matrix
render mode

per material:
base color
roughness
texture handles

per object:
model matrix
object ID

per instance:
instance transform
```

Grouping data by update frequency matters for:

```text
CPU update cost
descriptor churn
cache behavior
binding architecture
GPU bandwidth
```

This is the beginning of production renderer data architecture.

---

**Uniform buffers**

A uniform buffer is intended for relatively small read-only shader data shared across many invocations.

Conceptually:

```glsl
layout(binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
};
```

Every vertex/fragment invocation can read the same camera state.

Your Kairo renderer has exactly this concept.

Its `CameraUniform` currently contains:

```text
view matrix
projection matrix
lighting data
camera position
light VP
shadow parameters
viewport parameters
scene parameters
light arrays
background
```

and is uploaded into a Vulkan uniform buffer.

---

**Storage buffers**

Storage buffers are more general.

They can hold large arrays and can support reads and writes depending on shader/API configuration.

For GPU-driven rendering:

```glsl
layout(std430, binding = 4) readonly buffer Instances
{
    InstanceData instances[];
};
```

could store hundreds of thousands of instances.

A compute shader could write:

```glsl
layout(std430, binding = 5) buffer VisibleList
{
    uint visible[];
};
```

The philosophical difference is useful:

$$
\text{uniform buffer}
\approx
\text{small structured shared constants}
$$

whereas

$$
\text{storage buffer}
\approx
\text{general shader-visible structured memory}.
$$

Modern APIs blur the exact performance distinction, but the usage model remains valuable.

Your current main Kairo mesh path does not yet depend on storage-buffer-driven instance/draw generation.

---

**Push constants**

Push/root constants target very small, frequently changing data.

Instead of updating a buffer allocation or descriptor for every draw, small bytes can be embedded into command state.

Your current Kairo renderer uses exactly this.

Its `MeshPushConstants` is:

```cpp
struct MeshPushConstants
{
    std::array<float, 32> Values{};
};
```

That is:

$$
32\times4=128\text{ bytes}.
$$

It carries approximately:

```text
model matrix
normal matrix columns
shadow-receive flag / tint/object ID fields
```

and is sent with:

```cpp
vkCmdPushConstants(...)
```

for each draw.

This is a textbook example of dividing data by update frequency:

```text
camera/lights → uniform buffer
object transform → push constants
textures/material → descriptor set
mesh geometry → vertex/index buffers
```

---

**Render-state objects**

A modern graphics pipeline contains a large amount of state:

```text
shader stages
vertex input
topology
rasterization
culling
front-face
depth test
stencil
multisampling
blending
attachment formats
pipeline layout
```

Older APIs historically allowed much of this to be changed individually as mutable global state.

Modern explicit APIs push toward immutable or semi-immutable pipeline objects.

Why?

Because the driver can compile a coherent hardware configuration ahead of draw time.

The cost is pipeline creation complexity and potentially huge numbers of state combinations.

---

**Pipeline state caching**

Suppose pipeline creation involves:

```text
shader translation
hardware code generation
state validation
linking
internal representation construction
```

Doing that during a frame can cause a visible hitch.

Vulkan therefore has pipeline caches that let pipeline-construction results be reused between related pipelines and, when serialized appropriately, between application runs. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/pipelines.html?utm_source=chatgpt.com)

An important current Kairo detail:

```cpp
vkCreateGraphicsPipelines(
    m_Device,
    VK_NULL_HANDLE,
    ...
);
```

Kairo currently passes:

$$
\boxed{\texttt{VK_NULL_HANDLE}}
$$

as the application pipeline cache.

So **application-provided Vulkan pipeline caching is presently disabled in this path**.

That is not incorrect for a small renderer.

It becomes increasingly relevant as permutation count and pipeline complexity grow.

---

**Shader compilation**

Your `.vert` and `.frag` source is not what Vulkan GPU hardware executes directly.

Your Kairo path loads precompiled:

```text
*.spv
```

files.

At runtime:

```cpp
ReadSpirv(...)
→ vkCreateShaderModule(...)
→ vkCreateGraphicsPipelines(...)
```

So there are multiple compilation levels:

```text
GLSL source
    ↓ compiler outside/around runtime
SPIR-V
    ↓ Vulkan driver
device-specific machine representation
```

The driver-level stage can still be expensive even though GLSL has already become SPIR-V.

That is why pipeline creation can hitch.

---

**Shader permutation explosion**

Suppose your shader supports binary features:

```text
skin?
normal map?
alpha blend?
clearcoat?
shadow receive?
fog?
vertex color?
double sided?
```

Eight binary switches imply up to:

$$
2^8=256
$$

possible variants.

Twenty switches imply:

$$
2^{20}=1\,048\,576.
$$

This is permutation explosion.

Obviously many combinations are invalid or unnecessary, but the combinatorial growth is real.

The engineering problem becomes deciding what should be:

```text
compile-time shader variant
pipeline state
runtime uniform branch
specialization constant
separate pass/material family
```

There is no single correct answer.

---

**Kairo already has a tiny permutation system**

Current `VulkanTriangle` creates separate pipelines for approximately:

```text
static opaque
static transparent
skinned opaque
skinned transparent
static shadow
skinned shadow
debug lines
```

This is already a small permutation matrix.

For the mesh path:

$$
2\text{ skin states}
\times
2\text{ alpha states}
=
4\text{ primary mesh pipelines}.
$$

As you later add:

```text
two-sided
MSAA mode
depth prepass
wireframe
deferred/forward
clearcoat
transmission
```

you will have to decide carefully which dimensions deserve real PSO variants.

This is where engine rendering architecture begins becoming a combinatorial-management problem rather than only a graphics-math problem.

---

**Kairo's current renderer is not GPU-driven yet**

Current `DrawMeshes()` effectively performs:

```text
for every submitted draw
    bind material descriptor
    select static/skinned + opaque/blend pipeline
    push object constants
    bind vertex buffer
    bind index buffer
    vkCmdDrawIndexed
```

So submission granularity remains one indexed draw per mesh draw.

There is no indirect draw in the current `VulkanTriangle` path.

There is no compute-generated visible command buffer in that path.

That gives you a clean future progression:

```text
today:
CPU scene iteration
→ per-object vkCmdDrawIndexed

next:
CPU batching / instancing

then:
GPU instance buffer
→ compute culling
→ indirect command generation
→ MDI/IndirectCount

advanced:
meshlet representation
→ task/mesh shader path
```

That is a real architecture evolution rather than throwing every modern technique into the engine at once.

---

### Black-frame debugging, vertex tracing, complete CPU rasterization, GPU mapping

This is where we prove that we actually understand the previous 100 parts.

---

**A black frame is not one bug**

Suppose the application window appears and nothing renders.

“Graphics are broken” is too vague to be useful.

The frame is a dependency chain:

$$
\text{scene}
\rightarrow
\text{resource upload}
\rightarrow
\text{command recording}
\rightarrow
\text{submission}
\rightarrow
\text{vertex stage}
\rightarrow
\text{clipping}
\rightarrow
\text{rasterization}
\rightarrow
\text{fragment stage}
\rightarrow
\text{depth/stencil}
\rightarrow
\text{attachment}
\rightarrow
\text{presentation}.
$$

A systematic debugger asks:

$$
\boxed{
\text{What is the last stage that can be proven correct?}
}
$$

Do not randomly edit shaders.

Bisect the rendering machine.

---

**The black-frame attack tree**

Use this exact order because each stage removes huge classes of hypotheses.

1. **Prove the frame exists at all.** Clear the target to violent magenta. If magenta never reaches the screen, geometry and shaders are irrelevant; inspect acquire/record/submit/present and render-target routing. In Kairo, that means following `VulkanRendererRuntime::DrawFrame`: wait/retire → acquire → record → submit → present.

2. **Prove the correct render target is being written.** Kairo renders the 3D scene into `VulkanViewportTarget`, then later presents UI to the swapchain. A valid swapchain does not prove the offscreen viewport pass is correct.

3. **Remove geometry complexity.** Draw one hard-coded triangle with no index buffer, no material, no textures, no lighting, no skinning.

4. **Remove depth and culling.** Disable depth and face culling. If the triangle appears, reintroduce those independently.

5. **Replace the fragment shader with constant output.** `outColor = vec4(1,0,1,1);`. If it now appears, rasterization works and the failure is downstream of interpolation/material shading.

6. **Trace one vertex numerically.** Record object, world, view, clip and NDC coordinates. Verify finite $w$, clip inequalities and expected screen position.

7. **Check vertex layout contracts.** Buffer stride, offsets, formats and shader locations must describe the same bytes.

8. **Check draw ranges.** Index count, index type, bound offsets, vertex offsets and actual buffer sizes.

9. **Check attachment compatibility and writes.** Pipeline render-pass compatibility, color write masks, blend state, depth state and final image layouts.

10. **Only then inspect sophisticated systems.** Descriptor contents, material textures, normal maps, shadow passes, HDR exposure and render-graph synchronization.

That is our one deliberate list for the pass because it is meant to become an actual debugging procedure.

---

**Mapping that procedure onto current Kairo**

Your frame begins in:

```text
VulkanRendererRuntime::DrawFrame()
```

Current flow is:

```text
Retire
  ↓
vkWaitForFences

Acquire
  ↓
vkAcquireNextImageKHR

Record
  ↓
RecordFrameCommands()
  ↓
VulkanTriangle::Record()

Submit
  ↓
vkQueueSubmit

Present
  ↓
vkQueuePresentKHR
```

This already gives you a powerful first divide.

If `vkQueuePresentKHR` succeeds but you see only UI background, the swapchain path can be alive while scene rendering into `VulkanViewportTarget` is wrong.

Within `VulkanTriangle::Record()` the scene path is approximately:

```text
rebuild descriptors
update camera uniforms
upload debug vertices
begin command buffer

optional directional shadow pass

begin viewport render pass
set viewport/scissor
DrawMeshes()
DrawDebugLines()
end viewport render pass

optional picking/capture copy

begin presentation pass
draw editor overlay
end pass

end command buffer
```

So a black viewport while UI remains visible strongly narrows the bug.

---

**Broken transform debugging: trace one vertex**

Do not inspect an entire model.

Choose one vertex.

Suppose

$$
p_o=
(1,0,-5,1).
$$

Use identity model and identity view:

$$
M=I,\qquad V=I.
$$

Therefore

$$
p_w=p_o
$$

and

$$
p_v=p_w.
$$

Choose a simple 90° vertical FOV, aspect 1, near 1, far 10.

For a conventional right-handed Vulkan projection, the important relationships are approximately:

$$
x_c=x_v,
$$

$$
y_c=y_v,
$$

$$
w_c=-z_v.
$$

Thus

$$
w_c=5.
$$

A valid Vulkan projection maps the point's depth to about

$$
z_c=\frac{40}{9}\approx4.444.
$$

Therefore:

$$
p_c\approx
(1,0,4.444,5).
$$

Now perspective divide:

$$
x_n=\frac15=0.2,
$$

$$
y_n=0,
$$

$$
z_n=\frac{4.444}{5}\approx0.8889.
$$

All are valid:

$$
-1\le0.2\le1,
$$

$$
-1\le0\le1,
$$

$$
0\le0.8889\le1.
$$

For an 800×600 viewport:

$$
x_f=
\frac{0.2+1}{2}\times800
=
480.
$$

$$
y_f=
\frac{0+1}{2}\times600
=
300
$$

under the simple bottom-left mathematical mapping.

So the vertex should appear around:

$$
\boxed{(480,300)}.
$$

If instead your program gets:

```text
clip w = -5
```

you likely have a sign/convention problem.

If:

```text
w ≈ 0
```

you are near the projection singularity.

If:

```text
z_NDC = -3
```

you have a depth/projection mismatch.

If:

```text
x_NDC = 400
```

you likely have bad matrix data or a missing homogeneous divide.

Tracing one vertex turns “camera broken” into a mathematical diagnosis.

---

**Why this is especially relevant to Kairo**

Current Kairo vertex shader literally gives us the trace:

```glsl
const vec4 worldPosition =
    draw.model * vec4(inPosition, 1.0);

gl_Position =
    camera.projection *
    camera.view *
    worldPosition;
```

So instrumenting one vertex conceptually means extracting:

$$
p_o,
$$

$$
M p_o,
$$

$$
V M p_o,
$$

$$
PVMp_o.
$$

Remember also that Kairo modifies projection Y:

```cpp
projection(1,1) *= -1.0f;
```

because it uses a positive-height Vulkan viewport.

So an apparent Y inversion must be diagnosed with that fact included.

Never debug it as though the projection were a generic textbook matrix.

---

**Complete minimal CPU rasterizer**

Now we reconstruct the rasterizer.

This is deliberately not “pseudocode pretending to be implementation.”

The following C++ is a small coherent triangle rasterizer with:

```text
screen-space triangle setup
top-left edge ownership
pixel-center sampling
depth
barycentrics
perspective-correct interpolation
color output
```

The precondition is that clipping has already happened, because Pass 1 implemented the clip-space stage.

For clarity, this implementation uses a **bottom-left / +Y-up raster coordinate system**. A production framebuffer with top-left storage can flip the row when writing.

```cpp
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

struct Vec2
{
    float x{};
    float y{};

    Vec2 operator+(const Vec2& r) const { return { x + r.x, y + r.y }; }
    Vec2 operator-(const Vec2& r) const { return { x - r.x, y - r.y }; }
    Vec2 operator*(float s) const { return { x * s, y * s }; }
};

struct Vec3
{
    float x{};
    float y{};
    float z{};

    Vec3 operator+(const Vec3& r) const
    {
        return { x + r.x, y + r.y, z + r.z };
    }

    Vec3 operator*(float s) const
    {
        return { x * s, y * s, z * s };
    }

    Vec3 operator/(float s) const
    {
        return { x / s, y / s, z / s };
    }
};

struct Vec4
{
    float x{};
    float y{};
    float z{};
    float w{};
};

struct VertexOut
{
    // Clip-space position after the vertex shader.
    Vec4 clip{};

    // Example varying produced by the vertex shader.
    Vec3 color{};
};

struct ScreenVertex
{
    Vec2 position{};

    // Window/NDC depth. For conventional Vulkan depth:
    // near ≈ 0, far ≈ 1.
    float depth{};

    // Perspective interpolation state.
    float invW{};
    Vec3 colorOverW{};
};

struct Framebuffer
{
    int width{};
    int height{};

    std::vector<Vec3> color;
    std::vector<float> depth;

    Framebuffer(int w, int h)
        : width(w),
          height(h),
          color(static_cast<std::size_t>(w * h)),
          depth(static_cast<std::size_t>(w * h), 1.0f)
    {
    }

    std::size_t Index(int x, int y) const
    {
        // Our mathematical raster coordinates use +Y upward.
        // Memory rows use Y downward, so flip here.
        const int storedY = height - 1 - y;

        return static_cast<std::size_t>(
            storedY * width + x);
    }
};

// -----------------------------------------------------------------------------
// Clip → NDC → viewport.
//
// Assumption:
// the triangle has already been clipped against the clip volume,
// therefore clip.w is valid and nonzero.
// -----------------------------------------------------------------------------
ScreenVertex ToScreen(
    const VertexOut& v,
    int width,
    int height)
{
    const float invW = 1.0f / v.clip.w;

    const float ndcX = v.clip.x * invW;
    const float ndcY = v.clip.y * invW;
    const float ndcZ = v.clip.z * invW;

    ScreenVertex result{};

    // [-1,+1] → [0,width]
    result.position.x =
        (ndcX * 0.5f + 0.5f) *
        static_cast<float>(width);

    // [-1,+1] → [0,height]
    result.position.y =
        (ndcY * 0.5f + 0.5f) *
        static_cast<float>(height);

    result.depth = ndcZ;

    // Store reciprocal W so varyings can later be reconstructed
    // with perspective-correct interpolation.
    result.invW = invW;

    result.colorOverW = v.color * invW;

    return result;
}

// -----------------------------------------------------------------------------
// Signed edge equation.
//
// Positive means p lies on one side of directed edge a→b.
// -----------------------------------------------------------------------------
float Edge(
    const Vec2& a,
    const Vec2& b,
    const Vec2& p)
{
    return
        (b.x - a.x) * (p.y - a.y) -
        (b.y - a.y) * (p.x - a.x);
}

// -----------------------------------------------------------------------------
// Top-left ownership.
//
// Convention here:
// +Y points upward and the triangle is normalized to CCW orientation.
//
// For exact GPU agreement a production software rasterizer should normally
// use fixed/subpixel integer edge equations rather than float equality.
// -----------------------------------------------------------------------------
bool IsTopLeft(
    const Vec2& a,
    const Vec2& b)
{
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;

    return
        (dy > 0.0f) ||
        (dy == 0.0f && dx < 0.0f);
}

bool EdgePasses(
    float value,
    bool topLeft)
{
    if (value > 0.0f)
        return true;

    if (value < 0.0f)
        return false;

    // Exact boundary:
    // only the top/left owner includes it.
    return topLeft;
}

void RasterizeTriangle(
    ScreenVertex v0,
    ScreenVertex v1,
    ScreenVertex v2,
    Framebuffer& framebuffer)
{
    float area =
        Edge(v0.position,
             v1.position,
             v2.position);

    // Degenerate projected triangle.
    if (area == 0.0f)
        return;

    // Normalize to CCW according to this rasterizer's convention.
    if (area < 0.0f)
    {
        std::swap(v1, v2);
        area = -area;
    }

    // -------------------------------------------------------------------------
    // Triangle bounding box.
    // -------------------------------------------------------------------------

    const float minXf = std::min({
        v0.position.x,
        v1.position.x,
        v2.position.x
    });

    const float maxXf = std::max({
        v0.position.x,
        v1.position.x,
        v2.position.x
    });

    const float minYf = std::min({
        v0.position.y,
        v1.position.y,
        v2.position.y
    });

    const float maxYf = std::max({
        v0.position.y,
        v1.position.y,
        v2.position.y
    });

    int minX =
        std::max(
            0,
            static_cast<int>(std::floor(minXf)));

    int maxX =
        std::min(
            framebuffer.width - 1,
            static_cast<int>(std::ceil(maxXf)));

    int minY =
        std::max(
            0,
            static_cast<int>(std::floor(minYf)));

    int maxY =
        std::min(
            framebuffer.height - 1,
            static_cast<int>(std::ceil(maxYf)));

    // Each barycentric weight uses the opposite directed edge.
    const bool edge12TopLeft =
        IsTopLeft(v1.position, v2.position);

    const bool edge20TopLeft =
        IsTopLeft(v2.position, v0.position);

    const bool edge01TopLeft =
        IsTopLeft(v0.position, v1.position);

    const float invArea = 1.0f / area;

    // -------------------------------------------------------------------------
    // Raster loop.
    // -------------------------------------------------------------------------
    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            // Single-sample rasterization:
            // test the pixel center.
            const Vec2 p{
                static_cast<float>(x) + 0.5f,
                static_cast<float>(y) + 0.5f
            };

            const float e0 =
                Edge(
                    v1.position,
                    v2.position,
                    p);

            const float e1 =
                Edge(
                    v2.position,
                    v0.position,
                    p);

            const float e2 =
                Edge(
                    v0.position,
                    v1.position,
                    p);

            // Top-left coverage rule.
            if (!EdgePasses(e0, edge12TopLeft) ||
                !EdgePasses(e1, edge20TopLeft) ||
                !EdgePasses(e2, edge01TopLeft))
            {
                continue;
            }

            // -----------------------------------------------------------------
            // Barycentric coordinates.
            // -----------------------------------------------------------------
            const float lambda0 = e0 * invArea;
            const float lambda1 = e1 * invArea;
            const float lambda2 = e2 * invArea;

            // -----------------------------------------------------------------
            // Depth interpolation.
            //
            // Window/NDC depth is linearly interpolated across the projected
            // triangle.
            // -----------------------------------------------------------------
            const float depth =
                lambda0 * v0.depth +
                lambda1 * v1.depth +
                lambda2 * v2.depth;

            // Reject invalid Vulkan-style depth.
            if (depth < 0.0f || depth > 1.0f)
                continue;

            const std::size_t index =
                framebuffer.Index(x, y);

            // Conventional depth:
            // smaller = nearer.
            if (!(depth < framebuffer.depth[index]))
                continue;

            // -----------------------------------------------------------------
            // Perspective-correct varying interpolation.
            //
            // We interpolate:
            //
            //      color / w
            //      1 / w
            //
            // then divide them.
            // -----------------------------------------------------------------
            const float interpolatedInvW =
                lambda0 * v0.invW +
                lambda1 * v1.invW +
                lambda2 * v2.invW;

            if (std::abs(interpolatedInvW) <
                1.0e-12f)
            {
                continue;
            }

            const Vec3 colorOverW =
                v0.colorOverW * lambda0 +
                v1.colorOverW * lambda1 +
                v2.colorOverW * lambda2;

            const Vec3 finalColor =
                colorOverW /
                interpolatedInvW;

            // -----------------------------------------------------------------
            // Commit successful opaque fragment.
            // -----------------------------------------------------------------
            framebuffer.depth[index] = depth;
            framebuffer.color[index] = finalColor;
        }
    }
}
```

That is a real miniature raster pipeline.

---

**What is still educational rather than hardware-exact in that implementation**

The algorithm is mathematically valid, but a GPU-equivalent reference rasterizer needs stricter engineering.

The edge equations should generally operate in a fixed subpixel representation.

For example, with 8 fractional bits:

$$
x_f=\operatorname{round}(256x).
$$

Then pixel-center coordinates become:

$$
x_{\text{sample}}
=
256x+128.
$$

The edge equation can use 64-bit integers:

$$
E=
(b_x-a_x)(p_y-a_y)
-
(b_y-a_y)(p_x-a_x).
$$

Why?

Because neighboring triangles need to make **exactly identical shared-edge decisions**.

A floating epsilon such as:

```cpp
if (edge >= -1e-6f)
```

does not define a watertight rasterization convention.

It creates scale-dependent ambiguity.

The correct production idea is:

$$
\boxed{
\text{quantized subpixel coordinates}
+
\text{integer edge equations}
+
\text{exact top-left tie breaking}
}
$$

which is much closer to how hardware rasterization is reasoned about.

---

**Incremental edge stepping**

Our educational code recalculates:

$$
E(x,y)
$$

for every pixel.

But because:

$$
E(x,y)=Ax+By+C,
$$

we have:

$$
E(x+1,y)=E(x,y)+A,
$$

$$
E(x,y+1)=E(x,y)+B.
$$

A faster rasterizer computes edge values once at the start of a scan region.

Then moving horizontally only adds precomputed increments.

For the three edges:

```cpp
e0 += stepX0;
e1 += stepX1;
e2 += stepX2;
```

Moving to the next row adds the Y increments.

That turns coverage calculation into cheap integer additions.

The same edge values yield barycentrics.

Thus triangle setup pays once for coefficients and the inner loop becomes extremely regular.

---

**Tile-based raster traversal**

Rather than scan every pixel in the bounding box, partition the screen into tiles such as:

$$
8\times8
$$

or

$$
16\times16.
$$

For each tile, test whether each edge proves:

```text
whole tile outside
whole tile inside
partial overlap
```

Then entire empty blocks can be skipped.

This hierarchical reasoning is conceptually related to Hi-Z.

Modern hardware contains many such coarse/fine stages.

The important mental progression is:

```text
triangle
→ coarse screen region
→ fine coverage
→ samples
```

rather than thinking hardware literally runs a nested `for(y) for(x)` over every bounding-box pixel.

---

**One-for-one mapping: CPU rasterizer → GPU pipeline**

Now we can map every significant CPU operation to GPU machinery.

| CPU reconstruction | Modern GPU/Vulkan equivalent |
|---|---|
| `VertexOut` input | vertex/index buffers + vertex fetch |
| object/world/view/projection math | vertex shader |
| clip-space polygon clipping | fixed-function clipping |
| divide by `w` | perspective divide |
| NDC → screen | viewport transform |
| area determinant | triangle setup/orientation |
| reject backface | culling stage |
| edge equations | rasterizer setup |
| pixel-center test | sample coverage |
| top-left tie | API rasterization convention |
| barycentrics | interpolator |
| `a/w` and `1/w` | perspective-correct varying interpolation |
| fragment function | fragment shader |
| depth compare | depth test |
| stencil state machine | stencil test |
| source/destination recurrence | blend unit |
| write color | color attachment |
| depth array | depth attachment |
| framebuffer arrays | Vulkan framebuffer/image attachments |
| outer triangle loop | draw commands |
| repeated object loop | instancing/batching |
| GPU-generated loop | indirect/GPU-driven drawing |

This is Part 1900.

And it is the point of the entire phase.

The GPU pipeline is no longer a diagram you memorize.

It is a hardware-parallel implementation of mechanisms you could reconstruct yourself.

---

**CPU → Kairo → Vulkan → hardware trace**

For one current Kairo mesh:

```text
Mesh
    contains
    MeshVertex[]
    uint32 indices[]
        ↓
CreateMesh()
        ↓
VulkanHostBuffer vertex buffer
VulkanHostBuffer index buffer
        ↓
RenderScene MeshDraw
        ↓
VulkanTriangle::DrawMeshes
        ↓
select pipeline
        ↓
BindMaterial descriptor set
        ↓
MeshPushConstants:
model / normal data / ID
        ↓
vkCmdBindVertexBuffers
vkCmdBindIndexBuffer
        ↓
vkCmdDrawIndexed
        ↓
vertex fetch
        ↓
triangle.vert
        ↓
worldPosition = M p
        ↓
gl_Position = P V worldPosition
        ↓
clipping
        ↓
perspective divide
        ↓
viewport
        ↓
triangle setup / coverage
        ↓
fragment interpolation
        ↓
triangle.frag
        ↓
PBR + texture evaluation
        ↓
mask discard if necessary
        ↓
depth test
        ↓
optional source-over blend
        ↓
HDR color attachment
+
object-ID attachment
        ↓
viewport color becomes shader-readable
        ↓
editor/presentation path
```

That is the actual machine you are working with now.

---

**One subtle Kairo issue worth understanding: transparent object IDs**

Your viewport has two color outputs:

```text
HDR color
object ID
```

The color attachment can blend.

The object-ID attachment does not use blending.

So a transparent fragment that passes depth testing can write an object ID even though its visible color may only contribute partially.

Because transparent depth writes are disabled, the relationship among:

```text
visible composited surface
object picking identity
draw order
```

is not necessarily trivial.

This is not automatically wrong.

For an editor, picking the nearest submitted transparent object may be desired.

But once transparent overlap becomes complex, picking semantics must be explicitly defined.

Questions arise such as:

```text
Should nearly invisible glass be pickable?
Should alpha < 0.05 count?
Should masked holes write IDs?
Should transparent object picking use a separate pass?
```

That is a real engine-design issue hidden behind what looks like a simple second attachment.

---

**Another Kairo issue: transparent sort key is object-origin distance**

Current sorting uses distance squared from camera to model translation.

That avoids square roots because:

$$
d_1>d_2
\iff
d_1^2>d_2^2
$$

for nonnegative distances.

Good.

But it is not equivalent to true per-fragment ordering.

Consider a wall-sized transparent quad whose origin lies far away but which extends in front of another object's center.

The sort key cannot capture that geometry.

Production renderers therefore combine strategies such as:

```text
object sort
material-specific assumptions
depth peeling
weighted blended OIT
per-pixel linked lists
stochastic transparency
```

depending on cost and quality constraints.

We will revisit those in advanced real-time rendering.

---

**Current Kairo raster-state audit**

The following is what the current Vulkan mesh path actually does, as verified against the connected repository:

| Raster concern | Current Kairo behavior |
|---|---|
| Face culling | disabled (`VK_CULL_MODE_NONE`) |
| Front convention | CCW |
| Polygon mode | fill |
| MSAA | 1× |
| Stencil | absent from viewport D32 target |
| Depth test | enabled |
| Opaque depth write | enabled |
| Transparent depth write | disabled |
| Depth compare | LESS |
| Depth clear | 1.0 |
| Reversed-Z | not implemented |
| Color target | RGBA16F |
| Object-ID target | R32_UINT |
| Depth target | D32_FLOAT |
| Color load/store | CLEAR / STORE |
| ID load/store | CLEAR / STORE |
| Depth load/store | CLEAR / DONT_CARE |
| Transparent blending | straight-alpha source-over |
| Masked alpha | fragment `discard` |
| Viewport/scissor | dynamic |
| Main geometry draw | `vkCmdDrawIndexed` |
| Instancing | not used in main mesh draw |
| Indirect draws | not used in current `VulkanTriangle` path |
| Mesh/task shaders | classic vertex path instead |
| Pipeline cache | application cache currently `VK_NULL_HANDLE` |
| Pipeline variants | static/skinned × opaque/blend + shadow/debug |
| Frame orchestration | acquire → record → submit → present |

This is an unusually useful audit because almost every M111–M113 concept maps directly either to:

$$
\text{implemented}
$$

or

$$
\text{clear next architectural capability}.
$$

---

**How the pipeline state interacts rather than existing independently**

A common weak understanding is to learn:

```text
culling
depth
blending
MSAA
stencil
```

as separate checkboxes.

They interact.

Consider foliage.

If you use alpha blending:

```text
backface culling maybe disabled
depth testing enabled
depth writes often disabled
back-to-front ordering needed
overdraw high
early-Z usefulness reduced
```

If you instead use cutout alpha:

```text
depth writes can remain enabled
sorting becomes easier
but discard introduces aliasing
```

If you add MSAA + alpha-to-coverage:

```text
coverage becomes sample-based
depth can remain per-sample
cutout edges become smoother
bandwidth rises
```

This is the kind of system-level reasoning you need to choose techniques rather than name them.

---

**A production opaque pass**

A typical opaque pass wants something close to:

```text
backface culling       ON
depth test             ON
depth write            ON
blend                  OFF
front-to-back-ish sort
MSAA                    optional
stencil                 technique-dependent
```

because opaque surfaces obey a simple visibility invariant:

$$
\boxed{
\text{nearest visible surface completely replaces farther surfaces}
}
$$

Depth is extraordinarily effective for this case.

---

**A production transparent pass**

Traditional sorted transparency looks more like:

```text
depth test             ON
depth write            OFF
blend                  ON
back-to-front sorting
culling                material-dependent
```

because a transparent surface does not erase all radiance behind it.

This destroys many of the nice ordering freedoms opaque rendering has.

That is why transparent rendering is disproportionately difficult relative to the simple blend equation.

---

**Pipeline permutations versus dynamic state**

Suppose culling changes per material.

You could compile:

```text
opaque_cull_back
opaque_two_sided
transparent_cull_back
transparent_two_sided
...
```

or make cull mode dynamic when supported.

Dynamic state reduces pipeline count.

But there is no universal rule that “more dynamic state is better.”

Hardware/driver implementation, pipeline compilation, command recording and state-change frequency all matter.

The deeper design variable is:

$$
\boxed{
\text{what belongs in immutable pipeline identity}
\quad\text{vs}\quad
\text{runtime command state}
}
$$

This is one of the central Vulkan architecture decisions we will revisit in Phase 23.

---

**Pipeline cache versus engine-level PSO cache**

Do not confuse two caches.

An **engine PSO cache** might map:

```cpp
PipelineKey
→ VkPipeline
```

where `PipelineKey` includes:

```text
shader variant
blend mode
depth mode
cull mode
vertex layout
render target formats
```

This avoids creating duplicate Vulkan pipelines.

A Vulkan `VkPipelineCache`, meanwhile, lets the **driver** reuse internal compilation artifacts during pipeline creation.

They solve different layers:

$$
\boxed{
\text{engine cache}
:
\text{which VkPipeline object have I already made?}
}
$$

$$
\boxed{
\text{driver pipeline cache}
:
\text{can expensive pipeline compilation work be reused?}
}
$$

A mature renderer often has both.

---

**Shader permutations versus runtime branches**

Suppose normal mapping is optional.

Approach A:

```glsl
if (material.hasNormalMap)
    ApplyNormalMap();
```

One shader.

Potential divergence and unnecessary resources.

Approach B:

```text
NORMAL_MAP=0 shader
NORMAL_MAP=1 shader
```

No runtime branch, but another permutation dimension.

If there are 20 such features, compiling every combination becomes impossible.

A production strategy often categorizes features:

```text
large structural feature
→ permutation

small cheap coherent choice
→ runtime uniform branch

material specialization known at pipeline creation
→ specialization constant

entirely separate rendering technique
→ separate pass/shader family
```

This is shader architecture, not merely shader syntax.

---

**GPU-driven rendering changes ownership**

In today's Kairo loop, CPU code decides:

$$
\text{which draws exist}.
$$

In a GPU-driven architecture, CPU decides:

$$
\text{which broad workload exists},
$$

while GPU decides:

$$
\text{which individual instances are visible and emitted}.
$$

That changes ownership and debugging.

Your old debugging question:

> “Did CPU code call DrawIndexed for object 518?”

becomes:

> “Did compute classify instance 518 visible, did it survive compaction, did its indirect record get written, did synchronization make that record visible, and did the indirect draw consume it?”

GPU-driven systems improve throughput.

They also create new observability problems.

A mature implementation therefore needs debugging buffers, counters and capture tools from the start.

---

**Research bridge for Phase 19**

A genuinely useful Phase 19 experiment would compare several raster/submission policies using one identical Kairo scene.

Measure:

$$
T_{\text{CPU submit}},
$$

$$
T_{\text{GPU vertex}},
$$

$$
T_{\text{fragment}},
$$

$$
\text{draw count},
$$

$$
\text{fragment invocations},
$$

$$
\text{overdraw},
$$

$$
\text{visible-instance ratio}.
$$

Construct controlled scenes with:

```text
1,000 objects
10,000 objects
100,000 objects

low overlap
high overlap

one material
many materials

small triangles
large triangles
```

Compare:

```text
baseline per-object draws

CPU batching

instancing

compute culling + indirect draws
```

A falsifiable hypothesis could be:

$$
H:
\text{GPU-driven culling overtakes CPU submission only after a particular object-count/visibility threshold.}
$$

The independent variables would be:

$$
N_{\text{objects}},
$$

$$
\text{visibility fraction},
$$

$$
\text{material diversity}.
$$

The dependent metrics would be frame time broken into CPU submission and GPU stages.

That is far more meaningful than claiming “GPU-driven is faster.”

You find the boundary where it becomes faster.

---

**Phase-19 reconstruction**

You should now be capable of mentally rebuilding:

```text
CPU scene
    ↓
vertex/index/instance data
    ↓
draw command
    ↓
vertex fetch
    ↓
vertex processing
    ↓
clip coordinates
    ↓
clipping
    ↓
perspective divide
    ↓
viewport
    ↓
primitive orientation/culling
    ↓
triangle setup
    ↓
edge equations
    ↓
coverage / MSAA mask
    ↓
barycentrics
    ↓
perspective-correct varyings
    ↓
fragment shader
    ↓
alpha discard
    ↓
stencil/depth
    ↓
blend
    ↓
attachments
    ↓
store/resolve
    ↓
later pass / presentation
```

And alongside that, a second pipeline exists:

```text
CPU scene
     ↓
submission architecture
     ↓
per-object draw
     OR
instanced draw
     OR
indirect command buffer
     OR
GPU-generated draw
     OR
mesh/task workgroups
     ↓
same raster backend
```

That second chain is why rasterization knowledge and GPU architecture are inseparable.

---


| Concept | Reconstruction cue |
|---|---|
| **Stencil** | Integer per-sample state; compare reference vs stored value; separate fail/depth-fail/pass operations. |
| **Face culling** | Projected signed area → winding → front/back classification. |
| **Negative determinant** | Orientation reversal can invert winding. |
| **Two-sided** | Disable culling, then define back-face normal/material semantics. |
| **Viewport** | NDC → framebuffer transform. |
| **Scissor** | Rectangular fragment rejection. |
| **Framebuffer** | Collection of color/depth/stencil attachments participating in rendering. |
| **Load/store** | Is old data needed at pass start? Is new data needed after pass end? |
| **MSAA** | Multiple exact coverage/depth/color samples per pixel; resolve afterward. |
| **SSAA** | Render full shading at higher spatial resolution then filter down. |
| **Blend equation** | $C_o=F_sC_s+F_dC_d$. |
| **Straight source-over** | $C_o=\alpha_sC_s+(1-\alpha_s)C_d$. |
| **Premultiplied source-over** | $C'_o=C'_s+(1-\alpha_s)C'_d$. |
| **Transparency** | Ordinary source-over is order dependent. |
| **Cutout** | Alpha threshold → discard; can retain ordinary depth writes. |
| **Alpha-to-coverage** | Alpha modulates MSAA sample coverage instead of conventional blend opacity. |
| **Overdraw** | Fragment work exceeding visible output work. |
| **Early-Z** | Establish occluder depth first so hidden work can die before expensive shading. |
| **Hi-Z conventional** | Conservative coarse max-depth reasoning for LESS-style depth. |
| **Hi-Z reversed** | Conservative coarse min-depth reasoning for GREATER-style depth. |
| **Instancing** | One geometry draw × many instance records. |
| **Indirect draw** | Draw arguments stored in GPU-readable memory. |
| **GPU-driven** | Compute culls/compacts/generates command data consumed by graphics. |
| **MDI** | Many indirect records consumed by one recorded draw command. |
| **Mesh shader** | Workgroup generates vertex/primitive output directly for rasterizer. |
| **Task shader** | Optional coarse workgroup that emits mesh-shader work. |
| **Divergence** | Lanes disagree about control flow. |
| **Register pressure** | More registers/thread → potentially fewer resident threads / spills. |
| **Uniform buffer** | Shared read-only structured shader constants. |
| **Storage buffer** | Larger/general shader-visible structured memory. |
| **Push constants** | Tiny high-frequency command-state data. |
| **PSO** | Precompiled collection of graphics shader + fixed-function state. |
| **Pipeline cache** | Reuse internal pipeline compilation results. |
| **Permutation explosion** | Independent feature dimensions multiply shader/PSO variants. |
| **Black frame** | Prove pipeline stages in order; find last known-good stage. |
| **Vertex trace** | object → world → view → clip → NDC → screen. |
| **CPU rasterizer** | fixed-point/edge tests → top-left → barycentrics → depth → interpolation. |

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|Next chapter]]
