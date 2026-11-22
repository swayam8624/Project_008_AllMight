# Vulkan synchronization2 and resource transitions

Vulkan synchronization is not “put a barrier between two passes.” A dependency has to connect a real producer access to a real consumer access with execution and memory scopes that cover both operations.

Vulkan 1.3 includes the synchronization2 model promoted from VK_KHR_synchronization2. It places stage and access masks together in the version-2 barrier structures and uses commands such as vkCmdPipelineBarrier2 and vkQueueSubmit2.

Primary references:
- https://docs.vulkan.org/guide/latest/extensions/VK_KHR_synchronization2.html
- https://docs.vulkan.org/spec/latest/chapters/synchronization.html

## Producer → consumer reasoning

For every hazard, write the transition in words first.

Example:

A transfer command writes a storage buffer. A later compute shader reads that buffer.

The producer execution stage is transfer. The producer access is transfer write. The consumer execution stage is compute shader. The consumer access is shader storage read.

An abstract memory barrier therefore needs to cover:

source stage = TRANSFER  
source access = TRANSFER_WRITE  
destination stage = COMPUTE_SHADER  
destination access = SHADER_STORAGE_READ.

Using ALL_COMMANDS and MEMORY_READ/WRITE everywhere may accidentally work but destroys precision and can hide misunderstanding. Using a destination stage that never performs the consumer access is not made correct by a broad access mask.

## Image transitions add a layout contract

Images have another state dimension: layout. An image memory barrier can both synchronize accesses and transition from oldLayout to newLayout. The old layout must describe the state established by prior use unless the contents are intentionally discarded under the rules that permit that.

A render graph therefore needs more than “read” and “write.” For each image subresource it eventually needs:

- stage/access usage;
- required layout;
- aspect;
- mip range;
- array-layer range;
- queue ownership when queues/families differ.

A whole-image state tracker is a valid early implementation but cannot claim correct independent mip/layer scheduling.

## Queue submission and timeline semaphores

Synchronization2 also modernizes queue submission through VkSubmitInfo2 and related structures. Timeline semaphores carry monotonically increasing integer values and are useful for expressing ordered completion points across submissions. A timeline value is not a memory barrier by itself; the signal/wait operations participate in dependencies whose scopes must still match the work.

## Native witness now in the repository

tools/native/vulkan_sync_smoke.cpp is compiled and executed in the dedicated native graphics workflow on Ubuntu with a Vulkan implementation and validation layer packages. It:

- creates a Vulkan 1.3 instance;
- finds a device exposing synchronization2;
- creates a queue/device with the feature enabled;
- records a VkMemoryBarrier2 through vkCmdPipelineBarrier2;
- submits the command buffer using vkQueueSubmit2;
- waits for fence completion;
- fails on validation-layer errors when the layer is available.

That materially raises the evidence level from a CPU-only logical graph.

It still does not prove a resource-specific RAW transition, image layout transition, queue-family transfer, timeline protocol, transient aliasing, physical-GPU performance, or current Kairo/Vulkax integration. Each of those needs its own captured witness.

Related: [[GPU workgroups and synchronization]], [[Rendering history validity]], [[Render graph barrier lowering]].
