# Book 1 — Final mastery laboratory: from bytes to evidence

**Teaching boundary.** This concluding Book 1 laboratory is not a replacement for the sixteen original long-form chapters, the eighteen precisely positioned C++23 additions, or the original multi-thousand-page manuscripts. It asks the learner to *use* their material in a system they can inspect, explain, test, break, fix, and defend. A passing reference lab does not establish that an unrelated downstream game engine, renderer, emulator, simulation, or research paper is correct.

## The real graduation problem

Imagine two students who can each define a pointer, an exception guarantee, an atomic operation, a build system, a GPU fence, and an image format. The first can repeat the definitions. The second can take a report that an image was occasionally corrupted, follow the input bytes, lifetime, asynchronous publication, command submission, visibility, numerical work, and eventual destruction, and explain precisely where the failure becomes observable. Book 1 aims to develop the second ability.

There is one continuous thread through all the material: **an artifact is usable only if its representation, identity, lifetime, execution history, and evidence agree**. A well-formed file is not automatically a well-formed domain object. A valid domain object may still belong to an old scene revision. A correct task graph is not a proof that the GPU has observed its predecessors' writes. A command that has been submitted may not have completed. A benchmark may faithfully time the wrong workload. A green build says nothing about an API that was never linked or a source snapshot that was not the one used to make the PDF.

The final exercise therefore deliberately combines ideas that are often taught separately: numbers and binary layouts from Chapter 1; object placement and aliasing from Chapter 2; construction, moves and failure guarantees from Chapters 3–4; generic import and constraints from Chapters 5–6; typed interfaces, data structures and CPU traces from Chapter 7; dependency graphs from Chapter 8; memory ordering from Chapter 9; reproducibility and measurements from Chapter 10; borrowing and buffered lifetimes from Chapter 11; command and completion states from Chapter 12; and the capability, resource, frame, synchronization and callback contracts in Chapters 13–16.

The actual checked-in integrated reference executable is `tools/labs/book01_end_to_end_cpp23_lab.cpp`. Begin by reading it from its file inputs through its receipts rather than reading isolated function names. The executable reuses the real previously authored implementations. The material below asks you to extend that reference to your own domain; it does **not** claim that completing a synthetic fixture produces an engine or scientifically validated simulation.

## First investigation: prove the exact bytes before interpreting them

Start with one short binary asset whose schema you control. The reference uses a deliberately small `TXR4` texture and `MSH1` quantized triangle because their complete byte sequence fits under scrutiny. A PNG, glTF, KTX2 texture, COLMAP model or game cartridge is a different schema. Do not call a teaching fixture one of those formats. Specify the header length, field order, widths, endianness, legal dimensions, payload count and maximum accepted file size before writing the decoder.

For every field in the schema, decide which mathematical set it represents. An unsigned 16-bit length denotes integers in the range 0 through 65535; it does not denote a pointer or the size of a C++ vector on the host. A signed coordinate value has a representation whose scaling factor is part of the format, and a normal C++ integer expression may overflow before you ever narrow it to a smaller field. A file offset becomes meaningful only relative to a specific complete source buffer. Treat `size_t`, `uint32_t`, file offsets, object indices and persistent identifiers as different contracts, even when two of them happen to have the same bit width on your laptop.

Write down the preconditions for a texture before allocating its pixels. A robust length check must account for multiplication overflow. Instead of first computing `width * height * channels` and hoping the result fits, establish bounds progressively. The following small function models a general policy. Its exact maximum is an application decision; the ordering of its checks is the lesson.

```cpp
#include <cstddef>
#include <expected>
#include <limits>

// This is a teaching-level size gate, not a PNG/KTX/glTF decoder.
enum class ImageSizeError { zero_extent, multiplication_overflow, budget_exceeded };

std::expected<std::size_t, ImageSizeError>
checked_rgba_size(std::size_t width, std::size_t height,
                  std::size_t byte_budget) noexcept {
    if (width == 0 || height == 0)
        return std::unexpected(ImageSizeError::zero_extent);

    // Demonstrate that we must prove each multiplication will fit
    // BEFORE evaluating the multiplication itself.
    constexpr std::size_t channels = 4;
    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (width > maximum / height)
        return std::unexpected(ImageSizeError::multiplication_overflow);

    const auto pixels = width * height;
    if (pixels > maximum / channels)
        return std::unexpected(ImageSizeError::multiplication_overflow);

    const auto bytes = pixels * channels;
    if (bytes > byte_budget)
        return std::unexpected(ImageSizeError::budget_exceeded);
    return bytes;
}
```

The first test is not “it loads my example file.” It is the precise boundary table: zero width, zero height, a one-pixel image, maximum legal size, one byte above the budget, a width/height pair whose multiplication would overflow, truncated headers, trailing bytes when forbidden by your format contract, and random malformed files under a reproducible seed. For each rejected input, check not only the error code but also that the previously published asset remains unchanged. File-provenance tests should compare a checksum of all original bytes against a separately computed expected value; they should not trust a value that the same decoder simply copies into its own report.

## Second investigation: follow ownership instead of addresses

The importer starts with a buffer of owned bytes. A `std::span<const std::byte>` can inspect that allocation without taking ownership of it. A parser may hand out views only while their owner remains live and unchanged. The decoded texture owns its pixel data. A generational registry owns a slot for that asset and provides a typed handle. A renderer may later resolve the handle into a separate GPU resource with yet another lifetime.

Draw the ownership graph, not only a call graph. For each arrow, mark whether it is an owning value, move-only owner, shared immutable owner, borrowed view, or persistent logical identifier. A raw pointer points at storage; it is not an ownership proof. A `std::move` expression is a cast permitting a move constructor; it cannot erase a `const` qualifier, conjure a missing move operation or make a dead source live again. In the integrated CI repair, a `const std::expected<NromCartridge,...>` was precisely the reason a move-only cartridge could not be transferred. The corrected local expected is non-const so its contained cartridge can actually move.

Use the real registry in `tools/labs/nes_course/generational_asset_registry.hpp` to test three identities separately: the persistent file identity, a runtime owner/slot/generation handle, and the scene revision that controls publication. Load asset A into a slot, erase it, reuse that slot for B, and prove the old handle for A never resolves B. Then restart the process and demonstrate why runtime slot identity should not be written as a durable file identifier. Finally construct a candidate output for an old scene revision, publish a newer scene, and prove the late candidate is refused.

## Third investigation: make failures observable without destroying success

The strong exception guarantee can be stated as a concrete property: when a request fails, the previously valid externally visible state remains unchanged. An implementation that clears the old scene before parsing the new one cannot satisfy this property merely by catching an exception. Conversely, the general API contract of `std::optional::emplace` does not promise to preserve the old value on failed construction. The right approach for this particular teaching pipeline is to prepare and validate a complete owned candidate, then publish it using a move/swap operation whose exception properties have been checked for the concrete type.

You must describe both expected and unexpected failures. A malformed file can produce a typed `std::expected` error. A memory allocation may throw `std::bad_alloc`. An operating system call may fail. A worker may be cancelled after I/O but before publication. A device submission may return an ambiguous status where the command *might* be executing. Each deserves a distinct observed state; collapsing them all into “false” prevents safe recovery.

In the reference `ConcurrentScene`, all eight workers prepare owned receipts for the same revision and pass through one synchronized publication gate. Exactly one candidate can succeed, while readers obtain owned coherent snapshots. The result of `std::optional<Receipt>::swap` is proven nothrow for this receipt type. The revision cannot silently wrap. The thread-start latch is released even when constructing a worker throws, so already-started `std::jthread` objects can finish before destruction joins them. Understand why this last guard is necessary; it is an exception-safety problem in a concurrent subsystem.

For a deeper exercise, change the protected receipt into an immutable `std::shared_ptr<const SceneSnapshot>`, and carefully decide where copy/move/allocation can occur and how revision update and pointer publication stay one operation. Do not infer that a shared pointer is sufficient to retire an underlying GPU buffer: host ownership and device completion are separate facts.

## Fourth investigation: the graph is a dependency contract, not a stopwatch

A scheduler's nodes have identifiers, resource read/write sets, execution domains and explicit edges. When two nodes read the same immutable input, they may often run concurrently. When one writes a resource another reads, the graph must make the write visible before the read in the appropriate execution domain. When two nodes write the same resource, their relative ordering is a policy decision rather than a happy consequence of insertion order.

The reference `work_dependency_graph.hpp` computes a stable CPU plan with Kahn-style topological ordering and hazard-derived dependencies. Reconstruct the six-task trace by hand and compare it with the executable's independent edge oracle. Add a cycle that references real task IDs and require a typed rejection rather than “deadlock later.” Then add a task referencing a nonexistent predecessor and demand an error before execution starts. A reported critical path only has meaning under the model's assumed costs and dependencies; it is not a measured GPU command-buffer duration.

For a parallel CPU variant, keep immutable source snapshots separate from writable destination buffers. Record which thread owns the writable region and what happens on cancellation. A mutex protects a specific host critical section. It does not magically insert a Vulkan pipeline barrier or promise that Metal has completed a command buffer.

## Fifth investigation: distinguish visibility, completion and safe retirement

Consider the following tempting sequence: a producer writes a staging buffer, enqueues a GPU command, removes the source asset, and reuses the staging allocation when the submit function returns. The submit call returning is not a device-completion signal. The queue may still be reading the allocation. Even after the queue completes a compute step, a consumer on another pipeline stage may need a memory dependency to see the writes, and a presented swapchain image may have different availability rules than an ordinary temporary buffer.

The Book 1 CPU reference oracle deliberately refuses an execution-only hand-off that lacks the needed memory-visibility relationship; it separately requires modeled completion before retirement. State the limitation: these checks test the **model**, not a native Vulkan or Metal driver.

The backend project tracks ask the learner to write an evidence table for submission success, fence or completion-handler observation, memory visibility, image availability, resource destroyability, and scene relevance. Do not merge the columns. On native macOS Metal, a command-buffer completion callback has its own ownership and Objective-C++ type rules. In the integrated repair, an ordinary local variable named `id` hid the Objective-C `id` type needed by a callback declaration. On AppleClang, that is a compilation error, not a GPU timing anomaly.

## Choose your continuing project

All four routes below reuse the shared foundations, but each has a different domain oracle. The course never requires an artificial NES use case where a real engine, simulation or research workflow explains the idea better.

### Route A — NES debugging and state machines

Extend the existing NROM cartridge and bounded 6502 interpreter. Your system owns immutable ROM bytes, validates the reset vector, mounts the bus, executes a short synthetic program, and emits a typed trace recording registers, flags, effective addresses and cycle counts. Implement breakpoints and alias investigations using the existing debugger source rather than copying its implementation into a second toy program.

Write a table of actual implemented opcodes. The reference is a deliberately bounded subset and cannot be presented as a full 6502, a PPU, an APU, all NES mappers or a commercial-ROM emulator. Your acceptance gate checks each supported instruction against an independently derived register/memory trace, an invalid-opcode error path, a wrapped address case and a deterministic exact source hash. The next graphics/audio chapters belong to a deeper emulator track, not to a claim made by these Book 1 tests.

### Route B — Kairo and Maveb editor asset reliability

Extend the existing texture/mesh format reader and generational store into a small editor-like scene loader. Your workload is to reopen a scene, replace one material while a second asset is loading, and preserve the old visible state if the new load fails. An asset identity in the saved scene is not the same as a runtime handle. A GPU upload is not the same as publication. A material's displayed pixels depend on draw-state and shader correctness beyond the file decoder.

Include a deterministic CPU-rendered or numerical oracle. For example, compute a weighted RGBA pixel statistic and a signed triangle area from the decoded bytes; include alpha-only and reversed-winding metamorphic changes. The reference expected results are not enough for a production renderer; they serve to expose incorrect input routing and stale output. A native Vulkan or Metal path must add actual buffer upload, synchronization and readback checks when the hardware and SDK exist, with a CPU oracle and source-pinned receipt.

### Route C — Kyoto broad-phase and physics experiment provenance

Use a bounded scene with AABBs as the input domain. Given an exact geometry snapshot, enumerate a reference set of all candidate-overlapping pairs under an explicitly chosen interval convention. Run a broad-phase implementation and independently compare its candidate pair set. A fast candidate enumeration is not proof that downstream narrow-phase contacts are correct, and a collision visual that appears less folded is not by itself a validated physical simulation.

Write your experiment identity so it includes scene seed, material and regime, object count, frame count, geometry convention, algorithm selection, source commit, build flags, compiler, device class and whether counts were evaluated against an exact pair oracle. Treat independent seeds and split leakage explicitly. Use paired per-scene comparisons when evaluating performance, and avoid combining incompatible workloads into one seductive but meaningless speedup ratio. The Book 1 skill being assessed is trustworthy software/evidence construction; the novel broad-phase physics algorithm is a later research track.

### Route D — XR gaze-to-render and reproducibility

Start from a tiny recorded gaze/time stream. Give every timestamp a clock domain, unit and explicit sample validity rule. Decode the stream into owned records, reject invalid intervals, and construct a task graph for gaze update, foveation decision, rendering and observation. Include deterministic synthetic timelines to test whether a late gaze sample is rejected or tagged with an explicit policy.

Do not report motion-to-photon latency from host `std::chrono` around a CPU function. That measurement needs clock-domain alignment, display/scanout or equivalent presentation evidence, and a declared instrumentation path. Likewise, real user traces may carry privacy obligations that synthetic records do not. The Book 1 completion gate is a coherent pipeline with visible failure handling and clock provenance, not a finished VR device benchmark.

## The independent evidence packet

Choose one route and produce a reproducible evidence packet, not just a screen capture. Start with an *immutable source identity* and a plain description of what the system guarantees. Record the compiler and standard-library versions, exact CMake configuration, test inventory, build exit status, unskipped test results and the fixture source hash. Keep raw test output alongside any pretty summary. State whether you executed CPU-only models, native GPU work, or merely inspected source.

The packet needs five deliberate negative cases: malformed input, stale asset or scene identity, failure during candidate construction, wrong or missing dependency, and attempted retirement before confirmed completion. A sixth case must challenge the performance protocol: a run with a changed or missing workload input must not be silently compared to a different run. For each case state **the exact earliest layer expected to refuse it** and verify that downstream side effects have not occurred.

```cpp
// A minimal receipt schema. This is a design example: real runs must
// fill every field from observed evidence, not from a guessed value.
struct TestReceipt {
    std::string source_commit;       // Exact tested source revision.
    std::string compiler_identity;   // Compiler + standard-library version.
    std::string input_digest;        // Digest of bytes actually consumed.
    std::string test_name;           // Stable test identity.
    std::string oracle_description;  // Independent expectation, with units.
    bool actually_executed = false;  // False for a planned or skipped test.
    bool passed = false;             // Meaningful only when executed.
    bool native_gpu = false;         // Never inferred from a GPU-themed title.
};
```

Now make a table for each important claim. The columns are: claim, input identity, independent oracle, actual execution command, observed output, tested failure cases, limitations, and source record. A green CI result under one commit cannot certify a later changed source without rerunning the checks. A CPU simulator's execution success cannot certify native GPU memory ordering. A performance win cannot by itself certify physics correctness or absence of race conditions.

## Final demonstration and oral defense

The final demonstration is intentionally small enough to understand completely. Explain one byte of input all the way to its decoded meaning. Explain why an owned object survives the end of its decoding function. Show a stale handle being rejected and describe the generation/owner scope. Trigger one rejected scene publication and identify the linearization point of a successful one. Draw the dependency graph and justify one read-after-write edge. Show a missing-memory-visibility refusal and a completion-gated resource retirement. Finally, present one measured result without confusing correctness, latency, throughput, or source identity.

After the demonstration, answer the following objections in connected prose. How would your program behave if a file were truncated after its header was read? What if a user changed scenes between a worker's last check and its publication? What if two threads both wanted to publish revision seven? What if an allocation threw after the old scene was destroyed? What if a GPU queue accepted a command but later lost the device? What if an apparent tenfold speedup came from changing the number of candidate pairs? What does your test suite still not tell you?

The reference course can certify **the named, actually executed labs and their contracts**. It cannot certify that a student can independently implement every academic paper, write a complete AAA engine, replicate human-perception experiments, or produce a publication-quality figure without further practice. True mastery is demonstrated by deriving invariants, detecting false premises, carrying out independent tests, and revising the design under new evidence.

## Book 1 acceptance boundary and continued study

The Book 1 source release consists of the original sixteen Computing Foundations reading chapters, eighteen additive labs distributed at their exact first-use anchors, and this final integrative challenge. The additive insertion program can recover the exact original source by removing only the uniquely marked lessons. The default portable C++23 test suite has eighteen registered executables and eighteen registered CTests; separate optional native GPU probes require explicit hardware invocation and independent review.

For the next books, carry forward this distinction between **a representation, an owned object, a mathematical model, a runtime result, and observed evidence**. The mathematics, camera/geometry, light transport, physics/engine, native GPU, and XR books should never spend their first pages pretending that these contracts were already universally solved. They should reuse what has been proven, challenge the assumptions that change, and continue from there.
