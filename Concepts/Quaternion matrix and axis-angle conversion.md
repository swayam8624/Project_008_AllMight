---
tags: [concept, phase-7]
---

# Quaternion matrix and axis-angle conversion

**Curriculum depth:** Introduced · **First seen:** Phase7 · **Latest development:** Phase7

## Definition and mechanism

A unit axis and angle give q=(cos(theta/2),axis sin(theta/2)). A quaternion rotation matrix is assembled by rotating basis axes. Converting back uses trace or largest-diagonal branches and assumes a proper orthogonal input; scale, shear and reflection require separate treatment.

**Reconstruction cue:** Near a half turn, avoid dividing by a scalar component that approaches zero.

## Prerequisites

- [[Quaternion rotation and double cover]]
- [[Matrices as linear maps and basis images]]

## Used by

- [[Euler angles and gimbal singularity]]
- [[Transform hierarchies TRS and shear]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]
