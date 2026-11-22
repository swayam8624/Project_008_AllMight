// PROJECT ALLMIGHT — Book 01, Chapter 04, cumulative NES-02P proof suite.
// Strict C++23. Uses NES-01 -> NES-02 -> NES-02T shared code, not a new emulator.
// No intentional UB, no copyrighted ROMs, no GUI, no fake claims of cycle accuracy.
// Negative tests are designed to *avoid* invoking a dangling pointer/reference.
#include "nes_course/cartridge_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <expected>
#include <iostream>
#include <memory>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;

void require(bool condition, std::string_view reason) {
    if (!condition) throw std::runtime_error(std::string(reason));
}

struct InjectedFault : std::exception {
    const char* what() const noexcept override { return "deterministic construction fault"; }
};

// The event recorder needs no allocation. This avoids accidentally making a
// logging allocation part of the very constructor/destructor proof we observe.
struct Events {
    std::array<int, 32> sequence{};
    std::size_t used{};

    void record(int event) noexcept {
        if (used >= sequence.size()) std::terminate();
        sequence[used++] = event;
    }
    void equals(std::initializer_list<int> required, std::string_view message) const {
        require(required.size() == used, message);
        std::size_t i = 0;
        for (const int event : required)
            require(sequence[i++] == event, message);
    }
};

// Constructor event = positive id, destructor event = negative id.
// A throwing constructor records 100 + id, but no object has completed
// construction and so *that object's destructor must not run*.
struct Part {
    Events* log{};
    int id{};

    Part(Events& events, int part_id, bool fail) : log(&events), id(part_id) {
        if (fail) {
            log->record(100 + id);
            throw InjectedFault{};
        }
        log->record(id);
    }
    ~Part() noexcept { log->record(-id); }
    Part(const Part&) = delete;
    Part& operator=(const Part&) = delete;
};

struct MachineAssembly {
    Events* log{};
    Part first;
    Part second;

    MachineAssembly(Events& events, bool fail_second)
        : log(&events), first(events, 1, false), second(events, 2, fail_second) {
        events.record(20); // full constructor body reached
    }
    ~MachineAssembly() noexcept { log->record(30); }
    MachineAssembly(const MachineAssembly&) = delete;
    MachineAssembly& operator=(const MachineAssembly&) = delete;
};

// The two tests prove *observable* order, but the rule itself comes from
// the language specification: fully constructed members unwind in reverse.
void prove_construction_and_unwinding() {
    {
        Events events;
        bool caught = false;
        try {
            MachineAssembly assembly(events, true);
        } catch (const InjectedFault&) {
            caught = true;
        }
        require(caught, "second member must inject an exception");
        // First constructed; second construction attempted and failed;
        // first destroyed. No ~MachineAssembly and no ~second.
        events.equals({1, 102, -1}, "partial-construction cleanup order");
    }
    {
        Events events;
        {
            MachineAssembly assembly(events, false);
            events.equals({1, 2, 20}, "successful construction order");
        }
        events.equals({1, 2, 20, 30, -2, -1},
                      "complete object and member destruction order");
    }
}

// Ownership is exactly one release duty. Moving an owner transfers this duty;
// merely applying std::move produces an xvalue expression, not a transfer.
struct ReleaseCounter { int acquired{}, released{}; };

struct UniqueTicket {
    ReleaseCounter* counter{};
    bool active{};

    explicit UniqueTicket(ReleaseCounter& stats) noexcept
        : counter(&stats), active(true) { ++counter->acquired; }

    ~UniqueTicket() noexcept { reset(); }
    UniqueTicket(const UniqueTicket&) = delete;
    UniqueTicket& operator=(const UniqueTicket&) = delete;

    UniqueTicket(UniqueTicket&& other) noexcept
        : counter(std::exchange(other.counter, nullptr)),
          active(std::exchange(other.active, false)) {}

    UniqueTicket& operator=(UniqueTicket&& other) noexcept {
        if (this != &other) {
            reset();
            counter = std::exchange(other.counter, nullptr);
            active = std::exchange(other.active, false);
        }
        return *this;
    }
    void reset() noexcept {
        if (active) {
            ++counter->released;
            active = false;
            counter = nullptr;
        }
    }
};

static_assert(!std::is_copy_constructible_v<UniqueTicket>);
static_assert(std::is_nothrow_move_constructible_v<UniqueTicket>);
static_assert(!std::is_copy_constructible_v<NromCartridge>);
static_assert(std::is_nothrow_move_constructible_v<NromCartridge>);

void prove_move_permission_vs_transfer() {
    ReleaseCounter stats;
    {
        UniqueTicket first(stats);
        auto&& alias = std::move(first); // binds a reference; no move ctor yet
        require(first.active, "std::move alone cannot release or transfer anything");
        require(stats.acquired == 1 && stats.released == 0,
                "reference binding cannot create or destroy an owner");

        UniqueTicket second(std::move(alias)); // now move construction happens
        require(!first.active && second.active, "move construction transfers duty");
        require(stats.released == 0, "resource must still be active after transfer");

        UniqueTicket third(std::move(second));
        require(!second.active && third.active, "second transfer must preserve one owner");
    }
    require(stats.acquired == 1 && stats.released == 1,
            "three owner objects must release one resource exactly once");
}

// optional<T>::emplace replaces the existing value by destruction first.
// A throwing new constructor can leave optional disengaged; that is legal.
// Do not teach std::optional::emplace as a universal strong-guarantee commit.
struct RiskyConstruct {
    int* destruction_count{};
    explicit RiskyConstruct(int& destroyed, bool fail)
        : destruction_count(&destroyed) {
        if (fail) throw InjectedFault{};
    }
    ~RiskyConstruct() noexcept { ++*destruction_count; }
};

void prove_optional_replacement_counterexample() {
    int destroyed = 0;
    std::optional<RiskyConstruct> value(std::in_place, destroyed, false);
    bool caught = false;
    try {
        value.emplace(destroyed, true);
    } catch (const InjectedFault&) {
        caught = true;
    }
    require(caught, "attempted replacement must fail");
    require(!value.has_value(), "failed emplace may leave old value destroyed");
    require(destroyed == 1, "old optional value should be destroyed once");
}

// This fixture uses only synthetic, inert ROM bytes. Payload data is taken
// from valid NES-01 layout arithmetic; the actual code under test is the
// *shared* NROM parser, bus, and transaction runtime.
std::vector<Byte> make_image(Byte prg_banks, Byte chr_banks, Byte flags6 = 0) {
    const auto bytes = header_bytes
                     + static_cast<std::size_t>(prg_banks) * prg_bank_bytes
                     + static_cast<std::size_t>(chr_banks) * chr_bank_bytes;
    std::vector<Byte> result(bytes, 0);
    result[0] = 'N'; result[1] = 'E'; result[2] = 'S'; result[3] = 0x1A;
    result[4] = prg_banks;
    result[5] = chr_banks;
    result[6] = flags6;
    return result;
}

struct StateWitness {
    std::uint64_t revision{};
    Byte prg_lo{};
    Byte prg_hi{};
    Byte mirrored_ram{};
    Byte chr_first{};
};

StateWitness observe(const CartridgeRuntime& runtime) {
    const auto low = runtime.bus().read(0x8000);
    const auto high = runtime.bus().read(0xC000);
    const auto ram = runtime.bus().read(0x1001);
    const auto pattern = runtime.bus().read_pattern(0);
    require(low && high && ram && pattern, "live NROM state must be fully observable");
    return {runtime.generation(), *low, *high, *ram, *pattern};
}

bool equal(const StateWitness& a, const StateWitness& b) noexcept {
    return a.revision == b.revision &&
           a.prg_lo == b.prg_lo && a.prg_hi == b.prg_hi &&
           a.mirrored_ram == b.mirrored_ram && a.chr_first == b.chr_first;
}

// The positive/negative proof operates against real previous-milestone code,
// not a reimplemented model of what that code is supposed to do.
void prove_borrowed_file_and_transactional_commit() {
    CartridgeRuntime runtime;
    auto file = make_image(1, 0);
    file[header_bytes] = 0x31;
    require(runtime.replace(std::span<const Byte>(file)).has_value(),
            "first NROM mount must succeed");
    file.clear();
    file.shrink_to_fit(); // original buffer's old views may now dangle
    // Never read from any old span. The runtime owns a deep copy.
    require(runtime.bus().read(0x8000) == 0x31,
            "mounted program must outlive external file storage");

    require(runtime.write_ram(0x0001, 0x7B).has_value(),
            "CPU RAM write failed");
    require(runtime.write_pattern(0, 0x55), "CHR RAM write failed");
    const auto old = observe(runtime);

    auto new_game = make_image(2, 1);
    new_game[header_bytes] = 0x61;
    new_game[header_bytes + prg_bank_bytes] = 0x72;
    new_game[header_bytes + 2 * prg_bank_bytes] = 0x83;

    for (const auto fault : {TestFault::before_staging, TestFault::after_staging,
                             TestFault::throw_bad_alloc_after_staging}) {
        const auto attempt = runtime.replace(new_game, fault);
        require(!attempt, "injected preparation failure must not commit");
        require(equal(observe(runtime), old),
                "failure must preserve old cartridge and RAM exactly");
    }
    auto corrupted = new_game;
    corrupted[0] = 0;
    require(!runtime.replace(corrupted), "corrupt magic must be rejected");
    require(equal(observe(runtime), old), "format rejection must preserve state");

    const auto succeeded = runtime.replace(new_game);
    require(succeeded.has_value() && *succeeded == old.revision + 1,
            "successful mount must advance revision by one");
    const auto now = observe(runtime);
    require(now.prg_lo == 0x61 && now.prg_hi == 0x72,
            "two 16-KiB banks must map to distinct CPU regions");
    require(now.chr_first == 0x83, "CHR ROM must replace old CHR RAM");
    require(now.mirrored_ram == old.mirrored_ram,
            "this teaching model preserves internal RAM by policy");
    require(!runtime.write_pattern(0, 0xFE),
            "new CHR ROM must be read-only after successful commit");
}

// A weak observer must not extend a record's lifetime. This is a separate
// library experiment; the running NROM bus does NOT need shared_ptr merely
// because the chapter discusses it.
void prove_weak_observer_not_owner() {
    std::weak_ptr<const int> observer;
    {
        auto owner = std::make_shared<const int>(9);
        observer = owner;
        const auto pinned = observer.lock();
        require(pinned && *pinned == 9, "weak lock while owner alive");
    }
    require(!observer.lock(), "expired weak observer cannot resurrect a resource");
}

} // namespace

int main() {
    try {
        prove_construction_and_unwinding();
        prove_move_permission_vs_transfer();
        prove_optional_replacement_counterexample();
        prove_borrowed_file_and_transactional_commit();
        prove_weak_observer_not_owner();
        std::cout << "NES-02P: all five ownership/transaction proof groups passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "NES-02P FAILED: " << error.what() << '\n';
        return 1;
    }
}
