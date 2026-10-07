---
tags:
  - concept
  - foundation
---

# Affine quantization and zero point

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

A positive step s, integer zero point z and bounded code q reconstruct x̂=s(q-z). Nearest encode uses clamp(round(x/s)+z). Half-step error assumes no clipping and ideal parameters.

## Prerequisites

- [[Fixed-point representation]] — supplies the representation or validity rule used above.
- [[Absolute and relative error]] — supplies the representation or validity rule used above.

## Used by

- [[Groupwise quantization and metadata]] — uses this mechanism to define its representation, error or validation contract.
- [[UNORM and SNORM]] — uses this mechanism to define its representation, error or validation contract.

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Calibration objectives and actual model inference quality await experiments.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
