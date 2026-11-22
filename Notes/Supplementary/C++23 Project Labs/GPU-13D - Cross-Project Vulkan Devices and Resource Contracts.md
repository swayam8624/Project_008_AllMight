### GPU-13D — Open a Vulkan device for the work you actually need

The previous chapters built progressively stronger contracts. We learned that a decoded asset is not automatically a GPU allocation, a valid dependency graph is not a memory barrier, and an accepted command is not a completed command. We used the NES debugger as one small controlled producer of CPU data, but **that emulator is not the destination of the seven-book curriculum**. You have a Vulkan/Metal graphics engine, a Metal-powered neural/geometry reconstruction renderer, a physics and broad-phase workload, and plans for XR vision and neural rendering. These projects stress different resource families and hardware capabilities. A capable engineer must make the **same principled device and memory decisions** across all of them, not merely find a graphics adapter that can upload eight emulator pixels.

This pass therefore changes the center of gravity. It retains the NES overlay as a small regression workload but broadens the executable teaching system into a **cross-project GPU capability laboratory**. We inspected concrete source revisions in [KairoRenderer](https://github.com/swayam8624/KairoRenderer), [Maveb](https://github.com/swayam8624/Maveb), [KairoPhysicsEngine](https://github.com/swayam8624/KairoPhysicsEngine), and [KairoGPU](https://github.com/swayam8624/KairoGPU). They are *different repositories*, not modules we silently moved into the AllMight branch. The new files are committed under `tools/labs/gpu_course/` and `tools/labs/gpu_device_resource_cpp23_lab.cpp`. We only **read** those external repositories in this pass; no code in them was changed.

The original Chapter 13 contains approximately 4,556 lines of explanations about Vulkan instance and queue philosophy, device selection, surfaces and swapchains, VkBuffer/VkImage, memory types and heaps, suballocation, staging, texture formats, descriptor sets, SPIR-V, shader interfaces, graphics pipelines, dynamic rendering and attachment load/store rules. Do not summarize those pages away. The new lesson belongs **immediately before the original `### BUFFERS, IMAGES, MEMORY & UPLOADS` section**. The original chapter will first teach instances/devices/queues; the new lab then makes a real workload-driven device and resource-decision exercise; then the original thorough resource/descriptor/pipeline treatment continues unmodified.

#### Start with a workload contract, not “pick the fastest GPU”

A physical device's advertised name and device type do not answer whether it is suitable for a particular job. A discrete device can be excellent for large local-memory rendering but lack a feature required by a particular compute kernel. An integrated device may offer host-visible and device-local properties on the same memory type. A virtual or software implementation may be fully capable of running a correctness test but unsuitable for performance benchmarking. Choosing an adapter by taking index zero, or preferring "discrete" without validating required features, is a policy error.

Different jobs require different capabilities. Consider these concrete project-derived roles, each deliberately distinguished from its implementation backend:

| Workload | Relevant existing code / project | Basic capability question | Memory and resource question |
| --- | --- | --- | --- |
| NES debugger overlay | AllMight owned CPU snapshot + TXR4 typed image | Graphics and presentation for the windowed variant; transfer for an explicit upload | Tiny host-visible upload versus sampled image allocation |
| Kairo indexed scene | `KairoRenderer/VulkanDevice.cppm`, `VulkanBuffer.cppm`, `VulkanTexture.cppm` | Graphics queue, surface-specific present support and the device extensions/features actually used | Vertex/index/uniform buffers, depth/color images and shader-readable textures |
| Maveb Gaussian raster and compute | `Maveb/engine/metal/src/Renderer.cpp`, `FrameContext.cpp` | A *hypothetical Vulkan port* may require graphics, compute and transfer | Large storage/indirect/scan buffers, texture storage, alignment, transient heaps and actual feature-dependent formats |
| Kyoto collision-pair pipeline | Collision broad-phase goal, plus the inspected `KairoPhysicsEngine/Broadphase.cppm` as a separate code reference | Compute and transfer can suffice; no swapchain is needed for headless physics | Storage buffers for bounds, candidate pairs, counters and indirect compacted results |
| XR gaze-dependent rendering | XR/foveated work and camera/rendering engines | Graphics, compute and—when using a window/compositor surface—the appropriate presentation contract | View images, gaze maps, foveation data, depth/color formats and latency-sensitive upload/readback |

The last two columns are **requirements to check**, not assertions that the current engine repos already implement the same exact pipeline in Vulkan. Kairo's renderer contains actual Vulkan device and buffer classes. Maveb's inspected frame code is **Metal**, not Vulkan; mapping its storage choices to Vulkan is a design exercise. The Kyoto common-contact research code was not inspected from a remote repo in this pass, so the collision-pair case is grounded in the stated workload and in KairoPhysicsEngine's separately inspected `Broadphase.cppm`, not a fabricated claim about current Kyoto worktrees.

The lesson's `teaching_workloads()` fixture deliberately contains one example for each project family, with illustrative resource sizes. Those byte counts are **not queried from real hardware**, and the test never prints them as Maveb's measured splat storage or Kyoto's actual pair count. The purpose is to force a developer to ask which queues and memory requirements a given workload needs. Later a real run must substitute requirements returned by the selected GPU and exact resource creation flags.

#### What the actual Kairo Vulkan source currently does

We inspected `KairoRenderer/VulkanInstance.cppm`, `VulkanDevice.cppm`, `VulkanBuffer.cppm`, `VulkanTexture.cppm`, `VulkanRenderer.cppm` and `MetalRenderer.cppm` on the connected repo's main branch, using their returned Git object identifiers. These files establish more than the project's README could: the code actually calls `vkEnumeratePhysicalDevices`, queries `VkPhysicalDeviceFeatures` and queue families, checks surface support and swapchain extension availability, then creates the logical device. A specific requirement for `independentBlend` is visible in `VulkanDevice.cppm` because its rendering path expects independently blended attachments. That is a valid example of a **feature derived from a rendering contract**, rather than turning on every advertised feature indiscriminately.

Kairo's `VulkanHostBuffer` separately creates `VkBuffer`, calls `vkGetBufferMemoryRequirements`, chooses a type matching `memoryTypeBits` plus `HOST_VISIBLE|HOST_COHERENT`, allocates `VkDeviceMemory`, binds it and maps the requested buffer length. These are exactly the operations the abstract chapter tells you to understand. Its texture implementation chooses a memory type with `DEVICE_LOCAL`, creates an image and view/sampler and includes a staging/transition command path. The two cases are different: a CPU-friendly host buffer and a GPU-oriented texture should not be forced into one physical memory policy when the hardware advertises different suitable types.

There is also a concrete review target: in the currently retrieved `VulkanBuffer.cppm` destructor, the visible order is **unmap memory → free memory → destroy buffer**, whereas `VulkanTexture.cppm` explicitly destroys the image before freeing its backing memory. The buffer cleanup order deserves a targeted lifecycle correction and Vulkan-validation test: a buffer still bound to memory should be destroyed before its backing allocation is freed. Our Pass 13 native probe therefore deliberately owns both resources under one RAII cleanup object that invokes `vkDestroyBuffer` **before** `vkFreeMemory`. This does not mean we changed or tested Kairo's runtime; it means the read-only source audit found a precise issue worth addressing in that repository when authorized.

Another distinction matters in `VulkanDevice.cppm`: Kairo requires actual presentation support for a windowed renderer. A collision-pair compute runner with no surface should not copy that same requirement. Enforcing `VK_KHR_swapchain` for a headless compute benchmark would reject usable devices for a capability the benchmark never needs. Conversely, a windowed renderer must not infer presentation merely because a queue advertises `VK_QUEUE_GRAPHICS_BIT`. Presentation support depends on the actual `VkSurfaceKHR`, platform and queue family.

The KairoGameEngine and KairoRenderer repositories are separate and can evolve at different revisions. We only observed files on the connected main branches, not the user's local working directory, submodules, or uncommitted engine changes. A definitive production audit will need the exact engine revision and build logs at the time of integration.

#### The native chain has several different objects with different lifetimes

Vulkan is explicitly object-based. A common chain for a headless buffer operation is:

```text
Vulkan loader / library
    |
    vkCreateInstance
    |
    vkEnumeratePhysicalDevices
    |
    VkPhysicalDevice   (queried capability, not owned allocation)
    |           \
    |            VkPhysicalDeviceMemoryProperties
    |            queue families / supported features / extensions
    |
    vkCreateDevice  (enabled queues, features, extensions)
    |
    VkDevice + VkQueue
    |
    vkCreateBuffer       (description, no separately bound storage yet)
    |
    vkGetBufferMemoryRequirements
    |
    choose memory type consistent with memoryTypeBits and requested flags
    |
    vkAllocateMemory
    |
    vkBindBufferMemory
    |
    [later: record/submit correctly synchronized commands]
    |
    destroy buffer, free bound memory, destroy device, destroy instance
```

A windowed renderer extends this with instance extensions, surface creation, a compatible present-capable queue, swapchain format/mode/extent decisions, swapchain images and views, acquire/present operations and their independent lifetimes. A headless compute program does not need to invent a window merely to access a device.

The object graph is more than a sequence of API calls. Destruction must preserve dependency direction. A buffer or image needs its allocation to remain valid while it is bound and referenced. Destruction of one application wrapper while a command still executes can violate resource lifetime even if the wrapper's destructor has perfect reverse-order cleanup code. The Pass 12 command journal established that a native backend must track those in-flight dependencies *before* cleaning up memory. Device destruction is not an excuse to ignore queued work; native API requirements and device-loss recovery rules must be followed.

Kairo's and Maveb's native ownership idioms differ. Kairo's C++ modules use explicit Vulkan handles and destructors; Maveb's Metal code uses a `MetalPtr` ownership wrapper around Objective-C resources and a `FrameContext` for per-frame allocation. The correct shared lesson is not “write the same RAII class for both APIs.” It is that every resource needs an explicit *creator*, a statement of what owns it, a declaration of what may borrow it, and an evidence-bearing point at which destruction becomes legal. The concrete low-level API differs.

#### Separate core instance features, device features, extensions and presentation

Some capabilities belong to the Vulkan **instance** (layers and instance extensions), some to the physical-device feature/property query, and some to enabled **device** extensions/features. The existence of an extension name in a list does not mean it has been enabled on a created device. Nor does support for a Vulkan API version automatically enable every optional feature exposed by that version. For advanced functionality, applications often need `VkPhysicalDeviceFeatures2` with appropriate extension/core feature structures in its `pNext` chain and must then enable the exact supported features requested by the pipeline.

Consider a Gaussian raster/compute implementation. It may need storage buffers, compute dispatch and standard graphics queues, but additional paths could require descriptor indexing, subgroup features, buffer device address, shader float16, certain image formats or ray tracing support. A device profile that only records “supports Vulkan 1.3” cannot certify those extra requirements. The first useful implementation step is to name each advanced requirement at the operation where it is needed and query its actual support. **Unsupported optional acceleration paths should have a documented fallback**, rather than causing all CPU or basic GPU operation to fail.

The cross-project CPU contract intentionally stops at a small intersection of basic queue and memory capabilities. Its data types do not pretend to model ray-tracing pipelines, sparse residency, mesh shaders, video, XR compositor extensions, subgroup-size control or MTL4-specific capabilities. Those require dedicated feature/query interfaces and separate tests later in the curriculum.

The optional native probe chooses a Vulkan 1.0 API request because it uses foundational discovery, device creation and buffer memory binding. It does not request `VK_KHR_swapchain` without a surface; it separately checks whether portability enumeration is advertised and enables the corresponding instance extension/flag when available. On portability implementations such as a MoltenVK-backed Vulkan loader, it also requests the portability-subset device extension when advertised and required by that implementation. It intentionally **does not require a validation layer to be installed** merely to enumerate a device. A validation-enabled backend should first discover whether the requested layer is available and report unavailability rather than unconditionally reproducing a `VK_ERROR_LAYER_NOT_PRESENT` failure.

#### A queue family is not a guarantee of parallel engines

The portable header models queue families using three flags—graphics, compute and transfer—plus a queue count and an *optional* surface-present result. In a real Vulkan query these correspond to a subset of `VkQueueFamilyProperties::queueFlags`, `queueCount` and a separate `vkGetPhysicalDeviceSurfaceSupportKHR` query. A family with a count of zero is not selectable, even if a synthetic record mistakenly sets a capability bit.

For a workload asking for graphics, the selector requires a queue family that explicitly advertises graphics operations. For compute, it requires compute. For an explicit transfer phase, it requires transfer. When the chosen graphics family also supports compute and transfer, the basic lesson prefers to reuse it; this avoids introducing unnecessary queue-family ownership transfers into a first implementation. A separate dedicated compute or transfer family can be chosen when necessary, but that selection is **not** a claim that such a family runs simultaneously with other families on independent execution engines.

The presence of multiple queue families also does not automatically improve performance. Dedicated transfer hardware and queue priorities vary by device; compute and graphics may share arithmetic and memory bandwidth. Moving a kernel to a second queue without measuring actual overlap may simply add synchronization overhead and memory contention. The correct performance question is workload-specific: how much useful work finished per unit time under a valid ordering/visibility protocol?

For a presented workload, queue selection is stricter. The profile requires both the swapchain device extension and a queue family with positive presentation support for the intended surface. Presentation support in the CPU fixture is represented by `std::optional<bool>`, not an ordinary boolean defaulting to true. The three states mean **known yes, known no, and not queried**. If a surface is not available, the native probe cannot prove presentation, and the model refuses a presented workload with `unknown_surface_support` rather than manufacturing a successful render-capable device.

This lets the lab demonstrate a practical distinction. The same fake device with a compute-capable queue but no known surface support is acceptable for headless collision-pair computation. It must be rejected as a validated windowed renderer. The tests also construct a three-family device in which graphics, compute and present are separate; the selector records those actual indexes rather than assuming all work belongs to family zero.

#### Resource sizes come from requirements, not asset payload guesses

A mesh might contain 64 KiB of vertex data. A neural renderer might pack a million Gaussian attributes into buffers. A physics engine might append a variable number of broad-phase collision pairs. Yet the number of useful payload bytes is not automatically the `allocationSize` that Vulkan expects for the created resource. The backend first creates a `VkBuffer` or `VkImage` descriptor with usage, format, dimensions, samples and other constraints. Then `vkGetBufferMemoryRequirements` or `vkGetImageMemoryRequirements` returns the required allocation size, alignment and compatible memory-type bit mask. A memory allocator chooses a backing allocation and binds it under the corresponding requirements.

The Pass 13 host policy calls this structure `MemoryRequest`:

```cpp
struct MemoryRequest {
    std::uint64_t requirement_size; // Vulkan's queried size
    std::uint64_t alignment;       // Vulkan's queried alignment
    std::uint32_t memory_type_bits; // Vulkan's compatible types mask
    std::uint32_t required;        // required property flags
    std::uint32_t preferred;       // best-effort extra property flags
};
```

It is explicitly an input derived from *queried* native requirements in a real backend, or from a declared synthetic fixture in the lab. We do not use `pixel_count * 4` as an unquestioned GPU allocation size, nor assume a Gaussian buffer's array length determines the image tiling or resource alignment. The device reports relevant alignment and memory-type compatibility; the allocator checks them.

Memory-type eligibility is a conjunction. Let `B` be the resource's `memoryTypeBits` mask and `i` the candidate memory type index. Let `F_i` be that type's property flags and `R` the application-required flags. A candidate can pass the preliminary test only when

$$
(B\;\&\;(1\ll i))\ne 0
\quad\land\quad
(F_i\;\&\;R)=R.
$$

These checks are necessary, not sufficient for actual allocation success. The selected type must refer to a valid memory heap, its heap size and any known budget must be considered, and the requested native allocation must succeed. Some native resources require dedicated allocations, external-memory constraints or additional alignment/aliasing rules; those are not included in this introductory portable selector.

The C++23 helper refuses a zero byte requirement, zero or non-power-of-two alignment and zero memory-type mask. It handles the highest valid bit (index 31) without undefined shift operations and rejects an impossible table longer than the 32-bit mask can represent. It verifies the candidate's heap index before reading heap properties, checks whether the requirement fits, and selects among candidates according to explicit **required** versus **preferred** flags. An impossible resource mask is a typed failure, not permission to allocate a random heap.

#### Host-visible and device-local are not opposites

The original chapter correctly clarifies that a memory type may be both host-visible and device-local. Our synthetic "UMA" test device deliberately exposes that combination. It satisfies an upload request requiring `HOST_VISIBLE|HOST_COHERENT`, and it also satisfies a device-oriented storage-buffer request requiring `DEVICE_LOCAL`. The two properties can coexist; Vulkan memory flags form a bit mask, not a single-choice enum called "CPU" versus "GPU."

The test's synthetic "discrete" device instead has one device-local-only type and separate host-visible coherent types. When choosing storage for a Kairo-style mesh or Maveb-style Gaussian compute buffer, the selector can choose the device-local type. For a host upload it chooses a type satisfying host-visible and coherent flags; a host-cached preference may break a tie in the fixture. Neither result is measured hardware behavior. A real application must query its device instead of deciding that any machine labeled “discrete” necessarily has exactly this arrangement.

A memory type with `HOST_COHERENT` can simplify some host cache-management obligations, but it does not remove all queue synchronization requirements. The host must still respect access ordering and resource lifetime, and CPU readback of GPU-written storage may require observing command completion and the relevant memory dependencies. A host-visible allocation is not necessarily the optimal place for a large bandwidth-sensitive kernel, even when the device can access it. The choice depends on placement, access pattern, device architecture and measured workload cost.

Metal exposes related concerns through `MTLStorageModeShared`, `MTLStorageModePrivate`, `MTLStorageModeManaged` on supported environments, resource options and command synchronization. These are not bit-for-bit equivalent to Vulkan memory-type masks. Maveb's actual `FrameContext::create` allocates an **8 MiB shared upload buffer**, a **256 KiB shared readback buffer**, and a **64 MiB private transient heap** in the inspected source version. The `BufferSlice` contract carries resource pointer, offset, size and CPU address. Its allocator advances a cursor per frame, which is a useful example of an arena rather than the same physical behavior as our simple virtual memory-type selector.

One concrete source-review question in Maveb `FrameContext.cpp` is the local `alignUp(value, alignment)` formula `(value + alignment - 1) & ~(alignment - 1)`. This is the standard bitwise expression for a positive power-of-two alignment, but its addition must also be checked against integer overflow. Before replacing or reusing that helper, audit whether all callers and cap checks prove the addition safe and whether invalid alignments are rejected at the boundary. The source's later `allocate` code should be inspected in full before declaring a bug: a potentially dangerous expression is not automatically reachable with an invalid input. The curricular lesson is to prove arithmetic preconditions, not to accuse the project of an unverified crash.

#### A memory heap budget is an estimate, not a reservation

Khronos provides heap size and, on supported configurations, more dynamic memory-budget information. A budget can be useful for deciding whether a large mesh, Gaussian working set, image array or collision-pair output should be admitted. But it is not a guarantee that an arbitrary allocation of that size will succeed, and the reported available amount may change while the application runs. Vulkan's resource allocation may also be constrained by maximum allocation counts, dedicated requirements, device memory fragmentation, sparse residency or implementation-defined restrictions.

Our `MemoryHeap` records both a total size and an optional reported budget. If a known budget is smaller than the requested allocation, the portable selector conservatively refuses the choice. If no budget was queried, it represents that fact with `std::nullopt`, not an invented budget of zero and not a promise of infinite available memory. A selected memory type therefore means **eligible according to the available snapshot**, not allocated, resident, coherent for every operation or safe to use without further checks.

This distinction is directly relevant to Maveb's Gaussian, TSDF and mesh workloads. A neural renderer may need several concurrent large buffers: source attributes, projected splats, tile pair lists, radix-sort scratch, indirect commands and output images. Budget planning must count *all* live resources, not only the largest visible mesh. A simulation pipeline may emit a variable number of candidate collision pairs; its worst-case storage can approach quadratic growth in object count unless it checks a global count and capacity before scattering. A small NES upload need not solve that problem, which is why the cross-project workloads belong together in this chapter.

For a rigorous allocator, define a resource's memory obligation as more than payload size:

$$
M_{\text{required}} =
M_{\text{bindings}}
+
M_{\text{alignment/padding}}
+
M_{\text{temporary scratch}}
+
M_{\text{concurrently live outputs}}
+
M_{\text{deferred-retirement}}.
$$

This is a planning equation, not a Vulkan property returned in one query. It forces the programmer to include delayed releases and transient resources. The Pass 12 command journal supplies the complementary lifetime evidence: even a logically removed texture may still occupy physical memory while submitted work references it.

#### Images, formats and descriptors ask different questions

A storage buffer, a sampled image and a color attachment can contain the same nominal number of bytes yet have fundamentally different validity requirements. A buffer is an unformatted linear resource whose contents are interpreted by commands or shader declarations. An image has dimensions, format, tiling, samples, aspects, mip levels and possible compression. A shader-visible descriptor then connects a resource view to a particular binding and descriptor type. These are different contracts, not interchangeable synonyms for "GPU data."

For a renderer to sample a Kairo texture, the format must support the intended image usage and tiling. Its image must be created with compatible usage bits, memory bound, its image view created with a compatible aspect and format interpretation, and its layout and synchronization must be valid for the shader access. A descriptor of type `COMBINED_IMAGE_SAMPLER` is not magically supplied by owning a `VkImage`; the shader interface, descriptor layout, image view and sampler state must align.

For a Maveb Gaussian projection/scan stage, a storage buffer often expresses large arrays and random indexed access more naturally than a texture. But the buffer's shader-visible record layout—`std430`, scalar block layout or another appropriately enabled rule—must match the C++ encoding and the features/extensions supported by the device. A C++ struct's layout is not automatically the same as a SPIR-V shader block. The correct approach is to derive and test offsets, strides, alignment and representation for the chosen shader interface, rather than relying on the fact that two structs have fields with similar names.

For a Kyoto collision-pair append list, a counter buffer and an array of pairs are distinct resources. One may need atomic operations, bounds checks, compaction and indirect execution, depending on the actual algorithm. The work can often run headlessly with compute queues, but that does not prove every atomic width, subgroup operation or shader format used by a future implementation is supported. The broad-phase reference source `KairoPhysicsEngine/Broadphase.cppm` returns ordered candidate pairs from its CPU world. That is a useful **correctness oracle** against which a later Vulkan compute shader could be compared; it is not evidence that the Kyoto ML controller or Global Guard was ported or benchmarked in this pass.

For XR, timing and memory constraints add another dimension. Eye and gaze tracking might arrive at different update rates from the render frame; a gaze-dependent shading map needs the correct coordinate space, frame timestamp and synchronization. The ability to allocate a texture with some format says nothing about latency, compositor compatibility or gaze prediction. Those questions belong to later XR-specific tests, but they should influence how device requirements are recorded now.

#### The optional native probe uses Vulkan, not a fake success flag

A CPU-only synthetic policy test cannot prove a physical device exists. We therefore add a separate translation unit `tools/labs/gpu_course/native_vulkan_probe.cpp`. It compiles with real Vulkan headers and links `Vulkan::Vulkan` **only when CMake detects the Vulkan SDK**. Otherwise the normal portable lab still compiles and its default CTest executes without a Vulkan dependency.

When explicitly invoked with `--probe-native` on a configured environment, the native path calls Vulkan's real instance-extension enumeration, `vkCreateInstance`, `vkEnumeratePhysicalDevices`, queue-family and memory-property queries, then attempts a compatible **headless** `vkCreateDevice` and retrieves an actual queue. It creates a small 256-byte `VkBuffer` with transfer-source/storage-buffer usage, calls `vkGetBufferMemoryRequirements` to obtain the **real allocation size, alignment and type bit mask**, selects an eligible memory type, allocates actual `VkDeviceMemory` and binds it. It then releases the buffer, allocation, device and instance through explicit owning guards.

This is a meaningful new capability beyond our earlier models. It uses **real Vulkan API calls** if—and only if—the native opt-in path executes on a machine where the loader and physical device are available. It does not issue any shader dispatch, allocate a swapchain or present an image. It therefore cannot certify graphics correctness, descriptor binding, GPU synchronization or rendering performance. Native status is not smuggled into the ordinary CPU CTest result.

The exit convention is deliberate. A successful native creation/binding path returns 0 and prints the discovered device name and queried requirements. If the SDK was not available at build time or the runtime has no compatible adapter, the probe reports **UNAVAILABLE** and returns code 77. A native API operation that fails unexpectedly reports an error and returns a failure code. Code 77 is not counted as "GPU successfully tested"; it distinguishes an unavailable environment from a native creation pass. The default CTest has no `--probe-native` flag and remains a **CPU correctness test only**, even when Vulkan is installed.

Our probe uses a valid basic Vulkan 1.0 path to maximize compatibility. It makes no unsupported claim that Vulkan 1.3, 1.4, dynamic rendering, synchronization2, ray tracing, descriptor indexing or buffer device address was enabled. It does not query an actual `VkSurfaceKHR` and therefore records presentation as **not queried**. If a real project needs swapchain creation or feature-dependent shader interfaces, it must extend the probe to query and enable those exact capabilities, then execute a separate native regression.

#### Real RAII means correct destruction order even on an error branch

The difference between a passing toy function and a dependable low-level graphics routine is often the failure path. If `vkCreateBuffer` succeeds but `vkAllocateMemory` fails, who destroys the orphaned buffer? If binding fails, who frees the allocation, and in what order? If a physical adapter has no supported memory type, does the program accidentally shift by 32 bits or access outside the memory-type table? If device creation fails, is the parent Vulkan instance still destroyed?

The native probe owns the Vulkan instance, then the logical device, then a joint buffer-plus-allocation owner. A single RAII cleanup object destroys the bound buffer **before** freeing the memory. This avoids a subtle destructor ordering mistake in which two local guards are declared in the wrong sequence and automatically freed in reverse declaration order. The code does not rely on a manually remembered cleanup path in every early return; the destructor ordering is part of the class contract.

We deliberately do **not** construct the buffer first and immediately assume an allocation of exactly 256 bytes will suffice. The driver-reported `VkMemoryRequirements.size` may be larger or differently aligned. A real general-purpose allocator would decide whether to suballocate from a larger block and track the binding offset, but that would introduce additional aliasing and retirement questions. This introductory native probe allocates one compatible memory object for one small buffer because that makes its lifetime proof inspectable. A production renderer should usually avoid allocating one `VkDeviceMemory` object per tiny buffer.

The connected Kairo source inspection is especially useful here. Reading its RAII cleanup path and comparing it with the native probe demonstrates why the order of C++ destructor statements and the Vulkan valid-usage conditions matter as much as the success path. An engine that can draw a triangle while leaking an allocation or releasing bound memory early is not yet a correct resource-lifetime implementation.

#### The executable's negative cases form a genuine eligibility proof

The cross-project C++23 test creates a deliberately synthetic UMA fixture with graphics, compute and transfer capability in one family, known surface support and a memory type that is simultaneously device-local, host-visible and host-coherent. It evaluates five different workload requests—not one emulator texture—and checks that each requires the appropriate queue capabilities and memory flags. The same fixture demonstrates that memory type choice is an eligibility decision, not an allocation or benchmark.

A second synthetic discrete fixture has separate device-local and host-visible memory types and more than one queue family. It validates that a Kairo-style mesh can choose a device-local memory type, while a host staging request can choose a host-visible coherent type with a host-cached preference. It also proves that a headless collision-pair compute workload does not invent a graphics or present requirement. The XR fixture requires more operations, including presentation.

The negative cases are deliberately varied. A device with zero queue families, a zero-queue-count family, a compute-only family asked to present a scene, a missing swapchain extension and an unknown surface presentation result all have distinct typed failures. A mixed-family fixture tests that selected graphics, compute, transfer and presentation family indexes can differ. The helper does not infer parallel hardware execution simply because the queue indexes differ.

Memory tests reject zero size, bad alignment, empty memory-type bit mask, incompatible flags, nonexistent heap index, a requirement beyond a known budget and an oversized allocation. They check valid selection of memory type index 31—the highest index representable in Vulkan's 32-bit mask—while refusing invalid tables beyond the mask's representational capacity. They distinguish **unknown budget** from **known insufficient budget** and keep required flags separate from preferred ones.

These tests are CPU computations over synthetic device snapshots. Their role is to make the device and memory-choice policy inspectable, repeatable and reject invalid inputs. They do not establish that a real Apple, NVIDIA, AMD or software Vulkan implementation supports the same capabilities. The separate native opt-in path is the only component that can report actual physical device discovery.

#### Connect the evidence to all seven books rather than another NES-only detour

This lab belongs at the intersection of several books. Book 1 learns typed resource handles, C++23 error results, RAII, ownership and portable build/evidence contracts. Book 5 learns spatial collision-pair, scene and resource lifetimes. Book 6 owns the actual Vulkan/Metal device and GPU memory mechanisms, graphics/compute queues, image formats and descriptor interfaces. Book 7 uses those GPU capabilities for camera, vision, gaze and XR work. The same compiler, memory-layout and testing principles recur in Book 2's numerical geometry and Book 3's camera/mesh representations. Book 4 asks what resources and shader interfaces a physically based light-transport renderer requires.

There should not be one giant artificial example where every concept is forced into the NES emulator. Some subjects *are best learned through the actual Kairo renderer* because they depend on real graphics queues and framebuffers. Large Gaussian memory, scan buffers, transient allocations and Metal shader work belong naturally in Maveb. Broad-phase spatial indexing, pairs and CPU/GPU compaction belong in the collision research and Kairo physics code. XR latency, gaze coordinate systems and compositor behavior need an XR test bench, not a retro cartridge. Networking, web services, databases, machine learning and other domains in the seven-book system should similarly use the user's appropriate repositories and build original exercises when no existing repo genuinely represents the concept.

The central curriculum design is a set of **persistent, interoperable project threads**, not a single repo-themed tutorial. Every new lesson must identify the most authentic project that exposes its abstraction, inspect the actual relevant source revision, teach and derive the concept without jargon-only placeholders, implement a runnable controlled contract, provide negative and failure-path tests, and state what remains to be verified in the real subsystem. Cross-project comparisons should reveal how the *same C++ and systems principles* survive when the domain changes.

#### Reconstruction exercise: make five deliberate decisions

First, write the minimal capability profile for a headless physics broad-phase kernel, a Kairo windowed scene and a hypothetical Vulkan Gaussian raster/compute pipeline. For each, state whether graphics, compute, transfer and presentation are truly necessary, and distinguish a queue flag from demonstrated physical overlap. Explain why a compute-only device with no present-capable surface may be useful for a collision benchmark but unsuitable for a swapchain renderer.

Second, design a feature-query gate for a shader that uses descriptor indexing or buffer device address. State which queried feature struct and extension/core dependency enables it; do not assume a Vulkan version string means the feature is enabled. Show the fallback path if the device cannot meet that operation's needs.

Third, build a synthetic memory properties table with one combined host-visible and device-local memory type and one with separate device and staging types. For each resource, start from *queried* `memoryTypeBits` and the required flags. Derive a valid eligible memory type by hand. Repeat with the bit for the candidate type cleared and explain why the allocator must refuse it. Then introduce a known budget smaller than the request and distinguish a conservative admission refusal from the native allocation result.

Fourth, draw the object-lifetime graph for a real VkBuffer: instance, physical-device query, logical device, VkBuffer, VkMemoryRequirements, VkDeviceMemory, binding, possible in-flight command references, completion, destruction. Identify every failure branch after object creation. Compare Kairo's inspected buffer destructor ordering with its texture destructor and the safer joint RAII order demonstrated by the optional native probe. Describe exactly which native validation test should be added before declaring the Kairo code repaired.

Fifth, reason about the Maveb FrameContext: why are upload and readback buffers host-accessible but its transient heap private? How does a frame slot know when its monotonic upload cursor may safely reset? What happens if an allocation size or alignment computation can overflow? If you ported those choices to Vulkan, which flags would you require, which would you prefer, and where would you get real buffer/image memory requirements? If you further add a Gaussian scan and Kyoto collision-pair output, what bounded capacity, atomic and completion checks would prevent corrupt GPU-visible results?

The outcome should be a specific capability and resource contract for each workload, not a slogan about explicit APIs. A learner who can answer these questions is ready to start allocating Vulkan resources for an actual engine without assuming that every machine is the same.

**Primary verification sources:** [Vulkan physical devices and queue families](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html), [Vulkan device and queue specification](https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html), [Vulkan memory allocation specification](https://docs.vulkan.org/spec/latest/chapters/memory.html), [Vulkan memory allocation guide](https://docs.vulkan.org/guide/latest/memory_allocation.html), [Vulkan resource creation](https://docs.vulkan.org/spec/latest/chapters/resources.html), [VkPhysicalDeviceMemoryProperties](https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceMemoryProperties.html), [Apple Metal storage modes](https://developer.apple.com/documentation/metal/setting-resource-storage-modes), [Apple resource fundamentals](https://developer.apple.com/documentation/metal/resource-fundamentals), and the read-only connected [KairoRenderer](https://github.com/swayam8624/KairoRenderer), [Maveb](https://github.com/swayam8624/Maveb), [KairoPhysicsEngine](https://github.com/swayam8624/KairoPhysicsEngine) and [KairoGPU](https://github.com/swayam8624/KairoGPU) source snapshots. Vulkan and Metal semantics must ultimately be verified against the selected SDK version and real device.

**Continue the original Chapter 13.** The following original “BUFFERS, IMAGES, MEMORY & UPLOADS” section now has a concrete capability and allocation precondition. Its extensive image, buffer, suballocation, descriptor, pipeline, shader and attachment explanations remain in full. Chapter 14 can then take the real VkDevice and correctly bound VkBuffer further toward command recording, synchronization, submission and retirement—with independent GPU execution evidence rather than promoting the new device-creation probe into a completed renderer.
