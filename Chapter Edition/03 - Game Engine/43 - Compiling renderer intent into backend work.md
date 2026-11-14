# 43 — Compiling renderer intent into backend work

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/42 - From cartridge bytes to a reproducible machine|Previous chapter]] · [[Chapter Edition/03 - Game Engine/44 - Cross-backend conversion and agreement|Next chapter]]

A renderer should express what a frame needs before translating it into API-specific commands. This chapter makes the compilation boundary explicit so ordering, resource state, and backend execution can be checked separately.

**By the end:** Map a render request to backend work while preserving resource dependencies, barriers, and lifetime ownership.

**Recall question:** What logical dependency does this command represent, and where is it enforced in the selected backend?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 102536–105761; source-span SHA256 4825e8a42f5356f7c060e728c1c9a7e563f7ac3edfd1255c7f569076609bed16. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Compile renderer intent into backend work

### + M144

This pass closes **Phase 24**.

The curriculum boundary is exact: **M142 owns 2351–2360**, covering multi-backend motivation, neutral handles/resource descriptions, capability contracts, escape hatches and the failure of lowest-common-denominator abstractions; **M143 owns 2361–2384**, covering the complete render-graph dependency/lifetime/state model; and **M144 owns 2385–2400**, covering conversion boundaries, portable scene/material/mesh contracts, shader/layout strategy, Metal↔Vulkan adaptation, reference/backend agreement testing, and native-strength multi-backend design.

There are two ideas that dominate everything in this pass:

$$
\boxed{
\text{Share rendering intent, not native API mechanics}
}
$$

and

$$
\boxed{
\text{A render graph describes dependencies;
the backend chooses how those dependencies become GPU commands.}
}
$$

If you remember those two statements, the entire architecture reconstructs itself.

---

### MULTI-BACKEND RENDERER ARCHITECTURE

**Why have multiple rendering backends?**

Suppose your engine supports:

$$
Metal,\ Vulkan,\ D3D12,\ OpenGL.
$$

The naive reaction is:

> Write rendering four times.

That is not what we want.

The second naive reaction is:

> Build one fake API that looks vaguely like all four.

That is often even worse.

The correct goal is:

$$
\boxed{
\text{one engine rendering model}
}
$$

with:

$$
\boxed{
\text{multiple native execution implementations}.
}
$$

The engine should agree on things such as:

$$
\text{mesh},
$$

$$
\text{material},
$$

$$
\text{light},
$$

$$
\text{render pass intent},
$$

$$
\text{resource usage},
$$

$$
\text{camera}.
$$

It should not pretend these are universal:

```text
VkDescriptorSet
MTLArgumentBuffer
ID3D12DescriptorHeap
OpenGL texture unit
```

They are implementation mechanisms.

---

**The three-layer renderer**

A good architecture is:

```text
Engine / Scene
      ↓
Renderer-neutral intent
      ↓
Backend implementation
      ↓
Metal / Vulkan / D3D12 / ...
```

For example:

```cpp
MeshDraw draw;
draw.Mesh = meshHandle;
draw.Model = worldTransform;
draw.Material = material;
```

is renderer intent.

This:

```cpp
vkCmdBindDescriptorSets(...);
```

is Vulkan execution.

This:

```cpp
encoder->setFragmentTexture(...);
```

is Metal execution.

Do not mix the layers.

---

**Kairo is already moving in the right direction**

Kairo's public renderer API exposes concepts such as:

```cpp
Mesh
PBRMaterial
RenderScene
RenderLight
MeshHandle
TextureHandle
ViewportCapture
```

rather than exposing:

```text
VkBuffer
VkImage
MTLBuffer
MTLTexture
```

to ordinary scene code.

`RendererRuntime` then stores a variant of concrete backend runtimes:

```cpp
std::variant<
    std::unique_ptr<VulkanRendererRuntime>,
    std::unique_ptr<OpenGLRendererRuntime>,
    std::unique_ptr<MetalRendererRuntime>,
    std::unique_ptr<Direct3D12RendererRuntime>
>
```

depending on what is compiled.

That is already much closer to the correct architecture than trying to wrap native API calls one-for-one.

---

**Backend selection policy is separate from renderer semantics**

Kairo separates:

$$
\text{which backend should I use?}
$$

from:

$$
\text{how rendering works}.
$$

Current native-first policy is approximately:

```text
macOS   → Metal
Windows → D3D12
Linux   → Vulkan
```

with Vulkan/OpenGL fallbacks based on compiled availability.

Good.

This means application code can request:

```text
auto
metal
vulkan
d3d12
opengl
```

without changing scene extraction.

---

**backend-neutral resource handles**

Do not let gameplay code keep:

```cpp
VkImage image;
```

because now scene code knows Vulkan.

Instead:

```cpp
TextureHandle texture;
MeshHandle mesh;
BufferHandle buffer;
```

The renderer owns the native resource behind them.

Current Kairo:

```cpp
using TextureHandle =
    std::uint64_t;

using MeshHandle =
    std::uint64_t;
```

with:

$$
0=\text{invalid}.
$$

Simple and effective.

---

**Why handles beat native pointers**

Suppose backend changes from Vulkan to Metal.

An asset system storing:

```cpp
TextureHandle{17}
```

doesn't care.

Backend registry might map:

```text
17
→ VulkanTexture
```

or:

```text
17
→ MetalTexture
```

depending on active runtime.

The user of the handle sees identity.

The backend sees implementation.

---

**Raw integer handles have a stale-handle problem**

Suppose resource index 42 is destroyed.

Later a new resource receives 42.

Old code accidentally still stores:

$$
42.
$$

Now it refers to the new resource.

A stronger handle is generational:

```cpp
struct TextureHandle
{
    uint32_t Index;
    uint32_t Generation;
};
```

Registry entry:

```cpp
struct TextureEntry
{
    uint32_t Generation;
    NativeTexture Resource;
};
```

Validation:

$$
handle.Generation
=
entry.Generation.
$$

If not:

$$
\boxed{\text{stale handle detected}.}
$$

---

**Handle composition**

A compact 64-bit handle can encode:

$$
H=
(G<<32)|I.
$$

where:

$$
I=\text{index},
$$

$$
G=\text{generation}.
$$

Recover:

$$
I=H\&0xffffffff,
$$

$$
G=H>>32.
$$

Current Kairo's plain `uint64_t` can evolve into this without changing the external storage size.

---

**RenderGraph handles are already typed**

Kairo's render graph uses:

```cpp
struct RenderResourceHandle
{
    uint32_t Value;
};
```

and:

```cpp
struct RenderPassHandle
{
    uint32_t Value;
};
```

This prevents accidentally passing a pass handle where a resource handle is expected.

Maveb's render graph goes a little further:

```cpp
struct ResourceHandle
{
    uint32_t index;
    uint32_t generation;
};
```

which adds stale-handle protection.

The ideal general resource system combines both ideas:

$$
\boxed{
\text{typed + generational}.
}
$$

---

**backend-neutral buffer descriptions**

A generic buffer description should describe **intent**.

Not:

```cpp
VkBufferUsageFlags VulkanUsage;
```

Instead:

```cpp
enum class BufferUsage : uint32_t
{
    Vertex   = 1 << 0,
    Index    = 1 << 1,
    Uniform  = 1 << 2,
    Storage  = 1 << 3,
    Indirect = 1 << 4,
    CopySrc  = 1 << 5,
    CopyDst  = 1 << 6
};
```

Then:

```cpp
struct BufferDesc
{
    uint64_t Size;
    BufferUsage Usage;

    MemoryClass Memory;

    std::string DebugName;
};
```

---

**Memory class should also express intent**

Do not expose:

```text
VK_MEMORY_PROPERTY_HOST_VISIBLE
```

to portable engine code.

Use something like:

```cpp
enum class MemoryClass
{
    GPUOnly,
    CPUToGPU,
    GPUToCPU
};
```

Backend translates:

`GPUOnly`:

Vulkan:

$$
DEVICE_LOCAL
$$

Metal:

$$
Private.
$$

`CPUToGPU`:

Vulkan:

$$
HOST_VISIBLE
$$

plus suitable coherency strategy.

Metal:

$$
Shared.
$$

That is semantic translation.

---

**Avoid overpromising exact equivalence**

`GPUOnly` is not:

> exactly `Private` and exactly `DEVICE_LOCAL`.

Those APIs expose different memory models.

It means:

$$
\boxed{
\text{CPU does not require direct access and GPU performance is preferred}.
}
$$

Backend then chooses best representation.

---

**backend-neutral texture descriptions**

A useful texture description:

```cpp
struct TextureDesc
{
    TextureDimension Dimension;

    uint32_t Width;
    uint32_t Height;
    uint32_t Depth;

    uint32_t MipLevels;
    uint32_t ArrayLayers;
    uint32_t Samples;

    PixelFormat Format;
    TextureUsage Usage;

    MemoryClass Memory;

    std::string DebugName;
};
```

Again, semantic fields.

---

**Texture usage**

Portable usage might include:

```text
Sampled
Storage
ColorAttachment
DepthAttachment
CopySrc
CopyDst
Presentable
```

Backend conversion:

$$
Sampled
\rightarrow
VK_IMAGE_USAGE_SAMPLED
$$

or:

$$
MTLTextureUsageShaderRead.
$$

Same intent.

Different API flags.

---

**Format abstraction needs discipline**

You can define:

```text
RGBA8_UNorm
RGBA8_sRGB
RGBA16_Float
R32_UInt
D32_Float
```

But never invent a portable format unless you know how all target backends support it.

Backend capability query should answer:

$$
SupportsFormat(
format,
usage).
$$

For example, a format might support shader sampling but not rendering on one device.

---

**sampler descriptions**

Portable sampler:

```cpp
struct SamplerDesc
{
    Filter Min;
    Filter Mag;
    Filter Mip;

    AddressMode U;
    AddressMode V;
    AddressMode W;

    float MaxAnisotropy;

    CompareFunction Comparison;
};
```

Maps naturally to both Vulkan and Metal.

Kairo currently exposes only authored U/V addressing in its public `TextureSampler`, while filtering remains linear by policy.

That is perfectly reasonable for its current asset/material scope.

A generic RHI sampler description would be broader.

---

**backend-neutral pipeline descriptions**

This becomes trickier.

A neutral graphics pipeline might describe:

```cpp
struct GraphicsPipelineDesc
{
    ShaderHandle Vertex;
    ShaderHandle Fragment;

    VertexLayout VertexInput;

    PrimitiveTopology Topology;

    RasterState Raster;
    DepthStencilState DepthStencil;
    BlendState Blend;

    std::vector<PixelFormat>
        ColorFormats;

    PixelFormat DepthFormat;

    uint32_t SampleCount;
};
```

This works because these are rendering semantics.

---

**Do not try to normalize everything**

Metal can dynamically change states that Vulkan may traditionally bake into a pipeline.

Vulkan may support dynamic rendering.

Metal has tile-specific capabilities.

D3D12 has root signatures.

So pipeline description should describe:

$$
\boxed{\text{what the engine wants}}
$$

without pretending the compiled backend pipeline object has identical structure.

---

**Pipeline cache key**

A pipeline descriptor naturally becomes a cache key:

$$
K=
hash(
shaders,
formats,
vertexLayout,
raster,
depth,
blend,
samples
).
$$

Backend:

```text
K → native pipeline object
```

This avoids recompiling equivalent states.

---

**capability queries, not assumptions**

Never do:

```cpp
if (backend == Metal)
    assumeRayTracing = true;
```

Instead:

```cpp
RendererCapabilities caps =
    renderer.Capabilities();
```

perhaps:

```cpp
struct RendererCapabilities
{
    bool RayTracing;
    bool MeshShaders;
    bool BindlessTextures;
    bool AsyncCompute;
    bool MemorylessAttachments;
    bool TimelineSynchronization;

    uint32_t MaxColorAttachments;
    uint32_t MaxTextureDimension2D;
};
```

---

**Capability is device-level, not merely API-level**

Vulkan supports ray tracing extensions.

That does not imply every Vulkan GPU supports them.

Metal exposes ray tracing capabilities.

That does not imply every Metal device supports them.

Therefore:

$$
\boxed{
FeatureAvailable
=
BackendSupports
\land
DeviceSupports.
}
$$

---

**Maveb already follows this pattern**

Its Metal capability inspection asks the actual device:

$$
supportsFamily(...)
$$

$$
supportsRaytracing()
$$

$$
supportsDynamicLibraries().
$$

That is exactly the correct philosophy.

Kairo's current neutral backend query is more focused on:

$$
\text{compiled backend availability}
$$

than detailed GPU feature capabilities.

A future neutral capability object would complement it.

---

**feature-level contracts**

A single boolean such as:

```cpp
SupportsRenderer = true;
```

is too vague.

Define feature contracts.

Example:

```text
CoreRaster
    sampled textures
    depth
    MRT
    indexed draws

AdvancedRaster
    indirect draws
    storage buffers

Bindless
    large resource arrays
    nonuniform indexing

AsyncCompute
    independent compute scheduling

RayTracing
    acceleration structures
    ray queries/pipelines
```

Then renderer paths can require:

$$
FeatureLevel\ge X
$$

or test individual contracts.

---

**Graceful feature selection**

Suppose clustered forward renderer needs compute + storage buffers.

If unavailable:

$$
\boxed{
\text{fall back to CPU light lists}
}
$$

rather than:

> Vulkan failed.

Similarly:

Ray-traced reflections unavailable:

$$
SSR\rightarrow probe\ reflection.
$$

Feature contracts allow real fallback design.

---

**backend-specific escape hatches**

A portable renderer eventually needs native integration.

Examples:

- Dear ImGui Metal backend needs `MTLDevice`,
- Vulkan tooling needs render pass/device/queue,
- platform capture may need native texture,
- video encoder may consume Metal texture directly.

The answer is not:

> expose native handles everywhere.

The answer is:

$$
\boxed{
\text{explicit, narrow escape hatch}.
}
$$

---

**Kairo already does this well**

Ordinary rendering uses neutral API.

Tooling-specific accessors expose:

```text
VulkanBackendContext
MetalBackendContext
Direct3D12BackendContext
```

and viewport textures.

Metal viewport identity is exposed as erased:

```cpp
void* Texture;
```

with documentation that clients must treat it as borrowed and not retain/release it.

This is exactly what an escape hatch should look like:

$$
\boxed{
\text{localized + explicit + non-portable by construction}.
}
$$

---

**Escape hatch should advertise danger**

For example:

```cpp
NativeMetalContext&
GetNativeMetalContext();
```

should exist only in platform/tooling code.

Not:

```cpp
renderer.GetNativeHandle()
```

called throughout gameplay.

One possible policy:

```cpp
if (renderer.Backend()
    != GraphicsBackend::Metal)
{
    throw ...
}
```

so misuse fails immediately.

---

**why lowest-common-denominator abstractions fail**

Suppose your neutral API only supports what OpenGL 3, Vulkan, Metal and D3D12 all expose identically.

You may end up with:

```text
draw
texture
vertex buffer
uniform
```

and lose:

```text
argument buffers
descriptor indexing
mesh shaders
ray tracing
memoryless attachments
timeline synchronization
explicit heaps
async compute
subpass/tile features
```

Congratulations—you built a modern engine that behaves like an old API.

---

**Correct abstraction principle**

Do not define:

$$
\boxed{
Capabilities=
intersection(
Metal,
Vulkan,
D3D12,
OpenGL).
}
$$

Instead:

$$
\boxed{
CoreIntent
+
CapabilityQueries
+
OptionalFeatureInterfaces
+
NativeEscapeHatches.
}
$$

That lets common code remain common while backends remain strong.

---

**Portable does not mean identical**

A good abstraction says:

> Render this HDR scene with depth and shadowing.

A bad abstraction says:

> Every backend must implement a pretend `PipelineBarrier()` with exactly Vulkan semantics.

Metal does not need to become Vulkan.

Vulkan does not need to become Metal.

---

### COMPLETE RENDER GRAPH

Now we move to one of the most important engine systems in the entire curriculum.

---

**render-graph motivation**

A traditional renderer manually says:

```text
Create ShadowTexture

ShadowPass()
Barrier()

GeometryPass()
Barrier()

TAA()
Barrier()

Bloom()
Barrier()

Present()
```

Problems emerge as passes increase.

Which pass owns resource?

Who last wrote it?

Is it still needed?

Can two textures reuse memory?

Did we forget a barrier?

Can this pass be culled?

Can compute overlap graphics?

A render graph makes those relationships explicit.

---

**Render graph as a mathematical graph**

Define:

$$
G=(V,E)
$$

where vertices:

$$
V=
\text{render passes}.
$$

Directed edge:

$$
A\rightarrow B
$$

means:

$$
\boxed{
B\text{ depends on }A.
}
$$

For valid execution:

$$
G
$$

should be a DAG:

$$
\boxed{
\text{Directed Acyclic Graph}.
}
$$

Then topological sorting yields legal execution order.

---

**Resource graph interpretation**

You can also think bipartitely:

```text
Pass A
   ↓ writes
Resource X
   ↓ read by
Pass B
```

The engine usually compiles resource accesses into pass-to-pass dependency edges.

---

**pass nodes**

A pass should declare intent.

Example:

```cpp
graph.AddPass(
    "Lighting",
    {
        Read(gbufferNormal,
             ShaderRead),

        Read(gbufferDepth,
             ShaderRead),

        Write(hdrColor,
              ColorAttachment)
    },
    [] {
        EncodeLighting();
    });
```

Pass declaration should separate:

$$
\boxed{
\text{dependency description}
}
$$

from:

$$
\boxed{
\text{execution callback}.
}
$$

That is exactly what current Kairo graph does.

---

**Current Kairo pass object**

Each pass stores:

```cpp
Name
Accesses
Dependencies
Execute
```

and every access stores:

```cpp
Resource
Mode
State
```

with mode:

$$
Read,
Write,
ReadWrite.
$$

This is already a solid minimal model.

---

**resource nodes**

Resource node represents logical resource, not necessarily immediately allocated native memory.

Example:

```cpp
RenderResourceDesc
{
    .Name =
        "Scene HDR",

    .Kind =
        Texture,

    .EstimatedBytes =
        1920*1080*8,

    .Transient =
        true,

    .InitialState =
        Undefined
};
```

Logical identity exists before physical allocation decision.

That is critical.

---

**Logical versus physical resource**

Logical:

```text
BloomHalf
```

means:

> pass graph's half-resolution bloom intermediate.

Physical resource may be:

```text
VkImage #41
```

or:

```text
MTLTexture from transient heap slot 3.
```

Logical resource survives backend abstraction.

Physical object does not.

---

**declaring reads**

Suppose pass reads texture:

$$
R.
$$

Declaration:

```cpp
{
    R,
    RenderAccessMode::Read,
    RenderResourceState::ShaderRead
}
```

This communicates:

$$
\boxed{
\text{pass requires value produced earlier}.
}
$$

Compiler can now establish RAW dependency.

---

**Read-before-write validation**

For transient resource whose initial state is:

$$
Undefined,
$$

reading before first write is invalid.

Kairo explicitly detects this and throws.

This is powerful because a runtime black-screen bug becomes a graph compile error:

```text
Resource 'History' is read before its first write.
```

---

**writes**

Write declaration:

```cpp
{
    hdr,
    Write,
    ColorAttachment
}
```

means pass produces a new value.

Writes matter because all prior accesses may have to finish.

If previous writer exists:

$$
WAW.
$$

If readers exist:

$$
WAR.
$$

Both create ordering requirements.

---

**Hazard dependency construction**

For every resource track:

$$
lastWriter
$$

and:

$$
activeReaders.
$$

On read:

$$
lastWriter
\rightarrow
currentRead.
$$

On write:

$$
lastWriter
\rightarrow
currentWrite
$$

and:

$$
reader_i
\rightarrow
currentWrite
\quad\forall i.
$$

Then:

$$
activeReaders=\emptyset,
$$

$$
lastWriter=currentWrite.
$$

Current Kairo does exactly this.

---

**Why two readers need no edge**

If:

```text
Pass A reads X
Pass B reads X
```

neither mutates X.

Therefore:

$$
A
\not\rightarrow
B
$$

and:

$$
B
\not\rightarrow
A.
$$

They may be reordered or potentially overlap.

This is how graphs expose parallelism.

---

**dependency building**

Consider:

```text
Shadow writes ShadowMap

Geometry reads ShadowMap
         writes HDR

Bloom reads HDR
      writes Bloom

Present reads HDR and Bloom
```

Edges:

$$
Shadow\rightarrow Geometry
$$

$$
Geometry\rightarrow Bloom
$$

$$
Geometry\rightarrow Present
$$

$$
Bloom\rightarrow Present.
$$

The graph does not need hardcoded pass order.

Dependencies derive order.

---

**Explicit dependencies still matter**

Some dependencies do not arise from resource hazards.

Example:

```text
GPU timestamp reset
must happen before
profiling pass
```

or API lifecycle:

```text
Acquire
→
Record
→
Submit
→
Present.
```

Kairo provides:

```cpp
DependsOn(pass,
          dependency);
```

for these.

Its Vulkan frame graph currently uses explicit dependencies for lifecycle ordering.

---

**cycle detection**

Invalid:

$$
A\rightarrow B
$$

and:

$$
B\rightarrow A.
$$

No valid execution order exists.

Kairo uses Kahn-style topological sorting.

If sorted pass count:

$$
|O|
$$

is not:

$$
|V|,
$$

there is a cycle.

Formally:

$$
\boxed{
|TopologicalOrder|<|V|
\Rightarrow
cycle.
}
$$

---

**Kahn algorithm**

Compute indegree:

$$
deg^{-}(v).
$$

Put every zero-indegree node in ready set.

Repeatedly:

$$
v=popReady()
$$

append to order.

For every outgoing edge:

$$
v\rightarrow u
$$

decrement:

$$
deg^{-}(u).
$$

If it becomes zero:

$$
push(u).
$$

At end compare size.

---

**Kairo makes topological order deterministic**

It uses a priority queue ordered by pass index.

Therefore if multiple passes are valid simultaneously, lower declared index wins.

That is useful for:

- reproducible testing,
- deterministic captures,
- stable debugging.

Random legal schedules are technically correct but terrible for reproducibility.

---

**topological sorting**

Graph:

```text
      A
     / \
    B   C
     \ /
      D
```

Valid orders:

```text
A B C D
A C B D
```

Both correct.

Topological order represents one linearization of a partial order.

This distinction becomes important for async queues later.

---

**execution ordering**

Compiled graph should be immutable during execution.

Current Kairo compiles:

```text
resource descriptions
ordered passes
transitions
lifetimes
alias slots
```

then `Execute()` walks passes in order.

Before each callback:

```cpp
ApplyTransitions(...)
```

may translate graph state transitions into native backend barriers.

This is an excellent separation.

---

**Compilation versus execution**

Compile phase performs expensive reasoning:

$$
dependencies,
cycles,
lifetimes,
aliasing,
transitions.
$$

Execution phase should be simple:

```text
for pass in compiledOrder:
    apply inferred native state changes
    execute pass callback
```

This minimizes per-frame logic.

---

**Static versus dynamic graph**

Some engines rebuild graph every frame.

Others compile stable topology and only update runtime values.

If topology is stable:

$$
\boxed{
\text{cache compiled graph}.
}
$$

If feature toggles alter pass set, rebuild when configuration changes.

The right balance depends on renderer architecture.

---

**resource lifetime intervals**

If resource first appears at pass:

$$
f
$$

and last appears at:

$$
l,
$$

its logical lifetime is:

$$
\boxed{
[f,l].
}
$$

Example:

```text
Pass 0: write A
Pass 1: read A
Pass 2: read A
```

Then:

$$
A=[0,2].
$$

---

**first and last use**

Kairo scans compiled execution order.

For first encounter:

$$
FirstPass=position.
$$

Every encounter updates:

$$
LastPass=position.
$$

That gives interval.

This is exactly the information required for transient aliasing.

---

**Why declaration order is not lifetime order**

Imagine pass declarations:

```text
A
B
C
```

but dependency sort gives:

```text
B
A
C.
```

Lifetime must use **compiled order**.

Kairo correctly computes lifetimes after topological sorting.

---

**transient-resource allocation**

Transient resource exists only for frame/pass graph.

Examples:

```text
Bloom temporary
G-buffer intermediate
SSR ray buffer
post-process scratch
temporary compute histogram
```

You do not necessarily need one physical allocation for every logical transient.

---

**Naive memory**

Suppose:

$$
R_1=16MB,
R_2=16MB,
R_3=16MB.
$$

Naive:

$$
M=48MB.
$$

But if their lifetimes do not overlap:

$$
R_1:[0,1]
$$

$$
R_2:[2,3]
$$

$$
R_3:[4,5]
$$

then physical memory can potentially be:

$$
\boxed{16MB}
$$

instead.

---

**aliasing**

Two logical resources may share backing storage if lifetimes don't overlap.

For intervals:

$$
A=[a_0,a_1],
$$

$$
B=[b_0,b_1],
$$

non-overlap iff:

$$
\boxed{
a_1<b_0
\lor
b_1<a_0.
}
$$

Notice strict inequality.

If:

$$
a_1=b_0,
$$

both are used during same pass index.

They cannot blindly alias.

---

**Kairo's alias algorithm**

For each transient sorted by first use, find existing slot where:

$$
slot.Kind=resource.Kind,
$$

$$
slot.Capacity\ge resource.Bytes,
$$

$$
slot.LastPass<resource.FirstPass.
$$

If found, reuse.

Otherwise create new slot.

This is a greedy interval-coloring-like allocator.

---

**Current Kairo scale test is good**

It constructs:

$$
256
$$

transient textures in a chain.

Each overlaps only immediate neighbor.

Expected physical slots:

$$
\le2.
$$

The test asserts transient allocation:

$$
\le
2\times4096
$$

rather than:

$$
256\times4096.
$$

Excellent test of lifetime-based reuse.

---

**But current alias compatibility is intentionally incomplete**

Today Kairo checks only:

$$
Kind
$$

and:

$$
EstimatedBytes.
$$

That is not sufficient for production native aliasing.

Two textures might differ in:

- format,
- dimensions,
- sample count,
- usage,
- alignment,
- memory class,
- tiling requirements.

Vulkan memory requirements may differ.

Metal heap resource-size/alignment requirements may differ.

Therefore real alias compatibility must include:

$$
\boxed{
\text{native-compatible memory class and alignment requirements}.
}
$$

---

**Better alias descriptor**

Conceptually:

```cpp
struct AliasClass
{
    ResourceKind Kind;

    MemoryClass Memory;

    uint64_t Size;
    uint64_t Alignment;

    uint32_t SampleCount;

    FormatCompatibilityClass FormatClass;

    ResourceUsage Usage;
};
```

Then backend confirms compatibility.

Never alias merely because byte counts match.

---

**Aliasing is graph coloring intuition**

Each resource interval is a vertex.

If two intervals overlap, connect them.

Resources connected by conflict edge cannot share physical slot.

Then physical allocation resembles graph coloring.

For interval graphs this can be solved efficiently.

Greedy sorted-by-start works well.

---

**barrier inference**

Once graph knows previous state and next state:

$$
S_{before}
\rightarrow
S_{after},
$$

it can ask backend for required synchronization.

Kairo stores:

```cpp
RenderResourceTransition
{
    Resource;
    Before;
    After;
}
```

and creates one whenever requested state changes.

---

**Example transition**

Resource begins:

$$
Undefined.
$$

Geometry:

$$
ColorAttachment.
$$

Post:

$$
ShaderRead.
$$

So compiler produces:

$$
Undefined
\rightarrow
ColorAttachment
$$

then:

$$
ColorAttachment
\rightarrow
ShaderRead.
$$

Vulkan backend can map these to image barriers.

Metal tracked resources may need little/no explicit action.

Same graph.

Different implementation.

---

**State transition alone is not always enough**

Current Kairo state enum:

```text
Undefined
ColorAttachment
DepthAttachment
ShaderRead
CopySource
CopyDestination
Present
```

is a good minimal set.

But future GPU-driven renderer needs distinctions such as:

```text
ShaderStorageRead
ShaderStorageWrite
IndirectArgument
VertexBuffer
IndexBuffer
UniformRead
AccelerationStructureRead
RayTracingWrite
```

because Vulkan synchronization needs exact consumer/producer access scopes.

So as backend sophistication grows:

$$
\boxed{
\text{logical state model must grow with actual hazard semantics}.
}
$$

---

**Barrier derivation example**

Graph says:

$$
CopyDestination
\rightarrow
ShaderRead.
$$

Vulkan mapping:

$$
TRANSFER
+
TRANSFER_WRITE
$$

to:

$$
FRAGMENT/COMPUTE
+
SHADER_READ.
$$

Metal:

tracked resource:

$$
\text{encoder ordering may suffice}.
$$

Untracked resource:

$$
\text{appropriate barrier/fence}.
$$

---

**layout/state transition inference**

Vulkan images have layouts.

Metal does not expose equivalent image layout transitions to engine in same fashion.

Therefore neutral graph should express:

$$
\text{semantic state},
$$

not:

```text
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
```

Vulkan converts:

$$
ShaderRead
\rightarrow
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
$$

Metal interprets:

$$
ShaderRead
$$

as binding/hazard intent without fake layouts.

---

**State is broader than Vulkan layout**

A resource state often means:

$$
\boxed{
\text{how resource is being used}
}
$$

which may imply:

- pipeline stage,
- memory access,
- layout,
- ownership.

So graph state should be semantic.

Backend derives native requirements.

---

**cross-queue dependencies**

Suppose graphics produces depth.

Compute queue performs SSAO.

Graphics queue then consumes AO.

Logical:

$$
GraphicsDepth
\rightarrow
ComputeSSAO
\rightarrow
GraphicsLighting.
$$

A topological order alone does not synchronize two independent hardware queues.

You need cross-queue signaling.

---

**Vulkan cross-queue mapping**

Could be:

```text
Graphics submission
    signals timeline = 15

Compute submission
    waits timeline >= 15
    executes SSAO
    signals timeline = 16

Graphics lighting
    waits timeline >= 16
```

If exclusive queue families differ, resource ownership transfer may also be necessary.

---

**Metal cross-queue mapping**

Could use:

$$
MTLSharedEvent.
$$

Graphics signals:

$$
15.
$$

Compute waits:

$$
15.
$$

Compute signals:

$$
16.
$$

Graphics waits:

$$
16.
$$

Same graph dependency.

Native synchronization differs.

---

**Queue assignment must become part of pass metadata**

For example:

```cpp
enum class QueueClass
{
    Graphics,
    Compute,
    Transfer
};
```

Pass:

```cpp
PassDesc
{
    QueueClass::Compute,
    ...
};
```

Compiler can then identify:

$$
\boxed{
dependency crosses queue boundary.
}
$$

---

**Async queue scheduling changes lifetime analysis**

This is subtle and important.

Current Kairo computes lifetime using a single linear pass order.

That works when execution is serial.

But with async compute:

```text
Graphics A ────────┐
                  │
Compute B ─────────────
```

two passes can overlap in real time despite having some topological ordering.

Then:

$$
LastPass<FirstPass
$$

in one linear ordering does **not necessarily prove** physical non-overlap.

For cross-queue aliasing you need a happens-before analysis.

This is a major step beyond simple render graphs.

---

**Safe async aliasing rule**

Two resources may alias only if graph proves:

$$
end(A)
\prec
begin(B)
$$

in the dependency partial order.

Not merely:

$$
index(A)<index(B).
$$

If neither is ordered relative to the other:

$$
A\parallel B
$$

and they may overlap.

Therefore they cannot safely share memory.

---

**pass culling**

Suppose graph contains:

```text
DebugNormals
```

but debug output is not shown.

If no later live pass consumes its output and it has no side effect:

$$
\boxed{
\text{remove pass entirely}.
}
$$

This reduces GPU work.

---

**Reverse liveness analysis**

Start from roots:

$$
\text{presented outputs}
$$

$$
\text{explicit exports}
$$

$$
\text{side-effect passes}.
$$

Mark them live.

Then recursively mark their dependencies live.

Everything else:

$$
\boxed{\text{dead pass}.}
$$

---

**Maveb render graph already performs pass culling**

Its graph marks passes required if:

$$
sideEffect=true
$$

or they write a resource marked:

$$
output=true.
$$

Then walks dependencies backward.

Only live passes participate in topological scheduling.

That is a real pass-culling implementation.

---

**Kairo currently does not cull**

Current Kairo compiles all declared passes.

So compared with Maveb:

Kairo has stronger:

$$
state\ transitions,
alias\ slots,
transition\ hooks.
$$

Maveb has:

$$
pass\ culling,
DOT\ graph\ export,
generational\ handles.
$$

A future unified implementation can combine the strengths.

---

**imported external resources**

Some resources were created outside graph.

Examples:

```text
swapchain image
CAMetalDrawable texture
persistent shadow atlas
engine-owned history
video texture
editor viewport
```

Graph should import rather than own.

---

**Imported resource needs initial state**

The graph must know how external resource enters graph.

For example:

$$
SwapchainInitial=Present.
$$

or:

$$
HistoryInitial=ShaderRead.
$$

Otherwise first transition cannot be inferred.

Current Kairo models external resource via:

```text
RenderResourceKind::External
```

and requires external resources to be non-transient.

Good.

---

**Current Vulkan graph example**

It creates:

```text
VulkanNativeFrame
```

as:

$$
External
$$

with:

$$
InitialState=Present.
$$

Then logical stages:

```text
Retire
Acquire
Record
Submit
Present
```

use this token/resource.

This current graph is more of a **frame-lifecycle dependency object** than a complete per-image render graph, but the abstraction principle is sound.

---

**persistent history resources**

TAA history:

$$
H_{t-1}
$$

must survive to next frame.

Therefore it cannot be transiently aliased as soon as current frame ends.

Graph resource should be:

$$
Transient=false.
$$

Initial state:

$$
ShaderRead
$$

if valid.

Current Kairo tests exactly this notion: a persistent resource initialized as `ShaderRead` may be read immediately, while persistent `Undefined` read is rejected.

---

**History invalidation**

Persistent doesn't mean always valid.

Invalidate history on:

- camera cut,
- resolution change,
- projection change,
- scene discontinuity,
- major geometry repair,
- renderer mode switch.

Then either:

$$
HistoryValid=false
$$

or graph inserts clear/initialize pass.

Maveb's TAA system already carries explicit history validity state.

---

**Ping-pong history**

Typical:

$$
H_0,H_1.
$$

Frame $t$:

read:

$$
H_{t\bmod2}
$$

write:

$$
H_{(t+1)\bmod2}.
$$

Next frame swap roles.

You must ensure frames in flight do not cause simultaneous reuse hazards.

With multiple frames in flight, history scheduling becomes more complex than simple CPU frame parity unless GPU execution order guarantees it.

---

**swapchain/drawable resources in graph**

Presentation resource is special.

Vulkan:

$$
SwapchainImage.
$$

Metal:

$$
CAMetalDrawable.texture.
$$

These are acquired externally.

They should appear near graph boundary.

Conceptually:

```text
Import Presentable

ToneMap:
    read HDR
    write Presentable

Present:
    consume Presentable
```

---

**Acquire as graph boundary**

For Vulkan:

$$
Acquire
$$

produces access to presentation image.

For Metal:

$$
nextDrawable/currentDrawable
$$

provides drawable texture.

Graph may represent acquisition as external lifecycle operation rather than ordinary resource pass.

Either way:

$$
\boxed{
\text{presentation image lifetime begins late and ends at presentation}.
}
$$

---

**Do not build whole renderer around swapchain**

Internal HDR/depth/temporal resources should be independent.

Swapchain/drawable is only final destination.

Then resize/presentation format changes don't infect every pass.

---

**debugging dependency mistakes**

Common graph failures:

Read before write.

Missing write declaration.

Incorrect state.

Missing side-effect root causing pass culling.

Cycle.

Aliasing resources whose lifetimes actually overlap.

Cross-frame history improperly marked transient.

Queue dependency omitted.

---

**Debugging rule**

When image is wrong, do not immediately inspect shader.

First ask:

$$
\boxed{
\text{Was correct producer executed?}
}
$$

$$
\boxed{
\text{Was resource initialized?}
}
$$

$$
\boxed{
\text{Was consumer ordered after producer?}
}
$$

$$
\boxed{
\text{Was native synchronization emitted?}
}
$$

$$
\boxed{
\text{Was physical allocation reused too early?}
}
$$

Render graphs shift many “graphics bugs” into dependency bugs.

---

**visualizing graph**

Maveb emits DOT:

```text
digraph AetherRenderGraph {
  p0 [label="Shadow"];
  p1 [label="Geometry"];
  p2 [label="TAA"];
  p3 [label="Present"];

  p0 -> p1;
  p1 -> p2;
  p2 -> p3;
}
```

Then Graphviz can visualize it.

This is extremely valuable.

---

**Better graph visualization**

An engine UI should show:

```text
Pass name
Queue
Execution order
CPU/GPU timing

Reads
Writes

Resource state before/after

First use
Last use

Alias slot

Culled/live
```

Then resource:

```text
SceneHDR
  16.6 MB
  first: Geometry
  last: ToneMap
  alias slot: 3
```

You can debug memory and ordering visually.

---

**resource naming**

Never accept:

```text
Texture_27
```

if semantic name is known.

Use:

```text
Scene HDR
Scene Depth
TAA History A
Bloom Half
Gaussian IDs
Directional Shadow Cascades
```

Kairo requires unique graph resource names.

Maveb labels native resources extensively.

These should be connected.

---

**Name propagation**

Ideal:

```text
RenderGraph resource:
    Scene HDR

↓ Vulkan backend
VkImage debug name:
    Scene HDR

↓ Metal backend
MTLTexture label:
    Scene HDR

↓ capture/profiler
same label
```

Now the engine, graph and GPU debugger all speak one vocabulary.

---

**GPU capture correlation**

Suppose graph says:

```text
Pass 17: Temporal Resolve
```

GPU capture should show encoder/debug region:

```text
Temporal Resolve
```

If graph timing says pass 17 is slow, you can immediately find it in Xcode/RenderDoc.

This is why names are infrastructure, not decoration.

---

**Current Kairo profile warning**

Kairo's graph profile currently measures CPU callback/recording duration.

Not GPU duration.

So:

```text
Geometry: 0.35 ms
```

means CPU graph callback time.

A future GPU profiler should correlate:

$$
GraphPassID
$$

with:

$$
GPU timestamp range.
$$

Then you can compare:

```text
CPU encode: 0.18 ms
GPU execute: 2.7 ms
```

for the same pass.

---

**A production render-graph data model**

A stronger version could look like:

```cpp
struct PassDesc
{
    std::string Name;

    QueueClass Queue;

    std::vector<ResourceAccess>
        Accesses;

    bool HasSideEffects = false;

    ExecuteCallback Execute;
};

struct ResourceAccess
{
    ResourceHandle Resource;

    AccessMode Access;

    ResourceUsage Usage;

    ShaderStageMask Stages;
};
```

Notice separate:

$$
Access
$$

and:

$$
Usage/Stages.
$$

That gives enough information for Vulkan synchronization.

---

**Better state representation**

Instead of only:

```text
ShaderRead
```

you might represent:

```cpp
struct ResourceState
{
    PipelineDomain Domain;
    AccessFlags Access;
    UsageClass Usage;
};
```

Example:

```text
ComputeShader
StorageWrite
StorageBuffer
```

versus:

```text
FragmentShader
SampledRead
SampledTexture
```

These are both “shader use” but require different native synchronization.

---

**Compiler stages**

A production render graph can conceptually compile through:

```text
validate declarations
↓
construct hazards/dependencies
↓
mark live passes
↓
detect cycles
↓
assign queues
↓
schedule passes
↓
compute lifetimes
↓
plan transient allocations
↓
infer state transitions
↓
infer inter-queue synchronization
↓
generate backend execution plan
```

That is essentially a compiler.

Because it is one.

---

**Render graph as compiler IR**

This is the deepest useful perspective.

Scene rendering code produces high-level IR:

```text
Shadow writes S
Geometry reads S writes H
Bloom reads H writes B
Present reads H,B
```

Compiler transforms into:

```text
ordered passes
physical resources
barriers
queue waits/signals
load/store decisions
```

Vulkan/Metal backends are code generators.

That is why compiler theory ideas such as:

- dependency graphs,
- liveness,
- alias analysis,
- scheduling

appear here.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/42 - From cartridge bytes to a reproducible machine|Previous chapter]] · [[Chapter Edition/03 - Game Engine/44 - Cross-backend conversion and agreement|Next chapter]]
