---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "PDF transformation"
  - "change of variables"
  - "density Jacobian"
---

# Probability density and change of measure

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** Phase 10

## Definition and mechanism

A probability density describes probability per unit of a chosen measure, not probability at a point. A density $p_A(a)$ over area differs from a density $p_\omega(\omega)$ over solid angle. When a mapping changes variables, its Jacobian converts density so total probability remains one: $p_B(b)=p_A(a)|da/db|$ in one dimension, with the determinant of the inverse Jacobian in multiple dimensions. In rendering, choosing a point on a light and using its direction from a shading point requires an area-to-solid-angle conversion involving distance squared and the light’s projected cosine. Estimator formulas are only correct when the sampling density and integral use the same measure. This concept prevents missing factors in importance sampling, environment-map sampling, and camera-lens integration.

### Phase 9–10 development

A density is meaningful relative to a measure and its support; its value at a point is not itself an event probability. For a continuous scalar variable, an interval probability is the integral of the density over that interval. Under a differentiable one-to-one change of variables, the transformed density includes the absolute determinant of the inverse Jacobian. Values outside the transformed support have zero probability. A mixed law can contain both point masses and a continuous part, so one PDF alone cannot describe every distribution. When a renderer changes a surface-area density into a solid-angle density, the geometric Jacobian must preserve the same probability.

## Prerequisites

- [[Light spectra and radiometry]]
- [[Random variables and probability laws]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Monte Carlo rendering estimators and variance]]

- [[Attention and visual transformers]]
- [[Contrastive vision and image-language alignment]]
- [[Latent-variable models and generative flows]]
- [[Perception feedback and robot learning]]
- [[Radiance fields and Gaussian splatting]]
- [[Jacobians, pushforwards, and pullbacks]]
- [[Inverse, rejection, and importance sampling]]
- [[Multivariate Gaussian distributions and whitening]]

- [[Latin-hypercube sampling]]
- [[Stable probability normalization and log-sum-exp]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Light measurement and radiometry|Light measurement and radiometry]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Lens optics, focus, and image formation|Lens optics, focus, and image formation]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#2. Random quantities and their probability laws|Phase 10 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#7. Transformations turn simple inputs into useful samples|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
