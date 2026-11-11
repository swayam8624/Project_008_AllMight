// Project AllMight M016 / Parts 251-270, C++20 executable reference.
// Extends the growing asset registry into a read-only query snapshot, typed
// results, ordered searches, deterministic reporting and safe erasure.
// No GPU submission, allocator benchmark or concurrent registry is claimed.
#include <algorithm>
#include <any>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace allmight {

void check(bool condition, const char* failure) {
    if (!condition) throw std::runtime_error(failure); // active with -DNDEBUG
}

struct AssetHandle {
    std::uint32_t slot;
    std::uint64_t generation;
    friend bool operator==(AssetHandle, AssetHandle) = default;
};

enum class AssetKind { Mesh, Texture, Shader };
struct AssetRecord {
    AssetHandle id;
    std::string name; // owned; snapshots never borrow ephemeral file paths
    std::size_t bytes;
    int priority;
    AssetKind kind;
};

// A minimal snapshot of the earlier generation-checked registry. It is owned
// and detached from the live registry; handles in it may become stale later.
std::vector<AssetRecord> example_snapshot() {
    return {
        {{0, 1}, "brick.mesh", 48, 2, AssetKind::Mesh},
        {{1, 1}, "sky.tex", 64, 3, AssetKind::Texture},
        {{2, 1}, "ui.tex", 16, 3, AssetKind::Texture},
        {{3, 2}, "terrain.mesh", 128, 1, AssetKind::Mesh},
        {{4, 1}, "rock.mesh", 48, 2, AssetKind::Mesh},
    };
}

// Presence/absence answers a question with no meaningful failure reason.
std::optional<AssetHandle> find_optional(
    const std::vector<AssetRecord>& assets, std::string_view name) {
    const auto it = std::find_if(assets.begin(), assets.end(),
        [name](const AssetRecord& asset) { return asset.name == name; });
    if (it == assets.end()) return std::nullopt;
    return it->id; // value, not a reference into the snapshot
}

// Different error conditions need a typed result, not a magic sentinel ID.
enum class LookupError { EmptyName, NotFound };
template <class E> struct Unexpected { E error; };

template <class T, class E>
class ExpectedLike {
    std::variant<T, Unexpected<E>> state_;
    explicit ExpectedLike(T value) : state_(std::move(value)) {}
    explicit ExpectedLike(Unexpected<E> failed) : state_(failed) {}
public:
    static ExpectedLike success(T value) { return ExpectedLike(std::move(value)); }
    static ExpectedLike failure(E error) {
        return ExpectedLike(Unexpected<E>{error});
    }
    bool has_value() const noexcept { return std::holds_alternative<T>(state_); }
    const T& value() const { return std::get<T>(state_); }
    const E& error() const { return std::get<Unexpected<E>>(state_).error; }
};

ExpectedLike<AssetHandle, LookupError> lookup(
    const std::vector<AssetRecord>& assets, std::string_view name) {
    if (name.empty()) return ExpectedLike<AssetHandle, LookupError>::failure(
        LookupError::EmptyName);
    auto candidate = find_optional(assets, name);
    if (!candidate) return ExpectedLike<AssetHandle, LookupError>::failure(
        LookupError::NotFound);
    return ExpectedLike<AssetHandle, LookupError>::success(*candidate);
}

// A closed set of commands is expressed with std::variant and std::visit.
struct Inspect { AssetHandle id; };
struct Upload { AssetHandle id; };
struct Evict { AssetHandle id; };
using AssetCommand = std::variant<Inspect, Upload, Evict>;

template <class... Functions> struct Overloaded : Functions... {
    using Functions::operator()...;
};
template <class... Functions> Overloaded(Functions...) -> Overloaded<Functions...>;

// A comparator with both argument orders supports lower_bound, upper_bound
// and equal_range on a vector sorted by byte count. Their precondition is a
// properly partitioned/sorted range under the SAME ordering relation.
struct CompareBytes {
    bool operator()(const AssetRecord& asset, std::size_t n) const noexcept {
        return asset.bytes < n;
    }
    bool operator()(std::size_t n, const AssetRecord& asset) const noexcept {
        return n < asset.bytes;
    }
};

void run() {
    auto assets = example_snapshot();
    check(assets.size() == 5, "owned snapshot retains five assets");

    // 251: Optional means 'may be missing' but does not encode why.
    const auto present = find_optional(assets, "rock.mesh");
    const auto absent = find_optional(assets, "missing.tex");
    check(present && *present == AssetHandle{4, 1} && !absent,
          "optional presence/absence is explicit");

    // 254: C++20 expected-like result is a deliberately small teaching type.
    const auto good = lookup(assets, "rock.mesh");
    const auto blank = lookup(assets, "");
    const auto missing = lookup(assets, "other.mesh");
    check(good.has_value() && good.value() == AssetHandle{4, 1},
          "expected-like success carries an ID");
    check(!blank.has_value() && blank.error() == LookupError::EmptyName &&
          !missing.has_value() && missing.error() == LookupError::NotFound,
          "expected-like failure preserves error cause");

    // 252: A visitor must handle each of the closed command alternatives.
    std::vector<AssetCommand> commands{Inspect{{0, 1}}, Upload{{1, 1}}, Evict{{2, 1}}};
    std::size_t inspect_count = 0, upload_count = 0, evict_count = 0;
    for (const AssetCommand& command : commands) {
        std::visit(Overloaded{
            [&](const Inspect& op) { check(op.id.slot == 0, "inspect ID"); ++inspect_count; },
            [&](const Upload& op) { check(op.id.slot == 1, "upload ID"); ++upload_count; },
            [&](const Evict& op) { check(op.id.slot == 2, "evict ID"); ++evict_count; },
        }, command);
    }
    check(inspect_count == 1 && upload_count == 1 && evict_count == 1,
          "visit dispatches by active type rather than casted bytes");

    // 253: An open-ended diagnostics channel can accept std::any. Wrong-type
    // access throws; any_cast does not magically convert string to integer.
    std::any diagnostic = std::string("texture-rgba");
    check(std::any_cast<const std::string&>(diagnostic) == "texture-rgba",
          "any stores an owned concrete value");
    bool wrong_type = false;
    try { (void)std::any_cast<int>(diagnostic); }
    catch (const std::bad_any_cast&) { wrong_type = true; }
    check(wrong_type, "bad_any_cast is detectable");
    diagnostic.reset();
    check(!diagnostic.has_value(), "empty std::any owns no target");

    // 255-256: A pair is two values; a tuple allows an arbitrary fixed set of
    // heterogeneous fields. Structured bindings copy with 'auto', and bind
    // to the original pair members with 'auto&'.
    std::pair<std::size_t, std::size_t> counts{assets.size(), 0};
    auto [copied_count, copied_errors] = counts;
    auto& [live_count, live_errors] = counts;
    ++live_errors;
    check(copied_count == 5 && copied_errors == 0 &&
          live_count == 5 && counts.second == 1,
          "structured bindings distinguish copies from aliases");
    const auto report = std::tuple{std::string("frame-assets"), assets.size(), 1u};
    const auto& [report_name, report_count, report_version] = report;
    check(report_name == "frame-assets" && report_count == 5 && report_version == 1,
          "tuple exposes fixed typed fields");

    // 257-258: iterator categories are compile-time capabilities; a deque
    // has random-access iterators but NOT contiguous element storage.
    static_assert(std::contiguous_iterator<std::vector<int>::iterator>);
    static_assert(std::random_access_iterator<std::deque<int>::iterator>);
    static_assert(!std::contiguous_iterator<std::deque<int>::iterator>);
    static_assert(std::bidirectional_iterator<std::list<int>::iterator>);
    static_assert(!std::random_access_iterator<std::list<int>::iterator>);
    static_assert(std::forward_iterator<std::forward_list<int>::iterator>);
    static_assert(!std::bidirectional_iterator<std::forward_list<int>::iterator>);
    static_assert(std::input_iterator<std::istream_iterator<int>>);
    static_assert(!std::forward_iterator<std::istream_iterator<int>>);
    std::istringstream input{"7 8 9"};
    std::vector<int> parsed{std::istream_iterator<int>{input}, std::istream_iterator<int>{}};
    check(parsed == std::vector<int>({7, 8, 9}), "input iterator consumes stream once");

    // 260: Sorting is not required to preserve equal-key input order.
    auto sorted = assets; // own a separate snapshot before rearranging
    std::sort(sorted.begin(), sorted.end(), [](const AssetRecord& a, const AssetRecord& b) {
        if (a.bytes != b.bytes) return a.bytes < b.bytes;
        return a.name < b.name; // deterministic tie break, not relied on stability
    });
    check(sorted.front().name == "ui.tex" && sorted.back().name == "terrain.mesh",
          "sort orders by bytes with explicit tie break");

    // 261: Stable sort DOES preserve original order for equivalent priorities.
    auto stable = assets;
    std::stable_sort(stable.begin(), stable.end(), [](const AssetRecord& a, const AssetRecord& b) {
        return a.priority > b.priority;
    });
    check(stable[0].name == "sky.tex" && stable[1].name == "ui.tex" &&
          stable[2].name == "brick.mesh" && stable[3].name == "rock.mesh",
          "stable_sort preserves equal-priority relative order");

    // 262: Partition divides two groups but does not sort either group or
    // guarantee the original relative order inside the groups.
    auto partitioned = assets;
    const auto dividing = std::partition(partitioned.begin(), partitioned.end(),
        [](const AssetRecord& a) { return a.kind == AssetKind::Mesh; });
    check(std::all_of(partitioned.begin(), dividing,
            [](const AssetRecord& a) { return a.kind == AssetKind::Mesh; }) &&
          std::none_of(dividing, partitioned.end(),
            [](const AssetRecord& a) { return a.kind == AssetKind::Mesh; }),
          "partition places predicate-true records first");
    check(std::partition_point(partitioned.begin(), partitioned.end(),
          [](const AssetRecord& a) { return a.kind == AssetKind::Mesh; }) == dividing,
          "partition_point requires already partitioned input");

    // 263-264: binary search requires the appropriate sorted/partitioned
    // order. Find both limits of the byte-size==48 subrange.
    std::sort(sorted.begin(), sorted.end(), [](const AssetRecord& a, const AssetRecord& b) {
        return a.bytes < b.bytes;
    });
    const auto lower = std::lower_bound(sorted.begin(), sorted.end(), std::size_t{48}, CompareBytes{});
    const auto upper = std::upper_bound(sorted.begin(), sorted.end(), std::size_t{48}, CompareBytes{});
    const auto both = std::equal_range(sorted.begin(), sorted.end(), std::size_t{48}, CompareBytes{});
    check(std::distance(sorted.begin(), lower) == 1 &&
          std::distance(sorted.begin(), upper) == 3 &&
          both.first == lower && both.second == upper,
          "binary-search contract finds exactly two 48-byte assets");
    check(std::lower_bound(sorted.begin(), sorted.end(), std::size_t{50}, CompareBytes{})
              == upper, "insertion point for nonmatching byte count");

    // 265: Linear find is appropriate for unsorted names when building a
    // one-off snapshot; the live registry from M015 has a separate hash index.
    std::vector<std::string> names;
    names.reserve(assets.size());
    std::transform(assets.begin(), assets.end(), std::back_inserter(names),
                   [](const AssetRecord& a) { return a.name; });
    check(std::find(names.begin(), names.end(), "sky.tex") != names.end() &&
          std::find(names.begin(), names.end(), "absent") == names.end(),
          "linear find works without sorted input");

    // 266: transform is an elementwise expression here, not a guarantee of
    // parallel execution or SIMD. Preserve a numeric snapshot for reduction.
    std::vector<std::size_t> byte_sizes;
    byte_sizes.reserve(assets.size());
    std::transform(assets.begin(), assets.end(), std::back_inserter(byte_sizes),
                   [](const AssetRecord& a) { return a.bytes; });
    check(byte_sizes == std::vector<std::size_t>({48, 64, 16, 128, 48}),
          "transform projects each asset size");

    // 267-268: accumulate is an ordered left fold; reduce may regroup.
    // Unsigned sums are exact for these small values. Floating point requires
    // an explicit numerical-accuracy/reproducibility policy.
    const auto sequential_sum = std::accumulate(byte_sizes.begin(), byte_sizes.end(),
                                                std::size_t{0});
    const auto regroupable_sum = std::reduce(byte_sizes.begin(), byte_sizes.end(),
                                             std::size_t{0});
    check(sequential_sum == 304 && regroupable_sum == 304,
          "both integer reductions compute a small exact byte total");
    const double large = 1e20;
    const double a = ((large + 1.0) - large) + 3.0;
    const double b = (large - large) + (1.0 + 3.0);
    check(a == 3.0 && b == 4.0,
          "floating-point regrouping can change a result");

    // 269: remove_if rearranges the logical retained prefix; vector::erase
    // must destroy the obsolete suffix and actually reduce the size.
    std::vector<int> work_ids{1, 2, 3, 4, 5, 6};
    const auto retained_end = std::remove_if(work_ids.begin(), work_ids.end(),
                                            [](int id) { return id % 2 == 0; });
    check(std::distance(work_ids.begin(), retained_end) == 3 &&
          work_ids.size() == 6, "remove_if does not shrink vector");
    work_ids.erase(retained_end, work_ids.end());
    check(work_ids == std::vector<int>({1, 3, 5}),
          "erase/remove destroys the removed logical suffix");

    // 270: std::erase_if (C++20) expresses the combined operation directly.
    auto texture_removed = assets;
    const auto removed = std::erase_if(texture_removed,
        [](const AssetRecord& a) { return a.kind == AssetKind::Texture; });
    check(removed == 2 && texture_removed.size() == 3 &&
          std::all_of(texture_removed.begin(), texture_removed.end(),
            [](const AssetRecord& a) { return a.kind != AssetKind::Texture; }),
          "erase_if reports count and shrinks container");
    std::unordered_map<std::string, int> diagnostics{{"sky.tex", 3}, {"brick.mesh", 2}};
    const auto hash_removed = std::erase_if(diagnostics,
        [](const auto& key_and_priority) { return key_and_priority.second == 3; });
    check(hash_removed == 1 && diagnostics.size() == 1 &&
          diagnostics.contains("brick.mesh"),
          "C++20 erase_if works on appropriate associative containers");

    std::cout << "asset_query_algorithm_lab: all checks passed\n";
}
} // namespace allmight

int main() {
    try { allmight::run(); }
    catch (const std::exception& error) {
        std::cerr << "asset_query_algorithm_lab failed: " << error.what() << '\n';
        return 1;
    }
}
