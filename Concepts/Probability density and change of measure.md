---
tags:
  - concept
  - university-series
  - independent-series
---

# Probability density and change of measure

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A probability density describes probability per unit of a chosen measure, not probability at a point. A density $p_A(a)$ over area differs from a density $p_\omega(\omega)$ over solid angle. When a mapping changes variables, its Jacobian converts density so total probability remains one: $p_B(b)=p_A(a)|da/db|$ in one dimension, with the determinant of the inverse Jacobian in multiple dimensions. In rendering, choosing a point on a light and using its direction from a shading point requires an area-to-solid-angle conversion involving distance squared and the light’s projected cosine. Estimator formulas are only correct when the sampling density and integral use the same measure. This concept prevents missing factors in importance sampling, environment-map sampling, and camera-lens integration.

## Prerequisites

- [[Light spectra and radiometry]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Monte Carlo rendering estimators and variance]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Light measurement and radiometry|Light measurement and radiometry]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Lens optics, focus, and image formation|Lens optics, focus, and image formation]]
