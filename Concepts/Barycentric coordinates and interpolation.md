---
tags:
  - concept
  - phase-6
---

# Barycentric coordinates and interpolation

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Barycentric weights reconstruct a triangle point as an affine combination. The weights sum to one; nonnegative weights describe points inside or on a nondegenerate triangle. Dot-system solves on off-plane inputs describe their projection, not proven surface membership.

The panel sensor $(1,1,0)$ has weights $(5/12,1/4,1/3)$ and corner temperatures $(20,40,50)$ interpolate to $35$.

## Prerequisites

- [[Gram matrices and coordinate metrics]] — Pairwise basis measurements define the systems and metric used in reconstruction.
- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.

## Used by

- [[Convex combinations and convex hulls]] — Builds on this mechanism in the connected geometry story.

## Recall and next depth

Derive signed-area ratios; distinguish affine attribute interpolation from perspective-correct screen interpolation. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#The sensor needs coordinates inside the triangle|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
