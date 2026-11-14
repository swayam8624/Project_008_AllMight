# 21 — Buffers, images, memory, and uploads

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/20 - GPU performance models and principled redesign|Previous chapter]] · [[Chapter Edition/03 - Game Engine/22 - Vulkan synchronization and multi-frame lifetimes|Next chapter]]

GPU resources are not just arrays; formats, usage, alignment, staging, and device-memory requirements shape whether a transfer is valid. This chapter follows buffers and images through allocation, upload, and visibility.

**By the end:** Select resource layouts and memory types from API requirements, and trace a CPU upload until the GPU may safely consume it.

**Recall question:** Which size, alignment, usage, and memory-type checks must pass before this image can be bound for rendering?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 47458–51847; source-span SHA256 673c57948ffcadffee27142340c44407edd6ec2b89ea6ab803651c34615a0ea7. -->

<!-- CHAPTER-SOURCE-BEGIN -->
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


## Record, synchronize and retire explicit work

**Clarifications before the retained examples:** ordinary reusable and one-time-submit command buffers have different completion states. Frame completion does not prove present-wait semaphore consumption. Use the image-indexed present semaphore policy and late fence-reset/error handling developed above. A FIFO deletion example assumes ordered retirement values; independent queues require joined completion or a componentwise stamp. The retained `VulkanBuffer` move-constructor fragment delegates to an assignment operator that is not supplied, so it is an incomplete illustration, not a standalone compilable class. Readback additionally needs device-write-to-host-read visibility, and depth barriers must include the actual early/late writers.

### + M138

This closes **Phase 23**.

The exact curriculum ownership is: **M136 = Parts 2265–2279**, command pools/buffers, recording, draw/dispatch, submission, fences and binary/timeline semaphores; **M137 = Parts 2280–2292**, synchronization stages/accesses, execution versus memory dependencies, barriers/layouts/queue ownership, Synchronization2, frames in flight and deferred destruction; **M138 = Parts 2293–2300**, timestamp queries/readback, synchronization validation, debug naming, device loss, MoltenVK, full frame tracing and correct Vulkan RAII.

The original contract confirms the sequence: command pools begin at 2265; queue submission/fences/semaphores close M136 at 2279; synchronization starts at 2280; frame lifetime occupies 2290–2292; diagnostics and explicit RAII finish the phase at 2300.

This is arguably the most important Vulkan pass.

Most Vulkan bugs are not:

> My matrix multiplication is wrong.

They are:

> This resource was used too early, destroyed too early, transitioned incorrectly, waited at the wrong stage, reused while still in flight, or made visible without actually ordering the producer.

The central equation for this pass is therefore not numerical.

It is:

$$
\boxed{
\text{Correct GPU execution}
=
\text{ordering}
+
\text{memory visibility}
+
\text{lifetime}
}
$$

and all three are distinct.

---

### COMMAND POOLS, COMMAND BUFFERS, SUBMISSION & SEMAPHORES

**Vulkan commands are recorded before execution**

When you call:

```cpp
vkCmdDraw(...);
```

the GPU does **not** necessarily draw at that moment.

You are recording an instruction into a command buffer.

Conceptually:

```text
CPU recording time:
    vkCmdBindPipeline
    vkCmdBindDescriptorSets
    vkCmdDrawIndexed
    vkCmdPipelineBarrier
    ...

                ↓

VkCommandBuffer

                ↓ later

vkQueueSubmit

                ↓

GPU execution
```

This separation is one of Vulkan's fundamental architectural properties.

---

**Command pools**

A `VkCommandPool` owns allocation backing for command buffers.

It is created for one:

$$
\boxed{\text{queue family}}
$$

not one particular queue handle.

Kairo:

```cpp
VkCommandPoolCreateInfo pool{};
pool.sType =
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

pool.flags =
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

pool.queueFamilyIndex =
    device.GraphicsFamily();
```

So command buffers allocated from this pool are compatible with queues belonging to Kairo's graphics family.

---

**Why the queue-family index belongs to the pool**

Command-buffer implementation may maintain internal state optimized for a particular family of commands.

If a command buffer was allocated from a pool associated with graphics family $G$, it is intended to be submitted to compatible queues from that family.

This makes pool organization naturally follow threading/queue architecture.

---

**Command pools are externally synchronized**

A subtle Vulkan rule:

$$
\boxed{
\text{simultaneous host access to the same command pool must be synchronized}.
}
$$

This matters when multithread-recording.

A good design is often:

```text
thread 0
    own command pool

thread 1
    own command pool

thread 2
    own command pool
```

rather than all CPU workers allocating/resetting from one pool behind a mutex.

---

**Kairo currently has one command pool**

Current `VulkanCommandBuffer` owns:

$$
1\text{ command pool}
$$

and:

$$
1\text{ primary command buffer}.
$$

The source comment explicitly says this is a:

$$
\boxed{\text{single-frame renderer baseline}}
$$

and that the buffer is not safe for concurrent recording.

This is simple and correct for the current architecture.

---

**Command-pool reset strategies**

There are several ways to reuse command storage.

You can reset a particular command buffer if pool supports:

$$
VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT.
$$

Or reset the entire pool:

```cpp
vkResetCommandPool(...);
```

which can be more efficient when all command buffers have the same frame lifetime.

A multi-frame renderer often has:

```text
FrameContext 0 → command pool
FrameContext 1 → command pool
FrameContext 2 → command pool
```

and resets the whole pool only when that frame's fence/timeline proves completion.

---

**Primary command buffers**

Primary buffers can be submitted directly to a queue.

Kairo allocates:

```cpp
allocate.level =
    VK_COMMAND_BUFFER_LEVEL_PRIMARY;
```

Primary buffer can contain:

- render pass begin/end,
- pipeline bindings,
- barriers,
- draws,
- copies,
- dispatches,
- secondary command-buffer execution.

This is what Kairo records every frame.

---

**Secondary command buffers**

Secondary command buffers cannot ordinarily be independently submitted like primaries.

They are executed from primary buffers:

```cpp
vkCmdExecuteCommands(
    primary,
    count,
    secondaryBuffers);
```

They are particularly useful for multithreaded command generation.

For example:

```text
CPU thread 0 → secondary for opaque batch A
CPU thread 1 → secondary for opaque batch B
CPU thread 2 → secondary for opaque batch C

main thread:
    primary
        begin render pass
        execute A/B/C
        end render pass
```

This lets several CPU threads record rendering work in parallel.

---

**Secondary rendering inheritance**

If a secondary command buffer executes inside a render pass, Vulkan needs enough inherited render-state context to validate/execute it.

Traditional render-pass secondary command recording uses:

```cpp
VkCommandBufferInheritanceInfo
```

and flags such as:

```cpp
VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
```

to describe the render pass/subpass context.

This is one of the complexities secondary command buffers introduce.

---

**Secondary buffers are not automatically faster**

They can reduce CPU command-recording bottlenecks.

But they also add:

- more command buffers,
- execution indirection,
- pool management,
- synchronization between CPU workers.

If command recording costs only:

$$
0.2\text{ ms},
$$

parallelizing it may be pointless.

Use them when CPU profiling establishes a recording bottleneck.

---

**Command-buffer lifecycle**

A simplified state model is:

```text
Initial
   ↓ vkBeginCommandBuffer
Recording
   ↓ vkEndCommandBuffer
Executable
   ↓ vkQueueSubmit
Pending
   ↓ GPU completes
Executable again
   ↓ reset
Initial
```

The key dangerous state is:

$$
\boxed{Pending}.
$$

A pending command buffer is still potentially being consumed by the GPU.

You cannot simply reset/re-record it.

---

**Kairo's `Begin()`**

Current Kairo performs:

```cpp
vkResetCommandBuffer(
    m_Handle,
    0);
```

then:

```cpp
vkBeginCommandBuffer(
    m_Handle,
    &begin);
```

but its comment explicitly states the precondition:

$$
\boxed{
\text{GPU work previously submitted with this buffer has completed}.
}
$$

That precondition is guaranteed because `DrawFrame()` waits on the one in-flight fence before re-recording.

Correct.

---

**Why resetting a pending command buffer is disastrous**

Suppose GPU still reads command data:

```text
draw mesh A
draw mesh B
```

while CPU resets and overwrites buffer with:

```text
draw mesh C
```

You have violated lifetime/execution ownership.

The API may report validation errors or behavior may be undefined.

The fence is therefore protecting not just “the image.”

It protects reusable CPU-side Vulkan state too.

---

**Beginning and ending rendering**

Traditional Vulkan:

```cpp
vkCmdBeginRenderPass(...);
```

then draw commands.

Finally:

```cpp
vkCmdEndRenderPass(...);
```

Current Kairo does this for:

- directional shadow pass,
- HDR editor viewport pass,
- swapchain presentation/overlay pass.

Inside a render pass, attachment semantics and subpass structure are active.

---

**Kairo frame recording structure**

Current `VulkanTriangle::Record()` is approximately:

```text
Begin command buffer

if meshes:
    shadow render pass

viewport render pass
    draw meshes
    draw debug lines
end viewport pass

optional object-ID copy
optional HDR color readback

swapchain presentation render pass
    tooling overlay
end presentation pass

End command buffer
```

This is already a multi-pass frame inside one primary command buffer.

---

**Binding pipelines**

Before graphics commands use a graphics pipeline:

```cpp
vkCmdBindPipeline(
    command,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline);
```

Pipeline defines the shader/fixed-state contract we covered in Pass 11.

For compute:

```cpp
VK_PIPELINE_BIND_POINT_COMPUTE.
```

Pipeline binding persists until replaced, within applicable recording semantics.

---

**Pipeline sorting**

If 10,000 objects alternate:

```text
pipeline A
pipeline B
pipeline A
pipeline B
...
```

you repeatedly change GPU state.

Sorting compatible draws by pipeline/material can reduce state changes.

But transparent rendering may impose strict depth ordering.

Therefore command ordering balances:

$$
\boxed{
\text{state locality}
}
$$

against:

$$
\boxed{
\text{rendering correctness}.
}
$$

---

**Binding vertex buffers**

Kairo uses:

```cpp
vkCmdBindVertexBuffers(...)
```

with one or two bindings.

Static mesh:

$$
binding_0=\text{MeshVertex}.
$$

Skinned mesh additionally has:

$$
binding_1=\text{SkinVertexInfluence}.
$$

Each binding has its own byte offset.

---

**Binding an index buffer**

Kairo:

```cpp
vkCmdBindIndexBuffer(
    command,
    mesh.Indices->Handle(),
    0,
    VK_INDEX_TYPE_UINT32);
```

The index type tells Vulkan how to interpret index bytes.

Then indexed drawing becomes possible.

---

**Descriptor-set binding**

Current Kairo:

```cpp
vkCmdBindDescriptorSets(
    command,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    layout,
    0,
    1,
    &descriptor,
    0,
    nullptr);
```

This means:

- graphics bind point,
- pipeline layout defines compatibility,
- first set is set 0,
- one descriptor set,
- no dynamic offsets.

Descriptor-set contents connect actual resources to shader bindings.

---

**Push constants**

Kairo also records per-draw transform/object state:

```cpp
vkCmdPushConstants(...)
```

So its per-draw resource state is currently approximately:

```text
descriptor set
    camera + material + textures + skin

push constants
    model/normal matrix + draw flags/object ID
```

This cleanly separates larger persistent resources from tiny high-frequency draw constants.

---

**`vkCmdDraw`**

Non-indexed draw:

```cpp
vkCmdDraw(
    command,
    vertexCount,
    instanceCount,
    firstVertex,
    firstInstance);
```

The vertex shader's logical vertex index derives from these parameters.

Kairo's debug line system uses:

```cpp
vkCmdDraw(
    command,
    debugVertexCount,
    1,
    0,
    0);
```

because debug lines are directly stored as a vertex stream.

---

**`vkCmdDrawIndexed`**

Kairo meshes use:

```cpp
vkCmdDrawIndexed(
    command,
    mesh.IndexCount,
    1,
    0,
    0,
    0);
```

Parameters conceptually mean:

$$
indexCount,
instanceCount,
firstIndex,
vertexOffset,
firstInstance.
$$

Per indexed vertex:

$$
vertexIndex
=
indexBuffer[firstIndex+i]
+
vertexOffset.
$$

This is how submeshes can reference ranges/base vertices without copying data.

---

**Instancing**

If:

$$
instanceCount>1,
$$

one draw launches multiple logical instances.

Vertex shader receives instance identity.

This is important for:

- repeated meshes,
- foliage,
- crowds,
- particles.

Eventually GPU-driven indirect draws extend this further.

---

**Compute dispatch**

Compute pipeline dispatch:

```cpp
vkCmdDispatch(
    command,
    groupCountX,
    groupCountY,
    groupCountZ);
```

Total invocation count depends on shader's local size:

$$
N_x=
groupCountX\times localSizeX
$$

and similarly for $y,z$.

The workgroup ideas from Pass 9 map directly onto Vulkan compute dispatch.

---

**Queue submission**

Recording produces an executable command buffer.

Execution begins when it is submitted:

```cpp
vkQueueSubmit(...)
```

Current Kairo builds:

```cpp
VkSubmitInfo submit{};
```

containing:

- semaphore to wait on,
- stage at which to wait,
- command buffer,
- semaphore to signal.

It also associates a fence with the submission.

This one call links nearly every synchronization concept in M136.

---

**Kairo's submit**

Current flow:

```text
wait:
    imageAvailable semaphore

wait stage:
    COLOR_ATTACHMENT_OUTPUT

execute:
    primary command buffer

signal:
    renderFinished[imageIndex]

CPU completion:
    inFlight fence
```

Then presentation waits on:

$$
renderFinished[imageIndex].
$$

That is a classic acquire→submit→present chain.

---

**Fence**

A fence is primarily a:

$$
\boxed{\text{GPU → CPU completion primitive}.}
$$

Create a fence.

Submit work with fence.

Later CPU does:

```cpp
vkWaitForFences(...)
```

and learns:

> work associated with that fence has completed.

This is different from a semaphore, which primarily coordinates GPU/queue operations.

---

**Why Kairo's first fence starts signaled**

Kairo creates:

```cpp
fence.flags =
    VK_FENCE_CREATE_SIGNALED_BIT;
```

Why?

At the first frame, no previous GPU frame exists.

But frame loop begins by:

```cpp
vkWaitForFences(...)
```

If the fence began unsignaled, first frame would wait forever because nothing had ever been submitted to signal it.

Starting signaled means:

$$
\boxed{
\text{frame 0's reusable resources are initially free}.
}
$$

---

**Fence reuse cycle**

Kairo performs:

```text
wait fence
    ↓
fence is signaled

acquire image

reset fence
    ↓
fence becomes unsignaled

submit graphics with fence

GPU finishes
    ↓
fence becomes signaled
```

Then next frame repeats.

The ordering is crucial.

---

**Never reset a fence before knowing you will submit**

Suppose you reset the fence first.

Then acquire returns:

$$
VK_ERROR_OUT_OF_DATE.
$$

You skip submission.

The fence remains unsignaled forever.

Next frame waits forever.

Kairo correctly acquires first and only resets the fence after successful/suboptimal acquisition.

This is a very important frame-loop edge case.

---

**Binary semaphores**

Binary semaphore has two conceptual states:

$$
unsignaled,
$$

$$
signaled.
$$

One GPU operation signals it.

Another waits.

You do not assign a numerical value.

Typical chain:

```text
presentation engine
   signal imageAvailable
          ↓
graphics submission waits
          ↓
graphics finishes
   signal renderFinished
          ↓
present operation waits
```

Exactly Kairo's model.

---

**Binary semaphore is not a CPU mutex**

You typically do not inspect its “state” on CPU.

It exists to build asynchronous dependency chains.

The virtue is that CPU does not need to stall between:

$$
acquire,
render,
present.
$$

Dependencies execute on the GPU/queue side.

---

**Why Kairo has one image-available semaphore**

Current Kairo has only:

$$
1\text{ frame in flight}.
$$

It waits the previous frame's fence before reacquiring.

Therefore the single `imageAvailable` semaphore has already been consumed before being reused.

This is safe under the current architecture.

With several frames in flight, you normally need per-frame acquire semaphores.

---

**Why Kairo uses render-finished semaphore per swapchain image**

Presentation completion is subtle because the graphics submission fence does not directly mean the presentation engine has finished consuming the present-wait semaphore.

Using one render-finished semaphore associated with each swapchain image is a robust pattern: reacquiring that image gives a natural guarantee that its previous presentation cycle has progressed far enough for reuse.

Kairo creates:

$$
renderFinished[imageIndex].
$$

Good.

---

**Timeline semaphores**

A timeline semaphore contains monotonically increasing 64-bit values.

Instead of:

```text
semaphore A
semaphore B
semaphore C
```

you can have:

$$
timeline=1
$$

for frame/job 1,

$$
timeline=2
$$

for frame/job 2,

$$
timeline=3.
$$

GPU signals values.

GPU or CPU waits for values.

---

**Timeline model**

Suppose:

$$
T
$$

is one timeline semaphore.

Upload submission signals:

$$
T=5.
$$

Graphics waits:

$$
T\ge5.
$$

Later graphics signals:

$$
T=6.
$$

CPU resource-retirement logic can ask whether:

$$
T\ge6.
$$

This creates a much more expressive completion sequence than a large collection of one-shot fences/semaphores.

---

**Timeline semaphores are excellent for resource retirement**

Suppose texture last used in submission value:

$$
137.
$$

Do not destroy immediately.

Store:

```text
resource → retireAfter = 137
```

Later query completed timeline value:

$$
C.
$$

If:

$$
C\ge137,
$$

destruction is safe.

This is vastly better than calling:

```cpp
vkDeviceWaitIdle()
```

every time a resource is destroyed.

---

**Kairo does not currently use timeline semaphores**

Current repository search shows no:

```text
VK_SEMAPHORE_TYPE_TIMELINE
```

or `VkSemaphoreTypeCreateInfo`.

So current Kairo uses:

$$
\boxed{
\text{one fence + binary semaphores}
}
$$

for its one-frame-in-flight baseline.

Timeline retirement is the natural next architectural step.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/20 - GPU performance models and principled redesign|Previous chapter]] · [[Chapter Edition/03 - Game Engine/22 - Vulkan synchronization and multi-frame lifetimes|Next chapter]]
