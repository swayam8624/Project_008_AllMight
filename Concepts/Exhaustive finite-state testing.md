---
tags:
  - concept
  - foundation
---

# Exhaustive finite-state testing

**Curriculum depth:** Introduced · **First seen:** C002 · **Latest development:** C002

## Definition and mechanism

Small fixed domains can be enumerated completely; eight-bit add-with-carry has $256\times256\times2=131{,}072$ inputs.

Exhaustive testing enumerates every input in a chosen finite domain. Two bytes and carry give 256×256×2=131072 inputs. Use an independent signed-range oracle instead of restating the same overflow formula; complete arithmetic inputs do not cover all emulator state.

## Prerequisites

- [[Interpretation contract]] — For `0xAD`, unsigned interpretation gives 173; eight-bit two's complement gives −83.
- [[Boolean algebra]] — The logical domain is false/true.

## Used by

Connect later chunks here when they use this concept.

## Recall and next depth

Explain exhaustive finite-state testing without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** Apply and reconstruct this concept independently.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
