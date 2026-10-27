# QR factorization and least squares

QR writes $A=QR$ with orthonormal columns in $Q$ and triangular $R$. Orthogonal coordinates preserve Euclidean lengths, so least squares reduces to a triangular solve and a perpendicular residual. Householder QR is a stable dense method; Gram–Schmidt explains the geometry but can lose orthogonality.

**Use it when:** fitting overdetermined data with known full column rank.

**Developed in Phase 8:** Orthogonal transformations change coordinates without changing Euclidean lengths. QR therefore separates the fitted component, solved through triangular $R$, from the constant perpendicular residual; the Householder construction is the stable dense route, with Gram–Schmidt retained to make the geometry visible. [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Reflections and plane rotations build QR without losing lengths|Householder and Givens construction]] · [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#QR turns least squares into a triangular solve plus an irreducible residual|least squares and its residual]]

## Prerequisites

[[Linear systems and fundamental subspaces]], [[Dot products and orthogonality]], [[Conditioning and singularity]].

## Used by

[[Matrix factorization choices]], [[Singular value decomposition]], [[PCA and covariance eigensystems]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Structure makes a decomposition a better tool than generic elimination|QR and fitting]].
