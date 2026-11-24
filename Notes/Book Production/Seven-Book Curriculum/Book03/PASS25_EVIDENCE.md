# Book03 — Pass 25 evidence and acceptance

Chapter 25 — Colors That Tell the Truth. Four original analytic SVG figures (109–112), eight exercises, spectral ambiguity, relative CIE XYZ and xyY, D65 RGB basis construction, sRGB transfer, Display P3 conversion, explicit gamut decisions, toy RAW neutral balance and relative HDR compression.

## Observed validation

GCC 14.2.0 (Debian), C++23 with -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all. Both standalone test and studio compiled and ran without warnings or sanitizer diagnostics.

Observed P25 PASS matrix max=1.22125e-15 transfer max=4.44089e-16. The 10,000 seeded triplets cover matrix and transfer round trips; all 256 eight-bit codes, D65 white, black chromaticity, out-of-gamut P3 red, neutral gains, HDR channel ratios and invalid inputs have separate witnesses. The test tolerance accommodates rounded transfer breakpoints; the random trial does not guarantee sampling their narrow mismatch interval.

An independent NumPy oracle reconstructed each matrix from the primary chromaticities and D65 white using a linear solve and checked all 729 CSV rows. Maximum absolute disagreements: XYZ 3.3306690738754696e-16; P3 1.1102230246251565e-15; transfer 0.0. All checks below 1e-13. This verifies numerical transformations, not perceptual accuracy or physical displays.

Exact locally tested and committed Git blob matches:
- include/book03/color.hpp: cab90e0a92893bee83490508f73c2f5714b54239
- tests/color_tests.cpp: cf19e9fa6ad46389a06dd04c224bd04dc2806a9a
- src/color_studio.cpp: 1e40cd0542610cac6e37567dde21ba5eb38e2233

## Boundaries

Unit-range sRGB transfer functions, fixed D65 RGB spaces, relative luminance and toy neutral balance only. No full CIE spectral table, camera profile, demosaicer, ICC engine, perceptual gamut mapper, HDR display encoding or physical validation. The fixed-matrix inverse is not a general conditioned solver. Figures are analytical illustrations, not measured color swatches or spectral loci. Direct Git access from the test environment failed DNS; connector writes were independently checked by blob SHA. Cumulative build, CI, full source reconciliation, P21 front end and publication rendering remain unverified. No PDF/EPUB produced.
