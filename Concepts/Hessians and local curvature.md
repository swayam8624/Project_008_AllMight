---
tags:
  - concept
aliases:
  - "second derivative matrix"
  - "curvature matrix"
---

# Hessians and local curvature

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

The Hessian collects second partial derivatives of a scalar objective. It describes how the gradient changes and supplies the quadratic term in a local Taylor model. For a smooth scalar field, the Hessian is symmetric under the usual equality-of-mixed-partials assumptions. Its eigenvectors indicate local curvature directions: positive eigenvalues give upward curvature, negative values downward curvature, and near-zero values weakly constrained directions. A positive-definite Hessian supports a strict local minimum in the unconstrained setting, while an indefinite Hessian signals a saddle-like local model. The Hessian is not the same as the curvature of an embedded geometric surface, though both involve second-order change. In optimization, exact Newton steps solve with the Hessian; practical methods may regularize or approximate it because conditioning, cost, or indefiniteness can make direct steps unreliable.

## Prerequisites

- [[Jacobians, pushforwards, and pullbacks]]
- [[Symmetric positive-definite matrices and Cholesky]]

## Used by

- [[Newton and quasi-Newton optimization]]
- [[Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt]]
- [[Taylor models and approximation error]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 826 (T04 · M050), 827 (T04 · M050).
