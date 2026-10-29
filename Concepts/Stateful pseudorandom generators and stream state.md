---
tags:
  - concept
aliases:
  - "PRNG"
  - "LCG"
  - "engine state"
  - "random stream"
---

# Stateful pseudorandom generators and stream state

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A pseudorandom-number generator is a deterministic state transition plus an output rule. With a fixed engine, seed, and call order, its integer sequence is reproducible; it is not a source of mathematical independence by itself. Linear congruential generators are compact but have lattice and low-bit structure that can make multidimensional samples poor. Larger-state engines trade memory for period and other statistical properties. Parallel programs need an explicit stream strategy: shared state couples samples to thread scheduling, while copied seeds can duplicate streams. Reproducibility requires recording algorithm version, state, seed derivation, distribution mapping, and sampling order. Standard-library distributions may not yield identical transformed sequences across implementations. Simulation generators are not cryptographically secure unless specifically designed for that purpose.

## Prerequisites

- [[Common probability distributions and their assumptions]]

## Used by

- [[Inverse, rejection, and importance sampling]]
- [[Stateless keyed sampling and reproducibility]]

- [[Uniform shuffling without replacement]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 932 (T06 · M057), 933 (T06 · M057), 934 (T06 · M057), 935 (T06 · M057), 936 (T06 · M057), 937 (T06 · M057).
