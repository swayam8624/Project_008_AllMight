---
tags:
  - concept
aliases:
  - "normal vector"
  - "whitening transform"
  - "Mahalanobis distance"
---

# Multivariate Gaussian distributions and whitening

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

A multivariate Gaussian is determined by a mean vector and a symmetric positive-semidefinite covariance matrix. When the covariance is positive definite, its density is an elliptical bell whose principal directions are covariance eigenvectors. Cholesky factorization maps independent standard-normal samples into correlated samples; whitening applies an inverse factor to express observations in unit-variance coordinates. Singular covariance instead confines the distribution to a lower-dimensional affine subspace and has no ordinary density with respect to full-dimensional volume. Covariance transforms as AΣAᵀ under a linear map and approximately as JΣJᵀ through a nonlinear map near a reference point. These equations support sensor fusion and projected uncertainty, but a Gaussian rendering kernel is not automatically a normalized probability density or epistemic uncertainty model.

## Prerequisites

- [[Expectation, covariance, and dependence]]
- [[Symmetric positive-definite matrices and Cholesky]]
- [[Probability density and change of measure]]

## Used by

- [[Conditional Gaussians and Schur complements]]
- [[PCA and covariance eigensystems]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 928 (T01 · M056), 929 (T01 · M056), 930 (T01 · M056).
