---
tags:
  - concept
  - foundation
---

# Cache lines and memory bandwidth

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C003

## Definition and mechanism

Packing can place more logical values in each transferred cache line, trading arithmetic for lower footprint and traffic.

A cache line is the block moved between cache levels; memory bandwidth is bytes transferred per unit time. With an illustrative 64-byte line, byte-per-bit data stores 64 logical bits, while packed data stores 512. Real line sizes and performance must be measured for the target.

## Prerequisites

- [[Byte and octet]] — An octet is exactly eight bits.
- [[Addresses and virtual memory]] — An address names a storage location in an address space.

## Used by

- [[Field reordering and AoS versus SoA]] — depends on this concept; see its prerequisite explanation.

## M003 development

A four-byte load at line offset 62 crosses two 64-byte lines; a page crossing also requires a second translation. Reordering 24→16-byte records reduces footprint, not necessarily measured traffic or speed. [[Field reordering and AoS versus SoA]] separates hot field arrays from full records; [[TLB]] caches translations rather than contents.

## Recall and next depth

Explain cache lines and memory bandwidth without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** measurement, locality, hierarchy levels, and prefetch behavior.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 development]]
