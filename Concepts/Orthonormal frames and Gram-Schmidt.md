---
tags:
  - concept
  - phase-6
---

# Orthonormal frames and Gram-Schmidt

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

An orthonormal frame has unit perpendicular axes, so its Gram matrix is identity. Gram–Schmidt constructs axes by repeated rejection. Modified order uses the updated residual and changes rounding behavior. Frame existence, numerical reliability and output continuity are distinct questions.

For $(1,1,0)$ and $(1,0,0)$, rejection produces $(1/2,-1/2,0)$. A least-aligned helper axis avoids a near-parallel cross but can switch discontinuously.

## Prerequisites

- [[Surface tangents normals and bitangents]] — The surface tangent plane gives the axes that orthogonalization must preserve.
- [[Gram matrices and coordinate metrics]] — Pairwise basis measurements define the systems and metric used in reconstruction.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.

## Recall and next depth

Check reconstruction and orthogonality separately; know when a QR-based method is the next step. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#Orthonormal frames make reconstruction simple|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
