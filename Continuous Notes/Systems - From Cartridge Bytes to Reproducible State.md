# Systems: From Cartridge Bytes to a Reproducible Machine

A saved game state is not just a file, and a functioning emulator is not just a switch statement. The same intellectual task connects a bytecode interpreter, a GPU command stream, an operating-system file, and a remote game server: define an observable machine, preserve its state and ownership, and explain why a given sequence of bytes causes precisely the observed transition.

This new subject volume opens Phase 28, the original 2701–2800 interval, at the real boundary after graphics and engine implementation. It begins with M163–M164 (Parts 2701–2744): CPU/6502 emulation and NES cartridge/PPU/APU timing foundations. Later M165–M168 (OS, file systems, networking, cryptography and trusted low-level systems) belong to the same volume rather than to 56 disconnected notes. Original part numbers remain recorded in [[Supplementary/Foundations#Phase 28 M163–M164 — Parts 2701–2744 deterministic emulator state and device timing|the coverage ledger]]. The fully documented [[tools/labs/phase28_emulator_core_lab.cpp|CPU/bus reference]] is an executable teaching witness, not a complete NES emulator or a source audit of an existing NESEmulator repository.

## An executable byte acquires meaning only inside a machine

Imagine the user selecting a cartridge image. The file begins with identifying header bytes and some ROM banks. When the emulator is started, the CPU reads an instruction from the address indicated by its program counter. That byte has no meaning by itself: it becomes an instruction only under the chosen instruction-set specification and the device map that determines which memory owns the address. The state-transition view is

\[
S_{t+1}=T(S_t,input_t),
\]

where the architectural state may contain the accumulator A, index registers X and Y, stack pointer SP, program counter PC, processor flags P, CPU memory, device registers, mapper state, cycle counters, interrupt requests and controller state. Determinism means that *all machine-observable state* plus the same input sequence produces the same observable transition, under a fixed machine implementation. Saving A and PC while omitting the mapper's currently selected ROM bank does not preserve the machine state.

The Nintendo Entertainment System CPU is derived from the 6502 family. A, X, Y, SP and P are 8-bit registers, while PC is 16-bit. A fetch normally reads at PC and advances PC. Decode identifies the instruction and its addressing mode, execution reads or writes operands, updates registers and flags, and assigns timing. A straightforward interpreter can implement this with a switch. A dispatch table can map opcode values to handlers. A generated micro-operation interpreter can model individual bus reads and writes. The implementation style does not guarantee accuracy; the contract is that the resulting effects and timing match the specified CPU variant. In particular, a generic 6502 and the NES Ricoh 2A03 are not interchangeable for decimal arithmetic.

A reset vector is an excellent first test of whether we understand the bus. The CPU constructs PC from the two bytes visible at addresses $FFFC and $FFFD, low byte first. Those addresses are mapped by the cartridge. On an NROM cartridge with one 16-KiB PRG bank, the bank appears twice in CPU space from $8000 to $FFFF. The reset vector stored near the end of that bank can direct execution to $8000. A test that always hard-codes PC=$8000 could appear to work on a toy ROM while being architecturally wrong for a cartridge whose reset vector points elsewhere.

A stack pointer does not magically reference host language call frames. The 6502 stack lives on CPU page $0100–$01FF and grows downward as SP changes. Subroutine call JSR stores a return address using the emulated stack and jumps to the callee; RTS pulls the saved address and adds one to resume after the call. This differs from a typical high-level program's description of calling a function. Interrupt entry similarly saves a return context and jumps to an interrupt vector. The saved status byte must respect the 6502-specific break/unused-bit conventions; they are not simply independent physical flags that can be blindly copied.

### Addressing is an algorithm for finding a value, not a suffix

Immediate addressing obtains the operand directly from the instruction stream. Zero-page addressing takes an 8-bit address from the stream and reads at that address within $0000–$00FF. Absolute addressing combines two operand bytes into a 16-bit little-endian address. Zero-page-indexed addressing adds X or Y with 8-bit wraparound, while absolute-indexed modes add to a 16-bit base and may cross a 256-byte page. Indexed-indirect means the index is applied to a zero-page pointer location *before* fetching the target address. Indirect-indexed obtains the pointer first and adds the index to the fetched address. Confusing these yields a program that may execute correctly for one set of values and fail as soon as a pointer crosses a page.

Relative branches interpret an 8-bit displacement as signed. The displacement is added to PC *after* fetching the branch operand. A taken conditional branch normally adds a cycle over the two-cycle not-taken path, and crossing a 256-byte page adds another. Returning the correct resulting PC is not the whole execution contract when an instruction interacts with a device that changes every cycle. For each opcode, an implementation should record its instruction bytes, addressing mode, conditional cycle penalties, bus reads/writes and changed status bits. A complete cycle-accurate emulator also needs correct ordering of the individual memory accesses; a total-cycle counter is useful but is not sufficient to reproduce all hardware behavior.

### Unsigned carry is not signed overflow

For ADC, let A be the accumulator's original 8-bit unsigned value, M the operand and C the incoming carry bit. Form the full integer sum \(u=A+M+C\) and the 8-bit result \(r=u\bmod256\). Set carry if \(u>255\), zero if \(r=0\), and negative from result bit 7. Signed overflow is a different condition: the sum of two same-sign two's-complement operands cannot be represented in the signed 8-bit range. One bitwise expression for overflow is

\[
V= ((\neg(A\oplus M))\mathbin{\&}(A\oplus r)\mathbin{\&}0x80)\ne0.
\]

A=$7F, M=$01, C=0 produces $80: signed overflow V=1, N=1, and unsigned carry C=0. By contrast $FF+$01 produces $00 with C=1 and Z=1 but V=0. These two independently testable cases prove why a CPU interpreter must not use one generic overflow boolean for both flags. The NES 2A03 does not perform BCD decimal ADC even if the decimal-status bit is set. An apparently minor misunderstanding here breaks arithmetic, conditional branches and some game logic.

The CPU status also includes interrupt disable I and decimal D; B and the always-set status bit have nuanced behavior when a byte is pushed or pulled. Branches read flags generated by earlier instructions. Therefore a bug in a flag update can remain latent until a much later branch, making deterministic instruction traces invaluable. A useful trace row contains old PC, opcode and operands, old/new registers, old/new P, bus side effects and consumed cycles, not just the mnemonic.

## The memory bus is a device router, not one enormous RAM array

An NES CPU address is a request to the bus, and the address decoder decides which hardware responds. The 2 KiB internal RAM occupies $0000–$07FF, mirrored through $1FFF. CPU addresses $2000–$2007 correspond to PPU registers, mirrored across $2008–$3FFF. Selected APU and controller registers occur around $4000–$4017, while cartridge-controlled space usually includes $4020–$FFFF and, on common NROM boards, PRG ROM from $8000 upward.

This matters because a read can have effects. Reading a controller register can advance its shift position. Reading PPU status can clear or otherwise affect status/latch state under hardware timing rules. Writing a mapper register can change what later CPU ROM reads mean at the same address. Writing $4014 requests a DMA transfer into PPU sprite OAM; an actual transfer and CPU stall must obey the hardware cycle schedule. Representing every device register as a plain mutable byte array cannot reproduce those observable behaviors.

The reference CPU bus intentionally implements only 2-KiB RAM mirroring, NROM PRG read mapping, a controller serial interface and a simplified OAM copy. Reads from the unimplemented PPU registers return a placeholder; mapper/APU/open-bus behavior is not simulated. This is an explicit boundary, not an accidental omission hidden behind a broad claim of NES support. The CPU code increments total CPU cycles and a notional three PPU dots per CPU cycle. Such a counter can help explain relative timebases, but it cannot itself model device events at each dot.

A controller is itself a small state machine. With strobe asserted, reads return the current first-button state. After the appropriate strobe transition, reads return successive button bits, with defined behavior after the eighth read. An emulator that samples host keyboard state on every CPU read rather than latching at the prescribed event can produce non-deterministic playback even when the recorded button presses were identical.

### An iNES file describes cartridge memory; it is not executable code itself

The iNES header occupies 16 bytes and begins with the bytes $4E $45 $53 $1A. A classic header records PRG ROM size in 16-KiB units, CHR ROM size in 8-KiB units and flags encoding mapper number, mirroring and optional features. A 16-KiB PRG NROM bank mirrors in CPU space; a 32-KiB bank maps directly. CHR size zero commonly selects CHR RAM, not necessarily an error. The mapper determines how cartridge circuitry maps CPU and PPU addresses, including bank switching. NROM or mapper 0 is only the simplest case; a real emulator cannot accept every mapper value and treat it as NROM.

A robust parser first validates the header and supported format, then performs checked size/offset arithmetic before copying or mapping any data. If a trainer is present, its 512 bytes shift subsequent bank offsets. NES 2.0 headers encode some metadata differently from iNES 1.0, and treating one as the other can misinterpret bank size or mapper IDs. The lab deliberately rejects NES 2.0, nonzero mappers and trainers instead of guessing; it supports one or two PRG banks and zero or one CHR bank. That behavior is far preferable to silently loading a malformed image whose pointer arithmetic could exceed the file.

A parser should also distinguish content validation from ROM compatibility. Passing its supported-format checks proves that the byte layout can be interpreted as the selected subset, not that the ROM will execute under the few opcodes the test interpreter understands. The executable test uses a tiny synthetic PRG, sets its reset vector and confirms that $C000 aliases $8000 for a one-bank NROM fixture.

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
