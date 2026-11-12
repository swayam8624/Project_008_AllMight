# Archetype ECS

An ECS archetype groups entities that share a component-type set. Chunks provide compact column storage, while adding or removing a component can migrate an entity and invalidate addresses. Migration is a transaction only if its move/copy/construct/destruct order provides the claimed failure guarantee; destination-first allocation alone does not guarantee rollback.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].

## Mastery expansion — from signature to migration

An archetype groups entities that have the same component signature. The signature is the set of component types an entity owns; the archetype stores one dense column per component type plus entity identity. This is different from a sparse-set-per-component design: an archetype query can walk tightly packed columns for a matching signature, while adding or removing a component usually migrates the entity to another archetype.

A production layout needs explicit choices for chunk capacity, column alignment, entity-to-location lookup, destruction/move semantics, stable versus unstable row order, tags with no payload, shared/resources data and relationships. Migration is a transaction: allocate/locate destination row, move or construct retained/new components, update the entity location, destroy removed/source component instances, repair any row moved by compaction, and only then expose the new signature.

The dangerous cases are not the happy query loop. They are exceptions or failures during migration, non-trivial component lifetimes, stale entity locations after swap-and-pop, structural mutation while systems are iterating, hierarchy relationships whose targets die, and scheduling two systems whose declared access sets actually conflict.

A complete ECS witness should compare at least sparse-set and archetype designs on measured workloads: iteration-heavy homogeneous scenes, mutation-heavy scenes, sparse optional components and relationship-heavy worlds. Memory bytes per entity and structural-mutation cost matter as much as raw iteration throughput.

**Required next witness:** a generational entity table plus archetype migration/query benchmark with stale-handle, mutation-during-iteration and destruction tests.

