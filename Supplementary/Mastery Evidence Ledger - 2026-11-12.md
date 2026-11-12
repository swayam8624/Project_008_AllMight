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
| 14 | Spatial acceleration/navigation | Differential reference implemented | spatial_index_lab.cpp | SAH/LBVH quality, dynamic updates, SIMD/GPU traversal need workload/native evidence |
| 15 | ECS/data-oriented runtime | Reference implemented | ecs_reconstruction_lab.cpp | concurrent structural mutation, full scheduler and KairoECS parity need production evidence |
| 16 | Engine architecture/assets/tooling | Reference implemented with publication/concurrency witnesses | asset pipeline, world contract, threaded publication labs | current Kairo production integration remains a separate repository audit |
| 17 | Rigid-body dynamics | Reference implemented + numerical checks | rigid/spring/mass/inertia teaching and labs | production multi-body/3D engine parity remains separate |
| 18 | Contacts/constraints/CCD response | Strong reference/differential evidence | persistent contact, cache, penetration refresh, rigid-contact labs | complete 3D stacks/friction/islands and production parity are not implied |
| 19 | Rasterization | Independent CPU reference implemented | raster_reference_lab.cpp | clipping/MSAA/stencil/conservative raster and GPU parity remain additional layers |
| 20 | PBR/materials | Mathematical oracle implemented | pbr_reference_lab.cpp | full white-furnace suite, transmission, anisotropy and material-import parity remain |
| 21 | Ray/path transport | Estimator invariants independently verified | monte_carlo_transport_lab.cpp plus retained ray/path teaching | complete scene path tracer, NEE/MIS image convergence and media parity remain |
| 22 | GPU architecture/parallel algorithms | Derived/reference + compile/native boundary | prefix-scan teaching, native graphics workflow | occupancy/bank/coalescing claims require device counters |
| 23 | Vulkan | Native API execution smoke | tools/native/vulkan_sync_smoke.cpp | resource-specific layouts/queue ownership/timeline protocols and production renderer parity remain |
| 24 | Metal/render graph | Logical graph reference + real MSL compile | render_graph_contract_lab.cpp, metal_compile_smoke.metal | Metal command-buffer execution/capture, heap aliasing and physical Apple-GPU behavior remain |
| 25 | Gaussian splatting/captured worlds | Independent CPU mathematical oracle | gaussian_projection_reference_lab.cpp | conic/tile/sort/full SH/native GPU and current Maveb/Vulkax parity remain |
| 26 | Advanced real-time rendering | CPU semantic reference implemented | realtime_rendering_reference_lab.cpp | full TAA/SSR/SSAO/volumetrics/particles and GPU renderer parity remain |
| 27 | Meshes/animation | Topology + rigid-blend reference implemented | mesh_topology_lab.cpp, dual_quaternion_skinning_lab.cpp, animation controller lab | complete QEM/remeshing and production GPU skinning/import parity remain |
| 28 | Emulation/OS/network/crypto | Real POSIX/network execution + known-answer crypto | emulator lab, posix_vm_file_lab.cpp, network_loopback_lab.cpp, crypto_primitives_lab.cpp | internet faults, crash durability, side channels/key management and production security remain |
| 29 | ML runtime/tensors/compressed inference | New reference implementation | phase29_ml_runtime_lab.cpp | full GGUF, protobuf ONNX/opsets, tokenizer, SIMD/Metal kernels and model parity remain |
| 30 | Production/DCC/research/capstone | Host-neutral publication reference + research methodology | production_pipeline_contract_lab.cpp | actual Blender/Maya/Houdini/Nuke host runs, render farm/filesystem durability and learner oral mastery remain |

## What this repair changed

The repair specifically closes the former **representation gap** where a topic could be called developed because a heading or short concept card existed. The repository now has:

- no Concepts/*.md file below the structural 1000-byte smoke floor;
- explicit evidence levels rather than one ambiguous developed state;
- fifteen new C++ reference laboratories for the previously weakest subjects;
- native Vulkan synchronization2 execution CI and Metal shader compilation CI;
- reconstruction checkpoints inside all three canonical master volumes;
- known-answer cryptographic tests rather than prose-only primitive names;
- real POSIX loopback and mmap/descriptor execution;
- Phase 29 tensor/GGUF/ONNX/quantized-attention runtime ownership instead of an effectively blank phase;
- Phase 30 host-neutral publication/DCC boundaries instead of tool-name mentions;
- independent raster, PBR, Monte Carlo, Gaussian, temporal-rendering, ECS, spatial and mesh/skinning oracles.

## Why remaining boundaries are recorded rather than hidden

The goal is maximum depth, not maximum confidence language. Some proof layers require hardware, external applications, current production repositories or human reconstruction:

- a software Vulkan ICD can prove API execution/validation but not NVIDIA/AMD/Apple physical-GPU performance;
- MSL compilation can prove shader-language/toolchain validity but not Metal runtime hazard behavior without Apple-GPU execution;
- a CPU Gaussian oracle can expose projection bugs but does not certify the current Maveb Metal kernel;
- an educational SHA/HMAC/HKDF implementation can prove bit-level understanding and known-answer agreement but should not become production cryptography;
- a host-neutral DCC contract cannot prove Blender/Maya/Houdini/Nuke adapters until those hosts execute it;
- written material cannot prove Level 7 reconstructed mastery until a learner derives/implements/debugs it without copying.

Those are not excuses or hidden gaps. They are the next evidence layer attached to the exact claim that needs it.

## Regression gates

tools/check_depth_repair_contract.py checks that:

1. every required repair artifact still exists;
2. no concept card silently collapses below the 1000-byte structural floor;
3. exactly the three canonical continuous volumes remain;
4. each canonical volume retains its reconstruction checkpoints.

The generic C++20 workflow separately compiles and executes every tools/labs/*_lab.cpp under GCC sanitizer, Clang strict and GCC release modes. The native graphics workflow exercises Vulkan and Metal-specific toolchains. The consolidation/source-sync workflows protect the three-volume teaching surface.

These checks are necessary evidence, not a semantic completion certificate.
