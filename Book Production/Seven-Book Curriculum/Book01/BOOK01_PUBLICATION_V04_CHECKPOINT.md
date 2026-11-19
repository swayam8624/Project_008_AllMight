# Book 1 / Publication Proof v0.4 — source-preserving checkpoint

**Status:** committed source improvements; not a declaration that new PDFs have passed visual or technical proof. This document takes precedence over earlier four-part diagnostic build instructions.

## What the academic edition includes

The continuous digital edition draws on sixteen original Computing Foundations chapter bodies, eighteen exactly reversible additive C++23 project lessons, an integrative mastery chapter, the distinct academic front matter and back matter, and sixteen chapter-specific supplemental investigations placed alongside their relevant chapters. The investigations include the bit-to-CPU bridge; modern C++23 contracts; CPU instruction execution; GPU coalescing and scan; borrowing and resource lifetimes; asynchronous execution; Vulkan resources and hazards; Metal execution; and evidence-based performance analysis.

Every one of the sixteen chapters also concludes with a **Reconstruct, Falsify, Transfer** exercise. These are compact learning checkpoints, not a claim that reading alone verifies a program.

The original 39-chapter master manuscript and the chapter-edition source text are not rewritten by this publisher. Reader-only editorial adjustments, source-index entries, Markdown fence normalization, mathematical protection, figure rendering, and imposed print chapters are generated into the ignored build directory, with source hashes in the manifest.

## Editorial and technical changes

- The book's academic typography is restrained: a Times-family text face, legible monospaced listings, grayscale-safe distinctions, shortened chapter running heads, and one consistent page-number system.
- The two paperbacks carry explicit Volume I and Volume II cover identities within the common *Foundations of Computing Systems* series.
- Source diagrams are required artifacts, never silently converted to source-code listings. The new visual investigations add CPU, GPU, Vulkan and Metal diagrams on top of the existing author-drawn figures.
- The **C++23 Companion** contains four independently testable command-line models (a bounded binary CPU, generational handles, stable scan/scatter, and resource retirement) with a CMake/CTest project. Their source is included in the repository and mapped into the book's academic back matter.
- **GitHub Actions:** the `.github/workflows/book01-quality.yml` workflow runs unit tests, provenance preparation and C++ companion builds on branch pushes. A workflow file existing is not a successful run; check its actual Actions status.
- Repeated decorative rules and development-only scaffolding are normalized in the derived reader. The original prose is retained in the canonical source; justified omissions of author-directed instructions are recorded as editorial receipts in the manifest.
- Private Obsidian links become readable labels instead of invalid public hyperlinks. A source catalogue and primary standards references are in the back matter.
- C++ illustrations that intentionally use non-compilable algorithmic syntax are explicitly labelled pseudocode. Long compiler commands are emitted as shell listings to keep their spaces and flags copyable.
- Display math is source-hash protected before Pandoc and restored after conversion; PDF production stops when required expressions or graphics disappear. Actual mathematical correctness still requires independent review.
- The reading edition includes the glossary, indexed key terms, list of figures, source/copyright information, and fuller laboratory record instructions. A true page-level index is generated with makeindex.
- The cover and imprint identify this as publication proof v0.4, not a published ISBN-bearing edition.

## Build on macOS

Run from the book's Git checkout. If Node and Mermaid are already correctly installed, there is no need to install them again. Because older node@22 installs may shadow the working unversioned Homebrew Node, establish the PATH first.

~~~bash
cd "$HOME/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master"
git pull --ff-only origin curriculum/cpp23-nes-project-pass01
export PATH="$(brew --prefix node)/bin:/Library/TeX/texbin:$PATH"
rehash
node --version
npm --version
mmdc --version
pandoc --version
lualatex --version
.venv-book01/bin/python -m pip install pypdf pymupdf
.venv-book01/bin/python -m py_compile tools/book01_render_support.py tools/book01_publication_edit.py tools/book01_learning_gates.py tools/book01_pdf_proof.py tools/book01_index.py tools/build_book01_expanded_reader.py
.venv-book01/bin/python tools/test_book01_render_support.py &&
.venv-book01/bin/python tools/test_book01_publication_edit.py &&
.venv-book01/bin/python tools/build_book01_expanded_reader.py --check &&
.venv-book01/bin/python tools/build_book01_expanded_reader.py --prepare &&
.venv-book01/bin/python tools/build_book01_expanded_reader.py --verify-prepared &&
.venv-book01/bin/python tools/build_book01_expanded_reader.py --compile --open
.venv-book01/bin/python tools/build_book01_expanded_reader.py --compile-paperback --open
~~~

If a build fails, inspect the first error; do not skip subsequent checks or reuse an old PDF with the expected release name.

## Output artifacts and acceptance tests

The digital book is prepared under:

`build/book01-expanded-reader/Book01-From-Bits-to-Parallel-Systems-COMPLETE.pdf`

The two 7 × 10-inch black-ink-on-white-paper candidates are:

`build/book01-expanded-reader/Book01-From-Bits-to-Programs-KDP-Volume-01.pdf`
`build/book01-expanded-reader/Book01-From-Programs-to-GPUs-KDP-Volume-02.pdf`

The preparer writes `build/book01-expanded-reader/manifest.json`. Each actual LaTeX build writes `build/publication-proof.json` inside its own edition directory's build subfolder, for example:

`build/book01-expanded-reader/book01-print-v01/build/publication-proof.json`

The proof gate refuses release on a page-size violation, more than 828 paperback pages, missing document metadata, overlapping/cropped running heads, duplicated footer numbers, text beyond the page boundary, code smaller than 8 pt, or fewer included image instances than required source figures. Other possible editorial residue is reported for review.

A successful build is necessary but still not sufficient for publication: manually check representative pages from *every* chapter, actual figure meaning and line labels, tables, code-copy fidelity, mathematical rendering, bibliographic links, index cross-references, grayscale print contrast, spine/cover file, and KDP Print Previewer. PDF/UA tagging and accessible alternative descriptions of diagrams are separate technical work and **are not claimed complete by this edition**.

## Scope limits that must remain visible

The text does not cover every facility or normative paragraph of ISO C++23, and it is not a claim of current verification of any named third-party/private repository. Snapshot examples should be cited with a source revision and treated as examples. New graphics/API details should be checked against the current relevant specifications. The digital PDF may exceed the paperback limit; the two physical interiors must each pass their own 828-page gate. Their final page counts are not known until the updated manuscript is compiled.

An EPUB conversion or public release should happen only after the proven PDF edition has been visually reviewed; raw TeX source is not a substitute for such proof.
