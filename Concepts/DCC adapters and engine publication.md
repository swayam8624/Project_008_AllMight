# DCC adapters and engine publication

A production pipeline should not let every DCC application define a different engine truth. Blender, Maya, Houdini and Nuke expose different object models, dependency systems and scripting APIs, but the engine-facing publication contract can remain host-neutral.

The useful separation is:

**Host adapter → normalized asset description → validation → deterministic build plan → staged publication → engine-side verification.**

The host adapter owns how to read the authoring application. The normalized core owns what the engine requires. Publication owns filesystem and identity guarantees. Engine verification proves that the published result can actually be consumed.

## Blender boundary

Blender’s Python API exposes application data through the bpy module, including data access, context, operators, types, utilities and path helpers. A production exporter should prefer direct data access for deterministic inspection and use context-sensitive operators only when their context requirements are understood.

An adapter should establish which evaluated scene state it exports, how collections and object visibility are interpreted, which transforms are baked, and which external dependencies are copied or referenced. Exporting the editable authoring graph and exporting evaluated geometry are different contracts.

Primary reference: https://docs.blender.org/api/main/index.html

## Maya boundary

Maya exposes both scripting commands and C++/Python APIs, with explicit dependency-graph and DAG plug-in concepts. A Maya adapter should distinguish scene hierarchy from dependency relationships and should make attribute/default/reference semantics explicit rather than flattening everything into names.

Primary reference: https://help.autodesk.com/view/MAYADEV/2026/ENU/

## Houdini boundary

Houdini’s Object Model exposes nodes and geometry through the hou package. SOP networks generate geometry, while ROP networks can describe render/output dependencies. Geometry references can also depend on node cook state, so an adapter must decide when data is evaluated and copied into the host-neutral representation.

SideFX documentation explicitly distinguishes node families such as SOP geometry, LOP/USD scene description, ROP output networks and TOP task workflows. A pipeline should use those semantics instead of treating every node graph as the same kind of data.

Primary references:
- https://www.sidefx.com/docs/houdini/hom/
- https://www.sidefx.com/docs/houdini/

## Nuke boundary

Nuke publication often involves image nodes, knobs, file paths and frame ranges rather than mesh topology. The engine-neutral contract should therefore represent sequence ranges and missing-frame policy explicitly. Frame range syntax can represent contiguous and stepped ranges, so “first and last frame exist” is not sufficient validation.

Primary reference: https://learn.foundry.com/nuke/content/getting_started/managing_scripts/defining_frame_ranges.html

## Host-neutral core

A normalized publication record should carry at least:

- logical asset identity;
- schema version;
- source provenance;
- normalized relative input/output paths;
- declared coordinate/unit/color-space conventions where relevant;
- dependencies;
- content identity or cryptographic digest;
- frame range or subresource inventory;
- tool/version information when reproducibility depends on it;
- deterministic diagnostics.

The core should not contain live bpy, Maya, hou or Nuke objects. Those are host-lifetime objects. Convert them into owned normalized data before asynchronous work or engine publication.

## Validation before mutation

A dry-run plan should be complete before writing outputs. Validate path traversal, duplicate output destinations, missing dependencies, unsupported schema versions, frame gaps and identity collisions first.

Then write to staging locations. Only after every staged artifact passes validation should publication become visible. On filesystems where atomic replacement is relied upon, that property needs an actual platform/filesystem test; a logical transaction model does not prove disk semantics.

## Executable witness

tools/labs/production_pipeline_contract_lab.cpp implements the host-neutral part of this design. It validates a portable relative-path grammar, rejects parent traversal, checks schema and SHA-256 string shape, produces a deterministic sorted publication plan, checks output collisions, models idempotent publication, and verifies contiguous frame sequences.

It deliberately does not claim real Blender/Maya/Houdini/Nuke execution, SHA-256 computation, symlink resolution, atomic rename behavior or render-farm integration.

Related: [[Atomic publication and durability]], [[Binary format contracts]], [[Interface contracts and publication]].
