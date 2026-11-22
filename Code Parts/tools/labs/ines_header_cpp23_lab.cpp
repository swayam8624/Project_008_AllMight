// Project AllMight / Book 1 / Chapter 1, milestone NES-01.
// Self-contained C++23 iNES 1.0 layout parser and negative-test laboratory.
// This code never executes untrusted bytes and needs no copyrighted ROMs.
// Compile: c++ -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Werror
//               -O2 ines_header_cpp23_lab.cpp -o ines_header_cpp23_lab
// Run:     ./ines_header_cpp23_lab

#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "nes_course/ines_layout.hpp"


namespace {
using namespace allmight::nes;
void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

// Construct a valid synthetic image with inert zero-filled payloads.
std::vector<Byte> image(Byte prg, Byte chr, Byte flags6 = 0, Byte flags7 = 0) {
    const auto payload_size = static_cast<std::size_t>(prg) * prg_bank_bytes
                            + static_cast<std::size_t>(chr) * chr_bank_bytes
                            + (((flags6 & 0x04) != 0) ? trainer_bytes : 0);
    std::vector<Byte> bytes(header_bytes + payload_size, 0);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1A;
    bytes[4] = prg; bytes[5] = chr; bytes[6] = flags6; bytes[7] = flags7;
    return bytes;
}

void check_error(std::span<const Byte> bytes, ParseError expected) {
    const auto result = inspect(bytes);
    require(!result.has_value(), "expected parsing failure");
    require(result.error() == expected, describe(expected));
}

void run_tests() {
    { // Normal iNES: one 16-KiB PRG bank, one 8-KiB CHR bank.
        const auto bytes = image(1, 1);
        const auto layout = inspect(bytes);
        require(layout.has_value(), "valid image rejected");
        require(layout->prg_offset == 16, "bad PRG start");
        require(layout->chr_offset == 16 + 16384, "bad CHR start");
        require(layout->chr_size == 8192, "bad CHR size");
    }
    { // Flags: trainer, vertical mirroring, battery, mapper number 0x21.
        const auto bytes = image(2, 1, 0x17, 0x20);
        const auto layout = inspect(bytes);
        require(layout.has_value(), "trainer image rejected");
        require(layout->mapper == 0x21, "mapper nibble extraction wrong");
        require(layout->vertical_mirroring && layout->battery_backed, "flags lost");
        require(layout->prg_offset == 16 + 512, "trainer wasn't skipped");
    }
    { // A zero CHR-ROM count means CHR-RAM is supplied by the emulator.
        const auto bytes = image(1, 0);
        const auto layout = inspect(bytes);
        require(layout.has_value() && layout->uses_chr_ram, "CHR-RAM case rejected");
        require(layout->chr_size == 0, "invented on-disk CHR bytes");
    }
    {
        auto bytes = image(1, 1);
        bytes[0] = 'X';
        check_error(bytes, ParseError::bad_magic);
    }
    {
        auto bytes = image(1, 1);
        bytes.resize(15);
        check_error(bytes, ParseError::short_header);
    }
    {
        auto bytes = image(1, 0, 0x04);
        bytes.resize(16 + 511);
        check_error(bytes, ParseError::truncated_trainer);
    }
    {
        auto bytes = image(1, 1);
        bytes.resize(16 + 16384 - 1);
        check_error(bytes, ParseError::truncated_prg);
    }
    {
        auto bytes = image(1, 1);
        bytes.resize(16 + 16384 + 8192 - 1);
        check_error(bytes, ParseError::truncated_chr);
    }
    {
        const auto bytes = image(1, 0, 0, 0x08);
        check_error(bytes, ParseError::unsupported_nes2);
    }
    {
        auto bytes = image(0, 0);
        check_error(bytes, ParseError::missing_prg);
    }
    {
        const auto bytes = image(1, 1, 0x08);
        const auto layout = inspect(bytes);
        require(layout.has_value() && layout->four_screen, "four-screen flag lost");
    }
}
} // namespace

int main() {
    try {
        run_tests();
        std::cout << "NES-01: 11 C++23 synthetic test cases passed\n";
    } catch (const std::exception& error) {
        std::cerr << "NES-01 FAILED: " << error.what() << '\n';
        return 1;
    }
}
