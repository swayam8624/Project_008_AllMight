---
tags:
  - concept
  - foundation
---

# Interpretation contract

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C003

## Definition and mechanism

The width, layout, type, and conventions that turn an otherwise ambiguous bit pattern into data.

For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83. A contract must state width, signedness, field positions, byte order, allowed values, and any padding policy. Reinterpretation changes meaning; conversion may change the stored bits.

## Prerequisites

- [[Bit]] — A bit has two possible logical states.
- [[Bit significance]] — LSB means least significant bit, normally position 0 with weight 1.

## Used by

- [[Fixed-point representation]] — uses this mechanism to define its representation, error or validation contract.
- [[Offset-binary coding]] — uses this mechanism to define its representation, error or validation contract.
- [[Integrity and authenticity]] — uses this mechanism to define its representation, error or validation contract.

- [[Bit casting and representation]] — uses interpretation contract to establish this mechanism's representation or policy.

- [[Integral promotion]] — uses interpretation contract as part of its representation or reasoning.
- [[Bit packing]] — uses interpretation contract as part of its representation or reasoning.
- [[Undefined behavior]] — uses interpretation contract as part of its representation or reasoning.
- [[Serialization]] — uses interpretation contract as part of its representation or reasoning.
- [[Compiler IR and machine instructions]] — uses interpretation contract as part of its representation or reasoning.
- [[IEEE-754 floating point]] — uses interpretation contract as part of its representation or reasoning.
- [[Exhaustive finite-state testing]] — uses interpretation contract as part of its representation or reasoning.

- [[Storage duration and object lifetime]] — depends on this concept; see its prerequisite explanation.

## M003 development

M003 adds byte order, legal alignment, object-relative offsets, mapping context, permissions, and [[Storage duration and object lifetime]]. Portable [[Serialization]] fixes external representation; an aligned mapped address alone does not establish a live object.

## Recall and next depth

Explain interpretation contract without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** ABI, file formats, instruction encodings, pixels, and protocol schemas.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 development]]
