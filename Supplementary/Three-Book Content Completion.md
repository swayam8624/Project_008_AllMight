---
tags: [books, curriculum, verification, source-closure]
status: in-progress
updated: 2026-11-15
---
# Three-book content completion

The working books are [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|Computing Foundations]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Graphics, Vision and XR]], and [[Continuous Notes/Game Engine - From First Frame to Living Worlds|Game Engine]]. Their [[Chapter Edition/README|chapter reading copies]] preserve the content in 129 major chapters. Design and publication are deferred.

**The programme is not certified complete.** A missing audit does not mean a topic is absent. A heading, retained transcript, reviewed extracted claim or small working lab does not prove a whole source or chapter complete. The [[Supplementary/PROJECTALLMIGHT - Source Closure and Narrative Quality Contract|source and narrative contract]] governs these distinctions.

## Recorded evidence

| Layer | Current evidence | Boundary |
|---|---|---|
| Curriculum | Exactly 3,000 original parts, 180 chunks and 30 phases; titles/numbering retained in tools/book_completion/canonical_curriculum.json. | Source documents supply data, not new task instructions. |
| Actual-passage review | **1,820 parts** have matched quotations, hashes and bounded assessments. Full audits: Phases 4–11, 15–22 and 25. Partial: Parts 1–80 and 1301–1340. | Not full chapter/source/exercise/implementation signoff. |
| Final content acceptance | **4 parts**, each with nine reasoned gates and root acceptance: 338, 409, 410, 1585. | The other 2,996 parts await final review; this does not declare their existing content absent. |
| Source agreement | **34 declared complete programs and 9 CMake files** match full inline sources and comments. | Not compilation of every other fragment. |
| Chapter copies | **3 books, 129 chapters, 134 generated files**. | No measured 50-page PDF pagination. |
| Prior claim-text review | **29,715 reviewed texts, zero pending** in the preceding ledger. | Not exhaustion of underlying audio, pages, figures or current curriculum. |
| Older processing queue | **103 open jobs**, all with retained extracted unit files. | Open processing is not necessarily a missing download; final semantic disposition remains open. |
| Seven mathematics books | **4,205 physical PDF pages** inventoried. TOCs/prefaces and selected material inspected; Riley §24.13, physical 891–900, fully reviewed for the contour lesson. | Not a full-page review of all books. |

Source material and acquisition code remain retained: their conditional deletion gate has not been met.

## Additive teaching

Twenty root-reviewed sections are recorded in the integration checkpoint. They include three cumulative project ladders; first-contact response; a bounded Kohi scheduler reconstruction; source-pinned Gaussian training and optimizer/topology state; a worked residue calculation; joint velocity rows; sensor/filter/event lifetimes; captured-world identity; imported/alias CMake targets with all nine project files; shader-import transactions; packed lane arithmetic; recurrence analysis; and deeper GPU data structures, ownership and measurement.

The GPU data-structure lesson develops attempted versus stored append counts, visible capacity failure, signed bins, count→scan→scatter, deterministic alternatives, fixed-table probing, duplicates and build-to-query publication. A complete sequential C++ oracle checks examples and failures; it is not a GPU race or performance test.

The ownership lesson develops a CPU/GPU timeline, queue-overlap bounds, command submission versus completion, transfer units, persistent mapping, non-coherent visibility, ring retirement, readback, suballocation and aliasing. The measurement lesson derives roofline bounds, streaming versus reuse, occupancy tradeoffs, queries, fast-math contracts, binary16 and collective matrix concepts. A scalar numerical witness checks their worked calculations.

The sparse-set lesson now documents a reproduced limitation: conditional noexcept does not make two-array mutation transactional. A reader tool extracts the actual implementation and checks normal removal plus the expected throwing-component counterexample. The shown removal strategy needs a non-throwing component contract; this is not presented as a fixed unrestricted generic container.

The existing Phase 4, Phase 5, Graphics and Engine recall files have new short prompts and links to these explanations. Prior lessons and examples are retained. Documented TeX/subscript, delimiter and display-whitespace corrections change formatting rather than remove teaching; exact before/after records remain available.

## Executed local checks

- python3 tools/check_book_completion.py — numbering, quote/hash and acceptance schema pass; content_complete remains false.
- python3 -m unittest discover -s tools/tests -p test_book_completion.py — 7 tests pass.
- python3 -m unittest discover -s tools/tests -p test_note_integrity.py — 12 tests pass.
- python3 -m unittest discover -s tools/tests -p test_chapter_edition.py — 8 tests pass.
- python3 tools/check_continuous_code_sync.py — 34 declared programs and nine project sources agree.
- python3 tools/check_three_volume_consolidation.py — prior teaching and documented XR relocation preserved under explicit correction rules.
- python3 tools/build_chapter_edition.py and its --check mode — generation/current copies pass.
- python3 tools/check_vault_links.py --json — local note, heading and asset navigation pass.
- python3 tools/check_patch_idempotence.py — retained patches pass in isolated copies.
- git diff --check — passes after final regeneration.
- MathJax renders **3,293 distinct new/changed expressions, zero syntax errors**. This is not independent algebra proof or native whole-book Obsidian review.

Gaussian training passes normally and under Python -O, including 240 finite differences. Planar joints pass both modes, including 360 finite differences. Shader import and GPU budget/precision each pass six tests in both modes. Filter/events pass 18 strict-release and sanitizer checks. Packed arithmetic passes strict release. GPU structures pass strict release and AddressSanitizer/UndefinedBehaviorSanitizer. Sparse-set extraction compiles with C++20/warnings as errors and reproduces the intended normal behavior and expected exception-safety failure. The imported/alias project passes a positive host build/run and the expected missing-target negative configuration.

The CI workflow now includes the completion-schema checks and added bounded witnesses. No successful hosted run is claimed for these local changes.

## Remaining acceptance work

Continue unreviewed ranges and the nine-gate reviews: actual definitions, derivations, traces, failures, alternatives and solved exercises must support each required skill. A shared exercise can support several parts only when it exercises them.

Open source units need inspection and explicit disposition: taught with a verified destination, added, corrected, duplicate with evidence, or outside the finite programme. Original source receipt remains separate from independently reconstructed teaching, especially for the missing original 1701–1800 lecture receipt.

Vision/research extensions need a finite acceptance scope. Complete chapter reading, capstone reconstruction, numerical review, representative backend/device evidence and independent learner teach-back remain programme-level gates. Layout and publication follow later.

The smaller workers hit their service usage limit during partial Phase 1 and Phase 14 audits. Checkpoints are retained; unfinished ranges are not counted as reviewed. Root completed the subsequent additions and checks.

Resume authority: /Users/swayamsingal/.codex/source-cache/book-completion-2026-11-14/WORK_STATE.json. Books and chapter copies remain local for human diff inspection. This pass did not commit, push or open a pull request.
