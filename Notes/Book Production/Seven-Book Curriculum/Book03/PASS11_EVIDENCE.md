# Book03 — Pass 11 evidence and acceptance

**Chapter:** 11 — The Same Tree in Five Worlds  
**Date:** 24 November 2026  
**Branch:** curriculum/book03-geometry-images-pass01

## Delivered source

The 5,762-word narrative chapter derives reusable geometry versus placement, column-basis affine maps, TRS ordering, parent-local to world composition, iterative topological evaluation, cyclic-input rejection, world-preserving reparenting, affine inverse, inverse-transpose normal transforms, reflection parity and world bounds. Its twelve reader problems and selected answers use numerical witnesses rather than relying on a visual preview.

New C++23 files under `Code Parts/tools/labs/book03_cpp23/`:

- `include/book03/scene_forest.hpp`: Linear3 column maps, SceneAffine point/tangent/normal application, scale-normalized inverse with a relative determinant rejection heuristic, shared mesh assets, SceneForest validation/evaluation, and reparenting with explicit world/local policy.
- `tests/scene_forest_tests.cpp`: independent TRS/RTS results, sheared affine round trips, nonuniform normal perpendicularity, negative determinant parity, singular rejection, five shared asset references, default bounds, parent-after-child storage ordering, invalid references, failed cyclic edit rollback, world-preserving reparent, and an iterative 5000-node chain.
- `src/scene_studio.cpp`: actual evaluated world poses and bounds in CSV, fifteen source-derived projected triangular faces in SVG, inspect and a terminal editor with move and parent commands.
- CMake targets book03_scene and book03_scene_tests; test registration book03_affine_scene_forest_contracts, the twelfth cumulative CTest. Dedicated CI source adds a fixture run and SVG/CSV/report assertions.

The test fixture stores one asset containing seven positions and three faces, five drawable instances and seven nodes. Tree A has world origin (-3,0,0); B (-1,0,0); C (3,0,0); D (5,0,0) with top y=3.25; E (0,0,-3) and negative determinant. Evaluation order is not node index order; the bounds test was corrected to locate tree A by stable node ID.

## Original figures

Five linked SVG plates: 40-one-asset-five-instances.svg, 41-transform-order-witness.svg, 42-scene-forest-two-identities.svg, 43-reparent-keep-world.svg, 44-normal-inverse-transpose.svg. Total authored figures: 44. The first is a conceptual placement gallery, not a literal world projection. The other plates use the chapter's numeric witnesses. The scene studio's SVG is an actual projection of evaluated positions, but is not a visibility or camera renderer.

## Deliberate limitations and acceptance status

The reference forest permits one parent per node and stable array indices while no nodes/assets are erased or reordered. It recomputes the entire scene, transforms all stored positions for bounds, and has no GPU draw batching, animation, concurrent updates, skinning or general glTF loader. The inverse uses a numerical threshold, not certified conditioning; singular maps may still place collapsed geometry but cannot supply a normal transform or an inverse parent map. Reparenting rollback covers failed validation, not arbitrary resource-exhaustion guarantees. Bounds are not outward-rounded floating intervals.

Native compiler execution, CTest and GitHub Actions success are **not yet observed**. Source registration and source-level audit are not equivalent to runtime proof. Full original-source reconciliation and print/EPUB composition remain unperformed. No publication files were generated or promoted.

See PASS11_LEGACY_AUDIT.md for exact original-source identities and scope. Next chapter: Rotations Without Headaches.
