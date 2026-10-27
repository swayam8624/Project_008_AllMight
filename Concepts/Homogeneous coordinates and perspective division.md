---
tags:
  - concept
  - phase-6
---

# Homogeneous coordinates and perspective division

**Curriculum depth:** Developed · **First seen:** Phase6 · **Latest development:** University courses

## Definition and mechanism

Phase 7 derives the affine block matrix, distinguishes a singular affine map from an invertible frame, and traces model → view → homogeneous clip → NDC using a declared projection. [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#Figure M6 - Follow one point through named spaces|The worked figure]] keeps the divisor visible and checks the chosen near/far depth endpoints.

An affine homogeneous point has last coordinate one; a direction has zero. Nonzero common scale defines projective equivalence, while dehomogenization divides by a nonzero final component. Clip representatives and projective classes have related but distinct semantics.

$(2,4,6,2)$ and $(1,2,3,1)$ describe one finite projective point. NDC normalization standardizes ranges, not vector length.

## Prerequisites

- [[Vectors coordinates and affine points]] — Point, displacement and coordinate roles determine which geometric operations are meaningful.

## Used by
- [[Linear affine and projective transformations]]

Later curriculum sources can deepen this same node; no future coverage is marked complete.
- [[Ray-triangle intersection]]
- [[Camera calibration and bundle adjustment]]

- [[Homogeneous clipping and perspective interpolation]]
## Recall and next depth

Explain why perspective division is nonlinear and why clipping must precede casual division. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#Homogeneous coordinates preserve the role of a location|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]

## University course development

Extend the existing account with the geometric reason for the final divide. A 3D point is stored as $(x,y,z,1)$; after multiplying by a projective matrix it becomes $(X,Y,Z,W)$. Dividing the first three components by $W$ maps the homogeneous representative to ordinary coordinates. Any nonzero scalar multiple of a homogeneous vector represents the same point, which is why this division is meaningful. For a perspective camera, $W$ is chosen to depend on depth, so apparent size shrinks with distance. A direction has homogeneous coordinate zero and is not translated like a point. Projection matrices encode the camera frustum before division; afterward, a viewport transform scales and shifts normalized coordinates to pixels and commonly flips the vertical axis because image rows increase downward. Keep clipping before division to avoid unstable behavior around $W=0$.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Coordinate frames, transformations, and pose|Coordinate frames, transformations, and pose]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]]
