# Build graph usage requirements

A usage requirement is build information needed by a target’s consumers, such as headers, language features or exposed dependencies. CMake PUBLIC, PRIVATE and INTERFACE describe requirement propagation, not C++ member access. A small independent consumer is a useful check that a public target exposes everything its advertised headers require.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#The build graph becomes another interface]].

## Mastery expansion — build interfaces propagate contracts, not just paths

A build graph is a dependency graph whose edges can carry usage requirements: include/search paths, compile definitions, language features, link libraries, generated modules/headers and platform flags.

Private requirements affect only the target's own compilation. Public/interface requirements propagate because consumers need them to compile or link correctly. Over-propagation couples unrelated targets and can make platform flags or definitions leak across the graph.

Generated artifacts add ordering dependencies. The consumer cannot compile until the generator has run, and the incremental build must know the actual input set so a changed generator input triggers regeneration.

For C++20 modules the graph is even more explicit: interface units must be built before importers, and module-related compiler behavior belongs in the reproducible build contract.

**Required next witness:** a small multi-target CMake project showing private/public/interface propagation plus a generated file/module dependency whose ordering is intentionally broken and then repaired.

