---
tags:
  - concept
  - xr
  - independent-series
---

# Convolution and filter footprints

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Convolution combines shifted values with a weighting kernel; its spectrum multiplies transfer functions. Pixel footprints and boundary policies define what is averaged. A separable Gaussian uses two 1D passes; unpadded DFT filtering wraps circularly.

## Prerequisites

- [[Continuous signals and discrete samples]] — Filtering/frequency analysis operates on a declared domain and its samples.
- [[Fourier analysis and spectral phase]] — Frequency and phase reveal transfer, sampling ambiguity and structural loss.

## Used by

- [[PSF OTF and MTF]]
- [[Multiscale texture filtering]]

## Recall and next depth

Derive the three-tap filter's gain and constant preservation. Pending: implementation cost, padding and spatially varying kernels. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#A footprint is more meaningful than an isolated pixel center|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
