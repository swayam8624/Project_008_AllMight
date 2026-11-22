// CPU oracle for the GPU count -> exclusive scan -> stable scatter pattern.
// Every selected item gets a unique output position; the counter is an
// independently checked property rather than a presumed GPU side effect.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <span>
#include <vector>

struct Compaction {
    std::vector<std::size_t> exclusive_offsets;
    std::vector<int> selected;
};

std::optional<Compaction> compact(
    std::span<const std::uint8_t> predicate,
    std::span<const int> values) {

    if (predicate.size() != values.size()) return std::nullopt;
    Compaction result;
    result.exclusive_offsets.resize(predicate.size());

    std::size_t count = 0;
    for (std::size_t i = 0; i < predicate.size(); ++i) {
        if (predicate[i] > 1) return std::nullopt;
        result.exclusive_offsets[i] = count;
        count += predicate[i];
    }

    result.selected.resize(count);
    for (std::size_t i = 0; i < predicate.size(); ++i) {
        if (predicate[i] == 1) {
            // Every included item writes exactly once to a unique slot.
            result.selected[result.exclusive_offsets[i]] = values[i];
        }
    }
    return result;
}

int main() {
    const std::vector<std::uint8_t> pred{1, 0, 1, 1};
    const std::vector<int> values{7, 9, 3, 11};
    const auto result = compact(pred, values);
    if (!result) return 1;
    if (result->exclusive_offsets != std::vector<std::size_t>{0, 1, 1, 2}) return 2;
    if (result->selected != std::vector<int>{7, 3, 11}) return 3;

    if (compact(std::vector<std::uint8_t>{0, 2}, std::vector<int>{3, 9})) {
        return 4;  // A predicate is exactly 0 or 1, not any nonzero byte.
    }
    if (compact(std::vector<std::uint8_t>{1}, std::vector<int>{})) {
        return 5;  // Mismatched input extents are invalid.
    }

    std::mt19937 random{0x2042};
    for (unsigned trial = 0; trial < 200; ++trial) {
        const std::size_t n = static_cast<std::size_t>(random() % 64U);
        std::vector<std::uint8_t> flags(n);
        std::vector<int> records(n);
        for (std::size_t i = 0; i < n; ++i) {
            flags[i] = static_cast<std::uint8_t>(random() % 2U);
            records[i] = static_cast<int>(i);
        }

        std::vector<int> oracle;
        for (std::size_t i = 0; i < n; ++i) {
            if (flags[i]) oracle.push_back(records[i]);
        }

        const auto computed = compact(flags, records);
        if (!computed || computed->selected != oracle) return 6;
        if (computed->exclusive_offsets.size() != n) return 7;
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t left = static_cast<std::size_t>(
                std::count(flags.begin(), flags.begin() + static_cast<std::ptrdiff_t>(i),
                           std::uint8_t{1}));
            if (computed->exclusive_offsets[i] != left) return 8;
        }
    }

    std::cout << "parallel_scan: 200 seeded oracle checks passed\n";
}
