---
tags:
  - concept
  - foundation
---

# Binary32

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

A 32-bit binary interchange format with one sign bit, eight exponent bits and 23 fraction bits. Normal precision is 24 bits; exponent bias is 127. Word 3F800000 has E=127,F=0 and means 1.

## Prerequisites

- [[IEEE-754 floating point]] — The format/arithmetic contract gives these bits their numerical domain.
- [[Bit field]] — Masks, offsets and shifts isolate sign, exponent and fraction positions.

## Used by

- [[Binary64]] — uses this mechanism in its explanation.
- [[Exponent bias]] — uses this mechanism in its explanation.
- [[Significand and hidden bit]] — uses this mechanism in its explanation.
- [[Signed zero]] — uses this mechanism in its explanation.
- [[Floating-point infinities]] — uses this mechanism in its explanation.
- [[NaNs and payloads]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Derive 00800000 and 7F7FFFFF; do not infer this format from the name float alone.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
