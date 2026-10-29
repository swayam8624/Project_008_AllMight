# Singular value decomposition

For any real rectangular matrix, $A=U\Sigma V^\mathsf T$. Right singular vectors are input directions, left singular vectors are output directions, and nonnegative singular values are principal stretches. Unlike eigenvectors, the two sets live in different spaces.

**Use it when:** revealing rank and conditioning, solving rank-uncertain least squares, compressing a matrix, or understanding PCA.

**Developed in Phase 8:** The SVD expresses a rectangular map using orthonormal input and output directions joined by nonnegative stretches. Full, thin, and compact shapes have different dimensions; bidiagonal reduction and Jacobi methods compute the factors, and reconstruction, orthogonality, and triplet residuals check them independently. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Full, thin, and compact shapes for tall and wide matrices|factor shapes]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#How numerical software computes and checks an SVD|SVD algorithms and validation]]

## Prerequisites

[[Matrix multiplication and composition order]], [[Eigenvalues and eigenspaces]], [[Dot products and orthogonality]].

## Used by

[[Moore-Penrose pseudoinverse]], [[PCA and covariance eigensystems]], [[Polar decomposition]], [[Matrix factorization choices]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Singular value decomposition covers every rectangular map|SVD geometry]].
- [[Matrix norms and numerical error]] — Extends this foundation into the Phase 8 dependency spine.
- [[Camera calibration and bundle adjustment]]
- [[Constrained optimization and inverse kinematics]]

## University course development

Extend the existing factorization note with its role in estimation, conditioning, and geometry. For matrix $A$, the decomposition $A=U\Sigma V^T$ separates orthogonal input directions, axis-aligned scaling, and orthogonal output directions. Singular values on the diagonal of $\Sigma$ quantify how strongly each independent direction is preserved; a zero or tiny value signals rank loss or poor conditioning. Least-squares fitting can use this structure to identify which parameter combinations are well constrained and to avoid amplifying measurement noise along nearly invisible directions. The same analysis explains why a camera calibration or bundle-adjustment system can have gauge freedoms: translating or scaling the entire reconstruction may leave image measurements unchanged. Singular values diagnose these ambiguities, while constraints or fixed reference poses remove them. Thus SVD links linear algebra to robust vision, not just matrix compression.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]
