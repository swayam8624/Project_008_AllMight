# Pass 19 — Source custody: opaque visibility

Original continuous notes, reading copies, Books 01–02 and final release paths remain unchanged.

- Computing Foundations 33 — Projection, Clipping, and Raster Sampling, blob ef481955be60941713582f1e5600191498f4bd8d: located lines 1596–1857, 1997–2031, 2582–2584 and 2748 on depth mapping, quantization, Z-fighting, reversed depth, matching clear/comparison and reconstruction conventions.
- Computing Foundations 34 — Robust Predicates and Camera Sensitivity, blob c75a9727e22b771187f941ddfebe0042bb8ee9ba: line 230 distinguishes occlusion culling.
- P18 raster_coverage.hpp blob 267716ff7e9c24c1d8d9f5a290b744e14e74b5a6 and P14 projection_pipeline.hpp blob 17a1f9effa42c5f15d95f9b06813a5a7a591eec9 inspected for coverage callback and depth convention.

P19 derives opaque per-sample min/max visibility, screen-linear depth, quantization-before-comparison, explicit occupancy and stable ID ties. It explains painter ordering, depth test work, culling and transparency limitations. The standalone lab uses already-projected screen geometry; no full projection/clipping integration, transparency compositor, hardware depth attachment, early-Z implementation or GPU performance measurement is claimed. This is targeted source custody, not full source-unit reconciliation.

Next: Chapter 20 — A Pixel Is Not a Tiny Square of Reality.
