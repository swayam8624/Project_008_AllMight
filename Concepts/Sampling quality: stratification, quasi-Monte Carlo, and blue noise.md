---
tags:
  - concept
aliases:
  - "quasi-Monte Carlo"
  - "QMC"
  - "low-discrepancy sequence"
  - "blue-noise point set"
  - "stratified sampling"
---

# Sampling quality: stratification, quasi-Monte Carlo, and blue noise

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Sampling quality concerns the arrangement of points across dimensions used by an estimator, not only each coordinate's one-dimensional histogram. Stratification partitions a domain and allocates points across cells, reducing large gaps for suitable integrands. Quasi-Monte Carlo sequences such as Sobol aim for low discrepancy and often improve integration of smooth, structured functions, though their samples are dependent and error estimates differ from IID Monte Carlo. Blue-noise point sets suppress low-frequency clumping while retaining local separation; they are useful in image sampling and spatial placement but do not automatically minimize every integral's variance. Scrambling, domain mapping, dimensional ordering, and discontinuities affect performance. Compare methods against a clear target function and measure. The supplied probability narrative introduces stratification and Sobol but not blue-noise constructions; that part is an explicit teaching addition, not a received source claim.

## Prerequisites

- [[Stateless keyed sampling and reproducibility]]
- [[Sampling aliasing and reconstruction]]

- [[Uniform shuffling without replacement]]
## Used by

- [[Antialiasing and sample allocation]]

- [[Latin-hypercube sampling]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 939 (T06 · M057).
