---
tags:
  - concept
  - university-series
  - independent-series
---

# Lens optics and depth of field

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A pinhole camera maps every scene point to one image point through a single center, but a real lens has a finite aperture. Rays from one object point pass through different parts of the aperture and meet at a focused image point only for the chosen focus distance. Points nearer or farther from that plane form a blur disk called the circle of confusion. Stopping down the aperture narrows the cone of rays and increases depth of field, but reduces collected light and may require longer exposure or higher sensor gain. Focal length and focus distance also affect field of view and perspective magnification. A thin-lens model is an approximation; real lenses add aberrations and vignetting. The model helps explain why depth of field is both an optical effect and a sampling/integration process over aperture positions.

## Prerequisites

- [[Camera view and clip-space pipeline]]
- [[Reflection refraction and shading directions]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Exposure and sensor response]]

- [[Ray differentials and camera sampling measures]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Photographic composition, history, and communication|Photographic composition, history, and communication]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Photographic workflow and integrated decisions|Photographic workflow and integrated decisions]]

## Phase 11 camera development

A pinhole camera emits one ideal ray per image location. A finite-aperture lens samples a family of rays whose intersections form a blur disk away from the focal plane. A thin-lens camera therefore samples film and lens position, then aims through a focal point. Differentials from film offsets alone omit aperture variation unless the model includes it. Monte Carlo PDFs and camera weights must match the sampled film/aperture measure. The thin-lens model approximates real optics; aberrations and bokeh shape require richer models. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
