---
tags:
  - concept
  - foundation
---

# Logical versus bitwise operators

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Logical operators reduce operands to conditions; bitwise operators transform corresponding representation lanes.

Built-in `&&`, `||`, and `!` convert operands to truth conditions. `&`, `|`, `^`, and `~` operate on integer representation bits. For 10 and 12, `10 & 12` is 8 whereas `10 && 12` is true. Overloaded operators may have additional semantics.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.
- [[Bitwise AND]] — Per lane: 0&0=0, 0&1=0, 1&0=0, 1&1=1.
- [[Bitwise OR]] — Per lane: only 0|0 is 0; the other three cases are 1.

## Used by

- [[Short-circuit evaluation]] — uses logical versus bitwise operators as part of its representation or reasoning.

## Recall and next depth

Explain logical versus bitwise operators without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** overloads, precedence, vector predicates, and branch lowering.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
