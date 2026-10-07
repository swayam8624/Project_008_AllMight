---
tags:
  - concept
  - foundation
---

# Bit casting and representation

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

A same-size eligible bit_cast transfers representation, not numeric value. On the checked binary32 host, bit_cast<uint32_t>(1.0f)=3F800000, whereas a numeric cast gives 1. Pointer punning is not an equivalent legal operation.

## Prerequisites

- [[Fixed-width integer types]] — A same-size unsigned carrier holds the specified interchange representation.
- [[Interpretation contract]] — A bit transfer and a numeric conversion interpret the same carrier differently.
- [[IEEE-754 floating point]] — The format/arithmetic contract gives these bits their numerical domain.

## Used by

- [[Floating-point canonicalization]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Format checks, object-model validity, byte order, and payload transport are separate constraints.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
