---
tags:
  - concept
aliases: []
---

# Numerical error budgets

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A computed result can differ from a target because of model assumptions, discretization, solver termination, arithmetic, sampling, or uncertain reference data. These contributions may interact, so an additive budget is a diagnostic model rather than a universal identity. First identify which errors are deterministic bounds, which are observed estimates, and which are stochastic uncertainty. For example, a ray-traced pixel can combine approximation of geometry, finite sample variance, floating-point accumulation, and an imperfect reference image. Allocate effort where the dominant contribution lies: extra precision cannot fix model bias, and more samples cannot repair an incorrect PDF. Report units and assumptions with every tolerance.

## Prerequisites

- [[Forward and backward error]]
- [[Conditioning and singularity]]
- [[Algorithmic stability]]
- [[Discretization, truncation, and roundoff]]
- [[Bracketed and open root solvers]]
- [[A posteriori error bounds for contractive iteration]]
- [[Convergence order and Richardson extrapolation]]
- [[Position reconstruction from depth]]

## Used by

- [[Reproducible numerical benchmarking]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Separate discretization, truncation, and roundoff; measure convergence|Separate discretization, truncation, and roundoff; measure convergence]]

## Source coverage

Supplied lesson parts: 966–1000.

## Future keywords

model bias, solver tolerance, sampling uncertainty
