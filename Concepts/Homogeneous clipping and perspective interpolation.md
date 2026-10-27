---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "clip space"
  - "homogeneous clipping"
  - "perspective-correct interpolation"
  - "divide by w"
  - "clip volume"
  - "NDC"
---

# Homogeneous clipping and perspective interpolation

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

After a vertex is transformed, its clip-space coordinate is $(x_c,y_c,z_c,w_c)$. Clip primitives against the view volume before perspective division; under the conventional OpenGL clip convention the inequalities are $-w_c\le x_c,y_c,z_c\le w_c$. A crossing edge is split by interpolating its homogeneous endpoints, which preserves the projective line and avoids dividing by a small or negative $w$ too early. Only surviving points are divided by $w_c$ to enter normalized device coordinates. Attributes then need perspective-correct interpolation: screen-space barycentric weights interpolate $a/w$ and $1/w$, and the fragment value is their ratio. For a triangle with weights $\lambda_i$, $a=(\sum_i\lambda_i a_i/w_i)/(\sum_i\lambda_i/w_i)$. Direct linear interpolation in screen space is correct only for affine quantities or equal depth; otherwise textures warp. Clip-space depth conventions and the near/far interval vary by API and projection setup, so the inequalities must match the active pipeline.

## Prerequisites

- [[Homogeneous coordinates and perspective division]]
- [[Camera view and clip-space pipeline]]
- [[Barycentric coordinates and interpolation]]
- [[Triangle rasterization and coverage tests]]
- [[Depth buffering and visibility order]]
- [[GPU vertex specification]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]]

## Recall and next depth

- Derive the clip-space edge intersection and perspective-correct attribute ratio.
- Why must clipping happen before divide by w, and why is linear screen interpolation wrong for texture coordinates?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]; [[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code#Homogeneous coordinates preserve the role of a location|Homogeneous coordinates preserve the role of a location]]; [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions#A camera is another frame, so viewing uses its inverse|A camera is another frame, so viewing uses its inverse]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P1-02 - Module 1.2: Course Intro & Administrivia Part 2 Graphics & Visual Computing 2026|P1-02]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-05 - The Camera Transform|P3-05]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-06 - Clipping|P3-06]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#P3-12 - Scan Conversion|P3-12]].
