---
tags: [concept, phase-7]
---

# Linear affine and projective transformations

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

Linear maps preserve linear combinations and fix zero. An affine map adds translation and preserves affine combinations. Homogeneous coordinates encode affine maps linearly in one higher dimension; perspective division is nonlinear and needs a valid nonzero homogeneous denominator.

**Reconstruction cue:** The last row (0,0,0,1) distinguishes an ordinary column-vector affine matrix from a general projective matrix.

## Prerequisites

- [[Matrices as linear maps and basis images]]
- [[Homogeneous coordinates and perspective division]]

## Used by

- [[Coordinate frames and change of basis]]
- [[Dual quaternions and rigid motion]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
