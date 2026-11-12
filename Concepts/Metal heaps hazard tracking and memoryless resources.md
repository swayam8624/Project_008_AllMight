# Metal heaps, hazard tracking and memoryless resources

Metal resource synchronization depends on the Metal API generation and resource hazard mode. Tracked resources in applicable Metal APIs can receive automatic hazard tracking for supported dependencies, while untracked resources place more responsibility on the application. Metal also exposes explicit barriers, fences and events. Metal 4 changes the model further: Apple documents Metal 4 resources as untracked, requiring explicit barriers where concurrent read/write hazards demand them. citeturn438229search2turn438229search7turn438229search8

Heaps let multiple Metal resources share explicitly managed memory and enable transient aliasing strategies, but reuse must respect logical lifetimes and native synchronization. A render graph should infer aliasing only after it has proven non-overlapping lifetimes and the backend has established the required visibility/order. citeturn438229search4turn438229search5

Memoryless textures are a separate storage/lifetime optimization for temporary render targets whose contents do not need persistent backing outside their intended render usage. They should be inferred from the render graph's lifetime/use pattern rather than selected merely because a texture is “temporary.” citeturn438229search0turn438229search11

**Executable reference:** `tools/labs/gpu_transition_plan_lab.cpp` provides backend-neutral hazard/layout/queue planning.

**Native verification still required:** Metal version-specific lowering, tracked/untracked policy, heap aliasing tests, memoryless attachment validation, GPU captures and current Kairo/Maveb integration.
