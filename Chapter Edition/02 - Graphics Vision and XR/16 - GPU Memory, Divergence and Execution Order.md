# 16 — GPU Memory, Divergence and Execution Order

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/15 - Rasterization Foundations and Renderer Evidence|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/17 - Build, Test and Reproduce the Program|Next chapter]]

GPU performance depends on memory traffic, divergent execution and synchronization as well as arithmetic. The chapter links these hardware behaviors to subgroup operations and ordering constraints.

**By the end:** Reason about memory access, divergence and synchronization in GPU programs.

**Recall question:** How can divergent control flow reduce subgroup efficiency?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 37332–40668; source-span SHA256 97747b6c97e94bf083c6a849b1d49ce42a8620679d0c96f200ff99ada6dbc21d. -->

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
