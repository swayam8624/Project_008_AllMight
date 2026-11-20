# Book Two — editorial, typography, learning, and scientific-figure contract

Applies only to From Equations to Geometry, both physical print parts,
the complete-stage EPUB, and the source-led C++23 numerical workbench.
Book One is read-only reference material, not an active development target.

## Reading hierarchy and typographic composition

- Establish a repeatable hierarchy: chapter, section, subsection, paragraph,
  theorem/definition, derivation, C++23 lab, worked example, numerical pitfall,
  exercise, source listing, figure, caption, and source note.
- Keep long-form narrative, derivations and explanations outside boxes whenever
  that improves reading flow. A colored callout is a semantic tool, not a
  decorative container for every paragraph.
- Match the supplied Book Two cover's academic navy, restrained cyan and subtle
  gold; avoid neon, excessive line ornaments, glowing math on body pages or
  unrelated saturated accents. Use dark text on pale fills.
- Maintain a consistent baseline, comfortable measure, line spacing, balanced
  headings and small but legible citations/captions. Avoid short last lines,
  heading orphans, split equations, empty pages and widows when feasible.
- Keep theorem equations at genuine vector quality and distinguish all
  variables, coordinate frames, dimensions, units, and sign conventions.
- Tables need meaningful units, proper heading repetition, and no clipped
  columns. Figure text must remain legible after actual print scaling.

## Semantic boxes (Book Two print preamble)

- booktwoconcept: light cyan for mental models and invariant definitions.
- booktwoproof: pale violet for complete derivations and validity domains.
- booktwopitfall: subdued gold/rust for numerical and scientific limitations.
- booktwolab: pale teal for native C++23 experiment checkpoints.
- Shaded: neutral slate code panels with a quiet cyan border.
- Never use color as the only evidence of type: labels, headings, equations
  and code semantics must stand on their own in grayscale or screen readers.
- Avoid boxing figures unnecessarily. Meaningful plots should carry labels,
  explanation, exact experiment provenance and uncertainty where relevant.

## Code policy

Primary instructional language is ISO C++23, with portable CMake and CTest.
Python is secondary for optional CSV/JSON data manipulation, plotting,
independent references or reproducibility checks, never a substitute
implementation for a C++23 numerical project. Explain APIs before showing
them, comment on WHY not just WHAT, give full compile commands, and distinguish
portable C++ from hardware/OS/compiler extensions. A project should have
an independent oracle, boundary tests, invariants, failure-mode tests and
reproducible input/evidence exports.

## Figure and numerical integrity

Never imply an analytical illustration is measured benchmark output.
For every scientific plot record units, domain, method, parameter values,
solver, seeds, sample count, exact or independent reference, error definitions,
and limitations. Include calculated curves alongside diagrams only when the
source actually produces them. PDFs should use vector PDF artwork when possible;
EPUB should use real SVG or other correctly sized accessible media, not screenshots
of mathematical text. Define image alternative text and caption separately.

## Print/EPUB acceptance gates

1. Check 16 original chapter source checksums without modifying sources.
2. Retain 36 additive source lessons and their unique chapter ownership.
3. Confirm the C++23 CMake target builds, native tests run even in Release,
   and at least one dataset can be exported as structured rows.
4. Parse all added LaTeX through Pandoc; ensure CodeBlocks, math, headings,
   lab warnings and citations survive into the EPUB semantic representation.
5. Compile both full print interiors on the author's Mac. Inspect title and
   reading-map pages, first and last chapter, one math-heavy spread, one
   code-heavy spread, all callout types, figures/captions, end matter and TOC.
6. Render suspect pages and compare actual trim margins, gutter, hierarchy,
   no overfull boxes, no missing glyphs or low-resolution figures. Check
   both grayscale and full color.
7. Run EPUBCheck and sample at different reader/font sizes. Verify reflow,
   internal links, mathematical layout, semantic sidebars, code wrap,
   image captions, accessible image alt text, cover and complete source order.
8. Never call a book publication-approved solely because CMake and LuaLaTeX
   returned success. Math proof review and visual inspection are separate gates.

The uploaded Book Two artwork has the narrower proportions of a digital cover.
A print wrap needs the final selected trim, spine width, bleed and print resolution;
do not stretch the portrait JPEG to fill an 8-by-10-inch interior page.
The current release builder intentionally accepts that cover for EPUB,
not as a print-wrap substitute.
