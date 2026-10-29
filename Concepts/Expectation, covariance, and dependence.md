---
tags:
  - concept
aliases:
  - "first and second moments"
  - "covariance matrix"
  - "correlation coefficient"
---

# Expectation, covariance, and dependence

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Expectation is the probability-weighted average of a random quantity, and variance is the expected squared deviation from that average. Covariance measures how two centered quantities vary together; correlation scales it to a unitless linear-association measure. For a random vector, the covariance matrix is symmetric positive semidefinite because every directional quadratic form equals a variance. Zero covariance does not usually imply independence, except under special joint Gaussian conditions. Conditional expectation and total-variance identities decompose uncertainty into within-group spread and variation between group means. These moments drive Monte Carlo error analysis, Gaussian geometry, and linear uncertainty propagation. An empirical covariance depends on sample convention and denominator; Bessel correction concerns unbiased population-variance estimation under IID sampling, not a universal change to descriptive covariance.

## Prerequisites

- [[Probability spaces, events, and conditioning]]
- [[Random variables and probability laws]]

## Used by

- [[Common probability distributions and their assumptions]]
- [[Confidence intervals and sequential evidence]]
- [[Monte Carlo rendering estimators and variance]]
- [[Multivariate Gaussian distributions and whitening]]
- [[Online moments and Welford accumulation]]
- [[PCA and covariance eigensystems]]
- [[Robust summaries and error metrics]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#3. Expected values describe centers; covariance describes dependence|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 913 (T01 · M055), 914 (T01 · M055), 915 (T01 · M055), 916 (T01 · M055), 917 (T01 · M055), 918 (T01 · M055), 919 (T01 · M055), 920 (T01 · M055), 929 (T01 · M056), 930 (T01 · M056).
