# 18 — ECS contracts, diagnostics, and GPU handoff

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/17 - ECS performance through cache and scheduling|Previous chapter]] · [[Chapter Edition/03 - Game Engine/19 - GPU work and reusable Metal frames|Next chapter]]

An ECS needs evidence for its identity, query, and memory promises before rendering can consume its state. This chapter closes those defenses and transitions from world storage to explicit GPU work and resource lifetimes.

**By the end:** Audit ECS invariants with diagnostics and tests, then explain how entity data becomes render work with explicit ownership.

**Recall question:** Which ECS invariant would you inspect first when a rendered object is stale, duplicated, or associated with the wrong entity?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 38365–41220; source-span SHA256 2ecbedf31698c5c218155465870cd46220538733edb6ad7fa0a48b4b95f8bbad. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### DIAGNOSTICS, MEMORY, COMPLEXITY, API CONTRACTS AND COMPLETE ECS DEFENSE

---

**ECS debugging views**

A production ECS needs visibility into itself.

You want to inspect an entity and see:

```text
Entity {42,7}
Alive: yes

Components:
Transform
Velocity
Health
Renderable
```

For a sparse set:

```text
Transform:
Sparse[42] = 831
DenseEntity[831] = {42,7}
```

For an archetype ECS:

```text
Archetype #17
Chunk #4
Row 81
```

Without this, storage bugs are miserable to diagnose.

---

**Entity inspector**

A debug API could provide:

```cpp
struct EntityDebugInfo
{
    Entity Handle;
    bool Alive;

    std::vector<ComponentTypeInfo>
        Components;
};
```

But generic enumeration requires a component type registry.

Current Kairo's type-erased `StorageBase` can be extended with:

```cpp
virtual std::string_view TypeName() const = 0;
virtual bool Contains(Entity) const = 0;
virtual std::size_t Size() const = 0;
```

Then debug tooling can inspect without knowing T.

---

**Storage invariant checker**

Debug builds should expose:

```cpp
bool SparseSet<T>::Validate() const;
```

Check:

$$
|Entities|=|Components|.
$$

For every dense index $d$:

$$
Sparse[Entities[d].Index]=d.
$$

For every sparse entry $s\neq Missing$:

$$
s<Entities.size().
$$

No duplicate entity indices.

Generations match.

Such validators catch corruption close to its source.

---

**Registry invariant checker**

Registry-wide:

$$
generations.size=alive.size.
$$

For each free index:

```text
in bounds
not alive
generation not retired
```

No duplicate free indices.

For every live component owner:

```text
registry.IsAlive(owner) == true.
```

For destroyed entity:

```text
no storage contains exact stale handle.
```

Call this under debug/test builds.

---

**Component-pool statistics**

For each component pool record:

```text
count
capacity
sparse extent
dense bytes
sparse bytes
estimated allocated bytes
```

For example:

```text
Transform:
  count = 950,000
  dense capacity = 1,000,000
  sparse size = 1,000,000
```

This turns memory behavior into data.

---

**Useful density metric**

Define:

$$
Density_C=
\frac{ComponentCount_C}
{SparseExtent_C}.
$$

If:

$$
Count=100
$$

but:

$$
SparseExtent=1,000,000,
$$

then:

$$
Density=0.01\%.
$$

A flat sparse array is wasting memory for this component.

That is direct evidence to consider paged sparse storage.

---

**Dense utilization**

$$
Utilization=
\frac{size}
{capacity}.
$$

A dense vector with:

$$
size=100,000
$$

and:

$$
capacity=1,000,000
$$

uses only 10% of reserved component slots.

Perhaps reserve policy is overly aggressive.

Expose these numbers.

---

**Structural statistics**

Useful runtime counters:

```text
entities created/frame
entities destroyed/frame
components added/frame
components removed/frame
generation reuse count
query visits
journal events
```

A sudden spike in structural churn may explain frame-time regression.

Data-oriented architecture should be observable.

---

**Detecting dangling external component pointers**

This is one of the hardest practical problems.

Suppose:

```cpp
Transform* cached =
    registry.TryGet<Transform>(entity);
```

Later component pool reallocates.

`cached` points to freed old storage.

How can engine detect misuse?

---

**Debug storage epochs**

Give each component pool a structural epoch:

```cpp
uint64_t MutationEpoch;
```

Increment whenever an operation may invalidate addresses:

```text
reallocation
swap-remove
compaction
migration
```

A debug borrow stores:

```cpp
template<typename T>
struct ComponentBorrow
{
    T* Pointer;
    const PoolBase* Owner;
    uint64_t Epoch;
};
```

Dereference checks:

$$
Epoch=Owner.CurrentEpoch.
$$

If not:

```text
stale component borrow detected.
```

---

**Debug proxy handle**

Instead of exposing raw T* in debug mode:

```cpp
template<typename T>
class CheckedComponentRef
{
public:
    T& Get()
    {
        assert(
            m_Epoch ==
            m_Pool->MutationEpoch());

        return *m_Pointer;
    }
};
```

Release builds can use direct references if overhead is unacceptable.

Debug builds gain excellent diagnostics.

---

**AddressSanitizer helps, but not enough**

ASan can detect:

```text
use-after-free
```

when vector reallocation frees old storage.

But swap-remove can produce a nastier case:

```text
pointer still points to allocated memory
but now another entity occupies it.
```

ASan sees valid memory.

Semantically it is wrong.

Storage epochs/ownership checks can detect what memory sanitizers cannot.

---

**Poisoning removed storage**

Debug allocators can overwrite removed slots:

```text
0xDD
0xDEADBEEF
```

but if swap-removal immediately fills that slot with another component, poisoning does not help.

The stale pointer now aliases a live but wrong object.

That is why identity-aware debug handles are superior.

---

**Memory budgeting for millions of entities**

This is where hand-wavy ECS design ends.

Let's budget an example.

One million entity slots.

Kairo entity metadata currently roughly includes:

$$
Generation:4\text{ bytes}
$$

plus `vector<bool>` alive bitmap approximately:

$$
1\text{ bit/entity}.
$$

Generation array:

$$
1,000,000\times4
=
4,000,000\text{ bytes}
\approx3.81\text{ MiB}.
$$

Alive bitmap:

$$
1,000,000/8
=
125,000\text{ bytes}
\approx0.119\text{ MiB}.
$$

Ignoring capacity/allocator metadata:

$$
\approx3.93\text{ MiB}
$$

for core lifetime metadata.

---

**Free-list worst case**

If all one million slots become free:

$$
1,000,000\times4
=
3.81\text{ MiB}.
$$

So entity allocator metadata in a pathological mostly-dead world might approach roughly:

$$
7.7\text{ MiB}
$$

before vector overhead.

Not bad.

Components dominate.

---

**Transform pool example**

Assume:

```cpp
struct Transform
{
    // hypothetical 40 bytes
};
```

Kairo dense storage stores:

```text
Component array
Entity array
Sparse array
```

For one million Transform owners:

Component data:

$$
40,000,000
\approx38.15\text{ MiB}.
$$

Dense Entity array; Kairo Entity is 8 bytes:

$$
8,000,000
\approx7.63\text{ MiB}.
$$

Sparse `uint32_t`:

$$
4,000,000
\approx3.81\text{ MiB}.
$$

Total approximately:

$$
49.6\text{ MiB}.
$$

One component pool.

---

**Sparse component example**

Suppose only 1,000 entities have BossAI, but highest entity index is one million.

Component size:

$$
256\text{ bytes}.
$$

Dense components:

$$
256,000
\approx0.244\text{ MiB}.
$$

Dense entities:

$$
8,000
\approx0.008\text{ MiB}.
$$

Flat sparse array:

$$
4,000,000
\approx3.81\text{ MiB}.
$$

Sparse metadata is over 15× larger than actual component payload.

This is precisely the use case where paged sparse storage matters.

---

**Memory budget formula for flat sparse set**

For component C:

$$
\boxed{
M_C
\approx
E_{\max}\cdot sizeof(SparseIndex)
+
N_C\cdot sizeof(Entity)
+
N_C\cdot sizeof(C)
+
CapacityWaste.
}
$$

Where:

$$
E_{\max}
$$

is sparse extent, approximately largest entity index + 1.

And:

$$
N_C
$$

is number of owners.

This formula should be in your head.

---

**One million entities with ten common components**

If ten common components each pay a 4 MiB flat sparse array:

$$
10\times4
=
40\text{ MiB}
$$

just for sparse maps.

Often acceptable on desktop.

On constrained platforms or with 200 component types:

$$
200\times4
=
800\text{ MiB}
$$

if every pool expands to the million-slot extent.

That becomes disastrous.

Paged sparse arrays avoid paying full extent for untouched pages.

---

**Archetype memory overhead**

Archetypes eliminate per-component sparse arrays inside an archetype.

Entity location table might cost:

```text
archetype/chunk pointer/index
row
generation/etc.
```

and chunks contain packed component columns.

But archetypes can waste memory through partially filled chunks.

So compare:

$$
SparseMetadataWaste
$$

versus:

$$
ChunkFragmentationWaste.
$$

Again: workload.

---

**Sparse-set complexity**

For flat sparse set:

Membership:

$$
Contains(E)=O(1).
$$

Lookup:

$$
Get(E)=O(1).
$$

Insertion:

$$
O(1)
$$

amortized, because dense vector push is amortized constant and sparse resize may occasionally cost more.

Removal:

$$
O(1)
$$

with swap-remove, assuming move operation cost is treated as constant with respect to entity count.

Iteration:

$$
O(N_C).
$$

---

**Multi-component sparse query complexity**

For query requiring $k$ components with smallest pool size $m$:

$$
\boxed{
O(m(k-1)).
}
$$

For fixed $k$:

$$
O(m).
$$

However big-O hides memory latency.

Each membership check may access:

```text
another sparse array
another dense entity array
another dense component slot
```

so cache behavior matters enormously.

---

**Entity destruction complexity**

Current Kairo destruction checks every registered storage:

$$
O(T)
$$

where $T$ is number of component types/pools.

If only $c$ components belong to the entity:

$$
c\ll T,
$$

a per-entity signature/list could reduce removal work toward:

$$
O(c).
$$

This is an example where theoretical complexity reveals a scaling opportunity.

---

**Sparse array growth complexity**

Suppose component first appears on entity index 999,999.

`EnsureSparseIndex` may resize from tiny size to one million.

That single insertion costs:

$$
O(E_{\max})
$$

for initialization of new sparse entries.

Thus insertion is not strict constant-time in every operation.

It is amortized/workload dependent.

This matters for rare components added late to high-index entities.

Paged sparse storage avoids giant one-shot expansions.

---

**Archetype migration complexity**

Entity moving from archetype A to B must move all common components.

Let:

$$
C=
\text{number of shared components}.
$$

Let component move costs be:

$$
M_i.
$$

Migration cost roughly:

$$
\boxed{
T_{\text{migrate}}
=
T_{\text{lookup}}
+
T_{\text{allocate row}}
+
\sum_{i=1}^{C}M_i
+
T_{\text{construct/remove changed}}
+
T_{\text{source swap repair}}.
}
$$

If component moves are constant:

$$
O(C).
$$

But C++ nontrivial moves can hide allocations or external operations.

---

**Structural cost depends on bytes moved too**

Two archetypes each have five components.

Case A components total:

$$
64\text{ bytes}.
$$

Case B:

$$
4\text{ KiB}.
$$

Both are:

$$
O(5)
$$

components, yet real migration cost differs dramatically.

A more realistic performance model tracks:

$$
BytesMoved
$$

and component-specific move complexity.

---

**Archetype lookup complexity**

Destination archetype lookup using hash map:

$$
O(1)
$$

average.

But building/hash-comparing dynamic signatures may cost:

$$
O(C).
$$

Transition caching from Pass 11 changes repeated:

```text
A + Health
```

from signature rebuilding to a cached edge.

Then transition lookup can approach:

$$
O(1).
$$

This is why archetype transition graphs matter.

---

**Designing ECS APIs around ownership/invalidation**

This may be the most important M090 topic.

An API must make it hard to accidentally depend on unstable physical storage.

Bad API:

```cpp
Transform* GetPermanentTransform(Entity e);
```

The word "permanent" is a lie.

Better:

```cpp
Transform& Get(Entity e);
```

with documented lifetime:

> valid until the next structural operation that can invalidate this pool.

Even better for long-lived references:

```text
store Entity
resolve component when needed.
```

---

**Invalidation contract should be explicit**

For current sparse-set pool, document something like:

**`Emplace<T>`**

May reallocate T's dense arrays.

Therefore may invalidate all:

```text
T*
T&
spans/iterators into pool T.
```

**`Remove<T>`**

May move the last T into another slot.

Therefore invalidates:

```text
reference to removed T
reference/pointer to moved last T's old address
semantic association at removed slot address.
```

**Other component pools**

Adding Velocity should not invalidate Transform pointers if storages are truly independent.

That distinction is useful.

---

**Registry destruction invalidation**

Destroy entity may remove components from many pools.

Therefore:

```cpp
registry.Destroy(e);
```

potentially invalidates references in every pool from which e owns a component, because each removal may swap-move a last element.

This is wider than:

```cpp
Remove<Transform>(e).
```

Users should know it.

---

**Query callback ownership contract**

A safe contract:

```text
callback borrows component references
for the duration of that callback invocation.

do not store them beyond structural synchronization boundary.
```

Structural changes should be queued rather than executed directly.

This is far clearer than relying on users to reverse-engineer vector behavior.

---

**Const query APIs**

Provide separate:

```cpp
EachRead<T>
```

or const-registry iteration returning:

```cpp
const T&.
```

That communicates both C++ mutability and scheduler access intent.

Later:

```text
Read<T>
Write<U>
```

can feed hazard detection.

API semantics and scheduler semantics should reinforce each other.

---

**Entity handle validation at subsystem boundaries**

Do not assume an Entity received from another subsystem is live.

Before mutation:

```cpp
if (!registry.IsAlive(entity))
    ...
```

especially for:

- delayed events,
- asynchronous jobs,
- network packets,
- GPU readback,
- timers,
- script callbacks.

Generational handles make validation cheap.

Use it.

---

**Testing generation reuse bugs**

Kairo already has an excellent basic test:

```text
create first
destroy first
assert old not alive
create recycled
assert same index
assert different generation
assert recycled alive.
```

That is exactly the first required generation test.

But production testing should go deeper.

---

**Stale mutation test**

Create:

$$
E_1=(i,g).
$$

Destroy.

Create:

$$
E_2=(i,g+1).
$$

Then verify all operations using $E_1$ fail:

```text
Emplace
Get
Remove
Destroy
Has
```

while E2 remains untouched.

This proves stale handles cannot mutate replacement state.

Your current tests already cover part of this.

---

**Component stale-generation test**

Your direct SparseSet test is especially good.

It inserts:

```text
{42,1}
```

then attempts:

```text
{42,2}
```

without first removing old generation.

The pool rejects it.

This proves sparse mapping cannot silently create two generations sharing one sparse index.

That is a very valuable invariant test.

---

**Rapid reuse stress test**

Do:

```cpp
Entity previous {};

for (std::size_t i = 0;
     i < 10'000'000;
     ++i)
{
    Entity e = registry.Create();

    if (previous.IsValid())
    {
        assert(!registry.IsAlive(previous));
    }

    registry.Destroy(e);
    previous = e;
}
```

You expect:

- slot reuse,
- increasing generations,
- no stale resurrection,
- no component leakage,
- stable entity count.

---

**Max-generation retirement test**

Because Kairo retires a slot at:

$$
UINT32_MAX,
$$

this needs a direct test.

Do not loop four billion times.

Expose test-only state or construct a Registry fixture whose generation is:

$$
UINT32_MAX-1.
$$

Destroy the penultimate-generation entity.

Reuse once at max generation.

Destroy again.

Verify slot is **not pushed back onto free list**.

Then next Create must allocate another index.

That proves no wraparound alias.

---

**Randomized property testing**

Generate random sequences:

```text
Create
Destroy
Add<T>
Remove<T>
Get<T>
Create
...
```

Maintain a simple trusted reference model.

After every operation compare:

```text
alive set
component ownership
component values.
```

Also run:

```text
ValidateRegistryInvariants().
```

Randomized state-machine testing is exceptionally effective for ECS bugs.

---

**Benchmarking ECS versus object-oriented layout**

Current Kairo benchmark measures:

```text
Position + Velocity + Active
```

with Active on 1/10 entities.

It runs 120 iterations and emits JSON:

```text
kairo.ecs.benchmark.v1
```

including visits/sec.

That is useful.

But it does **not yet answer Part 1499 completely** because there is no OOP baseline in that benchmark.

---

**Fair OOP baseline**

Do not sabotage OOP with deliberately awful allocations like:

```cpp
vector<unique_ptr<GameObject>>
```

and then claim ECS is faster.

Use realistic alternatives.

For example:

```cpp
struct Object
{
    Position PositionData;
    Velocity VelocityData;
    bool Active;
};
```

stored contiguously:

```cpp
std::vector<Object>.
```

Then benchmark the same semantic operation.

---

**AoS baseline**

```cpp
struct Object
{
    Position P;
    Velocity V;
    bool Active;
};

for (Object& object : objects)
{
    if (!object.Active)
        continue;

    object.P.x += object.V.x;
    object.P.y += object.V.y;
    object.P.z += object.V.z;
}
```

Compare to ECS:

```cpp
registry.Each<
    Position,
    Velocity,
    Active>(...);
```

Now the comparison is meaningful.

---

**What do you actually expect?**

If every Object has Position, Velocity and Active and all are updated together, AoS may perform extremely well.

ECS does not magically win.

If objects contain many unrelated fields:

```text
Name
AI
Renderer
Audio
Inventory
Script
Debug
```

and movement uses only Position+Velocity, ECS/SoA begins to gain.

If Active exists on only 10% of entities, a sparse-set query scanning the 10% Active pool may avoid testing 90% entirely.

Now storage structure changes the amount of work.

---

**Benchmark several populations**

At minimum compare:

$$
N=
10^3,\ 10^4,\ 10^5,\ 10^6.
$$

And component density:

$$
1\%,10\%,50\%,100\%.
$$

Also test:

```text
single-component iteration
two-component join
three-component join
random Get
component add/remove churn
entity create/destroy
```

ECS performance is multidimensional.

---

**Measure latency and throughput**

Iteration:

$$
entities/sec
$$

is useful.

Also measure:

$$
ns/entity.
$$

For structural mutations:

$$
ns/add
$$

$$
ns/remove
$$

$$
ns/create
$$

$$
ns/destroy.
$$

For queries:

$$
ns/query
$$

and total matched entities.

Without operation-specific metrics, one "ECS benchmark score" is not very meaningful.

---

**Warm-up**

First iteration may include:

- page faults,
- branch predictor cold state,
- lazy allocations,
- instruction-cache warming.

Run warm-up iterations before timing.

Then repeat benchmark many times.

Report:

```text
median
p95
min/max
```

rather than one number.

---

**Avoid dead-code elimination**

Compiler may remove work whose result is never observed.

Current Kairo benchmark increments:

```cpp
visits
```

and prints output, and modifies component positions, so there is observable activity.

For more aggressive optimized benchmarks, compute a checksum at the end.

---

**Benchmark build configuration**

Never compare:

```text
ECS Release -O3
```

against:

```text
OOP Debug.
```

Record:

```text
compiler
compiler version
optimization flags
CPU
OS
entity count
commit hash
```

Kairo wisely emits schema-structured benchmark JSON instead of putting hardware-specific bragging numbers into the README.

That is good benchmark hygiene.

---

**Implementing a generational sparse-set ECS from a blank file**

Now we rebuild the whole thing.

No memorized library.

No magical containers beyond vectors/maps.

We need to defend every data structure.

Start with entity identity.

---

**Entity handle**

```cpp
#include <cstdint>
#include <limits>

struct Entity final
{
    static constexpr std::uint32_t InvalidIndex =
        std::numeric_limits<std::uint32_t>::max();

    std::uint32_t Index = InvalidIndex;
    std::uint32_t Generation = 0;

    [[nodiscard]]
    constexpr bool IsNull() const noexcept
    {
        return Index == InvalidIndex;
    }

    friend constexpr bool operator==(
        Entity,
        Entity) noexcept = default;
};
```

Why two fields?

Index gives:

$$
O(1)
$$

slot addressing.

Generation prevents ABA on index reuse.

Why 32 bits each?

Simple, large space, and generation wrap becomes astronomically remote; Kairo goes further and retires max-generation slots.

---

**Sparse set**

```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

template<typename T>
class SparseSet
{
public:
    static constexpr std::uint32_t Missing =
        std::numeric_limits<std::uint32_t>::max();

    [[nodiscard]]
    bool Contains(Entity entity) const noexcept
    {
        if (entity.IsNull() ||
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

    template<typename... Args>
    T& Emplace(
        Entity entity,
        Args&&... args)
    {
        EnsureSparse(entity.Index);

        if (Contains(entity))
        {
            throw std::invalid_argument(
                "Component already exists.");
        }

        // A different generation may still occupy this
        // sparse index. Allowing another dense entry
        // would orphan the old one.
        if (m_Sparse[entity.Index] != Missing)
        {
            throw std::logic_error(
                "Sparse entity index is occupied "
                "by another generation.");
        }

        if (m_Entities.size() >=
            static_cast<std::size_t>(Missing))
        {
            throw std::overflow_error(
                "SparseSet exhausted 32-bit dense indices.");
        }

        const auto dense =
            static_cast<std::uint32_t>(
                m_Entities.size());

        // Add entity first.
        m_Entities.push_back(entity);

        try
        {
            // Construct component.
            m_Components.emplace_back(
                std::forward<Args>(args)...);
        }
        catch (...)
        {
            // Restore parallel-array invariant.
            m_Entities.pop_back();
            throw;
        }

        // Publish sparse mapping only after both
        // dense entries exist successfully.
        m_Sparse[entity.Index] = dense;

        return m_Components.back();
    }

    bool Remove(Entity entity)
        noexcept(
            std::is_nothrow_move_assignable_v<T>)
    {
        if (!Contains(entity))
        {
            return false;
        }

        const std::uint32_t dense =
            m_Sparse[entity.Index];

        const std::uint32_t last =
            static_cast<std::uint32_t>(
                m_Entities.size() - 1);

        if (dense != last)
        {
            // Move final entity/component into hole.
            m_Entities[dense] =
                std::move(m_Entities[last]);

            m_Components[dense] =
                std::move(m_Components[last]);

            // Repair moved entity's reverse map.
            m_Sparse[
                m_Entities[dense].Index
            ] = dense;
        }

        m_Entities.pop_back();
        m_Components.pop_back();

        m_Sparse[entity.Index] = Missing;

        return true;
    }

    T* TryGet(Entity entity) noexcept
    {
        if (!Contains(entity))
        {
            return nullptr;
        }

        return &m_Components[
            m_Sparse[entity.Index]];
    }

    const T* TryGet(
        Entity entity) const noexcept
    {
        if (!Contains(entity))
        {
            return nullptr;
        }

        return &m_Components[
            m_Sparse[entity.Index]];
    }

    T& Get(Entity entity)
    {
        if (T* result = TryGet(entity))
        {
            return *result;
        }

        throw std::out_of_range(
            "Entity does not own component.");
    }

    [[nodiscard]]
    std::size_t Size() const noexcept
    {
        return m_Components.size();
    }

    [[nodiscard]]
    const std::vector<Entity>&
    Entities() const noexcept
    {
        return m_Entities;
    }

    [[nodiscard]]
    std::vector<T>&
    Components() noexcept
    {
        return m_Components;
    }

private:
    std::vector<std::uint32_t> m_Sparse;
    std::vector<Entity> m_Entities;
    std::vector<T> m_Components;

    void EnsureSparse(
        std::uint32_t index)
    {
        if (index >= m_Sparse.size())
        {
            m_Sparse.resize(
                static_cast<std::size_t>(
                    index) + 1,
                Missing);
        }
    }
};
```

This is the fundamental storage engine.

---

**Why three arrays?**

`m_Sparse` provides:

$$
EntityIndex\rightarrow DenseIndex.
$$

`m_Entities` provides:

$$
DenseIndex\rightarrow Entity.
$$

`m_Components` provides:

$$
DenseIndex\rightarrow Component.
$$

Together:

$$
Entity
\leftrightarrow
DenseRow
\leftrightarrow
Component.
$$

Without dense entity array, swap repair would not know which entity was moved.

Without sparse array, lookup would require search.

Without dense component array, iteration loses locality.

Every structure has a reason.

---

**Type-erased pool base**

Registry must remove components from all unknown pool types during entity destruction.

```cpp
class IComponentPool
{
public:
    virtual ~IComponentPool() = default;

    virtual bool Remove(
        Entity entity) = 0;

    virtual bool Contains(
        Entity entity) const = 0;

    virtual std::size_t Size()
        const noexcept = 0;
};
```

Typed pool:

```cpp
template<typename T>
class ComponentPool final
    : public IComponentPool
{
public:
    SparseSet<T> Storage;

    bool Remove(Entity e) override
    {
        return Storage.Remove(e);
    }

    bool Contains(
        Entity e) const override
    {
        return Storage.Contains(e);
    }

    std::size_t Size()
        const noexcept override
    {
        return Storage.Size();
    }
};
```

---

**Registry metadata**

```cpp
#include <memory>
#include <typeindex>
#include <unordered_map>

class Registry
{
private:
    std::vector<std::uint32_t>
        m_Generations;

    std::vector<bool>
        m_Alive;

    std::vector<std::uint32_t>
        m_Free;

    std::unordered_map<
        std::type_index,
        std::unique_ptr<IComponentPool>>
        m_Pools;
};
```

Defend each:

`m_Generations` is flat O(1) validation metadata. `m_Alive` distinguishes currently occupied slots. `m_Free` gives O(1) slot reuse. `m_Pools` maps C++ type identity to storage while keeping pools type-erased at registry level.

---

**Entity creation**

```cpp
Entity Create()
{
    std::uint32_t index;

    if (!m_Free.empty())
    {
        index = m_Free.back();
        m_Free.pop_back();

        assert(
            index < m_Alive.size());

        assert(
            !m_Alive[index]);

        m_Alive[index] = true;
    }
    else
    {
        if (m_Generations.size() >=
            Entity::InvalidIndex)
        {
            throw std::overflow_error(
                "Entity index space exhausted.");
        }

        index =
            static_cast<std::uint32_t>(
                m_Generations.size());

        m_Generations.push_back(1);

        try
        {
            m_Alive.push_back(true);
        }
        catch (...)
        {
            m_Generations.pop_back();
            throw;
        }
    }

    return
    {
        index,
        m_Generations[index]
    };
}
```

Notice rollback if metadata allocation fails.

---

**Handle validation**

```cpp
bool IsAlive(
    Entity entity) const noexcept
{
    return
        !entity.IsNull() &&
        entity.Index <
            m_Generations.size() &&
        m_Alive[entity.Index] &&
        m_Generations[entity.Index] ==
            entity.Generation;
}
```

This is the generation barrier.

Every externally supplied Entity must eventually be checked against it before mutation.

---

**Entity destruction**

```cpp
bool Destroy(Entity entity)
{
    if (!IsAlive(entity))
    {
        return false;
    }

    // Remove old generation's components before
    // allowing slot to become a new entity.
    for (auto& [_, pool] : m_Pools)
    {
        pool->Remove(entity);
    }

    m_Alive[entity.Index] = false;

    std::uint32_t& generation =
        m_Generations[entity.Index];

    if (generation !=
        std::numeric_limits<
            std::uint32_t>::max())
    {
        ++generation;
        m_Free.push_back(entity.Index);
    }
    else
    {
        // Retire permanently rather than wrap.
    }

    return true;
}
```

This ordering enforces:

$$
\boxed{
\text{No recycled slot retains old-generation components}.
}
$$

---

**Typed pool creation**

```cpp
template<typename T>
ComponentPool<T>& Pool()
{
    const std::type_index key =
        std::type_index(typeid(T));

    const auto found =
        m_Pools.find(key);

    if (found != m_Pools.end())
    {
        return *static_cast<
            ComponentPool<T>*>(
                found->second.get());
    }

    auto created =
        std::make_unique<
            ComponentPool<T>>();

    ComponentPool<T>* pointer =
        created.get();

    m_Pools.emplace(
        key,
        std::move(created));

    return *pointer;
}
```

This is lazy pool creation.

---

**Component insertion**

```cpp
template<typename T, typename... Args>
T& Emplace(
    Entity entity,
    Args&&... args)
{
    if (!IsAlive(entity))
    {
        throw std::invalid_argument(
            "Cannot mutate stale entity.");
    }

    return Pool<T>()
        .Storage
        .Emplace(
            entity,
            std::forward<Args>(
                args)...);
}
```

Registry validation and pool generation validation form two defensive layers.

---

**Multi-component iteration**

For two components:

```cpp
template<
    typename A,
    typename B,
    typename Function>
void Each(Function&& function)
{
    auto& a = Pool<A>().Storage;
    auto& b = Pool<B>().Storage;

    if (a.Size() <= b.Size())
    {
        const auto& entities =
            a.Entities();

        auto& values =
            a.Components();

        for (std::size_t i = 0;
             i < entities.size();
             ++i)
        {
            Entity entity = entities[i];

            if (B* second =
                    b.TryGet(entity))
            {
                std::invoke(
                    function,
                    entity,
                    values[i],
                    *second);
            }
        }
    }
    else
    {
        const auto& entities =
            b.Entities();

        auto& values =
            b.Components();

        for (std::size_t i = 0;
             i < entities.size();
             ++i)
        {
            Entity entity = entities[i];

            if (A* first =
                    a.TryGet(entity))
            {
                // Preserve caller's requested order.
                std::invoke(
                    function,
                    entity,
                    *first,
                    values[i]);
            }
        }
    }
}
```

This is the essence of Kairo's current query strategy.

---

**Why smallest-pool iteration is defensible**

Suppose:

$$
|A|=1,000,000
$$

and:

$$
|B|=10,000.
$$

Scanning A:

$$
1,000,000
$$

candidates.

Scanning B:

$$
10,000.
$$

Since membership tests are $O(1)$, B is clearly preferable.

So:

$$
\boxed{
driver=
argmin_i |Pool_i|.
}
$$

That is not an arbitrary micro-optimization.

It is the algorithmic query policy.

---

**But "smallest pool" is not always absolute optimality**

Imagine smallest pool is randomly ordered relative to other component dense arrays, causing many cache misses.

A slightly larger pool whose order correlates with another hot pool might sometimes outperform it.

Archetype storage solves much of this by aligning component row ownership.

Therefore smallest-pool is a strong sparse-set heuristic, not a universal theorem about machine performance.

Benchmark.

---

**The complete Kairo architecture now makes sense**

Your current codebase has three deliberately different layers.

```text
EngineCore::Scene
    durable authored identity
    validation
    hierarchy
    serialization
    reflection

          ↓ explicit extraction

KairoECS
    generational runtime identity
    sparse-set pools
    dense iteration
    structural journal

          ↓ adapters

Physics / Renderer / Gameplay
    process-local subsystem IDs
    hot runtime state
```

This separation is one of the strongest aspects of the current architecture.

Do not collapse it accidentally.

---

**Current Kairo gaps exposed by M089/M090**

The current implementation already has a lot: stale-generation tests, sparse sets, smallest-pool joins, authored/runtime identity separation, scene versioning, reflection descriptors, durable scene IDs, transactional runtime scene edits, and a deterministic JSON ECS benchmark.

The major remaining gaps for this specific pass are:

```text
generic KairoECS component serializer
entity-reference reflection/remapping at ECS level
generic ECS clone policy
prefab inheritance/override system
ECS-level inspector/debug storage views
storage epochs / checked component borrows
OOP/AoS benchmark baseline
paged sparse arrays for very sparse high-index pools
stronger formal nontrivial/noexcept component contracts
generalized network entity bridge
```

None of those invalidate the current architecture. They are the next production layers.

---

**Memory-budget exercise**

Assume:

$$
N=1,000,000.
$$

Components:

$$
Transform=48B,\quad N_T=1,000,000
$$

$$
Velocity=12B,\quad N_V=600,000
$$

$$
Health=4B,\quad N_H=100,000.
$$

Each flat sparse array spans one million entries:

$$
4\text{ MB approximately}.
$$

Dense Entity handle:

$$
8B.
$$

Transform pool:

$$
48M+8M+4M
=
60M\text{ bytes}.
$$

Velocity:

$$
12(600k)+8(600k)+4M
$$

$$
=7.2M+4.8M+4M
=
16M.
$$

Health:

$$
0.4M+0.8M+4M
=
5.2M.
$$

Total:

$$
81.2M
$$

bytes before allocator overhead, capacities and entity metadata.

Notice Health payload is only:

$$
0.4M
$$

but its sparse map is:

$$
4M.
$$

That ratio immediately tells you which pool benefits most from paging.

---

**One million entity performance thought experiment**

Movement system requires:

```text
Transform + Velocity.
```

Transform count:

$$
1M.
$$

Velocity count:

$$
600k.
$$

Sparse-set strategy scans:

$$
600k
$$

velocity owners.

Per candidate:

```text
Velocity contiguous read
Transform sparse lookup
Transform dense lookup
```

Archetype strategy might scan several matching archetype chunks totaling exactly 600k entities with:

```text
Transform[row]
Velocity[row]
```

aligned.

You should expect archetype iteration to have superior cross-component locality.

But adding/removing Velocity on 200k entities could make archetype migration far more expensive.

That is the actual trade-off to benchmark.

---

**The "object-oriented" comparison must also include virtual behavior**

A traditional design might be:

```cpp
class GameObject
{
public:
    virtual void Update(float dt) = 0;

    Transform TransformData;
    // many unrelated fields...
};
```

Then update loop:

```cpp
for (auto& object : objects)
{
    object->Update(dt);
}
```

This can introduce:

- pointer chasing,
- polymorphic dispatch,
- heterogeneous object sizes,
- unrelated data loaded,
- unpredictable instruction flow.

ECS system:

```text
one operation
over homogeneous data.
```

This is where data-oriented design often wins significantly.

But benchmark both a realistic polymorphic baseline **and** a strong contiguous AoS baseline.

Otherwise you learn nothing useful.

---

**Data-oriented design is about traversal direction**

Traditional object thinking:

```text
for each object:
    run all behavior touching many fields.
```

Data-oriented thinking:

```text
for each operation:
    traverse exactly the data needed.
```

That is the deeper distinction.

ECS is one implementation pattern for this.

Not the definition itself.

---

**The most important lifetime equation of this phase**

You should mentally separate four lifetimes:

$$
L_{\text{persistent ID}}
$$

$$
L_{\text{runtime entity}}
$$

$$
L_{\text{component object}}
$$

$$
L_{\text{component address}}.
$$

These are not equal.

A persistent Scene ID may survive save/load.

Runtime Entity survives until Destroy.

A Transform object survives until component removal/entity destroy.

Its **address** may survive only until vector relocation/swap-remove.

Thus typically:

$$
L_{\text{address}}
\le
L_{\text{component}}
\le
L_{\text{runtime entity}}
$$

while persistent identity can span multiple runtime incarnations.

This hierarchy explains a huge number of engine bugs.

---

**Reconstruction challenge — nontrivial component lifetime**

Given raw chunk memory and component T, you must be able to explain:

```text
reserve aligned storage
    ↓
construct_at
    ↓
object becomes live
    ↓
systems use it
    ↓
move-construct during migration
    ↓
destroy old instance
    ↓
eventually destroy new instance
```

And explain why:

```cpp
memcpy()
```

is not a universal substitute.

Also explain why `std::vector<T>` currently saves Kairo from most raw-lifetime complexity.

---

**Reconstruction challenge — persistence**

Given runtime:

```text
Entity {42,7}
Target -> Entity {91,4}
```

and persistent mapping:

```text
{42,7} -> SceneID 100
{91,4} -> SceneID 200
```

save:

```text
Entity 100
Target 200
```

Then on load:

```text
SceneID100 -> runtime {9,1}
SceneID200 -> runtime {10,1}
```

and reconstruct:

```text
{9,1}.Target = {10,1}.
```

If you ever serialize `{42,7}` directly as durable identity, you have violated the architecture.

---

**Reconstruction challenge — cloning**

Original:

```text
A
├── B
└── C

B.Target = C
```

Clone:

```text
A'
├── B'
└── C'
```

Build mapping:

$$
A\rightarrow A',
B\rightarrow B',
C\rightarrow C'.
$$

Then:

$$
B'.Target
=
remap[C]
=
C'.
$$

An external reference D not in clone set usually remains D.

This is exactly the algorithm you should be able to derive from scratch.

---

**Reconstruction challenge — sparse-set correctness proof**

Given dense index $d$:

$$
Sparse[Entities[d].Index]=d.
$$

That is the invariant.

Insert E:

$$
d=|Dense|
$$

append E/component,

then:

$$
Sparse[E.Index]=d.
$$

Remove E at d.

Let last entity be L at $l$.

If:

$$
d\neq l,
$$

move L to d and repair:

$$
Sparse[L.Index]=d.
$$

Then:

$$
Sparse[E.Index]=Missing.
$$

The invariant holds.

That is the entire mathematical core of sparse-set storage.

---

**Reconstruction challenge — generation correctness proof**

Entity handle:

$$
E=(i,g).
$$

Registry validates iff:

$$
Alive[i]
\land
Generation[i]=g.
$$

Destroy:

$$
Alive[i]\leftarrow false
$$

and:

$$
Generation[i]\leftarrow g+1.
$$

Recreate:

$$
E'=(i,g+1).
$$

For stale E:

$$
Generation[i]=g+1\neq g.
$$

Therefore:

$$
IsAlive(E)=false.
$$

Kairo further retires max-generation slots, so generation wrap cannot revalidate ancient handles.

That is your stale-handle proof.

---

**Reconstruction challenge — API invalidation**

Answer instantly:

If:

```cpp
Transform& t =
    registry.Get<Transform>(A);
```

and then:

```cpp
registry.Emplace<Transform>(B);
```

`t` may be invalidated by vector reallocation.

If:

```cpp
registry.Remove<Transform>(C);
```

`t` may be invalidated if A's transform was the last element moved into C's removed slot, or if `t` was C itself.

If:

```cpp
registry.Emplace<Velocity>(B);
```

a Transform reference normally remains unaffected because Velocity owns separate storage.

If:

```cpp
registry.Destroy(C);
```

several component pools may reorder.

This should become automatic reasoning.

---

**Exact coverage — M089 + M090**

| Part | Topic | Status |
|---:|---|---|
| 1475 | Component construction and destruction | ✓ |
| 1476 | Non-trivial C++ component types | ✓ |
| 1477 | Move-only components | ✓ |
| 1478 | Component serialization | ✓ |
| 1479 | Entity-reference serialization | ✓ |
| 1480 | Versioned component schemas | ✓ |
| 1481 | Reflection-driven component inspection | ✓ |
| 1482 | Editor property panels over ECS data | ✓ |
| 1483 | Undo/redo for component edits | ✓ |
| 1484 | Prefab/template entity concepts | ✓ |
| 1485 | Scene loading into an ECS | ✓ |
| 1486 | Scene saving from an ECS | ✓ |
| 1487 | Entity duplication | ✓ |
| 1488 | Entity cloning with remapped references | ✓ |
| 1489 | Network entity IDs vs local entity IDs | ✓ |
| 1490 | Deterministic entity iteration | ✓ |
| 1491 | ECS debugging views | ✓ |
| 1492 | Component-pool statistics | ✓ |
| 1493 | Detecting dangling external component pointers | ✓ |
| 1494 | Million-entity memory budgeting | ✓ |
| 1495 | Sparse-set complexity analysis | ✓ |
| 1496 | Archetype migration complexity | ✓ |
| 1497 | Ownership/invalidation API design | ✓ |
| 1498 | Entity-generation reuse testing | ✓ |
| 1499 | ECS vs object-oriented benchmarking | ✓ |
| 1500 | Build and defend a complete generational sparse-set ECS | ✓ |


You now have the full logical chain of the ECS phase:

$$
\boxed{
\begin{aligned}
&\text{Entity index}\\
&\downarrow\\
&\text{Generation safety}\\
&\downarrow\\
&\text{Free-list reuse}\\
&\downarrow\\
&\text{Sparse/dense component storage}\\
&\downarrow\\
&\text{Multi-component joins}\\
&\downarrow\\
&\text{Structural mutation boundaries}\\
&\downarrow\\
&\text{Archetypes/chunks as an alternative}\\
&\downarrow\\
&\text{SoA/hot-cold layouts}\\
&\downarrow\\
&\text{relationships and hierarchy}\\
&\downarrow\\
&\text{render/physics/script/input bridges}\\
&\downarrow\\
&\text{events and system DAGs}\\
&\downarrow\\
&\text{parallel scheduling}\\
&\downarrow\\
&\text{real C++ object lifetime}\\
&\downarrow\\
&\text{serialization/reflection/editor semantics}\\
&\downarrow\\
&\text{persistent/runtime/network ID separation}\\
&\downarrow\\
&\text{diagnostics, memory budgets and benchmarks}.
\end{aligned}
}
$$

The largest architectural lesson is that **an entity, a component object, the component's memory address, the authored scene object, the physics body, the renderer object and the network object are all different identities with different lifetimes**. Good engine architecture connects those identities with explicit bridges rather than pretending one integer or pointer can safely mean all of them.

**Pass 13 moves into Phase 16, beginning at Part 1501**: engine architecture as dependency management, foundation/runtime responsibilities, dependency inversion, cyclic-module avoidance, library boundaries, ABI/build decisions, module ownership, engine/application boundaries, and the larger asset/reflection/serialization/tooling architecture that turns all the subsystems we built so far into one maintainable engine.

## Give the renderer a clear job, then meet the graphics API

## Explicit backend execution closes the renderer's lifetime contract

The frontend knows *what* image must be produced; the backend knows *when* resources may be read, written and retired. This full migrated Vulkan/Metal chapter connects queues, synchronization, descriptors, resource transitions, frame overlap and device state to the engine's visible frame. Code fragments retain the original verification limits.

## From generated GPU work to safe explicit execution

The following extension follows data from GPU work generation through explicit API execution, renderer planning, Gaussian images, lighting, and animated surfaces. Definitions and worked examples precede the retained detailed lessons. Source-project observations about Kairo, Maveb and Vulkax are supplied draft claims; they have not been independently certified against a current repository revision. API examples retained below include fragments and pseudocode. Their presence is not a claim that every snippet was compiled or every device path tested.
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/17 - ECS performance through cache and scheduling|Previous chapter]] · [[Chapter Edition/03 - Game Engine/19 - GPU work and reusable Metal frames|Next chapter]]
