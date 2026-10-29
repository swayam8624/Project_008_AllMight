---
tags:
  - concept
aliases:
  - "product rule"
  - "quotient rule"
  - "chain rule"
---

# Differentiation rules and chain rule

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

Differentiation rules let us build derivatives from simpler functions. Linearity handles sums and constant factors; the product and quotient rules account for both factors changing; the chain rule handles nested mappings. For a composition, the derivative of the outer function is evaluated at the inner output and multiplied by the derivative of the inner function. In multiple dimensions, this becomes matrix multiplication of Jacobians in composition order. Elementary rules for powers, exponentials, logarithms, trigonometric functions, and inverse trigonometric functions follow from limits and implicit relations. These rules are the local sensitivity algebra used by automatic differentiation and gradient-based fitting. Each rule has a domain: quotient denominators must be nonzero, logarithms require positive arguments, and inverse trigonometric derivatives can become singular at boundary values.

## Prerequisites

- [[Function contracts: domains, codomains, and images]]
- [[Limits and continuity]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 808 (T04 · M049), 809 (T04 · M049), 810 (T04 · M049), 811 (T04 · M049), 812 (T04 · M049), 813 (T04 · M049), 814 (T04 · M049), 815 (T04 · M049).
