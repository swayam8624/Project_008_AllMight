---
tags:
  - concept
aliases:
  - "Schur complement covariance"
  - "Gaussian conditioning"
---

# Conditional Gaussians and Schur complements

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Conditioning one block of a jointly Gaussian vector on another produces a Gaussian conditional law under the usual nonsingular assumptions. Its mean shifts by the cross-covariance times the inverse observed-block covariance, while its covariance is the Schur complement Σbb−ΣbaΣaa⁻¹Σab. That conditional covariance is positive semidefinite: insert the vector (−Σaa⁻¹Σabv,v) into the joint covariance quadratic form. If the joint covariance is positive definite, the Schur complement is positive definite. The conditional covariance does not depend on the observed value, although the conditional mean does. This formula links Bayesian updating, Gaussian elimination, and block linear systems. Singular observed blocks require a compatible degenerate-Gaussian treatment or a rank-revealing method; blindly applying an inverse is invalid.

## Prerequisites

- [[Multivariate Gaussian distributions and whitening]]
- [[Gaussian elimination and pivoting]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 920 (T01 · M055), 928 (T01 · M056), 929 (T01 · M056), 930 (T01 · M056).
