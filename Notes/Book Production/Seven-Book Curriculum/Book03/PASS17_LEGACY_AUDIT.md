# Pass 17 — Original-source custody: primitive clipping

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

Original continuous notes, derived reading copies, published Books 01–02 and final distribution paths remain unchanged.

## Original material inspected

- `Notes/Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling.md`, Git blob `ef481955be60941713582f1e5600191498f4bd8d`. Reading-copy lines 5 and 16 identify the projection/clipping/division chain; line 610 warns against treating finite divided points as visible; line 726 identifies homogeneous clip boundaries. The P14 audit records the original master metadata.
- `Notes/Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity.md`, blob `c75a9727e22b771187f941ddfebe0042bb8ee9ba`. Located lines 1427 and 1459 identify an inward-facing oblique plane and its retained half-space. P05 and P14 audits preserve the related exact-predicate and frustum material.
- Existing P16 `vertex_pipeline.hpp` blob `0b161da528892c92b80a8d0d0bf075804a616bb0`, P14 `projection_pipeline.hpp` blob `17a1f9effa42c5f15d95f9b06813a5a7a591eec9` and P06 `geometry3d.hpp` blob `b386703e351ccb6bf8730754949f2683903628e7` were inspected for the actual raw vertex payload, clip-space classification, viewport and vector interfaces.

## New coverage and remaining obligations

The new reader independently derives the classic Cohen–Sutherland, Liang–Barsky and Sutherland–Hodgman algorithms, their convex half-space foundations, homogeneous edge intersections, raw smooth varying interpolation, source-corner provenance, fan assembly and numerical boundaries. It connects directly to P16 and explains why P18 needs an independent discrete coverage policy.

The original source passages are not a completed source-unit reconciliation. General oblique frusta, exact/adaptive polygon predicates, production watertightness, arbitrary interpolation qualifiers, raster coverage, hardware adapters and full rendering are outstanding. No original text was removed.

Next: Chapter 18 — Who Owns the Edge.
