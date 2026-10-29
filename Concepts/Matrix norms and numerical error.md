# Matrix norms and numerical error

The spectral norm is the largest 2-norm stretch and equals the largest singular value. The Frobenius norm measures aggregate squared entry energy. Residuals, orthogonality errors, and condition numbers need a declared norm and scale; a small residual alone need not imply a sensitive solution is accurate.

**Use it when:** setting scaled tolerances or interpreting numerical diagnostics.

**Developed in Phase 8:** The spectral norm measures worst-case amplification, while the Frobenius norm accumulates squared energy across entries. The chapter uses them to scale residuals, reconstruction errors, and orthogonality defects, and separates backward error in a nearby problem from forward error in the requested answer. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Matrix norms put scale into error statements|norms and scale]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Residuals distinguish backward accuracy from forward certainty|residuals and error interpretation]]

## Prerequisites

[[Norms lengths and induced distances]], [[Singular value decomposition]], [[Forward and backward error]].

## Used by

[[Moore-Penrose pseudoinverse]], [[Matrix functions]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Matrix norms give scale to residuals and error claims|norms and error]].
