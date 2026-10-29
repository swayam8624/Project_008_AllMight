---
tags:
  - concept
aliases: []
---

# Hardware depth-buffer linearization

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

Perspective projection maps axial distance nonlinearly into NDC depth. For a declared projection depth row $q=-A+B/d$, solve $d=B/(q+A)$; finite conventional, finite reversed, infinite reversed, and OpenGL-style conventions have different coefficients or NDC conversions. The depth sample must first be interpreted in the same range and direction used by the projection matrix. This is why a single generic formula without an API and projection contract is unsafe. Verify near and far endpoints, test round trips over a range of distances, and handle an infinite-far endpoint such as reversed depth $q=0$ explicitly. Near/far endpoint checks catch sign mistakes immediately; interior round trips then test scaling and finite precision.

## Prerequisites

- [[Camera view and clip-space pipeline]]
- [[Axial depth and Euclidean range]]
- [[Depth buffering and visibility order]]

## Used by

- [[Position reconstruction from depth]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover geometry from depth and construct screen-space rays|Recover geometry from depth and construct screen-space rays]]

## Source coverage

Supplied lesson parts: 1079.

## Future keywords

standard depth, reversed-Z, infinite far plane
