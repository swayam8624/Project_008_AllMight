---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "initial-value problem solver"
  - "stiff ODE integration"
---

# ODE integration and stability

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** Phase 9

## Definition and mechanism

An ordinary differential equation gives the rate of change of a state over time. Forward Euler uses the current derivative: $q_{k+1}=q_k+h f(q_k,t_k)$, where $h$ is the time step. For decay $\dot u=-au$, it multiplies each step by $1-ah$; if the magnitude exceeds one, the numerical solution grows even though the true solution decays. Spring systems can be stiff because high-frequency modes force very small stable steps. Backward Euler evaluates the rate at the unknown new state and is more stable for decay but requires solving an equation and may add numerical damping. Symplectic Euler updates velocity then position and often keeps energy bounded for conservative mechanics. Choose an integrator according to stability, accuracy, conservation, and runtime rather than displayed frame rate alone.

### Phase 9–10 development

An initial-value problem consists of a differential equation and a specified starting state. A numerical integrator replaces continuous evolution with finite updates, so accuracy and stability depend on both method and step size. Consistency concerns local approximation, stability concerns error growth under repeated steps, and convergence concerns the limiting solution as the step is refined. Stiff systems can force explicit methods to take very small steps even when the visible trajectory changes slowly. Symplectic methods preserve Hamiltonian structure but do not generally preserve exact energy at each step.

## Prerequisites

- [[Absolute and relative error]]
- [[Conditioning and singularity]]
- [[ODE initial-value problems and stiffness]]
- [[Time integrators and symplectic updates]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Implicit differentiation and parameter sensitivities]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Animation curves and motion paths|Animation curves and motion paths]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · Phase 9 visual recall
