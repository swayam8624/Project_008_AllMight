---
tags:
  - concept
  - foundation
---

# Short-circuit evaluation

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

`&&` may skip its right operand after false; `||` may skip it after true, so evaluation safety is part of semantics.

Built-in logical AND skips its right operand after false; logical OR skips it after true. `ptr && ptr->ready()` guards the dereference. A bitwise `&` evaluates both operands. Operator spelling therefore determines both result meaning and whether a dangerous expression is evaluated.

## Prerequisites

- [[Logical versus bitwise operators]] — Built-in `&&`, `||`, and `!` convert operands to truth conditions.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain short-circuit evaluation without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** sequencing, side effects, and optimizer control flow.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
