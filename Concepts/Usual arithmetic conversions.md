---
tags:
  - concept
  - foundation
---

# Usual arithmetic conversions

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

After promotions, binary operators choose a common type according to rank, signedness, and representability; the operands may change value before comparison or arithmetic.

After promotions, equal-signedness operands use the higher-ranked type. For signed S and unsigned U: if U rank≥S rank, convert S to U; otherwise use S if it represents all U values; otherwise use S's unsigned counterpart. Rank and width are related but distinct.

## Prerequisites

- [[Integral promotion]] — Narrow integer types become int if int represents every original value; otherwise they become unsigned int.

## Used by

- [[Mixed signedness]] — uses usual arithmetic conversions as part of its representation or reasoning.

## Recall and next depth

Explain usual arithmetic conversions without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** the full standard decision tree and floating/integer combinations.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
