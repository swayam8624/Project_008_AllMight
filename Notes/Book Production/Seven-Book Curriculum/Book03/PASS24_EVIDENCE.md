# Book03 — Pass 24 evidence and acceptance

Chapter 24 — Light Becomes a Measurement. Three original analytical SVG teaching figures (106–108), eight exercises, SI photon-count conversion, independent Poisson signal/dark and Gaussian read noise, charge saturation and gain/ADC simulation.

## Observed native validation

GCC 14.2.0 Debian, C++23, -Wall -Wextra -Wpedantic -O2 -fsanitize=undefined -fno-sanitize-recover=all. Standalone tests and studio compiled and ran without compiler warnings or sanitizer diagnostics after correcting the hand-computed photon fixture from 27688.6411 to 27687.6411 (independently recomputed using the SI constants). The final test returned P24 PASS mean=102.033 variance=110.814 from 100,000 seeded samples, against analytical mean 102 and variance 111. Tests also cover zero signal, zero noise, ADC boundaries, full-well saturation and invalid bit count. The studio simulated 170,000 exposures over 17 incident-count levels, at fixed 0.01 s dark integration time. Independent Python verified all 17 rows against the analytical signal, variance, SNR, range checks and regime-specific moment checks (six standard errors for unsaturated mean; 10% variance tolerance, and saturated read-noise checks). The highest three levels saturated full well in every observed draw, while none reached maximum ADC code in this configuration.

Exact local and committed Git blob matches:
- include/book03/sensor.hpp: 384285cb25c786d868c1f81827e11cea0489ffd7
- tests/sensor_tests.cpp: a273c84920dff44e8cc7956130863e827eff381a
- src/sensor_studio.cpp: ab019df473fea91eec41897ee9959ac6b551a7e4

## Boundaries

The emulator is monochromatic and independent-pixel; no real camera data, spectral QE curves, CFA, fixed-pattern noise, blooming, or correlated readout. The unclipped SNR is not a saturation-region quality measure. The ADC-top fraction measures maximum output code, not strictly analog clipping. Standard library random distributions need not yield identical draws across platforms. Cumulative CMake/CTest and CI, P21 frontend, original-source reconciliation, visual publication audit and PDF/EPUB remain unverified. Direct Git DNS was unavailable locally; source writes and exact blob checks used the repository connection. No release artifacts generated.
