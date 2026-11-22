// Tiny CPU: independently runnable teaching ISA and adversarial checks.
// The ISA is defined in CH01_CPU_FROM_LOGIC.md; not a claim about real CPUs.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

struct TinyCPU {
    std::array<std::uint8_t, 256> memory{};
    std::size_t pc = 0;
    std::uint8_t accumulator = 0;
    bool halted = false;

    std::uint8_t fetch() {
        if (pc >= memory.size())
            throw std::runtime_error("program counter exceeded memory");
        return memory[pc++];
    }

    void step() {
        if (halted) return;
        const auto opcode = fetch();
        switch (opcode) {
            case 0x10: { // LOAD address
                const auto address = fetch();
                accumulator = memory[address];
                break;
            }
            case 0x20: { // ADD address modulo 256
                const auto address = fetch();
                const unsigned wide = static_cast<unsigned>(accumulator)
                                    + memory[address];
                accumulator = static_cast<std::uint8_t>(wide & 0xFFu);
                break;
            }
            case 0x30: { // STORE address
                const auto address = fetch();
                memory[address] = accumulator;
                break;
            }
            case 0xFF:
                halted = true;
                break;
            default:
                throw std::runtime_error("invalid opcode");
        }
    }
};

int main() {
    TinyCPU cpu;
    cpu.memory[0] = 0x10; cpu.memory[1] = 0xF0;
    cpu.memory[2] = 0x20; cpu.memory[3] = 0xF1;
    cpu.memory[4] = 0x30; cpu.memory[5] = 0xF2;
    cpu.memory[6] = 0xFF;
    cpu.memory[0xF0] = 250;
    cpu.memory[0xF1] = 10;
    for (int i = 0; !cpu.halted && i < 32; ++i) cpu.step();
    if (!cpu.halted || cpu.memory[0xF2] != 4)
        throw std::runtime_error("incorrect ISA result");

    TinyCPU invalid_opcode;
    invalid_opcode.memory[0] = 0x99;
    bool rejected = false;
    try { invalid_opcode.step(); }
    catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("bad opcode was accepted");

    TinyCPU truncated;
    truncated.pc = 255;
    truncated.memory[255] = 0x10; // Requires an operand that cannot be fetched.
    rejected = false;
    try { truncated.step(); }
    catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("truncated instruction accepted");

    std::cout << "TinyCPU: output 4; invalid opcode and truncation rejected\n";
}
