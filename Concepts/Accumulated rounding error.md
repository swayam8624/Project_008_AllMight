---
tags:
  - concept
  - foundation
---

# Accumulated rounding error

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Under suitable nearest normal-range assumptions, products of rounding factors are bounded using gamma_n=nu/(1−nu), nu<1. A sequential n-term sum has absolute error bounded by gamma_(n−1) times sum of input magnitudes. Cancellation can make its relative error much larger.

## Prerequisites

- [[Unit roundoff]] — The local normal-rounding bound supplies the error-analysis scale.
- [[Absolute and relative error]] — Discrepancy, magnitude and reference units define sensitivity and threshold significance.

## Used by

- [[Compensated and pairwise summation]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Apply bounds to actual dependency paths, not a global count of source operators; subnormals/range failures require other analysis.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
