---
tags:
  - concept
aliases:
  - "Gauss-Newton"
  - "LM"
  - "damped least squares"
---

# Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Nonlinear least squares minimizes one half the squared norm of a residual vector. Its gradient and Hessian involve the residual Jacobian; the exact Hessian also contains second derivatives of each residual. Gauss–Newton drops those latter terms, producing a positive-semidefinite normal matrix and an efficient local step when residuals are small or the model is close to linear. Levenberg–Marquardt adds damping to control ill-conditioned or overly aggressive steps, blending Gauss–Newton with gradient descent. These algorithms are useful for camera calibration, bundle adjustment, and geometric fitting. Residual scaling, parameter units, rank deficiency, and gauge freedoms influence the solution. Normal equations can square the condition number, so QR or other structured solves may be safer. A small residual does not prove parameters are identifiable or physically correct.

## Prerequisites

- [[Jacobians, pushforwards, and pullbacks]]
- [[Hessians and local curvature]]
- [[Symmetric positive-definite matrices and Cholesky]]
- [[Constrained optimization and inverse kinematics]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 877 (T02 · M053), 878 (T02 · M053), 879 (T02 · M053), 880 (T02 · M053).
