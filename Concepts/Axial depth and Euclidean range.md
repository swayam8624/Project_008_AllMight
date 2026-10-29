---
tags:
  - concept
aliases: []
---

# Axial depth and Euclidean range

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

Axial depth measures distance along the camera’s forward axis; Euclidean range measures distance from the camera centre along the viewing ray. With a right-handed view frame looking along $-Z$, positive axial depth is $d=-z$, while range is $r=\sqrt{x^2+y^2+z^2}$. They agree only on the optical axis. For a unit ray direction $q$ with $q_z<0$, a range measurement converts to axial depth as $d=-rq_z$. A range sensor and a perspective depth buffer may therefore encode different quantities. Applying a depth-buffer formula directly to a range image distorts reconstructed geometry away from the image centre. At view point $(3,4,-12)$, axial depth is $12$ while range is $13$, a visible distinction off-axis.

## Prerequisites

- [[Camera view and clip-space pipeline]]
- [[Unit directions and safe normalization]]

## Used by

- [[Hardware depth-buffer linearization]]
- [[Position reconstruction from depth]]
- [[Off-axis stereo projection and disparity]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover geometry from depth and construct screen-space rays|Recover geometry from depth and construct screen-space rays]]

## Source coverage

Supplied lesson parts: 1078.

## Future keywords

range image, optical axis, axial distance
