---
tags:
  - concept
aliases:
  - "differential"
  - "tangent map"
  - "first derivative"
---

# Derivative as a local linear map

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

The derivative measures first-order change. In one variable it is the limit of the difference quotient; in several variables it is the linear map that best predicts small output changes near a point. For a scalar function, this local model is the tangent line. For a vector-valued function, it is a matrix acting on an input perturbation. The derivative is local: its approximation error becomes small relative to the perturbation as the step shrinks, but the linear model need not remain accurate over a large region. This view unifies instantaneous velocity, gradients, Jacobians, and optimization directions. It also gives a practical check: compare a small finite perturbation with the derivative's predicted change, while remembering that floating-point rounding and curvature limit how small or large the perturbation should be.

## Prerequisites

- No earlier concept node is required in this planned path.

## Used by

- [[Automatic differentiation and backpropagation]]
- [[Definite integrals and the fundamental theorem]]
- [[Directional and total derivatives]]
- [[Finite differences and step-size balance]]
- [[Jacobians, pushforwards, and pullbacks]]
- [[Partial derivatives and gradients]]
- [[Taylor models and approximation error]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 805 (T04 · M049), 806 (T04 · M049), 807 (T04 · M049).
