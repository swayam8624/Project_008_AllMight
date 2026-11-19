// Deterministic model of an asynchronous resource lifetime. The simulated
// completion token stands for a validated GPU completion witness; it is
// NOT a Vulkan semaphore, a Metal shared event, or a threading primitive.
//
// A host function returning is not enough to recycle device resources.
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <unordered_set>

class Resource {
    std::unordered_set<std::uint64_t> pending_;
    std::uint64_t next_token_ = 1;
    bool live_ = true;

public:
    std::optional<std::uint64_t> submit_use() {
        if (!live_ || next_token_ == std::numeric_limits<std::uint64_t>::max()) {
            return std::nullopt;
        }
        const std::uint64_t token = next_token_++;
        pending_.insert(token);
        return token;
    }

    bool complete_use(std::uint64_t token) {
        // Reject a guessed, stale, already completed, or non-existent token.
        return pending_.erase(token) == 1;
    }

    bool may_retire() const {
        return live_ && pending_.empty();
    }

    bool retire() {
        if (!may_retire()) return false;
        live_ = false;
        return true;
    }

    bool live() const { return live_; }
    std::size_t outstanding() const { return pending_.size(); }
};

int main() {
    Resource frame_constants;
    const auto frame0 = frame_constants.submit_use();
    const auto frame1 = frame_constants.submit_use();
    if (!frame0 || !frame1 || *frame0 == *frame1) return 1;
    if (frame_constants.outstanding() != 2) return 2;
    if (frame_constants.retire()) return 3; // Unsafe early retirement blocked.

    // Device work may complete in a different order from host submission.
    if (!frame_constants.complete_use(*frame1)) return 4;
    if (frame_constants.retire()) return 5;
    if (frame_constants.complete_use(*frame1)) return 6;
    if (!frame_constants.complete_use(*frame0)) return 7;

    if (!frame_constants.may_retire() || !frame_constants.retire()) return 8;
    if (frame_constants.live() || frame_constants.submit_use()) return 9;
    if (frame_constants.retire()) return 10;

    std::cout << "retirement_contract: all completion-order tests passed\n";
}
