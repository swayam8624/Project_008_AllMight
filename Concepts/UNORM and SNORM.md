---
tags:
  - concept
  - foundation
---

# UNORM and SNORM

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

UNORM decodes q/(2^n-1); UNORM8 code128 gives128/255. SNORM commonly clamps q/(2^(n-1)-1) at -1, so -128 and -127 both map to -1 for eight bits.

## Prerequisites

- [[Affine quantization and zero point]] — supplies the representation or validity rule used above.

## Used by

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** Hardware/API conversion and supported format use must be verified per target; sRGB is separate.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Choosing what information to keep|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]

## Reconstruction depth

An $n$-bit **UNORM** integer $q\in[0,2^n-1]$ represents a real value approximately in $[0,1]$ through

$$
x = \frac{q}{2^n-1}.
$$

Quantization reverses that map with a declared rounding rule and saturation. The endpoint denominator matters: dividing by $2^n$ would make the largest code fail to represent exactly $1$. This is why a normalized color channel can round-trip its endpoints even though most interior values incur quantization error.

**SNORM** maps a signed integer code to approximately $[-1,1]$. APIs differ in how the most-negative integer is treated, so the format specification—not intuition—owns the exact decode rule. This matters for normals and tangent vectors: decoding produces components, not a guaranteed unit vector. A sampled normal map usually requires later reconstruction or normalization and a coordinate-space convention.

A useful test suite checks endpoint codes, monotonicity, round-trip error bounds, saturation outside the nominal domain, and the API's exact most-negative SNORM behavior. Treating UNORM/SNORM as "small floats" hides where nonlinear color encoding, signed-vector normalization, and integer filtering contracts actually enter.

