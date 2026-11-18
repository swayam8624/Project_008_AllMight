#pragma once
// AllMight / Book 1 / Pass 07: data structures earned by a 6502 debugger.
// These are CPU-only, single-threaded learning instruments. Never mistake
// a Bloom hit for membership proof or a trace buffer for a lock-free queue.
#include "cpu6502_subset.hpp"
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::nes::cpu {

enum class TraceError { full, empty };
template<class T, std::size_t Capacity>
class TraceRing {
    static_assert(Capacity > 0);
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_nothrow_destructible_v<T>);
public:
    [[nodiscard]] bool full() const noexcept { return count_ == Capacity; }
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }
    [[nodiscard]] std::size_t size() const noexcept { return count_; }
    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }

    [[nodiscard]] std::expected<void, TraceError> push(T item) noexcept {
        if (full()) return std::unexpected(TraceError::full);
        const auto tail = (head_ + count_) % Capacity;
        cells_[tail].emplace(std::move(item));
        ++count_;
        return {};
    }

    [[nodiscard]] std::expected<T, TraceError> pop() noexcept {
        if (empty()) return std::unexpected(TraceError::empty);
        T result = std::move(*cells_[head_]);
        cells_[head_].reset();
        head_ = (head_ + 1) % Capacity;
        --count_;
        return result;
    }

    void clear() noexcept {
        while (!empty()) {
            // pop cannot fail while !empty. All T moves are noexcept.
            (void)pop();
        }
    }
private:
    std::array<std::optional<T>,Capacity> cells_{};
    std::size_t head_{};
    std::size_t count_{};
};

// A Bloom filter is a *negative* prefilter: if the query is false, the PC
// was never recorded since clear(). If true, an exact lookup may still be
// necessary. No deletions are supported, so no false negatives are introduced.
class VisitedPcBloom {
public:
    void record(ProgramCounter pc) noexcept {
        bits_.set(hash1(pc.value));
        bits_.set(hash2(pc.value));
    }
    [[nodiscard]] bool possibly_visited(ProgramCounter pc) const noexcept {
        return bits_.test(hash1(pc.value)) && bits_.test(hash2(pc.value));
    }
    void clear() noexcept { bits_.reset(); }
private:
    [[nodiscard]] static std::size_t hash1(std::uint16_t pc) noexcept {
        return (static_cast<std::uint32_t>(pc) * 73U + 17U) % 2048U;
    }
    [[nodiscard]] static std::size_t hash2(std::uint16_t pc) noexcept {
        return ((static_cast<std::uint32_t>(pc) ^ 0xA7B5U) * 181U + 61U) % 2048U;
    }
    std::bitset<2048> bits_{};
};

enum class DebugError { not_reset, breakpoint, trace_full, cpu_failure };
struct DebugFailure {
    DebugError kind{};
    CpuFailure cpu{};
};

class TracedCpu {
public:
    explicit TracedCpu(CpuBus& bus) noexcept : cpu_(bus) {}

    [[nodiscard]] std::expected<void, CpuFailure> reset() noexcept {
        const auto result = cpu_.reset();
        if (!result) return result;
        history_.clear();
        visits_.clear();
        return {};
    }

    [[nodiscard]] std::expected<InstructionTrace, DebugFailure>
    execute_one() noexcept {
        if (!cpu_.ready()) return std::unexpected(DebugFailure{DebugError::not_reset, {}});
        // Check PRECONDITIONS before modifying CPU state. When history is
        // full we do not execute an instruction and silently lose its trace.
        if (history_.full()) return std::unexpected(
            DebugFailure{DebugError::trace_full, {}});
        if (breakpoints_.contains(cpu_.registers().pc.value)) return std::unexpected(
            DebugFailure{DebugError::breakpoint, {}});
        const auto executed = cpu_.step();
        if (!executed) return std::unexpected(
            DebugFailure{DebugError::cpu_failure, executed.error()});
        const auto stored = history_.push(*executed);
        // This branch is unreachable after the precondition, and move/copy
        // of InstructionTrace is noexcept. Keep failure visible regardless.
        if (!stored) return std::unexpected(
            DebugFailure{DebugError::trace_full, {}});
        visits_.record(executed->before);
        return *executed;
    }

    [[nodiscard]] std::expected<InstructionTrace, TraceError> pop_trace() noexcept {
        return history_.pop();
    }
    [[nodiscard]] bool history_full() const noexcept { return history_.full(); }
    [[nodiscard]] std::size_t pending_traces() const noexcept {
        return history_.size();
    }
    [[nodiscard]] bool possibly_visited(ProgramCounter pc) const noexcept {
        return visits_.possibly_visited(pc);
    }
    [[nodiscard]] const CpuRegisters& registers() const noexcept {
        return cpu_.registers();
    }
    [[nodiscard]] std::uint64_t cycles() const noexcept { return cpu_.total_cycles(); }

    // Ordered set permits exact membership and lower_bound(range probes).
    // Insertion may allocate/throw; it is separate from CPU instruction commit.
    void add_breakpoint(ProgramCounter pc) { breakpoints_.insert(pc.value); }
    void remove_breakpoint(ProgramCounter pc) noexcept {
        breakpoints_.erase(pc.value);
    }
    [[nodiscard]] std::optional<ProgramCounter>
    next_breakpoint_at_or_after(ProgramCounter pc) const noexcept {
        const auto it = breakpoints_.lower_bound(pc.value);
        if (it == breakpoints_.end()) return std::nullopt;
        return ProgramCounter{*it};
    }

private:
    Cpu6502Subset cpu_;   // borrows the bus; must not outlive it
    TraceRing<InstructionTrace, 16> history_{};
    VisitedPcBloom visits_{};
    std::set<std::uint16_t> breakpoints_{};
};

// Heap scheduling is a DIFFERENT question from FIFO trace order.
// Highest priority wins; equal priority uses oldest submission index.
struct MemoryProbe {
    ProgramCounter address{0};
    std::uint8_t priority{};
    std::uint64_t order{};
    friend bool operator==(MemoryProbe, MemoryProbe) = default;
};
struct ProbeLess {
    bool operator()(const MemoryProbe& a, const MemoryProbe& b) const noexcept {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.order > b.order;
    }
};
enum class ProbeError { full, order_exhausted };
struct ProbeObservation {
    MemoryProbe request;
    std::expected<Byte, BusError> value;
};

class MemoryProbeScheduler {
public:
    explicit MemoryProbeScheduler(CpuBus& bus) noexcept : bus_(bus) {}
    [[nodiscard]] std::size_t pending() const noexcept { return pending_.size(); }
    [[nodiscard]] std::expected<void, ProbeError>
    schedule(ProgramCounter address, std::uint8_t priority) {
        if (pending_.size() >= 64) return std::unexpected(ProbeError::full);
        if (next_order_ == std::numeric_limits<std::uint64_t>::max())
            return std::unexpected(ProbeError::order_exhausted);
        pending_.push(MemoryProbe{address, priority, next_order_});
        ++next_order_;
        return {};
    }
    [[nodiscard]] std::optional<ProbeObservation> inspect_next() noexcept {
        if (pending_.empty()) return std::nullopt;
        const auto request = pending_.top();
        pending_.pop();
        // A failed bus read stays a typed observation, not a fabricated zero.
        return ProbeObservation{request, bus_.read(request.address.value)};
    }
private:
    CpuBus& bus_;
    std::priority_queue<MemoryProbe,std::vector<MemoryProbe>,ProbeLess> pending_;
    std::uint64_t next_order_{};
};

// A small DSU groups known aliases in the *analysis* layer. The real bus
// should keep its direct 0x07FF RAM mask; a DSU would be wrong in the hot path.
template<std::size_t N>
class DisjointSet {
    static_assert(N > 0);
public:
    DisjointSet() noexcept {
        for (std::size_t i=0; i<N; ++i) parent_[i] = i;
    }
    [[nodiscard]] std::size_t find(std::size_t x) noexcept {
        // Precondition x < N, ensured by the caller's fixed index set.
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];
            x = parent_[x];
        }
        return x;
    }
    bool join(std::size_t a, std::size_t b) noexcept {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rank_[a] < rank_[b]) std::swap(a,b);
        parent_[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        return true;
    }
    [[nodiscard]] bool connected(std::size_t a, std::size_t b) noexcept {
        return find(a) == find(b);
    }
private:
    std::array<std::size_t,N> parent_{};
    std::array<unsigned,N> rank_{};
};

} // namespace allmight::nes::cpu
