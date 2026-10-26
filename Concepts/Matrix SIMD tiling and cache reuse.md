---
tags: [concept, phase-7]
---

# Matrix SIMD tiling and cache reuse

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

Loop ordering and tiling change memory traffic and reuse while preserving the exact scalar algebra. SIMD maps operations to multiple lanes; reduction order and FMA can change rounding. Shape, stride, aliasing and target hardware determine whether an optimization helps.

**Reconstruction cue:** The i-k-j order reuses A(i,k) across a contiguous output row in row-major arrays.

## Prerequisites

- [[Matrix storage indexing and strides]]
- [[Floating-point reassociation]]

## Used by

- [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Phase 7 teaching and laboratory]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
