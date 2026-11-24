# Pass 18 — Source custody: discrete raster coverage

The original notes, earlier books and release paths are read-only. Targeted source references inspected:
- Computing Foundations 33 — Projection, Clipping, and Raster Sampling, Git blob ef481955be60941713582f1e5600191498f4bd8d: reading-copy lines 2786–2794, 3112–3186 and 3276 on image origin, sample coordinates and subpixel sampling.
- Computing Foundations 34 — Robust Predicates and Camera Sensitivity, blob c75a9727e22b771187f941ddfebe0042bb8ee9ba: lines 1480 and 2103–2121 on perspective interpolation, edge functions and top-left ownership.
- Existing P17 primitive_clipping.hpp blob 7cfae16780bb6d87fc4be723690b1502d492d1c3, P01 pixel_canvas.hpp blob 1b107cfb5e4b1c293ad1fc95c0d6bb431504bb82 and P03 triangle_geometry.hpp blob 38797d6ff341e185818435fbb0dca2341380cadd were inspected before integration.

P18 independently derives the top-left rule for its declared Y-down, positive-area convention, the bounded integer proof, signed candidate bounds, incremental evaluation and unbiased barycentric weights. It implements a coverage-only kernel and a P17/Canvas diagnostic consumer. It does not yet implement visibility, multisampling, texture filtering, programmable shading, GPU-specific coverage, tile traversal or universally watertight clipping. Complete original-source reconciliation is outstanding. See PASS18_EVIDENCE.md for the hash-verified standalone native run. Next: Chapter 19 — Which Surface Gets Seen.
