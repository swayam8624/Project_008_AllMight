# Manual integration — source coverage and verification

**Current revision:** the integrated math book has the direct textbook expansion; original bits/C++ files are retired after approval. The retained earlier corpus has 14,240 claim texts reviewed and 15,475 pending. Detailed review and integration are still incomplete. See [[Supplementary/Math Book Expansion - Teaching and Verification]].

This ledger accompanies the concept-led graphics, game-engine and combined computing-foundations volumes. It distinguishes source-body integration, direct teaching review, extracted-point comparison, and executable/numerical checks. A source being present is not a claim that every statement or repository observation is independently verified.

## Supplied Parts 1101–1700

All 18 supplied source bodies are retained in the expanded volumes after heading/metadata and mathematical-markup normalization. Original attachment bytes and hashes are preserved in the checkpoint. Every original fenced code body is retained unchanged. Numerical part labels are provenance, not the teaching chapter structure.

| Source | Canonical parts | Destination |
|---|---|---|
| P01 | 1101–1132 | graphics |
| P02 | 1133–1178 | graphics |
| P03 | 1179–1200 | graphics |
| P04 | 1201–1239 | graphics |
| P05 | 1240–1270 | graphics |
| P06 | 1271–1300 | graphics |
| P07 | 1301–1336 | engine |
| P08 | 1337–1374 | engine |
| P09 | 1375–1400 | engine |
| P10 | 1401–1434 | engine |
| P11 | 1435–1474 | engine |
| P12 | 1475–1500 | engine |
| P13 | 1501–1545 | engine |
| P14 | 1546–1582 | engine |
| P15 | 1583–1600 | foundations and engine |
| P16 | 1601–1642 | engine |
| P17 | 1643–1681 | engine |
| P18 | 1682–1700 | engine |

P15 is partitioned: general build/tooling/API material belongs in computing foundations; its source-to-editor-to-runtime ownership flow belongs in the engine volume. The page/heading routing map and exact omitted editorial metadata remain in the checkpoint.

## Existing notes and proofs

The pre-integration graphics and engine teaching remains in order. The combined foundations volume contains the complete substantive bodies of the original bits and C++ notes. The two original files remain available as recovery sources. Six stale references to a deleted derivations file are repaired in the combined volume by recovering the relevant saved proofs inline. Those references were not treated as evidence that the deleted file still existed.

Repository-specific Kairo observations in supplied passages remain supplied snapshot claims. They were not independently recompiled or re-audited against the current Kairo repository in this integration. Code fragments are not all standalone programs.

## Caption and earlier book extractions

The frozen input inventory contains **590 completed extraction jobs**: 457 video jobs with 24,718 extracted points, and 133 book-page jobs with 4,997 extracted points. The four failed book jobs and the paused/unprocessed jobs are not silently included as complete. The extracted point text totals roughly 510,000 words.

The original caption-based engine narrative is retained. The first new targeted synthesis directly checked 120 selected point IDs across 24 video sources and one Game Engine Architecture extract. One selected video source, Kohi #021, is generated ASR and is explicitly qualified. The broader comparison of all frozen extracted points is ongoing in three balanced groups; it has not yet established complete semantic coverage of the whole extraction inventory.

The caption-validity audit remains a separate check: raw hashes, cue parsing and lossless cue-to-reading-span ancestry passed for its 457-source snapshot. This is not word-level audio verification. Kohi Episodes 101 and 102 remain flagged for near-duplicate association review; they are not used as distinct new teaching evidence here.

## Newly supplied books

These seven books total **4,205 physical PDF pages**. All are registered; text was mechanically made accessible where possible, and selected sections were directly read for the new teaching. **None is claimed fully read or comprehensively taught in this pass.** Arnold is scanned; the mechanics review actually viewed selected rendered pages. Page ranges and unread chapters are recorded in the per-topic review files.

| Book | Physical PDF pages | Current use |
|---|---:|---|
| All of Statistics.pdf | 458 | Selected probability, estimation, concentration, intervals and Bayesian sections |
| Arnold Mathematical Methods Classical Mechanics 1989.pdf | 536 | Selected scanned rigid-body/rotation pages, physically viewed |
| Bandit Algorithms.pdf | 597 | Selected confidence exploration, importance weighting, regret, nonstationarity and pure-exploration sections |
| Book_Polygon Mesh Processing.pdf | 243 | Selected mesh representations, differential operators, remeshing and repair sections |
| BV Convex Book.pdf | 714 | Selected convex sets, projection/separation and support-function foundations |
| Edelcomp (1).pdf | 294 | Selected complexes, chains, homology, Euler characteristic and persistence sections |
| Mathematical Methods for Physics and Engineering.pdf | 1363 | Selected Fourier/eigenfunction, boundary-value, heat and Green-function sections |

The new textbook-based narrative defines probability spaces and observation models, contrasts confidence and posterior intervals, derives importance weights, explains UCB assumptions and adaptive-stopping bias, develops mesh boundary operators and topology checks, and connects heat modes to discrete mass/stiffness operators. These additions enrich the existing subject coverage; they do not substitute for an end-to-end traversal of all seven books.

## Verification and concrete corrections

- The original notes, supplied fenced code, normalized source-body placement, and Markdown fence balance are checked by `check_preservation.py`.
- MathJax rendering and Mermaid rendering are checked separately. Final counts are recorded in the current validation reports. Rendering success does not prove mathematical correctness.
- Thirty-five independent numerical witnesses check geometry, AABB transforms, normal transforms, SAH arithmetic, A* reopening, COM/inertia/impulses, damping, optional stopping, importance weighting, topology boundary maps/Betti ranks, cotangent energy and diffusion stability.
- The inline C++20 handle laboratory compiled with `-Wall -Wextra -Werror -fsanitize=address,undefined` and executed its assertions without sanitizer diagnostics. Maximum 64-bit generation retirement, concurrency and GPU behavior are outside the executed test scope.
- Supervisor corrections include the wrong first-success sample-mean example, fixed-count versus stopped averaging, unit conversion versus physical resizing, an erroneous double-timestep number, ordinary H0 elder-rule pairing, stiffness-energy convention, ASR/caption classification, handle liveness/bounds, integer picking versus color encoding, and first-use definitions.
- Two new PNG/SVG teaching figures have numerical witnesses and were visually inspected. Native whole-volume Obsidian/PDF pagination and every imported C++ snippet have not been validated.

## Durable checkpoints

- [Work state](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/WORK_STATE.json)
- [Source hashes and heading inventory](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/sources.json)
- [Destination map](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/route.json)
- [Frozen extracted-source inventory](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/extracted-source-manifest.json)
- [Topic review files](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/reviews)
- [Validation artifacts](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/validation)

The next required work is the remaining extracted-claim/source comparison, any resulting narrative deltas, the outstanding full-book traversal, and subsequent semantic/code/visual checks. These remain explicit work rather than being hidden behind a completed extraction count.


## Rendering and GPU addition — Parts 1801–2156

All nine supplied texts R01–R09 are preserved as byte-identical input copies with hashes. Their complete normalized teaching bodies are routed into the existing books, with source code bodies retained. Editorial batch metadata, unearned “depth gate passed” statements and unresolved citation stubs are excluded from teaching prose and remain recoverable in the raw inputs. Parts **1701–1800 have no supplied source in these batches** and are not marked received.

| Source | Received parts | Teaching destination |
|---|---|---|
| R01 | 1801–1845 | Graphics: rasterization, clipping, interpolation and depth |
| R02 | 1846–1900 | Graphics: fragment tests, compositing, draw submission and diagnosis |
| R03 | 1901–1944 | Graphics: color/radiometry and texture sampling, split at the signal-processing boundary |
| R04 | 1945–1986 | Graphics: reflectance, microfacets and physical material interpretation |
| R05 | 1987–2000 | Graphics: material semantics and validation; foundations: packed records, layout and descriptor contracts |
| R06 | 2001–2030 | Graphics: visibility queries and Whitted/Monte Carlo transport, split at the renderer boundary |
| R07 | 2031–2069 | Graphics: sampling, MIS, throughput, next-event estimation and variance |
| R08 | 2070–2100 | Graphics: media, reproducible streams, CPU rendering and validation |
| R09 | 2101–2156 | Foundations: GPU execution, memory, synchronization and parallel algorithms |

An additional engine chapter connects renderer changes to ownership, publication, deferred retirement, resource dependencies and accumulation-history validity. Earlier saved circle-rendering/picking and Rules of Play teaching is also integrated; the new rendering batch does not supersede older source work.

Directly authored teaching follows one workshop scene through geometry, coverage, material response and light transport. New first-use explanations include samples versus fragments, barycentric versus perspective weights, projected depth, solid angle, color encoding, PDF measures, microfacet normalization, discrete light choice, reflection versus transmission support, GPU residency and scan as an allocation proof.

### Corrections made during this rendering pass

- The supplied PBR numerical trace used mutually incompatible half-vector dot products. The replacement constructs feasible unit directions, derives the half-vector dot products, and recomputes the GGX/specular trace.
- Taking only the first five cells of a nine-cell stratification can bias a full-pixel estimator. Both source discussions now explain the missing support. A concrete integral has expected estimate 0.3 rather than 0.5.
- RGB channel roulette is described as a shared-path design choice; independently compensated channel estimators are not declared mathematically impossible.
- The convergence exponent distinguishes mean-square error from root-mean-square error, and reference noise/correlation/bias are qualified.
- The source's placeholder matrix-tile `valid` mask is not a complete boundary policy. Separate left and right load masks and complete barrier participation are taught explicitly. Seven CPU matrix-oracle cases cover partial tiles and an empty inner dimension; no GPU execution is claimed.
- Retained direct-light loops with a positive-normal cosine are qualified as reflective-hemisphere estimators, rather than complete rough-transmission estimators.
- A malformed unused-field expression is repaired for MathJax. Added Markdown prose whitespace is normalized; original code bodies remain unchanged.

### Primary sources checked for targeted contracts

These are selected relevant sections, not a claim to have reviewed every page of each specification or textbook: [Vulkan rasterization](https://docs.vulkan.org/spec/latest/chapters/primsrast.html), [Vulkan synchronization](https://docs.vulkan.org/spec/latest/chapters/synchronization.html), [Khronos shader memory layout](https://docs.vulkan.org/guide/latest/shader_memory_layout.html), [glTF material specification](https://raw.githubusercontent.com/KhronosGroup/glTF/main/specification/2.0/Specification.adoc), [PBRT microfacets](https://pbr-book.org/4ed/Reflection_Models/Roughness_Using_Microfacet_Theory), [PBRT complete path integrator](https://pbr-book.org/4ed/Light_Transport_I_Surface_Reflection/A_Better_Path_Tracer), and [Nathan Reed's depth analysis](https://developer.nvidia.com/blog/visualizing-depth-precision/).

### New verification and retained earlier work

`validation/rendering_witnesses.py` executes 20 numerical/layout witnesses, seven matrix-tile CPU emulations, nine source-body/code-preservation checks and three previous-volume preservation checks. MathJax, Mermaid and link checks are recorded separately in the validation directory. These prove the reported layer; they do not establish current Kairo/Maveb behavior, every snippet's compilation, whole-book semantic correctness, native Obsidian performance or completed full-corpus review.

The new PNG/SVG figure compares perspective interpolation, linear versus encoded averaging, and missing-strata bias; its exported image was visually inspected. Recall companions include short explanatory answers and links to the corresponding teaching and figure explanation.

**Earlier material is still required work.** [[Supplementary/Manual Integration - Pending Source Queue]] records all 590 earlier extraction jobs and all seven mathematics books. The exact queue contains 29,715 extracted points: 14,183 claim texts read in saved reviews and 15,532 pending. All 1,180 referenced point/unit JSON files existed and were hashed when this queue was written. Claim reading, raw verification and narrative integration remain separate statuses.

- [Nine input hashes and headings](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/rendering-sources.json)
- [Nine-source routing](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/rendering-route.json)
- [All earlier source records and pending point IDs](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/ALL_SOURCES_BACKLOG.json)
- [Rendering witnesses](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/validation/rendering-witnesses.json)


### Proof navigation recovery during the wider link check

The wider active-file audit found stale links to the retired derivations file. Complete Phase 6–8 and XR proof sections were recovered from the retained cleanup snapshot into their corresponding active books; navigation now targets those inline sections. The combined computing book already contains the recovered bits/C++ proofs. Original 01/02 recovery files retain their original bytes. See [proof recovery receipt](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/proof-navigation-recovery.json).


## Corrective mathematics expansion and approved bits/C++ retirement

The integrated mathematics book now contains a direct textbook-based expansion at 12 concept positions. [[Supplementary/Math Book Expansion - Teaching and Verification]] records the page-level reading, new definitions and derivations, supervisor corrections and numerical checks. Earlier statements in this historical ledger that originals remained live describe the prior revision. The user subsequently authorized retiring the two original bits/C++ files. Their complete teaching bodies and all 358 fenced bodies were verified in the combined foundations volume; remaining backlinks were migrated and a full active-vault target audit passed with the originals simulated absent. Byte-identical recovery copies and exact hashes are retained in the [retirement receipt](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/math-repair-2026-11-11/retirement-complete.json). The two original files are now removed from the live vault. Earlier caption/book review remains unfinished.


### Root caption review continuation — ten records

The next ten retained video records were checked against their cited raw caption units: 57 extracted claims were read and compared with their evidence. Incorrect animation and platform-testing claims were rejected; historical announcements are retained as catalog/roadmap evidence, not implemented features. Animation workflow, method inputs/composition order, packaging dependencies and API capability contracts were developed in the engine/foundations books. Current claim-text totals are **14,240 reviewed and 15,475 pending**, superseding the earlier snapshot counts above. This is still an unfinished whole-corpus review. [Exact source/point decisions](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/math-repair-2026-11-11/caption-batch-01-review.json).


## Explicit execution, captured worlds and animation - continuation received 2026-11-11

The eight new source attachments cover Parts 2157–2700. They are added to the existing volumes: GPU-driven systems, Vulkan and Metal in Computing Foundations; render-graph architecture and editable captured-world correspondence in Game Engine; Gaussian kernels/rasterization/validation and lighting/mesh/animation in Graphics. Existing book content is retained. The original supplied files have byte-identical checkpoint copies; normalized teaching bodies retain every original fenced-code body. Editorial part labels and source boilerplate are recorded separately from teaching content.

| Source | Received parts | Main destination |
|---|---|---|
| GPU-driven systems | 2157–2200 | Computing Foundations |
| Vulkan device, storage and pipelines | 2201–2264 | Computing Foundations |
| Vulkan commands, synchronization and RAII | 2265–2300 | Computing Foundations |
| Metal execution and language bridges | 2301–2350 | Computing Foundations |
| Multi-backend renderer and render graph | 2351–2400 | Game Engine |
| Gaussian representation and projection | 2401–2452 | Graphics |
| Gaussian rasterization and CPU/GPU comparison | 2453–2486 | Graphics |
| Editable captured-world correspondence | 2487–2500 | Game Engine |
| Lighting, reconstruction, meshes and animation | 2501–2700 | Graphics |

The explanatory extension adds definitions, projection/compositing/retirement witnesses and API-specific corrections. The original-source heading register includes 1,349 headings; 762 fenced code bodies are preserved. The shared [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Figure ER1 - Footprint, compositing order and storage lifetime|footprint/order/lifetime visual]] is linked from the [[Visual Recall/University - Graphics Vision and Photography - Visual Recall#Explicit rendering, captured kernels and animated surfaces - retrieval briefs|nine retrieval briefs]].

Verification distinguishes source retention from implementation proof. The CPU retirement model was compiled as C++20 with warnings as errors plus address/undefined-behavior sanitizers and executed successfully. Numeric witnesses check the projected covariance, conservative footprint radius, scan offsets, compositing order and affine covariance. No native Vulkan/Metal run or current Kairo/Maveb/Vulkax repository audit is established by those tests. In particular, supplied statements saying a project “currently implements” a feature remain attributed draft observations. Full earlier video coverage and full reading of every book remain unfinished; an extracted claim, a reviewed claim and an integrated explanation are separate states. Parts 1701–1800 remain unreceived in these batches; future Parts 2701–3000 are not implied by this continuation.

Exact hashes, source partitions, complete heading registers, review artifacts and test receipts are retained under `/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/continuation-2026-11-11/`. This checkpoint extends the earlier corpus queue; it does not replace it.


### Supervised continuation corrections and earlier-book additions

The continuation now also develops image-change certificates with explicit color/order/termination assumptions and a fixed-order compositing derivation. CPU tests execute 4,000 randomized inequality checks and explicit opacity/color/order counterexamples. Direct inspection of the reference Gaussian CUDA kernel adds a separate caveat for candidate-transmittance stopping before layer accumulation. These are mathematical and CPU bookkeeping checks, not native GPU or current project conformance tests.

Earlier-book/source additions include document purpose and consistency, historical creator interviews, coupled Centipede rules, interactive writing, implicit rules and cultural values, replay/recam, player-created systems, procedural representation, camera policy and contact-response mathematics. The contact derivation distinguishes six configuration degrees of freedom from a twelve-dimensional phase state, and the kinematic constraint Jacobian from the contact-space response matrix. Its three-mass CPU witness checks the coupling terms. Other additions clarify scene save-path/write-failure policy, Euler delta rotation, framebuffer numeric types and shader program activation. Original source statements remain retained beside explicit qualifications.

The live vault moved to `/Users/swayamsingal/Desktop/All Might/Master` during the latest resume. Engine and Graphics matched saved hashes at relocation; an independent 2,433-line insertion in Computing Foundations was preserved. Historical source receipts keep their original paths; `continuation-2026-11-11/WORK_STATE.json` identifies the current workspace. Source claim reading, selected raw verification and narrative integration remain distinct. The full earlier corpus and full-book gaps are not declared complete.


### Current supervised checkpoint — November 12, 2026

The authoritative backlog now records **22,831 claim-text points reviewed and 6,884 pending** across 590 retained records. Six additional checkpoint receipts were validated for exact claim text, source-unit membership and unchanged destination quotes/hashes. Reading, selected direct raw checks, staged deltas and published teaching remain separate; these counts are not a full-source integration certificate.

The latest published additions and regression checks are recorded in [[Supplementary/Sources and Code Anchors#November 12 refinement — checked operations and continuous teaching]]. Private outstanding source reviews and full-book gaps remain in the original backlog. The referenced six Pass 10–15 texts, plus Passes 16–17, remain retained in their routed continuous volumes; retention was checked again against the original copies and normalized bodies.


### Reconciled claim-reading checkpoint — November 12 continuation

The ledger records **26,131 claim-text points reviewed and 3,584 pending**. A global check found validated earlier reading receipts that had not reached the central ledger; 1,559 distinct pending IDs were reconciled without repeating their source reads. Subsequent new checkpoints were validated separately. Source selection now uses pending IDs and excludes all staged/checkpointed IDs, replacing stale positional queues. Group 1 has no pending claim-text IDs; that does not certify exhaustive raw reading or integration of every idea.

Selected teaching from shader/culling/editor sources is developed in the existing volumes; [[Supplementary/Sources and Code Anchors#November 12 continuation — clipping, conservative bounds and editor state|the continuation evidence]] records its scope. Staged spring-force discrepancies and further collision/design deltas remain awaiting supervisor review. Whole-book coverage and complete earlier-source development remain unfinished.

### Contact solver verification — November 12 additive continuation

Two distinct C++20 contact witnesses are available in `tools/labs/rigid_contact_lab.cpp` and `tools/labs/persistent_contact_lab.cpp`, connected through the README and the source-independent narrative [[Supplementary/Rigid Contact - Persistent Manifolds and Solver Reconstruction]]. The new witness illustrates frozen impact restitution, accumulated normal and tangential impulses, two-point row coupling, feature ID carried as cache metadata, single-use warm start, and invalid-input rejection. The associated GitHub Actions workflow compiles both under strict C++20 diagnostics and UndefinedBehaviorSanitizer. Workflow configuration exists in the repository, but **no successful GitHub-hosted run has been independently observed from this checkpoint**; an empty combined status is not a pass.

The solver reconstruction explicitly derives planar effective inverse mass and the Coulomb bound. A review exposed and corrected a sign error in an initial *test expectation* for the upward-normal / leftward-tangent example; the stored tangential scalar must be **+1** for a −x impulse with tangent (−1,0). The new locally constructed witness passed six deterministic suites under a strict compiler and combined address/undefined-behavior sanitizers. The published GitHub source was then re-fetched and its key sections verified present, but **an exact-byte local compilation of that connector-published file has not been established** in this checkpoint; remote workflow execution is the next exact-source proof. Neither laboratory measures real-engine convergence or proves native runtime integration.

The new feature identifier is **not** a contact matching algorithm: stale cache invalidation, 3D tangent-basis transport, fast motion/CCD, split impulses and stabilization, general scene integration, scene-set benchmark statistics, and full source-video/book reconciliation remain outstanding. The global claim-text queue and its prior reviewed/pending counts are unchanged by this targeted lab work.

### Supervised cache and linear CCD continuation — November 12, 2026

The additive physics verification now includes `tools/labs/contact_cache_lab.cpp` and `tools/labs/disk_ccd_lab.cpp`. The cache lab explicitly validates canonical ordered body/feature keys, previous-frame reuse, 15-degree normal-drift rejection, tangent-frame projection, bounded 0.5–2 timestep scaling, material-edit tangent invalidation, duplicate-feature rejection, stale pruning, and invalid normal rejection. Stable geometric feature IDs are assumed to be provided externally: the lab does not generate or match them. The swept-disk lab derives analytic time of impact for disks with constant relative velocity, uses a stable-root quadratic formula, and checks tunneling, frame-limited queries, misses, grazing, initial overlaps, coincident centres, Galilean invariance, large magnitudes, and invalid inputs. Its response is geometric only; it does not implement event substepping or contact impulses.

**Local verification:** strict C++20 `-O2 -Wall -Wextra -Wpedantic -Werror` and combined AddressSanitizer/UndefinedBehaviorSanitizer C++20 builds executed both new labs successfully. The published GitHub files were fetched and compared with the tested local sources using exact length and FNV-1a32 checks (the two pairs matched); file hashes are useful for byte agreement, not correctness proofs. The CCD test comment was corrected after supervision: the large-magnitude trajectory does **not** represent a small absolute time of impact. Both sources are registered in the repository's contact CI workflow, but no successful remotely hosted run has yet been observed in this checkpoint.

The continuous contact reconstruction chapter now explains geometric feature ownership, impulse projection into a rotated tangent, timestep policy, overlap-versus-velocity response, analytic disk CCD and finite-precision limitations in context. These added lab paths do not increment the number of reviewed source-corpus claims and do not discharge the open work on broader video ingestion, unreviewed mathematics books, full production-engine contact matching, rotating convex-shape CCD, or engine-level performance and stability.

**Independent source identity and second compiler check:** the GitHub Git-blob SHA-1 for `contact_cache_lab.cpp` is `3cad6d563f3f00ef77a431dcfb48c52f0cfb926c`, and for `disk_ccd_lab.cpp` (after correcting its comment) is `6b89db87f64897030710e646c4cc13114ad29cfa`. Each matches `git hash-object` applied directly to the locally compiled source file, strengthening the earlier length/FNV comparison to Git's actual full-file identity. Both sources also compiled and executed with `clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`, independently of the default `c++` run. This is two local toolchains, **not** a cross-platform macOS/Windows or remotely hosted GitHub Actions result.

### End-to-end disk event stepping and randomized supervision — November 12, 2026

An additional stand-alone C++20 laboratory at `tools/labs/disk_event_pipeline_lab.cpp` now connects continuous disk collision discovery to a complete fixed-duration state update. It repeatedly finds the earliest encounter among all unordered disk pairs, advances all positions to the event, groups approximately simultaneous events, freezes each incoming restitution target, runs 32 accumulated projected normal-impulse sweeps, and re-queries the remaining interval. Nine fixed scenes exercise collision timing against a static body, two-body elastic energy/momentum, symmetric simultaneous contacts, separating motion, initial overlap rejection, event budget failure, chained events, ideal inelastic response, and duplicate stable-ID / zero-duration validation.

**Discovered defect and correction:** the first nine scenes passed, but a separately constructed deterministic 10,000-case unequal-mass isolated-pair experiment failed at trial 937. A valid impact had advanced two centres to a near-tangent configuration whose squared distance was a tiny amount below the squared combined radius; the original squared-distance tolerance rejected it as a deep overlap. The published correction computes the linear centre gap and uses a finite-precision length tolerance scaled to world-coordinate magnitude. After this correction, all 10,000 seeded cases passed under GCC and Clang and sanitizer tests. No claim is made that this tolerance is globally robust for arbitrary very large/very small coordinate ratios.

The successful randomized experiment has been retained as independently executable `tools/labs/disk_event_property_lab.cpp`. For each isolated impact it checks exactly one event, Newton's restitution equation, linear momentum conservation and non-increasing kinetic energy. Both newly published source files were re-fetched from GitHub and their Git blob hashes matched the locally compiled files: event pipeline `e69b18ba580329db14ef0bde18811e6737790084` and property test `caa2fffdfe32048e6580887ad868eda36278700f`. Local strict C++20 GCC/Clang builds and combined AddressSanitizer/UndefinedBehaviorSanitizer executions passed. The shared workflow is configured to run both on GitHub but its hosted execution has not yet been confirmed. The contact narrative and README now link both sources and explain the numeric trace.

**Remaining boundary:** this is not a production Kairo world step; all-pairs event search is quadratic per rescan, contact detection is constant-velocity disk-only, friction/rotation and resting contacts are excluded, and thrown exceptions can leave the scene partially advanced. Fully robust CCD predicates, broadphase acceleration, transactional state restoration, general convex shapes, long-time stacks, numerical determinism under compiler/platform changes and runtime integration remain open. No prior video/book claim-review queue counts were incremented by this engineering experiment.

### Swept broad-phase candidate oracle and conservation check — November 12, 2026

The repository now contains `tools/labs/swept_broadphase_lab.cpp`, a tested reference implementation of constant-velocity disk swept-AABB candidate filtering. It conservatively constructs outward-rounded bounds, sorts by minimum x, tracks active x intervals and tests y overlap, then produces unique sorted candidate pairs. Each exact disk collision returned by a separately implemented analytic TOI oracle must be included in those candidates. The test covers a crossing scene, a 100-disk non-interacting scene, 200 seeded randomized 35-disk scenes, duplicate IDs and negative dt. The randomized comparison examines 200 × (35 × 34 / 2) = **119,000** possible pairs for missed collisions. GCC and Clang strict C++20 builds and a local combined AddressSanitizer/UndefinedBehaviorSanitizer build passed, and the GitHub source Git-blob SHA `11f9e86dbf1ee850e64b5dafdea63427b2177187` matched the compiled local source. The CI workflow includes this new lab but no GitHub-hosted run was confirmed here.

The broadphase is *not* integrated into the disk event stepper; it therefore establishes a conservative candidate-generation proof obligation in a small witness, **not** measured engine acceleration. The simple vector active set can still take O(n²) time in dense scenes, and floating-point accuracy, rapidly changing acceleration, general shapes, real scene data and production determinism remain unverified. No earlier-source corpus claims were marked reviewed solely because of this new laboratory.

### Three-volume implementation-depth integration — November 12, 2026

The Physics/Geometry/Mathematics lessons developed in this continuation are now **directly inside the three canonical continuous notes**, not only in supplementary summaries:

- Mathematics: `Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing.md` now contains the self-contained first-use reconstruction from a signed disk gap through stable quadratic TOI, Jacobian/effective mass, unilateral projection, swept bounds, finite-precision error policy, numerical trace and executable C++20 root laboratory.
- Graphics: `Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md` now includes a contact-feature/manifold clipping derivation, runnable segment clipper, numerical CCD and conservative swept-AABB proofs and the complete independent C++20 CCD and swept-broadphase code.
- Game Engine: `Continuous Notes/Game Engine - From First Frame to Living Worlds.md` now includes scene-state ownership, complete event scheduling, frozen restitution and impulses, caches and warm-starting, failure/rollback policy, a worked numerical state trace, and full embedded C++20 event stepper, persistent solver and cache implementations. This code is independent of the actual Kairo runtime.

The three large volumes were re-read after their mutations. Each new section occurs **exactly once**; program-code fence counts remain even (Mathematics: 366; Graphics: 1430 before the final full-code insertion; Game Engine: 4646 before the final persistent-code insertion). A syntactic fence check cannot establish Markdown/MathJax rendering or correct physics. Additions were made without deleting the pre-existing teaching bodies.

The event stepper at `tools/labs/disk_event_pipeline_lab.cpp` now accepts `useSweptBroadphase` while retaining the original exhaustive mode. It builds candidate swept intervals from current body motion after each event, sorts pair IDs into the same order as the exhaustive mode and then runs the identical disk TOI and impulse functions. `tools/labs/disk_event_broadphase_equivalence_lab.cpp` compares four fixed scenes and 1,000 seeded scenes between modes for event count, event batches, simulated time, final centres and velocities. Strict and UBSan GitHub Actions steps are configured for that comparison. **No successful GitHub-hosted run or independently compiled exact-byte post-integration binary has been confirmed in this checkpoint**; code presence and test configuration are not equivalent to execution success.

The previous verified local GCC/Clang/sanitizer results apply to the earlier independent labs and pre-integration source snapshots, not automatically to the newly modified event stepper. Further acceptance gates are an exact post-integration compile/run, runtime comparison against the full original deterministic suite and 10,000-case regression, actual hosted CI status, numerical stress tests for near-contact events and dense simultaneous contacts, and only then a real Kairo integration review.

**Global coverage constraint:** these additions deepen the received collision and physics concepts and do not validate every claim in the other thousands of source pages, every downloaded lecture, or all 3,000 original parts. Previously recorded source-review gaps, mathematical-book chapters and Phase 18 parts 1701–1800 remain explicit, not declared complete. The three continuous notes are authoritative teaching routes; supplementary source ledgers and executable witnesses provide provenance and reproducibility.

### Three canonical volumes — additive first-use reconstruction audit (November 12, 2026)

The three authoritative graphics/geometry/physics continuous volumes were edited **in place** rather than replaced by new per-topic notes. The Mathematics volume now adds an explicit swept-set to TOI derivation, dimensionally checked quadratic coefficients, stable-root policy, contact-space effective mass from angular impulse, projected unilateral impulses, conservative swept-bound inclusion proof, numerical wall-impact trace, and reconstruction test at the first swept-geometry encounter. Graphics now develops the broadphase/narrowphase/event-scheduler contracts, body identity and buffer ownership, no-false-negative property tests, time-of-impact/debug trace, finite-precision tolerance failure, and CPU/GPU evidence requirements beside its existing swept-geometry section. Game Engine embeds the complete new exhaustive-vs-swept C++20 differential test directly in its existing implementable world-step chapter, alongside ownership, transactional failure, replay and Kairo-integration requirements.

A **byte-for-byte preservation check** re-fetched each published Git blob and removed only the newly inserted chapter span; the result exactly equaled the pre-edit Git blob in all three cases. Reported text additions were 9,621 characters Mathematics, 8,845 Graphics, and 8,371 Game Engine. Pre-existing Markdown code fences and display-math boundary counts were unchanged; Game Engine gained exactly four correctly formed code-fence boundaries. An initially escaped Markdown fence in the new engine text was caught and repaired before this verification. This is strong preservation/formatting evidence, **not** a semantic certification of every previously existing paragraph, every math derivation, or every code block.

The new differential lab `tools/labs/disk_event_broadphase_differential_lab.cpp` compares full world outcomes in exhaustive and swept-pair modes: event/batch counts, elapsed step time, stable identity, final positions and velocities. It includes four explicit scenes and 2,000 reproducibly seeded isolated impacts. The strict and UBSan contact workflow now references the lab. The test source and test registration were verified in GitHub; their execution status must be tied to a named hosted CI job or an exact-source local compile, never inferred from presence.

**General C++ CI diagnosis.** The original `.github/workflows/allmight-cpp20-labs.yml` was returning immediately failed, zero-job Actions runs. Comparing it with the earlier successfully scheduled workflow identified only two added path-trigger lines referring to the legacy Computing Foundations note. The two-line edit was reverted byte-identically to the previously job-schedulable version, without removing any lab tests. GitHub then scheduled the matrix normally under workflow run [43127394396](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43127394396). Final job conclusions should be read independently from that run before claiming all compiler configurations passed.

**Scope and completeness boundary.** The canonical Mathematics, Graphics and Game Engine notes are the implementation-first reader route; their supplements and standalone code are independent evidence artifacts, not substitutes for essential first-use teaching. The former Computing Foundations volume remains preserved as raw provenance; its complete source body has since been mechanically embedded by subject into Game Engine and Graphics. Semantic integration and first-use ordering are still being audited. The 3,000-part original curriculum, unread textbook chapters, unfinished caption/source review, and unverified real Kairo/GPU runtime paths remain open. A concept is not complete merely because its name or part number appears; it needs its derivation, conventions, state, executable algorithm, trace, failure cases and reproduced tests inside the relevant continuous narrative.

### Seven-program canonical source-sync and hosted compilation gate

The canonical notes now embed seven complete C++20 witness programs whose source bodies are checked for exact equality in `tools/check_continuous_code_sync.py`: Mathematics' `math_contact_toi_lab.cpp`; Graphics' `disk_ccd_lab.cpp` and `swept_broadphase_lab.cpp`; and Game Engine's `disk_event_pipeline_lab.cpp`, `persistent_contact_lab.cpp`, `contact_cache_lab.cpp` and `disk_event_broadphase_differential_lab.cpp`. The independent Math first-contact program was expanded from one assertion to entry/no-hit/horizon/grazing/Galilean/overlap/validation cases. The embedded code and executable file now agree byte-for-byte; the surrounding Mathematics narrative was corrected to describe the implemented tests rather than promising them as absent exercises.

A source-to-chapter audit first found a **one-character stale structured-binding form** in the embedded Game Engine event-stepper (`const auto [i,j]` versus compiled `const auto& [i,j]`). The code block was synchronized to the compiled file and all six prior source/body pairs subsequently matched exactly. The new Mathematics program increases that invariant to seven. The dedicated [GitHub Actions source-sync run 43128537743](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43128537743) completed successfully. The [GCC/Clang/module matrix run 43128401612](https://github.com/swayam8624/Project_008_AllMight/actions/runs/43128401612) completed successfully after the Mathematics source banner newline was corrected: five jobs passed (GCC sanitizer, GCC release, Clang strict, modules and M018). Earlier failed commits remain historical evidence rather than being misreported as current successes.

A curriculum-wide responsibility table in README maps all 30 original phases into the three target continuous notes while explicitly retaining Computing Foundations for verification and later non-destructive consolidation. The fact that a topic has a destination, or that seven laboratories match source, does **not** prove the full 3,000-part curriculum is semantically complete. Claim reviews and unread book/video material remain open and must be resolved through source-specific teaching and an implementation-depth audit.


### Current reading and merge state after the November 12 pull

Canonical teaching now follows the three-volume README on 635dec4. The central ledger records **26,821 claim-text points reviewed and 2,894 pending**, across 590 retained records. These counts certify reading receipts at their stated scope, not exhaustive raw-video review or fully integrated teaching for every point. One correction-unit mismatch in a new receipt was held out, repaired against the actual point-owned unit and revalidated before its reading IDs advanced.

[[Supplementary/Sources and Code Anchors#November 12 pulled consolidation — supervised spring, contact and design additions|The supervised merge evidence]] records the new spring/contact/design teaching, complete inline models and actual local checks. Further staged friction, projectile-proxy and ABI examples remain queued for review; full supplied-book coverage and earlier-source development remain unfinished.

The final bounded continuation also reconciled the staged friction and projectile examples with the current canonical volume. Reading totals above are unchanged by this teaching merge. The new passages and recall links remain additive; the DLL demonstration was classified as corroborating existing coverage rather than a separate new lesson. Final mechanical checks cover the nine declared inline programs and retained source bodies; they do not certify all embedded fragments or full-book conceptual completeness.


### Post-merge semantic audit and logical render-graph reconstruction — November 12, 2026

The next corpus checkpoint is [[Supplementary/Corpus Development Audit - 2026-11-12]]. It examines the first consolidation against the **later** main revision `e25a8095815944a19e3ea3a3f69c40b7c90dd6fb` and distinguishes the physical seven Markdown files in Continuous Notes from the **three canonical destinations**. Roughly 23 archived-note cross-links still exist in reader-facing books, most of them valid but undesirable detours. Automated code and source-preservation gates cannot establish implementation depth; the first large imported chunks also need narrative-flow review. This is an explicitly open audit debt, not a silent completion claim.

The Game Engine's new [[Continuous Notes/Game Engine - From First Frame to Living Worlds#From a list of render passes to a proved execution order|render-graph compilation]] derives RAW/WAR/WAW ordering, last-writer/read-cohort state, imported resource initialization and stable topological scheduling, with a six-pass trace, executable test source and rejection cases. The source is `tools/labs/render_graph_contract_lab.cpp`, and its exact inline agreement is part of `tools/check_continuous_code_sync.py`. The source program passed local GCC/Clang strict debug/release builds; hosted CI results must be checked for the subsequent branch commit before representing them as successful. This is a CPU-only logical DAG oracle, **not** a Vulkan/Metal barrier implementation.

The reviewed/pending point count remains **26,821/2,894** from the latest receipt checkpoint. No additional pending claim IDs were counted as reviewed during this engineering pass; earlier versions of the ledger retain their historical counts as dated snapshots.
