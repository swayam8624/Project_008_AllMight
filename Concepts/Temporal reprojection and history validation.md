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

## Phase 11 camera development

Reprojection predicts where a prior sample lands under camera and object motion. Depth agreement tests whether the same visible surface likely occupies that location, but cannot prove stored color remains valid after lighting, exposure, material, or sampling changes. Combine projected position, depth tolerance, and disocclusion checks; reduce history weight when confidence is low. Monocular reconstruction has arbitrary global scale, so metric depth thresholds require an external scale or consistent units. Reprojection success is geometric evidence, not a photometric guarantee. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
