---
tags:
  - concept
aliases: []
---

# Position reconstruction from depth

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

Given a pixel and an axial depth value, reconstruct a view-space point by converting the pixel to NDC, forming its camera ray, and scaling that ray so its forward-axis component equals the measured depth. Alternatively, transform the corresponding homogeneous clip point through an inverse projection matrix and divide by its resulting homogeneous coordinate. To obtain world position, apply the inverse view transform. These routes must share the actual viewport, pixel-centre, NDC-depth, and projection conventions used to render the image. In large worlds, subtract camera-relative origins before reducing precision; converting an already-rounded coordinate cannot restore lost detail. In a camera-relative renderer, reconstruct near the camera origin before adding a large global offset.

## Prerequisites

- [[Axial depth and Euclidean range]]
- [[Hardware depth-buffer linearization]]
- [[Camera calibration and bundle adjustment]]
- [[Homogeneous coordinates and perspective division]]

## Used by

- [[Numerical error budgets]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover geometry from depth and construct screen-space rays|Recover geometry from depth and construct screen-space rays]]

## Source coverage

Supplied lesson parts: 1080–1081.

## Future keywords

unprojection, inverse projection, world reconstruction
