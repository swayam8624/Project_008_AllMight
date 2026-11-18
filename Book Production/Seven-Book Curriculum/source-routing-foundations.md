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
| 11 C++ Const, Borrowing, and CPU–GPU Buffering | B1 (borrowing), B6 (buffers) | std::span, lifetime ownership, transfer queues; use KairoRenderer later for device evidence. |
| 12 Commands, Async Work, and Resource Lifetimes | B1 (concurrency), B6 (submission) | Bounded async loading, work queues, cancellation/retirement; avoid equating CPU completion with GPU completion. |
| 13 Open Vulkan Devices and Describe Resources | B6 | KairoGameEngine Vulkan backend: selection, swapchain, validation, resource states. |
| 14 Submit, Synchronize, and Retire GPU Work | B6 | Synchronization2, timeline semantics, ownership transfer; device validation required. |
| 15 Execute and Measure a Metal Frame | B6 | Maveb/Kairo Metal backend; no fabricated Vulkan/Metal equivalence. |
| 16 Profile GPUs and Respect Apple Ownership Boundaries | B6 | Apple GPU captures and Objective-C++/Swift interoperability boundary; C++23 core. |
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
