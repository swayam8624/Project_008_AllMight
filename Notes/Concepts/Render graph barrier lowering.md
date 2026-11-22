# Render graph barrier lowering

A logical render graph and an API synchronization plan are different layers.

The logical graph answers:

- which pass reads or writes each resource;
- which hazards create ordering edges;
- whether the pass dependency graph is acyclic;
- when a logical resource is first and last used.

API lowering answers:

- what concrete stage/access state each use requires;
- what image layout or resource state is required;
- which subresources are affected;
- which queue executes each pass;
- what barrier, semaphore, fence/event, or encoder boundary realizes the dependency.

The repository already has tools/labs/render_graph_contract_lab.cpp for the first layer. That CPU witness proves RAW/WAR/WAW edge construction, deterministic topological order and several invalid-graph cases. It does not prove Vulkan or Metal memory visibility.

## State machine per resource version

A practical compiler treats each write as producing a new logical version. A read consumes a particular version. For every concrete resource/subresource, track the last known backend state:

producer pass  
queue  
stage/access  
layout or storage state  
version.

When the next use requires an incompatible state, emit a transition whose source scope covers the prior access and whose destination scope covers the next access.

This versioned model prevents an important bug: a pass can read “history texture” while the graph accidentally associates it with the current-frame version rather than the previous-frame version.

## Subresource granularity

Whole-resource tracking is simple but can over-synchronize. Independent mip levels, array layers or depth/stencil aspects may be used by different passes. Moving to subresource tracking requires interval/range splitting and merging so that state for mip 0 does not overwrite the state of mip 5.

The first correct implementation should prefer conservative whole-resource barriers over an incorrect fine-grained optimizer. Fine granularity is an optimization after invariants are tested.

## Vulkan lowering

For Vulkan synchronization2, a transition record eventually becomes VkBufferMemoryBarrier2, VkImageMemoryBarrier2 or VkMemoryBarrier2 inside VkDependencyInfo, possibly combined with queue submission dependencies.

See [[Vulkan synchronization2 and resource transitions]].

## Metal lowering

Metal does not mirror Vulkan’s barrier vocabulary one-for-one. The backend should lower the same logical dependency into Metal’s encoder ordering, resource hazard mode, fences/events where required, and heap/lifetime rules rather than forcing Metal to imitate Vulkan object names.

See [[Metal heaps memoryless resources and hazards]].

## Verification ladder

A render-graph compiler should be verified in layers:

1. dependency graph against a CPU oracle;
2. resource-state transition plan against hand-written traces;
3. Vulkan execution with validation enabled;
4. Metal execution with GPU capture;
5. cross-backend image/data parity;
6. transient aliasing stress with deliberately overlapping and non-overlapping lifetimes;
7. current production-engine integration.

The current branch now has stages 1 and a minimal Vulkan synchronization2 execution/Metal compile boundary. The deeper stages remain explicit rather than implied.
