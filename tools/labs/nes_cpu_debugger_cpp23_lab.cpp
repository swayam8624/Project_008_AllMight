// Project AllMight, Book 1 Pass 07 — C++23 NES CPU and structures.
// Cross-translation-unit test: public PIMPL, real NROM bus and instruction
// engine, ring buffer, exact ordered breakpoints, Bloom negative filter,
// priority heap, union-find RAM alias analysis. NO external ROMs or graphics.
//
// A correct partial CPU is more useful than a false claim of full 6502:
// unsupported BRK/opcodes, decimal arithmetic, interrupts, timing coupling,
// PPU and APU are deliberately absent.
#include "nes_course/cpu_debug_session.hpp"
#include "nes_course/cpu_debug_instruments.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::cpu;

void check(bool ok, std::string_view why) {
    if (!ok) throw std::runtime_error(std::string(why));
}
std::vector<Byte> image(std::uint16_t reset_pc = 0x8000) {
    // One 16KiB NROM-128 PRG bank, mirrored in the CPU's 0x8000-0xFFFF
    // range. Reset vector at virtual FFFC/FFFD maps to PRG offsets 3FFC/3FFD.
    std::vector<Byte> rom(header_bytes + prg_bank_bytes, 0);
    rom[0]='N'; rom[1]='E'; rom[2]='S'; rom[3]=0x1A;
    rom[4]=1; rom[5]=0;
    rom[header_bytes + 0x3FFCU] = static_cast<Byte>(reset_pc & 0xFFU);
    rom[header_bytes + 0x3FFDU] = static_cast<Byte>((reset_pc >> 8U) & 0xFFU);
    return rom;
}
void bytes_at(std::vector<Byte>& rom, std::uint16_t cpu_pc,
              std::initializer_list<Byte> opcodes) {
    check(cpu_pc >= 0x8000, "test programs use PRG address space");
    const std::size_t offset = (static_cast<std::size_t>(cpu_pc) - 0x8000U)
                              % prg_bank_bytes;
    check(offset + opcodes.size() <= prg_bank_bytes,
          "test instruction must fit in the bank");
    std::size_t cursor = header_bytes + offset;
    for (Byte byte : opcodes) rom[cursor++] = byte;
}
void mount(CpuBus& bus, const std::vector<Byte>& rom) {
    auto parsed = NromCartridge::from_image(rom);
    check(parsed.has_value(), "synthetic iNES must parse");
    const auto inserted = bus.insert(std::move(*parsed));
    check(inserted.has_value(), "supported NROM must mount");
}
void step_ok(CpuDebugSession& session, std::uint16_t before,
             std::uint16_t after, Byte opcode, Byte cycles,
             std::uint64_t total) {
    const auto result = session.step();
    check(result.has_value(), "supported opcode should execute");
    check(result->before == before && result->after == after &&
          result->opcode == opcode && result->cycles == cycles &&
          result->total_cycles == total,
          "instruction PC/opcode/cycle trace mismatch");
}

void complete_program_and_breakpoint() {
    auto rom = image();
    bytes_at(rom,0x8000,{0xA9,0x03, // LDA #3
                         0x85,0x02, // STA $02, mirrored in RAM
                         0xA2,0x00, // LDX #0
                         0xE8,      // INX -> 1
                         0xD0,0x02, // BNE +2, skip BRK bytes
                         0x00,0x00, // unsupported BRK (must not run)
                         0x18,      // CLC
                         0x69,0x7F, // ADC #127: 3+127=130, overflow
                         0xEA,      // NOP
                         0x4C,0x00,0x80}); // JMP $8000
    CpuBus bus;
    mount(bus,rom);
    CpuDebugSession session(bus);
    const auto before_reset = session.step();
    check(!before_reset && before_reset.error() == SessionError::not_reset,
          "executing before reset must be a typed error");
    const auto reset = session.reset();
    check(reset.has_value(), "reset vector from PRG must load");
    auto initial = session.snapshot();
    check(initial && initial->ready && initial->pc == 0x8000 &&
          initial->sp == 0xFD && initial->p == 0x24,
          "reset should establish declared teaching state");

    const auto bp = session.add_breakpoint(0x800F);
    check(bp.has_value(), "adding breakpoint should work");
    check(session.next_breakpoint_at_or_after(0x8000) == 0x800F,
          "ordered breakpoint lower_bound should select next PC");
    check(!session.next_breakpoint_at_or_after(0x8010),
          "ordered breakpoint query above last entry must be empty");
    step_ok(session,0x8000,0x8002,0xA9,2,2);
    step_ok(session,0x8002,0x8004,0x85,3,5);
    check(bus.read(0x0002) == 3 && bus.read(0x1802) == 3,
          "zero-page STA must respect 2KiB NES RAM mirrors");
    step_ok(session,0x8004,0x8006,0xA2,2,7);
    step_ok(session,0x8006,0x8007,0xE8,2,9);
    step_ok(session,0x8007,0x800B,0xD0,3,12);
    step_ok(session,0x800B,0x800C,0x18,2,14);
    step_ok(session,0x800C,0x800E,0x69,2,16);
    auto after_adc = session.snapshot();
    check(after_adc && after_adc->a == 0x82 &&
          (after_adc->p & flag_overflow) != 0 &&
          (after_adc->p & flag_negative) != 0 &&
          (after_adc->p & flag_carry) == 0,
          "binary ADC must set N and V without carry in this example");
    step_ok(session,0x800E,0x800F,0xEA,2,18);
    check(session.pending_traces() == 8, "eight instructions tracked");

    const auto stopped = session.step();
    check(!stopped && stopped.error() == SessionError::breakpoint,
          "exact breakpoint must stop before executing JMP");
    check(session.snapshot()->pc == 0x800F &&
          session.snapshot()->total_cycles == 18,
          "breakpoint must leave architectural state unchanged");
    check(session.remove_breakpoint(0x800F).has_value(),
          "breakpoint removal must permit execution");
    step_ok(session,0x800F,0x8000,0x4C,3,21);
    check(session.pending_traces() == 9,
          "JMP trace should add exactly one pending record");
    const auto visit = session.possibly_visited(0x8007);
    check(visit && *visit, "executed branch PC must be Bloom-present");
    check(session.possibly_visited(0x8000) &&
          *session.possibly_visited(0x8000),
          "first PC must be recorded without false negatives");

    const auto first = session.pop_trace();
    check(first && first->before == 0x8000 && first->opcode == 0xA9,
          "trace ring order must be FIFO");
    for (int i=1; i<9; ++i) {
        check(session.pop_trace().has_value(), "all nine traces must drain");
    }
    check(!session.pop_trace(), "trace exhausted after all records popped");
    check(session.reset().has_value(), "successful repeat reset");
    check(session.pending_traces() == 0 && session.snapshot()->total_cycles == 0,
          "reset must clear old traces and cycle accounting");
}

void branch_cycles_and_signed_offsets() {
    {
        auto rom = image(0x80FB);
        bytes_at(rom,0x80FB,{0xA9,0x00, // LDA #0 -> Z set
                             0xF0,0x01}); // BEQ +1, cross 80FF->8100
        bytes_at(rom,0x8100,{0xEA});
        CpuBus bus; mount(bus,rom);
        CpuDebugSession session(bus);
        check(session.reset().has_value(), "branch reset");
        step_ok(session,0x80FB,0x80FD,0xA9,2,2);
        const auto branch = session.step();
        check(branch && branch->branch_taken && branch->after == 0x8100 &&
              branch->cycles == 4 && branch->total_cycles == 6,
              "taken BEQ crossing page costs 4 theoretical CPU cycles");
        step_ok(session,0x8100,0x8101,0xEA,2,8);
    }
    {
        auto rom = image();
        bytes_at(rom,0x8000,{0xD0,0xFE}); // BNE -2 => same PC
        CpuBus bus; mount(bus,rom);
        CpuDebugSession session(bus);
        check(session.reset().has_value(), "backwards branch reset");
        const auto branch = session.step();
        check(branch && branch->branch_taken && branch->after == 0x8000 &&
              branch->cycles == 3, "signed -2 offset must return to PC");
    }
    {
        auto rom = image();
        bytes_at(rom,0x8000,{0xA9,0x00,0xD0,0x7F});
        CpuBus bus; mount(bus,rom);
        CpuDebugSession session(bus);
        check(session.reset().has_value(), "untaken branch reset");
        step_ok(session,0x8000,0x8002,0xA9,2,2);
        const auto branch = session.step();
        check(branch && !branch->branch_taken && branch->after == 0x8004 &&
              branch->cycles == 2,
              "untaken BNE must ignore offset and cost 2 cycles");
    }
}

void adc_carry_and_inx_wrap() {
    auto rom = image();
    bytes_at(rom,0x8000,{0xA9,0x7F,0x18,0x69,0x01, // 127+1 => -128, V
                         0x38,0xA9,0xFF,0x69,0x00, // FF+0+carry => 00, C,Z
                         0xA2,0xFF,0xE8});
    CpuBus bus; mount(bus,rom);
    CpuDebugSession session(bus);
    check(session.reset().has_value(), "ADC test reset");
    step_ok(session,0x8000,0x8002,0xA9,2,2);
    step_ok(session,0x8002,0x8003,0x18,2,4);
    step_ok(session,0x8003,0x8005,0x69,2,6);
    check(session.snapshot()->a == 0x80 &&
          (session.snapshot()->p & flag_overflow) != 0,
          "signed overflow must be distinct from carry");
    step_ok(session,0x8005,0x8006,0x38,2,8);
    step_ok(session,0x8006,0x8008,0xA9,2,10);
    step_ok(session,0x8008,0x800A,0x69,2,12);
    check(session.snapshot()->a == 0 &&
          (session.snapshot()->p & flag_carry) != 0 &&
          (session.snapshot()->p & flag_zero) != 0 &&
          (session.snapshot()->p & flag_overflow) == 0,
          "ADC wrapping unsigned sum must set carry and zero");
    step_ok(session,0x800A,0x800C,0xA2,2,14);
    step_ok(session,0x800C,0x800D,0xE8,2,16);
    check(session.snapshot()->x == 0 &&
          (session.snapshot()->p & flag_zero) != 0,
          "INX wraps to zero in eight-bit domain");
}

void unsupported_opcode_is_not_a_halt() {
    auto rom = image();
    bytes_at(rom,0x8000,{0x00}); // BRK needs stack/IRQ semantics; not HALT
    CpuBus bus; mount(bus,rom);
    CpuDebugSession session(bus);
    check(session.reset().has_value(), "BRK test reset");
    const auto initial = session.snapshot();
    const auto result = session.step();
    check(!result && result.error() == SessionError::unsupported_opcode,
          "BRK must be rejected, not misrepresented as graceful stop");
    check(session.snapshot() == initial && session.pending_traces() == 0,
          "unsupported instruction must leave CPU unchanged");
}

void trace_ring_guards_the_cpu_and_wraps() {
    auto rom = image();
    for (unsigned i=0; i<32; ++i) bytes_at(
        rom,static_cast<std::uint16_t>(0x8000U+i),{0xEA});
    CpuBus bus; mount(bus,rom);
    CpuDebugSession session(bus);
    check(session.reset().has_value(), "trace ring reset");
    for (int i=0; i<16; ++i)
        check(session.step().has_value(), "first sixteen NOP steps succeed");
    const auto frozen = session.snapshot();
    check(session.pending_traces() == 16, "trace buffer fills exactly");
    const auto refused = session.step();
    check(!refused && refused.error() == SessionError::trace_full,
          "seventeenth step must fail before advancing CPU");
    check(session.snapshot() == frozen, "full queue cannot drop instruction state");
    for (int i=0; i<7; ++i) {
        const auto old = session.pop_trace();
        check(old && old->before == 0x8000U + static_cast<unsigned>(i),
              "oldest trace must drain first");
    }
    for (int i=0; i<7; ++i)
        check(session.step().has_value(), "wraparound frees seven slots");
    check(session.pending_traces() == 16, "ring refills without overwrite");
    for (int i=7; i<23; ++i) {
        const auto old = session.pop_trace();
        check(old && old->before == 0x8000U + static_cast<unsigned>(i),
              "FIFO order must remain intact after modulo wrap");
    }
    check(!session.pop_trace(), "ring drained after wraparound");
}

void inspection_heap_and_ram_aliases() {
    auto rom = image();
    bytes_at(rom,0x8000,{0xA9,0x6D});
    CpuBus bus; mount(bus,rom);
    check(bus.write(0x0012,0x52).has_value(), "RAM diagnostic fixture");
    MemoryProbeScheduler probes(bus);
    check(probes.schedule(ProgramCounter{0x0012},1).has_value(), "probe 1");
    check(probes.schedule(ProgramCounter{0x8000},5).has_value(), "probe 2");
    check(probes.schedule(ProgramCounter{0x8001},5).has_value(), "probe 3");
    check(probes.schedule(ProgramCounter{0x4016},3).has_value(), "probe 4");

    const auto first = probes.inspect_next();
    const auto second = probes.inspect_next();
    const auto third = probes.inspect_next();
    const auto fourth = probes.inspect_next();
    check(first && first->request.address.value == 0x8000 &&
          first->value == 0xA9, "highest priority first");
    check(second && second->request.address.value == 0x8001 &&
          second->value == 0x6D, "equal priority must respect submission order");
    check(third && third->request.address.value == 0x4016 &&
          !third->value &&
          third->value.error() == BusError::unimplemented_device,
          "unimplemented controller is a typed bus read error, not zero");
    check(fourth && fourth->request.address.value == 0x0012 &&
          fourth->value == 0x52, "low priority RAM read last");
    check(!probes.inspect_next(), "priority heap exhausted");

    // Union-find is justified for offline grouping of alias candidates.
    // The real bus must continue using its O(1) direct RAM mask.
    constexpr std::array<std::uint16_t,8> addresses{
        0x0012,0x0812,0x1012,0x1812,
        0x0034,0x0834,0x1034,0x1834
    };
    DisjointSet<8> aliases;
    for (std::size_t a=0; a<8; ++a) {
        for (std::size_t b=a+1; b<8; ++b) {
            if (cpu_ram_index(addresses[a]) == cpu_ram_index(addresses[b])) {
                (void)aliases.join(a,b);
            }
        }
    }
    for (std::size_t a=0; a<8; ++a) {
        for (std::size_t b=0; b<8; ++b) {
            check(aliases.connected(a,b) ==
                  (cpu_ram_index(addresses[a]) == cpu_ram_index(addresses[b])),
                  "DSU equivalence must match actual CPU RAM decoder");
        }
    }
}

void pimpl_is_real_and_move_has_a_policy() {
    static_assert(!std::is_copy_constructible_v<CpuDebugSession>);
    static_assert(std::is_nothrow_move_constructible_v<CpuDebugSession>);
    CpuBus bus;
    mount(bus,image());
    CpuDebugSession original(bus);
    check(original.reset().has_value(), "original debugger reset");
    CpuDebugSession moved(std::move(original));
    check(!original.reset() &&
          original.reset().error() == SessionError::moved_from,
          "moved-from public session must reject use rather than dereference null");
    check(!original.snapshot(), "moved-from session offers no owned snapshot");
    check(moved.snapshot() && moved.snapshot()->pc == 0x8000,
          "owned session representation and CPU identity move together");
    CpuDebugSession target(bus);
    target = std::move(moved);
    check(!moved.step() && moved.step().error() == SessionError::moved_from,
          "move assignment must empty source session");
    check(target.snapshot() && target.snapshot()->pc == 0x8000,
          "move-assignment target must carry live debugger");
}

} // namespace

int main() {
    try {
        complete_program_and_breakpoint();
        branch_cycles_and_signed_offsets();
        adc_carry_and_inx_wrap();
        unsupported_opcode_is_not_a_halt();
        trace_ring_guards_the_cpu_and_wraps();
        inspection_heap_and_ram_aliases();
        pimpl_is_real_and_move_has_a_policy();
        std::cout <<
            "NES-07C: CPU subset, byte mapping, cycle accounting, ring, "
            "PIMPL, Bloom, heap, breakpoints and RAM-DSU assertions passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "NES-07C FAILED: " << error.what() << '\n';
        return 1;
    }
}
