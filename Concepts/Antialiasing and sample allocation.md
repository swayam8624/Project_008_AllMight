---
tags:
  - concept
  - xr
  - independent-series
aliases:
  - "adaptive sample allocation"
---

# Antialiasing and sample allocation

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** Phase 10

## Definition and mechanism

SSAA increases spatial observations; MSAA principally adds coverage-related sampling under its shading conventions. TAA reuses aligned temporal observations. Nonlinear shading makes pre-/post-filtering noncommutative, so coarse rates need appropriate input/output policies.

### Phase 9–10 development

Antialiasing combines a sampling pattern with a reconstruction or integration footprint so unresolved variation contributes with controlled error. More samples can reduce Monte Carlo variance, while better placement can reduce structured error; these are different effects. Adaptive allocation based on observed samples changes the sampling procedure and can affect uncertainty estimates. Spatial sample allocation should remain distinct from importance sampling over path or event space.

## Prerequisites

- [[Sampling aliasing and reconstruction]] — Allocation/filtering must respect what reduced samples can distinguish.
- [[Multiscale texture filtering]] — Coarse samples need appropriate prefilter/footprint representations.
- [[Monte Carlo rendering estimators and variance]]
- [[Sampling quality: stratification, quasi-Monte Carlo, and blue noise]]

## Used by

- [[Temporal reprojection and history validation]]

## Recall and next depth

Explain mean(x²) versus square(mean(x)). Pending: actual shader invocation reduction and VRS/GPU execution. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Multiresolution work must preserve the right information|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#6. A computer produces deterministic pseudorandom sequences|Phase 10 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#12. Use probability to evaluate graphics and research claims|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
