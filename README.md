# Graphics and Engine Mastery Notes

A growing, self-contained teaching library for the 3,000-part curriculum, organized into continuous subject volumes and reusable cumulative companions.

GitHub repository: `Project_008_AllMight`.

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
