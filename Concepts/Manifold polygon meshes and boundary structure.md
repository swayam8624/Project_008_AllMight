---
tags:
  - concept
  - university-series
  - independent-series
---

# Manifold polygon meshes and boundary structure

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A polygon mesh is a combinatorial surface made of vertices, edges, and faces plus an embedding that assigns each vertex a position. Manifoldness is a local connectivity condition: an interior edge belongs to two faces, and the faces around each interior vertex form one cycle or fan. At a boundary, an edge belongs to one face and the vertex neighborhood forms one open chain; each boundary component closes into a loop. A bow-tie vertex with two disconnected fans is nonmanifold even if every face is an ordinary triangle. These conditions let algorithms define an unambiguous local tangent neighborhood, traversal, and surface orientation. Connectivity validity does not guarantee good geometry: valid topology can still self-intersect, collapse to a point, or contain poor skinny triangles. Keep topological and geometric quality checks separate.

## Prerequisites

- [[Coordinate frames and change of basis]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Half-edge mesh connectivity]]
- [[Quadric error mesh simplification]]
- [[Bounding volume hierarchies]]

- [[Collision barriers and continuous detection]]
- [[Local rigidity and mesh orientation fields]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surfaces, patches, topology, and parameterization|Surfaces, patches, topology, and parameterization]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]]
