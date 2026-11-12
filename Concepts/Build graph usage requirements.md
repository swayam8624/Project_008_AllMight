# Build graph usage requirements

A build target has two interfaces at once. Its source code exposes a programming interface, while its build-system target exposes the information consumers need to compile and link against that interface.

In CMake terminology, PRIVATE, PUBLIC and INTERFACE describe propagation of usage requirements. They do not correspond to private/public members in C++.

A PRIVATE requirement is needed to build the target itself but should not be inherited by ordinary consumers.

A PUBLIC requirement is needed both by the target and by consumers because the target’s public interface depends on it.

An INTERFACE requirement is consumed by downstream targets but is not used to compile the target’s own sources.

## Why public headers force propagation

Suppose library EngineMath has a public header that includes ThirdParty/Vec.hpp. A consumer compiling only its own source plus EngineMath’s public include path still needs the third-party include directory. Marking that dependency PRIVATE may let EngineMath build while every independent consumer fails.

The same reasoning applies to compile features, preprocessor definitions, generated headers and transitive link dependencies whose symbols appear in the public ABI.

The test should therefore be from the consumer’s perspective: can a tiny program include the advertised public header and link using only the public target?

## Build graph versus include graph

A textual include graph and a target dependency graph answer different questions. A source file can include a header without the build graph correctly propagating its location. Conversely, linking to a library can be required even when no source file directly includes one of its headers.

Imported targets and namespaced targets are preferable to global compiler/linker flags because the dependency becomes attached to the thing that owns it.

## Cycles and architecture

A cycle in target dependencies is often an architecture smell. Static-library cycles can also produce linker-order problems. Dependency inversion, interface targets or extracting a lower-level common module can make the ownership direction explicit.

## Evidence

A strong build-interface test creates a minimal consumer target in a separate directory, links only the exported target, and compiles with no accidental include directories from the main project. Installation/export tests should repeat this against the installed package so the build tree cannot hide missing usage requirements.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#The build graph becomes another interface]].

Related: [[C++ modules and reachability]], [[ABI and object layout]], [[Interface contracts and publication]].
