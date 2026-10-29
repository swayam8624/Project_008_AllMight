---
tags:
  - concept
aliases: []
---

# Bracketed and open root solvers

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A root solver seeks $f(x)=0$. A bracketed method maintains an interval whose endpoints have opposite signs; continuity then guarantees at least one root remains inside, and bisection provides a predictable shrinking error bound. Open methods such as secant and Newton can converge faster near a well-behaved simple root, but they may diverge, encounter a zero derivative, or leave the relevant domain. A safeguarded hybrid uses a bracket for global protection and accepts fast proposals only when they preserve progress. Residual size alone does not certify root accuracy when the root is poorly conditioned, and discontinuous functions do not satisfy the sign-change guarantee.

## Prerequisites

- [[Algorithmic stability]]
- [[Conditioning and singularity]]
- [[Stable quadratic root evaluation]]
- [[A posteriori error bounds for contractive iteration]]

## Used by

- [[Numerical error budgets]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Finding roots with guarantees and convergence checks|Finding roots with guarantees and convergence checks]]

## Source coverage

Supplied lesson parts: 984–988.

## Future keywords

bisection, secant method, Newton method, Brent-style safeguard
