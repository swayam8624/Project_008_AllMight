---
tags:
  - concept
aliases:
  - "PMF"
  - "PDF"
  - "CDF"
  - "random law"
  - "quantile"
---

# Random variables and probability laws

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A random variable is a measurable mapping from outcomes to numerical values; its probability law describes how those values are distributed. Discrete laws assign probability mass to points, while continuous laws use densities relative to a chosen measure. A cumulative distribution function exists for either kind and records probability accumulated up to a threshold. Mixed distributions can contain both atoms and continuous parts. Density is not probability at an exact point, and it changes with coordinates according to a Jacobian. Expectation integrates a function against the law, whether the variable is represented by a PMF or PDF. These distinctions matter in sampling APIs: returned value, support, endpoint behavior, and reported density must refer to the same measure. A finite computer sampler approximates a continuous law with finitely many representable outputs.

## Prerequisites

- [[Probability spaces, events, and conditioning]]

## Used by

- [[Common probability distributions and their assumptions]]
- [[Expectation, covariance, and dependence]]
- [[Inverse, rejection, and importance sampling]]
- [[Probability density and change of measure]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#2. Random quantities and their probability laws|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 907 (T01 · M055), 908 (T01 · M055), 909 (T01 · M055), 910 (T01 · M055), 911 (T01 · M055), 912 (T01 · M055).
