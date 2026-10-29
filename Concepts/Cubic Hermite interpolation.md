---
tags:
  - concept
aliases: []
---

# Cubic Hermite interpolation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A cubic Hermite segment is determined by endpoint values and endpoint tangent data. With normalized $t\in[0,1]$, its four basis polynomials blend the two endpoint positions and two tangents. Tangents are derivatives with respect to the parameter, so physical-time derivatives must be scaled by segment duration: a velocity in units per second is not interchangeable with a tangent per normalized parameter. Hermite curves are useful when a camera path or animation must pass through known positions while exposing direct control over its entry and exit directions. The endpoint constraints determine one cubic polynomial; continuity between adjacent segments still requires compatible endpoint tangents.

## Prerequisites

- [[Interpolation versus approximation]]
- [[Spline bases and curve evaluation]]

## Used by

- No downstream concept node is added in this stage.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Reconstructing values and curves between samples|Reconstructing values and curves between samples]]

## Source coverage

Supplied lesson parts: 981.

## Future keywords

endpoint tangents, C1 continuity, parameter scaling
