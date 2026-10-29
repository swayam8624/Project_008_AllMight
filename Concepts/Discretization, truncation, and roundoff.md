---
tags:
  - concept
aliases: []
---

# Discretization, truncation, and roundoff

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Discretization replaces a continuous object or domain with a finite grid; truncation omits terms or limits an iterative expansion; roundoff comes from finite-precision arithmetic. They have different causes and respond to different changes. Refining a spatial grid may reduce discretization error while increasing computation. Adding more terms can reduce truncation error. Increasing precision or reformulating arithmetic can reduce roundoff, though a poor algorithm may still dominate. A signed-distance field sampled too coarsely can lose a thin feature entirely, while an ODE’s Euler step has a local truncation error even under exact arithmetic. An error budget should distinguish these mechanisms instead of calling every disagreement “numerical error.”

## Prerequisites

- [[Accumulated rounding error]]
- [[Taylor models and approximation error]]
- [[Numerical quadrature and error]]

## Used by

- [[Convergence order and Richardson extrapolation]]
- [[Numerical error budgets]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Separate discretization, truncation, and roundoff; measure convergence|Separate discretization, truncation, and roundoff; measure convergence]]

## Source coverage

Supplied lesson parts: 989–991.

## Future keywords

grid resolution, truncation remainder, finite precision
