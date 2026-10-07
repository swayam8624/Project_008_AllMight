---
tags:
  - concept
  - foundation
---

# Groupwise quantization and metadata

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

Local groups share a scale, improving adaptation at a metadata cost. Symmetric INT4 calibration maxAbs/7 reconstructs codes around [-7,7]; float scales add about 32/G bits per value for full groups.

## Prerequisites

- [[Affine quantization and zero point]] — supplies the representation or validity rule used above.
- [[Bit packing]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** No task-accuracy or throughput benchmark was run.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
