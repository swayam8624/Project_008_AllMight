# 10 — Engine changes across subsystem contracts

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/09 - Player choices, rules, and readable feedback|Previous chapter]] · [[Chapter Edition/03 - Game Engine/11 - Streaming resources through explicit states|Next chapter]]

The first playable exposes repeated needs, and those needs become engine boundaries. This chapter follows changes through runtime architecture, assets, tools, and subsystem ownership without treating a feature list as proof of a coherent system.

**By the end:** Explain how an engine change crosses dependencies and preserve stable contracts among game code, tools, runtime services, and backends.

**Recall question:** If a feature changes its asset format or runtime service, which identities, callers, caches, and tests must change together?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 11690–15914; source-span SHA256 15f54478436af6d322d8e47e3a732298e806f2986d4f717b882dd195bd6c541b. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Make a change cross the engine without breaking its promises

Repository-specific passages below describe the supplied Kairo snapshot. They have not been revalidated against a current checkout; code fragments retain their stated surrounding context and are not all standalone tested programs. Numerical and contract explanations are taught independently of those implementation observations.

### Engine Architecture, Dependency Direction, Runtime Boundaries, Platform/Backend Abstraction, Asset Identity, Import Pipelines, Dependency Graphs, DDC, Hot Reload, Streaming, Residency, and GPU-Safe Lifetime


The transition from the previous phase is important. ECS answered:

$$
\text{How should runtime world data be represented?}
$$

Phase 16 begins with a larger question:

$$
\boxed{
\text{How do all engine subsystems depend on each other without eventually becoming one inseparable program?}
}
$$

Then M092 asks the same question of content:

$$
\boxed{
\text{How does a file created by an artist become an immutable runtime resource,
while remaining reproducible, reloadable, cacheable and safe for the GPU?}
}
$$

Kairo is already unusually useful for teaching this phase because its repository boundaries actually embody many of these ideas: `KairoMath`, `KairoGeometry`, `KairoSpatial`, `KairoPhysicsEngine`, `KairoAssets`, `KairoECS`, `KairoReflection`, `KairoEngineCore`, `KairoRenderer`, `KairoEditor`, and the `KairoGameEngine` integration repository are separate pieces rather than directories pretending to be modular.

---

### ENGINE LAYERING, DEPENDENCY MANAGEMENT, PLATFORM BOUNDARIES, SERVICES, AND APPLICATION LIFETIME

**Game-engine architecture as dependency management**

A game engine is not fundamentally defined by having:

```text
renderer
physics
audio
ECS
assets
editor
```

Most sufficiently large applications can accumulate those systems.

The architecture is defined by **who is allowed to know about whom**.

Suppose:

```text
Renderer imports Scene
Scene imports Physics
Physics imports Renderer
```

You no longer have three systems.

You have one three-headed system whose pieces cannot evolve independently.

Architecture therefore begins with a directed graph:

$$
G=(V,E)
$$

where a node represents a module/library/subsystem and:

$$
A\rightarrow B
$$

means:

> A depends on B.

A healthy engine tries to make this graph mostly acyclic and directed toward increasingly fundamental abstractions.

For example:

```text
Application / Player / Editor
              |
              v
        integration bridges
        /      |       \
       v       v        v
 Renderer   Physics    ECS
       \       |        /
        \      v       /
        Foundation / Assets
              |
              v
             Math
```

This is not about drawing pretty boxes.

The direction determines which changes propagate.

If Math imports Renderer, every low-level mathematical utility becomes contaminated with a high-level runtime dependency.

If Renderer imports Math, that is natural:

$$
Renderer\rightarrow Math.
$$

The dependency arrow should generally point from policy-rich/high-level code toward simpler lower-level capabilities.

---

**Architecture has three different dependency graphs**

This distinction is important.

There is a **source dependency graph**:

```cpp
import Kairo.Foundation.Math;
```

There is a **link dependency graph**:

```cmake
target_link_libraries(
    KairoRenderer
    PUBLIC KairoMath)
```

And there is a **runtime dependency graph**:

```text
RendererRuntime instance owns/uses GraphicsDevice instance.
```

Those graphs often resemble one another but are not identical.

For example an interface target can create a build dependency without owning a runtime object.

A runtime service can be injected through an abstract interface while the source module containing the consumer never imports the concrete implementation.

That separation becomes the basis of dependency inversion.

---

**Current Kairo dependency structure**

Your umbrella `KairoGameEngine/CMakeLists.txt` explicitly resolves sibling repositories and adds foundational pieces before integration/runtime pieces.

The workspace contains separate targets for roughly:

```text
KairoMath
KairoGeometry
KairoSpatial
KairoPhysicsMath
KairoPhysicsEngine

KairoAssets
KairoAI
KairoECS
KairoReflection
KairoEngineCore
KairoRenderer

KairoEditor
KairoPlayer
bridges
samples
```

Then the integration target:

```cpp
KairoGameEngine
```

is an `INTERFACE` aggregation target joining the runtime-facing pieces.

That is a reasonable umbrella boundary because the umbrella sits at the **top**, not the bottom.

You do not want:

```text
KairoMath → KairoGameEngine.
```

You want:

```text
KairoGameEngine → KairoMath
```

directly or transitively.

The top is allowed to know the whole world.

The foundation should not.

---

**Foundation versus runtime responsibilities**

A foundation layer should answer questions like:

$$
\text{How do I represent a vector?}
$$

$$
\text{How do I intersect a ray and AABB?}
$$

$$
\text{How do I hash a stable asset ID?}
$$

It should not answer:

$$
\text{Which scene is currently loaded?}
$$

$$
\text{Which renderer is active?}
$$

$$
\text{Which entity is the player?}
$$

Those are runtime/application concerns.

The distinction can be expressed as:

$$
\boxed{
Foundation = reusable mechanism
}
$$

while:

$$
\boxed{
Runtime = policy + live world orchestration.
}
$$

---

**Mechanism versus policy**

Consider a BVH.

`KairoSpatial::BVH` knows:

```text
bounds
nodes
primitive IDs
ray traversal
```

It should not know:

```text
which collider belongs to the player
which renderer pass is active
which scene object is selected in the editor
```

Likewise PhysicsWorld can know:

```text
BodyID
ColliderID
forces
contacts
```

without knowing:

```text
EditorSelectionManager
```

or:

```text
InspectorPanel.
```

The more foundation code knows about application policy, the less reusable it becomes.

---

**The dependency rule**

A useful mental invariant is:

$$
\boxed{
\text{Lower layers must not import application policy from higher layers.}
}
$$

Suppose hierarchy:

```text
Layer 4: Game / Editor
Layer 3: Engine runtime
Layer 2: Subsystems
Layer 1: Foundation
```

Acceptable:

$$
L_4\rightarrow L_3\rightarrow L_2\rightarrow L_1.
$$

Suspicious:

$$
L_1\rightarrow L_4.
$$

Sometimes a lower-level system needs to invoke high-level behavior.

That does **not** mean reversing the dependency.

It means dependency inversion.

---

**Dependency inversion**

Suppose Physics needs to report a collision to gameplay.

The naive design:

```cpp
import Kairo.Gameplay.Player;

void PhysicsWorld::ResolveCollision(...)
{
    Player::Get().OnCollision(...);
}
```

Now Physics depends on gameplay.

Wrong direction.

Instead Physics defines a boundary it understands:

```cpp
struct ContactEvent
{
    BodyID A;
    BodyID B;
};
```

or:

```cpp
class ContactListener
{
public:
    virtual void OnContact(
        const ContactEvent&) = 0;
};
```

Gameplay supplies the implementation.

Source dependency remains:

$$
Gameplay\rightarrow Physics
$$

even though runtime control can flow:

$$
Physics\rightarrow GameplayCallback.
$$

This distinction is central:

$$
\boxed{
\text{control flow can point opposite to source dependency}.
}
$$

---

**Dependency inversion mathematically**

Suppose high-level module $H$ requires service $S$.

Without inversion:

$$
H\rightarrow ConcreteS.
$$

With an interface $I_S$:

$$
H\rightarrow I_S
$$

and:

$$
ConcreteS\rightarrow I_S.
$$

At composition:

```text
H
|
| uses
v
IService
^
|
ConcreteService
```

The concrete implementation depends on the contract rather than forcing the high-level module to know implementation details.

---

**Current Kairo example: `Application`**

Your current `Kairo.EngineCore.Application` imports:

```text
Scene
Time
Event
Input
Layer
```

but does **not** import Renderer or Physics.

Instead it owns:

```cpp
std::vector<std::unique_ptr<Layer>>
```

and layers implement:

```cpp
OnAttach()
OnDetach()
OnUpdate()
OnEvent()
```

The source comment explicitly says renderer and physics can be injected as concrete layers.

That is exactly the right dependency direction:

```text
EngineCore Application
        |
        v
     Layer API
        ^
        |
Renderer/Physics integration layer
```

EngineCore stays renderer-neutral.

---

**Current Kairo example: graphics backend policy**

`ProjectDescriptor` stores:

```cpp
std::string GraphicsBackend = "auto";
```

and explicitly does **not** import `KairoRenderer`.

Later the Player translates:

```text
"vulkan"
"metal"
"d3d12"
"opengl"
```

into renderer-specific backend selection.

This is a strong architectural choice.

The persistent project layer understands a portable **policy name**.

The rendering module understands:

```cpp
GraphicsBackend
```

and concrete API state.

Thus:

$$
EngineCore\nrightarrow KairoRenderer
$$

just because a project wants to say “use Metal.”

---

**Avoiding cyclic module dependencies**

Suppose:

```text
A imports B
B imports C
C imports A
```

This forms:

$$
A\rightarrow B\rightarrow C\rightarrow A.
$$

Conceptually, none of the three can exist independently.

With ordinary headers this produces painful include cycles and forward declarations.

With C++ named modules it becomes even more explicit: the module import graph must be acyclic in a valid build.

You cannot solve architectural cycles by rearranging `import` lines.

You must fix the ownership model.

---

**How cycles arise**

A common example:

```text
Scene needs Renderer types.
Renderer needs Scene objects.
```

Naive:

```text
Scene → Renderer
Renderer → Scene
```

The correct question is:

> What is the actual contract between them?

Usually:

```text
Scene
    ↓
render extraction bridge
    ↓
RenderScene / MeshDraw
    ↓
Renderer
```

Then:

```text
Scene → common contracts
Renderer → common contracts
Bridge → Scene + Renderer.
```

The bridge lives **above both**.

This is an extremely reusable cycle-breaking technique.

---

**Bridge modules**

Suppose two independent domains are:

```text
Physics
Renderer
```

and you need physics debug drawing.

Do not make:

```text
Physics → Renderer.
```

Expose:

```cpp
PhysicsDebugShape
```

from Physics.

Then:

```text
Physics
   |
   v
PhysicsDebugShape
   ^
   |
PhysicsRendererBridge
   |
   v
Renderer
```

This is exactly the architectural style your current repositories already lean toward.

`KairoSceneECSBridge` is another example.

It imports both EngineCore and KairoECS because **it is the integration boundary**.

Neither foundational side needs to import the other unnecessarily.

---

**Dependency cycles are usually ownership ambiguity**

If A and B “need each other,” one of four things is usually true:

$$
\text{shared contract belongs below both}
$$

or:

$$
\text{integration belongs above both}
$$

or:

$$
\text{one module owns too many responsibilities}
$$

or:

$$
\text{a callback/event/interface can reverse control flow}.
$$

Treat a module cycle as an architectural diagnostic, not merely a build problem.

---

**Static libraries versus shared libraries**

A static library is linked into the consuming binary.

Conceptually:

```text
libKairoMath.a
        +
libKairoAssets.a
        +
main.o
        ↓
     KairoPlayer
```

A shared library remains a separately loaded image:

```text
KairoPlayer
    |
    +-- KairoRenderer.dll / .so / .dylib
```

The distinction has consequences beyond file extension.

---

**Static-library advantages**

Static linking generally provides a simpler deployment model.

There is no runtime search for a separate library.

ABI boundaries are less troublesome because all code is normally compiled and linked together.

Link-time optimization can sometimes optimize across targets.

Version mismatches become build-time problems rather than runtime loader problems.

For an engine you completely control, static libraries are often the simplest default.

Your `add_library(...)` targets, when `BUILD_SHARED_LIBS` is not enabled, naturally behave this way in typical CMake configuration.

---

**Shared-library advantages**

Shared libraries are useful when you require:

```text
runtime plugin loading
independent updating
large code shared between several processes
binary extension APIs
```

But they create a real ABI boundary.

A function signature like:

```cpp
std::vector<std::string>
Plugin::GetNames();
```

crossing independently compiled C++ shared libraries can become problematic if compiler/STL/runtime versions differ.

---

**API versus ABI**

API is source-level contract:

```cpp
MeshHandle CreateMesh(...);
```

ABI includes:

```text
name mangling
calling convention
object layout
vtable layout
exception ABI
allocator ownership
STL layout
alignment
```

Two libraries can expose the same-looking API while being ABI incompatible.

This is why a truly durable external plugin API often prefers:

```cpp
extern "C"
```

with opaque handles and fixed-width types.

---

**Example robust plugin boundary**

```cpp
extern "C"
{
    struct KairoPluginHostV1;

    struct KairoPluginV1
    {
        std::uint32_t StructSize;
        std::uint32_t Version;

        void (*OnLoad)(
            KairoPluginHostV1* host);

        void (*OnUnload)();
    };

    using QueryPluginFn =
        bool (*)(
            std::uint32_t requestedABI,
            KairoPluginV1* outPlugin);
}
```

Why this is stronger than exporting a C++ class hierarchy across arbitrary DLLs:

$$
\boxed{
\text{explicit version}
+
\text{fixed representation}
+
\text{opaque ownership}
+
\text{minimal compiler ABI assumptions}.
}
$$

Inside one pinned engine toolchain, C++ interfaces can still be reasonable.

But know the trade.

---

**Header-only versus compiled libraries**

Header-only libraries put implementation in headers/templates.

Advantages include easy distribution and powerful compile-time generic code.

Costs include:

```text
longer compile times
more recompilation after implementation changes
implementation exposure
possible code bloat
```

Compiled libraries hide implementation behind translation/module boundaries.

---

**Templates naturally belong near interfaces**

For:

```cpp
template<typename T>
Vec<T,3>
```

the compiler usually needs the definition where it instantiates the template.

Likewise compile-time algorithms/concepts.

But a 4000-line Vulkan renderer does not belong in an inline header merely because “header-only is simpler.”

Mechanism should determine form.

---

**C++ modules change source organization, not architectural truth**

Your repositories use C++23 named modules such as:

```cpp
export module Kairo.EngineCore.Application;
```

A module lets you explicitly export an interface and hide non-exported implementation details.

It also changes dependency scanning and compilation units.

But:

$$
\boxed{
\text{a C++ module is not automatically an architectural layer}.
}
$$

You can still create terrible dependencies with modules.

Likewise:

$$
\boxed{
\text{module} \neq \text{shared library}.
}
$$

The source module system and binary library model solve different problems.

---

**CMake and module graphs**

Your integration target uses:

```cmake
target_sources(
    KairoSceneECSBridge
    PUBLIC
    FILE_SET CXX_MODULES
    ...
)
```

CMake's module scanning needs to discover import dependencies so BMI/module interfaces are compiled before consumers.

This makes clean dependency direction even more valuable.

A giant tangled import graph becomes expensive and fragile regardless of language-level cycle rules.

---

**Stable runtime interfaces**

A stable runtime interface should describe **capability** without exposing backend internals.

For renderer:

Bad:

```cpp
VkDevice GetDevice();
VkImage CreateTexture(...);
```

at the EngineCore boundary.

That forces EngineCore to know Vulkan.

Better high-level runtime contract:

```cpp
TextureHandle CreateTexture(
    const TextureArtifact& artifact);

void SubmitRenderScene(
    const RenderScene& scene);
```

The backend can be Vulkan, Metal, D3D12 or OpenGL.

---

**Stability does not mean frozen forever**

A good interface has:

```text
clear ownership
clear invalidation
clear failure behavior
clear threading rules
clear semantic meaning
```

That matters more than never changing the signature.

For internal engines, source compatibility can be broken intentionally.

The goal is to prevent implementation details from leaking into unrelated modules.

---

**Stable handle interfaces**

Your renderer's:

```cpp
using TextureHandle = std::uint64_t;
```

is a good example of hiding backend identity.

Engine-side code can pass a:

```text
TextureHandle
```

without knowing whether it corresponds internally to:

```text
VkImage
MTLTexture*
ID3D12Resource*
GLuint
```

Persistent asset identity remains a separate `AssetID`.

That separation is exactly right.

---

**Backend-specific boundaries**

A renderer should conceptually look like:

```text
                    Renderer front-end
                          |
          +---------------+---------------+
          |               |               |
          v               v               v
      Vulkan          Metal            D3D12
```

High-level systems should not repeatedly say:

```cpp
if (backend == Vulkan)
...
else if (backend == Metal)
...
```

throughout game logic.

Backend branching should be concentrated around backend ownership.

---

**What belongs above the backend**

Examples:

```text
RenderScene
MeshDraw
Camera data
Material description
Texture artifact
render graph intent
shader semantic binding descriptions
```

These represent renderer policy.

Below backend boundary:

```text
VkImage
VkDescriptorSet
MTLRenderPipelineState
ID3D12DescriptorHeap
OpenGL VAO
```

These are implementation mechanisms.

---

**Backend capabilities**

Do not force every backend into an artificially identical lowest common denominator.

Instead expose capability queries.

Conceptually:

```cpp
struct GraphicsCapabilities
{
    bool RayTracing;
    bool MeshShaders;
    bool BindlessTextures;

    std::uint32_t MaxTextureDimension;
};
```

Then high-level renderer policy can select an appropriate strategy.

Abstraction should hide API mechanics while still exposing meaningful capability differences.

---

**Platform abstraction layer**

Platform differences exist in:

```text
window creation
filesystem semantics
dynamic libraries
threads
timers
input
process spawning
memory mapping
file watching
sockets
```

The mistake is allowing:

```cpp
#ifdef _WIN32
```

and:

```cpp
#ifdef __APPLE__
```

to spread everywhere.

Put those decisions behind concentrated platform services.

---

**Platform layer versus backend layer**

These are related but not identical.

Platform:

```text
Windows
macOS
Linux
```

Graphics backend:

```text
Vulkan
Metal
D3D12
OpenGL
```

For example Vulkan can run on Linux/Windows and via translation on macOS.

Metal is macOS/iOS-specific.

Do not bake:

```text
if Windows => D3D12
```

into foundational architecture.

Treat backend selection as policy based on platform and capabilities.

Your `ProjectDescriptor` does this well by allowing:

```text
auto
vulkan
metal
d3d12
opengl.
```

---

**Service locators**

A service locator often looks like:

```cpp
Renderer& renderer =
    Services::Get<Renderer>();
```

Convenient.

But the function signature:

```cpp
void UpdatePlayer(Player& player);
```

now hides the fact that it may access:

```text
renderer
audio
physics
assets
networking.
```

Dependency visibility has disappeared.

---

**Why hidden service lookup hurts tests**

Suppose unit test calls:

```cpp
UpdatePlayer(player);
```

and unexpectedly receives:

```text
Services::Get<Audio>()
```

error because Audio was never globally registered.

The function's required environment is not encoded in its interface.

You must reconstruct dependencies from implementation.

That increases cognitive coupling.

---

**Service locator is not always forbidden**

There are legitimate cases:

```text
debug console
profiling
logging
legacy integration
optional tooling
```

where globally discoverable services are convenient.

The problem is making it the default dependency mechanism.

Use it knowingly.

---

**Dependency injection**

Dependency injection means required services are supplied rather than discovered.

Constructor injection:

```cpp
class CharacterSystem
{
public:
    CharacterSystem(
        PhysicsWorld& physics,
        AssetRuntime& assets)
        : m_Physics(physics),
          m_Assets(assets)
    {}

private:
    PhysicsWorld& m_Physics;
    AssetRuntime& m_Assets;
};
```

Now dependencies are explicit.

Testing can provide alternatives.

---

**Injection does not require a DI framework**

In C++, dependency injection can simply mean:

```cpp
Renderer renderer(...);
PhysicsWorld physics(...);

Gameplay gameplay{
    physics,
    renderer
};
```

That is often better than a giant reflection-driven DI container.

The top-level wiring code is called a **composition root**.

---

**Your current KairoPlayer `main` is a composition root**

It constructs roughly:

```text
RuntimeProject
PhysicsBridge
AudioBridge
SceneAudioBridge
InputBridge
CharacterMotorBridge
PlayerControllerBridge
LogicBridge
NativeGameplayBridge
ProductionSystemsBridge
ShippingBridge
RendererRuntime
RenderBridge
```

and passes references between them.

That explicit wiring is good.

You can actually read:

> what owns what, and what depends on what.

As this list grows, the solution is not necessarily a service locator.

It may be a dedicated bootstrap/composition object with explicit construction phases.

---

**Global state and hidden coupling**

Global:

```cpp
Renderer* g_Renderer;
```

looks easy.

But any code anywhere can mutate it.

Testing several worlds simultaneously becomes painful.

Initialization order becomes implicit.

Shutdown becomes risky.

A second renderer instance becomes awkward.

---

**Static initialization order**

Imagine:

```cpp
GlobalA a;
GlobalB b;
```

across translation units.

If A's constructor uses B before B is initialized, behavior becomes problematic.

This is one reason engines should avoid complex runtime services as static globals.

Construct them deliberately from `main()` or equivalent bootstrap.

---

**Hidden coupling can be temporal**

Even if two systems do not directly import one another, they may depend on order:

```text
SetGlobalProject()
then
InitializeRenderer()
```

If that order lives only in convention, architecture is fragile.

Explicit object dependencies make temporal requirements visible.

---

**Engine contexts and service bundles**

Passing twelve parameters everywhere is also unpleasant.

A context can group related services:

```cpp
struct RuntimeServices
{
    AssetRuntime& Assets;
    PhysicsWorld& Physics;
    JobSystem& Jobs;
    EventQueue& Events;
};
```

Then:

```cpp
GameplaySystem(RuntimeServices services);
```

This is still explicit enough to inspect.

---

**The god-context trap**

Do not create:

```cpp
struct EngineContext
{
    Renderer* Renderer;
    PhysicsWorld* Physics;
    Audio* Audio;
    Editor* Editor;
    Network* Network;
    AssetRegistry* Assets;
    ...
    70 services...
};
```

and pass it everywhere.

That is a service locator wearing a struct.

Prefer narrow contexts:

```text
SimulationContext
RenderExtractionContext
AssetImportContext
EditorContext.
```

A system receives the smallest capability bundle it logically needs.

---

**Capability views**

A stronger pattern:

```cpp
struct CharacterSystemContext
{
    PhysicsQueryAPI& Physics;
    CommandBuffer& Commands;
};
```

Character logic cannot accidentally access renderer or editor because those services are absent.

Architecture becomes enforceable by types.

---

**Application lifecycle**

An engine application has phases.

Conceptually:

```text
Process start
    ↓
Parse command line/configuration
    ↓
Create platform services
    ↓
Load project
    ↓
Initialize assets
    ↓
Initialize runtime subsystems
    ↓
Load startup world
    ↓
BeginPlay
    ↓
Frame loop
    ↓
EndPlay
    ↓
Drain asynchronous work
    ↓
Destroy runtime resources
    ↓
Destroy platform
    ↓
Process exit
```

Each stage should have explicit ownership.

---

**Lifecycle is not merely `main()`**

A robust application must define:

```text
what happens if initialization fails halfway
what happens if shutdown starts while jobs are running
what happens if renderer construction fails
what services may be used during teardown
```

RAII helps enormously.

If objects are constructed in dependency order as local members, C++ destroys them in reverse order.

But custom callbacks/events may still require explicit shutdown phases.

---

**Kairo `Application` lifecycle**

Current `Application::PushLayer()` does:

```cpp
layer->OnAttach();
m_Layers.push_back(...);
```

Then destructor invokes:

```text
OnDetach
```

in reverse layer order.

The reverse order is good:

$$
L_1,L_2,L_3
$$

attach order becomes:

$$
L_3,L_2,L_1
$$

detach order.

That mirrors stack-like dependency teardown.

---

**An important current lifecycle caveat**

`Application::~Application()` calls user-overridable:

```cpp
OnDetach()
```

but `OnDetach()` is not declared `noexcept`.

A destructor must not allow an exception to escape during stack unwinding.

If `OnDetach()` throws while another exception exists:

$$
std::terminate.
$$

Production options include declaring teardown callbacks `noexcept`, catching/logging inside the destructor, or exposing explicit `Shutdown()` that reports errors before the nonthrowing destructor performs final cleanup.

This is a real interface-level lifecycle decision.

---

**Engine initialization order**

Initialization should follow the dependency DAG.

If:

$$
A\rightarrow B,
$$

B must exist before A can initialize.

Example:

```text
Project descriptor
      ↓
Asset registry
      ↓
Scene
      ↓
Physics bridge
```

and:

```text
Platform window
      ↓
Graphics device
      ↓
Renderer runtime
      ↓
Render bridge.
```

Do not encode this in arbitrary singleton constructors.

Let the composition root build it explicitly.

---

**Topological initialization**

We can think of services exactly like the system DAG from Pass 11.

For every service S:

$$
Dependencies(S)=\{D_1,D_2,\dots\}.
$$

Initialize a service only after all its dependencies.

That is a topological ordering problem.

Usually the graph is small enough to express manually through constructor order.

But the mental model is the same.

---

**Two-phase initialization**

Some systems cannot finish construction until several objects mutually exist.

Avoid creating actual source cycles just to solve this.

Use:

```text
Construct inert object
    ↓
wire dependencies
    ↓
Initialize()
```

carefully.

But remember two-phase initialization creates states:

```text
constructed but unusable
```

which must be guarded.

Constructor injection is preferable where possible.

---

**Shutdown order**

Shutdown should usually reverse initialization dependencies.

If renderer depends on GraphicsDevice:

$$
Renderer\rightarrow Device.
$$

Destroy:

```text
Renderer
then Device.
```

Never:

```text
Device
then Renderer
```

because renderer destructor may attempt GPU cleanup on a destroyed device.

---

**Asynchronous shutdown complicates the picture**

Before destroying resources:

```text
stop accepting new work
wait/cancel worker jobs
flush asynchronous I/O
drain command buffers
wait for required GPU submissions
release runtime bridges
release resources
destroy device
destroy window/platform.
```

Otherwise a background thread can wake after its dependency has vanished.

---

**GPU shutdown**

Especially:

```text
vkDestroyImage
```

must not occur while GPU still references the image.

At complete shutdown it is acceptable to wait for the device/queues to become idle before destroying all GPU resources.

During ordinary runtime hot reload, waiting idle globally is too expensive; we'll design deferred destruction in M092.

---

**Runtime versus editor concerns**

The editor and game player solve different problems.

Editor needs:

```text
reflection
inspectors
asset importers
source watchers
undo/redo
debug visualization
scene mutation
profilers
migration tools.
```

Player needs:

```text
load cooked content
simulate
render
audio
input
gameplay.
```

Those should not become one executable with every feature merely disabled via `if (!editor)`.

---

**Separate processes are valuable**

If editor runs game inside the same process, play-mode crashes can crash the editor.

Global/static state can leak between edit and play worlds.

GPU/runtime state can become tangled.

A separate player process provides:

$$
\boxed{
\text{fault isolation}
+
\text{clean runtime semantics}
+
\text{shipping parity}.
}
$$

It also naturally tests that the game does not accidentally rely on editor-only services.

---

**Kairo already has this separation**

Your CMake has distinct options:

```text
BUILD_EDITOR
BUILD_PLAYER
```

and an actual:

```text
KairoPlayer
```

executable.

`ProjectDescriptor` can optionally name a project-relative Play executable.

This is much closer to a production engine architecture than simply embedding game mode in the editor.

---

**Authoring versus shipped runtime**

Consider a `.blend`, `.gltf`, `.png`, or shader source.

The editor needs:

```text
source files
import settings
importers
watchers
metadata
possibly DCC integration.
```

The shipped game ideally needs:

```text
cooked artifacts
runtime manifest
GPU-ready or near-GPU-ready data.
```

The shipped player should not necessarily contain Blender interoperability, OBJ parsing or Photoshop-image support.

---

**The source/cook/runtime split**

A strong pipeline:

```text
AUTHORING

PNG / glTF / OBJ / shader source
          |
          v
        importer
          |
          v
ENGINE ARTIFACT

texture2d.v1
mesh.v1
scene.v1
shader artifact
          |
          v
RUNTIME ADAPTER

Vulkan texture
Metal texture
GPU vertex buffers
pipeline state
```

Those are three different data layers.

Do not collapse them.

---

**Why cook offline**

If all source conversion happens before shipping:

$$
T_{\text{startup}}
$$

does not include expensive decoding/mesh processing/shader compilation.

You can validate content in CI.

You can make package contents deterministic.

You can remove import dependencies from shipping binaries.

You can precompute backend-specific formats.

---

**Standalone player architecture**

A standalone player is the composition root for runtime.

It should:

```text
load project/package metadata
load runtime manifest
resolve startup scene
construct runtime services
load world
begin play
run fixed/variable systems
extract rendering/audio
present
shutdown cleanly.
```

No editor window should be required.

---

**Current `KairoPlayer`**

Your player already creates runtime-specific bridges explicitly.

It supports:

```text
--renderer
--validate
--smoke
--package
```

It loads the project first, reports project/asset/entity information, constructs physics/audio/input/gameplay services, and only creates `RendererRuntime` after validation/package-only paths have been handled.

This is exactly the correct place for top-level policy.

---

**Composition-root growth**

Your current `main.cpp` now wires a considerable number of bridges.

That is a normal architectural milestone.

Eventually consider extracting:

```cpp
class RuntimeBootstrap
```

or:

```cpp
RuntimeServices BuildRuntime(
    RuntimeProject&);
```

not because `main` is "too long," but because bootstrap sequencing itself is becoming a coherent subsystem that deserves testing.

Do not hide those dependencies behind globals.

Move explicit wiring into a testable composition object.

---

**Headless validation**

A validation process should be able to answer:

```text
Is the project descriptor valid?
Are asset references valid?
Is the scene structurally valid?
Are source/cooked manifests consistent?
Do required components exist?
Are dependency graphs acyclic?
Can runtime contracts be satisfied?
```

without requiring:

```text
window
swapchain
GPU
audio device.
```

That lets CI run deterministic validation on headless machines.

---

**Current Kairo headless boundary**

`KairoPlayer --validate` exits before:

```cpp
RendererRuntime renderer(...)
```

is created.

That is a strong design.

Validation therefore tests project/runtime contracts without graphics-backend availability contaminating the result.

Your CMake tests already use this path for sample projects.

This is what a headless validation route should look like.

---

**Headless validation versus headless rendering**

These are not the same.

Headless validation:

$$
\text{no rendering required}.
$$

Headless rendering:

$$
\text{rendering occurs without a visible interactive window}.
$$

A server build may need neither.

A render farm may need headless GPU rendering.

A CI project validator should not require either.

Separate those concerns.

---

### ASSET IDENTITY, IMPORT PIPELINES, DEPENDENCY GRAPHS, DDC, HOT RELOAD, STREAMING AND GPU LIFETIME

Now we move from code architecture to content architecture.

The central idea is:

$$
\boxed{
\text{Asset identity is not file path,
loaded-object address,
or GPU-resource handle.}
}
$$

Those have different lifetimes.

---

**Asset identity**

An asset might represent:

```text
mesh
texture
material
scene
shader
audio
script
document.
```

We need a persistent answer to:

> Which asset is this?

The answer should ideally survive:

```text
editor restart
path movement
renaming
asset database reordering.
```

Kairo uses:

```cpp
AssetID
```

which is a persistent 128-bit UUID-like value.

That is a good durable identity.

---

**The four identity layers**

For one texture:

```text
Persistent AssetID
    |
    +---- metadata path
    |
    +---- derived cache key
    |
    +---- runtime TextureHandle
```

These are four different things.

For example:

```text
AssetID
f03b...c821
```

may remain stable when:

```text
Textures/Wall.png
```

becomes:

```text
Environment/Architecture/Wall.png.
```

The content hash changes if pixels change.

Runtime TextureHandle may change every process execution.

That is exactly what we want.

---

**Path-based asset identity**

Simplest asset identity:

```text
Textures/Wall.png.
```

Advantages are obvious: human readable and no separate ID database required.

But renaming:

```text
Textures/Wall.png
    ↓
Textures/StoneWall.png
```

changes identity.

Every referencing scene/material must be updated.

---

**Path normalization is harder than it looks**

Paths differ by:

```text
separator
case sensitivity
"." and ".."
drive roots
UNC forms
reserved names
Unicode representation
symlinks.
```

Kairo's `NormalizeAssetPath()` explicitly rejects absolute paths, traversal, unsafe characters and Windows device names, and creates a case-folded portable lookup key.

This is good defensive metadata handling.

But importantly:

$$
\boxed{
Kairo uses path as lookup metadata,
not durable identity.
}
$$

That is the right distinction.

---

**UUID asset identity**

Assign:

$$
ID=
128\text{-bit UUID}.
$$

References become:

```text
Material:
BaseColorTexture =
f03b9c...
```

Path becomes mutable metadata.

Move asset:

```text
Textures/A.png
        ↓
Characters/Hero/A.png
```

ID remains unchanged.

No scene references need rewriting.

---

**UUID collision probability**

For random UUID-like 122 effective random bits, birthday collision probability after $n$ IDs is approximately:

$$
p\approx
\frac{n^2}{2\cdot2^{122}}.
$$

Even at:

$$
n=10^9,
$$

the probability remains astronomically tiny.

But Kairo correctly does not treat randomness as a mathematical proof of uniqueness: registry creation can still reject/retry collisions.

That is sound engineering.

---

**Content-addressed identity**

Content addressing defines:

$$
ID=H(content).
$$

If content changes:

$$
H(old)\neq H(new)
$$

with overwhelming probability.

This is excellent for immutable blobs and caches.

Example:

```text
SHA-256(asset bytes) → cache object.
```

But it is often bad as the **authoring identity**.

Edit the texture by one pixel:

$$
AssetID
$$

would change.

Every scene/material reference would conceptually point to the old version.

---

**Hybrid identity is usually superior**

Kairo effectively uses a strong hybrid:

$$
\boxed{
UUID
\rightarrow
\text{persistent authored identity}
}
$$

$$
\boxed{
path
\rightarrow
\text{human/project location}
}
$$

$$
\boxed{
content hash
\rightarrow
\text{derived cache identity}.
}
$$

This is exactly how these three identity strategies naturally fit together.

Do not choose one mechanism and force it to solve all three problems.

---

**Asset handles**

A handle should represent an asset without embedding loaded runtime storage.

Kairo:

```cpp
template<AssetType ExpectedType>
struct AssetHandle
{
    AssetID ID;
};
```

Then:

```cpp
MeshAssetHandle
TextureAssetHandle
SceneAssetHandle
```

are type-safe persistent references.

Excellent.

---

**Asset handle does not imply residency**

This distinction is essential.

Suppose:

```cpp
TextureAssetHandle h;
```

exists.

That does not necessarily mean:

```text
texture bytes are in RAM
```

or:

```text
texture exists on GPU.
```

It means:

> this persistent asset is referenced.

So:

$$
\boxed{
Handle validity
\neq
Resource residency.
}
$$

We will build explicit residency state in Parts 1542–1544.

---

**Persistent handle versus runtime handle**

Persistent:

```cpp
TextureAssetHandle
```

might contain a 128-bit `AssetID`.

Renderer runtime handle:

```cpp
TextureHandle
```

may be an integer indexing renderer-owned resources.

Those must stay separate.

Your `Kairo.Renderer.Texture` explicitly documents its `TextureHandle` as a process-local renderer-owned key while persistent identity remains in KairoAssets.

This is correct architecture.

---

**Generational asset handles**

Persistent UUID handles do not need generation counters in the same sense because their identity is not a reused array slot.

But a **runtime resource table** often does.

Suppose:

```text
slot 7 = old texture
destroy
slot 7 = new texture.
```

A stale renderer handle `7` could alias the new texture.

So use:

$$
RuntimeHandle=(index,generation).
$$

Exactly the same reasoning as ECS entities.

---

**Example loaded-resource handle**

```cpp
struct TextureRuntimeHandle
{
    std::uint32_t Index;
    std::uint32_t Generation;
};
```

Resource table:

```text
slot 0 gen 5 resident
slot 1 gen 3 unloaded
slot 2 gen 9 resident
```

Then stale:

$$
(2,8)
$$

cannot access:

$$
(2,9).
$$

Persistent `AssetID` and process-local runtime generation solve different lifetime problems.

---

**Current Kairo runtime handle gap**

Current:

```cpp
using TextureHandle = std::uint64_t;
```

provides process-local abstraction but does not itself encode a documented generation structure.

That is fine if handles are monotonically issued and never reused unsafely.

If a reusable resource slot table is introduced, generational encoding would be advisable.

Do not assume “64-bit integer” itself prevents stale reuse.

---

**Asset registry**

An asset registry answers:

$$
AssetID\rightarrow AssetMetadata.
$$

It also often supports:

$$
Path\rightarrow AssetID.
$$

Your current `AssetRegistry` maintains exactly these mappings.

It also validates dependency existence, types and cycles.

---

**Registry is metadata authority, not loaded-resource manager**

This is important.

Registry should not necessarily own:

```text
VkImage
MTLTexture
audio voice
script VM
```

Its job is persistent metadata.

Current Kairo documentation is explicit that AssetRegistry leaves:

```text
decoding
GPU upload
audio creation
file watching
```

to other layers.

That is excellent separation.

---

**Thread-safe registry snapshots**

Kairo's registry uses a shared mutex and returns **copies** of metadata rather than references that could become invalid across concurrent mutation.

This is a strong API decision.

Instead of:

```cpp
const AssetMetadata&
```

that might outlive a lock, callers receive:

```cpp
AssetMetadata
```

by value.

Ownership is clear.

---

**Asset metadata**

Current metadata contains approximately:

```cpp
AssetID ID;
AssetType Type;
AssetOrigin Origin;
path Path;
string Importer;
uint64_t Revision;
vector<AssetReference> Dependencies;
```

Each field answers a different question.

`ID`: what asset?

`Type`: how should it be interpreted?

`Origin`: source/generated/builtin?

`Path`: where is its project representation?

`Importer`: which import mechanism?

`Revision`: how many metadata mutations?

`Dependencies`: what does it depend on?

---

**Metadata revision is not content version**

Kairo explicitly distinguishes:

$$
Metadata.Revision
$$

from:

$$
SourceFingerprint.
$$

Moving an asset changes metadata revision.

That does not necessarily change source bytes.

Editing source bytes changes source fingerprint even if metadata path is untouched.

This separation is excellent.

A single vague "version" field usually causes confusion.

---

**Source assets versus derived runtime assets**

Source asset:

```text
Wall.png
Character.glb
Rock.obj
shader.hlsl
```

exists for authoring/interchange.

Derived artifact:

```text
kairo.texture2d.v1
kairo.mesh.v1
kairo.gltf-scene.v1
```

exists for the engine.

Runtime GPU resource is a third layer.

Thus:

$$
\boxed{
Source
\rightarrow
DerivedArtifact
\rightarrow
RuntimeResource.
}
$$

---

**Why derived artifacts exist**

Source formats optimize for interchange and authoring.

They can contain:

```text
flexible JSON
text parsing
arbitrary numeric layouts
external references
compression formats
unused metadata.
```

Runtime wants:

```text
validated
canonical
bounded
fast-to-load
engine-specific data.
```

Derived artifacts let the engine own its runtime contract.

---

**Renderer adaptation**

Your renderer's:

```cpp
Mesh::FromArtifact(...)
```

takes KairoAssets mesh artifact data and converts it into renderer-side vertex representation.

This is a clean boundary:

```text
KairoAssets:
canonical engine artifact

KairoRenderer:
runtime rendering representation.
```

The asset importer does not create Vulkan buffers.

Good.

---

**Importer interfaces**

Current Kairo:

```cpp
class AssetImporter
{
public:
    virtual std::string Identifier() const = 0;
    virtual std::string Version() const = 0;

    virtual DerivedArtifact
    Import(const ImportRequest&) const = 0;
};
```

This is an excellent narrow plugin contract.

Importer receives validated request and returns artifact.

It does **not** mutate:

```text
asset registry
import database
DDC.
```

Those side effects belong to orchestration.

---

**Why importer purity matters**

Suppose importer itself did:

```text
parse
register new assets
write cache
modify manifest
upload GPU
notify editor
```

A failure halfway could leave several systems inconsistent.

Instead:

$$
Importer:
Source\rightarrow Artifact
$$

and:

$$
Orchestrator:
validation + cache + provenance + publication.
$$

This is much easier to test and make transactional.

---

**Exact importer versioning**

Your `ImporterRegistry` keys implementations by:

$$
(identifier,version).
$$

It explicitly refuses to silently substitute another version.

That is essential for reproducible builds.

If asset was imported with:

```text
kairo.texture.stb@1
```

and only:

```text
@2
```

exists, automatically using 2 may produce different output.

Reproducibility requires exact provenance.

---

**Import transaction**

Your `ImportSourceAsset()` follows a good sequence:

```text
validate importer/provenance
    ↓
read source bytes
    ↓
fingerprint source
    ↓
compute DDC key
    ↓
cache hit?
   / \
 yes  no
 |     |
load  importer.Import()
 |     |
 |   validate artifact
 |     |
 |   publish cache
  \   /
   \ /
update successful import provenance
```

Import provenance updates only after a usable artifact exists.

Thus failed import does not mark broken content as current.

Excellent transactional ordering.

---

**OBJ import pipeline**

A robust OBJ importer boundary should be:

```text
OBJ bytes
    ↓
syntax parsing
    ↓
indices/positions/normals/UV decoding
    ↓
validation
    ↓
coordinate/attribute normalization if policy requires
    ↓
canonical MeshArtifact
```

It should **not** perform:

```text
GPU upload
scene insertion
renderer material creation
editor UI mutation.
```

Those are separate layers.

Kairo's built-in registry exposes an `OBJMeshImporter`, fitting this model.

---

**OBJ-specific dangers**

OBJ can contain:

```text
negative relative indices
separate position/normal/UV indices
polygons rather than triangles
missing normals
duplicate vertices
huge counts
malformed floating point
```

Importer must convert flexible source semantics into a bounded canonical artifact.

The canonical artifact should not preserve every source-format quirk merely because OBJ allows it.

---

**glTF import boundaries**

glTF is much richer.

A glTF scene may include:

```text
nodes
hierarchy
meshes
materials
textures
skins
animations
buffers
external images.
```

So a glTF importer may output a **scene artifact**, not merely one Mesh.

Current Kairo `GltfSceneImporter` does exactly that class of conversion.

---

**glTF external dependencies**

A `.gltf` file can reference:

```text
buffer.bin
albedo.png
normal.png.
```

That means source is not necessarily one file.

Current Kairo validates local dependency URIs carefully, rejecting escaping paths and symlinked/nonregular dependencies.

This is strong security/portability design.

But it introduces an important cache question we will revisit:

$$
\boxed{
\text{all external files influencing output must influence the cache key}.
}
$$

---

**glTF import should not silently own project policy**

Your current asset pipeline deliberately preserves texture URIs in the glTF scene artifact instead of silently registering new texture assets.

The editor/import transaction owns that expansion.

This is a subtle but very good boundary.

Importer says:

> these source dependencies exist.

Project/editor layer decides:

> what persistent AssetIDs should they receive, how do undo and metadata behave?

That prevents parser code from becoming project-state code.

---

**Texture import pipeline**

A texture import pipeline should resolve:

```text
encoded source
    ↓
decode pixels
    ↓
validate dimensions
    ↓
classify color space
    ↓
normal-map handling
    ↓
mipmap generation
    ↓
runtime pixel format conversion/compression
    ↓
TextureArtifact
```

Your current STB importer handles decoding and produces canonical mip artifacts.

---

**sRGB versus linear**

Color textures generally encode approximately perceptual/sRGB values.

Lighting math requires linear values.

Normal maps are vector data and must be treated as linear.

So importer setting:

```text
normal=true
```

forces linear color space in Kairo.

That is correct.

---

**Mipmap generation**

For ordinary color:

$$
C_{mip}
$$

should be filtered in the correct linear-light domain rather than averaging gamma-encoded values naively.

For normal maps:

$$
n'=
Normalize\left(
\sum_i w_i n_i
\right)
$$

rather than simply averaging encoded RGB vectors and leaving the result unnormalized.

Your asset pipeline documentation specifically states that color mips use linear-light filtering and normal-map mips are renormalized.

That is exactly the sort of operation that belongs during derivation rather than every runtime load.

---

**Shader import/compilation pipeline**

This is currently a **design gap** in the inspected Kairo built-in importer set.

`AssetType::Shader` exists, but `RegisterBuiltinImporters()` currently registers:

```text
passthrough
OBJ
texture
WAV
glTF
```

and no dedicated shader compiler/importer was found.

So let's design it properly.

---

**Shader source is not the final runtime asset**

Source might be:

```text
HLSL
GLSL
MSL
shader DSL.
```

Runtime artifact might contain:

```text
SPIR-V
DXIL
metallib/MSL-derived binary
OpenGL GLSL/source variant
reflection metadata.
```

The compiler toolchain is itself part of asset derivation.

---

**Shader cache recipe**

A correct shader key may include:

$$
K=
H(
SourceClosure,
Stage,
EntryPoint,
Defines,
CompilerID,
CompilerVersion,
Optimization,
TargetBackend,
TargetProfile,
FeatureFlags
).
$$

If any input changes, shader artifact changes.

For example:

```text
source identical
compiler upgraded DXC 1.x → 2.x
```

can produce different binary.

Therefore compiler version belongs in the recipe.

---

**Shader include dependency closure**

Suppose:

```hlsl
#include "lighting.hlsl"
#include "pbr.hlsl"
```

Hashing only root shader file is insufficient.

You need:

$$
H(
RootSource,
IncludeA,
IncludeB,
...
).
$$

Prefer canonical dependency discovery/preprocessing.

The same principle applies to glTF external buffers.

---

**Shader reflection**

Compilation should often produce metadata:

```text
descriptor bindings
uniform buffers
push constants
vertex inputs
specialization constants
thread-group size.
```

Then pipeline creation does not need to reparsed source.

The derived shader artifact can be:

```text
binary code
+
stable reflection schema.
```

---

**Asset dependency graphs**

An asset graph has:

$$
A\rightarrow B
$$

if A depends on B.

Example:

```text
Scene
 ├── CharacterMesh
 │      └── Material
 │             ├── Albedo
 │             └── Normal
 └── EnvironmentTexture
```

Changing Normal may affect Material and every dependent scene/package.

---

**Why dependency topology belongs in metadata**

Without an explicit graph, detecting impact becomes:

> grep the project and hope.

With graph:

$$
Dependents(Texture)
$$

is computable.

This supports:

```text
safe deletion
incremental rebuild
hot reload propagation
package closure
dependency visualization.
```

Your AssetRegistry already stores typed dependency references.

---

**Cycle prevention**

Current AssetRegistry rejects cycles.

Suppose:

$$
A\rightarrow B
$$

and proposed dependency:

$$
B\rightarrow A.
$$

`SetDependencies()` performs reachability checking and refuses it.

That makes the asset graph a DAG.

This simplifies:

$$
topological import/build order.
$$

For asset types where cycles would be semantically meaningful, you would need an SCC-aware design instead, but Kairo chooses the simpler DAG contract.

That is a valid and useful policy.

---

**Reverse dependency graph**

Registry currently stores:

$$
Asset\rightarrow Dependencies.
$$

For incremental rebuild, you frequently need:

$$
Asset\rightarrow Dependents.
$$

You can derive reverse adjacency or maintain it incrementally.

If Texture T changes:

```text
T
↑
M1 M2
↑  ↑
S1 S2
```

reverse edges tell you what can become dirty.

---

**Asset cache keys**

A cache key must represent **all semantic inputs to the derivation**.

Ideal equation:

$$
\boxed{
K=
H(
SourceInputs,
ImporterIdentity,
ImporterVersion,
CanonicalSettings,
Target,
DependencyVersions
).
}
$$

If two derivations have identical keys, they must produce identical artifact bytes.

That is the fundamental cache contract.

---

**Kairo's current DDC key**

Current Kairo calculates roughly:

$$
K=
SHA256(
SourceFingerprint
\Vert
SourceByteCount
\Vert
AssetType
\Vert
ImporterID
\Vert
ImporterVersion
\Vert
CanonicalSettings
).
$$

This is strong.

Changing source bytes changes key.

Changing importer version changes key.

Changing settings changes key.

Different asset type changes key.

---

**Canonical settings matter**

Suppose these mean the same thing:

```text
mips=1;normal=0
```

and:

```text
normal=false;mips=true.
```

If importer allows arbitrary spelling, equivalent settings produce different cache keys.

Kairo's texture importer has:

```cpp
CanonicalTextureImportSettings(...)
```

that emits deterministic ordering/spelling.

This makes:

$$
semantic\ equality
\Rightarrow
byte\ equality
$$

for settings.

Exactly what DDC wants.

---

**A current Kairo glTF cache-invalidation gap**

This one matters.

`ImportSourceAsset()` calculates the DDC key **before** invoking the importer using the fingerprint of the registered source file.

For a self-contained GLB:

$$
SourceFingerprint
$$

covers everything.

Good.

For a `.gltf` that references:

```text
mesh.bin
texture.png
```

the output can depend on those sibling files.

But the current generic DDC recipe shown in `MakeDerivedDataKey()` contains only the primary source fingerprint plus importer/type/settings.

Therefore:

```text
scene.gltf unchanged
mesh.bin changed
```

can theoretically produce:

$$
same\ DDC\ key
$$

but:

$$
different\ correct\ artifact.
$$

That violates the immutable-cache invariant.

---

**Correct fix: dependency closure fingerprint**

Importer architecture needs a discovery phase.

Conceptually:

```cpp
struct SourceDependency
{
    path Path;
    Fingerprint Content;
};

struct ImportPlan
{
    vector<SourceDependency> Inputs;
    CanonicalSettings Settings;
};
```

Then key:

$$
K=
H(
Fingerprint(scene.gltf),
Fingerprint(mesh.bin),
Fingerprint(texture.png),
...
ImporterVersion,
Settings
).
$$

Only after computing the complete source closure do you consult DDC.

This is an important next improvement for KairoAssets.

---

**Alternative: registered dependency assets**

If external glTF textures/buffers are themselves registered AssetIDs, key could incorporate:

$$
(AssetID,DerivedKey)
$$

for each dependency.

Then:

$$
ParentKey=
H(
OwnInputs,
DependencyDerivedKeys
).
$$

This naturally propagates changes through the asset DAG.

It also makes incremental build reasoning much cleaner.

---

**Derived-data caches**

A DDC stores expensive-to-recompute artifacts.

Conceptually:

```text
key = SHA-256 recipe
             |
             v
Cache/
  ab/
    abcd...123.kdd
```

If key exists:

```text
load artifact
```

instead of:

```text
run importer.
```

The cache is disposable.

The source/project metadata remains authoritative.

---

**Cache is not the asset registry**

Registry answers:

> what is this asset?

DDC answers:

> have I already computed this exact derivation?

Do not merge them.

Deleting DDC should not destroy project identity.

It should merely make the next build slower.

---

**Kairo DDC immutability**

Current `DerivedDataCache::Store()` enforces:

> if a key already exists with different bytes, throw.

That is exactly right.

A content/recipe-addressed key should be immutable.

If:

$$
K=X
$$

ever maps to two byte sequences:

$$
B_1\neq B_2,
$$

your derivation function is nondeterministic or your key omitted an input.

Both are serious bugs.

---

**Multi-process cache publication**

Kairo's DDC does something notably careful.

It stages into a uniquely claimed temporary directory, writes the payload, then uses hard-link creation to publish the final immutable target without blindly overwriting another process's result.

This is much stronger than:

```text
write key.tmp
rename over key
```

when multiple build processes may race.

It is treating DDC as a real concurrent content store.

---

**Cache invalidation**

The best content-addressed cache invalidation strategy is often:

$$
\boxed{
\text{do not mutate old cache entries}.
}
$$

Change an input:

$$
I_0\rightarrow I_1.
$$

Then:

$$
K_0=H(I_0),
$$

$$
K_1=H(I_1).
$$

Load $K_1$.

$K_0$ becomes unused garbage that may later be collected.

No fragile:

```text
delete cache because timestamp changed
```

logic is required.

---

**Cache invalidation failures are usually key-design failures**

If changing a source input does not change cache key:

$$
\text{key omitted an input}.
$$

If semantically identical input creates new keys unnecessarily:

$$
\text{canonicalization is insufficient}.
$$

So the real cache invalidation question is:

$$
\boxed{
\text{Does the key uniquely identify the complete semantic recipe?}
}
$$

not:

> How do I delete stale files?

---

**Importer version invalidation**

Suppose importer bug is fixed but source/settings are unchanged.

If:

```text
ImporterVersion = 1
```

remains unchanged, DDC may return old broken artifact.

Therefore importer implementation changes that can change output must bump its version or otherwise change the toolchain fingerprint.

This is why exact importer version is part of Kairo's key.

---

**Target-platform cache dimensions**

Suppose derived artifact is backend-neutral.

Then platform need not enter key.

But if artifact includes:

```text
BC7 texture compression
ASTC texture compression
DXIL
metallib
```

the target matters.

Key must include something like:

```text
platform = windows-x64
gpu-family = desktop
backend = d3d12
format = BC7.
```

Otherwise one platform can accidentally consume another platform's cached output.

---

**Incremental asset builds**

Full rebuild:

```text
import every asset
```

is wasteful.

Incremental build begins from changed source nodes.

Suppose dependency graph:

```text
T -> M -> S
```

where:

```text
T = texture
M = material
S = scene/package.
```

If T changes, mark:

$$
Dirty=\{T,M,S\}.
$$

Then topologically rebuild.

---

**Incremental algorithm**

Conceptually:

```cpp
queue = changedAssets;

while (!queue.empty())
{
    AssetID changed = queue.pop();

    for (AssetID dependent
         : ReverseDependencies(changed))
    {
        if (dirty.insert(dependent).second)
        {
            queue.push(dependent);
        }
    }
}
```

Then topologically process dirty subgraph.

Each node computes its new DDC key.

Cache hit means no expensive importer work.

---

**Build graph versus metadata graph**

Sometimes asset A references B but A's derived bytes do not actually incorporate B.

Example:

```text
Scene stores AssetID of Texture
```

Changing texture pixels should not force scene serialization to rebuild.

The **runtime dependency graph** and **derived-build dependency graph** may differ.

This is another subtle point.

Model dependencies according to what output actually depends on.

Otherwise incremental builds over-invalidate.

---

**Current Kairo source watcher**

`SourceWatcher` provides a portable polling baseline.

It maintains state:

```text
Current
Changed
Missing
```

per imported asset.

`Synchronize()` establishes baseline without emitting fake startup changes.

`Poll()` emits deterministic AssetID-ordered transitions.

This is an excellent portable foundation.

Native file-system notifications can merely trigger earlier polls rather than changing semantics.

---

**Why polling remains valuable even with native notifications**

FSEvents/inotify/ReadDirectoryChangesW can:

```text
coalesce events
drop events
report directory-level changes
differ by platform.
```

Treat them as:

> something may have changed.

Then validate content using fingerprints.

That produces reliable semantics.

---

**Asset hot reloading**

Hot reload is not:

```text
file changed
destroy old object
load new object.
```

The robust state machine is:

```text
source change detected
        ↓
derive new artifact
        ↓
validate artifact
        ↓
create replacement runtime resource
        ↓
replacement fully ready?
      /    \
    no      yes
    |        |
keep old    publish new
             |
             v
         retire old safely
```

Critical invariant:

$$
\boxed{
\text{old known-good resource stays usable until the replacement succeeds.}
}
$$

---

**Never destroy first**

Bad:

```cpp
DestroyTexture(old);
Texture newTexture = Load(newFile);
```

If loading fails:

```text
no texture remains.
```

Good:

```cpp
Texture replacement =
    BuildReplacement();

SwapPublishedHandle(
    old,
    replacement);

RetireOld(old);
```

Failure leaves the previous resource active.

This is transactional hot reload.

---

**Versioned resource publication**

A runtime entry might be:

```cpp
struct RuntimeAssetEntry
{
    AssetID Asset;
    std::uint64_t Revision;

    ResourceState State;

    TextureRuntimeHandle Current;
};
```

On successful reload:

$$
Revision\leftarrow Revision+1.
$$

Consumers that cache derived information can observe revision changes.

---

**CPU hot reload versus GPU hot reload**

Replacing:

```text
CPU-side material metadata
```

can be straightforward.

Replacing:

```text
VkImage
descriptor set
pipeline
vertex buffer
```

must respect asynchronous GPU execution.

That is Part 1541.

---

**Safe GPU replacement**

Suppose frame 100 records:

```text
draw using Texture A.
```

CPU immediately creates Texture B and destroys A.

But GPU may still be executing frame 98–100.

Then it accesses freed:

```text
VkImage / memory / descriptor.
```

Undefined behavior.

So CPU ownership lifetime is not enough.

We need **GPU completion lifetime**.

---

**Frames-in-flight timeline**

CPU:

```text
submit frame 100
submit frame 101
submit frame 102
```

GPU might currently execute frame 99.

A resource referenced by frame 100 must survive until frame 100 completes.

Thus:

$$
DestroyTime(resource)
>
LastGPUUse(resource).
$$

This is the fundamental resource-lifetime inequality.

---

**Deferred destruction queue**

A simple frame-fence model:

```cpp
struct RetiredResource
{
    GPUResource Resource;
    std::uint64_t RetireFence;
};

std::vector<RetiredResource>
    retired;
```

On replacement:

```cpp
retired.push_back({
    std::move(old),
    lastSubmittedFence
});
```

Every frame:

```cpp
while (
    completedFence >=
    retired.front().RetireFence)
{
    Destroy(
        retired.front().Resource);

    retired.pop_front();
}
```

Now no resource is destroyed while referenced by the GPU.

---

**Timeline semaphores/fences**

If backend provides monotonic GPU timeline values:

$$
submitValue=431.
$$

When:

$$
completedValue\ge431,
$$

everything referenced exclusively by submissions $\le431$ may be retired.

This often creates cleaner resource lifetime tracking than an arbitrary “wait N frames.”

---

**Why "wait three frames" is not a proof**

Suppose engine usually has three frames in flight.

You retire resource after three CPU frames.

But GPU stalls for 100 ms.

The CPU may advance more than three logical frames while the GPU still references old resource, depending on synchronization model.

Correct lifetime should be tied to actual GPU completion, not only frame count, unless your per-frame fences explicitly guarantee that relationship.

---

**Descriptor lifetime**

Even if texture image survives, descriptor state referencing it may be in flight.

Hot reload may require:

```text
new image
new view
new descriptor
```

for future frames, while old descriptor/image remain valid for submitted frames.

Do not blindly overwrite descriptor memory that a submitted command buffer can still read unless backend features and synchronization rules explicitly permit it.

---

**Current Kairo Vulkan texture behavior**

`VulkanTexture2D`:

```text
creates image
creates view
creates sampler
uploads through staging
submits transfer
vkQueueWaitIdle
```

during creation.

This is safe but synchronous.

Its destructor immediately destroys:

```text
sampler
view
image
memory.
```

Therefore the owner must ensure no in-flight GPU work references it before destruction.

No general hot-reload retirement queue was found in the inspected asset/renderer layer.

That is a concrete future requirement for Part 1541-quality runtime replacement.

---

**Why `vkQueueWaitIdle` is unsuitable as a general hot-reload solution**

You could hot reload by doing:

```cpp
vkDeviceWaitIdle();
DestroyOld();
```

This is correct.

But every hot reload stalls the entire device.

For editor experimentation it may initially be acceptable.

For streaming/runtime reload it is terrible.

Deferred destruction lets:

```text
old resource finish naturally
while new frames use replacement.
```

---

**Asset streaming**

Streaming means not all content is resident at once.

Example open-world texture:

```text
AssetID exists
but texture may not currently be loaded.
```

A request initiates:

```text
disk read
decompression
artifact parse
GPU upload
publication.
```

That process may span many frames.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/09 - Player choices, rules, and readable feedback|Previous chapter]] · [[Chapter Edition/03 - Game Engine/11 - Streaming resources through explicit states|Next chapter]]
