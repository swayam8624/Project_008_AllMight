---
tags:
  - concept
  - foundation
---

# Bitwise XOR

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Toggles selected lanes and marks differences; applying the same mask twice restores the input.

XOR returns 1 when inputs differ. `0xAD ^ 0x08 = 0xA5` toggles visible; applying `0x08` again restores `0xAD`. XOR between old and new words marks changed positions. Reversibility alone does not provide secure encryption.

## Prerequisites

- [[Boolean algebra]] — The logical domain is false/true.

## Used by

- [[Checksums CRCs and hashes]] — uses this mechanism to define its representation, error or validation contract.

- [[Half adder]] — uses bitwise xor as part of its representation or reasoning.

## Recall and next depth

Explain bitwise xor without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** parity, checksums, coding theory, and cryptographic constructions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
