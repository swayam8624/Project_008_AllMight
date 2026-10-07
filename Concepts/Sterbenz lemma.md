---
tags:
  - concept
  - foundation
---

# Sterbenz lemma

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

For nonnegative stored radix-format floats x,y with x/2≤y≤2x, the difference is representable exactly under appropriate gradual-underflow assumptions. This does not recover errors already present in the operands.

## Prerequisites

- [[Cancellation and loss of significance]] — Near-equal operand errors motivate exact-subtraction distinctions and reformulation.
- [[Subnormals and gradual underflow]] — An exact tiny difference may need gradual-underflow representability.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Prove range/significand details for a chosen format; FTZ changes assumptions.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
