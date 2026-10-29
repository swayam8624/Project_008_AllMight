---
tags:
  - concept
aliases:
  - "gradient vector"
  - "partial derivative"
---

# Partial derivatives and gradients

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

For a function of several inputs, a partial derivative varies one coordinate while holding the others fixed. Collecting these derivatives gives the gradient for a scalar-valued function. The gradient is a covector of first-order sensitivities: its pairing with a small displacement predicts the scalar change. Under the Euclidean inner product, the gradient vector points in the direction of steepest ascent, and its negative gives steepest local descent. Coordinate scaling changes derivative magnitudes and can therefore change optimization behavior. A partial derivative is not generally the full effect when inputs move together; that requires the total derivative or directional derivative. In graphics, gradients describe image sensitivity to geometry or material parameters, while their coordinates must be interpreted with the parameter units and basis used by the model.

## Prerequisites

- [[Function contracts: domains, codomains, and images]]
- [[Derivative as a local linear map]]

## Used by

- [[Directional and total derivatives]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Multivariable derivatives and sensitivity|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 816 (T04 · M050), 817 (T04 · M050), 818 (T04 · M050), 820 (T04 · M050).
