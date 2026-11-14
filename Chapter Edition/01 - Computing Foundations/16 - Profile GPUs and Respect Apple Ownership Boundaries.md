# 16 — Profile GPUs and Respect Apple Ownership Boundaries

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems|Next chapter]]

Counters and timing tools reveal different parts of a frame, while Objective-C++, Swift, SwiftUI, and threading introduce ownership boundaries around native objects. The chapter connects measurement to safe cross-language and operating-system lifetimes.

**By the end:** Interpret GPU profiling evidence and define ownership, threading, and file/network boundaries across Apple platform APIs.

**Recall question:** What does a GPU counter establish, and what does it leave unknown about the full user-visible frame?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 37689–40136; source-span SHA256 13faebd39dbf30ebc015d30042cfe4cf02c2b2a84bd06204f0b7a175e0329a0a. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### PROFILING, OBJECTIVE-C++ OWNERSHIP, SWIFTUI & THREAD BOUNDARIES

**GPU counters and Metal profiling**

You should distinguish at least three kinds of performance measurements.

CPU encoding time:

$$
\text{how long CPU spent building commands}.
$$

Command-buffer GPU duration:

$$
\text{how long GPU command buffer occupied GPU timeline}.
$$

Detailed GPU counters:

$$
\text{what hardware was doing inside that duration}.
$$

Those answer different questions.

---

**Maveb's current coarse GPU timing**

Completion handler reads:

```cpp
completed->GPUEndTime()
-
completed->GPUStartTime()
```

then converts to milliseconds.

This gives whole-command-buffer GPU duration.

Useful for:

$$
\text{frame GPU ms}.
$$

But not sufficient to tell whether:

```text
Gaussian sort
shadow
TAA
bloom
presentation
```

is slow.

---

**Counter sample buffers**

Modern Metal profiling can use counter sampling mechanisms where supported.

Conceptually record counter samples at boundaries:

```text
sample 0
Gaussian projection
sample 1
Radix
sample 2
Composite
sample 3
```

Then resolve/read results after GPU completion.

Counters can expose architecture-dependent metrics such as timestamps and other supported statistics.

Capabilities must be queried.

---

**Profiling should not stall**

Bad profiler:

```text
commit
waitUntilCompleted
read counters
```

every frame.

That destroys CPU/GPU overlap.

Better:

```text
frame N records counters
frame N+2 reads frame N when already completed
```

This is the same latency-hiding principle as Vulkan query pools.

---

**Xcode GPU capture**

Xcode's Metal GPU capture can inspect:

- command buffers,
- encoders,
- resources,
- pipeline state,
- textures,
- draw calls,
- shader execution,
- performance.

This is the Metal equivalent of the workflow you might use RenderDoc for Vulkan, though Apple's integration can expose Apple-specific GPU details.

---

**Capture tells you execution reality**

Your source might say:

```text
Render scene
```

but GPU capture may reveal:

- five unexpected render passes,
- attachment stored and reloaded repeatedly,
- gigantic intermediate texture,
- redundant state binding,
- unexpected shader variant.

Performance optimization should eventually inspect the executed GPU workload, not merely source code.

---

**Metal labels**

Metal resources and command objects expose labels.

Maveb labels extensively:

```text
AETHER Primary Queue
AETHER Frame
Scene HDR Color
Scene Reverse-Z Depth
Gaussian HDR Color
Temporal Resolve Pass
Presentation Tone Map Pass
Mesh Upload
Presented Frame Capture
```

This is excellent.

It means GPU capture shows semantic engine names rather than anonymous resources.

---

**Encoder labels**

For every meaningful pass:

```cpp
encoder->
    setLabel(
        "Gaussian Radix Histogram");
```

or equivalent.

You can also use debug groups where useful:

```text
Frame
    Shadow
    Scene
    Gaussian
        Project
        Sort
        Composite
    TAA
    Bloom
    Present
```

A profiler/capture should reflect your engine's conceptual hierarchy.

---

**Why labels are not optional fluff**

When debugging a 200-pass renderer, names save enormous time.

Compare:

```text
Texture 0x173902
Encoder 0x77193
```

with:

```text
Scene Motion Vectors
Temporal Resolve Pass
```

Labels are part of engine observability.

---

**Objective-C retain/release intuition under ARC**

Objective-C objects use reference counting.

Conceptually:

$$
retain
\Rightarrow
refcount+1,
$$

$$
release
\Rightarrow
refcount-1.
$$

When count reaches zero:

$$
object\ destroyed.
$$

ARC automatically inserts many retain/release operations based on ownership semantics.

---

**Strong ownership**

A strong reference keeps object alive.

Objective-C property:

```objective-c
@property(strong)
id<MTLDevice> device;
```

means owner retains it.

When owner releases property, reference count can fall.

This is analogous conceptually to `shared_ptr`, but implementations and semantics are not identical.

---

**Weak references**

Weak reference:

$$
\boxed{\text{does not keep object alive}}
$$

and is automatically zeroed when referenced Objective-C object dies.

Useful for delegate relationships to avoid retain cycles.

Again, this is Objective-C object lifetime, not GPU completion lifetime.

A Metal object can be strongly alive while GPU work is unfinished, and command-buffer/resource retention semantics matter too.

---

**Autorelease pools**

Some Objective-C APIs return autoreleased temporary objects.

A tight render loop without a local autorelease pool can accumulate temporary objects until outer pool drains.

Therefore render code frequently uses:

```objective-c
@autoreleasepool
{
    ...
}
```

Kairo does this in Metal draw calls.

Maveb's C++ Metal renderer explicitly creates/releases an autorelease pool per frame.

That is correct.

---

**ARC does not manage C++ objects**

Suppose Objective-C++ object contains:

```cpp
std::unique_ptr<
    aether::metal::Renderer>
    _renderer;
```

ARC does not manage that pointer.

C++ RAII does.

So in Objective-C++ source you have **two ownership systems simultaneously**:

$$
\boxed{
\text{ARC for Objective-C objects}
}
$$

and:

$$
\boxed{
\text{C++ RAII for C++ objects}.
}
$$

You must understand which system owns each object.

---

**metal-cpp adds a third-looking syntax, but same ownership reality**

Maveb uses C++ wrappers around Objective-C Metal objects.

Its helper pattern distinguishes:

$$
adopt(x)
$$

from:

$$
retain(x).
$$

Conceptually:

`adopt()` means:

> this newly created object already carries ownership; wrap it without adding another retain.

`retain()` means:

> borrowed object must be retained because wrapper will own it.

Mix those incorrectly and you get:

- leaks,
- double releases,
- dangling pointers.

---

**Objective-C++ `.mm` translation units**

A `.mm` file can contain both:

```objective-c
id<MTLDevice>
```

and:

```cpp
std::unique_ptr<Renderer>
```

and Objective-C message syntax:

```objective-c
[object method]
```

plus ordinary modern C++.

This is the ideal bridge layer between:

$$
\text{C++ engine core}
$$

and:

$$
\text{AppKit/Metal/Objective-C APIs}.
$$

---

**Why not put Objective-C in all C++ files?**

It infects portability.

If core engine headers contain:

```objective-c
#import <Metal/Metal.h>
```

or Objective-C object syntax, Linux/Windows compilers cannot parse them.

Better architecture:

```text
portable C++ headers
        ↓
Objective-C++ implementation bridge (.mm)
        ↓
Metal / AppKit
```

This keeps platform code localized.

---

**Kairo follows this idea**

Public module:

```text
MetalRenderer.cppm
```

talks through:

```text
detail::MetalBackend
```

while raw Objective-C Metal implementation lives in:

```text
detail/MetalBackend.mm.
```

That is a good boundary.

The public renderer-neutral host does not need Objective-C syntax leaking through every module.

---

**C++ ownership across Objective-C++ bridges**

AetherStudio's bridge contains:

```cpp
std::unique_ptr<
    aether::metal::Renderer>
    _renderer;
```

inside:

```objective-c
@implementation
AetherViewportDelegate
```

This is a very clean boundary.

Objective-C object owns a C++ unique pointer.

When delegate dies, C++ renderer destructor executes.

Renderer then releases its own Metal C++ wrapper resources through its RAII mechanisms.

---

**Passing an Objective-C Metal device into C++**

Aether bridge:

```objective-c
id<MTLDevice> device
```

is converted to metal-cpp representation with a non-owning bridge cast.

Then:

```cpp
Renderer::create(
    reinterpret_cast<
        MTL::Device*>(
        (__bridge void*)device));
```

The `__bridge` cast does not transfer Objective-C ownership.

Therefore C++ renderer must establish its own ownership if it stores the device.

Maveb constructor does:

```cpp
device_(retain(device))
```

which is exactly what you want.

---

**Ownership equation**

At bridge:

$$
\boxed{
\text{borrow ObjC device}
}
$$

then inside persistent C++ renderer:

$$
\boxed{
\text{retain device for C++ lifetime}.
}
$$

If it merely stored borrowed pointer, Objective-C owner could disappear and leave dangling C++ Metal pointer.

The retain is the lifetime bridge.

---

**error bridging C++ → Objective-C/Swift**

Never casually allow C++ exceptions to cross into Swift/Objective-C API boundaries.

Different exception runtimes/ABI semantics make that unsafe architecture.

Instead transform:

$$
C++\ Result<T,Error>
$$

into:

$$
Objective\text{-}C\ return\ value
+
NSError.
$$

---

**Example bridge**

C++:

```cpp
Result<void>
SaveThing(...);
```

Objective-C++:

```objective-c
BOOL SaveThingBridge(
    NSURL* destination,
    NSError** error)
{
    auto result =
        SaveThing(...);

    if (!result)
    {
        if (error)
        {
            *error =
                [NSError
                    errorWithDomain:
                        @"com.engine"
                    code:
                        1
                    userInfo:
                        @{
                          NSLocalizedDescriptionKey:
                            ...
                        }];
        }

        return NO;
    }

    return YES;
}
```

This is exactly the pattern Aether uses in `AetherWriteDiagnostics`.

---

**Current Aether error bridge has multiple levels**

`AetherWriteDiagnostics()` converts failures into:

$$
NSError**.
$$

Many viewport operations currently instead:

- return `BOOL`,
- return `nil`,
- return empty arrays,
- log engine error.

That is acceptable for internal editor operations.

For APIs where caller must distinguish failure reasons, promote them to structured `NSError`/Swift `Error`.

---

**Preserve error semantics**

Do not reduce:

```text
resource exhausted
invalid argument
file not found
corrupt data
Metal command failure
```

into:

```text
"something failed"
```

A bridge should preserve:

$$
error\ code,
$$

$$
message,
$$

$$
context.
$$

Maveb's internal errors already carry:

$$
ErrorCode
+
description/context.
$$

That is useful information to carry upward.

---

**SwiftUI view versus rendering-core responsibility**

This separation is extremely important.

SwiftUI should own:

$$
\text{application/UI state}.
$$

Renderer should own:

$$
\text{GPU resources and frame execution}.
$$

Bridge translates between them.

Do not let SwiftUI understand:

```text
MTLBuffer
MTLTexture
command encoders
Gaussian radix buffers
BVH nodes
```

and do not make render core understand:

```text
@Binding
@State
@AppStorage
NSViewRepresentable
```

---

**Aether's architecture**

SwiftUI:

```text
AetherViewport
```

contains bindings:

```text
selectedGaussianId
selectedMeshId
camera
materials
lights
playback
debug settings
```

It creates:

```text
AetherViewportView
```

through `NSViewRepresentable`.

Objective-C++ view owns:

```text
MTKView
AetherViewportDelegate
```

Delegate owns:

```cpp
std::unique_ptr<
    aether::metal::Renderer>
```

This is a strong layered architecture.

---

**The responsibility chain**

```text
SwiftUI
    declarative editor state

        ↓

NSViewRepresentable
    lifecycle/update synchronization

        ↓

AetherViewportView
    AppKit input/window/view integration

        ↓

AetherViewportDelegate
    Objective-C++ conversion boundary

        ↓

aether::metal::Renderer
    C++ rendering system

        ↓

Metal
```

Every layer speaks its natural language.

That is what good platform integration looks like.

---

**Why SwiftUI should not issue GPU commands**

SwiftUI body/update execution is driven by declarative state changes.

It is not a stable real-time rendering loop.

GPU frame execution needs:

- consistent timing,
- resource lifetimes,
- command-buffer scheduling,
- frames in flight.

`MTKView`/renderer handles that.

SwiftUI configures it.

---

**`NSViewRepresentable`**

Aether:

```swift
struct AetherViewport:
    NSViewRepresentable
```

implements:

```swift
makeNSView(...)
```

and:

```swift
updateNSView(...)
```

`makeNSView` constructs native Metal-backed view once.

`updateNSView` propagates changes in Swift state into existing native renderer/view.

That is exactly how a long-lived renderer should integrate with SwiftUI.

---

**Coordinator prevents redundant work**

Aether's `Coordinator` caches previously applied:

```text
scenePath
dynamicMeshPath
transform overrides
materials
lights
camera
playback
```

Then `updateNSView` compares.

Only changed values are pushed into renderer.

This matters because SwiftUI may call update many times.

You do not want to reload a scene or re-upload every material because some unrelated UI state changed.

---

**main-thread UI constraints**

AppKit and SwiftUI state/view operations generally belong on the main thread.

Examples:

```text
creating/changing NSView hierarchy
mutating SwiftUI-observed UI state
handling many AppKit events
```

GPU completion handlers, worker threads and asset loaders may run elsewhere.

The rule becomes:

$$
\boxed{
\text{renderer thread may produce data}
\rightarrow
\text{main thread consumes UI-facing result}.
}
$$

---

**Why this matters with Metal completion handlers**

Completion handler might do:

```cpp
completedFrameCapture_ =
    capture;
```

under mutex.

Good.

It should not directly do:

```swift
selectedImage = capture
```

or manipulate `NSView` from unknown callback thread.

Instead:

```text
GPU callback
    ↓
thread-safe result queue/state
    ↓
DispatchQueue.main
    ↓
UI update
```

---

**Aether Swift code already uses main dispatch for deferred state updates**

Examples in `updateNSView`:

```swift
DispatchQueue.main.async {
    selectedMeshTransform =
        snapshot
}
```

and similar material/selection updates.

This avoids mutating SwiftUI binding synchronously in problematic update cycles and keeps UI mutation on main.

---

**cross-thread renderer callbacks**

Once renderer becomes asynchronous, events can originate from:

```text
GPU completion
asset loader
simulation
network
background reconstruction
```

while UI exists on main thread.

A callback boundary therefore needs an explicit threading contract.

For every callback answer:

$$
\boxed{
\text{What thread invokes it?}
}
$$

$$
\boxed{
\text{Who owns captured data?}
}
$$

$$
\boxed{
\text{May receiver outlive sender?}
}
$$

$$
\boxed{
\text{Must callback be marshalled to main thread?}
}
$$

---

**Bad cross-thread callback**

Imagine:

```cpp
commandBuffer->
    addCompletedHandler(
        [this]
        {
            uiView->
                setProgress(1.0);
        });
```

Problems:

`uiView` may have been destroyed.

Callback thread may not be main.

`this` may have died.

UI state may race.

This is several bugs hidden in five lines.

---

**Safer architecture**

GPU callback produces plain data:

```cpp
struct CompletedFrameInfo
{
    uint64_t Frame;
    double GpuMs;
};
```

Then places it into thread-safe storage.

Main/UI side polls or receives main-thread dispatch.

Conceptually:

```cpp
commandBuffer->addCompletedHandler(
    [weakState](...)
    {
        if (auto state =
                weakState.lock())
        {
            state->
                PushCompleted(...);
        }
    });
```

Then UI:

```swift
DispatchQueue.main.async {
    consumeResult()
}
```

Ownership and threading are explicit.

---

**Completion handlers and renderer destruction**

Maveb's renderer destructor drains its three frame-semaphore tokens.

Conceptually it waits until all submitted frames have completed before destruction continues.

That protects callbacks/resources referencing renderer state.

This is the Metal analogue of waiting/retiring in-flight frames before Vulkan teardown.

---

**Deferred cleanup in Maveb FrameContext**

Each frame context contains:

```cpp
deferredCleanup_
```

and:

```cpp
deferUntilReusable(...)
```

At:

```cpp
beginFrame()
```

it runs those cleanup functions.

Because frame slot is only reusable after semaphore completion discipline, this provides a simple frame-retirement mechanism.

Same idea as Vulkan deferred destruction.

Different synchronization primitive.

---

**This is the recurring cross-API pattern**

Vulkan:

$$
frameFence/timeline
\rightarrow
resource retirement.
$$

Metal:

$$
completion handler/frame semaphore/shared event
\rightarrow
resource retirement.
$$

The high-level engine concept is identical:

$$
\boxed{
\text{destroy only when last GPU use has completed}.
}
$$

This is precisely why the upcoming multi-backend abstraction must model **intent**, not API object names.

---

**Kairo Metal versus Maveb Metal**

The two backends represent two maturity points.

| Concern | Kairo Metal | Maveb Metal |
|---|---|---|
| Device | default Metal device | supplied/retained device |
| Queue | one command queue | one labelled primary queue |
| Shader library | runtime source compilation | offline `.metallib` |
| Frame command buffer | one/frame | one complex command buffer/frame |
| Frames in flight | effectively simple/last-command model | 3 |
| Frame limiting | waits where needed | semaphore count 3 |
| Uploads | direct/shared buffers | per-frame 8 MiB shared arena |
| Readback | direct/shared + waits for simple operations | 256 KiB frame readback + async capture |
| Render targets | shared in simple backend | private |
| Transient heap | none in simple backend | 64 MiB private/frame |
| Hazard mode | default | tracked transient heap |
| Presentation | direct `CAMetalLayer` | `MTKView` drawable |
| Capture | waits on `LastCommand` | asynchronous blit + completion callback |
| GPU timing | not central | `GPUStartTime/GPUEndTime` |
| Labels | limited | extensive |
| UI bridge | renderer-neutral runtime | Objective-C++ + SwiftUI editor bridge |

Neither codebase is “wrong.”

Kairo prioritizes:

$$
\boxed{
\text{small comprehensible backend}
}
$$

while Maveb prioritizes:

$$
\boxed{
\text{overlapped multi-pass production behavior}.
}
$$

---

**A more production-oriented Kairo Metal evolution**

The most useful progression would be:

$$
\text{single LastCommand wait}
$$

to:

$$
\text{FrameContext[2 or 3]}.
$$

Each frame context gets:

```cpp
struct MetalFrameContext
{
    id<MTLBuffer> Upload;
    size_t UploadCursor;

    std::vector<
        std::function<void()>>
        DeferredCleanup;
};
```

At frame begin, wait only if rotating into a still-busy slot.

Then encode/commit.

Completion handler releases slot.

Exactly the model Maveb proves works.

---

**Kairo static meshes should eventually become private**

Current Metal mesh buffers are created with:

$$
StorageModeShared.
$$

For static geometry:

$$
CPU writes once,
GPU reads repeatedly.
$$

Better architecture:

```text
shared upload buffer
    ↓
blit
    ↓
private vertex/index buffer
```

Whether this materially improves Apple-silicon performance should be benchmarked, but it is the more general GPU-local architecture and mirrors Maveb's static upload path.

---

**Maveb's upload path demonstrates this**

Its static mesh upload creates destination and staging buffers.

Then:

```cpp
blit->
    copyFromBuffer(
        staging,
        destination);
```

Completion handler keeps staging object alive until copy finishes.

This is a subtle but important lifetime technique.

The lambda captures `staging`.

Therefore staging cannot be destroyed before GPU finishes copy.

Very elegant.

---

**Command-buffer retention of resources**

Metal generally retains resources referenced by encoded commands sufficiently for execution semantics, but your wrapper/application ownership still matters around buffers not directly retained in the same way, indirect pointers, callbacks and CPU-side state.

Do not treat command-buffer retention as an excuse to forget logical resource lifetime.

The engine still needs a coherent ownership model.

---

**Apple GPU thinking versus desktop immediate thinking**

The biggest practical mental shift is this:

Desktop-style intuition often optimizes around:

$$
\boxed{
\text{VRAM bandwidth + large caches + immediate framebuffer writes}.
}
$$

Apple GPU intuition should additionally prioritize:

$$
\boxed{
\text{tile locality}
}
$$

$$
\boxed{
\text{attachment lifetime}
}
$$

$$
\boxed{
\text{load/store elimination}
}
$$

$$
\boxed{
\text{unified-memory traffic}
}
$$

$$
\boxed{
\text{render-pass grouping}.
}
$$

This does not mean every Apple GPU is identical.

It means the tile-based architecture strongly affects good rendering structure.

---

**Why unnecessary pass splitting can hurt**

Imagine:

```text
Pass A
    render color
    STORE to memory

Pass B
    LOAD color
    modify
    STORE

Pass C
    LOAD
    modify
    STORE
```

Each pass boundary can force memory traffic.

If operations could legally remain in one rendering scope or use tile-local mechanisms, you may save enormous bandwidth.

This is one motivation behind render-pass/subpass/tile-aware graph optimization.

---

**Why over-fusing can also hurt**

One giant pass can:

- require too many attachments,
- increase shader complexity,
- force resources to remain live,
- prevent async compute,
- reduce modularity.

Again, the objective is not:

> fewer passes at any cost.

It is:

$$
\boxed{
\text{minimize unnecessary external memory traffic while preserving useful execution structure}.
}
$$

---

**Attachment lifetime equation**

For attachment $R$:

If previous content is irrelevant:

$$
LoadCost(R)=0
$$

using Clear/DontCare semantics.

If result never needed after pass:

$$
StoreCost(R)=0.
$$

Ideal transient attachment:

$$
\boxed{
Load=DontCare/Clear
,\quad
Store=DontCare
}
$$

and memoryless where supported.

That resource can conceptually live entirely inside tile execution.

---

**Why temporal buffers cannot be transient**

TAA history:

$$
H_t
$$

must be consumed by frame:

$$
t+1.
$$

Therefore its lifetime crosses frame boundary.

It must:

$$
Store
$$

and remain allocated.

This is the opposite of memoryless.

The renderer must distinguish:

$$
\boxed{
\text{transient frame-local resource}
}
$$

from:

$$
\boxed{
\text{history/persistent resource}.
}
$$

#### First-use completion — lifetime analysis for a frame-local attachment and a history image

Here is the actual render-graph reasoning now, while transient and persistent storage first affect the result. Consider four operations: **geometry** writes a screen-space normal attachment $N_k$; **lighting** reads $N_k$ and writes a linear HDR image $C_k$; **temporal accumulation** reads $C_k$ and the preceding history $H_{k-1}$ and writes new history $H_k$; **presentation** reads $H_k$. These are logical resource *versions*. The graph edges are
$$
\text{geometry}\to\text{lighting}\to\text{temporal}\to\text{present}
$$
with another edge $H_{k-1}\to\text{temporal}$ arriving from the preceding frame. An edge describes both the required execution ordering and the visibility of the producer's writes to the consumer, using the exact API's synchronization mechanisms. A memory allocation remaining alive is not proof that this happens-before relationship exists.

The graph can find a resource's logical **live interval** by recording the first writing pass and the last pass that must read it. The normal attachment $N_k$ is needed from geometry through lighting but does not have to survive the frame when no later pass samples it. In a compatible tile-local rendering scope, and only where the platform/API supports the requested attachment usage, that can permit clear/dont-care load/store decisions or memoryless storage. The HDR image $C_k$ must survive until the temporal pass reads it, so the compiler cannot discard it immediately after lighting. The history $H_{k-1}$ must survive from its previous producer through *this* temporal read, and $H_k$ must survive until the following frame's temporal pass. Memoryless storage cannot fulfill that cross-frame read contract.

For an ordinary RGBA16F history, each pixel stores four 16-bit channels: $4\times2=8$ bytes. At $1920\times1080$ pixels one history uses $1920(1080)(8)=16{,}588{,}800$ bytes, about $15.82$ MiB, before alignment, tiling or additional metadata. Two such images alone account for about $31.64$ MiB. The word **persistent** does not say they must always live in dedicated physical allocations, but it says that their contents cannot be thrown away while future work still needs them.

Two history versions are also not automatically sufficient for safe frames-in-flight. Frame $k$ can read image A and write B. Frame $k+1$ may read B and intend to write A, but A can be recycled only after the last asynchronous read that uses A has completed. A ring of physical images plus completion-aware retirement may be necessary. The distinction is between a **logical version** (the meaning of a frame's history) and a **physical allocation** (memory that can carry one version when no overlapping use forbids it). Conversely, allocating a separate texture for every named graph version may waste large amounts of memory if two transient versions have provably disjoint lifetimes and compatible usage.

```text
geometry:
    WRITE N[k]                   # valid normal/color encoding, declared format
lighting:
    READ  N[k]                   # require producer-write visibility
    WRITE C[k]
temporal:
    READ  C[k]
    READ  H[k-1]                 # prior-frame content, not memoryless
    WRITE H[k]                   # new version, not an alias of a live input
present:
    READ H[k]

# The compiler can alias physical storage only if lifetime/schedule
# non-overlap, format/usage restrictions and synchronization all hold.
```

One important qualification is that a linear list of passes does **not** automatically establish safe aliasing. With asynchronous compute or multiple command queues, two apparently non-overlapping entries in a textual schedule may actually execute concurrently. Aliasing is safe only after the compiler proves no overlapping physical access in the *scheduled* dependency graph and establishes the required reuse barrier or completion. If a pass writes a resource and the next reads it, aliasing does not fix a missing visibility edge; if a pass reads a resource after its allocation has been reused, a correct barrier elsewhere does not restore the lost contents.

Temporal **validity** adds a second, semantic condition. Keeping $H_{k-1}$ allocated and synchronized does not make its pixels meaningful after a camera teleport, resolution change, projection-convention change, material edit, estimator change or discontinuity in object identity. A history epoch should identify which camera/scene state produced the previous estimate; reset or deliberately reproject according to documented validity rules. A renderer that displays a stable but ghosted image can have perfectly correct GPU lifetime synchronization and still have incorrect history semantics.

At first use, therefore, a render-graph resource requires five answers: **who creates this version, who last consumes it, which operation makes writes visible, what completion permits storage reuse, and what semantic state makes its contents valid?** A test should intentionally delay one GPU submission and verify no buffer is recycled early; another should change the camera and verify history invalidation. Those are different tests. This is the complete reasoning needed here, not a placeholder for a later render-graph pass.

---

**Metal resource synchronization mental model**

You can now think in four levels.

Same encoder:

$$
\text{use barriers where shader memory dependency requires}.
$$

Different encoders, tracked resource:

$$
\text{Metal hazard tracking commonly orders required use}.
$$

Different encoders, untracked resource:

$$
\text{explicit fence/barrier semantics may be necessary}.
$$

Different queues/command streams:

$$
\text{events/shared events encode inter-stream dependency}.
$$

This hierarchy prevents treating all Metal synchronization as one thing.

---

**Metal/Vulkan naming trap**

Never build a cross-platform abstraction by matching names:

```text
VkFence ↔ MTLFence
```

because semantics are different.

Instead map concepts:

| Engine concept | Vulkan | Metal |
|---|---|---|
| CPU knows GPU submission completed | `VkFence` / timeline value | completion handler / shared event value |
| GPU queue-to-queue timeline | timeline semaphore | shared event |
| encoder-local GPU dependency | barrier | memory barrier |
| resource dependency between phases | pipeline/image/buffer barrier | tracked hazard / fence / event depending scope |
| presentation image | swapchain image | drawable texture |

This is the beginning of correct multi-backend thinking.

Pass 14 will formalize it.

---

**ARC versus GPU lifetime**

ARC answers:

$$
\boxed{
\text{Is the Objective-C object alive?}
}
$$

It does not answer:

$$
\boxed{
\text{Has GPU stopped using the object's underlying resource?}
}
$$

Those are different questions.

Likewise C++ `unique_ptr` answers CPU ownership.

GPU completion remains another dimension.

Same lesson as Vulkan RAII.

---

**A robust bridge lifetime diagram**

```text
SwiftUI AetherViewport
       ↓ owns
AetherViewportView
       ↓ owns
AetherViewportDelegate
       ↓ C++ unique_ptr owns
aether::metal::Renderer
       ↓ MetalPtr owns
MTL resources

GPU command buffer
       ↓ temporarily uses
MTL resources

completion
       ↓
frame slot becomes reusable
```

Every arrow has a different ownership/synchronization meaning.

Understanding those arrows is much more valuable than memorizing ARC syntax.

---

**Swift closure lifetime**

A view property such as:

```objective-c
@property(copy)
void (^onEntityPicked)(...);
```

owns a copied block.

Swift closure assigned to it can capture values/objects.

If it strongly captures the same object that strongly owns the view, you can form a retain cycle.

Therefore class-heavy bridges often use:

```swift
[weak self]
```

when appropriate.

In Aether's current `NSViewRepresentable` closures, the bindings/value context differs from a typical view-controller retain cycle, but the general rule remains:

$$
\boxed{
\text{inspect closure capture graph explicitly}.
}
$$

---

**Thread ownership is part of API design**

Instead of documentation:

> callback occurs later.

Write:

> callback may execute on a Metal completion thread; callback must be nonblocking and may not access AppKit. Results are marshalled to main thread.

That is a real API contract.

Similarly:

> `draw()` is invoked by MTKView frame lifecycle.

> `setCameraState()` must be called from UI/main integration layer.

Threading assumptions should be first-class.

---

**Avoid holding locks in GPU completion handlers**

Completion handlers should be quick.

A callback that blocks on:

$$
heavy\ disk\ IO
$$

or a contended mutex can delay callback processing.

Good handler:

```text
copy minimal metadata
move finished payload
atomic counter
signal semaphore
enqueue lightweight task
```

Then larger work occurs elsewhere.

Maveb's handler mostly follows this pattern.

---

**Synchronous picking caveat in Maveb**

Some current picking paths create a command buffer, blit one pixel and call:

```cpp
waitUntilCompleted();
```

This is simple and correct.

But it stalls caller until GPU finishes.

For occasional editor clicks that may be acceptable.

For continuous hover picking at 60–120 Hz, convert to asynchronous frame readback just like Kairo's Vulkan pick path.

This is exactly the kind of design decision that depends on workload frequency.

---

**Kairo Metal's `WaitForLastCommand()`**

Kairo's destruction/pick/capture path waits for:

$$
LastCommand.
$$

Again:

$$
\boxed{
\text{simple and correct}
}
$$

but it serializes.

Its next maturity step is not “remove the wait.”

It is:

$$
\boxed{
\text{replace global wait with explicit frame/resource retirement}.
}
$$

Never remove synchronization without adding an ownership model.

---

**The three Metal memory questions**

For every resource ask:

$$
\boxed{
\text{Does CPU need direct access?}
}
$$

If yes, Shared/Managed may be appropriate.

Then:

$$
\boxed{
\text{Does data need survive the render pass/frame?}
}
$$

If no, memoryless/transient strategy may apply.

Then:

$$
\boxed{
\text{Who synchronizes hazards?}
}
$$

Tracked mode or explicit synchronization.

These three questions often determine the correct Metal resource architecture.

---

**The three encoder questions**

Before adding an encoder ask:

$$
\boxed{
\text{Is this rasterization, compute, or transfer?}
}
$$

Then:

$$
\boxed{
\text{What resources does it read/write?}
}
$$

Then:

$$
\boxed{
\text{Can its work remain on-chip or does it force resource store/load?}
}
$$

This prevents accidental pass fragmentation.

---

**Practical Maveb optimization candidates revealed by M140**

Your current architecture already does many strong things:

$$
Private\ render\ targets,
$$

$$
3\ frames\ in\ flight,
$$

$$
Shared\ per-frame\ upload,
$$

$$
Shared\ readback,
$$

$$
Private\ transient\ heap,
$$

$$
Tracked\ hazards,
$$

$$
offline\ metallib,
$$

$$
labels,
$$

$$
async\ frame\ capture.
$$

The next questions are therefore not basic Metal correctness.

They are more advanced:

Can some strictly transient attachments become memoryless where supported?

Can graph lifetime analysis allocate actual temporary resources from the per-frame heap and alias them?

Can direct bindings become argument-buffer/resource-table architecture where material scale demands it?

Can synchronous pick commands become asynchronous if interaction frequency rises?

Can detailed counter sampling show whether Gaussian work is compute-, bandwidth- or tile-pressure-limited?

Those are mature optimization questions.

---

**PASS 13 RECONSTRUCTION TABLE**

| Topic | Reconstruction |
|---|---|
| `MTLDevice` | GPU/capability/resource factory |
| `MTLCommandQueue` | ordered command-buffer submission stream |
| `MTLCommandBuffer` | one asynchronously executed GPU work package |
| Render encoder | rasterization commands |
| Compute encoder | general thread-grid kernels |
| Blit encoder | copies/mip generation/transfer work |
| `MTLBuffer` | linear byte resource |
| `MTLTexture` | structured image resource |
| `MTLSamplerState` | filtering/addressing/comparison |
| Render pipeline | compiled vertex+fragment and attachment-format/blend contract |
| Compute pipeline | compiled `kernel` state + execution properties |
| Metal library | container of compiled/runtime MSL functions |
| MSL vertex | `vertex` function producing `[[position]]` |
| MSL fragment | `fragment` function consuming interpolants/resources |
| MSL compute | `kernel` over thread grid |
| Shared | CPU+GPU accessible |
| Private | GPU-only optimized resource |
| Managed | CPU/GPU copies synchronized on applicable macOS configurations |
| Memoryless | transient tile-only attachment where supported |
| TBDR | bin geometry, rasterize per tile in on-chip memory |
| Load action | how attachment begins |
| Store action | whether tile result survives pass |
| Drawable | presentable texture instance |
| `CAMetalLayer` | drawable provider / Core Animation presentation layer |
| `presentDrawable` | schedule presentation through command buffer |
| Completion handler | asynchronous post-GPU callback |
| Frame semaphore | cap frames in flight |
| Upload arena | one shared per-frame buffer + aligned slices |
| Argument buffer | table of indirectly bound resources |
| Heap | pooled resource memory |
| Hazard tracking | automatic dependency tracking |
| Untracked resource | application owns synchronization |
| Memory barrier | shader/resource ordering within relevant execution scope |
| `MTLFence` | GPU encoder dependency primitive |
| Shared event | timeline-style inter-command/queue/CPU synchronization |
| GPU counters | hardware/timestamp measurements |
| Xcode capture | GPU workload inspection |
| Labels | semantic names for GPU tools |
| ARC | Objective-C reference-count automation |
| `.mm` | Objective-C++ translation unit |
| C++ bridge ownership | RAII and ARC coexist |
| Error bridge | C++ Result → NSError/BOOL/nil/etc. |
| SwiftUI role | declarative application/editor state |
| Renderer role | GPU resources/execution |
| Main-thread rule | UI work stays on UI thread |
| Renderer callback rule | thread and ownership contract must be explicit |

---

**Blank-page mastery gate**

Reconstruct a Metal frame from nothing.

Start with:

$$
MTLDevice
$$

then create:

$$
MTLCommandQueue.
$$

At frame $N$, acquire one frame context:

$$
F_{N\bmod3}.
$$

Explain why the frame semaphore must guarantee that $F$'s previous GPU use has completed before resetting:

$$
uploadCursor=0.
$$

Then create:

$$
MTLCommandBuffer.
$$

Encode a shadow render encoder.

End it.

Encode a compute encoder.

End it.

Encode the scene render encoder.

End it.

Encode TAA/bloom.

Acquire drawable.

Encode final presentation pass.

Schedule:

$$
presentDrawable.
$$

Add completion handler.

Commit.

Then explain why the CPU can immediately begin preparing a future frame without waiting for current GPU completion.

From there, derive why this resource:

```text
CPU updates every frame
small
GPU reads
```

should usually be:

$$
Shared.
$$

Why this:

```text
large HDR render target
CPU never directly reads it
```

should usually be:

$$
Private.
$$

Why this:

```text
depth used only during one tile-local pass
never sampled later
```

can potentially be:

$$
Memoryless.
$$

Then explain exactly why:

$$
LoadActionClear
$$

can save external memory bandwidth relative to `Load`, and why:

$$
StoreActionDontCare
$$

can eliminate writeback.

If you cannot connect load/store actions to Apple's tile memory, you do not yet understand Metal performance.

Then compare:

$$
MTLFence
$$

against:

$$
VkFence.
$$

Explain why mapping them by name would be wrong.

Then compare:

$$
MTLSharedEvent(value=N)
$$

with:

$$
VkTimelineSemaphore(value=N).
$$

Then reconstruct Maveb's `FrameContext`:

$$
8\text{ MiB Shared Upload}
+
256\text{ KiB Shared Readback}
+
64\text{ MiB Private Tracked Heap}.
$$

Explain why:

$$
offset=
(value+a-1)\&
\sim(a-1)
$$

requires $a$ to be a power of two.

Then explain why:

```cpp
frame.beginFrame();
```

is safe only after that frame slot has retired.

Then reconstruct the ownership boundary:

$$
SwiftUI
\rightarrow
AetherViewport
\rightarrow
AetherViewportView
\rightarrow
AetherViewportDelegate
\rightarrow
std::unique_ptr<Renderer>
\rightarrow
MetalPtr<MTLResource>.
$$

Explain where ARC is responsible.

Explain where C++ RAII is responsible.

Explain why neither one by itself proves GPU completion.

Then explain why this bridge cast:

```objective-c
(__bridge void*)device
```

does not transfer ownership, and why the persistent C++ renderer must retain the device if it stores it.

Then explain why a Metal completion handler must not blindly mutate SwiftUI state.

Correct chain is:

$$
GPU\ completion
\rightarrow
thread\text{-}safe\ result
\rightarrow
DispatchQueue.main
\rightarrow
UI.
$$

Finally compare your two implementations.

Kairo Metal currently chooses:

$$
\boxed{
\text{shared resources + simple last-command waiting}
}
$$

for clarity.

Maveb chooses:

$$
\boxed{
\text{private GPU resources + triple buffering + per-frame arenas + async completion}
}
$$

for throughput.

The difference is not API knowledge.

It is **lifetime and scheduling architecture**.

Once you can see that, Metal stops looking like a collection of Objective-C calls and becomes a coherent execution machine:

$$
\boxed{
\text{resources}
+
\text{encoder phases}
+
\text{tile locality}
+
\text{frame ownership}
+
\text{async lifetime}
}.
$$



This is where Phase 24 becomes genuinely engine-architectural: **backend-neutral handles and resource descriptions without lowest-common-denominator design; feature/capability contracts and backend escape hatches; then the complete render-graph system—resources, passes, reads/writes, dependency inference, cycle detection, topological scheduling, lifetime intervals, transient allocation and aliasing, barrier/state inference, cross-queue dependencies, pass culling, imported/history/swapchain resources and graph debugging—followed by Metal↔Vulkan conversion boundaries, portable mesh/material/scene extraction, private native handles, shader/reflection/layout strategy, coordinate/depth convention adaptation, backend visual agreement, and designing a multi-backend renderer that shares engine intent without pretending Vulkan and Metal are identical.**


Graph connections: [[GPU workgroups and synchronization]] and [[Prefix scan and stream compaction]]. The shared spatial/order/lifetime visual is [[Chapter Edition/02 - Graphics Vision and XR/37 - Gaussian Splatting and Editable Captured Worlds#Figure ER1 - Footprint, compositing order and storage lifetime|Figure ER1]].

### Distinguish host allocation callbacks from device-memory allocation

There are two allocator conversations inside a Vulkan renderer. An application allocates and binds device memory for buffers and images. Separately, Vulkan implementation code may need host memory for its own internal bookkeeping. `VkAllocationCallbacks` lets the application supply functions for that second purpose. A host allocation callback is not a replacement for `vkAllocateMemory`, and a counter attached to that callback is not a measurement of all resident graphics memory.

The allocation function receives a requested byte count and an address alignment. **Size** answers how many usable bytes must exist. **Alignment** answers which addresses are permitted for the returned pointer. A request for 37 bytes at 16-byte alignment does not require a 64-byte logical size: the returned address must be divisible by 16 and the block must contain at least 37 usable bytes. An allocator may round its internal consumption upward, but that is its storage policy. In particular, the power-of-two requirement belongs to alignment. An extracted caption claim that assigns it to the byte count must be corrected; the original subtitle text explicitly refers to alignment.

Reallocation asks to change a block while retaining the shared prefix of its contents. If an old 37-byte block becomes a 70-byte block, its first 37 bytes must remain. If it shrinks to 20 bytes, its first 20 remain. A replacement strategy can allocate the new block, copy the smaller byte count, then free the old block. It must not free the old block before it knows replacement succeeded. Otherwise failure destroys the caller's only valid copy.

Three boundary cases deserve explicit branches. A null original pointer requests allocation behavior. A zero requested size requests free behavior. When a non-null original block cannot be reallocated, the application must leave the old allocation intact. The requested alignment for reallocation of an existing block must agree with the alignment originally used. Free must also accept a null pointer safely. These are callback semantics, rather than a reason to guess at an apparent documentation typo. They are defined in the [Khronos allocation-callback specification](https://docs.vulkan.org/spec/latest/chapters/memory.html).

A useful trace begins with a 37-byte block containing an identifier and payload. A request for 70 bytes fails: the returned replacement is null, the original pointer still owns the same 37 bytes, and the caller can retry or report failure. A later successful request returns a valid aligned block with the prefix intact; only then does ownership move and the original block cease to be valid. Finally a zero-size request releases that allocation through the specified free behavior. An adapter must preserve these cases even if its underlying allocator uses a different convention.

The source episode also distinguishes internal-allocation notifications from application-supplied allocation/free callbacks. A notification reports an event for accounting; it does not hand the engine a new memory block to destroy. Mixing those paths can double-free memory or make diagnostics look more complete than they are. Keep each counter's measured scope beside its display: hooked host requests, application device allocations, driver estimates, and resident hardware usage answer different questions.

This clarification resolves the uncertainty noted in [[Chapter Edition/03 - Game Engine/14 - Application clocks and stable input identity#Bridge Vulkan allocation callbacks to an engine allocator|the engine allocator walkthrough]]. It checks the interface contract against primary documentation; it does not certify the episode's implementation or every allocation path in Kohi.


## A generated binding is still an API boundary

A code generator can remove repetitive wrapper work, but it does not erase the difference between languages. The Sparky branch in this devlog experiments with exposing selected C++ engine operations through C++/CLI. C++/CLI is a C++ extension for working with the .NET managed runtime while interoperating with native C++ code. The language extension is not a language-neutral ABI. Its wrapper calls the native API and converts values at the boundary—for example, a managed string must be converted into the representation expected by the native C++ method. The episode also describes limiting exposure to public or protected operations and says that the branch is a learning project with substantial work remaining. Treat this as a prototype snapshot rather than a production generator or complete C# integration (Sparky Engine Dev Log #12, 00:29:20–00:30:03; source `src-a539341c398d4c55cdb1`, unit `u-486c51f3035705147bf6ac71`, with the branch status in `u-461e47c9ea075d3a77cdba67`).

This small example exposes the actual generator contract. The native API has types, ownership rules, error behavior, and lifetimes. The target language has its own strings, arrays, exceptions, and object model. A wrapper must decide which native operations are visible, how each type is represented, who owns the converted value, how errors cross the boundary, and whether callbacks can safely outlive the wrapper object. Generating a method signature is easy compared with preserving those semantics.

A reliable binding workflow starts from a narrow API surface and an explicit mapping table. For each supported native type, define its target-language form and conversion, including ownership and nullability. For each function, identify which side allocates and releases memory, how errors are reported, and whether references are borrowed or retained. Generate a small wrapper, compile it, and exercise one call in both directions where callbacks are involved. Keep generated output reproducible from the declared source and tool version; inspect representative generated code during development so an accidental mapping does not become hundreds of opaque failures.

The devlog’s filesystem section offers a parallel lesson: a virtual path such as `/models/hero` decouples an asset reference from a workstation’s physical directory, while ordered mounts determine where a request resolves. Both mechanisms insert a stable vocabulary between a client and an implementation. The vocabulary is useful only when its resolution and ownership rules are explicit. The source says packed-asset support is future work, so no archive format or package performance is implied here (00:04:01–00:05:40, unit `u-700ebba84bd2d2a5f5cabfeb`, and future-plan point `p-000011`).


A generator does not bypass C++ access control: protected native operations still need a valid class/access context and a deliberate exposure policy. A managed string conversion must specify encoding and temporary-buffer lifetime, not merely make the types compile. A generated signature is evidence of a proposed interface; a compiled wrapper, a successful call and a correct ownership/error path are separate verification steps.


### Choose the language boundary before choosing a generator

**Managed code** runs with a runtime's type and execution model; **native code** follows its compiled platform's binary interface and its own ownership contracts. C++/CLI can combine managed and native operations in a .NET bridge. That does not make a native pointer a garbage-collected owner or define how long a borrowed buffer remains valid. Microsoft documents mixed managed/native interoperability and describes C++/CLI as a Windows bridge retained primarily for existing systems. The old Sparky experiment is useful for understanding the boundary, rather than as a default architecture for every new engine. [Microsoft native/.NET interoperability](https://learn.microsoft.com/en-us/cpp/dotnet/native-and-dotnet-interoperability), [C++/CLI scope](https://learn.microsoft.com/en-us/cpp/dotnet/).

For a cross-language engine interface, another design exposes a narrow C-compatible boundary: create an opaque engine handle, perform operations through functions, return explicit status/results, and destroy the handle through its owning module. Keep C++ object layouts, exceptions and allocator assumptions behind that boundary. This does not make one binary portable across all operating systems; each target still has a platform ABI. It does make the supported calling surface easier to specify and bind. Microsoft's .NET guidance recommends a C-style boundary for ordinary C++ interoperability because C++ has no single consistent ABI across its supported compilers and platforms. Choose the actual runtime, platforms, callback needs and ownership model first; then select tooling that can preserve that contract. [Microsoft native interoperability ABI guidance](https://learn.microsoft.com/dotnet/standard/native-interop/abi-support).


### A visual description does not identify a shader operator

The 2D-API narration describes leaving a red control at one and lowering green and blue controls to 0.9. It calls the effect “subtracting” those channels “in a way.” That language describes the visible suppression of green and blue; it does not identify the arithmetic operator in an uninspected shader. A standard multiplicative tint is fully compatible with the explanation:

$$
C_{out}=C_{texture}\odot(1,0.9,0.9),
$$

where $\odot$ means component-wise multiplication in the declared shader working space. The red component is unchanged and the others are reduced. White factors $(1,1,1)$ are neutral for multiplication. A literal subtraction of those control values would have a very different neutral point, while subtraction of a separately computed offset could again look similar for a particular image.

To distinguish models, use known input texels and inspect the shader or measured output. For a channel value of $0.5$, multiplying by $0.9$ yields $0.45$, whereas subtracting an offset of $0.1$ yields $0.4$. A saturated white input may hide that difference. This small witness explains why visual similarity and casual words are weak evidence about implementation. It does not prove what the historical episode's shader executed. Preserve the mathematical teaching model and label the exact episode operation unverified until its code or a controlled test establishes it.

## A platform escape hatch must also restore process-wide state

The Linux-windowing episode gives a concrete example of why a backend boundary needs to include side effects, not only function signatures. The Kohi #005 Linux platform episode uses Xlib and XCB together for an X11 window. It calls Xlib to disable key-repeat events because repeated key presses would complicate the engine's input handling. The presenter explicitly warns that the call is global to the X server/session rather than scoped to this window. If the process exits without turning repeat back on, the user may continue to experience changed keyboard behavior outside the game. The same episode therefore adds an explicit shutdown action to re-enable repeat before destroying its window. (Kohi #005, source `src-994a368b666ebd1bf250`, units `u-81fe6c39aad8fdcf8055e768`, `u-f4883a4974cbd13730aea658`.)

The transferable engineering lesson is to model a platform call as a resource or state transition with a matching cleanup obligation. Record: what state is changed, who else can observe it, how the change is scoped, and what cleanup runs on normal close and failure. A platform adapter that exposes only `disable_key_repeat()` hides the important contract. A safer boundary either avoids changing global behavior, uses a window-scoped mechanism where available, or makes acquisition and restoration explicit and testable. If initialization fails after changing the setting, rollback still needs to occur. This is proposed API design based on the tutorial's warning, not a claim about all X11 input systems.

The episode’s choice to use both Xlib and XCB is also a reminder to distinguish protocol concepts from bindings and libraries. It demonstrates one historically specific implementation path; its remarks about documentation and deprecation are the speaker's assessment at recording time, not current Linux platform guidance. A modern engine should first identify the presentation and input contracts it needs, then select and verify supported platform APIs for its target environments. The supplied source does not establish present-day X11 recommendations or behavior under Wayland.

### Restore the state that existed before the adapter changed it

Unconditionally enabling repeat on close is not the same as restoring the user's previous setting. Xlib exposes the current keyboard controls, including global auto-repeat mode, through `XGetKeyboardControl`; its global repeat calls affect the keyboard for the selected display. Record the initial state before a deliberate global change and restore that state on the supported cleanup paths. [X.Org Xlib keyboard controls](https://www.x.org/releases/X11R7.6/doc/libX11/specs/libX11/libX11.html).

A destructor or rollback handler can organize normal cleanup, but it cannot guarantee restoration after every abrupt process termination. Global state can also be changed by another client while the application runs. These limitations strengthen the case for an input design that avoids changing shared session preferences. If a platform workaround must change them, its scope, rollback and competing-user assumptions belong in the adapter contract alongside its function signatures. This is a design qualification of the historical tutorial, not a native X11 test on this machine.

### Range count, byte capacity and API guarantees are different quantities

An array containing 32 `VkPushConstantRange` records stores 32 descriptions in host memory. It does not create 128 bytes of push-constant capacity on a device. Each record describes a byte offset, a byte size and the shader stages that use that range. Counting records and counting bytes answer different questions.

For a declared range, its end offset must fit the queried `maxPushConstantsSize`. For example, offset 64 and size 96 end at byte 160: this fails a 128-byte device limit and passes that particular extent check against 256 bytes. Alignment, stage flags and the pipeline's interface still have their own requirements. Passing one byte-count check does not validate an entire pipeline layout. [Khronos physical-device limits](https://docs.vulkan.org/spec/latest/chapters/limits.html).

The name `maxPushConstantsSize` reports this device's supported maximum; the specification separately defines a minimum capability that an implementation must provide. Vulkan 1.0 through 1.3 guarantee at least 128 bytes, while Vulkan 1.4 raises that required minimum to 256 bytes. A device can report more than its baseline minimum. Query the actual property and state the application's supported baseline before designing a block layout. A tutorial's 128-byte example is a useful portable baseline for its version, not a universal maximum for all Vulkan devices. [Khronos versions and limit changes](https://docs.vulkan.org/guide/latest/versions.html).

This distinction applies far beyond push constants: a wrapper's array length, an allocation's host byte size, an API version's guaranteed floor and a device's advertised limit are four separate facts. Keep their units and owners beside their names so a local implementation constant cannot quietly become a claimed hardware rule.

---

## Integrated Systems Continuation — From Machine State to Reproducible Execution

The systems material that previously lived in its own continuous note belongs here because the C++ and systems-API story does not end when code compiles. Bytes become machine state, machine state becomes execution, and useful execution must eventually become observable, serializable, reproducible, and debuggable. The following material is retained from the former standalone systems narrative and now continues this volume directly.


A saved game state is not just a file, and a functioning emulator is not just a switch statement. The same intellectual task connects a bytecode interpreter, a GPU command stream, an operating-system file, and a remote game server: define an observable machine, preserve its state and ownership, and explain why a given sequence of bytes causes precisely the observed transition.

This new subject volume opens Phase 28, the original 2701–2800 interval, at the real boundary after graphics and engine implementation. It begins with M163–M164 (Parts 2701–2744): CPU/6502 emulation and NES cartridge/PPU/APU timing foundations. Later M165–M168 (OS, file systems, networking, cryptography and trusted low-level systems) belong to the same volume rather than to 56 disconnected notes. Original part numbers remain recorded in [[Supplementary/Foundations#Phase 28 M163–M168 — Parts 2701–2800 deterministic and trusted low-level systems|the coverage ledger]]. The fully documented [[tools/labs/phase28_emulator_core_lab.cpp|CPU/bus reference]] is an executable teaching witness, not a complete NES emulator or a source audit of an existing NESEmulator repository.

## An executable byte acquires meaning only inside a machine

Imagine the user selecting a cartridge image. The file begins with identifying header bytes and some ROM banks. When the emulator is started, the CPU reads an instruction from the address indicated by its program counter. That byte has no meaning by itself: it becomes an instruction only under the chosen instruction-set specification and the device map that determines which memory owns the address. The state-transition view is

$$
S_{t+1}=T(S_t,\mathrm{input}_t).
$$

where the architectural state may contain the accumulator A, index registers X and Y, stack pointer SP, program counter PC, processor flags P, CPU memory, device registers, mapper state, cycle counters, interrupt requests and controller state. Determinism means that *all machine-observable state* plus the same input sequence produces the same observable transition, under a fixed machine implementation. Saving A and PC while omitting the mapper's currently selected ROM bank does not preserve the machine state.

The Nintendo Entertainment System CPU is derived from the 6502 family. A, X, Y, SP and P are 8-bit registers, while PC is 16-bit. A fetch normally reads at PC and advances PC. Decode identifies the instruction and its addressing mode, execution reads or writes operands, updates registers and flags, and assigns timing. A straightforward interpreter can implement this with a switch. A dispatch table can map opcode values to handlers. A generated micro-operation interpreter can model individual bus reads and writes. The implementation style does not guarantee accuracy; the contract is that the resulting effects and timing match the specified CPU variant. In particular, a generic 6502 and the NES Ricoh 2A03 are not interchangeable for decimal arithmetic.

A reset vector is an excellent first test of whether we understand the bus. The CPU constructs PC from the two bytes visible at addresses `$FFFC` and `$FFFD`, low byte first. Those addresses are mapped by the cartridge. On an NROM cartridge with one 16-KiB PRG bank, the bank appears twice in CPU space from `$8000` to `$FFFF`. The reset vector stored near the end of that bank can direct execution to `$8000`. A test that always hard-codes PC=`$8000` could appear to work on a toy ROM while being architecturally wrong for a cartridge whose reset vector points elsewhere.

A stack pointer does not magically reference host language call frames. The 6502 stack lives on CPU page `$0100`–`$01FF` and grows downward as SP changes. Subroutine call JSR stores a return address using the emulated stack and jumps to the callee; RTS pulls the saved address and adds one to resume after the call. This differs from a typical high-level program's description of calling a function. Interrupt entry similarly saves a return context and jumps to an interrupt vector. The saved status byte must respect the 6502-specific break/unused-bit conventions; they are not simply independent physical flags that can be blindly copied.

### Addressing is an algorithm for finding a value, not a suffix

Immediate addressing obtains the operand directly from the instruction stream. Zero-page addressing takes an 8-bit address from the stream and reads at that address within `$0000`–`$00FF`. Absolute addressing combines two operand bytes into a 16-bit little-endian address. Zero-page-indexed addressing adds X or Y with 8-bit wraparound, while absolute-indexed modes add to a 16-bit base and may cross a 256-byte page. Indexed-indirect means the index is applied to a zero-page pointer location *before* fetching the target address. Indirect-indexed obtains the pointer first and adds the index to the fetched address. Confusing these yields a program that may execute correctly for one set of values and fail as soon as a pointer crosses a page.

Relative branches interpret an 8-bit displacement as signed. The displacement is added to PC *after* fetching the branch operand. A taken conditional branch normally adds a cycle over the two-cycle not-taken path, and crossing a 256-byte page adds another. Returning the correct resulting PC is not the whole execution contract when an instruction interacts with a device that changes every cycle. For each opcode, an implementation should record its instruction bytes, addressing mode, conditional cycle penalties, bus reads/writes and changed status bits. A complete cycle-accurate emulator also needs correct ordering of the individual memory accesses; a total-cycle counter is useful but is not sufficient to reproduce all hardware behavior.

### Unsigned carry is not signed overflow

For ADC, let A be the accumulator's original 8-bit unsigned value, M the operand and C the incoming carry bit. Form the full integer sum $u=A+M+C$ and the 8-bit result $r=u\bmod256$. Set carry if $u>255$, zero if $r=0$, and negative from result bit 7. Signed overflow is a different condition: the sum of two same-sign two's-complement operands cannot be represented in the signed 8-bit range. One bitwise expression for overflow is


$$
V= ((\neg(A\oplus M))\mathbin{\&}(A\oplus r)\mathbin{\&}0x80)\ne0.
$$


A=`$7F`, M=`$01`, C=0 produces `$80`: signed overflow V=1, N=1, and unsigned carry C=0. By contrast `$FF`+`$01` produces `$00` with C=1 and Z=1 but V=0. These two independently testable cases prove why a CPU interpreter must not use one generic overflow boolean for both flags. The NES 2A03 does not perform BCD decimal ADC even if the decimal-status bit is set. An apparently minor misunderstanding here breaks arithmetic, conditional branches and some game logic.

The CPU status also includes interrupt disable I and decimal D; B and the always-set status bit have nuanced behavior when a byte is pushed or pulled. Branches read flags generated by earlier instructions. Therefore a bug in a flag update can remain latent until a much later branch, making deterministic instruction traces invaluable. A useful trace row contains old PC, opcode and operands, old/new registers, old/new P, bus side effects and consumed cycles, not just the mnemonic.

## The memory bus is a device router, not one enormous RAM array

An NES CPU address is a request to the bus, and the address decoder decides which hardware responds. The 2 KiB internal RAM occupies `$0000`–`$07FF`, mirrored through `$1FFF`. CPU addresses `$2000`–`$2007` correspond to PPU registers, mirrored across `$2008`–`$3FFF`. Selected APU and controller registers occur around `$4000`–`$4017`, while cartridge-controlled space usually includes `$4020`–`$FFFF` and, on common NROM boards, PRG ROM from `$8000` upward.

This matters because a read can have effects. Reading a controller register can advance its shift position. Reading PPU status can clear or otherwise affect status/latch state under hardware timing rules. Writing a mapper register can change what later CPU ROM reads mean at the same address. Writing `$4014` requests a DMA transfer into PPU sprite OAM; an actual transfer and CPU stall must obey the hardware cycle schedule. Representing every device register as a plain mutable byte array cannot reproduce those observable behaviors.

The reference CPU bus intentionally implements only 2-KiB RAM mirroring, NROM PRG read mapping, a controller serial interface and a simplified OAM copy. Reads from the unimplemented PPU registers return a placeholder; mapper/APU/open-bus behavior is not simulated. This is an explicit boundary, not an accidental omission hidden behind a broad claim of NES support. The CPU code increments total CPU cycles and a notional three PPU dots per CPU cycle. Such a counter can help explain relative timebases, but it cannot itself model device events at each dot.

A controller is itself a small state machine. With strobe asserted, reads return the current first-button state. After the appropriate strobe transition, reads return successive button bits, with defined behavior after the eighth read. An emulator that samples host keyboard state on every CPU read rather than latching at the prescribed event can produce non-deterministic playback even when the recorded button presses were identical.

### An iNES file describes cartridge memory; it is not executable code itself

The iNES header occupies 16 bytes and begins with the bytes `$4E` `$45` `$53` `$1A`. A classic header records PRG ROM size in 16-KiB units, CHR ROM size in 8-KiB units and flags encoding mapper number, mirroring and optional features. A 16-KiB PRG NROM bank mirrors in CPU space; a 32-KiB bank maps directly. CHR size zero commonly selects CHR RAM, not necessarily an error. The mapper determines how cartridge circuitry maps CPU and PPU addresses, including bank switching. NROM or mapper 0 is only the simplest case; a real emulator cannot accept every mapper value and treat it as NROM.

A robust parser first validates the header and supported format, then performs checked size/offset arithmetic before copying or mapping any data. If a trainer is present, its 512 bytes shift subsequent bank offsets. NES 2.0 headers encode some metadata differently from iNES 1.0, and treating one as the other can misinterpret bank size or mapper IDs. The lab deliberately rejects NES 2.0, nonzero mappers and trainers instead of guessing; it supports one or two PRG banks and zero or one CHR bank. That behavior is far preferable to silently loading a malformed image whose pointer arithmetic could exceed the file.

A parser should also distinguish content validation from ROM compatibility. Passing its supported-format checks proves that the byte layout can be interpreted as the selected subset, not that the ROM will execute under the few opcodes the test interpreter understands. The executable test uses a tiny synthetic PRG, sets its reset vector and confirms that `$C000` aliases `$8000` for a one-bank NROM fixture.

## Rendering time is not the same clock as CPU time

The PPU is a separate video-processing machine. Pattern tables supply graphics patterns; nametables refer to tile placements and attribute regions; cartridge mirroring determines how logical nametable addresses map to physical memory. Object attribute memory (OAM) describes sprites. During visible scanlines the PPU fetches background and sprite data in an order that influences pixels and can trigger status conditions. A simplified framebuffer routine cannot reproduce the same mid-frame register effects as a dot-based emulator.

For NTSC operation, a PPU scanline has 341 dots and a nominal frame has 262 scanlines. The PPU runs three dots per CPU clock, but rendering-enabled odd frames have a documented skipped PPU clock. VBlank is a precisely timed condition, and an enabled NMI can arrive near its start. Reads of PPU status near this boundary can produce edge cases that cannot be modeled by sending one event every 16.67 milliseconds from the host application.

Sprite evaluation examines OAM during specific dot windows, selects sprites intersecting the relevant scanline and has behavior such as sprite limits, overflow and sprite-zero interactions. OAM DMA transfers 256 bytes from a CPU memory page and stalls the CPU for a phase-dependent interval commonly described as 513/514 CPU cycles, with further bus arbitration nuance. The test bus copies 256 bytes but does not emulate its stall; the CPU clock counter therefore cannot be used as a real timing reference for OAM DMA. APU channels similarly derive timing from emulated CPU/APU clocks and hardware state. Merely producing audio samples at the host device's sample rate does not emulate those channels.

An accurate PPU/APU is a substantial further implementation project, with visual test ROMs, traces and timing regressions. Nothing in the CPU-only test should imply that a commercial game can render or play audio. The distinction between reference teaching and completed software is part of the curriculum's research discipline.

## Deterministic replay requires an explicit state contract

An emulator save-state is not safe to write by copying the native bytes of a C++ object. A struct may contain padding, references, pointers, compiler-dependent alignment or host-endian values. Restoring those bytes on another executable or after a code change can corrupt state or silently change meaning.

Define a durable format with signature and version, explicit byte order, validated lengths, known field order, device-state versioning and cartridge identity. A snapshot must include **every state element that affects subsequent transitions**: CPU registers, memory, mapper banks, pending interrupt state, PPU timing/latches, APU phases, controller shift position, DMA state and synchronized input history as relevant. Recording only memory and CPU registers is insufficient when the program later touches an omitted device.

The reference laboratory demonstrates a limited but inspectable step. It writes a versioned little-endian sequence containing CPU registers, PC, cycle counters, RAM and OAM. It rejects an invalid version before mutating the target CPU. It saves a state, executes another instruction, restores, repeats the instruction and checks identical PC/cycle behavior. That is reproducibility for the modeled subset; the controller, PPU, APU, mapper and interrupt state are not fully serialized, so the result must **not** be marketed as a general NES save-state implementation.

A higher-integrity replay system would include exact cartridge content hash, emulator implementation version and feature support, a monotonic input-event stream with tick timestamps, deterministic startup state, selected architecture policies, and golden traces. Compare not just the final score or one screenshot but architectural registers, RAM mutations, bus transactions, frame hashes and audio checksums at known boundaries. Most importantly, identify whether differences are expected due to an explicitly different model or are evidence of a bug.

## Reference implementation and evidence gate

The standalone [[tools/labs/phase28_emulator_core_lab.cpp|C++20 machine oracle]] executes selected instructions (LDA, ADC, STA, INX, conditional BNE, CLC/SEC, CMP, JMP, JSR, RTS and NOP), reads the reset vector, tracks nominal instruction cycle costs, tests arithmetic flags, mirrors RAM/PRG, decodes a small iNES NROM fixture, latches/shifts controller input, copies a simple OAM page and serializes a narrow versioned snapshot. It is automatically picked up by the existing C++20 multi-compiler laboratory CI because its filename ends in _lab.cpp. Tests use active throwing checks, so GCC optimized -DNDEBUG does not silently disable them.

What it does **not** implement is equally important: most 6502 opcodes, interrupt entry/RTI, precise bus-cycle sequencing, other mapper types, PPU rendering, accurate sprite evaluation, APU generation, DMA stalls, open-bus electrical behavior, full save-state completeness or commercial ROM compatibility. The original curriculum references the user's NESEmulator sources; that external repository has not been fetched or audited here.

For authoritative technical distinctions, the original source route is [NESdev CPU memory map](https://www.nesdev.org/wiki/CPU_memory_map), [CPU addressing modes](https://www.nesdev.org/wiki/Addressing_modes), [status flags](https://www.nesdev.org/wiki/Status_flag), [iNES format](https://www.nesdev.org/wiki/INES), [PPU frame timing](https://www.nesdev.org/wiki/PPU_frame_timing) and [PPU sprite evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation). This chapter records them as current documentary references, not a claim that their every example was reproduced in code. The next systems chapters extend this same state-and-byte contract to OS files, networks and security. Their existence is not silently inferred from the first emulator laboratory.


## From a deterministic emulator state to an operating-system file

The CPU program can now produce a versioned snapshot. To retain the snapshot after the interpreter exits, we must make a stronger promise: bytes become durable state even if a process terminates during writing. This continues the exact same problem rather than introducing an unrelated operating-systems glossary.

A process runs in its own virtual address space and operating-system resource context. Threads in the same process share that address space but each has its own execution stack and scheduling state. A system call crosses the user/kernel boundary to request services such as opening a file or allocating mappings. A file descriptor is a process-local reference to an open file description or resource, not a universal stable identity for a pathname. Duplicated descriptors may share file offsets; two separate opens of a path need not. Closing a descriptor releases that process reference but does not necessarily remove the named file or invalidate another process's open descriptor.

Suppose our emulator snapshot is 2,324 bytes. A low-level write may transfer fewer bytes than requested. Repeating a write operation requires accounting for how many were actually consumed, with explicit treatment of interrupted calls, zero progress, errors and cancellation. Buffered standard-library streams may accumulate bytes before sending them to the kernel; unbuffered system calls can reduce intermediate buffering but increase syscall count. Neither the presence of a buffer nor the successful return of a single write guarantees that the bytes survive sudden power loss.

Random access involves an explicit offset. A seek changes the offset used for subsequent reads/writes on certain open-file descriptions; positional I/O can name the offset per operation. File offsets and file size must be checked in types wide enough to represent the full range. Memory mapping connects a file region to process virtual memory. It can avoid an explicit copy but introduces page-fault costs, alignment requirements, mapping lifetime and risks when files are truncated or replaced. An mmap view is borrowed and must not be mistaken for an owning vector; dereferencing it after unmapping is invalid. A mapped file should not be written by arbitrary raw pointer mutation if the application assumes transactional semantics.

For a published save state, use a temporary file in the target filesystem and an explicit **commit point**. Prepare and validate the entire versioned state first; open a unique temporary path with collision-safe creation and appropriate permissions; write and handle partial writes; flush user buffers; synchronize file content if required; close or otherwise establish the chosen durability preconditions; atomically rename/replace in the supported filesystem; and, on systems that require it for crash durability, synchronize the containing directory. Atomic rename provides a visibility property under particular filesystems. It is not automatically equivalent to durable persistence after power loss.

A loader should read a completed file, never a partially published temporary. It verifies magic, format version, declared lengths and integrity before mutating the live world. If parsing fails, keep the current world unchanged. The same transactional publication idea appeared in the C++ asset registry: prepare before changing the visible owner. Here the failure boundary is a crash, not just a thrown exception.

Directory traversal is another trust boundary. A save or archive that includes paths such as parent-directory components cannot be allowed to escape a designated root. String prefix comparisons alone are insufficient when symlinks, case folding, canonicalization and concurrent filesystem changes exist. Directory-relative, handle-based operations and appropriate no-follow policies can reduce race windows, but must be matched to the target OS. File locks can coordinate cooperating writers yet may be advisory and have filesystem-specific rules; they are not a substitute for atomic publication.

This section develops Parts 2745–2755, including processes, threads, system calls, descriptors, buffered/unbuffered I/O, mmap, offsets, directory traversal, atomic rename, temporary publication and locking. No filesystem fault injection has been executed in the CPU emulator lab, so these are architectural, specification-grounded teaching claims awaiting an independent OS laboratory.

## From a file to a message on an unreliable network

A remote editor or emulator cannot transmit a pointer to its local C++ object and expect it to identify equivalent memory on another machine. It must serialize a message with a precisely stated binary format. An IP address identifies a network-layer endpoint; a transport port identifies a service endpoint within the host's networking stack. A socket is a process-facing interface whose lifetime and errors are managed separately from any application messages.

A UDP send produces a datagram, preserving a message boundary in the receiving interface, but UDP provides no general guarantee that the packet is delivered exactly once, in order or at all. Packet loss, duplication, delay and reordering must be considered if the application wants ordered reliable state updates. TCP provides an ordered reliable byte stream while the connection remains healthy; it does not preserve application message boundaries. One send of 1,000 bytes may arrive through several reads, and multiple sends may be coalesced into one read.

A practical protocol therefore defines its own framing. For example, a frame could contain a four-byte big-endian payload length followed by a versioned message body. The receiver reads the entire length field, validates the bound before allocating memory, and then repeatedly reads until the declared body length has arrived or the stream fails. Never trust an attacker-controlled length that could overflow arithmetic or allocate gigabytes. Declare the maximum frame size, permitted message types, byte order, header validation, deadline and cancellation semantics. A disconnected peer in the middle of a frame is not a valid empty message.

Network byte order conventionally means big-endian for the relevant transport header fields; application formats must still declare their own endianness. Packet serialization must avoid copying host struct padding or raw pointers. A transport checksum can detect certain accidental corruption but does not authenticate the sender or prevent an attacker from altering data and recomputing the checksum. Any integrity and authenticity requirement must be addressed separately.

Latency and bandwidth measure different things. For payload size B and sustainable throughput R, B/R is a lower-bound transfer component, while round-trip delay, queueing, flow control, sender/receiver CPU work and serialization all contribute. A 1-KiB control message can feel slow on a high-bandwidth network if it must wait for multiple round trips. A giant snapshot can take substantial transfer time on a low-bandwidth connection despite low propagation latency. Benchmark end-to-end response under representative network conditions, not just a link's nominal speed.

When networking an interactive simulation, the client may predict local motion to hide round-trip delay. The server's authoritative state can later differ from the client's prediction. Reconciliation requires a common tick counter, acknowledgment of processed inputs, a retained input history, restoration of the authoritative state and replay of unacknowledged inputs. Physics solver ordering, floating-point modes, source revisions and time step become visible networking design choices. Smooth visual interpolation can hide corrections, but cannot make inconsistent state transitions mathematically identical.

Parts 2756–2770 therefore emerge from the need to transfer and reconcile an emulator or world snapshot. There is no tested socket, UDP-loss harness or TCP partial-read implementation in the current lab; those have been authored as contracts, not certified as completed network code.

## From received bytes to a trust decision

Once a remote peer sends a snapshot, parser validity alone is insufficient. We must answer whether the bytes were altered, whether the sender is authorized, whether the snapshot may be replayed, and whether the application should expose its contents. Cryptography helps with some of these questions, but secure system design also depends on identities, access policies and key custody.

A cryptographic hash deterministically maps an input byte sequence to a fixed-length digest. SHA-256 produces 256 bits, but different security properties should not be collapsed into one vague 'secure hash' label. **Preimage resistance** concerns finding an input for a given digest, **second-preimage resistance** concerns finding another message matching a given message's digest, and **collision resistance** concerns finding any pair of messages with the same digest. A random accidental collision estimate is not itself a proof against a targeted cryptanalytic attack. A bare SHA-256 checksum of a downloaded file does not authenticate the file if an attacker can replace both the file and its expected checksum.

An HMAC uses a secret key with an underlying hash to authenticate a message. It does not hide message contents. A key derivation function turns suitable initial key material into new keys for defined purposes. HKDF, specified by RFC 5869, separates an extract stage from an expand stage. Given input key material IKM and a salt, it derives PRK using HMAC. To expand, it generates blocks

$$
\begin{aligned}
T(0)&=\epsilon,\\
T(i)&=\operatorname{HMAC}_{\mathrm{PRK}}
\bigl(T(i-1)\parallel\mathrm{info}\parallel\operatorname{byte}(i)\bigr).
\end{aligned}
$$

then concatenates blocks until reaching the requested length. Here epsilon denotes the empty byte string, and byte(i) is one counter octet for i from 1 through 255, not the decimal text of i. RFC 5869 limits the requested output to 255 times the hash-output length in bytes. [HKDF expansion specification](https://www.rfc-editor.org/rfc/rfc5869.html#section-2.3). The information string helps separate cryptographic contexts, so a key intended for snapshot authentication need not coincide with one intended for a different protocol purpose. This is not a substitute for a password-hardening KDF or sufficient entropy in initial key material.

A symmetric encryption key protects confidentiality under an appropriate scheme. AES is a block cipher, while GCM defines authenticated encryption using counter-mode encryption and a polynomial tag. An AES-GCM application must ensure nonce uniqueness for a given key, validate authentication tags and apply the associated authenticated data (AAD) consistently. Reusing the same nonce under the same GCM key can expose relationships between plaintexts and undermine authenticity. A process-local nonce counter reset to zero at every restart is unsafe unless rekeying and global uniqueness are properly designed. AAD can authenticate metadata such as protocol version, message type or resource identifier even when those fields are not encrypted.

Authenticated decryption must not be turned into 'decrypt, parse, execute, then check the tag.' The appropriate API's authentication success must be established before untrusted plaintext is treated as authoritative. Error paths must not disclose keys or sensitive payloads in logs, and nonce/key lifecycle must survive crashes. Established cryptographic libraries and protocols are required; writing a fresh AES-GCM algorithm inside an educational game-engine repository would create unnecessary security risk.

Public-key methods solve different problems. A verified signature can authenticate a message relative to a trusted public key; key exchange can establish shared secrets to feed through a KDF; public-key encryption can provide confidentiality under a recipient's key. None automatically proves that the public key itself belongs to the claimed person. Trust establishment, certificate identity, revocation, replay prevention and authorization remain separate.

A Shamir secret-sharing threshold scheme places a secret s as the constant term in a random degree-(t−1) polynomial over a finite field:

$$
f(x)=s+a_1x+\cdots+a_{t-1}x^{t-1}\pmod p.
$$

Each share is a distinct pair (x_i,f(x_i)). Any t valid shares can reconstruct s by polynomial interpolation; under the ideal model fewer than t reveal no information about the secret. This requires secure random coefficients, distinct nonzero x values, a suitable field and correct encoding. Authentication of shares and handling of dishonest participants are additional system concerns. Floating-point polynomial fitting is not a substitute for finite-field threshold cryptography.

A Merkle tree recursively hashes encoded leaves and internal nodes to commit to a set or ordered sequence of records. An inclusion proof contains the necessary sibling hashes and left/right positions to recompute the root. Verifying the proof establishes membership *relative to a trusted root*, not that the root was issued by the right authority, is current, or contains truthful records. Append-only audit logs need further protection against rollback and split-view attacks.

A token vault maps sensitive values to surrogates under explicit authorization and retention policies. Format-preserving encryption retains syntactic structure under its own security assumptions but can leak structural information through permitted domains. Redaction must propagate through all views and storage that expose the sensitive value; hiding characters in a UI while retaining the value in logs or search indexes is not comprehensive redaction. A trust engine can return allow, redact or deny with an auditable reason, a default policy and a clear identity boundary.

The last interface is often between implementation languages. Rust-to-Python FFI through PyO3 changes who may borrow memory, which interpreter state must be held, how errors cross the boundary and whether a panic may unwind through foreign code. A Rust raw pointer crossing into Python or C++ does not automatically inherit Rust's ordinary borrow-checker protections. Define allocation/free responsibility, string encoding, object lifetime, thread rules and exception conversion. A Python wrapper can be memory-safe on its own side while still calling a native library after the referenced buffer has been freed.

Security threat modeling ties the pieces together. Identify attackers, assets, entry points and trust boundaries. A correct cryptographic primitive does not prevent arbitrary path traversal, bad access control, unsafe plugin loading, unbounded allocation, replayed authorization or accidental logging of secrets. Conversely, a safe parser does not by itself authenticate a received packet.

Parts 2771–2800 cover SHA-256/HMAC/HKDF, AES/GCM and nonce discipline, public keys, Shamir shares, Merkle proofs, tokenization, FFI, trust policy and a deterministic low-level-systems capstone. **Their present state is fully connected authored explanation, not a tested encryption engine or a security certification.** The CPU-only C++ lab does not claim those operations were executed.


---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/17 - Python and Numerical Tools for Graphics Systems|Next chapter]]
