# 16 — Archetypes, chunks, relationships, and transforms

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/15 - Sparse-set components and generational entity handles|Previous chapter]] · [[Chapter Edition/03 - Game Engine/17 - ECS performance through cache and scheduling|Next chapter]]

Sparse sets make independent component stores easy to understand; archetypes group entities by shared component layout. This chapter compares the tradeoffs and then follows hierarchy, relationships, and transforms through chunked storage.

**By the end:** Compare sparse-set and archetype operations, then specify how entity migration and transform dependencies preserve identity and update order.

**Recall question:** What changes physically when an entity gains a component, and how do parent changes invalidate dependent world transforms?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 30486–34531; source-span SHA256 747d734fc016075db6541714fa4be60987a2244167f3812feafbf1293cc25c4b. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### ARCHETYPES, CHUNKS, LAYOUTS, RELATIONSHIPS AND TRANSFORMS

---

**Archetype ECS concept**

In the sparse-set model, storage is organized by **component type**.

```text
Transform pool
--------------------
E1 T1
E2 T2
E4 T4
E9 T9

Velocity pool
--------------------
E1 V1
E4 V4
E7 V7

Health pool
--------------------
E1 H1
E2 H2
```

Entity 1 has:

$$
\{Transform,Velocity,Health\}.
$$

Entity 2 has:

$$
\{Transform,Health\}.
$$

Entity 4 has:

$$
\{Transform,Velocity\}.
$$

An archetype ECS instead organizes entities by the **entire set of component types they possess**.

So:

```text
Archetype A
{Transform, Velocity, Health}

E1


Archetype B
{Transform, Health}

E2


Archetype C
{Transform, Velocity}

E4
```

An archetype is therefore:

$$
\boxed{
A(E)=
\{\text{all component types owned by E}\}
}
$$

and every entity with exactly the same component set belongs to the same archetype.

Unity Entities uses this exact organizational idea: entities with the same component set are grouped into an archetype, and entities of an archetype are stored in chunks containing parallel arrays for those components. [Unity User Manual](https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/concepts-archetypes.html?utm_source=chatgpt.com)

---

**Archetypes are equivalence classes**

Define relation:

$$
E_i\sim E_j
$$

iff:

$$
Components(E_i)=Components(E_j).
$$

This relation is reflexive, symmetric and transitive, so entities partition into equivalence classes.

Each equivalence class is an archetype.

If:

$$
E_1=\{A,B,C\}
$$

and:

$$
E_2=\{A,B,C\},
$$

then:

$$
E_1\sim E_2.
$$

But:

$$
E_3=\{A,B,C,D\}
$$

belongs to another archetype even if D is only a zero-byte tag.

That last point matters.

The **presence of a component type** defines archetype identity, not its memory size.

---

**Why this radically changes query execution**

Suppose a system wants:

```text
Transform
Velocity
Health
```

Sparse-set ECS asks:

```text
Which pool is smallest?
For every entity there:
    Does it also have the other components?
```

Archetype ECS asks:

> Which archetypes contain at least Transform, Velocity and Health?

Suppose the world contains:

```text
A1 = {Transform, Velocity, Health}
A2 = {Transform, Velocity, Health, AI}
A3 = {Transform, Health}
A4 = {Transform, Velocity}
A5 = {Transform, Velocity, Health, Renderable}
```

The query matches:

$$
A_1,\quad A_2,\quad A_5.
$$

Once archetype matching is cached, the system iterates entire contiguous storage regions where **every entity is already known to match**.

No per-entity `Has<Velocity>()` check is needed.

That is the great archetype advantage.

---

**Query work moves from entities to archetypes**

Imagine:

$$
N=1,000,000
$$

entities but only:

$$
A=120
$$

archetypes.

Query matching can be performed against 120 archetype signatures rather than one million entities.

If archetypes stabilize early, matched archetype lists can be cached and reused. Unity's documentation explicitly notes that archetype sets often stabilize early, allowing queries to cache matching archetypes. [Unity User Manual](https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/concepts-archetypes.html?utm_source=chatgpt.com)

So query setup can approach:

$$
O(A)
$$

once when cache is rebuilt,

then iteration is simply:

$$
O(N_{\text{matching entities}}).
$$

---

**Kairo's current position**

Your current `KairoECS` is **not an archetype ECS**.

It currently owns independent:

```cpp
SparseSet<Component>
```

pools selected through:

```cpp
Pool<Component>.
```

That is completely legitimate.

M087 is therefore partly canonical theory and partly a design alternative you must be able to evaluate against your existing sparse-set implementation.

Do **not** rewrite KairoECS into archetypes merely because this chunk exists.

First understand when one wins.

We will compare them properly in Part 1441.

---

**Archetype signatures**

We need a canonical identity for:

$$
\{Transform,Velocity,Health\}.
$$

One option is a bitset:

```text
bit 0 Transform
bit 1 Velocity
bit 2 Health
bit 3 Renderable
bit 4 Light
...
```

Then:

```text
Transform + Velocity + Health

00000111
```

is the archetype signature.

But with a dynamic component registry, a more general representation is a **sorted list of component type IDs**.

```cpp
using ComponentTypeID = std::uint32_t;

struct ArchetypeKey
{
    std::vector<ComponentTypeID> Types;
};
```

Canonical requirement:

$$
Types_0<Types_1<\dots<Types_n.
$$

So:

```text
{Velocity, Health, Transform}
```

and:

```text
{Health, Transform, Velocity}
```

canonicalize to the same key.

---

**Why sorted signatures are powerful**

Given sorted component IDs:

```text
[2, 5, 8, 17]
```

adding component 11 produces:

```text
[2, 5, 8, 11, 17].
```

Removing 5 produces:

```text
[2, 8, 17].
```

Signature hashing becomes deterministic.

Matching required components can use sorted-set containment.

And unlike a fixed 64-bit mask, there is no hard 64-component ceiling.

---

**Archetype lookup**

World needs:

```text
signature → Archetype*
```

Conceptually:

```cpp
std::unordered_map<
    ArchetypeKey,
    std::unique_ptr<Archetype>,
    ArchetypeKeyHash
> archetypes;
```

When adding/removing a component:

```text
source signature
      ↓
derive destination signature
      ↓
lookup/create destination archetype
      ↓
migrate entity
```

This lookup should not be repeated unnecessarily.

---

**Archetype transition edges**

Suppose archetype:

$$
A=\{Transform,Velocity\}.
$$

Adding Health always produces:

$$
B=\{Transform,Velocity,Health\}.
$$

Once discovered, cache:

```text
A
  add Health
      ↓
      B
```

Similarly:

```text
B
  remove Health
      ↓
      A
```

So archetypes themselves form a **transition graph**.

A production representation might contain:

```cpp
struct ArchetypeTransition
{
    Archetype* OnAdd;
    Archetype* OnRemove;
};
```

indexed by component type.

Then component add/remove avoids repeatedly rebuilding and hashing signatures.

---

**Archetype chunks**

An archetype may contain millions of entities.

We usually do not allocate one gigantic ever-growing block.

Instead divide storage into fixed-ish blocks called **chunks**.

```text
Archetype:
{Transform, Velocity, Health}

Chunk 0
E0..E511

Chunk 1
E512..E1023

Chunk 2
...
```

Each chunk stores the same set of component columns.

Unity Entities is one production example using fixed **16 KiB chunks**; chunk capacity depends on the component sizes in the archetype. [Unity User Manual](https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/concepts-archetypes.html?utm_source=chatgpt.com)

The specific 16 KiB number is not universal.

The architectural idea is.

---

**Why chunks rather than one large array**

Chunks provide:

- bounded relocation,
- easy job partitioning,
- predictable memory blocks,
- chunk-level change versions,
- chunk-level metadata,
- easier allocation/reclamation,
- reduced giant-vector reallocation,
- locality-preserving work units.

A chunk becomes simultaneously:

$$
\boxed{
\text{storage unit}
+
\text{iteration unit}
+
\text{job unit}
+
\text{change-tracking unit}.
}
$$

That is enormously useful.

---

**A conceptual archetype chunk**

For:

$$
A=\{Transform,Velocity,Health\},
$$

chunk memory conceptually looks like:

```text
+------------------------------------------------------+
| chunk header                                         |
+------------------------------------------------------+
| Entity[capacity]                                     |
+------------------------------------------------------+
| Transform[capacity]                                  |
+------------------------------------------------------+
| Velocity[capacity]                                   |
+------------------------------------------------------+
| Health[capacity]                                     |
+------------------------------------------------------+
```

Not:

```text
Entity
Transform
Velocity
Health
Entity
Transform
Velocity
Health
...
```

That distinction is Part 1440's columnar storage.

---

**Entity locations in archetype ECS**

Sparse-set ECS locates component through:

$$
Entity\rightarrow Sparse\rightarrow Dense.
$$

Archetype ECS needs an entity-location table:

```cpp
struct EntityLocation
{
    Archetype* Archetype;
    Chunk* Chunk;
    std::uint32_t Row;
};
```

Then:

$$
Entity
\rightarrow
(Archetype,Chunk,Row)
$$

and component T is:

$$
Chunk.Column<T>()[Row].
$$

The entity record itself therefore points to its current **storage row**, not to individual component addresses.

---

**Entity movement between archetypes**

This is the defining structural operation.

Suppose entity has:

$$
A=\{Transform,Velocity\}.
$$

We execute:

```cpp
Emplace<Health>(entity);
```

The entity's component set becomes:

$$
B=\{Transform,Velocity,Health\}.
$$

It cannot remain in A because every row in A must have exactly the A signature.

So the entity physically migrates.

---

**Archetype migration sequence**

Suppose source:

```text
A = {Transform, Velocity}
```

destination:

```text
B = {Transform, Velocity, Health}
```

Migration is:

```text
1. Find/create B.
2. Find destination chunk with free row.
3. Allocate destination row.
4. Move/copy Entity ID.
5. Move Transform.
6. Move Velocity.
7. Construct Health.
8. Update entity-location record.
9. Remove source row.
10. Repair location of entity swapped into source hole.
```

That is a structural change.

---

**Migration is much more expensive than changing component values**

Compare:

```cpp
velocity.x += 1.0f;
```

with:

```cpp
registry.Emplace<Health>(entity);
```

The first mutates perhaps 4 bytes.

The second may:

- locate two archetypes,
- allocate chunk space,
- move several components,
- construct a component,
- destruct source components,
- swap-remove source row,
- update entity metadata,
- update chunk counts,
- potentially update query caches.

Therefore archetype ECS strongly rewards worlds where:

$$
\boxed{
\text{component sets are relatively stable}.
}
$$

If entities constantly add/remove components every frame, archetype migration cost can dominate.

---

**Swap-removal inside source chunk**

Suppose source chunk rows:

```text
row 0 E10
row 1 E20
row 2 E30
row 3 E40
```

Move E20 to another archetype.

To keep source packed:

```text
row 1 ← E40
```

Then source becomes:

```text
row 0 E10
row 1 E40
row 2 E30
```

Now update:

```text
Location[E40].Row = 1.
```

Exactly the same invariant repair we learned for sparse sets appears again.

Dense storage repeatedly leads to swap-remove.

---

**Migration and exception safety**

With trivial components, migration is straightforward.

With:

```cpp
std::string
unique_ptr<Resource>
custom destructors
throwing constructors
```

it becomes substantially harder.

If moving three components succeeds and the fourth throws, what is the entity's state?

That is why M089 later spends an entire chunk on nontrivial component lifetime.

For now, recognize that archetype migration implicitly requires a type-erased operation table:

```text
construct
move construct
destroy
possibly copy
```

for every component type.

---

**Chunk capacity**

Suppose chunk size is:

$$
B.
$$

Header uses:

$$
H.
$$

Archetype contains columns with element sizes:

$$
s_0,s_1,\dots,s_k.
$$

Capacity $N$ is the largest integer satisfying:

$$
\boxed{
H+
\sum_{i=0}^{k}
AlignUp(Ns_i,a_i)
\le B
}
$$

where $a_i$ is required alignment for each column.

Entity IDs themselves are another column.

---

**Approximate capacity example**

Suppose:

```text
Entity      8 bytes
Transform  48 bytes
Velocity   12 bytes
Health      4 bytes
```

Ignoring alignment/header:

$$
bytes/entity
=
8+48+12+4
=
72.
$$

For a 16 KiB chunk:

$$
16384/72
\approx227.
$$

Actual capacity is slightly lower after header and alignment.

Add a 256-byte AI state component:

$$
72+256=328.
$$

Now:

$$
16384/328
\approx49.
$$

A single large component can drastically reduce chunk density.

This motivates hot/cold splitting.

---

**Alignment**

Suppose component requires:

$$
alignof(T)=16.
$$

Its column start must be aligned accordingly.

A simplistic layout:

```text
header
Entity column
Transform column
Velocity column
...
```

must call:

$$
AlignUp(offset,alignment)
$$

before each column.

Canonical helper:

```cpp
constexpr std::size_t AlignUp(
    std::size_t value,
    std::size_t alignment)
{
    return
        (value + alignment - 1)
        & ~(alignment - 1);
}
```

for power-of-two alignment.

---

**Chunk size is a systems choice**

Larger chunks:

- fewer allocations,
- more entities per job,
- lower metadata overhead,
- potentially more cache pollution,
- larger migration working set.

Smaller chunks:

- finer job granularity,
- smaller working sets,
- more chunk metadata,
- more allocations,
- potentially more fragmentation.

There is no sacred universal chunk size.

The correct number depends on:

$$
\text{cache hierarchy}
+
\text{component widths}
+
\text{job system}
+
\text{allocator}
+
\text{workload}.
$$

Unity's 16 KiB chunks are a useful production design reference, not a universal law. [Unity User Manual](https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/concepts-archetypes.html?utm_source=chatgpt.com)

---

**Columnar storage inside chunks**

Suppose archetype:

```text
Transform
Velocity
Health
```

AoS-like chunk storage would be:

```text
T0 V0 H0
T1 V1 H1
T2 V2 H2
...
```

Columnar storage is:

```text
T0 T1 T2 T3 ...
V0 V1 V2 V3 ...
H0 H1 H2 H3 ...
```

Each system accesses only the columns it needs.

---

**Why columnar chunks are powerful**

A movement system needs:

```text
Transform
Velocity
```

but not:

```text
Health
AI
Renderable
Audio
Name
```

Columnar layout means the CPU streams only:

```text
Transform[]
Velocity[]
```

The irrelevant columns need not enter cache.

That is the real data-oriented advantage.

---

**Archetype query execution**

Suppose query matches chunks C1, C2, C3.

For each:

```cpp
auto* transforms =
    chunk.Column<Transform>();

auto* velocities =
    chunk.Column<Velocity>();

for (uint32_t i = 0;
     i < chunk.Count();
     ++i)
{
    transforms[i].Position +=
        velocities[i].Linear * dt;
}
```

No `Has<T>`.

No sparse lookup.

No entity-level component discovery.

Just contiguous arrays known to match.

---

**Sparse-set versus archetype ECS**

Now the real comparison.

Sparse-set ECS optimizes:

$$
\boxed{
\text{independent component pools + flexible structural mutation}.
}
$$

Archetype ECS optimizes:

$$
\boxed{
\text{whole-component-set grouping + extremely efficient multi-component iteration}.
}
$$

Neither universally wins.

---

**Sparse-set strengths**

Sparse sets are excellent when:

- component sets change frequently,
- random component lookup is common,
- components are very sparse,
- you want independent component pools,
- engine complexity must remain moderate,
- query component counts are small,
- entity population is not extreme.

Adding Health only inserts into:

```text
Health pool
```

instead of migrating every existing component.

That can be much cheaper structurally.

---

**Archetype strengths**

Archetypes excel when:

- huge populations share a few signatures,
- systems process many components together,
- component sets are comparatively stable,
- chunk-level parallelism matters,
- query cache overhead must be tiny,
- change tracking can operate at chunk granularity.

For simulation-heavy workloads:

```text
Transform + Velocity + Acceleration + Mass
```

across hundreds of thousands of entities, archetype chunks can be extremely effective.

---

**Sparse-set weakness versus archetypes**

For:

```text
A+B+C+D+E
```

sparse-set query might scan the smallest pool and execute four sparse lookups per candidate.

Those lookups jump through several sparse/dense arrays.

Archetype query already knows that matching chunks contain all five components.

It performs zero membership tests per entity.

---

**Archetype weakness versus sparse sets**

Add one component to entity owning ten components.

Sparse set:

```text
insert one component
```

Archetype:

```text
move entity
+ move ten common components
+ construct new component
+ remove source row
```

So archetype structural mutation can be significantly more expensive.

---

**Hybrid ECS designs**

Real engines need not choose one ideology absolutely.

You could use:

- archetype chunks for hot simulation components,
- separate sparse pools for large/rare components,
- external stable storage for pointer-stable objects,
- resources outside entity storage,
- relationship indices separate from archetypes.

The design goal is not:

> be a pure archetype ECS.

The goal is:

$$
\boxed{
\text{put each data category where its access pattern belongs}.
}
$$

---

**AoS component data**

Array of Structures:

```cpp
struct Particle
{
    Vec3 Position;
    Vec3 Velocity;
    float Mass;
    float Lifetime;
};

std::vector<Particle> particles;
```

Memory:

```text
P0 V0 M0 L0
P1 V1 M1 L1
P2 V2 M2 L2
...
```

This is AoS.

It is convenient because all properties of one object are adjacent.

---

**When AoS is good**

Suppose code does:

```cpp
for (Particle& p : particles)
{
    RenderDebugParticle(
        p.Position,
        p.Velocity,
        p.Mass,
        p.Lifetime);
}
```

Every field is used.

AoS is reasonable.

Similarly, a complex component itself may be naturally treated as one logical record.

AoS is not inherently bad.

The question is always:

$$
\boxed{
\text{Does the access pattern consume the whole record?}
}
$$

---

**AoS waste**

Suppose physics integration only uses:

```text
Position
Velocity
```

but each Particle also stores:

```text
Name
Material
DebugColor
Lifetime
CollisionHistory
AssetReference
```

The cache line fetches irrelevant bytes.

Useful-byte ratio can become terrible.

That is where SoA helps.

---

**SoA component data**

Structure of Arrays:

```cpp
struct Particles
{
    std::vector<Vec3> Positions;
    std::vector<Vec3> Velocities;
    std::vector<float> Masses;
    std::vector<float> Lifetimes;
};
```

Memory:

```text
P0 P1 P2 P3 ...
V0 V1 V2 V3 ...
M0 M1 M2 M3 ...
L0 L1 L2 L3 ...
```

Now integration reads only the Position and Velocity streams.

---

**ECS is already partly SoA**

A sparse-set ECS with separate component pools:

```text
Transform[]
Velocity[]
Health[]
```

is SoA **at the component level**.

But inside Transform:

```cpp
struct Transform
{
    Vec3 Position;
    Quaternion Rotation;
    Vec3 Scale;
};
```

it is still AoS relative to its fields.

So there are multiple layout levels:

```text
world
  → component type separation
      → component internal field layout
```

You can have:

```text
SoA components
with AoS fields inside each component.
```

That is often perfectly fine.

---

**Transform SoA example**

If a system only changes positions for one million objects, an extreme SoA might be:

```cpp
struct TransformColumns
{
    float* PosX;
    float* PosY;
    float* PosZ;

    float* RotX;
    float* RotY;
    float* RotZ;
    float* RotW;

    float* ScaleX;
    float* ScaleY;
    float* ScaleZ;
};
```

This can be wonderful for SIMD.

But it substantially complicates APIs.

Do not split data until profiling shows a reason.

---

**AoSoA**

There is an intermediate pattern:

**Array of Structures of Arrays**.

For SIMD width 4:

```text
Block 0:
x[4]
y[4]
z[4]

Block 1:
x[4]
y[4]
z[4]
```

This can align naturally with vector registers while preserving manageable grouping.

High-performance particle, animation and physics kernels often benefit from such arrangements.

---

**Hot-and-cold splitting**

Suppose one component contains:

```cpp
struct Character
{
    // hot every frame
    Vec3 Position;
    Vec3 Velocity;
    float Health;

    // cold
    std::string DisplayName;
    AssetHandle Portrait;
    std::string Biography;
    DebugMetadata Debug;
};
```

Every update pulls cold bytes near hot data.

Instead split:

```cpp
struct CharacterRuntime
{
    Vec3 Position;
    Vec3 Velocity;
    float Health;
};

struct CharacterMetadata
{
    std::string DisplayName;
    AssetHandle Portrait;
    std::string Biography;
    DebugMetadata Debug;
};
```

Now hot loops ignore cold data entirely.

---

**Hot/cold splitting and chunks**

Large cold components reduce archetype chunk capacity even when systems never touch them.

Suppose hot data per entity is:

$$
64\text{ bytes}.
$$

Cold metadata adds:

$$
512\text{ bytes}.
$$

If stored together:

$$
576\text{ bytes/entity}.
$$

16 KiB chunk fits only roughly:

$$
28
$$

entities.

If cold data is external:

$$
16384/64
\approx256.
$$

That is almost an order of magnitude difference in chunk density.

This is why hot/cold splitting is not cosmetic.

---

**Kairo-specific hot/cold example**

Your authored `MeshRendererComponent` contains persistent asset handles, material slot arrays, shadow flags and render-layer data.

A runtime renderer may not want all of that in the hottest per-draw path.

The renderer-facing `MeshDraw` already represents a more runtime-specific extraction:

```cpp
MeshHandle Mesh;
Mat4f Model;
PBRMaterial Material;
uint32_t ObjectID;
bool CastShadows;
bool ReceiveShadows;
```

That is a good conceptual bridge:

```text
authored component
      ↓
runtime extraction
      ↓
hot renderer data
```

Do not force authoring representation to be the most efficient GPU submission representation.

---

**Tag components**

A tag component has identity but no per-entity payload.

Examples:

```cpp
struct Player {};
struct Dead {};
struct Frozen {};
struct Static {};
struct NeedsTransformUpdate {};
```

The entity either has the tag or does not.

In archetype ECS, tag presence affects signature but requires no component column.

---

**Tags are data despite having zero bytes**

`Dead` expresses state:

$$
Dead(E)\in\{false,true\}.
$$

A system can query:

```text
Health
WITHOUT Dead
```

The tag encodes one bit of semantic information through membership.

So a zero-byte component is still meaningful data.

---

**Tags and archetype fragmentation**

Be careful.

Every distinct component combination creates a distinct archetype.

If you introduce many independent tags:

```text
Selected
Highlighted
Sleeping
Visible
NetworkDirty
EditorLocked
QuestTarget
...
```

all combinations may theoretically generate:

$$
2^N
$$

signatures.

Most will never exist, but excessive dynamic tags can still create archetype proliferation.

For rapidly toggled flags, sometimes one packed bitfield component is better than structural tag migration.

Again:

$$
\boxed{
\text{structural state}
\neq
\text{ordinary mutable state}.
}
$$

---

**Kairo's current tags are not ECS tag components**

Current `EngineCore::EntitySettingsComponent` contains:

```cpp
std::vector<std::string> Tags;
```

These are flexible authored string tags.

Examples might be:

```text
"player"
"enemy"
"spawn-point"
```

That is different from:

```cpp
struct Dead {};
```

as an ECS structural tag.

String tags are good for authoring/search.

Compile-time or numeric ECS tags are better for hot simulation queries.

Do not conflate them.

---

**Singleton/resource components**

Some data belongs to the **world**, not to each entity.

Examples:

```text
DeltaTime
PhysicsWorld
InputState
NavigationWorld
RenderSettings
Weather
RandomGenerator
ActiveCamera
```

Duplicating these across entities is nonsense.

Use a resource/singleton model.

---

**Singleton entity versus resource storage**

One pattern:

```text
Entity WorldSettings
    Time
    Gravity
    Environment
```

Another:

```cpp
registry.Resource<Time>();
registry.Resource<InputState>();
```

with separate world resource storage.

Resource storage often communicates intent more clearly:

$$
\boxed{
\text{there is exactly one value per ECS world}.
}
$$

A singleton entity is useful if you want ordinary entity queries/reflection/serialization.

Both are valid.

---

**Input is usually a resource first**

Your current `Kairo.EngineCore.Input` behaves like a resource/service.

It stores:

- current and previous keys,
- mouse buttons,
- gamepad buttons,
- gamepad axes,
- mouse position,
- mouse delta,
- scroll delta.

That is appropriate for raw device state.

It should not be copied onto every player entity.

Later a system can transform raw input into per-entity intent:

```cpp
struct CharacterInput
{
    Vec2 Move;
    Vec2 Look;
    bool JumpPressed;
};
```

So:

```text
Input resource
     ↓
input-mapping system
     ↓
CharacterInput component
```

This distinction becomes important in Part 1458.

---

**Relationship components**

Not all ECS data is unary.

Ordinary component says:

$$
Has(E,T).
$$

A relationship says:

$$
R(E_a,E_b).
$$

Examples:

```text
ChildOf(child,parent)
Targets(attacker,target)
OwnedBy(item,player)
AttachedTo(weapon,bone)
Follows(camera,character)
```

This is graph data inside an ECS.

---

**Relationship-as-component design**

Simplest:

```cpp
struct Parent
{
    Entity Value;
};

struct Target
{
    Entity Value;
};
```

Then each entity has at most one Parent and one Target.

This works extremely well for many relationships.

---

**Relationship pairs**

Another model treats:

$$
(RelationshipType,\TargetEntity)
$$

as a component-like ID.

For example:

```text
(ChildOf, Entity42)
(Eats, Apple)
(Likes, Entity17)
```

Flecs provides this pair model directly, allowing relationships to participate in ECS queries. [Flecs](https://www.flecs.dev/flecs/Relationships.html?utm_source=chatgpt.com)

It is powerful, but it has storage implications.

If target entity participates in archetype identity, many different relationship targets can fragment storage into many tables/archetypes.

---

**Parent-as-data versus parent-as-signature**

Consider 100,000 children with 10,000 possible parents.

If parent identity becomes part of archetype signature:

```text
ChildOf Parent17
ChildOf Parent18
ChildOf Parent19
...
```

you may get large numbers of storage groups.

If instead:

```cpp
struct Parent
{
    Entity Value;
};
```

all children with a Parent component can share the same archetype.

Flecs itself distinguishes hierarchy storage strategies for exactly this reason: its regular `ChildOf` relationship can partition storage by parent, while a parent-component-style hierarchy can avoid fragmenting ordinary component layout. [Flecs](https://www.flecs.dev/flecs/HierarchiesManual.html?utm_source=chatgpt.com)

This is a deep storage consequence of relationship modeling.

---

**Parent-child relationships**

A scene hierarchy requires:

$$
Parent(child)=parent.
$$

We often also want efficient:

$$
Children(parent).
$$

You can store only Parent and derive children by scanning.

But that is:

$$
O(N)
$$

per parent query.

Better maintain reverse adjacency.

---

**Bidirectional hierarchy invariants**

If child C says:

$$
Parent(C)=P,
$$

then P's child index should contain C.

So:

$$
\boxed{
Parent(C)=P
\iff
C\in Children(P)
}
$$

for live hierarchy relations.

This is exactly the invariant your current `EngineCore::Scene` maintains.

It stores:

```cpp
std::optional<Entity> Parent;
std::vector<Entity> Children;
```

and `SetParent()` updates both sides.

---

**Cycle prevention**

A hierarchy should be a forest/DAG, not:

```text
A parent B
B parent C
C parent A
```

Your current `Scene::SetParent()` walks ancestors of the prospective parent and rejects if it encounters the child.

That enforces:

$$
child\notin Ancestors(parent).
$$

This is essential because world-transform propagation assumes acyclicity.

---

**Destruction policy**

What happens when a parent dies?

Common choices:

1. destroy entire subtree,
2. reparent children to root,
3. reparent to parent's parent,
4. reject destruction until children removed.

Your current `Scene` chooses **subtree destruction**.

It performs an iterative walk rather than recursive destruction, avoiding stack overflow on very deep hierarchies.

That is a clear and coherent policy.

---

**Scene hierarchy transforms**

Each entity has local transform:

$$
L_e.
$$

Root world transform:

$$
W_r=L_r.
$$

For child C of P:

$$
\boxed{
W_C=W_P L_C
}
$$

assuming the engine's transform composition convention uses parent * local.

For a chain:

```text
Root → Arm → Hand → Sword
```

Sword world transform is:

$$
W_S=
L_RL_AL_HL_S.
$$

---

**Current Kairo hierarchy calculation**

Your current `Scene::WorldTransform()`:

1. walks from entity to root collecting ancestors,
2. reverses the ancestry,
3. multiplies local transforms root-to-leaf.

Conceptually:

```cpp
Transform world = Identity();

for (Entity e : rootToLeaf)
{
    world *= Local(e);
}
```

This is correct and authoritative.

But its complexity is:

$$
O(depth)
$$

per call.

---

**Why repeated on-demand hierarchy walks become expensive**

Suppose 100,000 entities form moderate hierarchies and renderer asks for world transform of each.

A parent's transform may be recomputed separately for every descendant.

If tree depth averages $d$:

$$
O(Nd).
$$

A dedicated transform system can instead traverse hierarchy once:

$$
O(N).
$$

Each parent's world transform is computed once and reused for its children.

---

**Hierarchy traversal order**

You must process parent before child.

One way is breadth-first:

```text
depth 0 roots
depth 1 children
depth 2 grandchildren
...
```

Another is depth-first pre-order.

What matters is:

$$
\boxed{
ParentWorld
\text{ must exist before }
ChildWorld.
}
$$

This introduces a dependency that ordinary flat ECS iteration does not automatically capture.

---

**Local and world transform components**

A runtime ECS may separate:

```cpp
struct LocalTransform
{
    Vec3 Position;
    Quaternion Rotation;
    Vec3 Scale;
};

struct WorldTransform
{
    Mat4 Matrix;
};
```

Then hierarchy system reads:

```text
LocalTransform
Parent
Parent.WorldTransform
```

and writes:

```text
WorldTransform
```

This makes dependencies explicit.

---

**Dirty transform propagation**

Recomputing every world transform every frame is unnecessary if most entities do not move.

Mark changed transforms dirty.

Simple approach:

```cpp
struct TransformDirty {};
```

When local transform changes:

```text
mark entity dirty
mark all descendants dirty
```

Then update only dirty subtrees.

---

**Immediate dirty propagation**

If entity A changes:

```text
A
├── B
│   ├── D
│   └── E
└── C
```

mark:

```text
A B C D E dirty.
```

Correct but modifying A may require:

$$
O(size(subtree)).
$$

If the hierarchy is huge and parents change frequently, dirty marking itself becomes expensive.

---

**Version-based lazy propagation**

A more elegant design stores:

```cpp
struct TransformState
{
    uint64_t LocalVersion;
    uint64_t WorldVersion;

    uint64_t CachedParentWorldVersion;
};
```

Recompute entity E if:

$$
LocalVersion_E
\text{ changed}
$$

or:

$$
WorldVersion_{Parent(E)}
\neq
CachedParentWorldVersion_E.
$$

After recomputation:

```text
CachedParentWorldVersion =
    Parent.WorldVersion
```

and increment E's world version.

Now changing a parent does not require immediately writing dirty flags through the entire subtree.

Descendants discover the change during ordered traversal.

---

**Dirty propagation and archetype tags**

You *could* use:

```cpp
struct DirtyTransform {};
```

as a tag.

But if this tag is frequently added and removed every frame in an archetype ECS, entities constantly migrate between:

```text
{Transform, Parent}
```

and:

```text
{Transform, Parent, DirtyTransform}.
```

That is expensive.

For very transient state, a bit/flag/version inside existing transform metadata can be better.

This is exactly the kind of decision Part 1445 was warning about.

---

**Previous and current transform snapshots**

Physics often runs at fixed frequency:

$$
dt_f=\frac1{60}.
$$

Rendering may run:

$$
90,\ 120,\ 144,\ 240\text{ Hz}.
$$

If renderer uses only latest physics state, objects visibly step at 60 Hz.

Store:

$$
T_{prev}
$$

and:

$$
T_{curr}.
$$

Then render between them.

---

**Fixed-step interpolation**

Frame accumulator:

$$
accumulator.
$$

After fixed simulation steps:

$$
\alpha=
\frac{accumulator}{dt_f}.
$$

Where:

$$
0\le\alpha<1.
$$

Render position:

$$
p_r=
(1-\alpha)p_{prev}
+
\alpha p_{curr}.
$$

Scale similarly.

Rotation should use:

$$
q_r=
Slerp(q_{prev},q_{curr},\alpha).
$$

Do **not** generally linearly interpolate complete transformation matrices.

That can introduce shear/non-orthonormal rotations.

---

**Snapshot update order**

Every physics tick:

```cpp
Previous = Current;

SimulateFixedStep();

Current = NewSimulationTransform;
```

Then rendering independently uses:

```cpp
Interpolate(
    Previous,
    Current,
    alpha);
```

This decouples simulation and rendering clocks.

---

**Teleports**

Suppose character teleports 100 meters.

If:

```text
Previous = old location
Current  = new location
```

interpolation makes it fly rapidly across the map for one rendered interval.

For teleport:

```cpp
Previous = teleportedTransform;
Current  = teleportedTransform;
```

to suppress interpolation.

This is a small but essential runtime rule.

---

**Current Kairo status for M087**

Current Kairo contains:

- a sparse-set ECS rather than archetype chunks,
- authored `Scene` parent/children hierarchy,
- cycle-safe parenting,
- on-demand world transform calculation,
- flexible string tags.

It does **not currently implement** in inspected KairoECS:

- archetypes,
- archetype chunks,
- archetype migration,
- chunk column layouts,
- archetype transition caches,
- dirty-transform propagation,
- previous/current ECS transform snapshots.

So M087 is largely a **next-level runtime architecture**, not a claim about what your sparse-set registry already does.

---

### ENGINE COMPONENTS, BRIDGES, EVENTS, TICKS AND PARALLEL SYSTEM SCHEDULING

---

**Renderable component representation**

A scene/ECS renderable should not own Vulkan/Metal command buffers.

It should describe **render intent**.

A useful authored component:

```cpp
struct Renderable
{
    MeshAssetHandle Mesh;
    MaterialAssetHandle Material;

    uint64_t RenderLayers;

    bool Visible;
    bool CastShadows;
    bool ReceiveShadows;
};
```

Your current `MeshRendererComponent` already follows this philosophy.

It contains persistent typed asset handles and renderer-neutral policy.

That is excellent separation.

---

**Authored renderable versus runtime renderer object**

Asset identity:

```text
MeshAssetHandle
MaterialAssetHandle
```

is not necessarily the renderer's runtime GPU handle.

The rendering bridge resolves:

```text
ECS Entity
MeshAssetHandle
MaterialAssetHandle
WorldTransform
        ↓
Renderer
        ↓
MeshHandle
GPU buffers
descriptor tables
pipeline/material state
```

Keep those domains separate.

Otherwise scene files accidentally serialize GPU-process-local IDs.

---

**Render extraction**

A good ECS/render pipeline often builds a frame-local render scene:

```text
ECS components
     ↓
visibility/culling
     ↓
render extraction
     ↓
MeshDraw[]
Light[]
Camera
     ↓
renderer
```

Your `KairoRenderer::RenderScene` already works this way.

A `MeshDraw` is a runtime submission record rather than the authored component itself.

This is precisely the architecture you want.

---

**Physics body component**

A rigid-body ECS component describes persistent physical intent:

```cpp
struct RigidBody
{
    MotionType Motion;

    float Density;
    float GravityScale;
    float LinearDamping;
    float AngularDamping;
};
```

Your current `RigidBodyComponent` stores exactly this kind of authoring data.

Critically, its documentation says runtime body IDs belong to the physics-world adapter and are **not serialized in the scene**.

That separation is correct.

---

**Why runtime physics IDs should not live in persistent authored data**

Suppose scene file stores:

```text
BodyID = 83.
```

Next run physics world allocates bodies in a different order.

Body 83 may now be unrelated.

Therefore persistent data contains:

```text
how this entity should behave physically
```

while runtime bridge stores:

```text
which PhysicsWorld body currently implements it.
```

Two distinct identities.

---

**Collider component**

Collider contains geometric and filtering data:

```cpp
struct Collider
{
    Shape ShapeType;

    Vec3 HalfExtents;
    float Radius;
    float HalfHeight;

    float Friction;
    float Restitution;

    uint32_t BelongsTo;
    uint32_t CollidesWith;

    bool IsTrigger;
};
```

Your current component contains these concepts.

That is a good persistent representation.

---

**Multiple colliders per entity**

A simple optional:

```text
ColliderComponent
```

implies at most one authored collider per entity.

Complex characters may need:

```text
torso capsule
head sphere
weapon hitbox
foot sensors
```

There are several ECS designs:

1. dynamic buffer of collider descriptors,
2. child entities each owning Collider,
3. multiple specialized collider components,
4. one external physics body with several colliders represented by a handle list.

Child-entity colliders fit naturally with entity relationships but increase entity count.

A buffer is compact but makes individual collider identity trickier.

Again, model according to access/lifetime needs.

---

**Light component**

Light data should describe physical/scene intent.

Your `LightComponent` includes:

- Directional/Point/Spot/RectangleArea,
- linear RGB color,
- physically meaningful intensity,
- Lux/Candela/Nit,
- range,
- spot cone angles,
- area dimensions,
- shadow policy,
- shadow bias,
- render layers.

This is considerably better than:

```cpp
float intensity = 5;
```

with no unit.

Explicit units make the renderer adapter deterministic.

---

**Light transforms**

Light component does not need to duplicate world position/direction.

Position and orientation should come from:

```text
Transform
```

while light stores photometric properties.

That is component composition:

```text
Transform + Light
```

rather than bloating Light with unrelated transform state.

---

**Camera component**

Your current Camera contains:

- perspective/orthographic mode,
- vertical FOV,
- near/far planes,
- orthographic size,
- exposure,
- clear mode/color,
- render-layer mask,
- Primary flag.

Again, camera transform comes from Transform.

Thus camera entity is:

```text
Transform
Camera
```

not one monolithic camera object.

---

**Primary camera is global state hiding in entity data**

`CameraComponent::Primary` allows at most one primary camera in your current `Scene`.

Another architecture would store:

```cpp
struct ActiveCameraResource
{
    Entity Camera;
};
```

Why?

Because "which camera is active?" is really world-level state.

Then cameras themselves do not need mutually exclusive `Primary` flags.

Both designs work.

The resource design avoids scanning or invariant enforcement across all camera components.

---

**Script or behavior component**

A script component should generally store:

```text
script asset/class identity
enabled state
serialized parameters
```

not a raw VM pointer.

Your current:

```cpp
struct LogicComponent
{
    DocumentAssetHandle Document;
    bool Enabled;
};
```

does exactly the persistent-identity part.

Runtime VM instances are deliberately process-local.

That is the right separation.

---

**Script runtime state**

The bridge might maintain:

```text
Entity
   ↓
ScriptInstanceID
   ↓
VM state
```

or a runtime component:

```cpp
struct ScriptRuntime
{
    ScriptInstanceID Instance;
};
```

but that process-local data should not be confused with the authored Logic component.

On scene load:

```text
LogicComponent
    ↓
compile/load bytecode
    ↓
create VM instance
    ↓
runtime bridge mapping
```

---

**Input component/state representation**

There are two different "input states."

Raw device state:

```text
W key down
mouse dx
gamepad axis 0 = 0.72
```

Gameplay intent:

```text
Move = (0.8,0.1)
Look = (-2.1,0.4)
JumpPressed = true
Fire = true
```

Raw state belongs naturally to a resource/service.

Gameplay intent can belong to an entity.

---

**Why this separation matters**

Keyboard W does not inherently mean:

$$
MoveForward.
$$

Bindings may change.

A gamepad may produce the same Move action.

AI may generate Move without any physical device.

Networking may inject remote inputs.

So:

```text
raw hardware
     ↓
input mapping
     ↓
gameplay command component
```

allows human, AI, replay and network inputs to converge onto the same simulation interface.

---

**Entity-to-physics ID mapping**

Your PhysicsWorld has runtime IDs:

```text
BodyID
ColliderID
```

Your ECS has:

```text
Entity(Index,Generation)
```

These are separate identity spaces.

You need a bridge.

---

**Bidirectional physics bridge**

A useful structure:

```cpp
struct PhysicsEntityBridge
{
    std::unordered_map<Entity, BodyID>
        EntityToBody;

    std::vector<Entity>
        BodyToEntity;
};
```

Then:

$$
Entity\rightarrow BodyID
$$

for writing forces/transforms,

and:

$$
BodyID\rightarrow Entity
$$

for collision events/raycast hits.

---

**Why reverse mapping matters**

Physics reports:

```text
Body 17 collided with Body 43.
```

Gameplay cares about:

```text
Player collided with Door.
```

So:

```text
Body17
   ↓
Entity A

Body43
   ↓
Entity B
```

must be cheap.

A one-way Entity→Body map is insufficient.

---

**Kairo PhysicsWorld ID behavior**

Current PhysicsWorld creates body IDs as vector indices and, on destruction, deactivates records rather than compacting/reusing them immediately.

That means an old BodyID does not silently alias a newly compacted body.

This is a useful deletion-safety property.

If you later introduce body-ID reuse, consider a generational physics handle for the same reason ECS entities use generations.

---

**Entity-to-render-object mapping**

Renderer may have:

```text
MeshHandle
MaterialHandle
RenderProxyID
ObjectID
```

ECS Entity should remain independent.

Bridge:

```text
Entity
  ↓
render extraction
  ↓
frame render object
```

Many engines do not need a permanent render object for every entity.

Frame extraction can simply generate transient draws.

---

**Current Kairo `MeshDraw`**

Your renderer has:

```cpp
struct MeshDraw
{
    MeshHandle Mesh;
    Mat4f Model;
    PBRMaterial Material;
    uint32_t ObjectID;
    ...
};
```

This is already a clear separation:

```text
renderer resource MeshHandle
```

versus:

```text
ObjectID used to identify the scene object.
```

Good architecture.

---

**Stable object IDs for picking**

GPU picking often renders an integer ID into an offscreen attachment:

```text
pixel → object ID
```

Then readback gives:

$$
ID=1234.
$$

Map 1234 to scene/entity.

But there is a subtle lifetime problem.

GPU readback is asynchronous.

---

**Why writing ECS index directly into the pick buffer can be unsafe**

Frame N:

```text
Entity {Index=42, Generation=7}
→ pick value 42
```

Entity dies.

Before readback completes, index 42 gets reused:

```text
Entity {42,8}.
```

GPU reports:

```text
42.
```

If CPU interprets only the index, it selects the wrong object.

This is the same ABA problem again.

---

**Safer GPU picking**

Option A: write persistent scene ID if it is guaranteed not to recycle during relevant lifetime.

Your current authored `Scene::Entity` uses stable scene-local `uint32_t` IDs, and `MeshDraw::ObjectID` can naturally carry that.

Option B: use a frame-local pick table:

```text
PickID 1 -> Entity{42,7}
PickID 2 -> Entity{97,3}
...
```

Render only PickID.

Keep the table until GPU readback for that frame finishes.

Then:

```text
pixel PickID
   ↓
that frame's immutable lookup table
   ↓
generational Entity
   ↓
IsAlive()
```

This is especially robust for runtime ECS handles.

---

**ECS event queues**

Not every interaction should call another system immediately.

Suppose physics detects 50 collisions.

Instead of recursively invoking gameplay inside solver code, produce events:

```cpp
struct CollisionEvent
{
    Entity A;
    Entity B;
    CollisionPhase Phase;
};
```

Push them into an event queue.

Later gameplay systems consume them.

---

**Why event queues help**

They decouple:

$$
\text{producer timing}
$$

from:

$$
\text{consumer execution}.
$$

Physics can finish in a clean state.

Gameplay can process events afterward.

Structural changes can be command-buffered.

Replay can record events.

Multiple listeners can consume them.

This is much easier to reason about than arbitrary nested callbacks.

---

**Double-buffered event queues**

A strong design is:

```text
WriteEvents
ReadEvents
```

During phase:

```text
producers append WriteEvents
consumers read ReadEvents
```

At synchronization:

```cpp
swap(ReadEvents, WriteEvents);
WriteEvents.clear();
```

This prevents an event handler generating another event from recursively entering the same iteration unexpectedly.

It establishes clear frame/tick boundaries.

---

**Current Kairo event systems**

`EngineCore::Event` currently represents platform/window events such as:

```text
WindowClose
WindowResize
KeyPressed
MouseMoved
...
```

and supports immediate handled propagation.

Separately, `PhysicsWorld` has:

```cpp
PhysicsContactEvent
```

with:

```text
Begin
Stay
End
```

and supports both polling and a synchronous callback.

So current Kairo already has **event-producing subsystems**, but not yet one generalized typed ECS event-queue layer.

---

**Collision event delivery**

Physics emits:

```text
BodyID A
BodyID B
ColliderID A
ColliderID B
Begin/Stay/End
Trigger flag
```

ECS bridge resolves:

```text
BodyID → Entity
```

then produces gameplay event:

```cpp
struct EntityCollisionEvent
{
    Entity A;
    Entity B;

    ColliderID ColliderA;
    ColliderID ColliderB;

    CollisionPhase Phase;
    bool IsTrigger;
};
```

Now gameplay never needs to know body-table internals.

---

**Event timing relative to physics**

Current Kairo PhysicsWorld carefully dispatches contact callbacks only after solver arrays and profiling have finished.

Its source explicitly avoids callback mutation while iterating live body/contact/joint storage.

That is exactly the right principle.

For ECS integration I would go one step further:

```text
Physics Step
    ↓
capture contact events
    ↓
map IDs to entities
    ↓
enqueue ECS events
    ↓
finish physics phase
    ↓
gameplay event systems
    ↓
deferred structural flush
```

This maximizes isolation.

---

**Begin-play lifecycle events**

Many components need one-time initialization.

Examples:

```text
script OnStart
AI initialize
audio autoplay
particle emitter activation
runtime resource creation
```

You need to distinguish:

```text
entity exists
```

from:

```text
entity has entered gameplay.
```

---

**Lifecycle state**

A simple component:

```cpp
enum class LifecycleState : uint8_t
{
    PendingBeginPlay,
    Running,
    PendingEndPlay
};

struct Lifecycle
{
    LifecycleState State;
};
```

Begin-play system queries:

```text
Lifecycle == PendingBeginPlay
```

executes initialization, then sets:

```text
Running.
```

---

**A tag-based lifecycle alternative**

Use:

```cpp
struct NeedsBeginPlay {};
```

System runs:

```text
Logic + NeedsBeginPlay
```

then removes tag.

But in archetype ECS, removing the tag migrates the entity.

For a one-time operation that occurs only once in entity lifetime, that may be entirely acceptable.

For state that toggles every frame, less so.

---

**Components added after gameplay starts**

Important semantic question.

Suppose world is already running and entity receives a new:

```text
LogicComponent.
```

Should that script receive BeginPlay?

Usually yes—but define it explicitly.

Lifecycle belongs not only to entity creation, but sometimes to **component activation**.

You may need:

```text
OnComponentAdded
OnBeginPlay
OnEnable
OnDisable
OnDestroy
```

as separate concepts.

Do not collapse them into one vague "Start."

---

**Fixed tick versus render tick**

Physics needs stable timestep.

Rendering needs actual display cadence.

So maintain at least two clocks.

Physics:

$$
dt_f=\frac{1}{60}.
$$

Render:

$$
dt_r=
\text{actual frame duration}.
$$

---

**Fixed-step accumulator**

Canonical:

```cpp
accumulator += frameDelta;

std::uint32_t substeps = 0;

while (accumulator >= fixedDelta &&
       substeps < maxSubsteps)
{
    FixedUpdate(fixedDelta);

    accumulator -= fixedDelta;
    ++substeps;
}

const float alpha =
    accumulator / fixedDelta;

Render(alpha);
```

This decouples simulation stability from rendering cadence.

---

**Spiral of death**

Suppose simulation takes longer than real time.

Frame takes 100 ms.

At 60 Hz you owe:

$$
100/16.67\approx6
$$

physics steps.

Those steps themselves may take another 100 ms.

Now you owe more.

System falls progressively behind.

This is the spiral of death.

Use:

```text
maximum substeps
frame-time clamp
adaptive degradation
```

depending on engine requirements.

Never allow an unbounded fixed-step catch-up loop.

---

**Tick groups**

A useful high-level order:

```text
Input sample
     ↓
Variable gameplay preparation
     ↓
Fixed gameplay
     ↓
Physics
     ↓
Collision event extraction
     ↓
Post-physics gameplay
     ↓
Animation / transform finalization
     ↓
Render extraction
     ↓
Rendering
```

Actual engines vary, but the principle is explicit phase boundaries.

---

**System ordering dependencies**

Some systems depend semantically on others.

Example:

```text
InputMapping
    ↓
CharacterControl
    ↓
PhysicsForceApplication
    ↓
PhysicsStep
    ↓
PhysicsToTransform
    ↓
RenderExtraction
```

Executing RenderExtraction before PhysicsToTransform yields one-frame-old transforms.

That is not a thread race.

It is a **semantic ordering bug**.

---

**Explicit before/after dependencies**

A system descriptor may declare:

```cpp
struct SystemDescriptor
{
    SystemID ID;

    std::vector<SystemID> After;
    std::vector<SystemID> Before;
};
```

For example:

```text
PhysicsStep.After(CharacterController)
RenderExtraction.After(PhysicsToTransform)
```

Scheduler converts these into edges.

---

**System dependency graph**

Represent systems as a DAG:

$$
G=(V,E)
$$

where:

$$
A\rightarrow B
$$

means:

> B may not execute until A finishes.

Example:

```text
        Animation
           |
           v
Input -> Gameplay -> Physics
           |           |
           |           v
           +------> Transform
                        |
                        v
                  RenderExtract
```

Independent branches can execute simultaneously.

---

**Data hazards also produce dependencies**

Suppose:

```text
System A writes Velocity
System B reads Velocity
```

They cannot overlap unless additional partition guarantees exist.

Similarly:

```text
A writes Position
B writes Position
```

conflicts.

But:

```text
A reads Position
B reads Position
```

is safe.

So access declarations can generate dependency edges.

---

**Topological scheduling**

A DAG can be executed in topological order.

Kahn's algorithm:

1. compute indegree of every system,
2. push zero-indegree systems into ready set,
3. execute/remove ready node,
4. decrement outgoing neighbor indegrees,
5. newly zero-indegree nodes become ready.

If processed count:

$$
<|V|,
$$

a cycle exists.

That is a configuration error.

---

**Parallel topological scheduler**

Instead of taking one zero-indegree node at a time, take **all currently ready nodes**.

```text
Wave 0:
A B C

Wave 1:
D E

Wave 2:
F
```

A/B/C can run concurrently.

After they complete, D/E become available.

This exposes parallelism while preserving dependencies.

---

**Current Kairo JobSystem**

Your `KairoEngineCore::JobSystem` currently provides:

- fixed worker pool,
- thread-safe job queue,
- futures,
- exception propagation,
- `WaitIdle()`.

It is a **generic executor**.

It does not currently know:

- ECS component reads,
- ECS component writes,
- system dependencies,
- topological scheduling,
- archetype/chunk iteration.

That is a good layering boundary.

An ECS scheduler can sit **above** the JobSystem.

---

**Parallel-safe ECS iteration**

Suppose one chunk contains:

$$
1000
$$

Transform + Velocity entities.

You can partition:

```text
Thread 0 rows   0–249
Thread 1 rows 250–499
Thread 2 rows 500–749
Thread 3 rows 750–999
```

If every worker writes only its own rows:

$$
\text{no component element is written by two threads}.
$$

Safe.

---

**Chunk-level parallelism**

Archetypes make this particularly natural:

```text
Chunk 0 -> worker A
Chunk 1 -> worker B
Chunk 2 -> worker C
Chunk 3 -> worker D
```

Each chunk is an independent dense memory region.

This is one of the strongest practical arguments for chunk storage.

---

**Structural changes during parallel iteration**

Still forbidden.

Thread A cannot migrate an entity out of a chunk while Thread B is iterating the same archetype storage.

Use thread-local command buffers:

```text
worker 0 → commands 0
worker 1 → commands 1
worker 2 → commands 2
```

merge/apply after jobs finish.

Exactly the command-buffer principle from Pass 10 becomes mandatory in parallel ECS.

---

**Read-only versus writable declarations**

A scheduler needs to know what every system intends to access.

Example:

```cpp
struct MovementSystemAccess
{
    Reads<Velocity> ReadsVelocity;
    Writes<Transform> WritesTransform;
};
```

Conceptually maintain:

$$
R(S)
$$

read set,

$$
W(S)
$$

write set.

---

**Why `const` at callback level is not enough**

C++ can enforce:

```cpp
const Velocity&
```

inside one callback.

But scheduler must know access **before the job runs**.

It needs metadata to decide concurrency.

So access intent is part of system registration.

---

**Resource accesses too**

Systems can also conflict through globals.

Example:

```text
System A writes PhysicsWorld resource
System B reads PhysicsWorld resource
```

even if their ECS component sets do not overlap.

Therefore descriptor needs:

```text
component reads/writes
resource reads/writes
structural access
```

All contribute to scheduling hazards.

---

**Detecting system data hazards**

For systems A and B:

No hazard if:

$$
W(A)\cap
(R(B)\cup W(B))
=
\varnothing
$$

and:

$$
W(B)\cap
(R(A)\cup W(A))
=
\varnothing.
$$

Equivalently, conflict exists if one writes something the other reads or writes.

---

**Hazard table**

| A access | B access | Safe concurrently? |
|---|---|---|
| Read | Read | Yes |
| Read | Write | No |
| Write | Read | No |
| Write | Write | No |

This is the fundamental scheduler rule.

---

**Hazards don't tell you direction**

Suppose:

```text
A writes Velocity
B reads Velocity
```

They conflict.

But should:

$$
A\rightarrow B
$$

or:

$$
B\rightarrow A?
$$

The access sets alone cannot tell you.

You still need:

- explicit before/after semantics,
- phase ordering,
- registration order as fallback,
- dataflow semantics.

Hazard detection determines **cannot overlap**.

It does not automatically determine **which result is desired**.

Important distinction.

---

**Structural access as a world-wide hazard**

A system that can:

```text
create entity
destroy entity
add component
remove component
```

can potentially invalidate queries/storage.

One conservative scheduler model marks structural access as:

$$
\text{exclusive world write}.
$$

Then no ordinary ECS iteration overlaps it.

Better model:

```text
systems queue structural commands
```

and actual structural flush happens at a dedicated barrier.

That preserves far more parallelism.

---

**ECS job partitioning**

One system may itself need many jobs.

Suppose:

$$
N=1,000,000
$$

entities.

Don't submit one job per entity.

Job scheduling overhead would dominate.

Partition into ranges.

---

**Grain size model**

Let:

$$
H=
\text{job scheduling overhead}
$$

and:

$$
C=
\text{cost per entity}.
$$

For grain size $G$, useful work per job:

$$
GC.
$$

We want:

$$
\boxed{
GC\gg H.
}
$$

If job costs 5 µs to schedule and 100 entities take 1 µs total, terrible.

If 10,000 entities take 100 µs, scheduling overhead is small.

Measure.

---

**Static versus dynamic partitioning**

Static:

```text
thread 0 gets fixed chunk range
thread 1 gets fixed chunk range
...
```

cheap but can become unbalanced if chunks differ in cost.

Dynamic job queue:

```text
many medium jobs
workers pull next available
```

provides load balancing.

Work stealing can improve this further.

---

**Archetype chunk = natural job unit**

One simple model:

```text
one job per chunk.
```

But if chunks are tiny or system work cheap, overhead may be excessive.

Batch several chunks per job.

Conversely, one huge chunk with expensive per-entity work may be split by row range.

Again, chunks are storage units, not necessarily sacred scheduling units.

---

**False sharing**

This is one of the most misunderstood multithreading problems.

Suppose cache line contains:

```text
x0 x1 x2 x3 x4 x5 x6 x7
```

Thread A writes x0.

Thread B writes x1.

They are completely different variables.

There is no logical data race.

But both variables occupy the same cache line.

Cache coherence operates at cache-line granularity.

The line repeatedly migrates between cores.

That is false sharing. Intel's optimization guidance explicitly warns that independently modified data on the same cache line can cause costly coherence traffic between cores. [Intel CDRD](https://cdrdv2-public.intel.com/814198/248966-Optimization-Reference-Manual-V1-049.pdf?utm_source=chatgpt.com)

---

**Terrible ECS partitioning**

Do not do:

```text
thread 0 → entities 0,2,4,6,8...
thread 1 → entities 1,3,5,7,9...
```

if both write a dense component array.

Adjacent elements share cache lines.

Threads ping-pong those lines constantly.

---

**Better partitioning**

Use contiguous ranges:

```text
thread 0:
0 ... 2499

thread 1:
2500 ... 4999

thread 2:
5000 ... 7499

thread 3:
7500 ... 9999
```

Now only the boundary lines risk sharing.

Most cache lines belong exclusively to one thread during the job.

Chunk-level assignment naturally encourages this.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/15 - Sparse-set components and generational entity handles|Previous chapter]] · [[Chapter Edition/03 - Game Engine/17 - ECS performance through cache and scheduling|Next chapter]]
