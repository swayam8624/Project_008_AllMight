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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Reconstruction depth

One-bit centroid quantization does not merely store a sign. A group of source values is partitioned into two reconstruction levels, often centroids $c_0$ and $c_1$, and each element stores one membership bit. Decoding is therefore

$$
\hat x_i = c_{b_i},
\qquad b_i\in\{0,1\}.
$$

If the centroids are fixed to symmetric values such as $\{-a,+a\}$, metadata becomes cheaper but the representation loses the ability to follow asymmetric distributions. If they are fitted per group, metadata cost increases but reconstruction error can fall.

The useful engineering quantity is total cost, not bits-per-weight in isolation:

$$
\text{effective bits/value}
=
1 + \frac{\text{centroid metadata bits}}{\text{group size}}.
$$

Small groups adapt better but pay more metadata and may reduce SIMD efficiency. Large groups amortize metadata but force unrelated values to share two reconstruction levels.

A correct implementation tests deterministic centroid selection, empty-cluster handling, tie policy, bit packing order, group tails, finite decoded values, and error against a declared metric. For inference, the important next question is whether the binary code can participate directly in a dot-product kernel or must be expanded first; that determines whether compression actually saves bandwidth.

