---
tags:
  - concept
aliases: []
---

# Stable probability normalization and log-sum-exp

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

To normalize nonnegative weights, divide each by their sum; direct summation can overflow even when the normalized probabilities are representable. Scale by the largest weight before summing. For log weights $a_i$, compute $m+\log\sum_i e^{a_i-m}$, where $m=\max_i a_i$. Subtracting the maximum prevents exponential overflow, and the result can be used to form softmax probabilities. This assumes at least one finite, valid weight and requires an explicit policy for all-zero, negative, NaN, or infinite inputs. Underflow of very small normalized probabilities is a separate concern from overflow; do not silently equate zero after underflow with mathematically impossible. For log weights $(1000,1001,1000)$, subtracting $1001$ keeps exponentials bounded while leaving the normalized probabilities unchanged.

## Prerequisites

- [[Probability density and change of measure]]
- [[Unit roundoff]]
- [[Stable reformulation]]

## Used by

- [[Monte Carlo rendering estimators and variance]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep probability weights stable before sampling|Keep probability weights stable before sampling]]

## Source coverage

Supplied lesson parts: 993–994.

## Future keywords

softmax, partition function, log-domain accumulation
