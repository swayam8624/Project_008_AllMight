---
tags:
  - concept
  - foundation
---

# Bit packing

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Multiple constrained values share a carrier word; extraction and mutation must preserve the declared schema.

Packing places several constrained values into a carrier. Input `[1,0,1,1,0,1,0,1]` packed LSB-first produces `0xAD`. Unpacking needs the logical count because the final byte may contain unused bits. Define whether padding must be zero, ignored, or rejected.

## Prerequisites

- [[Bit field]] — A field is a declared range with offset s and width w, not automatically a C++ language bit-field member.
- [[Bitwise OR]] — Per lane: only 0|0 is 0; the other three cases are 1.
- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.

## Used by

- [[Atomic operations and write contention]] — uses bit packing as part of its representation or reasoning.
- [[Quantization and compressed weights]] — uses bit packing as part of its representation or reasoning.

## Recall and next depth

Explain bit packing without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** bulk kernels, padding policy, random access, atomic mutation, and format evolution.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
