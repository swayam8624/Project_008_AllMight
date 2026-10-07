---
tags:
  - concept
  - foundation
---

# Arithmetic policies

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Modular, checked, widened, saturated, and arbitrary-precision arithmetic define different outcomes when a mathematical result does not fit.

A policy states the intended response to an unrepresentable answer: wrap modulo a base, report checked failure, widen storage, clamp to an endpoint, or grow arbitrary precision. Choose by subsystem meaning. Color-byte output and hash arithmetic need different policies.

## Prerequisites

- [[Unsigned modular arithmetic]] — Modulo m retains the remainder class after division by m.
- [[Two's complement]] — For raw unsigned value u in N bits, signed decoding is u when u<2ᴺ⁻¹, otherwise u−2ᴺ.

## Used by

- [[Quantization and compressed weights]] — uses arithmetic policies as part of its representation or reasoning.
- [[Saturating arithmetic]] — uses arithmetic policies as part of its representation or reasoning.

## Recall and next depth

Explain arithmetic policies without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** choosing and documenting policies per subsystem.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
