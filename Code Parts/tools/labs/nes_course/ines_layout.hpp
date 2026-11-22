#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

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
inline std::expected<Layout, ParseError> inspect(std::span<const Byte> image) {
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
