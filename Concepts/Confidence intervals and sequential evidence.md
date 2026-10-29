---
tags:
  - concept
aliases:
  - "coverage interval"
  - "frequentist interval"
  - "confidence sequence"
  - "sequential stopping"
---

# Confidence intervals and sequential evidence

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A confidence interval is a procedure with repeated-sample coverage under stated assumptions; it is not a posterior probability assigned to a fixed parameter. A normal interval for a mean uses a known population standard deviation or an approximation justified by the central limit theorem. With unknown scale and IID normal observations, the Student t distribution accounts for estimating the standard deviation. Heavy tails, sample dependence, multiple comparisons, and optional stopping can invalidate simple textbook guarantees. Adaptive algorithms that stop when a fixed-sample interval looks narrow need sequentially valid methods or a separately justified stopping rule. Report the target, sampling unit, dependence structure, estimator bias, interval procedure, stopping policy, and independent-run count. A narrow interval around a biased estimator measures precision about its expectation, not correctness of the target.

## Prerequisites

- [[Common probability distributions and their assumptions]]
- [[Expectation, covariance, and dependence]]
- [[Monte Carlo rendering estimators and variance]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

- [[Reproducible numerical benchmarking]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#10. Inference and error metrics need explicit assumptions|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 960 (T06 · M058), 961 (T06 · M058).
