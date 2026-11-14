# 13 — Open Vulkan Devices and Describe Resources

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|Next chapter]]

Vulkan begins by making device and presentation choices explicit, then ties resource descriptions to memory properties and descriptors. This chapter moves from instance/device setup through resource creation to the small set of state needed at draw time.

**By the end:** Trace Vulkan ownership from instance and device selection through swapchain, memory, descriptors, and draw-ready resource state.

**Recall question:** Which Vulkan objects describe a resource, which hold its bytes, and which make it available to a shader?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 25731–30267; source-span SHA256 a6c14aa70517aa292f103ef2f8e76050be8380f22566c3fbd4dd35245b506b11. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Open the device and describe its storage

**Clarification before the allocation examples:** device-local and host-visible are independent memory properties and can coexist. The explicit-execution narrative above defines the distinction.

### + M135

This pass begins **Phase 23: Vulkan — Objects, Resources, Descriptors, Commands, Synchronization, Presentation, and Profiling**. The exact ownership is: **M133 = Parts 2201–2220**, covering Vulkan's explicit design, instance/device/queues/surface/swapchain; **M134 = Parts 2221–2243**, covering buffers, images, memory, staging and resource classes; and **M135 = Parts 2244–2264**, covering descriptors, push constants, pipeline layouts, SPIR-V, graphics pipeline state, pipeline caches, dynamic rendering versus render passes/subpasses, and attachment load/store semantics.

The original curriculum sequence confirms the boundary: swapchain recreation ends at Part 2220, Vulkan resources/memory occupy 2221–2243, descriptors begin at 2244, and Part 2264 is attachment load/store. **Command pools begin at 2265, so command-buffer lifecycle belongs to Pass 12 rather than this pass.**

The important mental change is that Vulkan does not try to infer what you meant.

It asks you to state it.

That is the entire API philosophy.

---

### VULKAN PHILOSOPHY, INSTANCE, DEVICES, QUEUES, SURFACE & SWAPCHAIN

**Vulkan's central philosophy**

Older high-level graphics APIs tend to maintain substantial hidden state.

You might say:

```cpp
BindTexture(...);
Draw(...);
```

and the driver internally figures out:

- resource transitions,
- state validation,
- pipeline compilation,
- synchronization,
- memory placement,
- command batching.

Vulkan moves much of that responsibility toward the engine.

Conceptually:

$$
\boxed{
\text{more programmer responsibility}
\Longrightarrow
\text{less runtime guesswork}
}
$$

The intended advantages are:

$$
\boxed{
\text{predictability}
+
\text{parallel command construction}
+
\text{explicit synchronization}
+
\text{lower driver overhead}
}
$$

but the cost is also real:

$$
\boxed{
\text{more objects}
+
\text{more lifetime management}
+
\text{more ways to be subtly wrong}.
}
$$

---

**Vulkan is not “OpenGL with more parameters”**

The API is architecturally different.

OpenGL-like thinking:

```text
global context state
→ issue command
→ driver interprets current state
```

Vulkan thinking:

```text
construct immutable-ish state objects
→ explicitly bind resources/state
→ record commands
→ explicitly submit
→ explicitly synchronize
```

That is why a Vulkan renderer tends to have objects such as:

```text
Instance
PhysicalDevice
Device
Queue
Surface
Swapchain
Image
ImageView
Buffer
DeviceMemory
DescriptorSetLayout
DescriptorPool
DescriptorSet
PipelineLayout
Pipeline
RenderPass
Framebuffer
```

Each object corresponds to a real contract with the API.

---

**The Vulkan object hierarchy**

A useful ownership graph is:

```text
VkInstance
    ↓
VkPhysicalDevice
    ↓
VkDevice
    ↓
VkQueue

VkInstance
    ↓
VkSurfaceKHR
    ↓
VkSwapchainKHR
        ↓
    swapchain VkImage objects
        ↓
    VkImageView objects

VkDevice
    ↓
Buffers / Images
    ↓
VkDeviceMemory

VkDevice
    ↓
Descriptor layouts / pools / sets
    ↓
Pipeline layout
    ↓
Graphics / compute pipelines
```

Not all arrows imply strict C++ ownership, but they express API dependency.

Destroying things in the wrong order can violate object lifetime rules.

---

**`VkInstance`**

`VkInstance` is the connection between your application and the Vulkan implementation.

It does **not** represent the GPU.

It exists before physical-device selection.

You provide metadata:

```cpp
VkApplicationInfo app{};
app.sType =
    VK_STRUCTURE_TYPE_APPLICATION_INFO;

app.pApplicationName =
    "Kairo";

app.pEngineName =
    "Kairo";

app.apiVersion =
    VK_API_VERSION_1_1;
```

Then:

```cpp
VkInstanceCreateInfo create{};
create.sType =
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

create.pApplicationInfo =
    &app;
```

and:

```cpp
vkCreateInstance(
    &create,
    nullptr,
    &instance);
```

Kairo currently targets:

$$
\boxed{\text{Vulkan 1.1}}
$$

in `VulkanInstance.cppm`.

---

**Why instance extensions exist**

The Vulkan core cannot know every platform/window-system integration mechanism.

So optional capabilities are exposed through extensions.

For example, creating a drawable surface requires platform-specific window-system support.

GLFW provides the required instance extension names.

Kairo takes those surface extensions and passes them into instance creation.

This is exactly the correct architecture:

```text
Window system
    ↓
required Vulkan instance extensions
    ↓
VkInstance
```

---

**Extension discovery is mandatory**

You should not blindly enable an extension because your development machine supports it.

The general pattern is:

```cpp
uint32_t count = 0;

vkEnumerateInstanceExtensionProperties(
    nullptr,
    &count,
    nullptr);

std::vector<VkExtensionProperties>
extensions(count);

vkEnumerateInstanceExtensionProperties(
    nullptr,
    &count,
    extensions.data());
```

Then test by name.

Kairo's `HasExtension()` does exactly this.

---

**MoltenVK portability enumeration**

This matters specifically because you are developing on macOS.

Vulkan is not natively exposed by macOS in the same way Metal is.

MoltenVK implements Vulkan over Metal.

Portable Vulkan implementations may expose devices through:

$$
VK_KHR_portability_enumeration.
$$

Kairo checks for:

```cpp
VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
```

and if present adds:

```cpp
VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
```

to the instance flags.

Without this, physical-device enumeration through portability implementations may not behave as expected.

This is a particularly important cross-platform detail in your current setup.

---

**Validation layers**

Vulkan deliberately performs relatively little runtime validation in normal production operation.

Incorrect usage may produce:

- validation errors,
- undefined results,
- GPU hangs,
- device loss,
- silent corruption.

During development, use:

$$
\boxed{
VK_LAYER_KHRONOS_validation
}
$$

when available.

Kairo checks whether the validation layer exists before enabling it.

Good.

---

**Validation is not the graphics driver**

Validation layers sit between your application and implementation.

Conceptually:

```text
application
    ↓
validation layers
    ↓
Vulkan implementation
    ↓
driver / GPU
```

They inspect API use and report mistakes such as:

- wrong object lifetime,
- missing synchronization,
- invalid descriptor bindings,
- layout mismatches,
- wrong render-pass compatibility,
- out-of-range API parameters.

They do not make invalid code valid.

They diagnose it.

---

**Kairo validation nuance**

Kairo currently enables validation only when both:

```text
VK_LAYER_KHRONOS_validation
```

and:

```text
VK_EXT_debug_utils
```

are available.

That makes sense for a debug-capable configuration.

Later M138 will go deeper into debug messengers and diagnostic naming.

---

**Physical devices**

A:

$$
VkPhysicalDevice
$$

represents a physical or logical Vulkan-capable adapter exposed by the implementation.

Examples could correspond to:

- discrete GPU,
- integrated GPU,
- software implementation,
- portability device.

You do not create a `VkPhysicalDevice`.

You enumerate it:

```cpp
uint32_t count = 0;

vkEnumeratePhysicalDevices(
    instance,
    &count,
    nullptr);
```

Then fetch handles.

Kairo does exactly this.

---

**Physical-device selection should be capability driven**

A common tutorial mistake is:

```cpp
device = devices[0];
```

Kairo correctly does not assume the first device works.

It checks each candidate for:

$$
\text{graphics queue}
$$

and:

$$
\text{presentation support}
$$

and:

$$
VK_KHR_swapchain
$$

and:

$$
independentBlend.
$$

Only then does it select the adapter.

That is much better than hardcoding enumeration order.

---

**Device properties versus device features**

These are different concepts.

**Properties** describe characteristics/limits:

```text
device name
device type
vendor/device IDs
maximum image dimensions
maximum descriptor counts
timestamp period
alignment limits
subgroup characteristics
```

These are mostly facts about the implementation.

**Features** are capabilities that may need to be explicitly enabled:

```text
geometry shaders
sampler anisotropy
independent blend
wide lines
fill mode non-solid
```

You query support before device creation, then enable selected features when creating the logical device.

---

**`vkGetPhysicalDeviceProperties`**

General usage:

```cpp
VkPhysicalDeviceProperties props{};

vkGetPhysicalDeviceProperties(
    physical,
    &props);
```

Then:

```cpp
props.deviceName;
props.deviceType;
props.limits.maxImageDimension2D;
```

These limits should feed engine capability decisions.

Do not assume desktop-NVIDIA-style limits on every backend.

---

**`vkGetPhysicalDeviceFeatures`**

Kairo currently does:

```cpp
VkPhysicalDeviceFeatures
supported{};

vkGetPhysicalDeviceFeatures(
    candidate,
    &supported);
```

and specifically requires:

```cpp
supported.independentBlend ==
VK_TRUE
```

because its viewport writes multiple attachments whose blend behavior differs.

That is a great example of feature selection being driven by renderer semantics rather than by “enable everything.”

---

**Why enabling every feature is poor practice**

The logical device declares the feature contract your application intends to use.

A feature should generally be enabled because some engine path requires it.

That makes:

- portability clearer,
- compatibility checks easier,
- fallback design explicit.

Kairo enables:

$$
\boxed{
independentBlend
}
$$

because object-ID and color attachments need distinct blending behavior.

That is disciplined capability use.

---

**Queue families**

A physical device exposes one or more queue families.

Each family supports some subset of:

$$
VK_QUEUE_GRAPHICS_BIT,
$$

$$
VK_QUEUE_COMPUTE_BIT,
$$

$$
VK_QUEUE_TRANSFER_BIT,
$$

and possibly other operations.

Presentation support is surface-specific and queried separately.

This is important:

$$
\boxed{
\text{presentation capability is not merely a queue flag}.
}
$$

You ask:

```cpp
vkGetPhysicalDeviceSurfaceSupportKHR(
    physical,
    family,
    surface,
    &supported);
```

Kairo correctly does this.

---

**Graphics, compute and transfer queues**

A graphics-capable queue generally supports graphics work and often compute/transfer too.

A compute-only queue may exist.

A transfer-only queue may exist.

Why care?

Dedicated compute/transfer families can sometimes allow:

- concurrent work,
- lower contention,
- specialized scheduling.

But they also introduce:

- queue-family ownership transfers,
- more synchronization,
- more scheduling complexity.

Therefore:

$$
\boxed{
\text{dedicated queue}
\neq
\text{automatic performance win}.
}
$$

---

**Kairo's current queue model**

Kairo finds:

$$
\boxed{
\text{one graphics family}
}
$$

and:

$$
\boxed{
\text{one present family}.
}
$$

These may be the same or different.

It does **not** yet separately search for:

- dedicated compute family,
- dedicated transfer family.

That is reasonable for the renderer's current scope.

General Vulkan engine design should still understand those distinctions.

---

**Queue-family selection heuristic**

A more advanced engine may conceptually prefer:

```text
graphics:
    family with GRAPHICS

compute:
    COMPUTE but preferably not GRAPHICS

transfer:
    TRANSFER but preferably neither GRAPHICS nor COMPUTE
```

because dedicated families may map to specialized hardware queues.

But if no dedicated queue exists:

$$
\boxed{
\text{fallback to shared graphics family}.
}
$$

Capability-driven fallback is essential.

---

**Logical device**

`VkDevice` is your application's logical interface to one selected physical device.

You tell Vulkan:

- which queue families you need,
- how many queues,
- which device extensions,
- which features.

Kairo builds one `VkDeviceQueueCreateInfo` per unique graphics/present family.

If both families are identical, only one queue-family request is created.

That avoids redundant queue-create entries.

---

**Device queues**

After device creation:

```cpp
vkGetDeviceQueue(
    device,
    graphicsFamily,
    0,
    &graphicsQueue);
```

and similarly for present queue.

A `VkQueue` is where commands are eventually submitted.

Do not think of it as a CPU thread.

It is a logical submission channel into GPU execution.

---

**Multiple queues from one family**

A family may expose:

$$
queueCount>1.
$$

You can request multiple logical queues.

That does not necessarily imply completely independent physical engines.

The implementation may schedule them onto shared hardware.

Again:

$$
\boxed{
\text{logical concurrency}
\neq
\text{guaranteed hardware parallelism}.
}
$$

---

**Window-system surfaces**

`VkSurfaceKHR` connects Vulkan presentation to a native window.

It belongs conceptually to:

$$
VkInstance.
$$

It does not itself contain rendered pixels.

It describes:

> the window-system target to which swapchain images may eventually be presented.

Kairo uses:

```cpp
glfwCreateWindowSurface(...)
```

and wraps the resulting handle in RAII.

---

**Surface lifetime**

Kairo stores:

```cpp
VkInstance m_Instance;
VkSurfaceKHR m_Handle;
```

and destroys with:

```cpp
vkDestroySurfaceKHR(...)
```

before the instance disappears.

This is exactly the sort of explicit lifetime dependency Vulkan makes visible.

---

**Surface capabilities**

Before creating a swapchain, query:

```cpp
VkSurfaceCapabilitiesKHR caps{};

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physical,
    surface,
    &caps);
```

This gives information such as:

$$
minImageCount,
$$

$$
maxImageCount,
$$

$$
currentExtent,
$$

$$
minImageExtent,
$$

$$
maxImageExtent,
$$

$$
currentTransform.
$$

Swapchain configuration must fit these limits.

---

**Swapchain extent**

Sometimes:

```cpp
capabilities.currentExtent
```

already specifies the required extent.

Otherwise the application selects one constrained by:

$$
[minImageExtent,maxImageExtent].
$$

Kairo correctly does:

$$
width=
clamp(
framebufferWidth,
minWidth,
maxWidth
)
$$

and similarly for height.

---

**Why framebuffer size rather than logical window size matters**

On HiDPI displays:

$$
\text{window points}
\neq
\text{framebuffer pixels}.
$$

Rendering targets need physical framebuffer dimensions.

Using logical window dimensions can produce scaling bugs.

GLFW exposes framebuffer extent separately for this reason.

---

**Surface formats**

A surface exposes supported:

$$
(format,colorSpace)
$$

pairs.

Kairo prefers:

```text
VK_FORMAT_B8G8R8A8_SRGB
+
VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
```

and otherwise falls back to first available format.

This matters because swapchain images are display-facing resources.

---

**sRGB swapchain format**

With an sRGB format, color written from the shader is treated as linear input to the attachment and encoded appropriately for storage/display behavior according to the API's sRGB rules.

That is why your lighting should remain in linear space before final output.

This connects directly to Pass 3.

---

**Present modes**

Important present modes conceptually include:

$$
FIFO,
$$

$$
MAILBOX,
$$

$$
IMMEDIATE
$$

depending on platform.

`FIFO` behaves like queued vsync and is the universally required safe mode.

`MAILBOX` keeps a pending image model that can reduce latency without tearing when supported.

Kairo:

$$
\boxed{
MAILBOX\text{ if available}
}
$$

otherwise:

$$
\boxed{
FIFO.
}
$$

A sensible choice.

---

**Swapchain creation**

Swapchain is created from:

```cpp
VkSwapchainCreateInfoKHR
```

with:

- surface,
- image count,
- format,
- color space,
- extent,
- image usage,
- sharing mode,
- transform,
- present mode.

Kairo requests:

```cpp
VK_IMAGE_USAGE_TRANSFER_DST_BIT |
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
```

which means swapchain images can be rendering destinations and transfer destinations.

---

**Swapchain image count**

Kairo chooses:

$$
imageCount=
minImageCount+1
$$

then clamps to `maxImageCount` if nonzero.

The intuition is to request one additional image beyond the minimum to reduce chances that rendering blocks waiting for the presentation engine.

This is related to buffering, but swapchain image count is not exactly equivalent to your own frames-in-flight count.

Keep those concepts separate.

---

**Graphics and present queue-family sharing**

If graphics and present families are the same:

$$
VK_SHARING_MODE_EXCLUSIVE.
$$

If different, Kairo uses:

$$
VK_SHARING_MODE_CONCURRENT
$$

with both family indices.

Concurrent sharing avoids explicit ownership transfer between those families.

Potential cost is less optimized ownership assumptions.

An advanced renderer may choose exclusive mode plus explicit transfers, but concurrent mode simplifies the implementation.

---

**Swapchain images are not allocated by your code**

After creation:

```cpp
vkGetSwapchainImagesKHR(...)
```

returns image handles.

These images are owned by the swapchain implementation.

You do not call `vkDestroyImage()` on them.

You do, however, usually create image views for them.

Kairo does exactly that.

---

**Swapchain image views**

For every swapchain image, Kairo creates:

```cpp
VkImageViewCreateInfo view{};
view.image =
    image;

view.viewType =
    VK_IMAGE_VIEW_TYPE_2D;

view.format =
    swapchainFormat;
```

with color aspect.

Why image view?

Because many APIs do not bind raw image objects directly into render operations.

A view tells Vulkan:

> interpret this image using this format/subresource range/aspect.

The resource and its interpretation are separated.

---

**Acquiring a swapchain image**

Before rendering to a presentable image:

```cpp
vkAcquireNextImageKHR(...)
```

returns an image index.

This does not mean:

> CPU now owns the image forever.

It establishes presentation/rendering synchronization semantics through the provided semaphore/fence.

Kairo correctly handles:

$$
VK_ERROR_OUT_OF_DATE_KHR.
$$

It also permits:

$$
VK_SUBOPTIMAL_KHR
$$

as a usable-but-recreate-soon result.

---

**`VK_ERROR_OUT_OF_DATE_KHR`**

This commonly occurs when:

- window resized,
- display configuration changed,
- surface no longer matches swapchain configuration.

The existing swapchain cannot be used.

Kairo responds by recreating it.

Correct.

---

**`VK_SUBOPTIMAL_KHR`**

The swapchain can still function, but no longer perfectly matches surface conditions.

For example the surface characteristics may have changed.

The frame can often complete, then swapchain gets recreated.

Kairo treats this as recreation-worthy.

---

**Presenting**

After graphics work has produced the image, presentation uses:

```cpp
VkPresentInfoKHR present{};
```

then:

```cpp
vkQueuePresentKHR(
    presentQueue,
    &present);
```

Presentation is queue work.

It must wait for rendering completion through appropriate synchronization.

The exact semaphore mechanics come in Pass 12.

---

**Swapchain recreation**

Current Kairo's `Recreate()` does:

```cpp
vkDeviceWaitIdle(device);

Destroy();

Create(window);
```

This is simple and correct for its current one-frame-oriented architecture.

But it stalls the entire device.

For a more advanced multi-frame engine, you can create a replacement swapchain while old resources retire after fences/timeline values.

Kairo's source itself comments that a future multi-frame scheduler could do this.

Excellent comment.

---

**Resources depending on the swapchain must also be rebuilt**

When Kairo recreates the swapchain, it also recreates:

$$
\text{synchronization resources}
$$

and:

$$
\text{triangle renderer swapchain-dependent state}.
$$

Why?

Because framebuffers and render-pass compatibility may depend on:

- extent,
- format,
- image views.

A swapchain resize is not merely “replace one handle.”

It is a dependency-tree rebuild.

---

### BUFFERS, IMAGES, MEMORY & UPLOADS

**Vulkan separates resources from memory**

This is one of Vulkan's most important concepts.

Creating:

```cpp
VkBuffer
```

does not necessarily allocate its backing bytes.

Similarly:

```cpp
VkImage
```

describes an image resource but not necessarily its allocated memory.

The general model is:

```text
create resource
    ↓
query memory requirements
    ↓
allocate VkDeviceMemory
    ↓
bind memory to resource
```

This separation enables sophisticated memory allocators and aliasing.

---

**`VkBuffer`**

A buffer is an unformatted linear byte range.

Creation:

```cpp
VkBufferCreateInfo create{};
create.sType =
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

create.size =
    bytes;

create.usage =
    usageFlags;

create.sharingMode =
    VK_SHARING_MODE_EXCLUSIVE;
```

then:

```cpp
vkCreateBuffer(...);
```

Its meaning comes from usage and how commands/shaders interpret its bytes.

---

**Buffer usage flags**

Possible roles include:

```text
vertex data
index data
uniform data
storage data
transfer source
transfer destination
indirect arguments
texel buffer
```

A buffer can combine roles.

For example:

```cpp
VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
```

means:

> data may be copied into this buffer, then consumed as vertex input.

Usage flags are not decorative.

They tell the driver what operations will legally use the resource.

---

**Kairo's host buffer**

Current `VulkanHostBuffer` creates a buffer with caller-provided usage flags, then chooses memory requiring:

$$
HOST_VISIBLE
$$

and:

$$
HOST_COHERENT.
$$

It maps the allocation once during construction and retains:

```cpp
void* m_Mapped;
```

for its entire lifetime.

That is essentially persistent mapping.

---

**`VkImage`**

An image is not merely “a 2D pixel array.”

Its creation specifies:

$$
\text{image type},
$$

$$
\text{format},
$$

$$
\text{extent},
$$

$$
\text{mip levels},
$$

$$
\text{array layers},
$$

$$
\text{sample count},
$$

$$
\text{tiling},
$$

$$
\text{usage},
$$

$$
\text{initial layout}.
$$

These properties determine how memory is interpreted and accessed.

---

**Kairo texture image**

Kairo creates:

```cpp
imageType =
    VK_IMAGE_TYPE_2D;

extent =
    {width,height,1};

mipLevels =
    textureMipCount;

arrayLayers =
    1;

samples =
    VK_SAMPLE_COUNT_1_BIT;

tiling =
    VK_IMAGE_TILING_OPTIMAL;

usage =
    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
    VK_IMAGE_USAGE_SAMPLED_BIT;

initialLayout =
    VK_IMAGE_LAYOUT_UNDEFINED;
```

This describes a typical immutable sampled texture upload destination.

---

**Image dimensions**

Image types conceptually include:

$$
1D,\ 2D,\ 3D.
$$

A cube map is generally represented using a 2D image with six layers and cube-compatible usage/view semantics.

An array texture similarly uses:

$$
arrayLayers>1.
$$

Mipmap count:

$$
L=
1+
\lfloor
\log_2
\max(W,H,D)
\rfloor
$$

for a full chain.

---

**Mip subresources**

An image is divided into subresources by:

$$
\text{aspect},
$$

$$
\text{mip level},
$$

$$
\text{array layer}.
$$

A barrier/view/copy may affect one subset rather than the entire image.

For example:

```cpp
baseMipLevel = 2;
levelCount   = 3;
baseArrayLayer = 0;
layerCount   = 1;
```

selects mip levels 2–4.

This explicitness becomes crucial for streaming and render graphs.

---

**Image formats**

A format defines:

- channel count,
- channel size,
- numeric interpretation,
- ordering,
- possibly sRGB decoding/encoding behavior.

Examples from Kairo:

$$
VK_FORMAT_R8_UNORM,
$$

$$
VK_FORMAT_R8G8B8A8_SRGB,
$$

$$
VK_FORMAT_R16G16B16A16_SFLOAT.
$$

Kairo maps the asset's **semantic color-space metadata** to sRGB versus linear Vulkan formats.

That is exactly correct.

---

**`UNORM` versus `SFLOAT`**

`UNORM` stores integer bits interpreted as normalized values.

For 8-bit:

$$
0\rightarrow0,
$$

$$
255\rightarrow1.
$$

`SFLOAT` stores floating-point values.

Use:

$$
RGBA16F
$$

when HDR values above 1 must be represented.

Do not mistake “16-bit format” for a single numerical category.

---

**Image views**

A `VkImageView` selects how an image is accessed.

It specifies:

- view type,
- format interpretation,
- subresource range.

One underlying image may have several views.

For example, an array texture might have:

- one 2D-array view over all layers,
- separate 2D views for individual layers.

This resource/view separation is extremely useful.

---

**Kairo texture view**

Kairo creates one:

$$
VK_IMAGE_VIEW_TYPE_2D
$$

view covering:

$$
mip=0\dots L-1
$$

and:

$$
layer=0.
$$

The color aspect is selected.

So the shader can sample across the full mip chain through one view.

---

**Samplers**

A sampler describes **how** a sampled image is filtered/addressed.

This is separate from image storage.

Sampler state includes:

$$
magFilter,
$$

$$
minFilter,
$$

$$
mipmapMode,
$$

$$
addressU/V/W,
$$

$$
LOD range,
$$

$$
anisotropy,
$$

$$
comparison mode.
$$

Kairo currently uses linear min/mag and linear mip interpolation:

$$
\boxed{\text{trilinear filtering}}
$$

with configurable address modes.

---

**Sampler LOD range**

Kairo sets:

$$
minLod=0,
$$

$$
maxLod=L-1.
$$

That permits the sampler to access the entire mip chain.

The derivatives generated by fragment execution drive ordinary implicit LOD selection.

---

**Anisotropy**

Current Kairo:

```cpp
maxAnisotropy = 1.0f;
```

so anisotropic filtering is effectively disabled.

Enabling higher anisotropy generally requires:

- checking physical-device feature,
- enabling `samplerAnisotropy`,
- respecting device limit.

That would be an example of feature-driven optional quality.

---

**Device-memory heaps**

Vulkan distinguishes:

$$
\boxed{
\text{memory heaps}
}
$$

from:

$$
\boxed{
\text{memory types}.
}
$$

A heap represents a large physical memory pool/budget.

A memory type describes properties of allocations sourced from a heap.

Conceptually:

```text
heap 0: device local memory
    type 0: DEVICE_LOCAL

heap 1: host accessible memory
    type 1: HOST_VISIBLE | HOST_COHERENT
    type 2: HOST_VISIBLE | HOST_CACHED
```

Exact organization is device specific.

---

**Integrated versus discrete GPU memory types**

On a discrete GPU:

$$
DEVICE_LOCAL
$$

often means VRAM that the CPU cannot directly access efficiently.

Host-visible memory may reside in system memory or in a special host-visible region.

On an integrated/unified device, memory types may simultaneously expose combinations such as:

$$
DEVICE_LOCAL
+
HOST_VISIBLE.
$$

Never hardcode assumptions.

Query memory properties.

---

**Memory requirements**

After creating a buffer/image:

```cpp
VkMemoryRequirements req{};

vkGetBufferMemoryRequirements(
    device,
    buffer,
    &req);
```

or image equivalent.

Requirements include:

$$
size,
$$

$$
alignment,
$$

$$
memoryTypeBits.
$$

The `memoryTypeBits` mask says which memory types are legal for this resource.

---

**Finding a memory type**

Kairo's function:

```cpp
if (
    bits & (1u << index)
)
```

checks whether resource permits type `index`.

Then:

```cpp
(
propertyFlags & required
)
==
required
```

ensures requested properties are all present.

Mathematically:

$$
\boxed{
legal(i)
=
bit_i(memoryTypeBits)
\land
[(props_i\&required)=required].
}
$$

This pattern is canonical Vulkan memory selection.

---

**Host-visible memory**

$$
VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
$$

means CPU can map the allocation:

```cpp
vkMapMemory(...)
```

and access it through a pointer.

It does **not** automatically mean writes become immediately visible to the GPU without cache management.

That is where coherency matters.

---

**Host-coherent memory**

With:

$$
HOST_COHERENT
$$

you generally do not need explicit flush/invalidate calls for host/device visibility of mapped writes/reads within Vulkan's synchronization rules.

Without it, host writes may require:

```cpp
vkFlushMappedMemoryRanges(...)
```

and device-written data may require:

```cpp
vkInvalidateMappedMemoryRanges(...)
```

before CPU reads.

Kairo requests both:

$$
HOST_VISIBLE
|
HOST_COHERENT.
$$

Therefore `memcpy()` into mapped memory is sufficient from the host-cache-coherency perspective.

GPU execution ordering is still a separate concern.

---

**Coherency is not synchronization**

This deserves emphasis.

Host coherent means:

> caches do not require explicit flush/invalidate for visibility.

It does not mean:

> CPU may overwrite memory while GPU is reading it.

You still need fences/timeline ownership or per-frame buffer slices.

Similarly:

$$
\boxed{
\text{visibility/coherency}
\neq
\text{execution ordering}.
}
$$

This distinction becomes central in M137.

---

**Device-local memory**

$$
VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
$$

is desirable for frequently GPU-accessed resources.

Kairo textures allocate device-local memory.

That is the correct default for static sampled images.

The CPU therefore cannot simply `memcpy` texture pixels directly into their final image allocation.

So we need staging.

---

**Binding memory**

After allocation:

```cpp
vkBindBufferMemory(
    device,
    buffer,
    memory,
    offset);
```

or:

```cpp
vkBindImageMemory(...)
```

connects the logical resource to bytes inside a memory allocation.

The offset must satisfy alignment requirements.

Kairo currently binds dedicated allocations at:

$$
offset=0.
$$

---

**Dedicated allocation versus suballocation**

Kairo's small wrapper currently tends to perform:

```text
resource
→ query requirements
→ vkAllocateMemory
→ bind at offset 0
```

for each resource.

That is simple.

A production Vulkan allocator often instead does:

```text
large VkDeviceMemory block
    ↓
suballocate offsets
    ↓
bind many resources to slices
```

because frequent `vkAllocateMemory()` calls are expensive and device allocation counts are limited.

---

**Suballocation math**

Suppose allocation cursor:

$$
c.
$$

Resource alignment:

$$
a.
$$

Aligned start:

$$
\boxed{
o=
\left\lceil
\frac ca
\right\rceil a.
}
$$

For power-of-two alignment:

$$
\boxed{
o=
(c+a-1)\&( \sim(a-1)).
}
$$

Then resource occupies:

$$
[o,o+size).
$$

Next cursor:

$$
c'=o+size.
$$

This is the same bump-allocation principle we saw in Maveb's Metal frame arenas.

Different API; same allocator mathematics.

---

**Staging buffers**

A staging buffer is a host-visible temporary transfer source.

Static resource upload:

```text
CPU
 ↓
mapped host-visible staging buffer
 ↓ copy command
device-local buffer/image
```

This lets final GPU resource live in fast device-local memory.

Kairo texture upload follows exactly this design.

---

**Kairo texture upload**

It packs all mip pixels into one contiguous staging buffer.

For each mip it creates a `VkBufferImageCopy` with:

$$
bufferOffset
$$

and the mip's extent.

Then issues:

```cpp
vkCmdCopyBufferToImage(...)
```

covering all mip levels in one copy command sequence.

That is a good bulk-upload design.

---

**Image layouts appear during upload**

The texture starts at:

$$
VK_IMAGE_LAYOUT_UNDEFINED.
$$

Before copy:

$$
\boxed{
UNDEFINED
\rightarrow
TRANSFER_DST_OPTIMAL.
}
$$

After copy:

$$
\boxed{
TRANSFER_DST_OPTIMAL
\rightarrow
SHADER_READ_ONLY_OPTIMAL.
}
$$

Kairo performs exactly these transitions.

Detailed barrier semantics belong to M137, but the resource-state intent belongs here.

---

**Why `UNDEFINED` is useful**

If old contents do not matter, transitioning from:

$$
VK_IMAGE_LAYOUT_UNDEFINED
$$

allows the implementation to discard previous image contents.

This can avoid preserving memory unnecessarily.

Never use `UNDEFINED` when old contents must survive.

---

**Kairo's upload synchronization is intentionally simple**

Texture upload creates a temporary command pool and command buffer, submits to the graphics queue, then calls:

```cpp
vkQueueWaitIdle(...)
```

before destroying staging resources.

This is correct.

But it serializes the queue.

A high-throughput asset streamer would instead batch uploads and retire staging buffers using fences/timeline semaphores without queue-wide idle.

That optimization belongs naturally to later synchronization work.

---

**Vertex buffers**

A vertex buffer is merely a `VkBuffer` used through:

$$
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT.
$$

The pipeline's vertex-input state says how bytes are interpreted.

Your `MeshVertex` layout is:

$$
Position:float3,
$$

$$
Color:float3,
$$

$$
Normal:float3,
$$

$$
TexCoord:float2.
$$

The Vulkan pipeline specifies matching offsets and formats.

---

**Index buffers**

Index data allows multiple primitives to reuse vertices.

Kairo binds:

```cpp
vkCmdBindIndexBuffer(
    ...,
    VK_INDEX_TYPE_UINT32);
```

therefore indices are:

$$
32\text{-bit unsigned integers}.
$$

Index buffer's resource usage must permit index consumption.

---

**Uniform buffers**

Uniform buffers represent relatively small read-only shader parameter blocks.

Typical data:

- camera matrices,
- lighting constants,
- frame parameters.

Kairo has a large `CameraUniform` written through its host buffer and exposed via descriptor binding 0.

Uniform-buffer access is designed for relatively uniform reads across shader invocations.

---

**Storage buffers**

Storage buffers permit larger/general shader-readable/writable data.

Examples:

- particle arrays,
- material tables,
- visible-instance lists,
- GPU-driven draw data.

Unlike uniform buffers, storage buffers are suited to broader random/indexed access and shader writes.

Current Kairo material system does not yet use a large bindless material SSBO, but that is the natural future architecture discussed in Pass 5.

---

**Texel buffers**

A texel buffer combines a buffer with a format interpretation through a buffer view.

Instead of reading raw bytes/structs, shader accesses formatted texel values.

Useful for some large one-dimensional formatted datasets.

Less common in modern engine code than storage buffers/images, but still part of Vulkan's resource model.

---

### DESCRIPTORS, PIPELINE LAYOUTS, SHADERS & RENDERING STATE

**Descriptors solve one core question**

A shader contains something conceptually like:

```glsl
layout(set=0, binding=7)
uniform MaterialData
{
    vec4 baseColor;
};
```

The shader needs to know:

> Which actual buffer backs this declaration?

That association is represented through descriptors.

A descriptor is effectively metadata connecting a shader-visible binding slot to a resource.

---

**Descriptor types**

Important descriptor types include:

$$
UNIFORM_BUFFER,
$$

$$
STORAGE_BUFFER,
$$

$$
COMBINED_IMAGE_SAMPLER,
$$

$$
SAMPLED_IMAGE,
$$

$$
SAMPLER,
$$

$$
STORAGE_IMAGE,
$$

plus dynamic-buffer variants and others.

The descriptor type must match shader declaration semantics.

---

**Kairo's descriptor model**

Kairo's material descriptor set has ten bindings.

The important layout is approximately:

| Binding | Resource |
|---:|---|
| 0 | Camera uniform buffer |
| 1 | Shadow sampled image |
| 2 | Base-color texture |
| 3 | Normal texture |
| 4 | Metallic-roughness texture |
| 5 | Emissive texture |
| 6 | Occlusion texture |
| 7 | Material uniform buffer |
| 8 | Environment sampled image |
| 9 | Optional skin uniform buffer |

This is a conventional per-material/per-draw descriptor architecture.

---

**Descriptor-set layout**

`VkDescriptorSetLayout` defines the **shape** of a descriptor set.

For each binding:

```cpp
VkDescriptorSetLayoutBinding b{};
b.binding =
    7;

b.descriptorType =
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

b.descriptorCount =
    1;

b.stageFlags =
    VK_SHADER_STAGE_FRAGMENT_BIT;
```

The layout says:

> binding 7 exists, contains one UBO descriptor, and fragment shaders may access it.

It does not yet point to a particular buffer.

---

**Descriptor pool**

Descriptor sets are allocated from a:

$$
VkDescriptorPool.
$$

Pool creation reserves capacities by descriptor type.

If you want 100 sets each containing:

- 3 UBOs,
- 7 combined samplers,

pool needs approximately:

$$
300\text{ UBO descriptors}
$$

and:

$$
700\text{ combined image-sampler descriptors}.
$$

Kairo computes exactly this for its material set rebuild.

---

**Descriptor-set allocation**

Given a pool and one or more layouts:

```cpp
VkDescriptorSetAllocateInfo info{};
```

then:

```cpp
vkAllocateDescriptorSets(...)
```

returns actual sets.

A set has a defined layout but does not yet necessarily contain meaningful resource descriptors.

That comes through update.

---

**Updating descriptor sets**

Kairo builds:

```cpp
VkDescriptorBufferInfo
```

for buffers and:

```cpp
VkDescriptorImageInfo
```

for images/samplers.

Then fills:

```cpp
VkWriteDescriptorSet
```

and calls:

```cpp
vkUpdateDescriptorSets(...)
```

This is the “connect actual resource to slot” step.

---

**Descriptor lifetime hazard**

Suppose descriptor points to:

$$
VkImageView A.
$$

You destroy image view A while GPU still executes a draw that uses the descriptor.

That is invalid even though the descriptor set object itself still exists.

Resource lifetime must extend through all GPU uses.

Kairo's comments explicitly say descriptor pools are rebuilt only after fence-safe frame boundaries.

That is the right mindset.

---

**Kairo descriptor rebuild architecture**

`VulkanMaterialDescriptors::Rebuild()` first clears the old pool.

But its precondition says:

$$
\boxed{
\text{render fence must have completed}.
}
$$

That is essential.

Otherwise destroying pool/material buffers would invalidate resources still referenced by in-flight commands.

Later M137 will formalize deferred lifetime rules.

---

**Dynamic buffer offsets**

Suppose one large uniform buffer contains:

```text id="p11-dyn-ubo"
camera 0 at offset 0
camera 1 at offset 256
camera 2 at offset 512
...
```

A dynamic uniform-buffer descriptor can bind the same buffer descriptor, while the draw call provides a dynamic offset.

This avoids creating/updating separate descriptor sets for many small per-object blocks.

Offset must obey:

$$
minUniformBufferOffsetAlignment.
$$

---

**Why dynamic offsets matter**

Without dynamic offsets:

```text
1000 objects
→ potentially 1000 descriptor updates/sets
```

With one big ring buffer:

```text
one descriptor
+ per-draw offset
```

This can reduce descriptor management overhead substantially.

Kairo's current material system instead creates separate small host buffers and sets per draw/material instance.

Simple, but not maximally scalable.

---

**Descriptor indexing / bindless**

Traditional descriptors require resources to be bound into relatively fixed slots.

Bindless-style systems instead expose a large descriptor array:

```glsl
layout(set=0,binding=0)
uniform sampler2D textures[];
```

Material record stores:

$$
textureIndex.
$$

Shader:

```glsl
color =
    texture(
        textures[
            nonuniformEXT(material.textureIndex)
        ],
        uv);
```

This makes material data far more GPU-driven.

We already explored this architecturally in Pass 5.

---

**Descriptor indexing is not one feature bit**

A robust Vulkan bindless design may involve capabilities such as:

- runtime descriptor arrays,
- partially bound descriptors,
- variable descriptor counts,
- nonuniform indexing,
- update-after-bind.

These need to be queried/enabled appropriately.

Kairo currently does not use this path.

Its descriptor system is deliberately conventional.

---

**Push constants**

Push constants are a tiny block of command-supplied shader data associated with pipeline layout.

They are ideal for:

- per-draw matrix,
- object ID,
- small scalar constants.

Kairo uses a push block containing its mesh/model transform.

---

**Kairo push constants**

Pipeline layout declares:

```cpp
VkPushConstantRange
{
    VK_SHADER_STAGE_VERTEX_BIT |
    VK_SHADER_STAGE_FRAGMENT_BIT,

    0,

    sizeof(MeshPushConstants)
}
```

Earlier we verified this is:

$$
128\text{ bytes}
$$

in the current design.

Then draw code executes:

```cpp
vkCmdPushConstants(...)
```

before each mesh draw.

---

**Why push constants differ from uniform buffers**

Uniform buffer:

```text
resource object
→ descriptor
→ memory load
```

Push constant:

```text
small command-recorded value
→ pipeline layout range
```

Push constants are limited in size.

They are excellent for data changing extremely frequently.

Do not try to store an entire material database in them.

---

**Pipeline layout**

A:

$$
VkPipelineLayout
$$

defines the shader-visible interface at the pipeline level.

It contains:

$$
\text{descriptor-set layouts}
$$

and:

$$
\text{push-constant ranges}.
$$

Think of it as the ABI between command recording and shaders.

If shader expects set/binding layout incompatible with pipeline layout, the engine contract is broken.

---

**Kairo pipeline layout**

Kairo currently uses:

$$
1
$$

descriptor-set layout:

```text
material/camera/shadow/environment/skin set
```

plus one push-constant range.

This same layout is shared across mesh/shadow pipeline variants.

That allows common draw-binding logic.

---

**Shader modules**

Vulkan consumes shader code in SPIR-V form.

SPIR-V is a binary intermediate representation.

Pipeline creation references a:

$$
VkShaderModule.
$$

Kairo loads `.spv` files into:

```cpp
std::vector<uint32_t>
```

and passes the words to:

```cpp
vkCreateShaderModule(...)
```

Exactly correct.

---

**Why SPIR-V is 32-bit word aligned**

SPIR-V binary is defined as a stream of 32-bit words.

Kairo checks file byte count is divisible by:

$$
sizeof(uint32_t).
$$

This is a useful sanity check before creating the module.

---

**Shader module is not the final executable pipeline**

`VkShaderModule` represents shader IR.

Graphics pipeline creation combines shader stages with:

- vertex input,
- assembly,
- rasterizer state,
- multisampling,
- depth/stencil,
- blending,
- render-pass compatibility,
- pipeline layout.

Driver may perform compilation/optimization during pipeline creation.

This is why pipeline creation can be expensive.

---

**Graphics pipeline state**

A Vulkan graphics pipeline packages a huge amount of state that older APIs allowed to mutate independently.

Conceptually:

```text
shader stages
vertex-input interpretation
input assembly
viewport/scissor policy
rasterization
multisampling
depth/stencil
color blending
pipeline layout
rendering/render-pass compatibility
```

Some state can be marked dynamic.

Everything else is baked into the pipeline.

---

**Shader stages**

Kairo regular mesh pipeline uses:

$$
vertex
$$

plus:

$$
fragment.
$$

Shadow pipeline uses vertex only because it writes no color.

That is efficient.

No fragment stage is needed when only depth is relevant and rasterization rules permit omission.

---

**Input assembly**

Kairo opaque mesh pipeline:

$$
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST.
$$

Debug pipeline:

$$
VK_PRIMITIVE_TOPOLOGY_LINE_LIST.
$$

Input assembly interprets indices/vertices into primitives before rasterization.

---

**Rasterization state**

Kairo currently configures:

$$
polygonMode=FILL,
$$

$$
cullMode=NONE,
$$

$$
frontFace=CCW,
$$

$$
lineWidth=1.
$$

Disabling culling simplifies double-sided and current rendering behavior but sacrifices potential triangle rejection.

Material-dependent culling would require pipeline variants or dynamic cull state if supported/enabled.

---

**Vertex-input state**

Kairo's main vertex binding:

$$
stride=sizeof(MeshVertex).
$$

Attributes:

$$
location0:
R32G32B32_SFLOAT
\rightarrow Position,
$$

$$
location1:
R32G32B32_SFLOAT
\rightarrow Color,
$$

$$
location2:
R32G32B32_SFLOAT
\rightarrow Normal,
$$

$$
location3:
R32G32_SFLOAT
\rightarrow TexCoord.
$$

This is the CPU↔GPU vertex ABI.

If `offsetof()` disagrees with asset/buffer layout, vertices decode incorrectly.

---

**Skinning uses a second vertex binding**

Kairo skinned pipeline adds binding 1 with:

$$
Joints:
R32G32B32A32_UINT,
$$

$$
Weights:
R32G32B32A32_SFLOAT.
$$

Both bindings use per-vertex input rate.

This is a nice example of multiple vertex streams.

The static mesh buffer and skin influence buffer remain separate.

---

**Depth/stencil state**

Kairo uses:

$$
depthTestEnable=TRUE,
$$

$$
depthCompareOp=LESS.
$$

Opaque pipeline:

$$
depthWriteEnable=TRUE.
$$

Transparent pipeline:

$$
depthWriteEnable=FALSE.
$$

That matches our Pass 2 analysis.

Transparent fragments test against opaque depth but do not overwrite it.

---

**Kairo does not currently use reversed-Z in this Vulkan path**

The depth compare is:

$$
LESS
$$

with conventional depth behavior.

Your Maveb Metal pipeline has reverse-Z work, but current Kairo Vulkan mesh pipeline remains conventional.

Do not mix assumptions between those projects.

---

**Blend state**

Opaque color attachment:

$$
blendEnable=FALSE.
$$

Transparent:

$$
src=SRC_ALPHA,
$$

$$
dst=ONE_MINUS_SRC_ALPHA,
$$

$$
op=ADD.
$$

Thus:

$$
C_{out}
=
\alpha_sC_s
+
(1-\alpha_s)C_d.
$$

For alpha channel Kairo uses:

$$
src=ONE,
$$

$$
dst=ONE-\alpha_s.
$$

---

**Independent blending**

Kairo has two attachments in the viewport path:

- shaded color,
- object ID.

Color can blend.

Object IDs must remain exact.

Therefore attachment 0 and attachment 1 have different blend behavior.

That is why Kairo explicitly requires:

$$
\boxed{independentBlend}
$$

during physical-device selection.

This is a perfect example of an API feature requirement being traceable to renderer semantics.

---

**Dynamic state**

Kairo marks:

$$
viewport
$$

and:

$$
scissor
$$

dynamic.

Shadow pipeline additionally makes:

$$
depthBias
$$

dynamic.

Therefore those values can change at command-recording time without creating a new pipeline object.

Everything not dynamic remains baked.

---

**Why not make everything dynamic?**

Historically Vulkan exposed limited dynamic state; newer extensions/core versions expand it considerably.

Even when available, fully dynamic state is not automatically ideal.

Static state lets driver optimize more aggressively and makes pipeline behavior predictable.

Dynamic state reduces pipeline permutations.

It is an engine-design tradeoff.

---

**Pipeline creation cost**

`vkCreateGraphicsPipelines()` can trigger significant compilation.

Inputs include shaders and state.

That can involve:

- shader compilation,
- specialization,
- hardware ISA generation,
- state optimization.

Doing this during gameplay can cause stutter.

Pipelines should usually be:

- created ahead of time,
- cached,
- prewarmed,
- asynchronously compiled where architecture permits.

---

**Current Kairo pipeline variants**

Kairo builds variants for:

```text
static opaque
skinned opaque
static transparent
skinned transparent
static shadow
skinned shadow
debug lines
```

This is a manageable permutation set.

It has not yet exploded features such as normal-map on/off, metallic on/off, emissive on/off into separate pipelines.

That is good.

---

**Pipeline caches**

Vulkan lets you supply:

$$
VkPipelineCache
$$

to pipeline creation.

The implementation may store reusable compilation artifacts.

A cache can potentially be serialized and reused between runs, subject to compatibility requirements.

Kairo currently calls:

```cpp
vkCreateGraphicsPipelines(
    device,
    VK_NULL_HANDLE,
    ...
);
```

So:

$$
\boxed{
\text{current Kairo uses no pipeline cache}.
}
$$

That is a concrete future optimization.

---

**Pipeline cache is not magic portable shader binary storage**

Cache data is implementation/device/driver dependent.

You cannot assume a pipeline-cache blob from one GPU/driver is valid on another.

The engine needs cache identity/version handling.

Think:

$$
\boxed{
\text{implementation-specific acceleration artifact}
}
$$

not:

$$
\text{universal compiled shader package}.
$$

---

**Dynamic rendering**

Modern Vulkan can begin rendering without constructing a traditional `VkRenderPass`/`VkFramebuffer` pair.

Instead the command specifies current attachments directly.

Conceptually:

```text
old model:
VkRenderPass
+ VkFramebuffer
→ begin render pass

dynamic rendering:
attachment descriptions at command time
→ begin rendering
```

This substantially reduces boilerplate and render-pass object management.

---

**Dynamic rendering does not remove attachment semantics**

You still need to know:

- image format,
- layout,
- load operation,
- store operation,
- synchronization,
- sample count.

Dynamic rendering removes the traditional render-pass object structure.

It does **not** make rendering state implicit again.

---

**Current Kairo uses traditional render passes**

Current `VulkanTriangle` creates:

```cpp
vkCreateRenderPass(...)
```

and creates framebuffers per swapchain image.

Pipelines reference compatible render passes.

Therefore current Kairo does **not** use Vulkan dynamic rendering.

That is completely valid, especially for Vulkan 1.1 portability.

---

**Traditional render pass**

A `VkRenderPass` describes:

$$
\text{attachments},
$$

$$
\text{subpasses},
$$

$$
\text{dependencies}.
$$

Attachment description includes:

- format,
- sample count,
- load/store operations,
- initial/final layouts.

A framebuffer supplies the actual image views used by that render pass.

So:

$$
\boxed{
\text{render pass = structural rendering contract}
}
$$

while:

$$
\boxed{
\text{framebuffer = concrete attachment instances}.
}
$$

---

**Kairo swapchain render pass**

Its swapchain pass has one color attachment.

It begins:

$$
initialLayout=
UNDEFINED.
$$

Rendering uses:

$$
COLOR_ATTACHMENT_OPTIMAL.
$$

Ends:

$$
finalLayout=
PRESENT_SRC_KHR.
$$

That is the canonical render-to-present lifecycle.

---

**Why initial `UNDEFINED` is valid here**

Attachment uses:

$$
loadOp=CLEAR.
$$

Previous swapchain image contents are irrelevant.

Therefore:

$$
UNDEFINED
$$

is appropriate.

The implementation may discard old contents freely.

If you wanted to preserve old image contents, this would be wrong.

---

**Subpasses**

A render pass may contain multiple subpasses.

Conceptually:

```text
subpass 0:
    geometry writes attachments

subpass 1:
    lighting consumes prior attachment as input

subpass 2:
    composition
```

Subpasses historically let tile-based/mobile GPUs keep data in on-chip tile memory and avoid external memory round trips.

They also formalize attachment dependencies.

---

**Why modern engines sometimes prefer dynamic rendering instead**

Subpasses are powerful but make render-pass compatibility/state management more rigid.

Modern explicit APIs increasingly favor flexible pass graphs and dynamic rendering plus explicit synchronization.

This often maps better to complex cross-platform render-graph systems.

But traditional subpasses remain important to understand because:

- many engines use them,
- Vulkan compatibility rules reference them,
- tile-based architectures may benefit.

---

**Attachment load operation**

At beginning of rendering:

$$
loadOp
$$

specifies what happens to existing attachment contents.

Main choices conceptually:

$$
LOAD,
$$

$$
CLEAR,
$$

$$
DONT_CARE.
$$

`LOAD` preserves previous contents.

`CLEAR` initializes to specified clear value.

`DONT_CARE` says previous values need not be preserved.

---

**Attachment store operation**

At end:

$$
storeOp
$$

states whether rendered results must be preserved.

$$
STORE
$$

keeps results for later use/presentation.

$$
DONT_CARE
$$

allows implementation to discard them.

Using `STORE` unnecessarily can force memory traffic, especially on tile-based GPUs.

---

**Load/store are performance semantics, not just correctness flags**

Imagine a mobile tile renderer.

Color attachment begins in system memory.

If:

$$
loadOp=CLEAR,
$$

the GPU need not load old color from memory.

It can clear tile storage on-chip.

If final result will never be read:

$$
storeOp=DONT_CARE
$$

means no writeback required.

Thus correct load/store selection can eliminate significant bandwidth.

This is one reason explicit APIs expose them.

---

**Kairo swapchain attachment**

Current main swapchain attachment:

$$
loadOp=CLEAR,
$$

$$
storeOp=STORE.
$$

Correct:

- old frame contents irrelevant,
- final image must survive for presentation.

---

**Object-ID attachment**

For the offscreen viewport path, object-ID attachment is cleared at start and stored because tools/picking need the result later.

That also makes sense.

If the ID attachment were never read after the pass, `STORE` would be wasteful.

This is the mindset to maintain across every attachment.

---

**Depth attachment store semantics**

Depth may be:

$$
load=CLEAR
$$

and:

$$
store=DONT_CARE
$$

if no later pass samples/reads depth.

But if:

- SSAO,
- Hi-Z,
- TAA,
- picking,
- depth-aware postprocess

needs it later, then:

$$
store=STORE.
$$

Load/store decisions derive from downstream consumers.

A render graph can automate this from resource lifetime analysis.

---

**Vulkan's explicitness is dependency visibility**

At first Vulkan feels verbose because it asks you to specify:

```text
which queue?
which format?
which layout?
which memory type?
which descriptor binding?
which pipeline?
which render pass?
which load/store?
```

But every one of those questions corresponds to something the hardware/driver must know anyway.

Vulkan's philosophy is to move those decisions into a form where the engine can reason about them ahead of time.

That is why it pairs so naturally with:

- render graphs,
- explicit allocators,
- pipeline caches,
- GPU-driven rendering.

---

**Current Kairo Vulkan architecture end to end**

The current path is approximately:

```text id="p11-kairo-chain"
VulkanInstance
    ↓
GLFW Vulkan extensions
+ optional portability enumeration
+ optional validation

VulkanSurface
    ↓
physical-device enumeration

VulkanDevice
    ↓
select device with:
    graphics family
    present family
    swapchain extension
    independentBlend

VkDevice
    ↓
graphics queue
present queue

VulkanSwapchain
    ↓
surface capabilities
format
present mode
extent
images
image views

resources
    ↓
VulkanHostBuffer
VulkanTexture2D

texture upload
    ↓
host-coherent staging buffer
    ↓
copy to device-local image
    ↓
shader-read layout

descriptors
    ↓
camera
shadow
material textures
material UBO
environment
skin UBO

pipeline layout
    ↓
descriptor layout
+ push constants

SPIR-V modules
    ↓
graphics pipeline variants

traditional render pass
    ↓
framebuffers
    ↓
swapchain presentation
```

That is already a coherent explicit renderer.

---

**Where current Kairo is intentionally simple**

It currently favors correctness/clarity over maximum Vulkan sophistication.

It uses:

$$
\boxed{
\text{Vulkan 1.1}
}
$$

rather than building around newer core features.

It uses:

$$
\boxed{
\text{graphics + present queues}
}
$$

without separate compute/transfer queue architecture.

It uses:

$$
\boxed{
\text{one VkDeviceMemory allocation per simple resource}
}
$$

rather than a global suballocator.

Texture upload creates a transient command pool and waits on queue idle.

Descriptors are conventional per-draw/per-material rather than bindless.

Pipeline creation uses:

$$
VK_NULL_HANDLE
$$

pipeline cache.

Rendering uses traditional render passes rather than dynamic rendering.

These are all reasonable simplifications for an engine that is still establishing correctness.

---

**Where I would evolve Kairo next**

The highest-value resource-side step is a real Vulkan memory allocator.

Instead of:

```text
every buffer
→ vkAllocateMemory
```

use:

```text
device-local pools
host-visible upload pools
readback pools
```

with aligned suballocation.

Then static vertex/index data should move toward:

```text
host staging
→ device-local vertex/index buffers
```

instead of permanently host-visible allocations for everything.

This better matches the GPU memory hierarchy.

---

**Next descriptor evolution**

Current descriptor-per-draw design is easy to reason about.

The next scalable model would likely be:

```text
frame set
    camera/light globals

material table
    SSBO of material records

global texture descriptor array
    bindless/indexed textures

per-draw
    material ID + transform
```

This reduces descriptor-set binding frequency and fits GPU-driven draw generation.

But do this only when current architecture becomes a measured bottleneck.

---

**Next pipeline evolution**

Introduce a pipeline cache.

Potentially compile pipelines asynchronously.

Separate shader permutation identity from runtime material values.

Keep variant count controlled.

If portability target later moves beyond Vulkan 1.1, consider dynamic rendering to simplify render-target management.

But do not rewrite working render-pass infrastructure merely because newer API exists.

Solve an actual architectural problem.

---

**One subtle but important resource-lifetime issue**

Kairo texture destruction currently destroys:

```text
sampler
view
image
memory
```

in that order.

Correct conceptual dependency:

$$
sampler
$$

does not own image, but descriptors may reference both.

The engine must ensure no in-flight descriptor uses them before destruction.

RAII controls CPU lifetime.

It does **not** automatically solve GPU lifetime.

This is the single most important explicit-API lifetime concept to carry into Pass 12/13.

---

**CPU lifetime versus GPU lifetime**

Suppose:

```cpp
{
    VulkanTexture texture;
    RecordDraw(texture);
}
```

C++ scope ends.

Destructor runs.

But recorded command buffer may execute 10 ms later.

Then GPU references destroyed resources.

CPU lifetime:

$$
\text{ended}.
$$

GPU lifetime:

$$
\text{still active}.
$$

Therefore:

$$
\boxed{
\text{RAII alone is insufficient for GPU resources}.
}
$$

You need retirement synchronization.

This is why fences/timeline values become so important next.

---

**Descriptor ABI reasoning**

Shader:

```glsl
layout(set = 0, binding = 7)
uniform MaterialData
{
    vec4 baseColorFactor;
    vec4 emissiveAndNormalScale;
    vec4 factors;
    vec4 flags;
};
```

CPU layout:

$$
4\times vec4
=
64\text{ bytes}.
$$

Kairo CPU uses:

```cpp
std::array<float,16>
```

also:

$$
16\times4=64\text{ bytes}.
$$

This is an intentional ABI.

Descriptors point to bytes.

They do not understand C++ classes.

If CPU/GPU layout disagrees, Vulkan will faithfully provide the wrong bytes to your shader.

Explicit API does not rescue your struct layout.

---

**Pipeline ABI reasoning**

Pipeline layout says:

$$
set_0
=
\text{MaterialDescriptors layout}
$$

and push constants:

$$
[0,128)
$$

visible to vertex+fragment.

Every shader used with that layout must conform.

Therefore:

$$
\boxed{
\text{pipeline layout is effectively a shader-resource ABI}.
}
$$

This is a better mental model than “some Vulkan object I have to create before pipeline.”

---

**Render-pass compatibility reasoning**

A pipeline created against a traditional render pass expects compatible attachment structure.

The renderer cannot arbitrarily bind it inside an unrelated render pass.

That is one reason render-pass architecture can produce many pipeline compatibility relationships.

Dynamic rendering weakens some of that object coupling but not the underlying format/sample requirements.

---

**Swapchain is not your scene framebuffer**

This is a subtle architectural distinction.

A sophisticated renderer often renders scene into:

$$
HDR\ offscreen\ texture
$$

then tone maps/composites into swapchain.

The swapchain is presentation infrastructure.

It should not necessarily dictate all internal rendering formats.

Kairo already has offscreen viewport targets elsewhere in the rendering architecture, which is the right direction.

---

**Vulkan image view versus sampler versus descriptor**

These three are often confused.

`VkImage`:

$$
\text{storage/resource}.
$$

`VkImageView`:

$$
\text{interpretation/subresource selection}.
$$

`VkSampler`:

$$
\text{filter/addressing behavior}.
$$

Descriptor:

$$
\text{shader binding referencing view/sampler}.
$$

For combined image sampler:

$$
\boxed{
descriptor
=
(imageView,sampler,layout)
}
$$

conceptually.

Kairo's `VkDescriptorImageInfo` contains exactly those three pieces.

---

**Why descriptor contains image layout**

Kairo descriptor writes:

```cpp
{
    sampler,
    view,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
}
```

The descriptor states the layout in which the image is expected when accessed.

The actual image must truly be in that layout when shader access occurs.

Descriptor metadata does not perform the transition.

This is another crucial distinction:

$$
\boxed{
\text{descriptor declares expected state}
\neq
\text{barrier establishes state}.
}
$$

---

**Buffer memory type choice should derive from access frequency**

A static vertex buffer:

CPU writes once.

GPU reads millions of times.

Ideal:

$$
\boxed{
device-local final buffer
}
$$

with staging upload.

A frequently updated tiny uniform:

CPU writes every frame.

GPU reads once/few times.

Host-visible/coherent mapped ring buffer may be ideal.

A readback counter:

GPU writes.

CPU occasionally reads after synchronization.

Use host-visible readback memory.

Resource usage pattern should drive memory architecture.

---

**Current Kairo host buffers trade simplicity for performance**

`VulkanHostBuffer` always chooses:

$$
HOST_VISIBLE
|
HOST_COHERENT.
$$

That means even vertex/index/debug/uniform data can be straightforwardly updated.

But it does not guarantee ideal device-local bandwidth.

On some architectures host-visible memory is also device-local.

On others it is not.

A more mature allocator should distinguish:

```text
UploadBuffer
ReadbackBuffer
DeviceBuffer
DynamicUniformRing
```

rather than one universal host buffer class.

---

**Descriptor pool lifetime model**

A descriptor pool owns descriptor-set allocations.

Destroying the pool implicitly invalidates its sets.

Therefore:

```text
pool
    ↓ owns allocation storage
descriptor sets
```

Kairo's `ClearPool()` correctly clears set handles before destroying pool.

But CPU vector clearing does not itself synchronize GPU.

The method relies on its fence-safe precondition.

Again:

$$
\boxed{
\text{lifetime correctness has two layers}
}
$$

CPU ownership and GPU completion.

---

**Why descriptor pools exist at all**

If every descriptor-set allocation had to individually allocate driver memory, overhead would be high.

Pools allow bulk resource management.

They are especially useful for allocation patterns such as:

```text
per frame pool
→ reset when frame fence completes
```

or:

```text
long-lived material pool
```

Different lifetime classes can have separate pools.

---

**Pipeline creation object graph**

To create Kairo's mesh pipeline, all these contracts converge:

```text id="p11-pso-graph"
SPIR-V vertex module
SPIR-V fragment module

vertex binding/attributes

input assembly

raster state

multisample state

depth state

blend state

dynamic-state declaration

pipeline layout
    descriptors
    push constants

render-pass compatibility
```

Only after all that:

```cpp
vkCreateGraphicsPipelines(...)
```

returns a pipeline.

This is why Vulkan pipeline creation feels heavy.

The advantage is that a huge amount of state validation/compilation can happen before draw time.

---

**Draw-time state becomes relatively small**

Once pipeline/resources exist, actual draw recording mainly says:

```text
bind pipeline
bind descriptor set
set dynamic viewport/scissor
push constants
bind vertex/index buffers
draw
```

That is deliberately lightweight compared with rebuilding/validating state every draw.

This is the payoff of up-front pipeline construction.

---

**The explicit renderer lifecycle**

Before frame:

$$
\text{create stable objects/resources}.
$$

At frame time:

$$
\text{select current dynamic resources}.
$$

Then:

$$
\text{record lightweight commands}.
$$

Then:

$$
\text{submit}.
$$

Then GPU executes asynchronously.

Then resources retire only when synchronization proves they are unused.

That complete final half begins next pass.

---

**PASS 11 RECALL SHEET**

| Concept | Reconstruction |
|---|---|
| Vulkan philosophy | Explicit state, memory, synchronization and lifetime |
| `VkInstance` | Application↔Vulkan implementation connection |
| Instance extension | Optional instance-level capability |
| Validation layer | Development-time API correctness diagnostics |
| `VkPhysicalDevice` | Enumerated adapter |
| Properties | Device facts/limits |
| Features | Optional capabilities explicitly enabled |
| Queue family | Group of queues with operation capabilities |
| Present support | Surface-specific queue capability |
| `VkDevice` | Logical device created from selected physical device |
| `VkQueue` | Submission channel |
| `VkSurfaceKHR` | Window-system presentation target |
| Surface capabilities | Extent/image-count/transform constraints |
| Surface format | Image format + color space |
| Present mode | FIFO/mailbox/etc. presentation policy |
| Swapchain | Rotating presentable image set |
| Acquire | Obtain usable swapchain image index |
| Present | Queue presentation of completed image |
| Out-of-date | Swapchain no longer usable |
| Suboptimal | Still usable but should recreate |
| Buffer | Linear unformatted resource |
| Image | Structured multidimensional resource |
| Image view | Format/subresource interpretation of image |
| Sampler | Filtering/addressing state |
| Heap | Physical memory budget/pool |
| Memory type | Heap-backed memory with property flags |
| Host visible | CPU mappable |
| Host coherent | No explicit host cache flush/invalidate needed |
| Device local | Efficient GPU-local access |
| Requirements | Size, alignment, compatible memory-type mask |
| Bind memory | Attach resource to allocation/offset |
| Suballocation | Many resources from one large allocation |
| Staging | Host-visible temporary transfer source |
| Vertex buffer | Buffer consumed as vertex input |
| Index buffer | Buffer consumed as primitive indices |
| Uniform buffer | Small read-only shader parameter block |
| Storage buffer | General larger shader data |
| Texel buffer | Formatted buffer access through buffer view |
| Descriptor | Resource binding metadata |
| Descriptor set layout | Binding ABI/shape |
| Descriptor pool | Allocation arena for descriptor sets |
| Descriptor set | Concrete collection of descriptors |
| Dynamic offset | Runtime offset into dynamic buffer descriptor |
| Bindless | Large indexed descriptor/resource arrays |
| Push constant | Tiny high-frequency command-supplied data block |
| Pipeline layout | Descriptor-set + push-constant ABI |
| SPIR-V | Vulkan shader intermediate representation |
| Graphics pipeline | Shader + fixed/dynamic rendering state bundle |
| Pipeline cache | Implementation-specific pipeline compilation reuse |
| Dynamic rendering | Render directly with attachment info, no `VkRenderPass` object |
| Traditional render pass | Attachment/subpass/dependency contract |
| Subpass | Rendering phase inside one render pass |
| `LOAD` | Preserve existing attachment contents |
| `CLEAR` | Initialize attachment |
| `DONT_CARE` load | Old contents irrelevant |
| `STORE` | Preserve rendered result |
| `DONT_CARE` store | Final contents may be discarded |

---

**Blank-page mastery gate**

Without looking back, reconstruct Vulkan startup from nothing:

$$
\boxed{
VkInstance
\rightarrow
VkSurfaceKHR
\rightarrow
VkPhysicalDevice
\rightarrow
queue-family discovery
\rightarrow
VkDevice
\rightarrow
VkQueue
}
$$

and explain why presentation support cannot be inferred only from `VK_QUEUE_GRAPHICS_BIT`.

Then explain exactly why Kairo requires `independentBlend`, why it checks `VK_KHR_swapchain`, and why MoltenVK requires portability-enumeration awareness.

Then derive swapchain creation from:

$$
\text{surface capabilities}
\rightarrow
\text{format}
\rightarrow
\text{present mode}
\rightarrow
\text{extent}
\rightarrow
\text{image count}.
$$

Explain why swapchain images are not manually freed with `vkDestroyImage`, but their image views are.

Then explain:

$$
VkBuffer
\neq
VkDeviceMemory
$$

and derive the resource memory process:

$$
create
\rightarrow
requirements
\rightarrow
memory type
\rightarrow
allocate
\rightarrow
bind.
$$

Then distinguish:

$$
HOST_VISIBLE,
$$

$$
HOST_COHERENT,
$$

$$
DEVICE_LOCAL
$$

and explain why coherency does **not** remove GPU synchronization requirements.

Then reconstruct Kairo's texture upload:

$$
\text{asset mips}
\rightarrow
\text{mapped staging buffer}
\rightarrow
UNDEFINED
\rightarrow
TRANSFER_DST
\rightarrow
vkCmdCopyBufferToImage
\rightarrow
SHADER_READ_ONLY.
$$

Then explain the difference between:

$$
VkImage,
$$

$$
VkImageView,
$$

$$
VkSampler,
$$

$$
VkDescriptorImageInfo.
$$

Then reconstruct descriptors from shader declaration to actual bound resource:

$$
shader(set,binding)
\rightarrow
descriptor-set layout
\rightarrow
descriptor pool
\rightarrow
descriptor set
\rightarrow
vkUpdateDescriptorSets
\rightarrow
pipeline layout.
$$

Then explain why pipeline layout is an ABI, not merely “something Vulkan asks for.”

Then rebuild a graphics pipeline from its parts:

$$
SPIR\text{-}V
+
vertex\ input
+
assembly
+
rasterizer
+
multisampling
+
depth
+
blend
+
dynamic\ state
+
pipeline\ layout
+
rendering compatibility.
$$

Finally explain these two attachment configurations:

$$
load=CLEAR,\quad store=STORE
$$

versus:

$$
load=DONT_CARE,\quad store=DONT_CARE
$$

and identify exactly what memory traffic/correctness assumptions each permits.

If you can do all that from a blank sheet, then Vulkan stops looking like an arbitrary pile of handles. It becomes what it actually is:

$$
\boxed{
\text{a typed description of resources, execution interfaces, memory ownership and rendering contracts}.
}
$$



That will finish Phase 23 and tackle the part of Vulkan that causes most real bugs: **command pools and primary/secondary buffers, recording lifecycle, render scopes, binding and draws/dispatch, queue submissions, fences, binary/timeline semaphores, pipeline stages/access masks, execution versus memory dependencies, buffer/image barriers, queue-family ownership, Synchronization2, frames in flight, per-frame resources, deferred destruction, timestamp queries, validation synchronization diagnostics, debug naming, device loss, MoltenVK portability, the full acquire→record→submit→present frame trace, and an RAII layer that preserves—not hides—Vulkan's synchronization semantics.**


<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/12 - Commands, Async Work, and Resource Lifetimes|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work|Next chapter]]
