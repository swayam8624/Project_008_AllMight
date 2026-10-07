---
tags:
  - concept
  - foundation
---

# Quantization error metrics

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

For N>0, RMSE is sqrt(sum squared discrepancies/N), MAE is mean absolute discrepancy, and max error is the largest observed magnitude. Promote float operands before subtraction when measuring in double.

## Prerequisites

- [[Absolute and relative error]] — supplies the representation or validity rule used above.
- [[Quantization and compressed weights]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Representation error alone does not establish downstream task correctness.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
