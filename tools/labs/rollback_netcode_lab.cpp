/*
Teaching contract: State is the complete deterministic gameplay state for this
tiny two-player example; tick is an integer simulation step and positions are
integer cells. Input is the command assigned to one tick. The peer predicts a
missing remote command by repeating the latest known one, stores the state
before each tick plus inputs used, and when a late command differs restores the
preceding state and replays through the present. Capacity bounds memory and
resimulation age; a correction outside retained history is rejected. State
hashes are diagnostic only. This integer witness avoids floating-point drift;
it does not prove determinism for a real engine or implement packet transport.
*/
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

struct Input {
    int move = 0;   // -1, 0, +1
    bool fire = false;
    friend bool operator==(const Input&, const Input&) = default;
};

struct State {
    std::int64_t tick = 0;
    std::int64_t p0 = 0;
    std::int64_t p1 = 0;
    std::int64_t score0 = 0;
    std::int64_t score1 = 0;
    friend bool operator==(const State&, const State&) = default;
};

State step(State s, Input a, Input b) {
    ++s.tick;
    s.p0 += a.move;
    s.p1 += b.move;

    // Deterministic toy hit rule: firing scores when players are at most one
    // integer cell apart after movement. Integer state avoids floating-point
    // cross-platform drift in this teaching witness.
    if (a.fire && std::llabs(s.p0 - s.p1) <= 1) ++s.score0;
    if (b.fire && std::llabs(s.p0 - s.p1) <= 1) ++s.score1;
    return s;
}

std::uint64_t hash_state(const State& s) {
    // A tiny deterministic diagnostic hash, not a cryptographic hash.
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&](std::int64_t v) {
        std::uint64_t x = static_cast<std::uint64_t>(v);
        for (int i = 0; i < 8; ++i) {
            h ^= (x >> (i * 8)) & 0xffu;
            h *= 1099511628211ull;
        }
    };
    mix(s.tick); mix(s.p0); mix(s.p1); mix(s.score0); mix(s.score1);
    return h;
}

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class RollbackPeer {
public:
    explicit RollbackPeer(std::size_t history_capacity)
        : capacity_(history_capacity) {
        if (capacity_ < 2) throw std::invalid_argument("history must hold at least two states");
        states_[0] = State{};
    }

    void set_local_input(std::int64_t tick, Input in) {
        local_[tick] = in;
    }

    // Returns true when the new remote input changed a past prediction and a
    // rollback/resimulation was performed.
    bool receive_remote_input(std::int64_t tick, Input in) {
        if (tick <= confirmed_before_) return false; // duplicate/ancient confirmed packet
        if (tick <= current_.tick && !states_.contains(tick - 1))
            throw std::runtime_error("correction is older than retained rollback history");
        auto old = remote_.find(tick);
        if (old != remote_.end() && old->second == in) return false; // idempotent duplicate

        remote_[tick] = in;

        const auto predicted = remote_used_.find(tick);
        // Only an already-consumed prediction mismatch requires history repair.
        if (predicted != remote_used_.end() && predicted->second != in) {
            if (!states_.contains(tick - 1))
                throw std::runtime_error("misprediction is older than retained rollback history");
            rollback_from(tick);
            return true;
        }
        return false;
    }

    void advance_to(std::int64_t target_tick) {
        while (current_.tick < target_tick) simulate_one(current_.tick + 1);
    }

    const State& state() const { return current_; }
    std::uint64_t state_hash() const { return hash_state(current_); }

    void confirm_through(std::int64_t tick) {
        confirmed_before_ = std::max(confirmed_before_, tick);
        trim_history();
    }

private:
    Input local_for(std::int64_t tick) const {
        if (auto it = local_.find(tick); it != local_.end()) return it->second;
        return {};
    }

    Input remote_for(std::int64_t tick) const {
        if (auto it = remote_.find(tick); it != remote_.end()) return it->second;

        // Common rollback prediction policy: repeat the last known remote input.
        Input last{};
        auto it = remote_.upper_bound(tick);
        if (it != remote_.begin()) {
            --it;
            last = it->second;
        }
        return last;
    }

    void simulate_one(std::int64_t tick) {
        const Input a = local_for(tick);
        const Input b = remote_for(tick);
        remote_used_[tick] = b;
        current_ = step(current_, a, b);
        states_[current_.tick] = current_;
        trim_history();
    }

    // Restore the pre-tick snapshot and replay retained commands to the old present.
    void rollback_from(std::int64_t tick) {
        const std::int64_t previous_target = current_.tick;
        current_ = states_.at(tick - 1);

        for (auto it = states_.lower_bound(tick); it != states_.end(); )
            it = states_.erase(it);
        for (auto it = remote_used_.lower_bound(tick); it != remote_used_.end(); )
            it = remote_used_.erase(it);

        while (current_.tick < previous_target) simulate_one(current_.tick + 1);
    }

    void trim_history() {
        while (states_.size() > capacity_) states_.erase(states_.begin());

        // Inputs older than the oldest retained transition are no longer useful.
        if (!states_.empty()) {
            const std::int64_t oldest_state = states_.begin()->first;
            for (auto it = remote_used_.begin(); it != remote_used_.end() && it->first <= oldest_state; )
                it = remote_used_.erase(it);
        }
    }

    std::size_t capacity_;
    State current_{};
    std::map<std::int64_t, State> states_;
    std::map<std::int64_t, Input> local_;
    std::map<std::int64_t, Input> remote_;
    std::map<std::int64_t, Input> remote_used_;
    std::int64_t confirmed_before_ = -1;
};

State authoritative_run(
    std::int64_t ticks,
    const std::map<std::int64_t, Input>& a,
    const std::map<std::int64_t, Input>& b
) {
    State s{};
    for (std::int64_t t = 1; t <= ticks; ++t) {
        const Input ia = a.contains(t) ? a.at(t) : Input{};
        const Input ib = b.contains(t) ? b.at(t) : Input{};
        s = step(s, ia, ib);
    }
    return s;
}

int main() {
    std::map<std::int64_t, Input> local{
        {1,{1,false}}, {2,{1,false}}, {3,{0,true}}, {4,{-1,false}}, {5,{0,true}}
    };
    std::map<std::int64_t, Input> remote_truth{
        {1,{-1,false}}, {2,{-1,false}}, {3,{1,false}}, {4,{1,true}}, {5,{0,false}}
    };

    const State truth = authoritative_run(5, local, remote_truth);

    RollbackPeer peer(16);
    for (const auto& [t,in] : local) peer.set_local_input(t,in);

    // Tick 1 remote input arrives in time. Ticks 2..5 are initially predicted
    // by repeating that last known input.
    peer.receive_remote_input(1, remote_truth.at(1));
    peer.advance_to(5);
    const auto speculative_hash = peer.state_hash();
    require(!(peer.state() == truth), "prediction should differ before late packets arrive");

    // A late authoritative tick 2 happens to match the repeated prediction.
    require(!peer.receive_remote_input(2, remote_truth.at(2)),
            "matching late input should not require rollback");

    // Tick 3 differs from prediction, so the peer restores tick 2 and resimulates.
    require(peer.receive_remote_input(3, remote_truth.at(3)),
            "mispredicted past input must trigger rollback");
    require(peer.state_hash() != speculative_hash,
            "rollback should change the speculative result in this scenario");

    // Later authoritative inputs can trigger further resimulation.
    require(peer.receive_remote_input(4, remote_truth.at(4)),
            "tick 4 mismatch must trigger rollback");
    require(peer.receive_remote_input(5, remote_truth.at(5)),
            "tick 5 mismatch must trigger rollback");

    require(peer.state() == truth,
            "after all authoritative inputs arrive, rollback state must equal direct simulation");
    require(peer.state_hash() == hash_state(truth),
            "deterministic state hash must agree with authoritative run");

    // Duplicate packet: application layer remains idempotent.
    require(!peer.receive_remote_input(5, remote_truth.at(5)),
            "duplicate authoritative input must be harmless");

    // History overflow is an explicit policy, not silent corruption.
    RollbackPeer tiny(3);
    tiny.set_local_input(1,{1,false});
    tiny.receive_remote_input(1,{0,false});
    tiny.advance_to(8);
    bool rejected_old = false;
    try {
        tiny.receive_remote_input(2,{1,false});
    } catch (const std::runtime_error&) {
        rejected_old = true;
    }
    require(rejected_old, "too-old correction must fail visibly when history is gone");

    std::cout << "rollback netcode: 7 suites PASS\n";
}
