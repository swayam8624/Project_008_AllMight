---
tags:
  - concept
aliases:
  - "finite-difference derivative"
  - "central difference"
  - "gradient check"
---

# Finite differences and step-size balance

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Finite differences estimate derivatives from sampled function values. Forward and backward differences use one-sided samples and have first-order truncation error; central differences use samples on both sides and usually have second-order truncation error for smooth functions. Making the step arbitrarily small is not always better: subtractive cancellation and evaluation noise grow as the step shrinks, while truncation error grows when it is too large. The useful step balances these effects and depends on scale, precision, and function noise. Finite differences are valuable for black-box sensitivities and gradient checks, but they are not automatic differentiation and can fail near discontinuities or branch changes. A numerical gradient check should compare directional derivatives, use scale-aware steps, and avoid interpreting one tolerance pass as proof that every derivative path is correct.

## Prerequisites

- [[Derivative as a local linear map]]
- [[Absolute and relative error]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

- [[Convergence order and Richardson extrapolation]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 847 (T03 · M052), 848 (T03 · M052), 849 (T03 · M052), 850 (T03 · M052), 851 (T03 · M052), 884 (T02 · M054).
