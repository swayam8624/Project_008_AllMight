# Mastery evidence ledger — 12 November 2026

This ledger records **what kind of evidence exists**, not how many words were written. Use [[Concepts/Mastery evidence levels|Mastery evidence levels]] for the ladder:

0 Indexed → 1 Explained → 2 Derived/traced → 3 Reference implemented → 4 Independently verified → 5 Native-system verified → 6 Production-repository grounded → 7 Reconstructed mastery.

A phase can hold different levels for different claims. For example, Vulkan synchronization can have a native API smoke witness while transient aliasing remains only derived. Do not average those into one misleading score.

## Current phase-level evidence map

| Phase | Subject | Strongest current evidence | Representative executable/reference artifacts | Boundary that must not be overclaimed |
|---|---|---|---|---|
| 1 | Bits, numbers, memory | Derived + executable examples | existing binary/floating/format teaching and tests | hardware electrical behavior and every compiler target are not measured |
| 2 | C++ object/lifetime model | Reference implemented + compiler/sanitizer CI | tools/m018, asset/lifetime labs, module boundary workflow | exact ABI/compiler behavior still varies by toolchain/platform |
| 3 | Modern C++/STL/modules | Reference implemented + consumer/build tests | PMR/ranges/policy/module labs | broad zero-cost claims require workload-specific profiling |
| 4 | CPU/cache/SIMD/concurrency | Reference implemented; some benchmark evidence | phase4_cpu_systems_lab.cpp, scheduler/asset concurrency witnesses | hardware counters, NUMA and architecture-specific SIMD throughput remain device evidence |
| 5 | Algorithms/data structures | Differential/reference implemented | phase5_algorithm_workload_lab.cpp, algorithm_patterns_lab.cpp | production workload choice still needs real distribution/profile data |
| 6 | Vectors/geometric foundations | Derived + tested scalar references | basis/contact and vector reconstruction material | SIMD/backend parity is separate |
| 7 | Matrices/transforms/quaternions | Derived + reference implementation | basis/transform/quaternion code and dual-quaternion witness | all coordinate conventions/importers are not automatically verified |
| 8 | Advanced linear algebra | Derived + numerical implementations | LU/eigen/SVD/decomposition material and witnesses | large sparse solver robustness and production conditioning datasets need more evidence |
| 9 | Calculus/optimization | Derived + numerical implementations | finite-difference/optimizer/integrator teaching | production inverse-problem convergence is workload-specific |
| 10 | Probability/statistics/numerics | Derived + statistical/numerical witnesses | sampling/statistics code, Monte Carlo witness | empirical claims still need enough independent samples/datasets |
| 11 | Cameras/projective geometry | Derived + numerical traces | camera/projection/jacobian witnesses | real calibration datasets and device conventions need current project evidence |
| 12 | Geometry primitives | Reference implemented in teaching corpus | geometry/closest-point/intersection material | full production geometry-library fuzzing remains separate |
| 13 | Collision/GJK/CCD | Multiple independent reference witnesses | disk CCD, swept broadphase, contact/TOI labs | arbitrary rotating 3D convex CCD and production solver parity remain higher evidence |
| 14 | Spatial acceleration/navigation | BVH query parity + SAH/Morton/refit references | spatial_index_lab.cpp, bvh_sah_refit_lab.cpp | full binned SAH/LBVH construction, dynamic rotations, SIMD/GPU traversal need workload/native evidence |
| 15 | ECS/data-oriented runtime | Storage + scheduler/hierarchy references implemented | ecs_reconstruction_lab.cpp, ecs_scheduler_hierarchy_lab.cpp | concurrent structural mutation, work stealing and KairoECS parity need production evidence |
| 16 | Engine architecture/assets/tooling | Reference implemented with publication/concurrency witnesses | asset pipeline, world contract, threaded publication labs | current Kairo production integration remains a separate repository audit |
| 17 | Rigid-body dynamics | Reference implemented + numerical checks | rigid/spring/mass/inertia teaching and labs | production multi-body/3D engine parity remains separate |
| 18 | Contacts/constraints/CCD response | Strong reference/differential evidence including friction/islands | persistent contact, cache, penetration refresh, rigid-contact, constraint_friction_island_lab.cpp | angular Jacobians, full friction cones, large 3D stacks/islands and production parity remain |
| 19 | Rasterization | Coverage/interpolation + clipping/sample/stencil/blend CPU references | raster_reference_lab.cpp, raster_pipeline_state_lab.cpp | all clip planes, exact API sample rules, conservative rasterization and GPU parity remain |
| 20 | PBR/materials | Isotropic + anisotropic/dielectric/furnace mathematical oracles | pbr_reference_lab.cpp, pbr_extended_reference_lab.cpp | rough transmission, multiple scattering, measured-material and material-import parity remain |
| 21 | Ray/path transport | Scene-level geometry + MIS reference verified | monte_carlo_transport_lab.cpp, path_transport_scene_lab.cpp | emissive-geometry NEE, multibounce transport, BVH/image parity and media remain |
| 22 | GPU architecture/parallel algorithms | Derived/reference + compile/native boundary | prefix-scan teaching, native graphics workflow | occupancy/bank/coalescing claims require device counters |
| 23 | Vulkan | Native sync2 + buffer/timeline + image-layout/readback execution | vulkan_sync_smoke.cpp, vulkan_buffer_timeline_smoke.cpp, vulkan_image_layout_smoke.cpp | queue-family transfer, descriptor/shader visibility, transient aliasing, swapchain/present and production parity remain |
| 24 | Metal/render graph | Logical graph + real MSL compile + native compute execution | render_graph_contract_lab.cpp, metal_compile_smoke.metal, metal_runtime_smoke.mm | CI executed MTLDevice/queue/pipeline/buffer/dispatch/completion; heaps, hazard capture, render-pass behavior, multi-queue sync and production parity remain |
| 25 | Gaussian splatting/captured worlds | Projection + conic/tile/order CPU oracles | gaussian_projection_reference_lab.cpp, gaussian_tile_conic_lab.cpp | full SH, GPU radix/binning/rasterization and current Maveb/Vulkax parity remain |
| 26 | Advanced real-time rendering | Temporal/shadow/cluster + TAA/SSR/SSAO/volume/particle CPU references | realtime_rendering_reference_lab.cpp, realtime_effects_reference_lab.cpp | production TAA masks/variance clipping, Hi-Z SSR, GTAO, scattering/froxels, GPU particle systems and renderer parity remain |
| 27 | Meshes/animation | Topology + QEM + rigid-blend reference implemented | mesh_topology_lab.cpp, qem_edge_collapse_lab.cpp, dual_quaternion_skinning_lab.cpp, animation controller lab | optimal QEM/link-condition remeshing and production GPU skinning/import parity remain |
| 28 | Emulation/OS/network/crypto | Real POSIX/network execution + primitive and native AES-GCM known-answer evidence | emulator lab, posix_vm_file_lab.cpp, network_loopback_lab.cpp, crypto_primitives_lab.cpp, openssl_aes_gcm_smoke.cpp | internet faults, crash durability, nonce/key lifecycle, replay policy, side channels and production security remain |
| 29 | ML runtime/tensors/compressed inference | Tensor/attention + GGUF metadata/tensor subset references | phase29_ml_runtime_lab.cpp, gguf_metadata_tensor_lab.cpp | full GGUF type catalog/payload, protobuf ONNX/opsets, tokenizer, SIMD/Metal kernels and model parity remain |
| 30 | Production/DCC/research/capstone | Host-neutral pipeline + real Blender save/export/re-import host execution | production_pipeline_contract_lab.cpp, tools/dcc/blender_contract.py | Maya/Houdini/Nuke host runs, arbitrary-scene/material/armature parity, render farm/filesystem durability and learner oral mastery remain |

## What this repair changed

The repair specifically closes the former **representation gap** where a topic could be called developed because a heading or short concept card existed. The repository now has:

- no Concepts/*.md file below the structural 1200-byte smoke floor;
- explicit evidence levels rather than one ambiguous developed state;
- an expanded family of independently compiled/run C++ reference laboratories for the previously weakest subjects;
- native Vulkan synchronization2/resource/image execution CI and Metal shader compilation plus command-buffer compute execution CI;
- reconstruction checkpoints inside all three canonical master volumes;
- known-answer cryptographic tests rather than prose-only primitive names;
- real POSIX loopback and mmap/descriptor execution;
- Phase 29 tensor/GGUF/ONNX/quantized-attention runtime ownership instead of an effectively blank phase;
- Phase 30 host-neutral publication/DCC boundaries instead of tool-name mentions;
- independent raster, PBR, Monte Carlo, Gaussian, temporal-rendering, ECS, spatial and mesh/skinning oracles.

## Why remaining boundaries are recorded rather than hidden

The goal is maximum depth, not maximum confidence language. Some proof layers require hardware, external applications, current production repositories or human reconstruction:

- a software Vulkan ICD now proves API execution/validation for synchronization, buffer visibility/timeline, and image-layout/readback contracts, but not discrete-vendor physical-GPU performance;
- the current macOS CI host executed a real Metal compute pipeline, but heap aliasing, explicit hazard-tracking captures, render-pass behavior, multi-queue synchronization and production parity remain;
- a CPU Gaussian oracle can expose projection bugs but does not certify the current Maveb Metal kernel;
- an educational SHA/HMAC/HKDF implementation can prove bit-level understanding and known-answer agreement but should not become production cryptography;
- Blender now has a real headless host round trip; Maya/Houdini/Nuke remain host-neutral until equivalent licensed/current hosts execute their adapters;
- written material cannot prove Level 7 reconstructed mastery until a learner derives/implements/debugs it without copying.

Those are not excuses or hidden gaps. They are the next evidence layer attached to the exact claim that needs it.

## Regression gates

tools/check_depth_repair_contract.py checks that:

1. every required repair artifact still exists;
2. no concept card silently collapses below the 1200-byte structural floor;
3. exactly the three canonical continuous volumes remain;
4. each canonical volume retains its reconstruction checkpoints.

The generic C++20 workflow separately compiles and executes every tools/labs/*_lab.cpp under GCC sanitizer, Clang strict and GCC release modes. The native graphics workflow exercises Vulkan and Metal-specific toolchains. The consolidation/source-sync workflows protect the three-volume teaching surface.

These checks are necessary evidence, not a semantic completion certificate.


## Second-wave closure — executable boundaries tightened

The follow-up repair converted four previously explicit boundaries into tested reference artifacts:

- `ecs_scheduler_hierarchy_lab.cpp` adds read/write conflict scheduling and subtree transform invalidation to the ECS layer;
- `path_transport_scene_lab.cpp` connects camera geometry, Lambertian transport, two proposal PDFs and balance MIS to an analytic pixel target;
- `gaussian_tile_conic_lab.cpp` continues projected covariance through conic inversion, conservative tile duplication, depth ordering and transmittance;
- `qem_edge_collapse_lab.cpp` moves mesh simplification from topology-only discussion to plane quadrics, candidate error and geometric collapse rejection.

These raise the corresponding claims from prose/partial-reference status. They do not erase the higher evidence boundaries stated in the phase table.


## Third-wave closure — pipeline state, spatial quality, solver coupling, and model containers

The next repair wave closes another set of previously explicit reference gaps without promoting them beyond the evidence actually executed:

- `raster_pipeline_state_lab.cpp` extends raster semantics through homogeneous clipping, explicit multisample coverage, stencil state, and premultiplied blending;
- `pbr_extended_reference_lab.cpp` adds anisotropic GGX normalization, exact dielectric Fresnel, Snell refraction/total internal reflection, and a white-furnace energy integral;
- `bvh_sah_refit_lab.cpp` adds surface-area split reasoning, Morton interleaving, and parent-bound refit to the earlier BVH query oracle;
- `constraint_friction_island_lab.cpp` adds island connectivity, unilateral impulses, Coulomb friction clamping, and a tiny sequential-impulse stack;
- `gguf_metadata_tensor_lab.cpp` advances GGUF from prefix-only validation to typed metadata, alignment and tensor descriptors.

These are reference-level closures. Exact hardware/API behavior, production repository parity, third-party DCC execution, and learner Level-7 reconstruction are still tracked separately rather than absorbed into a generic “complete” label.


## Native execution escalation

The native graphics gate now goes beyond syntax-level API examples. Vulkan executes a real transfer fill into an allocated/bound buffer, records a resource-specific synchronization2 buffer barrier from transfer-write to host-read, signals a timeline semaphore, waits for a concrete timeline value, maps the memory and checks the written pattern. Metal compiles the MSL kernel and, when the macOS CI host exposes a Metal device, loads the metallib, creates a compute pipeline/queue/shared buffers, dispatches work, waits for command completion and validates the result. On the current macOS-15 CI run the host exposed a Metal device and the runtime witness passed; the SKIP path remains only as an honest fallback for future hosts without a device.

This still does not elevate every Phase 23/24 claim to native verification. Image layout transitions, queue-family ownership, descriptor/shader visibility, heap aliasing, multi-queue behavior, capture-derived hazard evidence, and current Kairo/Maveb integration remain individually gated.


## DCC execution escalation

Blender is no longer represented only by prose or a host-neutral adapter. The dedicated CI workflow launches Blender in background mode, constructs a deterministic mesh asset and collection through `bpy`, attaches logical publication metadata, saves the source `.blend`, exports GLB, validates the GLB container/JSON contract, resets the host, re-imports the published artifact and checks geometry plus custom identity fields.

This is intentionally a **Blender-only** native-host escalation. Maya, Houdini and Nuke are not marked executed because their current hosts/licenses are not available in this CI environment. Their contracts remain explicit and can be promoted only when equivalent host-side witnesses exist.


## Fourth-wave closure — temporal effects, native image state, security, and host DCC

The latest repair wave closes four more areas that were previously recorded only as future boundaries.

**Advanced real-time effects.** `realtime_effects_reference_lab.cpp` now checks deterministic Halton jitter, current-neighborhood history clipping, a screen-space depth-crossing ray-march oracle, SSAO-style depth/thickness visibility, homogeneous Beer–Lambert transmittance, and deterministic particle update/death/stable compaction. These are deliberately semantic CPU references; production Hi-Z traversal, GTAO, froxel scattering, reactive masks and native GPU particle systems remain higher evidence levels.

**Vulkan images.** `vulkan_image_layout_smoke.cpp` moves native evidence beyond a generic barrier and buffer visibility. It allocates an optimal-tiled RGBA image, records `UNDEFINED → TRANSFER_DST → TRANSFER_SRC` synchronization2 layout transitions, clears it, copies to a host-visible buffer, performs transfer-to-host synchronization and checks the UNORM bytes under validation. Queue-family ownership, descriptor/shader hazards and presentation remain separate.

**Authenticated encryption.** `openssl_aes_gcm_smoke.cpp` checks a standard AES-128-GCM known-answer vector through OpenSSL EVP, authenticated round trip, tag and AAD tamper rejection, nonce-sensitive output and discard-on-authentication-failure behavior. The evidence is for the primitive/API contract, not for nonce persistence, replay defense, key management or a complete production protocol.

**DCC execution.** Blender is now exercised as a real host rather than only named in an adapter contract: background Python creates the asset, saves the source scene, exports GLB with logical metadata, validates the binary glTF container, resets the host, re-imports the publication and checks geometry plus identity fields. Maya, Houdini and Nuke remain explicitly unexecuted rather than being promoted by analogy.
