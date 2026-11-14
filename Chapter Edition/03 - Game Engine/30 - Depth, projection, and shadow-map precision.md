# 30 — Depth, projection, and shadow-map precision

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/29 - Dynamic spatial indices for moving worlds|Previous chapter]] · [[Chapter Edition/03 - Game Engine/31 - Spatial-index serialization and query evidence|Next chapter]]

Depth buffers encode an ordering decision under finite precision, and projection choices determine where that precision is spent. This chapter connects conventional depth, reverse-Z, and shadow comparisons to geometry and camera range.

**By the end:** Derive how projection maps distance to depth and explain precision, bias, and coverage limits in a shadow pass.

**Recall question:** For the chosen near/far range, where is depth precision concentrated and how can you test whether shadow acne is numerical or geometric?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 72763–76641; source-span SHA256 9b739549c6e26330204a1f584cc78ea197418353d28ed6acdd7663971d7bb602. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Conventional depth

Suppose:

$$
near\rightarrow0,
\qquad
far\rightarrow1
$$

with depth comparison `LESS`.

At each full-resolution pixel, depth buffer contains the nearest visible surface:

$$
z_p.
$$

For a coarse 2×2 block to prove an object is hidden behind **all four pixels**, we need the farthest of those nearest occluders:

$$
\boxed{
z_{\text{coarse}}
=
\max(z_0,z_1,z_2,z_3).
}
$$

Why max?

Suppose depths are:

$$
0.1,\ 0.1,\ 0.1,\ 0.8.
$$

An object at depth 0.5 is hidden behind the first three pixels, but would be in front of the 0.8 surface.

Using minimum 0.1 would falsely classify it as occluded.

Using maximum 0.8 does not.

For object's nearest possible depth $z_{\text{obj}}$, conservative occlusion requires:

$$
\boxed{
z_{\text{obj}}>z_{\text{coarse}}
}
$$

for the relevant covered region.

---

**Reverse-Z changes the reduction**

This matters directly for your renderer because you have worked with reverse-Z.

Reverse-Z typically maps:

$$
near\rightarrow1,
\qquad
far\rightarrow0
$$

with `GREATER` depth testing.

Now larger depth means closer.

At full-resolution pixel, nearest visible surface has the **largest** depth.

To obtain a conservative coarse value representing the weakest/farthest occluder across a block, use:

$$
\boxed{
z_{\text{coarse}}
=
\min(z_0,z_1,z_2,z_3).
}
$$

Example:

$$
0.9,\ 0.9,\ 0.9,\ 0.2.
$$

Object nearest depth:

$$
0.5.
$$

It is behind the first three surfaces because:

$$
0.5<0.9,
$$

but in front of the fourth:

$$
0.5>0.2.
$$

Taking minimum:

$$
z_{\text{coarse}}=0.2
$$

prevents us from declaring it occluded.

The conservative reverse-Z condition becomes approximately:

$$
\boxed{
z_{\text{object nearest}}
<
z_{\text{HZB}}
\Rightarrow
\text{occluded}
}
$$

for a fully covered region and compatible depth conventions.

This is the exact opposite reduction direction from conventional Z.

A reverse-Z engine that builds its HZB pyramid using the wrong min/max operation can produce catastrophic false occlusion.

---

**Projecting a bounding volume to HZB**

For object AABB:

$$
B,
$$

we need a conservative screen-space rectangle.

Project its relevant corners using:

$$
p_c=PVMp.
$$

After correct clipping and perspective divide:

$$
p_{ndc}=
\left(
\frac{x_c}{w_c},
\frac{y_c}{w_c},
\frac{z_c}{w_c}
\right).
$$

Find screen bounds:

$$
x_{\min},x_{\max},
y_{\min},y_{\max}.
$$

Also determine the object's nearest possible depth.

Then select a pyramid mip level whose texels roughly match the projected rectangle.

If projected size is:

$$
W\times H
$$

pixels, a common idea is choose:

$$
L\approx
\lfloor
\log_2(\max(W,H))
\rfloor
$$

with implementation-specific adjustment so only a small number of HZB texels cover the rectangle.

The exact choice is a performance policy.

Correctness comes from conservative depth aggregation and conservative projected bounds.

---

**Why coarse HZB works**

Suppose object covers:

$$
128\times128
$$

pixels.

Testing full resolution could mean considering:

$$
16,384
$$

depth samples.

At mip level 7:

$$
128/2^7=1.
$$

One coarse sample summarizes roughly that region.

So instead of pixel-by-pixel occlusion, we ask:

> Is the nearest point of the entire object still behind the conservative farthest occluder covering this entire screen region?

If yes, the whole object can be culled before expensive rendering.

That is spatial pruning in screen space.

---

**HZB is not the same thing as ordinary early-Z**

Early-Z occurs while rasterizing fragments.

The triangle is already submitted and rasterization has begun.

HZB object culling attempts to reject geometry **before** submitting its expensive draw/mesh work.

NVIDIA's occlusion-culling discussions make the same broad distinction: depth testing can cheaply discard individual hidden fragments, while hierarchical/object-level visibility systems aim to avoid processing whole objects or hierarchy nodes when they are known to be hidden. [NVIDIA Developer](https://developer.nvidia.com/gpugems/gpugems/part-v-performance-and-practicalities/chapter-29-efficient-occlusion-culling?utm_source=chatgpt.com)

Hardware GPUs also maintain internal hierarchical depth representations for efficient depth rejection; application-built HZBs are conceptually related but are not necessarily the same data structure as the hardware's private Hi-Z metadata. GPUOpen documents hardware Hi-Z metadata being maintained and resummarized as depth data changes. [AMD GPUOpen](https://gpuopen.com/manuals/rgp_manual/overview_windows/?utm_source=chatgpt.com)

---

**HZB must be conservative**

A false negative in visibility means:

$$
\text{visible object classified occluded}.
$$

The result is missing geometry or flicker.

That is much worse than rendering an unnecessary hidden object.

Therefore if uncertainty exists:

$$
\boxed{\text{assume visible}.}
$$

Examples of uncertainty:

- projected bound crosses the near plane,
- object fills most of screen,
- depth bounds are poorly conditioned,
- previous-frame depth may no longer match,
- camera teleported,
- object moved rapidly,
- HZB rectangle coverage is uncertain.

This repeats the entire conservative-geometry philosophy of M078.

---

**Previous-frame HZB and temporal coherence**

Often the current frame's complete depth is not available before deciding what to render.

One solution uses previous-frame depth.

That introduces one-frame latency.

If an object was hidden last frame but camera suddenly moves and exposes it, blindly trusting old depth can incorrectly cull it.

Countermeasures include:

$$
\text{expand projected bounds}
$$

$$
\text{camera-motion thresholds}
$$

$$
\text{keep recently visible objects visible}
$$

$$
\text{disable occlusion after teleport}
$$

$$
\text{reproject previous depth}
$$

$$
\text{conservative velocity expansion}.
$$

This is the rendering analogue of fat AABBs.

Again:

$$
\text{temporal slack}
\leftrightarrow
\text{robustness}.
$$

Historical hierarchical occlusion systems similarly exploit temporal coherence because visibility tends to persist between nearby frames, though overly trusting it results in additional rendering or visibility errors depending on the policy. [NVIDIA Developer](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-6-hardware-occlusion-queries-made-useful?utm_source=chatgpt.com)

---

**The unifying picture of all structures in Pass 08**

Now compare the central abstraction each structure uses.

| Structure | What gets partitioned? | Best exploitation | Main weakness |
|---|---|---|---|
| Dynamic AABB tree | Object hierarchy | Dynamic bounded objects | Tree quality/updating |
| SAP | Axis intervals | Temporal coherence | Dense overlap |
| Spatial hash | Uniform cells | Uniform-scale local neighborhoods | Cell-size sensitivity |
| Dense grid | Uniform cells in bounded domain | Extreme locality/GPU friendliness | Memory |
| Octree | 3D space recursively | Sparse multiscale worlds | Boundary/large-object issues |
| Loose octree | Overlapping 3D cells | Moving extended objects | More overlap |
| Quadtree | 2D space recursively | Maps/terrain/2D worlds | Same regular-partition issues |
| KD-tree | Space via axis planes | Point nearest-neighbor | Dynamic/extended geometry |
| BSP | Arbitrary half-spaces | Static architectural space | Expensive preprocessing |
| Sphere hierarchy | Object groups by spheres | Cheap tests/rotation | Loose fit |
| Scene graph | Semantic/transform relationships | Ownership/transforms | Not intrinsically spatial |
| Portals/PVS | Visibility connectivity | Static indoor visibility | Static/precompute assumptions |
| HZB | Screen-space depth | Dynamic occlusion | Temporal/projection conservatism |

The deeper point is:

$$
\boxed{
\text{A spatial structure is a hypothesis about the workload.}
}
$$

A hash grid hypothesizes useful locality at one fixed spatial scale.

SAP hypothesizes strong temporal coherence.

KD-tree hypothesizes point-like data and useful axis separation.

Portal/PVS hypothesizes a cell-connected architectural environment.

HZB hypothesizes enough existing screen-space occlusion to make depth rejection worthwhile.

If that hypothesis is false, the data structure can become slower than the simpler alternative.

---

**Choosing between dynamic tree, SAP and spatial hash**

Imagine 50,000 particles uniformly spread through a cubic region, all roughly equal size.

Spatial hash is attractive.

Imagine 5,000 rigid bodies moving smoothly with similar frame-to-frame order.

SAP can be very effective.

Imagine 10,000 differently sized rigid bodies moving arbitrarily through a world.

Dynamic AABB tree is more general.

Imagine millions of static reconstructed triangles.

Static SAH BVH from Pass 07 is the better fit.

Imagine millions of static point samples requiring nearest-neighbor search.

KD-tree is a stronger candidate.

Imagine an indoor level consisting of rooms and corridors.

Portals/PVS can remove entire inaccessible regions before generic spatial culling even begins.

This is why production engines frequently use **several structures simultaneously**.

---

**A realistic engine composition**

A modern world might use:

```text
                    WORLD / ENTITY SYSTEM
                           |
               -------------------------
               |                       |
       static geometry           dynamic objects
               |                       |
             BVH / BLAS          Dynamic AABB tree
               |                       |
               -----------+------------
                          |
                          v
                     candidate set
                          |
                       frustum
                          |
                        PVS
                          |
                         HZB
                          |
                         GPU
```

Meanwhile:

```text
AI points / landmarks
          |
        KD-tree
```

Particle neighborhood queries:

```text
particles
    |
spatial hash
```

Indoor visibility:

```text
cell / portal graph
```

The same world can legitimately contain all of them.

---

**Current KairoSpatial implementation audit**

This pass exposes a useful maturity map.

**`DynamicAABBTree.cppm`**

Already implemented:

```text
binary dynamic tree
tight/fat leaf bounds
proxy handles
incremental insertion
remove/reinsert updates
AABB/sphere/ray queries
layer masks
pair generation
height/balance/area diagnostics
rebuild-through-reinsertion
```

Missing or improvable:

```text
node free-list reuse
rotations/balancing
motion-predictive fattening
stronger insertion SAH
moved-proxy pair generation
true optimized bottom-up rebuild
near-first ray traversal
```

**`SweepAndPrune.cppm`**

Already implemented:

```text
proxy storage
update/remove
X-axis sorting
early X rejection
full 3D candidate confirmation
```

Not yet implemented despite the declared endpoint type:

```text
persistent endpoints
active-set sweep
incremental endpoint crossing
temporal-coherence insertion sort
multi-axis pair masks
```

**`SpatialHash.cppm`**

Already quite coherent:

```text
floor-based cell mapping
negative coordinates
AABB multi-cell occupancy
coordinate hashing
reverse ID->cells mapping
remove/reinsert
AABB query
radius query
deduplication
```

Main limitations are structural rather than correctness errors:

```text
single fixed cell size
large-object cell explosion
unordered_map locality/overhead
full removal/reinsert on every update
```

**`Octree.cppm`**

Implemented as a real loose octree:

```text
8-way subdivision
depth/capacity limits
loose bounds
ancestor storage for large items
AABB/sphere/frustum/ray query
empty-subtree collapse
```

Important improvements:

```text
enforce root/world containment
ID->node proxy map for fast updates
population-based merge
near-first / closest-pruned raycast
pool/flat allocation
layer masks
```

**`Quadtree.cppm`**

Currently:

$$
\boxed{\text{not yet hierarchically subdivided}}
$$

It is a flat item vector with linear queries.

**`KDTree.cppm`**

Implemented well for point queries:

```text
median nth_element build
cyclic axes
nearest neighbor
radius search
K-nearest
max-heap pruning
```

Possible additions:

```text
AABB range query
spread-based axis choice
deterministic tie-breaks
bulk point-data locality optimization
```

**BSP / sphere hierarchy / portal-PVS / HZB**

No dedicated implementations were found in the inspected `KairoSpatial` repository. Those topics are therefore canonical system knowledge and future engine-integration targets rather than existing code descriptions.

---

**One especially useful implementation project from this pass**

The best single improvement to make after learning these chunks would be to turn your dynamic AABB tree into a genuinely production-style update structure.

The sequence would be:

```text
1. Implement free-list node reuse
2. Add displacement-aware fat bounds
3. Add local rotations
4. Add proper insertion SAH / inheritance cost
5. Maintain moved-proxy list
6. Generate pairs only from moved proxies
7. Add near-first ray traversal
8. Benchmark against current implementation
```

Do not implement all eight and merely declare victory.

Measure after each stage:

$$
\text{update time},
$$

$$
\text{pair generation time},
$$

$$
\text{node visits},
$$

$$
\text{area ratio},
$$

$$
\text{tree height},
$$

$$
\text{reinsertion rate}.
$$

That lets you know which features actually pay for your workloads.

---

**Research bridge: your existing adaptive collision hypothesis becomes much more meaningful now**

Your repository already contains the research hypothesis:

```text
Online scene telemetry can choose among:
dynamic tree
sweep-and-prune
spatial hash

such that p95 collision cost is lower
than one fixed policy.
```

Pass 08 gives that hypothesis real algorithmic substance.

The three methods fail under very different conditions.

For dynamic tree, useful telemetry includes:

$$
H=\text{height}
$$

$$
R_A=\text{area ratio}
$$

$$
V=\text{visited nodes/query}
$$

$$
U=\text{reinsertion rate}.
$$

For SAP:

$$
K=\text{endpoint inversions/frame}
$$

$$
A=\text{average active-set size}
$$

$$
P_x=\text{X candidate pairs}.
$$

For hash:

$$
O=\text{occupied cells}
$$

$$
B=\text{mean bucket occupancy}
$$

$$
B_{\max}=\text{max bucket occupancy}
$$

$$
C=\text{mean cells/object}.
$$

Then define estimated total broadphase cost:

$$
C_T
=
C_{\text{update}}
+
C_{\text{query}}
+
C_{\text{candidate narrowphase}}.
$$

A switching policy should minimize **total frame cost**, not merely broadphase query time.

If hash returns twice as many false-positive candidates, its narrowphase cost belongs in the evaluation.

Likewise switching structures has a migration cost:

$$
C_{\text{switch}}.
$$

A policy that switches every frame may be worse than the fixed structures it attempts to outperform.

So hysteresis is necessary:

$$
C_{\text{new}}
<
C_{\text{current}}-\Delta
$$

for several frames before switching.

That connects directly to your repository's existing `no_hysteresis` ablation idea.

This is a genuine systems research direction, provided the final contribution goes beyond simply comparing standard broadphase algorithms.

---

**Pass 08 deep reconstruction map**

You should now mentally see the full phase progression:

```text
PASS 07
STATIC / SEMI-STATIC ACCELERATION

Primitive bounds
       |
       v
      BVH
       |
SAH / Morton / LBVH
       |
Traversal / pruning
       |
Wide / quantized / SIMD


PASS 08
DYNAMIC / SPECIALIZED SPATIAL SYSTEMS

moving AABBs
   |
   +---- Dynamic AABB tree
   |
   +---- Sweep and prune
   |
   +---- Spatial hash / grid

explicit space partition
   |
   +---- Octree / loose octree
   +---- Quadtree
   +---- KD-tree
   +---- BSP

alternative bounding hierarchy
   |
   +---- sphere hierarchy

scene organization
   |
   +---- scene graph

visibility organization
   |
   +---- portals
   +---- PVS
   +---- hierarchical Z
```

These are not random data structures.

They are different answers to:

$$
\boxed{
\text{What information about space is stable enough that I can exploit it?}
}
$$

Dynamic tree exploits bounded movement.

SAP exploits ordering stability.

Hash grids exploit fixed scale.

Octrees exploit coordinate hierarchy.

KD-trees exploit point distributions.

BSP exploits planes.

Portals exploit architectural connectivity.

HZB exploits already-rendered depth.

---

**Parts 1337–1374 coverage check**

| Part | Required subject | Covered |
|---:|---|---|
| 1337 | Dynamic AABB tree node representation | ✓ |
| 1338 | Fat AABBs | ✓ |
| 1339 | Dynamic-tree insertion heuristic | ✓ |
| 1340 | Leaf removal | ✓ |
| 1341 | Proxy updates | ✓ |
| 1342 | Rotations and balancing | ✓ |
| 1343 | Dynamic-tree overlap queries | ✓ |
| 1344 | SAP endpoints | ✓ |
| 1345 | SAP active sets | ✓ |
| 1346 | Temporal coherence | ✓ |
| 1347 | Insertion sort for endpoints | ✓ |
| 1348 | Multi-axis SAP | ✓ |
| 1349 | Spatial-hash cell coordinates | ✓ |
| 1350 | World → cell mapping | ✓ |
| 1351 | Coordinate hashing | ✓ |
| 1352 | Hash bucket collision handling | ✓ |
| 1353 | Moving-object insertion/update | ✓ |
| 1354 | Neighbor/range queries | ✓ |
| 1355 | Uniform grids | ✓ |
| 1356 | Dense versus hashed grids | ✓ |
| 1357 | Octree node representation | ✓ |
| 1358 | Octree subdivision | ✓ |
| 1359 | Octree insertion | ✓ |
| 1360 | Octree range query | ✓ |
| 1361 | Loose octrees | ✓ |
| 1362 | Quadtree representation | ✓ |
| 1363 | Quadtree insertion/subdivision | ✓ |
| 1364 | KD splitting planes | ✓ |
| 1365 | Median KD construction | ✓ |
| 1366 | KD nearest neighbor | ✓ |
| 1367 | KD range queries | ✓ |
| 1368 | KD versus BVH | ✓ |
| 1369 | BSP concept | ✓ |
| 1370 | Bounding-sphere hierarchies | ✓ |
| 1371 | Scene graph vs spatial index | ✓ |
| 1372 | Portals/cells | ✓ |
| 1373 | PVS | ✓ |
| 1374 | Hierarchical-Z occlusion | ✓ |

**Pass 08 — complete.**

You now have **8 / 18 passes completed**, with **10 remaining**.

### Next — Pass 09: M083 + M084, Parts 1375–1400

That finishes this entire spatial phase: **navigation graphs, weighted edges, complete A\*, admissible/consistent heuristics, grids and navmeshes, funnel/string-pulling reasoning, dynamic obstacles, hierarchical pathfinding, then spatial serialization, visualization, build-vs-query benchmarking, workload-driven structure selection, and a unified query interface over the complete KairoSpatial family.**


### Navigation Graphs, A*, Heuristic Theory, Grids, NavMeshes, Funnel/String Pulling, Dynamic Obstacles, Hierarchical Pathfinding, Serialization, Visualization, Benchmarking, Workload Selection, and Unified Spatial APIs

This pass completes the entire spatial-acceleration/navigation phase. **M083 owns Parts 1375–1395 and M084 owns Parts 1396–1400**, so we go from graph representation all the way through A* theory and practical navmeshes, then finish by asking how all the spatial structures from Passes 07–09 should actually be serialized, visualized, benchmarked, selected and exposed through an engine API. Graphics_Engine_Mastery_Integra…

The central transition is this:

$$
\text{Spatial querying asks: "What is where?"}
$$

while navigation asks:

$$
\text{Pathfinding asks: "How can I get from here to there?"}
$$

Those are related but not identical problems. A BVH tells you that a wall occupies space. A navigation representation tells you that the wall prevents movement and therefore changes connectivity. A pathfinder then searches that connectivity, while a movement/steering system eventually turns the abstract route into actual character motion.

---

### NAVIGATION GRAPHS, A*, GRIDS, NAVMESHES AND HIERARCHICAL PATHFINDING

---

**Navigation-graph representation**

The most general navigation abstraction is a weighted directed graph:

$$
G=(V,E).
$$

$V$ is a set of states or locations, and $E$ is a set of legal transitions.

For navigation:

$$
v_i\in V
$$

might represent a waypoint, grid cell, navmesh polygon, room, road junction, ladder endpoint, teleport destination, or any other traversable state.

An edge

$$
e=(u,v)
$$

means:

> an agent may legally travel from node $u$ to node $v$.

Your Kairo representation is intentionally simple:

```cpp
struct GraphNode final
{
    NavigationNodeID ID;
    Vec3f Position;
};

struct GraphEdge final
{
    NavigationNodeID From;
    NavigationNodeID To;
    float Cost;
};
```

and the graph stores:

```cpp
std::vector<GraphNode> m_Nodes;

std::unordered_map<
    NavigationNodeID,
    std::size_t
> m_NodeLookup;

std::unordered_map<
    NavigationNodeID,
    std::vector<GraphEdge>
> m_Adjacency;
```

This is an **adjacency-list graph**.

For every node $u$:

$$
Adj(u)
=
\{
(u,v_1),
(u,v_2),
\dots
\}.
$$

That is the representation A* wants, because from the current node we usually care only about outgoing neighbors.

---

### Why not an adjacency matrix?

For $N$ nodes an adjacency matrix stores:

$$
N^2
$$

possible connections.

If:

$$
N=100\,000,
$$

that means:

$$
10^{10}
$$

matrix entries.

Navigation graphs are generally sparse. A grid node may have only 4 or 8 neighbors; a navmesh polygon may have perhaps 3–8 neighbors.

If average degree is $d\ll N$, an adjacency list uses approximately:

$$
O(N+Nd)
=
O(N+E).
$$

That is vastly more appropriate.

---

**Stable IDs versus storage indices**

Your graph separates:

$$
NavigationNodeID
$$

from:

$$
std::size_t\text{ vector index}.
$$

That is good engine architecture.

Suppose:

```text
ID 500 → vector slot 3
ID 17  → vector slot 4
ID 92  → vector slot 5
```

External systems can keep ID 500 even if internal storage moves.

The map:

```cpp
m_NodeLookup[id] -> index
```

provides the translation.

But note the current `RemoveNode()` behavior. It erases from the middle of `m_Nodes`:

```cpp
m_Nodes.erase(...)
```

then runs:

```cpp
RebuildLookup();
```

Therefore removal is at least:

$$
O(N)
$$

for vector movement and rebuilding, plus the cost of scanning adjacency lists to remove incoming edges.

For relatively static authored navigation graphs this is fine.

For graphs with heavy real-time node insertion/removal, sparse-set/generational-handle patterns from the next phase would be superior.

You are about to see why Phase 15's ECS/handle material naturally follows this phase.

---

**Directed versus bidirectional connectivity**

A physical connection is not always symmetric.

A staircase may be traversable both ways:

$$
A\leftrightarrow B.
$$

A one-way drop may permit:

$$
A\rightarrow B
$$

but not:

$$
B\rightarrow A.
$$

A jump pad, escalator, directional road lane, locked doorway, ledge drop or teleport can all produce asymmetric connectivity.

Your `AddEdge()` supports:

```cpp
bool bidirectional = true;
```

so:

```cpp
graph.AddEdge(A, B, cost, true);
```

creates:

$$
A\rightarrow B
$$

and:

$$
B\rightarrow A.
$$

Whereas:

```cpp
graph.AddEdge(A, B, cost, false);
```

creates only:

$$
A\rightarrow B.
$$

That distinction becomes essential for admissible pathfinding: A* searches the graph that actually exists, not a geometric guess about connectivity.

---

**Weighted navigation edges**

The shortest path is not necessarily the path with the fewest edges.

Suppose:

```text
A ----1---- B ----1---- C

 \                       ^
  \------ 100 -----------/
```

A → C is one edge but costs 100.

A → B → C has two edges but costs:

$$
1+1=2.
$$

Path cost is:

$$
\boxed{
g(P)=
\sum_{e\in P}c(e)
}
$$

where every edge has weight:

$$
c(e)\ge0.
$$

Your graph enforces:

```cpp
if (!std::isfinite(cost) ||
    cost < 0.0f)
{
    throw ...
}
```

That nonnegative-cost restriction is fundamental for Dijkstra/A* assumptions.

---

**What can an edge cost represent?**

Do not think:

$$
cost = distance
$$

by definition.

Cost is whatever objective the navigation system is minimizing.

A useful model might be:

$$
c(e)
=
w_dD(e)
+
w_tT(e)
+
w_rR(e)
+
w_nN(e),
$$

where $D$ is distance, $T$ estimated traversal time, $R$ danger/risk, and $N$ noise/exposure or some game-specific penalty.

Suppose two routes exist:

```text
Route 1:
short open road
distance = 20
danger   = 100

Route 2:
long covered tunnel
distance = 35
danger   = 5
```

An ordinary civilian NPC might minimize time.

A stealth NPC may heavily penalize exposure.

A wounded NPC may penalize danger.

The graph remains identical while edge costs change.

That is one reason graph search is so powerful: geometry and objective can remain separate.

---

**State expansion when costs depend on more than location**

There is a deeper issue.

Suppose entering a locked room is possible only after acquiring a key.

Then node:

$$
RoomA
$$

alone no longer describes search state.

We need:

$$
(RoomA,\ hasKey)
$$

versus:

$$
(RoomA,\ !hasKey).
$$

Similarly, if movement cost depends on remaining fuel, stance, weapon state, time of day, door state or vehicle type, the pathfinding state may need to include those variables.

So:

$$
\boxed{
\text{graph node}
\neq
\text{necessarily physical location}.
}
$$

It represents the minimum state required for the Markov property:

> once current state is known, future transition costs should not depend on forgotten history.

This perspective becomes important in advanced AI planning.

---

**A* open set**

A* maintains candidate nodes that have been discovered but not yet fully processed.

This is the **open set**.

A* repeatedly chooses the node with smallest:

$$
f(n).
$$

Therefore the natural data structure is a **min-priority queue**.

Conceptually:

```text
open

node   f
-----------
A      12
B       7   <- pop this
C      19
D       9
```

Your implementation uses:

```cpp
std::priority_queue<OpenNode> open;
```

C++'s `priority_queue` is normally a max-heap.

So your `OpenNode::operator<` reverses priority comparison:

```cpp
bool operator<(const OpenNode& other) const noexcept
{
    return Priority > other.Priority;
}
```

which effectively makes the smallest priority appear at the top.

---

**Why a binary heap?**

A binary heap gives:

$$
\text{push}=O(\log N)
$$

and:

$$
\text{pop-min}=O(\log N).
$$

A* can perform both operations many times.

A plain unsorted vector gives:

$$
O(1)
$$

insertion but:

$$
O(N)
$$

minimum extraction.

A sorted vector gives cheap extraction but:

$$
O(N)
$$

insertion.

A heap is a good general compromise.

Other structures can be better for special cases: bucket queues for bounded integer costs, radix heaps for monotonic integer keys, specialized priority queues for huge navigation workloads.

But the binary heap remains the standard general-purpose implementation.

---

**The decrease-key problem**

Suppose node X is already in open with:

$$
g(X)=15.
$$

Later we discover a better route:

$$
g(X)=8.
$$

Ideally we would decrease its heap priority.

`std::priority_queue` does not provide efficient arbitrary decrease-key.

Your implementation uses the standard duplicate-entry technique.

Instead of modifying the old entry:

```text
X cost 15
```

push another:

```text
X cost 8
```

Now both exist.

When popping:

```cpp
if (next.Cost != gScore.at(next.ID))
    continue;
```

the stale entry is ignored.

Thus:

```text
heap

X g=8   <- useful
X g=15  <- stale
```

This avoids complex heap-handle management.

It increases heap entries somewhat but is often an excellent engineering tradeoff.

---

**Closed sets**

Textbook A* often maintains:

```text
OPEN
CLOSED
```

OPEN contains discovered candidates.

CLOSED contains nodes whose best path has been finalized.

With a **consistent heuristic**, once node $n$ is popped with minimum $f$, its $g(n)$ is final.

It can safely enter CLOSED.

Your generic `FindPathAStar()` interestingly has **no explicit closed set**.

It stores:

```cpp
gScore
cameFrom
open
```

and permits nodes to be inserted again if a better route appears.

That means nodes can effectively be reopened.

This makes the implementation more tolerant of heuristic behavior than a strict no-reopening closed set.

---

**Your NavMesh A* does have a closed set**

Inside `NavMesh::FindPath()` the implementation uses:

```cpp
std::vector<bool> closed(count, false);
```

and once a polygon is popped:

```cpp
closed[current.Index] = true;
```

Then neighbors that are already closed are skipped:

```cpp
if (closed[next])
    continue;
```

This version depends more strongly on the heuristic being consistent.

That difference between the two Kairo searches is subtle but important.

The generic graph A* can reopen improved nodes.

The navmesh A* does not.

We will return to this when we derive consistency.

---

**The g score**

A* defines:

$$
\boxed{
g(n)
=
\text{best known cost from start to }n
}
$$

Initially:

$$
g(start)=0.
$$

All unknown nodes are conceptually:

$$
g(n)=\infty.
$$

Suppose current node is $u$, and edge:

$$
u\rightarrow v
$$

has cost $c(u,v)$.

Candidate path to $v$:

$$
g_{\text{tentative}}(v)
=
g(u)+c(u,v).
$$

If:

$$
g_{\text{tentative}}(v)<g(v),
$$

we found a better path.

So:

$$
g(v)
\leftarrow
g_{\text{tentative}}(v).
$$

Your code:

```cpp
const float tentative =
    gScore[current] + edge.Cost;

if (!gScore.contains(edge.To) ||
    tentative < gScore[edge.To])
{
    cameFrom[edge.To] = current;
    gScore[edge.To] = tentative;
    ...
}
```

This operation is called **relaxation**.

It is the core operation behind Dijkstra, A*, Bellman-Ford and many shortest-path algorithms.

---

**Why relaxation works**

Suppose current best route to V costs 20.

Now we discover:

$$
S\rightarrow A\rightarrow V
$$

costing:

$$
5+7=12.
$$

Then 20 is no longer relevant.

We replace:

$$
g(V)=20
$$

with:

$$
g(V)=12.
$$

The predecessor also changes:

$$
cameFrom[V]=A.
$$

A* gradually improves upper bounds on optimal path costs until nodes become finalized.

---

**Floating-point path costs**

Your implementation rejects nonfinite edge costs and checks accumulated cost:

```cpp
if (!std::isfinite(tentative))
    continue;
```

This is useful.

Even finite float edges can overflow after accumulation.

If:

$$
g(u)\approx3\times10^{38}
$$

and:

$$
c(u,v)\approx3\times10^{38},
$$

their sum becomes infinity in 32-bit float.

Putting NaN/∞ values into a heap comparator can destroy ordering assumptions.

Your code explicitly guards against that.

For ordinary game-scale navigation this may seem excessive, but these are the edge conditions that make foundational libraries reliable rather than merely functional.

---

**The heuristic h score**

A*'s second quantity is:

$$
\boxed{
h(n)=
\text{estimated remaining cost from }n\text{ to goal}.
}
$$

If:

$$
h(n)=0,
$$

the algorithm has no idea where the goal lies.

It behaves like Dijkstra.

If $h$ accurately predicts remaining cost without overestimating, A* can ignore enormous regions of the graph.

Consider:

```text
          irrelevant branch
             /
S ----------A--------- goal
             \
          irrelevant branch
```

Dijkstra explores primarily by distance from start.

A* biases search toward states likely to lead toward the goal.

The heuristic therefore injects **domain knowledge** into otherwise generic shortest-path search. The original Hart–Nilsson–Raphael formulation formalized exactly this use of heuristic information in minimum-cost graph search. [Stanford AI Lab](https://ai.stanford.edu/~nilsson/OnlinePubs-Nils/PublishedPapers/astar.pdf?utm_source=chatgpt.com)

---

**f score**

A* combines known and estimated costs:

$$
\boxed{
f(n)=g(n)+h(n).
}
$$

Interpretation:

$$
g(n)
=
\text{what the path has already cost}
$$

while:

$$
h(n)
=
\text{what we estimate remains}.
$$

Therefore:

$$
f(n)
=
\text{estimated complete path cost through }n.
$$

A* expands the node with the smallest predicted total.

Suppose:

```text
Node A:
g = 3
h = 20
f = 23

Node B:
g = 10
h = 4
f = 14
```

Even though B is more expensive to reach, it appears much closer to the goal overall.

A* chooses B.

---

**A* is not “greedy best-first + Dijkstra”**

It is tempting to think:

```text
Dijkstra uses g
Greedy best-first uses h
A* uses g+h
```

That is directionally useful, but the deeper interpretation is stronger.

With admissible $h$:

$$
f(n)
$$

acts as a **lower bound** on the cost of the best solution passing through $n$.

A* therefore performs a branch-and-bound-like search:

> explore nodes whose optimistic total cost is competitive with the best possible solution.

This is conceptually similar to BVH pruning.

BVH:

$$
\text{lower geometric bound}>
\text{best hit}
\Rightarrow
\text{prune}.
$$

A*:

$$
\text{optimistic route estimate}
$$

orders which partial solutions deserve exploration.

This connection is why spatial acceleration and graph search fit so naturally in one phase.

---

**cameFrom / parent map**

Knowing:

$$
g(goal)
$$

does not tell us the actual path.

Whenever relaxation improves node V through U:

```cpp
cameFrom[V] = U;
```

This records:

$$
V\leftarrow U.
$$

Suppose search discovers:

```text
cameFrom[D] = C
cameFrom[C] = B
cameFrom[B] = A
cameFrom[A] = S
```

Then the predecessor chain is:

```text
D ← C ← B ← A ← S
```

The actual forward path is:

```text
S → A → B → C → D
```

The parent map represents a shortest-path tree rooted at the start, or at least the portion discovered so far.

---

**Path reconstruction**

Start from goal:

```cpp
step = goal;
```

Walk backwards:

```cpp
while (step != start)
{
    path.push_back(step);
    step = cameFrom[step];
}
```

Finally push start and reverse.

Your generic implementation does exactly this.

If path length contains $L$ nodes:

$$
T_{\text{reconstruct}}
=
O(L).
$$

There is no reason to store full path prefixes for every open node.

Doing so would duplicate enormous amounts of memory.

The predecessor relation is enough.

---

**Path reconstruction failure modes**

If goal is reached but:

```text
cameFrom chain
```

fails to reach start, either the search state is corrupted or the implementation incorrectly marked the goal as reachable.

Your `NavMesh` code explicitly detects:

```cpp
if (parent[cursor] == count)
    throw std::logic_error(...)
```

while reconstructing.

That is a strong invariant check.

Another case is:

$$
start=goal.
$$

The generic A* naturally returns a single-node path.

The navmesh has a related but geometrically richer case: if start and end are within the same polygon, no graph traversal is needed and the route can simply be:

```text
start → end
```

with a one-polygon corridor.

Your tests verify exactly that.

---

**Euclidean heuristic**

For ordinary continuous geometric movement:

$$
\boxed{
h_E=
\sqrt{
(\Delta x)^2+
(\Delta y)^2+
(\Delta z)^2
}
}
$$

with:

$$
\Delta x=x_g-x_n
$$

and equivalent Y/Z.

Why is Euclidean distance attractive?

Because any physically traversed path from $n$ to goal must have length at least the straight-line distance:

$$
L_{\text{path}}
\ge
\|goal-n\|.
$$

If movement cost per meter is at least $k$:

$$
c(P)\ge kL(P),
$$

then:

$$
h(n)=k\|goal-n\|
$$

is a lower bound on remaining cost.

This is the important part.

Euclidean **distance** is admissible only if the graph's cost units support it.

---

**The common Euclidean-heuristic bug**

Suppose:

```text
A ---------------- B

distance = 100
edge cost = 1
```

Raw Euclidean heuristic says:

$$
h(A)=100.
$$

But the actual optimal remaining cost is:

$$
h^*(A)=1.
$$

Therefore:

$$
h(A)>h^*(A).
$$

The heuristic overestimates and is inadmissible.

This can happen with:

- teleporters,
- highways with speed bonuses,
- elevators where cost is time instead of distance,
- abstract edges,
- game-defined cost units.

Your repository previously encountered exactly this class of failure, as evidenced by the regression test where a detour over a node 100 units away costs only 2.

---

**Kairo's scaled Euclidean heuristic**

Your generic A* now calculates:

$$
\alpha=
\min_{(u,v)\in E,\ d(u,v)>0}
\frac{c(u,v)}{d(u,v)}.
$$

Then:

$$
\boxed{
h(n)=
\alpha\,d(n,goal).
}
$$

This is a very interesting general solution.

For every edge:

$$
\alpha
\le
\frac{c(u,v)}{d(u,v)}.
$$

Therefore:

$$
\alpha d(u,v)
\le
c(u,v).
$$

Now take any path:

$$
n=v_0,v_1,\dots,v_k=goal.
$$

By triangle inequality:

$$
d(n,goal)
\le
\sum_i d(v_i,v_{i+1}).
$$

Multiply by $\alpha$:

$$
\alpha d(n,goal)
\le
\sum_i
\alpha d(v_i,v_{i+1}).
$$

And because:

$$
\alpha d(v_i,v_{i+1})
\le
c(v_i,v_{i+1}),
$$

we get:

$$
\boxed{
h(n)
\le
\sum_i c(v_i,v_{i+1})
}
$$

for every possible path.

Therefore:

$$
h(n)\le h^*(n).
$$

The heuristic is admissible.

---

**The same construction proves consistency**

Consistency requires:

$$
\boxed{
h(u)\le c(u,v)+h(v)
}
$$

for every edge $u\rightarrow v$.

Using triangle inequality:

$$
d(u,g)
\le
d(u,v)+d(v,g).
$$

Multiply by $\alpha$:

$$
\alpha d(u,g)
\le
\alpha d(u,v)
+
\alpha d(v,g).
$$

Because:

$$
\alpha d(u,v)
\le
c(u,v),
$$

we obtain:

$$
h(u)
\le
c(u,v)+h(v).
$$

Therefore the scaled Euclidean heuristic is consistent as well.

That is a strong property of your current generic A* implementation.

---

**Why zero-cost travel collapses to Dijkstra**

Suppose there is a nonzero-distance edge:

$$
d(u,v)>0
$$

with:

$$
c(u,v)=0.
$$

Then:

$$
\frac{c(u,v)}{d(u,v)}
=
0.
$$

Therefore:

$$
\alpha=0.
$$

And:

$$
h(n)=0.
$$

So the search automatically becomes Dijkstra.

That is exactly what must happen if zero-cost long-distance transitions exist, because ordinary geometry may no longer provide a useful positive lower bound on cost.

Your test suite explicitly checks zero-cost travel links.

---

**Manhattan heuristic**

For a grid where movement is restricted to axis directions:

```text
← ↑ → ↓
```

in 2D:

$$
\boxed{
h_M=
|\Delta x|+|\Delta y|.
}
$$

In 3D:

$$
h_M=
|\Delta x|+
|\Delta y|+
|\Delta z|.
$$

Suppose target differs by:

$$
\Delta x=4,
\qquad
\Delta y=3.
$$

With four-neighbor movement, every legal route requires at least:

$$
4+3=7
$$

steps.

Thus:

$$
h_M=7.
$$

Euclidean would give:

$$
5,
$$

which is admissible but less informed.

A tighter admissible heuristic usually means fewer expansions.

---

**Why Manhattan fails for diagonal movement**

Suppose diagonal movement is allowed with cost:

$$
\sqrt2.
$$

Start:

$$
(0,0)
$$

goal:

$$
(1,1).
$$

Actual optimal cost:

$$
\sqrt2\approx1.414.
$$

Manhattan:

$$
|1|+|1|=2.
$$

Since:

$$
2>1.414,
$$

Manhattan overestimates.

It is no longer admissible.

A heuristic must match the movement model.

---

**Octile heuristic**

For an 8-connected 2D grid:

```text
N S E W
NE NW SE SW
```

let:

$$
dx=|\Delta x|,
\qquad
dy=|\Delta y|.
$$

Assume cardinal move cost:

$$
D
$$

and diagonal cost:

$$
D_2.
$$

Use diagonals for:

$$
\min(dx,dy)
$$

steps.

Then remaining straight steps:

$$
|dx-dy|.
$$

So:

$$
\boxed{
h_O
=
D\max(dx,dy)
+
(D_2-D)\min(dx,dy)
}
$$

which is algebraically equivalent to:

$$
D(dx+dy)
+
(D_2-2D)\min(dx,dy).
$$

For:

$$
D=1,
\qquad
D_2=\sqrt2,
$$

we get:

$$
\boxed{
h_O
=
\max(dx,dy)
+
(\sqrt2-1)\min(dx,dy).
}
$$

This is the exact unobstructed grid cost for an 8-connected grid with those move costs.

---

**Corner cutting matters**

Suppose diagonal movement from:

```text
A #
# B
```

is technically geometrically diagonal but both adjacent cardinal cells are blocked.

Can A move directly to B?

Different games choose different answers.

If no corner cutting is allowed, a diagonal neighbor should require sufficient clearance through adjacent cardinal cells.

That affects reachability but usually not the basic octile lower-bound argument.

For agents with radius, grid occupancy should also be expanded/eroded accordingly.

A point-agent path through a one-cell diagonal crack may be impossible for a physical character.

---

**Dijkstra as A* with zero heuristic**

Set:

$$
h(n)=0.
$$

Then:

$$
f(n)=g(n).
$$

A* now always expands the discovered node with smallest path cost from start.

That is Dijkstra's algorithm.

So:

$$
\boxed{
Dijkstra
=
A^*\text{ with }h=0.
}
$$

This is not merely a cute fact.

It gives a debugging baseline.

If A* returns a path costing more than Dijkstra's result under nonnegative edges:

$$
\text{something is wrong}.
$$

Usually the culprit is:

- an inadmissible heuristic,
- incorrect closed-set handling,
- stale priority logic,
- overflow,
- bad graph connectivity.

---

**Heuristic admissibility**

A heuristic is admissible if:

$$
\boxed{
0\le h(n)\le h^*(n)
}
$$

where $h^*(n)$ is the true optimal remaining cost.

It never overestimates.

Think of it as an **optimistic lower bound**.

Suppose true remaining cost is 20.

These are admissible:

$$
h=0,\quad5,\quad18,\quad20.
$$

This is inadmissible:

$$
h=25.
$$

The closer an admissible heuristic gets to $h^*$, the more informed it is.

In the ideal impossible case:

$$
h(n)=h^*(n),
$$

A* effectively knows the remaining optimal cost already.

---

**Why overestimation can break optimality**

Suppose:

```text
        A ---- 2 ---- Goal
       /
Start
       \
        B ---- 1 ---- Goal
```

Assume total route through A really costs 10, through B costs 5.

But heuristic says:

$$
h(B)=100.
$$

A* may completely deprioritize B and reach/terminate at the more expensive goal route first, depending on implementation and termination conditions.

The guarantee is gone.

An inadmissible heuristic can still be intentionally useful in games if faster, slightly suboptimal routes are acceptable.

Weighted A* uses:

$$
f(n)=g(n)+w h(n),
\qquad
w>1.
$$

That trades optimality for speed.

But that must be a deliberate algorithmic policy, not an accidental heuristic bug.

---

**Heuristic consistency**

Consistency, also called monotonicity, requires:

$$
\boxed{
h(u)
\le
c(u,v)+h(v)
}
$$

for every edge.

Interpretation:

> our estimated distance from U to goal should not exceed the cost of stepping to V plus our estimate from V onward.

It is essentially a triangle inequality in cost space.

Consistency implies admissibility when:

$$
h(goal)=0.
$$

It also implies that along any path:

$$
f
$$

does not decrease after relaxation in the problematic way that would require reopening finalized nodes.

This is why closed-set A* becomes much simpler with a consistent heuristic.

---

**Admissible but inconsistent heuristics**

A heuristic can be admissible globally but violate consistency locally.

Then a node that was previously expanded may later receive a better $g$.

If the implementation permanently closes it and refuses reopening, optimality can fail.

Therefore:

```text
consistent heuristic
    ↓
closed node never needs reopening

admissible but inconsistent heuristic
    ↓
reopening may be necessary
```

Your generic A* has no permanent closed set, so it naturally allows improvements.

Your navmesh search permanently closes polygons, so its heuristic should be consistent.

That brings us to an important repository issue.

---

**A real NavMesh heuristic issue in current Kairo**

Your navmesh edge cost is:

$$
c(i,j)
=
d(C_i,C_j)
\cdot
\frac{
AreaCost_i+AreaCost_j
}{2}.
$$

Your navmesh heuristic is:

$$
h(i)
=
d(C_i,C_g).
$$

If every polygon has:

$$
AreaCost\ge1,
$$

then:

$$
c(i,j)\ge d(C_i,C_j)
$$

and Euclidean centroid distance is consistent.

But `AddPolygon()` currently validates only:

$$
AreaCost>0.
$$

It allows:

$$
AreaCost=0.1.
$$

Then an edge of geometric length 10 may cost:

$$
1.
$$

Yet the raw Euclidean heuristic may estimate:

$$
10.
$$

That can overestimate.

Because navmesh A* also permanently closes nodes, this is not merely theoretical.

A clean fix is either to enforce:

$$
AreaCost\ge1
$$

or calculate a valid heuristic scale analogous to the generic graph A*:

$$
\alpha
=
\min
\text{effective cost multiplier}.
$$

Then:

$$
h(i)
=
\alpha d(C_i,C_g).
$$

That would preserve admissibility/consistency for discounted areas.

---

**Navigation grids**

A navigation grid maps walkable space into discrete cells.

Example:

```text
. . . # . .
. S . # . .
. . . # . .
. . . . . G
```

Each walkable cell becomes a node.

Edges connect legal neighbors.

With four-neighbor movement:

$$
degree\le4.
$$

With eight-neighbor movement:

$$
degree\le8.
$$

In 3D voxel navigation:

$$
degree
$$

can become 6, 18 or 26 depending on movement rules.

---

**Grid coordinate → node identity**

For a dense 2D grid width $W$:

$$
index=x+Wy.
$$

Reverse:

$$
x=index\bmod W,
$$

$$
y=\left\lfloor
\frac{index}{W}
\right\rfloor.
$$

This lets navigation avoid storing an explicit adjacency vector for every cell.

Neighbors can be generated procedurally.

For example:

```cpp
constexpr int offsets[4][2] =
{
    { 1,  0 },
    {-1,  0 },
    { 0,  1 },
    { 0, -1 }
};
```

That can make grids extremely compact.

---

**Grid resolution tradeoff**

Smaller cells give more detailed paths.

But node count scales rapidly.

For a 1000 m × 1000 m map:

At 1 m:

$$
10^6
$$

cells.

At 0.5 m:

$$
4\times10^6.
$$

At 0.1 m:

$$
100\times10^6.
$$

So halving cell size in 2D quadruples cell count:

$$
N\propto\frac1{h^2}.
$$

In 3D:

$$
N\propto\frac1{h^3}.
$$

Resolution therefore directly controls memory and A* search-space size.

This is one reason navmeshes are attractive for continuous walkable surfaces.

---

**Clearance and agent size in grids**

A grid marking only obstacle centers is insufficient.

Suppose an agent radius is:

$$
r.
$$

We need to ensure its center never comes within $r$ of obstacles.

One viewpoint is to Minkowski-expand obstacles by the agent radius:

$$
Obstacle'
=
Obstacle\oplus Sphere(r).
$$

Then pathfind a point-agent center through the remaining free space.

This same conceptual transformation appears in navmesh generation: walkable space is eroded according to agent radius.

Navigation should operate on the configuration space of the agent, not merely raw world geometry.

---

**Navigation meshes**

A navigation mesh represents walkable free space using convex polygons.

Instead of:

```text
hundreds of tiny grid cells
```

an open floor may become one large polygon.

Example:

```text
+--------------------+
|                    |
|     polygon A      |
|                    |
+----------+---------+
           |
+----------+---------+
|     polygon B      |
+--------------------+
```

Each polygon becomes a graph state.

Shared traversable edges become graph edges.

This gives two levels:

$$
\text{geometric representation}
$$

and:

$$
\text{graph representation}.
$$

That is why navmeshes are so efficient for ordinary character navigation.

---

**Why convex polygons?**

Inside a convex polygon, any line segment connecting two points stays inside the polygon.

For any:

$$
p,q\in P,
$$

convexity guarantees:

$$
(1-t)p+tq\in P
\quad
\forall t\in[0,1].
$$

Therefore once the agent enters a convex nav polygon, local movement within it is straightforward.

Complex concave walkable regions are decomposed into multiple convex polygons.

Your `NavMesh::ValidatePolygon()` explicitly checks convexity in XZ projection.

It also checks:

- at least 3 vertices,
- at most 64,
- finite coordinates,
- nondegenerate projected area,
- coplanarity,
- a surface normal with meaningful Y component.

So this navmesh is specifically designed for mostly horizontal walkable surfaces represented in XZ.

---

**Navigation mesh path pipeline**

A complete navmesh route is not simply:

```text
A* → final path
```

It is:

```text
world-space start/end
        |
        v
locate/snap to polygons
        |
        v
A* over polygon adjacency
        |
        v
polygon corridor
        |
        v
portal sequence
        |
        v
funnel/string pulling
        |
        v
waypoints
        |
        v
steering / locomotion
```

Each stage solves a different problem.

Your current Kairo implementation already contains the first five stages.

That is significantly more complete than a mere waypoint graph.

---

**Start/end polygon location**

Your navmesh first attempts:

```cpp
FindContainingPolygon(...)
```

using point-in-convex-XZ plus vertical tolerance.

If no polygon contains the point, it can snap to:

```cpp
FindNearestPolygon(...)
```

within `snapDistance`.

This is important because animation/physics can place a character slightly off the mathematical navmesh.

Without snapping, tiny numerical errors could cause every path query to fail.

But snapping should be bounded.

A character 50 m outside the navmesh should not silently teleport logically to the nearest polygon.

---

**Current polygon-location complexity**

`FindContainingPolygon()` scans every polygon.

`FindNearestPolygon()` also scans every polygon.

For $P$ navmesh polygons:

$$
O(P)
$$

per location query.

For small meshes this is fine.

For a huge open-world navmesh, use a spatial index:

```text
navmesh polygon bounds
        |
        v
BVH / grid / tile lookup
        |
        v
small candidate set
        |
        v
exact polygon test
```

This is a perfect example of the previous passes supporting navigation.

The navmesh does not replace spatial indexing; it can itself be spatially indexed.

---

**Navmesh polygon adjacency**

Two polygons are adjacent if they share a traversable boundary segment.

That segment becomes a **portal**.

Your representation is:

```cpp
struct NavMeshPortal final
{
    NavPolygonID From;
    NavPolygonID To;

    Vec3f Left;
    Vec3f Right;
};
```

Note the portal is directed.

Traveling:

$$
A\rightarrow B
$$

has a left/right orientation.

Traveling:

$$
B\rightarrow A
$$

reverses the viewpoint, so portal orientation must be recomputed.

This is necessary for funnel string pulling.

---

**Building adjacency in Kairo**

Current `BuildAdjacency()` compares every polygon pair:

$$
(A,B).
$$

Then `SharedEdge()` compares all edges of A against all edges of B looking for endpoints that match within:

$$
\epsilon.
$$

For polygons A and B with edge counts $E_A,E_B$:

$$
O(E_AE_B)
$$

for that pair.

Across $P$ polygons, naïvely:

$$
O(P^2E^2)
$$

for roughly equal edge count.

For a few hundred polygons this may be acceptable.

For hundreds of thousands, it is not.

---

**Edge-map adjacency construction**

A more scalable build creates a canonical edge key.

For each polygon edge:

$$
(a,b),
$$

quantize/weld endpoints and create unordered key:

$$
key=\{\min(a,b),\max(a,b)\}.
$$

Then:

```text
edge key
  |
  +-- polygon 17
  +-- polygon 42
```

Two polygons appearing under the same key are adjacent.

That can reduce adjacency construction toward the total number of edges, assuming effective hashing.

This is especially natural for navmeshes generated from indexed topology.

---

**T-junction issue**

Your current `SharedEdge()` expects the two edge endpoints to match within epsilon.

Suppose:

```text
Polygon A edge:
A0 --------------------------- A1

Polygon B:
          B0 -------- B1
```

B's edge is a strict subsegment of A's edge.

They geometrically share a portal but their edge endpoints do not match pairwise.

The current algorithm may fail to connect them.

A professionally generated navmesh usually removes such topological T-junction ambiguity during construction, or adjacency construction computes overlapping collinear segments rather than exact edge-pair equality.

So this is another important contract:

$$
\boxed{
\text{Kairo currently assumes authored/welded compatible polygon boundaries}.
}
$$

---

**Area costs on navmesh polygons**

Your polygon stores:

```cpp
float AreaCost = 1.0f;
```

and edge cost uses:

$$
d(C_A,C_B)
\cdot
\frac{Cost_A+Cost_B}{2}.
$$

This allows terrain preferences.

Examples:

```text
road       1.0
grass      1.5
mud        3.0
lava      100.0
```

assuming costs are multipliers above one.

A path may therefore choose a geometrically longer corridor if its traversability cost is lower.

But remember the heuristic issue from above if multipliers below one are allowed.

---

**Corridor versus actual geometric path**

A* on navmesh polygons returns:

```text
Poly 10
  ↓
Poly 20
  ↓
Poly 30
```

That does not mean the character should move through polygon centroids:

```text
start
 → center10
 → center20
 → center30
 → end
```

That often produces ugly zig-zags.

Instead, the polygon route defines a **corridor**.

The exact geometric shortest route through that corridor is found by string pulling.

---

**Funnel algorithm**

Imagine the sequence of shared portals:

```text
        L1        L2
         \        /
          \      /
start -----\----/-------
            \  /
             \/
             /\
            /  \
           /    \
         R1      R2
```

At any point we maintain an apex:

$$
A
$$

and two rays toward:

$$
L
$$

and:

$$
R.
$$

These form the current visibility funnel.

Each new portal provides:

$$
L_i,R_i.
$$

If a new left or right endpoint lies inside the funnel, tighten that side.

If one side crosses the opposite side, the opposite boundary becomes an unavoidable path corner.

Then:

1. emit that corner,
2. move apex there,
3. restart funnel construction from that portal.

This is the string-pulling algorithm.

The well-known "simple stupid funnel" implementation deliberately restarts from the new apex because the extra portal reprocessing is often worth the simpler, robust control flow. [digestingduck.blogspot.com](https://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html?utm_source=chatgpt.com)

---

**Signed area drives funnel orientation**

For XZ navigation, your code uses:

$$
Area_2(A,B,C)
=
(B_x-A_x)(C_z-A_z)
-
(B_z-A_z)(C_x-A_x).
$$

This is the 2D cross product.

Its sign tells us which side of ray $A\rightarrow B$ point C lies on.

That allows tests such as:

```text
Is new right endpoint tighter?
Has right crossed left?
Is new left endpoint tighter?
Has left crossed right?
```

Your code's `TriArea2XZ()` implements exactly this.

---

**Kairo's funnel setup**

The portal sequence begins with a degenerate portal:

$$
(start,start)
$$

and ends with:

$$
(end,end).
$$

Between them:

```cpp
PortalBetween(
    corridor[i],
    corridor[i + 1]
)
```

provides left/right boundaries.

So:

```text
(start,start)
portal 0
portal 1
portal 2
...
(end,end)
```

is passed to the funnel.

This is a very clean formulation because start and end then participate in the same algorithm as ordinary portals.

---

**Funnel narrowing**

Suppose current funnel is:

```text
        L
       /
      /
     A
      \
       \
        R
```

A new portal arrives.

If new right lies between current right and left, update:

$$
R\leftarrow R_{new}.
$$

Similarly for left.

The funnel becomes narrower as the corridor constrains visibility.

As long as the straight route from apex remains possible, no waypoint is needed.

This is why a perfectly straight corridor of many polygons can collapse to only:

```text
start
end
```

Your test suite verifies exactly this across three polygons.

---

**Funnel crossing**

Suppose new right crosses current left.

That means no line from the existing apex can remain inside the corridor while passing through the new portal.

The current left boundary is now a necessary corner.

Emit:

$$
waypoint=L.
$$

Set:

$$
A=L.
$$

Reset:

$$
L=A,
\qquad
R=A.
$$

Then reprocess portals from the new apex.

Symmetric logic applies when new left crosses right.

That is the core geometric proof underlying string pulling.

---

**Path cost versus funnel path length in current Kairo**

There are two different quantities in your `NavMeshPath`.

A* cost is based on:

$$
\text{centroid-to-centroid distance}
\times
\text{area costs}.
$$

Funnel outputs a geometric waypoint route through the selected corridor.

Therefore:

$$
result.Cost
$$

is **not necessarily the physical length of `Waypoints`**.

Nor is it necessarily the exact integrated terrain cost along the funnel path.

It is the graph-search corridor cost.

That is fine if documented.

But systems consuming `Cost` should understand its meaning.

---

**Funnel does not solve locomotion**

A funnel provides useful geometric corners.

It does not solve:

- acceleration,
- turning radius,
- character radius near corners,
- crowd avoidance,
- animation,
- velocity obstacles,
- moving obstacles,
- local steering.

A useful architecture is:

```text
A*
 ↓
polygon corridor
 ↓
funnel
 ↓
corner sequence
 ↓
steering
 ↓
locomotion / physics
```

Trying to make string pulling itself solve all movement behavior usually produces an overly complicated and brittle layer. This separation is also emphasized by the Recast/Detour author's practical funnel guidance: graph search selects a corridor, string pulling gives directions through it, and steering/locomotion handles final movement. [digestingduck.blogspot.com](https://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html?utm_source=chatgpt.com)

---

**Agent radius and portals**

The basic funnel algorithm treats the agent as a point.

For an agent with radius $r$, portal clearance must account for that radius.

A common solution is to build/erode the navmesh for a particular agent radius.

Then an agent center can safely be treated as a point moving over the already-clearance-adjusted mesh.

If instead using a generic mesh, portals need to be shrunk by radius and corners offset appropriately.

This becomes complicated around sharp turns.

That is why navigation meshes are usually generated from explicit agent configuration:

```text
radius
height
max slope
step height
```

rather than using raw render geometry directly.

---

**Dynamic obstacles**

A navmesh describes static walkable space well.

But worlds contain:

```text
closed doors
moving crates
vehicles
temporary barriers
fallen debris
crowds
```

There are several levels of dynamic response.

The first is **cost modification**.

Instead of deleting navigation topology, temporarily increase cost:

$$
c(e)\leftarrow c(e)+penalty.
$$

This is appropriate when passage remains possible but undesirable.

---

**Blocking edges or polygons**

A stronger policy marks some nodes/edges unavailable:

$$
walkable(n)=false.
$$

Then A* simply ignores them.

Example:

```text
Door open:
Room A ↔ Room B

Door closed:
Room A   Room B
```

This works well for discrete dynamic state changes.

Your current generic `NavigationGraph` does not expose an enabled/disabled edge flag, but such a flag would be straightforward.

---

**Navmesh carving**

For an obstacle occupying part of a nav polygon, merely disabling the entire polygon may be overly conservative.

You can carve the obstacle shape out of walkable space and retriangulate/repolygonize the local region.

This gives accurate navigation but modification is much more expensive.

Therefore dynamic navmesh systems often divide the world into **tiles**.

Only affected tiles are rebuilt.

Recast/Detour's tile-cache API follows exactly that model: obstacle changes mark affected tiles, and updates rebuild the touched tiles incrementally rather than regenerating the whole navigation mesh. [Recast Navigation](https://recastnav.com/classdtTileCache.html?utm_source=chatgpt.com)

---

**Static planning versus local avoidance**

Suppose another character walks across your path.

Rebuilding navmesh geometry for every other agent every frame would be absurd.

Instead:

```text
Navmesh/A*
    ↓
global route around walls
    ↓
local avoidance
    ↓
avoid nearby moving agents
```

This separates two timescales.

Global pathfinding handles persistent geometry.

Steering handles rapidly changing local interactions.

This is analogous to broadphase/narrowphase separation in collision detection.

---

**Current Kairo dynamic-navigation status**

Your `NavMesh` supports:

- adding/removing polygons,
- rebuilding adjacency,
- polygon costs,
- containment/nearest queries,
- corridor A*,
- funnel waypoints.

It does **not** currently expose a dedicated dynamic-obstacle system, tile cache, local carving API or edge-blocking layer.

So Part 1394 is not yet represented as a full runtime subsystem.

A practical next step would be a query-time navigation filter:

```cpp
struct NavQueryFilter
{
    bool IsPolygonEnabled(NavPolygonID id) const;
    float ExtraCost(NavPolygonID id) const;
};
```

That alone would support doors, temporary hazards and many game-state changes without rebuilding polygon topology.

---

**Hierarchical pathfinding**

A* search cost grows with the number of states it may need to expand.

For a huge grid:

$$
N=10^7
$$

cells.

Even a good heuristic may touch many nodes.

Hierarchical pathfinding compresses large regions into higher-level abstractions.

Suppose grid is divided into clusters:

```text
+-------+-------+-------+
|   A   |   B   |   C   |
|       |       |       |
+-------+-------+-------+
|   D   |   E   |   F   |
|       |       |       |
+-------+-------+-------+
```

We identify entrances between clusters.

Then construct abstract graph:

```text
A ↔ B ↔ C
↕   ↕   ↕
D ↔ E ↔ F
```

High-level A* searches cluster connectivity first.

Then refine only the relevant clusters.

---

**HPA* concept**

Hierarchical Path-Finding A* generally performs roughly:

```text
Low-level map
    ↓
partition into clusters
    ↓
find entrances
    ↓
precompute local entrance-to-entrance costs
    ↓
construct abstract graph
    ↓
search abstract graph
    ↓
refine each abstract segment locally
```

The great advantage is reducing search-space size.

Instead of exploring thousands of cells across an open region, high-level search crosses that region as one abstract transition.

The HPA* literature reports large reductions in search effort while accepting small path-quality loss in its approximate variants; the classic paper demonstrated multi-level abstractions and near-optimal paths for game maps. [webdocs.cs.ualberta.ca](https://webdocs.cs.ualberta.ca/~mmueller/ps/2004/hpastar.pdf?utm_source=chatgpt.com)

---

**Hierarchical navmeshes**

The same principle applies to navmeshes.

Individual polygons can be grouped into:

```text
region
sector
zone
island
```

Then search:

```text
world regions
     ↓
region portals
     ↓
polygon corridor inside chosen regions
     ↓
funnel
```

For an open-world game:

```text
continent
  ↓
district
  ↓
navmesh tile
  ↓
polygon
```

may all participate.

Hierarchy is not tied to grids.

It is a general search-space abstraction technique.

---

**Hierarchical pathfinding is an approximation problem**

If high-level abstraction stores only approximate local costs, the route can become suboptimal.

Suppose an abstract cluster appears cheap from entrance A to B, but the exact local geometry produces a detour.

If abstraction cost is inaccurate, high-level route selection may choose the wrong cluster sequence.

There is a spectrum:

$$
\text{exact precomputed abstraction}
$$

through:

$$
\text{near-optimal approximation}
$$

to:

$$
\text{very cheap coarse guidance}.
$$

Games often willingly sacrifice a tiny amount of path optimality for huge reductions in CPU cost.

An NPC taking a path 1% longer is usually invisible to the player.

A frame hitch from pathfinding is not.

---

**A* repository audit before leaving M083**

Your generic A* implementation has several strong properties.

It validates finite/nonnegative edge costs, handles start/goal absence cleanly, uses duplicate heap entries instead of requiring decrease-key, skips stale entries, protects against accumulated float overflow, records expansion count, reconstructs through `cameFrom`, and computes a Euclidean heuristic scale that remains safe when edge costs are cheaper than geometric distance.

There are also improvements available.

The generic priority queue has no deterministic secondary tie-breaker, so equal-$f$ routes may not have fully deterministic expansion ordering. Computing the global heuristic scale scans every edge on every path query, which can become expensive; because graph topology/costs usually change much less frequently than queries, the scale could be cached and invalidated when edges change. `unordered_map` storage is flexible but expensive for dense contiguous navigation IDs; vector-backed arrays can be much faster when node indices are compact.

The navmesh pathfinder has excellent deterministic tie-breaking by $F$, then $G$, then polygon ID, but as discussed, discounted `AreaCost < 1` can invalidate its unscaled Euclidean heuristic.

That is the important systems lesson:

$$
\boxed{
\text{algorithm correctness lives in data contracts as much as code}.
}
$$

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/29 - Dynamic spatial indices for moving worlds|Previous chapter]] · [[Chapter Edition/03 - Game Engine/31 - Spatial-index serialization and query evidence|Next chapter]]
