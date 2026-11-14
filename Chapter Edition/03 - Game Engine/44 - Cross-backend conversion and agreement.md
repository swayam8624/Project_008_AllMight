# 44 — Cross-backend conversion and agreement

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/43 - Compiling renderer intent into backend work|Previous chapter]] · [[Chapter Edition/03 - Game Engine/45 - Rasterizing kernels and connecting world identity|Next chapter]]

Two APIs can represent similar work with different resource and synchronization rules. This chapter tests conversion at those seams and states what agreement means for images and execution.

**By the end:** Define parity criteria for a cross-backend path, including formats, coordinate conventions, synchronization, and output comparison.

**Recall question:** Which differences are permitted by the contract, and what image or state witness would reveal a conversion error?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 105762–108498; source-span SHA256 806a7c1985cb289efca19c9e0364ba23b697eb0023c90548eb2fb80d3dc191ea. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### REAL CROSS-BACKEND CONVERSION & AGREEMENT

**backend conversion boundaries**

There should be one explicit point where neutral data becomes native data.

For example:

```text
MeshDraw
      ↓
MetalDraw
```

or:

```text
MeshDraw
      ↓
Vulkan draw uniforms/descriptors.
```

Conversion must not happen randomly across gameplay/editor code.

---

**Kairo's Metal boundary is explicit**

`MetalRendererRuntime::Convert(const MeshDraw&)` converts:

$$
Mesh,
Model,
NormalMatrix,
PBRMaterial,
ObjectID,
texture handles,
shadow flags
$$

into its Metal detail structure.

That is a good boundary.

The neutral object does not contain Metal types.

---

**Conversion boundary can correct backend representation**

CPU matrix may be row-major.

MSL expects data in representation suitable for shader code.

Vulkan GLSL/SPIR-V may use a different upload convention.

Conversion layer is allowed to transpose/copy.

Shared scene semantics remain unchanged.

---

**portable mesh representation**

Kairo's `MeshVertex`:

```cpp
struct MeshVertex
{
    Vec3f Position;
    Vec3f Color;
    Vec3f Normal;
    Vec2f TexCoord;
};
```

plus:

```text
uint32 indices
optional parallel skin stream.
```

This is backend-neutral.

Vulkan turns fields into vertex attribute formats.

Metal binds the bytes into its vertex/shader convention.

---

**Why CPU mesh format should not equal native vertex descriptor object**

Asset representation should survive backend changes.

Bad:

```cpp
struct Mesh
{
    VkVertexInputAttributeDescription
        attributes[4];
};
```

Good:

```cpp
MeshLayout
{
    Position Float3
    Normal Float3
    UV Float2
}
```

then backend translates.

---

**Portable mesh should still be expressive**

Future mesh layout might include:

```text
position
normal
tangent
UV0
UV1
color
joint indices
joint weights
morph streams
```

Renderer can reject unsupported layouts through capability/feature contract.

Do not hardcode backend API objects into assets.

---

**portable material representation**

Kairo's `PBRMaterial` is a good example.

It stores:

$$
BaseColor,
Metallic,
Roughness,
AO,
Emissive,
NormalScale,
AlphaCutoff,
AlphaMode,
DoubleSided
$$

plus neutral texture handles.

This is semantic PBR data.

No descriptor set.

No sampler object.

No `MTLTexture*`.

Excellent.

---

**Material semantics must be canonical**

Example:

$$
Roughness\in[0.04,1]
$$

must mean same thing on every backend.

Texture channel conventions must match.

Color-space semantics must match.

Normal-map convention must match.

Then shaders can be compared meaningfully.

Multi-backend parity begins at the material contract.

---

**portable scene extraction**

Scene system might contain:

```text
ECS entities
physics objects
animation components
editor-only objects
asset IDs
```

Renderer should not directly inspect all that.

Extract:

```text
RenderScene
```

containing:

$$
MeshDraws,
Lights,
Environment.
$$

Kairo already does this.

---

**Why extraction is valuable**

Gameplay scene can change architecture.

Renderer doesn't care.

Renderer accepts one canonical frame snapshot.

It also enables:

- deterministic tests,
- headless captures,
- recording/replay,
- backend comparison.

You can serialize a `RenderScene` and replay it through Metal and Vulkan.

That is extremely powerful.

---

**runtime render bridge**

`RendererRuntime` is Kairo's current bridge.

Application says:

```cpp
renderer.CreateMesh(...)
renderer.CreateTexture(...)
renderer.SubmitRenderScene(...)
renderer.SetCameraPose(...)
renderer.DrawFrame(...)
```

Runtime dispatches to current backend via `std::variant`.

This is a clean façade.

---

**Static polymorphism alternative**

Instead of virtual interface:

```cpp
IRenderBackend
```

or variant, one could template backend.

But runtime selection often requires:

$$
std::variant
$$

or dynamic polymorphism.

Kairo's variant approach has useful properties:

- concrete backend types remain known,
- no manual vtable interface proliferation,
- backend-specific accessors can still exist.

---

**GPU handles must stay private**

Do not allow:

```cpp
MeshDraw
{
    VkBuffer VertexBuffer;
}
```

because now Metal cannot consume `MeshDraw`.

Neutral handle:

```cpp
MeshHandle Mesh;
```

Backend owns:

Vulkan:

```text
VkBuffer
VkDeviceMemory
```

Metal:

```text
MTLBuffer
```

D3D12:

```text
ID3D12Resource
```

and resolves handle internally.

---

**Tooling exception**

Sometimes editor needs native texture.

Kairo exposes:

```text
MetalViewportTexture
VulkanViewportTexture
Direct3D12ViewportTexture
```

as explicit tooling APIs.

That is acceptable because non-portability is isolated.

Ordinary scene extraction remains native-handle-free.

---

**Generation is important for borrowed native views**

Kairo native viewport info includes:

$$
Generation.
$$

Resize may replace texture.

Editor sees generation change and knows previously cached native handle is stale.

That is exactly how borrowed cross-layer handles should be managed.

---

**backend-specific shader compilation**

There is no requirement that every backend use same shader binary.

Current Kairo already demonstrates separate strategies.

Vulkan:

$$
GLSL
\rightarrow
SPIR\text{-}V.
$$

Metal backend currently:

$$
MSL\ source
\rightarrow
runtime\ MTLLibrary.
$$

Maveb:

$$
MSL
\rightarrow
offline\ metallib.
$$

All can implement same material model.

---

**Backend-specific compilation is not duplication of renderer semantics**

Shader source may differ.

But formulas should agree:

$$
GGX,
Fresnel,
normal\ mapping,
tone\ mapping.
$$

You can share:

- generated constants,
- material ABI definitions,
- tests,
- CPU reference equations.

Source language does not have to be identical.

---

**cross-API shader language strategies**

There are several valid strategies.

One is separate native shaders:

```text
GLSL for Vulkan
MSL for Metal
HLSL for D3D12
```

Highest native control.

More maintenance.

Another is one source language plus translation:

```text
HLSL
→ DXIL
→ SPIR-V
→ MSL
```

using compiler/toolchains.

Another is custom shader IR/code generation.

The best approach depends on engine scale.

---

**Why source translation is attractive**

One material shader source.

Backend compiler generates target.

Avoid formula drift.

But translated shader may not expose native-specific features elegantly.

Special extensions still require backend-specific branches.

Again:

$$
\boxed{
\text{shared where semantic, specialized where native strength matters}.
}
$$

---

**Shader ABI is more important than source language**

CPU side needs to know:

$$
buffer slot,
texture slot,
constant layout,
vertex attributes,
push/root constants.
$$

Whether source was GLSL or MSL is secondary.

A stable reflected metadata contract is key.

---

**shader reflection**

Reflection extracts information from compiled shader:

```text
inputs
outputs
buffer bindings
texture bindings
samplers
constant layouts
resource array sizes
threadgroup size
```

For example:

```text
Camera:
    set 0 binding 0
    size 256

Material:
    set 1 binding 2
    size 64

BaseColor:
    texture slot 0
```

Then pipeline system can validate CPU bindings.

---

**Reflection prevents ABI drift**

Without reflection:

shader changes:

```glsl
binding=3
```

CPU still binds:

```text
binding=2.
```

Runtime visual corruption.

With reflection/build-time validation:

$$
\boxed{
\text{binding mismatch becomes build/initialization error}.
}
$$

Much better.

---

**Reflection sources differ by backend**

SPIR-V has structured decorations that can be reflected.

Metal can expose pipeline/function reflection information under appropriate creation options/tools.

D3D has reflection APIs.

A cross-backend build pipeline can normalize these into one engine metadata format.

---

**pipeline-layout generation from metadata**

Suppose reflection says:

```text
set 0:
    Camera UBO
    Shadow texture

set 1:
    Material UBO
    5 textures
```

Vulkan backend can generate:

```text
VkDescriptorSetLayout
VkPipelineLayout.
```

Metal backend can map metadata to expected buffer/texture slots or argument-buffer layout.

D3D12 can generate root signature/table mapping.

Same shader contract.

Different native layout object.

---

**Generated layout is safer than duplicated hand declarations**

Bad architecture:

```text
shader says binding 5

C++ manually says binding 5

Metal manually says slot 5

D3D manually says register 5
```

Four places can drift.

Better:

$$
shader/reflection\ metadata
\rightarrow
generated\ binding\ contract.
$$

---

**Metal versus Vulkan coordinate conventions**

This deserves careful separation.

Engine should define one canonical world-space convention.

Kairo currently documents:

$$
\boxed{
\text{right-handed world}
}
$$

with:

$$
+Y
$$

as up.

Then camera builds one projection convention.

---

**Depth convention**

Vulkan clip/NDC depth convention uses:

$$
z\in[0,1].
$$

Metal similarly uses:

$$
z\in[0,1].
$$

This makes Metal↔Vulkan projection sharing easier than OpenGL, where conventional NDC depth historically uses:

$$
[-1,1].
$$

Still, framebuffer Y mapping must be handled deliberately.

---

**Kairo's current camera convention**

Its `Projection()` deliberately flips:

$$
P_{11}
\leftarrow
-P_{11}
$$

with comment:

> renderer uses Vulkan's positive-height viewport; flip clip-space Y once.

Tests explicitly require:

$$
projection(1,1)<0.
$$

So Kairo has effectively chosen a Vulkan-oriented clip projection as its renderer-level convention.

Metal backend consumes that same scene/camera state, and its presentation/capture path is expected to produce top-left-oriented output like all other backends.

---

**Better architectural formulation**

Instead of saying:

> engine camera is Vulkan camera,

define:

```text
CanonicalClipConvention:
    right-handed view
    depth 0..1
    framebuffer-facing Y convention chosen once
```

Then each backend adapter decides whether it needs:

- projection Y flip,
- viewport inversion,
- texture-coordinate flip,
- winding correction.

The key rule:

$$
\boxed{
\text{adapt once, not in random shaders}.
}
$$

---

**Winding can change when Y is flipped**

A screen-space Y inversion changes orientation.

Triangle signed area:

$$
A=
(x_1-x_0)(y_2-y_0)
-
(y_1-y_0)(x_2-x_0).
$$

Replace:

$$
y\rightarrow-y.
$$

Then:

$$
A'\!=\!-A.
$$

So front/back winding reverses.

Backend must compensate through:

- front-face convention,
- viewport/projection strategy,
- cull state.

This is why coordinate adaptation cannot be treated as only “image upside down.”

---

**Texture-origin conventions**

Asset UVs should remain canonical.

Do not globally mutate UVs for each backend unless necessary.

Presentation/readback APIs may use different row orientation.

Kairo's neutral `ViewportCapture` contract explicitly requires:

$$
\boxed{
\text{top-left-origin row-major RGBA8}
}
$$

for every backend.

Excellent.

Backend converts native readback to that canonical format.

---

**synchronization adaptation**

This is where bad abstraction often appears.

Do not define:

```cpp
renderer.PipelineBarrier(
    srcStage,
    dstStage,
    ...);
```

as neutral API.

That is Vulkan-shaped.

Instead graph says:

```text
Resource X:
    previous = CopyDestination
    next = ShaderRead.
```

Backend translates.

---

**Vulkan translation**

Logical:

$$
CopyDestination
\rightarrow
ShaderRead.
$$

Could become:

$$
srcStage=TRANSFER,
$$

$$
srcAccess=TRANSFER_WRITE,
$$

$$
dstStage=FRAGMENT/COMPUTE,
$$

$$
dstAccess=SHADER_READ,
$$

plus:

$$
TRANSFER_DST
\rightarrow
SHADER_READ_ONLY.
$$

---

**Metal translation**

Same logical transition with tracked resource might require:

$$
\boxed{\text{no explicit image-layout barrier}}
$$

because Metal handles relevant hazard state.

If untracked and crossing encoder scopes, backend may emit:

- memory barrier,
- fence,
- event,
- encoder boundary as appropriate.

Same intent.

Native strength preserved.

---

**Synchronization abstraction should express dependency, not primitive**

Engine:

$$
ProducerUsage
\rightarrow
ConsumerUsage.
$$

Backend:

Vulkan:

$$
barrier/semaphore.
$$

Metal:

$$
tracked hazard/fence/event.
$$

D3D12:

$$
resource barrier/fence.
$$

That is correct portability.

---

**CPU reference testing against Metal**

GPU algorithms should have small CPU oracles where feasible.

Suppose Metal compute kernel transforms points:

$$
p'=Mp.
$$

CPU reference computes same.

Test random deterministic inputs.

Compare:

$$
\epsilon_i=
|GPU_i-CPU_i|.
$$

Accept:

$$
\epsilon_i
\le
\epsilon_{abs}
+
\epsilon_{rel}|CPU_i|.
$$

This handles both small and large magnitudes.

---

**CPU oracle candidates in your projects**

Excellent candidates include:

```text
skin matrix transform
normal transform
projection
prefix scan
radix histogram
BVH visibility
Gaussian covariance projection
tile bounds
PBR BRDF evaluation
TSDF integration
voxel agreement
```

This is exactly how Maveb should protect GPU kernels.

---

**Avoid bitwise float equality**

CPU and GPU may differ due to:

- fused multiply-add,
- operation ordering,
- precision contraction,
- transcendental implementation.

Use tolerance derived from numerical conditioning.

For deterministic integer scan/radix logic, exact equality may be appropriate.

For floating geometry:

$$
relative+absolute\ tolerance.
$$

---

**Property testing beats only example testing**

Instead of one point:

$$
(1,2,3)
$$

generate thousands of seeded cases.

Check invariants:

Projection finite.

Covariance remains PSD-ish within tolerance.

Sorted keys nondecreasing.

Prefix scan monotonic for nonnegative counts.

Tile IDs within bounds.

This finds entire bug classes.

---

**visual agreement across backends**

Exact image equality is usually too strict.

Rasterization may differ slightly because of:

- precision,
- derivative implementation,
- filtering,
- edge rules,
- shader compiler decisions.

We need meaningful image metrics.

---

**Per-pixel error**

For linear HDR pixels $A_i,B_i$:

$$
e_i=A_i-B_i.
$$

Mean squared error:

$$
MSE=
\frac1N
\sum_i
\|e_i\|^2.
$$

RMSE:

$$
\boxed{
RMSE=
\sqrt{MSE}.
}
$$

Useful first metric.

---

**Relative error**

Dark/bright values behave differently.

Relative form:

$$
e_{rel}
=
\frac{
|A-B|
}{
\max(|A|,|B|,\epsilon)
}.
$$

Useful for HDR.

Do comparison in linear space before tone mapping when possible.

Then separately compare final presentation.

---

**PSNR**

For normalized LDR values:

$$
PSNR=
10
\log_{10}
\left(
\frac{MAX^2}{MSE}
\right).
$$

Higher is better.

Useful for regression but not sufficient alone.

A spatially shifted edge can give high error while scene is visually correct.

---

**Masked comparison**

Some pixels are inherently unstable:

- silhouette edges,
- shadow boundaries,
- transparency,
- stochastic effects.

A robust backend agreement suite can compare:

$$
\text{stable interior mask}
$$

strictly.

Then evaluate edge pixels with larger tolerance.

---

**Compare semantic debug views**

Rather than only final beauty image, compare:

```text
Normals
Linear depth
Object IDs
Albedo
Roughness
Shadow visibility
Lighting-only
Motion vectors
```

If final images differ, these isolate where divergence begins.

This is far more useful than a single screenshot diff.

---

**Current Kairo capture contract helps**

Every backend must return:

$$
RGBA8,
$$

$$
top-left origin,
$$

$$
row-major.
$$

That normalization removes irrelevant presentation convention differences from tests.

A future backend agreement test can render same `RenderScene` through Vulkan and Metal, capture both, and compare.

---

**Object IDs should match exactly**

Final colors may tolerate differences.

Object IDs should not.

For identical scene and camera:

$$
ID_{Metal}(x,y)
=
ID_{Vulkan}(x,y)
$$

away from rasterization boundary ambiguity.

This is an excellent structural test.

Depth can be compared numerically.

Normals can be angle-compared:

$$
\theta=
\cos^{-1}
(
clamp(
n_A\cdot n_B,
-1,1
)).
$$

---

**one render-graph pass on Vulkan and Metal**

Take a shadow pass.

Shared graph declaration:

```cpp
graph.AddPass(
    "DirectionalShadow",
    {
        Write(
            shadowDepth,
            DepthAttachment)
    },
    [&]
    {
        backend.EncodeShadow(...);
    });
```

Shared intent:

$$
\boxed{
\text{write depth-only shadow map}.
}
$$

Now implementation diverges.

---

**Vulkan shadow mapping**

Backend may:

- transition image to depth attachment layout,
- begin render pass/dynamic rendering,
- bind depth-only pipeline,
- bind descriptors,
- draw shadow casters,
- transition to shader-read layout.

Potential barrier:

$$
DEPTH_WRITE
\rightarrow
SHADER_READ.
$$

---

**Metal shadow mapping**

Backend may:

- use private depth texture,
- begin render encoder,
- choose clear/load action,
- bind shadow pipeline,
- draw,
- end encoder,
- let tracked hazards handle later sampling.

If shadow were transient and never sampled, Metal might exploit memoryless storage.

But because shadow is sampled later:

$$
Store
$$

is required.

Same intent.

Native choices differ.

---

**Do not force Vulkan to imitate Metal tile semantics**

Vulkan backend may optimize via:

- dynamic rendering,
- explicit render pass,
- transient attachments,
- subpass behavior,
- memory aliasing.

Metal backend may use:

- memoryless attachments,
- tracked hazards,
- heaps,
- argument buffers.

Both implement:

$$
\text{same graph resource dependency}.
$$

That is a strong abstraction.

---

**shared engine intent, native backend strength**

This is the culmination of Phase 24.

A good renderer architecture does **not** say:

> Metal and Vulkan are the same.

It says:

> The scene means the same thing on Metal and Vulkan.

That difference is everything.

---

**Shared intent layer**

Shared:

```text
Mesh
Material
Camera
Light
RenderScene

Graph Pass
Graph Resource
Read/Write dependency
Texture format intent
Buffer usage intent

PBR equations
Depth convention
Capture convention
Object IDs
```

Native:

```text
VkImage / MTLTexture
VkDescriptorSet / argument buffer/direct slots
VkPipeline / MTLRenderPipelineState
VkBarrier / Metal hazard tracking
VkSemaphore / MTLSharedEvent
swapchain / CAMetalDrawable
```

---

**The abstraction inversion trap**

Bad:

```text
Engine
 ↓
Fake Vulkan API
 ↓
Metal backend translates Vulkan concepts
```

Now Metal is second-class.

Better:

```text
Engine Rendering Intent
      ↓
Render Graph / RHI Semantic Layer
      ↓            ↓
   Vulkan        Metal
```

No backend is the master language.

---

**Native-strength extensions**

Suppose Metal supports tile shader optimization beneficial to one effect.

Expose optional renderer feature:

```cpp
if (
    caps.SupportsTileDeferredPath)
{
    backend.EncodeNativeTilePath(...);
}
else
{
    EncodePortablePath(...);
}
```

Do not prohibit Metal optimization because Vulkan path differs.

Likewise Vulkan descriptor indexing should not be forbidden because legacy OpenGL cannot match it.

---

**One feature, different algorithms**

Even more importantly, two backends do not have to run exactly the same algorithm if final semantics match.

Example:

Metal:

$$
tile-local\ deferred\ lighting.
$$

Vulkan desktop:

$$
compute\ clustered\ lighting.
$$

Both produce:

$$
HDRLighting.
$$

Graph contract can remain:

```text
reads GBuffer
reads Lights
writes HDR.
```

This is backend-native strength.

---

**But visual agreement still matters**

Backend-specific algorithms must satisfy reference bounds.

You cannot say:

> Metal looks different because optimization.

You need:

$$
\boxed{
\text{defined acceptable output agreement}.
}
$$

That may be:

- exact IDs,
- close depth,
- close BRDF,
- bounded final HDR error.

Architecture plus validation.

---

**Current Kairo architecture assessment**

Kairo is already doing several things correctly.

Its scene representation is neutral.

Its handles are neutral.

Its renderer façade dispatches to concrete runtimes.

Its editor/tooling-native access is isolated.

Its captures normalize output orientation.

Its render graph can infer dependencies/states/lifetimes/alias slots.

Its backend selection is explicit and testable.

But several systems remain intentionally intermediate.

The neutral resource API is domain-specific rather than a full buffer/texture/pipeline RHI.

GPU capabilities are not yet represented by a rich neutral capability object.

Render-graph resource states are too coarse for advanced storage/indirect/ray-tracing synchronization.

Kairo's graph does not yet pass-cull.

It has no queue assignment/cross-queue scheduler.

Its alias compatibility is not yet native-resource-safe enough for real backing-memory reuse.

Its transition hooks exist, but full native barrier generation is not yet broadly integrated.

That is a strong foundation, not a finished render-graph compiler.

---

**Current Maveb render-graph assessment**

Maveb's graph has strengths Kairo currently lacks.

It uses generational resource handles.

It performs backward pass liveness.

It supports:

$$
sideEffect
$$

passes.

It supports explicit exported outputs.

It culls unused passes.

It generates DOT visualization.

It computes lifetimes.

But its current graph is less detailed than Kairo regarding:

- semantic resource states,
- native transition planning,
- alias slot planning.

The obvious research/engineering direction is to combine both designs.

---

**A stronger combined graph**

Imagine:

```cpp
ResourceHandle hdr =
    graph.CreateTexture(
        "Scene HDR",
        TextureDesc{...});

PassHandle geometry =
    graph.AddPass({
        .Name = "Geometry",
        .Queue = Graphics,

        .Writes = {
            {
                hdr,
                ColorAttachmentWrite
            }
        }
    });

PassHandle bloom =
    graph.AddPass({
        .Name = "Bloom",
        .Queue = Compute,

        .Reads = {
            {
                hdr,
                SampledRead
            }
        },

        .Writes = {
            {
                bloomHalf,
                StorageWrite
            }
        }
    });
```

Compiler determines everything else.

---

**The compiler output**

A compiled pass might contain:

```cpp
struct CompiledPass
{
    PassID Pass;

    QueueClass Queue;

    std::vector<Barrier>
        PreBarriers;

    std::vector<QueueWait>
        Waits;

    std::vector<QueueSignal>
        Signals;

    std::vector<PhysicalResourceBinding>
        Resources;

    ExecuteCallback Execute;
};
```

Then backend translator converts generic synchronization into native API.

---

**Physical allocation plan**

For every transient resource:

```text
logical handle
first use
last use
native compatibility class
size
alignment
alias slot
```

For Metal:

```text
alias slot
→ MTLHeap offset/resource allocation strategy
```

For Vulkan:

```text
alias slot
→ VkDeviceMemory region
→ VkImage/VkBuffer binding
```

Potentially different physical plan per backend.

Same lifetime graph.

---

**Backend-specific resource compatibility**

Metal may say two textures can share one heap allocation pattern.

Vulkan may have different memory type/alignment requirements.

Therefore graph should produce:

$$
\boxed{
\text{logical non-overlap proof}
}
$$

while backend allocator decides:

$$
\boxed{
\text{whether physical aliasing is legal}.
}
$$

Important separation.

---

**Render graph should not own every persistent resource**

Assets:

```text
textures
mesh buffers
materials
history
shadow atlas
```

often live in renderer resource manager.

Graph imports them.

Transient graph resources are graph-allocated.

Thus:

```text
Renderer Resource Registry
    persistent/native resources

Render Graph
    frame dependency/lifetime plan
```

These are complementary systems.

---

**Frame graph versus persistent graph**

Frame graph describes:

$$
\text{one frame's dependency structure}.
$$

Persistent systems track:

$$
\text{cross-frame resources and state}.
$$

Examples:

$$
TAAHistory,
ShadowCache,
ProbeAtlas.
$$

Do not force render graph to recreate persistent resource identity every frame.

Import/reference it.

---

**GPU-driven rendering integration**

Future:

```text
CullCompute
    writes VisibleInstances
    writes IndirectArgs

DrawOpaque
    reads VisibleInstances
    reads IndirectArgs
```

Graph automatically infers:

$$
COMPUTE_WRITE
\rightarrow
INDIRECT_READ.
$$

Vulkan barrier maps correctly.

Metal tracked/untracked dependency maps correctly.

This is where render graph becomes critical.

---

**Ray tracing integration**

Likewise:

```text
BuildBLAS
BuildTLAS
RayTraceReflections
Denoise
Composite
```

Graph tracks:

$$
AccelerationStructureWrite
\rightarrow
RayTracingRead.
$$

The neutral abstraction must grow, but core idea stays identical.

---

**A compact dependency derivation**

Let resource $r$.

Maintain:

$$
W_r
=
\text{last writer}.
$$

$$
R_r
=
\text{set of active readers}.
$$

For read by pass $p$:

$$
W_r\neq\varnothing
\Rightarrow
W_r\rightarrow p.
$$

Then:

$$
R_r\leftarrow
R_r\cup\{p\}.
$$

For write:

$$
W_r\neq\varnothing
\Rightarrow
W_r\rightarrow p.
$$

For every:

$$
q\in R_r:
q\rightarrow p.
$$

Then:

$$
R_r=\varnothing
$$

and:

$$
W_r=p.
$$

That is the heart of Kairo's current graph compiler.

---

**Why declaration order still influences hazard inference**

Current Kairo processes accesses in original pass declaration order before topological sorting.

This works as a version-ordering model:

each write creates a new logical version of resource in declared program order.

But an even more sophisticated render graph often models explicit resource versions:

```text
HDR v0
HDR v1
HDR v2
```

so dataflow becomes unambiguous.

This is called resource versioning in many frame-graph systems.

---

**Resource versioning**

Instead of:

```text
Pass A writes X
Pass B writes X
```

represent:

$$
X_0
\rightarrow
A
\rightarrow
X_1
$$

then:

$$
X_1
\rightarrow
B
\rightarrow
X_2.
$$

Readers explicitly consume:

$$
X_1
$$

or:

$$
X_2.
$$

This prevents accidental ordering ambiguity and improves culling.

---

**SSA analogy**

This resembles static single assignment.

Each write produces a new resource version.

Example:

```text
HDR0 = Geometry()
HDR1 = TAA(HDR0)
HDR2 = BloomComposite(HDR1)
```

Now dependencies are obvious.

Frame graphs often benefit from SSA-like thinking.

---

**Render graph and compiler optimization analogy**

Render graph optimization maps beautifully to compiler concepts:

| Compiler | Render graph |
|---|---|
| instruction | pass |
| value | resource version |
| def-use chain | write/read edge |
| dead-code elimination | pass culling |
| liveness analysis | resource lifetime |
| register allocation | transient aliasing |
| instruction scheduling | pass/queue scheduling |
| memory barrier | GPU synchronization |
| IR lowering | backend command generation |

This is not metaphorical fluff.

It is structurally almost the same problem.

---

**Research bridge: Apple locality**

Now combine render graphs with Apple GPU architecture.

If graph knows:

```text
Depth
first = GBuffer
last = GBuffer
```

and no later pass reads it:

$$
\boxed{
\text{Metal backend could choose memoryless depth}.
}
$$

If graph knows color is written and consumed in same tile-compatible sequence, backend may optimize pass grouping.

Thus render graph becomes a machine for discovering tile-locality opportunities.

---

**Vulkan analogue**

Same high-level graph may let Vulkan choose:

- transient attachments,
- dynamic rendering,
- subpass merging,
- attachment load/store decisions,
- memory aliasing.

Again:

$$
\boxed{
\text{same lifetime information}
\rightarrow
\text{different backend optimization}.
}
$$

This is exactly native-strength multi-backend rendering.

---

**Load/store inference**

If resource first use is write and old contents irrelevant:

$$
LoadOp=
Clear
\text{ or }
DontCare.
$$

If last use ends at pass and resource is not exported/persistent:

$$
StoreOp=
DontCare.
$$

If consumed later:

$$
Store.
$$

Graph lifetime analysis can derive these decisions automatically.

This is a huge practical benefit.

---

**Example**

Resource:

```text
Depth
first use = Geometry write
last use = Geometry
transient = true
```

Metal:

```text
storage = Memoryless
load = Clear
store = DontCare.
```

Vulkan:

```text
transient depth image
loadOp = Clear
storeOp = DontCare.
```

Same graph intent.

Excellent native mapping.

---

**Another example: SceneHDR**

```text
first = Geometry
last = ToneMap
```

Cannot discard after geometry.

Need:

$$
Store
$$

after producer if native pass boundary requires system-memory persistence.

Graph explains why.

---

**Debugging alias corruption**

Suppose bloom texture unexpectedly shows shadow data.

Potential cause:

$$
AliasSlot(Bloom)
=
AliasSlot(Shadow)
$$

but lifetimes actually overlap.

Graph debugger should display:

```text
Shadow [1,4]
Bloom  [4,7]
```

These overlap at:

$$
4.
$$

Therefore strict rule:

$$
Last<First
$$

would reject aliasing.

If allocator used:

$$
Last\le First,
$$

that would be a bug.

---

**Async aliasing bug**

Even worse:

Topological order:

```text
A index 2
B index 3
```

but queues run concurrently.

Intervals based solely on index may appear non-overlapping despite simultaneous execution.

Therefore async graph must use synchronization DAG, not only index ranges.

This is exactly why full queue scheduling is a major render-graph evolution.

---

**Backend agreement should be automated**

For every regression scene:

Render Vulkan.

Render Metal.

Capture:

```text
beauty
depth
normals
IDs
```

Compare.

Store metrics.

CI fails if error exceeds threshold.

This is much stronger than visually inspecting occasional screenshots.

---

**Agreement test hierarchy**

The most useful order is:

First CPU mathematical oracle.

Then backend semantic debug outputs.

Then final image.

Why?

If final image differs, you need localization.

CPU reference tells whether one backend is fundamentally wrong.

Debug buffers tell where divergence appears.

Beauty comparison confirms end-to-end parity.

---

**Example PBR agreement**

Choose one surface:

$$
N=(0,1,0),
$$

$$
V,
L,
roughness,
metallic,
baseColor.
$$

CPU computes:

$$
D_{GGX},
F,
G,
f_r.
$$

Metal shader output debug mode writes BRDF value.

Vulkan shader writes same.

Compare both against CPU.

That tests formula consistency independent from full scene.

---

**Example projection agreement**

CPU computes expected clip:

$$
p_c=PVMp.
$$

Metal vertex debug captures transformed point.

Vulkan does same.

Compare.

This instantly finds matrix transpose/Y convention mistakes.

---

**Example skinning agreement**

CPU:

$$
p'=
\sum_i
w_i
M_i p.
$$

GPU Metal.

GPU Vulkan.

Check:

$$
\|p'_{GPU}-p'_{CPU}\|
<
\epsilon.
$$

This is exactly the sort of test your Kairo skinning smoke infrastructure can evolve toward.

---

**Final architecture proposal**

A clean eventual structure:

```text
KairoAssets / EngineCore
        ↓
RenderScene Extraction
        ↓
RendererRuntime
        ↓
RenderGraph Builder
        ↓
Compiled Render Plan
        ↓
Backend Translator
 ┌────────┼─────────┐
Vulkan   Metal    D3D12
```

Renderer resource manager sits alongside:

```text
MeshHandle
TextureHandle
PipelineHandle
```

and backends resolve them to native objects.

---

**What belongs above the backend**

Above:

```text
world/view transforms
material semantics
lights
camera
resource intent
pass dependencies
logical formats
resource lifetime
feature requirements
rendering equations
```

---

**What belongs below**

Below:

```text
native memory allocation
native image/buffer handles
descriptor/argument binding
pipeline objects
barriers
events/semaphores
render encoders
swapchain/drawable
native shader binaries
```

That division is the essence of M144.

---

**What must sometimes straddle boundary**

Things like:

```text
texture format support
ray tracing availability
max samples
native tooling integration
GPU profiler integration
```

need capability queries or escape hatches.

Do not force them fully above or below.

Provide explicit interface.

---

**Phase-24 reconstruction table**

| Concept | Core meaning |
|---|---|
| Neutral handle | Stable renderer-owned identity without native API type |
| Generational handle | Index + generation prevents stale reuse |
| Neutral buffer description | Size/usage/memory intent |
| Neutral texture description | Dimensions/format/usage/memory intent |
| Neutral sampler | Filtering/addressing intent |
| Neutral pipeline | Rendering semantics, not native pipeline object |
| Capability query | Ask actual device rather than assume from API |
| Feature contract | Named renderer functionality requirement |
| Escape hatch | Explicit localized access to native backend |
| Lowest-common denominator | Anti-pattern that discards native strengths |
| Render graph | DAG of rendering dependencies |
| Pass node | Unit of GPU work |
| Resource node | Logical data object |
| Read declaration | Consumer dependency |
| Write declaration | Producer/versioning event |
| RAW | writer must precede reader |
| WAR | reader must precede later writer |
| WAW | writer must precede later writer |
| Cycle | Impossible dependency schedule |
| Topological order | Legal linearization of DAG |
| Lifetime | First-use to last-use interval |
| Transient resource | Frame-local logical resource |
| Aliasing | Physical reuse across non-overlapping lifetimes |
| Barrier inference | Convert usage transition into native sync |
| State inference | Track semantic resource use |
| Cross-queue dependency | Dependency requiring semaphore/event-like sync |
| Pass culling | Dead-code elimination for unused GPU work |
| Imported resource | Native/persistent resource entering graph |
| History resource | Cross-frame persistent data |
| Present resource | Swapchain/drawable external resource |
| Graph visualization | Human-readable dependency structure |
| Resource naming | Shared engine/debugger identity |
| Capture correlation | Match graph pass to native GPU event |
| Conversion boundary | Neutral → backend-native representation |
| Portable mesh | API-independent geometry |
| Portable material | Semantic shading data |
| Scene extraction | Canonical renderer snapshot |
| Runtime bridge | Selects concrete backend |
| Private GPU handles | Native handles remain backend-local |
| Shader strategy | Native, translated or generated |
| Reflection | Extract shader ABI metadata |
| Layout generation | Build native bindings from shader metadata |
| Coordinate adaptation | Backend-specific clip/framebuffer transform |
| Sync adaptation | Same dependency, native synchronization |
| CPU reference | Mathematical correctness oracle |
| Backend agreement | Cross-API output parity |
| Native-strength abstraction | Common intent, specialized implementation |

---

**Blank-page mastery gate**

From scratch, design a renderer supporting Metal and Vulkan.

You are not allowed to put either:

```text
Vk*
```

or:

```text
MTL*
```

in scene code.

Define:

$$
MeshHandle,
TextureHandle,
BufferHandle.
$$

Explain why a generational handle is stronger than a plain integer.

Then design a neutral:

$$
BufferDesc,
TextureDesc,
SamplerDesc,
GraphicsPipelineDesc.
$$

For every field explain whether it expresses:

$$
\text{rendering intent}
$$

or accidentally leaks one API.

Then explain why:

```cpp
if (backend == Vulkan)
    enableFeature();
```

is inferior to:

```cpp
if (
    capabilities.Supports(
        Feature::Bindless))
```

because device support matters.

Then design one escape hatch for an ImGui Metal adapter and explain why ordinary gameplay code must never see it.

Then explain why:

$$
Capabilities=
Metal\cap Vulkan
$$

is a bad renderer architecture.

Now build a graph:

```text
Shadow
Geometry
TAA
Bloom
Present
```

with resources:

```text
ShadowDepth
SceneHDR
HistoryIn
HistoryOut
BloomHalf
Drawable
```

Write all read/write edges.

From those derive RAW/WAR/WAW dependencies.

Run Kahn's algorithm conceptually.

Compute first/last-use intervals.

Find which transient resources may alias.

Prove aliasing using:

$$
last(A)<first(B).
$$

Then explain why that proof becomes insufficient when A and B execute asynchronously on different queues.

Then infer transitions:

$$
Undefined
\rightarrow
DepthAttachment
$$

$$
DepthAttachment
\rightarrow
ShaderRead
$$

and:

$$
Undefined
\rightarrow
ColorAttachment
\rightarrow
ShaderRead.
$$

Explain why the graph should not emit `VkImageMemoryBarrier` itself.

Vulkan backend should do that.

Metal backend may rely on tracked hazards or emit Metal-native synchronization.

Then mark:

$$
HistoryIn
$$

persistent/imported.

Mark:

$$
Drawable
$$

external.

Explain why neither should be transiently aliased like a bloom scratch texture.

Then perform pass liveness.

Assume Bloom output is disabled.

Show why the Bloom pass disappears if no side effect or live output depends on it.

Then design DOT output.

Then propagate pass/resource names into:

- Vulkan debug labels,
- Metal labels,
- GPU captures.

Then implement one shadow pass twice.

Shared graph intent:

$$
\text{write shadow depth}.
$$

Vulkan:

$$
barriers+
depth pipeline+
rendering+
shader-read transition.
$$

Metal:

$$
private depth texture+
render encoder+
load/store+
tracked hazard.
$$

Explain why forcing Metal to expose Vulkan layouts would be an abstraction failure.

Then explain the coordinate boundary.

Canonical engine world:

$$
right\text{-}handed,
+Y\ up.
$$

Define a canonical clip convention.

Backend alone handles:

- clip Y,
- viewport orientation,
- depth convention,
- winding.

Never scatter fixes through shaders.

Then build CPU reference tests for:

$$
matrix transform,
skinning,
BRDF,
scan.
$$

Then build Metal↔Vulkan image agreement using:

$$
RMSE
$$

and semantic debug buffers.

Final requirement:

Explain this sentence precisely:

$$
\boxed{
\text{A good multi-backend renderer shares meaning, not implementation.}
}
$$

If you can reconstruct all of that, you have crossed from:

> person who can call Metal and Vulkan

into:

> person who can design an actual renderer architecture.


**Phase 24 is complete: M139–M144, Parts 2301–2400.**


That begins **Phase 25 — 3D Gaussian Splatting**, and this one maps directly into Maveb's research core: point-based versus triangle rendering, 1D/multivariate Gaussians, means/covariance and covariance ellipsoids, converting Gaussian scale+rotation into a 3D covariance, projecting 3D covariance through the camera Jacobian into screen-space covariance, conic inversion, eigenvalues/eigenvectors and ellipse bounds, spherical harmonics and view-dependent color, opacity/compositing, tile assignment and prefix sums, depth-key construction, GPU radix sorting, front-to-back splat compositing, numerical stability, gradients/differentiability, and then tracing the complete Maveb Gaussian pipeline against the underlying 3DGS math.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/43 - Compiling renderer intent into backend work|Previous chapter]] · [[Chapter Edition/03 - Game Engine/45 - Rasterizing kernels and connecting world identity|Next chapter]]
