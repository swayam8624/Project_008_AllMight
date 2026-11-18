# 16 — GPU Memory, Divergence and Execution Order

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/17 - Build, Test and Reproduce the Program|Next chapter]]

GPU performance depends on memory traffic, divergent execution and synchronization as well as arithmetic. The chapter links these hardware behaviors to subgroup operations and ordering constraints.

**By the end:** Reason about memory access, divergence and synchronization in GPU programs.

**Recall question:** How can divergent control flow reduce subgroup efficiency?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 37363–41670; source-span SHA256 8379b807643454273f6eebd337220d877bb76c633619f4e24e75c526131a5b67. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### MEMORY, DIVERGENCE, SUBGROUPS, ATOMICS & ORDERING

**Device/global memory**

Device memory is the large memory space holding things such as:

$$
\text{vertex buffers},
\text{textures},
\text{material arrays},
\text{Gaussian records},
\text{BVHs},
\text{scan buffers}.
$$

It is huge compared with registers/threadgroup storage.

It is also much higher latency.

GPU programming therefore revolves around:

$$
\boxed{
\text{use device memory efficiently and reuse data closer to computation whenever possible}.
}
$$

---

**Unified memory does not remove GPU memory behavior**

On Apple silicon, CPU and GPU can share physical system memory.

That eliminates some discrete-GPU PCIe transfer problems.

It does **not** mean:

> GPU memory access patterns no longer matter.

The GPU still has:

- caches,
- transaction widths,
- load/store units,
- coalescing behavior,
- cache-line effects.

A random gather remains far more expensive than a contiguous stream merely because CPU and GPU share DRAM.

---

**GPU caches**

A simplified hierarchy might be conceptualized as:

```text id="q2nq6h"
registers
    ↓
threadgroup/shared memory
    ↓
local/on-chip cache
    ↓
larger shared cache
    ↓
device DRAM
```

Exact organizations vary heavily between GPUs.

Therefore engine code should optimize for universal principles:

$$
\text{locality},
$$

$$
\text{reuse},
$$

$$
\text{coalesced transactions},
$$

rather than assuming one vendor's exact cache dimensions.

---

**Constant / uniform memory**

Some shader data is identical across many lanes:

```text id="xfh30n"
camera matrix
frame constants
light count
viewport size
```

GPU architectures and APIs provide read-only/uniform access mechanisms optimized for this use case.

If every lane requests the same address, the hardware may broadcast/cached-deliver efficiently.

Contrast that with:

```cpp id="7e3tbt"
constants[laneID]
```

where each lane accesses a different location.

The first is genuinely uniform.

The second merely happens to reside in a buffer named `constant`.

---

**Texture cache**

Textures are usually sampled with strong 2D spatial locality.

Neighboring fragment invocations tend to access neighboring texels.

Texture hardware is therefore optimized around:

- spatial locality,
- dedicated caching,
- address modes,
- filtering.

Texture cache behavior differs from a plain linear-buffer cache.

This is why an image should often be represented as an actual texture when you need image-sampling semantics rather than manually implementing filtered random buffer reads.

---

**Memory coalescing**

Suppose subgroup lanes read:

$$
A[i+\text{lane}].
$$

Addresses are contiguous:

```text id="4d98r5"
lane 0 → A[100]
lane 1 → A[101]
lane 2 → A[102]
...
```

The memory system may service many lane requests with a small number of transactions.

This is coalescing.

Conceptually:

$$
\boxed{
\text{many lane accesses}
\rightarrow
\text{few wide memory transactions}.
}
$$

This is ideal.

---

**KairoGPU vector multiply has excellent access locality**

Thread:

$$
i
$$

loads:

$$
lhs[i],
\quad
rhs[i]
$$

and writes:

$$
out[i].
$$

Adjacent threads use adjacent values.

So memory traffic is approximately:

```text id="r4zgcd"
lane 0 → lhs[0]
lane 1 → lhs[1]
...
```

This is almost the ideal streaming GPU workload.

It is a useful baseline against which irregular workloads can be compared.

---

**Alignment**

Suppose a memory transaction naturally covers some aligned region.

A contiguous access beginning exactly at the region boundary can require fewer transactions than one straddling boundaries.

For example, conceptually:

```text id="yfpnbu"
aligned:
|----------transaction----------|
[ lane data .................... ]

misaligned:
|----------T0----------|
               [ lane data ............ ]
                    |----------T1----------|
```

The exact transaction width depends on hardware.

The universal principle is:

$$
\boxed{
\text{naturally aligned contiguous data makes coalescing easier}.
}
$$

---

**Strided access**

Consider:

$$
A[\text{lane}\times32].
$$

Adjacent lanes now read values separated by:

$$
32
$$

elements.

Instead of touching one compact cache region, a subgroup may touch dozens of cache lines.

Bandwidth efficiency collapses.

This is why a matrix should be tiled or transposed appropriately rather than having every thread walk a poorly oriented column directly from global memory.

---

**AoS versus SoA**

Suppose:

```cpp id="4azajn"
struct Particle
{
    Vec3 Position;
    Vec3 Velocity;
    float Mass;
    float Lifetime;
};
```

Array of structures:

```text id="9euomv"
P0 V0 M0 L0
P1 V1 M1 L1
...
```

If a kernel only needs positions, it loads irrelevant fields or jumps with a large structure stride.

Structure of arrays:

```text id="atbpdt"
Positions[]
Velocities[]
Masses[]
Lifetimes[]
```

lets adjacent lanes stream only the required field.

SoA often improves GPU bandwidth.

But if every invocation uses the full record, AoS may be perfectly reasonable.

Data layout must match access pattern.

---

**Gather-like access**

Suppose:

```cpp id="yx7r08"
value =
    objects[
        indices[id]
    ];
```

`indices[id]` may be arbitrary.

Adjacent lanes might fetch completely unrelated addresses.

That is a gather.

Examples include:

- BVH traversal,
- indirect particle lists,
- mesh adjacency,
- sorted Gaussian references.

These patterns are often cache-latency dominated.

GPU algorithms therefore frequently **sort or bin data first** to make later gathers more coherent.

This is exactly what your Gaussian pipeline is doing.

---

**Maveb Gaussian sorting is a locality transformation**

The Gaussian pipeline generates entries keyed by:

$$
(\text{depth},\text{tile ID})
$$

and sorts them so entries for one tile become contiguous.

Then the composite kernel can process one pixel/tile range without globally searching all Gaussians.

So sorting is not merely:

> I need things alphabetically ordered.

It transforms an irregular gather problem into a locally traversable range problem.

That is an important GPU-design pattern.

---

**Branch divergence**

Suppose lanes encounter:

```cpp id="86rlbx"
if (material == Glass)
    TraceGlass();
else
    TraceDiffuse();
```

If every lane selects Glass, subgroup runs one path efficiently.

If half choose Glass and half Diffuse, hardware must execute both instruction paths with different active masks.

Conceptually:

```text id="c7d0fn"
execute Glass:
    lanes 0,2,5 active
    others disabled

execute Diffuse:
    remaining lanes active
```

That is divergence.

---

**Divergence cost**

A simple mental model is:

$$
T_{\text{divergent}}
\approx
T_A+T_B
$$

rather than:

$$
\max(T_A,T_B).
$$

Real hardware control flow is more sophisticated, but this captures the danger.

If each lane enters a different deep path, SIMD/SIMT utilization can collapse.

This is exactly why wavefront path tracing regrouped rays by material/event in Pass 8.

---

**Uniform branching is cheap**

This:

```cpp id="3mrjt7"
if (frameDebugMode == 3)
```

is often uniform across an entire dispatch.

Every lane chooses the same branch.

Therefore:

$$
\boxed{
\text{branch instruction}
\neq
\text{divergence}.
}
$$

The expensive thing is differing control flow across lanes of one subgroup.

---

**Last-workgroup bounds divergence**

A check like:

```metal id="jynsrg"
if (index >= count)
    return;
```

typically diverges only in the final partially occupied subgroup/workgroup.

For large arrays, this cost is tiny.

Do not contort algorithms to eliminate a single tail bounds check.

---

**Reconvergence**

After divergent branches:

```cpp id="x2i0x6"
if (condition)
    A();
else
    B();

C();
```

lanes eventually reconverge at `C()`.

GPU control-flow hardware/compiler tracks active masks and reconvergence structure.

The programmer mainly needs to understand that divergent branches consume execution time until reconvergence.

---

**Predication**

For a tiny branch such as:

```cpp id="whgcgz"
if (x > 0)
    y = a;
else
    y = b;
```

compiler may execute both simple possibilities and select with a predicate rather than actually branching.

Conceptually:

$$
y=
condition?a:b.
$$

Predication avoids branch-control overhead but executes some unnecessary instructions.

It is excellent for short branches.

Terrible for giant expensive branches.

---

**Subgroup vote operations**

Subgroup operations let lanes cooperate without going through threadgroup memory.

Examples conceptually:

$$
\operatorname{any}(predicate),
$$

$$
\operatorname{all}(predicate),
$$

$$
\operatorname{ballot}(predicate).
$$

Suppose a ray subgroup wants to know:

> Does any ray remain active?

A subgroup-wide `any` can answer quickly.

Ballot returns an active-bit mask useful for compaction/ranking.

---

**Subgroup shuffle**

Shuffle allows one lane to read a value held in another lane's register.

Conceptually:

```cpp id="kjzl1q"
x =
    shuffle(value, sourceLane);
```

No shared memory array.

No ordinary load/store.

This makes reductions and scans inside a subgroup extremely efficient.

For example:

$$
x_i
\leftarrow
x_i+x_{i+16}
$$

then offsets 8,4,2,1 can reduce a subgroup.

---

**Cross-API subgroup terminology**

Your codebase spans several worlds.

`KairoSIMD` exposes explicit CPU vectors.

Metal exposes `simdgroup` intrinsics.

Vulkax's backend capability model already contains:

```cpp id="1hu377"
Feature::Subgroups
```

alongside:

```cpp id="4aem6v"
Compute
StorageBuffers
StorageImages
Atomics
Float16
```

That is the correct engine-level approach:

$$
\boxed{
\text{treat subgroup capability as a backend capability, not as an assumption}.
}
$$

---

**Maveb uses real subgroup reductions**

Its radix histogram performs:

```metal id="z1r98r"
const uint matches =
    digit == bucket ? 1 : 0;

const uint total =
    simd_sum(matches);

if (lane == 0)
    simdCounts[
        simdGroup][bucket] =
        total;
```

This is beautiful.

Instead of every thread performing a global atomic increment:

```text id="nh6bpq"
atomic histogram[bucket]++
```

each subgroup first aggregates:

$$
\boxed{
\text{one count per bucket per subgroup}.
}
$$

That drastically reduces contention.

This single kernel demonstrates M128 parts 2128, 2130 and 2133 in real production-like code.

---

**Atomic operations**

An atomic read-modify-write guarantees that competing threads do not lose updates.

For example:

```cpp id="xpapkm"
atomic_fetch_add(counter, 1);
```

Two threads cannot both read 5 and both write 6.

Atomic result becomes:

$$
7.
$$

Without atomicity:

$$
5\rightarrow6
$$

could occur twice with one increment lost.

---

**Atomics are not free serialization breakers**

Suppose:

$$
1,000,000
$$

threads all increment one counter.

The operation is correct.

But all accesses contend on the same memory location.

At some level they must serialize.

Thus:

$$
\boxed{
\text{atomics solve correctness, not scalability}.
}
$$

The algorithm should reduce contention.

---

**Atomic contention**

Compare:

```cpp id="3nct9g"
atomicAdd(globalCounter, 1);
```

executed by every thread,

against:

```text id="oah6qe"
each subgroup counts locally
one lane adds subgroup count
```

For subgroup size $S$:

atomic count falls roughly from:

$$
N
$$

to:

$$
N/S.
$$

Aggregate at workgroup level and it may drop further to:

$$
N/T_{\text{group}}.
$$

This is a fundamental pattern:

$$
\boxed{
\text{aggregate locally, atomically merge globally}.
}
$$

---

**Maveb does this explicitly**

Its parallel radix histogram first computes:

```metal id="l7udrw"
simd_sum(matches)
```

then stores subgroup-level counts into:

```metal id="af4u60"
threadgroup uint
simdCounts[32][16];
```

After a barrier, only a small set of threads combines subgroup counts into one group histogram.

That avoids a global atomic for every key.

Exactly what M128 asks you to understand.

---

**Atomic memory ordering**

Atomicity and memory ordering are separate.

Maveb often uses:

```metal id="7dyn47"
memory_order_relaxed
```

because it needs the counter update itself to be atomic but does not necessarily require that operation to impose ordering on unrelated memory writes.

General rule:

$$
\boxed{
\text{atomic operation}
\neq
\text{global synchronization barrier}.
}
$$

If another memory dependency exists, it must be expressed separately.

---

**Workgroup barrier**

Maveb's scan uses:

```metal id="jow2md"
threadgroup_barrier(
    mem_flags::mem_threadgroup);
```

This provides a synchronization point for invocations of one threadgroup and visibility of relevant threadgroup-memory writes.

It does **not** synchronize another workgroup somewhere else on the GPU.

That is perhaps the most important synchronization boundary in compute programming.

---

**Barrier convergence requirement**

Suppose:

```cpp id="sp9bsz"
if (localIndex < 10)
{
    threadgroup_barrier(...);
}
```

but the workgroup contains 256 invocations.

Only 10 reach the barrier.

The others do not.

That is fundamentally invalid synchronization logic.

Workgroup barriers generally need to be reached coherently by all participating workgroup threads.

This is why scan code places barriers outside conditional bodies even when only some lanes update data.

Maveb does exactly this correctly.

---

**No device-wide barrier inside a normal workgroup kernel**

Suppose group 0 computes a partial result and group 1 needs it.

You cannot generally do:

```cpp id="o1fakh"
device_barrier_for_every_workgroup();
```

mid-dispatch.

Workgroups can execute in any order and may not all be resident simultaneously.

Trying to implement a global spin barrier can deadlock:

resident groups wait for unscheduled groups that cannot start because the resident groups are occupying all resources.

The correct structure is often:

$$
\boxed{
\text{dispatch A}
\rightarrow
\text{global dependency}
\rightarrow
\text{dispatch B}.
}
$$

---

**Maveb's scan is a real multi-dispatch global synchronization algorithm**

Its pipeline performs conceptually:

```text id="0mnqgj"
dispatch:
    scan each 256-element block

dispatch:
    scan block sums

dispatch:
    add block offsets
```

No workgroup barrier could perform this entire global scan by itself because block 7 cannot safely synchronize with block 2.

The dispatch boundary becomes the algorithmic phase boundary.

On different graphics APIs, the precise execution/memory dependency mechanism differs, but the architecture remains:

$$
\boxed{
\text{global synchronization}
\Rightarrow
\text{kernel/dispatch phase decomposition}.
}
$$

---

### GENERAL COMPUTE & PARALLEL ALGORITHMS

**Compute shaders are general GPU kernels**

A compute shader does not need:

- vertices,
- triangles,
- rasterization,
- framebuffer coverage.

It says:

$$
\boxed{
\text{run this kernel over a logical N-dimensional grid}.
}
$$

That makes GPUs useful for:

- image processing,
- physics,
- simulation,
- sorting,
- scans,
- culling,
- ML,
- Gaussian splatting preprocessing.

KairoGPU's vector operations and matrix multiply are already pure compute workloads.

Maveb's Gaussian preparation is an entire compute pipeline.

---

**Workgroup size is an algorithmic choice**

You cannot choose:

$$
T=256
$$

just because “256 is standard.”

Workgroup size affects:

$$
\text{subgroup packing},
$$

$$
\text{register consumption},
$$

$$
\text{shared-memory consumption},
$$

$$
\text{occupancy},
$$

$$
\text{synchronization cost},
$$

$$
\text{data locality}.
$$

The ideal size depends on the kernel.

---

**Current Kairo/Maveb dispatch policy**

KairoGPU vector kernels choose:

```objc id="ozt45h"
width =
    MIN(
        pipeline.maxTotalThreadsPerThreadgroup,
        256);
```

Maveb's generic `dispatch1D()` similarly caps at:

$$
256.
$$

This guarantees the group size does not exceed a pipeline limit.

However, neither current code path queries:

```text id="p4wrmn"
threadExecutionWidth
```

when choosing the generic group width.

So:

$$
\boxed{
256\text{ is a valid cap, not yet an architecture-tuned choice}.
}
$$

A more tuned Metal implementation would consider both maximum group size and execution-width/subgroup characteristics.

---

**1D decomposition**

For:

```text id="zugmms"
vector length = N
```

map:

$$
\boxed{
\text{thread }i
\rightarrow
\text{element }i.
}
$$

Kairo's vector add/multiply are textbook examples.

Global ID:

$$
i.
$$

Bounds:

$$
i<N.
$$

Memory:

$$
A[i],B[i],C[i].
$$

This has excellent coalescing and essentially no inter-thread communication.

---

**2D decomposition**

For an image:

$$
W\times H
$$

thread:

$$
(x,y)
$$

handles pixel:

$$
(x,y).
$$

Linear index:

$$
\boxed{
i=yW+x.
}
$$

Typical group:

$$
16\times16,
$$

or:

$$
8\times8,
$$

depending on workload and hardware.

KairoGPU's matrix multiplication uses:

$$
16\times16=256
$$

threads per group.

---

**Kairo matrix dispatch geometry**

Host:

```cpp id="dgqe6p"
dispatchThreadgroups(
    {
        ceil(columns / 16),
        ceil(rows / 16),
        1
    },

    {
        16,
        16,
        1
    });
```

Shader:

$$
row=
group_y16+local_y,
$$

$$
column=
group_x16+local_x.
$$

This is exact 2D decomposition.

---

**3D decomposition**

A volume:

$$
W\times H\times D
$$

maps naturally to invocation coordinate:

$$
(x,y,z).
$$

Linear index:

$$
\boxed{
i=
x+
yW+
zWH.
}
$$

This appears in:

- voxel grids,
- TSDF volumes,
- fluid simulation,
- 3D cellular grids,
- volumetric lighting.

A group might be:

$$
8\times8\times4
=
256
$$

threads, for example.

But local neighborhood access pattern should determine the shape.

---

**Over-dispatch bounds**

For arbitrary dimensions:

$$
G_x=
\left\lceil\frac{W}{T_x}\right\rceil,
$$

$$
G_y=
\left\lceil\frac{H}{T_y}\right\rceil.
$$

Then shader:

```cpp id="ivlz2y"
if (x >= width ||
    y >= height)
{
    return;
}
```

Current Maveb Gaussian composite has exactly:

```metal id="zvkv4c"
if (pixel.x >= width ||
    pixel.y >= height)
{
    return;
}
```

This is standard and correct.

---

**Reduction**

A reduction converts:

$$
x_0,x_1,\dots,x_{N-1}
$$

into one value:

$$
\boxed{
R=
x_0\oplus x_1\oplus\dots
}
$$

where $\oplus$ is associative enough for the required application:

$$
+,\min,\max,\dots
$$

Serial:

$$
O(N)
$$

latency.

Parallel tree:

$$
O(\log N)
$$

dependency depth with:

$$
O(N)
$$

total work.

---

**Tree reduction**

For eight values:

```text id="uxlrwn"
a b c d e f g h
\ /   \ /   \ /
 A     B     C     D
   \ /         \ /
    E           F
         \ /
          R
```

At each step active thread count halves.

Mathematically:

$$
s^{(0)}_i=x_i
$$

then:

$$
s^{(k+1)}_i
=
s^{(k)}_{2i}
+
s^{(k)}_{2i+1}.
$$

After:

$$
\log_2N
$$

steps:

$$
s^{(\log N)}_0
=
\sum_i x_i.
$$

---

**Threadgroup reduction**

```metal id="zq2h7f"
threadgroup float scratch[256];

scratch[local] =
    value;

threadgroup_barrier(...);

for (uint stride = 128;
     stride > 0;
     stride >>= 1)
{
    if (local < stride)
        scratch[local] +=
            scratch[local + stride];

    threadgroup_barrier(...);
}

if (local == 0)
    blockSums[group] =
        scratch[0];
```

This reduces 256 values into one block sum.

Then another dispatch reduces block sums.

---

**Warp/subgroup-level reduction**

Inside one subgroup, shared memory and barriers can often be avoided.

Conceptually:

```cpp id="ywrrrk"
value +=
    shuffle_down(value, 16);

value +=
    shuffle_down(value, 8);

value +=
    shuffle_down(value, 4);

value +=
    shuffle_down(value, 2);

value +=
    shuffle_down(value, 1);
```

or use a native collective:

```metal id="jheza8"
simd_sum(value)
```

Maveb uses `simd_sum()` in its histogram.

This is exactly Part 2144 realized in code.

---

**Prefix sum / scan**

Given:

$$
x=
[3,1,4,0,2].
$$

Inclusive scan:

$$
[3,4,8,8,10].
$$

Exclusive scan:

$$
\boxed{
[0,3,4,8,8].
}
$$

Exclusive relation:

$$
y_i=
\sum_{j<i}x_j.
$$

Why is this so powerful?

Because $y_i$ tells element $i$ exactly how many preceding contributions exist.

That converts local counts/predicates into globally unique output offsets.

---

**Scan is the allocator of parallel algorithms**

Suppose every object emits:

$$
c_i
$$

output records.

If:

$$
c=[2,0,3,1],
$$

exclusive scan:

$$
o=[0,2,2,5].
$$

Then:

```text id="9j7ww5"
object 0 writes [0,1]
object 1 writes nothing
object 2 writes [2,3,4]
object 3 writes [5]
```

No atomics are required for each output record.

This pattern appears everywhere:

- compaction,
- binning,
- variable output generation,
- mesh processing,
- Gaussian tile expansion.

---

**Maveb Gaussian counts use exactly this pattern**

A projected Gaussian overlaps some number of screen tiles.

Gaussian $i$ computes:

$$
c_i=
\text{number of tile entries it needs}.
$$

The scan produces:

$$
offset_i=
\sum_{j<i}c_j.
$$

Then Gaussian $i$ writes its tile records starting at:

$$
offset_i.
$$

This gives each Gaussian a unique contiguous output region with no global append atomic for every tile.

That is an extremely good real-world scan example.

---

**Blelloch exclusive scan**

The Blelloch algorithm has two phases:

$$
\boxed{
\text{upsweep}
}
$$

and:

$$
\boxed{
\text{downsweep}.
}
$$

For power-of-two $N$.

Upsweep constructs partial sums.

For stride:

$$
1,2,4,\dots,N/2
$$

combine tree children.

At the end root contains total sum.

Then set root:

$$
0.
$$

Downsweep propagates exclusive prefixes toward leaves.

Total work:

$$
O(N).
$$

Parallel depth:

$$
O(\log N).
$$

That makes it **work-efficient** compared with some simpler $O(N\log N)$ scan methods.

---

**Maveb implements Blelloch almost literally**

Current `aetherGaussianScanBlocks`:

```metal id="4ie3j5"
threadgroup uint scratch[256];

scratch[localIndex] =
    valid
    ? counts[index]
    : 0;

threadgroup_barrier(...);
```

Upsweep:

```metal id="5188rr"
for (uint stride = 1;
     stride < 256;
     stride <<= 1)
{
    const uint element =
        (localIndex + 1) *
        stride * 2 - 1;

    if (element < 256)
        scratch[element] +=
            scratch[
                element - stride];

    threadgroup_barrier(...);
}
```

Then:

```metal id="sz0hjf"
if (localIndex == 0)
{
    blockSums[groupIndex] =
        scratch[255];

    scratch[255] = 0;
}
```

Then downsweep:

```metal id="67v40p"
for (uint stride = 128;
     stride > 0;
     stride >>= 1)
{
    ...
}
```

This is not “scan-like.”

It is recognizably a textbook work-efficient Blelloch exclusive scan.

---

**Why zeroing the root makes the scan exclusive**

Before zeroing, root contains:

$$
\sum_i x_i.
$$

For exclusive scan, first output must be:

$$
0.
$$

By setting root to zero and distributing left-prefix values during downsweep, each leaf receives the sum of all values strictly before it.

That is the mathematical purpose of:

```metal id="t9o8ez"
scratch[255] = 0;
```

---

**Hierarchical scan**

One workgroup can only scan a bounded number of elements.

For arbitrary $N$:

First scan each block independently.

Store each block's total.

Scan those block totals.

Then add each block's prefix to every element in that block.

Symbolically:

$$
\boxed{
\text{local scan}
\rightarrow
\text{scan block sums}
\rightarrow
\text{add block offsets}.
}
$$

This is Part 2147.

---

**Maveb's three stages**

Maveb has kernels:

```text id="r92vsu"
aetherGaussianScanBlocks
aetherGaussianScanBlockSums
aetherGaussianAddBlockOffsets
```

This is exactly the hierarchical decomposition.

But there is one scaling caveat.

---

**Current Maveb block-sum scan is serial**

`aetherGaussianScanBlockSums` contains:

```metal id="qa2bts"
if (index != 0)
    return;

for (uint block = 0;
     block < blockCount;
     ++block)
{
    ...
}
```

So one GPU invocation serially scans all block sums.

For moderate Gaussian counts this may be perfectly adequate.

For huge numbers of blocks, it becomes a bottleneck.

A fully scalable scan recursively applies another parallel scan to the block-sums array.

Thus Maveb currently has:

$$
\boxed{
\text{parallel local scan}
+
\text{serial second level}
+
\text{parallel offset add}.
}
$$

A useful optimization target if profiling shows the second stage mattering.

---

**Stream compaction**

Given:

$$
x=[a,b,c,d,e]
$$

and keep predicate:

$$
p=[1,0,1,1,0].
$$

Wanted output:

$$
[a,c,d].
$$

We cannot have every surviving thread perform:

```cpp id="ddkrn5"
output[atomicAdd(counter,1)] =
    value;
```

without contention.

Instead scan predicate.

Exclusive scan:

$$
s=
[0,1,1,2,3].
$$

Then if:

$$
p_i=1,
$$

write:

$$
\boxed{
output[s_i]=x_i.
}
$$

Result:

$$
a\rightarrow0,
$$

$$
c\rightarrow1,
$$

$$
d\rightarrow2.
$$

No collisions.

---

**Predicate–scan–scatter**

This is such an important general GPU pattern that it should be memorized structurally:

$$
\boxed{
\text{predicate}
\rightarrow
\text{exclusive scan}
\rightarrow
\text{scatter}.
}
$$

Predicate determines whether/how many outputs.

Scan allocates unique positions.

Scatter writes.

For variable expansion, replace binary predicate:

$$
0/1
$$

with count:

$$
0,1,2,\dots
$$

which is exactly what Gaussian tile expansion does.

---

**Parallel histogram**

Suppose every item has category:

$$
d_i\in[0,R).
$$

Histogram:

$$
H_b=
|\{i:d_i=b\}|.
$$

Naive GPU implementation:

```cpp id="mdso8e"
atomicAdd(
    histogram[digit],
    1);
```

Correct.

Potentially terrible.

If many items fall in bucket 0:

$$
\text{everyone contends on }H_0.
$$

---

**Histogram contention mitigation**

Better hierarchy:

$$
\text{lane}
\rightarrow
\text{subgroup histogram}
\rightarrow
\text{workgroup histogram}
\rightarrow
\text{global histogram}.
$$

Each level reduces how frequently global memory must be updated.

This is exactly what Maveb's radix histogram does.

It has:

```metal id="ctu20l"
threadgroup uint
simdCounts[32][16];
```

For each radix bucket, each subgroup computes:

```metal id="6uv2yi"
simd_sum(matches)
```

and lane 0 stores one result.

Then a handful of local threads combine subgroup counts.

Very little contention.

---

**Radix sort**

Radix sort sorts integer-like keys by processing fixed-width digit groups.

Suppose key has:

$$
K
$$

bits.

Choose:

$$
b
$$

bits per pass.

Radix:

$$
R=2^b.
$$

Number of passes:

$$
\boxed{
P=
\left\lceil
\frac Kb
\right\rceil.
}
$$

For:

$$
b=4,
$$

radix:

$$
R=16.
$$

A 64-bit key requires:

$$
16
$$

passes.

Exactly what Maveb's parallel radix path does.

---

**Radix digit extraction**

For pass $p$:

$$
shift=bp.
$$

Digit:

$$
\boxed{
d_p(key)
=
(key\gg shift)
\&
(R-1).
}
$$

For four-bit digits:

$$
R-1=15=0xF.
$$

Maveb:

```metal id="ugtp35"
const uint component =
    pass < 8
    ? key.x
    : key.y;

return
    (
        component >>
        ((pass & 7) * 4)
    )
    & 15;
```

That is literally Part 2153.

---

**One radix pass**

A stable radix pass has three conceptual stages:

$$
\boxed{
\text{histogram}
\rightarrow
\text{prefix offsets}
\rightarrow
\text{stable scatter}.
}
$$

Histogram says how many values belong to each digit.

Prefix tells where each bucket begins.

Scatter places each item at:

$$
\text{bucketBase}
+
\text{rankWithinBucket}.
$$

Stability is essential for least-significant-digit radix sorting.

---

**Why stability matters**

Suppose first pass sorts lower digit.

Second pass sorts next digit.

If second pass arbitrarily scrambles values having equal second digit, it destroys ordering established by the lower digit.

Stable sorting preserves earlier passes within equal current digit.

Therefore after all passes:

$$
\boxed{\text{all digits collectively determine correct ordering}.}
$$

---

**Maveb parallel radix histogram**

Each item computes digit:

$$
d.
$$

For every bucket:

```metal id="45mfzb"
matches =
    digit == bucket
    ? 1
    : 0;

total =
    simd_sum(matches);
```

So subgroup produces count of how many lanes belong to each of 16 buckets.

After workgroup consolidation:

$$
H_{g,b}
$$

contains count for:

$$
\text{group }g,
\text{ bucket }b.
$$

---

**Radix offset stage**

Maveb's current `aetherGaussianRadixOffsets` runs one invocation:

```metal id="akbc8n"
if (index != 0)
    return;
```

then computes bucket/group prefix positions.

Conceptually:

$$
offset[g,b]
=
\sum_{b'<b}
\sum_{g'}H[g',b']
+
\sum_{g'<g}H[g',b].
$$

This gives the global starting destination of group $g$'s entries in bucket $b$.

Again, this stage is serial today.

It is correct but eventually becomes a scalability candidate.

---

**Stable parallel scatter**

Within a subgroup Maveb computes:

```metal id="bypw71"
prefix =
    simd_prefix_exclusive_sum(
        matches);
```

If current lane belongs to bucket $b$, `prefix` says:

$$
\text{number of earlier lanes in same subgroup with digit }b.
$$

Then it adds counts from earlier subgroups in the same workgroup:

$$
rank_{\text{local}}
=
rank_{\text{subgroup}}
+
\sum_{s'<s}
count[s',b].
$$

Final destination:

$$
\boxed{
dst=
groupOffset[g,b]
+
rank_{\text{local}}.
}
$$

This is a very elegant practical use of subgroup scan.

---

**Maveb's radix sort is a direct fusion of M128 and M129**

One kernel uses:

$$
\text{SIMT lanes},
$$

$$
\text{subgroup IDs},
$$

$$
\text{subgroup reduction},
$$

$$
\text{subgroup exclusive scan},
$$

$$
\text{threadgroup memory},
$$

$$
\text{threadgroup barrier},
$$

$$
\text{coalesced buffers},
$$

$$
\text{histogram},
$$

$$
\text{stable scatter}.
$$

This is exactly why these curriculum topics belong in one phase.

They are not isolated tricks.

They compose into real algorithms.

---

**Maveb's 64-bit logical sorting key**

`GenerateKeys` creates:

```metal id="e9dtbb"
keys[destination] =
    uint2(
        as_type<uint>(
            gaussianDepth),
        tileID);
```

So:

$$
key.x=\text{depth bits},
$$

$$
key.y=\text{tile ID}.
$$

The parallel radix algorithm processes:

$$
key.x
$$

for passes:

$$
0\dots7
$$

and then:

$$
key.y
$$

for passes:

$$
8\dots15.
$$

Because it is stable least-significant-digit sorting, the final major key is:

$$
\boxed{\text{tile ID}}
$$

while previously established depth order remains inside equal tile IDs.

Therefore final ordering is effectively:

$$
\boxed{
\text{tile-major, depth-ordered within tile}.
}
$$

That is precisely what Gaussian compositing needs.

This is a beautiful architecture.

---

**Why reinterpreting positive float depth as uint works here**

IEEE-754 positive finite floats have bit patterns whose unsigned integer ordering preserves numerical ordering.

So for positive depth:

$$
d_1<d_2
$$

corresponds to:

$$
bits(d_1)<bits(d_2).
$$

That makes:

```metal id="brslup"
as_type<uint>(depth)
```

usable as a sortable depth key.

This trick would not directly work for arbitrary signed floating-point values without a sign-aware key transform.

The assumption that valid projected depth is positive matters.

---

**Key–value sorting**

Sorting geometry usually needs to retain identity.

If:

$$
key_i=\text{sorting key},
$$

and:

$$
value_i=\text{Gaussian index},
$$

then sorting keys alone destroys the mapping.

Therefore every scatter writes both:

```metal id="oxmxax"
outputKeys[destination] =
    inputKeys[index];

outputValues[destination] =
    inputValues[index];
```

This is key-value sorting.

After sort:

$$
\boxed{
(\text{sorted key},\text{original payload identity})
}
$$

remain paired.

Maveb's `keysA/keysB` and `valuesA/valuesB` buffers implement exactly this double-buffered key-value radix architecture.

---

**Ping-pong sorting buffers**

A radix pass cannot generally overwrite its input while arbitrary entries are being scattered.

So use:

$$
A\rightarrow B
$$

on pass 0,

$$
B\rightarrow A
$$

on pass 1,

and alternate.

Maveb:

```cpp id="w6g26n"
even
    ? input = keysA
    : input = keysB;

even
    ? output = keysB
    : output = keysA;
```

This is another standard GPU pattern.

#### First-use completion — a stable radix pass is a permutation, not an in-place edit

Ping-pong storage is not only a pattern for alternating GPU kernels. Here it is a **correctness requirement** of a scatter algorithm. A radix pass examines one digit of each key, counts how many entries belong to each digit value, computes each digit's starting output offset, and distributes records into distinct destinations. If the output aliases the input, an early thread may overwrite a record another thread has not yet read. A barrier at the end of the scatter cannot undo that overwrite. The input snapshot must remain readable until *every* output index has been assigned and its value written.

For a least-significant-digit (LSD) radix sort, stability makes successive passes compose correctly. A pass on bit 0 preserves input order among all zeros and separately among all ones. The following pass on bit 1 must preserve the preceding bit-0 ordering within its own zero/one groups. That is why simply partitioning without a stable scatter does not produce a correctly sorted multi-bit key. For four labeled records in arrival order
$$
[3_A,\ 1_B,\ 2_C,\ 0_D],
$$
the bit-0 pass writes $[2_C,0_D,3_A,1_B]$. The bit-1 pass writes $[0_D,1_B,2_C,3_A]$. If the original array also contained $2_E$ after $2_C$, the final sorted order must still put $2_C$ before $2_E$. The stable property is observable independently of whether the integer keys look sorted.

For an exclusive scan on the binary pass, let $z_i=1$ if key $i$ has a zero in the selected bit, and $0$ otherwise; let $Z=\sum_i z_i$. For a zero-bit item, its destination is $\sum_{j<i}z_j$. For a one-bit item, its destination is $Z+\sum_{j<i}(1-z_j)$. Both formulas are exclusive prefixes, so no two records receive the same destination and the within-group order is preserved. A GPU implementation normally computes block-local histograms and prefixes, scans the block totals, then scatters with the block prefix added to the local rank. The scan of block totals needs its own inter-dispatch or globally coordinated synchronization; a threadgroup barrier only orders threads within one group.

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

// Each record carries an identity, so we can verify stability for equal keys.
struct SortRecord {
    std::uint32_t key;
    std::uint32_t identity;
};

// Two stable one-bit passes sort a two-bit key into ascending order.
template <std::size_t N>
std::array<SortRecord, N> sort_two_bits(std::array<SortRecord, N> input)
{
    std::array<SortRecord, N> scratch{};
    auto* read = &input;
    auto* write = &scratch;

    for (unsigned bit = 0; bit != 2; ++bit) {
        std::size_t zeroCount = 0;
        for (const SortRecord& record : *read)
            zeroCount += ((record.key >> bit) & 1u) == 0u;

        std::size_t zeroOut = 0;
        std::size_t oneOut = zeroCount;
        for (const SortRecord& record : *read) {
            // Exclusive ranks: disjoint destinations, stable in each group.
            const bool one = ((record.key >> bit) & 1u) != 0u;
            (*write)[one ? oneOut++ : zeroOut++] = record;
        }

        // Swap the roles, not the contents of the two allocations.
        std::swap(read, write);
    }
    return *read; // May be either allocation when pass count changes.
}
```

The code intentionally exposes the scan's simple serial reference implementation: counting zeros plus per-group running offsets is equivalent to exclusive prefix ranks in input order. It is **not** pretending to be an efficient GPU parallel scan. The GPU version must provide disjoint outputs at every level, global offsets that are ready before scatter, correct device memory visibility between dispatches, and allocation lifetimes extending through the final consumer. Histogram, scan, scatter and indirect consumption are distinct phases even if a backend fuses some operations.

**Buffer parity is part of the API contract.** After an even number of complete passes, the final records are back in the original allocation; after an odd number, they reside in scratch. Returning the original pointer unconditionally can silently return stale data. If a later renderer pass expects one fixed buffer, either expose the actual final-buffer handle or perform an explicit copy after an odd pass and account for its cost. A frame allocator must not recycle the scratch range merely because its CPU-side pointer was swapped: the GPU may still be reading it. The same two-buffer idea is used for Jacobi-style iterative updates, where each iteration must read the previous state rather than partially updated neighbors, but the *reason* for alternation is the dependency of the particular algorithm, not the mere existence of two resources.

A reproducible regression sorts repeated keys with distinct identities and checks both ascending keys and stable identity order; the [first-use C++ witness](../../tools/labs/first_use_contract_lab.cpp) contains that check. GPU performance must be assessed separately: record histogram throughput, scan cost, scatter bandwidth, allocation pressure, command dependencies, and the amount of synchronization. A serial fallback can be correct without being fast; a parallel path can be fast and still be wrong if one prefix or visibility edge is missing. We do not need another teaching pass to establish these properties.

---

**Parallel versus serial radix fallback in Maveb**

Current Maveb has two paths.

When relevant compute pipelines support at least:

$$
256
$$

threads/group, it runs:

$$
16
$$

parallel 4-bit passes.

Otherwise it uses a one-thread serial fallback with:

$$
8
$$

8-bit passes.

Parallel path:

$$
R=16,
\quad
P=16.
$$

Serial fallback:

$$
R=256,
\quad
P=8.
$$

This makes sense because a huge 256-bin local histogram is acceptable in one serial invocation but much more expensive to parallelize per workgroup.

---

**Bitonic sort**

Bitonic sort is a comparison network rather than digit-based sort.

It creates bitonic sequences—sequences that monotonically increase then decrease—and repeatedly merges them.

A common compare-exchange condition uses:

$$
partner=i\oplus j.
$$

Each stage compares:

$$
a_i
$$

against:

$$
a_{i\oplus j}.
$$

The network topology is deterministic.

Complexity:

$$
\boxed{
O(N\log^2N)
}
$$

comparisons.

---

**Why bitonic sort is still useful on GPUs**

Radix sort is usually preferable for large integer keys.

Bitonic sort remains useful when:

- arrays are small,
- size is bounded,
- data fits in threadgroup memory,
- deterministic comparison structure is desirable,
- arbitrary comparable keys are required.

Its fixed communication pattern maps well to SIMD/SIMT.

For sorting millions of simple integer keys, radix generally wins.

For 32 or 64 values inside one workgroup, bitonic can be beautifully simple.

---

**Parallel algorithm design principle**

There is a recurring transformation:

Serial:

```cpp id="x1rcxl"
for each item:
    if useful:
        output.push_back(item);
```

GPU-naive:

```cpp id="di1gjk"
if useful:
    output[
        atomicAdd(counter,1)
    ] = item;
```

GPU-scalable:

```text id="0aznm5"
predicate/count
    ↓
parallel scan
    ↓
unique offsets
    ↓
scatter
```

This transition appears in:

- compaction,
- particle spawning,
- Gaussian tile entry generation,
- visible-object lists,
- clustered lighting,
- meshlet generation.

Once scan becomes intuitive, a huge amount of GPU algorithm design becomes intuitive.

---

**KairoGPU matrix multiplication also illustrates arithmetic intensity**

Naive matrix multiplication repeatedly reads global memory.

For each output element:

$$
C_{ij}
=
\sum_k A_{ik}B_{kj}.
$$

Naively approximately:

$$
2K
$$

global loads for:

$$
2K
$$

floating operations.

With a $16\times16$ tile, each workgroup loads:

$$
256
$$

A values and:

$$
256
$$

B values for a tile step.

Those values feed:

$$
256\times16
$$

multiply-add pairs.

Global memory bytes are reused across many arithmetic operations.

Thus shared-memory tiling increases:

$$
\boxed{
\text{arithmetic intensity}
=
\frac{
\text{operations}
}{
\text{bytes transferred}
}.
}
$$

That will become a central roofline-model concept in Pass 10/M132.

---

**One major KairoGPU improvement: execution-width-aware group selection**

Current vector kernels use:

$$
\min(
\texttt{maxTotalThreadsPerThreadgroup},
256
).
$$

A more informed Metal setup can consider:

$$
W=
\texttt{threadExecutionWidth}.
$$

Then choose group size:

$$
T=kW
$$

for integer $k$, while satisfying register/threadgroup-memory constraints.

You still benchmark.

But this reduces the chance of dispatching groups that leave an awkward partial SIMD group.

---

**One major Maveb scan improvement: fully parallel second level**

Current pipeline:

$$
N
\rightarrow
N/256
$$

block sums.

Then a single thread scans those block sums.

For:

$$
N=1,000,000,
$$

block count is around:

$$
3907.
$$

A single thread looping over 3907 entries may not dominate the whole frame.

For:

$$
N=100,000,000,
$$

block count:

$$
390625.
$$

Now the serial second level becomes serious.

A recursive hierarchical scan would process block sums in parallel:

$$
390625
\rightarrow
1526
\rightarrow
6
\rightarrow
1.
$$

Then propagate offsets downward.

That is the general scalable version.

---

**One major Maveb radix improvement: parallel global bucket-prefix stage**

Similarly, current `aetherGaussianRadixOffsets` performs the entire group/bucket prefix with one thread.

The parallel histogram and scatter scale well; the prefix stage eventually becomes the serial fraction.

A fully scalable implementation can scan per-bucket group counts in parallel and then compute bucket bases.

This is a classic manifestation of Amdahl's law:

$$
\boxed{
\text{parallelizing 99% does not make the remaining serial 1% disappear}.
}
$$

We will revisit that logic when discussing GPU performance modeling.

---

**Why Maveb's local subgroup logic is already strong**

The interesting point is that you do **not** need to rewrite the entire pipeline.

The local mechanics are already sophisticated:

$$
\texttt{simd_sum},
$$

$$
\texttt{simd_prefix_exclusive_sum},
$$

$$
\text{threadgroup-local aggregation},
$$

$$
\text{stable scatter}.
$$

The scalability bottleneck is now in global phase construction.

That is exactly the sort of distinction GPU performance work requires:

$$
\boxed{
\text{find the actual serial/global bottleneck instead of optimizing already-fast local math}.
}
$$

---

**GPU algorithm correctness invariant**

Parallel algorithms are easiest to trust when you can write down ownership.

For scan-based allocation:

$$
output_i
=
[offset_i,\ offset_i+count_i).
$$

For any:

$$
i<j,
$$

exclusive scan guarantees:

$$
offset_i+count_i
\le
offset_j.
$$

Therefore output intervals do not overlap.

That is the proof of race freedom.

Do not merely say:

> Threads probably write different locations.

Derive why.

---

**Histogram race-freedom**

If many threads update shared histogram bins nonatomically:

$$
H[d_i]++,
$$

there is a race.

Maveb avoids that within each subgroup by using:

$$
\operatorname{simd\_sum}(digit==bucket).
$$

Only one lane writes the subgroup count:

```metal id="yjtusl"
if (lane == 0)
    simdCounts[...] = total;
```

Distinct subgroups write distinct rows.

No collision.

After barrier, a small set of threads reads those rows.

That is explicit ownership again.

---

**Radix scatter race-freedom**

Every element receives:

$$
destination=
groupOffset[g,b]
+
rankWithinGroup.
$$

`groupOffset` assigns disjoint bucket/group ranges.

`rankWithinGroup` is unique among items in that group/bucket because it comes from prefix counts.

Therefore:

$$
destination_i
\neq
destination_j
$$

for distinct elements.

No atomic is needed.

That is what good GPU algorithms do:

$$
\boxed{
\text{replace synchronization with mathematically unique ownership whenever possible}.
}
$$

---

**Why atomics are sometimes still best**

Scan requires multiple phases and temporary buffers.

If only 20 threads occasionally append something, one atomic append may be much cheaper.

If 50 million threads append, scan becomes attractive.

Thus:

$$
\boxed{
\text{atomics versus scan is a workload-scale/contention tradeoff}.
}
$$

Pass 10's M130 starts precisely with this decision for append buffers and spatial bins.

---

**GPU architecture audit of your current code**

| Topic | Current repository evidence |
|---|---|
| Simple 1D compute | KairoGPU vector add/multiply |
| Global invocation ID | `thread_position_in_grid` |
| 2D workgroup decomposition | KairoGPU 16×16 matrix multiply |
| Threadgroup memory | matrix tiles + Gaussian scan scratch |
| Workgroup barriers | matrix multiply + Blelloch scan |
| Max group-size query | `maxTotalThreadsPerThreadgroup` |
| Execution-width-aware sizing | not currently used |
| Subgroup IDs/lane IDs | Maveb Gaussian radix kernels |
| Subgroup reduction | `simd_sum` |
| Subgroup scan | `simd_prefix_exclusive_sum` |
| Device-memory buffers | extensive Kairo/Maveb use |
| Storage capability abstraction | Vulkax backend features |
| Atomics | Gaussian counters |
| Local-before-global aggregation | radix histogram |
| Blelloch scan | `aetherGaussianScanBlocks` |
| Hierarchical scan | block scan + block sums + add offsets |
| Fully parallel higher scan level | not yet; second stage serial |
| Variable-output allocation | Gaussian tile counts → scan → offsets |
| Histogram | radix group histogram |
| Radix sort | 16 parallel 4-bit passes |
| Stable scatter | yes |
| Key-value sorting | `uint2` keys + Gaussian index values |
| Ping-pong buffers | keysA/B + valuesA/B |
| Large global prefix | currently serial offset kernel |
| Bitonic sort | conceptual curriculum topic, not needed in current Gaussian path |

That is a very meaningful implementation baseline.

---

**Reconstruct the Maveb Gaussian GPU pipeline as a parallel algorithm**

The pipeline can now be understood without looking at code names.

Each Gaussian is projected.

For Gaussian $i$, determine how many screen tiles it overlaps:

$$
c_i.
$$

Exclusive scan:

$$
o_i=
\sum_{j<i}c_j.
$$

Now Gaussian $i$ owns:

$$
[o_i,o_i+c_i).
$$

It generates one key/value entry per overlapped tile.

Key:

$$
(\text{depth bits},\text{tile ID}).
$$

Value:

$$
\text{Gaussian index}.
$$

Stable LSD radix sort first resolves all depth digits and then all tile digits.

Result:

```text id="387b66"
tile 0:
    nearest/farthest ordered Gaussians

tile 1:
    ordered Gaussians

...
```

Range construction records:

$$
[start_t,end_t)
$$

for each tile.

Composite kernel for each pixel reads only the Gaussian range relevant to its tile and blends in sorted order.

That is a complete GPU pipeline built from:

$$
\boxed{
\text{map}
\rightarrow
\text{count}
\rightarrow
\text{scan}
\rightarrow
\text{expand}
\rightarrow
\text{radix sort}
\rightarrow
\text{range construction}
\rightarrow
\text{parallel composite}.
}
$$

This is exactly what the abstract parallel-algorithm curriculum is preparing you to build.

---

**The fundamental GPU thinking shift**

CPU programmer often asks:

> What is the next operation?

GPU programmer must increasingly ask:

> What is the data-parallel ownership structure?

Instead of:

```cpp id="yllozt"
for each Gaussian:
    for each tile:
        append()
```

you derive:

$$
count_i,
$$

$$
offset_i,
$$

$$
\text{scatter region}_i.
$$

Instead of:

```cpp id="qfd2y3"
for each key:
    insert into sorted vector
```

you derive:

$$
digit,
$$

$$
histogram,
$$

$$
prefix,
$$

$$
rank,
$$

$$
scatter.
$$

The important GPU skill is not syntax.

It is:

$$
\boxed{
\text{turning dependency-heavy serial ownership into explicit parallel ownership}.
}
$$

---

**PASS 9 RECONSTRUCTION SHEET**

| Concept | Reconstruction |
|---|---|
| CPU emphasis | low-latency powerful threads |
| GPU emphasis | throughput across massive parallel work |
| Compute unit | scheduling + ALUs + register file + shared memory |
| SIMT | scalar-looking invocations execute in groups |
| Subgroup | hardware execution group |
| Lane | invocation inside subgroup |
| Workgroup | collaboration/synchronization domain |
| Global ID | group × group-size + local ID |
| Latency hiding | issue another ready subgroup while one stalls |
| Occupancy | resident subgroups / hardware maximum |
| Register pressure | live per-thread state consumes register file |
| Spill | logical register moved to slower private memory |
| Threadgroup memory | fast group-local explicitly managed storage |
| Bank conflict | simultaneous accesses collide on same shared-memory bank |
| Device memory | large high-latency storage |
| Coalescing | adjacent lane accesses collapse into few memory transactions |
| Stride | spreads lanes across transactions/cache lines |
| Gather | indirect/random addresses |
| Divergence | subgroup lanes take different control paths |
| Reconvergence | lanes regroup after branch |
| Predication | execute/select small branches without full control divergence |
| Vote | subgroup-wide any/all/ballot |
| Shuffle | register exchange between subgroup lanes |
| Atomic | indivisible RMW operation |
| Contention | many lanes serialize on same atomic location |
| Local aggregation | reduce locally before one global update |
| Workgroup barrier | synchronize one group, not entire GPU |
| Device-wide phase | separate dispatch + appropriate API dependency |
| 1D decomposition | one thread per linear element |
| 2D decomposition | one thread per pixel/matrix coordinate |
| 3D decomposition | one thread per voxel |
| Reduction | many values → one |
| Scan | every element gets prefix aggregate |
| Blelloch | upsweep + root zero + downsweep |
| Hierarchical scan | block scan → block-sum scan → offset add |
| Compaction | predicate → scan → scatter |
| Histogram | count items per bucket |
| Radix digit | $(key>>shift)\&(R-1)$ |
| Radix pass | histogram → prefix → stable scatter |
| LSD stability | preserves previous less-significant ordering |
| Bitonic | deterministic $O(N\log^2N)$ comparison network |
| Key-value sort | reorder key and payload together |
| Maveb key | depth bits + tile ID |
| Maveb final order | tile-major, depth-ordered within tile |

---

**Blank-page mastery gate**

Start with this innocent kernel:

```cpp id="d6k0c3"
output[id] =
    input[id] * 2;
```

and explain all of the invisible hardware underneath it: invocation, local ID, workgroup, subgroup, lane, scheduler, resident warps, register allocation, device-memory transaction and eventual store.

Then explain why a 400-cycle memory load does not necessarily stall the entire GPU for 400 cycles.

Derive conceptual occupancy from register and shared-memory limits, and explain why maximum occupancy is not equivalent to maximum performance.

Then compare:

$$
A[i]
$$

against:

$$
A[32i]
$$

for a subgroup of lanes and explain why the second can produce far more memory transactions.

Then distinguish:

$$
\text{CPU SIMD}
$$

from:

$$
\text{GPU SIMT}.
$$

Then explain why this is cheap:

```cpp id="lgnm0f"
if (id < count)
```

at only the dispatch tail, while this can be catastrophic:

```cpp id="nhzbj0"
if (material[id] == Glass)
    LongGlassPath();
else
    LongDiffusePath();
```

when mixed inside one subgroup.

Then explain why:

```cpp id="hbs55l"
threadgroup_barrier()
```

cannot implement a device-wide barrier.

Then take:

$$
[2,0,3,1]
$$

and derive exclusive scan:

$$
[0,2,2,5].
$$

Use those offsets to allocate unique variable-size output regions without atomics.

Then reconstruct Blelloch scan from the upsweep and downsweep trees.

Then derive stream compaction:

$$
predicate
\rightarrow
scan
\rightarrow
scatter.
$$

Then derive one radix pass:

$$
digit
\rightarrow
histogram
\rightarrow
bucket prefix
\rightarrow
local rank
\rightarrow
stable scatter.
$$

Finally reconstruct your own Maveb Gaussian pipeline:

$$
\text{project Gaussian}
\rightarrow
\text{tile-overlap count}
\rightarrow
\text{exclusive scan}
\rightarrow
\text{generate depth/tile key-value entries}
\rightarrow
\text{stable radix sort}
\rightarrow
\text{tile ranges}
\rightarrow
\text{sorted compositing}.
$$

If you can explain why the current radix histogram uses `simd_sum`, why the scatter uses `simd_prefix_exclusive_sum`, why a threadgroup barrier is required between subgroup-count publication and group aggregation, why the final destination addresses are race-free, and why processing depth digits before tile digits produces **tile-major/depth-minor final ordering**, then you are no longer just writing compute shaders.

You are reasoning in **parallel algorithm structure**.


<!-- Book-completion teaching addition 2326a513e3ee805f -->

## Build GPU bins and fixed hash tables only after capacity and ownership are explicit

The Phase 22 algorithm pass has already derived atomics, exclusive scans, stable scatter, dispatch boundaries and device-memory dependencies. This section uses those tools to build two query structures: a spatial bin index and a fixed-capacity open-addressing table. They solve different lookup problems, but both need explicit ownership and an honest capacity result before a later dispatch is allowed to query their output.

A **spatial bin** maps a position or object bound to one or more cells in a regular grid. A **hash table** maps a discrete key to a slot chosen by a hash and collision-resolution rule. Neither structure becomes correct merely because threads can atomically reserve output positions.

### First decide what a successful build means

Suppose a pass processes $N$ input records and has output capacity $C$. Keep these values distinct:

- `attempted`: how many records requested an output slot;
- `written`: how many records were actually stored within capacity;
- `overflow`: whether any record could not be represented.

Initialize `attempted=0` and `overflow=false` before the pass, and make those initialization writes visible to the producer invocations. Choose a counter width that can represent every attempted reservation without wrapping: even a rejected reservation increments it. Otherwise two requests could receive the same wrapped ticket. A bounded atomic append can reserve a unique ticket:

```text
 ticket = atomic_add(attempted, 1)
 if ticket < C:
     output[ticket] = record
     // This record owns exactly one in-range destination.
 else:
     atomic_store(overflow, true)
```

The atomic counter gives each request a different ticket. It does **not** guarantee that every request fits. When the build pass is finished, the consumer may use the output only if the published result says `overflow == false`. If overflow is true, the structure is incomplete: grow or reallocate and rebuild, or return a visible failure. Do not clamp the count to $C$ and quietly treat the prefix as a complete candidate set. The missing records may contain the only correct hit, collision candidate, or particle.

If every input contributes exactly one record and $N\le C$, then the completed output count is $N$. If some inputs are filtered, use a separate predicate/scan/scatter count or publish the exact completed count after the build. Do not expose a count before all payload writes have finished.

The safe publication point is after the producer dispatch has completed and the API has established the write-to-read dependency for the consumer. In Vulkan, the synchronization guide shows a compute shader-write to later compute shader-read barrier for this case; a dispatch boundary by itself should not be mistaken for the required memory visibility rule. CUDA kernel launches are asynchronous with respect to the host, so host-side reuse also needs stream/event/device synchronization appropriate to the producer and consumer streams. See the [Vulkan compute-to-compute synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html) and the [CUDA guide's kernel and synchronization model](https://docs.nvidia.com/cuda/cuda-programming-guide/02-basics/intro-to-cuda-cpp.html).

### Worked 2D spatial-bin build

Let a 2D grid cover the half-open domain

$$
[-2,2)\times[-2,2)
$$

with cell width $h=1$. There are $4\times4$ cells. For an in-domain point $(x,y)$, define

$$
i_x=\left\lfloor\frac{x-(-2)}{1}\right\rfloor,
\qquad
i_y=\left\lfloor\frac{y-(-2)}{1}\right\rfloor,
\qquad
b=i_y\cdot4+i_x.
$$

Use `floor`, not truncation toward zero. For example, with an origin at zero and width one, $x=-0.2$ belongs to cell $-1$ under a signed grid convention; truncation would incorrectly produce $0$. Here the explicit domain offset keeps valid cell coordinates nonnegative, while `floor` still defines the boundary rule.

Reject non-finite coordinates before applying `floor` or converting to a cell index, and check integer/index ranges before flattening. Assign these records:

| Record | Position | Cell coordinate | Flat bin |
|---|---:|---:|---:|
| A | $(-1.2,-0.1)$ | $(0,1)$ | 4 |
| B | $(0.2,-0.1)$ | $(2,1)$ | 6 |
| C | $(0.9,0.9)$ | $(2,2)$ | 10 |
| D | $(2.0,0.0)$ | outside: upper edge is excluded | reject |
| E | $(-2.1,0.0)$ | outside domain | reject |

The half-open rule is deliberate: points on the lower edge $-2$ are included, and points on upper edge $2$ are excluded. Another engine can clamp or place out-of-domain values in overflow cells, but the choice must be stated and used consistently by both build and query.

Initialize all bin counts to zero. Establish a producer-to-consumer memory dependency at **each** stage boundary: initialization → count, count → scan, scan → scatter, and scatter → query. A single barrier only after scatter cannot repair an earlier scan that read unfinished counts. Build the index in three stages:

1. **Count:** atomically increment `count[bin]` for each accepted record. This count is exact after the dispatch completes, even though the order of increments is nondeterministic.
2. **Exclusive scan:** scan the 16 bin counts to obtain each bin's start offset. For A, B and C, the nonzero counts are `count[4]=1`, `count[6]=1`, and `count[10]=1`. Their exclusive offsets are `offset[4]=0`, `offset[6]=1`, and `offset[10]=2`; the total number of entries is 3.
3. **Scatter:** initialize each bin cursor to zero (with the appropriate write-to-read dependency), then write each input ID into the range owned by its bin. With one record per bin, the result is `bin 4 → [A]`, `bin 6 → [B]`, and `bin 10 → [C]`.

If A and a new record F both land in bin 4, `atomic_add(cursor[4],1)` gives them distinct destinations inside bin 4's range. Which record gets the first destination may vary between runs. That is safe if later code treats a bin as an unordered candidate set. If deterministic order is needed for reproducible output, compute a deterministic per-bin rank—for example, sort by `(bin, inputID)` or use a stable segmented rank—rather than assuming atomic arrival order is stable.

For a record whose bound overlaps multiple cells, count one entry for every overlapped cell. The sum of bin counts is then the required entry capacity, not the number of objects. Use counter and offset widths that can represent the entire entry sum without overflow, and scan those counts before scatter. If the allocated entry array is too small, flag the build as failed and rebuild with enough space; never silently omit overlapped cells.

### Fixed-capacity open addressing with a bounded build pass

A fixed open-addressing table stores keys in slots. When two keys choose the same initial slot, a probing rule examines later slots. The NVIDIA [cuCollections `static_map` documentation](https://github.com/NVIDIA/cuCollections/blob/dev/include/cuco/static_map.cuh) describes a fixed-size open-addressing map and explicitly warns that its insert operations do not grow the table. That is the right capacity lesson: a fixed table needs a deliberate failure policy.

For a small teaching model, use eight slots, linear probing, and

$$
h(k)=k\bmod8.
$$

Reserve `EMPTY` as a sentinel key that is not a valid input key. Initialize every key slot to `EMPTY`. Preprocess the input so keys are unique. If duplicate keys are possible, first group them and either require byte-identical payloads or reduce them in a deterministic, documented order. “Whichever thread writes last” is not a valid duplicate policy.

Each build invocation probes at most eight slots. At a slot, use an atomic compare-and-swap (CAS) to change `EMPTY` to the incoming key. CAS returns the old value: if it was `EMPTY`, this invocation owns that slot and is the only writer of its payload; if it already held the same key, apply the declared duplicate policy; if it held a different key, continue probing. NVIDIA documents `atomicCAS` as an atomic compare-and-swap operation in the [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-programming-guide/pdf/cuda-programming-guide.pdf).

```text
for probe in 0 .. 7:
    slot = (hash(key) + probe) mod 8
    old = atomic_CAS(keySlot[slot], EMPTY, key)

    if old == EMPTY:
        payload[slot] = value       // this invocation owns this new slot
        return INSERTED

    if old == key:
        return DUPLICATE_OR_CONFLICT // resolve before build, by contract

return TABLE_FULL
```

This pseudocode is intentionally **build-only**. It does not allow a query to inspect a slot while another invocation is still writing its payload. The CAS claims the key slot, but this teaching protocol does not use the claimed key as a same-dispatch publication signal for the separate payload. Finish the complete build dispatch, establish the API-specific device-memory dependency, and only then launch the query dispatch. Do not add concurrent deletion, resize, or insert-and-lookup behavior to this example. Production concurrent maps need a stronger publication and memory-ordering design; the official cuCollections API supports richer operations under its documented configuration and constraints.

### Collision trace and full-table failure

Insert keys `1`, `9`, and `17` into an empty eight-slot table. All have initial slot 1:

| Insert | Probe sequence | Result |
|---|---|---|
| 1 | 1 | CAS claims slot 1; owner writes its payload |
| 9 | 1 occupied by 1; 2 empty | CAS claims slot 2; owner writes its payload |
| 17 | 1 occupied; 2 occupied; 3 empty | CAS claims slot 3; owner writes its payload |

After the build dispatch and the producer-to-consumer memory dependency, lookup for 9 probes slots 1 then 2 and returns the payload at slot 2. A lookup must stop at an `EMPTY` slot because this insertion policy never skips an empty slot while probing.

**Full-table exercise with solution.** Fill the table with eight distinct keys. Try to insert key 17. It may compare against eight occupied slots and then return `TABLE_FULL`; the loop must terminate after exactly eight probes. It must not wrap forever, overwrite an occupied key, or publish a partial payload. Set an overflow/failure flag, reject this table version for queries if completeness is required, and rebuild with a larger capacity or report failure. If key 17 is actually a duplicate already present, the equality check returns the existing-key outcome instead; duplicate handling is a separate condition from capacity exhaustion.

**Duplicate-key exercise with solution.** Two input records have key 9 but payloads 4 and 7. The build protocol cannot choose a meaningful value by racing two writers. Preprocess by key and define a deterministic reduction; for example, if the intended value is a sum, reduce to 11 in a fixed input order, then build one `(9,11)` pair. If no reduction is defined, reject the duplicate input as a contract violation.

### Limits and exit check

The bin example uses one point per cell assignment; an object's bounding region may need several bin entries. The hash example uses fixed capacity, a reserved empty sentinel, unique preprocessed keys, bounded linear probing and a build/query phase boundary. It does not prove a production GPU hash map, concurrent lookup during insertion, deletion, resize, cross-API atomic semantics, or performance. Those require API-specific memory-order rules, capacity sizing and tests. NVIDIA's cuCollections is a production implementation reference, while its fixed-map documentation also makes clear that a static map does not grow automatically.

Before using either structure, a learner should be able to: compute a signed grid cell with the declared boundary rule; derive count/scan offsets and scatter destinations; distinguish attempted appends from a complete published output; trace a collision chain and terminate a full-table insertion; explain which thread owns each payload write; state what duplicate keys mean; and show the producer-to-consumer device-memory dependency before querying the finished table.

<!-- Book-completion teaching addition 7165172590d93b10 -->

<!-- Book-completion teaching addition ad7020bce71c0990 -->

### Reconstruct the append, bin and hash contracts on the CPU

This complete C++20 oracle checks the preceding exact coordinates, scan offsets, collision chain, duplicate reduction and capacity failures. Every check stays active with NDEBUG. The sequential implementation deliberately supplies no GPU race, atomic-memory-order or performance evidence. Its publication flag models the contract to enforce; a real GPU backend must implement the device-memory handoff described above.

Run from the vault root:

```sh
c++ -std=c++20 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror tools/labs/gpu_structure_reference_lab.cpp -o /tmp/allmight-gpu-structures
/tmp/allmight-gpu-structures
```

```cpp
// CPU reference oracle for the staged GPU append/bin/hash lesson.
// This models the data contracts sequentially. It proves no GPU visibility,
// race-freedom, memory ordering, scheduling determinism, or performance claim.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void Check(bool condition, const std::string& message)
{
    // Deliberately not assert(): checks stay active under -DNDEBUG and -O2.
    if (!condition)
        throw std::runtime_error("CHECK FAILED: " + message);
}

// The next attempted ticket is returned, then the counter advances.
// Refuse wraparound so a wrapped ticket cannot alias an earlier destination.
bool TryTakeCounterTicket(std::uint64_t& counter, std::uint64_t& ticket)
{
    if (counter == std::numeric_limits<std::uint64_t>::max())
        return false;

    ticket = counter;
    ++counter;
    return true;
}

struct AppendResult
{
    std::uint64_t attempted = 0;
    std::vector<int> stored;
    bool overflow = false;
    bool publishable = false;
};

AppendResult BoundedAppend(const std::vector<int>& input, std::size_t capacity)
{
    AppendResult result;
    result.stored.reserve(capacity);

    for (int value : input)
    {
        std::uint64_t ticket = 0;
        if (!TryTakeCounterTicket(result.attempted, ticket))
        {
            result.overflow = true;
            break;
        }

        if (ticket < capacity)
        {
            // Each ticket below capacity owns one unique slot.
            result.stored.push_back(value);
        }
        else
        {
            // Preserve the failure signal. Never present a truncated prefix as
            // a complete candidate set.
            result.overflow = true;
        }
    }

    result.publishable = !result.overflow;
    return result;
}

struct Grid
{
    double minX = -2.0;
    double minY = -2.0;
    double maxX = 2.0; // Upper edge is excluded: [min, max).
    double maxY = 2.0;
    double cellWidth = 1.0;
    int cellsX = 4;
    int cellsY = 4;
};

struct Point
{
    int id;
    double x;
    double y;
};

struct Cell
{
    int x;
    int y;
    int flat;
};

void ValidateGrid(const Grid& grid)
{
    Check(std::isfinite(grid.minX) && std::isfinite(grid.minY) &&
          std::isfinite(grid.maxX) && std::isfinite(grid.maxY) &&
          std::isfinite(grid.cellWidth) && grid.cellWidth > 0,
          "grid domain and positive width must be finite");
    Check(grid.minX < grid.maxX && grid.minY < grid.maxY &&
          grid.cellsX > 0 && grid.cellsY > 0,
          "grid extent and cell counts must be positive");
    const auto count = static_cast<std::int64_t>(grid.cellsX) * grid.cellsY;
    Check(count <= std::numeric_limits<int>::max(), "flat bin index fits int");
    const double nx = (grid.maxX - grid.minX) / grid.cellWidth;
    const double ny = (grid.maxY - grid.minY) / grid.cellWidth;
    Check(std::isfinite(nx) && std::isfinite(ny) &&
          std::ceil(nx) == grid.cellsX && std::ceil(ny) == grid.cellsY,
          "cell counts must cover exactly the declared domain");
}

std::optional<Cell> ToCell(const Grid& grid, double x, double y)
{
    ValidateGrid(grid);
    // Reject NaN and infinities before floor or integer conversion.
    if (!std::isfinite(x) || !std::isfinite(y))
        return std::nullopt;

    // Half-open domain gives one unambiguous owner to every boundary point.
    if (x < grid.minX || x >= grid.maxX ||
        y < grid.minY || y >= grid.maxY)
        return std::nullopt;

    // floor is essential for signed coordinates. Truncation toward zero would
    // map a negative fraction to cell zero instead of the cell to its left.
    const double fx = std::floor((x - grid.minX) / grid.cellWidth);
    const double fy = std::floor((y - grid.minY) / grid.cellWidth);
    // Validate in floating point before any potentially out-of-range cast.
    if (!std::isfinite(fx) || !std::isfinite(fy) ||
        fx < 0 || fx >= grid.cellsX || fy < 0 || fy >= grid.cellsY)
        return std::nullopt;
    const int ix = static_cast<int>(fx);
    const int iy = static_cast<int>(fy);

    return Cell{ix, iy, iy * grid.cellsX + ix};
}

struct BinBuild
{
    std::vector<std::uint64_t> counts;
    std::vector<std::uint64_t> offsets;
    std::vector<int> recordIds;
    std::uint64_t totalEntries = 0;
    bool overflow = false;
    bool publishable = false;
};

BinBuild BuildPointBins(const Grid& grid,
                        const std::vector<Point>& points,
                        std::size_t entryCapacity)
{
    ValidateGrid(grid);
    const int binCount = grid.cellsX * grid.cellsY;
    BinBuild result;
    result.counts.assign(static_cast<std::size_t>(binCount), 0); // Initialize counts.
    result.offsets.assign(static_cast<std::size_t>(binCount), 0);

    // Count pass. Invalid/out-of-domain points follow the declared reject rule.
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (!cell)
            continue;

        std::uint64_t ignoredTicket = 0;
        if (!TryTakeCounterTicket(result.counts[static_cast<std::size_t>(cell->flat)],
                                  ignoredTicket))
        {
            result.overflow = true;
            return result;
        }
    }

    // Exclusive scan. Check addition so neither total nor offsets can wrap.
    std::uint64_t running = 0;
    for (int bin = 0; bin < binCount; ++bin)
    {
        result.offsets[static_cast<std::size_t>(bin)] = running;
        const std::uint64_t count = result.counts[static_cast<std::size_t>(bin)];
        if (count > std::numeric_limits<std::uint64_t>::max() - running)
        {
            result.overflow = true;
            return result;
        }
        running += count;
    }
    result.totalEntries = running;

    // Capacity is checked before scatter. An undersized output is a failed build,
    // not a partially valid index.
    if (result.totalEntries > entryCapacity)
    {
        result.overflow = true;
        return result;
    }

    result.recordIds.assign(static_cast<std::size_t>(result.totalEntries), -1);
    std::vector<std::uint64_t> cursors(static_cast<std::size_t>(binCount), 0); // Initialize cursors.

    // Sequential CPU reference scatter preserves input order within each bin.
    // A GPU atomic cursor can assign a different within-bin order across runs.
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (!cell)
            continue;

        const std::size_t bin = static_cast<std::size_t>(cell->flat);
        const std::uint64_t local = cursors[bin]++;
        const std::uint64_t destination = result.offsets[bin] + local;
        Check(destination < result.recordIds.size(), "scatter destination is in range");
        result.recordIds[static_cast<std::size_t>(destination)] = point.id;
    }

    result.publishable = true;
    return result;
}

std::vector<std::pair<int, int>> DeterministicBinOrder(
    const Grid& grid, const std::vector<Point>& points)
{
    // Sorting by (bin, record ID) is a deterministic alternative when a stable
    // order is required instead of atomic arrival order.
    std::vector<std::pair<int, int>> entries;
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (cell)
            entries.emplace_back(cell->flat, point.id);
    }
    std::sort(entries.begin(), entries.end());
    return entries;
}

struct KeyValue
{
    int key;
    int value;
};

enum class DuplicatePolicy
{
    Reject,
    SumAfterDeterministicSort
};

std::optional<std::vector<KeyValue>> PreprocessKeys(
    std::vector<KeyValue> input, DuplicatePolicy policy)
{
    std::stable_sort(input.begin(), input.end(),
        [](const KeyValue& a, const KeyValue& b) { return a.key < b.key; });

    std::vector<KeyValue> output;
    for (std::size_t i = 0; i < input.size();)
    {
        std::size_t end = i + 1;
        while (end < input.size() && input[end].key == input[i].key)
            ++end;

        if (end - i > 1 && policy == DuplicatePolicy::Reject)
            return std::nullopt;

        if (policy == DuplicatePolicy::SumAfterDeterministicSort)
        {
            // Input order among equal keys is retained by stable_sort, so this
            // reference reduction has an explicitly fixed order.
            std::int64_t sum = 0;
            for (std::size_t j = i; j < end; ++j)
            {
                const auto term = static_cast<std::int64_t>(input[j].value);
                Check((term >= 0 && sum <= std::numeric_limits<std::int64_t>::max() - term) ||
                      (term < 0 && sum >= std::numeric_limits<std::int64_t>::min() - term),
                      "duplicate accumulator must not overflow");
                sum += term;
            }
            Check(sum >= std::numeric_limits<int>::min() &&
                  sum <= std::numeric_limits<int>::max(),
                  "duplicate reduction fits payload type");
            output.push_back(KeyValue{input[i].key, static_cast<int>(sum)});
        }
        else
        {
            output.push_back(input[i]);
        }
        i = end;
    }
    return output;
}

class FixedHashTable
{
public:
    static constexpr std::size_t kCapacity = 8;
    static constexpr int kEmpty = std::numeric_limits<int>::min();

    enum class InsertStatus { Inserted, Duplicate, Full, InvalidSentinel };

    FixedHashTable()
    {
        // Initialize every key and payload before the build phase.
        keys_.fill(kEmpty);
        values_.fill(0);
    }

    InsertStatus InsertBuildOnly(KeyValue item, std::size_t& probes)
    {
        Check(!published_, "cannot insert after query publication");
        probes = 0;
        if (item.key == kEmpty)
        {
            failed_ = true;
            return InsertStatus::InvalidSentinel;
        }

        const std::size_t first = Hash(item.key);
        for (std::size_t attempt = 0; attempt < kCapacity; ++attempt)
        {
            ++probes;
            const std::size_t slot = (first + attempt) % kCapacity;

            if (keys_[slot] == kEmpty)
            {
                // The successful CAS-equivalent claim owns the payload write.
                // This sequential oracle models slot ownership only; it does
                // not prove a concurrent GPU implementation is race-free.
                keys_[slot] = item.key;
                values_[slot] = item.value;
                return InsertStatus::Inserted;
            }

            if (keys_[slot] == item.key)
            {
                // Duplicates must be resolved before this build pass. Never
                // let two racing writers choose a payload by arrival order.
                failed_ = true;
                return InsertStatus::Duplicate;
            }
        }

        failed_ = true;
        return InsertStatus::Full; // Bounded: at most eight probes.
    }

    bool FinishBuild()
    {
        // A real GPU must additionally establish API-specific device-memory
        // visibility before its separate query dispatch.
        published_ = !failed_;
        return published_;
    }

    std::optional<int> LookupPublished(int key, std::size_t& probes) const
    {
        Check(published_, "query is rejected until complete build is published");
        probes = 0;
        const std::size_t first = Hash(key);
        for (std::size_t attempt = 0; attempt < kCapacity; ++attempt)
        {
            ++probes;
            const std::size_t slot = (first + attempt) % kCapacity;
            if (keys_[slot] == kEmpty)
                return std::nullopt;
            if (keys_[slot] == key)
                return values_[slot];
        }
        return std::nullopt;
    }

private:
    static std::size_t Hash(int key)
    {
        // This is the lesson's deliberately simple positive-key hash.
        const int remainder = key % static_cast<int>(kCapacity);
        return static_cast<std::size_t>(remainder < 0 ? remainder + kCapacity : remainder);
    }

    std::array<int, kCapacity> keys_{};
    std::array<int, kCapacity> values_{};
    bool failed_ = false;
    bool published_ = false;
};

void RunChecks()
{
    // Check invalid contracts explicitly rather than relying on undefined casts.
    Grid invalidGrid;
    invalidGrid.cellWidth = 0;
    bool badGridRejected = false;
    try { (void)ToCell(invalidGrid, 0, 0); }
    catch (const std::runtime_error&) { badGridRejected = true; }
    Check(badGridRejected, "zero-width grid is rejected before division");

    FixedHashTable unpublished;
    std::size_t beforeProbes = 0;
    bool earlyQueryRejected = false;
    try { (void)unpublished.LookupPublished(1, beforeProbes); }
    catch (const std::runtime_error&) { earlyQueryRejected = true; }
    Check(earlyQueryRejected, "query before publication is rejected");
    Check(unpublished.InsertBuildOnly({FixedHashTable::kEmpty, 0}, beforeProbes) ==
          FixedHashTable::InsertStatus::InvalidSentinel, "reserved key is rejected");
    Check(!unpublished.FinishBuild(), "sentinel failure invalidates the build");

    // Counter-width boundary: no wraparound ticket is ever returned.
    std::uint64_t counter = std::numeric_limits<std::uint64_t>::max() - 1;
    std::uint64_t ticket = 0;
    Check(TryTakeCounterTicket(counter, ticket), "last representable increment succeeds");
    Check(ticket == std::numeric_limits<std::uint64_t>::max() - 1,
          "ticket equals pre-increment counter");
    Check(counter == std::numeric_limits<std::uint64_t>::max(), "counter reaches max");
    Check(!TryTakeCounterTicket(counter, ticket), "counter refuses wraparound");

    // Append capacity is explicit; attempted is not the same as stored.
    const AppendResult append = BoundedAppend({10, 20, 30}, 2);
    Check(append.attempted == 3, "append counts all requests");
    Check(append.stored == std::vector<int>({10, 20}), "only in-capacity tickets store");
    Check(append.overflow, "append overflow is visible");
    Check(!append.publishable, "truncated append result is not published as complete");

    const AppendResult appendFits = BoundedAppend({4, 5}, 2);
    Check(appendFits.attempted == 2 && appendFits.stored.size() == 2,
          "in-capacity append stores every input");
    Check(!appendFits.overflow && appendFits.publishable, "complete append is publishable");

    // Signed coordinate, finite-value, and half-open-boundary cases.
    const Grid grid;
    const auto lower = ToCell(grid, -2.0, -2.0);
    Check(lower && lower->x == 0 && lower->y == 0 && lower->flat == 0,
          "lower boundary is included");
    Check(!ToCell(grid, 2.0, 0.0), "upper x boundary is excluded");
    Check(!ToCell(grid, 0.0, 2.0), "upper y boundary is excluded");
    Check(!ToCell(grid, -2.1, 0.0), "out-of-domain point is rejected");
    Check(!ToCell(grid, std::numeric_limits<double>::infinity(), 0.0),
          "nonfinite point is rejected");
    const double negativeFraction = std::floor((-0.2 - (-1.0)) / 1.0);
    Check(negativeFraction == 0.0, "offset floor gives expected signed-grid cell");
    Check(std::trunc(-0.2) == 0.0 && std::floor(-0.2) == -1.0,
          "truncation differs from floor for a negative fraction");

    const std::vector<Point> points{
        {1, -1.2, -0.1}, // cell (0,1), flat bin 4
        {2,  0.2, -0.1}, // cell (2,1), flat bin 6
        {3,  0.9,  0.9}, // cell (2,2), flat bin 10
        {4,  2.0,  0.0}, // excluded upper boundary
        {5, -2.1,  0.0}  // outside
    };
    const BinBuild bins = BuildPointBins(grid, points, 3);
    Check(bins.publishable && !bins.overflow, "exact-capacity bin build succeeds");
    Check(bins.totalEntries == 3, "only three in-domain points are binned");
    Check(bins.counts[4] == 1 && bins.counts[6] == 1 && bins.counts[10] == 1,
          "expected bins have one entry each");
    Check(bins.offsets[4] == 0 && bins.offsets[6] == 1 && bins.offsets[10] == 2,
          "exclusive scan offsets are 0,1,2");
    Check(bins.recordIds == std::vector<int>({1, 2, 3}),
          "scatter writes records to scanned ranges");
    const BinBuild binsTooSmall = BuildPointBins(grid, points, 2);
    Check(binsTooSmall.overflow && !binsTooSmall.publishable,
          "undersized bin array fails before publishing partial output");

    const std::vector<Point> sameBin{
        {8, -1.2, -0.1}, {6, -1.4, -0.4}
    };
    const auto stable = DeterministicBinOrder(grid, sameBin);
    Check(stable == std::vector<std::pair<int, int>>({{4, 6}, {4, 8}}),
          "(bin,ID) sort yields deterministic within-bin order");

    // Duplicate-key policy: reject by default, or explicitly pre-reduce.
    Check(!PreprocessKeys({{9, 4}, {9, 7}}, DuplicatePolicy::Reject),
          "duplicate policy can reject conflicting keys");
    const auto reduced = PreprocessKeys({{9, 4}, {9, 7}},
                                        DuplicatePolicy::SumAfterDeterministicSort);
    Check(reduced && reduced->size() == 1 && (*reduced)[0].key == 9 &&
          (*reduced)[0].value == 11, "duplicate sum is explicit and deterministic");

    // Collision trace: 1, 9 and 17 all begin at slot 1 in the 8-slot table.
    FixedHashTable table;
    std::size_t probes = 0;
    Check(table.InsertBuildOnly({1, 100}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 1, "key 1 claims slot 1");
    Check(table.InsertBuildOnly({9, 900}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 2, "key 9 probes slots 1 then 2");
    Check(table.InsertBuildOnly({17, 1700}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 3, "key 17 probes slots 1,2,3");
    Check(table.FinishBuild(), "complete collision table is published");
    Check(table.LookupPublished(9, probes) == std::optional<int>(900) && probes == 2,
          "lookup follows probe chain after build publication");
    Check(!table.LookupPublished(25, probes), "missing lookup stops at empty slot");

    // Duplicate insertions invalidate this build rather than race to choose a value.
    FixedHashTable duplicateTable;
    Check(duplicateTable.InsertBuildOnly({9, 4}, probes) == FixedHashTable::InsertStatus::Inserted,
          "first unique key inserted");
    Check(duplicateTable.InsertBuildOnly({9, 7}, probes) == FixedHashTable::InsertStatus::Duplicate,
          "duplicate key is rejected");
    Check(!duplicateTable.FinishBuild(), "failed duplicate build is not published");

    // Full-table failure is bounded to exactly eight probes and not published.
    FixedHashTable full;
    for (int key = 0; key < 8; ++key)
        Check(full.InsertBuildOnly({key, key * 10}, probes) ==
              FixedHashTable::InsertStatus::Inserted, "fill one table slot per key");
    Check(full.InsertBuildOnly({17, 170}, probes) == FixedHashTable::InsertStatus::Full,
          "ninth unique key fails in a full table");
    Check(probes == FixedHashTable::kCapacity, "full-table search is bounded to capacity probes");
    Check(!full.FinishBuild(), "full-table build is not published as complete");

    std::cout << "PASS: CPU reference contracts (append, bins, duplicates, bounded hash build)\n";
}

} // namespace

int main()
{
    try
    {
        RunChecks();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

## Keep a frame's memory alive until its final consumer is finished

The bin builder has a clear end: publish a complete structure, then let a later dispatch read it. A renderer repeats the same problem at a larger scale. While the GPU reads frame $f$'s transforms, the CPU wants to prepare frame $f+1$. Giving both processors the same mutable bytes would make the answer depend on who reaches the memory first. Buffering gives each in-flight version its own storage. Synchronization decides when that storage becomes reusable.

A **frame in flight** is submitted work whose relevant consumers have not finished. Its lifetime is not necessarily the lifetime of the displayed image. A swapchain image, a CPU command-recording slot and an upload-memory slice may have different counts and completion conditions. Record which consumer retires each resource instead of treating one frame number as universal permission to overwrite.

### Double and triple buffering change ownership, not execution speed

With one transform buffer, the CPU must wait before overwriting data the GPU still reads. Two or three versions let it prepare another frame while earlier data remains owned by the GPU. This can hide unequal preparation times. It cannot make a six-millisecond GPU workload execute in two milliseconds. Extra queued frames can increase the age of input when it appears on screen.

Work a simple timeline. CPU preparation takes 2 ms per frame; one GPU queue takes 6 ms per frame. Ignore presentation and transfer costs. Assume one completion token covers every GPU read of a slot. The CPU may write a slot only after its previous token completes.

| Frame and slot | CPU writes | Submitted at | GPU execution | Slot may be reused |
|---|---:|---:|---:|---:|
| 0, A | 0–2 ms | 2 ms | 2–8 ms | 8 ms |
| 1, B | 2–4 ms | 4 ms | 8–14 ms | 14 ms |
| 2, C | 4–6 ms | 6 ms | 14–20 ms | 20 ms |
| 3, A | 8–10 ms | 10 ms | 20–26 ms | 26 ms |

At 6 ms the CPU cannot overwrite A: frame 0 reads it until 8 ms. It waits for **A's retirement**; cycling the slot index proves nothing about completion. After startup, the GPU finishes one frame every 6 ms, at most about $1000/6\approx166.7$ frames per second in this model.

**Solved comparison.** With two slots, frame 2 cannot begin writing A until 8 ms. It is prepared from 8–10 ms and executes from 14–20 ms. The first GPU completion times remain 8, 14 and 20 ms. Triple buffering did not improve sustained throughput here. It let the CPU prepare frame 2 earlier, which can absorb variable preparation time, but can also queue older input. Measure presentation latency separately from throughput.

A **GPU command processor** consumes submitted commands and directs work to execution engines. Recording or submitting a draw does not mean its shader invocations have finished. Before resetting command storage, recycling descriptors or releasing resources, observe the completion condition required by the backend.

### Async compute starts with a dependency graph

**Asynchronous compute** submits compute work through a path that can be scheduled independently of graphics where the device and API permit. A second queue is an opportunity for overlap, not proof of a second independent GPU. The workloads can compete for shader cores, registers, caches and memory bandwidth.

Suppose an upload $U$ takes 1 ms; culling $C$ takes 2 ms and reads that upload; an unrelated shadow pass $S$ takes 3 ms; main drawing $D$ takes 4 ms and reads both culling and shadows. Dependencies are $U\to C\to D$ and $S\to D$. If $U$ and $S$ start at time zero on suitable engines, $C$ follows $U$, and there is no resource contention, $D$ starts at $\max(1+2,3)=3$ ms and ends at 7 ms. A serial schedule takes 10 ms. Seven milliseconds is an **ideal schedule bound under those assumptions**, not a device benchmark.

If culling reads last frame's depth, identify that input version and its temporal validity. If it reads the same main pass's not-yet-produced depth, the proposed overlap is cyclic. If culling and shadows both saturate bandwidth, overlap may lengthen both. Compare measured serial and overlapped schedules using the same inputs and quality requirements. NVIDIA's [async-compute guidance](https://developer.nvidia.com/blog/advanced-api-performance-async-compute-and-overlap/) explains complementary workloads and contention on its hardware; it is not a universal speedup guarantee.

A **copy or transfer queue** accepts transfers separately from graphics/compute commands. Some hardware has dedicated copy engines, but queue availability does not guarantee physical independence or full overlap. Copied data still needs a transfer-write-to-consumer-read dependency. Vulkan queues from different families may also need ownership transfers for exclusively shared resources. Host completion, queue ordering and access/layout transitions answer different questions. Use the [Vulkan synchronization guide](https://docs.vulkan.org/guide/latest/synchronization.html) and its examples for backend-specific rules.

### Uploads need bandwidth, visibility and retirement

An **upload** moves CPU-produced data into storage usable by the GPU. A discrete GPU may use host-visible staging storage followed by a transfer to device-local storage. Unified-memory hardware has a different physical path; shared memory does not remove ordering, cache-policy or ownership rules.

For $B$ bytes at sustained effective bandwidth $W$ bytes per second, the transfer-only estimate is $B/W$. With an assumed 16 GiB/s, uploading 64 MiB takes

$$
t=\frac{64\cdot2^{20}}{16\cdot2^{30}}\ \mathrm{s}
 =\frac1{256}\ \mathrm{s}\approx3.90625\ \mathrm{ms}.
$$

This excludes allocation, command overhead, cache maintenance and contention. An advertised interconnect rate is not achieved application bandwidth.

**Persistent mapping** keeps a host-visible allocation mapped across updates, avoiding repeated map/unmap calls. It does not mean the GPU has stopped reading. In Vulkan, non-coherent mapped memory needs the specified flush for host writes. Coherent memory removes that explicit cache-maintenance requirement but still needs the applicable ordering and visibility rules. Respect non-coherent atom size and resource alignment; distinct logical slices may share a cache-maintenance atom. See the [Vulkan memory specification](https://docs.vulkan.org/spec/latest/chapters/memory.html).

A **ring upload buffer** wraps a write position within a bounded allocation. Track each live interval's offset, size and final-consumer completion token. Align a proposed start, check arithmetic and allocation bounds, and prove the whole interval avoids every unretired range. Wrapping is not evidence that bytes are free. If no safe interval exists, wait for the required completion, grow within budget, use another allocation or visibly defer/reject the upload. Microsoft gives a [fence-based ring-buffer lifetime example](https://learn.microsoft.com/en-us/windows/win32/direct3d12/fence-based-resource-management).

**Worked allocation.** A 1024-byte ring uses 256-byte alignment. A owns $[0,256)$ and retires at token 11; B owns $[256,512)$ and retires at 12. A 300-byte request at offset 512 occupies $[512,812)$; the next aligned start is 1024, so another request must wrap or use another allocation. At completed token 10, wrapping to zero would overwrite A. At token 11, a 128-byte request can use $[0,128)$, but a 300-byte request would overlap B and remains unsafe until token 12. These comparisons assume one ordered completion timeline covering every consumer. Independent queues need a join token or per-queue retirement conditions.

### Readback reverses the direction, not the lifetime rule

A **readback** makes GPU-produced data available to the CPU. Read only after the GPU write/copy completes and after the backend's host-visibility/cache operations. Vulkan non-coherent host reads require invalidation after completion. Mapping memory does not wait for its producer.

Waiting for an occasional screenshot may be acceptable. Per-frame statistics can use a readback ring: the CPU examines an older completed sample while the GPU produces another. Label samples with source frame and configuration. If a CPU decision requires the current GPU answer, that dependency is real; delaying readback changes the algorithm.

### Suballocation and aliasing require different proofs

A **suballocation** gives resources disjoint intervals within a larger allocation. It reduces allocation overhead but must respect queried size, alignment, compatible memory types and dedicated-allocation requirements. A **dedicated allocation** gives one resource its own allocation, simplifying ownership or satisfying a requirement at a potential packing cost. Do not infer allocation requirements solely from visible pixel or vertex counts.

**Aliasing** lets resources share physical bytes at different times. It needs spatial compatibility and a temporal proof that the old resource's final access precedes the new resource's first access, plus backend-specific aliasing, synchronization and layout rules. An asynchronous reader prevents reuse even after the resource's last graphics command has been submitted.

For compatible resources requiring 3 MiB, 5 MiB and 4 MiB, a serial A→B→C chain with no overlapping lifetimes may fit in one 5 MiB region if API/alignment constraints permit. If A and C overlap while B is disjoint from both, simultaneous A+C demand is 7 MiB. A valid plan gives A and C disjoint 3+4 MiB ranges and lets B reuse memory later. The one-resource maximum of 5 MiB is insufficient. Allocation therefore follows the scheduled dependency graph, not just pass declaration order.

**Transient** describes an intended short lifetime, not automatic permission to alias or discard data. It is also distinct from Vulkan's specific transient-attachment usage and lazily allocated memory features. The Engine book's [[Chapter Edition/03 - Game Engine/42 - From cartridge bytes to a reproducible machine#From a list of render passes to a proved execution order|render-graph compiler]] develops the ownership representation; these intervals explain what it must prove.

**Exercise and solution.** May the CPU rewrite a mapped staging slice when its copy finishes? Only if that copy is the slice's final consumer. If later work reads directly from the slice, retirement is later. If the copy reads staging storage and later work reads only a separate destination allocation, copy completion can retire staging while the destination stays alive. Draw both allocations and name their last consumers separately.

The exit test is practical: identify every owner, byte interval, visibility operation and completion condition in one frame graph. Buffer count, queue count and mapping duration are implementation choices around that proof.

<!-- Book-completion teaching addition ce8e1a049c0b90f5 -->

## Measure the limiting resource before changing the arithmetic

Once ownership and synchronization are correct, a slow frame becomes a measurement problem. The CPU's submission time is not the GPU's execution time, and a high count of active threads is not the same as useful throughput. Start with a reproducible input, a correct reference output and a clearly bounded region of work. Otherwise a faster result may simply be doing less work or returning a different answer.

### A query reports a defined event, not an explanation by itself

A **timestamp query** records a device timestamp at an API-defined point. A **pipeline-statistics query** counts supported events such as shader invocations or processed primitives. Statistics answer how much of a defined activity occurred; timestamps help measure elapsed device time. Neither directly identifies the cause of a bottleneck.

In Vulkan, check feature support, the chosen queue's timestamp-valid bits and timestamp period, the allowed query scope, reset/reuse rules and result availability. Read results only when available, or explicitly account for a wait. Convert timestamp differences using the advertised period and valid-bit width. Do not subtract arbitrary counters from different queues as if all devices guaranteed a single synchronized clock. The [Vulkan query specification](https://docs.vulkan.org/spec/latest/chapters/queries.html) defines those contracts.

For an illustrative queue with a 1 ns timestamp period and no wrap during the measured interval, ticks 120000 and 2120000 differ by 2000000 ticks, hence 2 ms. Waiting on those results immediately after each dispatch may serialize the workload being measured. Record multiple zones and read a completed result set later. A profiling zone is a named measurement interval; labels need the frame/configuration identity so an old result is not attached to a new experiment.

Suppose culling halves submitted triangles but draw time barely changes. The triangle count confirms reduced work at one stage. The remaining bottleneck could be fragment shading, memory traffic or fixed overhead. Conversely, fewer fragment invocations with a larger elapsed time can arise from more expensive surviving fragments. Use a controlled change and relevant counters rather than naming a winner from one statistic.

### Derive a roofline bound with matching units

**Arithmetic intensity** is useful arithmetic work divided by bytes crossing a specified memory boundary:

$$
I=\frac{F}{B}\quad\left[\frac{\mathrm{FLOP}}{\mathrm{byte}}\right].
$$

A FLOP is one floating-point operation in the stated counting convention. A fused multiply-add is usually counted as two operations even though it can be one instruction. Specify whether $B$ is DRAM traffic, cache traffic or a simplified source-level estimate. Those denominators produce different intensities.

If an appropriate arithmetic peak is $P_{\max}$ FLOP/s and sustainable bandwidth at that boundary is $W$ bytes/s, the basic roofline bound is

$$
P\leq\min(P_{\max},WI),\qquad
t\geq\max\left(\frac{F}{P_{\max}},\frac{B}{W}\right).
$$

The **ridge point** $I_*=P_{\max}/W$ separates the bandwidth ceiling from the arithmetic ceiling in this simplified model. The inequalities are optimistic bounds. Launch overhead, dependency chains, occupancy, instruction mix, cache misses and imperfect scheduling can make the kernel slower. A point below the roof is not proof of a compiler bug. See [NVIDIA's roofline analysis](https://developer.nvidia.com/blog/accelerating-hpc-applications-with-nsight-compute-roofline-analysis/) for the measurement method and hierarchy of memory boundaries.

**Worked streaming example.** For FP32 $y_i=a x_i+y_i$, count one multiplication and one addition: 2 FLOP per element. Assuming one 4-byte read of $x_i$, one 4-byte read of $y_i$ and one 4-byte write of $y_i$, the source-level estimate is 12 bytes and $I=1/6$ FLOP/byte. Treat the scalar $a$ as reused. On an illustrative machine with 2 TFLOP/s appropriate peak and 200 GB/s sustainable bandwidth, the ridge is 10 FLOP/byte. The streaming kernel's bandwidth ceiling is about 33.3 GFLOP/s, far below 2 TFLOP/s. Replacing one arithmetic instruction cannot eliminate its data movement. Actual DRAM bytes may differ because of caches, transactions or store policy; measure them before treating 12 bytes as an observed counter.

**Worked reuse example.** For dense square $C=AB$ with dimension $n$, count $2n^3$ operations in the conventional multiply-add model. If an idealized algorithm transfers A and B once and writes C once, FP32 traffic is $12n^2$ bytes, so $I=n/6$. At $n=128$, that estimate is about 21.3 FLOP/byte. This says that sufficient data reuse can move the kernel toward an arithmetic limit. It does not prove a naive implementation reads each input once. Tiling, register/shared storage and scheduling must actually realize reuse. Reading an old C for an additional scaled accumulation changes the byte count.

**Occupancy** measures resident active execution groups relative to a hardware limit. More occupancy can hide latency, but reducing registers to raise occupancy may spill values into memory. A lower-occupancy kernel that reuses data or performs more independent work per thread can win. Benchmark the actual instruction and memory behavior; do not optimize the occupancy percentage as if it were the application's objective. The [CUDA Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html) develops the hardware-specific measurement context.

### Fast math changes a numerical contract

A shader compiler can remove redundant work, combine operations and choose instructions under its permitted semantics. **Fast math** is a family of compiler options and assumptions, not one portable algorithm. Depending on compiler and flags, it can permit reassociation, contraction, approximate reciprocals or functions, flushing tiny values, or assumptions about NaNs, infinities and signed zero. Record exact compiler version, flags and target; do not assume a flag has identical meaning across backends.

Finite-precision addition is not associative. Under rounded FP32-like arithmetic, adding a small number to a huge opposite-sign pair can produce either the small number or zero depending on association. A threshold test can then choose a different branch. Fusing multiplication and addition changes where rounding happens; it can improve accuracy while also changing bitwise results. The Foundations book's floating-point lesson supplies the representation model; here the consequence is that optimization needs an output-error contract.

For an image pass, compare both image error and the decisions it changes: coverage, depth ordering, visibility, convergence or accumulated energy. For a geometric predicate, a tiny numerical error can flip a sign and change topology. A low average error does not make that acceptable. Keep a strict baseline, test exceptional and near-boundary inputs, and accept the optimization only within a justified domain and tolerance.

### Half precision is a range and accumulation decision

IEEE binary16 uses 1 sign bit, 5 exponent bits and 10 explicit fraction bits, giving 11 significant binary digits for normal values. Its largest finite value is 65504; the smallest positive normal is $2^{-14}$ and the smallest positive subnormal is $2^{-24}$. Device behavior for subnormals and intermediate arithmetic must still be checked.

Near one, adjacent normal values are spaced by $2^{-10}$. With round-to-nearest the usual unit-roundoff scale is $2^{-11}$, about $4.88\times10^{-4}$. Near 2048, spacing is 2, so 2049 lies halfway between representable neighbors; ties-to-even rounds it to 2048. Repeatedly adding a unit increment to an FP16 accumulator already at 2048 can lose every increment. An FP32 accumulator can retain those increments over this small example, but cannot undo inputs that were already rounded to FP16.

Distinguish storage format, multiplication format, accumulator format and final conversion. FP16 storage with FP32 accumulation can reduce traffic without forcing every partial sum into FP16. It still has input-quantization error, and a final FP16 conversion can overflow or lose detail. Matrix/Tensor hardware may have additional formats and rounding rules. The [CUDA half-precision API](https://docs.nvidia.com/cuda/cuda-math-api/cuda_math_api/group__CUDA__MATH__INTRINSIC__HALF.html) is an API reference, not permission to infer every target's behavior.

**Exercise and solution.** A position is roughly 2048 world units from the origin and must retain changes of 0.1 units. Direct binary16 world-coordinate storage cannot represent that spacing there. Store a local coordinate relative to a stable origin, use a wider representation, or redesign the error budget. A later FP32 computation cannot recover the lost 0.1 if quantization already erased it.

### Collective matrix operations distribute a tile across lanes

A **subgroup matrix operation** cooperatively computes a small matrix product using values distributed among participating lanes. The logical equation is still

$$
D_{ij}=\sum_{k=0}^{K-1} A_{ik}B_{kj}+C_{ij}.
$$

For $A=\begin{bmatrix}1&2\\3&4\end{bmatrix}$ and $B=\begin{bmatrix}5&6\\7&8\end{bmatrix}$ with $C=0$, the result is $\begin{bmatrix}19&22\\43&50\end{bmatrix}$. The four dot products contain eight multiply-add pairs, conventionally 16 FLOP. That mathematical result does not specify which lane stores each fragment.

A backend must query the supported tile shapes, input/accumulator types and subgroup/feature requirements, then use its documented load, multiply-accumulate and store operations. Do not guess fragment layout from lane IDs or assume a vendor's tile shape is portable. Every required participant must reach a collective operation under the API's execution rules. For a partial edge tile, pad or mask input loads according to a valid collective protocol rather than letting required lanes skip the operation.

This is the next form of the reuse argument: a tile's loaded elements contribute to several dot products before leaving fast storage. Tensor-style peak throughput matters only if the algorithm reaches that path with supported formats, sufficient reuse and acceptable numerical error. The exit test is to derive the result, account for bytes and FLOPs, name the precision at every stage, and explain why a supported collective cannot be treated as independent per-thread multiplication.

<!-- Book-completion teaching addition b5313e0d1775a1df -->

### Check the scheduling and precision examples with a scalar witness

The following complete standard-library Python program reproduces the two/three-slot timeline, retired byte intervals, transfer units, ideal overlap/roofline bounds, binary16 rounding and the small matrix result. Its six checks run normally and with Python optimization enabled. They validate the stated scalar model, not GPU scheduling, cache coherence, compiler fast-math behavior or hardware matrix operations.

```sh
python3 tools/labs/gpu_budget_precision_lab.py
python3 -O tools/labs/gpu_budget_precision_lab.py
```

```python
#!/usr/bin/env python3
"""Scalar witnesses for the book's GPU timeline/budget examples, not a GPU test."""
from dataclasses import dataclass
import math
import struct
import unittest


def rounded(value, code):
    # 'e' is IEEE binary16 storage; 'f' is binary32 storage. Packing performs a
    # storage conversion. Host arithmetic between conversions remains Python float.
    # This cannot certify shader flags, device subnormals or matrix instructions.
    return struct.unpack('<' + code, struct.pack('<' + code, float(value)))[0]


@dataclass(frozen=True)
class Frame:
    number: int
    slot: int
    cpu_start: float
    submitted: float
    gpu_start: float
    retired: float


def timeline(cpu_ms, gpu_ms, slots, frame_count):
    # One ordered GPU queue; retirement covers every consumer of a frame slot.
    # No transfer, presentation, separate queues or host-recording overhead.
    if (not math.isfinite(cpu_ms) or not math.isfinite(gpu_ms) or
            cpu_ms <= 0 or gpu_ms <= 0 or
            not isinstance(slots, int) or isinstance(slots, bool) or slots <= 0 or
            not isinstance(frame_count, int) or frame_count < 0):
        raise ValueError('positive finite durations/slots and nonnegative frame count required')
    slot_retired = [0.0] * slots
    cpu_time = gpu_time = 0.0
    result = []
    for number in range(frame_count):
        slot = number % slots
        # Reusing the index grants no ownership: wait for that slot's old reader.
        cpu_start = max(cpu_time, slot_retired[slot])
        submitted = cpu_start + cpu_ms
        gpu_start = max(submitted, gpu_time)
        retired = gpu_start + gpu_ms
        if not all(math.isfinite(t) for t in (submitted, retired)):
            raise OverflowError('timeline exceeds representable duration')
        result.append(Frame(number, slot, cpu_start, submitted, gpu_start, retired))
        slot_retired[slot] = retired
        cpu_time, gpu_time = submitted, retired
    return result


def overlap(first, second):
    # Adjacent half-open ranges do not overlap.
    return first[0] < second[1] and second[0] < first[1]


def can_reuse(request, live_ranges, completed):
    # One scalar token is valid only for one ordered/joined completion timeline.
    return all(token <= completed or not overlap(request, interval)
               for interval, token in live_ranges)


class BudgetPrecisionChecks(unittest.TestCase):
    def test_buffer_timeline_and_throughput(self):
        frames = timeline(2, 6, 3, 4)
        self.assertEqual([(f.cpu_start, f.submitted, f.gpu_start, f.retired)
                          for f in frames],
                         [(0, 2, 2, 8), (2, 4, 8, 14),
                          (4, 6, 14, 20), (8, 10, 20, 26)])
        twice = timeline(2, 6, 2, 3)
        self.assertEqual([f.retired for f in twice], [8, 14, 20])
        # More slots did not change the bottleneck's completion cadence here.
        self.assertEqual(twice[2].cpu_start, 8)
        self.assertEqual(frames[2].cpu_start, 4)

    def test_interval_retirement_not_modulo(self):
        live = [((0, 256), 11), ((256, 512), 12)]
        self.assertFalse(can_reuse((0, 128), live, 10))
        self.assertTrue(can_reuse((0, 128), live, 11))
        self.assertFalse(can_reuse((0, 300), live, 11))
        self.assertTrue(can_reuse((0, 300), live, 12))
        self.assertFalse(overlap((0, 256), (256, 512)))

    def test_transfer_and_roofline_units(self):
        # Binary MiB/GiB for the transfer, decimal GB/s/TFLOP/s for the
        # illustrative roofline. Mixing these conventions silently changes results.
        transfer_seconds = 64 * 2**20 / (16 * 2**30)
        self.assertEqual(transfer_seconds, 1 / 256)
        self.assertEqual(transfer_seconds * 1000, 3.90625)
        peak, bandwidth = 2e12, 200e9
        intensity = 2 / 12
        self.assertEqual(peak / bandwidth, 10)
        self.assertAlmostEqual(min(peak, bandwidth * intensity) / 1e9, 100 / 3)
        # 2*n^3 conventional FLOP and 12*n^2 bytes under ideal input reuse.
        self.assertAlmostEqual((2 * 128**3) / (12 * 128**2), 128 / 6)
        self.assertEqual(max(1 + 2, 3) + 4, 7)  # Ideal U/C versus S overlap.

    def test_storage_precision_and_association(self):
        self.assertEqual(rounded(65504, 'e'), 65504)
        self.assertEqual(rounded(2**-24, 'e'), 2**-24)
        with self.assertRaises(OverflowError):
            rounded(70000, 'e')
        # Ties-to-even: 2049 is between binary16 neighbors 2048 and 2050.
        self.assertEqual(rounded(2048 + 1, 'e'), 2048)
        half_acc = 2048.0
        wide_acc = 2048.0
        for _ in range(8):
            half_acc = rounded(half_acc + 1, 'e')
            wide_acc = rounded(wide_acc + 1, 'f')
        self.assertEqual(half_acc, 2048)
        self.assertEqual(wide_acc, 2056)
        a, b, c = 2**30, -(2**30), 1.0
        left = rounded(rounded(a + b, 'f') + c, 'f')
        right = rounded(a + rounded(b + c, 'f'), 'f')
        self.assertEqual((left, right), (1, 0))
        # This demonstrates explicit storage rounding, not execution of a
        # compiler's fast-math optimization on a GPU.

    def test_matrix_example(self):
        a, b = [[1, 2], [3, 4]], [[5, 6], [7, 8]]
        result = [[sum(a[i][k] * b[k][j] for k in range(2))
                   for j in range(2)] for i in range(2)]
        self.assertEqual(result, [[19, 22], [43, 50]])
        # A scalar oracle knows the result, not a backend's lane-fragment layout.

    def test_invalid_timeline_inputs(self):
        for cpu, gpu, slots in [(0, 1, 3), (2, -1, 3), (2, 6, 0),
                                (float('nan'), 6, 3), (2, float('inf'), 3)]:
            with self.assertRaises(ValueError):
                timeline(cpu, gpu, slots, 1)


if __name__ == '__main__':
    unittest.main(verbosity=2)
```

### Next — Pass 10: M130 + M131 + M132, Parts 2157–2200

That finishes Phase 22: **GPU hash tables and spatial binning, atomic append versus scan construction, indirect buffers and GPU-driven frustum/Hi-Z/LOD pipelines, particle free lists/compaction/sorting, ping-pong and multi-buffering, CPU–GPU pipelining, command processors and asynchronous compute, transfer/upload/readback behavior, persistent/ring buffers and suballocation, transient aliasing, timestamps/profiling zones, then occupancy-versus-throughput, arithmetic intensity and roofline analysis, bandwidth/compute bottlenecks, fast math, FP16, subgroup matrix concepts, race debugging and principled GPU algorithm redesign.**


Once the program has several modules, each module must provide enough information for its consumers to compile and link. A **[[Concepts/Build graph usage requirements|usage requirement]]** is build information a consumer needs because it uses a target: an include directory, a required language feature, a compile definition, or another dependency exposed through the interface.

This is why a build system should express relationships between targets. If `Renderer` exposes a public header that includes a public `Math` header, the consumer must be able to find that header through the build interface. Hiding every relationship in global include paths makes the project appear healthy until a clean build or an independent consumer exposes the missing dependency.

In CMake, `PUBLIC`, `PRIVATE`, and `INTERFACE` describe how requirements apply to the target and its consumers. They are not C++ access specifiers and do not hide header declarations from a determined reader. Exact propagation also depends on the command, target kind, and linking situation; static-library implementation dependencies deserve particular attention when following the final link. Use the [current command documentation](https://cmake.org/cmake/help/latest/command/target_link_libraries.html) when checking a specific build.

A clean experiment is to create a tiny consumer that includes one public engine header and links the advertised target, without adding the engine's internal directories by hand. If that consumer fails, the public build contract is incomplete even when the engine's own tests happen to compile. The later build-tool chapter develops the target graph, presets, toolchain inputs, reproducibility, profiling, and the complete path from source content to a runtime package.

The recurring lesson is that a working program is an agreement across time and boundaries. Bytes need an interpretation. Objects need valid lifetimes. Handles need identities and validity intervals. Asynchronous results need publication rules. Modules need build requirements. Once those agreements are explicit, the graphics and engine stories can depend on them without repeatedly rediscovering the same hidden assumptions.


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/17 - Build, Test and Reproduce the Program|Next chapter]]
