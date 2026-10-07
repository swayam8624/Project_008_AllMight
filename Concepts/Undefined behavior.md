---
tags:
  - concept
  - foundation
---

# Undefined behavior

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

Invalid shifts, out-of-bounds access, signed overflow, division by zero, and `INT_MIN / -1` provide no valid C++ execution result; optimizers may assume they never occur.

The C++ language imposes no requirements on an execution that invokes undefined behavior. It is not an error value or a guaranteed trap. Signed overflow, division by zero, invalid shift counts, and out-of-bounds accesses are examples. Check preconditions before performing the operation.

## Prerequisites

- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.

## Used by

- [[Signed integer overflow]] — uses undefined behavior as part of its representation or reasoning.

- [[Packed structures and misaligned access]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain undefined behavior without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** formal categories, provenance, optimizer proofs, and sanitizers.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
