---
tags:
  - concept
  - xr
  - independent-series
---

# Color encoding and linear-light operations

**Curriculum depth:** Developed · **First seen:** XR01 · **Latest development:** University courses

## Definition and mechanism

Color primaries/white point and nonlinear transfer encoding are different. Physical intensity filtering uses the appropriate linear quantity; black/white averaging then sRGB encoding gives about 0.735, not encoded 0.5. Tone/gamut mapping and absolute display luminance need separate contracts.

## Prerequisites

- [[Light spectra and radiometry]] — The light quantity and units must be established before its interaction or visual effect is modeled.
- [[Interpretation contract]] — A numeric sample requires a domain, units and encoding convention.

## Used by

- [[Perceptual metrics and display calibration]]
- [[Exposure and sensor response]]

## Recall and next depth

State the metric's encoding and peak before comparing images. Pending: standardized HDR and measured display calibration. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Encoded RGB must not masquerade as linear light|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## University course development

Extend the existing color note with why image-processing arithmetic must distinguish encoded values from light. A display encoding such as sRGB uses a nonlinear transfer curve so code values allocate precision in a way that suits perception and storage. Averaging or adding those encoded numbers does not generally average or add emitted light. Decode to linear-light RGB before blending, filtering, estimating radiance, or compositing, then encode for display. If two equal-area samples contain linear intensities $0$ and $1$, their physical average is $0.5$; averaging their encoded sRGB values and treating that result as light yields a different, brighter answer. Color primaries and a white point also define the coordinate system of RGB, so a numeric triplet is incomplete without its color space. Keep scene-referred radiance, display-referred color, and perceptual appearance separate.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Color perception, encoding, and display|Color perception, encoding, and display]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Light measurement and radiometry|Light measurement and radiometry]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]]
