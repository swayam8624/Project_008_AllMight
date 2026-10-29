---
tags:
  - concept
aliases: []
---

# Latin-hypercube sampling

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Latin-hypercube sampling divides each coordinate axis into $N$ equal strata and uses a permutation so every sample occupies one stratum per dimension. With randomized within-stratum positions and randomized permutations, each sample has uniform marginal coordinates; samples are dependent by design. This can improve coverage for some integrands at arbitrary sample counts, including counts that do not form a square grid. It does not guarantee lower variance for every function, and IID variance formulas cannot be applied without accounting for dependence. Test exact stratum coverage deterministically, then compare estimator error over repeated randomized trials for the intended integrand. For $N=5$, each axis still receives five strata, avoiding the missing-cell bias of taking five cells from a $3\times3$ grid.

## Prerequisites

- [[Uniform shuffling without replacement]]
- [[Sampling quality: stratification, quasi-Monte Carlo, and blue noise]]
- [[Probability density and change of measure]]

## Used by

- [[Reproducible numerical benchmarking]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Design samples: without replacement, stratified, and low-discrepancy|Design samples: without replacement, stratified, and low-discrepancy]]

## Source coverage

Supplied lesson parts: 997.

## Future keywords

marginal stratification, randomized Latin hypercube, dependence
