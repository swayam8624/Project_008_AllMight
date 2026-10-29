---
tags: [concept, phase-11]
---

# Ray differentials and camera sampling measures

**Curriculum depth:** Developed · **First seen:** Phase 11

## Definition and mechanism

A camera sample selects a film location and usually a shutter time, then maps that sample to a weighted ray or path contribution. Ray differentials approximate how neighboring pixel samples change the generated ray; after intersection they estimate the surface footprint covered by a pixel and help choose a texture filter level. A sampling measure states which quantity is sampled and therefore which probability density belongs in a Monte Carlo estimator; area, solid angle, image-plane area, and discrete pixel measures are not interchangeable without a Jacobian. Example: two adjacent pixels generate rays whose intersection points span a large patch on a distant oblique surface, so filtering should average that footprint rather than use a single high-frequency texel. Differential rays are local first-order approximations and can fail at discontinuities or complex lens mappings; the weight and PDF must match the camera model and chosen measure.

**Example:** For a pinhole camera, offset the raster sample by one pixel in x and y, generate neighboring rays, and intersect them with a textured plane; the resulting parallelogram estimates the pixel footprint for texture filtering.

**Assumptions:** Differentials approximate nearby rays locally and are not exact at visibility or lens discontinuities.; Importance weights/PDFs are valid only when defined in the same measure as the estimator.

## Prerequisites

- [[Camera view and clip-space pipeline]]
- [[Visual angle and calibrated rays]]
- [[Lens optics and depth of field]]

## Used by

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]]

## Future depth

r; a; y;  ; d; i; f; f; e; r; e; n; t; i; a; l; s; ,;  ; p; i; x; e; l;  ; f; o; o; t; p; r; i; n; t; ,;  ; c; a; m; e; r; a;  ; i; m; p; o; r; t; a; n; c; e; ,;  ; s; o; l; i; d; -; a; n; g; l; e;  ; P; D; F; ,;  ; m; e; a; s; u; r; e;  ; c; o; n; v; e; r; s; i; o; n
