---
tags:
  - concept
  - university-series
  - independent-series
---

# PDE fields and discrete Laplacians

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A partial differential equation constrains a field through spatial and temporal derivatives. Laplace’s equation $\Delta u=0$ seeks a harmonic field whose value matches its neighborhood average; Poisson’s equation adds a source term. On a regular grid, finite differences produce the five-point stencil with four neighbor weights and a negative center weight. On a triangle mesh, cotangent weights encode angles and a mass matrix normalizes by vertex area. Boundary conditions supply essential data: Dirichlet fixes values, Neumann fixes normal flux, and Robin combines them. Neumann Laplace systems require zero net flux and a pinned value or mean because constants lie in the nullspace. Explicit heat updates have a stability-limited time step; implicit updates solve a sparse system. PDE algorithms separate field representation, spatial operator, boundary conditions, and time integration.

## Prerequisites

- [[Sampling aliasing and reconstruction]]
- [[Convolution and filter footprints]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Collision barriers and continuous detection]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#PDEs, fields, and discrete solvers|PDEs, fields, and discrete solvers]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]
