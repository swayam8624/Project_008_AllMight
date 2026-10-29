---
tags: [concept, phase-7]
---

# Quaternion rotation and double cover

**Curriculum depth:** Developed · **First seen:** Phase7 · **Latest development:** University courses

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

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]

## University course development

Extend the existing quaternion note with interpolation and sign handling in animation. A unit quaternion represents a 3D rotation, but $q$ and $-q$ encode the same physical orientation. If two endpoint quaternions have a negative dot product, negate one endpoint before interpolation so the path follows the shorter arc on the unit four-dimensional sphere. Linear interpolation followed by normalization is inexpensive and often adequate for close orientations; spherical linear interpolation travels at constant angular speed for widely separated rotations. Quaternion interpolation acts on orientation, while translation and scale need their own rules. This matters in camera paths and articulated characters: interpolating four components without handling the double cover can create an unnecessary long spin, while interpolating Euler angles can encounter a gimbal singularity. Validate unit length after operations subject to floating-point drift.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Coordinate frames, transformations, and pose|Coordinate frames, transformations, and pose]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Scene graphs and articulated geometry|Scene graphs and articulated geometry]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Animation curves and motion paths|Animation curves and motion paths]]
