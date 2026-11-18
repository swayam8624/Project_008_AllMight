# Source-to-seven-book routing: Computing Foundations (39 chapters)

**Status:** initial structural routing, 18 November 2026. This is a chapter-by-chapter placement proposal based on the current **chapter reading map** and a close inspection of the opening chapter, NOT a claim that all 39 source chapter bodies, figures, citations, or executable examples have been independently reviewed. Their full content remains authoritative in the continuous master. Before cutting/rebuilding manuscripts, inspect each chapter body and record unique subsections and source IDs. No source deletion is authorized by this map.

**Book abbreviations:** B1 Bits, Programs and Reliable Systems; B2 Mathematics for Visual Computing; B3 Geometry, Cameras and Image Formation; B4 Light Transport and Image Synthesis; B5 Spatial Worlds, Engine State and Simulation; B6 GPU Architecture and Real-Time Rendering; B7 Vision, XR and Playable Worlds.

| Existing chapter | Primary home | Project-driven first-use / integration decision |
| --- | --- | --- |
| 01 From Bits to Running Systems | B1 | **NES-01**, iNES header interpretation, typed errors, tests, bytes and flags. Started and compiled. |
| 02 Values at Addresses: Layout, Lifetimes, and Representation | B1 | **NES-02 authored; pending canonical insertion**. Owned NROM cartridge, RAM mirroring, PRG mapping, typed missing-device errors, failed-load publication, moved-from safety and strict C++23 tests. Existing original text retained. |
| 03 Records as Resource Transactions | B1 | Cartridge ownership, constructors, expected failure, strong invariants. |
| 04 Prove C++ Ownership with Runnable Experiments | B1 | RAII/move/borrow experiments against a progressively safer cartridge and byte-buffer interface. |
| 05 Generic Asset Pipelines without Hidden Contracts | B1 | C++23 generic loaders and type-specific cartridge/asset validation; bridge to KairoAssets. |
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
