---
tags: [concept, phase-11]
---

# Camera gauge freedoms and observability

**Curriculum depth:** Developed · **First seen:** Phase 11

## Definition and mechanism

A gauge freedom is a coordinated change to unknown scene and camera parameters that leaves every measured image unchanged. In calibrated monocular structure from motion, translating and rotating the entire reconstruction changes only the chosen world frame, while a common scale change of points and camera translations preserves projections; absolute metric scale therefore needs an external reference. Observability asks which local parameter changes produce distinguishable measurements. Linearizing reprojection residuals gives a Jacobian; its weighted information tensor or matrix $J^{\mathsf T}WJ$ reveals weak parameter combinations through small eigenvalues after gauge directions are fixed. Example: adding more points to one planar, nearly fronto-parallel view may reduce pixel noise while focal length and distance remain strongly coupled. These conclusions assume a stated camera/scene model, correctly associated observations, and local linearization; the information matrix diagnoses local constraints, not global uniqueness. Report uncertainty only after fixing the gauge and specifying the observation noise model.

**Example:** A monocular map can be uniformly enlarged with all camera translations enlarged by the same factor and render the same images; a known baseline or measured object length fixes that scale gauge.

**Assumptions:** The gauge statement assumes calibrated monocular perspective cameras and no metric prior.; The information-matrix interpretation is local and depends on residual weighting and parameterization.

## Prerequisites

- [[Camera calibration and bundle adjustment]]
- [[Coordinate frames and change of basis]]
- [[Homogeneous coordinates and perspective division]]

## Used by

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]]

## Future depth

m; o; n; o; c; u; l; a; r;  ; s; c; a; l; e;  ; g; a; u; g; e; ,;  ; J; a; c; o; b; i; a; n;  ; n; u; l; l; s; p; a; c; e; ,;  ; H; e; s; s; i; a; n;  ; s; p; e; c; t; r; u; m; ,;  ; F; i; s; h; e; r;  ; i; n; f; o; r; m; a; t; i; o; n; ,;  ; c; a; l; i; b; r; a; t; i; o; n;  ; o; b; s; e; r; v; a; b; i; l; i; t; y
