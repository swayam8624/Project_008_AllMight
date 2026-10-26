---
tags: [concept, phase-7]
---

# Quaternion interpolation and shortest paths

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

Quaternion interpolation must account for the double cover. Flip one endpoint when their dot is negative for the usual shortest rotational path, then use NLERP or SLERP. NLERP changes angular speed; SLERP follows a great-circle arc with a small-angle fallback.

**Reconstruction cue:** Interpolating q and minus q without sign handling can pass through a zero quaternion.

## Prerequisites

- [[Quaternion rotation and double cover]]

## Used by

- [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Phase 7 teaching and laboratory]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
