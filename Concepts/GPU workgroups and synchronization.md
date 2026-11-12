# GPU workgroups and synchronization

Workgroups define a cooperation and shared-memory scope; subgroups define a smaller collective. Atomic updates, memory ordering and barriers provide different guarantees. A workgroup barrier cannot synchronize arbitrary other workgroups.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Let many invocations cooperate without inventing a global clock]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — execution scope, memory scope and visibility

A GPU workgroup/threadgroup is a scheduling and cooperation unit, not a global barrier domain. Threads inside one group can often synchronize through a workgroup barrier and share explicitly scoped fast memory. Threads in different groups generally cannot rendezvous inside one dispatch without a different algorithmic phase or API-level dependency.

Three questions must be separated:

1. **Execution ordering:** which operations are guaranteed to happen before which other operations?
2. **Memory availability:** when have previous writes completed strongly enough to participate in synchronization?
3. **Memory visibility:** when may a later reader observe those writes in its memory domain?

A barrier primitive may combine some of these concepts, but an engine must reason about the specific API and resource scope. A CPU logical dependency is not automatically a Vulkan pipeline barrier, Metal encoder boundary or cross-queue wait.

Within a tiled kernel, every thread that is required to participate in a group barrier must reach the barrier. Returning early for an out-of-bounds tile lane before a barrier can deadlock or violate the kernel contract. Use masked loads/stores while keeping barrier participation uniform.

Occupancy is also not the same as performance. Increasing workgroup size can increase register pressure or shared-memory use and reduce the number of resident groups. Measure the real kernel.

**Required next witness:** native GPU prefix-scan/tiled-matmul experiments with validation/capture evidence and a CPU oracle for exact or bounded-result comparison.

