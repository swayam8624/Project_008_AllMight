---
tags:
  - concept
  - foundation
---

# Guard round and sticky bits

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Guard is first discarded bit, round the next, sticky ORs all remaining bits. With retained low bit L, nearest-even increments when G AND (R OR sticky OR L). Sticky summarizes any nonzero tail.

## Prerequisites

- [[Boolean algebra]] — The rounding increment condition combines one-bit guard, round, sticky and parity inputs.
- [[Rounding to nearest ties to even]] — Midpoint parity determines discarded-tail decisions and the normal relative-error bound.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Directed rounding additionally depends on sign; renormalization may follow an increment.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C004 - M004 - Parts 0061-0076 - Floating-point representation, spacing, and rounding|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
