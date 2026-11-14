# 14 — Submit, Synchronize, and Retire GPU Work

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|Next chapter]]

Recorded command buffers and semaphores express dependencies whose completion must be matched to the right frame or image. The chapter then generalizes these rules to dependency and lifetime systems that retire resources only after all users finish.

**By the end:** Design submission and retirement paths that account for frame-in-flight state, presentation, errors, and multiple queues.

**Recall question:** Why does a frame fence alone fail to prove that every presentation semaphore or resource use has completed?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 30268–34646; source-span SHA256 57b3efd89da1037c6548fba942f9bd170f7c6bf5697294ec2b417cdc54b81c57. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Record, synchronize and retire explicit work

**Clarifications before the retained examples:** ordinary reusable and one-time-submit command buffers have different completion states. Frame completion does not prove present-wait semaphore consumption. Use the image-indexed present semaphore policy and late fence-reset/error handling developed above. A FIFO deletion example assumes ordered retirement values; independent queues require joined completion or a componentwise stamp. The retained `VulkanBuffer` move-constructor fragment delegates to an assignment operator that is not supplied, so it is an incomplete illustration, not a standalone compilable class. Readback additionally needs device-write-to-host-read visibility, and depth barriers must include the actual early/late writers.

### + M138

This closes **Phase 23**.

The exact curriculum ownership is: **M136 = Parts 2265–2279**, command pools/buffers, recording, draw/dispatch, submission, fences and binary/timeline semaphores; **M137 = Parts 2280–2292**, synchronization stages/accesses, execution versus memory dependencies, barriers/layouts/queue ownership, Synchronization2, frames in flight and deferred destruction; **M138 = Parts 2293–2300**, timestamp queries/readback, synchronization validation, debug naming, device loss, MoltenVK, full frame tracing and correct Vulkan RAII.

The original contract confirms the sequence: command pools begin at 2265; queue submission/fences/semaphores close M136 at 2279; synchronization starts at 2280; frame lifetime occupies 2290–2292; diagnostics and explicit RAII finish the phase at 2300.

This is arguably the most important Vulkan pass.

Most Vulkan bugs are not:

> My matrix multiplication is wrong.

They are:

> This resource was used too early, destroyed too early, transitioned incorrectly, waited at the wrong stage, reused while still in flight, or made visible without actually ordering the producer.

The central equation for this pass is therefore not numerical.

It is:

$$
\boxed{
\text{Correct GPU execution}
=
\text{ordering}
+
\text{memory visibility}
+
\text{lifetime}
}
$$

and all three are distinct.

---

### COMMAND POOLS, COMMAND BUFFERS, SUBMISSION & SEMAPHORES

**Vulkan commands are recorded before execution**

When you call:

```cpp
vkCmdDraw(...);
```

the GPU does **not** necessarily draw at that moment.

You are recording an instruction into a command buffer.

Conceptually:

```text
CPU recording time:
    vkCmdBindPipeline
    vkCmdBindDescriptorSets
    vkCmdDrawIndexed
    vkCmdPipelineBarrier
    ...

                ↓

VkCommandBuffer

                ↓ later

vkQueueSubmit

                ↓

GPU execution
```

This separation is one of Vulkan's fundamental architectural properties.

---

**Command pools**

A `VkCommandPool` owns allocation backing for command buffers.

It is created for one:

$$
\boxed{\text{queue family}}
$$

not one particular queue handle.

Kairo:

```cpp
VkCommandPoolCreateInfo pool{};
pool.sType =
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

pool.flags =
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

pool.queueFamilyIndex =
    device.GraphicsFamily();
```

So command buffers allocated from this pool are compatible with queues belonging to Kairo's graphics family.

---

**Why the queue-family index belongs to the pool**

Command-buffer implementation may maintain internal state optimized for a particular family of commands.

If a command buffer was allocated from a pool associated with graphics family $G$, it is intended to be submitted to compatible queues from that family.

This makes pool organization naturally follow threading/queue architecture.

---

**Command pools are externally synchronized**

A subtle Vulkan rule:

$$
\boxed{
\text{simultaneous host access to the same command pool must be synchronized}.
}
$$

This matters when multithread-recording.

A good design is often:

```text
thread 0
    own command pool

thread 1
    own command pool

thread 2
    own command pool
```

rather than all CPU workers allocating/resetting from one pool behind a mutex.

---

**Kairo currently has one command pool**

Current `VulkanCommandBuffer` owns:

$$
1\text{ command pool}
$$

and:

$$
1\text{ primary command buffer}.
$$

The source comment explicitly says this is a:

$$
\boxed{\text{single-frame renderer baseline}}
$$

and that the buffer is not safe for concurrent recording.

This is simple and correct for the current architecture.

---

**Command-pool reset strategies**

There are several ways to reuse command storage.

You can reset a particular command buffer if pool supports:

$$
VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT.
$$

Or reset the entire pool:

```cpp
vkResetCommandPool(...);
```

which can be more efficient when all command buffers have the same frame lifetime.

A multi-frame renderer often has:

```text
FrameContext 0 → command pool
FrameContext 1 → command pool
FrameContext 2 → command pool
```

and resets the whole pool only when that frame's fence/timeline proves completion.

---

**Primary command buffers**

Primary buffers can be submitted directly to a queue.

Kairo allocates:

```cpp
allocate.level =
    VK_COMMAND_BUFFER_LEVEL_PRIMARY;
```

Primary buffer can contain:

- render pass begin/end,
- pipeline bindings,
- barriers,
- draws,
- copies,
- dispatches,
- secondary command-buffer execution.

This is what Kairo records every frame.

---

**Secondary command buffers**

Secondary command buffers cannot ordinarily be independently submitted like primaries.

They are executed from primary buffers:

```cpp
vkCmdExecuteCommands(
    primary,
    count,
    secondaryBuffers);
```

They are particularly useful for multithreaded command generation.

For example:

```text
CPU thread 0 → secondary for opaque batch A
CPU thread 1 → secondary for opaque batch B
CPU thread 2 → secondary for opaque batch C

main thread:
    primary
        begin render pass
        execute A/B/C
        end render pass
```

This lets several CPU threads record rendering work in parallel.

---

**Secondary rendering inheritance**

If a secondary command buffer executes inside a render pass, Vulkan needs enough inherited render-state context to validate/execute it.

Traditional render-pass secondary command recording uses:

```cpp
VkCommandBufferInheritanceInfo
```

and flags such as:

```cpp
VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
```

to describe the render pass/subpass context.

This is one of the complexities secondary command buffers introduce.

---

**Secondary buffers are not automatically faster**

They can reduce CPU command-recording bottlenecks.

But they also add:

- more command buffers,
- execution indirection,
- pool management,
- synchronization between CPU workers.

If command recording costs only:

$$
0.2\text{ ms},
$$

parallelizing it may be pointless.

Use them when CPU profiling establishes a recording bottleneck.

---

**Command-buffer lifecycle**

A simplified state model is:

```text
Initial
   ↓ vkBeginCommandBuffer
Recording
   ↓ vkEndCommandBuffer
Executable
   ↓ vkQueueSubmit
Pending
   ↓ GPU completes
Executable again
   ↓ reset
Initial
```

The key dangerous state is:

$$
\boxed{Pending}.
$$

A pending command buffer is still potentially being consumed by the GPU.

You cannot simply reset/re-record it.

---

**Kairo's `Begin()`**

Current Kairo performs:

```cpp
vkResetCommandBuffer(
    m_Handle,
    0);
```

then:

```cpp
vkBeginCommandBuffer(
    m_Handle,
    &begin);
```

but its comment explicitly states the precondition:

$$
\boxed{
\text{GPU work previously submitted with this buffer has completed}.
}
$$

That precondition is guaranteed because `DrawFrame()` waits on the one in-flight fence before re-recording.

Correct.

---

**Why resetting a pending command buffer is disastrous**

Suppose GPU still reads command data:

```text
draw mesh A
draw mesh B
```

while CPU resets and overwrites buffer with:

```text
draw mesh C
```

You have violated lifetime/execution ownership.

The API may report validation errors or behavior may be undefined.

The fence is therefore protecting not just “the image.”

It protects reusable CPU-side Vulkan state too.

---

**Beginning and ending rendering**

Traditional Vulkan:

```cpp
vkCmdBeginRenderPass(...);
```

then draw commands.

Finally:

```cpp
vkCmdEndRenderPass(...);
```

Current Kairo does this for:

- directional shadow pass,
- HDR editor viewport pass,
- swapchain presentation/overlay pass.

Inside a render pass, attachment semantics and subpass structure are active.

---

**Kairo frame recording structure**

Current `VulkanTriangle::Record()` is approximately:

```text
Begin command buffer

if meshes:
    shadow render pass

viewport render pass
    draw meshes
    draw debug lines
end viewport pass

optional object-ID copy
optional HDR color readback

swapchain presentation render pass
    tooling overlay
end presentation pass

End command buffer
```

This is already a multi-pass frame inside one primary command buffer.

---

**Binding pipelines**

Before graphics commands use a graphics pipeline:

```cpp
vkCmdBindPipeline(
    command,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline);
```

Pipeline defines the shader/fixed-state contract we covered in Pass 11.

For compute:

```cpp
VK_PIPELINE_BIND_POINT_COMPUTE.
```

Pipeline binding persists until replaced, within applicable recording semantics.

---

**Pipeline sorting**

If 10,000 objects alternate:

```text
pipeline A
pipeline B
pipeline A
pipeline B
...
```

you repeatedly change GPU state.

Sorting compatible draws by pipeline/material can reduce state changes.

But transparent rendering may impose strict depth ordering.

Therefore command ordering balances:

$$
\boxed{
\text{state locality}
}
$$

against:

$$
\boxed{
\text{rendering correctness}.
}
$$

---

**Binding vertex buffers**

Kairo uses:

```cpp
vkCmdBindVertexBuffers(...)
```

with one or two bindings.

Static mesh:

$$
binding_0=\text{MeshVertex}.
$$

Skinned mesh additionally has:

$$
binding_1=\text{SkinVertexInfluence}.
$$

Each binding has its own byte offset.

---

**Binding an index buffer**

Kairo:

```cpp
vkCmdBindIndexBuffer(
    command,
    mesh.Indices->Handle(),
    0,
    VK_INDEX_TYPE_UINT32);
```

The index type tells Vulkan how to interpret index bytes.

Then indexed drawing becomes possible.

---

**Descriptor-set binding**

Current Kairo:

```cpp
vkCmdBindDescriptorSets(
    command,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    layout,
    0,
    1,
    &descriptor,
    0,
    nullptr);
```

This means:

- graphics bind point,
- pipeline layout defines compatibility,
- first set is set 0,
- one descriptor set,
- no dynamic offsets.

Descriptor-set contents connect actual resources to shader bindings.

---

**Push constants**

Kairo also records per-draw transform/object state:

```cpp
vkCmdPushConstants(...)
```

So its per-draw resource state is currently approximately:

```text
descriptor set
    camera + material + textures + skin

push constants
    model/normal matrix + draw flags/object ID
```

This cleanly separates larger persistent resources from tiny high-frequency draw constants.

---

**`vkCmdDraw`**

Non-indexed draw:

```cpp
vkCmdDraw(
    command,
    vertexCount,
    instanceCount,
    firstVertex,
    firstInstance);
```

The vertex shader's logical vertex index derives from these parameters.

Kairo's debug line system uses:

```cpp
vkCmdDraw(
    command,
    debugVertexCount,
    1,
    0,
    0);
```

because debug lines are directly stored as a vertex stream.

---

**`vkCmdDrawIndexed`**

Kairo meshes use:

```cpp
vkCmdDrawIndexed(
    command,
    mesh.IndexCount,
    1,
    0,
    0,
    0);
```

Parameters conceptually mean:

$$
indexCount,
instanceCount,
firstIndex,
vertexOffset,
firstInstance.
$$

Per indexed vertex:

$$
vertexIndex
=
indexBuffer[firstIndex+i]
+
vertexOffset.
$$

This is how submeshes can reference ranges/base vertices without copying data.

---

**Instancing**

If:

$$
instanceCount>1,
$$

one draw launches multiple logical instances.

Vertex shader receives instance identity.

This is important for:

- repeated meshes,
- foliage,
- crowds,
- particles.

Eventually GPU-driven indirect draws extend this further.

---

**Compute dispatch**

Compute pipeline dispatch:

```cpp
vkCmdDispatch(
    command,
    groupCountX,
    groupCountY,
    groupCountZ);
```

Total invocation count depends on shader's local size:

$$
N_x=
groupCountX\times localSizeX
$$

and similarly for $y,z$.

The workgroup ideas from Pass 9 map directly onto Vulkan compute dispatch.

---

**Queue submission**

Recording produces an executable command buffer.

Execution begins when it is submitted:

```cpp
vkQueueSubmit(...)
```

Current Kairo builds:

```cpp
VkSubmitInfo submit{};
```

containing:

- semaphore to wait on,
- stage at which to wait,
- command buffer,
- semaphore to signal.

It also associates a fence with the submission.

This one call links nearly every synchronization concept in M136.

---

**Kairo's submit**

Current flow:

```text
wait:
    imageAvailable semaphore

wait stage:
    COLOR_ATTACHMENT_OUTPUT

execute:
    primary command buffer

signal:
    renderFinished[imageIndex]

CPU completion:
    inFlight fence
```

Then presentation waits on:

$$
renderFinished[imageIndex].
$$

That is a classic acquire→submit→present chain.

---

**Fence**

A fence is primarily a:

$$
\boxed{\text{GPU → CPU completion primitive}.}
$$

Create a fence.

Submit work with fence.

Later CPU does:

```cpp
vkWaitForFences(...)
```

and learns:

> work associated with that fence has completed.

This is different from a semaphore, which primarily coordinates GPU/queue operations.

---

**Why Kairo's first fence starts signaled**

Kairo creates:

```cpp
fence.flags =
    VK_FENCE_CREATE_SIGNALED_BIT;
```

Why?

At the first frame, no previous GPU frame exists.

But frame loop begins by:

```cpp
vkWaitForFences(...)
```

If the fence began unsignaled, first frame would wait forever because nothing had ever been submitted to signal it.

Starting signaled means:

$$
\boxed{
\text{frame 0's reusable resources are initially free}.
}
$$

---

**Fence reuse cycle**

Kairo performs:

```text
wait fence
    ↓
fence is signaled

acquire image

reset fence
    ↓
fence becomes unsignaled

submit graphics with fence

GPU finishes
    ↓
fence becomes signaled
```

Then next frame repeats.

The ordering is crucial.

---

**Never reset a fence before knowing you will submit**

Suppose you reset the fence first.

Then acquire returns:

$$
VK_ERROR_OUT_OF_DATE.
$$

You skip submission.

The fence remains unsignaled forever.

Next frame waits forever.

Kairo correctly acquires first and only resets the fence after successful/suboptimal acquisition.

This is a very important frame-loop edge case.

---

**Binary semaphores**

Binary semaphore has two conceptual states:

$$
unsignaled,
$$

$$
signaled.
$$

One GPU operation signals it.

Another waits.

You do not assign a numerical value.

Typical chain:

```text
presentation engine
   signal imageAvailable
          ↓
graphics submission waits
          ↓
graphics finishes
   signal renderFinished
          ↓
present operation waits
```

Exactly Kairo's model.

---

**Binary semaphore is not a CPU mutex**

You typically do not inspect its “state” on CPU.

It exists to build asynchronous dependency chains.

The virtue is that CPU does not need to stall between:

$$
acquire,
render,
present.
$$

Dependencies execute on the GPU/queue side.

---

**Why Kairo has one image-available semaphore**

Current Kairo has only:

$$
1\text{ frame in flight}.
$$

It waits the previous frame's fence before reacquiring.

Therefore the single `imageAvailable` semaphore has already been consumed before being reused.

This is safe under the current architecture.

With several frames in flight, you normally need per-frame acquire semaphores.

---

**Why Kairo uses render-finished semaphore per swapchain image**

Presentation completion is subtle because the graphics submission fence does not directly mean the presentation engine has finished consuming the present-wait semaphore.

Using one render-finished semaphore associated with each swapchain image is a robust pattern: reacquiring that image gives a natural guarantee that its previous presentation cycle has progressed far enough for reuse.

Kairo creates:

$$
renderFinished[imageIndex].
$$

Good.

---

**Timeline semaphores**

A timeline semaphore contains monotonically increasing 64-bit values.

Instead of:

```text
semaphore A
semaphore B
semaphore C
```

you can have:

$$
timeline=1
$$

for frame/job 1,

$$
timeline=2
$$

for frame/job 2,

$$
timeline=3.
$$

GPU signals values.

GPU or CPU waits for values.

---

**Timeline model**

Suppose:

$$
T
$$

is one timeline semaphore.

Upload submission signals:

$$
T=5.
$$

Graphics waits:

$$
T\ge5.
$$

Later graphics signals:

$$
T=6.
$$

CPU resource-retirement logic can ask whether:

$$
T\ge6.
$$

This creates a much more expressive completion sequence than a large collection of one-shot fences/semaphores.

---

**Timeline semaphores are excellent for resource retirement**

Suppose texture last used in submission value:

$$
137.
$$

Do not destroy immediately.

Store:

```text
resource → retireAfter = 137
```

Later query completed timeline value:

$$
C.
$$

If:

$$
C\ge137,
$$

destruction is safe.

This is vastly better than calling:

```cpp
vkDeviceWaitIdle()
```

every time a resource is destroyed.

---

**Kairo does not currently use timeline semaphores**

Current repository search shows no:

```text
VK_SEMAPHORE_TYPE_TIMELINE
```

or `VkSemaphoreTypeCreateInfo`.

So current Kairo uses:

$$
\boxed{
\text{one fence + binary semaphores}
}
$$

for its one-frame-in-flight baseline.

Timeline retirement is the natural next architectural step.

---

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


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|Next chapter]]
