# Interface contracts and publication

An interface contract says more than which function name to call. It defines the states in which an operation is valid and what becomes true when it succeeds.

A complete contract should state:

- valid input domain and units;
- ownership and lifetime of arguments/results;
- mutation and aliasing;
- failure representation and exception/error guarantees;
- thread/concurrency rules;
- timing: immediate, deferred or asynchronous;
- invalidation;
- observability/publication point.

## Publication is a state transition

A producer may construct a resource privately and then publish it so consumers can rely on it. Before publication, partial state can remain invisible. After publication, every documented invariant must hold.

For CPU concurrency, release/acquire synchronization is one possible publication mechanism when the memory model requirements fit the data structure.

For asynchronous assets, “decode finished” does not mean “GPU resource is safe to sample.” A useful state machine may distinguish:

requested → decoded → upload recorded → submitted → GPU-ready → published.

Similarly, an obsolete request must not publish after a newer generation has replaced it. Generation IDs or request tokens protect the destination identity.

## Ownership versus readiness

A reference can be alive but not ready. A GPU buffer wrapper can exist while its upload is still in flight. Conversely, a resource can be GPU-ready while the CPU owner is about to destroy it unless retirement is deferred until all GPU use completes.

Lifetime and synchronization are independent axes.

## Failure guarantees

If publication is atomic, consumers should observe either the old valid version or the new valid version, not a half-updated mixture. That promise requires implementation evidence appropriate to the domain: atomics/locks for CPU state, transaction/staging for files, semaphore/fence/queue dependencies for GPU work.

## Evidence

tools/labs/threaded_asset_publication_lab.cpp and tools/labs/production_pipeline_contract_lab.cpp exercise two different publication domains: concurrent asset decoding/publication and deterministic staged production publication.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#An API is a promise about an operation]].

Related: [[Atomic publication and durability]], [[Production manifests and transactional publication]], [[Vulkan synchronization2 and resource transitions]].
