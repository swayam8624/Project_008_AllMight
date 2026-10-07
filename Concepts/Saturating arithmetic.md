---
tags:
  - concept
  - foundation
---

# Saturating arithmetic

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Clamp unrepresentable results to range boundaries instead of wrapping; useful for bounded media values but generally non-associative.

Saturation clamps a mathematical result to a permitted interval: unsigned-byte 250+20 becomes 255 instead of 14. Clamp a widened result. Signed saturation is not associative: saturating (100+100)−100 yields 27 while 100+saturating(100−100) yields 100.

## Prerequisites

- [[Arithmetic policies]] — A policy states the intended response to an unrepresentable answer: wrap modulo a base, report checked failure, widen storage, clamp to an endpoint, or grow arbitrary precision.
- [[Signed integer overflow]] — Overflow concerns the operation's signed type after promotions.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain saturating arithmetic without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** SIMD saturation, normalized formats, HDR policy, and evaluation-order effects.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
