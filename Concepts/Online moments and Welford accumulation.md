---
tags:
  - concept
aliases:
  - "online variance"
  - "Welford algorithm"
  - "centered sum of squares"
---

# Online moments and Welford accumulation

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Online statistics update a mean and centered sum of squares as observations arrive, using constant storage. Welford's recurrence computes a new mean from the old mean and one sample, then adds the product of deviations measured before and after that update. The centered form avoids severe cancellation in E[X²]−E[X]² for data with a large offset and small spread. Divide accumulated scatter by n for descriptive population-normalized variance or by n−1 for unbiased sample variance under IID finite-variance assumptions. The recurrence is numerically better behaved, not immune to overflow, count exhaustion, or all rounding error. Parallel workers can merge count, mean, and scatter summaries with a correction term for different means; merge order can change floating-point results.

## Prerequisites

- [[Expectation, covariance, and dependence]]
- [[Accumulated rounding error]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#9. Finite samples need stable descriptive statistics|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 951 (T06 · M058), 952 (T06 · M058), 953 (T06 · M058), 954 (T06 · M058), 955 (T06 · M058).
