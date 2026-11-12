---
tags:
  - concept
  - xr
  - independent-series
---

# Temporal reprojection and history validation

**Curriculum depth:** Introduced · **First seen:** XR01 · **Latest development:** XR01

## Definition and mechanism

Reprojection aligns history with a current target; disocclusion exposes content with no matching old observation. Validated blending trades noise against response/ghosting. Gaze-driven quality changes also change history reliability; TAA is not exposure integration.

## Prerequisites

- [[Spatiotemporal sampling and retinal motion]] — Temporal reuse must respect changing scene and retinal observations.
- [[Antialiasing and sample allocation]] — History depends on the actual current/prior sampling strategy.

## Used by

- [[Foveated rendering and uncertainty]]

- [[Reservoir resampling and ReSTIR]]
## Recall and next depth

Derive ideal history variance and identify every violated assumption. Pending: motion/depth/jitter conventions and robust implementation. Written coverage is not a claim of learner mastery or accepted numbered-curriculum progress.

## Source and navigation

[[Continuous Notes/XR - From Light to Perception - Seeing and Representing Reality#Temporal reconstruction is an inference about correspondence|Expanded teaching explanation]] · [[Supplementary/Foundations#XR01 - Independent seeing and representation source|Independent coverage]] · [[Dependency Map]]

## Phase 11 camera development

Reprojection predicts where a prior sample lands under camera and object motion. Depth agreement tests whether the same visible surface likely occupies that location, but cannot prove stored color remains valid after lighting, exposure, material, or sampling changes. Combine projected position, depth tolerance, and disocclusion checks; reduce history weight when confidence is low. Monocular reconstruction has arbitrary global scale, so metric depth thresholds require an external scale or consistent units. Reprojection success is geometric evidence, not a photometric guarantee. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].

## Neighborhood clipping and deterministic jitter witness

`tools/labs/realtime_effects_reference_lab.cpp` now extends history validity into two mechanisms that directly affect temporal stability.

The first is deterministic subpixel jitter generated from Halton bases 2 and 3. The sample lies inside one pixel but changes frame to frame, allowing temporal accumulation to observe different subpixel locations. A jitter sequence must be owned by the camera/projection history contract: if the current and previous transforms disagree about whether jitter is included, the motion vector and reprojection become inconsistent.

The second is neighborhood clipping. A reprojected history value can be geometrically valid yet photometrically stale after disocclusion or fast shading change. The smallest CPU oracle clamps the history sample into the current 3×3 neighborhood min/max interval before blending. Production TAA often uses variance-guided bounds, color-space transforms, reactive masks and velocity-dependent weighting, but the invariant remains: history should not inject values unsupported by the current local evidence.

The laboratory checks known first Halton points, uniqueness over an initial sequence, bright/dark history rejection and preservation of an in-range history value. It remains a semantic oracle rather than a complete production TAA implementation.

