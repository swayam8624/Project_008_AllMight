---
tags:
  - concept
  - future
---

# Quantization and compressed weights

**Curriculum depth:** Forward · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Low-precision values and packed model weights reuse field-width, mask, shift, and bandwidth trade-offs.

Quantization maps a larger numeric domain to a smaller discrete set, usually with information loss. Packing stores those discrete codes compactly; dequantization reconstructs approximations. Bit packing alone is lossless for valid codes, whereas quantization changes represented precision.

## Prerequisites

- [[Bit packing]] — Packing places several constrained values into a carrier.
- [[Arithmetic policies]] — A policy states the intended response to an unrepresentable answer: wrap modulo a base, report checked failure, widen storage, clamp to an endpoint, or grow arbitrary precision.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain quantization and compressed weights without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** scale/zero point, signed low-bit values, dequantization, and accuracy trade-offs.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
