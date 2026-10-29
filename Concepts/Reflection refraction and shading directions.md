---
tags:
  - concept
  - phase-6
---

# Reflection refraction and shading directions

**Curriculum depth:** Developed · **First seen:** Phase6 · **Latest development:** University courses

## Definition and mechanism

Plane reflection reverses a direction's normal component. Refraction scales its tangent component by the incident/transmitted index ratio and solves for a unit normal component. A negative computed discriminant indicates total internal reflection under the stated model. Half-vectors require normalized outward light/view directions.

Incident $(0.6,0,-0.8)$ reflects to $(0.6,0,0.8)$ across normal $(0,0,1)$. Antiparallel light/view directions have no unique half-vector.

## Prerequisites

- [[Projection rejection and scalar components]] — Removing an unwanted component creates residuals, tangent axes and mirror decompositions.
- [[Unit directions and safe normalization]] — Validated finite unit directions establish the preconditions for simplified geometry formulas.

## Used by

Later curriculum sources can deepen this same node; no future coverage is marked complete.
- [[Lens optics and depth of field]]

## Recall and next depth

Declare arrow and medium conventions before deriving formulas; distinguish direction geometry from optical energy. Full production proofs, target-specific integration and learner mastery require further evidence.

## Source and navigation

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Light uses the same decomposition in a new role|Teaching narrative]] · [[Supplementary/Foundations#Phase 6 - Parts 501-600|Coverage and source gap]] · [[Supplementary/Sources and Code Anchors#Phase 6 - Source analysis and verification|Evidence and limits]] · [[Dependency Map]]

## University course development

Extend the existing direction note by separating geometric surface orientation from optical ray behavior. The normal vector is perpendicular to a local tangent plane; it determines the cosine term in diffuse shading and the directions of ideal reflection and refraction. For unit incident direction $\omega_i$ and unit normal $n$, specular reflection mirrors the direction about the normal, while refraction changes direction according to the ratio of refractive indices and may undergo total internal reflection. Direction conventions matter: some texts define incoming vectors toward the surface, others point away from it, so signs must be checked before applying formulas. A shading normal may differ from the true geometric normal, as in normal mapping, but it cannot change silhouette or visibility. Distinguish shading perturbations from geometry displacement, and use consistent normals when evaluating BRDFs, ray offsets, and medium transitions.

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Lens optics, focus, and image formation|Lens optics, focus, and image formation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Reflectance, BRDFs, and shading|Reflectance, BRDFs, and shading]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Shape from shading and reflectance|Shape from shading and reflectance]]
