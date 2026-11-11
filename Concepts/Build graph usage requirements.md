# Build graph usage requirements

A usage requirement is build information needed by a target’s consumers, such as headers, language features or exposed dependencies. CMake PUBLIC, PRIVATE and INTERFACE describe requirement propagation, not C++ member access. A small independent consumer is a useful check that a public target exposes everything its advertised headers require.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#The build graph becomes another interface]].
