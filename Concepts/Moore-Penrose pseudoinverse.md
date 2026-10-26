# Moore-Penrose pseudoinverse

The SVD defines $A^+$ by reciprocating trusted nonzero singular values and transposing the orthogonal factors. $A^+b$ minimizes residual first and Euclidean solution norm among minimizers second. Numerical thresholding makes the meaning of weak directions explicit.

**Use it when:** rectangular or rank-deficient least squares needs a minimum-norm answer.

## Prerequisites

[[Singular value decomposition]], [[Linear systems and fundamental subspaces]], [[Matrix norms and numerical error]].

## Used by

[[PCA and covariance eigensystems]], [[Matrix factorization choices]].

**Teaching path:** [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions#Pseudoinverse makes least squares and minimum norm one operation|pseudoinverse]].
