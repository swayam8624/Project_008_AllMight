# Archetype ECS

An ECS archetype groups entities that share a component-type set. Chunks provide compact column storage, while adding or removing a component can migrate an entity and invalidate addresses. Migration is a transaction only if its move/copy/construct/destruct order provides the claimed failure guarantee; destination-first allocation alone does not guarantee rollback.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].
