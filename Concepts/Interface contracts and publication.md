# Interface contracts and publication

An interface contract states the caller’s valid inputs, ownership, result, failure behavior, timing and concurrency requirements. Publication is the transition that lets consumers rely on a prepared result under those requirements. CPU construction, GPU submission and GPU-read readiness are different events; an obsolete asynchronous request must not publish into a replacement destination.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#An API is a promise about an operation]].

## Mastery expansion — construction is not publication

An object can be fully constructed yet not safe for other threads or subsystems to observe. Publication is the transition that makes an initialized state reachable under an explicit synchronization/ownership contract.

For concurrent publication, all writes that establish the object invariants must happen-before readers that consume the published handle/pointer. Mutex release/acquire, atomic release/acquire, message queues and task-system completion edges can provide that relationship depending on design.

For asset publication, the same idea appears at a larger scale: decode into private state, validate, upload/prepare dependent resources, then atomically replace or register the visible version. If publication occurs earlier, readers can observe a structurally valid pointer whose logical dependencies are incomplete.

**Required next witness:** one threaded producer/consumer example with a deliberately broken relaxed publication variant and a correct release/acquire or mutex-protected variant.

