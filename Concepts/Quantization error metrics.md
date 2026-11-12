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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## A metric must match what the consumer loses

Quantization error is the difference $e=x-\hat x$ between source and reconstruction, but one scalar summary cannot answer every engineering question. Mean squared error emphasizes large deviations; mean absolute error is less sensitive to outliers; maximum absolute error exposes worst-case bounds; relative error becomes useful when scale varies but is unstable near zero.

For tensors, per-tensor MSE can hide one badly damaged channel or group. Diagnostics should often include per-group distributions, saturation counts, scale histograms, and downstream task error. A quantizer that minimizes weight MSE is not guaranteed to minimize model-output error because layers amplify directions differently.

Signal metrics also depend on representation. Comparing gamma-encoded RGB with ordinary squared error is not the same as comparing linear-light energy. Normals may need angular error rather than component MSE.

A useful quantization benchmark therefore reports storage cost together with several reconstruction metrics and one consumer-level metric. The metric is part of the experiment hypothesis, not a decorative number added after compression.

