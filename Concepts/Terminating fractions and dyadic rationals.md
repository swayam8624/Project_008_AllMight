---
tags:
  - concept
  - foundation
---

# Terminating fractions and dyadic rationals

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

A reduced rational p/q terminates in base B iff q divides a power of B. In binary q must be a power of two. Binary32 0.1 is exactly 13421773/2^27, chosen as an approximation of non-dyadic 1/10.

## Prerequisites

- [[Positional notation]] — Finite fractional digits correspond to an integer over a power of the base.
- [[Binary32]] — Its finite significand/exponent makes each stored finite value a dyadic rational.

## Used by

Connect later dependent concepts here when their lessons arrive.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Other bases, decimal conversion algorithms and exact rational arithmetic.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]

## Why decimal-friendly numbers can be binary-hostile

A reduced rational number $p/q$ has a terminating representation in base $b$ exactly when every prime factor of $q$ also divides $b$. Base 10 has prime factors 2 and 5, so fractions such as $1/10$ terminate. Base 2 has only prime factor 2, so only **dyadic rationals** with denominator $2^k$ terminate.

That is why $0.5$, $0.25$, and $0.125$ are exact in ordinary binary floating point while $0.1$ repeats forever and must be rounded. The issue is not that floating point is “bad at decimals”; it is a consequence of positional representation.

This perspective helps debugging: multiplying a binary approximation to 0.1 by ten need not recreate the exact path one imagines from decimal arithmetic. For currency or protocol fields whose semantics are decimal-exact, scaled integers or decimal floating formats may be better contracts.

A reconstruction exercise should reduce several fractions, classify their denominator prime factors, and predict whether they terminate in bases 2, 10, and 16 before checking actual floating encodings.

