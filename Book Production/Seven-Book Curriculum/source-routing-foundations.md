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
| 06 Grow Registries without Invalidating Identity | B1 | Safe generational handles for cartridge/texture registries; later ECS uses same proof. |
| 07 Typed Interfaces, Identity, and Data Structures | B1 | Refactor emulator bus and MLLibrary types; verify container choice with measurements. |
| 08 Graphs, Search, and GPU Scheduling | B1 (algorithms), B6 (GPU) | Graph traversal and dependency proofs in C++23; grow into KairoScheduler/render graph. Split GPU-specific material at first hardware use. |
| 09 GPU Memory, Divergence, Atomics, and Ordering | B6 | C++ host-side scheduling and shader execution laboratory; keep C++ memory-model bridge in B1. |
| 10 Reproducible Builds, Profiling, and Repository Contracts | B1 | CMake/CTest, sanitizers, deterministic NES traces; extend same harness across other repos. |
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
