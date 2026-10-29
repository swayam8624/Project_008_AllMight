---
tags:
  - concept
aliases: []
---

# A posteriori error bounds for contractive iteration

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

For a fixed-point iteration $x_{n+1}=G(x_n)$, suppose $G$ is a contraction with Lipschitz constant $q<1$ on a region containing the iterates and its fixed point. Then the remaining error is bounded by the next step divided by $1-q$: $\|x^*-x_n\|\le\|x_{n+1}-x_n\|/(1-q)$. The assumption is essential; a small step alone does not prove convergence because stagnation or a failed update can also make changes tiny. In implementation, report the contraction evidence or a suitable residual bound and keep failure distinct from success. This turns an observed update into a conditional, checkable estimate rather than a guess. If $q=1/2$ and the latest step is $10^{-6}$, this bound is $2\times10^{-6}$, conditional on the contraction proof.

## Prerequisites

- [[Forward and backward error]]
- [[Conditioning and singularity]]

## Used by

- [[Bracketed and open root solvers]]
- [[Numerical error budgets]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Finding roots with guarantees and convergence checks|Finding roots with guarantees and convergence checks]]

## Source coverage

Supplied lesson parts: 988.

## Future keywords

fixed-point iteration, contraction factor, residual certificate
