---
tags:
  - concept
aliases:
  - "initial-value problem"
  - "ODE stability"
  - "stiffness"
---

# ODE initial-value problems and stiffness

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

An ordinary differential equation specifies a state's rate of change as a function of state and time. An initial-value problem adds the starting state; together they define the trajectory when existence and uniqueness conditions hold. Numerical integration advances approximate states through finite time steps. Stability concerns whether numerical errors remain controlled under repeated updates, not just whether one step is accurate. A stiff system contains modes with very different time scales; explicit methods may require impractically small steps to remain stable even when the visible solution changes slowly. Implicit methods can enlarge the stable step region but require solving equations. Consistency, stability, and convergence are distinct properties. A simulator should choose its step and integrator based on the dynamics, not merely the display frame interval or apparent smoothness of one trajectory.

## Prerequisites

- [[Definite integrals and the fundamental theorem]]
- [[Eigenvalues and eigenspaces]]
- [[Absolute and relative error]]

## Used by

- [[ODE integration and stability]]
- [[Time integrators and symplectic updates]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 838 (T03 · M051), 839 (T03 · M051), 845 (T03 · M051), 846 (T03 · M051).
