#pragma once
// C++23 reusable teaching system shared by NES-02 and subsequent milestones.
#include "ines_layout.hpp"
#include <array>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>
#include <vector>
#include <type_traits>

namespace allmight::nes {

enum class CartridgeError { invalid_image, unsupported_mapper, unsupported_prg_banks,
                            unsupported_chr_banks };
enum class BusError { no_cartridge, unimplemented_device, unmapped_address, read_only, invalid_cartridge };

// Own the exact bytes we are allowed to read. No spans into the original
// caller-owned image are retained, so an image vector can be destroyed.
class NromCartridge {
public:
    NromCartridge(const NromCartridge&) = delete;
    NromCartridge& operator=(const NromCartridge&) = delete;
    NromCartridge(NromCartridge&&) noexcept = default;
    NromCartridge& operator=(NromCartridge&&) noexcept = default;

    static std::expected<NromCartridge, CartridgeError>
    from_image(std::span<const Byte> bytes) {
        const auto layout = inspect(bytes); // NES-01's trusted layout parser.
        if (!layout) return std::unexpected(CartridgeError::invalid_image);
        if (layout->mapper != 0)
            return std::unexpected(CartridgeError::unsupported_mapper);
        if (layout->prg_size != prg_bank_bytes && layout->prg_size != 2 * prg_bank_bytes)
            return std::unexpected(CartridgeError::unsupported_prg_banks);
        if (layout->chr_size != 0 && layout->chr_size != chr_bank_bytes)
            return std::unexpected(CartridgeError::unsupported_chr_banks);

        // The parser proved these subspans are in bounds. Copying moves the
        // external file bytes into independent, owned cartridge storage.
        const auto prg = bytes.subspan(layout->prg_offset, layout->prg_size);
        std::vector<Byte> program(prg.begin(), prg.end());
        std::vector<Byte> pattern;
        if (layout->uses_chr_ram) {
            pattern.resize(chr_bank_bytes, 0); // emulated CHR RAM, absent on disk
        } else {
            const auto chr = bytes.subspan(layout->chr_offset, layout->chr_size);
            pattern.assign(chr.begin(), chr.end());
        }
        return NromCartridge(std::move(program), std::move(pattern), layout->uses_chr_ram);
    }

    // Only addresses 0x8000..0xFFFF are supplied here; the bus owns dispatch.
    std::optional<Byte> read_prg(std::uint16_t address) const noexcept {
        if (address < 0x8000 || prg_.empty()) return std::nullopt;
        const auto offset = static_cast<std::size_t>(address - 0x8000);
        // NROM-128 mirrors the same 16 KiB twice. NROM-256 has 32 KiB.
        return prg_[offset % prg_.size()];
    }

    // Only pattern-table 0x0000..0x1FFF is modeled. Name tables/palette live
    // behind another PPU address decoder; not in this cartridge laboratory.
    std::optional<Byte> read_chr(std::uint16_t address) const noexcept {
        if (address >= 0x2000 || chr_.size() != chr_bank_bytes) return std::nullopt;
        return chr_[address];
    }
    bool write_chr(std::uint16_t address, Byte value) noexcept {
        if (!chr_ram_ || address >= 0x2000 || chr_.size() != chr_bank_bytes) return false;
        chr_[address] = value;
        return true;
    }
    bool uses_chr_ram() const noexcept { return chr_ram_; }
    bool ready() const noexcept {
        return (prg_.size() == prg_bank_bytes || prg_.size() == 2 * prg_bank_bytes)
            && chr_.size() == chr_bank_bytes;
    }

private:
    NromCartridge(std::vector<Byte> prg, std::vector<Byte> chr, bool chr_ram)
        : prg_(std::move(prg)), chr_(std::move(chr)), chr_ram_(chr_ram) {}
    std::vector<Byte> prg_;
    std::vector<Byte> chr_;
    bool chr_ram_{};
};

// The CPU bus owns its 2 KiB RAM and optionally one complete cartridge.
// Unimplemented device reads are explicit errors; returning zero would
// falsely imply an accurate NES open-bus/electrical behavior.
class CpuBus {
public:
    std::expected<void, BusError> insert(NromCartridge cartridge) noexcept {
        if (!cartridge.ready()) return std::unexpected(BusError::invalid_cartridge);
        cartridge_.emplace(std::move(cartridge));
        return {};
    }
    bool has_cartridge() const noexcept { return cartridge_.has_value(); }

    std::expected<Byte, BusError> read(std::uint16_t address) const noexcept {
        if (address <= 0x1FFF) return ram_[cpu_ram_index(address)];
        if (address <= 0x401F) return std::unexpected(BusError::unimplemented_device);
        if (address < 0x8000) return std::unexpected(BusError::unmapped_address);
        if (!cartridge_) return std::unexpected(BusError::no_cartridge);
        const auto value = cartridge_->read_prg(address);
        if (!value) return std::unexpected(BusError::no_cartridge);
        return *value;
    }
    std::expected<void, BusError> write(std::uint16_t address, Byte value) noexcept {
        if (address <= 0x1FFF) {
            ram_[cpu_ram_index(address)] = value;
            return {};
        }
        if (address <= 0x401F) return std::unexpected(BusError::unimplemented_device);
        if (address < 0x8000) return std::unexpected(BusError::unmapped_address);
        if (!cartridge_ || !cartridge_->read_prg(address))
            return std::unexpected(BusError::no_cartridge);
        // NROM PRG-ROM is immutable. A diagnostic error here is our API's
        // policy, not a claim that the physical CPU signals a write error.
        return std::unexpected(BusError::read_only);
    }

    std::optional<Byte> read_pattern(std::uint16_t address) const noexcept {
        return cartridge_ ? cartridge_->read_chr(address) : std::nullopt;
    }
    bool write_pattern(std::uint16_t address, Byte value) noexcept {
        return cartridge_ && cartridge_->write_chr(address, value);
    }
private:
    std::array<Byte, 2048> ram_{};
    std::optional<NromCartridge> cartridge_;
};

} // namespace allmight::nes

static_assert(std::is_nothrow_move_constructible_v<allmight::nes::NromCartridge>);
