---
tags:
  - concept
aliases:
  - "directional derivative"
  - "total differential"
---

# Directional and total derivatives

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

A directional derivative measures first-order change along a chosen direction. For a differentiable scalar field, it is the gradient paired with that direction; for a vector map, the Jacobian acts on the direction. The total derivative accounts for every input coordinate changing according to a path, combining partial derivatives with the rates of the coordinates themselves. A direction need not be normalized: normalization changes whether the derivative is per unit distance or per unit parameter. This distinction matters in signed-distance queries, motion paths, and parameterized camera projections. The total derivative clarifies the chain rule: if a state depends on time and an output depends on that state, the time rate is the local derivative composed with the state velocity. These are local statements, not global linearity claims.

## Prerequisites

- [[Partial derivatives and gradients]]
- [[Derivative as a local linear map]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Multivariable derivatives and sensitivity|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 819 (T04 · M050), 821 (T04 · M050).
