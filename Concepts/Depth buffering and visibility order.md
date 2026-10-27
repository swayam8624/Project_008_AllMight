---
tags:
  - concept
  - university-series
  - independent-series
---

# Depth buffering and visibility order

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A depth buffer stores the nearest depth seen so far for each image sample. When rasterizing a fragment, compare its depth with the stored value; if it is closer, update both depth and color, otherwise discard it. This per-sample test resolves opaque visibility without sorting all triangles. Perspective projection maps view-space depth nonlinearly, so the near and far clipping planes affect how floating-point precision is distributed. If they span an unnecessarily huge ratio, nearly coplanar surfaces may quantize to equal or inconsistent depths and flicker as the camera moves; this is z-fighting. Alpha-blended surfaces need additional ordering or order-independent methods because color composition is not commutative. Depth establishes visibility, while transparency determines how multiple visible layers contribute.

## Prerequisites

- [[Triangle rasterization and coverage tests]]
- [[Camera view and clip-space pipeline]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Textures and surface detail|Textures and surface detail]]
