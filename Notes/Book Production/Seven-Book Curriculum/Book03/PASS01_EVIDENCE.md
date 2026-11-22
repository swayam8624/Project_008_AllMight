# Book 03 — Pass 01 authoring evidence and honest limits

**Book:** From Geometry to Images  
**Branch:** curriculum/book03-geometry-images-pass01  
**Starting point:** restructured main at ba3e31681bb74a71800cd35ace375c2051bcbafa  
**Authoring date:** 23 November 2026  
**Scope:** first narrative chapter, reader route, source custody, two illustrative plates, one portable C++23 executable and tests.

## What has actually been written

- README.md: the Book 03 working source's entry point.
- EDITORIAL_CONTRACT.md: long-form, plain-human-language pedagogy; every derivation must be precise; existing manuscripts, years of accumulated work, and book-release layout must be preserved.
- SOURCE_LEDGER.md: known original chapters and live source locations; Stage 03 generation remains read-only; original text cannot be replaced by summaries.
- CHAPTER_PLAN.md: seven-part, thirty-two-chapter reader structure and twelve cumulative project milestones. This is a **plan**, not a statement that the chapters have all been written.
- Chapters/01 - The Picture Made of Little Windows.md: full-length original opening explanation moving from the tiny house to continuous/discrete image coordinates, encoded color, row-major storage, pixel centers, a C++23 canvas, P6 bytes, line rasterization, numerical worked examples, and reconstruct/falsify/transfer/explain exercises.
- Assets/01-pixel-addresses.svg: a 7-by-5 top-left-origin grid, marked index (3,2), center (3.5,2.5), and row-major byte arithmetic.
- Assets/02-lines-and-discrete-pixels.svg: seven selected pixels and their continuous geometric reference, with exact declared discrete path and source-controlled accessible description.
- Code Parts/tools/labs/book03_cpp23/include/book03/pixel_canvas.hpp: bounds-checked RGB8 canvas, continuous pixel centers, half-open rectangles, integer line walker, explicit PPM P6 writer.
- Code Parts/tools/labs/book03_cpp23/src/main.cpp: sample house and line-aliased image.
- Code Parts/tools/labs/book03_cpp23/tests/pixel_canvas_tests.cpp: deterministic fixtures and malformed-input tests.
- Code Parts/tools/labs/book03_cpp23/CMakeLists.txt: independent C++23 build + CTest.
- Code Parts/tools/labs/book03_cpp23/editorial_gate.py: read-only source narrative, original vector assets and current-repo-layout checker.
- .github/workflows/book03-cpp23-pixel-canvas.yml: automated C++23/test/PPM output and source contract pipeline.

## Preservation acceptance, inspected with connected GitHub

At the first end-of-pass branch comparison, the changes from main were **only added files**. The old full Notes/Continuous Notes/ manuscripts, Notes/Chapter Edition/ derived copies, Notes/Book Production/AllMight Codex/LaTeX/ generation, CURRENT.json, physical-parts.json, published Book 01 and Book 02 artifacts and the Git-ignored From Bits to Research/ output directory were not modified.

The correct directory structure is:
- Notes/Book Production/Seven-Book Curriculum/Book03/ for new Book 03 reader source and figures.
- Code Parts/tools/labs/book03_cpp23/ for code.
- From Bits to Research/EPUB/ and From Bits to Research/PDF/ **only after final author approval**.
- The original legacy stage 03 TeX remains input evidence; it is not relabeled as a completed Book 03.

## Validation state — do not overstate

- Chapter and code have been written and linked; **full independent mathematical audit pending**.
- A CI workflow has been authored; **do not describe it as passed without inspecting an actual run**.
- Native macOS build, real-device rendering, reader validation, complete PDF, EPUB and KDP releases **were not attempted**.
- Standalone visual diagrams are original scientific schematics; they have titles/descriptions, but final PDF print size, font legibility and EPUB rendering remain to be tested.
- The Chapter 01 image examples are encoded RGB8 values, not physically linear lighting measurements, and the line algorithm is discrete center-path drawing, not coverage integration.
- Legacy Book 03 source chapters have been identified but not yet exhaustively reviewed or folded into the new reader. Preservation, not shortening, is the constraint.

## Reproduction protocol for the final author-review stage

When the book is complete, run the independent source-only editorial gate, configure the C++23 CMake project, compile both targets, execute CTest, generate house and line PPM files, inspect their bytes and display, and record toolchain + outputs. Only after the full book's chapters, source integrations and figure proofs are accepted should the author run the Mac-specific LaTeX and EPUB production pipeline. Do not burden the author with per-pass build commands.

## The next true authoring task

Chapter 02, **Every Place Needs an Address**, should continue the story from the pixel grid into signed coordinates, point/displacement distinction, units, vector bases and coordinate frames with an interactive C++23 coordinate playground. It must connect mathematically to Chapter 01's image addresses and to the existing long geometry material, with exact numeric round-trips and wrong-convention failure fixtures. The next pass must also begin the substantive legacy source-by-source routing audit rather than replacing original camera chapters with a shallow overview.
