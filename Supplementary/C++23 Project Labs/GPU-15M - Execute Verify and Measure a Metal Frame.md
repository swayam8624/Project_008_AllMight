### GPU-15M — Execute, verify and measure an actual Metal frame

The strongest way to understand Apple's Metal is not to memorize a long set of types and guess how they map to Vulkan names. We already built a C++23 account of resources, queue requirements, frame ownership, command submission and retirement. Pass 15 now ties those contracts to **a real Metal graphics-and-compute execution path**, while keeping CPU correctness models useful on machines that do not have a Metal device. The primary production source is **Maveb**, with its existing multi-pass Gaussian/mesh renderer and triple-buffered `FrameContext`. The second source is **KairoRenderer**, whose Objective-C++ detail backend renders to an offscreen viewport, handles window presentation and supports readback. They are distinct working architectures that show us different aspects of the same problem.

An NES emulator remains available when its binary/CPU constraints are relevant, but it is not the right project for this chapter. Metal storage and tiling are exposed more authentically by Maveb's real frame heaps, Gaussian splat buffers, mesh rendering, temporal passes and capture path. GPU compute correctness is naturally illuminated by the **collision-pair problem** from Kyoto/KairoPhysicsEngine: an independent CPU AABB-intersection oracle can check every candidate pair. Frame timing and dynamic pixels later become central for XR/foveated rendering. This chapter will use all of those where they give different evidence, rather than pretending a tiny emulator overlay covers every graphics concept.

The original Chapter 15 is already approximately **3,061 lines / 47,241 characters**, including Metal's core execution model, encoders, pipeline states, resource and shader binding, storage modes, tile behavior, heaps, drawables, synchronization, profiling and Apple ownership boundaries. That material is **not being shortened or replaced**. The Pass 15 application section is staged before the original `### STORAGE MODES, TILE MEMORY, DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION` boundary, so the reader first encounters the original foundational Metal execution narrative, then this executable bridge, and then the original deep storage/tile/presentation treatment continues in full.

This pass adds four source files. `tools/labs/gpu_course/metal_frame_contracts.hpp` is a C++23 **portable reference model** for three frame permits, checked upload slices, command terminal states, capture rows and valid versus unavailable timestamps. `tools/labs/gpu_metal_frame_cpp23_lab.cpp` exercises that model with Maveb, Kairo, collision and XR scenarios plus adversarial failures. `tools/labs/gpu_course/native_metal_compute_probe.mm` is a **real Objective-C++ macOS Metal program** capable of compiling and executing original MSL shaders, drawing to an offscreen render target and verifying GPU readback. `tools/labs/gpu_course/native_metal_unavailable.cpp` is an explicit unavailable adapter used when Objective-C++/Metal is not selected by CMake. The build keeps the ordinary CTest portable; real GPU execution is opt-in and must be reported separately from a synthetic CPU success.

#### An actual frame is a dependency chain, not a command-buffer-shaped object

A Metal frame has at least six meaningful stages. The application obtains scene state, camera data, geometry and any newly required GPU resources; the CPU packages the relevant bytes for the next submission; encoders record draw/dispatch/blit commands; the command buffer commits into a queue; the device executes and reaches a terminal status; and any output used by the CPU is read only after the required completion and storage-mode operations. A windowed frame adds drawable acquisition, render-pass attachment setup and a presentation operation, whose time and ownership are different from the GPU commands that generate image content.

The basic object chain is familiar:

```text
MTLDevice
    ├── MTLCommandQueue
    │       └── MTLCommandBuffer
    │               ├── MTLComputeCommandEncoder
    │               ├── MTLRenderCommandEncoder
    │               └── MTLBlitCommandEncoder
    ├── MTLBuffer / MTLTexture / MTLHeap
    ├── MTLLibrary (compiled MSL shader functions)
    ├── MTLComputePipelineState
    └── MTLRenderPipelineState
```

Those arrows express creation and use, not a promise that one Objective-C object automatically retains every referenced native allocation on every Metal API generation. The resource-ownership behavior of classic `MTLCommandBuffer` differs in important details from Metal 4's core API. As in our Vulkan chapter, always ask which object owns storage, who borrows the native handle, and what actual completion proof prevents reuse or destruction.

The command encoder types are not interchangeable. A compute encoder uses a compute pipeline and a thread-grid/threads-per-threadgroup decomposition. A render encoder needs a render-pass descriptor whose attachment formats, sample counts, load/store actions and pipeline render state are compatible with its intended drawing. A blit encoder copies or fills resources and can perform certain transitions or synchronization operations. Recording a command in an encoder is a **CPU operation** that describes future GPU work; committing the command buffer makes the recorded work eligible for execution. Calling `commit` does not say that a pixel has been produced yet.

A successful command buffer can also produce the **wrong output** if its inputs, shader math, bounds or resource bindings are wrong. Therefore a completed terminal status is a necessary execution condition, while an independently checked output is a separate correctness condition. We explicitly verify both, rather than treating a GPU elapsed time as proof that a Gaussian image or collision-pair mask is correct.

#### The real Maveb frame context is a resource allocator with a lifetime contract

In the inspected Maveb source, `engine/metal/src/FrameContext.cpp` creates an **8 MiB shared upload buffer**, a **256 KiB shared readback buffer** and a **64 MiB private transient heap** per frame context. It stores an upload cursor and a readback cursor. New slices are aligned upward and allocated from the same Metal buffers using offsets, rather than allocating one whole Metal buffer for every small camera uniform or shader parameter block. The transient heap is marked private, and the heap's hazard-tracking mode is explicitly configured.

This has several benefits. CPU encoding can refer to a stable allocation while using different subranges for per-frame inputs. The allocator does not repeatedly enter the system resource allocator for each small upload. Memory can be budgeted by context. A readback region can be kept separately from CPU-to-GPU staging. But each benefit depends on the frame-context reuse protocol being correct.

`FrameContext::beginFrame()` invokes deferred cleanups, clears the cleanup list and resets both arena cursors to zero. Those operations are inexpensive; they are safe only after the GPU and any asynchronous reader have finished referring to the preceding frame's slices. A cursor reset is **not** a memory synchronization primitive. If any device work still reads the prior bytes, the next CPU write can race with the device and corrupt a frame. If a readback handler retains the prior readback slice, resetting and overwriting it can corrupt a capture even after the original shader has finished.

Maveb's renderer surrounds frame-context use with a three-permit dispatch semaphore. It waits for a permit before selecting a frame slot and resetting its arena, and a command-buffer completed handler returns a permit. This is a real coordination mechanism visible in the source; we should not replace it with naive `frameNumber % 3` indexing and assume the modulo expression proves reuse is legal. A generation-checked frame ID adds protection against late callback identity confusion, but the actual native completion remains the decisive permission to recycle storage.

The allocator's power-of-two alignment expression is the familiar

\[
\operatorname{alignUp}(x,a)=(x+a-1)\;\&\;\sim(a-1)
\]

for nonzero power-of-two \(a\). It is not safe to evaluate the addition blindly when \(x\) is near the maximum representable unsigned integer. The existing Maveb source explicitly rejects invalid alignment and checks cursor overflow before calling this helper. Our portable laboratory independently derives the precondition `x <= SIZE_MAX - (a - 1)` and refuses the operation before computing an overflowing sum. Then it validates that the aligned offset and requested size both fit within the allocated capacity, using subtraction in a range where overflow cannot occur.

The correct sequence is therefore *validate alignment → validate arithmetic → align cursor → validate capacity → copy data → publish new cursor*. A failed alignment or capacity check must not advance the cursor or partially overwrite previously published bytes. That is the same prepare/commit pattern from earlier C++ chapters, now applied to production-style GPU staging.

#### A frame permit is not a successful image

The portable `MetalFrameModel` represents each frame slot using an owner, index, generation, project tag, phase, upload arena and cursor. It starts FREE, becomes RECORDING when a caller acquires it, then ENCODED and SUBMITTED. An accepted or potentially accepted command cannot have its upload memory overwritten. A known failure before commit can safely cancel a draft. If acceptance is uncertain, the model quarantines the frame until the backend establishes whether the command was accepted. This is intentionally conservative.

Once the **real backend** supplies a terminal status, our CPU model may release its permit. But the terminal status can be **COMPLETED** or **ERROR**. Both may release storage after the command buffer's terminal lifetime and reference obligations are satisfied; only the former can publish a success-shaped capture or verified timing record. A bad frame must not be called successful merely because a completion callback ran. The callback indicates a terminal event, not necessarily successful execution.

Maveb's inspected completed handler computes `GPUEndTime() - GPUStartTime()`, stores a millisecond figure and increments its completed-frame count before signaling a permit. Its frame-capture handler separately processes readback data when the command buffer is not in an error state. Those mechanisms are valid reasons to study terminal status, timestamp quality, owner lifetime and separate capture correctness. They are not proof that every command buffer produces nonzero GPU timestamps on every supported device.

Our laboratory makes a missing or invalid measurement explicit: if a reported GPU start/end pair is zero, nonfinite, reversed, equal, or absent, it records **timing unavailable**, not a fabricated zero-millisecond GPU cost. If a command buffer terminates in error, no successful pixel capture or GPU duration is emitted. If the handler is invoked twice by a buggy adapter, the old frame generation is already stale, so it cannot release the same permit twice or overwrite the next frame's upload arena.

In a production Metal adapter, the command-buffer completion handler may run on a thread different from the renderer's main/UI thread. A C++ shared_ptr can keep some state alive but does not automatically make mutable vectors, counters or SwiftUI state thread-safe. An event queue or actor boundary must transfer the completed record to the right execution context. The portable model deliberately uses explicit, single-threaded completion observations; it does not fabricate an asynchronous callback or pretend that its host vector is a lock-free production message queue.

#### Three permits can be independently occupied by distinct projects

The test begins one frame for a Maveb Gaussian workload, one for Kairo scene rendering and one for a Kyoto collision-pair workload. All three receive independent staging slices. The first Maveb frame stages 19 bytes with 16-byte alignment, then another 15 bytes at 32-byte alignment. The second allocation must start at offset 32 rather than immediately after byte 18, because the next 32-byte boundary is required by the stated staging contract. Kairo and Kyoto use their own cursors, so an allocation made into one frame must not silently change the other frames' offsets.

Once all three are committed, a fourth frame request for XR gaze work must fail with a typed no-free-slot result. The test then supplies a simulated terminal **error** for Kairo's command and confirms that one permit becomes free but no successful image or GPU timestamp is emitted. A new XR frame can reuse that slot, but its generation is incremented and a stale Kairo frame token is refused. Finally, the other frames complete in a deliberately different order from submission, and the events retain their correct workload tags and identities.

This matters because real graphics engines are not a sequence of synchronously processed functions that always finish in the same wall-clock order the CPU begins them. Multiple queues, asynchronous uploads and graphics presentation can create overlapping work. Even on a single command queue, completion events can be delivered to application observers at different times. We need a stable identity and a proof of terminal completion, not a guess based on frame arithmetic.

The same frame contract works when the payload is very small or very large, but its capacity policy may need changing. A huge Gaussian attribute upload can fill an 8 MiB per-frame arena; a collision-pair list may require a dynamically bounded output; a gaze-dependent XR pipeline may need tighter latency and fewer queued frames. The *existence of three permits* does not prove that three is an optimal count for every workload.

#### CPU elapsed time and GPU execution time measure different spans

A user who sees "2.4 ms/frame" wants to know *which* time interval is being reported. A correct measurement should distinguish at least CPU encoding/submit time, command-buffer GPU execution time, end-to-end latency until available output, and presented-frame cadence. These values are related but not equal.

Let \(t_{\mathrm{encode}}\) be the host's time spent constructing and committing commands, \(t_{\mathrm{GPU}}\) the interval between the GPU execution start and end events, \(t_{\mathrm{ready}}\) the latency until the result can safely be consumed by the CPU, and \(t_{\mathrm{present}}\) the time until a rendered image participates in the actual display/presentation pipeline. In general,

\[
t_{\mathrm{encode}}\ne t_{\mathrm{GPU}}\ne
t_{\mathrm{ready}}\ne t_{\mathrm{present}}.
\]

Different work may overlap. CPU encoding and GPU execution of distinct frames can proceed concurrently. A command may spend time waiting in the device scheduling queue after CPU commit. Presentation may wait for vsync, drawable availability, compositor decisions or other frames even after command-buffer execution completes. None of these intervals should be added or compared without explaining what they contain and whether they overlap.

The native Metal test uses `std::chrono::steady_clock` to measure host time from just before command encoding to immediately after `commit`. It separately reads `GPUStartTime` and `GPUEndTime` only **after terminal command completion**. Apple documents that these GPU timestamp properties can remain zero until the command finishes. Their difference describes a GPU command-buffer interval, not end-to-end latency to pixels on a user's screen, and it is not necessarily equal to time spent exclusively in one shader kernel.

In ordinary `--probe-native` mode, the test performs one successful native command buffer and reports its correctness; any available GPU duration is explicitly labeled a **diagnostic sample**, not a benchmark. In explicit `--benchmark-native` mode, the test performs five warmups and thirty independent measured command buffers. Every measured command still checks CPU/GPU correctness before admitting its timing sample. The output reports nearest-rank median and 95th percentile for host encode-to-commit time and, only when all samples have valid GPU intervals, comparable GPU command intervals.

For \(n\) samples sorted as \(x_{(1)}\le\cdots\le x_{(n)}\), the reported nearest-rank percentile is

\[
P_p = x_{\left(\max(1,\lceil pn\rceil)\right)}
\]

for a fraction \(p\in(0,1]\). This statistic is easy to reproduce, but **thirty samples from one run do not establish a confidence interval**, a portable performance claim or causality between a refactor and a speed change. Clock variability, thermal behavior, shader compilation/warmup, GPU frequency, other processes, instrument overhead, tile behavior and workload size can all change the distribution.

Meaningful Maveb/Kairo comparisons should use the same scene, frame resolution, shaders, render options, camera path, device, SDK version, build configuration and measurement interval. For Gaussian rendering, also record splat counts, visible splat distribution, tile overlap, sort/worklist sizes and output image correctness. For a physics broad phase, record object count, overlap density, exact-pair differences and motion regime. For XR, track motion-to-photon and gaze-to-render age, not just command-buffer GPU time. A single native 3×2 offscreen pass cannot validate any of those production workloads.

#### Why we inspect both Maveb and Kairo source rather than guessing an “Apple approach”

Maveb's inspected `Renderer.cpp` has an actual multi-pass frame: Gaussian rendering, shadow encoders, temporal work, bloom and presentation appear in the source. It allocates a capture buffer, uses a blit encoder to copy a drawable texture, attaches a completed handler to produce a `FrameCapture` and returns the frame-capacity semaphore in a separate completion handler. The capture code deliberately handles row pitch; it does not assume every GPU row occupies exactly \(width\times4\) bytes. It also records the GPU command-buffer start/end timestamps into an atomic field.

KairoRenderer's `MetalRenderer.cppm`, by contrast, routes a renderer-neutral public frame through `detail::MetalBackend`. The actual `detail/MetalBackend.mm` implementation creates a command queue, records a scene and presentation pass, commits a Metal command buffer, and retains the last command so a subsequent capture can wait for its terminal execution before inspecting texture data. The public wrapper exposes viewport picking, capture, object IDs and render-graph profiling. This provides a simpler synchronous capture discipline than Maveb's callback-oriented in-flight frame system.

Neither architecture is universally superior. Kairo's explicit wait in capture may simplify the validity proof while introducing a CPU stall at the moment capture is requested. Maveb's asynchronous capture can avoid that immediate blocking wait but must retain the capture buffer, metadata, renderer and completion-thread ownership safely until the handler finishes. Their storage-mode choices, framebuffer formats and scene workloads differ. Comparing them honestly requires both the performance data **and** the conditions under which each result remains valid.

Kairo's source-level Metal backend is especially valuable for teaching Objective-C++ boundaries: one C++ module exposes renderer behavior, while a detail implementation owns Objective-C Metal resources. That separation keeps common engine code from depending on Apple frameworks and allows independent Vulkan/Metal implementation paths. Maveb's Metal-cpp approach instead uses NS:: and MTL:: C++ wrappers in its core renderer. We must explain retain/release/autorelease, object ownership and callback lifetime within the actual abstraction chosen, not mechanically mix Objective-C ARC with manual metal-cpp `release()` calls.

#### A GPU capture has a row pitch, byte format, origin and color space

One of the easiest mistakes in a graphics test is to inspect readback bytes without knowing which pixel they represent. A texture has a format, an extent, an origin convention, possible multisampling, row pitch and possibly separate color-space encoding. A CPU byte array alone does not reveal those properties.

Maveb's capture path copies from a drawable texture into a shared buffer, aligns the row stride to a 256-byte boundary, and then copies each *compact row* into an owned result. The relevant expressions are

\[
B_{\mathrm{row}}=4W,
\qquad
B_{\mathrm{padded}}=\operatorname{alignUp}(B_{\mathrm{row}},256),
\qquad
B_{\mathrm{copy}}=B_{\mathrm{padded}}H.
\]

These formulas assume a four-byte-per-pixel texture representation, such as BGRA8, and an intentional 256-byte row alignment for that transfer. They are not a universal row-stride formula for compressed, planar, HDR or other formats. A robust implementation derives byte count from the chosen pixel format and supported copy-layout requirements before allocating the readback buffer.

The CPU lab tests a **3×2 BGRA8 image**. Its tight output has only \(3\times2\times4=24\) bytes, but each source row uses 256 bytes, so the padded input needs 512 bytes. The compactor copies exactly twelve bytes from row zero and twelve from row one, skipping 244 bytes of padding after each row. It refuses an input whose pitch is less than the compact row, whose pitch does not satisfy the declared alignment contract, whose buffer lacks the required padded length, or whose dimensions/size arithmetic overflow.

The compactor also preserves **BGRA** byte order; it does not silently swap into RGBA. It does not infer sRGB, HDR transfer function, premultiplied alpha, texture origin or color-management conversion. Those properties must be carried through capture metadata and handled explicitly by consumers. A screenshot that has every byte in bounds may still be visibly wrong if it swaps red and blue channels, interprets linear values as sRGB or inverts the image vertically.

In the native test the offscreen render target is explicitly `MTLPixelFormatBGRA8Unorm`. A minimal original vertex/fragment shader draws an opaque red fullscreen triangle into a **private** 3×2 texture. A blit encoder copies it into a **shared** buffer with 256-byte row pitch. After the real terminal command-buffer completion, the test verifies every pixel in the compact image corresponds to BGRA \([0,0,255,255]\). This is an **actual render-encoder plus blit-encoder GPU correctness witness**, not merely a test that a clear color was assigned to a CPU array.

A separate native compute shader processes **32 AABBs** and writes an explicitly bounded \(32\times32=1024\)-element candidate-overlap mask to a shared output buffer. The CPU independently recomputes every unordered pair's half-open rectangle intersection and compares all 1024 mask entries. Sixteen additional guard words must remain unchanged, so an out-of-range GPU write becomes visible to this test. This is a small exact-pair witness in the spirit of the Kyoto research, not a benchmark or implementation of the Kyoto learned controller, Persistent SAP or Global Guard.

#### Why half-open AABB overlap is an ideal GPU/CPU oracle

A broad-phase collision detector often works with bounding boxes whose componentwise extents describe an axis-aligned rectangle or volume. For two nondegenerate 2D AABBs \(A\) and \(B\), strict overlap under a half-open intersection convention is

\[
A_{\max x}>B_{\min x}
\land B_{\max x}>A_{\min x}
\land A_{\max y}>B_{\min y}
\land B_{\max y}>A_{\min y}.
\]

If edges merely touch, strict inequalities classify the boxes as nonoverlapping. Different simulation engines can choose inclusive contact semantics, especially when tolerance or resting contact matters; the point is to state the convention **before** evaluating correctness. The native MSL shader emits a one for a pair only when \(i<j\) and the rectangles strictly overlap; the CPU oracle recomputes the same declared mathematical predicate using an independent C++ implementation and checks every output index.

Why not trust only a checksum? A checksum can tell us that two byte sequences differ, but it cannot show which collision pair is missing or fabricated, or whether a buffer overflow overwrote adjacent data. The test reports the first mismatched pair index and expected versus observed value. It also checks the output allocation's guard region. A future GPU compaction stage may emit pairs in a different order from the CPU oracle; then compare a sorted or canonicalized **set of pairs**, not merely raw output order, while still validating capacity and duplicates. Our introductory mask has a canonical row-major index, so byte-for-byte comparison is appropriate.

The native GPU work is not deliberately optimized. Its 1024-thread mask includes many noncolliding pairs and does \(O(n^2)\) comparisons, whereas a genuine high-performance broad phase should avoid naive all-pairs on large sparse scenes. This is intentional: a tiny exhaustive kernel is an excellent independent **correctness oracle** against which a future optimized GPU worklist algorithm can be checked. We must never publish its timing as evidence that all-pairs scales better than a spatial hierarchy.

#### Real Metal shaders are not interchangeable with Vulkan pipelines

A Vulkan shader path typically compiles GLSL/HLSL into SPIR-V or uses another supported shader representation, binds descriptor sets and pipeline layouts, and expresses resource access and image layout requirements through Vulkan objects and barriers. Metal uses MSL functions in an MTLLibrary, MTLRenderPipelineState or MTLComputePipelineState, and command encoders with their own resource binding methods. It is not generally correct to copy SPIR-V bindings into MSL and assume offset, alignment, visibility and resource lifetime will match.

The native probe compiles original small MSL functions from source using `newLibraryWithSource`. It looks up its compute, vertex and fragment entry points by name and creates the corresponding pipeline states. A missing library, missing function or failed pipeline compilation is a **test failure with the NSError description**, not a silent software fallback. The compute shader receives the exact AABB array, output buffer and logical object count through buffer bindings; the fragment shader is used only in a render pipeline whose color attachment format matches the actual offscreen texture.

The correct shader thread dispatch needs both a total grid and a valid threadgroup shape. `maxTotalThreadsPerThreadgroup` constrains the chosen group size, but hardware thread scheduling and occupancy depend on many other properties. Our native test chooses no more than 64 threads per group and dispatches a bounded 1024-thread grid. The shader explicitly bounds its global thread ID against the output size before writing. This avoids assuming that dispatch dimensions, SIMD-group width and data length are always equal.

The render path also has a dependency. It writes the offscreen color texture through a render encoder, then reads that texture in a blit encoder that copies it to a CPU-visible buffer. Metal's command buffer/encoder ordering and hazard tracking apply under the selected resource tracking mode; an engineer must not bypass them with unsupported aliasing, an untracked heap or manual resource reuse. The native test uses ordinary tracked resources and a straightforward one-command-buffer sequence, not an undocumented private placement optimization.

#### Shared, managed, private and memoryless storage are design choices

The inspected Maveb `FrameContext` uses **shared** upload/readback buffers because the CPU must fill or inspect those buffers. Its transient heap uses **private** storage because its resources are primarily GPU-owned. That distinction is a workload access policy, not a slogan that all Apple silicon memory is equivalent. Apple silicon has unified physical memory, but the Metal storage-mode choice still affects legal CPU access, allocation behavior, memory management and performance.

On some Intel/discrete Mac configurations, **managed** storage is relevant: CPU and GPU copies can require explicit `didModifyRange` or blit synchronization in the direction appropriate for the access. Those contracts differ from shared memory on Apple silicon. **Private** textures and buffers generally cannot be dereferenced via CPU `contents`; device commands must perform copy/readback into a host-accessible resource. **Memoryless** storage is suited only to certain transient render-pass attachment usages on supported devices and cannot be treated like persistent sampleable storage.

The native probe selects a private render target and shared input/output/readback buffers on devices that support those choices. If the device cannot provide the required resources, it reports **UNAVAILABLE** rather than treating a null Metal buffer as a successful capture. It does not use `contents` on the private offscreen texture. This is a minimal but meaningful hardware-backed assertion of resource accessibility.

An Apple GPU's tile-based rendering also changes performance choices. A render-pass load action determines whether previous attachment data must be preserved or the pass can clear or ignore it. Store actions decide which attachment contents must survive after the pass. Memoryless render targets can avoid certain external storage traffic when their data is not needed later. If a transient depth or G-buffer is sampled in a later pass, however, its necessary contents must survive in a supported storage arrangement. A rendering pipeline that changes store actions purely to improve timing without checking subsequent shader reads can gain apparent speed by simply producing incorrect output.

Our 3×2 render target chooses explicit `MTLLoadActionClear` and `MTLStoreActionStore` because the later blit must read the rendered image. That choice makes the dependency and output proof simple. It is not a benchmark of tile memory efficiency; a meaningful tile-memory experiment would compare validated render-pass architectures on the same scene and hardware with actual capture/profiler data.

#### Capturing a drawable is not the same as rendering offscreen

Maveb's actual capture requests can toggle `framebufferOnly` on the MTKView to permit operations that are not compatible with framebuffer-only usage, then copy pixels from the presented drawable using a blit encoder. A real application needs to handle the drawable's availability, pixel format, current render-pass descriptor, window resize and lifetime through presentation. An offscreen texture, by contrast, is created directly by the device and has no CAMetalLayer/MTKView presentation timing.

Our opt-in native test intentionally creates an **offscreen** texture. This avoids requiring a window server or a UI thread in the test runner while still verifying genuine vertex/fragment execution and texture-to-buffer blitting. The output is a real GPU-rendered image, but it is not proof that a CAMetalDrawable was acquired, presented, composited or displayed. A subsequent Kairo/Maveb integration test should exercise an actual application view, check drawable size changes, pixel format and presentation errors, and capture known scene output under reproducible conditions.

This distinction is particularly important for XR. A device can execute a compute shader quickly while still missing the end-to-end display deadline because eye tracking, pose updates, late-latching, compositor scheduling and display scanout add latency. If a benchmark reports only `GPUEndTime-GPUStartTime`, it has measured a useful subinterval—not motion-to-photon latency.

#### Objective-C++ and C++23 have different ownership layers

The portable header is ordinary ISO C++23 and compiles on GCC/Clang without Apple frameworks. Its finite state machine uses `std::expected`, `std::optional`, typed frame handles and owned event records. The native adapter is Objective-C++, compiled as a `.mm` translation unit with ARC enabled when an Apple Objective-C++ toolchain and both Metal/Foundation frameworks are found. It uses `id<MTLDevice>`, `id<MTLCommandQueue>`, `id<MTLBuffer>` and related Objective-C protocol types inside an autorelease pool.

ARC manages its Objective-C object ownership under its rules; the C++ containers and scalars still obey their own construction/destruction semantics. An Objective-C block can retain captured Objective-C references, but a raw C++ `this` pointer is not thereby made safe for an arbitrary late callback. For a production renderer, define a stable owned completion state or weak owner token and ensure it outlives the GPU completion handler or that destruction drains all pending callbacks before releasing it. Maveb's existing destructor waits for its frame permits, which is meaningful evidence of such a lifecycle design; an audit should still inspect all early exits and error paths rather than declare it correct from one line.

The native probe uses `waitUntilCompleted` instead of adding an asynchronous callback. This is intentional for a **bounded correctness experiment**: it makes the terminal status and mapped output check unambiguous. It also means its measured host/command timings must not be interpreted as the throughput of Maveb's real triple-buffered asynchronous frame loop. A future native load test must preserve asynchronous frame overlap and control callback ownership while verifying output and tracking in-flight resource generations.

This split is an engineering improvement, not an artificial abstraction requirement. It keeps the portable proof harness fast and usable in CI, makes the platform dependency explicit, and allows Metal failures to be diagnosed without forcing every C++23 chapter to compile Objective-C++.

#### Failure paths deserve tests, not reassuring comments

The portable tests refuse zero-length uploads, malformed alignment, capacity overflow, arithmetic overflow, stale frame generations, foreign frame owners and mutation of committed staging memory. They prove that a failed allocation leaves prior staged bytes intact. A known failure before commit returns a permit; an indeterminate outcome is quarantined instead of being mistaken for a command that never reached the device. Device loss prevents invented completion and keeps a pending frame occupied until a separate backend recovery protocol is established.

Readback failure paths are equally explicit. A capture with zero dimensions, insufficient row pitch or too few mapped bytes must not publish an image. In the portable model it leaves the frame in a submitted state until a valid terminal record is supplied; this reflects a strict **input-validation seam**, not the only possible production cleanup policy. A real callback that reaches terminal status but cannot allocate or compact a capture should still release its frame-capacity permit and emit an error record. Production code can separate terminal release from optional post-processing so an allocation failure never strands an in-flight slot. The current model intentionally keeps invalid synthetic completion inputs from being accepted as a terminal observation; an adapter must handle post-completion capture failure explicitly.

A failed command buffer may release the frame's permit after the native terminal error and any resource-reference requirements are satisfied, but it may not emit successful pixels. A duplicate completion event must not release the permit twice. An absent GPU timestamp must not be recorded as zero milliseconds. A stale frame ID must not write new scene or Gaussian data into a frame slot whose generation has changed. These are distinct conditions that deserve distinct tests.

Native API failures are not all the same either. No default Metal device on a headless environment means the native experiment is unavailable. A failure to compile our known MSL source on a machine that offers the requested Metal compiler is a real failed test and its NSError should be preserved. A shader computation that differs from the CPU oracle is an actual correctness failure even if the native command buffer reached COMPLETED. A device error after commit cannot be papered over by a fast host encoding timestamp.

#### The exact experiments that constitute Pass 15 evidence

**Portable C++23 tests** run the five scenario groups in the new `gpu_metal_frame_cpp23_lab` executable without Metal. They cover three-permit backpressure and out-of-order terminal events; checked frame-aligned upload arenas; errors returning capacity without false success; 256-byte row-pitch compaction of a synthetic 3×2 BGRA8 capture; invalid row and arithmetic refusal; indeterminate submission quarantine and device-loss refusal; finite positive GPU timestamp validation; generation wraparound retirement; and foreign-owner rejection. Their successful result would prove only that this **CPU reference model** behaved as designed under those deterministic inputs.

**Actual native Metal execution** is separately invoked with `--probe-native` on a Mac where CMake selected the Objective-C++ Metal adapter. It compiles original compute and render shaders, executes a 32-box AABB overlap mask and a 3×2 red offscreen render, copies the offscreen image into padded shared readback memory, waits for real completion and compares all GPU outputs against CPU oracles. Its status code means **success only after the real terminal status and all byte-level tests pass**. Code 77 means no usable native adapter/device; it is *not* a passing hardware test.

**Actual native timing** is separately invoked with `--benchmark-native`. It repeats the same correctness-gated compute+render+blit work with five warmups and thirty measured iterations, prints host median/p95 and prints GPU median/p95 only when every command supplies valid GPU timestamps. It does not claim to benchmark Maveb itself. A production performance report would need multiple independent runs, environmental metadata, profile captures, render resolution/scene configuration, device thermal state and comparison across alternative implementations.

Because this pass is committed through a connected source repository rather than executed on the user's macOS build machine, **none of those native runs are asserted to have happened** at authoring time. The new source and CMake target are deliverables; compilation, JUnit evidence, Metal device results and numerical timings are separate future observations. The user requested no intermediate commands or LaTeX regeneration, so both remain deferred.

#### Reconstruct the reasoning through five independent projects

Start with the **Kairo** Metal backend. Identify where a native command queue is created, which C++ module owns the Objective-C++ implementation, where a frame is encoded and committed, and which operation a capture waits for. Explain what code would have to change to capture asynchronously without losing the lifetime of its viewport texture, device, command buffer or destination memory. Distinguish a simple blocking capture's correctness from the renderer's throughput under frequent captures.

Next inspect **Maveb** FrameContext. Derive the cursor-alignment and capacity inequalities mathematically. Prove when resetting uploadCursor and readbackCursor is legal, identify why a command-buffer terminal handler and a frame-permit semaphore are linked, and enumerate every path on which a permit might be lost or returned twice. Explain why a command-buffer error is terminal for resource reuse but not a successful result. Describe how capture metadata must carry dimensions, row pitch, pixel format and color space.

Then apply the same host/GPU protocol to **Gaussian rendering**. Imagine a set of projected splats, tile bins, scan offsets, radix-sort indices and compositing outputs. Identify which buffers are written by one GPU kernel and consumed by another, what bounds must hold, and how a scene revision can retain its old GPU storage while a newer revision is being uploaded. A renderer that produces beautiful output but occasionally reuses a stale sort scratch buffer is not correct; a sorter whose output is fast but wrong under crowded tile overlap is not novel research.

For **collision broad-phase** work, reconstruct the tiny GPU AABB test. Explain the choice of strict half-open intersections and the \(i<j\) canonical mask, compute the expected result independently, and show how you would compare a future GPU append/prefix-scan pair list against the exact CPU oracle. Increase object count only after bounding memory and verifying every pair. Then evaluate sparse and dense regimes and material/geometry trajectories with performance measurements whose correctness gate remains closed on mismatch.

Finally take the experiment into **XR**. List at least four timestamps that a credible foveated rendering latency record should carry: gaze sample, pose/frame prediction, CPU submission, GPU completion and, if possible, display/compositor presentation. Explain why `GPUStartTime` cannot substitute for a gaze-to-display latency measurement, and why frame buffering can improve throughput while hurting responsiveness. Use independent image and motion-quality checks rather than optimizing a GPU duration that does not represent a user's visual experience.

The transferable insight is that **the renderer is a set of ownership and evidence contracts**. C++23 can make those contracts explicit; Objective-C++ and Metal can execute them on actual hardware; and carefully designed CPU oracles can tell us whether the computation deserves to be trusted. A real GPU run with a verified terminal status and correct pixels is more valuable than pages of API names, but it still does not certify a production engine without broader scenes, device coverage, performance data and resource-lifetime validation.

**Primary sources actually checked:** [Apple MTLCommandBuffer and terminal statuses](https://developer.apple.com/documentation/metal/mtlcommandbuffer), [Apple GPUStartTime](https://developer.apple.com/documentation/metal/mtlcommandbuffer/gpustarttime), [Apple GPUEndTime](https://developer.apple.com/documentation/metal/mtlcommandbuffer/gpuendtime), [Apple command-buffer completed handlers](https://developer.apple.com/documentation/metal/mtlcommandbuffer/addcompletedhandler(_:)), [Apple storage-mode guidance](https://developer.apple.com/documentation/metal/setting-resource-storage-modes), [Apple resource fundamentals](https://developer.apple.com/documentation/metal/resource-fundamentals), [Apple Metal Performance HUD metrics](https://developer.apple.com/documentation/xcode/understanding-metal-performance-hud-metrics), together with actual main-branch `Maveb/engine/metal/src/Renderer.cpp`, `FrameContext.cpp`, `FrameContext.hpp`, `KairoRenderer/MetalRenderer.cppm` and `detail/MetalBackend.mm` inspected this pass. Exact Git blob revisions belong in the multi-project evidence ledger.

**Continue the original Chapter 15.** The ensuing extensive storage modes, tile memory, drawable, frame pacing, heap and synchronization discussion remains intact. The next chapter, “Profile GPUs and Respect Apple Ownership Boundaries,” can take these real-oracle experiments into Maveb's GPU capture and Kairo's ObjC++ ownership boundary with targeted validation and profiler methodology, rather than declaring the whole book complete from one passing GPU micro-experiment.
