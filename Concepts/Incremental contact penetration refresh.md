---
tags: [concept, physics, game-engine]
aliases: ["penetration refresh", "neighboring contact update"]
---

# Incremental contact penetration refresh

Position correction moves every contact point attached to the corrected bodies. For a small rotation, its first-order point displacement is translation plus rotation vector crossed with the pre-correction world-space lever arm. Project the difference of the two point movements onto the frozen contact normal, using the declared normal and penetration signs.

Refresh each known contact once for the correction just applied, then reconsider the worst penetration. Reusing an old correction counts movement twice. This local estimate cannot discover new contacts or replace geometry refresh after substantial rotation or a feature change.

## Prerequisites

- [[Concepts/Cross products and oriented area]]
- [[Concepts/Unit directions and safe normalization]]

## Full teaching and executable evidence

- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Refresh neighboring penetrations after position correction|Derivation, three-contact ranking trace and complexity limits]]
- [[Continuous Notes/Game Engine - From First Frame to Living Worlds#A planar penetration refresh witness|Complete planar C++20 reference]]

## Used by

Iterative position correction and contact-priority updates. The reference is a frozen-feature updater, not a full collision detector or rigid-body solver.
