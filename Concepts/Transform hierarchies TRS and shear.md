---
tags: [concept, phase-7]
---

# Transform hierarchies TRS and shear

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

A TRS transform stores translation, rotation and diagonal scale, with an explicit application order. General affine matrices are closed under composition; TRS without shear is not. Parent nonuniform scale and rotated children can create shear, so exact world composition may require an affine matrix. Dirty caches must invalidate all dependent results.

**Reconstruction cue:** Nonorthogonal columns in a composed linear map rule out a pure rotation-times-diagonal-scale representation.

## Prerequisites

- [[Coordinate frames and change of basis]]
- [[Quaternion matrix and axis-angle conversion]]
- [[Dual quaternions and rigid motion]]

## Used by

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Phase 7 teaching and laboratory]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
