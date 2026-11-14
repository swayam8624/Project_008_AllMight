# 12 — Transactions, recovery, and safe project boundaries

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/11 - Streaming resources through explicit states|Previous chapter]] · [[Chapter Edition/03 - Game Engine/13 - Portable startup, windows, events, and input|Next chapter]]

Interactive tools need edits that can be undone and projects that can be reopened after failure. This chapter makes mutation transactional and treats paths, packages, backups, and atomic publication as security and recovery contracts.

**By the end:** Design undo/redo, save recovery, path validation, and package publication so partial failure cannot silently corrupt authored work.

**Recall question:** Which state is authoritative before and after a failed save, and how does the system prove that the published project is complete?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 20056–24191; source-span SHA256 20157c1d83676d7f803a28e9d2f441eb5d630a317cfd607ae1734913dddc41fb. -->

<!-- CHAPTER-SOURCE-BEGIN -->
### UNDO, TRANSACTIONS, RECOVERY, PROJECT BOUNDARIES, PATH SECURITY, PACKAGING AND ATOMIC PUBLICATION

---

**Undo commands**

Undo begins with one idea:

$$
\boxed{
\text{An edit must contain enough information to reverse itself.}
}
$$

For property edit:

```text
Entity
Component
Property
Before
After.
```

For create entity:

```text
CreatedEntityID
complete creation state.
```

For delete entity:

```text
serialized deleted subtree.
```

The inverse operation must be deterministic.

---

**Command interface**

Kairo currently defines:

```cpp
class EditorCommand
{
public:
    virtual std::string_view
    Name() const noexcept = 0;

    virtual void Execute() = 0;
    virtual void Undo() = 0;

    virtual bool TryMerge(
        EditorCommand&) noexcept
    {
        return false;
    }
};
```

This is an excellent minimal command model.

Each command owns the state necessary for forward and reverse transitions.

---

**Creation command identity**

A subtle problem:

Create entity.

Undo.

Redo.

If redo allocates a **different Entity ID**, references or selection state may behave unexpectedly.

Kairo's creation commands preserve the allocated scene identity.

First execution:

```text
allocate ID 42.
```

Undo:

```text
destroy 42.
```

Redo:

```text
CreateEntityWithID(42).
```

This is exactly the right editor semantic.

---

**Undo commands should store semantic state**

For Light edit:

Bad:

```text
store pointer to Light
store raw bytes.
```

Better:

```text
Entity ID
Before LightComponent
After LightComponent.
```

Kairo's `SetSceneComponentCommand` follows this model.

It captures complete before/after component values.

Undo therefore does not depend on stale memory addresses.

---

**Command-pattern undo/redo**

Command history is a sequence:

$$
C_0,C_1,\dots,C_{n-1}.
$$

Maintain cursor $k$:

```text
[ applied commands | redo commands ]
                  ^
                cursor
```

If:

$$
k=3,
$$

commands 0–2 are applied.

Commands 3 onward are redoable.

---

**Undo**

Undo command:

$$
C_{k-1}.Undo().
$$

Only after successful Undo:

$$
k\leftarrow k-1.
$$

This ordering matters.

If Undo throws and cursor moved first, history would claim state changed when it did not.

Current Kairo does the operation first and changes cursor afterward.

Strong design.

---

**Redo**

Redo:

$$
C_k.Execute().
$$

Then:

$$
k\leftarrow k+1.
$$

Again cursor changes only after successful execution.

---

**New edit after undo**

History:

```text
A B C D
      ^
```

Suppose C/D are redo branch.

Now execute X.

Expected:

```text
A B X
```

not:

```text
A B X C D.
```

Kairo truncates redo entries when a new command executes after undo.

This is standard linear undo semantics.

---

**Command history memory**

Unlimited undo can retain huge snapshots.

Imagine deleting a 2 GiB terrain entity snapshot 500 times.

History becomes enormous.

Kairo bounds command history, defaulting to:

$$
512
$$

entries.

When full, oldest applied command is evicted.

That is a reasonable first memory policy.

More advanced systems use:

```text
byte budget
compressed snapshots
disk-backed history.
```

---

**Command merging**

Slider drag generates many micro-edits:

```text
1.00
1.01
1.02
...
3.50.
```

Without merging, Ctrl-Z walks every intermediate frame.

Kairo's:

```cpp
TryMerge(...)
```

allows adjacent compatible commands to coalesce.

A merged command should preserve:

```text
Before = initial value
After = latest value.
```

That makes one intuitive undo step.

---

**Transactional editor changes**

A transaction is stronger than an undo command.

It means an operation involving several mutations behaves:

$$
\boxed{
\text{all succeed or none become visible}.
}
$$

Suppose "Add Physics Preview" adds:

```text
RigidBody
Collider.
```

If RigidBody succeeds but Collider validation fails, leaving only one component is undesirable.

A transactional command should prevalidate or mutate a candidate state first.

---

**Strong exception guarantee**

For an operation $F$:

$$
F(State)=
\begin{cases}
State',&success\\
State,&failure
\end{cases}
$$

not:

$$
State_{half-mutated}.
$$

Kairo's command interface explicitly requires a strong exception guarantee from commands.

This is a very good contract.

---

**Validate before mutation**

Example:

```cpp
void SetMaterial(
    Entity e,
    MaterialAssetHandle asset)
{
    // May throw.
    assets.Resolve(asset);

    // Only mutate after validation.
    scene.SetMaterial(e, asset);
}
```

Kairo's logic-document command uses precisely this pattern: it resolves the typed asset before replacing the component.

This keeps failure atomic.

---

**Candidate-copy transactions**

For complex state:

```cpp
Scene candidate = scene;

ApplyOperation(candidate);

candidate.Validate();

scene =
    std::move(candidate);
```

This can be expensive but gives strong semantics.

Your RuntimeWorld and several project-loading paths already use candidate state before commit.

---

**Multi-command transactions**

Sometimes one UI action logically contains many commands.

Example prefab instantiation:

```text
create root
create 20 children
add components
set hierarchy
resolve references.
```

Undo should happen as one action.

Use composite command:

```cpp
class CompositeCommand
{
    vector<Command> Commands;
};
```

Execute in order.

Undo in reverse:

$$
C_n^{-1},
C_{n-1}^{-1},
\dots,
C_0^{-1}.
$$

If an Execute fails midway, undo already-applied children before propagating failure.

---

**Transaction boundary should match user intent**

Not every low-level mutation deserves an undo record.

User performs:

> Duplicate subtree.

That is one logical operation even though hundreds of component writes occur.

The undo stack should reflect **semantic authoring actions**, not internal implementation calls.

This dramatically improves editor usability.

---

**Recovery journals**

Undo protects against:

```text
user mistake during a live session.
```

Recovery protects against:

```text
process crash
OS crash
power loss
editor bug.
```

These are different problems.

Undo history stored only in RAM disappears during a crash.

Recovery state must live outside the process.

---

**Snapshot versus write-ahead journal**

Two broad approaches exist.

**Snapshot**

Periodically save complete recovery state:

```text
Project
Manifest
Scene
Documents
Draft buffers.
```

**Operation journal**

Append every mutation:

```text
SetTransform
DeleteEntity
ChangeMaterial
...
```

Then replay after last durable checkpoint.

Kairo currently uses a recovery **snapshot** model.

That is simpler and robust.

---

**Kairo recovery snapshot**

A recovery point contains metadata such as:

```text
original project root
project file
active scene
creation time
list of recovery files.
```

Each payload records:

```text
role
target path
payload path
byte count
checksum
dirty state
active state.
```

The snapshot includes:

```text
project descriptor
asset manifest
scene
authoring documents
text drafts.
```

This is impressively complete.

---

**Drafts versus canonical documents**

One subtle feature matters greatly.

A text editor buffer may currently contain invalid syntax.

Example user is halfway typing:

```text
function foo(
```

An autosave/recovery system must preserve those bytes.

It must **not** require them to parse as a valid authoritative document.

Kairo distinguishes:

```text
AuthoringDocument
TextDraft.
```

The recovery code explicitly says drafts may be syntactically invalid and should not be promoted to authoritative state merely because they were recovered.

That is exactly correct.

---

**Recovery integrity**

On load Kairo verifies:

$$
ActualByteCount=
ManifestByteCount
$$

and:

$$
Checksum(payload)=
ManifestChecksum.
$$

If one payload fails:

```text
reject recovery snapshot.
```

This is much safer than opening a partially written state and hoping.

---

**Recovery publication**

A recovery snapshot is built in a staging directory.

Only after payloads and manifest are complete is the snapshot directory published into the recovery root.

Thus discovery sees either:

```text
old complete snapshots
```

or:

```text
new complete snapshot.
```

Not a partially assembled snapshot masquerading as valid.

This is atomic-publication thinking at directory granularity.

---

**Retention**

Recovery data grows indefinitely unless bounded.

Kairo defaults to:

$$
10
$$

snapshots and permits up to:

$$
64.
$$

Old recovery points can be removed according to creation ordering.

This is the correct distinction:

```text
recovery data is redundant safety state,
not permanent project history.
```

---

**Autosave architecture**

Autosave is a policy layered over recovery primitives.

The safe definition is not:

> Call Save All every 30 seconds.

That can overwrite the user's authoritative files with partially edited state.

Better:

$$
\boxed{
Autosave
\rightarrow
RecoverySnapshot.
}
$$

The user's real project remains unchanged.

---

**Autosave should not clear dirty state**

Suppose scene is unsaved.

Autosave creates recovery snapshot.

The editor must still display:

```text
*
unsaved changes.
```

Because autosave did not commit the user's canonical project.

Kairo's recovery API explicitly says creating a recovery point does not modify:

```text
dirty flags
command history
authored files.
```

Excellent.

---

**Autosave scheduling**

A practical policy might trigger when:

$$
TimeSinceLastRecovery
>
T
$$

and:

$$
HasUnsavedChanges
$$

and editor is not currently in a critical file operation.

For example:

```cpp
if (
    session.HasUnsavedChanges() &&
    now - lastRecovery >
        std::chrono::minutes(2))
{
    session.CreateRecoveryPoint();
}
```

But do not perform heavy serialization synchronously every frame.

---

**Snapshot consistency under asynchronous autosave**

If autosave happens on a background thread while UI mutates Scene concurrently, copying live structures without synchronization is dangerous.

Options:

```text
short lock + snapshot copy
immutable/persistent document structures
command barrier
serialize a versioned copy.
```

The recovery job should operate on a stable snapshot, not moving live state.

---

**Current Kairo status for autosave**

The inspected code clearly provides:

```text
CreateRecoveryPoint
LoadRecoverySnapshot
RestoreRecoveryPoint
retention
checksums
draft preservation.
```

A full time-based autosave scheduler was not evident in the inspected recovery/session files.

So the **recovery mechanism exists**; automatic scheduling is a host/editor policy that can be layered on top.

That is a reasonable architectural separation.

---

**Project descriptors**

A project descriptor tells the engine how to bootstrap the project.

Current Kairo descriptor contains:

```text
Name
AssetManifest
StartupScene
EngineVersion
InputMap
RenderingProfile
GraphicsBackend
optional PlayExecutable
EnabledPlugins
BuildProfiles.
```

This is exactly the right level of information.

It does not contain:

```text
VkDevice
runtime ECS entity IDs
GPU resources.
```

It contains persistent bootstrap policy.

---

**Descriptor versus runtime state**

Descriptor:

```text
graphics-backend metal
```

is configuration.

Runtime:

```text
selected physical device
queue family
swapchain image count
```

is runtime state.

Do not serialize runtime discoveries back into project descriptor unless they are deliberate project settings.

---

**Descriptor versioning**

Kairo supports:

```text
kairo-project 1
kairo-project 2.
```

New v2 fields include broader engine/build settings.

The parser explicitly applies version-specific requirements.

That is much better than:

```text
try reading fields and guess format.
```

---

**Manifest validation**

A manifest should be validated structurally and semantically.

Structural validation:

```text
header valid?
token count correct?
values parse?
counts bounded?
records closed?
```

Semantic validation:

```text
IDs valid?
paths valid?
asset types valid?
dependencies exist?
dependency types match?
cycles absent?
duplicates absent?
```

Do both.

---

**Kairo asset manifest validation**

Current parser bounds:

$$
ManifestSize\le64\text{ MiB}.
$$

It bounds:

$$
AssetCount\le1,000,000.
$$

It bounds dependency count per asset.

It limits path/importer lengths.

It rejects unknown statements.

Then registry installation performs graph/topology validation.

This is strong layered validation.

---

**Why limits are security features**

Input says:

```text
array-count = 4,000,000,000.
```

Naive parser:

```cpp
vector.resize(count);
```

may attempt huge allocation.

Always validate before allocating:

$$
count\le MaximumAllowed.
$$

Every variable-length structure needs a budget.

This applies even to "local project files" because corrupted data should not be capable of crashing the tool unpredictably.

---

**Duplicate validation**

Manifest might list same AssetID twice.

Or two paths differing only by case:

```text
Textures/A.png
textures/a.png.
```

On Windows/macOS configurations these may alias.

Kairo's portable path key case-folds logical paths to avoid such ambiguous manifests.

That is an important cross-platform policy.

---

**Sandboxed project-relative paths**

A project file should normally reference:

```text
Scenes/Main.kscene
Textures/A.png.
```

Not:

```text
C:\Users\Alice\Desktop\foo.png
```

or:

```text
/home/alice/foo.png.
```

Project-relative paths enable:

$$
\boxed{
relocatability
+
portability
+
sandboxing.
}
$$

---

**Path resolution model**

Given canonical root:

$$
R.
$$

Given validated relative path:

$$
p.
$$

Candidate:

$$
C=Canonical(R/p).
$$

Then require:

$$
C\in Descendants(R).
$$

That is stronger than lexical normalization alone because symlinks can alter filesystem resolution.

---

**Never use string-prefix containment**

Wrong:

```cpp
candidateString.starts_with(
    rootString);
```

If root:

```text
/Game
```

then:

```text
/Game2/evil.txt
```

also starts with `/Game`.

Kairo's `IsWithinProjectRoot()` compares filesystem path components instead.

Correct.

---

**Existing files and future outputs differ**

Existing path can use:

```cpp
std::filesystem::canonical(...)
```

because target exists.

Future save path may not exist yet.

Kairo uses:

```cpp
weakly_canonical(...)
```

for future output paths, resolving existing prefixes while allowing missing final components.

This distinction is exactly right.

---

**Path traversal prevention**

Classic traversal:

```text
../../../../etc/passwd
```

or:

```text
..\..\Windows\System32.
```

If joined naively:

```cpp
root / userPath
```

attacker can escape root.

Kairo's `NormalizeAssetPath()` rejects parent components.

That is the first line of defense.

---

**Absolute path injection**

Also reject:

```text
/etc/passwd
C:\Windows\System32
\\server\share
```

because path joining semantics may discard the project root.

A relative-project field should be **provably relative**.

Kairo checks:

```text
is_absolute
has_root_name
has_root_directory.
```

Good.

---

**Encodings and separators**

Cross-platform paths are tricky.

On Windows:

```text
\
```

is native separator.

Portable project formats should usually canonicalize to:

```text
/
```

when serialized.

Kairo uses generic path spelling and validates portable character sets.

This means the same manifest behaves consistently across host systems.

---

**`.` and empty components**

Normalize:

```text
Meshes/./Cube.obj
```

to:

```text
Meshes/Cube.obj.
```

Reject paths that normalize to the project root itself when a file is expected.

Canonical path representation reduces ambiguity and duplicate identities.

---

**Symlink escape prevention**

Lexical traversal checks are not enough.

Suppose:

```text
Project/
    Assets/
        External -> /etc
```

Then user path:

```text
Assets/External/passwd
```

contains no `..`.

Lexically it appears inside project.

Filesystem resolution escapes.

Therefore canonicalize after filesystem resolution and verify containment.

Kairo does this for existing project files.

---

**Future output symlink problem**

Suppose:

```text
Project/Saves
```

is a symlink to:

```text
/tmp/other.
```

Saving:

```text
Saves/Main.kscene
```

would escape project.

`ResolveProjectOutput()` uses `weakly_canonical(root / relative)` and then component-wise root containment.

This catches existing symlink redirects.

Good.

---

**Packaging is stricter**

Kairo's package collector rejects symbolic links entirely.

This is an excellent packaging policy.

Why?

A bundle should contain concrete files, not references whose meaning changes after relocation.

A symlink could point:

```text
outside project
to machine-specific location
to missing target after installation.
```

Rejecting all symlinks produces a much cleaner portable bundle.

---

**TOCTOU caveat**

There is one deeper security issue.

Sequence:

```text
canonicalize path
check path
open file
```

has a time-of-check/time-of-use window.

An adversarial process could replace a path component with a symlink between check and open.

For a normal trusted desktop project environment, canonicalization is usually sufficient.

For hardened untrusted sandboxes, use OS-level directory handles and no-follow APIs such as `openat`-style relative opening, platform equivalents, and descriptor-relative traversal.

That is the difference between:

$$
\text{robust editor sandboxing}
$$

and:

$$
\text{hostile multi-user filesystem security}.
$$

---

**Runtime packaging**

Packaging converts a development project into a runnable deployment bundle.

It should not simply `zip` the whole repository.

Exclude:

```text
.git
temporary caches
recovery journals
editor-only metadata
build outputs recursively copied into themselves.
```

Include:

```text
runtime project
required assets
player executable
required runtime libraries
manifest
launcher/config.
```

---

**Current Kairo package pipeline**

`PackageRuntimeProject()` roughly performs:

```text
select build profile
resolve player executable
validate output
create staging directory
collect bounded project payload
reject symlinks/non-regular entries
copy project
relocate descriptor as Project.kproject
copy player
write launcher
write package manifest
re-open staged RuntimeProject
publish staging directory.
```

That is a serious packaging pipeline rather than an ad-hoc copy script.

---

**Revalidate staged package**

One particularly good step is:

```cpp
RuntimeProject(
    packagedProject /
    "Project.kproject");
```

before publication.

This means all descriptor-relative paths are exercised in the **actual relocated layout**.

So:

```text
package builds
```

is not considered success until:

```text
packaged project can bootstrap.
```

Excellent acceptance gate.

---

**Package closure**

The next level beyond copying an entire project tree is dependency-closure packaging.

Given startup scene roots:

$$
R.
$$

Compute transitive asset closure:

$$
Closure(R).
$$

Package only needed cooked assets.

This reduces shipping size and prevents unused source content from leaking into distribution.

Current packaging appears project-tree based with filtering rather than complete asset-DAG cooking, so dependency-closure packaging remains a future optimization.

---

**Development versus release packaging**

Development bundle may include:

```text
symbols
debug shaders
validation layers configuration
diagnostics
editable assets.
```

Release:

```text
optimized cooked assets
stripped symbols
shipping config
no editor recovery state.
```

Project `BuildProfiles` already provide the right policy hook.

---

**Relocatable bundles**

A relocatable bundle runs regardless of absolute install directory.

Bad:

```text
/home/swayam/KairoBuild/project/Scenes/Main.kscene
```

embedded into executable.

Good:

```text
project/Project.kproject
```

resolved relative to package root.

---

**Launcher-relative discovery**

Kairo's Unix launcher determines:

```sh
KAIRO_PACKAGE_ROOT=
    directory of launcher
```

then executes:

```text
bin/KairoPlayer
project/Project.kproject
```

relative to that root.

Thus moving:

```text
Build/Release
```

to:

```text
/Applications/MyGame
```

does not break project paths.

That is relocatability.

---

**Shared libraries complicate relocatability**

If KairoPlayer dynamically depends on:

```text
libFoo.so
SomeFramework.dylib
Some.dll
```

package must include them and configure loader discovery.

Linux:

```text
RPATH / $ORIGIN
```

macOS:

```text
@executable_path
@loader_path
install names.
```

Windows:

```text
DLL beside executable
or controlled search paths.
```

Static linking reduces much of this complexity.

But a truly relocatable shared-library build must manage loader paths explicitly.

---

**Runtime content must also avoid absolute paths**

Even if executable locates project correctly, a scene containing:

```text
C:\Users\Swayam\Textures\a.png
```

kills relocatability.

That is why Part 1576's project-relative path discipline and Part 1580 are connected.

Relocatable packaging is not a packaging-only feature.

It starts at every persistent reference.

---

**Atomic publication by rename**

Suppose saving:

```text
Assets.kassets.
```

Bad:

```text
open destination truncate
write half
power loss.
```

Now original file is gone and replacement is incomplete.

Better:

```text
write temporary sibling
flush/close
validate
rename temp -> destination.
```

Within the same filesystem, rename/replacement can provide atomic **visibility** semantics.

Readers see old or new name, not half a file.

---

**Why same-directory temporary matters**

Rename between different filesystems may become:

```text
copy + delete
```

or fail.

Atomicity is typically guaranteed only within the same filesystem.

So create:

```text
Assets.kassets.tmp-UUID
```

beside:

```text
Assets.kassets.
```

Kairo follows this pattern.

---

**Windows versus POSIX replacement**

Kairo centralizes platform behavior in `AtomicFile.cppm`.

On Windows it uses:

```cpp
MoveFileExW(
    temporary,
    destination,
    MOVEFILE_REPLACE_EXISTING |
    MOVEFILE_WRITE_THROUGH);
```

On non-Windows it uses:

```cpp
std::filesystem::rename(
    temporary,
    destination);
```

This avoids duplicating platform-specific final-publication behavior across manifests/scenes/documents.

Good architecture.

---

**Atomic visibility is not full crash durability**

This distinction is extremely important.

Suppose:

```text
write()
flush C++ stream
rename()
```

completes.

`flush()` generally pushes data to the OS kernel.

It does **not necessarily guarantee** the data has reached stable physical storage.

For stronger POSIX crash durability you may need:

```text
fsync(temp file)
rename()
fsync(parent directory)
```

and equivalent platform-specific primitives.

Thus:

$$
\boxed{
Atomic\ rename
\neq
Guaranteed\ power-loss\ durability.
}
$$

Kairo's current atomic replacement design is strong against partial application-level writes and rename-level replacement failures, but do not overstate POSIX `std::filesystem::rename + ostream::flush` as full journaled-disk durability.

---

**Directory publication**

Packaging uses a similar principle.

Build complete staging directory:

```text
.Release.kairo-stage-123/
```

then rename:

```text
stage
→
Release.
```

If replacing an existing package, Kairo:

```text
existing -> backup
stage -> output
```

and attempts rollback if publication fails.

That is transactional directory publication.

---

**Multi-file atomicity**

Atomic rename solves one file.

Suppose project Save All modifies:

```text
Project.kproject
Assets.kassets
Main.kscene.
```

Saving each atomically does **not** make the set atomically consistent across a crash.

Crash after manifest replacement but before scene replacement can leave mixed generations.

Kairo's own comments correctly avoid claiming cross-file crash atomicity.

Recovery journaling addresses this higher-level problem.

---

**Ways to achieve stronger multi-file commits**

You can use:

**Generation directory**

Write:

```text
generation-42/
```

completely.

Then atomically switch:

```text
current -> generation-42
```

through a manifest/pointer.

**Journal**

Record intended transaction.

Apply files.

Mark commit complete.

Recovery either rolls forward or rolls back.

**Single container**

Store project in one database/container that supports transactions.

Each approach adds complexity.

For Kairo's current editor, independent atomic files + recovery snapshots is a very reasonable balance.

---

**Build configuration versus runtime configuration**

This distinction is frequently abused.

Build configuration determines **what code/artifacts are produced**.

Examples:

```text
Debug / Release
sanitizers enabled
editor target built
raytracer built
AI cloud provider compiled
Vulkan backend compiled.
```

Runtime configuration determines **how an existing executable behaves**.

Examples:

```text
renderer = metal
resolution = 1920×1080
volume = 0.8
startup scene
graphics quality.
```

---

**Build-time configuration**

Current Kairo CMake options include things like:

```text
BUILD_EDITOR
BUILD_PLAYER
BUILD_RAYTRACER
BUILD_COMPUTE_STACK
ENABLE_SANITIZERS
ENABLE_PACKAGING.
```

These change the target graph or compilation.

They belong to build configuration.

You cannot meaningfully toggle:

```text
ENABLE_SANITIZERS=true
```

at runtime if sanitizer instrumentation was never compiled in.

---

**Runtime configuration**

`ProjectDescriptor` contains:

```text
graphics-backend
rendering-profile
startup-scene
input-map
plugins
build profiles.
```

Some of these are persistent project policy consumed at startup.

For example:

```text
graphics-backend = auto
```

can be overridden by player command line:

```text
--renderer metal.
```

That is runtime policy.

---

**Compile-time feature vs runtime feature**

Suppose Vulkan support is not built.

Project says:

```text
graphics-backend vulkan.
```

Runtime cannot magically create Vulkan code.

So capability is:

$$
AvailableRuntimeOptions
=
CompiledCapabilities
\cap
PlatformCapabilities.
$$

Runtime configuration selects **within** compiled capabilities.

That is the correct relation.

---

**Environment configuration**

There is also deployment/environment configuration:

```text
cache directory
logging level
crash upload endpoint
server port.
```

Some may be runtime command-line/environment settings rather than project-persistent settings.

Do not stuff every setting into `ProjectDescriptor`.

Ask:

> Is this project content, user preference, machine configuration, build configuration or runtime session state?

Different scopes deserve different storage.

---

**Configuration precedence**

A common policy:

$$
Defaults
<
ProjectConfig
<
UserConfig
<
CommandLine.
$$

Example:

```text
engine default:
renderer auto

project:
renderer vulkan

user:
renderer metal

CLI:
--renderer opengl
```

Final:

```text
opengl.
```

The exact precedence is a product choice.

But it should be deterministic and documented.

---

**Never serialize secrets into project config accidentally**

Runtime configuration can include credentials:

```text
API tokens
server keys.
```

These should not enter:

```text
source-controlled Project.kproject.
```

Secrets belong in environment/secure credential stores.

This is another reason configuration scopes must remain distinct.

---

**Reflection + undo integration**

Now combine M093 and M094.

Inspector reads:

```text
TypeKey
PropertyKey
CurrentValue.
```

User edits.

Do not call reflection setter directly from UI.

Instead create:

```cpp
ReflectedPropertyCommand
{
    Entity;
    TypeKey;
    PropertyKey;
    Before;
    After;
};
```

Execute:

```text
registry.Write(...After).
```

Undo:

```text
registry.Write(...Before).
```

Now every reflected property automatically participates in undo.

That is one of the greatest practical benefits of stable property IDs.

---

**Example reflected property command**

```cpp
class SetReflectedPropertyCommand
    final : public EditorCommand
{
public:
    SetReflectedPropertyCommand(
        ReflectionRegistry& reflection,
        std::string type,
        std::string property,
        void* object,
        PropertyValue before,
        PropertyValue after)
        : m_Reflection(&reflection),
          m_Type(std::move(type)),
          m_Property(std::move(property)),
          m_Object(object),
          m_Before(std::move(before)),
          m_After(std::move(after))
    {}

    void Execute() override
    {
        m_Reflection->Write(
            m_Type,
            m_Property,
            m_Object,
            m_After);
    }

    void Undo() override
    {
        m_Reflection->Write(
            m_Type,
            m_Property,
            m_Object,
            m_Before);
    }

private:
    ReflectionRegistry* m_Reflection;

    std::string m_Type;
    std::string m_Property;

    void* m_Object;

    PropertyValue m_Before;
    PropertyValue m_After;
};
```

But there is a major problem.

---

**Do not store long-lived raw `void*` in editor commands**

That object's address may change.

Scene may be reloaded.

Entity may be destroyed.

Component may move.

So actual editor command should store:

```text
persistent Scene Entity ID
TypeKey
PropertyKey
Before
After.
```

On Execute/Undo:

```text
resolve entity
resolve component object
write reflected property.
```

Again:

$$
\boxed{
Identity
\neq
Address.
}
$$

This same principle has now appeared in ECS, assets, GPU resources, object references, undo and reflection.

---

**Reflection + serialization integration**

If every serializable property has stable ID:

```text
Camera.NearPlane
Camera.FarPlane
Camera.Exposure.
```

then file can store:

```text
type Kairo.Engine.Camera

property near-plane 0.1
property far-plane 1000
property exposure 1.
```

Generic loader resolves:

$$
TypeKey
\rightarrow
TypeDescriptor
$$

then:

$$
PropertyKey
\rightarrow
PropertyDescriptor.
$$

This is how reflection becomes schema machinery.

---

**Property rename problem**

Suppose:

```text
near-plane
```

renamed to:

```text
near-clip.
```

If property key changes, old serialized files break.

Therefore distinguish:

```text
DisplayName can change freely.

Persistent PropertyKey should remain stable.
```

If semantic key must change, provide migration alias:

```text
old near-plane
→ new near-clip.
```

Never casually rename persistent IDs for cosmetic reasons.

---

**Type rename problem**

Similarly:

```text
Kairo.Engine.MeshRenderer
```

might later be called:

```text
Kairo.Rendering.MeshRenderer
```

as code organization changes.

Should persistent type ID change?

Probably not unless schema identity truly changes.

Persistence identity should not necessarily mirror source namespace.

You may keep legacy stable key forever while C++ type moves namespaces.

This is why persistent IDs must not be auto-derived blindly from `typeid`.

---

**Unknown reflected properties**

Suppose plugin missing.

Scene has:

```text
Kairo.Plugin.Water
```

but registry does not contain that type.

Policies include:

```text
hard failure
preserve opaque component
load placeholder
disable component.
```

If editor aims to preserve projects even when optional plugin is temporarily unavailable, opaque preservation can be extremely valuable.

If runtime requires plugin for correctness, hard failure may be right.

This policy belongs to serialization/product design.

---

**Version reflection descriptors too**

If a reflected type's schema evolves, you may associate:

```cpp
struct TypeDescriptor
{
    StableTypeID ID;
    uint32_t SchemaVersion;
    ...
};
```

Then serializer can know:

```text
Component Camera schema 3.
```

This is more scalable than relying only on global scene-format version when plugins/components evolve independently.

Kairo currently uses stable type/property keys and versioned scene formats; per-type schema versions would be a possible future extension.

---

**Safe type erasure + plugins**

Suppose plugin registers:

```text
TypeDescriptor
accessor callbacks.
```

Then plugin unloads while registry still holds `std::function` lambdas whose code lives inside unloaded library.

Calling them becomes catastrophic.

So dynamic plugin systems require descriptor/code lifetime coupling.

Policy:

```text
plugin may unload only after
all registered descriptors are removed
and no reflected objects remain.
```

Or descriptors remain valid until process shutdown.

Type erasure solves type coupling.

It does not solve code lifetime automatically.

---

**Inspector transactions**

When user drags Transform X for 200 frames:

```text
UI needs immediate live preview.
```

But undo should be one command.

A strong pattern:

```text
BeginInteraction:
capture Before

During drag:
apply preview state

EndInteraction:
capture After
push one command representing Before→After
```

Be careful not to execute command twice if preview already applied.

Kairo's `TryMerge` model is another valid solution: each incremental command executes and subsequent commands merge into the previous history entry.

---

**Recovery versus undo history**

Should recovery snapshots store entire undo stack?

Not necessarily.

Two different recovery goals:

**Document recovery**

Restore unsaved content.

**Session recovery**

Restore undo/redo cursor, selection, tabs, camera, etc.

Kairo currently prioritizes authored/recovery payloads and session context such as active document.

That is a sound first target.

Recovering exact undo history requires serializing every command, which is significantly more complex.

---

**Recovery snapshots and checksum collision**

FNV-1a 64-bit collision is possible.

For accidental bit corruption, risk is sufficiently low for many editor recovery contexts.

For adversarially modified recovery directories, not secure.

If future threat model includes untrusted recovery payloads, replace or augment with SHA-256.

Security algorithms should match threat model rather than superstition.

---

**Recovery timestamp ordering**

Kairo goes beyond simply naming snapshots by wall clock.

Wall clocks can move backward.

Multiple snapshots can occur in same millisecond.

It ensures each published recovery timestamp is greater than prior valid snapshots.

This gives deterministic retention ordering even across clock rollback/collisions.

That is a thoughtful production detail.

---

**Project paths and case sensitivity**

Suppose Linux allows both:

```text
Textures/A.png
Textures/a.png.
```

Windows may treat them as same path.

A cross-platform project should reject ambiguous case-only duplicates.

Kairo's portable key lowercases paths for lookup while preserving authored spelling.

This is a pragmatic cross-platform policy.

---

**Unicode path complexity**

Even case folding does not solve every Unicode issue.

Characters may have multiple normalization forms:

$$
é
$$

can be one code point or:

$$
e+\text{combining accent}.
$$

Some filesystems normalize automatically; others do not.

A fully portable asset pipeline may need defined Unicode normalization rules.

Kairo's current path rules concentrate on portable ASCII-sensitive filesystem hazards and Windows reserved names, which is already strong; deeper Unicode canonicalization can become relevant for global content pipelines.

---

**Manifest canonical ordering**

Current AssetManifest serializer sorts assets by stable AssetID and dependencies by dependency ID.

This means semantically identical registry state serializes consistently.

Benefits:

```text
clean diffs
stable hashes
reproducible builds
lower merge noise.
```

Deterministic ordering is one of the cheapest high-value persistence improvements.

---

**Reflection registry canonical ordering**

Similarly Kairo:

```text
uses std::map for types
sorts properties by stable key.
```

That makes descriptor snapshots deterministic independent of registration call order.

Again, deterministic metadata matters for:

```text
tooling
testing
serialization
plugin diagnostics.
```

---

**Build config should not leak into data semantics accidentally**

Suppose Debug build serializes:

```text
sizeof(size_t)=8
```

while another target differs.

Persistent file must not depend on compile flags such as:

```text
NDEBUG
packing pragmas
compiler ABI.
```

The serialization layer acts as the firewall between:

$$
BuildConfiguration
$$

and:

$$
PersistentDataSchema.
$$

This is one reason explicit field serialization matters.

---

**Runtime config should not invalidate DDC unless semantically relevant**

Suppose runtime setting:

```text
masterVolume = 0.5.
```

It should not change asset cache keys.

But build/import setting:

```text
texture compression = BC7.
```

must.

So configuration scope influences derivation identity.

Ask:

$$
\text{Can this setting change artifact bytes?}
$$

If yes:

$$
\text{include in DDC recipe}.
$$

If no, do not over-invalidate.

---

**Configuration is another dependency graph**

Artifact:

$$
A=F(Source,Settings,Toolchain).
$$

Thus configuration values are hidden edges in build graph.

If a setting influences output but is not included in cache key:

$$
\text{cache correctness bug}.
$$

This connects M092's DDC logic directly to Part 1582.

---

**Current Kairo audit for M093/M094**

The inspected implementation currently provides:

| Area | Current Kairo status |
|---|---|
| Versioned project formats | Implemented |
| Versioned scene formats | Implemented |
| Versioned asset manifest | Implemented |
| Strict unknown-statement handling | Implemented in inspected text formats |
| Stable asset/entity references | Implemented |
| Stable reflection type/property keys | Implemented |
| UI-neutral reflection values | Implemented |
| Runtime reflection registry | Implemented |
| Generic getter/setter descriptors | Implemented |
| Generic ImGui reflection inspector | Implemented |
| Safe owned `PropertyValue` type erasure | Implemented |
| `void*` object erasure | Implemented, descriptor pairing remains invariant |
| Command-pattern undo/redo | Implemented |
| Bounded command history | Implemented |
| Redo-branch truncation | Implemented |
| Command merging hook | Implemented |
| Scene creation commands preserve IDs | Implemented |
| Strong-exception command contract | Explicit |
| Recovery snapshots | Implemented |
| Recovery payload checksums | Implemented |
| Invalid text draft recovery | Implemented |
| Recovery retention | Implemented |
| Periodic autosave scheduler | Not evident in inspected core recovery code |
| Project descriptor v2 | Implemented |
| Asset manifest validation | Implemented |
| Project-relative path normalization | Implemented |
| Traversal rejection | Implemented |
| Canonical symlink escape checks | Implemented |
| Packaging symlink rejection | Implemented |
| Runtime package staging | Implemented |
| Staged package bootstrap validation | Implemented |
| Relocatable launcher | Implemented |
| Atomic file replacement | Implemented |
| Atomic directory package publication | Implemented |
| Full fsync-style POSIX crash durability | Not established by the inspected atomic-file code |
| Build/runtime config separation | Strongly present |

---

**Important Kairo improvement: checked erased object views**

Current reflection accessors accept:

```cpp
const void*
void*.
```

The registry validates:

```text
type key
property key
PropertyValue kind.
```

But it cannot intrinsically prove that:

```text
void*
```

actually points to the C++ type associated with descriptor.

A useful future abstraction:

```cpp
struct ReflectedObject
{
    void* Address = nullptr;
    std::string_view TypeKey;
};
```

Then:

```cpp
registry.Write(
    object,
    property,
    value);
```

compares:

$$
object.TypeKey=
descriptor.Key.
$$

This turns one class of UB into deterministic validation.

---

**Stronger alternative using runtime token**

```cpp
using RuntimeTypeToken =
    std::uint32_t;

struct ReflectedObject
{
    void* Address;
    RuntimeTypeToken RuntimeType;
};
```

TypeDescriptor contains matching token.

Persistent key remains:

```text
Kairo.Engine.Camera.
```

Runtime token is dense and fast.

Again:

$$
PersistentID
\neq
RuntimeIndex.
$$

Same architecture as assets and ECS.

---

**Important Kairo improvement: reflection serialization policy flags**

Current metadata already has:

```text
ReadOnly
Multiline
Advanced.
```

Future flags might include:

```cpp
enum class PropertyFlags
{
    None,
    ReadOnly,
    Multiline,
    Advanced,

    Serializable,
    Transient,
    SaveGame,
    Replicated,
    EditorOnly
};
```

This lets reflection drive more systems without assuming every visible property belongs in every persistence domain.

---

**Important Kairo improvement: field aliases**

For schema evolution:

```cpp
PropertyMetadata
{
    Key = "near-clip",
    LegacyKeys =
    {
        "near-plane"
    }
};
```

Deserializer sees old field:

```text
near-plane
```

and maps it to current property.

Writer emits only:

```text
near-clip.
```

This gradually canonicalizes old formats.

---

**Important Kairo improvement: descriptor schema version**

```cpp
struct TypeDescriptor
{
    std::string Key;
    uint32_t SchemaVersion;

    ...
};
```

Then plugin/component persistence can evolve independently from whole-scene version.

This becomes especially valuable once third-party components exist.

---

**Important Kairo improvement: recovery scheduling**

Existing primitives are already strong.

A thin editor policy can add:

```cpp
class RecoveryScheduler
{
public:
    void Tick(
        ProjectSession& session,
        TimePoint now)
    {
        if (
            !session.HasUnsavedChanges())
        {
            return;
        }

        if (
            now - m_LastSnapshot <
            m_Interval)
        {
            return;
        }

        session.CreateRecoveryPoint();

        m_LastSnapshot = now;
    }

private:
    Duration m_Interval =
        std::chrono::minutes(2);

    TimePoint m_LastSnapshot{};
};
```

In production, snapshot creation may be delegated to an I/O worker after capturing immutable state.

---

**Important Kairo improvement: full durable save option**

Current atomic rename semantics are excellent for normal editor failure resistance.

For workloads where power-loss durability matters, introduce a lower-level durable file abstraction:

```text
write temp
flush userspace
fsync temp
atomic rename
fsync containing directory.
```

On Windows, use the appropriate write-through/handle flush semantics.

Then expose:

```text
SaveMode::Atomic
SaveMode::Durable.
```

Not every editor keystroke requires durability-grade fsync latency.

Recovery snapshots perhaps do.

---

**Important Kairo improvement: package dependency closure**

Current package process copies a validated project payload with exclusions.

Future release cooker should use:

```text
startup scene
+
runtime config roots
        ↓
asset dependency DAG
        ↓
transitive required assets
        ↓
platform-specific derived artifacts
        ↓
runtime package.
```

Then exclude:

```text
source OBJ
source PSD
unused scenes
unused editor documents
local DDC.
```

This would complete Pass 13's authoring/runtime separation.

---

**Reconstruction challenge — serialization**

Given:

```cpp
struct Camera
{
    float Near;
    float Far;
};
```

Explain why this is wrong:

```cpp
file.write(
    reinterpret_cast<char*>(&camera),
    sizeof(Camera));
```

Then derive a robust representation defining:

```text
format identity
version
field IDs/names
float representation
validation
unknown-field policy.
```

If you cannot explain padding, ABI, endian, schema evolution and unknown-field consequences, serialization is not yet mastered.

---

**Reconstruction challenge — compatibility**

Suppose v1:

```text
entity
position
rotation
```

v2 adds:

```text
scale.
```

Answer separately:

**New reader loading old file**

Backward compatibility.

Give:

```text
scale = 1.
```

**Old reader loading new file**

Forward compatibility.

Possible only if unknown `scale` can be skipped or preserved.

Then explain why strict parsers intentionally reject it.

---

**Reconstruction challenge — object references**

Given:

```text
Entity 10 parent Entity 5
```

runtime entities are:

```text
10 -> {42,7}
5  -> {91,3}.
```

Save:

```text
10 parent 5.
```

Never:

```text
42:7 parent 91:3.
```

Load:

```text
create all persistent entities
build ID map
resolve parent references
validate no cycles.
```

That algorithm should now be automatic.

---

**Reconstruction challenge — reflection**

Given C++ field:

```cpp
float Camera::Exposure;
```

derive:

```text
Stable TypeKey
Stable PropertyKey
DisplayName
PropertyValueKind
Read accessor
Write accessor
Range
Flags.
```

Then trace:

```text
Camera instance
    ↓
registry.Read
    ↓
PropertyValue
    ↓
ImGui adapter
    ↓
user changes
    ↓
command
    ↓
registry.Write.
```

That is the complete reflection/editor path.

---

**Reconstruction challenge — type erasure**

Explain why:

```cpp
void* object
```

does not provide type safety.

Safety comes from:

```text
object pointer
+
correct descriptor
+
validated PropertyValue kind.
```

Then propose runtime TypeToken if you need explicit misuse detection.

That is safe type erasure.

---

**Reconstruction challenge — undo**

History:

```text
A B C D
cursor after D.
```

Undo twice:

```text
A B | C D.
```

Execute X.

Result:

```text
A B X |
```

C/D discarded.

Then explain why cursor moves only **after** Execute/Undo succeeds.

That is command-history correctness.

---

**Reconstruction challenge — recovery**

Explain difference:

```text
Save
```

writes authoritative state.

```text
Undo
```

reverses semantic edits in current process.

```text
Recovery snapshot
```

protects unsaved state against process failure.

```text
Autosave
```

periodically produces recovery state without necessarily committing authoritative files.

Those are four separate mechanisms.

---

**Reconstruction challenge — path sandbox**

Given:

```text
ProjectRoot =
/Users/x/Game

Input =
Assets/../Outside/file
```

reject lexically.

Given:

```text
Assets/External
```

is symlink to:

```text
/etc.
```

canonical path escapes root.

Reject after filesystem resolution.

Then explain why string prefix comparison is unsafe and path-component comparison is required.

---

**Reconstruction challenge — atomic save**

Correct sequence:

```text
serialize/validate
      ↓
write sibling temp
      ↓
flush/close
      ↓
atomic replace
      ↓
cleanup.
```

Explain:

$$
\text{atomic visibility}
$$

versus:

$$
\text{power-loss durability}.
$$

They are not identical.

That distinction is important enough to remember permanently.

---

**Reconstruction challenge — package relocation**

Bundle:

```text
MyGame/
├── run.sh
├── bin/
│   └── KairoPlayer
└── project/
    ├── Project.kproject
    ├── Scenes/
    └── Assets...
```

Launcher discovers root from itself.

All project paths are relative.

Therefore moving:

```text
/home/a/MyGame
```

to:

```text
/opt/MyGame
```

should still work.

That is relocatability.

---

**Reconstruction challenge — configuration**

Classify:

```text
BUILD_EDITOR=ON
```

as build-time.

```text
--renderer metal
```

as runtime.

```text
startup-scene Scenes/Main.kscene
```

as persistent project runtime policy.

```text
OPENAI_API_KEY
```

as environment/secret configuration.

```text
texture-compression=BC7
```

as asset-build configuration and therefore potentially a DDC-key input.

If you can classify settings by lifetime/scope, you understand Part 1582 rather than merely memorizing terms.

---

**The deep unifying principle of M093**

Serialization, reflection and compatibility are all fundamentally about **stable semantic identity**.

You need stable identity for:

$$
Type
$$

$$
Property
$$

$$
Entity
$$

$$
Asset
$$

$$
FileFormat.
$$

And you must keep those identities independent from:

```text
memory address
vector index
compiler RTTI name
display label
filesystem location
GPU handle.
```

This is the exact same principle that has appeared repeatedly throughout the curriculum.

---

**The deep unifying principle of M094**

Undo, recovery and atomic save are all forms of **controlled state transition**.

Instead of:

$$
State
\rightarrow
Mutation
$$

think:

$$
\boxed{
State_{old}
\xrightarrow{validated\ transaction}
State_{new}.
}
$$

And preserve enough information to either:

$$
Rollback
$$

or:

$$
Recover.
$$

Undo gives semantic rollback.

Atomic rename gives file-level publication.

Recovery gives crash-level restoration.

Staging packaging gives directory-level publication.

These are the same systems idea applied at different scopes.

---

**Exact coverage — M093 + M094**

| Part | Topic | Status |
|---:|---|---|
| 1546 | Serialization goals | ✓ |
| 1547 | Binary versus text serialization | ✓ |
| 1548 | Versioned file formats | ✓ |
| 1549 | Schema evolution | ✓ |
| 1550 | Forward compatibility | ✓ |
| 1551 | Backward compatibility | ✓ |
| 1552 | Unknown-field handling | ✓ |
| 1553 | Stable endianness and numeric widths | ✓ |
| 1554 | Checksums and content hashes | ✓ |
| 1555 | Serialization of object references | ✓ |
| 1556 | Serialization of ECS entities | ✓ |
| 1557 | Scene-graph serialization | ✓ |
| 1558 | Reflection metadata | ✓ |
| 1559 | Type IDs | ✓ |
| 1560 | Property IDs | ✓ |
| 1561 | Reflection without language runtime reflection | ✓ |
| 1562 | Compile-time reflection patterns | ✓ |
| 1563 | Runtime reflection registries | ✓ |
| 1564 | Property getters and setters | ✓ |
| 1565 | Inspector adapters | ✓ |
| 1566 | Serialization driven by reflection | ✓ |
| 1567 | Editor UI driven by reflection | ✓ |
| 1568 | Safe type erasure | ✓ |
| 1569 | Undo commands | ✓ |
| 1570 | Command-pattern undo/redo | ✓ |
| 1571 | Transactional editor changes | ✓ |
| 1572 | Recovery journals | ✓ |
| 1573 | Autosave architecture | ✓ |
| 1574 | Project descriptors | ✓ |
| 1575 | Manifest validation | ✓ |
| 1576 | Sandboxed project-relative paths | ✓ |
| 1577 | Path traversal prevention | ✓ |
| 1578 | Symlink escape prevention | ✓ |
| 1579 | Runtime packaging | ✓ |
| 1580 | Relocatable bundles | ✓ |
| 1581 | Atomic publication by filesystem rename | ✓ |
| 1582 | Build configuration versus runtime configuration | ✓ |


You now have the complete persistence/tooling chain:

$$
\boxed{
\begin{aligned}
\text{C++ object}
&\rightarrow
\text{stable reflected schema}
\\
&\rightarrow
\text{versioned semantic representation}
\\
&\rightarrow
\text{validated references}
\\
&\rightarrow
\text{atomic persistent file}
\\
&\rightarrow
\text{recovery-protected editor state}
\\
&\rightarrow
\text{relocatable runtime package}.
\end{aligned}
}
$$

And the Kairo-specific architecture is already substantially aligned with that model: **strict versioned text formats, persistent IDs instead of pointers, stable reflection keys instead of RTTI names, generic owned reflection values, UI-neutral descriptors, command-based undo, recovery snapshots with checksums, canonicalized project-relative paths, symlink escape checking, staged package validation, and atomic replacement.**

The biggest remaining upgrades from this pass are not conceptual rewrites. They are production hardening: **runtime-checked reflected object/type pairing, reflection serialization-policy flags and schema aliases, automatic recovery scheduling, stronger fsync-level durability where required, and dependency-closure/platform-cooked runtime packages.**

**Pass 15 is M095 + M096, Parts 1583–1600.** It closes Phase 16 with **CMake's target model; PUBLIC/PRIVATE/INTERFACE dependency propagation; imported/alias targets; presets; Ninja's build DAG; compiler toolchains; Clang/MSVC portability; C++ modules across targets; Git submodule pinning; reproducible superbuilds; CI matrices; unit/integration/smoke testing; crash reporting; structured logs/profiling hooks; portable subsystem boundaries; and finally drawing the full ownership/data flow from source asset → editor → package → player → runtime subsystem.**


**Complete source→editor→runtime data and ownership flow**

This is the culmination of the entire Phase 16.

We are going to trace **one texture**, **one scene entity**, and **one frame** through the whole engine.

If you can draw this from memory, you understand the phase.

---

**Stage 1 — source content**

Artist creates:

```text
Project/
└── Textures/
    └── Wall.png
```

At this stage the authoritative thing is:

$$
SourceBytes.
$$

Ownership:

```text
filesystem/project content.
```

It is not yet:

```text
renderer texture
GPU image
runtime handle.
```

---

**Stage 2 — persistent asset registration**

Asset system gives it:

$$
AssetID=A_{wall}.
$$

Metadata:

```text
ID
Type = Texture
Path = Textures/Wall.png
Importer = kairo.texture.stb
Dependencies...
```

Ownership:

```text
AssetRegistry owns metadata.
```

Important separation:

$$
A_{wall}
\neq
"Textures/Wall.png".
$$

Move path; ID survives.

---

**Stage 3 — import recipe**

Import database records:

```text
source fingerprint
importer ID
importer version
canonical settings.
```

Derived key:

$$
K=
H(
SourceFingerprint,
ImporterID,
ImporterVersion,
Settings,
Target,
DependencyInputs
).
$$

Ownership:

```text
import pipeline.
```

This is reproducibility metadata.

---

**Stage 4 — derived artifact**

Importer produces:

```text
TextureArtifact
```

with:

```text
dimensions
format
mip levels
pixel payload.
```

DDC stores it under immutable key K.

Ownership:

```text
DerivedDataCache.
```

Deleting DDC should not destroy project identity.

The artifact is reproducible.

---

**Stage 5 — authored scene reference**

A scene entity's MeshRenderer/Material may store:

```text
TextureAssetHandle{
    AssetID = A_wall
}.
```

Ownership:

```text
EngineCore Scene.
```

It does not store:

```text
VkImage
TextureHandle
filesystem pointer.
```

Excellent lifetime separation.

---

**Stage 6 — editor inspection**

Reflection registry describes:

```text
MeshRenderer
Material
asset-reference property.
```

Inspector displays stable semantic reference.

User edits.

The ImGui layer emits intent:

```text
Set property
```

rather than mutating arbitrary bytes.

Ownership flow:

```text
UI
    ↓
Editor command
    ↓
Scene persistent state.
```

---

**Stage 7 — undo transaction**

Command captures:

```text
Entity ID
TypeKey
PropertyKey
Before
After.
```

Execute modifies scene.

History owns command.

Undo reverses.

The scene remains the authoritative authored state.

No renderer object is directly edited by the property widget.

---

**Stage 8 — persistent save**

Scene serializer writes:

```text
stable entity IDs
component schema
asset IDs
hierarchy references.
```

It uses a versioned format.

Save sequence:

```text
serialize candidate
write temporary sibling
flush/close
atomic replace.
```

Ownership transfers from in-memory scene representation to durable file representation.

They represent the same semantics, not the same memory.

---

**Stage 9 — recovery path**

If scene is unsaved:

```text
CreateRecoveryPoint
```

serializes current in-memory state into separate recovery snapshot.

Recovery owns redundant safety copies.

It does **not** become canonical project state unless explicitly restored.

---

**Stage 10 — packaging**

Build profile determines output.

Packaging creates staging tree:

```text
package/
├── bin/KairoPlayer
├── project/Project.kproject
├── project/Scenes/...
└── package.kmanifest
```

It rejects unsafe symlinks and transient editor state.

Then staged project is reopened and validated.

Only after success does package become visible.

Ownership:

```text
packaging pipeline
→ published bundle.
```

---

**Stage 11 — Player bootstrap**

Launcher determines package root.

Runs:

```text
KairoPlayer
project/Project.kproject.
```

Player loads:

```text
ProjectDescriptor
AssetManifest
StartupScene.
```

No editor is required.

The runtime composition root constructs:

```text
runtime project
ECS bridge
physics
audio
input
renderer
gameplay
render bridge.
```

---

**Stage 12 — scene→runtime ECS extraction**

Persistent authored:

```text
Scene Entity 1007
```

becomes:

```text
Runtime Entity {Index,Generation}.
```

Bridge records:

$$
SceneID
\leftrightarrow
RuntimeEntity.
$$

Persistent scene remains distinct from process-local ECS storage.

---

**Stage 13 — asset runtime resolution**

The runtime entity references:

$$
A_{wall}.
$$

Asset runtime resolves:

```text
AssetID
   ↓
derived artifact
   ↓
renderer resource creation.
```

Renderer creates:

```text
TextureHandle T17
```

which may internally correspond to:

```text
VkImage
MTLTexture
ID3D12Resource
OpenGL texture.
```

Now we have three distinct identities:

$$
A_{wall}
$$

persistent,

$$
T17
$$

runtime renderer identity,

and:

$$
GPUObject
$$

backend-specific object.

Never merge them conceptually.

---

**Stage 14 — simulation**

Runtime systems operate on:

```text
ECS entities
physics BodyIDs
input state
script state.
```

Physics may map:

$$
RuntimeEntity
\leftrightarrow
BodyID.
$$

Renderer does not become physics owner.

Subsystem-specific identities remain separate.

---

**Stage 15 — render extraction**

ECS:

```text
Transform
Renderable
Light
Camera
```

becomes frame data:

```text
RenderScene
MeshDraw[]
Lights
Camera.
```

This is a **copy/extraction boundary**.

Renderer receives exactly what it needs.

It does not traverse arbitrary editor Scene internals.

---

**Stage 16 — render graph**

Renderer compiles:

```text
passes
resources
accesses
transitions
lifetimes.
```

RenderGraph produces execution plan.

CPU timing can produce:

```text
RenderGraphExecutionProfile.
```

Backend records GPU work.

---

**Stage 17 — GPU submission**

Backend submits resources.

Now their lifetime extends beyond CPU frame construction.

For a resource R:

$$
Lifetime(R)
>
LastGPUUse(R).
$$

If hot reloaded:

```text
new resource published
old resource retired
old object destroyed only after GPU completion.
```

This is the frames-in-flight rule from Pass 13.

---

**Stage 18 — diagnostics**

Along the entire path:

```text
Logger
profiling
validation
tests
crash metadata.
```

observe the system.

Diagnostics should **observe** engine state.

They should not become the owner of core semantics.

---

**Stage 19 — CI proves the complete graph**

The build artifact did not emerge from nowhere.

CI reconstructs:

```text
workspace.lock
        ↓
exact repositories
        ↓
preset
        ↓
toolchain
        ↓
CMake target graph
        ↓
Ninja DAG
        ↓
build
        ↓
unit tests
        ↓
integration tests
        ↓
smoke tests
        ↓
package.
```

That chain is the **build-time equivalent** of your runtime ownership chain.

---

**The two master DAGs**

At this point you should mentally separate two large graphs.

### Build DAG

$$
SourceRepos
\rightarrow
CMakeTargets
\rightarrow
GeneratedArtifacts
\rightarrow
Executables.
$$

### Runtime DAG

$$
ProjectData
\rightarrow
RuntimeServices
\rightarrow
SubsystemResources
\rightarrow
FrameWork.
$$

Neither should contain hidden cycles.

And both should have explicit ownership.

---

**The full Kairo architecture in one view**

```text
                    DEVELOPMENT / AUTHORING

 Source repositories
       |
       | exact SHAs
       v
 workspace.lock
       |
       v
 CMake target graph
       |
       v
 Ninja build graph
       |
       +--------------------+
       |                    |
       v                    v
    KairoEditor          KairoPlayer
       |                    ^
       |                    |
       v                    |
 Project + source assets    |
       |                    |
       v                    |
 Asset Registry             |
       |                    |
       v                    |
 Importers / DDC            |
       |                    |
       v                    |
 Derived artifacts          |
       |                    |
       v                    |
 Scene authoring            |
       |                    |
       v                    |
 Reflection inspector       |
       |                    |
       v                    |
 Undo / recovery            |
       |                    |
       v                    |
 Persistent project files   |
       |                    |
       +-------> Packaging--+
                            |
                            v

                         RUNTIME

                     ProjectDescriptor
                            |
                            v
                       AssetManifest
                            |
                            v
                         Scene
                            |
                            v
                      ECS extraction
                    /       |        \
                   v        v         v
               Physics    Logic     Render
                   |        |         |
                   +--------+---------+
                            |
                            v
                        Frame state
                            |
                            v
                       RenderScene
                            |
                            v
                       RenderGraph
                            |
                            v
                      Backend commands
                            |
                            v
                           GPU
```

That is Phase 16.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/11 - Streaming resources through explicit states|Previous chapter]] · [[Chapter Edition/03 - Game Engine/13 - Portable startup, windows, events, and input|Next chapter]]
