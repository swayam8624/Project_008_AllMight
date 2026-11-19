# Project AllMight — Seven-book C++23 curriculum (in progress)

> **Correction, 18 November 2026 — preserve the full books.** This branch is an **additive revision layer** for existing long-form manuscripts. The older two-page LaTeX preview was a small standalone proof-of-format, **not a revised chapter, not a replacement for the thousands of pages, and not the expected amount of chapter work**. It must never be substituted for the canonical chapter or used to infer that a chapter is complete. No existing source has been deleted or shortened on this branch. The current manuscript source is the continuous Markdown under `Continuous Notes/`; its existing large file cannot be safely replaced using the limited content-fetch operation. New material is first developed as a source-preserving insert and applied locally, without regenerating the book.

**Actual NES-01 addition:** [Supplementary/C++23 Project Labs/NES-01 - Cartridge Bytes and Trusted Layout.md](../../Supplementary/C++23%20Project%20Labs/NES-01%20-%20Cartridge%20Bytes%20and%20Trusted%20Layout.md) — written to be inserted directly after the original bit-field exercise, before the existing heading "Arithmetic becomes a policy". The older lessons are retained verbatim. The runnable C++23 lab lives at [tools/labs/ines_header_cpp23_lab.cpp](../../tools/labs/ines_header_cpp23_lab.cpp). The former `Book01/01-bits-to-cartridge-nes.tex` remains an **optional prototype** only, not the canonical seven-book teaching source.

**On your Mac, from the repository root (safe first):**

```bash
git fetch origin
git switch curriculum/cpp23-nes-project-pass01
python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/apply_cpp23_project_inserts.py --apply
python3 tools/apply_cpp23_project_inserts.py --check

git diff --check
git diff --stat
git diff -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
```

The script refuses ambiguous anchors and modified partial insertions, checks that removing the inserted byte range exactly recovers the original source, and reports the original and final SHA-256. It makes **no PDF, TeX, export or regenerated reading edition**. Do not use it as a substitute for a source-by-source technical review. Updating derived Chapter Edition or LaTeX exports is a distinct, user-controlled action once the canonical revision has been reviewed.

**Build policy:** ChatGPT must not run LaTeX, render PDFs, or rebuild the book editions. Provide explicit Mac commands only when requested or when the proper existing master TeX file and its build tooling are identified. A synthetic C++ lab is evidence only for its named tests, not a full-emulator certification.


**Status:** authoring track, not a finished seven-book production build. The repository currently has three canonical continuous manuscripts and 129 chapter-edition chapters, with a six-volume print pipeline. These remain authoritative and unchanged. The seven titles below represent the seven-stage teaching architecture, **not** seven verified, already-existing TeX masters. Do not misreport a finished book, complete source review, or executable milestone on the strength of this file.

## Seven-book teaching route

| Book | Teaching question | Main growing project |
| --- | --- | --- |
| 01 — Bits, Programs and Reliable Systems | How do signals and bytes become safe running state? | **KairoNES / NESEmulator**: binary headers → memory bus → 6502 → graphics/audio → verified machine |
| 02 — Mathematics for Visual Computing | How do we derive and solve models reliably? | **MLLibrary, KairoMath, KairoGeometry**: numerical primitives → transforms → solvers → uncertainty |
| 03 — Geometry, Cameras and Image Formation | How do geometric worlds form measurable images? | **KairoGeometry, Maveb**: robust predicates → projection → calibration → reconstruction |
| 04 — Light Transport and Image Synthesis | How do physical light models become rendered pixels? | **KairoRayTracer, Maveb**: ray intersections → BVH → path tracing → validation |
| 05 — Spatial Worlds, Engine State and Simulation | How do worlds change without violating identity or physics? | **KairoECS, KairoPhysicsEngine, Kyoto Common Contact**: components → spatial indices → contacts → stable simulation |
| 06 — GPU Architecture and Real-Time Rendering | How do CPU intentions execute as efficient GPU work? | **KairoRenderer, KairoGameEngine, Maveb**: buffers → rasterization → PBR → Vulkan/Metal render graphs |
| 07 — Vision, XR and Playable Worlds | How do observation, perception and interaction close the loop? | **Maveb, Vulkax, FoveaBench/ILLIXR, KairoGameEngine**: vision → XR → gameplay → research replication |

Projects are grounded in the listed *repositories*, not presumed to be production-correct. Every reused implementation must be inspected at an exact commit, its defects noted, and its platform/verification scope stated. Existing Python, C, C# or Java systems can serve as sources to understand or compare; the **learner's primary implementations are C++23**. Graphics APIs, shaders, operating-system SDKs and external research tooling may require other languages at interop boundaries; explain these boundaries rather than misrepresenting them as C++ language features.

## Pass 01 — Book 01, Chapter 01: NES cartridge headers

This is the first **authored** intervention, not a blanket review of all 129 chapters. It supplements the existing opening on bytes, representation, storage and contracts.

- LaTeX source: [Book01/01-bits-to-cartridge-nes.tex](Book01/01-bits-to-cartridge-nes.tex). Chapter-module source, no document preamble, suitable for a later explicit \input into the seven-book build.
- Tested C++23 reference: [../../tools/labs/ines_header_cpp23_lab.cpp](../../tools/labs/ines_header_cpp23_lab.cpp). Self-contained iNES 1.0 layout inspection with synthetic malformed-cartridge tests.
- Case-study source: [NESEmulator/Cartridge.cpp](https://github.com/swayam8624/NESEmulator/blob/main/NESEmulator/Cartridge.cpp), compared with [NESEmulator README](https://github.com/swayam8624/NESEmulator/blob/main/README.md). The source can read a short header and truncated payload without checking extraction success and can accept unsupported mapper cases.
- Verification scope: code compiled and all 11 synthetic cases executed with GCC 14 in C++23 strict-warnings mode in the authoring workspace; stand-alone TeX module compiled into a two-page preview. **Not yet tested in the repository's own CI or native full seven-book production build.**
- Subsequent milestones: NES-02 checked memory bus and mirroring; NES-03 6502 state and instruction traces; NES-04 interrupts/cycles; NES-05 PPU; NES-06 APU/controllers; NES-07 mappers and cartridge metadata; NES-08 deterministic regression and performance.

### Reproduce NES-01

From the repository root:

```bash
cmake -S "Book Production/Seven-Book Curriculum/Book01" -B build/nes-01 \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/nes-01
ctest --test-dir build/nes-01 --output-on-failure
```

The header and address-mirroring rules were independently checked against
[NESdev iNES](https://www.nesdev.org/wiki/INES),
[NESdev NES 2.0](https://www.nesdev.org/wiki/NES_2.0), and
[NESdev mirroring](https://www.nesdev.org/wiki/Mirroring).
The lab intentionally declines NES 2.0, does not yet interpret older dirty-header
conventions, and does not claim to instantiate an actual mapper. Files with
additional trailing bytes may be valid under extensions; their treatment will be
specified in the next cartridge-loader milestone.

## Pass 02 — Book 01, Chapter 02: owning cartridge state and address mapping

**Status:** authored, tested, and staged for surgical insertion; **not yet applied to the canonical multi-thousand-page manuscript on GitHub**. The source-preserving local patcher now handles both NES-01 and NES-02, including machines where NES-01 was applied previously, without rewriting any original manuscript bytes.

The existing 2,417-line chapter-edition copy of "Values at Addresses: Layout, Lifetimes, and Representation" already covers byte order, alignment, virtual memory, storage, object lifetimes, build systems, identity, generational handles and binary schemas. Pass 02 does not replace or abridge these lessons. Its **14,645-character addition** goes at the first-use ownership/handle boundary, immediately before the pre-existing heading "Choose a type for its promise, not its familiar size":

- **Additive long-form lesson:** [NES-02 — Owned NROM and Memory Bus](../../Supplementary/C++23%20Project%20Labs/NES-02%20-%20Owned%20NROM%20and%20Memory%20Bus.md). Distinguishes file bytes, borrowed views, owned cartridge state, address identities and hardware mappings; traces NROM-128 versus NROM-256; includes explicit failure behavior, source comparisons, exercises and accuracy limits.
- **New C++23 executable:** [NES-02 owned NROM and CPU-bus lab](../../tools/labs/nes_bus_nrom_cpp23_lab.cpp). Keeps 2 KiB CPU RAM, 16-/32-KiB NROM program mapping, 8-KiB CHR ROM or CHR RAM and unsupported hardware regions distinct. Rejected cartridge parsing and moved-from publication cannot silently replace live state.
- **Single parser source:** [shared iNES C++23 header](../../tools/labs/nes_course/ines_layout.hpp), also used by the NES-01 test. No duplicate parser bodies.
- **Evidence:** GCC 14 and Clang 17 compiled both NES labs under C++23 with strict warnings; CMake/CTest ran both tests; NES-02 passed an ASan/UBSan run locally. It includes deterministic exhaustive checks of all 8,192 internal-RAM mirror addresses and both 16-KiB PRG mirror windows. The connected GitHub branch is not claimed to have run CI or native macOS tests.
- **Insertion safety:** [tools/apply_cpp23_project_inserts.py](../../tools/apply_cpp23_project_inserts.py) dry-runs by default, detects changed/partial previous inserts, and proves that removing each new block recovers the preceding book exactly. [Tools tests](../../tools/tests/test_cpp23_project_inserts.py) exercise preservation, idempotence, existing NES-01 compatibility and failure behavior in disposable fixtures (committed, not yet independently run on macOS).
- **Source boundaries:** [NESdev CPU memory map](https://www.nesdev.org/wiki/CPU_ALL), [NROM](https://www.nesdev.org/wiki/NROM), [CHR ROM vs RAM](https://www.nesdev.org/wiki/CHR-ROM_vs_CHR-RAM), and the actual NESEmulator cartridge/bus implementation. No complete emulator, production mapper support or full NES hardware behavior is certified.

### Apply both additive sections on your Mac (no book/PDF generation)

```bash
cd ~/Desktop/Programming/Project_008_AllMight
git fetch origin
git switch curriculum/cpp23-nes-project-pass01
git pull --ff-only origin curriculum/cpp23-nes-project-pass01

python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/apply_cpp23_project_inserts.py --apply
python3 tools/apply_cpp23_project_inserts.py --check

git diff --check
git diff --numstat -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
git diff -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
```

Both inserts are idempotent. If the previous section has been changed manually after insertion, the script refuses to overwrite those changes; reconcile them intentionally.

### Build only the C++23 exercises on your Mac (not LaTeX)

```bash
brew install cmake ninja llvm
cmake -S "Book Production/Seven-Book Curriculum/Book01" \\
      -B build/allmight-nes-course -G Ninja \\
      -DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++" \\
      -DCMAKE_BUILD_TYPE=Debug
cmake --build build/allmight-nes-course --parallel
ctest --test-dir build/allmight-nes-course --output-on-failure
python3 -m unittest discover -s tools/tests -p 'test_cpp23_project_inserts.py'
```

No LaTeX compilations, PDF builds, export pipeline changes or generated chapter copies were performed. Final TeX rebuilds remain user-controlled and must use the actual book master and existing build commands after the canonical source has been reviewed.


## Pass 03 — Book 01, Chapter 03: Records as Resource Transactions

**Editing rule:** the existing 1,892-line derived chapter is not replaced. The additive prose is attached to the original `## Failure guarantees shape the transfer algorithm` discussion, just before the canonical heading `## Let the compiler compose the operations`. It extends the actual long manuscript **after user-side insertion**, which is an explicit operation on the user's Mac; no LaTeX or PDF compilation occurs here.

**New work:** [NES-02T transactional lesson](../../Supplementary/C++23%20Project%20Labs/NES-02T%20-%20Transactional%20Cartridge%20Replacement.md) and [C++23 lab](../../tools/labs/nes_cartridge_transaction_cpp23_lab.cpp), with the [NROM cartridge/bus core](../../tools/labs/nes_course/nes_bus_nrom.hpp) now shared by NES-02 and NES-02T. A cartridge load stages a complete owned candidate, verifies invariants, then commits by nothrow move and increments a revision. Expected file errors and injected `std::bad_alloc` do not replace the old cartridge. This is a teaching-level, single-threaded guarantee and does not imply concurrent publication or GPU retirement.

The write deliberately distinguishes `std::optional::emplace`'s **general destruction-before-construction rule** from the **concrete** nothrow move path that makes publication safe in this implementation. Source: C++23 [N4950 draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4950.pdf), [cppreference optional::emplace](https://en.cppreference.com/w/cpp/utility/optional/emplace). These language guarantees are distinct from software test evidence.

**Test status:** the shared bus refactor and a local reconstruction of the NES-02T transactional implementation compiled under GCC 14 and Clang 17 in C++23 mode with strict warnings. Local AddressSanitizer/UndefinedBehaviorSanitizer execution passed, as did a three-test CMake/CTest run (NES-01, NES-02, NES-02T). The exact committed NES-02T file and new insertion-fixture revisions still require an independent build/test on the user's Mac. Do not claim native macOS or final book verification from the local Linux reconstruction.

### Apply all existing additions without deleting canonical text

```bash
cd ~/Desktop/Programming/Project_008_AllMight
git fetch origin
git switch curriculum/cpp23-nes-project-pass01
git pull --ff-only origin curriculum/cpp23-nes-project-pass01

python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/apply_cpp23_project_inserts.py --apply
python3 tools/apply_cpp23_project_inserts.py --check

git diff --check
git diff --numstat -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
git diff -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
```

The patcher knows the previous NES-01/NES-02 markers. It cannot silently overwrite a locally edited or partially inserted lesson, and it verifies byte-preservation for each new block before writing. It does not operate on generated Chapter Edition files.

### Compile C++23 labs only, with no LaTeX or PDFs

```bash
brew install cmake ninja llvm
cmake -S "Book Production/Seven-Book Curriculum/Book01" \\
  -B build/allmight-nes-course -G Ninja \\
  -DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++" \\
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/allmight-nes-course --parallel
ctest --test-dir build/allmight-nes-course --output-on-failure
python3 -m unittest discover -s tools/tests -p 'test_cpp23_project_inserts.py'
```

This builds NES-01, NES-02 and NES-02T, not KairoNES's GUI or the seven-volume LaTeX book. No completed seven-book edition is claimed.


## Pass 04 — Book 01, Chapter 04: prove ownership against the same NES runtime

**Original source preservation:** the already substantial chapter "Prove C++ Ownership with Runnable Experiments" (~1,603 reading-copy lines) remains intact. This is a surgical, long-form addition placed before the existing heading `## From one owned resource to many requests and a replaceable backend`, not a shorter chapter generated from scratch. The canonical `Continuous Notes/` manuscript remains unchanged on GitHub until the user explicitly applies the addition on the Mac.

**New work:** [NES-02P long-form teaching section](../../Supplementary/C++23%20Project%20Labs/NES-02P%20-%20Ownership%20Proofs%20for%20an%20Emulator.md) (~18,124 characters) and [C++23 ownership proof suite](../../tools/labs/nes_ownership_proofs_cpp23_lab.cpp) (~315 source lines). The suite imports the very same [transaction runtime header](../../tools/labs/nes_course/cartridge_runtime.hpp) used in the previous chapter, rather than a reconstructed duplicate. It proves five independent behaviors: partial-construction cleanup order, `std::move` as an xvalue cast versus actual transfer, `std::optional::emplace` destruction-before-construction on failure, cartridge independence from a destroyed external byte buffer with failed-load preservation, and `std::weak_ptr` observation without keeping an object alive. The original chapter's extensive prior laboratories remain available.

**Test wiring:** four C++23 executables are built through `Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt`. A focused [GitHub Actions CI workflow](../../.github/workflows/allmight-cpp23-nes-course.yml) builds the C++23 exercises on Ubuntu GCC and Clang and verifies the byte-preserving manuscript insertion fixtures. **Do not claim CI success, native Mac success or complete NES correctness until these outputs are inspected.**

**On the Mac, after pulling this branch:**

```bash
cd ~/Desktop/Programming/Project_008_AllMight
git fetch origin
git switch curriculum/cpp23-nes-project-pass01
git pull --ff-only origin curriculum/cpp23-nes-project-pass01

# Read-only preflight, then a byte-preserving edit of the original book.
python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/apply_cpp23_project_inserts.py --apply
python3 tools/apply_cpp23_project_inserts.py --check

# Original source must show additions, never deletion or large rewrites.
git diff --check
git diff --numstat -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
git diff -- "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"

# Only C++23 tests: never LaTeX, InDesign, PDF or book regeneration.
brew install cmake ninja llvm
cmake -S "Book Production/Seven-Book Curriculum/Book01" \\
  -B build/allmight-nes-course -G Ninja \\
  -DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++" \\
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/allmight-nes-course --parallel
ctest --test-dir build/allmight-nes-course --output-on-failure
python3 -m unittest discover -s tools/tests -p 'test_cpp23_project_inserts.py'
```

The insertion tool recognizes existing untouched NES-01, NES-02 and NES-02T blocks and adds only missing NES-02P. It refuses to overwrite a modified previous addition or to insert at an ambiguous heading. It never compiles LaTeX.

**Next learning stage:** Book 01 Chapter 05, generic asset pipelines; reuse these ownership proofs across cartridge, texture and mesh import data. The learner's primary implementation remains C++23, with every other language's interoperability boundary explicitly justified.


## Pass 05 — Book 01, Chapter 05: Generic Asset Pipelines without Hidden Contracts

**Delivery model changed per user instruction:** continue developing consecutive chapter passes on the same `curriculum/cpp23-nes-project-pass01` branch. Do not require the user to pull, apply inserts or run commands between passes. Reserve a consolidated pull/review/test/application plan for the **end-of-day or end-of-book checkpoint** when the user requests it. Nothing in this section asks for immediate local action.

**Preservation:** the existing roughly 1,516-line derived chapter already teaches class templates, explicit/partial specialization, forwarding, concepts, parameter packs, byte budgets, compile-time policies, value categories, std::variant tradeoffs and C++ performance considerations. All its content is preserved. The new project addition belongs before the original heading `### The real performance question is what gets instantiated, stored and moved`, after the publication-correctness discussion. It is staged as a fifth marker-delimited insert, not an abbreviated replacement manuscript.

**New narrative:** [NES-05A — Type-Safe Asset Import and Publication](../../Supplementary/C++23%20Project%20Labs/NES-05A%20-%20Type-Safe%20Asset%20Import%20and%20Publication.md) (25,513 characters). Follows one end-to-end type-aware import story, from external bytes to three different validated owned C++23 types and their transactional publication.

**Cumulative code:** [typed_asset_pipeline.hpp](../../tools/labs/nes_course/typed_asset_pipeline.hpp) and [typed_asset_pipeline_cpp23_lab.cpp](../../tools/labs/typed_asset_pipeline_cpp23_lab.cpp). The existing C++23 `NromCartridge` is a real variant alternative, alongside the deliberately educational `TXR4` (RGBA8) and `MSH1` (quantized 16-bit triangle mesh) formats. Validation includes byte offsets, little-endian decoding, checked lengths, resource budget, triangle-index bounds, per-domain errors, independent owned storage, staging failures and append-only index lookup across vector relocation.

**Integrity and evidence:** the source-preserving `tools/apply_cpp23_project_inserts.py` now handles five idempotent section additions. Test fixtures check exact recovery of original bytes after stripping these additions and fail closed on modified existing sections. CMake/CTest now registers five cumulative C++23 executables; the GitHub Actions workflow monitors the relevant source paths. **No new successful CI/macOS compilation is asserted until a job result is actually observed.**

**Important limitations:** the index token is not a generational handle, not globally unique and not a concurrency-safe pointer. It is valid only for an append-only store whose lifetime continues. A single successful import remains committed even if a subsequent independent import fails: this is *not* an all-or-nothing batch. CPU image and mesh preparation is *not* a GPU upload. These are intentional teaching boundaries for Chapters 06 and the later graphics books.

**Next:** Chapter 06, "Grow Registries without Invalidating Identity", will grow this exact store into owner-aware generational handles, explicit release/reuse policy, handle invalidation tests and a clear distinction between stable logical identity and movable physical storage.


## Pass 06 — Book 01, Chapter 06: Grow Registries without Invalidating Identity

**Workflow:** the user will not pull or run any commands yet. All six authoring passes accumulate on the existing `curriculum/cpp23-nes-project-pass01` branch. Do **not** present another per-pass command block. Final pull, canonical insertion, CMake/CTest, manuscript diff and LaTeX authoring handoff remain deferred until the user asks for an end-of-day or end-of-book review.

**Preserved original chapter:** "Grow Registries without Invalidating Identity" (~1,331 lines in the derived chapter copy). Its present M015 generational registry example, vector growth and invalidation reasoning, name-index transaction, free-list rules and container comparisons remain untouched. Pass 06 adds a *project-specific strengthening* of these existing ideas, not a replacement or claim that the material was absent.

**New additive narrative:** [NES-06G — Generational Identity and Safe Reuse](../../Supplementary/C++23%20Project%20Labs/NES-06G%20-%20Generational%20Identity%20and%20Safe%20Reuse.md) (27,993 characters), staged immediately before the existing heading "From a living asset registry to reliable queries and transformations".

**Shared code and tests:** [generational_asset_registry.hpp](../../tools/labs/nes_course/generational_asset_registry.hpp) uses the **exact existing** cartridge/texture/mesh `Spec<Tag>` decoders and `AssetRecord` variant. [generational_asset_registry_cpp23_lab.cpp](../../tools/labs/generational_asset_registry_cpp23_lab.cpp) exercises owner-scoped handle identity, runtime type checks despite compile-time tag types, vector relocation, slot reuse, stale generation rejection, typed errors, registry move semantics, near-overflow permanent retirement and 2,000 deterministic reuse cycles. It preserves Stage-05 import error boundaries and data ownership.

**Safety and limitations:** handle identity includes `owner + slot + generation + type`. Owner tokens are unique only in the current process execution, not persistent IDs or security capabilities. Borrowed C++ pointers and references are never safe merely because a logical handle stays valid. The registry is deliberately single-threaded; its atomic token issuer protects only construction of registry owner namespaces. It is not a concurrent GPU asset registry.

**Evidence:** the CMake/CTest project now registers six cumulative C++23 targets and GitHub Actions monitors the source paths. `tools/apply_cpp23_project_inserts.py` stages six exact idempotent additions; the fixture tests prove byte-exact recovery after removing all six blocks. **Compilation/CI run results for the newly committed Pass 06 code have not been verified**; do not call it fully tested until actual build output is available.

**Next:** Chapter 07, "Typed Interfaces, Identity, and Data Structures", with continued improvements to the same store and evidence-based choice of data structures.


## Pass 07 — Book 01, Chapter 07: Typed Interfaces, Identity, and Data Structures

**User workflow remains deferred.** Continue source additions on the same `curriculum/cpp23-nes-project-pass01` branch. The user does not need to pull, apply, compile, export or run commands after each pass. Consolidated review, byte-preserving canonical insertion, native Mac compilation, sanitizer checks and LaTeX/PDF production are deferred until the user requests an end-of-day or end-of-book checkpoint.

**Original long chapter preserved:** the existing chapter "Typed Interfaces, Identity, and Data Structures" (~877 source-edition lines and ~128K characters) already includes detailed PIMPL, phantom types, matrix/expression-template lifetimes, intrusive ownership, CPU/cache/SIMD, allocation/concurrency, recurrences, arrays/queues, hashing, Bloom filters, heaps and union-find. No original paragraph has been deleted, summarized away or directly rewritten on GitHub. The new additive lesson attaches just before `## The scene is now too large for the wrong container — Phase 5, M025–M027, Parts 401–445` so the existing CPU narrative leads into an executable instruction/debugger, then back into the original DSA lessons.

**Deep teaching addition:** [NES-07C — CPU Instructions and Typed Debugger Structures](../../Supplementary/C++23%20Project%20Labs/NES-07C%20-%20CPU%20Instructions%20and%20Typed%20Debugger%20Structures.md) (~35.6K characters). It derives three address domains, real NROM PRG mapping, the little-endian reset vector, binary ADC flags, signed relative branch offsets, branch-page cost accounting and the difference between an architectural instruction transition and a real hardware bus-cycle trace. It then integrates a genuinely separately compiled PIMPL, owned debugger snapshots, fixed lossless trace backpressure, ordered breakpoints, priority memory probes, Bloom negative filtering and DSU analysis of RAM mirrors.

**Cumulative code:**
- [cpu6502_subset.hpp](../../tools/labs/nes_course/cpu6502_subset.hpp) — a deliberately partial instruction executor for opcode bytes EA, A9, A2, 85, E8, 18, 38, 69, D0, F0 and 4C, reusing the actual `CpuBus` and `NromCartridge` from NES-02.
- [cpu_debug_instruments.hpp](../../tools/labs/nes_course/cpu_debug_instruments.hpp) — ring, ordered breakpoints, visited PC filter, priority probe scheduler and disjoint-set.
- [cpu_debug_session.hpp](../../tools/labs/nes_course/cpu_debug_session.hpp) + [cpu_debug_session.cpp](../../tools/labs/nes_course/cpu_debug_session.cpp) — real C++23 PIMPL with out-of-line destructor/move assignment and typed, owned snapshots.
- [nes_cpu_debugger_cpp23_lab.cpp](../../tools/labs/nes_cpu_debugger_cpp23_lab.cpp) — executable synthetic iNES/NROM-128/256 CPU trace, flag, branch, timing, reset failure, unsupported BRK, PPU MMIO fetch, FIFO, breakpoint, heap and DSU regression oracles.

**Safety boundary:** this is neither a complete MOS 6502 nor a complete Ricoh 2A03/NES. The numeric per-instruction cost counter does not simulate every cycle's real bus actions. No interrupts/stack, full opcode matrix, decimal-mode instructions, PPU, APU, DMA, audio/video, controller I/O or accurate frame synchronization are modeled. The debug structures and shared bus are single-threaded. A Bloom positive result may be a false positive. The public C++ PIMPL is not a portable cross-toolchain plugin ABI.

**Verification boundary:** the Book01 CMake/CTest now contains **seven targets**, and the seventh compiles the debugger client and the private implementation from separate translation units. The original additive patcher handles seven marker-delimited sections, with byte-exact restoration/idempotence fixtures extended accordingly. A dedicated GitHub Actions workflow tracks these paths. **Do not claim a passing native Mac build, complete CI, hardware differential validation or sanitizer certification until the actual outputs are inspected.** No LaTeX or PDF work was run here.

**Next:** Chapter 08, "Graphs, Search, and GPU Scheduling". The next curriculum pass should derive a graph of real resource and debugger workload dependencies rather than using disconnected graph snippets. Keep CPU instruction order, host task scheduling and eventual Vulkan/Metal GPU synchronization as *distinct semantics*.


## Pass 08 — Book 01, Chapter 08: Graphs, Search, and GPU Scheduling

**User handoff:** do not request pulls, edits, compilation commands or LaTeX work now. All passes continue accumulating on `curriculum/cpp23-nes-project-pass01`, with one later consolidated source insertion, C++23 audit and native Mac verification when the user chooses a checkpoint.

**Original content preserved:** The derived "Graphs, Search, and GPU Scheduling" chapter already has ~2,263 lines / 89.7K characters of BFS, DFS, topological sorting, shortest-path, A*, SCC, scan, API, GPU synchronization, shader layout and kernel material. None has been deleted or replaced. The new project lesson is staged at the graph-to-API transition immediately preceding `## An API is a promise about an operation` in the original continuous manuscript.

**Deep integrated lesson:** [NES-08G — Dependency Graphs and Explicit GPU Work Contracts](../../Supplementary/C++23%20Project%20Labs/NES-08G%20-%20Dependency%20Graphs%20and%20Explicit%20GPU%20Work%20Contracts.md) (~35,243 chars). Real NES/CPU execution motivates explicit DAG prerequisites, WAR/WAW/RAW hazards, deterministic topological planning, critical paths, CPU-prefix-scan resource partitioning and the boundary where a render graph must become actual synchronized device work. Primary Khronos Vulkan and Apple Metal/Metal4 docs underpin the distinction; no GPU runtime is claimed.

**Cumulative code:** [work_dependency_graph.hpp](../../tools/labs/nes_course/work_dependency_graph.hpp) exposes TaskId, Task/Domain/ResourceKey (including generation-aware identity), typed graph errors, explicit resource access tracking, deduplicated hazard edges, stable Kahn topological planning, earliest start/finish + critical-path estimates, GPU synchronization review edges and overflow-safe CPU exclusive scan. [nes_work_dependency_graph_cpp23_lab.cpp](../../tools/labs/nes_work_dependency_graph_cpp23_lab.cpp) imports NROM and RGBA8 through the existing Pass05/06 system, executes LDA/STA tasks through the actual separately compiled Pass07 CPU debugger, and validates the CPU RAM mirror. It generates and verifies 1,800 task declarations across 30 fixed-seed DAGs. GPU transfer/compute/graphics tasks are inspected as PLANS ONLY.

**Limits and evidence:** The CMake project now has eight cumulative C++23 targets. The seventh and eighth both link the same `cpu_debug_session.cpp` PIMPL implementation. `tools/apply_cpp23_project_inserts.py` stages eight marker-delimited additive chapters and `tools/tests/test_cpp23_project_inserts.py` checks recovery of all original fixture bytes and idempotence. A GitHub Actions workflow is configured to monitor changes. **Successful build/CTest/GPU results have not yet been observed; do not falsely report them passed.**

**Not implemented:** Native Vulkan/Metal barriers, Metal4 synchronization, GPU command buffers, real transfer/compute queue ownership, image layout transitions, in-flight resource retirement, full NES timings/PPU/APU, measured GPU/CPU performance, and currently checked Kairo/Maveb backend integration.

**Next:** Chapter 09 "GPU Memory, Divergence, Atomics, and Ordering" will use this host dependency model to derive device memory ranges, visibility/ordering contracts and per-backend verifiable experiments, with source preserved and no premature performance/correctness claims.


## Pass 09 — Book 01, Chapter 09: GPU Memory, Divergence, Atomics, and Ordering

**No interim user action.** Continue the chapter-by-chapter source work on `curriculum/cpp23-nes-project-pass01`. The user will pull, run the byte-preserving source insertion and complete native compiler/CTest/LaTeX checks only at their requested end-of-day or end-of-book checkpoint. Do not issue per-pass Mac commands or shorten/rebuild the original book.

**Existing chapter:** "GPU Memory, Divergence, Atomics, and Ordering" has ~3,356 lines of GPU memory hierarchy, shared/unified memory, memory coalescing, SIMT divergence, subgroup operations, atomics/order, barriers, multi-dispatch scans, GPU compute, stable sorting and resource-management material. It remains unmodified in the original source. Pass 09 adds a research-level, application-oriented continuation immediately before the original `### GENERAL COMPUTE & PARALLEL ALGORITHMS` heading.

**Long-form addition:** [NES-09M — GPU Memory and Ordering Contracts](../../Supplementary/C++23%20Project%20Labs/NES-09M%20-%20GPU%20Memory%20and%20Ordering%20Contracts.md) (~37.3K chars) bridges the existing typed TXR4 texture, generational registry and Pass08 CPU work graph with allocation alignment, initialized ranges, logical byte visibility, execution versus memory dependencies, conceptual resource ownership/layout, device completion-based retirement, coalescing proxy tests and workgroup barrier participation. Derives C++ host release/acquire publication and contrasts it with the distinct Vulkan/Metal shader/device memory models.

**Shared reference implementation:** [gpu_memory_contract_oracle.hpp](../../tools/labs/nes_course/gpu_memory_contract_oracle.hpp). A **CPU-only** monotonic virtual byte-allocation model with overflow-safe power-of-two alignment, typed failures, versioned initialization and visibility, independent execution-only role transition and memory dependency, byte-exact source transfer, conceptual compute/graphics reads, simulated in-flight ticket retirement, permanent tombstones, hypothetical aligned-segment counts and barrier-participation classification.

**Test executable:** [nes_gpu_memory_contract_cpp23_lab.cpp](../../tools/labs/nes_gpu_memory_contract_cpp23_lab.cpp). Uses real Pass05/06 RGBA8 imports, stage-to-stage work plan from Pass08, wrong-role/wrong-layout/uninitialized/invisible-byte tests, exact byte transformations, stale generational handle and pending completion refusal, and **512 actual host `std::atomic` release/acquire exchanges** between two C++ threads. A valid host-thread message protocol is not evidence of native GPU synchronization. The CMake target links `Threads::Threads`.

**Verification:** The Book01 CMake/CTest has **nine** cumulative C++23 targets. The strict byte-preservation insert script stages nine marker-delimited additions and fixture tests now include all nine in exact recovery, idempotence and conflict checks. The original canonical manuscript has not been edited; this branch contains the staged source and code. **The exact Pass09 build and complete cumulative tests have not been verified as passing**; no native Vulkan/Metal device code exists in this milestone.

**Limitations:** The C++ oracle does not create real GPU images/buffers, issue fences/events, perform queue-family ownership transfer, model subresource layouts, check GPU memory residency, recycle transient physical allocations or measure device behavior. Any native backend work must consult current Khronos synchronization requirements and Apple Metal/Metal4 queue/barrier guidance and validate against the actual Kairo/Maveb revision. Real GPU workgroups and lanes are NOT simulated by the participation/segment helper.

**Next:** Chapter 10, "Reproducible Builds, Profiling, and Repository Contracts": turn the growing source into reproducible, independently audited C++23 builds and reported results, preserving the user's no-command-until-checkpoint workflow.


## Pass 10 — Book 01, Chapter 10: Reproducible Builds, Profiling, and Repository Contracts

**The user's deferred workflow remains unchanged.** Do not request interim `git pull`, local compilation, manuscript application or LaTeX commands. All code and additive lessons accumulate on `curriculum/cpp23-nes-project-pass01`. When the user requests a final checkpoint, review the entire source diff, apply additions through the exact fail-closed tool and execute/inspect all compiler and CTest results.

**Preserved chapter:** the existing reading-copy chapter has ~4,037 lines of substantial build systems, CMake targets and modules, repository pinning, CI, sanitizers, diagnostics, profiling and resource transactions. No original chapter content was truncated or rewritten. The Pass10 project lesson is staged immediately before the original `### REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW` section.

**New narrative:** [NES-10R — Reproducibility and Evidence-Driven Profiling](../../Supplementary/C++23%20Project%20Labs/NES-10R%20-%20Reproducibility%20and%20Evidence-Driven%20Profiling.md) (~25K characters). Traces the exact existing NROM/CPU/typed registry/task graph system into a declared C++23 build, independent source/toolchain evidence, JUnit correctness and optional actual timing rather than using a disconnected build-system example.

**C++23 measurement:** [measurement_protocol.hpp](../../tools/labs/nes_course/measurement_protocol.hpp) provides bounded trials, warmups and iterations, work-result checksums, steady-clock durations, nearest-rank median and p90. [nes_reproducible_benchmark_cpp23_lab.cpp](../../tools/labs/nes_reproducible_benchmark_cpp23_lab.cpp) verifies prior real NROM import, 6502 CPU instruction results, 32-task DAG and 256-count scan, with `--benchmark` an optional, measured CSV mode, **not an automatically passing speed threshold**.

**Build contracts:** [Book01/CMakeLists.txt](Book01/CMakeLists.txt) now has **ten** cumulative C++23 executable/CTest targets and a public `nes_course_public_api` INTERFACE target expressing the include root and language requirements. [Book01/CMakePresets.json](Book01/CMakePresets.json) provides isolated Debug Ninja, Release Ninja and Clang ASan/UBSan build configurations. Compiler versions/SDKs are not magically pinned by preset names.

**Fail-closed evidence:** [tools/nes_course_evidence.py](../../tools/nes_course_evidence.py) separates a *source-only inventory audit* from an observed JUnit CTest receipt. It validates names and outcomes for all ten tests, CMake configuration, executable existence, freshness of test XML relative to source/build/binaries, source-tree cleanliness and source SHA-256 against an exact Git commit. It rejects missing, failed, skipped or stale evidence; old test results cannot be silently promoted to current-source certificates. [test_nes_course_evidence.py](../../tools/tests/test_nes_course_evidence.py) covers these negative cases with private test-only fixtures.

**CI and preservation:** GitHub Actions is configured for Ubuntu GCC, Ubuntu Clang, macOS AppleClang and Ubuntu ASan/UBSan, and configured to collect test JUnit/source-pinned provenance artifacts. This is **configured workflow, not proof the jobs executed successfully**. Manuscript insertion remains dry-run-only in CI. [apply_cpp23_project_inserts.py](../../tools/apply_cpp23_project_inserts.py) stages ten unique byte-preserving insertions; its fixture tests verify ten-block exact recovery, idempotence and hard conflicts.

**Hard boundaries:** No numerical benchmark was run or invented, no GPU device was created, no actual Metal/Vulkan correctness was inferred from CPU-only code, and no complete emulator or seven-book mastery certification is claimed. CMake/CTest execution results for the new cumulative tree still require direct observation.

**Next:** Chapter 11 "C++ Const, Borrowing, and CPU–GPU Buffering". Use the same NROM/PIMPL/asset/GPU-memory teaching system to derive immutable access authority, owned versus borrowed views and multi-buffering without abandoning the original manuscript.


## Pass 11 — Book 01, Chapter 11: C++ Const, Borrowing, and CPU–GPU Buffering

**No per-pass user actions.** This additive revision is committed to `curriculum/cpp23-nes-project-pass01`; the user's one-shot pull, exact manuscript insertion, native Mac build and LaTeX work remain deferred to an explicit checkpoint. Do not shorten or regenerate the original 5,000+ page curriculum.

**Existing chapter preserved:** The original Chapter 11 reading copy contains ~2,378 lines / 94.97K characters already covering const qualification, borrow interval proofs, generation lifetime, GPU-generated work, frame slots, ping-pong and triple buffering, Metal/Vulkan execution, presentation and multi-queue lifetime. None was replaced. The ~43.25K-character new long-form lesson [NES-11B — Scoped Borrows and CPU-GPU Frame Buffering](../../Supplementary/C++23%20Project%20Labs/NES-11B%20-%20Scoped%20Borrows%20and%20CPU-GPU%20Frame%20Buffering.md) is staged immediately before `## From generated GPU work to safe explicit execution`, where C++ borrowing turns into real graphics resource lifetimes.

**Cumulative C++23 code:** [scoped_frame_upload_ring.hpp](../../tools/labs/nes_course/scoped_frame_upload_ring.hpp) defines single-threaded `FrameUploadRing<Capacity>` with an owned heap control block, explicitly immutable `std::span<const Byte>` ReadLease pinned by RAII, move-only transactional WriteLease and rollback, per-pool owner/slot/generation identity and no ABA wrap, three independent simulated queue completion values, CPU pin refusal, permanent maximum-generation slot retirement and independent owned snapshot copying.

**Actual project integration:** [nes_scoped_frame_upload_cpp23_lab.cpp](../../tools/labs/nes_scoped_frame_upload_cpp23_lab.cpp) imports a TXR4 RGBA8 image through Pass05/06 typed decoding, executes LDA/STA on the real earlier NROM CpuBus/Pass07 separately compiled PIMPL debugger, and serializes a 16-byte NESF frame/CPU state header with eight actual texture pixels. It tests three in-flight frames, real data deep-copy isolation, queue completion componentwise checks, immutable views, reader pin lifetime, abandoned writer recovery, slot reuse/generation increment, foreign/forged handles, generation overflow retirement and an independent swapchain acquisition-index example.

**Build and provenance:** Book01 CMake/CTest now registers **11** cumulative C++23 executable targets; Pass11 links `cpu_debug_session.cpp` in its executable, reusing the actual prior debugger. The staged insertion tool, fixture restoration tests, source-manifest evidence collector and manifest unit tests were extended to the eleventh lesson, target and shared header. The existing GCC, Clang, macOS AppleClang and Linux sanitizer CI definitions cover the new source paths. A passing CI status is **not asserted until actual runner results and receipts are inspected**.

**Limits:** The ring models no real GPU device, buffer mapping, command buffer submission, semaphore, queue-family transfer, presentation, resource residency, display output or device lost recovery. Completion values are **synthetic caller-provided observations**, not native timestamps. All slot operations are single-threaded. A copied raw `span<const Byte>` can outlive its ReadLease if misused; the contract forbids that and provides `copy_snapshot()` for an independent value. The host model is not a concurrency safety proof and should not be called a production Vulkan/Metal upload implementation.

**Next:** Chapter 12, "Commands, Async Work, and Resource Lifetimes": retain the same ring and add a tested command-state and submission/cancellation lifecycle, with explicit native backend verification boundaries.


## Pass 12 — Book 01, Chapter 12: Commands, Async Work, and Resource Lifetimes

**Authoring/command policy:** Continue staged additive manuscript and C++23 work on `curriculum/cpp23-nes-project-pass01`. The user asked for a single consolidated Mac pull, canonical source insertion and native build check at a later requested checkpoint; do not issue intermediate commands, rewrite all pages or compile LaTeX.

**Original material preserved:** Chapter 12's reading copy contains ~3,198 lines / 50.9K characters of established command processor, async compute, upload ring, GPU command and synchronization, GPU profiler and performance methodology. The new long-form [NES-12Q — Command Lifecycles and Uncertain Submission](../../Supplementary/C++23%20Project%20Labs/NES-12Q%20-%20Command%20Lifecycles%20and%20Uncertain%20Submission.md) (~47.9K characters) is staged immediately before the original `### PERFORMANCE MODELING & PRINCIPLED GPU REDESIGN` heading, after the original device submission discussion; no prior material is removed.

**C++23 shared component:** [command_submission_lifecycle.hpp](../../tools/labs/nes_course/command_submission_lifecycle.hpp) integrates the actual Pass08 `DependencyGraph` and Pass11 `FrameUploadRing<Capacity>`. A move-only CommandStream records GPU intents, returns exact graph diagnostics, seals a DAG before publication, and rejects re-recording while queued. The simulated CommandSubmissionQueue supports a definite **pre-submit** rejection without publishing a frame; accepted or uncertain outcomes publish a ring frame and retain the command-control lifetime. Uncertain results stay quarantined until independently accepted; device lost prevents invented recovery. Event records are owned and are issued only once when *modeled* queue stamps reach completion. A prepublication pool-owner guard rejects foreign frame writers before they can create orphaned ring handles. The actual ring performs retirement after all queue requirements and CPU borrow pins are satisfied. Retired journal entries can be pruned without reusing submission tickets.

**Integrated executable:** [nes_command_lifecycle_cpp23_lab.cpp](../../tools/labs/nes_command_lifecycle_cpp23_lab.cpp) contains ~483 lines of typed fail-closed tests for empty/duplicate/cyclic/missing task graph commands, definite rejection and retry, invalid queue stamps, unknown submission quarantine, device loss, at-most-once completion notices, command wrapper destruction with owned queue state, bounded journal capacity, pinned CPU readers and exact frame retirement. It runs the real previously implemented NROM CPU/PIMPL debugger and TXR4 texture registry and serializes the actual 24-byte NESF overlay before staging it in the earlier frame ring.

**Cumulative wiring:** Book01 CMake/CTest has **12** C++23 targets, including the Pass12 executable and the actual `cpu_debug_session.cpp` translation unit. The staged insertion utility has **12** marker-delimited original-source-preserving sections. Its Python fixture verifies 12-block idempotence, conflict refusal and restoration of every original byte. The source-pinned evidence manifest and negative fixture tests now require 12 labs. Existing GCC/Clang/AppleClang/sanitizer CI definitions cover the new files.

**Evidence limits:** The command journal and device stamps are **single-threaded CPU reference models**, not actual Vulkan/Metal command buffers, synchronization primitives, command callbacks, GPU queues, device loss, swapchain synchronization or verified render results. The model's supplied queue decisions and counter values are not driver observations. Neither the exact 12-target build, end-to-end CI, native Mac compilation nor any GPU performance result is certified until actual runner evidence is observed.

**Next:** Chapter 13, "Open Vulkan Devices and Describe Resources": learn and test native Vulkan loader/adapter/feature and resource contracts as the first concrete device-backed bridge, with fallback behavior when a Vulkan runtime/device is absent and no invented hardware capabilities.


## Editorial acceptance rule for each subsequent chapter

1. State the practical problem and derive what must be represented; define notation at its first use.
2. Explain the mechanism in complete, connected paragraphs; derive mathematics, trace algorithm states, expose alternatives and failure cases.
3. Produce a reproducible C++23 implementation with detailed source comments, CMake instructions, deterministic positive and negative tests, and platform qualifications.
4. Attach that chapter to a persistent project milestone. New concepts should alter or validate a real subsystem, not produce disposable demo snippets.
5. Reconcile relevant book/specification/lecture/research sources by exact unit and source version; don't claim source exhaustion from indexes.
6. Verify compile, run, math, cross-references and LaTeX output; mark what remains unevaluated.
7. Keep generated chapter-edition files generated from the canonical continuous manuscripts; never hand-edit generated chapter bodies and call them authoritative.

## What is not yet complete

The seven-stage chapter-to-book remapping, all source-unit reconciliation, LaTeX integration into a finished seven-book/14-part output, independent research correctness audit, and native-platform verification of the existing emulator are **open**. This first pass contributes a real teaching section and executable witness, not a false completion certificate.
