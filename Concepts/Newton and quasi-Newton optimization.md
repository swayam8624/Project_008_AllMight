---
tags:
  - concept
aliases:
  - "Newton method"
  - "BFGS"
  - "quasi-Newton"
  - "trust region"
---

# Newton and quasi-Newton optimization

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Newton's method builds a local quadratic model of an objective from its gradient and Hessian, then solves for the model's stationary point. Near a well-conditioned solution with suitable smoothness and curvature, it can converge faster than gradient descent. Away from that regime, the Hessian may be indefinite, poorly scaled, or a poor model over the proposed step. Line searches and trust regions control step acceptance. Quasi-Newton methods such as BFGS estimate curvature from successive parameter and gradient changes, avoiding repeated exact Hessian construction. Their guarantees depend on assumptions and update safeguards. Optimization should monitor objective change, gradient norm, step size, constraints, and numerical failures; a small step alone can indicate stagnation rather than success. The method's result is a local solution unless global structure proves more.

## Prerequisites

- [[Hessians and local curvature]]
- [[Constrained optimization and inverse kinematics]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 867 (T02 · M053), 868 (T02 · M053), 869 (T02 · M053), 870 (T02 · M053), 871 (T02 · M053), 872 (T02 · M053), 873 (T02 · M053).
