---
tags:
  - concept
  - future
---

# Guest and host integer domains

**Curriculum depth:** Forward · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

An emulator may compute an $N$-bit guest operation in a wider host type so carry and other discarded architectural information remain observable.

Guest means the machine being emulated; host means the implementation platform. An eight-bit guest addition can use a host temporary holding 0–511, extract carry, then narrow to 0–255. Host signed overflow must never be used to imitate guest wrap.

## Prerequisites

- [[Fixed-width integer types]] — When supplied, std::uint32_t has exactly 32 bits; uint_least32_t has at least 32; uint_fast32_t is chosen for speed with at least 32.
- [[Carry, overflow, negative, and zero flags]] — For eight-bit addition, C tests exact sum>255; V tests signed sum outside [−128,127]; N is result bit 7; Z tests low result==0.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain guest and host integer domains without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** exact ISA behavior, host-language conversions, and cross-platform emulation tests.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
