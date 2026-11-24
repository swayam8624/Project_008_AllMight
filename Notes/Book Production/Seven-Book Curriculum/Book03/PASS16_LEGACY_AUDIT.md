# Pass 16 — Original-source custody: vertex processing and varying interpolation

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

The reader and native reference are additive. Original continuous notes, derived reading copies, legacy typesetting material, Books 01–02 and distribution paths remain untouched.

## Inspected original source identities

- `Notes/Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling.md`, blob `ef481955be60941713582f1e5600191498f4bd8d`. Located reading-copy line 728 connects homogeneous w to varying interpolation; lines 786–812 identify screen-affine interpolation error and projective correction; line 2706 identifies vertex shader clip-space output and subsequent divide and viewport. Prior P14 audit records the original master metadata.
- `Notes/Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md`, blob `ca921d2747a6321fb9bebcde97edb086bb74edb7`. Directly located opening per-corner attribute discussion at line 14 and vertex shader skinning discussion at line 2422. P10–12 preserve further source-span metadata and outstanding animation/skinning.
- Actual existing APIs inspected: `asset_import.hpp` blob `2a5c1eeb1363b065eac4309021906670b8c86a60`, `projection_pipeline.hpp` blob `17a1f9effa42c5f15d95f9b06813a5a7a591eec9`, `scene_forest.hpp` blob `5a0d29d51ec730b3b8793dc3c40e0ff5d98f875e`, `mesh_topology.hpp` blob `0a4e74f8df4ad6ae2052cd473685abb173c729c3`.

## Coverage and remaining obligations

P16 derives indexed attribute identity, byte addressing and format boundaries, draw-state-scoped evaluation, model/view/projection traces, inverse-transpose normal handling, raw clip payload preservation, reciprocal-w smooth varying interpolation, screen-linear NDC depth, and flat/noperspective distinctions. It directly reuses the P10 rendering mesh and P11–14 affine and camera contracts.

The reference consumes decoded OBJ tuples, not a raw GPU vertex-buffer format. It does not implement clipping, raster sample coverage, flat interpolation qualifiers, multisampling, skinning, vertex pulling, shader compilation, or actual hardware vertex caching. Those original topics remain preserved and require later integration. This is a targeted source mapping, not complete source-unit reconciliation.

Next: Chapter 17 — Shapes That Refuse to Leave the Window.
