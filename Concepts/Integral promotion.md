---
tags:
  - concept
  - foundation
---

# Integral promotion

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

Narrow integral operands promote to `int` when it represents their entire range, otherwise to `unsigned int`; the operator acts at that promoted width.

Narrow integer types become int if int represents every original value; otherwise they become unsigned int. Promotion happens before the operation, not only on assignment. On the usual host, uint8_t{250}+uint8_t{10} is int 260; converting afterward to uint8_t yields 4.

## Prerequisites

- [[Fixed-width integer types]] — When supplied, std::uint32_t has exactly 32 bits; uint_least32_t has at least 32; uint_fast32_t is chosen for speed with at least 32.
- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.

## Used by

- [[Usual arithmetic conversions]] — uses integral promotion as part of its representation or reasoning.

## Recall and next depth

Explain integral promotion without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** enumeration promotions, overload resolution, and implementation-specific ranks.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
