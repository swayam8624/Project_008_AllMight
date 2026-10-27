---
tags:
  - concept
  - university-series
  - independent-series
---

# Triangle rasterization and coverage tests

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Rasterization converts triangle geometry into a finite set of image samples. A pixel’s ideal coverage is the area fraction inside the triangle; a center test approximates this integral with one binary answer and therefore creates jagged edges. A directed edge defines an inside half-plane, so three signed edge tests classify a point against a triangle. A consistent tie rule assigns samples exactly on shared edges to one triangle, preventing cracks and double coverage. Supersampling tests several locations per pixel and averages their coverage, trading cost for a better area estimate. Hardware often checks candidate samples in parallel within a bounding box, then rejects or accepts coarse blocks that are wholly outside or inside. Triangle coverage is also the basis for interpolating depth, color, normals, and texture coordinates.

## Prerequisites

- [[Barycentric coordinates and interpolation]]
- [[Sampling aliasing and reconstruction]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Depth buffering and visibility order]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A scene description becomes an image|A scene description becomes an image]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]
