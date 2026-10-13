---
tags:
  - concept
  - xr
  - independent-series
---

# Multiscale texture filtering

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Mipmaps prefilter texture scales; anisotropy reflects unequal footprint directions. Gaussian pyramids store coarse structure; Laplacian levels store differences between scales. A texel is not an output pixel, and interpolation cannot ensure lost information is recovered.

## Prerequisites

- [[Convolution and filter footprints]] — A weighting footprint establishes the filtered response, boundary effects and frequency transfer.
- [[Sampling aliasing and reconstruction]] — Allocation/filtering must respect what reduced samples can distinguish.

## Used by

- [[Antialiasing and sample allocation]]

## Recall and next depth

Explain a grazing-angle footprint and level log 2 rho. Pending: derivative-based GPU footprints and calibrated reconstruction. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Multiresolution work must preserve the right information|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
