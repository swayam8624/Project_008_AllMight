---
tags:
  - concept
  - foundation
---

# Offset-binary coding

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

A bias turns signed levels into unsigned codes. For the source-shaped INT4 convention c=q+8, -3 becomes nibble0101, not the two's-complement nibble1101.

## Prerequisites

- [[Unsigned modular arithmetic]] — supplies the representation or validity rule used above.
- [[Interpretation contract]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Do not assume an INT4 label defines offset versus two's-complement storage.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
