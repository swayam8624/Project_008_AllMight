---
tags:
  - concept
aliases:
  - "domain and codomain"
  - "mapping contract"
  - "image of a function"
---

# Function contracts: domains, codomains, and images

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

A function is a rule that assigns exactly one output to every allowed input. Its domain states which inputs are valid; its codomain states the declared output set; its image is the subset actually reached. These sets affect whether an inverse exists and whether a formula is defined at a boundary. For example, squaring maps the real line into nonnegative outputs but is not one-to-one until its domain is restricted to nonnegative values. In a camera map, division by depth makes zero depth invalid, while a positive-forward convention further restricts visible points. Treating these as contracts makes composition safer: the output of one function must lie in the next function's domain. A typed implementation can encode some, but not all, of these mathematical restrictions.

## Prerequisites

- No earlier concept node is required in this planned path.

## Used by

- [[Differentiation rules and chain rule]]
- [[Partial derivatives and gradients]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 801 (T04 · M049).
