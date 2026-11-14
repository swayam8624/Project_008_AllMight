# 15 — Execute and Measure a Metal Frame

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/16 - Profile GPUs and Respect Apple Ownership Boundaries|Next chapter]]

Metal uses its own command, storage, presentation, and synchronization contracts, with Apple GPU tile behavior affecting practical choices. This chapter treats a Metal frame as an execution pipeline whose correctness and performance need separate evidence.

**By the end:** Map Metal device, command, storage, drawable, and synchronization concepts onto a measured frame without importing Vulkan assumptions blindly.

**Recall question:** Which measurements distinguish CPU command encoding cost from GPU execution and presentation delay?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 34647–37688; source-span SHA256 71e4bac8a630df75ca628f0e92faaadf94348d3efbc86b1f781ed0c597ace587. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Execute and measure a Metal frame

### + M141

Pass 13 begins **Phase 24 — Metal, Render Graphs, Resource Lifetimes, Apple GPU Concerns, and Multi-Backend Rendering**. The exact boundary is important: M139 owns **2301–2316**, Metal's core execution/resource/shader model; M140 owns **2317–2340**, storage modes, tile-memory implications, drawable/presentation, frames in flight, heaps and synchronization; and M141 owns **2341–2350**, profiling/debugging plus Objective-C++/Swift ownership and threading boundaries. Render-graph construction itself begins later at Part 2361, so that belongs to Pass 14.

This phase is especially useful for you because we now have three concrete implementation levels to compare:

$$
\boxed{
\text{Kairo Metal}
\rightarrow
\text{small explicit Metal backend}
}
$$

$$
\boxed{
\text{Maveb Metal}
\rightarrow
\text{multi-pass, triple-buffered production-style renderer}
}
$$

$$
\boxed{
\text{AetherStudio bridge}
\rightarrow
\text{C++ Metal core}\leftrightarrow\text{Objective-C++}\leftrightarrow\text{SwiftUI}
}
$$

The important conceptual shift from Vulkan is not that Metal is “less explicit.” It is that Metal chooses **different explicit boundaries**.

Vulkan makes state transitions, access scopes and synchronization extraordinarily visible.

Metal exposes a somewhat higher-level execution model around:

$$
\text{device}
\rightarrow
\text{queue}
\rightarrow
\text{command buffer}
\rightarrow
\text{encoder}
$$

while still giving you direct control over memory placement, render-pass semantics, heaps, hazards and GPU scheduling.

---

### METAL'S CORE EXECUTION MODEL

**Metal design philosophy**

Metal is Apple's low-overhead GPU API.

Its design goal is approximately:

$$
\boxed{
\text{minimal driver ambiguity}
+
\text{low CPU submission overhead}
+
\text{explicit resource ownership}
+
\text{tight mapping to Apple GPU architecture}
}
$$

but it intentionally exposes fewer low-level synchronization permutations than Vulkan.

A useful comparison is:

| Vulkan | Metal |
|---|---|
| `VkDevice` | `MTLDevice` |
| `VkQueue` | `MTLCommandQueue` |
| `VkCommandBuffer` | `MTLCommandBuffer` |
| command recording directly into buffer | commands grouped through encoders |
| graphics pipeline | `MTLRenderPipelineState` |
| compute pipeline | `MTLComputePipelineState` |
| descriptor sets | direct resource binding / argument buffers |
| image layouts | no direct equivalent exposed in same form |
| explicit pipeline barriers | hazard tracking + barriers/fences/events |
| swapchain | `CAMetalLayer` / `CAMetalDrawable` |
| SPIR-V | Metal library / MSL functions |

The biggest conceptual difference is the **encoder model**.

In Vulkan, one command buffer can directly contain:

```text
vkCmdDraw
vkCmdDispatch
vkCmdCopyBuffer
...
```

Metal groups commands by operation family:

```text
MTLRenderCommandEncoder
MTLComputeCommandEncoder
MTLBlitCommandEncoder
```

That is not merely cosmetic.

It gives Metal a strong phase structure.

---

**`MTLDevice`**

`MTLDevice` represents a GPU and its Metal capabilities.

Objective-C:

```objective-c
id<MTLDevice> device =
    MTLCreateSystemDefaultDevice();
```

metal-cpp equivalent:

```cpp
MTL::Device* device =
    MTL::CreateSystemDefaultDevice();
```

depending on wrapper/API style.

The device creates most GPU resources:

```text
buffers
textures
samplers
pipeline states
heaps
command queues
libraries
```

It also exposes hardware capabilities.

Maveb explicitly queries Apple GPU family support and capabilities such as:

$$
\text{ray tracing}
$$

and:

$$
\text{dynamic libraries}.
$$

Its current capability inspection iterates Apple GPU family support and records the highest family exposed.

That is preferable to saying:

> I am on an M-series Mac, therefore feature X exists.

Query actual capabilities.

---

**Kairo's device ownership**

Kairo's Metal backend creates:

```objective-c
Device =
    MTLCreateSystemDefaultDevice();
```

and throws if no device exists.

Then:

```objective-c
Queue =
    [Device newCommandQueue];
```

That gives the backend a single default device and one primary command queue.

Maveb instead receives/retains a device from its MetalKit bridge and creates its queue internally.

Both designs are legitimate.

The latter is especially useful when UI layer already owns an `MTKView` with a selected device.

---

**Device identity versus rendering context**

Unlike legacy OpenGL, there is no giant implicit global rendering context.

A Metal resource carries device association.

The application can have references to:

$$
device_A,
device_B
$$

although Apple systems commonly expose one integrated/default GPU in modern Apple-silicon environments.

The important architectural idea is:

$$
\boxed{
\text{GPU ownership is object-based, not implicit-context-based}.
}
$$

---

**`MTLCommandQueue`**

A command queue receives command buffers.

Creation:

```objective-c
id<MTLCommandQueue> queue =
    [device newCommandQueue];
```

Conceptually:

```text
CPU
 ↓
command buffer 0
command buffer 1
command buffer 2
 ↓
MTLCommandQueue
 ↓
GPU scheduling
```

Command buffers submitted to one queue have ordered queue semantics.

That does not mean every GPU instruction from one command buffer must completely finish before any useful work from the next can exist internally.

The implementation may pipeline work where legal.

But command dependencies/order must remain semantically correct.

---

**Maveb's queue**

Maveb:

```cpp
commandQueue_ =
    adopt(
        device->newCommandQueue());
```

and labels it:

```text
AETHER Primary Queue
```

This is good instrumentation.

The label becomes visible in GPU tools.

---

**One queue is often enough on Apple GPUs**

Do not import desktop Vulkan thinking too literally.

You might imagine:

```text
graphics queue
compute queue
copy queue
```

as mandatory architecture.

Metal supports multiple command queues, but Apple GPU execution and unified memory architecture mean more queues do not inherently mean more physical parallelism.

A single well-scheduled command queue can already provide excellent throughput.

Use additional queues where dependency structure and measurement justify them.

---

**`MTLCommandBuffer`**

A command buffer is a package of GPU work destined for a command queue.

Creation:

```objective-c
id<MTLCommandBuffer> command =
    [queue commandBuffer];
```

or metal-cpp:

```cpp
MTL::CommandBuffer* commandBuffer =
    commandQueue_->commandBuffer();
```

You create encoders from it.

Then:

```cpp
encoder->endEncoding();

commandBuffer->commit();
```

After commit, the GPU can execute it.

---

**Command-buffer lifecycle**

Conceptually:

```text
Created
    ↓
Encoding
    ↓
Committed
    ↓
Scheduled
    ↓
Completed
```

or:

```text
Completed with error
```

You cannot keep appending commands after committing.

This is analogous to Vulkan's command recording/submission model but without the same reusable command-buffer reset model exposed to the application.

Metal command buffers are normally short-lived command objects.

---

**Kairo's one-command-per-frame model**

Kairo does:

```objective-c
id<MTLCommandBuffer> command =
    [Queue commandBuffer];

EncodeScene(
    command,
    frame);

Present(command);

[command commit];

LastCommand =
    command;
```

So each frame produces a fresh command buffer.

That is idiomatic Metal.

---

**Maveb does the same at a larger scale**

At frame start:

```cpp
MTL::CommandBuffer* commandBuffer =
    commandQueue_->commandBuffer();
```

Then that one frame command buffer accumulates numerous passes:

$$
\text{Gaussian work},
$$

$$
\text{shadows},
$$

$$
\text{mesh scene},
$$

$$
\text{TAA},
$$

$$
\text{bloom},
$$

$$
\text{presentation},
$$

$$
\text{optional readback}.
$$

Then:

```cpp
commandBuffer->presentDrawable(
    drawable);

commandBuffer->commit();
```

That is a much richer example of the same execution model.

---

**One active encoder at a time**

Metal command buffers are generally structured as sequential encoding regions.

Conceptually:

```text
command buffer

    RenderEncoder A
        ...
    endEncoding

    ComputeEncoder B
        ...
    endEncoding

    BlitEncoder C
        ...
    endEncoding
```

You do not arbitrarily interleave commands from multiple normal encoders into the same command stream.

Each encoder defines a GPU operation domain.

This structure makes resource/hazard reasoning easier.

---

**`MTLRenderCommandEncoder`**

A render encoder records rasterization work.

Creation:

```objective-c
id<MTLRenderCommandEncoder> encoder =
    [command
        renderCommandEncoderWithDescriptor:
            renderPass];
```

The descriptor specifies attachments and their load/store semantics.

Inside encoder you bind:

```text
render pipeline
depth/stencil state
vertex buffers
fragment buffers
textures
samplers
viewport
scissor
culling state
```

then issue:

```text
drawPrimitives
drawIndexedPrimitives
```

---

**Kairo render encoder**

Kairo's scene pass creates:

```objective-c
id<MTLRenderCommandEncoder> encoder =
    [command
        renderCommandEncoderWithDescriptor:
            pass];
```

and binds:

```text
DepthState
CullMode
FrontFacingWinding
frame uniforms
shadow map
material textures
mesh buffers
```

Then draws its opaque and transparent objects.

This maps closely to its Vulkan backend semantically.

The API details differ.

---

**Metal separates pipeline state from many dynamic states**

`MTLRenderPipelineState` contains compiled shader/pipeline configuration, but states such as:

$$
cullMode,
$$

$$
frontFacingWinding,
$$

$$
viewport,
$$

$$
scissor,
$$

$$
depthStencilState
$$

are bound independently on encoder.

Compare Vulkan's more aggressively baked pipeline-state object.

Metal's split makes certain state changes less pipeline-permutation-heavy.

---

**`MTLComputeCommandEncoder`**

Compute encoder records general compute kernels.

Conceptually:

```cpp
auto* encoder =
    commandBuffer->
        computeCommandEncoder();

encoder->
    setComputePipelineState(
        pipeline);

encoder->
    setBuffer(
        buffer,
        offset,
        slot);

encoder->
    dispatchThreads(
        grid,
        group);

encoder->
    endEncoding();
```

This is where Maveb's Gaussian scan/radix pipeline from Pass 9 lives.

---

**Compute dispatch mapping**

If:

$$
N
$$

threads are needed and group size:

$$
G,
$$

you can issue:

```cpp
dispatchThreads(
    MTL::Size(N,1,1),
    MTL::Size(G,1,1));
```

Metal generates enough threadgroups to cover the logical grid.

Or explicitly dispatch threadgroups.

The MSL kernel receives IDs such as:

```metal
uint id
    [[thread_position_in_grid]]
```

exactly as we discussed in GPU architecture.

---

**Compute pipeline properties**

A `MTLComputePipelineState` can expose:

$$
threadExecutionWidth
$$

and:

$$
maxTotalThreadsPerThreadgroup.
$$

These matter for choosing good workgroup sizes.

Recall Pass 9:

$$
\boxed{
\text{workgroup size should respect subgroup/execution width}.
}
$$

Metal gives you implementation data for this rather than requiring you to assume a 32-lane warp.

---

**`MTLBlitCommandEncoder`**

Blit encoder handles data movement and utility transfer operations.

Examples include:

$$
buffer\rightarrow buffer,
$$

$$
texture\rightarrow buffer,
$$

$$
buffer\rightarrow texture,
$$

$$
texture\rightarrow texture,
$$

mipmap generation and synchronization/copy operations depending on resource/storage mode.

Maveb uses a blit encoder to generate mipmaps:

```cpp
blit->generateMipmaps(
    texture);
```

and to copy a presented drawable into a CPU-readable buffer for frame capture.

---

**Why use a blit encoder rather than a compute shader for copies?**

A compute shader can copy bytes.

But dedicated copy/blit hardware and driver paths may be more efficient.

Blit commands also clearly communicate intent:

$$
\boxed{
\text{this operation moves data}
}
$$

rather than:

$$
\text{this is arbitrary shader computation}.
$$

That can improve scheduling and optimization.

---

**Maveb frame capture**

Maveb creates a shared buffer and records:

```cpp
captureEncoder->
    copyFromTexture(
        drawableTexture,
        ...
        captureBuffer,
        ...);
```

Then completion handler reads `captureBuffer->contents()` only after command buffer has completed.

That is the correct asynchronous readback pattern.

---

**`MTLBuffer`**

A Metal buffer is a linear byte allocation.

Creation can be:

```objective-c
id<MTLBuffer> b =
    [device
        newBufferWithLength:
            size
        options:
            MTLResourceStorageModeShared];
```

or from initial bytes:

```objective-c
newBufferWithBytes:
    data
length:
    bytes
options:
    ...
```

Like Vulkan buffer, interpretation comes from binding context.

---

**Buffer offset binding**

You can bind one buffer at different offsets:

```cpp
encoder->setVertexBuffer(
    frameBuffer,
    frameOffset,
    0);
```

Thus one giant upload arena can hold:

```text
camera
draw constants
skin palette
lights
cluster data
...
```

without creating a new Metal buffer for each allocation.

Maveb's `FrameContext` is explicitly designed around this.

---

**`MTLTexture`**

Texture stores structured image data.

Descriptor specifies:

$$
type,
format,
width,
height,
depth,
mipCount,
arrayLength,
sampleCount,
storageMode,
usage.
$$

Maveb's scene HDR texture:

$$
RGBA16Float
$$

with:

$$
StorageModePrivate
$$

and usage:

$$
RenderTarget
|
ShaderRead.
$$

That is exactly the right shape for a GPU-only HDR render target later sampled by post-processing.

---

**Texture usage flags matter**

Examples:

$$
ShaderRead,
$$

$$
ShaderWrite,
$$

$$
RenderTarget,
$$

$$
PixelFormatView.
$$

Declaring correct usage lets Metal optimize resource placement/layout internally.

This is conceptually similar to Vulkan image usage flags, although Metal hides explicit image layouts from you.

---

**`MTLSamplerState`**

Sampler state controls:

$$
minification,
magnification,
mip filtering,
addressing,
comparison sampling,
anisotropy,
LOD range.
$$

Creation:

```objective-c
MTLSamplerDescriptor* d =
    [MTLSamplerDescriptor new];

d.minFilter =
    MTLSamplerMinMagFilterLinear;

d.magFilter =
    MTLSamplerMinMagFilterLinear;

id<MTLSamplerState> sampler =
    [device
        newSamplerStateWithDescriptor:d];
```

Kairo binds texture and sampler separately, unlike Vulkan's commonly used combined-image-sampler descriptor.

---

**Comparison samplers**

Maveb's shadow sampler uses:

$$
CompareFunctionLessEqual.
$$

Shader sampling can perform depth comparison directly.

This is useful for:

- PCF shadows,
- hardware comparison filtering.

Sampler state therefore has semantic impact beyond visual filtering.

---

**`MTLRenderPipelineState`**

This is a compiled render pipeline.

Descriptor includes:

- vertex function,
- fragment function,
- color formats,
- blend state,
- depth/stencil attachment formats,
- sample count,
- vertex descriptor where required.

Then:

```objective-c
id<MTLRenderPipelineState>
pipeline =
    [device
        newRenderPipelineStateWithDescriptor:
            descriptor
        error:
            &error];
```

---

**Kairo pipeline construction**

Kairo builds runtime Metal source into a library.

Then obtains:

```text
mesh_vertex
mesh_fragment
skinned_mesh_vertex
shadow_vertex
debug_vertex
...
```

and creates pipeline variants.

It configures:

$$
ViewportColorFormat=RGBA16Float
$$

and:

$$
ViewportIDFormat=R32Uint.
$$

The same semantic attachments exist in its Vulkan backend.

This is good backend agreement.

---

**Blend state belongs to pipeline descriptor**

For transparent pipeline Kairo sets:

$$
sourceRGB=
SourceAlpha,
$$

$$
destinationRGB=
OneMinusSourceAlpha.
$$

Exactly the same blend equation as Vulkan:

$$
C_o=
\alpha_s C_s
+
(1-\alpha_s)C_d.
$$

This demonstrates an important multi-backend principle:

$$
\boxed{
\text{rendering semantics can match even when API object models differ}.
}
$$

---

**`MTLComputePipelineState`**

Compute state is built from an MSL `kernel` function.

Conceptually:

```cpp
auto function =
    library->
        newFunction(
            "scan");

auto pipeline =
    device->
        newComputePipelineState(
            function,
            &error);
```

Then bind:

```cpp
encoder->
    setComputePipelineState(
        pipeline);
```

Maveb's Gaussian system contains multiple pipelines for:

- projection,
- counting,
- scan,
- key generation,
- radix histogram,
- radix scatter,
- compositing.

Each represents a specialized compute stage.

---

**Why compute pipeline state is separate**

Compute has no:

- rasterizer,
- blend state,
- color attachments,
- vertex assembly.

So its pipeline object is simpler.

It still contains compiled executable metadata and execution-width/resource usage relevant to dispatch.

---

**shader libraries**

Metal shader functions live inside a:

$$
MTLLibrary.
$$

There are two major workflows.

Runtime source compilation:

```objective-c
newLibraryWithSource
```

and offline compiled:

```text
.metal
↓
Metal compiler
↓
.metallib
```

loaded with:

```cpp
device->newLibrary(
    path,
    &error);
```

---

**Kairo versus Maveb shader strategy**

Kairo currently stores MSL source in its backend and performs:

```objective-c
newLibraryWithSource
```

during initialization.

Excellent for:

- simplicity,
- rapid testing,
- self-contained backend.

Maveb loads:

```text
AetherShaders.metallib
```

from its app bundle.

That is the more production-oriented approach because shader compilation moves out of the normal runtime path.

---

**Offline shader compilation advantages**

It reduces runtime pipeline startup cost.

It allows build-time validation.

It makes application binaries less dependent on runtime source compilation.

It enables better build-system integration.

For an engine like Kairo, you can eventually keep source in repository but compile to metallib during asset/build pipeline.

---

**MSL vertex functions**

A Metal vertex function looks conceptually like:

```metal
struct VertexOut
{
    float4 position
        [[position]];

    float2 uv;
};

vertex VertexOut vertexMain(
    uint vertexID
        [[vertex_id]],

    const device Vertex* vertices
        [[buffer(0)]])
{
    VertexOut out;

    const Vertex v =
        vertices[vertexID];

    out.position =
        ...;

    out.uv =
        v.uv;

    return out;
}
```

`vertex` is the stage qualifier.

---

**MSL address spaces**

MSL makes memory domains explicit.

Common qualifiers:

$$
device,
$$

$$
constant,
$$

$$
threadgroup,
$$

$$
thread.
$$

`device` refers to GPU-accessible device memory.

`constant` is read-only uniform-ish memory.

`threadgroup` is workgroup-shared on-chip memory.

`thread` is private per invocation.

These map directly to the GPU memory hierarchy from Pass 9.

---

**Vertex input can use buffer pointers or stage-in attributes**

Metal supports different vertex-input styles.

You can use a vertex descriptor and:

```metal
[[stage_in]]
```

or manually index a device buffer using:

$$
vertex_id.
$$

Both are valid.

Explicit buffer access can be useful when building custom geometry pipelines.

---

**MSL fragment functions**

Fragment:

```metal
fragment float4 fragmentMain(
    VertexOut in
        [[stage_in]],

    texture2d<float> albedo
        [[texture(0)]],

    sampler s
        [[sampler(0)]])
{
    return albedo.sample(
        s,
        in.uv);
}
```

Fragment function runs for generated fragment/sample work.

It can access:

- interpolated stage data,
- buffers,
- textures,
- samplers.

---

**Metal texture/sampler separation**

MSL:

```metal
texture2d<float>
    texture [[texture(0)]]

sampler
    sampler0 [[sampler(0)]]
```

lets same texture be used with different samplers.

This differs from Vulkan's commonly used:

```text
combined image sampler
```

although Vulkan can also bind images and samplers separately.

API abstraction should therefore not assume that:

$$
TextureHandle
=
Texture+Sampler
$$

is always the best universal representation.

---

**MSL compute kernels**

Compute functions use:

```metal
kernel
```

rather than `vertex` or `fragment`.

Example:

```metal
kernel void multiply(
    const device float* lhs
        [[buffer(0)]],

    const device float* rhs
        [[buffer(1)]],

    device float* output
        [[buffer(2)]],

    uint id
        [[thread_position_in_grid]])
{
    output[id] =
        lhs[id] *
        rhs[id];
}
```

This is essentially the exact execution model we derived in Pass 9.

---

**Threadgroup memory in MSL**

Example reduction:

```metal
kernel void reduce(
    const device float* input
        [[buffer(0)]],

    device float* output
        [[buffer(1)]],

    uint local
        [[thread_index_in_threadgroup]],

    uint group
        [[threadgroup_position_in_grid]])
{
    threadgroup float
        scratch[256];

    scratch[local] =
        input[
            group * 256 +
            local];

    threadgroup_barrier(
        mem_flags::mem_threadgroup);

    ...
}
```

The keyword `threadgroup` is the concrete Metal spelling of shared-memory concepts from Phase 22.

---

### STORAGE MODES, TILE MEMORY, DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION

**Metal storage modes**

Metal's storage modes describe where/how a resource's backing storage is accessible.

The major conceptual modes are:

$$
Shared,
$$

$$
Private,
$$

$$
Managed,
$$

$$
Memoryless.
$$

These are not merely performance hints.

They define CPU/GPU access rules.

---

**`Shared`**

Shared storage is directly CPU and GPU accessible.

For buffer:

```cpp
auto* b =
    device->
        newBuffer(
            bytes,
            MTL::ResourceStorageModeShared);
```

CPU pointer:

```cpp
void* p =
    b->contents();
```

Then CPU can write directly.

On Apple unified-memory architectures this maps very naturally to physical unified memory.

---

**Best uses of shared memory**

Typical:

$$
\text{per-frame constants},
$$

$$
\text{dynamic geometry},
$$

$$
\text{upload staging},
$$

$$
\text{readback}.
$$

But shared does not mean:

> use it for every large GPU resource.

GPU-only textures and large static compute buffers may be better as private resources because Metal can optimize them for GPU use.

---

**Maveb frame uploads**

Each `FrameContext` creates:

$$
8\text{ MiB}
$$

shared upload buffer.

It also creates:

$$
256\text{ KiB}
$$

shared readback buffer.

Allocation is:

$$
offset=
alignUp(cursor,alignment).
$$

Current default alignment:

$$
256.
$$

Then:

```cpp
cpuAddress =
    buffer->contents()
    + offset;
```

This is a clean per-frame arena.

---

**Why shared memory still needs lifetime synchronization**

Unified physical memory does not remove races.

Suppose CPU writes:

$$
cameraFrame1
$$

into the same bytes while GPU is still reading:

$$
cameraFrame0.
$$

Physical memory is shared.

That makes the race more direct, not less real.

You still need frame-slot ownership.

---

**`Private`**

Private resources are GPU-access-only from ordinary CPU perspective.

CPU does not obtain meaningful direct pointer to resource data.

Use for:

$$
\text{render targets},
$$

$$
\text{static geometry},
$$

$$
\text{large compute buffers},
$$

$$
\text{GPU-only intermediate textures}.
$$

Upload normally goes:

$$
Shared\ staging
\rightarrow
Blit
\rightarrow
Private.
$$

---

**Maveb uses private render targets**

Its:

```text
Scene HDR Color
Scene Reverse-Z Depth
Scene Entity IDs
Scene Motion Vectors
Bloom textures
TAA histories
Gaussian Color/Depth/IDs
Shadow arrays
```

are allocated with:

$$
MTL::StorageModePrivate.
$$

That is exactly the right architecture for GPU-resident working data.

---

**Kairo currently makes a different simplification**

Kairo's smaller Metal backend creates viewport textures with:

$$
MTLStorageModeShared.
$$

That allows simple synchronous:

```objective-c
getBytes:
```

readback for picking/capture.

This favors code simplicity.

Maveb's private targets plus explicit blit readback are the more scalable production architecture.

This is a good example of why architecture evolves with engine maturity.

---

**`Managed` storage concept**

Managed storage is primarily a macOS-oriented concept for systems where CPU and GPU may maintain distinct copies.

Conceptually:

```text
CPU-visible copy
↕ synchronization
GPU-visible copy
```

CPU writes may require notifying Metal that a range changed.

GPU-produced data may need synchronization back toward the CPU copy before CPU reads.

The exact availability/behavior is platform-dependent, so query/use it only where appropriate.

On Apple-silicon unified-memory engines, `Shared` and `Private` are usually the more central choices.

---

**Why Managed exists historically**

Discrete GPU memory architecture:

```text
CPU RAM
     ↕
interconnect
     ↕
GPU VRAM
```

A resource that both CPU and GPU use may have two copies.

Managed mode helps Metal track/coherently synchronize those representations.

This is fundamentally different from physically shared Apple-silicon DRAM.

---

**Memoryless resources**

A memoryless render target need not have persistent backing storage outside on-chip tile memory.

Conceptually:

```text
tile begins
    ↓
temporary depth/color data lives on-chip
    ↓
tile processing ends
    ↓
data discarded
```

No need to write the entire attachment to system memory if nobody needs it afterward.

This is powerful on tile-based GPUs.

---

**When memoryless is valid**

Suppose depth exists only for depth testing during one render pass.

No later pass samples depth.

Then:

$$
\boxed{
\text{depth data is temporally local to the pass}.
}
$$

A memoryless depth attachment may be ideal where supported.

But if later TAA/SSAO/Hi-Z needs depth:

$$
\boxed{
\text{depth must survive}.
}
$$

Then memoryless would be wrong.

---

**Apple tile-based deferred rendering intuition**

This is one of the most important Apple GPU concepts.

A conventional immediate mental model says:

```text
triangle
↓
rasterize
↓
write every fragment result to framebuffer memory
```

A tile-based renderer instead divides the framebuffer into screen tiles.

Conceptually:

```text
geometry processing
      ↓
bin primitives into tiles
      ↓

Tile 0:
    load/clear tile attachments
    rasterize relevant primitives
    blend/depth test in on-chip tile memory
    store needed results

Tile 1:
    same
...
```

The crucial resource is:

$$
\boxed{
\text{fast on-chip tile memory}.
}
$$

---

**Why tile rendering saves bandwidth**

Suppose the same pixel is written by 20 overlapping fragments.

Immediate external-memory mental model might imply repeated:

$$
read\ depth,
write\ depth,
read\ color,
write\ color.
$$

Tile memory lets many of those operations remain on-chip.

Only the final attachment result may need to be written to system memory.

This is why attachment `loadAction` and `storeAction` matter so much on Apple GPUs.

---

**Bandwidth is expensive**

Apple GPUs operate under power and memory-bandwidth constraints.

On-chip tile operations are much cheaper than repeatedly hitting DRAM.

Therefore one of the most important Metal performance principles is:

$$
\boxed{
\text{avoid unnecessary attachment loads and stores}.
}
$$

---

**Metal load/store actions**

At start of render pass, an attachment has a load action.

Conceptually:

$$
Load,
Clear,
DontCare.
$$

At end:

$$
Store,
DontCare
$$

plus resolve variants for multisampling.

Same semantic idea as Vulkan Pass 11.

But on tile-based GPUs the bandwidth consequences become especially tangible.

---

**LoadActionClear**

If pass overwrites entire target and old contents are irrelevant:

```cpp
attachment->
    setLoadAction(
        MTL::LoadActionClear);
```

GPU can initialize tile memory directly.

It need not load old attachment data from system memory.

This is often significantly better than:

$$
Load
$$

followed by shader overwrite.

---

**LoadActionDontCare**

If every relevant pixel will definitely be overwritten and you do not even require a defined initial clear:

$$
DontCare.
$$

The GPU may skip initialization entirely.

Only use it when shader/render coverage guarantees old values are never semantically observed.

---

**StoreActionDontCare**

If attachment is only temporary:

```cpp
depthAttachment->
    setStoreAction(
        MTL::StoreActionDontCare);
```

Metal does not need to preserve tile result after render pass.

For a transient depth buffer, this can avoid large memory writeback.

---

**Kairo's depth target**

Kairo's viewport scene pass sets:

$$
depth.loadAction=Clear
$$

and:

$$
depth.storeAction=DontCare.
$$

That is ideal if depth is used only for that pass.

It says:

> I need a fresh depth buffer during rasterization, but I do not care about its value afterward.

This is exactly how you should reason on a tile GPU.

---

**Maveb differs because it uses depth later**

Maveb's scene depth participates in later temporal/postprocessing logic.

Therefore it cannot universally throw it away at the end of the geometry pass.

Resource lifetime determines store action.

This is precisely the kind of decision a future render graph can infer.

---

**Color attachments**

A Metal render pass color attachment specifies:

$$
texture,
$$

$$
loadAction,
$$

$$
storeAction,
$$

$$
clearColor.
$$

For MSAA there may also be resolve texture.

Maveb temporal pass:

```text
destination = TAA history output
load = DontCare
store = Store
```

Correct because every result is newly written, but later presentation/bloom needs it.

---

**Multiple render targets**

Kairo/Maveb can attach multiple color targets:

```text
HDR color
Object ID
Motion vector
Normal/confidence
```

One fragment shader may output different values to different attachments.

This is Metal's equivalent of MRT.

Formats are baked into render pipeline state.

Therefore pipeline must match attachment formats.

---

**Depth attachments**

Depth attachment descriptor contains:

$$
texture,
loadAction,
storeAction,
clearDepth.
$$

Depth compare/write behavior itself lives in:

$$
MTLDepthStencilState.
$$

These are separate.

Render pass says:

> Which depth texture exists and how its lifetime begins/ends.

Depth state says:

> How fragments compare/write depth.

---

**Maveb reverse-Z**

Maveb explicitly labels scene depth:

```text
Scene Reverse-Z Depth
```

and uses reverse-Z states.

With reverse-Z:

$$
near\rightarrow1,
$$

$$
far\rightarrow0.
$$

So clear depth commonly becomes:

$$
0.
$$

Depth comparison becomes:

$$
Greater
$$

or equivalent.

This contrasts with Kairo's conventional depth path.

---

**Drawable acquisition**

To present to a screen, Metal gives you a drawable.

With `MTKView`:

```cpp
auto* drawable =
    view->
        currentDrawable();
```

or Objective-C:

```objective-c
id<CAMetalDrawable>
drawable =
    layer.nextDrawable;
```

This drawable owns/presents a texture.

You render into that texture.

---

**Acquire drawable as late as practical**

Drawable pools are finite.

Once you call:

```text
nextDrawable
```

you hold one presentation resource.

Acquiring far earlier than needed can reduce presentation pipeline flexibility and potentially block if drawable pool is exhausted.

A good rule:

$$
\boxed{
\text{acquire presentation resource near the point it becomes necessary}.
}
$$

---

**Maveb MetalKit acquisition**

Maveb obtains:

```cpp
renderPass =
    view->
        currentRenderPassDescriptor();

drawable =
    view->
        currentDrawable();
```

and immediately validates them.

If window is unavailable/minimized and drawable is nil:

$$
\boxed{
\text{skip frame cleanly}.
}
$$

Correct.

---

**`CAMetalLayer`**

`CAMetalLayer` is the lower-level Core Animation layer that supplies Metal drawables.

Kairo constructs one directly:

```objective-c
Layer =
    [CAMetalLayer layer];

Layer.device =
    Device;

Layer.pixelFormat =
    MTLPixelFormatBGRA8Unorm_sRGB;

Layer.framebufferOnly =
    YES;

Layer.displaySyncEnabled =
    YES;
```

then attaches it to the Cocoa window's content view.

---

**`framebufferOnly`**

If true, drawable texture is optimized for render-target presentation and restricted from broader shader/readback use.

This can improve performance.

Maveb temporarily sets its MetalKit view's framebuffer-only behavior differently when frame capture is required because it needs to copy/read the drawable.

That demonstrates the real tradeoff:

$$
\boxed{
\text{narrow usage permits stronger optimization}.
}
$$

---

**Drawable size**

Kairo updates:

```objective-c
Layer.drawableSize =
    CGSizeMake(
        width,
        height);
```

with physical framebuffer dimensions.

As with Vulkan, retina logical points are not necessarily framebuffer pixels.

Renderer must work in drawable pixel dimensions.

---

**presenting drawables**

You do not immediately display the drawable when you finish encoding.

You schedule presentation on a command buffer:

```objective-c
[command
    presentDrawable:
        drawable];

[command commit];
```

Presentation occurs after command buffer reaches appropriate point.

Maveb metal-cpp:

```cpp
commandBuffer->
    presentDrawable(
        drawable);

commandBuffer->
    commit();
```

---

**Why presentation belongs to command buffer**

The GPU must finish rendering the drawable before display consumes it.

By encoding presentation into the command stream, Metal naturally associates presentation with rendering completion.

This is conceptually similar to Vulkan:

$$
renderFinished
\rightarrow
vkQueuePresentKHR,
$$

but Metal makes the common dependency more integrated.

---

**command-buffer completion handlers**

Metal allows:

```cpp
commandBuffer->
    addCompletedHandler(
        [](MTL::CommandBuffer* cb)
        {
            ...
        });
```

Handler executes after GPU finishes command buffer.

This is useful for:

- releasing frame slot,
- processing readback,
- statistics,
- deferred cleanup,
- error inspection.

---

**Completion handlers are asynchronous**

Do not assume completion handler runs on:

$$
\text{main UI thread}.
$$

Treat it as arbitrary asynchronous callback context.

Therefore it should not directly mutate AppKit/SwiftUI UI state unless explicitly marshalled to main thread.

---

**Maveb completion handler**

Maveb:

```cpp
commandBuffer->
    addCompletedHandler(
        [this,semaphore](...)
        {
            ...
            dispatch_semaphore_signal(
                semaphore);
        });
```

It computes coarse GPU duration:

$$
GPUEndTime-GPUStartTime
$$

stores it atomically, increments completed-frame count and releases one frame-in-flight slot.

That is excellent use of completion handler.

---

**Frame capture completion**

Another handler copies data out of shared readback buffer only after GPU is finished.

It protects completed capture with:

```cpp
std::lock_guard
```

because completion occurs asynchronously from app/UI thread.

Again, correct cross-thread ownership.

---

**frames-in-flight semaphore limiting**

Maveb creates:

$$
dispatch_semaphore_create(3).
$$

At frame start:

```cpp
dispatch_semaphore_wait(
    frameSemaphore_,
    FOREVER);
```

At command completion:

```cpp
dispatch_semaphore_signal(
    frameSemaphore_);
```

So maximum submitted-not-completed frames is:

$$
\boxed{3}.
$$

---

**Why this prevents overwrite**

Frame slot:

$$
slot=
frameNumber\bmod3.
$$

Because semaphore guarantees no more than three incomplete frames, by the time CPU cycles back to same slot, earlier GPU work using that slot must have completed.

Then:

```cpp
frame.beginFrame();
```

can reset upload/readback cursors safely.

This is the Metal analogue of multi-frame fence slots in Vulkan.

---

**Triple buffering latency tradeoff**

More frames in flight improve CPU/GPU overlap.

But worst-case input-to-display latency can increase.

If CPU gets three frames ahead:

$$
Input_N
$$

may not be visible until several queued frames execute.

Therefore:

$$
\boxed{
\text{throughput}
\neq
\text{latency}.
}
$$

Competitive interactive applications may deliberately cap frames in flight lower.

---

**per-frame upload buffers**

Maveb has one:

$$
8\text{ MiB}
$$

shared upload arena per frame context.

At frame start:

$$
cursor=0.
$$

Each upload:

$$
offset=
alignUp(cursor,a).
$$

Then:

$$
cursor=
offset+size.
$$

This avoids hundreds/thousands of tiny allocations.

---

**Example frame allocation**

Suppose frame needs:

$$
320\text{ byte camera}
$$

with:

$$
256\text{ byte alignment}.
$$

Allocate at:

$$
0.
$$

Next 80-byte material data:

current cursor:

$$
320.
$$

Aligned:

$$
512.
$$

So:

$$
offset=512.
$$

Next cursor:

$$
592.
$$

That wasted padding is the cost of alignment.

---

**Metal buffer offsets and alignment**

Binding:

```cpp
encoder->
    setVertexBuffer(
        buffer,
        offset,
        index);
```

requires offset satisfying relevant alignment requirements.

Different resource usages have different constraints.

The safe architecture is to centralize allocation alignment rules rather than scatter magic padding throughout shaders/render code.

Maveb defaults its frame arena allocations to:

$$
256
$$

byte alignment, which is conservative and convenient for uniform-like data.

---

**Alignment formula**

For power-of-two alignment:

$$
\boxed{
alignUp(x,a)
=
(x+a-1)\&
\sim(a-1)
}
$$

Maveb uses exactly:

```cpp
(value + alignment - 1)
&
~(alignment - 1)
```

and validates alignment is power of two.

Good allocator hygiene.

---

**argument buffers**

Direct binding:

```text
setBuffer slot 0
setBuffer slot 1
setTexture slot 0
setTexture slot 1
setSampler slot 0
...
```

can become expensive for large material/resource sets.

Argument buffers package references to multiple resources into a buffer-backed structure.

MSL concept:

```metal
struct MaterialArguments
{
    texture2d<float>
        baseColor
        [[id(0)]];

    texture2d<float>
        normal
        [[id(1)]];

    sampler
        samplerState
        [[id(2)]];
};
```

Shader receives argument-buffer structure at one buffer slot.

---

**Argument buffers are Metal's route toward bindless-style resource indexing**

Instead of rebinding every texture per draw, you can build arrays/tables of:

$$
textures,
samplers,
buffers.
$$

Material stores indices.

GPU selects appropriate resources.

This aligns extremely well with:

- GPU-driven rendering,
- large material databases,
- indirect draws.

Conceptually it corresponds to Vulkan descriptor indexing/bindless architectures.

---

**Argument buffers do not erase residency/hazard concerns**

A buffer containing references to resources does not mean Metal automatically knows all intended indirect resource usage in every configuration.

Depending on argument-buffer tier/resource model, you may need to explicitly declare resource/heap usage to an encoder.

This is analogous to the principle:

$$
\boxed{
\text{indirect reference}
\neq
\text{resource lifetime magically solved}.
}
$$

---

**Current repository state**

The inspected Kairo/Maveb source paths use direct:

```text
setVertexBuffer
setFragmentBuffer
setFragmentTexture
setFragmentSamplerState
```

binding extensively.

I did not find an established argument-buffer architecture in the inspected paths.

So argument buffers are a future scalability tool, not something I will pretend is already implemented.

---

**Metal heaps**

`MTLHeap` is a large resource-memory pool from which buffers/textures can be created.

Conceptually:

```text
MTLHeap
    ↓
Texture A
Texture B
Buffer C
...
```

Benefits include:

- reduced allocation overhead,
- memory control,
- resource aliasing potential,
- predictable transient allocation.

---

**Maveb frame heap**

Each frame context creates:

$$
64\text{ MiB}
$$

private heap.

Descriptor:

$$
StorageModePrivate
$$

and:

$$
HazardTrackingModeTracked.
$$

It is labelled:

```text
Frame N Transient Heap
```

That is exactly the infrastructure you want for short-lived GPU resources.

---

**heap suballocation**

When resources are allocated from a heap, Metal manages placement within heap according to resource size/alignment requirements.

The engine still needs to reason about:

$$
\text{heap capacity},
$$

$$
\text{resource lifetime},
$$

$$
\text{reuse}.
$$

For advanced aliasing, logically non-overlapping resources can reuse backing heap space where API/resource rules allow.

---

**Heap aliasing intuition**

Suppose:

$$
Texture_A
$$

exists during passes 1–3.

$$
Texture_B
$$

exists during passes 5–7.

Their lifetimes do not overlap.

Instead of reserving:

$$
memory(A)+memory(B),
$$

they can conceptually reuse:

$$
\max(memory(A),memory(B))
$$

of physical storage.

This will become explicit render-graph lifetime analysis in Pass 14.

---

**Heaps do not automatically give optimal aliasing**

Creating a heap does not mean:

> all resources magically share memory efficiently.

Engine still needs lifetime knowledge and appropriate resource creation/reuse strategy.

Maveb has the per-frame heap infrastructure, but I would not claim its current renderer already performs a full graph-driven interval alias plan.

That belongs to the next architecture stage.

---

**hazard tracking**

A hazard occurs when memory accesses overlap in an unsafe way.

Same familiar classes:

$$
RAW,
WAR,
WAW.
$$

Metal can automatically track many hazards for tracked resources.

This is an important difference from Vulkan's normal explicit synchronization philosophy.

---

**Tracked resources**

With:

$$
MTLHazardTrackingModeTracked,
$$

Metal tracks usage dependencies and inserts required synchronization where needed across relevant encoders/command buffers according to its model.

This greatly simplifies application code.

Maveb's transient heap explicitly uses:

$$
\boxed{HazardTrackingModeTracked}.
$$

That is a deliberate correctness-first choice.

---

**explicit versus implicit hazard tracking**

Tracked mode:

$$
\boxed{
\text{driver/runtime tracks dependencies}
}
$$

Untracked mode:

$$
\boxed{
\text{application promises to synchronize correctly}
}
$$

Why ever use untracked?

Because automatic tracking has overhead.

A sophisticated render graph that already knows every producer/consumer may be able to emit precise synchronization itself.

But if the graph is wrong:

$$
\boxed{
\text{untracked resource can race silently}.
}
$$

---

**This is analogous to Vulkan philosophy**

Vulkan effectively expects you to own synchronization semantics.

Metal lets you choose a more automatic path for many resources.

A mature multi-backend renderer can have high-level resource graph semantics, then map:

```text
Vulkan:
    explicit barrier

Metal tracked resource:
    maybe no manual action

Metal untracked:
    fence/barrier/event as required
```

Same engine-level dependency.

Different backend mechanism.

---

**resource barriers in Metal**

Within compute/render encoders, Metal exposes memory barrier mechanisms for ordering shader/resource memory accesses where needed.

Exact APIs depend on encoder/resource scope and Metal version.

Conceptually:

$$
write
\rightarrow
memoryBarrier
\rightarrow
read.
$$

This is especially relevant when one dispatch/stage writes data that a later dispatch/stage inside the same encoder reads.

---

**Barrier versus encoder boundary**

Ending one encoder and beginning another can establish certain ordering/hazard behavior, especially for tracked resources.

But do not use encoder splitting as random “synchronization superstition.”

You need to know:

- which resource is written,
- where read occurs,
- whether automatic tracking covers it,
- whether same/different command buffer/queue is involved.

Same conceptual discipline as Vulkan remains.

---

**fences and events**

Metal exposes multiple synchronization primitives.

`MTLFence` is a GPU synchronization primitive suited to expressing dependencies between command encoders on a device/queue timeline.

Conceptually:

```text
encoder A
    updateFence(F)

encoder B
    waitForFence(F)
```

This is useful especially with untracked resources/heaps.

---

**Metal fence versus Vulkan fence**

They are **not semantically equivalent despite the name**.

Vulkan:

$$
VkFence
$$

is mainly CPU-visible submission completion.

Metal:

$$
MTLFence
$$

is GPU-side synchronization between encoded work.

Always translate semantics, not names.

This will matter massively in multi-backend abstraction.

---

**Events**

`MTLEvent` and especially:

$$
MTLSharedEvent
$$

support synchronization across command buffers/queues.

Events are more timeline-like than fences.

They can express signal/wait operations between asynchronous work streams.

---

**shared events as timeline synchronization**

Shared event has a:

$$
64\text{-bit value}
$$

conceptually similar to Vulkan timeline semaphore values.

GPU can encode:

$$
signal(event,v)
$$

and later:

$$
wait(event,v).
$$

CPU can also observe/signal shared event state under its API semantics.

Thus you can build:

```text
Upload queue:
    signal 52

Render queue:
    wait 52
    consume uploaded resource

Render:
    signal 53

CPU retirement:
    when completed >= 53
        free resource
```

This is much closer to Vulkan timeline semaphore thinking than `MTLFence`.

---

**Why monotonically increasing values are powerful**

With binary synchronization:

```text
Semaphore A
Semaphore B
Semaphore C
...
```

With a timeline:

$$
1,2,3,\dots
$$

one object can describe a long execution history.

Resource retirement becomes:

$$
lastUseValue
\le
completedValue.
$$

Exactly the model from Pass 12.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/16 - Profile GPUs and Respect Apple Ownership Boundaries|Next chapter]]
