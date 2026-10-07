---
tags:
  - concept
  - foundation
---

# Quantization and compressed weights

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C006

## Definition and mechanism

Low-precision values and packed model weights reuse field-width, mask, shift, and bandwidth trade-offs.

Quantization maps a larger numeric domain to a smaller discrete set, usually with information loss. Packing stores those discrete codes compactly; dequantization reconstructs approximations. Bit packing alone is lossless for valid codes, whereas quantization changes represented precision.

M006 develops affine scales and zero points, local group calibration, offset-coded INT4, one-bit row centroids, metadata overhead and error metrics. Packing preserves discrete codes; quantization deliberately loses information. See the worked five-value group in the continuous story.

## Prerequisites

- [[Bit packing]] — Packing places several constrained values into a carrier.
- [[Arithmetic policies]] — A policy states the intended response to an unrepresentable answer: wrap modulo a base, report checked failure, widen storage, clamp to an endpoint, or grow arbitrary precision.

## Used by

- [[One-bit centroid quantization]] — uses this mechanism to define its representation, error or validation contract.
- [[Quantization error metrics]] — uses this mechanism to define its representation, error or validation contract.

Connect later chunks here when they use this concept.

## Recall and next depth

Explain quantization and compressed weights without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** task-level accuracy and throughput experiments, alternative calibration objectives and target kernels; these were not measured in the teaching merge.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Narrative development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
