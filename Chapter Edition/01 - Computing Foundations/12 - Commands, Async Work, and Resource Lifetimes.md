# 12 — Commands, Async Work, and Resource Lifetimes

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|Next chapter]]

GPU command processors execute submitted work asynchronously, so recording a command is not the same as completing it. This chapter connects queue submission, synchronization, memory lifetime, and scheduling to frame-level performance choices.

**By the end:** Model when commands are recorded, submitted, executed, and retired, and use synchronization only at real dependency boundaries.

**Recall question:** How does a completion signal change what the CPU may safely read, overwrite, or destroy?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 22552–25730; source-span SHA256 42409f3d01bae1cf39b63ee62f06a72093a3be2f426b615a4f5d8274c3fe89c2. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### COMMAND EXECUTION, ASYNC WORK & MEMORY LIFETIMES

**GPU command processor**

Your CPU does not directly call an ALU and say:

> Run this shader now.

It records command data describing:

- pipeline state,
- resources,
- dispatch/draw dimensions,
- copies,
- barriers.

The GPU command processor/front-end consumes those commands and schedules work onto execution engines.

Therefore there are at least two parallel timelines:

$$
\boxed{
\text{CPU command generation}
}
$$

and:

$$
\boxed{
\text{GPU command execution}.
}
$$

A well-designed renderer prevents unnecessary synchronization between them.

---

**Submission overhead**

Imagine issuing 100,000 tiny dispatches each doing 20 arithmetic instructions.

Even if GPU arithmetic is fast, command encoding, validation, queue processing and state transitions become significant.

Hence GPU work should usually have enough granularity.

“Use more dispatches” is not free.

This is one reason kernels sometimes fuse adjacent stages.

But excessive fusion increases register pressure and divergence.

Once again:

$$
\boxed{\text{tradeoff}.}
$$

---

**KairoGPU's current compute wrapper is intentionally synchronous**

Current Kairo Metal vector operation performs:

```objc id="p10-kairo-sync"
command = queue.commandBuffer();
encode();
command.commit();
command.waitUntilCompleted();
```

for every vector add/multiply call.

That is excellent for a simple deterministic wrapper and unit-testable API.

But it destroys CPU–GPU overlap.

The caller cannot enqueue:

$$
64
$$

operations and let the GPU work asynchronously.

The CPU waits after each one.

That distinction is essential when interpreting its benchmark.

---

**Asynchronous compute**

Async compute means compute work can be scheduled independently of graphics work, potentially overlapping.

Conceptually:

```text id="p10-async"
Graphics: [shadow][gbuffer]------[lighting]
Compute :        [particles][SSAO]
```

If resource dependencies permit and hardware has available execution capacity, some work can overlap.

But:

$$
\boxed{
\text{asynchronous submission does not guarantee physical parallel execution}.
}
$$

---

**Why graphics + compute can fail to speed up**

Suppose graphics already saturates:

- shader ALUs,
- memory bandwidth,
- register capacity.

Launching compute concurrently may merely make both workloads compete.

Frame may become slower.

Async compute is valuable when workloads complement each other.

For example one phase may be raster/backend heavy while compute uses spare arithmetic capacity.

Or one workload may stall on memory while another has independent arithmetic.

You profile.

---

**Resource dependencies constrain overlap**

Suppose compute writes buffer:

$$
B.
$$

Graphics immediately reads:

$$
B.
$$

Then:

$$
Compute(B)
\rightarrow
Graphics(B)
$$

is a real dependency.

No amount of async queue magic removes it.

To overlap, find independent work:

$$
Compute(A)
$$

while:

$$
Graphics(B)
$$

runs.

Dependency graphs, not queue count, determine possible overlap.

---

**Transfer engines / copy queues**

Many GPUs provide dedicated mechanisms for:

- buffer copies,
- texture copies,
- uploads,
- readbacks.

On discrete GPUs these may correspond to separate DMA/copy engines.

Using them can allow copying while graphics/compute continue.

On Apple unified-memory systems, the physical memory model differs, but blit/copy encoders and resource synchronization still matter.

Do not blindly transfer discrete-GPU assumptions to unified memory.

---

**CPU→GPU upload bandwidth**

On a discrete GPU, uploads typically cross a PCIe-like interconnect.

If every frame uploads:

$$
1\text{ GB}
$$

at 60 FPS:

$$
60\text{ GB/s}
$$

of host→device traffic would be required before considering anything else.

That can exceed the interconnect budget.

Hence:

$$
\boxed{
\text{keep persistent data resident on the GPU}.
}
$$

Upload only changes.

---

**Apple unified memory changes the topology, not the optimization problem**

CPU and GPU can share physical memory.

That can avoid an explicit host→VRAM copy.

But memory traffic is still finite.

Caches still matter.

Synchronization still matters.

Writing massive CPU buffers that the GPU immediately streams still consumes DRAM bandwidth.

Unified memory removes one kind of transfer bottleneck; it does not create infinite bandwidth.

---

**GPU→CPU readback stalls**

This is among the easiest ways to destroy a renderer.

Bad pattern:

```cpp id="p10-bad-read"
DispatchGPU();

WaitForGPU();

count =
    ReadBuffer();

SubmitNextWork(count);
```

CPU stalls until GPU finishes.

GPU may then stall waiting for CPU to submit subsequent work.

The pipeline collapses.

---

**Indirect buffers are one answer to readback**

Instead:

```text id="p10-gpu-chain"
GPU creates count
GPU creates indirect args
GPU consumes indirect args
```

Read the result back only when the CPU actually needs it, possibly several frames later.

Maveb's radix dispatch does exactly this.

This is why indirect execution and pipelining are deeply connected topics.

---

**Asynchronous readback**

If CPU genuinely needs GPU data, copy into a CPU-visible readback buffer and process after completion.

Current Maveb frame capture does this correctly.

It encodes texture→buffer copy.

Then registers a command-buffer completion handler.

Only when GPU work has completed does the handler access buffer contents and construct the CPU capture.

The main frame function does not call:

```cpp id="p10-no-wait"
waitUntilCompleted()
```

for every screenshot copy.

That preserves normal frame pipelining.

---

**Persistent mapped upload buffers**

The inefficient model:

```text id="p10-map-everything"
create/map buffer
write 64 bytes
unmap
submit

create/map another buffer
...
```

causes allocation/mapping overhead.

Instead allocate a large CPU-visible upload buffer once and keep its CPU pointer available.

Every frame allocate slices:

$$
[offset,offset+size).
$$

Write directly.

Bind GPU buffer + offset.

This is persistent mapped arena allocation.

---

**Maveb FrameContext does exactly this**

Each frame context owns an 8 MiB `StorageModeShared` upload buffer.

Allocation:

$$
offset=
alignUp(cursor,alignment).
$$

Then:

$$
cursor\leftarrow offset+size.
$$

Returned slice contains:

```cpp id="p10-slice"
buffer
offset
size
cpuAddress
```

Default alignment:

$$
256\text{ bytes}.
$$

CPU writes directly through the shared buffer's `contents()` pointer.

No per-object Metal buffer creation.

Very good.

---

**It is technically a per-frame bump arena**

The source labels it an `"Upload Ring"`.

Inside one `FrameContext`, allocation is currently:

$$
\boxed{
\text{monotonic cursor reset at beginFrame()}
}
$$

rather than a circular buffer with independent head and tail.

The **three FrameContexts together form the frame ring**.

Slot 0's cursor is reset only when its frame context becomes reusable after GPU completion.

This architecture is simpler than managing one giant circular arena with arbitrary in-flight ranges.

---

**Ring buffer principle**

For a true single-buffer ring, maintain:

$$
head
$$

for new allocations and:

$$
tail
$$

for oldest GPU-in-use data.

Never let head overtake live tail.

If allocation reaches buffer end:

$$
head\rightarrow0
$$

only if the wrapped range is no longer in flight.

This requires fences/timeline tracking.

Per-frame arenas avoid much of that complexity.

---

**Readback arena**

Maveb also allocates a:

$$
256\text{ KiB}
$$

shared readback buffer per frame context.

Same aligned bump mechanism.

That provides bounded CPU-visible storage for GPU results without creating one tiny readback resource per query.

This is exactly the same allocation principle in the opposite data direction.

---

**GPU memory suballocation**

Suppose you need:

$$
10,000
$$

small 4 KiB GPU buffers.

Requesting 10,000 dedicated GPU memory allocations can be expensive and fragmented.

Instead reserve a large block:

$$
256\text{ MiB}
$$

and allocate slices from it.

Resource:

$$
(block,\ offset,\ size).
$$

This is suballocation.

---

**Dedicated allocations versus suballocation**

Dedicated allocation is attractive for:

- enormous textures/buffers,
- special alignment/tiling requirements,
- resources whose lifetime spans most of the application,
- allocations where backend recommends/requires dedication.

Suballocation is excellent for:

- many small resources,
- dynamic/transient resources,
- predictable lifetime pools.

A serious allocator usually supports both.

---

**Internal and external fragmentation**

Internal fragmentation:

You allocate 300 bytes from a 512-byte block.

Unused:

$$
212\text{ bytes}.
$$

External fragmentation:

There is enough total free memory, but it exists in disconnected chunks.

For example:

```text id="p10-frag"
free 10 MB
used 5 MB
free 10 MB
used 5 MB
free 10 MB
```

Total free:

$$
30\text{ MB}.
$$

A contiguous 25 MB allocation cannot fit.

Allocator design matters in long-running engines.

---

**Common suballocators**

A linear/bump allocator is ideal for:

$$
\text{reset-whole-pool-at-once}
$$

transient data.

A free-list allocator handles arbitrary lifetimes.

Buddy allocation makes split/merge efficient while restricting block sizes to powers of two.

Slab/pool allocators are excellent when many objects have equal size.

There is no single GPU allocator for every lifetime class.

---

**Resource aliasing**

Suppose texture A is only used during shadow generation.

Texture B is only used much later during bloom.

Their lifetimes do not overlap.

Instead of allocating:

$$
Memory(A)+Memory(B),
$$

the renderer may place both resources in the same physical region at different times.

That is resource aliasing.

---

**Lifetime interval view**

Represent resource lifetime as:

$$
[l_i,r_i].
$$

Two resources may alias if:

$$
[l_A,r_A]
\cap
[l_B,r_B]
=
\varnothing.
$$

This turns transient resource allocation into an interval-allocation problem.

A frame graph can compute lifetimes automatically.

---

**Aliasing requires synchronization discipline**

Memory reuse is legal only after prior resource use has truly ended.

Otherwise:

```text id="p10-alias-bug"
pass A still reading memory
pass B overwrites same bytes
```

gives corruption.

Alias transitions may require explicit API barriers or hazard tracking depending on backend.

Aliasing is memory optimization built on correct lifetime analysis.

---

**Transient memory**

A resource is transient if its contents need exist only during part of a frame.

Examples:

- blur intermediates,
- temporary visibility lists,
- scan workspace,
- G-buffer attachments not needed after lighting.

Because lifetimes are short and predictable, transient resources are ideal candidates for pooled/aliased memory.

---

**Maveb already reserves per-frame transient memory infrastructure**

Each `FrameContext` creates a private:

$$
64\text{ MiB}
$$

Metal heap labelled:

```text id="p10-transient"
Frame N Transient Heap
```

with tracked hazards.

That is exactly the kind of backing storage from which transient resources can be allocated.

From the inspected code, I would not yet claim that a general lifetime-aware frame-graph alias allocator is fully using this heap everywhere—the infrastructure is present, while full transient lifetime aliasing is a separate system.

That distinction matters.

---

**GPU timestamp queries**

CPU timing:

```cpp id="p10-wrong-gpu-time"
auto t0 = CPUClock::now();
submit();
auto t1 = CPUClock::now();
```

mostly measures:

$$
\text{command submission cost}.
$$

It does **not** measure shader execution.

To measure GPU duration, the GPU itself records timestamps around work.

Conceptually:

$$
t_0=\text{GPU timestamp before pass},
$$

$$
t_1=\text{GPU timestamp after pass}.
$$

Duration:

$$
\boxed{
T=
(t_1-t_0)\times
\text{timestamp period}.
}
$$

---

**Coarse GPU timing already exists in Maveb**

The frame command buffer completion handler reads:

```cpp id="p10-gpu-time"
GPUEndTime() -
GPUStartTime()
```

and stores milliseconds.

That gives coarse command-buffer/frame GPU duration.

Useful.

But it does not tell you:

```text id="p10-pass-timing-needed"
Gaussian projection: 0.3 ms
Radix sort: 1.1 ms
Scene pass: 2.8 ms
TAA: 0.4 ms
```

For that you need pass-level GPU timestamp/counter instrumentation.

---

**Pipeline statistics queries**

Timing tells you:

$$
\text{how long}.
$$

Pipeline statistics can tell you:

$$
\text{how much work}.
$$

Depending on API/hardware, possible counters include:

- vertices processed,
- primitives generated,
- fragments,
- compute invocations,
- clipping statistics.

If frame suddenly doubles from:

$$
8\text{ ms}\rightarrow16\text{ ms},
$$

statistics help distinguish:

> Shader got slower

from:

> We accidentally submitted twice as much geometry.

Not all metrics are available on every backend.

Capability querying matters.

---

**GPU profiling zones**

A profiling system should expose named zones:

```text id="p10-zones"
Frame
 ├── Shadow
 ├── Gaussian Project
 ├── Gaussian Scan
 ├── Gaussian Radix
 ├── Gaussian Composite
 ├── Scene
 ├── TAA
 └── Presentation
```

Each zone can associate:

- CPU encoding time,
- GPU start/end timestamp,
- counters,
- command-buffer identity.

Then profiler visualizes dependency/overlap.

Without zones, optimization becomes guesswork.

---

**Current Maveb profiler status**

The inspected `Profiler.cpp` is a CPU-side `ProfileScope`.

It records:

$$
\text{name},
\quad
\text{milliseconds}
$$

under a mutex.

The renderer separately records whole-command-buffer GPU time.

So current infrastructure distinguishes some CPU/GPU timing, but a full per-pass GPU timestamp-zone system is still a useful future step.

That would be particularly valuable for your Gaussian scan/radix pipeline.

---

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


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources|Next chapter]]
