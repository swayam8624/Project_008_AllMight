#pragma once
// Project AllMight — Book 01, Pass 10: correctness-gated host measurement.
//
// A timer makes a measurement, not a proof of performance. This utility
// reports recorded steady-clock nanoseconds, per-sample work digests and
// conservative nearest-rank quantiles. It has NO fabricated baseline speedup,
// CPU frequency assumption or GPU timestamp. A workload must return a
// deterministic numeric witness, so removing its useful work changes output.
//
// A compile-time benchmark template is also NOT a substitute for independently
// validating the result in a separate CTest self-test or differential oracle.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace allmight::nes::measure {

enum class MeasureError {
    zero_trials,
    zero_iterations,
    unsupported_iterations,
    inconsistent_work_result,
    clock_failure
};
struct Settings {
    std::size_t warmups{3};
    std::size_t trials{9};
    std::size_t iterations{32};
};
struct SampleReport {
    std::uint64_t min_ns{};
    std::uint64_t median_ns{};
    std::uint64_t p90_ns{};
    std::uint64_t max_ns{};
    std::uint64_t stable_digest{};
    std::size_t trials{};
    std::size_t iterations_per_trial{};
    std::size_t warmups{};
};

// Identical integer mixing across compilers. A digest documents observed
// result stability, it is not a cryptographic hash or full result oracle.
[[nodiscard]] constexpr std::uint64_t mix(std::uint64_t h,
                                           std::uint64_t value) noexcept {
    h ^= value + 0x9e3779b97f4a7c15ULL + (h<<6U) + (h>>2U);
    return h;
}
[[nodiscard]] inline std::size_t rank_nearest(std::size_t n,
                                              unsigned percentile) noexcept {
    // ceil(p*n/100) - 1 without overflowing p*n, given <=100.
    if (n==0) return 0;
    if (percentile>=100) return n-1;
    const auto whole=(n/100U)*percentile;
    const auto rem=((n%100U)*percentile+99U)/100U;
    const auto one_based=whole+rem;
    return one_based?one_based-1U:0U;
}
[[nodiscard]] inline std::expected<SampleReport,MeasureError>
summarize(std::span<const std::uint64_t> samples,
          std::uint64_t digest,Settings settings) {
    if (settings.trials==0)
        return std::unexpected(MeasureError::zero_trials);
    if (settings.iterations==0)
        return std::unexpected(MeasureError::zero_iterations);
    if (samples.size()!=settings.trials)
        return std::unexpected(MeasureError::unsupported_iterations);
    std::vector<std::uint64_t> sorted(samples.begin(),samples.end());
    std::sort(sorted.begin(),sorted.end());
    return SampleReport{
        sorted.front(),
        sorted[rank_nearest(sorted.size(),50)],
        sorted[rank_nearest(sorted.size(),90)],
        sorted.back(),
        digest,
        settings.trials,
        settings.iterations,
        settings.warmups
    };
}

template<class Work>
requires requires(Work& work,std::size_t index) {
    { work(index) } -> std::convertible_to<std::uint64_t>;
}
[[nodiscard]] std::expected<SampleReport,MeasureError>
run(Work&& work,Settings settings) {
    if (settings.trials==0)
        return std::unexpected(MeasureError::zero_trials);
    if (settings.iterations==0)
        return std::unexpected(MeasureError::zero_iterations);
    if (settings.trials>10000U ||
        settings.iterations>1000000U ||
        settings.warmups>10000U)
        return std::unexpected(MeasureError::unsupported_iterations);
    const auto execute=[&]() -> std::uint64_t {
        std::uint64_t checksum=0xCBF29CE484222325ULL;
        for(std::size_t i=0;i<settings.iterations;++i) {
            const std::uint64_t result=work(i);
            checksum=mix(checksum,result);
            // Compiler barrier against eliminating all data dependency across
            // work iterations; not a claim that optimization is impossible.
            std::atomic_signal_fence(std::memory_order_seq_cst);
        }
        return checksum;
    };
    for (std::size_t i=0;i<settings.warmups;++i) (void)execute();

    using clock=std::chrono::steady_clock;
    std::vector<std::uint64_t> samples;
    samples.reserve(settings.trials);
    std::uint64_t previous{};
    for (std::size_t trial=0;trial<settings.trials;++trial) {
        const auto start=clock::now();
        const auto observed=execute();
        const auto end=clock::now();
        if (trial!=0 && observed!=previous)
            return std::unexpected(MeasureError::inconsistent_work_result);
        previous=observed;
        if (end<start) return std::unexpected(MeasureError::clock_failure);
        const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        if (ns<0) return std::unexpected(MeasureError::clock_failure);
        samples.push_back(static_cast<std::uint64_t>(ns));
    }
    return summarize(samples,previous,settings);
}
} // namespace allmight::nes::measure
