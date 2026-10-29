---
tags:
  - concept
  - university-series
  - independent-series
---

# Spline bases and curve evaluation

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A spline is a piecewise polynomial curve whose neighboring pieces join under explicit continuity constraints. A cubic Bézier segment uses four control points and Bernstein weights; the weights sum to one, so the curve lies in their convex hull. A B-spline uses local basis functions so moving one control point changes only nearby spans rather than the whole curve. Knots mark where polynomial pieces meet; repeated knots reduce continuity. Catmull–Rom curves interpolate their control points and can be convenient for motion paths, while Bézier curves expose endpoint tangents through their handles. To evaluate a curve, compute basis weights at parameter $t$ and take their weighted sum of control points. Position, first derivative, and parameterization speed matter separately: even a smooth path may speed up or slow down unless its parameter is adjusted.

## Prerequisites

- [[Basis coordinates and dimension]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Bézier curves and De Casteljau evaluation]]
- [[Cubic Hermite interpolation]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Curves as controllable functions|Curves as controllable functions]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Animation curves and motion paths|Animation curves and motion paths]]
