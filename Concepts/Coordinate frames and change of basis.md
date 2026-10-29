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
- [[Manifold polygon meshes and boundary structure]]
- [[Constrained optimization and inverse kinematics]]

- [[Camera gauge freedoms and observability]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]

## Phase 11 camera development

A basis change re-expresses the same physical points and transforms; a reconstruction gauge allows different parameter assignments to produce identical measurements. Fixing one camera pose chooses a world origin and orientation, but adds no image evidence. Monocular scale remains free until a baseline, known length, or metric sensor anchors it. Do not mistake a coordinate convention for an empirical measurement. This distinction matters when defining bundle-adjustment priors and covariance, and assumes a rigid calibrated camera model. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
