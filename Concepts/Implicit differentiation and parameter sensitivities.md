---
tags:
  - concept
aliases:
  - "implicit differentiation"
  - "adjoint sensitivity"
  - "solution sensitivity"
---

# Implicit differentiation and parameter sensitivities

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Implicit differentiation obtains how a solution changes when it is defined by an equation rather than an explicit formula. If a state satisfies F(y,θ)=0 and the Jacobian with respect to y is invertible, differentiating gives dy/dθ=−(∂F/∂y)⁻¹(∂F/∂θ). This can differentiate equilibrium states, constrained optima, and roots without unrolling every solver iteration. For a converged optimizer, differentiating stationarity and constraint conditions yields sensitivity equations, but active-set changes, nonunique solutions, singular Jacobians, and imperfect convergence complicate the result. Continuous sensitivity equations propagate derivatives alongside an ODE; adjoint methods can reduce cost when there are many parameters and few scalar outputs. These are modeling tools with assumptions, not a guarantee that every algorithmic branch is differentiable.

## Prerequisites

- [[Automatic differentiation and backpropagation]]
- [[Constrained optimization and inverse kinematics]]
- [[ODE integration and stability]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 855 (T03 · M052), 899 (T02 · M054), 900 (T02 · M054).
