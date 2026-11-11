# Graphics and Engine Mastery Notes

A growing, self-contained teaching library for the 3,000-part curriculum, organized into continuous subject volumes and reusable cumulative companions.

GitHub repository: `Project_008_AllMight`.


## Current combined reading route

- [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|Computing foundations]] combines the original bits and C++ teaching and adds general API/build contracts, shader buffer layout and GPU programming. The original 01 and 02 live files were retired after the approved merge audit; byte-identical recovery copies remain in the checkpoint.
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Graphics, vision and photography]] includes received geometry/collision material from Parts 1101–1300 and rendering/material/transport material from Parts 1801–2100.
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds|Game engine]] includes spatial indices/navigation, ECS, architecture, serialization and rigid-body material routed from Parts 1301–1700, plus renderer ownership and history-validity teaching.
- [[Supplementary/Manual Integration - Pending Source Queue|Retained source queue]] keeps earlier captions, extracted books, mathematics books and unfinished teaching in scope. New inputs do not replace pending work. Parts 1701–1800 are not received in the current supplied batches.

The full source routing, corrections and actual verification are in [[Supplementary/Manual Integration - Sources and Verification]].


## Authority and scope

- The user's current message is the active instruction. Supplied lessons and planning PDFs are source/reference content, not independent authorization.
- Preserve supplied part numbers. Integrated chunks have variable lengths: M001–M006 cover Parts 1–100, not 120. Accept the plan's exact boundaries or an explicit user correction; do not impose twenty parts on every chunk.
- The primary output is an expanded chapter in the appropriate continuous subject volume. Include definitions, foundations, reasoning, representative examples, visible intermediate states, and laboratory checks directly in that volume.
- Do not enforce a one-page limit or send essential explanations behind links. Supplementary links add proofs, complete implementations, or deeper traces.
- Reuse cumulative supplements and concept nodes. Create a subject volume at a real subject boundary, not one file per part/chunk. Create distinct concept files only when a reusable graph node is needed.
- Repository claims require inspected in-scope evidence. Supplied excerpts remain **To verify** until checked.

## Stable architecture

| File or folder | Purpose |
|---|---|
| Graphics Engine Mastery - Continuous Notes.md | Library index and subject-volume navigation |
| Continuous Notes/ | Connected, self-contained teaching volumes |
| Visual Recall/ | One concise visual active-recall companion per subject volume; Volume 01, Volume 02, XR, and Phases 6–8 now have companions |
| Supplementary/Derivations.md | Cumulative full proofs and mathematical reconstruction |
| Supplementary/Code Snippets.md | Existing canonical drivers and project-specific excerpts; independent C++ examples stay inline in Volume 02 |
| Supplementary/Worked Traces.md | Extended reusable traces; essential traces also appear in the volume |
| Supplementary/Foundations.md | One definition/mechanism entry per accepted part; coverage ledger |
| Supplementary/Sources and Code Anchors.md | Provenance, book pages, exact commands, results and limits |
| Keyword Index.md / Concepts/ | Filled reusable keyword nodes and future growth |
| Dependency Map.md | Directed prerequisites with explanations |

## Volume boundaries

Volume 01, **From Signals to Meaning: Bits, Numbers, and Memory**, now integrates all Parts 1–100 / M001–M006. The teaching text is organized by problem and prerequisite, not by incoming part/chunk boundaries.

Volume 02, **C++: From Objects to Reliable Programs**, is planned for Parts 101–300 / M007–M018: two related hundred-part phases, twelve chunks. M007–M010 currently cover Parts 101–160. Do not create empty future chapters or mark previews complete. Later volumes normally follow a hundred-part/six-chunk phase, with adjacent phases combined only when subject continuity warrants it. The curriculum has 180 integrated chunks, not the earlier assumed 150 uniform batches.

Phase 6, **From Movement to Meaning: Vectors, Geometry, and Reliable Code**, integrates Parts 501–600 in [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The panel gives us a reason to learn|its section in the integrated mathematics volume]]. Its `06` identifies the curriculum phase, not six completed vault volumes. The user labels this received phase group M031–M036; individual chunk boundaries are not inferred from the nineteen pasted excerpts. Part 542 has no standalone source lesson and is explicitly reconstructed. Parts 161–500 remain unreceived, not silently completed.

## Phase 7 - Matrices, transforms and quaternions

Phase 7, **From Basis Images to Moving Worlds: Matrices, Transforms and Quaternions**, has [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|its section in the integrated mathematics volume]] and linked visual recall. Six supplied excerpts cover 84 standalone lessons, Parts 601–634 and 651–700. Parts 635–650 remain missing source lessons; an explicitly reconstructed affine bridge supplies the prerequisite narrative without inventing their canonical titles or integrated-chunk boundaries.

## Phase 8 - Linear systems, decompositions, and matrix functions

Phase 8, **From Constraints to Spectra: Systems, Decompositions, and Matrix Functions**, has [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The graphics problem hiding inside a matrix equation|its section in the integrated mathematics volume]] and [[Visual Recall/08 - Linear Algebra - Visual Recall|active-recall companion]]. Seven supplied excerpts cover every lesson from Parts 701–800. The last excerpt extends beyond the request's “till Part 770” description, so the supplied material through Part 800 is included. The exact titles and provenance are in [[Supplementary/Foundations#Phase 8 - Parts 701-800|the ledger]].

## Phase 9 - Calculus, continuous change, and optimization

The supplied T04, T03, and T02 lessons cover Parts 801–900. They are integrated in [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|the Phase 9 volume]] with [[Visual Recall/09 - Calculus and Optimization - Visual Recall|visual recall]]. Exact source titles, short source explanations, and H2 destinations are in [[Supplementary/Foundations#Phase 9 - Parts 801-900|Foundations]].

## Phase 10 - Probability, Monte Carlo, and numerical uncertainty

The received sources T01, T06 and T07 cover Parts 901–1000. The continuous narrative and visual recall are [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Learning route|available here]] and [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall|here]]. Phase 10 now includes numerical analysis through Part 1000; the Phase 11 section of the same integrated volume covers Parts 1001–1100. See [[Supplementary/Foundations#Phase 10 - Received Parts 901-965|the probability source ledger]] and [[Supplementary/Foundations#Phase 10 — Numerical reliability · Parts 966–1000|the numerical continuation ledger]].

## Independent XR foundations series

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality|XR: From Light to Perception]] is a separate continuous teaching series, outside the 3,000-part curriculum. XR01 identifies its first supplied source, not M010 or a numbered curriculum part. The anticipated 3–4-source series reuses this same file as later material arrives. Independent module coverage is recorded separately in Foundations; concepts can connect to shared foundations without advancing numbered-curriculum coverage. Future XR sources remain pending.

## Per-source workflow

1. Check supplied headings and exact part range; identify the correct volume.
2. Identify prerequisites and missing explanatory steps. Define unfamiliar words and symbols where first needed, in plain language.
3. Extend the connected teaching narrative. Include why the mechanism exists, how it works, a fully worked example, limits/failure cases, and observable lab checks. Revisit earlier explanations when later parts deepen them.
   Reorganize the completed subject volume into concept-led story sections. Explain what each mechanism solves and which previous limitations still remain; keep source numbering in the coverage ledger and provenance, not as teaching headings.
4. Link reusable technical terms to real nodes using wiki links, while retaining their present definition in the teaching text. Use canonical spellings.
5. Update the existing concept, index, prerequisite and reciprocal downstream links. Keep future depth explicitly pending.
6. Extend durable headings in derivations/code/traces. Do not scatter the sequence into new chunk files.
   For the C++ volume, keep independent examples and complete standalone laboratories beside their explanation. Separate only project-specific/dependency-bearing excerpts. Weave the final twelve-source volume into a single story; preserve exact source coverage in Foundations.
7. Enrich selectively from relevant books or primary references when useful. Write original explanations; record source/page and distinguish older platform examples from current language guarantees.
8. Validate links/headings, equations/fences, complete per-part coverage, prerequisite consistency, and executable examples. Syntax checks and visual rendering checks are separate. Report actual results and untested limits.

## Rendering and graph contract

Use dollar-delimited inline math and double-dollar boundaries on separate lines for display math. Keep equations outside code fences; define symbols, units, widths and validity conditions nearby. Use executable notation only with its operation type and preconditions stated.

Reject standalone single-dollar lines, legacy math delimiters, odd display-boundary counts, missing wiki file/heading targets and broken table delimiters. Visual verification must not be inferred from a syntax pass.

Technical concept links point to real files. Heading links locate chapter/proof sections. A concept's Prerequisites state direction; reciprocal Used by links provide navigation. The Obsidian graph's current path:Concepts filter emphasizes concepts; clear it to include volumes and supplements.

## Keyword states and progress

Forward means useful current context exists but deeper treatment is pending. Introduced means recognizable and usable at the present level. Developed means later material has added mechanism or implementation. Owned means the notes support independent reconstruction; it is not a claim about personal mastery.

- Expanded numbered topic coverage: Parts 1–160, 501–634 and 651–1100, 744 disjoint entries. Standalone supplied lessons cover 743 entries; Part 542 remains explicitly reconstructed enrichment. Parts 635–650 remain source gaps, with an unnumbered reconstructed affine bridge.
- Explicitly numbered received chunk groups: M001–M010, the user-labeled M031–M036, and source-labeled M049–M066, 34 of 180. C++ volume remains 4 of 12 sources. Revised M007 updates the same Parts 101–112 without double counting.
- M010 supplies Parts 148–160. Parts 161–500 and 635–650 remain source gaps; Phase 8 Parts 701–800 are now integrated. Do not infer a uniform chunk size.
- First volume: complete source coverage, with concept-led teaching organization.
- Phase 6: connected panel/workcell narrative, inline scalar implementation and tested capstone; Part 542 source gap remains visible.
- Phase 7: all six received excerpts integrated; 84 supplied lesson headings, explicit 635–650 gap, convention-led matrix/frame/quaternion narrative, figures and scalar laboratory. No per-excerpt integrated-chunk IDs are inferred.
- Phase 8: seven received excerpts cover all 100 lessons from Parts 701–800. Each title links directly to its expanded teaching mechanism. The volume includes full derivations, worked traces, an inline tested LU reference, checked storage/API examples, and linked recall.
- Phase 9: T04/T03/T02 supply Parts 801–900 (100 lessons), integrated in one continuous volume with visual recall.
- Phase 10: T01/T06/T07 supply Parts 901–1000 (100 lessons), including numerical stability, interpolation, root solving, sampling design and reproducible tests.
- Phase 11: T08/T09/T10 supply Parts 1001–1100 (100 lessons), from frames and projection to calibrated/stereo/physical cameras, with linked recall and independent camera checks.
- Independent XR series: XR01 accepted; source Modules1–19 mapped to its separate narrative. This is not additional main-curriculum progress.

[[Graphics Engine Mastery - Continuous Notes]] · [[Keyword Index]] · [[Dependency Map]]

## Independent university graphics and vision volume

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]] began with all 70 entries from the first three supplied CMU/MIT playlists and all 18 entries from the added Marc Levoy photography playlist. It now also integrates all 26 visual-computing lectures, all 24 MIT projection-theory lectures, all 21 UC Davis graphics lectures, and all 18 added Stanford CS231n lectures: 177 course entries across eight playlists, plus 12 research-paper studies and a research-orientation video. The concept-led volume retains definitions, derivations, worked examples and complete inline labs; its final source legend records all 177 course positions, the research studies, and their distinct evidence types. [[Visual Recall/University - Graphics Vision and Photography - Visual Recall|Its concise recall companion]] links back to lessons and figures. This is independent enrichment and does not advance the numbered 3,000-part curriculum.

## Phase 11 - Projective geometry, cameras, and viewing

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A coordinate is a claim about a frame|The camera section]] integrates all received Parts 1001–1100 / M061–M066, and [[Visual Recall/11 - Projective Geometry and Cameras - Visual Recall|its recall companion]] links short notes to derivations and figures. The selected TUM course legend distinguishes 13 full caption reads and four official-PDF fallbacks across all 17 positions. This independent enrichment does not advance main-curriculum part numbering.

## Approved mathematics consolidation

The six former continuous mathematics files 06–11 are consolidated into [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing|From Vectors to Images — Integrated Mathematics for Visual Computing]]. Their complete teaching content, figures, laboratories and source records remain in that book. The individual phase links below are chapter destinations inside the unified volume. Verified source snapshots are retained outside the vault in `/Users/swayamsingal/.codex/source-cache/math-cleanup-2026-10-29/retired-sources/`. The six original files were retired after the user approved cleanup and the preservation/backlink audits passed.

## Cumulative game-engine volume

[[Continuous Notes/Game Engine - From First Frame to Living Worlds|Game Engine — From First Frame to Living Worlds]] is the single cumulative home for engine architecture and implementation teaching. It combines source-qualified Kohi/Hazel studies, selected passages from the four supplied engine/physics/design books, primary API references, and original runnable laboratories. Future supplied game-engine lessons deepen this same narrative; do not create one file per incoming chunk. [[Supplementary/Game Engine - Sources and Coverage|The per-video and book ledger]] distinguishes captured captions, full versus sampled reading, code evidence, and pending sources. Exhaustive video ingestion remains unfinished; independent enrichment does not advance numbered curriculum coverage.

## Local note-generation tool

[tools/notes_pipeline.py](tools/notes_pipeline.py) retrieves YouTube subtitles and turns local captions or PDFs into reviewable teaching drafts using a local Ollama model. See the [setup and usage guide](tools/README.md) for importing existing sources, processing up to 50 videos at a time, resuming cached work, and reading the source and omission reports. Generated drafts stay outside the live notes until reviewed.


## Latest supplied continuation through Part 2700

The latest received Parts 2157–2700 are integrated across [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From generated GPU work to safe explicit execution|Computing Foundations]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From declared rendering work to editable captured worlds|Game Engine]] and [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From captured kernels to illuminated animated surfaces|Graphics]]. The [[Supplementary/Manual Integration - Sources and Verification#Explicit execution, captured worlds and animation - continuation received 2026-11-11|continuation ledger]] records routing, source preservation and evidence limits. The earlier source backlog remains active; this update does not certify complete coverage of every book or video.

## Additive reconstruction, Phase 1 — C++ choices and replaceable backends (November 11, 2026)

The first reconstruction has started without deleting or rewriting the previously accepted text. An authored, self-contained [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From one owned resource to many requests and a replaceable backend|new C++ chapter]] is placed directly after the existing RAII/ownership development and before the existing API-contract discussion. One realistic request-to-backend problem introduces unions, tagged alternatives, virtual interfaces and destruction, slicing, multiple/virtual inheritance, casts, comparisons, overloads and default arguments only when each becomes necessary. It targets canonical **M011 / Parts 161–180** but is explicitly labelled as *new reconstruction from the curriculum*, not a received source transcript or independent proof of learner mastery.

The complete standalone, commented [[tools/labs/backend_choice_lab.cpp|C++20 executable reference]] was compiled and executed with warnings as errors and address/undefined-behavior sanitizers; exact source-byte comparison was recorded. [[Supplementary/Foundations#M011 — Parts 161–180: a request changes shape and a backend changes implementation|Its 20-part audit mapping]] distinguishes implemented tests, narrative explanations, and remaining verification limits. [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#Requests and replaceable backends — M011 retrieval experiment|Recall prompts]] now reconstruct the same mechanism, not a disconnected vocabulary checklist.

**Historical progress statements above are retained unchanged.** Their earlier “M007–M010 / Parts 101–160” boundary describes the prior received-source snapshot. The new M011 material is an additive authored development, **not** an additional received transcript, and it does not imply that all of Parts 161–300 or the entire curriculum are complete. The external Kairo/Maveb repositories have not been re-audited during this phase, and the standalone lab does not validate actual GPU execution.

**Continuation order:** complete and verify the remaining C++ mechanics in their natural narrative locations (M012, then the broader templates, allocators and algorithms chain), strengthen CPU performance and data-structure evidence, then close the contact-solver gap and the remaining curriculum/research phases. Every later phase should preserve the same existing-text invariant: first make a substantive, coherent teaching addition with an executable or mathematical witness where applicable; only change older prose if a demonstrated contradiction makes correction necessary, with the exact reason recorded.

## Additive reconstruction, Phase 2 — safe callbacks and asset publication (November 11, 2026)

The second phase extends the Phase 1 request/backend story into file bytes, queued work, callback lifetimes, resource acquisition, exception-safe preparation and publication. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From an accepted render request to a resource that survives failures and callbacks|Read the integrated M012 narrative]]. It is intentionally inserted between the new backend-choice chapter and the pre-existing general API-contract chapter, not appended to a standalone list of definitions.

The complete, commented [[tools/labs/safe_asset_pipeline_lab.cpp|C++20 reference lab]] was executed under GCC with address/undefined-behavior sanitizers, Clang with warnings as errors, and a GCC release-style build. It verifies typed byte counts, callable ownership, rollback after acquisition but before publication, non-throwing move contracts and explicit wire-format bytes. [[Supplementary/Foundations#M012 — Parts 181–200: typed asset requests, deferred callbacks and transactional publication|The 20-part mapping]] keeps each original title inspectable; [[Supplementary/Sources and Code Anchors#Phase 2 additive reconstruction — M012 / Parts 181–200, November 11, 2026|the evidence record]] specifies what ran and what did not; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#One request, one transaction, callbacks that may outlive their caller — M012 reconstruction|the recall passage]] reconstructs the same mechanism without replacing the main explanations.

This is **authored coverage of M012**, not a newly received original lesson transcript, a verified audit of independent Kairo repositories, proof of async-thread correctness, or personal mastery certification. Historic earlier README counts are retained as earlier received-source snapshots. The next canonical content is M013 (Parts 201–216), building compile-time generic APIs from the same asset/request limitations. Nothing from the original learning corpus has been deleted by this phase.

## Additive reconstruction, Phase 3 — generic assets and constrained APIs (November 11, 2026)

The third phase extends the same safe renderer-request and asset-publication story from M011 and M012 into generic mesh/texture/shader preparation. Its [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#One asset pipeline, many asset types — generic code without accidental promises|continuous M013 chapter]] develops function/class templates, type/value parameters, full and partial specialization, template deduction, reference collapsing, perfect forwarding, parameter packs, checked fold expressions, SFINAE and C++20 concepts/constraints **when the generic asset store needs each mechanism**. It does not replace any earlier text or subdivide the narrative into sixteen disconnected lessons.

The [[tools/labs/generic_asset_pipeline_lab.cpp|249-line C++20 reference lab]] was compiled and executed with GCC 14.2 sanitizers, Clang 17 warnings-as-errors, and GCC 14.2 release-style flags. A real zero-argument fold warning was corrected, and all three final builds passed. [[Supplementary/Foundations#M013 — Parts 201–216: generic asset preparation with constrained APIs|The 16-title coverage ledger]] preserves the original M013 / Parts 201–216 numbering; [[Supplementary/Sources and Code Anchors#Phase 3 additive reconstruction — M013 / Parts 201–216, November 11, 2026|the evidence record]] documents exact source identity, builds and exclusions; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#One asset pipeline, many types — M013 retrieval|the reconstruction route]] links to the full explanation rather than presenting a second glossary.

**Status boundary:** this is *authored reconstruction*, not a received M013 transcript or proof of personal recall. The separate external Kairo code audits, real GPU execution, dynamic/deleting asset registry, concurrent lifetime tests and quantitative performance comparisons remain unverified. Historical received-source counts stay unchanged. The next canonical chunk is M014 / Parts 217–230; further work must remain additive unless a proven incorrect statement requires a documented correction.

## Additive reconstruction, Phase 4 — compile-time decisions and policy layouts (November 11, 2026)

The fourth reconstruction phase continues the **same** M011–M013 renderer/backend/asset pipeline, asking which work can be decided while compiling and which input validation must still occur at runtime. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A texture tile that chooses its work before the frame begins|The new continuous M014 chapter]] develops trait-driven pixel formats, compile-time checked configuration, an actually derived Morton lookup table, runtime dimensions and overflow checks, `constexpr`/`consteval`/`constinit` distinctions, statically selected budget policies, CRTP, tag dispatch and measured versus guaranteed empty-policy layout. It is inserted before the pre-existing API-contract chapter, directly after the Phase 3 generic asset explanation, rather than appending fourteen separated glossary entries.

The fully commented [[tools/labs/compile_time_asset_policy_lab.cpp|258-line C++20 tile-policy laboratory]] was compiled and executed under GCC 14.2 with address/undefined-behavior sanitizers, Clang 17 with strict diagnostics, and GCC release-style `-DNDEBUG`. All three final executions passed. On the tested Linux environment, plain-member/empty-base/`[[no_unique_address]]` holder sizes were 16/8/8 bytes; that is **not a cross-compiler ABI promise**. The exact [[Supplementary/Foundations#M014 — Parts 217–230: one tile planner and its compile-time policy decisions|fourteen-part curriculum map]], [[Supplementary/Sources and Code Anchors#Phase 4 additive reconstruction — M014 / Parts 217–230|source and verification record]], and [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#The tile planner makes a choice — M014 reconstruction|reconstruction exercise]] distinguish the authored chapter from independently demonstrated mastery.

**Scope:** M014 / Parts 217–230 have *authored teaching and a tested representative CPU laboratory*. This is not a newly received original transcript, a real GPU upload, a complete Kairo/Maveb code audit, a benchmark, or a portable object-layout guarantee. Historic received-source snapshots remain unchanged. M015 / Parts 231–250 will develop the container/storage/iterator/lookup decisions forced by this still fixed-capacity asset model.

## Additive reconstruction, Phase 5 — growing asset storage, lookup and borrowed views (November 11, 2026)

The fifth reconstruction phase starts from the bounded asset depot and tile planner established in M013–M014, then faces the editor's requirement to grow and remove assets without confusing physical addresses with stable logical identity. The continuous [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#An asset registry grows while its handles must not change|M015 growing-registry chapter]] develops vector size/capacity and geometric growth with a derivation; invalidation and `reserve` versus `resize`; array/deque/list/forward_list tradeoffs; ordered map/set and hash-based lookup; collisions, load factors and rehash; scheduling adapters; span borrowing; owning strings, SSO and string_view lifetimes. This chapter is inserted immediately after Phase 4, not appended as twenty disconnected definitions, and it does not replace any earlier prose.

The [[tools/labs/asset_container_lab.cpp|292-line C++20 registry laboratory]] was compiled and executed under GCC 14.2 with ASan/UBSan and strict warnings, Clang 17 with strict warnings, and GCC 14.2 release-style `-DNDEBUG`. All three executions passed; the committed Git blob matches the source actually compiled. The new registry uses a vector of optional asset records, a name-to-handle hash index, a free-slot list and generation-checked handles. It exercises two different failure-injection stages and verifies both rollback and stale-handle rejection after slot reuse. The observed vector growth 2→4 and string capacities 15/1000 are explicitly **implementation-dependent**, not normative performance or layout promises.

[[Supplementary/Foundations#M015 — Parts 231–250: growing the asset registry without losing identities|Original M015 Parts 231–250]] map exactly to the narrative, [[Supplementary/Sources and Code Anchors#Phase 5 additive reconstruction — M015 / Parts 231–250|the source/verification ledger]] states the compiler witnesses and limitations, and [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#A growing asset store must preserve its promises — M015 retrieval|the reconstruction exercise]] follows the same asset lifecycle without replacing the explanation.

**Scope remains bounded:** this is authored curriculum reconstruction, not a received M015 transcript, independently demonstrated learner mastery, a benchmark, real GPU/async retirement verification, or an audit of the current external Kairo sparse-set/render-graph repositories. Historical progress records and the original corpus remain unchanged. Next target: **M016, Parts 251–270**, introduced through the asset registry's new need for algorithms, optional and typed results, sorting, searches, iterators and safe erasure.

## Additive reconstruction, Phase 6 — asset queries, algorithms and precise result contracts (November 11, 2026)

The sixth reconstruction phase continues directly from the M015 generation-aware asset registry. The editor now needs to search, sort, group, summarize and filter assets without confusing an *owned metadata snapshot* with the live registry or breaking iterator and lifetime contracts. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From a living asset registry to reliable queries and transformations|The continuous M016 chapter]] introduces `std::optional`, `std::variant`, `std::any`, expected-style success/errors, pair/tuple and structured bindings, iterator capability/invalidation, sort/stable_sort/partition, the sorted-range contracts of lower/upper/equal bounds, linear find, transform, accumulate/reduce numerical ordering, erase–remove and C++20 erase_if as each problem arises.

The new [[tools/labs/asset_query_algorithm_lab.cpp|316-line C++20 query laboratory]] was compiled and executed with GCC 14.2 using address and undefined-behavior sanitizers, Clang 17 with warnings-as-errors, and GCC 14.2 in release-style `-DNDEBUG` mode; all three reported `asset_query_algorithm_lab: all checks passed`. The lab uses an owned five-asset snapshot and independently confirms range `[1,3)` for the two 48-byte records, exact small-integer total 304, stability of priority ties, iterator constraints, type-erased diagnostic behavior, typed error cases and safe report erasure. The source committed to GitHub was compared to the executed local file by exact Git blob identity.

[[Supplementary/Foundations#M016 — Parts 251–270: the asset registry becomes a query and transformation pipeline|The 20 exact curriculum titles]] are mapped to their actual narrative uses; [[Supplementary/Sources and Code Anchors#Phase 6 additive reconstruction — M016 / Parts 251–270|the evidence ledger]] records the builds, primary standard-library references and excluded claims; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#Query the registry without confusing identity and a snapshot — M016 reconstruction|the active reconstruction route]] asks for reasoning and independent code, not isolated memorization.

**Status boundary:** This is authored reconstruction of M016 / Parts 251–270, not an original M016 transcript newly provided by the user, independently demonstrated learner mastery or a full existing-project audit. Sorting and removing an owned report do not mutate or destroy live registry assets. No actual Kairo, GPU, multithreaded registry, performance benchmark or numerically reproducible parallel reduction has been validated by this lab. All pre-existing prose, source-count snapshots and unresolved source/visual checks remain preserved. The next original chunk is M017 / Parts 271–290, introduced from the need to compose these queries with ranges and manage allocator/module/API boundaries.

### C++20 laboratory regression checks

The new [`.github/workflows/allmight-cpp20-labs.yml`](.github/workflows/allmight-cpp20-labs.yml) discovers all standalone `tools/labs/*_lab.cpp` files and is configured to build/run each with a GCC ASan/UBSan job, a strict Clang job, and a GCC release-style job when relevant files change. This workflow is **configured**, not automatically certified as having passed on GitHub's hosted runner merely because a local lab passed. Check the pull request or Actions history for its actual hosted execution status.

## Additive reconstruction, Phase 7 — lazy ranges, explicit allocation lifetimes and compiled module boundaries (November 11, 2026)

The seventh phase extends M016's owned asset-report pipeline instead of opening a detached list of language features. The new [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From copying every asset report to a lazy, allocatable, separately compiled query service|continuous M017 chapter]] traces a real query through range-for and a tombstone-skipping custom forward iterator, lazy filter/transform views, owning versus borrowed ranges, projected find/sort operations, typed allocators and runtime PMR resources, monotonic arena overflow/bulk release, and the transition to a separately compiled C++20 module API. The explanation derives the exact lifetime, capability, synchronization, numerical and ABI boundaries; no earlier teaching prose is replaced.

Two distinct executable witnesses support the teaching. [[tools/labs/asset_ranges_pmr_lab.cpp|The 281-line C++20 ranges/PMR laboratory]] passed GCC 14.2 with address/undefined-behavior sanitizers, Clang 17 with warnings-as-errors, and GCC 14.2 `-O2 -DNDEBUG`. The [[tools/modules/build_module_lab.sh|GCC C++20 module graph]] builds an exported `:rules` partition, a primary module interface, an implementation unit, two importing translation units, a separately built header unit, and a versioned C ABI wrapper consumed by separately compiled C11 code. This is an *actually executed local GCC14 module build*, not a claim that Clang/MSVC interchange its binary CMI.

[[Supplementary/Foundations#M017 — Parts 271–290: lazy asset queries, allocation lifetimes and compiled API boundaries|The exact twenty-title M017 map]] preserves original curriculum traceability; [[Supplementary/Sources and Code Anchors#Phase 7 additive reconstruction — M017 Parts 271–290|the evidence/source record]] documents native tests, build commands, sources and open limitations; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#From borrowed reports to module boundaries — M017 reconstruction|the reconstruction path]] tests the learner's causal understanding. The existing GitHub Actions C++20 laboratory workflow is extended to trigger on `tools/modules/**` and run a dedicated GCC modules/C11/header-unit job. Hosted CI status is tracked separately from local proof; check Actions/PR results for an actual pass.

**Scope boundary:** this is authored reconstruction of M017 Parts 271–290, not a newly received source transcript, a production graphics module, an independently mastered assessment, a benchmark, a full ABI compatibility certification or an inspection of external `KairoGameEngine/CMakeLists.txt` and `KairoMath/KairoMath.cppm`. Previous source contracts, historical progress markers and original note text are retained. M018 Parts 291–300 next emerges from the need for PIMPL, strong types, generic math and measured abstraction costs at the compilation boundary established here.

### Phase 7 hosted regression — confirmed pass

The [Phase 7 GitHub Actions run #43013050127](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43013050127) completed successfully with **four green jobs**: GCC sanitizers, Clang strict, GCC release, and GCC C++20 modules/header unit/C ABI. The hosted compiler job discovered and executed all **seven** standalone C++ laboratories; the new module job built the real partition/interface/implementation graph and successfully ran its C++ importer, separately compiled C11 consumer and header-unit importer. Exact successes and limitations are recorded in the Phase 7 source/evidence ledger. This is verified hosted CI for the tested sources, not a performance or cross-compiler ABI claim.
