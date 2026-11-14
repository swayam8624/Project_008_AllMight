# Matrix norms and numerical error

The spectral norm is the largest 2-norm stretch and equals the largest singular value. The Frobenius norm measures aggregate squared entry energy. Residuals, orthogonality errors, and condition numbers need a declared norm and scale; a small residual alone need not imply a sensitive solution is accurate.

**Use it when:** setting scaled tolerances or interpreting numerical diagnostics.

**Developed in Phase 8:** The spectral norm measures worst-case amplification, while the Frobenius norm accumulates squared energy across entries. The chapter uses them to scale residuals, reconstruction errors, and orthogonality defects, and separates backward error in a nearby problem from forward error in the requested answer. [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Matrix norms put scale into error statements|norms and scale]] · [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Residuals distinguish backward accuracy from forward certainty|residuals and error interpretation]]

## Prerequisites

[[Norms lengths and induced distances]], [[Singular value decomposition]], [[Forward and backward error]].

## Used by

[[Moore-Penrose pseudoinverse]], [[Matrix functions]], [[Sparse and iterative solvers]].

**Teaching path:** [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Matrix norms give scale to residuals and error claims|norms and error]].


## A matrix norm can protect a geometry bound

A local sphere under a linear transform is generally an ellipsoid. The Frobenius norm times its original radius gives a safe, possibly loose enclosing radius. Taking only the longest matrix column can fail under shear. [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A line corridor can be a cheap prefilter for shadow-caster work|The culling example]] derives the bound, demonstrates the shear counterexample, and links its geometric figure. This connects numerical matrix norms to conservative rejection decisions.
