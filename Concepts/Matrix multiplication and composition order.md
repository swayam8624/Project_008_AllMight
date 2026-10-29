---
tags: [concept, phase-7]
---

# Matrix multiplication and composition order

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase 9

## Definition and mechanism

The product AB applies B first for column-vector actions. Its entries sum A(i,k)B(k,j) over the shared dimension. Composition is associative over exact scalars and usually noncommutative; floating-point evaluation orders need not agree bitwise.

**Reconstruction cue:** Rotation followed by translation differs from translation followed by rotation.

## Prerequisites

- [[Matrices as linear maps and basis images]]

## Used by

- [[Matrix transpose symmetry and trace]]
- [[Euler angles and gimbal singularity]]
- [[Eigenvalues and eigenspaces]] — Extends this foundation into linear-systems and decomposition analysis.
- [[Singular value decomposition]] — Extends this foundation into linear-systems and decomposition analysis.
- [[Matrix functions]] — Extends this foundation into linear-systems and decomposition analysis.
- [[Gaussian elimination and pivoting]] — Extends this foundation into the Phase 8 dependency spine.

- [[Automatic differentiation and backpropagation]]
- [[Jacobians, pushforwards, and pullbacks]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · Phase 9 visual recall
