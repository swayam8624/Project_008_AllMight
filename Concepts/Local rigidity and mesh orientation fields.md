---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "ARAP"
  - "as-rigid-as-possible"
  - "local rigidity"
  - "orientation field"
  - "cross field"
  - "Instant Meshes"
  - "quad remeshing"
---

# Local rigidity and mesh orientation fields

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

As-Rigid-As-Possible (ARAP) deformation seeks vertex positions that change as little as possible from a preferred local rigid transform. For each vertex neighborhood, fit a rotation to the deformed edge vectors, then minimize residuals between transformed rest edges and current edges. Alternating between local rotations and a global position solve makes a nonlinear shape-editing problem tractable, but the result depends on constraints, initialization, mesh quality, and rotation fitting. Instant Meshes uses direction or orientation fields over a surface to guide a quad-dominant remeshing; field singularities encode where a globally consistent direction cannot exist. A mesh's half-edge connectivity, boundary structure, and orientation determine valid neighborhoods and field transport. Poor triangles, seams, boundary conditions, or inconsistent orientation can create artifacts. ARAP is a local preservation objective, not exact rigidity of the whole shape; a direction field guides topology but does not uniquely determine a good mesh without sizing and feature constraints.

## Prerequisites

- [[Half-edge mesh connectivity]]
- [[Manifold polygon meshes and boundary structure]]
- [[Subdivision surface masks]]
- [[Constrained optimization and inverse kinematics]]
- [[Cross products and oriented area]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]

## Recall and next depth

- Describe one local rotation/global solve step of ARAP and the role of a field singularity in remeshing.
- Why can low local deformation energy fail to ensure a globally rigid shape?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surfaces, patches, topology, and parameterization|Surfaces, patches, topology, and parameterization]]; [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Put the algorithms in a graphics context|Put the algorithms in a graphics context]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-16 - Subdivision Surfaces Part 1|P3-16]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-17 - Subdivision Surfaces Part 2|P3-17]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-15 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 15: 3D Vision|SC-15]].
