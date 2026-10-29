---
tags: [concept, phase-7]
---

# Camera view and clip-space pipeline

**Curriculum depth:** Developed · **First seen:** Phase7 · **Latest development:** University courses

## Definition and mechanism

A camera pose maps camera coordinates to world coordinates; its inverse is the view map. Projection produces homogeneous clip coordinates, clipping applies the API-specific inequalities, and valid perspective division produces NDC. Storage order supplies none of these coordinate contracts.

**Reconstruction cue:** A standard minus-Z perspective example needs a point in front of the camera and a meaningful clip w before division.

## Prerequisites

- [[Coordinate frames and change of basis]]

## Used by

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Phase 7 teaching and laboratory]]
- [[Depth buffering and visibility order]]
- [[Lens optics and depth of field]]

- [[GPU vertex specification]]
- [[Homogeneous clipping and perspective interpolation]]
- [[Axial depth and Euclidean range]]
- [[Hardware depth-buffer linearization]]
- [[Screen-space ray generation and mouse picking]]
- [[Ray differentials and camera sampling measures]]

## Recall and next depth

Reconstruct the example, state its conventions and find an input that violates its preconditions. Deeper algorithms and measured production performance remain separate from the present model.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|Continuous explanation]] · [[Supplementary/Foundations#Phase 7 - Parts 601-700|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 7 - Source analysis and verification|Sources and verification]]

## University course development

Extend the existing transform pipeline with the clipping and depth-precision decisions that make projection robust. A view frustum is the set of points a camera can see, bounded by near and far planes as well as image sides. Transforming the frustum into a canonical clip cube makes outside tests cheap; partially intersecting triangles must be clipped and triangulated so the rasterizer still receives triangles. The near plane also avoids geometry crossing the eye plane, while the far plane limits depth range. Floating-point depth values are unevenly distributed after perspective division, so an excessively tiny near distance paired with a huge far distance can make two nearby surfaces quantize to the same depth and produce z-fighting. Choose the narrowest useful depth interval and keep the view, clip, normalized, and screen coordinates distinct.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A scene description becomes an image|A scene description becomes an image]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Rasterization, coverage, and depth|Rasterization, coverage, and depth]]

## Phase 11 camera development

A raster-to-ray camera maps a pixel to a geometric sample; In a pinhole model, invert projection and view transforms to recover a camera-space direction, then transform it to world space without applying translation as if it were a point. Pixel-center convention, viewport origin, handedness, clip-depth range, and normalization all affect the result. Offset a pixel to generate differential rays and estimate the surface footprint; that footprint grows with depth and changes with surface orientation. Keep ray generation distinct from raster projection while ensuring their conventions agree. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
