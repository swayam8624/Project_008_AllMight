# 31 — Spatial-index serialization and query evidence

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/30 - Depth, projection, and shadow-map precision|Previous chapter]] · [[Chapter Edition/03 - Game Engine/32 - Terrain materials and image composition|Next chapter]]

A spatial index must be saved, inspected, and compared against a reference query before its performance claims are useful. This chapter joins serialization, visualization, benchmarking, and structure choice around one query contract.

**By the end:** Define a reproducible spatial-query workload and compare candidate structures for correctness, update cost, and query quality.

**Recall question:** What reference result validates the index, and what measurements separate serialization, update, and query costs?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 76642–78476; source-span SHA256 d47bec240cb50e8b811461d0159c27b4dcf12c862428c365d2c1d9b3bc2bd99a. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### SERIALIZATION, VISUALIZATION, BENCHMARKING, STRUCTURE SELECTION, UNIFIED QUERY DESIGN

Parts 1396–1400 are only five atomic topics, but they are arguably the most important engineering topics of the entire spatial phase because they turn algorithms into an engine subsystem.

---

**Spatial-index serialization**

Suppose building a large SAH BVH takes:

$$
10\text{ seconds}.
$$

If geometry is static, rebuilding it every launch is wasteful.

We can serialize the built acceleration structure.

But "write the struct bytes to disk" is not a serialization system.

A valid format must define:

$$
\boxed{
\text{schema}
+
\text{version}
+
\text{endianness}
+
\text{numeric representation}
+
\text{offset semantics}
+
\text{validation}.
}
$$

---

**Never serialize raw pointers**

Suppose a node contains:

```cpp
Node* left;
Node* right;
```

Those addresses are meaningful only inside one process execution.

After loading tomorrow:

```text
0x7ff8a203...
```

may belong to something entirely different.

Therefore serialization should use:

```text
indices
offsets
IDs
```

rather than process addresses.

Your flat BVH already uses:

```cpp
SpatialIndex Left;
SpatialIndex Right;
```

which is serialization-friendly.

The pointer-based octree is less directly serializable because its `unique_ptr<Node>` relationships need flattening or reconstruction.

---

**Serialize source or derived structure?**

There are two valid strategies.

### Strategy A — serialize primitives and rebuild

Store:

```text
primitive bounds
IDs
settings
```

Then rebuild BVH/tree during load.

Advantages include portability and resilience to node-layout changes.

Disadvantage is rebuild cost.

### Strategy B — serialize built structure

Store:

```text
nodes
primitive order
build settings
metadata
```

Then load directly.

Load becomes extremely fast.

But format becomes more tightly coupled to implementation.

A production asset pipeline often does both conceptually:

```text
source asset
   ↓
offline cooking
   ↓
platform-specific cooked acceleration structure
```

The runtime only loads cooked data.

---

**A robust BVH file header**

A useful format might begin:

```cpp
struct SpatialFileHeader
{
    std::uint32_t magic;
    std::uint16_t versionMajor;
    std::uint16_t versionMinor;

    std::uint32_t endianTag;

    std::uint32_t nodeCount;
    std::uint32_t primitiveCount;

    std::uint32_t nodeStride;
    std::uint32_t primitiveStride;

    std::uint64_t sourceHash;
    std::uint64_t settingsHash;
};
```

Why store source/settings hash?

Suppose geometry changed but old BVH cache still exists.

If:

$$
Hash(geometry)\ne sourceHash,
$$

the cached tree is stale.

Likewise changing:

```text
MaxLeafSize
SAH settings
quantization policy
```

should invalidate cooked data.

---

**Versioning**

Suppose version 1 node is:

```cpp
Bounds
Left
Right
```

and version 2 adds:

```cpp
Parent
```

Blindly loading v1 bytes into v2 structs is dangerous.

Instead:

```text
magic
version
schema
```

controls migration.

A runtime asset loader might support only current cooked version:

```cpp
if (header.version != CurrentVersion)
    return RebuildRequired;
```

An editor tool can support conversion.

Separating **runtime simplicity** from **authoring compatibility** is often the right design.

---

**Endianness and alignment**

Suppose file was cooked on little-endian x86 and loaded on another architecture.

Integer byte representation may differ.

Likewise in-memory C++ struct padding is not a stable storage contract.

Never assume:

```cpp
sizeof(MyNode)
```

defines a universal file format.

Either serialize each field explicitly or define a strictly packed/versioned binary layout.

A file format is a data protocol, not a memory dump.

---

**What should be serialized for each Kairo structure?**

A static BVH is a strong candidate for direct serialization:

```text
nodes
primitives
primitive order
settings
stats optional
```

A dynamic AABB tree usually does **not** need to preserve transient topology. Often it is better to serialize object bounds/IDs and rebuild because fat bounds, balance state and free-list fragmentation are runtime concerns.

A spatial hash can serialize input object bounds but reconstruct hash buckets quickly.

A KD-tree for static point clouds can be worth serializing directly.

A navmesh is an excellent serialized asset:

```text
polygons
vertices
area types/costs
adjacency
portals
tile metadata
```

because navmesh generation can be expensive.

---

**Spatial visualization**

Spatial structures are almost impossible to debug from logs alone.

Imagine this report:

```text
VisitedNodes = 947
```

Is 947 good?

Without seeing the tree and query, you cannot tell.

Visual debugging lets you inspect structure.

For BVH:

```text
depth
bounds
leaf occupancy
traversal path
candidate primitives
```

For dynamic trees:

```text
tight bounds
fat bounds
height
rotations
area ratio
```

For spatial hashes:

```text
occupied cells
bucket occupancy
object duplication
```

For navigation:

```text
graph nodes
edges
open/closed sets
g/h/f
corridor
portals
funnel
waypoints
```

---

**Your current visualization stack**

`SpatialDebug.cppm` exposes lightweight debug geometry:

```cpp
DebugLine
DebugBox
DebugSphere
SpatialDebugDrawList
```

and extraction helpers for:

```text
BVH boxes
dynamic-tree bounds
octree cells
ray lines
```

Separately, `SpatialVisualizer.cpp` produces a much richer generated HTML/SVG debugger.

The repository README says the visualizer currently displays:

```text
BVH hierarchy/AABB queries
BVH ray traversal
dynamic-tree broadphase stats
spatial-hash grid/radius queries
KDTree nearest neighbors
NavigationGraph + A* paths
```

So Kairo already has a genuinely useful spatial debugging surface.

---

**A small current debug-data mismatch**

`ExtractBVHBoxes()` currently assigns:

```cpp
Depth = 0
```

to every BVH node.

So although the field is named `Depth`, static BVH extraction does not compute actual tree depth.

For dynamic tree extraction it writes:

```cpp
Depth = node.Height;
```

but height means:

$$
\text{distance from node to deepest descendant}
$$

not:

$$
\text{distance from root to node}.
$$

So the same field currently means two different things, and one of them is always zero.

A better debug record would separate:

```cpp
uint32_t DepthFromRoot;
uint32_t SubtreeHeight;
```

These are different diagnostic quantities.

---

**Visualizing A***

One of the best A* debuggers shows search evolution frame by frame.

For every node:

```text
white  = unseen
blue   = open
gray   = closed
green  = final path
red    = blocked
```

Then label:

$$
g,\ h,\ f.
$$

For node N:

```text
N
g=12
h=7
f=19
```

Watching expansion immediately reveals heuristic behavior.

If A* expands almost every node:

$$
h
$$

is weak or zero.

If it rushes toward goal and returns a wrong route:

$$
h
$$

may be inadmissible or closed-set logic wrong.

Visualization converts mathematical properties into visible behavior.

---

**Visualizing a navmesh path**

A useful navmesh debugger should display four layers simultaneously:

```text
all polygons
    ↓
selected A* corridor
    ↓
left/right portal edges
    ↓
funnel waypoints
```

Then if the path zig-zags, you can identify whether the bug lies in:

```text
A* corridor
portal orientation
funnel logic
steering
```

without guessing.

This separation is especially important because a poor final route is not necessarily an A* bug.

---

**Benchmarking build time versus query time**

This is one of the most important equations in the spatial phase.

Suppose structure $S$ costs:

$$
B_S
$$

to build/update, and average query cost is:

$$
Q_S.
$$

Suppose it will receive:

$$
N_q
$$

queries before the next rebuild.

Total cost:

$$
\boxed{
T_S
=
B_S+N_qQ_S.
}
$$

This explains why "fastest query" is not the same as "best data structure."

---

**The crossover point**

Suppose structure A:

$$
B_A=100ms,
\qquad
Q_A=0.01ms.
$$

Structure B:

$$
B_B=5ms,
\qquad
Q_B=0.05ms.
$$

Set total costs equal:

$$
100+0.01N
=
5+0.05N.
$$

Therefore:

$$
95=0.04N
$$

and:

$$
\boxed{
N=2375.
}
$$

If fewer than about 2375 queries occur before rebuild, B is faster overall.

If many more occur, A amortizes its expensive build and wins.

That is exactly why high-quality SAH BVHs dominate static ray tracing while fast Morton/refit methods are attractive for dynamic geometry.

---

**End-to-end cost includes candidate quality**

For collision broadphase:

$$
T
=
T_{\text{update}}
+
T_{\text{broadphase}}
+
N_cT_{\text{narrowphase}},
$$

where:

$$
N_c
=
\text{candidate pair count}.
$$

Suppose spatial hash query itself is 2× faster than dynamic tree but returns 10× more false-positive candidates.

The total physics frame may be slower.

Therefore benchmark:

$$
\boxed{
\text{whole pipeline, not just index lookup}.
}
$$

The same applies to rendering.

A culling structure that costs little but returns nearly every object saves almost nothing downstream.

---

**What a credible benchmark must control**

Use identical input data and query sets across structures.

Correctness must first be checked against a brute-force oracle.

Then record at minimum:

$$
\text{build/update time},
$$

$$
\text{query latency},
$$

$$
\text{candidate count},
$$

$$
\text{visited nodes/cells},
$$

$$
\text{memory use}.
$$

Average alone is not enough.

Record:

$$
p50,\ p95,\ p99,\ max.
$$

Games care about frame spikes.

A 0.1 ms average with an occasional 20 ms query may be worse than a consistent 0.5 ms algorithm.

---

**Benchmark distributions, not one toy scene**

Spatial structures are workload-sensitive.

Benchmark at least several distributions.

A uniform grid favors different structures from a highly clustered scene.

A line/corridor stresses axis choice.

Concentric/overlapping objects stress hierarchy pruning.

Huge scale variation stresses grids.

High churn stresses dynamic update policies.

Mostly-static scenes favor expensive preprocessing.

A research result claiming:

> "spatial hash is fastest"

without specifying spatial distribution, object sizes, motion, query shape and density is almost meaningless.

---

**Warm caches and cold caches**

Running the same query one million times measures a heavily warmed cache.

Real workloads may query different regions.

Therefore separate:

$$
\text{warm-cache throughput}
$$

from:

$$
\text{cold/random access}.
$$

Likewise distinguish:

```text
build once
query repeatedly
```

from:

```text
update
query
update
query
```

because dynamic structures interact strongly with cache state and allocator behavior.

---

**Current Kairo benchmark status**

`BVHBuildStats` already records:

```cpp
BuildMilliseconds
```

and traversal results record:

```cpp
VisitedNodes
TestedPrimitives
```

Dynamic tree exposes:

```text
Height
MaxBalance
AreaRatio
```

These are excellent instrumentation primitives.

But there is currently no dedicated benchmark suite comparing:

```text
BVH median
BVH SAH
Morton path
dynamic tree
SAP
spatial hash
octree
brute force
```

under controlled workloads.

That is the major missing piece for Part 1398.

---

**Choosing a spatial data structure by workload**

There is no "best spatial structure."

The relevant variables are:

$$
N
$$

object count,

$$
U
$$

update rate,

$$
Q
$$

query rate,

$$
D
$$

spatial density,

$$
S
$$

object-scale variance,

$$
C
$$

temporal coherence,

and query type.

A useful decision table is:

| Workload | Strong default |
|---|---|
| Static triangles + many rays | SAH BVH / wide BVH |
| Frequently rebuilt ray geometry | LBVH/Morton + refit/hybrid |
| Arbitrary moving rigid objects | Dynamic AABB tree |
| Coherent moving AABBs | Sweep-and-prune |
| Many similar-size local objects | Spatial hash/uniform grid |
| Sparse multiscale world | Loose octree |
| 2D spatial partition | Quadtree |
| Static point nearest-neighbor | KD-tree |
| Character navigation over surfaces | Navmesh |
| Huge regular navigation grid | Hierarchical grid/HPA* |
| Indoor visibility | Cells/portals/PVS |
| Dynamic rendered occlusion | HZB |

This table is not law. It is the first hypothesis you benchmark.

---

**Update/query ratio is often the deciding factor**

Suppose geometry changes every frame.

A high-quality static SAH build may be unacceptable.

If:

$$
B_{\text{SAH}}
\gg
B_{\text{fast}},
$$

and only a handful of queries occur before the next update, fast update wins even if each query is slower.

Conversely static collision mesh:

```text
build once
query millions of times
```

strongly favors expensive preprocessing.

So always ask:

$$
\boxed{
\text{How many queries will amortize one update?}
}
$$

This question is more useful than asking:

> Which tree is faster?

---

**Query shape also matters**

A structure optimized for point-neighbor queries may not be ideal for ray traversal.

KD-tree:

$$
\text{excellent nearest-point pruning}.
$$

BVH:

$$
\text{excellent bounded-primitive ray traversal}.
$$

Hash:

$$
\text{excellent local radius/neighborhood enumeration under suitable scale}.
$$

Portal/PVS:

$$
\text{excellent whole-region visibility rejection}.
$$

The word **spatial query** hides many very different computational problems.

---

**Data distribution matters**

Suppose spatial hash cell size is 1 m.

If every object is ~0.5 m, excellent.

If one object is 10 km wide, it may occupy:

$$
10^{12}
$$

1 m³ cells in 3D conceptually.

Completely unusable.

A dynamic tree handles scale variation much better.

Likewise SAP works well if intervals remain locally ordered, but a chaotic teleport-heavy scene destroys temporal coherence.

Thus structure selection is fundamentally empirical.

---

**Designing one spatial-query interface**

Now we reach the final atom of the entire spatial phase.

Kairo currently says it wants one consistent spatial-query language.

This is the correct goal.

But a dangerous design is:

```cpp
class ISpatialIndex
{
public:
    virtual QueryAABB(...)=0;
    virtual QuerySphere(...)=0;
    virtual Raycast(...)=0;
    virtual Nearest(...)=0;
    virtual FindPath(...)=0;
};
```

Why dangerous?

Because not every backend meaningfully supports every operation.

A navigation graph cannot naturally answer arbitrary ray-triangle intersection.

A BVH does not inherently know how to find an AI path.

A hash grid may support ray traversal only poorly.

Forcing every structure into every method produces fake or inefficient implementations.

---

**Separate query semantics from backend**

The better idea is:

$$
\boxed{
\text{one query vocabulary}
+
\text{explicit capabilities}
+
\text{multiple backends}.
}
$$

Shared geometric queries might be:

```cpp
struct AABBQuery;
struct SphereQuery;
struct RayQuery;
struct PointQuery;
struct NearestQuery;
```

Navigation:

```cpp
struct PathQuery;
```

Then each backend advertises capabilities.

For example:

```cpp
enum class SpatialCapability : std::uint32_t
{
    AABB    = 1u << 0,
    Sphere  = 1u << 1,
    Ray     = 1u << 2,
    Point   = 1u << 3,
    Nearest = 1u << 4,
    Path    = 1u << 5
};
```

A BVH might expose:

```text
AABB
Sphere
Ray
Point
```

KD-tree:

```text
Nearest
Radius
```

Navigation:

```text
Path
```

This is honest architecture.

---

**Candidate semantics must also be explicit**

Suppose dynamic tree returns collider 42 because its AABB intersects a ray.

Does that mean:

$$
\text{ray hit collider 42 exactly}?
$$

No.

It means:

$$
\text{ray hit collider 42's broadphase bound}.
$$

A unified interface must distinguish:

```text
candidate query
exact geometry query
```

Otherwise consumers will eventually treat broadphase results as exact hits.

Possible naming:

```cpp
QueryCandidates(...)
RaycastBounds(...)
RaycastExact(...)
```

or attach result quality:

```cpp
enum class SpatialResultKind
{
    ConservativeCandidate,
    Exact
};
```

Semantics matter more than naming.

---

**Any-hit, closest-hit and all-hit are separate contracts**

Do not expose one ambiguous `Raycast()` and assume everyone knows what it does.

There are at least three distinct operations:

$$
\text{AnyHit}
$$

returns as soon as one valid hit exists.

$$
\text{ClosestHit}
$$

returns minimum:

$$
t.
$$

$$
\text{AllHits}
$$

returns all acceptable intersections, usually sorted.

Those have different optimal traversal strategies.

The interface should express the semantic intent so the backend can optimize appropriately.

---

**A capability-oriented C++ design**

One practical architecture is compile-time concepts for engine-internal code.

```cpp
template<class T>
concept AABBQueryable =
    requires(
        const T& index,
        const SpatialAABB& bounds,
        SpatialLayerMask mask)
{
    {
        index.QueryAABB(bounds, mask)
    } -> std::same_as<SpatialOverlapResult>;
};

template<class T>
concept RayQueryable =
    requires(
        const T& index,
        const SpatialRay& ray,
        float maxDistance,
        SpatialLayerMask mask)
{
    {
        index.QueryRay(
            ray,
            maxDistance,
            mask)
    };
};
```

Then algorithms can require only what they use.

No giant universal base class is necessary.

---

**Runtime backend selection**

If backend must be selected dynamically, type erasure or `std::variant` works.

For example:

```cpp
using SpatialBackend =
    std::variant<
        StaticBVHIndex,
        DynamicTreeIndex,
        HashGridIndex,
        OctreeIndex
    >;
```

Then:

```cpp
SpatialOverlapResult QueryAABB(
    const SpatialBackend& backend,
    const SpatialAABB& query,
    SpatialLayerMask mask)
{
    return std::visit(
        [&](const auto& index)
            -> SpatialOverlapResult
        {
            if constexpr (
                requires {
                    index.QueryAABB(
                        query,
                        mask);
                })
            {
                return index.QueryAABB(
                    query,
                    mask);
            }
            else
            {
                throw UnsupportedQuery{};
            }
        },
        backend);
}
```

That gives one runtime object while retaining backend-specific capabilities.

---

**But navigation should probably remain a separate domain service**

A path query is conceptually different from geometric overlap.

It contains:

```text
start
goal
agent properties
cost filters
obstacle policy
```

rather than:

```text
shape
bounds
layer mask
```

So I would not force:

```cpp
FindPath()
```

into the same low-level interface as:

```cpp
QueryAABB().
```

Instead use a higher-level facade:

```text
SpatialService
├── GeometryQueries
└── NavigationQueries
```

Both can share IDs, vectors, debugging and spatial-index infrastructure without pretending they are identical operations.

That preserves conceptual clarity.

---

**A coherent Kairo-style final architecture**

A production-oriented design could look like:

```text
KairoSpatial
│
├── Geometry contracts
│   ├── SpatialID
│   ├── SpatialAABB
│   ├── SpatialRay
│   ├── overlap results
│   └── ray results
│
├── Static indices
│   ├── BVH
│   └── KDTree
│
├── Dynamic indices
│   ├── DynamicAABBTree
│   ├── SweepAndPrune
│   └── SpatialHash
│
├── Partitioning
│   ├── Octree
│   └── Quadtree
│
├── Navigation
│   ├── NavigationGraph
│   ├── AStar
│   └── NavMesh
│
├── Query facade
│   ├── AABB
│   ├── sphere
│   ├── ray
│   ├── nearest
│   └── path
│
├── Serialization
│
├── Benchmarking
│
└── Debug/visualization
```

This is much closer to what the entire Phase 14 curriculum is building toward.

---

**The final conceptual synthesis of Passes 07–09**

You began Part 1301 with:

$$
\text{"Spatial acceleration is pruning work."}
$$

That statement now expands into an entire engine philosophy.

For geometric queries:

$$
\text{cheap conservative bound}
\rightarrow
\text{discard impossible work}.
$$

For A*:

$$
\text{cheap optimistic heuristic}
\rightarrow
\text{prioritize plausible work}.
$$

For navmeshes:

$$
\text{compress continuous walkable space}
\rightarrow
\text{search a much smaller graph}.
$$

For HPA*:

$$
\text{compress the graph again}
\rightarrow
\text{search an abstraction first}.
$$

For HZB:

$$
\text{compress depth spatially}
\rightarrow
\text{discard hidden rendering work}.
$$

The unifying idea is:

$$
\boxed{
\text{construct a cheaper representation that preserves enough truth to reject or postpone expensive computation.}
}
$$

That is the intellectual center of spatial acceleration.

---

**Reconstruction challenge**

You should be able to rebuild A* from first principles without memorizing code.

Start with:

$$
g(start)=0.
$$

All other nodes:

$$
g=\infty.
$$

Put start in a min-priority open set with:

$$
f=g+h.
$$

Repeatedly pop lowest-$f$ node.

For each outgoing edge:

$$
tentative=
g(current)+cost.
$$

If:

$$
tentative<g(neighbor),
$$

update:

$$
cameFrom(neighbor)=current,
$$

$$
g(neighbor)=tentative,
$$

$$
f(neighbor)=g(neighbor)+h(neighbor),
$$

and push/update the open set.

Terminate when the goal is safely finalized under your heuristic assumptions.

Then walk `cameFrom` backward.

If you cannot derive the algorithm from those equations, do not claim mastery yet.

---

**Heuristic reconstruction challenge**

You should be able to explain without notes:

For 4-connected unit grid:

$$
h=
|dx|+|dy|.
$$

For 8-connected unit/$\sqrt2$ grid:

$$
h=
\max(dx,dy)
+
(\sqrt2-1)\min(dx,dy).
$$

For arbitrary geometric graph with costs unrelated to distance, raw Euclidean may fail.

Kairo's generic fix:

$$
\alpha=
\min_e
\frac{c(e)}{d(e)},
$$

$$
h(n)=
\alpha d(n,g).
$$

Then prove both:

$$
h(n)\le h^*(n)
$$

and:

$$
h(u)\le c(u,v)+h(v).
$$

That derivation is the theoretical mastery checkpoint of M083.

---

**Navmesh reconstruction challenge**

From a blank file, you should be able to design:

```text
NavPolygon
Portal
Adjacency
FindContainingPolygon
A* corridor
Funnel
```

and explain each stage.

You should know why polygons are convex, why portals require direction/orientation, why polygon centroids are acceptable graph representatives but produce ugly final paths, why the funnel fixes that, why agent radius cannot simply be ignored, and why moving agents belong in steering rather than navmesh carving.

You should also be able to identify the current Kairo limitation:

$$
AreaCost<1
$$

can invalidate the navmesh's unscaled Euclidean heuristic.

---

**M084 reconstruction challenge**

Given two candidate spatial structures A and B, derive:

$$
T_A=B_A+NQ_A
$$

and:

$$
T_B=B_B+NQ_B.
$$

Solve their crossover query count.

Then explain why query time alone is insufficient when candidate counts differ.

Design a serialized BVH file without raw pointers.

Design a debug view showing tree quality.

Finally design a query API that does not pretend BVH, spatial hash, KD-tree and navmesh all support the exact same semantics.

If you can do those things, you understand the **systems engineering** rather than merely the algorithms.

---

**Exact coverage**

| Part | Topic | Status |
|---:|---|---|
| 1375 | Navigation-graph representation | ✓ |
| 1376 | Weighted navigation edges | ✓ |
| 1377 | A* open-set representation | ✓ |
| 1378 | A* closed-set representation | ✓ |
| 1379 | A* g score | ✓ |
| 1380 | A* heuristic score | ✓ |
| 1381 | A* f score | ✓ |
| 1382 | Parent/came-from map | ✓ |
| 1383 | Path reconstruction | ✓ |
| 1384 | Euclidean heuristic | ✓ |
| 1385 | Manhattan heuristic | ✓ |
| 1386 | Octile heuristic | ✓ |
| 1387 | Dijkstra as A* with $h=0$ | ✓ |
| 1388 | Heuristic admissibility | ✓ |
| 1389 | Heuristic consistency | ✓ |
| 1390 | Navigation grids | ✓ |
| 1391 | Navigation meshes | ✓ |
| 1392 | Navmesh polygon adjacency | ✓ |
| 1393 | Funnel algorithm | ✓ |
| 1394 | Dynamic obstacle updates | ✓ |
| 1395 | Hierarchical pathfinding | ✓ |
| 1396 | Spatial-index serialization | ✓ |
| 1397 | Spatial visualization | ✓ |
| 1398 | Build-vs-query benchmarking | ✓ |
| 1399 | Workload-driven structure choice | ✓ |
| 1400 | Unified spatial-query interface | ✓ |


And with Part **1400**, the entire **Spatial Acceleration, Dynamic Broadphase and Navigation phase is complete**.

The next pass crosses a major engine boundary.


We enter **ECS, entity identity, packed generational handles, free lists, stale-handle prevention, component storage, sparse sets, dense iteration, add/remove mechanics and the beginning of data-oriented entity architecture**. That phase is where many of the design ideas you just saw—stable IDs versus vector positions, contiguous data, indirection, free lists, stale references, cache behavior—stop being isolated spatial-system concerns and become the architecture of the whole engine.

## Make a world that can be authored, saved, and run

Drawing one object makes a graphics demonstration. A useful engine also lets someone create and inspect the data behind many objects, then run that data without confusing authored state with temporary play state.

#### From geometry pieces to objects and transforms

Geometry is a drawable piece; a mesh groups one or more geometries and associates them with a transform so the group can behave as one object. Different pieces can keep different materials, which fits imported assets containing multiple subobjects. The renderer still only needs each geometry and the matrix at which to draw it, so the application can flatten mesh contents into geometry render records. The video's dynamic allocation on every frame is explicitly temporary. (Kohi #053, 00:01–00:03; 00:10–00:17)

A transform packages position, quaternion rotation, scale, a cached local matrix, a dirty flag, and an optional parent. The dirty flag is a correctness mechanism: changing a component means the cached matrix is stale, so changes should go through setters that mark it dirty. Absolute setters and relative operations have different meanings: setting position replaces it, while translating adds an offset; rotation and scale have corresponding relative operations. Local matrices can then be recomputed only when needed, and world transforms combine the local transform with the parent's world transform up the hierarchy. (Kohi #054, 00:01–00:15)

Parent pointers impose a lifetime requirement. If a mesh is copied from a temporary stack variable into another collection, a pointer to the original transform becomes invalid when that function returns. Keeping meshes in stable application-owned storage makes parent references usable. In the demo, a child inherits its parent's rotation while retaining its local offset and its own rotation. (Kohi #054, 00:15–00:27)

Source reading: [caption source](https://www.youtube.com/watch?v=73KBx90cD0M) · [caption source](https://www.youtube.com/watch?v=r2535XLneiI). Captions document the narrated development state; on-screen code and reported builds were not independently reproduced.


### Model a scene as data, then make it executable

A scene is a set of entities and the data attached to them, plus systems that interpret that data. The entity is a handle or identity; components hold facts such as transform, sprite, camera, collider, or script reference. Systems query entities that have the components they need and update or render them.

Hazel’s public snapshot uses EnTT’s registry. The `Scene` creates entities, destroys them, duplicates them, finds them by UUID or name, and provides views over component sets. A `TransformComponent` can describe position and orientation, while other components describe renderable or behavioral data. The entity wrapper gives editor-facing code a convenient handle while the registry stores component data. This is an entity-component system (ECS): instead of building deep class inheritance such as `AnimatedEnemyWithColliderAndSprite`, the game can assemble an entity from independent pieces.

The advantage appears when combinations vary. One entity might have a transform and camera. Another might have transform, sprite, and physics body. A third might have transform and script state. A system can process exactly the component combinations it needs. The cost is indirection and the need to manage identity, component relationships, and system scheduling. ECS is not automatically faster or simpler for every project. It is useful when the project needs flexible composition and data-oriented queries. A small game with a few stable object types may be clearer with direct objects.

#### Stable identity matters for saving

An ECS registry often uses compact internal indices that can change when entities are destroyed or storage is reorganized. A saved scene needs stable identities across a reload. Hazel’s `Scene` keeps UUIDs associated with registry entities, and its API exposes lookup by UUID. This separates a persistent identifier from a transient storage handle.

A UUID is a large identifier intended to be unique enough that independently created objects can be distinguished. It is not a human-readable name and it does not encode ordering. Names are useful for searching and display; UUIDs are useful for references that should survive rename. A saved camera target, parent relationship, or script reference should not depend only on an editable label like “Player.”

When cloning or duplicating a scene, choose whether IDs should be preserved or regenerated. Copying a scene for runtime play often needs a separate runtime instance, while the editor copy remains intact. Reusing the same UUID in two simultaneously active scenes can confuse global lookup or asset references. Define the lifetime and uniqueness domain: per scene, per project, or globally.

##### Save the state the player needs to resume

A world chunk describes initial conditions and static world content. A saved game records the changed state needed to resume play. Copying all static geometry into every save is redundant if it can be loaded from the world data; saving a smaller selection of gameplay-relevant changes can make a checkpoint file compact.

A **checkpoint save** can store a checkpoint ID plus the player state needed at that point, such as health, lives, inventory, weapons, or ammunition. If the game deliberately starts every player at a known state, even some of that data can be omitted. The tradeoff is less choice about where to save and the risk of frustration when checkpoints are far apart.

A **save-anywhere** system needs a broader snapshot: current locations and other relevant state for the objects whose state affects gameplay, while static world content can still come from the level data. It need not preserve every runtime detail. For example, exact animation time or every simulated body's precise velocity may be omitted if the difference after loading is imperceptible or irrelevant. This is a player-facing equivalence test: after save and reload, can the player tell a meaningful difference? The answer depends on the game, so save policy is part of gameplay design as well as file format design.

The chapter says world loading and saved-game loading often have distinct requirements, even if they can share parts of a data format. A level file may favor flexible authoring, while a save may favor compression and compactness. Treat this as the source’s architectural comparison, not as an argument that a current engine must maintain two completely separate systems.

#### Keep authored data visible to both code and the editor

A designer should be able to move a boundary or tune a speed without finding a magic number in source. An empty scene entity with only a transform can mark an upper or lower bound; a script can read that transform at runtime. The same pattern applies to spawn regions: represent the region as a visible collider or volume, then let the script use its bounds. This makes the level itself carry spatial decisions, while code describes what those decisions mean (Hazel game code review, 00:28:11–00:30:20).

Gameplay values can follow the same route. A script’s public speed or dash fields can be exposed in the scene editor, while update logic applies elapsed time to cooldowns. If a cooldown starts at half a second, subtract the frame’s elapsed seconds and trigger when it reaches zero; this keeps the duration stable as frame lengths vary. For a small prototype, a few local timers may be clearer than a generalized scheduler. When many behaviors need timed callbacks, a shared system may become worth the design and testing cost (Hazel game code review, 00:14:23–00:21:48).

Visual appearance and collision are separate representations. In a 3D tile world, the creator renders thousands of repeated tiles through instancing while a separate ground plane provides collision. This avoids treating every visual tile as a physics entity, but it also means a visible shoreline does not automatically block movement. The rule at a land/water boundary must be designed explicitly, because an invisible collider may contradict what the player sees (Remaking My First Game in 3D, 00:17:43–00:23:33).

### White is the identity texture for a color-only quad

A 2D renderer can draw a solid color and a textured sprite through one shader. Let $T$ be the sampled texture color and $C$ the per-quad tint; output $T\odot C$, where $\odot$ means channel-wise multiplication. For an ordinary color quad, bind a one-pixel white texture, so $T=(1,1,1,1)$ and the result is exactly $C$. For a textured quad, bind its image and use white as $C$ when no tint is wanted. The same shader and binding path now handles both cases. [Single Shader 2D Renderer](https://www.youtube.com/watch?v=-myXuS3t1W4)

The Hazel episode removes its separate flat-color shader, adds a color uniform to the texture shader, and binds that shader once at `BeginScene`. It creates the white texture directly in GPU memory rather than depending on an extra image file. This simplifies shader state changes, but the episode does not benchmark the size of any speedup; the demonstrated result is one shared path and fewer shader switches.

Creating a texture and filling it are different operations. The source adds a size-based constructor and a `SetData` upload method. For a full uncompressed image with width $W$, height $H$, and $B$ bytes per pixel, the data buffer must contain $W H B$ bytes. For example, a $1\times1$ RGBA8 white image uses four bytes. That check is valid only when the supported format and upload region are known; a generalized API must account for row pitch, subregions, compressed formats, channel order, and overflow in the size calculation. The episode explicitly leaves texture formats and sampling options for later refinement.

#### Source-to-volume map

- Existing foundations: `### Treat shaders and materials as a pipeline, not just files`, `#### Connecting per-object material data and texture sampling`, and `### Reuse geometry and choose a batch boundary from the data path`.
- This source adds the white-texture identity trick and a concrete full-image byte-count contract; it does not repeat material or sampling foundations.
- Limits: English automatic captions; the shader, texture API, and measured performance were not independently verified.

### An entity wrapper can hide registry vocabulary without owning component storage

An ECS registry stores component data; an entity is an identity used to associate that data. An editor-facing API can still make the common operation read naturally: create an entity, then ask it to add or retrieve a component. A thin wrapper can carry the registry identity and the scene context needed to perform that query, while leaving component storage in the registry. Its convenience methods do not mean the entity object owns its components.

The Hazel episode builds this wrapper around an EnTT handle and a scene pointer. `HasComponent<T>`, `GetComponent<T>`, `AddComponent<T>(args...)`, and `RemoveComponent<T>` forward to registry operations. Forwarding constructor arguments lets the registry construct a component in place. The scene also adds a transform and a tag when creating a named entity. This is one API design choice, not a requirement of ECS. [The ENTITY Class](https://www.youtube.com/watch?v=GfSzeAcsBb0)

The ownership boundary needs care. The presenter considers a weak scene reference but uses a raw pointer in this snapshot, keeping the wrapper small and non-owning. That means its validity depends on the scene outliving every operation performed through the wrapper. A wrapper copied into a panel or retained across scene replacement can dangle. A nonzero handle check is also not a complete liveness test: the registry may have destroyed that ID, and zero may or may not be reserved by the particular entity library. Check both context lifetime and registry validity before dereferencing, or arrange for wrappers to be short-lived and scoped to a known scene operation.

This source extends the existing scene/ECS explanation with the boundary between a convenient facade and actual ownership. For the broader system overview, see `### Model a scene as data, then make it executable` and `### Treat editor panels as clients of live engine state`.

#### Source-to-volume map

- Existing foundation: `### Model a scene as data, then make it executable`, which already identifies the entity as a handle and components as registry data.
- The source adds the wrapper's concrete method surface, in-place argument forwarding, and the raw-scene-pointer lifetime tradeoff.
- Limits: English automatic captions; the wrapper implementation and EnTT behavior were not independently inspected.

### Clear the entity-ID attachment to a deliberate sentinel

An editor can identify an object under the mouse by rendering entity IDs into a separate integer color attachment. The scene color attachment can keep its usual background color, but the ID attachment needs a value that means “no entity.” Otherwise empty pixels inherit whatever clear value happens to be configured and can look like valid IDs. In this episode, the object test pixels contain `50`, so the chosen background sentinel is `-1`. [Clearing Framebuffer Texture Attachments](https://www.youtube.com/watch?v=LPhBiIvDOs0)

The captioned prototype performs its ordinary scene clear, then explicitly initializes the ID texture with `glClearTexImage`, using an integer format and an `int` value. The integer guarantee comes from that type-matched operation: ordinary `glClear` has undefined results for integer color buffers. For a signed integer attachment, a correctly targeted `glClearBufferiv` is another typed path; use the intended draw-buffer index and matching framebuffer state. [OpenGL specification](https://registry.khronos.org/OpenGL/specs/gl/glspec30.pdf) The public framebuffer operation becomes `ClearAttachment(index, value)`. This narrowly typed interface makes accidental float input harder for the shown integer attachment. The episode notes that a broader typed API might be useful later, and that clear responsibility could ultimately belong to a render pass or another layer. The demo confirms the intended data result in its displayed picking buffer: object pixels show `50`, empty pixels show `-1`.

The accompanying Git lesson explains why unfinished feature work lives on a branch. Keep a usable main branch as the known baseline, isolate an in-progress mouse-picking feature, then merge it when ready. Branching allows the development snapshot to be incomplete, but does not itself make that snapshot stable. This source stops before actual selection: its next step is to read the ID when the mouse points at or clicks an entity. The narrated buffer values are not independent runtime verification, and exact API spelling comes from automatic captions.

The signed `R32I` example reserves $-1$ for empty pixels; declare the supported ID range rather than assuming every unsigned 32-bit identifier is a positive signed value. An unsigned `R32UI` design needs matching shader outputs, transfer types, and a reserved unsigned sentinel. Neither format creates an extra out-of-band value: reserve an ID or carry validity separately.

### Inspect and edit the selected entity

A properties panel can start with one reliable fact the scene hierarchy already knows: which entity is selected. The Hazel editor uses that selection context to show the entity's tag and transform. The tag is copied into a bounded character buffer for the ImGui input field, then written back to the tag string if the user edits it. A compiler warning prompts replacement of an unsafe copy with a size-aware version. The episode explicitly leaves Unicode support open. [Properties Panel](https://www.youtube.com/watch?v=NBpB0qscF3E)

For translation, the panel exposes the transform matrix's translation vector through a three-component drag control. The control reports whether a value changed; that matters for fields such as rotation that may require decomposing and rebuilding a matrix, although translation is edited in place in this example. A collapsible component heading uses a type-specific hash as its UI identifier and is open by default. The episode notes that polished labels, stable ID conventions, and a reusable style layer still need attention.

Selection should also be reversible. Clicking blank space in the hierarchy panel clears the selection, while clicking an entity row still selects it. The handler checks whether the panel is hovered and whether the left mouse button is down; ImGui's active-item blocking prevents the blank-area handler from swallowing a row click. This episode does not yet select objects by clicking the rendered scene, and its inspector still lacks broad component editing, add/remove controls, and a camera panel. Those limits keep the example a first working inspector step rather than a finished authoring workflow.

### Carry entity identity through a batched draw

The integer attachment from the previous step can only help if each rendered entity writes its own ID. A uniform is set for a draw call, but Hazel's 2D renderer batches many quads into one call. The episode therefore adds an integer ID to each quad's vertex data and forwards it through the editor shader to the integer color attachment. The entity identifier is unsigned in the entity system, while the picking buffer is signed so `-1` can represent empty space; the narrator notes that zero is a valid entity ID. [Mouse Picking](https://www.youtube.com/watch?v=GKERbQ7Sqyg)

Integer vertex attributes need the integer attribute setup path (`glVertexAttribIPointer`), rather than the floating-point conversion path. The speaker admits the existing vertex-layout code is not fully understood during the walkthrough, which is a cue to verify the exact API and types in code before copying the pattern. The renderer API is also still in flux: generic quads serve UI and debug drawing, while a sprite-specific entry point can take a sprite component, transform, and entity ID.

With IDs written, the editor can read a pixel and map its value back to a scene entity, then display that entity's tag while the pointer hovers. This episode does not yet implement click-to-select. It calls out the interaction cases that must be handled: dragging an entity should not accidentally select another, and a click over a front face should not replace the intended manipulation target. The extra buffer is presented as editor tooling; the episode gives no profiler evidence and does not argue that shipped games should pay for this path.

### Treat entities as IDs and components as data

Hazel adopts EnTT for its entity-component system rather than building a robust registry from scratch. The episode's reasons are practical: a reliable ECS is a large subsystem, and the author wants to spend more time on graphics. The library is vendored as a single header with its license, and exposed to both engine and client code because templates must be instantiated where client-defined component types are used. [Intro to EnTT](https://www.youtube.com/watch?v=D4hz0wEB978)

A scene owns a registry. The registry creates entity identifiers and stores their components; an entity itself is just an ID, not a nested object that owns fields. A component can therefore be a plain data struct, such as a transform wrapper around a matrix. The registry emplaces a component on an entity, retrieves it by reference, checks whether it exists, or removes it. No shared component base class is required.

Systems need sets of entities with the data they consume. A view iterates entities with one component type; a group can expose entities with several required component types, such as both transform and sprite data for rendering. The episode shows structured bindings for the retrieved references. EnTT also offers runtime views when component types are not known at compile time, and lifecycle signals for construct/destroy/replace events. Those features are described but not benchmarked or fully integrated here. Snapshot serialization is mentioned, then deferred in favor of the author's existing hand-written approach because the API did not feel clear. Verify exact methods against the EnTT version in use; this transcript is an introductory development recording, not a current API reference.

### Build a scene through entity and component menus

An editor becomes useful when a blank scene can be assembled without writing setup code. Hazel adds a right-click menu on empty hierarchy space to create an entity, and a separate context menu on an entity row to delete it. Deletion is deferred until the current UI pass finishes, because the row and properties panel may still read the entity during that frame. If the removed entity was selected, the selection context is cleared as well. [Adding/Removing Entities and Components UI](https://www.youtube.com/watch?v=PsyGmsIgp9M)

The properties panel adds a component menu for camera and sprite-renderer components. A per-component header control offers removal, while the transform stays mandatory and cannot be removed. Removal is likewise delayed until the current UI drawing is done. The options are hard-coded in this version; a component registry would be needed for extensible or user-defined component menus.

A lifecycle issue appears when a camera is added after the viewport's initial resize: it missed the earlier event and has no matching projection. The scene therefore handles component-added notifications and initializes a new camera with the stored viewport size. The example uses explicit template specializations for each built-in component and notes that this may not scale to custom types. Finally, the episode still needs a camera entity to display the scene; a separate editor camera is not part of this implementation yet. These mechanics produce a basic scene-building path, while camera ownership, extensibility, and UI polish remain incomplete.

### Bind native C++ behavior to an entity

A camera entity needs behavior such as moving in response to input, but that behavior belongs to the game scene rather than the editor layer. Hazel introduces a `NativeScriptComponent` that binds a C++ script type and gives the script access to its entity and scene through a `ScriptableEntity` base. This native path is presented as a choice alongside the planned C# scripting system; it does not require an external language runtime. [Native Scripting](https://www.youtube.com/watch?v=iIUhg88MK5M)

The initial binding stores callbacks for deferred construction, destruction, and the `OnCreate`, `OnDestroy`, and `OnUpdate` lifecycle methods. A scene update walks entities with native-script components, constructs an instance when needed, supplies its entity/scene context, calls `OnCreate`, then calls `OnUpdate` with the timestep. The camera-controller example changes a transform using keyboard input, illustrating how the script can use Hazel's C++ APIs directly.

This first implementation has important limits. It assumes all lifecycle methods exist, so a small script cannot omit unused hooks; the author proposes compile-time detection for a later episode. The scene has no proper play/stop lifecycle, and destruction is not fully connected. The callback approach is later revised in a separate episode to use virtual lifecycle methods. Performance concerns about callable wrappers are discussed but not measured. The recording is live coding, including a camera-selection false alarm, so treat the transcript as an implementation story rather than independent runtime proof.

### Use component pools for the work a system needs

A scene can contain a player, a light, an audio source, a mesh, and behavior in many combinations. Inheritance tends to turn those combinations into a growing family of special subclasses: an audio mesh, a lighted audio mesh, an enemy that is all of those, and so on. Composition represents each capability as a separate data component attached to an entity. The entity's key role is to associate the components that belong together. [Entity Component System](https://www.youtube.com/watch?v=Z-CILn2w9K0)

A simple entity-owned list of component pointers is flexible but can make systems scan every entity, test which components exist, and follow pointers to scattered memory. An ECS instead stores component types in pools so a job can iterate the data it needs together. A renderer can work on mesh and transform data; physics can use rigidbody and transform data; script updates can walk script components. The narrator explains cache locality conceptually, without benchmark measurements.

“System” describes the work over one or more component pools; it does not require a rigid separate class hierarchy. EnTT is chosen to provide registry storage, relationships, and iteration, while Hazel should keep components mostly as data and expose a convenient API. This episode is a design explanation, not an implementation walkthrough; the integration comes later. Treat its performance discussion as a motivation and mental model, not a quantified result.

### Write color and entity identity in one render pass

Editor picking needs a screen-space record of which entity produced each pixel. Hazel adds a second color attachment to the scene framebuffer: the usual color target remains visible, while a separate integer target stores entity IDs. Because the 2D renderer batches many quads into one draw call, the ID travels with each vertex; a per-object uniform cannot change between quads inside that call. Writing both outputs in one pass avoids drawing the geometry a second time. [Multiple Render Targets and Framebuffer Refactor](https://www.youtube.com/watch?v=f-GbHye1VFQ)

**Format clarification:** that episode establishes an entity-ID/color output, but the “integer target” wording above does not establish an integer texture format. Its stated RGBA8 support denotes normalized color storage, which differs from an integer-format attachment. Preserve the picking payload/encoding contract separately from the attachment component type and readback format; inspect the implementation before assigning a precise integer representation to this historical example. Writing selectable identity and display color in one pass is the demonstrated design idea. [Khronos framebuffer component types](https://registry.khronos.org/OpenGL/specs/gl/glspec30.pdf).

The previous framebuffer API always created one RGBA8 target and one depth24-stencil8 target. A specification list lets a caller describe color attachments and an optional depth attachment, so later passes can request different layouts such as a depth-only shadow map or multiple floating-point color targets. On invalidation, the implementation recreates textures from that fixed specification while resize changes dimensions. OpenGL also needs its active draw-buffer list configured for multiple fragment outputs; this episode caps that list at four and uses `GL_NONE` for a depth-only pass.

The experiment writes ordinary color at output location 0 and red at location 1, then displays attachment 1 in the viewport. This demonstrates two targets and confirms that the second texture can be read. The specification looks extensible, but the implementation here supports only RGBA8 and depth24-stencil8; filtering/wrap configuration, richer type conversion, other formats, and capability-based draw-buffer limits remain unfinished. Treat the API as a developing abstraction, not a complete framebuffer format system or general OpenGL reference.

### Drag and manipulate without confusing identity

In a content browser, a drag operation carries a typed payload that identifies what the target should load. The lesson uses the asset path and joins a relative item path with the content root. It finds several practical traps: path strings may be wide, a drop payload may be null, and repeated icons need distinct immediate-mode UI IDs or they all behave like one item. An asset path payload is a locator, not automatically a durable asset identity; validate it at the receiver and define how moved or renamed files resolve ([yjVSs1HtAng](https://www.youtube.com/watch?v=yjVSs1HtAng), 06:14–13:00; 26:11–37:36).

A transform gizmo is a viewport tool that maps pointer movement into an object transform. The selected entity and camera matrices go into a manipulation library; a viewport rectangle scopes drawing and input. Decompose the transform only while the gizmo actively changes it, and keep selection ownership separate so future mouse picking can replace the hierarchy-only selection source. The tutorial uses local manipulation, applies rotation as deltas, offers Q/W/E/R modes, and adds Control snapping with steps for translation, rotation, and scale. It also exposes a hover/focus boolean bug: accept a shortcut when the viewport is hovered or focused, not only when both are true ([Pegb5CZuibU](https://www.youtube.com/watch?v=Pegb5CZuibU), 12:32–27:12; 34:15–49:30). These are UI behavior details from the tutorial; they do not prove a general decomposition is numerically stable for every matrix or transform hierarchy.


#### Components and invariants

Components should describe data with a clear owner and meaning. A transform component should define its units, coordinate convention, and whether its matrix is derived or stored. A camera component should state its projection parameters and how viewport resizing updates aspect ratio. A sprite component should distinguish the source texture from tint and UV selection. A rigid body component should define whether physics or the editor owns the transform while simulation is running.

The public Hazel `Scene` listens for component additions through an internal template hook. This allows the scene to respond when a camera component appears, for example by initializing it against the current viewport size. That is an important pattern: component data and scene invariants should remain synchronized. But avoid making every component addition trigger hidden global work. A future maintainer should be able to locate where invariants are enforced.

##### ECS performance claims need a memory-layout argument

During the pull-request review stream, The Cherno argues against caching a transform matrix by adding substantial cached fields to every transform component. His explanation is that the extra bytes can reduce how many adjacent components fit in CPU cache, while checks and memory traffic can outweigh recomputing a small matrix. The useful lesson is methodological: identify what data a loop touches, account for the added component size, and profile before declaring a cache faster. The stream’s rough byte arithmetic is illustrative rather than a benchmark, and the specific code proposal is not independently inspected here (The Cherno, “Finishing the Pull Requests!,” 00:38:00–00:51:44).

#### Let the world own a changing population

One hard-coded player can be updated directly. Once the workshop contains a player, several moving crates, and temporary projectiles, the world needs a collection of live objects and a rule for visiting them. Adding another object should mean registering it with that collection, not adding another special update call to the application loop.

An inheritance-based design can give objects shared identity, position, and update operations in a base entity type, with concrete types specializing behavior. An entity-component design instead associates identity with selected data and lets systems process the relevant combinations. These are different arrangements of responsibility; an early tutorial using subclasses is not evidence that it has implemented an ECS. In either arrangement, explain who owns the objects, when they are registered, and which pass is allowed to remove them.

Suppose a projectile hits a crate while the world is iterating its object list. A removal request can mark the projectile inactive or queue its identity for retirement. The collection owner then performs removal at a defined boundary. Decide whether an object marked for removal can receive another update, event, render request, or query result before that boundary. Removing an entry during an index-based traversal can shift the next entry and accidentally skip it; immediate destruction can also invalidate borrowed references. The existing handle-and-deferred-removal lab makes this boundary executable for its small model.

A resizable collection does not promise stable element addresses. Adding objects may move storage, even though the logical objects still exist. Keep the identity/storage distinction from the earlier memory section when choosing how other systems refer to these objects.

The fully read early Cherno entities lesson moves from one player to a level-owned collection and describes removal as a state request. The ownership rules and cautions above complete the explanation without treating its live-coded timer, movement formulas, or constructor-access narration as verified implementation. [Caption source: 3D Game Programming — Entities](https://www.youtube.com/watch?v=J3wfaJk_IMw).


#### Geometry as a managed renderable resource

A model can contain several parts, often one geometry for each material. The geometry resource groups vertex and optional index data with its material, while a render record supplies that geometry and a model matrix. This separates “what mesh data exists” from “where this instance is placed.” The renderer packet can then carry an array of geometry draws, and the renderer simply loops through it.

The geometry system owns the higher-level lifetime: callers can acquire an existing geometry by ID, create one from a configuration, and release it. Reference counts and an auto-release option mirror the texture/material systems. Creating geometry asks the renderer to upload vertices and indices, then acquires the named material; if that lookup fails, it uses a default material. Releasing at zero can destroy geometry and release its material. The Vulkan backend stores geometry metadata in a fixed array and records each geometry's vertex/index count, byte size and offsets into shared buffers.

The test plane generator provides a practical way to exercise this path. It creates a grid from width, height, segment counts and texture tiling values. Each cell gets four vertices and six indices, with UVs scaled to repeat the texture. Because it duplicates vertices between adjacent cells and allocates arrays dynamically, the presenter explicitly treats it as test code rather than a production mesh builder; callers must free those arrays.

The draw path binds the shared vertex buffer at the geometry's recorded offset and chooses indexed drawing when index data exists, otherwise a non-indexed draw uses the vertex count. Material application and model push constants become separate steps, which can help when batching objects that share material state. Important infrastructure remains unfinished: ranges are not actually returned to a free list, offsets grow monotonically, and destruction waits for the whole device to idle. The demo reports a fallback texture when a referenced file is absent, then corrects the named asset and shows changed tiling; these are narrated outcomes.

Source reading: [caption source](https://www.youtube.com/watch?v=tHW0T6a0h5E). Captions document the narrated development state; on-screen code and reported builds were not independently reproduced.


#### Trace terrain from authored samples to submitted geometry

Terrain passes through several contracts before pixels appear. A height-map image is authored input; one pixel can become one normalized height sample and one grid vertex. The Kohi prototype steps RGBA data by four bytes, averages RGB while ignoring alpha, and divides by 255; a mistaken extra index multiplication skipped most samples. Image dimensions and samples must be copied into terrain-owned data before the temporary image resource is released. Large maps also exceeded fixed vertex/index buffer capacities in a later stream, where increasing those capacities enabled the reported sample render. [Height-map import](https://www.youtube.com/watch?v=hSvbfxOPCZ0) · [Terrain-owned data and buffer limits](https://www.youtube.com/watch?v=vLzvpFaX50A)

A grid's tile counts set resolution; extents and origin set placement and scale. Each cell becomes two triangles, with UVs following grid coordinates. One source proposes generating this geometry during initialization with flat placeholder heights; sampled elevation and derived normals/tangents remain later work. A terrain section in a scene file is not a runtime terrain: parse it, validate its record, create runtime config and object, register it with the scene, then include it in render data. One parser-to-runtime stream ends with both terrain and render-path counts still zero. [Terrain grid design](https://www.youtube.com/watch?v=unypFSvg7gI) · [Terrain scene parsing](https://www.youtube.com/watch?v=o3VS8tbwaRI)

When terrain exists but draws incorrectly, inspect each representation boundary: CPU vertices before upload, GPU attributes after upload, uniforms after binding, and final draw state. One debugging stream first finds 60,000 vertices at the origin, then incorrect matrix block layout; correcting generation and alignment produces a plane, still upside down. Another scene/render-view refactor replaces an opaque geometry-array pointer with typed scene-owned mesh and terrain arrays; a pointer/value mismatch and an uninitialized terrain array each cause separate failures. These streams report partial progress, not a complete terrain feature. [Terrain rendering diagnostics](https://www.youtube.com/watch?v=N34cyDfZVtA) · [Scene and view packet separation](https://www.youtube.com/watch?v=JgMCoxs0nqo)

A temporary terrain path may remove texture sampling, use fixed colors, or duplicate uniform handling to isolate geometry. Keep such scaffolding marked as provisional; the source ends before a finished terrain render. [Terrain bring-up slice](https://www.youtube.com/watch?v=X9EYh-EfOz4)


### The gameplay foundation is the negotiated boundary between engine and game

The phrase **gameplay foundation** is useful because the border between reusable engine code and one game’s rules is not a clean horizontal line. Gregory places a cluster of systems near that boundary: the runtime object model, world loading and streaming, periodic object updates, messaging/events, scripting, and high-level objectives or game flow. These services can be engineered generically, but their useful shape is usually influenced by genre and project needs. Treat the boundary as a gradient rather than pretending that everything below a directory named Engine is universally reusable.

A particularly important distinction is between the **authoring object model** and the **runtime representation**. The editor may show a designer one conceptual “Door” with transform, mesh, lock state, trigger, sound, and script fields. Runtime code does not need one C++ object containing exactly those fields. It may represent the door through ECS components, handles into rendering and physics systems, a script instance, and an objective-state reference. What must remain faithful is the authored meaning. The conversion must preserve identity, properties, references, and behavior the tools promised, even if the runtime reorganizes the data for locality, streaming, or backend ownership.

World streaming makes that separation unavoidable. A designer thinks in persistent locations and object identities; runtime memory contains only the chunks currently resident. An unloaded object may still be the target of a quest, save-game reference, network identity, or future script action. Therefore “object reference” cannot automatically mean “address of a currently allocated object.” Stable authored identifiers, runtime handles, and residency state answer different questions. The existing identity and streaming sections build those mechanisms; the gameplay-foundation view explains why they are needed by design rather than merely by memory management.

The same applies to updates and messages. A runtime object model needs some way to make changing elements act over time, but “call Update on every object” is only one implementation. Dependencies between animation, physics, AI, scripts, objectives, and rendering determine where state becomes visible. An event can communicate a discrete occurrence—plate pressed, enemy killed, area entered—while durable state answers what remains true. If high-level game flow listens for those events, it should update an explicit objective state rather than infer progression from transient presentation effects.

High-level **game flow** is the part most easily lost when an engine curriculum focuses only on low-level systems. A game can have perfect rendering, physics, scripting, and persistence yet still need a structure that answers: what objective is active, which prerequisites are satisfied, which areas or interactions are gated, what happens on failure, and what becomes possible after success? That structure may be a sequence, graph, state machine, quest system, mission script, or data-driven rule set. Its form should follow the game. The architectural principle is that it consumes meaningful gameplay state and events, not renderer details or editor widget state.

Finally, scripting exists here for iteration and authorship, not because native code is inherently unsuitable. A scripting boundary should expose the stable concepts the game needs—entities, components, queries, events, timers, objectives—while keeping allocator internals, graphics API objects, and unsafe lifetime assumptions behind engine-owned interfaces. The editor, runtime object model, and script API should therefore agree semantically even when they use different physical representations.

Source basis: Gregory, *Game Engine Architecture*, Chapters 13–14, especially §14.1’s gameplay-foundation components, runtime object models, world chunk loading/streaming, object references/queries, real-time updating, event/message passing, scripting, and high-level game flow.

### A game world is a changing state inside a larger designed space

A level is not the same thing as an objective. A **world chunk** is a discrete playable region of the authored world; it may correspond to a level, map, stage, or area and can be managed by a small team. An **objective** is a player goal with a success condition and, usually, a consequence for failure. A game may connect one objective to several chunks, and a chunk may host multiple objectives. That separation lets designers revise the order or meaning of goals without rebuilding the world boundaries around them. The chapter diagrams a typical world split into chunks and a separate objective graph that maps goals onto chunks (PDF pp. 715–717 / printed pp. 693–695; Figure 13.2 and 13.3 inspected on PDF pp. 715 and 717).

The runtime **game state** is the current state of the world elements that change over time. A moving character, a projectile, or a trigger volume can be dynamic; terrain and buildings are often static. The boundary is a design and optimization choice rather than a universal property of an asset: animated waterfalls may still be treated as static if they do not participate in gameplay interactions. Knowing that geometry will not move can permit precomputation or omission of work, while destructible scenery may swap between authored intact, damaged, and destroyed versions. Static and dynamic are therefore ends of a spectrum shaped by interaction and the cost of updating (PDF pp. 712–715 / printed pp. 690–693; Figure 13.1 inspected on PDF p. 713).

Large worlds are divided for more than memory limits. Chunks can control pacing and progression, make an open world manageable, and let different teams create separate regions. A hub-and-spoke layout gives a central area access to branches; a graph can connect regions in less regular ways. These are structural options, not mandatory level-loading architectures. Likewise, **instancing** means drawing or placing repeated copies of a small set of meshes at different transforms, creating variety without authoring every repeated piece as unique geometry. **Brush geometry** describes shapes as bounded convex volumes and can speed up early blockout; if the layout proves useful, artists can refine or replace it with detailed meshes (PDF pp. 714–716 / printed pp. 692–694).

#### A streamed world must preserve authored identity when its objects are absent

A door in world cell 7 may be named by an objective, a save file and a script even while cell 7 is not loaded. Therefore **durable identity** cannot be a raw pointer into a live ECS array. Use an authored key such as `(world ID, cell ID, local object ID)` or a stable GUID, and resolve it through a registry that knows which cell versions are resident. Resolution may return `Unloaded`, `Loading`, `Missing`, or a live runtime handle; those states must not be confused. A sparse-set entity index with a generation protects against reuse *inside a live world*, but it does not automatically provide stable identity across scene exports, runtime resets and streamed cell unload/reload. Store the authored-to-runtime mapping explicitly and invalidate it before destroying a cell's memory.

Walk a three-step scene. A quest registers a reference to `cell=7, object=42` while the player is in cell 3; resolution returns `Unloaded`, but the quest retains the durable ID. When cell 7 becomes resident, the registry maps object 42 to a live, generation-checked ECS handle and queues `OnAvailable` once. When the player leaves, unloading must wait for consumers that have pinned the cell, for outstanding simulation queries and for any GPU work that borrows its buffers. Once retired, a stale direct runtime handle must be rejected even though the *authored* reference remains legitimate. If the quest actually requires a target immediately, that is an explicit streaming/pinning policy—not a reason to assume every referenced object is permanently resident.

Spatial queries also cross residency boundaries. A gameplay radius query can visit the spatial structure of loaded cells while a navigation or quest system requests additional cells. But a query returning “no hit” among loaded cells is **not necessarily evidence that no matching world object exists**. Define whether the query is resident-only, blocks until a region is loaded, returns partial results with a completeness flag, or uses coarse baked proxies for unloaded cells. Deterministic physics must not suddenly begin colliding with an object half-way through an integration step; publish collision worlds at simulation phase boundaries. Visual streaming can obey different deadlines, but cannot silently determine gameplay authority.

The runtime loop now has a meaningfully ordered schedule. At the beginning of fixed tick `k`, accept logical input and approved publication commands; issue simulation-safe stream changes; resolve gameplay references; run scripts/AI that consume snapshot state; evaluate animation motion requests; perform physics and collision; finalize authoritative transforms, tick events and ECS structural mutations; and seal a read-only presentation snapshot. The render loop may interpolate between snapshots and draw at a different rate without executing gameplay events twice. Do not assume this is the only valid ordering—some engines need AI before animation, animation after physics, multiple substeps, or deferred event dispatch—but every read/write dependency needs an unambiguous producer, consumer and boundary. An event emitted during tick `k` and delivered in tick `k+1` must carry enough identity to be processed once and to be logged and replayed.

The world-editor representation is different again: editable source instances, prefab overrides, human-friendly paths, and undo history are authoring data. Runtime ECS archetypes, compact transforms, spatial indices and GPU draw packets are optimized derived data. Export a versioned immutable runtime package with an explicit dependency manifest and source identity; never promise that editing a scene document changes an already executing cell until the approved reload and publication protocol has run. A robust failure test edits/reloads a cell, preserves its authored IDs, verifies that obsolete runtime handles fail, and checks that gameplay events do not replay merely because a visual instance was recreated.

This is the missing bridge between the world-editor/game-object discussions of Gregory's Chapters 13–14 and the much more detailed ECS, serialization and graphics foundations already present in this volume. The important contribution is not a new object hierarchy; it is a precise contract for identity, visibility, simulation authority and memory lifetime across absent and resident worlds.


### Make common searches cheap by choosing them deliberately

Gameplay asks questions such as “which enemies are within ten meters?”, “what did this ray hit?”, or “which destructible objects have low health?” One universal database that answers every possible question with maximum speed is not a realistic design target. Pick the searches the game actually needs and arrange data around them: an id index for identity, maintained groups for common categories, a collision query for paths and visibility, or spatial partitions for nearby objects. These are design options from the chapter, not a claim that one structure fits every world.

For example, a blast can first query a radius to produce nearby candidates, then apply exact distance and gameplay rules to decide who takes damage. The first step narrows work; it does not itself define the game’s damage semantics. (Constructed example.)

(*Game Engine Architecture*, PDF pp. 778–779 / printed pp. 756–757.)

### A useful object needs rules for being noticed, reached, and used

Imagine reaching a dark room with a low battery. In the *Unspeakable* proposal, the flashlight dims and loses range as charge falls. It uses charge while on, spare batteries are inserted when it starts to dim, and some creatures are deterred by its beam; a nearly drained light is less effective. The player can also lose a flashlight when an event breaks it. Its HUD bar and spare-battery count appear while it is on. The map solves a different problem: it pauses play, offers two zoom levels and scrolling, lets the player browse found maps, marks current position and facing, and lists carried quest items. Maps may cover the current, future, or already visited areas; some places have none, and the player can collect an overview map of the whole island. The proposal leaves a placement question open: should ammunition quantities, weapons, puzzle parts, or health packs be randomized, with designer overrides? [Game Design Theory and Practice, PDF pp. 633–634 / printed pp. 608–609]

The Use button checks nearby objects roughly within a 45-degree facing cone, then chooses the closest one. The character may move into a position and play an object-specific animation. A door can open or close, fail because it needs a key or another event, or require damage to break a lock; a used key normally unlocks that door for the rest of the game, and may remain in inventory until the last door in a group is opened. Chests use similar rules but do not let the player walk through them. Switches can be one-use, resettable, or toggled. Quest items are used automatically when the player presses Use at a place that accepts them, and are then removed. A security monitor can switch to a remote camera view; ordinary controls stop working there except Use to exit, and damage to either camera or monitor breaks the connection. A table can have several monitors; a camera may pan, but the player cannot control its motion. A pushable object enters a constrained mode: the character can move forward, backward, or strafe but cannot turn; the object moves one rectilinear grid cell per move, never diagonally, and a blocked object blocks the character. Use exits this mode. These rules make selection, state change, feedback, and exit conditions explicit. They describe a proposal, not original implementation architecture. [PDF pp. 634–636 / printed pp. 609–611]

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/30 - Depth, projection, and shadow-map precision|Previous chapter]] · [[Chapter Edition/03 - Game Engine/32 - Terrain materials and image composition|Next chapter]]
