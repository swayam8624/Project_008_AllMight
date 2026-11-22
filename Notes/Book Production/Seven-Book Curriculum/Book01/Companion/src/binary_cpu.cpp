// A deliberately small binary CPU: four opcodes, bounded instruction fetch,
// 256 bytes of data memory, and an explicit execution budget.
//
// This is NOT a 6502 implementation or an NES emulator. It demonstrates
// the distinction between instruction bytes, addresses, arithmetic width,
// program counter and a safe decision to stop on malformed input.
//
// Build: c++ -std=c++23 -Wall -Wextra -Wpedantic binary_cpu.cpp -o binary_cpu
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>

enum class Stop { halted, truncated, unknown_opcode, missing_halt, budget_exhausted };

struct Outcome {
    Stop reason;
    std::size_t instructions;
};

struct Machine {
    std::array<std::uint8_t, 256> memory{};
    std::uint8_t accumulator = 0;

    Outcome execute(std::span<const std::uint8_t> program, std::size_t budget) {
        std::size_t pc = 0;
        std::size_t executed = 0;

        while (executed < budget) {
            // Bounds must be checked before fetching the next opcode.
            if (pc >= program.size()) {
                return {Stop::missing_halt, executed};
            }
            const auto opcode = program[pc++];
            ++executed;

            if (opcode == 0xFF) {
                return {Stop::halted, executed};
            }

            if (opcode != 0x10 && opcode != 0x20 && opcode != 0x30) {
                return {Stop::unknown_opcode, executed};
            }

            // Three opcodes have one *address byte*, not an immediate value.
            if (pc >= program.size()) {
                return {Stop::truncated, executed};
            }
            const std::uint8_t address = program[pc++];
            if (opcode == 0x10) {
                accumulator = memory[address];
            } else if (opcode == 0x20) {
                // The explicit narrowing defines our 8-bit modulo-256 rule.
                accumulator = static_cast<std::uint8_t>(
                    static_cast<unsigned>(accumulator)
                    + static_cast<unsigned>(memory[address]));
            } else {
                memory[address] = accumulator;
            }
        }
        return {Stop::budget_exhausted, executed};
    }
};

int main() {
    Machine machine;
    machine.memory[0xF0] = 250;
    machine.memory[0xF1] = 17;
    // LOAD [F0], ADD [F1], STORE [F2], HALT.
    constexpr std::array<std::uint8_t, 7> program{
        0x10, 0xF0, 0x20, 0xF1, 0x30, 0xF2, 0xFF
    };
    const Outcome valid = machine.execute(program, 16);
    if (valid.reason != Stop::halted || valid.instructions != 4
        || machine.memory[0xF2] != 11) {
        std::cerr << "The reference instruction trace was incorrect.\n";
        return 1;
    }

    Machine truncated_machine;
    constexpr std::array<std::uint8_t, 1> truncated{0x10};
    if (truncated_machine.execute(truncated, 8).reason != Stop::truncated) {
        return 2;
    }

    Machine unknown_machine;
    constexpr std::array<std::uint8_t, 1> invalid{0x00};
    if (unknown_machine.execute(invalid, 8).reason != Stop::unknown_opcode) {
        return 3;
    }

    Machine bounded_machine;
    if (bounded_machine.execute(program, 1).reason != Stop::budget_exhausted) {
        return 4;
    }

    std::cout << "binary_cpu: all deterministic contract tests passed\n";
}
