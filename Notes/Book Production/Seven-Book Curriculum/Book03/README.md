# Book 03 — From Geometry to Images

**Subtitle:** 3D Geometry, Transformations, Cameras, Projection, and Image Formation  
**Series:** From Bits to Research  
**Active branch:** curriculum/book03-geometry-images-pass01  
**Author:** Swayam Singal  
**State:** Authoring pass 01; NO Book 03 publication files have been generated or promoted.

> A computer can draw a triangle. This book is about discovering what that deceptively simple sentence requires us to know, and then doing it ourselves.

## First read

1. [The editorial and source-preservation contract](EDITORIAL_CONTRACT.md) — our rules for the manuscript, narrative and release layout.
2. [Thirty-two-chapter reading and project plan](CHAPTER_PLAN.md) — our roadmap, not a false claim of completion.
3. [Original-source routing ledger](SOURCE_LEDGER.md) — where the substantial pre-existing graphics/camera material lives and how we will preserve it.
4. [Chapter 01 — The Picture Made of Little Windows](Chapters/01%20-%20The%20Picture%20Made%20of%20Little%20Windows.md) — opening full-length, reader-facing chapter draft.
5. [P01: Pixel Canvas, C++23](../../../../../Code%20Parts/tools/labs/book03_cpp23/README.md) — first executable experiment and independently checkable tests.

## What is already authored?

Pass 01 begins the new book with an original narrative on the distinction between a physical scene, a mathematical image and a stored pixel; discrete addressing; continuous sample locations; integer drawing; overflow-aware image buffers; PPM export; and a deliberately imperfect line rasterizer. The first C++23 project and accompanying tests are intended as a runnable contract, not a replacement for the full reader book.

Pass 01 does **not** assert that all 32 chapters have been drafted, that the legacy source has been integrated, that all C++ tests have passed, or that a PDF or EPUB exists. Later passes will use the substantial legacy chapter material at the locations documented in SOURCE_LEDGER.md. The author explicitly plans to compile and visually review the finished book on macOS at the end of the book-development process.

## Teaching voice

Use one continuous, warm, lively explanatory voice. A curious child should be able to follow the *story* of the problem; an advanced reader should still be able to derive every equation, understand every program, and challenge its numerical assumptions. An analogy is an invitation, never a substitute for proof. Avoid empty jokes, filler, vague promises, and mechanical repetitions of “what we will learn.”

A typical chapter follows a tiny scene or question → a counterintuitive observation → careful definitions → diagram or exact worked example → derived mathematics → commented C++23 implementation → tests and failure cases → reader exercises → bridge to the next idea. Do not make every page a list. Put substantial meaning into paragraphs.

## Repository boundaries — nonnegotiable

The checked **current main** layout separates notes and code:

- Working Book 03 manuscript: Notes/Book Production/Seven-Book Curriculum/Book03/
- Authoritative existing manuscripts: Notes/Continuous Notes/
- Derived reading copies: Notes/Chapter Edition/ (do not hand-edit these)
- Existing seven-stage TeX generation: Notes/Book Production/AllMight Codex/LaTeX/
- Executable companion code: Code Parts/tools/labs/book03_cpp23/
- Actual, local publication library: From Bits to Research/EPUB/, From Bits to Research/PDF/, Archive/, Manifests/.

The publication directory remains untouched until explicit promotion of an approved single whole-book PDF and approved EPUB. Never create another publications tree, rename a released file, move the entire repository, overwrite a generated chapter or manufacture an output by copying a preview.

Do not modify Books 1 or 2: both are already published. The legacy stage name “03-Geometry-Cameras-and-Image-Formation” is a generator identifier; its content is relevant source, but the public title remains **From Geometry to Images**. Preserve original material and fold it into the eventual reader composition with traceable source mapping rather than replacing it with 32 short summaries.

## Planned release

When the manuscript, project labs and figure proofs are genuinely complete, compile and validate **one full reflowable EPUB** and **one full reading PDF** for Book 03. The author will run the Mac-specific production compile and inspect its output. Until then: manuscript and code only, no PDF, EPUB, or local release commands.

## Pass ledger

| Pass | Deliverable | State |
|---|---|---|
| 01 | Original first chapter, 32-chapter route, source-preservation ledger, portable C++23 Pixel Canvas | Authored on this branch; compilation/verification reported separately |
| 02 | Coordinate spaces, transforms and interactive geometry viewer | Not started |
| 03 | Meshes, topology and robust predicates | Not started |
| 04 | 3D cameras and projection | Not started |
| 05 | CPU clipping, triangle rasterization and visibility | Not started |
| 06 | Lens/sensor/image formation and calibration | Not started |
| 07 | Stereo geometry and introductory reconstruction | Not started |
| 08+ | Original-source reconciliation, deep editorial/math audit, figures, print/EPUB composition and acceptance | Not started |
