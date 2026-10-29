---
tags:
  - concept
aliases: []
---

# Omnidirectional camera projections

**Curriculum depth:** Developed · **First seen:** Phase 11 · **Latest development:** Phase 11

## Definition and mechanism

Cubemaps, equirectangular panoramas, and fisheye lenses map unit directions onto different image domains. A cubemap selects one of six faces and projects two direction components; equirectangular coordinates map longitude and latitude to a rectangle; fisheye mappings use a radial function of angular distance. These parameterizations are not equal-area: uniform pixel sampling does not imply uniform solid-angle sampling. Choose the mapping that matches capture or rendering needs, define face orientation and seam rules, and measure reconstruction with angular error between unit vectors. Fisheye formulas also differ (equidistant, equisolid, orthographic, stereographic), so name the model before interpreting pixels. At the poles, longitude is undefined, so seam and pole handling need explicit sampling rules.

## Prerequisites

- [[Visual angle and calibrated rays]]
- [[Camera calibration and bundle adjustment]]
- [[Unit directions and safe normalization]]

## Used by

- No downstream concept node is added in this stage.

## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Represent directions over the full sphere|Represent directions over the full sphere]]

## Source coverage

Supplied lesson parts: 1089–1091.

## Future keywords

cubemap seams, equirectangular map, fisheye model
