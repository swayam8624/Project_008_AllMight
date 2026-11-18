### NES-02P — Prove the lifetime contract of a running emulator

The earlier pages of this chapter already explained value transfer, references, borrowing, constructor order, RAII, smart pointers, exceptions and resource ownership. We must not replace those accounts with a short checklist. Instead, we need to answer a harder question that a professional reviewer or research examiner can reasonably ask: **how do we know those language rules were applied correctly in the emulator we have been building?** A C++ compiler accepting the program is not proof that its resources live long enough, that failures preserve a prior state, or that a moved-from object remains usable in the particular way a programmer assumes. A clean run under a sanitizer is useful evidence but is not a general proof either.

This chapter's project evidence uses the same C++23 cartridge pipeline that the first three milestones built, rather than introducing a disposable example. The source is `tools/labs/nes_ownership_proofs_cpp23_lab.cpp`, and the common runtime comes from `tools/labs/nes_course/cartridge_runtime.hpp`, `nes_bus_nrom.hpp` and `ines_layout.hpp`. These headers contain the actual code executed by NES-01, NES-02, NES-02T and this new NES-02P suite. The labels deliberately preserve the larger roadmap: NES-02P is a *proof pass* on the existing system; NES-03 will still be the instruction-executing 6502 CPU.

#### Start with claims, not code coverage

A test run is meaningful only if we can say which proposition a failure would refute. For a complete cartridge-loading transaction, define the externally observed state

$$
S=(G,R,P,C),
$$

where $G$ is the mounted cartridge revision, $R$ represents selected console RAM bytes, $P$ represents selected PRG-ROM bytes in CPU address space, and $C$ represents selected CHR-ROM/RAM bytes in the PPU pattern-table address space. These are observables, not pointers into the object's private storage. They let us compare values without continuing to access a destroyed old cartridge.

We choose two transaction properties. **Failure preservation** means that, for each declared rejected input and injected staging failure, the observable state does not change: $T(S,x_{\mathrm{failed}})=S$. **Successful publication** means that a valid replacement changes the cartridge values and advances the revision once, while keeping internal RAM intact under the *explicit teaching policy* of this laboratory: $G'=G+1,\;R'=R,\;P'=P_{\mathrm{new}},\;C'=C_{\mathrm{new}}$. The distinction is essential: merely seeing an error string does not establish that a corrupted or partially replaced asset has not escaped into the running engine.

Neither equality of a handful of witness bytes nor a successful test run establishes full-array equality. For this reason, the previous NES-02 lab additionally checks every CPU RAM mirror address and the full NROM-128 mirroring window. The new NES-02P suite tests the lifetime and publication *structure*, and the acceptance work should combine both kinds of evidence. The full contract is a set of mutually supporting proofs, not one fortunate green bar.

#### The object that throws does not get its own destructor

Construct a record with two resource-bearing members, A and B. A finishes construction. During B's constructor an injected fault occurs. It would be incorrect to assume that the destructor of the complete record runs, because the complete record never finished constructing; it would also be incorrect to assume that B's destructor runs after its own constructor failed. Yet C++ must destroy the fully constructed A while unwinding. This is precisely why each acquired handle should immediately be placed into a member owner.

The event tracer in the new lab records signed identifiers without dynamically allocating memory. A positive identifier means that a member completed construction; a negative identifier means that a member's destructor ran. The attempted but failed construction of B records a distinct fault identifier. The exact successful sequence is:

```text
construct A
construct B
enter complete-record constructor body
enter complete-record destructor body
destroy B
destroy A
```

The failure sequence is instead:

```text
construct A
attempt B; B throws before completing
destroy A
propagate exception to caller
```

There is no complete-record destructor call and no B destructor call in the failed sequence. The trace is compared against exact expected events rather than inferred from printed addresses, which might change across machines and optimizations. A failed trace would reveal either a mistake in our test model or a mismatch between our prediction and the implemented control flow.

**Why did we avoid writing these events to a growing vector?** The point of the exercise is exception cleanup. A vector's own allocation could unexpectedly fail during logging and obscure the constructor failure we intentionally injected. A fixed-size recorder makes the test's observation mechanism simpler than the behavior it observes. An instrumentation system is part of the experiment and has its own potential failure modes; controlling those modes is research methodology, not just testing style.

#### An xvalue expression is permission to move, not the move itself

One of the easiest C++ misunderstandings is the idea that `std::move(x)` physically moves the resource. It does not. It performs a cast yielding an xvalue expression, which lets subsequent overload resolution select operations accepting rvalues. The actual transfer, if any, happens in a selected move constructor or move assignment operator. A type can also lack a move operation or have one whose effects are more expensive than expected.

The proof suite creates a small move-only ticket that owns responsibility for exactly one simulated release. It then binds a named rvalue reference to `std::move(first)`. At that instant the original owner still owns the ticket, and no release has taken place. Only after explicitly constructing `second(std::move(alias))` does the ownership duty transfer. The test makes a second move and finally destroys all three wrapper objects. It must observe exactly one acquisition and one release.

This is why an engine must not implement two owners by copying a raw handle and assuming the compiler will know to release it only once. A move constructor for a custom native resource must transfer the handle *and neutralize the source's release duty*. The reference uses `std::exchange` to set moved-from fields to a harmless empty state. The concrete `NromCartridge` is move-only, has independent vector-owned PRG/CHR byte regions, and is statically checked to be nothrow move constructible. These constraints are part of the reason the previous cartridge transaction can commit safely.

A separate principle follows: `noexcept` is a promise about exceptions escaping an operation, not a request to the compiler to make its implementation infallible. Violating a `noexcept` promise causes termination. Checking `std::is_nothrow_move_constructible_v<T>` tells us about the declared operation, but a professional review must still verify that its implementation respects the promise. For standard-library member types, the relevant guarantees also depend on allocator and type properties. Do not replace that reasoning with a single build's runtime behavior.

#### Catch an attractive but false transaction pattern

A common source of accidental data loss is replacing a `std::optional<T>` value directly with `emplace` under the assumption that a failed constructor leaves the old value present. The general C++23 contract is different. `optional::emplace` destroys the previously contained value, then constructs the new one; if construction throws, the optional is disengaged. A test using a deliberately throwing constructor demonstrates this by observing one destruction of the previous value and an empty optional after the exception.

This is a **counterexample to a universal claim**, not a criticism of every `optional` use. Our current cartridge bus already owns a fully constructed `NromCartridge` before calling its `insert` method. The bus insertion checks that the candidate is valid and commits through a move constructor known to be `noexcept`. In that narrow case the replacement construction has no permitted throwing path. The safer rule to remember is therefore:

> Prepare under temporary ownership. Publish only through an operation whose failure behavior is proven for the concrete type.

If we later add a member with a potentially throwing move, we must re-evaluate the commit path. An alternative is to stage a complete immutable bundle behind a separately owned pointer, then atomically or synchronously exchange the handle under a defined reader-lifetime policy. That future design has a different concurrency and retirement problem; it should not be presented here as if a `shared_ptr` automatically solves all races or as if CPU-scope destruction proves GPU work is complete.

#### A span is never the owner of the game's bytes

The file parser accepts `std::span<const Byte>`, which borrows a contiguous region. A borrowed span is useful because it exposes the size and makes decoding independent of a particular container, but it does not extend the lifetime of the bytes. If we kept that view in a long-lived cartridge after the file buffer was cleared, subsequent reads could access dead storage. Neither `const` nor the mere presence of a size member makes a dangling view safe.

Our test deliberately creates a synthetic one-bank iNES image, loads it through `CartridgeRuntime::replace`, then clears and releases the original input vector. It **never reads through the now-invalid borrowed view**. Instead it reads the mounted cartridge's CPU-visible PRG byte and demands the value from before the input was destroyed. That establishes the intended ownership separation for this test: staging made a deep copy into cartridge-owned storage.

There is a crucial experimental distinction. Intentionally dereferencing a stale pointer to see whether the program crashes is a test of undefined behavior with no reliable expected output, not a constructive proof. A sanitizer may detect some invalid accesses but cannot promise to detect all of them. The legal test proves that the good path has independent ownership; a static/lifetime review rejects the bad path without executing it. The two forms of evidence complement one another.

#### Test the real failure transaction, not a second imitation

The NES-02P executable includes the actual `cartridge_runtime.hpp`. It loads a CHR-RAM cartridge, writes recognizable values to CPU RAM and the cartridge's writable pattern table, then captures a value witness. A second image declares two PRG-ROM banks and one CHR-ROM bank with deliberately different bytes in both CPU halves. The test first tries every fault point supported by the runtime: failure before staging, failure after staging, and an intentionally thrown `std::bad_alloc` after staging.

For each refused load, the witness must equal its original value, including the cartridge revision. A malformed signature produces another rejection and must leave the same mounted cartridge intact. Only the successful attempt may advance the revision. After success, the two CPU-visible PRG windows must carry different expected values, the newly mounted CHR-ROM must not accept pattern writes, and the intentionally retained internal CPU RAM value must still appear through the corresponding mirrored address.

**Simulating `std::bad_alloc` after staging is not proof of recovery from real operating-system memory exhaustion.** It tests the exception-to-error mapping and cleanup on one deliberately selected control-flow edge. Exhaustively injecting allocation failures at every possible allocator call would require different infrastructure, and signal/process-level out-of-memory behavior can differ by platform. Such an expansion belongs in a separate fault-model experiment, with each failure point identified and counted rather than simply labeled “tested.”

We also keep the real emulator hardware and the C++ teaching model separate. This project's bus deliberately reports unimplemented PPU/APU/controller regions instead of pretending to simulate all NES electrical open-bus behavior. Switching cartridges without resetting the CPU and PPU is a chosen laboratory operation, not an emulator feature claim. Boot tests, cycle-level instruction traces, mapper edge cases, frame timing and audio belong to the later NES milestones and require independent validation.

#### Weak observation does not justify shared ownership everywhere

The chapter also teaches `std::unique_ptr`, `std::shared_ptr` and `std::weak_ptr`. We check one more distinct ownership proposition: a weak observer can temporarily acquire a strong owner while an object is alive, but after all strong owners are gone, `lock()` returns empty. It does not resurrect a destroyed object. This is appropriate for an observer that must not extend a cache record's lifetime.

Yet our cartridge bus does not use `shared_ptr` merely because the concept appears in the curriculum. A single owner with clearly bounded borrowing is simpler for the current single-threaded emulator model. In a real multithreaded asset cache, a shared-ownership decision should be driven by documented reader lifetime requirements and synchronization costs, not by a blanket suggestion to replace every raw pointer with a shared pointer.

#### Build a layered confidence argument

Here is how to decide what the evidence establishes. **Static checks** prove properties that can be expressed in the type system: the cartridge is noncopyable and has a nothrow move constructor; an owning ticket is noncopyable. **Deterministic event traces** show a chosen constructor/destructor execution order without invoking undefined behavior. **Byte-based fixtures** show that the loaded cartridge retains data after the external input buffer has been released. **Typed error tests** show that particular failures are surfaced and do not silently replace live state. **Sanitizers** may discover additional illegal accesses or overflows during those specific executions. **Code review against the C++23 standard** identifies behavior a single compiler run cannot establish.

These sources of evidence answer different questions. A successful test does not turn a platform-specific assumption into a language rule. A standards quote does not prove that our own implementation obeys it. A sanitizer does not certify a program as safe. A source-file search does not certify complete coverage of a research book or a third-party emulator repository.

For a research-grade extension, add a machine-readable report recording compiler name and version, exact compilation flags, sanitizer availability, executed test-case names, assertion counts, source commit and a fixed random seed for any generated tests. Then perform a metamorphic check: create two distinct buffers containing identical iNES bytes and require the same parsed cartridge behavior; mutate only the input buffer **after** a successful load and require that the mounted cartridge remains unchanged; change one permitted flag without changing unrelated decoded fields; and repeat failed transactions many times while checking the full state witness. These are stronger than counting printed “PASS” labels.

#### Reconstruction and failure-analysis exercise

Implement the five proof groups on a blank screen, then compare their logic with the committed reference. First, construct a two-member object that throws during the second member's initialization and predict the exact destruction trace, including which destructors **must not** run. Next, create a move-only ticket and show that binding an rvalue reference changes no ownership; only constructing a new owner transfers it. Third, prove that the `optional::emplace` replacement can lose the old value on a throwing constructor. Fourth, use an owned cartridge and a cleared input vector to demonstrate that the running bus reads copied data, not a transient borrow. Fifth, test a rejected new ROM while recording the old revision and memory bytes. Include one weak-observer test to make clear why non-ownership has a different role from lifetime sharing.

Then deliberately alter **only the test fixture**, not the repository's production emulator: let a test's expected destructor order be wrong, or inject a failure at a different point. Verify that the suite fails with a useful diagnostic. A test that cannot detect a controlled violation is not convincing evidence, regardless of how many lines it contains.

For this milestone, *completion* means being able to explain every invariant before running the test, derive the expected event traces, distinguish language guarantees from runtime observations, identify where sanitizer and synthetic failure evidence stop, and reproduce the same NROM ownership behavior from the previous chapters using the single shared C++23 implementation.

**Primary contracts and limits.** Compare object construction and destruction with the C++23 [object lifetime](https://eel.is/c++draft/basic.life) and [class initialization](https://eel.is/c++draft/class.init) clauses; review [optional::emplace](https://en.cppreference.com/w/cpp/utility/optional/emplace), [std::move](https://en.cppreference.com/w/cpp/utility/move) and [std::expected](https://en.cppreference.com/w/cpp/utility/expected). Code uses synthetic iNES inputs and focuses on the declared legacy NROM cases. The lab is not a full NES emulator, a concurrency-safe loader or a substitute for native Vulkan/Metal lifetime verification.

**Next subsystem handoff.** Chapter 05 will apply these ownership proofs to a generic asset pipeline, with type-specific import validation, stable asset identity and clearly separated preparation/publication steps. The NES program data becomes one concrete asset kind alongside an engine's texture or mesh resource, while the learner continues implementing the core contracts in C++23.
