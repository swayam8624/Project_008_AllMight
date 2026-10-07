---
tags:
  - concept
  - foundation
---

# Positional notation

**Curriculum depth:** Introduced · **First seen:** C001 · **Latest development:** C001

## Definition and mechanism

Digits receive powers-of-base weights; binary uses base two and $V=\sum b_i2^i$.

A base is the number of available digits. Position i contributes digit × baseⁱ; the rightmost position is i=0. For `1011₂`, the weights 8,4,2,1 give 8+2+1=11. The number of states is 2ᴺ, while the maximum unsigned value is 2ᴺ−1.

## Prerequisites

- [[Bit]] — A bit has two possible logical states.

## Used by

- [[Significand and hidden bit]] — uses positional notation to establish this mechanism's representation or policy.

- [[Binary conversion]] — uses positional notation as part of its representation or reasoning.
- [[Hexadecimal]] — uses positional notation as part of its representation or reasoning.
- [[Octal]] — uses positional notation as part of its representation or reasoning.
- [[Bit significance]] — uses positional notation as part of its representation or reasoning.
- [[Unsigned modular arithmetic]] — uses positional notation as part of its representation or reasoning.
- [[Signed magnitude]] — uses positional notation as part of its representation or reasoning.
- [[Carry and borrow]] — uses positional notation as part of its representation or reasoning.

- [[Big endian]] — depends on this concept; see its prerequisite explanation.

## Recall and next depth

Explain positional notation without the label, work the example above, then state the assumption whose removal would change the result.

**Pending:** signed and fractional encodings.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#C001 - M001 - Parts 0001-0020 - Binary states to packed Boolean meaning|C001 teaching chapter]] · [[Supplementary/Foundations|Part definitions and mechanisms]] · [[Dependency Map]]
