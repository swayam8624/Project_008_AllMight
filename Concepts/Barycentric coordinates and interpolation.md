---
tags:
  - concept
  - phase-6
---

# Barycentric coordinates and interpolation

**Curriculum depth:** Developed · **First seen:** Phase6 · **Latest development:** University courses

## Definition and mechanism

Barycentric weights reconstruct a triangle point as an affine combination. The weights sum to one; nonnegative weights describe points inside or on a nondegenerate triangle. Dot-system solves on off-plane inputs describe their projection, not proven surface membership.

The panel sensor $(1,1,0)$ has weights $(5/12,1/4,1/3)$ and corner temperatures $(20,40,50)$ interpolate to $35$.

## Prerequisites

- [[Gram matrices and coordinate metrics]] — Pairwise basis measurements define the systems and metric used in reconstruction.
- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.

## Used by

- [[Convex combinations and convex hulls]] — Builds on this mechanism in the connected geometry story.
- [[Triangle rasterization and coverage tests]]
- [[Ray-triangle intersection]]
- [[Stereo correspondence and epipolar geometry]]

- [[Homogeneous clipping and perspective interpolation]]
## Recall and next depth

Derive signed-area ratios; distinguish affine attribute interpolation from perspective-correct screen interpolation. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#The sensor needs coordinates inside the triangle|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]

## University course development

Extend the existing interpolation note with the geometric meanings that make triangle rasterization and camera calibration easier to reason about. For triangle vertices $p_0,p_1,p_2$, a point inside can be written $p=\lambda_0p_0+\lambda_1p_1+\lambda_2p_2$, where the weights sum to one and are nonnegative. Each weight is the area of the opposite subtriangle divided by the full triangle area; equivalently it is a signed edge-distance normalized by altitude. The weights therefore provide both a point-in-triangle test and attribute interpolation. They also reveal a limitation: screen-space weights do not linearly interpolate a 3D attribute after perspective projection. Interpolate reciprocal depth and attribute divided by depth, then divide the interpolated values to recover perspective-correct texture coordinates or colors.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Textures and surface detail|Textures and surface detail]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]]
