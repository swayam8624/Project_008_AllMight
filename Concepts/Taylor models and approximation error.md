---
tags:
  - concept
aliases:
  - "Taylor polynomial"
  - "truncation error"
  - "linearization error"
---

# Taylor models and approximation error

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

A Taylor model approximates a smooth function near a reference point using its value, derivatives, and higher-order terms. The first-order model is the derivative's linear map; the second-order model adds curvature through the Hessian. A remainder term describes what the finite truncation omits, so local accuracy depends on step size and higher derivatives over the region. Big-O notation gives an asymptotic scale, while a concrete error bound needs assumptions such as bounded derivatives. Taylor models power local optimization, camera Jacobian updates, uncertainty propagation, and numerical error analysis. They are not reliable far from the expansion point merely because the polynomial is easy to evaluate. When the input distribution is broad or crosses a singularity, first-order covariance propagation can miss bias and tail behavior; validate or replace the local model.

## Prerequisites

- [[Derivative as a local linear map]]
- [[Hessians and local curvature]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

- [[Discretization, truncation, and roundoff]]
- [[Interpolation versus approximation]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 828 (T04 · M050), 829 (T04 · M050), 830 (T04 · M050), 831 (T04 · M050).
