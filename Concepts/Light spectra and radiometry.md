---
tags:
  - concept
  - xr
  - independent-series
---

# Light spectra and radiometry

**Curriculum depth:** Developed · **First seen:** XR01 · **Latest development:** University courses

## Definition and mechanism

Light models distinguish wave propagation, photon energy and geometrical rays. Radiometry describes physical flux/radiance/irradiance; photometry adds standardized visual weighting. RGB requires an interpretation and cannot identify a unique spectrum.

## Prerequisites

No formal graph prerequisite. Begin with the stated units and model; this is an independent-series foundation.

## Used by

- [[Reflectance and rendering equation]]
- [[Retinal encoding and receptive fields]]
- [[Optical focus and binocular conflict]]
- [[Color encoding and linear-light operations]]
- [[Probability density and change of measure]]
- [[Surface orientation from image brightness]]

## Recall and next depth

Explain W, W/m², W/(m²·sr), lux and nits. Pending: spectral/colorimetric calibration. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Light must become a quantity before it becomes a color|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## University course development

Radiometry distinguishes total radiant flux $\Phi$ in watts, irradiance $E$ in watts per square metre, radiant intensity $I$ in watts per steradian, and radiance $L$ in watts per square metre per steradian measured with projected source area. A spectrum distributes the energy over wavelength; a three-channel RGB value is a measurement in a chosen color system and does not uniquely identify that spectrum. Solid angle measures directional extent rather than physical area. Consequently an area-light sample requires the projected-area and inverse-square Jacobian when expressed as a directional sample. Free-space radiance remains constant along an unobstructed ray, while irradiance from a point-like source changes with geometry. Photometry instead applies standardized visual weighting: luminous flux uses lumens, illuminance lux, and luminous intensity candela. These distinctions prevent treating brightness, exposure, flux and radiance as interchangeable scalar labels.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Light measurement and radiometry|Light measurement and radiometry]]
