---
tags:
  - concept
  - foundation
---

# Floating-point encoding and decoding

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Encoding chooses fields for a value; decoding classifies fields and reconstructs their meaning. 10.625 normalizes to 1.010101₂×2³, giving 412A0000. C1540000 decodes to −13.25.

## Prerequisites

- [[Exponent bias]] — Stored exponents must be translated to effective powers before reconstructing value or spacing.
- [[Significand and hidden bit]] — Fraction weights and effective precision determine value and the smallest retained position.
- [[Hexadecimal]] — Four-bit groups make complete representation words independently inspectable.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Inexact conversion requires rounding; the exact worked examples do not explain every decimal conversion.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
