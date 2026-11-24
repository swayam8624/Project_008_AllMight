# Book03 — Pass 17 evidence and acceptance

**Chapter:** 17 — Shapes That Refuse to Leave the Window  
**Branch:** curriculum/book03-geometry-images-pass01

## Committed source

The 3,618-word chapter derives rectangle outcodes, parametric half-space intersection, ordered polygon clipping, homogeneous plane functions, clip-before-divide, raw attribute transport, source barycentric provenance, fan triangulation and numerical consistency. Twelve reader exercises and selected answers accompany the reference.

- `include/book03/primitive_clipping.hpp`: bounded continuous rectangle clipping with both Cohen–Sutherland and Liang–Barsky, six homogeneous clip planes, ordered outside-first edge crossing, active-plane snapping, convex polygon clipping, raw payload and original-corner weights, screen adapter and fan triangulation.
- `tests/primitive_clipping_tests.cpp`: 4,000 seeded segment comparisons, 2,000 single-plane triangle provenance witnesses, hand-calculated rectangle/corner/parallel examples, on-plane endpoint handling, reversed shared edge, fully rejected triangle, depth crossing and projected behind-camera triangle.
- `src/clipping_studio.cpp`: fixed constant-w triangle, actual surviving polygon, fan diagram and CSV. This is a geometric diagnostic, not an interactive editor or rasterizer.
- CMake targets book03_clip and book03_clip_tests, CTest book03_primitive_clipping_contracts. CI source verifies four generated/surviving vertices, two fan triangles, raw UVs, source weights and screen coordinates.
- Five original figures 73–77 bring the cumulative count to 77.

## Analytic witness

Input clip tuples A=(-2,0,0.5,1), B=(0,-0.5,0.5,1), C=(0,0.5,0.5,1) produce ordered polygon I,B,C,J. I=(-1,-0.25,0.5,1) and J=(-1,0.25,0.5,1). UVs are respectively (0.5,0), (1,0), (0,1), (0,0.5). In the default 640x480 viewport the screen coordinates are (0,300), (320,360), (320,120), (0,180). This is a hand-calculated reference, not observed native output.

## Limitations and acceptance

Input clip coordinates are bounded to 1e12. Floating classification and active-plane snapping are not certified exact. Any surviving nonpositive-w or drifted polygon triggers explicit failure rather than silent clamping. Raw normals are not normalized at clipping intersections. Mixed optional attribute presence is rejected. The screen adapter reuses P16's ProcessedVertex type; its original source/local/camera metadata is not meaningful for generated intersections. Degenerate polygons with at least three records may survive and require later screen-area rejection. No guarantee of globally watertight adjacent primitives or arbitrary interpolation qualifiers is asserted.

Native compilation, CTest execution, CI success, compiled SVG/CSV output, rendered publication proof and full original-source reconciliation remain unobserved. No PDF or EPUB generated.
