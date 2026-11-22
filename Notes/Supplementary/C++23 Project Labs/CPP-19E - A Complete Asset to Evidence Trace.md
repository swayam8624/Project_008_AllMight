### CPP-19E — A complete asset-to-evidence trace, from bytes to safe retirement

The preceding chapters have taught several individually defensible ideas. A file reader can own its bytes, a decoder can refuse malformed data without leaking a half-constructed value, a generational registry can prevent stale logical handles from identifying a new asset, and a dependency planner can derive ordering from reads and writes. Yet the central engineering question is harder: **does the complete path preserve all of those guarantees when the steps are connected?** A passing parser does not prove that the right parser was called. An acyclic task graph does not prove that a task saw initialized memory. A valid output image does not prove that it was produced by the scene revision currently open in the editor. This lesson turns those separations into one executable, inspectable CPU acceptance trace.

The physical inputs are two *real binary files* created in a disposable test directory: a tiny TXR4 RGBA8 texture and a tiny MSH1 quantized triangle. These formats belong to the existing Book 1 Pass 05 source; they are deliberately not described as PNG, KTX, OBJ or glTF. The file layer is the actual C++23 Pass 18 `std::filesystem` / `std::ifstream` / `std::jthread` pipeline. Once the input has become an owned byte vector, the Pass 05 codec decodes it, the Pass 06 registry assigns typed owner-aware handles, the Pass 08 dependency planner orders the work, and the Pass 09 memory oracle refuses an illegal publication or early retirement. The result is a deterministic textual receipt. Every module in this route is an existing implementation from earlier passes; the new file is `tools/labs/book01_end_to_end_cpp23_lab.cpp`, not a parallel imaginary asset library.

#### Start with the observable result, then work backward

Consider an artist who changes a texture while a geometry analysis is running. An editor needs to answer four questions before it displays the result: which exact bytes were used; whether they decoded into valid, owned domain objects; which version of the scene those objects belong to; and whether the jobs finished before any source or intermediate resource was retired. A research reproduction asks the same questions about the source geometry, input configuration, numerical transform and saved measurement, although its correctness oracle is different. The shared foundation is **provenance joined to lifetime and execution order**, not a particular renderer or emulator.

We construct a receipt with an explicitly versioned layout:

```text
AllMight-Book01-CPU-Trace-v1
texture_source_fnv1a=<decimal number>
mesh_source_fnv1a=<decimal number>
texture_weighted_rgb_sum=297
mesh_signed_double_area_q16=65536
```

The numerical outputs are not magic constants that the implementation merely prints. The executable reads the two actual files, decodes their payloads, computes the quantities and verifies these independently derived expected results. The input fingerprints are calculated over **all bytes of the source fixtures**, including format headers, whereas the computed results operate on the decoded pixel/vertex payload. A change in a header can therefore change source identity without necessarily changing a decoded numerical value. Conversely, a modified pixel changes both the source fingerprint and the reduction. These are different invariants and must not be conflated.

The fingerprint is the fixed 64-bit FNV-1a function already supplied by Pass 18. It is reproducible for the same ordered bytes, but it is **not cryptographic authentication** and should not be used as a security proof or as a substitute for a cryptographic artifact digest in research release manifests. We deliberately omit the temporary filesystem directory, process-local registry owner ID and wall-clock time from the receipt. If they appeared, repeating the identical operation would produce a different text file despite identical computation.

#### A real ownership boundary: disk bytes are not scene objects

The first boundary is asynchronous ingestion. Two names are submitted to the bounded `WorkerBatch`: `assets/tex.txr4` and `models/tri.msh1`. The worker owns a copy of its manifest, sorts the requested paths, reads under explicit file and aggregate byte budgets, and returns two `OwnedAsset` results. The caller may destroy the original request vector or close its local view without invalidating the returned bytes. There is no borrowed stream buffer escaping the worker. The producer might run on a thread, but `co_yield` itself never creates a thread: the single worker resumes the generator and transfers owned results under synchronization.

We identify the returned resources by both expected path and `AssetKind`, not just by whatever position a sorting algorithm happens to give them. A duplicate or wrong-kind input is an error. A missing file is also an error. We require exactly two successful outputs before attempting the next stage. This deliberately excludes a dangerous partial-success policy in which a missing mesh is silently replaced by yesterday's mesh or an empty vector.

The two domain decoders are invoked **before either asset is published**. That matters because the underlying registry guarantees per-asset transactional import, not an atomic transaction over an entire scene. Validating the texture alone and immediately publishing it would leave a partially updated live scene if the mesh then failed. Pass 19 builds a *local staging registry*. A failed second import causes local state to die, without exposing that staging registry as a live scene. When adapting the pattern into a real editor, scene publication must also be guarded by an owned scene-snapshot or transaction mechanism; preflight validation alone is not sufficient when multiple writers race.

The Pass 06 handles carry three independent pieces of logical identity, an `owner`, a `slot`, and a `generation`. They are **not pointers** and do not by themselves extend the lifetime of an asset. Here the registry's `lookup` returns a borrowed reference; our reduction finishes reading that reference inside one case arm before the later retirement step runs. Nothing caches a borrowed reference past `erase`. The final test deliberately asks for the old handles again and expects `LookupError::stale_or_dead`.

#### Derive the result, do not trust a screenshot

The TXR4 payload has width 2 and height 1. Each pixel contains four unsigned byte channels `(R,G,B,A)`. For this controlled CPU reference we choose the integer weighted-RGB sum

\[
L = \sum_{p=0}^{WH-1} \bigl(R_p + 2G_p + B_p\bigr).
\]

This is a *test reduction*, not a colorimetrically correct sRGB luminance conversion. Alpha is intentionally unused. For the first pixel `(255,3,4,255)`, the contribution is `255 + 2(3) + 4 = 265`. For the second `(7,8,9,255)`, it is `7 + 2(8) + 9 = 32`. Therefore the independent reference result is `L = 297`. The code iterates in groups of four bytes and accumulates in `std::uint64_t`, rather than allowing repeated byte-sized arithmetic to wrap.

The mesh stores three-dimensional positions as signed 16-bit integers measured in units of 1/256 world unit. Its one triangle uses `p=(0,0,0)`, `q=(256,0,0)`, and `r=(0,256,0)` in quantized coordinates. For a 2D projection of a triangle, twice the *signed* area is

\[
2 A_{xy} = (q_x-p_x)(r_y-p_y) - (q_y-p_y)(r_x-p_x).
\]

Substitution gives `256 × 256 - 0 = 65,536` integer squared quantization units, or a world-space twice-area of one when divided by `256^2`. A change in winding changes the sign, which is useful for exposing orientation errors; the output is not normalized with `abs`. The implementation promotes coordinates to signed 64-bit integers **before subtraction and multiplication**. A valid 16-bit displacement can approach 65,535, and multiplying two such differences can overflow signed 32-bit arithmetic. For arbitrary maximal triangle coordinates the cross-product difference fits within signed 64 bits, but it does not necessarily fit signed 32 bits.

This exercise uses actual decoded vertices and indices, not a hard-coded result. The MSH1 decoder already rejects out-of-range indices and incomplete triangles. The capstone relies on that contract while still checking there are at least three indices before accessing them. Such contracts should be explicit at a subsystem boundary; merely saying “the parser checked it” without referring to a concrete parser implementation makes a refactor dangerous.

#### Build the dependency graph from hazards

The work is divided into six host tasks: validate staged handles, compute the texture reduction, compute the triangle area, execute the CPU memory/retirement oracle, format the receipt, and retire the registry identities. The tasks share the exact `ResourceKey` representation used by Pass 08: `kind, owner, slot, generation`. Result slots are distinct keys so parallel *read-only* work is not serialized by a fake output collision.

```mermaid
flowchart TD
    A["1 — staged asset identity"] --> B["2 — texture reduction"]
    A --> C["3 — triangle area"]
    A --> D["4 — memory visibility oracle"]
    B --> E["5 — canonical receipt"]
    C --> E
    D --> E
    E --> F["6 — retire source handles"]
```

The graph is computed from declared resource accesses, not merely this drawing. Task 1 writes the two logical source keys, tasks 2–4 read them and write different result keys, task 5 reads all three outputs and writes the receipt key, and task 6 writes the two original identities to retire them after consumption. The actual planner may include additional transitive or explicit edges; what must be invariant is that every emitted dependency points forward in its topological execution order. The executable independently checks this edge-order property rather than accepting an ordered vector on faith.

A task DAG is **not an executor**, and a host DAG is **not a GPU synchronization primitive**. Pass 19 explicitly interprets the emitted `PlannedTask` order in a small sequential CPU executor. For Vulkan or Metal, the right actual pipeline stages, access masks, layouts, queue ownership transfers, timeline/fence completion and device-loss behavior would have to be mapped and validated independently. An edge in a CPU graph cannot replace any of those native contracts.

#### Why order and visibility are different

Pass 09 supplies a virtual memory-contract oracle. In the fourth task we allocate a conceptual GPU texture identity, write actual decoded pixel bytes from the host, and change the conceptual owner/layout to a compute-reader state using `execution_only_transition`. This transition intentionally does **not** publish the recent write epoch. The next read must fail with `consumer_not_visible`. If it succeeds, the example has failed to distinguish ordering from data visibility.

Only after an explicit `memory_dependency` from host producer to conceptual compute consumer does a read return the exact eight pixel bytes. That comparison is an independent byte oracle: the result is compared with the actual decoded texture, rather than with whatever the memory model happened to return. We then mark a virtual submission ticket in flight. Attempting to retire the resource before simulated completion must return `completion_pending`; after `observe_completion(1)`, retirement must succeed, and another read must fail with `already_retired`.

These API names sound GPU-like, but this remains a **portable CPU model**. No shader was dispatched, no GPU counter was sampled, and no device or queue was opened. That limitation is a strength of the test's interpretation: it can prove that our *host-side assumptions* enforce specific ordering and visibility rules, without implying that a particular native driver implements the mapping correctly.

#### Revision identity: stale work must not become current output

Even a perfectly valid receipt can belong to the wrong scene. Imagine a long-running file-import task starts when a scene is at revision 7. The user switches or edits the scene, advancing the live revision to 8, before the result is ready. A system that publishes solely because the job succeeded will overwrite more recent work. The Pass 19 sample demonstrates an optimistic revision check:

```cpp
// Illustrative excerpt; see the complete executable for the tested context.
// In a concurrent editor, both comparison and assignment must happen under
// one scene mutex or within one actor/serial executor.
bool publish(std::uint64_t expected, Receipt result) {
    if (revision != expected) return false;
    if (revision == std::numeric_limits<std::uint64_t>::max()) return false;
    visible.emplace(std::move(result));
    ++revision; // the same expected revision cannot commit twice
    return true;
}
```

The executable rejects an expected revision of 6 when the scene is at 7, accepts the correct revision 7 exactly once, and then rejects a second stale publication with expected revision 7 because the scene has advanced to 8. This is a **single-threaded reference protocol**, not a race-safe production scene editor. In an actual multi-threaded engine the equality check and commit must be made indivisible by a mutex, actor or appropriate compare-and-swap state protocol; source-resource lifetime and render-thread consumption still require independent ownership rules.

The core point is that *source content fingerprint*, *registry generation*, *scene revision*, and *GPU completion ticket* describe different kinds of identity and time. They cannot be replaced by a single “current version” integer. The source fingerprint answers “what bytes?”, the registry handle answers “which live logical slot?”, the scene revision answers “which publication epoch?”, and the completion ticket answers “has the dependent work finished?” Engineers who keep these separate can reason clearly about hot reload, experimental reproducibility and safe resource retirement.

#### Independent rejection cases and expected failures

The executable creates the required texture but omits the mesh. The input stage must return `read_failure`; no receipt may be produced. It then writes a mesh with a corrupted magic byte and expects `decode_failure` after the real file read. Finally it restores the valid mesh, truncates the texture's RGBA8 payload, and expects another `decode_failure`. These cases distinguish file availability from semantic format validation, and prohibit a valid-looking final output assembled from a mixture of good and invalid source objects.

A stronger test suite should eventually include duplicate manifest paths, symlink replacement races, invalid numerical indices, allocator-failure injection, cross-thread publication under competing revisions, interruption during file replacement, process restart, and actual GPU completion events. They should not be labeled “covered by the capstone” until a test explicitly exercises them. The Pass 18 reader already has independent tests for bounded requests, path policies, aggregate size limits and cooperative cancellation; Pass 19 consumes that reader rather than reimplementing those rules.

#### How to adapt the same trace into actual engineering projects

For a **Kairo or Maveb asset workflow**, the binary input is a verified asset or scene bundle, the typed registry stores a parsed mesh/texture or scene view, and the task DAG connects decode, upload, resource transition, computation/draw, readback and result capture. Borrowed CPU spans must end before ownership transfer, GPU resources must stay alive until a real completion signal, and the scene's revision gate must reject stale async jobs. The illustrative memory oracle is replaced by real Vulkan or Metal commands whose synchronization correctness is separately tested.

For **Kyoto collision broad-phase experiments**, the input identity should include scene generation, body topology, material assignment, collision regime, seed and frame configuration. The independent oracle is an exact pair-set or solver-state comparison, not a weighted RGB sum. The receipt must include both correctness and timing provenance; a fast candidate-pair path that changes physical solver behavior cannot be certified by timing alone. An immutable frozen evidence manifest plays the same role as the versioned receipt in this teaching exercise.

For **Vulkax scientific controls and an XR/foveation pipeline**, the important change is the deadline and confidence model. Vulkax requires independent residual and controlled-perturbation checks with separated validation data. XR requires a timestamped path from gaze sample and prediction through rendering, compositor and actual display; a CPU task graph cannot claim motion-to-photon latency without measured clocks. All these systems can reuse the foundation's idea of source-pinned owned inputs, explicit task and publication identities, rejected stale work, and independent correctness evidence without pretending they share a single physics or display backend.

#### Reconstruct and extend the project yourself

First, locate `tools/labs/book01_end_to_end_cpp23_lab.cpp` and redraw the task/resource graph from the **actual resource access declarations**, without using the diagram above. Explain every RAW and WAR edge the planner generates, and why two independent readers need not wait for one another. Then alter the mesh winding by swapping indices 1 and 2. Before running anything, derive the expected signed area and identify which assertion must change; after running, check whether the signed result agrees with the derivation. Repeat by modifying only a texture alpha value: the source fingerprint must change, but the weighted RGB sum must stay fixed. That distinction is a simple test of source provenance versus computational output.

Next, remove the `memory_dependency` call without weakening the output condition. The read should be rejected by the CPU oracle, teaching why a topologically correct plan cannot be used as a substitute for data visibility. Remove or move the virtual completion observation, and prove that early retirement fails. Finally, attempt to publish the same receipt twice against the same scene revision. Predict which attempt succeeds, why the second fails, and exactly which synchronization is missing before adapting the pattern to a threaded renderer.

For an independent acceptance exercise, instrument a real engine-owned asset and a real renderer command submission, while leaving the original capstone intact. Record the compiler version, all source and shader hashes, device/driver/SDK, input assets, task identities, observed completion and exact output oracle. If an actual native device run fails, show the failure and repair it; do not silently fall back to this CPU oracle and call the GPU path verified. The correct end state is not merely one more green test, but a learner who can trace every resource's lifetime and every claimed result back to reproducible inputs.

#### Pass 20 — When a correct output races against another correct output

Pass 19 produced a meaningful end-to-end CPU trace, but it exposed a particularly important limit: a successful file import, decoded asset, dependency plan and verified computation can all be correct **without having permission to become the currently displayed result**. A renderer and its asset workers are not isolated programs that politely finish one by one. They overlap in time. A modeler may open another scene, a shader may recompile, or an experiment may replace its reference dataset while yesterday's worker is still running. The old job's bytes might remain valid and its checksum might match, but its publication would now be wrong.

The smaller Pass 19 `Scene` intentionally illustrated revision gating in one thread. That is not enough when several workers may call `publish` at once. In particular, the naïve pattern

```cpp
// WRONG in concurrent code: a data race and a time-of-check/time-of-use hole.
if (live.revision == my_started_revision) {
    live.visible = my_finished_result;
    ++live.revision;
}
```

has two independent problems. Reading and writing `revision` without synchronization is a data race, which is undefined behavior in C++; and even if that access happened to be race-free, the equality check and assignment are not a single indivisible decision. Two workers can both read seven before either publishes, then both overwrite the scene while pretending to be its rightful next revision. Replacing `revision` with an isolated atomic integer would fix that integer's data race, but still would not automatically make the associated `visible` object update atomic with it. *Ownership of the content and atomicity of the publication decision must be designed together.*

The corrected executable retains the original educational `Scene` for comparison and introduces a real `ConcurrentScene`. A producer completely prepares an owned `Receipt` outside the scene mutex. It enters a short critical section, tests that the expected revision equals the actual revision and that increment will not wrap, swaps in the prepared value, increments the revision, and exits. A reader takes the same lock to copy an owned `Snapshot`, rather than returning a pointer into an object that another thread may replace.

The key property is **linearizability**. It means the concurrent operation behaves as if every successful or failed publication happened at one well-defined instant somewhere between its invocation and return. For our mutex-protected publisher, its linearization point is the locked update of the stored receipt and revision. No other publisher or snapshot can observe an intermediate state, because every such operation takes the same mutex. The implementation does not require every byte of the new receipt to be assembled under that lock. Moving expensive preparation outside the lock decreases contention while leaving the final decision indivisible.

We can derive the expected outcome of eight contending writers without relying on a scheduler guess. All eight submit independent owned candidates prepared for revision seven. Exactly one writer obtains the mutex while `revision == 7`; it swaps in its value and increments the revision to eight. Every later writer takes the same mutex and sees eight, which is not its expected seven, so it refuses without touching the visible result. Thus the number of wins is exactly one, regardless of which thread happened to arrive first. The identity of the winner is nondeterministic and must **not** be hard-coded by thread index. The correctness oracle checks the winner count, the new revision, and a coherent owned result whose origin remains valid. Scheduling fairness, thread creation order, and wall-clock timing are deliberately not treated as correctness oracles.

```cpp
// Condensed from the real Pass20 ConcurrentScene; the complete class and
// simultaneous-eight-worker fixture are in book01_end_to_end_cpp23_lab.cpp.
[[nodiscard]] bool publish(std::uint64_t expected, Receipt candidate) {
    // candidate is an owned value. Any potentially throwing caller copy
    // has already completed before the synchronized publication begins.
    std::optional<Receipt> prepared{std::move(candidate)};
    std::scoped_lock lock(mutex_);
    if (revision_ != expected ||
        revision_ == std::numeric_limits<std::uint64_t>::max()) {
        return false;
    }
    // The concrete Receipt uses a standard std::string plus integers.
    // static_assert checks that this optional swap cannot throw.
    visible_.swap(prepared);
    ++revision_;
    return true;
}
```

A second detail is easy to miss: if the live result already exists and our new candidate's publication could throw after deleting it, we might lose the previously valid scene. To avoid that, `Receipt` is an owned, nonthrowing-movable value and the executable contains `static_assert` checks that both its move constructor and `std::optional<Receipt>::swap` are `noexcept`. We build `prepared` before acquiring the lock. The locked swap-and-increment is then nonthrowing for this **specific concrete type**. The old result moves into `prepared` and its destruction happens *after* the lock is released. This is an important boundary, not a generic promise that any future asset type can be swapped safely under a mutex. If a later `Receipt` adds a throwing move operation, the compile-time assertions should fail so the publication protocol is redesigned before release.

The revision type has a second, independent edge case. If an unsigned revision at `UINT64_MAX` increments unchecked, it wraps to zero, potentially making an extremely old numerical identity appear current again. Therefore a saturated revision refuses further publication until a deliberate identity-epoch reset protocol exists. The test actually constructs that saturated state and checks that both the revision and visible value remain unchanged. The same principle explains why the earlier generational registry permanently retires slots rather than wrapping their generation counters.

The test starts the eight publisher threads behind a `std::latch` so all can attempt the same revision after the caller finishes preparing them. The latch improves the **chance of contention**, but it does not guarantee that all eight reach the mutex at the same machine instruction, nor does it prove every possible scheduling interleaving. The implementation also handles a less obvious failure: if creating the sixth thread throws an allocation or operating-system error while five threads are blocked on the latch, normal stack unwinding could wait forever for those blocked `std::jthread` objects to join. A small scope guard releases the start latch before the worker vector is destroyed, avoiding that exception-path deadlock. This is why lifetime reasoning must include *failure during preparation*, not just the successful operation.

The concurrent snapshot copies the visible receipt under the same mutex and returns an owned `std::optional<Receipt>` beside its revision. A later publication can replace the live scene while that returned snapshot stays coherent. A read-only borrowed `std::string_view` returned after unlocking would not provide the same safety. On a real engine, a heavy scene snapshot might instead carry a pinned shared immutable resource generation; that is an optimization with its own ownership and GPU-completion requirements, not permission to return an unprotected raw pointer.

A scene revision, a generational asset handle, a source fingerprint, and a GPU completion ticket remain **four different contracts**. The revision guards *which update may become live*. The generational handle guards *which logical slot is currently occupied*. The fingerprint records *which source bytes were read*. The completion signal guards *whether still-running device operations can release memory*. The mutex alone neither prevents a malicious filesystem symlink replacement nor guarantees that Metal/Vulkan has finished reading upload memory; those errors belong to distinct layers. The correct system connects those protections rather than pretending one mechanism solves them all.

#### Pass 20 — What the compilers actually taught us

During the integrated build, a prior NROM dependency-graph lab exposed a true C++ type-system bug. Its cartridge is move-only, but the code had written `const auto parsed = NromCartridge::from_image(...)` and later `std::move(*parsed)`. That last expression does **not** remove constness: it produces an rvalue reference to a const cartridge, which cannot bind to the ordinary move constructor taking `NromCartridge&&`. The compiler therefore searches for a copy constructor, finds that copying was deliberately disabled, and rejects the code. The correct local variable is `auto parsed`, preserving the owned `std::expected` as movable. The conceptual derivation is worth remembering: `std::move` is a cast enabling move overload resolution; it does not force a move or override ownership and const rules.

Another pre-existing bug came from the success operator on `std::expected`. Its `operator bool` is **explicit**. It is usable in contextual boolean expressions such as `if (outcome)`, `!outcome`, or `outcome && other`, but it cannot silently convert `std::expected<void, Error>` into a `bool` parameter in a call to `check(bool, message)`. The revised GPU test files add a well-typed overload taking `const std::expected<T,E>&` and asking `has_value()`. That overload is deliberately narrowly scoped to the test assertion function; it does not weaken the library error type or silently discard failures in production code.

A third issue is that passing `-std=c++23` does not guarantee every vendor compiler/standard-library combination implements the complete standard library. The Linux Clang 18 job lacked `std::expected` through its libstdc++ combination, because the library conditions availability on sufficiently complete Concepts support. The Linux CI now chooses Clang 19 and compiles an actual minimal `std::expected` probe before attempting the course build. On Apple's side, the older Xcode 16.4 SDK lacks the standard `std::stop_token` and `std::jthread` used by the coroutine-and-thread ingestion service. The supported future path is an AppleClang toolchain with those APIs, with a compile-time feature probe; pretending to exercise the feature through a skipped test would teach the wrong lesson.

Even the CI workflow's filename filters were part of the engineering result. A plain filesystem path containing `C++23` is not necessarily a valid GitHub Actions **glob pattern**: the workflow parser interprets `+` specially. The earlier workflow was failing *before creating any runner job*, so none of its thousands of GitHub failure entries proved an actual compiler failure. Replacing the unsafe pattern with `Supplementary/**` restored runner execution, and only then could the real move, conversion and toolchain problems become visible. This is why reproducible builds must distinguish configuration failures, compiler diagnostics, runtime test failures, and missing evidence receipts instead of treating a red workflow icon as a complete diagnosis.

#### Pass 20 — Independent invariants worth reproducing

The end-to-end fixture also gained two metamorphic tests. Such a test changes one controlled property of an input and checks the *relationship* between old and new output, even when the new complete output has not been independently tabulated. Changing a texture's alpha byte changes its whole-file source fingerprint, while preserving the weighted RGB sum because the equation intentionally ignores alpha. Swapping the triangle's second and third indices reverses its signed area, changing `+65536` to `-65536` in quantized squared units, and changes the mesh file fingerprint. Those are different contracts: byte provenance should react to both mutations, while each numerical output reacts only to inputs that are part of its defined computation.

Reconstruct these tests by hand before running them. Derive the texture sum, signed triangle determinant, and expected revision winner count from the contracts. Then change the CPU model to omit its memory dependency and explain the expected visibility rejection; remove the virtual completion observation and explain why retirement stays pending. For the concurrency test, sketch two, then eight, worker interleavings and identify the precise locked state transition that makes the observed outcome independent of scheduling. Finally, explain which assumptions would stop being valid if you replaced the `Receipt` with a raw Maveb Metal texture, a Kairo Vulkan descriptor, a Kyoto solver-state pointer, or an XR gaze-buffer view.

The corrected staged lesson still does not establish native GPU correctness, thread-sanitizer coverage, a hostile-filesystem sandbox, production renderer hot reload, or exact research replication. These require their own platform evidence and independent oracles. The educational gain in this pass is that the boundaries are now visible in code, proven under appropriate assumptions, and tested against concrete failure cases rather than hidden by a single claim of “the project works.”

#### Evidence boundary and book-release gate

This pass adds a complete portable C++23 test **source** grounded in existing APIs and a connected lesson. It is staged in the same additive manuscript workflow and does not shorten, rewrite, regenerate or compile the original multi-thousand-page book. The outcome of the new integrated 18th test **must not be reported as passing** until the full Git checkout compiles and executes it under GCC, Clang and applicable AppleClang configurations with real CTest/JUnit receipts. A staged source file, a CMake test declaration or a text receipt format is not a measured result.

The next release gate is therefore an observed **18-target** build and tests, validated source-preserving insertions, and a chapter-by-chapter narrative transition pass. Native Vulkan/Metal, editor hot-reload races, real asset formats and research-correctness claims remain separate deliverables. We preserve the whole prior manuscript while adding this new, precisely bounded witness at the existing build/evidence chapter's conceptual boundary.
