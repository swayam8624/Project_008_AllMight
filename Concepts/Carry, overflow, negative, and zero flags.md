---
tags:
  - concept
  - foundation
---

# Carry, overflow, negative, and zero flags

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

`C` reports unsigned carry, `V` signed-range failure, `N` the result's sign bit, and `Z` an all-zero architectural result.

For eight-bit addition, C tests exact sum>255; V tests signed sum outside [−128,127]; N is result bit 7; Z tests low result==0. `7F+01` gives 0,1,1,0; `FF+01` gives 1,0,0,1 in C,V,N,Z order. These are binary arithmetic flags, not decimal-mode behavior.

## Prerequisites

- [[Two's complement]] — For raw unsigned value u in N bits, signed decoding is u when u<2ᴺ⁻¹, otherwise u−2ᴺ.
- [[Full adder]] — A full adder adds A, B, and carry-in.

## Used by

- [[Guest and host integer domains]] — uses carry, overflow, negative, and zero flags as part of its representation or reasoning.

## Recall and next depth

Explain carry, overflow, negative, and zero flags without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
