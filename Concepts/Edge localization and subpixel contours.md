---
tags:
  - concept
  - university-series
  - independent-series
---

# Edge localization and subpixel contours

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

An edge is a rapid image-intensity change, often associated with a material or depth boundary. Differentiating a noisy image amplifies high-frequency noise, so edge detectors first smooth with a Gaussian-like filter and then compute a gradient. Gradient magnitude estimates edge strength; gradient direction estimates the local normal. Nonmaximum suppression keeps a candidate only if it is a local maximum along that normal, thinning a broad response to a contour. Thresholding and hysteresis connect strong responses through weaker neighboring pixels. Subpixel localization fits a local peak to gradient samples, estimating a contour position between pixel centers. Accuracy depends on sampling, blur, contrast, and the assumed edge profile. This makes edge detection a measurement problem whose output uncertainty should propagate into later line, pose, or shape fitting.

## Prerequisites

- [[Convolution and filter footprints]]
- [[Sampling aliasing and reconstruction]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Hough transform and geometric voting]]
- [[Template matching and distance transforms]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]]
