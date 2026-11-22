# Book Two — Pass 04 Publication Evidence and Remaining Gates

**Series:** From Bits to Research  
**Public book title:** From Equations to Geometry  
**Subtitle:** Mathematics, Numerical Methods, Optimization, and Visual Computing  
**Branch:** `curriculum/book02-mathematics-refinement-pass01`  
**Status:** **Publication engineering in progress; not yet a full-book KDP release certificate.**  
**Date:** 2026-11-20 (Asia/Kolkata).

## 1. Source preservation and semantic integration

The original production snapshot is `Book Production/AllMight Codex/LaTeX/Generations/.generation-n3azv4es`. It contains **16 Book Two source chapters**, eight in P01 and eight in P02, totaling **1,753,813 bytes**. Their Git blob fingerprints are now permanently pinned in `LaTeX/Tools/book02_original_source_lock.json`. The executable `book02_source_preflight.py` rejects an altered original chapter, missing chapter anchor, missing chapter, wrong source order, or lost supplemental source.

The added content comprises **35 distinct substantive and figure-source TeX fragments** (original print vector figures, mathematical exercises, paper-derived lessons, and one cumulative atlas project). No original chapter is rewritten or deleted. The print composer inserts them in temporary LaTeX working copies at specific chapter boundaries. The full-source EPUB builder now inserts their actual Pandoc AST conversions at the same boundaries, preserving the frozen original 568-row conversion manifest and original AST hashes.

EPUB figure coverage includes **12 science plots**: five analytic equivalents of the native PGFPlots plates, plus seven numerically computed research illustrations. Every EPUB figure has an SVG asset, a nonempty text alternative and the original source-derived caption. The EPUB adapter intentionally rejects a leaking raw TikZ image or missing packaged SVG.

EPUB metadata and the print build-copy title now use `From Equations to Geometry`. The frozen internal stage id `02-Mathematics-for-Visual-Computing` remains unchanged to preserve references and source ownership.

## 2. Successful reproducible checks

**Post-layout-correction code snapshot:** GitHub Actions source commit `86529be1668b` produced successful verification across all existing routes:

- [Original math and insertion contracts — run 44597849523](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44597849523)
- [Paper implementations and numerical reference parity — run 44597849226](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44597849226)
- [Seven source-computed research plates — run 44597849213](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44597849213)
- [Complete 35-source EPUB semantic check and EPUBCheck — run 44597849222](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44597849222)

The semantic EPUB integration test converts **all 35 additional source files**, places them after 16 original-chapter structural markers and creates an EPUB3 proof containing **12 properly packaged SVGs** and alternative text. This real EPUB proof passes the official **EPUBCheck 5.4.0** validation. It is a structural proof with placeholder original-chapter headers, **not** the actual complete 16-chapter Book Two EPUB. The exporter and proof workflow are recorded in `book02-publication-pass04.yml`.

## 3. Independent LaTeX print proof of all new material

All 35 supplemental TeX source files were also exported from GitHub Actions and compiled independently in a local 6×9-inch LuaLaTeX book test using the already validated research PDF figure assets. The result was **68 pages**, with the five native PGFPlots scientific diagrams and the seven imported research diagrams. The PDF was rendered into a nine-page visual contact sheet for human overview.

The first run exposed one unavailable proof-host font (TeX Gyre Pagella) and substantive LaTeX line-overflow warnings. This proof environment used **Linux Libertine O** rather than silently claiming the Mac's actual production font. Seven small source corrections fixed overly long headings, code paths, an oversized probability display and root-finder line wrapping; mathematical content and source chapter originals remained unchanged.

The final independent supplement-proof LuaLaTeX compilation returned success, with **only two remaining overfull boxes (3.33302 pt and 0.92885 pt)** in the 68-page proof, neither above 10 pt. No missing-character warnings occurred in this proof. This does **not** establish production font quality or full-book layout perfection.

## 4. Native candidate build now exists without overwriting other books

The shared `LaTeX/physical-parts.json` still references generation `.generation-wob4r3zk`, older than the currently selected `.generation-n3azv4es`. Calling `build.py --build-physical-parts` naively against that stale map would reject generation mismatch. Updating the global seven-volume map for a one-book release would create unnecessary collateral changes.

The independent `LaTeX/Tools/book02_release_candidates.py` instead:

1. Calls the frozen source-lock audit.
2. Constructs an isolated build directory under `LaTeX/Build/<generation>/book02-publication-candidates/`.
3. Runs the working-copy-only Book Two addition injector and source-driven figure generator for P01 and P02.
4. Invokes the production LuaLaTeX runtime and performs its TeX recorder original-source check for both physical books.
5. Requires each physical book to have between 1 and 800 pages, checks the public title, copies the candidate PDFs and records SHA256, compile logs, warnings and page counts.
6. Uses the existing `build_epubs.py --stage 02-Mathematics-for-Visual-Computing` for the native full-source stage EPUB, then checks EPUBCheck success, 35 actual supplementary lesson records, original chapter count and SVG/alt coverage.
7. Creates a report file without rewriting `CURRENT.json`, `physical-parts.json`, the original generation or unrelated books.

The full EPUB step **still requires the user's macOS source-cache `build_books.py`, the original SHA-locked ASTs, installed Pandoc/Java and pinned EPUBCheck 5.4.0**. The connected GitHub runner does not have this complete private source-cache environment, so an end-to-end full-book KDP build is **not** being claimed as run.

## 5. Running exact release-candidate checks on the author's Mac

From the Project_008_AllMight working-directory root, on branch `curriculum/book02-mathematics-refinement-pass01`, after pulling latest changes:

```bash
python3 -m pip install -r tools/labs/requirements-book02-atlas.txt pypdf
python3 "Book Production/AllMight Codex/LaTeX/Tools/book02_source_preflight.py"
python3 "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py"
```

The PDF-only and EPUB-only forms are:

```bash
python3 "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" --print-only
python3 "Book Production/AllMight Codex/LaTeX/Tools/book02_release_candidates.py" --epub-only
```

Use the same Python environment that has the repository's original production dependencies. For official fonts, TeX Live and external paths, use the author's pinned macOS environment. Do not upload proof artefacts to KDP as though they contain the entire book.

## 6. What still prevents final KDP release acceptance

- Run both full production **physical** Book Two PDFs on the author's Mac and inspect the actual print trim, embedded fonts, caption placement, image sizes, readability, orphan/widow lines and overfull warnings in the complete 16-chapter publications.
- Run the complete frozen-AST Book Two stage EPUB with 35 additions, validate the actual EPUBCheck result and inspect reflowable text, code, links, MathML and all 12 SVGs in at least two real EPUB/Kindle reader configurations.
- Complete a substantive full-chapter mathematical and editorial review of the **original 1.75 MB text**, verify all external paper claims against original sources and ensure all exercises have correct solutions. Earlier source audits were selective, not exhaustive proof review.
- Confirm final book/publication metadata, cover and KDP preview compatibility after the actual full books are compiled. The proof PDFs/EPUB are only supplemental integration artifacts.
- If the original TeX and EPUB production disagree in reading order or mathematical semantics, repair source mappings or verified errors, never delete material merely to make a validator pass.

**Disposition:** Pass 4 has achieved reproducible source preservation, 35-section EPUB semantic parity, EPUBCheck proof, 12 accessible SVGs, standalone 68-page print proof, and an isolated full-book candidate build script. **Final KDP upload approval is withheld until the complete native physical books and full stage EPUB pass author-environment QA.**
