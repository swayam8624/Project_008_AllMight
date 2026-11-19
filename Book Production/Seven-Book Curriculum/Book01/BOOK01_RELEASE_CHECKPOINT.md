# Book 01 — additive manuscript release checkpoint and Mac build

**Revision branch:** `curriculum/cpp23-nes-project-pass01`. **Date:** 19 November 2026.

This is the **final Book 1 C++23 foundations curriculum authoring checkpoint**, consisting of the preserved first sixteen *Computing Foundations* chapters; eighteen substantial named, precisely located, source-reversible C++23 project lessons; one final four-route integrative mastery chapter; a multi-project source/core/header laboratory; and a reproducible, four-part expanded-reader LaTeX source builder.

It is **not** a claim that the entire seven-book program is finished or that all 39 existing original foundational reading chapters have been moved into final seven-volume print masters. The existing seven-stage print-production generation is a separate older snapshot and remains untouched. The expanded-reader release is deliberately independent because the old print generator relies on local cached assets and source paths and predates the eighteen newly authored lessons.

## Current academic production pipeline — 19 November 2026

**This section supersedes earlier PDF/format instructions below.** The
source-preserving authoring checkpoint remains the same: 16 original reading
chapters, 18 reversible project additions and one final integrative laboratory.
The new academic publishing layer is explicitly separate from the canonical
Markdown.

### Changes and correctness gates

- Main scholarly title: **From Bits to Parallel Systems**; subtitle:
  **C++23, CPU Execution, GPU Memory, and Reliable Software**.
- One continuous digital PDF with a single cover, unified table of contents,
  substantive academic preface and renamed chapter headings.
- Two separate paperback interiors, printed from the same exact chapter
  fragments: **From Bits to Programs** (Chapters 1–8) and
  **From Programs to GPUs** (Chapters 9–16).
- The print trim is **7 × 10 inches** and page checks enforce a maximum of
  **828 pages per volume** for KDP black ink on white paper, with a wide
  inside margin. The digital PDF is not artificially truncated at that limit.
- Original fenced Mermaid diagrams are transformed into PNG figures before
  LaTeX compilation. The build refuses to proceed if a diagram cannot render,
  a token is lost, a source figure changes, or the rendered math appears to
  lose an authored boxed expression. Fenced ASCII and source-code diagrams
  remain readable as listings.
- Readable Times-family scholarly typography, compact display math, and
  grayscale-safe code/definition boxes. Chapter-opening generic study prompts
  and blank journal pages are omitted from the reader, but no source text
  is deleted. Pages are not padded with filler.
- Repetitive Markdown thematic breaks are converted **only in the derived reader**
  to compact run-in concept headings or restrained spacing. These conversion
  counts are recorded in the manifest; the original authored Markdown stays intact.
- Every original display formula must survive the Pandoc conversion with its
  complete mathematical content, not merely a matching number of equations.
- The EPUB is **not generated** until the PDF content and typography are
  reviewed and approved.

### Exact Mac commands for the refreshed edition

```bash
cd ~/Library/Mobile\ Documents/iCloud~md~obsidian/Documents/Master
git pull --ff-only origin curriculum/cpp23-nes-project-pass01
export PATH="/Library/TeX/texbin:$PATH"

# Mermaid's command-line diagram renderer requires Node.
brew install node
npm install -g @mermaid-js/mermaid-cli
mmdc --version

# Isolate the Python PDF audit dependency.
python3 -m venv .venv-book01
.venv-book01/bin/python -m pip install pypdf

# Syntax, diagram/mathematics regression, and exact-source preparation.
.venv-book01/bin/python -m py_compile tools/book01_render_support.py tools/book01_editorial.py tools/build_book01_expanded_reader.py
.venv-book01/bin/python tools/test_book01_render_support.py
.venv-book01/bin/python tools/build_book01_expanded_reader.py --prepare
.venv-book01/bin/python tools/build_book01_expanded_reader.py --verify-prepared

# The complete digital textbook with all sixteen selected reading chapters.
.venv-book01/bin/python tools/build_book01_expanded_reader.py --compile --open

# KDP paperback interiors with strict size and maximum-page validation.
.venv-book01/bin/python tools/build_book01_expanded_reader.py --compile-paperback --open
```

Digital output:
`build/book01-expanded-reader/Book01-From-Bits-to-Parallel-Systems-COMPLETE.pdf`

Paperback outputs:
`build/book01-expanded-reader/Book01-From-Bits-to-Programs-KDP-Volume-01.pdf`
and `Book01-From-Programs-to-GPUs-KDP-Volume-02.pdf`.

The four `--compile --part N` partitions remain optional diagnostics for
isolating TeX failures; they are not four published books. If the code refuses
a release because a formula, diagram, trim size or paperback page count is
wrong, **fix the input/build problem and rebuild**, rather than dropping
material or suppressing the check.

**Independent validation still required:** local LuaLaTeX compilation,
manual inspection of figures and equations in the actual PDF, source-to-TeX
comparison of a sample from every chapter, page and gutter inspection,
KDP Print Previewer approval, and a proper C++23 language-coverage audit.
The present Book 1 edition is not asserted to include all 39 foundation
chapter-edition chapters or to cover the full ISO C++23 specification.

---

## Academic reader-edition update — 19 November 2026

**Current release behavior supersedes the older four-part presentation instructions below.**
The default `python3 tools/build_book01_expanded_reader.py --compile --open`
now compiles **one continuous LaTeX book** in
`build/book01-expanded-reader/book01-complete/main.tex`, creating
`Book01-From-Bits-to-Parallel-Systems-COMPLETE.pdf` with one title page,
one full table of contents, continuous page numbering and chapter bookmarks.
The four `book01-expanded-p01` through `p04` directories remain only
for `--compile --part N` diagnostics and do not represent four books.

The new style file, `Book01/book01-reader.sty`, sets Times-like 10 pt
academic body text, 8.5 pt monospace listings, understated 19 pt chapter
headings, clean marginal and running-header typography, and grayscale-safe
neutral code/note boxes with restrained accents. The formerly repeated
generic chapter prefaces and empty notes-page inserts have been removed
from the generated edition; **no original source paragraph was removed**.
The common seven-book stylesheet remains untouched.

**Scope:** the source-pinned edition currently includes exactly 16
long-form original computing chapters, 18 additive C++23 laboratory lessons
and the final capstone. The Computing Foundations reading map has 39
chapters; inclusion of the other 23 chapters and any claim of *complete*
C++23 curriculum coverage require separate verified routing, editing and
coverage gates. Do not market this checkpoint as an entire 39-chapter or
fully comprehensive C++ reference. A two-volume physical printing can
be decided after measuring a successfully compiled and visually inspected
single-volume PDF; it must not duplicate cover/contents four times.

From the repository root:

```bash
git pull --ff-only origin curriculum/cpp23-nes-project-pass01
export PATH="/Library/TeX/texbin:$PATH"
python3 -m py_compile tools/build_book01_expanded_reader.py
python3 tools/build_book01_expanded_reader.py --prepare
python3 tools/build_book01_expanded_reader.py --verify-prepared
python3 tools/build_book01_expanded_reader.py --compile --open
```

The single TeX master has not yet been compiled or inspected as a finished
PDF by the assistant. If a complete-volume build fails, the `--part N`
option isolates the source-only diagnostic partitions. This print design
is **not** an EPUB template; a reflowable EPUB remains gated on PDF approval.

---

## What was completed

The new lesson material is staged in `Supplementary/C++23 Project Labs/` and inserted *before its actual original teaching heading*, never in a two-page replacement chapter. The original continuous master remains present and unchanged on the GitHub branch. The insertion utility can optionally apply the same additions to that long-form master on the user's machine; repeated application is idempotent. It compares exact marker-delimited contents, rejects partially edited prior insertions, and now **detects even overlapping occurrences of the same original heading**. Reversing only the eighteen marked additions yields the original source bytes. This is stricter than counting lesson titles or pages.

The source-level Book1 reader selects the complete original teaching bodies from the sixteen authored *Chapter Edition/01 - Computing Foundations* reading chapters, preserves their original bytes, and adds the lessons in narrative context. `CPP-18A` correctly lives beside Chapter06 asset import and registries; `CPP-19E` belongs in Chapter10 reproducible pipelines, not at the end of Chapter16. The editor/ownership/callback/GPU explanations remain at their assigned first-use locations.

The final chapter in `FINAL_CAPSTONE_AND_MASTERY_GATE.md` asks the learner to follow the same trustworthy state/evidence ideas through four different applications: a bounded NES debugger, Kairo/Maveb asset/scene publication, Kyoto exact-pair collision provenance, and a synthetic XR gaze-to-render workflow. It contains derivations, commented executable-style C++23 examples, failure cases, proof obligations, source identity, numerical oracles and a constrained oral-defense rubric. It does not claim that any of those **downstream production projects** has been fully implemented or independently reproduced by the curriculum labs.

The expanded reader's `--check` and `--prepare` do not mutate the original continuous master. They generate a new reading set in `build/book01-expanded-reader/`, with sixteen chapter text/TeX fragments, four complete `main.tex` files, an additional final `final-mastery.tex` chapter, and a source-pinned `manifest.json`. The implementation rejects a changed canonical master whose unchanged preimage no longer matches the known original Git blob: stale derived chapters must never silently pass source-fidelity checks. Generated artifacts are excluded from Git source control.

## Actual evidence, with narrow interpretations

The **whole Book01 C++23 suite** on GitHub Actions [run 44476138886](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44476138886), source commit `be6df3bac2eeb4cfb9055d359530ad61641b49d5`, completed successfully: GCC Debug, Clang19 C++23, macOS AppleClang/Xcode26, and Clang19 ASan+UBSan, each with **18/18 CTests** and source-identity receipts. The GCC workflow additionally ran its Python tests, exact source insertion dry-run, original-book structural audit, and source-pinned collection/verification. Native Vulkan or Metal probes remain explicit opt-ins and are **not** asserted to have been executed merely because their source was compiled.

The new **expanded-reader source-preparation workflow** is [run 44487625396](https://github.com/swayam8624/Project_008_AllMight/actions/runs/44487625396): source check 16/18 passed, six new negative and preservation fixtures passed, and Pandoc successfully generated the four complete TeX masters and the final mastery chapter. Its source-only artifact is named `book01-expanded-tex-sources-and-source-manifest`. That workflow intentionally does **not** invoke LuaLaTeX or render/inspect PDFs. The user's local compilation and actual typography/diagram/link review are separate final print gates.

A source-only validation proves that no passage in the **derived reading body** was dropped by the assembler; it does **not** prove perfect paragraph flow, correct figures, TeX font installation, or that all external publications/YouTube sources have individually been exhaustively audited. The completion claim is confined to the described original sixteen chapters, additive lessons, final exercise, and tested portable project contracts.

## One consolidated Mac checkout, verification, source insertion, and PDF build

Run from the local repository root. The example location below is only a path example; the Obsidian iCloud vault may be the actual repo instead. If local changes are already present, review `git status` and commit or intentionally preserve them **before** switching branches/pulling. Do **not** use `git reset --hard`, `git clean -fd`, or a destructive stash operation to make the checkout “clean”.

```bash
# Replace with the actual repository directory if different.
cd "$HOME/Desktop/Programming/Project_008_AllMight"

git status --short
git fetch origin
git switch curriculum/cpp23-nes-project-pass01
git pull --ff-only origin curriculum/cpp23-nes-project-pass01

# Pandoc converts the ORIGINAL complete body + additions to LuaLaTeX.
# CMake/Ninja build and test the executable teaching labs.
brew install pandoc cmake ninja

# Use an installed MacTeX/TeX Live distribution; MacTeX provides LuaLaTeX
# and latexmk. Install it separately ONLY if these commands are missing.
export PATH="/Library/TeX/texbin:$PATH"
pandoc --version | head -n 1
lualatex --version | head -n 1
latexmk -v | head -n 2

# Read-only audit, then apply only the missing exact additions to the
# original 3.5 MB source. This intentionally leaves a local Git diff.
python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/apply_cpp23_project_inserts.py --apply
python3 tools/apply_cpp23_project_inserts.py --check
python3 tools/book01_coverage_audit.py --allow-applied
git diff --check

# Verify C++23 target environment (Xcode 26 AppleClang preferred).
cmake -S "Book Production/Seven-Book Curriculum/Book01" \
  -B build/book01-cpp -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/book01-cpp --parallel 4
ctest --test-dir build/book01-cpp --output-on-failure

# Independent new reader preservation fixtures.
python3 -m unittest discover -s tools/tests \
  -p 'test_book01_expanded_reader.py' -v

# Verify 16 source chapters, 18 uniquely located lessons and final chapter.
python3 tools/build_book01_expanded_reader.py --check
python3 tools/build_book01_expanded_reader.py --prepare
python3 tools/build_book01_expanded_reader.py --verify-prepared

# This command, not CI, finally invokes LuaLaTeX on all four PDF parts.
python3 tools/build_book01_expanded_reader.py --compile
```

**If `lualatex` or `latexmk` is missing:** install the full MacTeX distribution using `brew install --cask mactex` (large download) and reopen the shell or run `export PATH="/Library/TeX/texbin:$PATH"`. The TeX build requires the committed `allmight-book.sty` and its existing local font assets; do not copy, distribute, or invent font files. A newer compiler/standard library may be required for the C++23 `std::expected`, `std::jthread`, and `std::stop_token` labs. The tested AppleClang runner uses Xcode26. The reader preparation uses Python's standard library plus Pandoc and the existing repository; it never requires the old converter's private cached `build_books` module.

**Four outputs** after `--compile`:

```text
build/book01-expanded-reader/Book01-Bits-Programs-Reliable-Systems-P01.pdf
build/book01-expanded-reader/Book01-Bits-Programs-Reliable-Systems-P02.pdf
build/book01-expanded-reader/Book01-Bits-Programs-Reliable-Systems-P03.pdf
build/book01-expanded-reader/Book01-Bits-Programs-Reliable-Systems-P04.pdf
```

Each part's actual LaTeX master and logs are under `build/book01-expanded-reader/book01-expanded-pXX/main.tex` and `build/book01-expanded-reader/book01-expanded-pXX/build/main.log`. The source and hash manifest is `build/book01-expanded-reader/manifest.json`. If one part fails, the script stops on that part with an explicit log path. Fix the actual conversion or TeX issue; do not bypass the preservation gate or discard original teaching material.

The `--compile --part 2` option builds only part 2 after preparation and validation. After all four PDFs exist, inspect table widths, equation legibility, graphic and code-fence rendering, author/source links, and continuation between adjacent physical parts. Passing compilation is necessary but **not sufficient** for a polished print release.

## Explicit remaining boundaries

- The long original master is updated **only when you run** the additive `--apply` command on your own checkout. The repository branch's original long file was deliberately kept intact; the expanded reader makes the full lesson additions visible without committing a destructive master rewrite.
- The existing production team's old seven-stage LaTeX generations and physical books are separately preserved and are **not** automatically replaced by the four new reader PDFs. Their own print/EPUB release depends on rebuilding the larger pipeline and its supporting assets; do not treat old snapshots as newly revised.
- No full native Vulkan/Metal GPU device experiment, commercial NES ROM compatibility suite, ThreadSanitizer run, Kyoto research benchmark, XR latency benchmark, or exhaustive review of every supplied external lecture/book is certified by the present Book01 CI.
- Book02 through Book07 remain independent curriculum authoring and verification tracks; subsequent books should reuse these contracts rather than duplicating their entire text.
