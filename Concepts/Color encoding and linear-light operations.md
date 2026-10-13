---
tags:
  - concept
  - xr
  - independent-series
---

# Color encoding and linear-light operations

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Color primaries/white point and nonlinear transfer encoding are different. Physical intensity filtering uses the appropriate linear quantity; black/white averaging then sRGB encoding gives about 0.735, not encoded 0.5. Tone/gamut mapping and absolute display luminance need separate contracts.

## Prerequisites

- [[Light spectra and radiometry]] — The light quantity and units must be established before its interaction or visual effect is modeled.
- [[Interpretation contract]] — A numeric sample requires a domain, units and encoding convention.

## Used by

- [[Perceptual metrics and display calibration]]

## Recall and next depth

State the metric's encoding and peak before comparing images. Pending: standardized HDR and measured display calibration. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Encoded RGB must not masquerade as linear light|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
