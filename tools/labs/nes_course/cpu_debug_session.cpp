// AllMight / Book 1 / Pass 07: actual separately-compiled PIMPL definition.
#include "cpu_debug_session.hpp"
#include "cpu_debug_instruments.hpp"
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace allmight::nes::cpu {

struct CpuDebugSession::Impl {
    explicit Impl(CpuBus& borrowed_bus) noexcept : debugger(borrowed_bus) {}
    TracedCpu debugger;
};

namespace {
[[nodiscard]] SessionError map_cpu(CpuError error) noexcept {
    switch (error) {
        case CpuError::not_reset: return SessionError::not_reset;
        case CpuError::bus_read:
        case CpuError::bus_write: return SessionError::bus_error;
        case CpuError::unsupported_opcode: return SessionError::unsupported_opcode;
        case CpuError::counter_overflow: return SessionError::counter_overflow;
    }
    return SessionError::bus_error;
}

[[nodiscard]] StepSnapshot to_public(InstructionTrace t) noexcept {
    return {
        t.before.value,
        t.after.value,
        t.opcode,
        t.cycle_cost,
        t.total_cycles,
        t.branch_taken
    };
}
} // namespace

CpuDebugSession::CpuDebugSession(CpuBus& borrowed_bus)
    : impl_(std::make_unique<Impl>(borrowed_bus)) {}

// Both destruction and move assignment are defined after Impl is complete.
// A default deleter for an incomplete type cannot be instantiated safely at
// an arbitrary client translation unit.
CpuDebugSession::~CpuDebugSession() = default;
CpuDebugSession::CpuDebugSession(CpuDebugSession&&) noexcept = default;
CpuDebugSession& CpuDebugSession::operator=(CpuDebugSession&&) noexcept = default;

std::expected<void, SessionError> CpuDebugSession::reset() noexcept {
    if (!impl_) return std::unexpected(SessionError::moved_from);
    const auto result = impl_->debugger.reset();
    if (!result) return std::unexpected(map_cpu(result.error().kind));
    return {};
}

std::expected<StepSnapshot, SessionError> CpuDebugSession::step() noexcept {
    if (!impl_) return std::unexpected(SessionError::moved_from);
    const auto result = impl_->debugger.execute_one();
    if (!result) {
        switch (result.error().kind) {
            case DebugError::not_reset:
                return std::unexpected(SessionError::not_reset);
            case DebugError::breakpoint:
                return std::unexpected(SessionError::breakpoint);
            case DebugError::trace_full:
                return std::unexpected(SessionError::trace_full);
            case DebugError::cpu_failure:
                return std::unexpected(map_cpu(result.error().cpu.kind));
        }
    }
    return to_public(*result);
}

std::optional<StepSnapshot> CpuDebugSession::pop_trace() noexcept {
    if (!impl_) return std::nullopt;
    const auto result = impl_->debugger.pop_trace();
    if (!result) return std::nullopt;
    return to_public(*result);
}

std::optional<CpuStateSnapshot> CpuDebugSession::snapshot() const noexcept {
    if (!impl_) return std::nullopt;
    const auto& state = impl_->debugger.registers();
    return CpuStateSnapshot{
        state.a, state.x, state.y, state.sp, state.p,
        state.pc.value, impl_->debugger.cycles(),
        // Distinguish a not-yet-reset state from one ready to execute.
        impl_->debugger.cpu_ready()
    };
}

std::expected<void, SessionError>
CpuDebugSession::add_breakpoint(std::uint16_t address) noexcept {
    if (!impl_) return std::unexpected(SessionError::moved_from);
    try {
        impl_->debugger.add_breakpoint(ProgramCounter{address});
    } catch (const std::bad_alloc&) {
        return std::unexpected(SessionError::out_of_memory);
    }
    return {};
}

std::optional<std::uint16_t>
CpuDebugSession::next_breakpoint_at_or_after(std::uint16_t address) const noexcept {
    if (!impl_) return std::nullopt;
    const auto result = impl_->debugger.next_breakpoint_at_or_after(
        ProgramCounter{address});
    if (!result) return std::nullopt;
    return result->value;
}

std::expected<bool, SessionError>
CpuDebugSession::possibly_visited(std::uint16_t address) const noexcept {
    if (!impl_) return std::unexpected(SessionError::moved_from);
    return impl_->debugger.possibly_visited(ProgramCounter{address});
}

std::size_t CpuDebugSession::pending_traces() const noexcept {
    return impl_ ? impl_->debugger.pending_traces() : 0U;
}

static_assert(!std::is_copy_constructible_v<CpuDebugSession>);
static_assert(std::is_nothrow_move_constructible_v<CpuDebugSession>);
static_assert(std::is_nothrow_move_assignable_v<CpuDebugSession>);

} // namespace allmight::nes::cpu
