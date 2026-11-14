# 10 — Reproducible Builds, Profiling, and Repository Contracts

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|Next chapter]]

A system is only reproducible when its build graph, dependencies, tests, diagnostics, and measurement method are explicit. The chapter follows the source-to-editor-to-runtime path and explains how to report evidence without elevating a repository snapshot into a universal promise.

**By the end:** Create build and measurement workflows whose dependencies, configuration, test evidence, and platform boundaries can be reconstructed.

**Recall question:** What evidence would let another engineer distinguish a successful build from a reproducible runtime result?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 16175–20192; source-span SHA256 2d674841a3d0f56d6591af78daf9bc1ed06b6e6cf5cd0e0afa5333e60389f1ba. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Build, document, measure, and reproduce the program

Repository-specific passages below describe the supplied Kairo snapshot. They have not been revalidated against a current checkout; code fragments retain their stated surrounding context and are not all standalone tested programs. Numerical and contract explanations are taught independently of those implementation observations.

### CMake Target Semantics, Usage Requirements, Presets, Ninja DAGs, Toolchains, Clang/MSVC Portability, C++ Modules, Repository Pinning, Reproducible Superbuilds, CI Matrices, Test Architecture, Crash/Logging/Profiling Hooks, Portable Engine Boundaries, and the Complete Source→Editor→Package→Runtime Ownership Graph

This pass **closes Phase 16**.


Pass 14 ended with something like:

$$
\text{correct source code}
+
\text{correct persistent data}.
$$

Pass 15 asks a much nastier question:

$$
\boxed{
\text{Can another machine, OS, compiler and CI worker actually reconstruct the same engine correctly?}
}
$$

Because an architecture that only builds on one developer's machine is not yet an architecture.

The other core question is:

$$
\boxed{
\text{How do we prove that the thing we built is actually the thing we intended to build?}
}
$$

That is why CMake, repository pinning, tests, CI, diagnostics and ownership diagrams belong in the same phase. They are all parts of **reproducibility**.

---

### THE BUILD GRAPH ITSELF

**The CMake target model**

The most important conceptual correction is this:

> Modern CMake should be thought of primarily as a **target graph generator**, not as a collection of global compiler flags.

Suppose the project has:

```text
KairoMath
KairoGeometry
KairoSpatial
KairoPhysicsEngine
KairoAssets
KairoRenderer
KairoEngineCore
KairoPlayer
```

The real build model is a graph:

$$
G=(V,E)
$$

where a vertex is a CMake target and an edge means one target has a usage/build/link relationship with another.

For example:

$$
KairoGeometry\rightarrow KairoMath
$$

$$
KairoSpatial\rightarrow KairoGeometry
$$

$$
KairoPhysicsEngine\rightarrow KairoSpatial
$$

$$
KairoPlayer\rightarrow KairoRenderer.
$$

That graph is what CMake eventually lowers into Ninja, Make, Visual Studio or another generator's rules.

The crucial mental shift is that a target is not merely:

```text
"a library file."
```

A target is a structured bundle of:

```text
sources
compile features
compile definitions
include requirements
link requirements
generated sources
module interfaces
platform-specific properties
transitive usage requirements.
```

---

**Target-local state beats global state**

Old-school CMake often looks like:

```cmake
include_directories(...)
add_definitions(...)
link_libraries(...)
set(CMAKE_CXX_FLAGS ...)
```

These mutate directory/global state.

Now imagine you add:

```text
KairoEditor
KairoPlayer
KairoTools
```

and only one target needs some definition.

Global state leaks it into all of them.

Modern target style instead:

```cmake
add_library(KairoRenderer)

target_compile_features(
    KairoRenderer
    PUBLIC cxx_std_23
)

target_compile_definitions(
    KairoRenderer
    PRIVATE
        KAIRO_RENDERER_INTERNAL_DIAGNOSTICS=1
)

target_link_libraries(
    KairoRenderer
    PUBLIC
        KairoMath
        KairoAssets
)
```

Everything is attached to the owning target.

This gives you locality:

$$
\boxed{
\text{the build specification follows architectural ownership}.
}
$$

---

**Build graph versus source architecture**

A CMake graph can expose architectural mistakes.

Suppose low-level:

```text
KairoMath
```

links:

```text
KairoRenderer.
```

You should immediately ask why.

Likewise if:

```text
KairoAssets
```

requires:

```text
KairoEditor.
```

your dependency direction is probably broken.

The build graph therefore acts as an **executable architecture document**.

If a dependency is absent from CMake, the code ideally cannot quietly use it.

---

**Current Kairo superbuild**

Your top-level `KairoGameEngine` CMake explicitly treats the project as a **sibling-repository workspace**.

It expects something like:

```text
Kairo/
├── KairoGameEngine/
├── KairoMath/
├── KairoGeometry/
├── KairoSpatial/
├── KairoPhysicsEngine/
├── KairoAssets/
├── KairoECS/
├── KairoReflection/
├── KairoEngineCore/
├── KairoRenderer/
└── ...
```

Then the integration repository calls:

```cmake
add_subdirectory(...)
```

for those sibling source trees.

This is important because there is only one physical checkout of each component in the integration workspace.

You explicitly migrated away from nested duplicated submodules.

That substantially simplifies local development.

---

**`add_library()` does not necessarily mean an archive**

CMake targets may represent:

```text
STATIC
SHARED
MODULE
OBJECT
INTERFACE
IMPORTED
ALIAS.
```

An `INTERFACE` target does not produce a library binary; it exists to carry usage requirements. Official CMake documentation describes interface libraries precisely as linkable usage-requirement targets rather than ordinary compiled artifacts. [cmake.org](https://cmake.org/cmake/help/latest/command/add_library.html?utm_source=chatgpt.com)

Your current:

```cmake
add_library(
    KairoGameEngine
    INTERFACE
)
```

is therefore not supposed to create:

```text
libKairoGameEngine.a.
```

It creates a convenience architectural umbrella.

---

**Current Kairo umbrella target**

You have conceptually:

```cmake
target_link_libraries(
    KairoGameEngine
    INTERFACE
        KairoAssets
        KairoECS
        KairoReflection
        KairoEngineCore
        KairoSceneECSBridge
        KairoRenderer
        KairoPhysicsEngine
)
```

This says:

> A consumer linking the KairoGameEngine umbrella inherits this engine integration surface.

That is reasonable **at the top boundary**.

But low-level targets should not link `KairoGameEngine` just because it is convenient.

Otherwise:

```text
KairoSpatial
```

would transitively gain visibility into:

```text
Renderer
Assets
ECS
EngineCore
Physics.
```

That destroys architectural discipline.

The umbrella belongs at the composition edge.

---

**Target properties are the build-system equivalent of component metadata**

Think of:

```cmake
target_compile_features
target_compile_definitions
target_link_libraries
target_sources
```

as assigning structured properties to a target.

CMake then computes the transitive closure of those properties.

This is similar to our ECS/system dependency thinking:

$$
Target_A
\rightarrow
Target_B
$$

can imply:

$$
Requirements(B)
\rightarrow
A.
$$

The difference is build-time rather than runtime.

---

**PUBLIC, PRIVATE and INTERFACE**

This is probably the most important modern-CMake concept to know perfectly.

For a target A depending on B:

```cmake
target_link_libraries(
    A
    PRIVATE B
)
```

means:

$$
A\ uses\ B
$$

but:

$$
Consumers(A)
$$

do **not** automatically inherit B as part of A's usage requirements.

---

**PRIVATE dependency**

Imagine:

```cpp
// A.cpp

#include <zstd.h>

void InternalCompression()
{
    ...
}
```

but A's public API exposes no zstd concepts.

Then:

```cmake
target_link_libraries(
    A
    PRIVATE zstd
)
```

makes sense.

B is implementation detail.

---

**PUBLIC dependency**

Suppose A's public interface exposes B:

```cpp
// exported API

MeshBounds ComputeBounds(
    const kairo::math::Vec3f* vertices);
```

If consumers of A must also satisfy B's usage requirements to compile/use A correctly, B is effectively part of A's public contract.

Then:

```cmake
target_link_libraries(
    A
    PUBLIC KairoMath
)
```

is appropriate.

CMake propagates PUBLIC and INTERFACE usage requirements transitively to consumers. That is how `INTERFACE_LINK_LIBRARIES`, include requirements, definitions, and compile features flow through the target graph. [cmake.org](https://cmake.org/cmake/help/latest/prop_tgt/INTERFACE_LINK_LIBRARIES.html?utm_source=chatgpt.com)

---

**INTERFACE dependency**

Suppose target A itself does not need B to build, but users of A do.

An `INTERFACE` library is the clearest example:

```cmake
add_library(
    KairoGameEngine
    INTERFACE
)

target_link_libraries(
    KairoGameEngine
    INTERFACE
        KairoRenderer
        KairoPhysicsEngine
)
```

There is no KairoGameEngine object code needing those libraries.

The **consumers** do.

So:

$$
INTERFACE
=
\text{consumer requirement only}.
$$

---

**Truth table**

The easiest way to remember scope is:

| Scope | Used to build target itself? | Propagated to consumers? |
|---|---:|---:|
| PRIVATE | yes | no |
| PUBLIC | yes | yes |
| INTERFACE | no | yes |

This applies conceptually to:

```text
link libraries
include directories
compile definitions
compile features
source/interface sets.
```

The exact property plumbing differs, but this mental model is excellent.

---

**Why incorrect PUBLIC creates dependency contagion**

Suppose:

```text
Renderer PUBLIC Physics
```

even though Physics is needed only inside one `.cpp`.

Then:

```text
Editor
→ Renderer
→ Physics
```

inherits Physics.

Later some editor code casually uses Physics because it "just works."

Now a build-system mistake created architectural coupling.

Therefore:

$$
\boxed{
PUBLIC\ is\ architectural\ API.
}
$$

Do not use PUBLIC because linking failed and you want the error to disappear.

---

**Why incorrect PRIVATE also fails**

Opposite problem:

```text
A's public interface genuinely exposes B
```

but B is marked PRIVATE.

A's own build succeeds.

A downstream consumer may fail because it lacks B's includes, module BMIs, compile definitions or link requirements.

Therefore scope describes **semantic ownership**.

Not "whatever gets today's build green."

---

**Current Kairo example: native framework transitivity**

`KairoRenderer` contains Objective-C++ Metal implementation code on macOS.

The final executable may be pure C++.

If the renderer's static archive requires:

```text
Foundation.framework
Metal.framework
AppKit.framework
QuartzCore.framework
libobjc
```

those final-link requirements have to reach consumers.

Your renderer marks the native link contract PUBLIC.

That is appropriate because:

$$
\text{consumer linking KairoRenderer}
$$

must satisfy that final link environment.

Even though the consumer does not call Objective-C directly.

---

**Public compile definitions**

Current renderer exports backend capability definitions such as:

```text
KAIRO_RENDERER_HAS_VULKAN_BACKEND
KAIRO_RENDERER_HAS_OPENGL_BACKEND
```

and platform-specific capabilities.

If public headers/module interfaces use these definitions to alter exposed contract, PUBLIC may make sense.

If a definition only changes `.cpp` internals, it should be PRIVATE.

Again:

$$
\text{Does the consumer need this to interpret the target's interface?}
$$

That is the test.

---

**Imported and alias targets**

An imported target represents something CMake does **not build here**.

Example:

```cmake
find_package(Vulkan REQUIRED)
```

might provide:

```text
Vulkan::Vulkan.
```

That is effectively an imported target.

It carries:

```text
library location
include requirements
link requirements
possibly per-config locations.
```

CMake documentation explicitly treats IMPORTED targets as externally provided targets with `IMPORTED_*` and `INTERFACE_*` properties. [cmake.org](https://cmake.org/cmake/help/latest/command/add_library.html?utm_source=chatgpt.com)

---

**Why imported targets beat raw library paths**

Bad:

```cmake
target_link_libraries(
    Renderer
    PRIVATE
    "/usr/local/lib/libvulkan.so"
)
```

This bakes in:

```text
one machine
one filesystem
one platform.
```

Better:

```cmake
target_link_libraries(
    Renderer
    PRIVATE
    Vulkan::Vulkan
)
```

Now the package finder/toolchain determines actual location.

The consuming target expresses a semantic dependency.

---

**Alias targets**

Alias:

```cmake
add_library(
    Kairo::SceneECSBridge
    ALIAS
    KairoSceneECSBridge
)
```

does not create another library.

It gives another stable target name.

Common convention:

```text
Namespace::Target.
```

Benefits include:

```text
clear ownership
package-like naming
easy typo detection
consistent consumer syntax.
```

Official CMake treats alias targets as alternate target names rather than separate build artifacts. [cmake.org](https://cmake.org/cmake/help/latest/command/add_library.html?utm_source=chatgpt.com)

---

**Alias naming is useful even internally**

Instead of:

```cmake
target_link_libraries(
    Game
    PRIVATE KairoRenderer
)
```

a future installed/exported API might prefer:

```cmake
target_link_libraries(
    Game
    PRIVATE Kairo::Renderer
)
```

This visually communicates:

> this is a CMake target contract, not a raw linker filename.

---

**CMake presets**

A preset is a versioned named build configuration.

Instead of telling every developer:

```bash
cmake -S . -B build/foo \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DKAIRO_GAME_ENGINE_BUILD_EDITOR=ON \
  -DKAIRO_GAME_ENGINE_BUILD_RAYTRACER=ON \
  ...
```

you write:

```bash
cmake --preset dev-clang
```

Official CMake distinguishes checked-in `CMakePresets.json` for project-wide shared configuration from `CMakeUserPresets.json` for developer-local settings that generally should not be committed. [cmake.org](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html?utm_source=chatgpt.com)

---

**Current Kairo presets**

Your current file defines profiles including roughly:

```text
dev
dev-clang
release
ci-clang
ci-sanitized
ci-msvc
compute-stack-clang.
```

This is good because each name describes an **intent**.

`ci-sanitized` means:

```text
Debug
ASan + UBSan
no heavyweight editor/raytracer paths
```

while:

```text
release
```

turns testing/validation features down appropriately.

---

**Preset inheritance**

Current:

```text
dev-clang
```

inherits:

```text
dev
```

then adds:

```text
Clang toolchain
separate binary directory.
```

This avoids duplicating 20 settings.

Conceptually:

$$
P_{child}
=
P_{parent}
+
Overrides.
$$

---

**Why binary directories should differ**

Do not use one build directory for:

```text
GCC Debug
Clang Release
MSVC
ASan.
```

Compiler and configuration-specific artifacts are not interchangeable.

Current Kairo uses:

```text
build/dev
build/dev-clang
build/release
build/ci-clang
...
```

which is correct.

---

**Developer-local compiler path**

You do **not** want this in checked-in presets:

```text
/opt/homebrew/Cellar/llvm/23.1.4/bin/clang++.
```

That path belongs to one machine.

Instead Kairo's toolchain accepts:

```text
KAIRO_CXX_COMPILER
```

from environment or locates Homebrew LLVM dynamically.

This cleanly separates:

$$
\text{shared build intent}
$$

from:

$$
\text{machine-local toolchain location}.
$$

---

**Command-line override**

Presets are defaults, not absolute prison.

You can do:

```bash
cmake --preset ci-clang \
  -DKAIRO_GAME_ENGINE_BUILD_RAYTRACER=OFF
```

CMake permits command-line settings to override preset variables. [cmake.org](https://cmake.org/cmake/help/git-stage/guide/tutorial/Configuration%20and%20Cache%20Variables.html?utm_source=chatgpt.com)

Your CI does this on macOS for a known current Clang/frontend limitation.

That is a legitimate targeted override.

---

**Ninja build scheduling**

CMake does not generally compile source files itself.

With Ninja generator:

```text
CMake
  |
  | generates
  v
build.ninja
  |
  v
Ninja
  |
  v
compiler/linker/custom commands.
```

Ninja is intentionally a low-level dependency-graph executor. It tries to do the minimum work needed and executes independent edges in parallel. [ninja-build.org](https://ninja-build.org/manual.html?utm_source=chatgpt.com)

---

**Build edge model**

A simple compile relationship:

```text
foo.cpp
  |
  v
foo.o
```

and:

```text
bar.cpp
  |
  v
bar.o
```

then:

```text
foo.o + bar.o
      |
      v
   executable.
```

Ninja can compile `foo.cpp` and `bar.cpp` concurrently because neither depends on the other.

Graph:

$$
foo.cpp\rightarrow foo.o
$$

$$
bar.cpp\rightarrow bar.o
$$

$$
(foo.o,bar.o)\rightarrow Game.
$$

---

**Parallelism is graph-limited**

Suppose 100 translation units are independent.

Ninja can run many simultaneously.

But if:

```text
GeneratedHeader
      ↓
A.cpp
      ↓
B.cpp
      ↓
C.cpp
```

the dependency chain constrains parallelism.

Critical-path build time approximately depends on:

$$
\boxed{
\max_{\text{path}}
\sum_i Cost(edge_i)
}
$$

rather than only total work.

This is why build architecture matters.

---

**Incorrect dependency declaration causes flaky clean builds**

Suppose shader code generation creates:

```text
generated_shader.hpp
```

but consumer target does not depend on generator.

Incremental build may succeed because stale generated file exists.

Fresh CI build fails.

This is a classic underspecified build DAG.

Ninja is deliberately strict: missing dependency edges can lead to incorrect concurrent scheduling. Its manual even provides `missingdeps` diagnostics for generated-file dependency problems. [ninja-build.org](https://ninja-build.org/manual.html?utm_source=chatgpt.com)

---

**Current Kairo shader build edges**

Renderer uses explicit:

```cmake
add_custom_command(
    OUTPUT ...
    COMMAND glslangValidator ...
    DEPENDS shader-source
)
```

then:

```cmake
add_custom_target(
    KairoRendererShaders
    DEPENDS generated SPIR-V files
)
```

and:

```cmake
add_dependencies(
    KairoRenderer
    KairoRendererShaders
)
```

That explicitly represents:

$$
ShaderSource
\rightarrow
SPIRV
\rightarrow
RendererTarget.
$$

Correct build graph thinking.

---

**Over-serialization of build graph**

The opposite mistake is:

```cmake
add_dependencies(A B)
```

when no true ordering exists.

That prevents parallelism.

If:

```text
A and B are independent
```

but build graph says:

$$
B\rightarrow A,
$$

Ninja must wait.

Use dependency edges only when correctness requires them.

---

**C++ modules change build scheduling**

Headers permit:

```text
A.cpp
B.cpp
C.cpp
```

to compile independently after preprocessing dependencies are known.

C++ modules introduce BMIs.

If:

```cpp
// B.cppm
export module B;
```

and:

```cpp
// A.cppm
export module A;
import B;
```

then:

$$
BMI_B
$$

must exist before A can compile.

CMake scans module imports and communicates dynamic ordering to the underlying build tool. CMake's C++ module support was introduced in 3.28 specifically because module compilation requires build-order discovery that ordinary header workflows do not. [cmake.org](https://cmake.org/cmake/help/git-stage/manual/cmake-cxxmodules.7.html?utm_source=chatgpt.com)

---

**Compiler toolchains**

A toolchain is more than:

```text
clang++.
```

It includes some combination of:

```text
C compiler
C++ compiler
linker
archiver
standard library
target triple
sysroot
SDK
platform frameworks
runtime libraries.
```

CMake's toolchain abstraction exists so these choices can be established early, especially for non-default or cross-compilation builds. [cmake.org](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html?highlight=toolchains\&utm_source=chatgpt.com)

---

**Why compiler selection must happen early**

CMake interrogates the compiler during configuration.

It learns:

```text
compiler ID
compiler version
ABI
supported features
platform defaults.
```

Changing compiler after a build directory is configured is unsafe.

That is why:

```text
one build dir per toolchain
```

is a strong rule.

---

**Current Kairo Clang toolchain**

Your toolchain does several good things.

It first checks:

```text
KAIRO_CXX_COMPILER
```

environment override.

On macOS it can discover Homebrew LLVM without hard-coding:

```text
/opt/homebrew
```

or:

```text
/usr/local.
```

Then it derives matching:

```text
clang
clang++
Objective-C++ driver
```

from one LLVM installation.

That avoids an ugly mixed toolchain such as:

```text
Homebrew clang++ compile
Apple clang Objective-C++
different linker assumptions.
```

---

**SDK/toolchain consistency**

A C++ object is not compiled in isolation.

Especially on macOS:

```text
compiler frontend
SDK headers
libc++
linker
Objective-C runtime
frameworks
debug info format
```

must cooperate.

Your current toolchain even pins Debug DWARF behavior because newer upstream Clang output can trigger diagnostic incompatibilities with Apple's linker.

This is exactly what "toolchain portability" means in practice.

Not just:

> install clang.

---

**Cross compilation**

Suppose host:

$$
x86_64\ Linux
$$

target:

$$
ARM64\ Linux.
$$

Compiler must use:

```text
target triple
target sysroot
target libraries.
```

CMake toolchain can define:

```cmake
set(
    CMAKE_SYSTEM_NAME
    Linux
)

set(
    CMAKE_SYSTEM_PROCESSOR
    arm64
)

set(
    CMAKE_CXX_COMPILER
    clang++
)

set(
    CMAKE_CXX_COMPILER_TARGET
    aarch64-linux-gnu
)
```

The target architecture is part of the build identity.

---

**Toolchain fingerprints matter for reproducibility**

Two developers can build the same source SHA using:

```text
Clang 18
Clang 23
```

and get different:

```text
warnings
code generation
module behavior
debug info
optimizer behavior.
```

Therefore a truly reproducible build record should capture:

```text
compiler
version
standard library
CMake
Ninja
dependency SHAs
configuration.
```

Source commit alone is not complete provenance.

---

**Clang versus MSVC compatibility**

"Valid C++23" is not enough to guarantee both toolchains compile the same code today.

Differences arise from:

```text
feature implementation maturity
diagnostics
extensions
preprocessor
module implementation
warning policy
linker behavior
runtime library
Windows ABI.
```

This is why CI must actually build with multiple compilers.

---

**Warning syntax differs**

Clang/GCC:

```cmake
-Wall
-Wextra
-Wpedantic
```

MSVC:

```cmake
/W4
```

Kairo's CMake selects appropriate warning options by compiler ID.

That is correct.

Do not feed:

```text
-Wall
```

to MSVC and assume CMake will translate it.

---

**Diagnostic suppression should be narrow**

Current renderer has an excellent example.

D3D12 implementation intentionally uses explicit alignment that produces MSVC warning:

```text
C4324
```

for padding.

Rather than disabling C4324 globally:

```cmake
add_compile_options(/wd4324)
```

you apply it only to:

```text
detail/Direct3D12Backend.cpp.
```

That preserves the warning everywhere else.

Principle:

$$
\boxed{
\text{suppress the warning at the narrowest justified scope}.
}
$$

---

**Vendor/compiler quirks need explicit gates**

Current Kairo also contains a very specific macOS LLVM 23 workaround for source-location exhaustion in a massive module graph.

It disables one host integration route only for:

```text
Apple platform
Clang
version >=23 and <24.
```

Other compilers/platforms retain the path.

That is vastly better than:

```text
disable feature everywhere.
```

This is how portability should work:

$$
\text{smallest capability-conditioned deviation}.
$$

---

**Never use compiler name as a substitute for capability forever**

Initially:

```cmake
if(MSVC)
...
endif()
```

is often necessary.

But when possible, test actual capability.

For example:

```text
supports feature?
library available?
header available?
backend available?
```

because compilers evolve.

The long-term abstraction should be:

$$
Capability
$$

rather than:

$$
VendorName.
$$

---

**C++ modules in multi-target builds**

Now we combine:

```text
targets
PUBLIC/PRIVATE
Ninja
compiler differences
dependency ordering.
```

A C++ module interface:

```cpp
export module Kairo.Math;

export struct Vec3
{
    ...
};
```

produces a compiled module interface/BMI.

If another target imports:

```cpp
import Kairo.Math;
```

the build system must know:

1. which target provides it,
2. how that target is compiled,
3. which BMI belongs to the current compiler/configuration,
4. that provider must be available before consumer module compiles.

---

**CMake `FILE_SET CXX_MODULES`**

Current Kairo uses patterns like:

```cmake
target_sources(
    KairoSceneECSBridge
    PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/Runtime
    FILES
        Runtime/KairoSceneECSBridge.cppm
)
```

CMake 3.28 added module dependency scanning support; files in `CXX_MODULES` file sets participate in module scanning and ordering. [cmake.org](https://cmake.org/cmake/help/git-stage/manual/cmake-cxxmodules.7.html?utm_source=chatgpt.com)

This is exactly why your root requires:

```cmake
cmake_minimum_required(
    VERSION 3.28
)
```

rather than an older version.

---

**Module BMI is compiler-specific**

Do not treat a Clang BMI as a portable artifact consumable by MSVC.

In general:

$$
BMI=
F(
Compiler,
CompilerVersion,
Flags,
Source,
Dependencies,
Configuration
).
$$

This means BMI should live in build tree/cache, not be checked into source control as universal binaries.

---

**Modules reduce textual inclusion but introduce stronger build dependencies**

Header model:

```text
A.cpp preprocesses header B
```

Module model:

```text
B must compile to BMI
then A consumes BMI.
```

This can improve compile isolation but creates more explicit graph ordering.

Large chains:

```text
M1 → M2 → M3 → M4 → M5
```

reduce available parallelism.

So module architecture still benefits from shallow dependency layering.

---

**Module cycles remain architectural cycles**

If:

```cpp
module A;
import B;
```

and:

```cpp
module B;
import A;
```

you have not found a "modules problem."

You have an architectural cycle.

Fix by:

```text
extracting shared contracts
moving bridge upward
splitting responsibility.
```

Exactly the same principles from M091.

---

**Public modules and target dependencies**

If an exported module interface exposes types or requirements from another target, that dependency is generally part of the consumer-facing contract.

For example:

```cpp
export module Renderer.Scene;

export import Kairo.Math;
```

is obviously public.

Even without `export import`, if exported signatures use types whose definitions/usage requirements are required downstream, the target dependency must be modeled accordingly.

PUBLIC/PRIVATE remains architectural even with modules.

Modules do not remove transitive dependency design.

---

### REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW

**Git submodule commit pinning**

A Git submodule fundamentally records:

```text
path
repository URL/config
exact commit object.
```

The important part for reproducibility is the commit.

Not:

```text
"main branch."
```

Because:

$$
main(t_1)
\neq
main(t_2).
$$

If your integration project says:

```text
KairoRenderer main
```

two clones at different dates may build different engines.

Pin:

$$
KairoRenderer@SHA.
$$

---

**Why commit pinning matters**

Suppose EngineCore expects renderer API:

```cpp
SubmitScene(
    RenderScene&);
```

Renderer main changes tomorrow to:

```cpp
SubmitFrame(
    FrameDescription&);
```

A fresh CI clone now fails despite GameEngine source being unchanged.

The integration state was never fully specified.

A reproducible workspace must define:

$$
\boxed{
EngineState=
\{Repo_i,SHA_i\}_{i=1}^{N}.
}
$$

---

**Current Kairo has moved beyond ordinary submodules**

Your repository includes a migration script explicitly removing old nested component submodules.

Current architecture instead uses sibling repositories plus:

```text
workspace.lock.
```

This is important.

The curriculum atom says "Git submodule commit pinning," but the underlying principle is **commit-pinned multi-repository composition**.

Kairo currently implements the principle without using active nested submodules.

---

**Current `workspace.lock`**

It records exact full 40-hex SHAs:

```text
KairoMath          <SHA>
KairoGeometry      <SHA>
KairoSpatial       <SHA>
KairoPhysicsEngine <SHA>
...
```

This is functionally the same integration guarantee as submodule gitlinks:

$$
RepoName
\rightarrow
ExactCommit.
$$

In some ways it is easier to reason about in your sibling-workspace model.

---

**Why full SHA, not short SHA**

Short SHAs are contextual prefixes.

They may become ambiguous as repository history grows.

A reproducibility lock should use full commit ID.

Current Kairo validates:

```text
40 lowercase hex characters.
```

That is good.

---

**Lock verification must also detect dirty trees**

Suppose repository HEAD exactly matches locked SHA but developer modified one file.

Then:

$$
HEAD=LockedSHA
$$

yet actual source is different.

So reproducible state requires:

$$
HEAD_i=LockedSHA_i
$$

and:

$$
WorkingTree_i=Clean.
$$

Your `verify_workspace_lock.sh` checks both.

This is an important detail many "reproducible" scripts miss.

---

**Materialization algorithm**

Your materializer roughly performs:

```text
read lock entry
      ↓
repo exists?
  no → clone
  yes → reject meaningful dirty changes
      ↓
fetch commit
      ↓
verify commit object exists
      ↓
checkout --detach exact SHA
      ↓
verify HEAD == expected SHA.
```

This is much stronger than:

```bash
git clone repo
git checkout main
```

---

**Detached HEAD is desirable in CI**

CI does not need to accidentally track a branch tip.

It wants exactly:

```text
commit X.
```

Detached HEAD communicates:

> this checkout represents immutable integration input.

That is appropriate for build workers.

---

**Reproducible superbuilds**

A **superbuild** composes many projects into one coordinated build.

Your KairoGameEngine root is effectively a superbuild.

It:

```text
materializes component repos
sets component options
adds component subdirectories
builds integration bridges
builds player/editor/samples
runs cross-component tests.
```

The key property is:

$$
\boxed{
\text{one configuration creates one coherent component graph}.
}
$$

---

**Why standalone CI is not enough**

Suppose:

```text
KairoRenderer tests pass.
KairoEngineCore tests pass.
```

But Renderer changed a public module contract that EngineCore integration bridge expects.

Each repo independently green.

Combined engine broken.

Therefore you need:

```text
component CI
+
superbuild CI.
```

They test different boundaries.

---

**Current dual strategy**

Your repositories do both.

Examples:

```text
KairoECS CI
KairoRenderer CI
KairoEngineCore CI
```

test standalone component correctness.

Then:

```text
KairoGameEngine Build, Test, and Package
```

materializes locked component versions and tests the integrated stack.

Excellent architecture.

---

**Immutable dependency fallback**

KairoRenderer also defines pinned commit SHAs for standalone dependencies.

If it is built inside superbuild:

```text
TARGET KairoMath already exists
```

so reuse.

If standalone:

```text
local source path?
sibling?
otherwise FetchContent exact pinned SHA.
```

This allows:

$$
StandaloneRenderer
$$

and:

$$
IntegratedRenderer
$$

without tracking dependency branch heads.

That is good reproducibility.

---

**Reproducibility has levels**

A useful hierarchy is:

**Source reproducibility**

Exact source SHAs known.

**Dependency reproducibility**

Third-party versions pinned.

**Toolchain reproducibility**

Compiler/CMake/Ninja/SDK known.

**Configuration reproducibility**

Preset/options known.

**Artifact reproducibility**

Same bytes produced.

Kairo is strong on the first four.

Bit-for-bit artifact reproducibility is a further bar and may require eliminating timestamps, absolute paths, nondeterministic archive ordering, linker build IDs, etc.

Do not claim more than you prove.

---

**Build provenance record**

A strong release could emit:

```text
KairoGameEngine SHA
workspace.lock hash
CMake version
Ninja version
compiler ID/version
platform/SDK
preset
configuration flags
dependency package versions.
```

Then any artifact can be traced to its exact build recipe.

This is extremely valuable for research reproducibility too.

---

**Continuous-integration matrices**

A CI matrix answers:

> Which combinations of environment do we prove?

Current top-level superbuild matrix includes approximately:

```text
Ubuntu + Clang
Ubuntu + Clang + ASan/UBSan
macOS + upstream/Homebrew Clang
Windows + MSVC.
```

This is meaningful coverage.

It is not simply four duplicate jobs.

Each axis catches different defects.

---

**Why Linux Clang matters**

It catches:

```text
POSIX assumptions
Clang diagnostics
Linux linker behavior
case-sensitive filesystem behavior
OpenGL/Vulkan Linux runtime issues.
```

---

**Why macOS LLVM matters**

It catches:

```text
Apple frameworks
Objective-C++
Metal
MoltenVK
Darwin linker semantics
macOS module/toolchain behavior.
```

---

**Why Windows MSVC matters**

It catches:

```text
MSVC language conformance differences
Windows filesystem/path behavior
D3D12
Windows linker/import-library semantics
different standard library implementation.
```

A project that compiles only with Clang has not proven portable C++.

---

**Sanitizer lane**

ASan:

```text
heap buffer overflow
use-after-free
stack errors
```

UBSan:

```text
undefined shifts
bad casts
signed overflow classes
alignment violations
```

depending on instrumentation.

These are orthogonal to normal Release build tests.

Current `ci-sanitized` removes some heavyweight targets to keep the sanitizer lane focused and tractable.

That is reasonable.

---

**CI matrix explosion**

Suppose you test:

$$
3\ OS
\times
3\ compilers
\times
3\ build\ modes
\times
4\ backends
=
108
$$

jobs.

That becomes expensive.

Choose representative combinations.

You do not need full Cartesian product every commit.

You can have:

```text
PR matrix
nightly extended matrix
release matrix.
```

Optimization problem:

$$
\max Coverage
$$

subject to:

$$
CI\ Cost\le Budget.
$$

---

**Fail-fast false**

Your main matrix uses:

```yaml
fail-fast: false
```

That is useful.

If Linux fails, still learn whether Windows/macOS pass.

One CI run produces a cross-platform failure map.

For portability work, that diagnostic value is often worth extra runner time.

---

**CI dependency installation is part of the test**

Current CI explicitly installs:

```text
Ninja
Clang
GLFW
Vulkan SDK/libraries
glslang
Xvfb
etc.
```

A clean runner therefore exercises bootstrap assumptions.

A developer machine may accidentally have hidden dependencies installed.

CI reveals them.

---

**Unit tests versus integration tests**

A unit test tries to isolate a small behavior.

Example ECS:

```cpp
TEST_CASE(
    "destroyed entity handle is stale")
{
    Registry registry;

    Entity a =
        registry.Create();

    registry.Destroy(a);

    REQUIRE_FALSE(
        registry.IsAlive(a));
}
```

No renderer.

No window.

No project loading.

No GPU.

This is a unit-level invariant test.

---

**Unit tests should localize failures**

If sparse-set invariant breaks, you want:

```text
KairoECS tests fail.
```

Not:

```text
KairoPlayer cannot render sample project.
```

The latter is too far from cause.

Unit tests provide fault localization.

---

**Integration tests**

An integration test deliberately crosses boundaries.

Example:

```text
Scene
    ↓
SceneECSBridge
    ↓
ECS
```

Current:

```text
KairoSceneECSBridgeTests
```

does exactly this class of test.

Another:

```text
Physics debug data
    ↓
Renderer bridge
```

is integration.

---

**Integration tests validate contracts, not implementation details**

Suppose:

```text
AssetRegistry
+
SceneLoader
+
RendererAdapter.
```

An integration test should verify:

```text
the same stable asset reaches renderer correctly.
```

It should not assert internal vector capacities unless those are contract.

This keeps tests robust during refactoring.

---

**Test pyramid for engine code**

A practical distribution:

```text
many fast unit tests

fewer subsystem integration tests

fewer full player/editor smoke tests

very few expensive hardware/content acceptance tests.
```

Why?

Fast unit tests run continuously.

Full GPU tests are slower and more environment-sensitive.

Every test layer has a role.

---

**Smoke tests**

A smoke test asks:

$$
\boxed{
\text{Does the major path basically start and function?}
}
$$

It does not prove every detail.

Example:

```text
launch KairoPlayer
load project
initialize renderer
execute one/few frames
exit successfully.
```

That is much broader than a unit test.

---

**Current shared-content smoke**

Your superbuild uses:

```text
KairoSharedContentPlayerSmoke.
```

It chooses a deterministic backend by platform rather than letting `auto` choose unpredictably.

Linux/macOS CI window constraints are handled explicitly.

This is good smoke-test design.

---

**Why smoke tests may legitimately skip**

Renderer smoke tests use:

```text
SKIP_RETURN_CODE 77.
```

Suppose a CI machine lacks required GPU/display capability.

That is not necessarily:

```text
test passed.
```

Nor necessarily:

```text
engine code failed.
```

It may be:

```text
environment cannot execute this test.
```

A distinct skip state is honest.

Never report unavailable-hardware tests as passes.

---

**Validation test versus smoke test**

Current Kairo distinguishes:

```text
KairoSharedContentProjectValidation
```

from:

```text
KairoSharedContentPlayerSmoke.
```

Validation asks:

> Is project structurally valid?

Smoke asks:

> Can runtime execute it?

This separation is excellent.

One can run headlessly even when GPU/window tests cannot.

---

**Deterministic test fixtures**

A deterministic fixture should behave identically across runs.

Avoid:

```text
download current latest model
use current time
random unseeded values
depend on user's home directory
depend on current network response.
```

Prefer:

```text
committed fixture
exact checksum
seeded PRNG
fixed paths
known expected outputs.
```

---

**Current Kairo content fixtures**

Your repository has:

```text
TestContent/Compatibility/committed/
```

containing known assets such as glTF/HDR fixtures and malformed input.

It also has checksum verification scripts.

This is exactly the correct strategy for deterministic compatibility tests.

---

**Why checksums matter for test content**

Imagine fixture file silently changes.

Tests may begin producing different results.

Without provenance you may think:

> engine regression.

But actual change was fixture.

Checksum gate verifies:

$$
FixtureBytes=
ExpectedFixtureBytes.
$$

Thus the input itself is controlled.

---

**Malformed fixtures are first-class test assets**

A good test corpus includes not only happy data.

Examples:

```text
invalid JSON
truncated binary
missing dependency
unsupported schema version
out-of-range count
path traversal
cycle.
```

A robust engine must prove:

$$
\text{invalid input}
\rightarrow
\text{controlled failure}.
$$

Not:

$$
\text{crash/UB}.
$$

Your compatibility tree includes malformed content, which is good.

---

**Randomness in deterministic tests**

If algorithm needs randomness:

```cpp
std::mt19937 rng(
    0x12345678);
```

Record seed.

On failure:

```text
seed=0x12345678
```

allows exact replay.

Property-based tests can use many seeds, but failing seed must be reported.

---

**Time**

Avoid:

```cpp
auto now =
    system_clock::now();
```

inside deterministic test semantics.

Inject:

```cpp
FakeClock.
```

Your logger's monotonic sequence numbers rather than wall-clock timestamps are a nice example of making diagnostics testable and deterministic.

---

**Crash reporting and structured logging**

These are related but distinct.

Structured logging answers:

> What happened before the failure?

Crash reporting answers:

> What failed at process level, where, and in what environment?

A serious engine eventually wants both.

---

**Structured logging**

Bad log:

```text
failed
```

Better:

```text
severity=error
category=assets.import
asset=f03b...
importer=kairo.texture.stb
message="invalid mip payload"
```

Structure lets tooling filter/search.

A log record can conceptually be:

```cpp
struct LogRecord
{
    Severity Severity;
    StringID Category;
    uint64_t Sequence;
    std::string Message;

    // optional fields
    std::vector<Field>
        Fields;
};
```

---

**Current Kairo Logger**

Current `EngineCore::Logger` has:

```text
Trace
Debug
Info
Warning
Error
Critical
```

and stores:

```text
Sequence
Severity
Category
Message.
```

It is:

```text
thread-safe
bounded
backend-neutral.
```

Very sensible foundation.

It deliberately does not force:

```text
stdout
file
network telemetry.
```

The host can snapshot and forward records.

---

**Deterministic sequence numbers**

Instead of attaching wall-clock timestamp as primary identity, Kairo increments:

$$
Sequence=1,2,3,\dots
$$

This gives deterministic event ordering per logger instance.

For runtime production you may additionally attach:

```text
monotonic timestamp
wall-clock timestamp
thread ID
frame number
run ID.
```

But sequence is a strong ordering key.

---

**Bounded logging**

Unbounded logger:

```text
runs 8 hours
writes millions of messages
memory grows indefinitely.
```

Kairo evicts oldest record when capacity is reached.

That is appropriate for in-memory diagnostic history.

Persistent sinks may use rolling files.

---

**Crash reporting**

No dedicated `CrashReporter`/`CoreDiagnostics.cppm` equivalent was found in the inspected current repositories even though the curriculum lab card names a `CoreDiagnostics.cppm` target concept.

So for Part 1597, this is partly **target architecture**, not a claim about current implementation.

A crash system generally needs platform-specific hooks:

```text
Windows:
unhandled exception / minidump

Linux:
signals + external dump tooling

macOS:
Mach/CrashReporter integration or external service.
```

Be extremely careful doing complex work inside POSIX signal handlers.

---

**Crash-report payload**

Useful crash metadata:

```text
build ID
workspace.lock hash
Git SHA
platform
compiler
exception/signal
thread
stack
recent structured log ring
active scene
last frame number
renderer backend
GPU/driver info
possibly recent profiler breadcrumbs.
```

The crash artifact becomes reproducibility evidence.

---

**Do not allocate wildly in a crash handler**

At crash time:

```text
heap may be corrupted
locks may be held
threads may be suspended.
```

Calling arbitrary engine code can deadlock or crash again.

A robust crash architecture pre-allocates/minimizes in-process work and delegates symbolization/upload to a safer postmortem helper where possible.

---

**Breadcrumbs**

Before risky transitions:

```text
"loading scene X"
"submitting frame 731"
"reloading shader Y"
```

record lightweight breadcrumbs.

Crash report can reveal the last known logical path even when stack is low-level.

This is particularly useful for asynchronous engines.

---

**Runtime profiling hooks**

Logging tells:

```text
what happened.
```

Profiling tells:

```text
where time/resources went.
```

A profiling hook should have extremely low overhead when disabled.

Typical API:

```cpp
ProfileScope scope{
    "Physics.Broadphase"
};
```

or macro:

```cpp
KAIRO_PROFILE_SCOPE(
    "Physics.Broadphase");
```

---

**RAII profile scope**

```cpp
class ProfileScope
{
public:
    ProfileScope(
        Profiler& profiler,
        StringID name)
        : m_Profiler(&profiler),
          m_Name(name),
          m_Start(
              Clock::now())
    {}

    ~ProfileScope() noexcept
    {
        m_Profiler->Record(
            m_Name,
            Clock::now() -
                m_Start);
    }

private:
    Profiler* m_Profiler;
    StringID m_Name;
    TimePoint m_Start;
};
```

Then:

```cpp
void PhysicsWorld::Step(...)
{
    ProfileScope profile{
        profiler,
        "Physics.Step"
    };

    ...
}
```

No forgotten end marker.

---

**Current Kairo has subsystem profiling but no unified profiler abstraction**

Current PhysicsWorld exposes:

```text
PhysicsStepProfile
```

with timing for:

```text
whole step
broadphase
narrowphase
solver.
```

RenderGraph exposes:

```text
RenderPassProfile
RenderGraphExecutionProfile
```

with per-pass and total milliseconds.

BVH exposes:

```text
BuildMilliseconds.
```

So profiling instrumentation exists in important subsystems.

But no single cross-engine tracing/profiler API was found in the inspected Core/integration repos.

That is the next architectural step.

---

**CPU profiling clocks**

Use:

```cpp
std::chrono::steady_clock
```

for durations.

Not:

```cpp
system_clock
```

because wall clock can jump due to:

```text
NTP
manual adjustment
timezone
daylight saving.
```

Duration measurement requires monotonic time.

Your PhysicsWorld and RenderGraph already use `steady_clock`.

Good.

---

**GPU profiling is different**

CPU timing:

```text
submit command buffer
```

does not measure actual GPU execution.

For GPU passes you need backend timestamp queries.

Conceptually:

```text
write GPU timestamp before pass
execute pass
write GPU timestamp after pass
resolve after GPU completion.
```

Then:

$$
T_{GPU}
=
(T_{end}-T_{begin})
\times TimestampPeriod.
$$

CPU and GPU timelines are separate.

---

**Profiling spans need hierarchy**

Frame:

```text
Frame
├── Input
├── Physics
│   ├── Broadphase
│   ├── Narrowphase
│   └── Solver
├── Animation
├── RenderExtract
└── Render
    ├── Shadow
    ├── GBuffer
    └── Lighting
```

A span model lets tools reconstruct nesting and thread activity.

Record:

```text
span ID
parent span ID
thread
start
end
category.
```

---

**Multithreaded profiling**

With jobs:

```text
thread A runs physics chunk
thread B runs animation chunk
thread C builds render commands.
```

Per-thread scopes need a shared monotonic clock domain.

Then visualization can show:

```text
worker0 █████
worker1   ███████
worker2 ███   ███
```

and expose:

```text
idle gaps
contention
critical path.
```

---

**Profiling should not become global hidden dependency**

Avoid every module directly calling:

```cpp
GlobalProfiler::Get().
```

A lightweight hook interface or compile-time macro layer can keep foundational modules independent.

Example:

```cpp
struct ProfileSink
{
    virtual void Record(
        const ProfileEvent&) = 0;
};
```

or a no-op backend.

Again:

$$
\text{instrumentation mechanism}
\neq
\text{telemetry product}.
$$

---

**Portable boundaries without lowest-common-denominator design**

This atom is extremely important.

A common portability mistake is:

> Vulkan, Metal and D3D12 differ. Therefore expose only the tiny intersection of all three.

You end up with:

```text
Renderer API from 2005
```

unable to use modern capabilities.

That is **lowest-common-denominator abstraction**.

Do not do this.

---

**Another portability mistake**

The opposite failure:

```cpp
if (backend == Vulkan)
{
    use VkImage...
}
else if (backend == Metal)
{
    use id<MTLTexture>...
}
```

through every higher-level subsystem.

Now no abstraction exists.

You need a middle path.

---

**Separate semantic intent from backend capability**

High-level rendering API can express intent:

```cpp
struct TextureDesc
{
    TextureDimension Dimension;
    PixelFormat Format;
    TextureUsage Usage;
    uint32_t Width;
    uint32_t Height;
};
```

Then backend maps to:

```text
VkImage
MTLTexture
ID3D12Resource.
```

The semantic model should be rich enough for modern hardware.

---

**Capabilities instead of pretending equality**

Expose:

```cpp
struct RenderCapabilities
{
    bool RayTracing = false;
    bool MeshShaders = false;
    bool VariableRateShading = false;

    uint32_t MaxBindlessTextures = 0;
};
```

Then high-level renderer chooses algorithm:

```cpp
if (caps.MeshShaders)
{
    UseMeshletPath();
}
else
{
    UseIndexedDrawPath();
}
```

Portable architecture does **not** mean identical algorithm everywhere.

It means capability-sensitive policy above clean backend mechanisms.

---

**Feature tiers**

Instead of dozens of backend checks:

```text
Tier 0:
basic rasterization

Tier 1:
compute + modern descriptors

Tier 2:
bindless

Tier 3:
ray tracing.
```

Then game/rendering policy can target tiers.

Backend maps hardware/API capabilities to tier.

This reduces conditional complexity.

---

**Backend-specific escape hatches**

Sometimes you genuinely need a platform-specific extension.

A portable engine can expose:

```text
generic core
+
optional extension interfaces.
```

For example:

```cpp
IRenderDevice
```

plus:

```cpp
IMetalRenderDeviceExtensions
```

accessible only where appropriate.

This is better than polluting every generic API with Metal-only fields.

---

**Portability applies beyond rendering**

Filesystem:

```text
project-relative semantic path
```

above:

```text
Windows/Unix filesystem mechanics.
```

Threading:

```text
JobSystem
```

above:

```text
OS thread primitives.
```

Input:

```text
gameplay actions
```

above:

```text
keyboard/gamepad APIs.
```

Assets:

```text
AssetID
```

above:

```text
filesystem path.
```

The repeated pattern is:

$$
\boxed{
SemanticContract
+
CapabilityModel
+
BackendAdapter.
}
$$

---

**Current Kairo renderer demonstrates this direction**

You have high-level:

```text
RenderScene
MeshDraw
TextureHandle
GraphicsBackend
RenderGraph
```

and backend-specific:

```text
VulkanRenderer
MetalRenderer
Direct3D12Renderer
OpenGLRenderer.
```

Compilation includes Metal only on Apple, D3D12 only on Windows, while Vulkan/OpenGL remain broad.

Backend capability definitions are exported.

That is a good basis for portable-not-neutered architecture.

---

**Current Kairo M095/M096 audit**

| Topic | Current state |
|---|---|
| Target-oriented CMake | Strongly implemented |
| Modern PUBLIC/PRIVATE/INTERFACE use | Implemented |
| Integration INTERFACE umbrella | Implemented |
| Alias targets | Present |
| Imported package targets | Used through find_package dependencies |
| Presets | Implemented |
| Ninja | Standard Kairo preset generator |
| Toolchain file | Implemented for Clang |
| MSVC CI | Implemented |
| macOS upstream LLVM CI | Implemented |
| C++23 modules | Heavily implemented |
| CMake CXX_MODULES file sets | Implemented |
| Module/compiler quirks | Explicitly handled |
| Old nested submodules | Migrated away |
| Exact repository commit pinning | Implemented via `workspace.lock` |
| Dirty-tree verification | Implemented |
| Workspace materialization | Implemented |
| Component CI | Implemented across repos |
| Integration superbuild CI | Implemented |
| Linux/macOS/Windows matrix | Implemented |
| Sanitizer lane | Implemented |
| Unit tests | Extensive |
| Integration tests | Extensive |
| Runtime smoke tests | Implemented |
| Content checksum fixtures | Implemented |
| Malformed-content fixtures | Present |
| Structured in-memory logging | Implemented |
| Dedicated crash-report subsystem | Not found |
| Physics profiling | Implemented |
| RenderGraph profiling | Implemented |
| BVH build timing | Implemented |
| Unified cross-engine profiler/tracing | Not found |
| Portable multi-backend renderer | Strongly implemented |
| Full source→package→runtime boundary | Present and increasingly explicit |

---

**One important CMake improvement**

Your top-level root uses some global:

```cmake
add_compile_options(...)
add_link_options(...)
```

for sanitizer enablement.

That is understandable for a superbuild because sanitizers intentionally need broad coverage.

But a more target-oriented architecture can define:

```cmake
add_library(
    KairoSanitizers
    INTERFACE
)

target_compile_options(
    KairoSanitizers
    INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
)

target_link_options(
    KairoSanitizers
    INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
)
```

then link participating targets:

```cmake
target_link_libraries(
    KairoEngineCore
    PRIVATE
    KairoSanitizers
)
```

Why?

Because the sanitizer configuration becomes a named usage requirement instead of ambient global state.

For a whole-engine sanitizer build your current global approach is functional; the interface-target pattern scales better when you need selective instrumentation.

---

**One reproducibility improvement**

`workspace.lock` records source SHAs.

A next-level lock could also record third-party/toolchain provenance, for example:

```text
cmake minimum 3.28
ninja minimum 1.11
glad v0.1.36
Catch2 v3.5.4
compiler-family expectations
Vulkan SDK compatibility range.
```

Not necessarily in the same file.

But a release provenance artifact should capture the whole input closure.

Because:

$$
\text{Repo SHAs}
$$

alone do not uniquely define:

$$
\text{Binary Artifact}.
$$

---

**One module-build improvement**

As the module graph grows, measure:

```text
number of BMI edges
module critical path
module fan-in
module fan-out.
```

A target can become a build bottleneck even if runtime architecture is clean.

For example:

```text
CoreHugeModule
```

imported by 80 modules means every change can rebuild enormous portions of the tree.

Module architecture should minimize unnecessary re-exporting and keep foundational interfaces stable.

---

**One diagnostics improvement**

You already have:

```text
Logger
PhysicsStepProfile
RenderGraphExecutionProfile
BVHBuildStats.
```

A natural next layer is one common diagnostics transport.

For example:

```cpp
struct ProfileEvent
{
    std::string_view Category;
    std::string_view Name;

    uint64_t Thread;
    uint64_t Frame;

    TimePoint Begin;
    TimePoint End;
};
```

Subsystems emit to a sink.

Then editor can show:

```text
CPU timeline
physics phases
render passes
jobs
asset loads.
```

without making every subsystem depend on ImGui or one third-party profiler.

---

**One crash-reporting improvement**

Add a thin Core diagnostics layer that captures:

```text
build provenance
current project
run ID
recent Logger ring
last frame
last profile breadcrumbs.
```

Platform backend creates:

```text
minidump / crash artifact.
```

Postmortem helper performs:

```text
symbolization
compression
optional upload.
```

Keep crash implementation platform-specific while crash metadata format remains portable.

---

**Reconstruction challenge — CMake scope**

Suppose:

```text
A.cpp internally uses B
A's public module API never exposes B.
```

Use:

```cmake
PRIVATE B.
```

Suppose A's exported API requires B's public types/usage requirements.

Use:

```cmake
PUBLIC B.
```

Suppose A is an INTERFACE convenience target forwarding B.

Use:

```cmake
INTERFACE B.
```

If you cannot justify the scope from architectural semantics, do not choose it based on linker error experimentation.

---

**Reconstruction challenge — Ninja DAG**

Given:

```text
shader.vert
shader.frag
Renderer.cppm
Player.cpp
```

and:

```text
Renderer imports compiled shader artifacts
Player depends on Renderer
```

derive:

$$
shader.vert
\rightarrow
vert.spv
$$

$$
shader.frag
\rightarrow
frag.spv
$$

$$
(vert.spv,frag.spv)
\rightarrow
Renderer
$$

$$
Renderer
\rightarrow
Player.
$$

Then explain which edges can execute concurrently and where critical path lies.

---

**Reconstruction challenge — modules**

Given:

```cpp
export module A;
import B;
```

explain why:

$$
BMI_B
$$

must exist before A compilation.

Explain why BMIs are compiler/configuration specific.

Explain why checked-in universal BMIs are a bad idea.

Explain why module dependency cycle is architecture problem rather than an include-guard problem.

That is Part 1590.

---

**Reconstruction challenge — reproducible workspace**

State the invariant:

$$
\forall R_i:
HEAD(R_i)=LockSHA(R_i)
$$

and:

$$
WorkingTree(R_i)=Clean.
$$

Then add:

$$
Toolchain
=
ExpectedToolchain
$$

and:

$$
Configuration
=
ExpectedPreset.
$$

Only then can you accurately call a build:

```text
the locked integration state.
```

Your verify script actually enforces the first two conditions.

---

**Reconstruction challenge — test types**

Given an ECS generation bug:

```text
unit test.
```

Given Scene→ECS conversion:

```text
integration test.
```

Given startup sample project through KairoPlayer:

```text
smoke test.
```

Given known glTF with checksum:

```text
deterministic fixture.
```

Given Vulkan test unavailable on a CI worker:

```text
skip/unavailable
```

not fake success.

The terminology matters because each test answers a different question.

---

**Reconstruction challenge — portable renderer**

Explain why both are wrong:

```text
generic API exposes only 1990s OpenGL-level features
```

and:

```text
engine logic directly manipulates VkImage/MTLTexture everywhere.
```

Correct model:

$$
SemanticAPI
+
Capabilities
+
BackendAdapters.
$$

The high-level algorithm may differ per capability.

Portability does not require identical execution path.

---

**Reconstruction challenge — complete ownership**

Take one texture and state the owner at each step:

$$
PNG
$$

owned by project filesystem.

$$
AssetID
$$

owned by asset metadata model.

$$
DerivedArtifact
$$

owned/cached by DDC/import layer.

$$
TextureAssetHandle
$$

stored by persistent scene/material.

$$
TextureHandle
$$

owned by renderer runtime table.

$$
VkImage/MTLTexture
$$

owned by concrete backend.

$$
InFlightGPUUse
$$

owned logically by submitted work until completion.

If you can trace that without conflating any identities, you have understood Phase 16.

---

**Exact coverage — M095 + M096**

| Part | Topic | Covered |
|---:|---|---|
| 1583 | CMake target model | ✓ |
| 1584 | PUBLIC, PRIVATE, INTERFACE dependencies | ✓ |
| 1585 | Imported and alias targets | ✓ |
| 1586 | CMake presets | ✓ |
| 1587 | Ninja build scheduling | ✓ |
| 1588 | Compiler toolchains | ✓ |
| 1589 | Clang versus MSVC compatibility | ✓ |
| 1590 | C++ modules in multi-target builds | ✓ |
| 1591 | Git submodule commit pinning | ✓ |
| 1592 | Reproducible superbuilds | ✓ |
| 1593 | Continuous-integration matrices | ✓ |
| 1594 | Unit versus integration tests | ✓ |
| 1595 | Smoke tests | ✓ |
| 1596 | Deterministic test fixtures | ✓ |
| 1597 | Crash reporting and structured logging | ✓ |
| 1598 | Runtime profiling hooks | ✓ |
| 1599 | Portable boundary without lowest-common-denominator abstraction | ✓ |
| 1600 | Complete source→editor→packaged-runtime ownership/data flow | ✓ |


At this point the entire engine architecture from Parts 1501–1600 can be compressed into two invariants.

The first is the **dependency invariant**:

$$
\boxed{
Foundation
\leftarrow
Subsystems
\leftarrow
Bridges
\leftarrow
Player/Editor/Game
}
$$

with the build system enforcing that direction through target dependencies, modules, CMake usage requirements and composition roots.

The second is the **identity/lifetime invariant**:

$$
\boxed{
PersistentIdentity
\neq
RuntimeHandle
\neq
StorageAddress
\neq
BackendResource
}
$$

with explicit bridges connecting them.

And now we can append the reproducibility invariant:

$$
\boxed{
BuildArtifact
=
F(
ExactSourceCommits,
Dependencies,
Toolchain,
Configuration,
GeneratedInputs
).
}
$$

If any input to $F$ is implicit, the build is not fully reproducible.

Your current Kairo ecosystem is already quite mature at this boundary: it has an exact sibling-workspace lock, clean-tree verification, component CI plus superbuild CI, Clang/MSVC/macOS coverage, sanitizer lanes, deterministic content fixtures, C++ module file sets, explicit platform backend compilation, structured logging, and several subsystem-level profiling instruments. The two clearest missing production layers are **one unified profiling/tracing transport and a dedicated crash-report pipeline tied to build provenance and recent structured diagnostics**.

**Pass 16 starts Phase 17 at Part 1601: rigid-body dynamics from first principles.** We leave engine/tooling architecture and go directly into the physics mathematics: particles versus rigid bodies, translational/rotational state, center of mass, mass/inverse mass, density, momentum, Newton's laws, forces, torques, inertia tensors, coordinate-space transformation of inertia, and the numerical integration machinery that turns those laws into an actual physics engine.


## Reconstruct the representation and lifetime proofs

## C++ - Assignment as a resource transaction

The [[Assignment and exception guarantees|strong guarantee]] requires a defined observable state before the operation and an unchanged promised state if preparation fails. Let D0 be the destination's original owned allocation/value and S the source. The teaching byte owner's copy assignment follows:

1. Construct replacement R from S without changing D0. Allocation failure propagates while D0 and S retain their original state.
2. Non-throwing swap transfers R's value into the destination and D0 into the temporary. This is the commit point: no remaining step can fail under the stated byte-copy/release contract.
3. Destroy R, releasing the former destination D0 exactly once.

Self-copy is explicitly guarded; an alternative copy-and-swap without that guard could still be safe but allocate unnecessarily. This proof assumes a non-throwing swap, safe release, and copy preparation that does not mutate its source. It does not establish whole-object strong guarantees for arbitrary memberwise assignment or a copy operation with external side effects.

For the failure-injection witness, initial owners A/B each hold one allocation. An injected exception before the new allocation succeeds leaves two owners/two live allocations and B's old byte9. Moving A into C then leaves A empty while C owns A's allocation. Moving C into B releases B's previous allocation before adoption, leaving one allocation. Destroying/resetting its final owner leaves zero. The source remains a live empty object throughout its moved-from interval.

### Why no-throw properties matter to growth

Suppose source element0 is moved and changed, then moving element1 throws. The partially populated new buffer can be cleaned up, but cleanup alone does not restore element0's old value. Copying that preserves source state can support rollback where available; a genuinely non-throwing move avoids that transfer failure. If copying is unavailable and moving may throw, consult the exact operation's exceptional-effects contract. move_if_noexcept encodes the relevant construction-expression choice, not a universal rollback theorem.

### Isolating sink counts from growth

With spare destination capacity and a Probe type whose copy/move constructors are instrumented: lvalue→value parameter→element gives one copy/one move; xvalue→parameter→element gives two moves; same-type prvalue→parameter→element gives direct parameter construction/one move. Cache growth would add transfer of old elements. Optional named-result NRVO is independent of these parameter routes, so record it separately instead of silently adding it to a fixed portable count.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/11 - C++ Const, Borrowing, and CPU–GPU Buffering|Next chapter]]
