### NES-02T — A cartridge replacement is a resource transaction, not an assignment

The preceding discussion explains how a class can protect an invariant through its constructors, how a move expression permits—but does not guarantee—resource transfer, and how copying differs from merely duplicating object representation. The NES project now gives those ideas an observable consequence. A user loads a new game while a previous game is still mounted. The new file may contain a bad signature, claim an unsupported mapper, demand impossible bank counts, or trigger an allocation failure while the loader is copying its bytes. What must happen to the running system?

For this chapter we choose a demanding but understandable answer: **a failed cartridge replacement must leave the previously mounted cartridge untouched**. Its program bytes, writable CHR RAM, the CPU's internal RAM and the recorded cartridge generation remain observable exactly as before the attempted replacement. A successful replacement must publish exactly one fully constructed cartridge and then advance the generation. These rules force us to design ownership and state transitions instead of relying on a hopeful sequence of assignments.

This milestone is called **NES-02T** because it extends NES-01's validated format and NES-02's owned NROM/bus model. It does **not** rename the planned NES-03 CPU instruction stage. Our full C++23 laboratory lives at `tools/labs/nes_cartridge_transaction_cpp23_lab.cpp` and consumes the same `nes_course/ines_layout.hpp` and `nes_course/nes_bus_nrom.hpp` used by the previous exercises. The code must never become a third unrelated copy of the cartridge parser or bus.

#### Begin with the state that must survive failure

A cartridge loader is not simply a function whose output is a pointer. It is a transformation of a running system. Let its observable state be

$$
S=(R,C,G),
$$

where $R$ denotes the console's internal RAM, $C$ the currently mounted cartridge and its own writable memory, and $G$ a monotonically increasing cartridge revision identifier. In this chapter's single-threaded model a failed replacement obeys

$$
\operatorname{replace}(S,\text{unusable input})
\longrightarrow (S,\text{error}),
$$

whereas a successful replacement obeys

$$
\operatorname{replace}(R,C,G,\text{valid input})
\longrightarrow (R,C_{\rm new},G+1).
$$

The new cartridge gets its own PRG and CHR storage. The CPU's internal RAM is retained deliberately for this *teaching transaction*. A real emulator's reset or game-launch policy will normally change more of the machine; treating RAM persistence here as universal hardware semantics would be a mistake. The central lesson is that we must **state** the policy and then test it, not that every console preserves RAM while switching cartridges.

Notice why the revision $G$ matters. An address may remain numerically identical after a vector reallocates or after a cartridge is replaced. A program address such as `0x8000` can also name different ROM content after a successful swap. The revision makes the application-level identity of the mounted image visible without exposing its native pointer address. It is not a cryptographic hash, not a persistent content ID, and not sufficient for concurrency safety on its own. A finite revision must also have an overflow policy; our reference rejects advancement at `std::numeric_limits<std::uint64_t>::max()` rather than wrapping to a previously used identity.

#### Which types should be simple records, and which should defend invariants?

The chapter's earlier `Vertex` examples can legitimately be aggregates because the client is allowed to construct the public components directly. A parser result like `Layout` can also be a plain record: it describes field values already validated by a separate operation. But a mounted cartridge has a stronger invariant. Its PRG storage must contain **exactly 16 or 32 KiB** for the NROM model, while its CHR storage must contain **exactly 8 KiB** of owned bytes. For a CHR-RAM cartridge, those bytes are *created by the emulator*, not read from the file. The mapper must be supported, and no live cartridge is allowed to contain an incomplete payload.

Allowing arbitrary callers to aggregate-initialize such an object would bypass these requirements. Instead, the existing `NromCartridge::from_image` factory checks the NES-01 parsed layout, checks the mapper and bank counts, makes independent vectors containing the approved PRG and CHR bytes, and then returns an owning object. Its constructor is private so callers cannot invent a supposedly valid NROM by setting a few fields. The factory may return a typed `CartridgeError` for an unsupported or invalid format, while a memory allocation can throw `std::bad_alloc`. These are different error channels and we will unify them deliberately at the runtime boundary.

This leads to a general design decision that will recur in the game engine. Use plain records when the language should permit arbitrary valid combinations of their public fields; use a controlled constructor or factory when correctness depends on a relationship between fields that the compiler cannot enforce through their individual types. Merely putting the fields under `private:` is not sufficient: every public operation, including moves and assignment, must continue to protect the relationship.

#### Construction must finish before publication begins

A poor replacement algorithm looks like this:

```cpp
// WRONG for a multi-stage loader: the live cartridge disappears
// before the replacement has even passed file validation.
bus.remove_current_cartridge();
auto replacement = parse_and_allocate(file); // may fail or throw
bus.insert(std::move(replacement));
```

If parsing or allocation fails, the previously usable cartridge has already gone. The algorithm can make the exception handler look sophisticated and still violate the user's required observable state. A safer sequence is **prepare, validate, commit**. All fallible work occurs in temporary, privately owned state; only after it succeeds does the system publish a replacement through an operation that cannot fail under the concrete type's guarantees.

Our lab expresses the central transition in C++23:

```cpp
std::expected<std::uint64_t, LoadError>
replace(std::span<const Byte> external_file,
        TestFault test_fault = TestFault::none)
{
    if (generation_ == std::numeric_limits<std::uint64_t>::max())
        return std::unexpected(LoadError::generation_exhausted);

    try {
        // Everything so far is temporary: the running bus is unchanged.
        auto candidate = NromCartridge::from_image(external_file);
        if (!candidate)
            return std::unexpected(translate(candidate.error()));

        // Test hooks deliberately interrupt preparation for negative tests.
        if (test_fault == TestFault::after_staging)
            return std::unexpected(LoadError::injected_failure);

        // The NROM cartridge's concrete move construction is noexcept.
        const auto result = bus_.insert(std::move(*candidate));
        if (!result)
            return std::unexpected(LoadError::invalid_candidate);

        // Publication completed; advance the revision exactly once.
        return ++generation_;
    } catch (const std::bad_alloc&) {
        return std::unexpected(LoadError::allocation_failure);
    }
}
```

The excerpt above omits some test-hook branches; the full program is the runnable specification. The `std::span` borrows the external input for the duration of the call only. The constructed `NromCartridge` owns independent vectors. If preparation fails, that local candidate and its partially constructed vector resources are cleaned up by C++ unwinding and RAII. A successful `std::move` transfers the cartridge to the bus and leaves the candidate with a valid but unspecified resource state; the code never tries to reuse it as a mounted game.

The visible generation change occurs **after**, not before, a successful commit. If we incremented it earlier, a failed operation could announce a cartridge revision that the bus never actually contained. The order is not a stylistic preference; it is part of the contract's proof.

#### Exception-safety levels are properties of an operation

A *no-throw guarantee* says the operation cannot let exceptions escape. A *strong exception guarantee* says that if it fails, its externally visible state is unchanged. A *basic exception guarantee* says that if it fails, invariants are maintained and resources do not leak, but the previous values may have changed. No guarantee means even validity or cleanup may be compromised. These categories describe an operation and its observable context, not a mystical property a class acquires because it uses RAII.

RAII automatically releases the local candidate if an exception is thrown during staging. That alone gives us cleanup, **not** a strong guarantee for the running game. The strong guarantee comes from keeping the live bus untouched until staging is complete, followed by a commit that cannot fail.

Here is the catch that a learner must understand: `std::optional<T>::emplace` destroys any old contained value before constructing the new one. If construction then throws, the optional becomes empty. Therefore, replacing a mounted cartridge by blindly calling `cartridge_.emplace(args_that_might_throw)` does **not** generally preserve the old cartridge. The C++23 optional contract says so explicitly. In our narrow implementation the replacement cartridge has already been constructed and we pass it by move into the bus. The contained `NromCartridge` owns ordinary `std::vector<std::uint8_t>` members with their default allocators, and its move constructor is explicitly declared `noexcept`. We require `std::is_nothrow_move_constructible_v<NromCartridge>` at compile time and mark the bus's validated insertion as `noexcept`. Its previous cartridge can therefore be destroyed and the candidate installed without a throwing construction step. This argument must be revisited if the class acquires new members with potentially throwing moves or a different allocator strategy.

For a future generalized asset system, a safer architecture may stage a `std::unique_ptr` to an immutable resource bundle and publish the pointer by a non-throwing swap, or use copy-and-swap where appropriate. The exact technique depends on whether the resource type is copyable, how concurrent readers acquire ownership, and whether any commit-time destruction or device synchronization can fail. `std::move` by itself does not imply no-throw behavior, and a successful compilation does not certify a strong guarantee.

#### Separate the result channel from the exception boundary

`std::expected<T,E>` models a successful value or an expected typed error. It is excellent for format failures such as `unsupported_mapper`, because those conditions are normal possibilities when reading an external file. But a vector's memory allocation can throw even when the file's fields are valid. We choose an explicit policy: the outer `CartridgeRuntime::replace` catches `std::bad_alloc` during staging and translates it into `LoadError::allocation_failure`. That promise is limited: it does not swallow every possible exception, and it does not magically guarantee successful resource allocation. Our staged file sizes are bounded by the legacy one-byte bank counts and by NROM's accepted sizes, but a general engine should also implement configurable resource budgets.

Typed failures should be **distinguishable**. Invalid magic is not the same as an unsupported mapper. Unsupported mapper is not the same as a malformed CHR length. An injected test failure is not evidence of real operating-system memory exhaustion. The laboratory deliberately produces a synthetic `std::bad_alloc` after staging and catches it, precisely to test the exception-to-result boundary. We do not claim that this simulates allocator internals, fragmentation, memory pressure, or a production out-of-memory recovery path.

The code's error translation function switches explicitly on `CartridgeError` rather than collapsing all failures into one Boolean. In a larger system we would preserve nested context, including the file path, parser stage, declared counts and precise field offset, while being careful not to make error reporting itself an unbounded allocation path.

#### A snapshot is evidence, not a pointer into the old system

To verify a state guarantee, do not save a raw pointer to the currently mounted cartridge and inspect it after replacement. That pointer's lifetime may end during a successful swap; reading it would be undefined behavior, not a test. The lab instead records a small *value witness* containing cartridge revision, PRG bytes at `0x8000` and `0xFFFF`, a mirrored RAM byte at `0x1801`, and CHR bytes at both ends of the pattern table. A witness is a snapshot of values, not an ownership claim or a full-state checksum.

For each failure, the test compares the new witness to the one captured before the attempted load. It exercises malformed input, wrong mapper, unsupported PRG banks, unsupported CHR banks, a truncated file, injected failure before staging, injected failure after staging, and simulated `std::bad_alloc` after staging. In every case the old data and revision must agree. An additional loop repeats a failed staged load 128 times to detect accidental revision increments or state drift.

The positive case is equally important. We prepare a new two-bank NROM-256 image with known different bytes at `0x8000` and `0xC000`. After commit the bus must return the new data, expose its new CHR-ROM bytes, reject attempts to mutate that CHR-ROM, and advance the revision from 1 to 2. The explicitly retained internal RAM value must remain readable through a mirrored address. The original external byte vector can be cleared or destroyed without making the mounted cartridge's storage dangle.

These tests are **not** a substitute for checking the whole 32 KiB program region, all possible allocator outcomes, concurrent access, CPU reset vectors, mapper side effects, PPU frame rendering or audio. They prove named model behaviors with synthetic input and deterministic fault points. Strengthen them by generating many valid and invalid images, comparing complete byte regions against an independent oracle, and checking invariants before and after each transition.

#### Reconstruct the mechanism rather than copying the code

Begin with a deliberately unsafe loader that destroys the previous cartridge before parsing a replacement. Write down exactly which test will fail when a malformed image is supplied. Then refactor it to prepare an owned candidate locally. Make the candidate's invariant explicit and write a static assertion for nothrow move construction. Add a typed error for an unsupported mapper and a separate error for allocation failure. Insert a fault point after the candidate has been built but before it is published, and prove that the previous cartridge survives. Finally, change the candidate type so its move constructor *could throw*: without running invalid code, explain precisely why blindly using `optional::emplace` now threatens the old cartridge.

A thorough answer must also explain why copy-and-swap is not universal. Copying a 32 KiB ROM has a different cost and identity effect from transferring a move-only cartridge; swapping a GPU resource whose old frames are still executing is not equivalent to swapping a CPU-only vector. The C++ object model gives you the legal operations, while the application contract tells you when to execute them. The upcoming engine and GPU books will add queue ownership and retirement fences to this story rather than assuming a CPU-side swap means the GPU has stopped using old memory.

There is an additional diagnostic question: does `CartridgeRuntime` preserve its guarantee if the CPU bus is accessed on another thread while replacement executes? **No.** The current reference has no synchronization, so concurrent unsynchronized access is not authorized. The promise is deliberately for a single-threaded transaction. Real multi-threaded loaders require a publication mechanism that gives readers stable ownership and an ordered retirement protocol; generation counters alone do not eliminate data races. We state that limit rather than hiding it.

#### Full project evidence and precise scope

The preceding chapter's real [NROM bus](https://github.com/swayam8624/NESEmulator/blob/main/NESEmulator/Bus.cpp) provides the historical motivation, but the new code does not change that external emulator repository. It builds on our **cumulative teaching model**: one iNES parser, one NROM cartridge/bus core, then this transactional orchestrator. The original broad cartridge factory, byte ownership, mirroring and mapper tests remain in their respective labs. Future integration into the production KairoNES project must inspect the exact repository revision and establish an end-to-end build, boot-ROM trace and system behavior.

The relevant C++23 technical contracts are `std::optional::emplace` ([cppreference](https://en.cppreference.com/w/cpp/utility/optional/emplace) and C++23's [N4950 working draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf)), `std::expected` ([cppreference](https://en.cppreference.com/w/cpp/utility/expected)), and the language's [object lifetime](https://eel.is/c++draft/basic.life) and [exception handling](https://eel.is/c++draft/except) rules. The sources determine the language guarantees; our executable probes test only concrete examples. No language standard claims are inferred solely from the results of one compiler.

**Milestone acceptance:** the reader can derive the observable state relation, classify exception guarantees, create a fully owned NROM candidate without publishing it, explain the special requirements for a non-throwing commit, preserve the previous cartridge on expected and exceptional failures, test each negative branch, and trace both successful and failed revisions. The existing chapter content remains in order and is not summarized away. The NES-03 CPU implementation remains a later stage, after these foundational ownership invariants have a reliable executable witness.
