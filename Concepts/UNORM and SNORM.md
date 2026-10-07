---
tags:
  - concept
  - foundation
---

# UNORM and SNORM

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

UNORM decodes q/(2^n-1); UNORM8 code128 gives128/255. SNORM commonly clamps q/(2^(n-1)-1) at -1, so -128 and -127 both map to -1 for eight bits.

## Prerequisites

- [[Affine quantization and zero point]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Hardware/API conversion and supported format use must be verified per target; sRGB is separate.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
