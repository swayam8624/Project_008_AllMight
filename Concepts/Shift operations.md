---
tags:
  - concept
  - foundation
---

# Shift operations

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

Relocate bit significance; unsigned shifts can correspond to multiply or floor-divide by powers of two within their validity range.

Logical right shift fills high positions with zero; arithmetic right shift replicates the sign bit. In C++23, signed right shift rounds toward negative infinity; signed integer division rounds toward zero. Thus −5>>1 gives −3 while −5/2 gives −2. A built-in shift count must be nonnegative and smaller than the promoted left operand's width.

## Prerequisites

- [[Bit significance]] — LSB means least significant bit, normally position 0 with weight 1.
- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.

## Used by

- [[Bit field]] — uses shift operations as part of its representation or reasoning.
- [[Shift-and-add multiplication]] — uses shift operations as part of its representation or reasoning.
- [[Binary long division]] — uses shift operations as part of its representation or reasoning.

- [[Little endian]] — depends on this concept; see its prerequisite explanation.
- [[Pages and page offsets]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain shift operations without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** signed-shift rules, compiler lowering, rotate instructions, and overshift behavior.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
