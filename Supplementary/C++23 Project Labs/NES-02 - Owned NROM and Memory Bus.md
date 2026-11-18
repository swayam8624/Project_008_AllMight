### NES-02 — The same address can name different storage, but ownership must never be ambiguous

The previous NES milestone made a file's first sixteen bytes intelligible and rejected malformed cartridge layouts before creating devices. That solved only the first problem. A valid file is not yet a running cartridge, and a CPU address is not an offset into a file. The file may be kept in a vector, the emulator may copy the program into cartridge-owned storage, the CPU may emit a 16-bit address, and a bus or mapper must decide which device supplies the byte. These are different coordinate systems and different lifetimes. Our second project milestone joins them without losing that distinction.

This addition belongs immediately after the existing explanations of storage reuse, object identity and generational handles. It **does not replace** those explanations or the intervening treatment of builds, modules, serialization, layouts, pointers or RAII. Instead, it gives the learner one executable system in which the terms can be tested.

#### Four identities that must not be confused

Consider a cartridge image stored in an input `std::vector<std::uint8_t>``. The vector object's address is not the address of its allocated elements, and neither address is an NES CPU address. Its elements have object lifetimes controlled by that vector. A parser borrowing a `std::span<const std::uint8_t>`` points into those elements, but the span does not own them and becomes invalid if the vector is destroyed or its storage is invalidated. A validated `Layout`` returned by NES-01 stores integer counts, offsets and flags; it does not retain any pointer. Finally, a `NromCartridge`` that copies the image's PRG and CHR regions into its own vectors owns completely separate bytes. Destroying the original file buffer no longer destroys the cartridge's program.

Draw these four objects on paper: input bytes, borrowed span, validated layout, and owned cartridge. Put an arrow on each borrowing relationship, and mark the operation that ends its validity. Before continuing, ask which of them a CPU bus should keep after loading. It should own or otherwise lifetime-manage the cartridge, **not** keep a borrowed view into an arbitrary caller's temporary file buffer.

In the C++23 laboratory, construction is expressed as a factory returning `std::expected<NromCartridge, CartridgeError>``. Only fully validated inputs can become a cartridge. The resulting object owns its PRG and CHR storage; the factory does not publish a half-built cartridge when it rejects a malformed file. The cartridge is moveable but deliberately not copyable, emphasizing resource transfer and a single authoritative owner at the bus boundary. Each vector cleans up through RAII; a move transfers the resources without promising that references to a moved-from object remain meaningful.

The CPU bus contains `std::array<std::uint8_t, 2048>`` for internal RAM and `std::optional<NromCartridge>`` for its optional cartridge. The first gives storage whose size is fixed by the model. The second represents absence without a null raw pointer. Inserting a validated cartridge publishes one complete object to the bus. The authoring contract permits replacing a cartridge, but a failed validation takes place **before** insertion and so cannot erase the previously running cartridge. This is a small but important example of transactional publication: prepare a complete candidate, then replace the visible state.

#### Derive the 2 KiB RAM mirrors from the electrical boundary

The NES CPU emits sixteen address bits, allowing 65,536 address values. That number is an **address-space size**, not a guarantee of 65,536 independently stored RAM bytes. In the lower address range, the machine has just 2 KiB of internal RAM, mirrored four times over addresses `0x0000``–`0x1FFF``. Thus all addresses with the same lower eleven bits reach the same physical byte.

Mathematically, define the RAM index only for addresses in that lower range:

$$
i_{\mathrm{RAM}}(a)=a \mathbin{\&} 0x07FF,\quad 0\le a\le 0x1FFF.
$$

Because 0x07FF has eleven low bits set, `0x0001``, `0x0801``, `0x1001`` and `0x1801`` all map to index 1. Likewise `0x1FFF`` maps to physical index 0x07FF, the last byte of the array. The actual C++ store is simply `ram_[cpu_ram_index(address)] = value``, but that expression becomes justified only after the range dispatch proves that the address refers to RAM.

This is a classic distinction between the **identity of an address** and the **identity of a memory location**. A decoder can ignore address lines. Two address values can refer to one storage cell without making the addresses numerically equal. Do not generalize the mask to PPU registers, APU, cartridge memory or the entire address space. Their hardware decoders follow other rules.

#### The cartridge mapper translates another address space

Our first mapper model is **NROM**, iNES mapper 0. NROM does not have CPU-program bank-switching registers. It is valuable as a teaching starting point precisely because it still forces us to distinguish the CPU-visible region, a physical ROM bank, and a file buffer.

When the CPU addresses `0x8000``–`0xFFFF``, let the offset from the beginning of the PRG-ROM window be

$$
d=a-0x8000.
$$

For **NROM-128**, one 16 KiB PRG-ROM bank is wired into both halves of the 32 KiB CPU-visible window. Its storage offset is

$$
o_{\mathrm{PRG}}(a)=d\bmod 16384.
$$

For **NROM-256**, two 16 KiB banks fill the window, so the storage offset is simply `d``, from zero through 32,767. The lab implements both cases through `offset % prg_.size()``, but *only after factory validation* has proved that the vector contains exactly 16,384 or 32,768 bytes. A modulo operation would be unsafe if the program size were zero, and a three-bank image would not be an NROM cartridge. A moved-from object is also not assumed to retain a nonempty vector; its read method checks for empty storage rather than performing a modulo by zero.

Hand trace the NROM-128 case: `0x8000`` and `0xC000`` both produce offset 0; `0xBFFF`` and `0xFFFF`` both produce 16,383. With NROM-256, `0x8000`` produces offset 0 while `0xC000`` produces offset 16,384. If your tests do not distinguish these cases, they may pass even though one of the two cartridge boards is modeled incorrectly.

The PPU uses a different address space. The cartridge may provide an 8 KiB CHR-ROM pattern region at PPU addresses `0x0000``–`0x1FFF``. CHR-ROM reads are permitted but writes do not change it. If the iNES header declares zero CHR-ROM banks, this teaching model allocates 8 KiB of **CHR RAM** on construction, initializes it to zero and permits writes. PPU name tables and palettes are **not** part of this pattern-store experiment; their addressing, mirroring and side effects will be implemented in a later rendering milestone.

Some real NROM-family boards have optional PRG RAM or other board-specific behavior. Our current teaching model does not pretend to emulate those variants. CPU addresses `0x4020``–`0x7FFF`` are reported as **unmapped in this lab**, not declared physically nonexistent in every NES cartridge. General mapper and board support is a later project milestone.

#### A program should express what is not implemented

An emulator that returns zero for every unimplemented device may look as though it runs, but it has made absence indistinguishable from an actual zero byte. The Book 1 bus model therefore has a typed result:

```cpp
// The bus owns internal RAM and may own an entire NROM cartridge.
// Absent devices and unsupported address ranges are different failures.
std::expected<Byte, BusError> read(std::uint16_t address) const noexcept {
    if (address <= 0x1FFF) {
        return ram_[cpu_ram_index(address)];
    }
    if (address <= 0x401F) {
        return std::unexpected(BusError::unimplemented_device);
    }
    if (address < 0x8000) {
        return std::unexpected(BusError::unmapped_address);
    }
    if (!cartridge_) {
        return std::unexpected(BusError::no_cartridge);
    }
    const auto value = cartridge_->read_prg(address);
    if (!value) {
        return std::unexpected(BusError::no_cartridge);
    }
    return *value;
}
```

This is a **teaching API policy**. Real NES hardware does not return C++ exceptions or `std::expected`` errors, and certain otherwise unhandled reads produce open-bus values influenced by electrical history. The lab refuses to invent such behavior until the corresponding hardware is modeled. The separation prevents an unimplemented PPU register from silently masquerading as valid RAM.

Writes to NROM PRG-ROM are likewise reported as a `read_only`` diagnostic and leave the stored PRG bytes unchanged. The real hardware does not deliver a C++ error to the CPU for a ROM write. For later switchable mappers, writes in high address ranges may instead control banking registers or board circuitry, so **do not** promote this NROM policy to a universal cartridge rule.

#### Connect this lab to real KairoNES code responsibly

Look at the current [NESEmulator/Bus.cpp](https://github.com/swayam8624/NESEmulator/blob/main/NESEmulator/Bus.cpp). Its RAM-mirroring mask `addr & 0x07FF`` is a useful starting point, and its CPU/PPU/APU clock structure shows the larger system the learner will eventually assemble. But some operations dispatch through `cart`` before verifying that a cartridge has been inserted. It also has an overlapping controller/APU write-dispatch area: `0x4017`` is caught by the APU branch before the later controller-address test. That is not automatically a bug—`0x4017`` is the APU frame counter's write address on the NES—but the teaching model must explicitly separate **controller data reads**, **controller strobe writes**, and **APU register writes**, instead of treating one address as if reads and writes served the same device. We do not yet implement controllers or APU registers.

The existing [NESEmulator/Cartridge.cpp](https://github.com/swayam8624/NESEmulator/blob/main/NESEmulator/Cartridge.cpp) already has mapper abstractions and CHR storage. Our new reference stays deliberately narrower. It validates mapper 0 and accepted NROM bank sizes, deep-copies the file payload into owned vectors, and gives negative tests for unsupported mapper/bank combinations. No changes are made to the production emulator. The next integration step requires reviewing its current revision, wiring a tested boundary into its actual call graph, and running the original emulator's own tests and frame traces on your Mac.

#### Run the cumulative C++23 project, not a disposable demonstration

NES-02 lives in `tools/labs/nes_bus_nrom_cpp23_lab.cpp``. It imports the *same* `tools/labs/nes_course/ines_layout.hpp`` parser used by NES-01, so the header format is defined once and used by successive project chapters. The existing CMake entry point under `Book Production/Seven-Book Curriculum/Book01/`` now builds both labs and runs both through CTest. No commercial ROM or GUI window is required for these source-level tests.

The tests first construct an empty bus and verify that reading a high cartridge address reports `no_cartridge``. They write to physical RAM through one address and read the same byte through all four mirrors; this directly demonstrates address equivalence without asserting that the pointer values are the same. They deliberately access unimplemented PPU/controller ranges and the unmodeled expansion region to ensure that those failures are not silently converted into success.

Next, synthetic cartridges verify one-bank NROM mirroring, two-bank NROM non-mirroring, unchanged CHR-ROM on attempted writes, writable CHR-RAM, wrong-mapper rejection, unsupported PRG/CHR bank counts and malformed image rejection. An especially important test fills a file buffer with known program and character bytes, inserts the parsed cartridge, then clears the original file buffer. Reads must still return the original values. The result is not merely “the pointer wasn't null”; it is evidence that the cartridge owns separate storage.

Test failure also matters. Try constructing a malformed replacement while a valid cartridge is inserted. The factory must return an error and the old cartridge must remain readable. This is how we demonstrate **failure atomicity** at the preparation/publication boundary instead of just describing it. Run the tests again with address and undefined-behavior sanitizers. These checks do not prove cycle accuracy or support for full NES hardware, and the PPU/APU address handling remains explicitly incomplete.

For a deeper reconstruction, remove the modulo from NROM-128 and implement its mapping with the mask `0x3FFF``. Prove the two forms are equivalent **for the validated 16 KiB case only**, then use a 32 KiB cartridge to demonstrate why applying the same mask universally is wrong. Change the bus's no-cartridge behavior to return zero and construct a test that distinguishes the two designs; use the test to argue why typed failure is pedagogically better at this stage. Finally, try to return a span of the cartridge's PRG bytes from a temporary cartridge factory and explain the lifetime hazard without executing the invalid code.

#### Completion contract and next handoff

A learner completes this project milestone when they can draw the storage ownership graph, distinguish file offsets from CPU and PPU addresses, derive all RAM and NROM mapping formulas, implement an owning cartridge and an explicit bus API in C++23, and explain why both negative tests and lifetime tests are necessary. An unchanged or shorter page count is not our criterion: we need an independently reconstructible behavior that ties directly into the existing long-form chapter.

**Next, NES-03 will model CPU state:** the 6502-compatible CPU's program counter, accumulator, index registers, stack pointer and status flags; instruction fetch and decode; address modes; and a deterministic trace driven through this tested bus. A full emulator, cycle accuracy, PPU pixel output, audio and all mapper variants are not claimed here.

**Source boundaries:** consult the [NESdev CPU memory map](https://www.nesdev.org/wiki/CPU_ALL), [NROM](https://www.nesdev.org/wiki/NROM), [programming NROM](https://www.nesdev.org/wiki/Programming_NROM) and [CHR-ROM versus CHR-RAM](https://www.nesdev.org/wiki/CHR-ROM_vs_CHR-RAM). The C++ semantics of spans, lifetime and `std::expected`` must be checked against the corresponding C++23 library and language contracts. Each repository-specific conclusion here is limited to the inspected source files, not an unverified assertion about the entire current emulator.
