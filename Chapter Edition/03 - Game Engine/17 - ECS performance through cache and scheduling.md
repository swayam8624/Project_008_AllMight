# 17 — ECS performance through cache and scheduling

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/16 - Archetypes, chunks, relationships, and transforms|Previous chapter]] · [[Chapter Edition/03 - Game Engine/18 - ECS contracts, diagnostics, and GPU handoff|Next chapter]]

Data layout affects both memory traffic and parallel ownership, not just the number of component lookups. This chapter connects false sharing, synchronization, and diagnostics to the query patterns an ECS must serve.

**By the end:** Explain how cache-line ownership and scheduling influence an ECS workload, and propose measurements that isolate the bottleneck.

**Recall question:** How can two workers update different entities yet still contend on one cache line, and what layout or partition change would test that cause?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 34532–38364; source-span SHA256 ec2be08410e1f6dee1325c54058b84fd0ee1d92798faa933c75183d8f1c52df6. -->

<!-- CHAPTER-SOURCE-BEGIN -->
**False sharing in metadata**

Even if component data is perfectly partitioned, metadata can suffer.

Bad:

```cpp
struct WorkerStats
{
    std::atomic<uint64_t> Count[8];
};
```

If counters sit on one line, every worker invalidates the others' cache line.

Better:

```cpp
struct alignas(64) WorkerCounter
{
    std::uint64_t Value = 0;
};
```

then one per worker, followed by a reduction.

The exact destructive-interference size is architecture/platform dependent, but separating heavily written per-thread state is the principle.

---

**Read-only sharing is fine**

False sharing requires modifications.

Many threads reading the same cache line is generally not the same problem.

So:

```text
immutable configuration
read-only Transform data
constant tables
```

can be broadly shared.

The danger is write ownership bouncing.

---

**Prefetch and sequential iteration**

Modern CPUs are extremely good at recognizing sequential access.

Loop:

```cpp
for (std::size_t i = 0; i < count; ++i)
{
    position[i] += velocity[i] * dt;
}
```

has predictable streams:

```text
position[i]
velocity[i]
```

Hardware prefetchers can fetch future cache lines before they are demanded.

This is the payoff of dense arrays.

---

**Pointer chasing defeats prefetch**

Compare:

```text
dense:
Component[0]
Component[1]
Component[2]
...
```

with:

```text
vector<Component*>
    ↓
random heap object
    ↓
random heap object
    ↓
random heap object
```

The latter has poor spatial predictability.

Each pointer may stall waiting for another cache miss.

This is why ECS data layout can outperform a traditional object hierarchy even when algorithmic big-O is identical.

---

**Sparse-set query locality versus archetype locality**

Sparse-set join:

```text
iterate Health dense array
      ↓
lookup Sparse<Transform>[entity]
      ↓
jump to Transform dense slot
      ↓
lookup Sparse<Velocity>[entity]
      ↓
jump to Velocity dense slot
```

Each component pool is individually dense, but cross-component access order may be poorly correlated.

Archetype chunk:

```text
Transform[row]
Velocity[row]
Health[row]
```

all use the same row.

This produces excellent parallel sequential streams.

That is a core archetype advantage.

---

**Software prefetch**

You *can* manually prefetch:

```cpp
__builtin_prefetch(&components[i + distance]);
```

on compilers that support it.

Do not reach for this first.

Sequential loops already benefit heavily from hardware prefetch.

Manual prefetch is mainly useful for irregular-but-predictable access patterns where profiling demonstrates memory stalls.

Poorly chosen prefetch can:

- waste bandwidth,
- evict useful lines,
- arrive too early,
- arrive too late.

Data layout is more important than sprinkling prefetch intrinsics.

---

**System scheduling example for a real frame**

Consider these systems:

```text
Input
CharacterController
AI
Animation
PhysicsPreSync
PhysicsStep
CollisionEvents
PhysicsPostSync
TransformHierarchy
RenderExtraction
AudioExtraction
```

A possible graph:

```text
Input ────────┐
              v
      CharacterController ──┐
                            v
AI --------------------> PhysicsPreSync
                            |
                            v
                       PhysicsStep
                            |
                  +---------+---------+
                  |                   |
                  v                   v
           CollisionEvents     PhysicsPostSync
                                      |
                                      v
                              TransformHierarchy
                              /               \
                             v                 v
                      RenderExtraction    AudioExtraction

Animation --------------------^
```

Independent nodes can run in parallel when access declarations allow it.

This is where ECS stops being merely a storage library and becomes a **runtime scheduler architecture**.

---

**Read/write declaration example**

Imagine:

```cpp
SystemDescriptor movement
{
    .Reads  = { Velocity },
    .Writes = { Transform }
};

SystemDescriptor renderer
{
    .Reads  =
    {
        Transform,
        Renderable
    }
};

SystemDescriptor ai
{
    .Reads  = { Transform },
    .Writes = { Velocity }
};
```

Conflicts:

```text
AI → Movement
```

because AI writes Velocity that Movement reads.

And:

```text
Movement → Renderer
```

because Movement writes Transform that renderer reads.

AI and renderer both read Transform, but renderer also depends indirectly on movement.

The resulting DAG emerges from semantic and hazard edges together.

---

**Kairo's current Scene versus ECS runtime**

This distinction is crucial to your engine.

Current `EngineCore::Scene` owns an authored record approximately like:

```text
Record
├── Name
├── Transform
├── Settings
├── Parent
├── Children
├── optional MeshRenderer
├── optional SceneInstance
├── optional Camera
├── optional Light
├── optional AudioEmitter
├── optional AudioListener
├── optional Environment
├── optional Logic
├── optional RigidBody
└── optional Collider
```

That is effectively an authored **entity record / AoS-like scene representation**.

It prioritizes:

- serialization,
- editor semantics,
- stable scene IDs,
- explicit hierarchy,
- validation.

That is not the same job as runtime ECS storage.

---

**Do not prematurely delete the authored Scene**

A strong architecture can be:

```text
Authoring Scene
    |
    | extraction / bridge
    v
Runtime ECS
    |
    +------> PhysicsWorld
    |
    +------> RenderScene
    |
    +------> Audio
```

The authored scene remains understandable and serializable.

The runtime ECS becomes optimized for simulation.

This is often superior to making the editor directly manipulate low-level chunk storage.

---

**Scene/ECS identity bridge**

Persistent scene identity:

```text
Scene Entity ID
```

Runtime identity:

```text
ECS Entity(Index,Generation)
```

Keep a bridge:

```text
SceneID → ECS Entity
ECS Entity → SceneID
```

Then:

- serialization uses SceneID,
- simulation uses ECS Entity,
- picking may use SceneID,
- physics maps ECS Entity → BodyID,
- rendering extracts from ECS.

Each subsystem gets identity suited to its lifetime.

---

**Current deferred runtime scene mutations**

Interestingly, `EngineCore::RuntimeSceneCommandBuffer` already implements several ideas from Pass 10 for the authored Scene:

- pending entity tokens,
- FIFO commands,
- deferred create/destroy,
- deferred parenting,
- deferred tags,
- explicit commit.

Its documentation says commands are applied only at synchronization points so callbacks/physics iteration do not invalidate scene traversal.

That is a very good architectural precedent.

But it is a **Scene command buffer**, not yet the `KairoECS` structural command buffer discussed in Pass 10.

The concept can be reused.

---

**Current runtime physics event design**

Your PhysicsWorld has:

```cpp
enum class PhysicsContactEventType
{
    Begin,
    Stay,
    End
};
```

and records:

```text
BodyA
BodyB
ColliderA
ColliderB
Trigger
Response
Type
```

This is already a strong subsystem-neutral event representation.

The next bridge should translate those runtime physics IDs into ECS entities.

Do not make gameplay search all bodies to discover their owner.

---

**Current runtime job design**

Your `JobSystem` has worker threads and a queue but deliberately does not own:

```text
renderer
physics
scene objects
```

That is good.

Build ECS scheduling as:

```text
SystemGraph
     ↓
ready systems
     ↓
JobSystem.Submit(...)
     ↓
barriers
     ↓
next dependency wave
```

rather than stuffing ECS-specific logic into the generic thread pool.

---

**A production ECS scheduler descriptor**

A useful future design:

```cpp
struct ECSSystemDescriptor
{
    SystemID ID;
    std::string Name;

    std::vector<ComponentTypeID> Reads;
    std::vector<ComponentTypeID> Writes;

    std::vector<ResourceTypeID> ResourceReads;
    std::vector<ResourceTypeID> ResourceWrites;

    std::vector<SystemID> Before;
    std::vector<SystemID> After;

    TickDomain Domain;

    bool MayQueueStructuralChanges = false;
};
```

where:

```cpp
enum class TickDomain
{
    Fixed,
    Variable,
    RenderExtraction
};
```

Now the scheduler has enough information to reason about execution.

---

**Hazard function**

```cpp
bool Intersects(
    std::span<const ComponentTypeID> a,
    std::span<const ComponentTypeID> b);

bool Conflicts(
    const ECSSystemDescriptor& a,
    const ECSSystemDescriptor& b)
{
    // A writes something B reads.
    if (Intersects(a.Writes, b.Reads))
        return true;

    // A writes something B writes.
    if (Intersects(a.Writes, b.Writes))
        return true;

    // B writes something A reads.
    if (Intersects(b.Writes, a.Reads))
        return true;

    // Repeat for world resources.
    if (Intersects(
            a.ResourceWrites,
            b.ResourceReads))
        return true;

    if (Intersects(
            a.ResourceWrites,
            b.ResourceWrites))
        return true;

    if (Intersects(
            b.ResourceWrites,
            a.ResourceReads))
        return true;

    return false;
}
```

This is the core static hazard detector.

---

**But scheduler must not over-serialize everything**

Suppose two systems both write Transform but to provably disjoint entity groups:

```text
PlayerTransformSystem
EnemyTransformSystem
```

A component-type-only hazard model says conflict.

That is conservative but safe.

Later you can expose partition guarantees:

```text
query filters
archetype ownership
entity ranges
exclusive tags
```

to recover parallelism.

Start conservative.

Correct concurrency is far more valuable than theoretical maximal parallelism.

---

**Chunk change versions**

Archetype chunks enable a powerful optimization.

Suppose system writes Transform in chunk C.

Set:

```text
C.TransformVersion = WorldVersion.
```

Later renderer asks:

> Did Transform change since my last update version?

If not, skip work.

This is chunk-level change detection.

It cannot prove which individual entity changed, only:

$$
\text{something in this column/chunk may have changed}.
$$

But it is extremely cheap.

This mechanism can accelerate:

- transform extraction,
- GPU uploads,
- replication,
- bounds updates,
- renderer proxy updates.

---

**Previous/current transform and physics sync pipeline**

A robust fixed-tick flow:

```text
Before fixed simulation:
PreviousTransform = CurrentTransform

Gameplay writes inputs/forces
        ↓
Physics Step
        ↓
physics body states
        ↓
PhysicsToTransform system
        ↓
CurrentTransform

Render:
alpha = accumulator / fixedDt
        ↓
interpolate Previous/Current
        ↓
RenderTransform
```

Static/kinematic entities may follow different authority rules.

---

**Transform authority must be explicit**

Physics body modes:

**Static**

Scene/ECS Transform drives physics initialization.

No runtime physics integration.

**Kinematic**

Gameplay Transform generally drives physics.

Physics computes contacts but does not freely integrate body from forces.

**Dynamic**

Physics owns world motion.

After step:

```text
Physics → Transform.
```

If gameplay simultaneously writes Transform for a dynamic body, you have two authorities.

That must be defined.

Otherwise systems fight every frame.

---

**Render interpolation and physics authority**

Dynamic physics entity may therefore have:

```text
PhysicsPreviousPose
PhysicsCurrentPose
RenderInterpolatedPose
```

rather than overwriting authored local transform at arbitrary render cadence.

This avoids mixing simulation state and render state.

Again:

$$
\boxed{
\text{one logical object can legitimately have several state representations for different domains}.
}
$$

---

**Input tick timing**

Raw input is usually sampled once per rendered frame.

But fixed simulation may execute:

```text
0 fixed ticks
1 fixed tick
2 fixed ticks
...
```

during that frame.

"Pressed this frame" events require care.

If JumpPressed is consumed by every fixed tick, two substeps could jump twice.

A robust input-command layer needs semantics like:

```text
continuous:
Move axis
Look rate

edge-triggered:
JumpPressed
FirePressed
```

with controlled consumption across fixed steps.

---

**Collision event timing**

Similarly:

```text
Begin collision
```

should normally be delivered once for a contact transition, not once per renderer frame.

PhysicsWorld already maintains Begin/Stay/End around simulation steps.

So collision event domain belongs naturally to fixed simulation time.

Rendering can observe resulting state later.

---

**Event queue lifetime**

Never put references into events that may outlive the producing storage.

Bad:

```cpp
struct CollisionEvent
{
    const ContactManifold& Contact;
};
```

if manifold vector is rebuilt next physics step.

Better:

```cpp
struct CollisionEvent
{
    Entity A;
    Entity B;
    Vec3 Point;
    Vec3 Normal;
    float Impulse;
};
```

or an immutable stable event record.

Events should own the data required by their lifetime.

---

**Render-picking lifetime**

Same principle.

Do not map GPU ObjectID to:

```cpp
Transform*
```

because component address may migrate before readback.

Map to stable identity:

```text
SceneID
or
generational Entity
```

and validate after readback.

Identity bridges repeatedly solve lifetime separation.

---

**False-sharing example with ECS counters**

Suppose every worker tracks:

```cpp
struct Stats
{
    uint64_t Processed;
};

Stats stats[8];
```

`sizeof(Stats)=8`.

Eight counters may fit in one 64-byte cache line.

Eight cores increment different counters.

The line bounces constantly.

Instead:

```cpp
struct alignas(64) WorkerStats
{
    uint64_t Processed = 0;
};
```

then reduce afterward.

This is a textbook example where logical independence is not physical independence.

---

**False sharing with Transform columns**

Suppose Transform is 48 bytes.

Cache line 64 bytes.

Rows can straddle cache-line boundaries.

If job split happens at arbitrary row:

```text
worker A ends at row 100
worker B starts row 101
```

one or two boundary cache lines can be shared.

That's usually fine.

If workers alternate rows, almost every line is shared.

So **contiguous partitioning** matters much more than perfect cache-line-aligned boundaries.

---

**Prefetch and system fusion**

There is another performance consideration.

Suppose:

```text
System A:
read Velocity
write Position

System B immediately after:
read Position
write Bounds
```

Position written by A may still be hot in cache when B executes.

That can be good.

But combining every system into one giant loop destroys modularity and scheduling flexibility.

Sometimes system fusion improves locality.

Sometimes separation enables parallelism.

This is another benchmark decision, not a dogma.

---

**Archetype fragmentation**

Suppose entities differ by tiny structural flags:

```text
A = Transform + Renderable
B = Transform + Renderable + Selected
C = Transform + Renderable + Highlighted
D = Transform + Renderable + Selected + Highlighted
```

Now one logical population is split across four archetypes.

Queries must visit more chunks.

Small chunks waste capacity.

This is archetype fragmentation.

Watch metrics:

$$
\text{archetype count}
$$

$$
\text{chunk count}
$$

$$
\text{average chunk occupancy}
$$

$$
\text{wasted bytes}.
$$

Production ECS tools expose archetype/chunk occupancy for exactly this reason. Unity's Archetypes window, for example, reports chunk counts, chunk capacity and unused entity capacity per archetype. [Unity User Manual](https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/editor-archetypes-window.html?utm_source=chatgpt.com)

---

**Chunk occupancy**

Define:

$$
Occupancy=
\frac{LiveEntities}
{TotalChunkCapacity}.
$$

Suppose archetype has ten chunks each capacity 256:

$$
capacity=2560.
$$

But only 900 entities:

$$
Occupancy=
900/2560
\approx35\%.
$$

Memory locality benefits are being squandered.

Structural churn/shared-value grouping/tag explosion can cause this.

---

**Sparse-set fragmentation is different**

Sparse sets do not have archetype fragmentation.

Each component type has one packed component array.

But cross-component query locality may be poorer.

So the trade is:

```text
Sparse set:
excellent per-component density
less guaranteed multi-component alignment

Archetype:
excellent multi-component alignment
possible population fragmentation across signatures
```

That is the actual nuanced comparison.

---

**Kairo recommendation for now**

Given current `KairoECS`, I would **not** replace the sparse-set core immediately.

Instead:

```text
Stage 1
keep sparse sets
add generic queries
add command buffers
add scheduling metadata

Stage 2
benchmark real engine systems

Stage 3
if multi-component traversal dominates:
prototype archetype/chunk backend

Stage 4
compare objectively
```

You need evidence from your engine workloads before paying archetype complexity.

---

**Good archetype prototype workload**

Build two storage implementations:

**Sparse set**

```text
Transform pool
Velocity pool
Health pool
```

**Archetype**

```text
{Transform,Velocity,Health} chunks
```

Benchmark:

$$
1,000,000
$$

entities.

Run:

```text
integrate transforms
damage/health iteration
component add/remove churn
destroy/create churn
```

Measure:

```text
iteration ns/entity
structural mutation ns/entity
cache misses
bytes/entity
```

Then you have data rather than architecture fashion.

---

**Repository audit — M087/M088**

Current implementation status after inspecting KairoECS, KairoEngineCore, KairoPhysicsEngine and KairoRenderer:

| Concept | Current status |
|---|---|
| Sparse-set ECS | Implemented |
| Archetype storage | Not currently in KairoECS |
| Archetype chunks | Not implemented |
| Archetype migration | Not implemented |
| AoS authored Scene records | Present in EngineCore Scene |
| SoA component pools | Present at KairoECS component level |
| Hot/cold runtime extraction | Partly present architecturally |
| Zero-size ECS tags | Not in KairoECS |
| Authored string tags | Implemented |
| World resources | Several systems naturally act as resources |
| Parent/child Scene hierarchy | Implemented |
| Cycle rejection | Implemented |
| World-transform hierarchy calculation | Implemented on demand |
| Dirty transform propagation | Not found in inspected ECS/Scene |
| Previous/current ECS transforms | Not found as a general transform system |
| Mesh renderer authored component | Implemented |
| Rigid body component | Implemented |
| Collider component | Implemented |
| Light component | Implemented |
| Camera component | Implemented |
| Logic/script authoring component | Implemented |
| Raw input state | Implemented as engine-level state |
| ECS input-intent component | Not generalised yet |
| Runtime physics IDs | Implemented in PhysicsWorld |
| Entity↔physics bridge | Architectural next step |
| Renderer ObjectID | Implemented |
| Physics Begin/Stay/End contacts | Implemented |
| Generic ECS event queue | Not yet a dedicated KairoECS layer |
| Deferred Scene mutations | Implemented in EngineCore |
| Fixed physics stepping | Implemented |
| Generic thread pool | Implemented |
| ECS dependency DAG | Not currently implemented |
| ECS access declarations | Not implemented |
| Automatic hazard detection | Not implemented |
| ECS topological scheduler | Not implemented |
| Parallel chunk iteration | Not applicable until chunk backend exists |

---

**Reconstruction challenge — archetype ECS**

From memory, derive this chain:

```text
Entity has signature S
      ↓
lookup Archetype(S)
      ↓
Archetype owns chunks
      ↓
chunk owns component columns
      ↓
entity location = archetype + chunk + row
```

Adding component C:

```text
S
 ↓
S ∪ {C}
 ↓
destination archetype
 ↓
allocate destination row
 ↓
move common columns
 ↓
construct C
 ↓
update entity location
 ↓
swap-remove source row
 ↓
repair moved source entity location
```

If you cannot explain all of those steps and why each is needed, archetype ECS is not yet mastered.

---

**Reconstruction challenge — sparse set versus archetype**

You should be able to defend both positions.

Sparse set:

$$
\text{cheap structural component changes}
+
\text{excellent independent component density}
+
\text{O(1) lookup}
$$

but:

$$
\text{multi-component queries require membership lookups}.
$$

Archetype:

$$
\text{excellent multi-component iteration}
+
\text{query matching at archetype granularity}
+
\text{natural chunk jobs}
$$

but:

$$
\text{component add/remove = entity migration}.
$$

There is no correct answer without workload.

---

**Reconstruction challenge — transform hierarchy**

Given:

```text
A root
└── B
    └── C
```

derive:

$$
W_A=L_A
$$

$$
W_B=W_AL_B
$$

$$
W_C=W_BL_C.
$$

Then explain why parent must execute before child.

Then explain two dirty models:

```text
eager subtree dirty marking
```

versus:

```text
lazy parent-version comparison.
```

Then explain fixed-tick interpolation:

$$
T_r=
Interpolate(
T_{prev},
T_{curr},
\alpha).
$$

That completes M087 conceptually.

---

**Reconstruction challenge — engine subsystem bridge**

You should be able to trace one entity through the whole engine:

```text
SceneID 57
    ↓
runtime ECS Entity {231,4}
    ↓
RigidBodyComponent
ColliderComponent
Transform
Renderable
    ↓
physics bridge:
Entity {231,4} ↔ BodyID 81
    ↓
PhysicsContactEvent Body81/Body99
    ↓
BodyID→Entity translation
    ↓
ECS CollisionEvent
    ↓
gameplay
```

Simultaneously:

```text
Entity {231,4}
    ↓
Renderable + Transform
    ↓
render extraction
    ↓
MeshDraw
ObjectID
    ↓
GPU picking
    ↓
ID→entity lookup
```

If those identity domains are kept separate, the engine remains robust.

---

**Reconstruction challenge — scheduler**

Given:

```text
A reads Position, writes Velocity
B reads Velocity, writes Transform
C reads Transform
D reads Position
```

You should immediately identify:

$$
A\rightarrow B
$$

because A writes what B reads,

and:

$$
B\rightarrow C
$$

because B writes what C reads.

A and D can potentially run concurrently if no hidden resource dependencies exist.

Then topologically schedule:

```text
Wave 0: A, D
Wave 1: B
Wave 2: C
```

That is M088's central scheduling skill.

---

**Reconstruction challenge — false sharing**

Explain why this can be race-free but slow:

```text
core 0 writes values[0]
core 1 writes values[1]
```

when both occupy one cache line.

Then explain why:

```text
core 0 writes [0..999]
core 1 writes [1000..1999]
```

usually behaves far better.

If you understand the difference between:

$$
\text{data race}
$$

and:

$$
\text{cache-coherence contention},
$$

you understand false sharing.

---

**Exact coverage**

| Part | Topic | Covered |
|---:|---|---|
| 1435 | Archetype ECS concept | ✓ |
| 1436 | Archetype signatures | ✓ |
| 1437 | Archetype chunks | ✓ |
| 1438 | Entity movement between archetypes | ✓ |
| 1439 | Chunk capacity and alignment | ✓ |
| 1440 | Columnar component storage | ✓ |
| 1441 | Sparse-set versus archetype trade-offs | ✓ |
| 1442 | AoS component data | ✓ |
| 1443 | SoA component data | ✓ |
| 1444 | Hot-and-cold component splitting | ✓ |
| 1445 | Tag components | ✓ |
| 1446 | Singleton/resource components | ✓ |
| 1447 | Relationship components | ✓ |
| 1448 | Parent-child relationships | ✓ |
| 1449 | Scene hierarchy transforms | ✓ |
| 1450 | Dirty transform propagation | ✓ |
| 1451 | Previous/current transform snapshots | ✓ |
| 1452 | Renderable component | ✓ |
| 1453 | Physics body component | ✓ |
| 1454 | Collider component | ✓ |
| 1455 | Light component | ✓ |
| 1456 | Camera component | ✓ |
| 1457 | Script/behavior component | ✓ |
| 1458 | Input component/state | ✓ |
| 1459 | Entity-to-physics mapping | ✓ |
| 1460 | Entity-to-render-object mapping | ✓ |
| 1461 | Stable picking IDs | ✓ |
| 1462 | ECS event queues | ✓ |
| 1463 | Collision-event delivery | ✓ |
| 1464 | Begin-play lifecycle | ✓ |
| 1465 | Fixed tick vs render tick | ✓ |
| 1466 | System ordering dependencies | ✓ |
| 1467 | System dependency graphs | ✓ |
| 1468 | Topological scheduling | ✓ |
| 1469 | Parallel-safe ECS iteration | ✓ |
| 1470 | Read-only vs writable declarations | ✓ |
| 1471 | Data-hazard detection | ✓ |
| 1472 | ECS job partitioning | ✓ |
| 1473 | False sharing | ✓ |
| 1474 | Prefetch/sequential iteration | ✓ |


The architecture you should now hold mentally is no longer merely:

$$
Entity\rightarrow Component.
$$

It is:

$$
\boxed{
\begin{aligned}
&\text{persistent scene identity}\\
&\qquad\downarrow\\
&\text{generational runtime entity}\\
&\qquad\downarrow\\
&\text{sparse pool or archetype/chunk storage}\\
&\qquad\downarrow\\
&\text{system queries}\\
&\qquad\downarrow\\
&\text{read/write declarations}\\
&\qquad\downarrow\\
&\text{dependency DAG}\\
&\qquad\downarrow\\
&\text{parallel jobs}\\
&\qquad\downarrow\\
&\text{renderer / physics / audio bridges}.
\end{aligned}
}
$$

And throughout the whole chain, **identity, storage location, subsystem handles and persistent IDs are deliberately separate things**. That separation is what lets data move freely without destroying correctness.

**Next: Pass 12 — M089 + M090, Parts 1475–1500.** That finishes the ECS phase with the ugly production details that toy ECS tutorials skip: **non-trivial C++ component lifetime, move-only components, serialization, entity-reference remapping, schema migration, reflection/editor inspection, undo/redo, prefab data, scene→ECS loading, cloning, network/determinism concerns, ECS diagnostics, dangling-pointer detection, million-entity memory budgets, complexity benchmarking and finally implementing/defending the entire generational sparse-set ECS from a blank file.**


### Component Lifetime, Move-Only Data, Serialization, Entity Reference Remapping, Schema Evolution, Reflection, Inspector Editing, Undo/Redo, Prefabs, Scene↔ECS Conversion, Cloning, Network Identity, Determinism, ECS Diagnostics, Memory Budgets, Complexity, Invalidation Contracts, Reuse Testing, Benchmarking, and Building the Whole Sparse-Set ECS From Scratch

This pass **finishes Phase 15**.


Passes 10 and 11 gave us the structural model:

$$
Entity
\rightarrow
ComponentStorage
\rightarrow
Systems.
$$

Pass 12 asks what happens when those components are **real C++ objects**, the world must survive save/load/editor/network boundaries, and somebody asks you:

> “Why did you choose this exact data structure, what invalidates what, what is the memory cost at one million entities, and can you prove stale handles never alias?”

That is where an ECS stops being a coding exercise.

---

### PRODUCTION COMPONENT LIFETIME, SERIALIZATION, REFLECTION, SCENES AND IDENTITY

---

**Component construction and destruction**

So far we have often used components like:

```cpp
struct Position
{
    float x;
    float y;
    float z;
};
```

For such a type, storage looks deceptively simple.

Allocate bytes.

Write three floats.

Eventually release bytes.

But C++ components are not just byte ranges.

Every object has a **lifetime**.

For an object $T$, there is a point at which suitably aligned storage exists but **no T object exists yet**, then construction begins its lifetime, and eventually destruction ends that lifetime. C++ object lifetime and storage duration are related but not identical. [Cppreference](https://en.cppreference.com/cpp/language/lifetime?utm_source=chatgpt.com)

This distinction becomes critical the moment an ECS owns raw chunk storage.

---

**Storage is not an object**

Suppose an archetype chunk allocates:

```cpp
std::byte memory[16384];
```

and reserves 64 bytes at some offset for a component.

Those 64 bytes do **not** magically contain a valid:

```cpp
std::string
```

just because:

```text
sizeof(std::string) <= 64.
```

You must begin the object's lifetime.

For ordinary dynamic construction:

```cpp
std::construct_at(
    reinterpret_cast<T*>(address),
    constructorArguments...);
```

Later:

```cpp
std::destroy_at(
    reinterpret_cast<T*>(address));
```

The sequence is:

$$
\boxed{
Storage
\rightarrow
Construction
\rightarrow
Live\ T
\rightarrow
Destruction
\rightarrow
Storage.
}
$$

You cannot legally treat arbitrary bytes as a fully initialized nontrivial T.

---

**Sparse sets hide much of this for you**

Your current Kairo sparse set stores:

```cpp
std::vector<Component> m_Components;
```

That is important.

`std::vector<T>` already manages:

- allocation,
- alignment,
- construction,
- move/copy during reallocation,
- destruction,
- exception cleanup.

So:

```cpp
m_Components.emplace_back(...);
```

constructs a real Component.

And:

```cpp
m_Components.pop_back();
```

destroys one.

This is a major simplification compared with writing a raw archetype chunk allocator.

---

**Archetype chunks force you to become a miniature `std::vector`**

If you later implement raw archetype chunks, **you inherit responsibility for object lifetime**.

An archetype column cannot merely know:

```text
component size = 48
alignment = 16
```

It also needs operations.

Conceptually:

```cpp
struct ComponentOperations
{
    std::size_t Size;
    std::size_t Alignment;

    void (*DefaultConstruct)(void*);
    void (*MoveConstruct)(void*, void*);
    void (*Destroy)(void*);

    // Depending on policy:
    void (*CopyConstruct)(void*, const void*);
};
```

Then moving an entity from one archetype to another becomes type-erased lifetime management.

---

**Type-erased construction**

For type T:

```cpp
template<typename T>
ComponentOperations MakeOperations()
{
    return
    {
        .Size = sizeof(T),
        .Alignment = alignof(T),

        .DefaultConstruct =
            [](void* destination)
            {
                std::construct_at(
                    static_cast<T*>(destination));
            },

        .MoveConstruct =
            [](void* destination,
               void* source)
            {
                std::construct_at(
                    static_cast<T*>(destination),
                    std::move(
                        *static_cast<T*>(source)));
            },

        .Destroy =
            [](void* object)
            {
                std::destroy_at(
                    static_cast<T*>(object));
            }
    };
}
```

Now your ECS can manipulate unknown component types through their descriptors.

This is the runtime type-erasure layer archetype storage ultimately requires.

---

**Destruction order matters**

Imagine entity owns:

```text
Transform
ScriptRuntime
AudioSource
PhysicsProxy
```

If some component destructors refer to other runtime state, arbitrary destruction ordering can become observable.

Ideally components should not depend heavily on one another in destructors.

Instead subsystem teardown should occur in explicit systems:

```text
pre-destroy phase
    ↓
release physics proxy
release script instance
release renderer proxy
    ↓
destroy ECS components
```

This prevents hidden destructor ordering dependencies.

That is another data-oriented principle:

$$
\boxed{
\text{behavioral lifecycle should usually be explicit,
not buried in destructors}.
}
$$

---

**Component destruction on entity destruction**

For sparse-set Kairo:

```text
Destroy Entity
    ↓
for every storage:
    Remove(entity)
    ↓
component swap-remove
    ↓
component destructor eventually runs on removed tail object
    ↓
generation increments
    ↓
slot recycled
```

This ordering from Pass 10 remains important.

Runtime identity cannot be reused until old component objects have ceased to exist logically.

---

**Non-trivial C++ component types**

A trivial-looking component:

```cpp
struct Health
{
    float Value;
};
```

does not own resources.

A non-trivial component may contain:

```cpp
struct Inventory
{
    std::vector<Item> Items;
    std::string OwnerName;
};
```

or:

```cpp
struct ScriptRuntime
{
    std::shared_ptr<VM> Machine;
    std::vector<Value> Locals;
};
```

or:

```cpp
struct AudioEmitterRuntime
{
    AudioVoiceHandle Voice;
    std::string DebugName;
};
```

These have real construction/destruction semantics.

---

**Why raw `memcpy` becomes illegal or semantically wrong**

Suppose:

```cpp
std::string a = "Kairo";
```

and you byte-copy its object representation into some other location:

```cpp
std::memcpy(&b, &a, sizeof(std::string));
```

Now two string objects may contain internal pointers referring to the same allocation without correct ownership semantics.

Destruction can double-free.

The broader rule:

$$
\boxed{
\text{object relocation is not equivalent to byte copying
unless the type's semantics permit it}.
}
$$

Trivially copyable types can be handled much more aggressively.

Nontrivial types require constructors/move operations/destructors.

---

**Traits should affect storage policy**

A sophisticated ECS can branch:

```cpp
if constexpr (
    std::is_trivially_copyable_v<T>)
{
    // memcpy/memmove may be available
}
else
{
    // actual move construction/destruction
}
```

Likewise:

```cpp
std::is_trivially_destructible_v<T>
```

can eliminate destructor loops.

For a million POD-like physics components, skipping unnecessary lifetime callbacks can materially matter.

---

**Trivial relocation is broader than trivial copying conceptually**

Engines frequently want:

> “This type can safely be relocated from one storage address to another.”

That is not exactly the same question as:

```cpp
std::is_trivially_copyable_v<T>.
```

For example a type could own a heap allocation through a pointer and moving its raw bytes might be semantically acceptable if the old object's lifetime is appropriately ended—but standard C++ has historically made general relocatability subtler than engine programmers might wish.

Unless you establish a stronger engine-specific relocation contract, use actual move construction for nontrivial types.

---

**Your current Kairo removal contract has an important consequence**

Current swap-removal essentially performs:

```cpp
m_Components[dense] =
    std::move(m_Components[last]);
```

Therefore Component must support:

$$
\text{move assignment}.
$$

More precisely, because the current `Remove()` path is `noexcept`, a throwing move assignment can lead to:

$$
std::terminate.
$$

So the practical contract should be stated as something close to:

```cpp
static_assert(
    std::is_nothrow_move_assignable_v<Component>);
```

or Kairo should weaken its `noexcept` guarantee.

This is not academic.

It is an API contract that users of the ECS deserve to know.

---

**A stronger sparse-set implementation can move-construct instead**

One alternative storage model is raw/paged storage.

To remove row $d$:

```text
destroy object at d
move-construct object from last into d
destroy moved-from last
```

rather than move-assigning into an existing object.

Conceptually:

```cpp
std::destroy_at(destination);

std::construct_at(
    destination,
    std::move(*last));

std::destroy_at(last);
```

But exception safety becomes harder.

If move construction throws after destination destruction, you have a hole.

So once again:

$$
\boxed{
nothrow move construction
}
$$

is extremely valuable for ECS components.

---

**Nontrivial components can hide allocations**

Consider:

```cpp
struct Name
{
    std::string Value;
};
```

Dense ECS iteration over one million Names is technically contiguous in terms of `std::string` objects.

But each long string may point somewhere else in the heap.

So:

```text
dense component array
```

does not automatically imply:

```text
all logically relevant data is contiguous.
```

Hot components should avoid unnecessary dynamic allocations.

Cold authoring metadata can tolerate them much more readily.

---

**Move-only components**

Some engine objects should have exactly one owner.

Example:

```cpp
struct ScriptInstance
{
    std::unique_ptr<VirtualMachine> VM;
};
```

or:

```cpp
struct GPUUploadTicket
{
    std::unique_ptr<UploadPayload> Payload;
};
```

These are move-only.

```cpp
ScriptInstance(const ScriptInstance&) = delete;
ScriptInstance& operator=(
    const ScriptInstance&) = delete;
```

but move is allowed.

An ECS should not unnecessarily require components to be copyable.

---

**Sparse sets naturally support many move-only types**

`std::vector<T>` can hold move-only T.

Insertion:

```cpp
registry.Emplace<ScriptInstance>(
    entity,
    std::make_unique<VM>());
```

works if constructors and vector relocation requirements are satisfied.

Swap-removal also uses move semantics.

So a sparse-set ECS can be friendly to move-only components.

---

**But entity duplication becomes semantically ambiguous**

Suppose entity contains:

```text
Transform
Health
unique_ptr<ScriptVM>
```

Now user clicks:

> Duplicate Entity.

What should happen to `ScriptVM`?

You cannot copy the `unique_ptr`.

Possible semantics include:

$$
\text{clone VM state}
$$

or:

$$
\text{create fresh VM from script asset}
$$

or:

$$
\text{do not duplicate runtime-only component}.
$$

This means **component copyability is not the same as entity duplicability**.

A component metadata system should expose an explicit clone operation.

---

**Runtime-only components**

A clean classification is:

```text
persistent authored components
runtime-derived components
runtime-owned external handles
transient frame state
```

For example:

```text
LogicComponent:
persistent script asset reference

ScriptRuntimeComponent:
runtime VM instance
```

Save scenes should serialize the former.

Not the latter.

Duplication may copy the former and reconstruct the latter.

This distinction is already visible in Kairo: `LogicComponent` stores persistent document identity while VM instances are explicitly process-local.

---

**Component serialization**

Serialization means converting component state into a durable representation:

$$
Component
\rightarrow
SerializedData.
$$

Deserialization reverses:

$$
SerializedData
\rightarrow
Component.
$$

Do not serialize C++ object memory directly.

For:

```cpp
struct Health
{
    float Current;
    float Maximum;
};
```

you want semantic fields:

```text
health.current = 80
health.maximum = 100
```

not bytes at offsets 0 and 4 merely because today's compiler layout happens to look that way.

---

**Why raw struct dumps are fragile**

Raw dump depends on:

- compiler padding,
- endianness,
- ABI,
- enum representation,
- pointer size,
- alignment,
- implementation changes,
- member order.

Suppose:

```cpp
struct A
{
    bool Enabled;
    float Value;
};
```

may be:

```text
byte 0 bool
bytes 1–3 padding
bytes 4–7 float
```

A future compiler/configuration can differ.

Durable formats should store logical fields.

---

**Component serializer interface**

A reflection-independent design:

```cpp
struct ComponentSerializer
{
    void (*Serialize)(
        const void* object,
        Writer& writer);

    void (*Deserialize)(
        void* object,
        Reader& reader,
        std::uint32_t version);
};
```

Type-specific:

```cpp
void Serialize(
    const Transform& t,
    Writer& w)
{
    w.Field("position", t.Position);
    w.Field("rotation", t.Rotation);
    w.Field("scale", t.Scale);
}
```

This is explicit and robust.

---

**Reflection-driven serialization**

Once reflection metadata exists, generic serialization becomes possible.

If descriptor knows:

```text
Component: Camera
Properties:
vertical-fov
near-plane
far-plane
projection
...
```

generic serializer can enumerate them.

That reduces hand-written code.

But beware:

$$
\boxed{
\text{editor-visible property}
\neq
\text{necessarily serialized property}.
}
$$

Some values are derived, transient, editor-only or runtime-only.

Reflection metadata needs flags.

---

**Kairo's current scene serialization**

Your EngineCore already has durable scene serialization with explicit parsing, validation and versioning.

Current files use headers such as:

```text
kairo-scene 4
kairo-scene 5
```

rather than assuming C++ memory layout.

The serializer imposes input limits such as maximum scene size and entity count, validates numeric ranges, validates assets, and uses a temporary file followed by atomic replacement when saving.

That is exactly the kind of defensive scene I/O a production engine should have.

---

**Runtime ECS entities are explicitly not serialized in Kairo**

Your KairoECS README says the runtime ECS is process-local.

The SceneECS bridge carries:

```cpp
struct AuthoredEntityIdentity
{
    kairo::engine::Entity Authored;
};
```

beside the runtime ECS handle.

This distinction is crucial:

$$
\boxed{
\text{durable authored ID}
\neq
\text{runtime ECS slot/generation}.
}
$$

Excellent design.

---

**Entity-reference serialization**

Suppose:

```cpp
struct Target
{
    Entity TargetEntity;
};
```

Runtime value might be:

```text
Index = 142
Generation = 8
```

You cannot serialize that as the persistent reference.

On next run:

```text
{142,8}
```

may not exist at all.

Instead convert:

$$
RuntimeEntity
\rightarrow
PersistentEntityID.
$$

---

**Save-time remapping**

Suppose:

```text
Runtime {41,3} ↔ SceneID 1007
Runtime {93,8} ↔ SceneID 1008
```

Entity 41 targets runtime 93.

Serialized:

```text
entity 1007
target 1008
```

Not:

```text
target index 93 generation 8.
```

That is the correct durable relation.

---

**Load-time remapping**

Loading should generally happen in at least two phases.

First create entities:

```text
persistent ID 1007 -> runtime E1
persistent ID 1008 -> runtime E2
```

Then resolve references:

```text
1007.target = lookup(1008)
```

The key idea:

$$
\boxed{
\text{create identities first,
resolve cross-references afterward}.
}
$$

---

**Kairo already applies this pattern for parent references**

`SceneECSMirror::Extract()` uses two passes.

First pass:

```text
create all ECS entities
store authored→runtime mapping
```

Second pass:

```text
resolve hierarchy parent references
```

This means a child can reference a parent regardless of authored file/entity ordering.

That is exactly the general technique Part 1479 demands.

---

**Broken references**

What happens when serialized entity A references entity B but B is missing?

Options include:

```text
hard error
null reference
warning + null
create placeholder
```

The correct policy depends on the field.

A required parent reference may warrant an error.

An optional target may safely become null.

The serializer/reflection metadata can express:

```text
required
optional
weak reference
external scene reference
```

instead of treating every entity reference identically.

---

**Versioned component schemas**

Suppose Camera v1:

```cpp
struct CameraV1
{
    float Fov;
    float Near;
    float Far;
};
```

Later v2 adds:

```cpp
float Exposure;
```

Old scenes do not contain Exposure.

What should happen?

A schema version allows migration.

---

**Component versioning model**

Store:

```text
component Camera
schema 2
...
```

or version the enclosing scene format.

Then:

```cpp
switch (serializedVersion)
{
case 1:
    camera.Fov = read(...);
    camera.Near = read(...);
    camera.Far = read(...);
    camera.Exposure = 0.0f;
    break;

case 2:
    ...
    break;
}
```

The old format is interpreted intentionally.

---

**Why a global scene version is sometimes insufficient**

Suppose:

```text
Scene format = 17
```

but individual components evolve independently.

Changing Audio should not conceptually require migration logic for Physics.

Component-level schema versions can be cleaner:

```text
Transform schema 3
Camera schema 5
AudioEmitter schema 2
```

However, global scene versions are simpler and can still work very well for a tightly controlled engine.

Kairo currently uses whole scene-format versions.

---

**Migration chains versus direct migration**

Suppose you have:

```text
v1 → v2 → v3 → v4 → v5.
```

One model applies every migration sequentially.

Advantages: each migration is small.

Disadvantages: loading very old assets requires all historical migration code forever.

Alternative:

```text
v1 → current
v2 → current
v3 → current
...
```

More code duplication but fewer intermediate dependencies.

A common authoring pipeline eventually **re-saves/cooks old assets into current format**, allowing runtime to support only recent/current versions.

---

**Kairo's current V5 pattern**

Your V5 serializer is particularly interesting.

It delegates much of the previous format and then extends the result for newer fields such as audio components.

This is a practical evolutionary strategy:

$$
\text{reuse stable previous serializer}
+
\text{append/migrate new semantics}.
$$

It reduces unnecessary rewriting while keeping the version boundary explicit.

---

**Schema evolution rules**

When designing durable formats, these changes are usually comparatively easy:

```text
add optional field with default
rename with migration
split one field into two with conversion
change enum with explicit mapping
```

Harder:

```text
remove semantic information
change coordinate system
change identity model
merge/split entities
change units
```

Those require actual migration logic.

Never assume:

> “We'll just deserialize old bytes into the new struct.”

That is exactly what schema versioning exists to prevent.

---

**Reflection-driven component inspection**

Reflection answers:

> What type is this?

> What properties does it expose?

> How do I read/write them generically?

Kairo already has a substantial reflection layer.

For example, it registers descriptors for:

```text
Transform
Camera
Environment
Light
MeshRenderer
Logic
RigidBody
Collider
```

with human-readable names, categories, tooltips, numeric ranges, enum options and property read/write functions.

That is significantly beyond toy reflection.

---

**Property descriptor model**

Conceptually:

```cpp
struct PropertyDescriptor
{
    std::string Key;
    std::string DisplayName;

    PropertyValueKind Kind;

    PropertyValue (*Read)(const void*);
    void (*Write)(
        void*,
        const PropertyValue&);

    NumericRange Range;
    PropertyFlags Flags;
};
```

Now editor does not need:

```cpp
if component == Camera
    hardcode every camera field
```

It can inspect metadata.

---

**Reflection requires semantic adapters**

Consider:

```cpp
Quaternionf Rotation;
```

Generic bytes are not editor-friendly.

Kairo's reflection explicitly encodes/decodes quaternion/vector property values and normalizes rotation on write.

That is important.

Reflection should expose **semantic values**, not simply memory addresses.

Likewise asset handles become persistent reference strings/IDs.

---

**Reflection is not C++ RTTI**

`typeid(T)` gives some runtime type identity.

It does not give:

```text
DisplayName
fields
ranges
tooltip
asset target type
editor category
serialization policy
```

Engine reflection is a domain metadata system.

RTTI and reflection solve different problems.

---

**Editor property panels**

Given reflected component:

```text
Camera
```

editor can generate:

```text
Projection       [Perspective ▼]
Vertical FOV     [60.0       ]
Near Plane       [0.1        ]
Far Plane        [1000       ]
Exposure         [0.0        ]
Primary          [✓]
```

from metadata.

No hand-written Camera inspector is strictly required.

---

**Generic inspector algorithm**

Conceptually:

```cpp
for (const PropertyDescriptor& property
     : component.Properties)
{
    PropertyValue value =
        property.Read(componentPointer);

    PropertyValue edited =
        DrawEditorWidget(
            property,
            value);

    if (edited != value)
    {
        property.Write(
            componentPointer,
            edited);
    }
}
```

The widget depends on property kind.

---

**Widget mapping**

Examples:

```text
bool        → checkbox
float       → drag/slider
enum        → combo box
Vec3        → xyz editor
Quaternion  → Euler UI or quaternion editor
Color       → color picker
AssetRef    → asset selector
EntityRef   → scene/entity selector
string      → text box
```

Reflection metadata provides enough information to choose sensible editors.

---

**Validation after reflective writes**

Suppose user changes:

```text
NearPlane = 100
FarPlane  = 10
```

A generic property write may temporarily make Camera invalid.

You need a policy.

Options:

```text
validate each property immediately
edit temporary copy then validate/commit
allow transient invalid editor state
clamp through metadata
```

For complex invariants involving multiple fields, editing a temporary copy then validating is often safest.

---

**Editor writes should not bypass engine invariants**

Current Kairo components already expose:

```cpp
Validate()
```

for Camera, Light, Collider, etc.

A reflection inspector should not write arbitrary bytes and leave them invalid.

A strong flow:

```text
read component
    ↓
copy to candidate
    ↓
apply reflected property change
    ↓
candidate.Validate()
    ↓
commit candidate
```

If validation fails, editor can show an error without corrupting live state.

---

**Undo/redo for component edits**

Suppose user changes light intensity:

$$
1000\rightarrow5000.
$$

Undo must restore 1000.

Redo must restore 5000.

The simplest command stores:

```cpp
struct PropertyEditCommand
{
    PersistentEntityID Entity;
    ComponentSchemaID Component;
    PropertyID Property;

    PropertyValue Before;
    PropertyValue After;
};
```

---

**Execute, undo and redo**

```cpp
void Execute()
{
    Set(
        Entity,
        Component,
        Property,
        After);
}

void Undo()
{
    Set(
        Entity,
        Component,
        Property,
        Before);
}
```

Undo stack:

```text
A
B
C <- current
```

Undo:

```text
A
B <- current
C
```

A new edit after undo usually drops C and begins a new branch.

---

**Do not store raw component pointers in undo commands**

Bad:

```cpp
struct Command
{
    Transform* Target;
};
```

Component storage may move.

Entity may be destroyed and recreated.

Editor may reload scene.

Use stable authored identity:

```text
Scene Entity ID
+
Component schema ID
+
Property key
```

Then resolve on execute/undo.

This is yet another application of:

$$
\text{identity}\neq\text{address}.
$$

---

**Undo command merging**

Dragging a slider can generate:

```text
1000
1001
1002
...
5000
```

You do not want 4000 undo steps.

During one interactive edit transaction:

```text
BeginEdit
capture Before

many live updates

EndEdit
capture final After
push one command
```

This is essential for usable editors.

---

**Structural undo**

Changing a property is easy.

Undoing:

```text
Delete Entity
```

requires enough data to recreate:

- persistent ID,
- components,
- hierarchy,
- children policy,
- references,
- asset references.

Often the easiest representation is a serialized snapshot of the deleted subtree.

Undo deletion:

```text
deserialize snapshot
restore IDs/references
```

This directly connects undo to serialization.

---

**Prefab/template entities**

A prefab is reusable authored entity structure.

Example:

```text
EnemySoldier.prefab

Root
├── Transform
├── MeshRenderer
├── RigidBody
├── Collider
├── Health
├── Logic
└── Weapon child
```

Instantiate it 500 times without manually recreating the structure.

---

**Prefab is not merely copying an entity**

A useful prefab system needs:

$$
\text{template identity}
+
\text{instance identity}
+
\text{overrides}.
$$

Suppose prefab Health.Maximum changes:

$$
100\rightarrow120.
$$

Instances without overrides should inherit 120.

An instance with override:

$$
Maximum=200
$$

should remain 200.

Thus prefab instances are usually:

$$
\boxed{
Template
+
OverrideSet
}
$$

rather than frozen copies only.

---

**Prefab instance override model**

Conceptually:

```text
Prefab:
Health.Maximum = 100
Renderable.Material = SoldierMaterial

Instance:
override Health.Maximum = 200
```

Resolved value:

$$
Value(instance,p)=
\begin{cases}
Override(p),&p\text{ overridden}\\
Prefab(p),&otherwise
\end{cases}
$$

Nested prefabs add another layer.

This quickly becomes an asset/inheritance system.

---

**Prefab runtime extraction**

Do not make runtime ECS query template inheritance constantly.

At load/cook/instantiate time:

```text
Prefab
+ overrides
    ↓
resolved entity data
    ↓
runtime ECS components
```

Runtime wants concrete component values.

Authoring wants inheritance.

Separate those layers.

---

**Scene loading into an ECS**

Kairo already has a very clean model for this.

Authoring:

```text
EngineCore::Scene
```

Runtime:

```text
KairoECS::Registry.
```

Bridge:

```text
SceneECSMirror::Extract(Scene).
```

This is much better than serializing ECS dense indices.

---

**Kairo's two-pass scene extraction**

Current bridge performs:

**First pass**

For every authored entity:

```text
create runtime Entity
add AuthoredEntityIdentity
add RuntimeName
add RuntimeTransform
add RuntimeSceneState
record authored→runtime
```

**Second pass**

Resolve parent:

```text
authored parent ID
    ↓
m_byAuthored
    ↓
runtime parent Entity
```

This is exactly how cross-reference loading should work.

---

**Why extraction rather than aliasing is valuable**

The bridge test explicitly verifies:

```text
change authored scene name
```

does not automatically mutate runtime ECS.

Only:

```cpp
RefreshAuthoredState(...)
```

crosses the boundary.

That gives clear ownership:

$$
\boxed{
Scene = authored source of truth
}
$$

$$
\boxed{
ECS = extracted process-local runtime state.
}
$$

No accidental shared mutable state.

---

**Scene loading pipeline**

A more complete production pipeline becomes:

```text
scene file
   ↓
parse
   ↓
validate
   ↓
migrate schema
   ↓
create authored Scene
   ↓
resolve asset references
   ↓
extract runtime ECS
   ↓
instantiate runtime-only components
   ↓
create physics/render/audio bridges
   ↓
BeginPlay
```

Every arrow is an explicit boundary.

That is good architecture.

---

**Transactional loading**

Never partially mutate the live world while parsing.

Kairo follows this principle in several places.

Load into:

```text
candidate Scene
```

then, if everything succeeds:

```cpp
destination = std::move(candidate);
```

Similarly `RuntimeWorld::Commit()` applies commands to a copy and only replaces the live scene when all commands succeed.

That provides strong all-or-nothing behavior.

---

**Scene saving from an ECS**

This is trickier.

If authored Scene remains the source of truth, you generally should not save arbitrary runtime ECS directly.

Runtime may contain:

```text
PhysicsProxy
ScriptVM
InterpolatedTransform
NavigationAgentRuntime
GPUVisibilityState
```

that should never become authored scene data.

So saving needs an extraction policy:

$$
Runtime
\rightarrow
\text{persistent authored subset}.
$$

---

**Runtime-to-authoring sync**

Potential flow:

```text
for each runtime entity with AuthoredEntityIdentity:
    find authored scene entity

    copy approved editable state:
        Transform
        enabled
        selected gameplay properties

    ignore:
        BodyID
        RenderProxy
        ScriptVM
        transient events
```

Then serialize the authored Scene.

This whitelist policy is safer than dumping every component.

---

**Play mode editor semantics**

Editors often distinguish:

```text
Edit mode
Play mode
```

During play, runtime transform may change due to physics.

When play stops, should those changes persist?

Possible policy:

```text
discard all runtime changes
```

or:

```text
user explicitly Apply Runtime Changes
```

or:

```text
certain fields persist.
```

That is not purely serialization.

It is product/editor semantics.

But ECS architecture must permit it.

---

**Entity duplication**

Naive duplication:

```text
for every component on source:
    copy to destination
```

sounds easy.

But we immediately encounter categories:

```text
plain value component
persistent asset reference
entity reference
runtime handle
move-only runtime object
hierarchy relationship
unique identity
```

They cannot all be copied identically.

---

**Component clone metadata**

A production component descriptor might contain:

```cpp
enum class ClonePolicy
{
    CopyValue,
    Reinitialize,
    Skip,
    Custom
};
```

Examples:

```text
Transform       CopyValue
Health          CopyValue
MeshRenderer    CopyValue
PhysicsBodyID   Reinitialize/Skip
ScriptRuntime   Reinitialize
NetworkOwner    Custom
```

Duplication becomes metadata-driven.

---

**Entity cloning with remapped references**

Suppose hierarchy:

```text
A
├── B
└── C
```

and B contains:

```text
Target = C.
```

Duplicate subtree.

Create:

```text
A'
├── B'
└── C'
```

What should:

```text
B'.Target
```

be?

Usually:

$$
C'
$$

not original C.

This is **reference remapping**.

---

**Clone map**

First clone all identities:

```text
A → A'
B → B'
C → C'
```

Store:

```cpp
std::unordered_map<
    PersistentEntityID,
    PersistentEntityID
> remap;
```

Then clone components.

For every entity-reference field:

$$
ref'=
\begin{cases}
remap[ref],&ref\text{ inside cloned set}\\
ref,&ref\text{ external and policy preserves it}\\
null/error,&\text{depending policy}
\end{cases}
$$

---

**Two-pass cloning is the same pattern again**

Notice the repeating architecture:

**Scene load**

Create all entities → resolve references.

**Scene→ECS extraction**

Create all runtime entities → resolve parent.

**Subtree clone**

Create all clones → remap references.

These are all instances of:

$$
\boxed{
\text{identity establishment before relational resolution}.
}
$$

This is a powerful general engine pattern.

---

**Internal versus external references during clone**

Suppose duplicated Weapon points to shared asset/controller entity outside subtree.

Should it remap?

No, usually.

So clone algorithm must know whether target belongs to cloned closure.

If:

$$
target\in CloneSet,
$$

remap.

Otherwise preserve or clear depending on relationship semantics.

---

**Parent references during cloning**

Duplicating an entity whose parent is outside the cloned set needs a policy.

For editor "Duplicate":

```text
clone becomes sibling
```

so both original and duplicate share parent.

For "Duplicate subtree as prefab":

root may become detached.

Again, cloning is not one universal byte-copy operation.

---

**Network entity IDs versus local IDs**

Multiplayer introduces another identity namespace.

Client A runtime entity:

```text
{Index=81, Generation=3}
```

Client B representing the same game object:

```text
{Index=409, Generation=9}.
```

Therefore ECS handles cannot be network identity.

---

**Network identity model**

Server assigns:

```text
NetworkEntityID = 761283.
```

Each machine maintains:

$$
NetworkID
\leftrightarrow
LocalEntity.
$$

For client:

```text
761283 -> {409,9}
```

For server:

```text
761283 -> {81,3}.
```

Replication packets mention:

```text
761283
```

never process-local ECS slot IDs.

---

**Network ID lifetime**

Network IDs themselves need lifetime rules.

If IDs are reused rapidly, delayed packets can create another ABA problem.

Solutions include:

- monotonically increasing large IDs,
- epoch/generation,
- connection-local spawn sequence,
- UUID-like IDs,
- destroy acknowledgement before reuse.

The same identity lesson repeats at another scale.

---

**Mapping creation order**

Suppose packet says:

```text
Entity 500 references Entity 700.
```

but 700's spawn packet has not arrived.

You need:

```text
unresolved reference
```

or deferred packet application.

Network replication is effectively distributed entity-reference deserialization.

So much of Part 1479's logic reappears in networking.

---

**Deterministic entity iteration**

Suppose physics/gameplay depends on entity update order.

If iteration order changes between runs, floating-point accumulation and event order can diverge.

Sparse-set order is affected by:

```text
insertion
deletion
swap-remove.
```

Therefore:

$$
\boxed{
\text{dense order is not inherently stable semantic order}.
}
$$

---

**Swap-remove destroys ordering**

Start:

```text
A B C D
```

remove B:

```text
A D C.
```

So D now executes before C.

If gameplay behavior depended on:

```text
A B C D
```

it is broken.

The ECS is correct.

The gameplay assumption was wrong.

---

**Determinism requires an explicit contract**

Options include:

```text
sort by stable entity ID before order-sensitive work
```

or:

```text
maintain ordered storage
```

or:

```text
make system semantics order-independent
```

or:

```text
define deterministic swap-remove lifecycle.
```

The best solution is often to make most systems order-insensitive, then impose order only where necessary.

---

**Kairo authored Scene already gives deterministic iteration**

`Scene::Entities()` gathers IDs then sorts them in ascending stable scene-ID order.

That is excellent for:

- serialization,
- deterministic editor behavior,
- stable diff output.

KairoECS dense pool order is instead runtime-storage order.

Those differing goals justify differing iteration contracts.

---

**Floating-point determinism**

Even identical entity order does not automatically provide bit-identical simulation across:

```text
x86
ARM
different compilers
different SIMD modes
different thread schedules.
```

Floating-point reductions are non-associative:

$$
(a+b)+c
\neq
a+(b+c)
$$

in general finite precision.

Parallel reduction order can change results.

For strict lockstep determinism, you must control much more than ECS order.

Do not promise "deterministic ECS" and imply "deterministic entire engine."

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/16 - Archetypes, chunks, relationships, and transforms|Previous chapter]] · [[Chapter Edition/03 - Game Engine/18 - ECS contracts, diagnostics, and GPU handoff|Next chapter]]
