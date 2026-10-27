# PCA and covariance eigensystems

Center sample rows, then find the orthogonal directions of greatest variance. For centered $X$, SVD gives directions in $V$, scores in $U\Sigma$, and covariance eigenvalues $\sigma_i^2/(N-1)$. Units and feature scaling change the question; near-equal eigenvalues make individual axes unstable.

**Use it when:** describing point clouds, reducing dimensions, or estimating a best-fit plane.

**Developed in Phase 8:** PCA first centers observations so the covariance measures variation around the sample mean. The right singular vectors of centered data give principal directions, $U\Sigma$ gives scores, and squared singular values scaled by sample count give covariance eigenvalues; feature units and small eigengaps affect interpretation. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Centered measurements reveal principal directions|centering and principal axes]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Plane fits, eigengaps, and interpretation of uncertain components|plane fits and eigengaps]]

## Prerequisites

[[Eigenvalues and eigenspaces]], [[Singular value decomposition]], [[Symmetric positive-definite matrices and Cholesky]].

## Used by

[[Polar decomposition]], [[Matrix factorization choices]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#PCA turns a cloud of points into measured variation|PCA from data]].
