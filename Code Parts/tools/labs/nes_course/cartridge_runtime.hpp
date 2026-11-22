#pragma once
// The same C++23 owned transaction runtime is reused by NES-02T and NES-04.
// No separate, drifting reimplementations of the cartridge state machine.
#include "nes_bus_nrom.hpp"
#include <cstdint>
#include <expected>
#include <limits>
#include <new>
#include <span>
#include <type_traits>
#include <utility>

namespace allmight::nes {

enum class LoadError {
    invalid_image,
    unsupported_mapper,
    unsupported_prg_banks,
    unsupported_chr_banks,
    invalid_candidate,
    allocation_failure,
    injected_failure,
    generation_exhausted
};

// Test hooks are explicit arguments, not hidden global state. An artificially
// thrown std::bad_alloc exercises our exception-to-result boundary. It is NOT
// a claim that the operating system actually ran out of memory.
enum class TestFault {
    none,
    before_staging,
    after_staging,
    throw_bad_alloc_after_staging
};

constexpr LoadError translate(CartridgeError cause) noexcept {
    switch (cause) {
        case CartridgeError::invalid_image: return LoadError::invalid_image;
        case CartridgeError::unsupported_mapper: return LoadError::unsupported_mapper;
        case CartridgeError::unsupported_prg_banks: return LoadError::unsupported_prg_banks;
        case CartridgeError::unsupported_chr_banks: return LoadError::unsupported_chr_banks;
    }
    return LoadError::invalid_image;
}

// Deliberately noncopyable: this runtime is the single owner of its bus and
// optional cartridge. No one can retain a borrowing reference to its PRG
// storage through this public interface.
class CartridgeRuntime {
public:
    CartridgeRuntime() = default;
    CartridgeRuntime(const CartridgeRuntime&) = delete;
    CartridgeRuntime& operator=(const CartridgeRuntime&) = delete;

    std::expected<std::uint64_t, LoadError>
    replace(std::span<const Byte> external_file, TestFault test_fault = TestFault::none) {
        // A wrapping revision number could make an old revision equal to a
        // later one. Refuse that rather than issuing ambiguous identities.
        if (generation_ == std::numeric_limits<std::uint64_t>::max()) {
            return std::unexpected(LoadError::generation_exhausted);
        }
        if (test_fault == TestFault::before_staging) {
            return std::unexpected(LoadError::injected_failure);
        }

        try {
            // PREPARE: validate an untrusted borrowed file and construct an
            // independently owned cartridge. The live bus is not touched.
            auto candidate = NromCartridge::from_image(external_file);
            if (!candidate) {
                return std::unexpected(translate(candidate.error()));
            }

            // A deterministic injected failure is different from a malformed
            // image: it tests whether partially completed work leaks out.
            if (test_fault == TestFault::after_staging) {
                return std::unexpected(LoadError::injected_failure);
            }
            if (test_fault == TestFault::throw_bad_alloc_after_staging) {
                throw std::bad_alloc{};
            }

            // COMMIT: the checked NROM object is moved into the pre-existing
            // bus. Both its vector moves and the bus's optional emplacement
            // are non-throwing for this concrete C++23 configuration.
            // No method exposes a stored borrow into the candidate.
            const auto committed = bus_.insert(std::move(*candidate));
            if (!committed) {
                return std::unexpected(LoadError::invalid_candidate);
            }

            // This is the sole write to generation_. It must follow commit.
            // It also cannot overflow due to the check above.
            return ++generation_;
        } catch (const std::bad_alloc&) {
            // std::vector construction can throw before COMMIT. All temporary
            // owners are destroyed by RAII during unwinding; the live bus
            // and generation have not changed.
            return std::unexpected(LoadError::allocation_failure);
        }
    }

    [[nodiscard]] const CpuBus& bus() const noexcept { return bus_; }
    [[nodiscard]] std::expected<void, BusError> write_ram(std::uint16_t address, Byte byte)
        noexcept {
        // Deliberately narrow mutation access: this chapter tests internal RAM
        // persistence but does not add unrestricted cartridge mutation APIs.
        if (address > 0x1FFF) return std::unexpected(BusError::unmapped_address);
        return bus_.write(address, byte);
    }
    [[nodiscard]] bool write_pattern(std::uint16_t address, Byte byte) noexcept {
        return bus_.write_pattern(address, byte);
    }
    [[nodiscard]] std::uint64_t generation() const noexcept { return generation_; }

private:
    CpuBus bus_{};
    std::uint64_t generation_{};
};

static_assert(std::is_nothrow_move_constructible_v<NromCartridge>);

} // namespace allmight::nes
