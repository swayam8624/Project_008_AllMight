# Pass 37 — Source custody diagnosis, chart accuracy and surface metric

## Actual checkout evidence

P36 CI run 45565355367 failed at the new historical manifest check, before native compilation. Investigation confirmed the original generator uses UTF-8 text with universal newline decoding and Unicode splitlines(keepends=True), then hashes UTF-8 encodings, not raw bytes. P37 matches this convention and separately pins 25 raw Git blob identities (three continuous masters, 21 selected derived chapters and the original manifest) from main revision ba3e31681bb74a71800cd35ace375c2051bcbafa. The original source, manifest and old production tree are not rewritten.

At revision 20ec053adf22b2e2084b58aa86b1a9c26ddb3456, [CI 45570631808](https://github.com/swayam8624/Project_008_AllMight/actions/runs/45570631808) completed successfully, including all 25 exact blob identity checks, 34 CTests and all native workshop/export gates. The corrected historical audit still reports 39 discrepancies (3 master checks, 21 derived-file checks, 15 span checks). This is evidence of inconsistency with the historical manifest, not evidence that Book 03 edits caused it. Exact current bytes are preserved; historical provenance remains unresolved. The CI explicitly uses --report-historical-drift, which reports each historical mismatch while retaining a hard failure for pinned byte changes. Running the script without that flag remains strict. Do not claim historical manifest acceptance merely because CI is green.

## Reader and figure improvements

Chapter 08: recovered the first fundamental form G=J^T J, squared principal stretches, area scale, conformality, area preservation, isometry and singular chart cases. Source: Graphics Vision and XR/02 derived lines 195–234. Independently evaluated four constant-J fixtures with NumPy (area/stretch ratios 2/2, 1/4, 9/1, 1/2.61803398875); all passed at 1e-12. This is a derivation witness, not an additional native solver.

- 129-sfm-error-metrics.svg: replace truncated bars by two explicitly zoomed, unit-labeled slope plots; preserve the measured endpoints. Exact blob df5baf5af08e71eface4ce051c9025d0164f0bfa.
- 132-tsdf-fusion.svg: numeric axes, mu=0.10 m, weights one and three, scaled linear field endpoints and exact zero-crossing markers. Exact blob 3b8e54885308424dc9d470d9319d566acfb29680. Independent extraction of circle coordinates confirms 1.01, 1.03 and 1.025 m.
- 136-studio-fixture.svg: align the 440x330 image border to the 1.375 scale of the 320x240 fixture; blob f5dd2cf2f0831d66c39dc5ae02d9c7f34c5e9903. Source geometry checked; not separately raster-rendered.

First two SVGs were rendered locally with CairoSVG, inspected visually and matched against committed Git blob identities. The TSDF negative tick glyphs were corrected after the first rendering. This is targeted visual review, not all 142 figures or PDF pages. The source gate and native CI success above predates the later Chapter 08 and figure edits. No PDF/EPUB created; original source and publication layout untouched.

## Remaining acceptance

Investigate historical manifest drift and preserve receipts; continue source-to-reader technical review and full figure/rendering audit; then prepare Book 2-compatible composition without replacing any published file. Full original-source reconciliation and publication acceptance are not complete.
