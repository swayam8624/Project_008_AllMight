# Phases 23–24 — From Logical Dependencies to Native GPU Synchronization

The repository already has a logical render-graph compiler. That is necessary, but it is not sufficient evidence for Vulkan or Metal correctness. A render graph can correctly say that pass B depends on pass A while the native backend still chooses the wrong pipeline stages, access scopes, image layouts, queue ownership transfer, Metal hazard mode, heap alias boundary or encoder synchronization.

This chapter establishes the missing middle layer: a **backend-neutral transition plan** that is explicit enough to test on the CPU and explicit enough to lower into native Vulkan/Metal commands. The companion reference oracle is `tools/labs/gpu_transition_plan_lab.cpp`.

The evidence ladder is:

logical pass/resource dependency
→ per-subresource state transition
→ backend lowering
→ validation/capture output
→ current production-renderer parity.

A passing CPU transition planner proves only the first two layers.

## 1. One resource name can contain many independent synchronization states

Treating an image as one indivisible resource is conservative but often wrong for a real renderer. Different mip levels, array layers and aspects can be used independently.

Represent native-relevant image state with at least:

- resource identity;
- mip range;
- array-layer range;
- aspect;
- previous access type;
- previous execution stage/domain;
- current layout/storage usage;
- queue/encoder ownership or scheduling domain.

The reference lab uses one exact `(mip, layer, aspect)` tuple per state so the invariant is easy to inspect. A production planner should support ranges and split/merge ranges when transitions overlap partially.

## 2. A dependency and a memory barrier are related but not identical

A logical dependency means one operation must precede another for semantic correctness.

A native memory dependency additionally answers:

- which earlier execution is in scope;
- which earlier memory accesses must become available;
- which later execution is in scope;
- which later accesses must observe them;
- whether the resource representation/layout changes;
- whether ownership moves across queues.

A write followed by a read is a RAW hazard. A read followed by a write is WAR. A write followed by a write is WAW. Read-after-read generally requires no memory hazard barrier unless another state change, layout requirement or queue transfer exists.

The planner therefore tracks access history rather than inserting a barrier simply because two passes mention the same texture.

## 3. Vulkan synchronization2 lowering

Current Vulkan synchronization uses structures such as `VkDependencyInfo` and `VkImageMemoryBarrier2`. An image barrier describes source and destination execution/access scopes, old/new image layouts, optional queue-family transfer, the image handle and a subresource range. Image layout transitions are themselves memory dependencies and apply to the specified subresources. citeturn438229search1turn438229search3

A backend-neutral transition:

`color-output write / COLOR_ATTACHMENT -> fragment read / SHADER_READ`

must therefore lower to backend-specific stage/access masks and the correct old/new layout. The exact masks depend on how the image was produced and consumed; “write then read” is not enough information unless the planner also retains execution domains.

Do not solve uncertainty by using `ALL_COMMANDS` and broad memory masks everywhere. That can be correct but destroys the scheduling information the graph was meant to expose and can create unnecessary stalls.

## 4. Queue-family ownership is a separate transition dimension

A resource moving from an asynchronous compute queue to a graphics queue has two obligations:

1. execution/memory synchronization between producer and consumer;
2. queue-family ownership transfer when the Vulkan sharing mode and queue families require it.

The reference planner marks `queue_transfer` independently from memory and layout transitions. A production lowering must create the appropriate release/acquire halves and semaphore/timeline ordering where needed.

If graphics and compute map to the same queue family, there may be no ownership transfer even though there is still an execution/memory dependency. Do not conflate “different logical queue role” with “different Vulkan queue family.”

## 5. Subresource precision prevents false dependencies

Suppose compute writes mip 0 of a luminance pyramid while graphics samples mip 1 produced earlier. A whole-image state tracker may serialize them even if the algorithm permits independence.

A per-subresource tracker can keep mip 0 and mip 1 state separate.

But precision increases bookkeeping complexity. Range overlap, \partial transitions, whole-image views and aliasing must all be correct. The safest development route is:

single whole-resource state
→ exact subresource tuple oracle
→ range splitting/merging
→ native validation.

Do not jump directly to a clever interval structure before the semantics are tested.

## 6. Transient aliasing introduces lifetime dependencies

Two logical resources can reuse the same physical memory only when their live intervals do not overlap and the API's aliasing rules are satisfied.

The render graph therefore needs both:

- dataflow lifetime: first/last pass that can access the resource;
- physical-memory lifetime: when the backend may safely reuse the allocation.

A pass graph that proves resource A is no longer logically used still needs the backend-specific visibility/encoder/heap rules before memory can be rebound or reused.

For Vulkan, transient aliasing interacts with memory binding and image aliasing requirements. For Metal heaps, resources can occupy heap memory with explicit aliasing/lifetime management; hazard behavior depends on resource/hazard-tracking mode and API generation. Apple documents heaps as a mechanism for sharing/reusing memory among resources, including transient resources. citeturn438229search4turn438229search5

## 7. Metal hazard tracking must be version-aware

Older/current Metal resource APIs can use tracked hazard behavior, where Metal automatically manages some read/write dependencies for tracked resources. Untracked resources require the application to provide the synchronization needed for hazards. Apple also documents explicit barriers, fences and events for synchronization across appropriate scopes. citeturn438229search2turn438229search8

Metal 4 changes the model further: Apple states that Metal 4 resources are untracked and applications use explicit barriers for concurrent read/write hazards. Therefore PROJECTALLMIGHT must not teach one timeless sentence such as “Metal handles hazards automatically.” The answer depends on the API generation and resource mode. citeturn438229search7turn438229search9

The backend abstraction should expose the semantic dependency and let the Metal implementation lower it according to the supported Metal version rather than baking a Vulkan-shaped barrier vocabulary into all backends.

## 8. Memoryless resources are a tile-memory lifetime contract

On Apple GPUs, memoryless textures are intended for temporary render-target data that need not survive outside the render pass/encoder use in which the tile memory is valid. They can reduce persistent memory traffic/footprint for suitable attachments, but they are not general-purpose textures. Apple documents memoryless storage specifically for temporary render targets. citeturn438229search0turn438229search11

A render graph can infer a memoryless candidate only when:

- the resource is produced and consumed entirely within a compatible render-pass lifetime;
- no later pass samples/copies/reads it;
- no CPU access or persistent history is needed;
- store/load actions match the intended lifetime.

The optimization must be derived from lifetime, not selected because the texture “looks temporary.”

## 9. History resources are versioned external state

TAA history, exposure history and temporal denoisers are not ordinary transients. They cross frame boundaries.

Model them as versions:

$$
H_n \rightarrow \text{read by frame }n+1,qquad
H_{n+1}\rightarrow \text{written by frame }n+1.
$$

The graph must distinguish “previous history” from “next history” even if a backend ping-pongs between two physical images. Resize, format changes, camera cuts or content-version changes can invalidate the semantic history independently of GPU synchronization.

This connects the render graph to [[Rendering history validity]] rather than treating temporal state as merely another texture.

## 10. Validation evidence is part of the implementation

A native Vulkan witness should retain:

- validation layer output;
- exact device/driver/API version;
- resource transition trace;
- queue family identities;
- GPU \timestamp evidence for the chosen scheduling;
- negative tests that intentionally omit a required transition and demonstrate validation failure where validation can detect it.

A Metal witness should retain:

- Metal API/device/OS version;
- resource storage/hazard modes;
- heap allocation/alias decisions;
- encoder/barrier/fence/event sequence;
- Xcode GPU capture or equivalent evidence;
- negative/alternative configurations where possible.

A screenshot saying “frame rendered” is not enough. Rendering bugs can remain latent because a driver happened to serialize work conservatively.

## 11. The CPU transition oracle

`gpu_transition_plan_lab.cpp` tracks one state per exact subresource and emits a barrier record when any of three independent dimensions require work:

- memory hazard;
- layout transition;
- queue transfer.

Its tests cover:

- undefined → color attachment transition;
- color write → shader read RAW + layout transition;
- compute → graphics queue transfer;
- independence between mip 0 and mip 1;
- read → write WAR;
- color attachment → present transition.

This is deliberately more precise than the existing whole-resource logical graph, but deliberately less ambitious than native synchronization.

## 12. What still prevents “native verified”

The mastery ledger must keep P23/P24 below native-verified until the repository has all of the following:

1. Vulkan lowering from transition records to `VkDependencyInfo` / barrier structures.
2. Subresource-range splitting and merging.
3. Queue-family release/acquire and semaphore/timeline integration.
4. Swapchain acquire/present lifecycle.
5. Vulkan validation output from a real device/runtime.
6. Metal lowering for the supported Metal generation.
7. Tracked/untracked hazard-mode policy where applicable.
8. Heap aliasing lifetime implementation.
9. Memoryless attachment inference with correctness tests.
10. GPU capture/\timestamp evidence.
11. Cross-backend image/result comparison for the same graph.
12. Current Kairo/Maveb integration at a pinned commit.

Until then, the correct status is **reference transition planning developed; native backend verification open**.
