---
tags:
  - concept
  - foundation
---

# Signed integer overflow

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

A signed arithmetic result outside its type's range is undefined in C++; hardware low bits do not create a language-level wrap guarantee.

Overflow concerns the operation's signed type after promotions. int maximum + 1 is undefined. For checked addition, reject b>0 and a>max−b, or b<0 and a<min−b, before evaluating a+b. Hardware status flags do not change the C++ contract.

## Prerequisites

- [[Minimum signed value]] — N-bit two's complement spans −2ᴺ⁻¹ through 2ᴺ⁻¹−1.
- [[Undefined behavior]] — The C++ language imposes no requirements on an execution that invokes undefined behavior.

## Used by

- [[Saturating arithmetic]] — uses signed integer overflow as part of its representation or reasoning.

## Recall and next depth

Explain signed integer overflow without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** checked arithmetic facilities, optimizer transformations, and sanitizers.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
