# 11 — Streaming resources through explicit states

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/10 - Engine changes across subsystem contracts|Previous chapter]] · [[Chapter Edition/03 - Game Engine/12 - Transactions, recovery, and safe project boundaries|Next chapter]]

Large worlds cannot load every resource before play begins, so readiness becomes a state transition with owners and versions. This chapter develops asynchronous work, publication, residency, and safe replacement as one resource lifecycle.

**By the end:** Model streaming as request, decode, validation, upload, publication, use, and retirement, including stale completion and memory-budget cases.

**Recall question:** When an older load finishes after a newer request, what prevents the stale result from becoming visible or freeing a resource still in use?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 15915–20055; source-span SHA256 81533611d264a650abc63f2d382154b8cf1a1e248c708174371955334754e42b. -->

<!-- CHAPTER-SOURCE-BEGIN -->
**A streaming state machine**

A useful state model:

```text
Unloaded
   |
   v
Requested
   |
   v
LoadingCPU
   |
   v
CPUReady
   |
   v
UploadQueued
   |
   v
Resident
   |
   v
EvictPending
   |
   v
Unloaded
```

With separate:

```text
Failed
```

state.

Do not represent all of this as:

```cpp
bool loaded;
```

There are many intermediate states with different ownership.

---

**Asset identity during streaming**

Asset handle remains:

```text
valid
```

while asset is Unloaded.

Queries may return:

```text
placeholder
fallback
pending future
not resident status.
```

Thus again:

$$
AssetExistence
\neq
CPUResidency
\neq
GPUResidency.
$$

---

**Async request object**

Conceptually:

```cpp
struct AssetRequest
{
    AssetID Asset;
    ResidencyPriority Priority;
};
```

Asset runtime deduplicates multiple requests for same asset.

If ten entities request the same texture:

```text
one load
ten consumers.
```

Do not launch ten disk reads.

---

**Streaming priorities**

Useful priority inputs include:

$$
\text{camera distance}
$$

$$
\text{screen-space size}
$$

$$
\text{gameplay criticality}
$$

$$
\text{predicted future visibility}
$$

$$
\text{explicit preload priority}.
$$

A hero character texture should beat a distant optional prop when budgets are constrained.

---

**Partial residency**

Residency need not be all-or-nothing.

Textures:

```text
coarse mip resident
high mips streaming.
```

Meshes:

```text
low LOD resident
high LOD requested.
```

Worlds:

```text
near cells resident
far cells absent.
```

This makes residency a level rather than a bool.

---

**Residency state**

A richer resource entry:

```cpp
enum class ResidencyState
{
    Unloaded,
    Loading,
    CPUResident,
    Uploading,
    GPUResident,
    EvictPending,
    Failed
};

struct AssetRuntimeEntry
{
    AssetID Asset;
    ResidencyState State;

    std::size_t CPUBytes;
    std::size_t GPUBytes;

    std::uint64_t LastUsedFrame;
    std::uint32_t PinCount;
};
```

Now resource manager can make budget decisions.

---

**CPU and GPU residency are independent**

A texture may be:

```text
CPU artifact present
GPU resource absent
```

during upload or after GPU eviction.

It may also be:

```text
GPU resident
CPU source artifact discarded
```

if the runtime no longer needs CPU pixels.

Therefore track separately where useful:

$$
R_{CPU},
R_{GPU}.
$$

---

**Memory budgeting**

Suppose GPU texture budget:

$$
B=4\text{ GiB}.
$$

Current resident textures consume:

$$
3.9\text{ GiB}.
$$

New request requires:

$$
500\text{ MiB}.
$$

Resource manager must evict approximately:

$$
400\text{ MiB}
$$

plus safety margin.

Candidates might be ranked by:

```text
unpinned
oldest use
lowest priority
largest reclaim value.
```

---

**Residency is policy**

Low-level GPU API should not decide:

> unload distant mountain.

It only knows allocation.

High-level streaming system has scene/game knowledge.

Thus:

```text
Streaming policy
      ↓
Residency manager
      ↓
Renderer/GPU allocator
```

preserves proper dependency direction.

---

**Reference counting versus explicit residency**

Reference counting says:

$$
resource\ stays\ alive
\iff
refcount>0.
$$

This works well for ordinary CPU object ownership.

It is often insufficient for streaming.

---

**Why references do not equal residency need**

Suppose a Scene contains references to 50,000 textures.

If each reference increments a strong runtime refcount:

$$
all\ 50,000
$$

may remain resident forever.

But many are currently:

```text
off-screen
far away
inactive level
low-priority.
```

The fact that an asset is **logically referenced** does not mean its heavy runtime payload must remain resident.

---

**Three concepts should be separated**

A good model distinguishes:

$$
\boxed{
Reference
}
$$

meaning:

> some durable object names this asset.

$$
\boxed{
Request
}
$$

meaning:

> some runtime consumer currently wants it available.

$$
\boxed{
Pin
}
$$

meaning:

> do not evict it yet.

These are different.

---

**Explicit residency leases**

Example:

```cpp
class ResidencyLease
{
public:
    ~ResidencyLease()
    {
        manager.Release(asset);
    }
};
```

A system can acquire:

```cpp
auto lease =
    assets.Acquire(texture);
```

while it requires residency.

But simple counts still need budget/priority semantics.

So:

```text
pin count
priority
last use
budget
```

often work together.

---

**Weak persistent handles**

An authored:

```cpp
TextureAssetHandle
```

is effectively a weak persistent reference to identity.

It should not force runtime GPU residency merely because a material exists somewhere in a project file.

This is another benefit of Kairo's AssetHandle containing only `AssetID`, not a shared_ptr to runtime texture.

---

**Resource lifetime across frames in flight**

This closes M092.

A resource may have several overlapping lifetimes:

$$
L_{\text{AssetID}}
$$

persistent project lifetime,

$$
L_{\text{DerivedArtifact}}
$$

cache/runtime CPU lifetime,

$$
L_{\text{RuntimeHandle}}
$$

process resource lifetime,

$$
L_{\text{GPUUse}}
$$

asynchronous submission lifetime.

These are not equal.

---

**The key inequality**

For resource R submitted to GPU at work item $S$:

$$
\boxed{
L_R
>
Completion(S).
}
$$

CPU saying:

```text
"I don't need it anymore"
```

is not enough if GPU still does.

This is conceptually identical to deferred reclamation in concurrent data structures.

A resource is retired logically first, physically freed later.

---

**Example three-frame timeline**

Imagine CPU:

```text
CPU frame 10 submitted using Texture A
CPU frame 11 reloads and publishes Texture B
CPU frame 12 uses B
```

GPU:

```text
currently executing frame 9.
```

Texture A must remain alive for frame 10 even though no future CPU frame will reference it.

So:

```text
Asset mapping:
A -> B immediately for future work

GPU memory:
old A remains until frame 10 fence signals.
```

Logical replacement and physical destruction are separate operations.

---

**Hot-reload publication model**

A robust runtime entry can be:

```cpp
struct TextureEntry
{
    AssetID Asset;

    std::uint64_t Revision;

    TextureRuntimeHandle Current;
};

void PublishReplacement(
    TextureEntry& entry,
    TextureRuntimeHandle replacement)
{
    TextureRuntimeHandle old =
        entry.Current;

    entry.Current = replacement;
    ++entry.Revision;

    RetireAfterGPUCompletion(old);
}
```

That is the core operation.

---

**Material/mesh transitive hot reload**

Suppose:

```text
Material
  ↓
Texture
```

Texture reload may not require rebuilding Material if material only stores stable AssetID/descriptor binding.

It may only require rebinding the new runtime texture.

But if a shader changes resource layout:

```text
descriptor layout changed
```

dependent pipelines/material bindings may need rebuilding.

The dependency graph must distinguish such relationships.

Hot reload is not always “replace one object.”

---

**Failure during hot reload**

If replacement shader fails to compile:

```text
keep old pipeline
report error
```

Do not replace working runtime state with null merely because source file is temporarily invalid while the user types.

Editor hot reload especially needs **last-known-good semantics**.

This drastically improves authoring UX.

---

**Current Kairo M092 architecture map**

Your code currently has a particularly strong split:

```text
AssetID
    persistent UUID

AssetMetadata
    path/type/origin/importer/revision/deps

AssetRegistry
    ID/path topology authority

ImportDatabase
    source fingerprint +
    importer/version/settings provenance

AssetImporter
    source -> derived artifact

ImporterRegistry
    exact importer ID/version registry

DerivedDataCache
    immutable recipe-addressed artifacts

SourceWatcher
    portable source-change observation

KairoRenderer
    artifact -> runtime GPU representation
```

That is a coherent pipeline.

It is not one monolithic asset manager.

---

**The current asset pipeline end-to-end**

For a texture:

```text
Textures/Wall.png
        |
        | AssetID identifies it persistently
        v
AssetRegistry metadata
        |
        v
ImportRecord
        |
        | source fingerprint
        | importer = kairo.texture.stb
        | importer version
        | canonical settings
        v
DerivedDataKey
        |
      cache?
      /    \
    yes     no
     |       |
     |   StbTextureImporter
     |       |
     |    mip chain
     |       |
     +--- TextureArtifact
              |
              v
       Renderer adapter
              |
              v
       VulkanTexture2D
              |
              v
          VkImage
```

This is the mental model to keep.

---

**Three kinds of hashes in an asset system**

Do not conflate:

**Persistent asset identity**

Kairo:

$$
UUID.
$$

**Source content fingerprint**

Answers:

> are source bytes the same?

**Derived cache key**

Answers:

> is the entire derivation recipe identical?

These serve different roles.

A robust asset system may use all three simultaneously.

Kairo does.

---

**Three kinds of versions**

Similarly distinguish:

$$
MetadataRevision
$$

for registry metadata changes,

$$
ImporterVersion
$$

for derivation implementation,

$$
RuntimeRevision
$$

for currently published runtime resource generation/hot reload.

One vague `version` field cannot express all three cleanly.

---

**Asset deletion**

Current AssetRegistry refuses to remove an asset while another registered asset depends on it.

That is a very good invariant.

Without this:

```text
Material -> Texture
```

could become silently dangling.

A higher-level editor may support:

```text
Delete anyway and clear references
```

or:

```text
Delete with dependency replacement.
```

But those should be explicit transactions.

Registry-level safety should remain strict.

---

**Asset move**

Moving asset path should not affect AssetID.

Kairo's registry updates:

```text
ID -> new path
path index
metadata revision
```

atomically enough to avoid leaving an asset with no path mapping if allocation fails.

This demonstrates an important principle:

$$
\boxed{
\text{metadata operations deserve strong exception guarantees too}.
}
$$

Asset databases are persistent state, not disposable scratch memory.

---

**Dependency graph complexity**

Let:

$$
V=
\text{asset count}
$$

and:

$$
E=
\text{dependency count}.
$$

Cycle detection through full reachability per mutation may cost:

$$
O(V+E)
$$

in worst case.

For editor-scale graphs this is usually acceptable.

If asset graphs become enormous and mutations frequent, maintain additional topological information or use more sophisticated incremental DAG algorithms.

Correctness first.

---

**DDC size management**

Content-addressed invalidation produces old unreachable entries.

Therefore DDC eventually needs garbage collection.

Possible policy:

```text
size budget
LRU
age threshold
workspace manifest reachability
```

Importantly, deleting DDC is safe.

All entries should be reproducible from source + tools.

That is the criterion for calling it a cache.

---

**DDC versus packaged build artifacts**

A local DDC may contain many historical/unused entries.

A shipping package should contain only the dependency closure of the chosen build/startup content.

Pipeline:

```text
project
    ↓
resolve package roots
    ↓
dependency closure
    ↓
resolve current derived artifacts
    ↓
copy/cook package manifest + payloads.
```

Do not simply ship the entire developer cache.

---

**Reproducibility**

For a derivation to be reproducible:

$$
Artifact
=
F(
Source,
Dependencies,
Settings,
ImporterVersion,
Target
).
$$

There must be no hidden dependency on:

```text
current directory
wall-clock time
random seed
machine locale
unordered-map iteration
untracked tool version.
```

Any hidden input undermines the cache.

This is why canonical settings, exact importer version and deterministic manifests matter.

---

**Locale**

Parsing:

```text
1.25
```

must mean the same thing on systems whose locale expects commas.

Asset formats should use locale-independent numeric parsing/serialization.

Kairo consistently uses explicit classic/character-based parsing in several persistence systems, which is the right philosophy.

---

**Deterministic dependency ordering**

Even if dependency vector semantics are unordered, writing them in arbitrary hash-table iteration order can produce different artifact bytes.

Canonicalize:

```text
sort by stable ID
```

before serialization/hashing unless order itself is meaningful.

Deterministic representation helps:

```text
cache hits
diffs
reproducible builds
tests.
```

---

**Asset importer error boundaries**

Importer errors should say:

```text
asset
source
importer
location if known
reason.
```

But importer failure should not partially mutate registry or runtime.

Your current functional boundary:

```text
Importer returns artifact or throws
```

is excellent for this.

Transaction owner can decide whether to:

```text
keep old artifact
mark source dirty
show error
retry.
```

---

**Build versus runtime import**

Editor:

```text
source watcher detects change
    ↓
import
    ↓
DDC
    ↓
hot reload.
```

Shipping runtime:

```text
manifest
    ↓
prebuilt artifact
    ↓
runtime resource.
```

Ideally shipped game does not need source file import paths.

This is Part 1518 and M092 working together.

---

**Cross-platform artifacts**

Some artifacts can be backend-neutral:

```text
canonical mesh vertices/indices
animation tracks
scene hierarchy.
```

Others should be target-specific:

```text
BC7/ASTC textures
shader binaries
pipeline libraries.
```

So the asset pipeline may have two derivation stages:

$$
Source
\rightarrow
CanonicalArtifact
\rightarrow
PlatformArtifact.
$$

This avoids re-parsing expensive source while still producing platform-specific output.

---

**The complete future Kairo asset runtime**

A strong next-generation runtime could be:

```text
AssetRegistry
    persistent metadata

Import pipeline / DDC
    authoring/build side

RuntimeAssetManager
    AssetID -> RuntimeEntry
                |
                +-- artifact state
                +-- residency
                +-- revision
                +-- current resource handle
                +-- load future
                +-- last GPU use

RendererResourceManager
    generational runtime handles
    deferred destruction
    memory budgeting
```

This preserves every lifetime boundary.

---

**Runtime asset entry design**

For example:

```cpp
enum class AssetLoadState
{
    Unloaded,
    LoadingCPU,
    CPUReady,
    UploadQueued,
    Resident,
    Failed
};

struct RuntimeTextureEntry
{
    AssetID PersistentID;

    std::uint64_t PublishedRevision = 0;

    AssetLoadState State =
        AssetLoadState::Unloaded;

    TextureRuntimeHandle Current{};

    std::size_t ResidentGPUBytes = 0;

    std::uint64_t LastUsedSubmission = 0;

    std::uint32_t PinCount = 0;
};
```

Notice what is **not** here:

```text
source path as identity
raw VkImage exposed to scene
shared_ptr forcing residency.
```

---

**Safe reload algorithm**

```cpp
void ReloadTexture(
    AssetID asset)
{
    // 1. Re-import or fetch new derived artifact.
    TextureArtifact artifact =
        BuildCurrentArtifact(asset);

    // 2. Build replacement completely.
    TextureRuntimeHandle replacement =
        renderer.CreateTexture(artifact);

    // 3. Publish only after success.
    auto& entry =
        runtimeAssets.Texture(asset);

    const auto old =
        entry.Current;

    entry.Current = replacement;
    ++entry.PublishedRevision;
    entry.State =
        AssetLoadState::Resident;

    // 4. Old resource is logically dead,
    //    but may remain physically alive.
    if (old.IsValid())
    {
        renderer.RetireTexture(
            old,
            renderer.LastSubmittedTimeline());
    }
}
```

This combines Parts 1540, 1541 and 1545 into one coherent runtime operation.

---

**Current Kairo gap priorities**

If I were extending the codebase specifically from this pass, the most valuable next work would be:

1. **External-source closure hashing** for glTF and shader include-style dependencies, because a DDC key must include every input that can change output.

2. A dedicated **shader import/compiler pipeline**, because `AssetType::Shader` exists but no built-in shader compiler was found.

3. A **runtime asset/resource manager** separating AssetID from loaded CPU/GPU resource state.

4. **Generational renderer resource handles** if slot reuse is introduced.

5. **GPU deferred destruction/timeline retirement**, required for safe hot reload and streaming.

6. Explicit **residency/budget management** rather than letting persistent references imply residency.

That order moves from correctness first to runtime sophistication.

---

**Architecture issue to watch in current umbrella linking**

Your top-level:

```cmake
KairoGameEngine INTERFACE
```

links a broad collection of engine targets.

That is fine as an application-facing convenience target.

But lower-level runtime libraries should still link the narrowest targets they truly need whenever practical.

Otherwise everything importing the umbrella can gradually acquire transitive access to:

```text
renderer
physics
assets
ECS
reflection
```

even when it should conceptually depend on only one or two.

An umbrella is useful at the composition boundary.

It should not become the default dependency of foundational modules.

---

**EngineCore's current direction is good**

Several of your current Core modules demonstrate the right principle.

`ProjectDescriptor` stores renderer policy without importing renderer.

`Application` owns generic layers rather than concrete rendering/physics.

Scene data stores persistent asset handles, not GPU handles.

Runtime VM IDs are not serialized.

Physics IDs are runtime-only.

Scene→ECS conversion lives in a bridge.

Those are not isolated coding details.

They are repeated applications of:

$$
\boxed{
\text{persistent data knows stable contracts;
runtime adapters know concrete implementations}.
}
$$

---

**Dependency reconstruction challenge**

Suppose you need to render physics debug shapes.

Bad:

```text
KairoPhysicsEngine
       ↓
KairoRenderer
```

if Physics is intended to remain renderer-neutral.

Design instead:

```text
KairoPhysicsEngine
   exposes DebugShape[]
          |
          v
PhysicsRenderBridge
          |
          v
KairoRenderer.
```

Now answer:

> Which module owns the bridge?

The highest-level integration module that is allowed to depend on both.

If you can derive this without memorization, you understand dependency inversion.

---

**Cycle reconstruction challenge**

Given:

```text
Scene → Renderer
Renderer → Scene
```

extract:

```text
RenderScene
MeshDraw
CameraPose
LightDraw
```

as a rendering contract.

Then:

```text
Scene
   |
   v
SceneRenderExtraction
   |
   v
RenderScene
   |
   v
Renderer.
```

Remove Renderer dependency from Scene.

That is architectural cycle removal through directional data extraction.

---

**Service reconstruction challenge**

Compare:

```cpp
void AI::Update()
{
    auto& physics =
        Services::Get<PhysicsWorld>();
}
```

against:

```cpp
AI::AI(
    PhysicsQuery& physics)
    : m_Physics(physics)
{}
```

Explain why the second provides:

```text
visible dependencies
testability
instance isolation
explicit lifetime.
```

Then explain why a narrow:

```cpp
AISystemContext
```

may be useful when several related services are needed.

But reject a god `EngineContext`.

---

**Lifecycle reconstruction challenge**

Given:

```text
Platform
Device
Renderer
RenderBridge
World
```

if:

```text
Renderer uses Device
RenderBridge uses Renderer + World
```

initialize:

```text
Platform
Device
Renderer
World
RenderBridge.
```

Shutdown dependency-safe:

```text
RenderBridge
World/runtime users
Renderer resources
Renderer
Device
Platform.
```

If jobs or GPU submissions exist, quiesce them before destroying dependencies.

---

**Asset-identity reconstruction challenge**

Given a texture that moves path and later changes pixels, identify the correct changing/stable values.

Rename:

```text
Textures/A.png
→ Environment/A.png
```

Persistent:

$$
AssetID:
\text{unchanged}.
$$

Path:

$$
\text{changed}.
$$

Metadata revision:

$$
\text{changed}.
$$

Source bytes:

$$
\text{same}.
$$

Source fingerprint:

$$
\text{same}.
$$

Derived key:

$$
\text{same}
$$

if path is intentionally not a semantic importer input.

Now edit pixels.

AssetID:

$$
\text{unchanged}.
$$

Source fingerprint:

$$
\text{changed}.
$$

Derived key:

$$
\text{changed}.
$$

That is the identity model.

---

**DDC reconstruction challenge**

You should be able to derive:

$$
K=
H(
SourceFingerprint,
AssetType,
ImporterID,
ImporterVersion,
CanonicalSettings,
DependencyFingerprints,
Target
).
$$

Some of those fields may be omitted only if they provably do not influence the artifact.

Then state the cache invariant:

$$
\boxed{
K_1=K_2
\Rightarrow
ArtifactBytes_1=ArtifactBytes_2.
}
$$

If this is false, the cache key is incomplete or the importer is nondeterministic.

---

**Hot-reload reconstruction challenge**

Given Texture A currently in use:

```text
detect source change
```

Do **not** destroy A.

Perform:

```text
build artifact B
create GPU resource B
validate B
publish B for future frames
retire A at its last GPU-use timeline
destroy A only after completion.
```

If any step before publication fails:

```text
A stays current.
```

That is safe hot reload.

---

**Streaming reconstruction challenge**

Explain why this is wrong:

```cpp
if (material.TextureHandle.IsValid())
{
    // therefore texture exists on GPU
}
```

A valid persistent handle only means the asset is referenced.

Correct flow:

```text
AssetHandle
    ↓
RuntimeAssetManager
    ↓
ResidencyState
    ↓
Resident?
   /     \
 yes      no
 |        |
use      request/fallback.
```

That distinction is central to any scalable asset system.

---

**Frames-in-flight reconstruction challenge**

CPU replaced resource at frame 20.

GPU still executing frame 18 and frame 19/20 are queued.

Question:

> Can old resource be destroyed immediately?

No.

Destroy only after the last submission that references it completes.

The relevant condition is:

$$
CompletedTimeline
\ge
LastUseTimeline(oldResource).
$$

Not:

```text
CPU has moved on.
```

That is Part 1545.

---

**Exact coverage — M091 + M092**

| Part | Topic | Status |
|---:|---|---|
| 1501 | Game-engine architecture as dependency management | ✓ |
| 1502 | Foundation versus runtime responsibilities | ✓ |
| 1503 | Dependency inversion in engine modules | ✓ |
| 1504 | Avoiding cyclic module dependencies | ✓ |
| 1505 | Static versus shared libraries | ✓ |
| 1506 | Header-only versus compiled libraries | ✓ |
| 1507 | Stable runtime interfaces | ✓ |
| 1508 | Backend-specific implementation boundaries | ✓ |
| 1509 | Platform abstraction layers | ✓ |
| 1510 | Service locators and trade-offs | ✓ |
| 1511 | Dependency injection | ✓ |
| 1512 | Global state and hidden coupling | ✓ |
| 1513 | Engine contexts/service bundles | ✓ |
| 1514 | Application lifecycle | ✓ |
| 1515 | Initialization order | ✓ |
| 1516 | Shutdown order | ✓ |
| 1517 | Runtime versus editor process concerns | ✓ |
| 1518 | Authoring versus shipped runtime | ✓ |
| 1519 | Standalone player architecture | ✓ |
| 1520 | Headless validation paths | ✓ |
| 1521 | Asset identity | ✓ |
| 1522 | Path-based asset identity | ✓ |
| 1523 | UUID-based asset identity | ✓ |
| 1524 | Content-addressed asset identity | ✓ |
| 1525 | Asset handles | ✓ |
| 1526 | Generational asset handles | ✓ |
| 1527 | Asset registry | ✓ |
| 1528 | Asset metadata | ✓ |
| 1529 | Source versus derived runtime assets | ✓ |
| 1530 | Importer interfaces | ✓ |
| 1531 | OBJ import pipeline boundaries | ✓ |
| 1532 | glTF import pipeline boundaries | ✓ |
| 1533 | Texture import pipeline boundaries | ✓ |
| 1534 | Shader import/compilation pipeline | ✓ |
| 1535 | Asset dependency graphs | ✓ |
| 1536 | Asset-cache keys | ✓ |
| 1537 | Derived-data caches | ✓ |
| 1538 | Cache invalidation | ✓ |
| 1539 | Incremental asset builds | ✓ |
| 1540 | Asset hot reloading | ✓ |
| 1541 | Safe GPU-resource replacement during hot reload | ✓ |
| 1542 | Asset streaming | ✓ |
| 1543 | Residency state | ✓ |
| 1544 | Reference counting versus explicit residency | ✓ |
| 1545 | Resource lifetime across frames in flight | ✓ |


The main architecture you should retain is this:

$$
\boxed{
\begin{array}{cccccc}
\text{Source Asset}
&\rightarrow&
\text{Persistent AssetID}
&\rightarrow&
\text{Import Recipe}
&\rightarrow
\\[4pt]
\text{Derived Artifact}
&\rightarrow&
\text{Runtime Resource}
&\rightarrow&
\text{GPU Submission}
&\rightarrow
\\[4pt]
&&
\text{Deferred Retirement}
&&&
\end{array}
}
$$

while the software architecture around it is:

$$
\boxed{
\text{Foundation}
\leftarrow
\text{Subsystems}
\leftarrow
\text{Integration Bridges}
\leftarrow
\text{Player / Editor / Game}
}
$$

with callbacks/interfaces allowing control flow to travel in the opposite direction **without reversing source dependencies**.

For your current Kairo codebase, the largest concrete findings from this pass are that the high-level layering and persistent/runtime identity separation are already strong, the DDC itself is unusually careful, but the next significant correctness/performance frontier is the runtime resource layer: **complete dependency-closure hashing for compound sources, dedicated shader derivation, generational loaded-resource handles, safe asynchronous GPU retirement, streaming/residency budgets, and last-known-good hot reload.**

**Pass 14 is M093 + M094, Parts 1546–1582.** That takes the next step into **serialization goals and formats, text versus binary, forward/backward compatibility, endianness, checksums, object/ECS/scene references, reflection metadata/type/property IDs, safe type erasure, inspector architecture, then undo/redo transactions, autosave/recovery, project manifests, path sandboxing, symlink/traversal defense, packaging, atomic publication, and build-config versus runtime-config separation.**


### Serialization, Schema Evolution, Compatibility, Endianness, Integrity, Object References, Reflection, Type Erasure, Undo/Redo, Transactions, Recovery, Autosave, Sandboxed Paths, Packaging, Relocatable Bundles, Atomic Publication, and Configuration Boundaries


Pass 13 ended with:

$$
\text{Source Asset}
\rightarrow
\text{Derived Artifact}
\rightarrow
\text{Runtime Resource}.
$$

Pass 14 asks how the *meaning* of all that data survives time.

A renderer object may live for milliseconds. A scene file may need to survive ten years, four engine versions, a CPU-architecture change, property renames, editor crashes, an interrupted save, a moved project directory, and a plugin no longer installed.

So the central question becomes:

$$
\boxed{
\text{How do we make engine state durable without making today's C++ layout tomorrow's file format?}
}
$$

Then M094 adds the editor-side problem:

$$
\boxed{
\text{How do we make destructive authoring operations reversible and crash-safe?}
}
$$

This is one of the most practically important passes because many engine projects get rendering right before they get persistence right—and bad persistence eventually destroys user work.

---

### SERIALIZATION, COMPATIBILITY, REFLECTION, INSPECTORS AND TYPE ERASURE

---

**Serialization goals**

Serialization is not merely:

> Turn object into bytes.

That definition is too weak.

For an engine, serialization commonly needs to satisfy several independent properties:

$$
\boxed{
Persistence
}
$$

The data survives process termination.

$$
\boxed{
Portability
}
$$

The data can move between machines/platforms.

$$
\boxed{
Determinism
}
$$

Equivalent logical states preferably produce equivalent serialized representations.

$$
\boxed{
Evolvability
}
$$

Future engine versions can interpret older data or migrate it.

$$
\boxed{
Integrity
}
$$

Corruption can be detected.

$$
\boxed{
Security
}
$$

Malformed input cannot request unbounded allocations, escape project roots or exploit parser assumptions.

$$
\boxed{
Performance
}
$$

Parsing and storage costs are appropriate for the use case.

Those goals conflict.

A human-editable scene file optimizes differently from a GPU texture artifact.

---

**Serialization domains**

You should not have one universal serializer for every purpose.

A game engine normally has several serialization domains.

```text
Editor scene
    human-readable
    diff-friendly
    versioned
    semantic

Derived asset
    compact
    validated
    deterministic
    fast to load

Network packet
    bandwidth-sensitive
    transient
    compatibility constrained

Save game
    durable
    version-aware
    player-facing

Recovery snapshot
    crash-resistant
    redundant
    checksummed
```

All serialize information.

They do not need the same format.

That is a recurring lesson:

$$
\boxed{
\text{serialization is a policy boundary, not a utility function.}
}
$$

---

**Serialization should capture semantics, not object representation**

Suppose:

```cpp
struct Camera
{
    ProjectionMode Mode;
    float VerticalFov;
    float NearPlane;
    float FarPlane;
};
```

Bad persistent format:

```cpp
write(
    reinterpret_cast<char*>(&camera),
    sizeof(camera));
```

That stores whatever today's compiler happened to choose for:

```text
padding
enum representation
alignment
float layout assumptions
member order
ABI
```

The file silently becomes coupled to one compiled object layout.

Instead store meaning:

```text
camera
projection perspective
vertical-fov 60
near-plane 0.1
far-plane 1000
```

or a binary equivalent with explicit field identifiers and numeric widths.

The serializer should describe:

$$
\boxed{
Camera\ semantics
}
$$

rather than:

$$
\boxed{
Camera\ memory.
}
$$

---

**Text versus binary serialization**

Text serialization represents logical values through characters.

Examples:

```text
near-plane 0.1
visible true
mesh "a912..."
```

Binary serialization stores encoded fields directly as bytes.

Neither is inherently superior.

They optimize different dimensions.

---

**Why text formats are powerful for authoring**

Text provides:

- easy inspection,
- source-control diffs,
- mergeability,
- manual repair,
- debugging,
- tooling with ordinary editors.

Suppose a light changes:

```diff
- intensity 1000
+ intensity 1500
```

That is excellent for source control.

Compare an opaque binary diff:

```text
8 bytes changed.
```

Far less useful to artists and programmers.

This is why your current Kairo project, scene and asset-manifest formats being deterministic textual formats is a strong authoring choice.

---

**Costs of text**

Text is usually:

- larger,
- slower to parse,
- requires numeric conversion,
- requires escaping,
- susceptible to representation ambiguity if not canonicalized.

For example floating value:

$$
1
$$

could be written:

```text
1
1.0
1.000000
1e0
```

All are semantically equivalent.

If serialized byte identity matters, choose one canonical representation.

---

**Binary serialization advantages**

Binary can provide:

- compact storage,
- faster decoding,
- direct fixed-width numeric representation,
- easier chunked random access,
- reduced parsing overhead.

A derived mesh artifact might have:

```text
header
vertex count
index count
vertex bytes
index bytes
```

There is no strong reason an end user needs to edit that manually.

Binary is natural.

---

**Binary does not automatically mean fast**

A bad binary format can be slower than a good textual one.

For example:

```text
millions of tiny length-prefixed heap allocations
```

will perform poorly regardless of binary encoding.

The real factors include:

$$
\text{allocation count}
$$

$$
\text{branching}
$$

$$
\text{copy count}
$$

$$
\text{data locality}
$$

$$
\text{compression}
$$

$$
\text{schema complexity}.
$$

Binary is an encoding choice, not a performance guarantee.

---

**Versioned file formats**

Every persistent format that may evolve should identify its schema version.

Kairo does this consistently.

Examples conceptually:

```text
kairo-project 2
kairo-assets 1
kairo-scene 5
kairo-recovery 1
kairo-package 1
```

This immediately answers:

> Which grammar and semantics should parse this file?

Without a version, a parser encounters an unknown structure and has no principled way to distinguish:

```text
corruption
new format
old format
wrong file type.
```

---

**Magic + version**

A robust header often carries at least:

$$
Magic
$$

and:

$$
Version.
$$

Binary example:

```cpp
struct FileHeader
{
    std::uint32_t Magic;
    std::uint32_t Version;
};
```

Where:

```cpp
constexpr uint32_t Magic =
    0x4B414952; // illustrative "KAIR"
```

Then parser first verifies:

```text
Is this even my file format?
```

before interpreting payload.

Text formats achieve the same through a canonical first line.

---

**Version numbers must correspond to semantic interpretation**

Do not increment versions for arbitrary reasons.

A file version means:

> The reader must potentially use different logic to interpret this representation.

If internal C++ code is refactored but serialized semantics remain identical, the file version need not change.

If field meaning changes:

```text
intensity used to mean arbitrary units
```

and now means:

```text
candela
```

that is a schema-semantic change.

Version/migration is required.

---

**Schema evolution**

Schema evolution is the process of changing persistent representation while preserving meaningful old data.

Suppose v1:

```text
light
color 1 1 1
intensity 5
```

v2 introduces unit:

```text
light
color 1 1 1
intensity 5
unit candela
```

Old files lack the field.

Reader can assign:

```text
unit = legacy
```

or convert according to a known v1 rule.

That is migration.

---

**Types of schema evolution**

The easiest change is adding an optional field with a default:

```text
v1:
roughness

v2:
roughness
double-sided = false
```

More complex:

```text
v1:
fov-horizontal

v2:
fov-vertical
```

requires viewport/aspect assumptions.

Harder:

```text
v1:
Euler rotation

v2:
Quaternion rotation.
```

Requires mathematical conversion.

Even harder:

```text
v1:
one Mesh component

v2:
MeshRenderer + MaterialSlot entities.
```

Now migration changes object topology.

---

**Migration should be semantic**

A good migration function is conceptually:

$$
M_{v1\rightarrow v2}:
Schema_{v1}
\rightarrow
Schema_{v2}.
$$

It should not be:

```cpp
reinterpret_cast<NewStruct*>(oldBytes);
```

You are converting meaning.

Example:

```cpp
CameraV2 Migrate(
    const CameraV1& old)
{
    CameraV2 result;

    result.VerticalFov =
        ConvertHorizontalToVertical(
            old.HorizontalFov,
            old.AssumedAspectRatio);

    result.NearPlane =
        old.NearPlane;

    return result;
}
```

---

**Forward compatibility**

Forward compatibility asks:

> Can an older reader tolerate data produced by a newer writer?

Suppose reader understands:

```text
position
rotation
```

but newer writer adds:

```text
scale.
```

If old reader can safely ignore unknown `scale`, forward compatibility exists for that change.

---

**Forward compatibility is difficult when semantics are required**

Suppose newer format adds:

```text
compression zstd
```

and old reader understands only:

```text
compression none.
```

It cannot simply ignore the field because it cannot decode the payload.

Forward compatibility is therefore not binary yes/no for a whole format.

It is feature-dependent.

---

**Backward compatibility**

Backward compatibility asks:

> Can a newer reader interpret files written by an older writer?

This is usually much more important for editor/project formats.

If Kairo 0.5 loads a Kairo 0.4 project and migrates it successfully, that is backward compatibility.

A mature engine often promises:

$$
CurrentReader
\rightarrow
OldFiles.
$$

It may not promise:

$$
OldReader
\rightarrow
CurrentFiles.
$$

---

**Compatibility matrix**

Imagine versions:

```text
Reader v1
Reader v2
Reader v3

Writer v1
Writer v2
Writer v3
```

Compatibility is a matrix, not a vague adjective.

For example:

| Reader | File v1 | File v2 | File v3 |
|---|---:|---:|---:|
| v1 | yes | maybe | no |
| v2 | yes | yes | maybe |
| v3 | yes | yes | yes |

Document this.

Testing compatibility fixtures is far more meaningful than saying:

> We support old files.

---

**Unknown-field handling**

Suppose new Camera file:

```text
camera
fov 60
near 0.1
chromatic-aberration 0.2
```

Older reader understands only first two fields.

It has choices.

**Strict rejection**

```text
unknown field -> error
```

Advantages:

- typo detection,
- no silent information loss,
- simple semantics.

Disadvantages:

- poor forward compatibility.

**Ignore unknown field**

Advantages:

- forward extensibility.

Disadvantages:

- read/save may destroy fields it did not understand.

**Preserve unknown field**

Old reader keeps opaque unknown data and writes it back unchanged.

This provides much stronger round-trip compatibility.

---

**Strictness is useful for authoring correctness**

Your current Kairo parsers are deliberately strict in several inspected formats.

For example `AssetManifest` rejects:

```text
unknown statement
unsupported version
duplicate/nested malformed records.
```

`ProjectDescriptor` similarly rejects unknown statements.

Recovery manifests also reject unknown statements.

This is a valid policy because it catches:

```text
typos
partial corruption
unsupported newer formats
ambiguous interpretation.
```

But it means these formats do not obtain generic forward compatibility through unknown-field preservation.

That is an explicit trade.

---

**Extensible binary formats commonly use field IDs**

Instead of positional layout:

```text
field 0 = x
field 1 = y
field 2 = z
```

encode:

```text
propertyID
wireType
length
payload
```

Unknown property ID can then be skipped using length.

This is why self-describing/tagged binary formats evolve more gracefully than naive fixed struct dumps.

---

**Stable endianness**

Consider integer:

$$
0x12345678.
$$

Little-endian memory:

```text
78 56 34 12
```

Big-endian:

```text
12 34 56 78.
```

If you write native integer bytes and read on opposite endian architecture, value changes.

Persistent binary formats must define byte order.

Usually:

```text
little-endian
```

or:

```text
network/big-endian.
```

Then convert explicitly.

---

**Endian-safe write**

Conceptually:

```cpp
void WriteU32LE(
    Writer& out,
    std::uint32_t value)
{
    std::byte bytes[4]
    {
        std::byte(value & 0xff),
        std::byte((value >> 8) & 0xff),
        std::byte((value >> 16) & 0xff),
        std::byte((value >> 24) & 0xff)
    };

    out.Write(bytes);
}
```

This representation is independent of host endianness.

---

**Stable numeric widths**

Avoid persistent:

```cpp
size_t
long
unsigned long
```

because their widths can vary by platform/compiler model.

Prefer:

```cpp
std::uint8_t
std::uint16_t
std::uint32_t
std::uint64_t

std::int32_t
std::int64_t
```

when binary representation requires a fixed width.

---

**Boolean representation**

Do not serialize `sizeof(bool)` bytes blindly.

Define:

```text
0 = false
1 = true.
```

Reject other values unless schema intentionally allows them.

Same for enums.

Do not serialize underlying enum bytes unless the persistent schema explicitly defines the numeric values.

Kairo frequently persists semantic enum names rather than compiler-dependent ordinals, which is stronger for textual durability.

---

**Floating point**

If storing IEEE-754 `float32`, say so explicitly.

If format must support architectures where float representation differs, conversion is required.

On modern target machines IEEE-754 is overwhelmingly common, but a durable format should still define:

```text
binary32
binary64
little-endian byte order.
```

Text float parsing must also reject:

```text
NaN
Infinity
```

where they violate engine invariants.

Your Kairo reflection layer explicitly rejects non-finite floating values in `PropertyValue`, which is good defensive behavior.

---

**Checksums and content hashes**

A checksum answers:

> Are these bytes likely corrupted?

A cryptographic/content hash can answer:

> Is this exactly the same content identity?

They are related but not identical roles.

---

**Simple checksum**

Example CRC/FNV-style checksum:

$$
C=Checksum(bytes).
$$

Recovery snapshot stores:

```text
payload size
payload checksum.
```

On load:

$$
size_{actual}=size_{expected}
$$

and:

$$
Checksum(actual)=Checksum(expected).
$$

If not:

```text
reject recovery point.
```

Your current recovery system does this.

---

**FNV is not a security primitive**

Kairo's recovery code explicitly uses FNV-1a as a **corruption detector**, not cryptographic authentication.

Correct.

If an attacker deliberately modifies both payload and checksum, FNV offers no meaningful tamper resistance.

For untrusted distribution/integrity, use something stronger:

$$
SHA\text{-}256
$$

or authenticated signatures/MACs depending on threat model.

---

**Content hash versus checksum**

DDC:

$$
SHA256(recipe)
$$

is serving identity.

Recovery:

$$
FNV(payload)
$$

is serving accidental-corruption detection.

Do not conflate these.

Same mathematical tool family, different security/semantic requirements.

---

**Chunk checksums**

Large binary artifact could contain:

```text
Header
Chunk table
Mesh chunk
Animation chunk
Texture chunk
```

You may checksum each chunk individually.

Then corruption can be localized.

You may also hash whole file.

Trade-off:

```text
one global hash = simpler
per-chunk hashes = better localization/streaming validation.
```

---

**Serialization of object references**

Object pointers cannot be persisted.

Suppose:

```cpp
Material* material;
```

At runtime address:

$$
0x7fa123...
$$

has no meaning on next launch.

Serialize stable identity instead:

```text
MaterialAssetID
```

or:

```text
SceneEntityID.
```

On load:

$$
ID\rightarrow object.
$$

---

**Two-pass reference resolution**

The universal pattern is:


Create every object and build:

$$
SerializedID
\rightarrow
RuntimeObject.
$$


Resolve references.

Example:

```text
Entity 10 parent 4
Entity 4 parent none
```

Even if Entity 10 appears first in file, loading works.

This same pattern appeared in Pass 12 for cloning and ECS extraction.

---

**Reference classes**

Not every reference has same ownership semantics.

You may distinguish:

```text
Strong reference
Weak reference
Optional reference
External asset reference
Scene-local reference
Cross-scene reference.
```

For example:

```text
MeshRenderer -> MeshAsset
```

can remain valid when entity changes.

A parent relation typically requires target in same loaded scene.

A gameplay target may be optional and disappear.

Serializer must understand the semantics.

---

**ECS entity serialization**

Never serialize:

```cpp
Entity{Index=42, Generation=7}
```

as persistent entity identity unless that runtime handle is explicitly defined as persistent—which it generally should not be.

Runtime slot allocation is transient.

Serialize:

```text
PersistentEntityID
```

and create runtime entities anew on load.

---

**Entity reference conversion**

Save:

```text
Runtime Entity {12,4}
      ↓
Scene Entity 105
```

Serialized reference:

```text
105.
```

Load:

```text
Scene Entity 105
      ↓
Runtime Entity {3,1}.
```

The runtime representation can be completely different.

That is desirable.

---

**ECS component schema**

A persistent ECS-like scene might encode:

```text
entity 105
component Transform
  position ...
component Camera
  fov ...
end
```

But remember Pass 12's architecture:

Kairo's durable `EngineCore::Scene` and process-local `KairoECS` are deliberately separate.

That means Kairo does not need to serialize its sparse-set internal indices.

It serializes authored scene semantics and extracts runtime ECS later.

This is the cleaner design.

---

**Scene-graph serialization**

A scene graph needs:

```text
entities
components
hierarchy
stable IDs
asset references.
```

There are two common hierarchy representations.

Nested:

```text
Root
{
    Child
    {
        Grandchild
    }
}
```

or flat IDs:

```text
entity 1 parent none
entity 2 parent 1
entity 3 parent 2.
```

Flat ID representation is often easier for:

```text
random ordering
references
incremental editing
streaming.
```

---

**Validate hierarchy after loading**

A parser must not assume file hierarchy is valid.

Check:

$$
Parent(E)\neq E.
$$

Check parent exists.

Check no cycles.

Check perhaps one root policy if required.

Check depth bounds if recursion is used.

Malicious/corrupt file:

```text
1 parent 2
2 parent 3
3 parent 1
```

must not lead to infinite traversal.

---

**Transactional scene loading**

Do not mutate live scene as parse proceeds.

Use:

```cpp
Scene candidate;

ParseInto(candidate);
Validate(candidate);

liveScene =
    std::move(candidate);
```

If parse fails at entity 997:

```text
live scene remains unchanged.
```

Your Kairo scene/project loading architecture repeatedly follows this strong-exception pattern.

---

**Reflection metadata**

Reflection metadata describes a type in a machine-readable engine-owned way.

Example:

```text
Type:
Kairo.Engine.Camera

Display:
Camera

Properties:
VerticalFov
NearPlane
FarPlane
Exposure
Primary
```

Each property can carry:

```text
stable key
display name
value kind
category
tooltip
numeric range
read-only flag
enum options
reference target type.
```

KairoReflection already implements essentially this model.

---

**Metadata is not the object's value**

Descriptor:

```text
"near plane is a floating point value"
```

is metadata.

Instance:

```text
NearPlane = 0.1
```

is data.

Separate:

$$
Schema
$$

from:

$$
InstanceState.
$$

That enables one descriptor to inspect thousands of objects.

---

**Stable reflection keys**

Your reflection system uses dotted ASCII identifiers such as:

```text
Kairo.Engine.Transform
```

instead of compiler RTTI names.

This is a very good choice.

RTTI names can be:

```text
compiler-dependent
mangled
unstable across builds.
```

A persistent reflection key should be designed as data schema identity.

---

**Type IDs**

A type ID is stable identity for a reflected type.

You have several implementation choices.

**String key**

```text
Kairo.Engine.Transform
```

Advantages:

- readable,
- collision-free if uniqueness enforced,
- persistent-friendly.

Cost:

- larger comparisons/storage.

**Integer ID assigned at runtime**

Fast, but registration order can make it unstable.

**Hashed stable string**

$$
TypeID=Hash("Kairo.Engine.Transform").
$$

Compact and deterministic, but collisions must be handled.

---

**Persistent versus runtime type IDs**

A useful design is:

```text
persistent:
stable string/hash

runtime:
dense TypeIndex
```

At startup:

```text
"Kairo.Engine.Transform"
    ↓
runtime index 7.
```

Then hot loops can use:

```cpp
uint16_t TypeIndex;
```

while persistence remains compiler/order independent.

This is the same persistent/runtime identity separation we have seen everywhere.

---

**Hash collisions**

If:

$$
TypeID=Hash(key),
$$

you cannot simply assume collisions never occur.

At registration:

```cpp
if (
    sameHash &&
    differentKey)
{
    fail loudly;
}
```

or use sufficiently wide IDs and store canonical key for validation.

A 64-bit hash makes accidental collision unlikely but not logically impossible.

---

**Property IDs**

Within one type, every property needs stable identity.

Bad identity:

```text
property index 3.
```

Reordering fields changes meaning.

Better:

```text
Kairo.Engine.Camera.NearPlane
```

or type-local:

```text
near-plane.
```

Stable property ID allows:

```text
undo
serialization
animation binding
editor presets
network replication
property diffing.
```

---

**Property display names are not IDs**

Display text:

```text
Near Plane
```

may later become:

```text
Near Clip
```

for UI wording.

That should not invalidate persistence.

Your ImGui inspector uses stable reflection keys for commit callbacks rather than display labels.

This is precisely correct:

```text
DisplayName = presentation
Key = identity.
```

---

**Reflection without language runtime reflection**

C++ historically does not provide Java/C#-style full runtime field enumeration.

So engines build their own reflection.

Kairo does not ask:

```cpp
for(auto member : typeof(Camera).members())
```

because ordinary C++ cannot generally do that in portable production code.

Instead descriptors are explicitly constructed and registered.

That is entirely valid.

---

**Explicit reflection registration**

Conceptually:

```cpp
TypeDescriptor CameraDescriptor()
{
    TypeDescriptor type;

    type.Key =
        "Kairo.Engine.Camera";

    type.DisplayName =
        "Camera";

    type.Properties.push_back(
        MakeMemberProperty<Camera>(
            {
                .Key =
                    "near-plane",
                .DisplayName =
                    "Near Plane"
            },
            &Camera::NearPlane));

    return type;
}
```

The compiler validates the member pointer type.

The engine owns metadata semantics.

---

**Benefits of explicit registration**

You choose deliberately what is reflected.

Private implementation details can remain invisible.

You can expose a semantic property that is not literally a field.

Example:

```text
Euler Rotation
```

could read/write underlying Quaternion.

Reflection is an abstraction layer.

It does not have to mirror memory layout.

---

**Compile-time reflection patterns**

Even without full language reflection, templates can generate descriptors.

Kairo's:

```cpp
MakeMemberProperty<Object, Member>(
    metadata,
    memberPointer)
```

is an example.

Compile time knows:

```text
Object type
Member type
pointer-to-member.
```

Runtime descriptor stores a type-erased accessor.

---

**Template-generated accessor**

Conceptually:

```cpp
descriptor.Read =
    [member](const void* object)
    {
        const Object* typed =
            static_cast<
                const Object*>(
                    object);

        return Encode(
            typed->*member);
    };
```

Write:

```cpp
descriptor.Write =
    [member](
        void* object,
        const PropertyValue& value)
    {
        Object* typed =
            static_cast<Object*>(
                object);

        typed->*member =
            Decode<Member>(value);
    };
```

This bridges compile-time type safety into runtime reflection.

---

**Reflection macro pattern**

Another common design:

```cpp
REFLECT_TYPE(Camera,
    PROPERTY(NearPlane),
    PROPERTY(FarPlane),
    PROPERTY(Exposure));
```

Macros reduce boilerplate but have drawbacks:

```text
debuggability
macro syntax
tooling complexity.
```

Modern template/constexpr code can often replace many macros.

Use macros only when they improve ergonomics materially.

---

**Static descriptor pattern**

You can make:

```cpp
template<>
struct Reflection<Camera>
{
    static constexpr auto Key =
        "Kairo.Engine.Camera";

    static TypeDescriptor Build();
};
```

Then generic registry bootstrap:

```cpp
registry.Register(
    Reflection<Camera>::Build());
```

This creates a strong association between C++ type and stable schema descriptor without relying on language RTTI names.

---

**Runtime reflection registry**

A registry maps stable key to descriptor:

$$
TypeKey
\rightarrow
TypeDescriptor.
$$

Kairo's registry uses deterministic ordered storage:

```cpp
std::map<std::string, TypeDescriptor>
```

and sorts properties by stable key during registration.

That is a deliberate determinism choice.

---

**Immutable descriptors after registration**

Your reflection registry does not silently mutate already registered descriptors.

This matters.

Suppose inspector is currently showing:

```text
property key "near-plane"
```

and a plugin reload changes its meaning underneath it.

That would be dangerous.

Immutable registration means descriptor semantics remain stable for that registry lifetime.

Plugin reload can construct a new registry/epoch rather than mutate existing contracts unpredictably.

---

**Registry validation**

Kairo validates:

```text
stable type keys
stable property keys
display names
categories
read accessor presence
write accessor for writable properties
numeric ranges
enum option uniqueness
reference metadata
array metadata limits.
```

This is exactly what reflection registration should do.

Fail when schema is registered, not three hours later when editor opens one rare component.

---

**Property getters and setters**

Reflection should not necessarily expose raw member addresses.

Getter/setter abstraction gives control.

Getter:

```cpp
PropertyValue Read(
    const void*);
```

Setter:

```cpp
void Write(
    void*,
    const PropertyValue&);
```

This enables validation and conversion.

---

**Why raw offset reflection is risky**

Some reflection systems store:

```cpp
offsetof(Camera, NearPlane).
```

Then compute:

```cpp
char* base +
offset.
```

Fast, but tightly coupled to standard-layout assumptions and exact memory layout.

It also bypasses semantic validation.

Getter/setter reflection is more flexible.

Kairo chooses getters/setters through `std::function`.

That is appropriate for editor/reflection workloads where correctness and flexibility matter more than nanosecond access.

---

**Semantic setters**

Suppose:

```cpp
void Camera::SetNear(float n)
{
    if (n <= 0)
        throw ...;

    NearPlane = n;
}
```

Reflection should ideally use this semantic path.

Likewise quaternion write may normalize.

Asset handle write may validate target type.

A property setter is an invariant boundary.

---

**Strong exception behavior**

Setter should preferably behave:

$$
\text{valid new value}
\Rightarrow
\text{commit}
$$

$$
\text{invalid new value}
\Rightarrow
\text{old value unchanged}.
$$

This is crucial because editor commands depend on reliable rollback/undo semantics later in M094.

---

**Inspector adapters**

Reflection metadata should remain UI-framework neutral.

It says:

```text
Property kind = FloatingPoint
Range = [0.01, 1000]
DisplayName = Near Plane
```

It should not say:

```text
ImGui::DragFloat
```

or:

```text
QtSlider.
```

UI adapter interprets metadata.

---

**Kairo's inspector boundary**

`ImGuiReflectionInspector` receives:

```text
ReflectionRegistry
Scene entity
commit callback.
```

It enumerates reflected components and picks an ImGui widget from `PropertyValueKind`.

That means:

```text
reflection metadata
```

does not import ImGui.

Instead:

```text
ImGui adapter
    ↓
reflection.
```

Correct dependency direction.

---

**Widget mapping**

Current logic naturally supports:

```text
Boolean          -> checkbox
Signed integer   -> numeric drag
Unsigned integer -> numeric drag
Floating point   -> ranged drag
String           -> text input
Vector2/3/4      -> component drag
Quaternion       -> 4-component editor
Enumeration      -> combo
Reference        -> reference string editor
Array            -> current summary/display
```

This is the generic-inspector payoff.

One UI implementation can inspect many component types.

---

**Inspector must not own mutation semantics**

Notice Kairo's inspector does not directly perform arbitrary scene writes.

When edited value changes, it invokes:

```cpp
commit(
    typeKey,
    propertyKey,
    edited);
```

That callback can generate an undoable command.

This is a very strong architectural separation:

```text
UI detects intent
      ↓
command layer performs mutation.
```

Without this, every widget becomes a secret mutation path bypassing undo.

---

**Serialization driven by reflection**

If reflection knows:

```text
type
properties
kinds
getters
```

generic serializer can do:

```cpp
for (const auto& property
     : descriptor.Properties)
{
    PropertyValue value =
        registry.Read(
            typeKey,
            property.Key,
            object);

    writer.Write(
        property.Key,
        value);
}
```

This removes duplicate hand-written field enumeration.

---

**Deserialization driven by reflection**

Conceptually:

```cpp
for (SerializedField field : input)
{
    const PropertyDescriptor* p =
        descriptor.Find(field.ID);

    if (!p)
    {
        HandleUnknown(field);
        continue;
    }

    PropertyValue value =
        Decode(
            field,
            p->ValueKind);

    registry.Write(
        descriptor.Key,
        p->Key,
        object,
        value);
}
```

Now reflection schema and persistence schema can share metadata.

---

**Reflection-driven serialization trade-off**

Be careful with the temptation:

> Reflect everything, therefore serialize everything.

Not every reflected property should be persisted.

Examples:

```text
runtime statistics
computed bounds
debug state
cached matrix
GPU handle.
```

So reflection metadata may need flags:

```cpp
Serializable
Transient
EditorVisible
ReadOnly
NetworkReplicated
SaveGame
```

depending on engine scale.

Reflection is a metadata platform.

Serialization is one consumer.

---

**Current Kairo design**

Kairo currently uses reflection primarily for editor inspection and semantic values while scene serialization remains an explicit dedicated format.

That is a defensible design.

Explicit scene serializers give precise control over:

```text
versioning
ordering
validation
migration.
```

Reflection can gradually reduce boilerplate later without forcing persistence to become purely automatic.

---

**Editor UI driven by reflection**

The generic inspector architecture is:

$$
Object
\rightarrow
TypeDescriptor
\rightarrow
PropertyDescriptors
\rightarrow
PropertyValue
\rightarrow
UIWidget.
$$

This means adding a new reflected property can automatically make it appear in the inspector.

That is a massive tooling scalability improvement.

---

**Metadata can improve UX**

Reflection property metadata can include:

```text
Tooltip
Category
Advanced flag
ReadOnly
NumericRange
Step
Enum options
Reference target.
```

Then an editor gains consistent behavior across hundreds of components.

Without metadata every component requires a bespoke panel.

---

**Generic inspector versus custom inspector**

Generic reflection UI handles the common 80–90%.

Some types still deserve a specialized inspector.

Example:

```text
Material
Animation graph
Particle system
Terrain
Navigation mesh.
```

Use generic reflection as the baseline.

Allow type-specific adapters when domain UX requires them.

Do not force everything through primitive widgets.

---

**Safe type erasure**

Type erasure allows runtime code to manipulate values whose concrete C++ type is not known at compile time.

Kairo reflection uses:

```cpp
const void*
void*
```

plus descriptor-specific functions.

This is a classic type-erasure boundary.

---

**`void*` itself is not safe**

This is dangerous:

```cpp
void* object =
    GetAnything();

descriptorForCamera.Read(object);
```

if object is actually a Light.

The cast inside accessor becomes invalid.

So safety comes from **pairing pointer with the correct descriptor**.

The invariant is:

$$
\boxed{
(objectAddress,TypeDescriptor)
}
$$

must correspond to the same concrete type.

---

**Safer erased object view**

You can encode this:

```cpp
struct ReflectedObjectView
{
    void* Object = nullptr;
    const TypeDescriptor* Type = nullptr;
};
```

Read:

```cpp
PropertyValue Read(
    ReflectedObjectView object,
    PropertyID property);
```

Now the pointer and schema travel together.

Still not mathematically impossible to construct incorrectly, but far harder to mix accidentally.

---

**Runtime type token**

Stronger:

```cpp
struct ErasedObject
{
    void* Data;
    TypeID Type;
};
```

Registry checks:

$$
object.Type=
descriptor.TypeID
$$

before accessor invocation.

This converts silent undefined behavior into a deterministic runtime error.

---

**Owned type-erased values**

Kairo's `PropertyValue` is another safer kind of type erasure.

It uses:

```cpp
std::variant<
    bool,
    int64_t,
    uint64_t,
    double,
    string,
    Vector...
>
```

This is significantly safer than:

```cpp
void* value;
```

because the variant remembers its active alternative.

`PropertyValueKind` can be checked against expected descriptor kind before writing.

---

**Why `std::any` is less ideal here**

You could use:

```cpp
std::any.
```

But then the reflection transport becomes dependent on concrete C++ type identities.

Kairo deliberately defines a canonical, engine-neutral value model:

```text
double
Vector3Value
ReferenceValue
EnumerationValue
ArrayValue.
```

That is much better for:

```text
editor
serialization
plugin boundaries
potential scripting.
```

---

**Reflection must own values, not dangling references**

Kairo's reflected `PropertyValue` owns strings, arrays and composite values.

This is important.

Returning:

```cpp
std::string_view
```

into a component would create lifetime hazards.

An inspector may keep the value after component storage changes.

Owned values decouple reflection transport lifetime from object storage lifetime.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/10 - Engine changes across subsystem contracts|Previous chapter]] · [[Chapter Edition/03 - Game Engine/12 - Transactions, recovery, and safe project boundaries|Next chapter]]
