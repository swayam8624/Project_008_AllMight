---
tags:
  - concept
  - xr
  - independent-series
---

# Reflectance and rendering equation

**Curriculum depth:** Developed · **First seen:** XR01 · **Latest development:** University courses

## Definition and mechanism

A BRDF relates incident irradiance to outgoing radiance with directional weighting. The surface rendering equation adds emission and cosine-weighted incoming contributions. Visibility and transport assumptions matter; blurring completed output does not avoid shading.

## Prerequisites

- [[Light spectra and radiometry]] — The light quantity and units must be established before its interaction or visual effect is modeled.

## Used by

Later independent XR sources will extend this same node.
- [[Monte Carlo rendering estimators and variance]]
- [[Surface orientation from image brightness]]

- [[Reservoir resampling and ReSTIR]]
## Recall and next depth

Explain Lambertian rho/pi and identify what sampled shading approximates. Pending: transport estimators and actual workload profiling. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Materials turn incoming light into outgoing light|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## University course development

Extend the existing light-transport note with the distinction between a material’s directional response and the incident illumination it receives. At surface point $x$, outgoing radiance depends on emitted radiance plus an integral over incoming directions. The BRDF $f_r(x,\omega_i,\omega_o)$ is reflected outgoing radiance per incident irradiance, with units of inverse steradians; it depends on incoming direction $\omega_i$ and viewing direction $\omega_o$. A physically plausible BRDF is nonnegative and conserves energy, but can still have narrow glossy lobes that are difficult to sample. The rendering equation combines this local material response with visibility, incoming radiance, and the cosine foreshortening factor. This form explains why shading, direct lighting, global illumination, and Monte Carlo integration are connected parts of one transport calculation rather than independent image effects.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#The rendering equation and light transport|The rendering equation and light transport]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]]
