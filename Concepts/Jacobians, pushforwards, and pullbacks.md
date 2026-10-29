---
tags:
  - concept
aliases:
  - "Jacobian matrix"
  - "pushforward"
  - "pullback"
  - "JVP"
  - "VJP"
---

# Jacobians, pushforwards, and pullbacks

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

The Jacobian is the matrix of first derivatives of a vector-valued function. It maps a small input displacement to a first-order output displacement, a forward sensitivity often called a pushforward or Jacobian-vector product. Gradients transform in the reverse direction by the transpose Jacobian, the pullback used in reverse-mode differentiation. These operations differ because vectors and covectors have different transformation rules. Jacobians compose by matrix multiplication and their determinants describe local volume changes. A Jacobian is local: it does not replace a nonlinear transformation globally. At singular points it can lose rank or become ill-conditioned, affecting inverse sensitivity, uncertainty propagation, and optimizer steps. State coordinate ordering and column-vector conventions before interpreting signs or multiplication order.

## Prerequisites

- [[Derivative as a local linear map]]
- [[Matrix multiplication and composition order]]
- [[Probability density and change of measure]]

## Used by

- [[Automatic differentiation and backpropagation]]
- [[Hessians and local curvature]]
- [[Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 822 (T04 · M050), 823 (T04 · M050), 824 (T04 · M050), 825 (T04 · M050).
