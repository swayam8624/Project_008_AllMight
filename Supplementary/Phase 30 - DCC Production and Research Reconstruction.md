# Phase 30 — From DCC State to Reproducible Engine Evidence

Phase 30 is not merely “production tooling.” It is the point where every earlier technical claim becomes an auditable artifact. A mesh edited in Blender, a rig authored in Maya, a procedural asset generated in Houdini, a frame sequence processed in Nuke, a benchmark run, and a research plot all share one systems problem: **how do we move state across tools without losing identity, meaning, reproducibility or evidence?**

The companion executable witness is `tools/labs/phase30_pipeline_contract_lab.cpp`. It provides a host-neutral publication oracle covering path normalization, traversal rejection, deterministic manifests, frame-sequence validation, stable diagnostics, DCC identity adaptation, staging and idempotent publication. It does not pretend to execute Blender, Maya, Houdini or Nuke in CI. Those host APIs must be verified with host-specific integration tests.

## 1. Separate authored state, exported state and published state

A DCC scene is not an engine asset.

An authored object may contain editor-only state: modifiers, construction history, references, constraints, hidden layers, procedural nodes, unit settings, animation curves, render-only attributes, host-specific metadata and temporary caches. The engine usually wants a smaller runtime representation.

Therefore model the pipeline as three distinct domains.

**Authoring domain.** The host application owns rich editable state.

**Exchange domain.** An exporter converts selected authoring state into an explicit, validated representation such as glTF, USD, Alembic, VDB, EXR sequences, custom binary data or a project-specific manifest.

**Runtime publication domain.** The engine ingests validated exchange data, resolves identifiers and dependencies, builds optimized runtime artifacts, and publishes them atomically.

Do not make the engine loader depend on undocumented host state. Do not make the host exporter directly mutate live runtime state without an intermediate contract.

## 2. Identity is more important than filenames

A path is a location. It is not necessarily object identity.

Suppose `crate.glb` is moved to a different folder. If every reference uses the path as identity, moving the file can break scenes, caches and saved references. A more durable publication system assigns a logical asset ID and treats the path as one property of a published version.

A useful record contains:

- logical asset ID;
- schema version;
- normalized relative publication path;
- content hash;
- source/tool provenance;
- dependency IDs;
- unit/axis conventions;
- optional build parameters;
- generated artifact version.

The Phase 30 lab uses a smaller subset so the state transition remains inspectable.

## 3. Normalize and validate paths before touching the filesystem

A pipeline that accepts paths from project files, scripts or DCC metadata must treat them as untrusted input.

The sequence should be:

raw path → reject absolute/drive-qualified paths if the contract requires project-relative paths → normalize separators → resolve `.` → resolve `..` while preventing escape above the publication root → reject invalid components → optionally resolve symlinks against an allowed root → only then construct a destination path.

String normalization alone cannot prove symlink safety. If a process writes to the real filesystem, resolve or open paths with platform-safe APIs and verify that the final target remains inside the allowed root.

The reference lab proves the lexical half of this contract and explicitly does not claim symlink containment.

## 4. A manifest is an executable publication contract

The manifest should be deterministic. Given the same logical inputs and build settings, its canonical representation should be identical.

Canonicalization usually requires:

- stable ordering;
- normalized paths;
- explicit schema versions;
- no timestamps inside identity hashes unless time is semantically part of the asset;
- normalized numeric/string formatting;
- explicit dependency ordering policy;
- deterministic diagnostic codes.

A deterministic manifest lets the build system answer an important question without rebuilding everything:

> Has the semantic input to this published artifact changed?

The reference lab sorts records by logical identity before hashing a plan so insertion order cannot silently change publication identity.

## 5. Hashes answer identity questions only when the hashed domain is defined

A content hash is meaningless unless you know exactly which bytes were hashed.

Potential domains include:

- raw source file bytes;
- normalized source representation;
- exported exchange artifact;
- canonical manifest;
- runtime-optimized artifact;
- all transitive dependencies plus build settings.

These answer different questions.

For a production system, SHA-256 or another approved cryptographic hash is appropriate for durable content identity. The reference lab deliberately uses FNV-1a only as a deterministic test hash because it is small enough to inspect and because the lab is proving publication logic rather than cryptographic collision resistance.

Do not copy that hash choice into a production content-addressed store.

## 6. Staging makes publication transactional

Never publish a half-written asset set.

The correct pattern is:

1. Validate all inputs.
2. Build a publication plan.
3. Write generated artifacts into a staging location.
4. Verify every staged artifact.
5. Flush/synchronize where durability matters.
6. Atomically switch the visible manifest/root/reference to the staged version.
7. Retire old data only after readers can no longer observe it.
8. On failure, keep the old published state intact.

The Phase 30 lab simulates this by constructing a complete staged map and swapping it into visibility only after every record passes.

Real filesystems require stronger handling: temporary directories, atomic rename semantics, cross-volume restrictions, crash recovery and platform-specific durability.

## 7. Idempotence is not “running twice happens to work”

A pipeline is idempotent when applying the same semantic publication request repeatedly produces the same externally visible result.

If a publish command receives the same validated inputs twice, the second invocation should ideally report that the requested version is already current rather than creating an unnecessary new version.

This is why a plan identity/hash belongs before the commit boundary.

Idempotence makes retries safe. That matters when editor automation, CI workers, render farms or distributed build systems can repeat work after timeouts.

## 8. Deterministic diagnostics are part of the API

A message such as “export failed” is not enough for automation.

A production diagnostic should contain a stable code plus structured context:

`ASSET004 path escapes publication root: ../../foo`

The exact human wording may evolve, but the code allows tests and tools to react reliably.

Prefer diagnostics that answer:

- what invariant failed;
- which asset/node/file caused it;
- whether the error is fatal;
- where the invalid value came from;
- what representation or unit was expected.

Sort diagnostics deterministically when their discovery order is not semantically relevant. Otherwise two identical runs can produce noisy diffs.

## 9. Blender adapter: extract evaluated state deliberately

Blender exposes objects, collections, modifiers, dependency-graph evaluation, meshes, materials, armatures, animation data and custom properties through `bpy`.

The critical distinction is between **authored object state** and **evaluated object state**. An object with modifiers may not match its original mesh datablock. An exporter must decide whether to publish the original control representation or the dependency-graph-evaluated result.

A robust Blender adapter should record at least:

- scene/object identity;
- evaluated transform;
- unit scale and axis convention;
- evaluated mesh topology if modifiers are baked;
- material references;
- armature/bone mapping where relevant;
- animation range/sample policy;
- custom metadata used by the engine;
- source Blender/version information if compatibility matters.

Do not let the host-neutral core import `bpy`. Keep `bpy` in the Blender adapter and emit neutral records.

## 10. Maya adapter: DAG path and dependency identity are not the same thing

Maya has a directed acyclic scene hierarchy for transforms/shapes and a broader dependency graph for computation and connections.

A production adapter must distinguish a node's current DAG path from persistent identity. Instancing can give one shape multiple DAG paths. References introduce external ownership. Attributes may be connected rather than authored as literals.

The exporter therefore needs explicit policies for:

- referenced versus local nodes;
- instance expansion or preservation;
- namespace handling;
- coordinate/unit conversion;
- animation evaluation;
- attribute connection baking;
- skin/joint identity;
- hidden/templated/intermediate objects.

Again, host-specific traversal belongs in the Maya adapter. The published result should be host-neutral.

## 11. Houdini adapter: the node graph describes a recipe, not necessarily the final geometry

Houdini's strength is procedural generation. The important production question is therefore whether the engine needs the procedural recipe, a cooked result, or both.

A typical runtime publication path cooks selected SOP/LOP outputs and exports deterministic geometry, volume or scene data. A tool should record parameters and upstream dependency versions required to reproduce the cook.

For simulation or VFX caches, publication also needs frame/time-sample contracts:

- start/end frame;
- frame rate;
- substep/sample convention;
- missing-frame policy;
- topology-varying versus topology-fixed assumptions;
- compression format;
- transform/world-space convention.

The Phase 30 lab includes frame-sequence validation because one missing frame in a cache can invalidate an otherwise successful build.

## 12. Nuke adapter: image sequences require explicit range and color meaning

A Nuke graph often turns source sequences into processed image sequences. The engine or research pipeline cannot treat “an EXR exists” as sufficient evidence.

A publication record should state:

- frame range;
- missing-frame policy;
- view/stereo identity;
- channel/layer set;
- data window/display window policy;
- color space / transfer interpretation;
- premultiplication convention;
- pixel type;
- metadata that must survive;
- output path pattern.

For comparison images used in a paper, preserve the exact processing graph or a reproducible equivalent. Otherwise the figure becomes presentation rather than evidence.

## 13. Host-neutral adapters keep the pipeline testable

The clean architecture is:

`BlenderAdapter | MayaAdapter | HoudiniAdapter | NukeAdapter`
→ neutral `AssetRecord/SequenceRecord`
→ validation
→ build plan
→ conversion/optimization
→ staging
→ verification
→ atomic publication.

This allows most of the pipeline to run in ordinary CI without installing every DCC package. Host integrations then receive smaller contract tests inside the actual application environment.

The Phase 30 C++ lab models exactly that boundary through `DccExport -> AssetRecord`.

## 14. Schema versioning must describe compatibility, not chronology

Incrementing a schema version simply because a file changed creates meaningless churn.

A schema version should change when the interpretation contract changes incompatibly or when readers need explicit branching. Compatible additions can often be handled through optional fields and capability checks.

A reader should reject a version it cannot safely interpret rather than guessing.

For long-lived assets, document:

- current schema;
- oldest supported schema;
- migration path;
- irreversible migrations;
- whether migration is deterministic;
- whether old artifacts remain readable.

## 15. Reproducible benchmarking needs experimental state, not one FPS number

A benchmark record should identify enough state to explain the result:

- commit/build identifier;
- compiler and optimization flags;
- CPU/GPU/device;
- operating system/driver;
- scene/model/dataset identity and hash;
- resolution/workload settings;
- warmup policy;
- number of measured samples;
- wall/monotonic/GPU clock used;
- summary statistics;
- raw results or artifact location.

Do not report only a mean when the distribution is unstable. Median and percentiles can expose stalls. Small samples need uncertainty qualification.

GPU timings should come from appropriate GPU timestamp mechanisms when measuring GPU work; CPU wall time includes queueing and synchronization effects.

## 16. Golden tests and exact tests solve different problems

An exact output test is appropriate when deterministic byte identity is part of the contract.

A golden-image test is useful when the expected output is intentionally stored as a reference artifact. But the comparison metric must match the claim.

Examples:

- exact pixel equality for deterministic integer IDs;
- bounded absolute/relative error for floating-point buffers;
- PSNR or SSIM for some image-quality regressions;
- task-specific metrics for reconstruction;
- structural assertions for manifests and scene graphs.

A high SSIM does not prove that object IDs, depth, normals or temporal stability are correct.

## 17. Property tests and fuzzing attack the contract boundary

Example-based tests ask whether selected known cases work.

Property tests ask whether an invariant holds across many generated cases.

For a normalized path function:

- normalization should be idempotent;
- normalized valid paths should remain relative;
- an accepted path should never contain a parent escape;
- separator variants should normalize consistently.

Fuzzing then explores malformed byte strings, extreme lengths and surprising combinations that humans did not write by hand.

This is especially important for model files, project manifests and asset importers because they parse externally stored data.

## 18. Performance regression tests need noise policy

A benchmark that fails CI whenever one run is 2% slower will become flaky.

Instead define:

- warmup count;
- sample count;
- machine class;
- acceptable noise band;
- comparison statistic;
- minimum meaningful regression;
- whether retry is permitted;
- when a regression is informational versus blocking.

For shared cloud runners, use broad smoke thresholds or dedicated benchmarking infrastructure rather than pretending noisy hardware is a stable laboratory.

## 19. Research questions and engineering objectives are different

An engineering objective can be:

> reduce Gaussian tile-sort time.

A research question is stronger:

> under what projected-footprint and opacity distributions does algorithm A reduce sort cost without changing image error beyond threshold E compared with baseline B?

The research version requires independent/dependent variables, baselines, workload selection, metrics, confounds and a falsifiable result.

A successful engineering implementation does not automatically establish a general research claim.

## 20. Preserve provenance from raw input to paper figure

For every reported result, be able to trace:

raw dataset/model/scene
→ preprocessing
→ executable commit
→ configuration
→ run ID
→ raw measurements/output
→ analysis script
→ table/figure.

If a result is manually copied into a spreadsheet and then manually copied again into a paper, provenance is fragile.

The better design lets the final plot refer back to machine-readable evidence.

## 21. Ablations isolate mechanisms

If a system contains three proposed changes and only the complete system is benchmarked, the experiment cannot show which change mattered.

Ablations selectively remove or replace mechanisms while preserving the rest of the experimental setup.

For a rendering system, useful ablations might disable:

- temporal history;
- uncertainty weighting;
- a new partitioning heuristic;
- local repair;
- a specific acceleration structure.

The ablation design should follow the causal claim, not simply produce many tables.

## 22. Baselines must be competitive and faithfully configured

A weak baseline can make a new method appear strong.

Record:

- implementation source/commit;
- configuration;
- parameter tuning policy;
- hardware;
- input preprocessing;
- stopping criteria;
- whether the implementation is the authors' version or your reimplementation.

If you reimplement a baseline, validate it against published or reference behavior before using it as evidence.

## 23. Paper reimplementation is an interpretation exercise

A paper rarely specifies every engineering detail.

Separate:

**stated algorithm** — what the paper explicitly defines;

**necessary implementation choice** — what must be decided to run it;

**your modification** — what you intentionally changed.

This prevents accidental novelty claims and helps diagnose disagreement with reported results.

## 24. The oral exam should reconstruct, not recite

The final mastery test should present an unfamiliar system and ask the learner to rebuild the reasoning.

A good oral/capstone prompt might be:

> Design a streamed renderer for a large captured world on a memory-constrained device. Assets arrive asynchronously, GPU resources retire later than CPU objects, camera motion invalidates some temporal history, and the renderer must support both Vulkan and Metal.

The learner should derive:

- ownership graph;
- state representation;
- loading/publication protocol;
- render-graph dependencies;
- synchronization;
- error/fallback behavior;
- memory budgets;
- profiling plan;
- test plan;
- alternative designs and trade-offs.

If the learner only repeats a chapter's exact architecture, the exam has not tested transfer.

## 25. What remains before Phase 30 can be called production-complete

The new lab closes the host-neutral contract gap but deliberately leaves host/native work visible:

1. Blender in-host integration test.
2. Maya in-host integration test.
3. Houdini cook/export fixture.
4. Nuke sequence/preflight fixture.
5. Real filesystem atomic-publication test including failure injection.
6. Symlink escape tests on supported platforms.
7. SHA-256 content identity in the production path.
8. Cross-platform compiler CI for the publication core.
9. Property/fuzz test harness over parsers and manifests.
10. Performance regression dashboard with stable-machine policy.
11. Evidence bundle generator linking run IDs to figures/tables.
12. A rotating unfamiliar-system oral-exam bank.

Until those exist, label the phase **reference-implemented**, not production-verified.

## Reconstruction questions

Explain why a DCC scene is not an engine asset. Distinguish identity from path. Describe a traversal attack and why lexical normalization alone does not solve symlinks. Define idempotent publication. Explain why deterministic diagnostic codes matter. Describe the Blender authored/evaluated distinction, Maya DAG versus dependency concerns, Houdini recipe versus cook, and Nuke frame/color contracts. Then trace one asset from a host object to an atomically published runtime artifact and finally to a benchmark figure with full provenance.
