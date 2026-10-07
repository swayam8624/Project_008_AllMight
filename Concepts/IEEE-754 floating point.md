---
tags:
  - concept
  - foundation
---

# IEEE-754 floating point

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C005

## Definition and mechanism

IEEE-754 specifies floating formats and behavior including representation classes, rounding directions and exception conditions. A finite bit pattern is interpreted through sign, exponent and significand rules, not as a two's-complement integer.

Binary32 has 1/8/23 stored fields and 24-bit normal precision; binary64 has 1/11/52 fields and 53-bit normal precision. Word 412A0000 encodes 10.625 in binary32. Exponent-zero and all-one fields distinguish zero/subnormal and infinity/NaN. Normal spacing expands with exponent; nearest-even and directed modes choose representable results under explicit rules. These format names do not prove every C++ float/double has those representations.

M005 develops alignment, cancellation, FMA and accumulated error. Numeric equality is not representation identity, and moving rounding points can change the computed result.

## Prerequisites

- [[Bit field]] — A field is a declared range with offset s and width w, not automatically a C++ language bit-field member.
- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.

## Used by

- [[Binary16]] — uses this mechanism to define its representation, error or validation contract.
- [[Bfloat16]] — uses this mechanism to define its representation, error or validation contract.

- [[Fused multiply-add]] — Fused operation semantics and destination rounding are part of the arithmetic contract.

- [[Binary32]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Floating-point exception flags]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Bit casting and representation]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Robust norm and intermediate range]] — uses ieee-754 floating point to establish this mechanism's representation or policy.

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct 10.625's fields, distinguish epsilon from minimum positive values, and resolve an exact midpoint without calling it decimal rounding.

**Pending:** Extend this mechanism to later algorithms and target-specific behavior. See the M005 chapter for its current examples and validity assumptions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|C004 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
