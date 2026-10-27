---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "ReSTIR"
  - "reservoir sampling"
  - "reservoir resampling"
  - "RIS"
  - "resampled importance sampling"
  - "temporal sample reuse"
  - "spatial sample reuse"
---

# Reservoir resampling and ReSTIR

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Reservoir sampling keeps a compact candidate for a weighted distribution without storing every sample. When candidate $y_i$ has target weight $w_i$, maintain a running weight sum and replace the stored candidate with probability proportional to $w_i$; the reservoir records enough normalization information to estimate the desired integral without bias under stated assumptions. ReSTIR reuses candidates across pixels and frames, which is valuable when direct lighting has many possible lights or paths but only a few can be evaluated per pixel. Reuse requires correcting for the source and target distributions, accounting for visibility changes, and managing temporal history. If weights or normalization are wrong, the image may be biased; stale reuse causes trails or light leaks when geometry or illumination moves. The method reduces expensive samples rather than making sampling free. It depends on a valid estimator, careful normalization, and an understanding of whether correlated reused samples preserve the intended expectation.

## Prerequisites

- [[Monte Carlo rendering estimators and variance]]
- [[Sampling aliasing and reconstruction]]
- [[Temporal reprojection and history validation]]
- [[Reflectance and rendering equation]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]]

## Recall and next depth

- Trace the running weight sum and replacement probability for one reservoir update.
- Which correction is needed when a sample is reused under a different target distribution?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#The rendering equation and light transport|The rendering equation and light transport]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Acceleration structures and robust queries|Acceleration structures and robust queries]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-15 - Ray Tracing|P3-15]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-19 - Shadows Part 2|P3-19]].
