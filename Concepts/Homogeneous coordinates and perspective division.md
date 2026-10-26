---
tags:
  - concept
  - phase-6
---

# Homogeneous coordinates and perspective division

**Curriculum depth:** Developed · **First seen:** Phase6 · **Latest development:** Phase7

## Definition and mechanism

Phase 7 derives the affine block matrix, distinguishes a singular affine map from an invertible frame, and traces model → view → homogeneous clip → NDC using a declared projection. [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#Figure M6 - Follow one point through named spaces|The worked figure]] keeps the divisor visible and checks the chosen near/far depth endpoints.

An affine homogeneous point has last coordinate one; a direction has zero. Nonzero common scale defines projective equivalence, while dehomogenization divides by a nonzero final component. Clip representatives and projective classes have related but distinct semantics.

$(2,4,6,2)$ and $(1,2,3,1)$ describe one finite projective point. NDC normalization standardizes ranges, not vector length.

## Prerequisites

- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.

## Used by
- [[Linear affine and projective transformations]]

Later curriculum sources can deepen this same node; no future coverage is marked complete.

## Recall and next depth

Explain why perspective division is nonlinear and why clipping must precede casual division. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#Homogeneous coordinates preserve the role of a location|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
