---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "IPC"
  - "Incremental Potential Contact"
  - "CCD"
  - "continuous collision detection"
  - "collision barrier"
  - "barrier energy"
  - "anti-tunneling"
---

# Collision barriers and continuous detection

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A collision barrier turns a geometric safety condition into an optimization energy that rises sharply as bodies approach contact. Incremental Potential Contact (IPC) combines barrier terms for vertex-face and edge-edge separations with a time-step formulation that prevents interpenetration while a deformable object moves. Continuous collision detection (CCD) asks whether two moving primitives touch at any time during a step, not only at its endpoints; a pair can cross and separate between discrete samples. Robust CCD therefore needs conservative root bounds and careful handling of near-degenerate geometry. The barrier does not replace a correct collision set: missing pairs, invalid orientations, or a nonconservative CCD test can still allow tunneling. Very stiff barriers also make optimization ill-conditioned and require line search or step restrictions. IPC's guarantees depend on assumptions about the initial configuration, collision constraints, and numerical tolerances. The core distinction is discrete overlap testing versus continuous-in-time contact prevention.

## Prerequisites

- [[Constrained optimization and inverse kinematics]]
- [[Robust geometric predicates]]
- [[PDE fields and discrete Laplacians]]
- [[Manifold polygon meshes and boundary structure]]
- [[Norms lengths and induced distances]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]]

## Recall and next depth

- Explain how a barrier energy and continuous collision check prevent a crossing between time-step endpoints.
- Which numerical or geometric assumption could still allow tunneling?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]; [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Weighted points lead from affine geometry to collision shapes|Weighted points lead from affine geometry to collision shapes]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-16 - Subdivision Surfaces Part 1|P3-16]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-15 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 15: 3D Vision|SC-15]].
