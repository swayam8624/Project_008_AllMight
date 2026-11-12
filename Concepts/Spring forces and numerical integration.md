---
tags: [concept, physics, game-engine]
aliases: ["Hooke's law", "spring force", "slack bungee"]
---

# Spring forces and numerical integration

For displacement from an endpoint to a particle, the ideal spring force preserves the sign of extension: stretching pulls toward the endpoint; compression pushes away. A bungee has zero force while slack and pulls only when stretched. Coincident endpoints need an explicit direction policy. Force magnitude alone cannot detect a direction error.

The force law and the time integrator are separate contracts. An undamped oscillator grows under forward Euler at every positive step size; semi-implicit Euler has a different bounded interval. An analytic target position does not become a reachable current-step position merely by converting it to force. Physical stiffness, mass-normalized coefficients and numerical damping settings need distinct units and definitions.

## Prerequisites

- [[Concepts/Norms lengths and induced distances]]
- [[Concepts/Partial derivatives and gradients]]
- [[Concepts/ODE integration and stability]]

## Full teaching and executable evidence

- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A spring law is signed, not just a magnitude|Signed force law, potential-energy derivation and figure]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Solve each linear damping regime with its own form|Underdamped, critical and overdamped solutions]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Standalone spring-direction and integration-order laboratory|Complete C++20 witness and its limits]]

## Used by

Force generators, elastic tethers and penalty-force approximations. A shared force equation does not establish that a contact or joint solver has the same stability properties.
