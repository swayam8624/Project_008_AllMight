---
tags:
  - concept
aliases: []
---

# Off-axis stereo projection and disparity

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

Stereo cameras use two eye centres separated by a baseline, while each projection frustum is shifted so a chosen zero-parallax plane aligns in the images. Parallel eye orientations with asymmetric frusta avoid the vertical mismatch and discomfort often introduced by toe-in cameras. For rectified pinhole cameras, disparity is approximately $\Delta x=f_xB/Z$, so inferred depth is $Z=f_xB/\Delta x$ under the calibration and rectification assumptions. Depth uncertainty grows roughly with $Z^2$ for fixed pixel disparity error. Distinguish interpupillary distance from lens and display-centre spacing; the rendered baseline is an optical/comfort design decision. Use consistent pixel units for focal length, baseline, and disparity before computing metric depth.

## Prerequisites

- [[Stereo correspondence and epipolar geometry]]
- [[Camera calibration and bundle adjustment]]
- [[Axial depth and Euclidean range]]

## Used by

- No downstream concept node is added in this stage.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Build projection and stereo geometry around real camera baselines|Build projection and stereo geometry around real camera baselines]]

## Source coverage

Supplied lesson parts: 1084–1088.

## Future keywords

zero-parallax plane, baseline, disparity uncertainty
