#pragma once
// AllMight / Book 1 / Pass 07: CPU state and legal-subset execution.
// This is a NOT-CYCLE-ACCURATE 6502 *instruction* laboratory. It is NOT a
// Nintendo 2A03 emulator, a complete CPU, or a PPU/APU/interrupt/DMA model.
//
// Supported opcodes only: EA NOP, A9 LDA #, A2 LDX #, 85 STA zp,
// E8 INX, 18 CLC, 38 SEC, 69 ADC # (binary mode),
// D0 BNE, F0 BEQ, 4C JMP absolute.
// BRK is NOT a halt and remains unsupported until real interrupt semantics
// are implemented. Unsupported opcodes return a typed failure with the CPU
// state unchanged. A completed instruction commits exactly once.
//
// Each fetch uses the already verified NES-01/02 NROM and CpuBus model.
#include "nes_bus_nrom.hpp"
#include <bit>
#include <cstdint>
#include <expected>
#include <limits>
#include <type_traits>

namespace allmight::nes::cpu {

struct ProgramCounter {
    std::uint16_t value{};
    explicit constexpr ProgramCounter(std::uint16_t address) noexcept : value(address) {}
    friend constexpr bool operator==(ProgramCounter, ProgramCounter) = default;
};
struct CpuRegisters {
    std::uint8_t a{};
    std::uint8_t x{};
    std::uint8_t y{};
    std::uint8_t sp{0xFD};
    std::uint8_t p{0x24}; // IRQ disable + reserved bit after reset.
    ProgramCounter pc{0};
    friend constexpr bool operator==(CpuRegisters, CpuRegisters) = default;
};
struct InstructionTrace {
    ProgramCounter before{0};
    ProgramCounter after{0};
    std::uint8_t opcode{};
    std::uint8_t cycle_cost{};
    std::uint64_t total_cycles{};
    bool branch_taken{};
    friend constexpr bool operator==(InstructionTrace, InstructionTrace) = default;
};
enum class CpuError {
    not_reset,
    bus_read,
    bus_write,
    unsupported_opcode,
    counter_overflow
};
struct CpuFailure {
    CpuError kind{};
    BusError bus{BusError::unimplemented_device};
    ProgramCounter at{0};
    std::uint8_t opcode{};
};

inline constexpr std::uint8_t flag_carry = 0x01;
inline constexpr std::uint8_t flag_zero = 0x02;
inline constexpr std::uint8_t flag_overflow = 0x40;
inline constexpr std::uint8_t flag_negative = 0x80;

class Cpu6502Subset {
public:
    explicit Cpu6502Subset(CpuBus& borrowed_bus) noexcept : bus_(borrowed_bus) {}
    Cpu6502Subset(const Cpu6502Subset&) = delete;
    Cpu6502Subset& operator=(const Cpu6502Subset&) = delete;

    [[nodiscard]] const CpuRegisters& registers() const noexcept { return state_; }
    [[nodiscard]] bool ready() const noexcept { return ready_; }
    [[nodiscard]] std::uint64_t total_cycles() const noexcept { return cycles_; }

    // A reset's vector read is staged. On failure the old CPU state persists.
    // This teaching model clears registers; NES reset hardware behavior is
    // more nuanced and must not be inferred from this initialization routine.
    [[nodiscard]] std::expected<void, CpuFailure> reset() noexcept {
        const auto low = bus_.read(0xFFFC);
        if (!low) return std::unexpected(CpuFailure{
            CpuError::bus_read, low.error(), ProgramCounter{0xFFFC}, 0});
        const auto high = bus_.read(0xFFFD);
        if (!high) return std::unexpected(CpuFailure{
            CpuError::bus_read, high.error(), ProgramCounter{0xFFFD}, 0});
        CpuRegisters initial{};
        initial.pc = ProgramCounter{
            static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(*low) |
                (static_cast<std::uint16_t>(*high) << 8U))};
        state_ = initial;
        cycles_ = 0;
        ready_ = true;
        return {};
    }

    [[nodiscard]] std::expected<InstructionTrace, CpuFailure> step() noexcept {
        if (!ready_) return std::unexpected(CpuFailure{
            CpuError::not_reset, BusError::unimplemented_device, state_.pc, 0});
        const auto begin = state_.pc;
        auto opcode_read = bus_.read(begin.value);
        if (!opcode_read) return std::unexpected(CpuFailure{
            CpuError::bus_read, opcode_read.error(), begin, 0});
        const std::uint8_t opcode = *opcode_read;
        CpuRegisters next = state_;
        std::uint16_t after = static_cast<std::uint16_t>(begin.value + 1U);
        std::uint8_t cost = 0;
        bool taken = false;
        bool write = false;
        std::uint16_t write_address{};
        std::uint8_t write_value{};

        // Read operand bytes from CPU address space, never from a raw ROM
        // buffer. Fetches can wrap the 16-bit program counter as on 6502.
        const auto operand = [this, begin, opcode](std::uint16_t address)
            noexcept -> std::expected<std::uint8_t, CpuFailure> {
            const auto value = bus_.read(address);
            if (!value) return std::unexpected(CpuFailure{
                CpuError::bus_read, value.error(), ProgramCounter{address}, opcode});
            return *value;
        };
        const auto update_zn = [](CpuRegisters& regs, std::uint8_t value) noexcept {
            regs.p = static_cast<std::uint8_t>(
                (regs.p & ~(flag_zero | flag_negative)) |
                (value == 0 ? flag_zero : 0) |
                (value & flag_negative));
        };

        switch (opcode) {
            case 0xEA: // NOP: two base cycles, no operands
                cost = 2;
                break;
            case 0xA9: // LDA #immediate
            case 0xA2: { // LDX #immediate
                const auto imm = operand(after);
                if (!imm) return std::unexpected(imm.error());
                after = static_cast<std::uint16_t>(after + 1U);
                if (opcode == 0xA9) next.a = *imm;
                else next.x = *imm;
                update_zn(next, *imm);
                cost = 2;
                break;
            }
            case 0x85: { // STA zero-page: a real bus write, not PRG mutation
                const auto zp = operand(after);
                if (!zp) return std::unexpected(zp.error());
                after = static_cast<std::uint16_t>(after + 1U);
                write = true;
                write_address = *zp;
                write_value = next.a;
                cost = 3;
                break;
            }
            case 0xE8: // INX, wraps in the 8-bit register domain
                next.x = static_cast<std::uint8_t>(next.x + 1U);
                update_zn(next, next.x);
                cost = 2;
                break;
            case 0x18: // CLC: preserve unrelated status bits
                next.p = static_cast<std::uint8_t>(next.p & ~flag_carry);
                cost = 2;
                break;
            case 0x38: // SEC
                next.p = static_cast<std::uint8_t>(next.p | flag_carry);
                cost = 2;
                break;
            case 0x69: { // ADC immediate in binary mode (D flag unreachable here)
                const auto imm = operand(after);
                if (!imm) return std::unexpected(imm.error());
                after = static_cast<std::uint16_t>(after + 1U);
                const unsigned lhs = next.a;
                const unsigned rhs = *imm;
                const unsigned input_carry = (next.p & flag_carry) ? 1U : 0U;
                const unsigned sum = lhs + rhs + input_carry;
                const auto result = static_cast<std::uint8_t>(sum & 0xFFU);
                const bool overflow = ((~(lhs ^ rhs) & (lhs ^ result) & 0x80U) != 0);
                next.p = static_cast<std::uint8_t>(
                    (next.p & ~(flag_carry | flag_overflow)) |
                    (sum > 0xFFU ? flag_carry : 0) |
                    (overflow ? flag_overflow : 0));
                next.a = result;
                update_zn(next, result);
                cost = 2;
                break;
            }
            case 0xD0: // BNE relative
            case 0xF0: { // BEQ relative
                const auto encoded = operand(after);
                if (!encoded) return std::unexpected(encoded.error());
                after = static_cast<std::uint16_t>(after + 1U);
                const bool is_zero = (next.p & flag_zero) != 0;
                taken = (opcode == 0xF0) ? is_zero : !is_zero;
                cost = 2;
                if (taken) {
                    const auto signed_delta = std::bit_cast<std::int8_t>(*encoded);
                    const auto target = static_cast<std::uint16_t>(
                        static_cast<std::int32_t>(after) +
                        static_cast<std::int32_t>(signed_delta));
                    const bool page_changed = (after & 0xFF00U) != (target & 0xFF00U);
                    after = target;
                    cost = static_cast<std::uint8_t>(cost + 1U +
                                                    (page_changed ? 1U : 0U));
                }
                break;
            }
            case 0x4C: { // JMP absolute, little endian
                const auto low = operand(after);
                if (!low) return std::unexpected(low.error());
                const auto high_address = static_cast<std::uint16_t>(after + 1U);
                const auto high = operand(high_address);
                if (!high) return std::unexpected(high.error());
                after = static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(*low) |
                    (static_cast<std::uint16_t>(*high) << 8U));
                cost = 3;
                break;
            }
            default:
                // 0x00 BRK is not a pedagogical HALT. It triggers an interrupt
                // in a real processor and needs the full stack/vector model.
                return std::unexpected(CpuFailure{
                    CpuError::unsupported_opcode,
                    BusError::unimplemented_device, begin, opcode});
        }
        if (cycles_ > std::numeric_limits<std::uint64_t>::max() - cost) {
            return std::unexpected(CpuFailure{
                CpuError::counter_overflow, BusError::unimplemented_device,
                begin, opcode});
        }

        // STA writes only to validated zero-page space for this subset. If
        // bus write fails, no architectural register state has been committed.
        if (write) {
            const auto stored = bus_.write(write_address, write_value);
            if (!stored) return std::unexpected(CpuFailure{
                CpuError::bus_write, stored.error(), ProgramCounter{write_address}, opcode});
        }
        next.pc = ProgramCounter{after};
        state_ = next;
        cycles_ += cost;
        return InstructionTrace{begin, next.pc, opcode, cost, cycles_, taken};
    }

private:
    CpuBus& bus_;          // borrowed; the bus must outlive the CPU
    CpuRegisters state_{};
    std::uint64_t cycles_{};
    bool ready_{false};
};
static_assert(!std::is_copy_constructible_v<Cpu6502Subset>);
static_assert(std::is_trivially_copyable_v<InstructionTrace>);

} // namespace allmight::nes::cpu
