---
tags:
  - concept
aliases: []
---

# Screen-space ray generation and mouse picking

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

A screen ray begins with a pixel location mapped through the viewport to NDC, then through inverse projection and camera pose into a world-space origin and direction. For perspective cameras, rays usually share the eye origin; orthographic rays have parallel directions but different origins across the image. Mouse picking must use the same high-DPI scaling, viewport offset, origin, and Y convention as rendering. Near and far unprojection can define a ray segment, but a point at infinity or an infinite far plane needs an explicit direction construction. Picking then intersects that ray with scene geometry or uses an object-ID buffer.

## Prerequisites

- [[Camera view and clip-space pipeline]]
- [[Unit directions and safe normalization]]
- [[Visual angle and calibrated rays]]

## Used by

- No downstream concept node is added in this stage.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover geometry from depth and construct screen-space rays|Recover geometry from depth and construct screen-space rays]]

## Source coverage

Supplied lesson parts: 1082–1083.

## Future keywords

inverse projection, viewport mapping, picking ray
