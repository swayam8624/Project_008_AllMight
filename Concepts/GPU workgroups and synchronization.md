# GPU workgroups and synchronization

GPU execution has several scopes that must not be collapsed into the word “thread.” An invocation executes one logical shader instance. Invocations are grouped into workgroups for explicitly shared local memory and workgroup barriers. Hardware may execute smaller subgroup or SIMD groups inside a workgroup. Many workgroups make up a dispatch.

A workgroup barrier can coordinate invocations that participate in that workgroup. It cannot create a global barrier across arbitrary workgroups in the same dispatch. If an algorithm requires all blocks to finish phase A before any block begins phase B, use a mechanism that establishes that dependency: usually separate dispatches/commands with API synchronization, or a carefully designed device-wide primitive supported by the target model.

## Execution barrier versus memory visibility

“Everyone reached the same point” and “everyone can now observe the required writes” are related but distinct contracts. APIs and shading languages expose memory scopes and semantics because a control barrier alone is not a universal cache flush.

The producer/consumer question is always:

- what operation produced the bytes?
- what later operation consumes them?
- which execution scopes must be ordered?
- which memory accesses must become visible?
- which resource/subresource is covered?

## Subgroups

Subgroup collectives can exchange values and perform ballots, reductions and scans without workgroup shared memory. They are fast but require care around subgroup width, inactive lanes and portability. An algorithm that silently assumes 32 lanes can break on hardware using a different subgroup size.

## Shared memory and bank conflicts

Workgroup memory is physically optimized for cooperation but has finite capacity and bank structure. Tiling can reduce global bandwidth, yet an unfortunate address pattern can serialize accesses through bank conflicts. Register pressure and shared-memory allocation also constrain occupancy, so increasing tile size is not free.

## Evidence boundary

The canonical notes now distinguish CPU semantic models from native hardware evidence. A CPU prefix-scan or render-graph test can prove dependency logic but cannot prove Vulkan visibility, Metal hazard behavior, subgroup width or driver performance. Hardware claims require validation layers, GPU capture/counters, and device-specific results.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Let many invocations cooperate without inventing a global clock]].

Related: [[Prefix scan and stream compaction]], [[Shader buffer ABI]].
