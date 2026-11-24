# Pass 11 — Original-source custody: scene frames and asset instances

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

This pass adds a new reader chapter and a small scene-forest reference implementation. It does not replace, shorten or edit the canonical continuous notes, derived reading copies, legacy typesetting stage or published Books 01–02.

## Inspected original source identities

- `Notes/Chapter Edition/02 - Graphics Vision and XR/41 - Per-Vertex and Per-Corner Mesh Attributes.md`, Git blob `ca921d2747a6321fb9bebcde97edb086bb74edb7`. Its header declares Graphics master source lines 95949–99206 and source-span SHA256 `c8d8b33f6c039e15153ca041dae1dd9fdbdecdebf8e8bea1c5148b831d7a5ad9`. Directly located reading-copy lines approximately 963–1026 contain parent-child TRS composition and matrix storage conversion; 1578–1617 describe skeleton hierarchy and cycle checking; 1831 and 2496 identify inverse-transpose normal handling; 2114–2118 give parent/local global evaluation; 2520–2526 describe scene validation; 2679–2881 discuss entity/world transforms and animated joint transforms. These are targeted passages, not a completed integration of all 3277 lines.
- `Notes/Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging.md`, Git blob `957f1b0ddd3c620cf28e5289b606687c8f32d71c`. Header-declared Computing master source range 93037–95790 and SHA256 `ff9fb23aab9ebc413a2ce9ee1acf22cb01bdfc4a2c0af26d14b3f670299495fb`. Located passages at reading-copy lines 456, 1544 and 2465 identify model/hierarchy and units as separate fault boundaries, world-pose inversion, and separate point/tangent/normal transformation laws.
- Existing Book03 C++23 `geometry3d.hpp` blob `b386703e351ccb6bf8730754949f2683903628e7` and `mesh_topology.hpp` blob `0a4e74f8df4ad6ae2052cd473685abb173c729c3` provide Point3, Vector3, Mesh3 and existing validation semantics. The new header reuses those APIs.

The hashes in original chapter headers were recorded, not independently recomputed against the complete master. Full original-source reconciliation remains necessary.

## Coverage map

| Original subject | P11 reader and executable | Remaining advanced coverage |
|---|---|---|
| Parent-child frame composition and TRS order | Column linear maps, affine composition, explicit parent forest, known coordinate witnesses | Full glTF parsing, animation channels and imported node semantics |
| Shared geometry identity | Five placements referencing one local Mesh3, independent node/asset indices | Generational handles, asset lifetime, material overrides, instanced GPU draw scheduling |
| Acyclic hierarchy | Iterative Kahn ordering, invalid reference and cycle rejection | General graph/DAG semantics, concurrent transform updates, dirty propagation |
| World-to-local transform | Full affine inverse, preserve-local or preserve-world reparenting, rollback on failed validation | Exact condition diagnostics, robust decomposition and animation editing |
| Normal transformation | Inverse transpose, perpendicularity check, reflection determinant and culling distinction | Full shading/skin deformation pipeline, GPU front-face parity, arbitrary numerical certification |
| Transform-chain diagnostics | Source-derived instance origin, bounds, determinant, SVG and CSV | Camera pose/view/projection, clip-space and screen-space evidence |

No automatic CAD certification, animation/skinning, complete glTF implementation, or GPU performance result is claimed. The new tree is a simple planar three-triangle silhouette, not a production botanical asset. Native compilation and runtime acceptance remain separate.

**Next:** Chapter 12, Rotations Without Headaches, with SO(3), axis-angle, Euler-order conventions, unit quaternions and interpolation.
