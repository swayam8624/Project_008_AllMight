#pragma once
// AllMight / Book 1 / Pass 07: separately compiled public CPU debugger API.
//
// Header clients need not see the CPU interpreter, its ring, the visited-PC
// Bloom filter, ordered breakpoints or their storage representations.
// The implementation is kept in cpu_debug_session.cpp (real PIMPL).
// This interface returns owned small snapshots; no internal pointers leak.
//
// NOT a stable cross-compiler C ABI. It is a normal C++23 build boundary.
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>

namespace allmight::nes {
class CpuBus; // borrowed lifetime; must outlive CpuDebugSession
}

namespace allmight::nes::cpu {

struct CpuStateSnapshot {
    std::uint8_t a{}, x{}, y{}, sp{}, p{};
    std::uint16_t pc{};
    std::uint64_t total_cycles{};
    bool ready{};
    friend bool operator==(CpuStateSnapshot, CpuStateSnapshot) = default;
};
struct StepSnapshot {
    std::uint16_t before{}, after{};
    std::uint8_t opcode{}, cycles{};
    std::uint64_t total_cycles{};
    bool branch_taken{};
    friend bool operator==(StepSnapshot, StepSnapshot) = default;
};
enum class SessionError {
    moved_from,
    not_reset,
    bus_error,
    unsupported_opcode,
    counter_overflow,
    breakpoint,
    trace_full,
    out_of_memory
};

class CpuDebugSession {
public:
    explicit CpuDebugSession(CpuBus& borrowed_bus);
    ~CpuDebugSession();
    CpuDebugSession(const CpuDebugSession&) = delete;
    CpuDebugSession& operator=(const CpuDebugSession&) = delete;
    CpuDebugSession(CpuDebugSession&&) noexcept;
    CpuDebugSession& operator=(CpuDebugSession&&) noexcept;

    [[nodiscard]] std::expected<void, SessionError> reset() noexcept;
    [[nodiscard]] std::expected<StepSnapshot, SessionError> step() noexcept;
    [[nodiscard]] std::optional<StepSnapshot> pop_trace() noexcept;
    [[nodiscard]] std::optional<CpuStateSnapshot> snapshot() const noexcept;
    [[nodiscard]] std::expected<void, SessionError>
    add_breakpoint(std::uint16_t address) noexcept;
    [[nodiscard]] std::optional<std::uint16_t>
    next_breakpoint_at_or_after(std::uint16_t address) const noexcept;
    [[nodiscard]] std::expected<bool, SessionError>
    possibly_visited(std::uint16_t address) const noexcept;
    [[nodiscard]] std::size_t pending_traces() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace allmight::nes::cpu
