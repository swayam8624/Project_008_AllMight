---
tags:
  - concept
  - foundation
---

# Fused multiply-add

**Curriculum depth:** Introduced · **First seen:** C005 · **Latest development:** C005

## Definition and mechanism

FMA rounds ab+c once rather than rounding the product first. For p=2^-23,a=b=1+p,c=−(1+2p), separate binary32 gives zero, FMA gives exact 2^-46. Compiler contraction can change an unfused-looking expression.

## Prerequisites

- [[IEEE-754 floating point]] — Fused operation semantics and destination rounding are part of the arithmetic contract.
- [[Rounding to nearest ties to even]] — Midpoint decisions determine separate/fused results and narrowed persistent state.

## Used by

- [[Floating-point reassociation]] — uses this mechanism to establish its numerical contract.
- [[Dot and cross-product rounding]] — uses this mechanism to establish its numerical contract.

## Recall and next depth

Reconstruct the definition's example and name the assumptions that make it valid.

**Pending:** Specify FMA policy for reference/reproducible builds; a fused chain is not a fully exact dot product.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005 teaching chapter]] · [[Supplementary/Foundations#M005 - Parts 77-88|Part definitions]] · [[Dependency Map]]
