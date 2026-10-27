---
tags:
  - concept
  - xr
  - independent-series
---

# Sampling aliasing and reconstruction

**Curriculum depth:** Developed · **First seen:** XR01 · **Latest development:** University courses

## Definition and mechanism

Uniform samples identify frequencies only modulo sample rate and corresponding real-signal equivalences. Strictly band-limited ideal reconstruction needs non-overlapping spectral replicas; exact-Nyquist phase is a boundary hazard. Prefilter before decimation.

## Prerequisites

- [[Fourier analysis and spectral phase]] — Frequency and phase reveal transfer, sampling ambiguity and structural loss.

## Used by

- [[Multiscale texture filtering]]
- [[Spatiotemporal sampling and retinal motion]]
- [[Antialiasing and sample allocation]]
- [[Triangle rasterization and coverage tests]]
- [[Edge localization and subpixel contours]]
- [[PDE fields and discrete Laplacians]]

- [[Learned image features and CNNs]]
- [[Radiance fields and Gaussian splatting]]
- [[Reservoir resampling and ReSTIR]]
## Recall and next depth

Prove 13Hz and 3Hz sample identity at 10Hz. Pending: anisotropic/nonuniform sampling and actual reconstruction errors. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Sampling is a question of what remains distinguishable|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## University course development

Sampling records measurements of a continuous signal on a discrete grid. Frequencies that differ by integer multiples of the sampling rate can produce the same samples, so the sampled values alone cannot identify which original signal was present. For ideal band-limited reconstruction, the signal must be restricted below half the sampling rate; equality at the limiting frequency needs additional phase and endpoint conditions. A real pixel also integrates light over area and exposure time, making its measurement a filtered sample. Prefiltering attenuates frequencies that would alias, while reconstruction estimates a continuous signal from the stored samples. Ideal sinc reconstruction has infinite support; practical filters trade computation, sharpness, ringing and residual aliasing. More samples reduce integration error only when their placement and weights match the intended pixel or path-space measure.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sampling continuous signals and image formation|Sampling continuous signals and image formation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]]
