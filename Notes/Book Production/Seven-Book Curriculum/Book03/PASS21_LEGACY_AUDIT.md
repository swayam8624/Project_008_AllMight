# Pass 21 — Source custody: full rasterizer integration

Authoritative CHAPTER_PLAN.md blob 77937aecee267f9772c742da0a979c030385af4c identifies P21 model loading, CPU draw pipeline, clipping, rasterization, output, performance and proof. This additive pass integrates the actual P10 asset import, P11 affine model, P12 rigid camera pose, P14 projection, P16 vertex stage (0b161da528892c92b80a8d0d0bf075804a616bb0), P17 clipping (7cfae16780bb6d87fc4be723690b1502d492d1c3), and P18–20 native-validated coverage, depth and encoding. Existing API declarations were read directly before authoring.

Computing Foundations 33 — Projection, Clipping, and Raster Sampling (blob ef481955be60941713582f1e5600191498f4bd8d) remains preserved with prior targeted source maps P14–20. No complete original-source reconciliation is asserted. This pass contains original integration narrative and a measured backend witness, not a claim of complete frontend validation. See PASS21_EVIDENCE.md for exact observed and unobserved gates. Original books and publication directories remain unchanged.

Next: Chapter 22 — A Lens Bends the Story.
