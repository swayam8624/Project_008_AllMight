# Moore-Penrose pseudoinverse

The exact SVD defines $A^+$ by reciprocating every positive singular value and transposing the orthogonal factors. $A^+b$ minimizes residual first and Euclidean solution norm among minimizers second. A numerical cutoff instead produces $A_\tau^+$: if it discards a positive singular value, it solves a retained-rank approximation and can increase the residual of the original system. The cutoff makes the treatment of weak directions explicit.

**Use it when:** rectangular or rank-deficient least squares needs a minimum-norm answer.

**Developed in Phase 8:** The exact pseudoinverse reciprocates every positive singular value, yielding the least-squares solution with minimum Euclidean norm. A practical cutoff defines a different, truncated operator: discarding even a positive singular value can increase the residual for the original system, so the tolerance and retained rank are part of the mathematical answer. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The Moore–Penrose inverse and its four identities|the exact pseudoinverse]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Thresholding and regularization are different choices|truncation and regularization]]

## Prerequisites

[[Singular value decomposition]], [[Linear systems and fundamental subspaces]], [[Matrix norms and numerical error]].

## Used by

[[PCA and covariance eigensystems]], [[Matrix factorization choices]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Pseudoinverse makes least squares and minimum norm one operation|pseudoinverse]].
