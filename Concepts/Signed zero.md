---
tags:
  - concept
  - foundation
---

# Signed zero

**Curriculum depth:** Introduced · **First seen:** C004 · **Latest development:** C004

## Definition and mechanism

Exponent and fraction zero permit either sign. 00000000 and 80000000 compare numerically equal, but signbit and reciprocal distinguish them. Ordinary x<0 does not detect negative zero.

## Prerequisites

- [[Binary32]] — The concrete fields provide the representation used in this mechanism.

## Used by

- [[ULP distance policies]] — uses this mechanism in its explanation.
- [[Floating-point canonicalization]] — uses this mechanism in its explanation.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Specify zero equality separately from raw-byte equality and ULP ordering.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]

## Reconstruction depth

IEEE-754 has both $+0$ and $-0$. They compare equal numerically in ordinary equality, but the sign remains observable through operations such as reciprocal, sign-bit inspection and some branch-cut-sensitive functions:

$$
\frac{1}{+0}=+\infty,
\qquad
\frac{1}{-0}=-\infty.
$$

Signed zero preserves directional information after underflow or exact cancellation. That can matter in complex functions, interval boundaries and geometric code that interprets the sign of a quantity near a separating surface.

It can also surprise serialization, hashing and deterministic comparison. If the application's semantic model says both zeros are identical, canonicalization may deliberately map negative zero to positive zero before hashing or bytewise equality. If directional zero is meaningful, that canonicalization destroys information.

Tests should distinguish numeric equality from bit representation and include operations that expose the sign. This is one reason floating-point equality policy must start from domain semantics rather than from a universal epsilon.

