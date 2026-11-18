// Project AllMight / Book 1 / Chapter 2 / NES-02.
// Focus: ownership, CPU RAM mirroring, NROM cartridge mapping, failure states.
// A teaching model, NOT a complete or cycle-accurate NES emulator.
#include "nes_course/ines_layout.hpp"
#include <array>
#include <cstdint>
#include <expected>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
    std::expected<void, BusError> insert(NromCartridge cartridge) {
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

namespace {
using namespace allmight::nes;

void require(bool true_condition, std::string_view message) {
    if (!true_condition) throw std::runtime_error(std::string(message));
}

std::vector<Byte> make_image(Byte prg_banks, Byte chr_banks, Byte flags6 = 0) {
    const auto size = header_bytes + static_cast<std::size_t>(prg_banks) * prg_bank_bytes
        + static_cast<std::size_t>(chr_banks) * chr_bank_bytes;
    std::vector<Byte> image(size, 0);
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1A;
    image[4] = prg_banks; image[5] = chr_banks; image[6] = flags6;
    return image;
}

NromCartridge load(std::span<const Byte> image) {
    auto created = NromCartridge::from_image(image);
    require(created.has_value(), "expected a supported NROM image");
    return std::move(*created);
}

void tests() {
    CpuBus bus;
    require(!bus.has_cartridge(), "default bus must own no cartridge");
    require(bus.read(0x8000).error() == BusError::no_cartridge, "missing cartridge must be explicit");
    require(bus.read(0x0001) == 0, "internal RAM should start zeroed in this model");
    require(bus.write(0x0001, 0x5A).has_value(), "internal RAM write must work");
    for (const std::uint16_t addr : std::array<std::uint16_t, 4>{0x0001, 0x0801, 0x1001, 0x1801})
        require(bus.read(addr) == 0x5A, "four 2-KiB RAM mirrors must agree");
    require(bus.write(0x1FFF, 0x71).has_value(), "last RAM mirror write");
    require(bus.read(0x07FF) == 0x71, "last RAM mirror wraps");
    require(bus.read(0x2000).error() == BusError::unimplemented_device, "PPU not yet modeled");
    require(bus.read(0x4016).error() == BusError::unimplemented_device, "controller not yet modeled");
    require(bus.read(0x5000).error() == BusError::unmapped_address, "expansion not yet modeled");

    {
        // Exhaustively check all 2,048 RAM indices in all four mirror windows.
        CpuBus sweep;
        for (std::uint16_t index = 0; index < 2048; ++index)
            require(sweep.write(index, static_cast<Byte>((index * 37U) & 0xFF)).has_value(),
                    "RAM sweep write failed");
        for (std::uint16_t address = 0; address < 0x2000; ++address)
            require(sweep.read(address) == static_cast<Byte>(((address & 0x07FF) * 37U) & 0xFF),
                    "RAM sweep mirror mismatch");
    }


    {
        auto bad = make_image(1, 1, 0x10); // mapper 1, unsupported in NROM lab
        const auto result = NromCartridge::from_image(bad);
        require(!result && result.error() == CartridgeError::unsupported_mapper, "reject mapper 1");
        require(!bus.has_cartridge(), "failed construction cannot publish partial state");
    }
    {
        auto bad = make_image(3, 1);
        const auto result = NromCartridge::from_image(bad);
        require(!result && result.error() == CartridgeError::unsupported_prg_banks, "reject 3 PRG banks");
    }
    {
        auto bad = make_image(1, 2);
        const auto result = NromCartridge::from_image(bad);
        require(!result && result.error() == CartridgeError::unsupported_chr_banks, "reject 2 CHR banks");
    }
    {
        auto bad = make_image(1, 1);
        bad.pop_back();
        const auto result = NromCartridge::from_image(bad);
        require(!result && result.error() == CartridgeError::invalid_image, "truncated CHR must be rejected");
    }
    {
        auto image = make_image(1, 1);
        image[16] = 0xA5;
        image[16 + 0x3FFF] = 0xB6;
        image[16 + prg_bank_bytes] = 0x77;
        require(bus.insert(load(image)).has_value(), "valid cartridge should insert");
        // Destroy and reuse the original image buffer: cartridge still owns
        // a private copy, never a dangling span into this input.
        image.clear(); image.shrink_to_fit();
        require(bus.read(0x8000) == 0xA5 && bus.read(0xC000) == 0xA5,
                "16 KiB NROM bank must be mirrored at 0xC000");
        require(bus.read(0xBFFF) == 0xB6 && bus.read(0xFFFF) == 0xB6,
                "last byte of mirrored PRG bank");
        require(bus.read_pattern(0) == 0x77, "CHR ROM must retain its own bytes");
        require(!bus.write_pattern(0, 0x88), "CHR ROM cannot be mutated");
        require(bus.read_pattern(0) == 0x77, "failed CHR-ROM write cannot mutate data");
        require(bus.write(0x8000, 0).error() == BusError::read_only, "PRG-ROM write must be rejected");
        require(bus.read(0x8000) == 0xA5, "PRG-ROM must remain unchanged");
    }
    {
        // Every byte in NROM-128 must agree with an independent image oracle.
        auto image = make_image(1, 1);
        for (std::size_t offset = 0; offset < prg_bank_bytes; ++offset)
            image[16 + offset] = static_cast<Byte>((offset * 19U + 7U) & 0xFF);
        CpuBus sweep;
        require(sweep.insert(load(image)).has_value(), "PRG mirror sweep insertion failed");
        for (std::size_t offset = 0; offset < prg_bank_bytes; ++offset) {
            const auto expected = static_cast<Byte>((offset * 19U + 7U) & 0xFF);
            require(sweep.read(static_cast<std::uint16_t>(0x8000U + offset)) == expected,
                    "low NROM-128 mirror mismatch");
            require(sweep.read(static_cast<std::uint16_t>(0xC000U + offset)) == expected,
                    "high NROM-128 mirror mismatch");
        }
    }
    {
        auto image = make_image(2, 1);
        image[16] = 0x41;
        image[16 + prg_bank_bytes] = 0x42;
        require(bus.insert(load(image)).has_value(), "valid cartridge should insert");
        require(bus.read(0x8000) == 0x41 && bus.read(0xC000) == 0x42,
                "32 KiB NROM has two distinct banks");
        require(bus.read(0x0001) == 0x5A, "cartridge replacement must preserve RAM");
    }
    {
        const auto image = make_image(1, 0);
        require(bus.insert(load(image)).has_value(), "valid cartridge should insert");
        require(bus.read_pattern(0) == 0, "emulated CHR RAM initialized to zero");
        require(bus.write_pattern(0x1FFF, 0x5C), "CHR RAM must be writable");
        require(bus.read_pattern(0x1FFF) == 0x5C, "CHR RAM must retain writes");
        require(!bus.write_pattern(0x2000, 0x0A), "PPU name table not in cartridge pattern RAM");
    }
    {
        const auto image = make_image(1, 0);
        auto moved = load(image);
        require(bus.insert(std::move(moved)).has_value(), "first move of ready cart works");
        const auto rejected = bus.insert(std::move(moved));
        require(!rejected && rejected.error() == BusError::invalid_cartridge,
                "moving an empty source twice must not publish broken cartridge");
        require(bus.read(0x8000).has_value(), "failed moved-from insert preserves previous cartridge");
    }
    {
        auto bad = make_image(1, 0);
        bad[0] = 'X';
        require(!NromCartridge::from_image(bad), "bad header must fail");
        require(bus.read(0x8000).has_value(), "failed replacement cannot destroy current cart");
    }
}
} // namespace

int main() {
    try {
        tests();
        std::cout << "NES-02: ownership, CPU-bus, NROM, CHR and failures passed\n";
    } catch (const std::exception& e) {
        std::cerr << "NES-02 FAILED: " << e.what() << '\n';
        return 1;
    }
}
