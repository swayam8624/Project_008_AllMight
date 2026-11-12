# Archetype ECS

An archetype ECS groups entities by an exact component-type signature. If Position and Velocity are present, that entity belongs in a chunk whose columns contain Position and Velocity values for the same row. If Velocity is later removed, the entity migrates to a different archetype. This differs from a sparse-set ECS, where each component type owns an independent dense pool.

The appeal is iteration density. A system that requires Position + Velocity can iterate only matching archetype chunks and then walk compact columns with predictable memory access. The cost moves into structural change: adding or removing a component is no longer a metadata toggle; it is a migration between storage layouts.

## Signature and chunk model

Represent an archetype signature as a stable set or bitset of component type identifiers. A chunk for signature S owns:

- a dense entity-identity column;
- one column per component in S;
- capacity and live-row count;
- optional alignment/padding metadata;
- whatever ownership is required for non-trivial component lifetimes.

All columns must agree on row meaning. Row i in the entity column and row i in every component column refer to the same logical entity. That is an invariant, not a convention.

A system query for required set Q matches archetype S when Q is a subset of S. Optional/excluded components add more predicates, but they should not change the core idea: queries select storage groups, then iterate rows.

## Migration

Suppose entity e moves from signature {Position} to {Position, Velocity}. A safe conceptual sequence is:

1. Validate that e is alive and that its location still names the source archetype and row.
2. Reserve or allocate destination capacity before mutating source storage.
3. Construct/copy/move retained components into the destination row.
4. Construct the newly added component.
5. Publish the new entity-to-location mapping.
6. Remove the old source row.
7. If removal uses swap-and-pop, repair the location of the source entity that moved into the vacated row.
8. Destroy any source component objects whose lifetime ends.

The exact order depends on exception and failure guarantees. “Destination first” helps, but it does not magically provide rollback for arbitrary component types. If a move constructor can throw after partially mutating a source object, the migration design needs a stronger policy: nothrow-movable components, copy fallback, staging storage, transactional indirection, or a documented weaker guarantee.

## Address invalidation

Archetype migration frequently invalidates raw component addresses because the entity moves to a different chunk and source/destination vectors can reallocate. APIs should therefore prefer short-lived borrows, handles, or query callbacks over storing arbitrary component pointers across structural changes.

Even without migration, chunk compaction may move rows. An address remaining inside the same allocation does not prove it still refers to the same entity.

## Scheduling consequence

Archetypes make data access visible enough to build a scheduler. A system can declare reads and writes by component type. Two systems that only read the same component can run concurrently; a writer conflicts with any overlapping reader or writer unless a stronger partition proves disjoint data. Structural mutation is usually an even broader hazard because it can move storage while queries are iterating.

## Executable reconstruction

tools/labs/ecs_reconstruction_lab.cpp contains a deliberately small two-archetype world. It proves:

- generational identity survives index reuse;
- sparse membership checks full identity rather than index alone;
- swap-and-pop repairs reverse mappings;
- Position-only → Position+Velocity migration preserves values;
- removing a middle source row repairs the moved entity’s location.

The witness explicitly does not claim arbitrary-component exception safety, parallel structural mutation, production chunk packing, or KairoECS parity.

Read the connected engine narrative at [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].

Related: [[Sparse-set storage]], [[Object identity and generation handles]], [[Field reordering and AoS versus SoA]].
