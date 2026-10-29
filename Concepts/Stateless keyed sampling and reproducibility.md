---
tags:
  - concept
aliases:
  - "counter-based RNG"
  - "random-access sampling"
  - "indexed random numbers"
---

# Stateless keyed sampling and reproducibility

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Stateless sampling derives an output from explicit coordinates such as a master seed, pixel, sample index, and dimension. The same key can be recomputed without replaying prior generator calls, which makes debugging and parallel scheduling easier. A hash mixer can provide deterministic, well-dispersed values, but reproducibility does not prove IID behavior or statistical quality. Counter-based generators use designed mixing rounds and can provide stronger parallel-stream properties under their analyzed models. Distinct sampling dimensions reduce accidental reuse, yet a dimension map must remain stable when code changes. Record the key layout, algorithm, version, and conversion to floating point. Test multidimensional patterns and estimator convergence; a good one-dimensional histogram does not establish good joint behavior.

## Prerequisites

- [[Stateful pseudorandom generators and stream state]]

## Used by

- [[Sampling quality: stratification, quasi-Monte Carlo, and blue noise]]

- [[Uniform shuffling without replacement]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 938 (T06 · M057), 939 (T06 · M057).
