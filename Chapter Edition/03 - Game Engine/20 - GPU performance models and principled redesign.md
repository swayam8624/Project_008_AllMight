# 20 — GPU performance models and principled redesign

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/19 - GPU work and reusable Metal frames|Previous chapter]] · [[Chapter Edition/03 - Game Engine/21 - Buffers, images, memory, and uploads|Next chapter]]

Once the frame is correct, its limiting resource can be measured rather than guessed from a screenshot. This chapter evaluates occupancy, memory traffic, work distribution, and redesign choices against explicit workloads.

**By the end:** Use a performance model to distinguish instruction, bandwidth, occupancy, and scheduling limits, then design a comparable measurement.

**Recall question:** What observation would distinguish a bandwidth limit from an occupancy limit in the same GPU workload?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 43849–47457; source-span SHA256 38cc3311bdaf957926baa5ad6ec73a8539bfb76d302c2fc0c3171bcd72575040. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### PERFORMANCE MODELING & PRINCIPLED GPU REDESIGN

**Occupancy versus instruction throughput**

Pass 9 established:

$$
\text{occupancy}
=
\frac{
\text{resident warps/waves}
}{
\text{maximum}
}.
$$

Now comes the warning:

$$
\boxed{
\text{high occupancy is useful only if it helps keep execution units productive}.
}
$$

Imagine kernel A:

$$
50\%\text{ occupancy}
$$

but every active warp continuously issues FMAs.

Kernel B:

$$
100\%\text{ occupancy}
$$

but half its instructions are spill loads/stores.

Kernel A may be much faster.

---

**Register reduction can make performance worse**

Suppose original kernel uses:

$$
80
$$

registers/thread.

Compiler keeps all important state in registers.

You manually rewrite it to use:

$$
48
$$

registers and occupancy rises.

But now expensive values are recomputed repeatedly or memory accesses increase.

Total work rises.

Occupancy is not the objective.

The objective is:

$$
\boxed{
\text{useful completed work per unit time}.
}
$$

---

**Arithmetic intensity**

Arithmetic intensity is:

$$
\boxed{
I=
\frac{
\text{arithmetic operations}
}{
\text{bytes transferred from limiting memory level}
}.
}
$$

Typically measured:

$$
\text{FLOP/byte}.
$$

A low-intensity kernel spends most of its effort moving data.

A high-intensity kernel reuses data heavily and performs substantial computation per byte.

---

**KairoGPU vector multiply arithmetic intensity**

Per element:

Load:

$$
lhs[i]
$$

= 4 bytes.

Load:

$$
rhs[i]
$$

= 4 bytes.

Store:

$$
out[i]
$$

= 4 bytes.

Total:

$$
12\text{ bytes}.
$$

Arithmetic:

$$
1\text{ floating multiply}.
$$

Therefore:

$$
\boxed{
I=
\frac1{12}
\approx0.0833
\text{ FLOP/byte}.
}
$$

Extremely low arithmetic intensity.

This is almost certainly a memory-bandwidth-oriented workload on any modern GPU.

Making the multiply itself twice as fast will barely help.

---

**Effective bandwidth metric**

If vector multiply processes $N$ elements in $T$ seconds:

$$
\boxed{
B_{\text{effective}}
=
\frac{
12N
}{
T
}.
}
$$

For repeated operations, multiply by iteration count.

This metric is more informative than FLOP/s for such a kernel.

---

**Current KairoGPU benchmark**

Benchmark uses:

$$
N=2^{20}
=
1,048,576
$$

elements.

Per dispatch logical traffic:

$$
12N
=
12,582,912\text{ bytes}
$$

approximately:

$$
12\text{ MiB}.
$$

For:

$$
64
$$

iterations:

$$
768\text{ MiB}
$$

of logical buffer traffic.

But every `VectorMultiplyFloat()` currently commits and waits for GPU completion.

Therefore benchmark includes repeated command submission/synchronization overhead.

It is useful as an end-to-end dispatch benchmark, but it is **not yet a pure kernel-bandwidth benchmark**.

---

**Better benchmark separation**

Measure separately:

$$
T_{\text{submit}},
$$

$$
T_{\text{GPU}},
$$

$$
B_{\text{effective}},
$$

$$
\text{dispatch count}.
$$

Warm the pipeline first.

Batch many dispatches in one command buffer if testing kernel throughput.

Use GPU timestamps rather than CPU wall time for kernel execution.

Then also maintain an end-to-end test because application overhead still matters.

Both measurements answer different questions.

---

**Matrix multiplication arithmetic intensity**

Now examine Kairo's tiled matrix multiply.

Tile width:

$$
T=16.
$$

For one $K$-tile step, workgroup loads:

$$
T^2
$$

elements from A and:

$$
T^2
$$

from B.

Float bytes:

$$
4(2T^2)
=
8T^2.
$$

It computes:

$$
T^3
$$

multiply-adds.

Counting one FMA as two FLOPs:

$$
2T^3.
$$

Arithmetic intensity approximately:

$$
\boxed{
I=
\frac{
2T^3
}{
8T^2
}
=
\frac T4.
}
$$

At:

$$
T=16:
$$

$$
\boxed{
I\approx4\text{ FLOP/byte}.
}
$$

That is roughly:

$$
48\times
$$

the arithmetic intensity of simple vector multiply.

Threadgroup-memory reuse is why.

---

**Why larger tiles can raise intensity**

Formula:

$$
I\approx T/4.
$$

At:

$$
T=32,
$$

theoretical tile intensity becomes around:

$$
8\text{ FLOP/byte}.
$$

But a $32\times32$ group would contain:

$$
1024
$$

threads and require much more shared memory/register state.

Occupancy or hardware group-size limits may collapse.

Hence:

$$
\boxed{
\text{greater data reuse can conflict with occupancy/resources}.
}
$$

This is the kind of multi-variable optimization M132 expects.

---

**Bandwidth-bound kernel**

A kernel is bandwidth-bound when arithmetic units could process more work, but memory cannot feed them fast enough.

Symptoms include:

- measured memory bandwidth near hardware limit,
- low ALU utilization,
- increasing arithmetic throughput doesn't improve runtime,
- reducing bytes improves runtime strongly.

Typical examples:

- vector copy,
- vector addition,
- simple particle state update,
- sparse gather.

---

**Compute-bound kernel**

A kernel is compute-bound when arithmetic pipelines are the limiting resource.

Symptoms:

- high math-unit utilization,
- memory bandwidth below peak,
- reducing arithmetic improves runtime,
- more data reuse no longer matters much.

Examples can include:

- large matrix multiplication,
- expensive procedural shading,
- heavy transcendental math,
- dense numerical kernels.

---

**Roofline model**

Let:

$$
P_{\max}
=
\text{peak compute throughput}
$$

in FLOP/s.

Let:

$$
B_{\max}
=
\text{peak memory bandwidth}
$$

in bytes/s.

For arithmetic intensity:

$$
I
$$

FLOP/byte, memory can supply at most:

$$
B_{\max}I
$$

FLOP/s worth of work.

Therefore attainable performance is bounded by:

$$
\boxed{
P
\le
\min(
P_{\max},
B_{\max}I
).
}
$$

That is the roofline model.

---

**Ridge point**

The two roofs intersect when:

$$
P_{\max}
=
B_{\max}I^*.
$$

Thus:

$$
\boxed{
I^*
=
\frac{
P_{\max}
}{
B_{\max}
}.
}
$$

If:

$$
I<I^*,
$$

kernel is potentially bandwidth bound.

If:

$$
I>I^*,
$$

kernel can potentially become compute bound.

This gives you a numerical way to decide whether to optimize data movement or arithmetic.

---

**Example roofline reasoning**

Imagine hypothetical GPU:

$$
P_{\max}=10\text{ TFLOP/s}
$$

and:

$$
B_{\max}=500\text{ GB/s}.
$$

Ridge intensity:

$$
I^*
=
\frac{
10\times10^{12}
}{
500\times10^9
}
=
20\text{ FLOP/byte}.
$$

Vector multiply:

$$
I=0.083.
$$

Memory roof:

$$
500\times0.083
\approx41.7\text{ GFLOP/s}.
$$

Far below:

$$
10\text{ TFLOP/s}.
$$

Optimizing multiply instruction is pointless.

Your challenge is bandwidth and overhead.

---

**Same hypothetical GPU, tiled matmul**

Kairo's $16\times16$ tiled kernel:

$$
I\approx4.
$$

Memory roof:

$$
500\text{ GB/s}
\times
4
=
2\text{ TFLOP/s}.
$$

Still below:

$$
10\text{ TFLOP/s}.
$$

So even that hypothetical implementation may remain bandwidth-limited unless cache reuse or larger tiles raise effective intensity.

This is why high-end GEMM kernels use sophisticated blocking across multiple memory levels.

---

**Arithmetic intensity is measured at a memory level**

Important subtlety.

A value loaded once from DRAM may then be reused ten times from shared memory.

DRAM arithmetic intensity is high.

Shared-memory arithmetic intensity is different.

Cache behavior changes effective traffic.

Roofline analysis requires specifying the memory level whose bandwidth roof you are modeling.

Advanced performance analysis can use hierarchical rooflines:

$$
L1,
L2,
DRAM.
$$

---

**Shader compiler optimization**

Shader source is not hardware machine code.

Compiler performs things such as:

- constant folding,
- dead-code elimination,
- common-subexpression elimination,
- loop transformations,
- FMA formation,
- instruction scheduling,
- register allocation,
- strength reduction.

Therefore source code that “looks shorter” can generate worse machine code.

And source that appears more complicated can compile better.

---

**Common-subexpression versus register lifetime**

Suppose:

```cpp id="p10-cse"
float expensive =
    f(x);

a =
    expensive * y;

... many instructions ...

b =
    expensive * z;
```

Keeping `expensive` avoids recomputation.

But it remains live for a long time and occupies a register.

Recomputing `f(x)` later may paradoxically improve occupancy if `f` is cheap.

This is an advanced but fundamental compiler/performance tradeoff:

$$
\boxed{
\text{compute versus storage even exists at the register level}.
}
$$

---

**Loop unrolling**

Unrolling:

```text id="p10-unroll"
for i=0..7
```

into eight explicit operations can remove loop-control overhead and expose instruction-level parallelism.

But it increases:

- code size,
- live temporaries,
- register pressure.

A small fixed loop may benefit.

A giant unrolled loop can spill catastrophically.

Never assume `unroll = faster`.

---

**Fast math**

Strict IEEE-like arithmetic preserves semantics around:

- NaN,
- infinity,
- signed zero,
- rounding,
- operation ordering.

Fast-math modes may permit:

- reassociation,
- reciprocal approximations,
- reciprocal-square-root approximations,
- approximate transcendental functions,
- FMA contraction,
- assumptions about finite values.

This can increase speed substantially.

But mathematical identity under real numbers does not imply bitwise identity under floating point.

---

**Reassociation example**

Strict:

$$
(a+b)+c
$$

is not necessarily equal to:

$$
a+(b+c)
$$

in floating point.

Take:

$$
a=10^{20},
\quad
b=-10^{20},
\quad
c=3.14.
$$

First:

$$
(a+b)+c
\approx
0+3.14
=
3.14.
$$

Second:

$$
a+(b+c)
\approx
a-b
\approx0
$$

depending on rounding.

Fast math can permit such reassociation.

For shading this may be harmless.

For deterministic reductions or fragile geometric predicates it may not be.

---

**Reciprocal approximation**

Instead of:

$$
y=\frac{x}{z},
$$

compiler might use approximate reciprocal:

$$
r\approx\frac1z
$$

then:

$$
y=xr.
$$

Likewise normalization may use approximate:

$$
\frac1{\sqrt{x}}
$$

hardware instructions.

These can be excellent for normals, lighting or iterative algorithms tolerant to small error.

They require careful validation in robust intersection/codecs/synchronization counters.

---

**Half precision**

IEEE FP16 has:

$$
1\text{ sign bit},
$$

$$
5\text{ exponent bits},
$$

$$
10\text{ explicit fraction bits}.
$$

Largest finite magnitude:

$$
65504.
$$

Precision is roughly:

$$
3\text{ decimal digits}
$$

depending on magnitude.

It is not “a slightly less precise float.”

Its numerical envelope is dramatically smaller than FP32.

---

**Why FP16 can be fast**

Half precision can reduce memory traffic by half:

$$
4\text{ bytes}
\rightarrow
2\text{ bytes}.
$$

That doubles the number of values fitting in:

- cache,
- shared memory,
- memory transactions.

Many GPUs also have higher native FP16 arithmetic throughput.

So FP16 can help both:

$$
\text{bandwidth}
$$

and:

$$
\text{compute}.
$$

---

**Good FP16 candidates**

Quantities with naturally bounded ranges can work well:

$$
color\in[0,1]\text{ or moderate HDR},
$$

$$
normal\in[-1,1],
$$

$$
roughness\in[0,1],
$$

$$
texture-like intermediate values.
$$

Poor candidates include huge world coordinates or accumulated sums spanning many orders of magnitude.

---

**Mixed precision**

The mature solution is rarely:

> Make the entire renderer FP16.

Instead:

```text id="p10-mixed"
storage: half
load
convert/compute critical terms: float
accumulate: float
store: half
```

or:

$$
FP16\times FP16
\rightarrow
FP32\text{ accumulator}.
$$

This is especially common in matrix hardware.

---

**FP16 Gaussian implications**

Gaussian rendering contains values such as:

- positions,
- log scales,
- rotations,
- spherical harmonic coefficients,
- opacities.

Some may tolerate half precision.

World position may not, especially in large scenes.

Depth sorting keys certainly should not casually become FP16 because ordering precision matters.

Mixed-precision analysis should be per field.

That would be an excellent research/performance experiment for Maveb.

---

**SIMD-group matrix operations**

Modern GPUs often expose specialized cooperative matrix hardware.

Instead of each lane calculating a scalar dot product independently, a subgroup cooperatively multiplies matrix tiles:

$$
D=
AB+C.
$$

Think:

$$
\boxed{
\text{subgroup owns a matrix fragment rather than one scalar output}.
}
$$

Exact APIs differ by platform:

- tensor/matrix cores,
- cooperative matrices,
- subgroup matrix operations.

The key architectural idea is common.

---

**Why matrix units change algorithm design**

Traditional tiled GEMM:

```text id="p10-gemm-old"
load tile
each thread performs scalar FMAs
```

Matrix units:

```text id="p10-gemm-matrix"
load compatible fragments
subgroup matrix multiply-accumulate
store result
```

Throughput can be enormously higher for supported:

- dimensions,
- layouts,
- data types.

But data must be shaped appropriately.

You do not get matrix-unit performance from arbitrary irregular code.

---

**KairoGPU matmul's future path**

Current KairoGPU $16\times16$ threadgroup matrix multiply is a clean educational tiled GEMM.

A later optimized backend could dispatch a capability-dependent matrix implementation when subgroup matrix operations are available.

That should remain behind the same semantic interface:

```cpp id="p10-matmul-interface"
Device::MatrixMultiply(...)
```

so correctness remains independent of the execution path.

This is exactly where Vulkax-style capability querying becomes useful.

---

**Debugging GPU races**

GPU races are especially nasty because output may depend on scheduling.

Typical symptoms:

```text id="p10-race-symptoms"
works in debug
fails in release

works on one GPU
fails on another

adding printf/logging fixes it

random flickering values

failure rate changes with workload
```

These are classic indicators of missing synchronization or overlapping ownership.

---

**Race example**

Wrong:

```cpp id="p10-race"
threadgroup uint sum;

if (local == 0)
    sum = 0;

sum += value;
```

Problems:

Other threads may execute before lane 0 initializes.

Multiple threads perform non-atomic RMW.

Correctness requires synchronization and aggregation design.

A barrier alone after initialization does **not** make `sum += value` atomic.

This distinction is essential:

$$
\boxed{
\text{ordering}
\neq
\text{mutual exclusion/atomicity}.
}
$$

---

**Correct reduction instead of atomic race**

```metal id="p10-correct-reduction"
threadgroup uint scratch[256];

scratch[local] =
    value;

threadgroup_barrier(
    mem_flags::mem_threadgroup);

for (uint stride = 128;
     stride > 0;
     stride >>= 1)
{
    if (local < stride)
    {
        scratch[local] +=
            scratch[
                local + stride];
    }

    threadgroup_barrier(
        mem_flags::mem_threadgroup);
}

if (local == 0)
    output[group] =
        scratch[0];
```

Ownership at each stage is explicit.

Far easier to prove correct.

---

**Never “debug” a race by adding random barriers**

A common reaction:

```text id="p10-barrier-everywhere"
maybe barrier here?
maybe another here?
```

This can:

- accidentally hide the race,
- destroy performance,
- still be wrong,
- deadlock if control flow diverges.

Instead establish:

$$
\boxed{
\text{who writes each location?}
}
$$

$$
\boxed{
\text{who reads it?}
}
$$

$$
\boxed{
\text{what dependency must order those events?}
}
$$

Then choose the minimum correct primitive.

---

**Race-debugging technique: deterministic oracle**

Exactly like your BVH oracle and Vulkax Gaussian scheduler oracle:

Implement a small CPU reference.

For randomized input:

$$
GPU(x)
\stackrel{?}{=}
CPU(x).
$$

Test thousands of seeds/sizes.

Especially test:

$$
0,
1,
W-1,
W,
W+1,
$$

where:

$$
W=\text{workgroup size}.
$$

Parallel bugs love boundary sizes.

---

**Poison and sentinel values**

Initialize output to impossible values:

$$
0xDEADBEEF
$$

or NaNs.

After execution, check whether every required element changed exactly once.

For compaction, validate:

- every output index in range,
- no duplicate IDs unless allowed,
- output count exact,
- stable ordering if promised.

Structural validation often catches race bugs before pixel inspection does.

---

**Designing a GPU algorithm systematically**

Before writing shader code, write down the data model.

What are inputs?

What are outputs?

What is one unit of independent work?

Which outputs have unique writers?

Where do threads need communication?

Where does global synchronization occur?

What bytes are read/written per element?

What branch divergence exists?

What limits occupancy?

Then design kernels.

This prevents “CPU code translated into 100,000 threads.”

---

**A serial CPU visibility algorithm**

Suppose CPU code is:

```cpp id="p10-serial-cull"
std::vector<DrawCommand>
BuildDraws(
    std::span<const Object> objects,
    const Camera& camera)
{
    std::vector<DrawCommand> out;

    for (const Object& object :
         objects)
    {
        if (!InsideFrustum(
                object.bounds,
                camera.frustum))
        {
            continue;
        }

        if (Occluded(
                object.bounds))
        {
            continue;
        }

        const uint32_t lod =
            ChooseLOD(
                object,
                camera);

        out.push_back(
            MakeDrawCommand(
                object,
                lod));
    }

    return out;
}
```

Naively moving loop body to GPU is not the full solution because `push_back()` is a serial dynamic operation.

---

**First GPU translation: atomic append**

```cpp id="p10-atomic-cull"
uint i =
    globalID;

if (i >= objectCount)
    return;

if (!Visible(objects[i]))
    return;

uint dst =
    atomic_fetch_add(
        visibleCount,
        1);

draws[dst] =
    MakeDraw(objects[i]);
```

This may be excellent.

Simple.

One pass.

If visibility is sparse and contention moderate, stop here.

Do not overengineer.

---

**Scalable deterministic translation**

If atomic append becomes bottleneck or deterministic order is required:

Pass A:

$$
visible_i\in\{0,1\}.
$$

Scan:

$$
offset_i=
exclusiveScan(visible).
$$

Pass B:

```cpp id="p10-scan-cull"
if (visible[i])
{
    draws[offset[i]] =
        MakeDraw(objects[i]);
}
```

Count:

$$
drawCount=
offset_{N-1}
+
visible_{N-1}.
$$

Generate indirect args on GPU.

Now every visible object has mathematically unique output ownership.

---

**“More threads” can be slower**

Suppose CPU algorithm processes:

$$
N=128
$$

items.

GPU implementation requires:

- upload,
- dispatch,
- barriers,
- scan,
- second dispatch,
- readback.

CPU may finish before GPU submission overhead alone.

Parallel hardware is useful when:

$$
\boxed{
\text{available parallel work}
\times
\text{work per item}
}
$$

is large enough to amortize overhead.

GPU is not a magical function accelerator.

---

**Serial dependency cannot be solved by launching N threads**

Consider:

$$
x_i=
x_{i-1}+a_i.
$$

Naively:

```cpp id="p10-bad-prefix"
thread i:
    x[i] =
        x[i-1] +
        a[i];
```

All threads have a dependency chain.

Launching them simultaneously creates a race, not a scan.

You must **change the algorithm**—for example to Blelloch's tree structure.

This is the core lesson of Part 2200:

$$
\boxed{
\text{GPU optimization often requires algorithmic reformulation, not parallel syntax}.
}
$$

---

**Amdahl's law**

Suppose fraction:

$$
p
$$

is perfectly parallelizable and fraction:

$$
1-p
$$

remains serial.

With theoretically infinite processors:

$$
\boxed{
S_{\max}
=
\frac1{1-p}.
}
$$

If:

$$
p=0.95,
$$

then:

$$
S_{\max}=20.
$$

Even infinite GPU cores cannot exceed 20× if 5% remains serial.

This directly applies to Maveb's currently serial block-sum/radix-offset stages at sufficiently large workloads.

They may be insignificant today.

But if they become dominant, adding more parallelism to histogram/scatter will not help.

---

**Gustafson-style perspective**

A GPU also lets us scale the problem.

Instead of rendering the same 10,000 Gaussians faster, perhaps process:

$$
10,000,000.
$$

Parallel computing often makes larger feasible workloads possible.

Thus speedup is not the only objective.

Throughput and problem scale matter.

That is particularly relevant for reconstruction/rendering research like Maveb.

---

**Optimization order**

Before modifying code, classify bottleneck.

If bandwidth-bound:

reduce memory traffic or improve locality.

If compute-bound:

reduce arithmetic, use faster operations or specialized units.

If latency-bound due to low occupancy:

reduce registers/shared memory or expose more parallelism.

If synchronization-bound:

redesign ownership/aggregation.

If command-bound:

batch work/reduce dispatch count.

If CPU-GPU synchronization-bound:

remove waits/readbacks.

If divergent:

reorganize work or data.

Without classification, optimization is gambling.

---

**A practical roofline analysis of KairoGPU vector multiply**

Known logical work per element:

$$
F=1\text{ FLOP}.
$$

Logical traffic:

$$
Q=12\text{ bytes}.
$$

Intensity:

$$
I=0.0833.
$$

Measure GPU duration:

$$
T.
$$

Then:

$$
P_{\text{measured}}
=
\frac{
N
}{
T
}
\text{ FLOP/s}
$$

and:

$$
B_{\text{measured}}
=
\frac{
12N
}{
T
}
.
$$

If:

$$
B_{\text{measured}}
$$

is near achievable memory bandwidth, stop optimizing arithmetic.

Possible improvements become:

- fuse neighboring kernels to eliminate intermediate writes,
- operate in-place where legal,
- compress data,
- use half precision if error permits.

---

**Kernel fusion increases arithmetic intensity**

Suppose pipeline:

```text id="p10-fusion-before"
A → kernel1 → temporary B
B → kernel2 → C
```

Kernel 1 writes:

$$
B.
$$

Kernel 2 reads:

$$
B.
$$

If fused:

```text id="p10-fusion-after"
A → fused kernel → C
```

temporary B may stay in registers.

You remove global:

$$
\text{write B}+\text{read B}.
$$

Arithmetic work stays roughly the same.

Bytes fall.

Thus:

$$
\boxed{
I=\frac{FLOPs}{bytes}
}
$$

increases.

But fused kernel may increase registers and reduce occupancy.

Again, every optimization has a shadow.

---

**Kernel fission can also help**

Suppose one giant shader handles:

```text id="p10-mega"
diffuse
glass
metal
skin
hair
clearcoat
volume
debug
```

Register pressure and divergence become enormous.

Splitting into specialized kernels can improve coherence and occupancy even though it increases dispatch overhead and intermediate traffic.

Wavefront path tracing from Pass 8 is exactly this tradeoff.

---

**Performance is a constrained optimization problem**

Think of throughput as constrained by several ceilings:

$$
\boxed{
P
\le
\min(
P_{\text{compute}},
P_{\text{bandwidth}},
P_{\text{occupancy}},
P_{\text{sync}},
P_{\text{front-end}}
).
}
$$

Improving a ceiling that is not currently minimum does nothing.

If memory bandwidth is bottleneck:

doubling ALUs gives:

$$
0\%
$$

speedup.

If command submission is bottleneck:

better shared-memory tiling may barely matter.

This mindset is more important than memorizing any one optimization.

---

**Maveb performance reading from this phase**

Your Gaussian pipeline currently performs:

$$
\text{projection}
\rightarrow
\text{tile counts}
\rightarrow
\text{scan}
\rightarrow
\text{key generation}
\rightarrow
\text{radix histogram}
\rightarrow
\text{prefix}
\rightarrow
\text{scatter}
\rightarrow
\text{tile ranges}
\rightarrow
\text{composite}.
$$

Potential bottleneck classes are very different.

Projection can be arithmetic-heavy.

Key generation can be output-bandwidth-heavy.

Radix sort can be memory-bandwidth/synchronization heavy.

Serial radix-offset stage can become latency/serial-fraction limited.

Composite can become overdraw/bandwidth or arithmetic heavy depending Gaussian occupancy and SH degree.

Do not optimize “the Gaussian renderer” as one monolithic thing.

Profile each phase.

---

**Gaussian occupancy diagnostic is already useful**

The shader has a debug mode visualizing approximately:

$$
\frac{
\text{Gaussians in tile}
}{
64
}.
$$

This tells you spatial load distribution.

A screen region with huge per-tile occupancy implies composite work is highly imbalanced.

That suggests different optimization strategies than a uniform screen.

For example:

- finer tiles,
- early opacity termination,
- workload splitting.

The correct response follows from data distribution.

---

**Early opacity termination as algorithmic work reduction**

Gaussian composite accumulates opacity and stops when:

$$
opacity>0.999.
$$

This is not merely an arithmetic micro-optimization.

If sorted front-to-back for the chosen compositing formulation, opaque-enough pixels can skip many later splats.

The best optimization is often:

$$
\boxed{
\text{do less work}
}
$$

rather than:

$$
\text{do identical work 5% faster}.
$$

---

**Current Kairo/Maveb/Vulkax audit for M130–M132**

| Topic | Current state from inspected code |
|---|---|
| GPU spatial binning | Yes: Gaussian tiles |
| CPU oracle for GPU scheduling | Yes in Vulkax |
| CSR-like counts/offsets/references | Yes by scheduler design |
| Scan-based variable output | Yes |
| Atomic counters | Yes |
| GPU indirect dispatch | Yes |
| GPU-generated dynamic dispatch count | Yes |
| Frustum/Hi-Z GPU culling | Not established by inspected snippets |
| GPU LOD selection | Not established here |
| Particle system/free list | Not established here |
| Ping-pong buffers | Yes: Gaussian radix keys/values |
| Double/triple frame buffering | Yes: Maveb triple frame contexts |
| CPU-GPU frame pipeline | Yes |
| Synchronous demo compute API | KairoGPU currently waits per operation |
| Persistent CPU-visible upload arena | Yes |
| Upload arena capacity | 8 MiB/frame context |
| Readback arena | 256 KiB/frame context |
| Private transient heap | 64 MiB/frame context |
| Full lifetime-based alias allocator | Not demonstrated in inspected snippets |
| Asynchronous frame capture readback | Yes |
| Whole-command-buffer GPU time | Yes |
| Per-pass GPU timestamp zones | Not established |
| CPU profiling scopes | Yes |
| Vector multiply AI | ~0.083 FLOP/byte |
| Tiled matmul AI | ~4 FLOP/byte per tile stage at T=16 |
| FP16/matrix-specialized backend path | Not established |
| Parallel radix | Yes |
| Parallel local scan | Yes |
| Serial block-sum scan | Yes; possible scale bottleneck |
| Serial global radix-offset stage | Yes; possible scale bottleneck |
| Race/correctness validation oracle | Strong in Vulkax scheduler design |

---

**The deepest M130–M132 connection**

Notice the progression:

Pass 9 taught:

$$
scan.
$$

M130 turns scan into:

$$
\text{bin construction},
$$

$$
\text{visible lists},
$$

$$
\text{indirect commands}.
$$

M131 asks:

> How do I keep all of those results GPU-resident and avoid waiting on the CPU?

M132 asks:

> After removing the obvious waits, what physical resource is actually limiting throughput?

This is one continuous chain.

---

**Serial CPU thinking versus GPU systems thinking**

A CPU algorithm often owns one mutable container:

```cpp id="p10-cpu-push"
for (...)
{
    if (...)
        vector.push_back(...);
}
```

GPU thinking asks:

$$
\text{How many outputs does each producer create?}
$$

$$
\text{Can I derive disjoint output ownership?}
$$

$$
\text{Can I avoid CPU visibility?}
$$

$$
\text{Can the GPU construct the next dispatch itself?}
$$

$$
\text{Can data remain resident?}
$$

$$
\text{Can CPU prepare frame }N+1
\text{ while GPU executes }N?
$$

$$
\text{What roof am I hitting afterward?}
$$

That is the architectural mindset Phase 22 is trying to establish.

---

**Reconstruction: build a GPU-driven renderer from first principles**

Start with objects:

$$
O_0,\dots,O_{N-1}.
$$

Each has:

$$
bounds,
mesh,
material,
LOD data.
$$

Compute kernel maps:

$$
\text{thread }i
\leftrightarrow
O_i.
$$

Frustum:

$$
p_i^{F}.
$$

Hi-Z:

$$
p_i^{O}.
$$

Combined:

$$
p_i=
p_i^F p_i^O.
$$

Select LOD:

$$
l_i=f(projectedSize_i).
$$

Compact:

$$
offset_i=
exclusiveScan(p).
$$

If visible:

$$
visible[offset_i]
=
(i,l_i).
$$

Then grouping/sorting can arrange by mesh/material/LOD.

Generate indirect commands.

GPU consumes those commands.

CPU never needs the individual visible set.

That is a GPU-driven pipeline.

---

**Reconstruction: spatial bin construction**

Given object $i$ overlaps bins:

$$
B_i.
$$

Count:

$$
c_i=|B_i|.
$$

Scan:

$$
o_i=
\sum_{j<i}c_j.
$$

Thread $i$ writes:

$$
references[
o_i+k
]
=
(
bin_{ik},
i
).
$$

Then either sort by `bin` or directly construct per-bin counts/offsets depending algorithm.

This is exactly the intellectual pattern behind Gaussian scheduling.

---

**Reconstruction: frame pipeline**

Frame contexts:

$$
F_0,F_1,F_2.
$$

CPU frame:

$$
n
$$

uses:

$$
F_{n\bmod3}.
$$

Before reuse:

$$
wait(
F_{n\bmod3}
\text{ retired}
).
$$

Then reset:

$$
uploadCursor=0,
$$

$$
readbackCursor=0.
$$

Encode.

Submit.

Completion marks slot reusable.

CPU proceeds.

That is how Maveb's current triple-buffered frame structure should be understood.

---

**Reconstruction: choose the optimization from the roof**

Suppose measured kernel has:

$$
I=0.1\text{ FLOP/B}
$$

and already consumes most memory bandwidth.

Do not:

- unroll arithmetic,
- use matrix units,
- chase higher ALU occupancy.

Instead reduce bytes.

Suppose:

$$
I=100\text{ FLOP/B}
$$

and bandwidth utilization is low while compute pipelines are saturated.

Then reduce arithmetic or use faster precision/special units.

Suppose both are low.

Look for:

- synchronization,
- poor occupancy,
- divergence,
- front-end/dispatch overhead,
- dependency stalls.

Roofline starts the diagnosis; it does not end it.

---

**Phase 22 recall sheet**

| Concept | Core idea |
|---|---|
| GPU hash table | flat concurrent key→value structure, often open addressed |
| Load factor | occupied/capacity; high values increase probe divergence |
| Spatial hashing | quantized cell coordinate → hash |
| Spatial binning | objects grouped into spatial cells/tiles |
| Atomic append | simple dynamic output, but contention/order issues |
| Scan construction | count → scan → unique offsets → scatter |
| Indirect args | GPU writes commands later consumed by GPU |
| GPU-driven renderer | GPU decides visibility/LOD/work counts |
| Frustum culling | reject bound outside any frustum plane |
| Hi-Z | hierarchical conservative depth summary |
| Conventional Hi-Z | commonly max depth for conservative region test |
| Reversed-Z Hi-Z | corresponding min-depth hierarchy |
| GPU LOD | choose detail from projected screen size |
| Particle free list | atomic pool of reusable IDs |
| Particle compaction | alive predicate → scan → dense output |
| Ping-pong | algorithm reads old buffer, writes new one |
| Double/triple buffering | duplicate frame resources for overlap |
| CPU-GPU pipeline | CPU prepares future frame while GPU executes prior |
| Command processor | GPU consumes encoded draw/dispatch/copy commands |
| Async compute | potentially overlap compute and graphics |
| Copy engine | independent transfer mechanism where available |
| Upload buffer | CPU-visible staging/persistent memory |
| Readback | GPU→CPU path; immediate wait is dangerous |
| Ring arena | aligned slices reclaimed after GPU retirement |
| Suballocation | many resources from large memory block |
| Dedicated allocation | one allocation for one large/special resource |
| Aliasing | nonoverlapping resource lifetimes reuse physical memory |
| Transient resource | short frame-local lifetime |
| GPU timestamp | timing measured on GPU timeline |
| Pipeline statistics | counters describing amount of processed work |
| Profiling zone | named GPU interval/counter region |
| Occupancy | resident execution groups, not a performance score |
| Arithmetic intensity | FLOPs / transferred bytes |
| Bandwidth bound | memory roof limits throughput |
| Compute bound | arithmetic roof limits throughput |
| Roofline | $P\le\min(P_{\max},B_{\max}I)$ |
| Ridge point | $I^*=P_{\max}/B_{\max}$ |
| Fast math | relaxed floating semantics for throughput |
| FP16 | lower bandwidth/storage and often higher throughput, much less precision |
| Matrix units | subgroup-cooperative tiled matrix MAC hardware |
| GPU race | scheduling-dependent conflicting memory access |
| GPU redesign | change dependency structure, not merely thread the CPU loop |

---

**Blank-page mastery gate**

You should now be able to take a CPU loop that appends visible objects into a vector and produce two correct GPU versions: first an atomic-append implementation and then a deterministic predicate–scan–scatter version, while explaining exactly when each is preferable.

From there, you should be able to explain why reading the visible count back to the CPU before drawing destroys GPU-driven execution, and replace that round trip with an indirect argument buffer.

Then derive a compute frustum test from:

$$
n\cdot C+d<-r,
$$

explain why conventional and reversed-Z Hi-Z pyramids use opposite conservative reductions, and derive a projected-size LOD metric proportional to:

$$
\frac{fr}{z}.
$$

You should be able to distinguish algorithmic ping-ponging from double/triple frame buffering, reconstruct Maveb's three-frame semaphore/context system, and explain why resetting a frame's upload cursor is only safe after that frame slot has retired on the GPU.

Then take KairoGPU's vector multiply and derive:

$$
I=
\frac1{12}
\approx0.083
\text{ FLOP/B}.
$$

Take its tiled matrix multiply and derive:

$$
I\approx\frac T4,
$$

therefore:

$$
I_{T=16}\approx4.
$$

Then derive the roofline:

$$
\boxed{
P
\le
\min(
P_{\max},
B_{\max}I
)
}
$$

and the ridge point:

$$
\boxed{
I^*=
P_{\max}/B_{\max}.
}
$$

You should be able to explain why reducing register count can make a shader slower, why FP16 can improve both storage and arithmetic throughput but destroy world-space precision, why a subgroup matrix operation requires restructuring data into matrix fragments, and why adding a barrier does not repair an atomicity bug.

Finally, reconstruct the complete Maveb Gaussian compute chain from the hardware perspective:

$$
\text{project}
\rightarrow
\text{count tile references}
\rightarrow
\text{parallel scan}
\rightarrow
\text{GPU-generated indirect count}
\rightarrow
\text{generate key/value records}
\rightarrow
\text{subgroup histogram}
\rightarrow
\text{prefix}
\rightarrow
\text{stable radix scatter}
\rightarrow
\text{tile ranges}
\rightarrow
\text{composite}.
$$

Then identify its likely scaling frontiers:

$$
\text{serial block-sum scan},
$$

$$
\text{serial radix group-offset computation},
$$

$$
\text{memory traffic from repeated radix passes},
$$

$$
\text{per-tile compositing imbalance}.
$$

That is the final Phase-22 skill: not merely knowing GPU algorithms, but being able to look at a real GPU pipeline and say **which resource, dependency or data movement pattern is likely to become the next limit—and redesign the algorithm accordingly**.




We now enter **Phase 23: Vulkan — Objects, Resources, Descriptors, Commands, Synchronization, Presentation, and Profiling**. Pass 11 will go from Vulkan's explicit-control philosophy through `VkInstance`, physical/logical devices, queues and queue families, swapchains/surface capabilities, buffers/images/memory heaps and memory types, staging and mapping, image layouts and views, samplers, descriptor sets/layouts/pools, push constants and pipeline layouts, shader modules/SPIR-V, graphics/compute pipeline construction and pipeline caches, command pools/buffers, primary/secondary command buffers, and command recording—all tied directly against your current Kairo Vulkan implementation rather than taught as disconnected API calls.


## Open the device and describe its storage

**Clarification before the allocation examples:** device-local and host-visible are independent memory properties and can coexist. The explicit-execution narrative above defines the distinction.

### + M135

This pass begins **Phase 23: Vulkan — Objects, Resources, Descriptors, Commands, Synchronization, Presentation, and Profiling**. The exact ownership is: **M133 = Parts 2201–2220**, covering Vulkan's explicit design, instance/device/queues/surface/swapchain; **M134 = Parts 2221–2243**, covering buffers, images, memory, staging and resource classes; and **M135 = Parts 2244–2264**, covering descriptors, push constants, pipeline layouts, SPIR-V, graphics pipeline state, pipeline caches, dynamic rendering versus render passes/subpasses, and attachment load/store semantics.

The original curriculum sequence confirms the boundary: swapchain recreation ends at Part 2220, Vulkan resources/memory occupy 2221–2243, descriptors begin at 2244, and Part 2264 is attachment load/store. **Command pools begin at 2265, so command-buffer lifecycle belongs to Pass 12 rather than this pass.**

The important mental change is that Vulkan does not try to infer what you meant.

It asks you to state it.

That is the entire API philosophy.

---

### VULKAN PHILOSOPHY, INSTANCE, DEVICES, QUEUES, SURFACE & SWAPCHAIN

**Vulkan's central philosophy**

Older high-level graphics APIs tend to maintain substantial hidden state.

You might say:

```cpp
BindTexture(...);
Draw(...);
```

and the driver internally figures out:

- resource transitions,
- state validation,
- pipeline compilation,
- synchronization,
- memory placement,
- command batching.

Vulkan moves much of that responsibility toward the engine.

Conceptually:

$$
\boxed{
\text{more programmer responsibility}
\Longrightarrow
\text{less runtime guesswork}
}
$$

The intended advantages are:

$$
\boxed{
\text{predictability}
+
\text{parallel command construction}
+
\text{explicit synchronization}
+
\text{lower driver overhead}
}
$$

but the cost is also real:

$$
\boxed{
\text{more objects}
+
\text{more lifetime management}
+
\text{more ways to be subtly wrong}.
}
$$

---

**Vulkan is not “OpenGL with more parameters”**

The API is architecturally different.

OpenGL-like thinking:

```text
global context state
→ issue command
→ driver interprets current state
```

Vulkan thinking:

```text
construct immutable-ish state objects
→ explicitly bind resources/state
→ record commands
→ explicitly submit
→ explicitly synchronize
```

That is why a Vulkan renderer tends to have objects such as:

```text
Instance
PhysicalDevice
Device
Queue
Surface
Swapchain
Image
ImageView
Buffer
DeviceMemory
DescriptorSetLayout
DescriptorPool
DescriptorSet
PipelineLayout
Pipeline
RenderPass
Framebuffer
```

Each object corresponds to a real contract with the API.

---

**The Vulkan object hierarchy**

A useful ownership graph is:

```text
VkInstance
    ↓
VkPhysicalDevice
    ↓
VkDevice
    ↓
VkQueue

VkInstance
    ↓
VkSurfaceKHR
    ↓
VkSwapchainKHR
        ↓
    swapchain VkImage objects
        ↓
    VkImageView objects

VkDevice
    ↓
Buffers / Images
    ↓
VkDeviceMemory

VkDevice
    ↓
Descriptor layouts / pools / sets
    ↓
Pipeline layout
    ↓
Graphics / compute pipelines
```

Not all arrows imply strict C++ ownership, but they express API dependency.

Destroying things in the wrong order can violate object lifetime rules.

---

**`VkInstance`**

`VkInstance` is the connection between your application and the Vulkan implementation.

It does **not** represent the GPU.

It exists before physical-device selection.

You provide metadata:

```cpp
VkApplicationInfo app{};
app.sType =
    VK_STRUCTURE_TYPE_APPLICATION_INFO;

app.pApplicationName =
    "Kairo";

app.pEngineName =
    "Kairo";

app.apiVersion =
    VK_API_VERSION_1_1;
```

Then:

```cpp
VkInstanceCreateInfo create{};
create.sType =
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

create.pApplicationInfo =
    &app;
```

and:

```cpp
vkCreateInstance(
    &create,
    nullptr,
    &instance);
```

Kairo currently targets:

$$
\boxed{\text{Vulkan 1.1}}
$$

in `VulkanInstance.cppm`.

---

**Why instance extensions exist**

The Vulkan core cannot know every platform/window-system integration mechanism.

So optional capabilities are exposed through extensions.

For example, creating a drawable surface requires platform-specific window-system support.

GLFW provides the required instance extension names.

Kairo takes those surface extensions and passes them into instance creation.

This is exactly the correct architecture:

```text
Window system
    ↓
required Vulkan instance extensions
    ↓
VkInstance
```

---

**Extension discovery is mandatory**

You should not blindly enable an extension because your development machine supports it.

The general pattern is:

```cpp
uint32_t count = 0;

vkEnumerateInstanceExtensionProperties(
    nullptr,
    &count,
    nullptr);

std::vector<VkExtensionProperties>
extensions(count);

vkEnumerateInstanceExtensionProperties(
    nullptr,
    &count,
    extensions.data());
```

Then test by name.

Kairo's `HasExtension()` does exactly this.

---

**MoltenVK portability enumeration**

This matters specifically because you are developing on macOS.

Vulkan is not natively exposed by macOS in the same way Metal is.

MoltenVK implements Vulkan over Metal.

Portable Vulkan implementations may expose devices through:

$$
VK_KHR_portability_enumeration.
$$

Kairo checks for:

```cpp
VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
```

and if present adds:

```cpp
VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
```

to the instance flags.

Without this, physical-device enumeration through portability implementations may not behave as expected.

This is a particularly important cross-platform detail in your current setup.

---

**Validation layers**

Vulkan deliberately performs relatively little runtime validation in normal production operation.

Incorrect usage may produce:

- validation errors,
- undefined results,
- GPU hangs,
- device loss,
- silent corruption.

During development, use:

$$
\boxed{
VK_LAYER_KHRONOS_validation
}
$$

when available.

Kairo checks whether the validation layer exists before enabling it.

Good.

---

**Validation is not the graphics driver**

Validation layers sit between your application and implementation.

Conceptually:

```text
application
    ↓
validation layers
    ↓
Vulkan implementation
    ↓
driver / GPU
```

They inspect API use and report mistakes such as:

- wrong object lifetime,
- missing synchronization,
- invalid descriptor bindings,
- layout mismatches,
- wrong render-pass compatibility,
- out-of-range API parameters.

They do not make invalid code valid.

They diagnose it.

---

**Kairo validation nuance**

Kairo currently enables validation only when both:

```text
VK_LAYER_KHRONOS_validation
```

and:

```text
VK_EXT_debug_utils
```

are available.

That makes sense for a debug-capable configuration.

Later M138 will go deeper into debug messengers and diagnostic naming.

---

**Physical devices**

A:

$$
VkPhysicalDevice
$$

represents a physical or logical Vulkan-capable adapter exposed by the implementation.

Examples could correspond to:

- discrete GPU,
- integrated GPU,
- software implementation,
- portability device.

You do not create a `VkPhysicalDevice`.

You enumerate it:

```cpp
uint32_t count = 0;

vkEnumeratePhysicalDevices(
    instance,
    &count,
    nullptr);
```

Then fetch handles.

Kairo does exactly this.

---

**Physical-device selection should be capability driven**

A common tutorial mistake is:

```cpp
device = devices[0];
```

Kairo correctly does not assume the first device works.

It checks each candidate for:

$$
\text{graphics queue}
$$

and:

$$
\text{presentation support}
$$

and:

$$
VK_KHR_swapchain
$$

and:

$$
independentBlend.
$$

Only then does it select the adapter.

That is much better than hardcoding enumeration order.

---

**Device properties versus device features**

These are different concepts.

**Properties** describe characteristics/limits:

```text
device name
device type
vendor/device IDs
maximum image dimensions
maximum descriptor counts
timestamp period
alignment limits
subgroup characteristics
```

These are mostly facts about the implementation.

**Features** are capabilities that may need to be explicitly enabled:

```text
geometry shaders
sampler anisotropy
independent blend
wide lines
fill mode non-solid
```

You query support before device creation, then enable selected features when creating the logical device.

---

**`vkGetPhysicalDeviceProperties`**

General usage:

```cpp
VkPhysicalDeviceProperties props{};

vkGetPhysicalDeviceProperties(
    physical,
    &props);
```

Then:

```cpp
props.deviceName;
props.deviceType;
props.limits.maxImageDimension2D;
```

These limits should feed engine capability decisions.

Do not assume desktop-NVIDIA-style limits on every backend.

---

**`vkGetPhysicalDeviceFeatures`**

Kairo currently does:

```cpp
VkPhysicalDeviceFeatures
supported{};

vkGetPhysicalDeviceFeatures(
    candidate,
    &supported);
```

and specifically requires:

```cpp
supported.independentBlend ==
VK_TRUE
```

because its viewport writes multiple attachments whose blend behavior differs.

That is a great example of feature selection being driven by renderer semantics rather than by “enable everything.”

---

**Why enabling every feature is poor practice**

The logical device declares the feature contract your application intends to use.

A feature should generally be enabled because some engine path requires it.

That makes:

- portability clearer,
- compatibility checks easier,
- fallback design explicit.

Kairo enables:

$$
\boxed{
independentBlend
}
$$

because object-ID and color attachments need distinct blending behavior.

That is disciplined capability use.

---

**Queue families**

A physical device exposes one or more queue families.

Each family supports some subset of:

$$
VK_QUEUE_GRAPHICS_BIT,
$$

$$
VK_QUEUE_COMPUTE_BIT,
$$

$$
VK_QUEUE_TRANSFER_BIT,
$$

and possibly other operations.

Presentation support is surface-specific and queried separately.

This is important:

$$
\boxed{
\text{presentation capability is not merely a queue flag}.
}
$$

You ask:

```cpp
vkGetPhysicalDeviceSurfaceSupportKHR(
    physical,
    family,
    surface,
    &supported);
```

Kairo correctly does this.

---

**Graphics, compute and transfer queues**

A graphics-capable queue generally supports graphics work and often compute/transfer too.

A compute-only queue may exist.

A transfer-only queue may exist.

Why care?

Dedicated compute/transfer families can sometimes allow:

- concurrent work,
- lower contention,
- specialized scheduling.

But they also introduce:

- queue-family ownership transfers,
- more synchronization,
- more scheduling complexity.

Therefore:

$$
\boxed{
\text{dedicated queue}
\neq
\text{automatic performance win}.
}
$$

---

**Kairo's current queue model**

Kairo finds:

$$
\boxed{
\text{one graphics family}
}
$$

and:

$$
\boxed{
\text{one present family}.
}
$$

These may be the same or different.

It does **not** yet separately search for:

- dedicated compute family,
- dedicated transfer family.

That is reasonable for the renderer's current scope.

General Vulkan engine design should still understand those distinctions.

---

**Queue-family selection heuristic**

A more advanced engine may conceptually prefer:

```text
graphics:
    family with GRAPHICS

compute:
    COMPUTE but preferably not GRAPHICS

transfer:
    TRANSFER but preferably neither GRAPHICS nor COMPUTE
```

because dedicated families may map to specialized hardware queues.

But if no dedicated queue exists:

$$
\boxed{
\text{fallback to shared graphics family}.
}
$$

Capability-driven fallback is essential.

---

**Logical device**

`VkDevice` is your application's logical interface to one selected physical device.

You tell Vulkan:

- which queue families you need,
- how many queues,
- which device extensions,
- which features.

Kairo builds one `VkDeviceQueueCreateInfo` per unique graphics/present family.

If both families are identical, only one queue-family request is created.

That avoids redundant queue-create entries.

---

**Device queues**

After device creation:

```cpp
vkGetDeviceQueue(
    device,
    graphicsFamily,
    0,
    &graphicsQueue);
```

and similarly for present queue.

A `VkQueue` is where commands are eventually submitted.

Do not think of it as a CPU thread.

It is a logical submission channel into GPU execution.

---

**Multiple queues from one family**

A family may expose:

$$
queueCount>1.
$$

You can request multiple logical queues.

That does not necessarily imply completely independent physical engines.

The implementation may schedule them onto shared hardware.

Again:

$$
\boxed{
\text{logical concurrency}
\neq
\text{guaranteed hardware parallelism}.
}
$$

---

**Window-system surfaces**

`VkSurfaceKHR` connects Vulkan presentation to a native window.

It belongs conceptually to:

$$
VkInstance.
$$

It does not itself contain rendered pixels.

It describes:

> the window-system target to which swapchain images may eventually be presented.

Kairo uses:

```cpp
glfwCreateWindowSurface(...)
```

and wraps the resulting handle in RAII.

---

**Surface lifetime**

Kairo stores:

```cpp
VkInstance m_Instance;
VkSurfaceKHR m_Handle;
```

and destroys with:

```cpp
vkDestroySurfaceKHR(...)
```

before the instance disappears.

This is exactly the sort of explicit lifetime dependency Vulkan makes visible.

---

**Surface capabilities**

Before creating a swapchain, query:

```cpp
VkSurfaceCapabilitiesKHR caps{};

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physical,
    surface,
    &caps);
```

This gives information such as:

$$
minImageCount,
$$

$$
maxImageCount,
$$

$$
currentExtent,
$$

$$
minImageExtent,
$$

$$
maxImageExtent,
$$

$$
currentTransform.
$$

Swapchain configuration must fit these limits.

---

**Swapchain extent**

Sometimes:

```cpp
capabilities.currentExtent
```

already specifies the required extent.

Otherwise the application selects one constrained by:

$$
[minImageExtent,maxImageExtent].
$$

Kairo correctly does:

$$
width=
clamp(
framebufferWidth,
minWidth,
maxWidth
)
$$

and similarly for height.

---

**Why framebuffer size rather than logical window size matters**

On HiDPI displays:

$$
\text{window points}
\neq
\text{framebuffer pixels}.
$$

Rendering targets need physical framebuffer dimensions.

Using logical window dimensions can produce scaling bugs.

GLFW exposes framebuffer extent separately for this reason.

---

**Surface formats**

A surface exposes supported:

$$
(format,colorSpace)
$$

pairs.

Kairo prefers:

```text
VK_FORMAT_B8G8R8A8_SRGB
+
VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
```

and otherwise falls back to first available format.

This matters because swapchain images are display-facing resources.

---

**sRGB swapchain format**

With an sRGB format, color written from the shader is treated as linear input to the attachment and encoded appropriately for storage/display behavior according to the API's sRGB rules.

That is why your lighting should remain in linear space before final output.

This connects directly to Pass 3.

---

**Present modes**

Important present modes conceptually include:

$$
FIFO,
$$

$$
MAILBOX,
$$

$$
IMMEDIATE
$$

depending on platform.

`FIFO` behaves like queued vsync and is the universally required safe mode.

`MAILBOX` keeps a pending image model that can reduce latency without tearing when supported.

Kairo:

$$
\boxed{
MAILBOX\text{ if available}
}
$$

otherwise:

$$
\boxed{
FIFO.
}
$$

A sensible choice.

---

**Swapchain creation**

Swapchain is created from:

```cpp
VkSwapchainCreateInfoKHR
```

with:

- surface,
- image count,
- format,
- color space,
- extent,
- image usage,
- sharing mode,
- transform,
- present mode.

Kairo requests:

```cpp
VK_IMAGE_USAGE_TRANSFER_DST_BIT |
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
```

which means swapchain images can be rendering destinations and transfer destinations.

---

**Swapchain image count**

Kairo chooses:

$$
imageCount=
minImageCount+1
$$

then clamps to `maxImageCount` if nonzero.

The intuition is to request one additional image beyond the minimum to reduce chances that rendering blocks waiting for the presentation engine.

This is related to buffering, but swapchain image count is not exactly equivalent to your own frames-in-flight count.

Keep those concepts separate.

---

**Graphics and present queue-family sharing**

If graphics and present families are the same:

$$
VK_SHARING_MODE_EXCLUSIVE.
$$

If different, Kairo uses:

$$
VK_SHARING_MODE_CONCURRENT
$$

with both family indices.

Concurrent sharing avoids explicit ownership transfer between those families.

Potential cost is less optimized ownership assumptions.

An advanced renderer may choose exclusive mode plus explicit transfers, but concurrent mode simplifies the implementation.

---

**Swapchain images are not allocated by your code**

After creation:

```cpp
vkGetSwapchainImagesKHR(...)
```

returns image handles.

These images are owned by the swapchain implementation.

You do not call `vkDestroyImage()` on them.

You do, however, usually create image views for them.

Kairo does exactly that.

---

**Swapchain image views**

For every swapchain image, Kairo creates:

```cpp
VkImageViewCreateInfo view{};
view.image =
    image;

view.viewType =
    VK_IMAGE_VIEW_TYPE_2D;

view.format =
    swapchainFormat;
```

with color aspect.

Why image view?

Because many APIs do not bind raw image objects directly into render operations.

A view tells Vulkan:

> interpret this image using this format/subresource range/aspect.

The resource and its interpretation are separated.

---

**Acquiring a swapchain image**

Before rendering to a presentable image:

```cpp
vkAcquireNextImageKHR(...)
```

returns an image index.

This does not mean:

> CPU now owns the image forever.

It establishes presentation/rendering synchronization semantics through the provided semaphore/fence.

Kairo correctly handles:

$$
VK_ERROR_OUT_OF_DATE_KHR.
$$

It also permits:

$$
VK_SUBOPTIMAL_KHR
$$

as a usable-but-recreate-soon result.

---

**`VK_ERROR_OUT_OF_DATE_KHR`**

This commonly occurs when:

- window resized,
- display configuration changed,
- surface no longer matches swapchain configuration.

The existing swapchain cannot be used.

Kairo responds by recreating it.

Correct.

---

**`VK_SUBOPTIMAL_KHR`**

The swapchain can still function, but no longer perfectly matches surface conditions.

For example the surface characteristics may have changed.

The frame can often complete, then swapchain gets recreated.

Kairo treats this as recreation-worthy.

---

**Presenting**

After graphics work has produced the image, presentation uses:

```cpp
VkPresentInfoKHR present{};
```

then:

```cpp
vkQueuePresentKHR(
    presentQueue,
    &present);
```

Presentation is queue work.

It must wait for rendering completion through appropriate synchronization.

The exact semaphore mechanics come in Pass 12.

---

**Swapchain recreation**

Current Kairo's `Recreate()` does:

```cpp
vkDeviceWaitIdle(device);

Destroy();

Create(window);
```

This is simple and correct for its current one-frame-oriented architecture.

But it stalls the entire device.

For a more advanced multi-frame engine, you can create a replacement swapchain while old resources retire after fences/timeline values.

Kairo's source itself comments that a future multi-frame scheduler could do this.

Excellent comment.

---

**Resources depending on the swapchain must also be rebuilt**

When Kairo recreates the swapchain, it also recreates:

$$
\text{synchronization resources}
$$

and:

$$
\text{triangle renderer swapchain-dependent state}.
$$

Why?

Because framebuffers and render-pass compatibility may depend on:

- extent,
- format,
- image views.

A swapchain resize is not merely “replace one handle.”

It is a dependency-tree rebuild.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/19 - GPU work and reusable Metal frames|Previous chapter]] · [[Chapter Edition/03 - Game Engine/21 - Buffers, images, memory, and uploads|Next chapter]]
