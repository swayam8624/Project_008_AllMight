---
aliases:
  - "covariance principal components"
---

# PCA and covariance eigensystems

Center sample rows, then find the orthogonal directions of greatest variance. For centered $X$, SVD gives directions in $V$, scores in $U\Sigma$, and covariance eigenvalues $\sigma_i^2/(N-1)$. Units and feature scaling change the question; near-equal eigenvalues make individual axes unstable.

**Use it when:** describing point clouds, reducing dimensions, or estimating a best-fit plane.

**Developed in Phase 8:** PCA first centers observations so the covariance measures variation around the sample mean. The right singular vectors of centered data give principal directions, $U\Sigma$ gives scores, and squared singular values scaled by sample count give covariance eigenvalues; feature units and small eigengaps affect interpretation. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Centered measurements reveal principal directions|centering and principal axes]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Plane fits, eigengaps, and interpretation of uncertain components|plane fits and eigengaps]]

**Phase 9–10 development:** For a random vector $X$ with finite second moments, covariance is $\Sigma=\mathbb E[(X-\mu)(X-\mu)^T]$. For every vector $v$, $v^T\Sigma v=\mathbb E[(v^T(X-\mu))^2]\geq0$, so covariance is positive semidefinite. Its eigenvectors give uncorrelated principal directions and the eigenvalues give variance along them. Whitening rescales nonzero-variance directions and needs a stated policy for zero or tiny eigenvalues. A Gaussian kernel used to model rendering footprints is a spatial distribution model, not automatically a statement about epistemic uncertainty.

## Prerequisites

[[Eigenvalues and eigenspaces]], [[Singular value decomposition]], [[Symmetric positive-definite matrices and Cholesky]].
- [[Expectation, covariance, and dependence]]
- [[Multivariate Gaussian distributions and whitening]]
- [[Moore-Penrose pseudoinverse]]
- [[QR factorization and least squares]]

## Used by

[[Polar decomposition]], [[Matrix factorization choices]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#PCA turns a cloud of points into measured variation|PCA from data]].

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#3. Expected values describe centers; covariance describes dependence|Phase 10 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
