---
tags:
  - concept
  - university-series
  - independent-series
---

# Constrained optimization and inverse kinematics

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Optimization chooses parameters to minimize an objective while satisfying constraints. Gradient descent steps opposite the gradient; Newton’s method also uses curvature, while quasi-Newton methods estimate curvature from gradient changes. Constraints may be enforced by parameterization, penalty terms, projection, or constrained solvers. In inverse kinematics, the parameters are joint angles and the objective measures distance between an end effector and its target. Forward kinematics maps angles to a hand or foot position; its Jacobian maps small joint changes to small endpoint changes. A damped least-squares step solves a stabilized linear system to reduce target error without exploding near singular poses. Joint limits, collisions, and multiple valid solutions make initialization and constraints important. The same framework applies to fitting cameras, deforming meshes, or matching image observations.

## Prerequisites

- [[Singular value decomposition]]
- [[Coordinate frames and change of basis]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Automatic differentiation and backpropagation]]
- [[Collision barriers and continuous detection]]
- [[Latent-variable models and generative flows]]
- [[Local rigidity and mesh orientation fields]]
- [[Perception feedback and robot learning]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Scene graphs and articulated geometry|Scene graphs and articulated geometry]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]]
