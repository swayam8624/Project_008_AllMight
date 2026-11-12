# Metal heaps, memoryless resources, and hazards

Metal exposes several resource-lifetime tools that matter directly to an engine render graph. Their purpose is not just API convenience; they let the engine state when storage must exist, when bytes may be reused, and who is responsible for synchronization.

Apple documents MTLHeap as a way to create resources from a larger allocation and to alias memory among resources whose lifetimes do not overlap. Apple also documents memoryless textures for temporary render targets whose contents can remain in tile memory rather than occupying ordinary persistent system/GPU memory.

Primary references:
- https://developer.apple.com/documentation/metal/memory-heaps
- https://developer.apple.com/documentation/metal/reducing-the-memory-footprint-of-metal-apps

## Heap aliasing is a lifetime proof

If resources A and B use overlapping heap memory, the engine must prove that A is dead before B begins using the same bytes. “They are in different render passes” is not enough when command encoding/execution overlap or when a resource is consumed asynchronously later.

A render graph can perform interval analysis:

A lifetime = [first use, last use]  
B lifetime = [first use, last use].

Only non-overlapping lifetimes are candidates for aliasing, and compatibility constraints such as size/alignment/storage mode still apply.

The allocator also needs to keep the heap itself alive for every resource created from it.

## Hazard tracking mode

Metal heap resources can use tracked or untracked hazard behavior. Apple’s heap documentation explicitly notes that Metal synchronizes heap-created resources when their hazard tracking mode is tracked. Choosing untracked mode transfers more responsibility to the application. That decision should therefore exist in the backend state model rather than being an incidental resource flag.

## Memoryless attachments

Memoryless texture storage is suited to temporary render-target contents that do not need to survive beyond the render-pass/tile use for which they are created. A later pass that samples the image needs persistent storage; marking such a resource memoryless would contradict the data-flow graph.

The render graph can infer memoryless eligibility only after it understands whether the produced contents escape the render-pass lifetime.

## Argument buffers and residency

Argument buffers can reduce CPU binding overhead and can reference many resources. That also makes lifetime/residency reasoning more indirect: the resource referenced by an argument buffer must remain valid for the encoded GPU work even if the CPU-side wrapper is no longer visible.

## Current evidence

tools/native/metal_compile_smoke.metal is compiled to AIR and metallib on the macOS CI job. This proves that the repository owns a real MSL compile boundary instead of only pseudocode.

Compile success is not GPU execution evidence. It does not prove heap aliasing, tracked/untracked hazard behavior, memoryless attachment correctness, argument-buffer residency, command-buffer timing or Apple-GPU performance. Those require a Metal execution/capture on suitable hardware.

Related: [[GPU workgroups and synchronization]], [[Render graph barrier lowering]], [[Interface contracts and publication]].
