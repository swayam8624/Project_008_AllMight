---
tags:
  - concept
  - foundation
---

# Bit field

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

A schema-owned range of positions described by width and offset.

A field is a declared range with offset s and width w, not automatically a C++ language bit-field member. Width two at offset four owns positions 4–5 and values 0–3. The schema must fit the carrier and either forbid overlaps or explain their meaning.

## Prerequisites

- [[Bit mask]] — A mask is a selection pattern, not an operation.
- [[Shift operations]] — Logical right shift fills high positions with zero; arithmetic right shift replicates the sign bit.

## Used by

- [[Binary32]] — uses bit field to establish this mechanism's representation or policy.

- [[Bit packing]] — uses bit field as part of its representation or reasoning.
- [[IEEE-754 floating point]] — uses bit field as part of its representation or reasoning.

## Recall and next depth

Explain bit field without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** C++ language bit-fields, instruction formats, GPU formats, and versioned schemas.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
