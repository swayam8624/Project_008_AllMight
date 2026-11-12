# Phase 28 — From Cartridge Bytes to OS, Network and Cryptographic Contracts

This chapter repairs the weakest side of Phase 28. The emulator core already has an executable witness, but operating-system boundaries, network framing and cryptographic composition were much thinner than the rest of PROJECTALLMIGHT. The goal here is one continuous systems story: externally supplied bytes enter a process, are interpreted under a format contract, mapped through an emulated or native address space, cross I/O boundaries, become network records, and are finally protected by integrity/authentication rules.

It also restores substantive lines from the retired Systems volume that were not textually present after the three-master consolidation. They are preserved here because provenance loss is a repository defect even when a newer explanation exists elsewhere.

## 1. Deterministic machines are state-transition systems

A useful emulator mental model is

[
S_{t+1}=T(S_t,input_t),
]

where (S_t) includes every state variable whose value can affect future observations: registers, flags, memory/device state, mapper state, interrupt lines, cycle/timing state and deterministic external input.

If a supposedly deterministic replay diverges, either the transition function differs, some relevant state was omitted from the snapshot, or the input stream differs.

That same state-transition framing transfers to parsers, network protocols and build pipelines: make the state and transition contract explicit enough that another implementation can reproduce it.

## 2. Reset is a bus transaction, not a hard-coded program counter

A reset vector is an excellent first test of whether we understand the bus. The CPU constructs PC from the two bytes visible at addresses $FFFC and $FFFD, low byte first. Those addresses are mapped by the cartridge. On an NROM cartridge with one 16-KiB PRG bank, the bank appears twice in CPU space from $8000 to $FFFF. The reset vector stored near the end of that bank can direct execution to $8000. A test that always hard-codes PC=$8000 could appear to work on a toy ROM while being architecturally wrong for a cartridge whose reset vector points elsewhere.

The forcing lesson is general: CPU-visible addresses are requests to an address decoder. They are not direct indices into one universal byte array.

## 3. The 6502 stack is guest machine state

A stack pointer does not magically reference host language call frames. The 6502 stack lives on CPU page $0100–$01FF and grows downward as SP changes. Subroutine call JSR stores a return address using the emulated stack and jumps to the callee; RTS pulls the saved address and adds one to resume after the call. This differs from a typical high-level program's description of calling a function. Interrupt entry similarly saves a return context and jumps to an interrupt vector. The saved status byte must respect the 6502-specific break/unused-bit conventions; they are not simply independent physical flags that can be blindly copied.

This is why emulator code should keep guest architectural state explicit rather than letting host-language control flow accidentally stand in for guest mechanisms.

## 4. Addressing modes are address-generation programs

Immediate addressing obtains the operand directly from the instruction stream. Zero-page addressing takes an 8-bit address from the stream and reads at that address within $0000–$00FF. Absolute addressing combines two operand bytes into a 16-bit little-endian address. Zero-page-indexed addressing adds X or Y with 8-bit wraparound, while absolute-indexed modes add to a 16-bit base and may cross a 256-byte page. Indexed-indirect means the index is applied to a zero-page pointer location *before* fetching the target address. Indirect-indexed obtains the pointer first and adds the index to the fetched address. Confusing these yields a program that may execute correctly for one set of values and fail as soon as a pointer crosses a page.

Do not implement each opcode as a monolithic special case. Separate instruction semantics from reusable address-generation behavior where the architecture permits it, while preserving timing/page-cross differences that are observable.

## 5. Carry and signed overflow answer different arithmetic questions

For ADC, let A be the accumulator's original 8-bit unsigned value, M the operand and C the incoming carry bit. Form the full integer sum (u=A+M+C) and the 8-bit result (r=umod256). Set carry if (u>255), zero if (r=0), and negative from result bit 7. Signed overflow is a different condition: the sum of two same-sign two's-complement operands cannot be represented in the signed 8-bit range. One bitwise expression for overflow is

[
V = igl(
eg(Aoplus M);&;(Aoplus r);&;0x80igr)
e0.
]

A=$7F, M=$01, C=0 produces $80: signed overflow V=1, N=1, and unsigned carry C=0. By contrast $FF+$01 produces $00 with C=1 and Z=1 but V=0. These two independently testable cases prove why a CPU interpreter must not use one generic overflow boolean for both flags. The NES 2A03 does not perform BCD decimal ADC even if the decimal-status bit is set. An apparently minor misunderstanding here breaks arithmetic, conditional branches and some game logic.

## 6. The NES CPU bus maps behavior, not only storage

An NES CPU address is a request to the bus, and the address decoder decides which hardware responds. The 2 KiB internal RAM occupies $0000–$07FF, mirrored through $1FFF. CPU addresses $2000–$2007 correspond to PPU registers, mirrored across $2008–$3FFF. Selected APU and controller registers occur around $4000–$4017, while cartridge-controlled space usually includes $4020–$FFFF and, on common NROM boards, PRG ROM from $8000 upward.

This matters because a read can have effects. Reading a controller register can advance its shift position. Reading PPU status can clear or otherwise affect status/latch state under hardware timing rules. Writing a mapper register can change what later CPU ROM reads mean at the same address. Writing $4014 requests a DMA transfer into PPU sprite OAM; an actual transfer and CPU stall must obey the hardware cycle schedule. Representing every device register as a plain mutable byte array cannot reproduce those observable behaviors.

A robust bus API therefore makes reads/writes calls into mapped devices rather than exposing one giant mutable array.

## 7. Cartridge parsing is validation plus compatibility policy

The iNES header occupies 16 bytes and begins with the bytes $4E $45 $53 $1A. A classic header records PRG ROM size in 16-KiB units, CHR ROM size in 8-KiB units and flags encoding mapper number, mirroring and optional features. A 16-KiB PRG NROM bank mirrors in CPU space; a 32-KiB bank maps directly. CHR size zero commonly selects CHR RAM, not necessarily an error. The mapper determines how cartridge circuitry maps CPU and PPU addresses, including bank switching. NROM or mapper 0 is only the simplest case; a real emulator cannot accept every mapper value and treat it as NROM.

A parser should also distinguish content validation from ROM compatibility. Passing its supported-format checks proves that the byte layout can be interpreted as the selected subset, not that the ROM will execute under the few opcodes the test interpreter understands. The executable test uses a tiny synthetic PRG, sets its reset vector and confirms that $C000 aliases $8000 for a one-bank NROM fixture.

This is the same contract used by model/asset parsers elsewhere in the repository: syntactic validity and runtime capability are separate gates.

## 8. Native OS boundaries: handles, descriptors and partial operations

A file descriptor or OS handle is a process-local capability to an operating-system resource. It is not the file itself and should not be serialized as durable identity.

Reads and writes may be partial. A correct loop tracks the remaining byte range and handles interruption, EOF and errors according to platform semantics. Network sockets make this especially visible: one `send` call is not a promise that a complete application record has been transmitted, and one stream `recv` is not a promise that one logical message has arrived.

Resource lifetime should be RAII-shaped: open/acquire creates an owned handle, move transfers ownership, close/destruction releases it, and duplicate/borrow operations must be explicit.

## 9. Stream protocols need framing above TCP

TCP provides an ordered reliable byte stream, not message boundaries. If the application sends records A and B, the receiver may observe bytes split or combined across receive calls.

A simple framing protocol can use:

[
[	ext{magic}][	ext{version}][	ext{length}][	ext{type}][	ext{payload}][	ext{auth/tag}],
]

with fixed endianness for integers and strict length limits before allocation.

The parser should be incremental: retain incomplete header/payload state across reads, reject impossible lengths, and consume exactly one complete frame at a time.

UDP preserves datagram boundaries but not reliable delivery, order or uniqueness. If the application needs those properties, its protocol must add them or choose a different transport.

## 10. Network byte order is an interpretation contract

Multi-byte integers crossing machines need an agreed byte order. Network protocols traditionally use big-endian network order, while many host CPUs are little-endian.

Never serialize a C++ struct by writing its raw object bytes unless the format explicitly defines the exact layout and you have proven the type satisfies it. Padding, alignment, enum/bool size and host endianness can all violate a wire protocol.

Use explicit field encoding/decoding with bounds checks.

## 11. Integrity, authentication and secrecy are different goals

A checksum can detect accidental corruption; a cryptographic hash gives a stronger collision/preimage-oriented digest property; a MAC authenticates a message to parties sharing a secret; encryption protects confidentiality. An AEAD construction combines encryption with authentication under a specified nonce/key contract.

Do not call CRC “security.” Do not call a bare hash “authentication” when an attacker can recompute it.

PROJECTALLMIGHT should describe the security property first, then choose a standard primitive/library rather than inventing cryptography in engine code.

## 12. HKDF separates extraction from expansion

HKDF uses an extract step to derive a pseudorandom key from input key material and salt, then expands context-separated output key material.

The expansion recurrence is

[
T(0)=epsilon,qquad
]

[
T(i)=operatorname{HMAC}_{PRK}
left(T(i-1)parallel infoparallel iight),
]

then concatenates blocks until reaching the requested length. The information string helps separate cryptographic contexts, so a key intended for snapshot authentication need not coincide with one intended for a different protocol purpose. This is not a substitute for a password-hardening KDF or sufficient entropy in initial key material.

Do not implement HMAC/HKDF from memory in production merely because the equation is understood; use a vetted cryptographic library and test against published vectors.

## 13. Merkle trees authenticate sets/streams through a root

A Merkle tree hashes leaves and internal combinations so a compact root commits to the entire structured set. Inclusion proofs provide sibling hashes along a path, allowing a verifier to recompute the root without receiving every leaf.

The exact domain separation matters: distinguish leaf hashing from internal-node hashing and define ordering. Ambiguous concatenation can make structurally different trees hash the same byte sequence before the hash is applied.

Merkle authentication proves membership relative to a trusted root; it does not by itself tell you who authenticated the root.

## 14. Shamir secret sharing is polynomial interpolation, not encryption

A (t)-of-(n) Shamir scheme chooses a random degree-(t-1) polynomial over a finite field whose constant term is the secret. Shares are evaluations at distinct nonzero field points. Any (t) shares reconstruct the constant term by interpolation; fewer than (t) reveal no information in the ideal information-theoretic model.

Engineering requirements include a correct finite field, cryptographically secure randomness, authenticated share identity/context and careful serialization. Shares do not automatically provide integrity; corrupted shares can break reconstruction unless the surrounding protocol detects them.

## 15. Phase-28 evidence boundary

The repository already contains `tools/labs/phase28_emulator_core_lab.cpp`, which is the strongest executable evidence in this phase. This chapter repairs conceptual/system coverage but does **not** pretend that the current repository has production-grade socket, filesystem or cryptographic-library integration tests.

The next executable additions should therefore be:

1. incremental framed-stream parser with randomized chunking;
2. file read/write loop with partial-operation simulation;
3. network-order serialization golden vectors;
4. crypto-library SHA-256/HMAC/HKDF published-vector tests;
5. Merkle inclusion proof witness;
6. Shamir finite-field reconstruction/corruption fixtures.

Until those exist, Phase 28 should remain `developing` in the mastery ledger.

## Verbatim preservation anchors from the retired Systems source

The following four lines are retained exactly as provenance anchors because the integrated explanations above normalize display-math formatting. They are not additional teaching; they allow the preservation audit to distinguish a formatting rewrite from source loss.

For ADC, let A be the accumulator's original 8-bit unsigned value, M the operand and C the incoming carry bit. Form the full integer sum \(u=A+M+C\) and the 8-bit result \(r=u\bmod256\). Set carry if \(u>255\), zero if \(r=0\), and negative from result bit 7. Signed overflow is a different condition: the sum of two same-sign two's-complement operands cannot be represented in the signed 8-bit range. One bitwise expression for overflow is

T(0)=\epsilon,\qquad
T(i)=\operatorname{HMAC}_{PRK}
\left(T(i-1)\parallel info\parallel i\right),

