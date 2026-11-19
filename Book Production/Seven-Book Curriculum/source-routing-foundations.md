# Source-to-seven-book routing: Computing Foundations (39 chapters)

**Status:** initial structural routing, 18 November 2026. This is a chapter-by-chapter placement proposal based on the current **chapter reading map** and a close inspection of the opening chapter, NOT a claim that all 39 source chapter bodies, figures, citations, or executable examples have been independently reviewed. Their full content remains authoritative in the continuous master. Before cutting/rebuilding manuscripts, inspect each chapter body and record unique subsections and source IDs. No source deletion is authorized by this map.

**Book abbreviations:** B1 Bits, Programs and Reliable Systems; B2 Mathematics for Visual Computing; B3 Geometry, Cameras and Image Formation; B4 Light Transport and Image Synthesis; B5 Spatial Worlds, Engine State and Simulation; B6 GPU Architecture and Real-Time Rendering; B7 Vision, XR and Playable Worlds.

| Existing chapter | Primary home | Project-driven first-use / integration decision |
| --- | --- | --- |
| 01 From Bits to Running Systems | B1 | **NES-01**, iNES header interpretation, typed errors, tests, bytes and flags. Started and compiled. |
| 02 Values at Addresses: Layout, Lifetimes, and Representation | B1 | **NES-02 authored; pending canonical insertion**. Owned NROM cartridge, RAM mirroring, PRG mapping, typed missing-device errors, failed-load publication, moved-from safety and strict C++23 tests. Existing original text retained. |
| 03 Records as Resource Transactions | B1 | **Pass 03 authored; canonical insertion pending.** NES-02T: staging an owned cartridge, strong exception guarantee, no-throw commit, validated publication, revision identity, simulated allocation failure, typed error translation, rollback witnesses. Original ~1,892-line chapter-edition material preserved. |
| 04 Prove C++ Ownership with Runnable Experiments | B1 | **Pass 04 authored; canonical insertion pending.** NES-02P shares the actual iNES, bus and cartridge runtime implementations; deterministic unwind/move/optional-emplace/owned-byte/failure-atomicity/weak-observer tests; preserves the original ~1,603-line derived reading copy. |
| 05 Generic Asset Pipelines without Hidden Contracts | B1 | **Pass 05 authored; canonical insertion deferred.** A single constrained C++23 importer/variant store reuses the actual NROM cartridge and adds independently validated RGBA8 and quantized triangle-mesh domains. Explicit resource limits, failure classes, capacity, ownership and 256-slot reallocation evidence; preserve the existing ~1,516-line reading copy. |
| 06 Grow Registries without Invalidating Identity | B1 | **Pass 06 authored; canonical insertion deferred.** Existing M015 registry explanations preserved; NES-06G upgrades the real typed NROM/texture/mesh pipeline with owner namespaces, generational handles, tombstones at UINT64_MAX, safe slot reuse, strict errors, moved-registry identity and 2,000-cycle oracle tests. |
| 07 Typed Interfaces, Identity, and Data Structures | B1 | **Pass 07 authored; canonical insertion deferred.** Actual NROM bus executes a rigorously bounded 6502 subset; independently compiled C++23 PIMPL debugger exposes typed snapshots, with fixed ring traces, ordered breakpoints, Bloom visitation prefilter, prioritized bus probes, and DSU RAM alias analysis. Original ~877-line chapter edition retained; benchmark results still need measurement. |
| 08 Graphs, Search, and GPU Scheduling | B1 (algorithms), B6 (GPU) | **Pass 08 authored; canonical insertion deferred.** Reuses NROM/typed registry/C++23 CPU debugger to execute actual host task DAG while separately planning—not submitting—GPU transfer/compute/graphics. Automatic RAW/WAR/WAW hazards, explicit dependencies, stable Kahn order, typed cycles/errors, checked critical path, CPU scan oracle; backend-specific synchronization remains in B6. |
| 09 GPU Memory, Divergence, Atomics, and Ordering | B6 (device semantics), B1 (C++23 host reference) | **Pass 09 authored; canonical insertion deferred.** Range-aware CPU oracle for initialized/visible bytes, alignments, conceptual ownership/layout, independent execution-only and memory dependencies, completion-gated retirement; actual C++ host release/acquire experiment and synthetic lane/barrier tests. No native GPU execution claimed. |
| 10 Reproducible Builds, Profiling, and Repository Contracts | B1 | **Pass 10 authored; canonical insertion deferred.** Cumulative ten-lab C++23 build target graph with explicit public INTERFACE usage requirements, isolated Ninja Debug/Release/Clang-san presets, deterministic NROM/CPU/graph/scan correctness-versus-benchmark protocol, SHA-256/JUnit/CTest fail-closed evidence receipts and GCC/Clang/macOS/ASan+UBSan CI definitions. No passing CI results asserted without observed runs. |
| 11 C++ Const, Borrowing, and CPU–GPU Buffering | B1 (borrowing), B6 (buffers) | **Pass 11 authored; canonical insertion deferred.** Actual NROM CPU/typed texture yields a byte-stable debugger overlay, published through move-only frame writers and pinned immutable read leases. Three independent transfer/compute/graphics stamps plus reader-count and owner/slot/generation checks govern safe simulated retirement; real Vulkan/Metal device completion remains separate and unverified. |
| 12 Commands, Async Work, and Resource Lifetimes | B1 (concurrency), B6 (submission) | **Pass 12 authored; canonical insertion deferred.** Real Pass08 DAG + Pass11 frame ring wrapped by C++23 command state machine, definite pre-submit rejection/retry, indeterminate-submission quarantine, owned completion notices, device-loss refusal, bounded journal pruning and actual NES/typed-texture overlay. No native Vulkan/Metal execution claimed. |
| 13 Open Vulkan Devices and Describe Resources | B6 (native Vulkan), B1 (typed C++23 contract) | **Pass 13 authored, canonical insertion deferred.** GPU-13D deliberately uses KairoRenderer device/buffer/texture source, Maveb Metal frame/Gaussian source and KairoPhysics broad-phase plus XR/one NES overlay. C++23 queue/memory capability contract and optional native headless Vulkan VkDevice/VkBuffer allocation probe. Synthetic CPU tests do not assert real GPU execution. |
| 14 Submit, Synchronize, and Retire GPU Work | B6 (Kairo Vulkan, Maveb Metal), B1 (C++23 lifetime contracts) | **Pass14 authored, canonical insertion deferred.** GPU-14F covers real Kairo command/fence reset failure path, image vs frame vs submission identity, multi-frame retirement, WSI presentation lifetime, Maveb frame arenas and completion handlers, deferred Gaussian/collision/XR resource reuse; 14th portable C++23 test plus optional real Vulkan buffer fill/submit/fence/readback. Native execution unverified. |
| 15 Execute and Measure a Metal Frame | B6 (Maveb native Metal), B1 (C++23 frame ownership), B5 (physics oracle), B7 (XR latency) | **Pass15 authored; canonical insertion deferred.** Actual Maveb Metal FrameContext/Renderer and Kairo Objective-C++ backend inspected. Portable three-permit/staging/capture/timing contracts plus macOS-only native original MSL AABB overlap and offscreen triangle/readback with independent CPU oracles; no unverified native GPU result asserted. |
| 16 Profile GPUs and Respect Apple Ownership Boundaries | B6 (Maveb Metal, Kairo ObjC++), B1 (C++23 callback lifetime), B5 (Kyoto oracle), B7 (XR metric provenance) | **Pass16 authored, canonical insertion deferred.** Actual renderer/callback code inspected; 16th portable C++23 concurrent completion/owner-safe mailbox and fail-closed GPU profile/counter evidence; optional real Metal 64-word kernel and post-renderer completion block. Native execution/captures/counters not claimed. |
| 17 Python and Numerical Tools for Graphics Systems | B2 | C++23 numerical and data pipeline first; Python-as-external-reference, binding cost, parity tests. |
| 18 Vectors for Light and Frames | B2 | KairoMath vector types; connect reflection/refraction to renderer B4. |
| 19 Coordinate Frames and Linear Systems | B2 | Named frames, transformations, independent linear-system oracles. |
| 20 Choose Matrix Factorizations from Structure | B2 | KairoMath matrix solvers, conditioning and residual tests. |
| 21 Multivariable Derivatives and Sensitivity | B2 | Dual/finite-difference and analytic gradient witnesses. |
| 22 Accumulation, Quadrature, and the Bridge to Probability | B2 | Integration and error bounds; later B4 Monte Carlo renderer. |
| 23 Probability Models and Measurement Error | B2 | Typed distributions and uncertainty laboratory; later Vulkax experimental studies. |
| 24 Waiting Times, Covariance, and Matrix Decomposition | B2 | Sampling and covariance numerical contract tests. |
| 25 Continuous Models and Numerical Error | B2 | ODE/PDE state, integration stability and convergence tests; bridge to B5. |
| 26 Matrix Functions and Nonlinear Least Squares | B2 | Matrix exponential and fit objective, KairoMath. |
| 27 Gauss–Newton for Geometry and Pose | B2 | Nonlinear least squares with robust residuals; then B3/B7 pose estimation. |
| 28 Reliable Optimization and Root Finding | B2 | Independent root-finding and optimizer tests with deliberate failures. |
| 29 Discretization, Convergence, and Experimental Design | B2 | Convergence studies in numerical library; B7 research protocol bridge. |
| 30 Monte Carlo Estimates and Uncertainty Metrics | B2 | C++ PRNG and confidence intervals; B4 light transport integration. |
| 31 Implement Probability with Numerical Contracts | B2 | Reproducible RNG, covariance, overflow and tail checks. |
| 32 Geometry Becomes an Image and an Inverse Problem | B3 | Maveb camera/reconstruction pipeline with grounded forward/inverse comparison. |
| 33 Projection, Clipping, and Raster Sampling | B3 (geometry), B6 (raster) | Camera transforms plus CPU reference rasterizer; bridge to GPU raster. |
| 34 Robust Predicates and Camera Sensitivity | B3 | KairoGeometry predicates and finite-difference sensitivity oracle. |
| 35 Camera Extrinsics and Projection Geometry | B3 | Multi-camera coordinate frame and ray tests. |
| 36 Exposure, Physical Optics, and Depth of Field | B3 (optics), B4 (rendering) | Thin-lens camera, sampling and photographic exposure validation. |
| 37 Camera Contracts and Transform-Chain Debugging | B3 | Reproduce/probe frame-chain failures against Maveb. |
| 38 Reconstruct the Complete Geometry Pipeline | B3 | Camera → projection → clipping → rasterization trace; independent numerical witness. |
| 39 Research Navigation and Source Boundaries | B7 (methodology), shared | Source-unit ledger, claims, reproducible experiments and citation precision applied across all books. |

## Cross-chapter invariant

A learner should never have to read a GPU submission chapter to understand a C++ pointer's lifetime, nor a camera-calibration appendix to understand the first vector operation. Conversely, the full treatment of GPU and optical issues should not be truncated to protect an arbitrary book boundary. The source may be **reordered by first use**, but every relocation needs a preservation trace, a narrative bridge and an independently checked destination.

## First concrete completion check (NES-01)

The tested project lab demonstrates a strict legacy iNES layout contract, typed `std::expected` failures, trainer/data bounds checks, mapper flag extraction, CHR-RAM representation and CPU RAM mirroring. The actual KairoNES cartridge implementation has not been patched. A passing synthetic unit does not imply commercial-ROM compatibility, CPU instruction correctness, valid PPU frames, or audio fidelity.

**Next editorial source unit:** source chapter 02, with cross-check against the latest NES repository's bus, cartridge object, mapper ownership and executable trace. Include object layout, pointer validity, `std::span` borrowing boundaries, bus mirroring and negative tests; retain original explanations unless correction is necessary.

## Pass 02 verification boundary (18 November 2026)

The Book 1 Chapter 02 additional lesson is source-located after the original C++ identity/lifetime route and before "Choose a type for its promise, not its familiar size." No original text is removed. The additive prose and the two self-contained C++23 labs are committed to `curriculum/cpp23-nes-project-pass01`; applying either section inside the **canonical long-form manuscript remains a local, explicit, idempotent operation**. See `tools/apply_cpp23_project_inserts.py`.

The new NES-02 code passed GCC 14 and Clang 17 C++23 strict-warning builds, a local CMake/CTest run, plus an ASan/UBSan build with deterministic RAM/PRG mapping sweeps. No native macOS, current KairoNES integration, full ROM/mapper tests or LaTeX builds have been performed. The next chronological chapter is 03, "Records as Resource Transactions," where ownership, construction, failure guarantees and cartridge publication become the next real system milestone.


## Pass 03 — Chapter 03 source-preservation and acceptance note

Existing chapter: "Records as Resource Transactions" (~1,892 lines in the derived Chapter Edition). Its original conceptual sequence includes aggregate and designated initialization, layout/alignment/padding, object and reference lifetime, move permission versus transfer, borrowing, constructors/destructors, copying and move ownership, and exception guarantees. The pass **does not overwrite or regenerate any of those topics**. The new story attaches a real subsystem to the failure-guarantees discussion at the pre-existing exact heading `## Let the compiler compose the operations`.

- Source addition: `Supplementary/C++23 Project Labs/NES-02T - Transactional Cartridge Replacement.md`.
- Shared engineering core: `tools/labs/nes_course/ines_layout.hpp` and `tools/labs/nes_course/nes_bus_nrom.hpp`.
- Executable laboratory: `tools/labs/nes_cartridge_transaction_cpp23_lab.cpp`.
- Preservation path: `tools/apply_cpp23_project_inserts.py` — idempotent NES-01, NES-02 and NES-02T exact sections; refuses an altered prior insertion.
- Tests: synthetic valid and rejected images, explicit pre/post-staging failure, injected `std::bad_alloc`, old-state witness preservation, CHR ROM/RAM behavior, revision updates, CPU RAM policy, repeated failures. **The newly committed NES-02T executable still requires an actual compiler/CTest run on the target Mac; do not mark it green from review alone.**
- Scope: a single-threaded, CPU-owned NROM transaction. Not an authenticated file parser, general mapper implementation, CPU execution engine, GPU resource retirement system or concurrency-safe publishing protocol.

The next original chapter, 04 ("Prove C++ Ownership with Runnable Experiments"), should inspect the real C++ witness suite, then test move/copy/exception/noexcept behavior with reproducible compiler configurations and sanitizers. The broader NES-03 CPU execution stage follows once its object lifetime and bus correctness dependencies are satisfied.


## Pass 04 — Independent ownership evidence against the same emulator

The existing "Prove C++ Ownership with Runnable Experiments" chapter already has 1,603 lines of derivations and runnable examples. It includes values and references, borrowed views, safe asset pipelines, constructor/destructor rules, smart pointers, partial construction, native resource lifetimes, callbacks, C++20/23 language boundaries and explicit limitations. This pass **does not rewrite these explanations**. It adds an independent **evidence-oriented C++23 experiment** at the first-use boundary immediately before `## From one owned resource to many requests and a replaceable backend`.

**New additive prose:** `Supplementary/C++23 Project Labs/NES-02P - Ownership Proofs for an Emulator.md` (~18,124 characters).
**New executable:** `tools/labs/nes_ownership_proofs_cpp23_lab.cpp` (five independent proof groups).
**Shared previous core:** `tools/labs/nes_course/ines_layout.hpp`, `nes_bus_nrom.hpp`, `cartridge_runtime.hpp`. The transaction runtime was extracted unchanged from the NES-02T lab so that the two binaries execute the **same implementation**.
**Tests:** exact constructor/unwinding event sequences; `std::move` cast vs move constructor; one release duty across owner transfers; `optional::emplace` throwing-constructor counterexample; deep-copy independence of a file buffer; prepared-but-rejected transaction preserving the old cartridge and RAM; weak observation without ownership.
**Operational verification:** CMake/CTest entry added to existing NES course, plus an isolated GCC/Clang GitHub Actions workflow (`.github/workflows/allmight-cpp23-nes-course.yml`). Workflow execution and native macOS C++23 tests are **not yet confirmed**, and no result may be called passed until the workflow or user-side run reports success. The canonical book remains unchanged on GitHub; source insertions are committed as supplements and locally applied by `tools/apply_cpp23_project_inserts.py`.
**Not claimed:** full memory safety, complete 6502/PPU/APU timing, Vulkan/Metal synchronization, comprehensive allocation-failure coverage, concurrency-safe cartridge swaps, or entire-source-unit exhaustiveness.


## Pass 05 — C++23 generic asset pipeline: chapter-body review and implementation

The current chapter "Generic Asset Pipelines without Hidden Contracts" already includes ~1,516 lines of substantive explanation covering traits, specialization, deduction, perfect forwarding, concepts, compile-time policy, variadic batches, byte budgets, CppCon-informed lifetime/performance topics, and compile-time tile-plan examples. Those original paragraphs are not rewritten, shortened or reissued in a tiny chapter. The additive insert is located **after** the current discussion "Publication still needs a correctness proof after introducing templates" and **before** the existing heading "The real performance question is what gets instantiated, stored and moved." The next original discussion is retained verbatim.

- Long-form addition: `Supplementary/C++23 Project Labs/NES-05A - Type-Safe Asset Import and Publication.md` (25,513 characters).
- C++23 shared interface: `tools/labs/nes_course/typed_asset_pipeline.hpp`. One constrained `AssetStore::import<Tag>` operation, with type-specific `Spec<Tag>` decoders. It reuses **the existing NromCartridge** with its iNES parser and adds documented educational TXR4 (RGBA8) and MSH1 (quantized triangle mesh) formats.
- Executable: `tools/labs/typed_asset_pipeline_cpp23_lab.cpp`, with semantic invalid-index rejection, endian tests, bounded byte calculations, typed failures, forced preparation failures, ownership independence, mixed record storage, typed lookup, and 256-slot growth with old index re-lookup. No commercial ROM, graphics driver or GPU is needed.
- Existing CMake/CTest now has five independent cumulative C++23 targets. The pre-existing GitHub Actions workflow follows `tools/labs/**` and the Book01 CMake file, so it is configured to recheck the latest change.
- Manuscript safety: `tools/apply_cpp23_project_inserts.py` recognizes five idempotent blocks; `tools/tests/test_cpp23_project_inserts.py` checks that removing the five new blocks exactly recovers the original fixture bytes, including earlier applied changes and partial-insertion conflicts.
- **Execution boundary:** writing the code and tests does not prove they pass. There is no verified current macOS compile/CTest/Actions result and no original emulator integration. Do not mark this a completed NES/asset engine. The next verification pass must inspect GCC/Clang compilation and all CTest failures before claiming a tested implementation.
- **User workflow:** all changes accumulate on `curriculum/cpp23-nes-project-pass01`. The user will pull/apply/build only after the chosen day or book is finished. Do not issue per-pass Mac commands unless requested.
- **Next:** Chapter 06 "Grow Registries without Invalidating Identity" must replace the intentionally weak append-only slot token with an owner-aware generational handle model, prove reuse/invalidation, and keep the new asset importer as its input boundary.

The TXR4/MSH1 examples are original educational wire formats, not PNG, glTF, KTX, Maveb or Kairo importers. They do not prove GPU row alignment, compressed texture legality, topology repair, frame-sync or concurrent asset hot reload. GPU and engine integration remain future deliverables.


## Pass 06 — Chapter 06 source-based extension: generational identity across a real typed import pipeline

The existing derived chapter "Grow Registries without Invalidating Identity" is ~1,331 lines and **already contains a generational registry laboratory** together with vector invalidation, amortized growth, free slots, name-based lookup and safe borrowing. Pass 06 does not replace that material or claim the original chapter lacked it. Its new role is to connect the *actual* NES-01 through NES-05A C++23 types to the stronger registry contract that the existing M015 chapter motivates.

- Source addition: `Supplementary/C++23 Project Labs/NES-06G - Generational Identity and Safe Reuse.md` (~27.5k chars). Insert before the original section `## From a living asset registry to reliable queries and transformations`, preserving all original text before and after.
- Shared source implementation: `tools/labs/nes_course/generational_asset_registry.hpp`. Uses the existing `Spec<Tag>` decoders and `AssetRecord` variant from Pass 05. A handle carries process-local registry owner, slot, generation and compile-time type; lookup separately verifies the actual variant alternative.
- Test executable: `tools/labs/generational_asset_registry_cpp23_lab.cpp`. Synthetic NROM, RGBA8 and quantized mesh inputs; 700-slot relocation, mixed-kind reuse, forged wrong-type access, owner namespace rejection, move transfer, generation overflow retirement, 2,000 repeated erase/reuse cycles, injection before/after staging, and zero-capacity/missing-slot behavior. All cases are authored; no invalid/dangling pointer is dereferenced.
- API limitations: single-threaded registry, process-local non-persistent token, no cryptographic access control, no GPU release fence or reader pin. Validated handles do **not** stabilize borrowed raw pointers across mutation.
- Integration: `tools/apply_cpp23_project_inserts.py` now stages six idempotent source additions, `tools/tests/test_cpp23_project_inserts.py` has six-section byte-restoration fixtures, and the Book01 CMake/CTest contains six cumulative C++23 executables. A GitHub workflow exists but a **passing run has not been verified**, and no native Mac tests have been executed.
- User workflow: no interim pull commands. Continue committing revision passes to the same branch; consolidated user-side verification and canonical application are deferred until the user calls an end-of-day or book checkpoint.
- Next chapter: `07 — Typed Interfaces, Identity, and Data Structures` should test which internal representation and lookup protocols are justified for the same registry, add explicit complexity/measurement evidence and avoid new unrelated one-off projects.

All original continuous-book bytes remain authoritative. "Supplementary" here means a staging location for an additive insertion, not a substitute short book and not permission to delete an established narrative.


## Pass 07 — Chapter 07 CPU execution and debugger structures on the continuous project

**Existing chapter inspection:** "Typed Interfaces, Identity, and Data Structures" has 877 lines and >128K characters in its derived edition. It already explains PIMPL and strong/phantom types, mathematical dimensions and expression templates, intrusive ownership, CPU pipelines and caches, SIMD, allocation and concurrent scheduling, recurrence analysis, dynamic arrays, rings, hashes/Bloom filters, heaps and disjoint sets. Those explanations were inspected at their actual chapter headings and relevant transitions; they remain **unchanged**. This pass is not a reduction of that material into a few headings.

**Additive long-form lesson:** `Supplementary/C++23 Project Labs/NES-07C - CPU Instructions and Typed Debugger Structures.md` (~35.6K characters) is placed at the precise boundary before `## The scene is now too large for the wrong container — Phase 5, M025–M027, Parts 401–445`. The lesson traces a single bounded 6502 execution and debugger workload, connecting the original CPU + DSA narratives rather than inventing unrelated topic examples.

**Cumulative C++23 implementation:**
- `tools/labs/nes_course/cpu6502_subset.hpp`: byte-accurate fetching through the actual NES-01/02 owned NROM/CpuBus, 6502 instruction subset EA, A9, A2, 85, E8, 18, 38, 69, D0, F0, 4C, typed PC/register state, binary ADC flags, branch offsets/page crossing and instruction-cost accounting. Explicit failure for unsupported opcodes including BRK; no claim of cycle-by-cycle side effects.
- `tools/labs/nes_course/cpu_debug_instruments.hpp`: bounded lossless FIFO ring with pre-step backpressure, ordered breakpoint lookups, visited-PC Bloom negative prefilter, priority heap for bus inspection, and offline DSU equivalence of RAM aliases. The real hot bus stays a bitmask, not a DSU.
- `tools/labs/nes_course/cpu_debug_session.hpp` and `cpu_debug_session.cpp`: a genuine separately compiled PIMPL boundary with move-only private implementation, owned CPU snapshots and explicit moved-from errors.
- `tools/labs/nes_cpu_debugger_cpp23_lab.cpp`: synthetic NROM-128/NROM-256 reset vectors and 6502 instruction traces, flag and cycle checks, bus write/read mirrors, branch page crossing, negative relative offsets, unsupported BRK, MMIO fetch failure, full trace ring rollback, FIFO wraparound, priority fairness, Bloom observed positives, ordered breakpoints, DSU alias verification, and PIMPL move semantics.

**Build and chapter-preservation routing:** Seven C++23 cumulative CMake/CTest targets, with the Pass07 target compiling **two different translation units**. `tools/apply_cpp23_project_inserts.py` stages seven marker-delimited, idempotent additions and `tools/tests/test_cpp23_project_inserts.py` includes the seventh in byte-restoration tests. The original canonical book is not edited on GitHub; staged additions are intended for a later consolidated local integration and review.

**Verification boundary:** The newly written source is committed and cross-checked for consistency, but no fully successful GCC/Clang/macOS CMake/CTest execution or prior-source-preservation fixture run is asserted for Pass07 without output. A green test message **in source code** is not a verified test execution. No real game ROM, complete CPU, PPU/APU/interrupt/DMA/timing, actual Kairo code change, measured performance or stable plugin ABI is claimed.

**User handoff:** Do not require a Git pull, local command or LaTeX build between passes. User plans one consolidated application and verification at end-of-day or book checkpoint.

**Next:** Chapter 08 "Graphs, Search, and GPU Scheduling" should make work-dependency graph tasks emerge from this same running system, while keeping actual GPU API scheduling conceptually and empirically separate from the NES instruction executor. Any new algorithmic structure must earn its place through a real query or dependency invariant rather than being included as a buzzword.


## Pass 08 — Chapter 08 graph scheduling against the established NES system

The derived chapter "Graphs, Search, and GPU Scheduling" has approximately **2,263 lines** and ~89.7k characters of existing graph, search, scheduling, GPU-API, shader-ABI and kernel-work-distribution material. It already explains BFS/DFS, shortest paths, A*, SCC, Kahn topological sort, cache/workload decisions, GPU execution/memory ordering and scans. Pass 08 is **not** a replacement or summary. It attaches one coherent C++23 *project-based experiment* exactly **before** the original heading `## An API is a promise about an operation`, preserving the source on both sides.

**New long-form addition:** `Supplementary/C++23 Project Labs/NES-08G - Dependency Graphs and Explicit GPU Work Contracts.md` (~35.2k chars); derives explicit task dependencies, read-after-write/write-after-read/write-after-write hazards, resource generation semantics, deterministic Kahn scheduling, longest critical path on a DAG, overflow-checked CPU scan, host execution boundary and backend synchronization obligations. Names the distinct Vulkan execution/memory/queue requirements and Metal/Metal 4 resource tracking conditions using current Khronos/Apple primary documentation.

**New shared C++23 planner:** `tools/labs/nes_course/work_dependency_graph.hpp`. Task IDs, abstract work domains, resource keys with owner/slot/generation, explicit predecessors, per-task read/write declarations, expected-constant-time duplicate edge merging, deterministic topological sort, layer and earliest-start/finish computation, typed graph errors, and a list of GPU-related edges requiring backend-specific synchronization review (not native API barriers). Resource accesses are conservative whole-resource declarations, not exact image mip, byte-range or physical-alias tracking.

**New executable:** `tools/labs/nes_work_dependency_graph_cpp23_lab.cpp`. 4-task RAW/WAR/WAW/read-read oracle, duplicated explicit/hazard edge merging, typed missing/cyclic/overflow refusal, deterministic tie-breaks and repeated plan identity, generation-aware resource key disambiguation, real import through earlier typed NROM+texture decoders, real host execution using the separately compiled Pass07 PIMPL debugger and NROM CPU bus (LDA #$7F then STA $01), CPU RAM mirror assertions, and GPU-only planning for transfer/compute/graphics tied to the CPU overlay. Also includes 30 fixed-seed, 60-task DAG checks (1,800 submitted task records), and checked exclusive-scan oracle for output counts (2,0,3,1) including empty and overflow cases.

**Build wiring:** Book01 CMake/CTest has **eight** cumulative C++23 targets; Pass08 includes both its test .cpp and the existing Pass07 `cpu_debug_session.cpp` separate implementation. The source insertion tool has **eight** marker-delimited additions and the fixture test's byte-for-byte recovery oracle includes all eight. The existing GitHub Actions workflow follows all relevant changes; complete CI results have **not** been observed and there is no verified native Mac compiler run in this pass.

**Nonclaims:** no new full NES CPU, no GPU execution, no Vulkan `VkDependencyInfo`, no Metal command buffer or MTL4 submission, no fence, no hardware benchmark, no verified Kairo render-graph or Maveb source integration. A planned host order and matching resource labels do not prove concurrent safety or that a later physical resource allocation is still alive. Stale asset handles must be validated at execution time, and actual GPU resource lifetime must be governed by backend completion semantics.

**User workflow:** no per-pass commands; all changes accumulate on `curriculum/cpp23-nes-project-pass01` for consolidated source application and verification at the requested end-of-day or end-of-book checkpoint.

**Next:** Chapter 09 "GPU Memory, Divergence, Atomics, and Ordering". Develop a device-neutral resource-state oracle, explicit allocation/range/readiness contracts and carefully scoped native-backend or simulation experiments. No pretend GPU pass claims.


## Pass 09 — GPU memory contracts and host-language ordering on the existing NES texture pipeline

**Chapter inspection:** The derived "GPU Memory, Divergence, Atomics, and Ordering" chapter contains ~3,356 lines / 56.98k characters of existing device/global/shared/uniform memory, unified-memory caveats, coalescing, cache behavior, divergence, subgroup and wave operations, atomics, barrier uniformity, multi-dispatch scan, general GPU compute and stable radix/compaction material. Its original explanations are preserved. This is a deep additive application of existing theory, **not** a summary or claim that the concepts were absent.

**Insertion:** Stage `Supplementary/C++23 Project Labs/NES-09M - GPU Memory and Ordering Contracts.md` (~37.3k chars) immediately before `### GENERAL COMPUTE & PARALLEL ALGORITHMS`. This is a unique conceptual boundary between the original ordering/memory-model section and compute algorithms. The canonical master file remains untouched on GitHub. Exact anchor presence in the giant master will be confirmed by the fail-closed local checker at the user's final checkpoint; Github's large-file adapter does not provide content to inspect it fully.

**C++23 implementation:** `tools/labs/nes_course/gpu_memory_contract_oracle.hpp`. Uses Pass08 `ResourceKey` for owner/slot/generation and `Domain` labels. The CPU-only model enforces power-of-two alignment and checked suballocation, per-byte initialization, versioned producer/consumer visibility, whole-allocation role/layout transitions, a deliberately separate execution-only transition and memory dependency, typed invalid accesses, byte-exact transfer and compute-read checks, simulated submission completion/tombstone retirement, illustrative 32-byte access-segment counting and workgroup-barrier participation classification. It is not a Vulkan or Metal implementation, does not allocate placed device memory, does not do physical alias reuse, and does not implement real GPU cache behavior.

**Executable:** `tools/labs/nes_gpu_memory_contract_cpp23_lab.cpp`. Real RGBA8 teaching image imported through Pass05 `Spec<TextureTag>` and Pass06 `GenerationalAssetRegistry`; a Pass08 host/transfer/compute/graphics DAG defines dependencies; memory oracle demonstrates role/layout versus visibility and initialized byte ranges, staged upload, bytewise XOR output consumed by conceptual graphics, stale handle rejection, in-flight ticket retirement, and the no-reuse tombstone policy. Includes a **real host C++ release/acquire** two-thread handshake for 512 independent payload publications, safely acknowledged before next payload write. This is a C++ host atomics test, not a shader or Vulkan memory model test.

**Build:** Book01 CMake/CTest has nine cumulative C++23 targets; Pass09 links `Threads::Threads` for its true host-thread experiment. The source-preserving patcher stages nine marker-delimited lessons. `tools/tests/test_cpp23_project_inserts.py` was extended to check nine-section byte-for-byte recovery, idempotence and hard conflicts. There is no verified successful current CI/Clang/GCC/macOS execution of all targets; tests remain authored, not certified.

**Source caution:** The previous chapter discusses Maveb/Kairo kernels as supplied project snapshots. No fresh live revision inspection of those engines was conducted in Pass09, and the new model must not be presented as tested native rendering backend behavior. Vulkan producer/consumer stage and access scopes, queue-family semantics, resource layout/residency and Metal/Metal4 synchronization must be verified against the API and the actual device execution; current Khronos and Apple primary documentation is linked within the new lesson.

**User workflow:** Continue committing to `curriculum/cpp23-nes-project-pass01`. User does not want interim commands, local pull, manuscript application or LaTeX compilation. Consolidate build, source-preserving insertion, coverage checks and book handoff on an explicit end-of-day/book request.

**Next:** Chapter 10 "Reproducible Builds, Profiling, and Repository Contracts". The natural next milestone is an audited, reproducible C++23 build/test/benchmark protocol for the accumulating NES system, with executable evidence and reliable source routing—not a disconnected build-system glossary.


## Pass 10 — Reproducible builds, source provenance and correctness-gated profiling

**Original scope:** Chapter Edition/01 - Computing Foundations/10 - Reproducible Builds, Profiling, and Repository Contracts.md contains approximately **4,037 lines** and 64.6k characters, including full CMake target/usage-requirement instruction, build graphs, C++ modules, pinning, CI, multi-platform portability, instrumentation, crash and profiling architecture, and later C++ resource-transaction notes. Pass 10 **does not summarize or delete** that material. Its chapter addition is staged before the original unique heading `### REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW`, after the preceding target/module discussion.

**Deep addition:** `Supplementary/C++23 Project Labs/NES-10R - Reproducibility and Evidence-Driven Profiling.md`, a ~25.1K-character applied narrative covering the exact previously built NROM-to-debugger-to-task-graph system, public build contracts, provenance states, evidence refusal semantics, meaningful timing, environmental noise, CI portability, original-book preservation and research reporting. It is not a substitute short book.

**Build and tooling code:**
- `Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt` now registers ten cumulative C++23 labs, one explicit `nes_course_public_api` INTERFACE target carrying the include root and language requirements, and a tenth deterministic correctness/measurement-protocol CTest.
- `Book01/CMakePresets.json` defines isolated Ninja Debug, Release and Clang AddressSanitizer/UndefinedBehaviorSanitizer build trees. A preset alone does not pin exact SDK/compiler binary versions.
- `tools/labs/nes_course/measurement_protocol.hpp` implements bounded warmup/trial/iteration settings, deterministic result mixing, stable work checksums, time samples and documented nearest-rank median/p90 quantiles. The compiler signal fence is an optimization-discouragement tool, not a universal guarantee.
- `tools/labs/nes_reproducible_benchmark_cpp23_lab.cpp` reuses the **real existing** NROM loader, CPU subset, work graph and prefix scan. Default invocation asserts deterministic correctness; opt-in `--benchmark` records genuine measured timing rows to CSV, with no published fabricated numbers or CTest speed thresholds.
- `tools/nes_course_evidence.py` maintains a strict ten-target/ten-chapter manifest, records per-file SHA-256 of named source including the original continuous master, checks CMake registrations, JUnit execution completeness/outcomes, compiler cache, binary existence/freshness, clean source state and exact Git commit. `audit` explicitly is NOT a build result. A passing receipt may be emitted only from observed complete all-pass tests and is invalidated when source contents or commit changes.
- `tools/tests/test_nes_course_evidence.py` tests missing/failed/skipped/duplicate/malformed JUnit refusal, incomplete test counts, source-record stale digest, missing CMakeCache or executable and older-than-build XML.
- `.github/workflows/allmight-cpp23-nes-course.yml` defines four CI lanes: Ubuntu GCC, Ubuntu Clang, macOS AppleClang and Ubuntu Clang ASan+UBSan. It captures JUnit and provenance receipts as artifacts, and checks manuscript source integrity without running LaTeX.

**Source integrity:** `tools/apply_cpp23_project_inserts.py` now stages **ten** marker-delimited additive insertions; `tools/tests/test_cpp23_project_inserts.py` has byte-exact reconstruction, idempotence and conflict fixtures covering all ten. The canonical continuous source is never replaced on GitHub. The user's one-shot Mac pull and source application remain deferred until explicitly requested.

**Strict evidence boundaries:** A source manifest is not compilation. CMake configuration is not successful linking. CTest registration is not a passing test. CI YAML being committed is not a verified run. A CPU-only benchmark is not a GPU profile. A test suite reporting a green result on one toolchain would not certify whole-book completeness, hardware correctness of a full NES, or Kairo/Maveb/Kyoto source performance. Actual GitHub CI statuses, compiler diagnostics and measurement rows must be observed before any numerical claims.

**Next:** Chapter 11 "C++ Const, Borrowing, and CPU–GPU Buffering" should use the established owned cartridge, PIMPL debugger, typed resource registry and CPU reference device buffer to teach access authority, lifetime-bound immutable views, const correctness and multi-buffering with actual negative tests. The same source-preservation standard remains mandatory.


## Pass 11 — Scoped C++ const borrows, owned NES overlays and safe frame-slot reuse

**Original chapter body inspected:** "C++ Const, Borrowing, and CPU–GPU Buffering" has ~2,378 lines / ~94.97K characters of qualification safety, nested pointers, borrow-interval intersections, layout and generation proofs, GPU-generated work, frame-slot and ping-pong distinction, Vulkan/Metal command/swapchain use, CPU–GPU pipelining, buffer lifetime, indirect work and particle kernels. None of those original long-form paragraphs was removed or overwritten. The added application section is staged immediately before the original heading `## From generated GPU work to safe explicit execution`, after the const/borrow introduction, so its proof becomes a prerequisite for the original graphics material.

**Deep applied prose:** `Supplementary/C++23 Project Labs/NES-11B - Scoped Borrows and CPU-GPU Frame Buffering.md` (~43.25K characters) derives const-element vs const-wrapper access, unsound double-pointer qualification, borrow intervals, RAII lifetime and ownership, move-only publication, generation/ABA identity, the per-frame state machine, multi-queue componentwise completion, independent CPU read pins, explicit NESF overlay serialization, algorithmic ping-pong vs frame-in-flight vs swapchain image distinction, host CPU vs native device ownership and an adversarial reconstruction exercise. Current primary C++ `span`, Khronos and Apple synchronization/retention references were checked.

**Shared C++23 implementation:** `tools/labs/nes_course/scoped_frame_upload_ring.hpp`, templated bounded 1..16 slot `FrameUploadRing<Capacity>`. A process-unique pool owner + fixed slot index + nonwrapping generation names a submitted frame. Move-only `WriteLease` exclusively stages an owned payload and automatically rolls back when abandoned; `ReadLease` holds a shared heap-control owner and reader pin and returns `std::span<const Byte>` valid only while that lease lives. A separate `copy_snapshot()` returns independent owned bytes. Retirement checks resource identity, submitted state, observed transfer/compute/graphics completion and zero read pins. Max generation permanently retires a slot. Control-block ownership allows a ReadLease to outlive the pool wrapper. **Single-threaded CPU reference only**: returned raw spans can escape if caller abuses them, and completion observations are caller-provided, not observed GPU synchronization.

**Executable test:** `tools/labs/nes_scoped_frame_upload_cpp23_lab.cpp`, using existing Pass05/06 TXR4 decoder/registry, owned NROM/CPU bus and separately compiled Pass07 debugger to serialize a 16-byte NESF CPU-state header plus real 8-byte RGBA8 texture payload. Checks immutable element types, nested const compile-time constraints, 3-slot backpressure, partial queue completion refusal, reader pin refusal, writer rollback/strong replacement, staged-copy independence, move semantics, pool-owner isolation, forged slot and stale generation rejection, lease outliving wrapper, max-generation retirement, and independently acquired swapchain image index distinctions.

**Build/coverage:** The cumulative Book01 CMake target list and CTest registry now include **eleven** C++23 executable targets. Pass11 links the same real `cpu_debug_session.cpp` implementation, not a mock. `tools/apply_cpp23_project_inserts.py` now stages eleven strict marker-delimited additions and its fixture test checks eleven-section idempotence, byte restoration and conflicts. `tools/nes_course_evidence.py` manifest and the negative evidence-inventory tests are extended to eleven targets/lessons and the new shared header; four existing CI lanes will exercise the new target when actual GitHub Actions runs are available.

**Hard boundaries:** No local user commands, pull, full LaTeX regeneration, PDF creation or canonical manuscript edits during this pass. The native Mac/Linux CMake/CTest outcome remains unverified until actual run evidence is inspected. No native Vulkan/Metal execution, PPU/APU, actual swapchain semaphore behavior, device-lost recovery, concurrent ring synchronization, Kairo or Maveb source edit, or GPU performance claim was made.

**Next:** Chapter 12 "Commands, Async Work, and Resource Lifetimes" should build on the real frame-slot lifecycle to derive concrete command recording, submission, completion callbacks, error recovery and cancellation/retirement semantics. Keep the distinction between GPU execution lifetime and CPU C++ ownership explicit.


## Pass 12 — Command recording, uncertain submission and lifetime-safe completion

**Original body inspected:** "Commands, Async Work, and Resource Lifetimes" is approximately 3,198 lines / 50,901 characters of established command processors, async compute, Metal/Kairo source snapshots, persistent upload arenas, frame rings, inter-queue synchronization, profiling zones, occupancy and GPU performance methodology. Its substantive original text remains authoritative, not compressed or rewritten. The new applied lesson is staged before the original `### PERFORMANCE MODELING & PRINCIPLED GPU REDESIGN` heading, following the original command-execution flow.

**New long-form chapter:** `Supplementary/C++23 Project Labs/NES-12Q - Command Lifecycles and Uncertain Submission.md` (~47.9K characters). Connects recording vs valid sealed graph vs definite pre-submit rejection vs accepted submission vs unknown/indeterminate acceptance vs completion vs resource retirement. Includes state diagrams, transaction proof, bounded control-plane journal, at-most-once owned completion events, persistent source identity, host lifetime vs device lifetime, device-loss quarantine, native Vulkan/Metal/Metal4 differences, original NES payload and adversarial reconstruction. Official current Khronos/Apple references were consulted; no real GPU results were inferred.

**Shared C++23 model:** `tools/labs/nes_course/command_submission_lifecycle.hpp` provides move-only `CommandStream` with shared state retained by the queue and Pass08 `DependencyGraph` validation, including the exact prior GraphError failure. `CommandSubmissionQueue<Capacity>` accepts real Pass11 `FrameUploadRing` WriteLeases, creates strictly increasing submission receipts, refuses missing GPU queue stamps, supports cross-ring writer rejection and known pre-submit rejection with retryable unsubmitted writer, publishes unknown outcomes as quarantined in-flight records, requires explicit independent acceptance confirmation, accepts synthetic external per-queue completion observations, delivers owned completion-event records only once per pending→completed transition, and retires only after the underlying ring proves completion and zero read pins. Device loss refuses further acceptance/completion/retirement in this reference model. Retired journal entries may be explicitly pruned to recover bounded capacity without ticket reuse.

**Executable:** `tools/labs/nes_command_lifecycle_cpp23_lab.cpp` (~483 lines) checks malformed/empty recording, host-work refusal, duplicate/missing/cyclic graph diagnostics, move-only ownership, definite rejection/retry, indeterminate quarantine, partial three-queue completion, at-most-once owned event notices, no early writer/command reset, pinned CPU-reader retirement refusal, device lost, bounded journal and command-wrapper destruction while pending. It also imports the genuine prior synthetic NROM and TXR4 image, executes the real previously compiled CPU debugger and creates the same 24-byte NESF overlay for submission through the Pass11 ring.

**CMake/CI/source integrity:** Book01 now has **twelve** cumulative C++23 executable/CTest targets; Pass12 links the real Pass07 `cpu_debug_session.cpp` implementation, not a second invented emulator. `tools/apply_cpp23_project_inserts.py` records twelve marker-delimited additive lessons; `tools/tests/test_cpp23_project_inserts.py` fixture covers twelve strict byte-exact removals, idempotence and conflict refusal. `tools/nes_course_evidence.py` now requires twelve correct targets, staged lessons and the new shared header; its negative outcome tests expect twelve. Existing GitHub workflow path filters cover the new files for GCC, Clang, macOS AppleClang and sanitizer lanes.

**Strict nonclaims:** There is no actual GPU queue, command buffer, semaphore, fence, presentation image, asynchronous thread or native callback in this chapter's new C++ code. Decisions and completion stamps are deterministic test fixtures supplied on the host, so unknown outcome handling and device-loss behavior are *model policies*, not validated recovery for Kairo/Maveb or Vulkan/Metal. A command completion event means completed under modeled obligations, not that rendering, presentation, shader arithmetic or GPU memory visibility is correct. The entire cumulative CMake/CTest build status is still unverified until observed compiler and runner results are available; no artificial speed numbers are claimed.

**User workflow:** Continue making additive commits on `curriculum/cpp23-nes-project-pass01`; do not ask the user to pull, execute commands or regenerate LaTeX between lessons. Consolidate only on explicit end-of-day/end-of-book request.

**Next:** Chapter 13 "Open Vulkan Devices and Describe Resources", moving from device-neutral CPU models to carefully constrained native Vulkan capability/resource discovery (with optional loader and validation), while preserving all existing teaching text and avoiding invented device capabilities.


## Pass 13 — Cross-project Vulkan devices, resource requirements and seven-book project policy

**User direction:** The NES emulator was mentioned as one project example, not the sole curriculum vehicle. This is now an explicit durable rule in `Book Production/Seven-Book Curriculum/project-lab-matrix.md`, routing B1–B7 through NES/CPU, KairoRenderer/KairoGameEngine, Maveb Metal/Gaussian/TSDF, Kyoto broad phase/KairoPhysicsEngine, KairoMath/Geometry/SIMD/RayTracer, KairoGPU, Vulkax and actual XR work as topics demand. Project source inspection, runnable tests, source lineage and independent oracles are mandatory; do not force NES into unrelated concepts.

**Original chapter retained:** `Chapter Edition/01 - Computing Foundations/13 - Open Vulkan Devices and Describe Resources.md` is approximately 4,556 lines and 72,137 characters, already covering Vulkan objects and queues, swapchains, resources, memory, descriptors, shader interfaces and graphics pipeline. The new ~42.3K-character lesson `GPU-13D - Cross-Project Vulkan Devices and Resource Contracts.md` is staged immediately before the original `### BUFFERS, IMAGES, MEMORY & UPLOADS` anchor, after the original instance/device/queue discussion. No original prose, code, math or diagrams are deleted or summarized away.

**Actually read cross-project source:** KairoRenderer `VulkanInstance.cppm`, `VulkanDevice.cppm`, `VulkanBuffer.cppm`, `VulkanTexture.cppm`, `VulkanRenderer.cppm`, `MetalRenderer.cppm`; Maveb `FrameContext.hpp`, `FrameContext.cpp`, `Renderer.cpp`, `GaussianAsset.hpp`; KairoPhysicsEngine `Broadphase.cppm`; KairoGPU `README.md`. Their exact observed Git blob SHAs are recorded in the multi-project matrix. Kairo buffer destructor cleanup order (free-before-destroy) is a targeted **review item**, NOT an applied external-repo fix. Maveb alignment-overflow preconditions need source-specific validation. No remote Kyoto common-contact worktree or ILLIXR VM source was inspected and no fake current-engine performance claims were made.

**Portable C++23 target:** `tools/labs/gpu_course/device_resource_contracts.hpp` and `tools/labs/gpu_device_resource_cpp23_lab.cpp`. CPU-only typed capability snapshot, workloads for Kairo presented scene, Maveb Gaussian raster/compute, Kyoto collision candidate append, XR gaze/render and NES overlay, queue family count/capabilities, surface-specific unknown/true/false presentation status, swapchain extension requirement, 32-bit memory-type mask, required versus preferred flags, heap size and optional budget, power-of-two alignment, and fail-closed errors. Integrated/discrete examples are clearly synthetic; no real GPU result inferred.

**Optional real Vulkan bridge:** `tools/labs/gpu_course/native_vulkan_probe.cpp`. With a detected Vulkan SDK and explicit `--probe-native`, it uses real Vulkan instance/physical device/queue queries, creates a headless logical device and 256-byte VkBuffer, queries actual VkMemoryRequirements, allocates compatible VkDeviceMemory and binds/destroys buffer and memory in safe order. No native surface, swapchain, shader dispatch, presentation, GPU readback, Vulkan 1.3/1.4 feature enablement, or Metal API call is inferred. Missing SDK/device returns explicit unavailable code 77. Its CMake target is always registered; the portable CTest does **not** invoke native probing and cannot certify GPU support.

**Evidence:** Book01 CMake/CTest has 13 cumulative targets; `tools/apply_cpp23_project_inserts.py` stages 13 exact marker-delimited byte-preserving source inserts. Insertion fixtures and fail-closed source/JUnit evidence manifest now require GPU-13D and both GPU helper sources, plus the multi-project matrix. Actual native compiler, JUnit result and real physical probe success are **pending**; no hardware times/scores invented. The original canonical manuscript stays unchanged on the revision branch until the user's consolidated checkpoint.

**Next:** Chapter 14 "Submit, Synchronize, and Retire GPU Work" should use the actual native Vulkan VkDevice and VkBuffer path while retaining Pass12's command-lifecycle oracle; add explicit backend queue submission, synchronization2/legacy-feature gating, resource retirement, real validation and a Kairo renderer integration exercise with device evidence, instead of returning to another NES-only illustration.


## Pass 14 — Kairo Vulkan and Maveb Metal frame submission and retirement

**Original Chapter14 inspected:** `Chapter Edition/01 - Computing Foundations/14 - Submit, Synchronize, and Retire GPU Work.md` contains ~4,398 lines/71,270 characters, already describing command pools, primary and secondary command buffers, queue submit, fences, binary/timeline semaphores, synchronization and memory barriers, multiframe lifetimes, validation, GPU timestamps, device loss, MoltenVK and RAII. It remains the full authoritative reading copy. The new ~37.4K-character lesson `Supplementary/C++23 Project Labs/GPU-14F - Frame Submission Synchronization and Retirement.md` is staged exactly before its original `### VULKAN MEMORY MODEL, BARRIERS & MULTI-FRAME LIFETIMES` heading. No chapter text is removed.

**Real repo audit:** Exact main-branch source snapshots inspected are `KairoRenderer/VulkanCommand.cppm`, `VulkanSync.cppm`, `VulkanRenderer.cppm`, `Maveb/engine/metal/src/Renderer.cpp` and `FrameContext.cpp`. Blob SHAs and precise observations are in `project-lab-matrix.md`. Kairo currently resets its in-flight fence after WSI acquire **before** command encoding/accepted vkQueueSubmit, leaving a source-level unsignaled-fence failure path on record/submit errors. Maveb's 3-frame dispatch semaphore and renderer destructor provide a callback/frame context lifetime discipline. This is a read-only investigation; no external repo code was changed or GPU failure reproduced.

**C++23 host model:** `tools/labs/gpu_course/submission_retirement.hpp` (~423 lines) provides bounded frame slots with nonwrapping generations, swapchain image states and generations, a deliberate fence-reset→definite-failure recovery obligation, pending vs indeterminate acceptance, independently verified GPU and present release notifications, device lost refusal, and generational resource tokens with GPU frame holds and CPU diagnostic reader pins. Host-generated events are **synthetic**, not native GPU fences or WSI proofs; safe native recovery requires backend evidence. The resource release predicate joins final GPU use and CPU borrowers.

**Executable adversarial cases:** `tools/labs/gpu_submit_retire_cpp23_lab.cpp` (~284 lines) tests Kairo-style fence reset before record/submit failure and explicit recovery, two outstanding frames over three WSI images, frame retirement while present image remains held, distinct swapchain generations, stale/foreign resource handles, Maveb-style frame resource reuse, Kyoto collision buffer and XR gaze resource retention, independent CPU pins, unknown submission quarantine and device-loss refusal.

**Opt-in real GPU execution:** `tools/labs/gpu_course/native_vulkan_submit_probe.cpp` (~328 lines) is compiled with Vulkan headers/library only when CMake finds them. On explicit `--probe-native` it creates a headless VkDevice + compatible host-coherent buffer, records `vkCmdFillBuffer` and transfer-write→host-read barrier, submits through `vkQueueSubmit`, waits with a **real** VkFence, maps actual memory and checks all 64 resulting 32-bit words against the independent CPU fill constant. It cleans up buffer before backing memory and protects in-flight resources on failure using a device-idle fallback. It does not claim a swapchain, graphics shader, native Metal run, Synchronization2 feature enablement or timing. SDK/device unavailable is reported as 77, not passing GPU evidence. **Native execution remains unverified** at authoring time.

**Build, source evidence and preservation:** Book01 CMake registers **14 C++23 executables and 14 normal CTest invocations**. GPU-14F's default CTest runs the portable CPU model only; native probe is always opt-in and must be accompanied by external hardware/validation evidence. `tools/apply_cpp23_project_inserts.py` has 14 strict marker-delimited manuscript additions; the byte-preservation fixtures and source-pinned JUnit/evidence manifest include the new lesson, three source files and native probe. The canonical continuous manuscript remains unchanged, without LaTeX generation or user commands.

**Next:** Chapter 15 "Execute and Measure a Metal Frame" should prioritize actual Maveb Metal/renderer source as the executable project, with hardware-conditional Metal shader/graphics checks, command completion status, resource retention and GPU timestamp interpretation. Compare Kairo Metal only at exact audited source revisions. No NES-only detour.


## Pass 15 — Maveb Metal frame execution, Kairo Objective-C++ bridge, correctness-gated GPU measurement

**Original chapter retained:** `Chapter Edition/01 - Computing Foundations/15 - Execute and Measure a Metal Frame.md` contains 3,061 lines / 47,241 characters of native Metal device, command buffer, encoder, shader, storage modes, tile memory, drawables, frame latency, synchronization, Metal-cpp/ObjC++ and profiling material. The original remains intact. The new `Supplementary/C++23 Project Labs/GPU-15M - Execute Verify and Measure a Metal Frame.md` (~43.6K characters, substantial narrative code and derivations) is staged before the original `### STORAGE MODES, TILE MEMORY, DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION` heading.

**Actual code inspected:** Connected GitHub source files `Maveb/engine/metal/src/Renderer.cpp` (GPU frame semaphores, commandBuffer, capture row stride and GPU timing), `Maveb/engine/metal/src/FrameContext.cpp` / `FrameContext.hpp` (shared upload/readback, private heap, correct alignment bounds), `KairoRenderer/MetalRenderer.cppm` (public renderer/runtime contract) and `KairoRenderer/detail/MetalBackend.mm` (actual ObjC++ Metal queue/encode/commit/present and blocking capture). Exact Git blobs are in `project-lab-matrix.md`. Those external repos were not modified; no fake source audit of their unobserved files.

**Portable ISO C++23 reference implementation:** `tools/labs/gpu_course/metal_frame_contracts.hpp` (3-frame model, nonwrapping owner/slot/generation, stage/align/size checked arenas, safe terminal-error permit reuse, separate CPU capture metadata error without stranded permit, compact BGRA8 readback, GPU timestamp validation that refuses 0/NaN/Inf, unknown submission quarantine and device-loss refusal). `tools/labs/gpu_metal_frame_cpp23_lab.cpp` tests a realistic mix of Maveb Gaussian, Kairo scene, Kyoto pair buffer and XR gaze frame tags, out-of-order completion, duplicate/stale handles, guarded capture rows, failed allocation/capture and no fabricated GPU timing. Default CTest uses deterministic synthetic host events and is NOT evidence of native GPU execution.

**Native Objective-C++ / actual Metal adapter:** `tools/labs/gpu_course/native_metal_compute_probe.mm`, only included on Apple when compiler and Metal/Foundation frameworks are available, uses an actual MTLDevice, MTLCommandQueue, original MSL library, compute and render pipeline states, shared buffer and private BGRA8 texture. It encodes a 32×32 AABB-overlap compute mask, a real offscreen red full-screen triangle and a texture→buffer blit with 256-byte-pitch rows in one command buffer. Actual `waitUntilCompleted` and terminal status are required; every pair-mask value, 16 guard elements and every 3×2 BGRA pixel are verified against independent CPU expectations. It does not use a swapchain/display. `--probe-native` is a single correctness probe; `--benchmark-native` runs 5 warmups + 30 correctness-checked repeats, distinguishes host encode-to-commit from GPU command interval, and refuses invalid/unavailable GPU timestamps rather than emitting fabricated zeros. **No native Metal run has occurred or been verified in this authoring pass.**

**Portable build fallback:** `tools/labs/gpu_course/native_metal_unavailable.cpp` returns 77/unavailable when the Metal ObjC++ backend was not selected. CMake enables ObjC++ and links frameworks only on compatible Apple environments. The CMake/CTest tree now has 15 C++23 target/test registrations; CI default invokes the CPU reference model only. All-pass compilation, macOS SDK compilation, native Metal GPU bytes, GPU timing distributions and Maveb/Kairo production performance remain unverified until exact source-pinned execution evidence is obtained.

**Preservation and user workflow:** `tools/apply_cpp23_project_inserts.py` stages 15 additive marker-delimited lessons. Exact-byte manuscript-restoration/idempotence/conflict fixtures are now extended to 15. The evidence manifest source hashing and JUnit inventory requires 15 labs and all Pass15 files. The canonical source is not replaced; no LaTeX or user-local commands are requested until the consolidated checkpoint. The user explicitly requires multiple independent long-running projects, with NES relevant to early CPU systems and Maveb/Kairo/physics/XR used for their real respective engineering topics.

**Next:** Chapter 16 `Profile GPUs and Respect Apple Ownership Boundaries` should use actual Maveb Metal capture/diagnostics, Kairo ObjC++ owner and callback lifetime, CPU/GPU timestamp protocol, asynchronous error delivery, Metal Performance HUD and resource capture; build a platform-conditional ownership/concurrency correctness test without resorting to NES as the only example.


## Pass 16 — Truthful Apple GPU profiling and asynchronous completion ownership

**Original authoritative Chapter16:** `Chapter Edition/01 - Computing Foundations/16 - Profile GPUs and Respect Apple Ownership Boundaries.md` already contains 2,467 lines and ~90,695 characters, including Metal profiling, counter scope, GPU resource lifetime, Maveb/Kairo Apple bindings, SwiftUI actor/thread boundaries, and later Vulkan allocator and systems integration. The new `Supplementary/C++23 Project Labs/GPU-16P - GPU Profiling and Apple Callback Ownership.md` is **44,409 characters** of additional narrative with proofs, failure cases, native execution protocol and project reconstruction. It is staged immediately before the original `### Distinguish host allocation callbacks from device-memory allocation` heading; no original prose is removed.

**Read-only source provenance:** `Maveb/engine/metal/src/Renderer.cpp` and `FrameContext.cpp`/`FrameContext.hpp`; `KairoRenderer/MetalRenderer.cppm` and `detail/MetalBackend.mm`. Exact connected Git blob IDs and source-level observations are recorded in `project-lab-matrix.md`. Maveb: three permit frame scheduling, completed-handler capture of `this`, zero/invalid GPU timestamp concerns, row-pitched async capture. Kairo: ObjC++/C++ façade, synchronous wait before capture/pick and relevant resource destruction. These observations are NOT native reproductions, patches to those repos or performance measurements.

**New real C++23 portable logic:** `tools/labs/gpu_course/gpu_profile_ownership.hpp` has a shared-internal-state `CompletionMailbox` that never captures a raw renderer/UI owner, mutex-protected at-most-once generation-specific completion and permit release, submitted/uncertain/aborted distinctions, typed CPU project tags, terminal status and optional GPU interval validation. `Configuration`/`FrameSample`/`CounterValue`/`summarize_native_verified` reject missing source/device/workload/tool provenance, mixed shader builds, synthetic observations, failed correctness checks, unsupported numeric counters, missing/invalid CPU/GPU intervals and too few observations. An origin field is **not a cryptographic hardware attestation**; native execution evidence must be observed separately.

**Adversarial executable:** `tools/labs/gpu_profile_ownership_cpp23_lab.cpp` is ~299 lines and runs **actual concurrent host `std::jthread` callbacks** and synthetic GPU-observation fixtures for Maveb Gaussian, Kairo frames, Kyoto collision pairs and XR. It checks duplicate/cross-owner/stale-ID rejection, destructor-before-callback safety, delayed quarantine, terminal-error evidence, counter support and strict report accept/reject behavior. Host threads are NOT proof of GPU queues; synthetic timing is NOT hardware profiling.

**Opt-in actual Metal code path:** `tools/labs/gpu_course/native_metal_callback_probe.mm` uses the real Apple Objective-C++23/ARC `addCompletedHandler:` block, actual MTLDevice/queue/compute kernel and 64 independently CPU-checked 32-bit results. It destroys the renderer-like mailbox wrapper and weak UI observer before commit, retains only a shared internal callback state, waits for real terminal command/callback delivery, checks the GPU output, and reports a valid single interval or unavailable. The source is committed but **was not compiled or executed on Metal hardware in this turn**. `tools/labs/gpu_course/native_metal_callback_unavailable.cpp` returns explicit 77 when a native toolchain/device is absent. The ordinary CTest path never invokes `--probe-native`. It does not produce Xcode GPU captures, counter samples, Maveb run evidence or XR motion-to-photon latency.

**Cumulative integrity:** Book01 CMake now registers **16** C++23 executables/normal CTest targets, with Pass16 linked against `Threads::Threads` for the real host callback concurrency tests, and ObjC++23/Metal/Foundation conditionally built only on Apple. `tools/apply_cpp23_project_inserts.py` stages **16** byte-preserving marker-delimited lessons. The fixture tests exercise all sixteen insertions, including conflict, source restoration and repeat application. The exact-source Git SHA/JUnit evidence manifest now requires all sixteen labs plus all Pass16 files; actual completed test runner evidence is **pending**. No user command, LaTeX regeneration, original text deletion or other repo edit is requested.

**Next:** Chapter 17 `Python and Numerical Tools for Graphics Systems` should change the project focus again, away from Metal/CPU emulator, and develop numerical/scientific Python workflows tied to Maveb geometry/TSDF/Gaussian fitting, Kyoto physics baseline evaluation, Vulkax scientific controls and statistically valid graphics benchmarking. Actual source/data inspection first, then both Python and C++ independent oracles and repeatable tests.


## Pass 17 — Book01 source coverage and editorial boundaries

**Authored audit:** `Book Production/Seven-Book Curriculum/Book01/PASS17_COVERAGE_AND_DEPTH_AUDIT.md` is the 16-chapter/16-supplement source inventory, project-boundary analysis and hard release-condition report. Source chapters 1–16 total about 1,441,560 characters as their derived reading copies; staged new lessons total about 507,498 characters. This is not a completed merged book nor a proof of conceptual correctness.

**Reading-map mapping:** The 39-chapter original manuscript is split by topic into seven proposed books, not 39 sequential new Book01 tasks. Early C++/ownership/system chapters 1–8 plus build/ch10 and borrow/async material 11–12 form a major Book 1 core, with GPU/Metal 9 and 13–16 strongly belonging to B6 as well, numerical chapters 17–31 to B2, camera/geometry chapters 32–38 to B3, research chapter39 cross-book. Explicitly own each source span in the eventual release map; do not duplicate or drop it.

**Exact structural checks verified by fetched reading-copy source:** All 16 staged lessons have exactly one matching insertion heading in their corresponding original derived chapter. A local full-master byte-level count requires execution on the checkout; GitHub source view for the giant canonical master returns its unmodified blob ID but does not stream every byte to this conversation. A new read-only `tools/book01_coverage_audit.py` validates all 16 chapter markers, canonical master source anchors, precise baseline blob, CMake and JUnit evidence source registration. Its negative tests and audit JSON are wired into the GCC CI workflow. **No successful toolchain/CI result has been observed at authoring time.**

**Substantive C++23 gap:** Lexical inspection of all 39 derived chapter files and all 16 staged lessons found no actual API spellings `std::format`/`std::print`, `std::filesystem`/file streams, `std::ranges`/`std::views` or coroutine `co_await`/`co_yield`/`promise_type`/`std::coroutine`. Related prose may exist and other source files were not all checked. These are real *worked-example coverage gaps* relative to a complete modern C++ book, to be closed through one coherent Kairo/Maveb asset+async service with portable negative tests and independent oracles.

**Readability audit:** Several original GPU chapters have median nonempty line widths ~12–15 characters and production-era breadcrumbs like “This is Part 2147” or “+ M135.” Keep all original bytes while staging transitions, explanatory derivations and code-level through-lines. Visual figure need remains a design decision, not proof that prior figure assets are absent.

**Release requirements:** The verified 16-target CTest and native-platform evidence pipeline, complete C++23 interface coverage, real multi-project ownership/asset/compute pipeline, selected graphics/physics/numerical witnesses, a narrative/editorial review, primary-book/source-span routing and authorized master application must all be complete. A passing static audit checks layout/presence only, not the quality or correctness of the book.

**Next:** Actual build/repair of the authored 16 labs, plus gap-closing C++23 filesystem/ranges/format/coroutine teaching work. Native Apple/Vulkan runtime outcomes must be reported independently. No request to run LaTeX, export books, or run local commands until the consolidated checkpoint.


## Pass 18 — Actual modern C++23 asset ingestion, files, ranges and owned coroutines

**Placement:** Book 1 / Chapter 06, directly after the established
generational typed asset registry and before the original “From a living asset
registry to reliable queries and transformations” heading. The 17th
`CPP-18A` lesson shares that original anchor with `NES-06G`. The strict
patcher applies the earlier insert first, then this additional gap repair;
neither changes or replaces the original source bytes. The first 16
supplemented *original chapters* remain 16; 17 is the **lesson count**.

**Original/source provenance:** `tools/labs/nes_course/typed_asset_pipeline.hpp`
and `generational_asset_registry.hpp` were actually inspected for their
`Spec<TextureTag/MeshTag>::decode` and
`GenerationalAssetRegistry::import/lookup/erase` APIs. These are the
shared code used in the new full-course target; they are not duplicated or
reimplemented. The target's public include directories and compile
definition `ALLMIGHT_COURSE_REQUIRE_REGISTRY=1` make the full build **fail
at compile time** if that original header is unavailable, instead of
silently skipping the important cross-pass integration.

**Substantive new teaching:** `Supplementary/C++23 Project Labs/CPP-18A - Filesystem Ranges Formatting and Coroutine Asset Import.md`
now exceeds 50K characters after bounded-input and cancellation repairs. It directly closes the identified worked-example
gaps for `std::filesystem` and `std::ifstream`/binary reads,
`std::ranges::sort` and `std::views::filter` with owned range lifetimes,
`std::format` and `std::println` with library feature gating, a
move-only `std::coroutine_handle` generator with `co_yield`,
a manually suspended `co_await` task with owned result state, and a
genuine host `std::jthread` worker with cooperative stop, mutex-protected
results, exception containment and aggregate byte budget. The text
distinguishes synthetic TXR4/MSH1 fixtures from real glTF/KTX/Maveb data,
and explicitly refuses to call portable pre-open symlink checks a
race-free security sandbox against malicious concurrent directory edits.

**Executable:** `tools/labs/foundations_course/modern_asset_ingestion.hpp`
and `tools/labs/modern_asset_ingestion_cpp23_lab.cpp`. The test creates
actual temporary directories, builds valid educational texture/mesh files,
refuses absolute/parent/dot/empty/NUL/symlink/missing/directory paths,
enforces file-size and aggregate batch budgets, verifies owned
coroutine payloads after caller-vector/file removal, validates duplicate
paths and two-stage cancellation, and uses the real earlier
`GenerationalAssetRegistry` in the full course include tree. File read,
domain decode, publication and GPU resource lifetime are distinct
contracts; a successful first contract is never misrepresented as proof
of the later contracts.

**Observed local compilation evidence:** GCC 14.2, Clang 17, optimized
GCC and Clang ASan+UBSan compiled and executed the standalone
source-equivalent lab with strict C++23 warnings. The local authoring
workspace lacks the earlier registry header, so the standalone runner
**explicitly reported that the Pass06 integration was not exercised**.
The full CMake target now *requires* that header at compile time; a
negative local compiler probe verified that it rejects the missing
header as intended. Source-pinned 17-target CTest, actual integrated
registry runtime and native GPUs remain unverified, with no fake green
receipt. Detailed evidence: `Book01/PASS18_CPP23_ASSET_PIPELINE_EVIDENCE.md`.

**Updated static and execution contracts:** `tools/apply_cpp23_project_inserts.py`
contains 17 addition specs, strict fixture tests include CPP18A in
restoration/idempotence/conflict proofs, the source-JUnit manifest requires
17 CTest results, and `tools/book01_coverage_audit.py` checks the
**17th supplemental lesson is anchored in original Chapter06** rather
than expecting a nonexistent seventeenth original book-one chapter.
The GitHub Actions workflow already triggers on these files; its results
must be read from an actual runner before asserting full-course success.
CMake/project preset *display labels* now reflect Book01 cross-project
work rather than NES exclusively, without renaming the earlier targets
or touching the original prose.

**Resumed Pass18 source and test hardening:** Request-count bounds (default
1024, maximum 4096) and a 4096-byte per-path ceiling now reject enormous
manifests synchronously before starting `std::jthread`. This closes the
bounded-error-queue hole left by a budget that counted only successful
payload bytes. `Root::load` now accepts the worker's `std::stop_token` and
checks it before path resolution, between 4096-byte file chunks and
before publication; it does not falsely promise preemption of a blocked
native I/O syscall or race-free path resolution.

**Observed byte-identical standalone source revisions:** header
`23a581956e35b8ffd6116e5483ff96a49e790be6`; test
`28369c2bb8ac68d0c08b529b02152c6d5038c46c`. After rebuilding
those exact Git blobs locally, GCC 14.2 C++23 strict warnings and
Clang 17 C++23 with ASan/UBSan each executed one CTest successfully,
with zero failures and recorded standalone JUnit receipts. The *full*
17-target course and Pass06 registry runtime integration were not run
in that working container, and no CI/native hardware success is claimed.
The evidence ledger contains the exact measurement boundary.

**Next high-priority gate:** Full repository 17-target GCC/Clang/AppleClang
build, sanitizer tests, actual legacy-registry integration and JUnit
receipts, followed by source-level correction of real failures. A
production extension should add race-resistant native filesystem access,
bounded number of jobs/consumer queue, durable file transactions and
scene-generation-aware asset publication. Native GPU correctness must
remain a separate verified evidence stream.
