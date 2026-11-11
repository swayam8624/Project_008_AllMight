---
tags: [visual-recall, phase4, cpu, caches, concurrency, performance]
coverage: "M019–M024; Parts 301–400"
companion: "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs"
---

# One scene update across the machine — Phase 4 retrieval

This is a reconstruction companion, not a replacement for the [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From a correct C++ asset service to the instructions a CPU can execute — Phase 4, M019–M021, Parts 301–346|first]] and [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From temporary memory to finished parallel frames — Phase 4, M022–M024, Parts 347–400|second]] continuous chapters. All 100 canonical titles and their actual proof limits appear in [[Supplementary/Foundations#Phase 4 CPU architecture and measured execution — Parts 301–400|the source contract]]. Each task below asks for a causal proof or independent implementation; reading the attached answer is not an assessment.

## Reconstruction A — from source to instruction schedule

A particle loop loads x and mass, multiplies them and accumulates into one scalar. Construct a dependency graph showing which loads can overlap and why the accumulator creates a true dependency. Explain why superscalar width is not achieved throughput. Consider what happens if the loop is unrolled into four independent partial accumulators: register pressure rises; dependence depth may shrink; floating-point sum order changes. Distinguish a branch predictor's speculation from the C++ semantics that decide which results are architecturally visible. Sketch a hypothetical five-stage pipeline, then identify what the model omits about modern out-of-order execution.

**Required evidence:** an optimized assembly excerpt, compiler version/flags and a separate handwritten baseline. If you cannot show them, do not claim a measured instruction cost.

## Reconstruction B — where the 64-byte record goes

A large AoS record has sixteen float components, but the hot loop reads only two of them. Derive the useful-byte fraction under a 64-byte-line illustrative cache. Compare the address sequence with separate x/mass arrays. Compute the 64 sets of a hypothetical 32-KiB, 8-way, 64-byte-line cache, and design a repeating stride that can produce conflicts under its simple index mapping. Explain why L1/L2/L3 latency, DRAM bandwidth, TLB misses and NUMA placement are different possible bottlenecks. Give a test that could disprove the hypothesis "SoA is always faster."

**Required evidence:** identical numerical results plus repeated timing and counters on a specified CPU. The [[tools/labs/phase4_cpu_systems_lab.cpp|current lab]] proves only selected equality and an optional timing experiment.

## Reconstruction C — why false sharing is not a data race

Two workers update two distinct atomic counters. Explain how a coherence unit covering both counters may migrate, even if both threads use race-free atomic increments. Compare this with two workers writing the *same ordinary integer* without synchronization (undefined behavior). Derive why alignas(64) can help on a machine with appropriate interference granularity but does not guarantee an arbitrary platform's cache-line size. Explain how a store-release plus observing load-acquire publishes a non-atomic payload, and give a counterexample where the acquire load does not observe the publishing value.

**Required evidence:** coherent happens-before argument, with actual source atomic orders and object lifetime. A passing run alone cannot establish the absence of all possible data races.

## Reconstruction D — storage acquisition is not object construction

Design a 4-KiB bounded arena for 64-byte aligned scratch records. Compute padding, capacity and overflow checks before producing a pointer. Explain when placement construction starts object lifetime, which destructors must run, and why monotonic allocation cannot be treated as a per-object free list. Compare slab, buddy, size-class, LIFO stack, free-list and frame arenas for a mixed workload of transient commands and persistent mesh records. Distinguish internal and external fragmentation.

Then trace a mesh whose CPU handle is erased while a GPU draw might still use its underlying buffer. Explain how a monotonic completion timeline permits physical destruction only after the actual submitted use finishes. Analyze what must change for multiple GPU queues or device loss. The [[tools/labs/phase4_cpu_systems_lab.cpp|CPU ticket model]] is not a Vulkan or Metal implementation.

## Reconstruction E — make three threads progress without a lost wake-up

Implement one task queue with mutex and condition variable. Write the exact shared predicate checked in a loop; explain why checking a condition once or relying solely on notify_one is insufficient under spurious wakes or races. Prove that workers release the lock before executing user tasks. Consider a dependency chain A→B→C and show how workers waiting synchronously for tasks from the same saturated pool may deadlock. Distinguish deadlock, livelock and starvation; construct an explicit circular lock-order counterexample.

Implement one producer publishing a payload through store-release and one consumer reading it after observing load-acquire. Distinguish relaxed counter increments from this publication protocol. Explain how compare-and-exchange can encounter ABA and why generation tags are not a general memory reclamation scheme.

## Reconstruction F — work/span, deterministic sums and profiler evidence

Given W units of work and S on the critical path, derive the ideal bounds T_P ≥ max(W/P,S). For a task cost c and scheduling overhead h, explain why increasing the number of tasks improves balancing until overhead and contention dominate. Compare a shared queue with a per-worker work-stealing deque, including the single-item steal race that needs a real algorithmic protocol. With ten thousand exact integer-valued doubles, demonstrate fixed partitioning and ordered reduction; then exhibit three floating-point numbers whose reassociation changes the result.

Starting from a hypothetical 16-ms frame breakdown, form one falsifiable hypothesis. Choose a sampling profiler, instrumentation trace or hardware counters appropriate to the hypothesis. State warm-up, sample count, cache conditions, confidence/variance, end-to-end measurement and rollback condition. Derive Amdahl speedup for serial fraction 0.2 and four ideal workers: 2.5, not 4. Then explain which model assumptions would fail for real heterogeneous CPU/GPU execution.

## End-to-end mastery gate

Reconstruct the entire asset-to-frame chain without looking at the book: a typed catalog retains an owned resource; a hot loop chooses AoS or SoA from actual accesses; the compiler emits an instruction schedule; threads share memory only through defined synchronization; a persistent pool executes independent jobs while dependencies constrain progress; scratch memory remains alive through its users; GPU resources cannot retire before completion; a profiler establishes whether the optimization truly improves the frame. Name every assumption, uncertainty and failure path. If the real Kairo/Maveb code is uninspected, say so rather than claiming correspondence.

**Research and source bridge:** [MIT 6.172](https://ocw.mit.edu/courses/6-172-performance-engineering-of-software-systems-fall-2018/video_galleries/lecture-videos/), [Agner Fog's manuals](https://www.agner.org/optimize/), [cppreference atomics](https://en.cppreference.com/w/cpp/atomic/memory_order), and [Amdahl's 1967 paper](https://doi.org/10.1145/1465482.1465560). These references establish concepts and comparison methods; they do not establish the current engine's measured results.
