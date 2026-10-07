---
tags:
  - concept
  - future
---

# Carry-lookahead and parallel-prefix adders

**Curriculum depth:** Forward · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Generate/propagate terms expose carries for parallel evaluation instead of serial ripple.

Generate Gᵢ=Aᵢ AND Bᵢ creates carry locally; propagate Pᵢ=Aᵢ XOR Bᵢ passes carry through. Expanding Cᵢ₊₁=Gᵢ OR (Pᵢ AND Cᵢ) permits parallel carry computation. Detailed prefix networks and wiring trade-offs remain future work.

## Prerequisites

- [[Ripple-carry addition]] — Connect carry-out at bit i to carry-in at i+1.
- [[Bitwise AND]] — Per lane: 0&0=0, 0&1=0, 1&0=0, 1&1=1.
- [[Bitwise OR]] — Per lane: only 0|0 is 0; the other three cases are 1.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain carry-lookahead and parallel-prefix adders without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** carry-lookahead, Kogge-Stone, Brent-Kung, carry-select, area, power, and wiring trade-offs.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C002 - M002 - Parts 0021-0040 - Finite integers, ALU arithmetic, and C++ hazards|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
