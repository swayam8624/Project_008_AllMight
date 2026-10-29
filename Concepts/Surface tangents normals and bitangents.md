---
tags:
  - concept
  - phase-6
---

# Surface tangents normals and bitangents

**Curriculum depth:** Introduced · **First seen:** Phase6 · **Latest development:** Phase6

## Definition and mechanism

Tangents span local surface movement; normals measure perpendicularity; bitangents supply the second tangent axis. Texture derivatives carry physical stretch. Cleaned TBN frames need normalization, degeneracy checks and a mirrored-UV sign convention.

Panel texture derivatives are $(4,0,0)$ and $(0,3,0)$ before normalization. A collapsed UV determinant cannot uniquely determine texture directions.

## Prerequisites

- [[Cross products and oriented area]] — Oriented area supplies face normals, frame signs and geometric determinants.
- [[Unit directions and safe normalization]] — Validated finite unit directions establish the preconditions for simplified geometry formulas.
- [[Projection rejection and scalar components]] — Removing an unwanted component creates residuals, tangent axes and mirror decompositions.

## Used by

- [[Orthonormal frames and Gram-Schmidt]] — Builds on this mechanism in the connected geometry story.

## Recall and next depth

Derive the edge/UV equations and retain the handedness sign while orthogonalizing. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Surface movement becomes a local frame|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]
