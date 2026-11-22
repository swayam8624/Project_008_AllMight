# Book Two — Pass 08: One Complete Reader Book (PDF + EPUB Semantic Parity)

**Book:** From Equations to Geometry  
**Date:** 20 November 2026  
**Branch:** \`curriculum/book02-mathematics-refinement-pass01\`

## Editorial decision

The author requires **one complete PDF book**, not two physical-part PDFs, and expects conventional book reading order: a minimalist three-field cover; copyright page; preface; a practical reader's guide; notation/conventions; one table of contents; list of figures; 16 continuous chapters; supporting notes, source code, and primary references.

The frozen two-part templates remain **read-only source inventories**, not the user's final reader-facing delivery. \`Tools/book02_single_volume.py\` composes one continuous LaTeX document using the exact original chapter/reference fragment inputs. The old P01/P02 PDFs, generated files and source manuscripts are not deleted.

## Full print composition

- One LuaLaTeX \`book\` document with one title page and one TOC, **not concatenated PDFs**.
- 16 original mathematical chapters, in their original source order and bytes.
- All 36 added scientific/educational TeX lessons, inserted at their prior chapter anchors.
- Roman-numbered front matter: copyright notice, author-written teaching preface, working guide, notation and conventions, contents and list of figures.
- Continuous 16-chapter main matter. No mid-book "Part 2" title page or chapter-number reset.
- Back matter: all **31 frozen reference notes**, three original C++ source attachments (each in its own breakable pale-slate/cyan panel), and a selected primary-reference reading list.
- The selected primary sources are Shewchuk (1997) for exact predicates, Crane–Weischedel–Wardetzky (2013) for heat geodesics, and Veach (1997) for sampling. Links are to original authors/institutions. The list is explicitly selective, not an invented claim of comprehensive citations.
- Exactly **three visible cover text elements**: \`From Equations to Geometry\`, \`Mathematics, Geometry, and Computational Foundations for Graphics, Simulation, and Research\`, and \`Swayam Singal\`. No series mark, volume number, edition badge, publisher logo, ISBN or additional caption.
- The PDF cover's text remains vector typeset in TeX. The EPUB's default cover JPEG is independently rendered from the same textual fields with installed serif fonts.

## Original-chapter code styling

The actual frozen 16 chapter files were audited. They contain **112 plain lowercase \`verbatim\` blocks and zero \`Shaded\` blocks**. This explains why earlier styling work largely missed original project code.

The **Book Two-only** preamble now configures FancyVerb with a print-safe, cyan-framed, pale-slate code surface. In the supplemental \`Shaded\` wrappers, the inner FancyVerb frame is disabled to prevent nested borders. The three long \`VerbatimInput\` attachments receive individually breakable \`tcolorbox\` panels. EPUB's Book Two CSS now styles raw \`pre\` blocks, not only \`div.Shaded\`. Native derivation callouts keep their separate lavender semantic role; numerical pitfalls gold, laboratory steps mint, concepts cyan.

## Build and export behavior

The existing \`book02_release_candidates.py --print-only\` now compiles the **one complete PDF** as \`From Equations to Geometry.pdf\`, allowing up to 2,500 pages for the digital master. It uses the two original part \`main.tex\` files solely to enumerate content and retains source-input/provenance checks. It does not relax the 800-page limit previously applicable to separately manufactured physical print parts.

\`--export-only --print-only --export-dir "$MASTER"\` exports the verified **single PDF** even while EPUBCheck is failing, to a dedicated immutable \`*-PDF-Review\` folder. A later successful full PDF+EPUB export uses its own folder, avoiding fingerprint/manifest conflicts.

When an older split-output verification report exists, the updated pipeline archives that report and starts a new release record instead of combining incompatible old print and new EPUB evidence.

EPUB composition now reuses the same copyright/preface/reader guide/notation/selected references. When no front-cover JPEG was explicitly supplied, the stage EPUB builds a controlled three-field typographic JPEG. An explicit JPEG override remains available. The full EPUB is still required to pass EPUBCheck before it is described as verified or exported alongside the book.

## Source tests and remaining production gates

- \`test_book02_unified.py\` verifies exactly 16 original inputs, 31 reference inputs, three attachments, 112 raw code blocks, the new framed code style, **one** title page and TOC, continuous document structure, selected references and shared EPUB semantic frontmatter.
- \`.github/workflows/book02-unified-digital-book.yml\` runs the source test without compiling LaTeX.
- The separate native C++23, source-lock, reconstruction, EPUB supplemental-parity and mathematical protocols remain intact.
- **Not yet accepted:** the full one-volume typeset PDF has not been compiled on the author's Mac; its page breaks, panel wrapping, gutter, references and cover require inspection. The full EPUB previously generated all 16 chapters but was rejected by EPUBCheck; that specific validator diagnostic is required for a truthful repair. A one-thousand-page digital PDF may be too long for standard print-on-demand manufacturing; no single-physical-volume claim has been made.

## Mac publication commands

\`\`\`bash
(
  set -e
  MASTER="$HOME/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master"
  cd "$MASTER"
  git pull --ff-only origin curriculum/book02-mathematics-refinement-pass01
  export PATH="/Library/TeX/texbin:$PATH"
  PY="$MASTER/.venv-book01/bin/python"
  BUILDER="$MASTER/Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py"
  "$PY" "$BUILDER" --check-only --print-only
  "$PY" "$BUILDER" --print-only
  "$PY" "$BUILDER" --export-only --print-only --export-dir "$MASTER"
)
\`\`\`

The separate EPUB operation is:

\`\`\`bash
"$PY" "$BUILDER" --check-only --epub-only
"$PY" "$BUILDER" --epub-only
"$PY" "$BUILDER" --export-only --export-dir "$MASTER"
\`\`\`

The above EPUB commands deliberately remain separately gated. Errors must be corrected, not ignored, and an already verified PDF must remain accessible even if EPUBCheck rejects the ebook.
