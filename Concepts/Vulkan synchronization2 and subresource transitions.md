# Vulkan synchronization2 and subresource transitions

A logical dependency such as “lighting reads the G-buffer after G-buffer rendering writes it” does not by itself define a Vulkan barrier. Native Vulkan synchronization needs execution scopes, memory access scopes, image layout state, optional queue-family ownership, and the exact subresource range.

With synchronization2, image transitions are represented through structures such as `VkImageMemoryBarrier2` inside `VkDependencyInfo`. The barrier identifies source/destination stage and access scopes, old/new layouts, queue-family indices, the image and its subresource range. Layout transitions are memory dependencies; they must be ordered after accesses using the old layout and before accesses using the new layout. citeturn438229search1turn438229search3

Track state per mip/layer/aspect when the renderer relies on independent subresource work. Whole-image tracking is simpler but can introduce false dependencies. Queue ownership is independent of the access hazard: two logical queues in the same family may need synchronization without an ownership transfer, while different families may require release/acquire ownership handling.

**Executable reference:** `tools/labs/gpu_transition_plan_lab.cpp` models the backend-neutral state machine. It does not emit Vulkan commands.

**Native verification still required:** real `VkDependencyInfo` lowering, validation-layer output, queue/timeline integration, swapchain lifecycle, GPU timestamps and pinned production-repository integration.
