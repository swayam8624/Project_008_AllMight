---
tags:
  - concept
aliases:
  - "symplectic integrator"
  - "semi-implicit Euler"
  - "Runge-Kutta"
---

# Time integrators and symplectic updates

**Curriculum depth:** Developed · **First seen:** Phase 9 · **Latest development:** Phase 9

## Definition and mechanism

A time integrator turns a differential equation into a discrete state update. Explicit Euler uses the current slope; midpoint and Runge–Kutta methods combine several slope evaluations to improve local accuracy; semi-implicit Euler updates velocity before position and can behave better for mechanical systems. Symplectic methods preserve the geometric structure of Hamiltonian dynamics, often keeping long-run energy error bounded rather than allowing systematic drift. No method is universally best: order, stability region, cost, conservation, and stiffness all matter. A higher formal order does not guarantee better behavior at an unsuitable step size or under discontinuous forces. Compare integrators on the same problem, step policy, and error target. Update order is part of the algorithm: changing it can change stability and physical behavior, not merely code style.

## Prerequisites

- [[ODE initial-value problems and stiffness]]

## Used by

- [[ODE integration and stability]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#ODEs and numerical simulation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 840 (T03 · M051), 841 (T03 · M051), 842 (T03 · M051), 843 (T03 · M051), 844 (T03 · M051).
