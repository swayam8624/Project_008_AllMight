---
tags:
  - concept
  - foundation
---

# Floating-point canonicalization

**Curriculum depth:** Developed · **First seen:** C004 · **Latest development:** C006

## Definition and mechanism

Canonicalization deliberately merges encodings: both zeros to positive zero, all NaNs to one quiet pattern. Raw preservation instead keeps sign/payload distinctions. Neither is universally correct; this is a file/hash contract choice.

M006's BF16 converter explicitly canonicalizes NaNs while its raw tensor codec preserves uint32 payload words. These are distinct contracts: numerical closeness, canonical identity and raw byte identity must not be conflated.

## Prerequisites

- [[Signed zero]] — Numerical equality merges two encodings and must be reflected in the chosen ordering or byte policy.
- [[NaNs and payloads]] — Unordered comparisons and payload distinctions require explicit metric, comparison and serialization policy.
- [[Serialization]] — Changing canonical representation changes the external byte contract.
- [[Bit casting and representation]] — A representation word must be distinguished from a numeric conversion before canonicalization.

## Used by

Connect later dependent concepts here as their lessons arrive.

## Recall and next depth

Reconstruct the concrete example in the definition and identify its validity assumptions.

**Pending:** Do not silently change an existing asset format; cross-platform payload transport needs its own tests.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Narrative development]]

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004 teaching chapter]] · [[Supplementary/Foundations#M004 - Parts 61-76|Per-part definitions]] · [[Dependency Map]]
