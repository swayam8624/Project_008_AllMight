---
tags:
  - concept
  - xr
  - independent-series
---

# Eye movements and display-time gaze

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Fixation, saccades, pursuit and VOR have different dynamics. Sampling rate differs from delivery/end-to-end delay. A rendering allocation should target visible display time, with phase-specific errors and uncertainty rather than a universal constant-velocity assumption.

## Prerequisites

- [[Visual angle and calibrated rays]] — Angular coordinates make sampling density and gaze-relative location physically meaningful.

## Used by

- [[Spatiotemporal sampling and retinal motion]]
- [[Foveated rendering and uncertainty]]

## Recall and next depth

Explain a gaze-landing failure despite low average angular error. Pending: tracking, fusion, prediction and timing pipelines. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#A gaze sample needs a time as well as a direction|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]
