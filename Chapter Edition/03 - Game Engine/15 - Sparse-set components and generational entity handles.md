# 15 — Sparse-set components and generational entity handles

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/14 - Application clocks and stable input identity|Previous chapter]] · [[Chapter Edition/03 - Game Engine/16 - Archetypes, chunks, relationships, and transforms|Next chapter]]

As the world grows, storing every entity as a separate object becomes costly and references can outlive their occupants. This chapter derives dense component pools, sparse lookup, and generation checks from those constraints.

**By the end:** Implement component membership and entity identity while accounting for swap removal, slot reuse, and stale handles.

**Recall question:** After an entity is removed and its slot reused, what check proves that an old handle cannot refer to the new entity?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 26787–30485; source-span SHA256 18e61883b4093cd8d2128f7c7fd86eb43d1edc8198491d3160d14f2075582ec1. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### COMPONENT POOLS AND SPARSE SETS

---

**Component pools**

A sparse-set ECS usually stores one pool per component type:

```text
Transform Pool
----------------
entities
components

Velocity Pool
----------------
entities
components

Health Pool
----------------
entities
components
```

An entity with Transform and Health exists in both corresponding pools.

There is no requirement that pools have equal sizes.

Example:

```text
100,000 entities total
100,000 transforms
 20,000 velocities
    300 bosses
     12 directional lights
```

That density difference is what later makes smallest-pool queries powerful.

---

**Type erasure in Kairo pools**

Your registry needs to do this during entity destruction:

```cpp
for (every component pool)
    remove(entity);
```

But the registry does not statically know every component type.

So Kairo uses:

```cpp
struct StorageBase
{
    virtual ~StorageBase() = default;

    virtual bool Remove(
        Entity entity) noexcept = 0;
};
```

and:

```cpp
template<typename Component>
struct Pool final : StorageBase
{
    SparseSet<Component> Set;

    bool Remove(
        Entity entity) noexcept override
    {
        return Set.Remove(entity);
    }
};
```

This is type erasure.

At typed access time:

```cpp
Pool<Transform>
```

is known.

At generic destruction time:

```cpp
StorageBase*
```

is enough.

That is a clean division.

---

**Why Kairo keeps both `m_Pools` and `m_Storages`**

You have:

```cpp
unordered_map<type_index, unique_ptr<StorageBase>>
    m_Pools;
```

and:

```cpp
vector<StorageBase*>
    m_Storages;
```

The map answers:

> give me the pool for type T.

The vector answers:

> iterate every existing pool quickly.

The unique ownership remains in `m_Pools`.

`m_Storages` contains non-owning pointers.

This avoids traversing an unordered map when bulk iterating all storages.

However, entity destruction still has complexity:

$$
O(T)
$$

where $T$ is the total number of registered component types, because it asks every pool whether the entity is present.

Later signatures or per-entity component lists can reduce that.

---

**Sparse-set concept**

Suppose entities owning Position are:

```text
Entity indices:
2, 7, 100
```

We want:

1. $O(1)$ membership lookup by entity,
2. dense contiguous component iteration.

A sparse set combines two representations.

```text
SPARSE ARRAY
indexed by entity index

entity index:
0 1 2 3 4 5 6 7 8 ... 100
    . 0 . . . . 1 . ...  2


DENSE ENTITY ARRAY

dense index:
0   1    2

entity:
2   7   100


DENSE COMPONENT ARRAY

dense index:
0       1       2

Position:
P2      P7      P100
```

The sparse array maps:

$$
entityIndex\rightarrow denseIndex.
$$

The dense arrays map:

$$
denseIndex\rightarrow entity/component.
$$

This exact sparse-to-packed architecture is also the core model documented by EnTT: its component pools are specialized sparse sets with packed entity/component storage; modern EnTT additionally pages sparse and component storage to control memory and pointer stability. [skypjack.github.io](https://skypjack.github.io/entt/md_docs_2md_2entity.html?utm_source=chatgpt.com)

---

**The fundamental sparse-set invariant**

For every dense slot $d$:

$$
\boxed{
Sparse[DenseEntity[d].Index]=d.
}
$$

And for entity E claiming membership:

$$
d=Sparse[E.Index].
$$

Then membership is valid only if:

$$
d<Dense.size
$$

and:

$$
DenseEntity[d]=E.
$$

That last equality is where generations enter.

This invariant is the heart of the entire data structure.

---

**Sparse index array**

Kairo stores:

```cpp
std::vector<std::uint32_t> m_Sparse;
```

Missing slots contain:

```cpp
Missing =
    std::numeric_limits<uint32_t>::max();
```

So:

```text
entity index 0 -> Missing
entity index 1 -> Missing
entity index 2 -> 0
entity index 3 -> Missing
entity index 7 -> 1
```

Lookup does not search.

It indexes directly:

$$
dense=Sparse[E.Index].
$$

That's why membership is $O(1)$.

---

**Sparse-array memory cost**

Suppose highest entity index is:

$$
10,000,000.
$$

Even if this component exists on only 3 entities, a flat `uint32_t` sparse array may require about:

$$
10,000,001\times4
\approx40\text{ MB}.
$$

For **one component type**.

If dozens of sparse components independently allocate huge sparse arrays, memory explodes.

Kairo's current flat sparse vector therefore works especially well when:

$$
\text{entity indices are reasonably dense}.
$$

Production ECS libraries often page sparse arrays precisely to avoid allocating the entire index range for every pool. EnTT's current storage documentation explicitly notes paged sparse arrays for this reason. [skypjack.github.io](https://skypjack.github.io/entt/md_docs_2md_2entity.html?utm_source=chatgpt.com)

A paged sparse set might allocate only pages touched by entities.

---

**Dense entity array**

Kairo stores:

```cpp
std::vector<Entity> m_Entities;
```

This array tells us:

> Which entity owns component at dense index $d$?

Example:

```text
dense 0 -> Entity {42,3}
dense 1 -> Entity {7,1}
dense 2 -> Entity {921,8}
```

Dense order has no requirement to match entity index order.

That is important.

We trade ordering for compactness.

---

**Dense component array**

Kairo stores:

```cpp
std::vector<Component> m_Components;
```

with exact positional correspondence:

$$
m_Entities[d]
\leftrightarrow
m_Components[d].
$$

So:

```text
DenseEntities[5] = Entity 73
DenseComponents[5] = Transform of Entity 73
```

The invariant:

$$
|Entities|=|Components|.
$$

This is why iteration can simply walk:

```cpp
for (size_t i = 0;
     i < components.size();
     ++i)
{
    Entity e = entities[i];
    Component& c = components[i];
}
```

No pointer chasing.

---

**Sparse-set membership lookup**

Current Kairo:

```cpp
bool Contains(Entity entity) const noexcept
{
    if (!entity.IsValid() ||
        entity.Index >= m_Sparse.size())
    {
        return false;
    }

    const std::uint32_t dense =
        m_Sparse[entity.Index];

    return
        dense != Missing &&
        dense < m_Entities.size() &&
        m_Entities[dense] == entity;
}
```

Let's derive every check.

**Valid index sentinel**

Reject null-like handles.

**Sparse bounds**

Avoid out-of-bounds indexing.

**Missing sentinel**

Entity index has no dense mapping.

**Dense bounds**

Defensive corruption check.

**Full entity equality**

Protect against stale generations.

Every line has a purpose.

---

**Why sparse index alone is insufficient**

Suppose:

```text
old entity = {42,1}
new entity = {42,2}
```

Sparse array says:

```text
Sparse[42] = 7
```

If membership checked only:

```cpp
Sparse[42] != Missing
```

then both generations would appear to own the component.

Generation comparison prevents that:

```cpp
DenseEntities[7] == Entity{42,2}
```

means old handle `{42,1}` fails.

This makes the sparse set generation-aware.

---

**Sparse-set component lookup**

Once membership is known:

$$
dense=Sparse[E.Index].
$$

Then:

$$
component=DenseComponents[dense].
$$

Kairo:

```cpp
Component* TryGet(Entity entity) noexcept
{
    return Contains(entity)
        ? &m_Components[
              m_Sparse[entity.Index]]
        : nullptr;
}
```

So successful lookup involves roughly:

```text
entity index
  ↓
sparse array load
  ↓
dense entity validation
  ↓
dense component access
```

No hash table is needed per entity/component lookup.

---

**Why this can outperform entity→component hash maps**

An alternative would be:

```cpp
unordered_map<Entity, Transform>
```

for every component.

That introduces:

- hash calculation,
- bucket lookup,
- pointer chasing,
- bucket metadata,
- poor iteration locality.

Sparse set uses direct indexing and contiguous packed data.

The tradeoff is sparse-array memory.

This is a classic space/time/layout trade.

---

**Sparse-set component insertion**

Insertion needs to preserve the invariant:

$$
Sparse[Entity.Index]=DensePosition.
$$

Kairo first ensures sparse capacity:

```cpp
EnsureSparseIndex(entity.Index);
```

Then verifies that entity does not already exist.

It also checks something subtle:

```cpp
if (m_Sparse[entity.Index] != Missing)
{
    throw ...
}
```

even after `Contains(entity)` returned false.

Why?

Because the slot could belong to a **different generation**.

---

**The different-generation insertion hazard**

Suppose:

```text
dense entities:
[ Entity{42,1} ]

Sparse[42] = 0
```

Now caller attempts:

```cpp
Emplace(Entity{42,2}, ...)
```

`Contains({42,2})` returns false because generation differs.

If insertion blindly proceeded:

```text
dense:
0 -> {42,1}
1 -> {42,2}

Sparse[42] = 1
```

Now old dense entry 0 exists but no sparse mapping can reach it.

It became an **orphan component**.

Your implementation detects this and rejects insertion until old generation is removed.

That is a genuinely important invariant defense.

---

**Dense insertion algorithm**

Let:

$$
d=|Dense|.
$$

Then:

```text
DenseEntities.push(E)
DenseComponents.emplace(component)
Sparse[E.Index] = d
```

Kairo does:

```cpp
const std::uint32_t dense =
    CheckedDenseIndex();

m_Entities.push_back(entity);

try
{
    m_Components.emplace_back(
        std::forward<Arguments>(arguments)...);
}
catch (...)
{
    m_Entities.pop_back();
    throw;
}

m_Sparse[entity.Index] = dense;
```

Note that sparse mapping is assigned last.

That is crucial for exception safety.

---

**Why sparse mapping must be updated last**

Suppose component construction throws.

If code had already done:

```cpp
Sparse[e.Index] = newDense;
```

then sparse would point to a component that never successfully existed.

Instead Kairo leaves sparse untouched until both dense arrays are successfully extended.

If component construction throws:

```text
entity push -> rolled back
component -> never exists
sparse -> still Missing
```

Internal state remains valid.

Good container engineering again.

---

**Swap-and-pop removal**

Removing an element from the middle of `std::vector` normally shifts everything after it:

$$
O(N).
$$

ECS pools generally do not require stable order.

So swap with the last dense element, then pop.

Suppose:

```text
dense index:

0   1   2   3   4

A   B   C   D   E
```

Remove B at index 1.

Move E into slot 1:

```text
0   1   2   3

A   E   C   D
```

Then pop old last slot.

Complexity:

$$
O(1).
$$

---

**Kairo removal code**

```cpp
const uint32_t dense =
    m_Sparse[entity.Index];

const uint32_t last =
    static_cast<uint32_t>(
        m_Entities.size() - 1u);

if (dense != last)
{
    m_Entities[dense] =
        std::move(m_Entities[last]);

    m_Components[dense] =
        std::move(m_Components[last]);

    m_Sparse[
        m_Entities[dense].Index
    ] = dense;
}

m_Entities.pop_back();
m_Components.pop_back();

m_Sparse[entity.Index] = Missing;
```

The critical step is not the move itself.

It is repairing sparse metadata.

---

**Repairing sparse indices**

Before removal:

```text
Sparse[A] = 0
Sparse[B] = 1
Sparse[C] = 2
Sparse[D] = 3
Sparse[E] = 4

Dense:
0 A
1 B
2 C
3 D
4 E
```

Move E:

```text
Dense:
0 A
1 E
2 C
3 D
```

If we forget to change:

$$
Sparse[E],
$$

it remains:

$$
4.
$$

But dense slot 4 no longer exists.

So after moving:

$$
\boxed{
Sparse[E.Index]\leftarrow1.
}
$$

Finally:

$$
Sparse[B.Index]\leftarrow Missing.
$$

This restores the sparse-set invariant.

---

**The removal invariant proof**

Let removed entity be $R$, dense index $d$, and final entity be $M$, previous last index $l$.

Before:

$$
Sparse[M.Index]=l.
$$

After move:

$$
DenseEntity[d]=M.
$$

Therefore invariant requires:

$$
Sparse[M.Index]=d.
$$

Then R no longer exists:

$$
Sparse[R.Index]=Missing.
$$

All other entries remain unchanged.

Hence invariant holds globally.

That is the entire correctness proof of swap-remove.

---

**Component address invalidation**

This is where many ECS APIs become dangerous.

Suppose:

```cpp
Transform* t =
    registry.TryGet<Transform>(entity);
```

You keep `t`.

Then another Transform is inserted.

If:

```cpp
m_Components.push_back(...)
```

causes `std::vector` reallocation, **every component pointer/reference in that vector becomes invalid**.

So even insertion can invalidate references.

---

**Removal has an even nastier hazard**

Suppose:

```text
dense:
0 A
1 B
2 C
3 D
```

You hold:

```cpp
Transform* ptrD =
    &components[3];

Transform* ptrB =
    &components[1];
```

Remove B.

D moves:

```text
0 A
1 D
2 C
```

Now:

- `ptrB` points to memory now representing D,
- `ptrD` points into the popped tail slot and is invalid.

The first case is especially dangerous.

The pointer may still point to valid memory.

It simply refers to a **different logical entity**.

That can create silent corruption rather than an immediate crash.

---

**ECS references require explicit lifetime contracts**

A safe rule is:

$$
\boxed{
\text{component references are temporary views, not stable handles}.
}
$$

Use them during one known-safe operation:

```cpp
auto& transform =
    registry.Get<Transform>(e);

transform.position += delta;
```

Do not casually cache them across structural changes.

If long-lived access is needed, keep:

```cpp
Entity
```

and call:

```cpp
TryGet<T>(entity)
```

again when needed.

Identity is stable.

Component addresses are not necessarily stable.

---

**`Reserve()` helps but does not solve removal invalidation**

Kairo supports:

```cpp
registry.Reserve<Transform>(100000);
```

which reserves dense vector capacity.

That can prevent reallocation during expected insertions.

Therefore component addresses may remain stable **under insertion until capacity is exceeded**.

But swap-removal can still move components.

So reserve reduces one invalidation mechanism.

It does not create a stable-address guarantee.

---

**Stable-component storage alternatives**

If components must keep stable addresses, several strategies exist.

**Paged storage**

Allocate components in fixed pages:

```text
page 0
[ ][ ][ ][ ]

page 1
[ ][ ][ ][ ]

page 2
[ ][ ][ ][ ]
```

Adding a page does not relocate old pages.

Lookup becomes:

$$
page=i/P
$$

$$
offset=i\bmod P.
$$

This preserves pointer stability across growth.

Modern EnTT explicitly uses paged component storage to provide pointer stability on additions, while retaining packed entity arrangements and optional in-place deletion policies. [skypjack.github.io](https://skypjack.github.io/entt/md_docs_2md_2entity.html?utm_source=chatgpt.com)

---

**Tombstones / in-place deletion**

Instead of swapping the last component into the removed slot:

```text
before:
A B C D

remove B:

A [hole] C D
```

Address of C and D remains stable.

But now iteration encounters holes.

You need:

- tombstone checks,
- free-slot reuse,
- periodic compaction,
- more complex indexing.

Stable addresses and perfectly dense iteration are opposing goals.

---

**Indirection storage**

Another option:

```text
dense entity list
      |
      v
component handles/pointers
      |
      v
stable component allocation
```

Then ECS can reorder handles without moving components.

But every component access introduces another pointer chase.

You sacrifice locality.

Again:

$$
\boxed{
\text{there is no free stability}.
}
$$

---

**Dense component iteration**

The payoff for sparse sets is here.

Suppose system updates every Position:

```cpp
for (Position& p :
     positionPool.Components())
{
    p.x += ...;
}
```

Components are contiguous.

CPU sees approximately:

```text
P0 P1 P2 P3 P4 P5 P6 P7 ...
```

rather than:

```text
pointer -> object
pointer -> object
pointer -> object
```

Hardware prefetchers can predict sequential access.

Cache lines deliver several components at once.

---

**Why contiguous iteration is so powerful**

Assume a hypothetical 64-byte cache line and a 12-byte Position:

```cpp
struct Position
{
    float x, y, z;
};
```

One line contains roughly five Positions worth of useful data, ignoring boundary effects.

A pointer-heavy object hierarchy might load an entire 128-byte or 256-byte object even when the system only needs 12 bytes of position.

Data-oriented design asks:

$$
\boxed{
\text{what data does this loop actually consume?}
}
$$

and stores that data accordingly.

This is the philosophical center of ECS performance.

---

**Current `Registry::Each<T>`**

Kairo does:

```cpp
const auto& entities =
    pool->Set.Entities();

auto& components =
    pool->Set.Components();

for (std::size_t index = 0;
     index < count;
     ++index)
{
    const Entity entity =
        entities[index];

    if (IsAlive(entity))
    {
        std::invoke(
            callable,
            entity,
            components[index]);
    }
}
```

This is a dense linear traversal.

The `IsAlive()` check should normally always succeed if registry invariants hold, because destroying entities removes their components.

It is an extra safety check.

Potential optimization in highly tuned code would be to rely on invariants and omit redundant liveness checks from hot loops, but only after proving registry mutation rules prevent stale dense entries.

Correctness first.

---

**Multi-component query**

Suppose system needs:

```text
Position
Velocity
Health
```

with pool sizes:

$$
|P|=1,000,000
$$

$$
|V|=400,000
$$

$$
|H|=10,000.
$$

You could scan every Position and ask:

```text
Has Velocity?
Has Health?
```

That performs around one million primary iterations.

Or scan Health:

$$
10,000
$$

and test:

```text
Has Position?
Has Velocity?
```

Much cheaper.

Therefore:

$$
\boxed{
\text{iterate the smallest required pool}.
}
$$

---

**Complexity of smallest-pool joins**

Query requires $k$ component types.

Let smallest pool size be:

$$
m.
$$

Each candidate performs approximately:

$$
k-1
$$

constant-time sparse membership tests.

Complexity:

$$
\boxed{
O(m(k-1)).
}
$$

For fixed small $k$:

$$
O(m).
$$

This is one of the strongest features of sparse-set ECS.

---

**Current Kairo two-component join**

Your registry explicitly chooses the smaller pool.

If:

```cpp
first->Set.Size()
<=
second->Set.Size()
```

scan first and sparse-lookup second.

Otherwise scan second.

Yet callback order remains:

```cpp
Entity, First&, Second&
```

regardless of which pool drives iteration.

That is excellent API behavior.

Storage optimization does not leak into callback semantics.

---

**Current three-component join**

The same strategy is manually expanded for three pools.

It identifies minimum of:

```text
First
Second
Third
```

and scans it.

Then performs `TryGet` on the other two.

Your tests explicitly construct:

```text
Position = 128
Velocity = 64
Health   = 8
```

and verify the join visits only the eight entities satisfying all components.

That is exactly the workload where smallest-pool iteration shines.

---

**Multi-pool membership tests**

For entity E and query:

$$
Q=\{A,B,C\},
$$

if A drives iteration:

```cpp
if (poolB.Contains(E) &&
    poolC.Contains(E))
{
    execute(E);
}
```

Each membership lookup is:

```text
entity index
    ↓
sparse index
    ↓
dense owner validation
```

roughly constant-time.

This is an intersection operation:

$$
A\cap B\cap C.
$$

The sparse set turns set intersection into direct indexed membership tests rather than sorting/merging IDs every frame.

---

**Required versus excluded components**

Production queries often include exclusions.

Example:

```text
Position
Velocity
WITHOUT Frozen
WITHOUT Dead
```

The condition is:

$$
E\in Position
\land
E\in Velocity
\land
E\notin Frozen
\land
E\notin Dead.
$$

Smallest required pool can still drive the query.

Then membership tests evaluate positive and negative constraints.

---

**Component signatures / bit masks**

Suppose there are at most 64 component types.

Assign each a bit:

```text
Transform = bit 0
Velocity  = bit 1
Health    = bit 2
Light     = bit 3
```

Entity signature:

```text
Transform + Health

000...0101
```

Query requiring Transform + Health:

```text
required = 0101
```

Entity matches if:

$$
\boxed{
(signature\ \&\ required)=required.
}
$$

Excluded mask:

$$
(signature\ \&\ excluded)=0.
$$

---

**Example signature code**

```cpp
using Signature = std::uint64_t;

constexpr Signature Bit(
    std::uint32_t componentID)
{
    return
        Signature{1}
        << componentID;
}

bool Matches(
    Signature entity,
    Signature required,
    Signature excluded)
{
    return
        (entity & required) == required
        &&
        (entity & excluded) == 0;
}
```

This makes set-membership predicates extremely cheap.

---

**Signatures do not replace component pools**

A signature can answer:

> Does entity 42 conceptually own Transform?

It does not tell you where Transform is stored.

You still need:

$$
SparseSet<Transform>
$$

or another storage backend.

Signatures are metadata.

Sparse sets are storage/index structures.

They complement each other.

---

**Signatures can accelerate entity destruction**

Current Kairo destruction does:

```cpp
for (every registered component pool)
    pool.Remove(entity);
```

If there are 200 component types but entity owns 4, this checks 196 irrelevant pools.

With entity signature:

```text
bits set:
3, 19, 77, 91
```

you can iterate only those four component storages.

That changes destruction work from roughly:

$$
O(T)
$$

to:

$$
O(C_e),
$$

where $C_e$ is number of components actually owned.

---

**The problem with a fixed 64-bit signature**

Once component count exceeds 64:

```cpp
uint64_t
```

is insufficient.

Could use:

```cpp
std::bitset<256>
```

if maximum is known at compile time.

But a plugin architecture may not know component count in advance.

Then we need dynamic bitsets.

---

**Dynamic bitsets**

Represent bitset as:

```cpp
std::vector<std::uint64_t> words;
```

For component ID $c$:

$$
word=c/64
$$

$$
bit=c\bmod64.
$$

Set:

```cpp
words[word] |=
    uint64_t{1} << bit;
```

Test:

```cpp
(words[word] &
 (uint64_t{1} << bit))
!= 0;
```

---

**Dynamic signature matching**

Required query signature contains multiple words.

Entity matches if for every word $i$:

$$
(E_i\ \&\ R_i)=R_i.
$$

C++:

```cpp
bool ContainsAll(
    std::span<const uint64_t> entity,
    std::span<const uint64_t> required)
{
    for (std::size_t i = 0;
         i < required.size();
         ++i)
    {
        const uint64_t e =
            i < entity.size()
                ? entity[i]
                : 0;

        if ((e & required[i])
            != required[i])
        {
            return false;
        }
    }

    return true;
}
```

---

**Dynamic bitsets can become expensive**

Suppose:

$$
256
$$

possible component types.

That requires:

$$
256/8=32\text{ bytes}
$$

of signature per entity.

For one million entities:

$$
32\text{ MB}
$$

just for signatures.

And most entities may use only a handful of those component types.

This is one motivation for archetype systems, where a signature belongs to a **group/archetype** rather than being duplicated per entity.

That is Pass 11 territory, but you should already see why it emerges.

---

**Kairo currently has no component signature layer**

Current membership is inferred through sparse-set ownership.

There is no:

```text
entity component mask
dynamic bitset
component signature
```

in the current `Registry`.

That is perfectly valid for a sparse-set ECS.

EnTT itself deliberately describes its registry model as sparse-set based and bitset-free, demonstrating that signatures are an optional architecture choice rather than an ECS requirement. [skypjack.github.io](https://skypjack.github.io/entt/md_docs_2md_2entity.html?utm_source=chatgpt.com)

The curriculum requires understanding signatures because they become especially important in archetype matching, query caching and generic tooling.

---

**Query caching**

Suppose every frame you run:

```text
Position + Velocity + Health
```

The naive query setup repeatedly does:

```text
find Position pool
find Velocity pool
find Health pool
determine smallest pool
```

before iteration.

That work is avoidable.

A query object can cache:

```cpp
struct CachedQuery
{
    Pool<Position>* PositionPool;
    Pool<Velocity>* VelocityPool;
    Pool<Health>* HealthPool;

    DriverKind Driver;
};
```

Then the query can run directly.

---

**Level 1 query cache: cache storage lookup**

This is safest.

Cache:

```text
type -> pool pointers
driver pool
```

but still determine membership live.

Structural changes do not invalidate the pool objects themselves as long as pool addresses remain stable.

In Kairo, pools are heap-allocated under `unique_ptr`, so `unordered_map` rehashing moves `unique_ptr` objects but does not move the allocated Pool object.

Thus cached raw `Pool<T>*` can remain valid until a pool is explicitly destroyed—which current Registry never does.

That makes this caching strategy attractive.

---

**Level 2 query cache: cache matching entities**

You could cache:

```text
entities currently matching
Position + Velocity + Health
```

Then iteration becomes extremely cheap.

But adding/removing a component changes membership.

Now cache requires invalidation.

Use a structural version:

```cpp
uint64_t m_StructureVersion;
```

Increment on:

```text
entity create
entity destroy
component add
component remove
```

Query stores:

```cpp
uint64_t CachedVersion;
```

If:

$$
CachedVersion\neq RegistryVersion,
$$

rebuild matching set.

---

**Global structural versions can over-invalidate**

Suppose query requires:

```text
Position + Velocity
```

and someone adds:

```text
AudioEmitter
```

to unrelated entity.

A global version invalidates the Position+Velocity cache unnecessarily.

More granular solution:

```text
Position pool version
Velocity pool version
Health pool version
...
```

Cache records versions only for pools it depends on.

Or maintain event-driven incremental query membership.

That is more complex but can scale better.

Current ECS libraries often have explicit cached query mechanisms and change/version tracking for exactly this reason; Flecs, for example, distinguishes cached queries and tracks structural/component changes to determine whether matched data changed. [Flecs](https://www.flecs.dev/flecs/Queries.html?utm_source=chatgpt.com)

---

**Cached entity lists versus sparse-set joins**

Do not assume caching all result entities is automatically faster.

Suppose structure changes frequently:

```text
thousands of add/remove operations every frame
```

Then maintaining query membership may cost more than simply scanning the smallest pool.

The decision is workload-dependent:

$$
C_{\text{cache}}
=
C_{\text{maintenance}}
+
C_{\text{iteration}}.
$$

Compare with:

$$
C_{\text{uncached}}
=
C_{\text{pool lookup}}
+
m(k-1)\cdot C_{\text{sparse lookup}}.
$$

Benchmark.

---

**Structural changes**

A structural change modifies **which components/entities exist or where they are stored**.

Examples:

```text
Create entity
Destroy entity
Add component
Remove component
```

Changing:

```cpp
position.x += 1;
```

is not structural.

The component stays in the same pool.

Adding:

```cpp
registry.Emplace<Velocity>(entity);
```

is structural because the Velocity pool changes.

Removing Transform is structural because dense storage can reorder.

---

**Why structural mutation during iteration is dangerous**

Suppose:

```cpp
registry.Each<Health>(
    [&](Entity e, Health& health)
    {
        if (health.Value <= 0)
            registry.Destroy(e);
    });
```

Assume dense Health pool:

```text
index
0 A
1 B
2 C
3 D
```

Loop currently at:

$$
i=1
$$

on B.

Destroy B.

Swap-remove moves D into index 1:

```text
0 A
1 D
2 C
```

Loop increments:

$$
i=2.
$$

D is never visited.

The structural mutation changed iteration order under the iterator.

---

**It can be worse than skipping an element**

If insertion reallocates:

```cpp
m_Components.emplace_back(...)
```

then references currently held by the loop may become dangling.

If entity destruction removes components from the primary pool, loop size changes.

If another callback stores `Component&` and later structural change swap-moves that slot, the reference may now denote a different entity.

Thus "don't mutate while iterating" is not stylistic.

It is an ownership/invalidation rule.

---

**Current Kairo explicitly documents the restriction**

Your `Registry::Each` says, in effect:

> do not add/remove the primary component or destroy entities during this call because dense storage may reorder.

That is correct.

But the next question is inevitable:

> What if gameplay logically needs to destroy an entity inside a system?

Answer:

$$
\boxed{
\text{defer the structural change}.
}
$$

---

**Deferred entity creation**

Suppose during projectile update:

```cpp
if (exploded)
{
    CreateExplosionEntity();
}
```

Creating immediately may mutate multiple component pools.

Instead record:

```text
Create entity command
```

and execute it after iteration.

Problem:

> How do later commands refer to an entity that does not exist yet?

Use a temporary token.

```cpp
struct DeferredEntity
{
    uint32_t Token;
};
```

Then:

```cpp
DeferredEntity explosion =
    commands.Create();

commands.Emplace<Transform>(
    explosion,
    transform);

commands.Emplace<Explosion>(
    explosion,
    explosionData);
```

At flush time:

```text
token 7 -> real Entity{241,6}
```

Subsequent commands resolve token 7 to that entity.

---

**Alternative: reserve identity immediately**

Another architecture is:

```text
allocate entity ID now
defer only storage mutation
```

Then systems immediately obtain a normal `Entity`.

This simplifies references between queued commands.

But now "creation is deferred" only partially; registry entity metadata changed immediately.

Whether that is safe depends on threading and iteration model.

In a fully readonly multithreaded stage, you may want **no world mutation at all** until merge.

Hence temporary deferred tokens are the stronger general model.

---

**Deferred entity destruction**

Instead of:

```cpp
registry.Destroy(entity);
```

inside a system:

```cpp
commands.Destroy(entity);
```

Physical destruction occurs later.

But now you need to define semantics.

During the remainder of this system pass, is the entity considered alive?

Two common models:

**Fully deferred semantics**

Until flush:

```cpp
registry.IsAlive(entity) == true;
```

Other systems in the same stage can still observe it.

**Logical tombstone semantics**

Mark entity:

```text
PendingDestroy
```

immediately for query/filter purposes, but delay actual pool mutation.

This provides more intuitive "dead now" behavior but adds complexity.

A command buffer must define this explicitly.

---

**Deferred component add/remove**

Same principle:

```cpp
commands.Emplace<Velocity>(
    entity,
    velocity);

commands.Remove<Health>(
    entity);
```

Nothing touches dense arrays until flush.

During the iteration:

```text
Position pool
Velocity pool
Health pool
```

remain structurally stable.

That is what makes current pointers/iterators safe.

---

**Command buffers**

A command buffer is not merely:

```cpp
vector<enum Command>
```

It is a **transaction log of deferred structural intent**.

Conceptually:

```text
CommandBuffer

0 Create token #0
1 Add Transform to token #0
2 Add Explosion to token #0
3 Remove Health from Entity A
4 Destroy Entity B
5 Add Velocity to Entity C
```

At a synchronization point:

```text
finish iteration
      ↓
flush commands in defined order
      ↓
registry becomes mutable
      ↓
apply structural mutations
```

Then next systems observe the new structure.

---

**A practical C++23 command-buffer skeleton**

Since Kairo is C++23, a useful implementation can use `std::move_only_function`.

```cpp
#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

struct DeferredEntity
{
    std::uint32_t Token = 0;
};

using EntityReference =
    std::variant<Entity, DeferredEntity>;

class ECSCommandBuffer
{
public:
    DeferredEntity Create()
    {
        DeferredEntity token{
            static_cast<std::uint32_t>(
                m_DeferredEntities.size())
        };

        m_DeferredEntities.push_back(Entity{});

        // Creation itself is executed in stream order.
        m_Commands.emplace_back(
            [token](
                Registry& registry,
                std::vector<Entity>& resolved)
            {
                resolved[token.Token] =
                    registry.Create();
            });

        return token;
    }

    void Destroy(EntityReference reference)
    {
        m_Commands.emplace_back(
            [reference](
                Registry& registry,
                std::vector<Entity>& resolved)
            {
                registry.Destroy(
                    Resolve(reference, resolved));
            });
    }

    template<
        typename Component,
        typename... Arguments>
    void Emplace(
        EntityReference reference,
        Arguments&&... arguments)
    {
        // Store arguments inside the command.
        // decay_t makes the command own its payload.
        auto values =
            std::tuple<
                std::decay_t<Arguments>...
            >(
                std::forward<Arguments>(
                    arguments)...);

        m_Commands.emplace_back(
            [
                reference,
                values = std::move(values)
            ](
                Registry& registry,
                std::vector<Entity>& resolved
            ) mutable
            {
                Entity entity =
                    Resolve(reference, resolved);

                std::apply(
                    [&](auto&&... unpacked)
                    {
                        registry.Emplace<Component>(
                            entity,
                            std::forward<
                                decltype(unpacked)
                            >(unpacked)...);
                    },
                    std::move(values));
            });
    }

    template<typename Component>
    void Remove(EntityReference reference)
    {
        m_Commands.emplace_back(
            [reference](
                Registry& registry,
                std::vector<Entity>& resolved)
            {
                registry.Remove<Component>(
                    Resolve(reference, resolved));
            });
    }

    void Flush(Registry& registry)
    {
        for (auto& command : m_Commands)
        {
            command(
                registry,
                m_DeferredEntities);
        }

        Clear();
    }

    void Clear()
    {
        m_Commands.clear();
        m_DeferredEntities.clear();
    }

private:
    using Command =
        std::move_only_function<
            void(
                Registry&,
                std::vector<Entity>&)>;

    std::vector<Command> m_Commands;
    std::vector<Entity> m_DeferredEntities;

    static Entity Resolve(
        const EntityReference& reference,
        const std::vector<Entity>& resolved)
    {
        if (const Entity* entity =
                std::get_if<Entity>(
                    &reference))
        {
            return *entity;
        }

        const DeferredEntity token =
            std::get<DeferredEntity>(
                reference);

        return resolved.at(token.Token);
    }
};
```

This is not the only design, but notice what it solves:

- mutations happen later,
- command payload is owned,
- move-only payloads can be supported,
- newly created entities can be referenced by later commands,
- command order is explicit.

---

**Safe gameplay usage**

Now:

```cpp
ECSCommandBuffer commands;

registry.Each<Health>(
    [&](Entity entity, Health& health)
    {
        if (health.Value <= 0)
        {
            commands.Destroy(entity);
        }
    });

// Iteration is over.
// Dense pools can now change.
commands.Flush(registry);
```

The primary component array remains stable during `Each`.

This is the correct structural-mutation model.

---

**Command order is observable**

Suppose command stream says:

```text
1 Remove Health from A
2 Destroy A
```

versus:

```text
1 Destroy A
2 Remove Health from A
```

Those are not necessarily equivalent from the API's perspective.

First ordering:

```text
Remove succeeds
Destroy succeeds
```

Second:

```text
Destroy succeeds
Remove sees dead entity and returns false
```

Therefore command buffer must define whether it:

- preserves issue order,
- canonicalizes commands,
- coalesces them,
- sorts by entity/type,
- exposes unspecified order.

For a first implementation, **preserve issue order**.

It is easiest to reason about.

---

**Command coalescing is possible later**

Suppose same system produces:

```text
Add Velocity E
Remove Velocity E
Add Velocity E
```

The final state may be simplified.

But coalescing is complicated by:

- constructors/destructors,
- events,
- observers,
- side effects,
- component values,
- replication logs.

Do not optimize command sequences until semantics are completely specified.

---

**Multithreaded command buffers**

This becomes even more powerful with jobs.

Give each worker thread:

```text
its own command buffer
```

while registry storage is readonly for structural operations.

Workers can safely iterate:

```text
Thread 0 -> commands0
Thread 1 -> commands1
Thread 2 -> commands2
Thread 3 -> commands3
```

At synchronization:

```text
merge command buffers
     ↓
apply structure changes
```

No worker needs to mutate component pool layout concurrently.

Flecs uses essentially this staging model: while systems iterate readonly world storage, add/remove/set/delete operations can be queued into command buffers, and thread-specific stages are merged at synchronization points. The reason is exactly that direct structural changes can reallocate component arrays and invalidate ongoing iteration. [Flecs](https://www.flecs.dev/flecs/Systems.html?utm_source=chatgpt.com)

---

**Deterministic multithreaded merging**

If deterministic simulation matters, do not merge thread buffers in whichever thread finishes first.

That gives scheduling-dependent command order.

Instead define something such as:

```text
system execution order
then worker index
then local command sequence
```

So global sequence is deterministic:

$$
(systemID,\ workerID,\ localSequence).
$$

This becomes important for:

- lockstep games,
- deterministic replay,
- simulation tests,
- reproducible bugs.

---

**Command-buffer exception semantics**

Suppose flush executes:

```text
command 0 succeeds
command 1 succeeds
command 2 component constructor throws
```

What happens to commands 3–10?

You must decide.

Options include:

**Fail-fast partial commit**

Commands 0–1 remain applied.

Flush throws.

Simple, but not transactional.

**Validate-then-apply**

Perform as much validation as possible before mutation.

Cannot protect against arbitrary user component constructor failures completely.

**Full transactional rollback**

Extremely complicated and expensive for arbitrary component types.

Most game ECS systems choose well-defined partial/fail-fast semantics and design component construction to avoid exceptions.

---

**The current Kairo structural journal is **not** a command buffer**

Kairo currently records:

```cpp
enum class StructuralChangeKind
{
    Created,
    Destroyed,
    ComponentAdded,
    ComponentRemoved
};
```

and:

```cpp
struct StructuralChange
{
    uint64_t Sequence;
    StructuralChangeKind Kind;
    Entity Subject;
};
```

That is a **journal of changes that already happened**.

Flow:

```text
mutation occurs
     ↓
AppendChange(...)
```

A command buffer does:

```text
request mutation
     ↓
store command
     ↓
apply mutation later
```

They solve different problems.

---

**Why both systems are useful**

Command buffer answers:

> How do I safely schedule structural changes?

Structural journal answers:

> What structural changes happened?

Journal can support:

- editor tooling,
- replication,
- incremental scene bridges,
- debugging,
- event processing,
- recording/replay.

Command buffer supports:

- safe iteration,
- staging,
- parallelism,
- deterministic structural commits.

A mature ECS can contain both.

---

**Current Kairo journal performance issue**

The journal has fixed logical capacity.

When full:

```cpp
if (m_Changes.size()
    == m_JournalCapacity)
{
    m_Changes.erase(
        m_Changes.begin());
}
```

`vector::erase(begin)` shifts every remaining event left.

For capacity $C$:

$$
O(C)
$$

per new structural event once full.

At:

$$
C=4096,
$$

each additional event can move roughly 4095 records.

Under heavy entity churn, that becomes unnecessary work.

A ring buffer is a much better representation.

---

**Ring-buffer journal**

Keep:

```text
buffer[C]
head
count
```

When full:

```text
overwrite oldest
advance head
```

Appending remains:

$$
O(1).
$$

Logical sequence numbers already exist:

```cpp
m_NextChangeSequence
```

so chronological meaning does not require physical vector ordering.

This is a concrete improvement for current KairoECS.

---

**Another current Kairo storage contract: move assignment**

Your sparse-set removal does:

```cpp
m_Components[dense] =
    std::move(m_Components[last]);
```

Therefore `Component` must be move-assignable.

README already states this.

There is an additional subtlety:

```cpp
SparseSet::Remove(...)
```

is declared:

```cpp
noexcept
```

but component move assignment is called inside it.

If a component's move assignment throws, the program invokes:

$$
std::terminate.
$$

Therefore the effective current Kairo contract is stronger than merely:

$$
MoveAssignable.
$$

For safe removal it should effectively be:

$$
\boxed{
\text{nothrow move-assignable}
}
$$

or the implementation must stop declaring removal `noexcept`.

We will go much deeper into non-trivial component lifetime in M089, but this is already an important current-code constraint.

---

**Current Kairo M085/M086 implementation map**

Here is where your actual code stands after the audit:

| Curriculum concept | Current Kairo status |
|---|---|
| Entity identity as data | Implemented |
| Entity index | Implemented |
| Generations | Implemented |
| Packed entity integer | **Not packed; stored as two `uint32_t`s** |
| Free list | Implemented with `vector<uint32_t>` |
| Recycled creation | Implemented |
| Generation increment on destruction | Implemented |
| Generation wrap defense | **Strong: retires max-generation slots** |
| Handle validation | Implemented |
| Component type identity | RTTI / `std::type_index` |
| Static numeric component IDs | Not implemented |
| Rich runtime component registry | Not implemented |
| Component pools | Implemented |
| Sparse-set storage | Implemented |
| Sparse array | Implemented |
| Dense entity array | Implemented |
| Dense component array | Implemented |
| Membership lookup | Implemented with generation validation |
| Component lookup | Implemented |
| Component insertion | Implemented with rollback |
| Swap-and-pop removal | Implemented |
| Sparse-index repair | Implemented |
| Address invalidation contract | Documented |
| Stable-address alternative | Not implemented |
| Dense iteration | Implemented |
| Smallest-pool multi-query | Implemented for 2 and 3 component types |
| Multi-pool sparse tests | Implemented |
| Component signatures | Not implemented |
| Dynamic masks | Not implemented |
| Query caching | Not implemented |
| Structural-change restrictions | Documented |
| Deferred creation | Not implemented |
| Deferred destruction | Not implemented |
| Deferred component changes | Not implemented |
| Structural command buffer | Not implemented |
| Structural journal | Implemented, but it is a different mechanism |

---

**The complete memory model**

Now combine everything.

Entity metadata:

```text
Generations:
[1][5][8][2][...]

Alive:
[1][0][1][1][...]

Free indices:
[1][...]
```

Transform sparse set:

```text
Sparse:
entity index -> dense index

[ . ][ . ][ 1 ][ . ][ 0 ][ ... ]
```

Transform dense entities:

```text
dense index
0 -> Entity{4,2}
1 -> Entity{2,8}
```

Transform dense components:

```text
dense index
0 -> Transform for {4,2}
1 -> Transform for {2,8}
```

Velocity sparse set may contain completely different entities.

The registry is not one giant object array.

It is a federation of independently packed data columns keyed by safe generational identities.

---

**Lookup pipeline from an entity to a component**

Suppose:

```cpp
registry.Get<Transform>(
    Entity{42,7});
```

Conceptually:

```text
1. Is Index != InvalidIndex?
2. Is 42 inside registry metadata?
3. Is slot 42 alive?
4. Is current generation == 7?
5. Find Pool<Transform>
6. Is sparse index 42 inside Transform sparse array?
7. dense = Sparse[42]
8. Is dense valid?
9. Does DenseEntities[dense] == {42,7}?
10. Return DenseComponents[dense]
```

Each layer protects a different invariant.

This is why a properly built ECS is far more than `unordered_map<Entity, Component>`.

---

**Destruction pipeline**

For:

```cpp
registry.Destroy(
    Entity{42,7});
```

conceptually:

```text
Validate entity
     ↓
Remove Transform{42,7}
     ↓
Remove Velocity{42,7}
     ↓
Remove any other owned components
     ↓
Alive[42] = false
     ↓
Generation[42] = 8
     ↓
FreeIndices.push(42)
```

Now:

```text
{42,7} invalid forever
```

and future:

```text
{42,8}
```

may reuse the slot safely.

---

**Query pipeline**

System:

```text
Transform + Velocity + Health
```

Pool counts:

```text
Transform 100000
Velocity   40000
Health      2000
```

Choose Health as driver.

For each:

```text
Health dense entry
     ↓
entity
     ↓
Transform sparse membership
     ↓
Velocity sparse membership
     ↓
if both exist:
    obtain dense component references
     ↓
execute system
```

Thus runtime is proportional primarily to the rarest component.

That is the algorithmic strength of sparse-set joins.

---

**Why Kairo's current registry is genuinely data-oriented**

"Data-oriented" does not simply mean:

> we use vectors.

It means organization follows the operations that dominate runtime.

Your registry does several things correctly:

- component values of one type are contiguous,
- entity IDs are tiny,
- membership uses direct indexing,
- iteration is dense,
- queries choose the rarest pool,
- entity reuse does not compromise safety,
- component storage is independent of scene hierarchy.

Those are actual data-oriented decisions.

---

**What it still lacks before becoming a stronger production ECS**

The highest-value next additions from this pass are:

```text
1. Dense numeric runtime component IDs
2. Persistent schema IDs kept separate from runtime IDs
3. Paged sparse storage if entity index extent becomes large
4. Explicit storage invalidation policy
5. Component/query structural version counters
6. Generic variadic sparse-set queries
7. Deferred structural command buffer
8. Thread-local command staging
9. Ring-buffer structural journal
10. Stronger component move/noexcept constraints
```

I would prioritize **command buffering** before signatures.

Why?

Because signatures mostly optimize metadata/query organization.

Command buffering fixes a correctness and scalability boundary:

$$
\boxed{
\text{systems currently cannot safely perform arbitrary structural changes during iteration}.
}
$$

That limitation will matter more and more when parallel scheduling arrives.

---

**A generic variadic query direction**

Current Kairo manually implements:

```cpp
Each<A>
Each<A,B>
Each<A,B,C>
```

This becomes unmanageable for:

```text
4 components
5 components
6 components
...
```

A future query abstraction should accept:

```cpp
registry.Each<
    Transform,
    Velocity,
    Health,
    PhysicsBody
>(callback);
```

Internally:

1. collect pool pointers,
2. determine smallest pool,
3. iterate that pool,
4. test all other pools,
5. invoke callback in template argument order.

The challenge is preserving typed references while the driver pool type varies.

This is where tuples, compile-time indices and fold expressions become useful.

---

**Generic membership fold**

Conceptually:

```cpp
bool matches =
    (pool<ComponentTypes>()
         .Contains(entity)
     && ...);
```

In a real implementation you avoid rechecking the driver's membership.

But the fold-expression concept is:

$$
\bigwedge_{T\in Q}
Contains_T(E).
$$

Then fetch:

```cpp
callback(
    entity,
    *pool<ComponentTypes>()
         .TryGet(entity)...
);
```

The compile-time type list provides callback ordering.

The runtime pool-size comparison chooses the driver.

This gives both genericity and sparse-set efficiency.

---

**Why query caching will eventually become more important**

Today:

```text
Each<Transform,Velocity>
```

does two `type_index` hash lookups to retrieve pools each call.

If hundreds of systems invoke similar queries every frame, those pool lookups become unnecessary overhead.

A persistent query:

```cpp
auto moving =
    registry.Query<
        Transform,
        Velocity>();
```

could cache storage pointers once.

Then per frame:

```cpp
moving.Each(...);
```

does only join work.

This is the natural next step without yet requiring archetypes.

---

**The ownership rule you should memorize**

The ECS owns components.

Systems borrow them.

So:

$$
\boxed{
Registry
\rightarrow
\text{owns entity metadata and component storage}
}
$$

while:

$$
\boxed{
System
\rightarrow
\text{temporarily borrows component references}
}
$$

and:

$$
\boxed{
Entity handle
\rightarrow
\text{stable logical reference}
}
$$

but:

$$
\boxed{
Component*
\rightarrow
\text{potentially unstable physical reference}.
}
$$

This one mental model prevents a huge class of engine bugs.

---

**Identity versus location**

This also connects to everything we learned earlier.

A BVH node index can change if hierarchy rebuilt.

A component's dense index can change if another component is removed.

A vector address can change if reallocated.

Therefore:

$$
\text{physical storage location}
$$

should usually not be identity.

Instead:

$$
\text{stable logical handle}
\rightarrow
\text{current storage location}.
$$

For ECS:

$$
Entity(Index,Generation)
\rightarrow
Sparse[Index]
\rightarrow
DenseIndex
\rightarrow
Component.
$$

That extra indirection buys storage freedom.

---

**Why this architecture scales better than pointers between objects**

Imagine one million entities.

If gameplay systems store raw pointers between components, then any compaction/migration invalidates relationships.

With entity handles:

```cpp
struct Target
{
    Entity TargetEntity;
};
```

the target remains logical.

On use:

```cpp
if (registry.IsAlive(target.TargetEntity))
{
    ...
}
```

The target entity's components may have moved ten times.

Relationship remains valid because it does not encode a component address.

This becomes essential in archetype migration next pass.

---

**Entity handles are not globally persistent IDs**

Another critical distinction.

Your Kairo Entity is documented as:

> process-local.

That is right.

Do not serialize:

```text
Index = 27
Generation = 4
```

and assume it identifies the same game object after loading tomorrow.

Index allocation depends on runtime lifecycle.

For persistent scene identity, use another ID:

```text
UUID
SceneEntityID
AssetObjectID
NetworkID
```

Then loading constructs fresh ECS entities and maps persistent IDs to runtime handles.

This separation will matter heavily in scene serialization later.

---

**Sparse-set memory complexity**

For one pool:

Let:

$$
E_{\max}
=
\text{largest entity index extent},
$$

$$
N_C
=
\text{number of entities owning component C}.
$$

With Kairo's flat sparse array:

$$
M_C
\approx
4E_{\max}
+
sizeof(Entity)N_C
+
sizeof(Component)N_C.
$$

The sparse portion depends on **entity extent**, not component population.

This is why rare components can still consume large sparse metadata if entity IDs grow very high.

Paged sparse arrays change this toward allocated-page occupancy.

---

**Time complexity summary**

For current Kairo sparse-set registry, ignoring allocation effects:

$$
IsAlive(E)=O(1)
$$

$$
Has<T>(E)=O(1)
$$

$$
Get<T>(E)=O(1)
$$

$$
Emplace<T>(E)=O(1)
$$

amortized,

$$
Remove<T>(E)=O(1)
$$

assuming movable component,

$$
Each<T>=O(|T|)
$$

and for query $T_1,\dots,T_k$:

$$
Each<T_1,\dots,T_k>
=
O(
\min_i|T_i|
\cdot(k-1)
).
$$

Entity destruction is currently:

$$
O(T)
$$

over number of registered component types because every pool is checked.

---

**Reconstruction challenge — build a generational registry from blank memory**

You should be able to derive the minimum entity allocator yourself.

State:

```cpp
vector<uint32_t> generations;
vector<bool> alive;
vector<uint32_t> free;
```

Create:

```text
if free nonempty:
    pop index
else:
    append metadata

alive[index] = true

return {index, generation[index]}
```

Destroy:

```text
validate
remove components
alive[index] = false

if generation not max:
    generation++
    free.push(index)
else:
    retire slot
```

Validate:

$$
IndexValid
\land
Bounds
\land
Alive
\land
GenerationMatch.
$$

If you cannot derive this without remembering code, revisit M085.

---

**Reconstruction challenge — sparse set**

You should be able to draw this from memory:

```text
Sparse
entity index → dense index

DenseEntities
dense index → entity

DenseComponents
dense index → component
```

Then state invariant:

$$
Sparse[DenseEntities[d].Index]=d.
$$

Membership:

$$
d=Sparse[E.Index]
$$

and:

$$
d\neq Missing
\land
d<DenseSize
\land
DenseEntities[d]=E.
$$

Removal:

```text
move last into hole
repair sparse[lastEntity.Index]
pop dense arrays
Sparse[removed.Index] = Missing
```

That is the entire sparse-set mechanism.

---

**Reconstruction challenge — invalidation**

You should know immediately:

**Component insertion may invalidate**

$$
\text{all component references}
$$

if vector reallocates.

**Swap-remove invalidates**

the removed component,

the old address of the moved last component,

and changes the logical entity represented at the removed slot address.

Therefore:

$$
\boxed{
\text{never promise pointer stability unless storage policy explicitly provides it}.
}
$$

---

**Reconstruction challenge — command buffering**

If a system wants:

```cpp
for every Health:
    if health <= 0:
        destroy entity
```

do not mutate the primary pool.

Instead:

```text
iteration
     ↓
record Destroy commands
     ↓
finish iteration
     ↓
flush command buffer
```

For deferred creation:

```text
temporary token
     ↓
Create command
     ↓
later AddComponent commands reference token
     ↓
flush resolves token → real Entity
```

That is Part 1431–1434 in one coherent mechanism.

---

**Exact coverage — M085 + M086**

| Part | Required topic | Covered |
|---:|---|---|
| 1401 | Entity identity as data rather than object hierarchy | ✓ |
| 1402 | Entity index representation | ✓ |
| 1403 | Generation counters | ✓ |
| 1404 | Packing index/generation into one integer | ✓ |
| 1405 | Entity free lists | ✓ |
| 1406 | Creation from free-list slot | ✓ |
| 1407 | Destruction and generation increments | ✓ |
| 1408 | Validating entity handle | ✓ |
| 1409 | Component type identity | ✓ |
| 1410 | Static component type IDs | ✓ |
| 1411 | Runtime component type registries | ✓ |
| 1412 | Component pools | ✓ |
| 1413 | Sparse-set concept | ✓ |
| 1414 | Sparse index array | ✓ |
| 1415 | Dense entity array | ✓ |
| 1416 | Dense component array | ✓ |
| 1417 | Sparse-set membership lookup | ✓ |
| 1418 | Sparse-set component lookup | ✓ |
| 1419 | Sparse-set component insertion | ✓ |
| 1420 | Swap-and-pop removal | ✓ |
| 1421 | Sparse-index repair after removal | ✓ |
| 1422 | Component-address invalidation | ✓ |
| 1423 | Stable-storage alternatives | ✓ |
| 1424 | Dense component iteration | ✓ |
| 1425 | Smallest-pool iteration | ✓ |
| 1426 | Multi-pool membership tests | ✓ |
| 1427 | Component signatures / masks | ✓ |
| 1428 | Dynamic bitsets | ✓ |
| 1429 | Query caching | ✓ |
| 1430 | Structural changes / iterator invalidation | ✓ |
| 1431 | Deferred entity creation | ✓ |
| 1432 | Deferred entity destruction | ✓ |
| 1433 | Deferred component add/remove | ✓ |
| 1434 | ECS command buffers | ✓ |


The important result is that you should no longer think of ECS as “entities + components + systems.”

You should now see this chain:

$$
\boxed{
\text{generational identity}
\rightarrow
\text{slot reuse}
\rightarrow
\text{typed pools}
\rightarrow
\text{sparse indirection}
\rightarrow
\text{dense storage}
\rightarrow
\text{constant-time membership}
\rightarrow
\text{smallest-pool joins}
\rightarrow
\text{invalidation rules}
\rightarrow
\text{deferred structural mutation}.
}
$$

And this sets up **Pass 11 — M087 + M088, Parts 1435–1474**, where the architecture becomes substantially more advanced: **archetype signatures, chunks, entity migration between archetypes, chunk capacity/alignment, columnar SoA storage, sparse-set vs archetype ECS, hot/cold splitting, tags, resources, relationships, scene hierarchies, transform dirty propagation, concrete render/physics/light/camera/script/input components, entity↔subsystem ID bridges, events, tick models, dependency graphs, parallel system execution, synchronization hazards, false sharing and prefetch behavior.**


### Archetype ECS, Chunks, Migration, AoS/SoA, Hot–Cold Layouts, Tags, Resources, Relationships, Transform Hierarchies, Engine Components, Subsystem ID Bridges, Events, Tick Models, System DAGs, Parallel ECS Scheduling, Hazards, False Sharing and Prefetch

**Coverage:** M087 owns **Parts 1435–1451** and M088 owns **Parts 1452–1474**, with every atom in both ranges required. Graphics_Engine_Mastery_Integra…

Pass 10 established a sparse-set ECS:

$$
Entity(Index,Generation)
\rightarrow
Sparse
\rightarrow
DenseIndex
\rightarrow
Component.
$$

That is already a powerful runtime model. Pass 11 asks a different question:

$$
\boxed{
\text{What if the dominant workload is not random component lookup, but processing huge groups of identically shaped entities?}
}
$$

That is where **archetypes and chunks** appear.

Then the second half asks the larger engine question:

$$
\boxed{
\text{How do rendering, physics, input, scripts, events and jobs actually attach to this ECS without destroying its data-oriented properties?}
}
$$

This is one of the most architecturally important passes in the entire curriculum.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/14 - Application clocks and stable input identity|Previous chapter]] · [[Chapter Edition/03 - Game Engine/16 - Archetypes, chunks, relationships, and transforms|Next chapter]]
