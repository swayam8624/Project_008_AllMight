# AllMight Codex LaTeX profiles

allmight-book.sty is an original LuaLaTeX style layer for the six book volumes. Add LaTeX to TEXINPUTS (or place the style beside a main file) and choose one profile:

    \documentclass[11pt,twoside,openany]{book}
    \usepackage[print]{allmight-book}

Use [print] for the 8 × 10 inch mirrored print layout. It uses TeX Gyre Pagella text with TeX Gyre Pagella Math, Menlo code, microtype, running heads, booktabs tables, longtable support, and understated chapter hierarchy. Body type is 10.5 pt on a 13 pt baseline. The converter may also load its required longtable, booktabs, array, calc, graphicx, hyperref, fancyvrb, xcolor, and amsmath packages; the style loads these itself and defines Pandoc's tightlist, Shaded, Highlighting, and pandocbounded compatibility macros.

Use [ebook] only for a fixed-page reading proof PDF (6 × 9 inches). Reflowable EPUB is a separate output made from semantic source: this profile does not make the PDF itself reflowable. The generated EPUB owns its viewport, font sizing, and reading order.

External diagrams should arrive at a sensible authored size. pandocbounded, BookFigure, and AllMightGraphic only shrink to fit within 80% of the text width and 40% of the page text height while preserving aspect ratio; they do not enlarge small images. These geometric caps do not certify label readability: source-aware QA must flag a figure if any label would fall below 8 pt at print size. Split complex diagrams into semantic panels before composition rather than shrinking their labels further. Native equations remain TeX math. Tables remain columnar, use booktabs rules, and repeat their authored headers in longtable; the theme does not convert rows into cards.

Definitions, theorems, and worked examples can use allmightdefinition, allmighttheorem, and allmightexample, each with an optional title. Reserve these callouts for those semantic roles.

## Design references and source handling

- [LaTeX ePub / eBook Template](https://www.overleaf.com/latex/templates/latex-epub-slash-ebook-template/csjgmvzppmcr), writeLaTeX, LPPL 1.3c. The public page says its memoir example imitates an ebook-like PDF and does not export EPUB directly. The page's source snippet and license statement were reviewed. “Open as Template” redirected to a login page and the public ZIP URL returned 404, so the full source package was not available for review or inclusion.
- [Preprint Book Manuscript Template](https://www.overleaf.com/latex/templates/preprint-book-manuscript-template/ghrbwppzhzjx), Agni Datta, CC BY 4.0. The public page's source excerpt was reviewed: it uses the book class, two-sided/open-right layout, and ptmt; the full project could not be downloaded without Overleaf login. No template source was copied.
- [BookCalculus](https://github.com/texdrop/BookCalculus), texdrop. The complete repository archive, calculus.tex, and LICENSE were downloaded and reviewed. Its file license permits use and modification in final works but prohibits redistribution of the template source and requires preserving its notice in distributed template files. This project includes no BookCalculus source or derivative code. The original style instead uses semantic definition/theorem/worked-example callouts, modest chapter hierarchy, compact booktables, and a restrained color system as functional design cues. Keep BookCalculus source and its license outside this package.

The print profile includes a static default-weight instance of Google Fonts Noto Emoji in `Assets/Fonts/`, under the SIL Open Font License 1.1; its license text and the variable-font source/provenance are included beside the font. TeX packages were not installed as part of this style work. The verified runtime was the existing TeX Live 2025 full distribution; see the parent production state for the pinned executable paths.

## Design proof

sample/design-proof.tex covers a five-column rounding table, native matrices and derivatives, a code block, a vector flow, semantic callouts, and a continued multi-page table. Compile twice:

    TEXMFHOME=/tmp/allmight-local-texmf TEXINPUTS=..//: \
      /usr/local/texlive/2025/bin/universal-darwin/lualatex \
      -interaction=nonstopmode -halt-on-error -output-directory=build design-proof.tex

sample/ebook-profile-proof.tex verifies the distinct fixed-page ebook proof profile. Both profiles still require visual review; compiling is not acceptance of the complete six-book layout.

The production Lua filter `Tools/semantic-callouts.lua` marks only an explicit role heading (definition, theorem, worked example, exercise, or active recall) together with its immediately following lead paragraph. In LaTeX that pair uses a non-numbered colored wrapper; later prose remains outside. In semantic formats the wrapper classes remain on the Div for styling. The filter also emits a `\hypertarget{header-id}{}` immediately after each identified LaTeX heading so cross-volume references can address stable source IDs while the normal section label and table-of-contents entry remain intact.

For LaTeX, Pandoc inline `Code` spans use `\BookInlineCode`, which permits line breaks inside long identifiers and paths without shrinking the type size or adding hyphens. The conversion uses Pandoc's own LaTeX escaping before wrapping the text; authored math `\texttt` and verbatim code are unaffected.

Compact print diagram candidates and their source hashes are listed in `Assets/Diagrams/Print-v1/overrides.json`. The manifest records the selected layout and projected minimum label size for each flagged asset. Eleven candidates remain explicitly held for panel recomposition because their compact variants still fall below 8 pt. Canonical Mermaid Markdown and SVG assets are unchanged.

The Noto Emoji variable font was downloaded from [Google Fonts' Noto Emoji family](https://github.com/google/fonts/tree/main/ofl/notoemoji) as `NotoEmoji[wght].ttf`, renamed `NotoEmoji-Variable.ttf` for provenance, and is distributed with the adjacent [SIL Open Font License 1.1](Assets/Fonts/NotoEmoji-OFL.txt). `NotoEmoji-Static.ttf` is its default-weight (`wght=400`) static instance, with the source hash, axes and generated-file hash recorded in `Assets/Fonts/NotoEmoji-Static.provenance.json`. The static instance is used only to render source Unicode emoji that the selected macOS TeX runtime cannot render from its system color font.
