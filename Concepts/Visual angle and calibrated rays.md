---
tags:
  - concept
  - xr
  - independent-series
---

# Visual angle and calibrated rays

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Visual angle depends on geometry, not an image's pixel count alone. Eccentricity is the angle between calibrated unit gaze/pixel directions. Average PPD differs from local density under a rectilinear mapping; UV distance is dimensionless.

## Prerequisites

- [[Optical focus and binocular conflict]] — Optical image formation determines the direction/focus assumptions used by geometry and binocular analysis.

## Used by

- [[Visual acuity and eccentricity]]
- [[Eye movements and display-time gaze]]

- [[Omnidirectional camera projections]]
- [[Screen-space ray generation and mouse picking]]
- [[Ray differentials and camera sampling measures]]

## Recall and next depth

Derive center PPD from pixel pitch separately from average PPD. Pending: distortion mappings and measured headset rays. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Angles connect display samples to a viewer|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## Phase 11 camera development

A calibrated pixel maps to a viewing direction through inverse intrinsics and a declared distortion model. Nearby pixels define nearby directions; their derivatives estimate how a finite pixel footprint expands on a surface. Visual angle measures directional separation, while ray differentials describe local change in generated rays and may include lens sampling. For fisheye or omnidirectional cameras, angular footprint varies across the image. Transform sampling densities with the correct Jacobian when changing from pixel area to solid angle. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
