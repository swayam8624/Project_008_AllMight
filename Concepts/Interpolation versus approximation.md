---
tags:
  - concept
aliases: []
---

# Interpolation versus approximation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Interpolation reproduces every prescribed sample exactly, while approximation minimizes or controls an error criterion and may miss the observations. The distinction matters when data are noisy: a high-degree interpolant can fit measurement noise and oscillate between samples, while a lower-complexity approximation can better represent the underlying trend. In texture reconstruction, the samples themselves often define the represented signal, so interpolation is an appropriate reconstruction rule. In calibration, observations have uncertainty, so fitting a model may be more defensible. Neither method is automatically superior; choose based on the meaning of the data, the error measure, and whether evaluation stays within or extrapolates beyond the sampled domain.

## Prerequisites

- [[Taylor models and approximation error]]
- [[Barycentric coordinates and interpolation]]

## Used by

- [[Tensor-product interpolation]]
- [[Cubic Hermite interpolation]]
- [[Bézier curves and De Casteljau evaluation]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Reconstructing values and curves between samples|Reconstructing values and curves between samples]]

## Source coverage

Supplied lesson parts: 976.

## Future keywords

interpolation nodes, extrapolation, Runge phenomenon
