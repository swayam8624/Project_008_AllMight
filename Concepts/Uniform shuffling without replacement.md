---
tags:
  - concept
aliases: []
---

# Uniform shuffling without replacement

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Fisher–Yates generates a uniform permutation by walking from the last array element downward and choosing an index uniformly from the unshuffled prefix. Each permutation has probability $1/n!$ if every bounded integer choice is unbiased and the random source meets its assumptions. A common error is to use a random integer routine with modulo bias; another is to draw from the wrong interval and omit permutations. For a fixed input array, test that the output contains every original element once; statistical tests can supplement, but not replace, deterministic checks of bounds and uniqueness. The same primitive supports uniform sample subsets and randomized stratification.

## Prerequisites

- [[Stateful pseudorandom generators and stream state]]
- [[Stateless keyed sampling and reproducibility]]
- [[Probability spaces, events, and conditioning]]

## Used by

- [[Latin-hypercube sampling]]
- [[Sampling quality: stratification, quasi-Monte Carlo, and blue noise]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Design samples: without replacement, stratified, and low-discrepancy|Design samples: without replacement, stratified, and low-discrepancy]]

## Source coverage

Supplied lesson parts: 995.

## Future keywords

Fisher–Yates, bounded random integer, permutation uniformity
