# Polar decomposition

The right polar form $A=QH$ separates an orthogonal factor from a positive-semidefinite stretch $H=(A^\mathsf T A)^{1/2}$. The orthogonal factor may include reflection; singular matrices can make its completion nonunique. Symmetric stretch can include shear in the chosen coordinates.

**Use it when:** separating local rigid-like motion from symmetric deformation.

**Developed in Phase 8:** Polar decomposition factors a map into an orthogonal part and a positive-semidefinite stretch. The SVD constructs both left and right forms, separates reflection from a proper rotation, and gives the nearest orthogonal or special-orthogonal factor when the singular directions allow a unique choice; zero singular values leave completion freedom. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Polar decomposition separates frame dependent stretch from orientation|left and right stretch]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Two-sided rotations, nearest proper rotation, and nonunique directions|nearest proper rotation and ambiguity]]

## Prerequisites

[[Singular value decomposition]], [[Eigenvalues and eigenspaces]], [[Determinants orientation and volume]].

## Used by

[[Matrix functions]], [[Transform hierarchies TRS and shear]].

**Teaching path:** [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Polar decomposition separates rotation-like motion from stretch|polar factors]].
