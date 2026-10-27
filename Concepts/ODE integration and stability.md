---
tags:
  - concept
  - university-series
  - independent-series
---

# ODE integration and stability

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

An ordinary differential equation gives the rate of change of a state over time. Forward Euler uses the current derivative: $q_{k+1}=q_k+h f(q_k,t_k)$, where $h$ is the time step. For decay $\dot u=-au$, it multiplies each step by $1-ah$; if the magnitude exceeds one, the numerical solution grows even though the true solution decays. Spring systems can be stiff because high-frequency modes force very small stable steps. Backward Euler evaluates the rate at the unknown new state and is more stable for decay but requires solving an equation and may add numerical damping. Symplectic Euler updates velocity then position and often keeps energy bounded for conservative mechanics. Choose an integrator according to stability, accuracy, conservation, and runtime rather than displayed frame rate alone.

## Prerequisites

- [[Absolute and relative error]]
- [[Conditioning and singularity]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Dynamics, forces, and numerical integration|Dynamics, forces, and numerical integration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Animation curves and motion paths|Animation curves and motion paths]]
