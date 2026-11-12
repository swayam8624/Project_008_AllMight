# BVH build, traversal, and workload selection

A bounding-volume hierarchy is a tree whose nodes bound groups of primitives. Its purpose is not to make intersection magically faster; it is to let a query prove that large groups cannot matter.

For an overlap query Q, traversal follows a simple implication:

if Q does not overlap node.bounds, then Q cannot overlap any primitive below that node.

That pruning claim is only correct when every child primitive is actually contained by every ancestor bound. Tree construction therefore owns an invariant: each internal bound is the union of its children, and each leaf bound contains the primitives assigned to the leaf.

## Build choices

A median split sorts or partitions primitive centroids and divides them into similarly sized groups. It is simple and usually avoids catastrophically unbalanced trees, but equal primitive counts do not imply equal traversal cost.

Surface-area heuristic construction estimates expected query/ray work. In the common ray-tracing interpretation, a split cost has the form

$$
C = C_T +
\frac{A_L}{A_P}N_L C_I +
\frac{A_R}{A_P}N_R C_I,
$$

where $A_P$ is parent surface area, $A_L,A_R$ are child areas, $N_L,N_R$ are primitive counts, $C_T$ is traversal cost and $C_I$ is primitive-intersection cost. Binned SAH approximates candidate split positions efficiently.

LBVH chooses hierarchy structure from Morton ordering. It can build quickly and map well to parallel hardware, but tree quality can be worse than a careful SAH build for some scenes. Which strategy wins depends on how often the scene rebuilds and how expensive later queries are.

## Traversal representation

A production BVH must choose node layout as deliberately as split policy. Questions include:

- binary versus wider nodes;
- array indices versus pointers;
- parent links or stackless encodings;
- leaf primitive ranges;
- AoS versus SoA bounds;
- quantized bounds;
- traversal stack depth and overflow behavior;
- child ordering by query direction or distance.

For GPU traversal, divergence and memory coalescing can dominate the nominal tree complexity.

## Dynamic scenes

Refitting recomputes ancestor bounds without changing topology. It is cheap when objects move modestly, but tree quality can decay. Rebuilding restores quality at higher cost. A useful dynamic policy measures a quality proxy and rebuilds when continued bad traversal is more expensive than reconstruction.

Dynamic AABB trees instead maintain a mutable hierarchy and often enlarge leaf bounds by a motion margin. The margin trades fewer reinserts for more false candidates.

## Executable reference

tools/labs/spatial_index_lab.cpp builds a median-split BVH over deterministic random AABBs and compares 250 overlap queries against a brute-force oracle. Any missed or extra primitive is a test failure. The same laboratory also contains A* graph search with relaxation, reinsertion and unreachable-goal handling.

The witness does not claim SAH quality, LBVH construction, dynamic updates, SIMD traversal, ray packet behavior, navmesh construction, or KairoSpatial parity. Those remain separate evidence layers.

Related: [[AABB overlap and containment]], [[Prefix scan and stream compaction]], [[Field reordering and AoS versus SoA]].

## SAH, Morton and refit witness

`tools/labs/bvh_sah_refit_lab.cpp` advances the construction side beyond the earlier median-split query oracle. It computes prefix/suffix bounds for candidate partitions and minimizes the surface-area-weighted primitive cost, verifies that two separated three-object clusters split between the clusters, tests 3D Morton bit interleaving on a controlled monotone coordinate sweep, and refits a parent bound after one child moves.

This is enough to make the equations executable, but not enough to call the builder a production binned SAH or LBVH. A complete next layer needs fixed-bin accumulation, stable primitive partitioning, LBVH range/split construction from Morton prefixes, dynamic-tree rotations or reinsertion, and measured build-versus-query trade-offs on real workloads.

