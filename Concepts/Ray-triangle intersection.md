---
tags:
  - concept
  - university-series
  - independent-series
---

# Ray-triangle intersection

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A ray is a parametric half-line $r(t)=o+td$, where $o$ is its origin, $d$ is its direction, and only $t\ge0$ lies ahead of the ray. To intersect a triangle, first intersect the ray with its plane, then express the hit point in barycentric coordinates and accept it only if the weights lie inside the triangle. An equivalent determinant test solves for ray distance and two barycentric coordinates together. The hit record should state whether the result is nearest, any, or all intersections; those are different query contracts. Parallel or nearly parallel rays, degenerate triangles, and hits near edges require care because small floating-point errors can change classification. Scale-aware tolerances and consistent orientation predicates reduce cracks and self-intersection artifacts.

## Prerequisites

- [[Barycentric coordinates and interpolation]]
- [[Homogeneous coordinates and perspective division]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Bounding volume hierarchies]]

- [[Radiance fields and Gaussian splatting]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Visibility queries and ray intersections|Visibility queries and ray intersections]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Acceleration structures and robust queries|Acceleration structures and robust queries]]
