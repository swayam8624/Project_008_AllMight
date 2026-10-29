---
tags:
  - concept
aliases: []
---

# Algorithmic stability

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A numerical problem can be sensitive even when an algorithm handles it well. Stability describes whether the finite-precision algorithm introduces only the error expected from rounding, often by showing that its output exactly solves a nearby problem. Backward stability is one useful form; forward stability asks directly how close the computed answer is to the exact answer for the original input. The claim must name its assumptions, arithmetic model, and problem class. For example, a stable linear solver can still have a large forward error on an ill-conditioned matrix. Stability therefore belongs beside conditioning, not in its place. A dot product with a small residual can still conceal an ill-conditioned problem, so report which notion of stability is actually supported.

## Prerequisites

- [[Forward and backward error]]
- [[Conditioning and singularity]]

## Used by

- [[Stable quadratic root evaluation]]
- [[Bracketed and open root solvers]]
- [[Numerical error budgets]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Floating-point answers: error, conditioning, stability, and stable arithmetic|Floating-point answers: error, conditioning, stability, and stable arithmetic]]

## Source coverage

Supplied lesson parts: 966–975.

## Future keywords

backward stability, mixed stability, componentwise stability
