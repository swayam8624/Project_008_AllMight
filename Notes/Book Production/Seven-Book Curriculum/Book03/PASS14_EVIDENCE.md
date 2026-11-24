# Book03 — Pass 14 evidence and acceptance gates

**Chapter:** 14 — From a 3D Point to a Screen Address
**Branch:** curriculum/book03-geometry-images-pass01

## Committed source

The 3,906-word reader derives PVM, homogeneous coordinates, calibrated perspective matrix from the P13 pinhole, orthographic matrix, finite and infinite forward and reversed depth, clip half-spaces, perspective divide, viewport, reciprocal-depth derivative, uniform versus floating storage and depth inversion. Twelve problems and selected answers accompany the native project.

- `include/book03/projection_pipeline.hpp`: Homogeneous4, row-stored column-vector matrix, DepthMap with explicit finite/infinite endpoints, calibrated skewed/off-axis perspective, orthographic matrix, closed clip classification and positive w, NDC and viewport trace. Classifies points only; no primitive clipping.
- `tests/projection_pipeline_tests.cpp`: hand-derived clip tuple (1.25,5/3,3.375,4), screen (420,140), forward depth 0.84375 and reverse 0.15625; orthographic and finite/infinite endpoint tests, invalid input and 4,000 seeded P13 crosschecks.
- `src/projection_studio.cpp`: actual scalar projection trace, REPL, ideal reciprocal-depth SVG and 964-row quantization CSV (241 depths x 2 orders x 2 storage models). Binary32 storage experiment requires IEEE binary32 float. Exports use fixed near 0.1 and infinite far, independently of REPL state.
- Registered book03_projection, book03_projection_tests and CTest book03_projection_and_depth_contracts. CI source specifies independent Python checks of the actual exports.

Six original technical SVGs, 55 through 60, are linked in the chapter. Total source figure count: 60. Figure 60 plots six independently calculated positive-distance sample errors on log-log axes; the separate native SVG plots ideal depth values.

## Independently evaluated scalar witnesses

In this authoring pass, analytic q=1-n/d and q=n/d were evaluated in JavaScript binary64 and quantized with Math.fround (binary32) or nearest 24-bit uniform-grid rounding. Inversion used n/(1-q) or n/q. At d=100000 m, n=0.1 m, infinite far, absolute reconstruction errors were forward float32 1310.4941176470602 m, reverse float32 0.0002524757292121649 m, forward uniform 24-bit 1310.4999999996508 m and reverse uniform 24-bit 1310.5 m. These are exact-to-reported-floating-calculation samples, not GPU measurements or bounds. No claim of GPU speed or raster precision is made.

## Acceptance and limitations

Native compile/CTest, successful CI, actual compiled-program output, final SVG render/print proof, and full original-source reconciliation are unobserved. Floating clip boundaries are not certified predicates and may round across a mathematical boundary. Point classification is not primitive clipping; outside vertices may belong to a surviving clipped triangle. The depth-storage experiment excludes transform, interpolation and actual GPU attachment errors. Inverse depth returns no finite value at the infinite far endpoint. All release artifacts remain untouched.
