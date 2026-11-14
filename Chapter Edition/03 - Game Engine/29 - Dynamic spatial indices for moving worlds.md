# 29 — Dynamic spatial indices for moving worlds

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/28 - BVH traversal, refitting, and SIMD|Previous chapter]] · [[Chapter Edition/03 - Game Engine/30 - Depth, projection, and shadow-map precision|Next chapter]]

A static hierarchy is not the only answer when objects move or appear in clusters. This chapter compares dynamic trees, sweep-and-prune, spatial hashing, and grids by update and query cost.

**By the end:** Choose a dynamic broad-phase structure for a stated motion and density workload, including duplicate suppression and update rules.

**Recall question:** What scene change makes this structure expensive, and how would you measure candidate quality as well as update time?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 68707–72762; source-span SHA256 0a1d95e8e64589028feba2df9ad295ec874ad0af8d1b6b600776ddefbc76e85b. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### Dynamic AABB Trees, Sweep-and-Prune, Spatial Hashes and Uniform Grids

---

**Dynamic AABB tree node representation**

A static BVH answers:

> Given a fixed set of primitives, what tree should I build?

A dynamic AABB tree answers:

> How can I preserve a useful hierarchy while objects continually enter, leave and move?

The tree remains binary:

```text
                         ROOT
                        /    \
                       /      \
                  Internal    Leaf D
                  /      \
               Leaf A   Internal
                        /      \
                     Leaf B   Leaf C
```

But leaves now need stable proxy identities, because objects can be updated independently.

A conceptual node is:

```cpp
struct DynamicNode
{
    AABB bounds;

    uint32_t parent;
    uint32_t left;
    uint32_t right;

    uint32_t height;

    ObjectID object;
};
```

Your actual `KairoSpatial/DynamicAABBTree.cppm` goes further:

```cpp
struct DynamicAABBTreeNode final
{
    SpatialAABB TightBounds = SpatialAABB::Empty();
    SpatialAABB FatBounds = SpatialAABB::Empty();

    SpatialIndex Parent = SpatialInvalidIndex;
    SpatialIndex Left = SpatialInvalidIndex;
    SpatialIndex Right = SpatialInvalidIndex;

    SpatialIndex NextFree = SpatialInvalidIndex;

    std::uint32_t Height = 0;

    SpatialID UserID = SpatialInvalidID;
    SpatialLayerMask LayerMask = 1u;

    bool Allocated = false;

    constexpr bool IsLeaf() const noexcept
    {
        return Left == SpatialInvalidIndex;
    }
};
```

There are two bounds per leaf:

$$
B_{\text{tight}}
$$

and

$$
B_{\text{fat}}.
$$

That distinction is the defining feature of the dynamic-tree strategy.

Internal nodes only really need conservative hierarchy bounds. Leaves additionally retain the object's actual current bound.

The tree returns a `SpatialIndex` when inserting an object. That is a **proxy handle**:

```cpp
SpatialIndex proxy =
    tree.Insert(id, bounds);
```

Updates then address the leaf directly:

```cpp
tree.Update(proxy, newBounds);
```

This matters enormously.

Without a proxy, updating one object might require searching for it.

With a proxy:

$$
\text{object update}
\rightarrow
\text{direct node index}.
$$

Box2D uses the same general architecture: its dynamic tree is a binary AABB hierarchy whose leaves are externally managed proxies and whose nodes use relocatable indices rather than arbitrary object-tree pointers. [Box2D](https://box2d.org/documentation/group__tree.html?utm_source=chatgpt.com)

---

**Tight bounds versus fat bounds**

Suppose an object's current exact AABB is:

$$
B_t=
[\mathbf m,\mathbf M].
$$

If the object moves by an extremely small distance every frame and we store only $B_t$, its leaf may need reinsertion every frame.

That defeats much of the point of maintaining the tree.

Instead create a larger box:

$$
B_f=B_t\oplus \delta
$$

where $\delta$ is a spatial margin.

For isotropic margin $f$:

$$
B_{f,\min}
=
B_{t,\min}-(f,f,f)
$$

and

$$
B_{f,\max}
=
B_{t,\max}+(f,f,f).
$$

Now the object can move around inside its fat box without modifying tree topology.

```text
Fat AABB

+----------------------------+
|                            |
|       Tight AABB           |
|       +----------+         |
|       | object   |         |
|       +----------+         |
|                            |
+----------------------------+
```

The update rule becomes:

$$
B_{\text{new}}\subseteq B_f
\quad\Longrightarrow\quad
\text{no tree reinsertion}.
$$

Only when the new tight bound escapes:

$$
B_{\text{new}}\not\subseteq B_f
$$

do we remove and reinsert the leaf.

This trades two costs.

A larger fat margin decreases:

$$
\text{reinsertion frequency}
$$

but increases:

$$
\text{false-positive overlap}.
$$

A smaller fat margin does the reverse.

That is an important general pattern:

$$
\boxed{
\text{spatial slack}
\leftrightarrow
\text{update frequency}.
}
$$

Your implementation currently uses a fixed isotropic margin:

```cpp
node.TightBounds = bounds;
node.FatBounds =
    Expanded(bounds, m_FattenAmount);
```

with default:

```cpp
float m_FattenAmount = 0.1f;
```

and update:

```cpp
if (m_Nodes[proxy].FatBounds.ContainsAABB(newBounds))
{
    m_Nodes[proxy].TightBounds = newBounds;
    return false;
}
```

Returning `false` means no reinsertion occurred.

That is exactly the intended dynamic-tree optimization.

---

**Predictive fattening**

A fixed margin assumes all objects have similar motion.

But suppose object A moves at:

$$
0.01\,m/s
$$

and object B at:

$$
100\,m/s.
$$

Giving both the same spatial margin is crude.

A stronger approach incorporates displacement.

Let frame displacement be

$$
\Delta x=v\Delta t.
$$

Then the future-motion bound could be extended toward motion direction.

For X:

$$
\Delta x>0
\Rightarrow
M_x'=M_x+f+k\Delta x
$$

while

$$
\Delta x<0
\Rightarrow
m_x'=m_x-f+k\Delta x.
$$

This creates an asymmetric predictive fat box.

Conceptually:

```text
stationary padding

       +------------+
       |   object   |
       +------------+


object moving right --->

       +-------------------------------+
       | object   predicted region     |
       +-------------------------------+
```

Older Box2D documentation describes both a fixed AABB extension and a displacement multiplier for predictive fattening; its dynamic tree reinserted proxies only after they escaped their fat AABB. [Box2D](https://box2d.org/doc_version_2_4/classb2_dynamic_tree.html?utm_source=chatgpt.com)

Your current Kairo tree does **not** use displacement prediction.

That is not incorrect; it is simply a simpler policy.

For fast physics objects, a velocity-aware fattening mode would be a natural extension.

---

**Dynamic-tree insertion heuristic**

When inserting a new leaf, where should it go?

A completely naïve strategy could descend left/right randomly.

Correctness would survive because ancestor bounds could always be expanded.

Performance would become terrible.

We instead want to choose a sibling such that the additional bounding-box area introduced is small.

Suppose candidate child $C$ has bound $B_C$, and new leaf has bound $B_L$.

After combining them:

$$
B'=
B_C\cup B_L.
$$

The surface-area increase is:

$$
\boxed{
\Delta A_C
=
A(B_C\cup B_L)-A(B_C)
}
$$

The greedy insertion rule chooses the child with smaller $\Delta A$.

Your implementation does exactly this:

```cpp
const float leftCost =
    Union(
        m_Nodes[left].FatBounds,
        m_Nodes[leaf].FatBounds
    ).SurfaceArea()
    -
    m_Nodes[left].FatBounds.SurfaceArea();

const float rightCost =
    Union(
        m_Nodes[right].FatBounds,
        m_Nodes[leaf].FatBounds
    ).SurfaceArea()
    -
    m_Nodes[right].FatBounds.SurfaceArea();

sibling =
    leftCost < rightCost
        ? left
        : right;
```

This continues until a leaf is found.

Then a new internal parent is created:

```text
before

         oldParent
             |
          sibling


after

         oldParent
             |
          newParent
          /      \
     sibling    newLeaf
```

and

$$
B_{\text{newParent}}
=
B_{\text{sibling}}
\cup
B_{\text{leaf}}.
$$

Ancestors are refitted upward.

---

**The stronger insertion-cost model**

The simple greedy test only considers:

$$
\text{local child area increase}.
$$

But enlarging a child can force all of its ancestors to enlarge too.

A more complete heuristic considers **inherited cost**.

Suppose current node has area $A_N$, and inserting the leaf causes:

$$
A_N'
=
A(B_N\cup B_L).
$$

The parent-area increase is:

$$
\Delta A_N=A_N'-A_N.
$$

If we descend into one child, that extra area may propagate through several levels.

A stronger descent cost therefore resembles:

$$
C_L
=
\Delta A_{\text{left local}}
+
C_{\text{inherit}}
$$

and

$$
C_R
=
\Delta A_{\text{right local}}
+
C_{\text{inherit}}.
$$

You can also compare against making the new leaf a sibling of the **current** node immediately.

This turns insertion into a small local SAH problem.

Your current Kairo implementation uses the simpler greedy local increase.

It is cheap and understandable, but it can choose a locally attractive child that is globally poor.

---

**Dynamic-tree leaf removal**

Consider:

```text
        G
        |
        P
       / \
      L   S
```

where:

- $L$ = leaf being removed,
- $S$ = its sibling,
- $P$ = their parent,
- $G$ = grandparent.

After removing $L$, parent $P$ is unnecessary.

So replace $P$ with $S$:

```text
before                 after

      G                      G
      |                      |
      P                      S
     / \
    L   S
```

Then refit ancestors of G.

Your implementation follows this exactly:

```cpp
const SpatialIndex parent =
    m_Nodes[leaf].Parent;

const SpatialIndex grandParent =
    m_Nodes[parent].Parent;

const SpatialIndex sibling =
    m_Nodes[parent].Left == leaf
        ? m_Nodes[parent].Right
        : m_Nodes[parent].Left;
```

Then if grandparent exists:

```cpp
grandParent.child = sibling;
sibling.Parent = grandParent;

RefitAncestors(grandParent);
```

Finally:

```cpp
FreeNode(parent);
```

If the removed leaf was the root, the tree simply becomes empty.

If its parent was the root, its sibling becomes the new root.

Those edge cases are essential.

---

**A current node-pool issue in Kairo**

Your node representation contains:

```cpp
SpatialIndex NextFree;
```

which strongly suggests a free-list node pool was intended.

But current allocation does:

```cpp
SpatialIndex AllocateNode()
{
    const SpatialIndex index =
        static_cast<SpatialIndex>(m_Nodes.size());

    m_Nodes.push_back(DynamicAABBTreeNode{});
    ...
    return index;
}
```

and freeing does:

```cpp
m_Nodes[index] =
    DynamicAABBTreeNode{};
```

There is currently no free-list reuse.

Therefore repeated insertion/removal can produce:

```text
[allocated][free][free][allocated][free] ... new nodes appended forever
```

`NextFree` is presently unused.

For a long-running physics world with high churn, the vector can grow according to historical maximum allocation traffic rather than current live node count.

A node pool should instead maintain:

```text
freeHead -> free node -> free node -> ...
```

Then:

```cpp
SpatialIndex AllocateNode()
{
    if (freeHead != Invalid)
    {
        const auto index = freeHead;
        freeHead = nodes[index].NextFree;

        nodes[index] = Node{};
        nodes[index].Allocated = true;

        return index;
    }

    ...
}
```

and `FreeNode` should push the index back onto the free list.

That is a concrete improvement available directly from the fields already present.

---

**Dynamic proxy updates**

A dynamic update has two possible paths.

**Cheap path**

$$
B_{\text{new}}\subseteq B_{\text{fat}}.
$$

Only:

```cpp
TightBounds = newBounds;
```

changes.

Tree topology remains untouched.

**Expensive path**

$$
B_{\text{new}}\not\subseteq B_{\text{fat}}.
$$

Then:

```text
remove leaf
    ↓
update tight bound
    ↓
create new fat bound
    ↓
reinsert leaf
    ↓
refit ancestors
```

Your implementation:

```cpp
RemoveLeaf(proxy);

m_Nodes[proxy].TightBounds =
    newBounds;

m_Nodes[proxy].FatBounds =
    Expanded(
        newBounds,
        m_FattenAmount);

InsertLeaf(proxy);
```

The proxy index remains stable because only its tree location changes.

That is useful for physics systems holding collider-to-proxy mappings.

---

**Why not simply change the leaf bound in place?**

Suppose a leaf originally belongs in the far-left portion of the tree:

```text
LEFT WORLD                        RIGHT WORLD

[leaf]
```

Then the object teleports to the right.

If we merely change its bound and refit ancestors:

```text
left subtree bound
+---------------------------------------------+
|                                             |
|                                      [leaf] |
+---------------------------------------------+
```

The tree is correct but spatially awful.

The object's hierarchy location no longer corresponds to spatial proximity.

Remove/reinsert lets the insertion heuristic choose a more suitable sibling.

So:

$$
\boxed{
\text{refit handles small motion;}
\quad
\text{reinsert handles topological relocation.}
}
$$

---

**Rotations and balancing**

A dynamic tree can become unbalanced.

Imagine inserting objects in monotonically increasing X order with an unlucky insertion strategy:

```text
root
  \
   A
    \
     B
      \
       C
        \
         D
```

Depth becomes approximately:

$$
O(N)
$$

instead of:

$$
O(\log N).
$$

We need rotations.

The concept resembles AVL-tree rotations, but a spatial tree's quality objective is not merely key ordering.

There is no strict binary-search ordering constraint.

We care about:

$$
\text{height}
$$

and:

$$
\text{bounding-volume cost}.
$$

Consider:

```text
              A
             / \
            B   C
               / \
              F   G
```

If C becomes much taller than B, rotate C upward:

```text
              C
             / \
            A   G
           / \
          B   F
```

Then recompute:

$$
B_A=B_B\cup B_F
$$

and

$$
B_C=B_A\cup B_G.
$$

Also recompute heights:

$$
h_A=1+\max(h_B,h_F)
$$

$$
h_C=1+\max(h_A,h_G).
$$

The spatial variant can choose whether F or G moves based on resulting area.

---

**What balancing should optimize**

A simple policy minimizes height:

$$
|h_L-h_R|.
$$

A more spatially aware policy estimates area cost:

$$
A(B_{\text{new child}})
+
A(B_{\text{new parent}}).
$$

The best rotation may be the one reducing traversal cost, even if height does not become perfectly balanced.

This is why dynamic AABB trees are related to balanced binary trees but are not ordinary AVL search trees.

Current Box2D documentation explicitly notes that its dynamic tree uses rotations to maintain balance, including degenerate insertion sequences. [Box2D](https://box2d.org/doc_version_2_4/md__e_1_2github_2box2d__24_2docs_2collision.html?utm_source=chatgpt.com)

**Current Kairo status**

Your tree tracks:

```cpp
Height
```

and computes diagnostic:

```cpp
MaxBalance
```

where balance is child-height difference.

But there is currently **no rotation/balance operation** in `DynamicAABBTree.cppm`.

So:

$$
\boxed{
\text{imbalance is measured but not actively repaired}.
}
$$

That is an important Part 1342 implementation gap.

---

**Dynamic-tree overlap queries**

Traversal uses fat bounds for hierarchy pruning:

```cpp
if (!Intersects(node.FatBounds, query))
    continue;
```

Why fat bounds?

Because internal hierarchy bounds are constructed from fat leaves.

If query misses a fat internal node, it certainly misses every tight object underneath.

At a leaf, however, your tree performs:

```cpp
Intersects(node.TightBounds, query)
```

before returning the object.

That is excellent.

It means fat bounds cause additional traversal but do not necessarily leak false-positive AABB results to the caller.

The pipeline is:

```text
query
  |
  v
fat hierarchy bounds
  |
  v
candidate leaf
  |
  v
tight leaf bound
  |
  v
return ID
```

That mirrors broadphase/narrower broadphase reasoning.

---

**Pair generation in a dynamic tree**

Physics broadphase wants candidate pairs:

$$
(A,B)
$$

whose bounds overlap.

Your current implementation iterates every leaf, executes a tree query using that leaf's tight bound, adds every other returned ID, orders pairs, sorts them, then removes duplicates.

That is correct for tested AABB overlap.

But observe the work.

If A overlaps B:

```text
query A finds B
query B finds A
```

so the pair appears twice before deduplication.

A more simulation-oriented broadphase often tracks **moved proxies**.

Only objects that moved enough to invalidate their previous broadphase relationship need fresh queries.

Then:

```text
moved proxy
    |
query fat/tight tree
    |
generate new candidate pairs
```

instead of querying every object every frame.

This exploits another form of temporal coherence.

---

**`RebuildBottomUp()` is currently not really a bottom-up optimal rebuild**

Your function:

```cpp
void RebuildBottomUp()
{
    collect live leaves;
    Clear();

    for (leaf : leaves)
        Insert(...);
}
```

rebuilds through repeated ordinary insertion.

It does **not** perform the traditional expensive bottom-up algorithm that repeatedly chooses node pairs whose union produces the best cost.

So its name currently means:

> discard hierarchy and reinsert everything

rather than:

> construct an optimized hierarchy bottom-up.

Your tests correctly verify an important property: repeated rebuilds do not accidentally fatten already-fat bounds again.

That is good.

But tree quality remains determined by the normal greedy insertion heuristic.

---

**Dynamic-tree quality metrics**

Your tree exposes:

```cpp
Height
MaxBalance
AreaRatio
AllocatedNodeCount
LeafCount
```

The most interesting is:

$$
\boxed{
\text{AreaRatio}
=
\frac{
\sum_i A(B_i)
}{
A(B_{\text{root}})
}
}
$$

roughly speaking.

If many internal bounds overlap heavily or become inflated, total node area rises.

High area ratio often correlates with poorer pruning.

Box2D exposes similar tree metrics, including tree height and area ratio, specifically to evaluate hierarchy quality. [Box2D](https://box2d.org/documentation/group__tree.html?utm_source=chatgpt.com)

These metrics can drive automatic rebuild logic.

For example:

$$
\text{if AreaRatio}>\tau_A
$$

or:

$$
\text{MaxBalance}>\tau_B
$$

or:

$$
\frac{
\text{visited nodes/query}
}{
\text{historical baseline}
}>\tau_Q,
$$

schedule rebuild or local rotations.

The last metric is usually the most directly relevant: measure actual workload cost.

---

**Sweep-and-prune endpoints**

Now consider an entirely different strategy.

Every AABB projects onto X as an interval:

$$
I_i=[x_i^{\min},x_i^{\max}].
$$

Two 3D AABBs cannot intersect unless their X intervals overlap.

Therefore, sort interval endpoints along X.

Object A:

```text
Amin ---------------- Amax
```

Object B:

```text
       Bmin ---------------- Bmax
```

A and B overlap on X because:

$$
B_{\min}\le A_{\max}.
$$

A canonical sweep-and-prune system stores endpoints:

```cpp
struct Endpoint
{
    float value;
    ObjectID object;
    bool isMin;
};
```

Each AABB contributes:

```text
[min, max]
```

for each chosen axis.

Interestingly, your repository already defines:

```cpp
struct SAPEndpoint final
{
    float Value;
    SpatialID ID;
    bool IsMin;
};
```

but the current algorithm does **not use it**.

That will matter shortly.

---

**Active sets**

Sort endpoints left-to-right.

Maintain an active set.

When encountering a minimum endpoint, the object begins overlapping the sweep position.

Every object already active overlaps this object along the sweep axis.

When encountering a maximum endpoint, remove the object.

Example:

```text
position →

Amin     Bmin     Amax       Cmin     Bmax      Cmax
 |        |         |          |        |         |

active:
{A}
{A,B}      => candidate A-B
{B}
{B,C}      => candidate B-C
{C}
{}
```

Canonical pseudocode:

```cpp
sort(endpoints);

ActiveSet active;

for (const Endpoint& e : endpoints)
{
    if (e.isMin)
    {
        for (ObjectID other : active)
            candidates.emplace(e.id, other);

        active.insert(e.id);
    }
    else
    {
        active.erase(e.id);
    }
}
```

This only proves overlap on one axis.

Then Y/Z can confirm.

---

**Endpoint tie ordering**

Suppose:

$$
A_{\max}=B_{\min}.
$$

Do touching boxes count as overlap?

Your AABB intersection convention uses inclusive comparisons, so touching should count.

Therefore when coordinates are equal, the **minimum endpoint should be processed before the maximum endpoint**.

Otherwise A may be removed before B is inserted and the touching pair is missed.

So endpoint comparator should be something like:

```cpp
if (a.value != b.value)
    return a.value < b.value;

// Inclusive contact: starts before ends.
if (a.isMin != b.isMin)
    return a.isMin && !b.isMin;

return a.id < b.id;
```

This is an example of topology hiding inside sorting semantics.

---

**Temporal coherence in sweep-and-prune**

The real power of sweep-and-prune is not merely sorting intervals.

It is that dynamic scenes are usually temporally coherent.

Between adjacent frames:

$$
x_i(t+\Delta t)
\approx
x_i(t).
$$

Therefore the endpoint array from the previous frame is already *almost sorted*.

Instead of sorting from scratch:

$$
O(N\log N),
$$

maintain the endpoint array across frames.

When objects move slightly, only a few endpoints cross neighbors.

If the number of inversions is $K$, insertion-sort-like maintenance costs roughly:

$$
O(N+K).
$$

When $K\ll N$:

$$
O(N+K)\approx O(N).
$$

That is why sweep-and-prune can be extremely effective in smoothly moving physics worlds.

---

**Insertion sort for nearly sorted endpoints**

Insertion sort's worst-case complexity is:

$$
O(N^2).
$$

So why use it?

Because for nearly sorted arrays its cost is:

$$
O(N+K)
$$

where $K$ is the number of inversions.

A local endpoint movement looks like:

```text
before

Amin Bmin Cmin Amax Bmax Cmax


after objects move slightly

Amin Cmin Bmin Amax Bmax Cmax
          ^
only one local inversion
```

Insertion sort fixes this with one small shift.

A more advanced SAP implementation can generate overlap begin/end events as endpoints swap.

For example, when a min endpoint crosses another object's max endpoint, an overlap can begin or end depending on crossing direction.

This means pair maintenance and endpoint sorting can be integrated.

---

**Current Kairo sweep-and-prune is actually sort-and-sweep**

Your current `SweepAndPrune::ComputePairs()` does:

```cpp
std::stable_sort(
    sorted.begin(),
    sorted.end(),
    [](const SAPProxy* lhs,
       const SAPProxy* rhs)
    {
        return lhs->Bounds.Min.x <
               rhs->Bounds.Min.x;
    });
```

Then:

```cpp
for each A:
    for each later B:
        if B.minX > A.maxX:
            break

        if Intersects(A.Bounds, B.Bounds):
            pair
```

This is a valid sweep broadphase.

But it does **not** currently implement:

- persistent endpoint arrays,
- active-set endpoint processing,
- temporal-coherence sorting,
- insertion sort.

`SAPEndpoint` exists but is not wired into the current algorithm.

Every `ComputePairs()` performs a fresh `stable_sort`.

So Part 1344–1347 represent meaningful future optimization work.

---

**Complexity of the current Kairo SAP**

Sorting active proxies:

$$
O(N\log N).
$$

Then candidate scanning depends on X overlap.

Best case, intervals rarely overlap:

$$
O(N)
$$

after sorting.

Worst case, every X interval overlaps every other:

$$
O(N^2).
$$

For example, if all boxes span:

$$
x\in[-1000,1000],
$$

the X sweep rejects nothing.

The full 3D `Intersects` test may still eliminate pairs on Y/Z, but those candidate tests have already been generated.

This motivates multi-axis SAP.

---

**Multi-axis sweep-and-prune**

For two AABBs to overlap:

$$
X_A\cap X_B\ne\varnothing
$$

and:

$$
Y_A\cap Y_B\ne\varnothing
$$

and:

$$
Z_A\cap Z_B\ne\varnothing.
$$

A multi-axis system can maintain candidate sets independently:

$$
S_x,\quad S_y,\quad S_z.
$$

Final candidates are:

$$
\boxed{
S=S_x\cap S_y\cap S_z.
}
$$

A practical implementation may maintain a pair overlap counter.

For each pair:

```text
X overlap -> bit 0
Y overlap -> bit 1
Z overlap -> bit 2
```

If mask becomes:

```text
111₂
```

the pair overlaps on all axes.

This works nicely with endpoint crossing events.

When endpoints change order, update the appropriate axis bit.

Then full candidate pairs can be maintained incrementally rather than regenerated every frame.

---

**Choosing the best SAP axis**

Single-axis sweep should prefer an axis that creates few candidate overlaps.

A common heuristic is largest variance or scene extent.

But the optimal axis is the one minimizing interval overlap.

Imagine a corridor:

```text
X axis:

[A][B][C][D][E][F]

Y axis:

[------ every object overlaps ------]
```

Sweep X is excellent.

Sweep Y is terrible.

A dynamic system can measure active-set size or pair generation per axis and choose whichever axis is currently most discriminating.

This connects directly to the adaptive spatial-policy research direction already present in your repository.

---

**Spatial hash grid coordinates**

A spatial hash divides space into uniform cells of size $h$.

For world point:

$$
p=(x,y,z),
$$

cell coordinates are:

$$
\boxed{
c_x=\left\lfloor\frac{x}{h}\right\rfloor
}
$$

$$
c_y=\left\lfloor\frac{y}{h}\right\rfloor
$$

$$
c_z=\left\lfloor\frac{z}{h}\right\rfloor.
$$

Why **floor**, rather than integer truncation?

Consider:

$$
x=-0.2,\quad h=1.
$$

Mathematically:

$$
\lfloor-0.2\rfloor=-1.
$$

But C++ integer truncation:

```cpp
static_cast<int>(-0.2f)
```

produces:

$$
0.
$$

That would place negative positions in the wrong cell.

Your implementation correctly uses:

```cpp
std::floor(position.x / m_CellSize)
```

before converting to integer.

That is exactly what we want.

---

**Mapping an AABB to hash cells**

For AABB:

$$
B=[B_{\min},B_{\max}],
$$

calculate:

$$
C_{\min}
=
\operatorname{Cell}(B_{\min})
$$

and:

$$
C_{\max}
=
\operatorname{Cell}(B_{\max}).
$$

Then enumerate:

$$
x\in[C_{\min,x},C_{\max,x}]
$$

$$
y\in[C_{\min,y},C_{\max,y}]
$$

$$
z\in[C_{\min,z},C_{\max,z}].
$$

Your implementation:

```cpp
for (int z = minCell.z;
     z <= maxCell.z;
     ++z)
{
    for (int y = minCell.y;
         y <= maxCell.y;
         ++y)
    {
        for (int x = minCell.x;
             x <= maxCell.x;
             ++x)
        {
            ...
        }
    }
}
```

If an object spans:

$$
n_x
$$

cells in X,

$$
n_y
$$

in Y, and

$$
n_z
$$

in Z, insertion costs approximately:

$$
\boxed{
O(n_xn_yn_z).
}
$$

This is the spatial hash's most important weakness.

A giant object can occupy thousands or millions of cells.

---

**Boundary semantics in hash cells**

Suppose cell size is 1 and an AABB is:

$$
x\in[0,1].
$$

Then:

$$
\operatorname{Cell}(0)=0,
$$

$$
\operatorname{Cell}(1)=1.
$$

So the box is inserted into cells 0 and 1.

Is that wrong?

Not necessarily.

Your AABB overlap convention treats touching boundaries as intersection.

The point at $x=1$ belongs geometrically to the closed box.

So inserting into the next cell is conservative.

It increases duplication but avoids missing boundary-touching queries.

If instead the engine used half-open intervals:

$$
[min,max)
$$

the max mapping policy could differ.

Again, representation conventions propagate into spatial indexing.

---

**Hashing integer coordinates**

An infinite conceptual grid has coordinates:

$$
(x,y,z)\in\mathbb Z^3.
$$

We do not want a gigantic dense array.

So map cell coordinate to a hash value.

Your implementation uses:

$$
h(x,y,z)
=
(73856093x)
\oplus
(19349663y)
\oplus
(83492791z).
$$

In code:

```cpp
const std::int64_t x =
    static_cast<std::int64_t>(coord.x)
    * 73856093;

const std::int64_t y =
    static_cast<std::int64_t>(coord.y)
    * 19349663;

const std::int64_t z =
    static_cast<std::int64_t>(coord.z)
    * 83492791;

return static_cast<std::size_t>(
    x ^ y ^ z);
```

The large distinct multipliers scramble regular grid patterns before XOR.

A good hash aims for roughly uniform bucket distribution.

It does **not** need to be collision-free.

---

**Two completely different meanings of “collision”**

This topic is worth making explicit.

A **spatial collision** means:

> two objects occupy the same spatial region/cell.

A **hash collision** means:

> two different integer keys produce the same hash bucket.

For example:

```text
cell (1,2,3)
cell (92,-4,17)

different cells
same hash bucket possible
```

That must not make them semantically the same cell.

`std::unordered_map<GridCoord3,...>` solves this by storing the actual key and using equality after hashing.

The hash only selects a bucket.

The full coordinate determines identity.

---

**Bucket collision handling**

Conceptually a hash table implements something like:

```text
hash(key) -> bucket

bucket:
    key A -> value
    key B -> value
    key C -> value
```

Different keys may occupy the same bucket.

The table searches the bucket and verifies:

$$
key_{\text{stored}}==key_{\text{query}}.
$$

Your `unordered_map` handles this automatically.

But there is another bucket issue at the **spatial** level.

One cell can contain many object IDs:

```cpp
unordered_map<
    GridCoord3,
    vector<SpatialID>
> m_Cells;
```

If a cell contains $K$ objects, scanning it costs:

$$
O(K).
$$

Therefore hash-table lookup can be $O(1)$ average while spatial-query cost is still terrible because one cell contains 10,000 objects.

Cell-size choice is critical.

---

**Inserting moving objects**

When object moves from cells:

$$
S_{\text{old}}
$$

to:

$$
S_{\text{new}},
$$

we need to remove it from old cells and add it to new cells.

Your implementation retains a reverse map:

```cpp
unordered_map<
    SpatialID,
    vector<GridCoord3>
> m_IDCells;
```

This is very useful.

Without it, removing object 42 would require searching the entire grid.

Instead:

```text
object ID 42
   |
   v
list of occupied cells
   |
remove 42 from those buckets
```

Your `Insert()` currently calls:

```cpp
Remove(id);
```

then completely reinserts the object's new bounds.

That is simple and correct.

A more incremental update can calculate:

$$
S_{\text{remove}}
=
S_{\text{old}}-S_{\text{new}}
$$

and:

$$
S_{\text{add}}
=
S_{\text{new}}-S_{\text{old}}.
$$

Cells in the intersection remain unchanged.

That can save work for slowly moving large objects.

---

**Neighbor-cell range queries**

Suppose we want all objects within radius $r$ of point $p$.

First find a conservative query box:

$$
Q=
[p-(r,r,r),\,
 p+(r,r,r)].
$$

Map Q to a cell range.

Visit each cell in that range.

Because an object can occupy several cells, the same ID can appear multiple times.

Therefore deduplicate.

Your implementation uses:

```cpp
std::unordered_set<SpatialID> unique;
```

Then performs actual bounds intersection before accepting the ID.

For radius query it further checks:

```cpp
OverlapsSphere(
    m_Bounds.at(id),
    center,
    radius);
```

So the pipeline is:

```text
sphere
  |
  v
sphere AABB
  |
  v
candidate grid cells
  |
  v
candidate IDs
  |
  v
deduplicate
  |
  v
AABB/sphere check
```

Again, spatial indexing narrows work; it does not automatically constitute final shape collision.

---

**Query complexity in a spatial hash**

Suppose a query spans:

$$
n_x\times n_y\times n_z
$$

cells.

Cell enumeration costs:

$$
O(n_xn_yn_z).
$$

Then if cell $i$ contains $K_i$ entries:

$$
O\left(
n_xn_yn_z
+
\sum_iK_i
\right)
$$

before deduplication overhead.

Thus performance depends on:

$$
\boxed{
\text{cell size}
+
\text{object size distribution}
+
\text{density}
+
\text{query size}.
}
$$

There is no magic $O(1)$ spatial hash query independent of the scene.

Hashing only makes cell lookup approximately constant-time.

It does not make the number of cells or objects constant.

---

**Uniform grids for broadphase**

A uniform grid is conceptually simpler than a hierarchy.

Divide space:

```text
+---+---+---+---+
|   | A | A |   |
+---+---+---+---+
| B | A | A |   |
+---+---+---+---+
| B |   | C | C |
+---+---+---+---+
```

Objects sharing a cell become broadphase candidates.

If the characteristic object diameter is approximately $d$, a common starting cell-size choice is:

$$
h\approx d.
$$

If:

$$
h\ll d,
$$

each object occupies many cells.

If:

$$
h\gg d,
$$

each cell contains many unrelated objects.

Both increase cost.

A useful simplified expected-work model is:

$$
C(h)
=
C_{\text{cell enumeration}}(h)
+
C_{\text{bucket candidate}}(h).
$$

Decreasing $h$ raises the first term.

Increasing $h$ raises the second.

Optimal $h$ lies somewhere between.

---

**Multi-level grids and heterogeneous object sizes**

Suppose a world contains:

```text
dust particle:   1 cm
human:           2 m
building:       50 m
mountain:     1000 m
```

One cell size cannot serve all four efficiently.

A common solution is hierarchical grids:

```text
fine grid     -> tiny objects
medium grid   -> ordinary objects
coarse grid   -> huge objects
```

or assign giant objects to a separate acceleration structure.

This is why grids work particularly well for relatively uniform-size populations such as:

- particle systems,
- boids,
- cloth nodes,
- fluid neighborhoods,
- many similar rigid bodies.

They become less attractive for extremely heterogeneous scenes.

---

**Dense grids versus hashed grids**

A dense grid explicitly allocates every cell.

For dimensions:

$$
N_x\times N_y\times N_z,
$$

cell count is:

$$
N_xN_yN_z.
$$

Example:

$$
1024^3
=
1,073,741,824
$$

cells.

Even four bytes per cell costs roughly 4 GiB before object lists.

But dense grids have important advantages:

$$
\text{index}
=
x+N_x(y+N_yz)
$$

is extremely cheap.

Cells are contiguous.

Cache behavior can be excellent.

GPU mapping is straightforward.

A hashed grid stores only occupied cells.

Memory becomes related to:

$$
N_{\text{occupied}}
$$

rather than world volume.

That makes it suitable for sparse or effectively unbounded scenes.

But you pay for:

- hashing,
- hash-table metadata,
- poorer locality,
- bucket allocations,
- collision handling.

Your `SpatialHash` is a hashed sparse grid.

The right choice depends on occupancy:

$$
\rho=
\frac{N_{\text{occupied}}}
{N_{\text{possible}}}.
$$

When $\rho$ is high and the domain is bounded, dense arrays can dominate.

When $\rho$ is tiny, dense allocation is wasteful.

---

### Octrees, Quadtrees, KD-Trees, BSP, Sphere Hierarchies, Scene Graphs, Portals/PVS and Hi-Z

M081 focused on dynamic broadphase structures.

M082 broadens the picture to explicit spatial decomposition and visibility structures.

---

**Octree node representation**

An octree recursively subdivides 3D space into eight octants.

A node represents one spatial region:

$$
B=[B_{\min},B_{\max}].
$$

Its center is:

$$
C=\frac{B_{\min}+B_{\max}}2.
$$

The three splitting planes are:

$$
x=C_x,\qquad
y=C_y,\qquad
z=C_z.
$$

Together they produce:

$$
2\times2\times2=8
$$

children.

Your current node is:

```cpp
struct Node final
{
    SpatialAABB Bounds;
    SpatialAABB LooseBounds;

    std::uint32_t Depth = 0;

    std::vector<OctreeItem> Items;

    std::array<
        std::unique_ptr<Node>,
        8
    > Children;
};
```

This is a genuine octree implementation.

Unlike your flat BVH arrays, it is pointer-based.

Every subdivision allocates eight child nodes separately.

That is simple but less compact/cache-friendly than a node pool.

---

**Octree subdivision**

Child number can be encoded using three bits:

```text
bit 0 -> high X?
bit 1 -> high Y?
bit 2 -> high Z?
```

For child $i$:

$$
i\&1
$$

selects X half,

$$
i\&2
$$

selects Y,

$$
i\&4
$$

selects Z.

Your implementation:

```cpp
const Vec3f childMin
{
    (i & 1)
        ? center.x
        : node.Bounds.Min.x,

    (i & 2)
        ? center.y
        : node.Bounds.Min.y,

    (i & 4)
        ? center.z
        : node.Bounds.Min.z
};
```

and similarly for max.

This is compact and deterministic.

The eight combinations correspond to all octants.

---

**Octree insertion**

A typical insertion policy is:

```text
insert item at node

if item completely fits child:
    descend

else:
    store in current node
```

Why not assign by centroid?

Suppose a huge object crosses the node center.

Its centroid may lie in one child, but the object itself overlaps several children.

Putting it only in the centroid's strict child could make queries through neighboring children miss it.

Therefore either:

1. store it at the ancestor that fully contains it, or
2. duplicate it into every overlapping child.

Your Kairo octree chooses ancestor storage.

When capacity is exceeded:

```cpp
if (!node.HasChildren() &&
    node.Items.size() >=
        MaxItemsPerNode)
{
    Subdivide(node);

    reinsert old items;
}
```

Then:

```cpp
if (Node* child =
        ChildContaining(node, item.Bounds))
{
    Insert(*child, item);
    return;
}

node.Items.push_back(item);
```

That is a standard containment-based octree policy.

---

**Why ancestor storage matters**

Consider:

```text
+-------------------------------+
|             |                 |
|       +--------------+        |
|       | BIG OBJECT   |        |
|-------|--------------|--------|
|       |              |        |
|       +--------------+        |
|             |                 |
+-------------------------------+
```

The object overlaps four or eight children.

If duplicated, one logical object may appear many times.

If stored in current node, it is tested once whenever that node is visited.

So:

$$
\boxed{
\text{small contained items descend;}
\quad
\text{large cross-boundary items stay high}.
}
$$

If many giant objects stay at the root, queries still test them frequently.

Again, heterogeneous object scale hurts regular spatial subdivision.

---

**Octree range query**

For query region Q:

$$
Q\cap B_N=\varnothing
\Rightarrow
\text{prune node}.
$$

Otherwise:

1. test objects stored directly in node,
2. recurse into children intersecting Q.

Your implementation does:

```cpp
if (!Intersects(
        node.LooseBounds,
        query))
{
    return;
}

for (const OctreeItem& item :
     node.Items)
{
    if (Intersects(
            item.Bounds,
            query))
    {
        result.push_back(item.ID);
    }
}
```

then recursively queries children.

The same structure supports sphere and frustum queries.

This is precisely why spatial hierarchies are reusable: only the bound-vs-query predicate changes.

---

**Loose octrees**

Strict octree child cells do not overlap.

That is elegant for points, but awkward for extended moving objects.

A loose octree gives each child an enlarged effective bound.

Suppose strict child half-size is $e$.

Loose factor $\lambda\ge1$ gives:

$$
e_{\text{loose}}
=
\lambda e.
$$

For:

$$
\lambda=1.5,
$$

the effective child region is 50% larger in each half-extent.

Sibling loose bounds overlap.

```text
strict children

+-------+-------+
|       |       |
|   A   |   B   |
|       |       |
+-------+-------+


loose children

+----------+
| A loose  |
|     +----------+
+-----| B loose  |
      +----------+
```

Why deliberately introduce overlap?

Because an object near a splitting plane can remain assigned to one child while moving slightly across the strict boundary.

This is analogous to fat AABBs in dynamic trees.

Again we trade:

$$
\text{fewer relocations}
\leftrightarrow
\text{more query overlap}.
$$

Your default:

```cpp
LooseFactor = 1.5f;
```

implements this.

---

**How Kairo constructs loose bounds**

Your code computes:

```cpp
const Vec3f amount =
    bounds.Extent()
    * (LooseFactor - 1.0f);

loose.Expand(amount);
```

With `Extent()` representing AABB half-extents, if strict half extent is $e$:

$$
amount=e(\lambda-1).
$$

After expanding both sides, new half extent is:

$$
e+e(\lambda-1)
=
\lambda e.
$$

So it implements the usual multiplicative loose factor.

---

**A subtle loose-octree consequence**

Because loose siblings overlap, one item may be completely contained by more than one child's loose bounds.

Your `ChildContaining()` loops children and returns the **first** matching child.

So assignment can depend on child iteration order.

Correctness can still hold because the item is stored exactly once and queries test all loose child bounds.

But the chosen placement may not be optimal.

A stronger policy might choose:

- child whose strict region contains the object's center,
- child producing smallest loose-bound relation,
- deepest fitting child using deterministic spatial bits.

Loose hierarchies trade unique geometric partitioning for update flexibility.

---

**A concrete current octree correctness hazard**

Your public `Octree::Insert()` validates:

```cpp
bounds.IsValid()
```

but does **not** currently require:

$$
m_{\text{WorldBounds}}
\supseteq
bounds.
$$

Now suppose world root is:

$$
[-10,10]^3
$$

but an item is inserted at:

$$
[100,101]^3.
$$

No child contains it, so it remains in the root's `Items`.

But a query around $[100,101]^3$ begins with:

```cpp
if (!Intersects(
        node.LooseBounds,
        query))
{
    return;
}
```

The root loose bounds do not intersect the query.

Therefore the root returns **before testing the root-stored out-of-world item**.

That creates a false negative.

So the current API needs one of these contracts:

$$
\boxed{
\text{require every inserted item inside root bounds}
}
$$

or:

$$
\boxed{
\text{dynamically expand/rebuild the root}
}
$$

or a special overflow collection tested separately.

At present, the containment precondition is not enforced.

That is an important real repository issue.

---

**Octree update/removal considerations**

Your `Insert(id,bounds)` first calls:

```cpp
Remove(id);
```

`Remove()` recursively searches the octree.

Without a map:

$$
ID\rightarrow Node*
$$

removal can cost:

$$
O(N_{\text{nodes}}+N_{\text{items}})
$$

in the worst case.

For mostly-static scene indexing this may be acceptable.

For thousands of moving objects every frame, it is not.

A production dynamic loose octree typically benefits from proxy handles similar to the dynamic AABB tree:

```text
object -> owning node
```

Then relocation can directly remove from its old node.

Also note `CollapseIfEmpty()` only destroys children when **all** child subtrees are empty.

It does not merge children when their combined population drops below a threshold.

A more complete tree may use:

```text
split when count > high threshold
merge when count < low threshold
```

with different thresholds to avoid split/merge oscillation.

That is hysteresis.

---

**Octree ray traversal**

Your current raycast checks:

```cpp
node.LooseBounds
```

then every item and recursively every child.

However it passes the original:

```cpp
maxDistance
```

throughout traversal.

It does not shrink the node-test interval to:

$$
result.Closest.Distance.
$$

It also does not visit children near-to-far.

Therefore if it finds a close hit, distant subtrees are still traversed whenever they intersect the original ray interval.

The BVH traversal from Pass 07 is more optimized in this respect.

A better octree raycast would maintain:

$$
t_{\text{closest}}
$$

and prune:

$$
t_{\text{nodeNear}}>
t_{\text{closest}}.
$$

So the underlying acceleration principle is shared across hierarchy types.

---

**Quadtree representation**

A quadtree is the 2D equivalent of an octree.

Each node splits into four quadrants:

```text
+---------+---------+
|         |         |
|   NW    |   NE    |
|         |         |
+---------+---------+
|         |         |
|   SW    |   SE    |
|         |         |
+---------+---------+
```

A canonical node:

```cpp
struct QuadNode
{
    Rect bounds;

    std::vector<Item> items;

    std::array<
        std::unique_ptr<QuadNode>,
        4
    > children;

    uint32_t depth;
};
```

Its center:

$$
C=
\frac{Min+Max}{2}
$$

generates two splitting lines:

$$
x=C_x,
\qquad
y=C_y.
$$

This gives four children.

---

**Important Kairo status: `Quadtree.cppm` is currently not a quadtree**

Your current file defines:

```cpp
class Quadtree
{
    SpatialRect m_WorldBounds;
    std::vector<QuadtreeItem> m_Items;
};
```

Insertion:

```cpp
m_Items.push_back(...)
```

and query:

```cpp
for (const QuadtreeItem& item :
     m_Items)
{
    if (item.Bounds.Intersects(query))
        result.push_back(item.ID);
}
```

There are:

- no nodes,
- no subdivision,
- no four children,
- no depth,
- no hierarchical pruning.

So the current class is effectively:

$$
\boxed{
\text{flat 2D spatial collection with linear query}
}
$$

with API shape resembling a future quadtree.

It is correct for the test cases, but complexity is:

$$
O(N)
$$

per query.

Part 1362–1363 therefore remains a genuine implementation gap.

---

**Quadtree insertion and subdivision**

Canonical insertion is almost identical to octree insertion.

```cpp
void insert(
    QuadNode& node,
    Item item)
{
    // If children exist and one completely
    // contains the item, descend.
    if (node.hasChildren())
    {
        if (QuadNode* child =
                containingChild(
                    node,
                    item.bounds))
        {
            insert(*child, item);
            return;
        }
    }

    node.items.push_back(item);

    // Split only after capacity is exceeded
    // and maximum depth is not reached.
    if (node.items.size() >
            MaxItemsPerNode &&
        node.depth < MaxDepth)
    {
        if (!node.hasChildren())
            subdivide(node);

        redistributeItems(node);
    }
}
```

A quadtree is particularly useful for worlds where Y/elevation is not important to the broadphase:

- 2D games,
- map editors,
- top-down strategy games,
- terrain tiles,
- geographic data,
- UI/canvas partitioning.

Using a 3D octree for essentially 2D content wastes subdivisions.

---

**KD-tree splitting planes**

A KD-tree recursively partitions $k$-dimensional space using axis-aligned hyperplanes.

In 3D, node split is:

$$
x=s
$$

or:

$$
y=s
$$

or:

$$
z=s.
$$

Example:

```text
           x = 5
             |
      LEFT   |   RIGHT
             |
```

A point goes left if:

$$
p_x<5
$$

and right otherwise, depending on tie convention.

Unlike a BVH, a KD-tree node fundamentally defines a **partition plane**, not merely a bounding group.

Your KD-tree stores:

```cpp
struct Node final
{
    SpatialIndex PointIndex;
    SpatialIndex Left;
    SpatialIndex Right;
    std::uint32_t Axis;
};
```

Each node stores one point and its splitting axis.

The splitting coordinate is simply that point's coordinate on the axis.

---

**Median KD-tree construction**

Your build chooses:

$$
axis=depth\bmod3.
$$

So:

```text
depth 0 -> X
depth 1 -> Y
depth 2 -> Z
depth 3 -> X
...
```

Then it finds the median point along that axis using:

```cpp
std::nth_element(...)
```

If range size is $N$, choose:

$$
m=\left\lfloor\frac N2\right\rfloor.
$$

The median becomes current node.

Left subrange recursively forms left child.

Right subrange forms right child.

Because median splitting keeps point counts approximately balanced:

$$
depth\approx O(\log N)
$$

for ordinary inputs.

Using `nth_element` avoids fully sorting every node range.

Expected work per level is linear in the total number of points across that level, giving an expected build on the order of:

$$
O(N\log N)
$$

for this straightforward recursive construction.

---

**Axis selection alternatives**

Cycling X/Y/Z is cheap and simple.

But suppose all points lie along X:

```text
------------------------------ points -------------------->
```

Y and Z splits convey almost no spatial information.

A stronger builder can choose:

$$
axis=
\arg\max(
\text{point spread}
).
$$

That is analogous to longest-centroid-axis BVH construction.

Or use variance:

$$
axis=
\arg\max
\operatorname{Var}(p_a).
$$

For point clouds from reconstruction, PCA-like axis selection can go even further, although then the tree stops being a classical axis-aligned KD-tree.

Your current Kairo implementation deliberately uses the simpler cyclic strategy.

---

**KD-tree nearest-neighbor search**

This is where KD-trees become beautiful.

Suppose query point is $Q$, and current node stores point $P$ with splitting axis $a$.

Calculate:

$$
d^2=\|Q-P\|^2.
$$

If:

$$
d^2<d^2_{\text{best}},
$$

update best.

Now calculate distance from query to splitting plane:

$$
\delta=Q_a-P_a.
$$

Visit the near child first.

After returning, ask:

> Could the far side contain a point closer than current best?

The minimum possible distance to any point on the far side is at least:

$$
|\delta|.
$$

So if:

$$
\delta^2>d^2_{\text{best}},
$$

the far side cannot improve the answer.

Prune it.

Thus:

$$
\boxed{
\delta^2
\le
d^2_{\text{best}}
\Rightarrow
\text{far side might matter}.
}
$$

Your `SearchNearest()` implements exactly this.

---

**Manual KD nearest-neighbor trace**

Suppose node splits X at:

$$
x=5.
$$

Query:

$$
Q=(3,2).
$$

Current best distance:

$$
d_{\text{best}}=1.5.
$$

Distance to split plane:

$$
|3-5|=2.
$$

Since:

$$
2>1.5,
$$

no point across the X=5 plane can be closer.

Far branch is eliminated.

Graphically:

```text
                 split x=5

 query Q *
       |<-- 2 -->|

-----------------|-----------------------
                 |
                 |
       near      |       far
                 |
```

The closest sphere around Q has radius 1.5.

It does not cross the splitting plane.

Therefore the far half-space is irrelevant.

This is exactly analogous to BVH closest-hit pruning:

$$
\text{lower bound}>
\text{best known answer}
\Rightarrow
\text{prune}.
$$

---

**K-nearest neighbors**

For $k>1$, keep the best $k$ points.

A max-heap is ideal.

Why max-heap?

Because among the current accepted points, we repeatedly need the **worst** one.

Let:

$$
d^2_{\max}
$$

be distance of heap top.

If new point has:

$$
d^2<d^2_{\max},
$$

remove worst and insert new point.

For pruning, if heap already contains $k$ points:

$$
\delta^2>
d^2_{\max}
$$

means far branch cannot contain a point better than any member required to replace the current set.

Your `KNearest()` implementation does precisely this.

---

**KD-tree radius search**

Given radius $r$, accept points satisfying:

$$
\|Q-P\|^2\le r^2.
$$

The far side needs exploring only if:

$$
\delta^2\le r^2.
$$

This is your current `RadiusSearch()` logic.

Using squared distances avoids repeated square roots:

$$
d^2\le r^2
$$

is equivalent to:

$$
d\le r
$$

for nonnegative distances.

---

**KD-tree range queries**

The curriculum's range-query topic extends beyond the current Kairo API.

Suppose query AABB is:

$$
Q=[Q_{\min},Q_{\max}].
$$

At node point $P$ with splitting axis $a$:

Accept point if:

$$
Q_{\min}\le P\le Q_{\max}
$$

componentwise.

For child traversal:

If:

$$
Q_{\min,a}\le P_a,
$$

the query extends into the left half-space.

If:

$$
Q_{\max,a}\ge P_a,
$$

it extends into right.

Canonical code:

```cpp
void rangeSearch(
    NodeIndex nodeIndex,
    const AABB& query,
    std::vector<PointID>& output)
{
    if (nodeIndex == Invalid)
        return;

    const Node& node =
        nodes[nodeIndex];

    const Point& p =
        points[node.pointIndex];

    if (query.contains(p.position))
        output.push_back(p.id);

    const float split =
        p.position[node.axis];

    if (query.min[node.axis] <= split)
    {
        rangeSearch(
            node.left,
            query,
            output);
    }

    if (query.max[node.axis] >= split)
    {
        rangeSearch(
            node.right,
            query,
            output);
    }
}
```

Your current KD-tree exposes radius and nearest queries, but not an AABB range query.

That is a small, natural addition.

---

**KD-tree versus BVH**

This is worth understanding deeply rather than memorizing "KD good for points, BVH good for objects."

### KD-tree

The tree partitions **space**:

$$
\text{left half-space}
\cup
\text{right half-space}.
$$

A point belongs naturally to one side.

Excellent for:

- nearest neighbors,
- radius search,
- point clouds,
- multidimensional records.

Extended geometry complicates things because a triangle can cross a split plane.

You must either duplicate it, split it, or store it at a higher node.

### BVH

The tree partitions **primitive references into groups**.

Child bounds may overlap.

Extended primitives fit naturally.

Transforms/refits are straightforward.

Excellent for:

- ray tracing,
- collision geometry,
- arbitrary bounded objects.

A useful summary is:

$$
\boxed{
KD:
\text{partition space}
}
$$

versus:

$$
\boxed{
BVH:
\text{partition objects}.
}
$$

Neither universally dominates.

For Maveb-style point neighborhoods, KD-tree can be natural.

For ray tracing reconstructed triangles, BVH is usually much more natural.

---

**BSP trees**

A Binary Space Partitioning tree generalizes KD-trees.

A KD-tree uses axis-aligned split planes.

A BSP tree can use arbitrary planes:

$$
n\cdot x+d=0.
$$

Geometry is classified as:

$$
n\cdot x+d>0
$$

front,

$$
<0
$$

back,

or intersecting the plane.

Intersecting polygons may be split.

Conceptually:

```text
                     Plane P0
                     /      \
                  front      back
                  /            \
              Plane P1        Plane P2
```

Why permit arbitrary planes?

Because architectural geometry often already contains useful partition planes:

- walls,
- floors,
- ceilings.

A BSP can align partitions with scene structure.

Historically this made BSP trees powerful for indoor rendering, constructive solid geometry and visibility.

The price is complicated preprocessing and polygon splitting.

Unlike dynamic AABB trees, BSP trees are generally much more suited to mostly-static geometry.

There is currently no dedicated BSP implementation in `KairoSpatial`.

---

**BSP front-to-back traversal**

A nice property of BSP is viewpoint-relative ordering.

At split plane:

$$
d=n\cdot C+d_0
$$

for camera position C.

If camera lies on front side:

```text
visit front subtree
then plane geometry
then back subtree
```

for front-to-back.

Reverse for back-to-front.

This was historically useful for painter-style visibility and transparency ordering.

Modern depth buffers reduce the need for BSP purely for polygon sorting, but BSP/cell decomposition remains conceptually important for portal/PVS systems.

---

**Bounding-sphere hierarchies**

A bounding-sphere hierarchy is like a BVH but uses spheres.

Node:

$$
S=(C,r).
$$

Parent sphere contains all descendant geometry.

Sphere-ray, sphere-sphere and frustum-sphere tests are cheap.

Spheres have one enormous advantage under rigid rotation:

$$
\text{rotation does not change a sphere}.
$$

If an object rotates, its local bounding sphere remains exactly the same radius.

A world-space AABB often changes with rotation.

This can make sphere hierarchies useful for highly rotating objects.

But spheres fit elongated geometry badly.

Imagine a sword:

```text
--------------------------------
```

Its bounding sphere:

```text
             ___________
          .-'           '-.
        .'                 '.
       / ------------------- \
       \                     /
        '.                 .'
          '-.___________.-'
```

Most of the sphere contains empty space.

That causes false positives.

An AABB/OBB may fit it much more tightly.

Thus:

$$
\text{spheres}
=
\text{cheap + rotation invariant + loose fit}
$$

while:

$$
\text{boxes}
=
\text{tighter fit + slightly more complex}.
$$

A sphere hierarchy therefore represents a different point in the fit-vs-test-cost tradeoff.

---

**Constructing a sphere parent**

For two child spheres:

$$
S_1=(C_1,r_1),
\qquad
S_2=(C_2,r_2),
$$

let:

$$
d=\|C_2-C_1\|.
$$

If:

$$
r_1\ge d+r_2,
$$

sphere 1 already contains sphere 2.

Return sphere 1.

Similarly for sphere 2.

Otherwise minimal enclosing sphere along the center line has radius:

$$
\boxed{
r=
\frac{d+r_1+r_2}{2}
}
$$

and its center lies from $C_1$ toward $C_2$ by:

$$
r-r_1.
$$

Let:

$$
u=
\frac{C_2-C_1}{d}.
$$

Then:

$$
\boxed{
C=C_1+u(r-r_1).
}
$$

This gives a simple pairwise merge operation.

Repeated merging is not guaranteed to produce globally minimal enclosing spheres for many primitives, but it is useful for hierarchical construction.

---

**Scene graphs versus spatial indices**

This is one of the most important architectural distinctions in the entire graphics curriculum.

A **scene graph** answers:

> What is related to what semantically or transform-wise?

A **spatial index** answers:

> What is near/intersecting/visible in this region?

Suppose:

```text
Car
├── chassis
├── leftFrontWheel
├── rightFrontWheel
├── leftRearWheel
└── rightRearWheel
```

This is a natural scene graph.

A wheel transform is relative to the car.

But now suppose the car is parked beside a tree.

Spatially:

```text
wheel
tree branch
lamp post
another car
```

may be closer than other nodes in the car hierarchy.

Scene-graph parenting does not imply spatial adjacency.

Therefore:

$$
\boxed{
\text{transform hierarchy}
\neq
\text{spatial hierarchy}.
}
$$

You often need both.

---

**Why using the scene graph as the only culling hierarchy can fail**

Imagine root:

```text
World
├── Enemies
│   ├── enemy on west side
│   ├── enemy on east side
│   └── enemy underground
├── Buildings
│   ├── building west
│   └── building east
└── Effects
```

`Enemies` is semantically meaningful.

Its world-space bound could encompass almost the entire map.

Frustum testing that parent gives little pruning.

A BVH might instead group:

```text
west-side enemy
west building
west props
```

together because they are spatially close.

That may have no semantic relationship at all.

Hence engines typically separate:

```text
Entity/scene hierarchy
          |
          +---- transforms
          +---- ownership
          +---- activation
          +---- logical grouping

Spatial structures
          |
          +---- culling
          +---- collision
          +---- raycasts
          +---- neighborhoods
```

Objects register their world bounds with one or more spatial systems.

---

**Portal and cell visibility**

Some worlds have strong architectural structure.

Think of:

```text
Room A ---- doorway ---- Room B ---- doorway ---- Room C
```

Rooms become **cells**.

Doors/windows/openings become **portals**.

The cell connectivity forms a graph:

```text
[A] -- portal P0 -- [B] -- portal P1 -- [C]
 |
 P2
 |
[D]
```

If the camera is in A, visibility into B can only pass through P0.

Instead of rendering the entire world, recursively traverse visible portals.

---

**Portal traversal with clipped frusta**

Start with camera frustum $F_0$.

For portal P from current cell:

$$
F_1=
F_0\cap
\text{portal visibility cone}.
$$

Traverse neighboring cell using $F_1$.

For its portal Q:

$$
F_2=
F_1\cap
\text{cone through Q}.
$$

Each portal narrows the visible region.

Conceptually:

```text
Camera
   \
    \       portal 1
     \      |   |
      \_____|   |____________
             \             portal 2
              \             | |
               \____________| |
```

A cell may be connected but not visible through the current sequence of openings.

Portal clipping exploits exactly that.

It is particularly powerful in indoor architecture where walls create strong occlusion.

---

**Portals are a graph, not necessarily a tree**

A room may connect back to a previously visited room through another corridor.

Therefore portal traversal needs visited-state logic or recursion guards.

You may need state based not only on:

```text
visited cell
```

but on:

```text
cell + current visibility region
```

because the same cell could be visible through two portals with different clipped frusta.

This is why portal visibility becomes more involved than ordinary graph traversal.

---

**Dynamic objects in portal systems**

Static walls and portals fit naturally.

Dynamic objects still need handling.

A character moving between rooms can be associated with current cell.

A large object crossing a doorway may belong to multiple cells.

A moving door can change portal openness.

This is where portal systems and generic spatial indices complement one another.

For example:

```text
PVS/portals
     ↓
which cells might matter?
     ↓
dynamic tree inside those cells
     ↓
which moving objects matter?
```

Combining structures is often better than forcing one structure to solve everything.

---

**Potentially Visible Sets**

A portal system can determine visibility at runtime.

A **Potentially Visible Set**, or PVS, pushes some work offline.

For each cell $C_i$, precompute:

$$
PVS(C_i)
=
\{
C_j
\mid
C_j
\text{ might be visible from }C_i
\}.
$$

The keyword is **might**.

A correct PVS is conservative:

$$
\text{actually visible}
\Rightarrow
\text{member of PVS}.
$$

It is allowed to include cells that are not actually visible from the current camera position.

It must not omit a cell that can be visible.

Classical cell/PVS systems precompute a conservative superset for each view region so runtime rendering can skip regions known to be impossible to see; portal-based PVS is especially suited to static indoor environments and trades preprocessing/storage for cheap runtime rejection. [TU/e Pure](https://pure.tue.nl/ws/files/46823201/556098-1.pdf?utm_source=chatgpt.com)

---

**PVS as bitsets**

Suppose there are 8 cells.

For cell A:

```text
cell:  A B C D E F G H
PVS :  1 1 0 1 0 0 0 0
```

This can be stored as:

```text
11010000₂
```

Then runtime:

```cpp
if (!pvs[currentCell].test(objectCell))
    rejectObject();
```

A thousand cells require:

$$
1000\text{ bits}
=
125\text{ bytes}
$$

per uncompressed PVS row.

For 1000 cells:

$$
125\,000\text{ bytes}
$$

about 122 KiB.

For 100,000 cells:

$$
\frac{10^{10}}8
\approx1.25\text{ GiB}
$$

if storing a complete dense matrix.

Thus compression and hierarchy become important at large scale.

Classical Quake-style PVS implementations famously store cluster visibility as bit vectors and exploit the fact that many rows contain long runs of invisible clusters. [VR & AR Wiki](https://vrarwiki.com/wiki/Potentially_visible_set?utm_source=chatgpt.com)

---

**PVS is not exact visibility**

Suppose cell B is potentially visible from A through some viewpoint.

Then B must remain in:

$$
PVS(A)
$$

even if from the player's current location inside A a pillar hides B completely.

So runtime can be:

```text
PVS rejection
      |
      v
portal/frustum culling
      |
      v
occlusion culling
      |
      v
render
```

Each stage gets more expensive and more precise.

This is a recurring rendering principle:

$$
\boxed{
\text{cheap conservative rejectors first;}
\quad
\text{expensive precise work later}.
}
$$

---

**Hierarchical Z occlusion**

This final topic moves from world-space spatial indexing to **image-space spatial testing**.

Suppose the camera already has a depth buffer.

At full resolution:

```text
level 0

+---+---+---+---+
| z | z | z | z |
+---+---+---+---+
| z | z | z | z |
+---+---+---+---+
| z | z | z | z |
+---+---+---+---+
| z | z | z | z |
+---+---+---+---+
```

Construct progressively smaller depth levels:

```text
level 1

+-------+-------+
| depth | depth |
+-------+-------+
| depth | depth |
+-------+-------+


level 2

+---------------+
|     depth     |
+---------------+
```

This is a depth pyramid / hierarchical Z representation.

A coarse texel summarizes a larger screen-space region.

Now a large object's projected bounding rectangle can be tested using only a few coarse texels instead of every covered pixel.

That converts occlusion into a hierarchical spatial test.

---

**The conservative depth reduction rule**

We must derive this carefully.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/28 - BVH traversal, refitting, and SIMD|Previous chapter]] · [[Chapter Edition/03 - Game Engine/30 - Depth, projection, and shadow-map precision|Next chapter]]
