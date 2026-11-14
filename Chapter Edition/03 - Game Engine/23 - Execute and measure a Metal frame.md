# 23 — Execute and measure a Metal frame

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/22 - Vulkan synchronization and multi-frame lifetimes|Previous chapter]] · [[Chapter Edition/03 - Game Engine/24 - Profiling and languageplatform ownership|Next chapter]]

The previous API contracts now meet in a complete frame that must be recorded, submitted, and measured. This chapter turns Metal objects and synchronization into an executable sequence rather than an isolated setup list.

**By the end:** Reconstruct command encoding, submission, presentation, and timing for a Metal frame while preserving each object’s ownership.

**Recall question:** Which operation records work, which submits it, and what evidence shows that the measured interval matches the intended GPU work?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 55750–59391; source-span SHA256 19954dc3a4f963e477d125e7042dfeabbf36e726aedb64e647287ae45975394f. -->

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


### CURRENT VULKAN PRACTICE: LEARN THE EXECUTION CONTRACT, NOT AN OLD OBJECT CHECKLIST

The original Vulkan object model above remains valuable because the API is deliberately explicit, but a modern learning path must distinguish enduring concepts from historical setup patterns. The current Khronos documentation teaches Vulkan using facilities such as dynamic rendering, synchronization2, descriptor indexing, newer feature/property chains, ray tracing and modern compute paths. A tutorial that teaches only the original render-pass/framebuffer-centric style may still explain the architecture correctly, yet leave the reader with ceremony that a current engine no longer needs to bake into every path.

The durable mental model is

\[
\boxed{
\text{resource lifetime}
+
\text{resource state}
+
\text{execution dependency}
+
\text{binding contract}
+
\text{submission order}
}
\]

Everything else is a concrete mechanism for expressing those five things.

#### Dynamic rendering removes an object layer, not attachment semantics

Traditional Vulkan rendering uses render-pass and framebuffer objects. That model remains useful because it exposes attachment lifetime, load/store behavior, subpass dependencies and compatibility. Dynamic rendering can instead describe attachments when encoding work.

~~~
traditional:
render-pass object
+ framebuffer object
+ compatible pipeline
→ begin render pass
→ draw
→ end

dynamic rendering:
attachment descriptions at encoding time
+ pipeline declares compatible formats
→ begin rendering
→ draw
→ end rendering
~~~

The engine still owns image creation, views, formats, sample counts, load/store choices, layouts, access masks, dependencies, aliasing and lifetime. Dynamic rendering removes predeclared objects; it does not remove data hazards.

This is why a cross-backend renderer should expose a pass contract such as “these attachments are read, these are written, these values must be preserved” rather than expose VkRenderPass as the engine abstraction.

#### Synchronization2 is easiest to understand as producer-to-consumer intent

The real question is never “which barrier function do I call?” It is:

> Which earlier access must become visible before which later access, and in which execution stages can those accesses occur?

A dependency is conceptually

\[
\boxed{
(\text{source stage},\text{source access})
\rightarrow
(\text{destination stage},\text{destination access})
}
\]

plus any layout transition or queue-family ownership transfer.

If compute writes a compacted visible-draw buffer and a graphics pass consumes it as indirect/vertex input, the engine dependency is

~~~
compute write
    ↓
availability + visibility
    ↓
graphics/indirect read
~~~

The barrier should be as narrow as correctness allows. “All commands to all commands” can serialize correctly while throwing away overlap opportunities.

A render graph can derive this because it knows producer, consumer, resource and access type. Scattered broad barriers are usually evidence that the architecture lost that information.

#### Timeline semaphores turn completion into epochs

Binary semaphores encode one signal/wait edge. Timeline semaphores associate a monotonically increasing value with progress.

~~~
submission A signals 41
submission B waits 41, signals 42
submission C waits 42, signals 43
~~~

That maps naturally to deferred destruction:

\[
\boxed{
\text{resource last used at epoch }E
\rightarrow
\text{completed epoch}\ge E
\rightarrow
\text{resource may be recycled}
}
\]

The same epoch model can drive staging-ring reuse, upload completion, transient-resource retirement and frame-slot reuse.

The backend-independent lesson is that destruction safety is a relation between last use and completion; timeline semaphores are one clean Vulkan representation of that relation.

#### Descriptor indexing is not literal absence of bindings

Descriptor indexing allows shaders to index large arrays of resources while reducing per-draw rebinding.

A material path can become

~~~
draw record:
    materialIndex
    meshIndex
    transformIndex

material table:
    albedo texture index
    normal texture index
    roughness texture index
    sampler index
~~~

That reduces CPU binding traffic and makes GPU-driven submission practical, but creates other obligations: stable indices, in-flight lifetime protection, fallback slots, capacity budgets, partially bound rules and nonuniform-indexing correctness.

“Bindless” is therefore shorthand for a binding architecture, not for a program with no bindings.

#### GPU-driven rendering reorganizes dataflow

The modern Vulkan rendering literature increasingly emphasizes descriptor indexing, compute-generated work, indirect rendering, data-oriented scene representation, GPU culling, large-scene handling and asynchronous asset work. These should not be learned as disconnected Vulkan tricks. They are consequences of moving decisions from CPU loops into GPU-visible tables.

CPU-driven:

~~~
for each visible object
    bind material
    bind mesh
    upload transform
    draw
~~~

GPU-driven:

~~~
objects[]
materials[]
meshes[]
transforms[]
drawCommands[]

compute:
    cull
    compact
    build indirect arguments

graphics:
    consume indirect work
~~~

The transformation is

\[
\boxed{
\text{CPU decides once per object}
\rightarrow
\text{CPU submits coarse work + data; GPU resolves many decisions}
}
\]

This can reduce CPU/driver overhead, but debugging now spans extraction, culling, compaction, indirect arguments, descriptor indices and the draw itself. Complexity is relocated, not erased.

#### Use tutorial sequence, engine evolution and current docs together

Brendan Galea's Vulkan Game Engine series is valuable because it introduces abstractions in a visible order: device, swapchain, pipelines, command buffers, descriptors, frame resources, models, cameras and lighting appear when each becomes necessary.

Kohi is valuable for long-running engine evolution. Its public code exposes platform, memory, logging, assets, renderer plugins, UI, audio, tools, manifests, testing and hot-reload concerns accumulating over years. Kohi explicitly describes itself as a learning engine and acknowledges that approaches may be explored and later repaired, so its history is part of the lesson.

The official Khronos guide/specification supplies current contract truth: feature availability, synchronization rules, ray-tracing access requirements and modern replacements for older tutorial ceremony.

The synthesis is

\[
\boxed{
\text{tutorial sequence}
+
\text{long-running engine evolution}
+
\text{current specification}
}
\]

The older Vulkan Programming Guide remains excellent for the architectural reason Vulkan exists: application code takes ownership of synchronization, scheduling, memory strategy and command construction that older APIs often hid. Its concrete examples come from an earlier Vulkan generation, so keep the architecture and refresh the mechanisms.

Source basis: Graham Sellers, Vulkan Programming Guide; Kosarevsky, Medvedev and Latypov, Vulkan 3D Graphics Rendering Cookbook, 2nd ed.; current Khronos Vulkan Guide/Tutorial; Brendan Galea's Vulkan Game Engine series; and Travis Vroman's Kohi series.



### CURRENT VULKAN DEEPENING: DESCRIPTORS ARE BECOMING MEMORY, PIPELINES ARE BECOMING MORE DYNAMIC, BUT THE CONTRACT IS STILL EXPLICIT

The modern Vulkan samples add several ideas beyond the baseline dynamic-rendering/synchronization2 path above. They are useful because they show which historical abstractions are optional and which responsibilities remain unavoidable.

#### Descriptor buffers expose the fact that descriptors are encoded data

Traditional Vulkan descriptor management uses descriptor pools, descriptor sets and update calls. Descriptor buffers provide another model in which descriptor data is written into specially created buffers and the command buffer binds those buffers plus offsets.

Conceptually:

~~~text
traditional:
application state
    ↓
vkUpdateDescriptorSets
    ↓
driver-owned descriptor representation
    ↓
bind descriptor set

descriptor buffer:
application asks driver for encoded descriptor bytes
    ↓
application stores bytes in descriptor memory
    ↓
bind descriptor buffer
    ↓
set offsets
~~~

This does not mean descriptor layout becomes arbitrary. The driver still defines:

- encoded descriptor sizes,
- set-layout size,
- binding offsets,
- required alignment,
- supported binding counts.

That is why code must query layout sizes and offsets instead of multiplying a descriptor size by a guessed count.

The deeper lesson is:

\[
\boxed{
\text{descriptor buffer removes a management abstraction}
\quad
\text{but not the shader-interface contract}
}
\]

#### Descriptor-buffer writes are ordinary memory hazards

Once descriptors live in memory, updating that memory on the GPU creates a synchronization problem like any other producer/consumer relationship.

If a transfer or compute operation writes descriptor bytes that a later shader will read, the dependency must make those writes visible to descriptor-buffer reads.

The conceptual edge is:

~~~text
descriptor memory write
    ↓
visibility dependency
    ↓
descriptor fetch by later shader
~~~

This matters because a renderer can no longer think of descriptors as purely CPU-side metadata.

Descriptor data becomes part of GPU-visible state.

#### Host descriptor writes and device descriptor writes have different visibility concerns

A host-written descriptor buffer can rely on the submission visibility rules applicable to host writes and the buffer's memory properties.

A device-written descriptor buffer needs an explicit GPU dependency before shader descriptor reads.

The general rule is:

\[
\boxed{
\text{same logical update}
\not\Rightarrow
\text{same synchronization path}
}
\]

because the producer domain changed.

This is a recurring systems theme: always identify who wrote the data and who will read it.

#### Descriptor buffers make allocator architecture important again

A renderer may allocate descriptor-buffer regions per frame, per material, per pass, or from a persistent heap.

Each choice creates a different lifetime policy.

A frame-linear allocator is simple:

~~~text
Frame N descriptor region
    allocate sequentially
    bind offsets
    submit
    wait for frame retirement
    reset entire region
~~~

A persistent material heap requires free-list or slot-reuse logic plus deferred reuse.

The resource-binding allocator should therefore be designed using the same lifetime categories as ordinary GPU memory.

#### Shader objects decouple shader-stage selection from monolithic pipeline objects

The shader-object extension allows stages and dynamic state to be set without constructing a conventional graphics pipeline object for every permutation.

This changes where state composition happens.

Traditional model:

~~~text
many fixed-function and shader choices
    ↓
pipeline object
    ↓
bind pipeline
    ↓
draw
~~~

Shader-object model:

~~~text
bind/select shader stages
set required dynamic state
    ↓
draw
~~~

This can reduce pipeline-permutation pressure for highly dynamic applications, but it also transfers responsibility to the engine to ensure that all required state has been established before drawing.

The architectural trade is:

\[
\boxed{
\text{less precompiled state bundle}
\Rightarrow
\text{more explicit command-time state responsibility}
}
\]

It is not automatically simpler.

#### Dynamic rendering plus shader objects pushes the backend toward semantic state tracking

When render passes and full pipeline objects become less central, the backend increasingly tracks semantic state such as:

~~~text
active color formats
depth format
shader stages
blend state
raster state
vertex input
topology
sample count
dynamic state
~~~

That can align well with a cross-backend renderer, but only if the engine knows which state is valid and which state must be re-established after changes.

A state cache should therefore be a correctness mechanism first and an optimization second.

#### Dynamic rendering local read closes part of the old subpass gap

Classic Vulkan render passes and subpasses can express tightly coupled attachment reads and writes with tile-local advantages on some architectures.

Dynamic rendering originally simplified render-pass setup but did not reproduce every subpass/local-read pattern.

Dynamic rendering local-read facilities address more of that space, allowing current engines to use dynamic rendering in workflows that previously needed subpasses for local attachment reads.

The important engine lesson is not a specific extension name. It is that API modernization often removes an object model in stages; a new abstraction may initially cover common cases and later absorb specialized capabilities.

Therefore never infer:

~~~text
newer API path exists
→ every older semantic has disappeared
~~~

Instead compare capabilities.

#### Timeline semaphores are powerful, but swapchain presentation remains a special boundary

The Khronos timeline-semaphore sample deliberately demonstrates out-of-order submission, multi-queue waits, host waits and monotonic progress.

It also highlights a practical limitation: window-system swapchain acquire/present synchronization still has special binary-semaphore requirements in common paths.

That means an engine often has two synchronization domains:

~~~text
internal engine progress:
    timeline epochs

WSI edge:
    acquire/present binary synchronization
~~~

Trying to force presentation into the same abstraction without acknowledging the WSI rules can create incorrect assumptions.

A clean backend can keep a generic completion-epoch system internally while isolating the presentation bridge.

#### Out-of-order submission can increase latency even when it is legal

Timeline semaphores permit flexible scheduling, including waits submitted before corresponding signals in scenarios where the API allows it.

But legal scheduling is not automatically good frame pacing.

If the CPU submits far ahead:

\[
\text{queue depth}\uparrow
\Rightarrow
\text{input-to-photon latency may rise}
\]

even if GPU throughput remains high.

A frame scheduler therefore needs both correctness and pacing constraints.

~~~text
correct:
    dependencies eventually resolve

well paced:
    bounded frames/submissions in flight
~~~

The engine should measure both throughput and latency.

#### Streaming needs a safe descriptor publication point

The current Khronos engine tutorial illustrates an important practical pattern:

~~~text
background upload
    ↓
transfer queue copies
    ↓
timeline value signals upload completion
    ↓
graphics waits as needed
    ↓
frame fence proves this frame's descriptor set is not in use
    ↓
descriptor update occurs at safe point
~~~

The critical idea is not the exact descriptor-set mechanism. It is that publishing a new resource binding must happen when both conditions are true:

\[
\boxed{
\text{resource contents ready}
\land
\text{binding storage safe to mutate}
}
\]

If either condition is false, the renderer risks sampling incomplete data or mutating state still consumed by an in-flight frame.

This maps directly to Metal 4 argument-table publication.

#### Descriptor update-after-bind is a permission with conditions, not universal mutability

Vulkan descriptor binding flags can permit certain descriptors to be updated while sets are pending, especially when the updated descriptors are not dynamically used by those command buffers.

That does not mean every bound descriptor can be freely overwritten at any time.

The engine still needs to know which slots an in-flight workload may access.

This is especially important for partially bound arrays and bindless tables.

The safe abstraction is again epoch-based:

~~~text
slot last read by epoch E
new binding wants same slot
    ↓
if completedEpoch < E:
    allocate another slot or defer update
else:
    reuse permitted
~~~

#### Ray queries show why acceleration structures belong to the shared scene database

A ray-query path can run inside compute or fragment shaders without requiring a full ray-generation/miss/hit pipeline.

The renderer still needs BLAS, TLAS, instance transforms, geometry addresses, materials and texture tables.

This means rasterization and ray queries should share scene identity rather than maintain unrelated scene copies.

~~~text
SceneDatabase
|
+-- mesh buffers
+-- material table
+-- texture table
+-- transforms
+-- instance IDs
|
+-- raster visibility path
|
+-- ray-query / ray-tracing path
~~~

The path changes; the scene truth does not.

This is essential for hybrid rendering because divergence between raster and ray-scene representations creates correctness bugs that are difficult to diagnose.

#### Ray-query rebuild policy is a workload tradeoff

Acceleration structures can be built with priorities such as fast traversal or fast rebuild/update.

The right choice depends on geometry behavior.

~~~text
static environment:
    expensive build acceptable
    optimize traversal

frequently deformed/rebuilt geometry:
    rebuild/update cost matters more

mixed scene:
    separate policies by BLAS class
~~~

A single global flag wastes either build time or traversal performance.

The broader rule is that acceleration structure policy belongs to asset/runtime metadata.

#### Mesh shaders change geometry amplification and culling location

A mesh-shader pipeline moves work that traditionally depended on fixed vertex/index assembly plus optional geometry/tessellation stages into programmable task/mesh stages.

This enables cluster/meshlet-oriented culling and amplification.

The engine-level dataflow becomes:

~~~text
meshlets/clusters
    ↓
task-stage culling/work generation
    ↓
mesh-stage vertex + primitive emission
    ↓
rasterization
~~~

This aligns naturally with GPU-driven rendering, but only when assets are preprocessed into suitable clusters and metadata.

Mesh shaders are therefore not a drop-in replacement for vertex shaders. They often require a different asset representation.

#### Fragment shading rate connects rendering policy to perception and content frequency

The Vulkan samples include fragment-shading-rate paths, including dynamic generation of rate information.

For XR/foveated rendering and adaptive shading, the key conceptual mapping is:

\[
\text{importance/frequency field}
\rightarrow
\text{shading-rate field}
\rightarrow
\text{fragment invocation density}
\]

A gaze-driven foveated renderer adds another temporal problem: the shading-rate field is only useful if gaze prediction, display timing, and frame scheduling keep the high-quality region aligned with perception.

Thus a seemingly local Vulkan feature connects directly to XR latency and perceptual error.

#### Modern Vulkan study should preserve one invariant across every feature

Dynamic rendering, descriptor buffers, shader objects, mesh shaders, ray queries, timeline semaphores and synchronization2 all reduce or reorganize API ceremony.

None of them remove the need to answer:

~~~text
Who owns this resource?
Who may read/write it?
When do those accesses happen?
What state/layout/interface does the consumer expect?
When is the last use complete?
When may storage or identity be reused?
~~~

That six-question checklist is more durable than any Vulkan version-specific object checklist.

Source integration basis: current Khronos Vulkan Samples and Guide material covering timeline semaphores, synchronization2, streaming/frame pacing, descriptor management, descriptor buffers, shader objects, dynamic rendering, mesh shaders, ray queries, fragment shading rate, and GPU-driven resource-binding patterns.


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

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/22 - Vulkan synchronization and multi-frame lifetimes|Previous chapter]] · [[Chapter Edition/03 - Game Engine/24 - Profiling and languageplatform ownership|Next chapter]]
