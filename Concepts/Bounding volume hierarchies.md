---
tags:
  - concept
  - university-series
  - independent-series
---

# Bounding volume hierarchies

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A bounding volume hierarchy (BVH) organizes primitives into a tree of enclosing boxes. An internal node’s box contains the boxes of its children; a leaf stores a small set of triangles. To trace a ray, test the root box, recurse only into intersected children, and compare triangle hits in reached leaves. A box miss safely rejects every primitive beneath it, so the hierarchy reduces expensive exact tests when geometry is spatially coherent. Construction chooses how to group primitives and where to split; a surface-area heuristic estimates expected traversal work, while simpler median splits are faster to build. Dynamic scenes may refit boxes after deformation or rebuild the hierarchy when overlap grows. Correctness requires every primitive to remain enclosed; performance depends on tree quality and ray distribution.

## Prerequisites

- [[Ray-triangle intersection]]
- [[Manifold polygon meshes and boundary structure]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Acceleration structures and robust queries|Acceleration structures and robust queries]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Visibility queries and ray intersections|Visibility queries and ray intersections]]
