---
tags:
  - concept
  - foundation
---

# Cancellation and loss of significance

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

Nearly equal magnitudes subtract to a small difference, amplifying prior operand errors by roughly (|a|+|b|)/|a−b|. Stored subtraction can itself be exact. The source binary pair 1753/1024 and 1751/1024 differs by exactly 2^-9.

## Prerequisites

- [[Absolute and relative error]] — Discrepancy, magnitude and reference units define sensitivity and threshold significance.
- [[Significand and hidden bit]] — Significant digit positions determine alignment and what leading digits cancel.
- [[Conditioning and singularity]] — Input sensitivity is distinct from errors an implementation introduces.

## Used by

- [[Sterbenz lemma]] — uses this mechanism to establish its numerical contract.
- [[Stable reformulation]] — uses this mechanism to establish its numerical contract.
- [[Dot and cross-product rounding]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Separate input quantization, product rounding, final subtraction, and underlying sensitivity.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
