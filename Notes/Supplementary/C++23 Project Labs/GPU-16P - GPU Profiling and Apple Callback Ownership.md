### GPU-16P — A fast GPU command is not a trustworthy performance result

At this point in the course we know that a renderer needs far more than an API handle. We have built controlled binary/resource ownership, a CPU execution model, explicit command lifecycle contracts, a device capability selector, an optional real Vulkan submission/readback probe and an optional Metal shader/graphics execution probe. Those chapters deliberately separated **the ability to call an API** from **having proved an algorithm correct on actual hardware**. There is one more separation that engineers and research groups frequently get wrong: **an observed GPU duration is not automatically a valid performance comparison**, and a callback that reports terminal GPU work is not automatically safe to deliver to an Objective-C++ renderer, a SwiftUI view or a C++ resource owner.

This lesson makes those two obligations concrete through the real **Maveb Metal renderer**, **Kairo's native Objective-C++ Metal backend**, the **Kyoto collision-pair correctness oracle**, and the **XR gaze-to-display measurement problem**. The NES emulator is not the primary teaching project here; a simulator is useful when you need to reason about CPU cycles and owned bytes, but it is not a substitute for native Apple GPU traces, asynchronously captured frame state, or gaze-coupled frame latency.

The original Chapter 16, **Profile GPUs and Respect Apple Ownership Boundaries**, already contains approximately **2,467 lines / 90,695 characters** with substantial treatment of GPU counters, Metal execution stages, resource aliasing, Objective-C++/SwiftUI boundaries, render-graph lifetime analysis and additional engine interoperability and systems topics. We preserve all of it. Our new material is a connected, executable **application-and-evidence bridge** staged *immediately before the original* `### Distinguish host allocation callbacks from device-memory allocation` heading. That location keeps the already developed Metal/Apple ownership narrative intact, then moves into a real callback/profiling protocol before the chapter transitions to host allocator callbacks and general language/toolchain boundaries.

This pass adds a portable C++23 profile/ownership library `gpu_course/gpu_profile_ownership.hpp`, a host-multithreaded adversarial test executable `gpu_profile_ownership_cpp23_lab.cpp`, an optional real Objective-C++/Metal callback and compute probe `gpu_course/native_metal_callback_probe.mm` and a platform-unavailable fallback `gpu_course/native_metal_callback_unavailable.cpp`. The regular CTest only runs portable host tests and labels its GPU-looking observations **synthetic**. An actual macOS Metal run must be explicitly requested and observed before the project can claim GPU execution, resource ownership under Metal callbacks or a real measured duration.

#### Read the real renderer before redesigning its callback

The inspected `Maveb/engine/metal/src/Renderer.cpp` creates a real Metal command queue and a dispatch semaphore with capacity three. The render method obtains its current drawable and frame render-pass descriptor, waits on a permit, chooses a `FrameContext` by frame number, calls `frame.beginFrame()`, records multiple kinds of work and finally calls `commandBuffer->commit()`. It also records a frame capture when requested: a shared CPU-readable buffer receives a blit from the drawable with 256-byte-padded rows; an asynchronous completed handler compacts that buffer into a stored `FrameCapture`.

A separate Maveb completion handler computes `completed->GPUEndTime() - completed->GPUStartTime()`, publishes a last-GPU-milliseconds field, increments a completed-frame counter and signals the frame semaphore. The renderer destructor waits for three permits before releasing its owned semaphore. Those details matter because they tell us why `frame.beginFrame()` may reuse its 8 MiB shared upload arena, 256 KiB readback arena and private transient heap: the *permit protocol* is meant to ensure previous GPU use of that slot has reached a terminal condition. The arithmetic `frameNumber % 3` does not by itself establish that condition.

The inspected `KairoRenderer/detail/MetalBackend.mm` takes a different approach. Its C++ facade `MetalRenderer.cppm` exposes renderer-neutral scene, picking and capture operations. The Objective-C++ detail backend creates the actual Metal device, command queue and layer, compiles its shader functions and retains `LastCommand`. Before destroying meshes or textures, or reading viewport pixels for capture or picking, the backend calls `WaitForLastCommand()`. It waits until the last native command finishes, then checks `MTLCommandBufferStatusError` before allowing the read. This deliberately favors a simpler synchronous lifetime proof at the cost of a possible CPU stall. Maveb favors throughput with three in-flight frames and completion handlers, at the cost of more demanding callback ownership and state-transfer rules.

Neither implementation becomes “better” just because its GPU duration looks smaller. Kairo may block its CPU capture path while Maveb defers and overlaps the readback; the two architectures would need the same scene, GPU, shader source, capture frequency, graphics fidelity and complete latency definition for a fair comparison. A render graph or UI editor that captures only occasionally might prefer a synchronous path for simplicity; a reconstruction viewer displaying hundreds of thousands of splats at interactive frame rates might require an asynchronous capture path. A fair test must measure both the actual work and the time scope that matters to the user.

The exact Git blob revisions for Maveb and Kairo read in this pass are recorded in the seven-book project's source ledger. We are **not modifying those external repositories** as part of this curriculum pass. In particular, `Maveb::Renderer` is not claimed to have had its callbacks fixed and `Kairo::MetalBackend` is not claimed to have received new profiler instrumentation.

#### Separate four definitions of “the frame took 4 milliseconds”

Suppose a UI overlays the text “GPU 4 ms” after a Metal command buffer finishes. What exactly does that figure represent? Metal's `GPUStartTime` and `GPUEndTime` are host-time seconds associated with the execution of a command buffer. Their difference can provide a useful GPU command interval **after completion** when the device supplies valid values. It is not the same as the CPU time spent encoding and committing that buffer; it does not include arbitrary work before that command buffer, and it is not necessarily the time at which a CAMetalDrawable finally appears on the user's display.

A rigorous trace should distinguish at least these events:

```text
input/camera/gaze sample received
        ↓
simulation/scene update begins
        ↓
CPU starts encoding GPU commands
        ↓
CPU commits command buffer to native queue
        ↓
GPU actually starts command buffer
        ↓
GPU completes command buffer (COMPLETED or ERROR)
        ↓
output is validated / readback becomes legal
        ↓
present request and display/compositor completion
```

The intervals answer different questions. CPU encode-to-commit cost is relevant to engine submission overhead and main-thread responsiveness. GPU command duration is relevant to graphics/compute throughput for that work. The time until a capture can be safely delivered depends on the command's completion, CPU post-processing and possible callback scheduling. In XR, gaze-to-display and pose-to-photon delay include sampling, prediction, rendering, compositor and display scheduling; neither is determined solely by a GPU command-buffer interval.

A useful symbolic formulation is

\[
T_{\mathrm{CPU\,encode}}=t_{\mathrm{commit}}-t_{\mathrm{encode\,begin}},
\qquad
T_{\mathrm{GPU\,cmd}}=t_{\mathrm{GPU\,end}}-t_{\mathrm{GPU\,begin}},
\]

and, when genuine compositor/display markers exist,

\[
T_{\mathrm{gaze\to display}} =
t_{\mathrm{display}}-t_{\mathrm{gaze\,sample}}.
\]

These are **distinct measurements**. They can overlap across frames, so summing or subtracting them without aligning clock domains and frame identities can be misleading. A GPU-completed timestamp from frame \(k\) must not be joined with a gaze sample from frame \(k+1\) or a drawable presentation from an earlier generation.

This is also why profiling requires stable identifiers. Record at least a frame/command identity, source and shader revision, scene/camera or workload revision, device and OS/SDK identity, render resolution, feature configuration, correctness result and timing units. A floating-point number in a text log with no such metadata may be a clue, but it is not yet a reproducible experiment.

#### Terminal completion is not the same as successful output

Apple's `MTLCommandBuffer` exposes distinct terminal states, notably `MTLCommandBufferStatusCompleted` and `MTLCommandBufferStatusError`. A completion handler is invoked when the command buffer reaches a terminal state; the handler must check which state occurred. A device command can finish unsuccessfully and still need its native resources, CPU frame permit and asynchronous cleanup released appropriately. No renderer should convert that terminal error into “frame successfully rendered” merely to satisfy the permit accounting.

Even `Completed` is not a proof that a shader computed the right values or that an image has the desired visual appearance. A compute shader may have indexed the wrong buffer, used a stale Gaussian covariance or emitted too many collision candidates while the command still finishes. The correct evidence gate therefore requires a terminal success **and** an output oracle appropriate to the workload. Our earlier real Metal lab uses a CPU AABB-overlap reference for 32 objects and checks every GPU-produced pair-mask entry; its render path checks every BGRA pixel of a 3×2 offscreen image after a blit and terminal completion. Those tiny experiments establish specific execution and data-transformation properties if run natively. They do **not** establish whole Maveb correctness or Kyoto's benchmark leadership.

The new portable completion event carries two separate axes: `terminal` (completed or failed) and `correctness_checked/correctness_pass`. A completed event whose pair or pixel output failed a check is still a terminal event, so the frame permit can be released; but the benchmark sample must be excluded or, for a strict correctness-gated run, the entire performance comparison must be considered failed. A terminal error carries no successful GPU duration or successful image. A missing correctness check is not silently considered a pass.

This distinction resolves a subtle profiler temptation. A GPU kernel can become “faster” if it silently skips a workload or drops collision pairs. A Maveb splat pass can become faster if it renders fewer visible splats, ignores the correct compositing order or fails to include expensive occlusion cases. A Kyoto broad-phase implementation can become faster if it misses candidate pairs. Without a correctness gate and a workload identity, timing such failures would reward the wrong algorithm.

#### GPU timestamps must be treated as optional evidence

Metal's `GPUStartTime` and `GPUEndTime` may initially be zero and should be read only after the command has completed or within a completion handler. A value of zero is *not* evidence of a command that took zero time. A finite positive end value greater than a finite positive start value is the basic validity condition for a usable interval. A device may not expose the desired timestamp information on every target or execution path.

Our `gpu_interval_ms` helper therefore returns `std::optional<double>` rather than a bare double. It rejects missing values, nonfinite inputs, zero starts, reversed/equal intervals and failed terminal status. This is more informative than storing a default 0.0 in a telemetry field, because missing observation and measured zero would otherwise look identical. The helper also checks the computed millisecond conversion for overflow or nonfinite results.

A debug overlay should display **“GPU interval unavailable”** rather than “0.000 ms” whenever the source could not supply valid timing. It should not infer a GPU timer from `std::chrono::steady_clock` around `commit()` or around `waitUntilCompleted()`. The former measures host enqueue/encoding work; the latter includes waiting, scheduling and perhaps work already in flight before the wait. It can be a useful end-to-end observation if clearly labeled, but it is not the device execution interval.

In native Pass 16, the optional Metal probe executes a compute command and reads timestamps after a real completed callback. It prints the measured interval only if valid and labels it **one diagnostic sample, not a benchmark**. Its default CTest path never runs the native Metal code. No invented GPU result is presented simply because a timestamp getter exists in a header.

#### A supported counter set is not a supported counter value

Hardware performance counters offer insight unavailable from command duration alone. Depending on GPU and API availability, a profile might expose busy periods, occupancy, memory bandwidth or transfers, shader activity, tile memory behavior, cache effects and related measures. But those metrics are **not guaranteed to exist under identical names or units across Apple GPU families and SDK versions**.

Metal exposes counter sets and counter sample buffers. A tool must first query which counter sets a specific device exposes and which counters are supported within those sets, then create compatible sample buffers, encode the appropriate samples and resolve their contents under the rules supported by that device. A counter's presence in documentation or in a GUI mockup is not evidence that the current device returned it. A profile should include the exact counter identifier, unit, device and sampling method when it has a numeric value.

Our `CounterValue` model has three independent fields of meaning: `identifier/unit`, `supported` and `measured`. An absent or unsupported counter has no numeric value. A numeric result marked unsupported is rejected by the evidence gate, as is a nonfinite counter value or a counter with no named unit. We never replace missing tile-memory traffic with zero transactions, missing occupancy with “100%” or an unavailable cache metric with a guessed GPU family default.

For example, a reconstruction paper comparing Gaussian splatting passes might ask whether a new tiled sorting method reduced memory traffic. The performance-causality claim requires device-specific supported counters measuring that traffic under the same workload and tooling. A command-buffer duration falling by 10% alone does not identify which memory stage changed. That duration can be meaningful performance evidence, but it does not explain the mechanism. A capture of scheduling, shader resources and actual counters can make the explanation more credible when controlled.

A GPU counter trace still has sampling limitations. Sampling can perturb the workload, some counters describe a stage or pass rather than a specific shader invocation, some are estimates or aggregated over concurrent work, and certain counters are absent or incompatible with the chosen encoder. The result must be interpreted in the scope of the instrument. The [Apple counter-support guidance](https://developer.apple.com/documentation/metal/confirming-which-counters-and-counter-sets-a-gpu-supports) and [counter sample buffer guide](https://developer.apple.com/documentation/metal/gpu-counters-and-counter-sample-buffers) are mandatory references at the actual device-profiling checkpoint, not permission to invent values.

#### Metal frame capture is a controlled experiment, not a screenshot

A pixel screenshot tells us how an image appeared under a particular color pipeline; it cannot explain which command encoder owned an attachment, where a resource was allocated or whether a shader spent its time on texture fetches versus arithmetic. A **Metal frame capture** provides a trace of encoded commands and resource state that Xcode's Metal debugger can inspect. `MTLCaptureManager` can also programmatically define capture scopes and start/stop capture for a supported device/command queue, subject to capture-tool configuration and platform availability.

Capture traces are especially valuable in Maveb because one frame has many dependent stages: shadow maps, scene geometry, Gaussian projection/bins, scan and sort, compositing, temporal accumulation, bloom and final presentation. Debugging only the final image can obscure the exact pass where data became stale or incorrect. A capture can inspect relevant attachments and commands when the platform supports the requested diagnostics. It also often changes timing; an instrumented trace should not be combined uncritically with an uninstrumented steady-state benchmark.

A well-formed capture protocol records the engine commit, shader library digest, chosen scene revision, camera path, render resolution, enabled passes, GPU name, OS/SDK and capture-tool version. It defines a short reproducible range of frames. It captures the target frame only after scene warmup and resource setup if the goal is steady-state GPU cost, or explicitly includes first-use compilation/allocation if the goal is startup latency. The resulting trace may contain proprietary shader/source assets and scene data, so it must be shared and retained under the appropriate project controls.

For Kairo's more synchronous backend, a capture can focus on the scene encoder, viewport render target, pick pass, offscreen readback and presentation pass. For Maveb's multi-pass frame, the experiment must include enough trace context to identify a logical frame across asynchronous command submission and callbacks. For collision-pair GPU compute, the trace should show source AABB buffers, output capacity, count/scan passes and the final pair list; an independent CPU exact-pair oracle still decides whether the outputs are correct.

The native GPU-16P executable **does not itself start a full Xcode frame capture or query every hardware counter**. Its job is to establish a small real GPU completion, callback and result witness under correct ownership, and to reject fabricated evidence. An actual Xcode/MTLCaptureManager trace is a separately observable artifact that must be produced on the user's Mac. The lesson supplies the experiment contract now; it does not pretend such a trace file was generated by the connected GitHub source edit.

#### The Objective-C++ callback problem: who outlives whom?

Look at a Metal completed-handler lambda or Objective-C block. The native API can schedule its invocation after the CPU function that registered it has returned. That means every captured object must either **outlive the callback** or be accessed through a checked non-owning handle whose owner may have disappeared. The existence of ARC or `shared_ptr` somewhere in the code does not automatically make a raw C++ `this` pointer valid after the renderer has been destroyed.

Maveb currently registers completed handlers that capture `this`. Its destructor waits for its three frame permits, providing a meaningful form of lifetime coordination. The relevant review question is not “all lambdas capturing this are unsafe”; it is whether **every possible callback** is accounted for by that permit-drain protocol, whether errors can leak/duplicate permits, whether a capture callback can run after its parent has freed state, and whether frame contexts can reset only after all uses finish. We should not assert a production use-after-free without a reachable scenario and real evidence.

Kairo's Objective-C++ backend instead records and retains its last command and waits for it before certain resource destruction or capture operations. That gives a different lifetime guarantee but can stall CPU callers. The shared lesson is to separate the C++ wrapper's lifetime from any native completion that may still be pending. For an asynchronous backend, moving a completion receipt into a ref-counted state object can decouple resource cleanup from the UI renderer object and make shutdown behavior inspectable.

The new `CompletionMailbox` is precisely such a **host-side state object**. It has an internal `std::shared_ptr<State>` with mutex, slot generations and owned completion events. A callback captures only that state and the intended `FrameId`. It does **not** capture a raw `Renderer*` or a SwiftUI view. The renderer wrapper may be destroyed before a callback arrives; the callback can still safely update its internal state because it retains that state, while an unrelated weak UI owner can expire without being dereferenced.

This does not mean a global shared_ptr should keep every renderer or texture alive forever. A **reference cycle** would prevent shutdown; an unbounded queue could leak thousands of event records. Production design needs bounded capacity, explicit event draining and a shutdown protocol that covers possible callbacks. A shared-state mailbox is appropriate for short-lived completion receipts, but the UI should observe them through a proper main-actor or dispatch boundary. It must not mutate SwiftUI state directly from an arbitrary Metal driver callback thread.

#### A frame identity must survive slot reuse without aliasing an old callback

A resource or frame slot can be recycled. If a late callback merely says “frame slot 0 completed,” it might free storage that now belongs to a *different* frame. To avoid this **ABA-style identity** problem, every frame ticket carries owner ID, slot index and a monotonically increasing generation. The callback validates all three under the same mutex it uses to release a permit.

Let \(S_{k}\) be a slot on generation \(k\). An accepted callback may transform it into a reusable slot with a new generation \(k+1\), but an old callback retaining \((owner,slot,k)\) must not mutate \(S_{k+1}\). If the generation could wrap to an earlier value, that protection would eventually fail. The model therefore exhausts a slot rather than wrapping `uint64_t` generation.

This is another place where a single shared mutex simplifies correctness. The callback's critical transition performs: **check identity → verify submitted state → create an owned event → advance generation/release permit → notify waiters**. There is no window where a second callback can observe the old generation as still in flight after the first callback has released the permit. In a lock-free implementation the developer would need to prove equivalent atomic publication and memory ordering; a casual relaxed-atomic increment would not be enough.

The model's callbacks are actual C++ callable objects invoked by **real host threads** in the portable test, so it can expose data races and duplicate-event bugs under sanitizers when a runner executes it. The commands and GPU timestamps used in those tests remain **synthetic fixtures**. There is no claim that three host threads mean three GPU queues or that arbitrary callback order reflects a hardware GPU schedule. The host concurrency test is about memory safety and correct bookkeeping under overlapping completions.

The callback model distinguishes recording, submitted and quarantined states. A definite failure before submission can cancel a recording slot and restore its capacity. After possible submission, however, the code cannot safely act as though the GPU never saw the buffers. An uncertain outcome retains the slot until acceptance is confirmed or a stronger native recovery protocol proves it safe to retire. These are the same ownership rules developed in the Vulkan command journal, now tested through actual C++23 concurrent callbacks and native Metal status observations.

#### The optional native Metal callback witness

The `gpu_course/native_metal_callback_probe.mm` file is deliberately small. When CMake finds an Apple Objective-C++ compiler and the Metal/Foundation frameworks, it can compile an ARC translation unit that creates a real `MTLDevice` and `MTLCommandQueue`. It compiles a simple original Metal Shading Language kernel that writes 64 deterministic 32-bit words into a shared Metal buffer and dispatches that compute work through a real command buffer.

Before submission, the test creates a one-frame `CompletionMailbox`, reserves its frame ticket and prepares a callback that retains **only** the mailbox's internal shared state. It also creates a UI-like owner observed through `std::weak_ptr`. It registers an actual Objective-C `addCompletedHandler:` block that supplies Metal's real terminal status and GPU timestamps to the callback state. **Before** committing the GPU command, the test destroys the original mailbox wrapper and the UI owner. The callback must therefore be able to complete without accessing or resurrecting the former UI object.

After a real `waitUntilCompleted` and receipt delivery, the test checks that the Metal command ended in `MTLCommandBufferStatusCompleted`, that the callback updated the internally retained state exactly once, and that the weak UI observer has expired. It then checks every one of the 64 GPU-written words against a separately recomputed CPU formula. A callback merely reporting terminal completion does **not** mark the output correct; the CPU check happens after the GPU completion observation. These two facts are reported separately.

If no native Metal device is available or the necessary Apple toolchain was not configured, the explicit native route reports **UNAVAILABLE / exit 77**, not a fake green GPU test. A failure in MSL compilation, command creation, terminal status, callback delivery or the CPU/GPU output comparison produces a failure. The default CTest does not invoke this opt-in native mode, so it remains portable across Linux/macOS and cannot be counted as physical GPU evidence.

This adapter uses **actual Objective-C blocks**, not C++ lambdas passed where the Metal API expects a block, and it compiles in Objective-C++23 mode because it imports our C++23 `std::expected`-based mailbox. ARC is enabled only for this native Apple compilation unit. These are nontrivial language boundary rules: a C++ codebase that compiles its portable tests with GCC does not thereby prove its Objective-C++ code will compile on AppleClang, and the existence of Foundation headers does not mean Metal execution succeeded.

For a production renderer, there is a further design issue that our simple callback witness does not pretend to solve: how to report an error if allocating an owned completion event itself fails inside a native callback. The sample catches C++ exceptions before they cross the Objective-C block boundary, marks the receipt failed, and leaves any unprocessed state visible to its owner. A robust engine would use a preallocated bounded completion queue or guaranteed reserved error path, then handle shutdown/device loss without leaking a frame permit or falsely declaring a successful image.

#### A performance report needs an identity and a correctness gate

Two timing samples should be compared only when they mean the same thing. In graphics, “same device” is not sufficient: shader variant, graphics pipeline, scene, frame resolution, camera, compiler options, resource residency, capture tooling and counter support may differ. A benchmark with half its samples using one Gaussian shader revision and half another is not one homogeneous distribution.

The new `Configuration` carries project, device, OS/SDK, source commit, shader digest, workload digest, capture tool, dimensions and validation status. A `FrameSample` carries that configuration along with terminal status, origin, correctness check/result, CPU/GPU timings and explicitly labeled counters. The portable `summarize_native_verified` function is deliberately conservative: it refuses a missing environment identity, mixed configurations, synthetic observations, failed pixel/pair checks, invalid CPU or GPU timing, unsupported numeric counters, and insufficient sample counts. It reports median and nearest-rank 95th percentile only after those checks pass.

Mathematically, the summary is a function defined over a set of validated samples \(X=\{x_1,\dots,x_n\}\) satisfying

\[
\forall i:\quad
\operatorname{sameConfig}(x_i)
\land\operatorname{native}(x_i)
\land\operatorname{terminalSuccess}(x_i)
\land\operatorname{oraclePass}(x_i)
\land\operatorname{validTimes}(x_i).
\]

If any required property fails, the correct result is an error or a clearly scoped diagnostic subset—not a fabricated headline that quietly removes failing cases. Performance comparisons can use additional statistical methods and independent runs, but a data-validation gate is the first requirement.

The gate is not a cryptographic proof that a caller supplied actual hardware results. A synthetic test can assign a field named `native_metal_observation` to exercise the acceptance logic; that does **not** make the fixture a physical GPU sample. The native-adapter evidence must originate from an executed, source-pinned binary and contain the device/tool observations. The ordinary CTest explicitly labels its completion origins `synthetic_fixture` and tests that they cannot produce a native summary without altering that status for an isolated **negative/control fixture**.

When actual benchmark records become available, a richer report should distinguish compiler/commit versions, warmup count, number of independent trials, within-run distribution, run-to-run variation, calibration/counter overhead, device power and thermal state, scene fidelity and confidence estimates. A nearest-rank \(p95\) from one small run is descriptive; it is not proof of stable generalization. The previous GPU-15M opt-in native benchmark uses five warmups and thirty correctness-gated samples, but no such samples have been observed in this authoring pass.

#### Counterfactuals make GPU bottleneck conclusions stronger

Suppose Maveb's Gaussian pass takes 5 ms in one profile. That duration alone does not identify the bottleneck. To decide whether the cost comes from projected splat count, tile overlap, sort offsets, blending, shader occupancy or memory traffic, change exactly one workload dimension and record the corresponding counters and correctness outcomes.

A useful experiment might hold camera, resolution and scene revision fixed, then compare the original Gaussian bin/sort path to an alternative scan/sort implementation under the same tested input set. Validate the resulting visible splat order and final image numerically, not just the number of generated pairs. Record CPU encode-to-commit time, GPU command interval, the supported counter values most directly associated with the hypothesis and the cost of any extra buffers. A proposed “optimization” that moves work from one pass to another but increases total frame time is not successful.

For Kyoto broad phase, use matched trajectories and exact-pair oracles across SAP, DBVT, Guard and the learned controller. Report the distribution of overlap density and material regimes, not just a geomean of times stripped of work-count information. A GPU broad-phase kernel that drops a small percentage of candidates is incorrect even if a visually plausible simulation happens to render most frames. Pair correctness, solver stability, penetration/folding visual metrics and throughput are related but independent evidence streams.

For XR, isolate whether a reduction in GPU shading work improves **the perceptual deadline that matters**. A foveation change could save GPU time but use a gaze sample that is too old, shift detail away from the user's fixation or create temporal artifacts. The experiment should log gaze samples, predicted pose, CPU submit, native GPU completion and any available compositor/display timestamps, then pair objective image-quality metrics with motion/latency evidence. Hardware counter data is helpful only when its sampling scope can be matched to those frame identities.

These are real reasons why the curriculum should use *several persistent projects* rather than force all graphics and scientific problems into one emulator demonstration. Each project reveals a different constraint that a sound C++23/graphics engineering abstraction must survive.

#### C++23 design exercise: a callback that never touches the renderer

A safe host callback skeleton has a small shape:

```cpp
// A native adapter may capture this by value into a completion block.
struct CompletionCallback {
    std::shared_ptr<SharedCompletionState> state;
    FrameId expected;

    void operator()(NativeCompletion observation) const noexcept {
        // Never access Renderer*; shared state owns the record independently.
        // On a real code path, use a reserved/nonthrowing publication route.
        // Validate expected owner, slot, generation and submitted phase.
        // Deliver terminal/error data, then release exactly one permit.
        // Forward UI updates to the main actor/queue outside any state lock.
    }
};

// A SwiftUI view/controller is an OBSERVER, not the owner of a GPU callback.
// The observer may disappear while native work continues.
std::weak_ptr<UiOwner> observer = uiOwner;
auto completion = queue.make_callback(frameTicket);
uiOwner.reset();  // callback remains valid; UI object may be destroyed.
```

The new compiled mailbox implements a real version of this concept over concrete `std::shared_ptr<State>`, `std::mutex` and `std::expected` types. A terminal callback checks its frame ID, emits a value-owned event, increments the generation and returns the frame permit inside one protected state transition. It notifies waiters only after unlocking. A second callback for the same generation is rejected. The portable tests use three actual `std::jthread` host threads delivering different terminal statuses concurrently. They also destroy a renderer-like owner before delivering a pending completion and prove a weak UI observer has expired.

One extra caution is important for the user as an engine author: **capturing a shared_ptr is not equivalent to making all code thread safe**. The state it points to must synchronize access to mutable fields. Moving all state into a shared_ptr without a mutex can still cause data races. Conversely, a mutex held while invoking UI callbacks can cause deadlocks or re-entrancy hazards. The correct pattern is to create and store an owned event while the lock is held, release the lock, and then transfer the event through the supported UI thread/actor boundary.

Another caution: an Objective-C `addCompletedHandler:` callback can arrive after the method registering it has returned. A local autorelease pool may have drained, but a retained Objective-C Metal resource can still be alive through the API's ownership protocol. A raw C++ pointer to a stack local from the registering function will not be kept alive by ARC. Explicitly capture a correct owning object, or convert the value into an owned completion receipt that survives independently of temporary stack memory.

#### Shutdown and device loss are first-class runtime states

A renderer can shut down while GPU work remains in flight. A naive destructor may free the frame arenas and event state while the device still has commands referencing them. The opposite naive approach retains the entire renderer forever because each callback holds a strong reference to the renderer and the renderer holds a list of callbacks. We need a termination protocol that is both memory-safe and eventually releases resources.

Maveb's destructor drains its frame permits, which is a meaningful source-level approach to ensuring in-flight callbacks have returned capacity before destructor release. But robust validation must check *all* paths: commit failure, command-buffer error, unavailable drawable, capture allocation failure, callback exception, device loss, and any case where an early return occurs after acquiring a permit. For a hard-to-reproduce hang, instrument each transition with frame ID and permit count, then assert that the sum of free slots and pending/possibly accepted slots remains constant. Never use a terminal **error** as proof of correct image output simply to keep the destructor from blocking.

Kairo's synchronous `WaitForLastCommand` can simplify destructor safety but must be analyzed for queues whose completion status is error or whose device is lost. Waiting forever on an event that can no longer complete is not a viable recovery strategy. A production backend may need to stop submitting, mark pending work failed, detach the UI, wait for or forcibly invalidate native objects through the platform's specified device-loss protocol, and rebuild a fresh backend generation.

Our portable mailbox stops short of promising full native device-loss recovery. It makes accepted and quarantined work distinct and refuses to turn an uncertain submission into “never submitted” simply because the application wants to close. The native Metal adapter tests one bounded, successful completion path under a renderer-owner teardown. The remaining failure paths belong to future device-specific integration, where actual errors and validation logs are observable.

#### Exact tests and what they can honestly prove

The Pass 16 portable executable has several independent test groups. Three real **host threads** deliver terminal events for Maveb, Kairo and Kyoto-tagged frames with different status and correctness outcomes; a fourth frame is refused until capacity returns. A duplicate callback is rejected through generation checks. The source checks that a failed command cannot publish successful GPU timing, and that a completed-but-wrong collision result cannot become a passed image or benchmark sample.

Another test destroys the renderer-like mailbox wrapper and UI owner before invoking a callback retaining only internal state; it checks that a stale second callback is refused and that a foreign owner cannot manipulate another scheduler. A third exercises indeterminate submission quarantine and definite cancellation rules. A fourth feeds a synthetic profiling series through the provenance gate: mixed shaders, missing GPU identity, wrong pixels, unsupported counter values, invalid/zero timestamps, synthetic origin and too few samples all cause distinct rejection. A final group exercises zero, missing, reversed, infinite and nonfinite GPU timestamp inputs.

These tests are **authored**, not yet certified as compiled or passed across GCC/Clang/AppleClang. They use actual C++23 host synchronization when run, but no Metal device or hardware counters. Their role is to make native callback policy and evidence interpretation inspectable under deterministic failure cases and sanitizers.

The optional Objective-C++23 adapter, selected only on compatible Apple toolchains, can execute a real Metal compute kernel and a completion handler that outlives the original renderer/UI wrapper. It tests the real terminal status, real callback delivery, expired weak observer and all 64 GPU-produced words against CPU arithmetic. It reads available GPU start/end times after completion; if invalid it prints **UNAVAILABLE**, not 0 ms. It does not query a hardware counter set, start an Xcode trace or generate a GPU report. Those are separately designed experiments, not phantom artifacts.

The source-pinned course evidence manifest must enumerate all sixteen cumulative CTest targets and all added files. The strict insertion tool must stage this lesson without changing any of the original continuous manuscript's bytes. The final canonical Git blob should match `main` until the user authorizes the consolidated addition/application checkpoint.

#### A profiler and ownership experiment for each project

For **Maveb**, choose a fixed Gaussian scene and camera sequence; use a repeatable frame capture for projection, tile binning, sorting, compositing, TAA and presentation; distinguish command GPU time from stage counters and from capture overhead. Derive every resource's last use, including frame-local normal/depth attachments and persistent history images. Check that command-completion callbacks and frame-arena reuse obey their lifetime contracts under both success and error paths. Do not claim visual accuracy solely because a frame is fast.

For **KairoRenderer**, first establish a reliable baseline where the ObjC++ detail backend commits and waits for a native command before pick/readback. Then design a deferred capture path with one stable owner state and a weak UI observer, and prove its lifetime through window closure, resize, malformed readback, GPU error and immediate application shutdown. Compare capture stall and frame cadence on the same scene, keeping the result independent of a renderer mode that changes fidelity.

For **Kyoto/KairoPhysicsEngine**, replace the toy output after its independent CPU oracle is proven with actual streamed broad-phase pair sets. Compare exact pairs and observed allocation bounds across dynamic trajectories; use the same per-trial source/seed/material/size metadata to interpret performance. If running on GPU, inspect supported counters for memory traffic, compaction and kernel execution, and never treat counter names as portable across unsupported devices.

For **XR and gaze-dependent rendering**, define frame identity across gaze input, predicted pose, GPU submission and eventual compositor presentation. Use a latency timeline rather than only a GPU command duration. Check image quality and temporal stability, including gaze shifts, dropped frames and resource reuse. Do not claim the native callback/compute microtest validated ILLIXR, an XR headset or motion-to-photon latency.

Finally, for **cross-language engine architecture**, map which operations remain C++23, which are Objective-C++ detail implementations, which run on the main SwiftUI actor and which callbacks arrive from native worker queues. Define ownership, borrowing, error and cancellation at each crossing. A generated binding cannot remove these responsibilities merely by turning a pointer into a different language's method signature.

**Pass 16 acceptance boundary:** A meaningful additive chapter, a portable C++23 concurrent callback/evidence model with adversarial tests, an explicit optional real Metal callback/kernel witness, and source-preserving cumulative test wiring. **Not** an observed Metal profiling run, not an Xcode GPU trace, not measured hardware counters, not a patched production renderer, and not a completed seven-book quality audit.

**Primary sources inspected:** `Maveb/engine/metal/src/Renderer.cpp`, `FrameContext.cpp` and `FrameContext.hpp`; `KairoRenderer/MetalRenderer.cppm` and `detail/MetalBackend.mm`. Primary API references: [MTLCommandBuffer status](https://developer.apple.com/documentation/metal/mtlcommandbuffer/status), [Metal completed handlers](https://developer.apple.com/documentation/metal/mtlcommandbuffer/addcompletedhandler(_:)), [GPU start time](https://developer.apple.com/documentation/metal/mtlcommandbuffer/gpustarttime), [GPU end time](https://developer.apple.com/documentation/metal/mtlcommandbuffer/gpuendtime), [supported GPU counters](https://developer.apple.com/documentation/metal/confirming-which-counters-and-counter-sets-a-gpu-supports), [Metal counter sample buffers](https://developer.apple.com/documentation/metal/gpu-counters-and-counter-sample-buffers), [programmatic Metal capture](https://developer.apple.com/documentation/xcode/capturing-a-metal-workload-programmatically) and [MTLCaptureManager](https://developer.apple.com/documentation/metal/mtlcapturemanager).

**Continue the original Chapter 16.** The next original heading concerns Vulkan host allocation callbacks and device-memory allocation. Our profiler's measurement-source distinctions now provide a natural bridge: a counter for driver-side host allocation requests is not a count of VkDeviceMemory or Metal heaps. The remaining original material—cross-language generated bindings, process-wide state, C++ machine state, network/file trust and the full systems continuation—remains unchanged and available for its proper narrative position.
