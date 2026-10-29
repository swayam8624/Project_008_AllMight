---
tags:
  - concept
  - phase-6
---

# Gram matrices and coordinate metrics

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

A Gram matrix stores pairwise basis inner products. Coordinate vectors measure physical dot products through $c^\mathsf TGd$. It is positive semidefinite, and positive definite for independent generators. It records metric geometry but not orientation.

The basis $(1,0),(1,1)$ has $G=\begin{pmatrix}1&1\\1&2\end{pmatrix}$. Coordinates $(1,2)$ give squared length $13$, not $5$.

## Prerequisites

- [[Basis coordinates and dimension]] — An ordered basis fixes coordinate meaning and the space whose geometry is measured.
- [[Dot products and orthogonality]] — Inner-product measurements supply the perpendicularity and alignment constraints.

## Used by

- [[Orthonormal frames and Gram-Schmidt]] — Builds on this mechanism in the connected geometry story.
- [[Barycentric coordinates and interpolation]] — Builds on this mechanism in the connected geometry story.
- [[Symmetric positive-definite matrices and Cholesky]] — Extends this foundation into linear-systems and decomposition analysis.
## Recall and next depth

Explain the residual equations for span projection and why normal equations can worsen conditioning. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A Gram matrix remembers the geometry of a slanted basis|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
