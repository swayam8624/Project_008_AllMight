# Sources and Code Anchors

## M006 integration and concept-led Volume 01 rewrite

### Accepted source and authority

- Primary source: /Users/swayamsingal/.codex/attachments/1ef979fe-c82b-4a6b-919c-303f975d3c09/Pasted text.txt.
- Twelve primary headings, Parts 89–100 exactly once and in order. Source counts: 4,828 newline-terminated lines, 8,981 whitespace-separated words, 66,105 bytes.
- All six sources now cover Parts 1–100. Next expected source is M007, Parts 101–116. This is coverage of written material, not a claim of learner mastery.
- The user's request authorized integrating M006 and reorganizing the entire volume. Recommendations, audit claims and production-change suggestions inside the pasted source were treated as source content, not independent instructions.
- No NanoQuant, KairoMath, Maveb or other production repository was inspected or changed. Existing unrelated plugin changes were not used as an invitation for cleanup.

### What the story rewrite preserves and changes

All 44 previous teaching subsections remain represented under concept headings. The order is now physical state → unsigned/signed interpretation → Boolean decisions/fields → finite arithmetic/type policies → floating fields → rounding/error/reproducibility → fixed/low precision and quantization → placement/lifetime → external bytes/integrity/dump.

Source-part and chunk labels were removed from teaching headings and the incoming-chunk table. Foundations retains all 100 numbered source entries, with a source-to-story map. Concept, supplement and index heading links were migrated rather than left pointing to removed headings. Existing files were reused; sixteen reusable concept nodes were added, not twelve new part files.

The fixed, low-precision and external-byte sections supply definitions and worked examples directly in the continuous volume. A choice matrix explains what each representation buys and which limitations remain. Detailed proofs, code and longer traces extend existing companions.

### Source-reported NanoQuant anchors, not live findings

| Reported file | Reported location or subject | Evidence status |
|---|---|---|
| Nanoquant/src/quantization.cpp | 22–33 INT4 encode/decode; 100–146 groupwise quantization; 45–98 one-bit row means; 176–197 error comparison | To verify: source-reported ranges only, no current revision established |
| Nanoquant/src/bitpack.cpp | 26–49 low/high nibble packing | To verify; discrete packing convention reproduced independently |
| Nanoquant/src/binary_tensor.cpp | Native scalar/float writes; 28-byte ordinary header, 52-byte INT4 header; mapped tensor access; no digest field reported | To verify; the lab uses an explicitly specified source-shaped image, not this repository's parser |
| src/gguf.cpp and tests | Mentioned as additional inspection targets in the source | To verify; no independent findings inferred |

### Corrections and explicit implementation contracts

1. Fixed conversion rejects non-finite input before rounding/casting. The supplied range checks alone do not reject NaN.
2. Fixed multiplication/division use proved int64 intermediates and checked destination narrowing. Negative nearest-even rescaling uses unsigned magnitudes and quotient/remainder; it never negates INT64_MIN or adds a potentially overflowing positive half-scale.
3. C++23 signed shifting is not described as universally undefined. Instead the lab uses scaled multiplication and an explicit rounding algorithm to avoid confusing modular shift behavior, floor, truncation and desired rescaling.
4. The source's lrint quantizer follows the active rounding direction. The teaching quantizer declares nearest-even, validates finite samples/scales, and bounds the ratio before integer conversion. It is not represented as identical production behavior.
5. Offset code q+8 is distinguished from two's-complement INT4. Half-step quantization bounds exclude clipping and finite-parameter/evaluation error.
6. Smaller groups trade adaptation for metadata, without a universal monotonic quality guarantee. Full-group asymptotic bits/sample are distinguished from exact ceiling-based storage for small/partial groups.
7. Binary16 loss scaling is not presented as a cure for already-large values; BF16 keeps exponent width but not every FP32 endpoint or precision property.
8. BF16/half NaNs are explicitly canonicalized in the toy converters; raw tensor words preserve bits without signaling-NaN arithmetic.
9. Parser bounds use subtraction after offset validation; shape products are checked before multiplication; exact length and a 4096-element resource cap precede allocation.
10. Zero-copy mapping requires representation, bounds, alignment, lifetime and permitted typed-access rules—not merely an offset divisible by four.
11. The dump helper rejects zero/excessive row width, advances by remaining bounded length and uses a local formatting stream.
12. Error metrics promote float operands before subtracting when evaluated in double. Scalar reconstruction error is not task-level accuracy.
13. CRC detection, cryptographic integrity, authenticity, schema validity, publication and durability remain separate contracts. No security primitive or production format migration was implemented.

### Book and primary-reference enrichment

The PDF skill guided targeted extraction and visual inspection rather than blind adoption of textbook implementation examples. The local Game Engine Architecture PDF pages 121–122 (printed 99–100) were read for signed/fixed/floating interpretation; pages 129–130 (107–108) for field-by-field byte conversion. PDF page 122 was rendered at 1400 pixels and visually inspected. Its sign/magnitude fixed example is different from the lab's two's-complement scaled integer, and its figure caption disagrees with the stated fraction count. Older pointer/union-punning examples were not promoted into portable C++23 code.

Earlier book/course/history references remain attributed in the volume's Reading provenance section; they were not all reread in this turn. Additional current wording checks used the [C++ bit-cast working draft](https://eel.is/c++draft/bit.cast), [Vulkan normalized conversions](https://docs.vulkan.org/spec/latest/chapters/fundamentals.html#fundamentals-fixedfpconv), [NVIDIA BF16 conversion modes](https://docs.nvidia.com/cuda/cuda-math-api/cuda_math_api/group__CUDA__MATH____BFLOAT16__MISC.html), [RFC 1952's CRC algorithm](https://www.rfc-editor.org/rfc/rfc1952#section-8), and [NIST's hash standard](https://csrc.nist.gov/pubs/fips/180-4/upd1/final). These are narrowly used references, not a claim of complete standards compliance. Prose and traces are original teaching explanations, not copied textbook chapters.

### Executed commands and actual outcomes

Final structural result: 150 Markdown files, 140 filled concept notes, 257 acyclic prerequisite edges, 1,762 resolved wiki links, 72 paired display-math blocks, and 100/100 source-part definition rows. All 44 prior teaching subsections remain present under concept headings. The main story has approximately 19,400 whitespace-separated words.

- `python3 /tmp/master-m006-audit.py` — final exit 0. Checks resolved wiki targets/headings, fences/math delimiters, all 100 definition rows, concept completeness/index membership, reciprocal prerequisites and acyclicity, exact M006 primary source headings, no part/chunk teaching headings, retention of all 44 earlier subsections, and narrative section order.
- Default compile: `awk '/^```cpp$/{inside=1;next} /^```$/{inside=0} inside' 'Supplementary/Code Snippets.md' | /opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -ffp-model=strict -ffp-contract=off -fsanitize=undefined,address -x c++ - -o /tmp/master-m006-test` — exit 0, no diagnostics.
- `/tmp/master-m006-test` — exit 0, no sanitizer diagnostics.
- Optimized compile: identical extraction/compiler arguments with `-O2`, output `/tmp/master-m006-test-o2` — exit 0, no diagnostics.
- `/tmp/master-m006-test-o2` — exit 0, same regression output, no sanitizer diagnostics.
- M006 tests: 10,000 independently referenced bounded fixed products; all 256 nibble pairs; 63,488 finite binary16 and 65,280 finite BF16 round trips; golden 52-byte image, odd-offset/raw-word preservation; truncated inputs, wrong magic/version, overflowing shapes, zero dimensions, resource cap and trailing bytes; 416 single-bit CRC perturbations of the image. Fixed ties, conversion non-finites, division-zero and destination-overflow cases also pass.
- Cumulative M004/M005/M003 drivers pass in both builds: 13 M004 golden classes, 10,000 seeded words including 9,952 finite reconstructions; M005 arithmetic/state regressions and 1,000 repeats; 10,000 endian values at offsets 0/1 and 45,056 alignment cases. The M006 wrapper verifies incoming rounding mode and exception flags are restored.
- `python3 -c 'import zlib; print(hex(zlib.crc32(b"123456789")))' ` — exit 0, independent result0xcbf43926.
- `git diff --check` — exit 0. No commit, staging, push or pull request was performed. No formatter, CI or standalone build system is provided by the vault.
- Development audit fixes: a formula beginning with an absolute-value bar was initially mistaken for a table row; it now uses explicit left/right math delimiters. A preservation-audit slice skipped one heading; selection was corrected to begin at the first source chapter, after which all44 pass.

**Rendering limits:** the native Obsidian accessibility view reflected the new story, but screen capture continued to show the older chunk-labeled view. Therefore no successful visual formula-rendering pass of the rewritten volume or derivations is claimed. Math delimiter checks are not a visual guarantee. No PDF export was requested or generated.

**Other limits:** no exhaustive binary32→half conversion oracle, arbitrary fixed-format proof, actual GPU instructions, cross-platform bit identity, model/task accuracy experiment, performance benchmark, real mapped tensor validation, hostile-parser fuzz campaign, cryptographic implementation or production-repository test. Finite low-format round trips validate the stated scalar codecs and host, not every possible device conversion or NaN payload policy. Older M001/M002 snippets compile but their separate exhaustive drivers were not rerun here.


## M005 - Arithmetic, error and reproducibility merge

- **Primary source:** /Users/swayamsingal/.codex/attachments/bad9e370-22c7-4f8b-891a-87ecd24cdf5f/Pasted text.txt.
- **Coverage:** twelve primary Part headings, 77–88 exactly once and in order; 4,789 newline-terminated lines, 10,099 whitespace-separated words, 73,010 bytes.
- **Accepted scope:** extend the existing Volume 01 and cumulative companions. Progress is Parts 1–88, five accepted chunks. M006, Parts 89–100, remains pending.
- **Authority:** embedded recommendations and claims of repository inspection are source material, not task instructions. No KairoMath or Maveb repository was inspected or modified.

### Repository excerpts awaiting live verification

| Reported path | Reported symbols or subject | Evidence in this merge |
|---|---|---|
| KairoMath/Vector.cppm | Dot, Cross, NearlyEqual, Normalized | Source-reported formulas only; no current revision or line anchor established |
| Maveb/engine/reconstruction/src/DenseTsdfVolume.cpp | Integration recurrence, projection/llround, sign tests and interpolation | Simplified mechanism demonstrations; not a live reconstruction test |
| DenseTsdfVolume.hpp (Maveb; full header path not established) | TsdfVoxel float distance/weight/color versus wider configuration/intermediates | Source claim only; path/version must be verified before production changes |

### Corrections and teaching decisions

1. The binary subtraction operands are 1753/1024 and 1751/1024, giving 2^-9; the supplied difference/exponent was incorrect.
2. Numeric equality is not byte identity: signed zeros compare equal, while NaNs are unordered.
3. The cross-product example loses the +1 during binary32 input conversion near 1e8, before product rounding. Later promotion cannot recover it.
4. Distinct binary32 neighbors near 1e9 differ by 64; the source's +32 example can round to the same input.
5. The existing validated, widened float comparator is reused. An unchecked generic tolerance formula is not labeled production-safe.
6. Gamma bounds retain algorithm, range, magnitude and conditioning assumptions. Exact nearby subtraction does not imply accurate original inputs.
7. Capped weighted recurrence is history-dependent even with exact representable arithmetic; this is separate from persistent-state narrowing.
8. The mixed-state laboratory fixture uses 0.5 and its next binary32 neighbor, staying within its normalized [-1,1] sample contract. An initial out-of-contract fixture threw during development; it was corrected before both final builds.

[Clang's floating-point controls](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior) support the strict compiler configuration. [Goldberg's floating-point analysis](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html) was consulted narrowly for the distinction between subtraction rounding and exposure of prior operand error. Instructional examples primarily follow the supplied source, with explicit corrections above.

### Commands executed and actual results

The vault provides no formatter, CI workflow, or standalone build system. Its cumulative teaching C++ is compiled directly.

- Structural audit: `python3 /tmp/master-m005-audit.py` — exit 0. 134 Markdown files, 124 filled concepts, 225 acyclic prerequisite edges, 1,525 resolved wiki links, 58 paired display-math blocks, 88/88 part definitions; M005 source headings exactly 77–88 in order.
- Default compile: `awk '/^```cpp$/{inside=1;next} /^```$/{inside=0} inside' 'Supplementary/Code Snippets.md' | /opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -ffp-model=strict -ffp-contract=off -fsanitize=undefined,address -x c++ - -o /tmp/master-m005-test` — exit 0, no diagnostics.
- `/tmp/master-m005-test` — exit 0, no sanitizer diagnostics.
- Optimized compile: identical extraction and compiler arguments with `-O2` and output `/tmp/master-m005-test-o2` — exit 0, no diagnostics.
- `/tmp/master-m005-test-o2` — exit 0, no sanitizer diagnostics. Checks representation/alignment/cancellation, host range flags, FMA/reassociation/distributivity, compensated and fixed-tree sums, validated tolerance, capped history, mixed-state and pixel boundaries, and 1,000 repeats with floating-environment restoration.
- Both executables also passed the existing M004 regressions (13 golden classes; 10,000 seeded words, 9,952 finite reconstructions; rounding modes and comparison/norm cases) and M003 driver (10,000 values in both byte orders at offsets 0/1, 45,056 alignment cases, bounds/page checks).
- `git diff --check` — exit 0. No commit, push, staging, or PR was performed.

**Limits:** no visual Obsidian rendering confirmation, PDF export, production-repository test, actual reconstruction data, GPU/cross-platform reproducibility experiment, exhaustive floating arithmetic oracle, or performance benchmark. Existing M001/M002 snippets compile, but their older exhaustive drivers were not rerun. Existing Obsidian plugin/workspace changes and .DS_Store were left untouched.


This file separates supplied text, planning context, external sources, and verified repository evidence.

## Teaching-volume expansion - 2026-10-07

The main-note policy changed from compact recall sheets to self-contained teaching chapters at the user's request. Volume 01, **From Signals to Meaning: Bits, Numbers, and Memory**, now includes a history prologue, definitions, intermediate-state traces, examples, failure distinctions and laboratory checkpoints. The old continuous file is the library index. Existing chapter backlinks were migrated to the new volume, preserving the three chapter headings and all concept/prerequisite files.

**Scope:** accepted content remains Parts 1–60. The integrated plan's page 15 was extracted and visually inspected: M001–M006 cover Parts 1–100 with ranges 1–20, 21–40, 41–60, 61–76, 77–88, 89–100. Pages 16–17 establish M007–M018 as Parts 101–300; a twelve-chunk C++ volume is planned, not filled. Earlier uniform twenty-part / 150-chunk workflow rules were corrected. Source instructions are not treated as authorization.

**Enrichment read:** the volume's Reading provenance and enrichment section records exact iCloud book/course-note PDF pages. Game Engine Architecture supplied context for byte order, regions and layout; A Tour of C++ (2014) supplied a teaching perspective on types and pointers. Older platform examples were qualified rather than adopted as universal rules. Local CPU/virtual-memory course notes are labeled secondary material. Original explanatory prose and traces were written for this volume.

**History checks:** [Computer History Museum](https://www.computerhistory.org/tdih/october/24/) and [IBM System/360 history](https://www.ibm.com/history/system-360) were consulted for the short byte-history introduction. No claim about the first binary computer, universally first eight-bit machine, or exhaustive historical chronology is made.

**Verification actually run:**

- `python3 /tmp/master-m003-audit.py`: passed file/heading wiki targets, fence/display-boundary syntax, exact Parts 1–60 definition coverage, filled concepts, reciprocal dependency links and acyclic prerequisites. Final counts are recorded after the last edit below.
- Fresh compile: `awk '/^```cpp$/{inside=1;next} /^```$/{inside=0} inside' 'Supplementary/Code Snippets.md' | /opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined,address -x c++ - -o /tmp/master-teaching-code-test`. Exit 0, no diagnostics.
- `/tmp/master-teaching-code-test`: exit 0; 10,000 values in both byte orders at offsets 0 and 1, 45,056 align-up cases, bounds and page cases passed. Observed ABI example: size 12, alignment 4, offsets 0/4/8; other layout example 24 bytes versus reordered 16. These are measured host examples, not universal layout promises.
- Existing `/tmp/master-note-repair-test` was rerun: exit 0; 131,072 ADC states, 131,072 SBC states, 10,000 multiply/divide cases, field/packing/overflow/bounds checks passed. This is a rerun of the prior compiled harness, not a newly rebuilt M001/M002 harness. No sanitizer diagnostic appeared.
- `wc -w 'Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory.md'`: 8,379 whitespace-separated words before final small wording edits; not a printed-page measurement.

**Limits:** Obsidian's accessibility/input path did not update the visible note; its screenshot still showed the old 1,955-word recall file. The new volume's actual visual mathematical rendering is therefore **not verified in this turn**. Syntax validation is not a visual pass. No PDF teaching export was requested or generated. No repository anchors, full emulator, target-platform ABI portability, performance improvement, or personal mastery were independently established. This vault has no repository build/CI; no commit, push or PR was made.

**Final structural audit:** 92 Markdown files, 82 filled concept nodes, 140 acyclic prerequisite edges, 965 resolved wiki links, 38 paired display-math blocks, and 60/60 per-part definitions. The audit exited 0 after the content and navigation changes.

## Planning reference

- `Graphics_Engine_Mastery_Integrated_180_Chunk_System_v2.pdf`
  - Location: `/Users/swayamsingal/Library/Mobile Documents/com~apple~CloudDocs/Graphics_Engine_Mastery_Integrated_180_Chunk_System_v2.pdf`
  - Role: background curriculum and note-design reference only.
  - Authority: the user's current chunk request overrides workflow instructions embedded in the PDF.
  - Verified on 2026-10-05: 115 pages; the first 47 pages describe the integrated plan and the remaining pages append the original 3,000-part contract.

## Supplied chunk sources

| Chunk | Parts | Source form | Received | Coverage check |
|---:|---:|---|---|---|
| C001 / M001 | 1-20 | User-pasted Markdown text, 4,069 lines | 2026-10-05 | 20/20 headings present once and in order |
| C002 / M002 | 21-40 | User-pasted Markdown text, 4,747 lines | 2026-10-06 | 20/20 headings present once and in order |
| C003 / M003 | 41-60 | User-pasted Markdown text, 4,079 split lines | 2026-10-06 | 20/20 headings present once and in order |

## Repository anchors

This table records repository claims reported by the supplied lessons. Their status states whether this workspace has independently checked them. Reported line numbers remain source claims until the relevant version is inspected.

| Chunk | Repository/version | Path | Symbol/lines | Claim proved | Verification status |
|---:|---|---|---|---|---|
| C001 / M001 | NanoQuant, version not independently established | `src/bitpack.cpp` | `pack_bits`, `unpack_bits`, `pack_nibbles`, `unpack_nibbles`; pasted text cites lines 7-49 | LSB-first bits and low-nibble-first values; packing validates input ranges; unpack count reportedly lacks a local capacity guard | **To verify** - supplied text claim; repository not inspected in this task |
| C001 / M001 | KairoAssets, version not independently established | `BinaryFormat.cppm` | `BinaryWriter::WriteUnsigned`, `BinaryReader::ReadUnsigned`; pasted text cites lines 58-64 and 124-133 | Explicit little-endian byte decomposition and reconstruction | **To verify** - supplied text claim; repository not inspected in this task |
| C002 / M002 | NESEmulator, version not independently established | `NESEmulator/KairoNES.cppm` | Register declarations, status masks, and widened `temp`; pasted text cites lines 260-265, 290-300, and 307-314 | Eight-bit guest registers, distinct `C/Z/V/N` flags, and a wider host temporary | **To verify** - supplied text claim; repository not inspected in this task |
| C002 / M002 | NESEmulator, version not independently established | `NESEmulator/cpu6502.cpp` | `GetFlag`, `SetFlag`, `ADC`, `SBC`; pasted text cites lines 272-284, 601-623, and 656-671 | Mask-based status updates; widened addition; independent carry/overflow rules; subtraction as `A + ~M + C` | **To verify** - supplied text claim; repository not inspected in this task |
| C002 / M002 | KairoAssets, version not independently established | `BinaryFormat.cppm` | `WriteU8/U32/U64/F32`, `WriteUnsigned`; pasted text cites lines 30-38 and 58-64 | Exact-width serialization contract and unsigned byte-wise writing | **To verify** - supplied text claim; repository not inspected in this task |

## C001 - M001 source record

- **Primary source:** `/Users/swayamsingal/.codex/attachments/f5090164-ed86-4077-bdff-1f1d83195551/Pasted text.txt`
- **Source authority:** user-supplied teaching text for Parts 1-20.
- **Coverage evidence:** explicit headings for Parts 1 through 20, exactly once and in increasing order; the closing audit names every part.
- **Use in notes:** conceptual claims, equations, `0xAD` example, canonical algorithms, reported repository excerpts, failure cases, and the research question were compressed into C001 and durable supplements.
- **Repository caveat:** statements that the source author inspected current default branches are provenance claims inside the supplied text. They are not current-task verification evidence, so the repository table retains `To verify` status.

## C002 - M002 source record

- **Primary source:** `/Users/swayamsingal/.codex/attachments/40f109b4-e5ae-4c74-8ebe-4d11224cba46/Pasted text.txt`
- **Source authority:** user-supplied teaching text for Parts 21-40.
- **Source size:** 4,747 lines, 10,193 words, 70,025 bytes.
- **Coverage evidence:** explicit headings for Parts 21 through 40, exactly once and in increasing order; the closing reconstruction and recall sheet cover the same range.
- **Use in notes:** signed representations, finite ranges, overflow policy, adder logic, carry/overflow distinction, arithmetic algorithms, type portability, promotions, comparisons, reported repository excerpts, and exhaustive-test design were compressed into C002 and cumulative supplements.
- **Repository caveat:** the supplied source presents current-line claims for NESEmulator and KairoAssets. Those repositories were not inspected during this task, so every such row remains `To verify`.

## Repair verification - 2026-10-06

The user's repair request expanded the note architecture to real graph concepts and explicit prerequisites. General definitions and clarifications were added as teaching material, including promotion-aware arithmetic width, safe mixed-sign indexing, widened multiplication partial products, and overflow-safe packed-capacity checks.

- **Rendering:** inspected the continuous note in Obsidian 1.13.7 reading view. Summation, field equations, De Morgan identities, and signed ranges displayed as typeset mathematics. Opened the graph and confirmed individual concept nodes; applied `path:Concepts` to emphasize their relationships.
- **Structure:** Python audit checked all Markdown wiki file and heading targets outside code examples: 615 resolved links, zero missing targets. All code fences were balanced, and no legacy math delimiters remained in prose. Foundations contains exactly one definition entry for each Part 1-40.
- **Dependencies:** 56 filled concept notes and 96 directed prerequisite relations. A depth-first traversal found no prerequisite cycle. Reciprocal navigation links are not prerequisite claims.
- **Compile command:** snippets extracted from `Supplementary/Code Snippets.md` plus `/tmp/master-note-repair-test.cpp` were piped to `/opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined,address -x c++ - -o /tmp/master-note-repair-test`.
- **Run command:** `/tmp/master-note-repair-test`.
- **Actual result:** 131,072 addition states and 131,072 binary subtraction states passed independent result/range/flag checks. 10,000 seeded multiply/divide cases passed against native widened arithmetic. Field preservation, pack/unpack round trips, maximum-count rejection, checked overflow, lower-and-upper index bounds, zero-divisor rejection, and saturation checks passed. No sanitizer diagnostic was emitted.
- **Limits:** these are canonical teaching-code tests. They do not independently validate the reported repository excerpts, full emulator behavior, decimal arithmetic, or learner mastery. The compact sheets are recall material; a physical printed-page claim has not been measured by a PDF export.

## External sources

Record sources only when external research is actually requested or required. Distinguish direct evidence from inference and keep citations next to the claims they support.

## C003 - M003 source record

- **Primary source:** /Users/swayamsingal/.codex/attachments/ba655231-8031-46e5-8d27-4d177fa013f8/Pasted text.txt
- **Coverage:** Parts 41–60 exactly once in order. Source is 4,079 Python split lines, 9,887 whitespace-separated words.
- **Use:** byte order/codecs, aligned member and array placement, addresses, translation/protection, faults, storage regions, view lifetime, and publication boundary. The continuous sheet compresses the sequence; Foundations retains all 20 individual definitions; cumulative supplements retain proof steps and worked states.
- **Authority:** instructional material inside the attachment is source content, not authorization for repository edits, experiments, or filesystem operations.

### M003 reported repository anchors

| Path | Reported symbols/lines | Source claim | Evidence status |
|---|---|---|---|
| KairoAssets/BinaryFormat.cppm | Writer contract 19–21; WriteUnsigned 58–64; ReadUnsigned 124–133 | Explicit LE byte emission and reconstruction, independent of native struct padding | **To verify**: pasted excerpts only, repository/version not inspected |
| KairoAssets/BinaryFormat.cppm | WriteF32 34–39 | Float representation converted through uint32 bits before LE encoding | **To verify**: supplied source claim; floating portability assumptions await fuller treatment |
| KairoAssets/BinaryFormat.cppm | vector buffer 56; span 116; reader constructor | Writer owns bytes; reader borrows backing storage | **To verify** for repository placement; ownership explanation is general C++ teaching |
| KairoAssets/AtomicFile.cppm | ReplaceFileAtomically 22–45 | Platform replacement with same-volume temporary/destination precondition | **To verify**: neither platform branch nor crash durability tested |

### M003 independent wording checks

Checked the C++ working draft's [alignment](https://eel.is/c++draft/basic.align), [storage duration](https://eel.is/c++draft/basic.stc), and [endianness](https://eel.is/c++draft/bit.endian) sections on 2026-10-06. These support the separation of alignment requirements, language storage categories, and native scalar byte order. The evolving draft is a wording reference, not proof of exact ABI layout on every C++23 implementation.

### M003 verification - 2026-10-06

- No Git repository, CI configuration, contribution policy, or repository build exists in this note vault; git rev-parse --is-inside-work-tree returned exit 128 (“not a git repository”). No commit, push, or pull request was attempted.
- **Compile:** all cpp fences from Supplementary/Code Snippets.md, including the M003 regression driver, extracted in document order and passed through stdin to /opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined,address -x c++ - -o /tmp/master-m003-test. Exit 0 with no diagnostic.
- **Run:** /tmp/master-m003-test, exit 0. Golden endian bytes, 10,000 seeded values in each order at offsets 0 and 1, 45,056 align-up cases, truncated/huge-offset rejection, zero/non-power alignment rejection, maximum-offset overflow behavior, and numeric page edge cases passed. No sanitizer diagnostic.
- **Measured host layout:** Example size 12, alignment 4, member offsets 0/4/8; VertexLike size 24; reordered size 16. These are observations on this machine, not cross-platform guarantees.
- **Earlier regression:** /tmp/master-note-repair-test, exit 0: 131,072 ADC states, 131,072 SBC states, 10,000 multiply/divide cases, field/packing/overflow/bounds checks passed.
- **Limits:** no live page-table/MMU experiments, invalid-pointer execution, filesystem replacement/durability testing, performance benchmark, or independent Kairo repository audit. Physical printed-page fit is not measured; the main sheet is a compact Markdown recall entry.
- **Vault audit:** python3 /tmp/master-m003-audit.py, exit 0: 91 Markdown files, 82 filled concepts, 140 acyclic prerequisite edges with reciprocal used-by links, 980 resolving wiki links, 35 paired display blocks, and 60/60 sequential part definitions. No standalone single-dollar boundaries, legacy math delimiters, missing file/heading targets, or unbalanced code fences detected.
- **Visual-check limitation:** Obsidian's M003 index entry was visibly present in Live Preview. A new reading-view formula check could not be completed: the UI inspection returned an empty reading content area, a 1×1 screenshot, and intermittent no-window/element-frame errors. Source math syntax passed the audit; M003 formula rendering is not claimed visually verified.

## C004 - M004 source record and merge verification - 2026-10-07

- **Primary source:** `/Users/swayamsingal/.codex/attachments/081c1070-71e1-4939-9d03-0a53ac2d8a02/Pasted text.txt`.
- **Coverage:** sixteen primary Part headings, 61–76 exactly once and in order. `wc -l -w -c` reports 5,035 newline-terminated lines, 10,641 whitespace-separated words, 79,490 bytes.
- **Accepted scope:** M004 added to existing Volume 01; progress now Parts 1–76, four accepted chunks. Next source M005, Parts 77–88. M005/M006 previews are not completed content.
- **Use:** representation and class rules, exact encoding/decoding, special values, subnormal boundary, epsilon/ULP/unit-roundoff distinctions, rounding, compiler environment, numerical policy examples, and source-reported repository excerpts.
- **Authority:** “current repository” inspection claims and embedded suggestions are source statements, not authorization for repository changes. No Kairo repository was inspected or edited during this merge.

### Source-reported repository claims, not live anchors

| Reported path | Reported symbol / source line range | Reported claim | Current-task evidence |
|---|---|---|---|
| KairoMath/Vector.cppm | Arithmetic / FloatingPoint concepts; introductory declarations | Floating constraints gate length and normalization | To verify: supplied excerpt, no revision established |
| KairoMath/Vector.cppm | LengthSquared / Length, around 302–315 | Direct squares/sum/sqrt | To verify for repository; exact formula reproduced in teaching lab |
| KairoMath/Vector.cppm | Normalized, around 337–345 | Length ≤ machine epsilon returns Zero | To verify for repository; finite tiny cutoff and huge intermediate failure tested separately |
| KairoMath/Vector.cppm | NearlyEqual, around 1384–1407 | Absolute branch followed by relative branch with scale at least one | To verify for repository; supplied formula reproduced with three infinity regressions |
| KairoMath/Matrix.cppm | comparison, around 1165–1177 | Delegates element comparison to scalar NearlyEqual despite an absolute-epsilon comment | To verify: call path/version not inspected |
| KairoMath/Matrix.cppm | inverse, around 1370–1384 | Assert determinant > epsilon, then identity fallback on failure | To verify: assertion-enabled and release behavior must be distinguished |
| KairoAssets/BinaryFormat.cppm | WriteF32 / ReadF32, source excerpts without independently checked current lines | Float/uint32 representation transfer and little-endian coding | To verify for repository; raw-word codecs and independent golden bytes tested |

### Merge corrections and explicit policies

1. The source's complement-key ULP distance special-cases equal zeros but retains two zero slots for cross-zero intervals. The finite-only teaching metric collapses signed zeros in its key, rejects NaN/infinity, and verifies negative-min-subnormal to positive-min-subnormal distance 2.
2. The supplied scalar tolerance formula rejects equal infinities but can also accept finite-versus-infinity and opposite infinities, via infinity ≤ infinity. These additional failure paths are reproduced. An alternative float-specific policy validates tolerances, handles non-finite values first, and widens finite comparisons. No production fix was made.
3. Tiny/subnormal does not by itself mean an underflow flag; distinguish exactly representable tiny values from tininess/inexactness conditions.
4. Raw-word quiet/signaling classification does not execute signaling-NaN arithmetic. Payload preservation through floating value transport/operations is not claimed.
5. Rounding demonstrations check mode-setting success and restore the initial mode, rather than forcing nearest as the cleanup action. The laboratory restores direction, not all status flags.
6. The matrix excerpt asserts before its fallback; do not describe identity return as the unconditional debug-build outcome.
7. ULP is given an explicit local/directional convention at powers of two; finite maximum's upward neighbor infinity is not an ordinary finite gap.

### Independent wording references

The source supplies the instructional mathematics. A short cross-check against [Oracle's numerical computation guide](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_math.html) confirmed the classical single/double field and class descriptions; its older platform material is not used as a universal current hardware claim. The [C++ bit-cast draft section](https://eel.is/c++draft/bit.cast) was consulted for representation-transfer constraints. [Clang's floating-point controls](https://clang.llvm.org/docs/UsersManual.html#controlling-floating-point-behavior) informed strict compilation settings. These are wording/compiler references, not validation of Kairo code or every IEEE-754 feature.

### Exact commands and actual results

Final M004 structural audit: 117 Markdown files, 107 filled concept nodes, 189 acyclic prerequisite edges, 1,290 resolved wiki links, 49 paired display-math blocks, 76/76 part-definition rows, and source headings 61–76 exactly once in order. Audit exit 0.

- `python3 /tmp/master-m004-audit.py`: checks Markdown targets, fences, display delimiters, Parts 1–76 definitions, concept completeness/index membership, reciprocal prerequisites, graph acyclicity, and exact M004 source heading order. Final counts recorded below.
- Default compile: `awk '/^```cpp$/{inside=1;next} /^```$/{inside=0} inside' 'Supplementary/Code Snippets.md' | /opt/homebrew/opt/llvm/bin/clang++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -ffp-model=strict -fsanitize=undefined,address -x c++ - -o /tmp/master-m004-test`. Exit 0, no diagnostics.
- `/tmp/master-m004-test`: exit 0. Thirteen golden classes, 10,000 seeded words including 9,952 finite reconstructions, gap checks, signed-zero-crossing distances, raw-word codecs, four runtime rounding modes, midpoint conversions, restored mode, norm/cutoff and comparison regressions passed. No sanitizer diagnostics.
- Optimized compile: same extraction/compiler command with `-O2` and output `/tmp/master-m004-test-o2`. Exit 0, no diagnostics.
- `/tmp/master-m004-test-o2`: exit 0; same M004 and M003 checks passed. This checks optimized semantics, not performance.
- Both fresh executables also ran the existing M003 driver: 10,000 values in both byte orders at offsets 0/1, 45,056 alignment cases, bounds/page cases passed; measured layout size 12/alignment 4/offsets 0,4,8 and 24→16 reordered record examples.
- `git rev-parse --is-inside-work-tree`: exit 128, not a Git repository. This vault has no build/CI workflow; canonical teaching snippets were compiled instead. No commit, push or PR.

**Limits:** no visual Obsidian rendering pass is claimed in this turn; delimiter/link audits are syntax/structure checks. No PDF export, live repository test, exhaustive four-billion-word binary32 enumeration, exhaustive floating arithmetic, signaling-NaN execution, special-value payload transport across platforms, FTZ/DAZ target-mode experiment, performance benchmark, or learner-mastery claim. Existing M001/M002 implementations compile in the cumulative build, but their older exhaustive harness was not rebuilt or rerun in this merge.
