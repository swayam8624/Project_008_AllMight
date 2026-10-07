---
tags:
  - concept
  - foundation
---

# Bitwise OR

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Sets selected lanes while preserving lanes paired with 0; it cannot clear an existing 1.

Per lane: only 0|0 is 0; the other three cases are 1. `0xAD | 0x40 = 0xED` sets locked. Repeating the operation leaves the same result: OR is idempotent. It cannot clear an existing 1, so replacing a field requires a clear step.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.

## Used by

- [[Bit packing]] — uses bitwise or as part of its representation or reasoning.
- [[Logical versus bitwise operators]] — uses bitwise or as part of its representation or reasoning.
- [[Carry-lookahead and parallel-prefix adders]] — uses bitwise or as part of its representation or reasoning.

## Recall and next depth

Explain bitwise or without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** flag sets and concurrent updates.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
