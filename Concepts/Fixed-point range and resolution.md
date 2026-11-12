---
tags:
  - concept
  - foundation
---

# Fixed-point range and resolution

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

For signed N-bit raw storage and F fraction bits, step is 2^-F and the positive endpoint is (2^(N-1)-1)/2^F. Required step 0.001 and magnitude 1e6 in 32 bits allow F=10 or 11.

## Prerequisites

- [[Fixed-point representation]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Coordinate-origin and wider-carrier designs need their own range proofs.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Reconstruction depth

A fixed-point value stores an integer $q$ together with an implied scale $s$, so the represented real quantity is

$$
x=q\,s.
$$

For a binary fractional format with $F$ fractional bits, $s=2^{-F}$. Increasing $F$ improves resolution but consumes integer range because storage width has not changed. For signed $N$-bit two's-complement storage, the approximate real range is

$$
[-2^{N-1}s,(2^{N-1}-1)s].
$$

This makes format choice an error-budget decision rather than a syntax decision. Position data may need large range; normalized coefficients may need fine local resolution. Multiplication temporarily doubles the number of fractional bits, so a production operation needs a widened intermediate or a deliberate rescale, round and saturate policy.

Tests should exercise representable endpoints, half-step rounding, overflow, negative values, multiplication rescaling and accumulation. A format can be perfectly deterministic and still be numerically unsuitable if its step size exceeds the physical or visual tolerance of the subsystem using it.

