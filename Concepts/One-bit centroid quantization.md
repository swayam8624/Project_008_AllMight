---
tags:
  - concept
  - foundation
---

# One-bit centroid quantization

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

One sign label selects one of two row means. Row [-1,-3,2,4] reconstructs [-2,-2,3,3], with RMSE one. Two float32 centroids cost 64 additional bits per row.

## Prerequisites

- [[Quantization and compressed weights]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Empty-side, zero and non-finite policies must be explicit in a production codec.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
