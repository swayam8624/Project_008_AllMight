### NES-06G — A handle must identify the asset, not yesterday's address

The chapter has already developed vector growth, geometric amortized analysis, iterator invalidation, a name-indexed asset registry, free slots, lookup, container trade-offs and the distinction between a borrowed span and an owned array. Its existing M015 laboratory even demonstrates a generation-bearing registry for named byte assets. The new work therefore **does not pretend that generational handles were absent**. Instead, the previous five passes have left us with a specific engineering question: can we make the *actual typed cartridge, texture and mesh import pipeline* survive removal, reuse, movement of its physical storage, movement of its owning registry, and maliciously or accidentally misapplied handles? And can we do this without weakening the source validation, lifetime, and failure guarantees already built?

The current NES-05A `AssetStore` is intentionally append-only. Its `AssetId<Tag>` is just a slot number: it remains meaningful while that particular store keeps appending, but it contains no version and no owner namespace. If we began reusing freed slots in that implementation, an old identifier would silently point at a different asset. If we combined IDs from two independent stores, slot 7 would be indistinguishable from another store's slot 7. A templated tag helps reject accidental assignment between cartridge and texture ID types, but it does not stop somebody from constructing a handle with manually supplied numbers. And a valid slot number still says nothing about the lifetime of a native pointer borrowed from a `std::vector` element.

Our **NES-06G** milestone makes the missing guarantees explicit. It does not reimplement iNES, NROM, TXR4 or MSH1 decoding. Those remain in the existing `Spec<Tag>` specializations and the original `NromCartridge`. We introduce a `GenerationalAssetRegistry` in `tools/labs/nes_course/generational_asset_registry.hpp` and exercise it in `tools/labs/generational_asset_registry_cpp23_lab.cpp`. Both are source-level additions to our cumulative C++23 book project, not separate emulator prototypes or replacements for the chapter's existing C++20 laboratories.

#### One coordinate is not an identity

Think about a library shelf. “Position 12” is useful for finding the book currently stored there, but that position does not preserve the identity of a book after it is removed and replaced. An index into a `vector` is exactly such a coordinate. The address of the storage holding the indexed slot can change when the vector reallocates; the slot number itself can continue to designate the same position after reallocation. And even when the slot number is unchanged, the occupant can change after deletion.

We therefore define a typed logical handle as

$$
H=(O,I,G,T),
$$

where $O$ is an **owner namespace** identifying a particular registry instance's logical lifetime, $I$ is a slot index, $G$ is a **generation** identifying which occupant of that slot was intended, and $T$ is the asset kind. In the implementation $T$ is largely encoded in the C++ template argument and checked against the currently active `std::variant` alternative. The runtime fields are the owner token, slot and generation.

A handle is accepted only if all of the following are true: the current registry has owner token $O$, slot $I$ is in bounds, the slot contains a live object, it is not permanently retired, its stored generation equals $G$, and the stored variant alternative matches $T$. The final alternative check is vital. Although `AssetHandle<TextureTag>` is a different C++ type from `AssetHandle<MeshTag>`, a programmer can still forge an instance of the former with the numerical fields copied from a mesh handle. C++ templates prevent one category of accidental conversion; a runtime validation prevents the forged or corrupted input from masquerading as the wrong concrete asset.

The implementation returns `std::expected<std::reference_wrapper<const Asset<Tag>>, LookupError>` for successful lookups. That is **a borrowed reference**, not a new owner and not a long-lived pin. It can be used while the registry is not being mutated in ways that invalidate the referent. A caller must not store it through a later `import` that can reallocate the slot vector, an `erase` of its occupant, a registry move, or destruction of the registry. The handle itself survives a valid relocation; the old C++ reference does not. This difference is the main reason for using handles in a growing engine.

#### Why we need an owner namespace in addition to a generation

Suppose two registries independently begin with an empty slot vector. Each imports one texture; each produces slot zero at generation one. The pair $(0,1)$ cannot identify which registry's texture was meant. The API could require the caller always to pass a correct registry reference, but a mistaken cross-registry lookup can still appear to succeed when the numeric values coincide. That is the same “wrong world” hazard that an ECS encounters when two scenes allocate entity number zero.

Our handle includes a process-local owner token issued from a monotonic 64-bit counter. Registry construction mints a fresh nonzero token using an atomic compare-and-exchange so two simultaneous *registry constructions* cannot issue the same token in one program execution. This use of an atomic does **not** make the registry's container operations thread-safe; it protects only the narrow token-minting operation. The counter refuses to wrap when its namespace is exhausted, rather than silently returning a value that a previous registry might have used.

Owner tokens are deliberately **not persistent UUIDs**. A new process begins with a new counter history; two processes can mint the same number, and these tokens are not suitable as durable file IDs, cross-process networking identifiers or security capabilities. The scope of the guarantee is a single program execution of this teaching runtime. When a future engine serializes asset references, it must introduce a persistent resource key, content identity, project namespace or explicit re-resolution mechanism.

Move construction of the registry transfers the owner token and the owned slot vectors to the destination. Existing handles therefore refer to the moved-to object. The moved-from registry has token zero and must reject further imports and lookups until a separately defined reinitialization protocol is provided. That avoids duplicating one owner namespace in two live registries. Move **assignment** is deliberately deleted for this prototype, because silently destroying the destination's existing namespace would require a much more explicit handle invalidation and ownership policy. Merely declaring a class movable is not permission to leave two registries that accept the same handles.

#### A generation is a version of the occupant, not the slot address

Now simulate a slot's life over time. The first texture is installed into slot 5 at generation 1. A caller stores handle $(O,5,1,\text{Texture})$. The registry erases the texture; its C++ object is destroyed and the slot becomes unoccupied. Before allowing reuse, the registry increments the slot's generation from 1 to 2. A later valid import can then install a different texture into the vacant slot and return $(O,5,2,\text{Texture})$.

The old handle still contains the same owner and slot number, but its generation differs. Therefore it fails lookup. It cannot become a reference to the new asset merely because the storage and C++ type happen to be similar. This is an instance of the **ABA identity problem**: if a system checks only a slot coordinate or pointer, the state may go from object A to empty to object B, while an insufficiently detailed observation still looks like “the same thing.” Our generation provides a simple single-threaded way to distinguish those occupants.

The full state transition for a recyclable slot is:

```text
Empty, generation g
       |
       | import successfully completes
       v
Live<T>, generation g
       |
       | erase succeeds
       v
Empty, generation g+1
       |
       | later import, perhaps of a different T
       v
Live<U>, generation g+1
```

There are two ordering requirements. First, the new asset must be completely decoded and owned **before** the free slot is claimed. A failed import cannot consume the only available free slot or advance its generation. Second, erasure must end the occupant's lifetime and change the generation as part of the same single-threaded operation, before an observer can use the old handle again. That gives the runtime a specific checkable postcondition: after successful erase, the old handle cannot resolve, and no subsequent reuse can make it resolve again under the defined generation policy.

A 64-bit generation has a finite range. Let its maximum be $M=2^{64}-1$. Naively doing $G\gets G+1\bmod 2^{64}$ could eventually reproduce an older value, violating the “never resurrect” claim. For the ordinary case $G<M$, erase destroys the occupant, increments the generation, and pushes the slot onto the free list. For the boundary $G=M$, the registry destroys the occupant but marks its slot **permanently retired** rather than attempting an increment. The old handle fails because the slot has no live object, and this slot can never acquire a new occupant with that exhausted generation. The price is permanently lost capacity; it is consciously preferable here to aliasing a previously issued identity.

The test uses a special constructor seed of `UINT64_MAX` so this edge can be reached without executing an impossible number of removals. It deletes that one asset, verifies the slot was retired rather than recycled, and then checks that a one-slot registry refuses another import because no usable capacity remains. Production code begins at generation one; the special seed is a deterministic fault-boundary experiment, not a reason for ordinary code to initialize slots near exhaustion.

#### Reuse and vector growth are two separate algorithms

Our registry contains a `std::vector<Slot>` holding the physical slots, a second `std::vector<std::size_t>` holding indexes of free slots, and a count of currently live records. A slot owns an `std::optional<AssetRecord>` and its generation/retirement metadata. The `AssetRecord` is the same closed `std::variant<NromCartridge, TextureRgba8, TriangleMesh>` introduced in Pass 05.

If a free slot exists, a successful import constructs its new `variant` value at the end of the decoding process and removes that index from the free list. If no free slot exists and maximum capacity permits it, the registry appends a new slot to the slot vector. The storage of earlier slots may move during reallocation. Their indices and generation numbers remain the same, so handle-based lookup continues to work; borrowed references into their earlier storage do not. If no free slot exists and the maximum number of allocated slots has been reached, the importer returns a typed capacity error.

This design prioritizes a simple identity proof. A `vector<Slot>` is cache-friendly when scanning the metadata of slots and avoids a heap allocation per slot. But it does not guarantee stable object addresses while growing, and each slot contains an optional variant large enough for the largest alternative plus its tag, potentially making registry metadata expensive. A paged registry, sparse-set or separate dense payload pools may be more suitable for millions of mixed assets. The correct alternative depends on measured access patterns, handle indirection costs, allocation churn, fragmentation and required pointer stability. The existing chapter already teaches why no single container wins every workload; we now have a concrete system whose trade-offs can be measured rather than guessed.

There is also a distinction between **slot count** and **active asset count**. A one-slot registry whose only asset was deleted can contain one physical slot and zero active assets. A registry with an exhausted retired slot can also have a physical slot but no usable free entry. A vector's `capacity()` is yet a different quantity, describing available allocated element storage before another reallocation. Keeping these counts separate prevents tests and diagnostic tools from making misleading claims about how many assets are currently alive or how many more imports can succeed.

#### A strong failure guarantee requires controlling the free list as well as the asset

Before the registry can install a new asset, its source decoder must validate the external data and return a fully owned domain object. It reuses the same `Spec<Tag>::decode` implementations from Pass 05, with their checked payload sizes and NROM mapper restrictions. If validation fails, the live slot vector, free-list sequence, active count and generations remain unchanged. A synthetic failure before staging, after staging, or after a deliberately thrown `std::bad_alloc` also leaves those observable fields unchanged.

A reused slot is not popped from the free list *before* staging. Doing so would create a rollback obligation that a careless importer might omit after a decoder throws. Instead we select the free slot without modifying the list, finish construction under temporary ownership, then place the completed value into the slot and pop the free entry. The concrete `AssetRecord` alternative constructions use no-throw moves, so the final emplacement cannot propagate a new allocation exception for the supported types. A compile-time assertion checks this specific property for each `Asset<Tag>`. Without that assumption, using `std::optional::emplace` as a last-minute commit could destroy an old state or leave a slot empty on a throwing constructor, as the earlier transaction chapter demonstrated.

Growing the slot vector is another potential allocation point. `std::vector<Slot>::emplace_back` may need to acquire a larger contiguous block. Its strong guarantee in this model depends on the ability to move existing slot records without throwing and on the default construction of a new empty slot not throwing. We require `Slot` to be nothrow-move-constructible; only after the append succeeds do we construct the new asset in the fresh slot and advance the active count. The same rule of complete staging and no-throw publication underlies both the reuse and append cases.

Erasure has its own allocation trap. A naive operation might destroy the asset first and then call `free_.push_back(index)` to record the reusable slot. If that push allocates and throws, the registry has already deleted the asset but may have no bookkeeping path for reusing its slot, violating a promised strong operation guarantee. Our implementation therefore reserves sufficient capacity in the free-list vector **before** destroying the live occupant. If that reserve fails, erasure returns an allocation error with the asset still alive and its generation unchanged. Once reservation succeeds, reset, generation increment and free-list push are no-throw operations for this model.

This is a narrow but concrete strong failure guarantee: on the declared recoverable error paths, the registry remains in the same observable state, including its live assets. The implementation does not catch arbitrary exceptions from user callbacks or custom allocators, and it does not make deletion of a native GPU resource safe during in-flight work. The real promise must be expanded when those operations are added, not silently assumed today.

#### The lookup error tells us which invariant was violated

A Boolean lookup failure would hide several genuinely distinct cases. Our C++23 API distinguishes a handle presented to the wrong registry, a slot index that does not exist, a stale or dead occupant, and an existing slot whose variant alternative does not match the requested type.

These distinctions matter. A **wrong owner** often indicates a cross-world or cross-project misuse. A **missing slot** may mean a corrupted handle or an index generated for a different configuration. A **stale generation** is normal when an editor has deleted an asset and a UI selection remains open. A **wrong kind** suggests a fabricated or incorrectly decoded handle. Conflating all four as “null” would throw away useful debugging information and make it easier to accidentally create an unsafe fallback.

Our tests deliberately forge a texture handle using a mesh handle's owner, slot and generation. The C++ type parameter says “texture,” but the currently occupied slot contains a `TriangleMesh`. The runtime rejects the lookup as `wrong_kind` and also refuses to erase the mesh through the forged texture handle. This demonstrates why type-safe public APIs still need validation at their boundaries; template type distinctions do not authenticate arbitrary integer fields.

An owner token itself also is not a security capability. It is visible program data, and a caller may deliberately construct a matching token. This registry protects against accidental identity confusion and some malformed values, not adversarial callers executing arbitrary code inside the process. Security domains require separate access control and isolation.

#### Do not turn generations into a false concurrency proof

It is tempting to call a generational handle “thread-safe” because the index and generation appear small enough to load atomically. That conclusion would be wrong. The registry's `std::vector` storage, `std::optional` occupancy state, free list and mixed asset variants are ordinary mutable C++ objects. Concurrent unsynchronized access can race with writes, relocation and destruction. A generation check performed before a concurrent erase does not keep the asset alive during the later read. Even if the generation field itself were atomic, the reader could lose access to the underlying object.

For a concurrent engine, we must design an additional ownership protocol. Possibilities include a mutex or reader-writer lock around lookup and use, a pinned immutable asset object behind a stable shared lifetime, a read-copy-update/epoch scheme, or a paged registry with explicit reader leases. Each option changes latency, memory reclamation and who may destroy the resource. A GPU-rendering engine needs another layer: freeing a C++ owner does not imply that previously submitted GPU commands have finished using the native buffer. Logical handle invalidation and physical device-memory retirement are separate events.

The current teaching registry is explicitly **single-threaded**. Its atomic counter only prevents duplicate owner tokens during registry construction, not data races in reads, imports or erases. The laboratory never launches concurrent readers and writers. A correct research report must state this boundary instead of claiming thread safety from a successful multi-thread-free test.

#### Evidence against three tempting incorrect designs

First, suppose we returned a pointer into the slot vector as the permanent asset ID. The 701-asset growth test intentionally forces the registry to add many slots; this can cause vector reallocation, so previously borrowed pointers are not safe afterward. The test never dereferences any stale pointer. It saves the logical handle and calls `lookup` again to fetch the current location. The test's success demonstrates address-independent re-resolution; the C++ container specification supplies the invalidation rule.

Second, suppose we reused slot zero without a generation change. A stale UI selection or gameplay component would silently refer to the new asset. Our test inserts a texture, deletes it, rejects repeated deletion, and reimports another texture into the same free slot. It verifies that the old handle fails while the new one resolves with generation incremented. It also checks that unsuccessful staging operations do **not** consume the free slot or advance its generation. This distinguishes state-machine correctness from simply having a generation field in the struct.

Third, suppose the registry identified assets with only slot and generation. Two independent registries could both issue $(0,1)$. The test creates both, imports one texture into each, and demonstrates that the owner tokens differ even though their indices and generations agree. It then moves the first registry and checks that the existing handle resolves against the moved-to owner, while the moved-from object refuses to mint another asset under the same namespace.

These are separate demonstrations of **physical relocation**, **temporal identity**, and **registry ownership**. None alone substitutes for the others.

#### The deterministic 2,000-cycle test is not a general proof, but it is useful

A class can pass a single delete/reinsert test and still mishandle the free list after many operations. The new regression suite therefore keeps one real NROM cartridge live while it repeatedly deletes and replaces a texture in another slot for two thousand cycles. An independent counter predicts the expected generation and the expected byte value of the newly decoded texture. Every cycle checks the outgoing handle becomes stale, the replacement returns the same slot with a different generation, and the unrelated cartridge's PRG byte remains intact.

That test exercises the actual NROM alternative imported through our common `Spec<CartridgeTag>`, not a fake stub. At the end there should still be two live assets and two allocated slots, not thousands of abandoned slot records. It also covers the boundary cases of a zero-capacity registry, a nonexistent slot index, a wrong-kind handle, and permanent retirement when the generation reaches its maximum.

However, two thousand transitions are finite evidence. They do not exhaust all possible interleavings, memory allocation failures or corruption patterns. To strengthen the experiment in a future research pass, generate random operation sequences using a fixed seed and compare the registry after each step against an independent, simpler reference state machine. Track successful insertions, reuses, retirements, failed deletions and peak allocated slots as separate metrics. This leads naturally into the next chapter's data-structure selection and measurement rather than treating a microbenchmark as a correctness proof.

#### Complexity and memory claims require the right level of abstraction

With a vector of slots, indexed lookup performs a constant number of bounds, owner, generation and type checks independent of the number of existing assets, assuming those ordinary operations are constant-time. It is therefore reasonable to describe the expected **single-threaded lookup work** as $O(1)$ in a conventional RAM model. This does not mean a lookup takes a fixed number of processor cycles independent of cache state, branch prediction, memory hierarchy or the complexity of the caller's later operation on the asset.

Appending an all-new slot has amortized constant relocation work under ordinary geometric vector growth, but one particular append can require moving every previous slot record. Decoding the image or mesh remains proportional to its input payload size and is not absorbed into the slot allocation's asymptotic bound. Erasing an asset may be proportional to the cost of destroying its owned payload; its free-list bookkeeping is small but `reserve` can allocate or move free-list indices. Reusing a free slot avoids growing the slot vector, but still pays for domain decoding and any owned payload allocation.

An important engineering metric is not “how many operations per second” in isolation. It is the distribution of frame-time stalls, bytes allocated, bytes retained by retired slots, cache misses during lookups, and the cost of moving metadata when capacity grows. Those quantities should be benchmarked using identical workloads and independent correctness oracles before arguing that a paged registry, sparse set, separate pools or this vector-based model is the right choice for the game engine.

#### Rebuild the registry as a proof, not as a recipe

The learner's first reconstruction should draw the slot state machine and define precisely when a handle is live. Then implement the four independent checks for owner, bounds, generation/occupancy and type. Build an initially empty registry and import the same actual NES cartridge, texture and mesh types from Pass 05, preserving their existing source validators. Erase one asset and verify that its handle fails while the unaffected assets remain accessible. Reuse the vacant index with a different domain type and verify that both the former generation and wrong-type forged handles are rejected.

Next, implement a staged import that leaves the free list untouched while source decoding occurs. Inject a failure before staging, after staging, and after simulated allocation failure. Check that previous assets, generations, active counts and free-slot availability are unchanged in every refused case. For deletion, move the free-list reserve step ahead of destruction and explain which guarantee that ordering supplies. Force a new slot allocation to grow the slot vector, then resolve earlier handles without using stale raw pointers.

Afterward, add a second registry, compare owner tokens, move-construct one registry, and prove that handles follow the moved-to namespace. Finally, use the configurable test generation to reach the 64-bit limit and verify permanent slot retirement. Write down a proof sketch of why an old handle cannot become valid again: a slot's generation increases monotonically while reusable; it never wraps; once exhausted, the slot is never reused; and owner tokens never repeat during this process execution. Explain which assumptions would fail if a registry were serialized, moved between processes, or accessed concurrently without synchronization.

The completion criterion is not memorizing “index plus generation.” It is reconstructing the precise **state and failure protocol**, deriving the conditions under which handle lookup is permitted, and distinguishing legal long-lived identity from short-lived borrowed access to its current storage. The test suite should reject intentionally broken variants of these invariants; merely running it and seeing a success line does not replace that understanding.

**Source and integration boundaries.** Compare vector iterator/reference invalidation with [cppreference vector](https://en.cppreference.com/w/cpp/container/vector), the language's object lifetime contracts with the [C++ draft](https://eel.is/c++draft/basic.life), and typed variant access with [std::variant](https://en.cppreference.com/w/cpp/utility/variant). The source chapter's original named registry and container experiments remain intact and distinct. Our new code is an additive C++23 project integration; it has not been merged into the real external Kairo game engine or Maveb. No GPU queue retirement, persistent asset identifier, security capability, concurrency-safe registry, multi-format production importer or complete NES emulator is claimed.

**Next chapter boundary.** “Typed Interfaces, Identity, and Data Structures” will use this registry's real success and failure cases to examine more advanced interfaces, algorithmic data structures and empirical trade-offs. The system must continue to evolve from these exact shared C++23 types rather than restarting with unrelated one-off snippets.
