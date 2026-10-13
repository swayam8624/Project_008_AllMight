---
tags:
  - concept
  - xr
  - independent-series
---

# Antialiasing and sample allocation

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

SSAA increases spatial observations; MSAA principally adds coverage-related sampling under its shading conventions. TAA reuses aligned temporal observations. Nonlinear shading makes pre-/post-filtering noncommutative, so coarse rates need appropriate input/output policies.

## Prerequisites

- [[Sampling aliasing and reconstruction]] — Allocation/filtering must respect what reduced samples can distinguish.
- [[Multiscale texture filtering]] — Coarse samples need appropriate prefilter/footprint representations.

## Used by

- [[Temporal reprojection and history validation]]

## Recall and next depth

Explain mean(x²) versus square(mean(x)). Pending: actual shader invocation reduction and VRS/GPU execution. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Multiresolution work must preserve the right information|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
