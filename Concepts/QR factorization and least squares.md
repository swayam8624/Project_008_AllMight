# QR factorization and least squares

QR writes $A=QR$ with orthonormal columns in $Q$ and triangular $R$. Orthogonal coordinates preserve Euclidean lengths, so least squares reduces to a triangular solve and a perpendicular residual. Householder QR is a stable dense method; Gram–Schmidt explains the geometry but can lose orthogonality.

**Use it when:** fitting overdetermined data with known full column rank.

## Prerequisites

[[Linear systems and fundamental subspaces]], [[Dot products and orthogonality]], [[Conditioning and singularity]].

## Used by

[[Matrix factorization choices]], [[Singular value decomposition]], [[PCA and covariance eigensystems]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Structure makes a decomposition a better tool than generic elimination|QR and fitting]].
