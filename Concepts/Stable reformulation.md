---
tags:
  - concept
  - foundation
---

# Stable reformulation

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A mathematically equivalent expression can avoid unnecessary numerical loss. sqrt(x+1)−sqrt(x)=1/(sqrt(x+1)+sqrt(x)) for x≥0; at double x=1e16 the direct result is zero while the reciprocal form remains near 5e-9.

## Prerequisites

- [[Cancellation and loss of significance]] — Near-equal operand errors motivate exact-subtraction distinctions and reformulation.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Every reformulation still needs domain, range, conditioning and accuracy checks.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
