---
tags:
  - concept
aliases: []
---

# Convergence order and Richardson extrapolation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

If an approximation has leading error $Q_h=Q^*+Ch^p+O(h^{p+1})$, halving the step reduces that term by about $2^p$. Comparing results at $h$, $h/2$, and $h/4$ estimates the observed order; Richardson extrapolation combines two resolutions to cancel the leading term when the assumed order is valid. The method is diagnostic, not magic: measurements must be in the asymptotic regime, the same mathematical quantity must be computed at each resolution, and roundoff or stochastic noise must not dominate. For Euler integration, first-order refinement predicts the leading error to halve. Record the sequence and residuals rather than reporting an order from one pair alone. Ratios near an expected power provide evidence, but one noisy ratio is not a convergence study.

## Prerequisites

- [[Discretization, truncation, and roundoff]]
- [[Finite differences and step-size balance]]
- [[Numerical quadrature and error]]

## Used by

- [[Numerical error budgets]]
- [[Reproducible numerical benchmarking]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Separate discretization, truncation, and roundoff; measure convergence|Separate discretization, truncation, and roundoff; measure convergence]]

## Source coverage

Supplied lesson parts: 992.

## Future keywords

mesh refinement, observed order, extrapolation
