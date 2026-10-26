---
tags: [concept, phase-7]
---

# Camera view and clip-space pipeline

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

A camera pose maps camera coordinates to world coordinates; its inverse is the view map. Projection produces homogeneous clip coordinates, clipping applies the API-specific inequalities, and valid perspective division produces NDC. Storage order supplies none of these coordinate contracts.

**Reconstruction cue:** A standard minus-Z perspective example needs a point in front of the camera and a meaningful clip w before division.

## Prerequisites

- [[Coordinate frames and change of basis]]

## Used by

- [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Phase 7 teaching and laboratory]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
