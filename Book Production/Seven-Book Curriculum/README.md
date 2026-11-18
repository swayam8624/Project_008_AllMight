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
