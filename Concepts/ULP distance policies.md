---
tags:
  - concept
  - foundation
---

# ULP distance policies

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Distance counts representable steps under a declared ordering/equality policy. The finite-only laboratory collapses both zeros to one key and rejects NaNs/infinities. Across negative minimum subnormal to positive minimum subnormal distance is two.

## Prerequisites

- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.
- [[Signed zero]] — Numerical equality merges two encodings and must be reflected in the chosen ordering or byte policy.
- [[NaNs and payloads]] — Unordered comparisons and payload distinctions require explicit metric, comparison and serialization policy.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** A raw complement key with two zero slots overcounts cross-zero intervals unless corrected.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
