---
tags:
  - concept
  - university-series
  - independent-series
---

# Template matching and distance transforms

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Template matching searches an image for a known pattern by comparing a small reference patch at candidate positions. Sum of squared differences penalizes intensity mismatch; normalized correlation compensates for brightness scale and offset. Direct search costs work proportional to image area times template area, while convolution or Fourier methods can accelerate broad searches. Rotation and scale changes require transforming the template or searching additional parameters. For contour-based matching, a distance transform stores each pixel’s distance to the nearest edge. A template edge point then receives a low cost when it lands near an observed contour even if the exact pixels do not align. The chamfer score sums these distances over template points. These methods suit industrial inspection where the target geometry is known, but need explicit thresholds for occlusion, deformation, and false positives.

## Prerequisites

- [[Convolution and filter footprints]]
- [[Edge localization and subpixel contours]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]]
