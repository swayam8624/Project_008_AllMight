# Symmetric positive-definite matrices and Cholesky

A real symmetric matrix $A$ is positive definite if every nonzero $x$ has positive quadratic energy $x^\mathsf T A x$. Then $A=LL^\mathsf T$ with nonsingular triangular $L$. Positive semidefinite permits zero energy directions and may be singular, so Cholesky's assumptions and failure status must be respected.

**Use it when:** covariance, Gram systems, energy minimization, or an SPD solve is established.

**Developed in Phase 8:** Positive definiteness means every nonzero direction has positive quadratic energy, which permits a Cholesky factor and stable triangular solves. The continuation derives the positive pivots, then broadens to $LDL^\mathsf T$ when a symmetric matrix is indefinite and explains how a failed pivot reflects the assumptions or numerical scale. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Positive definiteness is a structural promise|Cholesky and positive energy]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#LDLT reaches symmetric indefinite matrices|the symmetric-indefinite extension]]

## Prerequisites

[[Matrix transpose symmetry and trace]], [[Gram matrices and coordinate metrics]].

## Used by

[[Matrix factorization choices]], [[PCA and covariance eigensystems]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Structure makes a decomposition a better tool than generic elimination|structure-aware factors]].
- [[Hessians and local curvature]]
- [[Multivariate Gaussian distributions and whitening]]
- [[Nonlinear least squares: Gauss-Newton and Levenberg-Marquardt]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Jacobians, curvature, and local approximation|Phase 9 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#5. Waiting times, event counts, and covariance geometry|Phase 10 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Optimization and inverse problems|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · Phase 9 visual recall
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
