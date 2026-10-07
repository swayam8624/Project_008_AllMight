---
tags:
  - concept
  - foundation
---

# Alignment and padding

**Curriculum depth:** Developed · **First seen:** C002 · **Latest development:** C003

## Definition and mechanism

Alignment A restricts valid object starts to addresses divisible by A. alignof reports type alignment, which need not equal sizeof; alignas requests a supported stronger alignment. Padding fills layout holes and rounds array stride. Under 1/4/1 member rules, byte–uint32–byte uses offsets 0/4/8 and size 12. Power-of-two masks test low bits; checked align-up avoids unsigned overflow.

## Prerequisites

- [[Byte and octet]] — Byte units define both storage offsets and significance digits.
- [[Addresses and virtual memory]] — Mappings give numeric locations their process-specific storage identity.

## Used by

- [[ABI and object layout]] — uses this mechanism as stated in its prerequisites.
- [[Internal and tail padding]] — uses this mechanism as stated in its prerequisites.
- [[Packed structures and misaligned access]] — uses this mechanism as stated in its prerequisites.
- [[Dynamic allocation and heap]] — uses this mechanism as stated in its prerequisites.

- [[SIMD and GPU data layouts]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Derive 12 bytes from six payload bytes and safely align 0x1003 to eight.

**Pending:** Over-aligned allocation, GPU layout rules and instruction-specific costs.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C003 - M003 - Parts 0041-0060 - Byte placement, virtual memory, and storage lifetime|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
