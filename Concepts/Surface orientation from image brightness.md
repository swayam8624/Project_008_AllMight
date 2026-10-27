---
tags:
  - concept
  - university-series
  - independent-series
---

# Surface orientation from image brightness

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Shape-from-illumination methods infer surface orientation or depth from how light changes across an image. Photometric stereo observes a fixed camera view under multiple known light directions; for a Lambertian surface, measured intensity is proportional to albedo times the dot product between light direction and surface normal. Solving several such equations estimates the scaled normal, then integration recovers height up to boundary and scale ambiguities. A reflectance map plots brightness as a function of surface normal under fixed illumination and can reveal which orientations are visible. The extended Gaussian image maps each surface area to its normal on the unit sphere; its distribution summarizes orientation and convex shape properties. Shape-from-shading uses one image and must jointly reason about unknown lighting, albedo, and depth, so it is more ambiguous. Each method needs assumptions stated explicitly.

## Prerequisites

- [[Reflectance and rendering equation]]
- [[Light spectra and radiometry]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surface orientation from illumination|Surface orientation from illumination]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Shape from shading and reflectance|Shape from shading and reflectance]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Surface descriptions and Gaussian images|Surface descriptions and Gaussian images]]
