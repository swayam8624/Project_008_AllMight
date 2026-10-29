---
tags:
  - concept
aliases: []
---

# Bézier curves and De Casteljau evaluation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A Bézier curve blends control points with Bernstein basis weights. The weights are nonnegative and sum to one on $[0,1]$, so the curve remains in the control points’ convex hull and reaches its first and last control points at the endpoints. De Casteljau’s repeated linear interpolation evaluates the curve using numerically straightforward convex combinations; the intermediate levels also reveal subdivision geometry. A cubic Bézier’s endpoint tangents depend on its first and last control-polygon edges, so moving a handle changes direction and scale. The convex-hull guarantee applies to the curve parameter interval; extrapolation outside it loses that containment property. Subdivide the curve before tessellation when screen-space curvature changes rapidly; parameter steps alone do not guarantee uniform pixel error.

## Prerequisites

- [[Interpolation versus approximation]]
- [[Spline bases and curve evaluation]]

## Used by


## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Reconstructing values and curves between samples|Reconstructing values and curves between samples]]

## Source coverage

Supplied lesson parts: 983.

## Future keywords

Bernstein basis, convex hull property, curve subdivision
