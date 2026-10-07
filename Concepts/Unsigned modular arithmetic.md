---
tags:
  - concept
  - foundation
---

# Unsigned modular arithmetic

**Curriculum depth:** Developed · **First seen:** C001 · **Latest development:** C002

## Definition and mechanism

Conversion and arithmetic in an unsigned $N$-bit domain produce residues modulo $2^N$; intentional wrap is valid, but wrapped bounds expressions can become security bugs.

Modulo m retains the remainder class after division by m. An unsigned N-bit domain uses m=2ᴺ: 250+10 becomes 4 when stored in eight bits, and 0−1 becomes 255. Narrow operands may promote first, so identify the operation type separately from the destination type.

## Prerequisites

- [[Positional notation]] — A base is the number of available digits.
- [[Machine word]] — A processor's natural integer processing width is often called its word width, but the term is architecture dependent.

## Used by

- [[Offset-binary coding]] — uses this mechanism to define its representation, error or validation contract.
- [[Checked binary parsing]] — uses this mechanism to define its representation, error or validation contract.

- [[Two's complement]] — uses unsigned modular arithmetic as part of its representation or reasoning.
- [[Arithmetic policies]] — uses unsigned modular arithmetic as part of its representation or reasoning.

## Recall and next depth

Explain unsigned modular arithmetic without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** modular algorithms, counters, hashes, and sequence-number comparisons.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 teaching chapter]] · [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 development]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
