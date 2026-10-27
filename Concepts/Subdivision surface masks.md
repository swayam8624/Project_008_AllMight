---
tags:
  - concept
  - university-series
  - independent-series
---

# Subdivision surface masks

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Subdivision refines a control mesh and updates old and newly inserted vertices with local masks, producing a sequence that approaches a smooth limit surface. Loop subdivision targets triangle meshes: for an interior edge with endpoints $a,b$ and opposite vertices $c,d$, its new point is $3(a+b)/8+(c+d)/8$. An old vertex of degree $n$ becomes $(1-n\beta)S+\beta\sum_iP_i$, with the lecture’s simple weight $\beta=3/16$ for $n=3$ and $3/(8n)$ otherwise. Catmull–Clark targets polygon meshes: face points are face-vertex averages, interior edge points average endpoints and two adjacent face points, and old vertices use face-average, edge-midpoint-average, and original-position terms. Boundary rules differ from interior masks and must be stated. Extraordinary vertices have nonregular valence and need deliberate treatment.

## Prerequisites

- [[Half-edge mesh connectivity]]
- [[Convex combinations and convex hulls]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Local rigidity and mesh orientation fields]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]]
