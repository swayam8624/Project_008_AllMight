---
tags:
  - concept
  - foundation
---

# Rounding to nearest ties to even

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Choose the nearest representable value; exactly halfway choose the even retained significand integer. At toy precision three bits, 1.125→1 and 1.375→1.5. std::round uses halfway-away-from-zero instead.

## Prerequisites

- [[ULP and representable spacing]] — Adjacent representable points define rounding candidates, errors and step distances.

## Used by

- [[Unit roundoff]] — uses this mechanism in its explanation.
- [[Guard round and sticky bits]] — uses this mechanism in its explanation.
- [[Directed rounding modes]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Carry from rounding can renormalize; full arithmetic rounding awaits M005.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
