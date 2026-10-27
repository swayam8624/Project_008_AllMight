---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "NeRF"
  - "neural radiance field"
  - "3DGS"
  - "3D Gaussian splatting"
  - "2DGS"
  - "2D Gaussian splatting"
  - "novel view synthesis"
  - "radiance field"
---

# Radiance fields and Gaussian splatting

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A radiance field represents a scene by the color and density seen along each 3D viewing ray. NeRF maps a position and viewing direction through a learned function, then volume-renders samples using transmittance and opacity weights; multiple calibrated photographs supervise the result. This representation handles view-dependent appearance but can require many ray samples and careful camera alignment. 3D Gaussian splatting instead stores anisotropic colored Gaussian primitives in space and projects them to screen, sorting and blending their footprints for fast interactive views. 2D Gaussian splatting uses oriented surface-like splats to improve geometric consistency. These methods trade continuous function evaluation for explicit primitives and rasterization. Sparse or biased views leave unseen regions unconstrained; floaters, blur, aliasing, incorrect transparency ordering, or poor geometry can remain even when training views look convincing. Novel-view quality is not equivalent to a watertight mesh or physically correct material. Reconstruction quality depends on camera poses, coverage, calibration, and the scene's changing illumination.

## Prerequisites

- [[Camera calibration and bundle adjustment]]
- [[Sampling aliasing and reconstruction]]
- [[Probability density and change of measure]]
- [[Bounding volume hierarchies]]
- [[Ray-triangle intersection]]
- [[Automatic differentiation and backpropagation]]
- [[Latent-variable models and generative flows]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learned representations of 3D shape and scenes|Learned representations of 3D shape and scenes]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learned representations of 3D shape and scenes|Learned representations of 3D shape and scenes]]

## Recall and next depth

- Contrast ray-sampled density compositing with projected Gaussian primitive blending.
- What can excellent training-view synthesis fail to establish about unseen geometry?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learned representations of 3D shape and scenes|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#The rendering equation and light transport|The rendering equation and light transport]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Visibility queries and ray intersections|Visibility queries and ray intersections]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Meshes, connectivity, and geometry processing|Meshes, connectivity, and geometry processing]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-15 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 15: 3D Vision|SC-15]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-13 - Curves and Surfaces|P3-13]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-15 - Ray Tracing|P3-15]].
