# Pass 17 — Book 1 source coverage, depth and verification audit

**State:** Authoring-level audit of Project AllMight's existing sources, not a
completion certificate. Date: 19 November 2026. Revision branch:
`curriculum/cpp23-nes-project-pass01`. The original continuous manuscript is
preserved byte-for-byte. This report and its companion read-only structural
checker are **additions**, not a rewrite or a LaTeX/export operation.

## Executive conclusion

**Book 1 is NOT yet complete to the requested standard.** The first sixteen
large derived reading chapters exist, and sixteen substantial supplemental
lessons with real C++23 source files have been authored and committed. They are
not superficial two-page substitutes. But (a) there is no observed compiler,
CTest/ASan/UBSan or native-device proof for the complete set, (b) the derived
source has significant narrative fragmentation, (c) certain major modern C++
API families have no identifiable worked treatment in the reading material
searched, and (d) a seven-book editorial split and a conclusive real-project
capstone have not been verified.

This is a source-depth audit with **three distinct evidence sets**:

1. All **39** existing Chapter Edition reading-map entries were located.
   The 16 directly supplemented chapters were fetched in full from the
   GitHub curriculum branch and examined for file identity, section markers,
   line structure, topic references and anchor placement. All 16 staged
   lessons were separately fetched and measured.
2. For each of the 16 source chapters, the supplement's exact original
   heading was found **once** in its corresponding derived chapter. This
   supports a plausible insertion route; it does not alone prove that every
   heading occurs exactly once in the much larger canonical master. A new
   read-only script checks canonical occurrence counts on a complete local
   checkout before any authorized application.
3. The Book01 CMake target graph, CMake presets, source-evidence inventory,
   insertion patcher, insertion fixture tests and GitHub Actions workflow
   were inspected as **source**. The earlier pass's published Git blob SHA
   for the original master is
   `d3202af31e1eb644a74565eeb63e7a3352438823`. Actual passing
   compiler/CTest or native GPU receipts were not observed in this audit.

The **first 16 original reading chapters** collectively contain about
**1,441,560 characters**. Their **16 staged lessons** add approximately
**507,498 characters**. These are separate source files, not 1.95 million
characters already reconciled into a verified print master. Counting words
or characters is *never* a proxy for conceptual completeness, correctness,
clarity or successful application.

## Book boundary: 39 reading chapters is NOT 39 unfinished Book-1 passes

The three continuous manuscripts and the planned seven-book pedagogical
structure are separate artifacts. The first continuous manuscript currently
contains **39** derived chapters and a six-volume print pipeline is noted
elsewhere. The seven books are the proposed *teaching structure*, not seven
independently compiled, final TeX masters. We must explicitly route and
reconcile source spans, rather than count 39 Chapter Edition entries and
arbitrarily name all of them "Book 1."

In the current routing sheet, chapters **1–8** primarily introduce Book 1
data representations, ownership, generic asset import, typed interfaces,
containers/CPU state and graph algorithms. Chapter **10** is a strong Book 1
build/toolchain/profiling chapter. Chapters **11–12** have important Book 1
borrowing and asynchronous/command lifetime material that also belongs
at the Book 6 GPU boundary. Chapter **16** bridges C++ ownership,
Objective-C++ and SwiftUI. Chapters **9 and 13–16** are simultaneously
substantial **Book 6 GPU/Metal/Vulkan** material; they must not be silently
duplicated or omitted in the eventual seven-volume route.

Reading chapters **17–31** chiefly serve the Book 2 numerical/mathematical
track, **32–38** the Book 3 camera/geometry track, and **39** the cross-book
research/source-navigation method. Thus **"16/39 chapters" is not a
meaningful Book 1 completion percentage**. The next editorial milestone
must freeze a per-book inclusion/exclusion/bridge table with chapter and
source-span provenance before publishing any master.

## Chapter-by-chapter source audit, chapters 01–16

All source chapters below existed on the curriculum branch. Their derived
reading copies already contain large amounts of original detailed text.
Each addition is a separate lesson for the corresponding chapter; a
supplement's CMake test registration must not be treated as evidence that
the entire original lesson or production engine has compiled.

| Original source chapter | Original chars / lines | Staged lesson | New chars | Source-based assessment and required completion witness |
| --- | ---: | --- | ---: | --- |
| 01 From Bits to Running Systems | 105,075 / 1,252 | NES-01 | 10,684 | Real iNES header and validation exercise. Good binary-contract spine; still only a subset of historical mapper/PPU compatibility and not a complete emulator. Confirm all invalid sizes/flags and parser diagnostics under compilers and sanitizers |
| 02 Values at Addresses, Layout, Lifetimes and Representation | 88,668 / 2,436 | NES-02 | 14,645 | Owned NROM cartridge and CPU-bus mapping. Needs actual language/lifetime/aliasing checks and a conclusive source ownership/lifetime diagram, not just a reachable happy-path bus |
| 03 Records as Resource Transactions | 102,679 / 1,892 | NES-02T | 18,354 | Transactional staging/commit and strong exception guarantee. Prove failure injection, copy/move/rollback and ownership with actual compiler execution and stable external observer state |
| 04 Prove C++ Ownership with Runnable Experiments | 127,814 / 1,603 | NES-02P | 18,124 | Many original ownership/lifetime explanations; supplement adds executable failure probes. Await verified test/sanitizer receipts, explain distinction between object lifetime and allocation lifetime through an owned engine resource |
| 05 Generic Asset Pipelines without Hidden Contracts | 104,337 / 1,516 | NES-05A | 25,513 | Typed NROM/RGBA/mesh staging and publication. Extend coverage to actual Kairo/Maveb import schemas and malformed asset corpus; no claim that full model/texture streaming is validated |
| 06 Grow Registries without Invalidating Identity | 146,415 / 1,331 | NES-06G | 27,993 | Generational registry and stale-handle refusal. Strong identity theme. Need production asset-store migration and concurrency/read-borrow/retirement analysis before claiming robust engine usage |
| 07 Typed Interfaces, Identity and Data Structures | 128,162 / 877 | NES-07C | 35,591 | Partial 6502 interpreter, typed debugger, containers and indices. Real CPU subset is **not** a full emulator. Needs complete DSA implementation audit, independent debugger API test and clearer reader checkpoint to prevent sprawl |
| 08 Graphs, Search and GPU Scheduling | 89,660 / 2,263 | NES-08G | 35,243 | Host DAG and hazards with actual algorithmic work. The GPU scheduling half is a model, not a Vulkan/Metal barrier implementation. Lesson has no fenced source excerpt; add carefully selected commented algorithm excerpts/diagrams beside verified executable |
| 09 GPU Memory, Divergence, Atomics and Ordering | 56,979 / 3,356 | NES-09M | 37,263 | Good conceptual GPU vs host memory distinction. **Original median nonempty line only about 13 characters**; many short note fragments. The new CPU model is not a native GPU memory test. Needs narrative consolidation/visual memory timelines and native validation, routed chiefly to Book 6 |
| 10 Reproducible Builds, Profiling and Repository Contracts | 64,553 / 4,037 | NES-10R | 25,224 | Explicit CMake/CI/evidence protocol, but no observed all-pass CI. **Original median nonempty line about 12 characters**: improve continuous teaching prose. Some repository-specific examples are admitted snapshots rather than verified current builds |
| 11 C++ Const, Borrowing and CPU–GPU Buffering | 94,974 / 2,378 | NES-11B | 43,250 | Move-only leases, pinned views, generation checks and CPU/queue-stamp models. Native GPU completion is simulated; audit const APIs, reader ownership and tests before calling pool production-safe |
| 12 Commands, Async Work and Resource Lifetimes | 50,901 / 3,198 | NES-12Q | 47,934 | Command acceptance, indeterminate outcome and ownership are modeled. **Original median nonempty line about 13 characters**, source reads like segmented small notes. Need stronger reader narrative and a truly executed concurrent/task/native-backend trace |
| 13 Open Vulkan Devices and Describe Resources | 72,137 / 4,556 | GPU-13D | 42,298 | Real Kairo source inspection and optional Vulkan instance/device/buffer probe authored. No native probe executed; device feature requirements and allocation errors remain hardware-dependent. Source carries metadata such as “+ M135” that needs editorial explanation, not unexplained jargon |
| 14 Submit, Synchronize and Retire GPU Work | 71,270 / 4,398 | GPU-14F | 37,388 | Kairo fence-reset failure path observed in source; CPU model and opt-in real buffer fill/fence code authored. A source-level potential hang is **not** a reproduced GPU fault. Requires native fence/WSI/validation evidence; original heavily fragmented |
| 15 Execute and Measure a Metal Frame | 47,241 / 3,061 | GPU-15M | 43,585 | Maveb/Kairo native code inspected; optional MSL compute + actual offscreen draw/readback prepared. No real Metal result or timing yet. Original language uses long-running pass/phase bookkeeping and many fragments; add a clean execution-to-measurement narrative and actual device results |
| 16 Profile GPUs and Respect Apple Ownership Boundaries | 90,695 / 2,467 | GPU-16P | 44,409 | Maveb callback and Kairo Objective-C++ source read; new C++23 host concurrent mailbox and opt-in native compute callback. Source-level callback risk is not proof of UAF; counter availability/timing/reproducibility and compiler results all unverified |

The original chapter sizes here reflect **derived Chapter Edition** file
characters, which include introductory navigation and source markers. Lesson
size is separate. The table is a source inventory and targeted review
assessment, not 16 fully passed technical correctness audits.

## Critical gap: general C++23 mastery is not yet evidenced

A lexical scan of **all 39 derived chapter files and all 16 staged lesson
files** found **zero exact API examples** of the following terms:

| API family checked | Search spellings | Finding / required learning outcome |
| --- | --- | --- |
| C++23 formatting/printing | `std::format`, `std::print` | **No matching spelling in surveyed reading corpus.** Teach format parsing and compile-time format strings, safe error reporting, custom formatters where supported, and comparisons to streams; verify actual toolchain/library support |
| C++17/20 filesystem and file streams | `std::filesystem`, `std::ifstream`, `std::ofstream` | **No matching spelling.** This is serious for asset loaders, binary I/O, Unicode/paths, atomic file replacement, symlinks, canonicalization, time-of-check/use races, sandbox boundaries and durable data pipelines; use real Kairo/Maveb asset inputs |
| C++20 ranges and views | `std::ranges`, `std::views` | **No matching spelling.** A proper modern C++ book needs iterators, ranges, projections, borrowed ranges, lazy view lifetimes and invalidation, testable against container and asset registry examples |
| C++20 coroutines | `co_await`, `co_yield`, `promise_type`, `std::coroutine` | **No matching spelling** in reading corpus; there are general textual mentions of coroutines elsewhere. Teach coroutine-frame lifetime, promise state, suspension/resumption, cancellation and correct ownership via a bounded async asset/command processing example—not a vague statement that coroutines are “advanced” |

**Interpretation:** A zero *exact spelling* is strong evidence that worked
examples of these APIs have not been included under their canonical names.
It is **not a proof** that every related idea is absent from every source
file, nor that those functions are supported by all current toolchains.
The scan did not claim to cover three separate continuous masters in their
entirety, unmounted local worktrees, every external textbook or all source
files in other repositories. We must inspect those separately before an
absolute “missing everywhere” statement.

These areas should be addressed in a dedicated **language-completeness
reconstruction** before Book 1 release. C++23 coverage should also examine
`std::expected` (already in labs), `std::span` and lifetime, templates and
constraints, modules and ABI, `std::pmr`/memory resources, error boundaries,
`std::jthread`/stop tokens, atomics and memory ordering, Unicode/string
processing and C++ standard-library ABI/toolchain differences. Do not
confuse a source mention with a fully taught/validated implementation.

A practical integrated Book-1 exercise is an **owned async asset/import
service**: use actual filesystem input and durable output transactions,
strict untrusted-binary parsing, ranges for traversal, format-safe
diagnostics and a coroutine-driven/cancellable bounded producer-consumer
pipeline. Build its core around existing typed asset/registry and Kairo
resource types, not another disconnected toy. Add independent tests for
path escaping, symlink and race behavior where supported, traversal
invalidations, temporary-range borrows, failed cancellation, exceptions
during allocation and checkpoint persistence. A native engine asset
integration can follow after the pure C++ contract passes.

## Narrative/depth defects that cannot be repaired by word count alone

**First, note-fragment formatting.** The original chapters 09–15 have
thousands of short separate lines (median nonempty widths of roughly
12–15 characters in the measured examples), often expressing concepts
as tiny fragments, line-isolated equations, lists and section separators.
Some line breaks are appropriate for code/math. But such formatting
across whole chapters breaks explanatory continuity and creates the
precise “jargon/microlines/read later” experience that the curriculum
is supposed to eliminate. Avoid rewriting or deleting original text;
instead add cohesive explanations at first use, editorial transition
sections, selected derivations, properly commented code and repeated
end-to-end examples. At final publication a separately reviewed
reflow may be appropriate if the user authorizes it.

**Second, leaked production metadata.** Examples in the read source include
`This is Part 2147.` and `### + M135`, and an older source segment says
`Pass 13 begins Phase 24` in a chapter that a modern student sees as
Chapter 15. These are not concepts; they are artifacts of earlier writing
passes. Track their locations, then hide them in an editorial/provenance
index or explain them in narrative without silently shortening the source.
A finished teaching book must not make the reader infer a 2,700-part
production history just to follow one GPU command.

**Third, chapter-to-chapter continuity.** The original learning sequence
jumps from C++ object semantics to an emulator, GPU scheduling, Vulkan,
Metal, Python, mathematics, numerical solvers and camera geometry. The
seven-book route needs a carefully authored *destination-specific bridge*
for each transition. The core concept should be explained at the point
needed, not deferred with “learn later” or recited in several volumes.
Add named carried-forward invariants (owner, generation, byte extent,
frame identity, producer/consumer range, error status, numerical unit and
coordinate frame) to link the projects while preserving their genuine
differences.

**Fourth, teaching visuals.** The Chapter Edition files inspected did not
include Markdown inline image references. The supplements include some
text diagrams and tables, which are valuable, but the book still needs
clean, author-owned figure plans for: C++ allocation/object/view lifetime,
CPU/GPU queue timelines, WSI image versus frame resources, memory type
masks, Gaussian scan/sort, camera transform chain and call-site-to-resource
ownership. The audit has not verified whether separate figure assets
already exist in the repository. Do **not** create copyright-unclear
illustrations or pretend a new picture substitutes for derivation/code.

**Fifth, source and code provenance.** The lessons include official
documentation links and named repository snapshots, but the four source
areas we actually inspected in recent passes do not constitute a
source-by-source audit of every Kairo, Maveb, Kyoto, Vulkax and ILLIXR
code path. The two local research worktrees and any uncommitted changes
were not fetched through GitHub in this pass. A real completion audit
must record source file/commit, actual build configuration, test data and
independent oracle for each project integration. Unsupported claims must
stay labeled hypothetical or not yet verified.

## What the current toolchain and evidence actually establish

Source inspection confirms **16** CMake executable registrations and
**16** normal CTest names plus a GitHub Actions workflow with GCC/Clang,
AppleClang and ASan/UBSan job definitions. The same source registers
optional Vulkan/Metal probes separately, with portable default CTest
modes labeled CPU models. The insertion patcher stages **16** marker-
delimited lessons and its synthetic fixtures check preservation,
idempotence and conflict refusal.

That means **the automated test system has been authored**, not that it
has passed. Recent GitHub status/workflow checks returned no observed
successful run receipts for the recent commits. The current turn did
not execute any of the whole-suite C++ binaries, run a native GPU
adapter, create a JUnit report or verify real device readback.
Neither a compiled cross-platform C++ executable nor a verified native
Metal/Vulkan benchmark should be inferred from a commit SHA.

The new `tools/book01_coverage_audit.py` makes a narrower but real
improvement: it performs a *read-only* local audit of all 16 original
derived chapter files, exact source anchors, staged lesson headings,
CMake/CTest registrations, core source files, source-evidence inventory,
and preserved canonical Git blob. Its result explicitly labels compiler,
GPU and research validation **not verified**. It does not run LaTeX or
alter any manuscript. The script itself is committed source; a passing
run has not yet been observed on the full checkout.

## Prioritized corrective plan and acceptance criteria

### P0 — Fix before calling Book 1 technically complete

**P0.1: Run and repair the full 16-target C++23 test matrix.**
All labs must compile under supported GCC/Clang/AppleClang; validate
debug/release/sanitizer runs, read CTest/JUnit outcome rather than
accept a configured target, and produce a source-pinned receipt for
every executable. Fix all warnings and actual failures with comments
and regression tests. The report must distinguish a passing CPU
oracle from actual native Vulkan/Metal GPU execution.

**P0.2: Close general C++23 foundation gaps.** Add meaningful continuous
lessons and commented *running* examples for filesystem and file-stream
trust, ranges/views and borrowed lifetimes, format/print, coroutines
and cooperative cancellation. Integrate them into actual Kairo/Maveb
asset/data or async execution threads rather than producing a random
set of disconnected API snippets. Verify with standard-library/OS
feature checks and fallbacks when necessary.

**P0.3: Finish a multi-project, end-to-end acceptance trace.**
At minimum, demonstrate source bytes/asset import → owned typed
representation → registry identity → scheduled task dependency →
checked compute or transform → safe retirement → reproducible
output/result from **actual code**. This can begin as a portable CPU
headless trace and then gain real device execution evidence on
compatible hardware. Record expected negative cases and source
provenance. A partial 6502 emulator is not falsely renamed a
complete NES emulator, and a 64-word GPU test is not renamed a whole
renderer.

### P1 — Fix before calling the *book* pedagogically complete

**P1.1: Rebuild transitions as a continuous narrative.** Audit each
concept's first use and ensure motivation, representation, derivation,
plain-English interpretation, implementation, failure, independent
tests and consequences. Add bridging prose where the original is
fragmented or metadata-heavy, without erasing earlier content.

**P1.2: Freeze the seven-book source-span routing map.** Give every
reused source span a home/primary book and referenced cross-book
concept map. The Book 6 Vulkan/Metal content should not be mistakenly
counted as completed Book 1 foundational coverage. Avoid
duplicating entire chapters in multiple books.

**P1.3: Verify every major external reference actually used.**
Re-open the exact documents, videos/papers/API versions and source
snapshots, distinguish an intended source from a source actually
examined, and check the claim against the source instead of citing
a channel name or a book title.

**P1.4: Build figures and exercises with actual answers/oracles.**
Every major chapter needs meaningful projects, input fixtures,
sample failed output and a practical self-check. diagrams must
show data/lifetime flows concretely rather than decorate pages.

### P2 — Production/release

**P2.1:** Verify canonical master insertions are applied only after
the user authorizes the consolidated checkpoint, with unique
anchoring, conflict refusal and byte-for-byte restoration of
original source when additions are stripped.

**P2.2:** Rebuild derived reading chapters and final seven-book print
structure only from reconciled approved source; verify navigation,
cross-references, math/code formatting, figure placement, source
citations and print legibility.

**P2.3:** Produce a release manifest with commit SHAs, source lengths,
tests, build configs, platform-specific limitations and a list of
unverified native hardware/research claims. Do not use a page count
as a declaration of curriculum completeness.

## Revised estimate

The previous estimate of **4–5 finishing passes** was a reasonable
planning placeholder when the work appeared to consist primarily of
auditing, integration, testing, editorial review and source merges.
The present evidence shows an additional **substantive C++23
coverage-closing implementation**, plus unresolved native test and
narrative problems. Therefore **4–5 passes should not be promised as
a guarantee**. Use outcome-based release gates: this Pass17 source
audit, a build/repair pass, a C++23 gap-closing pass, a multi-project
capstone/editorial integration pass, and a final separately authorized
application/verification checkpoint, adding passes if the tests or
source audit reveal further necessary repairs.

The strongest next action is to **compile/repair the 16 targets and
write a genuinely complete C++23 filesystem/ranges/format/coroutine
asset pipeline**, then audit the repaired result rather than call Book 1
finished merely because the pass counter reaches twenty.
