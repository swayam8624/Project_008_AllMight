---
tags:
  - concept
  - foundation
---

# ABI and object layout

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

An application binary interface (ABI) specifies implementation-level calling and data-layout conventions. Member offsets, complete size/alignment, and calling conventions affect binary compatibility. sizeof/alignof/offsetof inspect a standard-layout record; they do not define a portable file schema. Assume stated target rules before deriving 0/4/8 offsets.

## Prerequisites

- [[Alignment and padding]] — Address divisibility determines legal placement and layout holes.
- [[Fixed-width integer types]] — The representation width fixes byte count and valid shift range.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Separate measured compiler layout from a proposed wire schema.

**Pending:** Calling conventions, inheritance, bit-fields, and cross-module ABI.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]
