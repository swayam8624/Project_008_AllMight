# Prefix scan and stream compaction

A prefix scan turns local counts into globally consistent offsets. For an exclusive sum scan of input $x$,

$$
y_0=0,
\qquad
y_i=\sum_{j=0}^{i-1}x_j.
$$

If each element says how many output records it will emit, $y_i$ is the first output slot owned by element $i$. This converts a race-prone global append into deterministic non-overlapping regions.

For Boolean compaction, let predicate $b_i\in\{0,1\}$. Scan $b$ to obtain offsets. Every active element with $b_i=1$ writes itself to output[scan[i]]. The final output count is scan[n-1] + b[n-1].

## Parallel structure

The naive parallel scan that repeatedly doubles an offset performs $O(n\log n)$ work. A work-efficient tree scan performs an up-sweep/reduction followed by a down-sweep and uses $O(n)$ work with $O(\log n)$ depth.

On a GPU, a practical large-array scan normally has several levels:

1. scan each workgroup block into local output;
2. record each block total;
3. scan the block totals;
4. add each scanned block offset back into that block’s local results.

Every phase has a dependency. Dispatching all phases without establishing visibility/order is not a valid scan merely because each shader is individually correct.

Shared-memory bank conflicts and subgroup width affect performance, while workgroup barriers only synchronize participating invocations in that workgroup.

## Stream compaction as an allocation proof

Compaction is useful beyond filtering visible objects. It appears in GPU-driven draw generation, BVH construction, particle survival, tiled light lists, Gaussian tile duplication and sparse work queues.

The scan proves that each surviving producer owns a distinct range before any scatter writes occur.

## Evidence boundary

The current repository has CPU-side scan/compaction reasoning in the canonical volumes but does not yet claim a native Vulkan/Metal scan kernel benchmark. The next hardware proof should compare a scalar CPU oracle, a workgroup implementation, multi-workgroup hierarchy, adversarial non-power-of-two sizes, and captured GPU timing.

Primary background: NVIDIA GPU Gems 3, Chapter 39, “Parallel Prefix Sum (Scan) with CUDA.”

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Scan turns local counts into nonoverlapping output regions]].

Related: [[GPU workgroups and synchronization]], [[BVH build traversal and workload selection]].
