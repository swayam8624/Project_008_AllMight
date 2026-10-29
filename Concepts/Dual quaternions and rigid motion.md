---
tags: [concept, phase-7]
---

# Dual quaternions and rigid motion

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

A rigid dual quaternion consists of a unit real quaternion and a dual quaternion part satisfying real dot dual equals zero. Encoding dual=(0,t)real/2 combines rotation and translation; composition follows the Hamilton convention. It does not represent arbitrary scale or shear.

**Reconstruction cue:** Recover translation with twice dual times conjugate(real); normalization must preserve the coupled constraint.

## Prerequisites

- [[Quaternion algebra and Hamilton product]]
- [[Linear affine and projective transformations]]

## Used by

- [[Transform hierarchies TRS and shear]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
