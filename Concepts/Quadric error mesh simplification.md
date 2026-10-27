---
tags:
  - concept
  - university-series
  - independent-series
---

# Quadric error mesh simplification

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Quadric error simplification greedily collapses the edge with the least predicted geometric error. For each triangle plane with unit normal $n$ and offset $d$, form $v=(n_x,n_y,n_z,d)^T$ and $K=vv^T$. A homogeneous point $U=(x,y,z,1)^T$ has squared point-to-plane error $U^TKU$. Sum the matrices of incident planes at each vertex; an edge candidate combines endpoint matrices. Partitioning the combined quadratic gives $Q(x)=x^TAx+2b^Tx+c$, whose stationary point solves $Ax=-b$. If the system is singular or indefinite, test endpoints and midpoint and keep the least-cost candidate. After selecting a location, reject collapses that flip incident face normals. The metric is a local approximation, so topology preconditions and orientation checks remain necessary.

## Prerequisites

- [[Half-edge mesh connectivity]]
- [[Manifold polygon meshes and boundary structure]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]
