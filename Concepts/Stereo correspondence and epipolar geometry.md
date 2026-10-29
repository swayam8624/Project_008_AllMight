---
tags:
  - concept
  - university-series
  - independent-series
---

# Stereo correspondence and epipolar geometry

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Stereo reconstruction uses two or more views to infer depth from corresponding image features. A 3D point and the two camera centers define an epipolar plane; its intersection with each image plane is an epipolar line. Thus a feature in one image should match along a one-dimensional line in the other, reducing a 2D search to a constrained search. With calibrated cameras, triangulation intersects the two viewing rays; noise means they are skew, so estimate the 3D point minimizing reprojection or ray distance. Depth uncertainty grows rapidly when the viewing rays are nearly parallel, which happens for distant points or small baselines. Rectification rotates images so epipolar lines become horizontal scanlines. Occlusion, repetitive texture, and calibration error can still produce mismatches, so confidence and consistency checks are essential.

## Prerequisites

- [[Camera calibration and bundle adjustment]]
- [[Barycentric coordinates and interpolation]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Off-axis stereo projection and disparity]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]]
