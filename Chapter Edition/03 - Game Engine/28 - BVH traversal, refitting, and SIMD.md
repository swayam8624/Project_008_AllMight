# 28 — BVH traversal, refitting, and SIMD

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/27 - Preparing and publishing runtime content|Previous chapter]] · [[Chapter Edition/03 - Game Engine/29 - Dynamic spatial indices for moving worlds|Next chapter]]

A hierarchy reduces query work only when its bounds remain valid after scene changes. This chapter follows traversal and refitting, then asks which parts can be widened or vectorized without weakening correctness.

**By the end:** Trace a BVH query, update bounds after motion, and describe the invariants required for wide nodes and SIMD traversal.

**Recall question:** Which bound should be tested next during traversal, and what update must occur when one primitive moves?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 65866–68706; source-span SHA256 1580276dba98f17fb98555784211497c698d4620ac47d3b3333db5b734885863. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### TRAVERSAL, REFITTING, WIDE BVHs AND SIMD

We now own tree construction.

The next question is:

> How does a query move through that tree while doing the minimum necessary work?

---

**Depth-first BVH traversal**

Consider:

```text
                  N0
               /      \
             N1        N2
            /  \      /  \
          L3   L4   L5   L6
```

Suppose the ray intersects N0.

Depth-first traversal follows one branch until reaching a leaf, then returns to unexplored branches.

Pseudocode:

```text
push(root)

while stack not empty:
    node = pop()

    if query misses node.bounds:
        continue

    if leaf:
        test primitives
    else:
        push children
```

That is the core BVH traversal algorithm.

The tree itself does not answer the final query.

It eliminates impossible work.

---

**Explicit traversal stacks**

Recursive traversal can be expressed naturally:

```cpp
void traverse(Node* node)
{
    if (!intersect(node->bounds))
        return;

    if (node->leaf)
        test(node->primitives);
    else {
        traverse(node->left);
        traverse(node->right);
    }
}
```

But high-performance implementations often use explicit stacks.

Why?

Function recursion consumes call-stack state and gives the implementation less explicit control over traversal order.

An explicit stack:

```cpp
std::array<uint32_t, 64> stack;
uint32_t top = 0;

stack[top++] = root;

while (top != 0) {
    const uint32_t nodeIndex =
        stack[--top];

    ...
}
```

allows you to decide which child is visited next.

It can also avoid dynamic allocation.

Your traversal currently uses:

```cpp
std::vector<SpatialIndex> stack;
stack.reserve(64);
stack.push_back(0);
```

This is convenient and safe for arbitrary depth, but `reserve(64)` may allocate heap storage for every query.

For a very hot ray-tracing path, a fixed local stack or small-buffer container can remove that per-query allocation.

But there is a tradeoff: fixed stacks need an overflow policy.

Never assume tree depth can never exceed the capacity unless construction enforces that invariant.

Your builder already has:

```cpp
MaxDepth = 64;
```

which could support a carefully designed fixed traversal stack, although wide traversal and sibling scheduling may affect the exact capacity calculation.

---

**Stackless BVH traversal**

Can traversal avoid a stack entirely?

Yes, by storing enough structural information to know where to continue after finishing a subtree.

One option is a **parent pointer**, which your node already stores.

Another common concept is an **escape index** or **rope**.

For depth-first linearized nodes:

```text
Node:
    bounds
    firstChild
    escape
```

If the node intersects, descend.

If it misses, jump directly to `escape`.

Conceptually:

```text
N0
|
+-- N1
|   |
|   +-- L2
|   +-- L3
|
+-- N4
```

The escape link for N1 points to N4.

A stackless traversal might be:

```cpp
index = root;

while (index != INVALID)
{
    const Node& node = nodes[index];

    if (!intersects(node.bounds))
    {
        index = node.escape;
        continue;
    }

    if (node.isLeaf())
    {
        testLeaf(node);
        index = node.escape;
    }
    else
    {
        index = node.firstChild;
    }
}
```

The advantage is reduced per-ray stack storage.

The disadvantage is more complex node layout and potentially less flexible ordering.

For closest-hit rays, near-first ordering becomes harder if the linear layout disagrees with ray direction.

---

**Near-child-first ray traversal**

Suppose a ray intersects both child boxes.

Their first intersection parameters are:

$$
t_L=3
$$

and

$$
t_R=20.
$$

Which child should we traverse first?

For closest-hit queries, the near child.

Why?

Suppose the left subtree contains an actual primitive hit at

$$
t=4.
$$

Then current closest becomes

$$
t_{\text{closest}}=4.
$$

The right child begins at $20$, therefore:

$$
20>4.
$$

The entire right subtree can now be discarded.

If we traversed the far child first, we might perform substantial work before discovering the near hit.

Therefore near-first ordering is not required for correctness, but can massively improve pruning.

Your current `Raycast()` calculates:

```cpp
leftDistance
rightDistance
```

and when both children intersect:

```cpp
if (leftDistance < rightDistance)
{
    stack.push_back(node.Right);
    stack.push_back(node.Left);
}
```

Because the stack is LIFO, the left child is processed next.

So your closest-hit traversal already implements near-child-first ordering.

---

**Closest-hit pruning by current ray t**

Suppose current nearest primitive hit is

$$
t_{\text{best}}.
$$

For any node whose bounding box cannot be entered before $t_{\text{best}}$, no primitive in that node can improve the solution.

Thus the node's ray-box test uses:

$$
t_{\max}=t_{\text{best}}.
$$

Every time we discover a closer primitive:

$$
t_{\text{best}}
\leftarrow
t_{\text{hit}}.
$$

The search domain shrinks dynamically.

This is branch-and-bound reasoning.

The BVH tree gives spatial lower bounds.

The current closest hit gives an upper bound.

When:

$$
t_{\text{nodeNear}}>t_{\text{best}},
$$

the subtree cannot contain the optimum.

This same pattern appears far beyond ray tracing:

$$
\boxed{
\text{Lower bound of unexplored region}
>
\text{best known solution}
\Rightarrow
\text{prune}
}
$$

Your implementation correctly passes `closestDistance` into its node AABB tests and updates it whenever a closer primitive hit appears.

---

**A subtle inefficiency in your current traversal**

Your implementation performs child bounds tests here:

```cpp
RayIntersectsAABB(
    ray,
    bvh.Nodes[node.Left].Bounds,
    closestDistance,
    leftDistance);
```

and similarly for right.

It then pushes the intersected child.

When that child is popped on the next loop iteration, the code again performs:

```cpp
RayIntersectsAABB(
    ray,
    node.Bounds,
    closestDistance,
    boundsDistance);
```

Therefore many node AABBs are tested twice.

Why might this have been written this way?

Because child pretesting is required to determine near versus far ordering.

A common optimization is to store the already calculated entry distance on the stack:

```cpp
struct StackEntry
{
    uint32_t node;
    float nearT;
};
```

Then the popped node need not repeat the ray-box test unless the current closest hit has changed enough that a cheap `nearT > closestDistance` check can reject it.

Conceptually:

```cpp
if (entry.nearT > closestDistance)
    continue;
```

This converts a repeated six-slab test into one scalar comparison.

Whether this materially improves performance must be benchmarked.

---

**Any-hit traversal**

Any-hit asks:

$$
\exists p:
\operatorname{Intersect}(Q,p)?
$$

The moment one acceptable hit is found:

$$
\boxed{\text{return true}}
$$

No closest distance must be maintained.

This makes it ideal for binary visibility queries.

Shadow ray:

```text
Light ------------------------- Surface point
             ^
             |
      Is anything here?
```

If one opaque occluder exists, traversal ends.

Your `RaycastAny()` does exactly this.

It traverses nodes until `primitiveHit()` returns true, then returns immediately.

It does **not** perform near-child sorting.

That does not affect correctness.

However, ordering can still affect performance: a child likely to contain a hit should ideally be visited first.

For incoherent shadow rays, deciding that order may or may not be worth the additional box-intersection work.

This is a workload-dependent tradeoff.

---

**One current API subtlety: `Raycast().Hits`**

Your closest-hit traversal performs:

```cpp
if (hit &&
    hit->Distance <= closestDistance)
{
    result.Hits.push_back(*hit);
    result.Closest = *hit;
    closestDistance = hit->Distance;
}
```

That means `result.Hits` is **not an all-hit list**.

Suppose traversal encounters distances:

$$
12,\quad 7,\quad 9,\quad 3.
$$

The stored sequence becomes:

$$
[12,7,3].
$$

The hit at 9 is valid geometrically but is discarded because 7 was already closer.

Therefore `Hits` currently means something like:

> sequence of progressively improving closest candidates encountered during traversal.

That is a perfectly usable internal diagnostic concept, but the name `Hits` can be mistaken for an all-hit query.

If external users interpret it as every ray hit, the API would be misleading.

A clearer design could expose only `Closest`, or rename the vector to something like `ImprovingHits`/`TraversalHits`, while implementing a distinct `RaycastAll()` traversal if actual all-hit semantics are needed.

---

**BVH overlap traversal**

Ray traversal is only one BVH query.

Consider AABB query $Q$.

At node $N$:

$$
Q\cap B_N=\varnothing
\Rightarrow
\text{skip entire subtree}.
$$

If it overlaps, continue.

At leaves, test the actual primitive bounds:

$$
Q\cap B_{P_i}\ne\varnothing.
$$

Canonical traversal:

```cpp
stack.push(root);

while (!stack.empty())
{
    Node node = pop();

    if (!overlap(query, node.bounds))
        continue;

    if (node.isLeaf())
    {
        for (primitive : node)
            if (overlap(query, primitive.bounds))
                output.push_back(primitive);
    }
    else
    {
        stack.push(node.left);
        stack.push(node.right);
    }
}
```

Your `QueryAABB()` implements exactly this structure and supports layer-mask filtering.

This is acceleration, not necessarily final narrowphase.

If each `SpatialPrimitive.Bounds` represents a broadphase collider box, a returned ID means:

$$
\text{"candidate whose bound overlaps query"}
$$

not necessarily:

$$
\text{"exact underlying shape overlaps query"}.
$$

That distinction prevents broadphase results from being mistaken for exact collision results.

---

**BVH point query**

A point can be treated as a zero-size AABB:

$$
B_P=[P,P].
$$

Your `QueryPoint()` literally delegates:

```cpp
return QueryAABB(
    bvh,
    SpatialAABB::FromPoint(point),
    layerMask);
```

That is excellent API reuse because containment:

$$
P\in B
$$

is equivalent to:

$$
[P,P]\cap B\ne\varnothing.
$$

Again, the result identifies primitive **bounds** containing the point.

It does not prove that the point lies inside an arbitrary triangle mesh or convex hull.

That would require the appropriate narrowphase/containment test.

---

**BVH frustum query**

For each node:

$$
F\cap B_N=\varnothing
\Rightarrow
\text{prune}.
$$

If the bound survives frustum rejection, descend.

At leaves, classify primitive bounds.

Your `QueryFrustum()` uses:

```cpp
frustum.IntersectsAABB(node.Bounds)
```

for nodes and again for leaf primitive bounds.

This is particularly effective for rendering because the hierarchy can reject entire groups of objects with six-plane tests against one node bound.

If 10,000 nearby renderables occupy one region outside the camera, a high-level BVH node may eliminate all 10,000.

The query structure becomes:

```text
Camera frustum
       |
       v
BVH root
       |
       +--- outside? ---> reject enormous subtree
       |
       v
surviving children
       |
       v
leaf bounds
       |
       v
visible candidate IDs
```

The output should normally remain conservative for rendering: extra visible candidates are preferable to incorrectly disappearing geometry.

---

**BVH refitting**

Suppose the topology remains valid but primitives move.

Do we have to rebuild the tree?

Not necessarily.

**Refitting** preserves the tree topology and updates bounds.

For a leaf:

$$
B_L
=
\bigcup_{p\in L}
B(p).
$$

For an internal node:

$$
B_N=
B_{\text{left}(N)}
\cup
B_{\text{right}(N)}.
$$

Therefore process bottom-up.

```cpp
void refitBVH(
    BVH& bvh,
    std::span<const AABB> updatedPrimitiveBounds)
{
    // 1. Update leaf bounds.
    for (std::size_t nodeIndex = 0;
         nodeIndex < bvh.nodes.size();
         ++nodeIndex)
    {
        Node& node = bvh.nodes[nodeIndex];

        if (!node.isLeaf())
            continue;

        AABB bounds = AABB::Empty();

        for (std::uint32_t i = 0;
             i < node.primitiveCount;
             ++i)
        {
            const auto primitive =
                bvh.primitiveOrder[
                    node.firstPrimitive + i];

            bounds.expand(
                updatedPrimitiveBounds[primitive]);
        }

        node.bounds = bounds;
    }

    // 2. Update internal nodes bottom-up.
    //
    // This assumes construction order guarantees children
    // appear after parents, as in a pre-order flat layout.
    for (std::size_t i = bvh.nodes.size();
         i-- > 0;)
    {
        Node& node = bvh.nodes[i];

        if (node.isLeaf())
            continue;

        node.bounds =
            Union(
                bvh.nodes[node.left].bounds,
                bvh.nodes[node.right].bounds);
    }
}
```

This is usually much cheaper than rebuilding because no split decisions or sorting are performed.

If each node is visited once:

$$
T_{\text{refit}}=O(N_{\text{nodes}}).
$$

For a binary BVH with $L$ leaves, internal node count is on the order of $L-1$, so refitting is linear.

---

**When reverse node order is valid**

The previous code assumes a construction layout in which descendants have larger indices than ancestors.

Your recursive builder does exactly that:

1. append parent,
2. recurse left,
3. recurse right.

Thus:

$$
index(parent)<index(child).
$$

Consequently walking the flat node array backwards visits children before parents.

That makes reverse-order refitting possible without recursion.

This is a beautiful example of how construction order creates an algorithmic property later.

The flat layout is not merely storage.

It can encode traversal/refit relationships.

---

**Selective refitting**

Suppose only one primitive moves.

Refitting the entire tree is unnecessary.

If leaves know their parents, update the affected leaf, then walk upward:

```text
changed primitive
      |
      v
leaf bound
      |
      v
parent
      |
      v
grandparent
      |
      v
root
```

At each ancestor:

$$
B_N=
B_L\cup B_R.
$$

Your `BVHNode` already stores `Parent`.

Therefore the representation already contains one of the pieces needed for incremental refitting.

What is missing is a reverse mapping from primitive to containing leaf.

You currently know:

```text
leaf -> primitive range
```

but efficient selective updates also benefit from:

```text
primitive -> leaf
```

Otherwise locating the affected leaf may itself require searching the tree or scanning leaves.

---

**Rebuilding**

Refit changes bounds.

Rebuild changes topology.

A rebuild re-runs partition decisions:

$$
\text{primitives}
\rightarrow
\text{new bounds}
\rightarrow
\text{new splits}
\rightarrow
\text{new tree}.
$$

Why ever pay this cost?

Because topology can become poor after large motion.

Imagine the tree initially separates objects spatially:

```text
LEFT subtree              RIGHT subtree

AAAA AAAAA                BBBB BBBB
```

After deformation:

```text
LEFT objects:   A A A A scattered everywhere
RIGHT objects:  B B B B scattered everywhere
```

Refitting creates enormous overlapping child bounds:

```text
+---------------------------------------+
| LEFT BOUND                            |
|     +---------------------------------+----+
|     | RIGHT BOUND                          |
|     |                                      |
+-----|--------------------------------------+
      +--------------------------------------+
```

The tree remains mathematically valid.

But nearly every query enters both branches.

Acceleration collapses.

---

**Quality degradation under deformation**

This distinction is essential:

$$
\boxed{
\text{BVH correctness}
\ne
\text{BVH quality}
}
$$

A refitted tree can be 100% correct and almost useless.

Useful quality measures include expected SAH cost, sibling overlap, child-to-parent area ratio, traversal node counts, or actual measured query cost.

For an internal node, a simple overlap-volume metric could be:

$$
Q_{\text{overlap}}
=
\frac{
V(B_L\cap B_R)
}{
V(B_P)
}.
$$

But volume overlap does not necessarily predict ray traversal perfectly.

SAH-inspired metric:

$$
Q_N
=
\frac{A_L}{A_P}C_L
+
\frac{A_R}{A_P}C_R.
$$

An even better production metric may be direct telemetry:

$$
\text{average visited nodes/query}
$$

and

$$
\text{primitive tests/query}.
$$

Your traversal already returns:

```cpp
VisitedNodes
TestedPrimitives
```

which gives you exactly the instrumentation required to detect quality degradation empirically.

A practical rebuild policy could look like:

$$
\frac{
\text{current avg visited nodes}
}{
\text{baseline avg visited nodes}
}
>\tau
\quad\Rightarrow\quad
\text{schedule rebuild}.
$$

This is not automatically optimal, but it gives you a measurable control signal instead of guessing.

Dynamic-scene BVH literature explicitly observes that refitting can preserve correctness while traversal performance deteriorates enough to justify rebuilding. [ResearchGate](https://www.researchgate.net/publication/234777055_Special_Section_Parallel_Graphics_and_Visualization_Fast_parallel_and_asynchronous_construction_of_BVHs_for_ray_tracing_animated_scenes?utm_source=chatgpt.com)

---

**Your repository currently has no static-BVH refit/rebuild API**

I searched the current `KairoSpatial` implementation for static-BVH refit, rebuild, quantized-node, wide-node and packet-traversal implementations.

They are not currently exposed as implemented BVH mechanisms.

That means Parts 1330–1336 are primarily **canonical theory and implementation targets**, not descriptions of already-existing Kairo behavior.

This is important to say explicitly rather than pretending the repository already contains all curriculum topics.

You do already possess several prerequisites for refitting:

```text
BVHNode.Parent
BVHNode.Bounds
BVHNode.Left / Right
leaf primitive ranges
flat recursive node order
```

So adding static refit would fit naturally into the current representation.

---

**Wide BVHs**

So far every internal node has two children.

That is BVH2.

A wide BVH might use four children:

$$
BVH4
$$

or eight:

$$
BVH8.
$$

Conceptually:

```text
Binary:

                 Node
                /    \
               A      B


BVH4:

              Node
          /    |    |    \
         A     B    C     D
```

Why make nodes wider?

Tree depth decreases.

A perfectly balanced binary tree with $N$ leaves has approximate depth:

$$
D_2\approx\log_2N.
$$

For branching factor $k$:

$$
D_k\approx\log_kN.
$$

With one million leaves:

$$
\log_2(10^6)\approx20
$$

while

$$
\log_8(10^6)\approx6.64.
$$

So a BVH8 can reduce traversal levels dramatically.

But every node now needs to test more children.

This becomes attractive when SIMD can test several child boxes in parallel.

---

**Binary traversal versus wide traversal**

For BVH2:

```text
test left box
test right box
choose order
```

For BVH8:

```text
test 8 child boxes simultaneously
        |
        v
8-bit hit mask
        |
        v
sort/interleave surviving children
        |
        v
continue
```

A wide node representation might use structure-of-arrays:

```cpp
struct BVH8Node
{
    float minX[8];
    float minY[8];
    float minZ[8];

    float maxX[8];
    float maxY[8];
    float maxZ[8];

    uint32_t child[8];
};
```

That layout is deliberately SIMD-friendly.

One vector load obtains multiple `minX` values.

This connects directly back to the SoA material from M021/M078.

Wide BVHs are therefore not merely "trees with more children."

They are a co-design between:

$$
\boxed{
\text{tree topology}
+
\text{node layout}
+
\text{SIMD width}
+
\text{traversal algorithm}.
}
$$

Embree is an excellent production example of this architecture-aware design, including BVH4 variants and multiple primitive layouts selected for different memory/performance characteristics. [embree.github.io](https://embree.github.io/papers/2014-Siggraph-Embree.pdf?utm_source=chatgpt.com)

---

**Wide-node construction**

How do we obtain a BVH4 or BVH8?

One option is to build a binary BVH first and collapse subtrees.

Suppose:

```text
              P
            /   \
           A     B
          / \   / \
         C  D  E  F
```

Collapse one level:

```text
              P
        /     |     |     \
       C      D     E      F
```

Now P has four children.

The collapse decision should consider traversal quality and node width.

Blindly flattening everything may worsen bounding overlap.

More advanced builders construct wide BVHs directly or use SAH-based collapsing.

The important invariant remains:

$$
B_P
\supseteq
\bigcup_iB_{C_i}.
$$

---

**Quantized BVH bounds**

Bounds consume substantial memory.

For a BVH8 storing full 32-bit floating point bounds:

$$
8\text{ children}
\times
6\text{ coordinates}
\times
4\text{ bytes}
=
192\text{ bytes}
$$

just for child AABBs.

That is several cache lines before child metadata.

Quantization reduces this.

Suppose parent bounds are:

$$
B_P=[P_{\min},P_{\max}].
$$

A child min coordinate $x_{\min}$ can be represented relative to parent extent:

$$
u=
\frac{x_{\min}-P_{\min,x}}
{P_{\max,x}-P_{\min,x}}.
$$

With $b$ quantization bits:

$$
q=
\operatorname{round}
\left(
u(2^b-1)
\right).
$$

Decoding gives:

$$
\hat x=
P_{\min,x}
+
\frac{q}{2^b-1}
(P_{\max,x}-P_{\min,x}).
$$

---

**Conservative quantization**

Ordinary nearest-value rounding can shrink a bound.

That is dangerous.

A quantized BVH must preserve:

$$
B_{\text{true}}\subseteq B_{\text{decoded}}.
$$

Therefore minima should round outward downward and maxima outward upward.

For $Q=2^b-1$:

$$
q_{\min}
=
\left\lfloor
Q
\frac{x_{\min}-P_{\min}}
{P_{\max}-P_{\min}}
\right\rfloor
$$

and

$$
q_{\max}
=
\left\lceil
Q
\frac{x_{\max}-P_{\min}}
{P_{\max}-P_{\min}}
\right\rceil.
$$

Then decode both.

This may enlarge the child slightly.

That introduces extra false-positive traversal but preserves correctness.

This is the same conservative principle from M078, now applied to compressed spatial representation.

---

**Quantization error**

With parent extent $E$ and $b$ bits, one quantization step is approximately:

$$
\Delta=
\frac{E}{2^b-1}.
$$

With eight bits:

$$
\Delta\approx \frac{E}{255}.
$$

If parent X extent is 100 m:

$$
\Delta_x\approx0.392\text{ m}.
$$

That can significantly inflate small child boxes.

With 16 bits:

$$
\Delta_x\approx
\frac{100}{65535}
\approx1.53\text{ mm}.
$$

Therefore quantization quality is relative to parent extent.

Hierarchical quantization helps because deeper nodes usually cover progressively smaller spatial regions.

Compressed wide BVHs can reduce working-set size and improve cache behavior, which is one reason compressed wide layouts are actively studied for high-performance traversal. [Intel](https://www.intel.com/content/dam/develop/external/us/en/documents/wide-bvh-traversal-with-a-short-stack-837099.pdf?utm_source=chatgpt.com)

---

**BVH node memory layout**

Now we reach a topic people often underestimate.

Two BVHs can have identical topology and identical bounds but perform differently because their nodes occupy memory differently.

Consider an AoS binary node:

```cpp
struct Node
{
    AABB bounds;
    uint32_t left;
    uint32_t right;
    uint32_t first;
    uint32_t count;
};
```

Traversal reads:

```text
bounds
child metadata
possibly primitive metadata
```

For internal nodes, leaf fields may be unused.

For leaves, child fields may be unused.

That means every node carries mutually exclusive information.

Alternative packed representations encode node type inside metadata:

```cpp
struct PackedNode
{
    AABB bounds;
    uint32_t offset;
    uint32_t meta;
};
```

For an internal node:

```text
offset -> first child
meta   -> internal flag
```

For a leaf:

```text
offset -> primitive offset
meta   -> leaf flag + primitive count
```

This reduces memory footprint.

But compression may add decoding operations.

Again:

$$
\text{smaller}
\not\Rightarrow
\text{always faster}.
$$

You trade memory bandwidth against decode complexity and implementation flexibility.

---

**Cache-line reasoning**

Suppose a node fits in one 64-byte cache line.

Loading the node may require one cache-line transfer.

If it is 80 bytes and straddles two cache lines, potentially two lines must be fetched.

But the exact cost depends on alignment, cache state, hardware prefetching and neighboring accesses.

Therefore never optimize node layout purely from `sizeof`.

Measure:

$$
\text{cache misses},
\quad
\text{bytes/ray},
\quad
\text{nodes/ray},
\quad
\text{cycles/node}.
$$

Your current node contains:

```text
AABB
Left
Right
FirstPrimitive
PrimitiveCount
Parent
```

To know its actual ABI size, alignment and padding, measure:

```cpp
std::cout << sizeof(BVHNode)
          << ' '
          << alignof(BVHNode);
```

Do not infer it solely from source fields because the underlying `Vec3f/AABBf` alignment rules matter.

---

**Primitive ordering matters too**

Your hierarchy separates:

```cpp
Primitives
PrimitiveOrder
```

A leaf owns a range in `PrimitiveOrder`.

Then:

```cpp
PrimitiveAtOrderedOffset(offset)
```

performs:

$$
i=\text{PrimitiveOrder[offset]}
$$

then:

$$
\text{Primitives}[i].
$$

That adds an indirection.

Why keep it?

Because construction can reorder primitive **references** without moving the original primitive records.

This makes building convenient and preserves stable original indexing.

An alternative is to physically reorder primitive records into leaf order after construction.

That can improve leaf traversal locality but changes indices and may require remapping external identifiers.

Again, there is no universally free choice.

Embree's published architecture explicitly discusses the memory/performance trade-off between leaves that store references to primitives and layouts that place primitive data contiguously for faster traversal. [embree.github.io](https://embree.github.io/papers/2014-Siggraph-Embree.pdf?utm_source=chatgpt.com)

---

**SIMD packet BVH traversal**

There are two fundamentally different SIMD strategies that beginners often conflate.

**Strategy A: one ray versus many child boxes**

A wide BVH node contains four or eight child AABBs.

SIMD lanes represent child nodes.

```text
one ray

lane 0 -> child 0
lane 1 -> child 1
lane 2 -> child 2
lane 3 -> child 3
...
```

This is node SIMD.

**Strategy B: many rays versus one box**

A packet contains rays:

$$
R_0,R_1,\ldots,R_7.
$$

Each SIMD lane represents one ray.

```text
one BVH node

lane 0 -> ray 0
lane 1 -> ray 1
lane 2 -> ray 2
...
```

This is ray-packet traversal.

They solve different vectorization problems.

---

**Why ray packets work**

Primary camera rays are often coherent.

Neighboring pixels have nearby origins and similar directions.

Thus a packet such as:

```text
R0 R1 R2 R3
R4 R5 R6 R7
```

often traverses the same BVH regions.

Instead of running eight independent traversals, we can process them together.

For each node calculate a lane mask:

```text
ray 0 intersects node? 1
ray 1 intersects node? 1
ray 2 intersects node? 0
ray 3 intersects node? 1
...

mask = 1101...
```

If mask is zero, reject the node for the entire packet.

Otherwise continue with only active lanes.

---

**Packet divergence**

Packet traversal performs poorly when rays become incoherent.

Imagine reflected rays bouncing in different directions.

```text
R0 -> north
R1 -> south
R2 -> west
R3 -> up
R4 -> east
...
```

Now every ray may require a different subtree.

SIMD lanes diverge.

If one lane needs child A and another needs child B, the packet may eventually visit both.

The work approaches the union of all individual traversal paths.

This can eliminate packet efficiency.

Therefore:

$$
\boxed{
\text{Packet traversal benefits from ray coherence.}
}
$$

Primary rays: often coherent.

Diffuse secondary rays: frequently less coherent.

Shadow rays: depends heavily on light geometry and scene layout.

---

**Packet traversal state**

Suppose packet width is $W$.

Maintain:

$$
M_N\in\{0,1\}^W
$$

for every stack entry, where bit/lane $i$ indicates whether ray $i$ remains active for that node.

Pseudo-architecture:

```cpp
struct PacketStackEntry
{
    uint32_t node;
    LaneMask activeRays;
};

push({root, allRays});

while (!stack.empty())
{
    auto [nodeIndex, active] = pop();

    LaneMask intersects =
        intersectAABBPacket(
            rays,
            active,
            nodes[nodeIndex].bounds);

    if (intersects.none())
        continue;

    if (nodes[nodeIndex].isLeaf())
    {
        intersectPrimitivePacket(
            rays,
            intersects,
            leaf);
    }
    else
    {
        push({
            nodes[nodeIndex].left,
            intersects
        });

        push({
            nodes[nodeIndex].right,
            intersects
        });
    }
}
```

The actual high-performance version becomes substantially more complicated because each lane has its own closest distance:

$$
t_{\max,0},
t_{\max,1},
\dots,
t_{\max,W-1}.
$$

Node intersection therefore uses a vector of ray-specific bounds.

---

**Single-ray SIMD versus packet SIMD**

Modern CPU ray tracers frequently use SIMD even for individual rays by testing multiple child boxes or primitives at once.

This can be superior to packets for incoherent workloads.

The distinction is:

```text
Packet traversal:
SIMD axis = rays

Wide-node traversal:
SIMD axis = child bounds

Primitive SIMD:
SIMD axis = primitives
```

A sophisticated renderer may choose among all three depending on ray coherence.

This is one of the major lessons from production ray-tracing systems such as Embree: SIMD width, node arity, triangle layout and ray coherence have to be designed together rather than treated as independent optimizations. [embree.github.io](https://embree.github.io/papers/2014-Siggraph-Embree.pdf?utm_source=chatgpt.com)

---

**A complete closest-hit traversal implementation**

Let us now write the canonical mechanism with the optimization we identified earlier: store the already-computed near distance on the stack.

```cpp
struct TraversalEntry
{
    std::uint32_t nodeIndex;
    float nearT;
};

template <class PrimitiveIntersector>
std::optional<Hit> closestHitBVH(
    const BVH& bvh,
    const Ray& ray,
    PrimitiveIntersector&& intersectPrimitive,
    float maxDistance)
{
    if (bvh.nodes.empty())
        return std::nullopt;

    std::array<TraversalEntry, 128> stack{};
    std::size_t stackSize = 0;

    float rootNear = 0.0f;

    if (!intersectAABB(
            ray,
            bvh.nodes[0].bounds,
            maxDistance,
            rootNear))
    {
        return std::nullopt;
    }

    stack[stackSize++] =
        {0u, rootNear};

    float closest =
        maxDistance;

    std::optional<Hit> best;

    while (stackSize != 0)
    {
        const TraversalEntry current =
            stack[--stackSize];

        // The box was already tested when inserted.
        // Since closest may have decreased meanwhile,
        // one scalar test can now prune the node.
        if (current.nearT > closest)
            continue;

        const BVHNode& node =
            bvh.nodes[current.nodeIndex];

        if (node.isLeaf())
        {
            for (std::uint32_t i = 0;
                 i < node.primitiveCount;
                 ++i)
            {
                const std::uint32_t primitive =
                    bvh.primitiveOrder[
                        node.firstPrimitive + i];

                auto hit =
                    intersectPrimitive(
                        primitive,
                        ray,
                        closest);

                if (!hit)
                    continue;

                if (hit->distance < 0.0f ||
                    hit->distance > closest)
                    continue;

                closest = hit->distance;
                best = *hit;
            }

            continue;
        }

        float leftNear = 0.0f;
        float rightNear = 0.0f;

        const bool hitLeft =
            intersectAABB(
                ray,
                bvh.nodes[node.left].bounds,
                closest,
                leftNear);

        const bool hitRight =
            intersectAABB(
                ray,
                bvh.nodes[node.right].bounds,
                closest,
                rightNear);

        if (hitLeft && hitRight)
        {
            // Push far first because stack is LIFO.
            if (leftNear <= rightNear)
            {
                stack[stackSize++] =
                    {node.right, rightNear};

                stack[stackSize++] =
                    {node.left, leftNear};
            }
            else
            {
                stack[stackSize++] =
                    {node.left, leftNear};

                stack[stackSize++] =
                    {node.right, rightNear};
            }
        }
        else if (hitLeft)
        {
            stack[stackSize++] =
                {node.left, leftNear};
        }
        else if (hitRight)
        {
            stack[stackSize++] =
                {node.right, rightNear};
        }
    }

    return best;
}
```

The complete causal chain is now visible:

$$
\text{node bounds}
\rightarrow
t_{\text{near}}
\rightarrow
\text{near-first ordering}
\rightarrow
\text{early primitive hit}
\rightarrow
t_{\text{closest}}\downarrow
\rightarrow
\text{more subtree pruning}.
$$

That is why construction quality and traversal algorithm are inseparable.

A good tree gives traversal more opportunities to prune.

A good traversal exploits those opportunities quickly.

---

**Manual traversal trace**

Consider:

```text
                         N0
                  [0 ---------------- 20]
                    /               \
                   /                 \
                 N1                   N2
              [1---8]              [9----20]
             /      \              /       \
           P0       P1           P2         P3
          t=7      t=3          t=11       miss
```

Initial:

$$
t_{\text{closest}}=\infty.
$$

Root N0 intersects.

Child entry distances:

$$
t_{N1}=1,
\qquad
t_{N2}=9.
$$

Visit N1 first.

Suppose N1's children yield primitive P1 at:

$$
t=3.
$$

Now:

$$
t_{\text{closest}}=3.
$$

Return to pending N2.

Its recorded entry distance is:

$$
9.
$$

Since

$$
9>3,
$$

N2 is pruned without touching P2 or P3.

The BVH saved two primitive tests and further node traversal.

Scale this to millions of primitives and this mechanism becomes the difference between interactive and unusable rendering.

---

**Build quality affects traversal exponentially through overlap**

Suppose siblings barely overlap:

```text
+--------+      +--------+
| LEFT   |      | RIGHT  |
+--------+      +--------+
```

Most rays enter one or the other.

Now suppose:

```text
+-------------------+
| LEFT              |
|    +-------------------+
|    | RIGHT        |    |
+----|--------------+    |
     +-------------------+
```

Many rays enter both.

Every time both children survive, traversal branches.

Repeated overlap throughout several levels can produce a combinatorial increase in visited nodes.

This is why balancing primitive counts alone is not enough.

A slightly unbalanced SAH tree can outperform a perfectly balanced median tree because it reduces simultaneous child intersections.

---

**Build-time complexity comparison**

Let $N$ be primitive count.

**Median builder with sorting at every node**

At a node of size $m$:

$$
O(m\log m)
$$

for sorting.

Across a balanced hierarchy:

$$
O(N\log^2N)
$$

is a reasonable upper-level characterization of repeated full sorting.

**Binned SAH**

Each node can bin and partition in roughly:

$$
O(m+B)
$$

for fixed bin count $B$.

Across balanced levels:

$$
O(N\log N)
$$

approximately.

**Morton / LBVH-style builder**

Morton-code calculation:

$$
O(N).
$$

Sorting:

$$
O(N\log N)
$$

with comparison sort, or potentially closer to linear passes using radix sort.

Topology generation from sorted keys can be:

$$
O(N).
$$

This is why LBVH is attractive for rapidly changing scenes, particularly on GPUs.

The tradeoff is tree quality: raw Morton-based trees can have poorer traversal efficiency than high-quality SAH trees.

Hybrid techniques therefore often use fast Morton construction plus local/treelet optimization.

---

**Your builder's current complexity profile**

Your current repository has three distinct paths.

**Median path**

Each recursive node:

```cpp
stable_sort(range by selected axis)
```

then splits midpoint.

Build quality is simple and predictable.

Build cost can be substantial because ranges are repeatedly sorted.

**SAH path**

For every node:

```text
sort X
prefix/suffix sweep

sort Y
prefix/suffix sweep

sort Z
prefix/suffix sweep
```

plus temporary `left/right` vectors.

This gives relatively high-quality split candidates but significant build overhead.

**Morton path**

One global Morton sort occurs before recursion.

Then recursive ranges are split at their midpoint.

No per-node sorting happens.

This makes it substantially closer to a fast build path, although the current `stable_sort` comparator recomputes normalized centroids and Morton codes during comparisons instead of precomputing one key per primitive.

A more efficient design would store:

```cpp
struct MortonRef
{
    uint32_t code;
    uint32_t primitive;
};
```

compute each code once, then sort those records.

For large builds, that avoids repeated Morton encoding inside the comparison function.

---

**Repository tests and what they actually prove**

Your `SpatialTests.cpp` checks several important invariants.

The suite verifies that root bounds equal the brute union of all primitive bounds, empty/single-element trees behave correctly, leaf primitive counts obey `MaxLeafSize`, Morton build mode produces a valid hierarchy, closest raycasts return the expected primitive, any-hit exits before visiting every primitive in the sample, and AABB/sphere/frustum queries agree with direct expected results.

`ScaleStressTests.cpp` raises the size.

It constructs a:

$$
64\times64=4096
$$

primitive grid, builds with Morton ordering and leaf size eight, then checks 128 AABB queries against brute force.

This is valuable correctness evidence.

What it does **not** yet establish is comparative build quality.

The test does not currently ask questions such as:

$$
\text{median nodes/query}
$$

versus

$$
\text{SAH nodes/query}
$$

versus

$$
\text{Morton nodes/query}.
$$

Nor does it benchmark:

$$
\text{build ms}
$$

against:

$$
\text{query ms}.
$$

Those are the exact experiments M079/M080 naturally motivate.

---

**A proper BVH benchmark matrix**

For each build strategy, record:

| Metric | What it tells you |
|---|---|
| Build time | Construction overhead |
| Node count | Hierarchy storage |
| Leaf count | Terminal grouping |
| Maximum/average depth | Structural shape |
| Bytes/tree | Memory cost |
| Average nodes/ray | Traversal quality |
| p95 nodes/ray | Bad-case traversal |
| Average primitive tests/ray | Pruning effectiveness |
| Rays/sec | End-to-end throughput |
| AABB queries/sec | Non-ray workload quality |
| Refit time | Dynamic update cost |
| Post-deformation traversal cost | Quality degradation |
| Rebuild threshold benefit | Whether rebuilding pays |

Do this on different distributions.

Uniform grid.

Clustered scene.

One giant primitive plus many tiny primitives.

Long corridor.

Random clouds.

Highly overlapping primitives.

Deforming mesh.

This is important because there is no universally "best BVH builder."

There is a best builder **for a workload and cost budget**.

---

**CPU cache behavior**

Traversal repeatedly performs:

```text
load node
test bounds
decide children
load next node
...
```

A highly coherent traversal may reuse upper nodes heavily.

Root and top-level nodes can remain hot in cache.

Lower nodes are visited more sparsely.

This creates an asymmetry:

```text
Top BVH levels:
high reuse
small count
likely cache-resident

Bottom BVH levels:
huge count
lower reuse
memory-bandwidth sensitive
```

Wide-node compression is especially useful deeper in the hierarchy because reducing the node working set increases the chance relevant child data arrives in fewer cache lines.

But wide nodes also increase each individual node's raw information.

Hence compression and quantization become natural partners for wide BVHs.

---

**GPU traversal**

The same conceptual BVH works on GPUs, but the execution constraints change.

CPU:

```text
few powerful threads
large caches
out-of-order execution
branch prediction
SIMD
```

GPU:

```text
many lightweight threads
SIMT warps/waves
latency hiding
coalesced memory access
divergence concerns
limited per-thread local state
```

An explicit per-ray traversal stack consumes registers/local memory.

Too much stack state reduces occupancy.

A wide or stackless representation may therefore become attractive.

Packet coherence maps differently because GPU hardware already groups threads into execution warps/waves.

The tree layout can determine whether neighboring threads access nearby nodes or scatter across memory.

This is why BVHs used in hardware ray tracing are deeply tied to architecture.

The abstract tree is only the beginning.

---

**RT hardware perspective**

Modern ray-tracing hardware accelerates parts of the traversal/intersection pipeline.

Conceptually the application still supplies acceleration structures and rays.

The hardware can accelerate operations analogous to:

```text
ray-box traversal
hierarchy navigation
triangle intersection
```

But it cannot make a badly structured scene representation irrelevant.

Build mode, update mode, instance structure, memory footprint and dynamic geometry still affect performance.

A useful mental model is:

$$
\boxed{
\text{RT hardware accelerates BVH machinery;}
\quad
\text{it does not repeal spatial reasoning.}
}
$$

Understanding software BVHs remains essential for debugging hardware-accelerated ray tracing.

---

**Two-level BVHs**

Real rendering engines frequently distinguish geometry-level and scene-level hierarchy.

Conceptually:

```text
Top-Level Acceleration Structure

             TLAS
           /      \
       Instance A  Instance B
          |           |
          v           v
        BLAS A       BLAS B
          |           |
       triangles    triangles
```

A BLAS represents local geometry.

TLAS represents transformed instances.

Why do this?

Suppose one chair mesh appears 100 times.

We should not duplicate its triangle BVH 100 times.

Store one BLAS.

Each instance stores a transformation and a top-level bound.

Traversal becomes:

```text
world ray
  |
TLAS
  |
instance hit
  |
transform ray into instance local space
  |
BLAS
  |
triangle
```

This is the same local-space query strategy you already saw with triangle-mesh colliders.

It is a general engine pattern.

---

**Instancing and transform correctness**

For rigid transform:

$$
x_w=Rx_l+t.
$$

Ray origin transforms:

$$
O_l=R^{-1}(O_w-t).
$$

Direction:

$$
D_l=R^{-1}D_w.
$$

If scale exists, especially nonuniform scale, parameter/distance semantics become more subtle.

If you renormalize local direction, local $t$ no longer directly equals world-space $t$.

A robust instance traversal must preserve a clear parameter convention.

This is the same issue we covered in M077 mesh raycasts but now expressed as BVH hierarchy design.

---

**BVH build determinism**

Your builder uses `std::stable_sort`.

That gives equal-key primitives stable relative ordering.

Determinism matters for:

```text
reproducible debugging
networked simulations
benchmark comparisons
regression tests
research experiments
binary output comparison
```

However stable sorting alone does not guarantee cross-platform bit-identical trees.

Floating-point centroid computation, sort implementations, equal NaN behavior, compiler settings and parallel construction can still differ.

If strict reproducibility matters, define explicit tie-breaks.

For example:

```cpp
if (lhsCentroid < rhsCentroid)
    return true;

if (rhsCentroid < lhsCentroid)
    return false;

return lhsPrimitiveID < rhsPrimitiveID;
```

That turns equality into a total ordering.

For Morton keys:

```text
(code, primitiveID)
```

is even more explicit.

---

**Numerical considerations during BVH construction**

BVHs are often considered data-structure problems, but floating-point behavior matters.

AABB union repeatedly performs min/max:

$$
B_{\min}=\min(B_{\min},P_{\min})
$$

$$
B_{\max}=\max(B_{\max},P_{\max}).
$$

Ordinary finite numbers behave well here.

NaNs are dangerous because comparison semantics can poison or inconsistently ignore coordinates depending on implementation.

Therefore primitive bounds should be validated before build.

Your `BuildBVH()` already checks:

```cpp
if (!primitives[i].Bounds.IsValid())
    throw ...
```

That is the correct boundary.

However, `IsValid()` should itself ensure not merely:

$$
\min\le\max
$$

but also appropriate finite-coordinate requirements if the system expects them.

An infinite root bound would technically contain geometry but destroy useful SAH and traversal behavior.

---

**Degenerate primitive bounds**

A triangle perfectly parallel to XY can have:

$$
z_{\min}=z_{\max}.
$$

Its AABB has zero thickness along Z.

That is not automatically invalid.

Its surface area remains:

$$
2d_xd_y
$$

when $d_z=0$.

A point primitive could have:

$$
d_x=d_y=d_z=0.
$$

Then surface area is zero.

SAH divisions involving a zero-area parent require a fallback strategy.

Examples:

```text
all primitive centroids identical
all primitive bounds collapse to one point
all primitives lie on same degenerate geometric configuration
```

A safe builder should detect these cases and choose median/index splitting or create a leaf rather than divide by zero.

---

**BVH failure modes you should be able to diagnose**

A ray missing visible triangles despite brute-force intersection succeeding usually indicates an **incorrect bound or traversal false negative**, not merely a poor tree.

A correct BVH that is slower than brute force on some scene may indicate excessive overlap, tiny leaves, pathological structure or a workload too small to amortize traversal.

Stack overflow indicates pathological depth or inadequate traversal-stack capacity.

A dynamic mesh that becomes slower every frame despite correct output suggests refit quality degradation.

Random crashes after building often indicate node-reference invalidation, bad child indices, or primitive-order corruption.

Flickering misses around huge coordinates may originate in ray–AABB numerical precision rather than tree topology.

SAH build taking unexpectedly long may result from repeated sorting and allocations—the exact issue your current full-sweep implementation exposes.

Morton tree correct but slower than SAH is not contradictory. Build speed and traversal quality are separate objectives.

---

**Debugging a BVH visually**

Your `SpatialDebug.cppm` already exposes `ExtractBVHBoxes()`.

That is useful.

But note the current implementation writes:

```cpp
Depth = 0
```

for every extracted BVH node.

So the debug record does not currently reconstruct static-BVH depth.

An improved visualization should derive depth from parent links or traversal.

Then color nodes by hierarchy depth:

```text
depth 0   root
depth 1   children
depth 2   grandchildren
...
```

Useful visualizations include:

```text
node AABBs by depth
leaf occupancy
overlapping siblings
ray traversal order
visited node count
pruned nodes
Morton order
SAH split planes
refit before/after bounds
```

For SAH specifically, displaying candidate split planes and their costs can make an apparently mysterious builder completely understandable.

---

**Research bridge — what is actually worth testing here?**

A useful research-grade question is:

**Can a hybrid builder select among full SAH, binned SAH, Morton hierarchy and refit/rebuild using scene telemetry such that total build + query cost beats any one fixed strategy across changing workloads?**

This is related to, but distinct from, the adaptive broadphase research file already present in `KairoSpatial/research/adaptive_collision_pipeline.yaml`.

That file currently proposes choosing among:

```text
dynamic AABB tree
sweep-and-prune
spatial hash
```

using online density, velocity and churn telemetry.

The same philosophy can be brought inside the BVH family.

Let

$$
T_{\text{total}}
=
T_{\text{build/update}}
+
Q\,T_{\text{query}},
$$

where $Q$ is expected query count before the next update.

A high-quality SAH build is valuable when $Q$ is huge.

A fast Morton build may dominate when geometry changes every frame and only a small number of queries occur.

A refit dominates when motion is modest and tree topology remains good.

A rebuild dominates when refitting has degraded traversal badly.

Therefore the actual optimization target is not:

$$
\min T_{\text{build}}
$$

or:

$$
\min T_{\text{query}}.
$$

It is:

$$
\boxed{
\min
\left(
T_{\text{update}}
+
Q\,T_{\text{query}}
\right)
}
$$

subject to correctness and latency constraints.

That turns "which BVH is best?" into a measurable systems question rather than a naming contest.

---

**Repository audit summary**

Your current `KairoSpatial` design is a **binary, flat-array, index-linked BVH** with cached primitive bounds/centroids, stable primitive ordering, parent indices and configurable leaf/depth limits.

Its median builder chooses the largest centroid axis and recursively median-splits.

Its SAH path performs exact candidate sweeps over centroid-sorted primitives for all three axes, using:

$$
A_LN_L+A_RN_R
$$

as the split objective.

Its Morton path performs 10-bit-per-axis centroid quantization and 30-bit Morton ordering, followed by midpoint recursive splitting.

Its closest-hit traversal uses an explicit vector stack, current-distance pruning and near-child-first ordering.

Its any-hit traversal short-circuits.

It supports AABB, sphere, point and frustum BVH queries.

Its tests verify correctness against direct/brute-force expectations, including a 4096-primitive Morton-ordered stress case.

It currently does **not** implement canonical prefix-based LBVH topology construction, static-BVH refitting, static rebuild heuristics, wide BVHs, quantized bounds or packet SIMD traversal.

Those are not criticisms—they are simply the exact boundary between the repository's current implementation and the complete M079/M080 curriculum.

---

**Compact recall sheet — M079**

**CONCEPT:** Use conservative hierarchical bounds so one cheap query can eliminate many expensive primitive tests.

**REPRESENTATION:**

```text
BVH
├── Nodes[]
│    ├── Bounds
│    ├── Left/Right
│    ├── FirstPrimitive/Count
│    └── Parent
├── Primitives[]
└── PrimitiveOrder[]
```

**CORE INVARIANT:**

$$
B_N\supseteq
\bigcup_{\text{subtree}}B_P.
$$

**MEDIAN SPLIT:**

$$
axis=\arg\max
(C_{\max}-C_{\min}),
$$

sort centroids, split middle.

**SAH:**

$$
C_{\text{split}}
=
C_T+
C_I
\frac{
A_LN_L+A_RN_R
}{
A_P
}.
$$

Compare against:

$$
C_{\text{leaf}}
=
NC_I.
$$

**BINNED SAH:** centroid → fixed bins → prefix/suffix bounds → evaluate $B-1$ boundaries.

**MORTON:** normalize → quantize → interleave bits → sort.

**LBVH:** sorted Morton keys → longest common prefixes → radix-tree topology → bounds.

**KAIRO REALITY:** Morton sort exists; prefix/radix LBVH topology does not yet. Current Morton mode splits sorted ranges by midpoint.

---

**Compact recall sheet — M080**

**DEPTH-FIRST TRAVERSAL:**

```text
push root

while stack:
    pop node
    reject if query misses bounds

    if leaf:
        test primitives
    else:
        push children
```

**CLOSEST HIT:**

$$
t_{\text{best}}
\downarrow
$$

as hits improve.

Reject subtree when:

$$
t_{\text{nodeNear}}>t_{\text{best}}.
$$

Visit near child first to discover tight $t_{\text{best}}$ early.

**ANY HIT:** terminate at first accepted primitive intersection.

**OVERLAP:** node AABB prune → leaf bound check.

**POINT:** zero-size AABB query.

**FRUSTUM:** conservative AABB/frustum pruning.

**REFIT:**

$$
B_{\text{leaf}}
=
\bigcup B_P,
$$

then:

$$
B_{\text{parent}}
=
B_L\cup B_R.
$$

**REBUILD:** rerun topology generation when refitted hierarchy quality deteriorates.

**WIDE BVH:** fewer levels, more child tests, natural SIMD mapping.

**QUANTIZATION:** encode child bounds relative to parent; round outward to preserve containment.

**PACKET SIMD:** one lane per ray; useful with coherent rays, degraded by divergence.

**CURRENT KAIRO:** BVH2 + explicit stack + near-first closest traversal. No static refit/wide/quantized/packet implementation yet.

---

**Reconstruction challenge**

Before Pass 08, you should be able to reconstruct the following from a blank page without memorized prose.

Derive why SAH contains the ratio $A_C/A_P$, then derive the leaf-versus-split decision including $C_T$ and $C_I$. Explain why the simplified Kairo expression preserves split ranking but changes the formal stopping model. Starting from three 10-bit coordinates, explain how Morton dilation and interleaving produce one 30-bit key, then explain why sorting those keys is **not by itself** canonical LBVH construction. Draw a flat `BVHNode[]` representation and manually execute a closest-hit ray with near-first traversal, showing exactly when `tClosest` removes a pending subtree. Then explain why refitting preserves correctness while potentially destroying performance, derive bottom-up refit, and finally explain why BVH8 + quantized child bounds + SIMD is a coherent design rather than three unrelated optimizations.

If you can do those six things and implement the skeletons without copying, you own the central machinery of this pass.

**Pass 07 is complete in coverage: M079 + M080, Parts 1301–1336.**

**Next — Pass 08: M081 + M082, Parts 1337–1374.** That moves from largely static BVHs into **dynamic AABB trees, fat bounds, insertion/removal heuristics, sweep-and-prune, temporal coherence, spatial hashes, dense/hashed grids, octrees, quadtrees, KD-trees, BSP/sphere hierarchies, scene graphs, portals/PVS and hierarchical occlusion.** [NVIDIA](https://research.nvidia.com/publication/2012-06_maximizing-parallelism-construction-bvhs-octrees-and-k-d-trees?utm_source=chatgpt.com)


### Dynamic Spatial Indices, Sweep-and-Prune, Hash Grids, Octrees, Quadtrees, KD-Trees, BSP, Sphere Hierarchies, Portals/PVS and Hierarchical-Z

**Coverage:** M081 owns **Parts 1337–1356** and M082 owns **Parts 1357–1374**. The curriculum explicitly requires every part in both ranges with no skips. Graphics_Engine_Mastery_Integra… Graphics_Engine_Mastery_Integra…

We finished Pass 07 with a mainly static hierarchy:

$$
\text{primitives}
\rightarrow
\text{BVH construction}
\rightarrow
\text{many accelerated queries}.
$$

Pass 08 changes the problem.

A game world is not static. Characters move. Projectiles move. Doors open. Particles spawn and die. The camera crosses rooms. A physics broadphase is queried every frame. A renderer wants to discard objects that are not merely outside the frustum, but actually hidden behind other geometry.

So the question becomes:

$$
\boxed{
\text{How do we organize a world whose spatial relationships themselves change?}
}
$$

And there is no single universal answer.

A dynamic AABB tree trades slightly inflated bounds for cheap motion. Sweep-and-prune exploits temporal coherence. Hash grids exploit approximately uniform object scale. Octrees explicitly subdivide space. KD-trees dominate many point-neighbor problems. Portals exploit architectural connectivity. Hierarchical-Z exploits the image-space depth already generated by the renderer.

The entire pass is ultimately about **matching the index to the structure of the workload**.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/27 - Preparing and publishing runtime content|Previous chapter]] · [[Chapter Edition/03 - Game Engine/29 - Dynamic spatial indices for moving worlds|Next chapter]]
