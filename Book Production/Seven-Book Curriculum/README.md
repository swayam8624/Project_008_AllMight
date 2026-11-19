# Project AllMight — Seven-book C++23 curriculum (in progress)

## Book 1 foundations authoring checkpoint — ready for the user's four-part LaTeX build

The **first sixteen original Computing Foundations reading chapters** are now
releasable as a self-contained, source-preserving expanded reader with all
**eighteen authored C++23 project lessons** inserted at verified original
teaching anchors and a separate long-form, four-route, final mastery chapter.
This is **Book01's foundations-curriculum checkpoint**, not seven finished
books and not a claim that the older separately routed seven-stage print
generation has been regenerated.

The source-only assembly and Pandoc conversion passed in
[expanded reader CI run 44487625396](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44487625396);
the prior complete portable C++23 suite passed GCC, Clang19, AppleClang
and sanitizer validation in
[run 44476138886](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44476138886).
The reader never discards or replaces a canonical master paragraph; its
reversal proof detects every exact marked addition. The user-side local
PDF compilation remains deliberately separate and has **not** been run by
the assistant; native GPU experimental proof is not implied by a CPU lab.

**For the exact checkout, optional original-master insertion, CMake/CTest,
Pandoc and Mac LuaLaTeX build, use the dedicated [Book01 release guide](Book01/BOOK01_RELEASE_CHECKPOINT.md).**
The executable builder is [tools/build_book01_expanded_reader.py](../../tools/build_book01_expanded_reader.py).
It emits four updated LuaLaTeX reader parts under
`build/book01-expanded-reader/` and a SHA-256 source manifest. The older
`Book Production/AllMight Codex/LaTeX/Generations` snapshots are preserved,
not silently overwritten.


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


## Pass 13 — Book 01, Chapter 13: Open Vulkan Devices and Describe Resources

**Important change in project emphasis.** The NES emulator is one deliberately
small systems exercise, **not the full C++23 seven-book curriculum**. The
binding [multi-project teaching matrix](project-lab-matrix.md) now assigns
real teaching milestones to KairoRenderer/KairoGameEngine (Vulkan engine),
Maveb (Metal, Gaussian, TSDF, reconstruction), KairoPhysicsEngine/Kyoto
collision work (broad phase and numerical physics), KairoMath/KairoGeometry/
KairoRayTracer (numerics and graphics), Vulkax (scientific validation) and
XR/ILLIXR (vision and perceptual real-time systems). Reuse and inspect each
repo precisely when its topic belongs; do not invent cross-project tests.

**Original source intact.** Chapter 13 has ~4,556 lines of existing Vulkan
instance, queue, presentation, resource, memory, descriptor, pipeline and
shader explanations. No original source content was replaced. The new
~42,298-character [GPU-13D — Cross-Project Vulkan Devices and Resource
Contracts](../../Supplementary/C++23%20Project%20Labs/GPU-13D%20-%20Cross-Project%20Vulkan%20Devices%20and%20Resource%20Contracts.md)
is staged immediately before the original `### BUFFERS, IMAGES, MEMORY & UPLOADS`.

**New portable lab.** `tools/labs/gpu_course/device_resource_contracts.hpp`
and `tools/labs/gpu_device_resource_cpp23_lab.cpp` implement C++23 typed
device capability and resource matching for **Kairo graphics, Maveb
Gaussian compute, collision-pair buffers, XR gaze/render and NES overlay**.
All default tests use explicitly synthetic device snapshots and check queue
flags/counts, surface-specific unknown presentation support, device extension
gate, memoryTypeBits, required/preferred flags, heap index/size/budget,
alignment and typed rejection. These are *policy tests*, not GPU execution.

**Optional real API bridge.** `tools/labs/gpu_course/native_vulkan_probe.cpp`
is linked to the same target only when CMake locates a Vulkan SDK. Explicit
`--probe-native` queries real instance/physical device/queue/memory data,
creates a **headless VkDevice**, creates a small VkBuffer, queries actual
VkMemoryRequirements, allocates and binds compatible VkDeviceMemory and
releases resources through correct RAII dependency order. Without SDK or
compatible device it reports unavailable (77); **the regular CTest never
invokes this path**. No surface, swapchain, shader, draw, synchronization
or hardware benchmarking is claimed.

**Grounded source inspection.** Actual main-branch files inspected include
KairoRenderer `VulkanInstance.cppm` / `VulkanDevice.cppm` /
`VulkanBuffer.cppm` / `VulkanTexture.cppm` / `VulkanRenderer.cppm` /
`MetalRenderer.cppm`; Maveb `FrameContext.cpp` / `FrameContext.hpp` /
`Renderer.cpp` and `GaussianAsset.hpp`; KairoPhysicsEngine
`Broadphase.cppm` and KairoGPU `README.md`. Exact Git blobs are listed in
the [multi-project matrix](project-lab-matrix.md). The buffer-destructor
free-before-destroy ordering in Kairo is a **review item**, not an actual
modification of Kairo. The Kyoto and ILLIXR local source trees were not
retrieved in this pass.

**Cumulative build and source preservation.** Book01 now has **13** C++23
targets and CTest registrations, with the Pass13 Vulkan dependency **optional**.
The staged byte-preserving source patcher now registers **13** additions;
its fixture tests check whole-manuscript byte restoration, idempotence and
fail-closed conflicts. The CTest/JUnit evidence manifest includes the new
cross-project source and seven-book project matrix. Original manuscript stays
unchanged. **Actual compilation, all-pass CI, native Vulkan device creation
and GPU benchmark evidence have not been verified.**

**Future direction.** Pass14 must implement and verify real Vulkan command
submission/synchronization and retired bound resources, and compare against
the Kairo renderer's actual source and Maveb's native Metal alternative.
Future books will select the appropriate long-running project instead of
defaulting to the NES emulator.


## Pass 14 — Book 01, Chapter 14: Submit, Synchronize, and Retire GPU Work

**Project scope:** The primary executable/source architecture is **KairoRenderer's
real Vulkan command, fence and presentation path**, contrasted with **Maveb's
Metal FrameContext and completed-handler** lifecycle. These are stronger and
more authentic teaching examples than forcing an NES emulator into Vulkan WSI.
The separate [seven-book project matrix](project-lab-matrix.md) makes the
multi-project progression a durable requirement.

**Authoritative original chapter remains intact:** Chapter 14 has about 4,398
lines / 71.27K characters of existing command buffers, queue submissions,
fences, semaphores, synchronization, barriers, frames in flight, validation,
timestamps, device lost and MoltenVK material. The newly staged
[GPU-14F - Frame Submission Synchronization and Retirement](../../Supplementary/C++23%20Project%20Labs/GPU-14F%20-%20Frame%20Submission%20Synchronization%20and%20Retirement.md)
(~37.4K characters) is inserted *only* before the original
`### VULKAN MEMORY MODEL, BARRIERS & MULTI-FRAME LIFETIMES` heading; it
does not delete or regenerate the original manuscript.

**Portable executable:** `tools/labs/gpu_course/submission_retirement.hpp`
and `tools/labs/gpu_submit_retire_cpp23_lab.cpp` implement a single-threaded
C++23 CPU model and negative tests for frame-slot versus WSI-image index,
independent generations, Kairo fence reset before failed command submission,
explicit recovery, uncertain submission quarantine, graphics completion
separate from presentation completion, deferred resource retirement,
Maveb arena reuse, CPU pins and Kyoto/XR resource references. CPU model
events are synthetic and do **not** claim native Vulkan fence or WSI success.

**Actual native Vulkan path, separately opt-in:** When CMake detects a Vulkan
SDK, `tools/labs/gpu_course/native_vulkan_submit_probe.cpp` may be executed
with `gpu_submit_retire_cpp23_lab --probe-native`. It creates a real headless
VkDevice and transfer buffer, allocates/binds compatible host-visible coherent
memory, records a VkCmdFillBuffer and transfer-write-to-host-read barrier,
submits the primary command buffer via VkQueueSubmit, waits for an actual
VkFence, maps and compares 64 x 32-bit result words, and tears down resources
in dependency order. No graphics shader, surface/swapchain presentation,
synchronization2/Metal backend, or performance claim is inferred. Missing
Vulkan SDK/device returns **unavailable (77)**, not a passing GPU test.
The portable default CTest **never invokes** this opt-in native path.

**Source-level Kairo review item:** `KairoRenderer/VulkanRenderer.cppm`
currently resets `m_Sync.InFlight()` immediately after successful image
acquisition and **before** command recording and accepted submission.
If a later recording or known rejected submission error occurs, the next
frame's fence wait could block without a submission that will signal it.
This is a grounded *source-control-flow* concern, not a reproduced native
hang or an applied Kairo fix. `KairoRenderer/VulkanCommand.cppm` and
`VulkanSync.cppm` were inspected to verify command-pool and semaphore
ownership; `Maveb/engine/metal/src/Renderer.cpp` and `FrameContext.cpp`
were inspected for three-frame permits and completion/arena lifetimes.
Their exact Git blob SHAs are preserved in [project-lab-matrix.md](project-lab-matrix.md).
No external repo was edited.

**Cumulative authoring:** 14 registered C++23 CMake executable targets and
14 default CPU CTest invocations, 14 marker-delimited additive source
insertions, updated whole-manuscript restoration/idempotence/conflict tests,
and a strict source-pinned CTest/JUnit evidence manifest. The canonical master
is unchanged; no user commands, LaTeX compilation or original text removal.

**Evidence boundaries:** A committed test source is not proof it compiles
or passes. Actual full build, GitHub CI, native vkQueueSubmit/fence execution,
GPU bytes, validation logs, timing and physical device identity remain
**unverified** until observed evidence arrives.

**Next:** Chapter15 "Execute and Measure a Metal Frame" should make
Maveb/Kairo Metal the primary platform project, with correct Apple GPU
resource ownership and native shader/frame evidence, backed by portable
CPU oracles and platform-conditional tests rather than invented devices.


## Pass 15 — Book 01, Chapter 15: Execute and Measure a Metal Frame

**Deliberate multi-project continuation.** Maveb is the main native Metal
teaching engine, KairoRenderer's actual Objective-C++ Metal implementation
the comparative engine, KairoPhysics/Kyoto the exact-pair GPU-compute
correctness client, and XR the timing/latency client. NES remains useful for
earlier CPU representation topics but **is not** the sole project thread.
The durable [seven-book multi-project matrix](project-lab-matrix.md) records
source revisions and genuinely different project responsibilities.

**Original writing preserved:** The actual Chapter 15 reading copy has
3,061 lines / 47,241 characters already covering Metal's execution model,
render/compute/blit encoders, storage, tile memory, drawables, frames in
flight, heaps, native hazards, profiling and ObjC++/Metal-cpp lifetime.
No existing lines were removed or regenerated. The new ~43,585-character
[GPU-15M — Execute, Verify and Measure a Metal Frame](../../Supplementary/C++23%20Project%20Labs/GPU-15M%20-%20Execute%20Verify%20and%20Measure%20a%20Metal%20Frame.md)
is staged immediately before the original `### STORAGE MODES, TILE MEMORY,
DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION` heading.

**Portable C++23 reference model:** `tools/labs/gpu_course/metal_frame_contracts.hpp`
models Maveb-style three-frame permits, owner/slot/generation identity,
checked monotonic upload arenas, definite precommit cancellation,
indeterminate submission quarantine, device-lost refusal, terminal
completed versus error, valid/nonzero GPU timestamp intervals, and
format-aware BGRA8 capture row-stride compaction. A malformed capture
records a separate `capture_error` but still **releases the frame permit
when GPU terminal completion is known**. `gpu_metal_frame_cpp23_lab.cpp`
tests Maveb Gaussian, Kairo scene, Kyoto collision and XR gaze tags,
arena overflow, out-of-order terminal callbacks, stale identity, error
result handling, correct 256-byte-pitch capture, and positive/missing
timing evidence. Default CTest is an explicit **synthetic CPU oracle**.

**Actual native Metal possibility:** `tools/labs/gpu_course/native_metal_compute_probe.mm`
uses Apple Objective-C++ ARC and actual Metal/Foundation frameworks, only
when the CMake compiler/framework checks pass. On explicit `--probe-native`,
the program compiles original MSL compute/vertex/fragment shaders, creates
real compute and rendering pipelines, dispatches a 32x32 AABB overlap
mask, draws a solid-red full-screen triangle to a private 3x2
`MTLPixelFormatBGRA8Unorm` texture, blits it into a shared readback
buffer at 256 bytes per row, waits for real terminal GPU completion,
then checks **every pair mask entry, guard word and pixel** against CPU
expectations. This is a real offscreen GPU compute+graphics+blit
correctness witness **only if that native test is actually executed**.
No drawable, real presentation, XR compositor or production Maveb shader
is inferred.

**Native measurement possibility:** Explicit `--benchmark-native` repeats
the same correctness-checked command buffer for five warmups + thirty
measurements and prints nearest-rank median/p95 host encode-to-commit
time, plus GPU command interval only when all reported GPUStartTime/
GPUEndTime values are nonzero, positive, finite and ordered. These
numbers are **not present yet**; source-code presence is not GPU evidence
and a small offscreen command sample is not a whole-engine FPS claim.

**Portability and diagnostics:** If the platform lacks a usable
Objective-C++ Metal/Foundation environment, CMake includes
`tools/labs/gpu_course/native_metal_unavailable.cpp` which returns
77/unavailable, not a fake passed hardware result. The default CPU CTest
still runs on GCC/Clang/macOS independently of hardware availability.
The exact CTest source-pinned provenance record was expanded to **15
targets**, and the insert tool and fixture preservation tests now
cover **15 marker-delimited additions**. The original canonical
manuscript remains unchanged. No user-local commands, LaTeX compilation
or cross-repo changes have been performed.

**Actually inspected source (not altered):** Maveb
`engine/metal/src/Renderer.cpp`, `FrameContext.cpp`, `FrameContext.hpp`;
KairoRenderer `MetalRenderer.cppm` and `detail/MetalBackend.mm`.
The exact observed blob SHAs, source-level frame-lifetime observations
and open follow-ups are in [project-lab-matrix.md](project-lab-matrix.md).
No actual Metal SDK compilation, CTest pass, Metal compute dispatch,
GPU screenshot or benchmark sample has been observed yet in this turn.

**Next:** Chapter 16 — "Profile GPUs and Respect Apple Ownership Boundaries."
Use the same real Maveb/Kairo sources for Objective-C++/C++ callback
lifetime, native diagnostics/counters, Metal GPU capture, retained
references and truthful performance reports rather than another
NES-only detour.


## Pass 16 — Book 01, Chapter 16: Profile GPUs and Respect Apple Ownership Boundaries

**Primary project:** Real Maveb Metal rendering/FrameContext and Kairo's
Objective-C++ Metal implementation, with Kyoto pair correctness and XR
frame/gaze timing as independent clients. NES remains part of the early
computing/binary/project thread, not the default for every new concept.

**Original chapter untouched:** Chapter 16 contains 2,467 existing lines /
~90,695 characters spanning GPU profiling/counters, render-graph/resource
lifetimes, Metal/Objective-C++/SwiftUI boundaries, Vulkan host allocator
callbacks, generated binding boundaries and systems continuation. All that
substantive prose remains authoritative. The additional [GPU-16P — GPU
Profiling and Apple Callback
Ownership](../../Supplementary/C++23%20Project%20Labs/GPU-16P%20-%20GPU%20Profiling%20and%20Apple%20Callback%20Ownership.md)
(~44,409 characters) is staged before the original `### Distinguish host
allocation callbacks from device-memory allocation` heading, after the
original Apple/Metal narrative rather than replacing its explanations.

**Portable C++23 concurrent callback engine:**
`tools/labs/gpu_course/gpu_profile_ownership.hpp` and
`tools/labs/gpu_profile_ownership_cpp23_lab.cpp` implement a
`std::shared_ptr`-owned internal mailbox and frame slot/generation identity;
callbacks do **not** retain a raw renderer/UI pointer. Three concurrent
`std::jthread` HOST callbacks, duplicate/foreign/stale event rejection,
known-before-submit abort, unknown submission quarantine, late callback
after wrapper/UI destruction, terminal status and missing/unusable timing
are tested with SYNTHETIC GPU observation fixtures. The app is responsible
for handing events to the Swift/SwiftUI main actor; this model does not
write UI state from an arbitrary native callback thread.

**Fail-closed profiling evidence:** `Configuration`, `FrameSample`,
`CounterValue` and `summarize_native_verified` require consistent
device, SDK, source, shader, workload, capture tool, extent, successful
terminal and independent pixel/pair correctness, valid CPU/GPU intervals,
and supported/non-fabricated GPU counters. The tests intentionally change
one field at a time and prove rejection. An origin enum can be spoofed in a
synthetic fixture; it is **not** hardware attestation. Actual native run
logs and exact source identity are indispensable for research claims.

**Optional REAL Metal callback and compute:** On Apple with compatible
ObjC++23 compiler, Metal and Foundation frameworks, `tools/labs/gpu_course/
native_metal_callback_probe.mm` registers an actual Objective-C ARC
`addCompletedHandler:` block for a real Metal 64-word compute kernel. The
native test destroys its original mailbox wrapper and weak UI observer
before command commitment, retains only internal completion state,
waits for native callback and terminal status, independently verifies all
GPU words against CPU arithmetic, and labels valid GPU command time as
**one diagnostic sample only**. No Metal execution occurred in this
authoring turn. With absent toolchain/device, the explicit fallback file
`native_metal_callback_unavailable.cpp` reports code 77/unavailable.

**Actual source inspected, NOT edited:** Maveb `Renderer.cpp`,
`FrameContext.cpp` and `FrameContext.hpp`; KairoRenderer
`MetalRenderer.cppm` and `detail/MetalBackend.mm`. Exact Git blob
identities and source-level permit, callback, blocking capture and
timing observations are recorded in [project-lab-matrix.md](project-lab-matrix.md).
An observed `this` callback capture with a destructor draining permits
is a review target, **not** proof of a use-after-free; likewise, a
potential callback timing issue is not a reproduced native failure.

**Cumulative installation:** 16 CMake C++23 executables and normal CTest
targets, 16 exact marker-delimited byte-preserving manuscript inserts,
updated source-restoration/idempotence/conflict tests, and a strict
source-pinned source-hash/CTest JUnit evidence manifest for all 16.
The canonical source file is still unchanged on the revision branch.
There is no LaTeX build, user-local command, or new native GPU result.

**Real evidence still pending:** Cross-compiler build/CTest, macOS
ObjC++23 compilation, actual native Metal callback/kernels, Xcode GPU
capture, device-specific hardware counter sampling, multi-run performance
statistics, and any performance/ownership changes to Maveb or Kairo.
Neither a registered test nor a prepared native source file is a passing
runtime certificate.

**Next:** Book01 Chapter17 — "Python and Numerical Tools for Graphics
Systems." Shift from Apple GPU ownership to numerical/scientific Python
reproduction integrated with real Maveb Gaussian/geometry, Kyoto collision
benchmark statistical validation, Vulkax physics controls and C++23/NumPy
cross-language error checks. Inspect the actual project/source data first,
then produce a substantive additive chapter and independent runnable
oracles without shortening prior material.


## Pass 17 — Book 1 sixteen-chapter coverage/depth audit and release gate

**Audit committed, not a completed book.** [Pass17 source coverage and depth
report](Book01/PASS17_COVERAGE_AND_DEPTH_AUDIT.md) inventories the sixteen
authoritative derived reading chapters and sixteen substantial staged
supplements (~1.44 million original chapter characters and ~507 thousand
additional lesson characters, counted separately). The actual original
reading map contains 39 chapters; **these are not 39 remaining Book 1
passes**, since chapters 17–31 chiefly route to Book 2 and 32–38 to
Book 3, while Vulkan/Metal chapters 9 and 13–16 also belong in Book 6.
Freeze primary-book and cross-book span routing before final print production.

**Verified from fetched chapter/lesson content:** Each of the sixteen
staged supplementary lessons has one matching original heading in its
corresponding derived reading chapter, and each lesson carries substantial
new narrative content. This is a structural fact, not proof of the exact
canonical-master anchor count, executable correctness or editorial depth.

**Genuine coverage gap:** A lexical scan of **all 39 derived chapters
and all 16 staged lessons** found no exact `std::format`/`std::print`,
`std::filesystem`/`std::ifstream`/`std::ofstream`,
`std::ranges`/`std::views`, or coroutine-operator
`co_await`/`co_yield`/`promise_type`/`std::coroutine` examples.
This does not establish absence from every uninspected project or
continuous-master source, but it prevents calling Book 1 a complete
modern C++ mastery course. Prioritize a **real C++23 asset/async
service** integrating trusted filesystem I/O, ranges/views,
format-safe diagnostics and coroutine-frame lifetime/cancellation
with Kairo/Maveb typed assets, plus portable negative-case tests.

**Narrative integrity gaps:** GPU-related original chapters contain
extensive fragmented micro-lines (measured median nonempty line widths
near 12–15 characters in several chapters) and inherited production
tokens like `This is Part 2147` and `### + M135`. The original text must
not be arbitrarily shortened, but coherent explanatory bridges,
commented runnable code, clear first-use derivations and separated
provenance metadata are needed before publication.

**New fail-closed static source gate:**
`tools/book01_coverage_audit.py` compares the exact current canonical
Git blob to the preserved baseline
`d3202af31e1eb644a74565eeb63e7a3352438823`, checks the 39-entry
reading map, 16 chapter-source markers and source anchors, 16 staged
lesson headings, CMake/CTest target registration and evidence source
files. Its output explicitly sets compiler/native-GPU/editorial
verification to **false**. `tools/tests/test_book01_coverage_audit.py`
contains negative tests for changed canonical bytes, a missing CTest
entry and warning-only prose/code proximity. The GCC CI workflow
now runs this audit, publishes machine-readable JSON and tests the
failure behavior. This check has been **authored and connected to CI**;
a successful executed result has **not** been observed in this turn.

**Book 1 release gates:** compiler/CTest+sanitizers on supported
toolchains; truthful native Vulkan/Metal hardware status; complete
foundational C++23 API exercises; cross-project runnable integration
with real Kairo/Maveb assets; readable narrative-first transitions;
seven-book source-span ownership; actual references reviewed; and
source-preserving application/insertion only at the user's chosen
consolidated checkpoint. Word count and pass count are insufficient.

**Next work:** First run/fix the full sixteen-target build and static
audit, then close the identified C++23 language coverage gaps with
substantive engineering code and tests; independently verify the
final editorial/technical release before declaring Book 1 complete.
The prior “4–5 passes remaining” is a planning estimate, **not a
guaranteed completion count**; new issues may require more passes.


## Pass 18 — Book 1 C++23 completeness: real filesystem, ranges and owned coroutines

**Closing a verified curriculum gap:** The Pass17 lexical review of the 39
derived original reading chapters and 16 earlier supplements found no
worked `std::filesystem`, `std::ranges`/`std::views`,
`std::format`/`std::print` or `co_await`/`co_yield` examples by their
standard API spellings. Pass18 now has a **50K+ character cohesive lesson**
[CPP-18A — Filesystem Ranges Formatting and Coroutine Asset Import](../../Supplementary/C++23%20Project%20Labs/CPP-18A%20-%20Filesystem%20Ranges%20Formatting%20and%20Coroutine%20Asset%20Import.md)
adjacent to the existing Chapter06 typed and generational registry. It
explains a useful end-to-end ingestion design, not four detached API tips.

**Real file/data and earlier project interop:**
`tools/labs/foundations_course/modern_asset_ingestion.hpp` and
`tools/labs/modern_asset_ingestion_cpp23_lab.cpp` implement a bounded
binary `std::ifstream` source, per-component symbolic-link and path
policy with explicit TOCTOU caveat, owned file bytes/checksum, sorted
`std::ranges` and lazy `std::views::filter` over manifest entries
owned by a coroutine frame, move-only `std::coroutine_handle`
generator using `co_yield`, manual `co_await` suspend/resume and
cooperative `std::stop_token`, actual `std::jthread` worker with
exception containment and 8 MiB default aggregate byte budget, and
type-safe `std::format`/`std::println` where available. The test
creates temporary TXR4 texture and MSH1 triangle mesh files, rejects
invalid filesystem inputs and size limits, checks owned bytes across
coroutine suspension and file removal, exercises bounded result
publication and cancellation, then calls the **original Pass05/06**
`GenerationalAssetRegistry::import/lookup/erase` under the full
Book01 include tree. The test does **not** invent glTF/PNG/KTX support.

**Observed local standalone compilation:** GCC 14.2, Clang 17, GCC
optimized Release and Clang Address/UndefinedBehaviorSanitizer builds
of the standalone implementation passed actual tests in the author's
container. The full earlier typed registry source was not mounted in
that local smoke environment, and its branch integration was **not
executed there**. The repository CMake now defines
`ALLMIGHT_COURSE_REQUIRE_REGISTRY=1`; any missing original registry
header is an explicit compilation error rather than a passing CTest
that skipped the hard part. A negative local compile verified that
requirement. The 17th full course CTest is registered but its
whole-repository result is **not yet observed**.

**Cumulative integrity:** Seventeen C++23 executable targets and
seventeen default CTest names; seventeen exact marker-delimited
additions staged with conflict/idempotence/source-restoration tests;
source/JUnit evidence requires seventeen real tests to run.
`tools/book01_coverage_audit.py` still checks the **sixteen original
reading chapters** but also checks that the newly added **seventeenth
lesson** belongs to the unique original Chapter06 anchor (not an
invented seventeenth chapter). It preserves the original master Git
blob `d3202af31e1eb644a74565eeb63e7a3352438823` when staged.
The source changes do not regenerate LaTeX or alter existing manuscript
prose. Human-readable CMake project/preset names now reflect
**AllMight Book01 cross-project C++23**, not an NES-only book.

**Cross-project sequence:** Maveb/Kairo consume the typed filesystem/
async asset interface; Kyoto collision research and Vulkax simulation
consume source-pinned input manifests with independent correctness
oracles; XR consumes revision-aware asynchronous tasks and separate
gaze/display deadlines. No production engine or dataset is claimed
modified, built or benchmarked by this source pass. The original NES
parser/emulator is one earlier model among several, not the only
application developed by these books.

**Evidence document:** [Pass18 executable and integration-status
ledger](Book01/PASS18_CPP23_ASSET_PIPELINE_EVIDENCE.md) records actual
observed compiler modes, failure tests, code portability and remaining
constraints. This report is descriptive; only future fully executed,
source-pinned CTest/JUnit receipts can certify the whole course.

**Resumed Pass18 hardening (same branch):** The worker now validates an
independent request-count limit (default 1024, maximum 4096) and a
4096-byte pathname ceiling *before* launching its jthread. The chunked
file reader checks the passed stop_token before path work, between
4096-byte reads and before successful publication. This prevents an
unbounded stream of errors bypassing the byte budget, but does not
pretend to interrupt an already blocked native read or prevent
filesystem TOCTOU races. New adversarial tests cover both limits and
cancellation.

**Exact local test evidence:** The Git blobs
`23a581956e35b8ffd6116e5483ff96a49e790be6` (header) and
`28369c2bb8ac68d0c08b529b02152c6d5038c46c` (test) were
confirmed byte-identical to the rebuilt standalone sources.
GCC 14.2 C++23 strict-warning CTest: **1/1 passed**.
Clang 17 C++23 + ASan/UBSan CTest: **1/1 passed**.
These two real local outcomes have standalone JUnit receipts in the
working container. They **do not test** the shared Pass06 registry,
the full 17-target course or Apple Metal/Vulkan devices; see the
updated [Pass18 evidence ledger](Book01/PASS18_CPP23_ASSET_PIPELINE_EVIDENCE.md).

**Remaining technical work:** run and repair the 17-target GCC/Clang/
AppleClang/sanitizer build, verify registry integration and native
Vulkan/Metal probes, add true native race-resistant file operations and
bounded async job queues where the production threat model requires,
then complete narrative/cross-book source routing and the
consolidated-only manuscript insertion/release review. One new
standalone passing program does not certify Book 1 complete.


## Pass 19 — Book 1 integrated asset-to-evidence CPU capstone

**A deliberately connected milestone, not another unrelated demo.** The
[CPP-19E continuous project lesson](../../Supplementary/C++23%20Project%20Labs/CPP-19E%20-%20A%20Complete%20Asset%20to%20Evidence%20Trace.md)
and [real cross-pass C++23 capstone](../../tools/labs/book01_end_to_end_cpp23_lab.cpp)
join the existing Pass18 on-disk file reader/worker, Pass05 TXR4/MSH1 codecs,
Pass06 generational registry, Pass08 task-dependency planner and Pass09
memory/retirement oracle into **one** portable source->scene->compute->receipt
trace. The earlier NES emulator is not the focus of this stage: these general
asset, ownership and evidence contracts carry onward into Kairo, Maveb, Kyoto,
Vulkax and XR with appropriately different domain oracles.

**What the authored executable checks:** two real temporary binary inputs,
exact independent weighted pixel sum **297** and mesh signed double-area
**65536**, reproducible source fingerprints and byte-identical receipt strings
despite fresh temp paths/registry owners, six declared DAG tasks and
independent topological edge verification, CPU-only visibility failure before
explicit memory publication, refusal of premature virtual retirement,
successful retirement only after modeled completion, rejection of stale
registry handles, and scene revision 7 accepting once while old version 6
and repeated version 7 are refused. Missing mesh, malformed mesh and truncated
texture are independent negative fixtures. Scene commit is *single-threaded*;
the memory oracle is *not* a native GPU submission.

**Staging and provenance:** eighteenth `add_executable` and eighteenth default
CTest registered; eighteenth marker-delimited non-destructive insertion
staged under original Chapter 10, after the earlier Pass10 reproducibility
addition; source/JUnit checks and byte-preserving insertion fixture expect
18 rather than 17. The original continuous manuscript was **not rewritten,
truncated, regenerated or compiled**. For exact source blob IDs and the
unverified build boundary, consult the
[Pass19 evidence ledger](Book01/PASS19_END_TO_END_EVIDENCE.md).

**Not certified yet:** A full real 18-target GCC/Clang/AppleClang build,
sanitizer runs, JUnit source-pinned release proof, native Metal/Vulkan execution,
concurrent editor publication, and authorized consolidated manuscript insertion
remain open. Source counts and an authored CTest are not pass results. Continue
the next verification/editorial pass without asking for a per-pass Mac pull,
build or LaTeX command.

## Pass 20 — Restored CI, corrected ownership contracts and concurrent scene publication

**Original complete book preserved.** Pass20 expands the existing
[CPP-19E source lesson](../../Supplementary/C++23%20Project%20Labs/CPP-19E%20-%20A%20Complete%20Asset%20to%20Evidence%20Trace.md)
with a deep derivation of linearizable asset publication, no-throw
transactional swap, synchronized owned read snapshots, revision overflow,
eight competing publishers, thread-creation failure unwinding and
metamorphic texture/triangle correctness checks. This is an additive
continuation of Pass19's already mapped original Chapter10; there are still
**18** staged lessons/targets, not an invented nineteenth original chapter.
No canonical source insertion, LaTeX regeneration or book shortening occurred.

**Actual defects discovered and repaired:** the original GitHub Actions
workflow was invalid before job startup because `C++23` appeared
unescaped in a glob; the real NROM dependency lab tried to move a
move-only cartridge out of a `const std::expected`; two GPU C++23
tests passed an explicitly bool-convertible `std::expected` to a
`check(bool)` parameter; the native Metal probe narrowed `size_t`
inside braced `uint32_t` initialization. Linux Clang18 lacked the
correct `std::expected` feature gate and is replaced in CI with
probed Clang19. The older Apple toolchain lacked `std::jthread` and
`std::stop_token`; its successor CI image now runs an explicit
support check before attempting to build.

**Observed build evidence:** one targeted capstone GCC CTest passed;
a later capstone CTest containing real eight-worker scene publication
also passed. In the restored full-matrix job at commit
`0697e7e181f7d7a0e1c71d53f1f598bf52930c03`, **GCC passed 18/18
CTests**, 19 Python fixture tests in three groups and the source-pinned
provenance verification; **Clang19 ASan+UBSan also succeeded**.
This does not imply all-platform certification: that particular macOS
job still used Xcode16.4 and failed, and commits made afterward require
their own source-matched green suite.

Detailed links, exact failure explanations, qualification of source
revisions and still-open native GPU and research verification are in
[Pass20 evidence and source audit](Book01/PASS19_END_TO_END_EVIDENCE.md#pass-20--actual-multi-toolchain-repair-and-evidence-19-november-2026).

**Next release gate:** obtain actual successful GCC, Clang19, sanitizer
and AppleClang/Xcode26 CTest+JUnit receipts against one source revision,
then continue the cross-book editorial integrity audit. Do not issue
user-side Mac commands until the requested one-shot checkpoint.

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
