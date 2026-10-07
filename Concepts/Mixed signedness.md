---
tags:
  - concept
  - foundation
---

# Mixed signedness

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

A negative signed operand can convert to a large unsigned value; choose semantic types consistently or use `std::cmp_*`, `std::in_range`, or `std::ssize` where appropriate.

int −1 compared with unsigned int 1 converts to a large unsigned value on the usual host, so the comparison is false. std::cmp_less preserves mathematical ordering. A valid index still needs a lower bound: `index>=0 && std::cmp_less(index,size)`.

## Prerequisites

- [[Usual arithmetic conversions]] — After promotions, equal-signedness operands use the higher-ranked type.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain mixed signedness without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** API design, diagnostics, and range-safe indexing.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
