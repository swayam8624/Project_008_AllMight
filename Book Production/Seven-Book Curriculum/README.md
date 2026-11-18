# Project AllMight — Seven-book C++23 curriculum (in progress)

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
