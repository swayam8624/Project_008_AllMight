---
tags: [concept, phase-7]
---

# Quaternion rotation and double cover

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

For a unit quaternion q, the sandwich q(0,v)conjugate(q) rotates v. q and minus q give the same physical rotation. Every nonzero quaternion also rotates without scale if its true inverse is used; substituting conjugate for inverse off the unit sphere scales by the squared norm.

**Reconstruction cue:** The scalar-first quaternion (sqrt(1/2),0,0,sqrt(1/2)) rotates the X axis to Y.

## Prerequisites

- [[Quaternion algebra and Hamilton product]]

## Used by

- [[Quaternion matrix and axis-angle conversion]]
- [[Quaternion interpolation and shortest paths]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
