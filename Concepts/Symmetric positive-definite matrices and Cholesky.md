# Symmetric positive-definite matrices and Cholesky

A real symmetric matrix $A$ is positive definite if every nonzero $x$ has positive quadratic energy $x^\mathsf T A x$. Then $A=LL^\mathsf T$ with nonsingular triangular $L$. Positive semidefinite permits zero energy directions and may be singular, so Cholesky's assumptions and failure status must be respected.

**Use it when:** covariance, Gram systems, energy minimization, or an SPD solve is established.

**Developed in Phase 8:** Positive definiteness means every nonzero direction has positive quadratic energy, which permits a Cholesky factor and stable triangular solves. The continuation derives the positive pivots, then broadens to $LDL^\mathsf T$ when a symmetric matrix is indefinite and explains how a failed pivot reflects the assumptions or numerical scale. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Positive definiteness is a structural promise|Cholesky and positive energy]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#LDLT reaches symmetric indefinite matrices|the symmetric-indefinite extension]]

## Prerequisites

[[Matrix transpose symmetry and trace]], [[Gram matrices and coordinate metrics]].

## Used by

[[Matrix factorization choices]], [[PCA and covariance eigensystems]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Structure makes a decomposition a better tool than generic elimination|structure-aware factors]].
