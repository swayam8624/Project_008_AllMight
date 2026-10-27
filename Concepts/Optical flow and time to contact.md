---
tags:
  - concept
  - university-series
  - independent-series
---

# Optical flow and time to contact

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Optical flow is the apparent 2D motion of image features between frames. Under brightness constancy, a tracked point keeps approximately the same intensity as it moves, yielding the constraint $I_xu+I_yv+I_t=0$, where spatial derivatives $I_x,I_y$ and temporal derivative $I_t$ are measured from the image and $(u,v)$ is image velocity. One equation cannot determine two unknown components at a single pixel, so algorithms add neighborhood smoothness, feature tracking, or multiple scales. Flow can reveal camera motion, object motion, and looming. For an object approaching at speed $V$, its image size grows at a rate related to depth; the ratio of size to its time derivative estimates time to contact without separately knowing absolute scale under simplifying assumptions. Texture, occlusion, and illumination changes violate brightness constancy.

## Prerequisites

- [[Spatiotemporal sampling and retinal motion]]
- [[Convolution and filter footprints]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optical flow and motion from images|Optical flow and motion from images]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Photographic composition, history, and communication|Photographic composition, history, and communication]]
