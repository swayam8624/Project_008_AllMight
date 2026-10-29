---
tags:
  - concept
aliases:
  - "epsilon-delta limit"
  - "pointwise continuity"
---

# Limits and continuity

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

A limit describes the value a function approaches as its input approaches a point, even when the function is undefined or assigned a different value there. Continuity at a point adds the requirement that the function value equal that limit. Epsilon-delta language makes this precise: every requested output tolerance can be met by choosing a sufficiently small input neighborhood. Continuity rules out finite jumps but does not guarantee a derivative; the absolute-value function is continuous at zero yet has a corner there. Limits expose removable singularities such as a cancelled factor, while one-sided limits reveal boundaries and discontinuities. In numerical software, local smoothness matters because derivative-based approximations rely on nearby inputs producing controlled output changes. Domain restrictions, branch conditions, and singular denominators must be checked before applying derivative formulas.

## Prerequisites

- No earlier concept node is required in this planned path.

## Used by

- [[Differentiation rules and chain rule]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Functions, limits, continuity, and scalar change|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 803 (T04 · M049), 804 (T04 · M049).
