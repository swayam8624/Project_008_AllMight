# 06 — Grow Registries without Invalidating Identity

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|Next chapter]]

A fixed-capacity store is a useful model but cannot serve a growing editor. This chapter connects vector growth and deletion to stable logical identity, generational handles, transactional insertion, lookup, and the boundary between internal storage and external APIs.

**By the end:** Design a growing asset registry that tolerates relocation and removal while preventing stale or mis-typed handles from silently naming another object.

**Recall question:** When the vector moves its elements, what must stay stable for a caller holding an asset reference?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 8605–9916; source-span SHA256 6ea9315a09e1eb5e38d58a30455366791263a9caf95e707e7ce19c6b5f8e4316. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## An asset registry grows while its handles must not change

The last asset-pipeline chapter could plan a texture tile, choose a compile-time pixel format and policy, reject an invalid byte count, and record accepted work. But we deliberately left the asset depot with a fixed `Capacity`. That is a useful testing model; it is not how a long-running editor behaves. An editor loads ten meshes, then ten thousand. It closes a scene, removes some of them, hot-reloads others, searches for a texture by name, schedules hundreds of uploads, and allows rendering code to keep a reference to the *logical* asset while the underlying collection changes.

The first temptation is to replace `std::array<std::optional<T>,Capacity>` with `std::vector<T>` and declare the problem solved. That gives growth, but opens a second question: **does the address or iterator used to identify an asset remain valid after the vector moves its storage?** The answer is not always. The next temptation is to use a linked list for address stability, but then we give up contiguous iteration and often pay for extra allocations. Another tempting choice is a hash table for all names, but a hash table can rehash and invalidate its iterators, and a maliciously poor hash can turn an apparently constant-time lookup into a long scan.

These are not twenty unrelated library facts. They are decisions about three independent promises the editor must make: **where objects physically live**, **how logical identities survive movement and deletion**, and **which operations need sequential traversal, lookup, or scheduling**. Our running design will use contiguous storage where useful, a generation-aware logical handle to reject an old occupant, an owned string for persistent names, a hash index for name lookup, and standard adapters for the task queue. We will compare the alternatives at the exact points where their tradeoffs matter. The companion executable is [[tools/labs/asset_container_lab.cpp|the C++20 growing asset registry laboratory]].

### A vector has three quantities, but only one of them counts living elements

Start with a dense collection of integers representing indices or compact asset records. A `std::vector<int>` is a dynamically growing sequence whose elements are contiguous. On typical implementations the vector object contains metadata describing a pointer, a size and an end-of-capacity boundary; **that three-pointer arrangement is an implementation model, not a mandated class layout**. What the C++ interface guarantees is enough for our reasoning:

- `size()` counts the constructed, live elements in the current sequence.
- `capacity()` is the number of elements that can be held without another allocation of the element storage.
- `data()` points to the beginning of the contiguous element range when applicable; element `i` is reachable through `data()+i` for indices within `[0,size())`.

In other words, reserving physical capacity is not the same as constructing objects in that capacity. Suppose we begin with an empty vector and call `reserve(2)`. The vector is still empty: its size is zero, and accessing `v[0]` is invalid even though the vector has enough reserved memory for two future `int` elements. After pushing 10 and 20, its live range has size two. Pushing another element when `size()==capacity()` requires new element storage and relocation of the existing values; after that operation, the new live range contains 10, 20, 30.

The tested code records the capacity before and immediately after that third insertion. On the local Linux standard-library implementation it printed `vector capacity growth: 2 -> 4`. That number is a **measurement**, not an invariant of the C++ standard. Other implementations can select different capacities. The portable fact is that `capacity()` must be large enough for the new element and that the old storage may be replaced when the vector has reached its existing capacity.

```cpp
std::vector<int> values;
values.reserve(2);                    // capacity >= 2, size == 0
values.push_back(10);                  // size == 1
values.push_back(20);                  // size == 2

const auto before = values.capacity();
values.push_back(30);                  // requires reallocation if size was capacity
const auto after = values.capacity();  // after > before in this case

values.resize(5);                      // now creates two more live int objects
// values[3] == 0 and values[4] == 0 for this allocator/value type.
```

This is where `reserve` and `resize` separate. `reserve(n)` requests *space* for at least `n` elements, leaving `size` untouched. `resize(n)` changes the number of *live* elements: growing constructs new ones, and shrinking destroys those removed. For ordinary `int` with the default allocator, appended value-initialized entries are zero. For a nontrivial asset type, `resize` needs a valid construction path and can have very different costs. Calling `reserve` before a known batch can avoid repeated allocations; calling `resize` to prepare capacity when no objects are intended is a semantic error. Shrinking size does not necessarily shrink capacity. `shrink_to_fit` requests a reduction but does not promise an exact amount or timing. A bad practice is to call `reserve(size()+1)` before every insertion, which can defeat geometric growth and cause repeated relocations.

One important nonstandard-looking exception to "vector is a normal contiguous array of its value type" is `std::vector<bool>`. It is a specialized representation that may pack Boolean values into bits and uses proxy references rather than ordinary independent `bool&` elements. A graphics engine's tightly packed mask should choose an explicit bit-packing abstraction when representation is part of the contract; do not infer that `std::vector<bool>` gives the same address/view semantics as `std::vector<std::uint8_t>`.

### Why vectors usually grow geometrically: derive the cost instead of repeating “amortized O(1)”

Allocating a new block for *every* append would be expensive. If a sequence grows from zero to `n` elements, and on each append we move every existing element into a precisely sized replacement block, the total number of moved elements is:

\[
0+1+2+\cdots+(n-1)=\frac{n(n-1)}{2}.
\]

This is quadratic in `n`. It is not a reasonable growth strategy for a large asset array.

A common implementation instead chooses a new capacity that is a factor `g>1` larger when the vector fills. If the capacities are approximately `1,g,g^2,\ldots,g^k`, the total number of previously stored elements copied or moved across those reallocation events is a geometric series:

\[
1+g+g^2+\cdots+g^{k-1}
=\frac{g^k-1}{g-1}.
\]

Since the last capacity is on the order of the final element count, the total relocation work over all appends is proportional to `n` for fixed `g>1`. Divide total work by `n` appends and the *amortized* relocation cost per append is constant, even though one particular append may relocate a large existing sequence. For a factor around two, the geometric series of prior capacities is bounded by a small multiple of the final size; for a smaller growth factor the storage overhead may be lower but the number of reallocations higher. These are tradeoffs, not standardized growth factors.

**Amortized complexity** is not the same as every operation being worst-case constant time. A single append that grows storage can be linear in the old size. That distinction matters if the operation occurs on a frame-critical thread with a strict latency budget. You might preallocate the expected high-water mark, use a ring buffer, use a paged registry, or move allocations into a noncritical phase. Choosing a geometric factor yourself also entails working-set and memory-retention tradeoffs. The C++ standard guarantees the relevant vector operation complexity, but does **not** mandate the specific ratio, or promise that copying and moving an arbitrarily expensive asset type costs a fixed number of cycles.

An engine's data-oriented design is shaped by locality as well as asymptotic complexity. A vector of small records that are read sequentially may use caches efficiently. A vector of pointers into separately allocated objects might preserve the vector's contiguous **pointers** while still requiring many cache misses to reach the objects. Physical contiguity belongs to the actual `T` elements in the vector; it does not recursively make every allocation reachable through `T` contiguous.

### A growing vector invalidates addresses, not logical identity—if we design identity correctly

Our old fixed-capacity depot returned a slot number. A naive replacement might return `Asset*` or retain a `std::span` of the vector's elements. That can work within a carefully bounded interval, but it breaks when a later reallocation moves elements.

When `std::vector` reallocates, all iterators, references, and pointers into its previous element storage are invalidated. Calling an old iterator or dereferencing an old pointer after reallocation is not a way to ask the vector for the new location. **We do not run intentionally invalid accesses in the laboratory.** Instead, the test checks that the capacity-changing operation occurred and states the standard's resulting invalidation rule. The distinction between observing the event and deliberately executing undefined behavior is part of the required engineering proof.

When a vector appends *without* reallocation, existing element references/iterators remain valid, but the past-the-end iterator is invalidated. Inserting or erasing at an interior position can invalidate iterators and references at or after the point because the following elements shift. `pop_back` invalidates the removed element and the old end. `clear` ends the lifetimes of all elements. `reserve` invalidates all element references only when it actually changes capacity. Keep those rules attached to the operation; do not write "vector pointers never survive mutations" as a substitute for reasoning about the concrete mutation.

A **logical handle** addresses a different problem. It should identify the asset through a registry, not by carrying the current memory address. In our CPU teaching model, the handle is:

```cpp
struct AssetHandle {
    std::size_t slot;
    std::uint64_t generation;

    friend bool operator==(const AssetHandle&,
                           const AssetHandle&) = default;
};
```

The `slot` is an index into `std::vector<Slot>`. The generation identifies which occupant is expected at that slot. Suppose `rocks.mesh` is inserted into slot zero with generation one. If the vector later reallocates to hold many other slots, the numeric slot index zero still denotes the first *position of the new vector*. The registry checks the slot and generation afresh each time a handle is resolved. That is why a stable logical handle can survive physical relocation even though a previously borrowed `Asset*` cannot.

Now remove the asset. The registry destroys the occupant and increments that slot's generation before making it reusable. A new asset may occupy slot zero at generation two. The stale handle `{0,1}` fails, even though slot zero now contains a perfectly live *different* asset. This is the same ABA-like identity hazard we studied in ownership and ECS: equal numeric storage locations do not guarantee equal object identity across lifetimes. Our test verifies that the old handle cannot resolve and the new handle can.

The implementation handles one additional edge condition deliberately: a 64-bit generation must not silently wrap and accidentally recreate an ancient ID. The example rejects recycling a slot whose generation has reached `UINT64_MAX`, rather than wrapping. In a production system, the type width, exhaustion policy, registry lifetime and cross-session identity all need explicit design. The test registry does not implement concurrent access, persistence, device completion or globally unique IDs. Those are different contracts.

### A stable handle is not a transactional insertion algorithm by itself

The registry stores a `std::vector` of slots, an additional vector of reusable free-slot indices, and a name-to-handle index. Inserting a new asset first validates nonempty name/data and uniqueness. If a reusable slot exists, it is selected; otherwise the slot vector grows. The code constructs a complete `Asset` containing an owning `std::string` name and `std::vector<std::uint8_t>` source payload. Only then does it insert a name-to-handle entry in the hash table. That entry is the registry's publication path for name lookups.

Why do we need more than "construct, then insert"? Hash-table insertion can allocate or throw. If an exception occurs *after* the slot has become occupied but *before* the name index is updated, a naive implementation leaks an unreachable live slot into its own registry state. The lab therefore surrounds the publication operation with a rollback path. If the hash insertion or deliberate commit-stage injection throws, it resets the unpublished slot. If the slot was newly appended, it also pops the unused last slot; if it was selected from the free list, it leaves that index free for another attempt. The test injects two separate failures: before slot mutation and after construction but before name publication. Both preserve the previously published asset and do not increase the persistent slot count.

```cpp
// Simplified structure of the actual tested publication protocol:
if (!reusing_free_slot)
    slots_.emplace_back();      // may allocate; no published entry added

Slot& pending = slots_[index];
pending.value.emplace(Asset{std::move(name), std::move(bytes)});

try {
    names_.emplace(pending.value->name, AssetHandle{index, pending.generation});
} catch (...) {
    pending.value.reset();      // destroy the unpublished candidate
    if (!reusing_free_slot)
        slots_.pop_back();      // restore the prior slot count
    throw;
}
if (reusing_free_slot)
    free_.pop_back();           // commit retirement of the free slot
```

The complete code has additional validation, duplicate handling and a test injection flag. The rollback depends on specific operations being nonthrowing on this chosen element type, which the lab checks with `std::is_nothrow_move_constructible_v<Asset>`. The program also arranges free-list capacity before removal. That supports a meaningful *single-threaded observable-state* guarantee for the demonstrated paths, but it does not prove every allocator, custom hash, throwing equality operator, or reentrant observer is handled correctly. If a user-installed comparator or callback can throw after publishing an external side effect, a more extensive transaction or compensation protocol is necessary. Do not extrapolate the test's result beyond the operations it actually exercises.

Deletion is also a protocol, not simply `erase(name)`. The example first reserves a place in the free list and removes the name index, then ends the occupant's C++ lifetime, increments the generation, and records the slot for reuse. Every outstanding raw view into that deleted `Asset` becomes invalid. A logical handle to that exact generation becomes stale. In a renderer, the GPU may still be using the native resource after the CPU asset's logical deletion, so the native object may require deferred retirement. The CPU handle registry by itself cannot prove it is safe to free GPU memory at this point.

### Why a fixed array is still the right answer in some parts of this engine

Not every collection needs dynamic growth. The tile planner's four-element channel metadata, one small camera basis, or a fixed group of diagnostic counters may have a known number of elements. `std::array<T,N>` owns exactly `N` elements, stores them contiguously within the array object, supports indexed access and does not add a dynamic growth protocol. Its size is part of its type; `std::array<int,3>` and `std::array<int,4>` are different types.

In the laboratory, `std::array<int,3>{2,4,6}` is a fixed contiguous sequence, and `data()[2]` denotes the third element. The array does not choose a heap allocation by itself, but where the containing object lives still depends on its own storage duration and ownership. An `std::array` that is a member of a heap-allocated object is stored inside that heap object; an automatic local one commonly resides in automatic storage. Saying "std::array is always on the stack" is inaccurate.

A zero-length array specialization has its own documented iterator/data subtleties; the C++ standard does not make an arbitrary zero-sized array a useful source of a dereferenceable first element. For graphics buffers, array bounds and alignment remain explicit. A fixed `std::array` is excellent for the program-known four-channel metadata; it is not appropriate for an editor's unbounded list of user-imported textures. Choosing between vector and array follows from the collection's **growth contract**, not from a simplistic claim that one is always faster.

### A queue of both recent and old work needs inexpensive front and back operations

The editor also accumulates a stream of user events or network packets. It may append events at the back and consume them from the front. Removing the first element of a vector shifts all later elements toward the beginning, a linear operation. If this is a dominant operation, a `std::deque<T>` is often a better fit.

`deque` means **double-ended queue**. It provides indexed random access and efficient insertion/removal at both ends, but it is **not one contiguous `T[]` range** in the same way as `vector`. Typical implementations use segmented blocks and a structure to locate those blocks; the exact block size and representation are not standardized. The lab pushes 2 at the back, 1 at the front, and 3 at the back, then checks indexed order 1,2,3. This demonstrates its sequence semantics and both-end operations, not a particular physical block layout.

The invalidation rules differ importantly from vector. End insertion into a deque can invalidate **iterators**, even though references to existing non-erased elements remain valid under the specified operations. Middle insertion and erasure have more disruptive rules. `deque` is therefore not a universal answer to pointer/iterator stability, and a `span` cannot be made over an arbitrary whole deque as though it were one contiguous vector. Its segmented layout can increase pointer chasing and affect traversal locality, whereas vector's contiguous storage often wins for scan-heavy data. Use `deque` because the queue operation needs both ends, not because its name sounds more stable.

### Node-based linked structures trade locality for specific mutation guarantees

Some editor data change their order frequently while callers retain iterators to individual nodes. A `std::list<T>` is a doubly linked sequence abstraction with efficient insertion/removal at a known position and a splice operation that can transfer existing nodes under the operation's allocator requirements. Many implementations use one allocation per node with two link pointers, but the precise representation is not mandated.

The lab creates a list of operations `import` and `compile`, holds an iterator to `compile`, then **splices** an `upload` node from another list. The iterator to `compile` remains usable; it is not affected by transferring a different node. This is a valuable guarantee for algorithms that need to reorganize work without relocating each payload. It does not automatically justify list usage in a renderer. A linked traversal visits nodes with less spatial locality and more pointer indirection than a vector scan. Finding the position in the first place can cost linear time; constant-time insertion at a known iterator is not constant-time search.

`std::forward_list<T>` is a singly linked sequence abstraction. It commonly stores fewer links per node and offers only forward iteration. That changes the interface: to remove an element efficiently, we often need an iterator to the element **before** the one being removed, so operations are named `insert_after` and `erase_after`. The lab constructs 1,2,3 and calls `erase_after(begin())`, obtaining 1,3. The operation removes two because the supplied iterator names its predecessor.

Why not use forward_list everywhere if it has fewer links? Because its API sacrifices backward traversal, direct indexing, and common operations such as constant-time `size()`. For scanning dense renderable components, node allocation and pointer chasing usually deserve scrutiny. Linked structures are valuable when the real workload needs their mutation and iterator guarantees; "O(1) insertion" without including the search cost or allocator cost is a misleading argument.

### Asset name lookup is a separate job from storing the asset bytes

The registry has two fundamentally different access patterns. During rendering it may iterate all live asset records or construct dense component arrays. During an editor search it may ask "Where is `rocks.mesh`?" A sequential vector of all strings could answer the question by comparing every name, but that scales linearly with the number of assets in the worst case. We want a dedicated name index.

A `std::map<Key,Value>` maintains keys in comparison order. A `std::set<Key>` maintains unique keys, without a separate mapped value. Both are typically implemented using balanced search trees, but C++ specifies the relevant ordering and complexity contracts rather than mandating a particular red-black-tree node layout. For a balanced tree, the height grows on the order of `log₂ n`, giving logarithmic lookup/insertion/erasure. The map can provide ordered iteration, lower/upper-bound queries and predictable key comparison semantics. The set answers membership questions with ordered unique keys.

The laboratory's map stores names `mesh`, `shader` and `texture` and iterates them in their standard lexical comparison order; the set inserts IDs 8,3,8,5 and retains three unique values, beginning with 3. Those tests demonstrate the semantics, not the actual shape or rotations of a tree. Storing a key in a `map` also has a real ownership implication: an inserted `std::string` key owns its characters, while a `std::string_view` key may become invalid if the original bytes are destroyed. Ordered keys must obey a **strict weak ordering** relation; an inconsistent comparator can break the container's assumptions. Mutating a key in place would also break ordering, which is why associative-container key access is constrained.

Ordered containers have node-based storage, comparison costs and memory overhead. They are not categorically slower or faster than hash tables. They may win when the editor needs sorted traversal, range queries or deterministic order independent of hash bucket arrangement. If the actual requirement is only fast exact-name lookup, the next option is worth examining.

### A hash index finds names quickly on average—but its correctness is not its speed

`std::unordered_map<Key,Value>` and `std::unordered_set<Key>` provide associative lookup organized by a hash function and a key-equivalence relation. The map stores a value alongside each unique key; the set stores membership. A hash converts the key into a `std::size_t`-like hash value; the container uses that result to choose a bucket or another implementation-specific lookup location.

Two *equivalent* keys must produce the same hash value. But **different** keys may produce the same value: that is a **collision**. A correct hash table must still distinguish colliding keys by equality checks, not assume the hash alone proves identity. The laboratory uses an intentionally terrible hasher:

```cpp
struct ConstantHash {
    std::size_t operator()(const std::string&) const noexcept {
        return 0; // all names collide, deliberately
    }
};

std::unordered_map<std::string, int, ConstantHash> bad;
for (int i = 0; i < 24; ++i)
    bad.emplace("asset" + std::to_string(i), i);
```

Every name hashes to the same bucket. Lookups remain **correct**—the test retrieves `asset19` and gets 19—but the bucket containing `asset0` has all 24 elements. The number of collisions is an observable property of this chosen function and workload, not a claim about the standard library's default hasher. The example is a counterexample to the naive belief that unordered means guaranteed constant-time lookup.

Under typical hash distribution assumptions, insertion, lookup and deletion have **average constant-time** complexity. In the worst case, many keys collide and operations can become linear in `n`. The relevant hash *quality* depends on the key distribution and attacker model. A fast hash that clusters similar file paths may degrade in a real asset catalog; a cryptographic or keyed hash may be appropriate when untrusted users can choose adversarial keys, though it has different cost and threat-model considerations. Security, distribution, and performance should not be collapsed into one adjective such as "good". Always verify that the equality and hash functions are mutually consistent, including case-folding or path-normalization policies: if an equality relation declares two strings equivalent, they must have equal hashes.

### Load factor controls rehash pressure, not the absence of collisions

A hash table's **load factor** is:

\[
\alpha = \frac{n}{b},
\]

where `n` is the number of elements and `b` is the number of buckets. The `max_load_factor` setting establishes the desired upper bound after applicable operations and controls when growth is needed. If we want to support at least `N` elements while targeting maximum load `m`, the bucket count needs to be at least approximately:

\[
b \ge \left\lceil \frac{N}{m} \right\rceil.
\]

In the lab we set `max_load_factor(0.5f)` and later call `reserve(96)`, requesting enough bucket capacity for that many elements under the configured maximum. This request is not the same as reserving space for 96 contiguous objects. `unordered_map::reserve` is connected to bucket reorganization, not vector's element storage. Some implementations may grow or choose a bucket count using their own policies; the exact bucket number is not a portable constant.

When an unordered container **rehashes**, its iterators are invalidated. Pointers and references to elements generally remain valid across rehash under the container's specified guarantees; erasing the actual element destroys that element and invalidates views of it. This is a subtle difference from vector, where reallocation invalidates references to the former element storage. Our laboratory deliberately does not dereference any pre-rehash iterator after calling `reserve`. Checking the resulting `bucket_count` and `load_factor` is safe; accessing an invalidated iterator would not be.

Lower load factor can reduce average bucket occupancy but increase bucket metadata and memory use; it cannot repair a constant hash function. The terrible hasher still puts all 24 names in one bucket even if the table allocates hundreds of buckets. This is a useful research bridge: vary key distributions, number of assets, maximum load factors and hash algorithms; measure lookup latency distributions, total allocations and memory use. Do not claim hash lookup is always constant time, or that a lower load factor always improves overall throughput.

The actual registry uses `std::unordered_map<std::string, AssetHandle> names_` as a **name index**, while the vector holds the actual asset objects. It owns the string key because the index must remain valid after the temporary input buffer or file parser is gone. Its `find(std::string_view)` creates a temporary owned string for the lookup; that may allocate. A production table could use compatible transparent hashing and equality to support heterogeneous lookup without making that temporary, but the exact C++20 requirements and implementation support must be checked rather than assumed. Keeping this allocation explicit in the teaching code makes the source of cost visible.

### The order in which work executes is yet another independent decision

The editor now has assets, but it also has events: decode mesh A, upload texture B, compile shader C, process an error, or undo a command. Not every worklist needs random indexing or name-based lookup. Sometimes the public **access policy** is the whole reason to choose a container.

A `std::queue<T>` is a *first-in, first-out* adapter. Insert at the back, process from the front. In the lab we push 7 and then 8, confirm the first readable item is 7, pop it, and then see 8. A `std::stack<T>` is a *last-in, first-out* adapter, useful for undo histories, nested operations, or temporary traversal states: push 7, push 8, pop 8 before 7. Both are **container adapters**: they expose selected operations on top of an underlying container rather than providing full general iteration. Their named semantics are the API contract; whether they use a particular default underlying `deque` or another suitable container is an implementation/configuration detail.

A `std::priority_queue<T>` retrieves the highest-priority item according to a comparator; it is commonly implemented as a heap over a sequence such as `std::vector`. A binary heap supports access to the top in constant time and insertion/removal of the top with logarithmic complexity. Its underlying storage is not automatically a fully sorted array. If we have tasks at priorities 3, 1 and 3, which of the two priority-3 tasks comes first? **Priority queues do not promise stable ordering of equivalent-priority elements.**

The lab encodes a deliberate tie-breaking policy:

```cpp
struct UploadJob {
    int priority;
    std::uint64_t sequence;
    std::string name;
};

struct JobLess {
    bool operator()(const UploadJob& a, const UploadJob& b) const {
        if (a.priority != b.priority)
            return a.priority < b.priority; // larger numeric priority wins
        return a.sequence > b.sequence;     // earlier sequence wins a tie
    }
};
```

With two priority-3 jobs having sequence two and one, the job with sequence one is selected first, followed by sequence two; the priority-1 job follows. The comparator supplies the ordering that the adapter itself does not promise. Real job schedulers must also consider fairness, starvation, cancellation, dependencies and concurrent ownership. A `std::priority_queue` is not automatically a thread-safe work-stealing scheduler, and choosing a heap is not sufficient to implement a render graph.

### A span can inspect bytes without owning them—and that is precisely its danger

Decoding and validation often need to examine a contiguous subset of asset bytes. Copying the entire byte vector into every helper would increase allocation and bandwidth. Taking an unbounded raw pointer loses the length needed for safe range checks. C++20's `std::span<T>` gives a non-owning view of a contiguous sequence, conceptually a pointer plus an extent, with the exact representation left to the implementation. A dynamic-extent span records a runtime length; a fixed-extent span can put a known extent into its type.

In our laboratory the checksum helper is:

```cpp
std::size_t checksum(std::span<const std::uint8_t> bytes) noexcept {
    std::size_t sum = 0;
    for (std::uint8_t byte : bytes)
        sum += byte;
    return sum;
}
```

This accepts a vector-backed contiguous payload without taking ownership of it. A caller can pass the current live byte range, and the function iterates only the span's extent. The test checks that `{10,20,30}` sums to 60. The helper remains valid because the owning asset and its byte vector are alive and unchanged **during that synchronous call**. The span does not keep the asset alive, does not prevent the vector from reallocating, and does not synchronize another thread that might mutate the bytes.

A span's size makes some bugs easier to prevent, but it is not a general proof that its pointer is valid. If you return a span to a local vector, the vector's storage is destroyed at scope exit and the returned span dangles. If you store a span of a vector and then resize or append in a way that reallocates that vector, the span may dangle. If you erase an asset and then use a previously obtained byte span, the asset's payload may have been destroyed. C++20's range and borrowing concepts do not magically extend ownership here; lifetime is a separate contract. For GPU API calls, a span can be a convenient CPU input view, but an asynchronous upload must copy or otherwise retain its storage through the required completion interval.

### Paths and asset names need two different string types at two different boundaries

The registry's name lookup also exposes the important distinction between `std::string` and `std::string_view`. A `std::string` owns its character sequence and maintains its size, and can grow or shrink. On many standard-library implementations, **small-string optimization (SSO)** stores short strings within the string object's own representation instead of allocating a separate heap buffer. That can be extremely useful for short entity names or small path fragments, but the C++ standard does not mandate a particular SSO capacity or even that every implementation employ the same optimization. The lab prints the capacities of `"mesh"` and a 1,000-character string; on the local Linux environment they were `15/1000`. Those are observed capacities, not universal SSO thresholds or allocation counts.

A `std::string_view` does **not** own characters. It refers to a contiguous sequence of characters with a length. That is perfect for parsing or comparing a borrowed path while the owner remains alive:

```cpp
std::string owner = "texture/stone/normal.png";
std::string_view full{owner};
std::string_view part = full.substr(8, 5); // "stone", length 5

std::string retained{part}; // copy the five characters into owning storage
owner.clear();              // don't use part after this invalidates its content
// retained remains "stone".
```

The substring is not necessarily NUL-terminated at its end. Passing `part.data()` to a C API expecting a NUL-terminated string would be incorrect: the next bytes in the original string could continue into `/normal.png`. A `string_view` is not just a cheap `const char*`; it is a *pointer-and-length view*. The view's constructor or substring operation does not extend the underlying string's lifetime. Taking a view of a temporary `std::string` and retaining it for later is a common dangling-reference bug.

This directly explains our registry API choices. A synchronous `find(std::string_view)` can borrow an input path just for the lookup. A stored key in `names_` must own its characters, so the implementation copies or moves the string into `std::string` storage. A callback that retains a path for asynchronous work must also own the characters or have an explicitly longer-lived owner. Choosing the right type depends on **who uses the text and for how long**, not simply on the cost of one character copy.

Strings have their own iterator/reference invalidation rules when they mutate. An SSO implementation might change between internal and allocated storage as it grows, so do not retain a `string_view` into a mutable string across operations that can invalidate it. An empty string or zero-length view may have a valid size while its `data()` is not a dereferenceable character location; check length rather than assume every pointer designates a readable element.

### The container selection can be stated as an engineering contract, not a popularity ranking

The final registry design uses a vector because indexed slots give good dense storage and allow stable **index-based** identity across capacity growth; its elements are not stable raw-address identities. A free-list vector reuses vacant indices. A hash map provides average efficient name lookup with owned keys; it does not give deterministic iteration order or immunity to bad hashes. The byte payload within each asset is another vector because decoding and checksum operations need contiguous bytes. A `span` borrows that payload during an explicitly bounded call. A string view borrows a name only during lookup, while a string owns the name in the stored index. Queue/stack/priority queue adapters organize event handling according to scheduling semantics. Arrays, deques and linked lists remain alternatives where their particular fixed-size, double-ended or node-stability properties solve a real workload.

Do not read this as a fixed universal design. If the renderer iterates tens of millions of tiny components, a sparse-set or archetype ECS with compact structure-of-arrays storage may outperform this generic registry. If assets are asynchronously loaded and evicted, the registry will need thread-safe state transitions and deferred GPU retirement. If hash collisions are adversarial, hash function and key normalization become security concerns. If memory is fragmented, custom allocators or pools may matter. If stable addresses are required during concurrent access, a paged registry or carefully owned indirection may be appropriate. Those are **new requirements** for future system layers, not gaps to paper over with claims that a container solves everything.

The next concept should become necessary only after we understand what happens to *iterators* and *algorithms* when these containers change. For now, the standard contracts above are enough to choose the right representation for the demonstrated growing registry and recognize the failures that would invalidate borrowed state.

### Trace the full lifecycle and verify precisely what the code proves

Begin with an empty `AssetRegistry`. Insert `rocks.mesh` with bytes 10,20,30. It receives a logical handle containing slot zero and generation one. Look up its name and resolve the handle, then checksum its bytes to obtain 60. Next inject a preparation failure for `broken.mesh` before changing any registry structure. The published asset and total slot count remain unchanged. Inject a second failure **after** a candidate slot has been constructed but before the name index is committed. The rollback destroys that candidate and restores the prior persistent slot count. The old mesh is still discoverable.

Now erase the first asset. Its owned string and byte vector are destroyed, its name index entry disappears, and its generation advances. The old handle must no longer resolve. Insert `fresh.mesh` and deliberately reuse the same slot; the new handle has the same numeric index but a different generation. The test proves that the new asset resolves and the old one does not. This is the distinction between a physical slot and a logical occupant in executable form.

In the same run, vector growth prints 2→4 before the later resize creates five live integers; list splicing retains an unrelated iterator; forward-list removal uses a predecessor; ordered collections preserve key order and uniqueness; a deliberately pathological hasher sends 24 keys to one bucket without breaking equality; reserve/rehash exercises load-factor policy; priority scheduling honors an explicit tie break; queue and stack expose FIFO and LIFO behavior; a span reads live bytes synchronously; and a string-view substring is copied into ownership before the source is mutated. None of the tests dereferences an invalidated iterator or dangling view.

The complete source is [[tools/labs/asset_container_lab.cpp|the M015 C++20 asset-container laboratory]]. Its actual local commands were:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_san
/tmp/allmight_m015_san

clang++ -std=c++20 -O1 -Wall -Wextra -Wpedantic -Werror \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_clang
/tmp/allmight_m015_clang

g++ -std=c++20 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_release
/tmp/allmight_m015_release
```

Each executable reported `asset_container_lab: all checks passed`. The GCC sanitizer run reported no address or undefined-behavior diagnostics. The locally executed file is 292 lines, 13,525 bytes, with Git blob `c9a323bcf4a3f2e41df13db9d10d381bdd71468f`; the committed source was checked against that exact blob. These results support the **specific representative CPU-side tests**. They do not establish stress-tested concurrency, a portable allocator performance ranking, exhaustive exception injection for arbitrary user hashers, a stable ABI layout, true GPU lifetime safety, or the current implementation of the separately named Kairo codebases.

**Blank-screen reconstruction:** derive the difference between vector size and capacity, the amortized geometric-series cost and the invalidation rules of appending with versus without reallocation. Explain why a raw pointer cannot safely serve as a long-lived asset identity, then implement a slot-and-generation handle with a retire/reuse check. Choose a contiguous store, an ordered name index and a hash index for different requirements, showing what you gain and lose in lookup order, locality and worst-case behavior. Construct a constant-hash collision demonstration and derive the load-factor equation. Use a deque for both-end work, list splice for node transfer and forward_list's predecessor-based removal; then build FIFO, LIFO and priority scheduling with explicit tie semantics. Finally explain why a span of bytes and a string view are *borrowed*, why their lifetime cannot outlast their owner or a reallocation, and why short-string optimization cannot be inferred as a portable fixed-size promise. Recreate the two injected transaction failures and verify that an old handle never resolves after its slot is reused.

This is an **authored reconstruction** of M015, original Parts 231–250. Every original title is mapped to its first-needed explanation in [[Supplementary/Foundations#M015 — Parts 231–250: growing the asset registry without losing identities|the coverage ledger]], and the evidence boundaries are recorded in [[Supplementary/Sources and Code Anchors#Phase 5 additive reconstruction — M015 / Parts 231–250|the verification ledger]]. The associated [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#A growing asset store must preserve its promises — M015 retrieval|recall exercise]] asks the reader to rebuild the mechanism, not repeat a glossary. The original curriculum's `KairoECS/SparseSet.cppm` and `KairoRenderer/RenderGraph.cppm` remain **project-specific audit targets**, not claims about their currently inspected source.

## From a living asset registry to reliable queries and transformations

Our growing asset registry can now accept a new mesh, reuse a deleted slot without accepting a stale handle, look up an owned filename, and borrow a byte range for a synchronous operation. The last chapter left us with a question that an editor and renderer encounter immediately: **what happens when we must inspect, rank, filter, search, summarize and remove hundreds of assets without accidentally mutating the live registry or returning an ambiguous result?**

Imagine that the editor needs a report for the next frame. The live registry contains `brick.mesh`, `sky.tex`, `ui.tex`, `terrain.mesh` and `rock.mesh`. It should find a named asset, explain why a lookup failed, group mesh resources, sort work by upload priority, find assets occupying exactly 48 bytes, sum the total payload size, and discard texture records from a *report* without deleting any asset from the actual engine. That is a single coherent task with several distinct correctness obligations. The report must not retain references into a registry that may be edited or reallocated. Binary search must not run on data in the wrong order. A summation must not change meaning merely because an algorithm decides to combine its operands differently. And an algorithm that moves values out of a range must not be mistaken for one that destroys the corresponding objects.

We will solve this task with an **owned snapshot of asset metadata**. A snapshot is a vector of values containing a handle, name, byte count, priority and kind for each observed asset. Copying the metadata makes sorting and erasing the report independent of the live registry's physical storage. That copy is not a transactionally coherent concurrent read of the live registry by itself. If the real registry can mutate on another thread, the snapshot must first be created under a suitable synchronization/versioning protocol. The snapshot may also contain a handle that becomes stale *after* capture; resolving a handle back into live state must still check its generation. Those limitations are part of the design, not minor footnotes.

This problem introduces the original M016 / Parts 251–270, but it will not become twenty isolated algorithm definitions. Each mechanism appears because the report requires it. The complete, runnable C++20 witness is [[tools/labs/asset_query_algorithm_lab.cpp|the asset-query and algorithm laboratory]].

### A missing asset is different from an invalid request

The editor asks for `rock.mesh`. The snapshot may contain it or not. In some operations those are the only two meaningful outcomes: a value is present, or it is absent. Returning a default-looking handle such as slot zero with generation zero is a poor choice because it forces every caller to remember whether that combination means something special. Even if we reserve the value today, future changes to slot and generation policy can silently break the sentinel convention.

`std::optional<T>` expresses the intended result directly: **either a live `T` value is contained, or no `T` value is contained**. It is itself an object whose active state is tracked; `std::nullopt` expresses the empty state.

```cpp
std::optional<AssetHandle> find_optional(
    const std::vector<AssetRecord>& assets,
    std::string_view requested_name)
{
    const auto it = std::find_if(
        assets.begin(), assets.end(),
        [requested_name](const AssetRecord& asset) {
            return asset.name == requested_name;
        });

    if (it == assets.end())
        return std::nullopt;

    return it->id; // copy a logical handle, not a pointer into the snapshot
}
```

We already know `std::string_view` borrows the query's characters. Here the view is used during a synchronous call and is not retained by the returned result. The returned `AssetHandle` contains a slot and generation **by value**. It does not borrow the asset name or a vector element. The snapshot has its own owning strings, so even if the query string came from a temporary caller-owned buffer, this function does not store that borrowed reference.

An optional is good when absence is an ordinary answer. But suppose the caller passes an empty name. Is that equivalent to a real, nonempty name that simply is not present? Not necessarily. The editor may want to show **invalid input** for the first case and **not found** for the second. Encoding both as `std::nullopt` loses information. That is where a typed success-or-error result becomes necessary.

### One value or one explanation, without pretending C++20 already has std::expected

We define a small error enumeration:

```cpp
enum class LookupError {
    EmptyName,
    NotFound
};
```

The successful value is `AssetHandle`. The failure is a `LookupError`. A C++20 program can model that with `std::variant`, which we introduced when rendering commands were one of several known request shapes. The lab builds a deliberately minimal `ExpectedLike<T,E>` wrapper around `std::variant<T,Unexpected<E>>`. The extra `Unexpected<E>` wrapper distinguishes error construction from success construction even if the template arguments could be otherwise confusing.

```cpp
template<class E>
struct Unexpected {
    E error;
};

template<class T, class E>
class ExpectedLike {
    std::variant<T, Unexpected<E>> state_;

    explicit ExpectedLike(T value)
        : state_(std::move(value)) {}

    explicit ExpectedLike(Unexpected<E> failure)
        : state_(failure) {}

public:
    static ExpectedLike success(T value) {
        return ExpectedLike(std::move(value));
    }

    static ExpectedLike failure(E error) {
        return ExpectedLike(Unexpected<E>{error});
    }

    bool has_value() const noexcept {
        return std::holds_alternative<T>(state_);
    }

    const T& value() const {
        return std::get<T>(state_);
    }

    const E& error() const {
        return std::get<Unexpected<E>>(state_).error;
    }
};
```

This interface uses a common **expected-style** pattern: if success is present, inspect the value; otherwise inspect a typed error. It is not a full implementation of `std::expected`. It omits several operations and contracts of the standard type: constructors, monadic operations, `void` success handling, reference/value category overloads, exception behavior of various accessors, comparisons and more. Standard `std::expected<T,E>` is a C++23 library facility, so we do **not** claim the C++20 standard supplies it. In a real C++23 build, prefer the standard implementation once toolchain support is confirmed; in a C++20 engine, a reviewed library implementation or a clearly bounded wrapper may be appropriate.

The lookup now has three distinguishable observations. Requesting `rock.mesh` yields success and handle `{4,1}`. Requesting an empty name yields `LookupError::EmptyName`. Requesting `other.mesh` yields `LookupError::NotFound`. A caller must check `has_value()` before calling `value()`. In this teaching wrapper, calling the wrong accessor leads to `std::bad_variant_access` from `std::get`. That is an API precondition we explain explicitly, not a silently assumed success.

Why not use exceptions for every missing name? In a frequently used query, absence might be a normal control-flow outcome rather than an exceptional failure. Typed results make expected branches visible in the function's return type. Exceptions are still useful when a boundary needs stack unwinding and cannot locally recover, as the Phase 2 resource-ownership chapter derived. A correct engine should choose the error model by the operation's semantics and document whether allocation errors or user callback failures can still escape. Returning an expected-like object is not a proof that the operation cannot throw.

### Some requests are one of several command shapes; that is not the same as a missing value

After looking up an asset, the editor may ask the engine to inspect it, upload it or evict it. Unlike `optional`, these are not variations of "present or absent." They are three distinct commands with different payload meanings. The previous renderer request story therefore reappears naturally:

```cpp
struct Inspect { AssetHandle id; };
struct Upload  { AssetHandle id; };
struct Evict   { AssetHandle id; };

using AssetCommand = std::variant<Inspect, Upload, Evict>;
```

A visitor selects the handler for the **active alternative**. The lab uses an overload-set helper to combine three lambdas:

```cpp
template<class... F>
struct Overloaded : F... {
    using F::operator()...;
};
template<class... F>
Overloaded(F...) -> Overloaded<F...>;

// In the lab, each lambda increments its own command counter:
std::visit(Overloaded{
    [&](const Inspect& command) { /* inspect the handle */ },
    [&](const Upload& command)  { /* plan an upload */ },
    [&](const Evict& command)   { /* request eviction */ }
}, an_asset_command);
```

The `using F::operator()...` declaration makes each inherited callable overload visible to overload resolution. The deduction guide lets the compiler infer the helper's template arguments from the supplied lambda closure types. For a non-generic visitor with explicit alternatives, adding a new command type requires an appropriate matching operation. The compile-time error is useful: it forces a change in our processing contract when the command vocabulary expands. A `variant` can become `valueless_by_exception` under certain replacement operations, so a production visitor handling potentially throwing alternative changes must acknowledge that exceptional state. The lab uses a fixed set of trivially movable command alternatives and checks one invocation of each handler.

This is exactly why our learning flow distinguishes `optional`, `variant` and the expected-style wrapper. **Optional:** an answer may be missing. **Variant:** a value is one of several known alternatives. **Expected-style result:** an operation has succeeded or has a typed error reason. These can all use tagged storage, but the public meanings are different.

### The diagnostics system needs a different escape hatch: std::any

The engine's normal command vocabulary should not casually accept any arbitrary type, because rendering and resource operations need a closed, testable set of alternatives. But consider a diagnostic attachment that an editor plugin may supply: a source filename, a numerical counter, or a backend-specific detail. We may not know all attachment types when compiling the host. `std::any` is a runtime type-erased holder for an owned value of a copy-constructible type.

```cpp
std::any diagnostic = std::string{"texture-rgba"};

const auto& text =
    std::any_cast<const std::string&>(diagnostic);

// This does not convert a string to an int:
try {
    (void)std::any_cast<int>(diagnostic);
} catch (const std::bad_any_cast&) {
    // The stored dynamic type was not int.
}

diagnostic.reset(); // destroy the stored value
```

Unlike `variant`, which enumerates permitted alternatives in its static type, `any` hides the concrete stored type behind runtime type information. A wrong `any_cast` throws `std::bad_any_cast` for the throwing overload; pointer forms can provide a nullable checked query. The wrapper may allocate for its target, depending on target size and library implementation; its copyability contract can reject a directly move-only target. It is not automatically an efficient serialization format, dynamic plugin ABI, or cross-process schema. In particular, a receiver must know which type it expects and trust that both sides agree on the meaning of the value.

The lab checks that a stored `std::string` is retrieved as a string, that retrieving it as `int` fails, and that resetting the holder removes the value. We keep `std::any` in the diagnostics boundary rather than in every hot renderer request. That design preserves explicit type coverage for core engine behavior.

### A report contains several related values; what is the correct shape?

The next request asks how many assets were examined and how many errors occurred. Returning an anonymous vector of two integers would force callers to remember that element zero means count and element one means errors. A `std::pair<A,B>` has exactly two typed fields named `first` and `second`. A `std::tuple<A,B,C,...>` generalizes the idea to a fixed number of heterogeneous fields, useful for passing or temporarily grouping values when a dedicated named record would be unnecessary.

```cpp
std::pair<std::size_t, std::size_t> counts{5, 0};

auto [copied_count, copied_errors] = counts;
auto& [live_count, live_errors] = counts;

++live_errors;
// copied_errors is still 0; counts.second is now 1.
```

The `auto [a,b]` form creates a hidden decomposition object initialized from `counts`; here it **copies** the pair. The `auto& [a,b]` form binds the decomposition to the original pair, so changes through `live_errors` affect the original. A `const auto& [a,b]` gives an immutable borrowed view of the pair's members, provided the pair outlives that binding. Other binding contexts and move-only member types have their own initialization constraints; the important principle is that the declaration's reference qualifiers determine whether the decomposition owns a new value or refers to an existing one.

A tuple such as `std::tuple{std::string("frame-assets"), std::size_t{5}, 1u}` can carry a report name, count and version. `std::get<0>(tuple)` or a structured binding accesses its fields without runtime indexing of heterogeneous values. That is valuable for local composition, but a named `struct ReportHeader { std::string name; size_t count; unsigned version; };` often communicates domain meaning better across a stable public API. Tuples are product types: all listed fields coexist at once. Variants are sum types: only one alternative is active. Understanding the distinction prevents misusing `variant` for a record whose fields all need to exist simultaneously.

### Before searching anything, decide what kind of iterator the algorithm actually needs

Our snapshot is a `std::vector<AssetRecord>`, and `std::vector` offers random-access, contiguous iterators. But the earlier container chapter included `std::deque`, `std::list`, `std::forward_list` and input streams, each with different traversal guarantees. An **iterator** is a position-like object associated with a traversal operation, and an **iterator category or concept** describes which movements and accesses are legal. This is where the generic standard algorithms meet the physical storage decisions of Phase 5.

An input iterator can read values in one forward progression; it need not support multiple independent passes. A forward iterator can traverse repeatedly over the same sequence under its iterator requirements. A bidirectional iterator can also move backward. A random-access iterator supports arithmetic such as jumping to a position by an offset and comparing distance efficiently. A contiguous iterator adds the guarantee that its elements occupy consecutive memory positions compatible with pointer-like address behavior. An output iterator is an access pattern for writing values; it is not simply the next stronger kind of input iterator. These capabilities are expressed through C++20 concepts; older algorithm APIs also refer to named legacy iterator requirements.

Our lab checks concrete differences at compile time:

```cpp
static_assert(std::contiguous_iterator<std::vector<int>::iterator>);
static_assert(std::random_access_iterator<std::deque<int>::iterator>);
static_assert(!std::contiguous_iterator<std::deque<int>::iterator>);

static_assert(std::bidirectional_iterator<std::list<int>::iterator>);
static_assert(!std::random_access_iterator<std::list<int>::iterator>);

static_assert(std::forward_iterator<std::forward_list<int>::iterator>);
static_assert(!std::bidirectional_iterator<std::forward_list<int>::iterator>);

static_assert(std::input_iterator<std::istream_iterator<int>>);
static_assert(!std::forward_iterator<std::istream_iterator<int>>);
```

That explains why we can sort a vector with `std::sort` but cannot invoke the same overload directly on `std::list` iterators, which do not meet its random-access requirement. A list instead provides its own `list::sort` operation suited to its node structure. It also explains a subtle binary-search cost: `std::lower_bound` can work with forward iterators and achieve logarithmic *comparisons*, but on non-random-access iterators, advancing across the sequence may still take linear total iterator steps. "Binary search is O(log n)" is incomplete unless we say what operation's cost we mean and what iterator capabilities we have.

The lab constructs a vector from `std::istream_iterator<int>` reading the stream `"7 8 9"`. Reading the stream consumes the input; repeatedly trying to traverse an already consumed input source as if it were a random-access array would be a different operation. We do not introduce iterator adapters as decorative syntax: `std::back_inserter(vector)` later gives algorithms an output iterator that appends to a vector, so the algorithm does not need to calculate or preconstruct output elements.

Iterator **invalidation** remains part of this discussion. If we sort a snapshot, iterators still refer into its live storage while the algorithm runs under its preconditions, but the *values at positions* can change through swaps and moves. A caller that assumes an iterator still denotes the same logical asset after sorting may make a semantic mistake even if its iterator object remains technically valid. After erasing vector elements, references and iterators at or after the erase point can be invalidated. An owning value snapshot is therefore useful partly because its transformation can be isolated from live registry handles and side-effectful render consumers.

### The upload report needs sorted work, but what does “sorted” mean?

Suppose the report shows the assets by byte count, from smallest to largest. We make a **separate copy** of the owned snapshot, then sort it:

```cpp
auto sorted = assets; // copy the metadata snapshot

std::sort(sorted.begin(), sorted.end(),
    [](const AssetRecord& a, const AssetRecord& b) {
        if (a.bytes != b.bytes)
            return a.bytes < b.bytes;
        return a.name < b.name;
    });
```

Why compare names after equal byte counts? Because the editor may want a deterministic secondary order. The source snapshot contains two assets with 48 bytes: `brick.mesh` and `rock.mesh`. A sort comparator that looks **only** at `bytes` considers them equivalent and is permitted to reorder them. Here a lexical name comparison explicitly breaks the tie, giving a total ordering for these records when the names themselves are unique. A deterministic tie-breaker is a semantic requirement, not something we should hope an implementation's internal quicksort happens to preserve.

A comparator used with `std::sort` must satisfy a **strict weak ordering** relation. In practical terms, `comp(x,x)` must be false; if `x<y` and `y<z` under the comparator then `x<z` must hold; and the induced equivalence groups must behave consistently. Accidentally writing `return a.priority >= b.priority` would violate the irreflexivity requirement when the values are equal. A floating-point key containing NaNs also requires a deliberate policy if the comparator's effective relation does not meet the algorithm's ordering preconditions. The compiler cannot generally discover a bad semantic comparator, and a test passing once does not establish the relation for every possible asset.

What does `std::sort` actually promise? For `n` elements it provides the specified comparison complexity bound and produces the requested ordering under the comparator. Many implementations use an **introsort-like** hybrid: start with quicksort-style partitioning, switch when recursion depth threatens poor worst-case behavior, and use insertion sort for small partitions. This is useful implementation intuition, not a normative requirement that the standard library must use an algorithm with that exact name or internal threshold. Sorting normally mutates the values within the supplied range. In our case it is an owned snapshot; this mutation does not change the live asset registry's index.

A priority report introduces a different requirement. Consider the original sequence of priority values:

| Original position | Asset | Priority |
|---|---|---:|
| 0 | brick.mesh | 2 |
| 1 | sky.tex | 3 |
| 2 | ui.tex | 3 |
| 3 | terrain.mesh | 1 |
| 4 | rock.mesh | 2 |

If we group by descending priority but want **equal-priority assets to keep their original relative order**, the precise contract is *stable sorting*. `std::stable_sort` preserves the original order of elements that compare equivalent under the supplied comparator. The sorted prefix therefore has `sky.tex` then `ui.tex` for priority 3, followed by `brick.mesh` then `rock.mesh` for priority 2. Our lab tests that exact outcome. `std::sort` does not promise it for equal keys.

Stable sort may use additional temporary memory and may have different algorithmic and allocation behavior than ordinary sort, depending on the implementation and available buffer. Its documented complexity conditions must be checked rather than replaced with "stable sort always uses mergesort." In an asset tool, retaining deterministic user-visible order can be more important than the last fraction of sorting throughput. In a render hot path with a stable, consistent tie-breaker encoded in the comparator, ordinary sort might be sufficient. The right operation follows from the report's **ordering contract**, not a popularity ranking.

### Sometimes we only need to separate meshes from textures, not sort everything

The editor asks for a temporary list with meshes first and all non-mesh assets afterward, so it can perform a mesh-specific pass. Sorting every asset by a total order would solve more than we need and could waste work. `std::partition` instead rearranges the sequence so that elements satisfying a predicate come before the elements that do not. It returns the boundary iterator between the two groups.

```cpp
auto partitioned = assets;
auto boundary = std::partition(
    partitioned.begin(), partitioned.end(),
    [](const AssetRecord& asset) {
        return asset.kind == AssetKind::Mesh;
    });
```

All elements before `boundary` must be meshes; all elements after it must fail the mesh predicate. The operation does **not** promise that the original relative order inside each group is preserved. That would be `std::stable_partition`'s additional contract. The lab verifies the two halves with `std::all_of` and `std::none_of`, then calls `std::partition_point` on the already partitioned range to check the boundary. The latter's precondition matters: applying partition-point logic to an arbitrary unpartitioned sequence cannot be justified simply because all its elements happen to be valid objects.

Partitioning is an important building block in many algorithms, from in-place filtering to quicksort-like pivots and some broad-phase collision organization. But a **partitioned** sequence is not necessarily **sorted**. We cannot assume that finding the mesh boundary has made every mesh's byte count increasing, nor can we run byte-count binary search on that arbitrary partition without first establishing its required byte-count ordering.

### Once byte counts are ordered, we can find an entire equal-size range

Our owned snapshot's byte sizes are 48, 64, 16, 128 and 48. After sorting by byte count they are:

\[
[16,\ 48,\ 48,\ 64,\ 128].
\]

The editor asks for all assets whose source payload is exactly 48 bytes. A linear scan would work, but because we have *already sorted by this key*, we can search the boundaries of the equal-key range.

`std::lower_bound` locates the first position where the element is **not less than** the query under the specified relation. In our sequence, the lower bound for 48 is index 1. `std::upper_bound` locates the first element **greater than** the query; for 48 that is index 3. The half-open range `[1,3)` therefore contains the two matching 48-byte assets and excludes the 16-byte and 64-byte neighbors. `std::equal_range` provides the matching pair of boundaries in one interface.

In C++ the comparator signatures for lower and upper bounds are easy to confuse, because one operation may call `comp(element,value)` while the other may require `comp(value,element)`. The lab supplies both directions:

```cpp
struct CompareBytes {
    bool operator()(const AssetRecord& a,
                    std::size_t requested) const noexcept {
        return a.bytes < requested;
    }

    bool operator()(std::size_t requested,
                    const AssetRecord& a) const noexcept {
        return requested < a.bytes;
    }
};

auto first = std::lower_bound(sorted.begin(), sorted.end(),
                              std::size_t{48}, CompareBytes{});
auto last = std::upper_bound(sorted.begin(), sorted.end(),
                            std::size_t{48}, CompareBytes{});
auto range = std::equal_range(sorted.begin(), sorted.end(),
                              std::size_t{48}, CompareBytes{});
```

The tests confirm that `std::distance(begin,first)==1`, `std::distance(begin,last)==3`, and `equal_range` returns the same two iterators. A query for 50 has no exact match but its lower bound is index 3, the insertion position before 64. This is why lower/upper bound are more general than a function that only reports "found" or "not found." They are also useful for inserting into a maintained sorted sequence, counting duplicate keys, and selecting ranges of records with a shared key.

The proof depends on a nonnegotiable precondition: the input range must be partitioned appropriately according to the comparison made by the algorithm; a fully sorted vector under that same comparison is the common sufficient condition. Using a comparator inconsistent with the sort order, or sorting by priority and then searching by bytes, invalidates the reasoning. The tests intentionally sort by bytes immediately before these searches rather than relying on the earlier copy that was sorted using a different comparison.

For a random-access iterator such as vector's, repeatedly halving a search interval requires logarithmically many comparisons and efficient midpoint movement. With only forward iterators, comparisons may remain logarithmic but iterator increments may be linear across the range. The question "Is binary search faster than find?" must therefore be answered with the cost of creating/maintaining the sorted input, the number of queries, and the iterator category, not with two detached big-O symbols.

### A one-off unsorted name query may still be better as a simple linear search

Suppose the report needs to check whether it contains the name `sky.tex`, but the snapshot is sorted by byte count or priority, not by name. `std::find` performs equality comparison over a range and does not require sorting. The lab first transforms the asset records into a vector of owned names and then applies `std::find`, finding `sky.tex` while correctly rejecting `absent`.

A linear scan costs at most proportional to the number of examined elements. That can be competitive for a small snapshot, particularly when its data are contiguous and cache-resident. Building a new hash map for one query may require allocations and hashing work that exceed the cost of a short scan. If the editor performs thousands of exact-name queries, our **live registry's existing name-to-handle hash index from M015** is the more natural starting point, provided the registry's synchronization and identity contracts are satisfied.

This is the actual engineering choice: **do not rebuild a heavyweight index for every one-off report**, but do not scan millions of asset names repeatedly when the live registry already maintains a suitable index. Use source-independent query snapshots when mutation or stable reporting order makes that isolation valuable.

### The report wants just byte counts; transform rather than write a second complicated loop

The next step is to compute a total payload size. We could write a loop that walks each `AssetRecord` and adds its `bytes` field immediately. But the editor may also need a histogram, a size-sorted report, or to hand the numeric values to a separate statistics function. We can separate **projection** from **aggregation** by transforming records into byte counts:

```cpp
std::vector<std::size_t> sizes;
sizes.reserve(assets.size());

std::transform(
    assets.begin(), assets.end(),
    std::back_inserter(sizes),
    [](const AssetRecord& asset) {
        return asset.bytes;
    });
```

`std::transform` applies an operation to each input element and writes the result into an output range. The `std::back_inserter` adapter makes assignment append to the vector through `push_back`, so the output vector does not need five default-constructed destination elements before we begin. We reserve space because the input count is known; reserve changes capacity but not size, which the earlier chapter proved. The resulting values are `[48,64,16,128,48]`, exactly the source-byte counts of the snapshot in original order.

The term **projection** here means extracting or computing a selected value from a larger record, not a geometric camera projection. The operation in our example is pure and has no external side effects. That makes its semantics easy to test and facilitates reuse. The ordinary `std::transform` overload used in this lab is not a promise of parallel execution or SIMD vectorization; the compiler may optimize it, but that requires evidence. Some overloads accept execution policies, introducing additional constraints about data races, exceptions and ordering, but those are not needed for the present synchronous query.

An overlapping input/output arrangement must satisfy the algorithm's specified iterator and mutation requirements. Using a back inserter into the **same vector that is currently being iterated over** can reallocate and invalidate the input iterators. Our lab intentionally writes into a separate vector. That choice is not merely aesthetic; it preserves the data and iterator lifetime contract while the report is being built.

### Adding a sequence is not always the same operation as reducing it in any order

Our byte counts are:

\[
48+64+16+128+48 = 304.
\]

With integers in a sufficiently wide type, this is straightforward. The lab uses `std::accumulate` with a `std::size_t{0}` initial value:

```cpp
auto total = std::accumulate(
    sizes.begin(), sizes.end(), std::size_t{0});
```

The initial value matters twice. First, it sets the result's initial value and type. Supplying a plain `0` would commonly select an `int` accumulator, which can truncate conversions or overflow when counting a large number of bytes. Second, `accumulate` is an **ordered left fold**. For elements \(a_1,\ldots,a_n\) and initial \(z\), its grouping follows:

\[
(((z+a_1)+a_2)+\cdots)+a_n.
\]

The algorithm traverses the range from beginning to end and combines each next element with the current accumulator in that sequence. If the operation itself has side effects, they must still respect its specified use and not illegally invalidate the input range.

`std::reduce` has a different semantic allowance. It may combine elements in an unspecified grouping and order rather than necessarily following that left fold. This can enable more freedom for a reduction implementation, including implementations taking advantage of parallel-friendly combinations. But there is **no guarantee that a particular call without an execution policy actually launches parallel threads**. The important difference is what order and grouping the language permits.

Why does the distinction matter? Addition of *mathematical real numbers* is associative. Floating-point addition on a finite machine is not, because intermediate results are rounded. Take \(L=10^{20}\) and the sequence \(L,1,-L,3\). In the tested ordinary binary64 representation:

\[
((L+1)-L)+3 = 3
\]

because adding one to a quantity of magnitude \(10^{20}\) loses the one at the available precision. But another grouping gives:

\[
(L-L)+(1+3)=4.
\]

The difference is not merely performance; it changes the numerical result. A floating-point `reduce` is allowed to group operations in ways that yield a different rounding pattern from a specified sequential `accumulate`. The lab checks two explicit groupings to demonstrate the numerical issue, rather than asserting that `std::reduce` on this compiler *must* choose one particular grouping. For the five small `std::size_t` byte counts, both accumulate and reduce yield 304 under the tested implementation, and their integer addition is exact within the chosen range.

For a production engine, we must separately think about overflow and reproducibility. Unsigned integer addition can wrap if the total exceeds its range, producing an incorrect allocation budget even though it is defined modulo \(2^N\); signed overflow is an undefined-behavior risk in the wrong context. A checked sum can reject values before addition, or use a wider bounded accumulator. For floating-point statistics, we can choose a deterministic accumulation order, wider precision, pairwise summation, compensated summation such as Kahan's method when appropriate, or reproducible reduction schemes. These methods have their own numerical assumptions and performance costs; simply selecting `reduce` does not certify an accurate, deterministic total.

### Erasing “by algorithm” is not automatically erasing “from the container”

The report wants to hide completed work IDs or remove all texture rows. A classic trap is to call `std::remove_if` on a vector and assume the vector got shorter. It did not. This algorithm reorganizes the elements, moving those that do not satisfy the removal predicate into a retained prefix, and returns an iterator to the new **logical end** of that retained range. The vector's `size()` is unchanged until the container's erase operation actually destroys its obsolete suffix.

Consider:

```cpp
std::vector<int> jobs{1, 2, 3, 4, 5, 6};

auto logical_end = std::remove_if(
    jobs.begin(), jobs.end(),
    [](int id) { return id % 2 == 0; });

// [jobs.begin(), logical_end) now contains 1,3,5 in that order.
// jobs.size() is still 6.

jobs.erase(logical_end, jobs.end());
// jobs now owns exactly three live ints: 1,3,5.
```

The contents of the range past `logical_end` after `remove_if` are not values we should treat as valid retained work records, even though the vector still contains live objects there before `erase`. The retained elements preserve their relative order under `remove_if`'s semantic guarantee. The actual `erase` operation destroys the removed suffix and updates the vector's size, with invalidation effects on references and iterators at or after the point of erasure. This is the **erase–remove idiom**, not a mysterious two-step tradition without a reason. It separates a generic algorithm over iterators from a container-specific operation that ends element lifetimes.

The C++20 standard library also provides nonmember `std::erase_if(container,predicate)` for supported containers. The lab uses it with a vector of asset report records:

```cpp
auto report = assets;

const auto removed = std::erase_if(
    report,
    [](const AssetRecord& asset) {
        return asset.kind == AssetKind::Texture;
    });

// removed == 2, report.size() == 3.
// The live registry is unchanged: only the owned report copy was edited.
```

This operation combines the predicate-based removal with actual container erasure and reports the number removed. It is not necessarily implemented the same way for every container category. For an `unordered_map`, it erases matching key/value entries using the associative container's deletion operations, not by trying to assign a new key into a const key field. The lab also tests removal of a diagnostic map entry whose priority equals three. Attempting to use the generic `std::remove_if` algorithm directly on associative keys would violate the iterator's modification requirements.

There is an engine-specific danger here: **erasing a record from an owned query snapshot must not be mistaken for evicting the corresponding live asset**. The snapshot holds copied metadata and handles. Removing its texture rows changes only the report. Actual eviction must go back through the live registry, validate each handle's generation, remove indexes, coordinate pending consumers and—if there is GPU use—defer native resource destruction until it is safe. That is the reason for keeping the query/report operation and the eviction command as separate semantic steps. A convenient algorithm call cannot silently assume ownership of live resources it does not possess.

### Putting the entire pipeline together without losing the guarantees we already earned

We can now trace a complete report over our five assets. The editor captures an owned snapshot of `brick.mesh` (48 bytes, priority 2), `sky.tex` (64 bytes, priority 3), `ui.tex` (16 bytes, priority 3), `terrain.mesh` (128 bytes, priority 1) and `rock.mesh` (48 bytes, priority 2). A normal lookup for `rock.mesh` returns an optional handle `{4,1}`; an expected-style lookup distinguishes empty-name input from a missing name. A visitor handles one Inspect, one Upload and one Evict command using the active `variant` alternative, not guessed bytes or a downcast. The diagnostics channel holds a string in `std::any` and demonstrates that attempting to extract an `int` is an error, not a conversion.

A pair stores counts and errors; a structured binding made with `auto` copies those values, while an `auto&` decomposition refers to the original pair. A tuple carries three heterogeneous report fields. Compile-time iterator assertions establish that our vector is contiguous, deque is random-access but not contiguous, list is bidirectional, forward_list is forward-only, and a stream iterator is an input iterator. That is why we choose vector snapshots for the algorithms that need random access.

A normal sort by byte count plus name gives a deterministic sequence. A stable sort by priority retains the original order of equal-priority tasks. A partition separates meshes from other types without pretending either group is sorted. After sorting by byte count, lower bound 48 returns index one, upper bound returns index three, and equal range gives exactly the two 48-byte records. A linear find still works for a one-off unsorted name query without constructing a new index. Transform extracts the five byte counts, and accumulate/reduce give 304 for this exact small integer dataset. The separate floating-point example demonstrates why regrouping is not numerically irrelevant. Remove–erase reduces a six-element job vector to the three odd IDs, and `std::erase_if` removes two texture records from a report without modifying live registry state.

Every step has a *precondition*, an *observable effect* and an *invalidation or ownership boundary*. That is the habit a reliable engine developer needs. The result type communicates what failure means; the container supplies an iterator with particular capabilities; the algorithm requires a certain ordering; the numerical operation assumes certain algebraic properties; and the caller must know whether it is mutating an owned snapshot or a live resource.

### The executable witness, independent reconstruction and remaining evidence

The new reference is [[tools/labs/asset_query_algorithm_lab.cpp|asset_query_algorithm_lab.cpp]], 316 lines of commented C++20. It is a standalone CPU program. It does not depend on installed graphics drivers, the current Kairo modules, or a prior laboratory's compiled binary. Build from the repository root:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/asset_query_algorithm_lab.cpp -o /tmp/allmight_m016_gcc_san
/tmp/allmight_m016_gcc_san
```

The exact committed source was compiled and executed with GCC 14.2.0 and AddressSanitizer/UndefinedBehaviorSanitizer, Clang 17.0.0 with warnings as errors, and GCC 14.2.0 with `-O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror`. Each run printed `asset_query_algorithm_lab: all checks passed` and the sanitizer run reported no diagnostics. The local executed and GitHub committed source matched the same Git blob `0fa0a888b2aa0c348a52307a049bbf38527fcde1` and SHA-256 `c479bd493444d093f6c34898d4ecdf43938cdbd82de8986a7b1906a5bc374f9c`. These are witnesses for the modeled C++ semantics, not proofs of thread safety, a production asset query service or native GPU execution.

To **reconstruct from a blank file**, begin by explaining why `optional<AssetHandle>` is a useful presence test but cannot say whether the input name was invalid. Build a success-or-error type, then contrast it with a three-alternative command variant and an open-ended diagnostic `any`. Give an example where a structured binding copies its pair versus borrowing it. Prove the iterator-category requirements for vector, deque, list, forward_list and input streams. Sort five asset records by bytes with an explicit tie-breaker, then use stable sort when the original order of equal-priority items matters. Partition by asset kind, locate the partition boundary, and explain why this does not satisfy the byte-count sorting precondition for binary search. From the sorted byte sequence `[16,48,48,64,128]`, derive the exact lower and upper bound indices for 48. Transform the records into sizes and derive 304 with a correctly typed initial accumulator. Show, with \(10^{20},1,-10^{20},3\), why floating-point grouping can produce 3 or 4. Finally implement both erase–remove and `std::erase_if`, and explain why deleting *report rows* is not the same as destroying *live assets*.

The complete coverage reference is [[Supplementary/Foundations#M016 — Parts 251–270: the asset registry becomes a query and transformation pipeline|M016's original twenty-part audit map]], supported by [[Supplementary/Sources and Code Anchors#Phase 6 additive reconstruction — M016 / Parts 251–270|the verification and provenance ledger]] and [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#Query the registry without confusing identity and a snapshot — M016 reconstruction|the reconstruction board]]. This chapter is **authored reconstruction based on the canonical curriculum**, not a newly received M016 transcript. The current separate `KairoSpatial/BVH.cppm` and `KairoRayTracer/Scene.cppm` repositories and their specific revisions have not been inspected in this phase. No independent learner mastery test, real asset-registry concurrency proof, numerical-reproducibility benchmark, or GPU/graphics test is claimed. The next contract range, M017 / Parts 271–290, should become necessary when this query pipeline needs lazy ranges, allocators, PMR and stable modules/ABI boundaries.

## From copying every asset report to a lazy, allocatable, separately compiled query service

Our last editor query took a *snapshot* of the growing asset registry, then searched, sorted, partitioned, transformed, reduced and erased its metadata without pretending those operations changed the live registry. It was deliberately safe and direct. But every new report now repeats a familiar pattern: walk the owned snapshot, skip retired records, select the assets above a byte threshold, read their names or sizes, and hand those results to an editor panel. If we first build a vector of surviving records, then a vector of names, then another vector of byte counts, we may allocate and copy intermediate storage that we never actually needed. We also must answer a harder architectural question: where do all those small allocations come from, how long do they remain valid, and how can the query logic be compiled into a reusable engine library without forcing every plugin and consumer to rebuild every implementation detail?

We are going to follow one operation across three engineering boundaries. First, an editor wants a **lazy query over an owned asset snapshot**, where a view describes a computation rather than eagerly copying all intermediate results. Second, the editor wants to construct and discard per-frame report data using an **explicit allocation lifetime**, rather than hiding an unbounded number of individual heap allocations. Third, other engine components need to **import the query API across separately compiled translation units**, while the engine authors retain control over the implementation and understand the consequences of exported C++ types for binary compatibility.

Those boundaries are why M017, original Parts 271–290, exists. The source contract assigns it range-based iteration, lazy ranges and their ownership, projections, custom iterators, allocators, PMR and monotonic arenas, allocator-aware hash tables, callable representations, C++20 modules and partitions, header units, linkage, ODR, namespaces and ABI design. This is one connected systems problem, not twenty glossary entries. The CPU-side executable is [[tools/labs/asset_ranges_pmr_lab.cpp|the M017 ranges/PMR laboratory]]. The separate *actually compiled* module graph and build script are in [[tools/modules/build_module_lab.sh|the M017 module boundary laboratory]]. These have different test scopes, and the boundary between them is intentional.

### What the range-based for statement really does at the query boundary

Begin with six snapshot rows. Five represent live assets and one represents a retired texture. The live byte sizes remain 48, 64, 16, 128 and 48, totaling 304 bytes. We do not want the caller to remember to check `row.live` on every loop. Instead, the query layer offers a range whose iterator skips retired rows. Its public interface can be as simple as:

```cpp
struct LiveRows {
    const std::vector<Row>* owner; // borrowed owner: NEVER extends lifetime

    LiveRowIterator begin() const {
        return LiveRowIterator{*owner, 0};
    }

    LiveRowIterator end() const {
        return LiveRowIterator{*owner, owner->size()};
    }
};

LiveRows live{&rows};
std::size_t sum = 0;
for (const Row& row : live) {
    sum += row.bytes;
}
```

The **range-based for** statement is not a mysterious instruction that only works with a concrete container. Its conceptual translation introduces a hidden variable for the range expression, obtains a begin iterator and an end sentinel, compares them, dereferences the current iterator, executes the body, then increments. At this level, the loop is equivalent in intent to:

```cpp
auto&& hidden_range = live;
auto hidden_begin = hidden_range.begin();
auto hidden_end = hidden_range.end();

for (; hidden_begin != hidden_end; ++hidden_begin) {
    const Row& row = *hidden_begin;
    sum += row.bytes;
}
```

This is an explanatory model, not a claim that the compiler literally allocates these variable names or emits a fixed machine-code sequence. The language's exact range-for lookup and lifetime rules distinguish member `begin/end` discovery, argument-dependent lookup of suitable free functions, range expressions and different language versions. In particular, a temporary range object directly used in the range initializer can have its lifetime extended for the loop. That does **not** establish a general lifetime extension for every temporary object that a subexpression might have borrowed. C++23 strengthened some range-initializer temporary-lifetime rules, and even then a function returning a view to already destroyed storage remains a bug. Never infer that `for (auto x : get_bad_view())` can repair a dangling view returned by a callee.

Our `LiveRows` range itself only contains a pointer to the existing vector. It owns no row storage. The range-for statement safely runs while `rows` is alive and while no invalidating mutation occurs. If the vector reallocates, disappears, or is edited concurrently without appropriate synchronization, the iterator's accesses can become invalid. This is the exact same ownership boundary derived in the previous container chapter, now resurfacing in a more convenient syntax.

### The custom iterator has an observable promise, not merely two operators

The reader wants to walk only live assets. A hand-written iterator is appropriate when the traversal policy has useful semantics of its own, such as skipping vacant ECS slots or a sparse registry's tombstones. The concrete `LiveRowIterator` holds a borrowed pointer to the snapshot vector and a numeric position. Construction and increment advance the position past rows where `live == false`. Dereference yields a `const Row&`, so the loop does not copy asset names or byte counts merely to inspect them.

```cpp
class LiveRowIterator {
    const std::vector<Row>* rows_ = nullptr;
    std::size_t pos_ = 0;

    void skip_dead() noexcept {
        if (!rows_) return;
        while (pos_ < rows_->size() && !(*rows_)[pos_].live)
            ++pos_;
    }

public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = Row;
    using difference_type = std::ptrdiff_t;
    using reference = const Row&;

    LiveRowIterator() = default;
    LiveRowIterator(const std::vector<Row>& source, std::size_t first)
        : rows_(&source), pos_(first) { skip_dead(); }

    reference operator*() const noexcept { return (*rows_)[pos_]; }
    const Row* operator->() const noexcept {
        return std::addressof(**this);
    }
    LiveRowIterator& operator++() noexcept {
        ++pos_;
        skip_dead();
        return *this;
    }
    LiveRowIterator operator++(int) noexcept {
        auto old = *this;
        ++*this;
        return old;
    }
    friend bool operator==(const LiveRowIterator&,
                           const LiveRowIterator&) = default;
};
```

The member types describe the iterator's value and traversal capabilities; `operator*` returns a reference to the referenced value, `operator++` advances to the next live row, and equality compares the position and owner. The default-initialized iterator is not an invitation to dereference a null owner; it is provided to satisfy the relevant iterator concept's construction requirements and can participate in defined iterator operations under their preconditions. A normal end iterator is formed with the vector's size and must never be dereferenced.

**What makes this a forward iterator?** Its increment can be repeated independently on copies, and it supports the required multi-pass semantics while the underlying range remains unchanged. Unlike a one-pass stream iterator, a copied cursor at the same position remains a valid way to revisit that position during an unchanged traversal. It cannot move backward, perform random-access pointer arithmetic, or claim contiguity. The actual lab enforces that distinction at compilation:

```cpp
static_assert(std::forward_iterator<LiveRowIterator>);
static_assert(!std::bidirectional_iterator<LiveRowIterator>);
static_assert(!std::contiguous_iterator<LiveRowIterator>);
static_assert(std::ranges::forward_range<LiveRows>);
```

These checks establish that the declared expressions meet the compiler's structural concept requirements; the *semantic* multi-pass property and the validity of the borrowed owner remain an engineering proof obligation. The test checks that five rows are visited and their byte sum is 304, skipping the one retired record. A programmer could write operators that satisfy a concept syntactically while violating its semantic laws; a `static_assert` cannot prove every possible behavior. Likewise, the iterator does not make concurrent deletion safe. It simply packages a consistent traversal rule for the **owned, stable snapshot**.

There is a performance tradeoff: every increment may inspect one or several rows to find the next live entry. Over a full traversal of `N` slots, the total number of skipped slots is bounded by `N`, so the scan is linear in the snapshot's physical row count, not merely the number of live entries. A sparse snapshot with very few live rows might be better compacted once when many queries reuse it. A single query that needs only a few rows may benefit from not allocating a compacted copy. That is the kind of workload-dependent decision ranges make easier to express but do not resolve automatically.

### A view describes a computation; it does not necessarily compute the answer now

The editor wants the names of live assets with at least 48 source bytes. A conventional implementation might allocate a filtered vector, then allocate another vector of names. C++20 **views** let us compose a computation that applies each step as elements are requested:

```cpp
auto pipeline = std::views::all(rows)
    | std::views::filter([](const Row& row) {
          return row.live && row.bytes >= 48;
      })
    | std::views::transform([](const Row& row) {
          return row.name;
      });
```

The pipe character in this expression is an overloaded operator forming a view-adaptor composition. It is not a shell pipe, and it does not create a background task. `views::all` adapts a range into an appropriate view; `filter` exposes only elements satisfying a predicate; `transform` maps each exposed element through a callable. Unlike `std::transform` writing to a back inserter, this pipeline **does not immediately fill a destination vector**. Its normal evaluation is lazy: iterating the view asks the underlying range for the next acceptable element and computes its projected value at that step.

Our lab increments one counter inside the filter predicate and another inside the transformation callable. Immediately after constructing the pipeline, both counters are zero. After traversing it once into an owned destination vector, the selected names are `brick.mesh`, `sky.tex`, `terrain.mesh` and `rock.mesh`, the transform ran four times, and the predicate ran as needed to inspect the input. That is a concrete witness of deferred work, not a claim that every view iteration uses the same number of predicate calls across all implementations and all adaptor compositions.

A view has **laziness**, not automatically **memoization**. A repeated traversal may re-evaluate a transformation; some view adaptors may cache certain iterator positions according to their semantics and implementation. The `filter_view` beginning may be cached after its first search, and the precise number of predicate invocations is not a stable performance contract for arbitrary compositions. Predicates with externally visible side effects can make such cached or repeated evaluation surprising. Prefer predicates and projections that behave like ordinary, stable mathematical functions for the duration of the traversal. Our counters are deliberately diagnostic and used in one controlled pass, not production state on which a query's correctness relies.

What has been saved? We avoid constructing intermediate *collections* solely to represent each filtering/projection stage. That does not mean no allocations occur anywhere: the underlying snapshot owns strings, and our final `std::vector<std::string> names` must allocate to retain copies of the selected names. A filtered iterator may hold several pointers and callable objects, and it still performs condition checks while traversing. A captured predicate might itself be costly or allocate. The actual evaluation strategy must be measured before asserting a throughput improvement.

### Owning ranges, borrowed views, and dangling are three separate questions

The original asset snapshot is owned by `std::vector<Row>`. But if we call `std::views::all(rows)` with a **named lvalue** vector, the normal view uses a lightweight borrowed reference to that vector. The view does not make a deep copy of the snapshot. If `rows` dies, the borrowed view cannot keep accessing it safely. If `rows` reallocates, cached iterators into its old storage may be invalidated even though the view's *type* still exists. This is precisely why a lazy query pipeline cannot outlive the snapshot it refers to unless it takes ownership explicitly.

The contrasting call supplies a temporary vector:

```cpp
auto owned = std::views::all(make_snapshot());
```

On the tested C++20 GCC14/Clang17 standard libraries, this produces an owning adaptation that keeps the temporary vector within the view object. The lab confirms that its range still contains the six source rows. If `owned` dies, its stored vector dies. Copying or moving the view has the view type's own copy/move constraints; owning a vector is a different contract from borrowing one. `std::ranges::owning_view` is move-only in the relevant C++20 implementation. The editor should choose ownership deliberately; "view" does **not** mean "always non-owning."

There is also a subtle library concept called **borrowed_range**. It asks whether iterators obtained from a *range object* may remain valid after that range object itself is destroyed, according to the type's declared contract. A reference view can satisfy it because destroying the lightweight view does not destroy the underlying referenced container. But that says absolutely nothing about whether the referenced container remains alive. The tested assertions are:

```cpp
static_assert(std::ranges::borrowed_range<
    std::ranges::ref_view<std::vector<Row>>>);

static_assert(!std::ranges::borrowed_range<std::vector<Row>>);

static_assert(!std::ranges::borrowed_range<
    std::ranges::owning_view<std::vector<Row>>>);
```

The last result may initially look counterintuitive: if the owning view *owns* the vector, why is it not a borrowed range? Because destroying the view destroys the owned vector, so iterators into it would not survive destruction of the view object. These names answer different questions: **owning** says who destroys the data, while **borrowed_range** addresses whether iterators can survive destruction of the *range object* itself under the concept's contract. Neither concept grants a right to ignore mutation or thread safety. Ranges algorithms can use `std::ranges::dangling` in return types when an iterator would otherwise refer to a destroyed non-borrowed temporary; that is a compile-time defense against certain mistakes, not a substitute for careful lifetime design.

The problem also appears when we return a view from a function. If the function creates a local vector, then returns `std::views::all(local) | filter(...)`, the returned view typically refers to a dead vector. A safer design is to return the materialized owned result, return an explicitly owning range, or require the caller to pass an owner whose lifetime is documented. An engine API that returns an ephemeral borrowed range should say so prominently; it should not rely on users guessing the lifetime from template types in a long error message.

### Projections are not the same thing as constructing another list of names

Our last report built a vector of names and searched that vector. When we only want to locate `sky.tex` once, copying names into another container is unnecessary. C++20 ranges algorithms accept **projections**: a callable applied to each candidate element to obtain the value used by the algorithm's comparison. The operation becomes:

```cpp
auto found = std::ranges::find(
    rows,
    std::string_view{"sky.tex"},
    &Row::name
);
```

The member pointer `&Row::name` is used by the algorithm's invocation machinery as a projection. The search compares the projected name against the requested string view, rather than comparing entire `Row` objects. Our test confirms that the returned row has 64 bytes. That does not allocate a separate names vector simply to find one match. It also does not need a custom equality operator for `Row` whose meaning might be inappropriate for other tasks.

The same approach applies to sorting a *separate owned copy* of the snapshot by byte count:

```cpp
auto sorted = rows;
std::ranges::sort(sorted, std::less<>{}, &Row::bytes);
```

The comparator operates on the projection results. This sorting operation is eager and **mutates** the owned copy; it is not a lazy view. The resulting first record is the 16-byte row and the last is the 512-byte retired row. That last point is deliberate: sorting by bytes did not automatically filter out inactive entries. If our semantic requirement were "sort live rows by bytes," we would need to filter first and materialize a suitable mutable sequence or use another algorithm carefully matched to the view's iterator/value category and mutation capabilities. A filter view is not an arbitrary writable, random-access container just because its source is a vector.

A projection supplied to an algorithm also carries lifetime and exception implications. A member-pointer projection is cheap to describe, but a stateful lambda projection may capture borrowed objects and throw. In a performance-sensitive pipeline, the cost of projection calls, comparator invocations and cache behavior matters. Again, the correct starting point is semantic equivalence: does the algorithm compare exactly the key we intended, under a consistent strict weak ordering? Only then is it worth benchmarking alternate representations.

### We have stopped copying intermediate vectors; now who pays for the final allocations?

The final report still needs owned storage. The snapshot vector, the selected names, any hash index, and internal job queues all allocate memory. In a per-frame editor workflow, repeated allocation can create allocator contention, memory fragmentation, unpredictable tail latency and unnecessary work. But replacing ordinary allocation with a custom allocator introduces a new correctness obligation: a container must obtain and return storage under a compatible protocol, respecting type size, alignment, exceptions and ownership.

A **C++ allocator** is an object satisfying an allocator-aware container's requirements. In the common typed model, it provides a `value_type` and suitable `allocate(n)` and `deallocate(pointer,n)` operations. The standard library's `std::allocator_traits<A>` mediates much of the interface, including rebinding to internal node types where needed. Allocating raw storage and constructing the objects in it are different operations. The allocator controls *where storage comes from*; a vector's element lifetime and exception behavior still arise from the container and the element type.

The laboratory introduces `TrackingAllocator<T>`, storing a shared pointer to allocation statistics. Its relevant operations are:

```cpp
template<class T>
struct TrackingAllocator {
    using value_type = T;
    std::shared_ptr<AllocationStats> stats;

    template<class U>
    TrackingAllocator(const TrackingAllocator<U>& other) noexcept
        : stats(other.stats) {}

    T* allocate(std::size_t count) {
        // Check the allocation is representable before asking for memory.
        auto* p = std::allocator<T>{}.allocate(count);
        ++stats->allocations;
        stats->allocated_bytes += count * sizeof(T);
        return p;
    }

    void deallocate(T* p, std::size_t count) noexcept {
        ++stats->deallocations;
        std::allocator<T>{}.deallocate(p, count);
    }

    // The complete lab contains size checks, constructors, equality and
    // access to shared statistics across its allocator rebindings.
};
```

We use the tracking allocator in `std::vector<std::size_t, TrackingAllocator<std::size_t>>` and insert the same five live byte counts. The resulting sum is still 304. After the vector leaves scope, the number of recorded successful storage acquisitions equals the number of releases. That is a **bookkeeping witness**, not a proof that the allocator never fragments memory or that every C++ type can be used safely with this illustrative implementation.

Allocator **equality** and **propagation** matter. If two allocator objects compare equal, the relevant allocator model says their allocations/deallocations are compatible in the required sense. Moving or swapping containers with unequal, non-propagating allocators can impose different behavior or preconditions than simply moving a pointer. The standard defines allocator traits such as `propagate_on_container_move_assignment`, `propagate_on_container_swap` and `is_always_equal`, which affect these operations. Our shared-statistics equality is appropriate for the isolated test because the actual upstream storage mechanism is the same `std::allocator<T>`; it is not a general recipe for every stateful pool allocator. An engine allocator that holds a pointer into a short-lived frame arena must not compare compatible with an unrelated arena and then allow deallocation through the wrong lifetime.

Another subtlety: `allocate(n)` returns **raw, appropriately aligned storage** for `n` objects of `T`, not `n` already constructed objects. The allocator-aware container performs construction/destruction under its contracts. An owning allocator cannot use a scope-exiting stack buffer to satisfy storage whose elements survive that stack frame. The memory resource and its users must have a properly nested lifetime.

### The problem with a compile-time allocator type is that allocation strategy may change per frame

A typed allocator parameter is part of a container's **type**:

```cpp
std::vector<std::size_t, TrackingAllocator<std::size_t>> tracked;
```

A vector using `TrackingAllocator<T>` and one using another allocator type are different C++ types. That may be fine inside a subsystem, but if the editor wishes to choose a resource provider at runtime without proliferating template specializations in every API, C++'s **polymorphic memory resources** provide a useful boundary.

A `std::pmr::memory_resource` is a runtime allocation interface exposing allocation, deallocation and resource-equivalence behavior through virtual customization points. `std::pmr::polymorphic_allocator<T>` refers to a memory resource; its type does not change for every concrete resource implementation. Standard PMR aliases such as `std::pmr::vector<T>` use the polymorphic allocator so the same container type can refer to a new/delete resource, a pool resource, a monotonic arena or another implementation chosen at runtime.

The lab builds a concrete `CountingMemoryResource` which delegates to `std::pmr::new_delete_resource()` and records successful block allocations and deallocations:

```cpp
class CountingMemoryResource final : public std::pmr::memory_resource {
    std::pmr::memory_resource* upstream_;
public:
    std::size_t allocated_blocks = 0;
    std::size_t freed_blocks = 0;

    explicit CountingMemoryResource(std::pmr::memory_resource* upstream)
        : upstream_(upstream) {}

private:
    void* do_allocate(std::size_t bytes,
                      std::size_t alignment) override {
        void* result = upstream_->allocate(bytes, alignment);
        ++allocated_blocks;
        return result;
    }

    void do_deallocate(void* p, std::size_t bytes,
                       std::size_t alignment) override {
        ++freed_blocks;
        upstream_->deallocate(p, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other)
                    const noexcept override {
        return this == &other;
    }
};
```

The resource's `allocate` receives both a byte count and an alignment request, unlike a naive byte-array allocator that might silently return misaligned storage. Our concrete implementation delegates alignment to the upstream resource. The custom equality rule uses resource identity rather than making two unrelated resource objects appear safely interchangeable.

The complete code then creates `std::pmr::vector<std::size_t>` and `std::pmr::unordered_map<std::pmr::string,std::size_t>` using the same resource. It inserts five live names and sizes, confirms the index contains five entries, looks up `sky.tex` and reads 64 bytes, and observes that the PMR resource intercepted allocations. After both containers leave scope, their ordinary deallocation calls have balanced the resource's successful allocation count.

This hash-table example is important because an unordered container normally allocates more than its user-visible value objects: it needs bucket storage and node or equivalent internal storage under the implementation's policy. An **allocator-aware unordered container** must use its allocator for the relevant internal storage via allocator rebinding and traits. A PMR hash table can therefore route those allocations through a selected memory resource, but the exact number, block sizes and node representation are *implementation-dependent*. The allocator does not change the semantic obligations of a hash table: equivalent keys must hash equally; rehash can invalidate iterators; and collisions still affect lookup performance. A memory resource is not a substitute for a good key representation or a correctness-preserving hash/equality relation.

The `std::pmr::string` keys also expose a nested-allocator issue. Allocating the hash nodes through a PMR resource does not prove that an arbitrary user-defined object stored in those nodes sends all *its own nested allocations* to that same resource. Our code deliberately constructs keys associated with the intended resource. The standard's `uses_allocator` construction machinery can propagate allocators to appropriately designed allocator-aware types, but not every third-party class participates. A common engine mistake is to assume that making the *outer* container PMR automatically gives every owned pointee, texture byte array and callback capture the same arena lifetime. Those objects require their own allocator-aware design.

### An arena is excellent for one frame—but only if its destruction boundary is real

The editor's per-frame query results can have a natural group lifetime: create records during one update, consume them, then discard everything together. A `std::pmr::monotonic_buffer_resource` is intended for that pattern. It can start with an already-provided buffer and allocate successive regions from it. When that buffer runs out, it can request larger blocks from an upstream memory resource. Individual `deallocate` calls do **not** return each block for reuse in the ordinary way; memory is reclaimed when the resource is released or destroyed.

The lab provides a small aligned stack scratch buffer and a counting upstream resource:

```cpp
CountingMemoryResource upstream{std::pmr::new_delete_resource()};
alignas(std::max_align_t) std::array<std::byte, 128> scratch{};

{
    std::pmr::monotonic_buffer_resource arena{
        scratch.data(), scratch.size(), &upstream
    };

    {
        std::pmr::vector<std::uint64_t> values{&arena};
        values.resize(2048, 7); // bigger than the initial 128-byte buffer
    }

    // The vector has been destroyed, but the monotonic resource has
    // not returned its individual upstream blocks yet.
}

// Now arena's destruction releases the acquired upstream blocks.
```

Because 2,048 64-bit elements cannot fit in the 128-byte scratch buffer, the arena obtains additional storage upstream. The test witnesses at least one upstream allocation while the vector is alive. After the vector is destroyed but before the arena is destroyed, the upstream resource has not received individual release calls for those acquired blocks. After the arena itself leaves scope, upstream allocation and deallocation counts balance. This is a *directly measured lifetime distinction*.

Monotonic allocation is often quick because the common path advances a current position in an existing block, subject to alignment and capacity. That does not mean the whole workflow is allocation-free, or that a known fixed number of bytes is sufficient for every input. Monotonic resources can request more upstream blocks, sometimes with geometric growth, and memory usage can remain high until the arena's entire lifetime ends. If one request allocates a very large temporary and the arena persists for many frames, retaining all its blocks can be undesirable. If you release the arena **before destroying objects that still own PMR pointers into it**, their later destruction or use becomes invalid. The safe order is: destroy or complete all users first, then release the arena, or choose a different lifetime policy.

A second test uses a 64-byte scratch buffer and `std::pmr::null_memory_resource()` as upstream. When a request is too large to fit, the resource cannot allocate another block, so the operation throws `std::bad_alloc`. That demonstrates a **bounded arena with explicit failure** rather than secretly falling back to unrestricted heap allocation. A production frame allocator can choose to reject work, divide it into batches, spill to a secondary resource or increase the budget. The chosen behavior belongs in the engine's API contract.

Crucially, `monotonic_buffer_resource` is **not thread-safe**. If several threads share one mutable arena concurrently, they need an explicitly synchronized resource or a design with separate thread-local arenas and well-defined ownership transfer. Even a synchronized pool resource does not automatically make the container operations on the allocated objects thread-safe. Allocator safety and logical data-race safety are independent.

If data must outlive a single frame or be returned to a renderer that might consume it asynchronously, a frame arena is appropriate only if its lifetime covers that asynchronous completion interval. Once the renderer has submitted GPU work, CPU query metadata may be releasable immediately if the necessary data have been copied, but staging bytes still referenced by pending transfers are not. The correct retirement boundary depends on actual API ownership and synchronization contracts, which this CPU-only laboratory deliberately does not simulate.

### A function object or lambda is not necessarily slow; its *representation* must be measured

The lazy filter requires a callable predicate. We already know from the callback chapter that a lambda creates a unique closure type and that a function object defines `operator()`. Which should the hot query use?

```cpp
struct AtLeastBytes {
    std::size_t threshold;

    bool operator()(const Row& row) const noexcept {
        return row.bytes >= threshold;
    }
};

AtLeastBytes functor{48};

auto lambda = [threshold = std::size_t{48}]
              (const Row& row) noexcept {
    return row.bytes >= threshold;
};

auto a = std::ranges::count_if(rows, functor);
auto b = std::ranges::count_if(rows, lambda);
// The lab checks a == b for its snapshot.
```

The concrete functor and the lambda closure both carry a numeric threshold by value. Both have statically known concrete types at the `count_if` call site, giving the compiler similar opportunities for inlining and constant propagation. We should not claim that lambdas are slower because they are anonymous, or that a named function object is automatically faster because it looks more explicit. A capturing lambda can be larger or smaller than another functor depending on captured state and layout; a `std::function` wrapper adds a different runtime-erasure contract and may allocate or dispatch indirectly depending on its target.

The correct experiment controls the predicate's observable behavior, the dataset, compiler, optimization level, capture size and invocation context. Measure both the time and resulting machine code, including potential instruction-cache growth if templates specialize for many closure types. Use a benchmark that prevents the compiler from removing all work as dead code, and report distributions rather than a single convenient number. The current lab verifies *behavioral equivalence* only; it does **not** benchmark execution time or compiler output. That distinction respects the original research bridge: **abstraction cost must be proved with evidence, not asserted by folklore**.

### The first compilation boundary: headers duplicate text, modules provide an import graph

Our query interface is now useful enough that the editor, renderer and an external tool all need it. A familiar header can contain declarations, inline functions, type definitions, macros and templates. Each translation unit that textually includes a header receives preprocessor-expanded source. Include guards or `#pragma once` prevent multiple inclusion in a single translation unit; they do not prevent the same declarations and template definitions from being processed by many separate translation units. A header can also bring macros and unexpected dependencies into callers. Well-designed headers remain valuable, but large, tightly coupled header graphs can make build dependencies and recompilation costs difficult to manage.

C++20 **named modules** offer another approach. An importing translation unit consumes an interface produced by a module build step, rather than textually inserting that interface as ordinary source. A module may contain non-exported implementation details that do not become public simply because they share a source file. Build tools must understand the graph of imports, and compilers need a compatible way to produce and locate compiled module interface artifacts. A module is not a dynamic-link library, a namespace, a C ABI, or automatic run-time plugin loading. It is fundamentally a C++ language and compilation organization mechanism.

The tested module's **primary interface unit** is [[tools/modules/allmight_asset_query.cppm|allmight_asset_query.cppm]]. Its essential shape is:

```cpp
module;
#include <cstddef>
#include <cstdint>

export module allmight.asset_query;
export import :rules;

export namespace allmight::asset_query {
    struct AssetHandle {
        std::uint32_t slot;
        std::uint32_t generation;
    };

    struct PreviewPlan {
        std::uint64_t byte_count;
        bool accepted;
    };

    PreviewPlan make_preview(std::uint64_t byte_count);

    inline constexpr std::uint32_t kApiEpoch = 1;

    inline constexpr bool has_generation(AssetHandle h) noexcept {
        return h.generation != 0;
    }
}
```

The leading `module;` begins a **global module fragment**, in which we include ordinary library headers before the named module declaration. This is a practical interoperability technique for current standard libraries and toolchains that do not provide universal `import std;` support. The `export module allmight.asset_query;` declaration introduces a primary module interface. `export namespace` makes the declarations within that region available to an importer according to the applicable module visibility and export rules. It does not convert their types to a C ABI. The `make_preview` declaration is exported, while its function body is intentionally elsewhere.

Why is the interface small? A plugin should need the *meaning* of a preview plan, not the source for every parser, allocator or platform-specific loader. But a small exported C++ structure is not automatically binary-stable across compiler versions or build options. We will return to that after verifying how implementation units and partitions actually compile.

### Module partitions separate interface decisions without splitting their identity

The primary module needs one rule: a preview with zero bytes is invalid, and the teaching boundary permits at most 1,024 bytes. That rule belongs to the same conceptual module but may be kept in a separate **module partition**. Its source is [[tools/modules/allmight_asset_query_rules.cppm|allmight_asset_query_rules.cppm]]:

```cpp
module;
#include <cstddef>

export module allmight.asset_query:rules;

export namespace allmight::asset_query {
    inline constexpr std::size_t kMaxPreviewBytes = 1024;

    inline constexpr bool within_budget(std::size_t bytes) noexcept {
        return bytes > 0 && bytes <= kMaxPreviewBytes;
    }
}
```

The colon names a partition of the **same named module**. The primary interface uses `export import :rules;` to re-export the partition's exported declarations. Consumers importing `allmight.asset_query` therefore have access to `within_budget` without importing the partition name directly. This is not a promise that every internal partition should be publicly exposed; we re-export it here deliberately to test the mechanism. An internal module partition could be imported only within the module without exporting its declarations to external consumers.

An import graph is **directed and acyclic** for the relevant module interface dependencies. In the tested GCC build, the rules partition is compiled first, producing its compiled module information. The primary interface, which imports that partition, comes second. Importers cannot correctly compile the interface before the required partition information is available in this deliberately ordered build. A modern build system should calculate this dependency graph rather than relying on a random order of source files or the order returned by a shell glob.

A source suffix such as `.cppm` is a useful naming convention but not a universal compiler promise. Our GCC14 commands explicitly pass `-x c++` when compiling those files. The build script also supplies `-fmodules-ts`, because GCC's C++20 module support is not enabled solely by `-std=c++20`. Another compiler or build system may require different flags, module scanner behavior, cache layout and compiler artifacts. The *language-level module model* is more portable than a handwritten one-size-fits-all module build command.

### The implementation unit defines the operation but does not export its body

The primary interface declares `PreviewPlan make_preview(std::uint64_t)`. The implementation unit is [[tools/modules/allmight_asset_query_impl.cpp|allmight_asset_query_impl.cpp]]:

```cpp
module;
#include <cstdint>

module allmight.asset_query;

namespace allmight::asset_query {
    PreviewPlan make_preview(std::uint64_t bytes) {
        return PreviewPlan{bytes, within_budget(bytes)};
    }
}
```

This is a **module implementation unit** because its module declaration lacks `export`. It belongs to the same named module as the interface, so it can define the exported function in the right module context. Its function body is not exposed to a consumer merely because the consumer imports the interface. The code calls the partition rule and constructs a small value. Its behavior is intentionally deterministic: `make_preview(64)` is accepted, while `make_preview(2048)` is rejected.

Now trace the compilation chain: first the rules partition; then the primary interface; then the implementation; then two independent importer translation units. The importer does not require a textual inclusion of the implementation source. It sees the declarations made reachable by the module interface and links against the compiled function definition. That is the actual separation modules provide. They are not a guarantee that implementations can change without recompiling consumers in all circumstances, especially when exported type layouts or inline definitions change.

The first consumer imports the module and calls `make_preview` and `has_generation`. A **second translation unit** independently imports the same module and invokes the exported inline function. The linked executable calls into both. This matters because a single test source can accidentally hide One Definition Rule or exported-visibility problems that only emerge when several translation units use the same module. We actually link the implementation and both importers in the tested build.

### Inline, linkage, and the One Definition Rule survive the transition to modules

We previously used `inline constexpr` helper functions and variables in headers and template-based classes. The **One Definition Rule (ODR)** constrains which entities may have multiple definitions and under what conditions. For ordinary inline functions in headers, definitions can appear in multiple translation units if the required conditions are met and the definitions are appropriately identical. Templates also impose ODR-related conditions, and many violations are not required to produce a diagnostic. Those facts are not magically erased by modules.

Modules introduce **module linkage** for certain namespace-scope names attached to a named module, and distinguish declarations attached to the named module from declarations in the global module fragment. The exact visibility and linkage relationships depend on whether an entity is exported, whether it has internal linkage or external linkage, and where its declaration appears. A non-exported name is not simply "global" because it is in a C++ namespace, and an exported name is not automatically a C link symbol. Keep the questions separate: who can *name* this entity from source; which translation units are allowed to *refer to the same entity*; and which compiled symbols or layouts form the actual binary interface?

In the tested module interface, `inline constexpr std::uint32_t kApiEpoch = 1` and `inline constexpr bool has_generation(AssetHandle)` appear in the exported region. Two importer translation units compile and link successfully while using these declarations. This is a direct check of the particular module export/inline arrangement, not a proof that every combination of macros, partitions, compiler flags and imported header units is ODR-safe. Defining a member function inside a class body *inside a named module* does not always carry the same implicit-inline semantics people expect from traditional header definitions; explicitly stating the intended `inline` contract can be necessary. GCC exposes compatibility options that change some of this behavior and can affect ABI, which must be used consistently if chosen. We avoid such flags in the test.

The lesson for a growing engine is that **public inline code and templates expose more implementation to clients**. Changing an exported inline algorithm can require client recompilation, and an incompatible mixture of old and new compiled module interfaces is not a supported binary-upgrade strategy. Modules help organize and potentially reduce textual compilation, but exported definitions remain a source-level contract. The CMI is typically tied to the compiler, flags and build environment; it is not a portable standardized binary package that arbitrary compilers can exchange.

### Header units are a bridge, not an unlimited replacement for every include

The engine already has legacy headers and C interfaces. C++20 **header units** let some headers be compiled as importable translation units, allowing source to write an import declaration for the header rather than always expanding its text into each importing source. But support and build procedures vary, and not every header has the right macro/interface behavior to migrate unchanged.

The demonstrated legacy header is deliberately small:

```cpp
#pragma once
inline constexpr unsigned kLegacyAssetFormatVersion = 7;
```

A separately compiled header-unit consumer can write:

```cpp
import "asset_query_interop.hpp";
import allmight.asset_query;

int main() {
    if (kLegacyAssetFormatVersion != 7)
        return 1;
    if (!allmight::asset_query::make_preview(64).accepted)
        return 1;
}
```

Our GCC14 build explicitly produces the header-unit CMI using `-x c++-user-header` *before* building this importer. It does not presume that `import "file.hpp";` will cause the compiler to transparently build the header unit on demand. C++ standard-library header units also have implementation-specific support and can need explicit compilation. Treat macro-heavy third-party headers carefully: macro visibility and include/import interactions can differ from the expected textual preprocessor behavior. A header unit is a **transitional compilation mechanism**, not a promise that macros become regular exported C++ module declarations.

We also demonstrate ordinary textual inclusion of a **C-compatible header** for a stable boundary. Both techniques can coexist. The decision should follow the consumer's compiler, build system, dependency graph and compatibility requirements rather than any blanket rule that modules eliminate all headers.

### The engine namespace organizes source names; it does not by itself version binaries

All the query interface's exported C++ declarations live in `namespace allmight::asset_query`. This organizes names and avoids collisions with similarly named functions in math, rendering or physics. It lets a caller distinguish `allmight::asset_query::make_preview` from some other library's `make_preview`. A nested namespace can also help an API document subsystem ownership, and inline namespaces can participate in versioned source-name lookup when designed carefully.

But **a namespace is not an ownership mechanism, a dynamic-loader boundary, or a frozen ABI**. It does not control where a resource was allocated, how long it stays alive, how the compiler lays out an exported struct, or whether two C++ compilers agree on mangled symbol names. Changing the content or layout of an exported C++ class can force clients to rebuild even if the namespace stays unchanged. Modules and namespaces solve different problems: modules control compilation reachability and attachment, while namespaces control source-level name organization.

Our exported API is intentionally small and uses familiar fixed-width integer fields, yet the C++ `PreviewPlan` structure is still not declared to be a portable wire format or stable binary interface. Its padding, calling convention and return-value ABI can depend on the target ABI and compiler settings. The test compiles and links its consumers with one GCC toolchain; that validates that configuration, not arbitrary cross-toolchain linking.

### A C-linkage wrapper makes a narrower promise than a C++ template API

The editor's C++ tools can import `allmight.asset_query`. Suppose a small C11 utility needs only to ask whether a payload size fits the preview rule. C cannot import a C++ named module or use a C++ template interface directly. We can expose a simple, versioned C-compatible function:

```c
/* asset_query_c_api.h: included by both C11 and C++20 sources */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int allmight_asset_approved_v1(uint64_t byte_count);

#ifdef __cplusplus
}
#endif
```

The C++ wrapper is compiled separately, imports the C++ module and implements that symbol:

```cpp
#include "asset_query_c_api.h"
import allmight.asset_query;

extern "C" int allmight_asset_approved_v1(std::uint64_t bytes) {
    return allmight::asset_query::make_preview(bytes).accepted ? 1 : 0;
}
```

The separate C11 program includes only the header and calls `allmight_asset_approved_v1(64)` and `allmight_asset_approved_v1(2048)`. The first returns one; the second returns zero. We compile that C program with `gcc -std=c11`, compile the wrapper and module with `g++ -std=c++20 -fmodules-ts`, then link them through the C++ linker so the C++ runtime dependencies are satisfied. This linked executable was run successfully. There is no C++ module syntax inside the C translation unit.

Why bother with a C wrapper? `extern "C"` requests the specified C language linkage for the function so the symbol can be used through a conventional C toolchain on the selected platform, avoiding ordinary C++ overload-based name mangling for that function. The version suffix `_v1` gives us a place to introduce a separately named incompatible future interface instead of silently changing a caller's expectation.

**That is not a universal ABI guarantee.** C language linkage does not standardize the binary layout of every C struct across unrelated platforms, define cross-language exception unwinding, make a callback's ownership policy safe, or protect us from compiler/architecture calling-convention differences. A real plugin API should define symbol visibility, library loading, platform target, field widths, padding/versioning of externally visible structures, allocator ownership across the boundary, thread safety, error reporting and how handles are validated. It must **not let C++ exceptions escape into a C caller** unless a specific interoperable exception contract really exists; a production wrapper normally catches and translates failures at the boundary.

In this minimal experiment the C function returns a small integer based on an internal deterministic rule, so it creates no resource handle and needs no allocation ownership transfer. That is why it can be demonstrated cleanly. If we exposed a pointer to an engine-owned asset, we would need corresponding retain/release or opaque-handle operations and a clear versioned lifetime protocol. Those contracts belong at the same boundary, not in a future vague promise that "the plugin must be careful."

### Why the exported C++ template API and the external ABI deserve different treatment

The earlier template chapters showed how a concrete `AssetDepot<T,Capacity>` produces statically checked, type-specific operations. That is appropriate for internal, source-level reuse, where users build against matching compiler settings and a known library revision. It can also create multiple instantiations, long diagnostic messages, additional compile dependencies and visible implementation details. The exact machine code may be optimized or merged, but templates are not inherently ABI-neutral.

A stable external plugin boundary wants a **small, opaque, versioned surface**, so that an internal vector growth strategy, custom allocator, or template specialization does not become a structural promise to an independently distributed client. The C wrapper is one such boundary for this small question. Other production designs can use a carefully versioned C++ interface, a PIMPL implementation object, a table of function pointers, an opaque handle registry, or message-based interfaces. Each choice involves explicit binary/toolchain compatibility and ownership rules; none is automatically stable merely because the API uses C++20 modules.

The practical split is not "templates are bad" versus "C APIs are fast." It is **where variation is supposed to live**. Within a compilation unit or a mutually rebuilt engine, concrete templates and ranges can express efficient, type-safe work. Across separately versioned binaries, a versioned C-compatible adapter or deliberately managed C++ ABI surface can limit what callers need to know. The C boundary itself has call overhead and validation costs, and every crossed call may create a new opportunity for errors or mismatched ownership. A useful system chooses coarse, purposeful boundaries so that hot per-element work stays inside the engine while a plugin makes occasional high-level requests.

That architectural separation completes the original M017 ABI-stability question without pretending all later advanced topics are prerequisites we can defer. We have already made a tested concrete boundary. Further refinements such as opaque-state implementation patterns or a reusable math/container API can strengthen it, but the reader can now implement and explain the essential design end to end.

### Reproduce the module graph, not merely a code snippet that looks like a module

The module test is compiled by [[tools/modules/build_module_lab.sh|build_module_lab.sh]], an executable Bash script using GCC14 C++20 module support. It creates a clean temporary working directory, so stale CMIs from previous builds cannot make a broken import order appear to work. Its build order is:

```text
allmight_asset_query_rules.cppm   exported :rules partition
                 |
                 v
allmight_asset_query.cppm         primary exported module interface
                 |
     +-----------+-----------+------------------+
     v                       v                  v
allmight_asset_query_impl.cpp  asset_query_module_lab.cpp  asset_query_secondary.cpp
     |                       |                  |
     +-----------------------+------------------+
                         |
               linked C++ executable

asset_query_c_api.cpp + asset_query_c_consumer.c
   |                       |
   +---- linked C/C++ boundary executable

asset_query_interop.hpp (compiled separately as a header unit)
   |
asset_query_header_unit_lab.cpp
   |
linked header-unit importer executable
```

The actual script builds the partition, primary interface, implementation, wrapper and consumers with `g++ -std=c++20 -fmodules-ts` and warnings treated as errors. It builds the C11 consumer with `gcc -std=c11`. The header-unit build uses `-x c++-user-header` and the configured source include path, then builds its importer. All three produced programs execute checks and print explicit success messages; the script prints its own final success line.

On the local GCC 14.2.0 environment, all three module-path executables passed. The source-level ranges/PMR lab separately passed GCC 14.2 with AddressSanitizer/UndefinedBehaviorSanitizer, Clang 17 with warnings as errors, and a GCC release-style `-O2 -DNDEBUG` build. The module script is GCC-specific; we do not claim a Clang or MSVC module build passed simply because the C++ syntax is standard. GCC's documented C++20 modules support still has implementation limitations, and this test deliberately sticks to features its local toolchain can compile. The repository's CI workflow includes a separate GCC module build job, not an attempt to compile all module-partition files independently as if they were standalone `*_lab.cpp` programs.

The two tests are independent witnesses for different contracts. The runtime lab proves that the chosen lazy query returns the expected live records, that an owning view stores its snapshot, that a custom forward iterator satisfies the compiler's concepts, that PMR hash/vector operations intercept allocations, and that monotonic release timing differs from ordinary deallocation. The module test proves the chosen GCC toolchain can build and link exported partitions, interface and implementation units, two importers, a header unit and a C-facing wrapper. Neither proves thread safety, a real GPU frame allocator, cross-platform binary compatibility or zero abstraction overhead. Those would require new measurements and platform-specific tests, not stronger adjectives in our notes.

### One end-to-end reconstruction from a blank directory

Start with a five-live/one-retired owned snapshot and write `LiveRows` so that a normal range-for skips the retired element. Before testing, explain the hidden begin/end iteration model, the cursor's equality and increment invariants, and why it is a forward iterator but not a contiguous iterator. Use C++20 filter/transform views to select four names, then verify that adaptor construction performs no filtering. Explain why repeating the query may repeat work. Predict the difference between `views::all(lvalue_vector)` and `views::all(rvalue_vector)`, and state why `borrowed_range` does not guarantee ownership. Replace a separate name-vector allocation with a ranges algorithm using `&Row::name` as its projection.

Now count the allocations of one vector with a typed tracking allocator and of a PMR vector plus a PMR unordered map using a runtime memory resource. Explain how allocator traits, rebinding, nested allocator-aware objects and the resource's lifetime determine correctness. Allocate an oversized vector through a monotonic arena and prove that its upstream blocks are reclaimed only at the arena boundary, not individually when a vector leaves scope. Then use a fixed scratch buffer and a null upstream resource to force a clear `std::bad_alloc`. Compare a named predicate functor with a captured lambda and state what you would measure before claiming one runs faster.

Finally, in a fresh build directory, compile a named module partition, the primary interface which re-exports it, the non-exported implementation, and two independent importers. Explain the difference between **export visibility**, namespace organization and linkage. Compile a header as a header unit before importing it. Link a C11 consumer through a versioned C-linkage wrapper and explain why the wrapper is narrower than exporting a C++ class's layout. Describe when a client must recompile after changes to exported inline code, template definitions or public struct layout. If you can reproduce the three linked module executables and the allocation/lifetime assertions from the source alone, the twenty parts have become one functioning engineering model rather than twenty labels.

The original source map [[Supplementary/Foundations#M017 — Parts 271–290: lazy asset queries, allocation lifetimes and compiled API boundaries|M017 Parts 271–290]] records every title. [[Supplementary/Sources and Code Anchors#Phase 7 additive reconstruction — M017 Parts 271–290|The evidence ledger]] distinguishes native compiled results from language references and open audits. [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#From borrowed reports to module boundaries — M017 reconstruction|The recall companion]] requires independent derivation and reimplementation. **This is an authored reconstruction of the canonical curriculum**, not a newly received M017 transcript, a claim of tested native graphics modules in external Kairo sources, or certification of learner mastery. External targets `KairoGameEngine/CMakeLists.txt` and `KairoMath/KairoMath.cppm` are still project-specific audit targets; our tested module graph is a self-contained reference, not an invented report about those files.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/05 - Generic Asset Pipelines without Hidden Contracts|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|Next chapter]]
