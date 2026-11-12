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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]

## Why octal still matters

Octal groups binary digits in threes. Each octal digit therefore represents exactly three bits, so a binary field such as `111 101 010` maps directly to `752₈` without repeated division. Hexadecimal dominates modern memory dumps because bytes align naturally to two hex digits, but octal survives in domains whose historical or semantic fields are three-bit groups.

POSIX permission notation is the canonical example: read/write/execute are bit values $4,2,1$, so permission `0754` means owner `111`, group `101`, other `100`. The leading zero in C/C++ source historically denotes an octal integer literal, which creates a real parsing hazard: `010` is eight, not ten. Modern code should prefer explicit intent and avoid visually ambiguous numeric literals.

A reconstruction test should convert binary↔octal by bit grouping, explain why each digit is 0–7, decode a permission triple, and identify the literal-syntax hazard. Octal is not a different storage representation in memory; it is a textual interpretation of the same integer bits.

