---
tags:
  - concept
  - foundation
---

# Octal

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Base 8 maps one digit to three bits; it survives in permissions and legacy literal syntax.

Octal digits run from 0 through 7. Group binary from the right in threes: `010 101 101₂ = 255₈`. In C++, `0255` is octal 173; decimal `255` is a different value. Permission triples such as rwx map naturally onto three bits.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain octal without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Unix permission semantics.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
