# Book03 — Pass 16 evidence and acceptance gates

**Chapter:** 16 — The Long Journey of One Vertex  
**Branch:** curriculum/book03-geometry-images-pass01

## Delivered sources

The 4,166-word chapter derives the indexed vertex-stage chain and perspective-correct interpolation from homogeneous coordinates. Twelve reader exercises and selected answers link geometry, source identity, varying attributes and depth.

- `include/book03/vertex_pipeline.hpp`: fixed draw-context VertexStage, referenced rendering-ID reuse, optional source UV/normal handling, lazy inverse-transpose normal matrix, world/camera/raw clip and optional screen records, screen barycentrics, reciprocal-w surface weights and screen-linear depth.
- `tests/vertex_pipeline_tests.cpp`: independent centroid and edge witnesses, texture seam identity, instance placement, rotated view and nonuniform scale, singular normal rejection, invalid index, outside clip payload preservation, mixed presence rejection, 3,000 seeded original-surface/project/interpolate crosschecks.
- `src/vertex_studio.cpp`: synthetic OBJ stage trace, 101-row edge CSV and actual-source-derived triangle plus interpolation-curve SVG. The program is noninteractive and uses a fixed reference fixture.
- CMake targets book03_vertex, book03_vertex_tests and seventeenth CTest book03_indexed_vertex_and_varying_contracts. CI source specifies analytic checks of centroid UV (2/7,1/7), depth 85/114 and all 101 samples.

Six original figures 67–72 cover rendering identity, attribute routes, retained outside vertices, screen/surface weights, interpolation curves and depth contracts. Total source figure count: 72. The analytic example uses endpoint w values 4 and 8 along one edge, with V=t/(2-t) rather than V=t. These are mathematical witnesses, not recorded native output.

## Earlier CI defect corrected

P05 precision CSV previously assigned the default current triangle the same case label as a fixed fixture, conflicting with the CI dictionary assertion of six distinct names. The CSV now labels its current row Current triangle while keeping all five fixed fixture keys and exact numerical data. This is a source-level correction, not an observed CI pass.

## Limitations

A processed outside vertex remains available for P17 clipping; P16 does not clip triangles. Screen barycentrics use a floating determinant rejection heuristic, not a certified edge coverage test. Only already-clipped nonnegative weights are accepted by interpolation. Reciprocal-w rescaling avoids some intermediate overflow, but extreme finite values can underflow or overflow; normal interpolation can reject cancellation. The lab implements smooth position/UV/normal and screen-linear NDC depth, not arbitrary varying qualifiers or GPU cache behavior.

**Acceptance:** native compilation, CTest execution, actual CI success, compiled-program exports, rendered figure/print proof and complete original-source reconciliation remain unobserved. No PDF or EPUB generated.
