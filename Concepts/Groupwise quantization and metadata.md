---
tags:
  - concept
  - foundation
---

# Groupwise quantization and metadata

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

Local groups share a scale, improving adaptation at a metadata cost. Symmetric INT4 calibration maxAbs/7 reconstructs codes around [-7,7]; float scales add about 32/G bits per value for full groups.

## Prerequisites

- [[Affine quantization and zero point]] — supplies the representation or validity rule used above.
- [[Bit packing]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** No task-accuracy or throughput benchmark was run.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Reconstruction depth

Groupwise quantization divides a tensor into blocks and assigns quantization parameters per block rather than per entire tensor. For affine quantization, a group may store scale $s$ and zero point $z$ so that $\hat x=s(q-z)$.

The group size controls a three-way trade: smaller groups follow local distributions more accurately but increase metadata bandwidth and reduce the work sharing one scale; larger groups amortize metadata and simplify vector kernels but increase approximation error when values have heterogeneous ranges.

Metadata is part of the runtime format. A decoder needs the exact grouping axis and order, group length, code width, scale type, zero-point type, tail policy, alignment and byte order. A mismatch can produce plausible but entirely wrong model outputs.

Benchmarking should report payload plus metadata bytes and measure the real dequantized dot-product kernel rather than only offline reconstruction error. The best group size depends on cache behavior, SIMD width, memory bandwidth and model sensitivity.

