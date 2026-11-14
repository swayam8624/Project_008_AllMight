# 22 — Vulkan synchronization and multi-frame lifetimes

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/21 - Buffers, images, memory, and uploads|Previous chapter]] · [[Chapter Edition/03 - Game Engine/23 - Execute and measure a Metal frame|Next chapter]]

Valid allocations are not enough if reads and writes overlap across queues or frames. This chapter turns pipeline stages, barriers, ownership transfer, and retirement into explicit proofs of execution order.

**By the end:** Derive the dependency and lifetime requirements for a resource crossing Vulkan commands, queues, or frames in flight.

**Recall question:** For each write and later read, which barrier or completion event establishes both visibility and safe reuse?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 51848–55749; source-span SHA256 44c52c968d522cd4f7ec8fa483ef65aad6fbd372a7114433ca610d77b7a313de. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### VULKAN MEMORY MODEL, BARRIERS & MULTI-FRAME LIFETIMES

**Synchronization starts with pipeline stages**

A GPU command does not execute at one indivisible moment.

Graphics pipeline contains conceptual stages such as:

```text
vertex input
vertex shader
rasterization
early fragment tests
fragment shader
late fragment tests
color attachment output
```

Transfer and compute operations have their own stages.

Synchronization says not merely:

> B happens after A.

It says:

> Which **parts** of A and B participate in the dependency?

---

**Source stage and destination stage**

A dependency often has:

$$
S_{src}
$$

and:

$$
S_{dst}.
$$

Source stage identifies producer-side execution scope.

Destination stage identifies consumer-side execution scope.

If transfer writes texture and fragment shader later samples it:

$$
S_{src}
=
TRANSFER,
$$

$$
S_{dst}
=
FRAGMENT_SHADER.
$$

That matches Kairo texture upload.

---

**Stage mask is not memory type**

This is wrong reasoning:

> `FRAGMENT_SHADER` means shader-readable memory.

No.

Pipeline stage says **where in execution**.

Memory access mask says **what kind of memory operation**.

You usually need both concepts.

---

**Access masks**

Examples:

$$
VK_ACCESS_TRANSFER_WRITE_BIT,
$$

$$
VK_ACCESS_SHADER_READ_BIT,
$$

$$
VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
$$

$$
VK_ACCESS_TRANSFER_READ_BIT.
$$

These identify memory access types in producer/consumer scopes.

---

**A dependency has two orthogonal dimensions**

Think:

$$
\boxed{
(\text{pipeline stage},\text{memory access})
}
$$

Producer:

$$
(S_s,A_s).
$$

Consumer:

$$
(S_d,A_d).
$$

Example:

$$
(TRANSFER,\ TRANSFER_WRITE)
$$

to:

$$
(FRAGMENT_SHADER,\ SHADER_READ).
$$

That pair says far more precisely what is being synchronized.

---

**Execution dependency**

Execution dependency alone ensures:

$$
\boxed{
\text{certain destination execution does not begin before certain source execution reaches the required point}.
}
$$

Think:

$$
A_{\text{execution}}
\prec
B_{\text{execution}}.
$$

This is ordering.

It does not by itself fully express memory availability/visibility for arbitrary read/write hazards.

---

**Memory dependency**

A memory dependency additionally handles producer memory writes becoming available and visible to consumer reads/writes.

Conceptually:

Producer:

$$
W_A(x).
$$

Consumer:

$$
R_B(x).
$$

Correctness requires:

$$
\boxed{
W_A
\rightarrow
\text{available}
\rightarrow
\text{visible}
\rightarrow
R_B.
}
$$

This is deeper than “A executes before B.”

---

**Why ordering alone is insufficient**

CPU analogy:

Core 0 writes `x=5`.

Core 1 is told to execute later.

Without appropriate cache/memory-order semantics, “later” is not automatically equivalent to “observes value 5.”

GPU memory systems similarly require explicit visibility rules.

Hence Vulkan distinguishes execution and memory dependencies.

---

**Pipeline barriers**

Classic Vulkan uses:

```cpp
vkCmdPipelineBarrier(...)
```

with:

- source stage mask,
- destination stage mask,
- memory barriers,
- buffer barriers,
- image barriers.

It records a dependency into a command buffer.

A barrier is not a CPU-side blocking call.

It constrains GPU execution.

---

**The misconception: “barrier stalls the whole GPU”**

A carefully scoped barrier does not necessarily mean:

> every unit stops until all previous work finishes.

It restricts only relevant source/destination scopes.

Modern GPUs can continue unrelated work if dependency permits.

This is why overly broad barriers hurt performance.

---

**`TOP_OF_PIPE → BOTTOM_OF_PIPE` is not “extra safe”**

A common beginner habit:

```text
src = ALL_COMMANDS
dst = ALL_COMMANDS
```

for everything.

It may produce correctness in some cases but destroys useful overlap.

Worse, access/stage combinations can still be semantically wrong.

Correct synchronization is:

$$
\boxed{
\text{smallest scopes that fully cover producer and consumer}.
}
$$

---

**Buffer memory barrier**

`VkBufferMemoryBarrier` additionally identifies:

- buffer,
- byte offset/range,
- access masks,
- queue-family ownership indices.

Use when specific buffer memory requires synchronization.

Example:

```text
compute writes SSBO
↓
indirect draw reads same bytes as commands
```

Producer:

$$
COMPUTE_SHADER
+
SHADER_WRITE.
$$

Consumer:

$$
DRAW_INDIRECT
+
INDIRECT_COMMAND_READ.
$$

---

**Image memory barrier**

Images have an extra dimension:

$$
\boxed{\text{layout}.}
$$

A `VkImageMemoryBarrier` can combine:

- execution/memory dependency,
- layout transition,
- queue ownership transfer,
- subresource selection.

That is why image barriers appear everywhere in Vulkan.

---

**Kairo HDR capture barrier**

After viewport render pass, HDR color image is in:

$$
SHADER_READ_ONLY_OPTIMAL.
$$

For capture Kairo wants to copy it to a buffer.

It records:

$$
COLOR_ATTACHMENT_OUTPUT
+
COLOR_ATTACHMENT_WRITE
$$

to:

$$
TRANSFER
+
TRANSFER_READ
$$

and transitions:

$$
SHADER_READ_ONLY_OPTIMAL
\rightarrow
TRANSFER_SRC_OPTIMAL.
$$

Then performs:

```cpp
vkCmdCopyImageToBuffer(...)
```

Then transitions back:

$$
TRANSFER_SRC_OPTIMAL
\rightarrow
SHADER_READ_ONLY_OPTIMAL
$$

with:

$$
TRANSFER_READ
\rightarrow
SHADER_READ.
$$

This is a concrete correct synchronization story.

---

**Why source access says color-attachment write even though oldLayout is shader-read**

This initially looks odd.

The image's **layout at the barrier** is `SHADER_READ_ONLY_OPTIMAL` because the render pass automatically transitioned it to its final layout.

But the producer operation whose writes we need to make available was:

$$
COLOR_ATTACHMENT_WRITE.
$$

Layout describes image state.

Access mask describes the producer memory operation.

These are related but not identical.

Excellent example.

---

**Render passes can perform implicit layout transitions**

Current viewport color attachment:

$$
initialLayout=UNDEFINED
$$

is used as:

$$
COLOR_ATTACHMENT_OPTIMAL
$$

inside subpass.

Final:

$$
SHADER_READ_ONLY_OPTIMAL.
$$

The render pass handles these transitions through its declared attachment/subpass structure and dependencies.

You do not need an explicit external barrier for every internal render-pass transition.

---

**Object-ID attachment final state**

Kairo viewport target sets object-ID attachment final layout to:

$$
\boxed{
TRANSFER_SRC_OPTIMAL
}
$$

because object picking immediately copies a pixel to a readback buffer.

Its external subpass dependency covers:

$$
COLOR_ATTACHMENT_WRITE
\rightarrow
TRANSFER_READ.
$$

Therefore `Record()` can issue:

```cpp
vkCmdCopyImageToBuffer(
    ObjectIDImage,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    ...);
```

without a separate explicit layout barrier there.

This is deliberately encoded into the render-pass contract.

---

**Shadow-map synchronization**

Shadow render pass writes depth in:

$$
EARLY/LATE_FRAGMENT_TESTS
$$

with:

$$
DEPTH_STENCIL_ATTACHMENT_WRITE.
$$

It ends in:

$$
DEPTH_STENCIL_READ_ONLY_OPTIMAL.
$$

Later fragment shading samples it with:

$$
SHADER_READ.
$$

Kairo's subpass dependency expresses exactly:

$$
\text{depth write}
\rightarrow
\text{fragment shader read}.
$$

This is the canonical shadow-map producer→consumer hazard.

---

**RAW, WAR and WAW hazards**

Synchronization reasoning gets easier when you classify hazards.

**RAW — Read After Write**

$$
W_A(x)
\rightarrow
R_B(x)
$$

Consumer must see producer.

Example:

shadow pass writes depth, lighting samples shadow map.

**WAR — Write After Read**

$$
R_A(x)
\rightarrow
W_B(x)
$$

New write must not destroy data before earlier read finishes.

**WAW — Write After Write**

$$
W_A(x)
\rightarrow
W_B(x)
$$

Ordering must define which result wins.

These are the same dependency classes render graphs track.

---

**Kairo's backend-neutral render graph already reasons in hazards**

Current `RenderGraph` compiler tracks:

```text
last writer
active readers
initialized state
```

For read:

- depend on previous writer.

For write:

- depend on previous writer,
- depend on all readers.

That is essentially RAW/WAW/WAR dependency construction.

The graph then topologically sorts passes.

This is exactly the right high-level model.

---

**But current Vulkan frame graph is not yet translating every graph transition to native barriers**

The generic `CompiledRenderGraph` exposes an `ApplyTransitions` hook intended for Vulkan/D3D12/Metal backends.

Current Vulkan frame lifecycle graph mainly uses it to establish CPU/control ordering among:

```text
Retire
Acquire
Record
Submit
Present
```

while actual image barriers remain manually recorded inside Vulkan rendering code.

This is a sensible intermediate architecture.

A future Vulkan render-graph backend can map logical state transitions into native Vulkan synchronization.

---

**Image layouts**

Layouts describe optimized usage/state of image subresources.

Common examples:

$$
UNDEFINED,
$$

$$
TRANSFER_SRC_OPTIMAL,
$$

$$
TRANSFER_DST_OPTIMAL,
$$

$$
COLOR_ATTACHMENT_OPTIMAL,
$$

$$
DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
$$

$$
SHADER_READ_ONLY_OPTIMAL,
$$

$$
PRESENT_SRC_KHR.
$$

A layout transition is part of synchronization.

---

**Layout does not imply automatic synchronization**

This is important:

$$
\boxed{
\text{newLayout}
}
$$

does not alone tell Vulkan:

- which prior writes matter,
- which consumer accesses need visibility.

You must specify correct dependency scopes.

A layout is not a substitute for access/stage masks.

---

**Image subresource transitions**

Barrier can affect:

$$
aspect,
mip,
layer.
$$

You do not have to transition entire image.

Texture streaming might transition only:

$$
mip_5.
$$

Shadow atlas might transition one layer.

This granularity is essential for advanced engines.

---

**Queue-family ownership**

Resources in:

$$
VK_SHARING_MODE_EXCLUSIVE
$$

are generally owned by one queue family at a time.

If transfer queue writes a texture and graphics queue later uses it, ownership may need transfer.

This is distinct from ordinary memory visibility.

---

**Ownership transfer concept**

Transfer queue:

```text
release resource from transfer family
```

Graphics queue:

```text
acquire resource into graphics family
```

and queues also need an inter-queue execution dependency, usually semaphore.

So cross-family resource transfer requires two conceptual things:

$$
\boxed{
\text{ownership transfer}
+
\text{queue synchronization}.
}
$$

One does not replace the other.

---

**Why Kairo avoids this complexity currently**

Kairo uploads through the graphics queue itself.

Textures use:

$$
VK_SHARING_MODE_EXCLUSIVE.
$$

No queue-family transfer is needed because the resource remains within one family.

This is simpler.

A dedicated transfer queue becomes worthwhile only when the performance benefit outweighs synchronization complexity.

---

**Swapchain concurrent sharing exception**

In Pass 11 we saw Kairo uses:

$$
VK_SHARING_MODE_CONCURRENT
$$

if graphics and present families differ.

This permits both families to access swapchain image without explicit ownership transfer.

It trades some optimization opportunity for simpler correctness.

---

**Synchronization2**

Classic synchronization APIs combine many old flag types and call shapes.

Synchronization2 modernizes the model with structures such as:

```cpp
VkMemoryBarrier2
VkBufferMemoryBarrier2
VkImageMemoryBarrier2
VkDependencyInfo
VkSubmitInfo2
```

and 64-bit stage/access flags.

The conceptual model remains the same.

The API becomes easier to express locally.

---

**Synchronization2 image barrier**

Conceptually:

```cpp
VkImageMemoryBarrier2 b{};

b.srcStageMask =
    VK_PIPELINE_STAGE_2_TRANSFER_BIT;

b.srcAccessMask =
    VK_ACCESS_2_TRANSFER_WRITE_BIT;

b.dstStageMask =
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

b.dstAccessMask =
    VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

b.oldLayout =
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

b.newLayout =
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
```

Then put it into:

```cpp
VkDependencyInfo
```

and call:

```cpp
vkCmdPipelineBarrier2(...);
```

This expresses stage/access pairs together instead of splitting stage masks into the outer function arguments.

---

**Why Synchronization2 is conceptually cleaner**

Old API:

```text
source stages supplied globally
destination stages supplied globally
barriers separately carry access masks
```

Synchronization2:

```text
each barrier describes:
    source stage + source access
    destination stage + destination access
```

This matches the mental model we actually want:

$$
(S_s,A_s)\rightarrow(S_d,A_d).
$$

---

**Kairo currently uses classic synchronization**

Repository search shows no:

```text
vkCmdPipelineBarrier2
VK_KHR_synchronization2
```

in current Vulkan renderer.

This fits its Vulkan 1.1 baseline.

The existing barriers are valid classic Vulkan barriers.

---

**Wait-stage masks in queue submission**

This deserves detailed attention.

Kairo waits on:

$$
imageAvailable
$$

at:

$$
VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT.
$$

This means commands in the submission may begin before acquire semaphore signals **until they reach relevant destination-stage work**.

That is more permissive than:

$$
TOP_OF_PIPE,
$$

which would effectively block the whole submission earlier.

---

**Why `COLOR_ATTACHMENT_OUTPUT` is common for swapchain acquisition**

The swapchain image is first used as a color attachment.

Earlier work such as:

- vertex processing,
- unrelated compute,
- unrelated shadow work

does not fundamentally need the acquired presentation image.

So waiting only when color-output stage needs the image can permit overlap.

---

**A subtle Kairo performance limitation**

Kairo places shadow rendering, offscreen viewport rendering and final swapchain presentation in **one primary submission**.

Its acquire semaphore waits at:

$$
COLOR_ATTACHMENT_OUTPUT.
$$

However the earlier offscreen viewport pass also uses color-attachment-output stage.

Therefore its color writes may also be held behind the swapchain acquire semaphore even though they do not touch the swapchain image.

The dependency is correct but broader than semantically necessary.

A more advanced architecture could separate:

```text
offscreen rendering submission
```

from:

```text
swapchain-dependent presentation submission
```

or use a more precise scheduling structure.

This is the difference between **correct synchronization** and **optimal synchronization**.

---

**Frames in flight**

Current Kairo:

$$
\boxed{1\text{ frame in flight}.}
$$

That means before recording frame $N+1$, CPU waits for GPU completion of frame $N$.

Timeline:

```text
CPU: record F0 ------- wait ------ record F1
GPU:          execute F0 ------- execute F1
```

Simple.

But CPU and GPU cannot run far ahead of one another.

---

**Two frames in flight**

With two frame contexts:

```text
CPU records F0
GPU begins F0

CPU records F1 while GPU executes F0

CPU waits for slot 0 only when preparing F2
```

This increases overlap.

Each frame needs separate mutable resources that cannot be overwritten while its GPU work is pending.

---

**Per-frame resources**

A frame context commonly contains:

```cpp
struct FrameContext
{
    VkCommandPool CommandPool;
    VkCommandBuffer Command;

    VkFence Fence;

    VkSemaphore ImageAvailable;
    VkSemaphore RenderFinished;

    DescriptorArena Descriptors;
    UniformRing Uniforms;

    DeferredDeletionQueue Garbage;
};
```

If:

$$
framesInFlight=3,
$$

you maintain three such contexts.

---

**Why descriptor resources must often be per-frame**

Suppose frame 0 descriptor references:

$$
uniform offset=0.
$$

Before GPU finishes, CPU updates descriptor to point to new frame-1 buffer.

If descriptor is modified while in use and feature/lifetime rules do not permit it, you have a hazard.

Simplest solution:

$$
\boxed{
\text{each in-flight frame gets immutable-for-that-frame descriptor state}.
}
$$

---

**Uniform ring buffering**

If one large host-visible uniform buffer is split into frame regions:

$$
[Frame0],
[Frame1],
[Frame2],
$$

CPU writes only current frame's region.

Region is reused after corresponding frame completion.

This avoids per-object buffer allocations and prevents CPU overwrite races.

---

**Command pool per frame**

Likewise:

```text
Frame 0 command pool
Frame 1 command pool
Frame 2 command pool
```

When frame 0 fence completes:

```cpp
vkResetCommandPool(
    frame0.pool,
    0);
```

You know every command buffer from that pool is no longer pending.

This is much cleaner than individually tracking hundreds of command buffers.

---

**Deferred destruction**

Suppose texture is deleted during frame 100.

GPU frame 98/99 may still reference it.

Do **not** immediately destroy.

Instead:

```cpp
garbage.push({
    .retireValue = submittedTimelineValue,
    .destroy = [...]
});
```

Later:

$$
completed\ge retireValue
$$

then execute destruction.

---

**Fence-based retirement**

With frame fences:

```text
resource scheduled for destruction
↓
attach to current frame's garbage list
↓
when that frame fence completes
↓
destroy
```

Works well when resource use is tied to frame boundaries.

---

**Timeline-based retirement**

More general:

Every submission gets increasing value:

$$
1,2,3,\dots
$$

Resource remembers last submission value using it:

$$
lastUse=417.
$$

Safe when:

$$
completedTimeline\ge417.
$$

This supports:

- async uploads,
- multiple queues,
- resources spanning different frame lifetimes.

---

**Current Kairo resource destruction strategy**

`DestroyMesh()` and `DestroyTexture()` currently call:

```cpp
vkDeviceWaitIdle(...)
```

before releasing resources.

This is:

$$
\boxed{\text{correct but globally blocking}.}
$$

No in-flight command can still use them after the device idle.

But you stop all GPU work.

A deferred destruction queue would eliminate that global stall.

---

**Same with viewport resize**

Current resize:

```cpp
vkDeviceWaitIdle(...)
```

then replace viewport resources.

Again, very safe.

A production multi-frame renderer would retire the old image-generation resources after their last-use fence/timeline instead.

The current code comments explicitly anticipate this future design.

---

### TIMESTAMPS, VALIDATION, DEVICE LOSS, MOLTENVK & EXPLICIT RAII

**GPU timestamp query pools**

CPU wall-clock around:

```cpp
vkQueueSubmit(...)
```

does not measure actual shader duration.

Submission is asynchronous.

For GPU time, create:

```cpp
VkQueryPool
```

with:

$$
VK_QUERY_TYPE_TIMESTAMP.
$$

Write GPU timestamps into slots during command execution.

---

**Timestamp recording**

Conceptually:

```cpp
vkCmdWriteTimestamp(
    command,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    queryPool,
    beginQuery);

DrawStuff();

vkCmdWriteTimestamp(
    command,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    queryPool,
    endQuery);
```

Modern synchronization APIs provide more precise variants/stages.

Timestamp difference gives GPU clock ticks.

---

**Convert timestamp ticks to time**

Physical-device limits provide:

$$
timestampPeriod
$$

in nanoseconds per timestamp unit.

If:

$$
\Delta ticks=
t_1-t_0,
$$

then:

$$
\boxed{
T_{ns}
=
\Delta ticks
\times
timestampPeriod.
}
$$

Milliseconds:

$$
T_{ms}
=
\frac{
\Delta ticks\times timestampPeriod
}{
10^6
}.
$$

---

**Timestamp valid bits**

Queue-family properties can expose how many timestamp bits are valid.

If only $b$ bits are valid, timestamps wrap modulo:

$$
2^b.
$$

Robust profiler respects that.

Another reason GPU profiling is more subtle than subtracting arbitrary integers.

---

**Resetting query pools**

Query indices must be reset before reuse.

Depending on Vulkan version/features, this may be performed by:

- command-buffer reset commands,
- host reset extension/core functionality.

The reset itself has lifetime/order requirements.

A per-frame query region is often easiest.

---

**Safe query readback**

This is wrong:

```cpp
submit();

vkGetQueryPoolResults(...);
```

with assumptions that GPU has finished.

Correct approaches include:

- wait for frame fence,
- read an earlier frame's queries,
- use availability bits,
- use `VK_QUERY_RESULT_WAIT_BIT` when blocking is intentionally acceptable.

For a real-time profiler, avoid immediate CPU wait.

---

**Query availability**

With availability results, each query can return:

$$
(value,available).
$$

CPU can say:

> If frame $N-2$'s timestamp results are ready, consume them; otherwise skip for now.

This prevents profiler itself from stalling rendering.

---

**Kairo currently lacks Vulkan timestamp queries**

Repository inspection found no:

```text
vkCreateQueryPool
vkCmdWriteTimestamp
vkGetQueryPoolResults
```

in current Vulkan backend.

Its `RenderGraphExecutionProfile` currently measures:

$$
\boxed{\text{CPU pass callback/recording duration}}
$$

with `std::chrono`.

That is useful CPU-side information.

It is **not GPU pass timing**.

This distinction matters enormously.

---

**What Kairo's current frame profile actually measures**

For each graph pass:

```text
Retire
Acquire
Record
Submit
Present
```

it measures the CPU duration spent executing its callback.

For example:

$$
Record=0.7\text{ ms}
$$

means CPU command recording took roughly 0.7 ms.

It does not mean the GPU rendered the scene in 0.7 ms.

A future profiling system should expose both:

```text
CPU Record: 0.7 ms
GPU Shadow: 0.4 ms
GPU Viewport: 2.8 ms
GPU Present pass: 0.2 ms
```

---

**Synchronization validation**

Standard Vulkan validation catches broad API misuse.

Synchronization validation performs hazard analysis looking for cases such as:

- RAW without dependency,
- WAW without dependency,
- incorrect layout/use,
- bad stage/access scopes.

It is one of the most valuable tools for Vulkan development.

---

**Synchronization validation does not prove correctness**

If your barrier is far too broad:

```text
ALL_COMMANDS
→ ALL_COMMANDS
```

validation may be satisfied.

But performance may be terrible.

And logical mistakes outside what the validator can infer may remain.

Validation establishes:

$$
\boxed{
\text{absence of detected API hazards}
}
$$

not:

$$
\text{optimal synchronization}.
$$

---

**Kairo currently enables standard validation layer but not explicit synchronization-validation feature configuration**

Repository search found no:

```text
VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
```

configuration.

So current validation infrastructure is a good baseline, but enabling dedicated synchronization validation during development would be valuable as the render graph begins generating native barriers.

---

**Debug utils naming**

Opaque handles are miserable in GPU debuggers:

```text
Image 0x18F2A
Buffer 0x83CB1
```

Use:

```cpp
vkSetDebugUtilsObjectNameEXT(...)
```

to label resources:

```text
Viewport HDR Color
Viewport Object ID
Directional Shadow Map
Frame 1 Uniform Ring
Gaussian Visible List
```

Then RenderDoc/Xcode/MoltenVK diagnostics become dramatically easier to understand.

---

**Kairo currently doesn't assign Vulkan debug object names**

Repository search found no:

```text
vkSetDebugUtilsObjectNameEXT.
```

This is a low-complexity, high-value addition.

Kairo already enables `VK_EXT_debug_utils` when validation is available, so adding a helper around object naming fits naturally.

---

**Debug labels**

Debug utils can also annotate command buffers with regions:

```text
Directional Shadow
Viewport Opaque
Viewport Transparent
Object-ID Readback
Presentation
```

GPU debuggers then display meaningful nested regions.

This should mirror render-graph pass names.

Your render graph already has semantic names.

The natural next step is emitting those names into native GPU debug labels.

---

**Device loss**

Vulkan commands can return:

$$
VK_ERROR_DEVICE_LOST.
$$

This means normal operation of the logical device can no longer continue.

Causes can include:

- GPU reset,
- driver failure,
- severe invalid GPU behavior,
- hardware/OS intervention.

Do not treat it like:

$$
VK_SUBOPTIMAL_KHR.
$$

You cannot simply recreate the swapchain and continue.

---

**Device-loss strategy**

When a Vulkan call returns device lost:

First preserve diagnostic context.

Record:

- operation that failed,
- last submitted frame number,
- active passes,
- pipeline/resource IDs,
- driver/device metadata,
- validation messages.

Then stop issuing normal device work.

Depending on application class, either:

- gracefully terminate renderer,
- attempt full device recreation,
- restart application/renderer subsystem.

Full recreation is a major operation because every device object becomes invalid.

---

**Device loss and fault extensions**

On supported implementations, extensions can provide extra device-fault information.

A production renderer should query optional diagnostic capabilities rather than assuming availability.

The basic engine rule is:

$$
\boxed{
\text{device loss must have an explicit failure path}.
}
$$

Do not throw generic `"vkQueueSubmit failed"` and discard the specific `VkResult`.

---

**Current Kairo loses some diagnostic specificity**

Many current calls do:

```cpp
if (vkQueueSubmit(...) != VK_SUCCESS)
    throw runtime_error(
        "vkQueueSubmit failed.");
```

This is adequate while building the renderer.

But for robust diagnostics, store the result:

```cpp
const VkResult result =
    vkQueueSubmit(...);

if (result == VK_ERROR_DEVICE_LOST)
{
    HandleDeviceLoss(...);
}
```

and include:

$$
VulkanResultText(result).
$$

The instance code already uses result-to-text for `vkCreateInstance`; that discipline can be extended.

---

**MoltenVK portability**

Your macOS Vulkan environment matters.

MoltenVK translates Vulkan semantics onto Metal.

This means:

$$
\boxed{
\text{valid Vulkan}
}
$$

does not always mean:

$$
\boxed{
\text{identical implementation behavior/performance to native desktop Vulkan}.
}
$$

Portability matters in:

- supported extensions,
- feature availability,
- queue topology,
- memory model exposure,
- performance characteristics.

---

**Portability subset**

Kairo checks:

```text
VK_KHR_portability_subset
```

during logical-device creation and enables it when exposed.

That is correct for portability implementations such as MoltenVK.

Combined with portability enumeration at instance level, this makes Kairo more robust on macOS.

---

**Queue-family expectations under MoltenVK**

Do not expect a desktop discrete-GPU topology with:

```text
graphics queue
dedicated async compute queue
dedicated DMA queue
```

to map exactly to Apple GPU hardware.

MoltenVK must map Vulkan queue abstractions into Metal command queues/encoders.

Therefore:

$$
\boxed{
\text{query capabilities and benchmark}
}
$$

rather than designing around assumed queue hardware.

---

**Vulkan barriers on Metal-backed implementation**

Vulkan may express:

```text
pipeline stage
access mask
layout transition
```

but MoltenVK translates those semantics into what Metal needs.

This is another reason to write **correct minimal Vulkan dependencies**.

Do not rely on accidental behavior of one native Vulkan driver if you want portability.

---

**Full Kairo frame — CPU/GPU synchronization trace**

Now reconstruct current frame.

At frame start:

$$
F=\text{m\_Sync.InFlight()}.
$$

CPU waits:

```cpp
vkWaitForFences(
    device,
    1,
    &F,
    VK_TRUE,
    UINT64_MAX);
```

Meaning:

$$
\boxed{
\text{previous graphics submission using reusable frame resources is complete}.
}
$$

This also makes pending viewport readbacks safe for CPU consumption.

---

**Readback completion**

After fence:

```cpp
CompleteViewportPick();
CompleteViewportCapture();
```

This ordering is essential.

The readback buffer is host-coherent, but host coherency alone did not prove GPU had completed transfer.

The fence did.

So:

$$
\boxed{
\text{Fence gives execution completion}
+
\text{coherent memory gives host visibility}.
}
$$

Both are part of the story.

---

**Acquire**

Then:

```cpp
vkAcquireNextImageKHR(
    swapchain,
    UINT64_MAX,
    imageAvailable,
    nullptr,
    &imageIndex);
```

Presentation engine eventually signals:

$$
imageAvailable.
$$

If out of date, frame aborts and swapchain recreates.

Fence has **not** been reset yet.

Correct.

---

**Fence reset**

Only after a usable acquired image:

```cpp
vkResetFences(...)
```

Now fence represents future submission completion.

This avoids the unsignaled-fence deadlock discussed earlier.

---

**Command recording begins**

`VulkanTriangle::Record()` calls:

```cpp
command.Begin();
```

which resets and begins the single primary command buffer.

Safe because we waited its previous submission fence.

Then resource CPU updates occur before/around recording:

- material descriptor rebuild if needed,
- uniform write,
- debug vertex upload.

Current host buffers are coherent.

---

**Shadow pass**

If draws exist:

```text
begin shadow render pass
clear depth to 1
bind shadow pipeline
bind descriptors
push model transform
bind vertex/index
draw indexed
end shadow pass
```

Render-pass dependency transitions depth map into:

$$
DEPTH_STENCIL_READ_ONLY_OPTIMAL.
$$

This makes shadow writes visible to later fragment shader sampling.

---

**Viewport pass**

Begin HDR/editor render pass.

Attachments:

$$
RGBA16F\ color,
$$

$$
R32_UINT\ objectID,
$$

$$
D32\ depth.
$$

Clear all.

Draw opaque/transparent meshes and debug lines.

End pass.

Final layouts:

Color:

$$
SHADER_READ_ONLY_OPTIMAL.
$$

Object ID:

$$
TRANSFER_SRC_OPTIMAL.
$$

Depth remains depth-attachment state and can be discarded because its store op is `DONT_CARE`.

---

**Optional picking**

If object pick requested:

```cpp
vkCmdCopyImageToBuffer(
    ObjectID,
    TRANSFER_SRC_OPTIMAL,
    pickReadback);
```

The viewport render-pass external dependency already established:

$$
COLOR_ATTACHMENT_WRITE
\rightarrow
TRANSFER_READ.
$$

After the submission fence signals next frame, CPU reads one uint32 object ID.

No immediate GPU wait at request time.

Good asynchronous pattern.

---

**Optional HDR capture**

If capture requested:

Color image transitions:

$$
SHADER_READ_ONLY
\rightarrow
TRANSFER_SRC.
$$

Copy whole HDR image to host-visible buffer.

Then transition back:

$$
TRANSFER_SRC
\rightarrow
SHADER_READ_ONLY.
$$

Later frame fence signals.

CPU reads half-float pixels.

No immediate `vkDeviceWaitIdle()`.

This is a much better readback architecture than synchronous screenshot capture.

---

**Swapchain render pass**

Then current acquired swapchain framebuffer is begun.

Clear background.

Set viewport/scissor.

Tooling overlay records commands.

End render pass.

Render pass transitions swapchain image to:

$$
PRESENT_SRC_KHR.
$$

Command buffer ends.

---

**Submission**

Graphics queue submission waits on:

$$
imageAvailable
$$

at:

$$
COLOR_ATTACHMENT_OUTPUT.
$$

Executes command buffer.

Signals:

$$
renderFinished[imageIndex].
$$

Associates:

$$
inFlight fence.
$$

GPU can now execute asynchronously.

---

**Presentation**

`vkQueuePresentKHR()` waits on:

$$
renderFinished[imageIndex].
$$

Therefore presentation engine will not use image before rendering is done.

If present returns out-of-date/suboptimal, swapchain recreation happens.

Current recreation:

$$
vkDeviceWaitIdle
$$

then destroys/recreates dependent swapchain state.

Correct, though coarse.

---

**Current Kairo frame dependency diagram**

```text
CPU
│
├─ wait previous inFlight fence
│      ↓
│   read previous pick/capture
│
├─ acquire swapchain image
│      ↓ signals imageAvailable asynchronously
│
├─ reset fence
│
├─ record primary command buffer
│      ├─ shadow pass
│      ├─ viewport pass
│      ├─ optional readback copies
│      └─ swapchain pass
│
├─ queue submit
│      wait imageAvailable
│      execute command
│      signal renderFinished[image]
│      signal inFlight fence on completion
│
└─ queue present
       wait renderFinished[image]
```

This is a complete valid explicit frame.

---

**What one-frame-in-flight buys Kairo**

A huge amount of complexity disappears.

Because every frame starts with:

$$
vkWaitForFences(previousFrame),
$$

Kairo can safely reuse:

- one primary command buffer,
- one camera uniform buffer,
- one acquire semaphore,
- descriptor state,
- debug buffer,
- pick buffer.

No CPU writes can overlap prior GPU reads because there is no prior GPU frame still alive.

Correctness is exceptionally easy.

---

**What it costs**

CPU and GPU overlap is limited.

Every frame has a mandatory CPU wait for previous GPU frame.

If GPU frame takes:

$$
12\text{ ms}
$$

and CPU frame work would take:

$$
4\text{ ms},
$$

the CPU often cannot use those 4 ms productively to prepare the next frame while GPU runs.

Two/three frame contexts can improve throughput.

---

**Converting Kairo to three frames in flight**

Architecturally:

```cpp
struct VulkanFrameContext
{
    VulkanCommandPool Pool;
    VkCommandBuffer Command;

    VkFence Completion;

    VkSemaphore ImageAvailable;

    UniformRing Uniforms;
    DescriptorArena Descriptors;

    std::vector<DeferredDestroy>
        Garbage;
};
```

Create:

$$
frames[3].
$$

Current frame:

$$
F=
frames[
frameNumber\bmod3
].
$$

Wait only:

$$
F.Completion.
$$

Do not wait other frames.

---

**Present semaphores need special thought**

Naively placing one `renderFinished` semaphore per frame can be dangerous because CPU frame fence signals graphics completion, not necessarily completion of presentation wait consumption.

A robust pattern is the one Kairo already uses:

$$
\boxed{
renderFinished\ semaphore\ per\ swapchain\ image.
}
$$

Then reuse occurs naturally when same image is reacquired.

Keep that design when expanding frames in flight.

---

**Per-frame camera uniform**

Current one global camera uniform becomes:

```text
frame 0 uniform region
frame 1 uniform region
frame 2 uniform region
```

or separate buffers.

CPU writes only current frame.

GPU may simultaneously read previous frame.

This removes write-after-read races.

---

**Descriptor architecture for multiple frames**

Static material texture descriptors need not be duplicated if they never mutate in flight.

Dynamic per-frame resources do.

A sensible set architecture:

```text
Set 0 — frame
    camera/lights/environment frame state

Set 1 — material
    mostly immutable textures/material data

Set 2 — object / bindless indices
```

Then only frame set rotates per frame.

This scales better than duplicating every descriptor blindly.

---

**Correct Vulkan RAII**

Now the final curriculum part.

Naive RAII:

```cpp
class Buffer
{
public:
    ~Buffer()
    {
        vkDestroyBuffer(
            device,
            buffer,
            nullptr);
    }
};
```

This solves CPU handle ownership.

It does **not** solve GPU lifetime.

If destructor runs while command buffer still uses buffer:

$$
\boxed{\text{undefined behavior}.}
$$

---

**RAII has two lifetime axes**

CPU ownership:

$$
\boxed{
\text{Who calls }vkDestroyX?
}
$$

GPU ownership:

$$
\boxed{
\text{When is }vkDestroyX\text{ allowed?}
}
$$

A good Vulkan abstraction must preserve both.

---

**The wrong abstraction: hidden `vkDeviceWaitIdle()`**

You could make every destructor:

```cpp
~Texture()
{
    vkDeviceWaitIdle(device);
    Destroy();
}
```

Now lifetime is always safe.

But every destruction stalls the entire GPU.

The abstraction technically “works” while destroying explicit Vulkan performance semantics.

This is exactly what Part 2300 warns against.

---

**Better RAII: explicit retirement**

Resource wrapper owns native handles.

When high-level resource is released:

```cpp
renderer.Retire(
    std::move(texture),
    lastUseValue);
```

Retirement queue keeps ownership.

When GPU timeline reaches value:

$$
completed\ge lastUseValue,
$$

queue finally destroys it.

Thus C++ owner can logically release resource now while native bytes survive until GPU safety is proven.

---

**Example deferred destruction queue**

```cpp
class DeferredDeletionQueue
{
    struct Entry
    {
        uint64_t RetireAfter;
        std::function<void()> Destroy;
    };

    std::deque<Entry> m_Entries;

public:
    void Retire(
        uint64_t value,
        std::function<void()> destroy)
    {
        m_Entries.push_back(
            {
                value,
                std::move(destroy)
            });
    }

    void Collect(
        uint64_t completedValue)
    {
        while (!m_Entries.empty() &&
               m_Entries.front()
                       .RetireAfter
                   <= completedValue)
        {
            m_Entries.front()
                .Destroy();

            m_Entries.pop_front();
        }
    }
};
```

The critical point is not this exact container.

It is that destruction occurs only after explicit completion evidence.

---

**Resource wrapper can remain move-only**

```cpp
class VulkanBuffer
{
    VkDevice m_Device = VK_NULL_HANDLE;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory =
        VK_NULL_HANDLE;

public:
    VulkanBuffer(
        const VulkanBuffer&) = delete;

    VulkanBuffer&
    operator=(
        const VulkanBuffer&) = delete;

    VulkanBuffer(
        VulkanBuffer&& other)
        noexcept
    {
        *this =
            std::move(other);
    }

    // Destruction assumes wrapper itself owns
    // only already-retired native resources.
};
```

Move-only RAII is good.

The mistake is letting ordinary C++ destruction implicitly decide GPU retirement timing.

---

**Separate logical resource from native retired payload**

A powerful architecture:

```text
TextureHandle
    ↓
Renderer resource registry
    ↓
NativeTexture generation

DestroyTexture(handle)
    ↓
remove from future scene use
    ↓
move NativeTexture to deferred queue
    ↓
destroy after timeline completion
```

This cleanly separates:

$$
\text{logical lifetime}
$$

from:

$$
\text{physical GPU lifetime}.
$$

---

**Generation IDs**

Your `VulkanViewportTexture` already uses a:

$$
Generation
$$

counter when resize replaces views.

That is excellent.

Borrowing systems can compare:

$$
cachedGeneration
\neq
currentGeneration
$$

and know their descriptor/reference is stale.

Generation IDs are a powerful defense against handle reuse/stale-resource bugs.

---

**Explicit synchronization should remain visible in abstractions**

Avoid wrapper:

```cpp
Texture::MakeReadable();
```

if nobody can tell what stage/access dependency it emits.

Prefer something semantically explicit such as:

```cpp
TransitionImage(
    command,
    image,

    {
        .srcStage =
            Transfer,

        .srcAccess =
            TransferWrite,

        .dstStage =
            FragmentShader,

        .dstAccess =
            ShaderSampledRead,

        .oldLayout =
            TransferDst,

        .newLayout =
            ShaderReadOnly
    });
```

The helper removes boilerplate but preserves synchronization meaning.

That is good abstraction.

---

**Render graph can become the synchronization owner**

Rather than manually author every barrier:

```text
Pass A:
    writes ShadowMap as DepthAttachment

Pass B:
    reads ShadowMap as Sampled
```

render graph compiler derives:

$$
DEPTH_WRITE
\rightarrow
SHADER_READ.
$$

Then backend maps to Vulkan barrier/subpass dependency.

This is why your existing RenderGraph is strategically important.

It already tracks logical hazards and state transitions.

Phase 24 will take this much further.

---

**Validation + render graph**

A very strong development loop is:

```text
render graph
    derives dependency

Vulkan backend
    emits native barrier

synchronization validation
    verifies no detectable hazard

GPU timestamp profiler
    measures whether dependency is too broad
```

Correctness and performance become two separate feedback channels.

Exactly what an explicit renderer needs.

---

**Debug-name everything the render graph creates**

If graph knows resource name:

```text
ViewportColor
ShadowDepth
ObjectID
TAAHistory
```

then Vulkan backend should name native resources with those strings.

If graph knows pass name:

```text
Shadow
Viewport
Capture
Present
```

emit debug command labels.

Now RenderDoc/Xcode captures reflect the same vocabulary as your engine.

That dramatically reduces cognitive load.

---

**Robust Vulkan result handling**

Instead of:

```cpp
if (vkCall() != VK_SUCCESS)
    throw;
```

prefer:

```cpp
const VkResult result =
    vkCall();

switch (result)
{
case VK_SUCCESS:
    break;

case VK_ERROR_DEVICE_LOST:
    HandleDeviceLoss();
    break;

case VK_ERROR_OUT_OF_DATE_KHR:
    RecreateSwapchain();
    break;

default:
    ThrowVulkanError(result);
}
```

Not every non-success has the same recovery semantics.

Explicit APIs deserve explicit error classification.

---

**Current Kairo phase audit**

| System | Current state |
|---|---|
| Command pool | One graphics-family pool |
| Command buffers | One reusable primary |
| Secondary buffers | Not currently used |
| Reset | Per command buffer |
| Render scopes | Traditional render passes |
| Pipeline binding | Yes |
| Vertex/index binding | Yes |
| Descriptor binding | Yes |
| Draw | Yes, debug lines |
| Indexed draw | Yes, meshes/shadows |
| Compute dispatch | Conceptually supported by Vulkan, not central in inspected Kairo path |
| Queue submit | One graphics submit/frame |
| Fence | One in-flight fence |
| Binary acquire semaphore | One |
| Binary render-finished | One per swapchain image |
| Timeline semaphore | Not currently used |
| Classic barriers | Yes |
| Synchronization2 | Not currently used |
| Image transitions | Explicit + render-pass automatic |
| Queue ownership transfer | Avoided by current design |
| Frames in flight | One |
| Deferred destruction | Not yet; uses `vkDeviceWaitIdle()` at destructive boundaries |
| Async pick readback | Yes |
| Async HDR capture readback | Yes |
| Timestamp queries | Not currently used |
| CPU render-graph profiling | Yes |
| Sync validation feature | Not explicitly enabled |
| Debug object naming | Not currently used |
| Device-loss strategy | Not specialized yet |
| Portability enumeration | Yes |
| Portability subset | Yes |
| MoltenVK-aware architecture | Yes |
| Hazard-aware generic RenderGraph | Yes |
| Native graph-driven Vulkan barriers | Future integration point |

---

**The three synchronization questions**

Whenever you see a Vulkan hazard, ask in this order.

**Who produced the data?**

Example:

$$
ColorAttachmentWrite.
$$

**Who consumes it?**

Example:

$$
TransferRead.
$$

**What is the resource state/lifetime transition?**

Example:

$$
ShaderReadOnly
\rightarrow
TransferSrc.
$$

Then formulate:

$$
(S_s,A_s,L_s)
\rightarrow
(S_d,A_d,L_d).
$$

This is the most useful synchronization equation in the whole phase.

---

**Example: texture upload**

Producer:

$$
S_s=TRANSFER
$$

$$
A_s=TRANSFER_WRITE.
$$

Consumer:

$$
S_d=FRAGMENT_SHADER
$$

$$
A_d=SHADER_READ.
$$

Layout:

$$
TRANSFER_DST
\rightarrow
SHADER_READ_ONLY.
$$

Therefore:

$$
\boxed{
(TRANSFER,TRANSFER_WRITE,TRANSFER_DST)
\rightarrow
(FRAGMENT,SHADER_READ,SHADER_READ_ONLY)
}
$$

That is the barrier.

---

**Example: compute culling → indirect draw**

Compute writes indirect commands.

Producer:

$$
COMPUTE_SHADER
+
SHADER_WRITE.
$$

Consumer:

$$
DRAW_INDIRECT
+
INDIRECT_COMMAND_READ.
$$

Buffer has no image layout.

Barrier:

$$
\boxed{
(COMPUTE,SHADER_WRITE)
\rightarrow
(DRAW_INDIRECT,INDIRECT_READ)
}
$$

This exact reasoning will matter when Kairo gets GPU-driven rendering.

---

**Example: compute writes vertex data**

Producer:

$$
COMPUTE_SHADER
+
SHADER_WRITE.
$$

Consumer:

$$
VERTEX_INPUT
+
VERTEX_ATTRIBUTE_READ.
$$

Not:

$$
VERTEX_SHADER
$$

unless shader itself reads the buffer as a descriptor.

Pipeline stage must correspond to the actual consumer operation.

This is a common synchronization mistake.

---

**Example: depth prepass → fragment sampling**

Producer:

$$
LATE_FRAGMENT_TESTS
+
DEPTH_STENCIL_ATTACHMENT_WRITE.
$$

Consumer:

$$
FRAGMENT_SHADER
+
SHADER_SAMPLED_READ.
$$

Layout:

$$
DEPTH_STENCIL_ATTACHMENT
\rightarrow
DEPTH_STENCIL_READ_ONLY.
$$

Again, describe actual producer/consumer.

---

**Why “ALL_COMMANDS” hides understanding**

If you write:

```text
ALL_COMMANDS
ALL_MEMORY
→
ALL_COMMANDS
ALL_MEMORY
```

you no longer know:

- who produced resource,
- who consumes it,
- whether graph dependencies are correct.

The program may work, but architecture has lost semantic information.

Precise synchronization is both a performance tool and a correctness-documentation tool.

---

**Phase-23 mental model**

Vulkan is now no longer:

> API with thousands of structs.

It is four explicit systems.

**Resource system**

$$
Buffer/Image
+
Memory
+
Views/Descriptors.
$$

**Execution system**

$$
CommandBuffer
\rightarrow
Queue.
$$

**Dependency system**

$$
Stages
+
Accesses
+
Barriers
+
Semaphores/Fences.
$$

**Lifetime system**

$$
Frames
+
Completion values
+
Deferred retirement.
$$

Every Vulkan object fits primarily into one of those systems.

---

**PASS 12 RECALL SHEET**

| Concept | Reconstruction |
|---|---|
| Command pool | Queue-family-associated command allocator |
| Primary command buffer | Directly queue-submittable command stream |
| Secondary command buffer | Executed from primary, useful for parallel recording |
| Command lifecycle | Initial → Recording → Executable → Pending → reusable |
| Reset rule | Never reset while pending |
| Render scope | Region where render-pass/dynamic-rendering attachment contract is active |
| Bind pipeline | Select shader/fixed-state program |
| Bind vertex/index | Select geometry data |
| Bind descriptors | Connect shader bindings to resources |
| Draw | Non-indexed vertex execution |
| DrawIndexed | Index-buffer-driven primitive execution |
| Dispatch | Compute workgroup launch |
| Queue submit | Schedule recorded work |
| Fence | GPU→CPU completion |
| Binary semaphore | GPU/queue dependency token |
| Timeline semaphore | Monotonic value-based dependency/completion primitive |
| Pipeline stage | Where execution occurs |
| Access mask | What memory operation occurs |
| Execution dependency | Orders execution scopes |
| Memory dependency | Makes writes available/visible to later accesses |
| Pipeline barrier | Command-buffer dependency |
| Buffer barrier | Dependency for buffer range/ownership |
| Image barrier | Dependency + possible layout/ownership transition |
| Image layout | Image usage/state optimization contract |
| Queue ownership | Exclusive resource family ownership |
| Synchronization2 | Stage/access-paired modern sync API |
| Frame in flight | Submitted frame whose GPU work has not retired |
| Per-frame resources | Mutable state isolated by frame slot |
| Deferred destruction | Destroy only after last GPU use completes |
| Query pool | GPU-side measurements/results storage |
| Timestamp period | Nanoseconds per GPU timestamp tick |
| Safe timestamp read | Read only after availability/completion |
| Sync validation | Validation-layer hazard analysis |
| Debug name | Human-readable native GPU object identity |
| Device lost | Device no longer usable for ordinary rendering |
| MoltenVK | Vulkan portability implementation over Metal |
| Explicit RAII | CPU ownership without hiding GPU synchronization/lifetime |
| Current Kairo | One frame, classic sync, binary semaphores, async readback, device-idle destruction |
| Future Kairo | Frames in flight + timeline retirement + native graph barriers + GPU timestamps |

---

**Blank-page mastery gate**

From nothing, reconstruct the state machine of a command buffer and explain exactly why this is illegal:

```cpp
vkQueueSubmit(queue, command, fence);

vkResetCommandBuffer(command, 0);
```

unless completion has first been proven.

Then explain the difference between:

$$
\boxed{\text{fence}}
$$

and:

$$
\boxed{\text{semaphore}}.
$$

A fence tells the CPU that submitted GPU work completed.

A semaphore builds asynchronous execution dependencies.

Then derive Kairo's frame:

$$
\text{wait fence}
\rightarrow
\text{acquire}
\rightarrow
\text{reset fence}
\rightarrow
\text{record}
\rightarrow
\text{submit}
\rightarrow
\text{present}.
$$

Explain why resetting the fence **before** acquisition could deadlock after an out-of-date swapchain result.

Then explain why Kairo's acquire semaphore waits at:

$$
COLOR_ATTACHMENT_OUTPUT
$$

rather than `TOP_OF_PIPE`, and also identify why its single combined command submission can make that dependency broader than necessary for the earlier offscreen pass.

Then derive the synchronization tuple:

$$
\boxed{
(S_s,A_s,L_s)
\rightarrow
(S_d,A_d,L_d)
}
$$

for all four cases:

$$
\text{texture upload}
\rightarrow
\text{fragment sampling},
$$

$$
\text{shadow depth write}
\rightarrow
\text{shadow-map sampling},
$$

$$
\text{viewport color write}
\rightarrow
\text{readback transfer},
$$

$$
\text{compute-generated indirect commands}
\rightarrow
\text{indirect drawing}.
$$

Then explain why:

$$
HOST_COHERENT
$$

does not mean the CPU can read a GPU-written mapped buffer before its fence completes.

Then distinguish:

$$
\text{execution dependency}
$$

from:

$$
\text{memory dependency}.
$$

Then distinguish:

$$
\text{image layout}
$$

from:

$$
\text{access mask}.
$$

Then explain why a resource may have layout `SHADER_READ_ONLY_OPTIMAL` at a barrier while the source access being synchronized is still:

$$
COLOR_ATTACHMENT_WRITE.
$$

That one question proves whether you actually understand Vulkan image synchronization.

Then convert Kairo from one frame in flight to three on paper:

$$
frameIndex\bmod3
$$

with per-frame:

- command pool/buffer,
- fence,
- acquire semaphore,
- dynamic descriptors,
- uniform ring,
- deletion queue,

while keeping present-wait semaphores safely associated with swapchain images.

Then design timeline retirement:

$$
resource.lastUse=417
$$

and destroy only when:

$$
completedTimeline\ge417.
$$

Then explain why this is a terrible destructor:

```cpp
~GpuTexture()
{
    vkDeviceWaitIdle(device);
    DestroyNativeHandles();
}
```

even though it is “safe.”

Finally reconstruct the complete Kairo frame at the hardware-dependency level:

$$
\boxed{
\begin{aligned}
&\text{Fence}_{N-1}\text{ complete}\\
&\downarrow\\
&\text{CPU consumes previous readback}\\
&\downarrow\\
&\text{Acquire image}\\
&\downarrow\\
&\text{Record shadow pass}\\
&\downarrow\\
&\text{Depth write}\rightarrow\text{fragment sample}\\
&\downarrow\\
&\text{Record HDR/object-ID viewport}\\
&\downarrow\\
&\text{Optional transfer readback}\\
&\downarrow\\
&\text{Record swapchain pass}\\
&\downarrow\\
&\text{Submit waiting on imageAvailable}\\
&\downarrow\\
&\text{Signal renderFinished + fence}\\
&\downarrow\\
&\text{Present waits renderFinished}
\end{aligned}
}
$$

If you can explain which object proves **execution completion**, which dependency proves **memory visibility**, which object proves **presentation ordering**, and which mechanism proves **resource destruction safety**, then Vulkan synchronization has stopped being “barrier magic.”

It has become a dependency graph you can reconstruct.




That starts **Phase 24 — Metal, Render Graphs, Resource Lifetimes, Apple GPU Concerns, and Multi-Backend Rendering**: Metal's object model and command queues/buffers/encoders, buffers/textures/samplers/resource options, render/compute pipelines and argument buffers, drawable/present flow, Metal synchronization and resource hazards, then render-graph DAG construction, logical resource state, pass dependencies, lifetime analysis, transient-resource aliasing, barrier generation, pass culling, queue scheduling, and finally the Apple GPU/tile-based architecture consequences that explain *why* Metal wants you to structure rendering differently from a desktop immediate-memory GPU.



### METAL 4 IS A NEW SUBMISSION MODEL, NOT A RENAME OF OLD METAL

The preceding Metal model remains essential background, but Metal 4 changes ownership and submission around several core objects. The MTL4 family introduces a new command queue, command buffer, encoders, command allocator, argument table and barrier model intended to reduce CPU overhead and make multithreaded encoding/reuse more explicit.

A useful contrast is:

| Earlier Metal | Metal 4 |
|---|---|
| command buffer normally created from queue | command buffer created from device |
| command buffer commits itself | queue commits command buffers |
| command buffers normally short-lived/one-use | command buffers can be reused |
| queue tightly associated with buffer creation | compatible MTL4 queue can submit the buffer |
| command storage mostly implicit | command allocator owns encoding storage |
| direct slots / argument buffers | argument tables add a newer binding path |

The architectural consequence is that **recorded work and the queue that submits it are less tightly coupled**.

#### Command allocators make command-memory lifetime explicit

A reusable command buffer still needs memory that stores encoded commands. Metal 4 exposes this through MTL4CommandAllocator.

A frame ring becomes:

~~~
allocator[0]
allocator[1]
allocator[2]

choose frame slot
    ↓
ensure GPU no longer uses slot
    ↓
reset allocator
    ↓
begin/reuse command buffer
    ↓
encode
    ↓
submit
~~~

The invariant is:

\[
\boxed{
\text{never reset command storage while GPU work can still reference it}
}
\]

That invariant belongs in frame-lifetime management, not in ad-hoc rendering calls.

#### Reusable command buffers separate object identity from recorded-work identity

Old Metal code often creates one short-lived command buffer per frame. Metal 4 allows the command buffer object to persist while the encoded command sequence changes each time it begins with a new allocator.

So distinguish:

\[
\boxed{
\text{command-buffer object identity}
\neq
\text{command-storage epoch}
\neq
\text{GPU completion epoch}
}
\]

A persistent C++ wrapper must not imply that the commands inside the object are persistent.

#### Queue commit becomes an explicit batching point

Metal 4 queues can commit multiple command buffers together.

~~~
worker A → command buffer A
worker B → command buffer B
worker C → command buffer C

submission phase:
    establish dependency/order
    commit group
~~~

Parallel CPU encoding and parallel GPU execution are different. The engine still needs resource-access knowledge, ordering, barriers/events, presentation dependencies and allocator lifetime safety.

The gain is that workers can encode independently without each worker owning a queue.

#### Argument tables push binding toward shader-visible tables

Argument buffers already package Metal resource references. Metal 4 adds argument tables to the new core API family.

A scalable renderer should therefore think in terms of shader-visible resource tables:

~~~
FrameTable:
    camera
    lighting
    global buffers

MaterialTable:
    texture handles
    material scalars

ObjectTable:
    transform
    mesh/material indices
~~~

A backend can map that model to MTL4 argument tables, older argument buffers/direct slots, Vulkan descriptor indexing, or another API's binding system.

Do not infer that one giant global table is always best. Update frequency, lifetime, visibility and capacity still determine table structure.

#### Residency sets expose the working set

Metal 4 queues can associate residency sets with submitted work. Residency asks:

> Which resources must be available to the GPU while this workload executes?

Unified memory does not make this irrelevant. The runtime still manages mappings, working sets, compression state, heaps and residency pressure.

A render graph can conceptually derive:

~~~
selected passes
    ↓
resources touched
    ↓
working set
    ↓
residency decisions
~~~

This becomes important for large streamed scenes, ray-tracing acceleration structures and tensor-heavy workloads.

#### Next-generation barriers should come from graph dependencies

The cross-backend truth is:

\[
\boxed{
\text{producer access}
\rightarrow
\text{dependency}
\rightarrow
\text{consumer access}
}
\]

If ComputeCull writes VisibleDraws and GraphicsMain reads it, the render graph owns that fact. Vulkan and Metal encode it differently. The engine should not own a function named “insert Metal barrier because Metal needs one”; it should own the producer/consumer relation.

#### Metal Shading Language 4 expands the GPU programming model

The current MSL specification includes:

- argument buffers,
- uniform values/control flow,
- visible function tables,
- ray-tracing types/intersectors,
- object and mesh shader types,
- imageblocks,
- SIMD-group and quad-group operations,
- Metal-specific atomics and scopes,
- tensor and cooperative-tensor types,
- Metal Performance Primitives tensor operations.

The tensor additions matter because rendering, reconstruction, denoising and ML inference now coexist in one GPU programming environment.

An ordinary tensor represents multidimensional data with extents/strides. A cooperative tensor partitions its elements across participating threads according to a device-oriented layout. The correct mental model is therefore a collective data structure:

\[
\boxed{
\text{one logical tensor operation distributed across lanes}
}
\]

not “every thread owns a complete little matrix.”

#### Uniformity is an execution property

A uniform value is logically identical across the relevant participating execution scope, while a varying value may differ per lane. Uniformity affects branch convergence, resource indexing, subgroup operations and compiler reasoning.

Do not confuse this with “uniform buffer” terminology from other APIs. One describes value/control-flow behavior; the other describes a resource class.

#### MSL atomics resemble C++ vocabulary but have Metal rules

The specification exposes a subset of C++17-like atomic concepts, but allowed orders, scopes and address spaces are Metal-specific.

For every GPU synchronization operation, check:

~~~
operation
address space
memory order
thread scope
barrier/fence semantics
~~~

Do not copy a host-side C++ memory-model intuition merely because enum names look familiar.

#### Capabilities should drive backend paths

Do not reduce feature selection to:

~~~
if Metal4 then use all new features
~~~

Resolve capabilities such as:

~~~
MTL4 command path
argument tables
ray tracing
mesh shaders
tensor operations
dynamic libraries
GPU family constraints
~~~

and print them in startup diagnostics. “Works on my M-series chip” is not a reproducible capability report.

Source basis: Apple's current Metal 4 documentation and Metal Shading Language Specification v4 dated 2025-10-23. The frame-ring, render-graph and cross-backend interpretations are integrations into this engine narrative.



### Metal 4 deepening: argument tables, residency, barriers, and multi-threaded encoding are one resource protocol

The first Metal 4 section establishes the new objects. The deeper architectural point is that these objects are not independent conveniences. Together they move more of the renderer's hidden resource bookkeeping into explicit application-visible structures.

The useful way to read the Metal 4 core API is as four linked questions:

\[
\boxed{
\begin{aligned}
&\text{Where are commands stored?} &&\rightarrow \text{command allocator}\\
&\text{What resources can shaders address?} &&\rightarrow \text{argument table}\\
&\text{What resources must be GPU-visible?} &&\rightarrow \text{residency set}\\
&\text{What earlier work must precede later work?} &&\rightarrow \text{barrier/dependency}
\end{aligned}
}
\]

A renderer that answers only one of those questions is incomplete.

#### Argument tables are a root-binding layer, not a replacement for all resource indirection

Metal 4 argument tables store bindings for buffers, textures, samplers and other resources. The key performance idea is that table population can happen outside the draw-critical encoding path and the same table can be applied to several encoder stages.

A scene renderer can therefore separate slow-changing binding structure from per-draw identity.

~~~text
GlobalTable
    frame constants
    scene buffers
    bindless material table
    light lists

MaterialTable
    texture table / argument-buffer root
    material constants

DrawRecord
    mesh index
    material index
    transform index
~~~

The draw call does not need to re-describe every resource that the shader may touch.

However, argument tables should not be interpreted as permission to place every resource in one immortal global namespace. The engine still has to reason about:

- update frequency,
- frame overlap,
- slot reuse,
- table capacity,
- resource destruction,
- shader-stage visibility,
- residency,
- transient resources.

A good table hierarchy reflects lifetime domains.

\[
\boxed{
\text{binding domain should usually follow resource lifetime/update domain}
}
\]

This is the same reason a frame table, material table and scene table are often better than one mutable mega-table.

#### Bindless resource identity still needs generation safety

Suppose a texture occupies table slot 731. If the texture is destroyed and slot 731 is immediately reused, an in-flight draw referring to the old slot can silently sample the new resource.

The index is therefore not enough.

The CPU-side identity can be:

~~~cpp
struct TextureHandle {
    std::uint32_t index;
    std::uint32_t generation;
};
~~~

while the shader-visible table uses the compact index only after the CPU has guaranteed that the index cannot be recycled until the last dependent GPU epoch completes.

This yields two separate correctness layers:

~~~text
CPU handle:
    detects stale logical references

GPU table slot:
    remains physically stable while in flight
~~~

The deferred-reuse rule is:

\[
\boxed{
\text{recycle slot only after completedEpoch}
\ge
\text{lastEpochThatCanReadSlot}
}
\]

That same invariant applies to Vulkan descriptor arrays, descriptor buffers, Metal argument tables, and custom GPU-visible heaps.

#### Residency is the physical counterpart to logical binding

A shader can logically refer to a resource through a table while the runtime still needs to know that the resource must be available to the GPU.

Metal 4 makes this explicit through residency sets.

The render graph already has almost enough information to derive residency:

~~~text
selected pass
    ↓
declared reads/writes
    ↓
concrete buffers/textures/pipelines
    ↓
resources needed during submission
    ↓
residency set
~~~

The difficult cases are not ordinary static scene resources. They are:

- streamed textures arriving mid-frame,
- sparse resources with partial backing,
- large acceleration structures,
- transient heap aliases,
- machine-learning tensors,
- resources created by one worker while another worker encodes commands,
- resources removed from the scene while prior frames remain in flight.

A residency system should therefore distinguish logical liveness from physical GPU liveness.

~~~text
logical live:
    scene may reference resource

GPU live:
    submitted/in-flight work may still reference resource

resident:
    runtime must make resource available to GPU
~~~

These are related but not identical states.

#### Residency sets and streaming need a publication point

Imagine a streaming thread creates a texture, fills it, and adds it to a residency set while the render thread is encoding.

The renderer must define when the new resource becomes visible to scene tables.

A safe conceptual sequence is:

~~~text
IO/decode
    ↓
resource creation
    ↓
upload
    ↓
upload completion known
    ↓
add/update residency information
    ↓
publish table slot at a frame-safe point
    ↓
future draw may reference slot
~~~

Publishing earlier can expose a resource whose contents are not ready. Publishing table identity without residency can expose a resource the GPU cannot access. Reusing a table slot too early can expose a different resource to an older frame.

This is why asset streaming is ultimately a synchronization protocol.

#### Queue barriers should be derived from resource hazards

Apple describes Metal 4 queue barriers as a scalable way to express dependencies. The engine should not think in terms of manually sprinkling barrier calls.

Instead define a resource edge:

~~~text
Pass A:
    writes VisibilityBuffer

Pass B:
    reads VisibilityBuffer
~~~

That edge carries four pieces of information:

\[
(\text{resource},\text{producer access},\text{consumer access},\text{execution order})
\]

The Metal backend maps the edge to the appropriate Metal 4 dependency mechanism.

The Vulkan backend maps the same edge to stage/access masks, barriers and potentially semaphore dependencies.

This is exactly why the render graph is more than an ordering graph. It is a resource-hazard compiler.

#### A barrier is not a lifetime guarantee

A subtle but important distinction:

~~~text
barrier:
    constrains visibility/order of accesses

completion:
    proves GPU has finished using a resource

lifetime:
    determines whether memory/object may be destroyed or reused
~~~

A barrier between two passes does not mean the CPU can destroy the resource after recording the second pass. The resource may still be referenced by queued or executing GPU work.

The correct chain is:

\[
\boxed{
\text{hazard ordering}
\neq
\text{submission completion}
\neq
\text{resource retirement}
}
\]

This distinction should appear in backend abstractions. Otherwise one generic word such as sync ends up hiding three different problems.

#### Command allocators create an allocator epoch

The command allocator should be treated similarly to a frame arena.

Its memory belongs to one encoding/submission epoch. Resetting it is safe only after all command buffers that use that storage are no longer in flight.

For triple buffering:

~~~text
slot 0 allocator → frame N
slot 1 allocator → frame N+1
slot 2 allocator → frame N+2
slot 0 cannot reset until frame N completed
~~~

The correct frame object therefore owns more than a drawable index:

~~~cpp
struct FrameContext {
    MTL4CommandAllocatorHandle commandAllocator;
    UploadArena uploadArena;
    DescriptorOrArgumentScratch bindingScratch;
    CompletionEpoch retireEpoch;
};
~~~

The exact wrapper types vary, but the architectural rule is that all frame-scoped CPU/GPU resources retire together under an explicit completion condition.

#### Reusable command-buffer objects do not imply reusable encoded commands

Metal 4 can reuse a command-buffer object while replacing its encoded contents using a new allocator epoch.

That means three identities must stay separate:

~~~text
wrapper object identity
command storage identity
submission identity
~~~

A profiler/debugger should log submission identity, not merely the wrapper pointer, or multiple frames will appear to be the same command buffer.

A useful debug label is:

~~~text
Frame 481 / Main / Submission 1437
~~~

rather than only:

~~~text
MainCommandBuffer
~~~

#### Suspended/resumed render passes change how parallel encoding is partitioned

Metal 4 can suspend a render pass in one command buffer and resume it in another. This provides a different path to multithreaded encoding than tying all worker encoders to a single parallel-render-encoder object.

The engine can think in logical chunks:

~~~text
RenderPass: GBuffer
|
+-- chunk A: opaque batch 0
+-- chunk B: opaque batch 1
+-- chunk C: opaque batch 2
+-- chunk D: terrain
~~~

Workers can encode chunks independently, while the renderer preserves one logical attachment lifetime.

The important warning is that parallel CPU encoding does not automatically produce parallel GPU execution. All chunks writing the same attachments still obey the ordering and attachment semantics of that logical render pass.

#### Attachment maps separate logical shader outputs from physical attachment slots

Metal 4's attachment mapping allows one encoder to map logical shader outputs to physical color attachments more flexibly.

That is useful when a render graph wants one shader/pipeline family to target different physical attachments across pass variants.

The cross-backend abstraction should therefore represent semantic outputs:

~~~text
Color
Normal
Material
Velocity
ObjectId
~~~

and let the backend map those to concrete attachment slots/formats.

This reduces pipeline explosion caused solely by attachment wiring differences.

#### Texture view pools and placement sparse resources belong to the memory system

Texture views are metadata objects that reinterpret or select portions of underlying texture storage. Large engines may create many views for mips, layers, formats, streaming tiles, or transient passes.

A view pool reduces repeated allocation pressure.

Placement sparse resources go deeper: they separate virtual resource shape from physical backing. The useful mental model is:

\[
\text{virtual texture address space}
\quad\neq\quad
\text{currently backed physical pages}
\]

This is valuable for huge textures, streamed worlds, and memory-budgeted content, but it means the engine must track residency at subresource/page granularity when it chooses to exploit sparsity.

Sparse resources are therefore not just a texture feature; they extend the resource-state machine.

#### Metal 4 machine-learning encoding strengthens the need for one graph of heterogeneous GPU work

Metal 4 includes a machine-learning command encoder alongside render and compute encoders.

A modern graphics engine can therefore have a frame such as:

~~~text
visibility compute
    ↓
raster G-buffer
    ↓
ray-traced reflections
    ↓
ML denoiser/upscaler
    ↓
tone map
    ↓
presentation
~~~

Treating ML as an external black box would lose resource lifetime and scheduling information. A better architecture places the ML pass in the same dependency graph, with explicit input/output resources and completion edges.

The frame graph becomes a heterogeneous GPU work graph.

#### The cross-backend lesson from Metal 4 and modern Vulkan is convergence around explicit resource databases

The APIs remain different, but their engine pressures increasingly converge:

~~~text
Vulkan:
    descriptor indexing / descriptor buffers
    synchronization2
    timeline semaphores
    dynamic rendering
    explicit queue submission

Metal 4:
    argument tables
    residency sets
    next-generation barriers
    reusable command buffers
    explicit command allocators
~~~

The portable engine abstraction should therefore not try to make one API look like the other. It should preserve the shared truths:

\[
\boxed{
\text{resource identity}
+
\text{binding}
+
\text{residency}
+
\text{access dependency}
+
\text{completion}
+
\text{retirement}
}
\]

Then each backend can express those truths using its native mechanisms.

Source integration basis: Apple's current Metal 4 core API documentation and WWDC25 Metal 4 sessions, especially the command allocator, argument table, residency-set, queue-barrier, drawable-resource, sparse-resource, and multithreaded encoding material.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/21 - Buffers, images, memory, and uploads|Previous chapter]] · [[Chapter Edition/03 - Game Engine/23 - Execute and measure a Metal frame|Next chapter]]
