---
tags:
  - concept
  - xr
  - independent-series
---

# Sampling aliasing and reconstruction

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Uniform samples identify frequencies only modulo sample rate and corresponding real-signal equivalences. Strictly band-limited ideal reconstruction needs non-overlapping spectral replicas; exact-Nyquist phase is a boundary hazard. Prefilter before decimation.

## Prerequisites

- [[Fourier analysis and spectral phase]] — Frequency and phase reveal transfer, sampling ambiguity and structural loss.

## Used by

- [[Multiscale texture filtering]]
- [[Spatiotemporal sampling and retinal motion]]
- [[Antialiasing and sample allocation]]

## Recall and next depth

Prove 13Hz and 3Hz sample identity at 10Hz. Pending: anisotropic/nonuniform sampling and actual reconstruction errors. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Sampling is a question of what remains distinguishable|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
