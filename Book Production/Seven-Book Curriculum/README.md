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
