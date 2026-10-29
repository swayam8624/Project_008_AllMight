---
tags:
  - concept
aliases: []
---

# Reproducible numerical benchmarking

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A useful numerical benchmark separates sample-level variability from run-level variability and pairs competing methods on the same inputs or seeds. Record the algorithm, precision, compiler/runtime settings, sample count, seed or keyed input, warm-up policy, timing method, and error metric. Compare distributions or paired differences rather than a single favorable run. A speed improvement is meaningful only if accuracy and failure rates remain acceptable for the target workload. For Monte Carlo methods, repeat independent runs and report uncertainty; for deterministic solvers, use analytical or manufactured solutions and measure convergence. Reproducibility is an experimental contract, not merely a fixed random seed. If the workload is stochastic, compare paired seeds and report both runtime and error distributions.

## Prerequisites

- [[Numerical reproducibility]]
- [[Confidence intervals and sequential evidence]]
- [[Robust summaries and error metrics]]
- [[Numerical error budgets]]
- [[Convergence order and Richardson extrapolation]]
- [[Latin-hypercube sampling]]

## Used by

- [[Research validity and ablation]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Test numerical methods and benchmark reproducibly|Test numerical methods and benchmark reproducibly]]

## Source coverage

Supplied lesson parts: 999–1000.

## Future keywords

paired trials, random seeds, uncertainty reporting
