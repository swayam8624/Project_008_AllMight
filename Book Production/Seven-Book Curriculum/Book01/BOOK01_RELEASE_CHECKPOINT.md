# Book 01 — additive manuscript release checkpoint and Mac build

**Revision branch:** `curriculum/cpp23-nes-project-pass01`. **Date:** 19 November 2026.

This is the **final Book 1 C++23 foundations curriculum authoring checkpoint**, consisting of the preserved first sixteen *Computing Foundations* chapters; eighteen substantial named, precisely located, source-reversible C++23 project lessons; one final four-route integrative mastery chapter; a multi-project source/core/header laboratory; and a reproducible, four-part expanded-reader LaTeX source builder.

It is **not** a claim that the entire seven-book program is finished or that all 39 existing original foundational reading chapters have been moved into final seven-volume print masters. The existing seven-stage print-production generation is a separate older snapshot and remains untouched. The expanded-reader release is deliberately independent because the old print generator relies on local cached assets and source paths and predates the eighteen newly authored lessons.

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
