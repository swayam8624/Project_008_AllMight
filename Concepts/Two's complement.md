---
tags:
  - concept
  - foundation
---

# Two's complement

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

An $N$-bit negative $-x$ is residue $2^N-x=\sim x+1$; the signed high-bit weight is $-2^{N-1}$, giving one zero and an asymmetric range.

For raw unsigned value u in N bits, signed decoding is u when u<2ᴺ⁻¹, otherwise u−2ᴺ. Thus `0xAD` means 173−256=−83. Negation uses width-limited complement plus one modulo 2ᴺ. Signed interpretation and unsigned residue share bits but have different language arithmetic contracts.

## Prerequisites

- [[Unsigned modular arithmetic]] — Modulo m retains the remainder class after division by m.
- [[Bitwise NOT]] — Complement must specify width: eight-bit complement of `0x0F` is `0xF0`, while sixteen-bit complement is `0xFFF0`.

## Used by

- [[Fixed-point representation]] — uses this mechanism to define its representation, error or validation contract.

- [[Minimum signed value]] — uses two's complement as part of its representation or reasoning.
- [[Carry, overflow, negative, and zero flags]] — uses two's complement as part of its representation or reasoning.
- [[Subtraction through addition]] — uses two's complement as part of its representation or reasoning.
- [[Arithmetic policies]] — uses two's complement as part of its representation or reasoning.

## Recall and next depth

Explain two's complement without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** signed shifts, instruction semantics, overflow intrinsics, and wider arithmetic.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
