---
tags: [concept, phase-7]
---

# Coordinate frames and change of basis

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

A frame gives an origin and independent axes relative to another frame. Its matrix maps local coordinates to that parent description. Changing description uses the inverse destination frame followed by the source frame; active motion and passive coordinate change answer different questions.

**Reconstruction cue:** Coordinates from frame A to frame B use inverse(F_B) times F_A.

## Prerequisites

- [[Linear affine and projective transformations]]
- [[Matrix inversion pivoting and residuals]]

## Used by

- [[Camera view and clip-space pipeline]]
- [[Transform hierarchies TRS and shear]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
