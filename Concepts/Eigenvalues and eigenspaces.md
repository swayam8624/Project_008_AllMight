# Eigenvalues and eigenspaces

An eigenvector is a nonzero direction that a square linear map scales by its eigenvalue. The eigenspace is the null space of $A-\lambda I$. Real symmetric matrices have an orthonormal eigenbasis; general matrices may be defective or have sensitive eigenvectors.

**Use it when:** studying repeated powers, invariant directions, covariance, stability, or matrix functions.

**Developed in Phase 8:** Eigenanalysis begins with invariant directions, then distinguishes algebraic multiplicity from eigenspace dimension and shows why a defective matrix may not have a full eigenbasis. Power and inverse iteration target selected eigenpairs; QR iteration approaches a Schur form, while residual size and eigenvector sensitivity answer different accuracy questions. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Eigenvalues, eigenspaces, and the two kinds of multiplicity|eigenspaces and multiplicity]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#QR iteration and Jacobi rotations preserve the spectrum while simplifying shape|eigenvalue iteration and Schur form]]

## Prerequisites

[[Matrices as linear maps and basis images]], [[Linear systems and fundamental subspaces]], [[Matrix transpose symmetry and trace]].

## Used by

[[Singular value decomposition]], [[PCA and covariance eigensystems]], [[Polar decomposition]], [[Matrix functions]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Eigenvectors reveal the directions a square map does not turn|eigenanalysis]].
