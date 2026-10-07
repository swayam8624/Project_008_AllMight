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

- [[Binary16]] — uses this mechanism to define its representation, error or validation contract.
- [[Bfloat16]] — uses this mechanism to define its representation, error or validation contract.

- [[Fused multiply-add]] — Midpoint decisions determine separate/fused results and narrowed persistent state.
- [[Mixed-precision state updates]] — Midpoint decisions determine separate/fused results and narrowed persistent state.

- [[Unit roundoff]] — uses this mechanism in its explanation.
- [[Guard round and sticky bits]] — uses this mechanism in its explanation.
- [[Directed rounding modes]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Carry from rounding can renormalize; full arithmetic rounding awaits M005.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
