---
tags: [projectallmight, curriculum-governance, narrative-audit, source-closure]
status: normative-contract
date: 2026-11-12
scope: all-canonical-continuous-notes
---
# PROJECTALLMIGHT — Source closure, reconstruction depth and narrative quality contract

> **Status of this document:** a governing *editing and certification contract*, not a completed audit, technical lesson, or claim that the repository meets the requirements. The user's 12 November 2026 adversarial audit is the originating requirements document. Preserve its distinction between actually reading a source, integrating its unique content, teaching it well, reconstructing its implementations, and independently verifying claims.

## The only acceptable end state

A learner should be able to begin without assumed specialist vocabulary, follow a coherent causal story, and eventually reconstruct research-grade mathematics, numerical methods, C++ systems, graphics, vision, rendering, XR, game engines, simulation, ML runtimes, DCC pipelines and experimentation. The story must not achieve breadth by silently truncating detail, shifting essential explanation to an appendix, inserting gigantic disconnected source quotations, or repeating an explanation at every occurrence.

The atomic source of the curriculum remains the **180 semantic chunks and 3,000 original atomic parts**. The canonical continuous teaching volumes are the learning surface, not the coverage ledger. Sources, concept pages, executable labs, and excerpts are evidence or reusable references; they do not replace first-use teaching. Research extensions may add parts beyond 3,000, but cannot change the original denominators or declare previously absent source text to be preserved.

### Important snapshot discrepancy (verified via live GitHub API on 12 November 2026)

The live default branch at the start of this work is **5be7d79c337c3d359569c9531621dd386fa0b7f0**, which is **36 commits ahead** of the audit's **e25a8095815944a19e3ea3a3f69c40b7c90dd6fb** comparison commit. The live tree contains precisely these three physical Continuous Notes files: Computing Foundations, Game Engine, and University/Graphics; the separate Mathematics file and several retained archival notes described in the older audit tree do not exist at the live main head. Their content may be mechanically integrated into current files, but that is a separate preservation test, not an assumption. Some current README and check scripts still describe the older tree: those are configuration/documentation defects to reconcile before certification. Never overwrite newer main data with an older consolidation snapshot.

## Four non-equivalent denominators

**Curriculum contract:** 3,000 parts, 180 chunks, 30 phases. A part's existence in a mapping proves neither that its required source was received nor that the skill is teachable.

**Received source units:** individual book chapter/page ranges; playlist episodes/timecodes and transcripts; specification sections and versions; research-paper sections/figures and related artifacts; repository commit/path/line references. A playlist URL or an entire book title is an *inventory receipt*, not an inspection record. Failed, unsupported, scanned-but-unreadable and postponed units stay open; duplicates must retain provenance.

**Semantic integration:** every unique point from an inspected source has a precise disposition: already deeply taught (with verified destination), needs correction, needs expansion, genuinely duplicative (with matching derivation/evidence), advanced but required, or explicitly out-of-scope with reason. An extracted claim is not the same as the underlying audio or printed chapter.

**Executable / independent evidence:** algorithms should have implementations, traces, invariants and negative tests. Hardware/API behavior also needs spec/version checks and validation or device capture; production-engine integration needs a current commit SHA, inspected call path and actual build/test evidence. A CPU graph compiler is not a Vulkan or Metal backend; a synthetic XR replay is not a human-subject or headset latency result.

Historic review receipts of **26,821 reviewed and 2,894 pending claim texts** relate only to the review-queue snapshot described by the audit. They are neither guarantees of video transcription fidelity nor numbers of S4 parts. The seven fixed mathematics books (4,205 PDF pages), seven later Python/scientific books, four failed book extraction jobs, uncertain caption associations, original Parts 1701–1800 provenance, and all outstanding original source units must remain open until specific evidence closes them. Recalculate counters from the current ledger rather than copying historic totals into a current success claim.

## Depth rubric — never infer a level from length or headings

| Level | What is demonstrably true | What does not suffice |
|---|---|---|
| S0 — indexed | Unique source/concept/part identity is recorded. | A heading, hyperlink or search result. |
| S1 — read | Exact relevant unit and reading evidence are recorded, including extraction defects. | Reading a table of contents or playlist index. |
| S2 — mapped | Every distinct contribution is assigned to a specific destination and disposition. | A broad topic bucket without a first-use anchor. |
| S3 — taught | The actual continuous chapter teaches the forcing problem, prerequisites, reasoning and useful working examples with coherent notation. | A definition dump, paraphrase or unmotivated formula. |
| S4 — reconstructable | The learner can derive, hand-trace, implement, test, break and repair the mechanism, with complete branches, conventions and failure behavior. | Unexecuted pseudocode, a toy that is presented as general, or a linked source file without explanation. |
| S5 — independently verified | Formal/specification claims, code, and environment-specific behavior have recorded authoritative/versioned validation; limitations are stated. | A reference test passing without the device, integration path or claim in question. |

Do not auto-promote a whole chunk from one successful witness. Record the exact claim/submechanism that the witness establishes.

## Teaching-episode structure — follow the problem, not a template dump

The chapter should begin from a concrete *forcing problem* in a system the learner can envision: a point tracked across two coordinate frames, a mutable scene asset passing through an upload queue, a sampled image reconstructed by a headset, or a rigid contact deciding whether to apply an impulse. State what fails with the naive approach, then introduce only the minimal definitions required to fix that failure. A new symbol must receive its units, shape, meaning, coordinate space and lifetime as applicable. No unexplained “obviously”, “as we know”, “see later”, or four-word pseudo-explanations.

Explain the underlying model before deriving the key equations. Show why each algebraic step is legal and where numerical assumptions enter. Continue into the memory representation and the *complete* algorithm, including initialization, ordinary updates, alternate branches, shutdown, concurrent ownership and exceptional/degenerate paths. Do not stop at the happy path.

Provide a fully worked trace small enough to check by hand. Then supply or closely explain runnable code with meaningful names, type/units/contracts and comments documenting why operations exist. Show deterministic positive and negative tests, counterexamples, invariants, tolerance or exactness strategy, and scope limits. Compare at least one genuinely different algorithm or representation using the same problem and measured resource costs when possible. Derive the research extension and what evidence would distinguish competing hypotheses.

A chapter is a **story**: its transitions explain why the reader's current model becomes inadequate and why the next concept follows. It must never read like a collection of unconnected snippets imported from seven sources. The reader should understand the system at each boundary, not discover pages later that an unstated prerequisite was needed.

### Depth without encyclopedic disorder

Avoid an isolated appendix-style exposition when a concept has a genuine first use. Insert the complete base derivation at first meaningful use, and deepen it at the moment a new task stresses the earlier assumptions. For example, teach frames and rigid transforms in the mathematics route, pose estimation when camera/IMU measurements appear, and GPU late-latching only when the XR runtime and compositor need an updated pose. The later chapter may briefly recall an invariant but must not require the reader to go hunting for the basic concept.

A prerequisite may be mentioned before its full use as a *narrative preview*, but may not be relied on before being taught. A large source preservation block must be split or repositioned with provenance references and conservation checks if it destroys the learning sequence. Preserve source text in an archival record when a correction or narrative relocation is required; do not silently delete it or reclassify it as "irrelevant" without a recorded rationale.

### Narrative supervision is an independent gate

For each integrated seam, an editor must read approximately two headings before and two headings after the insertion and evaluate: whether the scene/problem continues naturally; whether voices and levels of abstraction match; whether a definition or formula is introduced for the first time at this location; whether names and conventions agree with earlier chapters; whether the inserted code is introduced and debriefed; whether the next section begins with a now-broken transition; and whether a reader has to follow an archival link to understand a required step.

Use a simple outcome record: **pass**, **rewrite needed**, or **blocked by prerequisite**, with the reason and exact correction. A grammar checker, heading counter or regex for terms is not this review. The reviewer must be able to paraphrase the preceding problem and the next problem and explain why the inserted mechanism connects them.

## Required ledgers

The **source-unit closure ledger** must include stable source ID; source type; precise edition/version/URL/commit/DOI; book chapter and page bounds or episode/video ID and timestamps or paper section/figure; receipt state; actual reading/verification evidence; extraction modality and defects (native captions, aligned ASR, author PDF, scanned/OCR); unique technical claims; exact first-use destination; disposition; current S0–S5 level; executable witness; external verification; duplication/supersession relationship; and the next unresolved action.

The **curriculum-part closure ledger** must include original phase, chunk and part; concept/forcing problem; prerequisite anchors; definitions/notation/units; mathematical derivation; memory/state representation; complete algorithm and branches; hand trace; executable code anchor; edge/degenerate cases; positive/negative/property/metamorphic tests; alternatives/performance tradeoffs; platform or project SHA; research connection; narrative-review decision; and verified depth. A source row is not a curriculum row. Every S4 claim needs both the text location and executable test record.

No unknown source unit may be silently marked "duplicate". Duplicates require a compared claim and destination. No source unit may be called "checked" merely because a tool found a preview or its title in a catalog. No absence may be declared solely from keyword counts. "Not yet audited" is a legitimate status.

## First-use subject custody and boundaries

| Part family | First-use owner / bridge | Mandatory closure concern |
|---|---|---|
| P1–P5: bits, C++, CPU, algorithms | Computing foundations integrated into the engine route | Object lifetime, ABI, language rules, allocators, atomics and lock-free reclamation; reproducible benchmarks. |
| P6–P10: algebra, calculus, statistics, numerics | Mathematics route inside the actual live canonical structure | Conditioned computation, sparse solves, SO(3)/SE(3), constrained optimization, Fourier/PDE and formal inference. |
| P11: cameras | Math geometry → graphics image formation | Frames, inversion, projection, calibration, uncertainty and numerical camera reconstruction. |
| P12–P13: geometric queries | Graphics | Robust predicates, consistent degeneracy classification, collision intersection and CCD. |
| P14–P18: spatial queries, ECS, engine and rigid dynamics | Game Engine | Memory/state ownership, contact consistency, warm-starting and hard provenance hole for Parts 1701–1800. |
| P19–P22: rendering and GPU execution | Graphics | Differentials, sampling, color, shader ABI, cross-vendor GPU behavior and numerical validation. |
| P23–P24: Vulkan/Metal | Game Engine backend + Graphics execution | Versioned resource-state compilation, synchronization2, per-subresource hazards, queue transfers, Metal 4 distinct queue/allocator/argument-table/barrier model; real validation. |
| P25: Gaussian/neural scene rendering | Graphics with mathematics and vision bridges | Training and render derivatives, NeRF/Instant-NGP, 3DGS optimization/densification, anti-aliasing, uncertain or dynamic geometry. |
| P26–P27: advanced light transport, temporal, meshes/animation | Graphics for mathematical/visual parts and Game Engine for ownership | Temporal history validity; path guiding, wavefront and reservoirs; mesh topology and discretization; controller and skinning. |
| P28–P29: OS, emulation, networks, ML runtimes | Engine and systems | Complete network simulation/rollback and tensor→operator→quant→scheduler→backend witnesses. |
| P30: DCC, reliability, research | Engine operations + Graphics experimentation | Asset cooking, versioned schemas, real integration, statistical experimental design, reproducible evidence and XR human study methodology. |
| XR/VR/AR extensions | Math frames/uncertainty → Graphics pose estimation and perception → engine runtime/backend | Inertial propagation, VIO/SLAM, prediction, OpenXR spaces and display times, optics, interaction, human factors; ILLIXR is one testbed, not the whole field. |
| Scientific Python and data extensions | Foundations/Math numerics → Graphics/Engine experiment execution | Protocols, ndarray views/strides, profiling/native boundaries, ODE/PDE/FFT, data cleaning/joins, reproducible figures and uncertainty. |
| Game design and contemporary research extensions | Engine feature narrative + research evidence | Mechanics, dynamics, playtests, level/economy feedback; source-specific frontier claims must be timestamped and reproducible. |

A scope decision must be written explicitly for advanced articulations/FEM/XPBD/fluids, spectral and differentiable transport, modern GI, neural surfaces and game design; they cannot disappear behind "physics" or "graphics complete".

## Mandatory closure examples

**A render graph** must first establish declared resource versions and RAW/WAR/WAW ordering, then derive image-layout/access/stage and memory visibility. A six-pass CPU DAG test is evidence for a dependency *model*, not hardware barriers. Vulkan S5 requires legal emitted synchronization2 barriers, subresource and queue-family transfers, timeline handoffs and validation-layer evidence. Metal 4 S5 separately requires supported hardware, correct MTL4 queue/barrier/allocation lifecycle and GPU evidence. Their resource models are not identical.

**An XR estimator** must state world/body/camera/display frames, units, clock and bias conventions, propagation and measurement residual/Jacobians. A synthetic 2-D or 3-D estimate can reach S4 for the explicitly implemented model but does not prove full VIO, SLAM, or headset tracking. Mark observability, calibration, rolling shutter, outliers, delayed measurements, loop closure and prediction as separate obligations.

**A Python scientific workflow** must distinguish Python objects from contiguous numerical buffers, demonstrate shape/stride/view ownership, profiling and numeric tolerances, then tie collection→cleaning→simulation→inference→figures to a reproducible experiment manifest. Seven book titles do not certify these competencies.

**A 3D Gaussian renderer** must derive covariance and projection and alpha compositing, but also account for training initialization, gradients, optimizer state, densification, pruning and comparison to continuous radiance fields. Correct GPU projection alone is not neural-rendering completion.

**An experimental claim** must specify an estimand, baseline, uncertainty, selection/stopping rules and possible confounds. For XR human studies also specify protocol, counterbalancing, sickness/comfort instrument and ethics/consent as applicable. A profiler's median is not an effect-size or causal inference result.

## Minimal workflow for every future change

1. Pin the current repository commit and inventory the actual affected files; do not trust an older audit snapshot's path or count.
2. Inspect exact source units, with provenance and extraction caveats. Record *what was read* separately from *what was found*.
3. Compare the unique content against the canonical first-use chapter. Assign S0–S5 and distinguish absent, shallow and already reconstructable; do not invent the absence from keyword matching.
4. Develop the passage with forcing problem, derivation, state/code, trace, failure tests, alternatives and a research bridge; explicitly label any intentionally incomplete model.
5. Insert into the narrative at first meaningful use. Verify the surrounding chapter, prerequisite order, tone, duplicated claims, internal links and onward transitions.
6. Run the applicable compiler/tests, source preservation/checkers and link/style QA. For device/project-specific claims, attach separate hardware/commit evidence or leave status open.
7. Update the *two* ledgers and review log, including blocked or unverified items. Only then raise the specific claim's level; do not promote a whole phase by implication.

## Prohibitions and completion declaration

Never announce that all books, videos, papers, 180 chunks, 3,000 parts, GPUs or research extensions are "complete" before the two ledgers reconcile and their stated acceptance evidence exists. Never substitute a source list, broad index, compressed summary or isolated glossary for teaching. Do not silently rewrite supplied source, misrepresent a reconstruction as original provenance, turn a CPU witness into a device claim, label a linked Kairo code path as verified without rebuilding its current SHA, or make the learning route depend on obsolete archived books.

The project's final certification is **per source unit, per atomic curriculum part, per implementation claim, and per narrative seam**. The existence of this contract and the work that first implements it do not imply those certifications have been achieved.
