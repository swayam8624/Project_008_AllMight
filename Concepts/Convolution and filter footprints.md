---
tags:
  - concept
  - xr
  - independent-series
---

# Convolution and filter footprints

**Curriculum depth:** Developed · **First seen:** XR01 · **Latest development:** University courses

## Definition and mechanism

Convolution combines shifted values with a weighting kernel; its spectrum multiplies transfer functions. Pixel footprints and boundary policies define what is averaged. A separable Gaussian uses two 1D passes; unpadded DFT filtering wraps circularly.

## Prerequisites

- [[Continuous signals and discrete samples]] — Filtering/frequency analysis operates on a declared domain and its samples.
- [[Fourier analysis and spectral phase]] — Frequency and phase reveal transfer, sampling ambiguity and structural loss.

## Used by

- [[PSF OTF and MTF]]
- [[Multiscale texture filtering]]
- [[Optical flow and time to contact]]
- [[Edge localization and subpixel contours]]
- [[Template matching and distance transforms]]
- [[PDE fields and discrete Laplacians]]

## Recall and next depth

Derive the three-tap filter's gain and constant preservation. Pending: implementation cost, padding and spatially varying kernels. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#A footprint is more meaningful than an isolated pixel center|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## University course development

Extend the current convolution note from a sliding kernel to image formation and resampling. A pixel integrates incoming image energy over a finite footprint; point sampling is only an approximation to that area integral. Convolution combines nearby values with weights whose spatial support is the kernel footprint, and its Fourier transform describes which spatial frequencies survive. Before reducing resolution, low-pass filtering suppresses frequencies the coarser grid cannot represent. In texture mapping, the screen-to-texture Jacobian maps a nearly square screen pixel to a texture-space parallelogram. If the two singular directions have very different lengths, an isotropic mip level blurs one direction too much; anisotropic filters take extra samples along the long axis. This same footprint reasoning links pixel reconstruction, antialiasing, texture filtering, and sensor blur without treating them as unrelated tricks.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Textures and surface detail|Textures and surface detail]]
