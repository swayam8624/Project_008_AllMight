---
tags:
  - concept
  - university-series
  - independent-series
---

# Hough transform and geometric voting

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

The Hough transform detects a parametric shape by letting each image feature vote for compatible shape parameters. A line can be represented by normal angle $\theta$ and signed distance $\rho$, avoiding the vertical-line singularity of slope-intercept form. Each edge point votes along a curve in $(\rho,\theta)$ space; a bin with many votes indicates a line supported by many image points. Circle detection adds center and radius parameters, increasing accumulator dimension and cost. Quantization controls a tradeoff: coarse bins merge nearby hypotheses but reduce precision; fine bins need more votes and memory. Gradient direction can restrict votes and reduce clutter. The transform tolerates missing edge segments because evidence accumulates globally, but unrelated edges can also create peaks, so candidates need geometric validation and robust fitting.

## Prerequisites

- [[Edge localization and subpixel contours]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]]
