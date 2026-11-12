# Graphics and Engine Mastery Notes

A growing, self-contained teaching library for the 3,000-part curriculum, organized into continuous subject volumes and reusable cumulative companions.

GitHub repository: `Project_008_AllMight`.


## Three-volume consolidation checkpoint — 12 November 2026

The **only canonical continuous teaching destinations** are **Mathematics**, **Graphics**, and **Game Engine** (linked below). The full former *Computing Foundations* body has been copied without abridging paragraphs or code: bits, C++ objects/lifetimes, CPU/algorithms and APIs into Game Engine; shader memory ABI and GPU algorithms into Graphics; Vulkan/Metal backend and resource-lifetime teaching into Game Engine. The complete original XR teaching is integrated into Graphics, and the original rigid-body and systems/emulator chapters into Game Engine. Original files remain as **source/provenance copies** during the backlink and semantic audit; they are not additional required reading routes.

An executable C++20 affine-frame/reciprocal-basis laboratory is **inline in Mathematics** at coordinate recovery and in `tools/labs/basis_measurement_lab.cpp`. Run `python3 tools/check_three_volume_consolidation.py` to check source inclusion, the inline laboratory and Markdown fences. The C++ program was compiled and run with Clang 17, warnings-as-errors, ASan and UBSan in the authoring environment. This is **mechanical preservation, not a completed conceptual audit**. Earlier-source reading, raw verification and narrative reconciliation remain open; the dated current state is in [[Supplementary/Manual Integration - Sources and Verification#Current reading and merge state after the November 12 pull|the source-review checkpoint]].

The canonical Engine book now also develops [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A spring law is signed, not just a magnitude|signed spring and bungee forces]], audits the supplied book’s sign and endpoint-step discrepancies, and embeds a complete spring witness. Its [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Refresh neighboring penetrations after position correction|neighboring-contact refresh]] includes a complete planar C++ witness and a three-contact ordering trace. The [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Read a game feature at three nested scales|game-system framing]] connects formal rules, participant experience and cultural context through one worked feature. Run `python3 tools/check_continuous_code_sync.py` to verify the declared inline programs; the M018 saved-example checker reads the canonical Engine volume.

## 12 November: independent semantic audit and next implementation witness

The current review ledger, now at **26,821 claim-text points reviewed and 2,894 pending**, is newer than the earlier 14,240/15,475 snapshot and records *reading receipts*, not fully verified audio/books or 29,715 implementations. See [[Supplementary/Corpus Development Audit - 2026-11-12|the change-by-change corpus audit]] for first-use weaknesses, archival-link detours, reading-sequence problems and exactly what the tests do **not** prove.

Game Engine now includes [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From a list of render passes to a proved execution order|an end-to-end logical render-graph compiler]] with a full documented C++20 reference laboratory, a six-pass trace, RAW/WAR/WAW dependencies and negative tests. The source is `tools/labs/render_graph_contract_lab.cpp`. Its exact code is checked inside the Engine narrative by `tools/check_continuous_code_sync.py`, which now covers **thirteen** C++20 witnesses; `tools/check_three_volume_consolidation.py` separately checks the Mathematics affine-basis lab. Passing the CPU graph contract does **not** certify Vulkan/Metal barrier correctness, transient aliasing, or live Kairo/Maveb integration.

## Gregory (2009) — runtime boundary reconstruction, 12 November 2026

A chapter-by-chapter source comparison is retained at [[Supplementary/Gregory Game Engine Architecture - Chapter Audit and Integration]]. The **canonical Game Engine volume** now teaches the previously underdeveloped subsystem hand-offs at their natural first uses: [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A service is not ready merely because its constructor returned — Gregory integration|startup/dependency rollback]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Input travels through two clocks, a focus owner, and a durable tick command|controller dead zones and tick commands]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Resource streaming is a publication protocol, not just a background read|async publication and memory accounting]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A streamed world must preserve authored identity when its objects are absent|streamed identity and fixed-tick phases]], [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Animation proposes motion; a single system commits it|animation/physics ownership]] and [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Correlate one player action through every subsystem hand-off|end-to-end observability]].

The [[Continuous Notes/Game Engine - From First Frame to Living Worlds#One executable world-contract reconstruction — where these hand-offs meet|embedded C++20 end-to-end witness]] is mirrored at `tools/labs/engine_world_contract_lab.cpp` and checked for literal agreement by `tools/check_continuous_code_sync.py`. The existing PR laboratory workflow compiles/runs this new file because it discovers `*_lab.cpp` automatically. Local Clang ASan/UBSan and GCC release builds passed, but this reference proves only isolated CPU state contracts. Actual streaming I/O, GPU retirement, skeletal animation, platform device handling and current Kairo integration remain open. The 2009 source is a historical architecture reference, not authority for modern platform behavior.



The Engine book also extends the deterministic streaming example with [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From a simulated asset job to real concurrent decoding — Gregory integration|actual concurrent decoder workers, safe publication and retirement]]. It explains ownership of queued encoded bytes, condition-variable scheduling, request generations versus simulated GPU-completion fences, package validation, and overlapping residency charges. Its full embedded C++20 reference is `tools/labs/threaded_asset_publication_lab.cpp`; the checker verifies exact-source agreement and the C++ lab CI compiles with `-pthread`. Local Clang ASan/UBSan and GCC release tests succeeded. This does not assert that the real Kairo asset loader, GPU upload, driver resource retirement, or file I/O implementation has been examined.

The animation chapter has also received [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Reconstruct the animation controller, not just the pose formula|a complete, independently tested C++20 controller reconstruction]]: validated clip keys and event markers, shortest-arc quaternion interpolation, temporal crossfades, unwrapped root motion, event crossings across clip loops, collision-limited root authority and render/simulation separation. Its exact source is `tools/labs/animation_controller_contract_lab.cpp`. It is a focused CPU reference, not a skeleton importer, GPU skinning implementation or production character controller.

## Three authoritative continuous teaching routes

For the graphics/geometry/physics implementation track, the **reader-facing teaching lives in exactly three connected continuous notes**, rather than depending on short supplementary summaries. Each teaches the concepts when they first become necessary, with their mathematical assumptions, physical meaning, executable data structures, derived algorithms, numerical traces, failure modes and tests. These are the canonical teaching destinations:

1. **Mathematics** — [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing|From Vectors to Images: Integrated Mathematics for Visual Computing]]. In the numerical-methods narrative, the implementable contact section derives relative motion, quadratic time of impact, stable roots, distance gradients, constraint Jacobians, inverse effective mass, unilateral projection, finite-precision tolerances and conservation/invariance checks. It embeds a complete stand-alone C++20 first-contact computation.
2. **Graphics** — [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference: Graphics, Vision and Photography]]. Inside the collision-geometry narrative, the implementable contact section explains surface witnesses, polygon-face clipping, stable feature identity, CCD, swept motion bounds, the proof of conservative broadphase coverage and its complexity. It embeds the full disk-CCD and swept-AABB CPU witnesses, plus an executable half-space segment clipper.
3. **Game Engine** — [[Continuous Notes/Game Engine - From First Frame to Living Worlds|Game Engine: From First Frame to Living Worlds]]. Inside the mechanics and contact-system narrative, the implementable world-step section explains ownership of body/manifold/solver/cache state, advances to the earliest impact, freezes restitution, solves simultaneous constraints, reschedules the remaining interval, and validates persistent contacts. It embeds complete event-pipeline, persistent-solver and cache-policy C++20 implementations with numerical traces and test interpretation.

The former **Computing Foundations** volume is now an archival source copy: all its paragraphs and code are integrated into Game Engine and Graphics rather than outsourced to a fourth required teaching route. Supplementary laboratory files remain independently buildable exact-source witnesses; the three continuous notes contain the main teaching and relevant complete code so they can be read without a compulsory tour of supplementary chapters. Do not confuse the present physics integration with complete audited ownership of all 3,000 curriculum parts: the original source ledger still identifies unreceived parts, source-review debt, and unverified production-engine behavior.

**Implementation acceptance rule:** a mechanism is not marked taught to reconstruction depth unless the continuous note itself states the forcing problem, definitions with units and conventions, derivation, memory representation, complete state/algorithm flow, commented implementation, worked numerical trace, edge cases, mathematical invariants, runtime limits, and reproducible tests. A link or a code file by itself does not meet this gate. The direct C++20 physics laboratories are a tested independent reference family; current Kairo production integration, GPU behavior, general rotating convex CCD and full hosted CI success require separate evidence.

## Curriculum-wide three-volume responsibility map

The following is a **routing and completion-audit contract**, not a declaration that every listed part is already developed. The original 180 mastery chunks / 3,000 atomic parts remain the coverage checklist; each concept must ultimately be reconstructable from its assigned continuous volume, with a numeric trace, full algorithm, code and failure tests at first meaningful use. A subject may refer to another book for a genuinely different application, but cannot outsource prerequisites needed for its own derivation.

| Original phases | Assigned canonical destination | Complete-teaching obligation |
|---|---|---|
| 1–3: bits, object model, modern C++ | **Game Engine** (systems foundations embedded when used) | Representation, lifetime/ownership, templates, modules, API semantics and complete C++ labs |
| 4–5: CPU and algorithms | **Game Engine** | Cache, SIMD, allocation, concurrency, complexity, data structures, testable implementations |
| 6–10: vectors through statistics | **Mathematics** | Definitions, geometric meaning, proofs, stable numerical algorithms, implementations and error analysis |
| 11: cameras and projection | **Mathematics** for transforms, **Graphics** for image/pixel implementation | Coordinate conventions, projective derivations, calibration and reproducible raster tests |
| 12–13: geometry and collision queries | **Graphics** | Predicates, GJK/SAT/rays, degeneracies, contact generation, proofs, CPU reference code |
| 14–18: spatial structures, ECS, architecture, dynamics and response | **Game Engine** | End-to-end world state, broad/narrow phase, persistent constraints, serialization and engine ownership |
| 19–21: rasterization, PBR and light transport | **Graphics** | Image formation, BRDF derivations, shaders, sampling estimators and numerical/visual comparisons |
| 22–24: GPU execution, Vulkan and Metal | **Graphics** for shader execution, **Game Engine** for resource/backend/lifetime architecture | Full buffer formats, synchronization, commands, shader kernels, backend parity and validation |
| 25–27: Gaussian rendering, advanced real-time and mesh/animation | **Graphics** for representations/rendering, **Game Engine** for runtime ownership | Covariance projection, blending, temporal validity, mesh operators, skinning and motion testing |
| 28–29: emulation/OS and ML runtime | **Game Engine** (reusable systems and runtime foundations) | Executable CPU/emulator/tensor laboratories, file/network contracts, scheduling and performance |
| 30: DCC, reliability, research and capstones | **Game Engine** for build/runtime and **Graphics** for rendering/DCC evidence | End-to-end reproducibility, metrics, benchmark gates, provenance and production acceptance |

The former **Computing Foundations** book is retained as provenance, while all its source paragraphs are now mechanically present in Game Engine or Graphics. This does **not** establish that every concept has received an independent semantic and first-use audit. It should remain available until every unique mathematical, C++ and system-level derivation, code block, failure example and source annotation is accounted for in the three-volume destination. Likewise, the source queue explicitly records unreceived parts and unreviewed book/video claims; source presence, a topic-name match, an extracted claim and an independently implementable explanation are different states. A 30-row map, a 180-chunk catalog or a part-number search alone cannot certify completeness.

## Automated source integrity for the three books

Run `python3 tools/check_continuous_code_sync.py` from the repository root. It checks exact byte agreement for **seven complete C++20 witnesses embedded in Mathematics, Graphics and Game Engine**, and checks display-math/fenced-code boundary parity in all three canonical volumes. Mathematics now includes its own executable first-contact quadratic witness with edge-case tests. The CI workflow [Continuous volume executable source agreement](.github/workflows/continuous-volume-sync.yml) executes that check whenever a canonical volume or its laboratory source changes. The [successful hosted check](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43128537743) confirms the current mechanical agreement. The [five-job GCC/Clang/modules laboratory matrix](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43127394396) also passed after the invalid general-workflow trigger was repaired.

Neither gate proves semantic correctness or complete original-part coverage; that requires source-by-source review, mathematical/implementation evidence and explicit manually audited reconstruction. In particular, the code extracted from an older lesson cannot be treated as current without this check. One stale value-versus-reference structured-binding line was caught and synchronized to the compiled version during supervision.

## Current combined reading route

- **Archived source (not a fourth reading route):** [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs|Computing Foundations]] is retained for comparison; all its teaching is now in Game Engine or Graphics. The original 01 and 02 live files were retired after the approved merge audit; byte-identical recovery copies remain in the checkpoint.
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

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality|XR: From Light to Perception]] is an independent source copy outside the 3,000-part curriculum; its complete teaching now appears in Graphics. XR01 identifies its first supplied source, not M010 or a numbered curriculum part. The anticipated 3–4-source series reuses this same file as later material arrives. Independent module coverage is recorded separately in Foundations; concepts can connect to shared foundations without advancing numbered-curriculum coverage. Future XR sources remain pending.

## First-use completeness and additive narrative revision

The main books are **reader-facing explanations**, not delivery-pass transcripts or an index of topics to learn elsewhere. A concept's first *meaningful use* is the point at which the current equation, algorithm, data structure, code or engineering decision would fail to make sense without that concept. At that point, give the complete depth needed to use it correctly in this system. A link to a glossary, a later chapter, a previous prompt or a different video is never a replacement for that depth. A later chapter may develop the same idea in a genuinely new problem domain, but must not be required to understand the current calculation.

For every nontrivial first use, the narrative must answer **what problem forced this concept, what the quantities mean, which assumptions permit the operation, how the result is derived, what actually changes in memory or state, how the code follows the derivation, what an observable example computes, and what breaks**. Supply numerically checkable intermediate states and important alternatives. Explain symbols and units before relying on them. Establish the scope of a simplified example: scalar and GPU implementations, ideal and finite precision, and CPU and device behavior are different evidence categories. When introducing a specialized term such as covector, effective inverse mass, PDF measure or transmittance, explain the actual mechanism, rather than inserting a dictionary sentence and a link.

A complete explanation **does not mean repeating an entire subject on every occurrence**. If the earlier mechanism was fully taught inside the present reading route, show exactly how the prior invariant applies to the new problem, name any changed assumptions, and include the new derivation or state trace. If the earlier treatment was merely a definition or promise, repair that deficit at this encounter. Depth-preserving compression removes duplicated wording and delivery metadata, never equations, state transitions, constraints, edge cases or independently useful examples.

### Additive merge discipline

New material normally belongs **inside the existing causal chapter at the first use of the relevant mechanism**, not at the tail of a huge volume under a date, pass number or source name. Before writing, inspect the immediately preceding and following paragraphs and search for existing treatments of the same mechanism. If the passage already contains an adequate derivation, add only the missing distinction, proof, example, test or verified correction. If two passages repeat the same explanation verbatim, unify only genuine redundancy and keep every unique worked example, caveat, source citation and runnable code block. Preserve the originals in version history and maintain the exact original-part coverage ledger; do not manufacture completion for unreviewed sources.

Do not retain reader-facing sentences such as "we will explain this in Pass 18" when the present topic needs the mechanism. Replace the scheduling sentence with the missing explanation here. Keep the historical pass number in the source ledger if it identifies provenance. Do not substitute a one-paragraph overview for an algorithm whose correctness depends on a specific invariant or execution order.

A change is **editorially integrated** only when the new explanation is placed at the correct encounter, its local transitions read coherently, terminology and conventions match adjacent material, related earlier claims are reconciled, and its tests/evidence are accurate. Merely appending the input source, preserving all words, passing a Markdown parser, or rendering every equation does not establish semantic integration.

### Proof and execution categories

Explain separately whether code is a full standalone laboratory, a compilable fragment needing a declared API/context, an implementation-specific excerpt, or pseudocode. Full laboratories must compile and run in the declared language mode; a single successful run does not prove platform, concurrency, numerical or GPU correctness. Record expected results, failure conditions and evidence limits. The additional [first-use C++20 contract laboratory](tools/labs/first_use_contract_lab.cpp) exercises coordinate recovery and singularity, stable two-pass radix order, alpha/transmittance compositing, and a two-technique sampling expectation. These small CPU witnesses do not certify the Vulkan/Metal backends.

Use direct links to independent sources and references where they deepen the explanation, but keep enough material locally that the chapter is self-contained. The reader should finish an explanation able to reconstruct the mechanism, not merely able to follow a link.

## Rigid-contact laboratories and reconstruction

The [basic C++20 contact witness](tools/labs/rigid_contact_lab.cpp) covers normal impulses, planar angular response, Coulomb friction and unresponsive contact pairs. The complementary [persistent-contact and two-point manifold witness](tools/labs/persistent_contact_lab.cpp) adds an explicit frozen restitution target, sequential projected impulses, accumulated unilateral and tangential multipliers, single-use warm start and malformed-input tests. The coherent [contact reconstruction chapter](Supplementary/Rigid%20Contact%20-%20Persistent%20Manifolds%20and%20Solver%20Reconstruction.md) derives the contact Jacobian, explains the scalar tangent sign with a numerical example, and separates velocity-level constraints from penetration repair and CCD.

```sh
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/rigid_contact_lab.cpp -o /tmp/rigid_contact && /tmp/rigid_contact
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/persistent_contact_lab.cpp -o /tmp/persistent_contact && /tmp/persistent_contact
```

The separate [GitHub Actions regression workflow](.github/workflows/rigid-contact-lab.yml) runs strict builds and UndefinedBehaviorSanitizer for both witnesses; its remote status must be checked rather than inferred from the workflow file. These independently authored 2D CPU checks do not certify production broadphase, narrowphase manifold generation, Kairo runtime physics, CCD, 3D friction, stable stacking over long simulations or platform-specific GPU behavior.

### Verified contact-cache and continuous-collision witnesses

The [contact feature cache policy lab](tools/labs/contact_cache_lab.cpp) tests canonical body/feature identity, normal-angle invalidation, world-to-current-tangent impulse projection, conservative timestep-ratio handling, material-edit invalidation, duplicate feature detection, and frame-to-frame stale pruning. It assumes the narrow phase has **already** produced stable feature identifiers; it is not itself a polygon clipping or feature matching algorithm.

The [swept-disk continuous collision lab](tools/labs/disk_ccd_lab.cpp) reconstructs exact linear-motion circle-circle time of impact from relative motion and a quadratic, with numerically safer root computation, initial-overlap policy, grazing contacts, near misses, relative-motion invariance, and parameter validation. The expanded [contact and CCD teaching chapter](Supplementary/Rigid%20Contact%20-%20Persistent%20Manifolds%20and%20Solver%20Reconstruction.md) gives the mathematics, data representation, numerical trace, caching decisions and failure cases next to the preceding impulse-solver explanation.

```sh
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/contact_cache_lab.cpp -o /tmp/contact_cache && /tmp/contact_cache
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/disk_ccd_lab.cpp -o /tmp/disk_ccd && /tmp/disk_ccd
```

Both new sources were compiled and executed locally with strict C++20 compiler diagnostics and combined AddressSanitizer/UndefinedBehaviorSanitizer. The GitHub copies of the tested sources were re-fetched and hash-matched against their local counterparts. The workflow now includes both. The analytic disk oracle **does not** implement general convex-shape/rotating-body CCD, continuous impulse resolution, production event scheduling or native Kairo simulation; remote GitHub Actions success is separately unconfirmed.

### Complete disk impact event and randomized invariant witnesses

The [event-pipeline C++20 lab](tools/labs/disk_event_pipeline_lab.cpp) connects all-pairs disk contact discovery, earliest time-of-impact selection, advancement to impact, simultaneous normal-impulse iteration, re-querying after velocity changes, and advancement through the remaining step interval. Its nine deterministic scene suites cover tunneling against a static disk, elastic/inelastic response, simultaneous and sequential contacts, separating motion, overlap rejection, exhausted event budgets, and identity/duration validation. The companion [seeded property regression](tools/labs/disk_event_property_lab.cpp) checks **10,000 isolated unequal-mass collisions** over a range of closing speeds and restitution coefficients for event count, outgoing relative speed, linear momentum and non-increasing kinetic energy.

An initial randomized run discovered a genuine floating-point failure: event advancement could leave a near-tangent pair microscopically overlapping because the old squared-gap tolerance did not scale appropriately with translated world coordinates. The event lab now uses a linear surface-gap tolerance scaled by the world-coordinate magnitude. The corrected source and property test passed locally under GCC, Clang, and combined address/undefined-behavior sanitizers; both exact published files were re-fetched and compared against the tested local sources using Git blob hashes. The complete derivation and step-by-step example are integrated in [the contact and CCD chapter](Supplementary/Rigid%20Contact%20-%20Persistent%20Manifolds%20and%20Solver%20Reconstruction.md).

```sh
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/disk_event_pipeline_lab.cpp -o /tmp/disk_events && /tmp/disk_events
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/disk_event_property_lab.cpp -o /tmp/disk_properties && /tmp/disk_properties
```

This is a **self-contained disk-only CPU oracle**, not Kairo production physics. It has an O(n²) pair scan on every event, assumes piecewise constant velocity and instantaneous frictionless impulses, and does not handle acceleration, angular motion, persistent resting manifolds, deep penetration correction, general convex CCD or engine integration. Exceeding the event budget raises an exception, potentially after partial in-place state updates. GitHub-hosted Actions success remains separately unverified.

### Conservative swept broadphase and exact candidate coverage

The [C++20 swept-AABB broadphase lab](tools/labs/swept_broadphase_lab.cpp) computes outward-rounded motion bounds for constant-velocity disks, sorts by minimum x, filters active intervals by x and y overlap, and returns deterministic unique candidate pairs. It does **not** claim that every candidate truly collides; the narrow phase is still responsible for time-of-impact. The independent analytic disk collision oracle verifies that **no true collision pair is lost**, across deterministic scenarios and **200 seeded 35-disk scenes** (119,000 independently evaluated unordered pairs). GCC, Clang, and combined address/undefined-behavior sanitizer runs passed locally; the committed source matches the exact locally compiled Git blob. The [physics derivation chapter](Supplementary/Rigid%20Contact%20-%20Persistent%20Manifolds%20and%20Solver%20Reconstruction.md) explains why the bounds are conservative, why false positives are acceptable, and why the simple active-set implementation still has O(n²) worst-case behavior.

```sh
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tools/labs/swept_broadphase_lab.cpp -o /tmp/swept_broadphase && /tmp/swept_broadphase
```

This is a separate verified candidate-generation oracle. It is **not yet used by the independent disk event stepper** or any Kairo production collision path, so no broadphase performance improvement is claimed for the engine. The workflow includes this laboratory, but GitHub-hosted CI completion remains separately unverified.

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

## Phase 3 — first-use narrative integration

The live [Integrated Mathematics volume](Continuous%20Notes/00%20-%20From%20Vectors%20to%20Images%20-%20Integrated%20Mathematics%20for%20Visual%20Computing.md) now begins with the robot inspection problem instead of requiring a glossary-like preflight. The earlier compact field/matrix/determinant/covector/adjoint definitions have been *moved intact* to a reference recap at the end of the Phase 6 physical investigation; full first-use development remains in the causal chapters. In the live [Game Engine volume](Continuous%20Notes/Game%20Engine%20-%20From%20First%20Frame%20to%20Living%20Worlds.md), the rigid-body sequence begins with a persistent 2 kg crate example that derives box inertia, an off-centre force's torque, angular acceleration, and a fixed-timestep update before proceeding into the original Part 1601–1700 material. Those numbered source passages and their unique derivations remain retained. The companion contact chapter and limited C++20 witness remain linked below. These are editorial improvements, not a claim that all remaining large appended batches or first-use gaps have been reviewed.

## Continuous rigid-body teaching route (Phase 2)

The additive [rigid-body narrative](Continuous%20Notes/Rigid%20Bodies%20-%20From%20Extended%20Motion%20to%20Contact%20Constraints.md) develops the full causal chain from extended motion and inertia to normal impulses, accumulated contact constraints, friction, correction, implementation contracts and failure tests. It is intended as a coherent companion to the existing Game Engine volume's Parts 1601–1700, **not** a replacement for its retained original teaching or provenance ledger. The [C++20 central-contact witness](tools/labs/rigid_body_contact_lab.cpp) checks momentum, restitution, non-attractive separation and accumulated-impulse clamping. It does not certify the three-dimensional solver. Future semantic reweaving should put relevant first-use derivations directly in the main volume once its oversized Markdown file can be safely patched.

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


## Additive reconstruction, Phase 8 — M018: stable engine API identity, PIMPL and measured cost (November 11, 2026)

The final Phase 3 chunk is now **authored** as [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A registry crosses its last C++ boundary: hidden implementation, typed identity and measured abstraction cost|a single continuous engine-system chapter]] immediately following M017's range/allocator/module boundary and before the pre-existing general API-contract narrative. Its original **M018 / Parts 291–300** remain exactly mapped in [[Supplementary/Foundations#M018 — Parts 291–300: the same asset service becomes a typed, hidden and measurable engine API|the canonical coverage ledger]], with [[Supplementary/Sources and Code Anchors#Phase 8 additive reconstruction — M018 / original Parts 291–300, November 11, 2026|source and evidence scope]] and [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#Preserve an asset while the engine changes — M018 reconstruction|active reconstruction prompts]].

The new [[tools/m018/m018_lab.cpp|C++20 test program]] is separately compiled and linked with [[tools/m018/asset_catalog.cpp|a private PIMPL implementation]], instead of pretending a single header-only example proves a compilation firewall. It exercises kind- and catalog-safe generation handles, reuse/invalidation, owned lookup snapshots, moved-catalog policy, frame/unit-tagged fixed vectors, dynamic span length checks, borrowed lazy-expression lifetime and intrusive list membership. [[tools/m018/build_m018_lab.sh|The build script]] runs GCC optimized release with checks active under -DNDEBUG, Clang warnings as errors and GCC ASan/UBSan; it also emits real GCC and Clang assembly for a named generic function. The existing GitHub workflow has a separate multi-translation-unit M018 job. Check the hosted Actions run before claiming it passed.

**Scope of claims:** authored teaching and a representative CPU contract, **not** a newly supplied source transcript or independent proof of reader mastery; no current KairoMath/KairoRenderer audit, native Vulkan/Metal resource-lifetime test, dynamic plugin ABI certification, real CPU performance comparison or exhaustive concurrency/fault-injection proof. Previous source and caption-review backlogs are still active. Existing teaching was preserved; this was an additive insertion at its dependency seam.



### Phase 8 hosted regression — confirmed pass

The [M018 GitHub Actions run #43038422705](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43038422705), for source commit 9226f19e867fc89cef869f209408d434daaaf27f, completed with **five successful jobs**: GCC sanitizer, GCC release, Clang strict, the existing C++20 module/header-unit/C ABI graph and the new separately compiled M018 PIMPL/type-safe/generic-math laboratory. The M018 executable additionally exercised 128 asset insertions and verified that the original logical handle survived vector growth. The two-compiler assembly outputs were generated and checked for a named function. These are actual hosted regression results, not throughput benchmarks, GPU execution or cross-toolchain ABI certification.


## Phase 9 — CPU architecture, memory systems, concurrency and profiling (Parts 301–400)

The additive CPU reconstruction now follows directly from M018's owned asset service. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From a correct C++ asset service to the instructions a CPU can execute — Phase 4, M019–M021, Parts 301–346|The first chapter]] works from source-level arithmetic to instruction dependencies, cache/TLB/memory traffic, AoS/SoA/AoSoA, false sharing, SIMD and numerical contracts. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From temporary memory to finished parallel frames — Phase 4, M022–M024, Parts 347–400|The second chapter]] continues through allocators, object lifetimes, deferred GPU resource safety, C++ atomics, locks/condition variables, persistent jobs, deterministic reductions and profiler/Amdahl reasoning.

The exact original 100 titles are preserved in [[Supplementary/Foundations#Phase 4 CPU architecture and measured execution — Parts 301–400|the coverage ledger]], with runnable [[tools/labs/phase4_cpu_systems_lab.cpp|CPU reference lab]], [[Visual Recall/Phase 4 - CPU, Memory and Job Systems - Visual Recall|reconstruction exercises]] and [[Supplementary/Sources and Code Anchors#Phase 9 — Canonical CPU execution, memory, jobs and profiling (M019–M024 / Parts 301–400)|bounded source/evidence record]]. The text is authored reconstruction rather than a newly received Phase 4 transcript or proof of personal mastery. The lab does not establish real Kairo/Maveb behavior, work stealing, CPU hardware-counter performance or native Vulkan/Metal completion.


## Phase 10 — Data structures and algorithmic reasoning (Parts 401–500)

Following the completed CPU phase, the continuous computing volume now develops **all six original Phase 5 mastery chunks** through an asset catalog, an editor query, a navigation graph, a render-pass dependency graph, spatial ordering and an LRU streaming cache. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#The scene is now too large for the wrong container — Phase 5, M025–M027, Parts 401–445|The first connected chapter]] derives ADTs, complexity, amortized growth, arrays/rings/hash tables, Bloom filters, ordered trees/heaps/tries and DSU. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From an asset map to a correct navigable world — Phase 5, M028–M030, Parts 446–500|The second]] derives graph traversals and paths, MSTs, search/sort tradeoffs, Morton ordering, scans, DP, randomized sampling, caches and sparse-set/generation contracts.

Every canonical part 401–500 is mapped in [[Supplementary/Foundations#Phase 5 algorithms and data-structure workload contracts — Parts 401–500|the original-title ledger]], with the [[tools/labs/phase5_algorithm_workload_lab.cpp|C++20 executable laboratory]], [[Visual Recall/Phase 5 - Algorithms by Workload - Visual Recall|reconstruction tests]] and [[Supplementary/Sources and Code Anchors#Phase 10 — Algorithms chosen by workload (M025–M030 / Parts 401–500)|source and evidence record]]. The text is **authored curriculum reconstruction**, not newly received Phase 5 transcripts or independent certification of current Kairo implementations. Specialized trees, GPU scans and per-engine performance claims remain distinct future verification requirements.


## Additive reconstruction — Phase 28, Parts 2701–2800 (November 11, 2026)

A new continuous [[Continuous Notes/Systems - From Cartridge Bytes to Reproducible State|Systems volume]] connects CPU emulation and an NES cartridge to OS file publication, network framing and cryptographic trust boundaries rather than treating these as isolated features. [[Supplementary/Foundations#Phase 28 M163–M168 — Parts 2701–2800 deterministic and trusted low-level systems|All 100 canonical part titles]] are mapped, with separate status for executable CPU evidence and narrative-only OS/network/security mechanisms. The [[tools/labs/phase28_emulator_core_lab.cpp|C++20 CPU/iNES NROM lab]] includes typed bytes, reset vectors, RAM/PRG mapping, selected 6502 opcodes/flags, controller/OAM stand-ins and versioned snapshot replay. [[Visual Recall/Phase 28 - Systems From Emulator to Trust - Visual Recall|The recall companion]] asks for complete reconstruction and failure reasoning.

**Do not infer a complete emulator or secure distributed system.** The lab tests a narrow 6502/NROM subset; no actual PPU/APU, full opcode suite, network service, authenticated cipher, filesystem durability test, PyO3 integration or current NESEmulator/ReliQuary source audit was performed. The text preserves all earlier content and pending source queues. Check the current GitHub CI run rather than extrapolating from local descriptions.

