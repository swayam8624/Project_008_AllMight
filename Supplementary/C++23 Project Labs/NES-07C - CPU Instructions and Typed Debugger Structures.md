### NES-07C — From a typed asset to a CPU instruction and a debugger that earns its data structures

Until this point the growing NES project has answered how untrusted cartridge bytes become a validated layout, how PRG and CHR memory become owned resources, how a CPU address reaches the appropriate NROM bank, how replacement avoids publishing an incomplete cartridge, and how an editor can give its resources typed, generation-aware identities. The surrounding chapter already develops PIMPL, phantom types, mathematical dimensions, expression templates, intrusive ownership, CPU pipelines, cache locality, SIMD, allocation, threads and asymptotic data structures. We retain that existing explanation rather than turning it into another list of terminology. Now we give those ideas a single concrete purpose: **the computer must actually execute bytes from the cartridge, and the debugger must keep its observations correct while the machine changes.**

A loaded ROM is not a running program. In the NROM cartridge we constructed, `0x8000` is a *CPU-visible address*, not an index into the source file. When the CPU requests a byte at that address, the bus checks its address map and asks the cartridge for the corresponding PRG byte. The CPU then interprets the byte as an opcode, fetches any additional operand bytes, reads or writes memory through the same bus, updates registers, and chooses the next program counter. This is the point at which the earlier lessons on representation, lifetime and typed interfaces become an executable computer-system model.

The new laboratory lives in four coordinated source units. `tools/labs/nes_course/cpu6502_subset.hpp` contains a strictly bounded CPU instruction executor. `cpu_debug_instruments.hpp` contains the trace ring, exact breakpoints, probabilistic visitation filter, inspection heap and disjoint-set analysis. `cpu_debug_session.hpp` declares a deliberately small public interface; its real `cpu_debug_session.cpp` implementation defines the hidden private state in a separately compiled translation unit. `tools/labs/nes_cpu_debugger_cpp23_lab.cpp` tests the combined system using synthetic iNES images. All read from the **same `CpuBus` and `NromCartridge` types** introduced by NES-01 and NES-02, not independently invented emulators.

**Scope first:** this is a *6502 instruction-subset executor with an instruction cost counter*, not a complete NES 2A03 core and not a cycle-accurate emulator. It supports a documented set of eleven opcode encodings and their selected addressing modes. It does not yet implement all legal instructions, unofficial opcodes, stack instructions, interrupts, DMA, PPU/APU I/O, controller reads, bus-cycle side effects, reset micro-operations, video timing, or frame-level synchronization. The trace's numerical cycle costs match the modeled instruction cases, but the implementation does not perform every individual bus action that the real processor performs during those cycles. That difference will be carried forward explicitly rather than erased by the fact that an instruction test passes.

#### There are three different addresses in the very first instruction

Begin with a cartridge image on disk. The first sixteen bytes are the legacy iNES header. After any optional trainer comes PRG-ROM. The emulator's host-side vector therefore has an index measured from the beginning of the file or from the beginning of its extracted PRG payload. But the running NES CPU issues a *sixteen-bit machine address*. NROM maps some of these CPU addresses into its PRG-ROM banks.

For a one-bank, 16 KiB NROM-128 cartridge, the PRG mapping for CPU addresses $a$ in `0x8000` through `0xFFFF` is

$$
i_\mathrm{PRG}=(a-0x8000)\bmod 0x4000.
$$

The modulo duplicates the same 16 KiB program in both halves of that 32 KiB CPU window. For a two-bank, 32 KiB NROM-256 cartridge the modulo uses `0x8000` instead, and the two halves contain distinct program bytes. This is not a claim about virtual memory page tables or the host process's C++ address space; it is the emulated machine's bus mapping.

The CPU's **program counter** is yet another meaningful object. It is not the pointer to a host array and not the parsed cartridge's storage offset. Our `ProgramCounter` contains a sixteen-bit value with an explicit constructor so the code cannot casually treat every other integer as a valid PC object without a deliberate conversion. The extra type does not magically validate that the address points at an implemented bus region. That is still checked on fetch; the strong type means callers must state which *kind of quantity* they are providing.

Now consider reset. The program does not simply begin at host offset zero. The reset vector lives at CPU addresses `0xFFFC` and `0xFFFD`, holding a little-endian address. For NROM-128, those two CPU addresses map to PRG offsets `0x3FFC` and `0x3FFD`. A synthetic test image can place the bytes `00 80` there, and the CPU's next instruction will be fetched at `0x8000`. The code explicitly reads the vector **through the CPU bus**, so missing cartridges and unimplemented bus regions remain typed failures rather than being disguised as an all-zero reset vector.

The CPU wrapper resets its register state to one documented teaching state, with accumulator and index registers zero, stack pointer `0xFD` and status `0x24`. This reset method does **not** simulate every hardware reset cycle. The actual 2A03's power-on/reset details differ and must be researched independently; a classroom initial state is not evidence that all electrical reset behavior is implemented. The instruction executor also does not clear the bus's RAM when resetting its own registers, because CPU architectural reset and the memory device's initialization policy are different subsystems.

#### One step must be an architectural state transition

Let the modeled CPU state be

$$
S=(A,X,Y,SP,P,PC,C),
$$

with eight-bit accumulator $A$, index registers $X$ and $Y$, stack pointer $SP$, status flags $P$, sixteen-bit program counter $PC$ and accumulated instruction cost counter $C$. A successful instruction step receives the current bus state and returns a new register state, a possibly updated memory device, and a trace. An unsupported opcode should not silently advance the PC or spend cycles as though the instruction had executed.

Our implementation therefore stages register changes in a local copy named `next`. It fetches the opcode and operands first, computes the next PC and flags in that temporary record, and checks whether adding the modeled instruction cost would overflow the diagnostic counter. Only after all those steps succeed does it perform the approved CPU RAM write, if any, and publish the new register state and total cost.

This is a *bounded architectural transaction*. It does not claim a general exception-atomic instruction execution for an arbitrary bus with callbacks or partial memory-mapped I/O side effects. The only modeled writing opcode is zero-page `STA`, targeting the NES bus's internal RAM where the current `write` implementation completes without allocation or asynchronous device effects. As later chapters add I/O devices, PPU register writes, interrupts and DMA, the real bus may produce partial externally visible effects and the transaction contract must be reconsidered. Real hardware performs intermediate bus cycles that cannot always be rolled back. We do not confuse our debugger-facing state guarantee with a hardware claim.

The supported instruction encodings are deliberately small enough to derive and independently test:

| Opcode | Modeled instruction | Base cost | Observable responsibility |
| --- | --- | ---: | --- |
| `EA` | NOP | 2 | Advance PC without changing registers |
| `A9` | LDA immediate | 2 | Load accumulator; recompute Z and N |
| `A2` | LDX immediate | 2 | Load X; recompute Z and N |
| `85` | STA zero page | 3 | Write A through CPU RAM bus |
| `E8` | INX | 2 | Increment X modulo 256; recompute Z and N |
| `18` / `38` | CLC / SEC | 2 | Clear or set carry; preserve unrelated status |
| `69` | ADC immediate | 2 | Binary add with input carry; update N,V,Z,C |
| `D0` / `F0` | BNE / BEQ relative | 2–4 | Conditionally add signed displacement and page penalty |
| `4C` | JMP absolute | 3 | Read little-endian target and set PC |

These are **eleven byte encodings**, not eleven independent full instruction families with every addressing mode. Some instruction names have many other legal modes, and the complete processor has substantially more behavior. For example, `STA absolute` and `LDA indexed` are not modeled. Encountering `BRK` (`00`) currently returns `unsupported_opcode`. It would be false to treat BRK as a convenient *halt* instruction: real BRK transfers control through the interrupt protocol and interacts with the stack and status flags. We will add it only when those dependencies have been taught and implemented correctly.

#### The arithmetic flags are a small proof with a real CPU consequence

A status register is not one vague “result okay” Boolean. Selected bits record different interpretations of the previous operation. The zero flag $Z$ indicates that the relevant eight-bit result is zero. The negative flag $N$ reflects bit seven of that result, not a native C++ signed comparison done in a wider type. The carry flag $C$ represents an unsigned carry out of the eight-bit addition, while the overflow flag $V$ indicates a signed two's-complement overflow condition. The same result may set $V$ without $C$, or $C$ without $V$.

For binary-mode `ADC #imm`, let $a$ and $b$ be eight-bit unsigned operands and $c\in\{0,1\}$ be the incoming carry. Compute the sum in a wider unsigned type:

$$
s=a+b+c,\qquad r=s\bmod 256.
$$

The carry flag is set exactly when $s>255$, and the zero and negative flags derive from $r$. Signed overflow occurs when the operands have the same sign bit but the result has a different sign bit. One bitwise expression for that property is

$$
V=\Bigl(\neg(a\oplus b)\;\&\;(a\oplus r)\;\&\;0x80\Bigr)\ne0.
$$

For $a=3$, $b=127$, $c=0$, the result is 130 or `0x82`. It is negative when interpreted as signed eight-bit, even though two nonnegative operands were added; therefore $N=1$, $V=1$, but $C=0$. By contrast $a=255$, $b=0$, $c=1$ produces $s=256$ and $r=0$, so $C=1$ and $Z=1$ while $V=0$. The test checks both examples and makes it impossible to replace the CPU's flags with a single “overflow” predicate without failing.

This model uses NES-style **binary arithmetic**. The NES 2A03 does not implement BCD decimal arithmetic for `ADC`/`SBC`, unlike the generic MOS 6502's decimal behavior. The code does not currently implement `SED`, `CLD` or the other instructions needed to explore decimal-mode semantics; we should not claim to test a feature that cannot be reached. The key source reference is the NESdev [instruction reference](https://www.nesdev.org/wiki/Instruction_reference), whose opcode tables and documented 2A03 decimal distinction should be consulted when adding the remaining instructions.

Why use `std::uint8_t` for registers but a wider `unsigned` for the addition? The register type represents the actual eight-bit architectural storage, while arithmetic in C++ follows integral promotions. Computing the sum as a wider unsigned integer preserves the carry-out information that would be lost if we truncated too early. The explicit final conversion to `uint8_t` gives the defined modulo-256 result. This is another point at which representation is a *chosen contract* rather than a random assortment of integers.

#### Relative branches require signed interpretation and an independent page-crossing test

A branch instruction's operand is one byte, but the byte is interpreted as a **signed displacement** from the PC immediately after both the opcode and operand bytes. If `BNE` is located at `0x8000` and its operand is `FE`, the offset is $-2$ in signed eight-bit form. The PC after fetching the branch is `0x8002`; adding $-2$ returns to `0x8000`. A debugger that simply adds the encoded byte as a positive 254 would jump to an unrelated location.

The code uses `std::bit_cast<std::int8_t>` on the byte representation, then widens the signed displacement before computing the final sixteen-bit address. This both makes the signed interpretation explicit and avoids relying on an implementation-defined intuition about how a large unsigned value converts into a small signed type. Conversion back to sixteen-bit unsigned is defined modulo $2^{16}$, matching the PC's address width.

Branch cost is another independent rule. An untaken conditional branch costs two modeled CPU cycles. A taken branch costs a third, and crossing a 256-byte page boundary costs a fourth. Compare the high eight bits of the post-operand PC with those of the target:

$$
\text{page crossing} \iff
(PC_{\mathrm{after\ operand}}\;\&\;0xFF00)
\ne
(PC_{\mathrm{target}}\;\&\;0xFF00).
$$

Our synthetic test puts `LDA #0` at `0x80FB`, then `BEQ +1` at `0x80FD`. After the branch operand, the PC is `0x80FF`. The taken branch lands at `0x8100`, crossing from page `0x80` to `0x81`, so the trace records a cost of four. A separate test covers a negative displacement, and another checks the untaken branch cost and sequential PC. The CPU instruction reference provides the intended cycle-cost oracle, but our counter is **not** the same as executing exact hardware memory cycles or modeling DMA stalls.

#### PIMPL earns its complexity at the editor-facing debugger boundary

The chapter's earlier service design explains that C++ `private` members still appear in a class definition and therefore contribute to its representation and compilation dependencies. Instead of repeating that abstract claim, we build a genuine two-translation-unit interface:

```cpp
// Public header, used by the editor and the test executable.
class CpuDebugSession {
public:
    explicit CpuDebugSession(CpuBus& borrowed_bus);
    ~CpuDebugSession();

    CpuDebugSession(const CpuDebugSession&) = delete;
    CpuDebugSession& operator=(const CpuDebugSession&) = delete;
    CpuDebugSession(CpuDebugSession&&) noexcept;
    CpuDebugSession& operator=(CpuDebugSession&&) noexcept;

    std::expected<StepSnapshot, SessionError> step() noexcept;
    std::optional<CpuStateSnapshot> snapshot() const noexcept;
    // Other carefully defined debugger operations...

private:
    struct Impl;                  // Incomplete in this header.
    std::unique_ptr<Impl> impl_;   // Exclusive ownership of the hidden state.
};
```

The public header declares only small owned snapshots, typed errors and the incomplete implementation type. The implementation file defines `Impl` with the actual `TracedCpu`, its ring, breakpoints and Bloom filter. The destructor and move assignment are defined **after** the full implementation type is visible, so `unique_ptr`'s deleter does not need to instantiate destruction of an incomplete type in arbitrary client source files. The executable links the test source and the separate implementation file, proving that this is a real compilation boundary rather than one header stuffed with an “Impl” comment.

The debugger's public `snapshot` returns an **owned value** of the register state, not a reference into a private interpreter. Such a snapshot can be retained after the debugger changes or dies without becoming a dangling pointer. It may become stale relative to a later CPU step, but stale data and invalid memory are different problems. A frontend should label the sample's trace time or cycle count rather than quietly presenting it as the machine's current state forever.

The session owns its private implementation but **borrows the CPU bus**. A CPU that outlives its bus is still invalid even though its private `unique_ptr` is perfectly managed. PIMPL solves compilation exposure and implementation lifetime, not external referent lifetime. The public API therefore documents that the bus must outlive the session. The session is move-only. Move construction or assignment transfers the hidden state, and calls made through a moved-from session return a typed error or empty optional rather than dereferencing a null `impl_`. Copying is forbidden because duplicating one debugger's mutable trace and interpreter identity would need a deliberate cloning contract.

PIMPL has real costs: one owned allocation for the implementation, an extra pointer indirection and possibly less opportunity to inline cross-unit calls without link-time optimization. None of those automatically makes it inappropriate. The service is relatively coarse-grained; it can afford an indirection when an editor requests a step or snapshot. It would be unreasonable to put an independently allocated PIMPL in every vertex of a mesh. We must measure actual call frequency and target machine code before claiming one form is faster. And this C++23 header still does not promise a stable binary interface across unrelated compilers or standard-library versions; a plugin C ABI would require its own explicit types, versioning and release protocol.

#### The ring buffer is a bounded temporal contract

An instruction debugger needs to remember the order in which instructions ran. That is a FIFO problem, not a sorted-map problem. The trace buffer uses a fixed array of sixteen optional instruction-trace values, a head position $h$, a live count $n$ and capacity $C=16$. Its next append slot is

$$
t=(h+n)\bmod C.
$$

The invariant is $0\le n\le C$. When $n=0$, the queue is empty; when $n=C$, it is full. Without that count (or another explicit full-state representation), equal head and tail positions can mean either full or empty. Our implementation uses a count and checks the boundary before overwriting any old record.

This detail changes CPU correctness. If the debugger executed the CPU instruction first and only then discovered the trace ring was full, it would have advanced the machine without recording the step, violating a promised *lossless trace* mode. The debugger therefore checks queue capacity **before** calling the CPU. A full ring returns `trace_full`, leaving the PC, flags, memory and cycle counter unchanged. The test executes sixteen NOPs, attempts a seventeenth, verifies no CPU advance, drains seven records, executes seven more and then verifies FIFO order after the ring wraps. These are state-transition tests, not performance benchmarks.

The ring's elements are small trivially copyable instruction summaries with non-throwing moves. No heap allocation occurs in its push/pop path. The implementation is intentionally single-threaded. A lock-free ring would need atomic head/tail publication, a memory-order protocol and lifetime conditions for producers and consumers. An array modulo operation by itself does not provide synchronization or protect against data races.

A useful design question follows from backpressure: should the emulator freeze when the debugger's ring fills, drop the oldest trace, grow the buffer, or ask the UI to drain it? Different applications can choose different policies. Our current **lossless fixed-capacity** policy is intentionally strict and easy to test. A production high-throughput trace recorder could choose another policy, but it must explicitly describe the information that would be lost and the timing overhead it imposes.

#### A sorted breakpoint set is not a priority queue

A debugger commonly asks two separate questions. “Is the current PC a breakpoint?” is exact membership. “What is the next configured breakpoint at or above address X?” is an ordered range query. We use a `std::set<std::uint16_t>` for these operations. It stores unique ordered addresses, supports exact membership with `contains` and answers the latter question using `lower_bound`. A hash set might offer good expected membership cost but would not directly answer the ordered query without additional work. A flat sorted vector might be ideal for a nearly immutable small breakpoint list, but editing would involve shifting entries. The correct structure follows the workload, not a container popularity contest.

The breakpoint is checked **before** executing the instruction at its PC. If a breakpoint at `0x800F` is active, the CPU stops with PC still `0x800F` and no newly consumed cycles. Removing the breakpoint permits the pending instruction to run. This is part of the public behavior: a frontend must not need to guess whether a reported breakpoint fired before or after the instruction executed.

Memory probes answer a different question again: which requested memory inspection should be serviced first? For that purpose the lab uses a binary-heap-backed `std::priority_queue`. A probe carries an address, an integer priority and a monotonically assigned submission order. Higher priority is processed first; when priorities tie, the older request is processed first. The tie break matters because a plain priority queue does **not** guarantee FIFO order among equal-priority elements merely because its code happens to use a heap.

The test submits two high-priority probes of the synthetic ROM, one medium-priority probe of an unimplemented controller address and one low-priority probe of RAM. It verifies the exact extraction order, then checks that the controller read becomes an explicit `BusError::unimplemented_device` rather than an invented byte value. The queue schedules **inspection requests**, not CPU execution cycles. The test therefore makes no claim that the heap changes CPU hardware instruction order. Further, this scheduler may allocate in its underlying vector when adding requests; it is not a fixed-allocation real-time queue or a hard worst-case-latency guarantee.

#### A Bloom filter can rule out visits but cannot prove one

The debugger may need to ask whether it has ever executed an instruction at a given PC during the current trace epoch. If it stored every PC in an exact ordered set, queries would be definitive but memory would grow with the number of distinct addresses. If it needs only a cheap **negative prefilter**, it can use a Bloom filter: a fixed bit array and multiple hash-derived bit positions.

For each actually executed PC, our filter sets two bit positions. A query returns true only if both corresponding bits are set. Therefore, for an inserted PC, the result cannot be false while the filter is maintained correctly and has not been cleared. A PC that was never inserted can still return true, because other PCs may have set the same bits. This is a **false positive**, not an implementation bug. The meanings are:

```text
possibly_visited(PC) == false  -> definitely not recorded in this epoch
possibly_visited(PC) == true   -> perhaps recorded; check exact history if needed
```

The bit array holds 2,048 bits and uses two deterministic hash formulas. This is a demonstration, not a claim that these particular hash functions produce an optimal false-positive distribution for arbitrary programs. Under the conventional idealized independent-uniform hashing assumptions, a Bloom filter with $m$ bits, $k$ hashes and $n$ inserted keys has approximate false-positive probability

$$
p\approx(1-e^{-kn/m})^k.
$$

Real instruction addresses may be clustered, hashes may be correlated, and an approximation under a random-hash model is not a measured false-positive rate. This particular implementation has no deletion support, and resetting the debugger clears the epoch. Its public API returns `possibly_visited`, deliberately avoiding a misleading name such as `is_certainly_visited`.

We also distinguish a visited-PC filter from an executable-code coverage report. The instruction subset cannot execute every legal opcode and the system does not model interrupts or dynamic self-modifying code, so “visited” means only “the debugger committed a modeled instruction at this address in the current epoch.” It does not establish full-game reachability or formal control-flow coverage. A coverage tool needs an exact oracle and an instruction model appropriate to its claims.

#### A disjoint-set structure is appropriate for offline equivalence, not for every RAM access

Our NES CPU bus mirrors its physical 2 KiB RAM four times across CPU addresses `0x0000` through `0x1FFF`. For instance `0x0012`, `0x0812`, `0x1012` and `0x1812` all identify the same physical RAM byte. The existing bus expresses this mapping directly with `address & 0x07FF`, a cheap calculation that is appropriate in the hot instruction path.

But suppose the debugger is analyzing a collection of observed addresses from several sources and wants to group those known to refer to the same underlying storage. That is a separate **equivalence-class** problem. The disjoint-set union (DSU) structure represents a partition of the observed addresses. `join(a,b)` merges two equivalence classes; `find(a)` returns a representative of its class; `connected(a,b)` tells us whether they share a representative. Path compression and union by rank keep trees shallow over repeated operations.

The new test builds eight addresses in two groups of four. It joins pairs only when the **existing real bus decoder** maps them to the same RAM index. It then checks every pair against that independent decoder, rather than declaring success because two DSU queries happen to agree with each other. Addresses with offsets `0x0012` and `0x0034` must remain in different sets; each offset's four mirrors must be connected. This is a direct example of selecting a data structure according to the required *query*, not inserting DSU into the CPU's critical path for no reason.

The conventional amortized complexity of union-find with union by rank/size and path compression is $O(\alpha(n))$ per operation over a sequence, with $\alpha$ the inverse Ackermann function. That is a mathematical bound under the algorithm's model, not a claim that DSU is faster than one bitmask instruction for the NES RAM decoder. We keep the O(1) mask in the bus and use DSU only in offline analysis of equivalence among known addresses.

The same abstraction will reappear later when geometry and game-engine systems ask for connected components, mesh island identification, mergeable collision groups or offline alias analysis. Each of those applications needs its own rules for when two objects should be joined; sharing the DSU implementation does not make those domain-specific equivalence decisions correct automatically.

#### Follow one nine-instruction program all the way through

The executable's main synthetic program is deliberately simple enough to reason about without an emulator UI. The reset vector points to `0x8000`. The following byte sequence is placed into the synthetic NROM PRG image, with a couple of unsupported BRK bytes deliberately positioned on a branch path that should be skipped:

```asm
8000: A9 03       LDA #$03
8002: 85 02       STA $02
8004: A2 00       LDX #$00
8006: E8          INX
8007: D0 02       BNE $800B
8009: 00          ; BRK deliberately unreachable in this test
800A: 00          ; BRK deliberately unreachable in this test
800B: 18          CLC
800C: 69 7F       ADC #$7F
800E: EA          NOP
800F: 4C 00 80    JMP $8000
```

The first instruction sets $A=3$. `STA` writes 3 to zero-page address `0x0002`, and the test also reads that value through the mirrored address `0x1802` to exercise the previously developed bus model. `LDX #0` clears X; `INX` increments it to 1 and updates the zero flag, so `BNE` is taken and skips the two BRK bytes. `CLC` clears the carry flag. `ADC #$7F` produces `0x82` and sets the negative and signed-overflow flags. `NOP` advances the PC. A breakpoint at `0x800F` stops execution before the final `JMP`; removing it allows the jump to return to the program's beginning.

The modeled instruction costs are $2+3+2+2+3+2+2+2+3=21$ cycles for the nine successfully executed instructions. The test checks every intermediate PC, opcode, per-instruction cost and cumulative cost instead of checking only the final value. It also checks that the trace ring contains exactly one record for each committed step and that popping those records follows execution order.

Separate fixtures test signed negative branch offsets, page-crossing branches, untaken branch costs, signed-overflow versus unsigned-carry behavior, wraparound of X from 255 to zero, and explicit refusal to treat BRK as HALT. Another fixture fills the sixteen-entry trace ring, verifies the seventeenth step is refused before CPU state changes, then tests FIFO order after buffer wraparound. A PIMPL fixture move-constructs and move-assigns the public session and demonstrates that the moved-from object returns a safe typed error.

All of these tests use synthetic program images, not commercial game ROMs. Their value is that the expected trace can be derived by hand and checked against a separate instruction reference. They are **not** an independent validation against a full 6502 instruction-test suite or the user's actual desktop NES emulator, and they do not prove native macOS behavior until compiled and executed on that toolchain.

#### The measured cost story still needs measurements

We have now accumulated multiple representations with different theoretical costs. The CPU bus's RAM mirror calculation is constant work using bit masking. Opcode dispatch is a closed switch. The ring buffer's push/pop does constant work given fixed capacity. The ordered breakpoint tree has logarithmic membership and lower-bound complexity in the number of breakpoints. A binary heap has logarithmic insertion and removal in the number of pending probes. The Bloom query checks a constant number of bit positions. The DSU has extremely slowly growing amortized cost under its stated algorithms. None of these asymptotic statements tells us which operation dominates a frame or whether the CPU is limited by cache misses, branch mispredictions, memory allocation or instruction decode.

The interpreter itself is currently a deliberately unoptimized correctness model. Before optimizing it, collect repeated traces of deterministic synthetic programs, separate execution time from debugger instrumentation time, and report compiler versions, optimization flags, sample sizes, variance and the machine's memory hierarchy. Compare the interpreter with and without its debugger, then inspect generated machine code before proposing that a function is “free” or that a branchless rewrite is faster. Dynamic debugging allocations must not be silently included in a claim about interpreter throughput.

An important distinction from the original chapter's SIMD, threading and GPU sections is that parallelism does not immediately make sense for the individual instruction stream of one mutable 6502 state. Each instruction depends on the previous state, including flags and program counter. Independent test ROMs, traces or offline analysis tasks can run in parallel; simultaneous unsynchronized calls to `step` on the same mutable CPU cannot. A future job system may process different emulator instances or analysis tasks concurrently, but that requires ownership and synchronization boundaries. We must not add threads merely because the book has reached the concurrency chapter.

Likewise, a CPU instruction-cost counter is not a physical performance timer. It models selected 6502 costs, whereas the host machine executes many instructions to interpret them. A modern host CPU's pipelining, caches, branch prediction and SIMD capabilities affect **host execution time**, not the historical CPU's architectural cycle definitions. Keeping those two meanings of “CPU cycle” separate is necessary for accurate emulator and hardware education.

#### Regression protocol and explicit limitations

The new CMake target compiles the public debugger test and `cpu_debug_session.cpp` as separate translation units under C++23. It is registered alongside all six earlier cumulative NES exercises. The broad test protocol checks actual NROM reset-vector decoding, typed CPU state, supported opcode behavior, bus-visible writes, flags, branch offsets and costs, unsupported-opcode invariants, capacity backpressure, correct public PIMPL move semantics, exact breakpoint lookups, typed failures from unimplemented memory-mapped devices, priority order, Bloom no-false-negative membership for inserted PCs and DSU agreement with the bus's RAM alias calculation.

There is no intentional dereference of a dangling pointer, no invented hardware PPU output, no audio, and no artificial “completed ROM” claim. The implementation is a bounded teaching model to be extended incrementally. A full 2A03 emulator requires a substantially more complete opcode/addressing-mode matrix, status and stack correctness, IRQ/NMI/BRK/RTI semantics, mapped PPU/APU devices, timing coupling, direct-memory-access behavior, open-bus semantics and hardware-reference differential tests. Those omissions are named so that a learner does not mistake scope for completion.

To turn this chapter into an even stronger research artifact, make the trace machine-readable and compare it with an independent known-correct emulator on programs entirely within our supported instruction subset. Then add one instruction family at a time with a specification-derived oracle, negative tests for unsupported addressing modes, documented CPU-state transitions and independent randomized fixtures. Only after the CPU and bus contracts stabilize should we add a visual frame/debugger interface and evaluate speedups; a polished interface cannot establish the machine's correctness.

For the four main conceptual boundaries, use [NESdev's instruction reference](https://www.nesdev.org/wiki/Instruction_reference), [NESdev's CPU power-up state](https://www.nesdev.org/wiki/CPU_power_up_state), [NESdev's interrupt documentation](https://www.nesdev.org/wiki/Interrupts), and the standard-library definitions of [std::unique_ptr and incomplete types](https://en.cppreference.com/w/cpp/memory/unique_ptr), [std::priority_queue](https://en.cppreference.com/w/cpp/container/priority_queue), [std::set](https://en.cppreference.com/w/cpp/container/set) and [std::bitset](https://en.cppreference.com/w/cpp/utility/bitset). The CPU reference supplies hardware-oriented instruction rules; the C++ references supply language/library rules; neither alone proves our custom code correct.

**Milestone completion question.** Can the learner explain which three address domains are involved in fetching `0xA9` at `0x8000`, derive the little-endian reset vector, trace the nine instructions and 21 modeled cycles without running the program, explain why one sign overflow is not a carry overflow, prove why a full trace ring must reject a step before executing it, explain why a Bloom hit cannot prove visitation, explain why union-find is useful for offline RAM aliases but unnecessary in the bus, and justify why PIMPL returns owned state snapshots rather than retained references into its hidden vector? The new code makes each question falsifiable. The original chapter's deep CPU, performance and data-structure material remains the authoritative surrounding narrative.

**Next existing section:** “The scene is now too large for the wrong container.” We now carry real instruction traces, handles and bounded debugger events into the original DSA and complexity lessons. The next chapter moves to graphs and scheduling, where dependencies between tasks, resources and emulator events will require explicit graph operations rather than a random new container chosen in advance.
