# Sparse-set storage

A sparse set answers a common ECS question: how do we keep component values densely packed while entities live in a much larger, sparse identifier space?

Use two index structures:

- dense: a compact sequence of live entities, often parallel to a compact component-value array;
- sparse: indexed by entity index and storing where that entity should appear in dense.

If sparse[e.index] says dense slot d, membership is not established until dense[d] is checked against the complete entity identity. This matters when entity indices are reused with newer generations. An old handle and a new handle may share the same numeric index.

A common representation encodes absence as zero and stores dense_index + 1 in sparse. Then:

contains(e) =
1. e.index is within sparse bounds;
2. sparse[e.index] is nonzero;
3. d = sparse[e.index] - 1 is within dense bounds;
4. dense_entities[d] equals the full entity handle e.

Step 4 prevents a stale generation from inheriting the new occupant’s component by index coincidence.

## Swap-and-pop removal

To erase dense slot d in O(1), move the last dense element into d and pop the last row. But the moved entity’s sparse reverse mapping must be repaired.

If b is removed from [a,b,c], c moves into b’s old dense slot. The operation is therefore not just:

dense[d] = dense.back();
dense.pop_back();

It also requires:

sparse[c.index] = d.

Forgetting that repair produces one of the nastiest ECS bugs: a membership query succeeds but returns the wrong component row later.

## Address stability

Swap-and-pop can invalidate a pointer even when the underlying vector does not allocate. The bytes at a saved address may now belong to another entity. Vector reallocation creates an additional invalidation mode. Therefore sparse-set component addresses should be treated as borrows with a clear mutation boundary.

Stable-address designs exist—paged pools, indirection, tombstones, node storage—but they trade locality, memory, branch cost, or iteration simplicity for stability.

## Complexity is not the whole workload

Sparse sets give expected O(1) membership, insertion and removal and dense O(n) iteration. But real performance depends on:

- size of the sparse index array;
- cache behavior of random entity-index probes;
- component payload size;
- frequency of structural mutations;
- query combinations across several component pools;
- whether iteration follows the smallest pool or performs archetype-style grouping.

So “O(1)” does not settle the architecture decision.

## Executable reconstruction

tools/labs/ecs_reconstruction_lab.cpp implements a generational entity pool and a sparse-set component pool. Its tests remove a middle dense element and verify that the swap-moved entity still resolves correctly. It also checks stale-generation behavior.

Read the connected engine narrative at [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].

Related: [[Archetype ECS]], [[Object identity and generation handles]], [[Borrow invalidation and logical identity]].
