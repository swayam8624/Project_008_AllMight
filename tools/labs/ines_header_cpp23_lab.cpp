// Project AllMight / Book 1 / Chapter 1, milestone NES-01.
// Self-contained C++23 iNES 1.0 layout parser and negative-test laboratory.
// This code never executes untrusted bytes and needs no copyrighted ROMs.
// Compile: c++ -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Werror
//               -O2 ines_header_cpp23_lab.cpp -o ines_header_cpp23_lab
// Run:     ./ines_header_cpp23_lab

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace allmight::nes {
using Byte = std::uint8_t;
constexpr std::size_t header_bytes = 16;
constexpr std::size_t trainer_bytes = 512;
constexpr std::size_t prg_bank_bytes = 16 * 1024;
constexpr std::size_t chr_bank_bytes = 8 * 1024;

// Keep parsing failures typed: a caller must not confuse a bad file with a
// valid cartridge containing zero PRG-ROM banks.
enum class ParseError {
    short_header,
    bad_magic,
    unsupported_nes2,
    missing_prg,
    truncated_trainer,
    truncated_prg,
    truncated_chr,
};

constexpr std::string_view describe(ParseError error) noexcept {
    switch (error) {
        case ParseError::short_header: return "header shorter than 16 bytes";
        case ParseError::bad_magic: return "missing NES + 0x1A signature";
        case ParseError::unsupported_nes2: return "NES 2.0 needs a separate parser";
        case ParseError::missing_prg: return "legacy iNES image contains no PRG ROM";
        case ParseError::truncated_trainer: return "trainer bit is set but trainer is incomplete";
        case ParseError::truncated_prg: return "PRG ROM payload is incomplete";
        case ParseError::truncated_chr: return "CHR ROM payload is incomplete";
    }
    return "unknown parse failure";
}

struct Layout {
    std::uint16_t mapper{};       // High nibble of flags 7 and high nibble of flags 6.
    bool vertical_mirroring{};
    bool battery_backed{};
    bool four_screen{};
    bool has_trainer{};
    bool uses_chr_ram{};           // Zero CHR-ROM banks does NOT mean 8 KiB on disk.
    std::size_t prg_offset{};
    std::size_t prg_size{};
    std::size_t chr_offset{};
    std::size_t chr_size{};
};

// Bytes are read by their format-defined indices, not by memcpy-ing a C++
// struct whose padding, alignment and object representation are unrelated to
// the external file-format contract.
std::expected<Layout, ParseError> inspect(std::span<const Byte> image) {
    if (image.size() < header_bytes) {
        return std::unexpected(ParseError::short_header);
    }
    if (image[0] != 'N' || image[1] != 'E' || image[2] != 'S' || image[3] != 0x1A) {
        return std::unexpected(ParseError::bad_magic);
    }

    const Byte flags6 = image[6];
    const Byte flags7 = image[7];
    // NES 2.0 reinterprets additional header fields and bank sizes. It must
    // not be silently parsed using the older iNES 1.0 arithmetic.
    if ((flags7 & 0x0C) == 0x08) {
        return std::unexpected(ParseError::unsupported_nes2);
    }
    if (image[4] == 0) {
        return std::unexpected(ParseError::missing_prg);
    }

    const bool trainer = (flags6 & 0x04) != 0;
    const auto offset = header_bytes + (trainer ? trainer_bytes : 0);
    if (image.size() < offset) {
        return std::unexpected(ParseError::truncated_trainer);
    }

    // Each count is one byte, so the products and sums fit in size_t on all
    // standard-supported targets capable of storing this input buffer.
    const auto prg_size = static_cast<std::size_t>(image[4]) * prg_bank_bytes;
    const auto chr_size = static_cast<std::size_t>(image[5]) * chr_bank_bytes;
    // Subtraction after the checked offset avoids overflow-prone bound tests.
    if (image.size() - offset < prg_size) {
        return std::unexpected(ParseError::truncated_prg);
    }
    const auto chr_offset = offset + prg_size;
    if (image.size() - chr_offset < chr_size) {
        return std::unexpected(ParseError::truncated_chr);
    }

    return Layout{
        .mapper = static_cast<std::uint16_t>((flags6 >> 4) | (flags7 & 0xF0)),
        .vertical_mirroring = (flags6 & 0x01) != 0,
        .battery_backed = (flags6 & 0x02) != 0,
        .four_screen = (flags6 & 0x08) != 0,
        .has_trainer = trainer,
        .uses_chr_ram = image[5] == 0,
        .prg_offset = offset,
        .prg_size = prg_size,
        .chr_offset = chr_offset,
        .chr_size = chr_size,
    };
}

// Only 2 KiB of the CPU's $0000-$1FFF RAM address range are distinct.
// The lower 11 bits select one of 2048 physical RAM bytes.
constexpr std::uint16_t cpu_ram_index(std::uint16_t address) noexcept {
    return static_cast<std::uint16_t>(address & 0x07FF);
}
static_assert(cpu_ram_index(0x0001) == 1);
static_assert(cpu_ram_index(0x0801) == 1);
static_assert(cpu_ram_index(0x1801) == 1);

} // namespace allmight::nes

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
