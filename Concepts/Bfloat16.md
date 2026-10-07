---
tags:
  - concept
  - foundation
---

# Bfloat16

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

BF16 has1/8/7 fields, bias127 and8 normal precision bits. It keeps broad exponent range at coarse spacing2^-7 above1. A nearest-even conversion must handle NaNs separately from finite rounding.

## Prerequisites

- [[IEEE-754 floating point]] — supplies the representation or validity rule used above.
- [[Rounding to nearest ties to even]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Target flushing, payload conventions and accumulation instructions require independent checks.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
