---
tags:
  - concept
  - foundation
---

# Bit mask

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

A pattern naming selected bit positions; its effect depends on the operation used with it.

A mask is a selection pattern, not an operation. `0x30 = 00110000₂` names positions 4 and 5. AND reads selected positions, OR sets them, XOR toggles them, and AND with the width-limited complement clears them. Full-width masks need a special implementation case.

## Prerequisites

- [[Bit significance]] — LSB means least significant bit, normally position 0 with weight 1.
- [[Byte and octet]] — An octet is exactly eight bits.

## Used by

- [[Bit field]] — uses bit mask as part of its representation or reasoning.

- [[Pages and page offsets]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain bit mask without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
