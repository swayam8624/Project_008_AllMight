// AllMight M017, Parts 271-280: owned snapshot queries, ranges, allocators, PMR.
// C++20; deterministic CPU model, NOT a graphics driver / concurrent registry.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace allmight::asset_query {

void require(bool valid, const char* what) {
    if (!valid) throw std::runtime_error(what); // valid even with -DNDEBUG
}

// An owning snapshot is a separate lifetime from the live asset registry.
// Tombstones stay in this example to make a custom forward iterator useful.
struct Row {
    std::string name;
    std::size_t bytes;
    bool live;
};

std::vector<Row> make_snapshot() {
    return {{"brick.mesh", 48, true}, {"retired.tex", 512, false},
            {"sky.tex", 64, true}, {"ui.tex", 16, true},
            {"terrain.mesh", 128, true}, {"rock.mesh", 48, true}};
}

// Custom multi-pass forward iterator. It borrows the Row vector and skips
// tombstones, but does not own the vector or preserve validity on reallocation.
class LiveRowIterator {
    const std::vector<Row>* rows_ = nullptr;
    std::size_t pos_ = 0;

    void skip_dead() noexcept {
        if (!rows_) return;
        while (pos_ < rows_->size() && !(*rows_)[pos_].live) ++pos_;
    }
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = Row;
    using difference_type = std::ptrdiff_t;
    using reference = const Row&;

    LiveRowIterator() = default;
    LiveRowIterator(const std::vector<Row>& rows, std::size_t pos)
        : rows_(&rows), pos_(pos) { skip_dead(); }
    reference operator*() const noexcept { return (*rows_)[pos_]; }
    const Row* operator->() const noexcept { return std::addressof(**this); }
    LiveRowIterator& operator++() noexcept { ++pos_; skip_dead(); return *this; }
    LiveRowIterator operator++(int) noexcept { auto old = *this; ++*this; return old; }
    friend bool operator==(const LiveRowIterator&, const LiveRowIterator&) = default;
};
static_assert(std::forward_iterator<LiveRowIterator>);
static_assert(!std::bidirectional_iterator<LiveRowIterator>);
static_assert(!std::contiguous_iterator<LiveRowIterator>);

// begin() and end() are discovered in range-based for's exposition model.
// A range-for keeps its range expression alive for the loop, but a returned
// view or stored iterator still requires explicit owner-lifetime reasoning.
struct LiveRows {
    const std::vector<Row>* owner;
    LiveRowIterator begin() const { return LiveRowIterator{*owner, 0}; }
    LiveRowIterator end() const { return LiveRowIterator{*owner, owner->size()}; }
};
static_assert(std::ranges::forward_range<LiveRows>);

// An illustrative C++ allocator: typed allocate/deallocate must be paired.
// The shared statistics object survives copies/rebinds of the allocator.
struct AllocationStats {
    std::size_t allocations = 0;
    std::size_t deallocations = 0;
    std::size_t allocated_bytes = 0;
};
template<class T>
struct TrackingAllocator {
    using value_type = T;
    std::shared_ptr<AllocationStats> stats;
    TrackingAllocator() : stats(std::make_shared<AllocationStats>()) {}
    explicit TrackingAllocator(std::shared_ptr<AllocationStats> s) : stats(std::move(s)) {}
    template<class U>
    TrackingAllocator(const TrackingAllocator<U>& other) noexcept : stats(other.stats) {}
    T* allocate(std::size_t count) {
        if (count > std::allocator_traits<std::allocator<T>>::max_size(std::allocator<T>{}))
            throw std::bad_array_new_length();
        auto* raw = std::allocator<T>{}.allocate(count);
        ++stats->allocations;
        stats->allocated_bytes += count * sizeof(T);
        return raw;
    }
    void deallocate(T* ptr, std::size_t count) noexcept {
        ++stats->deallocations;
        std::allocator<T>{}.deallocate(ptr, count);
    }
    template<class U> friend struct TrackingAllocator;
    template<class U> bool operator==(const TrackingAllocator<U>& rhs) const noexcept {
        return stats == rhs.stats;
    }
};

// std::pmr moves the allocation-policy choice to runtime. The resource must
// outlive every container and object retaining a pointer to it.
class CountingMemoryResource final : public std::pmr::memory_resource {
    std::pmr::memory_resource* upstream_;
public:
    std::size_t allocated_blocks = 0;
    std::size_t freed_blocks = 0;
    std::size_t allocated_bytes = 0;
    explicit CountingMemoryResource(std::pmr::memory_resource* upstream)
        : upstream_(upstream) {}
private:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        void* ptr = upstream_->allocate(bytes, alignment);
        ++allocated_blocks;
        allocated_bytes += bytes;
        return ptr;
    }
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {
        ++freed_blocks;
        upstream_->deallocate(ptr, bytes, alignment);
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};

// Function objects and lambdas give equivalent call semantics here, but
// code generation/allocations/capture size need MEASUREMENT, not slogans.
struct AtLeastBytes {
    std::size_t threshold;
    bool operator()(const Row& row) const noexcept { return row.bytes >= threshold; }
};

void run() {
    auto rows = make_snapshot();
    LiveRows live{&rows};
    std::size_t visits = 0;
    std::size_t total = 0;
    for (const Row& row : live) { // range-based for begin/end, not indexing
        ++visits;
        total += row.bytes;
    }
    require(visits == 5 && total == 304, "custom forward iterator skips tombstones");

    // Filter/transform adaptor creation is lazy. We can observe that neither
    // predicate nor transform runs until the range is iterated.
    std::size_t predicate_calls = 0;
    std::size_t projection_calls = 0;
    auto pipeline = std::views::all(rows)
        | std::views::filter([&](const Row& row) {
              ++predicate_calls;
              return row.live && row.bytes >= 48;
          })
        | std::views::transform([&](const Row& row) {
              ++projection_calls;
              return row.name;
          });
    require(predicate_calls == 0 && projection_calls == 0, "views compose lazily");
    std::vector<std::string> names;
    for (const auto& name : pipeline) names.push_back(name);
    require(names == std::vector<std::string>({"brick.mesh", "sky.tex",
                                               "terrain.mesh", "rock.mesh"}),
            "filter and transform produce the correct sequence");
    require(predicate_calls > 0 && projection_calls == 4,
            "predicates/transform execute during traversal");

    // Borrowed means iterators can outlive the range VIEW under its contract;
    // it does not mean the underlying vector's owner is kept alive.
    static_assert(std::ranges::borrowed_range<std::ranges::ref_view<std::vector<Row>>>);
    static_assert(!std::ranges::borrowed_range<std::vector<Row>>);
    static_assert(!std::ranges::borrowed_range<std::ranges::owning_view<std::vector<Row>>>);
    auto owned = std::views::all(make_snapshot()); // owns its vector in the view
    require(std::ranges::distance(owned) == 6, "rvalue vector becomes owning view");

    // A projection is supplied separately from a comparator: ranges::find
    // compares row.name, not the Row object or an allocated name list.
    auto located = std::ranges::find(rows, std::string_view{"sky.tex"}, &Row::name);
    require(located != rows.end() && located->bytes == 64,
            "projected ranges::find locates the named asset");
    auto sorted = rows;
    std::ranges::sort(sorted, std::less<>{}, &Row::bytes);
    require(sorted.front().bytes == 16 && sorted.back().bytes == 512,
            "projected ranges::sort uses byte count");

    AtLeastBytes functor{48};
    auto equivalent_lambda = [threshold = std::size_t{48}](const Row& row) noexcept {
        return row.bytes >= threshold;
    };
    require(std::ranges::count_if(rows, functor) ==
            std::ranges::count_if(rows, equivalent_lambda),
            "functor and captured lambda express same predicate");

    auto stats = std::make_shared<AllocationStats>();
    {
        std::vector<std::size_t, TrackingAllocator<std::size_t>> tracked{
            TrackingAllocator<std::size_t>{stats}};
        tracked.reserve(5);
        for (const Row& row : live) tracked.push_back(row.bytes);
        require(std::accumulate(tracked.begin(), tracked.end(), std::size_t{0}) == 304,
                "custom allocator does not change vector's value semantics");
        require(stats->allocations > 0 && stats->allocated_bytes >= 5*sizeof(std::size_t),
                "custom allocator observes requests");
    }
    require(stats->allocations == stats->deallocations,
            "typed allocator allocations paired by vector lifetime");

    CountingMemoryResource resource{std::pmr::new_delete_resource()};
    {
        std::pmr::vector<std::size_t> sizes{&resource};
        sizes.reserve(5);
        for (const Row& row : live) sizes.push_back(row.bytes);
        std::pmr::unordered_map<std::pmr::string, std::size_t> index{&resource};
        for (const Row& row : live)
            index.emplace(std::pmr::string{row.name, &resource}, row.bytes);
        require(index.size() == 5 && index.at(std::pmr::string{"sky.tex", &resource}) == 64,
                "pmr unordered_map uses resource for nodes/buckets");
        require(std::accumulate(sizes.begin(), sizes.end(), std::size_t{0}) == 304,
                "pmr vector and custom allocator produce same data result");
        require(resource.allocated_blocks > 0, "pmr resource intercepts allocation");
    }
    require(resource.allocated_blocks == resource.freed_blocks,
            "ordinary upstream resource releases deallocated blocks");

    CountingMemoryResource arena_upstream{std::pmr::new_delete_resource()};
    alignas(std::max_align_t) std::array<std::byte, 128> scratch{};
    {
        std::pmr::monotonic_buffer_resource arena{scratch.data(), scratch.size(),
                                                   &arena_upstream};
        {
            std::pmr::vector<std::uint64_t> pixels{&arena};
            pixels.resize(2048, 7); // exceeds scratch, must ask upstream
            require(pixels.size() == 2048 && pixels[2000] == 7,
                    "monotonic arena supplies live vector storage");
            require(arena_upstream.allocated_blocks > 0,
                    "exhausting stack scratch delegates upstream");
        }
        // pixels' destructor can call deallocate, but monotonic arena does
        // not return its blocks individually; release happens in bulk.
        require(arena_upstream.freed_blocks == 0,
                "monotonic deallocation doesn't return upstream blocks now");
    }
    require(arena_upstream.allocated_blocks == arena_upstream.freed_blocks,
            "arena destruction returns its upstream allocations");

    alignas(std::max_align_t) std::array<std::byte, 64> fixed_scratch{};
    std::pmr::monotonic_buffer_resource bounded{fixed_scratch.data(),
                                                fixed_scratch.size(),
                                                std::pmr::null_memory_resource()};
    bool exhausted = false;
    try {
        std::pmr::vector<std::uint64_t> huge{&bounded};
        huge.resize(2048, 2);
    } catch (const std::bad_alloc&) { exhausted = true; }
    require(exhausted, "fixed arena plus null upstream detects capacity exhaustion");
    std::cout << "asset_ranges_pmr_lab: all checks passed\n";
}
} // namespace allmight::asset_query

int main() {
    try { allmight::asset_query::run(); }
    catch (const std::exception& error) {
        std::cerr << "asset_ranges_pmr_lab failed: " << error.what() << '\n';
        return 1;
    }
}
