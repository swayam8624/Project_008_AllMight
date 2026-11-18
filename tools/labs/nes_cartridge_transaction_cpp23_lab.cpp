// Project AllMight — Book 01, Chapter 03: Records as Resource Transactions
// NES-02T: a cumulative cartridge transaction milestone, NOT a new emulator.
// Uses exactly the NES-01 parser and NES-02 bus / NROM core.
//
// Build/run via the Book01 CMake/CTest project. No ROMs or GUI required.
//
// Core contract: each failed load leaves the old cartridge, generation,
// CPU RAM contents and writable CHR RAM unchanged. A successful load commits
// one fully validated and owned cartridge, then advances the generation.

#include "nes_course/nes_bus_nrom.hpp"
#include <array>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <new>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

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

namespace {
using namespace allmight::nes;

void check(bool condition, std::string_view why) {
    if (!condition) throw std::runtime_error(std::string(why));
}

// A synthetic ROM fixture; the user doesn't need a copyrighted game ROM.
// The payload size and field meanings come from the real NES-01 parser.
std::vector<Byte> image(Byte prg_count, Byte chr_count,
                        Byte flags6 = 0, Byte flags7 = 0) {
    const std::size_t count =
        header_bytes + static_cast<std::size_t>(prg_count) * prg_bank_bytes
                     + static_cast<std::size_t>(chr_count) * chr_bank_bytes;
    std::vector<Byte> data(count, 0);
    data[0] = 'N'; data[1] = 'E'; data[2] = 'S'; data[3] = 0x1A;
    data[4] = prg_count; data[5] = chr_count; data[6] = flags6; data[7] = flags7;
    return data;
}

// Snapshot values, not pointers. The old bus state is compared after each
// unsuccessful operation. This makes the strong guarantee observable.
struct Witness {
    std::uint64_t generation{};
    Byte program_start{};
    Byte program_end{};
    Byte ram_mirror{};
    Byte pattern_start{};
    Byte pattern_end{};
};

Witness witness(const CartridgeRuntime& runtime) {
    const CpuBus& bus = runtime.bus();
    const auto start = bus.read(0x8000);
    const auto end = bus.read(0xFFFF);
    const auto ram = bus.read(0x1801);
    const auto chr_start = bus.read_pattern(0);
    const auto chr_end = bus.read_pattern(0x1FFF);
    check(start && end && ram && chr_start && chr_end, "invalid witness state");
    return {runtime.generation(), *start, *end, *ram, *chr_start, *chr_end};
}

bool same(const Witness& a, const Witness& b) {
    return a.generation == b.generation &&
           a.program_start == b.program_start &&
           a.program_end == b.program_end &&
           a.ram_mirror == b.ram_mirror &&
           a.pattern_start == b.pattern_start &&
           a.pattern_end == b.pattern_end;
}

void exercise() {
    CartridgeRuntime runtime;
    check(runtime.generation() == 0, "initial generation must be zero");
    check(!runtime.bus().has_cartridge(), "initial cartridge must be absent");
    check(runtime.bus().read(0x8000).error() == BusError::no_cartridge,
          "initial program read must fail explicitly");

    auto first = image(1, 0);    // 16 KiB PRG, 8 KiB runtime CHR-RAM
    first[header_bytes] = 0x3A;
    first[header_bytes + prg_bank_bytes - 1] = 0x91;
    const auto first_commit = runtime.replace(first);
    check(first_commit.has_value() && *first_commit == 1, "first commit must produce generation one");

    // Prove that the runtime does not borrow the original input storage.
    first.clear();
    first.shrink_to_fit();
    check(runtime.write_ram(0x0001, 0x51).has_value(), "RAM store failed");
    check(runtime.write_pattern(0, 0x22), "CHR-RAM write failed");
    check(runtime.write_pattern(0x1FFF, 0x6D), "CHR-RAM end write failed");
    const Witness stable = witness(runtime);
    check(stable.program_start == 0x3A, "initial PRG data lost");
    check(stable.program_end == 0x91, "initial PRG end lost");
    check(stable.ram_mirror == 0x51, "mirrored RAM should retain write");
    check(stable.pattern_start == 0x22 && stable.pattern_end == 0x6D,
          "CHR-RAM edits should persist");

    // Unsupported formats and devices must fail without publication.
    auto bad_signature = image(1, 0);
    bad_signature[0] = 0x00;
    auto wrong_mapper = image(1, 0, 0x10);
    auto wrong_prg = image(3, 0);
    auto wrong_chr = image(1, 2);
    auto short_image = image(1, 1);
    short_image.pop_back();

    const std::array<std::pair<std::span<const Byte>, LoadError>, 5> errors{{
        {bad_signature, LoadError::invalid_image},
        {wrong_mapper, LoadError::unsupported_mapper},
        {wrong_prg, LoadError::unsupported_prg_banks},
        {wrong_chr, LoadError::unsupported_chr_banks},
        {short_image, LoadError::invalid_image}
    }};
    for (const auto& [bytes, expected] : errors) {
        const auto attempt = runtime.replace(bytes);
        check(!attempt && attempt.error() == expected, "typed failure mismatch");
        check(same(witness(runtime), stable), "validation error changed live state");
    }

    auto second = image(2, 1); // two distinct 16 KiB PRG banks + CHR-ROM
    second[header_bytes] = 0xA4;
    second[header_bytes + prg_bank_bytes] = 0xB5;
    second[header_bytes + 2 * prg_bank_bytes - 1] = 0xC6;
    second[header_bytes + 2 * prg_bank_bytes] = 0xD7;

    for (const auto fault : {TestFault::before_staging, TestFault::after_staging,
                             TestFault::throw_bad_alloc_after_staging}) {
        const auto attempted = runtime.replace(second, fault);
        const auto expected = fault == TestFault::throw_bad_alloc_after_staging
                            ? LoadError::allocation_failure : LoadError::injected_failure;
        check(!attempted && attempted.error() == expected, "injected failure mismatch");
        check(same(witness(runtime), stable), "failed staged load changed state");
    }

    const auto committed = runtime.replace(second);
    check(committed.has_value() && *committed == 2, "only successful replacements advance generation");
    check(runtime.bus().read(0x8000) == 0xA4, "new first bank not committed");
    check(runtime.bus().read(0xC000) == 0xB5, "new second bank not committed");
    check(runtime.bus().read(0xFFFF) == 0xC6, "new last PRG byte wrong");
    check(runtime.bus().read_pattern(0) == 0xD7, "new CHR-ROM not committed");
    check(!runtime.write_pattern(0, 0x31), "CHR-ROM must remain immutable");
    check(runtime.bus().read(0x1801) == 0x51,
          "CPU RAM preservation policy must remain explicit");

    // Repetition catches accidental generation updates and corrupted state.
    const Witness new_stable = witness(runtime);
    for (int i = 0; i < 128; ++i) {
        const auto attempted = runtime.replace(second, TestFault::after_staging);
        check(!attempted && attempted.error() == LoadError::injected_failure,
              "repeated failure should not succeed");
        check(same(witness(runtime), new_stable),
              "repeated failure modified data or generation");
    }
}
} // namespace

int main() {
    try {
        exercise();
        std::cout << "NES-02T: prepare/commit, rollback, typed errors, lifetime and generation passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "NES-02T FAILED: " << e.what() << '\n';
        return 1;
    }
}
