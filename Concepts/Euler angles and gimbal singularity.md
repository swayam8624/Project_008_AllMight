---
tags: [concept, phase-7]
---

# Euler angles and gimbal singularity

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

Euler/Tait-Bryan angles are coordinates for rotation with a specified axis/order convention. Some orientations make two parameter directions dependent. Quaternions represent the orientation smoothly, but converting to singular Euler coordinates still requires a branch and cannot recover unique original angles.

**Reconstruction cue:** For Rz(yaw)Ry(pitch)Rx(roll) at pitch pi/2, only yaw minus roll is observable.

## Prerequisites

- [[Quaternion matrix and axis-angle conversion]]
- [[Matrix multiplication and composition order]]

## Used by

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Phase 7 teaching and laboratory]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
