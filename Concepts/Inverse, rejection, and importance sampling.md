---
tags:
  - concept
aliases:
  - "inverse transform"
  - "inverse CDF"
  - "rejection sampler"
  - "importance weights"
---

# Inverse, rejection, and importance sampling

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Sampling transforms convert simpler random inputs into a target law. Inverse-CDF sampling maps a uniform value through a quantile function; rejection sampling accepts proposal draws according to a bounded target-to-proposal ratio; importance sampling estimates an integral by dividing each sampled integrand by the actual proposal density. Each method requires correct support and endpoint handling. Importance sampling is unbiased when the proposal covers every contributing region and the weighted contribution is integrable; finite variance additionally requires the integral of f²/p to be finite. Rejection sampling needs a valid finite envelope. Discrete floating-point inputs can hit boundaries that have zero probability in an ideal continuous model. Report the density with its measure and use the same measure in the estimator. A proposal that misses target support cannot be repaired by a larger weight.

## Prerequisites

- [[Random variables and probability laws]]
- [[Probability density and change of measure]]
- [[Stateful pseudorandom generators and stream state]]

## Used by

- [[Monte Carlo rendering estimators and variance]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#7. Transformations turn simple inputs into useful samples|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 940 (T06 · M057), 941 (T06 · M057), 942 (T06 · M057), 943 (T06 · M057), 944 (T06 · M057).
