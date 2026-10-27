---
tags:
  - concept
  - xr
  - independent-series
---

# Temporal reprojection and history validation

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Reprojection aligns history with a current target; disocclusion exposes content with no matching old observation. Validated blending trades noise against response/ghosting. Gaze-driven quality changes also change history reliability; TAA is not exposure integration.

## Prerequisites

- [[Spatiotemporal sampling and retinal motion]] — Temporal reuse must respect changing scene and retinal observations.
- [[Antialiasing and sample allocation]] — History depends on the actual current/prior sampling strategy.

## Used by

- [[Foveated rendering and uncertainty]]

- [[Reservoir resampling and ReSTIR]]
## Recall and next depth

Derive ideal history variance and identify every violated assumption. Pending: motion/depth/jitter conventions and robust implementation. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Temporal reconstruction is an inference about correspondence|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
