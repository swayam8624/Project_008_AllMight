// Project AllMight M015 / Parts 231-250.
// C++20 standalone laboratory: a growing asset registry, borrowed views,
// container invalidation, hash collisions, and scheduling adapters.
// This models CPU-side ownership. No GPU queue or native asset source is used.
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <functional>
#include <iostream>
#include <limits>
#include <iterator>
#include <memory>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace allmight {
void verify(bool ok, const char* explanation) {
    if (!ok) throw std::runtime_error(explanation); // checks survive NDEBUG
}

struct Asset {
    std::string name;                     // owns its characters
    std::vector<std::uint8_t> source;     // owns its bytes
};

// Keep identity independent of where a std::vector element happens to live.
// Reusing an old slot changes its generation, so stale handles are rejected.
struct AssetHandle {
    std::size_t slot;
    std::uint64_t generation;
    friend bool operator==(const AssetHandle&, const AssetHandle&) = default;
};

class AssetRegistry {
    struct Slot {
        std::optional<Asset> value;
        std::uint64_t generation = 1;
    };
    std::vector<Slot> slots_;
    std::vector<std::size_t> free_;
    std::unordered_map<std::string, AssetHandle> names_;

public:
    std::size_t slot_count() const noexcept { return slots_.size(); }
    std::size_t size() const noexcept { return names_.size(); }
    std::size_t capacity() const noexcept { return slots_.capacity(); }

    // Single-threaded transaction: a new slot is published to the name index
    // only after the owning Asset is complete. Roll back a slot if the hash
    // insertion throws; existing handles and values remain unchanged.
    AssetHandle insert(std::string name, std::vector<std::uint8_t> source,
                       bool fail_preparation = false, bool fail_commit = false) {
        if (name.empty() || source.empty())
            throw std::invalid_argument("asset name/data cannot be empty");
        if (names_.contains(name))
            throw std::invalid_argument("duplicate asset name");
        if (fail_preparation)
            throw std::runtime_error("simulated failure before publication");

        static_assert(std::is_nothrow_move_constructible_v<Asset>);
        // When a slot is reused, its already-incremented generation identifies
        // the NEW occupant, not the previous object once held at that index.
        const bool reuse = !free_.empty();
        const std::size_t index = reuse ? free_.back() : slots_.size();
        if (!reuse) slots_.emplace_back();
        Slot& slot = slots_[index];
        slot.value.emplace(Asset{std::move(name), std::move(source)});
        const AssetHandle handle{index, slot.generation};
        try {
            // Deliberately fail after constructing the slot but before its
            // name becomes externally discoverable, then verify rollback.
            if (fail_commit) throw std::runtime_error("injected commit failure");
            const auto [it, published] = names_.emplace(slot.value->name, handle);
            (void)it;
            if (!published) throw std::logic_error("duplicate during commit");
        } catch (...) {
            slot.value.reset(); // release the uncommitted owning payload
            if (!reuse) slots_.pop_back();
            throw;
        }
        if (reuse) free_.pop_back();
        return handle;
    }

    const Asset* resolve(AssetHandle h) const noexcept {
        if (h.slot >= slots_.size()) return nullptr;
        const Slot& slot = slots_[h.slot];
        if (slot.generation != h.generation || !slot.value) return nullptr;
        return std::addressof(*slot.value);
    }
    std::optional<AssetHandle> find(std::string_view name) const {
        // The temporary owns the queried characters during this lookup.
        // A production transparent hasher could avoid this allocation.
        auto it = names_.find(std::string(name));
        if (it == names_.end()) return std::nullopt;
        return it->second;
    }
    bool erase(AssetHandle h) {
        const Asset* found = resolve(h);
        if (!found) return false;
        if (slots_[h.slot].generation == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("generation exhausted; cannot reuse slot");
        // Reserving free-list capacity happens before mutations, so failure
        // here does not remove the published asset.
        free_.reserve(free_.size() + 1);
        names_.erase(found->name);
        Slot& slot = slots_[h.slot];
        slot.value.reset(); // destroys the stored name and byte vector
        ++slot.generation;
        free_.push_back(h.slot);
        return true;
    }
};

struct ConstantHash {
    std::size_t operator()(const std::string&) const noexcept { return 0; }
};

struct UploadJob { int priority; std::uint64_t sequence; std::string name; };
struct JobLess {
    bool operator()(const UploadJob& left, const UploadJob& right) const {
        if (left.priority != right.priority) return left.priority < right.priority;
        // Earlier sequence wins ties; priority_queue is not inherently stable.
        return left.sequence > right.sequence;
    }
};

std::size_t checksum(std::span<const std::uint8_t> bytes) noexcept {
    std::size_t sum = 0;
    for (std::uint8_t byte : bytes) sum += byte;
    return sum;
}

void run() {
    // vector: size means live elements, capacity means available storage.
    std::vector<int> dense;
    dense.reserve(2);
    verify(dense.size() == 0 && dense.capacity() >= 2, "reserve does not create elements");
    dense.push_back(10);
    dense.push_back(20);
    const std::size_t capacity_before = dense.capacity();
    // At this point a pointer/span into dense could be retained only if no
    // invalidating operation follows. We intentionally retain NO such pointer.
    dense.push_back(30); // guaranteed to reallocate when size == capacity
    const std::size_t capacity_after_growth = dense.capacity();
    verify(capacity_after_growth > capacity_before && dense.size() == 3,
           "growth requires new capacity; old iterators would be invalid");
    dense.resize(5); // constructs two live zero-initialized int elements
    verify(dense.size() == 5 && dense[3] == 0 && dense[4] == 0,
           "resize changes size, not merely reserved storage");
    std::cout << "vector capacity growth: " << capacity_before << " -> "
              << capacity_after_growth << "\n";

    // array stays fixed and contiguous. deque permits cheap front/back work,
    // but is not itself one contiguous T[N] allocation and has distinct rules.
    std::array<int, 3> fixed{2, 4, 6};
    verify(checksum(std::span<const std::uint8_t>{}) == 0, "empty span is valid");
    verify(fixed.size() == 3 && fixed.data()[2] == 6, "fixed array contiguous");
    std::deque<int> ends;
    ends.push_back(2);
    ends.push_front(1);
    ends.push_back(3);
    verify(ends.front() == 1 && ends.back() == 3 && ends[1] == 2,
           "deque indexed segmented sequence");

    // Linked containers keep node iterators/references stable under splicing,
    // subject to the operations' normal preconditions and erasure rules.
    std::list<std::string> edits{"import", "compile"};
    auto compile_it = std::next(edits.begin());
    std::list<std::string> incoming{"upload"};
    edits.splice(edits.end(), incoming, incoming.begin());
    verify(*compile_it == "compile" && incoming.empty() && edits.back() == "upload",
           "list node transfer preserves unrelated iterators");
    std::forward_list<int> singly{1, 2, 3};
    singly.erase_after(singly.begin()); // removes 2, not 1
    verify(std::vector<int>(singly.begin(), singly.end()) == std::vector<int>({1, 3}),
           "forward_list erase_after needs predecessor");

    // Ordered maps/sets use comparison order and logarithmic operations.
    std::map<std::string, int> named{{"mesh", 1}, {"shader", 3}, {"texture", 2}};
    std::set<int> unique_ids{8, 3, 8, 5};
    verify(named.begin()->first == "mesh" && unique_ids.size() == 3 &&
               *unique_ids.begin() == 3, "ordered unique-key containers");

    // Unordered containers are lookup-oriented. Hash equality MUST be
    // consistent with key equality. A pathological constant hash preserves
    // correctness, but forces the keys into one bucket.
    std::unordered_map<std::string, int, ConstantHash> collisions;
    collisions.max_load_factor(0.5f);
    for (int i = 0; i < 24; ++i) collisions.emplace("asset" + std::to_string(i), i);
    verify(collisions.size() == 24 && collisions.at("asset19") == 19,
           "collisions do not change key equality or lookup results");
    const auto bucket = collisions.bucket("asset0");
    verify(collisions.bucket_size(bucket) == collisions.size(),
           "constant hasher sends every key to one bucket");
    const auto buckets_before = collisions.bucket_count();
    collisions.reserve(96); // at max_load_factor 0.5, requests enough buckets
    verify(collisions.bucket_count() >= buckets_before &&
               collisions.load_factor() <= collisions.max_load_factor(),
           "rehash policy and load factor contract");
    // Do not dereference any pre-rehash iterator: rehash invalidates iterators.
    std::unordered_set<std::string> pending{"mesh", "texture", "mesh"};
    verify(pending.size() == 2 && pending.contains("texture"),
           "unordered_set expresses membership without a value field");

    // Adapters expose scheduling semantics, not iteration/layout guarantees.
    std::priority_queue<UploadJob, std::vector<UploadJob>, JobLess> priority;
    priority.push({3, 2, "later-high"});
    priority.push({1, 3, "low"});
    priority.push({3, 1, "earlier-high"});
    verify(priority.top().name == "earlier-high", "priority and tie break");
    priority.pop();
    verify(priority.top().name == "later-high", "heap next highest job");
    std::queue<int> arrivals;
    arrivals.push(7); arrivals.push(8);
    verify(arrivals.front() == 7, "queue FIFO"); arrivals.pop();
    verify(arrivals.front() == 8, "queue second arrival");
    std::stack<int> undo;
    undo.push(7); undo.push(8);
    verify(undo.top() == 8, "stack LIFO"); undo.pop();
    verify(undo.top() == 7, "stack previous entry");

    AssetRegistry registry;
    const AssetHandle first = registry.insert("rocks.mesh", {10, 20, 30});
    verify(registry.resolve(first) && registry.size() == 1,
           "asset is published at stable logical handle");
    verify(checksum(registry.resolve(first)->source) == 60,
           "span-like read over stored contiguous bytes");
    const auto found = registry.find("rocks.mesh");
    verify(found && *found == first, "owned name index returns logical handle");
    const std::size_t old_slots = registry.slot_count();
    bool failed = false;
    try { (void)registry.insert("broken.mesh", {1, 2, 3}, true); }
    catch (const std::runtime_error&) { failed = true; }
    verify(failed && registry.size() == 1 && registry.slot_count() == old_slots,
           "injected pre-publication error does not mutate registry");
    bool commit_failed = false;
    try { (void)registry.insert("commit-broken.mesh", {9, 9}, false, true); }
    catch (const std::runtime_error&) { commit_failed = true; }
    verify(commit_failed && registry.size() == 1 &&
               registry.slot_count() == old_slots && registry.resolve(first),
           "failed publication rolls back the unpublished slot");
    verify(registry.erase(first) && !registry.resolve(first),
           "old generation cannot resolve after erase");
    const AssetHandle second = registry.insert("fresh.mesh", {1, 2});
    verify(second.slot == first.slot && second.generation != first.generation &&
               !registry.resolve(first) && registry.resolve(second),
           "reused slot has a new generation and old handle remains stale");

    // string_view is a borrowed (pointer,length) view and may contain no NUL.
    // Copy to std::string when a name must survive the temporary source.
    std::string owner = "texture/stone/normal.png";
    std::string_view name_view{owner};
    const auto subpath = name_view.substr(8, 5); // "stone", not NUL terminated
    verify(subpath == "stone" && subpath.size() == 5, "view knows its length");
    const std::string owned_name{subpath};
    owner.clear(); // invalidates borrowed content; do not use subpath afterward
    verify(owned_name == "stone", "copied string survives source mutation");
    std::string short_text = "mesh";
    std::string long_text(1000, 'x');
    verify(short_text.size() == 4 && long_text.size() == 1000,
           "strings own content whether or not implementation uses SSO");
    std::cout << "string capacities (short/long): " << short_text.capacity()
              << '/' << long_text.capacity() << " (implementation-dependent)\n";
    std::cout << "asset_container_lab: all checks passed\n";
}
} // namespace allmight

int main() {
    try { allmight::run(); }
    catch (const std::exception& error) {
        std::cerr << "asset_container_lab failed: " << error.what() << '\n';
        return 1;
    }
}
