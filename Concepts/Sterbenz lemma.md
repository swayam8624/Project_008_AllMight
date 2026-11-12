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

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]

## Exact subtraction in a bounded ratio

Sterbenz's lemma gives a rare exactness guarantee for floating subtraction. Under the usual floating-point assumptions, if two finite floating values $x$ and $y$ satisfy

$$
\frac{y}{2} \le x \le 2y,
$$

then $x-y$ is exactly representable in the same floating format, provided the result remains in the supported range conditions.

This explains why cancellation is not synonymous with rounding error. Subtracting two nearby floating numbers can lose **relative information about the original quantities**, yet the subtraction operation itself may be exact. The conditioning of the problem and the rounding of an operation are different questions.

The lemma is useful when proving error bounds for compensated algorithms and geometric predicates. A reconstruction test should generate representable pairs inside and outside the ratio interval, compare the subtraction against a higher-precision exact reference, and distinguish an exactly rounded zero/small difference from a well-conditioned final answer.

