---
tags:
  - concept
  - foundation
---

# Bit casting and representation

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C006

## Definition and mechanism

A same-size eligible bit_cast transfers representation, not numeric value. On the checked binary32 host, bit_cast<uint32_t>(1.0f)=3F800000, whereas a numeric cast gives 1. Pointer punning is not an equivalent legal operation.

M006 separates representation copying, numeric conversion and endian coding. Eligibility does not validate arbitrary padding/target representations or establish a typed object in file-mapped bytes. Use initialized compatible scalars and explicit external encoding.

## Prerequisites

- [[Fixed-width integer types]] — A same-size unsigned carrier holds the specified interchange representation.
- [[Interpretation contract]] — A bit transfer and a numeric conversion interpret the same carrier differently.
- [[IEEE-754 floating point]] — The format/arithmetic contract gives these bits their numerical domain.

## Used by

- [[Binary format contracts]] — uses this mechanism to define its representation, error or validation contract.
- [[Memory dump interpretation]] — uses this mechanism to define its representation, error or validation contract.

- [[Floating-point canonicalization]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Format checks, object-model validity, byte order, and payload transport are separate constraints.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Bytes become a durable agreement|Narrative development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
