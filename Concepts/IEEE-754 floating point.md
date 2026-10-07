---
tags:
  - concept
  - foundation
---

# IEEE-754 floating point

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C004

## Definition and mechanism

IEEE-754 specifies floating formats and behavior including representation classes, rounding directions and exception conditions. A finite bit pattern is interpreted through sign, exponent and significand rules, not as a two's-complement integer.

Binary32 has 1/8/23 stored fields and 24-bit normal precision; binary64 has 1/11/52 fields and 53-bit normal precision. Word 412A0000 encodes 10.625 in binary32. Exponent-zero and all-one fields distinguish zero/subnormal and infinity/NaN. Normal spacing expands with exponent; nearest-even and directed modes choose representable results under explicit rules. These format names do not prove every C++ float/double has those representations.

## Prerequisites

- [[Bit field]] — A field is a declared range with offset s and width w, not automatically a C++ language bit-field member.
- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.

## Used by

- [[Binary32]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Floating-point exception flags]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Bit casting and representation]] — uses ieee-754 floating point to establish this mechanism's representation or policy.
- [[Robust norm and intermediate range]] — uses ieee-754 floating point to establish this mechanism's representation or policy.

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct 10.625's fields, distinguish epsilon from minimum positive values, and resolve an exact midpoint without calling it decimal rounding.

**Pending:** M005, Parts 77–88: arithmetic alignment, cancellation, error and determinism; later low-precision formats.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|C004 development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
