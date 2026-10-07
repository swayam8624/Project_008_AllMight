---
tags:
  - concept
  - foundation
---

# De Morgan's laws

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Negation crossing a conjunction or disjunction swaps the operator: $\neg(A\land B)=\neg A\lor\neg B$, and dually for OR.

Negating 'both' means at least one condition fails; negating 'either' means both fail. `!(visible && loaded)` equals `!visible || !loaded`. The identities hold lane by lane for fixed-width vectors. Rewriting comparisons also requires their domain: negating a floating comparison needs care around NaN.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.
- [[Bitwise NOT]] — Complement must specify width: eight-bit complement of `0x0F` is `0xF0`, while sixteen-bit complement is `0xFFF0`.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain de morgan's laws without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
