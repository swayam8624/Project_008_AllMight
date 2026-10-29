---
tags:
  - concept
aliases:
  - "integration rule"
  - "trapezoid rule"
  - "Simpson quadrature"
---

# Numerical quadrature and error

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Quadrature approximates a definite integral from finitely many function evaluations. Left, right, and midpoint rectangle rules use constant local models; the trapezoid rule uses linear interpolation; Simpson's rule integrates a local quadratic fit over paired intervals. Their errors depend on smoothness, interval size, and rule-specific derivative bounds. Refining the mesh usually improves accuracy for smooth functions, but increases computation and may expose floating-point accumulation error. Adaptive quadrature places more samples where the integrand changes rapidly, provided its error estimator is trustworthy. Quadrature differs from Monte Carlo: deterministic rules have dimension-dependent behavior, while random sampling is attractive in high dimensions but has statistical uncertainty. Report the interval, nodes, weights, stopping tolerance, and error-control method so numerical results can be reproduced.

## Prerequisites

- [[Definite integrals and the fundamental theorem]]
- [[Absolute and relative error]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

- [[Convergence order and Richardson extrapolation]]
- [[Discretization, truncation, and roundoff]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Integration and quadrature|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 835 (T03 · M051), 836 (T03 · M051), 837 (T03 · M051).
