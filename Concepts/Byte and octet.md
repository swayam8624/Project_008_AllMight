---
tags:
  - concept
  - foundation
---

# Byte and octet

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

An octet is exactly eight bits; a byte is the addressable unit and is eight bits on the target systems assumed here.

An octet is exactly eight bits. In C++, a byte is the storage unit of `char`, `sizeof(char)==1`, and `CHAR_BIT` tells its bit count. This curriculum's worked layouts assume eight-bit bytes. Addressing usually selects a byte, so individual bits are selected with masks.

## Prerequisites

- [[Bit]] — A bit has two possible logical states.

## Used by

- [[Machine word]] — uses byte and octet as part of its representation or reasoning.
- [[Bit mask]] — uses byte and octet as part of its representation or reasoning.
- [[Endianness]] — uses byte and octet as part of its representation or reasoning.
- [[Cache lines and memory bandwidth]] — uses byte and octet as part of its representation or reasoning.
- [[Fixed-width integer types]] — uses byte and octet as part of its representation or reasoning.
- [[Alignment and padding]] — uses byte and octet as part of its representation or reasoning.
- [[Addresses and virtual memory]] — uses byte and octet as part of its representation or reasoning.

## Recall and next depth

Explain byte and octet without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** `CHAR_BIT`, object representation, addressing, and alignment.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
